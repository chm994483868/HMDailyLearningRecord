# iOS 从源码解析Run Loop (八)：Run Loop 与 AutoreleasePool、NSTimer、PerformSelector 系列函数

> &emsp;本篇学习我们日常开发中涉及到 run loop 的一些知识点，我们使用它们的时候可能不会想到这些知识点的背后其实都是 run loop 在做支撑的。

## 回顾 run loop mode item
&emsp;我们首先再次回顾一下 Source/Timer/Observer，因为 run loop 正是通过这些 run loop mode item 来向外提供功能支持的。

1. CFRunLoopSourceRef 是事件产生的地方。Source 有两个版本：Source0 和 Source1。
+ Source0 只包含了一个回调（函数指针），它并不能主动触发事件。使用时，你需要先调用 CFRunLoopSourceSignal(source)，将这个 Source 标记为待处理，然后手动调用 CFRunLoopWakeUp(runloop) 来唤醒 run loop，让其处理这个事件。
+ Source1 包含了一个 mach_port 和一个回调（函数指针），被用于通过内核和其他线程相互发送消息（mach_msg），这种 Source 能主动唤醒 run loop 的线程。

&emsp;下面看一下它们相关的数据结构，CFRunLoopSourceContext 和 CFRunLoopSourceContext1 具有一些相同的字段和不同字段。
```c++
typedef struct {
    CFIndex version;
    void * info; // 作为 perform 函数的参数
    const void *(*retain)(const void *info); // retain 函数
    void (*release)(const void *info); // release 函数
    CFStringRef (*copyDescription)(const void *info); // 返回描述字符串的函数
    Boolean (*equal)(const void *info1, const void *info2); // 判断 source 对象是否相等的函数
    CFHashCode (*hash)(const void *info); // 哈希函数
    ...
} CFRunLoopSourceContext/1;
```
&emsp;version、info、retain 函数、release 函数、描述字符串的函数、判断 source 对象是否相等的函数、哈希函数，是 CFRunLoopSourceContext 和 CFRunLoopSourceContext1 的基础内容双方完全等同，两者的区别主要在下面，它们表示了 source0 和 source1 的不同功能。
```c++
typedef struct {
    ...
    // 当 source0 加入到 run loop 时触发的回调函数（在 CFRunLoopAddSource 函数内部可看到 schedule 被调用）
    void (*schedule)(void *info, CFRunLoopRef rl, CFStringRef mode); 
    
    void (*cancel)(void *info, CFRunLoopRef rl, CFStringRef mode); // 当 source0 从 run loop 中移除时触发的回调函数
    
    // source0 要执行的任务块，当 source0 事件被触发时的回调, 调用 __CFRUNLOOP_IS_CALLING_OUT_TO_A_SOURCE0_PERFORM_FUNCTION__ 函数来执行 perform
    void (*perform)(void *info); 
} CFRunLoopSourceContext;
```
```c++
typedef struct {
    ...
#if (TARGET_OS_MAC && !(TARGET_OS_EMBEDDED || TARGET_OS_IPHONE)) || (TARGET_OS_EMBEDDED || TARGET_OS_IPHONE)
    // getPort 函数指针，用于当 source1 被添加到 run loop 中的时候，从该函数中获取具体的 mach_port_t 对象，用来唤醒 run loop。
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
&emsp;可看到 Source0 中仅有一些回调函数会在 run loop 的本次循环中执行，而 Source1 中有 mach port 可用来主动唤醒 run loop。

2. CFRunLoopTimerRef 是基于时间的触发器，它和 NSTimer 是 toll-free bridged 的，可以混用。其包含一个时间长度和一个回调（函数指针）。当其加入到 run loop 时，run loop 会注册对应的时间点，当时间点到时，run loop会被唤醒以执行那个回调。
3. CFRunLoopObserverRef 是观察者，每个 Observer 都包含了一个回调（函数指针），当 run loop 的状态发生变化时，观察者就能通过这个回调接收到。

## 观察 run loop 的状态变化/观察 run loop mode 的切换
&emsp;下面是观察主线程 run loop 的状态变化以及当前 run loop mode 切换（kCFRunLoopDefaultMode 和 UITrackingRunLoopMode 的切换）的部分示例代码，其中在 ViewController 上添加一个能滚动的 tableView 的代码可自行添加:
```c++
- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    // 只给 void * info 字段传入了初始值，它会作为下面 mainRunLoopActivitie 回调函数的 info 参数
    CFRunLoopObserverContext context = {0, (__bridge void *)(self), NULL, NULL, NULL};
    
    // kCFRunLoopAllActivities 参数表示观察 run loop 的所有状态变化 
    // YES 表示重复观察 run lop 的状态变化
    // mainRunLoopActivitie 对应于 __CFRunLoopObserver 结构体的 _callout 字段，是 run loop 状态变化时的回调函数
    // 0 是对应 __CFRunLoopObserver 的 _order 字段，当一个 run loop 添加的了多个 run loop observer 时，_order 会作为它们的调用顺序的依据，_order 值越小优先级越高，
    // context 是上下文，这里主要用来传递 info 了。
    CFRunLoopObserverRef observer = CFRunLoopObserverCreate(kCFAllocatorDefault, kCFRunLoopAllActivities, YES, 0, mainRunLoopActivitie, &context);
    if (observer) {
        // 把 observer 添加到 main run loop 的 kCFRunLoopCommonModes 模式下
        CFRunLoopAddObserver(CFRunLoopGetMain(), observer, kCFRunLoopCommonModes);
        CFRelease(observer);
    }
}

