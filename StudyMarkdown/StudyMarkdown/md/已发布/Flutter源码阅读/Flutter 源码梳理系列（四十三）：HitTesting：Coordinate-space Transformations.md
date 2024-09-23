# Flutter 源码梳理系列（四十三）：HitTesting：Coordinate-space Transformations

# 前言

&emsp;在前面几篇中多次我们多次提到了坐标系的转换，即 RenderBox.hitTest 函数的参数：`{ required Offset position }` 是如何由父级 RenderBox 的坐标空间转换到子级 RenderBox 的坐标空间的，然后才能判断点击事件（的坐标 Offset position）是否位于子级 RenderBox 的空间范围内的（`if (size.contains(position)) { //... }`），那么这一过程是如何发生的？以及同一个点（`Offset position`）在不同坐标系内是如何转换的？等等问题，其实这里牵涉到了整个 Flutter 布局系统的知识点，所以还是比较难学习的，那么下面我们继续进行探索吧！⛽️

&emsp;以及看一下把前面几篇提到的类以及函数都真正用起来时是什么样的。

# sample code

&emsp;这里我们继续以 [《Flutter 源码梳理系列（四十）：HitTesting：At first sight》](https://juejin.cn/post/7406512341112717322) 篇中的示例代码为基础来进行探索。

```dart
//...
              Listener(
                child: Container(
                  width: 100,
                  height: 100,
                  color: Colors.yellow,
                ),
                onPointerDown: (p) {
                  debugPrint('onPointerDown 执行');
                },
              ),
//...
```

&emsp;首先我们给 Listener Widget 的 child 替换为一个宽高是 100 的 Container，这利于我们追踪坐标转换。我们刻意去点击 Container 的中心点，大概是 (50, 50) 的位置，然后由此 Flutter framework 接收到一个点击事件后，看下它是如何把点击事件的坐标从屏幕的全局坐标系转换到 Container 的中心点大概 (50, 50) 的位置的。

&emsp;下面我们依然在 RenderProxyBoxWithHitTestBehavior.hitTest 函数处打一个断点，然后以 debug 模式调试示例代码，项目启动以后我们尽量点击到 Container 的中心点，此时断点便会第一次命中，但是看到的 position 的参数的值是：Offset(194.7, 163.7) this 指针指向的 RenderPointerListener 的 depth 是 10，即 hit testing 还没有到我们的 Container 对应的 RenderProxyBoxWithHitTestBehavior，我们连续点击 Resume Program 按钮多次重复命中断点，直到 position 参数到值是：Offset(48.2, 48.7) （接近 50 左右），this 指针指向的 RenderObject 是 `_RenderColoredBox`，depth 是 31。（即直到再次点击 Resume Program 按钮的话程序就正常执行。）

![截屏2024-09-24 00.37.54.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/ba50920e48ef4f74b46dc96fd25b0cf0~tplv-73owjymdk6-jj-mark-v1:0:0:0:0:5o6Y6YeR5oqA5pyv56S-5Yy6IEAg6bOE6bG85LiN5oCVX-eJmeWMu-S4jeaAlQ==:q75.awebp?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1727195967&x-orig-sign=37J1ZsL2qHBPAgPthwng%2FFGLLIY%3D)

&emsp;这张图蕴含的信息巨大：

+ `_RenderColorBox` 是 RenderProxyBoxWithHitTestBehavior 的子类，也是直接参与 hit testing 的。
+ `RenderView.hitTest` 函数开始处的值为 `Offset(194.7, 163.7)` 的 `Offset position` 参数目前已经被转换为 RenderProxyBoxWithHitTestBehavior.hitTest 函数处的值为 `Offset(48.2, 48.7)` 的 `Offset position` 参数。
+ result 的 `_localTransforms` 的属性中 3 和 4 的 `_OffsetTransformPart` 值分别是：Offset(-0.0, -115.0) 和 Offset(-146.5, -0.0)，而 48.2 + 146.5 和 48.7 + 115.0 的值正是（194.7, 163.7），而我们示例代码中，Container 的左上角在当前屏幕坐标空间中的位置正是：`Offset(146.5, 115)`。 

# PlatformDispatcher.instance._dispatchPointerDataPacket(packet)

&emsp;hooks.dart 中的全局`_dispatchPointerDataPacket` 函数内，直接通过 PlatformDispatcher 的单例对象调用 PlatformDispatcher 的 `_dispatchPointerDataPacket` 函数，入参 `ByteData packet` 它是一个描述 Pointer 事件的二进制数据，它会被转化为 PointerData 数据，并通过一个中间类：PointerDataPacket 来包装它们，（实际 PointerDataPacket 类有一个 `List<PointerData>` 列表属性，用来存放由 `ByteData packet` 转化来的 PointerData 实例对象。）所以 dispatchPointerDataPacket 函数通过见名知意，可以理解为：调度 PointerData 数据/通过 PointerDataPacket 类进行调度 PointerData 数据。 

&emsp;**`PlatformDispatcher.instance._dispatchPointerDataPacket(packet)` 是从 engine 通过 hooks.dart 进行直接调用。** 

## PlatformDispatcher

&emsp;PlatformDispatcher：平台事件调度器（Platform event dispatcher）单例。它是与主机操作系统接口（host operating system's interface）的最基本接口。这是来自平台的平台消息（platform messages）和配置事件（configuration events）的中央入口点。它公开了核心调度器 API（core scheduler API）、输入事件回调（input event callback）、图形绘制 API（graphics drawing API） 和其他一些核心服务。它管理应用程序视图列表以及各种平台属性的配置。考虑避免通过 `PlatformDispatcher.instance` 静态引用此单例，而是更倾向于使用绑定进行依赖解析，例如 `WidgetsBinding.instance.platformDispatcher`。为了完成此操作，看到在 binding.dart 的 BindingBase 类中定义了：`ui.PlatformDispatcher get platformDispatcher => ui.PlatformDispatcher.instance;`，然后在 GestureBinding 中直接使用的 `platformDispatcher` 变量，其实就是使用的 PlatformDispatcher 单例对象。

```dart
@pragma('vm:entry-point')
void _dispatchPointerDataPacket(ByteData packet) {

  // 直接调用 PlatformDispatcher 单例的 _dispatchPointerDataPacket 函数。
  PlatformDispatcher.instance._dispatchPointerDataPacket(packet);  
}
```

&emsp;而在 PlatformDispatcher 单例的 `_dispatchPointerDataPacket` 函数内，则是通过一个 `_invoke1` 工具函数来执行 `onPointerDataPacket` getter 函数，而这个 `_invoke1` 工具函数的主要用途是为了保证 `onPointerDataPacket` getter 函数能在指定的 Zone 中执行。

&emsp;这里可能会问 PlatformDispatcher.instance 单例对象的 `onPointerDataPacket` setter 是什么时候调用赋值的，全局搜一下可以发现是在 GestureBinding 单例初始化的 initInstances 函数内进行赋值的，且赋的值正是 `GestureBinding._handlePointerDataPacket` 函数，看 `_invoke1` 函数后面函数堆栈里也正是 `GestureBinding._handlePointerDataPacket` 函数。

```dart
mixin GestureBinding on BindingBase implements HitTestable, HitTestDispatcher, HitTestTarget {
  @override
  void initInstances() {
    super.initInstances();
    _instance = this;
    
    // 在 GestureBinding 单例初始化时，
    // 直接把 GestureBinding 的 _handlePointerDataPacket 函数赋值给 platformDispatcher.onPointerDataPacket。
    platformDispatcher.onPointerDataPacket = _handlePointerDataPacket;
  }
  // ...
}
```

&emsp;并且在 PlatformDispatcher 的 `onPointerDataPacket` setter 中看到同时会把默认值是 `Zone.root` 的 `Zone _onPointerDataPacketZone` 属性修改为 `Zone.current`，即当前执行 initInstances 函数的这个 Zone。那么这个 Zone 是什么呢？

## Zone

&emsp;一个 zone 表示一个跨异步调用保持稳定的环境。

&emsp;所有的代码都是在一个 zone 的上下文中执行的，可以通过 Zone.current 访问到该 zone。初始的 main 函数在默认 zone（Zone.root）中运行。代码可以在不同的 zone 中运行，使用 runZoned 或 runZonedGuarded 来创建一个新的 zone 并在其中运行代码，或者使用 Zone.run 在已存在的 zone 中运行代码，这个 zone 可能是之前使用 Zone.fork 创建的。

&emsp;开发者可以创建一个新的 Zone，重写现有 Zone 的一些功能。例如，自定义 Zone 可以替换或修改 print、timers、microtasks 的行为，或者修改未捕获错误的处理方式。

&emsp;Zone 类不能被继承，但用户可以通过使用 ZoneSpecification fork 一个现有 zone（通常是 Zone.current）来提供自定义 Zone。这类似于创建一个继承基础 Zone 类的新类并覆盖一些方法，但实际上并没有创建新的类。取而代之的是，覆盖的方法提供为函数，明确地接受相应的类、超类和 this 对象作为参数。

&emsp;异步回调总是在安排它们的 Zone 中执行。这是通过两个步骤实现的：

1. 首先，回调函数被使用其中一个 registerCallback、registerUnaryCallback 或 registerBinaryCallback 来注册。这样做允许 Zone 记录回调函数的存在并且可能修改它（通过返回一个不同的回调函数）。进行注册的代码（比如 Future.then）也会记住当前的 Zone，这样它可以在稍后在该 Zone 中运行回调函数。

2. 在后续的某个时刻，注册的回调函数会在记住的 Zone 中运行，使用其中的 run、runUnary 或 runBinary 之一。

&emsp;这一切都由平台代码内部处理，大多数用户无需担心。然而，由底层系统提供的新异步操作的开发人员必须遵循协议以保持与 Zone 兼容。

&emsp;为了方便起见，Zone 提供了 bindCallback（以及对应的 bindUnaryCallback 和 bindBinaryCallback）来更轻松地遵循 Zone 契约：这些函数首先调用对应的注册函数，然后包装返回的函数，使其在稍后异步调用时在当前 Zone 中运行。

&emsp;类似地，当回调应通过 Zone.runGuarded 被调用时，Zone 提供了 bindCallbackGuarded（以及对应的 bindUnaryCallbackGuarded 和 bindBinaryCallbackGuarded）。

&emsp;看到这里即可明白 PlatformDispatcher 的 `_dispatchPointerDataPacket` 函数调用时使用的 `Zone _onPointerDataPacketZone` 参数是为了保证 hit testing 的整个过程在 PlatformDispatcher 单例对象指定好的 Zone 中进行。   

```dart
  void _dispatchPointerDataPacket(ByteData packet) {
    if (onPointerDataPacket != null) {
      
      // 1️⃣：onPointerDataPacket 已经被设定为 GestureBinding._handlePointerDataPacket，
      // 2️⃣：_onPointerDataPacketZone 已经被设定为 GestureBinding 单例执行 initInstances 函数时所在的 Zone，
      // 3️⃣：⚠️ _unpackPointerDataPacket 则是把 hooks.dart 中 `_dispatchPointerDataPacket` 函数传递来的 ByteData packet 数据整理为 PointerDataPacket 数据，这里超级重要，当前点击（触摸）事件的点击位置在当前屏幕空间的坐标值就要计算出来了。
      _invoke1<PointerDataPacket>(
        onPointerDataPacket,
        _onPointerDataPacketZone,
        _unpackPointerDataPacket(packet),
      );
    }
  }
```

&emsp;下面👇我们重点看一下 `_unpackPointerDataPacket(packet)` 函数调用。

## `_unpackPointerDataPacket`

&emsp;`_unpackPointerDataPacket` 是在 PlatformDispatcher 中定义的一个静态函数，它的返回值是一个 PointerDataPacket 实例对象，PointerDataPacket 类极其简单，可以把它理解为一个 PointerData 的包装类。PointerDataPacket 仅有一个 `final List<PointerData> data;` 列表属性用来存储 PointerData 实例对象。

```dart
/// A sequence of reports about the state of pointers.
/// PointerData 类可以理解为是 the state of pointers.
class PointerDataPacket {
  /// Creates a packet of pointer data reports.
  const PointerDataPacket({ this.data = const <PointerData>[] });

  /// Data about the individual pointers in this packet.
  ///
  /// This list might contain multiple pieces of data about the same pointer.
  final List<PointerData> data;
}
```

&emsp;`_unpackPointerDataPacket(packet)` 函数调用呢正是把入参 ByteData packet 转换为 PointerData 列表，而这个 PointerData 是什么呢？它表示的是 pointer 状态信息。其中最重要的便是本篇要关注的两个属性：physicalX 和 physicalY，其它还有很多重要的属性，例如：

+ viewId：此 PointerEvent 的 FlutterView 的 ID。
+ timeStamp：事件调度的时间，相对于任意时间轴。
+ change：pointer 自上次报告以来的变化情况。
+ kind：为其生成事件的输入设备类型。目前提供了：touch：一种基于触摸的 pointer 设备。mouse：一个基于鼠标的 pointer 设备。stylus：一种带有触笔的 pointer 装置。trackpad：触控板的手势。

&emsp;等等属性，建议自行浏览一下，对我们理解 Flutter 处理 PointerEvent 的过程有一些帮助。下面我们重点看一下 physicalX 和 physicalY：
  
```dart
  /// X coordinate of the position of the pointer, in physical pixels in the global coordinate space.
  /// pointer 位置的 X 坐标，在全局坐标空间（屏幕坐标空间）中以物理像素为单位。（例如在 iOS 3x 设备上时除以 3，即可得到逻辑像素为单位的值。）
  final double physicalX;

  /// Y coordinate of the position of the pointer, in physical pixels in the global coordinate space.
  /// pointer 位置的 Y 坐标，在全局坐标空间（屏幕坐标空间）中以物理像素为单位。（例如在 iOS 3x 设备上时除以 3，即可得到逻辑像素为单位的值。）
  final double physicalY;
```

&emsp;基于以上逻辑我们通过堆栈中的变量看到，本地点击事件的：`ByteData packer` 参数被转换为了：

+ `PointerData(viewId: 0, x: 583.9999694824219, y: 490.9999694824219)`。

&emsp;然后我们把 x 和 y 值分别除以 3（我这里用的 15 pro 模拟器，所以需要除以 3）即可得到，本地点击事件在当前屏幕坐标空间中坐标值：(194.7, 163.7)。

&emsp;至此，我们本次点击事件的在当前屏幕的坐标起点就找到啦！继续往下，三个参数的内容看完了，该调用 `_invoke1` 函数了：`_invoke1<PointerDataPacket>(onPointerDataPacket, _onPointerDataPacketZone, _unpackPointerDataPacket(packet),);`。

# `_invoke1`

&emsp;`_invoke1` 也是 hooks.dart 中定义的一个全局函数。在给定的 `Zone zone` 中调用 `void Function(A a)? callback`，并传入参数 `A arg`，如果传入的 `Zone zone` 和当前执行 `_invoke1` 函数不是同一个 Zone 的话，会通过调用 `zone.runUnaryGuarded` 函数在给定的 `zone` 中调用 `void Function(A a)? callback`。

&emsp;`_invoke1` 名称中的 1 指的是入参 `void Function(A a)? callback` 回调函数所期望的参数数量（因此除了回调本身和回调执行的区域外，还会将参数传递给该函数）。其它还提供了 `_invoke2`、`_invoke3` 等。

```dart
void _invoke1<A>(void Function(A a)? callback, Zone zone, A arg) {
  // 如果入参 callback 为 null，则直接 return。
  if (callback == null) {
    return;
  }
  
  // 如果入参 zone 和当前代码执行所在的不是一个 zone，则通过调用 Zone 的 runUnaryGuarded 函数，把 callback 在入参 zone 中执行。
  if (identical(zone, Zone.current)) {
  
    // identical 执行严格的判等操作。
    callback(arg);
  } else {
    
    // runUnaryGuarded 在当前指定的 Zone 中执行给定的 [action]，并捕获同步错误。
    zone.runUnaryGuarded<A>(callback, arg);
  }
}
```

&emsp;所以 `_invoke1` 出现在 hit testing 开始处，则表示了 hit testing 的整个过程需要在指定的 Zone 中进行。看上面👆的 `_invoke1` 调用时传入的 Zone 参数是 PlatformDispatcher 的 `_onPointerDataPacketZone` 属性，而 `_onPointerDataPacketZone` 属性正是在 GestureBinding 的 initInstances 函数执行时被赋值为 Zone.current 的，即执行 GestureBinding 的 initInstances 函数时所在的 Zone 被记录了下来，用于后续的 hit testing 时，也在此 Zone 中进行。

&emsp;通过 Threads & Variables 选项卡，看到当前传入 `_invoke1` 函数的 Zone 参数是：`{_RootZone}`。

&emsp;下面我们继续向下，已知 `_invoke1` 调用时入参 callback 是 `GestureBinding._handlePointerDataPacket`。

# `GestureBinding._handlePointerDataPacket`

&emsp;整体而言，`_handlePointerDataPacket` 也可以作为一个中间函数看待，它做的事情比上面👆的 `_unpackPointerDataPacket` 更进一步。

&emsp;上面👆的 `_unpackPointerDataPacket` 静态函数是把最原始的 `ByteData packet` 数据转化为内部是 `final List<PointerData> data` 列表的 `PointerDataPacket` 数据，而且当时我们也提到 PointerData 中的物理像素为单位的 physicalX 和 physicalY 分别除以 3 可到逻辑像素为单位的 x 和 y 坐标，而到了 `_handlePointerDataPacket` 这里，我们则可以从代码中看到切实的物理像素到逻辑像素到转换。

&emsp;我们看到 `_handlePointerDataPacket` 函数最开始是一个：`_pendingPointerEvents.addAll(PointerEventConverter.expand(packet.data, _devicePixelRatioForView));` 调用，而它的入参部分：`PointerEventConverter.expand(packet.data, _devicePixelRatioForView)` 则是我们首要关注的，它做的事件就是更进一步，把 PointerData 数据转化为 PointerEvent 数据。所以整体的数据转换流程是：`ByteData -> PointerData -> PointerEvent`。

```dart
  void _handlePointerDataPacket(ui.PointerDataPacket packet) {
    // We convert pointer data to logical pixels so that e.g. the touch slop can be defined in a device-independent manner.
    // 我们将 PointerData 转换为逻辑像素，以便触摸斜率可以以与设备无关的方式定义。
    try {
      _pendingPointerEvents.addAll(PointerEventConverter.expand(packet.data, _devicePixelRatioForView));
      
      if (!locked) {
        // 由 ByteData -> PointerData -> PointerEvent 转化完成后，就开始切实处理本次的 pointer 事件啦！
        _flushPointerEventQueue();
      }
    } catch (error, stack) {
    
      // catch 补错。
      FlutterError.reportError(FlutterErrorDetails(
        exception: error,
        stack: stack,
        library: 'gestures library',
        context: ErrorDescription('while handling a pointer data packet'),
      ));
    }
  }
```

## `PointerEventConverter.expand(packet.data, _devicePixelRatioForView)`

&emsp;首先是一个辅助函数 `_devicePixelRatioForView`，它是用来获取当前设备屏幕上每个逻辑像素的设备像素数量，如 iOS 设备的 2X 和 3X。

```dart
  double? _devicePixelRatioForView(int viewId) {
    return platformDispatcher.view(id: viewId)?.devicePixelRatio;
  }
```

&emsp;如上👆 `devicePixelRatio` 表示 FlutterView 显示在设备屏幕上每个逻辑像素的设备像素数量。这个数字可能不是 2 的幂。事实上，它甚至可能不是整数。例如，Nexus 6 的设备像素比为 3.5。设备像素（device pixels）也被称为物理像素（physical pixels）。逻辑像素（logical pixels）也被称为无关分辨率或无关像素。根据定义，每厘米大约有 38 个逻辑像素，或者每英寸的物理显示器上大约有 96 个逻辑像素。`devicePixelRatio` 返回的值最终来自硬件本身、设备驱动程序或存储在操作系统或固件中的硬编码值，有时可能是不准确的，有时误差可能很大。

&emsp;**如在我们当前测试用的 15 pro 模拟器上，获取的 devicePixelRatio 值是：3。**

&emsp;然后是 PointerEventConverter 中定义的静态函数：expand，它的代码较多，所以这里我们就不贴出来了。它的主要功能就是遍历入参 `ui.PointerDataPacket packet` 的 `final List<PointerData> data` 属性中的 PointerData，根据每个 PointerData 的 change 属性的值，来创建 PointerAddedEvent/PointerDownEvent，如我们的实例中，就是创建一个 PointerAddedEvent 和一个 PointerDownEvent。而我们最关注的物理像素向逻辑像素转换便是：

```dart
final Offset position = Offset(datum.physicalX, datum.physicalY) / devicePixelRatio;
final Offset delta = Offset(datum.physicalDeltaX, datum.physicalDeltaY) / devicePixelRatio;
```

&emsp;即直接由 PointerData 的 physicalX 和 physicalY 属性除以 devicePixelRatio 得到一个 Offset position 值，并会直接把它赋值给 PointerEvent 对象的 position 属性。

&emsp;然后在我们的实例代码中，PointerEventConverter.expand 调用最终是返回一个 `Iterable<PointerEvent>` 实例对象，其中是一个 PointerAddedEvent 实例对象和一个 PointerDownEvent 实例对象。然后这个 `Iterable<PointerEvent>` 实例对象被添加到了：`final Queue<PointerEvent> _pendingPointerEvents = Queue<PointerEvent>();` 中，它是 GestureBinding 的一个属性，专门用来记录 PointerEvent 的。

![截屏2024-09-24 00.24.05.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/70b726a16fe5489182e1a4f7dbb22321~tplv-73owjymdk6-jj-mark-v1:0:0:0:0:5o6Y6YeR5oqA5pyv56S-5Yy6IEAg6bOE6bG85LiN5oCVX-eJmeWMu-S4jeaAlQ==:q75.awebp?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1727195704&x-orig-sign=eEDGhlcUwwy7A5sZqr740VbhRhg%3D)

![截屏2024-09-24 00.23.34.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/d146111a669c446ba7f0938b192fea5b~tplv-73owjymdk6-jj-mark-v1:0:0:0:0:5o6Y6YeR5oqA5pyv56S-5Yy6IEAg6bOE6bG85LiN5oCVX-eJmeWMu-S4jeaAlQ==:q75.awebp?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1727195715&x-orig-sign=F8ApHJ19DTaBJu34vp7hCaiF0UY%3D)






































## 参考链接
**参考链接:🔗**
+ [HitTestTarget class](https://api.flutter.dev/flutter/gestures/HitTestTarget-class.html)
+ [HitTestResult class](https://api.flutter.dev/flutter/gestures/HitTestResult-class.html)
+ [BoxHitTestResult class](https://api.flutter.dev/flutter/rendering/BoxHitTestResult-class.html)
+ [`HitTestEntry<T extends HitTestTarget> class`](https://api.flutter.dev/flutter/gestures/HitTestEntry-class.html)
+ [BoxHitTestEntry class](https://api.flutter.dev/flutter/rendering/BoxHitTestEntry-class.html)
+ [HitTestResult class](https://api.flutter.dev/flutter/gestures/HitTestResult-class.html)
+ [PlatformDispatcher class](https://api.flutter.dev/flutter/dart-ui/PlatformDispatcher-class.html)
+ [Zone class](https://api.flutter.dev/flutter/dart-async/Zone-class.html)
