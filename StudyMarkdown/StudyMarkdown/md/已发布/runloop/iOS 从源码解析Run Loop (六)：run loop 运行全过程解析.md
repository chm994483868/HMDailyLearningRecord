# iOS 从源码解析Run Loop (六)：run loop 运行全过程解析

> &emsp;前面几篇算是把 run loop 相关的数据结构都看完了，也算是把 run loop 开启运行的前期数据都准备好了，下面我们开始正式进入 run loop 的整个的运行过程的探索和学习。⛽️⛽️

&emsp;查看 CFRunLoop.h 文件，看到涉及 run loop 运行的函数有两个 `CFRunLoopRun` 和 `CFRunLoopRunInMode` 下面我们跟着源码学习一下这两个函数。

## CFRunLoopRun/CFRunLoopRunInMode
&emsp;`CFRunLoopRun` 函数同 NSRunLoop 的 `- (void)run;` 函数，无限期地以其默认模式运行当前线程的 CFRunLoop 对象。当前线程的运行循环将以默认模式运行，直到使用 `CFRunLoopStop` 停止 run loop 或将所有 Sources 和 Timers 从默认运行循环模式中移除为止。run loop 可以递归运行，你可以从任何 run loop 调用中调用 `CFRunLoopRun` 函数，并在当前线程的调用堆栈上创建嵌套的 run loop 激活。

&emsp;`CFRunLoopRunInMode` 在特定模式下运行当前线程的 CFRunLoop 对象。
+ `mode`：以运行循环模式运行。模式可以是任意 CFString。尽管运行循环模式需要至少包含一个源或计时器才能运行，但是你无需显式创建运行循环模式。
+ `seconds`：运行 run loop 的时间长度。如果为 0，则返回之前仅运行循环一次；如果有多个源或计时器准备立即触发，那么无论 `returnAfterSourceHandled` 的值如何，都将仅触发一个（如果一个是 version 0 source，则可能触发两个）。
+ `returnAfterSourceHandled`：一个标志，指示 run loop 是否应在处理一个源之后退出。如果为 false，则运行循环将继续处理事件，直到经过 `seconds`。

&emsp;`CFRunLoopRunInMode` 函数返回一个值，指示 run loop 退出的原因。

&emsp;Run loops 可以递归运行。你可以从任何运行循环调用中调用 `CFRunLoopRunInMode` 函数，并在当前线程的调用堆栈上创建嵌套的运行循环激活。在调用中可以运行的模式不受限制。你可以创建另一个在任何可用的运行循环模式下运行的运行循环激活，包括调用堆栈中已经运行的任何模式。

&emsp;在指定的条件下，运行循环退出并返回以下值:
+ `kCFRunLoopRunFinished` 运行循环模式没有源或计时器。（当 run loop 对象被标记为正在销毁时也会返回 kCFRunLoopRunFinished）
+ `kCFRunLoopRunStopped` 运行循环已使用 `CFRunLoopStop` 函数停止。
+ `kCFRunLoopRunTimedOut` 时间间隔秒数（seconds）过去了。
+ `kCFRunLoopRunHandledSource` 已处理源。此退出条件仅适用于 `returnAfterSourceHandled` 为 `true` 时。

&emsp;不能为 `mode` 参数指定 `kCFRunLoopCommonModes` 常量。运行循环总是以特定模式运行。只有在配置运行循环观察者时，以及仅在希望该观察者以多种模式运行的情况下，才能指定 common mode。

&emsp;下面是 `CFRunLoopRun` 和 `CFRunLoopRunInMode` 函数的定义:
```c++
/* Reasons for CFRunLoopRunInMode() to Return */
// CFRunLoopRunInMode 函数返回的原因
enum {
    kCFRunLoopRunFinished = 1,
    kCFRunLoopRunStopped = 2,
    kCFRunLoopRunTimedOut = 3, 
    kCFRunLoopRunHandledSource = 4 
};

void CFRunLoopRun(void) {    /* DOES CALLOUT */
    int32_t result;
    do {
        // 调用 CFRunLoopRunSpecific 函数，以 kCFRunLoopDefaultMode 启动当前线程的 run loop，运行时间传入的是 10^10 秒（2777777 个小时），
        // returnAfterSourceHandled 参数传入的是 false，指示 run loop 是在处理一个源之后不退出并持续处理事件。
        result = CFRunLoopRunSpecific(CFRunLoopGetCurrent(), kCFRunLoopDefaultMode, 1.0e10, false);
        
        CHECK_FOR_FORK();
    } while (kCFRunLoopRunStopped != result && kCFRunLoopRunFinished != result);
}

SInt32 CFRunLoopRunInMode(CFStringRef modeName,
                          CFTimeInterval seconds,
                          Boolean returnAfterSourceHandled) { 
    CHECK_FOR_FORK();
    
    // 以指定的 run loop mode 启动当前线程的 run loop，且可以自定义 seconds 和 returnAfterSourceHandled 参数的值
    return CFRunLoopRunSpecific(CFRunLoopGetCurrent(), modeName, seconds, returnAfterSourceHandled);
}

```
&emsp;看到 `CFRunLoopRun` 函数是内部是一个 do while 循环，内部调用了 `CFRunLoopRunSpecific` 函数当其返回值是 `kCFRunLoopRunTimedOut` 或 `kCFRunLoopRunHandledSource` 时一直持续进行 do while 循环。（根据之前的文章记得只有当前 run loop mode 没有 sources0/sources1/timers/block 时当前线程的 NSRunLoop 对象调用 `-(void)run;` 函数，run loop 会启动失败或者说是启动后就立即退出了，其他情况就是一直无限循环，所以想这里的 do while 结束循环的条件不是应该只有 `kCFRunLoopRunFinished != result` 吗，即使是调用了 `CFRunLoopStop` 函数，结束的也只是本次 run loop 并不会导致 do while 退出...但是现在则是多了 `kCFRunLoopRunStopped != result`）

&emsp;看到 `CFRunLoopRun` 和 `CFRunLoopRunInMode` 函数内部都是调用了 `CFRunLoopRunSpecific` 函数，第一个参数都是直接使用 `CFRunLoopGetCurrent` 函数获取当前线程的 run loop，然后是第二个参数 `CFStringRef modeName` 则是传入 run loop mode 的名字，而非直接传入 CFRunLoopMode 实例，第三个参数则是 `CFTimeInterval seconds` 指示 run loop 需要运行多久。

### CFRunLoopRunSpecific
&emsp;`CFRunLoopRunSpecific` 函数内部会调用 `__CFRunLoopRun` 函数，然后可以把 `result = __CFRunLoopRun(rl, currentMode, seconds, returnAfterSourceHandled, previousMode);` 此行的调用看作一个分界线。行前是，则是首先判断 `rl` 是否被标记为正在销毁，如果是的话则直接返回 kCFRunLoopRunFinished，否则继续往下执行，会根据 `modeName` 从 `rl` 的 `_modes` 中找到其对应的 `CFRunLoopModeRef`，如果未找到或者 `CFRunLoopModeRef` 的 sources0/sources1/timers/block 为空，则也是直接返回  kCFRunLoopRunFinished。然后是修改 `rl` 的 `_perRunData` 和 `_currentMode` 同时还会记录之前的旧值，此时一切准备就绪，在调用之前会根据 `rl` 的 `_currentMode` 的 `_observerMask` 判断是否需要回调 run loop observer 观察者来告诉它们 run loop 要进入 kCFRunLoopEntry 状态了，然后调用 `__CFRunLoopRun` 函数正式启动 run loop。

&emsp;`__CFRunLoopRun` 函数返回后则是，首先根据 `rl` 的 `_currentMode` 的 `_observerMask` 判断是否需要回调 run loop observer 观察者来告诉它们 run loop 要进入 kCFRunLoopExit 状态了。然后是把 run loop 对象恢复到之前的 `_perRunData` 和 `_currentMode`（处理 run loop 的嵌套）。

&emsp;上面描述的可能不太清晰，看下面的代码和注释已经极其清晰了。
```c++
SInt32 CFRunLoopRunSpecific(CFRunLoopRef rl,
                            CFStringRef modeName,
                            CFTimeInterval seconds,
                            Boolean returnAfterSourceHandled) {     /* DOES CALLOUT */
    CHECK_FOR_FORK();
    
    // 从 rl 的 _cfinfo 字段中取 rl 是否正在销毁的标记值，如果是的话，则直接返回 kCFRunLoopRunFinished
    if (__CFRunLoopIsDeallocating(rl)) return kCFRunLoopRunFinished;
    
    // CFRunLoop 加锁
    __CFRunLoopLock(rl);
    
    // 调用 __CFRunLoopFindMode 函数从 rl 的 _modes 中找到名字是 modeName 的 run loop mode，
    // 如果找不到的话第三个参数传的是 false 则不进行新建 run loop mode，则直接返回 NULL。 
    //（CFRunLoopMode 加锁）
    CFRunLoopModeRef currentMode = __CFRunLoopFindMode(rl, modeName, false);
    
    // 如果 currentMode 为 NULL 或者 currentMode 里面是空的不包含 sources0/sources1/timers/block 则 return 
    if (NULL == currentMode || __CFRunLoopModeIsEmpty(rl, currentMode, rl->_currentMode)) {
        Boolean did = false;
        
        // 如果 currentMode 存在，则进行 CFRunLoopMode 解锁，
        // 对应了上面 __CFRunLoopFindMode(rl, modeName, false) 调用内部的 CFRunLoopMode 加锁 
        if (currentMode) __CFRunLoopModeUnlock(currentMode);
        
        // CFRunLoop 解锁
        __CFRunLoopUnlock(rl);
        
        // 返回 kCFRunLoopRunFinished
        return did ? kCFRunLoopRunHandledSource : kCFRunLoopRunFinished;
    }
    
    // __CFRunLoopPushPerRunData 函数内部是修改 rl 的 _perRunData 字段的各成员变量的值，并返回之前的 _perRunData，
    //（函数内部修改 _perRunData 的值其实是在标记 run loop 不同状态）
    //（这里的 previousPerRun 是用于下面的 __CFRunLoopRun 函数调用返回后，当前的 run loop 对象要回到之前的 _perRunData）。
    volatile _per_run_data *previousPerRun = __CFRunLoopPushPerRunData(rl);
    
    // previousMode 记录 rl 当前的 run loop mode，相比入参传入的 modeName 取得的 run loop mode 而言，它是之前的 run loop mode，
    // 这个 previousMode 主要用于下面的那行 __CFRunLoopRun 函数调用返回后，当前的 run loop 对象要回到之前的 run loop mode。
    //（同上面的 previousPerRun 数据，也要把当前的 run loop 对象回到之前的 _perRunData 数据的状态）
    CFRunLoopModeRef previousMode = rl->_currentMode;
    
    // 更新 rl 的 _currentMode 为入参 modeName 对应的 run loop mode 
    rl->_currentMode = currentMode;
    
    // 临时变量 result，用于当函数返回时记录 run loop 不同的退出原因
    int32_t result = kCFRunLoopRunFinished;
    
    // 判断如果 currentMode 的 _observerMask 字段中包含 kCFRunLoopEntry 的值（_observerMask 内记录了需要观察 run loop 哪些状态变化），
    // 则告诉 currentMode 的 run loop observer 发生了一个 run loop 即将进入循环的状态变化。 
    if (currentMode->_observerMask & kCFRunLoopEntry) __CFRunLoopDoObservers(rl, currentMode, kCFRunLoopEntry);
    
    // 启动 run loop，__CFRunLoopRun 函数超长，可能是看源码以来最长的一个函数，下面会逐行进行细致的分析
    // ♻️♻️♻️♻️♻️♻️
    result = __CFRunLoopRun(rl, currentMode, seconds, returnAfterSourceHandled, previousMode);
    
    // ⬆️⬆️⬆️ __CFRunLoopRun 函数好像也是不会返回的，当它返回时就代表当前的 run loop 要退出了。 
    
    // 同上的 kCFRunLoopEntry 进入循环的回调，这里则是退出 run loop 的回调。
    // 如果 currentMode 的 _observerMask 中包含 kCFRunLoopExit 的值，
    // 即 run loop observer 需要观察 run loop 的 kCFRunLoopExit 退出状态切换
    if (currentMode->_observerMask & kCFRunLoopExit ) __CFRunLoopDoObservers(rl, currentMode, kCFRunLoopExit);
    
    // CFRunLoopMode 解锁
    __CFRunLoopModeUnlock(currentMode);
    
    // 销毁 rl 当前的 _perRunData，并把 previousPerRun 重新赋值给 rl 的 _perRunData 
    __CFRunLoopPopPerRunData(rl, previousPerRun);
    
    // 回到之前的 _currentMode 
    rl->_currentMode = previousMode;
    
    // CFRunLoop 解锁
    __CFRunLoopUnlock(rl);
    
    // 返回 result 结果
    return result;
}
```
&emsp;这里需要注意的一个点是 `CFRunLoopRunSpecific` 函数最后又把之前的 `previousPerRun` 和 `previousMode` 重新赋值给 run loop 的 `_perRunData` 和 `_currentMode`，它们正是用来处理 run loop 的嵌套运行的。下面看一下 `CFRunLoopRunSpecific` 函数内部调用的一些函数。

