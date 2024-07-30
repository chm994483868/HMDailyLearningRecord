# Flutter 源码梳理系列（二十九）：Layer

# Layer 

&emsp;Layer：一个合成图层。

&emsp;在绘制过程中，Render Tree 会生成一颗由合成图层组成的树（a tree of composited layers 即大家口中的 Layer Tree），这些合成图层会被上传到引擎中，并由合成器显示。这个类是所有合成图层的基类，即 Layer 是所有 composited layers 的基类。

&emsp;大多数 Layer 的属性可以被改变，Layer 也可以移动到不同的父级上。在进行这些改变后，必须显式地重新合成场景（Scene）；图层树（layer tree）并不会维护自己的脏状态。

&emsp;为了合成树形结构，需要创建一个 SceneBuilder 对象，将其传递给根 Layer 对象的 addToScene 方法，然后调用 SceneBuilder.build 方法以获取一个 Scene。然后可以使用 dart:ui.FlutterView.render 方法来绘制 Scene。

## Memory

&emsp;Layers 在帧之间保留资源以加快渲染速度。一个 Layer 将保留这些资源，直到所有引用该 Layer 的 LayerHandles 都取消了对此 Layer 的引用。

&emsp;Layer 在丢弃之后（dispose）不得再使用。如果 RenderObject 需要保留一个 Layer 以供以后使用，它必须创建对该图层的 handle。对于 RenderObject.layer 属性，这是自动处理的，但额外的 Layer 必须使用它们自己的 LayerHandle。

&emsp;如下，这个 RenderObject 是一个重绘边界（repaint boundary），它会添加一个额外的 ClipRectLayer。

```dart
class ClippingRenderObject extends RenderBox {
  final LayerHandle<ClipRectLayer> _clipRectLayer = LayerHandle<ClipRectLayer>();

  @override
  bool get isRepaintBoundary => true; // The [layer] property will be used.

  @override
  void paint(PaintingContext context, Offset offset) {
    _clipRectLayer.layer = context.pushClipRect(
      needsCompositing,
      offset,
      Offset.zero & size,
      super.paint,
      oldLayer: _clipRectLayer.layer,
    );
  }

  @override
  void dispose() {
  
    // 释放。
    _clipRectLayer.layer = null;
    
    super.dispose();
  }
}
```

&emsp;另请参阅：

+ RenderView.compositeFrame，它为在显示屏上绘制 RenderObject 树实现了此重组协议。

&emsp;下面看一下 Layer 的源码。

## Constructors

&emsp;创建一个 Layer 实例，Layer 是一个混入了诊断树的抽象类。没什么特别的。 

```dart
abstract class Layer with DiagnosticableTreeMixin {
  Layer() {
   // ...
  }
  // ...
}
```

##

```dart
  final Map<int, VoidCallback> _callbacks = <int, VoidCallback>{};
  static int _nextCallbackId = 0;
```

## subtreeHasCompositionCallbacks

&emsp;这个节点根节点下的子树是否有任何组合回调观察者。

&emsp;只有当这个节点根节点下的子树有观察者时，这个值才会为真。例如，如果父节点有观察者但子节点没有，那么可能在父节点上会得到 true，而在子节点上会得到 false。

```dart
  bool get subtreeHasCompositionCallbacks => _compositionCallbackCount > 0;

  int _compositionCallbackCount = 0;
  void _updateSubtreeCompositionObserverCount(int delta) {
    assert(delta != 0);
    _compositionCallbackCount += delta;
    assert(_compositionCallbackCount >= 0);
    parent?._updateSubtreeCompositionObserverCount(delta);
  }

  void _fireCompositionCallbacks({required bool includeChildren}) {
    if (_callbacks.isEmpty) {
      return;
    }
    for (final VoidCallback callback in List<VoidCallback>.of(_callbacks.values)) {
      callback();
    }
  }
```

## supportsRasterization

