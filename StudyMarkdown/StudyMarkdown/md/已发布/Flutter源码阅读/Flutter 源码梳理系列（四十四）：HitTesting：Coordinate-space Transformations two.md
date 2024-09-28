# Flutter 源码梳理系列（四十四）：HitTesting：Coordinate-space Transformations two

# `GestureBinding._flushPointerEventQueue` 

&emsp;接上篇，`_handlePointerDataPacket` 函数内会进行 PointerData 的数据处理，会把入参传递来的 `ui.PointerDataPacket packet` 参数的 data 属性：`final List<PointerData> data` 列表中的 PointerData 转换为对应类型的 PointerEvent，目前通过打印看到实际是转换了两个 PointerEvent 的子类实例对象：PointerAddedEvent 和 PointerDownEvent，并且它们两个的 postion 属性值都是：`Offset(194.7, 163.7)`。

&emsp;当把点击事件的数据转化为 PointerEvent 并收集在 `_pendingPointerEvents` 队列中，接下来便是循环遍历此队列中的 PointerEvent 对象了，即开始执行 `_flushPointerEventQueue` 函数。

```dart
  void _flushPointerEventQueue() {
  
    // 循环遍历 _pendingPointerEvents 中的 PointerEvent 对象，对此 Pointer 事件进行处理。
    while (_pendingPointerEvents.isNotEmpty) {
      handlePointerEvent(_pendingPointerEvents.removeFirst());
    }
  }
```

&emsp;在 `_flushPointerEventQueue` 内会对 `_pendingPointerEvents` 队列中的 PointerEvent 从头开始进行处理，在我们的实例代码中，看到第一个被处理的是 PointerDownEvent 实例对象。

# `GestureBinding.handlePointerEvent & GestureBinding._handlePointerEventImmediately`

&emsp;实际 handlePointerEvent 只是 `_handlePointerEventImmediately` 函数的超简单包装，在其内部添加了一些 debug 下的测试代码。

&emsp;handlePointerEvent 函数的官方注释是：将事件（PointerEvent event）分派给 hit test 在其 position 上找到的 target。这个方法根据事件类型（不同的 PointerEvent 子类，是不同的事件类型）将给定的事件发送给 dispatchEvent:（即：`void dispatchEvent(PointerEvent event, HitTestResult? hitTestResult)` 函数，hit test 结束后找到了 target，然后带着参数调用 dispatchEvent 函数。）。

```dart
  void handlePointerEvent(PointerEvent event) {
    // ⬇️ 测试代码。
    if (resamplingEnabled) {
      _resampler.addOrDispatch(event);
      _resampler.sample(samplingOffset, samplingClock);
      return;
    }

    // Stop resampler if resampling is not enabled. This is a no-op if resampling was never enabled.
    _resampler.stop();
    
    // ⬆️ 以上都是测试代码。
    
    // 实际立即执行：handlePointerEvent。
    _handlePointerEventImmediately(event);
  }
```

&emsp;实际直接调用 `_handlePointerEventImmediately` 函数，注意当前的 event 参数的类型是：PointerDownEvent。所以本次执行的是 `_handlePointerEventImmediately` 函数中第一个 if 中的函数，首先创建一个 HitTestResult 实例对象，（它会在接下来的整个 hit testing 过程中使用，并最终记录一组 HitTestEntry 对象。）然后是 hitTestInView 的调用，由此开始发起整个 hit testing 的过程。

&emsp;注意当这里调用 hitTestInView 函数时，会发现调用的是：RendererBinding.hitTestInView，那么这里为什么由 GestureBinding 转移到了 RendererBinding 中去了呢？首先我们看一下 GestureBinding、RendererBinding 和 WidgetsFlutterBinding 的定义：

&emsp;GestureBinding 定义：

```dart
mixin GestureBinding on BindingBase implements HitTestable, 
                                               HitTestDispatcher,
                                               HitTestTarget { //... }
```

&emsp;RendererBinding 定义：

```dart
mixin RendererBinding on BindingBase,
                         ServicesBinding,
                         SchedulerBinding, 
                         GestureBinding, 
                         SemanticsBinding, 
                         HitTestable { //... }
```

&emsp;WidgetsFlutterBinding 定义：

```dart
class WidgetsFlutterBinding extends BindingBase with GestureBinding,
                                                     SchedulerBinding,
                                                     ServicesBinding, 
                                                     PaintingBinding, 
                                                     SemanticsBinding, 
                                                     RendererBinding, 
                                                     WidgetsBinding { //... }
```

