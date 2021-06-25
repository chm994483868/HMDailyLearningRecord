# iOS 从源码解析Run Loop (五)：NSPort、TSD 相关内容解析

> &emsp;Port 相关的内容不知道如何入手学习，那么就从 NSPort 开始吧。Port 相关的内容是极其重要的，source1（port-based input sources） 以及 run loop 的唤醒相关的内容都是通过端口来实现的，不要焦虑不要浮躁静下心来，死磕下去！！⛽️⛽️

&emsp; Cocoa Foundation 为 iOS 线程间通信提供 2 种方式，1 种是 performSelector，另 1 种是 Port。performSelector 在前面文章我们已经详细学习过，这里只看第二种：NSMachPort 方式。NSPort 有 3 个子类，NSSocketPort、NSMessagePort、NSMachPort，但在 iOS 下只有 NSMachPort 可用。使用的方式为接收线程中注册 NSMachPort，在另外的线程中使用此 port 发送消息，则被注册线程会收到相应消息，然后最终在主线程里调用某个回调函数（handleMachMessage:/handlePortMessage:）。可以看到，使用 NSMachPort 的结果为调用了其它线程的 1 个函数，而这也正是 performSelector 所做的事情。

## NSMachPort 使用
&emsp;下面看一段 NSMachPort 的实例代码：（也可以先看下面的 NSPort 相关的文档然后再回过头来看此处的使用示例）
```c++
// ViewController 遵循 NSMachPortDelegate 协议

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    NSLog(@"🔞 START: %@", [NSThread currentThread]);

    // 1. 创建 NSMachPort 对象，并把当前 vc 设置为它的 delegate
    NSMachPort *port = [[NSMachPort alloc] init];
    [port setDelegate:self];
    
    // 2. 把 NSMachPort 对象添加到主线程的 run loop 的 NSRunLoopCommonModes 模式下
    [[NSRunLoop currentRunLoop] addPort:port forMode:NSRunLoopCommonModes];
    NSLog(@"🙀🙀 %@", port);
    
    // 3. 开辟子线程并把 main run loop 的 NSMachPort 对象作为参数传递，下面需要在子线程中使用 NSMachPort 对象向主线程的 run loop 发送消息，
    //   （当主线程的 run loop 收到消息时调用下面的 handleMachMessage: 代理方法，这样子线程就通过 NSMachPort 对象完成了与主线程的通信）
    [NSThread detachNewThreadSelector:@selector(customThread:) toTarget:self withObject:port];
    
    NSLog(@"🔞 END: %@", [NSThread currentThread]);
}

// 4. 在当前 VC 实现 NSMachPortDelegate 协议的委托方法
- (void)handleMachMessage:(void *)msg {
    NSLog(@"📢📢 Mach port %s", msg);
}

// 也可以用 handlePortMessage: 委托函数，这样则是把 mach msg 包装成 NSPortMessage，
// 如果两个委托函数都实现的话则只执行 handleMachMessage:，
// NSMachPortDelegate 协议是继承自 NSPortDelegate协议的，
// handlePortMessage: 是 NSPortDelegate 协议下的可选委托函数，
// handleMachMessage: 是 NSMachPortDelegate 协议下的可选委托函数。

//- (void)handlePortMessage:(NSPortMessage *)message {
//    NSLog(@"📢📢 Mach port %@", message);
//}

- (void)customThread:(NSMachPort *)sender {
    NSLog(@"😻😻 %@", sender);
    
    NSMachPort *p = [[NSMachPort alloc] init];
    
    // 5. 通过和主线程关联的 NSMachPort 对象向主线程的 run loop 发送消息
    [sender sendBeforeDate:[NSDate distantFuture] components:nil from:p reserved:0];
    
    NSLog(@"🤏🤏 subthread=%@", [NSThread currentThread]);
}

// 控制台打印:
🔞 START: <NSThread: 0x6000022883c0>{number = 1, name = main}
🙀🙀 <NSMachPort: 0x600000098210>
🔞 END: <NSThread: 0x6000022883c0>{number = 1, name = main}
😻😻 <NSMachPort: 0x600000098210>
🤏🤏 subthread=<NSThread: 0x6000022c6500>{number = 6, name = (null)}
📢📢 Mach port 
```
&emsp;MachPort 的工作方式其实是将 NSMachPort 对象添加到一个线程所对应的 run loop 中，并给 NSMachPort 对象设置相应的代理。在其他线程中调用该 MachPort 对象发消息时会在 MachPort 所关联的线程中执行相关的代理方法。