#### \__CFRunLoopIsDeallocating
&emsp;`__CFRunLoopIsDeallocating` 函数用于判断 `rl` 是否被标记为正在销毁。该值记录在 `_cfinfo` 字段中。
```c++
CF_INLINE Boolean __CFRunLoopIsDeallocating(CFRunLoopRef rl) {
    return (Boolean)__CFBitfieldGetValue(((const CFRuntimeBase *)rl)->_cfinfo[CF_INFO_BITS], 2, 2);
}
```

#### \__CFRunLoopModeIsEmpty
&emsp;`__CFRunLoopModeIsEmpty` 函数用于判断 `rlm` 中是否没有 sources0/sources1/timers/block，在 `CFRunLoopRunSpecific` 函数内部调用 `__CFRunLoopModeIsEmpty` 函数时这里的三个参数要区分一下：`rl` 是 run loop 对象指针，然后 `rlm` 是 `rl` 即将要用此 `rlm` 启动，然后 `previousMode` 则是 `rl` 当前的 `_currentMode` 字段的值。（其中 rl 的 block 链表在一轮循环中，block 执行结束后会被移除并释放，那么下一轮 run loop 循环进来，再去判断 block 链就是空的了，那么这次 run loop 是不是会以 kCFRunLoopRunFinished 原因而退出）
```c++
// expects rl and rlm locked 进入 __CFRunLoopModeIsEmpty 函数调用前 rl 和 rlm 的 _lock 都已经加锁了
static Boolean __CFRunLoopModeIsEmpty(CFRunLoopRef rl, CFRunLoopModeRef rlm, CFRunLoopModeRef previousMode) {
    CHECK_FOR_FORK();
    
    // 如果 rlm 为 NULL 则直接返回 true
    if (NULL == rlm) return true;
    
#if DEPLOYMENT_TARGET_WINDOWS
    if (0 != rlm->_msgQMask) return false;
#endif
    
    // pthread_main_np() 是判断当前是否是主线程，主线程的 run loop 是程序启动时就启动了，
    // 这些事情是系统自己处理的，我们开发者能做的是控制自己创建的子线程的 run loop，所以当我们自己调用 __CFRunLoopModeIsEmpty 函数时，
    // 一定是在我们自己的子线程内，此时 libdispatchQSafe 的值就一定都是 false 的。
    
    // #define HANDLE_DISPATCH_ON_BASE_INVOCATION_ONLY 0
    Boolean libdispatchQSafe = pthread_main_np() && 
                               ((HANDLE_DISPATCH_ON_BASE_INVOCATION_ONLY && NULL == previousMode) ||
                               (!HANDLE_DISPATCH_ON_BASE_INVOCATION_ONLY && 0 == _CFGetTSD(__CFTSDKeyIsInGCDMainQ)));
                               
    // 1. 当前在主线程，且 0 == _CFGetTSD(__CFTSDKeyIsInGCDMainQ)(在当前线程的 TSD 中获取 __CFTSDKeyIsInGCDMainQ 的值)
    // 2. rl 是主线程的 run loop，
    // 3. rl 的 _commonModes 包含 rlm->_name，
    // 满足以上 3 个条件则返回 false，表示 rlm 不是空的，rl 在此入参的 rlm 下可以运行
    if (libdispatchQSafe && (CFRunLoopGetMain() == rl) && CFSetContainsValue(rl->_commonModes, rlm->_name)) return false; // represents the libdispatch main queue
    
    // 下面三条分别判断 rlm 的 _sources0 集合不为空、_sources1 集合不为空、_timers 数组不为空，
    // 只要有任一不为空即可直接表示 rlm 不是空的，rl 可以在此入参 rlm 下运行。
    if (NULL != rlm->_sources0 && 0 < CFSetGetCount(rlm->_sources0)) return false;
    if (NULL != rlm->_sources1 && 0 < CFSetGetCount(rlm->_sources1)) return false;
    if (NULL != rlm->_timers && 0 < CFArrayGetCount(rlm->_timers)) return false;
    
    // 下面还有一点判断 run loop mode 不为空的依据，判断 rl 的 block 链表中包含的 block 的 _mode 是否和入参的 rlm 的 name 相同。
    // 这里是一个新知识点，前面我们说过无数次如果 run loop mode 的 sources0/sources1/timers/block 为空时 run loop 则不能在此 mode 下运行，
    // 下面涉及到了一个新的点，还有一种情况下，此情况对应了 run loop block 链表。
    
    // 这里要注意一下: _mode 的值可能是一个字符串也可能是一个集合，当是一个字符串时表示一个 run loop mode 的 name，
    // 当是一个集合时包含的是一组 run loop mode 的 name。
    
    // struct _block_item {
    //     struct _block_item *_next; // 下一个节点
    //     CFTypeRef _mode; // CFString or CFSet 可表示在一个 mode 下执行或者在多种 mode 下都可以执行
    //     void (^_block)(void); // 当前的 block 
    // };
    
    // 取得 rl 的 block 链表的头节点
    struct _block_item *item = rl->_blocks_head;
    
    // 开始遍历 block 的链表，但凡找到一个可在 rlm 下执行的 block 节点，都表示 rlm 不为空，run loop 可在此 mode 下运行
    while (item) {
        struct _block_item *curr = item;
        item = item->_next;
        Boolean doit = false;
        
        // curr 的 _mode 是字符串或者集合
        if (CFStringGetTypeID() == CFGetTypeID(curr->_mode)) {
            // 是字符串时，rlm 的 _name 是否和它相等，或者 curr 的 _mode 是 kCFRunLoopCommonModes，
            // 判断 rlm 的 _name 是否被包含在 rl 的 _commonModes 中
            doit = CFEqual(curr->_mode, rlm->_name) || 
                   (CFEqual(curr->_mode, kCFRunLoopCommonModes) &&
                    CFSetContainsValue(rl->_commonModes, rlm->_name));
        } else {
            // 是集合时，同上判断 curr 的 _mode 集合内是否包含 rlm 的 _name，或者 curr 的 _mode 集合包含 kCFRunLoopCommonModes，
            // 那么判断 rl 的 _commonModes 是否包含 rlm 的 _name
            doit = CFSetContainsValue((CFSetRef)curr->_mode, rlm->_name) || 
                   (CFSetContainsValue((CFSetRef)curr->_mode, kCFRunLoopCommonModes) &&
                    CFSetContainsValue(rl->_commonModes, rlm->_name));
        }
        
        // 如果 doit 为真，即 rl 的 block 链表中的 block 可执行的模式包含 rlm。
        if (doit) return false;
    }
    
    return true;
}
```
&emsp;`__CFRunLoopModeIsEmpty` 函数内部主要用于判断 souces0/source1/timers 是否为空，同时还有判断 rl  的 block 链表中包含的 block 是否能在指定的 rlm 下执行。（其中 block 链表的知识点我们后面会详细接触分析）

&emsp;`__CFRunLoopPushPerRunData` 和 `__CFRunLoopPopPerRunData` 函数我们前面已经看过了，这里不再重复展开了。

##### pthread_main_np()
&emsp;`pthread_main_np` 是一个宏定义，它最终是调用 `_NS_pthread_main_np` 函数，判断当前线程是否是主线程。（主线程全局只有一条，应该是一个全局变量）
```c++
#define pthread_main_np _NS_pthread_main_np

static pthread_t __initialPthread = { NULL, 0 };
CF_EXPORT int _NS_pthread_main_np() {
    // 取得当前线程
    pthread_t me = pthread_self();
    
    // __initialPthread 是一个静态全局变量，
    // 此函数第一次调用应该是在主线程里调用，然后给 __initialPthread 赋值以后，__initialPthread 就固定表示主线程了。
    if (NULL == __initialPthread.p) {
        __initialPthread.p = me.p;
        __initialPthread.x = me.x;
    }
    
    // 判断线程是否相等
    return (pthread_equal(__initialPthread, me));
}
```
#### \__CFRunLoopDoObservers
&emsp;`__CFRunLoopDoObservers` 函数是一个极重要的函数，它用于回调 run loop 发生了状态变化。

