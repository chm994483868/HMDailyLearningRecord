# Flutter 源码梳理系列（三十四）：ContainerLayer

&emsp;ContainerLayer 作为 PaintingContext 构造函数的必传参数，我们来学习一下。只有 ContainerLayer 的子类可以在 Layer Tree 中拥有子级，所有其他 Layer 类都用作 Layer Tree 中的叶子节点。

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

## attach

&emsp;将这个 ContainerLayer 标记为已附加到给定的 owner。

&emsp;具有子级的 Layer 子类应该重写此方法，在调用继承方法 super.attach(owner) 后将所有子级连接到相同的 owner 上。

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

&emsp;具有子级的 Layer 子类应该重写此方法，在调用继承方法 super.detach() 后将所有子级从 owner 脱离。

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
    
    // 并且要回调自己的 _fireCompositionCallbacks，includeChildren 入参为 false，
    // 表示只调用自己的。
    _fireCompositionCallbacks(includeChildren: false);
  }
```

## `_adoptChild`

&emsp;`_adoptChild` 作为把入参 child 添加作为当前 ContainterLayer 的子级所使用的一个私有函数，主要的功能是为入参 child 的各个属性赋值。

```dart
  void _adoptChild(Layer child) {
    // 如果当前 ContainerLayer 的 alwaysNeedsAddToScene 值不为 false，
    // 则需要把它标记为需要进行 addToScene。（因为它添加了新的子级，所以肯定要进行新的 addToScene，生成新的位图！）
    if (!alwaysNeedsAddToScene) {
      markNeedsAddToScene();
    }
    
    // 如果入参 child 的合成回调的数量不等于 0，则需要更新当前这个 ContainerLayer 的合成回调的数量，
    // 即在原有的合成回调的数量上，再加上当前这个入参 child 的合成回调的数量。
    if (child._compositionCallbackCount != 0) {
      _updateSubtreeCompositionObserverCount(child._compositionCallbackCount);
    }
    
    // 把入参 child 的 parent 指针，执行当前的 ContainerLayer
    child._parent = this;
    
    // 如果当前 ContainerLayer 的 attached 为 true，即它的 owner 不为 null，
    // 则也更新入参 child 的 owner 为此 owner 属性。
    if (attached) {
      child.attach(_owner!);
    }
    
    // 然后更新以入参 child 为根节点的，整个 Layer 子树上各个节点的 depth 值。
    redepthChild(child);
  }
```

## append

&emsp;将给定的入参 Layer child 添加到该 ContainerLayer 的子级列表的末尾。

&emsp;还记得 PaintingContext 的 appendLayer 函数吗？内部就是直接调用 PaintingContext 的 `_containerLayer` 属性的 append 函数，正是使用此函数完成新的 Layer 层的添加，既而进行 Layer Tree 的构建。

&emsp;在学习 PaintingContext 的 appendLayer 函数时并没有深入学习 append 函数，下面看一下它的具体实现内容。

```dart
  void append(Layer child) {
    // 把入参 child 添加作为当前 ContainerLayer 的子级。
    _adoptChild(child);
    
    // 然后，下面主要是更新当前 ContainerLayer 的 _lastChild 和 _firstChild 两个属性的值，
    // 以及入参 child 的 _previousSibling 属性，把它与它的兄弟节点连接起来。
    //（ContainerLayer 子级列表中的 Layer 节点，它们兄弟节点是一个双向链表。）
    
    child._previousSibling = lastChild;
    if (lastChild != null) {
      lastChild!._nextSibling = child;
    }
    
    _lastChild = child;
    _firstChild ??= child;
    child._parentHandle.layer = child;
  }
```

## redepthChildren & redepthChild

&emsp;redepthChildren：如果该节点有子级节点，请调整子级节点的深度信息：`_depth`。在具有子级节点的 Layer 子类中重写此方法，对每个子级节点调用 ContainerLayer.redepthChild，请勿直接调用此方法。

&emsp;redepthChild：调整给定子级节点的深度，使其大于此节点自身的深度。只能在重写 redepthChildren 方法时调用此方法。

```dart
  @override
  void redepthChildren() {
    // 沿着 firstChild 和 nextSibling 更新节点的 depth 信息。
    Layer? child = firstChild;
    while (child != null) {
      redepthChild(child);
      child = child.nextSibling;
    }
  }

  @protected
  void redepthChild(Layer child) {
    // 如果入参 child 的 depth 小于当前的它的直接父级的 depth 的话，
    // 那肯定要更新它的 depth 的值，更新为当前它的直接父级的 depth + 1。
    if (child._depth <= _depth) {
    
      // 更新为直接父级的 depth + 1
      child._depth = _depth + 1;
      
      // 然后同样的要更新自己的子级的 depth。
      child.redepthChildren();
    }
  }