int count = 0; // 定义全局变量来计算一个 mode 中状态切换的统计数据
void mainRunLoopActivitie(CFRunLoopObserverRef observer, CFRunLoopActivity activity, void *info) {
    // observer：上面 viewDidLoad 函数中添加到 main run loop 的 CFRunLoopObserverRef 实例
    // activity：本次的状态变化：kCFRunLoopEntry、kCFRunLoopBeforeTimers、kCFRunLoopBeforeSources、kCFRunLoopBeforeWaiting、kCFRunLoopAfterWaiting、kCFRunLoopExit、（kCFRunLoopAllActivities）
    // info：上面 viewDidLoad 函数中 CFRunLoopObserverContext 实例的 info 成员变量，上面是 (__bridge void *)(self)
    
    ++count;
    switch (activity) {
        case kCFRunLoopEntry:
            count = 0;
            NSLog(@"🤫 - %d kCFRunLoopEntry 即将进入: %@", count, CFRunLoopCopyCurrentMode(CFRunLoopGetCurrent()));
            break;
        case kCFRunLoopBeforeTimers:
            NSLog(@"🤫 - %d kCFRunLoopBeforeTimers 即将处理 timers", count);
            break;
        case kCFRunLoopBeforeSources:
            NSLog(@"🤫 - %d kCFRunLoopBeforeSources 即将处理 sources", count);
            break;
        case kCFRunLoopBeforeWaiting:
            count = 0; // 每次 run loop 即将进入休眠时，count 置为 0，可表示一轮 run loop 循环结束
            NSLog(@"🤫 - %d kCFRunLoopBeforeWaiting 即将进入休眠", count);
            break;
        case kCFRunLoopAfterWaiting:
            NSLog(@"🤫 - %d kCFRunLoopAfterWaiting 即将从休眠中醒来", count);
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

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    NSLog(@"%s",__func__);
}

// 从 App 静止状态点击屏幕空白区域可看到如下打印:
 🤫 - 1 kCFRunLoopAfterWaiting 即将从休眠中醒来
 🤫 - 2 kCFRunLoopBeforeTimers 即将处理 timers
 🤫 - 3 kCFRunLoopBeforeSources 即将处理 sources
 🤫 - 0 kCFRunLoopBeforeWaiting 即将进入休眠 // run loop 1⃣️ 组循环结束
 
 🤫 - 1 kCFRunLoopAfterWaiting 即将从休眠中醒来
 🤫 - 2 kCFRunLoopBeforeTimers 即将处理 timers
 🤫 - 3 kCFRunLoopBeforeSources 即将处理 sources
 -[ViewController touchesBegan:withEvent:] // 由 App 静止状态点击屏幕开始，上面是固定的循环两次才进入 touche 事件
 🤫 - 4 kCFRunLoopBeforeTimers 即将处理 timers
 🤫 - 5 kCFRunLoopBeforeSources 即将处理 sources
 🤫 - 0 kCFRunLoopBeforeWaiting 即将进入休眠 // run loop 2⃣️ 组循环结束
 
 🤫 - 1 kCFRunLoopAfterWaiting 即将从休眠中醒来
 🤫 - 2 kCFRunLoopBeforeTimers 即将处理 timers
 🤫 - 3 kCFRunLoopBeforeSources 即将处理 sources
 🤫 - 4 kCFRunLoopBeforeTimers 即将处理 timers
 🤫 - 5 kCFRunLoopBeforeSources 即将处理 sources
 🤫 - 0 kCFRunLoopBeforeWaiting 即将进入休眠 // run loop 3⃣️ 组循环结束
 
 🤫 - 1 kCFRunLoopAfterWaiting 即将从休眠中醒来
 🤫 - 2 kCFRunLoopBeforeTimers 即将处理 timers
 🤫 - 3 kCFRunLoopBeforeSources 即将处理 sources
 🤫 - 4 kCFRunLoopBeforeTimers 即将处理 timers
 🤫 - 5 kCFRunLoopBeforeSources 即将处理 sources
 🤫 - 0 kCFRunLoopBeforeWaiting 即将进入休眠 // run loop 4⃣️ 组循环结束
 
 // 下面则是固定的循环两次后 App 进入静止状态。
 
 🤫 - 1 kCFRunLoopAfterWaiting 即将从休眠中醒来
 🤫 - 2 kCFRunLoopBeforeTimers 即将处理 timers
 🤫 - 3 kCFRunLoopBeforeSources 即将处理 sources
 🤫 - 0 kCFRunLoopBeforeWaiting 即将进入休眠 // run loop 5⃣️ 组循环结束
 
 🤫 - 1 kCFRunLoopAfterWaiting 即将从休眠中醒来
 🤫 - 2 kCFRunLoopBeforeTimers 即将处理 timers
 🤫 - 3 kCFRunLoopBeforeSources 即将处理 sources
 🤫 - 0 kCFRunLoopBeforeWaiting 即将进入休眠 // run loop 6⃣️ 组循环结束
 // 此后 run loop 进入长久休眠
```
&emsp;首先运行模式切换相关，当我们从静止状态滚动 tableView 的时候，会看到 `🤫 - 0 kCFRunLoopExit 即将退出: kCFRunLoopDefaultMode` 和 `🤫 - 0 kCFRunLoopEntry 即将进入: UITrackingRunLoopMode`，当滑动停止的时候又会看到 `🤫 - 0 kCFRunLoopExit 即将退出: UITrackingRunLoopMode` 和 `🤫 - 0 kCFRunLoopEntry 即将进入: kCFRunLoopDefaultMode`。即从 Default 退出进入 UITracking，然后滑动停止后是退出 UITracking 再进入 Default。

&emsp;状态切换的话是，从程序静止状态时，点击屏幕空白区域，则是固定的 `AfterWaiting -> BeforeTimers -> BeforeSources` 然后进入休眠 `BeforeWaiting`，然后是再来一次 `AfterWaiting -> BeforeTimers -> BeforeSources` 后才会执行 `touchesBegan:withEvent:` 回调，即 run loop 唤醒之后不是立马处理 touch 事件的，而是看看 timer 有没有事情，然后是 sources（这里是一个 source0），且第一轮是不执行 touch 事件回调，第二轮才会执行 touch 事件回调，然后是固定循环两轮后程序进入长久休眠状态。

&emsp;当 main run loop 的状态发生变化时会调用 mainRunLoopActivitie 函数，我们可以在其中根据 activity 做想要的处理。具体详细的 CFRunLoopObserverCreate 和 CFRunLoopAddObserver 函数的实现过程在前面都已经分析过，可以参考前面 [iOS 从源码解析Run Loop (四)：Source、Timer、Observer 创建以及添加到 mode 的过程](https://juejin.cn/post/6908639874857828366)

## 线程保活
&emsp;线程为什么需要保活？性能其实很大的瓶颈是在于空间的申请和释放，当我们执行一个任务的时候创建了一个线程，任务结束就释放该线程，如果任务频率比较高，那么一个一直活跃的线程来执行我们的任务就省去申请和释放空间的时间和性能。前面已经讲过了 run loop 需要有 source0/source1/timer/block（\__CFRunLoopModeIsEmpty 函数前面详细分析过） 才能不退出，总不可能直接让他执行 while(1) 吧，这种方法明显不对的，由源码得知，当有监测端口（mach port）的时候（即有 source1 时），也不会退出，也不会影响性能，所以在线程初始化的时候可以使用 `[[NSRunLoop currentRunLoop] addPort:[NSPort port] forMode:NSRunLoopCommonModes];` 来保证 run loop 启动后保活。（CFRunLoopRunSpecific 函数内调用 \__CFRunLoopModeIsEmpty 函数返回 ture 的话，会直接返回 kCFRunLoopRunFinished）

&emsp;如果想让子线程永久保持活性那么就在子线程内调用其 run loop 实例的 run 函数，如果想自由控制线程 run loop 结束时机的话则使用一个变量控制 do while 循环，在循环内部调用子线程的 run loop 实例的 runMode: beforeDate: 函数，当需要停止子线程的 run loop 时则在子线程内调用 `CFRunLoopStop(CFRunLoopGetCurrent());` 并结束 do while 循环，详细内容可参考前面 [iOS 从源码解析Run Loop (一)：run loop 基本概念理解与 NSRunLoop 文档](https://juejin.cn/post/6904921175546298375)

## 控制自动释放池的 push 和 pop
&emsp;自动释放池什么时候执行 pop 操作把池中的对象的都执行一次 release  呢？这里要分两种情况：
+ 一种是我们手动以 `@autoreleasepool {...}`  的形式添加的自动释放池，使用 clang -rewrite-objc 转换为 C++ 后其实是
```c++
struct __AtAutoreleasePool {
  __AtAutoreleasePool() {atautoreleasepoolobj = objc_autoreleasePoolPush();}
  ~__AtAutoreleasePool() {objc_autoreleasePoolPop(atautoreleasepoolobj);}
  void * atautoreleasepoolobj;
};

/* @autoreleasepool */ 
{ 
    // 直接构建了一个 __AtAutoreleasePool 实例，
    // 构造函数调用了 AutoreleasePoolPage 的 push 函数，构建了一个自动释放池。
    __AtAutoreleasePool __autoreleasepool;
    // ...
}
```
&emsp;可看到 `__autoreleasepool` 是被包裹在一对 `{}` 之中的，当出了右边花括号时自动释放池便会执行 pop 操作，也可理解为如下代码:
```c++
void *pool = objc_autoreleasePoolPush();
// {}中的代码
objc_autoreleasePoolPop(pool);
```
&emsp;在原始 main 函数中，打一个断点，并开启 Debug Workflow 的 Always Show Disassembly 可看到对应的汇编代码：
```c++
int main(int argc, char * argv[]) {
    NSString * appDelegateClassName;
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
        // appDelegateClassName = NSStringFromClass([AppDelegate class]);
    } // ⬅️ 在这里打一个断点
    return UIApplicationMain(argc, argv, nil, appDelegateClassName);
}
```
&emsp;由于上面代码中自动释放池什么也没有放，Push 完便接着 Pop 了。
```c++
...
0x101319b78 <+32>:  bl     0x101319eb8               ; symbol stub for: objc_autoreleasePoolPush
0x101319b7c <+36>:  bl     0x101319eac               ; symbol stub for: objc_autoreleasePoolPop
...
```
+ 一种是由 run loop 创建的自动释放池。ibireme 大佬如是说:
> &emsp;App 启动后，苹果在主线程 RunLoop 里注册了两个 Observer，其回调都是 \_wrapRunLoopWithAutoreleasePoolHandler()。
> &emsp;第一个 Observer 监视的事件是 Entry(即将进入Loop)，其回调内会调用 \_objc_autoreleasePoolPush() 创建自动释放池。其 order 是-2147483647，优先级最高，保证创建释放池发生在其他所有回调之前。
> &emsp;第二个 Observer 监视了两个事件： BeforeWaiting(准备进入休眠) 时调用 \_objc_autoreleasePoolPop() 和 \_objc_autoreleasePoolPush() 释放旧的池并创建新池；Exit(即将退出Loop) 时调用 \_objc_autoreleasePoolPop() 来释放自动释放池。这个 Observer 的 order 是 2147483647，优先级最低，保证其释放池子发生在其他所有回调之后。
> &emsp;在主线程执行的代码，通常是写在诸如事件回调、Timer 回调内的。这些回调会被 RunLoop 创建好的 AutoreleasePool 环绕着，所以不会出现内存泄漏，开发者也不必显示创建 Pool 了。[深入理解RunLoop](https://blog.ibireme.com/2015/05/18/runloop/)

&emsp;下面我们试着验证一下上面的结论，在 application:didFinishLaunchingWithOptions: 函数中添加一个断点，在控制台打印 po [NSRunLoop mainRunLoop]，可看到在 main run loop 的 kCFRunLoopDefaultMode 和 UITrackingRunLoopMode 模式下的 observers 中均有如下两个 CFRunLoopObserver。
```c++
"<CFRunLoopObserver 0x600001c30320 [0x7fff80617cb0]>{valid = Yes, activities = 0x1, repeats = Yes, order = -2147483647, callout = _wrapRunLoopWithAutoreleasePoolHandler (0x7fff4808bf54), context = <CFArray 0x60000235dc20 [0x7fff80617cb0]>{type = mutable-small, count = 0, values = ()}}"

