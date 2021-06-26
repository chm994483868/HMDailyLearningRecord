# iOS 从源码解析Run Loop (二)：Run Loops 官方文档翻译 

&emsp;Run loops 是与 threads 关联的基本基础结构的一部分。Run loop 是一个 event processing loop （**事件处理循环**），可用于计划工作并协调收到的事件的接收。Run loop 的目的是让 thread 在有工作要做时保持忙碌，而在没有工作时让 thread 进入睡眠状态。

&emsp;Run loop 管理不是完全自动的。你仍然必须设计 thread 的代码以在适当的时候启动 run loop 并响应传入的事件。Cocoa 和 Core Foundation 都提供 run loop 类和结构（NSRunLoop 和 \__CFRunLoop），以帮助你配置和管理 thread 的 run loop。你的应用程序不需要显式创建这些对象；每个 thread，包括应用程序的 main thread，都有一个与之关联的 run loop 对象。但是，只有子线程需要显式地运行其 run loop。在应用程序启动过程中，应用程序框架会自动在 main thread 上设置并运行其 run loop。

&emsp;以下各节提供有关 run loop 以及如何为应用程序配置 run loop 的更多信息。

## Anatomy of a Run Loop（剖析 Run Loop）
&emsp;run loop 和它的名字听起来很像。它是 thread 进入并用于运行事件处理程序以响应传入事件的循环。你的代码提供了用于实现 run loop 的实际循环部分的控制语句—换句话说，你的代码提供了驱动 run loop 的 while 或 for 循环。在循环内部，你可以使用 run loop 对象来 "run"（运行） 事件处理代码，以接收事件并调用已安装的处理程序（installed handlers）。

&emsp;Run loop 从两种不同类型的 sources 来接收事件。 Input sources 传递异步事件，通常是来自另一个 thread 或其它应用程序的消息。Timer sources（计时器源 NSTimer）传递同步事件，这些事件在计划的时间点或重复的时间间隔发生。两种类型的 source  在事件到达时都使用特定于应用程序的处理程序例程（application-specific handler routine）来处理事件。（Input sources 和 Timer sources）

&emsp;图 3-1 显示了 run loop 和各种 sources 的概念结构。Input sources 将异步事件（asynchronous events）传递给相应的处理程序，并使 `runUntilDate:` 方法（在 thread 关联的 NSRunLoop 对象上调用）退出（这里是不是有问题：`runMode:beforeDate:` 是使 run loop 运行一次，在 run loop 处理了第一个 input source 后会退出，`runUntilDate:` 则是通过重复调用 `runMode:beforeDate:` 一直处理 input source 直到指定的到期日期后 run loop 退出）。Timer sources 将事件传递到其处理程序例程（handler routines），但不会导致 run loop 退出。

&emsp;图3-1 run loop 的结构及其 sources

![run loop 的结构及其 sources](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/708ef62547ec4f2bba83a95382d993d9~tplv-k3u1fbpfcp-watermark.image)

&emsp;除了处理 input sources 之外，run loop 还会生成有关 run loop behavior（行为）的通知。注册的运行循环观察者（run-loop observers）可以接收这些通知，并使用它们在 thread 上进行附加处理 。你可以使用 Core Foundation 在 thread 上安装运行循环观察者（install run-loop observers）。

&emsp;以下各节提供有关 run loop 的组件及其运行方式的更多信息。它们还描述了在处理事件期间的不同时间生成的通知。

### Run Loop Modes
&emsp;Run loop mode 是要监视的 input sources 和 timers 的集合，以及要通知的 run loop observers 的集合。每次运行 run loop 时，都要（显式或隐式）指定运行的特定 "mode"。在 run loop 的整个过程中，仅监视与该 mode 关联的 sources，并允许其传递事件。
在这个 run loop 的传递过程中（During that pass of the run loop），只监视与该 mode 关联的 sources，并允许它们传递事件。（类似地，只有与该 mode 相关联的 observers 才会收到 run loop 进度（loop’s progress）的通知。）与其它 mode 相关联的 sources 将保留任何新事件，直到随后以适当的 mode 通过循环。

&emsp;在代码中，你可以通过名称识别 modes。Cocoa 和 Core Foundation 都定义了默认模式和几种常用模式，以及用于在代码中指定这些 mode 的字符串。你可以通过简单地为 mode 名称指定自定义字符串来定义自定义 mode。尽管你分配给自​​定义 mode 的名称是任意的，但是这些 mode 的内容不是任意的。你必须确保将一个或多个 input sources，timers 或 run-loop observers 添加到你创建的任何 mode 中，以使其有用。

&emsp;你可以使用模式从运行循环的特定遍历中过滤掉有害来源的事件。大多数时候，你将需要在系统定义的“默认”模式下运行运行循环。但是，模式面板可以在“模式”模式下运行。在这种模式下，只有与模式面板相关的源才将事件传递给线程。对于辅助线程，你可以使用自定义模式来防止低优先级源在时间紧迫的操作期间传递事件。

&emsp;在 run loop 的特定传递过程中，可以使用 mode 从不需要的 sources 中筛选出事件。大多数情况下，你希望以系统定义的 "default" mode 运行 run loop。然而，modal panel 可能以 "modal" mode 运行，在这种 mode 下，只有与 modal panel 相关的 sources 才会向线程传递事件。对于子线程（secondary threads），可以使用自定义 mode 来防止低优先级源（low-priority sources）在时间关键型操作（time-critical operations）期间传递事件。

> Note: Modes 的区别基于事件的 source，而不是事件的类型（type of the event）。例如，你不会使用 modes 仅匹配鼠标按下事件或仅匹配键盘事件。你可以使用 modes 来监听不同的端口集（set of ports），暂时挂起 timers，或者更改当前正在监视的 sources 和 run loop observers。

&emsp;表 3-1 列出了 Cocoa 和 Core Foundation 定义的 standard modes，以及何时使用该 mode 的说明。 name 列列出了用于在代码中指定 mode 的实际常量。

&emsp;表 3-1 预定义的 run loop modes
| Mode | Name | Description |
| --- | --- | ---- |
| Default | NSDefaultRunLoopMode (Cocoa) kCFRunLoopDefaultMode (Core Foundation) | 默认模式是用于大多数操作的模式。大多数时候，你应该使用此模式启动 run loop 并配置 input sources。 |
| Connection | NSConnectionReplyMode (Cocoa) | Cocoa 将此模式与 NSConnection 对象结合使用来监视响应。你应该很少需要自己使用这种模式。 |
| Modal | NSModalPanelRunLoopMode (Cocoa) | Cocoa 使用此模式来识别用于 modal panels 的事件。 |
| Event tracking | NSEventTrackingRunLoopMode (Cocoa) | Cocoa 使用此模式在鼠标拖动循环和其他类型的用户界面跟踪循环期间限制传入事件。 |
| Common modes | NSRunLoopCommonModes (Cocoa) kCFRunLoopCommonModes (Core Foundation) | 这是一组可配置的常用模式。将 input source 与此模式关联也会将其与组中的每个模式相关联。对于 Cocoa 应用程序，默认情况下，此集合包括 default、modal、event tracking 模式。Core Foundation 最初只包括 default mode。你可以使用 CFRunLoopAddCommonMode 函数向集合添加自定义模式。 |

