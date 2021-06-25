# iOS 响应者链（Responder Chain）与触摸事件处理全过程分析

> &emsp;本文会对从手指触摸屏幕开始一直到这个触摸事件得到处理的完整过程进行分析。（侧重点放在当前应用程序处理触摸事件部分）

> &emsp;在开始之前觉得还是有必要先对一些继承关系有一个理解：NSObject 是我们日常接触最多的基类，然后大概在 iOS 中可以理解为表示数据层的 Foundation 框架下的以 NS 开头的类如 NSDictionary、NSArray 等几乎都是直接继承自 NSObject 或者是 NSObject 的间接子类，可以说是依靠 NSObject 基类完成了它们所有的内存管理等内容。那么 UI 层呢？如 UIView 它们则都是 UIResponder 的子类，UIResponder 则是 NSObject 的子类，UIButton 则是继承自 UIControl，而 UIControl 则是继承自 UIView，UIView 等一众子类正是因为继承自 UIResponder 所以才可以被作为响应者使用，而之所以能被称为响应者，就是因为它们可以重写 UIResponder 的 touches...（响应触摸事件）、presses...（响应按键事件）、motion...（响应运动事件） 系列函数而已。UIControl 系列则是依据 Target-Action 机制来响应用户事件。好了，继承关系就是这些吧，想必在我们日常开发中没事就按住 command 往下点的过程中就已经对它们的继承关系烂熟于心了，那么下面我们就对这些内容进行详细的展开吧！ 

