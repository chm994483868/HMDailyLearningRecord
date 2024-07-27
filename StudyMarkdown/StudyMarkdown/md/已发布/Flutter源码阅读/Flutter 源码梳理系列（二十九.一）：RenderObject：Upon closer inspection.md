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

&emsp;下面我们沿着 runApp 函数调用梳理一下：Widget Tree 和 Element Tree 的构建，这里我们秉承找 Widget 下级的原则：

+ 如果看到一个 Widget 是 StatelessWidget 的话直接看它的 build 函数，此 build 函数返回的便是它的下一个 Widget。
+ 如果看到一个 Widget 是 StatefulWidget 的话直接看它的 State 的 build 函数，此 build 函数返回的便是下一个 Widget。
 
## runApp

&emsp;Flutter 项目启动，执行 runApp 函数，

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
      deprecatedDoNotUseWillBeRemovedWithoutNoticePipelineOwner: pipelineOwner,
      deprecatedDoNotUseWillBeRemovedWithoutNoticeRenderView: renderView,
      child: rootWidget,
    );
  }
  
  late final RenderView renderView = _ReusableRenderView(
    view: platformDispatcher.implicitView!,
  );
```

| **depth**           | **Widget**  | **Element**  | **RenderObject**  |
| :-----------------: | :---------- | :----------- | :---------------- |
  |  **1**  |  RootWidget：`class RootWidget extends Widget {}`         | RootElement：`class RootElement extends Element with RootElementMixin {}`  | - |
  |  **2**  |  View：`class View extends StatefulWidget {}` | - | - |
  |  **3**  |  RawView：`class RawView extends StatelessWidget {}` | - | - |
  |  **4**  |  _RawViewInternal：`class _RawViewInternal extends RenderObjectWidget {}` | _RawViewElement：`class _RawViewElement extends RenderTreeRootElement {}` | _ReusableRenderView：`class _ReusableRenderView extends RenderView {}` |









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
