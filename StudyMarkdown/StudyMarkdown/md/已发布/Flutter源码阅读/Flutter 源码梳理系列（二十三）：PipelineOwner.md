# Flutter 源码梳理系列（二十三）：PipelineOwner

&emsp;PipelineOwner 负责管理渲染管线（rendering pipeline）。

&emsp;PipelineOwner 提供了一个接口来驱动渲染管线，并存储关于在管线的每个阶段中请求被访问的渲染对象的状态。要刷新管线，请按照以下顺序调用这些函数：

1. flushLayout 更新任何需要计算其布局的渲染对象。在此阶段，计算每个渲染对象的大小和位置。在这个阶段，渲染对象可能会在其绘制（painting）或合成状态（compositing）中标记为脏。

2. flushCompositingBits 更新具有脏混合位的任何渲染对象。在此阶段，每个渲染对象都会了解其子级是否需要混合。在绘制阶段使用此信息来选择如何实现诸如裁剪之类的视觉效果（看到这里想到了 iOS 的离屏渲染绘制圆角、阴影、mask 等。）。如果一个渲染对象具有一个合成子级，它需要使用一个 Layer 来创建 clip，以便 clip 应用到合成子级上（后者将绘制到自己的 Layer 中）。

3. flushPaint 会访问需要绘制的任何渲染对象。在此阶段，渲染对象有机会将绘制命令记录到 PictureLayer 中，并构造其他合成 Layer。

4. 最后，如果启用了语义分析，flushSemantics 将为渲染对象编译语义。这些语义信息被辅助技术用于改善 Render Tree 的可访问性。

&emsp;RendererBinding 持有屏幕上可见的渲染对象的 PipelineOwner。你可以创建其他 PipelineOwner 来管理屏幕外（off-screen）的对象，这些对象可以独立于屏幕上（on-screen）的渲染对象刷新它们的管线。

&emsp;PipelineOwner 可以按树状结构组织，用于管理多个 render 树，其中每个 PipelineOwner 负责一个 render 树。要构建或修改树，请调用 adoptChild 或 dropChild。在上面描述的不同 flush 阶段中，PipelineOwner 在调用其子节点的相同 flush 方法之前，将首先在其自己的 render 树中管理的节点上执行该阶段。不应该假设子 PipelineOwner 的刷新顺序。

&emsp;一个 PipelineOwner 也可以附加到一个 PipelineManifold，这样它就可以访问通常由 bindings 暴露的平台功能，而不必将其绑定到特定的绑定实现。在给定树中的所有 PipelineOwner 必须附加到相同的 PipelineManifold。这在 adoptChild 中会自动发生。

## Constructors

&emsp;创建一个 PipelineOwner。通常由 binding（例如 RendererBinding）创建，但也可以独立于 binding 创建，以通过渲染管线驱动屏幕外（off-screen）渲染对象。

```dart
class PipelineOwner with DiagnosticableTreeMixin {
  PipelineOwner({
    this.onNeedVisualUpdate,
    this.onSemanticsOwnerCreated,
    this.onSemanticsUpdate,
    this.onSemanticsOwnerDisposed,
  }){
    // ...
  }
  
  // ...
}
```

## onNeedVisualUpdate

&emsp;每当与此 PipelineOwner 相关联的渲染对象希望更新其可视外观时调用。

&emsp;这个函数的典型实现会安排一个任务来刷新管线的各个阶段。这个函数可能会被快速连续地调用多次。实现应该注意迅速丢弃重复调用。

&emsp;当 PipelineOwner 附加到 PipelineManifold 并提供 onNeedVisualUpdate 时, 将调用 onNeedVisualUpdate 回调，而不是调用 PipelineManifold.requestVisualUpdate。

```dart
  final VoidCallback? onNeedVisualUpdate;
```

## onSemanticsOwnerCreated

&emsp;每当该 PipelineOwner 创建语义（semantics）对象时调用。典型的实现将安排创建初始语义树（initial semantics tree）。

```dart
  final VoidCallback? onSemanticsOwnerCreated;
```