&emsp;可以看到 WidgetsFlutterBinding 是 RendererBinding 的子类，而 RendererBinding 又是 GestureBinding 的子类，并且它重写了父类的 hitTestInView 函数。而在 Threads & Varibles 选项卡中看到当前的 this 指针指向正是一个 WidgetsFlutterBinding 对象，所以当它要执行 hitTestInView 函数时，必定是执行距离自己最近的父类，那么便是 RendererBinding 了，所以这里由 `GestureBinding._handlePointerEventImmediately` 执行到了 `RendererBinding.hitTestinView` 中。 

&emsp;注意看如果本次是一个 PointerDownEvent/PointerPanZoomStartEvent 事件的话，则会把本次的 hit testing 的结果缓存下来，直接把 `hitTestResult` 实例对象记录在 GestureBinding 的 `final Map<int, HitTestResult> _hitTests = <int, HitTestResult>{}` 属性中，它会以本次 event 的 pointer 为 key，以本次 hit testing 的结果 hitTestResult 对象为 value 保存在这个 `_hitTests` Map 中。

```dart
  void _handlePointerEventImmediately(PointerEvent event) {
    HitTestResult? hitTestResult;
    
    if (event is PointerDownEvent ||
        event is PointerSignalEvent ||
        event is PointerHoverEvent ||
        event is PointerPanZoomStartEvent) {
        
      // 创建一个 HitTestResult 变量，用于记录并参与本次 hit testing 的整个过程。 
      hitTestResult = HitTestResult();
      
      // 真正要开始 hit testing 了，看到入参：position 正是：Offset(194.7, 163.7)。
      hitTestInView(hitTestResult, event.position, event.viewId);
      
      if (event is PointerDownEvent ||
          event is PointerPanZoomStartEvent) {
        
        // 如果本次处理的是 PointerDownEvent 或者 PointerPanZoomStartEvent 的话，
        // 则把本次 hit test 的结果：hitTestResult 缓存起来。
        _hitTests[event.pointer] = hitTestResult;
      }
    } else if (event is PointerUpEvent ||
               event is PointerCancelEvent ||
               event is PointerPanZoomEndEvent) {
               
      // 如果本次的 event 是：PointerUpEvent、PointerCancelEvent、PointerPanZoomEndEvent 事件的话，
      // 则把之前的根据 event.pointer 缓存的 HitTestResult 移除掉。
      hitTestResult = _hitTests.remove(event.pointer);
      
    } else if (event.down ||
               event is PointerPanZoomUpdateEvent) {
      
      // 因为 pointer 向下发生的事件(如 PointerMoveEvent)应该被分派到它们最初的 PointerDownEvent 所在的相同位置，
      // 我们希望重用 pointer 向下时找到的 path，而不是每次得到这样的事件时都进行 hit test。
      
      // 即这里是把上次 hit test 缓存的 HitTestResult 取出来直接复用。
      hitTestResult = _hitTests[event.pointer];
    }
    
    // ⚠️ 注意如果本次 event 是 PointerAddedEvent 的话，则是进行 dispatchEvent 的调用，
    // 我们的实例代码中，在本次测试中当 GestureBinding._flushPointerEventQueue 遍历到 _pendingPointerEvents 的第二个 PointerEvent 时，它正是一个 PointerAddedEvent 实例，所以此时便会执行 dispatchEvent 调用。  

    if (hitTestResult != null ||
        event is PointerAddedEvent ||
        event is PointerRemovedEvent) {
       
      // 向 hitTestResult 中被收集的所有 HitTestEntry 对象调度 event 事件，执行它们的 handleEvent 函数。
      dispatchEvent(event, hitTestResult);
    }
  }
```

&emsp;OK，下面我们开始看 RendererBinding.hitTestInView 中超长的 hitTest 递归调用，先看如何把点击事件的起始坐标：Offset(194.7, 163.7) 转换为目标 RenderBox 的坐标的。 

# RendererBinding.hitTestInView

&emsp;首先是看一下调用 hitTestInView 时传入的三个参数：

+ `HitTestResult result`: 传入一个刚刚创建的空的 HitTestResult 实例对象。
+ Offset position: 传入点击发生时的坐标点，x 和 y 坐标值已经由屏幕的物理像素为单位转换为逻辑像素为单位。
+ int viewId: 指定的 View ID，目前传入的是 0。

