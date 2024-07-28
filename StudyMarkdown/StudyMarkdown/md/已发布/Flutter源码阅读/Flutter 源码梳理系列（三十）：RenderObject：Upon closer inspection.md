# Flutter 源码梳理系列（三十）：RenderObject：Upon closer inspection

# 前言

&emsp;继续接上篇的探索进度。本篇的重点是 RendererBinding.drawFrame 中调用的 PipelineOwner 刷新函数，不过我们重点只放在其中两个刷新函数：flushCompositingBits 和 flushPaint 上，关于 flushLayout 内容的话，前面的 RenderObject LAYOUT 部分已经详细分析过了，这里就不再重复了。

&emsp;通过上一篇的学习，我们知道当断点首次命中 RendererBinding.drawFrame 时 Widget Tree、Element Tree、Render Tree 已经构建好了，并且 Render Tree 根节点也已经被全局 PipelineOwner 收集好了，就待执行以 Render Tree 根节点为起点的 **布局、合成位更新和绘制** 工作了，即执行 PipelineOwner 的三连击 flush 函数。在之前我们学习 RenderObject LAYOUT 和 PipelineOwner 时有详细学习 flushLayout 相关的函数：

+ PipelineOwner.flushLayout、
+ RenderObject._layoutWithoutResize、
+ RenderObject.performLayout、
+ RenderObject.layout、

&emsp;这些函数中，特别是 RenderObject.performLayout 和 RenderObject.layout 函数。当父级 RenderObject 调用 performLayout 函数时，会依次调用自己子级的 layout 函数，然后子级的 layout 内部会再调用 performLayout 函数，依次不断的往子级中递归调用，直到整个 Render 子树上所有 RenderObject 节点布局完成。所以当以 Render Tree 的根节点为起点调用 `RenderObject._layoutWithoutResize` 函数时，其实就是对整颗 Render Tree 上的所有节点进行布局。

&emsp;那么当以 Render Tree 的根节点为起点时，PipelineOwner.flushCompositingBits 和 PipelineOwner.flushPaint 的执行逻辑是如何呢，下面我们一起跟着函数调用堆栈来看一下。

# flushCompositingBits

&emsp;在 Pipeline.flushCompositingBits 函数内，会首先针对收集到的 RenderObject 对象根据 depth 进行排序，且排序规则是把 depth 小的排在前面，即把父级排在子级前面去，然后从父级往子级进行遍历，调用 RenderObject._updateCompositingBits 函数。

&emsp;因为目前我们只有一个 Render Tree 的根节点，所以我们可以完全忽略 flushCompositingBits 函数中其它部分的内部，只要把目光聚集在 `node._updateCompositingBits();` 行即可。即当 Render Tree 根节点调用 `_updateCompositingBits` 函数时内部都会做什么呢？看起来是比 Layout 要复杂的，首先我们要明白一点 `_updateCompositingBits` 函数的目的是什么？它是为了更新 Render Tree 上每一个 RenderObject 节点的 `_needsCompositing` 属性值的，说是更新吧也不对，并不是更新，是计算 `_needsCompositing` 属性值的，`_needsCompositing` 是一个 bool 值，所以 RenderObject 节点的 `_needsCompositing` 属性值它不是 true 就是 fasle，那么谁又能决定此值是 true 还是 false 呢？由子级决定父级的！ 

&emsp;`_updateCompositingBits` 函数是一个递归函数，父级 RenderObject 会向子级 RenderObject 中调用直到叶子节点，并且会把子级的 `_needsCompositing` 为 true 时，传递给父级，父级也需要把 `_needsCompositing` 置为 true，即子级传递给父级。

&emsp;然后还有一种情况，当 RenderObject 的 isRepaintBoundary 属性为 true，或者 alwaysNeedsCompositing 属性为 true，那么 `_needsCompositing` 也要赋值为 true，但是 RenderObject 的这两个属性默认都是 false，子类可能回重写它们。

