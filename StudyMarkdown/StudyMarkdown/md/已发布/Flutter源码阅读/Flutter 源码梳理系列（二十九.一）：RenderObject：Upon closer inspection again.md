# Flutter 源码梳理系列（二十九.一）：RenderObject：Upon closer inspection again

# 前情回顾

&emsp;还记得上一篇的 "猜想三阶段" 吗？我们本来还想证明这猜想三阶段是否正确呢，通过打断点发现仅有阶段 1️⃣ 的前半部分总结对了，而阶段 2️⃣ 和 阶段 3️⃣ 其实都是完全提前进行了，而并不是说再等待新的一帧再进行处理。整体而言是 Flutter 项目启动后，第一次命中 RendererBinding.drawFrame 时，就会进行以 Render Tree 根节点为起点的 Layout、CompositingBits 和 Paint 操作，并且更重要的是，这些操作都是在沿着 RenderObject 链递归往子级进行的，即每次都是整个 Render Tree 上的所有节点都可以得到 Layout、CompositingBits 和 Paint 操作。

&emsp;通过上一篇的学习我们已知 Render Tree 的根节点会通过 RenderObject.scheduleInitialLayout 和 RenderObject.scheduleInitialPaint 两个函数的调度，把自己加入到所有 RenderObject 节点的 `_owner` 属性共同指向的 PipelineOwner 对象的布局列表（`owner!._nodesNeedingLayout`）和绘制列表（`owner!._nodesNeedingPaint`）中，并在 RenderObject.scheduleInitialPaint 函数被调用时创建了 Layer Tree 的根节点。

&emsp;以及在正常的通过在 RenderObject.adoptChild 函数内部调用 RenderObject.markNeedsCompositingBitsUpdate 把 Render Tree 的根节点加入到所有 RenderObject 节点的 `_owner` 属性共同指向的 PipelineOwner 对象的合成位更新列表（`owner!._nodesNeedingCompositingBitsUpdate`），这样就完成了在初次调用 RendererBinding.drawFrame 前 Render Tree 的根节点分别加入到全局 PipelineOwner 对象的三个列表中。

&emsp;我们通过在 RendererBinding.drawFrame 中打断点发现首次 drawFrame 执行时，Render Tree 的根节点已经被分别加入到全局 PipelineOwner 的：需要布局、需要合成位更新、需要布局 三个列表里面了。如下截图，这样也就意味着在首次调用 RendererBinding.drawFrame 时，其内部的：`rootPipelineOwner.flushXXX` 三连击 Flush 会以 Render Tree 的根节点为起点进行。  

![截屏2024-07-27 13.55.56.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/31bbf1f3d85c4f18ad1d800d4be20eb3~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1722146363&x-orig-sign=2vApcnW6Uv1sqUI5HK47js2bfA4%3D)

&emsp;并且我们还注意到 Render Tree 的根节点对应的 RenderObjectElement 的 depth 是 4，即当 Element Tree 构建到第 4 个 Element 节点时，开始着手构建 Render Tree。既然如此我们就自 runApp 函数梳理一下这第 4 个 Element 节点。

# Render Tree 根节点

&emsp;我们直接在 `_RawViewElement.mount` 的 `renderObject.prepareInitialFrame();` 行打一个断点，然后运行项目，会直接命中此断点，看当前：

![截屏2024-07-27 21.08.29.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/212c3ce154e940f7933bb2ab4b21009d~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1722172401&x-orig-sign=YlPOali8C6mU7%2BmTGYXdrljy7jQ%3D)

&emsp;可看到当前挂载的是一个：`class _RawViewElement extends RenderTreeRootElement { // ... }` 实例对象，并且它的 `_depth` 是 4，并且看到它的 `_renderObject` 属性是一个：`class _ReusableRenderView extends RenderView { // ... }` 实例对象，此实例对象即是 Render Tree 的根节点，然后此 `_RawViewElement` 实例对象便可认为是根 RenderObjectElement 节点（持有根 RenderObject 节点的 RenderObjectElement 节点）。

&emsp;下面我们沿着 runApp 函数调用梳理一下：Widget Tree 和 Element Tree 的构建，这里我们找 Widget 下级时，可以秉承如下原则：

