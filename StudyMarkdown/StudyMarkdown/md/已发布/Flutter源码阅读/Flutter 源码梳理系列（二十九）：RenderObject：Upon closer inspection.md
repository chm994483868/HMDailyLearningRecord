# Flutter 源码梳理系列（二十九）：RenderObject：Upon closer inspection

&emsp;在正式看 RenderObject 的 PAINTING 部分之前，我们还是要先梳理一下相关的知识点，要不然见到 PAINTING 部分的一众函数时我们很容易会迷糊。

&emsp;通过前文的学习我们已经知道 Widget Tree 和 Element Tree 是从根节点开始一层一层同步构建的，Widget 和 Element 一一对应，然后呢如果 一个 Element 对象是 RenderObjectElement 的话，它会在自己挂载到父 Element 后，创建一个 RenderObject 对象并把它附加到 Render Tree 上去，即随着 Element Tree 的构建，当遇到 RenderObjectElement 时也同步扩建 Render Tree，不同于所有的 Element 节点都会参与 Element Tree 的构建，仅有遇到 RenderObjectElement 时才会创建 RenderObject 对象并让它参与 Render Tree 的构建。

# 再探 adoptChild
 
&emsp;那么我们继续从 RenderObjectElement.mount 挂载函数开始看起。已知 RenderObjectElement 创建了自己的 RenderObject 后会把它附加到 Render Tree 中，而附加任务的末尾必定是执行 RenderObject.adoptChild 函数。在 adoptChild 内部呢则是我们熟悉的 mark 系列函数：

```dart
  @mustCallSuper
  @protected
  void adoptChild(RenderObject child) {
    setupParentData(child);
    
    // mark 系列函数
    markNeedsLayout();
    markNeedsCompositingBitsUpdate();
    markNeedsSemanticsUpdate();
    
    // 这里👇则是把入参 RenderObject child 加入到当前的 Render Tree 中。
    child._parent = this;
    if (attached) {
      child.attach(_owner!);
    }
    redepthChild(child);
  }
```

&emsp;看到 Mark 函数就意味着本帧内是不处理 mark 对应的标记事件的，也就是意味着 Layout、CompositingBitsUpdate 和 SemanticsUpdate 这些事件都是在当前帧内不处理的。这也就是意味着在 Element Tree 和 Render Tree 构建完成以后，才会统一对被标记了 Layout、CompositingBitsUpdate 和 SemanticsUpdate 的 RenderObject 执行布局、合成位更新 和 语义化更新事件。

&emsp;然后我们要把目光聚集在 markNeedsLayout 和 markNeedsCompositingBitsUpdate 两个函数内部，在它们两个 mark 函数内部看到：只有分别满足自己是 "重新布局边界" 和 "重新绘制边界" 的 RenderObject 才能被加入到 PipelineOwner 的 `_nodesNeedingXXX` 列表中。并在末尾请求视觉更新，以待在新的一帧中处理被收集到 `_nodesNeedingXXX` 列表的 RenderObject 对象。

&emsp;其实这也是可以理解的，以我们前面学习 RenderObject LAYOUT 部分为例，我们知道当父级需要子级的 Size 时和子级不能自己决定自己的 Size 时，那么父级会作为子级的 "重新布局边界"，这样可以保证当子级需要布局更新时会直接通知到父级进行布局更新，然后子级会顺带被父级更新布局。所以直白一点的理解 "重新布局边界" 的话，可以把它理解为一组彼此有布局依赖的 RenderObject 对象。通过把它们绑定在一个布局边界中，然后当其中有某个 RenderObject 需要更新布局时，就让边界进行更新，那么此一组自边界起的 RenderObject 对象都可以得到布局更新，这样就保证了这一组彼此有依赖的 RenderObject 能保持布局的同步性，所以仅有是 **边界** 的 RenderObject 对象可以加入到 PipelineOwner 的 `_nodesNeedingXXX` 列表中也就比较能理解了。

&emsp;另外关于仅有是 **重新绘制边界** 的 RenderObject 对象才能加入到 PipelineOwner 的 `_nodesNeedingXXX` 列表中的也是类似的逻辑。