"<CFRunLoopObserver 0x600001c30280 [0x7fff80617cb0]>{valid = Yes, activities = 0xa0, repeats = Yes, order = 2147483647, callout = _wrapRunLoopWithAutoreleasePoolHandler (0x7fff4808bf54), context = <CFArray 0x60000235dc20 [0x7fff80617cb0]>{type = mutable-small, count = 0, values = ()}}"
```
&emsp;order 是 -2147483647 的 CFRunLoopObserver 优先级最高，会在其它所有 CFRunLoopObserver 之前回调，然后它的 activities 是 0x1，对应 kCFRunLoopEntry = (1UL << 0)，即只观察 kCFRunLoopEntry 状态，回调函数是 \_wrapRunLoopWithAutoreleasePoolHandler，添加一个 \_wrapRunLoopWithAutoreleasePoolHandler 符号断点，添加一个 objc_autoreleasePoolPush 符号断点，运行程序，并在控制台 bt 打印函数堆栈，确实能看到如下的函数调用：
```c++
(lldb) bt
* thread #1, queue = 'com.apple.main-thread', stop reason = breakpoint 3.1
  * frame #0: 0x00000001dd971864 libobjc.A.dylib`objc_autoreleasePoolPush // push 构建自动释放池
    frame #1: 0x00000001de78d61c CoreFoundation`_CFAutoreleasePoolPush + 16
    frame #2: 0x000000020af66324 UIKitCore`_wrapRunLoopWithAutoreleasePoolHandler + 56
    frame #3: 0x00000001de7104fc CoreFoundation`__CFRUNLOOP_IS_CALLING_OUT_TO_AN_OBSERVER_CALLBACK_FUNCTION__ + 32 // 执行 run loop observer 回调函数，
    frame #4: 0x00000001de70b224 CoreFoundation`__CFRunLoopDoObservers + 412
    frame #5: 0x00000001de70af9c CoreFoundation`CFRunLoopRunSpecific + 412
    frame #6: 0x00000001e090c79c GraphicsServices`GSEventRunModal + 104
    frame #7: 0x000000020af6cc38 UIKitCore`UIApplicationMain + 212
    frame #8: 0x0000000100a75b90 Simple_iOS`main(argc=1, argv=0x000000016f38f8e8) at main.m:77:12
    frame #9: 0x00000001de1ce8e0 libdyld.dylib`start + 4
(lldb) 
```
&emsp;在主线程中确实看到了 `__CFRUNLOOP_IS_CALLING_OUT_TO_AN_OBSERVER_CALLBACK_FUNCTION__` 执行 CFRunLoopObserver 的回调函数调用了 `_wrapRunLoopWithAutoreleasePoolHandler` 函数接着调用了 `objc_autoreleasePoolPush` 创建自动释放池。

&emsp;order 是 2147483647 的 CFRunLoopObserver 优先级最低，会在其它所有 CFRunLoopObserver 之后回调，然后它的 activities 是 0xa0（0b10100000），对应 kCFRunLoopBeforeWaiting = (1UL << 5) 和 kCFRunLoopExit = (1UL << 7)，即观察 run loop 的即将进入休眠和 run loop 退出的两个状态变化，回调函数的话也是 \_wrapRunLoopWithAutoreleasePoolHandler，我们再添加一个 objc_autoreleasePoolPop 符号断点，此时需要我们添加一些测试代码，我们添加一个 main run loop 的观察者，然后再添加一个主线程的 main run loop 的 timer，程序启动后我们可看到控制台如下循环打印:
```c++
 🎯... kCFRunLoopAfterWaiting
 ⏰⏰⏰ timer 回调...
 🎯... kCFRunLoopBeforeTimers
 🎯... kCFRunLoopBeforeSources
 🎯... kCFRunLoopBeforeWaiting
 🎯... kCFRunLoopAfterWaiting
 ⏰⏰⏰ timer 回调...