&emsp;viewId 用于从 RendererBinding 的 `_viewIdToRenderView` 属性中找到与这个 viewId 对应的 RenderView 实例对象。在本次调用这里看到 `_viewIdToRenderView[viewId]` 取得的是我们的 Render Tree 的根节点：`_ReusableRenderView` 实例对象。由此三个参数可得，hit testing 的三个开始条件就达成了：

1. 一个 HitTestResult 空的实例对象，用于记录 hit test 的过程。
2. 一个 Offset 当前点击发生时在当前屏幕坐标系的坐标点，并且已经把坐标值的单位转换为逻辑像素，不同物理分辨率的设备都能取得相同的逻辑坐标。
3. Render Tree 的根节点。

```dart
final Map<Object, RenderView> _viewIdToRenderView = <Object, RenderView>{}` 
``` 

&emsp;在 RendererBinding.hitTestInView 中首先是根据入参 int viewId 取 view，通过 Threads & Variables 选项看到当前 this 指针指向的是 WidgetsFlutterBinding 单例对象，然后它的 `_viewIdToRenderView` 属性有值，是一个 size 是 1 的 `_Map`，而这个 Map 仅有的一个元素是：key 是 0，value 是 `_ReusableRenderView`（Render Tree 根节点）。 

![截屏2024-09-28 12.04.47.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/781ab1ddd7f34eaebc42798714dc9d51~tplv-73owjymdk6-jj-mark-v1:0:0:0:0:5o6Y6YeR5oqA5pyv56S-5Yy6IEAg6bOE6bG85LiN5oCVX-eJmeWMu-S4jeaAlQ==:q75.awebp?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1727582842&x-orig-sign=Ii3FP2Wwj8A%2B0CrHr8UTe1yMtH8%3D)

&emsp;然后是 RendererBinding.hitTestInView 函数的实现，此函数是重写自父类。看到实际仅有两行：`_viewIdToRenderView[viewId]` 取得 Render tree 的根节点，然后开始调用它的 hitTest 函数，开启整个 hit Testing 过程，而入参仅是最简单的空的 HitTestResult 实例对象和一个点击起点在当前屏幕的逻辑坐标。 

```dart
  @override
  void hitTestInView(HitTestResult result, Offset position, int viewId) {
  
    // _viewIdToRenderView[viewId] 首先取得的就是当前 Render Tree 的根节点，由此开启 hit testing。 
    _viewIdToRenderView[viewId]?.hitTest(result, position: position);
    
    // 然后再调用父类的 hitTestInView 函数。
    super.hitTestInView(result, position, viewId);
  }
```

&emsp;这里随着 RendererBinding 的继承链去看的话，找到 RendererBing 中 super 的指向，其实是 GestureBinding，即这里的 `super.hitTestInView(result, position, viewId)` 的调用其实调用的是 GestureBinding.hitTestInView 函数，而它的实现贼简单，仅仅是把当前的 GestureBinding 对象构建一个 HitTestEntry 对象添加到当前的 HitTestResult result 实例对象的 `_path` 属性中。

```dart
  @override // from HitTestable
  void hitTestInView(HitTestResult result, Offset position, int viewId) {
  
    // 直接用 this 构建一个 HitTestEntry 对象添加到当前的 result 中去。
    result.add(HitTestEntry(this));
  }