&emsp;关于 markNeedsCompositingBitsUpdate 的话，因为它是与 Paint 紧密相连的，但是我们还没有学习 Paint 部分，所以这里我们先拓展一下。markNeedsCompositingBitsUpdate 所做的事情是标记需要对 RenderObject 的合成位进行更新，而合成位是谁呢？其实就是指的 RenderObject 的 `late bool _needsCompositing` 的属性，这个属性特别重要，后续我们学习 Layer 时会对它进行详细的解读，当前的话，我们先对它有个粗略的了解。

# 拓展 `_needsCompositing`

&emsp;`_needsCompositing` 是一个 RenderObject 的标识位，用于表示当前的 RenderObject 是否需要进行合成操作（compositing）。当 RenderObject 拥有子级并且子级拥有透明度、变换、剪裁等属性时，父级就需要进行合成。在 Flutter 中，合成操作会将多个透明、旋转、裁剪等操作合并成一个单一的纹理，以提高渲染性能。从这里我们就可以看到一些细节了，并且是对比 "重新布局边界" 的细节。

&emsp;关于 "重新布局边界" 我们知道它是由父级来决定子级的。而 "是否需要进行合成操作（即 `_needsCompositing` 属性值是 true 还是 false）" 则是由子级决定父级的，在代码中实际的执行方式就是父级去访问子级的 `_needsCompositing` 来决定自己的 `_needsCompositing` 属性。前面提到的 Render Tree 构建完成以后才会处理这些 **布局/合成位更新/绘制/语义化更新** 相关的事情，而这个提前构建完成的 Render Tree 就是父级 RenderObject 得以去访问自己的子级的 `_needsCompositing` 的值提供了基础。

&emsp;如果 `_needsCompositing` 标识为 true，Flutter 将会为该 RenderObject 创建一个层（Layer），并通过图层合成（layer composition）方式来渲染当前节点及其子级节点，减少绘制和布局的开销，提高性能。如下伪代码：

```dart
@override
void performLayout() {
  super.performLayout();

  // ⚠️⚠️⚠️ 注意，一般都是在 RenderObject._updateCompositingBits 函数内，
  // 决定 _needsCompositing 的值是 true 还是 false。然后它有两种情况会被置为 true：
  // 1. 如果当前 RenderObject 的直接子级的 _needsCompositing 为 true。
  // 2. 如果当前 RenderObject 是 重新绘制边界 或者它的 alwaysNeedsCompositing 属性为 true。
  // 在这两种情况下，此 RenderObject 的 _needsCompositing 会被置为 true。
  
  // ⬇️ 这里的伪代码，可以帮助我们理解 _needsCompositing 何时为 true。
  
  // 当子级节点包含透明度或者旋转或者被裁剪等属性时，将 _needsCompositing 置为 true
  _needsCompositing = hasTransparency || hasTransform || hasClip;

  // 进行自身布局操作
  // ...
}

@override
void paint(PaintingContext context, Offset offset) {

  // ⬇️（这里的知识点就是我们以前别的地方见到过的，多个 RenderObject 会绘制到一个 Layer 上。）
  
  // 如果 _needsCompositing 为 true，创建一个层（Layer），否则直接绘制。
  if (_needsCompositing) {
    context.pushLayer(MyRenderLayer(offset: offset, children: children));
  } else {
  
    // 直接绘制当前节点
    super.paint(context, offset);
  }
}
```

&emsp;需要注意的是，尽量减少使用合成操作，因为创建和维护层（Layer）会带来额外的开销。只有在必要时才设置 `_needsCompositing` 标识为 true。

&emsp;OK，我们下面继续以 Element Tree 和 Render Tree 初次构建为线索，分析其中涉及到的 Layout、CompositingBitsUpdate、Paint 事件。

# 猜想三阶段

&emsp;我们再次回顾一下上面提到的 RenderObject 节点附加到 Render Tree 所使用的 adoptChild 函数：

&emsp;adoptChild 函数提示我们的最最重要的大概是：当一个新的 RenderObject 对象被附加到 Render Tree 后都需要被标记为需要 Layout、CompositingBitsUpdate，然后呢，在新的一帧内，我们在真正执行 Layout 和 CompositingBitsUpdate 操作后，才会标记 RenderObject 需要 Paint。所以大概可以理解为：当前帧进行 Element Tree 和 Render Tree 的构建，然后 Tree 构建完成后，才会在新的帧回调中对构建 Render Tree 过程中收集到的是边界的 RenderObject 进行布局和把它的合成位：`_needsCompositing` 置为 true 或 false，而在这个过程中会调用 RenderObject.markNeedsPaint，同样在 markNeedsPaint 函数内部也是只对是重新绘制边界的 RenerObjedt 才会被收集在 PipelineOwner 的 `_nodesNeedingXXX` 列表中，然后等待进入新的一帧后才会对这些被收集的 RenderObject 进行绘制。

