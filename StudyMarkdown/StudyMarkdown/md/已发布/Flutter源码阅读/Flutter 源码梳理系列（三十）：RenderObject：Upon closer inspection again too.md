# Flutter 源码梳理系列（三十）：初次调用 PipelineOwner.flushPaint 都做了什么？

# 前言

&emsp;继续接上篇的探索进度。本篇的重点是 RendererBinding.drawFrame 中调用的 PipelineOwner 刷新函数，不过我们重点只放在其中两个刷新函数：flushCompositingBits 和 flushPaint 上，关于 flushLayout 内容的话，前面的 RenderObject LAYOUT 部分已经详细分析过了，这里就不再重复了。

&emsp;通过上一篇的学习，我们知道当断点首次命中 RendererBinding.drawFrame 时 Widget Tree、Element Tree、Render Tree 已经构建好了，并且 Render Tree 根节点也已经被全局 PipelineOwner 收集好了，就待执行以 Render Tree 根节点为起点的 **布局、合成位更新和绘制** 工作了，即执行 PipelineOwner 的三连击 flush 函数。

&emsp;在之前我们学习 RenderObject LAYOUT 和 PipelineOwner 时有详细学习 flushLayout 相关的函数：

+ PipelineOwner.flushLayout、
+ RenderObject._layoutWithoutResize、
+ RenderObject.performLayout、
+ RenderObject.layout、

&emsp;这些函数中，特别是 RenderObject.performLayout 和 RenderObject.layout 函数。当父级 RenderObject 调用 performLayout 函数时，会依次调用自己子级的 layout 函数，然后子级的 layout 内部会再调用 performLayout 函数，依次不断的往子级中递归调用，直到整个 Render 子树上所有 RenderObject 节点完成布局。

&emsp;所以当以 Render Tree 的根节点为起点在 PipelineOwner.flushLayout 函数内部调用 RenderObject._layoutWithoutResize 函数时，其实就是对整颗 Render Tree 上的所有节点进行布局。

&emsp;那么当以 Render Tree 的根节点为起点时，PipelineOwner.flushCompositingBits 和 PipelineOwner.flushPaint 的执行逻辑又是如何呢？下面我们一起跟着函数调用堆栈来看一下。

# flushCompositingBits

&emsp;在 PipelineOwner.flushCompositingBits 函数内，会首先针对收集到的 RenderObject 对象根据 depth 进行排序，且排序规则是把 depth 小的排在前面，即把父级排在子级前面去，然后从父级往子级进行遍历调用它们的 RenderObject._updateCompositingBits 函数。

&emsp;这里之所以要排序是因为以某个父级节点开始调用它的 RenderObject._updateCompositingBits 函数起，它会一直沿着自己的 child 链往下调用，直到叶子节点，并且对整个链路上的所有 RenderObject 调用完毕后都会把它们的 `_needsCompositingBitsUpdate` 属性标识置为 false，这样就防止了后续某个子级再调用自己的 RenderObject._updateCompositingBits 函数时，造成进行重复操作的浪费。当子级调用自己的 RenderObject._updateCompositingBits 时，发现 `_needsCompositingBitsUpdate` 属性已经是 false 了，它们就会提前返回，

&emsp;因为目前我们只有一个 Render Tree 的根节点，所以我们可以完全忽略 PipelineOwner.flushCompositingBits 函数中其它部分的内部，只要把目光聚集在 `node._updateCompositingBits();` 行即可。即我们只需要关注当 Render Tree 根节点调用 `_updateCompositingBits` 函数时内部都会做什么？看起来是比 Layout 要复杂的，但是首先我们要明白一点 `_updateCompositingBits` 函数的目的是什么？它的内部其实仅是为了更新每个调用此函数的 RenderObject 节点的 `_needsCompositing` 属性的值的，说是更新吧也不对，并不是更新，应该是计算 `_needsCompositing` 属性值的，`_needsCompositing` 是一个 bool 值，所以 RenderObject 节点的 `_needsCompositing` 属性值它不是 true 就是 fasle，那么谁又能决定此值是 true 还是 false 呢？由其子级决定父级的或者是由其自身的 isRepaintBoundary 和 alwaysNeedsCompositing 两个 getter 来决定！ 