```
&emsp;主线程进入了一种 “休眠--被 timer 唤醒执行回调--休眠” 的循环之中，此时我们打开 `_wrapRunLoopWithAutoreleasePoolHandler` 断点发现程序进入，然后再打开 objc_autoreleasePoolPop 断点，然后点击 Continue program execution 按钮，此时会进入 objc_autoreleasePoolPop 断点，在控制台 bt 打印函数调用栈：
```c++
(lldb) bt
* thread #1, queue = 'com.apple.main-thread', stop reason = breakpoint 1.1
  * frame #0: 0x00000001dd9718f8 libobjc.A.dylib`objc_autoreleasePoolPop
    frame #1: 0x00000001de78cba0 CoreFoundation`_CFAutoreleasePoolPop + 28
    frame #2: 0x000000020af66360 UIKitCore`_wrapRunLoopWithAutoreleasePoolHandler + 116
    frame #3: 0x00000001de7104fc CoreFoundation`__CFRUNLOOP_IS_CALLING_OUT_TO_AN_OBSERVER_CALLBACK_FUNCTION__ + 32
    frame #4: 0x00000001de70b224 CoreFoundation`__CFRunLoopDoObservers + 412
    frame #5: 0x00000001de70b7a0 CoreFoundation`__CFRunLoopRun + 1228
    frame #6: 0x00000001de70afb4 CoreFoundation`CFRunLoopRunSpecific + 436
    frame #7: 0x00000001e090c79c GraphicsServices`GSEventRunModal + 104
    frame #8: 0x000000020af6cc38 UIKitCore`UIApplicationMain + 212
    frame #9: 0x0000000100bc9b2c Simple_iOS`main(argc=1, argv=0x000000016f23b8e8) at main.m:76:12
    frame #10: 0x00000001de1ce8e0 libdyld.dylib`start + 4
(lldb)
```
&emsp;确实看到了 \_wrapRunLoopWithAutoreleasePoolHandler 调用了 objc_autoreleasePoolPop。

&emsp;这样整体下来：Entry-->push ➡️ BeforeWaiting--->pop-->push ➡️ Exit-->pop，按照这样的顺序，保证了在每次 run loop 循环中都进行一次 push 和 pop。

&emsp;从上面 run loop observer 工作便知，每一次 loop，便会有一次 pop 和 push，因此我们得出：

1. 如果手动添加 autoreleasePool，autoreleasePool 作用域里的自动释放对象会在出 pool 作用域的那一刻释放。
2. 如果是 run loop 自动添加的 autoreleasePool，首先在 run loop 循环开启时 push 一个新的自动释放池，然后在每一次 run loop 循环将要进入休眠时 autoreleasePool 执行 pop 操作释放这次循环中所有的自动释放对象，并同时再 push 一个新的自动释放池在下一个 loop 循环中使用，这样保证 run loop 的每次循环中的创建的自动释放对象都得到释放，然后在 run loop 切换 mode 退出时，再执行最后一次 pop，保证在 run loop 的运行过程中自动释放池的 push 和 pop 成对出现。

## NSTimer 实现过程
&emsp;NSTimer.h 中提供了一组 NSTimer 的创建方法，其中不同构造函数的 NSInvocation、SEL、block 类型的参数分别代表 NSTimer 对象的不同的回调方式。其中 block  的回调形式是 iOS 10.0 后新增的，可以帮助我们避免 NSTimer 对象和其 target 参数的循环引用问题，`timerWithTimeInterval...` 和 `initWithFireDate` 返回的 NSTimer 对象还需要我们手动添加到当前线程的 run loop 中，`scheduledTimerWithTimeInterval...` 构建的 NSTimer 对象则是默认添加到当前线程的 run loop 的 NSDefaultRunLoopMode 模式下的（必要的情况下我们还要再补一行把 timer 添加到当前线程的 run loop 的 NSRunLoopCommonModes 模式下）。

&emsp;block 回调的形式都有一个 `API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0));` 标注，表示是 iOS 10 后新增的。

### NSTimer 创建函数
&emsp;下面五个方法返回的 NSTimer 对象需要手动调用 NSRunLoop 的 `-(void)addTimer:(NSTimer *)timer forMode:(NSRunLoopMode)mode;` 函数添加到指定 run loop 的指定 mode 下。
```c++
+ (NSTimer *)timerWithTimeInterval:(NSTimeInterval)ti invocation:(NSInvocation *)invocation repeats:(BOOL)yesOrNo;
+ (NSTimer *)timerWithTimeInterval:(NSTimeInterval)ti target:(id)aTarget selector:(SEL)aSelector userInfo:(nullable id)userInfo repeats:(BOOL)yesOrNo;
+ (NSTimer *)timerWithTimeInterval:(NSTimeInterval)interval repeats:(BOOL)repeats block:(void (^)(NSTimer *timer))block API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0));
- (instancetype)initWithFireDate:(NSDate *)date interval:(NSTimeInterval)interval repeats:(BOOL)repeats block:(void (^)(NSTimer *timer))block API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0));
- (instancetype)initWithFireDate:(NSDate *)date interval:(NSTimeInterval)ti target:(id)t selector:(SEL)s userInfo:(nullable id)ui repeats:(BOOL)rep NS_DESIGNATED_INITIALIZER;
```
&emsp;下面三个方法返回的 NSTimer 对象会被自动添加到当前线程的 run loop 的 default mode 下。
```c++
+ (NSTimer *)scheduledTimerWithTimeInterval:(NSTimeInterval)ti invocation:(NSInvocation *)invocation repeats:(BOOL)yesOrNo;
+ (NSTimer *)scheduledTimerWithTimeInterval:(NSTimeInterval)ti target:(id)aTarget selector:(SEL)aSelector userInfo:(nullable id)userInfo repeats:(BOOL)yesOrNo;
+ (NSTimer *)scheduledTimerWithTimeInterval:(NSTimeInterval)interval repeats:(BOOL)repeats block:(void (^)(NSTimer *timer))block API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0));
```
&emsp;如果使用 `scheduledTimerWithTimeInterval...` 则需要注意 run loop 的 mode 切换到 UITrackingRunLoopMode 模式时，计时器会停止回调，当滑动停止 run loop 切回到 kCFRunLoopDefaultMode 模式时计时器又开始正常回调，必要情况下我们需要把 timer 添加到  NSRunLoopCommonModes 模式下可保证 run loop 的 mode 切换不影响计时器的回调（此时的计时器对象会被同时添加到多个 common 标记的 run loop mode 的 \_timers 中）。

&emsp;还有一个知识点需要注意一下，添加到 run loop 指定 mode 下的 NSTimer 会被 mode 所持有，因为它会被加入到 run loop mode 的 \_timers 中去，如果 mode name 是 NSRunLoopCommonModes 的话，同时还会被加入到 run loop 的 \_commonModeItems 中，所以当不再需要使用  NSTimer 对象计时时必须调用 invalidate 函数把它从 \_timers 和 \_commonModeItems 集合中移除。如下代码在 ARC 下打印各个计时器的引用计数可进行证实：
```c++
// timer 默认添加到 run loop 的 NSDefaultRunLoopMode 下，引用计数应该是 3 (觉得这里应该是 2 呀？)
NSTimer *timer = [NSTimer scheduledTimerWithTimeInterval:1 repeats:YES block:^(NSTimer * _Nonnull timer) { }]; // 3

// 起始引用计数是 1
NSTimer *timer2 = [[NSTimer alloc] initWithFireDate:[NSDate date] interval:1 repeats:YES block:^(NSTimer * _Nonnull timer) { }]; // 1
// 把 timer2 添加到 run loop 的 NSDefaultRunLoopMode 时引用计数 +1  
// 被 timer2 和 NSDefaultRunLoopMode 的 _timers 持有
[[NSRunLoop currentRunLoop] addTimer:timer2 forMode:NSDefaultRunLoopMode]; // 2