&emsp;所以可以把以上内容简单拆分为以下阶段：

1. 阶段 1️⃣：Element Tree 和 Render Tree 一层一层构建，并在 Render Tree 构建过程中把新创建的 RenderObject 或者是它的父级加入到 PipelineOwner 的 `_nodesNeedingLayout` 需要进行布局的 RenderObject 列表中，以及加入到 PipelineOwner 的 `_nodesNeedingCompositingBitsUpdate` 需要进行合成位更新的 RenderObject 列表中。

2. 阶段 2️⃣：在新的一帧中对上面 PipelineOwner 收集的 RenderObject 对象进行 Layout 和 CompositingBitsUpdate，而在这两个事件内部根据具体的情况，都有可能调用到 markNeedsPaint，即当前 RenderObject 已经完成了布局和合成位更新了，那么需要进行布局了。同样是 mark 事件，也预示了 Paint 是在新的一帧中进行的。markNeedsPaint 则是把是重新绘制边界的 RenderObject 收集在 PipelineOwner 的 `_nodesNeedingPaint` 列表中。

3. 阶段 3️⃣：PipelineOwner 的 `_nodesNeedingPaint` 列表中的 RenderObject 进行 Paint。

&emsp;下面我们就尝试一下证明一下是不是这样的过程。首先是 RenderObject 的如下属性和 getter 的默认值我们需要关注一下：（当然在 RenderObject 子类中这些默认属性和 getter 都有可能会重写。）

1. Layout 相关，默认情况下新建的 RenderObject 对象都是需要 Layout 布局的，并且它们的重新布局边界都是 null。

+ `bool _needsLayout = true;` ⬅️🔔
+ `RenderObject? _relayoutBoundary;`

2. CompositingBitsUpdate 相关，默认情况下新建的 RenderObject 对象的合成位是不需要更新的，并且它们都不是重新绘制边界。

+ `bool _needsCompositingBitsUpdate = false;` ⬅️🔔
+ `bool get isRepaintBoundary => false;` ⬅️🔔
+ `bool get alwaysNeedsCompositing => false;` // 如果此值为 true，则 `_needsCompositing` 恒定也为 true。

+ `late bool _wasRepaintBoundary;` // 在 RenderObject 的构造函数中：`_wasRepaintBoundary = isRepaintBoundary;`，由于 isRepaintBoundary 默认是 false，所以 `_wasRepaintBoundary` 初始值也是 false。

+ `late bool _needsCompositing;` // 在 RenderObject 的构造函数中：`_needsCompositing = isRepaintBoundary || alwaysNeedsCompositing;`，由于 isRepaintBoundary 和 alwaysNeedsCompositing 默认都是 false，所以 `_needsCompositing` 初始值也是 false。

3. Paint 相关，默认情况下新建的 RenderObject 对象都是需要 Paint 绘制的。

+ `bool _needsPaint = true;` ⬅️🔔

&emsp;上面的属性和 getter 有点多，我们一次记住的话有点困难，不过我们可以反向来记忆，即除了 `_needsLayout` 和 `_needsPaint` 为 true 外，其它的都是 false。即新建的 RenderObject 需要进行 Layout 布局和 Paint 绘制，然后其它的都是 false。

## 强化 Mark 系列函数

&emsp;然后再次一行一行捋一下 RenderObject.markNeedsLayout、RenderObject.markNeedsCompositingBitsUpdate、RenderObject.markNeedsPaint 三个函数：

+ markNeedsLayout：

