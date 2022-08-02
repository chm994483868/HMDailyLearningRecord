# 函数调用堆栈

&emsp;在学习线程的相关知识时，我们一定接触过 NSThread 类，它有一个类属性：`@property (class, readonly, copy) NSArray<NSString *> *callStackSymbols` 用来获取当前线程的函数调用堆栈，该属性返回包含调用堆栈符号的数组，每个元素都是一个 NSString 对象，其值的格式由 backtrace_symbols() 函数确定。初看此属性给人眼前一亮，但是看到它仅是一个类属性时，我们仿佛意识到了什么，对，它有一个限制，此类属性返回值仅能描述调用此方法时当前线程的调用堆栈回溯。我们常见的在一个子线程中抓取主线程的函数调用堆栈的场景它就无法胜任了。例如：进行性能监控时通常会开一个子线程监控，如监控主线程的卡顿情况、CPU Usage 情况，当出现主线程卡顿、某个线程 CPU Usage 占用过高时，需要进行抓栈，那么就需要通过其它方式获取非当前线程的调用栈了。

> &emsp;在 Developer Documentation 中搜索 callStackSymbols 时，发现 NSException 类中也有一个同名的实例属性：`@property(readonly, copy) NSArray<NSString *> *callStackSymbols;` 此实例属性描述引发 Objective-C 异常时调用堆栈回溯的字符串数组。每个字符串的格式由 backtrace_symbols() 确定，感兴趣的小伙伴可以尝试在一个 Try-Catch 中制造一个 Objective-C 异常，打印一下 callStackSymbols 属性的内容试一下。

