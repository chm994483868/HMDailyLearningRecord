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






```dart
// 1️⃣ Layer 中和 Scene 相关的代码。
  bool _needsAddToScene = true;

  @protected
  @visibleForTesting
  void markNeedsAddToScene() {
    if (_needsAddToScene) {
      return;
    }

    _needsAddToScene = true;
  }

  @protected
  bool get alwaysNeedsAddToScene => false;
  
    @protected
  @visibleForTesting
  void updateSubtreeNeedsAddToScene() {
    _needsAddToScene = _needsAddToScene || alwaysNeedsAddToScene;
  }
  
  @protected
  void addToScene(ui.SceneBuilder builder);

  void _addToSceneWithRetainedRendering(ui.SceneBuilder builder) {
    // 如果不需要 add to Scene 并且 _engineLayer 不为 null，
    // 则直接拿当前的 _engineLayer 进行复用。
    if (!_needsAddToScene && _engineLayer != null) {
    
      // 需要专注看一下 builder.addRetained 的内容。
      builder.addRetained(_engineLayer!);
      
      return;
    }
    
    // 正常进行 addToScene 的调用。
    addToScene(builder);
    
    _needsAddToScene = false;
  }
  
// 2️⃣ ContainerLayer 中和 Scene 相关的代码。
  ui.Scene buildScene(ui.SceneBuilder builder) {
    updateSubtreeNeedsAddToScene();
    
    addToScene(builder);
    
    if (subtreeHasCompositionCallbacks) {
      _fireCompositionCallbacks(includeChildren: true);
    }

    _needsAddToScene = false;
    
    // 生成 Scene
    final ui.Scene scene = builder.build();
    
    return scene;
  }

  @override
  void updateSubtreeNeedsAddToScene() {
    super.updateSubtreeNeedsAddToScene();
    
    Layer? child = firstChild;
    while (child != null) {
      child.updateSubtreeNeedsAddToScene();
      
      _needsAddToScene = _needsAddToScene || child._needsAddToScene;
      
      child = child.nextSibling;
    }
  }
  
  @override
  void addToScene(ui.SceneBuilder builder) {
    addChildrenToScene(builder);
  }

  void addChildrenToScene(ui.SceneBuilder builder) {
    Layer? child = firstChild;
    
    while (child != null) {
      // 直接使用 Layer 的 _addToSceneWithRetainedRendering 函数
      child._addToSceneWithRetainedRendering(builder);
      
      child = child.nextSibling;
    }
  }
```
