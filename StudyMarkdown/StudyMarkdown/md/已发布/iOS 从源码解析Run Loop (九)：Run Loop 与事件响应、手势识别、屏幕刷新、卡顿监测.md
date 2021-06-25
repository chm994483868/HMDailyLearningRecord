# iOS 从源码解析Run Loop (九)：Run Loop 与事件响应、手势识别、屏幕刷新、卡顿监测

> &emsp;上一篇我们主要分析了 CFRunLoopTimerRef 相关的内容和部分 CFRunLoopObserverRef 相关的内容，本篇我们详细分析 CFRunLoopSourceRef 相关的内容。

&emsp;在开始之前我们再详细区分一下 CFRunLoopSourceRef 的 source0 和 source1 两个版本。
## source0 和 source1 的区别
&emsp;首先我们从代码层面对 source0 和 source1 版本的 CFRunLoopSourceRef 进行区分，struct \__CFRunLoopSource 通过其内部的 \_context 来进行区分 source0 和 source1。
```c++
struct __CFRunLoopSource {
    ...
    union {
        CFRunLoopSourceContext version0;   
        CFRunLoopSourceContext1 version1;
    } _context;
};
```
&emsp;其中 version0、version1 分别对应 source0 和 source1，下面我们再看一下 CFRunLoopSourceContext 和 CFRunLoopSourceContext1 的定义：
```c++
typedef struct {
    ...
    void * info; // 作为 perform 函数的参数
    ...
    void (*schedule)(void *info, CFRunLoopRef rl, CFStringRef mode); // 当 source0 加入到 run loop 时触发的回调函数（在 CFRunLoopAddSource 函数中可看到其被调用）
    void (*cancel)(void *info, CFRunLoopRef rl, CFStringRef mode); // 当 source0 从 run loop 中移除时触发的回调函数
    
    // source0 要执行的任务块，当 source0 事件被触发时的回调, 调用 __CFRUNLOOP_IS_CALLING_OUT_TO_A_SOURCE0_PERFORM_FUNCTION__ 函数来执行 perform(info)
    void (*perform)(void *info);
} CFRunLoopSourceContext;
```

```c++
typedef struct {
    ...
    void * info; // 作为 perform 函数的参数
    ...
#if (TARGET_OS_MAC && !(TARGET_OS_EMBEDDED || TARGET_OS_IPHONE)) || (TARGET_OS_EMBEDDED || TARGET_OS_IPHONE)

    // getPort 函数指针，用于当 source1 被添加到 run loop 中的时候，从该函数中获取具体的 mach_port_t 对象，用来唤醒 run loop
    mach_port_t (*getPort)(void *info);
    
    // perform 函数指针即指向 run loop 被唤醒后 source1 要执行的回调函数，调用 __CFRUNLOOP_IS_CALLING_OUT_TO_A_SOURCE1_PERFORM_FUNCTION__ 函数来执行
    void * (*perform)(void *msg, CFIndex size, CFAllocatorRef allocator, void *info);
#else
    // 其它平台
    void * (*getPort)(void *info);
    void (*perform)(void *info);
#endif
} CFRunLoopSourceContext1;
```
&emsp;source0 仅包含一个回调函数（perform），它并不能主动唤醒 run loop（进入休眠的 run loop 仅能通过 mach port 和 mach_msg 来唤醒）。使用时，你需要先调用 CFRunLoopSourceSignal(rls) 将这个 source 标记为待处理，然后手动调用 CFRunLoopWakeUp(rl) 来唤醒 run loop（CFRunLoopWakeUp 函数内部是通过 run loop 实例的 \_wakeUpPort 成员变量来唤醒 run loop 的），唤醒后的 run loop 继续执行 \__CFRunLoopRun 函数内部的外层 do while 循环来执行 timers（执行到达执行时间点的 timer 以及更新下次最近的时间点） 和 sources 以及 observer 回调 run loop 状态，其中通过调用 \__CFRunLoopDoSources0 函数来执行 source0 事件，执行过后的 source0 会被 \__CFRunLoopSourceUnsetSignaled(rls) 标记为已处理，后续 run loop 循环中不会再执行标记为已处理的 source0。source0 不同于不重复执行的 timer 和 run loop 的 block 链表中的 block 节点，source0 执行过后不会自己主动移除，不重复执行的 timer 和 block 执行过后会自己主动移除，执行过后的 source0 可手动调用 CFRunLoopRemoveSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode) 来移除。

&emsp;source0 具体执行时的函数如下，info 做参数执行 perform 函数。
```c++
__CFRUNLOOP_IS_CALLING_OUT_TO_A_SOURCE0_PERFORM_FUNCTION__(rls->_context.version0.perform, rls->_context.version0.info); // perform(info)
```
&emsp;下面是我们手动创建 source0 的示例代码，创建好的 CFRunLoopSourceRef 必须调用 CFRunLoopSourceSignal 函数把其标记为待处理，否则即使 run loop 正常循环，这里的 rls 也得不到执行，由于 thread 线程中的计时器存在所以这里也可以不用手动调用 CFRunLoopWakeUp 唤醒 run loop，run loop 已是唤醒状态，rls 能在 run loop 的一个循环中正常得到执行，然后是其中的三个断点，当执行到断点时我们在控制台打印 po [NSRunLoop currentRunLoop] 可在 kCFRunLoopDefaultMode 的 sources0 哈希表中看到 rls，以及它的 signalled 标记的值，通过源码可知在 rls 的 perform 待执行之前就会先调用 \__CFRunLoopSourceUnsetSignaled(rls) 把其标记为已经处理，且处理过的 rls 并不会主动移除，它依然被保存在 kCFRunLoopDefaultMode 的 sources0 哈希表中，我们可以使用 CFRunLoopRemoveSource 函数手动移除。source0 不同于不重复执行的 timer 和 run loop 的 block 链表中的 block 节点，source0 执行过后不会自己主动移除，不重复执行的 timer 和 block 执行过后自己会主动移除。