&emsp;这个 Layer 或任何子 Layer 是否可以使用 Scene.toImage 或 Scene.toImageSync 进行栅格化（栅格化的意思是是否可以转换为位图）。

&emsp;如果为 false，则调用上述方法可能会产生不完整的图像。

&emsp;这个值可能会在对象的生命周期中发生变化，因为子 Layer 本身会被添加或移除。

```dart
  bool supportsRasterization() {
    return true;
  }
```

## describeClipBounds

&emsp;描述了如果有的话将应用到此 Layer 内容的剪辑。

```dart
  Rect? describeClipBounds() => null;
```

## addCompositionCallback

&emsp;为该 Layert 所属的 Layer Tree 被合成时添加回调函数，或当它被分离且不再被渲染时，该回调函数将被触发。

即使以保留渲染的祖先图层添加了该图层，也会触发此回调，这意味着即使通过对其中一个祖先图层调用 ui.SceneBuilder.addRetained 将该图层添加到场景中，仍会触发。

回调函数接收对该图层的引用。接收者在回调范围内不能对该图层进行修改，但可以遍历树来查找有关当前变换或裁剪的信息。在这种状态下，该图层可能已经分离，但即使它已分离，它可能仍具有一个也已分离但可以访问的父级。

如果在回调函数内添加或删除了新的回调函数，则新的回调函数将在下一个合成事件中触发（或停止触发）。

合成回调在没有实际影响合成的情况下，可以代替推送会尝试观察图层树的图层。例如，可以使用合成回调来观察当前容器图层的总变换和裁剪，以确定是否可以看到绘制在其中的渲染对象。

调用返回的回调函数将从合成回调中移除回调函数。

```dart
  VoidCallback addCompositionCallback(CompositionCallback callback) {
    _updateSubtreeCompositionObserverCount(1);
    
    final int callbackId = _nextCallbackId += 1;
    _callbacks[callbackId] = () {
      callback(this);
    };
    
    return () {
      _callbacks.remove(callbackId);
      _updateSubtreeCompositionObserverCount(-1);
    };
  }
```

## `_parentHandle`

&emsp;当这个 Layer 被附加到一个 ContainerLayer 上时被设置，当它被移除时被取消设置。这不能从 attach 或 detach 中设置，当整个子树被附加到所有者或从所有者中分离时会调用这两个方法。层可以被附加到或从 ContainerLayer 中移除，而不管它们是否被附加或分离，将层从所有者中分离并不意味着它已经从其父级移除。

&emsp;`_refCount` 在 LayerHandle 的 layer setter 中会进行递增。

&emsp;`_unref` 同样在 LayerHandle 的 layer setter 中被调用。

```dart
  final LayerHandle<Layer> _parentHandle = LayerHandle<Layer>();
  
  int _refCount = 0;
  
  void _unref() {
    _refCount -= 1;
    
    if (_refCount == 0) {
      dispose();
    }
  }
```

## dispose

&emsp;清除此 Layer 保留的任何资源。此方法必须处理 EngineLayer 和 Picture 对象等资源。调用后，Layer 仍然可用，但它所持有的任何与图形相关的资源都需要重新创建。

&emsp;此方法仅处理此 Layer 的资源。例如，如果它是一个 ContainerLayer，它不会处理任何子级的资源。但是，当调用此方法时，ContainerLayers 会删除任何子级，如果该 Layer 是已移除子级句柄的最后一个持有者，则子级可能会递归清理其资源。

&emsp;此方法在所有未解除引用的 LayerHandles 被处理时自动调用。LayerHandle 对象通常由此图层的父级和任何参与创建此图层的 RenderObjects 持有。

&emsp;调用此方法后，对象将无法使用。

```dart
  @mustCallSuper
  @protected
  @visibleForTesting
  void dispose() {
    _engineLayer?.dispose();
    _engineLayer = null;
  }
```

## parent

&emsp;是的，每个 Layer 对象也有一个 parent 指针，指向自己的父级。Layer Tree 初见倪端。

