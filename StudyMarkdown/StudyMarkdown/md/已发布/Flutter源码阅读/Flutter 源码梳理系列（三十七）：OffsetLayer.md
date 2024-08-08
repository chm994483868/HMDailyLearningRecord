# Flutter 源码梳理系列（三十七）：OffsetLayer

# 前言

&emsp;本篇我们看一个特殊的 ContainerLayer 子类：OffsetLayer，在之前的内容中我们多次见到它，特别是在 PaintContext 中我们看到当进行 RenderObject 绘制时，默认情况下为是重绘边界的 RenderObject 的 layer 属性创建的就是 OffsetLayer 对象，特别是 OffsetLayer 的名字前面的 Offset 也暗示了一些东西，OffsetLayer 有一个 offset 属性，它记录的正是当前这个 OffsetLayer 相对于其父级 Layer 的偏移。

# OffsetLayer

&emsp;OffsetLayer：一个相对于其父级 Layer 显示的偏移图层。OffsetLayer 对于高效的重绘至关重要，因为它们是由 Render Tree 中的重绘边界（RenderObject 的 isRepaintBoundary 属性是 true）创建的。当要求作为重绘边界的 RenderObject 在绘制上下文中的特定偏移处进行绘制时，该 RenderObject 对象首先检查自身是否需要重绘。如果不需要，它通过改变其偏移属性来重用现有的 OffsetLayer（以及整个子树），从而削减了绘制步骤。（具体实现可见于 PaintingContext 的 `void _compositeChild(RenderObject child, Offset offset) { //...}` 函数。）

+ Object -> Layer -> ContainerLayer -> OffsetLayer

&emsp;我们之前学习的 Layer Tree 的根节点 TransformLayer 就是 OffsetLayer 的子类，还有 ImageFilterLayer 和 OpacityLayer 也是 OffsetLayer 子类。

## Constructors

&emsp;OffsetLayer 的构造函数，默认 offset 属性赋值为 Offset.zero。

```dart
class OffsetLayer extends ContainerLayer {
  /// 创建一个 OffsetLayer 对象，默认情况下， offset 为零。在 pipeline 的合成阶段之前，它必须是非空的。
  OffsetLayer({ Offset offset = Offset.zero }) : _offset = offset;
  // ...
}
```

## offset

&emsp;在父坐标系中相对于父级的偏移量。在更改此属性后，必须明确地重新组合场景（如 Layer 中所述）。在渲染流水线的合成阶段之前，offset 属性必须为非空。

```dart
  Offset get offset => _offset;
  Offset _offset;
  
  set offset(Offset value) {
    // 当设置了新的 offst 值后，会被标记为需要 AddToScene 中
    if (value != _offset) {
      markNeedsAddToScene();
    }
    
    _offset = value;
  }
```

## findAnnotations

&emsp;重写了父类的 findAnnotations 函数，主要把 localPosition 参数减去自己的 offset 属性后再进行搜索。

```dart
  @override
  bool findAnnotations<S extends Object>(AnnotationResult<S> result, Offset localPosition, { required bool onlyFirst }) {
    return super.findAnnotations<S>(result, localPosition - offset, onlyFirst: onlyFirst);
  }
```

## applyTransform

&emsp;重写了父类 ContainerLayer 的 applyTransform 函数。

```dart
  @override
  void applyTransform(Layer? child, Matrix4 transform) {
    transform.translate(offset.dx, offset.dy);
  }
```

## addToScene

&emsp;重写了父类 ContainerLayer 的 addToScene 方法。

```dart
  @override
  void addToScene(ui.SceneBuilder builder) {
  
    // Skia 中有一个快速路径，用于连接仅包含 缩放/平移 的矩阵。因此，推送仅包含平移的变换层应该很快。对于保留渲染，我们不想将偏移量推送到每个叶子节点。否则，在非常高级别更改偏移图层时，可能会导致太多叶子受到影响。
  
    // 先把当前 OffsetLayer 的 offset 属性推入 SceneBuilder 的操作堆栈中。
    engineLayer = builder.pushOffset(
      offset.dx,
      offset.dy,
      oldLayer: _engineLayer as ui.OffsetEngineLayer?,
    );
    
    // 然后把自己的子级加入 builder。
    addChildrenToScene(builder);
    
    // 然后再执行 pop，把上面推入的 Offset 效果给弹出 SceneBuilder 的操作堆栈。
    builder.pop();
  }
```

## `_createSceneForImage`

&emsp;私有函数，为下面的 toImage、toImageSnc 创建 Scene 使用，可看到主要集中在 offset 属性中，直接在当前的 SceneBuilder 中推入经过 translate Offset 的 Transform。