&emsp;`_updateCompositingBits` 函数是一个递归函数，父级 RenderObject 会向子级 RenderObject 中调用直到叶子节点，并且会把子级的 `_needsCompositing` 为 true 时，传递给父级，父级也需要把 `_needsCompositing` 置为 true，即子级传递给父级。

&emsp;然后还有一种情况，当 RenderObject 的 isRepaintBoundary 属性为 true，或者 alwaysNeedsCompositing 属性为 true，那么 `_needsCompositing` 也要赋值为 true，不过 RenderObject 的这两个 getter 默认都是 false，但是 RenderObject 子类可能回重写它们。

```dart
void _updateCompositingBits() {
    // 如果 合成位更新标识 为 false，那说明不需要更新，直接返回即可。
    if (!_needsCompositingBitsUpdate) {
      return;
    }
    
    // 使用临时变量记录 _needsCompositing 的值（旧值）
    final bool oldNeedsCompositing = _needsCompositing;
    
    // 直接把 _needsCompositing 置为 false，
    // 并且仅有两种情况会被置回 true，如下面的 情况 1️⃣ 和 情况 2️⃣。
    _needsCompositing = false;
    
    // 向当前的 RenderObject 的所有子级递归调用 _updateCompositingBits 函数。
    visitChildren((RenderObject child) {
    
      // ⬇️⬇️ 这里会用当前 RenderObject 的子级继续递归调用 _updateCompositingBits 函数。
      child._updateCompositingBits();
      // ⬆️⬆️ 
      
      // 情况 1️⃣：如果子级的 _needsCompositing 为 true，那么父级也要置回 true。
      if (child.needsCompositing) {
        _needsCompositing = true;
      }
    });
    
    // 情况 2️⃣：如果当前 RenderObject 的 isRepaintBoundary 属性为 true，
    // 或者 alwaysNeedsCompositing 属性为 true，那么把 _needsCompositing 置回 true，
    // RenderObject 的这两个属性默认都是 false，RenderObject 子类可能回重写它们。
    if (isRepaintBoundary || alwaysNeedsCompositing) {
      _needsCompositing = true;
    }
    
    // 如果一个节点之前是一个重绘边界，但现在不再是了，那么无论其合成状态如何，我们都需要找到一个新的父节点来绘制。
    // 为了做到这一点，我们再次标记为干净状态，以便在 markNeedsPaint 中的遍历不被提前终止。
    // 它会从 _nodesNeedingPaint 中移除，这样我们在找到父节点之后就不会尝试从它绘制。

    if (!isRepaintBoundary && _wasRepaintBoundary) {
      // 这里针对的情况是一个 RenderObject 节点之前是一个重新绘制边界，
      // 但是现在不是了，针对这种转变，肯定需要进行重新绘制。
      
      // 这里先把它的绘制状态置为 false 并把它从绘制列表中移除，
      // 然后再调用 markNeedsPaint 找它的最近的是重新绘制边界的父级 RenderObject，并把其标记为需要绘制，
      _needsPaint = false;
      
      _needsCompositedLayerUpdate = false;
      
      owner?._nodesNeedingPaint.remove(this);
      
      _needsCompositingBitsUpdate = false;
      
      // 标记需要绘制。
      markNeedsPaint();
    } else if (oldNeedsCompositing != _needsCompositing) {
      // 合成位更新完毕了，可以把 _needsCompositingBitsUpdate 置回 false 了。
      _needsCompositingBitsUpdate = false;
      
      // 因为之前的 合成位 和现在的 合成位 的值不一样了，
      // 所以此 RenderObject 肯定需要重新绘制，所以调用 markNeedsPaint 标记需要绘制。
      markNeedsPaint();
    } else {
    
      // 合成位更新完毕了，且没有发生变化，
      // 仅需要把 _needsCompositingBitsUpdate 标识位置为 false，
      // 说明当前这个 RenderObject 对象的合成位更新已经完毕了。
      
      _needsCompositingBitsUpdate = false;
    }
  }
```

# First PipelineOwner.flushCompositingBits 总结