```dart
  void markNeedsLayout() {
    // 默认新建的 RenderObject 的 _needsLayout 都是 true，
    // 所以新建的 RenderObject 对象附加到 Render Tree 时，
    // 调用 markNeedsLayout 函数到这里会直接 return。
    if (_needsLayout) {
      return;
    }
    
    // 除了新建的 RenderObject 的 _relayoutBoundary 为 null，
    // 其它则是在父级的 _relayoutBoundary 要发生变化时，
    // 它会把自己下面的子级中和自己是同一 _relayoutBoundary 的那部分子级的 _relayoutBoundary 属性置为 null，
    // 这一部分逻辑在 RenderObject.layout 函数内部，可以翻回前面的文章再仔细看看。
    
    // 如果一个 RenderObject 对象的 _relayoutBoundary 为 null，
    // 则继续往其父级中去找 "重新布局边界"。
    if (_relayoutBoundary == null) {
      // 把自己标记为需要布局
      _needsLayout = true;
      
      // 然后去父级中找重新布局边界，去标记它需要布局，只有这样才能保证同一边界中的一组 RenderObject 的布局都得到更新。
      if (parent != null) {
        markParentNeedsLayout();
      }
      
      return;
    }
    
    // 如果一个 RenderObject 对象的 _relayoutBoundary 属性不是自己，那必定是它的父级，
    // 所以同样去往它父级中去找重新布局边界。
    if (_relayoutBoundary != this) {
      markParentNeedsLayout();
    } else {
      // 最后一种情况了：RenderObject 对象的 _relayoutBoundary 属性是自己，
      // 然后同样把自己标记为需要布局，当真正进行了布局以后 _needsLayout 才会置回 false。
      _needsLayout = true;
      
      // 然后把自己加入到 PipelineOwner 的 _nodesNeedingLayout 列表中，
      // 并请求进行视觉更新，在新的一帧回调中，统一对 _nodesNeedingLayout 列表中的 RenderObject 对象进行重新布局。 
      if (owner != null) {
        owner!._nodesNeedingLayout.add(this);
        owner!.requestVisualUpdate();
      }
    }
  }
```

+ markNeedsCompositingBitsUpdate：

```dart
  void markNeedsCompositingBitsUpdate() {
    // 默认新建的 RenderObject 的 _needsCompositingBitsUpdate 都是 false，
    // 所以新建的 RenderObject 对象附加到 Render Tree 时，
    // 调用 markNeedsCompositingBitsUpdate 函数到这里并不会直接 return。
    if (_needsCompositingBitsUpdate) {
      return;
    }
    
    // 然后直接把 _needsCompositingBitsUpdate 由 false 置为 true
    _needsCompositingBitsUpdate = true;
    
    // 新建的 RenerObject 对象，在首次执行 markNeedsCompositingBitsUpdate 函数前，
    // 还没有附加到父级上，所以这里 parent 是 null，并不会进入此 if 内部。
    if (parent is RenderObject) {
    
      // 当后续 RenderObject 执行 markNeedsCompositingBitsUpdate 时，
      // 如果父级已经 _needsCompositingBitsUpdate 为 true 了，
      // 自己则直接 return 即可。
      final RenderObject parent = this.parent!;
      if (parent._needsCompositingBitsUpdate) {
        return;
      }
      
      // 如果自己之前或者现在不是重新绘制边界，并且父级也不是重新绘制边界，则继续往父级中去执行 markNeedsCompositingBitsUpdate。
      if ((!_wasRepaintBoundary || !isRepaintBoundary) && !parent.isRepaintBoundary) {
        parent.markNeedsCompositingBitsUpdate();
        
        return;
      }
    }
    
    // 新建的 RenerObject 对象，在首次执行 markNeedsCompositingBitsUpdate 函数前，
    // 还没有附加到父级上，所以这里 owner 是 null，并不会进入此 if 内部。
    // 但是根 RenderObject 对象除外。
    if (owner != null) {
      owner!._nodesNeedingCompositingBitsUpdate.add(this);
    }
  }
```

+ markNeedsPaint：

