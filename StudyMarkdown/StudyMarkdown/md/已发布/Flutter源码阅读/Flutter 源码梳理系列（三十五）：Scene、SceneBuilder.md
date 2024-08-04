# Flutter 源码梳理系列（三十五）：OffsetLayer

# 前言

&emsp;我们前一篇学习了 ContainerLayer，关于它的子级 Layer 的拼接以及双向链表结构的子级 Layer 列表的构建都已经很清楚了，然后再结合之前的 PaintingContext 篇的内容：RenderObject 的绘制、pushLayer 添加子级 Layer 等内容，相信大家已经对 Layer Tree 的结构基础以及构建过程都比较清晰了！

&emsp;但是 ContainerLayer 类中以及 Layer 基类中关于 Scene 的内容，如：buildScene、addToScene、addChildrenToScene、addToSceneWithRetainedRendering 等函数的功能，还是有点让人摸不着头脑，所以本篇我们主要来学习一个关于 Scene 和 SceneBuilder 的内容，学习一下跟场景相关的内容！   

&emsp;在我们前面学习 Layer 和 ContainerLayer 的内容时，多次见到与 Scene 相关的 API，一时间对 Scene 可能不好理解，其实它已经很接近最终的屏幕显示阶段了。下面我们梳理下 Paint 阶段的函数堆栈，找一下 Scene 对象是在哪里创建的。

&emsp;我们已知 RenderObject 的 Paint 的起点是在 RendererBinding.drawFrame 函数中对 rootPipelineOwner.flushPaint 的调用。它会带领我们进入 RenderObject 的 Paint 阶段，首先会以 Render Tree 的根节点的 layer 属性的值作为 Layer Tree 的根节点，然后沿着 Render Tree 上每个 RenderObject 对象的子级链进行每个 RenderObject 对象的绘制，当遇到是绘制边界的 RenderObject 时会为其创建单独的 Layer 节点，然后它的子级统一会绘制在这一个 Layer 上，以及遇到某个 RenderObject 对象的自定义 paint 函数中调用 PaintingContext 的 pushXXX 系列某个函数时，且其 needCompositing 属性为 true 时，也会为其创建单独的 Layer 节点进行绘制，并且会把每个新建的 Layer 节点连接起来，最终构建出一颗完整的 Layer Tree 来。

&emsp;然后我们回到 RendererBinding.drawFrame 函数，当 rootPipelineOwner.flushPaint 执行完毕后，会继续调用下面的 RenderView 的 compositeFrame 函数，即 Render Tree 的根节点的 compositeFrame 函数，在 RenderView.compositeFrame 内，会创建一个 SceneBuilder 对象，然后调用 Render Tree 的根节点的 layer.buildScene 函数，已知 Render Tree 的根节的 layer 属性是一个 TransformLayer 对象，即调用 TransformLayer.buildScene 函数，然后以返回的 Scene 对象为参数调用 FlutterView.render 函数，即把这以 Layer Tree 根节点生成的 Scene 送入了 Engine 层，最终经过 GPU 光栅化后显示在屏幕上。

&emsp;而自打调用 TransformLayer.buildScene 起，实际就是在整个 Layer Tree 上，调用每个 Layer 节点的 addToScene 函数，不同的 Layer 子类为了自己的绘制效果会重写自己的 addToScene 函数，不过整体看下来的话就是 Layer Tree 上的每个 Layer 节点都把自己添加到 RenderView.compositeFrame 函数起始创建的 SceneBuilder 对象中，最终通过 builder.build 函数，生成一个 Layer Tree 对应的 Scene 对象。

&emsp;而不同的 Layer 子类都重写了 addToScene 函数，我们全局搜 "void addToScene(ui.SceneBuilder builder) {" 可以看到不同的 Layer 子类的 addToScene 函数实现中基本都会把 builder.pushXXX 函数的返回值通过自己的 engineLayer 属性记录下来，而此 enginerLayer 属性的赋值即可以理解为把当前这 Layer 节点的绘制结果存储在它的 enginerLayer 属性中。方便后续的重绘时的重用。

&emsp;看完以上内容有点乱，下面我们看一下 Scene 和 SceneBuilder 中的内容来进行解惑。

# Scene