```dart
  ui.Scene _createSceneForImage(Rect bounds, { double pixelRatio = 1.0 }) {
    final ui.SceneBuilder builder = ui.SceneBuilder();
    
    // 可看到主要集中在 offset 属性中，直接在当前的 SceneBuilder 中推入经过 translate Offset 的 Transform。
    final Matrix4 transform = Matrix4.diagonal3Values(pixelRatio, pixelRatio, 1);
    transform.translate(-(bounds.left + offset.dx), -(bounds.top + offset.dy));
    
    builder.pushTransform(transform.storage);
    
    return buildScene(builder);
  }
```

## toImage

&emsp;捕获当前 OffsetLayer 及其子级的状态的图像。

&emsp;返回的 ui.Image 具有未压缩的原始 RGBA 字节，将由边界的左上角偏移，并具有尺寸等于边界尺寸乘以像素比(pixelRatio)的大小。

&emsp;像素比(pixelRatio)描述了逻辑像素与输出图像尺寸之间的比例。它与设备的 dart:ui.FlutterView.devicePixelRatio 独立，因此指定为 1.0（默认值）将为你提供逻辑像素和图像中输出像素之间的 1:1 映射。

&emsp;此 API 的功能类似于 toImageSync，但只在光栅化完成后返回。

&emsp;另请参见：

+ RenderRepaintBoundary.toImage 用于在渲染对象级别获取类似的 API。
+ dart:ui.Scene.toImage 获取有关返回图像的更多信息。

```dart
  Future<ui.Image> toImage(Rect bounds, { double pixelRatio = 1.0 }) async {
    final ui.Scene scene = _createSceneForImage(bounds, pixelRatio: pixelRatio);

    try {
    
      // 大小四舍五入一下。
      return await scene.toImage(
        (pixelRatio * bounds.width).ceil(),
        (pixelRatio * bounds.height).ceil(),
      );
      
    } finally {
      scene.dispose();
    }
  }
```

## toImageSync

&emsp;捕获当前 OffsetLayer 及其子级的状态的图像。

&emsp;返回的 ui.Image 具有未压缩的原始 RGBA 字节，将由边界的左上角偏移，并具有尺寸等于边界尺寸乘以像素比(pixelRatio)的大小。

&emsp;像素比(pixelRatio)描述了逻辑像素与输出图像尺寸之间的比例。它与设备的 dart:ui.FlutterView.devicePixelRatio 独立，因此指定为 1.0（默认值）将为你提供逻辑像素和图像中输出像素之间的 1:1 映射。

&emsp;这个 API 的功能类似于 toImage，只不过它会立即在光栅线程上急切地开始光栅化，然后在这之前返回图像。

&emsp;另请参见：

+ RenderRepaintBoundary.toImage 用于在渲染对象级别获取类似的 API。
+ dart:ui.Scene.toImage 获取有关返回图像的更多信息。

```dart
  ui.Image toImageSync(Rect bounds, { double pixelRatio = 1.0 }) {
    final ui.Scene scene = _createSceneForImage(bounds, pixelRatio: pixelRatio);

    try {
    
      // 大小四舍五入一下。
      return scene.toImageSync(
        (pixelRatio * bounds.width).ceil(),
        (pixelRatio * bounds.height).ceil(),
      );
      
    } finally {
      scene.dispose();
    }
  }
```

## OffsetLayer 总结

&emsp;OffsetLayer 作为是重新绘制边界的 RenderObject 对象进行绘制时默认提供的 Layer 类型，但其实它的内容并不多，主要集中在其 addToScene 函数的重写中。当需要把以当前 OffsetLayer 为根节点的 Layer 子树添加进场景时，首先会在当前 SceneBuilder 的操作堆栈中推入 offset 偏移，然后再把其子级添加到场景中，然后再执行个 SceneBuilder 的 pop 操作，防止影响后续的 Layer 添加到场景时的效果。

&emsp;OffsetLayer 相对于其直接父类 ContainerLayer 而言，主要多了一个 offset 属性，然后是重写的一些父类函数，多围着 offset 属性进行。

&emsp;然后另外一点时，PipelineOwner 的 flushPaint 函数中，当是重绘边界的 RenderObject 仅需要 Layer 更新时，会调用的 PaintingContext 的 updateLayerProperties 函数，仅更新 RenderObject 的 layer 的 offset 即可，可以直接复用此 layer 属性。

## 参考链接
**参考链接:🔗**
+ [OffsetLayer class](https://api.flutter.dev/flutter/rendering/OffsetLayer-class.html)