## onSemanticsUpdate

&emsp;每当这个 PipelineOwner 的语义所有者（semantics owner）发出 SemanticsUpdate 时都会调用。

&emsp;典型的实现方式会将 SemanticsUpdate 委托给一个能够处理 SemanticsUpdate 的 FlutterView。

```dart
  final SemanticsUpdateCallback? onSemanticsUpdate;
```

## onSemanticsOwnerDisposed

&emsp;每当这个 PipelineOwner 销毁其语义所有者（semantics owner）时都会调用此方法。典型的实现将拆除语义树（semantics tree）。

```dart
  final VoidCallback? onSemanticsOwnerDisposed;
```

## requestVisualUpdate

&emsp;如果 onNeedVisualUpdate 不为 null，则调用 onNeedVisualUpdate。用于通知 PipelineOwner，关联的渲染对象希望更新其视觉外观。

```dart
  void requestVisualUpdate() {
    if (onNeedVisualUpdate != null) {
      onNeedVisualUpdate!();
    } else {
      _manifold?.requestVisualUpdate();
    }
  }

```

## rootNode

&emsp;由该 PipelineOwner 管理且没有父级的唯一对象（RenderObject）。

```dart
  RenderObject? get rootNode => _rootNode;
  RenderObject? _rootNode;
  
  set rootNode(RenderObject? value) {
    if (_rootNode == value) {
      return;
    }
    
    _rootNode?.detach();
    _rootNode = value;
    _rootNode?.attach(this);
  }
```

## `_shouldMergeDirtyNodes`

&emsp;在继续处理 dirty relayout boundaries（脏重新布局边界）之前，当前的 flushLayout 调用是否应该暂停以将 `_nodesNeedingLayout` 中的 RenderObject 合并到当前的脏列表中。

&emsp;当一个 RenderObject.invokeLayoutCallback 返回时，这个标志被设置为 true，以避免以错误的顺序布局 dirty relayout boundaries（脏重新布局边界），并导致它们在一个帧内被布局多次。

&emsp;在调用 RenderObject.invokeLayoutCallback 后，新的脏节点不会立即合并，因为在处理单个 relayout boundary（重新布局边界）时，可能会遇到多次这样的调用。批量处理新的脏节点可以减少 flushLayout 需要执行的合并次数。

```dart
  bool _shouldMergeDirtyNodes = false;
```

## `_nodesNeedingLayout`

&emsp;超级重要的一个 List，在一个批次内所有的位于同一个 PipelineOwner 中需要被重新布局的 RenderObject 都被收集在这里。

```dart
  List<RenderObject> _nodesNeedingLayout = <RenderObject>[];
```

## flushLayout

&emsp;更新所有需要重新渲染的渲染对象的布局信息。

&emsp;这个函数是渲染流水线（rendering pipeline）中的核心阶段之一。在绘制之前，布局信息会被清理，以确保渲染对象以其最新位置显示在屏幕上。

&emsp;可以查看 RendererBinding 以了解此函数的使用示例。（后续会深入 RendererBinding 学习。）

