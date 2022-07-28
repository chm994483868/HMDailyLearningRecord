# 函数调用堆栈

&emsp;在学习线程的相关知识时，我们一定接触过 NSThread 类，它有一个类属性：`@property (class, readonly, copy) NSArray<NSString *> *callStackSymbols` 用来获取当前线程的函数调用堆栈，该属性返回包含调用堆栈符号的数组，每个元素都是一个 NSString 对象，其值的格式由 backtrace_symbols() 函数确定。初看此属性给人眼前一亮，但是看到它仅是一个类属性时，我们仿佛意识到了什么，对，它有一个限制，此类属性返回值仅能描述调用此方法时当前线程的调用堆栈回溯。我们常见的在一个子线程中抓取主线程的函数调用堆栈的场景它就无法胜任了。例如：进行性能监控时通常会开一个子线程监控，如监控主线程的卡顿情况、CPU Usage 情况，当出现主线程卡顿、某个线程 CPU Usage 占用过高时，需要进行抓栈，那么就需要通过其它方式获取非当前线程的调用栈了。

> &emsp;在 Developer Documentation 中搜索 callStackSymbols 时，发现 NSException 类中也有一个同名的实例属性：`@property(readonly, copy) NSArray<NSString *> *callStackSymbols;` 此实例属性描述首次引发异常时调用堆栈回溯的字符串数组。每个字符串的格式由 backtrace_symbols() 确定，感兴趣的小伙伴可以尝试在一个 Try-Catch 中制造一个 Objective-C 异常，打印一下 callStackSymbols 属性的内容试一下。

&emsp;继续开始之前我们需要补充一些 macOS 三种线程的关系：