```dart
  void markNeedsPaint() {
    // 默认新建的 RenderObject 的 _needsPaint 都是 true，
    // 所以新建的 RenderObject 对象附加到 Render Tree 时，
    // 调用 markNeedsPaint 函数到这里会直接 return。
    // ⚠️ 但是请注意，新建的 RenderObject 对象并不会调用 markNeedsPaint。
    
    if (_needsPaint) {
      return;
    }
    
    // 把需要绘制位置为 true
    _needsPaint = true;
        
    // 这里的原注释：如果这之前不是一个重绘边界，它将没有一个我们可以绘制的 Layer。(我们知道的，当一个 RenderObject 是重新绘制边界的话，它会有自己独立的 Layer 的。)
    
    if (isRepaintBoundary && _wasRepaintBoundary) {
      
      // 这里的原注释：如果我们总是有自己的图层，那么我们可以只重新绘制自己，而不涉及任何其他节点。
      
      if (owner != null) {
        // 自己就是重绘边界的话，即可以把自己往 _nodesNeedingPaint 添加了，等待新的一帧进行绘制。
        owner!._nodesNeedingPaint.add(this);
        owner!.requestVisualUpdate();
      }
      
    } else if (parent != null) {
    
      // 自己不是重绘边界的话，则继续往父级中传递需要重新绘制。
      parent!.markNeedsPaint();
      
    } else {
    
      // 这里的原注释：如果我们是渲染树的根，而不是重绘边界，那么我们必须自己绘制，因为没有人可以绘制我们。
      // 在这种情况下，我们没有将自己添加到 _nodesNeedingPaint 中，因为根节点总是被告知无论如何都要绘制。
      // 基于 RenderView 的树不会经过这个代码路径，因为 RenderView 是重绘边界。
      
      owner?.requestVisualUpdate();
    }
  }
```

&emsp;综上 markNeedsLayout 和 markNeedsCompositingBitsUpdate 两个函数，可以发现，新建的 RenderObject 对象是不可能被 PipelineOwner 收集起来的，但是有一个要除外，那就是 Render Tree 的根节点！

# 由 Render Tree 根节点引出 Layer Tree 根节点 

&emsp;这里我们直接在 RendererBinding.drawFrame 函数处打一个断点，已知的 `owner?.requestVisualUpdate();` 请求视觉更新的回调就是这里。

```dart
  void drawFrame() {
  
    // 这里 RendererBinding.instance.rootPipelineOwner 分别调用自己的三个 flush 函数，
    // 即刷新 mark 系列函数收集的 RenderObject 对象。
    rootPipelineOwner.flushLayout();
    rootPipelineOwner.flushCompositingBits();
    rootPipelineOwner.flushPaint();
    
    if (sendFramesToEngine) {
      for (final RenderView renderView in renderViews) {
        renderView.compositeFrame(); // this sends the bits to the GPU
      }
      
      rootPipelineOwner.flushSemantics(); // this sends the semantics to the OS.
      _firstFrameSent = true;
    }
  }
```

&emsp;然后我们运行项目，会直接命中此断点，此时 Element Tree 和 Render Tree 已经构建完成了，如果有耐心的话可以点开 `RendererBinding.instance._rootElement` 的 child 指针看下有多少级 Element，已知的这个 `RendererBinding.instance._rootElement` 就是 Element Tree 的根节点。

&emsp;如果有耐心的话可以点开 `RendererBinding.instance.renderView` 的 child 指针看下有多少级 RenderObject，已知的这个 `RendererBinding.instance.renderView` 就是 Render Tree 的根节点。

&emsp;可以看到在 RendererBinding.instance.rootPipelineOwner 的 children set 内仅有一个 PipelineOwner，其实它正是 RendererBinding.instance.pipelineOwner 属性，它呢就是 Render Tree 中每个 RenderObject 节点的 owner 的指向。我们的 RenderObject.markNeedsLayout、RenderObject.markNeedsCompositingBitsUpdate、RenderObject.markNeedsPaint 三个 mark 函数内部的往 `_nodesNeedingXXX` 列表中添加 RenderObject 对象：

+ `owner!._nodesNeedingLayout.add(this);`
+ `owner!._nodesNeedingCompositingBitsUpdate.add(this);`
+ `owner!._nodesNeedingPaint.add(this);`

&emsp;其中的 owner 就是 RendererBinding.instance.pipelineOwner 属性，我们直接点开看它的三个 `_nodesNeedingXXX` 列表，可看到即使是首次命中 RendererBinding.drawFrame，在此三个列表中也都会仅有一个 RenderObject 对象，而且都是我们的 Render Tree 根节点：RendererBinding.instance.renderView。

&emsp;其实据我们上面的分析在 Render Tree 初次构建时，所有的新建 RenderObject 对象是无法加入 `_nodesNeedingLayout` 和 `_nodesNeedingPaint` 的列表的，此时我们可以直接全局搜索 `_nodesNeedingLayout.add` 关键字，看下除了 `RenderObject.markNeedsLayout` 函数内部会往 `_nodesNeedingLayout` 列表中添加 RenderObject 对象外，还有哪里会往此列表中添加对象，在搜索结果中，可看到在 RenderObject.scheduleInitialLayout 函数内部会往 `_nodesNeedingLayout` 列表中添加 RenderObject，同样的全局搜索 `_nodesNeedingPaint.add` 关键字，可看到在 RenderObject.scheduleInitialPaint 内部也会使用往 `_nodesNeedingPaint` 列表中添加 RenderObject 对象。如下函数：