NSTimer *timer3 = [NSTimer timerWithTimeInterval:1 repeats:YES block:^(NSTimer * _Nonnull timer) { }]; // 1
[[NSRunLoop currentRunLoop] addTimer:timer3 forMode:NSDefaultRunLoopMode]; // 2

// 把 timer3 添加到 run loop 的 NSRunLoopCommonModes 时引用计数 +3 
// 被 timer3、UITrackingRunLoopMode 的 _timers、NSDefaultRunLoopMode 的 _timers、run loop 的 _commonModeItems 持有
[[NSRunLoop currentRunLoop] addTimer:timer3 forMode:NSRunLoopCommonModes]; // 4

// timer3 调用 invalidate 函数后引用计数变回 1
// 被从两个 _timers 和 _commonModeItems 中移除后 -3
[timer3 invalidate]; // 1
```

&emsp;NSTimer 创建时会持有传入的 target:
```c++
+ (NSTimer *)timerWithTimeInterval:(NSTimeInterval)ti target:(id)aTarget selector:(SEL)aSelector userInfo:(nullable id)userInfo repeats:(BOOL)yesOrNo;
- (instancetype)initWithFireDate:(NSDate *)date interval:(NSTimeInterval)ti target:(id)t selector:(SEL)s userInfo:(nullable id)ui repeats:(BOOL)rep NS_DESIGNATED_INITIALIZER;
+ (NSTimer *)scheduledTimerWithTimeInterval:(NSTimeInterval)ti target:(id)aTarget selector:(SEL)aSelector userInfo:(nullable id)userInfo repeats:(BOOL)yesOrNo;
```
&emsp;使用以上三个函数构建或初始化 NSTimer 对象时，NSTimer 对象会持有传入的 target 的，因为 NSTimer 对象回调时要执行 target 的 aSelector 函数，如果此时 target 同时也持有 NSTimer 对象的话则会构成循环引用导致内存泄漏，一般在 ViewController 中添加 NSTimer 属性会遇到此问题。解决这个问题的方法通常有两种：一种是将 target 分离出来独立成一个对象（在这个对象中弱引用 NSTimer 并将对象本身作为 NSTimer 的 target），控制器通过这个对象间接使用 NSTimer；另一种方式的思路仍然是转移 target，只是可以直接增加 NSTimer 扩展（分类），让 NSTimer 类对象做为 target，同时可以将操作 selector 封装到 block 中，示例代码如下。（类对象全局唯一且不需要也不能释放）[iOS刨根问底-深入理解RunLoop](https://www.cnblogs.com/kenshincui/p/6823841.html)
```c++
#import "NSTimer+Block.h"

@implementation NSTimer (Block)

- (instancetype)initWithFireDate:(NSDate *)date interval:(NSTimeInterval)seconds repeats:(BOOL)repeats block:(void (^)(void))block {
    // target 传入的是 self.class 即 NSTimer 类对象，然后计时器的回调函数就是 NSTimer 类对象的 runBlock: 函数，runBlock 是一个类方法，
    // 把回调的 block 放在 userInfo 中，然后在计时器的触发函数 runBlock: 中根据 NSTimer 对象读出其 userInfo 即为 block，执行即可。
    return [self initWithFireDate:date interval:seconds target:self.class selector:@selector(runBlock:) userInfo:block repeats:repeats];
}

+ (NSTimer *)scheduledTimerWithTimeInterval:(NSTimeInterval)seconds repeats:(BOOL)repeats block:(void (^)(void))block {
    // self 即为 NSTimer 类对象
    return [self scheduledTimerWithTimeInterval:seconds target:self selector:@selector(runBlock:) userInfo:block repeats:repeats];
}

+ (NSTimer *)timerWithTimeInterval:(NSTimeInterval)seconds repeats:(BOOL)repeats block:(void (^)(void))block {
    // self 即为 NSTimer 类对象
    return [self timerWithTimeInterval:seconds target:self selector:@selector(runBlock:) userInfo:block repeats:repeats];
}

#pragma mark - Private methods

+ (void)runBlock:(NSTimer *)timer {
    // 从入参 timer 对象中读出 block 执行
    if ([timer.userInfo isKindOfClass:NSClassFromString(@"NSBlock")]) {
        void (^block)(void) = timer.userInfo;
        block();
    }
}

@end
```
&emsp;iOS 10.0 以后苹果也提供了 block 形式的 NSTimer 构建函数，我们直接使用即可。（大概现在还有 iOS 10.0 之前的用户吗）

&emsp;看到这里会发现计时器是不能暂停的，invalidate 函数是移除计数器使用的，所以无论是重复执行的计时器还是一次性的计时器只要调用 invalidate 方法则会变得无效，只是一次性的计时器执行完操作后会自动调用 invalidate 方法。所以想要暂停和恢复计时器的只能 invalidate 旧计时器然后再新建计时器，且当我们不再需要使用计时器时必须调用 invalidate 方法。

### NSTimer 执行流程
&emsp;CFRunLoopTimerRef 与 NSTimer 是可以 toll-free bridged（免费桥接转换）的。当 timer 加到 run loop 的时候，run loop 会注册对应的触发时间点，时间到了，run loop 若处于休眠则会被唤醒，执行 timer 对应的回调函数。下面我们沿着 CFRunLoopTimerRef 的源码来完整分析一下计时器的流程。

#### CFRunLoopTimerRef 创建
&emsp;首先是 CFRunLoopTimerRef 的创建函数：(详细分析可参考前面的：[iOS 从源码解析Run Loop (四)：Source、Timer、Observer 创建以及添加到 mode 的过程](https://juejin.cn/post/6908639874857828366))
```c++
CFRunLoopTimerRef CFRunLoopTimerCreate(CFAllocatorRef allocator,
                                       CFAbsoluteTime fireDate,
                                       CFTimeInterval interval,
                                       CFOptionFlags flags,
                                       CFIndex order,
                                       CFRunLoopTimerCallBack callout,
                                       CFRunLoopTimerContext *context);
```
&emsp;`allocator` 是 CF 下为新对象分配内存的分配器，可传 NULL 或 kCFAllocatorDefault。

&emsp;`fireDate` 是计时器第一次触发回调的时间点，然后后续沿着 `interval` 间隔时间连续回调。

&emsp;`interval` 是计时器的连续回调的时间间隔，如果为 0 或负数，计时器将触发一次，然后自动失效。

&emsp;`order` 优先级索引，指示 CFRunLoopModeRef 的 _timers 中不同计时器的回调执行顺序。当前忽略此参数，传递 0。

&emsp;`callout` 计时器触发时调用的回调函数。

&emsp;`context` 保存计时器的上下文信息的结构。该函数将信息从结构中复制出来，因此上下文所指向的内存不需要在函数调用之后继续存在。如果回调函数不需要上下文的信息指针来跟踪状态，则可以为 NULL。其中的 `void * info` 字段内容是 `callout` 函数执行时的参数。

&emsp;CFRunLoopTimerCreate 函数中比较重要的是对触发时间的设置：
```c++
...
// #define TIMER_DATE_LIMIT    4039289856.0
// 如果入参 fireDate 过大，则置为 TIMER_DATE_LIMIT
if (TIMER_DATE_LIMIT < fireDate) fireDate = TIMER_DATE_LIMIT;

// 下次触发的时间
memory->_nextFireDate = fireDate;
memory->_fireTSR = 0ULL;