1. `pthread_t pthread_self(void)` 返回的是 `pthread_t`，glibc 库的线程 id。实际上是线程控制块 tcb 首地址。(pthread_self 是 POSIX 标准中的接口) 
2. `syscall(SYS_gettid)` 内核级线程 id，系统唯一。该函数为系统调用函数，glibc 可能没有该函数声明，此时需要使用 syscall(SYS_gettid)。可以参考 [pthread_self() VS syscall(SYS_gettid)](https://www.cnblogs.com/alix-1988/p/14886847.html) 详细了解它们之间的关系。（此种线程不涉及本篇的知识点，可直接忽略）
3. `mach_port_t mach_thread_self(void)` 直接获取 mach_port_t 和 `mach_port_t pthread_mach_thread_np(pthread_t)`、`pthread_t pthread_self(void)` 搭配间接获取 mach_port_t。mach_port_t 是 macOS 特有的 id，实际上不能说是 thread id，而应该当做是端口。
  
&emsp;获取 Thread ID 的方式，看到这个链接 [一个“反问”引发的内存反思](https://blog.csdn.net/killer1989/article/details/106674973) 我们可以详细阅读一下，增强一下对 `extern mach_port_t mach_thread_self(void);` 和 `mach_port_t pthread_mach_thread_np(pthread_t);` 函数的认识。

```c++
/* return the mach thread bound to the pthread */
__API_AVAILABLE(macos(10.4), ios(2.0))
mach_port_t pthread_mach_thread_np(pthread_t);
```

&emsp;针对上述优化点，我们大概可以这样通过以下方式获取 pthread_t 的线程 ID：

```c++
mach_port_t thread_self(void) {
//    mach_port_t thread_self = mach_thread_self();
//    mach_port_deallocate(mach_task_self(), thread_self);
//    return thread_self;
    
    return pthread_mach_thread_np(pthread_self());
}
```

&emsp;FP(x29) 栈底 SP 栈顶 PC 下一条指令 LR(x30) 函数返回后的下一个函数的第一条指令

@emsp;NSException:

```c++
NSException

Instance Property

callStackSymbols

An array containing the current call stack symbols.

@property(readonly, copy) NSArray<NSString *> *callStackSymbols;

Discussion
An array of strings describing the call stack backtrace at the moment the exception was first raised. The format of each string is determined by the backtrace_symbols() API
```

```c++
NSThread

Type Property

callStackSymbols

Returns an array containing the call stack symbols.

@property(class, readonly, copy) NSArray<NSString *> *callStackSymbols;

Return Value
An array containing the call stack symbols. Each element is an NSString object with a value in a format determined by the backtrace_symbols() function. For more information, see backtrace_symbols(3) macOS Developer Tools Manual Page.

Discussion
The return value describes the call stack backtrace of the current thread at the moment this method was called.
```

## 参考链接
**参考链接:🔗**
+ [iOS中符号的那些事儿](https://juejin.cn/post/6844904164208689166)
+ [获取任意线程的调用栈](https://zhuanlan.zhihu.com/p/357039492)
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

//
//  BSBacktraceLogger.m
//  BSBacktraceLogger
//
//  Created by 张星宇 on 16/8/27.
//  Copyright © 2016年 bestswifter. All rights reserved.
//

#import "BSBacktraceLogger.h"
#import <mach/mach.h>
#include <dlfcn.h>
#include <pthread.h>
#include <sys/types.h>
#include <limits.h>
#include <string.h>
#include <mach-o/dyld.h>
#include <mach-o/nlist.h>

#pragma -mark DEFINE MACRO FOR DIFFERENT CPU ARCHITECTURE
#if defined(__arm64__)
#define DETAG_INSTRUCTION_ADDRESS(A) ((A) & ~(3UL))
#define BS_THREAD_STATE_COUNT ARM_THREAD_STATE64_COUNT
#define BS_THREAD_STATE ARM_THREAD_STATE64
#define BS_FRAME_POINTER __fp
#define BS_STACK_POINTER __sp
#define BS_INSTRUCTION_ADDRESS __pc

#elif defined(__arm__)
#define DETAG_INSTRUCTION_ADDRESS(A) ((A) & ~(1UL))
#define BS_THREAD_STATE_COUNT ARM_THREAD_STATE_COUNT
#define BS_THREAD_STATE ARM_THREAD_STATE
#define BS_FRAME_POINTER __r[7]
#define BS_STACK_POINTER __sp
#define BS_INSTRUCTION_ADDRESS __pc

#elif defined(__x86_64__)
#define DETAG_INSTRUCTION_ADDRESS(A) (A)
#define BS_THREAD_STATE_COUNT x86_THREAD_STATE64_COUNT
#define BS_THREAD_STATE x86_THREAD_STATE64
#define BS_FRAME_POINTER __rbp
#define BS_STACK_POINTER __rsp
#define BS_INSTRUCTION_ADDRESS __rip

#elif defined(__i386__)
#define DETAG_INSTRUCTION_ADDRESS(A) (A)
#define BS_THREAD_STATE_COUNT x86_THREAD_STATE32_COUNT
#define BS_THREAD_STATE x86_THREAD_STATE32
#define BS_FRAME_POINTER __ebp
#define BS_STACK_POINTER __esp
#define BS_INSTRUCTION_ADDRESS __eip

#endif

#define CALL_INSTRUCTION_FROM_RETURN_ADDRESS(A) (DETAG_INSTRUCTION_ADDRESS((A)) - 1)

#if defined(__LP64__)
#define TRACE_FMT         "%-4d%-31s 0x%016lx %s + %lu"
#define POINTER_FMT       "0x%016lx"
#define POINTER_SHORT_FMT "0x%lx"
#define BS_NLIST struct nlist_64
#else
#define TRACE_FMT         "%-4d%-31s 0x%08lx %s + %lu"
#define POINTER_FMT       "0x%08lx"
#define POINTER_SHORT_FMT "0x%lx"
#define BS_NLIST struct nlist
#endif

typedef struct BSStackFrameEntry{
    const struct BSStackFrameEntry *const previous;
    const uintptr_t return_address;
} BSStackFrameEntry;

static mach_port_t main_thread_id;

@implementation BSBacktraceLogger

+ (void)load {
    
    // main_thread_id 是在 load 方法中赋值，确保一定是在主线程
    main_thread_id = mach_thread_self();
    
    // ⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️
    // 下面是一些测试的代码，不用管它们
    pthread_t thread = pthread_self();
    mach_port_t temp = pthread_mach_thread_np(pthread_self());
    
    // returns non-zero if the current thread is the main thread
    // 当前是主线程时返回 1
    int np = pthread_main_np();
    
//    int t1 = syscall(sel_getUid);
    
    NSLog(@"123");
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        mach_port_t tt1 = mach_thread_self();
        pthread_t thread1 = pthread_self();
        mach_port_t temp1 = pthread_mach_thread_np(pthread_self());
        
        // returns non-zero if the current thread is the main thread
        // 当前不是主线程，所以返回 0
        int np1 = pthread_main_np();
        
        NSLog(@"123");
    });
    // ⬆️⬆️⬆️⬆️⬆️⬆️⬆️⬆️
}

/// mach_thread_self 函数的优化版本 [一个“反问”引发的内存反思](https://blog.csdn.net/killer1989/article/details/106674973)
mach_port_t thread_self(void) {
//    mach_port_t thread_self = mach_thread_self();
//    mach_port_deallocate(mach_task_self(), thread_self);
//    return thread_self;
    
    return pthread_mach_thread_np(pthread_self());
}

#pragma -mark Implementation of interface
+ (NSString *)bs_backtraceOfNSThread:(NSThread *)thread {
    
    // 把 thread 转换为 thread_t 类型的 Mach 线程
    return _bs_backtraceOfThread(bs_machThreadFromNSThread(thread));
}

+ (NSString *)bs_backtraceOfCurrentThread {
    
    // 对 [NSThread currentThread] 当前线程进行调用栈回溯
    return [self bs_backtraceOfNSThread:[NSThread currentThread]];
}

+ (NSString *)bs_backtraceOfMainThread {
    
    // 对 [NSThread mainThread] 主线程进行调用栈回溯
    return [self bs_backtraceOfNSThread:[NSThread mainThread]];
}

/// 对当前所有线程进行栈回溯
+ (NSString *)bs_backtraceOfAllThread {
    // 遍历对当前所有线程进行进行栈回溯
    thread_act_array_t threads;
    mach_msg_type_number_t thread_count = 0;
    const task_t this_task = mach_task_self();
    
    kern_return_t kr = task_threads(this_task, &threads, &thread_count);
    if(kr != KERN_SUCCESS) {
        return @"Fail to get information of all threads";
    }
    
    NSMutableString *resultString = [NSMutableString stringWithFormat:@"Call Backtrace of %u threads:\n", thread_count];
    for(int i = 0; i < thread_count; i++) {
        [resultString appendString:_bs_backtraceOfThread(threads[i])];
    }
    return [resultString copy];
}

#pragma -mark Get call backtrace of a mach_thread

/// 获取 Mach 线程（thread_t）的所有栈回溯
/// @param thread Mach 线程 thread_t
NSString *_bs_backtraceOfThread(thread_t thread) {
    // 默认栈深度是 50
    uintptr_t backtraceBuffer[50];
    
    int i = 0;
    
    // 栈回溯字符串，默认开头字符串都是 thread ID
    NSMutableString *resultString = [[NSMutableString alloc] initWithFormat:@"Backtrace of Thread %u:\n", thread];
    
    _STRUCT_MCONTEXT machineContext;
    if(!bs_fillThreadStateIntoMachineContext(thread, &machineContext)) {
        return [NSString stringWithFormat:@"Fail to get information about thread: %u", thread];
    }
    
    // __rip 的值
    const uintptr_t instructionAddress = bs_mach_instructionAddress(&machineContext);
    backtraceBuffer[i] = instructionAddress;
    ++i;
    
    // FP(x29) 栈底 SP 栈顶 PC 下一条指令 LR(x30) 函数返回后的下一个函数的第一条指令
    // x29(FP) 栈底寄存器 SP 栈顶寄存器 LR（x30）是当前函数返回后，下一个函数的第一条指令 PC 下一条指令
    
    // LR 寄存器的值
    uintptr_t linkRegister = bs_mach_linkRegister(&machineContext);
    if (linkRegister) {
        backtraceBuffer[i] = linkRegister;
        i++;
    }
    
    // instructionAddress 为 0 的话，返回一个错误字符串，感觉这个判断应该放在上面获取后直接判断吧，没必要读了 LR 寄存器再判断！
    if (instructionAddress == 0) {
        return @"Fail to get instruction address";
    }
    
    BSStackFrameEntry frame = {0};
    // FP 栈底寄存器的值
    const uintptr_t framePtr = bs_mach_framePointer(&machineContext);
    
    // 使用 vm_read_overwrite() 函数，从目标进程 "读取" 内存。注意，这个函数与 vm_read() 不同，应该并没有做实际的数据拷贝，而是将 region.address ~ region.address + region.size 范围对应的所有映射状态同步给了 region_data ~ region_data + region.size，对于 Resident 的部分，两个进程中不同的虚拟内存地址对应的应该是相同的物理内存地址。
    
    // 如果 framePtr 等于 0 或者以 framePtr 为起始地址，复制 sizeof(frame) 个长度的虚拟内存的数据到 frame 指针中去失败，则返回错误描述
    if (framePtr == 0 ||
       bs_mach_copyMem((void *)framePtr, &frame, sizeof(frame)) != KERN_SUCCESS) {
        return @"Fail to get frame pointer";
    }
    
    // 循环 50 次，沿着栈底指针构建一个链表，链表的每个节点都是每个函数帧的栈底指针，即前一个函数帧的起始地址
    for(; i < 50; i++) {
        backtraceBuffer[i] = frame.return_address;
        
        if(backtraceBuffer[i] == 0 ||
           frame.previous == 0 ||
           bs_mach_copyMem(frame.previous, &frame, sizeof(frame)) != KERN_SUCCESS) {
            break;
        }
    }
    
    int backtraceLength = i;
    Dl_info symbolicated[backtraceLength];
    bs_symbolicate(backtraceBuffer, symbolicated, backtraceLength, 0);
    
    for (int i = 0; i < backtraceLength; ++i) {
        [resultString appendFormat:@"%@", bs_logBacktraceEntry(i, backtraceBuffer[i], &symbolicated[i])];
    }
    
    [resultString appendFormat:@"\n"];
    return [resultString copy];
}

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
    
    // 调用 task_threads 函数根据当前的 task 来获取所有线程（线程端口）
    
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
    
    // 如果 nsthread 是主线程的话直接返回，在 +load 函数中获取的主线程的 mach_port_t
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

#pragma -mark GenerateBacbsrackEnrty
NSString* bs_logBacktraceEntry(const int entryNum,
                               const uintptr_t address,
                               const Dl_info* const dlInfo) {
    char faddrBuff[20];
    char saddrBuff[20];
    
    const char* fname = bs_lastPathEntry(dlInfo->dli_fname);
    if(fname == NULL) {
        sprintf(faddrBuff, POINTER_FMT, (uintptr_t)dlInfo->dli_fbase);
        fname = faddrBuff;
    }
    
    uintptr_t offset = address - (uintptr_t)dlInfo->dli_saddr;
    const char* sname = dlInfo->dli_sname;
    if(sname == NULL) {
        sprintf(saddrBuff, POINTER_SHORT_FMT, (uintptr_t)dlInfo->dli_fbase);
        sname = saddrBuff;
        offset = address - (uintptr_t)dlInfo->dli_fbase;
    }
    return [NSString stringWithFormat:@"%-30s  0x%08" PRIxPTR " %s + %lu\n" ,fname, (uintptr_t)address, sname, offset];
}

const char* bs_lastPathEntry(const char* const path) {
    if(path == NULL) {
        return NULL;
    }
    
    char* lastFile = strrchr(path, '/');
    return lastFile == NULL ? path : lastFile + 1;
}

#pragma -mark HandleMachineContext
bool bs_fillThreadStateIntoMachineContext(thread_t thread, _STRUCT_MCONTEXT *machineContext) {
    mach_msg_type_number_t state_count = BS_THREAD_STATE_COUNT;
    
    // 获取 thread 的状态
    kern_return_t kr = thread_get_state(thread, BS_THREAD_STATE, (thread_state_t)&machineContext->__ss, &state_count);
    
    return (kr == KERN_SUCCESS);
}

uintptr_t bs_mach_framePointer(mcontext_t const machineContext){
    return machineContext->__ss.BS_FRAME_POINTER;
}

uintptr_t bs_mach_stackPointer(mcontext_t const machineContext){
    return machineContext->__ss.BS_STACK_POINTER;
}

uintptr_t bs_mach_instructionAddress(mcontext_t const machineContext){
    return machineContext->__ss.BS_INSTRUCTION_ADDRESS;
}

uintptr_t bs_mach_linkRegister(mcontext_t const machineContext){
#if defined(__i386__) || defined(__x86_64__)
    return 0;
#else
    return machineContext->__ss.__lr;
#endif
}

kern_return_t bs_mach_copyMem(const void *const src, void *const dst, const size_t numBytes){
    vm_size_t bytesCopied = 0;
    return vm_read_overwrite(mach_task_self(), (vm_address_t)src, (vm_size_t)numBytes, (vm_address_t)dst, &bytesCopied);
}

#pragma -mark Symbolicate
void bs_symbolicate(const uintptr_t* const backtraceBuffer,
                    Dl_info* const symbolsBuffer,
                    const int numEntries,
                    const int skippedEntries){
    int i = 0;
    
    if(!skippedEntries && i < numEntries) {
        bs_dladdr(backtraceBuffer[i], &symbolsBuffer[i]);
        i++;
    }
    
    for(; i < numEntries; i++) {
        bs_dladdr(CALL_INSTRUCTION_FROM_RETURN_ADDRESS(backtraceBuffer[i]), &symbolsBuffer[i]);
    }
}

bool bs_dladdr(const uintptr_t address, Dl_info* const info) {
    info->dli_fname = NULL;
    info->dli_fbase = NULL;
    info->dli_sname = NULL;
    info->dli_saddr = NULL;
    
    const uint32_t idx = bs_imageIndexContainingAddress(address);
    if(idx == UINT_MAX) {
        return false;
    }
    const struct mach_header* header = _dyld_get_image_header(idx);
    const uintptr_t imageVMAddrSlide = (uintptr_t)_dyld_get_image_vmaddr_slide(idx);
    const uintptr_t addressWithSlide = address - imageVMAddrSlide;
    const uintptr_t segmentBase = bs_segmentBaseOfImageIndex(idx) + imageVMAddrSlide;
    if(segmentBase == 0) {
        return false;
    }
    
    info->dli_fname = _dyld_get_image_name(idx);
    info->dli_fbase = (void*)header;
    
    // Find symbol tables and get whichever symbol is closest to the address.
    const BS_NLIST* bestMatch = NULL;
    uintptr_t bestDistance = ULONG_MAX;
    uintptr_t cmdPtr = bs_firstCmdAfterHeader(header);
    if(cmdPtr == 0) {
        return false;
    }
    for(uint32_t iCmd = 0; iCmd < header->ncmds; iCmd++) {
        const struct load_command* loadCmd = (struct load_command*)cmdPtr;
        if(loadCmd->cmd == LC_SYMTAB) {
            const struct symtab_command* symtabCmd = (struct symtab_command*)cmdPtr;
            const BS_NLIST* symbolTable = (BS_NLIST*)(segmentBase + symtabCmd->symoff);
            const uintptr_t stringTable = segmentBase + symtabCmd->stroff;
            
            for(uint32_t iSym = 0; iSym < symtabCmd->nsyms; iSym++) {
                // If n_value is 0, the symbol refers to an external object.
                if(symbolTable[iSym].n_value != 0) {
                    uintptr_t symbolBase = symbolTable[iSym].n_value;
                    uintptr_t currentDistance = addressWithSlide - symbolBase;
                    if((addressWithSlide >= symbolBase) &&
                       (currentDistance <= bestDistance)) {
                        bestMatch = symbolTable + iSym;
                        bestDistance = currentDistance;
                    }
                }
            }
            if(bestMatch != NULL) {
                info->dli_saddr = (void*)(bestMatch->n_value + imageVMAddrSlide);
                info->dli_sname = (char*)((intptr_t)stringTable + (intptr_t)bestMatch->n_un.n_strx);
                if(*info->dli_sname == '_') {
                    info->dli_sname++;
                }
                // This happens if all symbols have been stripped.
                if(info->dli_saddr == info->dli_fbase && bestMatch->n_type == 3) {
                    info->dli_sname = NULL;
                }
                break;
            }
        }
        cmdPtr += loadCmd->cmdsize;
    }
    return true;
}

/// 这里的 +1 是跳过 __PAGEZERO 段
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

/// 判断一个指定地址是否在 Image 中
/// @param address 指定地址
uint32_t bs_imageIndexContainingAddress(const uintptr_t address) {
    // 当前 dyld 加载的 Image 镜像的数量
    const uint32_t imageCount = _dyld_image_count();
    
    // image header 的指针
    const struct mach_header* header = 0;
    
    // 开始遍历这些 Image 镜像
    for(uint32_t iImg = 0; iImg < imageCount; iImg++) {
        //
        header = _dyld_get_image_header(iImg);
        
        if(header != NULL) {
            // Look for a segment command with this address within its range.
            uintptr_t addressWSlide = address - (uintptr_t)_dyld_get_image_vmaddr_slide(iImg);
            uintptr_t cmdPtr = bs_firstCmdAfterHeader(header);
            if(cmdPtr == 0) {
                continue;
            }
            for(uint32_t iCmd = 0; iCmd < header->ncmds; iCmd++) {
                const struct load_command* loadCmd = (struct load_command*)cmdPtr;
                if(loadCmd->cmd == LC_SEGMENT) {
                    const struct segment_command* segCmd = (struct segment_command*)cmdPtr;
                    if(addressWSlide >= segCmd->vmaddr &&
                       addressWSlide < segCmd->vmaddr + segCmd->vmsize) {
                        return iImg;
                    }
                }
                else if(loadCmd->cmd == LC_SEGMENT_64) {
                    const struct segment_command_64* segCmd = (struct segment_command_64*)cmdPtr;
                    if(addressWSlide >= segCmd->vmaddr &&
                       addressWSlide < segCmd->vmaddr + segCmd->vmsize) {
                        return iImg;
                    }
                }
                cmdPtr += loadCmd->cmdsize;
            }
        }
    }
    return UINT_MAX;
}

/// 取得指定 Image 的 __LINKEDIT 段的虚拟基地址
/// @param idx image 索引
uintptr_t bs_segmentBaseOfImageIndex(const uint32_t idx) {
    const struct mach_header* header = _dyld_get_image_header(idx);
    
    // Look for a segment command and return the file image address.
    uintptr_t cmdPtr = bs_firstCmdAfterHeader(header);
    
    if(cmdPtr == 0) {
        return 0;
    }
    
    // 遍历所有的 Load Command
    for(uint32_t i = 0;i < header->ncmds; i++) {
        
        const struct load_command* loadCmd = (struct load_command*)cmdPtr;
        
        // 仅排查类型是 LC_SEGMENT 和 LC_SEGMENT_64 的 Load Command
        if(loadCmd->cmd == LC_SEGMENT) {
            // #define SEG_LINKEDIT "__LINKEDIT"
            // 取得段名是 __LINKEDIT 的段的虚拟基地址
            const struct segment_command* segmentCmd = (struct segment_command*)cmdPtr;
            if (strcmp(segmentCmd->segname, SEG_LINKEDIT) == 0) {
                return segmentCmd->vmaddr - segmentCmd->fileoff;
            }
            
        } else if(loadCmd->cmd == LC_SEGMENT_64) {
            // 取得段名是 __LINKEDIT 的段的虚拟基地址
            const struct segment_command_64* segmentCmd = (struct segment_command_64*)cmdPtr;
            if (strcmp(segmentCmd->segname, SEG_LINKEDIT) == 0) {
                return (uintptr_t)(segmentCmd->vmaddr - segmentCmd->fileoff);
            }
        }
        
        // 偏移到下一个段
        cmdPtr += loadCmd->cmdsize;
    }
    
    return 0;
}

@end

