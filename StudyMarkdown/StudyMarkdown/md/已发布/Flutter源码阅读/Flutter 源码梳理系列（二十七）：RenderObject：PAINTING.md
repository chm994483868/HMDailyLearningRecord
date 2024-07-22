# Flutter 源码梳理系列（二十七）：RenderObject：PAINTING

# RenderObject：PAINTING section

&emsp;超级重要的和绘制相关的内容。

## isRepaintBoundary

&emsp;这个 RenderObject 对象是否独立于其父级绘制。

&emsp;在 RenderObject 子类中重写这个方法以指示 RenderObject 子类的实例是否应该独立重绘。例如，经常需要重绘的 RenderObject 子类对象可能希望自己重绘，而不要求其父级也一起重绘。

&emsp;如果这个 getter 返回 true，paintBounds（Rect get paintBounds;）getter 应用于此 RenderObject 对象和它的所有子级对象。framework 调用 RenderObject.updateCompositedLayer 创建一个 OffsetLayer 并将其分配给 layer 字段。声明自己为重绘边界的 RenderObject 对象不得替换 framework 创建的图层（layer）。

&emsp;如果这个 getter 的值发生变化，必须调用 markNeedsCompositingBitsUpdate。（即如果自己是重绘边界或者不是重绘边界发生了变化，则需要更新合成位 bits，它会影响图层 Layer 的合成。）

```dart
  bool get isRepaintBoundary => false;
```

## alwaysNeedsCompositing

&emsp;这个 RenderObject 对象是否总是需要合成。

&ems;在 RenderObject 子类中重写此方法，以指示绘制函数总是会创建至少一个合成图层（composited layer）。例如，视频应该在使用硬件解码器时返回 true。

&emsp;如果此 getter 的值发生更改，必须调用 markNeedsCompositingBitsUpdate。（在调用 adoptChild 或 dropChild 时，会暗示此操作。）

```dart
  @protected
  bool get alwaysNeedsCompositing => false;
```

## `_wasRepaintBoundary`

&emsp;这个属性比较特殊，它用来记录，当 RenderObject 本次进行重绘时，记录之前这个 RenderObject 是否是重绘边界（RepaintBoundary）。

```dart
  late bool _wasRepaintBoundary;
```

## updateCompositedLayer

&emsp;更新此 RenderObject 对象拥有的合成层（composited layer）。

&emsp;当 isRepaintBoundary 为 true 时，framework 会调用此方法。

&emsp;如果 oldLayer 为 null，则此方法必须返回一个新的 OffsetLayer（或其子类型）。如果 oldLayer 不为 null，则此方法必须重用所提供的图层实例 - 在此实例中创建新图层是错误的。当 RenderObject 对象被销毁或不再是重绘边界时，framework 将处理该图层的销毁。

&emsp;OffsetLayer.offset 属性将由 framework 管理，不能由此方法更新。

&emsp;如果需要更新合成层的属性，则 RenderObject 对象必须调用 markNeedsCompositedLayerUpdate 方法，该方法将安排调用此方法而无需重绘子级。如果此 Widget 被标记为需要绘制并需要合成层更新，则只会调用此方法一次。

```dart
  OffsetLayer updateCompositedLayer({required covariant OffsetLayer? oldLayer}) {
    return oldLayer ?? OffsetLayer();
  }
```

## layer

&emsp;此 RenderObject 对象用于重绘的合成层。

&emsp;如果此 RenderObject 对象不是重绘边界，则由 paint 方法负责填充此字段。如果 needsCompositing 为 true，则此字段可能会填充 RenderObject 对象实现中使用的最顶层层。在重绘时，RenderObject 对象可以更新此字段中存储的层，而不是创建新的层，以获得更好的性能。也可以将此字段保留为 null，并在每次重绘时创建新的层，但不会获得性能优势。如果 needsCompositing 为 false，则此字段必须设为 null，要么通过从未填充此字段来实现，要么在 needsCompositing 从 true 更改为 false 时将其设置为 null。

&emsp;如果创建了新的层并存储在 RenderObject 对象的其他字段中，则 RenderObject 对象必须使用 LayerHandle 来存储它。LayerHandle 将防止在 RenderObject 对象完成使用之前丢弃层，并确保在 RenderObject 对象创建替代层或将其值设为 null 时适当地处理该层。RenderObject 对象必须在其 dispose 方法中将 LayerHandle.layer 设为 null。