```dart
  void flushLayout() {
    try {
      while (_nodesNeedingLayout.isNotEmpty) {
        // 临时变量，记录当前需要重新布局的脏 RenderObject 对象
        final List<RenderObject> dirtyNodes = _nodesNeedingLayout;
        
        // 把当前 PipelineOwner 下的，记录脏 RenderObject 对象的列表置空，
        // 后续有新的脏 RenderObject 的话还会被添加进行。
        _nodesNeedingLayout = <RenderObject>[];
        
        // 还记得在 BuildOwner 中记录的所有的标记为脏的 Element，
        // 它们在统一进行重建之前会进行排序吗？父级排在子级前面去。
        dirtyNodes.sort((RenderObject a, RenderObject b) => a.depth - b.depth);
        
        // 遍历已排序的 dirtyNodes 中记录的脏 RenderObject 对象，
        // 对它们执行重新布局 
        for (int i = 0; i < dirtyNodes.length; i++) {
        
          // 如果 _shouldMergeDirtyNodes 为 true，
          // 并且在当前遍历 dirtyNodes 中脏 RenderObject 进行重新布局过程中，
          // 又有新的脏 RenderObject 被加入到 _nodesNeedingLayout 中的话，
          // 那么把 dirtyNodes 中剩余的未重新布局完成的脏 RenderObject 全部转移到 _nodesNeedingLayout 中，
          // 并跳出当前 for 循环，重新最外层的 while 循环，对所有的脏 RenderObject 对象进行重新布局。
          if (_shouldMergeDirtyNodes) {
            _shouldMergeDirtyNodes = false;
            
            // 在依次对现有的脏渲染对象进行重新布局时，
            // 又有新的渲染对象被标记需要重新布局，它们被添加进 _nodesNeedingLayout 中
            if (_nodesNeedingLayout.isNotEmpty) {
            
              // 把 dirtyNodes 中剩余的脏渲染对象添加到 _nodesNeedingLayout 中去
              _nodesNeedingLayout.addAll(dirtyNodes.getRange(i, dirtyNodes.length));
              break;
            }
          }
          
          // 每次取出一个脏渲染对象，对它进行重新布局
          final RenderObject node = dirtyNodes[i];
          if (node._needsLayout && node.owner == this) {
            // 执行 RenderObject._layoutWithoutResize
            node._layoutWithoutResize();
          }
        }
        
        // 不需要将上一次 relayout boundary 处理后新生成的脏 RenderObject 进行合并
        _shouldMergeDirtyNodes = false;
      }
      
      // 对当前 PipelineOwner 中的子级 PipelineOwner 进行重新布局
      for (final PipelineOwner child in _children) {
        child.flushLayout();
      }
      
    } finally {
      _shouldMergeDirtyNodes = false;
    }
  }
```

## `_nodesNeedingCompositingBitsUpdate`

&emsp;超级重要的一个 List，同 `_nodesNeedingLayout`。

&emsp;在一个批次内所有的位于同一个 PipelineOwner 中需要被重新合成的 RenderObject 都被收集在这里。

```dart
  final List<RenderObject> _nodesNeedingCompositingBitsUpdate = <RenderObject>[];
```

## flushCompositingBits

&emsp;更新 RenderObject.needsCompositing bits。在 flushLayout 之后和 flushPaint 之前作为渲染流水线（rendering pipeline）的一部分被调用。

```dart
  void flushCompositingBits() {
    // 同样的首先进行排序，父级在前面子级在后面
    _nodesNeedingCompositingBitsUpdate.sort((RenderObject a, RenderObject b) => a.depth - b.depth);
    
    // 遍历对所有需要重新合成的 RenderObject 进行合成
    for (final RenderObject node in _nodesNeedingCompositingBitsUpdate) {
      if (node._needsCompositingBitsUpdate && node.owner == this) {
        // 执行重新合成
        node._updateCompositingBits();
      }
    }
    
    // 重新合成完毕了，可以清空了
    _nodesNeedingCompositingBitsUpdate.clear();
    
    // 对当前 PipelineOwner 中的子级 PipelineOwner 进行重新合成
    for (final PipelineOwner child in _children) {
      child.flushCompositingBits();
    }
  }
```

## `_nodesNeedingPaint`

&emsp;超级重要的一个 List，同 `_nodesNeedingLayout`。

&emsp;在一个批次内所有的位于同一个 PipelineOwner 中需要被重新绘制的 RenderObject 都被收集在这里。

```dart
  List<RenderObject> _nodesNeedingPaint = <RenderObject>[];
```

## flushPaint

&emsp;更新所有渲染对象的显示列表（display lists）。

&emsp;该函数是渲染流水线（rendering pipeline）的核心阶段之一。绘制发生在布局之后、场景重新组合之前，以便将场景与每个渲染对象的最新显示列表（up-to-date display lists）一起合成。

&emsp;可以查看 RendererBinding 以了解此函数的使用示例。（后续会深入 RendererBinding 学习。）