+ 如果看到一个 Widget 是 StatelessWidget 的话直接看它的 build 函数，此 build 函数返回的便是它的下一个 Widget。
+ 如果看到一个 Widget 是 StatefulWidget 的话直接看它的 State 的 build 函数，此 build 函数返回的便是下一个 Widget。

&emsp;梳理通畅后我们可以获得一个如下表格：

| **depth**           | **Widget**          | **Element**      | **RenderObject**      |
| :-----------------: | :------------------ | :--------------- | :-------------------- |
|  **1**              |  RootWidget         | RootElement      | -                     |
|  **2**              |  View               | StatefulElement  | -                     |
|  **3**              |  RawView            | StatelessElement | -                     |
|  **4**              |  _RawViewInternal   | _RawViewElement  | _ReusableRenderView   |

&emsp;其中涉及的 class 和 mixin 有如下定义：

| name                  | define    |
| :-------------------: | :-----------------: |
| RootWidget            | class RootWidget extends Widget {}  |
| RootElement           | class RootElement extends Element with RootElementMixin {}  |
| RootElementMixin      | mixin RootElementMixin on Element {}  |
| View                  | class View extends StatefulWidget {}  |
| RawView               | class RawView extends StatelessWidget {}  |
| _RawViewInternal      | class _RawViewInternal extends RenderObjectWidget {}  |
| _RawViewElement       | class _RawViewElement extends RenderTreeRootElement {} |
| RenderTreeRootElement | abstract class RenderTreeRootElement extends RenderObjectElement {}  |
| _ReusableRenderView   | class _ReusableRenderView extends RenderView {}  |
| RenderView            | class RenderView extends RenderObject with RenderObjectWithChildMixin<RenderBox> {}  |
| RenderObjectWithChildMixin | mixin RenderObjectWithChildMixin<ChildType extends RenderObject> on RenderObject {}  |

&emsp;下面我们简单过一下其中涉及部分重要代码，由于代码量太多了，所以不一一展开，我们只看重点。

## runApp

&emsp;Flutter 项目启动，执行 runApp 函数，第一行会创建一个 WidgetsFlutterBinding 的单例出来，它混入了一整组的 Binding，用来为 Flutter 项目提供基础功能。

```dart
class WidgetsFlutterBinding extends BindingBase with GestureBinding, SchedulerBinding, ServicesBinding, PaintingBinding, SemanticsBinding, RendererBinding, WidgetsBinding { // ... }
```

&emsp;在 Flutter 中，Binding 是 framework 提供的一种机制，用于管理特定平台的操作系统级别功能，例如渲染、布局、输入事件等。每个平台（比如 Android、iOS、Web）都有自己对应的 Binding，它们负责将 Flutter 应用程序与平台特定功能进行交互。Flutter 的 Binding 负责协调各种功能并将它们整合到统一的框架中，从而实现跨平台的一致性。

&emsp;Binding 通常是隐藏的，Flutter 开发者在大多数情况下无需直接和 Binding 交互。Binding 的存在使得 Flutter 能够高效地处理各平台间的差异，让开发者专注于应用逻辑的编写。

```dart
void runApp(Widget app) {
  final WidgetsBinding binding = WidgetsFlutterBinding.ensureInitialized();
  _runWidget(binding.wrapWithDefaultView(app), binding, 'runApp');
}
```

```dart
  Widget wrapWithDefaultView(Widget rootWidget) {
    return View(
      view: platformDispatcher.implicitView!,
      deprecatedDoNotUseWillBeRemovedWithoutNoticePipelineOwner: pipelineOwner, // ⬅️ Render Tree 所有节点的 owner 属性
      deprecatedDoNotUseWillBeRemovedWithoutNoticeRenderView: renderView, // ⬅️⚠️⬇️ Render Tree 根节点传递下去。
      child: rootWidget, // ⬅️ 就是我们传递给 runApp 的第一个我们自己手写的 Widget
    );
  }
```