&emsp;在前面学习时，我们从来没有详细讲解过 Scene 和 SceneBuilder 的内容。其实主要是鉴于 Scene 和 SceneBuilder 是 Flutter framework 层到 Engine 层的桥接，它们真正的功能实现是在 Engine 层，在 Framework 层只能简单浏览一下它们的 API 看它们实现了什么功能而已。

&emsp;与 PictureRecorder、Picture、Canvas 类似，SceneBuilder、Scene 的实现类分别是：`_NativeSceneBuilder` 和 `_NativeScene`。

```dart
base class _NativeSceneBuilder extends NativeFieldWrapperClass1 implements SceneBuilder { //... }
base class _NativeScene extends NativeFieldWrapperClass1 implements Scene { //... }
```

&emsp;Scene：表示一个合成场景的不透明对象。要创建一个 Scene 对象，可以使用 SceneBuilder（可以以 SceneBuilder 对象为参数调用 ContainerLayer 的 ui.Scene buildScene(ui.SceneBuilder builder) 函数创建一个 Scene 对象。）。可以使用 FlutterView.render 方法在屏幕上显示 Scene 对象。

```dart
abstract class Scene { //... }
```

## toImageSync

&emsp;同步地从这个 Scene 中创建一个图片句柄（Image）。返回的 Image 将是 width 像素宽，height 像素高。Image 在 0(left)、0(top)、width(right)、height(bottom) 边界内被栅格化。超出这些边界的内容将被裁剪。

&emsp;Image 对象是同步创建并返回的，但是栅格化是异步进行的。如果栅格化失败，当 Image 绘制到 Canvas 上时会抛出异常。

&emsp;如果存在 GPU 上下文，则这个 Image 将被创建为 GPU 驻留项，并不会复制回 host。这意味着绘制这个 Image 会更有效率。如果没有 GPU 上下文可用，则该 Image 将在 CPU 上栅格化。

&emsp;之前学习的 Picture 也有类似的：toImage、toImageSync 函数，不知道它与 Scene 的 toImage、toImageSync 是什么关系。

```dart
  Image toImageSync(int width, int height);
```

## toImage

&emsp;创建 Scene 当前状态的光栅图像表示。这是一个在后台线程上执行的耗时操作。调用者在完成操作后必须处理图像。如果结果将与其他方法或类共享，应使用 Image.clone，并且必须处理（dispose）创建的每个句柄。

```dart
  Future<Image> toImage(int width, int height);
```

## dispose

&emsp;释放此 Scene 使用的资源。调用此函数后，Scene 将不能再被使用。

&emsp;这不能是一个叶子调用，因为 native 函数调用了 Dart API（Dart_SetNativeInstanceField）。

```dart
  void dispose();
```

## Scene 总结

&emsp;简单来说 Scene 对象由以 SceneBuilder 对象为参数调用 ContainerLayer 的 ui.Scene buildScene(ui.SceneBuilder builder) 函数生成，然后由 FlutterView 的 void render(Scene scene, {Size? size}) 函数将 Scene 对象显示到屏幕上。它本身的 toImage/toImageSync 几乎没有直接使用，如在 RenderView 的 compositeFrame 函数中：

```dart
  void compositeFrame() {
      // ...
      // 1️⃣ 准备一个 SceneBuilder 对象。
      final ui.SceneBuilder builder = RendererBinding.instance.createSceneBuilder();
      
      // 2️⃣ buildScene 内部会有一个所有 Layer 节点的 addToScene 的递归调用，
      // 即 Scene 是以 layer 为根节点的整个 Layer 子树的 build 的结果，并非仅是当前此一个 layer 对象。
      final ui.Scene scene = layer!.buildScene(builder);
      
      // _view 是 RenderView 的：final ui.FlutterView _view 属性。
      // 3️⃣ 然后直接调用 _view 的 render 函数，把其显示到屏幕上去。 
      _view.render(scene, size: configuration.toPhysicalSize(size));
      
      // 4️⃣ 然后释放 scene 对象持有的资源。
      scene.dispose();
      // ...
  }
```

&emsp;看到 Scene 对象从创建到释放资源一气呵成，连任何处理停顿都没有。我们可以直接把它理解为是整棵 Layer Tree 在屏幕上的显示效果的一个中间承接对象。