### Input Sources
&emsp;Input sources 将事件异步传递到你的线程。事件的 source 取决于 input source 的类型，通常是两个类别之一（Input source 和 Timer source 两种类型）。基于端口的输入源（port-based input sources）监视你的应用程序的 Mach 端口。定制输入源监视事件的定制源。就你的 run loop 而言，input source 是基于端口（port-based）的还是定制的（custom）都无关紧要。系统通常实现两种类型的输入源，你可以按原样使用。两种信号源之间的唯一区别是信号的发送方式。基于端口的源（port-based sources）由内核（kernel）自动发出信号，而自定义源（custom sources）必须从另一个线程手动发出信号。

&emsp;创建 input source 时，可以将其分配给 run loop 的一种或多种 modes。Modes 会影响在任何给定时刻监视哪些 input sources。大多数情况下，你会在默认模式下运行 run loop，但也可以指定自定义模式（custom modes）。如果 input sources 不在当前监视的 mode 下，则它生成的任何事件都将保留，直到 run loop 以对应（correct）的 mode 运行。

&emsp;以下各节描述了一些 input sources。

#### Port-Based Sources（基于端口的 source）
&emsp;Cocoa 和 Core Foundation 为使用端口相关（port-related）的对象和函数创建基于端口的输入源（port-based input sources）提供了内置支持（provide built-in support）。例如，在 Cocoa 中，你根本不需要直接创建 input source。你只需创建一个端口对象（NSPort object），然后使用 NSPort 的方法将该端口添加到 run loop 中即可。port object 为你处理所需 input source 的创建和配置。

&emsp;在 Core Foundation 中，你必须手动创建端口及其运行循环源（run loop source）。在这两种情况下，都可以使用与端口不透明类型（CFMachPortRef、CFMessagePortRef 或 CFSocketRef）关联的函数来创建适当的对象。

&emsp;有关如何设置和配置基于端口的定制源的示例，参考下文 Configuring a Port-Based Input Source。

#### Custom Input Sources（定制输入源）
&emsp;要创建自定义输入源，必须使用与 Core Foundation 中的 CFRunLoopSourceRef 不透明类型关联的函数。你可以使用几个回调函数（callback functions）配置一个自定义输入源（custom input source ）。Core Foundation 在不同的点调用这些函数来配置 source，处理任何传入事件，并在 source 从 run loop 中移除后时将其销毁。

&emsp;除了定义（defining）事件到达（event arrives）时自定义源（custom source）的行为（behavior）外，还必须定义事件传递机制（event delivery mechanism）。这部分 source（this part of the source）运行在一个单独的线程上，负责向输入源（input source）提供其数据，并在数据准备好进行处理时发出信号。事件传递机制（event delivery mechanism）由你决定，但不必过于复杂。

&emsp;有关如何创建自定义输入源的示例，请参阅 Defining a Custom Input Source。有关自定义输入源的参考信息，请参考 CFRunLoopSource Reference。

#### Cocoa Perform Selector Sources
&emsp;除了基于端口的源（port-based sources）外，Cocoa 还定义了一个自定义输入源（custom input source），允许你在任何线程上执行 selector。与基于端口的源（port-based source）一样，perform selector 请求在目标线程上序列化（serialized），从而减轻了在一个线程上运行多个方法时可能出现的许多同步问题。与基于端口的源不同，执行选择器源（perform selector source）在执行其 selector 后将自身从 run loop 中移除。

&emsp;Note：在 OS X v10.5 之前，执行选择器源主要用于将消息发送到主线程，但是在 OS X v10.5 和更高版本以及 iOS 中，可以使用它们将消息发送到任何线程。

&emsp;在另一个线程上执行 selector 时，目标线程必须具有活动的 run loop。对于你创建的子线程，这意味着等待直到你的代码显式启动当前线程的 run loop。但是，由于主线程启动了自己的 run loop，因此你可以在应用程序调用应用程序委托的 `applicationDidFinishLaunching:` 函数后立即开始在主线程上发出调用（添加 selector 在主线程执行）。每次循环时，run loop 都会处理队列中所有的执行 selector 的调用，而不是在每次循环迭代时仅处理一个。

&emsp;表 3-2 列出了在 NSObject 上定义的可用于在其它线程上执行 selector 的方法。因为这些方法是在 NSObject 上声明的（NSObject 的 NSThreadPerformAdditions 和 NSDelayedPerforming 分类中声明 ），所以你可以从任何可以访问 Objective-C 对象的线程中使用它们，包括 POSIX 线程。这些方法实际上并不创建新线程来执行选择器，仅在你指定的或当前的开启了 run loop 的线程中执行。

&emsp;表 3-2 在指定的 thread 执行 selector
| Methods | Description |
| - | - |
| performSelectorOnMainThread:withObject:waitUntilDone: performSelectorOnMainThread:withObject:waitUntilDone:modes: | 在应用程序的主线程的下一个 run loop 周期中执行指定的 selector。这些方法提供了在执行 selector 之前阻塞当前线程的选项，即 waitUntilDone 参数表示是否等待 selector 执行完成再执行接下来的语句。 |
| performSelector:onThread:withObject:waitUntilDone:
performSelector:onThread:withObject:waitUntilDone:modes: | 在具有 NSThread 对象的任何线程上执行指定的选择器。这些方法使你可以选择阻塞当前线程，直到 selector 执行完成为止。 |
| performSelector:withObject:afterDelay:
performSelector:withObject:afterDelay:inModes: | 在下一个 run loop 周期中以及可选的延迟时间之后，在当前线程上执行指定的 selector。因为它一直等到下一个 run loop 周期执行 selector，所以这些方法提供了当前执行代码的最小自动延迟（执行等待时间至少大于等于 afterDelay 参数）。多个排队的 selectors 按照排队的顺序依次执行。 |
| cancelPreviousPerformRequestsWithTarget:
cancelPreviousPerformRequestsWithTarget:selector:object: | 使你可以取消使用 performSelector:withObject:afterDelay: 或 performSelector:withObject:afterDelay:inModes: 方法发送到当前线程的消息。 |

&emsp;有关每种方法的详细信息，可参考 NSObject Class Reference。

### Timer Sources
&emsp;Timer sources 在将来的预设时间将事件同步传递到你的线程。Timers 是线程通知自己执行某事的一种方式。例如，搜索字段可以使用 timer 在用户连续按键之间经过一定时间后启动自动搜索，使用这个延迟时间，用户就有机会在开始搜索之前键入尽可能多的所需搜索字符串。

&emsp;尽管 timer 生成基于时间的通知（time-based notifications），但它不是一种实时机制（real-time mechanism）。与 input sources 一样，timer 与 run loop 的特定 mode 相关联。如果 timer 未处于 run loop 当前监视的模式，则在以 timer 支持的 mode 之一运行 run loop 之前，timer 不会触发。类似地，如果在 run loop 正在执行处理程序例程（handler routine）的过程中触发 timer ，则 timer 将等到下一次通过 run loop 调用其处理程序例程（handler routine）。如果 run loop 根本没有运行，计时器就不会触发。