&emsp;这个层在层树中的父级。层树中根节点的父级是 null。只有 ContainerLayer 的子类可以在层树中拥有子级。所有其他层类都用作层树中的叶子节点。

```dart
  ContainerLayer? get parent => _parent;
  ContainerLayer? _parent;
```

## `_needsAddToScene`

&emsp;这一图层自上次调用 addToScene 以来是否有任何更改。初始设置为 true，因为一个新的图层从未调用过 addToScene，在调用 addToScene 后设置为 false。

&emsp;如果调用了 markNeedsAddToScene，或者在这个图层或祖先图层上调用了 updateSubtreeNeedsAddToScene，则该值可以再次变为 true。图层树中的 `_needsAddToScene` 值被称为 “一致的”，如果树中的每个图层都满足以下条件：

+ 如果 alwaysNeedsAddToScene 为true，则 `_needsAddToScene` 也为 true。
+ 如果 `_needsAddToScene` 为true且 parent 不为 null，则 parent._needsAddToScene 为true。

&emsp;通常，此值在绘制阶段和合成期间设置。在绘制阶段，渲染对象会创建新的图层，并在现有图层上调用 markNeedsAddToScene，导致该值变为 true。绘制阶段结束后，树可能处于不一致的状态。在合成期间，ContainerLayer.buildScene 首先调用 updateSubtreeNeedsAddToScene 将此树带入一致状态，然后调用 addToScene，最后将该字段设置为 false。

```dart
  bool _needsAddToScene = true;

  /// Mark that this layer has changed and [addToScene] needs to be called.
  @protected
  @visibleForTesting
  void markNeedsAddToScene() {
    assert(!_debugMutationsLocked);
    assert(
      !alwaysNeedsAddToScene,
      '$runtimeType with alwaysNeedsAddToScene set called markNeedsAddToScene.\n'
      "The layer's alwaysNeedsAddToScene is set to true, and therefore it should not call markNeedsAddToScene.",
    );
    assert(!_debugDisposed);

    // Already marked. Short-circuit.
    if (_needsAddToScene) {
      return;
    }

    _needsAddToScene = true;
  }
```

## alwaysNeedsAddToScene

&emsp;子类可以覆盖此方法为 true 以禁用保留渲染。

```dart
  @protected
  bool get alwaysNeedsAddToScene => false;
```

## engineLayer

&emsp;将为这个 Layer 创建的引擎图层存储起来，以便在跨帧重复使用引擎资源，以提高应用性能。

&emsp;这个值可以传递给 ui.scenebuilder.addretain 来告知引擎这个层或者它的后代层没有任何改变。例如，本机引擎可以重用在前一帧中渲染的纹理。例如，web 引擎可以重用为前一个框架创建的 HTML DOM 节点。

&emsp;这个值可以作为 oldLayer 参数传递给 “push” 方法，以告知引擎一个层正在更新先前渲染的层。例如，web 引擎可以更新先前呈现的 HTML DOM 节点的属性，而不是创建新节点。