&emsp;在开始 SceneBuilder 的内容之前，我们先看一下 EngineLayer，最最重要的，EngineLayer 可不是 Layer 的子类！它也是同 PictureRecorder、Picture、Canvas、Scene、SceneBuiler 等一样是 Flutter framework 层到 Engine 层的桥接，它们真正的功能实现是在 Engine 层，在 Framework 层只能简单浏览一下它们的 API 看它们实现了什么功能而已。

# EngineLayer

&emsp;EngineLayer：Flutter framework 用来在帧之间持有和保留 EngineLayer 的一个句柄。（每个 Layer 都有一个 engineLayer 属性，可以直接把 EngineLayer engineLayer 理解为保存当前 Layer 对象渲染结果的，特别是在 Layer 的 addToSceneWithRetainedRendering 函数内，如果 Layer 的 needsAddToScene 为 false 且 engineLayer 属性非 null 时，SceneBuilder 会通过 addRetained 持有 engineLayer，直接复用之前的渲染结果。）

&emsp;然后针对不同的渲染结果 EngineLayer 有一众子类：BackdropFilterEngineLayer、ClipPathEngineLayer、ClipRectEngineLayer、ClipRRectEngineLayer、ColorFilterEngineLayer、ImageFilterEngineLayer、OffsetEngineLayer、OpacityEngineLayer、ShaderMaskEngineLayer、TransformEngineLayer。

&emsp;EngineLayer 本身仅有一个 dispose 的抽象函数。

## dispose

&emsp;dispose 用于释放此 EngineLayer 对象使用的资源。调用此方法后，该 EngineLayer 对象将不再可用。

&emsp;EngineLayer 间接保留特定于平台的图形资源。其中一些资源，如图像，可能会占用大量内存。为了避免将这些资源保留直到下一次垃圾回收，最好尽快释放将不再使用的 EngineLayer 对象。

&emsp;一旦此 EngineLayer 被 dispose，它将不再适用于作为 retained layer 使用，并且不得将其作为 oldLayer 参数传递给任何接受该参数的 SceneBuilder 方法。

&emsp;这个调用不可能是叶子调用，因为 native 函数调用 Dart API（Dart_SetNativeInstanceField）。

```dart
abstract class EngineLayer {
  void dispose();
}
```

&emsp;然后 EngineLayer 的 native 实现也是仅有一个 dispose 函数：

```dart
base class _NativeEngineLayer extends NativeFieldWrapperClass1 implements EngineLayer {
  // 这个类是由 engine 创建的，不应该直接实例化或继承。
  _NativeEngineLayer._();

  @override
  @Native<Void Function(Pointer<Void>)>(symbol: 'EngineLayer::dispose')
  external void dispose();
}
```

# SceneBuilder

&emsp;SceneBuilder：用于构建一个包含给定视觉元素的 Scene。然后可以使用 FlutterView.render 渲染 Scene。

&emsp;要在 Scene 上绘制图形操作，首先使用 PictureRecorder 和 Canvas 创建一个 Picture，然后使用 SceneBuilder 的 addPicture 将其添加到 Scene 中。

## Constructors

&emsp;SceneBuilder 工厂构造函数，直接返回 `_NativeSceneBuilder`。

```dart
abstract class SceneBuilder {
  factory SceneBuilder() = _NativeSceneBuilder;
  // ...
}

base class _NativeSceneBuilder extends NativeFieldWrapperClass1 implements SceneBuilder {
  // ...
}
```

## pushTransform

&emsp;将一个 transform 操作推送到操作堆栈中（operation stack）。

&emsp;在栅格化之前，给定的矩阵（matrix4）将会对对象进行 transform。

&emsp;如果 oldLayer 不为 null，则 engine 在渲染新 Layer 时会尝试重用为旧 Layer 分配的资源。这仅仅是一种优化，对渲染的准确性没有影响。

&emsp;将一个 EngineLayer 传递给 addRetained 方法或作为 push 方法的 oldLayer 参数都算作使用。一个 EngineLayer 在一个 Scene 中只能使用一次。例如，它不能同时传递给两个 push 方法，或者传递给一个 push 方法和 addRetained 方法。