&emsp;你可以将 timer 配置为仅生成一次事件或重复生成事件。重复 timer 根据预定的触发时间而不是实际触发时间自动重新调度自己。例如，如果一个 timer 被安排在某个特定时间触发，并且此后每隔 5 秒触发一次，那么即使实际触发时间被延迟，计划的触发时间也将始终落在原来的 5 秒时间间隔上。如果触发时间延迟太久，以致错过了一个或多个预定的触发时间，则对于错过的时间段，timer 只触发一次。在为错过的时间段触发后，timer 将重新安排为下一个预定的触发时间。

&emsp;有关配置 timer sources 的更多信息，参考 Configuring Timer Sources。有关参考信息，可参见 NSTimer Coass Reference 或 CFRunLoopTimer Reference。
### Run Loop Observers
&emsp;与在发生适当的异步或同步事件时触发的 sources 不同，run loop observers 在 run loop 本身执行期间在特殊位置激发。你可以使用 run loop observers 来准备线程来处理给定的事件，或者在线程进入休眠状态之前对其进行准备。可以将 run loop observers 与 run loop 中的以下事件关联：

+ run loop 进入入口。
+ run loop 将要处理 timer 时。
+ run loop 将要处理 input source 时。
+ run loop 即将进入睡眠状态时。
+ 当 run loop 已唤醒时，但在它处理唤醒它的事件之前。
+ run loop 退出。

&emsp;你可以使用 Core Foundation 向应用程序添加 run loop observers。要创建 run loop observers，需要创建 CFRunLoopObserverRef 不透明类型的新实例。此类型跟踪自定义回调函数（keeps track of your custom callback function）及其感兴趣的活动。

&emsp;与 timers 类似， run loop observers 可以使用一次或重复使用。一次性 observer 在激发后从 run loop 中移除自己，而重复 observer 保持连接。你可以指定在创建 observer 时是运行一次还是重复运行。

&emsp;有关如何创建 run loop observer 的示例，请参见 Configuring the Run Loop。有关参考信息，请参见 CFRunLoopObserver Reference。

### The Run Loop Sequence of Events
&emsp;每次运行它时，线程的 run loop 都会处理待办事件（pending events），并为所有附加的 observers 生成通知。它执行此操作的顺序非常 具体/明确，如下所示：

1. 通知 observers 即将进入 run loop。
2. 通知 observers 任何准备就绪的 timers 即将触发。
3. 通知 observers 任何不基于端口（not port based）的 input sources（source0）都将被触发。
4. 触发所有准备触发的非基于端口的输入源（non-port-based input sources）(source0)。
5. 如果基于端口的输入源（port-based input source）已准备好并等待启动，请立即处理事件。**转到步骤 9**。
6. 通知 observers，线程即将进入休眠状态。

7. 使线程进入休眠状态并等待唤醒，直到发生以下事件之一时会被唤醒：
  + 基于端口的输入源（port-based input source）（source1）的事件到达。
  + timers 触发。
  + 为 run loop 设置的超时时间过期。
  + run loop 被显式唤醒。
  
8. 通知 observer，线程刚刚唤醒。
9. 处理唤醒时收到的待办事件。
  + 如果触发了用户定义的 timer（处理 timer 事件并重新启动循环）。**转到步骤2**。
  + 如果触发了 input source，传递事件。（source1）
  + 如果 run loop 被明确唤醒但尚未超时，重新启动循环。**转到步骤2**。
10. 通知 observer，run loop 已退出。

&emsp;因为 timer 和 input sources 的 observer 通知是在这些事件实际发生之前传递的，所以通知的时间和实际事件的时间之间可能会有间隔。如果这些事件之间的时间安排很关键，那么可以使用 sleep 和 aweak-from-sleep 通知来帮助你关联实际事件之间的时间安排。

&emsp;因为 timers 和其他周期性事件是在运行 run loop 时传递的，因此绕过该循环会中断这些事件的传递。这种行为的典型示例是，每当你通过进入循环并从应用程序反复请求事件来实现鼠标跟踪例程（mouse-tracking routine）时，都会发生这种行为。因为你的 code 直接撷取事件，而不是让应用程序正常 dispatch 这些事件，所以在你的鼠标跟踪例程（mouse-tracking routine）退出并将控制权传回应用程序之前，活动 timer 将无法触发（unable to fire）。

&emsp;可以使用 run loop 对象显式地唤醒 run loop。其他事件也可能导致 run loop 被唤醒。例如，添加另一个非基于端口的输入源（non-port-based input source）会唤醒 run loop，以便可以立即处理该输入源，而不是等待其他事件发生。

## When Would You Use a Run Loop?
&emsp;唯一需要显式运行 run loop 的时机是在为应用程序创建子线程时。应用程序主线程的 run loop 是基础架构的重要组成部分。因此，应用程序框架（app frameworks）提供了用于运行主应用程序循环（main application loop）并自动启动该循环的代码。 iOS 中 UIApplication 的 run 方法（或 OS X 中 NSApplication）的 run 方法将启动应用程序的主循环（application's main loop），这是正常启动顺序的一部分。如果使用 Xcode 模板项目创建应用程序，则永远不必显式调用这些例程。

&emsp;对于子线程，你需要确定是否需要 run loop，如果需要，请自己配置并启动它。你不需要在所有情况下都启动线程的 run loop。例如，如果你使用一个线程来执行一些长期运行的预定任务，你可能可以避免启动 run loop。run loop 用于希望与线程进行更多交互的情况。例如，如果计划执行以下任一操作，则需要启动 run loop：

+ 使用端口（ports）或自定义输入源（custom input sources）与其他线程通信。
+ 在线程上使用 timer。
+ 在 Cocoa 应用程序中使用任何 `performSelector…` 方法。
+ 保持线程执行定期任务（periodic tasks）。

&emsp;如果选择使用 run loop，则配置和设置很简单。不过，与所有线程编程一样，你应该有一个在适当情况下退出子线程的计划。通过让线程退出而干净地结束它总是比强制它终止要好。有关如何配置和退出运行循环的信息，请参考 Using Run Loop Objects。

## Using Run Loop Objects
&emsp;Run loop 对象提供主接口，用于向 run loop 添加 input sources、timers 和 run loop observers，然后运行它。每个线程都有一个与之关联的 run loop 对象。在 Cocoa 中，此对象是 NSRunLoop 类的实例。在低级应用程序中（low-level application），它是指向 CFRunLoopRef 不透明类型的指针。

### Getting a Run Loop Object
&emsp;要获取当前线程的 run loop，请使用以下方法之一：

+ 在 Cocoa 应用程序中，使用 NSRunLoop 的 `currentRunLoop` 类方法检索 NSRunLoop 对象。
+ 使用 `CFRunLoopGetCurrent` 函数。

&emsp;尽管它们不是免费的桥接类型（not toll-free bridged types），但你可以在需要时从 NSRunLoop 对象获取 CFRunLoopRef 不透明类型。NSRunLoop 类定义一个 `getCFRunLoop` 方法，该方法返回可以传递给 Core Foundation 例程的 CFRunLoopRef 类型。因为这两个对象引用同一个 run loop，因此可以根据需要混合调用 NSRunLoop 对象和 CFRunLoopRef 不透明类型。

### Configuring the Run Loop
&emsp;在子线程上运行 run loop 之前，必须至少向其添加一个 input source 或 timer。如果 run loop 没有任何要监视的 source，则当你尝试运行它时，它会立即退出。有关如何向 run loop 添加 sources 的示例，请参考 Configuring Run Loop Sources。

