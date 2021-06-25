# iOS 从源码解析Run Loop (七)：mach msg 解析
> &emsp;经过前面 NSPort 内容的学习，我们大概对 port 在线程通信中的使用有一点模糊的概念了。macOS/iOS 中利用 Run Loop 实现了自动释放池、延迟回调、触摸事件、屏幕刷新等等功能，关于 Run Loop 的各种应用我们下篇再进行全面的学习。那么本篇我们学习一下 Mach。

&emsp;Run Loop 最核心的事情就是保证线程在没有消息时休眠以避免系统资源占用，有消息时能够及时唤醒。Run Loop 的这个机制完全依靠系统内核来完成，具体来说是苹果操作系统核心组件 Darwin 中的 Mach 来完成的。**Mach 与 BSD、File System、Mach、Networking 共同位于 Kernel and Device Drivers 层。**

&emsp;在 Mach 中，所有的东西都是通过自己的对象实现的，进程、线程和虚拟内存都被称为 “对象”，和其他架构不同， Mach 的对象间不能直接调用，只能通过消息传递的方式实现对象间的通信。“消息”（mach msg）是 Mach 中最基础的概念，消息在两个端口 (mach port) 之间传递，这就是 Mach 的 IPC (进程间通信) 的核心。

&emsp;Mach 是 Darwin 的核心，可以说是内核的核心，提供了进程间通信（IPC）、处理器调度等基础服务。在 Mach 中，进程、线程间的通信是以消息（mach msg）的方式来完成的，而消息则是在两个 mach port 之间进行传递（或者说是通过 mach port 进行消息的传递）（这也正是 Source1 之所以称之为 Port-based Source 的原因，因为它就是依靠 mach msg 发送消息到指定的 mach port 来唤醒 run loop）。