```dart
void _updateCompositingBits() {
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
    // RenderObject 的这两个属性默认都是 false，子类可能回重写它们。
    if (isRepaintBoundary || alwaysNeedsCompositing) {
      _needsCompositing = true;
    }
    
    // 如果一个节点之前是一个重绘边界，但现在不再是，那么无论其合成状态如何，我们都需要找到一个新的父节点来绘制。
    // 为了做到这一点，我们再次标记为干净状态，以便在 markNeedsPaint 中的遍历不被提前终止。
    // 它会从 _nodesNeedingPaint 中移除，这样我们在找到父节点之后就不会尝试从它绘制。

    if (!isRepaintBoundary && _wasRepaintBoundary) {
      // 这里针对的情况是一个 RenderObject 节点之前是一个重新绘制边界，
      // 但是现在不是了，针对这种转变，肯定需要进行重新绘制。
      
      // 这里先把它的绘制状态置为 false 并把它从绘制列表中移除，
      // 然后再调用 markNeedsPaint 把它标记为需要绘制。
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
      // 则需要把 _needsCompositingBitsUpdate 标识位置为 false，说明合成位更新已经完毕了。
      _needsCompositingBitsUpdate = false;
    }
  }
```

# flushPaint

&emsp;flushPaint 函数是超级复杂的，但是呢，我们是初次执行 RendererBinding.drawFrame，并且全局 PipelineOwner 的 `_nodesNeedingPaint` 列表，仅有我们的 Render Tree 的根节点，并且它的 `_needsPaint` 为 true，它的 `_layerHandle.layer` 是一个 TransformLayer 对象，并且 attached 为 true。 

```dart
  void flushPaint() {
    try {
      // 当前仅有 Render Tree 根节点
      final List<RenderObject> dirtyNodes = _nodesNeedingPaint;
      _nodesNeedingPaint = <RenderObject>[];

      // Sort the dirty nodes in reverse order (deepest first).
      
      // 这里对 dirtyNodes 排序的话，是把 depth 大的放在前面，即子级先于父级。
      for (final RenderObject node in dirtyNodes..sort((RenderObject a, RenderObject b) => b.depth - a.depth)) {
      
        // 当前 Render Tree 的根节点的 _needsPaint 值为 true。
        if ((node._needsPaint || node._needsCompositedLayerUpdate) && node.owner == this) {
          
          // 当前 Render Tree 的根节点的 _layerHandle.layer 是一个 TransformLayer 对象，
          // 并且 attached 为 true。 
          if (node._layerHandle.layer!.attached) {
            
            if (node._needsPaint) {
            
              // ⬇️ 默认执行到这里，入参 node 是当前 Render Tree 的根节点。
              PaintingContext.repaintCompositedChild(node);
              // ⬆️
              
            } else {
              PaintingContext.updateLayerProperties(node);
            }
            
          } else {
            node._skippedPaintingOnLayer();
          }
        }
      }
      
      for (final PipelineOwner child in _children) {
        child.flushPaint();
      }
      
    } finally {

    }
  }
```

&emsp;repaintCompositedChild 是 PaintingContext 的静态函数，内部仅仅是对 `_repaintCompositedChild` 的封装调用。

```dart
  // PaintingContext 的 repaintCompositedChild 函数：
  
  static void repaintCompositedChild(RenderObject child, { bool debugAlsoPaintedParent = false }) {
    _repaintCompositedChild(
      child,
      debugAlsoPaintedParent: debugAlsoPaintedParent,
    );
  }
```

&emsp;根据当前 Render Tree 根节点的情况以及作为入参 RenderObject child，我们继续看 `_repaintCompositedChild` 函数的内容。

