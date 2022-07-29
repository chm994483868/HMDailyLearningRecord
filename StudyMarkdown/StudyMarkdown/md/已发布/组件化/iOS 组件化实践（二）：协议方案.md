# iOS 组件化实践（二）：协议方案

&emsp;




## 参考链接
**参考链接:🔗**
+ [casatwy/CTMediator](https://github.com/casatwy/CTMediator)

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

// 地址与 ~(3UL) 做与操作，最后两位置 0
#define DETAG_INSTRUCTION_ADDRESS(A) ((A) & ~(3UL))

// ARM 架构 64 位
#define BS_THREAD_STATE_COUNT ARM_THREAD_STATE64_COUNT

// ARM 架构 64 位
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

// ARM 架构 32 位
#define BS_THREAD_STATE_COUNT ARM_THREAD_STATE_COUNT

// ARM 架构 32 位
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

// x86 架构 64 位
#define BS_THREAD_STATE_COUNT x86_THREAD_STATE64_COUNT

// x86 架构 64 位
#define BS_THREAD_STATE x86_THREAD_STATE64

// 栈底寄存器
#define BS_FRAME_POINTER __rbp

// 栈顶寄存器
#define BS_STACK_POINTER __rsp

// x86 64 位架构，对应 ARM 架构下 PC 寄存器
#define BS_INSTRUCTION_ADDRESS __rip

#elif defined(__i386__)

// 不变
#define DETAG_INSTRUCTION_ADDRESS(A) (A)

// x86 架构 32 位
#define BS_THREAD_STATE_COUNT x86_THREAD_STATE32_COUNT

// x86 架构 32 位
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

// 用于表示栈帧链表的结构体
typedef struct BSStackFrameEntry{
    const struct BSStackFrameEntry *const previous;
    const uintptr_t return_address;
} BSStackFrameEntry;

// 用于记录主线程 port 的全局变量
static mach_port_t main_thread_id;

@implementation BSBacktraceLogger

+ (void)load {
    
    // main_thread_id 是在 load 方法中赋值，确保一定是在主线程
    main_thread_id = mach_thread_self();
    
//    // ⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️
//    // 下面是一些测试的代码，不用管它们
//    pthread_t thread = pthread_self();
//    mach_port_t temp = pthread_mach_thread_np(pthread_self());
//
//    // returns non-zero if the current thread is the main thread
//    // 当前是主线程时返回 1
//    int np = pthread_main_np();
//
////    int t1 = syscall(sel_getUid);
//
//    NSLog(@"123");
//
//    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
//        mach_port_t tt1 = mach_thread_self();
//        pthread_t thread1 = pthread_self();
//        mach_port_t temp1 = pthread_mach_thread_np(pthread_self());
//
//        // returns non-zero if the current thread is the main thread
//        // 当前不是主线程，所以返回 0
//        int np1 = pthread_main_np();
//
//        NSLog(@"123");
//    });
//    // ⬆️⬆️⬆️⬆️⬆️⬆️⬆️⬆️
}

/// mach_thread_self 函数的优化版本 [一个“反问”引发的内存反思](https://blog.csdn.net/killer1989/article/details/106674973)
mach_port_t thread_self(void) {
//    mach_port_t thread_self = mach_thread_self();
//    mach_port_deallocate(mach_task_self(), thread_self);
//    return thread_self;
    
    return pthread_mach_thread_np(pthread_self());
}

#pragma -mark Implementation of interface

// 获取指定线程的调用栈回溯
+ (NSString *)bs_backtraceOfNSThread:(NSThread *)thread {
    
    // 把 thread 转换为 thread_t 类型的 Mach 线程
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

/// 对当前所有线程进行栈回溯
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
    
    // 栈回溯字符串开头拼接线程数量
    NSMutableString *resultString = [NSMutableString stringWithFormat:@"Call Backtrace of %u threads:\n", thread_count];
    
    // 然后循环对所有的线程进行调用栈回溯，把回溯的字符串拼接在 resultString 中
    for (int i = 0; i < thread_count; i++) {
        [resultString appendString:_bs_backtraceOfThread(threads[i])];
    }
    
    return [resultString copy];
}

#pragma -mark Get call backtrace of a mach_thread

/// 获取指定 Mach 线程（thread_t）的调用栈回溯
/// @param thread Mach 线程 thread_t
NSString *_bs_backtraceOfThread(thread_t thread) {
    // 默认栈深度是 50
    uintptr_t backtraceBuffer[50];
    
    int i = 0;
    
    // 栈回溯字符串，默认开头字符串都是 thread ID
    NSMutableString *resultString = [[NSMutableString alloc] initWithFormat:@"Backtrace of Thread %u:\n", thread];
    
    // 针对不同平台的机器上下文变量
    _STRUCT_MCONTEXT machineContext;
    
    // 获取指定线程的上下文，如果获取失败的话直接返回错误描述
    if(!bs_fillThreadStateIntoMachineContext(thread, &machineContext)) {
        return [NSString stringWithFormat:@"Fail to get information about thread: %u", thread];
    }
    
    // 获取 __rip 的值（对应 ARM 架构下 PC 寄存器的值）
    const uintptr_t instructionAddress = bs_mach_instructionAddress(&machineContext);
    // 记录一个 PC 寄存器的值
    backtraceBuffer[i] = instructionAddress;
    ++i;
    
    // FP(x29) 栈底 SP 栈顶 PC 下一条指令 LR(x30) 函数返回后的下一个函数的第一条指令
    // x29(FP) 栈底寄存器 SP 栈顶寄存器 LR（x30）是当前函数返回后，下一个函数的第一条指令 PC 下一条指令
    
    // 读取 LR 寄存器的值，只有 ARM 平台有，x86 平台返回 0
    uintptr_t linkRegister = bs_mach_linkRegister(&machineContext);
    if (linkRegister) {
        backtraceBuffer[i] = linkRegister;
        i++;
    }
    
    // instructionAddress 为 0 的话，返回一个错误字符串，感觉这个判断应该放在上面获取后直接判断吧，没必要读了 LR 寄存器再判断吧！
    if (instructionAddress == 0) {
        return @"Fail to get instruction address";
    }
    
    // 创建一个栈帧节点
    BSStackFrameEntry frame = {0};
    
    // FP 栈底寄存器的值
    const uintptr_t framePtr = bs_mach_framePointer(&machineContext);
    
    // 使用 vm_read_overwrite() 函数，从目标进程 "读取" 内存。
    // 注意，这个函数与 vm_read() 不同，应该并没有做实际的数据拷贝，而是将 region.address ~ region.address + region.size 范围对应的所有映射状态同步给了 region_data ~ region_data + region.size，对于 Resident 的部分，两个进程中不同的虚拟内存地址对应的应该是相同的物理内存地址。
    
    // 如果 framePtr 等于 0 或者以 framePtr 为起始地址，复制 sizeof(frame) 个长度的虚拟内存的数据到 frame 指针中去失败，则返回错误描述，
    // 这里 frame 变量是 struct BSStackFrameEntry 类型的结构体，它内部一个指针，一个无符号 long 变量，所以 sizeof(frame) 的值为 16，
    // 即这里的作用是把 FP 栈底寄存器的值复制到 frame 中
    if (framePtr == 0 || bs_mach_copyMem((void *)framePtr, &frame, sizeof(frame)) != KERN_SUCCESS) {
        return @"Fail to get frame pointer";
    }
    
    // 循环 50 次，沿着栈底指针构建一个链表，链表的每个节点都是每个函数帧的栈底指针，即前一个函数帧的起始地址
    for (; i < 50; i++) {
        backtraceBuffer[i] = frame.return_address;
        
        if (backtraceBuffer[i] == 0 || frame.previous == 0 || bs_mach_copyMem(frame.previous, &frame, sizeof(frame)) != KERN_SUCCESS) {
            break;
        }
    }
    
    // 准备一个长度是 backtraceLength 的 Dl_info 数组
    int backtraceLength = i;
    Dl_info symbolicated[backtraceLength];
    
    bs_symbolicate(backtraceBuffer, symbolicated, backtraceLength, 0);
    
    //
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

//
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

// 获取 thread 的状态赋值到 machineContext 参数，bool 返回值表示是否获取成功/失败
bool bs_fillThreadStateIntoMachineContext(thread_t thread, _STRUCT_MCONTEXT *machineContext) {
    mach_msg_type_number_t state_count = BS_THREAD_STATE_COUNT;
    
    // 获取指定 thread 的上下文，并赋值在 machineContext 参数中
    kern_return_t kr = thread_get_state(thread, BS_THREAD_STATE, (thread_state_t)&machineContext->__ss, &state_count);
    
    return (kr == KERN_SUCCESS);
}

// 获取栈底寄存器的值
uintptr_t bs_mach_framePointer(mcontext_t const machineContext){
    return machineContext->__ss.BS_FRAME_POINTER;
}

uintptr_t bs_mach_stackPointer(mcontext_t const machineContext){
    return machineContext->__ss.BS_STACK_POINTER;
}

// 获取 x86 平台下 IP 寄存器的值，对应 ARM 架构下 PC 寄存器的值
uintptr_t bs_mach_instructionAddress(mcontext_t const machineContext){
    return machineContext->__ss.BS_INSTRUCTION_ADDRESS;
}

// 读取 LR 寄存器的值，LR 是当前函数结束后，下一个函数的第一条指令。x86 平台没有这个寄存器，只有 ARM 平台才有。
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

//
void bs_symbolicate(const uintptr_t* const backtraceBuffer,
                    Dl_info* const symbolsBuffer,
                    const int numEntries,
                    const int skippedEntries){
    int i = 0;
    
    if (!skippedEntries && i < numEntries) {
        bs_dladdr(backtraceBuffer[i], &symbolsBuffer[i]);
        i++;
    }
    
    for (; i < numEntries; i++) {
        bs_dladdr(CALL_INSTRUCTION_FROM_RETURN_ADDRESS(backtraceBuffer[i]), &symbolsBuffer[i]);
    }
}

//
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
    
    // 取得 Image 在当前可执行文件中的虚拟地址的基地址 然后 加上 Slide
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
    
    // 符号表中的每个元素正是这个 struct nlist_64/nlist 类型
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
            
            // 然后对符号表中的符号进行遍历
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
                    // 所以这里 currentDistance 的值是减法溢出后转换为无符号 long
                    uintptr_t currentDistance = addressWithSlide - symbolBase;
                    
                    //
                    if ((addressWithSlide >= symbolBase) && (currentDistance <= bestDistance)) {
                        //
                        bestMatch = symbolTable + iSym;
                        //
                        bestDistance = currentDistance;
                    }
                }
            }
            
            //
            if (bestMatch != NULL) {
                info->dli_saddr = (void*)(bestMatch->n_value + imageVMAddrSlide);
                info->dli_sname = (char*)((intptr_t)stringTable + (intptr_t)bestMatch->n_un.n_strx);
                
                if (*info->dli_sname == '_') {
                    info->dli_sname++;
                }
                
                // This happens if all symbols have been stripped.
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

@end