```dart
  void flushPaint() {
    try {
      // 临时变量记录下当前需要进行重新绘制的 RenderObject
      final List<RenderObject> dirtyNodes = _nodesNeedingPaint;
      
      // 把 _nodesNeedingPaint 置为空，可以开始记录下一个批次的需要重新绘制的渲染对象了
      _nodesNeedingPaint = <RenderObject>[];

      // 以相反的顺序对脏节点排序(最深的优先)。
      // 注意，和👆上面的重新布局和重新合成的排序是刚好相反的！
      for (final RenderObject node in dirtyNodes..sort((RenderObject a, RenderObject b) => b.depth - a.depth)) {
      
        // 需要重绘或者合成层需要更新的话，并且是位于当前 PipelineOwner 下
        if ((node._needsPaint || node._needsCompositedLayerUpdate) && node.owner == this) {
        
          // layer 已经被附加
          if (node._layerHandle.layer!.attached) {
            if (node._needsPaint) {
            
              // 如果需要的是重绘
              PaintingContext.repaintCompositedChild(node);
            } else {
            
              // 如果需要的是更新层
              PaintingContext.updateLayerProperties(node);
            }
          } else {
            // 
            node._skippedPaintingOnLayer();
          }
        }
      }
      
      // 对当前 PipelineOwner 中的子级 PipelineOwner 进行刷新重绘
      for (final PipelineOwner child in _children) {
        child.flushPaint();
      }
    } finally {
      // ...
    }
  }
```

## semanticsOwner

&emsp;如果有的话，该对象负责管理这个 PipelineOwner 的语义（semantics）。

&emsp;owner 是通过 ensureSemantics 创建的，或者当该 owner 连接的 PipelineManifold 设置 PipelineManifold.semanticsEnabled 为 true 时创建。只要 PipelineManifold.semanticsEnabled 保持为 true，或者在调用 ensureSemantics 时存在未解决的 SemanticsHandle，owner 就是有效的。一旦这两个条件不再满足，semanticsOwner 字段将重新变为 null。

&emsp;当 semanticsOwner 为 null 时，PipelineOwner 将跳过与语义相关的所有步骤。

```dart
  SemanticsOwner? get semanticsOwner => _semanticsOwner;
  SemanticsOwner? _semanticsOwner;
```

## `_updateSemanticsOwner`

&emsp;

```dart
  void _updateSemanticsOwner() {
    if ((_manifold?.semanticsEnabled ?? false) || _outstandingSemanticsHandles > 0) {
      if (_semanticsOwner == null) {
        _semanticsOwner = SemanticsOwner(onSemanticsUpdate: onSemanticsUpdate!);
        
        onSemanticsOwnerCreated?.call();
      }
    } else if (_semanticsOwner != null) {
      _semanticsOwner?.dispose();
      _semanticsOwner = null;
      
      onSemanticsOwnerDisposed?.call();
    }
  }
```

## `_didDisposeSemanticsHandle`

&emsp;

```dart
  void _didDisposeSemanticsHandle() {
    _outstandingSemanticsHandles -= 1;
    _updateSemanticsOwner();
  }
```

## `_nodesNeedingSemantics`

&emsp;

```dart
  final Set<RenderObject> _nodesNeedingSemantics = <RenderObject>{};
```

## flushSemantics

&emsp;

```dart
  void flushSemantics() {
    if (_semanticsOwner == null) {
      return;
    }
    
    try {
      // 临时变量 nodesToProcess 列表中记录已根据 depth 排序的 _nodesNeedingSemantics 中的 RenderObject，
      // 父级在前面，子级在后面
      final List<RenderObject> nodesToProcess = _nodesNeedingSemantics.toList()
        ..sort((RenderObject a, RenderObject b) => a.depth - b.depth);
      
      // 把原 set 清空
      _nodesNeedingSemantics.clear();
      
      // 
      for (final RenderObject node in nodesToProcess) {
        if (node._needsSemanticsUpdate && node.owner == this) {
          node._updateSemantics();
        }
      }
      
      _semanticsOwner!.sendSemanticsUpdate();
      
      for (final PipelineOwner child in _children) {
        child.flushSemantics();
      }
    } finally {
      // ...
    }
  }
```