&emsp;当 run loop 的状态将要（注意这里是将要、将要、将要... kCFRunLoopExit 则除外，退出回调是真的退出完成以后的回调）发生变化时，首先根据 run loop 当前的 run loop mode 的 `_observerMask` 是否包含了此状态的变化，那么就可以调用 `__CFRunLoopDoObservers` 函数执行 run loop 状态变化的回调，我们在此状态变化里面可以做很多重要的事情，后面学习 run loop 的使用场景时我们再详细学习。（这里回顾一下前面看过的 run loop 都有哪些状态变化：即将进入 run loop、即将处理 source 事件、即将处理 timer 事件、即将休眠、休眠即将结束、run loop 退出）
```c++
// CFRunLoopRunSpecific 函数内回调了 kCFRunLoopEntry 和 kCFRunLoopExit 两个状态变化
// if (currentMode->_observerMask & kCFRunLoopEntry ) __CFRunLoopDoObservers(rl, currentMode, kCFRunLoopEntry);
// if (currentMode->_observerMask & kCFRunLoopExit ) __CFRunLoopDoObservers(rl, currentMode, kCFRunLoopExit);

/* rl is locked, rlm is locked on entrance and exit */ 

/* 
 * 进入 __CFRunLoopDoObservers 函数前 rl 和 rlm 的 _lock 都已经加锁了，
 * 在 __CFRunLoopDoObservers 函数内部当需要执行回调时，会对 rl 和 rlm 进行解锁。
 * 然后在回调函数执行完成后，在 __CFRunLoopDoObservers 函数即将返回之前会重新对 rl 和 rlm 进行加锁。
 */

// 声明
static void __CFRunLoopDoObservers() __attribute__((noinline));
// 实现
static void __CFRunLoopDoObservers(CFRunLoopRef rl, CFRunLoopModeRef rlm, CFRunLoopActivity activity) {    /* DOES CALLOUT */
    CHECK_FOR_FORK();

    // 取出 rlm 的 _observers 数组中的元素数量
    CFIndex cnt = rlm->_observers ? CFArrayGetCount(rlm->_observers) : 0;
    
    // 如果 run loop observer 数量小于 1，则直接返回
    if (cnt < 1) return;

    /* Fire the observers */
    
    // #define STACK_BUFFER_DECL(T, N, C) T N[C]
    // CFRunLoopObserverRef buffer[cnt]，即申请一个长度是 cnt/1 的 CFRunLoopObserverRef 数组 
    STACK_BUFFER_DECL(CFRunLoopObserverRef, buffer, (cnt <= 1024) ? cnt : 1);
    
    // 如果 cnt 小于等于 1024，则 collectedObservers 是一个 CFRunLoopObserverRef buffer[cnt]，
    // 否则 collectedObservers = (CFRunLoopObserverRef *)malloc(cnt * sizeof(CFRunLoopObserverRef))。
    CFRunLoopObserverRef *collectedObservers = (cnt <= 1024) ? buffer : (CFRunLoopObserverRef *)malloc(cnt * sizeof(CFRunLoopObserverRef));
    
    // obs_cnt 用于记录 collectedObservers 收集了多少个 CFRunLoopObserverRef
    CFIndex obs_cnt = 0;
    
    // 遍历 rlm 的 _observers，把能触发的 CFRunLoopObserverRef 都收集在 collectedObservers 中。
    for (CFIndex idx = 0; idx < cnt; idx++) {
        // 取出 rlm 的 _observers 中指定下标的 CFRunLoopObserverRef
        CFRunLoopObserverRef rlo = (CFRunLoopObserverRef)CFArrayGetValueAtIndex(rlm->_observers, idx);
        
        // 1. 判断 rlo 观察的状态 _activities 中包含入参 activity
        // 2. rlo 是有效的
        // 3. rlo 的 _cfinfo 字段中的位，当前不是正在执行回调的状态
        // 同时满足上面三个条件时，把 rlo 收集在 collectedObservers 数组中，用了 CFRetain(rlo)，即 collectedObservers 持有 rlo
        
        if (0 != (rlo->_activities & activity) && __CFIsValid(rlo) && !__CFRunLoopObserverIsFiring(rlo)) {
            collectedObservers[obs_cnt++] = (CFRunLoopObserverRef)CFRetain(rlo);
        }
    }
    
    // 执行 run loop observer 的回调函数前，需要把 rlm 和 rl 解锁
    __CFRunLoopModeUnlock(rlm);
    __CFRunLoopUnlock(rl);
    
    // 遍历 collectedObservers 执行每个 CFRunLoopObserverRef 的回调函数
    for (CFIndex idx = 0; idx < obs_cnt; idx++) {
        // 根据下标取出 CFRunLoopObserverRef
        CFRunLoopObserverRef rlo = collectedObservers[idx];
        
        // CFRunLoopObserver 加锁
        __CFRunLoopObserverLock(rlo);
        
        // 如果 rlo 是有效的，则进入 if 执行回调，否则 rlo 解锁，进入下次循环
        if (__CFIsValid(rlo)) {
        
            // 取出 rlo 是否重复观察 run loop 的状态变化的标记
            Boolean doInvalidate = !__CFRunLoopObserverRepeats(rlo);
            
            // 设置 rlo 的为正在执行状态
            __CFRunLoopObserverSetFiring(rlo);
            
            // CFRunLoopObserver 解锁
            __CFRunLoopObserverUnlock(rlo);
            
            // 执行回调函数，函数名超长，而且都是大写，其中的 OBSERVER 标记这是一个 rlo 的回调，
            // 不过其内部实现很简单，就是把 rlo 的 _context 的 info 和 activity 做参数然后调用 rlo 的 _callout 函数。
            //（activity 标记了此次 run loop 的状态变化的状态值） 
            __CFRUNLOOP_IS_CALLING_OUT_TO_AN_OBSERVER_CALLBACK_FUNCTION__(rlo->_callout,
                                                                          rlo,
                                                                          activity,
                                                                          rlo->_context.info);
            
            // 如果 rlo 是仅观察 run loop 状态变化一次的话，此时观察完毕了，则需要把 rlo 作废，
            //（CFRunLoopObserverInvalidate 内部有有一系列的 rlo 的成员变量的释放操作）
            if (doInvalidate) {
                CFRunLoopObserverInvalidate(rlo);
            }
            
            // 设置 rlo 的已经结束正在执行状态
            __CFRunLoopObserverUnsetFiring(rlo);
        } else {
            // CFRunLoopObserver 解锁
            __CFRunLoopObserverUnlock(rlo);
        }
        
        // 释放 rlo，这里的释放对应了上面收集时的 CFRetain 
        CFRelease(rlo);
    }
    
    // 执行完 run loop observer 的回调函数后，需要再把 rlm 和 rl 加锁
    __CFRunLoopLock(rl);
    __CFRunLoopModeLock(rlm);

    // 如果 collectedObservers 是调用 malloc 申请的，则调用 free 释放其内存空间
    if (collectedObservers != buffer) free(collectedObservers);
}
```
&emsp;run loop observer 的回调函数。
```c++
static void __CFRUNLOOP_IS_CALLING_OUT_TO_AN_OBSERVER_CALLBACK_FUNCTION__() __attribute__((noinline));
static void __CFRUNLOOP_IS_CALLING_OUT_TO_AN_OBSERVER_CALLBACK_FUNCTION__(CFRunLoopObserverCallBack func,
                                                                          CFRunLoopObserverRef observer,
                                                                          CFRunLoopActivity activity,
                                                                          void *info) {
    // 就是简单的带着参数调用 func 函数                                                                      
    if (func) {
        func(observer, activity, info);
    }
    
    asm __volatile__(""); // thwart tail-call optimization
}
```
&emsp;`__CFRunLoopDoObservers` 函数至此就分析完毕了，注释已经极其清晰了，这里就不总结了。

&emsp;现在 `CFRunLoopRunSpecific` 函数内部调用的其它函数就只剩下 `__CFRunLoopRun` 函数了...超长...!

