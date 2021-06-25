# iOS 从源码解析Run Loop (十)：Run Loop 与GCD、FPS、CADisplayLink

> &emsp;本篇我们继续学习日常开发中可能被我们忽略但是内部实现其实涉及到 Run Loop 做支撑的一些知识点。

## GCD
&emsp;在 Run Loop 和 GCD 的底层双方各自都会相互用到对方。首先我们先看一下读 run loop 源码的过程中用到 GCD 的地方，前面我们学习 GCD 的时候已知使用 `dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue)` 可以构建计时器，且它比 NSTimer 的精度更高。

&emsp;在 run loop 中有两个地方用到了 dispatch_source，一是在 run loop mode 中有一个与 \_timerPort（mk_timer_create）对应的 \_timerSource，它们两个的作用是相同的，都是用来当到达 run loop mode 的 \_timers 数组中最近的某个计时器的触发时间时用来唤醒当前 run loop 的，然后还有一个地方是在 \__CFRunLoopRun 函数中直接使用 dispatch_source 构建一个计时器用来为入参 run loop 的运行时间计时的，当入参 run loop 运行超时时此计时器便会触发。

&emsp;在 run loop mode 中使用 dispatch_source 还是 MK_TIMER 来构建一个计时器是有一个平台限制的，源码内部使用了两个宏做区分 USE_DISPATCH_SOURCE_FOR_TIMERS 和 USE_DISPATCH_SOURCE_FOR_TIMERS。
```c++
#if DEPLOYMENT_TARGET_MACOSX

// 在 macOS 下则同时支持使用 dispatch_source 和 MK_TIMER 来构建定时器
#define USE_DISPATCH_SOURCE_FOR_TIMERS 1
#define USE_MK_TIMER_TOO 1

#else

// 其他平台则只支持 MK_TIMER 
#define USE_DISPATCH_SOURCE_FOR_TIMERS 0
#define USE_MK_TIMER_TOO 1

#endif
```
&emsp;这里我们可以全局搜索 USE_DISPATCH_SOURCE_FOR_TIMERS 然后看到它和 USE_MK_TIMER_TOO 几乎都是前后🦶使用的，且都是为了同一个目标为 CFRunLoopTimerRef 而唤醒 run loop。

&emsp;注意这里并不是说 macOS 之外的平台就不支持使用 dispatch_source 了，大家（iOS、macOS）都支持，这里只是针对的是 run loop mode 中用来为 \_timers 数组中的某个 CFRunLoopTimerRef 到达触发时间时唤醒当前 run loop 的方式不同而已。然后在 \__CFRunLoopRun 函数中我们看到所有平台下都是使用 dispatch_source 来构建计时器为 run loop 的运行时间而计时的。

&emsp;（一个题外话：看到这里我们似乎可以得到一些理解和启发，CFRunLoopTimerRef 虽一直被我们称为计时器，但其实它的触发执行是完全依赖 run loop mode 中的 \_timerPort 或者 \_timerSource 来唤醒当前 run loop，然后在当前 run loop 的本次循环中判断本次 run loop 被唤醒的来源，如果是因为 timer ，则执行某个 CFRunLoopTimerRef 的回调事件并更新最近的下次执行时间，所以这里 CFRunLoopTimerRef 虽被称为计时器其实它的计时部分是依靠别人来做的，它本身并不具备计时功能，只是有一个值记录自己的下次触发时间而已。）

&emsp;下面我们看一下 GCD 中使用到 Run Loop 的地方。

&emsp;当调用 dispatch_async(dispatch_get_main_queue(), block) 时，libDispatch 会向主线程的 run loop 发送消息，run loop 会被唤醒，并从消息中取得这个 block，并在回调 \_\_CFRUNLOOP_IS_SERVICING_THE_MAIN_DISPATCH_QUEUE\_\_ 里执行这个 block。但这个逻辑仅限于 dispatch 到主线程，dispatch 到其他线程仍然是由 libDispatch 处理的。为什么子线程没有这个和 GCD 交互的逻辑？原因有二：
1. 主线程 run loop 是主线程的事件管理者。run loop 负责何时让 run loop 处理何种事件。所有分发给主线程的任务必须统一交给主线程 run loop 排队处理。举例：UI 操作只能在主线程，不在主线程操作 UI 会带来很多 UI 错乱问题以及 UI 更新延迟问题。
2. 子线程不接受 GCD 的交互。因为子线程不一定开启了 run loop。