&emsp;话说是执行 source0 时需要手动调用 CFRunLoopWakeUp 来唤醒 run loop，实际觉得好像大部分场景下其它事件都会导致 run loop 正常进行着循环，只要 run loop 进行循环则标记为待处理的 source0 就能得到执行，好像并不需要我们刻意的手动调用 CFRunLoopWakeUp 来唤醒当前的 run loop。 
```c++
- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    NSThread *thread = [[NSThread alloc] initWithBlock:^{
        NSLog(@"🧗‍♀️🧗‍♀️ ....");
        
        // 构建下下文，这里只有三个参数有值，0 是 version 值代表是 source0，info 则直接传的 self 即当前的 vc，schedule 和 cancel 偷懒了传的 NULL，它们分别是
        // 执行 CFRunLoopAddSource 添加 rls 和 CFRunLoopRemoveSource 移除 rls 时调用的，大家可以自己试试，
        // 然后最后是执行函数 perform 传了 runLoopSourcePerformRoutine。
        CFRunLoopSourceContext context = {0, (__bridge void *)(self), NULL, NULL, NULL, NULL, NULL, NULL, NULL, runLoopSourcePerformRoutine};
        
        CFRunLoopSourceRef rls = CFRunLoopSourceCreate(kCFAllocatorDefault, 0, &context);
        CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);

        // 创建好的 rls 必须手动标记为待处理，否则即使 run loop 正常循环也不会执行此 rls
        CFRunLoopSourceSignal(rls); // ⬅️ 断点 1
        
        // 由于计时器一直在循环执行，所以这里可不需要我们手动唤醒 run loop 
        CFRunLoopWakeUp(CFRunLoopGetCurrent()); // ⬅️ 断点 2

        [NSTimer scheduledTimerWithTimeInterval:1 repeats:YES block:^(NSTimer * _Nonnull timer) {
            NSLog(@"⏰⏰⏰ timer 回调...");
            CFRunLoopRemoveSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode); // ⬅️ 断点 4（这里执行一次计时器回调再打断点）
        }];

        [[NSRunLoop currentRunLoop] run]; 
    }];
    
    [thread start];
}

void runLoopSourcePerformRoutine (void *info) {
    NSLog(@"👘👘 %@", [NSThread currentThread]); // ⬅️ 断点 3
}
```
&emsp;初始创建完成的 rls 的 signalled 值为 NO，如果接下来不执行 CFRunLoopSourceSignal(rls) 的话，rls 是不会被 run loop 执行的。
```c++
// ⬅️ 断点 1
...
sources0 = <CFBasicHash 0x282aa55f0 [0x20e729430]>{type = mutable set, count = 1,
entries =>
    1 : <CFRunLoopSource 0x2811f6580 [0x20e729430]>{signalled = No, valid = Yes, order = 0, context = <CFRunLoopSource context>{version = 0, info = 0x139d1c2e0, callout = runLoopSourcePerformRoutine (0x100e929ec)}}
}
...
```
&emsp;CFRunLoopSourceSignal(rls) 执行后，看到 rls 的 signalled 置为 Yes，在 run loop 循环中调用 \__CFRunLoopDoSources0 函数时 rls 会得到执行。
```c++
// ⬅️ 断点 2
...
sources0 = <CFBasicHash 0x282aa55f0 [0x20e729430]>{type = mutable set, count = 1,
entries =>
    1 : <CFRunLoopSource 0x2811f6580 [0x20e729430]>{signalled = Yes, valid = Yes, order = 0, context = <CFRunLoopSource context>{version = 0, info = 0x139d1c2e0, callout = runLoopSourcePerformRoutine (0x100e929ec)}}
}
...
```
&emsp;通过 \__CFRunLoopDoSources0 函数的源码可知在 rls 的 perform 函数执行之前 \__CFRunLoopSourceUnsetSignaled(rls) 已经把 rls 标记为已处理。
```c++
// ⬅️ 断点 3
...
sources0 = <CFBasicHash 0x282aa55f0 [0x20e729430]>{type = mutable set, count = 1,
entries =>
    1 : <CFRunLoopSource 0x2811f6580 [0x20e729430]>{signalled = No, valid = Yes, order = 0, context = <CFRunLoopSource context>{version = 0, info = 0x139d1c2e0, callout = runLoopSourcePerformRoutine (0x100e929ec)}}
}
...
}
```
&emsp;CFRunLoopRemoveSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode) 执行过后看到 rls 已经被移除，这里 source0 不同于不重复执行的 timer 和 run loop 的 block 链表中的 block 节点，source0 执行过后不会自己主动移除，timer 和 block 执行过后自己会主动移除。
```c++
// ⬅️ 断点 4
...
sources0 = <CFBasicHash 0x282aa55f0 [0x20e729430]>{type = mutable set, count = 0,
entries =>
}
...
```
&emsp;针对 timers/sources（0/1） 的执行流程（暂时忽略 run loop 休眠和 main run loop 执行，其实极极极大部分情况我们都是在使用主线程的 run loop，这里为了分析 timers/sources 暂时假装是在子线程的 run loop 中）我们这里再回顾一下 \__CFRunLoopRun 函数，从 \__CFRunLoopRun 函数的外层 do while 循环开始，首先进来会连着回调 kCFRunLoopBeforeTimers 和 kCFRunLoopBeforeSources 两个 run loop 的活动变化，然后接下来就是调用 \__CFRunLoopDoSources0(rl, rlm, stopAfterHandle) 来执行 source0，如果有 source0 被执行了，则 sourceHandledThisLoop 为 True，就不会回调 kCFRunLoopBeforeWaiting 和 kCFRunLoopAfterWaiting 两个活动变化。接着是根据当前 run loop 的本次循环被某个 mach port 唤醒的（\__CFRunLoopServiceMachPort(waitSet, &msg, sizeof(msg_buffer), &livePort, poll ? 0 : TIMEOUT_INFINITY, &voucherState, &voucherCopy) 唤醒本次 run loop 的 mach port 会被赋值到 livePort 中）来处理具体的内容，假如是 rlm->_timerPort（或 modeQueuePort 它两等同只是针对不同的平台不同的 timer 使用方式）唤醒的则调用 \__CFRunLoopDoTimers(rl, rlm, mach_absolute_time()) 来执行 timer 的回调，如果还有其它 timer 或者 timer 重复执行的话会调用 \__CFArmNextTimerInMode(rlm, rl) 来更新注册下次最近的 timer 的触发时间。  最后的话就是 source1 的端口了，首先通过 CFRunLoopSourceRef rls = __CFRunLoopModeFindSourceForMachPort(rl, rlm, livePort)（内部是 CFRunLoopSourceRef found = rlm->_portToV1SourceMap ? (CFRunLoopSourceRef)CFDictionaryGetValue(rlm->_portToV1SourceMap, (const void *)(uintptr_t)port) : NULL;，即从 rlm 的 \_portToV1SourceMap 字典中以 livePort 为 Key 找到对应的 CFRunLoopSourceRef）来找到 livePort 所对应的具体的 rls（source1），然后是调用 \__CFRunLoopDoSource1(rl, rlm, rls, msg, msg->msgh_size, &reply) 来执行 rls 的回调，内部具体的执行是 \_\_CFRUNLOOP_IS_CALLING_OUT_TO_A_SOURCE1_PERFORM_FUNCTION\_\_(rls->_context.version1.perform, msg, size, reply, rls->_context.version1.info) 即同样是 info 做参数执行 perform 函数，且在临近执行前同样会调用 \__CFRunLoopSourceUnsetSignaled(rls) 把 source1 标记为已处理，且同 soure0 一样也不会主动从 rlm 的 sources1 哈希表中主动移除。（source1 系统会自动 signaled）

&emsp; Source1 包含了一个 mach port（由 CFRunLoopSourceRef 创建时的 CFRunLoopSourceContext1 传入） 和一个回调（CFRunLoopSourceContext1 的 perform 函数指针），被用于通过内核和其它线程相互发送消息（mach_msg），这种 Source 能主动唤醒 run loop 的线程。

&emsp;Source1 包含的 mach port 来自于创建 source1 时 CFRunLoopSourceContext1 的 info 成员变量，CFRunLoopSourceRef 通过 \_context  的 info 持有 mach port，同时以 CFRunLoopSourceRef 为 Key，以 mach port 为 Value 保存在 rlm 的 \_portToV1SourceMap 中，并且会把该 mach port 插入到 rlm 的 \_portSet 中。如下代码摘录自 CFRunLoopAddSource 函数中：
```c++
...
} else if (1 == rls->_context.version0.version) {
    // 把 rls 添加到 rlm 的 _sources1 集合中
    CFSetAddValue(rlm->_sources1, rls);
    
    // 以 info 为参，调用 rls->_context.version1.getPort 函数读出 mach port
    // 基于 CFMachPort 创建的 CFRunLoopSourceRef 其 context 的 getPort 指针被赋值为 __CFMachPortGetPort 函数（iOS 下仅能使用 CFMachPort，不能使用 CFMessagePort）
    // 基于 CFMessagePort 创建的 CFRunLoopSourceRef 其 context 的 getPort 指针被赋值为 __CFMessagePortGetPort 函数（macOS 下可用 CFMessagePort）
    __CFPort src_port = rls->_context.version1.getPort(rls->_context.version1.info);
    
    if (CFPORT_NULL != src_port) {
        // 把 rls 和 src_port 保存在 rlm 的 _portToV1SourceMap 字典中
        CFDictionarySetValue(rlm->_portToV1SourceMap, (const void *)(uintptr_t)src_port, rls);
        // 把 src_port 插入到 rlm 的 _portSet 中
        __CFPortSetInsert(src_port, rlm->_portSet);
    }
}
...
```
&emsp;可看到 source0 中仅有一些回调函数（perform 函数指针）会在 run loop 的本次循环中执行，而 source1 中有 mach port 可用来主动唤醒 run loop 后执行 source1 中的回调函数（perform 函数指针），即 source1 创建时会有 mach port 传入，然后当通过 mach_msg 函数向这个 mach port 发消息时，当前的 run loop 就会被唤醒来执行这个 source1 事件，但是 source0 则是依赖于由 “别人” 来唤醒 run loop，例如由开发者手动调用 CFRunLoopWakeUp 函数来唤醒 run loop，或者由 source1 唤醒 run loop 后，在当前 run loop 的本次循环中被标记为待处理的 source0 也趁机得到执行。 

&emsp;source1 由 run loop 和内核管理，mach port 驱动。 source0 则偏向应用层一些，如 Cocoa 里面的 UIEvent 处理，会以 source0 的形式发送给 main run loop。

&emsp;翻看了几篇博客后发现手动唤醒 run loop 适用的场景可以是在主线程中唤醒休眠中的子线程。只要能拿到子线程的 run loop 对象就能通过调用 CFRunLoopWakeUp 函数来唤醒指定的子线程，唤醒的方式是调用 mach_msg 函数向子线程的 run loop 对象的 \_weakUpPort 发送消息即可。下面我们看一下挺简短的源码。

