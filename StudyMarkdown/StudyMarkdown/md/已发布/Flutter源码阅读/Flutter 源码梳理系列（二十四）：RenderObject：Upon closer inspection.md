# Flutter 源码梳理系列（二十四）：RenderObject：Upon closer inspection

&emsp;在正式看 RenderObject 的 PAINTING 部分之前，我们还是要先梳理一下相关的知识点，要不然见到 PAINTING 部分的一众函数时我们很容易会迷糊。

&emsp;通过前文的学习我们已经知道 Widget Tree 和 Element Tree 是从根节点开始一层一层同步构建的，Widget 和 Element 一一对应，然后呢如果 Element 是 RenderObjectElement 的话，它会在自己挂载到父 Element 后，创建一个 RenderObject 并把它附加到 Render Tree 上去，即随着 Element Tree 的构建，当遇到 RenderObjectElement 时也同步构建 Render Tree，不同于所有的 Element 节点都会参与 Element Tree 的构建，仅有 RenderObjectElement 才会创建 RenderObject 并让它参与 Render Tree 的构建。

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
    
    // 下面👇则是把当前 RenderObject child 加入到当前的 Render Tree 中
    child._parent = this;
    if (attached) {
      child.attach(_owner!);
    }
    redepthChild(child);
  }
```

&emsp;看到 Mark 函数就意味着本帧内是不处理 mark 对应的事件的，也就是意味着 Layout 和 Paint 这些事件都是在当前帧不处理的。也就是意味着在 Element Tree 和 Render Tree 构建完成以后，才会统一进行 Layout 和 Paint。

&emsp;然后我们要把目光聚集在 markNeedsLayout 和 markNeedsCompositingBitsUpdate 两个函数内部，在它们两个 mark 函数内部看到：只有分别满足自己是 "重新布局边界" 和 "重新绘制边界" 的 RenderObject 才能被加入到 PipelineOwner 的脏列表中。并在末尾请求视觉更新，以待在新的一帧中处理被收集到脏列表的 RenderObject 对象。（其实这也是可以理解的，以我们前面学习 RenderObject LAYOUT 部分为例，我们知道当父级需要子级的 Size 时和子级不能自己决定自己的 Size 时，那么父级会作为子级的 "重新布局边界"，这样可以保证当子级需要布局更新时会直接通知到父级进行布局更新，然后子级会顺带被父级更新布局。所以直白一点的理解 "重新布局边界" 的话，可以把它理解为一组彼此有布局依赖的 RenderObject 对象。通过把它们绑定在一个布局边界中，然后当其中有某个 RenderObject 需要更新布局时，就让边界进行更新，那么此一组 RenderObject 对象都可以得到布局更新了，这样即保证了不会发生布局错乱。所以加入 PipelineOwner 的脏列表中的 RenderObject 都是 **边界** 也是很好理解的！）

&emsp;关于 markNeedsCompositingBitsUpdate 的话，因为它是与 Paint 紧密相连的，但是我们还没有学习 Paint 部分，所以这里先拓展一下。markNeedsCompositingBitsUpdate 所做的事情是标记需要对 RenderObject 的合成位进行更新，而合成位是谁呢？其实就是指的 RenderObject 的 `late bool _needsCompositing;` 的属性，这个属性特别重要。

&emsp;`_needsCompositing` 是一个标识位，用于表示当前的 RenderObject 是否需要进行合成操作（compositing）。当 RenderObject 拥有子级并且子级拥有透明度、变换、剪裁等属性时，父级就需要进行合成。在 Flutter 中，合成操作会将多个透明、旋转、裁剪等操作合并成一个单一的纹理，以提高渲染性能。

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
  
  // 当子节点包含透明度或者旋转等属性时，将 _needsCompositing 置为 true
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

&emsp;OK，我们下面继续以 Element Tree 和 Render Tree 初次构建为线索，分析其中涉及到的 Layout 和 Paint 流程。

&emsp;adoptChild 函数提示我们的最最重要的大概是：当一个新的 RenderObject 对象被附加到 Render Tree 后都需要被标记为需要 Layout 和 CompositingBitsUpdate，然后呢，在新的一帧内，我们在真正执行 Layout 和 CompositingBitsUpdate 后呢，才会标记 RenderObject 需要 Paint。所以大概可以理解为：当前帧进行 Element Tree 和 Render Tree 的构建，然后 Tree 构建完成后，才会在新的一帧回调中对构建 Render Tree 过程中收集到的是边界的 RenderObject 进行布局和把它的合成位：`_needsCompositing` 置为 true 或 false，而在这个过程中会调用 RenderObject.markNeedsPaint，同样在 markNeedsPaint 函数内部也是只对是重新绘制边界的 RenerObjedt 才会被收集在 PipelineOwner 的脏列表中，然后等待进入新的一帧后才会对这些被收集的 RenderObject 进行绘制。

&emsp;所以可以把以上内容简单拆分为以下阶段：

1. 阶段 1️⃣：Element Tree 和 Render Tree 一层一层构建，并在 Render Tree 构建过程中 标记 ... 
2. 阶段 2️⃣：重新布局边界是自己的 RenderObject 进行 Layout 和 CompositingBitsUpdate，在此过程中 ...
3. 阶段 3️⃣：重新绘制边界是 True 的 RenderObject 进行 Paint。在 此过程中 ...

&emsp;下面我们就证明这个过程。  