&emsp;如果此 RenderObject 对象是一个重绘边界，则在调用 paint 方法之前，framework 会自动创建一个 OffsetLayer 并填充此字段。paint 方法不得更改此字段的值。

```dart
  @protected
  ContainerLayer? get layer {
    assert(!isRepaintBoundary || _layerHandle.layer == null || _layerHandle.layer is OffsetLayer);
    
    return _layerHandle.layer;
  }
  
  @protected
  set layer(ContainerLayer? newLayer) {
  
    // 尝试将一个图层设为重绘边界渲染对象。框架会自动创建并分配一个 OffsetLayer 给重绘边界。
    _layerHandle.layer = newLayer;
  }
```

## `_layerHandle`

&emsp;LayerHandle 类的内容很简单，仅仅是一个有一个泛型 T 的 `_layer` 属性，T 需要是继承自 Layer，即 LayerHandle 是一个仅有一个 `Layer _layer` 属性的类。它主要用来为 RenderObject 对象提供一个 Layer 属性的句柄。每个 RenderObject 对象有一个 `_layerHandle` 属性，然后此 `_layerHandle` 属性持有一个可选的 `_layer` 属性。 

```dart
  final LayerHandle<ContainerLayer> _layerHandle = LayerHandle<ContainerLayer>();
```

## `_needsCompositingBitsUpdate`

&emsp;当添加了一个子级时，将其设置为 true。主要用来标识当前的 RenderOjbect 对象是否需要合成 Layer。在 paint 函数中会用到此标识。

```dart
  bool _needsCompositingBitsUpdate = false;
```

## markNeedsCompositingBitsUpdate

&emsp;将此 RenderObject 对象的合成状态（`_needsCompositingBitsUpdate`）标记为脏。

&emsp;调用此方法表示在下一个 PipelineOwner.flushCompositingBits 引擎阶段需要重新计算 needsCompositing 的值。

&emsp;当子树发生变化时，我们需要重新计算我们的 needsCompositing 位，并且一些祖先节点也需要做相同的事情（以防我们的位因某种更改而导致它们的更改）。为此，adoptChild 和 dropChild 方法调用此方法，并在必要时调用父级的此方法，等等，沿着 Render Tree 向上遍历标记所有需要更新的节点。

&emsp;该方法不会去请求新的帧，因为只有合成位发生变化是不可能的，必定会有其他原因一起执行。(即不同于 markNeedsLayout 或者 markNeedsPaint 函数，它们在结尾处都会调用：`owner!.requestVisualUpdate();` 请求新的帧，而 markNeedsCompositingBitsUpdate 函数则是蹭的它们的。)

```dart
  void markNeedsCompositingBitsUpdate() {
    // 如果 "需要合成位更新标识" 已经为 true 了，直接返回即可。
    if (_needsCompositingBitsUpdate) {
      return;
    }
    
    // 标识置为 true
    _needsCompositingBitsUpdate = true;
    
    if (parent is RenderObject) {
    
      // 如果当前 RenderObject 对象的父级也被标记需要更新的话，
      // 那么直接返回即可，因为父级更新时也会连带子级进行更新。
      final RenderObject parent = this.parent!;
      if (parent._needsCompositingBitsUpdate) {
        return;
      }

      // 如果自己不是重绘边界，并且直接父级也不是重绘边界，则继续往上传递，需要合成位更新，
      // 直到一个重绘边界为止。
      if ((!_wasRepaintBoundary || !isRepaintBoundary) && !parent.isRepaintBoundary) {
        // 继续往父级传递，需要合成位更新
        parent.markNeedsCompositingBitsUpdate();
        
        return;
      }
    }
    
    // 如果自己就是重绘边界的话，把自己添加到需要合成位更新的列表中即可。父级不需要一起更新。
    
    // parent is fine (or there isn't one), but we are dirty
    if (owner != null) {
      // 然后把当前 RenderObject 对象添加到 owner 的 _nodesNeedingCompositingBitsUpdate 列表中去。
      // 等待下一帧被更新。
      owner!._nodesNeedingCompositingBitsUpdate.add(this);
    }
  }
```