&emsp;关于 Darwin 的信息可以看 ibireme 大佬的文章，5 年前的文章放在今天依然是目前能看到的关于 run loop 最深入的文章，可能这就是大佬吧！：[RunLoop 的底层实现](https://blog.ibireme.com/2015/05/18/runloop/#core)

&emsp;（概念理解起来可能过于干涩特别是内核什么的，如果没有学习过操作系统相关的知识可能更是只识字不识意，那么下面我们从源码中找线索，从函数的使用上找线索，慢慢的理出头绪来。）

## mach_msg_header_t
&emsp;Mach 消息相关的数据结构：mach_msg_base_t、mach_msg_header_t、mach_msg_body_t 定义在 <mach/message.h> 头文件中：
```c++
typedef struct{
    mach_msg_header_t       header;
    mach_msg_body_t         body;
} mach_msg_base_t;

typedef struct{
    mach_msg_bits_t       msgh_bits;
    mach_msg_size_t       msgh_size; // 消息传递数据大小
    
    // typedef __darwin_mach_port_t mach_port_t; =>
    // typedef __darwin_mach_port_name_t __darwin_mach_port_t; /* Used by mach */
    // typedef __darwin_natural_t __darwin_mach_port_name_t; /* Used by mach */
    // typedef unsigned int __darwin_natural_t;
    // mach_port_t 实际上是一个整数类型，用于标记端口。
    
    mach_port_t           msgh_remote_port;
    mach_port_t           msgh_local_port;
    
    // mach_port_name_t 是 mach port 的本地标识
    mach_port_name_t      msgh_voucher_port;
    
    mach_msg_id_t         msgh_id;
} mach_msg_header_t;

typedef struct{
    mach_msg_size_t msgh_descriptor_count;
} mach_msg_body_t;

#define MACH_PORT_NULL 0  /* intentional loose typing */
#define MACH_PORT_DEAD ((mach_port_name_t) ~0)
```
&emsp;每条消息均以 message header 开头。mach_msg_header_t 中存储了 mach msg 的基本信息，包括端口信息等。如本地端口 msgh_local_port 和远程端口 msgh_remote_port，mach msg 的传递方向在 header 中已经非常明确了。
+ msgh_remote_port 字段指定消息的目的地。它必须指定为一个有效的发送端口或有一次发送权限的端口。
+ msgh_local_port 字段指定 "reply port"。通常，此字段带有一次发送权限，接收方将使用该一次发送权限来回复消息。It may carry the values MACH_PORT_NULL, MACH_PORT_DEAD, a send-once right, or a send right. 
+ msgh_voucher_port 字段指定一个 Mach 凭证端口。除 MACH_PORT_NULL 或 MACH_PORT_DEAD 之外，只能传递对内核实现的 Mach Voucher 内核对象的发送权限。
+ 消息原语（message primitives）不解释 msgh_id 字段。它通常携带指定消息格式或含义的信息。

&emsp;一条 Mach 消息实际上就是一个二进制数据包 (BLOB)，其头部定义了当前端口 local_port 和目标端口 remote_port，发送和接受消息是通过同一个 API（mach_msg） 进行的，其 option 标记了消息传递的方向。

## mach_msg
&emsp;首先看一下 mach_msg 函数声明:
```c++
/*
 *    Routine: mach_msg
 *    Purpose:
 *        Send and/or receive a message.  If the message operation
 *        is interrupted, and the user did not request an indication
 *        of that fact, then restart the appropriate parts of the
 *        operation silently (trap version does not restart).
 */
 // 发送和/或接收消息。如果消息操作被中断，并且用户没有请求指示该事实，
 // 则静默地重新启动操作的相应部分（trap 版本不会重新启动）。
__WATCHOS_PROHIBITED __TVOS_PROHIBITED
extern mach_msg_return_t mach_msg(mach_msg_header_t *msg,
                                  mach_msg_option_t option,
                                  mach_msg_size_t send_size,
                                  mach_msg_size_t rcv_size,
                                  mach_port_name_t rcv_name,
                                  mach_msg_timeout_t timeout,
                                  mach_port_name_t notify);
```
> &emsp;为了实现消息的发送和接收，mach_msg 函数实际上是调用了一个 Mach 陷阱 (trap)，即函数 mach_msg_trap，陷阱这个概念在 Mach 中等同于系统调用。当在用户态调用 mach_msg_trap 时会触发陷阱机制，切换到内核态；内核态中内核实现的 mach_msg 函数会完成实际的工作。
> 
> &emsp;run loop 的核心就是一个 mach_msg ，run loop 调用这个函数去接收消息，如果没有别人发送 port 消息过来，内核会将线程置于等待状态。例如在模拟器里跑起一个 iOS 的 App，然后在 App 静止时点击暂停，会看到主线程调用栈是停留在 mach_msg_trap 这个地方。[深入理解RunLoop](https://blog.ibireme.com/2015/05/18/runloop/)

&emsp;（mach_msg 函数可以设置 timeout 参数，如果在 timeout 到来之前没有读到 msg，当前线程的 run loop 会处于休眠状态。）

&emsp;消息的发送和接收统一使用 `mach_msg` 函数，而 `mach_msg` 的本质是调用了 `mach_msg_trap`，这相当于一个系统调用，会触发内核态与用户态的切换。

&emsp;点击 App 图标，App 启动完成后处于静止状态（一般如果没有 timer 需要一遍一遍执行的话），此时主线程的 run loop 会进入休眠状态，通过在主线程的 run loop 添加 CFRunLoopObserverRef 在回调函数中可看到主线程的 run loop 的最后活动状态是 kCFRunLoopBeforeWaiting，此时点击 Xcode 控制台底部的 Pause program execution 按钮，从 Xcode 左侧的 Debug navigator 可看到主线程的调用栈停在了 mach_msg_trap，然后在控制台输入 bt 后回车，可看到如下调用栈，看到 mach_msg_trap 是由 mach_msg 函数调用的。
```c++
(lldb) bt
* thread #1, queue = 'com.apple.main-thread', stop reason = signal SIGSTOP
  * frame #0: 0x00007fff60c51e6e libsystem_kernel.dylib`mach_msg_trap + 10 // ⬅️ mach_msg_trap
    frame #1: 0x00007fff60c521e0 libsystem_kernel.dylib`mach_msg + 60
    frame #2: 0x00007fff2038e9bc CoreFoundation`__CFRunLoopServiceMachPort + 316 // ⬅️ __CFRunLoopServiceMachPort 进入休眠
    frame #3: 0x00007fff203890c5 CoreFoundation`__CFRunLoopRun + 1284 // ⬅️ __CFRunLoopRun
    frame #4: 0x00007fff203886d6 CoreFoundation`CFRunLoopRunSpecific + 567 // ⬅️  CFRunLoopRunSpecific
    frame #5: 0x00007fff2bededb3 GraphicsServices`GSEventRunModal + 139
    frame #6: 0x00007fff24690e0b UIKitCore`-[UIApplication _run] + 912 // ⬅️ main run loop 启动
    frame #7: 0x00007fff24695cbc UIKitCore`UIApplicationMain + 101
    frame #8: 0x0000000107121d4a Simple_iOS`main(argc=1, argv=0x00007ffee8addcf8) at main.m:20:12
    frame #9: 0x00007fff202593e9 libdyld.dylib`start + 1
    frame #10: 0x00007fff202593e9 libdyld.dylib`start + 1
(lldb) 
```
&emsp;通过上篇 \__CFRunLoopRun 函数的学习，已知 run loop 进入休眠状态时会调用 \__CFRunLoopServiceMachPort 函数，该函数内部即调用了 `mach_msg` 相关的函数操作使得系统内核的状态发生改变：用户态切换至内核态。
## mach_msg_trap
&emsp;mach_msg 内部实际上是调用了 mach_msg_trap  系统调用。陷阱（trap）是操作系统层面的基本概念，用于操作系统状态的更改，如触发内核态与用户态的切换操作。trap 通常由异常条件触发，如断点、除 0 操作、无效内存访问等。

&emsp;当 run loop 休眠时，通过 mach port 消息可以唤醒 run loop，那么从 run loop 创建开始到现在我们在代码层面的学习过程中遇到过哪些 mach port 呢？下面我们就一起回顾一下。最典型的应该当数 \__CFRunLoop 中的 \_wakeUpPort 和 \__CFRunLoopMode 中的 \_timerPort。

## \__CFRunLoop-\_wakeUpPort
&emsp;struct \__CFRunLoop 结构体的成员变量 \__CFPort \_wakeUpPort 应该是我们在 run loop 里见到的第一个 mach port 了，创建 run loop 对象时即会同时创建一个 mach_port_t 实例为 \_wakeUpPort 赋值。\_wakeUpPort 被用于在 CFRunLoopWakeUp 函数中调用 \__CFSendTrivialMachMessage 函数时作为其参数来唤醒 run loop。\_wakeUpPort 声明类型是 \__CFPort，实际类型是 mach_port_t。
```c++
struct __CFRunLoop {
    ...
    // typedef mach_port_t __CFPort;
    __CFPort _wakeUpPort; // used for CFRunLoopWakeUp 用于 CFRunLoopWakeUp 函数
    ...
};
```
&emsp;在前面 NSMachPort 的学习中我们已知 `+(NSPort *)portWithMachPort:(uint32_t)machPort;` 函数中 `machPort` 参数原始即为 mach_port_t 类型。

&emsp;当为线程创建 run loop 对象时会直接对 run loop 的 \_wakeUpPort 成员变量进行初始化。在 `__CFRunLoopCreate` 函数中初始化 \_wakeUpPort。
```c++
static CFRunLoopRef __CFRunLoopCreate(pthread_t t) {
    ...
    // __CFPortAllocate 创建具有发送和接收权限的 mach_port_t
    loop->_wakeUpPort = __CFPortAllocate();
    if (CFPORT_NULL == loop->_wakeUpPort) HALT; // 创建失败的话会直接 crash
    ...
}
```
&emsp;在 \__CFRunLoopDeallocate run loop 销毁函数中会释放 \_wakeUpPort。
```c++
static void __CFRunLoopDeallocate(CFTypeRef cf) {
    ...
    // __CFPortFree 内部是调用 mach_port_destroy(mach_task_self(), rl->_wakeUpPort) 函数
    __CFPortFree(rl->_wakeUpPort);
    rl->_wakeUpPort = CFPORT_NULL;
    ...
}
```
&emsp;全局搜索 \_wakeUpPort 看到相关的结果仅有：创建、释放、被插入到 run loop mode 的 \_portSet 和 CFRunLoopWakeUp 函数唤醒 run loop 时使用，下面我们看一下以 run loop 对象的 \_wakeUpPort 端口为参调用 \__CFSendTrivialMachMessage 函数来唤醒 run loop 的过程。

### CFRunLoopWakeUp
&emsp;`CFRunLoopWakeUp` 函数是用来唤醒 run loop 的，唤醒的方式是以 run loop 对象的 \_wakeUpPort 端口为参数调用 `__CFSendTrivialMachMessage` 函数
```c++
void CFRunLoopWakeUp(CFRunLoopRef rl) {
    CHECK_FOR_FORK();
    // This lock is crucial to ignorable wakeups, do not remove it.
    // 此锁对于可唤醒系统至关重要，请不要删除它。
    
    // CRRunLoop 加锁
    __CFRunLoopLock(rl);
    
    // 如果 rl 被标记为忽略唤醒的状态，则直接解锁并返回
    if (__CFRunLoopIsIgnoringWakeUps(rl)) {
        __CFRunLoopUnlock(rl);
        return;
    }
    
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
    kern_return_t ret;
    
    /* We unconditionally try to send the message, since we don't want to lose a wakeup,
    but the send may fail if there is already a wakeup pending, since the queue length is 1. */
    // 因为我们不想丢失唤醒，所以我们无条件地尝试发送消息，但由于队列长度为 1，因此如果已经存在唤醒等待，则发送可能会失败。
    
    // 向 rl->_wakeUpPort 端口发送消息，内部是调用了 mach_msg
    ret = __CFSendTrivialMachMessage(rl->_wakeUpPort, 0, MACH_SEND_TIMEOUT, 0);
    if (ret != MACH_MSG_SUCCESS && ret != MACH_SEND_TIMED_OUT) CRASH("*** Unable to send message to wake up port. (%d) ***", ret);
    
#elif DEPLOYMENT_TARGET_WINDOWS
    SetEvent(rl->_wakeUpPort);
#endif

    // CFRunLoop 解锁
    __CFRunLoopUnlock(rl);
}
```
### \__CFSendTrivialMachMessage
&emsp;`__CFSendTrivialMachMessage` 函数内部主要是调用 `mach_msg` 函数向 port 发送消息。
```c++
static uint32_t __CFSendTrivialMachMessage(mach_port_t port, uint32_t msg_id, CFOptionFlags options, uint32_t timeout) {
    // 记录 mach_msg 函数返回结果
    kern_return_t result;
    
    // 构建 mach_msg_header_t 用于发送消息
    mach_msg_header_t header;
    
    // #define MACH_MSG_TYPE_COPY_SEND 19 // Must hold send right(s) 必须持有发送权限
    // #define MACH_MSGH_BITS(remote, local) ((remote) | ((local) << 8)) // 位操作
    header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, 0);
    
    header.msgh_size = sizeof(mach_msg_header_t);
    
    header.msgh_remote_port = port; // 远程端口
    
    header.msgh_local_port = MACH_PORT_NULL;
    header.msgh_id = msg_id; // 0 
    
    // #define MACH_SEND_TIMEOUT 0x00000010 /* timeout value applies to send */ 超时值应用于发送
    // 调用 mach_msg 函数发送消息
    result = mach_msg(&header, MACH_SEND_MSG|options, header.msgh_size, 0, MACH_PORT_NULL, timeout, MACH_PORT_NULL);
    
    if (result == MACH_SEND_TIMED_OUT) mach_msg_destroy(&header);
    
    return result;
}
```
&emsp;可看到 `CFRunLoopWakeUp` 函数的功能就是调用 mach_msg 函数向 run loop 的 \_wakeUpPort 端口发送消息来唤醒 run loop。

## \__CFRunLoopMode-\_timerPort
&emsp;\_timerPort 是 \__CFRunLoopMode 的一个成员变量。在 macOS 下同时支持 dispatch_source 和 mk 构建 timer，在 iOS 下则只支持使用 mk。这里我们只关注 \_timerPort。我们在 Cocoa Foundation 层会通过手动创建并添加计时器 NSTimer 到  run loop 的指定 run loop mode 下，同样在 Core Foundation 层会通过创建 CFRunLoopTimerRef 实例并把它添加到 run loop 的指定 run loop mode 下，内部实现是则是把 CFRunLoopTimerRef 实例添加到 run loop mode 的 \_timers 集合中，当 \_timers 集合中的计时器需要执行时则正是通过 \_timerPort 来唤醒 run loop，且 run loop mode 的 \_timers 集合中的所有计时器共用这一个 \_timerPort。

&emsp;这里我们可以做一个验证，我们为主线程添加一个 CFRunLoopOberver 观察 main run loop 的状态变化和一个 1 秒执行一次的 NSTimer。程序运行后可看到一直如下的重复打印：(代码过于简单，这里就不贴出来了)
```c++
...
⏰⏰⏰ timer 回调...
🎯... kCFRunLoopBeforeTimers
🎯... kCFRunLoopBeforeSources
🎯... kCFRunLoopBeforeWaiting
🎯... kCFRunLoopAfterWaiting
⏰⏰⏰ timer 回调...
🎯... kCFRunLoopBeforeTimers
🎯... kCFRunLoopBeforeSources
🎯... kCFRunLoopBeforeWaiting
🎯... kCFRunLoopAfterWaiting
...
```
&emsp;计时器到了触发时间唤醒 run loop（kCFRunLoopAfterWaiting）执行计时器的回调，计时器回调执行完毕后 run loop 又进入休眠状态（kCFRunLoopBeforeWaiting）然后到达下次计时器触发时间时 run loop 再次被唤醒，如果不手动停止计时器的话则会这样一直无限重复下去。 

&emsp;下面我们看一下其中唤醒 run loop 的关键 \_timerPort 的创建和使用逻辑。
```c++
#if DEPLOYMENT_TARGET_MACOSX
#define USE_DISPATCH_SOURCE_FOR_TIMERS 1
#define USE_MK_TIMER_TOO 1
#else
#define USE_DISPATCH_SOURCE_FOR_TIMERS 0
#define USE_MK_TIMER_TOO 1
#endif

struct __CFRunLoopMode {
    ...
#if USE_DISPATCH_SOURCE_FOR_TIMERS
    dispatch_source_t _timerSource;
    dispatch_queue_t _queue;
    Boolean _timerFired; // set to true by the source when a timer has fired
    Boolean _dispatchTimerArmed;
#endif

#if USE_MK_TIMER_TOO
    mach_port_t _timerPort;
    Boolean _mkTimerArmed;
#endif
    ...
};
```
&emsp;首先是创建同样也是跟着 CFRunLoopModeRef 一起创建的。在 \__CFRunLoopFindMode 函数中当创建 CFRunLoopModeRef 时会同时创建一个 mach_port_t 实例并赋值给 \_timerPort。并同时会把 \_timerPort 插入到 CFRunLoopModeRef 的 \_portSet 中。
```c++
static CFRunLoopModeRef __CFRunLoopFindMode(CFRunLoopRef rl, CFStringRef modeName, Boolean create) {
    ...
#if USE_MK_TIMER_TOO
    // 为 _timerPort 赋值
    rlm->_timerPort = mk_timer_create();
    // 同时也把 _timerPort 插入 _portSet 集合中
    ret = __CFPortSetInsert(rlm->_timerPort, rlm->_portSet);
    // 如果插入失败则 crash
    if (KERN_SUCCESS != ret) CRASH("*** Unable to insert timer port into port set. (%d) ***", ret);
#endif
    
    // 把 rl 的 _wakeUpPort 插入到 _portSet 中
    ret = __CFPortSetInsert(rl->_wakeUpPort, rlm->_portSet);
    // 插入失败的话会 crash 
    if (KERN_SUCCESS != ret) CRASH("*** Unable to insert wake up port into port set. (%d) ***", ret);
    ...
}
```
&emsp;在 \__CFRunLoopModeDeallocate run loop mode 的销毁函数中同样会销毁 \_timerPort。
```c++
static void __CFRunLoopModeDeallocate(CFTypeRef cf) {
    ...
#if USE_MK_TIMER_TOO
    // 调用 mk_timer_destroy 函数销毁 _timerPort
    if (MACH_PORT_NULL != rlm->_timerPort) mk_timer_destroy(rlm->_timerPort);
#endif
    ...
}
```
&emsp;可看到 \_timerPort 和 CFRunLoopModeRef 一同创建一同销毁的。

&emsp;看到 \_timerPort 创建时调用了 mk_timer_create 函数，销毁时调用了  mk_timer_destroy 函数，大概这种 mk_timer 做前缀的函数还是第一次见到其中还有两个比较重要的函数：[mk_timer.h](https://opensource.apple.com/source/xnu/xnu-1228/osfmk/mach/mk_timer.h.auto.html)

```c++
#if USE_MK_TIMER_TOO
extern mach_port_name_t mk_timer_create(void); // 创建
extern kern_return_t mk_timer_destroy(mach_port_name_t name); // 销毁
extern kern_return_t mk_timer_arm(mach_port_name_t name, AbsoluteTime expire_time); // 在指定时间发送消息
extern kern_return_t mk_timer_cancel(mach_port_name_t name, AbsoluteTime *result_time); // 取消未发送的消息
...
#endif
```
+ `mk_timer_arm(mach_port_t, expire_time)` 在 expire_time 的时候给指定了 mach_port（\_timerPort） 的 mach_msg 发送消息。
+ `mk_timer_cancel(mach_port_t, &result_time)` 取消 mk_timer_arm 注册的消息。

&emsp;同一个 run loop mode 下的多个 timer 共享同一个 \_timerPort，这是一个循环的流程：注册 timer(mk_timer_arm)—接收 timer(mach_msg)—根据多个 timer 计算离当前最近的下次 handle 时间—注册 timer(mk_timer_arm)。

&emsp;在使用 CFRunLoopAddTimer 添加 timer 时的调用堆栈是：
```c++
CFRunLoopAddTimer
__CFRepositionTimerInMode
    __CFArmNextTimerInMode
        mk_timer_arm
```
&emsp;mach_msg 收到 timer 事件时的调用堆栈是：
```c++
__CFRunLoopRun
__CFRunLoopDoTimers
    __CFRunLoopDoTimer
        CALL_OUT_Timer
            __CFArmNextTimerInMode
                mk_timer_arm 
```
&emsp;至此 mach_msg 和一些 run loop 相关的 mach port 都看完了，mach_msg 依靠用户态和内核态的切换完成了 run loop 的休眠与唤醒，唤醒操作则是离不开向指定的 mach port 发送消息。那么 mach_msg 就看到这里吧，下篇我们开始学习系统中 run loop 的各种应用。⛽️⛽️

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