// 取得当前时间
uint64_t now2 = mach_absolute_time();
CFAbsoluteTime now1 = CFAbsoluteTimeGetCurrent();

if (fireDate < now1) {
    // 如果第一次触发的时间已经过了，则把 _fireTSR 置为当前
    memory->_fireTSR = now2;
} else if (TIMER_INTERVAL_LIMIT < fireDate - now1) {
    // 如果第一次触发的时间点与当前是时间差距超过了 TIMER_INTERVAL_LIMIT，则把 _fireTSR 置为 TIMER_INTERVAL_LIMIT
    memory->_fireTSR = now2 + __CFTimeIntervalToTSR(TIMER_INTERVAL_LIMIT);
} else {
    // 这里则是正常的，如果第一次触发的时间还没有到，则把触发时间设置为当前时间和第一次触发时间点的差值
    memory->_fireTSR = now2 + __CFTimeIntervalToTSR(fireDate - now1);
}
...
```
&emsp;这一部分代码保证计时器第一次触发的时间点正常。下面看一下把创建好的 CFRunLoopModeRef 添加到指定的 run loop 的指定的 run loop mode 下。

#### CFRunLoopAddTimer
&emsp;CFRunLoopAddTimer 函数主要完成把 CFRunLoopTimerRef rlt 插入到 CFRunLoopRef rl 的 CFStringRef modeName 模式下的 \_timer 集合中，如果 modeName 是 kCFRunLoopCommonModes 的话，则把 rlt 插入到 rl 的 \_commonModeItems 中，然后调用 \__CFRunLoopAddItemToCommonModes 函数把 rlt 添加到所有被标记为 common 的 mode 的 \_timer 中，同时也会把 modeName 添加到 rlt 的 \_rlModes 中，记录 rlt 都能在那种 run loop mode 下执行。 
```c++
void CFRunLoopAddTimer(CFRunLoopRef rl, CFRunLoopTimerRef rlt, CFStringRef modeName);
```
&emsp;上面添加完成后，会调用 \__CFRepositionTimerInMode 函数，然后调用 \__CFArmNextTimerInMode，再调用 mk_timer_arm 函数把 CFRunLoopModeRef 的 \_timerPort 和一个时间点注册到系统中，等待着 mach_msg 发消息唤醒休眠中的 run loop 起来执行到达时间的计时器。

#### \__CFArmNextTimerInMode
&emsp;同一个 run loop mode 下的多个 timer 共享同一个 \_timerPort，这是一个循环的流程：注册 timer(mk_timer_arm)—接收 timer(mach_msg)—根据多个 timer 计算离当前最近的下次回调的触发时间点—注册 timer(mk_timer_arm)。

&emsp;在使用 CFRunLoopAddTimer 添加 timer 时的调用堆栈如下：
```c++
CFRunLoopAddTimer
__CFRepositionTimerInMode
    __CFArmNextTimerInMode
        mk_timer_arm
```
&emsp;然后 mach_msg 收到 timer 事件时的调用堆栈如下：
```c++
__CFRunLoopRun
__CFRunLoopDoTimers
    __CFRunLoopDoTimer
        __CFRUNLOOP_IS_CALLING_OUT_TO_A_TIMER_CALLBACK_FUNCTION__
__CFArmNextTimerInMode
    mk_timer_arm 
```
&emsp;每次计时器都会调用 \__CFArmNextTimerInMode 函数，注册计时器的下次回调。休眠中的 run loop 通过当前的 run loop mode 的 \_timerPort 端口唤醒后，在本次 run loop 循环中在 \__CFRunLoopDoTimers 函数中循环调用 \__CFRunLoopDoTimer 函数，执行达到触发时间的 timer 的 \_callout 函数。`__CFRUNLOOP_IS_CALLING_OUT_TO_A_TIMER_CALLBACK_FUNCTION__(rlt->_callout, rlt, context_info);` 是执行计时器的 \_callout 函数。

### NSTimer 不准时问题
&emsp;通过上面的 NSTimer 执行流程可看到计时器的触发回调完全依赖 run loop 的运行（macOS 和 iOS 下都是使用 mk_timer 来唤醒 run loop），使用 NSTimer 之前必须注册到 run loop，但是 run loop 为了节省资源并不会在非常准确的时间点调用计时器，如果一个任务执行时间较长（例如本次 run loop 循环中 source0 事件执行时间过长或者计时器自身回调执行时间过长，都会导致计时器下次正常时间点的回调被延后或者延后时间过长的话则直接忽略这次回调（计时器回调执行之前会判断当前的执行状态 !__CFRunLoopTimerIsFiring(rlt)，如果是计时器自身回调执行时间过长导致下次回调被忽略的情况大概与此标识有关 ）），那么当错过一个时间点后只能等到下一个时间点执行，并不会延后执行（NSTimer 提供了一个 tolerance 属性用于设置宽容度，即当前时间点已经过了计时器的本次触发点，但是超过的时间长度小于 tolerance 的话，那么本次计时器回调还可以正常执行，不过是不准时的延后执行。 tolerance 的值默认是 0，最大值的话是计时器间隔时间 \_interval 的一半，可以根据自身的情况酌情设置 tolerance 的值，（其实还是觉得如果自己的计时器不准时了还是应该从自己写的代码中找原因，自己去找该优化的点，或者是主线实在优化不动的话就把计时器放到子线程中去））。

&emsp;（NSTimer 不是一种实时机制，以 main run loop 来说它负责了所有的主线程事件，例如 UI 界面的操作，负责的运算使当前 run loop 持续的时间超过了计时器的间隔时间，那么计时器下一次回调就被延后，这样就造成 timer 的不准时，计时器有个属性叫做 tolerance (宽容度)，标示了当时间点到后，容许有多少最大误差。如果延后时间过长的话会直接导致计时器本次回调被忽略。）

&emsp;在苹果的 Timer 文档中可看到关于计时精度的描述：[Timer Programming Topics](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/Timers/Articles/timerConcepts.html#//apple_ref/doc/uid/20000806-SW2)
> &emsp;Timing Accuracy
> &emsp;A timer is not a real-time mechanism; it fires only when one of the run loop modes to which the timer has been added is running and able to check if the timer’s firing time has passed. Because of the various input sources a typical run loop manages, the effective resolution of the time interval for a timer is limited to on the order of 50-100 milliseconds. If a timer’s firing time occurs while the run loop is in a mode that is not monitoring the timer or during a long callout, the timer does not fire until the next time the run loop checks the timer. Therefore, the actual time at which the timer fires potentially can be a significant period of time after the scheduled firing time.
> &emsp;
> &emsp;A repeating timer reschedules itself based on the scheduled firing time, not the actual firing time. For example, if a timer is scheduled to fire at a particular time and every 5 seconds after that, the scheduled firing time will always fall on the original 5 second time intervals, even if the actual firing time gets delayed. If the firing time is delayed so far that it passes one or more of the scheduled firing times, the timer is fired only once for that time period; the timer is then rescheduled, after firing, for the next scheduled firing time in the future.

&emsp;计时器不是一种实时机制；仅当已添加计时器的 run loop mode 之一正在运行并且能够检查计时器的触发时间是否经过时，它才会触发。由于典型的 run loop 管理着各种输入源，因此计时器时间间隔的有效分辨率被限制在 50-100 毫秒的数量级。如果在运行循环处于不监视计时器的模式下或长时间调用期间，计时器的触发时间发生，则直到下一次运行循环检查计时器时，计时器才会启动。因此，计时器可能实际触发的时间可能是在计划的触发时间之后的相当长的一段时间。

&emsp;重复计时器会根据计划的触发时间而不是实际的触发时间重新安排自身的时间。例如，如果计划将计时器在特定时间触发，然后每5秒触发一次，则即使实际触发时间被延迟，计划的触发时间也将始终落在原始的5秒时间间隔上。如果触发时间延迟得太远，以至于超过了计划的触发时间中的一个或多个，则计时器在该时间段仅触发一次；计时器会在触发后重新安排为将来的下一个计划的触发时间。

&emsp;如下代码申请一条子线程然后启动它的 run loop，可观察 timer 回调的时间点。
```c++
- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    NSThread *thread = [[NSThread alloc] initWithBlock:^{
    [NSTimer scheduledTimerWithTimeInterval:1 repeats:YES block:^(NSTimer * _Nonnull timer) {
    //  sleep(1);
        NSLog(@"⏰⏰⏰ timer 回调...");
    }];
    
    // 2 秒后在 thread 线程中执行 caculate 函数
    [self performSelector:@selector(caculate) withObject:nil afterDelay:2];
    
    [[NSRunLoop currentRunLoop] run];
    }];
    [thread start];
}