### \__CFRunLoopRun
&emsp;`__CFRunLoopRun` 函数是 run loop 真正的运行函数，超长（并且里面包含了一些在 windows 平台下的代码）。因为其是 run loop 最最核心的函数，下面我们就一行一行看一下吧，耐心看完后相信会对 run loop 能有一个全面彻底的认识。
```c++
/* rl, rlm are locked on entrance and exit */
// 同上面的 __CFRunLoopDoObservers 函数

/* 
* 进入 __CFRunLoopRun 函数前 rl 和 rlm 的 _lock 都已经加锁了，
* 在 __CFRunLoopRun 函数内部当需要执行回调时，会对 rl 和 rlm 进行解锁，
* 然后在回调函数执行完成后，会重新对 rl 和 rlm 进行加锁。
*/
static int32_t __CFRunLoopRun(CFRunLoopRef rl,
                              CFRunLoopModeRef rlm,
                              CFTimeInterval seconds,
                              Boolean stopAfterHandle,
                              CFRunLoopModeRef previousMode) {
                              
    // mach_absolute_time 返回一个基于系统启动后的时钟嘀嗒数，是一个 CPU/总线 依赖函数。
    // 在 macOS 上可以确保它的行为，并且它包含系统时钟所拥有的全部时间区域，精度达到纳秒级。
    // 时钟嘀嗒数在每次手机重启后，都会重新开始计数，而且 iPhone 锁屏进入休眠之后，tick 也会暂停计数
    uint64_t startTSR = mach_absolute_time();
    
    // 判断 rl 是否已停止，(rl->_perRunData->stopped) ? true : false;
    // rl->_perRunData->stopped 的值为 0x53544F50/0x0，
    // 当值是 0x53544F50 时表示 rl 已经停止，是 0x0 时表示未设置停止标记即非停止状态。
    
    // 如果 rl 是 stop 标记，则把它置为未设置 stop 标记，然后返回 kCFRunLoopRunStopped，
    // 如果 rlm 是 stop 标记，则把它置为未设置 stop 标记，然后返回 kCFRunLoopRunStopped。
    //（这里把当前是停止状态的 rl 和 rlm 的 stop 标记都置为未设置状态，然后返回 kCFRunLoopRunStopped 是一个伏笔...）
    
    if (__CFRunLoopIsStopped(rl)) {
        // 设置 rl->_perRunData->stopped = 0x0 表示未设置停止标记的状态，即表示 rl 是非停止状态。
        __CFRunLoopUnsetStopped(rl);
        
        // 然后直接返回 kCFRunLoopRunStopped
        return kCFRunLoopRunStopped;
    } else if (rlm->_stopped) { // 判断 rlm 的 _stopped 是否标记为 true
        // 如果 _stopped 是 true，则把 _stopped 置为 false，表示 flm 是非停止状态。
        rlm->_stopped = false;
        
        // 然后直接返回 kCFRunLoopRunStopped
        return kCFRunLoopRunStopped;
    }
    
    // 声明一个 mach_port_name_t 类型的局部变量 dispatchPort，
    // 用于记录主队列的端口，只有当前处于主线程时下面才会被赋值为主队列的端口。
    // unsigned int
    // #define MACH_PORT_NULL   0
    // dispatchPort 初始为 0
    mach_port_name_t dispatchPort = MACH_PORT_NULL;
    
    // #define HANDLE_DISPATCH_ON_BASE_INVOCATION_ONLY 0
    // 当前是主线程并且从当前线程的 TSD 中获取 __CFTSDKeyIsInGCDMainQ 得到的是 0 的话 libdispatchQSafe 的值才会为 true。
    Boolean libdispatchQSafe = pthread_main_np() &&
                               ((HANDLE_DISPATCH_ON_BASE_INVOCATION_ONLY && NULL == previousMode) ||
                               (!HANDLE_DISPATCH_ON_BASE_INVOCATION_ONLY && 0 == _CFGetTSD(__CFTSDKeyIsInGCDMainQ)));
                               
    // 1. libdispatchQSafe 为真
    // 2. 入参 rl 是 main run loop
    // 3. 入参 rlm->_name 被 rl->_commonModes 包含
    // 以上三个条件都是真的话，则把主线程主队列的端口号赋值给 dispatchPort 变量，用于当使用 dispatch_async 添加到主队列任务时唤醒 main run loop 执行。
    
    if (libdispatchQSafe && (CFRunLoopGetMain() == rl) && CFSetContainsValue(rl->_commonModes, rlm->_name)) 
        // _dispatch_get_main_queue_port_4CF 用于获取主线程主队列的端口号，然后赋值给 dispatchPort
        dispatchPort = _dispatch_get_main_queue_port_4CF();
   
// 在 rlm 中使用 dispatch_source 构建的 timer
#if USE_DISPATCH_SOURCE_FOR_TIMERS
    mach_port_name_t modeQueuePort = MACH_PORT_NULL;
    
    // run loop mode 创建时，会对 _queue 字段赋初值
    // rlm->_queue = _dispatch_runloop_root_queue_create_4CF("Run Loop Mode Queue", 0);
    
    if (rlm->_queue) {
        // 获取 rlm->_queue 的 port 
        modeQueuePort = _dispatch_runloop_root_queue_get_port_4CF(rlm->_queue);
        
        if (!modeQueuePort) {
            // 如果获取端口失败，则 carsh 描述信息是：无法获取运行循环模式队列的端口
            CRASH("Unable to get port for run loop mode queue (%d)", -1);
        }
    }
#endif

    // GCD timer 是依赖于内核的，所以非常精准，不受 run loop 影响。
    
    // 由 dispatch_suorce 构建计时器
    dispatch_source_t timeout_timer = NULL;
    
    // struct __timeout_context {
    //     dispatch_source_t ds;
    //     CFRunLoopRef rl;
    //     uint64_t termTSR;
    // };
    
    // 为计时器参数 timeout_context 申请内存空间
    struct __timeout_context *timeout_context = (struct __timeout_context *)malloc(sizeof(*timeout_context));
    
    if (seconds <= 0.0) { // instant timeout 立即超时
        // 如果 run loop 运行时间 seconds 参数小于等于 0.0，则立即超时
        seconds = 0.0;
        timeout_context->termTSR = 0ULL;
    } else if (seconds <= TIMER_INTERVAL_LIMIT) { // 大于 0.0 小于 504911232.0 的 run loop 运行时间，其它情况的 seconds 的话都表示永不超时
        // #define TIMER_INTERVAL_LIMIT   504911232.0
        
        // 如果当前是主线程，则 queue = dispatch_get_global_queue(qos_class_main(), DISPATCH_QUEUE_OVERCOMMIT)，
        // 否则 queue = dispatch_get_global_queue(QOS_CLASS_UTILITY, DISPATCH_QUEUE_OVERCOMMIT)。
        // 当 DEPLOYMENT_TARGET_IPHONESIMULATOR 下运行时，#define qos_class_main() (QOS_CLASS_UTILITY)，
        // 即不管当前是主线程还是子线程，queue 都表示是一个全局并发队列。
        // 这个队列主要用来执行 run loop 的休眠的计时器用的，所以理论上只要是一个主队列之外的并发队列即可。
        
        dispatch_queue_t queue = pthread_main_np() ? __CFDispatchQueueGetGenericMatchingMain() : __CFDispatchQueueGetGenericBackground();
        
        // 指定 dispatch_source 为 DISPATCH_SOURCE_TYPE_TIMER 类型，即构建一个计时器类型的 dispatch_source，赋值给 timeout_timer
        timeout_timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue);
        
        // retain timeout_timer 计时器
        dispatch_retain(timeout_timer);
        
        // 设置 __timeout_context 的三个成员变量
        //（在 __CFRunLoopTimeoutCancel 回调函数中 __timeout_context 以及其 rl、ds 成员变量会进行释放）
        
        // ds 是 dispatch_source 的首字母缩写，
        //（timer 的回调参数 timeout_context 会携带 timeout_timer 计时器本身）
        timeout_context->ds = timeout_timer; 
        
        // timeout_context 持有 rl，
        //（timer 的回调参数 timeout_context 会携带 timeout_timer 计时器所处的 run loop）
        timeout_context->rl = (CFRunLoopRef)CFRetain(rl);
        
        // termTSR 是当前时间加上 run loop 运行时间的参数 seconds
        timeout_context->termTSR = startTSR + __CFTimeIntervalToTSR(seconds);
        
        // 设置 timeout_timer 计数器的上下文，即计时器回调函数的参数
        dispatch_set_context(timeout_timer, timeout_context); // source gets ownership of context
        
        // 设置 timeout_timer 计时器的执行的回调函数 __CFRunLoopTimeout
        // __CFRunLoopTimeout 函数内部会调用 CFRunLoopWakeUp(context->rl) 唤醒 timeout_context->rl
        //（可跳到下面先看一下 __CFRunLoopTimeout 函数实现）
        dispatch_source_set_event_handler_f(timeout_timer, __CFRunLoopTimeout); // 1⃣️
        
        // 设置 timeout_timer 计时器取消时的回调函数，对 timeout_timer 调用 dispatch_source_cancel 函数后，会触发此回调
        //（可跳到下面先看一下 __CFRunLoopTimeoutCancel 函数实现）
        dispatch_source_set_cancel_handler_f(timeout_timer, __CFRunLoopTimeoutCancel); // 2⃣️
        
        // 换算秒数
        // * 1000000000ULL 是把纳秒转化为秒
        uint64_t ns_at = (uint64_t)((__CFTSRToTimeInterval(startTSR) + seconds) * 1000000000ULL);
        
        // 计时器 timeout_timer 的执行时间间隔是 DISPATCH_TIME_FOREVER，第一次触发时间是 dispatch_time(1, ns_at) 后
        //（时间间隔为 DISPATCH_TIME_FOREVER，因此不会再次触发）
        dispatch_source_set_timer(timeout_timer, dispatch_time(1, ns_at), DISPATCH_TIME_FOREVER, 1000ULL);
        
        // dispatch_resume 恢复调度对象上块的调用，这里的作用是启动 timeout_timer 计时器
        dispatch_resume(timeout_timer);
        
        // 综上计时器 timeout_timer 是用来为 run loop 运行超时计时用的，当运行了 dispatch_time(1, ns_at) 后会触发此计时器执行
    } else { // infinite timeout 无效超时，永不超时
        seconds = 9999999999.0;
        timeout_context->termTSR = UINT64_MAX;
    }
    
    Boolean didDispatchPortLastTime = true;
    
    // run loop run 返回值，默认为 0，会在 do while 中根据情况被修改，
    // 当不为 0 时，run loop 退出，如果 do while 结尾处 retVal 还是 0 则继续循环。
    int32_t retVal = 0;
    
    // 进入这个外层 do while 循环，这个 do while 循环超长几乎包含了剩下的所有函数内容，
    // 中间还嵌套了一个较短的 do while 循环用于处理 run loop 的休眠和唤醒。
    do {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
        // macOS 下
        voucher_mach_msg_state_t voucherState = VOUCHER_MACH_MSG_STATE_UNCHANGED;
        voucher_t voucherCopy = NULL;
#endif
        // 3072
        uint8_t msg_buffer[3 * 1024];
        
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
        // macOS 下，申请两个局部变量 msg 和 livePort
        mach_msg_header_t *msg = NULL;
        
        // 用于记录唤醒休眠的 run loop 的 mach port，休眠前是 NULL
        mach_port_t livePort = MACH_PORT_NULL; 
        
#elif DEPLOYMENT_TARGET_WINDOWS
        HANDLE livePort = NULL;
        Boolean windowsMessageReceived = false;
#endif

        // do while 循环进来，macOS 下连续申请了 5 个局部变量：voucherState、voucherCopy、msg_buffer、msg、livePort。 3⃣️
        
        // 取当前 rlm 所需要监听的 mach port 集合，用于唤醒 run loop（__CFPortSet 实际上是 unsigned int 类型）
        __CFPortSet waitSet = rlm->_portSet;
        
        // 设置 rl->_perRunData->ignoreWakeUps = 0x0，表示未设置 IgnoreWakeUps 标记位。
        // rl->_perRunData->ignoreWakeUps = 0x57414B45/0x0，当值是 0x57414B45 时表示设置为 "忽略唤醒" 标记（IgnoreWakeUps），
        // 当值是 0x0 时表示未设置 "忽略唤醒"，此时 CFRunLoopWakeUp 函数才能正常唤醒 run loop，否则会直接 return。
        // Unset 
        __CFRunLoopUnsetIgnoreWakeUps(rl);
        
        // kCFRunLoopBeforeTimers（处理 timer 前） 和 kCFRunLoopBeforeSources（处理 source0 前) 当前所有的 observer 进行 run loop 活动状态回调 4⃣️
        if (rlm->_observerMask & kCFRunLoopBeforeTimers) __CFRunLoopDoObservers(rl, rlm, kCFRunLoopBeforeTimers); // 通知即将处理 Timers
        if (rlm->_observerMask & kCFRunLoopBeforeSources) __CFRunLoopDoObservers(rl, rlm, kCFRunLoopBeforeSources); // 通知即将处理 Sources
        
        // 遍历 rl 的 block 链表中的可在当前 run loop 运行模式下执行的 block，执行它们，执行完会把它们从链表中移除，并调用 Block_release 函数释放，
        // 得不到执行的 block 则继续留在链表中，等待 run loop 切换到 block 适合的 run loop mode 时再执行，
        // 会首先把 rl 的 _blocks_head 和 _blocks_tail 置为 NULL，然后得到执行的 block 执行完毕后会从链表中移除并调用 Block_release 函数。
        //（block 执行时调用的是 __CFRUNLOOP_IS_CALLING_OUT_TO_A_BLOCK__ 函数）
        //（我们开始收集这种名字大写的函数，在 run loop 学习过程中我们会遇到多个这种命名方式的函数，当我们都收集完了，那么 run loop 的学习就很熟悉了）
        
        // 目前我们收集到两个：
        // __CFRUNLOOP_IS_CALLING_OUT_TO_AN_OBSERVER_CALLBACK_FUNCTION__ // run loop 的状态发生变化前执行 run loop observer 的回调函数
        // __CFRUNLOOP_IS_CALLING_OUT_TO_A_BLOCK__ // 执行 run loop 的 block 链表中的 block（在 run loop 当前运行模式下可执行的链表中的 block，
        //                                            会得到执行，执行完以后会被释放并移除，不能在此模式下执行的 block 则还会保留在 block 链表中）
        
        //（可跳到下面先看一下 __CFRunLoopDoBlocks 函数实现）
        // 处理 Blocks
        __CFRunLoopDoBlocks(rl, rlm); // 5⃣️
        
        // 执行 rlm 的 _sources0 集合中的 Valid 和 Signaled 的 source，（执行 CFRunLoopSourceRef 的 perform 回调函数）
        
        // 目前我们收集到三个：
        // __CFRUNLOOP_IS_CALLING_OUT_TO_AN_OBSERVER_CALLBACK_FUNCTION__ // run loop 的状态发生变化前执行 run loop observer 的回调函数
        // __CFRUNLOOP_IS_CALLING_OUT_TO_A_BLOCK__ // 执行 run loop 的 block 链表中的 block（block 执行完以后会被释放并移除）
        // __CFRUNLOOP_IS_CALLING_OUT_TO_A_SOURCE0_PERFORM_FUNCTION__ // 执行 run loop mode 的 _source0 中的 CFRunLoopSourceRef 的 perfom 函数（以其 info 为参数）
        
        // 遍历 rlm 的 _source0 中的 Valid 和 Signaled 的 CFRunLoopSourceRef，执行其 perform 函数（perform(info)），且要执行的 CFRunLoopSourceRef 会被置为 UnsetSignaled， 
        // 那么下次 run loop 循环便不再执行这个 UnsetSignaled 的 CFRunLoopSourceRef 了。
        // 当有执行 source0 的 perform 函数时则返回 true，否则返回 false。
        // 处理 Source0
        Boolean sourceHandledThisLoop = __CFRunLoopDoSources0(rl, rlm, stopAfterHandle); // 6⃣️
        
        // sourceHandledThisLoop 的值表示 __CFRunLoopDoSources0 函数内部是否对 rlm 的 _sources0 中的 CFRunLoopSourceRef 执行了它的 void (*perform)(void *info) 函数。
        
        // 如果为真则再次遍历 rl 的 block 链表中的在指定 rlm 下执行的 block，
        //（这里没看出来 __CFRunLoopDoSources0 和 rl 的 block 链表有啥联系呀，为什么又执行链表 block 呢？难道 source0 执行会改变 run loop 的运行模式吗？也不对呀，这里入参还是 rl 和 rlm）
        if (sourceHandledThisLoop) {
            __CFRunLoopDoBlocks(rl, rlm); // 处理 Block
        }
        
        // 如果 rlm 的 _sources0 中有 CFRunLoopSourceRef 执行了 perform 函数 或者 timeout_context->termTSR 等于 0，则 poll 的值为 true 否则为 false。
        // timeout_context->termTSR == 0ULL 的情况：
        // 1. 当 seconds 入参小于等于 0 时（入参的 run loop 运行时间小于等于 0）
        // 2. 当👆创建的 timeout_timer 计时被回调时（即入参的 run loop 运行时间到了，表示 run loop 要退出了）
        
        Boolean poll = sourceHandledThisLoop || (0ULL == timeout_context->termTSR);
        
        // ⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️
        // 如果当前是主线程并且 dispatchPort 不为空且 didDispatchPortLastTime 为 false（ didDispatchPortLastTime 是在 do while 外声明的局部变量，初值为 true）
        if (MACH_PORT_NULL != dispatchPort && !didDispatchPortLastTime) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
            // macOS 下执行
            msg = (mach_msg_header_t *)msg_buffer;
            
            // 如果有 source1 被 signaled，则不休眠，直接跳到 handle_msg 去处理 source1
            // （主队列由任务要执行）
            if (__CFRunLoopServiceMachPort(dispatchPort,
                                           &msg,
                                           sizeof(msg_buffer),
                                           &livePort,
                                           0,
                                           &voucherState,
                                           NULL)) {
                // 有 source1 则跳转到 handle_msg
                goto handle_msg;
            }
            
#elif DEPLOYMENT_TARGET_WINDOWS
            if (__CFRunLoopWaitForMultipleObjects(NULL, &dispatchPort, 0, 0, &livePort, NULL)) {
                goto handle_msg;
            }
#endif
        }
        // ⬆️⬆️⬆️⬆️⬆️⬆️⬆️⬆️⬆️⬆️⬆️⬆️⬆️⬆️⬆️⬆️⬆️⬆️⬆️⬆️⬆️⬆️⬆️⬆️⬆️
        
        // didDispatchPortLastTime 置为 false
        didDispatchPortLastTime = false;
        
        // 若需要 poll 为假，则需要上面 sourceHandledThisLoop 为假即 rlm 的 _sources0 中没有 source 需要执行且 0ULL == timeout_context->termTSR，
        // 则调用 __CFRunLoopDoObservers 函数回调 rl 切换到 kCFRunLoopBeforeWaiting，即 rl 即将进入休眠状态。
        
        // 那么这里可以得出一个结论，当 run loop mode 中 source0 为空，或者 run loop 前一轮循环中 source0 中的 CFRunLoopSourceRef 的 perform 函数都已经执行完并被标记为 UnsetSignaled，
        // 并且入参的 seconds 小于等于 0 或者达到了入参 seconds 的运行时间，则 run loop 可以进入休眠
        if (!poll && (rlm->_observerMask & kCFRunLoopBeforeWaiting)) __CFRunLoopDoObservers(rl, rlm, kCFRunLoopBeforeWaiting); // 即将进入休眠
        
        // 设置 __CFBitfieldSetValue(((CFRuntimeBase *)rl)->_cfinfo[CF_INFO_BITS], 1, 1, 1)，
        // 标记 rl 进入休眠状态
        __CFRunLoopSetSleeping(rl); // 开始休眠
        
        // do not do any user callouts after this point (after notifying of sleeping)
        // 在此之后（通知睡眠之后）不进行任何用户标注
        
        // Must push the local-to-this-activation ports in on every loop iteration, 
        // as this mode could be run re-entrantly and we don't want these ports to get serviced.
        // 必须在每次循环迭代中都将 local-to-this-activation 端口推入，因为此模式可以重新进入运行，我们不希望为这些端口提供服务。
        
        // 把 dispatchPort 插入到 rlm 的 _portSet 中（waitSet）
        __CFPortSetInsert(dispatchPort, waitSet);
        
        // CFRunLoopMode 解锁
        __CFRunLoopModeUnlock(rlm);
        // CFRunLoop 解锁
        __CFRunLoopUnlock(rl);
        
        // sleepStart 用于记录睡眠开始的时间，poll 为 false 时预示着进入休眠状态
        CFAbsoluteTime sleepStart = poll ? 0.0 : CFAbsoluteTimeGetCurrent();
        
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
        // 在 macOS 下
#if USE_DISPATCH_SOURCE_FOR_TIMERS
        // 如果 rlm 使用 dispatch_source 构建的计时器
        
        // 这个内层的 do while 循环主要是用于 "保持" run looop 的睡眠状态的，直到需要被唤醒了才会跳出这个 do while 循环。只有在下面的事件发生时才会进行唤醒：
        // 1. 基于端口的输入源（port-based input source）（source1）的事件到达。
        // 2. CFRunLoopMode 中的 timers 触发。（CFRunLoopMode 可添加多个 timer，它们共用一个 _timerPort 唤醒 run loop，并且会计算所有 timer 中最近的下次要触发的 timer 的时间）
        // 3. 为 run loop 设置的超时时间过期。
        // 4. run loop 被显式唤醒。（被其他什么调用者手动唤醒）
        do {
            if (kCFUseCollectableAllocator) {
                // objc_clear_stack(0);
                // <rdar://problem/16393959>
                
                // 把以 msg_buffer 为起点长度为 sizeof(msg_buffer) 的内存置为 0
                memset(msg_buffer, 0, sizeof(msg_buffer));
            }
            
            // 强转为 mach_msg_header_t 指针
            // uint8_t msg_buffer[3 * 1024];
            // mach_msg_header_t *msg = NULL;
            
            msg = (mach_msg_header_t *)msg_buffer;
            
            // MachPort
            // 端口消息（mach_msg），（正式进入休眠）
            // 等待 waitSet 中的端口发送消息，等待接收消息唤醒当前 run loop
            // run loop 的核心就是一个 mach_msg，run loop 调用这个函数去接收消息，如果没有别人发送 port 消息过来，内核会将线程置于等待状态
            __CFRunLoopServiceMachPort(waitSet,
                                       &msg,
                                       sizeof(msg_buffer),
                                       &livePort,
                                       poll ? 0 : TIMEOUT_INFINITY,
                                       &voucherState,
                                       &voucherCopy);
            
            // modeQueuePort = _dispatch_runloop_root_queue_get_port_4CF(rlm->_queue) 来自于 rlm 的 _queue 队列端口
            // 基于 port 的 source 事件 或 调用者唤醒
            if (modeQueuePort != MACH_PORT_NULL && livePort == modeQueuePort) {
                // Drain the internal queue. If one of the callout blocks sets the timerFired flag, break out and service the timer.
                // 清空内部队列。如果其中一个标注块设置了 timerFired 标志，请中断并为计时器提供服务。
                
                // 如果一直能取到 rlm 的 _queue 的端口则一直 while 循环
                while (_dispatch_runloop_root_queue_perform_4CF(rlm->_queue));
                
                // _timerFired 首先赋值为 false，然后在 timer 的回调函数执行的时候会赋值为 true
                // rlm->_timerFired = false;
                // 当 _timerSource（计时器）回调时会执行这个 block，block 内部是把 _timerFired 修改为 true
                // __block Boolean *timerFiredPointer = &(rlm->_timerFired);
                // dispatch_source_set_event_handler(rlm->_timerSource, ^{
                //     *timerFiredPointer = true;
                // });
                // rlm 的 _timerSource 是启动时间是 DISPATCH_TIME_FOREVER，间隔是 DISPATCH_TIME_FOREVER 的计时器
                // _dispatch_source_set_runloop_timer_4CF(rlm->_timerSource, DISPATCH_TIME_FOREVER, DISPATCH_TIME_FOREVER, 321);
            
                //  timer 时间到 或 run loop 超时
                if (rlm->_timerFired) {
                    // Leave livePort as the queue port, and service timers below
                    // 将 livePort 保留为队列端口，并在下面保留服务计时器。 
                    
                    // rlm 的 _timerSource 计时器回调后 run loop 会结束休眠
                    rlm->_timerFired = false;
                    // 离开内循环。
                    break;
                } else {
                    if (msg && msg != (mach_msg_header_t *)msg_buffer) free(msg);
                }
            } else {
                // Go ahead and leave the inner loop.
                // 继续并离开内循环。
                break;
            }
        } while (1);
#else
        if (kCFUseCollectableAllocator) {
            // objc_clear_stack(0);
            // <rdar://problem/16393959>
            memset(msg_buffer, 0, sizeof(msg_buffer));
        }
        msg = (mach_msg_header_t *)msg_buffer;
        
        // 等待 waitSet 中的端口发送消息，等待接收消息
        __CFRunLoopServiceMachPort(waitSet,
                                   &msg,
                                   sizeof(msg_buffer),
                                   &livePort,
                                   poll ? 0 : TIMEOUT_INFINITY,
                                   &voucherState,
                                   &voucherCopy);
#endif
        
#elif DEPLOYMENT_TARGET_WINDOWS
        // Here, use the app-supplied message queue mask. They will set this if they are interested in having this run loop receive windows messages.
        __CFRunLoopWaitForMultipleObjects(waitSet,
                                          NULL,
                                          poll ? 0 : TIMEOUT_INFINITY,
                                          rlm->_msgQMask,
                                          &livePort,
                                          &windowsMessageReceived);
#endif
        
        // CFRunLoop 加锁
        __CFRunLoopLock(rl);
        // CFRunLoopMode 加锁
        __CFRunLoopModeLock(rlm);
        
        // 此时 run loop 要被唤醒了...
        
        // 统计 rl 的休眠时间，CFAbsoluteTimeGetCurrent() 当前时间减去 sleepStart 休眠开始时间
        rl->_sleepTime += (poll ? 0.0 : (CFAbsoluteTimeGetCurrent() - sleepStart));
        
        // Must remove the local-to-this-activation ports in on every loop iteration, 
        // as this mode could be run re-entrantly and we don't want these ports to get serviced. 
        // Also, we don't want them left in there if this function returns.
        // 必须在每次循环迭代中都删除本地激活端口，因为此模式可以重新进入，并且我们不希望为这些端口提供服务。另外，如果此函数返回，我们不希望它们留在那里。
        
        // 从 waitSet 中移除 dispatchPort
        __CFPortSetRemove(dispatchPort, waitSet);
        
        // 设置 rl "忽略唤醒"，意指当前的 run loop 已经是唤醒状态了，此时再来唤醒的话直接 return
        // rl->_perRunData->ignoreWakeUps = 0x57414B45; // 'WAKE'
        __CFRunLoopSetIgnoreWakeUps(rl);
        
        // user callouts now OK again
        // __CFBitfieldSetValue(((CFRuntimeBase *)rl)->_cfinfo[CF_INFO_BITS], 1, 1, 0);
        // 标记 rl 为非休眠状态
        __CFRunLoopUnsetSleeping(rl);
        
        // 调用 __CFRunLoopDoObservers 函数，回调 rl 切换到 kCFRunLoopAfterWaiting 状态了 
        // !poll 条件用于判断上面有进入休眠状态，通知 run loop observer 休眠要结束了
        if (!poll && (rlm->_observerMask & kCFRunLoopAfterWaiting)) __CFRunLoopDoObservers(rl, rlm, kCFRunLoopAfterWaiting); // 结束休眠
        
    handle_msg:;
        // rl->_perRunData->ignoreWakeUps = 0x57414B45
        // 设置 rl 忽略唤醒（表示已经进入唤醒状态，设置此值以阻止其他的唤醒操作）
        // 设置 rl "忽略唤醒"，意指当前的 run loop 已经是唤醒状态了，此时再来唤醒的话直接 return
        __CFRunLoopSetIgnoreWakeUps(rl);
        
        // 一大段 windows 平台下的代码，可忽略
#if DEPLOYMENT_TARGET_WINDOWS
        if (windowsMessageReceived) {
            // These Win32 APIs cause a callout, so make sure we're unlocked first and relocked after
            __CFRunLoopModeUnlock(rlm);
            __CFRunLoopUnlock(rl);
            
            if (rlm->_msgPump) {
                rlm->_msgPump();
            } else {
                MSG msg;
                if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE | PM_NOYIELD)) {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
            
            __CFRunLoopLock(rl);
            __CFRunLoopModeLock(rlm);
            sourceHandledThisLoop = true;
            
            // To prevent starvation of sources other than the message queue, we check again to see if any other sources need to be serviced
            // Use 0 for the mask so windows messages are ignored this time. Also use 0 for the timeout, because we're just checking to see if the things are signalled right now -- we will wait on them again later.
            // NOTE: Ignore the dispatch source (it's not in the wait set anymore) and also don't run the observers here since we are polling.
            __CFRunLoopSetSleeping(rl);
            __CFRunLoopModeUnlock(rlm);
            __CFRunLoopUnlock(rl);
            
            __CFRunLoopWaitForMultipleObjects(waitSet, NULL, 0, 0, &livePort, NULL);
            
            __CFRunLoopLock(rl);
            __CFRunLoopModeLock(rlm);            
            __CFRunLoopUnsetSleeping(rl);
            // If we have a new live port then it will be handled below as normal
        }
#endif
        // 根据唤醒 run loop 的 livePort 值，来进行对应逻辑处理
        if (MACH_PORT_NULL == livePort) {
            // 如果 livePort 为 MACH_PORT_NULL，可能是 run loop 休眠超时，啥都不做
            
            // #define CFRUNLOOP_WAKEUP_FOR_NOTHING() do { } while (0)
            CFRUNLOOP_WAKEUP_FOR_NOTHING();
            // handle nothing
        } else if (livePort == rl->_wakeUpPort) {
            // 如果 livePort 是 rl->_wakeUpPort，是指被其他线程或进程唤醒，啥都不做
            
            // #define CFRUNLOOP_WAKEUP_FOR_WAKEUP() do { } while (0)
            CFRUNLOOP_WAKEUP_FOR_WAKEUP();
            // do nothing on Mac OS
            
            // windows 平台下
#if DEPLOYMENT_TARGET_WINDOWS
            // Always reset the wake up port, or risk spinning forever
            ResetEvent(rl->_wakeUpPort);
#endif
        }
        
        // 如果计时器是使用 dispatch_source 实现的
#if USE_DISPATCH_SOURCE_FOR_TIMERS
        else if (modeQueuePort != MACH_PORT_NULL && livePort == modeQueuePort) {
            // 如果 rlm 的 queue 的 modeQueuePort 不为 NULL，且此时 livePort 等于 modeQueuePort，
            // 则表示此时需要处理 timer 回调。（这里表示是 timer 回调时间到了唤醒 run loop）
            
            // #define CFRUNLOOP_WAKEUP_FOR_TIMER() do { } while (0)
            CFRUNLOOP_WAKEUP_FOR_TIMER();
            
            // 遍历执行 rlm 的 _timers 集合中到达触发时间的 timer 的回调函数并更新其 `_fireTSR` 和 `_nextFireDate`
            if (!__CFRunLoopDoTimers(rl, rlm, mach_absolute_time())) { // 7⃣️
                // Re-arm the next timer, because we apparently fired early
        
                // 目前我们收集到四个：
                // __CFRUNLOOP_IS_CALLING_OUT_TO_AN_OBSERVER_CALLBACK_FUNCTION__ // run loop 的状态发生变化前执行 run loop observer 的回调函数
                // __CFRUNLOOP_IS_CALLING_OUT_TO_A_BLOCK__ // 执行 run loop 的 block 链表中的 block（block 执行完以后会被释放并移除）
                // __CFRUNLOOP_IS_CALLING_OUT_TO_A_SOURCE0_PERFORM_FUNCTION__ // 执行 run loop mode 的 _source0 中的 CFRunLoopSourceRef 的 perfom 函数（以其 info 为参数）
                // __CFRUNLOOP_IS_CALLING_OUT_TO_A_TIMER_CALLBACK_FUNCTION__ // 执行 run loop mode 的 _timers 中的 CFRunLoopTimerRef 的 _callout 函数（以其 _context.info 为参数）
                
                // 重新布防下一个计时器（计算计时器中下次最近的触发时间，时间到时唤醒 run loop）
                __CFArmNextTimerInMode(rlm, rl);
            }
        }
#endif

#if USE_MK_TIMER_TOO
        else if (rlm->_timerPort != MACH_PORT_NULL && livePort == rlm->_timerPort) {
            // 如果计时器是使用 MK 实现的
            
            // #define   CFRUNLOOP_WAKEUP_FOR_TIMER() do { } while (0)
            CFRUNLOOP_WAKEUP_FOR_TIMER();（这里表示是 timer 回调时间到了唤醒 run loop）
            
            // On Windows, we have observed an issue where the timer port is set before the time which we requested it to be set. For example, we set the fire time to be TSR 167646765860, but it is actually observed firing at TSR 167646764145, which is 1715 ticks early. The result is that, when __CFRunLoopDoTimers checks to see if any of the run loop timers should be firing, it appears to be 'too early' for the next timer, and no timers are handled.
            // 在 Windows 上，我们发现了一个问题，即在我们要求设置定时器端口之前设置了定时器端口。例如，我们将开火时间设置为 TSR 167646765860，但实际上可以观察到以 TSR 167646764145 开火，这是提早 1715 滴答。结果是，当 __CFRunLoopDoTimers 检查是否应触发任何运行循环计时器时，下一个计时器似乎为时过早，并且不处理任何计时器。
            
            // In this case, the timer port has been automatically reset (since it was returned from MsgWaitForMultipleObjectsEx), and if we do not re-arm it, then no timers will ever be serviced again unless something adjusts the timer list (e.g. adding or removing timers). The fix for the issue is to reset the timer here if CFRunLoopDoTimers did not handle a timer itself. 9308754
            // 在 Windows 上，我们发现了一个问题，即在我们要求设置定时器端口之前设置了定时器端口。例如，我们将开火时间设置为 TSR 167646765860，但实际上可以观察到以 TSR 167646764145开火，这是提早 1715 滴答。结果是，当 __CFRunLoopDoTimers 检查是否应触发任何运行循环计时器时，下一个计时器似乎为时过早，并且不处理任何计时器。
            
            // run loop mode 中使用 MK_TIMER 构建计时器时。触发到达执行时间的 run loop mode 中的 timer 回调。
            
            // 遍历执行 rlm 的 _timers 集合中到达触发时间的 timer 的回调函数并更新其 `_fireTSR` 和 `_nextFireDate`
            if (!__CFRunLoopDoTimers(rl, rlm, mach_absolute_time())) {
                // Re-arm the next timer
                // 重新布防下一个计时器
                __CFArmNextTimerInMode(rlm, rl);
            }
        }
#endif
        else if (livePort == dispatchPort) {
            // dispatchPort: 处理分发到 main queue 上的事件（这里表示是 dispatch 触发 run loop 唤醒）
            
            // #define   CFRUNLOOP_WAKEUP_FOR_DISPATCH() do { } while (0)
            CFRUNLOOP_WAKEUP_FOR_DISPATCH();
            
            // CFRunLoopMode 解锁
            __CFRunLoopModeUnlock(rlm);
            // CFRunLoop 解锁
            __CFRunLoopUnlock(rl);
            
            // 设置 TSD 中的 __CFTSDKeyIsInGCDMainQ 为 6（与下面的 0 成对，大概理解为构成锁）
            _CFSetTSD(__CFTSDKeyIsInGCDMainQ, (void *)6, NULL);
            
#if DEPLOYMENT_TARGET_WINDOWS
            void *msg = 0;
#endif

            // 目前我们收集到五个：
            // __CFRUNLOOP_IS_CALLING_OUT_TO_AN_OBSERVER_CALLBACK_FUNCTION__
            // __CFRUNLOOP_IS_CALLING_OUT_TO_A_BLOCK__
            // __CFRUNLOOP_IS_CALLING_OUT_TO_A_SOURCE0_PERFORM_FUNCTION__
            // __CFRUNLOOP_IS_CALLING_OUT_TO_A_TIMER_CALLBACK_FUNCTION__
            // __CFRUNLOOP_IS_SERVICING_THE_MAIN_DISPATCH_QUEUE__
            
            // 主队列回调事件
            __CFRUNLOOP_IS_SERVICING_THE_MAIN_DISPATCH_QUEUE__(msg);
            
            // 设置 TSD 中的 __CFTSDKeyIsInGCDMainQ 为 0
            _CFSetTSD(__CFTSDKeyIsInGCDMainQ, (void *)0, NULL);
            
            // CFRunLoop CFRunLoopMode 加锁
            __CFRunLoopLock(rl);
            __CFRunLoopModeLock(rlm);
            
            sourceHandledThisLoop = true;
            didDispatchPortLastTime = true;
        } else {
            // 其余的，肯定是各种 source1 事件
            
            // #define   CFRUNLOOP_WAKEUP_FOR_SOURCE() do { } while (0)
            CFRUNLOOP_WAKEUP_FOR_SOURCE();（这里表示是 source1 触发 run loop 唤醒）
            
            // If we received a voucher from this mach_msg, then put a copy of the new voucher into TSD. 
            // CFMachPortBoost will look in the TSD for the voucher. 
            // By using the value in the TSD we tie the CFMachPortBoost to this received mach_msg explicitly 
            // without a chance for anything in between the two pieces of code to set the voucher again.
            // 如果我们收到了来自此 mach_msg 的凭证，则将新凭证的副本放入 TSD。 
            // CFMachPortBoost 将在 TSD 中查找该凭证。通过使用 TSD 中的值，我们将 CFMachPortBoost 明确地绑定到此接收到的 mach_msg 上，
            // 而在这两段代码之间没有任何机会再次设置凭单。
            
            // TSD 释放时，voucherCopy 调用 os_release 释放
            voucher_t previousVoucher = _CFSetTSD(__CFTSDKeyMachMessageHasVoucher, (void *)voucherCopy, os_release);
            
            // Despite the name, this works for windows handles as well
            
            // 从 rlm 的 _portToV1SourceMap 中，根据 livePort 找到其对应的 CFRunLoopSourceRef
            CFRunLoopSourceRef rls = __CFRunLoopModeFindSourceForMachPort(rl, rlm, livePort);
            if (rls) {
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
                mach_msg_header_t *reply = NULL;
                
                // 执行 source1 回调 __CFRUNLOOP_IS_CALLING_OUT_TO_A_SOURCE1_PERFORM_FUNCTION__
                sourceHandledThisLoop = __CFRunLoopDoSource1(rl, rlm, rls, msg, msg->msgh_size, &reply) || sourceHandledThisLoop;
                
                // 如果有需要回复soruce1的消息，则回复
                if (NULL != reply) {
                    (void)mach_msg(reply, MACH_SEND_MSG, reply->msgh_size, 0, MACH_PORT_NULL, 0, MACH_PORT_NULL);
                    CFAllocatorDeallocate(kCFAllocatorSystemDefault, reply);
                }
                
#elif DEPLOYMENT_TARGET_WINDOWS
                sourceHandledThisLoop = __CFRunLoopDoSource1(rl, rlm, rls) || sourceHandledThisLoop;
#endif
            }
            
            // Restore the previous voucher 恢复以前的凭证
            _CFSetTSD(__CFTSDKeyMachMessageHasVoucher, previousVoucher, os_release);
        }
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
        // 释放 msg 的内存空间
        if (msg && msg != (mach_msg_header_t *)msg_buffer) free(msg);
#endif
        
        // 执行 rl 的 block 链表中的 block
        __CFRunLoopDoBlocks(rl, rlm);
        
        // 根据当前 run loop 的状态来判断是否需要走下一个 loop。
        if (sourceHandledThisLoop && stopAfterHandle) {
            // stopAfterHandle 为真表示仅处理一个 sourc0，标记为 kCFRunLoopRunHandledSource。退出本次 run loop 循环。 
            retVal = kCFRunLoopRunHandledSource; // 4
        } else if (timeout_context->termTSR < mach_absolute_time()) {
            // run loop 运行超时。退出本次 run loop 循环。
            retVal = kCFRunLoopRunTimedOut; // 3
        } else if (__CFRunLoopIsStopped(rl)) {
            // 外部停止。退出本次 run loop 循环。
            __CFRunLoopUnsetStopped(rl);
            retVal = kCFRunLoopRunStopped; // 2
        } else if (rlm->_stopped) {
            // rlm 停止（外部强制停止）。退出本次 run loop 循环。
            rlm->_stopped = false;
            retVal = kCFRunLoopRunStopped; // 2
        } else if (__CFRunLoopModeIsEmpty(rl, rlm, previousMode)) {
            // rlm 的 sources0/sources1/timers/block 为空。退出本次 run loop 循环。
            retVal = kCFRunLoopRunFinished; // 1
        }
        
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
        voucher_mach_msg_revert(voucherState);
        // 释放 voucherCopy
        os_release(voucherCopy);
#endif
        
    } while (0 == retVal); // 外层的 do while 循环结束的条件是 retVal 不等于 0 时
    
    // timeout_timer 是记录 run loop 运行时间的计时器，run loop 退出时要对 timeout_timer 进行释放
    if (timeout_timer) {
        // 取消计时器，会在取消的回调函数 __CFRunLoopTimeoutCancel 里面做清理工作 
        dispatch_source_cancel(timeout_timer);
        // 释放 timeout_timer
        dispatch_release(timeout_timer);
    } else {
        // 释放 timeout_context，
        // 对应前面的 timeout_context = (struct __timeout_context *)malloc(sizeof(*timeout_context))，malloc 的申请空间。
        free(timeout_context);
    }
    
    return retVal;
}
```