&emsp;CFRunLoopWakeUp 函数定义如下，只需要一个我们想要唤醒的线程的 run loop 对象。
```c++
void CFRunLoopWakeUp(CFRunLoopRef rl) {
    CHECK_FOR_FORK();
    // This lock is crucial to ignorable wakeups, do not remove it.
    
    // CFRunLoopRef 加锁
    __CFRunLoopLock(rl);
    
    // 如果 rl 已经被标记为 "忽略唤醒"，则直接解锁 return，
    // 其实当 rl 有这个 "忽略唤醒" 的标记时代表的是 rl 此时已经是唤醒状态了，所以本次唤醒操作可以忽略。
    // 全局搜索 __CFRunLoopSetIgnoreWakeUps 设置 "忽略唤醒" 标记的函数，
    // 可发现其调用都是在 __CFRunLoopRun 函数中 run loop 唤醒之前，用来标记 run loop 此时是唤醒状态。 
    if (__CFRunLoopIsIgnoringWakeUps(rl)) {
        __CFRunLoopUnlock(rl);
        return;
    }
    
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
    kern_return_t ret;
    
    /* We unconditionally try to send the message, since we don't want to lose a wakeup,
    but the send may fail if there is already a wakeup pending, since the queue length is 1. */
    
    // __CFSendTrivialMachMessage 函数内部正是调用 mach_msg 向 rl->_wakeUpPort 端口发送消息
    ret = __CFSendTrivialMachMessage(rl->_wakeUpPort, 0, MACH_SEND_TIMEOUT, 0);
    // 发送不成功且不是超时，则 crash
    if (ret != MACH_MSG_SUCCESS && ret != MACH_SEND_TIMED_OUT) CRASH("*** Unable to send message to wake up port. (%d) ***", ret);
    
#elif DEPLOYMENT_TARGET_WINDOWS
    SetEvent(rl->_wakeUpPort);
#endif
    // CFRunLoopRef 解锁
    __CFRunLoopUnlock(rl);
}
```
&emsp;如此，主线程通过调用 CFRunLoopWakeUp(rl) 来唤醒子线程的 run loop，那么添加到子线程中的标记为待处理的 source0 就能得到执行了。

&emsp;Cocoa Foundation 和 Core Foundation 为使用与端口相关的对象和函数创建基于端口的输入源（source1）提供内置支持。例如，在 Cocoa Foundation 中，我们根本不需要直接创建 source1，只需创建一个端口对象，并使用 NSRunLoop  的实例方法将该端口添加到 run loop 中。port 对象会处理所需 source1 的创建和配置。如下代码在子线程中:
```c++
NSPort *port = [NSPort port];
[[NSRunLoop currentRunLoop] addPort:port forMode:NSDefaultRunLoopMode];
```
&emsp;即可在当前 run loop 的 NSDefaultRunLoopMode 模式的 sources1 集合中添加一个 source1，此时只要在主线程中能拿到 port 我们就可以实现主线和子线的通信（唤醒子线程）。

&emsp;在上面示例代码中打一个断点，然后在控制台执行 po [NSRunLoop currentRunLoop]，可看到 kCFRunLoopDefaultMode 模式的 sources1 哈希表中多了一个 source1: 
```c++
...
sources1 = <CFBasicHash 0x28148ebe0 [0x20e729430]>{type = mutable set, count = 1,
entries =>
    2 : <CFRunLoopSource 0x282fd9980 [0x20e729430]>{signalled = No, valid = Yes, order = 200, context = <CFMachPort 0x282ddca50 [0x20e729430]>{valid = Yes, port = a20b, source = 0x282fd9980, callout = __NSFireMachPort (0x1df1ee1f0), context = <CFMachPort context 0x28148ec70>}}
}
...
```
&emsp;在 Core Foundation 中则必须手动创建端口及其 source1。在这两种情况下，都使用与端口不透明类型（CFMachPortRef、CFMessagePortRef 或 CFSocketRef）相关联的函数来创建适当的对象。