&emsp;当一个 EngineLayer 被传递给 addRetained 方法时，所有后代 EngineLayer 在这个 Scene 中也被认为被使用了。对后代也适用同样的单次使用限制。

&emsp;当一个 EngineLayer 作为 push 方法的 oldLayer 参数被传递时，它将不能在接下来的帧中再次使用。如果想要继续重用与该 EngineLayer 相关的资源，可以将 push 方法返回的 EngineLayer 对象存储起来，并在下一帧中使用该对象而非原始对象。

```dart
  TransformEngineLayer pushTransform(
    Float64List matrix4, {
    TransformEngineLayer? oldLayer,
  });
```

## pushOffset

&emsp;将一个偏移操作（Offset）推送到操作堆栈中（operation stack）。这等同于使用仅包含平移的矩阵进行 pushTransform 调用。

```dart
  OffsetEngineLayer pushOffset(
    double dx,
    double dy, {
    OffsetEngineLayer? oldLayer,
  });
```

## pushClipRect

&emsp;将一个矩形裁剪操作推送到操作堆栈中（operation stack）。给定矩形之外的栅格化内容会被丢弃。默认情况下，裁剪将采用反锯齿（clip = Clip.antiAlias）。

```dart
  ClipRectEngineLayer pushClipRect(
    Rect rect, {
    Clip clipBehavior = Clip.antiAlias,
    ClipRectEngineLayer? oldLayer,
  });
```

## pushClipRRect

&emsp;将一个圆角矩形裁剪操作推送到操作堆栈中（operation stack）。给定圆角矩形之外的栅格化内容会被丢弃。

```dart
  ClipRRectEngineLayer pushClipRRect(
    RRect rrect, {
    Clip clipBehavior = Clip.antiAlias,
    ClipRRectEngineLayer? oldLayer,
  });
```

## pushClipPath

&emsp;将一个路径裁剪操作推送到操作堆栈中（operation stack）。给定路径之外的栅格化内容会被丢弃。

```dart
  ClipPathEngineLayer pushClipPath(
    Path path, {
    Clip clipBehavior = Clip.antiAlias,
    ClipPathEngineLayer? oldLayer,
  });
```

## pushOpacity

&emsp;将一个不透明度操作推送到操作堆栈中（operation stack）。给定的 alpha 值被混合到对象的光栅化的 alpha 值中。一个 alpha 值为 0 会使对象完全不可见。一个 alpha 值为 255 则没有效果（即，对象保持当前透明度）。

```dart
  OpacityEngineLayer pushOpacity(
    int alpha, {
    Offset? offset = Offset.zero,
    OpacityEngineLayer? oldLayer,
  });
```

## pushColorFilter

&emsp;将一个颜色滤镜（ColorFilter）操作推送到操作堆栈中（operation stack）。给定的颜色会使用给定的混合模式应用于对象的光栅化。

```dart
  ColorFilterEngineLayer pushColorFilter(
    ColorFilter filter, {
    ColorFilterEngineLayer? oldLayer,
  });
```

## pushImageFilter

&emsp;将一个图像的滤镜操作推送到操作堆栈中（operation stack）。在将子元素合成到 Scene 中之前，会将给定的滤镜应用于子元素的栅格化。

```dart
  ImageFilterEngineLayer pushImageFilter(
    ImageFilter filter, {
    Offset offset = Offset.zero,
    ImageFilterEngineLayer? oldLayer,
  });
```

## pushBackdropFilter

&emsp;将一个背景滤镜操作推送到操作堆栈中（operation stack）。给定的滤镜被应用于 Scene 当前内容直至最近保存图层，并在栅格化子图层之前使用指定的混合模式将其渲染回 Scene。

```dart
  BackdropFilterEngineLayer pushBackdropFilter(
    ImageFilter filter, {
    BlendMode blendMode = BlendMode.srcOver,
    BackdropFilterEngineLayer? oldLayer,
  });
```

## pushShaderMask

&emsp;将一个着色器蒙版操作推送到操作堆栈中（operation stack）。使用给定的混合模式，将给定的着色器应用于给定矩形中对象的栅格化过程。

