# Flutter 源码梳理系列（二十四）：PipelineOwner

&emsp;首先我们要有一个意识哦：经过前面学习脏 Element 列表中所有脏 Element 重建的过程，我们知道 Element 重建并不是在当前帧进行的，而是在当前帧会把所有的脏 Element 对象收集起来，然后再通过 WidgetsBinding.drawFrame 的回调，在下一帧然后进行批量重建。

&emsp;下面是 WidgetsBinding.drawFrame 函数的内容：

```dart
@override
  void drawFrame() {
    try {
      // 首先是对 Element Tree 的根节点 rootElement 下的 BuildScope 进行重建。
      // 此 BuildScope 即 Element Tree 上所有 Element 节点共用的 build scope。
      // 对其中的所有脏 Element 节点进行排序重建。
      if (rootElement != null) {
        buildOwner!.buildScope(rootElement!);
      }
      
      // ⬇️⚠️ 这里便是调用 RendererBinding.drawFrame 函数。
      super.drawFrame();
      
      // Element Tree 中脏 Element 重建完毕后，
      // 对剩余的非活动 Element 节点调用 unmount 进行卸载回收，
      // 它们已经完全不会再被用到了，卸载即可。
      buildOwner!.finalizeTree();
    } finally {
      // ...
    }
  }
```

&emsp;注意到其中的：super.drawFrame() 调用，其实就是调用的：RendererBinding.drawFrame，然后在其内部调用 flushLayout、flushCompositingBits、flushPaint 函数对需要重新布局的、合成更新的、重新绘制的 RenderObject 对象进行批量处理。

&emsp;是的，RenderObject 的重新布局和重新绘制等操作是和脏 Element 的重建是一样的，都是在当前帧进行统一收集，然后在下一帧进行批量处理。

&emsp;在涉及脏 Element 节点处理重建的时候，我们学习了 BuildOwner，它来负责收集整理标记为脏的 Element 对象。那么到了 RenderObject 这里，是谁来管理这个过程的呢？是的，没错，正是 PipelineOwner，下面我们对 PipelineOwner 的内容进行集中学习。 

# PipelineOwner

&emsp;PipelineOwner 负责管理渲染管线（rendering pipeline）。

&emsp;PipelineOwner 提供了一个接口来驱动渲染管线，并存储关于在管线的每个阶段中请求被访问的 RenderObject 的状态。要刷新管线，请按照以下顺序调用这些函数：（即 PipelineOwner 的三个属性：`List<RenderObject> _nodesNeedingLayout`、`List<RenderObject> _nodesNeedingPaint`、`final Set<RenderObject> _nodesNeedingSemantics` 它们负责存储需要重新布局、需要重新绘制、需要重新语义化的 RenderObject。然后是刷新管线的如下顺序调用的三个函数正是在 RendererBinding.drawFrame 中调用的。RendererBinding.drawFrame 函数如下：）

&emsp;RendererBinding.drawFrame 函数内正是下面👇四个函数的调用顺序。

```dart
  @protected
  void drawFrame() {
    // 1️⃣ flushLayout
    rootPipelineOwner.flushLayout();
    
    // 2️⃣ flushCompositingBits
    rootPipelineOwner.flushCompositingBits();
    
    // 3️⃣ flushPaint
    rootPipelineOwner.flushPaint();
    
    if (sendFramesToEngine) {
      for (final RenderView renderView in renderViews) {
        // this sends the bits to the GPU
        renderView.compositeFrame();
      }
      
      // this sends the semantics to the OS.
      // 4️⃣ flushSemantics
      rootPipelineOwner.flushSemantics(); 
      
      _firstFrameSent = true;
    }
  }
```

1. `flushLayout` 更新任何需要计算其布局的 RenderObject 对象。在此阶段，计算每个 RenderObject 对象的大小和位置。在这个阶段，RenderObject 对象可能会在其绘制（painting）或合成状态（compositing）中标记为脏。

2. `flushCompositingBits` 更新具有脏混合位的任何 RenderObject 对象。在此阶段，每个 RenderObject 对象都会了解其子级是否需要混合。在绘制阶段使用此信息来选择如何实现诸如裁剪之类的视觉效果（看到这里想到了 iOS 的离屏渲染绘制圆角、阴影、mask 等。）。如果一个 RenderObject 对象具有一个合成子级，它需要使用一个 Layer 来创建 clip，以便 clip 应用到合成子级上（后者将绘制到自己的 Layer 中）。