```dart
  @protected
  @visibleForTesting
  ui.EngineLayer? get engineLayer => _engineLayer;

  // 设置用于渲染此图层的引擎图层。通常情况下，该字段是设置为 addToScene 返回的值，
  // 而 addToScene 又返回由 ui.SceneBuilder 的 "push" 方法（例如 ui.SceneBuilder.pushOpacity）生成的引擎图层。
  @protected
  @visibleForTesting
  set engineLayer(ui.EngineLayer? value) {
    _engineLayer?.dispose();
    
    _engineLayer = value;
    
    if (!alwaysNeedsAddToScene) {
      // 父节点必须构建一个新的引擎层来添加这个图层，并且我们将其标记为需要 [addToScene]。
    
      // 这是设计用来处理两种情况的：
      //
      // 1. 在正常情况下渲染完整的层树时，我们首先调用子节点的 `addToScene` 方法，然后为父节点调用 `set engineLayer`。子节点会调用父节点的 `markNeedsAddToScene` 方法，以通知他们生成了新的引擎层，因此父节点需要更新。在这种情况下，父节点已经通过 [addToScene] 将自己添加到场景中，因此在完成后，会调用它的 `set engineLayer` 并清除 `_needsAddToScene` 标志。
      //
      // 2. 在呈现内部图层（例如 `OffsetLayer.toImage`）时，我们对其中一个子项调用 `addToScene`，但不调用父项，也就是说，我们为子项生成了新的引擎图层，但未为父项生成。在这种情况下，子项将标记父项需要 `addToScene`，但父项在某个未来的帧确定渲染时才清除该标志，此时父项知道它不能保留引擎图层，并会再次调用 `addToScene`。
      if (parent != null && !parent!.alwaysNeedsAddToScene) {
        parent!.markNeedsAddToScene();
      }
    }
  }
  
  ui.EngineLayer? _engineLayer;
```

## updateSubtreeNeedsAddToScene

&emsp;从这个图层开始遍历图层子树，并确定它是否需要 addToScene。

&emsp;如果满足以下任何一种情况，图层就需要 addToScene：

+ alwaysNeedsAddToScene 为 true。
+ 已调用 markNeedsAddToScene。
+ 任何后代都需要 addToScene。

&emsp;ContainerLayer 覆盖了此方法，以便递归调用其子级。

```dart
  @protected
  @visibleForTesting
  void updateSubtreeNeedsAddToScene() {
    assert(!_debugMutationsLocked);
    _needsAddToScene = _needsAddToScene || alwaysNeedsAddToScene;
  }
```

## owner

&emsp;这个层的所有者（如果未附加则为 null）。这个层所属的整个层树将具有相同的所有者。通常情况下，所有者是一个 RenderView。

```dart
  Object? get owner => _owner;
  Object? _owner;
```

## attached & attach & detach

&emsp;attached：包含此图层的图层树是否已被附加到所有者。此条件在调用 attach 时为真。此条件在调用 detach 时为假。

&emsp;attach：将该层标记为已附加到指定所有者。通常仅从父级的 attach 方法中调用，并由所有者标记树的根已附加。具有子项的子类应该覆盖此方法，在调用继承方法后，如 super.attach(owner) ，将所有子项都附加到相同的所有者。

&emsp;detach：将此层标记为与其所有者分离。通常只从父级的 detach 中调用，并由所有者标记树的根节点为分离。具有子项的子类应该重写此方法，在调用继承方法 super.detach() 后分离所有子项。

```dart
  bool get attached => _owner != null;

  @mustCallSuper
  void attach(covariant Object owner) {
    assert(_owner == null);
    _owner = owner;
  }

  @mustCallSuper
  void detach() {
    assert(_owner != null);
    _owner = null;
    assert(parent == null || attached == parent!.attached);
  }
```

## depth

&emsp;在图层树中，该图层的深度。树中节点的深度随着向下遍历树而单调增加。兄弟节点之间的深度没有保证。深度用于确保节点按照深度顺序进行处理。

&emsp;redepthChildren：调整该节点的子节点的深度，如果有的话。在具有子节点的子类中重写此方法，对每个子节点调用 ContainerLayer.redepthChild。不要直接调用此方法。

```dart
  int get depth => _depth;
  int _depth = 0;

  @protected
  void redepthChildren() {
    // ContainerLayer 提供了一种实现，因为它是唯一一个能够真正拥有子节点的图层。
  }
```

## nextSibling & previousSibling

&emsp;nextSibling：这一层的下一个兄弟层在父层的子层列表中。

&emsp;previousSibling：这个层级在父层级的子层级列表中的前一个兄弟层级。

```dart
  Layer? get nextSibling => _nextSibling;
  Layer? _nextSibling;

  /// This layer's previous sibling in the parent layer's child list.
  Layer? get previousSibling => _previousSibling;
  Layer? _previousSibling;
```