```

## `_removeChild` & `_dropChild`

&emsp;在 Layer.remove 函数中使用到的函数，从父级的子级列表中移除指定的某个子级。

```dart
  void _removeChild(Layer child) {
  
    // 首先更新当前 ContainerLayer 的子级列表的双向链表，把 child 从此链表中移除。 
    
    // 判断入参 child 的 _previousSibling 是否 null，更新 _firstChild 指向。
    if (child._previousSibling == null) {
      // 如果入参 child 的 _previousSibling 为 null，则说明它是当前 ContainerLayer 的 _firstChild 指向，
      // 则更新 _firstChild 为 child 的 _nextSibling，即 child 的后一个兄弟节点作为当前 ContainerLayer 的 _firstChild。
      _firstChild = child._nextSibling;
    } else {
      // 否则的话，则更新 child 的前向节点的 next 节点为 child 的 next 节点。
      child._previousSibling!._nextSibling = child.nextSibling;
    }
    
    // 判断入参 child 的 _nextSibling 是否为 null，更新 _lastChild 指向。
    if (child._nextSibling == null) {
      // 如果入参 child 的 _nextSibling 为 null，则说明它是当前 ContainerLayer 的 _lastChild 指向，
      // 则更新 _lastChild 为 child 的 previousSibling，即 child 的前一个兄弟节点作为当前 ContainerLayer 的 _lastChild。
      _lastChild = child.previousSibling;
    } else {
      // 否则的话，则更新 child 的后向节点的 "前向" 指向为 child 的 "前向" 节点。
      child.nextSibling!._previousSibling = child.previousSibling;
    }
    
    // 然后是清理 child 的内容：前向和后向都指向 null，其它 Layer Tree 相关的属性也置为 null，
    // parentHandle.layer 也置为 null！
    child._previousSibling = null;
    child._nextSibling = null;
    _dropChild(child);
    child._parentHandle.layer = null;
  }
  
  void _dropChild(Layer child) {
    // 如果当前的 ContainerLayer 的 alwaysNeedsAddToScene 为 false，
    // 则需要此 ContainerLayer 进行 addToScene 操作。
    if (!alwaysNeedsAddToScene) {
      markNeedsAddToScene();
    }
    
    // 此 child 的合成回调数量不是 0 的话，则需要更新自己的父级的合成回调的数量，减去这个值。
    if (child._compositionCallbackCount != 0) {
      // 注意这里传入的是负值：-child._compositionCallbackCount
      _updateSubtreeCompositionObserverCount(-child._compositionCallbackCount);
    }
    
    // parent 指向置为 null
    child._parent = null;
    
    // detach 执行，即 owner 置为 null。
    if (attached) {
      child.detach();
    }
  }