```

&emsp;通过之前对 HitTestEntry 的学习，我们已知能作为 HitTestEntry 构建参数的值必须是 HitTestTarget 的子类对象，而我们全局搜索发现仅有：RenderObject、GestureBinding、TextSpan 继承自 HitTestTarget，所以这里只有它们三个或者其子类实例对象才可用于构建 HitTestEntry 实例对象。而被 result 收集的 HitTestEntry 实例对象，则会在 hit test 执行结束时被调用其 handleEvent 函数进行调度 PointerEvent 事件。

&emsp;下面👇是 RenderView.hitTest 调用过程。

# RenderView.hitTest

&emsp;首先在上面的函数堆栈追踪过程中记得 `_viewIdToRenderView[viewId]` 取到的 Render Tree 根节点是 `_ReusableRenderView` 实例对象，那么这里怎么调用到 RenderView.hitTest 这里了呢？是因为 `_ReusableRenderView` 是 RenderView 的子类，并且它没有重写 RenderView 的 hitTest 函数，所以这里的 `_ReusableRenderView` 实例对象调用 hitTest 函数时，其实是执行的自己的父类 RenderView 的 hitTest 函数。

&emsp;hitTest 函数用于确定给定位置 Offset position 入参处的 RenderObject 集合。如果给定的 position 坐标包含在此 RenderObject 或其子级之一中，则返回 true。将包含该 position 的任何 RenderObject 添加到给定的 HitTestResult result 入参中。position 参数位于 RenderView 的坐标系中，也就是逻辑像素中。这不一定是根 Layer 期望的坐标系，后者通常是物理（设备）像素。

```dart
  bool hitTest(HitTestResult result, { required Offset position }) {
  
    // 直接交给自己的 child 处理 hit test。可以看到这里通过 BoxHitTestResult.wrap(result) 调用，
    // 直接把 HitTestResult result 入参转化为一个 BoxHitTestResult 对象，
    // 通过前面的学习我们已知的 BoxHitTestResult 被用作 RenderBox 的 hit test 的结果，
    // wrap 函数只是简单的属性转化，最最重要的是后续 hit test 过程中，就可以使用 BoxHitTestResult 的 addWithXXX 系列函数了，
    // 它们在后续的坐标转换过程中起至关重要的作用。
    if (child != null) {
    
      // 直接调用 child 的 hitTest 函数。
      child!.hitTest(BoxHitTestResult.wrap(result), position: position);
    }
    
    // 以当前 Render Tree 根节点为参数构建一个 HitTestEntry 实例对象并被添加到 result 中，
    // 实际在每次 hit test 时，Render Tree 的根节点都会以这种形式被收集到 HitTestResult 中去。
    result.add(HitTestEntry(this));
    
    // 并且直接返回 true，表示自己是可以响应 hit test 的（自己是可以处理本次 PointerEvent 事件的。）
    return true;
  }
```

&emsp;我们知道 Render Tree 的根节点的 child 是绝对不可能为 null 的，所以这里的 `child!.hitTest(BoxHitTestResult.wrap(result), position: position)` 继续往下执行，那么会执行向何方呢？通过 Threads & Variables 选项卡可以看到当前 `_ReusableRenderView` 对象的 child 属性是一个 RenderSemanticsAnnotations 实例对象，看它的定义看到是一个直接继承自 RenderProxyBox 的一个子类：

```dart
// 为该子树的 [SemanticsNode] 添加注释。
class RenderSemanticsAnnotations extends RenderProxyBox { //... }
``` 

&emsp;RenderSemanticsAnnotations 并没有重写 hitTest 函数，继续向下看它的父类 RenderProxyBox 的定义。

&emsp;RenderProxyBox 是一个用于 RenderBox 的基类，其外观类似于其 child。RenderProxyBox 具有单个 child，并通过调用 child 的 render box protocol 中的每个函数来模仿该 child 的所有属性。例如，RenderProxyBox 通过使用相同的约束要求其 child 进行布局然后匹配尺寸来确定其大小。RenderProxyBox 本身并不实用，因为你可能会更好地用其 child 替换 RenderProxyBox。然而，RenderProxyBox 是一个有用的基类，用于希望模仿其 child 大部分但非全部属性的 RenderBox。

&emsp;看下面👇 RenderProxyBox 的定义的全部内容，可以看到它自己仅是一个空类，它的全部内容是来自父类 RenderBox 和混入的 `RenderObjectWithChildMixin<RenderBox>` 和 `RenderProxyBoxMixin<RenderBox>`。

&emsp;而混入的 `RenderObjectWithChildMixin<RenderBox>` 也指明了，此 RenderProxyBox 仅有一个子级，或者说是 RenderProxyBox 的子类也是仅有一个子级的存在。

&emsp;然后分别点入 RenderObjectWithChildMixin 和 RenderProxyBoxMixin 内部看到它们仅仅是自定义自己设定下的功能，而它们自己并没有重写 hitTest 函数，所以当这里的 RenderSemanticsAnnotations 实例对象调用 hitTest 函数时，其实是执行到了 RenderBox.hitTest 函数中。

```dart
class RenderProxyBox extends RenderBox with RenderObjectWithChildMixin<RenderBox>, RenderProxyBoxMixin<RenderBox> {

  /// Creates a proxy render box.
  /// Proxy render boxes are rarely created directly because they proxy the render box protocol to [child]. Instead, consider using one of the subclasses.
  
  RenderProxyBox([RenderBox? child]) {
    this.child = child;
  }
}
```

# RenderBox.hitTest

&emsp;

```dart
  bool hitTest(BoxHitTestResult result, { required Offset position }) {
    if (_size!.contains(position)) {
    
      if (hitTestChildren(result, position: position) || hitTestSelf(position)) {
      
        result.add(BoxHitTestEntry(this, position));
        return true;
      }
    }
    
    return false;
  }
```




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