## `_needsCompositing`

&emsp;在构造函数中初始化。

&emsp;无论我们还是我们的后代是否有一个合成层。

&emsp;如果该节点需要合成，如此 Bits 所示，那么所有祖先节点也将需要合成。只有在 PipelineOwner.flushLayout 和 PipelineOwner.flushCompositingBits 被调用后才合法调用。

```dart
  late bool _needsCompositing;
  
  bool get needsCompositing {
    return _needsCompositing;
  }
```

## `_updateCompositingBits`

&emsp;执行 CompositingBits 合成位更新。

```dart
  void _updateCompositingBits() {
    if (!_needsCompositingBitsUpdate) {
      return;
    }
    
    final bool oldNeedsCompositing = _needsCompositing;
    _needsCompositing = false;
    
    visitChildren((RenderObject child) {
      child._updateCompositingBits();
      
      if (child.needsCompositing) {
        _needsCompositing = true;
      }
    });
    
    if (isRepaintBoundary || alwaysNeedsCompositing) {
      _needsCompositing = true;
    }
    
    // 如果一个节点之前是一个重绘边界，但现在不是了，
    // 那么无论其合成状态如何，我们都需要找到一个新的父节点进行绘制。
    // 为了做到这一点，我们重新标记该节点为干净状态，这样在 markNeedsPaint 中的遍历就不会被提前终止。
    // 它将从 _nodesNeedingPaint 中移除，这样我们在找到父节点之后就不会再尝试从它进行绘制。
    if (!isRepaintBoundary && _wasRepaintBoundary) {
      _needsPaint = false;
      _needsCompositedLayerUpdate = false;
      
      owner?._nodesNeedingPaint.remove(this);
      
      _needsCompositingBitsUpdate = false;
      markNeedsPaint();
    } else if (oldNeedsCompositing != _needsCompositing) {
      _needsCompositingBitsUpdate = false;
      markNeedsPaint();
    } else {
      _needsCompositingBitsUpdate = false;
    }
  }
```

## `_needsPaint`

&emsp;是否需要绘制的标识。

```dart
  bool _needsPaint = true;
```

## `_needsCompositedLayerUpdate`

&emsp;是否需要合成层的更新。

```dart
  bool _needsCompositedLayerUpdate = false;
```

## markNeedsPaint

&emsp;将此 RenderObject 对象标记为已更改其视觉外观。（即需要在下一帧进行重新绘制了，因为其外观已经发生变化。）

&emsp;与立即对此 RenderObject 对象的显示列表（display list）进行更新以响应写入不同，我们改为将 RenderObject 对象标记为需要绘制，从而安排了一个视觉更新（`owner!.requestVisualUpdate();`）。作为视觉更新的一部分，渲染管线将为此 RenderObject 对象提供更新其显示列表的机会。

&emsp;这种机制批量处理绘制工作，使多个连续的写入被合并，消除了冗余计算。

&emsp;另请参阅：

 + RepaintBoundary 用于将一颗 RenderObject 对象的子树作用域限定为其自己的图层，从而限制 markNeedsPaint 必须标记为脏的节点数量。

```dart
  void markNeedsPaint() {
    // 如果 _needsPaint 为 true，则表示已经被标记过了，不用重复进行了，直接返回即可。
    if (_needsPaint) {
      return;
    }
    
    // _needsPaint 标识置为 true
    _needsPaint = true;
    
    // 如果此 RenderObject 之前不是一个重绘边界的话，那么它不会有一个我们可以从中绘制的图层。
    
    if (isRepaintBoundary && _wasRepaintBoundary) {
      
      // 如果我们始终有自己的图层，那么我们可以在不涉及任何其他节点的情况下重绘自己。
      // 即不需要向父级传递。
      if (owner != null) {
        
        // 1️⃣ 把当前 RenderObject 对象添加到 owner 的 _nodesNeedingPaint 列表中去，
        // 然后请求新的帧，在下一帧对所有需要重绘的节点进行批处理。
        owner!._nodesNeedingPaint.add(this);
        owner!.requestVisualUpdate();
      }
    } else if (parent != null) {
    
      // 2️⃣ 如果自己不是重绘边界的话并且之前也不是重绘边界的话，
      // 则把重绘的需求向上传递到父级去。
      parent!.markNeedsPaint();
      
    } else {
      // 3️⃣ 没有父级的 RenderObject 节点，那只有 Render Tree 的根节点是这样。
      
      // 如果我们是渲染树的根且不是重绘边界，那么我们必须把自己绘制出来，因为没有其他人可以绘制我们。
      // 在这种情况下，我们不会将自己添加到 _nodesNeedingPaint 中，因为根节点总是被告知进行绘制操作。
      
      // 根节点为 RenderView 的树不会经过此代码路径，因为 RenderView 是重绘边界。
      
      owner?.requestVisualUpdate();
    }
  }
```