&emsp;除了安装 sources 之外，还可以安装 run loop observers，并使用它们来检测 run loop 的不同执行阶段。要安装 run loop observers，需要创建一个 CFRunLoopObserverRef 不透明类型，并使用 `CFRunLoopAddObserver` 函数将其添加到 run loop 中。Run loop observers 必须使用 Core Foundation 创建，即使对于 Cocoa 应用程序也是如此。

&emsp;清单 3-1显示了一个线程的主例程，该线程将一个 run loop observer 附加到它的 run loop 上。该示例的目的是向你展示如何创建 run loop observer，因此代码只需设置一个 run loop observer 来监视所有 run loop 活动。基本处理程序例程（basic handler routine）（未显示）只是在处理 timer 请求时记录 run loop 活动。

&emsp;清单 3-1 创建一个 run loop observer
```c++
- (void)threadMain {
    // The application uses garbage collection, so no autorelease pool is needed.
    NSRunLoop *myRunLoop = [NSRunLoop currentRunLoop];
    
    // Create a run loop observer and attach it to the run loop.
    CFRunLoopObserverContext context = {0, (__bridge void *)(self), NULL, NULL, NULL};
    CFRunLoopObserverRef observer = CFRunLoopObserverCreate(kCFAllocatorDefault, kCFRunLoopAllActivities, YES, 0, &myRunLoopObserver, &context);
    
    if (observer) {
        CFRunLoopRef cfLoop = [myRunLoop getCFRunLoop];
        CFRunLoopAddObserver(cfLoop, observer, kCFRunLoopDefaultMode);
    }
    
    // Create and schedule the timer.
    [NSTimer scheduledTimerWithTimeInterval:0.1 target:self selector:@selector(doFireTimer:) userInfo:nil repeats:YES];
    
    NSInteger loopCount = 10;
    do {
        // Run the run loop 10 times to let the timer fire.
        [myRunLoop runUntilDate:[NSDate dateWithTimeIntervalSinceNow:1]];
        loopCount--;
    } while (loopCount);
}
```
&emsp;在为长生存期线程（long-lived thread）配置运行循环时，最好至少添加一个 input source 来接收消息。虽然你可以在只附加 timer 的情况下进入运行 run loop，但一旦 timer 触发，它通常会失效，这将导致 run loop 退出。附加一个重复 timer 可以使 run loop 在较长的时间内运行，但需要定期启动 timer 以唤醒线程，这实际上是另一种轮询形式。相比之下，input source 等待事件发生，让线程一直处于休眠状态。

### Starting the Run Loop
&emsp;仅对于应用程序中的子线程，才需要启动运行循环。一个 run loop 必须至少具有一个要监视的 input source 或 timer。如果未附加的话，则 run loop 立即退出。
&emsp;有几种启动 run loop 的方法，包括以下几种：

+ 无条件的（Unconditionally，run 函数）
+ 有固定的时间限制（runUntilDate: 函数）
+ 以特定的 mode（runMode:beforeDate: 函数）

&emsp;无条件地进入 run loop 是最简单的选择，但也是最不可取的。无条件地运行 run loop 会将线程放入一个永久循环中，这使你几乎无法控制 run loop 本身。你可以添加和删除 input source 和 timer（但是 run loop 依然不会退出），但停止 run loop 的唯一方法是终止它（退出并销毁）。也无法在自定义模式（custom mode）下运行 run loop。

&emsp;与其无条件运行运行 run loop，不如使用超时值运行 run loop。使用超时值时，run loop 将一直运行，直到事件到达或分配的时间过期。如果事件到达，则将该事件分派给处理程序进行处理，然后 run loop 退出。然后，你的代码可以重新启动 run loop 来处理下一个事件。如果分配的时间过期，你可以简单地重新启动 run loop 或使用该时间来执行任何需要的内务处理（housekeeping）。

&emsp;除了超时值，还可以使用特定 mode 运行 run loop 。mode 和超时值不是互斥的，并且在启动 run loop 时都可以使用。Modes 限制向 run loop 传递事件的 sources 的类型，在 Run Loop Modes 中有更详细的描述。

&emsp;清单 3-2 显示了一个线程主入口例程的框架版本。本例的关键部分是显示了 run loop 的基本结构。本质上，你将  input sources 和 timers 添加到 run loop 中，然后反复调用其中一个例程来启动 run loop。每次 run loop 例程返回时，都要检查是否出现了任何可能需要退出线程的条件。该示例使用 Core Foundation run loop 例程，以便检查返回结果并确定 run loop 退出的原因。如果你使用的是 Cocoa，并且不需要检查返回值，也可以使用 NSRunLoop 类的方法以类似的方式运行 run loop。（有关调用 NSRunLoop 类方法的 run loop 示例，请参见清单 3-14。）

&emsp;清单 3-2 运行一个 run loop
```c++
- (void)skeletonThreadMain {
    // Set up an autorelease pool here if not using garbage collection.
    BOOL done = NO;
 
    // Add your sources or timers to the run loop and do any other setup.
    // 将你的 sources 或 timers 添加到运行循环中，然后进行其他任何设置。
 
    do {
        // Start the run loop but return after each source is handled.
        // 开始运行循环，但在处理完每个 source 之后返回。
        SInt32    result = CFRunLoopRunInMode(kCFRunLoopDefaultMode, 10, YES);
 
        // If a source explicitly stopped the run loop, or if there are no sources or timers, go ahead and exit.
        // 如果 source 明确停止了运行循环，或者没有 source 或 timer，则继续并退出。
        if ((result == kCFRunLoopRunStopped) || (result == kCFRunLoopRunFinished))
            done = YES;
 
        // Check for any other exit conditions here and set the done variable as needed.
        // 在此处检查其他退出条件，并根据需要设置 done 变量。
    }
    while (!done);
 
    // Clean up code here. Be sure to release any allocated autorelease pools.
    // 在这里清理代码。确保释放所有分配的自动释放池。
}
```

&emsp;可以递归地运行 run loop。换句话说，可以调用 `CFRunLoopRun`、`CFRunLoopRunInMode` 或任何 NSRunLoop 方法，以便从 input source 或 timer 的处理程序例程中启动 run loop。执行此操作时，可以使用任何要运行嵌套 run loop 的 Mode，包括外部 run loop 使用的 mode。

### Exiting the Run Loop
&emsp;在处理事件之前，有两种方法可以使 run loop 退出：

+ 配置 run loop 以使用超时值运行。
+ 告诉运行循环停止。

&emsp;如果可以管理的话，使用超时值当然是首选。指定一个超时值可以让 run loop 在退出之前完成其所有的正常处理，包括向 run loop observers 发送通知。

&emsp;使用 `CFRunLoopStop` 函数显式停止 run loop 会产生类似超时的结果。run loop 发送任何剩余的 run loop 通知，然后退出。不同之处在于，你可以在无条件启动的运行循环中使用此技术。

&emsp;尽管删除 run loop 的 input sources 和 timers 也可能导致 run loop 退出，但这不是停止 run loop 的可靠方法。一些系统例程将 input sources 添加到 run loop 以处理所需的事件。因为你的 code 可能不知道这些 input source，所以无法移除它们，这会阻止 run loop 退出。