&emsp;由于是初次调用 PipelineOwner.flushCompositingBits 函数，所以当前情况下所有的 RenderObject 节点的 `_needsPaint` 属性都是 true，所以在 `_updateCompositingBits` 内部调用的 `markNeedsPaint()` 函数并不会把某个 RenderObject 节点添加到全局 PipelineOwner 的 `_nodesNeedingPaint` 列表中，此列表在 PipelineOwner.flushCompositingBits 执行完毕后，还会是仅有一个 Render Tree 的根节点。

&emsp;所以初次调用 PipelineOwner.flushCompositingBits 函数担负起的任务仅仅是沿着 RenderObject 链更新每个 RenderObject 对象的 `_needsCompositing` 属性的值。并且遵循如下规则：

+ 如果子级 RenderObject 的 `_needsCompositing` 为 true，则父级 RenderObject 的 `_needsCompositing` 属性也是 true。
+ 如果 RenderObject 对象 isRepaintBoundary 或 alwaysNeedsCompositing 属性是 true，则它的 `_needsCompositing` 属性也会是 true。

# flushPaint

&emsp;PipelineOwner.flushPaint 函数到底是干了啥呢？初始时的调用，其目的仅仅是沿着 Render Tree 的根节点开始一层一层的构建 Layer Tree，首先这个 Layer Tree 的根节点就是我们前面提了无数次的首帧准备时的为 Render Tree 的根节点 layer 赋值的 TransformLayer 实例对象，然后是以此为根这个 Layer Tree 的构建，由于 Layer 节点也存在某个 Layer 节点有多个子级的情况，所以 Layer Tree 也是一个树形结构。然后就是每个 Layer 节点的由来，当沿着 RenderObject 链往下的过程中并不是说每个 RenderObject 节点都会创建一个 Layer 节点，而是只有是绘制边界的 RenderObject 才会有独立的 Layer，并把以它为根的多个子级 RenderObject 对象绘制在这一个 Layer 上，即多个 RenderObject 共用一个 Layer，然后就是这每个新的 Layer 创建出来后都会被拼接在前一个父级 Layer 上，所以最终沿着 Render Tree 就构建出了完整的 Layer Tree。

&emsp;所以其实这个被初次调用的 PipelineOwner.flushPaint 就担负起了初次构建 Layer Tree 的重要任务。 

&emsp;flushPaint 函数是超级复杂的，但是呢，我们这次是初次执行，并且全局 PipelineOwner 的 `_nodesNeedingPaint` 列表中仅有我们的 Render Tree 的根节点，并且它的 `_needsPaint` 属性为 true，它的 `_layerHandle.layer` 是一个 TransformLayer 对象，并且 attached 也为 true，所以下面我们以此情况为开局，再看 PipelineOwner.flushPaint 的整个函数执行流程就会简单很多了。 

```dart
  void flushPaint() {
    try {
      // 当前仅有 Render Tree 根节点
      final List<RenderObject> dirtyNodes = _nodesNeedingPaint;
      _nodesNeedingPaint = <RenderObject>[];

      // Sort the dirty nodes in reverse order (deepest first).
      
      // 这里对 dirtyNodes 排序，是把 depth 大的放在前面，即子级先于父级，
      // 所以当后续遇到多个是绘制边界的 RenderObject 对象执行 flushPaint 时，
      // 是子级的 RenderObject 对象先去执行以自己为根节点的 Render 子树的 Paint。
      // 这样后序的父级 RenderObject 执行 paint 时，它就可以直接复用子级的 paint 结果了，
      // 而不用再重新沿着 child 链往下绘制了。
      
      for (final RenderObject node in dirtyNodes..sort((RenderObject a, RenderObject b) => b.depth - a.depth)) {
      
        // 当前 Render Tree 的根节点的 _needsPaint 值为 true，_needsCompositedLayerUpdate 值为 false。
        if ((node._needsPaint || node._needsCompositedLayerUpdate) && node.owner == this) {
          
          // 当前 Render Tree 的根节点的 _layerHandle.layer 是一个 TransformLayer 对象，并且 attached 为 true。 
          if (node._layerHandle.layer!.attached) {
            
            if (node._needsPaint) {
            
              // ⬇️ 初始情况执行到这里，入参 node 是当前 Render Tree 的根节点，它的 _needsPaint 值为 true。
              PaintingContext.repaintCompositedChild(node);
              // ⬆️
              
            } else {
              
              // 当前 node 的 _needsCompositedLayerUpdate 为 true 时，执行到这里，
              // 这里针对的则是 层更新 的情况，后续我们会再研究。
              PaintingContext.updateLayerProperties(node);
            }
          } else {
          
            // 当前 node 的 layer 已经被分离，后续我们学习 Layer 时会再研究。 
            node._skippedPaintingOnLayer();
          }
        }
      }
      
      // 对 _children 列表中的 PipelineOwner 也调用 flushPaint，
      // 在前面 PipelineOwner 篇已经分析过了，可以翻回去看看。
      for (final PipelineOwner child in _children) {
        child.flushPaint();
      }
      
    } finally {

    }
  }
```

