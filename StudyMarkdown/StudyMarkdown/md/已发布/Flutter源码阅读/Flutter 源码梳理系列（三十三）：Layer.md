# Flutter 源码梳理系列（三十三）：Layer

# Layer

&emsp;在前面的多篇内容中都涉及到了 Layer 的内容，特别是 PaintingContext 篇中关于 Layer Tree 的构建过程，但是我们并没有详细看过 Layer 的文档和源码，本篇一起来看一下。

&emsp;Layer：一个合成图层。在绘制过程中，Render Tree 会生成一颗由合成图层组成的树（a tree of composited layers 即大家口中的 Layer Tree），这些合成图层会被上传到引擎中，并由合成器显示（compositor）。这个类是所有合成图层的基类，即 Layer 是所有 composited layer 的基类。

&emsp;大多数 Layer 的属性可以被改变，Layer 也可以移动到不同的父级上。在进行这些改变后，必须显式地重新合成场景（Scene）；图层树（layer tree）并不会维护自己的脏状态。

&emsp;为了合成树形结构，需要创建一个 SceneBuilder 对象，将其传递给根 Layer 对象的 addToScene 方法，然后调用 SceneBuilder.build 方法以获取一个 Scene。然后可以使用 dart:ui.FlutterView.render 方法来绘制 Scene。

## Memory

&emsp;Layers 在帧之间保留资源以加快渲染速度。一个 Layer 将保留这些资源，直到所有引用该 Layer 的 LayerHandles 都取消了对此 Layer 的引用。

&emsp;Layer 在丢弃之后（dispose）不得再使用。如果 RenderObject 需要保留一个 Layer 以供以后使用，它必须创建对该图层的 handle。对于 RenderObject.layer 属性，这是自动处理的，但额外的 Layer 必须使用它们自己的 LayerHandle。

&emsp;如下，这个 RenderObject 是一个重绘边界（repaint boundary），它会添加一个额外的 ClipRectLayer。