### Thread Safety and Run Loop Objects
&emsp;线程安全性因使用哪个 API 来操作 run loop 而不同。Core Foundation 中的函数通常是线程安全的，可以从任何线程调用。但是，如果执行的操作更改了 run loop 的配置，那么只要可能，最好从拥有 run loop 的线程中进行更改。

&emsp;Cocoa NSRunLoop 类不像它的 Core Foundation 中对应类那样本质上是线程安全的。如果使用 NSRunLoop 类来修改运行循环，则应仅从拥有该 run loop 的同一线程进行修改。将 input sources 或 timers 添加到属于不同线程的 run loop 中可能会导致代码崩溃或行为异常。

## Configuring Run Loop Sources
&emsp;下面的部分展示了如何在 Cocoa 和 Core Foundation 中设置不同类型的 input sources 的示例。

### Defining a Custom Input Source
&emsp;创建自定义输入源（custom input source）涉及定义以下内容：

+ 希望 input source 处理的信息。
+ 调度程序例程（scheduler routine），让感兴趣的 clients 知道如何联系你的 input source。
+ 处理程序例程（handler routine），用于执行任何 clients 发送的请求。
+ 取消例程（cancellation routine）使 input source 无效。

&emsp;因为你创建了一个自定义输入源来处理自定义信息，所以实际配置的设计是灵活的。调度程序、处理程序和取消例程（scheduler, handler, and cancellation routines）是自定义输入源几乎总是需要的关键例程。然而，其余的输入源行为大多发生在这些处理程序例程之外。例如，由你定义将数据传递到输入源以及将输入源的存在与其他线程通信的机制。

&emsp;图 3-2 显示了一个自定义输入源（custom input source）的配置示例。在本例中，应用程序的主线程维护对输入源、该输入源的定制命令缓冲区以及安装该输入源的运行循环的引用。当主线程有一个任务要交给工作线程时，它将一个命令以及工作线程启动该任务所需的任何信息一起发布到命令缓冲区。（因为主线程和工作线程的输入源都可以访问命令缓冲区，所以该访问必须同步。）一旦发布命令，主线程就会向输入源发出信号并唤醒工作线程的 run loop。在接收到唤醒（wake-up）命令后，run loop 调用输入源的处理程序，处理命令缓冲区中的命令。

&emsp;图 3-2 操作自定义 input source

![Operating a custom input source](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/7b8018f5ca19411ebc14d21f36b18d57~tplv-k3u1fbpfcp-watermark.image)

&emsp;以下部分将解释上图中自定义输入源的实现，并显示需要实现的关键代码。

### Defining the Input Source
&emsp;定义自定义输入源需要使用 Core Foundation 例程来配置你的运行循环源并将其附加到运行循环。尽管基本处理程序是基于 C 的函数，但这并不妨碍你编写这些函数的包装程序并使用 Objective-C 或 C++ 来实现代码主体。

&emsp;图 3-2 中引入的输入源使用 Objective-C 对象来管理命令缓冲区并与 run loop 协调。清单 3-3 显示了这个对象的定义。RunLoopSource 对象管理命令缓冲区，并使用该缓冲区从其它线程接收消息。这个清单还显示了 RunLoopContext 对象的定义，它实际上只是一个容器对象，用于传递 RunLoopSource 对象和对应用程序主线程的 run loop 引用。

&emsp;清单 3-3 custom input source 对象定义
```c++
@interface RunLoopSource : NSObject {
    CFRunLoopSourceRef runLoopSource;
    NSMutableArray* commands;
}
 
- (id)init;
- (void)addToCurrentRunLoop;
- (void)invalidate;
 
// Handler method
- (void)sourceFired;
 
// Client interface for registering commands to process
- (void)addCommand:(NSInteger)command withData:(id)data;
- (void)fireAllCommandsOnRunLoop:(CFRunLoopRef)runloop;
 
@end
 
// These are the CFRunLoopSourceRef callback functions.
void RunLoopSourceScheduleRoutine (void *info, CFRunLoopRef rl, CFStringRef mode);
void RunLoopSourcePerformRoutine (void *info);
void RunLoopSourceCancelRoutine (void *info, CFRunLoopRef rl, CFStringRef mode);
 
// RunLoopContext is a container object used during registration of the input source.
@interface RunLoopContext : NSObject {
    CFRunLoopRef        runLoop;
    RunLoopSource*        source;
}
@property (readonly) CFRunLoopRef runLoop;
@property (readonly) RunLoopSource* source;
 
- (id)initWithSource:(RunLoopSource*)src andLoop:(CFRunLoopRef)loop;
@end
```
&emsp;尽管 Objective-C 代码管理输入源的自定义数据，但将输入源附加到 run loop 需要基于 C 的回调函数。当你实际将 run loop 源附加到运行循环时，将调用这些函数中的第一个，如清单 3-4 所示。因为这个输入源只有一个 client（主线程），所以它使用调度程序函数发送一条消息，向该线程上的应用程序委托注册自己。当委托想要与输入源通信时，它使用 RunLoopContext 对象中的信息进行通信。

&emsp;清单 3-4 Scheduling a run loop source（安排运行循环源）
```c++
void RunLoopSourceScheduleRoutine (void *info, CFRunLoopRef rl, CFStringRef mode) {
    RunLoopSource* obj = (RunLoopSource*)info;
    AppDelegate*   del = [AppDelegate sharedAppDelegate];
    RunLoopContext* theContext = [[RunLoopContext alloc] initWithSource:obj andLoop:rl];
 
    [del performSelectorOnMainThread:@selector(registerSource:)
                                withObject:theContext waitUntilDone:NO];
}
```
&emsp;最重要的回调例程之一是在向输入源发出信号时用于处理自定义数据的回调例程。清单 3-5 显示了与 RunLoopSource 对象关联的执行回调例程。此函数只需将执行工作的请求转发给 sourceFired 方法，然后该方法处理命令缓冲区中的任何命令。

&emsp;清单 3-5 Performing work in the input source（在输入源中执行工作）
```c++
void RunLoopSourcePerformRoutine (void *info) {
    RunLoopSource*  obj = (RunLoopSource*)info;
    [obj sourceFired];
}
```
&emsp;如果使用 CFRunLoopSourceInvalidate 函数从其 run loop 中删除输入源，系统将调用输入源的取消例程。你可以使用此例程通知客户端你的输入源不再有效，并且应该删除对它的任何引用。清单 3-6 显示了注册到 RunLoopSource 对象的取消回调例程。此函数将另一个 RunLoopContext 对象发送给应用程序委托，但这次请求委托删除对 run loop source 的引用。

&emsp;清单 3-6 Invalidating an input sourc（使输入源无效）
```c++
void RunLoopSourceCancelRoutine (void *info, CFRunLoopRef rl, CFStringRef mode) {
    RunLoopSource* obj = (RunLoopSource*)info;
    AppDelegate* del = [AppDelegate sharedAppDelegate];
    RunLoopContext* theContext = [[RunLoopContext alloc] initWithSource:obj andLoop:rl];
 
    [del performSelectorOnMainThread:@selector(removeSource:)
                                withObject:theContext waitUntilDone:YES];
}
```
&emsp;注意：应用程序委托的 `registerSource:` 和 `removeSource:` 方法的代码显示在 Coordinating with Clients of the Input Source。（与输入源的 Clients 协调）