## remove

&emsp;将此图层从其父图层的子图层列表中移除。如果图层的父级已经是 null，则此操作不会产生任何效果。

```dart
  @mustCallSuper
  void remove() {
    parent?._removeChild(this);
  }
```

## 

&emsp;findAnnotations：在给定的 localPosition 位置，搜索这个图层及其子树中类型为 S 的注释。这个方法被 find 和 findAllAnnotations 的默认实现调用。可以重写这个方法来自定义图层搜索注释的行为，或者如果这个图层有自己的注释需要添加的话。默认实现总是返回 false，意味着这个图层及其子图层都没有注释，注释搜索也没有被吸收（见下面的解释）。

### About layer annotations

&emsp;注解是可以携带在图层中的任意类型的可选对象。只要所属的图层包含该位置并且可以被遍历到，就可以在某个位置找到注解。

注解是通过首先递归访问每个子级，然后是该图层本身进行搜索，以实现从视觉上前到后的顺序。注解必须符合给定的限制条件，比如类型和位置。

在这里找到值的常见方式是通过将 AnnotatedRegionLayer 推入图层树，或者通过覆盖 findAnnotations 函数来添加所需的注解。

### Parameters and return value

&emsp;result 参数是方法输出结果的地方。在遍历期间发现的新注释会被添加到末尾。

onlyFirst 参数指示，如果设置为 true，则搜索将在找到第一个符合条件的注释时停止；否则，它将遍历整个子树。

返回值表示在此位置这个图层及其子树的不透明度。如果返回 true，则该图层的父级应跳过该图层后面的子级。换句话说，对这种类型的注释是不透明的，并且已吸收了搜索，因此它后面的兄弟们不知道这次搜索。如果返回值为 false，则父级可能会继续处理其他兄弟节点。

返回值不影响父级是否添加自己的注释；换句话说，如果一个图层应该添加一个注释，即使其子级对这种类型的注释不透明，它仍然会始终添加它。然而，父级返回的不透明度可能会受到其子级的影响，从而使其所有祖先对这种类型的注释不透明。

```dart
  @protected
  bool findAnnotations<S extends Object>(
    AnnotationResult<S> result,
    Offset localPosition, {
    required bool onlyFirst,
  }) {
    return false;
  }
```

##

```dart
  S? find<S extends Object>(Offset localPosition) {
    final AnnotationResult<S> result = AnnotationResult<S>();
    findAnnotations<S>(result, localPosition, onlyFirst: true);
    return result.entries.isEmpty ? null : result.entries.first.annotation;
  }
  
  AnnotationResult<S> findAllAnnotations<S extends Object>(Offset localPosition) {
    final AnnotationResult<S> result = AnnotationResult<S>();
    findAnnotations<S>(result, localPosition, onlyFirst: false);
    return result;
  }
```

##

```dart
  /// Override this method to upload this layer to the engine.
  @protected
  void addToScene(ui.SceneBuilder builder);

  void _addToSceneWithRetainedRendering(ui.SceneBuilder builder) {
    assert(!_debugMutationsLocked);
    // There can't be a loop by adding a retained layer subtree whose
    // _needsAddToScene is false.
    //
    // Proof by contradiction:
    //
    // If we introduce a loop, this retained layer must be appended to one of
    // its descendant layers, say A. That means the child structure of A has
    // changed so A's _needsAddToScene is true. This contradicts
    // _needsAddToScene being false.
    if (!_needsAddToScene && _engineLayer != null) {
      builder.addRetained(_engineLayer!);
      return;
    }
    addToScene(builder);
    // Clearing the flag _after_ calling `addToScene`, not _before_. This is
    // because `addToScene` calls children's `addToScene` methods, which may
    // mark this layer as dirty.
    _needsAddToScene = false;
  }
```


## 参考链接
**参考链接:🔗**
+ [Canvas class](https://api.flutter.dev/flutter/dart-ui/Canvas-class.html)