#### \__CFRunLoopTimeout
&emsp;`__CFRunLoopTimeout` 函数是 `__CFRunLoopRun` 函数内部构建的一个局部计时器变量（`dispatch_source_t timeout_timer = NULL;`）的回调函数，且全局搜索 `__CFRunLoopTimeout` 仅用于此处，意思就是 `__CFRunLoopTimeout` 函数仅提供给 `__CFRunLoopRun` 函数内部来使用的。（是一个专一的好函数！）
```c++
static void __CFRunLoopTimeout(void *arg) {
    // 入参 arg 是我们在 __CFRunLoopRun 函数里面辛辛苦苦构建的：
    // struct __timeout_context *timeout_context = (struct __timeout_context *)malloc(sizeof(*timeout_context));
    // timeout_context 仅有三个成员变量：
    // ds: 是 timeout_timer 计时器本身
    // rl: 是 __CFRunLoopRun 函数入参 CFRunLoopRef
    // termTSR: 是当前时间加 __CFRunLoopRun 函数入参 seconds，即表示 rl 运行结束的一个具体时间点，
    //          也是 timeout_timer 计时器从启动到第一次触发的时间点，即 __CFRunLoopTimeout 函数本函第一次被调用的时间点
    // 好了，arg 函数参数的内容介绍完了，下面我们看函数内容。
    
    // 强转指针
    struct __timeout_context *context = (struct __timeout_context *)arg;
    
    // 把 termTSR 置为 0ULL
    context->termTSR = 0ULL;
    
    // #define CFRUNLOOP_WAKEUP_FOR_TIMEOUT() do { } while (0) 什么也没做
    CFRUNLOOP_WAKEUP_FOR_TIMEOUT();
    
    // 唤醒 run loop
    CFRunLoopWakeUp(context->rl);
    
    // The interval is DISPATCH_TIME_FOREVER, so this won't fire again.
    // 时间间隔为 DISPATCH_TIME_FOREVER，因此不会再次触发。
    
    // 此行注释也提醒我们了，timeout_timer 计时器构建时时间间隔设置的正是 DISPATCH_TIME_FOREVER，即它只会回调 __CFRunLoopTimeout 函数一次，
    // 以后都不会再触发了，同时它也预示着 context->rl 的运行时间就到了，context->rl 要退出了。
    // timeout_timer 计时器的清理工作在 __CFRunLoopTimeoutCancel 函数中，我们等会看。
}
```
&emsp;`__CFRunLoopTimeout` 函数内部很清晰，首先把入参 \__timeout_context 结构体实例的 termTSR 置为 0ULL，这是一个指针传值，也同时修改了 `__CFRunLoopRun` 函数内部的 `timeout_context->termTSR` 被置为了 0ULL。然后就是唤醒 `__CFRunLoopRun` 函数内传入的 CFRunLoopRef 了，下面我们看一下 run loop 的唤醒函数 `CFRunLoopWakeUp` 的定义。
##### CFRunLoopWakeUp
&emsp;`CFRunLoopWakeUp` 函数内部，通过 run loop 的 `_wakeUpPort` 唤醒端口来唤醒 run loop 对象。
```c++
void CFRunLoopWakeUp(CFRunLoopRef rl) {
    CHECK_FOR_FORK();
    // This lock is crucial to ignorable wakeups, do not remove it.
    // 此锁对于可唤醒系统至关重要，请不要删除它。
    // CFRunLoop 加锁
    __CFRunLoopLock(rl);
    
    // (rl->_perRunData->ignoreWakeUps) ? true : false
    // 如果 rl 被标记为了忽略唤醒则直接 return 不再进行唤醒操作
    if (__CFRunLoopIsIgnoringWakeUps(rl)) {
        // CFRunLoop 解锁
        __CFRunLoopUnlock(rl);
        return;
    }
    
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
    // 内核返回值
    kern_return_t ret;
    
    /* We unconditionally try to send the message, since we don't want to lose a wakeup, 
    but the send may fail if there is already a wakeup pending, since the queue length is 1. */
    // 因为我们不想丢失唤醒，所以我们无条件地尝试发送消息，但是如果队列中的队列长度为1，则如果已经存在唤醒，则发送可能会失败。
    
    // rl->_wakeUpPort 唤醒端口，在创建 run loop 对象时就会赋值 loop->_wakeUpPort = __CFPortAllocate()
    ret = __CFSendTrivialMachMessage(rl->_wakeUpPort, 0, MACH_SEND_TIMEOUT, 0);
    
    // 如果 ret 未成功且未返回超时则 crash 无法发送消息以唤醒端口。
    // #define MACH_MSG_SUCCESS   0x00000000
    // #define MACH_SEND_TIMED_OUT   0x10000004 // 超时之前未发送消息
    
    if (ret != MACH_MSG_SUCCESS && ret != MACH_SEND_TIMED_OUT) CRASH("*** Unable to send message to wake up port. (%d) ***", ret);
    
#elif DEPLOYMENT_TARGET_WINDOWS
    SetEvent(rl->_wakeUpPort);
#endif
    // CFRunLoop 解锁
    __CFRunLoopUnlock(rl);
}
```
&emsp;`__CFSendTrivialMachMessage` 函数定义：
```c++
static uint32_t __CFSendTrivialMachMessage(mach_port_t port, uint32_t msg_id, CFOptionFlags options, uint32_t timeout) {
    // typedef int   kern_return_t;
    kern_return_t result;
    
    mach_msg_header_t header;
    header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, 0);
    header.msgh_size = sizeof(mach_msg_header_t);
    
    // msgh_remote_port 赋值为 port
    header.msgh_remote_port = port;
    
    header.msgh_local_port = MACH_PORT_NULL;
    header.msgh_id = msg_id;
    
    // mach_msg 
    result = mach_msg(&header, MACH_SEND_MSG|options, header.msgh_size, 0, MACH_PORT_NULL, timeout, MACH_PORT_NULL);
    if (result == MACH_SEND_TIMED_OUT) mach_msg_destroy(&header);
    
    return result;
}
```
&emsp;`mach_msg` 根据 `mach_msg_header_t header` 中的 `msgh_remote_port` 来唤醒 run loop。