&emsp;`_runWidget` 调用时的：`binding.wrapWithDefaultView(app)` 函数调用，会执行到 WidgetsBinding.wrapWithDefaultView 函数内，它会直接返回上面表格中的 (depth = 2, Viwe)，然后 View 初始化时的 `deprecatedDoNotUseWillBeRemovedWithoutNoticeRenderView: renderView` 参数其实就是我们的 Render Tree 的根节点，它是在 RendererBinding 中定义的一个默认 final 属性 renderView：

```dart
  // _ReusableRenderView 是直接继承自 RenderView 的子类：class _ReusableRenderView extends RenderView {}
  late final RenderView renderView = _ReusableRenderView(
    view: platformDispatcher.implicitView!,
  );
```
 
## attachRootWidget

&emsp;然后我们继续沿着 `_runWidget` 函数往下看的话，会看到创建 RootWidget 和 RootElement 对象，它们分别是 Widget Tree 和 Element Tree 的根节点。特别是 RootWidget 创建时会直接把上面创建的 View 对象作为自己的 child 属性。

```dart
  void attachRootWidget(Widget rootWidget) {
    attachToBuildOwner(RootWidget( // ⬅️ 创建 RootWidget 实例对象
      debugShortDescription: '[root]',
      child: rootWidget, // ⬅️ 此处的 rootWidget 就是上面传递来的 View 对象
    ));
  }
```

## View.State.build

&emsp;此处调用的 attachToBuildOwner 函数，内部会执行 RootElement 的创建和挂载，即开启 Element Tree 的构建，它完成了 depth 等于 1 的节点的构建，然后是 RootWidget 的 child 的构建，即我们上面传递来的 View，它作为 depth 等于 2 的节点来构建，然后我们把目光转移到 View 中去，它是一个 StatefulWidget，所以我们直接找它的 State，看到它的 State.build 返回的 Widget 是 RawView，即它便是构建 depth 等于 3 的节点。

```dart
  // View 的 State 的 build 函数
  
  @override
  Widget build(BuildContext context) {
  
    // depth = 3 的 Element 对应的 Widget
    return RawView(
      view: widget.view,
      deprecatedDoNotUseWillBeRemovedWithoutNoticePipelineOwner: widget._deprecatedPipelineOwner,
      deprecatedDoNotUseWillBeRemovedWithoutNoticeRenderView: widget._deprecatedRenderView, ⬅️⚠️⬇️ Render Tree 根节点传递下去。
      child: ...
    );
    
  }
}
```

## RawView.build

&emsp;然后我们把目光转移到 RawView 内部去，它是一个 StatelessWidget，所以我们直接找它的 build 函数，看到它的 build 函数返回的 Widget 是：`_RawViewInternal`，即它便是构建 depth 等于 4 的节点，而它不仅如此，它还是 Render Tree 的根节点所对应的 Widget，并且通过它创建的 RenderObjectElement 节点的挂载，也正式开启了 Render Tree 的构建。

```dart
  // RawView 的 build 函数
  
  @override
  Widget build(BuildContext context) {
    return _RawViewInternal(
      view: view,
      deprecatedPipelineOwner: _deprecatedPipelineOwner,
      deprecatedRenderView: _deprecatedRenderView, ⬅️⚠️⬇️ Render Tree 根节点传递下去。
      builder: (BuildContext context, PipelineOwner owner) {
        return ...
      },
    );
  }
```

## `_RawViewInternal`

&emsp;然后我们把目光转移到 `_RawViewInternal` 内部去，看到它是一个 RenderObjectWidget 子类，它有自己的 createElement 和 createRenderObject 函数。它的 createElement 函数返回的 Element 是：`_RawViewElement` 对象，而它的 createRenderObject 函数返回的 RenderObject 是：`_deprecatedRenderView`，即我们前面一路传递下来的 RendererBinding.renderView。

&emsp;然后我们接着把目光转移到 `_RawViewElement.mount` 函数中去：