```dart
  // PaintingContext 的 _repaintCompositedChild 函数：
  
  static void _repaintCompositedChild(
    RenderObject child, {
    bool debugAlsoPaintedParent = false,
    PaintingContext? childContext,
  }) {
    
    // 有值，TransformLayer 是 OffsetLayer 的直接子类，所以 根节点的 _layerHandle.layer 转换为 OffsetLayer，
    // 没有任何问题。
    OffsetLayer? childLayer = child._layerHandle.layer as OffsetLayer?;
    
    if (childLayer == null) {
    
      // 这里是如果 无值 的情况，则返回一个 OffsetLayer() 对象，赋值给 child._layerHandle.layer。
      final OffsetLayer layer = child.updateCompositedLayer(oldLayer: null);
      child._layerHandle.layer = childLayer = layer;
      
    } else {
    
      // 从 childLayer 子列表中删除该 Layer 的所有子 Layer。
      childLayer.removeAllChildren();
      
    }
    
    // 需要合成层更新标识置为 false。
    child._needsCompositedLayerUpdate = false;
    
    // 刚刚入参 PaintingContext? childContext 为 null，
    // 所以这里会创建一个 PaintingContext 对象。
    // 入参 childLayer 即 Layer Tree 的根节点。
    childContext ??= PaintingContext(childLayer, child.paintBounds);
    
    // 直接调用 RenderObject._paintWithContext 函数。
    // ⚠️⚠️：请注意这里，会引发一个递归调用，目前第一次调用时，
    // child 参数是我们的 Render Tree 的根节点，它会一路调用下去。
    child._paintWithContext(childContext, Offset.zero);

    // 
    childContext.stopRecordingIfNeeded();
  }
```

&emsp;继续下一个函数：`RenderObject._paintWithContext`，这里又来到了 RenderObject 的主场，在 `RenderObject._paintWithContext` 内部是 RenderObject.paint 的调用。

&emsp;上面从 PipelineOwner.flushPaint 一路执行到 RenderObject._paintWithContext 这里，看似执行路线很长，实际做的内容并不多，仅仅是为了准备 PaintingContext context 参数，此 PaintingContext context 参数创建时传入的 Layer 参数是当前 Render Tree 的根节点的 layer，同时它也是我们的 Layer Tree 的根节点。

```dart
  // RenderObject 的 _paintWithContext 函数：
  
  void _paintWithContext(PaintingContext context, Offset offset) {
    
    // 如果当前 RenderObject 仍需要布局，则直接 return。
    if (_needsLayout) {
      return;
    }
    
    // 需要绘制标识置为 false
    _needsPaint = false;
    
    // 需要合成层更新标识置为 false
    _needsCompositedLayerUpdate = false;
    
    // 当前的 isRepaintBoundary 属性也要变为过去式了，
    // 所以直接赋值给 _wasRepaintBoundary 属性。
    _wasRepaintBoundary = isRepaintBoundary;
    
    try {
      
      // ⚠️⚠️ RenderObject.panint 函数调用，最最重要的 paint 函数，
      // 入参 context 是经过上面的调用准备好的 PaintingContext。
      paint(context, offset);
      
    } catch (e, stack) {
      
    }
  }
```

&emsp;

```dart
  // RenderView 的 paint 函数：
  
  @override
  void paint(PaintingContext context, Offset offset) {
    if (child != null) {
      context.paintChild(child!, offset);
    }
  }
```

&emsp;

```dart
  // PaintingContext 的 paintChild 函数：
  
  void paintChild(RenderObject child, Offset offset) {
    if (child.isRepaintBoundary) {
      // 如果 child 是绘制边界，则需要当前的 PaintingContext 停止记录，
      // 需要把当前的 RenderObject child 的内容绘制到单独的 Layer 中去。
      stopRecordingIfNeeded();
      
      // 
      _compositeChild(child, offset);
      
    } else if (child._wasRepaintBoundary) {
      // 如果 child 之前是绘制边界，但是现在不是了，
      // 则需要把它持有的 Layer 释放了，不能再持有它了，
      // 因为它需要进行重绘了，它的内容会被绘制到新的 Layer 中去了。
      child._layerHandle.layer = null;
      
      // 
      child._paintWithContext(this, offset);
      
    } else {
      
      // 看到了，由子级 RenderObject 再去调用 _paintWithContext 函数，
      // 刚刚我们是从父级 RenderObject 调用到这里的。
      child._paintWithContext(this, offset);
    }
  }
```









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