## `_children`

```dart
  final Set<PipelineOwner> _children = <PipelineOwner>{};
```

## `_manifold`

```dart
  PipelineManifold? _manifold;
```

## attach

&emsp;将此 PipelineOwner 标记为已附加到给定的 PipelineManifold。

&emsp;通常，这只会直接在根 PipelineOwner 上调用。当调用 adoptChild 时，子项会自动附加到其父 PipelineManifold。

```dart
  void attach(PipelineManifold manifold) {
    _manifold = manifold;
    _manifold!.addListener(_updateSemanticsOwner);
    _updateSemanticsOwner();

    for (final PipelineOwner child in _children) {
      child.attach(manifold);
    }
  }
```

## detach

&emsp;将该 PipelineOwner 标记为已分离状态。通常，这只会直接在根 PipelineOwner 上调用。当调用 dropChild 时，子项会自动从其父 PipelineManifold 中分离。

```dart
  void detach() {
    _manifold!.removeListener(_updateSemanticsOwner);
    _manifold = null;
    
    // 为了避免在 re-attached 时干扰任何客户端，此处不会更新 semantics owner。
    // 如果必要，在 "attach" 中将更新 semantics owner，或者在 "dispose" 中将其处置，如果不被重新附加的话。

    for (final PipelineOwner child in _children) {
      child.detach();
    }
  }
```

## adoptChild

&emsp;将 child 添加到此 PipelineOwner 中。

&emsp;在帧生成的阶段（参见 RendererBinding.drawFrame），父 PipelineOwner 将在调用其子 PipelineOwner 上当前阶段对应的 flush 方法之前，完成自己拥有的节点的阶段。例如，在布局期间，父 PipelineOwner 将首先布局自己的节点，然后调用其子节点上的 flushLayout。在绘制期间，它将在绘制自己的节点之前调用其子节点上的 flushPaint。这种顺序也适用于所有其他阶段。

&emsp;不应假设子 PipelineOwners 被刷新的顺序。

&emsp;在 PipelineOwner 开始在任何子节点上调用 flushLayout 后，直到当前帧结束，不得添加新的子节点。

&emsp;要移除子节点，请调用 dropChild。

```dart
  void adoptChild(PipelineOwner child) {
    _children.add(child);
    
    if (_manifold != null) {
      child.attach(_manifold!);
    }
  }
```

## dropChild

&emsp;移除之前通过 adoptChild 添加的子 PipelineOwner。

&emsp;在帧生成过程中，此节点将不再调用子节点的 flush 方法。

&emsp;在 PipelineOwner 开始调用任何子节点的 flushLayout 后，直到当前帧结束之前，不能移除任何子节点。

```dart
  void dropChild(PipelineOwner child) {
    _children.remove(child);
    
    if (_manifold != null) {
      child.detach();
    }
  }

```

## visitChildren

&emsp;为该 PipelineOwner 的每个直接子节点调用 visitor。

```dart
  void visitChildren(PipelineOwnerVisitor visitor) {
    _children.forEach(visitor);
  }
```

## dispose

&emsp;释放由此 PipelineOwner 持有的任何资源。

&emsp;在调用此方法之前，此 PipelineOwner 必须从 pipeline owner tree 中移除，也就是说它必须既没有父级也没有任何子级（参见 dropChild）。它还必须与任何 PipelineManifold 分离。

&emsp;调用 dispose 后，该对象不再可用。

```dart
  void dispose() {
    _semanticsOwner?.dispose();
    _semanticsOwner = null;
    
    _nodesNeedingLayout.clear();
    _nodesNeedingCompositingBitsUpdate.clear();
    _nodesNeedingPaint.clear();
    _nodesNeedingSemantics.clear();
  }
```















## 参考链接
**参考链接:🔗**
+ [PipelineOwner class](https://api.flutter.dev/flutter/rendering/PipelineOwner-class.html)
