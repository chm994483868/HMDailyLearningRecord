# iOS App Crash 学习：(四)：KSCrash 使用以及源码解读（2）

&emsp;[kstenerud/KSCrash](https://github.com/kstenerud/KSCrash) The Ultimate Crash Reporter!

&emsp;书接上回，这里我们接着聊，这篇会涉及更细节的东西，看 KSCrash 框架如何获取调用堆栈、如何捕获和处理 Unix Signals、如何捕获处理 Objective-C 异常。

&emsp;我们接着看 Mach 异常处理函数 KSCrashMonitor_MachException.c 文件中定义的 `static void* handleExceptions(void* const userData) { ... }` 的内容。

## handleExceptions

&emsp;KSCrashMonitor_MachException.c 文件中定义的 handleExceptions 函数超长，内容挺多的，核心则是：等待异常消息，卸载我们的异常端口，记录异常信息，并编写崩溃报告。

```c++
/** Our exception handler thread routine. Wait for an exception message, uninstall our exception port, record the exception information, and write a report. */

static void* handleExceptions(void* const userData) {

    // MachExceptionMessage 结构体用来描述一个 Mach 异常消息（根据 xnu-1699.22.81 源码中的 ux_exception.c 中的内容定义）
    // 包含：Mach header、Mach 消息内容数据、引发异常的线程、引发异常的 task、网络数据、引发的异常、异常 code 和 subcode 等
    MachExceptionMessage exceptionMessage = {{0}};
    
    // MachReplyMessage 结构体用来描述一个 Reply Mach 消息（根据 xnu-1699.22.81 源码中的 ux_exception.c 中的内容定义）
    MachReplyMessage replyMessage = {{0}};
    
    // 前面生成的 UUID，表示唯一的 evnet ID
    char* eventID = g_primaryEventID;
    
    // 取得当前线程的名字
    const char* threadName = (const char*) userData;
    pthread_setname_np(threadName);
    
    // 如果当前线程是 Mach 异常处理的辅助线程，则把此线程挂起（thread_suspend）
    if (threadName == kThreadSecondary) {
        KSLOG_DEBUG("This is the secondary thread. Suspending.");
        
        thread_suspend((thread_t)ksthread_self());
        eventID = g_secondaryEventID;
    }

    for(;;) {
        KSLOG_DEBUG("Waiting for mach exception");

        // Wait for a message.
        
        // mach_msg 函数，会阻塞在此处等待着发生异常时内核向 g_exceptionPort 端口发来 Mach 异常消息
        kern_return_t kr = mach_msg(&exceptionMessage.header,
                                    MACH_RCV_MSG,
                                    0,
                                    sizeof(exceptionMessage),
                                    g_exceptionPort,
                                    MACH_MSG_TIMEOUT_NONE,
                                    MACH_PORT_NULL);
        if (kr == KERN_SUCCESS) {
            break;
        }

        // Loop and try again on failure.
        KSLOG_ERROR("mach_msg: %s", mach_error_string(kr));
    }
    
    // 捕获的 Mach 的异常 code 和 subcode 
    KSLOG_DEBUG("Trapped mach exception code 0x%llx, subcode 0x%llx", exceptionMessage.code[0], exceptionMessage.code[1]);
    
    if (g_isEnabled) {
        thread_act_array_t threads = NULL;
        mach_msg_type_number_t numThreads = 0;
        
        // 把当前 task 的所有非当前线程以及 g_reservedThreads 中保留的线程挂起
        ksmc_suspendEnvironment(&threads, &numThreads);
        
        // 静态全局变量，记录进行了 Mach 异常的自定义处理
        g_isHandlingCrash = true;
        
        // 通知发生了致命异常被捕获
        kscm_notifyFatalExceptionCaptured(true);
        
        // 异常处理程序已经安装，继续进行异常处理...
        KSLOG_DEBUG("Exception handler is installed. Continuing exception handling.");


        // Switch to the secondary thread if necessary, or uninstall the handler to avoid a death loop.
        // 如有必要，请切换到辅助线程，或卸载处理程序以避免死亡循环。
        
        // 如果当前是 Mach 异常处理主线程，则恢复 Mach 异常处理辅助线程
        if (ksthread_self() == g_primaryMachThread) {
        
            KSLOG_DEBUG("This is the primary exception thread. Activating secondary thread.");j
            
// TODO: This was put here to avoid a freeze. Does secondary thread ever fire?
            restoreExceptionPorts();
            
            //
            if (thread_resume(g_secondaryMachThread) != KERN_SUCCESS) {
                KSLOG_DEBUG("Could not activate secondary thread. Restoring original exception ports.");
            }
        } else {
            KSLOG_DEBUG("This is the secondary exception thread.");// Restoring original exception ports.");
//            restoreExceptionPorts();
        }

        // Fill out crash information
        KSLOG_DEBUG("Fetching machine state.");
        
        // KSMC_NEW_CONTEXT(machineContext) 相当于：
        // 创建临时变量 ksmc_machineContext_storage 是一个长度可以保存下 Machine Context 数据（一个 KSMachineContext 结构体）的 char 数组，
        // 然后创建一个指向 KSMachineContext 结构体的指针 machineContext 并把它指向 ksmc_machineContext_storage。
        // char ksmc_machineContext_storage[ksmc_contextSize()];
        // struct KSMachineContext* machineContext = (struct KSMachineContext*)ksmc_machineContext_storage;
        
        KSMC_NEW_CONTEXT(machineContext);
        
        // KSCrash_MonitorContext 是一个成员变量超多的结构体，它内部记录了大量的信息，例如：崩溃类型信息/崩溃时的 APP 状态/系统信息 等等...
        // g_monitorContext 是一个此类型的静态全局变量：static KSCrash_MonitorContext g_monitorContext，
        KSCrash_MonitorContext* crashContext = &g_monitorContext;
        
        // offendingMachineContext 是 event 的 machine context
        crashContext->offendingMachineContext = machineContext;
        
        // static KSStackCursor g_stackCursor; 是一个静态全局变量，KSStackCursor 是一个描述 stack trace 的结构体，下面是对 g_stackCursor 进行初始化，
        // 其中最重要的是：cursor->symbolicate = kssymbolicator_symbolicate; symbolicate 在 KSStackCursor 结构体中是一个函数指针，指向一个用来尝试对当前地址进行符号化的函数，
        // 用于填写 KSStackCursor 结构体中嵌套定义的 stackEntry 结构体中的字段。
        // stackEntry 是 KSStackCursor 结构体中内嵌的一个结构体，用来描述 stack trace 中某个地址对应的符号的信息，kssc_initCursor 涉及的内容挺多，下面我们会单独分析一下。
        kssc_initCursor(&g_stackCursor, NULL, NULL);
        
        
        // 暂停在这里 ⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️
        
        // 获取 Mach 异常发生时所在线程的上下文信息
        if (ksmc_getContextForThread(exceptionMessage.thread.name, machineContext, true)) {
            
            // 根据取得的上下文信息接着去初始化 g_stackCursor 这个静态全局变量 
            kssc_initWithMachineContext(&g_stackCursor, KSSC_MAX_STACK_DEPTH, machineContext);
            
            // faultAddress 和 instructionAddress 地址
            KSLOG_TRACE("Fault address %p, instruction address %p", kscpu_faultAddress(machineContext), kscpu_instructionAddress(machineContext));
            
            // 根据异常了类型来为 faultAddress 属性赋值
            if (exceptionMessage.exception == EXC_BAD_ACCESS) {
                crashContext->faultAddress = kscpu_faultAddress(machineContext);
            } else {
                crashContext->faultAddress = kscpu_instructionAddress(machineContext);
            }
        }

        KSLOG_DEBUG("Filling out context.");
        
        // 崩溃上下文信息
        crashContext->crashType = KSCrashMonitorTypeMachException;
        crashContext->eventID = eventID;
        crashContext->registersAreValid = true;
        
        // Mach 异常的类型、code、subcode
        crashContext->mach.type = exceptionMessage.exception;
        crashContext->mach.code = exceptionMessage.code[0] & (int64_t)MACH_ERROR_CODE_MASK;
        crashContext->mach.subcode = exceptionMessage.code[1] & (int64_t)MACH_ERROR_CODE_MASK;
        
        // 如果是堆栈溢出的 Mach 异常
        if (crashContext->mach.code == KERN_PROTECTION_FAILURE && crashContext->isStackOverflow) {
            // A stack overflow should return KERN_INVALID_ADDRESS, but
            // when a stack blasts through the guard pages at the top of the stack,
            // it generates KERN_PROTECTION_FAILURE. Correct for this.
            
            // 堆栈溢出应返回 KERN_INVALID_ADDRESS，但是当堆栈通过堆栈顶部的保护页时，它会生成 KERN_PROTECTION_FAILURE。请正确执行此操作。
            crashContext->mach.code = KERN_INVALID_ADDRESS;
        }
        
        // 根据 Mach 异常类型转换为对应的 Unix Signal 
        crashContext->signal.signum = signalForMachException(crashContext->mach.type, crashContext->mach.code);
        // 函数堆栈 "探针"
        crashContext->stackCursor = &g_stackCursor;
    
        kscm_handleException(crashContext);

        KSLOG_DEBUG("Crash handling complete. Restoring original handlers.");
        g_isHandlingCrash = false;
        
        // 恢复线程
        ksmc_resumeEnvironment(threads, numThreads);
    }

    KSLOG_DEBUG("Replying to mach exception message.");
    
    // Send a reply saying "I didn't handle this exception".
    // 发送回复，指出"我没有处理此异常"，我们自己处理完成以后，还需要把 Mach 异常消息抛出
    
    replyMessage.header = exceptionMessage.header;
    replyMessage.NDR = exceptionMessage.NDR;
    replyMessage.returnCode = KERN_FAILURE;

    mach_msg(&replyMessage.header,
             MACH_SEND_MSG,
             sizeof(replyMessage),
             0,
             MACH_PORT_NULL,
             MACH_MSG_TIMEOUT_NONE,
             MACH_PORT_NULL);

    return NULL;
}
```

## kssc_initCursor

&emsp;这里 kssc 前缀中的 sc 即是 Stack Cursor 的首字母缩写。那么 Stack Cursor 是什么呢？

&emsp;在上面 handleExceptions 函数内部调用 kssc_initCursor 函数时，cursor 参数传递了一个静态全局变量 static KSStackCursor g_stackCursor，kssc_initCursor 函数执行过程中便对此变量进行初始化操作，g_stackCursor 会被作为后续 Stack Cursor 使用。

&emsp;聚焦在 KSStackCursor.h/.c 一对文件中内容，其实还是挺清晰的：

1. 首先是 KSStackCursor 结构体中的 stackEntry 结构体描述函数堆栈中某个栈帧（函数调用）的入口，内部的成员变量包含：stack trace 当前地址、此地址对应的 image（镜像）名称、此镜像的起始地址、最接近当前地址的符号的名称（如果有）、最接近当前地址的符号的地址。
2. 然后是 state 结构体描述当前遍历函数堆栈的状态，内部的成员变量包含：遍历函数堆栈的当前深度（基于 1）、是否已放弃遍历堆栈。
3. 再往下则是三个函数指针：resetCursor 重置 Stack Cursor（即把上面 stackEntry 和 state 两个结构体的所有成员变量置 0/false/NULL）、advanceCursor 前进 Stack Cursor 到下一个 Stack Entry、symbolicate 尝试对当前地址进行符号化并把值记录在 stackEntry 结构体中的成员变量中。
4. 最后的 context 是一个长度为 100 的 void 指针数组用来存储上下文的内部信息。

&emsp;看到这里，我们大概明白了一些 Stack Cursor 大概是用来做函数堆栈回溯的。

```c++
kssc_initCursor(&g_stackCursor, NULL, NULL);
```

```c++
void kssc_initCursor(KSStackCursor *cursor,
                     void (*resetCursor)(KSStackCursor*),
                     bool (*advanceCursor)(KSStackCursor*)) {
    // 给 symbolicate 函数指针默认赋值为 kssymbolicator_symbolicate 函数
    cursor->symbolicate = kssymbolicator_symbolicate;
    
    // 默认为 g_advanceCursor 
    cursor->advanceCursor = advanceCursor != NULL ? advanceCursor : g_advanceCursor;
    
    // 默认为 kssc_resetCursor
    cursor->resetCursor = resetCursor != NULL ? resetCursor : kssc_resetCursor;
    
    // 调用 resetCursor 函数，把 cursor 各成员变量置为 0/false/NULL 
    cursor->resetCursor(cursor);
}
```

&emsp;在上面 kssc_initCursor 函数中 kssymbolicator_symbolicate 函数格外瞩目，它牵涉的内容很对，下面我们一起看一下。

## kssymbolicator_symbolicate

&emsp;kssymbolicator_symbolicate 函数用于对 Stack Cursor 进行符号化。看到 Dl_info 时我们是不是有一丝熟悉呢，还有它对应的 dladdr 函数，之前我们学习 Mach-O 和 fishhook 时有详细学习过，现在我们再回顾一下。[iOS APP 启动优化(六)：在指定的 segment 和 section 中存入数据](https://juejin.cn/post/6980545001126101005)

&emsp;`int dladdr(const void *, Dl_info *)` 函数用于获取某个地址的符号信息，进程可通过 dladdr 获取有关最接近定义给定地址的符号信息。dladdr 可确定指定的地址是否位于构成进程的地址空间中的一个加载模块（可执行库或共享库）内。如果某个地址位于在其上面映射加载模块的基址和为该加载模块映射的最高虚拟地址之间（包括两端），则认为该地址在加载模块的范围内。如果某个加
载模块符合这个条件，则会搜索其动态符号表，以查找与指定的地址最接近的符号。最接近的符号是指其值等于或最为接近但小于指定的地址的符号。

&emsp;dladdr 函数中的 `Dl_info *` 参数是一个指向 Dl_info 结构体的指针，该结构体必须由用户创建分配，如果 dladdr 函数指定的地址在其中一个加载模块的范围内，则 Dl_info 结构体成员变量的值由 dladdr 函数设置。

&emsp;看上面的文字有点晕，看如下的示例代码，我们首先通过 class_getMethodImplementation 函数获取到 NSArray 类的 description 函数的地址，然后使用 dladdr 函数获取 description 函数的符号信息。

```c++
#import <dlfcn.h>
#include <objc/objc.h>
#include <objc/runtime.h>
#include <stdio.h>

int main(int argc, const char * argv[]) {
    
    Dl_info info;
    IMP imp = class_getMethodImplementation(objc_getClass("NSArray"), sel_registerName("description"));
    
    printf("✳️✳️✳️ pointer %p\n", imp);
    
    if (dladdr((const void *)(imp), &info)) {
        printf("✳️✳️✳️ dli_fname: %s\n", info.dli_fname);
        printf("✳️✳️✳️ dli_fbase: %p\n", info.dli_fbase);
        printf("✳️✳️✳️ dli_sname: %s\n", info.dli_sname);
        printf("✳️✳️✳️ dli_saddr: %p\n", info.dli_saddr);
    } else {
        printf("error: can't find that symbol.\n");
    }
    
    return 0;
}

// ⬇️ 控制台打印内容如下：
✳️✳️✳️ pointer 0x7fff203f44dd
✳️✳️✳️ dli_fname: /System/Library/Frameworks/CoreFoundation.framework/Versions/A/CoreFoundation
✳️✳️✳️ dli_fbase: 0x7fff20387000
✳️✳️✳️ dli_sname: -[NSArray description]
✳️✳️✳️ dli_saddr: 0x7fff203f44dd
```

&emsp;Dl_info 结构体的成员变量：

+ dli_fname 一个 char 指针，指向包含指定地址的加载模块的路径。
+ dli_fbase 加载模块的句柄，该句柄可用作 dlsym 的第一个参数。
+ dli_sname 一个 char 指针，指向与指定的地址最接近的符号的名称，该符号要么带有相同的地址，要么是带有低位地址的最接近符号。
+ dli_saddr 最接近符号的实际地址。

&emsp;dladdr 函数是有一个 int 类型的返回值的，如果指定的地址不在其中一个加载模块的范围内，则返回 0，且不修改 `Dl_info` 结构体的成员变量，否则，将返回一个非零值，同时设置 `Dl_info` 结构体的各成员变量的值。

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

extern int dladdr(const void *, Dl_info *);
```

```c++
bool kssymbolicator_symbolicate(KSStackCursor *cursor) {
    // 记录指定地址的符号信息
    Dl_info symbolsBuffer;
    
    // 如果 cursor->stackEntry.address 在一个加载模块的范围内则返回非 0 值
    if (ksdl_dladdr(CALL_INSTRUCTION_FROM_RETURN_ADDRESS(cursor->stackEntry.address), &symbolsBuffer)) {
    
        // 找到了 cursor->stackEntry.address 的符号信息，则把值赋给 cursor->stackEntry 的各个成员变量   
        cursor->stackEntry.imageAddress = (uintptr_t)symbolsBuffer.dli_fbase;
        cursor->stackEntry.imageName = symbolsBuffer.dli_fname;
        cursor->stackEntry.symbolAddress = (uintptr_t)symbolsBuffer.dli_saddr;
        cursor->stackEntry.symbolName = symbolsBuffer.dli_sname;
        
        return true;
    }
    
    // 未找到的话全部置为 0
    cursor->stackEntry.imageAddress = 0;
    cursor->stackEntry.imageName = 0;
    cursor->stackEntry.symbolAddress = 0;
    cursor->stackEntry.symbolName = 0;
    
    return false;
}
```

&emsp;上面我们看学习了 dladdr 函数的内容，但发现 ksdl_dladdr 函数并不是对 dladdr 函数的封装，ksdl_dladdr 函数内部是直接查找 image -> 查找 LC_SYMTAB -> 比较符号地址。



























+ [chinese](https://faisalmemon.github.io/ios-crash-dump-analysis-book/zh/)


## 参考链接
**参考链接:🔗**
+ [NSThead和内核线程的转换](https://www.qingheblog.online/原理分析/NSThead和内核线程的转换/)
+ [浅谈函数调用栈](https://www.qingheblog.online/原理分析/浅谈函数调用栈/)
+ [动态链接库加载拾遗&dladdr函数使用](https://www.jianshu.com/p/1ef4460b63db)
+ [ios-crash-dump-analysis-book](https://github.com/faisalmemon/ios-crash-dump-analysis-book)
+ [iOS Crash/崩溃/异常 堆栈获取](https://www.jianshu.com/p/8ece78d71b3d)