- (void)caculate {
    NSLog(@"👘👘 %@", [NSThread currentThread]);
    sleep(2);
}
```
&emsp;运行代码根据打印时间可看到前两秒计时器正常执行，然后 caculate 的执行导致定时器执行被延后两秒，两秒以后计时器继续正常的每秒执行一次。如果把计时器的回调中的  sleep(1) 注释打开，会发现计时器是每两秒执行一次。

## performSelector 系列函数
&emsp;当调用 NSObject 的 performSelecter:afterDelay: 后，实际上其内部会创建一个 Timer 并添加到当前线程的 run loop 中。所以如果当前线程没有 run loop，则这个方法会失效。

### NSObject + NSDelayedPerforming
&emsp;在 NSObject 的 NSDelayedPerforming 分类下声明了如下函数。 
```c++
@interface NSObject (NSDelayedPerforming)
// 指定 NSRunLoopMode
- (void)performSelector:(SEL)aSelector withObject:(nullable id)anArgument afterDelay:(NSTimeInterval)delay inModes:(NSArray<NSRunLoopMode> *)modes;
// 默认在 NSDefaultRunLoopMode
- (void)performSelector:(SEL)aSelector withObject:(nullable id)anArgument afterDelay:(NSTimeInterval)delay;

+ (void)cancelPreviousPerformRequestsWithTarget:(id)aTarget selector:(SEL)aSelector object:(nullable id)anArgument;
+ (void)cancelPreviousPerformRequestsWithTarget:(id)aTarget;
@end
```
&emsp;performSelector:withObject:afterDelay:inModes: 在延迟之后使用指定的模式在当前线程上调用接收方（NSObject 及其子类对象）的方法。

&emsp;`aSelector`：一个选择器，用于标识要调用的方法。该方法应该没有明显的返回值（void），并且应该采用 id 类型的单个参数，或者不带参数。

&emsp;`anArgument`：调用时传递给方法的参数。如果该方法不接受参数，则传递 nil。

&emsp;`delay`：发送消息之前的最短时间。指定延迟 0 不一定会导致选择器立即执行。选择器仍在线程的 run loop 中排队并尽快执行。

&emsp;`modes`：一个字符串数组，用于标识与执行选择器的 timer 关联的模式。此数组必须至少包含一个字符串。如果为此参数指定 nil 或空数组，则此方法将返回而不执行指定的选择器。

&emsp;此方法设置一个 timer，以便在当前线程的 run loop 上执行 aSelector 消息。timer 配置在 modes 参数指定的模式下运行。当 timer 触发时，线程尝试从 run loop 中取出消息并执行选择器。如果 run loop 正在运行并且处于指定的模式之一，则它成功；否则， timer 将等待直到 run loop 处于这些模式之一。关于它会在当前 run loop 的 run loop mode 下添加一个 timer 可通过如下代码验证：
```c++
    NSThread *thread = [[NSThread alloc] initWithBlock:^{
        NSLog(@"🧗‍♀️🧗‍♀️ ....");

        [NSTimer scheduledTimerWithTimeInterval:1 repeats:YES block:^(NSTimer * _Nonnull timer) {
            NSLog(@"⏰⏰⏰ timer 回调...");
        }];

        [self performSelector:@selector(caculate) withObject:nil afterDelay:2]; // ⬅️ 断点 1
        
        NSRunLoop *runloop = [NSRunLoop currentRunLoop]; // ⬅️ 断点 2
        [runloop run];
    }];
    [thread start];