### Installing the Input Source on the Run Loop
&emsp;清单 3-7 显示了 RunLoopSource 类的 `init` 和 `addToCurrentRunLoop` 方法。 `init` 方法创建 CFRunLoopSourceRef 不透明类型，该类型必须实际附加到 run loop。它会将 RunLoopSource 对象本身作为上下文信息传递，以便回调例程具有指向该对象的指针。在工作线程调用 `addToCurrentRunLoop` 方法之前，不会安装输入源，此时将调用 `RunLoopSourceScheduleRoutine` 回调函数。将输入源添加到 run loop 后，线程可以运行其 run loop 以等待它。

&emsp;清单 3-7  Installing the run loop source（安装运行循环源）
```c++
- (id)init {
    CFRunLoopSourceContext    context = {0, self, NULL, NULL, NULL, NULL, NULL,
                                        &RunLoopSourceScheduleRoutine,
                                        RunLoopSourceCancelRoutine,
                                        RunLoopSourcePerformRoutine};
 
    runLoopSource = CFRunLoopSourceCreate(NULL, 0, &context);
    commands = [[NSMutableArray alloc] init];
 
    return self;
}
 
- (void)addToCurrentRunLoop {
    CFRunLoopRef runLoop = CFRunLoopGetCurrent();
    CFRunLoopAddSource(runLoop, runLoopSource, kCFRunLoopDefaultMode);
}
```

### Coordinating with Clients of the Input Source
&emsp;为了使你的输入源有用，你需要对其进行操作并从另一个线程发出信号。输入源的全部意义是将其关联的线程进入休眠状态，直到有事情要做。这一事实使得你的应用程序中的其它线程必须了解输入源并有一种与之通信的方法。

&emsp;通知 clients 输入源的一种方法是在输入源首次安装到其运行循环中时发送注册请求。你可以向任意多个 clients 注册你的输入源，或者你可以简单地向某个中央机构注册，然后将你的输入源出售给感兴趣的 clients。清单 3-8 显示了由应用程序委托定义的注册方法，并在 RunLoopSource 对象的调度程序函数被调用时调用。此方法接收 RunLoopSource 对象提供的 RunLoopContext 对象并将其添加到其源列表中。此清单还显示了从运行循环中删除输入源时用于注销输入源的例程。

&emsp;清单 3-8 Registering and removing an input source with the application delegate（使用应用程序委托注册和删除输入源）
```c++
- (void)registerSource:(RunLoopContext*)sourceInfo {
    [sourcesToPing addObject:sourceInfo];
}

- (void)removeSource:(RunLoopContext*)sourceInfo {
    id    objToRemove = nil;
 
    for (RunLoopContext* context in sourcesToPing) {
        if ([context isEqual:sourceInfo]) {
            objToRemove = context;
            break;
        }
    }
 
    if (objToRemove)
        [sourcesToPing removeObject:objToRemove];
}
```
&emsp;Note：调用前面清单中方法的回调函数如清单 3-4 和清单 3-6 所示。

### Signaling the Input Source
&emsp;在将数据传递给输入源之后，client 必须向源发送信号并唤醒其运行循环。发信号给源让运行循环知道源已经准备好被处理。因为当信号发生时线程可能处于休眠状态，所以应该始终显式地唤醒运行循环。否则，可能会导致处理输入源的延迟。

&emsp;清单 3-9 显示了 RunLoopSource 对象的 `fireCommandsOnRunLoop` 方法。当 clients 准备好让 source 处理它们添加到缓冲区中的命令时，它们会调用此方法。

&emsp;清单 3-9 Waking up the run loop（唤醒运行循环）
```c++
- (void)fireCommandsOnRunLoop:(CFRunLoopRef)runloop {
    CFRunLoopSourceSignal(runLoopSource);
    CFRunLoopWakeUp(runloop);
}
```
&emsp;Note：你不应该试图通过消息传递自定义输入源来处理 SIGHUP 或其他类型的进程级信号。用于唤醒运行循环的 Core Foundation 函数不是信号安全的，因此不应在应用程序的信号处理程序例程中使用。有关信号处理程序例程的更多信息，请参见 sigaction 手册页。

## Configuring Timer Sources
&emsp;要创建一个 timer source，你所要做的就是创建一个 timer 对象并在 run loop 中调度它。在 Cocoa 中，使用 NSTimer 类创建新的 timer 对象，在 Core Foundation 中使用 CFRunLoopTimerRef 不透明类型。在内部，NSTimer 类只是 Core Foundation 的一个扩展，提供了一些便利功能，例如可以使用相同的方法创建和调度 timer。

&emsp;在 Cocoa 中，你可以使用以下两种方法之一同时创建和安排 timer：

+ `scheduledTimerWithTimeInterval:target:selector:userInfo:repeats:`
+ `scheduledTimerWithTimeInterval:invocation:repeats:`

&emsp;这些方法创建 timer 并以默认模式（NSDefaultRunLoopMode）将其添加到当前线程的 run loop 中。如果需要，也可以手动计划 timer，方法是创建 NSTimer 对象，然后使用 NSRunLoop 的 `addTimer:forMode:` 方法。这两种技术基本上都是一样的，但是对 timer 的配置有不同的控制级别。例如，如果你创建 timer 并手动将其添加到 run loop 中，则可以使用默认模式以外的模式执行此操作。清单 3-10 展示了如何使用这两种技术创建 timer。第一个 timer 的初始延迟为 1 秒，此后每隔 0.1 秒定期触发一次。第二个 timer 在初始 0.2 秒延迟后开始触发，然后每隔 0.2 秒触发一次。

&emsp;清单 3-10  Creating and scheduling timers using NSTimer（使用 NSTimer 创建和安排计时器）
```c++
NSRunLoop* myRunLoop = [NSRunLoop currentRunLoop];
 
// Create and schedule the first timer.
NSDate* futureDate = [NSDate dateWithTimeIntervalSinceNow:1.0];
NSTimer* myTimer = [[NSTimer alloc] initWithFireDate:futureDate
                        interval:0.1
                        target:self
                        selector:@selector(myDoFireTimer1:)
                        userInfo:nil
                        repeats:YES];
[myRunLoop addTimer:myTimer forMode:NSDefaultRunLoopMode];
 
// Create and schedule the second timer.
[NSTimer scheduledTimerWithTimeInterval:0.2
                        target:self
                        selector:@selector(myDoFireTimer2:)
                        userInfo:nil
                        repeats:YES];
```
&emsp;清单 3-11显示了使用 Core Foundation 函数配置计时器所需的代码。尽管此示例未在上下文结构中传递任何用户定义的信息，但是你可以使用此结构传递计时器所需的任何自定义数据。有关此结构的内容的更多信息，请参见 CFRunLoopTimer Reference 中的描述。

&emsp;清单 3-11 Creating and scheduling a timer using Core Foundation（使用 Core Foundation 创建和安排计时器）
```c++
CFRunLoopRef runLoop = CFRunLoopGetCurrent();
CFRunLoopTimerContext context = {0, NULL, NULL, NULL, NULL};
CFRunLoopTimerRef timer = CFRunLoopTimerCreate(kCFAllocatorDefault, 0.1, 0.3, 0, 0,
                                        &myCFTimerCallback, &context);
 
CFRunLoopAddTimer(runLoop, timer, kCFRunLoopCommonModes);
```
## Configuring a Port-Based Input Source
&emsp;Cocoa 和 Core Foundation 都提供了基于端口的对象，用于在线程之间或进程之间进行通信。以下各节说明如何使用几种不同类型的端口来设置端口通信。