```dart
class ClippingRenderObject extends RenderBox {
  // 一个内部 Layer 类型是 ClipRectLayer 的 LayerHandle
  final LayerHandle<ClipRectLayer> _clipRectLayer = LayerHandle<ClipRectLayer>();

  // 重写了 isRepaintBoundary getter，在 RenderObject 中此 getter 返回 false
  @override
  bool get isRepaintBoundary => true; // The [layer] property will be used.

  // 重点来了，重写 RenderObject 的 paint 函数，来进行自己的自定义绘制
  @override
  void paint(PaintingContext context, Offset offset) {
    
    // PaintingContext 的 pushClipRect 函数，上一篇刚学的。
    
    // needsCompositing 参数是当前 ClippingRenderObject 的合成位的值，
    // 值是 true，它默认 _needsCompositing = isRepaintBoundary || alwaysNeedsCompositing，
    // 上面 isRepaintBoundary 是 true，所以在这里 needsCompositing 值也是 true。
    // 所以此处的 context.pushClipRect 调用，会在 子 PaintingContext 中在单独的 _clipRectLayer.layer 中进行绘制。
    // 这里 context.pushClipRect 会返回一个 ClipRectLayer 对象，直接赋值给 _clipRectLayer.layer
    
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

## subtreeHasCompositionCallbacks

&emsp;以这个 Layer 作为根节点下的 Layer 子树是否有任何组合（Composition）回调观察者。

&emsp;只有当以这个节点为根节点下的子树有观察者时，这个值才会为真。例如，如果父节点有观察者但子节点没有，那么可能在父节点上会得到 true，而在子节点上会得到 false。

```dart
  final Map<int, VoidCallback> _callbacks = <int, VoidCallback>{};
  static int _nextCallbackId = 0;
  
  bool get subtreeHasCompositionCallbacks => _compositionCallbackCount > 0;
  int _compositionCallbackCount = 0;
  
  // 沿着 parent 指针，往父级中更新 _compositionCallbackCount 的值。
  void _updateSubtreeCompositionObserverCount(int delta) {
    _compositionCallbackCount += delta;
    
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

&emsp;即使以保留渲染的祖先图层添加了该图层，也会触发此回调，这意味着即使通过对其中一个祖先图层调用 ui.SceneBuilder.addRetained 将该图层添加到场景中，仍会触发。

&emsp;回调函数接收对该图层的引用。接收者在回调范围内不能对该图层进行修改，但可以遍历树来查找有关当前变换或裁剪的信息。在这种状态下，该图层可能已经分离，但即使它已分离，它可能仍具有一个也已分离但可以访问的父级。

&emsp;如果在回调函数内添加或删除了新的回调函数，则新的回调函数将在下一个合成事件中触发（或停止触发）。

```dart
  VoidCallback addCompositionCallback(CompositionCallback callback) {
    // 往父级更新 _compositionCallbackCount 的值
    _updateSubtreeCompositionObserverCount(1);
    
    // 在 _callbacks 中记录下这新值。
    final int callbackId = _nextCallbackId += 1;
    _callbacks[callbackId] = () {
      callback(this);
    };
    
    // 返回旧的
    return () {
      _callbacks.remove(callbackId);
      _updateSubtreeCompositionObserverCount(-1);
    };
  }
```

## `_parentHandle`

&emsp;当这个 Layer 被附加到一个 ContainerLayer 上时被设置，当它被移除时被取消设置。（在 ContainerLayer 的 append 函数内，对 `_parentHandle.layer` 进行赋值，然后在 ContainerLayer 的 `_removeChild` 和 `removeAllChildren` 中会置回 null。）

&emsp;`_parentHandle.layer` 不能从 attach 或 detach 中设置，当整个子树被附加到 owner 或从 owner 中分离时会调用这两个方法。Layer 可以被附加到或从 ContainerLayer 中移除，而不管它们是否被附加或分离，将 Layer 从 owner 中分离并不意味着它已经从其父级移除。

&emsp;`_refCount` 在 LayerHandle 的 layer setter 中会进行递增。

&emsp;`_unref` 同样在 LayerHandle 的 layer setter 中被调用，这里是指当对 `_parentHandle.layer` 设置新值时，会调用旧 Layer 的 `_unref` 函数，防止旧 Layer 泄漏，即可以理解为增加 Layer 新值的 refCount，减少旧 Layer 的 refCount。（`_layer?._unref();`）

```dart
  final LayerHandle<Layer> _parentHandle = LayerHandle<Layer>();
  
  int _refCount = 0;
  
  void _unref() {
    _refCount -= 1;
    
    // 等于 0 的时候会调用 dispose 进行销毁。
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

&emsp;这个 Layer 在 Layer Tree 中的父级。Layer Tree 中根节点的父级是 null（还记得我们前面学习过的 TransformLayer 吗？Layer Tree 的根节点）。只有 ContainerLayer 的子类可以在层树中拥有子级。所有其他层类都用作层树中的叶子节点。（这里我们后面学习 Layer 的一众子类时会看到。）

```dart
  ContainerLayer? get parent => _parent;
  ContainerLayer? _parent;
```

## `_needsAddToScene`

&emsp;这一 Layer 自上次调用 addToScene 以来是否有任何更改。初始设置为 true，因为一个新的 Layer 从未调用过 addToScene，在调用 addToScene 后设置为 false。

&emsp;如果调用了 markNeedsAddToScene，或者在这个 Layer 或祖先 Layer 上调用了 updateSubtreeNeedsAddToScene，则该值可以再次变为 true。

&emsp;如果 Layer Tree 中的 `_needsAddToScene` 值被称为 `_consistent_`，那么 Layer Tree 中的每个 Layer 都满足以下条件：

+ 如果 `alwaysNeedsAddToScene` 为 true，则 `_needsAddToScene` 也为 true。
+ 如果 `_needsAddToScene` 为 true 且 parent 不为 null，则 `parent._needsAddToScene` 为 true。

&emsp;通常，此值在绘制阶段和合成期间设置。

&emsp;在绘制阶段，RenderObject 对象会创建新的 Layer，并在现有 Layer 上调用 markNeedsAddToScene，导致该值变为 true。绘制阶段结束后，树可能处于不一致的状态。

&emsp;在合成期间，ContainerLayer.buildScene 首先调用 updateSubtreeNeedsAddToScene 将此树带入一致状态，然后调用 addToScene，最后将该字段设置为 false。

```dart
  bool _needsAddToScene = true;
  
  // 标记这个 Layer 已经改变，addToScene 需要被调用。
  @protected
  @visibleForTesting
  void markNeedsAddToScene() {
    // 已经被标记过了，则直接 return 即可。
    if (_needsAddToScene) {
      return;
    }

    _needsAddToScene = true;
  }
```

## alwaysNeedsAddToScene

&emsp;子类可以覆盖此方法为 true 以禁用保留渲染（alwaysNeedsAddToScene 为真时，表示每帧都需要重新渲染，所以这是与保留前一帧的渲染结果进行复用是相悖的。）。

```dart
  @protected
  bool get alwaysNeedsAddToScene => false;
```

## EngineLayer 

&emsp;在进行接下来的内容之前，我们先看一下 EngineLayer 是什么。EngineLayer 同 Canvas、PictureRecorder、Picture 一样，是 Flutter framework 层到 Engine 层的桥接，它们真正的功能实现是在 Engine 层，在 Framework 层只能简单浏览一下它们的 API 看它们实现了什么功能而已。

&emsp;EngineLayer 仅有一个 dispose 函数。

&emsp;dispose 释放此对象使用的资源。在调用此方法后，对象将不再可用。

&emsp;EngineLayers 间接保留特定于平台的图形资源。其中一些资源，如图像，可能会占用大量内存。为了避免保留这些资源直到下一次垃圾回收，应尽快释放不再使用的 EngineLayer 对象。

&emsp;一旦释放了此 EngineLayer，它就不再可以用作保留的图层，也不得将其作为 oldLayer 传递给接受该参数的任何 SceneBuilder 方法。这不可能是一个叶子调用，因为 native 函数调用了 Dart API（Dart_SetNativeInstanceField）。

&emsp;其实 EngineLayer 可以理解为是当前 Layer 渲染的结果，它属于 Engine 层。当 Layer 自上次调用 addToScene 以来没有任何更改的话，可以直接复用这个 EngineLayer。

```dart
  abstract class EngineLayer {
    void dispose();
  }

  base class _NativeEngineLayer extends NativeFieldWrapperClass1 implements EngineLayer {
    // 这个类是由引擎创建的，不应该直接实例化或继承。
    _NativeEngineLayer._();

    @override
    @Native<Void Function(Pointer<Void>)>(symbol: 'EngineLayer::dispose')
    external void dispose();
  }
```

## engineLayer

&emsp;将为此 Layer 创建的 EngineLayer 存储起来，以便在跨帧重复使用引擎资源，以提高应用性能。

&emsp;这个值可以传递给 ui.scenebuilder.addRetained 来告知引擎这个 Layer 或者它的后代 Layer 没有任何改变。例如，native 引擎可以重用在前一帧中渲染的纹理。例如，web 引擎可以重用在前一帧中创建的 HTML DOM 节点。

&emsp;这个值可以作为 oldLayer 参数传递给 "push" 方法，以告知引擎一个 Layer 正在更新先前渲染的 Layer。例如，web 引擎可以更新先前呈现的 HTML DOM 节点的属性，而不是创建新节点。

```dart
  @protected
  @visibleForTesting
  ui.EngineLayer? get engineLayer => _engineLayer;

  // 设置用于渲染此 Layer 的 EngineLayer。通常情况下，该字段是设置为 addToScene 返回的值，
  // 而 addToScene 又返回由 ui.SceneBuilder 的 "push" 方法（例如 ui.SceneBuilder.pushOpacity）生成的引擎图层。
  @protected
  @visibleForTesting
  set engineLayer(ui.EngineLayer? value) {
    // 首先把旧的 dispose 了。
    _engineLayer?.dispose();
    
    // 然后赋值为新值。
    _engineLayer = value;
    
    // 当前 Layer 的 _engineLayer 被更新了，但是此 Layer 的 alwaysNeedsAddToScene 属性为 false，即并不是每帧都 addToScene，
    // 所以如果 parent 不为 null 且 parent 的 alwaysNeedsAddToScene 也是 false，
    // 那么需要标记 parent 需要 addToScene。
    
    if (!alwaysNeedsAddToScene) {
      // 父节点必须构建一个新的 EngineLayer 来添加这个图层，并且我们将其标记为需要 addToScene。
    
      // 这是设计用来处理两种情况的：
      //
      // 1. 在正常情况下渲染完整的层树时，我们首先调用子节点的 addToScene 方法，然后为父节点调用 set engineLayer。子节点会调用父节点的 markNeedsAddToScene 方法，以通知他们生成了新的 EngineLayer，因此父节点需要更新。在这种情况下，父节点已经通过 addToScene 将自己添加到 Scene 中，因此在完成后，会调用它的 set engineLayer 并清除 _needsAddToScene 标志。
      //
      // 2. 在呈现内部图层（例如 OffsetLayer.toImage）时，我们对其中一个子项调用 addToScene，但不调用父项，也就是说，我们为子项生成了新的 EngineLayer，但未为父项生成。在这种情况下，子项将标记父项需要 addToScene，但父项在某个未来的帧确定渲染时才清除该标志，此时父项知道它不能保留 EngineLayer，并会再次调用 addToScene。
      if (parent != null && !parent!.alwaysNeedsAddToScene) {
        parent!.markNeedsAddToScene();
      }
    }
  }
  
  ui.EngineLayer? _engineLayer;
```

## updateSubtreeNeedsAddToScene

&emsp;从这个 Layer 开始遍历 Layer Subtree，并确定它是否需要 addToScene。

&emsp;如果满足以下任何一种情况，Layer 就需要 addToScene：

+ alwaysNeedsAddToScene 为 true。
+ 已调用 markNeedsAddToScene。
+ 任何后代都需要 addToScene。

&emsp;ContainerLayer 覆盖了此方法，以便递归调用其子级。

```dart
  @protected
  @visibleForTesting
  void updateSubtreeNeedsAddToScene() {
    _needsAddToScene = _needsAddToScene || alwaysNeedsAddToScene;
  }
```

## owner

&emsp;这个 Layer 的所有者（如果未附加则为 null）。这个 Layer 所属的整个 Layer Tree 将具有相同的所有者。通常情况下，所有者是一个 RenderView。如 Layer Tree 的根节点的 owner 就是 Render Tree 的根节点。

```dart
  Object? get owner => _owner;
  Object? _owner;
```

## attached & attach & detach

&emsp;attached：包含此 Layer 的 Layer Tree 是否已被附加到 owner。此条件在调用 attach 时为真。此条件在调用 detach 时为假。

&emsp;attach：将该 Layer 标记为已附加到指定 owner。通常仅从父级的 attach 方法中调用，并由 owner 标记树的根已附加。具有子项的子类应该重写此方法，在调用继承方法后，如 super.attach(owner)，将所有子项都附加到相同的 owner。

&emsp;detach：将此层标记为与其 owner 分离。通常只从父级的 detach 中调用，并由 owner 标记树的根节点为分离。具有子项的子类应该重写此方法，在调用继承方法 super.detach() 后分离所有子项。

```dart
  bool get attached => _owner != null;

  @mustCallSuper
  void attach(covariant Object owner) {
    _owner = owner;
  }

  @mustCallSuper
  void detach() {
    _owner = null;
  }
```

## depth

&emsp;在 Layer Tree 中，该 Layer 的深度。树中节点的深度随着向下遍历树而单调增加。兄弟节点之间的深度没有保证。深度用于确保节点按照深度顺序进行处理。

&emsp;redepthChildren：调整该节点的子节点的深度，如果有的话。在具有子节点的子类中重写此方法，对每个子节点调用 ContainerLayer.redepthChild。不要直接调用此方法。

```dart
  int get depth => _depth;
  int _depth = 0;

  @protected
  void redepthChildren() {
    // ContainerLayer 提供了一种实现，因为它是唯一一个能够真正拥有子节点的 Layer。
  }
```

## nextSibling & previousSibling

&emsp;nextSibling：这一 Layer 的下一个兄弟 Layer 在父 Laeyr 的子 Layer 列表中。

&emsp;previousSibling：这个 Layer 在父 Layer 的子 Layer 列表中的前一个兄弟层级。

```dart
  Layer? get nextSibling => _nextSibling;
  Layer? _nextSibling;

  Layer? get previousSibling => _previousSibling;
  Layer? _previousSibling;
```

## remove

&emsp;将此 Layer 从其父 Layer 的子 Layer 列表中移除。如果 Layer 的父级已经是 null，则此操作不会产生任何效果。

```dart
  @mustCallSuper
  void remove() {
    parent?._removeChild(this);
  }
```

## `findAnnotations<S extends Object>`

&emsp;在给定的 localPosition 位置，搜索这个 Layer 及其 Layer 子树中类型为 S 的 annotations。这个方法被 find 和 findAllAnnotations 的默认实现调用。可以重写这个方法来自定义 Layer 搜索 annotations 的行为，或者如果这个 Layer 有自己的 annotations 需要添加的话。默认实现总是返回 false，意味着这个 Layer 及其子 Layer 都没有 annotations，annotations 搜索也没有被吸收（见下面的解释）。

### About layer annotations

&emsp;annotation 是可以携带在 Layer 中的任意类型的可选对象。只要所属的 Layer 包含该位置并且可以被遍历到，就可以在某个位置找到 annotation。

&emsp;annotation 是通过首先递归访问每个子级，然后是该 Layer 本身进行搜索，以实现从视觉上前到后的顺序。annotation 必须符合给定的限制条件，比如类型和位置。

&emsp;在这里找到值的常见方式是通过将 AnnotatedRegionLayer 推入 Layer Tree，或者通过覆盖 findAnnotations 函数来添加所需的 annotation。

### Parameters and return value

&emsp;result 参数是方法输出结果的地方。在遍历期间发现的新 annotation 会被添加到末尾。

&emsp;onlyFirst 参数指示，如果设置为 true，则搜索将在找到第一个符合条件的 annotation 时停止；否则，它将遍历整个子树。

&emsp;返回值表示在此位置这个 Layer 及其子树的不透明度。如果返回 true，则该 Layer 的父级应跳过该 Layer 后面的子级。换句话说，对这种类型的 annotation 是不透明的，并且已吸收了搜索，因此它后面的兄弟们不知道这次搜索。如果返回值为 false，则父级可能会继续处理其他兄弟节点。

&emsp;返回值不影响父级是否添加自己的 annotation；换句话说，如果一个 Layer 应该添加一个 annotation，即使其子级对这种类型的 annotation 不透明，它仍然会始终添加它。然而，父级返回的不透明度可能会受到其子级的影响，从而使其所有祖先对这种类型的 annotation 不透明。

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

## `S? find<S extends Object>(Offset localPosition)`

&emsp;在以 localPosition 描述的点下，搜索此 Layer 及其子树中类型为 S 的第一个 annotation。如果找不到匹配的 annotation，则返回 null。

&emsp;默认情况下，此方法调用 findAnnotations，并且仅将 onlyFirst 设置为 true，并返回第一个结果的 annotation。最好重写 findAnnotations 而不是此方法，因为在 annotation 搜索期间，只会递归调用 findAnnotations，而会忽略此方法中的自定义行为。

```dart
  S? find<S extends Object>(Offset localPosition) {
    final AnnotationResult<S> result = AnnotationResult<S>();
    
    // 直接调用 findAnnotations 函数，只查找入参 localPosition 的第一个。
    findAnnotations<S>(result, localPosition, onlyFirst: true);
    
    return result.entries.isEmpty ? null : result.entries.first.annotation;
  }
```

## `AnnotationResult<S> findAllAnnotations<S extends Object>(Offset localPosition)`

&emsp;在以 localPosition 描述的点下，搜索此 Layer 及其子树中所有类型为 S 的 annotation。如果未找到匹配的 annotation，则返回带有空条目的 AnnotationResult。

&emsp;默认情况下，此方法使用 onlyFirst: false 调用 findAnnotations，并返回其结果的 annotation。最好重写 findAnnotations 而不是此方法，因为在 annotation 搜索期间，只有 findAnnotations 会递归调用，而此方法中的自定义行为会被忽略。

```dart
  AnnotationResult<S> findAllAnnotations<S extends Object>(Offset localPosition) {
    final AnnotationResult<S> result = AnnotationResult<S>();
    
    // 直接调用 findAnnotations 函数，查找入参 localPosition 的所有。
    findAnnotations<S>(result, localPosition, onlyFirst: false);
    
    return result;
  }
```

## addToScene

&emsp;子类重写此方法将此 Layer 上传到引擎。

```dart
  @protected
  void addToScene(ui.SceneBuilder builder);
```

## `_addToSceneWithRetainedRendering`

&emsp;当不需要 `_needsAddToScene` 且 `_engineLayer` 不为 null 时，直接复用当前的 `_engineLayer`。

```dart
  void _addToSceneWithRetainedRendering(ui.SceneBuilder builder) {
    // 不能通过添加一个保留的图层子树来创建一个循环，其中 _needsAddToScene 为 false。
    //
    // 反证法：
    // 
    // 如果我们引入一个循环，那么这个保留的 Layer 必须被附加到它的一个子 Layer 中，比如说 A。这意味着 A 的子结构已经发生了变化，
    // 所以 A 的 _needsAddToScene 是 true。
    // 这与 _needsAddToScene 为 false 是矛盾的。

    if (!_needsAddToScene && _engineLayer != null) {
    
      // 当不需要 _needsAddToScene 且 _engineLayer 不为 null 时，
      // 直接复用当前的 _engineLayer
      builder.addRetained(_engineLayer!);
      
      return;
    }
    
    // 重新生成 _engineLayer 渲染结果
    addToScene(builder);
    
    // 在调用 addToScene 之后清除标志位，而不是在调用之前。
    // 这是因为 addToScene 会调用子节点的 addToScene 方法，这可能会将该图层标记为脏。
    _needsAddToScene = false;
  }
```

## Layer 总结

&emsp;每个 Layer 都有个指向父级 Layer 的 parent 指针，以及表示自己深度信息的 depth 属性，同 Element、RenderObject 保持一致。Layer 作为不同功能的 Layer 子类的基类，只提供了最基础的功能。其中最主要的 parent 指针的指向、EngineLayer 的生成与复用、markNeedsAddToScene 的调用时机、attach & detach 的调用时机、owner 属性的作用、nextSibling & previousSibling 的指向赋值、addToScene 的调用时机、needsAddToScene 和 alwaysNeedsAddToScene 属性的使用，等等问题还没有明确的答案，需要我们继续去 Layer 的子类中去发掘。 

## 参考链接
**参考链接:🔗**
+ [Layer class](https://api.flutter.dev/flutter/dart-ui/Canvas-class.html)