```
&emsp;分别在执行到以上两个断点时，在控制台通过 `po [NSRunLoop currentRunLoop]` 打印:
```c++
// 断点 1 处：po [NSRunLoop currentRunLoop]
...
    timers = <CFArray 0x28314e9a0 [0x20e729430]>{type = mutable-small, count = 1, values = (
    0 : <CFRunLoopTimer 0x28204df80 [0x20e729430]>{valid = Yes, firing = No, interval = 1, tolerance = 0, next fire date = 631096717 (-14.273319 @ 16571855540445), callout = (NSTimer) [_NSTimerBlockTarget fire:] (0x1df20764c / 0x1df163018) (/System/Library/Frameworks/Foundation.framework/Foundation), context = <CFRunLoopTimer context 0x28154b900>}
)
...
// 断点 2 处：po [NSRunLoop currentRunLoop]
...
    timers = <CFArray 0x28314e9a0 [0x20e729430]>{type = mutable-small, count = 2, values = (
    0 : <CFRunLoopTimer 0x28204df80 [0x20e729430]>{valid = Yes, firing = No, interval = 1, tolerance = 0, next fire date = 631096717 (-32.979197 @ 16571855540445), callout = (NSTimer) [_NSTimerBlockTarget fire:] (0x1df20764c / 0x1df163018) (/System/Library/Frameworks/Foundation.framework/Foundation), context = <CFRunLoopTimer context 0x28154b900>}
    1 : <CFRunLoopTimer 0x28204db00 [0x20e729430]>{valid = Yes, firing = No, interval = 0, tolerance = 0, next fire date = 631096747 (-2.84795797 @ 16572578697099), callout = (Delayed Perform) ViewController caculate (0x1df1f4094 / 0x10093ab88) (/var/containers/Bundle/Application/C2E33DEA-1FB0-48A0-AEDD-2D13AF564389/Simple_iOS.app/Simple_iOS), context = <CFRunLoopTimer context 0x28003d4c0>}
)
...
```
&emsp;可看到 performSelector:withObject:afterDelay: 添加了一个 timer。

&emsp;如果希望在 run loop 处于默认模式以外的模式时使消息出列，请使用 `performSelector:withObject:afterDelay:inModes:` 方法。如果不确定当前线程是否为主线程，可以使用 `performSelectorOnMainThread:withObject:waitUntilDone:` 或 `performSelectorOnMainThread:withObject:waitUntilDone:modes:` 方法来确保选择器在主线程上执行。要取消排队的消息，请使用 `cancelPreviousPerformRequestsWithTarget:` 或 `cancelPreviousPerformRequestsWithTarget:selector:object:` 方法。

&emsp;此方法向其当前上下文的 runloop 注册，并依赖于定期运行的 runloop 才能正确执行。一个常见的上下文是当调度队列调用时，你可能调用此方法并最终注册到一个不自动定期运行的 runloop。如果在调度队列上运行时需要此类功能，则应使用 dispatch_after 和相关方法来获得所需的行为。（类似的还有 NSTimer 不准时时，也可以使用 dispatch_source 来替代）

### NSRunLoop + NSOrderedPerform
```c++
@interface NSRunLoop (NSOrderedPerform)
- (void)performSelector:(SEL)aSelector target:(id)target argument:(nullable id)arg order:(NSUInteger)order modes:(NSArray<NSRunLoopMode> *)modes;
- (void)cancelPerformSelector:(SEL)aSelector target:(id)target argument:(nullable id)arg;
- (void)cancelPerformSelectorsWithTarget:(id)target;
@end
```
&emsp;performSelector:target:argument:order:modes: 安排在接收方（NSRunLoop 实例对象）上发送消息。

&emsp;`aSelector`：一个选择器，用于标识要调用的方法。此方法应该没有明显的返回值，并且应该采用 id 类型的单个参数。

&emsp;`target`：在 aSelector 中定义选择器的对象。

&emsp;`anArgument`：调用时传递给方法的参数。如果该方法不接受参数，则传递 nil。

&emsp;`order`：消息的优先级。如果计划了多条消息，则顺序值较低的消息将在具有较高顺序值的消息之前发送。

&emsp;`modes`：可以为其发送消息的输入模式的数组。你可以指定自定义模式或使用 Run Loop Modes 中列出的模式之一。

&emsp;此方法设置一个 timer，以便在下一次 run loop 迭代开始时在 target 上执行 aSelector 消息。timer 被配置为在 modes 参数指定的模式下运行。当 timer 触发时，线程将尝试从 run loop 中取消消息队列并执行选择器。如果 run loop 正在运行并且处于指定的模式之一，则会成功；否则，timer 将等待直到 run loop 处于这些模式之一。

&emsp;发送 aSelector 消息之前，此方法返回。接收器会保留 target 和 anArgument 对象，直到选择器的 timer 触发，然后将其释放作为清理的一部分。

&emsp;如果要在处理当前事件后发送多个消息，并且要确保这些消息按特定顺序发送，请使用此方法。

### NSObject + NSThreadPerformAdditions
&emsp;当调用 performSelector:onThread: 时，实际上其会创建一个 timer 加到对应的线程去，同样的，如果对应线程没有 run loop 该方法也会失效。
```c++
@interface NSObject (NSThreadPerformAdditions)

// 主线程
- (void)performSelectorOnMainThread:(SEL)aSelector withObject:(nullable id)arg waitUntilDone:(BOOL)wait modes:(nullable NSArray<NSString *> *)array;
- (void)performSelectorOnMainThread:(SEL)aSelector withObject:(nullable id)arg waitUntilDone:(BOOL)wait; // equivalent to the first method with kCFRunLoopCommonModes

// 指定线程
- (void)performSelector:(SEL)aSelector onThread:(NSThread *)thr withObject:(nullable id)arg waitUntilDone:(BOOL)wait modes:(nullable NSArray<NSString *> *)array;
- (void)performSelector:(SEL)aSelector onThread:(NSThread *)thr withObject:(nullable id)arg waitUntilDone:(BOOL)wait; // equivalent to the first method with kCFRunLoopCommonModes

// 后台线程
- (void)performSelectorInBackground:(SEL)aSelector withObject:(nullable id)arg;

@end
```
&emsp;performSelectorOnMainThread:withObject:waitUntilDone:modes: 使用指定的模式在主线程上调用 receiver 的方法。

emsp;`aSelector`: 一个选择器，用于标识要调用的方法。该方法不应有明显的返回值，并且应采用 id 类型的单个参数或不带参数。

&emsp;`arg`: 调用时传递给 `aSelector` 的参数。如果该方法不接受参数，则传递 `nil`。

&emsp;`wait`: 一个布尔值，指定当前线程是否在主线程上的接收器上执行指定的选择器之后才阻塞。指定 YES 是阻止该线程；否则，请指定 NO 以使此方法立即返回。如果当前线程也是主线程，并且你传递 YES，则立即执行该消息，否则将执行队列排队，以使其下次通过 run loop 运行。

&emsp;`array`: 字符串数组，标识允许执行指定选择器的模式。该数组必须至少包含一个字符串。如果为该参数指定 nil 或空数组，则此方法将返回而不执行指定的选择器。

&emsp;你可以使用此方法将消息传递到应用程序的主线程。主线程包含应用程序的 main run loop，并且是 NSApplication 对象接收事件的地方。在这种情况下，消息是你要在线程上执行的当前对象的方法。

&emsp;此方法使用 array 参数中指定的 run loop 模式，将消息在主线程的 run loop 中排队。作为其正常 run loop 处理的一部分，主线程使消息出队（假定它正在以指定的模式之一运行）并调用所需的方法。假设每个选择器的关联 run loop 模式相同，那么从同一线程对该方法的多次调用会导致相应的选择器排队，并以与调用相同的顺序执行。如果为每个选择器指定不同的模式，则其关联模式与当前 run loop 模式不匹配的所有选择器都将被跳过，直到 runloop 随后在该模式下执行。

&emsp;你无法取消使用此方法排队的消息。如果要取消当前线程上的消息的选项，则必须使用 performSelector:withObject:afterDelay: 或 performSelector:withObject:afterDelay:inModes: 方法。

&emsp;该方法向其当前上下文的 run loop 进行注册，并依赖于定期运行的 run loop 才能正确执行。一个常见的上下文是调用 dispatch queue 时调用，可能会调用此方法并最终向不是定期自动运行的 run loop 注册。如果在一个 dispatch queue 上运行时需要这种功能，则应使用 dispatch_after 和相关方法来获取所需的行为。

### NSObject 协议中的 performSelector 方法
&emsp;在 NSObject 类中实现的 NSObject 协议的 performSelector 方法，则与 run loop 无关，它们相当于直接调用 recevier 的方法。
```c++
@protocol NSObject
...
- (id)performSelector:(SEL)aSelector;
- (id)performSelector:(SEL)aSelector withObject:(id)object;
- (id)performSelector:(SEL)aSelector withObject:(id)object1 withObject:(id)object2;
...
@end
```
&emsp;在 NSObject 类下的实现，可看到相当于直接调用函数。
```c++
- (id)performSelector:(SEL)sel {
    if (!sel) [self doesNotRecognizeSelector:sel];
    return ((id(*)(id, SEL))objc_msgSend)(self, sel);
}

- (id)performSelector:(SEL)sel withObject:(id)obj {
    if (!sel) [self doesNotRecognizeSelector:sel];
    return ((id(*)(id, SEL, id))objc_msgSend)(self, sel, obj);
}

- (id)performSelector:(SEL)sel withObject:(id)obj1 withObject:(id)obj2 {
    if (!sel) [self doesNotRecognizeSelector:sel];
    return ((id(*)(id, SEL, id, id))objc_msgSend)(self, sel, obj1, obj2);
}
```
&emsp;本篇主要分析了自动释放池和计时器与 run loop 相关的一些内容，计时器相关的内容在我们的日常开发中还挺常用的，需要认真学习总结，下篇我们继续学习 iOS 系统中与 run loop 相关的内容。

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