#### \__CFRunLoopTimeoutCancel
&emsp;`__CFRunLoopTimeoutCancel` 函数是 `__CFRunLoopRun` 函数内部构建的一个局部计时器变量（`dispatch_source_t timeout_timer = NULL;`）取消时的回调函数，且全局搜索 `__CFRunLoopTimeoutCancel` 仅用于此处，意思就是 `__CFRunLoopTimeoutCancel` 函数仅提供给 `__CFRunLoopRun` 函数内部来使用的。（是一个专一的好函数！）
```c++
static void __CFRunLoopTimeoutCancel(void *arg) {
    // 入参 arg 是我们在 __CFRunLoopRun 函数里面辛辛苦苦构建的 timeout_context：
    // struct __timeout_context *timeout_context = (struct __timeout_context *)malloc(sizeof(*timeout_context));
    // timeout_context 仅有三个成员变量：
    // ds: 是 timeout_timer 计时器本身
    // rl: 是 __CFRunLoopRun 函数入参 CFRunLoopRef
    // termTSR: 是当前时间加 __CFRunLoopRun 函数入参 seconds，即表示 rl 运行结束的一个具体时间点，
    //          也是 timeout_timer 计时器从启动到第一次触发的时间点，即 __CFRunLoopTimeout 函数本函第一次被调用的时间点
    // 好了，arg 函数参数的内容介绍完了，下面我们看函数内容。
    
    // 强转指针
    struct __timeout_context *context = (struct __timeout_context *)arg;
    
    // 对应赋值时的 timeout_context->rl = (CFRunLoopRef)CFRetain(rl) 持有操作
    CFRelease(context->rl);
    
    // 对应于构建时的 dispatch_retain 操作
    // timeout_timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue);
    // dispatch_retain(timeout_timer);
    
    dispatch_release(context->ds);
    
    // 释放 context 内存
    free(context);
}
```
&emsp;`__CFRunLoopTimeoutCancel` 函数较简单，就是用来做释放操作。