## markNeedsCompositedLayerUpdate

&emsp;将此 RenderObject 对象标记为已在其合成层上更改属性。

&emsp;具有合成层的 RenderObject 对象，其 isRepaintBoundary 等于 true，可以更新合成层的属性，而无需重新绘制其子级。如果此 RenderObject 对象是一个重新绘制边界，但尚未为其创建合成层，则此方法将标记最近的重新绘制边界父级需要重新绘制。

&emsp;如果在一个非重新绘制边界的 RenderObject 对象上调用此方法，或者是一个重新绘制边界但尚未合成的 RenderObject 对象，则调用此方法等效于调用 markNeedsPaint。

&emsp;另请参阅：

+ RenderOpacity，当其不透明度更新时使用此方法来更新层的不透明度而不重新绘制子级。

```dart
  void markNeedsCompositedLayerUpdate() {
    if (_needsCompositedLayerUpdate || _needsPaint) {
      return;
    }
    
    _needsCompositedLayerUpdate = true;
    
    // If this was not previously a repaint boundary it will not have a layer we can paint from.
    if (isRepaintBoundary && _wasRepaintBoundary) {
      // If we always have our own layer, then we can just repaint ourselves without involving any other nodes.
      assert(_layerHandle.layer != null);
      
      if (owner != null) {
        owner!._nodesNeedingPaint.add(this);
        owner!.requestVisualUpdate();
      }
    } else {
      markNeedsPaint();
    }
  }
```

## `_skippedPaintingOnLayer`

&emsp;当 flushPaint() 尝试让我们绘制但是我们的图层已分离时调用。为确保当它最终重新附加时我们的子树被重绘，即使在某个祖先图层本身从未标记为脏的情况下，我们必须标记整个分离的子树为脏，需要被重绘。这样，我们最终会被重绘。

```dart
  void _skippedPaintingOnLayer() {
    assert(attached);
    assert(isRepaintBoundary);
    assert(_needsPaint || _needsCompositedLayerUpdate);
    assert(_layerHandle.layer != null);
    assert(!_layerHandle.layer!.attached);
    
    RenderObject? node = parent;
    while (node is RenderObject) {
      if (node.isRepaintBoundary) {
        if (node._layerHandle.layer == null) {
          // Looks like the subtree here has never been painted. Let it handle itself.
          break;
        }
        
        if (node._layerHandle.layer!.attached) {
          // It's the one that detached us, so it's the one that will decide to repaint us.
          break;
        }
        
        node._needsPaint = true;
      }
      
      node = node.parent;
    }
  }
```

## scheduleInitialPaint

&emsp;通过安排第一次绘制来引导渲染流水线。

&emsp;需要这个 RenderObject 对象已经附加到 Render Tree 的根，并且具有一个复合图层。

&emsp;查看 RenderView 以了解此函数的使用示例。

```dart
  void scheduleInitialPaint(ContainerLayer rootLayer) {
    assert(rootLayer.attached);
    assert(attached);
    assert(parent is! RenderObject);
    assert(!owner!._debugDoingPaint);
    assert(isRepaintBoundary);
    assert(_layerHandle.layer == null);
    
    _layerHandle.layer = rootLayer;
    
    assert(_needsPaint);
    
    owner!._nodesNeedingPaint.add(this);
  }
```

## replaceRootLayer

&emsp;替换图层。这仅适用于 RenderObject 对象子树的根（无论 scheduleInitialPaint 调用对象是什么）。

&emsp;例如，如果设备像素比率更改，则可能会调用此操作。