&emsp;所以针对我们当前的情况， 我们只关注 PaintingContext.repaintCompositedChild(node) 函数调用即可，它是以我们的 Render Tree 的根节点为参数调用 PaintingContext 类的静态函数 repaintCompositedChild。下面我们看一下它的内容。

## PaintingContext.repaintCompositedChild

&emsp;repaintCompositedChild 是 PaintingContext 的静态函数，内部仅仅是对 `_repaintCompositedChild` 的封装调用，添加了一个默认为 false 的 debugAlsoPaintedParent 参数。

```dart
  // PaintingContext 的 repaintCompositedChild 函数：
  
  static void repaintCompositedChild(RenderObject child, { bool debugAlsoPaintedParent = false }) {
    _repaintCompositedChild(
      child,
      debugAlsoPaintedParent: debugAlsoPaintedParent,
    );
  }
```

## PaintingContext._repaintCompositedChild

&emsp;继续以当前 Render Tree 根节点作为入参 child。下面我们直接根据当前 Render Tree 根节点各属性的情况来分析一下 `_repaintCompositedChild` 函数的执行情况。

```dart
  // PaintingContext 的 _repaintCompositedChild 函数：
  
  static void _repaintCompositedChild(
    RenderObject child, {
    bool debugAlsoPaintedParent = false,
    PaintingContext? childContext,
  }) {
    
    // childLayer 有值，TransformLayer 是 OffsetLayer 的直接子类，
    // 所以根节点的 _layerHandle.layer 转换为 OffsetLayer 是没有任何问题的。
    OffsetLayer? childLayer = child._layerHandle.layer as OffsetLayer?;
    
    if (childLayer == null) {
      // 这里 childLayer 如果是 null 的情况，
      // 在 RenderObject 的 updateCompositedLayer 函数的默认实现中，仅仅是返回 oldLayer 或者返回一个新建的 OffsetLayer 对象，
      // 然后赋值给 child._layerHandle.layer。
      
      // 此函数在不同的 RenderObject 的子类中会进行自己的重写，RenderObject 子类对象，会根据自己的当前的情况，来更新自己的 layer。
      
      // 但是此处更重要的意义其实是为 child._layerHandle.layer 提供一个 Layer 对象，
      // 让它不致于为 null。
      
      final OffsetLayer layer = child.updateCompositedLayer(oldLayer: null);
      child._layerHandle.layer = childLayer = layer;
    } else {
      // 如果传递来的 RenderObject child 参数已经有 Layer 了，则对其进行一个清理。
      // 从 childLayer 子列表中删除该 Layer 的所有子 Layer。
      childLayer.removeAllChildren();
    }
    
    // 需要合成层更新的标识 置为 false，来表示已经更新完成了。
    child._needsCompositedLayerUpdate = false;
    
    // 刚刚入参 PaintingContext? childContext 为 null，所以这里会创建一个 PaintingContext 对象。
    // childLayer 是 Layer Tree 的根节点的 layer。
    childContext ??= PaintingContext(childLayer, child.paintBounds);
    
    // 以上面👆新建的 PaintingContext 为参数，调用 RenderObject._paintWithContext 函数。
    
    // ⚠️⚠️：请注意这里，会引发一个递归♻️调用，目前第一次调用时，
    // child 参数是我们的 Render Tree 的根节点，后续会沿着它的 child 链，
    // 依次用它的子级 RenderObject 对象调用到这里。
    child._paintWithContext(childContext, Offset.zero);

    // 停止当前的绘制。
    childContext.stopRecordingIfNeeded();
  }
```