### Configuring an NSMachPort Object
&emsp;要与 NSMachPort 对象建立本地连接，需要创建端口对象并将其添加到主线程的运行循环中。启动子线程时，将同一对象传递给线程的入口点函数。子线程可以使用相同的对象将消息发送回主线程。

#### Implementing the Main Thread Code
&emsp;清单 3-12 显示了启动辅助（子）工作线程的主线程代码。因为 Cocoa 框架执行许多配置端口和运行循环的中间步骤，`launchThread` 方法明显比它的 Core Foundation 等效方法短（清单 3-17）；但是，两者的行为几乎相同。一个不同之处在于，这个方法直接发送 NSPort 对象，而不是将本地端口的名称发送给工作线程。

&emsp;清单 3-12 Main thread launch method（主线程启动方法）
```c++
- (void)launchThread {
    NSPort* myPort = [NSMachPort port];
    if (myPort) {
        // This class handles incoming port messages. 此类处理传入的端口消息。
        [myPort setDelegate:self];
 
        // Install the port as an input source on the current run loop. 将端口安装为当前运行循环上的输入源。
        [[NSRunLoop currentRunLoop] addPort:myPort forMode:NSDefaultRunLoopMode];
 
        // Detach the thread. Let the worker release the port. 分离线程。释放端口。
        [NSThread detachNewThreadSelector:@selector(LaunchThreadWithPort:)
               toTarget:[MyWorkerClass class] withObject:myPort];
    }
}
```
&emsp;为了在线程之间建立双向通信通道，你可能希望工作线程在 check-in 消息中向主线程发送自己的本地端口。接收到 check-in 消息可以让你的主线程知道在启动第二个线程时一切顺利，还可以向该线程发送进一步的消息。

&emsp;清单 3-13 显示了主线程的 `handlePortMessage:` 方法。当数据到达线程自己的本地端口时调用此方法。当 check-in 消息到达时，该方法直接从端口消息中检索辅助线程的端口，并将其保存以供以后使用。

&emsp;清单 3-13 Handling Mach port messages
```c++
#define kCheckinMessage 100
 
// Handle responses from the worker thread.
- (void)handlePortMessage:(NSPortMessage *)portMessage {
    unsigned int message = [portMessage msgid];
    NSPort* distantPort = nil;
 
    if (message == kCheckinMessage) {
        // Get the worker thread’s communications port. 获取辅助线程的通信端口。
        distantPort = [portMessage sendPort];
 
        // Retain and save the worker port for later use. 保留并保存工作端口，以备后用。
        [self storeDistantPort:distantPort];
    } else {
        // Handle other messages. 处理其他消息。
    }
}
```

#### Implementing the Secondary Thread Code
&emsp;对于辅助工作线程，你必须配置线程并使用指定的端口将信息传递回主线程。

&emsp;清单 3-14 显示了设置工作线程的代码。在为线程创建自动释放池之后，该方法将创建一个工作器对象以驱动线程执行。工作程序对象的 `sendCheckinMessage:` 方法（如清单 3-15 所示）为工作程序线程创建本地端口，并将 check-in 消息发送回主线程。

&emsp;清单 3-14 Launching the worker thread using Mach ports（使用 Mach 端口启动辅助线程）
```c++
+(void)LaunchThreadWithPort:(id)inData {
    NSAutoreleasePool*  pool = [[NSAutoreleasePool alloc] init];
 
    // Set up the connection between this thread and the main thread. 设置此线程和主线程之间的连接。
    NSPort* distantPort = (NSPort*)inData;
 
    MyWorkerClass*  workerObj = [[self alloc] init];
    [workerObj sendCheckinMessage:distantPort];
    [distantPort release];
 
    // Let the run loop process things. 让运行循环处理事物。
    do {
        [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode
                            beforeDate:[NSDate distantFuture]];
    }
    while (![workerObj shouldExit]);
 
    [workerObj release];
    [pool release];
}
```
&emsp;当使用 NSMachPort 时，本地和远程线程可以在线程之间使用相同的端口对象进行单向通信。换句话说，由一个线程创建的本地端口对象成为另一个线程的远程端口对象。

&emsp;清单 3-15 显示了辅助线程的签入例程。此方法设置自己的本地端口以用于将来的通信，然后将签入消息发送回主线程。该方法使用在 `LaunchThreadWithPort:` 方法中接收的端口对象作为消息的目标。

&emsp;清单 3-15 显示了子线程的签入例程（check-in routine）。此方法为将来的通信设置自己的本地端口，然后将 check-in 消息发送回主线程。该方法使用 `LaunchThreadWithPort:` 方法中接收的端口对象作为消息的目标。

&emsp;清单 3-15 Sending the check-in message using Mach ports（使用 Mach 端口发送签入消息）
```c++
// Worker thread check-in method 工作线程签入方法
- (void)sendCheckinMessage:(NSPort*)outPort {
    // Retain and save the remote port for future use. 保留并保存远程端口以备将来使用。
    [self setRemotePort:outPort];
 
    // Create and configure the worker thread port. 创建并配置工作线程端口。
    NSPort* myPort = [NSMachPort port];
    [myPort setDelegate:self];
    [[NSRunLoop currentRunLoop] addPort:myPort forMode:NSDefaultRunLoopMode];
 
    // Create the check-in message. 创建 check-in 消息。
    NSPortMessage* messageObj = [[NSPortMessage alloc] initWithSendPort:outPort
                                         receivePort:myPort components:nil];
 
    if (messageObj) {
        // Finish configuring the message and send it immediately. 完成配置消息并立即发送。
        [messageObj setMsgId:setMsgid:kCheckinMessage];
        [messageObj sendBeforeDate:[NSDate date]];
    }
}
```
### Configuring an NSMessagePort Object
&emsp;要与 NSMessagePort 对象建立本地连接，不能简单地在线程之间传递端口对象。远程消息端口必须按名称获取。在 Cocoa 中实现这一点需要用一个特定的名称注册本地端口，然后将该名称传递给远程线程，以便它可以获得适当的端口对象进行通信。清单 3-16 显示了在需要使用消息端口的情况下创建和注册端口的过程。

&emsp;清单 3-16  Registering a message port
```c++
NSPort* localPort = [[NSMessagePort alloc] init];
 
// Configure the object and add it to the current run loop. 配置对象并将其添加到当前运行循环。
[localPort setDelegate:self];
[[NSRunLoop currentRunLoop] addPort:localPort forMode:NSDefaultRunLoopMode];
 
// Register the port using a specific name. The name must be unique. 使用特定名称注册端口。名称必须唯一。
NSString* localPortName = [NSString stringWithFormat:@"MyPortName"];
[[NSMessagePortNameServer sharedInstance] registerPort:localPort
                     name:localPortName];
```
### Configuring a Port-Based Input Source in Core Foundation
&emsp;本节介绍如何使用 Core Foundation 在应用程序的主线程和工作线程之间设置双向通信通道。

&emsp;清单 3-17 显示了应用程序的主线程调用以启动工作线程的代码。代码所做的第一件事是设置一个 CFMessagePortRef 不透明类型来侦听来自工作线程的消息。工作线程需要连接端口的名称，以便将字符串值传递到工作线程的入口点函数。端口名在当前用户上下文中通常应该是唯一的；否则，你可能会遇到冲突。