```dart
  void replaceRootLayer(OffsetLayer rootLayer) {
    assert(!_debugDisposed);
    assert(rootLayer.attached);
    assert(attached);
    assert(parent is! RenderObject);
    assert(!owner!._debugDoingPaint);
    assert(isRepaintBoundary);
    assert(_layerHandle.layer != null); // use scheduleInitialPaint the first time
    
    _layerHandle.layer!.detach();
    _layerHandle.layer = rootLayer;
    
    markNeedsPaint();
  }
```

## `_paintWithContext`

&emsp;使用传递来的 PaintingContext context 对象，可以进行绘制了。

```dart
  void _paintWithContext(PaintingContext context, Offset offset) {

    // 如果我们仍然需要布局，那意味着我们在布局阶段被跳过，因此不需要绘制。
    // 也许我们还不知道这一点（也就是说，我们的层可能还没有被分离），
    // 因为跳过我们的布局节点在树中位于我们上方（显而易见） ，因此可能尚未绘制（因为树以相反的顺序绘制）。
    // 特别是如果它们拥有不同的层，因为我们之间存在重绘边界，所以会发生这种情况。
    if (_needsLayout) {
      return;
    }
    
    _needsPaint = false;
    _needsCompositedLayerUpdate = false;
    _wasRepaintBoundary = isRepaintBoundary;
    
    try {
      paint(context, offset);
    } catch (e, stack) {
      _reportException('paint', e, stack);
    }
  }
```

## paintBounds

&emsp;这个 RenderObject 对象将绘制的边界的估计值。对于调试标志如 debugPaintLayerBordersEnabled 非常有用。

&emsp;这些边界也是 showOnScreen 使用的，用来使一个 RenderObject 在屏幕上可见。

```dart
  Rect get paintBounds;
```

## paint

&emsp;在给定的偏移量（Offset）处将此 RenderObject 对象绘制到给定的上下文中。

&emsp;子类应重写此方法以提供自身的视觉外观。RenderObject 对象的本地坐标系统与上下文的画布坐标系是对齐的，并且 RenderObject 对象的本地原点（即，x=0 和 y=0）放置在上下文的画布中的给定偏移处。

&emsp;不要直接调用此函数。如果希望进行绘制，请调用 markNeedsPaint 而不是直接调用此函数。如果希望绘制其中一个子级，请在给定的上下文上调用 PaintingContext.paintChild。

&emsp;当绘制其中一个子级时（通过在给定上下文上的绘制子对象函数），由于在绘制子级之前和之后绘制的绘制操作可能需要记录在不同的合成层上，因此上下文持有的当前画布可能会更改。

```dart
  void paint(PaintingContext context, Offset offset) { }
```

## applyPaintTransform

&emsp;将应用于正在绘制的给定子级（child）的转换应用到给定的矩阵中。

&emsp;用于坐标转换函数，将一个 RenderObject 对象局部的坐标转换为另一个 RenderObject 对象的本地坐标。

&emsp;一些 RenderObject 在此方法中提供了一个被清零的矩阵，表示子级当前不应该绘制任何内容或响应点击测试。即使父级当前不绘制其子级，例如父级是一个 offstage 设置为 true 的 RenderOffstage，父级也可以提供一个非零矩阵。在这两种情况下，父级必须从 paintsChild 返回 false。

```dart
  void applyPaintTransform(covariant RenderObject child, Matrix4 transform) {
    assert(child.parent == this);
  }
```

## paintsChild

&emsp;如果调用了 paint，给定的子级是否会被绘制。

&emsp;一些 RenderObjects 在配置为不产生任何可见效果时，会跳过绘制它们的子组件。例如，一个 RenderOffstage，其 offstage 属性设置为 true，或者一个 RenderOpacity，其 opacity 值设置为零。

&emsp;在这些情况下，父级仍然可以向 applyPaintTransform 提供一个非零的矩阵，以通知调用者在什么位置绘制子级（即使子级根本没有被绘制）。另外，如果父级无法确定子级的有效矩阵，因此无法意义地确定子组件会在哪里绘制，父组件可以提供一个零矩阵。

```dart
  bool paintsChild(covariant RenderObject child) {
    assert(child.parent == this);
    
    return true;
  }
```

## getTransformTo

&emsp;将绘制变换应用于祖先节点。