3. `flushPaint` 会访问需要绘制的任何 RenderObject 对象。在此阶段，RenderObject 对象有机会将绘制命令记录到 PictureLayer 中，并构造其他合成 Layer。

4. 最后，如果启用了语义化，`flushSemantics` 将为 RenderObject 对象编译语义。这些语义信息被辅助技术用于改善 Render Tree 的可访问性。

&emsp;RendererBinding 持有屏幕上可见的渲染对象的 PipelineOwner。你可以创建其他 PipelineOwner 来管理屏幕外（off-screen）的对象，这些对象可以独立于屏幕上（on-screen）的渲染对象刷新它们的管线。（即 RendererBinding 的 rootPipelineOwner 属性，Render Tree 上的所有 RenderObject 节点的 `PipelineOwner? _owner` 属性都指向它。）

&emsp;PipelineOwner 可以按树状结构组织，用于管理多个 Render Tree，其中每个 PipelineOwner 负责一个 Render Tree。要构建或修改 PipelineOwner Tree，请调用 adoptChild 或 dropChild。在上面描述的不同 flush 阶段中，PipelineOwner 在调用其子级的相同 flush 方法之前，将首先在其自己的 Render Tree 中管理的 RenderObject 节点上执行该阶段。不应该假设子级 PipelineOwner 的 flush 顺序。(即 PipelineOwner 的 flush 一组函数中，首先要调用自己的收集的 RenderObject 的刷新，然后才是 PipelineOwner 子级的刷新。)

&emsp;一个 PipelineOwner 也可以附加（调用：`void attach(PipelineManifold manifold)` 函数。）到一个 PipelineManifold，这样它就可以访问通常由 binding 暴露的平台功能，而不必将其绑定到特定的 binding 实现。在给定 PipelineOwner Tree 中的所有 PipelineOwner 节点必须附加到相同的 PipelineManifold。这在 adoptChild 中会自动发生。（即 PipelineOwner Tree 中所有的 PipelineOwner 节点的 `PipelineManifold? _manifold` 属性都指向同一个 PipelineManifold 对象。）

&emsp;OK，下面看一下 PipelineOwner 的源码。

## Constructors

&emsp;创建一个 PipelineOwner。通常由 binding（例如 RendererBinding）创建，但也可以独立于 binding 创建，以通过 rendering pipeline 驱动屏幕外（off-screen）渲染对象。

&emsp;参数是四个回调函数。

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

&emsp;每当与此 PipelineOwner 相关联的 RenderObject 对象希望更新其可视外观时调用。

&emsp;这个函数的典型实现会安排一个任务来刷新管线的各个阶段。这个函数可能会被快速连续地调用多次。实现应该注意迅速丢弃重复调用。

&emsp;当 PipelineOwner 附加到 PipelineManifold 并提供 onNeedVisualUpdate 时, 将调用 onNeedVisualUpdate 回调，而不是调用 PipelineManifold.requestVisualUpdate。

```dart
  final VoidCallback? onNeedVisualUpdate;
```

## onSemanticsOwnerCreated

&emsp;每当该 PipelineOwner 创建语义（semantics）对象时调用。典型的实现将安排创建初始语义树（initial semantics tree）。（看到此回调执行时会调用：RenderObject.scheduleInitialSemantics 函数。）

```dart
  final VoidCallback? onSemanticsOwnerCreated;
```

## onSemanticsUpdate

&emsp;每当这个 PipelineOwner 的语义所有者（semantics owner）发出 SemanticsUpdate 时都会调用。

&emsp;典型的实现方式会将 SemanticsUpdate 委托给一个能够处理 SemanticsUpdate 的 FlutterView。（看到此回调执行时会调用：RenderView.updateSemantics 函数。）

```dart
  final SemanticsUpdateCallback? onSemanticsUpdate;
```

## onSemanticsOwnerDisposed

&emsp;每当这个 PipelineOwner 销毁其语义所有者（semantics owner）时都会调用此方法。典型的实现将拆除语义树（semantics tree）。（看到此回调执行时会调用：RenderObject.clearSemantics 函数。）

```dart
  final VoidCallback? onSemanticsOwnerDisposed;
```

## requestVisualUpdate

&emsp;如果 onNeedVisualUpdate 不为 null，则调用 onNeedVisualUpdate。用于通知 PipelineOwner，关联的 RenderObject 希望更新其视觉外观。

