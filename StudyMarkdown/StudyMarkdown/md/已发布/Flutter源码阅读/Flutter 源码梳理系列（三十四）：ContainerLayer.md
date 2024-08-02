# Flutter 源码梳理系列（三十四）：ContainerLayer

&emsp;ContainerLayer 作为 PaintingContext 构造函数的必传参数，我们来学习一下。只有 ContainerLayer 的子类可以在 Layer Tree 中拥有子级，所有其他层类都用作层树中的叶子节点。

# ContainerLayer

&emsp;ContainerLayer 是一个具有子节点列表的 Layer 子类（这里注意一下，子 Layer 列表的组织形式不像是 MultiChildRenderObjectElement 的子级 Element 列表是一个 List 类型的属性：`late List<Element> _children`，ContainerLayer 的子 Layer 列表是一个双向链表的结构，而这个双向链表的结构的代码实现就是通过 Layer 的 previousSibling 和 nextSibling 两个属性来实现的）。ContainerLayer 实例仅接受一个子 Layer 列表，并按顺序将它们插入到复合渲染中。有一些 ContainerLayer 的子类在此过程中应用更复杂的效果。

## Constructors

&emsp;ContainerLayer 直接继承自 Layer，并且作为 PaintingContext 的必传参数，所以它是我们在上层开发中会真实使用的类，下面一起来看一下它的其它内容。

```dart
class ContainerLayer extends Layer {
  // ...
}
```

## `_fireCompositionCallbacks`

&emsp;在 ContainerLayer 的 buildScene 和 detach 函数中会执行此函数，即当进行 Layer 的合成时进行回调，此函数是重写的 Layer 类的，到了 ContainerLayer 这里，则还需要沿着自己的子级链对所有子级进行遍历，执行它们的 `_fireCompositionCallbacks` 函数。

```dart
  @override
  void _fireCompositionCallbacks({required bool includeChildren}) {
    super._fireCompositionCallbacks(includeChildren: includeChildren);
    
    // 如果入参 includeChildren 为 true，即不包含自己的子级的话，直接 return。
    if (!includeChildren) {
      return;
    }
    
    // 遍历自己的子级 Layer，调用它们的 _fireCompositionCallbacks 函数，
    // 即沿着子级链表进行。
    Layer? child = firstChild;
    
    while (child != null) {
      // 执行子级的 _fireCompositionCallbacks 函数
      child._fireCompositionCallbacks(includeChildren: includeChildren);
      
      // 继续遍历下一个子级
      child = child.nextSibling;
    }
  }
```

## firstChild & lastChild & hasChildren

&emsp;firstChild 是此 ContainerLayer 子级列表中的第一个 Layer。lastChild 是此 ContainerLayer 子级列表中的最后一个 Layer。ContainerLayer 的子级列表其实是以一个双向链表的形式存在的，并且这里还用两个指针分别执行链表头和链表尾，后面看 append 函数时，会详细看到此双向链表的构建过程。

&emsp;hasChildren getter，只要 `_firstChild` 不为 null，说明此 ContainerLayer 至少有一个子级 Layer。 

```dart
  Layer? get firstChild => _firstChild;
  Layer? _firstChild;

  Layer? get lastChild => _lastChild;
  Layer? _lastChild;

  // 即只要 _firstChild 不为 null，说明此 ContainerLayer 至少有一个子级 Layer。 
  bool get hasChildren => _firstChild != null;
```

## supportsRasterization

&emsp;需要此 ContainerLayer 的所有子级 Layer 都支持光栅化，那么此 ContainerLayer 才支持光栅化。

&emsp;这个 Layer 或任何子 Layer 是否可以使用 Scene.toImage 或 Scene.toImageSync 进行栅格化（栅格化的意思是是否可以转换为位图）。如果为 false，则调用上述方法可能会产生不完整的图像。