&emsp;返回一个矩阵，将本地绘制坐标系映射到祖先节点的坐标系。

&emsp;如果祖先节点为 null，则此方法返回一个矩阵，将本地绘制坐标系映射到 PipelineOwner.rootNode 的坐标系。

&emsp;对于由 RendererBinding 拥有的 Render Tree（即在设备上显示的主渲染树），这意味着该方法映射到逻辑像素中的全局坐标系。要获取物理像素，请使用 RenderView 的 applyPaintTransform 进一步变换坐标。

```dart
  Matrix4 getTransformTo(RenderObject? target) {
    // 从 fromRenderObject 到 toRenderObject 以及它们共同的祖先的路径。
    // 如果不为 null，则每个列表的长度大于 1。
    // 返回值为 `this`、`commonAncestorRenderObject` 或者 null，如果 `this` 是公共祖先的话。
    
    List<RenderObject>? fromPath;
    
    // `target` 是公共祖先渲染对象之前的一组对象，或者如果 `target` 本身就是公共祖先，则为 null。
    List<RenderObject>? toPath;

    RenderObject from = this;
    RenderObject to = target ?? owner!.rootNode!;

    while (!identical(from, to)) {
      final int fromDepth = from.depth;
      final int toDepth = to.depth;

      if (fromDepth >= toDepth) {
        final RenderObject fromParent = from.parent ?? (throw FlutterError('$target and $this are not in the same render tree.'));
        (fromPath ??= <RenderObject>[this]).add(fromParent);
        from = fromParent;
      }
      
      if (fromDepth <= toDepth) {
        final RenderObject toParent = to.parent ?? (throw FlutterError('$target and $this are not in the same render tree.'));
        (toPath ??= <RenderObject>[target!]).add(toParent);
        
        to = toParent;
      }
    }

    Matrix4? fromTransform;
    if (fromPath != null) {
      fromTransform = Matrix4.identity();
      
      final int lastIndex = target == null ? fromPath.length - 2 : fromPath.length - 1;
      
      for (int index = lastIndex; index > 0; index -= 1) {
        fromPath[index].applyPaintTransform(fromPath[index - 1], fromTransform);
      }
    }
    
    if (toPath == null) {
      return fromTransform ?? Matrix4.identity();
    }

    final Matrix4 toTransform = Matrix4.identity();
    for (int index = toPath.length - 1; index > 0; index -= 1) {
      toPath[index].applyPaintTransform(toPath[index - 1], toTransform);
    }
    
    // 如果矩阵是单数，那么 `invert()` 方法不会执行任何操作。
    if (toTransform.invert() == 0) {
      return Matrix4.zero();
    }
    
    return (fromTransform?..multiply(toTransform)) ?? toTransform;
  }
```

## describeApproximatePaintClip

&emsp;返回在此对象的坐标系中描述给定子元素在绘制阶段期间所应用的剪裁矩形的近似边界框的矩形。

&emsp;如果子元素不会被剪裁，则返回 null。

&emsp;这在语义阶段中用于避免包括那些在屏幕上不可见的子元素。

&emsp;在绘制时遵守剪裁行为的 RenderObjects 必须在描述此值时也要遵守相同的行为。例如，如果将 Clip.none 作为 clipBehavior 传递给 PaintingContext.pushClipRect，则此方法的实现必须返回 null。

```dart
  Rect? describeApproximatePaintClip(covariant RenderObject child) => null;
```











## 参考链接
**参考链接:🔗**
+ [RenderObject class](https://api.flutter.dev/flutter/rendering/RenderObject-class.html)
+ [RenderObjects?! | Decoding Flutter](https://www.youtube.com/watch?v=zmbmrw07qBc)
+ [PipelineOwner class](https://api.flutter.dev/flutter/rendering/PipelineOwner-class.html)
+ [RendererBinding mixin](https://api.flutter.dev/flutter/rendering/RendererBinding-mixin.html)
+ [PipelineManifold class](https://api.flutter.dev/flutter/rendering/PipelineManifold-class.html)
+ [Listenable class](https://api.flutter.dev/flutter/foundation/Listenable-class.html)
+ [ChangeNotifier class](https://api.flutter.dev/flutter/foundation/ChangeNotifier-class.html)
