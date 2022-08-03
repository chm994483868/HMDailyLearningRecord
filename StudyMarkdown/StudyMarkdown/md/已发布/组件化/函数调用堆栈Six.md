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

&emsp;然后 ARM 架构下：FP(x29) 栈底 SP 栈顶 PC 下一条指令 LR(x30) 函数返回后的下一个函数的第一条指令 寄存器相关的内容，网上好多大佬都做了总结，这里就不再展开了。可以参考这篇：[iOS开发--探究iOS线程调用栈及符号化](https://blog.csdn.net/qq_36237037/article/details/107473867)。

&emsp;下面我们开始分析 BSBacktraceLogger 源码。

## DEFINE MACRO FOR DIFFERENT CPU ARCHITECTURE 

&emsp;BSBacktraceLogger.m 文件开头，定义了一大组宏，主要针对不同的 CPU 架构体系定义的。这里主要是 `__arm64__`（64 位 ARM 架构）、`__arm__`（32 位 ARM 架构）、`__x86_64__`（64 位 x86 架构）、`__i386__`（32 位 x86 架构），这里我们主要关注 `__x86_64__` 和 `__arm64__` 就好了。

```c++
#pragma -mark DEFINE MACRO FOR DIFFERENT CPU ARCHITECTURE

#if defined(__arm64__)

// DETAG_INSTRUCTION_ADDRESS 是为了去掉指令地址中的指针标签，ARM 指令分为 Thumb 和 Normal，指令长度分别为 16 位和 32 位，ARM64 下指令长度为 32 位，所以至少是按照 4 字节对齐，指令地址一定是 4 的整数倍，指令最低 2 位正常总是为 0，系统可能会在这两位插入指针标签
// 所以要去掉这个标签的值，得到真正的指令地址，ARM64 去除低两位，armv7 下指令可能为 Thumb 和 Normal，同理去除最低一位
// x86_64 和 i386 中指令是可变长度的，所有位都是有意义的，不进行去除

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
// 用于表示栈帧栈底指针 FP 链表的结构体，本质结构为 x29、x30 组合，即 FP 和 LR 寄存器
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
    // 调用 bs_machThreadFromNSThread 函数把 thread 转换为 thread_t（实际是 typedef mach_port_t thread_t）类型的 Mach 线程，然后调用 _bs_backtraceOfThread 获取调用栈回溯字符串，
    // 这里并不是一个什么数据结构的转换过程，只是一个对应线程的查询：在当前 task 的所有线程中找到与指定 NSThread 对应的 Mach 线程。 
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
        
            // 如果 mach_port_t 相等的话，直接返回 list[i] 即可
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
            
            // strcmp 函数是 string compare（字符串比较）的缩写，用于比较两个字符串并根据比较结果返回整数。基本形式为 strcmp(str1,str2)，若 str1=str2，则返回零；若 str1<str2，则返回负数；若str1>str2，则返回正数。
            
            // 如果两者相等，表示找到了 nsthread 对应的 Mach 线程，然后把 nsthread 恢复原名，并返回 list[i]
            if (!strcmp(name, [nsthread name].UTF8String)) {
                [nsthread setName:originName];
                return list[i];
            }
        }
    }
    
    // 未找到 nsthread 对应的 Mach 线程，把 nsthread 恢复为原名
    [nsthread setName:originName];
    
    // 返回当前线程的 port（`mach_port_t`）
    return mach_thread_self();
}
```

&emsp;上面的铺陈和摆设都做好了，下面看下最重要的过程：`return _bs_backtraceOfThread(bs_machThreadFromNSThread(thread));`。

## `_bs_backtraceOfThread`（Get call backtrace of a mach_thread）

&emsp;`NSString *_bs_backtraceOfThread(thread_t thread)` 函数获取指定 Mach 线程（thread_t）的调用栈回溯字符串。

&emsp;这里涉及到一个比较复杂的结构体：`_STRUCT_MCONTEXT machineContext;`。 下面我们以 x86_64 平台为例看下它的具体内容，首先是根据当前是 64 位 CPU 架构所以其指向了 `_STRUCT_MCONTEXT64` 结构体，然后 `_STRUCT_MCONTEXT64` 结构体中嵌套了三个结构体，我们只需要关注其中的 `_STRUCT_X86_THREAD_STATE64 __ss;` 结构体，因为在后续获取线程状态的函数 `thread_get_state` 中，只传递了 `&machineContext->__ss` 作为参数，`__ss` 对应了一组寄存器列表，我们也可以直接把它们理解为线程的执行上下文。所以 `_bs_backtraceOfThread` 函数中申请的 `machineContext` 变量正是用来记录指定线程当前上下文信息的（寄存器列表值）。

```c++
#define _STRUCT_MCONTEXT _STRUCT_MCONTEXT64

#define _STRUCT_MCONTEXT64      struct __darwin_mcontext64
_STRUCT_MCONTEXT64
{
    _STRUCT_X86_EXCEPTION_STATE64   __es; // 异常状态记录
    _STRUCT_X86_THREAD_STATE64      __ss; // 线程上下文（寄存器列表）
    _STRUCT_X86_FLOAT_STATE64       __fs; // 浮点寄存器
};

#define    _STRUCT_X86_THREAD_STATE64    struct __darwin_x86_thread_state64
_STRUCT_X86_THREAD_STATE64
{
    __uint64_t    __rax;
    __uint64_t    __rbx;
    __uint64_t    __rcx;
    __uint64_t    __rdx;
    __uint64_t    __rdi;
    __uint64_t    __rsi;
    __uint64_t    __rbp;
    __uint64_t    __rsp;
    __uint64_t    __r8;
    __uint64_t    __r9;
    __uint64_t    __r10;
    __uint64_t    __r11;
    __uint64_t    __r12;
    __uint64_t    __r13;
    __uint64_t    __r14;
    __uint64_t    __r15;
    __uint64_t    __rip;
    __uint64_t    __rflags;
    __uint64_t    __cs;
    __uint64_t    __fs;
    __uint64_t    __gs;
};

#define    _STRUCT_X86_FLOAT_STATE64    struct __darwin_x86_float_state64
_STRUCT_X86_FLOAT_STATE64
{
    int             __fpu_reserved[2];
    _STRUCT_FP_CONTROL    __fpu_fcw;        /* x87 FPU control word */
    _STRUCT_FP_STATUS    __fpu_fsw;        /* x87 FPU status word */
    __uint8_t        __fpu_ftw;        /* x87 FPU tag word */
    __uint8_t        __fpu_rsrv1;        /* reserved */ 
    __uint16_t        __fpu_fop;        /* x87 FPU Opcode */

    /* x87 FPU Instruction Pointer */
    __uint32_t        __fpu_ip;        /* offset */
    __uint16_t        __fpu_cs;        /* Selector */

    __uint16_t        __fpu_rsrv2;        /* reserved */

    /* x87 FPU Instruction Operand(Data) Pointer */
    __uint32_t        __fpu_dp;        /* offset */
    __uint16_t        __fpu_ds;        /* Selector */

    ...
};
```

&emsp;下面我们继续看 `_bs_backtraceOfThread` 函数的内容。

```c++
NSString *_bs_backtraceOfThread(thread_t thread) {
    // 默认栈深度是 50
    uintptr_t backtraceBuffer[50];
    
    int i = 0;
    
    // 调用栈回溯字符串变量 resultString，默认开头都是 thread ID（port）
    NSMutableString *resultString = [[NSMutableString alloc] initWithFormat:@"Backtrace of Thread %u:\n", thread];
    
    // 针对不同平台的机器声明一个用来存储线程上下文的变量
    _STRUCT_MCONTEXT machineContext;
    
    // 1⃣️ 获取指定线程的上下文信息，如果获取失败的话直接返回错误描述
    if(!bs_fillThreadStateIntoMachineContext(thread, &machineContext)) {
        return [NSString stringWithFormat:@"Fail to get information about thread: %u", thread];
    }
    
    // 2⃣️ 获取 __rip 寄存器的值（对应 ARM 架构下 PC 寄存器的值）
    const uintptr_t instructionAddress = bs_mach_instructionAddress(&machineContext);
    
    // 把 PC 寄存器的值记录在回溯缓冲数组中...
    backtraceBuffer[i] = instructionAddress;
    ++i;
    
    // FP(x29) 栈底 SP 栈顶 PC 下一条指令 LR(x30) 函数返回后的下一个函数的第一条指令
    // x29(FP) 栈底寄存器 SP 栈顶寄存器 LR（x30）是当前函数返回后，下一个函数的第一条指令 PC 下一条指令
    
    // 3⃣️ 读取 LR 寄存器的值，只有 ARM 平台有，x86 平台返回 0
    uintptr_t linkRegister = bs_mach_linkRegister(&machineContext);
    if (linkRegister) {
        // 把 LR 寄存器的值记录在回溯缓冲数组中...
        backtraceBuffer[i] = linkRegister;
        i++;
    }
    
    // 如果 instructionAddress 为 0 的话，即获取 PC 寄存器的值为 0，则返回一个错误字符串，感觉这个判断应该放在上面获取后直接判断吧，没必要读了 LR 寄存器再判断吧！
    if (instructionAddress == 0) {
        return @"Fail to get instruction address";
    }
    
    // 创建一个栈帧节点
    BSStackFrameEntry frame = {0};
    
    // 4⃣️ 取得 FP 栈底寄存器的值
    const uintptr_t framePtr = bs_mach_framePointer(&machineContext);
    
    // 5⃣️ bs_mach_copyMem 函数内部对 vm_read_overwrite 函数进行封装。
    // 使用 vm_read_overwrite() 函数，从目标进程 "读取" 内存。
    // 注意，这个函数与 vm_read() 不同，应该并没有做实际的数据拷贝，而是将 [region.address ~ region.address + region.size] 范围对应的所有映射状态同步给了 [region_data ~ region_data + region.size]，对于 Resident 的部分，两个进程中不同的虚拟内存地址对应的应该是相同的物理内存地址。
    
    // 如果 framePtr 等于 0 或者以 framePtr 为起始地址，复制 sizeof(frame) 个长度的虚拟内存的数据到 frame 指针中去失败，则返回错误描述，
    // 这里 frame 变量是 struct BSStackFrameEntry 类型的结构体，它内部一个指针，一个无符号 long 变量，所以 sizeof(frame) 的值为 16，
    // 即这里的作用是把 FP 栈底寄存器的值和 SP 栈顶寄存器的值复制到 frame 中
    
    if (framePtr == 0 || bs_mach_copyMem((void *)framePtr, &frame, sizeof(frame)) != KERN_SUCCESS) {
        return @"Fail to get frame pointer";
    }
    
    // 循环 50 次，沿着栈底指针构建一个链表，链表的每个节点都是每个调用帧的栈底指针，即前一个函数帧的起始地址
    for (; i < 50; i++) {
        backtraceBuffer[i] = frame.return_address;
        
        // 直到 FP 为 0，前一个 FP 指向 0，内存读取失败，跳出循环
        if (backtraceBuffer[i] == 0 || frame.previous == 0 || bs_mach_copyMem(frame.previous, &frame, sizeof(frame)) != KERN_SUCCESS) {
            break;
        }
    }
    
    // 准备一个值 i 的 backtraceLength 长度的 Dl_info 数组
    int backtraceLength = i;
    Dl_info symbolicated[backtraceLength];
    
    // 7⃣️ 查找 backtraceBuffer 数组中地址对应的符号信息
    bs_symbolicate(backtraceBuffer, symbolicated, backtraceLength, 0);
    
    // 遍历调用栈回溯中的函数字符串拼接在 resultString 字符串中
    for (int i = 0; i < backtraceLength; ++i) {
    
        // 8⃣️ 根据 `BsBacktraceLogger               0x10fa4359c -[ViewController bar] + 12` 这个格式，把调用栈的回溯字符串拼接在一起
        [resultString appendFormat:@"%@", bs_logBacktraceEntry(i, backtraceBuffer[i], &symbolicated[i])];
    }
    
    [resultString appendFormat:@"\n"];
    
    return [resultString copy];
}
```

&emsp;下面我们详细一个一个看一下 `_bs_backtraceOfThread` 函数中涉及的其他函数。

## bs_fillThreadStateIntoMachineContext（HandleMachineContext）

&emsp;获取 thread 的状态赋值到 machineContext 参数（实际只给 `&machineContext->__ss` 赋值），bool 类型返回值表示是否获取成功/失败，`&machineContext->__ss` 结构体在上面也已经看过了，是记录寄存器列表的一个结构体。

```c++
bool bs_fillThreadStateIntoMachineContext(thread_t thread, _STRUCT_MCONTEXT *machineContext) {
    mach_msg_type_number_t state_count = BS_THREAD_STATE_COUNT;

    // #define BS_THREAD_STATE_COUNT x86_THREAD_STATE64_COUNT
    // typedef _STRUCT_X86_THREAD_STATE64 x86_thread_state64_t;
    // #define x86_THREAD_STATE64_COUNT ((mach_msg_type_number_t) ( sizeof (x86_thread_state64_t) / sizeof (int) ))

    // #define BS_THREAD_STATE x86_THREAD_STATE64
    // x86_THREAD_STATE64 值为 4
    // state_count 值为 42
    // 42 * 4 和 21 * 8 都等于 168 
    
    // 获取指定 thread 的上下文，并赋值在 &machineContext->__ss 参数中（寄存器列表）
    kern_return_t kr = thread_get_state(thread, BS_THREAD_STATE, (thread_state_t)&machineContext->__ss, &state_count);
    
    return (kr == KERN_SUCCESS);
}

// 获取栈底寄存器的值
uintptr_t bs_mach_framePointer(mcontext_t const machineContext){
    return machineContext->__ss.BS_FRAME_POINTER;
}

// 获取栈顶寄存器的值
uintptr_t bs_mach_stackPointer(mcontext_t const machineContext){
    return machineContext->__ss.BS_STACK_POINTER;
}

// 获取 x86 平台下 IP 寄存器的值，对应 ARM 架构下 PC 寄存器的值
uintptr_t bs_mach_instructionAddress(mcontext_t const machineContext){
    return machineContext->__ss.BS_INSTRUCTION_ADDRESS;
}

// 读取 LR 寄存器的值，LR 是当前函数结束后，下一个函数的第一条指令。x86 平台没有这个寄存器，只有 ARM 平台才有
uintptr_t bs_mach_linkRegister(mcontext_t const machineContext){
#if defined(__i386__) || defined(__x86_64__)
    return 0;
#else
    return machineContext->__ss.__lr;
#endif
}

// 复制当前 task 指定位置的指定长度的虚拟内存空间中的内容，主要用于复制寄存器空间中的值
kern_return_t bs_mach_copyMem(const void *const src, void *const dst, const size_t numBytes){
    vm_size_t bytesCopied = 0;
    return vm_read_overwrite(mach_task_self(), (vm_address_t)src, (vm_size_t)numBytes, (vm_address_t)dst, &bytesCopied);
}
```

&emsp;这部分的代码全和线程状态有关，读取线程寄存器的值。

## bs_symbolicate（Symbolicate）

&emsp;bs_symbolicate 符号化，把指定地址进行符号化，即找到指定地址所对应的符号信息。

&emsp;这里我们首先看一下它的参数：`const uintptr_t* const backtraceBuffer` 这个是栈底指针的数组，也是每个函数调用栈起始地址的指针，我们就是根据这个指针去 Image 中查找最接近的符号。`Dl_info* const symbolsBuffer` 是一个 Dl_info 数组（`Dl_info symbolicated[backtraceLength]`）：

```c++
/*
 * Structure filled in by dladdr().
 */
typedef struct dl_info {
        const char      *dli_fname;     /* Pathname of shared object */
        void            *dli_fbase;     /* Base address of shared object */
        const char      *dli_sname;     /* Name of nearest symbol */
        void            *dli_saddr;     /* Address of nearest symbol */
} Dl_info;
``` 

&emsp;Dl_info 是我们获取指定符号时会用到的结构体，然后 `const int numEntries` 是 `const uintptr_t* const backtraceBuffer` 数组的长度。`const int skippedEntries` 可直接忽略，它传递的是 0。

```c++
void bs_symbolicate(const uintptr_t* const backtraceBuffer,
                    Dl_info* const symbolsBuffer,
                    const int numEntries,
                    const int skippedEntries){
    int i = 0;
    
    // _bs_backtraceOfThread 函数内调用 bs_symbolicate 函数时，skippedEntries 传的到都是 0 可忽略
    if (!skippedEntries && i < numEntries) {
    
        // 查找指定地址 address 最接近的符号的信息
        bs_dladdr(backtraceBuffer[i], &symbolsBuffer[i]);
        
        i++;
    }
    
    // 然后遍历查找指定地址 address 最接近的符号的信息，
    for (; i < numEntries; i++) {
        // CALL_INSTRUCTION_FROM_RETURN_ADDRESS(backtraceBuffer[i]) 去掉地址最后两位的地址签名，并数组偏移 1 个元素
        bs_dladdr(CALL_INSTRUCTION_FROM_RETURN_ADDRESS(backtraceBuffer[i]), &symbolsBuffer[i]);
    }
}

// 查找指定地址 address 最接近的符号的信息
bool bs_dladdr(const uintptr_t address, Dl_info* const info) {
    info->dli_fname = NULL;
    info->dli_fbase = NULL;
    info->dli_sname = NULL;
    info->dli_saddr = NULL;
    
    // 判断一个指定地址是否在当前已经加载的某个 Image 中并返回该 Image 在 _dyld_image_count 数值中的索引，即取得指定地址在某个 image 中并返回此 image 的索引
    const uint32_t idx = bs_imageIndexContainingAddress(address);
    
    // 如果返回 UINT_MAX 表示在当前已经加载的 Image 镜像中找不到 address 地址
    if (idx == UINT_MAX) {
        return false;
    }
    
    // 取得此 Image 镜像的 header 地址
    const struct mach_header* header = _dyld_get_image_header(idx);
    // 取得此 Image 内存地址的 slide 值
    const uintptr_t imageVMAddrSlide = (uintptr_t)_dyld_get_image_vmaddr_slide(idx);
    // 取得此 Image 内存地址的基地址
    const uintptr_t addressWithSlide = address - imageVMAddrSlide;
    
    // 取得 Image 在当前可执行文件中的虚拟地址的基地址然后加上 Slide
    const uintptr_t segmentBase = bs_segmentBaseOfImageIndex(idx) + imageVMAddrSlide;
    
    if (segmentBase == 0) {
        return false;
    }
    
    // Image 的名字赋值给 dli_fname，实际的值是 Image 的完整路径
    info->dli_fname = _dyld_get_image_name(idx);
    
    // Base address of shared object
    info->dli_fbase = (void*)header;
    
    // Find symbol tables and get whichever symbol is closest to the address.
    // 查找符号表并获取最接近地址的符号
    
    // #define BS_NLIST struct nlist_64
    
    // 符号表中的每个元素正是这个 struct nlist_64/nlist 结构体
    const BS_NLIST* bestMatch = NULL;
    
    // 无符号 long 最大值
    uintptr_t bestDistance = ULONG_MAX;
    
    // 针对 64 位和非 64 位的可执行文件，内部的 +1 是跳过 __PAGEZERO 段
    uintptr_t cmdPtr = bs_firstCmdAfterHeader(header);
    
    if (cmdPtr == 0) {
        return false;
    }
    
    // 遍历 Image 的 Load Command
    for (uint32_t iCmd = 0; iCmd < header->ncmds; iCmd++) {
        const struct load_command* loadCmd = (struct load_command*)cmdPtr;
        
        // #define LC_SYMTAB 0x2 /* link-edit stab symbol table info */
        
        // 找到 LC_SYMTAB 段，
        if (loadCmd->cmd == LC_SYMTAB) {
            
// The symtab_command contains the offsets and sizes of the link-edit 4.3 BSD "stab" style symbol table information as described in the header files <nlist.h> and <stab.h>.
//            struct symtab_command {
//                uint32_t    cmd;        /* LC_SYMTAB */
//                uint32_t    cmdsize;    /* sizeof(struct symtab_command) */
//                uint32_t    symoff;        /* symbol table offset */
//                uint32_t    nsyms;        /* number of symbol table entries */
//                uint32_t    stroff;        /* string table offset */
//                uint32_t    strsize;    /* string table size in bytes */
//            };
            
            // 因为 loadCmd 是 LC_SYMTAB 类型，所以这里可直接把 cmdPtr 强制转换为 struct symtab_command * 指针
            const struct symtab_command* symtabCmd = (struct symtab_command*)cmdPtr;
           
            // 直接基地址 + symbol table 偏移，取得符号表的首地址，且符号表中正是 struct nlist/nlist_64 类型数组，所以这里直接强转为 BS_NLIST 指针
            const BS_NLIST* symbolTable = (BS_NLIST*)(segmentBase + symtabCmd->symoff);
            
            // 然后直接基地址 + string table 偏移，取得保存符号名字符串的表的起始地址
            const uintptr_t stringTable = segmentBase + symtabCmd->stroff;
            
            // 然后对符号表中的符号进行遍历，找到最接近 address 的符号，保存在 bestMatch 变量中，
            for (uint32_t iSym = 0; iSym < symtabCmd->nsyms; iSym++) {
                
//                This is the symbol table entry structure for 64-bit architectures. 这是 64 位体系结构的符号表条目结构。
//                struct nlist_64 {
//                    union {
//                        uint32_t  n_strx; /* index into the string table */
//                    } n_un;
//                    uint8_t n_type;        /* type flag, see below */
//                    uint8_t n_sect;        /* section number or NO_SECT */
//                    uint16_t n_desc;       /* see <mach-o/stab.h> */
//                    uint64_t n_value;      /* value of this symbol (or stab offset) */
//                };
                
                // If n_value is 0, the symbol refers to an external object.
                // 如果 n_value 为 0，则该符号引用外部对象
                
                if (symbolTable[iSym].n_value != 0) {
                    // 取得当前符号的地址
                    uintptr_t symbolBase = symbolTable[iSym].n_value;
                    
                    // 这里没太理解，用 addressWithSlide 减去 symbolBase，理论上 addressWithSlide 的值应该会小于 symbolBase，硬减的话会得到一个负值，然后因为 currentDistance 是一个无符号 long，
                    // 所以这里 currentDistance 的值是减法溢出后转换为无符号 long，
                    // 这里遍历符号，每次记录当前符号和指定地址的距离，记录下来
                    uintptr_t currentDistance = addressWithSlide - symbolBase;
                    
                    // 然后记录到一个最接近指定地址的符号（每遇到一个最近的距离值就更新一下 bestDistance 的值）
                    if ((addressWithSlide >= symbolBase) && (currentDistance <= bestDistance)) {
                        bestMatch = symbolTable + iSym;
                        bestDistance = currentDistance;
                    }
                }
            }
            
            // 找到 bestMatch 时，记录下当前 Image 的：
            if (bestMatch != NULL) {
                // dli_saddr 最近符号的地址
                info->dli_saddr = (void*)(bestMatch->n_value + imageVMAddrSlide);
                // dli_sname 最近符号的名称
                info->dli_sname = (char*)((intptr_t)stringTable + (intptr_t)bestMatch->n_un.n_strx);
                
                if (*info->dli_sname == '_') {
                    info->dli_sname++;
                }
                
                // This happens if all symbols have been stripped.
                // 如果所有符号都已被剥离，则会发生这种情况。
                if(info->dli_saddr == info->dli_fbase && bestMatch->n_type == 3) {
                    info->dli_sname = NULL;
                }
                
                break;
            }
        }
        
        // 偏移到下一个 Load Command
        cmdPtr += loadCmd->cmdsize;
    }
    
    return true;
}

/// 针对 64 位和非 64 位的可执行文件，这里的 +1 是跳过 __PAGEZERO 段
/// @param header Image header
uintptr_t bs_firstCmdAfterHeader(const struct mach_header* const header) {
    switch(header->magic) {
        case MH_MAGIC:
        case MH_CIGAM:
            return (uintptr_t)(header + 1);
        case MH_MAGIC_64:
        case MH_CIGAM_64:
            return (uintptr_t)(((struct mach_header_64*)header) + 1);
        default:
            return 0;  // Header is corrupt
    }
}

/// 判断一个指定地址是否在当前已经加载的某个 Image 中并返回该 Image 在 _dyld_image_count 数值中的索引，即取得指定地址在某个 image 中并返回此 image 的索引
/// @param address 指定地址
uint32_t bs_imageIndexContainingAddress(const uintptr_t address) {
    // 当前 dyld 加载的 Image 镜像的数量
    const uint32_t imageCount = _dyld_image_count();
    
    // image header 的指针
    const struct mach_header* header = 0;
    
    // 开始遍历这些 Image 镜像
    for(uint32_t iImg = 0; iImg < imageCount; iImg++) {
        // 取得当前这个 image 的 header 指针
        header = _dyld_get_image_header(iImg);
        
        if (header != NULL) {
            // Look for a segment command with this address within its range.
            
            // address 减去 image 的 slide 随机值，取得它的基地址
            uintptr_t addressWSlide = address - (uintptr_t)_dyld_get_image_vmaddr_slide(iImg);
            
            // 当前 image 的第一个段的地址（撇开 __PAGEZERO 段）
            uintptr_t cmdPtr = bs_firstCmdAfterHeader(header);
            
            if (cmdPtr == 0) {
                continue;
            }
            
            // 然后再开始遍历这个 Image 中的所有 Load Command
            for (uint32_t iCmd = 0; iCmd < header->ncmds; iCmd++) {
                // 强转为 struct load_command * 指针
                const struct load_command* loadCmd = (struct load_command*)cmdPtr;
                
                // 然后仅需要遍历 LC_SEGMENT/LC_SEGMENT_64 类型的段
                
                if (loadCmd->cmd == LC_SEGMENT) {
                    // 强转为 struct segment_command * 指针
                    const struct segment_command* segCmd = (struct segment_command*)cmdPtr;
                    
                    // 然后判断 addressWSlide 是否在这个段的虚拟地址的范围内
                    if (addressWSlide >= segCmd->vmaddr && addressWSlide < segCmd->vmaddr + segCmd->vmsize) {
                        // 如果在的话直接返回此 Image 的索引
                        return iImg;
                    }
                } else if (loadCmd->cmd == LC_SEGMENT_64) {
                    // 强转为 struct segment_command_64 * 指针
                    const struct segment_command_64* segCmd = (struct segment_command_64*)cmdPtr;
                    
                    // 然后判断 addressWSlide 是否在这个段的虚拟地址的范围内
                    if (addressWSlide >= segCmd->vmaddr && addressWSlide < segCmd->vmaddr + segCmd->vmsize) {
                        
                        // 如果在的话直接返回此 Image 的索引
                        return iImg;
                    }
                }
                
                // 偏移当前 cmd 的宽度，到下一个 Load Command
                cmdPtr += loadCmd->cmdsize;
            }
        }
    }
    
    // 如果未找到的话，就返回无符号 Int 的最大值
    return UINT_MAX;
}

/// 取得指定索引的 Image 的 __LINKEDIT 段的虚拟地址减去 fileoff（file offset of this segment），得出此 Image 的虚拟基地址，
/// 这里为什么一定要用 __LINKEDIT 段没看明白，我使用 MachOView 查看了一下可执行文件，如下，看到使用其它几个段的 VM Address 减去 File Offset 得到的值是一样的，都是 4294967296
///
/// __TEXT: VM Address: 4294967296，File Offset: 0
/// __DATA_CONST: VM Address: 4295000064，File Offset: 32768 => 4295000064 - 32768 = 4294967296
/// __DATA: VM Address: 4295016448，File Offset: 49152 => 4295016448 - 49152 = 4294967296
/// __LINKEDIT: VM Address: 4295032832，File Offset: 65536 => 4295032832 - 65536 = 4294967296
///
/// @param idx image 索引
uintptr_t bs_segmentBaseOfImageIndex(const uint32_t idx) {
    const struct mach_header* header = _dyld_get_image_header(idx);
    
    // Look for a segment command and return the file image address.
    
    // typedef unsigned long uintptr_t;
    // 取得 image 的第一个段(撇掉 __PAGEZERO 段)的地址，并把 struct mach_header_64 * 指针强转为了 uintptr_t（无符号 long）
    uintptr_t cmdPtr = bs_firstCmdAfterHeader(header);
    
    if (cmdPtr == 0) {
        return 0;
    }
    
    // 遍历所有的 Load Command
    for (uint32_t i = 0;i < header->ncmds; i++) {
        
        const struct load_command* loadCmd = (struct load_command*)cmdPtr;
        
        // 仅排查类型是 LC_SEGMENT 和 LC_SEGMENT_64 类型的 Load Command，并找到 __LINKEDIT 名字的段，计算出虚拟基地址并返回
        if (loadCmd->cmd == LC_SEGMENT) {
            // #define SEG_LINKEDIT "__LINKEDIT"
            // 取得段名是 __LINKEDIT 的段的虚拟基地址
            
            // 把地址强转为 struct segment_command * 指针
            const struct segment_command* segmentCmd = (struct segment_command*)cmdPtr;
            
            // the segment containing all structs created and maintained by the link editor.
            // Created with -seglinkedit option to ld(1) for MH_EXECUTE and FVMLIB file types only
            // #define SEG_LINKEDIT "__LINKEDIT"
            
            if (strcmp(segmentCmd->segname, SEG_LINKEDIT) == 0) {
                return segmentCmd->vmaddr - segmentCmd->fileoff;
            }
        } else if(loadCmd->cmd == LC_SEGMENT_64) {
            // 64 位的情况
            const struct segment_command_64* segmentCmd = (struct segment_command_64*)cmdPtr;
            
            // 取得段名是 __LINKEDIT 的段的虚拟基地址
            if (strcmp(segmentCmd->segname, SEG_LINKEDIT) == 0) {
                return (uintptr_t)(segmentCmd->vmaddr - segmentCmd->fileoff);
            }
        }
        
        // 根据当前段的大小宽度：cmdsize 偏移到下一个段
        cmdPtr += loadCmd->cmdsize;
    }
    
    // 未找到的话就返回 0
    return 0;
}
```

## bs_logBacktraceEntry（GenerateBacbsrackEnrty）

&emsp;传入函数地址和此地址对应的符号信息，然后生成对应的 Image 和 函数名字的字符串，类似这种格式：`BsBacktraceLogger               0x10fa4359c -[ViewController bar] + 12`

```c++
NSString* bs_logBacktraceEntry(const int entryNum,
                               const uintptr_t address,
                               const Dl_info* const dlInfo) {
    char faddrBuff[20];
    char saddrBuff[20];
    
    // 首先取得 Image 的名字字符串，例如：BsBacktraceLogger、UIKitCore、CoreFoundation 等等
    const char* fname = bs_lastPathEntry(dlInfo->dli_fname);
    
    // 如果 fname 为 NULL，则把 dlInfo->dli_fbase 作为 fname 使用
    if (fname == NULL) {
        // #define POINTER_FMT "0x%016lx"
        // 按指针格式把 (uintptr_t)dlInfo->dli_fbase 写入 faddrBuff 中
        sprintf(faddrBuff, POINTER_FMT, (uintptr_t)dlInfo->dli_fbase);
        
        // 把地址作为 fname 的名字
        fname = faddrBuff;
    }
    
    // address 减去最近符号的地址，即得到 address 对应符号的偏移地址
    uintptr_t offset = address - (uintptr_t)dlInfo->dli_saddr;
    // dlInfo->dli_sname 取得最近符号的名称
    const char* sname = dlInfo->dli_sname;
    
    // 如果符号名字为 NULL 的话，则同样把 dlInfo->dli_fbase 作为 sname 使用
    if (sname == NULL) {
        // #define POINTER_SHORT_FMT "0x%lx"
        // 这里使用的是 0x%lx 格式
        sprintf(saddrBuff, POINTER_SHORT_FMT, (uintptr_t)dlInfo->dli_fbase);
        
        sname = saddrBuff;
        offset = address - (uintptr_t)dlInfo->dli_fbase;
    }
    
    // 组装栈帧字符串，例如：UIKitCore                       0x7fff2489d02c -[UIViewController view] + 27 这样的格式
    // 首先是 Image 的名字，然后是符号的地址，然后是函数符号的名字，最后是函数符号的偏移长度也就是函数内容的宽度吧好像
    return [NSString stringWithFormat:@"%-30s  0x%08" PRIxPTR " %s + %lu\n", fname, (uintptr_t)address, sname, offset];
}

// 取得指定 path 的最后一段，
// 例如：path 是："xxx/Application/E3358B42-7325-4EA4-BD81-2210A0F4AC8F/BsBacktraceLogger.app/BsBacktraceLogger"
// 然后返回 BsBacktraceLogger
const char* bs_lastPathEntry(const char* const path) {
    if (path == NULL) {
        return NULL;
    }
    
    // char *strrchr(const char *__s, int __c);
    // 查找一个字符 __c 在另一个字符串 __s 中末次出现的位置（也就是从 __s 的右侧开始查找字符 __c 首次出现的位置），并返回这个位置的地址。
    // 如果未能找到指定字符，那么函数将返回 NULL。使用这个地址返回从最后一个字符 __c 到 __s 末尾的字符串。
    char* lastFile = strrchr(path, '/');
    
//    printf("🐶🐶🐶 %s \n", lastFile);
    
    // +1 是去掉 lastFile 最前面的 / 符号
    return lastFile == NULL ? path : lastFile + 1;
}
```

&emsp;至此 BSBacktraceLogger 的源码我们就全部看完了，注释超详细，这里就不再总结了！

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

