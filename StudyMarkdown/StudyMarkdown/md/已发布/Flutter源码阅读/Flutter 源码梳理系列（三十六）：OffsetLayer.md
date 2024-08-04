# OffsetLayer

&emsp;一个相对于其父图层显示的偏移图层。

&emsp;偏移图层对于高效的重绘至关重要，因为它们是由 RenderObject 树中的重绘边界（参见 RenderObject.isRepaintBoundary）创建的。当要求作为重绘边界的渲染对象在绘制上下文中的特定偏移处进行绘制时，该渲染对象首先检查自身是否需要重绘。如果不需要，它通过改变其偏移属性来重用现有的 OffsetLayer（以及整个子树），从而削减了绘制步骤。



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