```dart
  void requestVisualUpdate() {
    if (onNeedVisualUpdate != null) {
      onNeedVisualUpdate!();
    } else {
      // 正常情况下会调用这里。
      _manifold?.requestVisualUpdate();
    }
  }
```

## rootNode

&emsp;由该 PipelineOwner 管理且没有父级（parent 指向为 null）的唯一对象（RenderObject）。（即：当前 PipelineOwner 管理的 Render Tree 的根节点。）

```dart
  RenderObject? get rootNode => _rootNode;
  RenderObject? _rootNode;
  
  set rootNode(RenderObject? value) {
    if (_rootNode == value) {
      return;
    }
    
    // 首先进行一个清理释放之前的引用后，再进行新的赋值。
    // 内部时把之前的旧 _rootNode 引用的 owner 置为 null，
    // 即之前的旧 _rootNode 不再引用当前的 PipelineOwner 对象了。
    _rootNode?.detach();
    
    // 然后把新的 RenderObject 对象赋值给当前 PipelineOwner 对象的 rootNode 属性
    _rootNode = value;
    
    // 把 _rootNode 的 owner 属性指向当前这个 PipelineOwner 对象
    _rootNode?.attach(this);
  }
```

## `_shouldMergeDirtyNodes`

&emsp;在继续处理 dirty relayout boundaries（脏重新布局边界）之前，当前的 flushLayout 调用是否应该暂停以将 `_nodesNeedingLayout` 中的 RenderObject 合并到当前的脏列表中。（具体细节在 flushLayout 函数内，打断当前 for 循环，重新进入 while 循环。）

&emsp;当一个 RenderObject.invokeLayoutCallback 返回时（就表示有新的标记为需要重新布局的 RenderObject 对象添加到了 `_nodesNeedingLayout` 列表中），这个标志被设置为 true，以避免以错误的顺序布局 dirty relayout boundaries（脏重新布局边界），并导致它们在一个帧内被布局多次。

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

&emsp;更新所有需要重新渲染的渲染对象的布局信息。（`_nodesNeedingLayout` 列表中收集的。）

&emsp;这个函数是渲染流水线（rendering pipeline）中的核心阶段之一。在绘制之前，布局信息会被清理，以确保渲染对象以其最新位置显示在屏幕上。

&emsp;可以查看 RendererBinding 以了解此函数的使用示例。（即在 RendererBinding.drawFrame 函数中被调用，在新的一帧中刷新渲染对象的布局信息。）

```dart
  void flushLayout() {
    try {
      while (_nodesNeedingLayout.isNotEmpty) {
        // 临时变量，记录当前需要重新布局的脏 RenderObject 对象
        final List<RenderObject> dirtyNodes = _nodesNeedingLayout;
        
        // 把当前 PipelineOwner 下的，记录脏 RenderObject 对象的列表置空，
        // 后续有新的脏 RenderObject 的话还会被添加进来。
        _nodesNeedingLayout = <RenderObject>[];
        
        // 还记得在 BuildOwner 中记录的所有的标记为脏的 Element，
        // 它们在统一进行重建之前会进行排序吗？父级排在子级前面去。
        
        // 这里也一样，对脏 RenderObject 对象根据它们的 depth 值进行排序，
        // 把 depth 小的排在前面，即父级 RenderObject 在子级 RenderObject 前面去，
        // 防止子级 RenderObject 被重叠布局。
        dirtyNodes.sort((RenderObject a, RenderObject b) => a.depth - b.depth);
        
        // 遍历已排序的 dirtyNodes 中记录的脏 RenderObject 对象，
        // 对它们执行重新布局 
        for (int i = 0; i < dirtyNodes.length; i++) {
        
          // 如果 _shouldMergeDirtyNodes 为 true 表示有新的 RenderObject 被加入到 _nodesNeedingLayout 中了，
          // 当前正在遍历 dirtyNodes 中脏 RenderObject 进行重新布局过程中，
          // 又有新的脏 RenderObject 被加入到 _nodesNeedingLayout 中的话，
          // 那么把 dirtyNodes 中剩余的未重新布局完成的脏 RenderObject 全部转移到 _nodesNeedingLayout 中，
          // 并跳出当前 for 循环，重新最外层的 while 循环，对所有的脏 RenderObject 对象进行重新布局。
          if (_shouldMergeDirtyNodes) {
            _shouldMergeDirtyNodes = false;
            
            // 在依次对现有的脏 RenderObject 进行重新布局时，
            // 又有新的 RenderObject 被标记需要重新布局，
            // 它们被添加进 _nodesNeedingLayout 中
            if (_nodesNeedingLayout.isNotEmpty) {
            
              // 把 dirtyNodes 中剩余的未来的及重新布局的脏 RenderObject 添加到 _nodesNeedingLayout 中去
              _nodesNeedingLayout.addAll(dirtyNodes.getRange(i, dirtyNodes.length));
              
              // 然后跳出这个 for 循环，回到前面的 while 循环
              break;
            }
          }
          
          // 每次取出一个脏 RenderObject，对它进行重新布局
          final RenderObject node = dirtyNodes[i];
          
          if (node._needsLayout && node.owner == this) {
            // 执行 RenderObject._layoutWithoutResize 函数。
            // 执行 performLayout 执行布局，
            // 并被标记需要语义更新，并被标记需要重绘，
            // 并把 _needsLayout 置为 false，表示当前不再需要被重新布局了
            node._layoutWithoutResize();
          }
        }
        
        // 不需要将上一次 relayout boundary 处理后新生成的脏 RenderObject 进行合并
        _shouldMergeDirtyNodes = false;
      }
      
      // 对当前 PipelineOwner 中的子级 PipelineOwner 进行重新布局刷新。
      for (final PipelineOwner child in _children) {
        child.flushLayout();
      }
      
    } finally {
      // 脏 RenderObject 合并标识置为 false 
      _shouldMergeDirtyNodes = false;
    }
  }
```