#### \__CFRunLoopDoBlocks
&emsp;`__CFRunLoopDoBlocks` 函数内部是遍历 run loop 的 block 的链表，在指定的 rlm 下执行 block，执行完节点的 block 以后会把该节点从链表中移除，最后更新链表的头节点和尾节点。
```c++
// run loop 的 block 链表的节点定义
struct _block_item {
    struct _block_item *_next;
    
    // typedef const void * CFTypeRef;
    // _mode 成员变量类型是 CFString 或者 CFSet。
    // 指定了 _block 执行时所处的模式，_block 只能在 _mode 包含的模式下执行
    CFTypeRef _mode;    // CFString or CFSet
    
    // block 本体
    void (^_block)(void);
};

static Boolean __CFRunLoopDoBlocks(CFRunLoopRef rl, CFRunLoopModeRef rlm) { // Call with rl and rlm locked（函数调用前已经加锁）
    // 如果 rl 的 _blocks_head block 链表头节点为空则返回 false
    if (!rl->_blocks_head) return false;
    // 如果 rlm 不存在或者 rlm->_name 不存在则返回 false
    if (!rlm || !rlm->_name) return false;
    
    // 当链表中的 block 执行时会被置为 true
    Boolean did = false;
    // 取得 block 链表的头节点和尾节点
    struct _block_item *head = rl->_blocks_head;
    struct _block_item *tail = rl->_blocks_tail;
    
    // 此时直接把 rl 的 _blocks_head 和 _blocks_tail 置为 NULL
    rl->_blocks_head = NULL;
    rl->_blocks_tail = NULL;
    
    // 取出 common mode 集合和 rlm 的 _name
    CFSetRef commonModes = rl->_commonModes;
    CFStringRef curMode = rlm->_name;
    
    // CFRunLoopMode CFRunLoop 解锁
    __CFRunLoopModeUnlock(rlm);
    __CFRunLoopUnlock(rl);
    
    struct _block_item *prev = NULL;
    struct _block_item *item = head;
    
    // 对 rl 的 block 链表头开始遍历
    while (item) {
        // 取得链表当前节点和下一个节点
        struct _block_item *curr = item;
        item = item->_next;
        
        // doit 标记是否有 block 需要在 rlm 模式下执行
        Boolean doit = false;
        
        if (CFStringGetTypeID() == CFGetTypeID(curr->_mode)) {
            // block 节点的 _mode 是字符串时，判断其是否和入参 rlm 的 _name 相等，
            // 或者节点的 _mode 是 kCFRunLoopCommonModes 时，判断 rl 的 _commonModes 集合是否包含入参 rlm 的 _name。
            doit = CFEqual(curr->_mode, curMode) || 
                   (CFEqual(curr->_mode, kCFRunLoopCommonModes) &&
                    CFSetContainsValue(commonModes, curMode));
        } else {
            // block 节点的 _mode 是集合时，判断入参 rlm 的 _name 是否被包含其中，
            // 或者节点的 _mode 包含 kCFRunLoopCommonModes 时，判断 rl 的 _commonModes 集合是否包含入参 rlm 的 _name。
            doit = CFSetContainsValue((CFSetRef)curr->_mode, curMode) ||
                   (CFSetContainsValue((CFSetRef)curr->_mode, kCFRunLoopCommonModes) &&
                    CFSetContainsValue(commonModes, curMode));
        }
        
        // 如果 curr 节点的 _mode 中不包含 rlm 的 _name，则把当前节点 curr 赋值给 prev
        if (!doit) prev = curr;
        
        // doit 为 true，即可以在指定的 blm 下执行 block 
        if (doit) {
            // 下一个节点
            if (prev) prev->_next = item;
            
            // 如果当前执行的是头节点的 block，则更新 head（头节点）为下一个节点
            if (curr == head) head = item;
            // 如果当前执行的是尾节点的 block，则更新 tail（尾节点）为前一个节点
            if (curr == tail) tail = prev;
            
            // 取出节点里的 block 本体
            void (^block)(void) = curr->_block;
            // 释放节点的 _mode 
            CFRelease(curr->_mode);
            // 释放节点
            free(curr);
            
            if (doit) {
                // 执行 block 
                __CFRUNLOOP_IS_CALLING_OUT_TO_A_BLOCK__(block);
                // 把 did 置为 true，表示 __CFRunLoopDoBlocks 函数内执行过 block，也即是说 rl 的 block 链表中有 block 得到执行
                did = true;
            }
            
            // 释放 block
            Block_release(block); // do this before relocking to prevent deadlocks where some yahoo wants to run the run loop reentrantly from their dealloc
        }
    }
    
    // CFRunLoop CFRunLoopMode 加锁
    __CFRunLoopLock(rl);
    __CFRunLoopModeLock(rlm);
    
    // 更新 rl block 链表的头节点和尾节点
    if (head) {
        tail->_next = rl->_blocks_head;
        rl->_blocks_head = head;
        if (!rl->_blocks_tail) rl->_blocks_tail = tail;
    }
    
    return did;
}
```
##### \_\_CFRUNLOOP_IS_CALLING_OUT_TO_A_BLOCK\_\_
&emsp;`__CFRUNLOOP_IS_CALLING_OUT_TO_A_BLOCK__` 执行 block。
```c++
static void __CFRUNLOOP_IS_CALLING_OUT_TO_A_BLOCK__(void (^block)(void)) {
    if (block) {
        block();
    }
    asm __volatile__(""); // thwart tail-call optimization
}
```
#### \__CFRunLoopDoSources0
&emsp;`__CFRunLoopDoSources0` 函数是遍历收集 rlm 的 \_source0 把 Valid、Signaled 的 CFRunLoopSourceRef 收集起来，然后执行以 source0 的 info 为参数执行 source0 的 perform 函数，且会把 CFRunLoopSourceRef 置为 UnsetSignaled，等待被再次标记并执行。
```c++
/* rl is locked, rlm is locked on entrance and exit */

/* 
* 进入 __CFRunLoopDoSources0 函数前 rl 和 rlm 的 _lock 都已经加锁了，
* 在 __CFRunLoopDoSources0 函数内部当需要执行回调时，会对 rl 和 rlm 进行解锁，
* 然后在回调函数执行完成后，会重新对 rl 和 rlm 进行加锁。
*/
static Boolean __CFRunLoopDoSources0(CFRunLoopRef rl, CFRunLoopModeRef rlm, Boolean stopAfterHandle) {    /* DOES CALLOUT */
    CHECK_FOR_FORK();
    
    // 用来收集 rlm 的 _sources0 中的
    CFTypeRef sources = NULL;
    // sourceHandled 用于标记是否执行了 _source0 集合中 CFRunLoopSourceRef 的函数
    Boolean sourceHandled = false;
    
    /* Fire the version 0 sources */
    if (NULL != rlm->_sources0 && 0 < CFSetGetCount(rlm->_sources0)) {
        // rlm 的 _sources0 不为空且其内部包含的元素数大于 0
        
        // 调用 __CFRunLoopCollectSources0 函数把 rlm->_sources0 添加到 sources 中
        CFSetApplyFunction(rlm->_sources0, (__CFRunLoopCollectSources0), &sources);
    }
    
    if (NULL != sources) {
        // CFRunLoopMode CFRunLoop 解锁
        __CFRunLoopModeUnlock(rlm);
        __CFRunLoopUnlock(rl);
        
        // sources is either a single (retained) CFRunLoopSourceRef or an array of (retained) CFRunLoopSourceRef
        // 源可以是单个（保留的）CFRunLoopSourceRef，也可以是（保留的）CFRunLoopSourceRef 数组
        
        if (CFGetTypeID(sources) == CFRunLoopSourceGetTypeID()) {
            // 如果 sources 是 CFRunLoopSourceRef
            CFRunLoopSourceRef rls = (CFRunLoopSourceRef)sources;
            
            // CFRunLoopSource 加锁
            __CFRunLoopSourceLock(rls);
            
            // (Boolean)__CFBitfieldGetValue(rls->_bits, 1, 1) 
            // 判断 _bits  位
            if (__CFRunLoopSourceIsSignaled(rls)) {
                // __CFBitfieldSetValue(rls->_bits, 1, 1, 0);
                // 置为 0，表示 unset 状态
                
                //（这里把 rls 设置为 UnsetSignaled 的以后，此 rls 以后就不会再被执行了，这里同 run loop 的 block 链表不同，
                // block 链表是 block 执行完以后直接把 block 从链表中移除了，source0 这里则是把执行过的 CFRunLoopSource 置为一个状态（相当于已经被执行过的状态了）。）
                __CFRunLoopSourceUnsetSignaled(rls);
                
                // (Boolean)__CFBitfieldGetValue(((const CFRuntimeBase *)cf)->_cfinfo[CF_INFO_BITS], 3, 3)
                // _cfinfo 位表示有效有效
                if (__CFIsValid(rls)) {
                    // CFRunLoopSource 解锁
                    __CFRunLoopSourceUnlock(rls);
                    
                    // 以 source0 的 void * info 为参数，执行 source0 的 void (*perform)(void *info) 函数
                    __CFRUNLOOP_IS_CALLING_OUT_TO_A_SOURCE0_PERFORM_FUNCTION__(rls->_context.version0.perform, rls->_context.version0.info);
                    
                    CHECK_FOR_FORK();
                    // 标记 source0 被执行了
                    sourceHandled = true;
                } else {
                    // CFRunLoopSource 解锁
                    __CFRunLoopSourceUnlock(rls);
                }
            } else {
                // CFRunLoopSource 解锁
                __CFRunLoopSourceUnlock(rls);
            }
        } else {
            CFIndex cnt = CFArrayGetCount((CFArrayRef)sources);
            // 对数组中的 CFRunLoopSourceRef 进行排序，排序规则是 __CFRunLoopSourceComparator，其内部是根据 CFRunLoopSourceRef 的 _order 字段进行排序
            CFArraySortValues((CFMutableArrayRef)sources, CFRangeMake(0, cnt), (__CFRunLoopSourceComparator), NULL);
            
            // 然后就遍历数组中的 CFRunLoopSourceRef，同上以 source0 的 info 为参数执行 source0 的 perform 函数
            for (CFIndex idx = 0; idx < cnt; idx++) {
                CFRunLoopSourceRef rls = (CFRunLoopSourceRef)CFArrayGetValueAtIndex((CFArrayRef)sources, idx);
                __CFRunLoopSourceLock(rls);
                if (__CFRunLoopSourceIsSignaled(rls)) {
                    // 置为 UnsetSignaled
                    __CFRunLoopSourceUnsetSignaled(rls);
                    if (__CFIsValid(rls)) {
                        __CFRunLoopSourceUnlock(rls);
                        
                        // 执行 
                        __CFRUNLOOP_IS_CALLING_OUT_TO_A_SOURCE0_PERFORM_FUNCTION__(rls->_context.version0.perform, rls->_context.version0.info);
                        
                        CHECK_FOR_FORK();
                        sourceHandled = true;
                    } else {
                        __CFRunLoopSourceUnlock(rls);
                    }
                } else {
                    __CFRunLoopSourceUnlock(rls);
                }
                if (stopAfterHandle && sourceHandled) {
                    break;
                }
            }
        }
        // 释放临时变量 sources
        CFRelease(sources);
        
        // CFRunLoop CFRunLoopMode 加锁
        __CFRunLoopLock(rl);
        __CFRunLoopModeLock(rlm);
    }
    return sourceHandled;
}
```
##### \__CFRunLoopCollectSources0
&emsp;`__CFRunLoopCollectSources0` 函数是把 Valid、Signaled 的 source0 的  CFRunLoopSourceRef 收集到入参 `context` 中。
```c++
static void __CFRunLoopCollectSources0(const void *value, void *context) {
    // 类型转换，
    // value 是 rlm 的 CFMutableSetRef _sources0，其内部存放的是 CFRunLoopSourceRef
    CFRunLoopSourceRef rls = (CFRunLoopSourceRef)value;
    
    // context 是一个指针参数用于传递最终的结果
    CFTypeRef *sources = (CFTypeRef *)context;
    
    // (Boolean)__CFBitfieldGetValue(rls->_bits, 1, 1) 
    if (0 == rls->_context.version0.version && __CFIsValid(rls) && __CFRunLoopSourceIsSignaled(rls)) {
        // 当前是 source0 且 rls 是有效的且 rls 是可发送信号的
        
        if (NULL == *sources) {
            // 如果入参 sources 中是空的则直接持有 rls
            *sources = CFRetain(rls);
        } else if (CFGetTypeID(*sources) == CFRunLoopSourceGetTypeID()) {
            // 如果入参 sources 不为空，且内部存放的是 CFRunLoopSourceRef
            
            CFTypeRef oldrls = *sources;
            
            // 申请一个数组，把入参的 rls 和 *sources 拼接在一个数组中
            *sources = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
            CFArrayAppendValue((CFMutableArrayRef)*sources, oldrls);
            CFArrayAppendValue((CFMutableArrayRef)*sources, rls);
            
            // 释放临时变量
            CFRelease(oldrls);
        } else {
            // sources 是个数组的话，直接把 rls 拼接在 sources 中
            CFArrayAppendValue((CFMutableArrayRef)*sources, rls);
        }
    }
}
```
##### \_\_CFRUNLOOP_IS_CALLING_OUT_TO_A_SOURCE0_PERFORM_FUNCTION\_\_
&emsp;`__CFRUNLOOP_IS_CALLING_OUT_TO_A_SOURCE0_PERFORM_FUNCTION__` 函数以 info 做参执行 source0 中 perform 函数。
```c++
static void __CFRUNLOOP_IS_CALLING_OUT_TO_A_SOURCE0_PERFORM_FUNCTION__(void (*perform)(void *), void *info) {
    if (perform) {
        perform(info);
    }
    asm __volatile__(""); // thwart tail-call optimization
}
```
#### \__CFRunLoopDoTimers
&emsp;`__CFRunLoopDoTimers` 函数执行 CFRunLoopTimerRef 的回调函数并更新其 `_fireTSR` 和 `_nextFireDate`。
```c++
/* rl and rlm are locked on entry and exit */

/* 
* 进入 __CFRunLoopDoTimers 函数前 rl 和 rlm 的 _lock 都已经加锁了，
* 在 __CFRunLoopDoTimers 函数内部当需要执行回调时，会对 rl 和 rlm 进行解锁，
* 然后在回调函数执行完成后，会重新对 rl 和 rlm 进行加锁。
*/

static Boolean __CFRunLoopDoTimers(CFRunLoopRef rl, CFRunLoopModeRef rlm, uint64_t limitTSR) {    /* DOES CALLOUT */
    // timerHandled 标记是否执行了 timer 的回调事件
    Boolean timerHandled = false;
    
    // 用于收集 rlm->_timers 中待要触发的计时器
    CFMutableArrayRef timers = NULL;
    
    // 遍历 rlm 的 _timers 数组中的 CFRunLoopTimerRef，如果 CFRunLoopTimerRef 是 Valid 并且非 Firing 状态并且其 _fireTSR 小于等于 limitTSR 时间，则把其添加到 timers 中
    // （大于 limitTSR 的本次 timer 回调会被忽略）
    for (CFIndex idx = 0, cnt = rlm->_timers ? CFArrayGetCount(rlm->_timers) : 0; idx < cnt; idx++) {
        CFRunLoopTimerRef rlt = (CFRunLoopTimerRef)CFArrayGetValueAtIndex(rlm->_timers, idx);
        // rlt 是 Valid 的并且当前是未在执行的
        if (__CFIsValid(rlt) && !__CFRunLoopTimerIsFiring(rlt)) {
            if (rlt->_fireTSR <= limitTSR) {
                // 首次进来需要为 timers 申请空间
                if (!timers) timers = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
                
                // 把 rlt 添加到 timers 中
                CFArrayAppendValue(timers, rlt);
            }
        }
    }
    
    // 遍历 timers 数组，执行 CFRunLoopTimerRef 的回调函数
    for (CFIndex idx = 0, cnt = timers ? CFArrayGetCount(timers) : 0; idx < cnt; idx++) {
        CFRunLoopTimerRef rlt = (CFRunLoopTimerRef)CFArrayGetValueAtIndex(timers, idx);
        
        // __CFRUNLOOP_IS_CALLING_OUT_TO_A_TIMER_CALLBACK_FUNCTION__ 执行 CFRunLoopTimerRef 的回调函数，
        // 并更新其 _fireTSR 和 _nextFireDate。
        
        Boolean did = __CFRunLoopDoTimer(rl, rlm, rlt);
        
        // did 为 true 则 timerHandled 也为 true，否则为 false
        timerHandled = timerHandled || did;
    }
    
    // 释放 timers
    if (timers) CFRelease(timers);
    return timerHandled;
}
```
&emsp;`__CFRunLoopRun` 函数到这里就看完了，包含 observers/sources0/sources1/timers/block 的执行逻辑，以及 run loop 休眠唤醒的逻辑。 port 和 mach_msg 相关的内容还要开一篇系统学习一下。⛽️⛽️

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