```dart
  ShaderMaskEngineLayer pushShaderMask(
    Shader shader,
    Rect maskRect,
    BlendMode blendMode, {
    ShaderMaskEngineLayer? oldLayer,
    FilterQuality filterQuality = FilterQuality.low,
  });
```

## pop

&emsp;终止最近推送的操作的效果。在内部，SceneBuilder 维护一个操作堆栈。堆栈中的每个操作都应用于 Scene 中添加的每个对象。调用此函数会从堆栈中移除最近添加的操作。（类似 Canvas 的 save 和 restore 操作。）

```dart
  void pop();
```

## addRetained

&emsp;添加一个来自之前帧的 retainedLayer 子树。所有位于 retainedLayer 子树中的 EngineLayer 将自动附加到当前的 EngineLayer Tree 中。

&emsp;因此，在实现 Flutter framework 的渲染层中定义的 Layer 概念的子类时，一旦调用此方法，就无需为其子级 Layer 调用 Layer.addToScene。（如：Layer 的 addToSceneWithRetainedRendering 函数中所示，调用过 addRetained 函数后直接 return。）

```dart
  void addRetained(EngineLayer retainedLayer);
```

## addPerformanceOverlay

&emsp;向 Scene 中添加一个显示性能统计信息的对象。在开发过程中对应用程序的性能进行评估很有用。enabledOptions 控制显示哪些统计信息。bounds 控制统计信息显示在哪里。

&emsp;enabledOptions 是一个 bits 字段，具有以下位定义：

+ 0x01: displayRasterizerStatistics - 显示光栅线程帧时间
+ 0x02: visualizeRasterizerStatistics - 绘制光栅线程帧时间图
+ 0x04: displayEngineStatistics - 显示 UI 线程帧时间
+ 0x08: visualizeEngineStatistics - 绘制 UI 线程帧时间图 将 enabledOptions 设置为 0x0F 以启用所有当前定义的功能。

&emsp;"UI 线程" 是包括 main Dart isolate 中所有执行（能够调用 FlutterView.render 的 isolate）的线程。UI 线程帧时间是执行 PlatformDispatcher.onBeginFrame 回调所花费的总时间。"光栅线程（raster thread）" 是（在 CPU 上运行的）线程，随后处理 Dart 代码提供的 Scene，将其转换为 GPU 命令并将其发送到 GPU。

```dart
  void addPerformanceOverlay(int enabledOptions, Rect bounds);
```

## addPicture

&emsp;在 Scene 中添加一个 Picture。Picture 在给定的偏移处被栅格化。

&emsp;绘制可能会被缓存以降低绘制 Picture 的成本，如果 Picture 在后续帧中被重复使用。一个 Picture 是否被缓存取决于后端实现。当考虑缓存时，是否缓存取决于一种基于图片绘制频率和绘制成本的启发式方法。要禁用这种缓存，请将 willChangeHint 设置为 true。要强制进行缓存（在支持缓存的后端中），请将 isComplexHint 设置为 true。当两者都设置时，willChangeHint 优先。

&emsp;一般情况下，设置这些提示并不是非常有用的。只有在已经渲染过三次的图片才会被后端缓存；因此，将 willChangeHint 设置为 true 以避免缓存每帧都在变化的动画图片是多余的，本来这种图片也不会被缓存。同样，后端缓存图片时会相对积极，以至于任何足够复杂需要缓存的图片可能已经被缓存，即使未将 isComplexHint 设置为 true。

```dart
  void addPicture(
    Offset offset,
    Picture picture, {
    bool isComplexHint = false,
    bool willChangeHint = false,
  });
```

## addTexture

&emsp;向 Scene 添加一个后端纹理。该纹理将被缩放到给定的大小，并在给定的偏移处进行栅格化。

&emsp;如果 freeze 为 true，则添加到 Scene 中的纹理将不会随新帧更新。在调整嵌入式 Android 视图大小时使用 freeze：调整 Android 视图大小时，存在一个短暂时期，framework 无法确定最新的纹理帧是先前的还是新的大小，为解决此问题，framework 会在调整 Android 视图大小之前 "freezes" 纹理，而在确信具有新尺寸的帧准备就绪时解除 freezes。