&emsp;这个值可能会在 ContainerLayer 对象的生命周期中发生变化，因为它的子 Layer 本身会被添加或移除。

```dart
  @override
  bool supportsRasterization() {
    
    // 从 lastChild 往前遍历，只要有一个子图层不支持光栅化，那么此 ContainerLayer 就不支持光栅化。 
    for (Layer? child = lastChild; child != null; child = child.previousSibling) {
      
      // 只要 ContainerLayer 有一个 child 不支持光栅化，则直接返回 false。 
      if (!child.supportsRasterization()) {
        return false;
      }
    }
    
    // 都支持的话，那么此 ContainerLayer 也支持光栅化。
    return true;
  }
```

## buildScene

&emsp;把这个 ContainerLayer 当作根层，在 engine 中构建一个 Scene。

&emsp;这个方法位于 ContainerLayer 类中，而不是 PipelineOwner 或其他单例级别，是因为这个方法既可以用于渲染整个 Layer Tree（例如普通应用程序帧），也可以用于渲染 Layer SubTree（例如 OffsetLayer.toImage）。

```dart
  ui.Scene buildScene(ui.SceneBuilder builder) {
    // 更新以当前 ContainerLayer 为根的整棵 Layer 子树的 _needsAddToScene 属性，
    // 且同我们之前学习的 RenderObject 的 needsCompositing 的更新方式类似，当子级为 true 时，父级也要为 true。
    // 这里则是子级 Layer 的 needsAddToScene 为 true 时，父级 Layer 的 needsAddToScene 也必为 true。 
    updateSubtreeNeedsAddToScene();
    
    // addToScene 是 Layer 的一个抽象函数，需要它的子类来实现。
    // 这里 ContainerLayer 对它进行了自己的实现，即执行所有子级的 addToScene 函数。
    addToScene(builder);
    
    // 如果有的话，执行所有子级的合成回调。
    if (subtreeHasCompositionCallbacks) {
      _fireCompositionCallbacks(includeChildren: true);
    }
    
    // 在调用 addToScene 之后再清除 needsAddToScene 标记，而不是在调用之前。
    // 这是因为 addToScene 会调用子节点的 addToScene 方法，这可能会将该 CoontainerLayer 的 needsAddToScene 标记为 true。
    _needsAddToScene = false;
    
    // 生成一个 scene
    final ui.Scene scene = builder.build();
    return scene;
  }
```

## dispose

&emsp;当 ContainerLayer 执行 dispose 时，需要先移除自己的所有子级 Layer，然后再清理自己的 callbacks。

```dart
  @override
  void dispose() {
    removeAllChildren();
    _callbacks.clear();
    
    super.dispose();
  }
```

## updateSubtreeNeedsAddToScene

&emsp;从这个 ContainerLayer 开始遍历整棵 Layer 子树，并确定是否需要 addToScene。

&emsp;如果满足以下任一条件，则图层需要 addToScene：

+ alwaysNeedsAddToScene 为 true。
+ 已调用 markNeedsAddToScene。
+ 任何后代图层需要 addToScene。

&emsp;ContainerLayer 重写此方法以递归地在其子级 Layer 上调用它。

```dart
  @override
  void updateSubtreeNeedsAddToScene() {
    super.updateSubtreeNeedsAddToScene();
    
    // 遍历所有的子级 Layer，并且如下子级 needsAddToScene 为 true，则父级也必为 true。
    Layer? child = firstChild; 
    
    while (child != null) {
      child.updateSubtreeNeedsAddToScene();
      
      // 子级 _needsAddToScene 为 true，则自己的 _needsAddToScene 也必为 true
      _needsAddToScene = _needsAddToScene || child._needsAddToScene;
      
      child = child.nextSibling;
    }
  }
```

## addChildrenToScene

&emsp;将该 ContainerLayer 的所有子级 Layer 上传到 engine。