&emsp;下面我们以大佬的 [bestswifter/BSBacktraceLogger](https://github.com/bestswifter/BSBacktraceLogger) 代码为示例来研究 iOS 下对函数调用栈进行回溯的过程。
 
## Mach 线程（mach_port_t）

&emsp;继续开始之前我们需要补充一些 macOS 三种线程的关系：

1. `pthread_t pthread_self(void)` 返回的是 `pthread_t`，glibc 库的线程 id。实际上是线程控制块 tcb 首地址。(pthread_self 是 POSIX 标准中的接口，pthread_t 是 POSIX 标准中线程的类型) 
2. `syscall(SYS_gettid)` 内核级线程 id，系统唯一。该函数为系统调用函数，glibc 可能没有该函数声明，此时需要使用 syscall(SYS_gettid)。可以参考 [pthread_self() VS syscall(SYS_gettid)](https://www.cnblogs.com/alix-1988/p/14886847.html) 详细了解它们之间的关系。（此种线程不涉及本篇的知识点，可直接忽略）
3. 此种正是 macOS 中 Mach 内核中的线程（在 mach 中或者我们把线程理解为一个个端口，获取某条线程就是获取此线程的端口）：`mach_port_t mach_thread_self(void)` 直接获取线程端口 mach_port_t，另外一种是：首先 `pthread_t pthread_self(void)` 获取 POSIX 标准线程：`pthread_t`，然后通过 `mach_port_t pthread_mach_thread_np(pthread_t)` 把它转换为 Mach 线程（端口）：`mach_port_t`。`mach_port_t` 是 macOS 特有的 id，实际上不能说是 thread id，而应该当做是线程端口，它是 Mach 中表示线程的一种方式。
  
&emsp;上面的第 3 条获取 Mach 线程的方式中，其中涉及一些内存方面的问题，看到这个链接 [一个“反问”引发的内存反思](https://blog.csdn.net/killer1989/article/details/106674973) 我们可以详细阅读一下，增强一下对 `extern mach_port_t mach_thread_self(void);` 和 `mach_port_t pthread_mach_thread_np(pthread_t);` 函数的认识，其中 np 是 not posix 的首字母缩写。

```c++
/* return the mach thread bound to the pthread 返回绑定到 pthread 的 mach 线程 */
__API_AVAILABLE(macos(10.4), ios(2.0))
mach_port_t pthread_mach_thread_np(pthread_t);
```

&emsp;针对上述文章中提到的优化点，我们大概可以通过以下两种方式获取 Mach 线程（port）：

```c++
// 方式 1
mach_port_t safe_thread_self(void) {
    // mach_thread_self 和 mach_port_deallocate 配对使用，使用完毕后立刻释放空间 
    mach_port_t thread_self = mach_thread_self();
    mach_port_deallocate(mach_task_self(), thread_self);
    return thread_self;
}

// 方式 2
mach_port_t safe_thread_self(void) {
    return pthread_mach_thread_np(pthread_self());
}
```

&emsp;下面我们开始分析 BSBacktraceLogger 源码。

## DEFINE MACRO FOR DIFFERENT CPU ARCHITECTURE 

&emsp;BSBacktraceLogger.m 文件开头，定义了一大组宏，主要针对不同的 CPU 架构体系定义的。这里主要是 `__arm64__`（64 位 ARM 架构）、`__arm__`（32 位 ARM 架构）、`__x86_64__`（64 位 x86 架构）、`__i386__`（32 位 x86 架构），这里我们主要关注 `__x86_64__` 和 `__arm64__` 就好了。

```c++
#pragma -mark DEFINE MACRO FOR DIFFERENT CPU ARCHITECTURE

#if defined(__arm64__)

// 地址与 ~(3UL) 做与操作，即把地址二进制表示的最后两位置 0
#define DETAG_INSTRUCTION_ADDRESS(A) ((A) & ~(3UL))

// ARM 架构 64 位下表示线程状态个数，此值没什么用，仅用于 thread_get_state 函数中的第四个临时参数。x86 下值为 42
#define BS_THREAD_STATE_COUNT ARM_THREAD_STATE64_COUNT
// ARM 架构 64 位表示线程状态，此值没什么用，仅用于 thread_get_state 函数中的第二个临时参数。x86 下值为 4
#define BS_THREAD_STATE ARM_THREAD_STATE64

// 栈底寄存器
#define BS_FRAME_POINTER __fp
// 栈顶寄存器
#define BS_STACK_POINTER __sp
// ARM 架构下：程序计数器寄存器
#define BS_INSTRUCTION_ADDRESS __pc

#elif defined(__arm__)

// 地址与 ~(1UL) 做与操作，最后一位置 0
#define DETAG_INSTRUCTION_ADDRESS(A) ((A) & ~(1UL))

// 同上的两个临时参数
#define BS_THREAD_STATE_COUNT ARM_THREAD_STATE_COUNT
#define BS_THREAD_STATE ARM_THREAD_STATE

// 栈底寄存器
#define BS_FRAME_POINTER __r[7]
// 栈顶寄存器
#define BS_STACK_POINTER __sp
// ARM 架构下：程序计数器寄存器
#define BS_INSTRUCTION_ADDRESS __pc

#elif defined(__x86_64__)

// 不变
#define DETAG_INSTRUCTION_ADDRESS(A) (A)

// 同上的两个临时参数
#define BS_THREAD_STATE_COUNT x86_THREAD_STATE64_COUNT
#define BS_THREAD_STATE x86_THREAD_STATE64

// 栈底寄存器
#define BS_FRAME_POINTER __rbp
// 栈顶寄存器
#define BS_STACK_POINTER __rsp
// x86 64 位架构，对应 ARM 架构下 PC 寄存器
#define BS_INSTRUCTION_ADDRESS __rip

#elif defined(__i386__)

// 保持不变
#define DETAG_INSTRUCTION_ADDRESS(A) (A)

// 同上的两个临时参数
#define BS_THREAD_STATE_COUNT x86_THREAD_STATE32_COUNT
#define BS_THREAD_STATE x86_THREAD_STATE32

// 栈底寄存器
#define BS_FRAME_POINTER __ebp
// 栈顶寄存器
#define BS_STACK_POINTER __esp
// x86 32 位架构，对应 ARM 架构下 PC 寄存器
#define BS_INSTRUCTION_ADDRESS __eip

#endif

#define CALL_INSTRUCTION_FROM_RETURN_ADDRESS(A) (DETAG_INSTRUCTION_ADDRESS((A)) - 1)

#if defined(__LP64__)

// 针对 64 位架构输出地址字符串的格式
#define POINTER_FMT       "0x%016lx"
#define POINTER_SHORT_FMT "0x%lx"

// 64 位架构中符号表数组中元素的类型
#define BS_NLIST struct nlist_64

#else

// 针对 32 位架构输出地址字符串的格式
#define POINTER_FMT       "0x%08lx"
#define POINTER_SHORT_FMT "0x%lx"

// 32 位架构中符号表数组中元素的类型
#define BS_NLIST struct nlist

#endif
```

## BSStackFrameEntry

&emsp;`struct BSStackFrameEntry` 是定义的用来表示函数栈帧内栈底指针（FP 寄存器）链表节点的结构体，这里其实很简单但是描述的有点绕，BSStackFrameEntry 结构体中的 `const uintptr_t return_address` 成员变量表示的是一个栈帧中的栈底寄存器（FP）的值，即 BSStackFrameEntry 结构体是用来构建栈底指针链表时的节点结构体。

```c++
// 用于表示栈帧栈底指针 FP 链表的结构体
typedef struct BSStackFrameEntry{
    const struct BSStackFrameEntry *const previous;
    const uintptr_t return_address;
} BSStackFrameEntry;

// 用于记录主线程 port 的全局变量
static mach_port_t main_thread_id;

+ (void)load {
    // main_thread_id 是在 load 方法中赋值，确保一定是在主线程
    main_thread_id = mach_thread_self();
}
```

&emsp;这里直接使用的 `mach_thread_self()` 获取主线程，可以替换为上面的 `safe_thread_self()` 函数。

&emsp;继续向下看到定义在 .h 中四个 API 的实现，分别是：获取全部线程的调用栈回溯、获取当前线程的调用栈回溯、获取主线程的调用栈回溯、获取指定 NSThread 的调用栈回溯。

```c++
#pragma -mark Implementation of interface

// 获取指定线程的调用栈回溯
+ (NSString *)bs_backtraceOfNSThread:(NSThread *)thread {
    // 调用 bs_machThreadFromNSThread 函数把 thread 转换为 thread_t（实际是 typedef mach_port_t thread_t）类型的 Mach 线程，然后调用 _bs_backtraceOfThread 获取调用栈回溯字符串
    return _bs_backtraceOfThread(bs_machThreadFromNSThread(thread));
}

// 获取当前线程的调用栈回溯
+ (NSString *)bs_backtraceOfCurrentThread {
    // 对 [NSThread currentThread] 当前线程进行调用栈回溯
    return [self bs_backtraceOfNSThread:[NSThread currentThread]];
}

// 获取主线程的调用栈回溯
+ (NSString *)bs_backtraceOfMainThread {
    // 对 [NSThread mainThread] 主线程进行调用栈回溯
    return [self bs_backtraceOfNSThread:[NSThread mainThread]];
}

// 对当前所有线程进行栈回溯（此函数可以先看下大概过程，下面我们会对其中涉及的函数进行详细讲解）
+ (NSString *)bs_backtraceOfAllThread {
    // 记录当前所有线程的 port
    thread_act_array_t threads;
    
    // 记录当前线程的数量
    mach_msg_type_number_t thread_count = 0;
    
    // 当前的 task
    const task_t this_task = mach_task_self();
    
    // 获取当前所有线程和线程数量，分别记录在 threads 和 thread_count 中
    kern_return_t kr = task_threads(this_task, &threads, &thread_count);
    
    // 如果获取失败的话，返回错误信息
    if (kr != KERN_SUCCESS) {
        return @"Fail to get information of all threads";
    }
    
    // 调用栈回溯字符串的开头拼接上线程数量字符串
    NSMutableString *resultString = [NSMutableString stringWithFormat:@"Call Backtrace of %u threads:\n", thread_count];
    
    // 然后循环对所有的线程进行调用栈回溯，把回溯的字符串拼接在 resultString 中
    for (int i = 0; i < thread_count; i++) {
        [resultString appendString:_bs_backtraceOfThread(threads[i])];
    }
    
    // 返回结果
    return [resultString copy];
}
```

## bs_machThreadFromNSThread（Convert NSThread to Mach thread ）

&emsp;将 NSThread 转换为 Mach 线程（mach_port_t），其实是在当前 task 的所有线程中找到与指定 NSThread 对应的 Mach 线程。 

```c++
#pragma -mark Convert NSThread to Mach thread

/// NSThread 转换为 thread_t 类型的 Mach 线程
/// @param nsthread NSThread 线程对象
thread_t bs_machThreadFromNSThread(NSThread *nsthread) {
    char name[256];
    
    // 用来存储当前 task 的线程数量
    mach_msg_type_number_t count;
    
    // 用来存储当前所有线程的 mach_port_t 的数组（typedef mach_port_t thread_t; mach_port_t 是 thread_t 的别名）
    
    // 这里我们按住 command 点击查看一下 thread_act_array_t 的实际类型：
    // 首先 `typedef thread_act_t *thread_act_array_t;` 看到 thread_act_array_t 是一个 thread_act_t 指针，
    // 然后 `typedef mach_port_t thread_act_t;` 即 list 实际就是一个 mach_port_t 数组，实际就是一个 thread_t 数组。
    thread_act_array_t list;
    
    // 调用 task_threads 函数根据当前的 task 来获取所有线程（线程端口），保存在 list 变量中，count 记录线程的总数量
    
    // mach_task_self() 表示获取当前的 Mach task，它的类型其实也是 mach_port_t，这里牵涉到 macOS 中 Mach 微内核用户态和内核态的一些的知识点。
    // mach_task_self() 获取当前 task，看到该函数返回的类型也是 mach_port_t：extern mach_port_t mach_task_self_;
    // #define mach_task_self() mach_task_self_
    task_threads(mach_task_self(), &list, &count);
    
    // 当前时间戳
    NSTimeInterval currentTimestamp = [[NSDate date] timeIntervalSince1970];
    
    // 取出 nsthread 的 name 记录在 originName 中（大概率是空字符串，如果没有给 thread 设置 name 的话），然后取当前的时间戳作为 nsthread 的新名字
    NSString *originName = [nsthread name];
    
    // 这里除了把 nsthread 的名字设置为时间戳，也会把 nsthread 对应的 pthread_t 的名字设置为同一个值
    [nsthread setName:[NSString stringWithFormat:@"%f", currentTimestamp]];
    
    // 如果 nsthread 是主线程的话直接返回在 +load 函数中获取的主线程的 mach_port_t
    if ([nsthread isMainThread]) {
        // 这里直接把 mach_port_t 强制转换为了 thread_t，因为实际 `typedef mach_port_t thread_t;`，mach_port_t 就是 thread_t 的别名
        return (thread_t)main_thread_id;
    }
    
    // 遍历 list 数组中的 mach_port_t
    for (int i = 0; i < count; ++i) {
        
        // _Nullable pthread_t pthread_from_mach_thread_np(mach_port_t);
        // 调用 pthread_from_mach_thread_np 函数，从 mach_port_t 转换为 pthread_t，注意这里是 pthread_t 比上面的 thread_t 多了一个 p
        pthread_t pt = pthread_from_mach_thread_np(list[i]);
        
        printf("✈️✈️✈️ \t%p\n", pt);
        
        // 这里的再一次的 if ([nsthread isMainThread]) {} 判断，没看懂，上面不是有了一个判断了吗？
        // 如果是主线程的话，再次返回主线程对应的 mach_port_t
        if ([nsthread isMainThread]) {
            // mach_port_t 相等的话，直接返回即可
            if (list[i] == main_thread_id) {
                return list[i];
            }
        }
        
        // 获取 pt 的名字，然后与 nsthread 进行比较，取得 nsthread 对应的 thread_t
        if (pt) {
            name[0] = '\0';
            
            // 获取 pthread_t 的名字，保存在 name char 数组中
            pthread_getname_np(pt, name, sizeof name);
            
            printf("🔥🔥🔥 \t%s\n", name);
            printf("🔥🔥🔥🔥 \t%s\n", [nsthread name].UTF8String);
            
            if (!strcmp(name, [nsthread name].UTF8String)) {
                [nsthread setName:originName];
                return list[i];
            }
        }
    }
    
    // nsthread 设置回原始的名字
    [nsthread setName:originName];
    
    // 返回当前线程的 port（`mach_port_t`）
    return mach_thread_self();
}

```
















&emsp;FP(x29) 栈底 SP 栈顶 PC 下一条指令 LR(x30) 函数返回后的下一个函数的第一条指令


## 参考链接
**参考链接:🔗**
+ [iOS开发--探究iOS线程调用栈及符号化](https://blog.csdn.net/qq_36237037/article/details/107473867)
+ [iOS内存扫描工具实现](https://blog.csdn.net/liumazi/article/details/106417276)
+ [获取任意线程的调用栈](https://zhuanlan.zhihu.com/p/357039492)








+ [iOS中符号的那些事儿](https://juejin.cn/post/6844904164208689166)

+ [bestswifter/BSBacktraceLogger](https://github.com/bestswifter/BSBacktraceLogger)
+ [iOS——CPU监控](https://blog.csdn.net/cym_bj/article/details/109677752)
+ [深入解析Mac OS X & iOS 操作系统 学习笔记（十二）](https://www.jianshu.com/p/cc655bfdac13)
+ [一个“反问”引发的内存反思](https://blog.csdn.net/killer1989/article/details/106674973)
+ [BSBackTracelogger学习笔记](https://juejin.cn/post/6910791727670362125#heading-6)
+ [Swift堆栈信息获取](https://juejin.cn/post/6979138204154724382)
+ [MAC OS 的 mach_port_t 和 pthread_self()](https://blog.csdn.net/yxccc_914/article/details/79854603)
+ [iOS性能监控](https://www.jianshu.com/p/f04a1447aaa0?utm_campaign=shakespeare)
+ [syscall(SyS_gettid)是什么](https://blog.csdn.net/woainilixuhao/article/details/100144159)
+ [pthread_self() VS syscall(SYS_gettid)](https://www.cnblogs.com/alix-1988/p/14886847.html)