```

## removeAllChildren

&emsp;将此 ContainerLayer 的所有子级节点从其子级列表中全部移除。

```dart
  void removeAllChildren() {
    Layer? child = firstChild;
    
    // 沿着子级的链表进行移除，无甚复杂的，可以帮助大家熟悉熟悉链表的操作。
    while (child != null) {
      final Layer? next = child.nextSibling;
      child._previousSibling = null;
      child._nextSibling = null;
      
      _dropChild(child);
      
      child._parentHandle.layer = null;
      child = next;
    }
    
    _firstChild = null;
    _lastChild = null;
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

&emsp;可以看到，不同于 RenderObject 的布局，当子级需要布局时，如果子级不是布局边界的话，仅是往上找自己的父级中的重新布局边界，然后再执行布局更新。这里的更新 Layer 的 needsAddToScene 标识，则是以当前 ContainerLayer 为根节点，以深度优先的方式往其 Layer 子树中去遍历，但凡是子级的 needsAddToScene 为 true 的话，父级的 needsAddToScene 标识也必须为 true。可以看到它的 "传染性" 明显是要强于重新布局的。

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

## addToScene

&emsp;重写此方法以将此 ContainerLayer 上传到 engine。addToScene 是 Layer 的一个抽象函数，需要由 Layer 的子类来实现。而 ContainerLayer 的 addToScene 函数内部则是仅调用：addChildrenToScene 函数，所以下面我们直接看 addChildrenToScene 函数。

```dart
  @override
  void addToScene(ui.SceneBuilder builder) {
    addChildrenToScene(builder);
  }
```

## addChildrenToScene

&emsp;将该 ContainerLayer 的所有子级 Layer 上传到 engine。

&emsp;通常，此方法由 addToScene 使用，以将子级 Layer 插入到 Scene 中。ContainerLayer 的子类通常会重写 addToScene 方法，利用 SceneBuilder API 对 Scene 应用效果，然后使用 addChildrenToScene 将它们的子级 Layer 插入到 Scene 中，最后在从 addToScene 返回之前撤销上述效果。

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

## applyTransform

&emsp;入参是 child 和 transform。将应用于给定矩阵的变换（Matrix4 transform），该 transform 在对给定子级 child 进行合成时会应用。

&emsp;具体来说，这应用于子级原点的 transform。当在一系列 Layer 中使用 applyTransform 时，除非链中最深的 Layer 将图层偏移(layerOffset) 折叠为零，否则结果将不可靠。这意味着它向其子级传递 Offset.zero，并将任何传入的图层偏移(bakes any incoming layerOffset) 到 SceneBuilder 中作为（例如）一个 transform，然后这个 transform 也包含在 applyTransform 应用的 transform 中。

&emsp;例如，如果 addToScene 应用了 layerOffset，然后将 Offset.zero 传递给子级，那么它应该包含在此处应用的 transform 中；而如果 addToScene 只是将 layerOffset 传递给子级，那么它就不应该包含在此处应用的 transform 中。

&emsp;这个方法只在 addToScene 被调用后立即有效，在任何属性被更改之前。

&emsp;默认实现什么也不做，因为默认情况下，ContainerLayer 在 ContainerLayer 本身的原点上合成其子级。（ContainerLayer 的此函数默认实现是空的。）

&emsp;通常情况下，child 参数不应为 null，因为原则上每个 Layer 都可以独立地对每个子级进行 transform。然而，某些 Layer 可能会明确允许 null 作为值，例如，如果它们知道它们会以相同的方式转换所有子级。

&emsp;被 FollowerLayer 使用，以将其子级转换为 LeaderLayer 的位置。（学习 FollowerLayer 时再深入学习。）

```dart
  void applyTransform(Layer? child, Matrix4 transform) {
    // 
  }
```

## depthFirstIterateChildren

&emsp;以深度优先顺序返回该 ContainerLayer 的子级 Layer。

```dart
  @visibleForTesting
  List<Layer> depthFirstIterateChildren() {
    // 如果 firstChild 为 null，说明当前没有子级，则直接返回空：<Layer>[]。
    if (firstChild == null) {
      return <Layer>[];
    }
    
    // 准备一个 List
    final List<Layer> children = <Layer>[];
    
    Layer? child = firstChild;
    
    // 沿着子级链表进行遍历，返回整个 Layer 子树上的所有 Layer。
    while (child != null) {
      children.add(child);
      
      // 如果此 child 是一个 ContainerLayer，则遍历返回它的所有子级。
      if (child is ContainerLayer) {
        children.addAll(child.depthFirstIterateChildren());
      }
      
      child = child.nextSibling;
    }
    
    return children;
  }
```

## ContainerLayer 总结

&emsp;只有 ContainerLayer 的子类可以在 Layer Tree 中拥有子级，所有其他 Layer 类都用作 Layer Tree 中的叶子节点。在 ContainerLayer 中继承自 Layer 的 nextSibling 和 previousSibling 两个属性发挥了自己的作用：ContainerLayer 的子级会通过这两个指针构建 ContainerLayer 的子级列表的双向链表结构，然后 ContainerLayer 还通过 firstChild 和 lastChild 属性来记录 ContainerLayer 的子级链表的头和尾，然后在加上 Layer 的 parent 指针，以及 ContainerLayer 的 append 函数，正是这些属性和函数为 Layer Tree 的树形结构奠定了代码基础。

&emsp;ContainerLayer 让我们看到了 Layer Tree 中 Layer 节点的拼接过程，后面还有更多关于叶子节点的内容，以及 Scene 的内容，ContainerLayer 中的 `ui.Scene buildScene(ui.SceneBuilder builder)` 函数让我们更加接近 SceneBuilder 和 Scene 本质。我们下篇继续！ 

## 参考链接
**参考链接:🔗**
+ [Layer class](https://api.flutter.dev/flutter/dart-ui/Canvas-class.html)
+ [ContainerLayer class](https://api.flutter.dev/flutter/rendering/ContainerLayer-class.html)