&emsp;下面我们继续看 RenderObject._paintWithContext 函数调用，到这里就要离开 PaintingContext 类，往 RenderObject 中看了。

## RenderObject._paintWithContext

&emsp;继续下一个函数：RenderObject._paintWithContext，是的，这里又来到了 RenderObject 的主场，上面的 PaintingContext 类中的函数调用可以看出其目的还是很明确的：如果 RenderObject 的 `_layerHandle.layer` 属性为 null，则为它提供一个初始的 Layer，然后再创建一个 PaintingContext 对象，为接下来的 RenderObject 的绘制做好准备。

&emsp;上面从 PipelineOwner.flushPaint 一路执行到 RenderObject._paintWithContext 这里，看似执行路线很长，实际做的内容并不多，针对入参 RenderObject child 是 Render Tree 根节点的情况，仅仅是为其准备 PaintingContext context 参数，并且此 PaintingContext 对象创建时传入的 Layer 参数是当前 Render Tree 的根节点的 layer，同时它也是我们的 Layer Tree 的根节点。 **其实看到这里我们应该恍然大悟的，是的，没错，执行到这里的话，可以认为是正式开启了构建 Layer Tree 的工作。**

&emsp;下面看下 RenderObject._paintWithContext 函数的内容，在其内部主要是 **RenderObject.paint** 的调用。

```dart
  // RenderObject 的 _paintWithContext 函数：
  
  void _paintWithContext(PaintingContext context, Offset offset) {
    
    // 如果当前 RenderObject 仍需要布局，则直接 return。
    if (_needsLayout) {
      return;
    }
    
    // 需要绘制标识 置为 false，下面要真正的执行绘制了。
    _needsPaint = false;
    
    // 需要合成层更新标识置为 false，下面要开始真正的绘制了，肯定不需要这里 Layer 还要进行更新的。
    _needsCompositedLayerUpdate = false;
    
    // 当前的 isRepaintBoundary 属性也要变为过去式了，所以直接赋值给 _wasRepaintBoundary 属性。
    _wasRepaintBoundary = isRepaintBoundary;
    
    try {
      // ⚠️⚠️ RenderObject.panint 函数调用，最最重要的 paint 函数，
      // 执行到此处时，不同的 RenderObject 子类会重写自己的 paint 函数，
      // 此时会执行到各个 RenderObject 子类中去。
       
      //（Render Tree 的根节点的 paint 函数就执行到了 RenderView 中！）
      // 不过它的内部和 performLayout 和 layout 函数的逻辑是类似的，
      // 在父级的布局函数 performLayout 中调用子级的布局函数 layout，
      // 即父级沿着 child 链往子级中递归执行布局。
      
      paint(context, offset);
      
    } catch (e, stack) {
      // 
    }
  }
```

## RenderView.paint

&emsp;RenderView 的 paint 函数内部很简单，仅仅是直接以自己的 child 和上面传递来的 offset 为参数，调用 PaintingContext.paintChild 函数。所以这里的函数调用又回到了 PaintingContext 类中去。（这个 offset 参数，表示绘制时的偏移值，一般默认情况下都是 Offset.zero 零偏移。）

```dart
  // RenderView 的 paint 函数：
  
  @override
  void paint(PaintingContext context, Offset offset) {
    if (child != null) {
      
      // 仅仅是根据自己有没有子级，然后调用 PaintingContext 的绘制子级函数。
      context.paintChild(child!, offset);
    }
  }
```

&emsp;所以 paint 的整体逻辑，还是即递归往子级中进行绘制。

## PaintingContext.paintChild

&emsp;下面我们来到了 PaintingContext 的 paintChild 函数。在 PaintingContext.paintChild 内部仅是根据入参 RenderObject child 的 isRepaintBoundary 属性来决定是继续在当前 RenderObject child 的 layer 上继续绘制呢，还是说需要新创建 Layer。即这里就是我们在别处看到的：**多个 RenderObject 会绘制在一个 layer 上，当遇到是绘制边界的 RenderObject 的话，则是给它创建新的 layer。**