&emsp;清单 3-17 Attaching a Core Foundation message port to a new thread（将 Core Foundation 消息端口附加到新线程）
```c++
#define kThreadStackSize        (8 *4096)
 
OSStatus MySpawnThread() {
    // Create a local port for receiving responses. 创建一个本地端口以接收响应。
    CFStringRef myPortName;
    CFMessagePortRef myPort;
    CFRunLoopSourceRef rlSource;
    CFMessagePortContext context = {0, NULL, NULL, NULL, NULL};
    Boolean shouldFreeInfo;
 
    // Create a string with the port name. 用端口名称创建一个字符串。
    myPortName = CFStringCreateWithFormat(NULL, NULL, CFSTR("com.myapp.MainThread"));
 
    // Create the port. 创建端口。
    myPort = CFMessagePortCreateLocal(NULL,
                myPortName,
                &MainThreadResponseHandler,
                &context,
                &shouldFreeInfo);
 
    if (myPort != NULL) {
        // The port was successfully created. 端口已成功创建。
        // Now create a run loop source for it. 现在为其创建一个运行循环源。
        rlSource = CFMessagePortCreateRunLoopSource(NULL, myPort, 0);
 
        if (rlSource) {
            // Add the source to the current run loop. 将源添加到当前的运行循环中。
            CFRunLoopAddSource(CFRunLoopGetCurrent(), rlSource, kCFRunLoopDefaultMode);
 
            // Once installed, these can be freed. 安装后，可以将其释放。
            CFRelease(myPort);
            CFRelease(rlSource);
        }
    }
 
    // Create the thread and continue processing. 创建线程并继续处理。
    MPTaskID        taskID;
    return(MPCreateTask(&ServerThreadEntryPoint,
                    (void*)myPortName,
                    kThreadStackSize,
                    NULL,
                    NULL,
                    NULL,
                    0,
                    &taskID));
}
```
&emsp;安装了端口并启动了线程后，主线程可以在等待线程 check in 时继续其常规执行。当 check-in 消息到达时，它被调度到主线程的 `MainThreadResponseHandler` 函数，如清单 3-18 所示。此函数用于提取工作线程的端口名，并为将来的通信创建管道。

&emsp;清单 3-18 Receiving the checkin message
```c++
#define kCheckinMessage 100
 
// Main thread port message handler 主线程端口消息处理程序
CFDataRef MainThreadResponseHandler(CFMessagePortRef local,
                    SInt32 msgid,
                    CFDataRef data,
                    void* info) {
    if (msgid == kCheckinMessage) {
        CFMessagePortRef messagePort;
        CFStringRef threadPortName;
        CFIndex bufferLength = CFDataGetLength(data);
        UInt8* buffer = CFAllocatorAllocate(NULL, bufferLength, 0);
 
        CFDataGetBytes(data, CFRangeMake(0, bufferLength), buffer);
        threadPortName = CFStringCreateWithBytes (NULL, buffer, bufferLength, kCFStringEncodingASCII, FALSE);
 
        // You must obtain a remote message port by name. 你必须按名称获取远程消息端口。
        messagePort = CFMessagePortCreateRemote(NULL, (CFStringRef)threadPortName);
 
        if (messagePort) {
            // Retain and save the thread’s comm port for future reference. 保留并保存线程的通讯端口，以备将来参考。
            AddPortToListOfActiveThreads(messagePort);
 
            // Since the port is retained by the previous function, release it here. 由于该端口由先前的功能保留，因此请在此处释放它。
            CFRelease(messagePort);
        }
 
        // Clean up.
        CFRelease(threadPortName);
        CFAllocatorDeallocate(NULL, buffer);
    } else {
        // Process other messages. 处理其他消息。
    }
 
    return NULL;
}
```
&emsp;配置了主线程后，剩下的唯一任务就是让新创建的工作线程创建自己的端口并 check in。清单 3-19 显示了工作线程的入口点函数。该函数提取主线程的端口名，并使用它来创建回主线程的远程连接。然后，该函数为自己创建一个本地端口，在线程的运行循环中安装该端口，并向主线程发送一个包含本地端口名的 check-in 消息。

&emsp;清单 3-19  Setting up the thread structures
```c++
OSStatus ServerThreadEntryPoint(void* param) {
    // Create the remote port to the main thread. 创建到主线程的远程端口。
    CFMessagePortRef mainThreadPort;
    CFStringRef portName = (CFStringRef)param;
 
    mainThreadPort = CFMessagePortCreateRemote(NULL, portName);
 
    // Free the string that was passed in param. 释放在参数中传递的字符串。
    CFRelease(portName);
 
    // Create a port for the worker thread. 为工作线程创建端口。
    CFStringRef myPortName = CFStringCreateWithFormat(NULL, NULL, CFSTR("com.MyApp.Thread-%d"), MPCurrentTaskID());
 
    // Store the port in this thread’s context info for later reference. 将端口存储在此线程的上下文信息中，以供以后参考。
    CFMessagePortContext context = {0, mainThreadPort, NULL, NULL, NULL};
    Boolean shouldFreeInfo;
    Boolean shouldAbort = TRUE;
 
    CFMessagePortRef myPort = CFMessagePortCreateLocal(NULL,
                myPortName,
                &ProcessClientRequest,
                &context,
                &shouldFreeInfo);
 
    if (shouldFreeInfo) {
        // Couldn't create a local port, so kill the thread. 无法创建本地端口，请杀死线程。
        MPExit(0);
    }
 
    CFRunLoopSourceRef rlSource = CFMessagePortCreateRunLoopSource(NULL, myPort, 0);
    if (!rlSource) {
        // Couldn't create a local port, so kill the thread. 无法创建本地端口，请杀死线程。
        MPExit(0);
    }
 
    // Add the source to the current run loop. 将源添加到当前的运行循环中。
    CFRunLoopAddSource(CFRunLoopGetCurrent(), rlSource, kCFRunLoopDefaultMode);
 
    // Once installed, these can be freed. 安装后，可以将其释放。
    CFRelease(myPort);
    CFRelease(rlSource);
 
    // Package up the port name and send the check-in message. 打包端口名称并发送签入消息。
    CFDataRef returnData = nil;
    CFDataRef outData;
    CFIndex stringLength = CFStringGetLength(myPortName);
    UInt8* buffer = CFAllocatorAllocate(NULL, stringLength, 0);
 
    CFStringGetBytes(myPortName,
                CFRangeMake(0,stringLength),
                kCFStringEncodingASCII,
                0,
                FALSE,
                buffer,
                stringLength,
                NULL);
 
    outData = CFDataCreate(NULL, buffer, stringLength);
 
    CFMessagePortSendRequest(mainThreadPort, kCheckinMessage, outData, 0.1, 0.0, NULL, NULL);
 
    // Clean up thread data structures. 清理线程数据结构。
    CFRelease(outData);
    CFAllocatorDeallocate(NULL, buffer);
 
    // Enter the run loop. 进入运行循环。
    CFRunLoopRun();
}
```
&emsp;进入运行循环后，所有将来发送到线程端口的事件都将由 `ProcessClientRequest` 函数处理。该函数的实现取决于线程执行的工作类型，此处未显示。

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