## IOKit.framework/SpringBoard
&emsp;IOKit.framework 是与硬件或内核服务通信的低级框架。尽管这是一个公共框架，但苹果不鼓励开发人员使用它，并且任何使用它的应用都将被App Store拒绝。[IOKit.framework](http://iphonedevwiki.net/index.php/IOKit.framework)

&emsp;SpringBoard.app 是 iPhone 的应用程序启动器。它提供所有应用程序启动服务、图标管理、状态栏控件等等内容。有关管理 Springboard.app 的类，可参见 SpringBoard。 [SpringBoard.app](https://iphonedevwiki.net/index.php/SpringBoard.app)

> &emsp;SpringBoard.app 是 iOS 和 iPadOS 负责管理主屏幕的基础程序，并在设备启动时启动 WindowServer、开启应用程序（实现该功能的程序称为应用启动器）和对设备进行某些设置。有时候主屏幕也被作为 SpringBoard 的代称。主要处理按键（锁屏/静音等）、触摸、加速、距离传感器（UIEventTypeMotion）等几种事件，随后通过 mac port 进程间通信转发至需要的 APP。
> 
> &emsp;Mac OSX 中使用的是 Launchpad，能让用户以从类似于 iOS 的 SpringBoard 的界面点击一下图示来启动应用程序。在 Launchpad 推出之前，用户能以 Dock、Finder、Spotlight 或终端来启动应用程序。不过 Launchpad 并不会占据整个主屏幕，而更像是一个 Space（类似于仪表板）。[细数iOS触摸事件流动](https://juejin.cn/post/6844904175415853064)

> &emsp;当一个硬件事件（触摸/锁屏/摇晃等）发生后，首先由 IOKit.framework 生成一个 IOHIDEvent 事件并由 SpringBoard 接收。这个过程的详细情况可以参考 IOHIDFamily。SpringBoard 只接收按键（锁屏/静音等）、触摸、加速、距离传感器（UIEventTypeMotion）等几种 Event，SpringBoard 判断桌面是否存在前台应用，若无（如处于桌面翻页），则触发 SpringBoard 应用内部主线程 run loop 的 source0 事件回调，由桌面应用内部消耗；若有则通过 mach port 转发给需要的前台 App 进程。

&emsp;下面我们接着看由前台 App 的哪个 mach port 来接收 SpringBoard 的消息并唤醒前台 App 的主线程。 
## com.apple.uikit.eventfetch-thread
&emsp;App 启动后会创建一条名为 com.apple.uikit.eventfetch-thread 的线程，并直接启动此线程的 run loop，且在其 kCFRunLoopDefaultMode 运行模式下添加了一个回调函数是 \_\_IOHIDEventSystemClientQueueCallback 的 source1，用于接收上面提到的 SpringBoard 通过 mach port 发来的消息。

&emsp;前台 App 进程的 com.apple.uikit.eventfetch-thread 线程被 SpringBoard 根据指定的 mach port 唤醒后，执行其 source1 对应的回调函数 \_\_IOHIDEventSystemClientQueueCallback，并将 main run loop 中的回调函数是 \_\_handleEventQueue 的 source0 的 signalled 设置为 YES 标记其为待处理状态，同时唤醒 main run loop，主线程则调用 \_\_handleEventQueue 来进行事件（IOHIDEvent）的处理。

&emsp;[ibireme](https://blog.ibireme.com/2015/11/12/smooth_user_interfaces_for_ios/) 大佬说在 \_UIApplicationHandleEventQueue 内会把 IOHIDEvent 处理并包装成 UIEvent 进行处理或分发，可是添加符号断点并不能找到此方法，猜测现在应该是在 \_\_handleEventQueue 函数内处理 IOHIDEvent。接着就是 进行 [UIApplication sendEvent:] 把 UIEvent 分发到我们的 App 了。

&emsp;我们使用 Xcode 创建一个 Single View App 后并直接运行，然后点击 Xcode 底部的暂停程序执行的按钮，当前程序的 com.apple.main-thread 线程定位到 mach_msg_trap，然后在其下方有一条名为 com.apple.uikit.eventfetch-thread 的线程，我们直接选中它，然后在 Xcode 控制台输入: po [NSRunLoop currentRunLoop] 打印当前线程的 run loop，可看到它仅有一个被标记为 common 的 kCFRunLoopDefaultMode 模式，然后 Default Mode 中的内容也很简单，仅有一个 source0 和三个 source1，且它们的回调事件正与 IOHIDEvent 相关。这里我们只列出回调函数是 \_\_IOHIDEventSystemClientQueueCallback 的 source1。
```c++
...
sources1 = <CFBasicHash 0x600001fff330 [0x7fff80617cb0]>{type = mutable set, count = 3,
entries =>
...
1 : <CFRunLoopSource 0x6000024e4540 [0x7fff80617cb0]>{signalled = No, valid = Yes, order = 0, context = <CFMachPort 0x6000026ec210 [0x7fff80617cb0]>{valid = Yes, port = 440b, source = 0x6000024e4540, callout = __IOHIDEventSystemClientQueueCallback (0x7fff25e91d1d), context = <CFMachPort context 0x7fb555601c50>}}
...
}
...
```
&emsp;然后我们使用 po [NSRunLoop mainRunLoop] 打印 main run loop，可看到在其 UITrackingRunLoopMode 和 kCFRunLoopDefaultMode 模式下有同一个回调函数是 \_\_handleEventQueue 的 source0，且 Tracking Mode 和 Default Mode 都被标记为 common。
```c++
...
2 : <CFRunLoopMode 0x6000028101a0 [0x7fff80617cb0]>{name = UITrackingRunLoopMode, port set = 0x2a03, queue = 0x600003d1ca00, source = 0x600003d1cb00 (not fired), timer port = 0x2c03, 
sources0 = <CFBasicHash 0x600001d319b0 [0x7fff80617cb0]>{type = mutable set, count = 4,
entries =>
...
4 : <CFRunLoopSource 0x600002618240 [0x7fff80617cb0]>{signalled = No, valid = Yes, order = -1, context = <CFRunLoopSource context>{version = 0, info = 0x6000026103c0, callout = __handleEventQueue (0x7fff48126d97)}}
},
...
...
4 : <CFRunLoopMode 0x600002814410 [0x7fff80617cb0]>{name = kCFRunLoopDefaultMode, port set = 0x2503, queue = 0x600003d10e80, source = 0x600003d10f80 (not fired), timer port = 0x1e03, 
sources0 = <CFBasicHash 0x600001d31a10 [0x7fff80617cb0]>{type = mutable set, count = 4,
entries =>
...
4 : <CFRunLoopSource 0x600002618240 [0x7fff80617cb0]>{signalled = No, valid = Yes, order = -1, context = <CFRunLoopSource context>{version = 0, info = 0x6000026103c0, callout = __handleEventQueue (0x7fff48126d97)}}
}
...
```
&emsp;我们在上面的 Single View App 中创建一个名字是 CustomView 的自定义 UIView，并重写该 view 的 touchesBegan:withEvent: 方法，然后在其中打一个断点。然后在当前 App 的 root view 上添加一个 CustomView 对象，运行程序并点击该 CustomView。然后在控制台输入 bt 并回车，可看到如下函数调用堆栈：
```c++
(lldb) bt
* thread #1, queue = 'com.apple.main-thread', stop reason = breakpoint 5.1
  * frame #0: 0x0000000101522cbd EmptySimpleApp`-[CustomView touchesBegan:withEvent:](self=0x00007ff88fc0b5e0, _cmd="touchesBegan:withEvent:", touches=1 element, event=0x000060000190c500) at CustomView.m:22:30
    frame #1: 0x00007fff480ce8de UIKitCore`-[UIWindow _sendTouchesForEvent:] + 1867
    frame #2: 0x00007fff480d04c6 UIKitCore`-[UIWindow sendEvent:] + 4596
    frame #3: 0x00007fff480ab53b UIKitCore`-[UIApplication sendEvent:] + 356
    frame #4: 0x0000000103724bd4 UIKit`-[UIApplicationAccessibility sendEvent:] + 85
    frame #5: 0x00007fff4812c71a UIKitCore`__dispatchPreprocessedEventFromEventQueue + 6847
    frame #6: 0x00007fff4812f1e0 UIKitCore`__handleEventQueueInternal + 5980
    frame #7: 0x00007fff23bd4471 CoreFoundation`__CFRUNLOOP_IS_CALLING_OUT_TO_A_SOURCE0_PERFORM_FUNCTION__ + 17
    frame #8: 0x00007fff23bd439c CoreFoundation`__CFRunLoopDoSource0 + 76
    frame #9: 0x00007fff23bd3b74 CoreFoundation`__CFRunLoopDoSources0 + 180
    frame #10: 0x00007fff23bce87f CoreFoundation`__CFRunLoopRun + 1263
    frame #11: 0x00007fff23bce066 CoreFoundation`CFRunLoopRunSpecific + 438
    frame #12: 0x00007fff384c0bb0 GraphicsServices`GSEventRunModal + 65
    frame #13: 0x00007fff48092d4d UIKitCore`UIApplicationMain + 1621
    frame #14: 0x0000000101523104 EmptySimpleApp`main(argc=1, argv=0x00007ffeee6dbd38) at main.m:18:12
    frame #15: 0x00007fff5227ec25 libdyld.dylib`start + 1
    frame #16: 0x00007fff5227ec25 libdyld.dylib`start + 1
(lldb) 
```

&emsp;其中的 [UIApplication sendEvent:] 开始进行 UIEvent 事件在 App 中的分发，看到随后并分发到了 UIWindow 中。下面涉及到第一响应者的寻找以及响应者链执行事件，下面我们继续分章节进行学习。
## [UIApplication sendEvent:]
&emsp;将事件调度到应用程序中的相应响应者对象。
```c++
- (void)sendEvent:(UIEvent *)event;
```
&emsp;`event`: 一个 UIEvent 对象，封装有关事件的信息，包括涉及的触摸。

&emsp;如果需要，可以通过子类化 UIApplication 并重写此方法来拦截传入的事件。对于你拦截的每个事件，必须在实现中处理事件后通过调用 [super sendEvent:event] 进行调度。

&emsp;根据上面的函数调用堆栈 UIApplication 会接着把 event 发送到 UIWindow 中。
## [UIWindow sendEvent:]
&emsp;将指定的事件调度到其 view。
```c++
- (void)sendEvent:(UIEvent *)event; // called by UIApplication to dispatch events to views inside the window 由 UIApplication 调用以将事件调度到 window 内的 view 
```
&emsp;`event`: 要调度的事件。

&emsp;UIApplication 对象调用此方法将事件调度到 window。window 对象将触摸事件调度到发生触摸的 view，并将其他类型的事件分派到最合适的目标对象。你可以根据需要在应用程序中调用此方法以调度你创建的自定义事件。例如，你可以调用此方法将自定义事件调度到 window 的响应者链。

&emsp;看到这里我们大概明白了 UIApplication 会把 UIEvent 发送到 UIWindow，那么 UIEvent 是什么呢？那么我们的思路切下分支来看一下 UIEvent 的文档，然后再回到我们的主分支。
## UIEvent
&emsp;描述单个用户与你的应用交互的对象。
```c++
UIKIT_EXTERN API_AVAILABLE(ios(2.0)) @interface UIEvent : NSObject
```
&emsp;应用程序可以接收许多不同类型的事件，包括触摸事件（touch events）、运动事件（motion events）、远程控制事件（remote-control events）和按键事件（press events）。
+ 触摸事件是最常见的，并且被传递到最初发生触摸的 view 中。
+ 运动事件是 UIKit 触发的，与 Core Motion 框架报告的运动事件是分开的。
+ 远程控制事件允许响应者对象从外部附件或耳机接收命令，以便它可以管理音频和视频的管理，例如，播放视频或跳至下一个音轨。
+ 按键事件表示与游戏控制器、AppleTV 遥控器或其他具有物理按钮的设备的交互。
&emsp;可以使用类型（`type`）和子类型（`subtype`）属性确定事件的类型。

&emsp;触摸事件对象包含与事件有某种关系的 touches（即屏幕上的手指）。触摸事件对象可以包含一个或多个 touch，并且每个触摸都由 UITouch 对象表示。
当触摸事件发生时，系统将其路由到相应的响应者并调用相应的方法，如 touchesBegan:withEvent:。然后，响应者使用 touches 来确定适当的行动方案。

&emsp;在多点触摸序列中，UIKit 在将更新的 touch 数据传递到你的应用程序时会重用同一 UIEvent 对象。你永远不应 retain UIEvent 对象或 UIEvent 对象返回的任何对象。如果需要在用于处理该数据的响应程序方法之外保留数据，请将该数据从 UITouch 或 UIEvent 对象复制到本地数据结构。

&emsp;有关如何在 UIKit 应用中处理事件的更多信息，请参见 Event Handling Guide for UIKit Apps。（UIKit 文档内容过多，这里我们只阅读 Handling Touches in Your View 文档）
### UIEventType
&emsp;指定事件的常规类型。
```c++
typedef NS_ENUM(NSInteger, UIEventType) {
    UIEventTypeTouches,
    UIEventTypeMotion,
    UIEventTypeRemoteControl,
    UIEventTypePresses API_AVAILABLE(ios(9.0)),
};
```
&emsp;你可以从 type 属性获取事件的类型。为了进一步识别事件，你可能还需要确定其子类型，该子类型是从 subtype 属性获得的。
+ UIEventTypeTouches: 该事件与屏幕上的触摸有关。
+ UIEventTypeMotion: 该事件与设备的运动有关，例如用户摇晃设备。
+ UIEventTypeRemoteControl: 该事件是一个远程控制事件。远程控制事件源于从耳机或外部附件接收的命令，用于控制设备上的多媒体。
+ UIEventTypePresses: 该事件与按下物理按钮有关。

### UIEventSubtype
&emsp;指定事件的子类型（相对于其常规类型）。
```c++
typedef NS_ENUM(NSInteger, UIEventSubtype) {
    // available in iPhone OS 3.0
    UIEventSubtypeNone                              = 0,
    
    // for UIEventTypeMotion, available in iPhone OS 3.0
    UIEventSubtypeMotionShake                       = 1,
    
    // for UIEventTypeRemoteControl, available in iOS 4.0
    UIEventSubtypeRemoteControlPlay                 = 100,
    UIEventSubtypeRemoteControlPause                = 101,
    UIEventSubtypeRemoteControlStop                 = 102,
    UIEventSubtypeRemoteControlTogglePlayPause      = 103,
    UIEventSubtypeRemoteControlNextTrack            = 104,
    UIEventSubtypeRemoteControlPreviousTrack        = 105,
    UIEventSubtypeRemoteControlBeginSeekingBackward = 106,
    UIEventSubtypeRemoteControlEndSeekingBackward   = 107,
    UIEventSubtypeRemoteControlBeginSeekingForward  = 108,
    UIEventSubtypeRemoteControlEndSeekingForward    = 109,
};
```
&emsp;你可以从 subtype 属性获取事件的子类型。
+ UIEventSubtypeNone: 该事件没有子类型。这是 UIEventTypeTouches 常规类型的事件的子类型。
+ UIEventSubtypeMotionShake: 该事件与用户摇晃设备有关。它是 UIEventTypeMotion 常规事件类型的子类型。
+ UIEventSubtypeRemoteControlPlay: 播放音频或视频的远程控制事件。它是 UIEventTypeRemoteControl 常规事件类型的子类型。
+ UIEventSubtypeRemoteControlPause: 暂停音频或视频的远程控制事件。它是 UIEventTypeRemoteControl 常规事件类型的子类型。
+ UIEventSubtypeRemoteControlStop: 用于停止播放音频或视频的远程控制事件。它是 UIEventTypeRemoteControl 常规事件类型的子类型。
+ UIEventSubtypeRemoteControlTogglePlayPause: 在播放和暂停之间切换音频或视频的远程控制事件。它是 UIEventTypeRemoteControl 常规事件类型的子类型。
+ UIEventSubtypeRemoteControlNextTrack: 跳至下一个音频或视频轨道的远程控制事件。它是 UIEventTypeRemoteControl 常规事件类型的子类型。
+ UIEventSubtypeRemoteControlPreviousTrack: 跳到上一个音频或视频轨道的远程控制事件。它是 UIEventTypeRemoteControl 常规事件类型的子类型。
+ UIEventSubtypeRemoteControlBeginSeekingBackward: 一个远程控制事件，开始通过音频或视频媒体向后搜索。它是 UIEventTypeRemoteControl 常规事件类型的子类型。
+ UIEventSubtypeRemoteControlEndSeekingBackward: 结束通过音频或视频媒体向后搜索的远程控制事件。它是 UIEventTypeRemoteControl 常规事件类型的子类型。 
+ UIEventSubtypeRemoteControlBeginSeekingForward: 一个开始通过音频或视频介质向前搜索的远程控制事件。它是 UIEventTypeRemoteControl 常规事件类型的子类型。
+ UIEventSubtypeRemoteControlEndSeekingForward: 结束通过音频或视频介质向前搜索的远程控制事件。它是 UIEventTypeRemoteControl 常规事件类型的子类型。

### type
&emsp;返回事件的类型。
```c++
@property(nonatomic,readonly) UIEventType type API_AVAILABLE(ios(3.0));
```
&emsp;此属性返回的 UIEventType 常量指示此事件的常规类型，例如，它是触摸事件还是运动事件。
### subtype
&emsp;返回事件的子类型。
```c++
@property(nonatomic,readonly) UIEventSubtype  subtype API_AVAILABLE(ios(3.0));
```
&emsp;此属性返回的 UIEventSubtype 常量指示与常规类型相关的事件的子类型，该子类型是从 type 属性返回的。
### timestamp
&emsp;事件发生的时间。
```c++
@property(nonatomic,readonly) NSTimeInterval  timestamp;
```
&emsp;此属性包含自系统启动以来经过的秒数。有关此时间值的描述，请参见 NSProcessInfo 类的 systemUptime 方法的描述。
### allTouches
&emsp;返回与事件关联的所有 touches。（alltouchs 属性只包含触摸序列中的最后一次触摸）
```c++
@property(nonatomic, readonly, nullable) NSSet <UITouch *> *allTouches; // 只读集合
```
&emsp;一组 UITouch 对象，表示与事件关联的所有 touches。

&emsp;如果事件的 touches 源自不同的 views 和 windows，则从此方法获得的 UITouch 对象将与不同的响应者对象相关联。

&emsp;（在 touchesBegan:withEvent: 函数中 touches 参数和 event.allTouches 是完全一样的，那这里可能会迷惑既然 event 中完全包含 touches 那为什么还要用两个参数呢？传递一个 event 对象不就够了吗，比如看 [UIApplication sendEvent:] 中就仅传了一个 event，其实这里是因为 touchesBegan:withEvent: 函数中可以仅传 touches 参数，把 event 参数传 nil 的。因为仅使用 touches 即可进行 hit-testing 来判断触摸点是否在某个 view 上，便可判断该 view 是否可以作为响应者。（如果要从事件处理代码外部调用此方法，则可以指定 nil。））

### touchesForWindow:
&emsp;从 UIEvent 中返回属于指定 window 的 UITouch 对象。
```c++
- (nullable NSSet <UITouch *> *)touchesForWindow:(UIWindow *)window;
```
&emsp;`window`: 最初发生 touches 的 UIWindow 对象。

&emsp;一组 UITouch 对象，它们代表属于指定 window 的触摸。
### touchesForView:
&emsp;从 UIEvent 中返回属于指定 view 的 UITouch 对象。
```c++
- (nullable NSSet <UITouch *> *)touchesForView:(UIView *)view;
```
&emsp;`view`: 最初发生 touches 的 UIView 对象。

&emsp;一组 UITouch 对象，它们代表属于指定 view 的触摸。
### touchesForGestureRecognizer:
&emsp;返回要传递到指定 gesture recognizer 的 UITouch 对象。
```c++
- (nullable NSSet <UITouch *> *)touchesForGestureRecognizer:(UIGestureRecognizer *)gesture API_AVAILABLE(ios(3.2));
```
&emsp;`gesture`: 抽象基类 UIGestureRecognizer 的子类的实例。必须将此 gesture-recognizer object  附加到视图，以接收对该视图及其子视图进行了 hit-tested。

&emsp;一组表示触摸的 UITouch 对象，这些对象将传递给 receiver 表示的 UIEvent 的指定 gesture recognizer。

&emsp;下面两个函数用于获取 UIEvent 对象的 main  touch objecct 关联的 UITouch 和预测的 UITouch 数组。主要用于 Apple Pencil 获取高精度的 touch 输入（在 iPad 上使用 Apple Pencil 时，iPad 屏幕刷新会提到 120 赫兹）以及预测 touch  走向，提高用户体验。（不想看的可以忽略）

### coalescedTouchesForTouch:
&emsp;返回与指定 main touch 关联的所有 touches。（一组针对给定 main touch 未传递的触摸事件的辅助 UITouch。这还包括 main touch 本身的辅助版本。）
```c++
- (nullable NSArray <UITouch *> *)coalescedTouchesForTouch:(UITouch *)touch API_AVAILABLE(ios(9.0));
```
&emsp;`touch`: 与 event 一起报告的 main touch 对象。你指定的触摸对象确定返回附加触摸的序列。

&emsp;Return Value: UITouch 对象的数组，代表自上次传递事件以来针对指定 UITouch 报告的所有 UITouch。数组中对象的顺序与将触摸报告给系统的顺序匹配，最后一次触摸是你在 touch 参数中指定的同一 UITouch 的副本。如果 touch 参数中的对象与当前事件不关联，则返回值为 nil。

&emsp;使用此方法可获取系统接收到但未在上一个 UIEvent 对象中传递的任何其他 UITouch。一些设备以高达 240 赫兹的频率收集 UITouch 数据，这通常高于将 UITouch 传送到应用程序的速率。尽管这些额外的 UITouch 数据提供了更高的精度，但许多应用程序并不需要这种精度，也不想招致与处理它们相关的开销。但是，想要提高精度的应用程序可以使用此方法来检索额外的 UITouch 对象。例如，绘图应用程序可以使用这些 UITouch 来获取用户绘图输入的更精确记录。然后，你可以将额外的 UITouch 数据应用于应用程序的内容。如果你想要合并的 UITouch 或使用传递给响应者方法的一组 UITouch，请使用此方法，但不要将两组 UITouch 混合在一起。此方法返回自上次事件以来报告的 UITouch 的完整序列，包括报告给响应者方法的 UITouch 的副本。传递给响应者方法的事件只包含序列中的最后一次触摸。类似地，alltouchs 属性只包含序列中的最后一次触摸。（想到了 Apple pencil 在 iPad 上使用时，iPad 屏幕刷新会提高到 120 赫兹）
### predictedTouchesForTouch:
&emsp;返回预计将针对指定 UITouch 发生的 UITouch 数组。（一组辅助 UITouch，用于预测在给定的 main touch 下会发生的触摸事件。这些预测可能与触摸的真实行为不完全匹配，因此应将其解释为一种估计。）
```c++
- (nullable NSArray <UITouch *> *)predictedTouchesForTouch:(UITouch *)touch API_AVAILABLE(ios(9.0));
```
&emsp;`touch`: 与事件一起报告的 main touch 对象。你指定的 UITouch 对象用于确定返回附加 UITouch 的序列。

&emsp;Return Value: 一组 UITouch 对象，它们表示系统将预测的下一组 UITouch。数组中对象的顺序与预期将触摸传递到你的应用程序的顺序匹配。该数组不包括你在 touch 参数中指定的原始 UITouch。如果 touch 参数中的对象与当前事件没有关联，则返回值为 nil。

&emsp;使用此方法可以最小化用户的触摸输入和屏幕内容呈现之间的明显延迟。处理来自用户的 UITouch 输入并将该信息转换为绘图命令需要时间，而将这些绘图命令转换为呈现的内容则需要额外的时间。如果用户的手指或 Apple Pencil 移动速度足够快，这些延迟可能会导致当前触摸位置和渲染内容之间出现明显的间隙。为了最大限度地减少感觉到的延迟，请使用此方法的预期效果作为对内容的附加临时输入。

&emsp;此方法返回的 UITouch 表示系统根据用户过去的输入估计用户的触摸输入将在何处。仅将这些 UITouch 暂时附加到用于绘制或更新内容的结构中，并在收到带有新 UITouch 的新事件后立即丢弃它们。当与合并的 UITouch 和高效的绘图代码结合使用时，你可以创建一种感觉，即用户的输入被立即处理，几乎没有延迟。这种感觉改善了用户对绘图应用程序或任何让用户直接在屏幕上操作对象的应用程序的体验。

&emsp;UIEvent 文档共有这么多内容，并不多，其实并不应该想当然的觉得 UIEvent 复杂，其实它很简单，我们更需要关注的是 UIEvent 中包含的手指触摸位置，只有这些位置或者说是触摸点信息才是最重要的，下面我们看一下 UITouch 的文档。

## UITouch
&emsp;表示屏幕上发生的触摸的位置（location）、大小（size）、移动（movement）和力度（force，针对 3D Touch 和 Apple Pencil）的对象。
```c++
UIKIT_EXTERN API_AVAILABLE(ios(2.0)) @interface UITouch : NSObject
```
&emsp;打印一个 UITouch 对象的可看到如下内容:
```c++
<UITouch: 0x7f9089614b70> 
phase: Moved 
tap count: 1 
force: 0.000 

window: <UIWindow: 0x7f908950d170; frame = (0 0; 375 812); gestureRecognizers = <NSArray: 0x600003c5ff90>; layer = <UIWindowLayer: 0x60000321e4e0>> 
view: <CustomView: 0x7f90897078d0; frame = (112.667 331; 150 150); autoresize = RM+BM; layer = <CALayer: 0x600003219a80>> 

location in window: {219.33332824707031, 428.66665649414062} 
previous location in window: {220, 429} 
location in view: {106.66666158040363, 97.666656494140625} 
previous location in view: {107.33333333333331, 98}
```
&emsp;你可以通过传递给响应者对象（UIResponder 或其子类）的 UIEvent 对象访问 touch 对象（allTouches 属性），以进行事件处理。touch 对象包括用于以下对象的访问：

+ 发生触摸的 view 或 window
+ 触摸在 view 或 window 中的位置
+ 触摸的近似半径（approximate radius）
+ 触摸的力度（force）（在支持 3D Touch 或 Apple Pencil 的设备上）

&emsp;touch 对象还包含一个时间戳，该时间戳指示何时发生触摸；一个整数，代表用户 tapped 屏幕的次数；以及触摸的阶段，其形式为常数，描述了触摸是开始，移动还是结束，或系统是否取消触摸。

&emsp;要了解如何使用 swipes 手势，请阅读 Event Handling Guide for UIKit Apps 中的 Handling Swipe and Drag Gestures。

&emsp;touch 对象在多点触摸序列（multi-touch sequence）中始终存在。你可以在处理多点触控序列时存储对 touch 的引用，只要在序列结束时释放该引用即可。如果需要在多点触控序列之外存储有关 touch 的信息，请从 touch 中复制该信息。

&emsp;touch 的 gestureRecognizers 属性包含当前正在处理 touch 的 gesture recognizers。每个 gesture recognizer 都是 UIGestureRecognizer 的具体子类的实例。
### locationInView:
&emsp;返回给定 view 坐标系中 UITouch 对象的当前位置。（即返回一个 UITouch 对象在 view 的坐标系中的位置（CGPoint））
```c++
- (CGPoint)locationInView:(nullable UIView *)view;
```
&emsp;`view`: 要在其坐标系中定位 touch 的视图对象。处理 touch 的自定义视图可以指定 self 以在其自己的坐标系中获取 touch 位置。传递 nil 以获取 window 坐标系中的 touch 位置。

&emsp;Return Value: 一个指定 UITouch 在 view 中位置的 point。

&emsp;此方法返回 UITouch 对象在指定 view 的坐标系中的当前位置。因为 touch 对象可能已从另一个视图转发到一个视图，所以此方法将 touch 位置执行任何必要的转换到指定 view 的坐标系。
### previousLocationInView:
&emsp;返回 UITouch 在给定 view 坐标系中的先前位置。
```c++
- (CGPoint)previousLocationInView:(nullable UIView *)view;
```
&emsp;`view`: 要在其坐标系中定位 touch 的视图对象。处理 touch 的自定义视图可以指定 self 以在其自己的坐标系中获取 touch 位置。传递 nil 以获取 window 坐标系中的 touch 位置。

&emsp;Return Value: 此方法返回 UITouch 对象在指定 view 的坐标系中的上一个位置。因为 touch 对象可能已从另一个视图转发到一个视图，所以此方法将 touch 位置执行任何必要的转换到指定 view 的坐标系。
### view
&emsp;UITouch 要传递到的视图（如果有的话）。
```c++
@property(nullable,nonatomic,readonly,strong) UIView *view;
```
&emsp;此属性的值是将 touche 传递到的 view 对象，不一定是 touch 当前所在的 view。例如，当 gesture recognizer 识别到 touch 时，此属性为 nil，因为没有 view 在接收 touch。
### window
&emsp;最初发生 touch 的 window。
```c++
@property(nullable,nonatomic,readonly,strong) UIWindow *window;
```
&emsp;该属性的值是最初发生 touch 的 window。该 window 可能与当前包含 touch 的 window 不同。
### majorRadius
&emsp;touch 的半径（以点（points）表示）。
```c++
@property(nonatomic,readonly) CGFloat majorRadius API_AVAILABLE(ios(8.0));
```
&emsp;使用此属性中的值确定硬件报告的 touch 大小。此值是大小的近似值，可以根据 majorRadiusTolerance 属性中指定的量而变化。
### majorRadiusTolerance
&emsp;touch 的半径的容差（以点表示）。
```c++
@property(nonatomic,readonly) CGFloat majorRadiusTolerance API_AVAILABLE(ios(8.0));
```
&emsp;此值确定 majorRadius 属性中值的准确性。将此值添加到半径以获得最大触摸半径。减去该值以获得最小触摸半径。
### preciseLocationInView:
&emsp;返回 UITouch 的精确位置（如果可用）。
```c++
- (CGPoint)preciseLocationInView:(nullable UIView *)view API_AVAILABLE(ios(9.1));
```
&emsp;`view`: 包含 touch 的视图。

&emsp;Return Value: touch 的精确位置。

&emsp;使用此方法可获取 touch 的额外精度（如果可用）。不要使用返回点进行命中测试（hit testing）。在某些情况下，命中测试可能表示 touch 位于视图中，但针对更精确位置的命中测试可能表示 touch 在视图之外。
### precisePreviousLocationInView:
&emsp;返回 touch 的精确先前位置（如果可用）。
```c++
- (CGPoint)precisePreviousLocationInView:(nullable UIView *)view API_AVAILABLE(ios(9.1));
```
&emsp;使用此方法可以获得 touch 先前位置的额外精度（如果可用）。不要使用返回点进行命中测试。在某些情况下，命中测试可能表示 touch 位于视图中，但针对更精确位置的命中测试可能表示 touch 在视图之外。
### tapCount
&emsp;给定 touch 的 tap 次数。
```c++
@property(nonatomic,readonly) NSUInteger tapCount; // 在一定时间内在某个点内触摸
```
&emsp;此属性的值是一个整数，包含在预定义的时间段内此 touch 发生的点击数。使用此属性可评估用户是单点、双点、还是甚至三击特定 view 或 window。
### timestamp
&emsp;touch 发生的时间或上次发生 mutated 的时间。
```c++
@property(nonatomic,readonly) NSTimeInterval timestamp;
```
&emsp;此属性的值是自系统启动以来触发 touch 或上次更改 touch 的时间（以秒为单位）。你可以存储此属性的值，并将其与后续 UITouch 对象中的时间戳进行比较，以确定触摸的持续时间，如果 touch 正在轻扫，则确定移动速度。有关系统启动后的时间定义，请参阅 NSProcessInfo 类的 systemUptime 方法的说明。
### UITouchType
&emsp;touch 类型。
```c++
typedef NS_ENUM(NSInteger, UITouchType) {
    UITouchTypeDirect, // A direct touch from a finger (on a screen) 手指直接触摸（在屏幕上）
    UITouchTypeIndirect, // An indirect touch (not a screen) 间接触摸（不是屏幕）
    UITouchTypePencil API_AVAILABLE(ios(9.1)), // Add pencil name variant 添加 pencil 名称变体 
    UITouchTypeStylus API_AVAILABLE(ios(9.1)) = UITouchTypePencil, // A touch from a stylus (deprecated name, use pencil) 手写笔的触摸（已弃用，UITouchTypePencil）
    
    // A touch representing a button-based, indirect input device describing the input sequence from button press to button release
    // 表示基于按钮的间接输入设备的触摸，描述从按钮按下到按钮释放的输入序列
    UITouchTypeIndirectPointer API_AVAILABLE(ios(13.4), tvos(13.4)) API_UNAVAILABLE(watchos),
    
} API_AVAILABLE(ios(9.0));
```
+ UITouchTypeDirect: 与屏幕直接接触产生的触摸。当用户的手指接触屏幕时，会发生直接接触。
+ UITouchTypeIndirect: 不是接触屏幕造成的触摸。间接触摸是由与屏幕分离的触摸输入设备产生的。例如，Apple TV 遥控器的触控板会产生间接触摸。
+ UITouchTypePencil: Apple Pencil 的 touch。当 Apple Pencil 与设备的屏幕交互时，会发生 Pencil Touch。
+ UITouchTypeStylus: 已废弃，使用 UITouchTypePencil 代替。

### type
&emsp;表示 touch 类型的属性。
```c++
@property(nonatomic,readonly) UITouchType type API_AVAILABLE(ios(9.0));
```
&emsp;有关触摸类型的完整列表，请参阅 maximumPossibleForce。
### UITouchPhase
&emsp;touch 的阶段。
```c++
typedef NS_ENUM(NSInteger, UITouchPhase) {
    UITouchPhaseBegan, // whenever a finger touches the surface. 只要手指碰到表面。
    UITouchPhaseMoved, // whenever a finger moves on the surface. 当手指在表面上移动时。
    UITouchPhaseStationary, // whenever a finger is touching the surface but hasn't moved since the previous event. 当手指接触到表面，但自上次事件后没有移动时。
    UITouchPhaseEnded, // whenever a finger leaves the surface. 当手指离开表面。
    UITouchPhaseCancelled, // whenever a touch doesn't end but we need to stop tracking (e.g. putting device to face) 当触摸未结束但我们需要停止跟踪时（例如，接听电话时将设备放在脸上移动、或者识别到触摸是手势后会强制把 touch 置为 cancel）
    
    UITouchPhaseRegionEntered   API_AVAILABLE(ios(13.4), tvos(13.4)) API_UNAVAILABLE(watchos),  // whenever a touch is entering the region of a user interface 每当触摸进入用户界面区域时
    
    // when a touch is inside the region of a user interface, but hasn’t yet made contact or left the region
    // 当触摸位于用户界面区域内，但尚未联系或离开该区域时
    UITouchPhaseRegionMoved     API_AVAILABLE(ios(13.4), tvos(13.4)) API_UNAVAILABLE(watchos),
    
    UITouchPhaseRegionExited    API_AVAILABLE(ios(13.4), tvos(13.4)) API_UNAVAILABLE(watchos),  // when a touch is exiting the region of a user interface 当触摸退出用户界面区域时
};
```
&emsp;UITouch 实例的阶段随着系统在事件过程中接收更新而改变。通过 phase 属性访问此值。

+ UITouchPhaseBegan: 屏幕上按下了对给定事件的 touch。
+ UITouchPhaseMoved: 给定事件的 touch 已在屏幕上移动。
+ UITouchPhaseStationary: 在屏幕上按下给定事件的 touch，但自上一个事件后就再也没有移动过。
+ UITouchPhaseEnded: 给定事件的 touch 已从屏幕上抬起。
+ UITouchPhaseCancelled: 例如，当用户将设备靠在脸上移动时，系统取消了对触摸的跟踪。
+ UITouchPhaseRegionEntered: 给定事件的触摸已进入屏幕上的 window。UITouchPhaseRegionEntered、UITouchPhaseRegionMoved 和 UITouchPhaseRegionSited 阶段并不总是与 UIHoverGestureRecognizer 的状态属性对齐。hover gesture recognizer 的状态仅适用于 gesture’s 视图的上下文，而 touch states 适用于 window。
+ UITouchPhaseRegionMoved: 给定事件的触摸在屏幕上的窗口内，但尚未按下。
+ UITouchPhaseRegionExited: 对给定事件的触摸在屏幕上留下了一个窗口。

### phase
&emsp;touch 阶段。属性值是一个常量，指示触摸是开始、移动、结束还是取消。有关此属性可能值的描述，请参见 UITouchPhase。
```c++
@property(nonatomic,readonly) UITouchPhase phase;
```
### gestureRecognizers
&emsp;接收 touch 对象的 gesture recognizers。
```c++
@property(nullable,nonatomic,readonly,copy)   NSArray <UIGestureRecognizer *> *gestureRecognizers API_AVAILABLE(ios(3.2));
```
&emsp;数组中的对象是抽象基类 UIGestureRecognizer 的子类的实例。如果当前没有接收 touch 的 gesture recognizers，则此属性包含空数组。

&emsp;⬇️⬇️ 下面是一些与 3D Touch、Apple Pencil 相关的内容，不感兴趣的话可直接忽略。

### force
&emsp;touch 的力，其中值 1.0 表示平均触摸力（由系统预先确定，而不是特定于用户）。
```c++
@property(nonatomic,readonly) CGFloat force API_AVAILABLE(ios(9.0));
```
&emsp;此属性在支持 3D Touch 或 Apple Pencil 的设备上可用。要在运行时检查设备是否支持 3D Touch，请读取应用程序中具有 trait 环境的任何对象的 trait 集合上 forceTouchCapability 属性的值。

&emsp;Apple Pencil 所报告的力是沿着 Pencil 的轴线测量的。如果想要垂直于设备的力，需要使用 altitudeAngle 值计算该值。

&emsp;Apple Pencil 报告的力最初是估计的，可能并不总是更新。要确定是否需要更新，请参阅 estimatedPropertiesExpectingUpdates 并查找 UITouchPropertyForce 标志。在这种情况下，estimationUpdateIndex 索引包含一个非 nil 值，你可以在更新发生时将该值与原始触摸相关联。当没有预期的力更新时，整个触摸序列通常不会有更新，因此可以对触摸序列应用自定义的、特定于工具的力曲线。
### maximumPossibleForce
&emsp;touch 的最大可能力。
```c++
@property(nonatomic,readonly) CGFloat maximumPossibleForce API_AVAILABLE(ios(9.0));
```
&emsp;该属性的值足够高，可以为 force 属性的值提供广泛的动态范围。

&emsp;此属性在支持 3D Touch 或 Apple Pencil 的设备上可用。要在运行时检查设备是否支持 3D Touch，请读取应用程序中具有 trait 环境的任何对象的 trait 集合上 forceTouchCapability 属性的值。
### altitudeAngle
&emsp;Pencil 的高度（弧度）。仅适用于 UITouchTypePencil 类型。
```c++
@property(nonatomic,readonly) CGFloat altitudeAngle API_AVAILABLE(ios(9.1));
```
&emsp;值为 0 弧度表示 Apple Pencil 与曲面平行。当 Apple Pencil 垂直于曲面时，此属性的值为 Pi/2。

&emsp;下面好像暂时用不到的两个方法。
### azimuthAngleInView:
&emsp;返回 Apple Pencil 的方位角（以弧度为单位）。
```c++
- (CGFloat)azimuthAngleInView:(nullable UIView *)view API_AVAILABLE(ios(9.1));
```
&emsp;在屏幕平面中，方位角是指手写笔指向的方向。当触针尖端接触屏幕时，当触针的帽端（即尖端对面的端部）指向设备屏幕的正x轴时，此属性的值为0弧度。当用户围绕笔尖顺时针方向摆动手写笔的笔帽端时，方位角会增加。
> &emsp;Note: 获取方位角（相对于方位单位矢量）的成本更高，但也更方便。
### azimuthUnitVectorInView:
&emsp;返回指向 Apple Pencil 方位角方向的单位向量。
```c++
- (CGVector)azimuthUnitVectorInView:(nullable UIView *)view API_AVAILABLE(ios(9.1));
```
&emsp;得到方位单位矢量比得到方位角要便宜。如果要创建变换矩阵，单位向量也会更有用。
### UITouchProperties
&emsp;一些可能会更新的 touch 属性的位掩码。
```c++
typedef NS_OPTIONS(NSInteger, UITouchProperties) {
    UITouchPropertyForce = (1UL << 0),
    UITouchPropertyAzimuth = (1UL << 1),
    UITouchPropertyAltitude = (1UL << 2),
    UITouchPropertyLocation = (1UL << 3), // For predicted Touches 对于预测的触摸
} API_AVAILABLE(ios(9.1));
```
+ UITouchPropertyForce: 位掩码中表示 force 的 touch 属性。
+ UITouchPropertyAzimuth: 位掩码中表示 azimuth（方位角） 的 touch 属性。（用于 Apple Pencil）
+ UITouchPropertyAltitude: 位掩码中表示 altitude（高度/海拔） 的 touch 属性。（用于 Apple Pencil）
+ UITouchPropertyLocation: 位掩码中表示 location 的 touch 属性。

### estimatedProperties
&emsp;一组 touch 属性，其值仅包含估计值。
```c++
@property(nonatomic,readonly) UITouchProperties estimatedProperties API_AVAILABLE(ios(9.1));
```
&emsp;此属性包含一个常量位掩码，表示无法立即报告哪些触摸属性。例如，Apple Pencil 记录了触摸的力度，但必须通过空中传输信息到底层 iPad。传输数据所产生的延迟可能会导致在向应用程序报告触摸后接收信息。

&emsp;不保证以后更新此属性中的值。有关希望更新其值的属性列表，请参阅 estimatedPropertiesExpectingUpdates。
### estimatedPropertiesExpectingUpdates
&emsp;一组 touch 属性，预计将来会更新这些属性的值。
```c++
@property(nonatomic,readonly) UITouchProperties estimatedPropertiesExpectingUpdates API_AVAILABLE(ios(9.1));
```
&emsp;此属性包含常量的位掩码，该位掩码指示哪些触摸属性无法立即报告，哪些触摸属性需要稍后更新。当此属性包含非空集时，可以期望 UIKit 稍后使用给定属性的更新值调用响应程序或手势识别器的 toucheEstimatedPropertiesUpdated: 方法。将 estimationUpdateIndex 属性中的值附加到应用程序的触摸数据副本。当 UIKit 稍后调用 toucheEstimatedPropertiesUpdated: 方法时，使用新 touch 的估计更新索引来定位和更新应用程序的 touch 数据副本。

&emsp;当此属性包含空集时，不需要更多更新。在该场景中，估计值或更新值是最终值。
### estimationUpdateIndex
&emsp;一个索引编号，用于将更新的 touch 与原始 touch 关联起来。
```c++
@property(nonatomic,readonly) NSNumber * _Nullable estimationUpdateIndex API_AVAILABLE(ios(9.1));
```
&emsp;此属性包含当前触摸数据的唯一标记。当触摸包含估计属性时，将此索引与其余触摸数据一起保存到应用程序的数据结构中。当系统稍后报告实际触摸值时，使用此索引定位应用程序数据结构中的原始数据，并替换先前存储的估计值。例如，当触摸包含估计属性时，可以将此属性用作字典中的键，字典的值是用于存储触摸数据的对象。

&emsp;对于包含估计属性的每个 touch，此属性的值都会单调增加。当 touch 对象不包含估计或更新的属性时，此属性的值为零。

&emsp;UITouch 的内容就这么多，也比较简单，最重要的记录其在 view  和 window 中的坐标点以及 phase 等下在学习 UIResponder 是会用到，UITouch 在不同的阶段时响应者会调用不同的响应函数。（touchesBegan:withEvent:、touchesMoved:withEvent:、touchesEnded:withEvent:、touchesCancelled:withEvent:）

&emsp;UIResponder 响应者相关的内容我们放在后面，现在 UIEvent 和 UITouch 看完了，我们继续顺着上面的 [UIApplication sendEvent:]、[UIWindow sendEvent:] 向下看，此时需要一层一层找到第一响应者。  那么我们如何找到第一响应者呢，我们继续向下看。  

## Hit-Testing
&emsp;判断一个 touch 的触摸点是否在一个 view 中涉及到下面 UIView 类中的几个函数。（UIWindow 继承自 UIView 大家应该都知道的）
### hitTest:withEvent:
&emsp;返回包含指定点（`point`）的视图层次结构中 UIView 的最远子视图（最远子视图，也可能是其自身）。
```c++
- (UIView *)hitTest:(CGPoint)point withEvent:(UIEvent *)event;
```
&emsp;`point`: UIView 的本地坐标系（bounds）中指定的点。`event`: 需要调用此方法的事件。如果要从事件处理代码外部调用此方法，则可以指定 nil。

&emsp;Return Value: view 对象是当前 view 的最远子视图，并且包含 `point`。如果该 `point` 完全位于 UIView 的视图层次之外，则返回 nil。

&emsp;此方法通过调用每个子视图的 `pointInside:withEvent:` 方法来遍历视图层次结构，以确定哪个子视图应接收 touch 事件。如果 `pointInside:withEvent:` 返回 YES，然后类似地遍历其子视图的层次结构，直到找到包含 `point` 的最前面的视图。如果视图不包含该 `point`，则将忽略其视图层次结构的分支。你很少需要自己调用此方法，但可以重写它以从子视图中隐藏 touch 事件，或者扩大 view 响应范围。

&emsp;此方法将忽略 hidden 设置为 YES 的、禁用用户交互（userInteractionEnabled 设置为 NO）或 alpha 小于 0.01 的 view 对象。确定点击（determining a hit）时，此方法不会考虑 view 的内容。因此，即使 `point` 位于该 view 内容的透明部分中，该 view 仍然可以返回。

&emsp;超出 view 的 bounds 的 `point` 永远不会被报告为命中，即使它们实际上位于 receiver 的一个子视图中。如果当前视图的 clipsToBounds 属性设置为 NO，并且受影响的子视图超出了视图的边界，则会发生这种情况。（例如一个 button 按钮超出了其父试图的 bounds，此时点击 button 未超出父视图的区域的话可以响应点击事件，如果点击 button 超出父视图的区域的话则不能响应点击事件）

&emsp;hitTest:withEvent: 寻找一个包含 `point` 的 view 的过程可以理解为如下：
```c++
- (UIView *)hitTest:(CGPoint)point withEvent:(UIEvent *)event{
    // 3 种状态无法响应事件
    // 1): userInteractionEnabled 为 NO，禁止了用户交互。
    // 2): hidden 为 YES，被隐藏了。
    // 3): alpha 小于等于 0.01，透明度小于 0.01。
    if (self.userInteractionEnabled == NO || self.hidden == YES ||  self.alpha <= 0.01) return nil;
    
    // 触摸点若不在当前视图上则无法响应事件
    if ([self pointInside:point withEvent:event] == NO) return nil;
    
    // ⬇️⬇️⬇️ 从后往前遍历子视图数组（倒序）
    int count = (int)self.subviews.count;
    for (int i = count - 1; i >= 0; i--) {
        // 获取子视图
        UIView *childView = self.subviews[i];
        
        // 坐标系的转换，把触摸点在当前视图上坐标转换为在子视图上的坐标
        CGPoint childP = [self convertPoint:point toView:childView];
        
        // 询问子视图层级中的最佳响应视图（递归）
        UIView *fitView = [childView hitTest:childP withEvent:event];
        
        if (fitView) {
            // 如果子视图中有更合适的就返回
            return fitView;
        }
    }
    
    // 没有在子视图中找到更合适的响应视图，那么自身就是最合适的
    return self;
}
```
### pointInside:withEvent:
&emsp;返回一个布尔值，该值指示 UIView 是否包含 `point`。
```c++
- (BOOL)pointInside:(CGPoint)point withEvent:(UIEvent *)event;
```
&emsp;`point`: UIView 的本地坐标系（bounds）中指定的点。`event`: 需要调用此方法的事件。如果要从事件处理代码外部调用此方法，则可以指定 nil。

&emsp;如果 `point` 包含在 UIView 的 bounds 中，则返回 YES，否则返回 NO。
### convertPoint:toView:
&emsp;将 `point` 从 UIView 的坐标系转换为指定视图（`view`）的点（CGPoint）。
```c++
- (CGPoint)convertPoint:(CGPoint)point toView:(UIView *)view;
```
&emsp;`point`: UIView 的本地坐标系（bounds）中指定的点。

&emsp;以上是从一个指定 view 中找到最远的一个可以包含 touch 的子 view 的方法。所以上面顺着 [UIApplication sendEvent:]、[UIWindow sendEvent:] 再往下走其实就是我们 App 的根 window（[UIApplication sharedApplication].keyWindow，iOS 13 推出 UISceneSession 后获取根窗口的方式已经改变，这里我们还是使用以前的方式来获取根 window） 的根控制器（[UIApplication sharedApplication].keyWindow.rootViewController）的 View 了，然后一路随着 view  的层级关系一路寻找第一响应者（对每一个 view 进行 Hit-Testing）。那么找到第一响应者以后呢，就是直接把 event 交给它处理，而它能不能处理就是看它有没有实现 touches... 系列函数，如果有的话，则是直接执行，如果自己不想执行的话可以调用 super 函数，如果自己完全没有实现  touches... 系列函数的话，则是沿者响应者链一路向上去找可以响应此次事件的响应者。如果最终都没有找到的话，则把此次事件丢弃。那么接下来就是看 Responser 以及 Responder Chian 了，那么它们到底是什么呢？接着向下看吧...

&emsp;这里我们先阅读一下 UIResponder 的文档。
## UIResponder
&emsp;响应和处理事件的 abstract interface。（UIResponder 是一个极重要的继承自 NSObject 的抽象接口，它几乎是 UIKit 框架下所有 UI 类的父类（也有特殊，如 UIImage 类则是直接继承自 NSObject），正是从它建立了 NSObject 和 UI 层之间的连接。）
```c++
UIKIT_EXTERN API_AVAILABLE(ios(2.0)) @interface UIResponder : NSObject <UIResponderStandardEditActions>
```
&emsp;响应者对象（即 UIResponder 实例）构成 UIKit 应用程序的事件处理主干（event-handling backbone）。许多关键对象也是响应者，包括 UIApplication 对象、UIViewController 对象和所有 UIView 对象（包括 UIWindow）。当事件发生时，UIKit 会将它们调度给应用程序的响应者对象进行处理。

&emsp;有几种事件，包括触摸事件（touch events）、运动事件（motion events）、遥控事件（remote-control events）和按键事件（press events）。要处理特定类型的事件，响应者必须重写相应的方法。例如，为了处理触摸事件，响应者实现 touchesBegan:withEvent:、touchesMoved:withEvent:、touchesEnded:withEvent: 和 touchesCancelled:withEvent: 方法。在触摸的情况下，响应者使用 UIKit 提供的事件信息（UIEvent）来跟踪这些触摸的变化，并适当地更新应用程序的界面。

&emsp;除了处理事件外，UIKit 响应者还管理将未处理的事件转发到应用程序的其他部分。如果给定的响应者不能处理事件，它会将该事件转发给响应者链中的下一个响应者（文档应该错了，文档写的是 "next event"）。UIKit 动态管理响应者链，使用预定义的规则来确定下一个接收事件的响应者对象。例如，view 将事件转发到其 superview，层次结构的 root view 将事件转发到其 view controller。

&emsp;响应者处理 UIEvent 对象，但也可以通过 input view 接受 custom input view。系统的键盘是 input view 最明显的例子。当用户在屏幕上点击 UITextField 和 UITextView 对象时，此 view 将成为第一个响应者，并显示其 input view，即系统键盘。类似地，你可以创建 custom input views，并在其他响应者激活时显示它们。要将 custom input view 与响应者关联，请将此 view 指定给响应者的 inputView 属性。

&emsp;有关响应者和响应者链的信息，请参见 Using Responders and the Responder Chain to Handle Events。（后面我们会详细学习该文档）
### Managing the Responder Chain（管理响应者链）
#### nextResponder
&emsp;返回响应者链中的下一个响应者，如果没有下一个响应者，则返回 nil。
```c++
@property(nonatomic, readonly, nullable) UIResponder *nextResponder;
```
&emsp;Return Value: 响应者链中的下一个对象；如果这是链中的最后一个对象，则为 nil。

&emsp;UIResponder 类不会自动存储或设置下一个响应者，因此此方法在默认情况下返回 nil。子类必须重写此方法并返回适当的下一个响应者。例如，UIView 实现此方法并返回管理它的 UIViewController 对象（如果有）或它的 superview 对象（如果没有 UIViewController）。UIViewController 类似地实现该方法并返回其 view 的 superview（测试的是返回 null）。UIWindow 返回应用程序对象（application）（或 UIWindowScene）。共享 UIApplication 对象（shared UIApplication object）通常返回 nil，但如果该对象是 UIResponder 的子类并且尚未被调用来处理事件，则返回其 app delegate。
#### isFirstResponder
&emsp;返回一个布尔值，指示此对象是否是第一响应者。
```c++
@property(nonatomic, readonly) BOOL isFirstResponder;
```
&emsp;Return Value: 如果 receiver 是第一响应者，则为 YES；否则，则为 NO。

&emsp;UIKit 最初将一些类型的事件（例如运动事件 UIEventTypeMotion）调度给第一响应者。
#### canBecomeFirstResponder
&emsp;返回一个布尔值，指示此对象是否可以成为第一响应者。
```c++
@property(nonatomic, readonly) BOOL canBecomeFirstResponder; // default is NO
```
&emsp;Return Value: 如果 receiver 可以成为第一响应者，则为 YES，否则为 NO。

&emsp;默认情况下，此方法返回 NO。子类必须重写此方法并返回 YES 才能成为第一响应者。不要在当前不在活动视图层次结构中的 view 上调用此方法。结果是不确定的。
#### becomeFirstResponder
&emsp;要求 UIKit 使该对象成为其 window 中的第一响应者。
```c++
- (BOOL)becomeFirstResponder;
```
&emsp;Return Value: 如果此对象现在是第一响应者，则为 YES；否则，则为 NO。

&emsp;如果希望当前对象成为第一响应者，请调用此方法。调用此方法并不能保证对象将成为第一个响应者。UIKit 要求当前的第一响应者辞去第一响应者的职务，但它可能不会。如果是这样，UIKit 将调用该对象的 canBecomeFirstResponder 方法，默认情况下该方法返回 NO。如果此对象成功成为第一响应者，则以第一响应者为目标的后续事件将首先传递到此对象，UIKit 将尝试显示对象的 input view（如果有）。

&emsp;切勿对不属于活动视图层次结构的 view 调用此方法。你可以通过检查其 window 属性来确定 view 是否在屏幕上。如果该属性包含有效 window，则它是活动视图层次结构的一部分。如果该属性为 nil，则该 view 不是有效视图层次结构的一部分。

&emsp;你可以在自定义响应程序中重写此方法，以更新对象的状态或执行某些操作，例如突出显示所选内容。如果重写此方法，则必须在实现中的某个点调用super。
#### canResignFirstResponder
&emsp;返回一个布尔值，指示 receiver 是否愿意放弃其第一响应者的身份。
```c++
@property(nonatomic, readonly) BOOL canResignFirstResponder; // default is YES
```
&emsp;Return Value: 如果 receiver 可以放弃其第一响应者的身份，则为 YES，否则为 NO。

&emsp;默认情况下，此方法返回 YES。你可以在自定义响应程序中重写此方法，并在需要时返回其他值。例如，包含无效内容的 text field（UITextField）可能想要返回 NO，以确保用户首先更正该内容。
#### resignFirstResponder
&emsp;通知此对象已被要求在其 window 中放弃其作为第一响应者的身份。
```c++
- (BOOL)resignFirstResponder;
```
&emsp;默认实现返回 YES，即放弃第一响应者身份。你可以在自定义响应程序中重写此方法，以更新对象的状态或执行其他操作，例如从选择中删除突出显示。你也可以返回 NO，拒绝放弃第一响应者身份。如果重写此方法，则必须在代码中的某个点调用 super（超类实现）。（想起 UITextField 调用 resignFirstResponder 函数，隐藏系统键盘，大概是放弃第一响应者身份时同时隐藏其关联的 input view）
### Responding to Touch Events（响应触摸事件）
&emsp;通常，所有执行自定义触摸处理的响应者都应重写所有这四种方法。对于每个正在处理的 touch（你在 touchesBegan:withEvent: 中收到的 touch），你的响应者将收到 touchesEnded:withEvent: 或 touchesCancelled:withEvent:。你必须处理已取消的触摸（touchesCancelled:withEvent:），以确保应用程序中的正确行为。否则很可能导致不正确的行为或崩溃。
#### touchesBegan:withEvent:
&emsp;告诉该对象 view 或 window 中发生了一个或多个 new touches。
```c++
- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(nullable UIEvent *)event;
```
&emsp;`touches`: 一组 UITouch 实例，表示事件开始阶段的 touch，由事件表示。对于 view 中的 touch，默认情况下，此集合仅包含一个 touch。要接收多次 touch，必须将 view 的 multipleTouchEnabled 属性设置为 YES。`event`: touchs 所属的事件。

&emsp;当在 view 或 window 中检测到 new touch 时，UIKit 会调用此方法。许多 UIKit 类重写此方法并使用它来处理相应的 touch event。此方法的默认实现将消息转发到响应者链上。在创建自己的子类时，调用 super 来转发你自己没有处理的任何事件。例如，
```c++
[super touchesBegan:touches withEvent:event];
```
&emsp;如果你在不调用 super 的情况下重写此方法（一种常用模式），那么你还必须重写处理 touch event 的其他方法，即使你的实现什么都不做。
#### touchesMoved:withEvent:
&emsp;当与事件相关联的一个或多个 touches 发生更改时，通知响应者。
```c++
- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(nullable UIEvent *)event;
```
&emsp;`touches`: 一组 UITouch 实例，表示其值已更改的 touch。这些 touch 都属于指定的事件。对于 view 中的 touch，默认情况下，此集合仅包含一个 touch。要接收多个 touch，必须将 view 的 multipleTouchEnabled 属性设置为 YES。`event`: touchs 所属的事件。

&emsp;当 touch 的 location 或 force 发生变化时，UIKit 调用此方法。许多 UIKit 类重写此方法并使用它来处理相应的 touch event。此方法的默认实现将消息转发到响应者链上。在创建自己的子类时，调用 super 来转发你自己没有处理的任何事件。例如，
```c++
[super touchesMoved:touches withEvent:event];
```
&emsp;如果你在不调用 super 的情况下重写此方法（一种常用模式），那么你还必须重写处理 touch event 的其他方法，即使你的实现什么都不做。
#### touchesEnded:withEvent:
&emsp;当一个或多个手指从 view 或 window 抬起时，通知响应者。
```c++
- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(nullable UIEvent *)event;
```
&emsp;`touches`: 一组 UITouch 实例，表示由 event 表示的事件的结束阶段的 touch。对于视图中的触摸，默认情况下，此集合仅包含一个 touch。要接收多个 touch，必须将 view 的 multipleTouchEnabled 属性设置为 YES。`event`: touchs 所属的事件。

&emsp;当手指或 Apple Pencil 不再接触屏幕时，UIKit 调用此方法。许多 UIKit 类重写此方法，并使用它来清除处理相应触摸事件所涉及的状态。此方法的默认实现将消息转发到响应者链上。在创建自己的子类时，调用 super 来转发你自己没有处理的任何事件。例如，
```c++
[super touchesEnded:touches withEvent:event];
```
&emsp;如果你在不调用 super 的情况下重写此方法（一种常用模式），那么你还必须重写处理 touch event 的其他方法，即使你的实现什么都不做。
#### touchesCancelled:withEvent:
&emsp;当系统事件取消触摸序列时通知响应者（例如系统警报、手机来电）。
```c++
- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(nullable UIEvent *)event;
```
&emsp;`touches`: 一组 UITouch 实例，表示由 event 表示的事件的结束阶段的 touch。对于视图中的触摸，默认情况下，此集合仅包含一个 touch。要接收多个 touch，必须将 view 的 multipleTouchEnabled 属性设置为 YES。`event`: touchs 所属的事件。

&emsp;UIKit 在接收到需要取消触摸序列的系统中断时调用此方法。中断是指导致应用程序处于非活动状态或导致处理触摸事件的视图从其窗口中移除的任何内容。此方法的实现应该清除与处理触摸序列相关的任何状态。此方法的默认实现将消息转发到响应者链上。在创建自己的子类时，调用 super 来转发你自己没有处理的任何事件。例如，
```c++
[super touchesCancelled:touches withEvent:event];
```
&emsp;如果在不调用 super 的情况下重写此方法（一种常用模式），则还必须重写处理 touch event 的其他方法（如果仅作为 stub（empty）实现）。
#### touchesEstimatedPropertiesUpdated:
&emsp;告诉响应者已收到先前估计的属性的更新值，或者不再期望进行更新。
```c++
- (void)touchesEstimatedPropertiesUpdated:(NSSet<UITouch *> *)touches API_AVAILABLE(ios(9.1));
```
&emsp;`touches`: 包含更新属性的 UITouch 对象数组。在每个触摸对象中，UIKit 通过删除每个已更新属性的位标志来更新 estimatedPropertiesExpectingUpdates 属性。

&emsp;当 UIKit 无法报告触摸的实际值时，它会传递值的估计值，并在 UITouch 对象的 estimatedProperties 和 estimatedPropertiesExpectingUpdates 属性中设置适当的位。当接收到 estimatedPropertiesExpectingUpdate 属性中项目的更新时，UIKit 调用此方法来传递这些更新。如果不再需要一个或多个更新，UIKit 也会调用此方法。使用此方法可以使用 UIKit 提供的新值更新应用程序的内部数据结构。

&emsp;在实现此方法时，请在 touchs 参数中使用 UITouch 对象的 estimationUpdateIndex 属性来定位应用程序中的原始数据。定位数据后，将来自触摸对象的新值应用于该数据。你可以通过检查触摸对象的 estimatedPropertiesExpectingUpdates 位掩码来确定更新了哪些触摸属性；位掩码中不再包括更新的属性。

&emsp;由于硬件方面的考虑，与触摸相关的属性可能仍然是估计的。例如，当 Apple Pencil 靠近屏幕边缘时，传感器可能无法确定它的确切高度或方位角。在这些情况下，estimatedProperties 属性继续存储其值仅为估计值的属性列表。
### Responding to Motion Events（响应运动事件）
#### motionBegan:withEvent:
&emsp;告诉 receiver 运动事件已经开始。
```c++
- (void)motionBegan:(UIEventSubtype)motion withEvent:(nullable UIEvent *)event API_AVAILABLE(ios(3.0));
```
&emsp;`motion`: 一个事件子类型常量，指示运动的类型。常见的运动是晃动，由 UIEventSubtypeMotionShake 指示。`event`: 表示与运动关联的 UIEvent 的对象。

&emsp;UIKit 仅在运动事件开始和结束时通知响应者。它不报告中间晃动（UIEventSubtypeMotionShake）。运动事件最初被传递给第一响应者，并根据需要被转发到响应者链上。此方法的默认实现将消息转发到响应者链。
#### motionEnded:withEvent:
&emsp;告诉 receiver 运动事件已经结束。
```c++
- (void)motionEnded:(UIEventSubtype)motion withEvent:(nullable UIEvent *)event API_AVAILABLE(ios(3.0));
```
&emsp;`motion`: 一个事件子类型常量，指示运动的类型。常见的运动是晃动，由 UIEventSubtypeMotionShake 指示。`event`: 表示与运动关联的 UIEvent 的对象。

&emsp;UIKit 仅在运动事件开始和结束时通知响应者。它不报告中间晃动（UIEventSubtypeMotionShake）。运动事件最初被传递给第一响应者，并根据需要被转发到响应者链上。此方法的默认实现将消息转发到响应者链。
#### motionCancelled:withEvent:
&emsp;告诉 receiver 运动事件已被取消。
```c++
- (void)motionCancelled:(UIEventSubtype)motion withEvent:(nullable UIEvent *)event API_AVAILABLE(ios(3.0));
```
&emsp;`motion`: 一个事件子类型常量，指示运动的类型。常见的运动是晃动，由 UIEventSubtypeMotionShake 指示。`event`: 表示与运动关联的 UIEvent 的对象。

&emsp;UIKit 在接收到需要取消运动事件的中断时调用此方法。中断是指导致应用程序处于非活动状态或导致处理运动事件的 view 从其 window 中移除的任何内容。如果 shaking 持续时间过长，UIKit 也可能调用此方法。所有处理运动事件的响应者都应该实现此方法。在实现中，清除与处理运动事件相关的所有状态信息。（即运动事件被打断时，我们需要做必要的收尾工作）

&emsp;此方法的默认实现将消息转发到响应者链。
### Responding to Press Events（响应 Press 事件）
&emsp;通常，所有执行自定义 press 处理的响应者都应重写所有这四种方法。

&emsp;对于每个正在处理的 press，你的响应者将收到 pressesEnded:withEvent: 或 pressesCancelled:withEvent:（在pressesBegan：withEvent：中收到的印刷机）。pressesChanged:withEvent: 将被提供模拟值的 presses 调用（如指尖或模拟按钮）。

&emsp;你必须处理取消的 presses，以确保应用程序中的正确行为。否则很可能导致不正确的行为或崩溃。
#### pressesBegan:withEvent:
&emsp;第一次按下物理按钮时告诉该对象。
```c++
- (void)pressesBegan:(NSSet<UIPress *> *)presses withEvent:(nullable UIPressesEvent *)event API_AVAILABLE(ios(9.0));
```
&emsp;`presses`: 一组 UIPress 实例，代表发生的 new presses。每次按下的阶段都设置为 UIPressPhaseBegan。`event`: presses 所属的事件。

&emsp;用户按下新按钮时，UIKit 会调用此方法。使用此方法可以确定按下了哪个按钮并采取了所需的操作。

&emsp;此方法的默认实现将消息转发到响应者链。创建自己的子类时，请调用 super 来转发你自己无法处理的所有事件。
#### pressesChanged:withEvent:
&emsp;当与 press 关联的值发生更改时告诉该对象。
```c++
- (void)pressesChanged:(NSSet<UIPress *> *)presses withEvent:(nullable UIPressesEvent *)event API_AVAILABLE(ios(9.0));
```
&emsp;`presses`: 一组包含更改值的 UIPress 实例。`event`: presses 所属的事件。

&emsp;当与按钮或指尖关联的模拟值更改时，UIKit 会调用此方法。例如，当按钮的模拟力值改变时，它将调用此方法。使用此方法可以采取任何必要的措施来响应更改。

&emsp;此方法的默认实现将消息转发到响应者链。创建自己的子类时，请调用 super 来转发你自己无法处理的所有事件。
#### pressesEnded:withEvent:
&emsp;告诉对象何时释放按钮。
```c++
- (void)pressesEnded:(NSSet<UIPress *> *)presses withEvent:(nullable UIPressesEvent *)event API_AVAILABLE(ios(9.0));
```
&emsp;`presses`: 一组 UIPress 实例，代表用户不再按下的按钮。每次按下的阶段都设置为 UIPressPhaseEnded。`event`: presses 所属的事件。

&emsp;当用户停止按一个或多个按钮时，UIKit 调用此方法。使用此方法可以采取任何必要的操作来响应 press 的结束。此方法的默认实现将消息转发到响应者链上。在创建自己的子类时，调用 super 来转发你自己没有处理的任何事件。
#### pressesCancelled:withEvent:
&emsp;当系统事件取消按下事件时告诉该对象（例如内存不足警告）。
```c++
- (void)pressesCancelled:(NSSet<UIPress *> *)presses withEvent:(nullable UIPressesEvent *)event API_AVAILABLE(ios(9.0));
```
&emsp;`presses`: 一组 UIPress 实例，代表与事件关联的 presses。每次 press 的阶段都设置为 UIPressPhaseCancelled。`event`: presses 所属的事件。

&emsp;当 UIKit 接收到需要取消按下顺序的系统中断时，它调用此方法。中断是指导致应用程序处于非活动状态或导致处理 press 事件的 view 从其 window 中删除的任何内容。此方法的实现应该清除与处理 press 序列相关的任何状态。未能处理取消可能会导致不正确的行为或崩溃。

&emsp;此方法的默认实现将消息转发到响应者链。创建自己的子类时，请调用 super 来转发你自己无法处理的所有事件。
### Responding to Remote-Control Events（响应远程控制事件）
&emsp;在接收到远程控制事件时通知对象。
```c++
- (void)remoteControlReceivedWithEvent:(nullable UIEvent *)event API_AVAILABLE(ios(4.0));
```
&emsp;`event`: 封装了远程控制命令的事件对象。远程控制事件具有 UIEventTypeRemoteControl 的类型。

&emsp;远程控制事件源于来自外部附件（包括耳机）的命令。应用程序通过控制呈现给用户的音频或视频媒体来响应这些命令。接收响应程序对象应该检查事件的子类型，以确定预期的命令（例如，play（UIEventSubtypeRemoteControlPlay）），然后相应地继续。

&emsp;要允许传递远程控制事件，必须调用 UIApplication 的 beginReceivingRemoteControlEvents 方法。要关闭远程控制事件的传递，请调用 endReceivingRemoteControlEvents 方法。
### Managing Input Views（管理输入视图）
#### inputView
&emsp;当 receiver 成为第一响应者时显示的自定义输入视图。
```c++
@property (nullable, nonatomic, readonly, strong) __kindof UIView *inputView API_AVAILABLE(ios(3.2));
```
&emsp;此属性通常用于提供一个视图，以替换为 UITextField 和 UITextView 对象显示的系统提供的键盘。

&emsp;此只读属性的值为 nil。需要自定义视图来收集用户输入的响应者对象应将此属性重新声明为可读写，并使用它来管理其自定义输入视图。当 receiver 成为第一响应者时，响应者基础结构会自动显示指定的输入视图。类似地，当 receiver 退出其第一响应者状态时，响应者基础结构会自动关闭指定的输入视图。
#### inputViewController
&emsp;receiver 成为第一响应者时要使用的 custom input view controller。
```c++
@property (nullable, nonatomic, readonly, strong) UIInputViewController *inputViewController API_AVAILABLE(ios(8.0));
```
&emsp;此属性通常用于提供视图控制器，以替换为 UITextField 和 UITextView 对象显示的系统提供的键盘。

&emsp;此只读属性的值为 nil。如果要提供自定义输入视图控制器以替换应用程序中的系统键盘，请在 UIResponder 子类中将此属性重新声明为读写。然后可以使用此属性管理自定义输入视图控制器。当接收者成为第一个响应者时，响应者基础结构自动呈现指定的输入视图控制器。类似地，当接收器放弃其第一响应程序状态时，响应程序基础结构会自动取消指定的输入视图控制器。
#### inputAccessoryView
&emsp;当 receiver 成为第一响应者时显示的自定义输入附件视图（custom input accessory view）。
```c++
@property (nullable, nonatomic, readonly, strong) __kindof UIView *inputAccessoryView API_AVAILABLE(ios(3.2));
```
&emsp;此属性通常用于将附件视图附加到为 UITextField 和 UITextView 对象显示的系统提供的键盘上。(位置是在键盘的顶部)

&emsp;此只读属性的值为 nil。如果要将自定义控件附加到系统提供的输入视图（如系统键盘）或自定义输入视图（在 inputView 属性中提供的视图），请在 UIResponder 子类中将此属性重新声明为读写。然后可以使用此属性管理自定义附件视图。当 receiver 成为第一个响应者时，响应者基础结构会在显示附件视图之前将其附加到适当的输入视图。
#### inputAccessoryViewController
&emsp;自定义输入附件视图控制器，以在 receiver 成为第一响应者时显示。
```c++
@property (nullable, nonatomic, readonly, strong) UIInputViewController *inputAccessoryViewController API_AVAILABLE(ios(8.0));
```
#### reloadInputViews
&emsp;当对象是第一响应者时，更新自定义输入和附件视图（custom input and accessory views）。（如果在对象是第一响应者的情况下调用，则重新加载 inputView、inputAccessoryView 和 textInputMode。否则忽略。）
```c++
- (void)reloadInputViews API_AVAILABLE(ios(3.2));
```
&emsp;当它是第一响应者时，可以使用此方法刷新与当前对象关联的自定义输入视图（custom input view）或输入附件视图（ input accessory view）。视图将立即替换（即不会附加动画）。如果当前对象不是第一响应者，则此方法无效。
### Getting the Undo Manager（）
#### undoManager
&emsp;返回响应者链中最近的共享撤消（shared undo）管理器。
```c++
@property(nullable, nonatomic,readonly) NSUndoManager *undoManager API_AVAILABLE(ios(3.0));
```
&emsp;默认情况下，应用程序的每个 window 都有一个撤消管理器：一个用于管理撤消和重做操作的共享对象。但是，响应程序链中任何对象的类都可以有自己的自定义撤消管理器。（例如，UITextField的实例有自己的 undoManager，当文本字段退出第一响应者状态时，该管理器将被清除。）当你请求 undoManager 时，请求将进入响应者链，UIWindow 对象将返回一个可用的实例。

&emsp;你可以将撤消管理器添加到视图控制器，以执行托管视图本地的撤消和重做操作。
...

&emsp;UIResponder 的文档就看到这吧，我们最需要记住的就是当我们需要自己处理事件时需要重写 touches...（响应触摸事件）、presses...（响应按键事件）、motion...（响应运动事件） 系列函数，以及 nextResponder 属性，是它链接了响应者链。

&emsp;下面我们阅读 Responder object 文档，虽然它被标记为过时，但是还是有一定的参考意义。
## Responder object
&emsp;响应者是可以响应事件并处理它们的对象。所有响应者对象都是最终从 UIResponder（iOS）或 NSResponder（OS X）继承的类的实例。这些类声明了一个用于事件处理的编程接口，并定义了响应者的默认行为。应用程序的可见对象几乎总是响应者（例如，windows、views 和 controls），而应用程序对象（AppDelegate）也是响应者。在 iOS 中，视图控制器（UIViewController 对象）也是响应者对象。

![responder](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/118b9dd7a273495c9d21b66dbf8020e5~tplv-k3u1fbpfcp-watermark.image)

&emsp;若要接收事件，响应者必须实现适当的事件处理方法，在某些情况下，告诉应用它可以成为第一响应者。
### The First Responder Receives Some Events First（第一响应者首先接收一些事件）
&emsp;在应用程序中，首先接收多种事件的响应者对象称为第一响应者。它接收关键事件、运动事件和 action 消息等。（鼠标事件和多点触控事件首先转到鼠标指针或手指下的视图；该视图可能是也可能不是第一响应者。）第一响应者通常是应用程序认为最适合处理事件的窗口中的视图。为了接收事件，响应者还必须表明其成为第一响应者的意愿；对于每个平台，响应者以不同的方式进行：
```c++
// OS X
- (BOOL)acceptsFirstResponder { 
    return YES; 
}
 
// iOS
- (BOOL)canBecomeFirstResponder {
    return YES; 
}
```
&emsp;除了接收事件消息外，响应者还可以接收未指定 target 的 action 消息。（action 消息由 buttons 和 controls 等控件在用户操作时发送。）
### The Responder Chain Enables Cooperative Event Handling（响应者链支持协作事件处理）
&emsp;如果第一个响应者无法处理事件或 action 消息，它会将其转发给一个称为响应者链的链接系列中的 “下一个响应者” （next responder）。响应者链允许响应者对象将处理事件或 action 消息的责任转移到应用程序中的其他对象。如果响应者链中的对象无法处理事件或 action，它会将消息传递给链中的下一个响应者。消息沿着链向上传播，指向更高级别的对象，直到被处理为止。如果未处理，应用程序将丢弃它。

&emsp;The responder chain for iOS (left) and OS X (right)

![iOS_and_OSX_responder_chain](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/a21b84f206d14ebebd1e9cbd83ea57af~tplv-k3u1fbpfcp-watermark.image)

&emsp;The path of an event。事件在响应者链上的一般路径从第一个响应者的视图或鼠标指针或手指下的视图开始。从那里开始，它向上进入视图层次结构，进入 window 对象，然后进入全局应用程序对象。但是，iOS 中事件的响应者链为该路径添加了一个变体：如果 view 由 view controller 管理，并且 view 无法处理事件，则 view controller 将成为下一个响应者。

&emsp;The path of an action message。对于 action 消息，OS X 和 iOS 都将响应者链扩展到其他对象。在 OS X 中，对于基于文档体系结构的应用程序、使用窗口控制器的应用程序（NSWindowController）和不适合这两个类别的应用程序，action 消息的响应者链是不同的。此外，如果 OS X 上的应用程序同时具有一个 key window 和一个 main window，那么 action 消息所经过的响应者链可能涉及两个窗口的视图层次结构。

&emsp;接着下面是 Handling Touches in Your View（处理视图中的触摸）文档。
## Handling Touches in Your View（处理视图中的触摸）
&emsp;如果触摸处理（touch handling）与 view 的内容有复杂的链接（intricately linked），则直接在 view 子类上使用触摸事件（touch events）。

&emsp;如果你不打算对自定义视图使用手势识别器，则可以直接从视图本身处理触摸事件。因为 view 是响应者，所以它们可以处理多点触控事件和许多其他类型的事件。当 UIKit 确定某个 view 中发生了触摸事件时，它将调用该 view 的 `touchesBegan:withEvent:`、`touchesMoved:withEvent:` 或 `touchesEnded:withEvent:` 方法。你可以在自定义视图中重写这些方法，并使用它们提供对触摸事件的响应。（来自 UIResponder）

&emsp;你在视图（或任何响应者）中重写的处理触摸的方法对应于触摸事件处理过程的不同阶段。例如，图1 说明了触摸事件的不同阶段。当手指（或 Apple Pencil）接触屏幕时，UIKit 会创建 UITouch 对象，将触摸位置设置为适当的点，并将其 phase 属性设置为 UITouchPhaseBegan。当同一个手指在屏幕上移动时，UIKit 会更新触摸位置，并将触摸对象的 phase 属性更改为 UITouchPhaseMoved。当用户将手指从屏幕上提起时，UIKit 将 phase 属性更改为 UITouchPhaseEnded，触摸序列结束。

&emsp;Figure 1 The phases of a touch event（触摸事件的各个阶段）
![phases_of_a_touch_event](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/e4b571cc659149239045a7bd9c8e0355~tplv-k3u1fbpfcp-watermark.image)

&emsp;类似地，系统可以随时取消正在进行的触摸序列；例如，当来电中断应用程序时。当它这样做时，UIKit 通过调用 touchs 来通知你的 view 的 `touchesCancelled:withEvent:` 方法。你可以使用该方法对 view 的数据结构执行任何必要的清理。

&emsp;UIKit 为触摸屏幕的每个新手指创建一个新的 UITouch 对象。触摸本身是通过当前 UIEvent 对象传递的。UIKit 区分了来自手指和 Apple Pencil 的触摸，你可以对它们进行不同的处理。

> &emsp;Important: 在其默认配置中，view 仅接收与事件关联的第一个 UITouch 对象，即使有多个手指接触 view 也是如此。要接收额外的触摸，必须将 view 的 multipleTouchEnabled 属性设置为 true。也可以使用属性检查器在 Interface Builder 中配置此属性。

&emsp;接着下面是 Using Responders and the Responder Chain to Handle Events（使用响应者和响应者链来处理事件）文档。
## Using Responders and the Responder Chain to Handle Events（使用响应者和响应者链来处理事件）
&emsp;了解如何处理通过你的应用传播的事件。

&emsp;应用程序使用响应者对象（responder objects）接收和处理事件。responder 对象是 UIResponder 类的任何实例，常见的子类包括 UIView、UIViewController 和 UIApplication。响应者接收原始事件数据，并且必须处理该事件或将其转发给另一个响应者对象。当应用程序接收到事件时，UIKit 会自动将该事件定向到最合适的响应者对象（称为第一响应者）。

&emsp;未处理的事件在活动响应者链中从一个响应者传递到另一个响应者，这是应用程序响应者对象的动态配置。Figure 1 显示了应用程序中的响应者，其界面包含一个 label、一个 text field、一个 button 和两个背景 views。该图还显示了事件如何沿着响应者链从一个响应者移动到下一个响应者。

&emsp;Figure 1 Responder chains in an app（应用中的响应者链）
![responder_chains_in_an_app](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/7ed8b25e72b84091936355631dfa39a4~tplv-k3u1fbpfcp-watermark.image)

&emsp;如果 text field 不处理事件，UIKit 会将事件发送到 text field 的父 view 对象，后跟 window 的根视图。在将事件定向到 window 之前，响应者链从根视图转移到拥有的视图控制器。如果 window 无法处理事件，UIKit 会将事件传递给 UIApplication 对象，如果该对象是 UIResponder 的实例而不是响应程序链的一部分，则可能会传递给 app delegate。
### Determining an Event's First Responder（确定事件的第一响应者）
&emsp;UIKit 根据事件的类型将对象指定为事件的第一响应者。事件类型包括：
| Event type | First responder |
| --- | --- |
| Touch events | The view in which the touch occurred.（发生触摸的视图） |
| Press events | The object that has focus. |
| Shake-motion events | The object that you (or UIKit) designate. |
| Remote-control events | The object that you (or UIKit) designate. |
| Editing menu messages | The object that you (or UIKit) designate. |

> &emsp;Note: 与加速计（accelerometers）、陀螺仪（gyroscopes）和磁力计（magnetometer）相关的运动事件不遵循响应链。相反，Core Motion 将这些事件直接传递给指定的对象。有关详细信息，请参见 Core Motion Framework

&emsp;Controls 使用 action 消息直接与其关联的 target 对象通信。当用户与 control 交互时，control 将向其 target 对象发送 action 消息。Action messages 不是事件，但它们仍然可以利用响应者链。当 control 的 target 对象为 nil 时，UIKit 从目标对象开始遍历响应者链，直到找到实现适当 action 方法的对象。例如，UIKit 编辑菜单（editing menu）使用此行为来搜索响应者对象，这些对象实现了名称为 cut:、copy: 或 paste: 的方法。

&emsp;Gesture recognizers 在 view 之前接收 touch 和 press 事件。如果 view 的 gesture recognizers 无法识别一系列 touches，UIKit 会将 touches 发送到 view。如果 view 不能处理 touches，UIKit 会将它们向上传递到响应者链。有关使用 gesture recognizer 处理事件的详细信息，请参阅 Handling UIKit Gestures（下面有其翻译）。
### Determining Which Responder Contained a Touch Event（确定哪个响应者包含触摸事件）
&emsp;UIKit 使用基于 view 的 Hit-Testing（view-based hit-testing）来确定触摸事件发生的位置。具体来说，UIKit 将触摸位置与 view 层次结构中 view 对象的 bounds 进行比较。UIView 的 hitTest:withEvent: 方法遍历视图层次结构，查找包含指定触摸的最深子视图，该子视图将成为触摸事件的第一响应者。（会直接把 UIEvent 交给它处理）

> &emsp;Note: 如果触摸位置在视图 bounds 之外，则 hitTest:withEvent: 方法忽略该视图及其所有子视图。因此，当视图的 clipsToBounds 属性为 NO 时，即使超出该视图 bounds 的子视图恰好包含触摸，也不会返回这些子视图。有关 Hit-Testing 行为的更多信息，请参阅 UIView 的 hitTest:withEvent: 方法（上面我们已经详细分析过了）。

&emsp;当触摸发生时，UIKit 创建一个 UITouch 对象并将其与 view 相关联。当触摸位置或其他参数改变时，UIKit 用新信息更新同一 UITouch 对象。唯一不变的属性是 view。（即使触摸位置移动到原始 view 之外，触摸 view 属性中的值也不会更改。（如我们常见的页面上有一个小的滚动区域时，我们手指先摸到它并滑动该小区域开始滚动，当我们的手指超出此块滚动区域并不离开屏幕时，此小滚动区域也一直响应我们手指的滑动））当触摸结束时，UIKit 释放 UITouch 对象。
### Altering the Responder Chain（改变响应者链）
&emsp;你可以通过重写响应者对象的 nextResponder 属性来更改响应者链。当你这样做时，下一个响应者就是你返回的对象。

&emsp;许多 UIKit 类已经重写此属性并返回特定的对象，包括：
+ UIView 对象。如果 view 是 view controller 的 root view，则下一个响应者是 view controller；否则，下一个响应者是 view 的 superview。
+ UIViewController 对象。
  + 如果 view controller 的 view 是 window 的 root view，则它的下一个响应者是 window 对象。
  + 如果 view controller 由另一个 view controller 呈现，则它的下一个响应者是呈现 view controller。（parent view controller）
+ UIWindow 对象。window 的下一个响应者是 UIApplication 对象。
+ UIApplication 对象。下一个响应者是 app delegate，但仅当该 app delegate 是 UIResponder 的实例并且不是 view、view controller 或 app  对象本身时，才是下一个响应者。

&emsp;事件处理、响应者以及响应者链的相关内容到这里就可以结束了，下面我们再对 gesture recognizers（继承自 NSObject，响应事件的方式同 UIControl，也是 target-action 机制） 和 target-action 进行一个拓展学习，它们还挺重要的。（当一个 view 同时实现了 touches... 系列函数和添加了手势时，我们去触摸该 view 首先会调用 touchesBegan:withEvent: 函数，而当 gesture recognizers 识别出手势后会调用 view 的 touchesCancelled:withEvent: 打断 touches... 系列函数的执行，然后去执行手势的 action 函数。）
## Handling UIKit Gestures（处理 UIKit 手势）
&emsp;使用 gesture recognizers 简化 touch handling 并创建一致的用户体验。

&emsp;Gesture recognizers 是处理视图中的 touch（UIEventTypeTouches 屏幕上的触摸事件） 或 press （UIEventTypePresses 设备的物理按钮）事件的最简单方法。可以将一个或多个 gesture recognizers 附加到任何视图。Gesture recognizers 封装了为该视图处理和解释传入事件所需的所有逻辑，并将它们与已知模式（手势类型）相匹配。当检测到匹配时，gesture recognizer 会通知其指定的目标对象，该目标对象可以是 view controller、view 本身或应用程序中的任何其他对象。

&emsp;Gesture recognizers 使用目标动作设计模式（target-action design pattern）发送通知。 当 UITapGestureRecognizer 对象在视图中检测到单个手指点击时，它将调用 view 的 view controller 的操作方法，你可以使用该方法提供响应。

&emsp;Figure 1 Gesture recognizer notifying its target（手势识别器通知目标）
![gesture_recognizer_notifying_its_target](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/2be0b127274a4ae9850b843b1a3ee832~tplv-k3u1fbpfcp-watermark.image)

&emsp;Gesture recognizers 有两种类型：离散（discrete）和连续（continuous）。一个离散的手势识别器（discrete gesture recognizer）会在手势被识别后准确地调用你的动作方法（action method）一次。满足初始识别条件后，连续手势识别器（continuous gesture recognizer）会多次执行对动作方法的调用，并在手势事件中的信息发生更改时通知你。例如，每次触摸位置更改时，UIPanGestureRecognizer 对象都会调用动作方法。

&emsp;Interface Builder 包含每个标准 UIKit gesture recognizers 的对象。它还包括一个自定义手势识别器对象，你可以使用它来表示自定义 UIGestureRecognizer 子类。
### Configuring a gesture recognizer（配置手势识别器）
&emsp;要配置手势识别器：
1. 在 storyboard 中，将手势识别器拖到视图中。
2. 实现识别手势时要调用的动作方法；参见  Listing 1。
3. 将你的动作方法连接到手势识别器。

&emsp;通过右键单击手势识别器并将其 Sent Action selector 连接到界面中的相应对象，可以在 Interface Builder 中创建此连接。你还可以使用手势识别器的 addTarget(_:action:) 方法以编程方式配置 action 方法。

&emsp;Listing 1 显示了手势识别器的 action 方法的通用格式。如果愿意，可以更改参数类型以匹配特定的手势识别器子类。

&emsp;Listing 1 Gesture recognizer action methods（手势识别器 action 方法）
```c++
// Swift
@IBAction func myActionMethod(_ sender: UIGestureRecognizer) { ... }
// Objective-c
- (IBAction)myActionMethod:(UITapGestureRecognizer *)sender { ... }
```
### Responding to Gestures（响应手势）
&emsp;与 gesture recognizer 关联的 action method 提供应用程序对该手势的响应。对于离散手势，你的 action method 类似于 button 的 action method。一旦调用了 action method，就可以执行适合该手势的任何任务。对于连续的手势，action method 可以响应对手势的识别，但也可以在识别手势之前跟踪事件。跟踪事件可以让你创建更具交互性的体验。例如，你可以使用 UIPanGestureRecognizer 对象的更新来重新定位应用程序中的内容。（如让一个 imageView 跟着你的手指移动位置）

&emsp;gesture recognizer 的 state 属性传递对象的当前识别状态。对于连续的手势，gesture recognizer 将从更新此属性的值 UIGestureRecognizer.State.began 至 UIGestureRecognizer.State.changed 至 UIGestureRecognizer.State.ended，或 UIGestureRecognizer.State.cancelled。action methods 使用此属性来确定适当的 action 过程。例如，可以使用 began 和 changed 状态对内容进行临时更改，使用 ended 状态使这些更改永久化，使用 cancelled 状态放弃更改。在执行操作之前，请始终检查 gesture recognizer 的 state 属性值。

&emsp;手势的文档就看到这里吧，文档里面还有各种类型手势的处理以及实现自定义手势、离散手势、连续手势的实现、手势的状态机等等内容，大家可以根据需要进行学习。

## Target-Action
&emsp;尽管 delegation、bindings 和 notification 对于处理程序中对象之间的某些形式的通信很有用，但它们并不特别适合于最明显的通信类型。典型的应用程序的用户界面由许多图形对象组成，其中最常见的对象可能是控件（controls）。控件是真实世界或逻辑设备（按钮（button）、滑块（slider）、复选框（checkboxes）等）的图形模拟；与真实世界控件（如收音机调谐器）一样，你使用它将你的意图传达给某个系统，而该系统是应用程序的一部分。

&emsp;控件（control）在用户界面上的作用很简单：它解释用户的意图并指示其他对象执行该请求。当用户通过单击控件或按返回键对控件进行操作时，硬件设备将生成一个原始事件。控件接受事件（为 Cocoa 适当包装），并将其转换为特定于应用程序的指令。然而，事件本身并不能提供很多关于用户意图的信息；它们只是告诉你用户单击了鼠标按钮或按下了一个键。因此，必须借助某种机制来提供事件与指令之间的转换。这种机制被称为 target-action。

&emsp;Cocoa 使用 target-action 机制在 control 和另一个对象（target）之间进行通信。这种机制允许 control 和 OS X 中的一个或多个单元格封装必要的信息，以便将特定于应用程序的指令发送到适当的对象。接收对象通常是自定义类的实例，称为 target。action 是 control 发送给 target 的消息。对用户事件感兴趣的对象（target）是赋予它意义的对象，这种意义通常反映在它赋予 action 的名称中。
### The Target
&emsp;target 是 action 消息的接收者。control（或更常见的是 cell）将其 action 消息的 target 作为 outlet 保存（请参见 Outlets）。target 通常是你的一个自定义类的实例，尽管它可以是其类实现适当 action 方法的任何 Cocoa 对象。

&emsp;还可以将 cell 或 control 的 target outlet 设置为 nil，并在运行时确定 target 对象。当 target 为 nil 时，应用程序对象（NSApplication 或 UIApplication）按指定的顺序搜索适当的 receiver：
1. 它从 key window 中的第一个响应者开始，然后跟随 nextResponder 链接将响应者链链接到 window 对象（NSWindow 或 UIWindow）的内容视图。

> &emsp;Note: OS X 中的 key window 响应于应用程序的按键（key presses）操作，并且是菜单和对话框中消息的接收者。应用程序的 main window 是用户 actions 的主要焦点，并且通常还具有关键状态。

2. 它先尝试 window 对象，然后再尝试 window 对象的 delegate。
3. 如果 main window 与 key window 不同，那么它将从 main window 中的第一响应者开始，并沿着 main window 的响应者链向上到达 window 对象及其 delegate。
4. 接下来，application 对象尝试响应。如果无法响应，它将尝试其 delegate。application 对象及其 delegate 是最后的 receiver。

&emsp;Control 不（也不应该）保留其 targets。但是，发送 action 消息的 controls 的 clients（通常是 applications）负责确保其 targets 可用于接收 action 消息。为此，它们可能必须将 targets 保留在内存管理的环境中。此预防措施同样适用于委托（delegates）和数据源（data sources）。
### The Action
&emsp;action 是 control 发送给 target 的消息，或者从 target 的角度来看，是 target 实现的响应 action 消息的方法。control 或（通常是 AppKit 中的情况）control 的 cell 将 action 存储为 SEL 类型的实例变量。SEL 是一种 Objective-C 数据类型，用于指定消息的签名。action 消息必须具有简单、独特的签名。它调用的方法不返回任何内容，通常只有一个 id 类型的参数。按照约定，此参数名为 sender。下面是 NSResponder 类中的一个示例，该类定义了许多 action 方法：
```c++
- (void)capitalizeWord:(id)sender;
```
&emsp;某些 Cocoa 类声明的 action 方法也可以具有等效的签名：
```c++
- (IBAction) deleteRecord:(id)sender;
```
&emsp;在这种情况下，IBAction 不会为返回值指定数据类型；不会返回值。IBAction 是一个类型限定符，Interface Builder 在应用程序开发过程中会注意到该类型限定符，将以编程方式添加的 action 与其为项目定义的 action 方法的内部列表同步。

&emsp;sender 参数通常标识发送 action 消息的 control（尽管它可以是另一个被实际发送方替换的对象）。这背后的想法类似于明信片上的回信地址。如果需要，target 可以向发送者查询更多信息。如果实际的发送对象将另一个对象替换为发送方，则应以相同的方式处理该对象。例如，假设你有一个 text field，并且当用户输入文本时，将在 target 中调用 action 方法 nameEntered :：
```c++
- (void)nameEntered:(id) sender {
    NSString *name = [sender stringValue];
    if (![name isEqualToString:@""]) {
        NSMutableArray *names = [self nameList];
        [names addObject:name];
        [sender setStringValue:@""];
    }
}
```
&emsp;在这里，响应方法提取文本字段的内容，将字符串添加到作为实例变量缓存的数组中，然后清除该字段。对发送方的其他可能查询包括：向 NSMatrix 对象询问其所选行（[sender selectedRow]）、向 NSButton 对象询问其状态（[sender state]）、并向与控件关联的任何单元格询问其标记（[[sender cell] tag]）标记是数字标识符。

&emsp;文档下面是 Target-Action in the AppKit Framework 和 Target-Action in UIKit，这里只看一下 Target-Action in UIKit。

### Target-Action in UIKit
&emsp;UIKit framework 还声明并实现一组 control 类；此框架中的 control 类继承自 UIControl 类，后者定义了 iOS 的大多数 target-action mechanism。然而，在 AppKit 和 UIKit 框架如何实现 target-action 方面存在一些根本性的差异。其中一个区别是 UIKit 没有任何真正的 cell 类。UIKit 中的 controls 不依赖其 cells 获取 target 和 action 信息。

&emsp;两个框架如何实现 target-action 的一个更大的区别在于事件模型的性质（nature of the event model）。在 AppKit 框架中，用户通常使用鼠标和键盘来注册事件以供系统处理。这些事件（如单击按钮）是有限和离散的。因此，AppKit 中的 control 对象通常会将单个物理事件识别为其发送到 target 的 action 的触发器。（在按钮的情况下，这是一个 mouse-up 的事件。）在 iOS 中，用户的手指是事件的始作俑者，而不是鼠标点击、鼠标拖动或物理按键。一次可以有多个手指触摸屏幕上的一个对象，这些触摸甚至可以朝不同的方向进行。

&emsp;为了说明此多点触控事件模型，UIKit 在 UIControl.h 中声明了一组 control-event 常量，这些常量指定用户可以在 control 上进行的各种物理手势，例如从控件中抬起手指、将手指拖动到控件中以及在文本字段中触碰。你可以配置 control 对象，以便它通过向 target 发送 action 消息来响应一个或多个这些触摸事件。UIKit 中的许多控件类实现为生成某些控件事件；例如，UISlider 类的实例生成 UIControlEventValueChanged 控件事件，你可以使用该事件向 target 对象发送 action 消息。

&emsp;设置 control，使其通过将 target 和 action 与一个或多个 control 事件关联，向 target 对象发送 action 消息。为此，发送 addTarget:action:forControlEvents: 消息到要指定 target-action pair 的控件。当用户以指定的方式触摸控件时，控件通过 sendAction:to:from:forEvent: 消息将 action 消息转发到全局 UIApplication 对象。与在 AppKit 中一样，全局 application 对象是 action 消息的集中调度点。如果控件为 action 消息指定了nil target，应用程序将查询响应程序链中的对象，直到找到一个愿意处理 action 消息的对象，即实现与 action 选择器对应的方法的对象。

&emsp;与 AppKit 框架（一种操作方法可能仅具有一个或两个有效签名）相比，UIKit 框架允许三种不同形式的 action selector：
```c++
- (void)action
- (void)action:(id)sender
- (void)action:(id)sender forEvent:(UIEvent *)event
```
&emsp;要了解有关 UIKit 中 target-action mechanism 的更多信息，请阅读 UIControl Class Reference。

## 参考链接
**参考链接:🔗**
+ [Using Responders and the Responder Chain to Handle Events](https://developer.apple.com/documentation/uikit/touches_presses_and_gestures/using_responders_and_the_responder_chain_to_handle_events)
+ [Handling Touches in Your View](https://developer.apple.com/documentation/uikit/touches_presses_and_gestures/handling_touches_in_your_view)
+ [Responder object](https://developer.apple.com/library/archive/documentation/General/Conceptual/Devpedia-CocoaApp/Responder.html#//apple_ref/doc/uid/TP40009071-CH1-SW1)
+ [Events (iOS)](https://developer.apple.com/library/archive/documentation/General/Conceptual/Devpedia-CocoaApp/EventHandlingiPhone.html#//apple_ref/doc/uid/TP40009071-CH13-SW1)
+ [Target-Action](https://developer.apple.com/library/archive/documentation/General/Conceptual/CocoaEncyclopedia/Target-Action/Target-Action.html#//apple_ref/doc/uid/TP40010810-CH12)
+ [细数iOS触摸事件流动](https://juejin.cn/post/6844904175415853064)
+ [iOS 响应者链与事件处理](https://www.xiaobotalk.com/2020/03/responder-chain/)
+ [iOS开发系列--触摸事件、手势识别、摇晃事件、耳机线控](https://www.cnblogs.com/kenshincui/p/3950646.html)