+ scheduleInitialLayout:（此函数名可以直译为：调度初始化布局）

```dart
  void scheduleInitialLayout() {
    _relayoutBoundary = this;
    owner!._nodesNeedingLayout.add(this);
  }
```

&emsp;然后我们直接在 scheduleInitialLayout 函数处打一个断点，运行项目会被直接命中，且会早于 RendererBinding.drawFrame 执行。然后沿着它的函数堆栈往前看，可以看到是在 depth 是 4 的 Render Tree 的根节点 `RendererBinding.instance.renderView` 所对应的 RenderObjectElement：`_RawViewElement` 对象执行 Element Tree 挂载时，调用的 mount 函数内执行：`renderObject.prepareInitialFrame()` 调用，即发起 Render Tree 根节点准备初始帧，然后依次如下函数堆栈：

+ `_ReusableRenderView.prepareInitialFrame` ->
+ `RenderView.prepareInitialFrame` ->
+ `RenderObject.scheduleInitialLayout` 

&emsp;一路调用到了：RenderObject.scheduleInitialLayout 函数，在其内部它把 Render Tree 根节点自己的 `_relayoutBoundary` 赋值为根节点自己，并把自己加入 owner 的重新布局列表中。

&emsp;其实这里由 runApp 函数通一下 depth 是 4 的 `_RawViewElement` 节点的 renderObject 属性就是前面的 RendererBinding.instance.renderView，它是被一路传过来的，不过这部分代码我们留在后面学习完整的 Flutter App 启动流程里面再分析。

&emsp;到这里我们可以明白：Element Tree 构建到 depth 是 4 的时候遇到第一个 RenderObjectElement 节点，它会带着自己的 renderObject 属性，即 Render Tree 的根节点，执行一个首帧准备和调度初始化布局，把自己作为自己的重新布局边界，并把自己加入到全局的 PipelineOwner 的布局列表中，即完成了：**通过调度第一次布局，来引导渲染流水线的启动。**

+ scheduleInitialPaint:（此函数名可以直译为：调度初始化绘制）

```dart
  void scheduleInitialPaint(ContainerLayer rootLayer) {
    _layerHandle.layer = rootLayer;
    owner!._nodesNeedingPaint.add(this);
  }
```

&emsp;同样的我们在 scheduleInitialPaint 函数处打一个断点，可看到在上面的 `RenderObject.scheduleInitialLayout` 函数下面一行就是调用 RenderObject.scheduleInitialPaint 函数。同时最重要的点来了，scheduleInitialPaint 的完整调用是：`scheduleInitialPaint(_updateMatricesAndCreateNewRootLayer());`，而它的这个 `ContainerLayer rootLayer` 入参就是 Layer Tree 的根节点，完整创建过程如下：

```dart
  TransformLayer _updateMatricesAndCreateNewRootLayer() {
    _rootTransform = configuration.toMatrix();
    
    final TransformLayer rootLayer = TransformLayer(transform: _rootTransform);
    rootLayer.attach(this);
    return rootLayer;
  }
```

&emsp;在 RenderObject.scheduleInitialPaint 内部就是把这个 TransformLayer 对象作为 Render Tree 根节点的 layer，即 Render Tree 的根节点和 Layer Tree 的根节点连接在了一起。并把 Render Tree 根节点加入到全局的 PipelineOwner 的绘制列表中，即完成了：**通过调度第一次绘制，来引导渲染流水线的启动。** 

&emsp;看到 Layer Tree 的根节点是一个 TransformLayer 对象。所以从上面一路看到现在，终于看到最重要的 Layer Tree 的根节点了。

&emsp;由于篇幅限制，本篇我们就先看到这里吧，这里引出了 Layer Tree 的根节点。下篇我们再继续沿着 RendererBinding.drawFrame 中的三个 Flush 函数：PipelineOwner.flushLayout、PipelineOwner.flushCompositingBits、PipelineOwner.flushPaint 的执行堆栈再探 Layout 和 Paint 流程。⛽️

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