## `_nodesNeedingCompositingBitsUpdate`

&emsp;超级重要的一个 List，同 `_nodesNeedingLayout`。

&emsp;在一个批次内所有的位于同一个 PipelineOwner 中需要进行合成更新的 RenderObject 都被收集在这里。

```dart
  final List<RenderObject> _nodesNeedingCompositingBitsUpdate = <RenderObject>[];
```

## flushCompositingBits

&emsp;更新 RenderObject.needsCompositing bits。在 flushLayout 之后和 flushPaint 之前作为渲染流水线（rendering pipeline）的一部分被调用。

```dart
  void flushCompositingBits() {
    // 同样的首先进行排序，父级在前面子级在后面
    _nodesNeedingCompositingBitsUpdate.sort((RenderObject a, RenderObject b) => a.depth - b.depth);
    
    // 遍历对所有需要合成更新的 RenderObject 进行合成更新
    for (final RenderObject node in _nodesNeedingCompositingBitsUpdate) {
      
      if (node._needsCompositingBitsUpdate && node.owner == this) {
        // 执行合成更新
        node._updateCompositingBits();
      }
    }
    
    // 合成更新完毕了，可以清空 _nodesNeedingCompositingBitsUpdate 列表了
    _nodesNeedingCompositingBitsUpdate.clear();
    
    // 对当前 PipelineOwner 的子级 PipelineOwner 进行合成更新刷新。
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

&emsp;可以查看 RendererBinding 以了解此函数的使用示例。（即在 RendererBinding.drawFrame 函数中被调用，在新的一帧中刷新渲染对象的绘制信息，且调用位置在 flushLayout、flushCompositingBits 后面。）

```dart
  void flushPaint() {
    try {
      // 临时变量记录下当前需要进行重新绘制的 RenderObject
      final List<RenderObject> dirtyNodes = _nodesNeedingPaint;
      
      // 把 _nodesNeedingPaint 置为空，可以开始记录下一个批次的需要重新绘制的渲染对象了
      _nodesNeedingPaint = <RenderObject>[];

      // 以相反的顺序对脏 RenderObject 排序(最深的优先)。
      // 即子级 RenderObject 在父级 RenderObject 的前面。
      // 注意，和👆上面的重新布局和合成更新的排序是刚好相反的！
      for (final RenderObject node in dirtyNodes..sort((RenderObject a, RenderObject b) => b.depth - a.depth)) {
      
        // 需要重绘或者合成层需要更新的话，并且是位于当前 PipelineOwner 下
        if ((node._needsPaint || node._needsCompositedLayerUpdate) && node.owner == this) {
        
          // node._layerHandle.layer 是否已经被附加了
          if (node._layerHandle.layer!.attached) {
            
            // 能进到这里的，就说明 RenderObject 需要重新绘制或者是合成层更新
            if (node._needsPaint) {
              // 1️⃣ 需要的是重绘
              PaintingContext.repaintCompositedChild(node);
            } else {
              // 2️⃣ 需要的是更新层
              PaintingContext.updateLayerProperties(node);
            }
          } else {
            // node._layerHandle.layer 如果未被附加的话，
            
            // 当 flushPaint() 试图让我们绘制，但我们的层被分离时调用。
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

&emsp;更新 `_semanticsOwner` 属性，赋值或者置为 null。

```dart
  void _updateSemanticsOwner() {
    if ((_manifold?.semanticsEnabled ?? false) || _outstandingSemanticsHandles > 0) {
      if (_semanticsOwner == null) {
        // 创建一个 SemanticsOwner
        _semanticsOwner = SemanticsOwner(onSemanticsUpdate: onSemanticsUpdate!);
        
        // 回调 SemanticsOwner 新建了，
        // 会回调到 RenderObject.scheduleInitialSemantics 函数。
        onSemanticsOwnerCreated?.call();
      }
    } else if (_semanticsOwner != null) {
      _semanticsOwner?.dispose();
      _semanticsOwner = null;
      
      // 回调 SemanticsOwner 销毁了，
      // 会回调到 RenderObject.clearSemantics 函数。
      onSemanticsOwnerDisposed?.call();
    }
  }
```

## `_nodesNeedingSemantics`

&emsp;超级重要的一个 List，同 `_nodesNeedingLayout`。

&emsp;在一个批次内所有的位于同一个 PipelineOwner 中需要被语义更新的 RenderObject 都被收集在这里。

```dart
  final Set<RenderObject> _nodesNeedingSemantics = <RenderObject>{};
```

## flushSemantics

&emsp;更新被标记为需要语义更新的 RenderObject 的语义。

&emsp;最初，只有通过 RenderObject.scheduleInitialSemantics 安排的根节点需要进行语义更新。

&emsp;这个函数是渲染管线的核心阶段之一。语义是在绘制之后编制的，只有在调用 RenderObject.scheduleInitialSemantics 之后才会进行。

```dart
  void flushSemantics() {
    // 如果当前没有 _semanticsOwner 则直接返回即可。
    if (_semanticsOwner == null) {
      return;
    }
    
    try {
      // 临时变量 nodesToProcess 列表中记录已根据 depth 排序的 _nodesNeedingSemantics 中的 RenderObject，
      // 父级在前面，子级在后面
      final List<RenderObject> nodesToProcess = _nodesNeedingSemantics.toList()
        ..sort((RenderObject a, RenderObject b) => a.depth - b.depth);
      
      // 把原 _nodesNeedingSemantics 清空即可
      _nodesNeedingSemantics.clear();
      
      // 循环对 nodesToProcess 中的 RenderObject 对象调用 _updateSemantics
      for (final RenderObject node in nodesToProcess) {
        if (node._needsSemanticsUpdate && node.owner == this) {
          node._updateSemantics();
        }
      }
      
      _semanticsOwner!.sendSemanticsUpdate();
      
      // 对当前 PipelineOwner 中的子级 PipelineOwner 进行语义更新
      for (final PipelineOwner child in _children) {
        child.flushSemantics();
      }
    } finally {
      // ...
    }
  }
```

## `_children`

&emsp;当前 PipelineOwner 的所有子级 PipelineOwner。

```dart
  final Set<PipelineOwner> _children = <PipelineOwner>{};
```

## `_manifold`

&emsp;管理当前 PipelineOwner Tree 的 PipelineManifold 对象。

```dart
  PipelineManifold? _manifold;
```

## attach

&emsp;将此 PipelineOwner 标记为已附加到给定的 PipelineManifold。

&emsp;通常，这只会直接在根 PipelineOwner 上调用。当调用 adoptChild 时，子级 PipelineOwner 会自动附加到其父 PipelineManifold。

```dart
  void attach(PipelineManifold manifold) {
    // 当前 PipelineOwner 的 _manifold 指向此 manifold
    _manifold = manifold;
    
    _manifold!.addListener(_updateSemanticsOwner);
    _updateSemanticsOwner();

    // 所有的子级 PipelineOwner 的 _manifold 属性都指向此 manifold
    for (final PipelineOwner child in _children) {
      child.attach(manifold);
    }
  }
```

## detach

&emsp;将该 PipelineOwner 标记为已分离状态。通常，这只会直接在根 PipelineOwner 上调用。当调用 dropChild 时，子级 PipelineOwner 会自动从其父 PipelineManifold 中分离。

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

&emsp;将 child 子级 PipelineOwner 添加到此 PipelineOwner 中。

&emsp;在帧生成的阶段（RendererBinding.drawFrame），父 PipelineOwner 将在调用其子 PipelineOwner 上当前阶段对应的 flush 方法之前，完成自己拥有的节点的阶段。例如，在布局期间，父 PipelineOwner 将首先布局自己的节点（RenderObject），然后调用其子级 PipelineOwner 上的 flushLayout。在绘制过程中，它会先绘制自己的节点（RenderObject），然后再调用子级 PipelineOwner 的 flushPaint 方法。这种顺序在所有其他阶段也适用。

&emsp;不应假设子级 PipelineOwner 被刷新的顺序。

&emsp;在 PipelineOwner 开始在任何子级 PipelineOwner 上调用 flushLayout 后，直到当前帧结束，不得添加新的子级 PipelineOwner。

&emsp;要移除子节点，请调用 dropChild。

```dart
  void adoptChild(PipelineOwner child) {
    // 把 child 添加到自己的 _children 集合中
    _children.add(child);
    
    // 如果 _manifold 不为 null，
    // 则 child 的 _manifold 以及 child 的子级 PipelineOwner 也指向此 _manifold，
    // 即 PipelineOwner Tree 上的所有节点的 _manifold 都是指向同一个。
    if (_manifold != null) {
      child.attach(_manifold!);
    }
  }
```

## dropChild

&emsp;移除之前通过 adoptChild 添加的子级 PipelineOwner。

&emsp;在生成帧期间，该 PipelineOwner 将停止调用子级上的 flush 方法。

&emsp;在 PipelineOwner 开始调用任何子级 PipelineOwner 的 flushLayout 后，直到当前帧结束之前，不能移除任何子级 PipelineOwner。

```dart
  void dropChild(PipelineOwner child) {
    // 把此 child 从当前 PipelineOwner 的 _children 集合中移除
    _children.remove(child);
    
    // 把 child 的 _manifold 以及 child 的所有子级 PipelineOwner 的 _manifold 指向都置为 null
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

&emsp;在调用此方法之前，此 PipelineOwner 必须从 PipelineOwner Tree 中移除，也就是说它必须既没有父级也没有任何子级。它还必须与任何 PipelineManifold 分离。

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

## PipelineOwner 总结

&emsp;不出意外的我们又学了一颗新树：PipelineOwner Tree。至此我们已经学习了：Widget Tree、Element Tree、Notification Tree、Render Tree、PipelineOwner Tree 五颗树🌲了。

&emsp;通过四个属性可以看出 PipelineOwner 是：需要重新布局、需要合成更新、需要重绘、需要语义化的 RenderObject 的管理者。

+ `List<RenderObject> _nodesNeedingLayout = <RenderObject>[];`
+ `final List<RenderObject> _nodesNeedingCompositingBitsUpdate = <RenderObject>[];`
+ `List<RenderObject> _nodesNeedingPaint = <RenderObject>[];`
+ `final Set<RenderObject> _nodesNeedingSemantics = <RenderObject>{};`

&emsp;当前 Render Tree 上的所有 RenderObject 对象的 `PipelineOwner? _owner` 都指向 RendererBinding 的 pipelineOwner 属性。

&emsp;然后在 RendererBinding.drawFrame 回调函数中，即在新的一帧中对前一帧分别收集到的上述四个属性中的 RenderObject 进行刷新（并会根据 depth 进行排序，父级靠前，子级靠后，防止子级 RenderObject 被重复操作，flushPaint 中则相反，子级靠前，父级靠后），依次是：

1. `rootPipelineOwner.flushLayout();`
2. `rootPipelineOwner.flushCompositingBits();`
3. `rootPipelineOwner.flushPaint();`
4. `rootPipelineOwner.flushSemantics();`

&emsp;OK，PipelineOwner 的内容就到这里吧，下面直击 RenderObject class。

## 参考链接
**参考链接:🔗**
+ [PipelineOwner class](https://api.flutter.dev/flutter/rendering/PipelineOwner-class.html)
+ [RendererBinding mixin](https://api.flutter.dev/flutter/rendering/RendererBinding-mixin.html)
+ [PipelineManifold class](https://api.flutter.dev/flutter/rendering/PipelineManifold-class.html)
+ [Listenable class](https://api.flutter.dev/flutter/foundation/Listenable-class.html)
+ [ChangeNotifier class](https://api.flutter.dev/flutter/foundation/ChangeNotifier-class.html)
