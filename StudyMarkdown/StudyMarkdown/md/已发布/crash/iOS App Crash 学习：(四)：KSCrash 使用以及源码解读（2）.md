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
        
        // KSCrash_MonitorContext 是一个成员变量超多的结构体，它内部记录了大量的信息，g_monitorContext 是一个此类型的静态全局变量：`static KSCrash_MonitorContext g_monitorContext;`
        KSCrash_MonitorContext* crashContext = &g_monitorContext;
        
        // offendingMachineContext 是 event 的 machine context
        crashContext->offendingMachineContext = machineContext;
        
        // static KSStackCursor g_stackCursor; 是一个静态全局变量，KSStackCursor 是一个描述 stack trace 的结构体，下面是对 g_stackCursor 进行初始化，
        // 其中最重要的是：cursor->symbolicate = kssymbolicator_symbolicate; symbolicate 在 KSStackCursor 结构体中是一个函数指针，指向一个用来尝试对当前地址进行符号化的函数，
        // 用于填写 KSStackCursor 结构体中嵌套定义的 stackEntry 结构体中的字段。
        // stackEntry 是 KSStackCursor 结构体中内嵌的一个结构体，用来描述 stack trace 中某个地址对应的符号的信息，kssc_initCursor 涉及的内容挺多，下面我们会单独分析一下。
        kssc_initCursor(&g_stackCursor, NULL, NULL);
        
        
        // 暂停在这里 ⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️
        
        
        if (ksmc_getContextForThread(exceptionMessage.thread.name, machineContext, true)) {
        
            kssc_initWithMachineContext(&g_stackCursor, KSSC_MAX_STACK_DEPTH, machineContext);
            
            KSLOG_TRACE("Fault address %p, instruction address %p", kscpu_faultAddress(machineContext), kscpu_instructionAddress(machineContext));
            
            if (exceptionMessage.exception == EXC_BAD_ACCESS) {
                crashContext->faultAddress = kscpu_faultAddress(machineContext);
            } else {
                crashContext->faultAddress = kscpu_instructionAddress(machineContext);
            }
        }

        KSLOG_DEBUG("Filling out context.");
        crashContext->crashType = KSCrashMonitorTypeMachException;
        crashContext->eventID = eventID;
        crashContext->registersAreValid = true;
        crashContext->mach.type = exceptionMessage.exception;
        crashContext->mach.code = exceptionMessage.code[0] & (int64_t)MACH_ERROR_CODE_MASK;
        crashContext->mach.subcode = exceptionMessage.code[1] & (int64_t)MACH_ERROR_CODE_MASK;
        
        if (crashContext->mach.code == KERN_PROTECTION_FAILURE && crashContext->isStackOverflow) {
            // A stack overflow should return KERN_INVALID_ADDRESS, but
            // when a stack blasts through the guard pages at the top of the stack,
            // it generates KERN_PROTECTION_FAILURE. Correct for this.
            crashContext->mach.code = KERN_INVALID_ADDRESS;
        }
        
        crashContext->signal.signum = signalForMachException(crashContext->mach.type, crashContext->mach.code);
        crashContext->stackCursor = &g_stackCursor;

        kscm_handleException(crashContext);

        KSLOG_DEBUG("Crash handling complete. Restoring original handlers.");
        g_isHandlingCrash = false;
        ksmc_resumeEnvironment(threads, numThreads);
    }

    KSLOG_DEBUG("Replying to mach exception message.");
    // Send a reply saying "I didn't handle this exception".
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

&emsp;这里 kssc 前缀中的 sc 即是 Stack Cursor 的首字母缩写。那么 stack cursor 是什么呢？

&emsp;在上面 handleExceptions 函数内部调用 kssc_initCursor 函数时，cursor 参数传递了一个静态全局变量 static KSStackCursor g_stackCursor，kssc_initCursor 函数内便对此变量进行初始化操作，g_stackCursor 会被作为后续 Stack Cursor 使用。

&emsp;聚焦在 KSStackCursor.h/.c 一对文件中内容，其实还是挺清晰的。KSStackCursor 结构体中的 stackEntry 结构体描述函数堆栈中某个元素（函数调用）的入口，内部的成员变量包含：stack trace 当前地址、此地址对应的 image（镜像）名称、此镜像的起始地址、最接近当前地址的符号的名称（如果有）、最接近当前地址的符号的地址，然后是 state 结构体描述当前遍历堆栈的状态，内部的成员变量包含：遍历堆栈的当前深度（基于 1）、是否已放弃遍历堆栈。

&emsp;再往下则是三个函数指针：resetCursor 重置 Stack Cursor（即把上面的各个结构体的成员变量置 0/NULL/）、advanceCursor 前进 Stack Cursor 到下一个 Stack Entry、symbolicate 尝试对当前地址进行符号化，对 stackEntry 结构体中的各个成员变量赋值、context 是一个长度为 100 的 void 指针数组用来存储上下文的内部信息。

```c++
kssc_initCursor(&g_stackCursor, NULL, NULL);
```

```c++
void kssc_initCursor(KSStackCursor *cursor,
                     void (*resetCursor)(KSStackCursor*),
                     bool (*advanceCursor)(KSStackCursor*)) {
    // 这里给 symbolicate 函数指针默认赋值为 kssymbolicator_symbolicate 函数
    cursor->symbolicate = kssymbolicator_symbolicate;
    
    // 
    cursor->advanceCursor = advanceCursor != NULL ? advanceCursor : g_advanceCursor;
    
    // 
    cursor->resetCursor = resetCursor != NULL ? resetCursor : kssc_resetCursor;
    
    // 
    cursor->resetCursor(cursor);
}
```





























## 参考链接
**参考链接:🔗**
+ [NSThead和内核线程的转换](https://www.qingheblog.online/原理分析/NSThead和内核线程的转换/)
+ [iOS Crash/崩溃/异常 堆栈获取](https://www.jianshu.com/p/8ece78d71b3d)