&emsp;上面一段结论我们在梳理 \__CFRunLoopRun 函数流程时已经看的一清二楚了。如函数开始时判断当前是否是主线程来获取主队列的 port 并赋值给 dispatchPort，然后在 run loop 本次循环中判断唤醒来源是 dispatchPort 时，执行添加到主队列中的任务（_dispatch_main_queue_drain）。
```c++
...
        else if (livePort == dispatchPort) {
            CFRUNLOOP_WAKEUP_FOR_DISPATCH();
            __CFRunLoopModeUnlock(rlm);
            __CFRunLoopUnlock(rl);
            
            // TSD 给 __CFTSDKeyIsInGCDMainQ 置为 6 和 下面的置 0 对应，可以理解为一个加锁行为!
            _CFSetTSD(__CFTSDKeyIsInGCDMainQ, (void *)6, NULL);
            
#if DEPLOYMENT_TARGET_WINDOWS
            void *msg = 0;
#endif

            // 内部是调用 static void _dispatch_main_queue_drain(dispatch_queue_main_t dq) 函数，即处理主队列中的任务
            __CFRUNLOOP_IS_SERVICING_THE_MAIN_DISPATCH_QUEUE__(msg);
            
            _CFSetTSD(__CFTSDKeyIsInGCDMainQ, (void *)0, NULL);
            
            __CFRunLoopLock(rl);
            __CFRunLoopModeLock(rlm);
            
            sourceHandledThisLoop = true;
            didDispatchPortLastTime = true;
        }
...
```
&emsp;到这里 GCD 和 Run Loop 的相互使用就看完了，下面我们看一下屏幕 FPS 相关的内容。
## FPS
&emsp;FPS（Frames Per Second）是图像领域中的定义，是指画面每秒传输帧数，通俗来讲就是指动画或视频的画面数。FPS 是测量用于保存、显示动态视频的信息数量，每秒钟帧数越多，所显示的动作就会越流畅，iPhone 屏幕最大帧率是每秒 60 帧，一般我们的 APP 的 FPS 恒定的保持在 50-60 之间，用户滑动体验都是比较流畅的。关于屏幕卡顿的一些原因可以参考：[iOS 保持界面流畅的技巧](https://blog.ibireme.com/2015/11/12/smooth_user_interfaces_for_ios/)

&emsp;YYKit 下的 YYFPSLabel 提供了一种监测 FPS 的方案，实现原理是把一个 CADisplayLink 对象添加到主线程的 run loop 的 NSRunLoopCommonModes 模式下，然后在 CADisplayLink 对象的回调函数中统计每秒钟屏幕的刷新次数。不熟悉 CADisplayLink 的小伙伴可以先看一下 下面 CADisplayLink 的文档，然后我们再一起看一下  YYFPSLabel 的源码。

### CADisplayLink
&emsp;CADisplayLink 是一个和屏幕刷新率一致的定时器（但实际实现原理更复杂，和 NSTimer 并不一样，其内部实际是操作了一个 Source）。如果在两次屏幕刷新之间执行了一个长任务，那其中就会有一帧被跳过去（和 NSTimer 相似），造成界面卡顿的感觉。在快速滑动 TableView 时，即使一帧的卡顿也会让用户有所察觉。Facebook 开源的 AsyncDisplayLink 就是为了解决界面卡顿的问题，其内部也用到了 Run Loop。下面我们首先看一下 CADisplayLink 的文档。

> &emsp;A timer object that allows your application to synchronize its drawing to the refresh rate of the display.

&emsp;CADisplayLink 表示一个绑定到显示 vsync 的计时器的类。（其中 CA 表示的是 Core Animation（核心动画） 首字母缩写，CoreAnimation.h 是 QuartzCore 框架中的一个包含 QuartzCore 框架所有头文件的文件）
```c++
/** Class representing a timer bound to the display vsync. **/

API_AVAILABLE(ios(3.1), watchos(2.0), tvos(9.0)) API_UNAVAILABLE(macos)
@interface CADisplayLink : NSObject {
@private
  void *_impl;
}
```
&emsp;在应用程序中初始化一个新的 display link 对象时使用 displayLinkWithTarget:selector: 函数，此函数提供了一个 target 对象和一个在屏幕更新时要调用的 sel。为了使 run loop 与 display link 同步，需要使用 addToRunLoop:forMode: 函数将 display link 对象添加到指定 run loop 的指定 mode 下。

&emsp;一旦 display link 与 run loop 相关联，当需要更新屏幕内容时，就会调用 target 上的 sel。target 可以读取 display link 的 timestamp 属性，以检索上一帧显示时的时间戳。例如，播放电影的应用程序可能使用 timestamp 来计算下一个要显示的视频帧。执行自己的动画的应用程序可能会使用 timestamp 来确定在下一帧中显示对象的位置和方式。

&emsp;duration 属性以 maximumFramesPerSecond（屏幕每秒可显示的最大帧数：60）提供帧之间的时间间隔。要计算实际的两帧之间的时长（frame duration），请使用 targetTimestamp - timestamp（即下一帧的时间戳减去上一帧的时间戳）。你可以在应用程序中使用实际的 frame duration 来计算显示器的帧率、下一帧的大概显示时间、并调整绘图行为（drawing behavior），以便及时准备下一帧以供显示。

&emsp;应用程序可以通过将 paused 属性设置为 YES 来禁用通知（停止回调 target 的 sel 函数）。另外，如果你的应用程序无法在提供的时间内提供帧，你可能需要选择较慢的帧速率。对于用户来说，帧率较慢但能保持一致的应用程序会比跳帧的应用程序感觉更平滑。通过设置 preferredFramesPerSecond 属性，可以定义每秒帧数。

&emsp;当你的应用程序完成 display link 时，应调用 invalidate 函数将其从所有 run loop 中移除，并将其与 target 解除关联。

&emsp;CADisplayLink 不应被子类化。

#### displayLinkWithTarget:selector:
&emsp;返回一个新建的 display link 对象。
```c++
// 为 main display 创建一个新的 display link 对象。
// 它将在 'target' 上调用名为 'sel' 的方法，该方法具有 '(void)selector:(CADisplayLink *)sender' 的签名。
+ (CADisplayLink *)displayLinkWithTarget:(id)target selector:(SEL)sel;
```
&emsp;`target`：当屏幕应该更新时要通知的对象。`sel`：在 `target` 上调用的方法。

&emsp;要在 `target` 上调用的 selector 必须是具有以下签名的方法：
```c++
- (void) selector:(CADisplayLink *)sender;
```
&emsp;其中 sender 是 displayLinkWithTarget:selector: 返回的 display link 对象。

&emsp;新建的 display link 对象 retain 了 `target`。

#### addToRunLoop:forMode:
&emsp;注册 display link 对象到 run loop 中。
```c++
// 将 display link 对象添加到给定的 run loop 的给定 mode 下。除非暂停，否则它将触发每个 vsync，直到将其移除。
// 每个 display link 对象只能添加到一个 run loop 中，但可以一次以多种 mode 添加。
// 当添加到 run loop 时，display link 对象将被 run loop 隐式 retain。
- (void)addToRunLoop:(NSRunLoop *)runloop forMode:(NSRunLoopMode)mode;
```
&emsp;`runloop`：与 display link 对象关联的 run loop。`mode`：将 display link 添加到 run loop 的 mode。你可以指定自定义 mode 或使用 NSRunLoop 中列出的 modes 之一。

&emsp;你可以将 display link 与多个 input modes 关联。当 run loop 以你指定的 mode 执行时，当需要新帧时，display link 会通知 target。(即执行 display link 对象创建时的 target 的 sel 函数。)

&emsp;run loop 会 retain display link 对象。当 display link 对象不再使用时要从所有 run loop 中移除 display link，移除方式是调用 display link 的 invalidate 函数。（这里和 NSTimer 极其类似，例如创建他们的对象时他们都会持有传入的 target，当把它们加入到 run loop 时它们也都会被 run loop 所持有，当不再需要它们时都是调用它们的 invalidate 函数大概是把它们从 run loop 的 \_commonModeItems 集合中、run loop mode 的 \_timers 或 \_sources0 集合中移除。）
#### removeFromRunLoop:forMode:
&emsp;从给定 mode 的 run loop 中移除 display link 对象。
```c++
// 从 run loop 的给定 mode 中移除 display link 对象。从最后一个已注册的 mode 中移除时，display link 对象将被隐式释放。
- (void)removeFromRunLoop:(NSRunLoop *)runloop forMode:(NSRunLoopMode)mode;
```
&emsp;`runloop`：与 display link 关联的 run loop。`mode`：display link 正在运行的 run loop mode。

&emsp;如果 display link 不再与任何 run loop mode 相关联，则 run loop 将释放该 display link。

#### invalidate
&emsp;从所有的 run loop modes 中移除 display link。
```c++
// 从所有 run loop modes 中移除 display link 对象（如果已隐式保留 display link 对象，则释放 display link 对象），
// 然后释放 display link 对象构建时传入的 target。
- (void)invalidate;
```
&emsp;从所有 run loop modes 中移除 display link 会导致其被 run loop 释放。display link 还会释放 target。

&emsp;invalidate 是线程安全的，这意味着可以从与运行 display link 的线程 “分开” 的线程中调用 invalidate 函数。

#### duration
&emsp;（只读）屏幕刷新更新两帧之间的时间间隔。（duration 属性用于提供屏幕最大刷新频率（maximumFramesPerSecond 60）下每一帧的时间间隔，这个属性可以用于在应用中获取帧率。）
```c++
@property(readonly, nonatomic) CFTimeInterval duration;
```
&emsp;在至少一次调用 target 的 sel 之前，duration 的值是不确定的。（应用程序可以通过将 duration 乘以 frameInterval 来计算渲染每个帧所需的时间量。）

&emsp;duration 提供了每帧之间的时间，也就是屏幕每次刷新之间的时间。 duration 只是个大概的时间，如果 CPU 忙于其它计算，就没法保证以相同的频率执行屏幕的绘制操作，这样会跳过几次调用回调方法的机会。

##### frameInterval
&emsp;（已废弃）在 display link 再次通知 target 之前必须经过的帧数。
```c++
@property(nonatomic) NSInteger frameInterval;
```
&emsp;默认值为 1，这将导致以显示的刷新率通知应用程序。如果该值设置为大于 1 的值，则 display link 将以本机刷新率的一小部分通知应用程序。例如，将间隔设置为 2 会导致 display link 每隔一帧触发一次，从而提供一半的帧速率。

&emsp;将此值设置为小于1会导致未定义的行为。

#### timestamp
&emsp;与显示的最后一帧关联的时间戳。（这个属性用来返回上一次屏幕刷新的时间戳。例如视频播放的应用，可以通过时间戳来获取上一帧的具体时间，来计算下一帧。）
```c++
// 与最近的 target 调用关联的显示帧的当前时间和持续时间。时间是使用正常的 Core Animation 约定表示的，即将 mach host time 转换为秒。
@property(readonly, nonatomic) CFTimeInterval timestamp;
```
&emsp;target 应使用此属性的值来计算应在下一帧中显示的内容。

#### preferredFramesPerSecond
&emsp;display link 回调的首选帧速率。
```c++
// 定义此 display link 想要的回调速率（以每秒帧数为单位）。如果设置为零（则实际使用默认值），则使用本机的默认帧率（60）。
// display link 将尽最大努力尝试以设置的 preferredFramesPerSecond 的值进行回调。
@property(nonatomic) NSInteger preferredFramesPerSecond API_AVAILABLE(ios(10.0), watchos(3.0), tvos(10.0));
```
&emsp;当你为 display link 指定首选帧速率（preferred frame rate）时，它会根据硬件的功能以及你的游戏或应用可能正在执行的其他任务，以尽可能接近的速率通知目标（即以尽可能接近 preferredFramesPerSecond 的值来回调 target 的 sel 函数）。选择的实际帧率通常是屏幕最大刷新率的一个因素，以提供一致的帧率。例如，如果屏幕的最大刷新速率是每秒 60 帧，则这也是 display link 设置为实际帧速率的最高帧速率。但是，如果你要求较低的帧率，则 display link 可能会选择每秒 30、20 或 15 帧或其他速率作为实际帧率。尽量选择你的应用程序可以一直保持的帧率。

&emsp;默认值为 0。此值为 0 时，preferred frame rate 会使用显示器的最大刷新率（每秒 60 帧），如 maximumFramesPerSecond 属性所示。有关详细信息，请参阅[Setting Refresh Rates on ProMotion and Traditional Displays](https://developer.apple.com/library/archive/technotes/tn2460/_index.html#//apple_ref/doc/uid/DTS40017657)。

&emsp;如果在特定帧率内无法提供对象的操作，可以通过降低帧率解决。一个拥有持续稳定但是较慢帧率的应用要比跳帧的应用顺滑的多。可以通过 preferredFramesPerSecond 来设置每秒刷新次数。preferredFramesPerSecond 默认值为屏幕最大帧率（maximumFramesPerSecond）目前是60。

&emsp;实际的屏幕帧率会和 preferredFramesPerSecond 有一定的出入，结果是由设置的值和屏幕最大帧率（maximumFramesPerSecond）相互影响产生的。规则大概如下：如果屏幕最大帧率（preferredFramesPerSecond）是 60,实际帧率只能是15、20、30、60 中的一种。如果设置大于 60 的值，屏幕实际帧率为 60。如果设置的是 26~35 之间的值，实际帧率是 30。如果设置为 0，会使用最高帧率。

##### maximumFramesPerSecond
&emsp;maximumFramesPerSecond 是 UIScreen 的一个只读属性，表示屏幕每秒可以显示的最大帧数。
```c++
@property(readonly) NSInteger maximumFramesPerSecond;
```
&emsp;在 iOS 上，每秒最大帧数通常为 60。对于 tvOS 设备，此值可能会根据所连接屏幕的硬件功能或用户在 Apple TV 上选择的分辨率而有所不同。

#### paused
&emsp;paused 是一个布尔值，用于说明 display link 到 target 的通知（回调 sel）是否已暂停。
```c++
/* When true the object is prevented from firing. Initial state is false. */
@property(getter=isPaused, nonatomic) BOOL paused;
```
&emsp;默认值为 NO。如果值为 YES，则 display link 不会向 target 发送通知（回调 sel）。

&emsp;paused 是线程安全的，这意味着可以从与运行 display link 的线程分开的线程中进行设置。

#### targetTimestamp
&emsp;（只读的）iOS 10.0 后新增的属性，与显示的下一帧关联的时间戳。
```c++
@property(readonly, nonatomic) CFTimeInterval targetTimestamp
API_AVAILABLE(ios(10.0), watchos(3.0), tvos(10.0));
```
&emsp;你可以使用  target timestamp 来取消或暂停长时间运行的进程，这些进程可能会超出帧之间的可用时间，以便保持一致的帧率。（targetTimestamp 和 timestamp 好像是对应的，一个表示下一帧的时间戳，一个表示上一帧的时间戳。）

&emsp;以上是 CADisplayLink 文档的全部内容，我们创建一个 CADisplayLink 对象并添加到当前线程的主线程中。
```c++
- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    CADisplayLink *displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(displayLinkAction:)];
    [displayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSRunLoopCommonModes];
}

- (void)displayLinkAction:(CADisplayLink *)displayLink {
    NSLog(@"📧📧 %@", displayLink);
    NSLog(@"duration: %lf timestamp: %lf targetTimestamp: %lf frameInterval: %d preferredFramesPerSecond: %d maximumFramesPerSecond: %d", displayLink.duration, displayLink.timestamp, displayLink.targetTimestamp, displayLink.frameInterval, displayLink.preferredFramesPerSecond, UIScreen.mainScreen.maximumFramesPerSecond);
}
// 控制台打印：
📧📧 <CADisplayLink: 0x6000008ec2c0>
duration: 0.016667 timestamp: 366093.060335 targetTimestamp: 366093.077002 frameInterval: 1 preferredFramesPerSecond: 0 maximumFramesPerSecond: 60
```
&emsp;直接打印 CADisplayLink 对象的各个属性，可看到 duration 是我们熟悉的 0.016667 秒（16.7 毫秒），targetTimestamp - timestamp 约等于 16.7 毫秒，preferredFramesPerSecond 的值是 0，实际是屏幕的最大刷新率每秒 60 帧，iPhone 下 maximumFramesPerSecond 是 60。 

&emsp;在上面的 displayLinkAction 函数内打一个断点，进入断点后打印当前函数调用堆栈：
```c++
(lldb) bt
* thread #1, queue = 'com.apple.main-thread', stop reason = breakpoint 1.1
  * frame #0: 0x00000001007b3b1e Simple_iOS`-[ViewController displayLinkAction:](self=0x00007fc4ab601df0, _cmd="displayLinkAction:", displayLink=0x00006000013cc090) at ViewController.m:382:27
    frame #1: 0x00007fff2afeb266 QuartzCore`CA::Display::DisplayLink::dispatch_items(unsigned long long, unsigned long long, unsigned long long) + 640
    frame #2: 0x00007fff2b0c3e03 QuartzCore`display_timer_callback(__CFMachPort*, void*, long, void*) + 299
    frame #3: 0x00007fff23b9503d CoreFoundation`__CFMachPortPerform + 157
    frame #4: 0x00007fff23bd4bc9 CoreFoundation`__CFRUNLOOP_IS_CALLING_OUT_TO_A_SOURCE1_PERFORM_FUNCTION__ + 41 // ⬅️ 看到是执行 source1 的回调函数
    frame #5: 0x00007fff23bd4228 CoreFoundation`__CFRunLoopDoSource1 + 472
    frame #6: 0x00007fff23bced64 CoreFoundation`__CFRunLoopRun + 2516
    frame #7: 0x00007fff23bce066 CoreFoundation`CFRunLoopRunSpecific + 438
    frame #8: 0x00007fff384c0bb0 GraphicsServices`GSEventRunModal + 65
    frame #9: 0x00007fff48092d4d UIKitCore`UIApplicationMain + 1621
    frame #10: 0x00000001007b486d Simple_iOS`main(argc=1, argv=0x00007ffeef44bd60) at main.m:76:12
    frame #11: 0x00007fff5227ec25 libdyld.dylib`start + 1
(lldb)
```
&emsp;看到 CADisplayLink 的回调函数是通过 source1 的回调来执行的。然后打印当前线程的 run loop 可看到创建了一个回调函数是 \_ZL22display_timer_callbackP12__CFMachPortPvlS1\_ 的 source1。
```c++
...
0 : <CFRunLoopSource 0x600003b11140 [0x7fff80617cb0]>{signalled = No, valid = Yes, order = -1, context = <CFMachPort 0x6000039146e0 [0x7fff80617cb0]>{valid = Yes, port = 6507, source = 0x600003b11140, callout = _ZL22display_timer_callbackP12__CFMachPortPvlS1_ (0x7fff2b0c3cd8), context = <CFMachPort context 0x6000035200d0>}}
...
```
&emsp;通过以上可知 CADisplayLink 的内部是 source1 来驱动的。

&emsp;以上部分便是 CADisplayLink 部分的学习，下面我们来看 YYFPSLabel 的具体实现。

### YYFPSLabel 帧率监测
&emsp; [YYFPSLabel](https://github.com/ibireme/YYText/blob/master/Demo/YYTextDemo/YYFPSLabel.m)
```c++
#import "YYFPSLabel.h"
//#import <YYKit/YYKit.h>
#import "YYText.h"
#import "YYWeakProxy.h"

#define kSize CGSizeMake(55, 20)

@implementation YYFPSLabel {
    CADisplayLink *_link;
    NSUInteger _count;
    NSTimeInterval _lastTime;
    UIFont *_font;
    UIFont *_subFont;
    
    NSTimeInterval _llll;
}

- (instancetype)initWithFrame:(CGRect)frame {
    if (frame.size.width == 0 && frame.size.height == 0) {
        frame.size = kSize;
    }
    self = [super initWithFrame:frame];
    
    self.layer.cornerRadius = 5;
    self.clipsToBounds = YES;
    self.textAlignment = NSTextAlignmentCenter;
    self.userInteractionEnabled = NO;
    self.backgroundColor = [UIColor colorWithWhite:0.000 alpha:0.700];
    
    _font = [UIFont fontWithName:@"Menlo" size:14];
    if (_font) {
        _subFont = [UIFont fontWithName:@"Menlo" size:4];
    } else {
        _font = [UIFont fontWithName:@"Courier" size:14];
        _subFont = [UIFont fontWithName:@"Courier" size:4];
    }
    
    // 创建一个 CADisplayLink 对象添加到 main run loop 的 NSRunLoopCommonModes 模式下。
    // 因为 CADisplayLink 对象会 retain target，所以这里用了一个 [YYWeakProxy proxyWithTarget:self] 做中间的桥梁，
    // self 赋值给 YYWeakProxy 对象的 weak 属性 _target，即 self 被 YYWeakProxy 对象弱引用，
    // 并重写 YYWeakProxy 的 forwardingTargetForSelector: 函数，直接返回 _target 对象来接收处理发送给 YYWeakProxy 的消息，
    // 即把 CADisplayLink 的回调函数 tick: 转移到 YYFPSLabel 类来处理。
    
    //（self 持有 _link、_link 持有 YYWeakProxy、YYWeakProxy 弱引用 self，这样就破开了原有的引用循环） 
    
    _link = [CADisplayLink displayLinkWithTarget:[YYWeakProxy proxyWithTarget:self] selector:@selector(tick:)];
    [_link addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSRunLoopCommonModes];
    
    return self;
}

- (void)dealloc {
    // 销毁时一定要调用 CADisplayLink 的 invalidate 函数
    [_link invalidate];
}

- (CGSize)sizeThatFits:(CGSize)size {
    return kSize;
}

- (void)tick:(CADisplayLink *)link {
    if (_lastTime == 0) {
    
        // 初次调用 tick 函数时，_lastTime 记录第一帧的时间戳
        _lastTime = link.timestamp;
        
        return;
    }
    
    // 统计 tick 被调用的次数
    _count++;
    
    // link.timestamp 是当前帧的时间戳，减去上一次统计帧率的时间戳，当时间间隔大于等于 1 秒时才进行帧率统计，
    // 即 1 秒钟统计一次帧率（也没必要过于频繁的统计帧率）
    NSTimeInterval delta = link.timestamp - _lastTime;
    
    // 时间大于等于 1 秒钟计算一次帧率，刷新一次 YYFPSLabel 显示的帧率值
    if (delta < 1) return;
    
    // 更新 _lastTime 为当前帧的时间戳
    _lastTime = link.timestamp;
    
    // tick 被调用的次数除以时间间隔，即为当前的帧率
    float fps = _count / delta;
    
    // tick 被调用的次数清 0（开始下一轮帧率统计）
    _count = 0;
    
    CGFloat progress = fps / 60.0;
    UIColor *color = [UIColor colorWithHue:0.27 * (progress - 0.2) saturation:1 brightness:0.9 alpha:1];
    
    NSMutableAttributedString *text = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithFormat:@"%d FPS",(int)round(fps)]];
    [text yy_setColor:color range:NSMakeRange(0, text.length - 3)];
    [text yy_setColor:[UIColor whiteColor] range:NSMakeRange(text.length - 3, 3)];
    text.yy_font = _font;
    [text yy_setFont:_subFont range:NSMakeRange(text.length - 4, 1)];
    
    self.attributedText = text;
}

@end
```
&emsp;tick: 函数内部借助 CADisplayLink 对象中记录的每一帧的时间戳来统计出每秒钟的帧率，足够我们日常开发中监测滑动帧率。

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
+ [iOS 保持界面流畅的技巧](https://blog.ibireme.com/2015/11/12/smooth_user_interfaces_for_ios/)