## 事件响应
> &emsp;在 com.apple.uikit.eventfetch-thread 线程下苹果注册了一个 Source1 (基于 mach port 的) 用来接收系统事件，其回调函数为 \__IOHIDEventSystemClientQueueCallback()，HID 是 Human Interface Devices “人机交互” 的首字母缩写。
> 
> &emsp;当一个硬件事件(触摸/锁屏/摇晃等)发生后，首先由 IOKit.framework 生成一个 IOHIDEvent 事件并由 SpringBoard 接收。这个过程的详细情况可以参考[IOHIDFamily](http://iphonedevwiki.net/index.php/IOHIDFamily)。SpringBoard 只接收按键(锁屏/静音等)，触摸，加速，接近传感器等几种 Event，随后用 mach port 转发给需要的 App 进程。随后苹果注册的那个 Source1 就会触发回调，并调用 \_UIApplicationHandleEventQueue 进行应用内部的分发（这里的函数名已经发生了变化，但是处理流程还是和大佬说的是一样的，在 Xcode 11 和 iOS 13 下打印看到的函数名是：\__handleEventQueue，在 Xcode 12 和 iOS 14 下是：\__eventQueueSourceCallback） 。
> 
> &emsp;\_UIApplicationHandleEventQueue 会把 IOHIDEvent 处理并包装成 UIEvent 进行处理或分发，其中包括识别 UIGesture/处理屏幕旋转/发送给 UIWindow 等。通常事件比如 UIButton 点击、touchesBegin/Move/End/Cancel 事件都是在这个回调中完成的。[深入理解RunLoop](https://blog.ibireme.com/2015/05/18/runloop/)

&emsp;我们在程序中添加一个 \__IOHIDEventSystemClientQueueCallback 的符号断点，运行程序后触摸屏幕会进入该断点，然后 bt 打印当前的函数调用堆栈如下，可看到目前是在 com.apple.uikit.eventfetch-thread 线程，此时主线程是休眠状态，系统正是通过 com.apple.uikit.eventfetch-thread 来唤醒主线程。
```c++
(lldb) bt
* thread #6, name = 'com.apple.uikit.eventfetch-thread', stop reason = breakpoint 2.1
  * frame #0: 0x00000001dea0745c IOKit`__IOHIDEventSystemClientQueueCallback // ⬅️ （mp 是 CFMachPortRef）mp->_callout(mp, msg, size, context_info);
    frame #1: 0x00000001de6ea990 CoreFoundation`__CFMachPortPerform + 188 // ⬅️ CFMachPort 端口的回调函数
    frame #2: 0x00000001de711594 CoreFoundation`__CFRUNLOOP_IS_CALLING_OUT_TO_A_SOURCE1_PERFORM_FUNCTION__ + 56 // ⬅️ 
    frame #3: 0x00000001de710ce0 CoreFoundation`__CFRunLoopDoSource1 + 440 // ⬅️ 可看到触摸事件确实是 source1 事件
    frame #4: 0x00000001de70bb04 CoreFoundation`__CFRunLoopRun + 2096
    frame #5: 0x00000001de70afb4 CoreFoundation`CFRunLoopRunSpecific + 436
    frame #6: 0x00000001df0d995c Foundation`-[NSRunLoop(NSRunLoop) runMode:beforeDate:] + 300
    frame #7: 0x00000001df0d97ec Foundation`-[NSRunLoop(NSRunLoop) runUntilDate:] + 96
    frame #8: 0x000000020b052754 UIKitCore`-[UIEventFetcher threadMain] + 136
    frame #9: 0x00000001df2064a0 Foundation`__NSThread__start__ + 984
    frame #10: 0x00000001de39d2c0 libsystem_pthread.dylib`_pthread_body + 128
    frame #11: 0x00000001de39d220 libsystem_pthread.dylib`_pthread_start + 44
    frame #12: 0x00000001de3a0cdc libsystem_pthread.dylib`thread_start + 4
(lldb) 
```
&emsp;在控制台打印 po [NSRunLoop currentRunLoop]，看一下当前线程的 run loop，此时应在模拟器中运行，可能由于真机的访问控制有关，如果使用真机的话无法看到 sources 的具体的回调函数名，用模拟器可以看到。看到 com.apple.uikit.eventfetch-thread 线程的 run loop 仅有一个默认模式，且仅有一组 sources1，其他 sources0/observers/timers 集合中都是空的。由于内容太多，这里我们只摘录出 kCFRunLoopDefaultMode 模式下回调函数名是 \__IOHIDEventSystemClientQueueCallback 的 source1。
```c++
...
sources1 = <CFBasicHash 0x600000cf0210 [0x7fff80617cb0]>{type = mutable set, count = 3,
entries =>
    ...
    1 : <CFRunLoopSource 0x6000037a8780 [0x7fff80617cb0]>{signalled = No, valid = Yes, order = 0, context = <CFMachPort 0x6000035a0580 [0x7fff80617cb0]>{valid = Yes, port = 3307, source = 0x6000037a8780, callout = __IOHIDEventSystemClientQueueCallback (0x7fff25e91d1d), context = <CFMachPort context 0x7fbc69e007f0>}}
    ...
}
...
```
&emsp;这里确实如大佬所说，在 com.apple.uikit.eventfetch-thread 线程的 run loop 的 kCFRunLoopDefaultMode 模式（且它仅有这一个模式）下包含一个回调函数是 \__IOHIDEventSystemClientQueueCallback 的 source1，正是通过它的 mach port 来接收由 SpringBoard  转发来的硬件事件(触摸/锁屏/摇晃等)，我们当前的 App 进程接收到事件以后，然后大佬说的是调用 \_UIApplicationHandleEventQueue 来进行应用内部的分发。可能是年代较久远这里苹果对此函数名做了修改，在 Xcode 11 和 iOS 13 下打印主线程的 run loop 发现是回调函数名字是 \__handleEventQueue 的 source0，然后在 Xcode 12 和 iOS 14 下打印主线程的 run loop 发现回调函数名字是 \__eventQueueSourceCallback 的 source0。

&emsp;下面我们看一下另一位大佬关于事件响应更详细一点的分析：[iOS RunLoop完全指南](https://blog.csdn.net/u013378438/article/details/80239686)

> &emsp;iOS 设备的事件响应，是有 RunLoop 参与的。提起 iOS 设备的事件响应，相信大家都会有一个大概的了解: (1) 用户触发事件 -> (2) 系统将事件转交到对应 APP 的事件队列 -> (3) APP 从消息队列头取出事件 -> (4) 交由 Main Window 进行消息分发 -> (5) 找到合适的 Responder 进行处理，如果没找到，则会沿着 Responder chain 返回到 APP 层，丢弃不响应该事件。
> 
> &emsp;这里涉及到两个问题，(3) 到 (5) 步是由进程内处理的，而 (1) 到 (2) 步则涉及到设备硬件，iOS 操作系统，以及目标 APP 之间的通信，通信的大致步骤是什么样的呢？当我们的 APP 在接收到任何事件请求之前，main RunLoop 都是处于 mach_msg_trap 休眠状态中的，那么，又是谁唤醒它的呢？（com.apple.uikit.eventfetch-thread）

&emsp;首先我们在控制台用 po [NSRunLoop currentRunLoop] 打印出主线程的 run loop 的内容，这里内容超多，我们只摘录和我们分析相关的内容，可看到当前 main run loop 有 4 种 mode，这里我们只看 kCFRunLoopDefaultMode 和 UITrackingRunLoopMode 以及 kCFRunLoopCommonModes，它们三者下均有一个回调函数是 \__handleEventQueue（Xcode 11 + iOS 13）的 source0 事件。（在 Xcode 12 + iOS 14 下是 \__eventQueueSourceCallback 和 \__eventFetcherSourceCallback）
```c++
...
current mode = kCFRunLoopDefaultMode,
common modes = <CFBasicHash 0x60000014a400 [0x7fff80617cb0]>{type = mutable set, count = 2,
entries =>
    0 : <CFString 0x7fff867f6c40 [0x7fff80617cb0]>{contents = "UITrackingRunLoopMode"}
    2 : <CFString 0x7fff8062b0a0 [0x7fff80617cb0]>{contents = "kCFRunLoopDefaultMode"}
}

// UITrackingRunLoopMode
2 : <CFRunLoopMode 0x6000034500d0 [0x7fff80617cb0]>{name = UITrackingRunLoopMode, port set = 0x3003, queue = 0x60000215c500, source = 0x60000215c600 (not fired), timer port = 0x3203, 
sources0 = <CFBasicHash 0x600000167cc0 [0x7fff80617cb0]>{type = mutable set, count = 4,
entries =>
    ...
    // iOS 13 下
    4 : <CFRunLoopSource 0x600003a58780 [0x7fff80617cb0]>{signalled = No, valid = Yes, order = -1, context = <CFRunLoopSource context>{version = 0, info = 0x600003a5c180, callout = __handleEventQueue (0x7fff48126d97)}}
    
    // iOS 14 下
    5 : <CFRunLoopSource 0x600000914fc0 [0x7fff8002e8c0]>{signalled = No, valid = Yes, order = -1, context = <CFRunLoopSource context>{version = 0, info = 0x60000073c0d0, callout = __eventQueueSourceCallback (0x7fff24736899)}}
    6 : <CFRunLoopSource 0x600000915140 [0x7fff8002e8c0]>{signalled = No, valid = Yes, order = -2, context = <CFRunLoopSource context>{version = 0, info = 0x600003234240, callout = __eventFetcherSourceCallback (0x7fff2473690b)}}
    ...
}

// kCFRunLoopDefaultMode
4 : <CFRunLoopMode 0x60000345c270 [0x7fff80617cb0]>{name = kCFRunLoopDefaultMode, port set = 0x2103, queue = 0x600002150c00, source = 0x600002150d00 (not fired), timer port = 0x2a03, 
sources0 = <CFBasicHash 0x600000167d20 [0x7fff80617cb0]>{type = mutable set, count = 4,
entries =>
    ...
    // iOS 13 下
    4 : <CFRunLoopSource 0x600003a58780 [0x7fff80617cb0]>{signalled = No, valid = Yes, order = -1, context = <CFRunLoopSource context>{version = 0, info = 0x600003a5c180, callout = __handleEventQueue (0x7fff48126d97)}}
    
    // iOS 14 下
    5 : <CFRunLoopSource 0x600000914fc0 [0x7fff8002e8c0]>{signalled = No, valid = Yes, order = -1, context = <CFRunLoopSource context>{version = 0, info = 0x60000073c0d0, callout = __eventQueueSourceCallback (0x7fff24736899)}}
    6 : <CFRunLoopSource 0x600000915140 [0x7fff8002e8c0]>{signalled = No, valid = Yes, order = -2, context = <CFRunLoopSource context>{version = 0, info = 0x600003234240, callout = __eventFetcherSourceCallback (0x7fff2473690b)}}
    ...
}
...
```
> &emsp;此 source0 的回调函数是 \__handleEventQueue（\__eventQueueSourceCallback、\__eventFetcherSourceCallback），APP 就是通过这个回调函数来处理事件队列的。
>
> &emsp;但是，我们注意到了，\__handleEventQueue 所对应的 source 类型是 0，也就是说它本身不会唤醒休眠的 main RunLoop, main 线程自身在休眠状态中也不可能自己去唤醒自己，那么，系统肯定还有一个子线程，用来接收事件并唤醒 main thread，并将事件传递到 main thread上。

&emsp;确实是有一个子线程，我们将 APP 暂停，就会看到。除了主线程外，系统还为我们自动创建了几个子线程，通过 Xcode 左侧 Debug 导航可看到一个名字比较特殊的线程：com.apple.uikit.eventfetch-thread(7)。 

&emsp;看线程的名字知道，它是 UIKit 所创建的用于接收 event 的线程（以下我们简称为 event fetch thread）。我们打印出 com.apple.uikit.eventfetch-thread 的 RunLoop：
```c++
// 仅有 kCFRunLoopDefaultMode 模式
modes = <CFBasicHash 0x600000c1c030 [0x7fff8002e8c0]>{type = mutable set, count = 1,
entries =>
    2 : <CFRunLoopMode 0x600003978000 [0x7fff8002e8c0]>{name = kCFRunLoopDefaultMode, port set = 0x3003, queue = 0x600002c68000, source = 0x600002c68100 (not fired), timer port = 0x3203, 
    ...
,

sources1 = <CFBasicHash 0x600000c22160 [0x7fff8002e8c0]>{type = mutable set, count = 3,
entries =>
    0 : <CFRunLoopSource 0x6000037703c0 [0x7fff8002e8c0]>{signalled = No, valid = Yes, order = 1, context = <CFMachPort 0x600003574160 [0x7fff8002e8c0]>{valid = Yes, port = 4707, source = 0x6000037703c0, callout = __IOMIGMachPortPortCallback (0x7fff212c068c), context = <CFMachPort context 0x60000297c1c0>}}
    
    // ⬇️⬇️⬇️ __IOHIDEventSystemClientQueueCallback 是我们关注的重点
    1 : <CFRunLoopSource 0x600003770480 [0x7fff8002e8c0]>{signalled = No, valid = Yes, order = 0, context = <CFMachPort 0x600003560000 [0x7fff8002e8c0]>{valid = Yes, port = 4507, source = 0x600003770480, callout = __IOHIDEventSystemClientQueueCallback (0x7fff212ac8e4), context = <CFMachPort context 0x7fa8aec046a0>}}
    
    2 : <CFRunLoopSource 0x600003770540 [0x7fff8002e8c0]>{signalled = No, valid = Yes, order = 0, context = <CFMachPort 0x6000035600b0 [0x7fff8002e8c0]>{valid = Yes, port = 4403, source = 0x600003770540, callout = __IOHIDEventSystemClientAvailabilityCallback (0x7fff212acaa9), context = <CFMachPort context 0x7fa8aec046a0>}}
}
...
```
&emsp;可看到一个回调函数是 \__IOHIDEventSystemClientQueueCallback 的 source1。既然这个是 source1 类型，则是可以被系统通过 mach port 唤醒 com.apple.uikit.eventfetch-thread 线程的 RunLoop， 来执行__IOHIDEventSystemClientQueueCallback 回调的。我们添加符号断点 \__IOHIDEventSystemClientQueueCallback 和  \__handleEventQueue（\__eventQueueSourceCallback），然后触摸屏幕，打印函数调用堆栈。可以发现，会依次调用 \__IOHIDEventSystemClientQueueCallback、\__handleEventQueue（\__eventQueueSourceCallback、__eventFetcherSourceCallback）来处理事件。 
```c++
(lldb) bt
* thread #6, name = 'com.apple.uikit.eventfetch-thread', stop reason = breakpoint 3.1
  * frame #0: 0x00007fff212ac8e4 IOKit`__IOHIDEventSystemClientQueueCallback // ⬅️
    frame #1: 0x00007fff2035c9d8 CoreFoundation`__CFMachPortPerform + 157
    frame #2: 0x00007fff2038fc82 CoreFoundation`__CFRUNLOOP_IS_CALLING_OUT_TO_A_SOURCE1_PERFORM_FUNCTION__ + 41 // ⬅️ source1 执行回调
    frame #3: 0x00007fff2038f023 CoreFoundation`__CFRunLoopDoSource1 + 614
    frame #4: 0x00007fff203894f2 CoreFoundation`__CFRunLoopRun + 2353
    frame #5: 0x00007fff203886d6 CoreFoundation`CFRunLoopRunSpecific + 567
    frame #6: 0x00007fff2082f7b9 Foundation`-[NSRunLoop(NSRunLoop) runMode:beforeDate:] + 209
    frame #7: 0x00007fff2082fa28 Foundation`-[NSRunLoop(NSRunLoop) runUntilDate:] + 72
    frame #8: 0x00007fff24748171 UIKitCore`-[UIEventFetcher threadMain] + 464
    frame #9: 0x00007fff20857e68 Foundation`__NSThread__start__ + 1042
    frame #10: 0x00007fff60c91950 libsystem_pthread.dylib`_pthread_start + 224
    frame #11: 0x00007fff60c8d47b libsystem_pthread.dylib`thread_start + 15
    
(lldb) bt
* thread #1, queue = 'com.apple.main-thread', stop reason = breakpoint 2.1
  * frame #0: 0x000000010fd0bc9d Simple_iOS`-[ViewController touchesBegan:withEvent:](self=0x00007fb73c005560, _cmd="touchesBegan:withEvent:", touches=1 element, event=0x0000600002e7ce40) at ViewController.m:130:5 // ⬅️ VC 的 touchesBegan:withEvent: 回调
    frame #1: 0x00007fff246c4823 UIKitCore`forwardTouchMethod + 321
    frame #2: 0x00007fff246c46d1 UIKitCore`-[UIResponder touchesBegan:withEvent:] + 49
    frame #3: 0x00007fff246d36c9 UIKitCore`-[UIWindow _sendTouchesForEvent:] + 622
    frame #4: 0x00007fff246d57db UIKitCore`-[UIWindow sendEvent:] + 4774
    frame #5: 0x00007fff246af57a UIKitCore`-[UIApplication sendEvent:] + 633
    frame #6: 0x00000001101fe6a6 UIKit`-[UIApplicationAccessibility sendEvent:] + 85
    frame #7: 0x00007fff24740000 UIKitCore`__processEventQueue + 13895
    frame #8: 0x00007fff24736973 UIKitCore`__eventFetcherSourceCallback + 104 // ⬅️ 
    frame #9: 0x00007fff2038f38a CoreFoundation`__CFRUNLOOP_IS_CALLING_OUT_TO_A_SOURCE0_PERFORM_FUNCTION__ + 17 // 
    frame #10: 0x00007fff2038f282 CoreFoundation`__CFRunLoopDoSource0 + 180
    frame #11: 0x00007fff2038e764 CoreFoundation`__CFRunLoopDoSources0 + 248
    frame #12: 0x00007fff20388f2f CoreFoundation`__CFRunLoopRun + 878
    frame #13: 0x00007fff203886d6 CoreFoundation`CFRunLoopRunSpecific + 567
    frame #14: 0x00007fff2bededb3 GraphicsServices`GSEventRunModal + 139
    frame #15: 0x00007fff24690e0b UIKitCore`-[UIApplication _run] + 912
    frame #16: 0x00007fff24695cbc UIKitCore`UIApplicationMain + 101
    frame #17: 0x000000010fd0c4ea Simple_iOS`main(argc=1, argv=0x00007ffedfef5cf8) at main.m:20:12
    frame #18: 0x00007fff202593e9 libdyld.dylib`start + 1
    frame #19: 0x00007fff202593e9 libdyld.dylib`start + 1
(lldb) 
```
&emsp;上面第一段函数堆栈打印是进入 \__IOHIDEventSystemClientQueueCallback 断点时的打印，可看到在 com.apple.uikit.eventfetch-thread 线程的 run loop 中执行 \__CFRunLoopDoSource1，其中回调函数是 \__IOHIDEventSystemClientQueueCallback。

&emsp;接着切换到主线程 com.apple.main-thread 执行 \__CFRunLoopDoSources0，并从 \__processEventQueue 开始处理直到  VC 的 touchesBegan:withEvent: 函数。（Xcode 11 和 Xcode 12 有所区别，大家可以自己手动测试下）

> &emsp;测试可得如下结论：用户触发事件， IOKit.framework 生成一个 IOHIDEvent 事件并由 SpringBoard 接收，SpringBoard 会利用 mach port，产生 source1，来唤醒目标 APP 的 com.apple.uikit.eventfetch-thread 的 RunLoop。Eventfetch thread 会将 main runloop 中 \__handleEventQueue 所对应的 source0 设置为 signalled == Yes 状态，同时唤醒 main RunLoop。mainRunLoop 则调用 \__handleEventQueue 进行事件队列处理。

&emsp;接下来我们顺着刚刚的事件响应的过程再细化一个分支。我们当前的 App 进程接收到事件以后（SpringBoard 只接收按键(锁屏/静音等)、触摸、加速、接近传感器等几种 Event，随后用 mach port 转发给需要的 App 进程），会调用 \__eventFetcherSourceCallback 和 \__eventQueueSourceCallback 进行应用内部分发，此时会对事件做一个细化，会把 IOHIDEvent 处理并包装成 UIEvent 进行处理或分发，其中包括识别 UIGesture/处理屏幕旋转/发送给 UIWindow 等。通常事件比如 UIButton 点击、touchesBegin/Move/End/Cancel 事件都是在这个回调中完成的，那如果是手势的话呢，那么我们的 App 进程如何处理呢？下面我们来一起看一下。
## 手势识别
&emso;我们继续先看一下 [ibireme](https://blog.ibireme.com/2015/05/18/runloop/) 大佬的结论，然后进行证明。（目前自己也不知道怎么学习这一部分内容，不同于前面的只要分析源码就好了，这里就沿着大佬的结论一步一步分析好了。）

> &emsp;当上面的 _UIApplicationHandleEventQueue() 识别了一个手势时，其首先会调用 Cancel 将当前的 touchesBegin/Move/End 系列回调打断。随后系统将对应的 UIGestureRecognizer 标记为待处理。
> 
> &emsp;苹果注册了一个 Observer 监测 BeforeWaiting (Loop 即将进入休眠) 事件，这个Observer的回调函数是 \_UIGestureRecognizerUpdateObserver()，其内部会获取所有刚被标记为待处理的 GestureRecognizer，并执行 GestureRecognizer 的回调。
>
> &emsp;当有 UIGestureRecognizer 的变化(创建/销毁/状态改变)时，这个回调都会进行相应处理。

&emsp;首先我们从 main run loop 中去找一下这个 回调函数是 \_UIGestureRecognizerUpdateObserver 的 CFRunLoopObserverRef。我们继续添加一个 \__IOHIDEventSystemClientQueueCallback 的符号断点和一个 \_UIGestureRecognizerUpdateObserver 的符号断点，然后尝试在屏幕上滑一下（这里我们依然使用模拟器，用真机的话是看不到具体的回调函数名的），当进入断点以后，我们首先在控制台打印 po [NSRunLoop mainRunLoop]。然后在 main run loop 的 kCFRunLoopDefaultMode、UITrackingRunLoopMode、kCFRunLoopCommonModes 模式下都有同一个回调函数是 \_UIGestureRecognizerUpdateObserver 的 CFRunLoopObserver，其 activities 值为 0x20（十进制 32），表示只监听 main run loop 的 kCFRunLoopBeforeWaiting = (1UL << 5) 状态。
```c++
...
observers = (
...
"<CFRunLoopObserver 0x600001bd8320 [0x7fff80617cb0]>{valid = Yes, activities = 0x20, repeats = Yes, order = 0, callout = _UIGestureRecognizerUpdateObserver (0x7fff47c2f06a), context = <CFRunLoopObserver context 0x6000001dc7e0>}",
...
)
...
```
&emsp;可知该 CFRunLoopObserver 监听 main run loop 的 kCFRunLoopBeforeWaiting 事件。每当 main run loop 即将休眠时，该 CFRunLoopObserver 被触发，同时调用回调函数 \_UIGestureRecognizerUpdateObserver。\_UIGestureRecognizerUpdateObserver 会检测当前需要被更新状态的 UIGestureRecognizer（创建，触发，销毁）。

> &emsp;如果有手势被触发，在 \_UIGestureRecognizerUpdateObserver 回调中会借助 UIKit 一个内部类 UIGestureEnvironment 来进行一系列处理。
&emsp;其中会向 APP 的 event queue 中投递一个 gesture event，这个 gesture event 的处理流程应该和上面的事件处理类似的，内部会调用 \__handleEventQueueInternal 处理该 gesture event，并通过 UIKit 内部类 UIGestureEnvironment 来处理这个 gesture event，并最终回调到我们自己所写的 gesture 回调中。[iOS RunLoop完全指南](https://blog.csdn.net/u013378438/article/details/80239686)

&emsp;看一下函数调用栈，验证上面大佬的结论。
```c++
(lldb) bt
* thread #1, queue = 'com.apple.main-thread', stop reason = breakpoint 2.1
  * frame #0: 0x00007fff47c2f06a UIKitCore`_UIGestureRecognizerUpdateObserver // ⬅️ main run loop 的 CFRunLoopObserver 执行其 _UIGestureRecognizerUpdateObserver 回调
    frame #1: 0x00007fff23bd3867 CoreFoundation`__CFRUNLOOP_IS_CALLING_OUT_TO_AN_OBSERVER_CALLBACK_FUNCTION__ + 23 // ⬅️ CFRunLoopObserver 执行
    frame #2: 0x00007fff23bce2fe CoreFoundation`__CFRunLoopDoObservers + 430
    frame #3: 0x00007fff23bce97a CoreFoundation`__CFRunLoopRun + 1514
    frame #4: 0x00007fff23bce066 CoreFoundation`CFRunLoopRunSpecific + 438
    frame #5: 0x00007fff384c0bb0 GraphicsServices`GSEventRunModal + 65
    frame #6: 0x00007fff48092d4d UIKitCore`UIApplicationMain + 1621
    frame #7: 0x000000010201a8fd Simple_iOS`main(argc=1, argv=0x00007ffeedbe5d60) at main.m:76:12
    frame #8: 0x00007fff5227ec25 libdyld.dylib`start + 1
(lldb) bt
* thread #1, queue = 'com.apple.main-thread', stop reason = breakpoint 1.1
  * frame #0: 0x0000000102019dd0 Simple_iOS`-[ViewController tapSelector](self=0x00007ff95af0fdb0, _cmd="tapSelector") at ViewController.m:415:27 // ⬅️ 执行我们的手势回调 tapSelector 
    frame #1: 0x00007fff47c3a347 UIKitCore`-[UIGestureRecognizerTarget _sendActionWithGestureRecognizer:] + 44
    frame #2: 0x00007fff47c4333d UIKitCore`_UIGestureRecognizerSendTargetActions + 109
    frame #3: 0x00007fff47c409ea UIKitCore`_UIGestureRecognizerSendActions + 298
    frame #4: 0x00007fff47c3fd17 UIKitCore`-[UIGestureRecognizer _updateGestureForActiveEvents] + 757
    frame #5: 0x00007fff47c31eda UIKitCore`_UIGestureEnvironmentUpdate + 2706
    frame #6: 0x00007fff47c3140a UIKitCore`-[UIGestureEnvironment _deliverEvent:toGestureRecognizers:usingBlock:] + 467
    frame #7: 0x00007fff47c3117f UIKitCore`-[UIGestureEnvironment _updateForEvent:window:] + 200
    frame #8: 0x00007fff480d04b0 UIKitCore`-[UIWindow sendEvent:] + 4574 // ⬅️ UIWindow
    frame #9: 0x00007fff480ab53b UIKitCore`-[UIApplication sendEvent:] + 356 // ⬅️ UIApplication
    frame #10: 0x0000000102578bd4 UIKit`-[UIApplicationAccessibility sendEvent:] + 85
    frame #11: 0x00007fff4812c71a UIKitCore`__dispatchPreprocessedEventFromEventQueue + 6847
    frame #12: 0x00007fff4812f1e0 UIKitCore`__handleEventQueueInternal + 5980 // ⬅️ 处理该 gesture event
    frame #13: 0x00007fff23bd4471 CoreFoundation`__CFRUNLOOP_IS_CALLING_OUT_TO_A_SOURCE0_PERFORM_FUNCTION__ + 17 // ⬅️ source0 回调
    frame #14: 0x00007fff23bd439c CoreFoundation`__CFRunLoopDoSource0 + 76
    frame #15: 0x00007fff23bd3bcc CoreFoundation`__CFRunLoopDoSources0 + 268
    frame #16: 0x00007fff23bce87f CoreFoundation`__CFRunLoopRun + 1263
    frame #17: 0x00007fff23bce066 CoreFoundation`CFRunLoopRunSpecific + 438
    frame #18: 0x00007fff384c0bb0 GraphicsServices`GSEventRunModal + 65
    frame #19: 0x00007fff48092d4d UIKitCore`UIApplicationMain + 1621
    frame #20: 0x000000010201a8fd Simple_iOS`main(argc=1, argv=0x00007ffeedbe5d60) at main.m:76:12
    frame #21: 0x00007fff5227ec25 libdyld.dylib`start + 1
(lldb)
```
## 界面刷新
> &emsp;当在操作 UI 时，比如改变了 Frame、更新了 UIView/CALayer 的层次时，或者手动调用了 UIView/CALayer 的 setNeedsLayout/setNeedsDisplay 方法后，这个 UIView/CALayer 就被标记为待处理，并被提交到一个全局的容器去。
> 
> &emsp;苹果注册了一个 Observer 监听 BeforeWaiting(即将进入休眠) 和 Exit (即将退出 Loop) 事件，回调去执行一个很长的函数：\_ZN2CA11Transaction17observer_callbackEP19__CFRunLoopObservermPv()。这个函数里会遍历所有待处理的 UIView/CAlayer 以执行实际的绘制和调整，并更新 UI 界面。
> 
> &emsp;这个函数内部的调用栈大概是这样的：
```c++
_ZN2CA11Transaction17observer_callbackEP19__CFRunLoopObservermPv()
QuartzCore:CA::Transaction::observer_callback:
    CA::Transaction::commit();
        CA::Context::commit_transaction();
            CA::Layer::layout_and_display_if_needed();
                CA::Layer::layout_if_needed();
                    [CALayer layoutSublayers];
                        [UIView layoutSubviews];
                CA::Layer::display_if_needed();
                    [CALayer display];
                        [UIView drawRect];
```
&emsp;在控制台打印 main run loop，在其 kCFRunLoopDefaultMode、UITrackingRunLoopMode、kCFRunLoopCommonModes 模式下都有同一个回调函数是 \_ZN2CA11Transaction17observer_callbackEP19__CFRunLoopObservermPv 的 CFRunLoopObserver，其 activities 值为 0xa0（kCFRunLoopBeforeWaiting | kCFRunLoopExit），表示只监听 main run loop 的休眠前和退出状态。其 order = 2000000 比上面的手势识别的 order = 0 的 CFRunLoopObserver 的优先级要低。
```c++
    "<CFRunLoopObserver 0x600001bd88c0 [0x7fff80617cb0]>{valid = Yes, activities = 0xa0, repeats = Yes, order = 2000000, callout = _ZN2CA11Transaction17observer_callbackEP19__CFRunLoopObservermPv (0x7fff2b0c046e), context = <CFRunLoopObserver context 0x0>}"
```
> &emsp;当我们需要界面刷新，如 UIView/CALayer 调用了 setNeedsLayout/setNeedsDisplay，或更新了 UIView 的 frame，或 UI 层次。 
其实，系统并不会立刻就开始刷新界面，而是先提交 UI 刷新请求，再等到下一次 main run loop 循环时，集中处理（集中处理的好处在于可以合并一些重复或矛盾的 UI 刷新）。而这个实现方式，则是通过监听 main run loop 的 before waitting 和 Exit 通知实现的。

&emsp;\_ZN2CA11Transaction17observer_callbackEP19__CFRunLoopObservermPv 内部会调用 CA::Transaction::observer_callback(__CFRunLoopObserver*, unsigned long, void*) 在该函数中，会将所有的界面刷新请求提交，刷新界面，以及调用相关回调。
```c++
(lldb) bt
* thread #1, queue = 'com.apple.main-thread', stop reason = breakpoint 1.1
  * frame #0: 0x00007fff2b0c046e QuartzCore`CA::Transaction::observer_callback(__CFRunLoopObserver*, unsigned long, void*)
    frame #1: 0x00007fff23bd3867 CoreFoundation`__CFRUNLOOP_IS_CALLING_OUT_TO_AN_OBSERVER_CALLBACK_FUNCTION__ + 23
    frame #2: 0x00007fff23bce2fe CoreFoundation`__CFRunLoopDoObservers + 430
    frame #3: 0x00007fff23bce97a CoreFoundation`__CFRunLoopRun + 1514
    frame #4: 0x00007fff23bce066 CoreFoundation`CFRunLoopRunSpecific + 438
    frame #5: 0x00007fff384c0bb0 GraphicsServices`GSEventRunModal + 65
    frame #6: 0x00007fff48092d4d UIKitCore`UIApplicationMain + 1621
    frame #7: 0x00000001019348fd Simple_iOS`main(argc=1, argv=0x00007ffeee2cbd60) at main.m:76:12
    frame #8: 0x00007fff5227ec25 libdyld.dylib`start + 1
(lldb) 
```
## 卡顿监测
&emsp;卡顿的呈现方式大概可以理解为我们触摸屏幕时系统回馈不及时或者连续滑动屏幕时肉眼可见的掉帧，回归到程序层面的话可知这些感知的来源都是主线程，而分析有没有卡顿发生则可以从主线程的 run loop 入手，可以通过监听 main run loop 的活动变化，从而发现主线程的调用方法堆栈中是否某些方法执行时间过长而导致了 run loop 循环周期被拉长继而发生了卡顿，所以监测卡顿的方案是：**通过监控 main run loop 从 kCFRunLoopBeforeSources（或者 kCFRunLoopBeforeTimers） 到 kCFRunLoopAfterWaiting  的活动变化所用时间是否超过了我们预定的阈值进而判断是否出现了卡顿，当出现卡顿时可以读出当前函数调用堆栈帮助我们来分析代码问题。**
```c++
#import "HCCMonitor.h"
#include <mach/mach_time.h>

@interface HCCMonitor () {
    // 往主线程添加一个 CFRunLoopObserverRef
    CFRunLoopObserverRef runLoopObserver;
}

@property (nonatomic, assign) int timeoutCount;
@property (nonatomic, strong) dispatch_semaphore_t dispatchSemaphore;
@property (nonatomic, assign) CFRunLoopActivity runLoopActivity;

@end

@implementation HCCMonitor

// 单例
+ (instancetype)shareInstance {
    static HCCMonitor *instance = nil;
    static dispatch_once_t onceToken;
    
    dispatch_once(&onceToken, ^{
        instance = [[HCCMonitor alloc] init];
    });
    return instance;
}

// 开始监听
- (void)beginMonitor {
    if (runLoopObserver) {
        return;
    }
    
    self.dispatchSemaphore = dispatch_semaphore_create(0); // Dispatch Semaphore 保证同步
    // 创建一个观察者
    CFRunLoopObserverContext context = {0, (__bridge void *)self, NULL, NULL};
    runLoopObserver = CFRunLoopObserverCreate(kCFAllocatorDefault, kCFRunLoopAllActivities, YES, 0, &runLoopObserverCallBack, &context);
    // 将观察者添加到主线程 run loop 的 common 模式下的观察中
    CFRunLoopAddObserver(CFRunLoopGetMain(), runLoopObserver, kCFRunLoopCommonModes);
    
    // 创建子线程监控
    dispatch_async(dispatch_get_global_queue(0, 0), ^{
        while (YES) {
            long semaphoreWait = dispatch_semaphore_wait(self.dispatchSemaphore, dispatch_time(DISPATCH_TIME_NOW, 20 * NSEC_PER_MSEC));
            
            // 当 semaphoreWait 值不为 0 时，表示 semaphoreWait 等待时间超过了 dispatch_time(DISPATCH_TIME_NOW, 20 * NSEC_PER_MSEC)
            if (semaphoreWait != 0) {
                if (!self->runLoopObserver) {
                    self.timeoutCount = 0;
                    self.dispatchSemaphore = 0;
                    self.runLoopActivity = 0;
                    return ;
                }
                
                // 监测 kCFRunLoopBeforeSources 或者 kCFRunLoopAfterWaiting 两个活动状态变化，即一旦发现进入睡眠前的 kCFRunLoopBeforeSources 状态，
                // 或者唤醒后的状态 kCFRunLoopAfterWaiting，在设置的时间阈值内一直没有变化，即可判定为卡顿。
    
                // 在 run loop 的本次循环中，从 kCFRunLoopBeforeSources 到 kCFRunLoopBeforeWaiting 处理了 source/timer/block 的事情，如果时间花的太长必然导致主线程卡顿。
                // 从 kCFRunLoopBeforeWaiting 到 kCFRunLoopAfterWaiting 状态，如果本次唤醒花了太多时间也会必然造成卡顿。
                
                if (self.runLoopActivity == kCFRunLoopBeforeSources || self.runLoopActivity == kCFRunLoopAfterWaiting) {
                    if (++self.timeoutCount < 3) {
                        continue;
                    }
                    
                    // 如果连续超过了 3 次则表示监测到卡顿 
                    NSLog(@"🔠🔠🔠 卡顿发生了...");
                    // 打印当前的函数堆栈，（也可直接上传到服务器，方便我们统计分析原因）
                    NSLog(@"🗂🗂 %@", [NSThread callStackSymbols]);
                }
            }

            self.timeoutCount = 0;
        } // end while
    });
}

// 结束监听
- (void)endMonitor {
    if (!runLoopObserver) {
        return;
    }
    
    CFRunLoopRemoveObserver(CFRunLoopGetMain(), runLoopObserver, kCFRunLoopCommonModes);
    CFRelease(runLoopObserver);
    runLoopObserver = NULL;
}

int count = 0;
// runLoopObserver 的回调事件
static void runLoopObserverCallBack(CFRunLoopObserverRef observer, CFRunLoopActivity activity, void *info) {
    HCCMonitor *lagMonitor = (__bridge HCCMonitor *)info;
    
    // 记录 main run loop 本次活动状态变化
    lagMonitor.runLoopActivity = activity;
    // 向子线程 while 循环中的 self.dispatchSemaphore 发送信号，结束等待继续向下执行
    dispatch_semaphore_signal(lagMonitor.dispatchSemaphore);

    // 下面是一些不同状态变化之间的时间跨度打印，可以帮助我们观察。
    ++count;
    static uint64_t beforeTimersTSR = 0;
    static uint64_t beforeSourcesTSR = 0;
    static uint64_t beforeWaitingTSR = 0;
    static uint64_t afterWaitingTSR = 0;
    
    // uint64_t ns_at = (uint64_t)((__CFTSRToTimeInterval(beforeTimersTSR)) * 1000000000ULL);
    // NSLog(@"✳️✳️✳️ beforeTimersTSR %llu", beforeTimersTSR);
    
    switch (activity) {
        case kCFRunLoopEntry:
            count = 0;
            NSLog(@"🤫 - %d kCFRunLoopEntry 即将进入: %@", count, CFRunLoopCopyCurrentMode(CFRunLoopGetCurrent()));
            break;
        case kCFRunLoopBeforeTimers:
            NSLog(@"⏳ - %d kCFRunLoopBeforeTimers 即将处理 timers", count);
            beforeTimersTSR = mach_absolute_time();
            // NSLog(@"🔂 AfterWaiting~Timer: %llu", beforeTimersTSR - afterWaitingTSR);
            break;
        case kCFRunLoopBeforeSources:
            NSLog(@"💦 - %d kCFRunLoopBeforeSources 即将处理 sources", count);
            beforeSourcesTSR = mach_absolute_time();
            // NSLog(@"🔂 Timer~Source: %llu", beforeSourcesTSR - beforeTimersTSR);
            break;
        case kCFRunLoopBeforeWaiting:
            count = 0; // 每次 run loop 即将进入休眠时，count 置为 0，可表示一轮 run loop 循环结束
            NSLog(@"🛏 - %d kCFRunLoopBeforeWaiting 即将进入休眠", count);
            beforeWaitingTSR = mach_absolute_time();
            // NSLog(@"🔂 Source~BeforeWaiting %llu", beforeWaitingTSR - beforeSourcesTSR);
            break;
        case kCFRunLoopAfterWaiting:
            NSLog(@"🦍 - %d kCFRunLoopAfterWaiting 即将从休眠中醒来", count);
            afterWaitingTSR = mach_absolute_time();
            // NSLog(@"🔂 BeforeWaiting~AfterWaiting: %llu", afterWaitingTSR - beforeWaitingTSR);
            break;
        case kCFRunLoopExit:
            count = 0;
            NSLog(@"🤫 - %d kCFRunLoopExit 即将退出: %@", count, CFRunLoopCopyCurrentMode(CFRunLoopGetCurrent()));
            break;
        case kCFRunLoopAllActivities:
            NSLog(@"🤫 kCFRunLoopAllActivities");
            break;
    }
}

@end
```
&emsp;首先给 main run loop 添加一个 CFRunLoopObserverRef runLoopObserver 来帮助我们监听主线程的活动状态变化，然后创建一条子线程在子线程里面用一个死循环 while(YES) 来等待着主线程的状态变化，等待的方式是在子线程的 while 循环内部用 `long dispatch_semaphore_wait(dispatch_semaphore_t dsema, dispatch_time_t timeout)` 函数，它的 timeout 参数刚好可以设置一个我们想要观察的 main run loop 的不同的活动状态变化之间的时间长度，当 dispatch_semaphore_wait 函数返回非 0 值时表示等待的时间超过了 timeout，所以我们只需要关注 dispatch_semaphore_wait 函数返回非 0 值的情况。我们使用 HCCMonitor 的单例对象在 runLoopObserver 的回调函数和子线程之间进行 "传值"，当 runLoopObserver 的回调函数执行时我们调用 dispatch_semaphore_signal 函数结束子线程 while 循环中的 dispatch_semaphore_wait 等待，同时使用单例对象的 runLoopActivity 成员变量记录 main run loop 本次变化的活动状态值，然后如果子线程的 while 循环中连续三次出现 kCFRunLoopBeforeSources 或者  kCFRunLoopAfterWaiting 状态变化等待超时了，那么就可认为是主线程卡顿了。

&emsp;监测 kCFRunLoopBeforeSources 或者 kCFRunLoopAfterWaiting 两个活动状态变化，即一旦发现进入睡眠前的 kCFRunLoopBeforeSources 状态，或者唤醒后的状态 kCFRunLoopAfterWaiting，在设置的时间阈值内一直没有变化，即可判定为卡顿。

&emsp;在 run loop 的本次循环中，从 kCFRunLoopBeforeSources 到 kCFRunLoopBeforeWaiting 处理了 source/timer/block 的事情，如果时间花的太长必然导致主线程卡顿。从 kCFRunLoopBeforeWaiting 到 kCFRunLoopAfterWaiting 状态，如果本次唤醒花了太多时间也会必然造成卡顿。

## 参考链接
**参考链接:🔗**
+ [runloop 源码](https://opensource.apple.com/tarballs/CF/)
+ [Run Loops 官方文档](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/Multithreading/RunLoopManagement/RunLoopManagement.html#//apple_ref/doc/uid/10000057i-CH16-SW1)
+ [iOS RunLoop完全指南](https://blog.csdn.net/u013378438/article/details/80239686)
+ [iOS源码解析: runloop的底层数据结构](https://juejin.cn/post/6844904090330234894)
+ [iOS源码解析: runloop的运行原理](https://juejin.cn/post/6844904090166624270)
+ [深入理解RunLoop](https://blog.ibireme.com/2015/05/18/runloop/)
+ [iOS底层学习 - 深入RunLoop](https://juejin.cn/post/6844903973665636360)
+ [一份走心的runloop源码分析](https://cloud.tencent.com/developer/article/1633329)
+ [NSRunLoop](https://www.cnblogs.com/wsnb/p/4753685.html)
+ [iOS刨根问底-深入理解RunLoop](https://www.cnblogs.com/kenshincui/p/6823841.html)
+ [RunLoop总结与面试](https://www.jianshu.com/p/3ccde737d3f3)
+ [Runloop-实际开发你想用的应用场景](https://juejin.cn/post/6889769418541252615)
+ [RunLoop 源码阅读](https://juejin.cn/post/6844903592369848328#heading-17)
+ [do {...} while (0) 在宏定义中的作用](https://www.cnblogs.com/lanxuezaipiao/p/3535626.html)
+ [CFRunLoop 源码学习笔记(CF-1151.16)](https://www.cnblogs.com/chengsh/p/8629605.html)
+ [操作系统大端模式和小端模式](https://www.cnblogs.com/wuyuankun/p/3930829.html)
+ [CFBag](https://nshipster.cn/cfbag/)
+ [mach_absolute_time 使用](https://www.cnblogs.com/zpsoe/p/6994811.html)
+ [iOS 探讨之 mach_absolute_time](https://blog.csdn.net/yanglei3kyou/article/details/86679177)
+ [iOS多线程——RunLoop与GCD、AutoreleasePool你要知道的iOS多线程NSThread、GCD、NSOperation、RunLoop都在这里](https://cloud.tencent.com/developer/article/1089330)
+ [Mach原语：一切以消息为媒介](https://www.jianshu.com/p/284b1777586c?nomobile=yes)
+ [操作系统双重模式和中断机制和定时器概念](https://blog.csdn.net/zcmuczx/article/details/79937023)
+ [iOS底层原理 RunLoop 基础总结和随心所欲掌握子线程 RunLoop 生命周期 --(9)](http://www.cocoachina.com/articles/28800)
+ [从NSRunLoop说起](https://zhuanlan.zhihu.com/p/63184073)
+ [runloop 与autorelase对象、Autorelease Pool 在什么时候释放](https://blog.csdn.net/leikezhu1981/article/details/51246684)
+ [内存管理：autoreleasepool与runloop](https://www.jianshu.com/p/d769c1653347)
+ [Objective-C的AutoreleasePool与Runloop的关联](https://blog.csdn.net/zyx196/article/details/50824564)
+ [iOS开发-Runloop中自定义输入源Source](https://blog.csdn.net/shengpeng3344/article/details/104518051)
+ [IOHIDFamily](http://iphonedevwiki.net/index.php/IOHIDFamily)
+ [iOS卡顿监测方案总结](https://juejin.cn/post/6844903944867545096)