&emsp;通常，此方法由 addToScene 使用，以将子级 Layer 插入到 Scene 中。ContainerLayer 的子类通常会重写 addToScene 方法，利用 SceneBuilder API 对 Scene 应用效果，然后使用 addChildrenToScene 将它们的 子级 Layer  插入到 Scene 中，在从 addToScene 返回之前撤销上述效果。

```dart
  void addChildrenToScene(ui.SceneBuilder builder) {
    Layer? child = firstChild;
    
    // 沿着 firstChild 遍历所有的子级，调用它们的 _addToSceneWithRetainedRendering 函数。
    while (child != null) {
      child._addToSceneWithRetainedRendering(builder);
      
      child = child.nextSibling;
    }
  }
```

## attach

&emsp;将这个 ContainerLayer 标记为已附加到给定的 owner。

&emsp;具有子级的 Layer 子类应该重写此方法，在调用继承的方法之后，通过 super.attach(owner) 来将所有子节点都附加到同一个 owner 上。

```dart
  @override
  void attach(Object owner) {
    super.attach(owner);
    
    Layer? child = firstChild;
    // 遍历整个 child 链递归调用。
    while (child != null) {
      child.attach(owner);
      
      child = child.nextSibling;
    }
  }
```

## detach

&emsp;将该 ContainerLayer 标记为与其 owner 脱离关系。

&emsp;具有子级的 Layer 子类应该重写此方法，在调用继承的方法之后将所有子节点脱离，如 super.detach()。

```dart
  @override
  void detach() {
    super.detach();
    
    // 沿着 child 链递归调用子级的 detach 函数
    Layer? child = firstChild;
    while (child != null) {
      child.detach();
      child = child.nextSibling;
    }
    
    _fireCompositionCallbacks(includeChildren: false);
  }
```

## append

&emsp;

```dart
  void append(Layer child) {
    _adoptChild(child);
    
    child._previousSibling = lastChild;
    if (lastChild != null) {
      lastChild!._nextSibling = child;
    }
    
    _lastChild = child;
    _firstChild ??= child;
    child._parentHandle.layer = child;
  }
```

## applyTransform

&emsp;应用将应用于给定矩阵的子层的变换。

&emsp;具体来说，这应该应用于子层原点的变换。当使用一系列层级调用 applyTransform 时，除非链中最深的层将 layerOffset 折叠到零，即向其子层传递 Offset.zero，并将任何传入的 layerOffset 作为 transform（例如）混合到 SceneBuilder 中（然后还包含在 applyTransform 应用的变换中），否则结果将不可靠。

例如，如果 addToScene 应用 layerOffset，然后将 Offset.zero 传递给子层，则它应包含在此处应用的变换中；而如果 addToScene 只是将 layerOffset 传递给子层，则不应包含在此处应用的变换中。

此方法仅在调用 addToScene 之后即刻有效，在更改任何属性之前。

默认实现什么也不做，因为默认情况下 ContainerLayer 在 ContainerLayer 本身的原点处合成其子层。

child 参数通常不应为 null，因为原则上每个层都可以独立地转换每个子层。但是，某些层可能显式地允许 null 作为值，例如如果它们知道它们将所有子层都以相同方式转换。

供 FollowerLayer 使用，将其子层转换为 LeaderLayer 所在位置。

```dart
  void applyTransform(Layer? child, Matrix4 transform) {
    assert(child != null);
  }
```



# OffsetLayer

&emsp;一个相对于其父图层显示的偏移图层。

&emsp;偏移图层对于高效的重绘至关重要，因为它们是由 RenderObject 树中的重绘边界（参见 RenderObject.isRepaintBoundary）创建的。当要求作为重绘边界的渲染对象在绘制上下文中的特定偏移处进行绘制时，该渲染对象首先检查自身是否需要重绘。如果不需要，它通过改变其偏移属性来重用现有的 OffsetLayer（以及整个子树），从而削减了绘制步骤。