```dart
  // PaintingContext 的 paintChild 函数：
  
  void paintChild(RenderObject child, Offset offset) {
    if (child.isRepaintBoundary) {
      // 如果入参 child 是绘制边界，则需要当前的 PaintingContext 停止记录，
      // 需要把当前入参 RenderObject child 的内容绘制到单独的 Layer 中去。
      
      // 这里的 stopRecordingIfNeeded 函数也是超级重要，
      // 可以说是把目前已绘制的内容固化下来，内部会调用 PictureRecorder.endRecording 函数生成 Picture，
      // 然后再开始新的绘制，新的绘制在 _compositeChild 函数内部发生。
      stopRecordingIfNeeded();
      
      // 情况 1️⃣：去为 child 构建它自己的单独的 layer。（也可以理解为为 Layer Tree 添加新的 Layer 节点。）
      _compositeChild(child, offset);
      
    } else if (child._wasRepaintBoundary) {
      // 如果 child 之前是绘制边界，但是现在不是了，
      // 则需要把它的之前持有的单独的 Layer 对象释放了，不能再持有它了，
      // 因为它需要重新进行重绘了，它的内容会被绘制到它的父级的 Layer 中去了。
      
      // 情况 2️⃣：把 child 之前持有的 layer 释放了，然后继续在父级 RenderObject 的 layer 上绘制。 
      child._layerHandle.layer = null;
      child._paintWithContext(this, offset);
    } else {
      // 看到了，由子级 RenderObject 再去调用 _paintWithContext 函数，刚刚我们是从父级 RenderObject 调用到这里来的，
      // 即要开始递归调用了。
      
      // 情况 3️⃣：继续在父级 RenderObject 的 layer 上绘制。
      child._paintWithContext(this, offset);
    }
  }
```

&emsp;paintChild 内部的情况 2️⃣ 和情况 3️⃣ 我们就不看了，它们仅仅是继续沿着子级 RenderObject 链重复走上面梳理的 RenderObject._paintWithContext 流程。

&emsp;这里我们重点看一下情况 1️⃣，它会为当前是绘制边界的 RenderObject 创建新的 Layer 进行接下来的绘制，并且会把这新建的 Layer 拼接在父级 RenderObject 的 Layer 上。

## PaintingContext._compositeChild

&emsp;创建新的 Layer 对象，并开始在新的 Layer 上进行绘制，后续的非绘制边界的子级 RenderObject 的内容也会绘制在这个 Layer 上，并且把这新的 Layer 拼接到父级 Layer 上去。

```dart
  void _compositeChild(RenderObject child, Offset offset) {
    
    // 如果当前 RenderObject 需要绘制，或者它之前不是绘制边界，但是目前是绘制边界了，这就要给它创建独立的 Layer 了。
    if (child._needsPaint || !child._wasRepaintBoundary) {
    
      // 看哦，这里又回到了，PaintingContext.repaintCompositedChild 调用，
      // 即以当前这个是绘制边界的子级 RenderObject 为新的起点来进行绘制。
      
      // 它的内部会继续沿着子级 RenderObject 链递归♻️下去。
      //（其实看到这里有点疑惑，repaintCompositedChild 是 PaintingContext 的静态函数，而这里是一个非静态函数，
      // 那么在这里调用 repaintCompositedChild 函数不是应该加 PaintingContext 前缀吗？）
      
      // 到这里也可以看出，独立的 Layer 绘制时，会使用新的 PaintingContext 对象，
      // 保证它们的绘制隔离性。
      
      repaintCompositedChild(child, debugAlsoPaintedParent: true);
    } else {
      // 此处是针对的合成层更新的情况，与我们目前追踪的初次调用 PipelineOwner.flushPaint 没有关系。  
      if (child._needsCompositedLayerUpdate) {
      
        // 内部是对 RenderObject 的 layer 进行更新。
        updateLayerProperties(child);
      }
    }
    
    // 为 offset 属性赋值。
    final OffsetLayer childOffsetLayer = child._layerHandle.layer! as OffsetLayer;
    childOffsetLayer.offset = offset;
    
    // ⚠️⚠️⚠️ 超级重要的！因为上面的 repaintCompositedChild(child, debugAlsoPaintedParent: true) 内部是递归进行的，
    // 所以当上面的递归调用出栈时，表示以入参 child 为根节点 Render 子树上的所有 RenderObject 都执行完成自己的绘制了，
    // 到这里则是已经把子级 RenderObject 的绘制结果保存在 childOffsetLayer 中了，
    // 然后这里可以理解是把当前这个 child 的绘制结果 Layer 拼接到它的父级 Layer 中，
    // 以此进行的便是 Layer Tree 的 Layer 节点的拼接工作，或者是说 Layer Tree 的拼接工作。
    
    appendLayer(childOffsetLayer);
  }
```