```dart
  @override
  void mount(Element? parent, Object? newSlot) {
    super.mount(parent, newSlot);
    
    // 把 RendererBinding.renderView 赋值给 RendererBinding.pipelineOwner.rootNode，
    // 即指定这颗 Render Tree 的根节点。 
    _effectivePipelineOwner.rootNode = renderObject;
    
    _attachView();
    
    // 这里会一直沿着 Widget 链路走下去，构建完整的 Widget Tree 和 Element Tree。
    _updateChild();
    
    // 然后这里，就是我们上篇看到的 为初始化首帧做准备！
    renderObject.prepareInitialFrame(); // ⬅️ 起点！
    
    if (_effectivePipelineOwner.semanticsOwner != null) {
      renderObject.scheduleInitialSemantics();
    }
  }
```

&emsp;然后我们直接在 `renderObject.prepareInitialFrame();` 行打一个断点，运行项目会被直接命中，然后我们可以沿着当前的 this 指针的 child 指向一直点击下去看每一级 Element 节点。

&emsp;此时已经完整构建了：Widget Tree（虽然没有实际的代码结构支撑，但是附庸在每个 Element 节点上，还是可以使用每个 Widget 对象组建出一颗完整的 Widget Tree 的）、Element Tree、Render Tree。

&emsp;因为 `_updateChild()` 函数是沿着 Widget 链一直往下构建的，直到遇到叶子节点为止。

# RenderView.prepareInitialFrame

&emsp;然后就是我们的 `renderObject.prepareInitialFrame();` 函数执行了，它通过准备第一帧来引导渲染管道。

&emsp;首先我们先看一下这个函数的内容，实际它的内容超简单，就是把 RendererBinding.renderView 添加到 RendererBinding.pipelineOwner 的 `_nodesNeedingLayout` 和 `_nodesNeedingPaint` 列表中。把 RendererBinding.renderView 的 `_relayoutBoundary` 属性置为自己。给 RendererBinding.renderView 的 `_layerHandle.layer` 指定一个 TransformLayer 对象。

```dart
  // RenderView 内部：
  
  void prepareInitialFrame() {
    scheduleInitialLayout();
    scheduleInitialPaint(_updateMatricesAndCreateNewRootLayer());
  }

  // 根据当前的配置创建一个 TransformLayer 实例对象，作为 Layer Tree 的根节点。
  TransformLayer _updateMatricesAndCreateNewRootLayer() {
    _rootTransform = configuration.toMatrix();
    
    // 直接创建一个 TransformLayer 实例对象
    final TransformLayer rootLayer = TransformLayer(transform: _rootTransform);
    rootLayer.attach(this);
    
    return rootLayer;
  }
```

```dart
  // RenderObject 内部： 
  
  void scheduleInitialLayout() {
    // 根节点的重新布局边界还是自己
    _relayoutBoundary = this;
    
    // 把自己添加到 需要布局列表 中，等待在新的一帧中执行布局。
    owner!._nodesNeedingLayout.add(this);
  }
  
  void scheduleInitialPaint(ContainerLayer rootLayer) {
    // 为其提供一个 Layer 
    _layerHandle.layer = rootLayer;
    
    // 把自己添加到 需要绘制列表 中，等待在新的一帧中执行绘制。
    owner!._nodesNeedingPaint.add(this);
  }
```

&emsp;到这里看似确实是一切准备就绪了，就等着 RendererBinding.drawFrame 执行了！

## 参考链接
**参考链接:🔗**
+ [RenderObject class](https://api.flutter.dev/flutter/rendering/RenderObject-class.html)
+ [RenderObjects?! | Decoding Flutter](https://www.youtube.com/watch?v=zmbmrw07qBc)
+ [PipelineOwner class](https://api.flutter.dev/flutter/rendering/PipelineOwner-class.html)
+ [RendererBinding mixin](https://api.flutter.dev/flutter/rendering/RendererBinding-mixin.html)
+ [PipelineManifold class](https://api.flutter.dev/flutter/rendering/PipelineManifold-class.html)
+ [RenderTreeRootElement class](https://api.flutter.dev/flutter/widgets/RenderTreeRootElement-class.html)
+ [RenderObjectElement class](https://api.flutter.dev/flutter/widgets/RenderObjectElement-class.html)
+ [View class](https://api.flutter.dev/flutter/widgets/View-class.html)
+ [RootWidget class](https://api.flutter.dev/flutter/widgets/RootWidget-class.html)
+ [RootElement](https://api.flutter.dev/flutter/widgets/RootElement-class.html)