&emsp;[iOS开发之线程间的MachPort通信与子线程中的Notification转发](https://www.cnblogs.com/ludashi/p/7460907.html) 大佬的文章描述了一个使用 NSMachPort 在线程间通信的场景。
```c++
- (void)viewDidLoad {
    [super viewDidLoad];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(customNotify) name:@"NOMO" object:nil];
    NSLog(@"✉️✉️: %@", [NSThread currentThread]);
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        NSLog(@"✉️✉️ SEND: %@", [NSThread currentThread]);
        [[NSNotificationCenter defaultCenter] postNotificationName:@"NOMO" object:nil];
    });
}

- (void)customNotify {
    NSLog(@"✉️✉️ RECEIVE: %@", [NSThread currentThread]);
}

// 控制台打印：
✉️✉️: <NSThread: 0x283379a40>{number = 1, name = main}
✉️✉️ SEND: <NSThread: 0x28331ed40>{number = 5, name = (null)}
✉️✉️ RECEIVE: <NSThread: 0x28331ed40>{number = 5, name = (null)}
```
&emsp;看大佬的文章时学到一个新知识点，就是上面的示例代码中，虽然是在主线程中添加的观察者，但是如果在子线程中发出通知，那么就在该子线程中处理通知所关联的方法。[Delivering Notifications To Particular Threads](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/Notifications/Articles/Threading.html#//apple_ref/doc/uid/20001289-CEGJFDFG)

## NSPort（官方文档翻译）
&emsp;`NSPort` 表示通信通道（communication channel）的抽象类。
```c++
@interface NSPort : NSObject <NSCopying, NSCoding>
```
&emsp;通信发生在 NSPort 对象之间，这些对象通常位于不同的线程或任务中。分布式对象系统（distributed objects system）使用 NSPort 对象来回发送 NSPortMessage（定义在 macOS 中） 对象。尽可能使用分布式对象（distributed objects）实现应用程序间通信（interapplication communication），并且仅在必要时使用 NSPort 对象。

&emsp;要接收传入的消息，必须将 NSPort 对象作为 input sources 添加到 NSRunLoop 对象中。 NSConnection 对象在初始化时会自动添加其接收端口（receive port）。

&emsp;当 NSPort 对象接收到端口消息时，它将消息通过 `handleMachMessage:` 或 `handlePortMessage:` 消息转发给其 delegate。delegate 应仅实现这些方法中的一种，以所需的任何形式处理传入的消息。`handleMachMessage:` 提供以 msg_header_t 结构开头的 "原始 Mach 消息" 的消息。`handlePortMessage:` 将消息作为 NSPortMessage 对象提供，它是 Mach 消息的面向对象封装。如果尚未设置委托，NSPort 对象将处理消息本身。

&emsp;使用完端口对象后，必须先显式地使端口对象无效，然后再向其发送释放消息。类似地，如果应用程序使用垃圾回收，则必须在删除对端口对象的任何强引用之前使其无效。如果不使端口无效，则生成的端口对象可能会延迟并导致内存泄漏。要使端口对象无效，请调用其 `invalidate` 方法。

&emsp;Foundation 定义了 NSPort 的三个具体子类。NSMachPort 和 NSMessagePort 只允许本地（在同一台机器上）通信。NSSocketPort 允许本地和远程通信，但对于本地情况，可能比其他端口更昂贵。使用 `allocWithZone:` 或 `port` 创建 `NSPort` 对象时，将改为创建 NSMachPort 对象。

&emsp;NSPort 符合 NSCoding 协议，但只支持 NSPortCoder 进行编码。NSPort 及其子类不支持 archiving。

### allocWithZone:
&emsp;返回 NSMachPort 类的实例。
```c++
+ (id)allocWithZone:(NSZone *)zone
```
&emsp;`zone`：要在其中分配新对象的内存区域。

&emsp;为了 Mach 上的向后兼容性，`allocWithZone:` 在发送到 NSPort 类时返回 NSMachPort 类的实例。否则，它将返回一个具体子类的实例，该实例可用于本地计算机上的线程或进程之间的消息传递，或者在 NSSocketPort 的情况下，在不同计算机上的进程之间进行消息传递。

### port
&emsp;创建并返回一个可以发送和接收消息的新 NSPort 对象。
```c++
+ (NSPort *)port;
```
### invalidate
&emsp;将 receiver 标记为无效，并向默认通知中心发布 NSPortDidBecomeInvalidNotification。（即发送一个 NSPortDidBecomeInvalidNotification 通知）
```c++
- (void)invalidate;
```
&emsp;必须在释放端口对象之前调用此方法（如果应用程序被垃圾回收，则删除对该对象的强引用）。
### valid
&emsp;指示 receiver 是否有效的布尔值。
```c++
@property (readonly, getter=isValid) BOOL valid;
```
&emsp;如果已知 receiver 无效，则为 NO，否则为 YES。NSPort 对象在其依赖于操作系统的底层通信资源关闭或损坏时变为无效。
### setDelegate:
&emsp;将 receiver’s delegate 设置为指定对象。
```c++
// NSPortDelegate 协议仅有一个可选的 - (void)handlePortMessage:(NSPortMessage *)message; 方法
- (void)setDelegate:(nullable id <NSPortDelegate>)anObject;
```
### delegate
&emsp;返回 receiver’s delegate，可能为 NULL。
```c++
- (nullable id <NSPortDelegate>)delegate;
```
### scheduleInRunLoop:forMode:
&emsp;这个方法应该由一个子类来实现，当在给定的 input mode（NSRunLoopMode）下添加到给定的 run loop 中时，它可以设置对端口的监视。
```c++
- (void)scheduleInRunLoop:(NSRunLoop *)runLoop forMode:(NSRunLoopMode)mode;
```
&emsp;不应直接调用此方法。

### removeFromRunLoop:forMode:
&emsp;这个方法应该由一个子类来实现，当在给定的 input mode（NSRunLoopMode）下从给定的 run loop 中删除时，停止对端口的监视。
```c++
- (void)removeFromRunLoop:(NSRunLoop *)runLoop forMode:(NSRunLoopMode)mode;
```
&emsp;不应直接调用此方法。
### reservedSpaceLength
&emsp;receiver 为发送数据而保留的空间字节数。默认长度为 0。
```c++
@property (readonly) NSUInteger reservedSpaceLength;
```
### sendBeforeDate:components:from:reserved:
&emsp;此方法是为具有自定义 NSPort 类型的子类提供的。（NSConnection）
```c++
- (BOOL)sendBeforeDate:(NSDate *)limitDate
            components:(nullable NSMutableArray *)components 
                  from:(nullable NSPort *) receivePort
              reserved:(NSUInteger)headerSpaceReserved;
```
&emsp;`limitDate`：消息发送的最后时刻。

&emsp;`components`：消息组件。

&emsp;`receivePort`：接收端口。

&emsp;`headerSpaceReserved`：为 header 保留的字节数。

&emsp;NSConnection 在适当的时间调用此方法。不应直接调用此方法。此方法可能引发 NSInvalidSendPortException、NSInvalidReceivePortException 或 NSPortSendException，具体取决于发送端口的类型和错误的类型。

### sendBeforeDate:msgid:components:from:reserved:
&emsp;此方法是为具有自定义 NSPort 类型的子类提供的。（NSConnection）
```c++
- (BOOL)sendBeforeDate:(NSDate *)limitDate 
                 msgid:(NSUInteger)msgID 
            components:(NSMutableArray *)components 
                  from:(NSPort *)receivePort 
              reserved:(NSUInteger)headerSpaceReserved;
```
&emsp;`msgID`：message ID。

&emsp;NSConnection 在适当的时间调用此方法。不应直接调用此方法。此方法可能引发 NSInvalidSendPortException、NSInvalidReceivePortException 或 NSPortSendException，具体取决于发送端口的类型和错误的类型。

&emsp;`components` 数组由一系列 NSData 子类的实例和一些NSPort子类的实例组成。由于 NSPort 的一个子类不一定知道如何传输 NSPort 的另一个子类的实例（即使知道另一个子类也可以做到），因此，`components` 数组中的所有 NSPort 实例和 `receivePort` 参数必须属于接收此消息的 NSPort 的同一子类。如果在同一程序中使用了多个 DO transports，则需要格外小心。

### NSPortDidBecomeInvalidNotification
&emsp;从 `invalidate` 方法发布，当解除分配 NSPort 或它发现其通信通道已损坏时调用该方法。通知对象是无效的 NSPort 对象。此通知不包含 userInfo 字典。
```c++
FOUNDATION_EXPORT NSNotificationName const NSPortDidBecomeInvalidNotification;
```
&emsp;NSSocketPort 对象无法检测到其与远程端口的连接何时丢失，即使远程端口位于同一台计算机上。因此，它不能使自己失效并发布此通知。相反，你必须在发送下一条消息时检测超时错误。

&emsp;发布此通知的 NSPort 对象不再有用，因此所有接收者都应该注销自己的任何涉及 NSPort 的通知。接收此通知的方法应在尝试执行任何操作之前检查哪个端口无效。特别是，接收所有 NSPortDidBecomeInvalidNotification 消息的观察者应该知道，与 window server 的通信是通过 NSPort 处理的。如果此端口无效，drawing operations 将导致致命错误。

## NSPortDelegate
&emsp;用于处理传入消息的接口。NSPortDelegate 协议定义了由 NSPort 对象的 delegates 实现的可选方法。

### handlePortMessage:
&emsp;处理端口上的给定传入消息。
```c++
@protocol NSPortDelegate <NSObject>
@optional

- (void)handlePortMessage:(NSPortMessage *)message;
    // This is the delegate method that subclasses should send to their delegates, 
    // unless the subclass has something more specific that it wants to try to send first.
    // 这是子类应该发送给其 delegates 的 delegate method，除非子类有更具体的东西要首先尝试发送。
@end
```
&emsp;delegate 应实现 `handlePortMessage:` 或 NSMachPortDelegate 协议方法 `handleMachMessage:`。你不能同时实现两个委托方法。

## NSMachPortDelegate
&emsp;用于处理传入的 Mach 消息的接口。NSMachPort 对象的可以选择遵循此协议。

### handleMachMessage:
&emsp;处理传入的 Mach 消息。
```c++
@protocol NSMachPortDelegate <NSPortDelegate> // 继承自 NSPortDelegate
@optional

// Delegates are sent this if they respond, otherwise they are sent handlePortMessage:; argument is the raw Mach message.
// 如果他们响应，则向 delegates 发送此消息，否则，将向他们发送 handlePortMessage: ;。参数是原始 Mach 消息。
- (void)handleMachMessage:(void *)msg;
@end
```
&emsp;`msg`：指向 Mach 消息的指针，转换为指向 void 的指针。

&emsp;delegate 应将此数据解释为指向以 msg_header_t 结构开头的 Mach 消息的指针，并应适当地处理该消息。

&emsp;delegate 应实现 `handleMachMessage:` 或 NSPortDelegate 协议方法 `handlePortMessage:`。

## NSMachPort
&emsp;可以用作分布式对象连接（distributed object connections）（或原始消息传递）端点的端口。
```c++
@interface NSMachPort : NSPort {
    @private
    id _delegate;
    NSUInteger _flags;
    uint32_t _machPort;
    NSUInteger _reserved;
}
```
&emsp;NSMachPort 是 NSPort 的一个子类，它封装了 Mach 端口，macOS 中的基本通信端口。NSMachPort 只允许本地（在同一台机器上）通信。附带类 NSSocketPort 允许本地和远程分布式对象通信，但是对于本地情况，可能比 NSMachPort 更昂贵。

&emsp;要有效地使用 NSMachPort，你应该熟悉 Mach 端口、端口访问权限和 Mach 消息。有关更多信息，可参阅 Mach OS 文档。

&emsp;NSMachPort 符合 NSCoding 协议，但只支持 NSPortCoder 进行编码。NSPort 及其子类不支持 archiving。

### portWithMachPort:
&emsp;创建并返回一个用给定 Mach 端口配置的端口对象。
```c++
+ (NSPort *)portWithMachPort:(uint32_t)machPort;
```
&emsp;`machPort`：新端口的 Mach 端口。此参数原始应为 mach_port_t 类型。

&emsp;返回值是使用 machPort 发送或接收消息的 NSMachPort 对象。

&emsp;如果需要，创建端口对象。根据与 machPort 相关联的访问权限，新端口对象可能仅用于发送消息。
### NSMachPortOptions
&emsp;当 NSMachPort 对象无效或销毁时，用于移除对 Mach 端口的访问权限。
```c++
typedef NS_OPTIONS(NSUInteger, NSMachPortOptions) {
    NSMachPortDeallocateNone = 0,
    NSMachPortDeallocateSendRight = (1UL << 0),
    NSMachPortDeallocateReceiveRight = (1UL << 1)
} API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
```
+ `NSMachPortDeallocateNone` 不删除任何发送或接收权限。
+ `NSMachPortDeallocateReceiveRight` NSMachPort 对象无效或销毁时，删除接收权限。
+ `NSMachPortDeallocateSendRight` NSMachPort 对象无效或销毁时，取消分配发送权限。
### portWithMachPort:options:
&emsp;创建并返回配置有指定选项和给定 Mach 端口的端口对象。
```c++
+ (NSPort *)portWithMachPort:(uint32_t)machPort options:(NSMachPortOptions)f API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
```
&emsp;`machPort`：新端口的 Mach 端口。此参数原始应为 mach_port_t 类型。

&emsp;`options`：指定在 NSMachPort 对象无效（invalidated）或销毁（destroyed）时如何处理基础端口权限（underlying port rights）的选项。有关常量的列表，参见 Mach Port Rights。

&emsp;返回值是使用 machPort 发送或接收消息的 NSMachPort 对象。

&emsp;如有必要，创建端口对象。根据与 machPort 相关的访问权限，新的端口对象可能仅可用于发送消息。
### initWithMachPort:
&emsp;使用给定的 Mach 端口初始化新分配的 NSMachPort 对象。
```c++
- (instancetype)initWithMachPort:(uint32_t)machPort NS_DESIGNATED_INITIALIZER;
```
&emsp;`machPort`：新端口的 Mach 端口。此参数原始应为 mach_port_t 类型。

&emsp;返回一个初始化的 NSMachPort 对象，该对象使用 machPort 发送或接收消息。返回的对象可能与原始 receiver 不同。

&emsp;根据 machPort 的访问权限，新端口可能只能发送消息。如果已经存在具有 machPort 的端口，此方法将释放接收者（deallocates the receiver ），然后保留并返回那个具有 machPort 的端口。此方法是 NSMachPort 类的指定初始化器。
### initWithMachPort:options:
&emsp;使用给定的 Mach 端口和指定的选项初始化新分配的 NSMachPort 对象。
```c++
- (instancetype)initWithMachPort:(uint32_t)machPort
                         options:(NSMachPortOptions)f API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0)) NS_DESIGNATED_INITIALIZER;
```
&emsp;同上。
### machPort
&emsp;receiver 使用的 Mach 端口，以整数表示。
```c++
@property (readonly) uint32_t machPort;
```
&emsp;receiver 使用的 Mach 端口。与 Mach 系统调用一起使用时，将此值转换为 mach_port_t。
### removeFromRunLoop:forMode:
&emsp;将 receiver 从 runLoop 的 run loop mode 下删除。
```c++
- (void)removeFromRunLoop:(NSRunLoop *)runLoop forMode:(NSRunLoopMode)mode;
```
&emsp;移除 receiver 后，run loop 将停止监视 Mach 端口是否有传入消息。
### scheduleInRunLoop:forMode:
&emsp;将 receiver schedules 在 runLoop 的指定 run loop mode 下。
```c++
- (void)scheduleInRunLoop:(NSRunLoop *)runLoop forMode:(NSRunLoopMode)mode;
```
&emsp;scheduled 完成 receiver 后，run loop 将监视 Mach 端口中是否有传入消息，并且当消息到达时，将调用委托方法 `handleMachMessage:`。

&emsp;（如果你将 NSMachPort 子类化，则必须从 NSPort override（重载）这 2 种方法；由于这很复杂，因此不建议对 NSMachPort 进行子类化。）
### delegate/setDelegate:
&emsp;读取和设置 receiver 的 delegate。
```c++
- (void)setDelegate:(nullable id <NSMachPortDelegate>)anObject;
- (nullable id <NSMachPortDelegate>)delegate;
```
## NSMessagePort
&emsp;可用作分布式对象连接（distributed object connections）（或原始消息传递）端点的端口。NSPort 的子类，可用于在所有平台上发送本地消息。
```c++
@interface NSMessagePort : NSPort {
    @private
    void *_port;
    id _delegate;
}

@end
```
&emsp;NSMessagePort 是 NSPort 的子类，仅允许本地（在同一台计算机上）通信。伴随类 NSSocketPort 允许本地和远程通信，但在本地情况下可能比 NSMessagePort 昂贵。

&emsp;除了 NSPort 已定义的方法外，NSMessagePort 不会定义其他方法。

&emsp;NSMessagePort 符合 NSCoding 协议，但仅支持通过 NSPortCoder 对象进行编码。 NSPort 及其子类不支持 archiving。

&emsp;避免 NSMessagePort。没有理由使用 NSMessagePort 而不是 NSMachPort 或 NSSocketPort。没有特别的性能或功能优势，建议避免使用。在 macOS 10.6 或更高版本中可能会废弃 NSMessagePort。
## NSSocketPort
&emsp;代表 BSD socket 的端口。NSPort 的子类，可用于在所有平台上进行远程消息发送。
```c++
@interface NSSocketPort : NSPort {
    @private
    void *_receiver;
    id _connectors;
    void *_loops;
    void *_data;
    id _signature;
    id _delegate;
    id _lock;
    NSUInteger _maxSize;
    NSUInteger _useCount;
    NSUInteger _reserved;
}
```
&emsp;NSSocketPort 对象可以用作分布式对象连接（distributed object connections）的端点。伴随类 NSMachPort 和 NSMessagePort 只允许本地（在同一台机器上）通信。NSSocketPort 类允许本地和远程通信，但对于本地情况，可能比其他类（NSMachPort 和 NSMessagePort）更昂贵。

&emsp;NSSocketPort 类符合 NSCoding 协议，但仅支持通过 NSPortCoder 进行编码。 NSPort 及其其他子类不支持 archiving。
### init
&emsp;将 receiver 初始化为 SOCK_STREAM 类型的本地 TCP/IP socket。
```c++
- (instancetype)init;
```
&emsp;类型为 SOCK_STREAM 的已初始化本地 TCP/IP socket 端口。

&emsp;端口号由系统选择。

### initWithTCPPort:
&emsp;将 receiver 初始化为 SOCK_STREAM 类型的本地 TCP/IP socket，监听指定的端口号。
```c++
- (nullable instancetype)initWithTCPPort:(unsigned short)port;
```
&emsp;`port`：新创建的监听 socket 端口的端口号。如果 port 为 0，系统将分配一个端口号。

&emsp;监听端口 port 的 SOCK_STREAM 类型的已初始化本地 TCP/IP socket。

&emsp;此方法创建一个 IPv4 端口，而不是 IPv6 端口。
### initWithProtocolFamily:socketType:protocol:address:
&emsp;使用提供的参数将 receiver 初始化为本地 socket。
```c++
- (nullable instancetype)initWithProtocolFamily:(int)family
                                     socketType:(int)type
                                       protocol:(int)protocol
                                        address:(NSData *)address NS_DESIGNATED_INITIALIZER;
```
&emsp;`family`：socket 端口的协议族。可能的值在 <sys/socket.h> 中定义，例如 AF_LOCAL、AF_INET 和 AF_INET6。

&emsp;`type`：socket 的类型。

&emsp;`protocol`：协议族中要使用的特定协议。

&emsp;`address`：The family-specific socket address for the receiver copied into an NSData object.

&emsp;必须先将 receiver 添加到 run loop，然后才能接受连接（connections）或接收消息（messages）。传入的消息将传递到 receiver 的委托方法 `handlePortMessage:`。要创建标准的 TCP/IP socket，使用 `initWithTCPPort:`。
### initWithProtocolFamily:socketType:protocol:socket:
&emsp;用先前创建的本地 socket 初始化 receiver。
```c++
- (nullable instancetype)initWithProtocolFamily:(int)family
                                     socketType:(int)type
                                       protocol:(int)protocol
                                         socket:(NSSocketNativeHandle)sock NS_DESIGNATED_INITIALIZER;
```
&emsp;`family`：socket 端口的协议族。可能的值在 <sys/socket.h> 中定义，例如 AF_LOCAL、AF_INET 和 AF_INET6。

&emsp;`type`：提供的 socket 的类型。

&emsp;`protocol`：提供的 socket 使用的特定 protocol。

&emsp;`sock`：先前创建的 socket。
### initRemoteWithTCPPort:host:
&emsp;将 receiver 初始化为 SOCK_STREAM 类型的 TCP/IP socket，该 socket 可以连接到指定端口上的 remote host。
```c++
- (nullable instancetype)initRemoteWithTCPPort:(unsigned short)port
                                          host:(nullable NSString *)hostName;
```
&emsp;`port`：要连接的 port。

&emsp;`hostName`：要连接的 host name。 hostName 可以是主机名或 IPv4 样式的地址。

&emsp;发送数据之前，不会打开与远程主机（remote host）的连接。
### initRemoteWithProtocolFamily:socketType:protocol:address:
&emsp;使用提供的参数将 receiver 初始化为 remote socket。
```c++
- (instancetype)initRemoteWithProtocolFamily:(int)family
                                  socketType:(int)type
                                    protocol:(int)protocol
                                     address:(NSData *)address NS_DESIGNATED_INITIALIZER;
```
&emsp;`family`：socket 端口的协议族。可能的值在 <sys/socket.h> 中定义，例如 AF_LOCAL、AF_INET 和 AF_INET6。

&emsp;`type`：socket 类型。

&emsp;`protocol`：协议族中要使用的特定协议。

&emsp;`address`：The family-specific socket address for the receiver copied into an NSData object.

&emsp;发送数据之前，不会打开与远程地址（remote address）的连接。
### protocolFamily
&emsp;receiver 用于通信的协议族。
```c++
@property (readonly) int protocolFamily;
```
&emsp;可能的值在 <sys/socket.h> 中定义，例如 AF_LOCAL、AF_INET 和 AF_INET6。
### socketType
&emsp;receiver 的 socket 类型。
```c++
@property (readonly) int socketType;
```
### protocol
&emsp;receiver 用于通信的协议。
```c++
@property (readonly) int protocol;
```
### address
&emsp;receiver 的 socket 地址结构存储在 NSData 对象中。
```c++
@property (readonly, copy) NSData *address;
```
### socket
&emsp;The receiver’s native socket identifier on the platform. 在 macOS 中，本机套接字标识符（native socket identifier）是完整文件描述符。
```c++
typedef int NSSocketNativeHandle;

@property (readonly) NSSocketNativeHandle socket;
```
## NSPortMessage
&emsp;一个低级的、与操作系统无关的类型用于描述应用程序间（和线程间）的消息。（仅定义在 macOS 下） 
```c++
@interface NSPortMessage : NSObject
```
&emsp;端口消息主要由分布式对象系统（distributed objects system）使用。你应尽可能使用分布式对象来实现应用程序间通信，并仅在必要时使用 NSPortMessage。
&emsp;NSPortMessage 对象有三个主要部分：发送和接收端口，它们是将消息的发送方链接到接收方的 NSPort 对象，组成消息主体的组件。组件作为包含 NSData 和 NSPort 对象的 NSArray 对象保存。`sendBeforeDate:` 消息通过发送端口发送组件；对消息的任何答复都到达接收端口。有关处理传入消息的信息，参阅 NSPort 类。

&emsp;NSPortMessage 实例可以用一对 NSPort 对象和一组组件初始化。端口消息的正文只能包含 NSPort 对象或 NSData 对象。在分布式对象系统中，byte/character 数组通常被编码为 NSInvocation 对象，从代理转发到相应的真实对象。

&emsp;NSPortMessage 对象还维护一个消息标识符，该标识符可用于指示消息的类别，例如，Objective-C 方法调用、连接请求、错误等。使用 msgid 和 msgid 方法访问标识符。
### initWithSendPort:receivePort:components:
&emsp;初始化新分配的 NSPortMessage 对象，以在给定端口上发送给定数据，并在另一个给定端口上发送接收方应答。
```c++
- (instancetype)initWithSendPort:(NSPort *)sendPort
                     receivePort:(NSPort *)replyPort
                      components:(NSArray *)components;
```
&emsp;`sendPort`：在其上发送消息的端口。

&emsp;`receivePort`：消息答复到达的端口。

&emsp;`components`：消息中要发送的数据。components 应仅包含 NSData 和 NSPort 对象，并且 NSData 对象的内容应按网络字节顺序排列。

&emsp;返回值初始化为 NSPortMessage 对象，以在 sendPort 上发送组件，并在 receivePort 上向接收者回复。

&emsp;使用此方法初始化的 NSPortMessage 对象的消息标识符为 0，这是 NSPortMessage 的指定初始化程序。
### sendBeforeDate:
&emsp;尝试在 date 之前发送消息，如果成功则返回 YES，如果操作超时则返回 NO。
```c++
- (BOOL)sendBeforeDate:(NSDate *)date;
```
&emsp;`date`：消息应在其之前发送的瞬间。

&emsp;如果操作成功，则为是，否则为否（例如，如果操作超时）。

&emsp;如果发生超时以外的错误，则此方法可能会引发 NSInvalidSendPortException、NSInvalidReceivePortException 或 NSPortSendException，具体取决于发送端口的类型和错误的类型。

&emsp;如果无法立即发送消息，则发送线程将阻塞，直到消息被发送或到达 date。
已发送的消息则排队，以最大程度地减少阻塞，但是如果将多条消息发送到端口的速度快于端口所有者接收消息的速度，则可能会失败，导致队列填满。因此，为 date 选择一个值，该值为发送下一条消息之前有足够的时间来处理该消息。有关接收端口消息的信息，参阅 NSPort 类。
#### components
&emsp;返回 receiver 的数据分量。
```c++
@property(readonly, copy) NSArray *components;
```
&emsp;receiver 的数据组件。
#### receivePort
&emsp;对于传出消息，返回接收方的答复将到达的端口。对于传入消息，返回 receiver 到达的端口。
```c++
@property(readonly, retain) NSPort *receivePort;
```
#### sendPort
&emsp;对于传出消息，返回接收方将通过的端口。对于传入的消息，返回应发送给接收方的端口回复。
```c++
@property(readonly, retain) NSPort *sendPort;
```
&emsp;对于传出消息，接收方在接收到 `sendBeforeDate:` 消息时将通过自身发送的端口。对于传入消息，应将发送给接收方的端口答复发送出去。
#### msgid
&emsp;返回 receiver 的标识符。
```c++
@property uint32_t msgid;
```
&emsp;Cooperating applications 可以使用它来定义不同类型的消息，例如连接请求，RPCs，错误等。
## NSConnection
&emsp;管理不同线程中的对象之间或线程与本地或远程系统上运行的进程之间通信的对象。
```c++
@interface NSConnection : NSObject
```
&emsp;Connection 对象构成了分布式对象机制的骨干，通常在后台运行。在将对象 vending 给其他应用程序时，通过代理访问此类 vended 对象以及更改默认通信参数时，可以显式使用 NSConnection 的方法。在其他时候，你只需与 vended 对象或其代理进行交互。单个连接对象可以由多个线程共享，并用于访问 vended 对象。

## Thread Local Data
&emsp;在前面 pthreads 的学习过程中我们有学到两个比较特殊的 API：
```c++
__API_AVAILABLE(macos(10.4), ios(2.0))
int pthread_setspecific(pthread_key_t , const void * _Nullable); 

__API_AVAILABLE(macos(10.4), ios(2.0))
void* _Nullable pthread_getspecific(pthread_key_t);
```
&emsp;`pthread_getspecific` 和 `pthread_setspecific` 这两个接口分别用于获取和设置线程本地存储区的数据，在不同的线程下相同的 `pthread_key_t` 读取的结果是不同的，即线程的本地存储空间是相互隔离的，这也是线程本地存储的关键所在。

&emsp;（这里还有一个隐藏点，我们不能以面向对象的思想看待这两个接口，调用 `pthread_getspecific` 和 `pthread_setspecific` 时我们是不需要传入 pthread_t 对象的，如果我们想要在某条线程内读取其存储空间的数据，那么我们只能在当前线程内执行 `pthread_getspecific` 函数，存储同理，即我们想要操作哪条线程，那么我们只能在哪条线程内执行操作。）

&emsp;在前面的学习中我们多次用到 TSD 技术，例如自动释放池、autorelease 优化等等，在 run loop 的学习过程中我们又与 TSD 再次相遇。在 `CFRunLoopGetMain` 和 `CFRunLoopGetCurrent` 两个函数学习过程中，看到线程对应的 run loop 会被 “直接” 保存在线程的 TSD 中。在 CFPlatform.c 文件中我们直接翻看其源码，看到 run loop 中使用 TSD 时又进行了一次 “封装”，emmm...大概可以理解为又包装了一下，前面我们使用到 TSD 时都是直接类似以 Key-Value 的形式存储数据，这里则又提供了一个中间数据结构 struct __CFTSDTable，构建一个  \__CFTSDTable 实例然后以 CF_TSD_KEY 为 Key 把 \__CFTSDTable 实例 保存在 TSD 中，然后在 \__CFTSDTable 实例以数组形式保存数据，如 run loop 对象、run loop 对象的销毁函数等等数据，下面我们先看一下源码然后再看在 run loop 中的应用。

&emsp;（CFPlatform.c 文件 Thread Local Data 块中包含 WINDOWS、MACOSX、LINUX 平台的代码，这里我们只看 MACOSX 下的实现。）

### CF_TSD_MAX_SLOTS
&emsp;由于 run loop 对象要保存在线程 TSD 中的数据不多，所以这里直接限制了 \__CFTSDTable 结构体内部数组长度为 70。
```c++
// 如果 slot >= CF_TSD_MAX_SLOTS，则 SPI 函数将在NULL + slot 地址处 crash。
// 如果线程数据已被删除，则这些功能应在 CF_TSD_BAD_PTR + slot 地址上崩溃。
#define CF_TSD_MAX_SLOTS 70
```

### CF_TSD_KEY
&emsp;\__CFTSDTable 实例在 TSD 中保存时的 pthread_key_t，一条线程中只会创建一个 \__CFTSDTable 实例。
```c++
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
static const unsigned long CF_TSD_KEY = __PTK_FRAMEWORK_COREFOUNDATION_KEY5;
#endif
```

### CF_TSD_BAD_PTR
```c++
#define CF_TSD_BAD_PTR ((void *)0x1000)
```

### \__CFTSDTable
&emsp;在 `__CFTSDTable` 结构体中保存数据以及每条数据对应的析构函数。
```c++
typedef void (*tsdDestructor)(void *);

// 数据结构保存 TSD 数据，每个都有清除功能
typedef struct __CFTSDTable {
    uint32_t destructorCount; // __CFTSDTable 实例进行析构的次数
    uintptr_t data[CF_TSD_MAX_SLOTS]; // 长度为 70 的 uintptr_t 数组，用来保存数据
    tsdDestructor destructors[CF_TSD_MAX_SLOTS]; // 长度为 70 的 tsdDestructor 数组，tsdDestructor 是 data 对应的析构函数指针
} __CFTSDTable;
```

### \__CFTSDSetSpecific/ \__CFTSDGetSpecific
&emsp;`__CFTSDSetSpecific` 函数的 `arg` 参数是 `__CFTSDTable` 实例以 CF_TSD_KEY 为 pthread_key_t 保存在线程的 TSD 中，`__CFTSDGetSpecific` 函数则是以 CF_TSD_KEY 为 pthread_key_t 从线程的 TSD 中读取 `__CFTSDTable` 实例。
```c++
static void __CFTSDSetSpecific(void *arg) {
    _pthread_setspecific_direct(CF_TSD_KEY, arg); // 把 arg 以 CF_TSD_KEY 为 key 保存在线程的 TSD 中
}
static void *__CFTSDGetSpecific() {
    return _pthread_getspecific_direct(CF_TSD_KEY); // 从线程的 TSD 中读取 CF_TSD_KEY 对应的数据
}
```

### \__CFTSDFinalize
&esmp;TSD 的销毁函数。
```c++
static void __CFTSDFinalize(void *arg) {
    // Set our TSD so we're called again by pthreads. 
    // It will call the destructor PTHREAD_DESTRUCTOR_ITERATIONS times as long as a value is set in the thread specific data. 
    // We handle each case below.
    // 设置我们的 TSD，以便 pthread 再次调用我们。只要在线程特定数据中设置了值，它将调用析构函数 PTHREAD_DESTRUCTOR_ITERATIONS 次。我们在下面处理每种情况。
    // PTHREAD_DESTRUCTOR_ITERATIONS 是线程退出时销毁其私有数据 TSD 的最大次数，在 x86_64 macOS/iOS 下打印其值都是 4。
    
    __CFTSDSetSpecific(arg);
    
    if (!arg || arg == CF_TSD_BAD_PTR) {
        // We've already been destroyed. The call above set the bad pointer again. Now we just return.
        // 我们已经被销毁了。上面的调用再次设置了错误的指针。现在我们返回。
        return;
    }
    
    // 强转为 __CFTSDTable
    __CFTSDTable *table = (__CFTSDTable *)arg;
    table->destructorCount++; // 析构次数自增
    
    // On first calls invoke destructor. Later we destroy the data.
    // Note that invocation of the destructor may cause a value to be set again in the per-thread data slots. 
    // The destructor count and destructors are preserved.
    // This logic is basically the same as what pthreads does. We just skip the 'created' flag.
    
    // 在第一次调用时，调用析构函数。稍后我们销毁数据。
    // 请注意，析构函数的调用可能导致在每个线程数据 slots 中再次设置一个值。
    // 析构函数计数和析构函数将保留。
    // 这种逻辑基本上与 pthreads 相同。我们只是跳过 'created' 标志。
    
    // 遍历 table 中的 data 数组，把 data 数组每个元素置为 NULL，并以每个 data 数组元素为参数执行 destructors 数组中对应的析构函数
    
    //（CF_PRIVATE void __CFFinalizeRunLoop(uintptr_t data) 可以理解是 run loop 对象的析构函数，data 参数理解为是 run loop 对象的引用计数，
    //  __CFFinalizeRunLoop 只要在当前线程执行那么就能以当前线程为 key 从 __CFRunLoops 全局字典中找到其对应的 run loop 对象，
    //  所以不同与我们的 OC/C++ 的实例函数，需要把对象作为参数传入释放函数，__CFFinalizeRunLoop 函数调用时不需要我们主动传入 run loop 对象）
    
    for (int32_t i = 0; i < CF_TSD_MAX_SLOTS; i++) {
        if (table->data[i] && table->destructors[i]) {
            uintptr_t old = table->data[i];
            table->data[i] = (uintptr_t)NULL;
            table->destructors[i]((void *)(old));
        }
    }
    
    // 如果 destructorCount 等于 PTHREAD_DESTRUCTOR_ITERATIONS - 1 则释放 table 的内存空间。
    // PTHREAD_DESTRUCTOR_ITERATIONS 是线程退出时销毁其私有数据 TSD 的最大次数，在 x86_64 macOS/iOS 下打印其值都是 4。
    if (table->destructorCount == PTHREAD_DESTRUCTOR_ITERATIONS - 1) {    // On PTHREAD_DESTRUCTOR_ITERATIONS-1 call, destroy our data
        free(table);
        
        // Now if the destructor is called again we will take the shortcut at the beginning of this function.
        // 现在，如果再次调用析构函数，我们将在此函数的开头使用快捷方式。
        
        // 把 CF_TSD_BAD_PTR 以 CF_TSD_KEY 为 pthread_key_t 保存在线程的 TSD 中
        __CFTSDSetSpecific(CF_TSD_BAD_PTR);
        
        return;
    }
}
```

### \__CFTSDGetTable
&emsp;从 TSD 中读取 \__CFTSDTable 实例，如果不存在则进行创建。 
```c++
// Get or initialize a thread local storage. It is created on demand.
// 获取或初始化线程本地存储。它是按需创建的。
static __CFTSDTable *__CFTSDGetTable() {
    // 读取 TSD 中的 __CFTSDTable 实例
    __CFTSDTable *table = (__CFTSDTable *)__CFTSDGetSpecific();
    
    // Make sure we're not setting data again after destruction.
    // 确保销毁后不再设置数据。
    if (table == CF_TSD_BAD_PTR) {
        return NULL;
    }
    
    // Create table on demand
    // 按需创建 __CFTSDTable 实例
    if (!table) {
        // This memory is freed in the finalize function
        // 该内存在 finalize 函数中释放
        
        // 创建 __CFTSDTable 实例
        table = (__CFTSDTable *)calloc(1, sizeof(__CFTSDTable));
        
        // Windows and Linux have created the table already, we need to initialize it here for other platforms. 
        // On Windows, the cleanup function is called by DllMain when a thread exits. On Linux the destructor is set at init time.
        // Windows 和 Linux 已经创建了该表，对于其他平台，我们需要在此处对其进行初始化。在 Windows 上，当线程退出时，DllMain 会调用 cleanup 函数。在 Linux上，析构函数在初始化时设置。
        
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
        // 当线程退出时以 CF_TSD_KEY 为 key 从 TSD 中读出的 __CFTSDTable 实例为参数调用 __CFTSDFinalize 函数
        pthread_key_init_np(CF_TSD_KEY, __CFTSDFinalize);
#endif
        // __CFTSDFinalize 保存在 TSD 中
        __CFTSDSetSpecific(table);
    }
    
    return table;
}
```
### \_CFGetTSD
&emsp;返回 \__CFTSDTable 实例 data 数组中  slot 下标的数据。
```c++
// For the use of CF and Foundation only
// 仅用于 CF 和 Foundation
CF_EXPORT void *_CFGetTSD(uint32_t slot) {
    // 如果 slot 大于 70 则 crash
    if (slot > CF_TSD_MAX_SLOTS) {
        _CFLogSimple(kCFLogLevelError, "Error: TSD slot %d out of range (get)", slot);
        HALT;
    }
    
    // 从 TSD 中读出 __CFTSDTable 
    __CFTSDTable *table = __CFTSDGetTable();
    
    // 如果 table 不存在则 crash
    if (!table) {
        // Someone is getting TSD during thread destruction. The table is gone, so we can't get any data anymore.
        // 线程销毁期间有人正在获取 TSD。该表已消失，因此我们无法再获取任何数据。
        
        _CFLogSimple(kCFLogLevelWarning, "Warning: TSD slot %d retrieved but the thread data has already been torn down.", slot);
        return NULL;
    }
    
    // 返回 table 的 data 数组中 slot 位置的数据
    uintptr_t *slots = (uintptr_t *)(table->data);
    return (void *)slots[slot];
}
```
### \_CFSetTSD
&emsp;把 newVal 放在 data 数组的 slot 下标处，把 destructor 放在 destructors 数组的 slot 下标处。
```c++
// For the use of CF and Foundation only
// 仅用于 CF 和 Foundation
CF_EXPORT void *_CFSetTSD(uint32_t slot, void *newVal, tsdDestructor destructor) {
    // slot 大于 70 则 crash
    if (slot > CF_TSD_MAX_SLOTS) {
        _CFLogSimple(kCFLogLevelError, "Error: TSD slot %d out of range (set)", slot);
        HALT;
    }
    
    // 读取 TSD 中的 __CFTSDTable
    __CFTSDTable *table = __CFTSDGetTable();
    
    // 如果 table 不存在
    if (!table) {
        // Someone is setting TSD during thread destruction. The table is gone, so we can't get any data anymore.
        // 有人在销毁线程的过程中设置了 TSD。该表已消失，因此我们无法再获取任何数据。
        _CFLogSimple(kCFLogLevelWarning, "Warning: TSD slot %d set but the thread data has already been torn down.", slot);
        return NULL;
    }
    
    // 取出 slot 之前的旧数据
    void *oldVal = (void *)table->data[slot];
    
    // 把 newVal 放在 data 的 slot 下标处
    table->data[slot] = (uintptr_t)newVal;
    // 把 destructor 放在 destructors 的 slot 下标处
    table->destructors[slot] = destructor;
    
    return oldVal;
}
```
&emsp;上面是 run loop 使用 TSD 时涉及的所有源码，下面我们看一下具体的应用。

## Run Loop 对象保存在 TSD 中
&emsp;在初次获取线程的 run loop 对象时，会把创建好的线程的 run loop 对象放进当前线程的 TSD 中：
```c++
enum {
    ...
    __CFTSDKeyRunLoop = 10,
    __CFTSDKeyRunLoopCntr = 11,
    ...
};

// _CFRunLoopGet0 函数摘录部分代码：
...
if (pthread_equal(t, pthread_self())) {
    _CFSetTSD(__CFTSDKeyRunLoop, (void *)loop, NULL);
    if (0 == _CFGetTSD(__CFTSDKeyRunLoopCntr)) {
    
        // PTHREAD_DESTRUCTOR_ITERATIONS 是线程退出时销毁其私有数据 TSD 的最大次数，在 x86_64 macOS/iOS 下打印其值都是 4，
        // 那么这里 __CFFinalizeRunLoop 析构函数对应的 data 数字就是 3。（不知道为什么是 3 ？虽然 __CFFinalizeRunLoop 函数调用时会有个 data - 1 的操作，
        // 但是线程和 run loop 是一一对应的，那么一次调用 __CFFinalizeRunLoop 函数把当前线程的 run loop 对象释放了不好吗？）
        _CFSetTSD(__CFTSDKeyRunLoopCntr, (void *)(PTHREAD_DESTRUCTOR_ITERATIONS-1), (void (*)(void *))__CFFinalizeRunLoop);
    }
}
...
```
&emsp;首先外层 if 判断当前调用 `_CFRunLoopGet0` 函数的线程是否和参数 t 是同一条线程，如果是的话则调用 `_CFSetTSD(__CFTSDKeyRunLoop, (void *)loop, NULL);` 函数把创建好的当前线程的 run loop 对象 loop 存储在当前线程 TSD 中 \__CFTSDTable 实例的 data 数组的  `__CFTSDKeyRunLoop`（10）索引处。

&emsp;`_CFSetTSD(__CFTSDKeyRunLoopCntr, (void *)(PTHREAD_DESTRUCTOR_ITERATIONS-1), (void (*)(void *))__CFFinalizeRunLoop);` 函数把 `PTHREAD_DESTRUCTOR_ITERATIONS-1` 存储在当前线程 TSD 中 \__CFTSDTable 实例的 data 数组的  `__CFTSDKeyRunLoopCntr`（11） 索引处，把 `__CFFinalizeRunLoop` 存储在当前线程 TSD 中 \__CFTSDTable 实例的 destructors 数组的  `__CFTSDKeyRunLoopCntr`（11） 索引处。`__CFFinalizeRunLoop` 函数是 run loop 对象的析构函数。

&emsp;看到 run loop 对象和 run loop 对象的析构函数都保存在了线程的 TSD 中。在 `__CFTSDGetTable` 函数中我们看到 `pthread_key_init_np(CF_TSD_KEY, __CFTSDFinalize);` 即线程销毁时会调用 `__CFTSDFinalize` 函数，而在 `__CFTSDFinalize` 函数内，则会遍历 \__CFTSDTable 实例中的数组（data、destructors 数组），把 data 数组每个元素置为 NULL，并以每个 data 数组元素为参数执行 destructors 数组中对应的析构函数。（那么 `__CFTSDKeyRunLoopCntr`（11） 索引处，调用 \__CFFinalizeRunLoop 函数则是：`__CFFinalizeRunLoop(3)`，好疑惑呀，为什么不是直接是 1 ？）

&emsp;`PTHREAD_DESTRUCTOR_ITERATIONS` 是线程退出时销毁其私有数据 TSD 的最大次数，在 x86_64 macOS/iOS 下打印其值都是 4。

### \__CFFinalizeRunLoop
&emsp;`CF_PRIVATE void __CFFinalizeRunLoop(uintptr_t data)` 可以理解是 run loop 对象的析构函数，data 参数大概可以理解为 run loop 对象的引用计数。

&emsp;\__CFFinalizeRunLoop 只要在当前线程执行那么就能以当前线程为 key（pthreadPointer(pthread_self())） 从 \__CFRunLoops 全局字典中找到其对应的 run loop 对象，所以不同与我们的 OC/C++ 的实例函数，需要把对象作为参数传入释放函数才能进行释放，\__CFFinalizeRunLoop 函数调用时只要是在当前线程执行就不需要我们主动传入 run loop 对象，其内部会自己找到 run loop 对象。

&emsp;全局搜索 `__CFTSDKeyRunLoopCntr` 看到只有三处 `_CFSetTSD` 函数的调用。
1. `_CFRunLoopGet0` 函数内当 `0 == _CFGetTSD(__CFTSDKeyRunLoopCntr)` 读到是 0 的时候调用:`_CFSetTSD(__CFTSDKeyRunLoopCntr, (void *)(PTHREAD_DESTRUCTOR_ITERATIONS-1), (void (*)(void *))__CFFinalizeRunLoop);`  把 data 数组对应的值设置为 3。
2. `__CFFinalizeRunLoop` 函数中如果 data 大于 1，则执行 `_CFSetTSD(__CFTSDKeyRunLoopCntr, (void *)(data - 1), (void (*)(void *))__CFFinalizeRunLoop);` 把 `__CFTSDTable` 中的 data 数组中  `__CFTSDKeyRunLoopCntr` 处的值减 1。
3. `_CFRunLoopSetCurrent` 函数中调用 `_CFSetTSD(__CFTSDKeyRunLoopCntr, 0, (void (*)(void *))__CFFinalizeRunLoop);` 更新为 0。

```c++
// Called for each thread as it exits
// 每个线程退出时调用

CF_PRIVATE void __CFFinalizeRunLoop(uintptr_t data) {
    CFRunLoopRef rl = NULL;
    
    if (data <= 1) {
        // 当 data 小于等于 1 开始执行销毁
        
        // static CFLock_t loopsLock = CFLockInit;
        // loopsLock 是一个全局的锁，执行加锁
        __CFLock(&loopsLock);
        
        // 从 __CFRunLoops 全局字典中读出当前线程的 run loop 对象
        if (__CFRunLoops) {
            // 以 pthreadPointer(pthread_self()) 为 key 读取 run loop
            rl = (CFRunLoopRef)CFDictionaryGetValue(__CFRunLoops, pthreadPointer(pthread_self()));
            
            // 这里的 retain 是为了下面继续使用 rl，这里从 __CFRunLoops 字典中移除 rl，它的引用计数会减 1
            if (rl) CFRetain(rl);
            CFDictionaryRemoveValue(__CFRunLoops, pthreadPointer(pthread_self()));
        }
        
        __CFUnlock(&loopsLock);
    } else {
        // 初始时是 PTHREAD_DESTRUCTOR_ITERATIONS-1 是 3，那么 __CFFinalizeRunLoop 函数需要调用两次减 1，才能真正的执行 run loop 对象的销毁工作 
        _CFSetTSD(__CFTSDKeyRunLoopCntr, (void *)(data - 1), (void (*)(void *))__CFFinalizeRunLoop);
    }
    // 这里的判断主线程的 run loop 是绝对不能销毁的，只能销毁子线程的 run loop，话说除了我们自己开辟的子线程外，系统会创建启动了 run loop 的子线程吗？
    if (rl && CFRunLoopGetMain() != rl) { // protect against cooperative threads
        // 如果 _counterpart 存在则进行释放
        if (NULL != rl->_counterpart) {
            CFRelease(rl->_counterpart);
            rl->_counterpart = NULL;
        }
        
        // purge all sources before deallocation
        // 在销毁 run loop 之前清除所有来源
        
        // 取得 mode 数组
        CFArrayRef array = CFRunLoopCopyAllModes(rl);
        
        // 遍历 mode 数组，移除 mode 中的所有 sources
        for (CFIndex idx = CFArrayGetCount(array); idx--;) {
            CFStringRef modeName = (CFStringRef)CFArrayGetValueAtIndex(array, idx);
            __CFRunLoopRemoveAllSources(rl, modeName);
        }
        
        // 移除 common mode 中的所有 sources
        __CFRunLoopRemoveAllSources(rl, kCFRunLoopCommonModes);
        CFRelease(array);
    }
    // 释放 rl
    if (rl) CFRelease(rl);
}
```
&emsp;销毁 run loop 对象之前，要先将其从 \__CFRunLoops 全局字典中移除，同时遍历其所有的 mode，依次移除每个 mode 中的所有 sources，最后销毁 run loop 对象。mode 销毁前同样也会释放所有的 mode item，前面已经讲解过这里就不复制粘贴了。

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
+ [iOS 线程通信 NSPort](http://blog.sina.com.cn/s/blog_7815a31f0101ea0n.html)
+ [iOS开发·RunLoop源码与用法完全解析(输入源，定时源，观察者，线程间通信，端口间通信，NSPort，NSMessagePort，NSMachPort，NSPortMessage)](https://cloud.tencent.com/developer/article/1332254)
+ [RunLoop NSMachPort 详解](https://blog.csdn.net/jeffasd/article/details/52027733)
+ [iOS-NSRunLoop编程详解](https://www.cnblogs.com/fanyiyao-980404514/p/4227536.html)
+ [iOS开发之线程间的MachPort通信与子线程中的Notification转发](https://www.cnblogs.com/ludashi/p/7460907.html)