```dart
  void addTexture(
    int textureId, {
    Offset offset = Offset.zero,
    double width = 0.0,
    double height = 0.0,
    bool freeze = false,
    FilterQuality filterQuality = FilterQuality.low,
  });
```

## addPlatformView

&emsp;将一个平台视图（例如一个 iOS UIView）添加到 Scene 中。在 iOS 上，这个层将当前的输出表面分成两个表面，一个用于平台视图之前的场景节点，另一个用于平台视图之后的场景节点。

### Performance impact

&emsp;增加一个额外的表面会直接导致 Flutter 用于输出缓冲区的图形内存量翻倍。Quartz 可能会为合成 Flutter 表面和平台视图分配额外的缓冲区。

&emsp;在场景中带有平台视图时，Quartz 必须合成两个 Flutter 表面和嵌入的 UIView。此外，在 iOS 版本大于 9 时，Flutter 帧与 UIView 帧进行了同步，增加了额外的性能开销。

&emsp;在 iOS 和 Android 中，offset 参数都不被使用。

```dart
  void addPlatformView(
    int viewId, {
    Offset offset = Offset.zero,
    double width = 0.0,
    double height = 0.0,
  });
```

## build

&emsp;完成构建 Scene。返回一个包含已添加到此 SceneBuilder 中的对象的 Scene。然后可以使用 FlutterView.render 在屏幕上显示该 Scene。调用此函数后，SceneBuilder 对象将无效，并且不能再继续使用。

```dart
  Scene build();
```

## SceneBuilder 总结

&emsp;SceneBuilder 的一众 pushXXX 和 addXXX 函数的使用正与 layer.dart 中的一众 Layer 和 ContainerLayer 子类的 addToScene 函数相对应，如：

+ PictureLayer 重写的 Layer 的 addToScene 函数中直接调用 SceneBuilder 的 addPicture 函数，把 PictureLayer 的 picture 属性添加到场景构建中。
+ TextureLayer 重写的 Layer 的 addToScene 函数中直接调用 SceneBuilder 的 addTexture 函数，把 TextureLayer 的 textureId 属性添加到场景构建中。

&emsp;等等，还有下面的 ContainerLayer 子类中对 SceneBuilder 的 pushXXX 系列函数的使用，它们较 Layer 子类中的 SceneBuilder 的 addXXX 函数的使用复杂一些，ContainerLayer 子类调用 pushXXX 函数后，会用自己的 engineLayer 属性接收着 pushXXX 的返回值，然后递归在自己的子级 Layer 中调用 addToScene 函数，然后最后再执行 SceneBuilder 的 pop 函数。 

+ OffsetLayer 重写的 ContainerLayer 的 addToScene 函数中直接调用 SceneBuilder 的 pushOffset，把自己的 offset 属性以及 engineLayer 添加到场景构建中，并且使用自己的 engineLayer 属性接收 pushOffset 函数的返回值，然后调用 addChildrenToScene 函数把自己的子级 Layer 添加到场景构建中，并在末尾调用 SceneBuilder 的 pop 函数，进行操作效果的复原，不会影响接下来的其它 pushXXX 效果的添加。
+ 类似 OffsetLayer 的 ClipRectLayer 的 addToScene 函数中对 SceneBuilder 的 pushClipRect 函数的使用。
+ 类似 OffsetLayer 的 ColorFilterLayer 的 addToScene 函数中对 SceneBuilder 的 pushColorFilter 函数的使用。

&emsp;等等，即 SceneBuilder 的 pushXXX 和 addXXX 函数正是把不同的 Layer 子类节点添加到 Scene 构建中。

&emsp;SceneBuilder 和 Scene 完成了 Flutter UI 显示之前的最后一环，随着 Scene 通过 FlutterView.render 的调用在屏幕上显示该 Scene，完成最终的 Flutter UI 在屏幕上的显示。

## 参考链接
**参考链接:🔗**
+ [Layer class](https://api.flutter.dev/flutter/dart-ui/Canvas-class.html)
+ [ContainerLayer class](https://api.flutter.dev/flutter/rendering/ContainerLayer-class.html)
+ [Scene class](https://api.flutter.dev/flutter/dart-ui/Scene-class.html)
+ [SceneBuilder class](https://api.flutter.dev/flutter/dart-ui/SceneBuilder-class.html)