## PaintingContext.appendLayer

&emsp;因为目前我们还没有展开 Layer 的学习，所以这里大概理解 appendLayer 函数做的事情即可。它就是把入参 Layer layer 往自己的父级 Layer 中进行拼接，可以理解为是用来完成 Layer Tree 构建过程中 Layer 节点之间的拼接工作的。

```dart
  @protected
  void appendLayer(Layer layer) {
    layer.remove();
    
    _containerLayer.append(layer);
  }
```

## First PipelineOwner.flushPaint 总结

&emsp;由于是初次调用 PipelineOwner.flushPaint 函数，所以整个执行流程看下来，可以理解为是以 Render Tree 的根节点为起点的 Layer Tree 的构建，Layer Tree 的根节点就是 Render Tree 的根节点的 layer。然后沿着 Render Tree 的链路往下进行，遇到非绘制边界的 RenderObject 对象的话，就把它的内容绘制在父级 RenderObject 的 layer 上，当遇到是绘制边界的 RenderObject 对象时，先把之前的绘制结果保存起来，然后为此是绘制边界的 RenderObject 对象创建新的 Layer 对象并创建新的 PaintingContext 对象开始以它为起点进行接下来的绘制，就这样依次沿着 RenderObject child 链往子级中重复进行，并会把每个新建的 Layer 拼接在一起。组成一颗完整的 Layer Tree。

&emsp;每遇到一个是绘制边界 RenderObject 对象就创建新的 Layer 节点开始新的绘制，并把接下来连续的非绘制边界的子级 RenderObject 都绘制到这一个 Layer 上。

1. PaintingContext.repaintCompositedChild -> 
2. PaintingContext._repaintCompositedChild：阶段绘制的起点，为当前 RenderObject 准备好 Layer 和 PaintingContext 对象，开始绘制。 -> 
3. RenderObject._paintWithContext：内部调用 paint，真正执行不同的 RenderObject 子类的绘制 ->
4. (RenderView 等不同的 RenderObject 子类分别会有自己的 paint).paint：叶子节点的话只绘制自己的内容即可，如果有子级的话，会通过调用 PaintingContext.paintChild 继续绘制自己的子级 ->
5. PaintingContext.paintChild ->
6. RenderObject._paintWithContext / PaintingContext._compositeChild ->
7. RenderObject._paintWithContext 则以当前子级 RenderObject 为起点重复调用 3。
8. PaintingContext._compositeChild 则以当前子级 RenderObject 为起点重复调用 1。

## 参考链接
**参考链接:🔗**
+ [RenderObject class](https://api.flutter.dev/flutter/rendering/RenderObject-class.html)
+ [RenderObjects?! | Decoding Flutter](https://www.youtube.com/watch?v=zmbmrw07qBc)
+ [PipelineOwner class](https://api.flutter.dev/flutter/rendering/PipelineOwner-class.html)
+ [RendererBinding mixin](https://api.flutter.dev/flutter/rendering/RendererBinding-mixin.html)
+ [PipelineManifold class](https://api.flutter.dev/flutter/rendering/PipelineManifold-class.html)
+ [RenderTreeRootElement class](https://api.flutter.dev/flutter/widgets/RenderTreeRootElement-class.html)
+ [RenderObjectElement class](https://api.flutter.dev/flutter/widgets/RenderObjectElement-class.html)
+ [View class](https://api.flutter.dev/flutter/widgets/View-class.html)
+ [RootWidget class](https://api.flutter.dev/flutter/widgets/RootWidget-class.html)
+ [RootElement](https://api.flutter.dev/flutter/widgets/RootElement-class.html)
