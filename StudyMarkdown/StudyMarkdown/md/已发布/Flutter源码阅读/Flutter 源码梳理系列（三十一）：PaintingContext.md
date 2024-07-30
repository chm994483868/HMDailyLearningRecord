# Flutter 源码梳理系列（三十一）：PaintingContext

# PaintingContext

&emsp;PaintingContext：是一个为 RenderObject 对象提供绘制操作的上下文或者说是提供绘制操作环境的类，注意它仅是针对 RenderObject 及其子类的绘制，并不提供像 Canavas 那种画线、画圆等基础的绘制操作，它本身持有 Canvas、PictureRecorder 和 PictureLayer 等绘制相关的属性，然后把它们整合起来，向外为绘制 RenderObject 对象提供接口。

&emsp;在 PaintingContext 类内部有完整的 Layer Tree 的构建踪迹，如果要研究 Layer Tree 的构建流程的话，必定要研究 PaintingContext 类的内容。 

&emsp;PaintingContext 可谓是我们学习 Flutter 绘制相关内容的整合类，接下来我们会接触一大波绘制相关的类，为了防止我们没有头绪，所以在这里，我们先对这些类的功能进行总结，不然学习起来真的容易迷糊。

+ Offset：表示一个二维坐标系中的偏移量，通常用来表示绘制时在 Canvas 的偏移。
+ Size：表示一个矩形区域的宽度和高度，常用于确定组件的大小。
+ Rect：表示一个矩形区域，用 left/right/top/bottom 四个值标识在坐标系中的位置和大小。
+ RRect：表示一个圆角矩形，可以指定矩形四个角的圆角。
+ Path：表示一条绘制路径，用来描述图形的轮廓。
+ Paint：用来描述绘制图形的样式，如颜色、线宽、填充方式等。
+ Canvas：提供了一系列绘制方法，可以在画布上进行绘制操作。
+ PictureRecorder：用于记录绘制操作，并生成所有绘制操作的 Picture 对象。
+ Picture：记录了一系列的绘制操作，可以通过 Scene 绘制到屏幕上。

&emsp;OK，我们接下来继续学习 PaintingContext 类的内容以及它的源码。 

&emsp;与直接持有 Canvas 不同，RenderObject 使用 PaintingContext 进行绘制。PaintingContext 中包含一个 Canvas，即 PaintingContext 类的 `Canvas? _canvas` 属性，用于接收各个绘制操作（draw operations），并且 PaintingContext 类还具有用于绘制子级 RenderObject 的函数（PaintingContext：void paintChild(RenderObject child, Offset offset) 函数超级重要，还记得上篇吗？当父级 RenderObject 需要 **直接绘制** 子级 RenderObject 对象时，就是调用的此函数，当绘制子级 RenderObject 时，当前的 PaintingContext 对象会被传递下去，保证父级和子级 RenderObject 是在一个绘制环境下，内容都绘制在一起，如果子级是绘制边界的话则需要开辟新的绘制环境。）。

&emsp;在绘制子级 RenderObject 时，由 PaintingContext 持有的 Canvas 可能会发生变化，因为在绘制子级 RenderObject 之前和之后发出的绘制操作（draw operations）可能会记录在单独的合成层（compositing layers）中。因此，在可能绘制子级 RenderObject 的操作之间不要保留对 Canvas 的引用。（当子级 RenderObject 对象是绘制边界时，会创建新的 Layer 和 PaintingContext 来进行绘制，旧的 Canvas 会被丢弃。）

&emsp;当使用 PaintingContext.repaintCompositedChild 和 pushLayer 时，会自动创建新的 PaintingContext 对象。（它们会结束之前的绘制记录，生成 Picture，然后重新开辟新的绘制环境进行接下来的绘制操作。）

+ Object -> ClipContext -> PaintingContext

&emsp;看到 PaintingContext 直接继承自 ClipContext。ClipContext 提供了三个裁剪函数：根据指定的 Rect 裁剪、根据指定的 RRect 裁剪、根据指定的 Path 裁剪，而 PaintingContext 的三个裁剪函数：pushClipRect、pushClipRRect、pushClipPath 内部就是通过它们实现的。那看到裁剪是不是会想起 Canvas 呢？是的，其实 ClipContext 提供的裁剪操作就是由 Canvas 进行裁剪，又分别对应了 Canvas 的三个裁剪函数：

+ `void clipRect(Rect rect, { ClipOp clipOp = ClipOp.intersect, bool doAntiAlias = true });`
+ `void clipRRect(RRect rrect, {bool doAntiAlias = true});`
+ `void clipPath(Path path, {bool doAntiAlias = true});`

&emsp;下面我们先看一下 ClipContext 抽象类的内容。

## ClipContext

&emsp;ClipContext 抽象类的内容很简单，它只有一个 canvas getter 和 3 个非抽象函数，官方给它下的定义是：PaintingContext 使用的裁剪工具。然后 ClipContext 类的 3 个非抽象函数完成的功能分别是：

1. clipPathAndPaint：根据指定的 Path 对 Canvas canvas 进行裁剪。
2. clipRRectAndPaint：根据指定的圆角矩形 RRect 对 Canvas canvas 进行裁剪。
3. clipRectAndPaint：根据给定的 Rect 对 Canvas 进行裁剪。

### canvas

&emsp;ClipContext 抽象类有一个 canvas 的 getter，这样也表示了 PaintingContext 类需要有一个 Canvas canvas 属性。

```dart
  Canvas get canvas;
```

### `_clipAndPaint`

&emsp;`_clipAndPaint` 是一个私有的工具函数，主要为下面的三个真正的裁剪函数的功能进行统一封装。`_clipAndPaint` 函数会根据不同的参数执行不同操作。

&emsp;`_clipAndPaint` 函数的第 1 个参数是一个入参为 bool，返回值为 void 的函数，bool 的参数名是：doAntiAlias，即表示是否执行抗锯齿。第 2 个参数 clipBehavior 是一个 Clip 枚举值，下面我们看下这个 Clip 枚举都有哪些值：（Clip 枚举值表示裁剪 widgets 内容时的不同方式。）

1. Clip.none：没有任何裁剪。这是大多数 widgets 的默认选项：如果内容不溢出 widgets 边界，则不要为裁剪支付任何性能成本。如果内容确实溢出，那请明确指定以下裁剪选项：

2. Clip.hardEdge：这是最快的裁剪选项，但精度较低。hardEdge 表示进行剪裁，但不会应用抗锯齿效果。这种模式启用了剪裁，但曲线和非轴对齐的直线会呈锯齿状，因为没有尝试抗锯齿。比其他裁剪模式快，但比不裁剪（Clip.none）慢。当需要裁剪时，如果容器是一个轴对齐的矩形或一个带有非常小角半径的轴对齐圆角矩形，这是一个合理的选择。

+ Clip.antiAlias：比 hardEdge 稍慢，但具有平滑边缘。antiAlias 表示带有抗锯齿效果的裁剪。此模式具有抗锯齿的裁剪边缘，以实现更平滑的外观。它比 antiAliasWithSaveLayer 更快，但比 hardEdge 更慢。处理圆和弧时，这将是常见情况。

+ Clip.antiAliasWithSaveLayer：比 antiAlias 慢得多，几乎不应该被使用。antiAliasWithSaveLayer 表示使用抗锯齿裁剪并紧接着保存图层（SaveLayer）。这种模式不仅可以进行抗锯齿裁剪，还会分配一个离屏缓冲区（offscreen buffer）。所有后续的绘制都会在该缓冲区上进行，最后再进行裁剪和合成返回。这种方法速度非常慢。它没有可能出现的锯齿边缘效果（具有抗锯齿的效果），但是由于引入了一个离屏缓冲区，会改变语义。这种情况很少需要。可能需要这种情况的一种情形是：如果要在一个非常不同的背景颜色上叠加图像。但在这些情况下，考虑是否可以避免在一个地方叠加多种颜色（例如通过仅在图像缺席的地方存在背景颜色）。如果可以的话，抗锯齿就可以胜任并且速度更快。

&emsp;总结下来即根据裁剪情况和判断是否需要抗锯齿来使用哪个 Clip 的值，当处理圆或者弧时开启抗锯齿用 Clip.antiAlias，其它只需要裁剪不需要抗锯齿时使用 Clip.hardEdge 即可。

&emsp;然后是 `_clipAndPaint` 的第 3 个参数 Rect bounds，当使用 Clip.antiAliasWithSaveLayer 裁剪行为时作为 canvas.saveLayer 函数的参数。第 4 个参数则作为一个裁剪完成后执行的绘制操作，会直接和裁剪操作在同一个绘制栈中进行。

```dart
  void _clipAndPaint(void Function(bool doAntiAlias) canvasClipCall,
                     Clip clipBehavior,
                     Rect bounds,
                     VoidCallback painter) {
                     
    // 首先对 canvas 当前状态进行保存
    canvas.save();
    
    // 根据 clipBehavior 裁剪行为进行不同的裁剪
    switch (clipBehavior) {
      case Clip.none:
      
        // 完全无需裁剪
        break;
        
      case Clip.hardEdge:
      
        // 仅需要进行裁剪，并不进行抗锯齿处理，减少不必要的性能损耗
        canvasClipCall(false);
        
      case Clip.antiAlias:
      
        // 进行裁剪并且进行抗锯齿处理
        canvasClipCall(true);
        
      case Clip.antiAliasWithSaveLayer:
      
        // 进行裁剪并且进行抗锯齿处理，并且 执行 canvas.saveLayer，使得入参 VoidCallback painter 在新的 Layer 中进行。
        canvasClipCall(true);
        canvas.saveLayer(bounds, Paint());
    }
    
    // canvas 进行裁剪后，继续直接在当前的绘制栈中执行入参 VoidCallback painter传递来的绘制操作。
    painter();
    
    // 如果裁剪行为是 抗锯齿并且保存 Layer，则要多一次 canvas.restore，
    // 因为 save 和 restore 要一一对应的。所以，这里是两次 save 对应两次 restore。
    // 关于 save 和 restore 要一一对应的问题，可以翻看前面的 Canvas 篇。
    if (clipBehavior == Clip.antiAliasWithSaveLayer) {
      canvas.restore();
    }
    
    // 与函数开始处的 canvas.save() 相匹配，回到此 _clipAndPaint 函数执行前的绘制状态。
    canvas.restore();
  }
```

&emsp;OK，`_clipAndPaint` 这个简单的工具函数看完了，看下下面都会使用它进行哪些裁剪行为。

### clipPathAndPaint

&emsp;根据 Clip clipBehavior 和 Path path 对 canvas 进行裁剪，然后执行入参 VoidCallback painter 中的绘制操作。在此之后，canvas 将恢复到裁剪之前的状态。（Rect bounds 参数是用于当 Clip clipBehavior 参数值是 Clip.antiAliasWithSaveLayer 时，Canvas.saveLayer 函数的边界。）

&emsp;可看到内部 `_clipAndPaint` 第一个参数直接使用的 Canvas.clipPath 函数。

```dart
  void clipPathAndPaint(Path path, Clip clipBehavior, Rect bounds, VoidCallback painter) {
  
    // 第一个参数直接使用 canvas.clipPath(path, doAntiAlias: doAntiAlias) 函数
    _clipAndPaint((bool doAntiAlias) => canvas.clipPath(path, doAntiAlias: doAntiAlias), clipBehavior, bounds, painter);
    
  }
```

### clipRRectAndPaint

&emsp;根据 Clip clipBehavior 和 RRect rrect 对 canvas 进行裁剪，然后执行入参 VoidCallback painter 中的绘制操作。在此之后，canvas 将恢复到裁剪之前的状态。（Rect bounds 参数是用于当 Clip clipBehavior 参数值是 Clip.antiAliasWithSaveLayer 时，Canvas.saveLayer 函数的边界。）

&emsp;可看到内部 `_clipAndPaint` 第一个参数直接使用的 Canvas.clipRRect 函数。

```dart
  void clipRRectAndPaint(RRect rrect, Clip clipBehavior, Rect bounds, VoidCallback painter) {
  
    // 第一个参数直接使用 canvas.clipRRect(rrect, doAntiAlias: doAntiAlias) 函数
    _clipAndPaint((bool doAntiAlias) => canvas.clipRRect(rrect, doAntiAlias: doAntiAlias), clipBehavior, bounds, painter);
    
  }
```

### clipRectAndPaint

&emsp;根据 Clip clipBehavior 和 Rect rect 对 canvas 进行裁剪，然后执行入参 VoidCallback painter 中的绘制操作。在此之后，canvas 将恢复到裁剪之前的状态。（Rect bounds 参数是用于当 Clip clipBehavior 参数值是 Clip.antiAliasWithSaveLayer 时，Canvas.saveLayer 函数的边界。）

&emsp;可看到内部 `_clipAndPaint` 第一个参数直接使用的 Canvas.clipRect 函数。

```dart
  void clipRectAndPaint(Rect rect, Clip clipBehavior, Rect bounds, VoidCallback painter) {
  
    // 第一个参数直接使用 canvas.clipRect(rect, doAntiAlias: doAntiAlias) 函数
    _clipAndPaint((bool doAntiAlias) => canvas.clipRect(rect, doAntiAlias: doAntiAlias), clipBehavior, bounds, painter);
    
  }
```

&emsp;OK，到这里 ClipContext 抽象类的内容就看完了，它确实是直接提供了三个非抽象的裁剪函数，内部直接使用 canvas 的裁剪函数：根据路径裁剪、根据矩形裁剪、根据圆角矩形裁剪。作为 PaintContext 的裁剪工具，名副其实。下面我们则继续看 PaintContext 类的内容。

## Constructors

&emsp;创建一个 PaintingContext 对象。通常只会被 PaintingContext.repaintCompositedChild 和 pushLayer 调用。（注意 PaintingContext、Canvas、Layer 它们是一一对应的，在 PaintingContext.repaintCompositedChild 和 pushLayer 中，绘制使用的 Canvas、Layer 也会提供新的，以此来保证绘制操作的隔离性。）

```dart
class PaintingContext extends ClipContext {
  @protected
  PaintingContext(this._containerLayer, this.estimatedBounds);
  
  // ...
}
```

## `_containerLayer`

&emsp;PaintingContext 创建时传入的 Layer 参数，一般是开启绘制时的那个 RenderObject 对象的 layer，后续如果有是绘制边界的 RenderObject 时，它的独立的 Layer 会拼接到此 Layer 上，以此进行的便是 Layer Tree 的构建。所以每个 PaintingContext 的 `_containerLayer` 属性都可以理解是 Layer Tree 上的一个节点。

```dart
  final ContainerLayer _containerLayer;
```

## estimatedBounds

&emsp;estimatedBounds 是对于 PaintingContext 的 canvas 记录绘画命令的边界的估计。 这对于调试非常有用。

&emsp;canvas 将允许在这些边界之外进行绘制。estimatedBounds 矩形位于 canvas 坐标系统中。

```dart
  final Rect estimatedBounds;
```

## repaintCompositedChild

&emsp;PaintingContext 的静态函数。绘制给定的入参 RenderObject child，该 RenderObject 对象必须附加到一个 PipelineOwner（`child.owner != null`），必须有一个合成的图层（`child._layerHandle.layer != null`），并且必须需要绘制（`child._needsPaint == true`）。

&emsp;该 RenderObject child 对象的 layer（如果有的话）会被重用，以及以 child 为根的 Render 子树中不需要重新绘制的任何 RenderObject 节点的 layer 都将被重用。（即下面绘制开始前的 child._layerHandle.layer 的判断逻辑，如果为 null 的话则会为其创建一个新的 Layer，如果不为 null 的话则拿出来重用，保证真正开始绘制前，child._layerHandle.layer 必不是 null 的。）

```dart
  static void repaintCompositedChild(RenderObject child, { bool debugAlsoPaintedParent = false }) {
    _repaintCompositedChild(
      child,
      debugAlsoPaintedParent: debugAlsoPaintedParent,
    );
  }
  
  static void _repaintCompositedChild(
    RenderObject child, {
    bool debugAlsoPaintedParent = false,
    PaintingContext? childContext,
  }) {
    // 读取 child 的当前的 layer 并把它转换为 OffsetLayer。
    OffsetLayer? childLayer = child._layerHandle.layer as OffsetLayer?;
    
    // 如果入参 child 的 layer 是 null 的话，会给他提供一个新值。
    if (childLayer == null) {
    
      // ⚠️ RenderObject.updateCompositedLayer 函数如果入参 oldLayer 为 null 的话，会新建一个 Layer 对象并返回。
      // RenderObject 的 updateCompositedLayer 函数默认返回一个 OffsetLayer 对象，
      // 其它的 RenderObject 子类，如 RenderAnimatedOpacityMixin 返回一个设置了 alpha 属性的 OpacityLayer 对象，
      // 如 _ImageFilterRenderObject 返回一个设置了 imageFilter 属性的 ImageFilterLayer 对象。
      // 当入参 oldLayer 不为 null 的话，会先更新它的属性值，不同的 RenderObject 子类会更新不同的属性，
      // 然后再把这个入参 oldLayer 直接返回。
      
      final OffsetLayer layer = child.updateCompositedLayer(oldLayer: null);
      
      // 所以这里算是给 child 设置一个 layer 值，经过这里后，child 的 layer 就不是 null 了。
      child._layerHandle.layer = childLayer = layer;
    } else {
      // 如果入参 child 的 layer 已经有值了，则对其内部的子级 Layer 进行清理。
      childLayer.removeAllChildren();
      
      // 更新入参 child 的 layer 的属性值后再返回。（这里的 updateCompositedLayer 函数，不同的 RenderObject 子类会有不同的实现。）
      final OffsetLayer updatedLayer = child.updateCompositedLayer(oldLayer: childLayer);
    }
    
    // 需要合成层进行更新 的标识置为 false。
    child._needsCompositedLayerUpdate = false;
    
    // 以入参 RenderObject child 的 layer 属性为参数，创建一个 PaintingContext 对象。
    // 自此此 layer 也会成为 Layer Tree 上的一个节点，后续遇到需要新增独立的 Layer 之前的绘制操作都会在这个 Layer 上进行。 
    childContext ??= PaintingContext(childLayer, child.paintBounds);
    
    // 然后以入参 RenderObject child 为起点沿着它的 child 链对其子级 RenderObject 进行递归绘制。
    // 这里可以注意下 childContext 会直接被作为参数传递下去。（如果下面是 RenderObject 的连续绘制的话保证它们都处于相同的绘制环境中。）
    child._paintWithContext(childContext, Offset.zero);

    // 当执行到这里的时候，说明以入参 RenderObject child 为起点的整个 Render 子树上的所有 RenderObject 对象都绘制完成了，
    // 在这里停止记录，或者说是停止绘制了，这里会把之前的种种绘制结果整合在一起。
    childContext.stopRecordingIfNeeded();
  }
```

## updateLayerProperties

&emsp;静态函数。更新入参 RenderObject child 的 layer 而不重绘其子级。(此函数内部使用的 updateCompositedLayer 函数，不同的 RenderObject 子类会根据自己的需要进行重写。)

```dart
  static void updateLayerProperties(RenderObject child) {
    // 读取入参 RenderObject child 的 layer
    final OffsetLayer childLayer = child._layerHandle.layer! as OffsetLayer;
    
    // RenderObject.updateCompositedLayer 函数如果入参 oldLayer 为 null 的话，会新建一个 Layer 对象并返回。
    
    // RenderObject 的 updateCompositedLayer 函数默认返回一个 OffsetLayer 对象，
    // 其它的 RenderObject 子类，
    // 如 RenderAnimatedOpacityMixin 返回一个设置了 alpha 属性的 OpacityLayer 对象，
    // 如 _ImageFilterRenderObject 返回一个设置了 imageFilter 属性的 ImageFilterLayer 对象。
    // 当入参 oldLayer 不为 null 的话，会先更新它的属性值，不同的 RenderObject 子类会更新不同的属性，
    // 然后再把这个入参 oldLayer 直接返回。
    final OffsetLayer updatedLayer = child.updateCompositedLayer(oldLayer: childLayer);
    
    // 需要合成层更新标识 置为 false
    child._needsCompositedLayerUpdate = false;
  }
```

## paintChild

&emsp;绘制入参 RenderObject child。如果 child 有自己的 layer，则该 layer 将合成到与此 PaintingContext 关联的 layer subtree 中。否则，该 RenderObject child 将绘制到此 PaintingContext 的当前 PictureLayer 中。

```dart
  void paintChild(RenderObject child, Offset offset) {
    // ⚠️⚠️ 如果入参 child 是绘制边界的话，会为它单独创建自己的 layer。
    if (child.isRepaintBoundary) {
      // 把之前的绘制结果保存起来。
      stopRecordingIfNeeded();
      
      // 以当前 child 为起点，重头开始以它为根节点的整个 Render 子树的绘制流程。
      _compositeChild(child, offset);
    } else if (child._wasRepaintBoundary) {
      // 如果之前是绘制边界，但是现在不是了，需要把它之前的独立的 layer 释放掉。
      child._layerHandle.layer = null;
      
      // 然后则是把此 child 和它的父级一同绘制在当前 PaintingContext 的当前 PictureLayer 中。
      child._paintWithContext(this, offset);
    } else {
      // 不是绘制边界的，则是把此 child 和它的父级一同绘制在当前 PaintingContext 的当前 PictureLayer 中。
      child._paintWithContext(this, offset);
    }
  }
  
  void _compositeChild(RenderObject child, Offset offset) {
    // 为入参 RenderObject child 创建一个独立的 Layer，并将它后续的非绘制边界的子级 RenderObject 绘制在此 Layer 上。
    // 执行到这里的 child 肯定是一个绘制边界 RenderObject，
    // 所以如果此 child 需要绘制，或者它之前不是绘制边界，则开始为它进行绘制。
    if (child._needsPaint || !child._wasRepaintBoundary) {
    
      // 以 child 为起点，调用 PaintingContext.repaintCompositedChild 函数，开启一轮新的绘制。
      // 这里也是一个递归调用。
      repaintCompositedChild(child, debugAlsoPaintedParent: true);
      
    } else {
      // 如果 child 之前已经是绘制边界了，则进行 layer 更新即可。
      if (child._needsCompositedLayerUpdate) {
        updateLayerProperties(child);
      }
    }

    // 当执行到这里时，则以 child 为根节点的整个 Render 子树就绘制完成了，它们的绘制内容都被整合在了 childOffsetLayer 中。
    final OffsetLayer childOffsetLayer = child._layerHandle.layer! as OffsetLayer;
    childOffsetLayer.offset = offset;
    
    // 把此 childOffsetLayer 与当前的 PaintingContext 的 _containerLayer 拼接在一起。
    appendLayer(childOffsetLayer);
  }
```

&emsp;OK，下面基本就是全部与 Layer 相关的 API 了，觉得大家需要补充点 Layer 的知识点在看这一部分内容。

## appendLayer

&emsp;向 `_containerLayer` 拼接入参 layer，需要确保绘制记录已经停止。请不要直接调用这个函数：而是应该调用 addLayer 或 pushLayer。当所有不是从 Canvas 生成的 Layer 都被添加时，这个函数会在内部被调用。

&emsp;需要自定义如何添加 Layer 的 Layer 子类应该重写 append 方法。

```dart
  @protected
  void appendLayer(Layer layer) {
    // 入参 layer 需要从已有的父级 Layer 中移除，如果有的话。
    layer.remove();
    
    // 直接把 layer 拼接在当前 PaintingContext 的 _containerLayer 中
    _containerLayer.append(layer);
  }
```

## `_isRecording`

&emsp;如果此 PaintingContext 的 canvas 属性不为 null，则就是正在记录绘制操作。因为有 canvas 就代表着有绘制操作进行了。

```dart
  bool get _isRecording {
    final bool hasCanvas = _canvas != null;
    
    return hasCanvas;
  }
```

## canvas

&emsp;此 PaintingContext 用于绘制的画布。当使用此 PaintingContext 绘制子级 RenderObject 时，当前 canvas 可能会改变，这意味着保持对此 getter 返回的 canvas 的引用是不稳定的。

```dart
  // Recording state
  
  // 由 RendererBinding 提供创建接口，在 _startRecording 函数中赋值
  PictureLayer? _currentLayer; 
  ui.PictureRecorder? _recorder;
  
  Canvas? _canvas;

  @override
  Canvas get canvas {
    if (_canvas == null) {
      // 如果当前 canvas 为 null，则调用 _startRecording 函数。
      // _startRecording 函数内是：_currentLayer、_recorder、_canvas 初始化
      _startRecording();
    }
    
    return _canvas!;
  }
```

## `_startRecording`

&emsp;当已停止的 PaintingContext 开始新的记录/新的绘制时，四个属性都会来新的值，以此保证跟之前的绘制隔离。

```dart
  void _startRecording() {
    // 新的 PictureLayer。
    _currentLayer = PictureLayer(estimatedBounds);
    
    // 直接通过 RendererBinding 的接口创建新的 PictureRecorder、Canvas 对象。
    _recorder = RendererBinding.instance.createPictureRecorder();
    _canvas = RendererBinding.instance.createCanvas(_recorder!);
    
    // 新的 _currentLayer 会直接拼接在 _containerLayer 上，
    //（这里也直接可以理解为：在构建以 _containerLayer 为根节点的 Layer 子树。）
    _containerLayer.append(_currentLayer!);
  }
```

## addCompositionCallback

&emsp;为当前 PaintingContext 使用的 `ContainerLayer _containerLayer` 添加 CompositionCallback。当包含当前 PaintingContext 的 Layer 树进行合成或被分离且不会再次呈现时，将调用合成回调。无论 Layer 是通过保留渲染还是其他方式添加的，都会发生这种情况。

&emsp;合成回调对于推送一个本应尝试观察层树但实际上不影响合成的层很有用。例如，合成回调可用于观察当前 `_containerLayer` 的总变换和裁剪，以确定绘制到其中的 RenderObject 是否可见。

&emsp;调用返回的回调将从合成回调中移除回调。

&emsp;这里是添加到 Layer 的回调，后续学习 Layer 时再深入。

```dart
  VoidCallback addCompositionCallback(CompositionCallback callback) {
    return _containerLayer.addCompositionCallback(callback);
  }
```

## stopRecordingIfNeeded

&emsp;如果已经开始记录绘制操作，请停止，并且把前面的绘制结果生成 Picture。

&emsp;请不要直接调用此函数：此类中的函数会根据需要调用此方法。此函数会在内部调用以确保在添加 Layer 或完成绘制结果之前停止记录。需要定制如何将绘制记录保存到 Canvas 的子类应重写此方法以保存自定义 Canvas 记录的结果。

```dart
  @protected
  @mustCallSuper
  void stopRecordingIfNeeded() {
    if (!_isRecording) {
      return;
    }
    
    // PictureRecorder 调用 endRecording 会停止记录当前的绘制操作，并把之前记录的绘制操作生成 Picture 对象。
    // 因为之前 _currentLayer 已经被拼接在 _containerLayer 中了，所以下面的 _currentLayer 置 null 不会影响，
    // 下面的 _currentLayer = null 只是断开了当前 PaintingContext 对象的 _currentLayer 属性的引用。
    _currentLayer!.picture = _recorder!.endRecording();
    
    _currentLayer = null;
    _recorder = null;
    _canvas = null;
  }
```

## setIsComplexHint

&emsp;这个提示表明当前图层中的绘制操作比较复杂，可以从缓存中受益。

&emsp;如果未设置此提示，compositor 将使用自己的启发法来决定当前图层是否足够复杂以从缓存中受益。

&emsp;调用这个函数会确保 Canvas 可用。只有在当前 Canvas 上的绘制调用会收到提示；此提示不会传播到在添加新 Layer 到 PaintingContext 时创建的新 Canvas（例如使用 addLayer 或 pushLayer）。

```dart
  void setIsComplexHint() {
    if (_currentLayer == null) {
    
     // _startRecording 函数内是：_currentLayer、_recorder、_canvas 初始化，
     // 为后续的绘制操作做准备。
      _startRecording();
    }
    
    _currentLayer!.isComplexHint = true;
  }
```

## setWillChangeHint

&emsp;当前图层中的绘画可能在下一帧发生变化的提示。

&emsp;这个提示告诉 compositor 不要缓存当前图层，因为缓存在将来不会被使用。如果未设置此提示，compositor 会应用自己的启发式算法来决定当前图层是否可能在将来被重用。

&emsp;调用这个函数会确保 Canvas 可用。只有在当前 Canvas 上的绘制调用会收到提示；此提示不会传播到在添加新 Layer 到 PaintingContext 时创建的新 Canvas（例如使用 addLayer 或 pushLayer）。

```dart
  void setWillChangeHint() {
    if (_currentLayer == null) {
    
      // _startRecording 函数内是：_currentLayer、_recorder、_canvas 初始化，
      // 为后续的绘制操作做准备。
      _startRecording();
    }
    
    _currentLayer!.willChangeHint = true;
  }
```

## `_startRecording` ~ stopRecordingIfNeeded 总结

&emsp;OK，看到这里我们暂停总结一下 PaintingContext：停止记录、开始记录和各个函数的关系：

&emsp;首先一个 PaintingContext 对象有以下重要属性：

+ `final ContainerLayer _containerLayer;`

+ `Canvas? _canvas;`
+ `ui.PictureRecorder? _recorder;`
+ `PictureLayer? _currentLayer;`

&emsp;首先是 `_containerLayer` 属性，它的值是由 PaintingtContext 构造函数传入的，有两个地方会涉及需要创建新的 PaintingContext 对象：

1. 情况 1️⃣：当要绘制 isRepaintBoundary 返回 true 的 RenderObject 对象时。
2. 情况 2️⃣：pushLayer 调用时，以及后续的一组嵌套 pushLayer 的 push 函数：pushClipRect、pushClipRRect、pushClipPath、pushColorFilter、pushTransform、pushOpacity。
3. 情况 3️⃣：addLayer 调用时。（此时没有创建新的 PaintingContext 对象，但是停止了绘制记录，所以我们还是给它拎出来。）

&emsp;以上三种情况还有一个共同点，就是它们会在当前的 PaintingContext 的 `_containerLayer` 中拼接新的 Layer 对象，这其实就是在构建 Layer Tree。

&emsp;当情况 1️⃣ 发生时，PaintingtContext 构造函数传入的 `_containerLayer` 参数就是此 RenderObject 的 `_layerHandle.layer`。当情况 2️⃣ 发生时，PaintingContext 构造函数传入的 `_containerLayer` 参数就是 pushLayer 的入参 ContainerLayer childLayer。

&emsp;然后另外三个属性就是统一在 PaintingContext 的 `_startRecording` 函数中进行初始化，并且会直接把 `_currentLayer` 属性拼接到 `_containerLayer` 中，而 `_startRecording` 函数的调用时机也很巧妙，每次都是在本轮绘制要开始前进行，会发现在 canvas 属性的 getter 中通过判断如果 canvas 为 null 则调用 `_startRecording` 函数，以及在 setIsComplexHint 和 setWillChangeHint 函数中通过判断 `_currentLayer` 为 null 调用 `_startRecording` 函数。

&emsp;再然后就是当情况 1️⃣ 出现需要创建新的 PaintingContext 时，以及 情况 2️⃣ 和 情况 3️⃣ 出现需要拼接新的 Layer 时，都会提前调用 PaintingContext 的 stopRecordingIfNeeded 函数，停止当前正在绘制的记录了，并且会把这一轮的绘制内容通过 `_currentLayer!.picture = _recorder!.endRecording()` 调用直接生成 Picture 对象而固化下来，由于前面已经把 `_currentLayer` 指向的 PictureLayer 对象拼接到 Layer Tree 上了，所以这里生成的 Picture 对象其实已经保存到 Layer Tree 中了，stopRecordingIfNeeded 函数接下来还会直接把 `_containerLayer` 之外的三属性置为 null 了，然后如果后续还有绘制进行的话会再调用 `_startRecording` 函数对这三属性进行初始化。 

&emsp;所以整体通过上面的分析可发现：情况 1️⃣ 和 情况 2️⃣ 每一轮的绘制使用的 PaintingContext、Canvas、PictureRecorder、PictureLayer、ContainerLayer 都是独立的，情况 3️⃣ 则是 Canvas、PictureRecorder、PictureLayer 都是独立的，而三种情况中，新的一轮绘制的开始的由头都可以理解为是要拼接一个新的 Layer。如上面的情况 1️⃣ 中，当沿着 RenderObject 链进行绘制时，如果不遇到是绘制边界的 RenderObject 对象，那么这一组 RenderObject 都会使用同一个 PaintingContext 绘制，都会直接绘制在同一个 PictureLayer 上，直到遇到是绘制边界的 RenderObject 对象，首先会结束本轮绘制，然后再为这个是绘制边界的 RenderObject 创建新的 Layer，然后再构建一组新的 PaintingContext、Canvas、PictureRecorder、PictureLayer、ContainerLayer 进行接下来的绘制。情况 2️⃣ 和 情况 3️⃣ 则是首先会结束本轮绘制，然后直接把入参 Layer 拼接到当前 PaintingContext 的 `_containerLayer` 中。

&emsp;然后，再重要的就是情况 1️⃣ 中每轮绘制之间的联系：下一轮的绘制会把自己的 `_containerLayer` 拼接到前一轮绘制的 `_containerLayer`，用以构建 Layer Tree，具体在代码层面是通过 PaintingContext 的 appendLayer 函数进行。更具体的代码是在 `_compositeChild` 函数内的：`appendLayer(childOffsetLayer);` 调用。情况 2️⃣ 则是在 `pushLayer` 函数内的：`appendLayer(childLayer);` 调用。情况 3️⃣ 则是在 `addLayer` 函数内的：`appendLayer(layer);` 调用。

```dart
  void _startRecording() {
    _currentLayer = PictureLayer(estimatedBounds);
    
    // 这里直接调用 
    _recorder = RendererBinding.instance.createPictureRecorder();
    _canvas = RendererBinding.instance.createCanvas(_recorder!);
    
    // ⚠️⚠️ 直接把刚刚创建的 _currentLayer 拼接到 _containerLayer 属性上，
    // 即在这里就直接把 _currentLayer 拼接到以 _containerLayer 为根节点的 Layer 子树上了。
    
    // 从代码层面看的话：_containerLayer 会直接持有这个 PictureLayer 对象，
    // 所以即使在 stopRecordingIfNeeded 函数中会把此 _currentLayer 属性置为 null，
    // 那也只是把当前的 PaintingContext 的 _currentLayer 属性置为 null，
    // 因为 _containerLayer 也直接持有这个 PictureLayer 对象，
    // 所以在 stopRecordingIfNeeded 中直接 _currentLayer = null 并不会导致这个 PictureLayer 对象的释放。
    // 
    _containerLayer.append(_currentLayer!);
  }
```

```dart
  void stopRecordingIfNeeded() {
    if (!_isRecording) {
      return;
    }
    
    // 保存下之前绘制的内容，或者说是保存下这一轮的绘制内容，生成 Picture。
    _currentLayer!.picture = _recorder!.endRecording();
    
    // 然后直接把另外三个属性置为 null。
    
    // 在 _startRecording 函数中已经把 _currentLayer 指向的 PictureLayer 对象拼接在 _containerLayer 中了，
    // 所以这里只是把当前 PaintingContext 对象的 _currentLayer 属性置为 null 了。
    _currentLayer = null;
    _recorder = null;
    _canvas = null;
  }
```

&emsp;Ok，接下来我们详细看一下 addLayer 和 pushLayer 的内容。

## addLayer

&emsp;将入参 Layer layer 拼接到当前 PaintingContext 的 `_containerLayer` 属性中，并且在拼接 layer 之前，停止当前的绘制记录，每次停止绘制记录就代表着把之前的绘制操作生成 Picture，并保存在 Layer Tree 中。

&emsp;调用此函数后，canvas 属性将会引用一个新的 Canvas，该 Canvas 会在给定图层的顶部绘制。（如 stopRecordingIfNeeded 中把 canvas 属性置为 null，然后在 `_startRecording` 中，重新初始化。）

&emsp;使用此函数的 RenderObject 很可能需要使其 RenderObject.alwaysNeedsCompositing 属性返回 true。这会通知祖先 RenderObject 对象，此 RenderObject 对象将包括一个自己的独立 Layer，这会导致它们使用 composited clips。（例如在 TextureBox 中它的 alwaysNeedsCompositing 就是返回 true，然后它的 paint 函数内就是使用的 `context.addLayer(TextureLayer(...));` 调用。）

```dart
  void addLayer(Layer layer) {
    // 停止当前的绘制记录，并生成 Picture 对象。
    stopRecordingIfNeeded();
    
    // 把入参 layer 添加到当前 PaintingContext 的 _containerLayer 中。
    appendLayer(layer);
  }
```

## pushLayer

&emsp;将给定的入参 ContainerLayer childLayer 拼接到当前 PaintingContext 的 `_containerLayer` 中，并以入参 ContainerLayer childLayer 为参数执行入参 PaintingContextCallback painter 函数，将 childPaintBounds 作为新建的 PaintingContext 的估计绘制边界参数。childPaintBounds 可用于调试，但不会影响绘制。

&emsp;给定的入参 ContainerLayer childLayer 必须是一个未附加的孤立 Layer。（提供一个新创建的对象，而不是重用现有的图层，可以满足此要求。）

&emsp;offset 是传递给 painter 的偏移量。特别地，它不是应用于图层本身的偏移。从概念上讲，图层默认没有位置或大小，尽管它们可以改变其内容。例如，OffsetLayer 会对其子项应用偏移。

&emsp;如果 childPaintBounds 没有指定，则使用当前图层的绘画边界。如果子图层不对其内容应用任何转换或裁剪，则这是合适的。如果指定了 childPaintBounds，则必须在新图层的坐标系中（即在应用任何转换到其内容之后，由其子元素看到的位置），并且不应超出当前图层的绘画边界。

```dart
  void pushLayer(ContainerLayer childLayer, PaintingContextCallback painter, Offset offset, { Rect? childPaintBounds }) {
    // 如果图层正在被重用，它可能已经包含子组件。需要移除它们，以便 painter 可以添加对于这一帧而言相关的子组件。
    if (childLayer.hasChildren) {
      childLayer.removeAllChildren();
    }
    
    // 停止绘制记录并生成 Picture 对象。
    stopRecordingIfNeeded();
    // 把入参 childLayer 拼接到当前的 PaintingContext 中。
    appendLayer(childLayer);
    
    // 创建一个新的 PaintingContext，并以入参 ContainerLayer childLayer 作为它的 _containerLayer 属性，
    // 即在此 PaintingContext 下绘制的所有内容生成的所有 Layer 都会以 ContainerLayer childLayer 为根。
    final PaintingContext childContext = createChildContext(childLayer, childPaintBounds ?? estimatedBounds);
    
    // 执行入参 painter，并其所有的内容在 childContext 下进行绘制。
    painter(childContext, offset);
    
    // 注意这里是执行 childContext 的 stopRecordingIfNeeded，
    // 即把上面 painter 的绘制结果生成 Picture，附加在入参 ContainerLayer childLayer 中。
    childContext.stopRecordingIfNeeded();
  }
```

## createChildContext

&emsp;创建一个配置为在 childLayer 中绘制的 PaintingContext。bounds 是用于调试目的的估算绘制边界。

```dart
  @protected
  PaintingContext createChildContext(ContainerLayer childLayer, Rect bounds) {
    return PaintingContext(childLayer, bounds);
  }
```

## pushClipRect & pushClipRRect & pushClipPath

&emsp;使用矩形对进一步的绘制进行裁剪。

&emsp;needsCompositing 参数指定子节点是否需要合成。通常，这与调用者的 RenderObject.needsCompositing 的值相匹配。如果为 false，则此方法返回 null，表示不再需要 Layer。如果调用此方法的 RenderObject 将 oldLayer 存储在其 RenderObject.layer 字段中，则应将该字段设置为 null。

&emsp;当 needsCompositing 为 false 时，此方法将使用更高效的方法来应用图层效果，而不是实际创建新 Layer，即直接在当前 PaintingContext 进行绘制。

&emsp;offset 参数是从 Canvas 坐标系的原点到调用者坐标系原点的偏移量。

&emsp;clipRect 是应用于入参 PaintingContextCallback painter 所作画的矩形（在调用者坐标系中）的裁剪区域。它不应包括偏移。

&emsp;painter 回调将在应用 clipRect 时被调用。在调用 pushClipRect 时同步调用。

&emsp;clipBehavior 参数控制如何裁剪矩形。

&emsp;对于 oldLayer 参数，指定在上一帧中创建的 Layer。这为引擎提供了更多信息以进行性能优化。通常，这是 RenderObject 对象仅创建一次，然后在所有后续帧中重用的 RenderObject.layer 的值，直到不再需要图层（例如，RenderObject 对象不再需要合成）或者直到 RenderObject 对象更改图层类型（例如，从不透明度图层更改为裁剪矩形图层）。

&emsp;这里入参 bool needsCompositing 就是调用此函数的 RenderObject 的 needsCompositing 属性，就是我们之前文章画很多篇幅介绍的 needsCompositing 属性。一般 RenderObject 子类根据自己的情况决定此属性的 true 或者 false。然后对应到此函数，此属性决定了这个裁剪绘制操作是直接在当前的 PaintingContext 进行，还是新建 Layer 和 PaintingContext 进行。 

&emsp;然后下面还有一模一样流程的：pushClipRRect、pushClipPath 根据指定圆角矩形裁剪绘制 和 根据指定 Path 进行裁剪绘制。

```dart
  ClipRectLayer? pushClipRect(bool needsCompositing, Offset offset, Rect clipRect, PaintingContextCallback painter, { Clip clipBehavior = Clip.hardEdge, ClipRectLayer? oldLayer }) {
  
    // 如果裁剪行为 clipBehavior 参数是 none，表示不进行裁剪，则直接执行 painter 并 return。
    if (clipBehavior == Clip.none) {
    
      // 直接在当前的 PaintingContext 中进行绘制
      painter(this, offset);
      
      return null;
    }
    
    // shift 函数是把 Rect clipRect 添加 offset 偏移，
    // 即在 X 轴偏移 offset.dx，在 Y 轴偏移 offset.dy。
    final Rect offsetClipRect = clipRect.shift(offset);
    
    // 这里是根据入参 needsCompositing 是否需要新建 Layer 进行绘制，还是说在当前 Layer 进行绘制。
    if (needsCompositing) {
      // 如果入参 ClipRectLayer? oldLayer 有值则直接进行复用，
      // 如果无值，则新建一个新的 ClipRectLayer 对象。
      final ClipRectLayer layer = oldLayer ?? ClipRectLayer();
      
      // 并设置 ClipRectLayer 的 clipRect 和 clipBehavior 属性。
      layer
        ..clipRect = offsetClipRect
        ..clipBehavior = clipBehavior;
      
      // 以入参 layer 为根节点进行绘制，并把绘制结果附加到当前 PaintingContext 中。   
      pushLayer(layer, painter, offset, childPaintBounds: offsetClipRect);
      
      return layer;
    } else {
      // 这里是直接使用 PaintingContext 的父类 ClipContext 的裁剪函数，
      // 直接使用当前 PaintingContext 的 canvas 进行指定矩形的裁剪。
      clipRectAndPaint(offsetClipRect, clipBehavior, offsetClipRect, () => painter(this, offset));
      
      return null;
    }
  }
```

## pushColorFilter & pushOpacity & pushTransform

&emsp;进一步使用颜色滤镜进行混合绘图。

&emsp;offset 是要传递给 PaintingContextCallback painter 的偏移量。特别地，它不是应用于图层本身的偏移量。在概念上，默认情况下图层没有位置或大小，虽然它们可以转换其内容。例如，一个 OffsetLayer 会将偏移应用于其子元素。

&emsp;colorFilter 参数是用于混合 painter 绘制内容时要使用的 ColorFilter 值。

&emsp;当应用 colorFilter 时，painter 将被调用。它在调用 pushColorFilter 时同步调用。

&emsp;对于 oldLayer 参数，需要指定在上一帧中创建的 Layer。这为引擎提供了更多关于性能优化的信息。通常，这是 RenderObject 对象在创建一次后重用于所有后续帧直到不再需要 Layer（例如，RenderObject 对象不再需要合成）或者 RenderObject 对象更改 Layer 类型（例如，从 OpacityLayer 更改为 ClipRectLayer）的 RenderObject.layer 的值。

&emsp;使用这个函数的 RenderObject 很可能需要其 RenderObject.alwaysNeedsCompositing 属性返回 true。这通知祖先 RenderObject 对象，此 RenderObject 对象将包括一个合成图层，例如，这会导致它们使用 composited clips。

&emsp;pushOpacity 是完全一样流程的向当前的 PaintingContext 拼接 OpacityLayer。

&emsp;pushTransform 和之前的 pushClip 系列类似的流程。同样是拼接新的 Layer 进行绘制，或者是直接在当前 PaintingContext 的 Canvas 下进行绘制。

```dart
  ColorFilterLayer pushColorFilter(Offset offset, ColorFilter colorFilter, PaintingContextCallback painter, { ColorFilterLayer? oldLayer }) {
    
    // 准备 Layer，如果 oldLayer 有值则直接使用，无值的话新建一个 ColorFilterLayer
    final ColorFilterLayer layer = oldLayer ?? ColorFilterLayer();
    
    // 把颜色滤镜赋值给 layer 的 colorFilter 属性
    layer.colorFilter = colorFilter;
    
    // pushLayer，停止当前绘制生成 Picture，然后把 layer 附加在当前 PaintingContext，然后准备子 PaintingContext 进行绘制，
    // 并在绘制完成后直接结束绘制，生成对应的 Picture。
    pushLayer(layer, painter, offset);
    
    return layer;
  }

  TransformLayer? pushTransform(bool needsCompositing, Offset offset, Matrix4 transform, PaintingContextCallback painter, { TransformLayer? oldLayer }) {
  
    final Matrix4 effectiveTransform = Matrix4.translationValues(offset.dx, offset.dy, 0.0)
      ..multiply(transform)..translate(-offset.dx, -offset.dy);
      
    if (needsCompositing) {
      // 拼接新的 Layer 进行绘制。
      final TransformLayer layer = oldLayer ?? TransformLayer();
      layer.transform = effectiveTransform;
      
      pushLayer(
        layer,
        painter,
        offset,
        childPaintBounds: MatrixUtils.inverseTransformRect(effectiveTransform, estimatedBounds),
      );
      
      return layer;
    } else {
      // 在当前 PaintingContext 下的 canvas 直接进行绘制。
      // 这里对 save 和 restore 不理解的话，可以翻翻看前面的 Canvas 篇。
      canvas
        ..save()
        ..transform(effectiveTransform.storage);
        
      painter(this, offset);
      
      canvas.restore();
      
      return null;
    }
  }
```

## PaintingContext 总结

&emsp;PaintingContext 可谓是 Flutter 进行 RenderObject 和 Layer 进行绘制的整合类，它的内部持有一众的：Canvas、PictureRecorder、PictureLayer、ContainerLayer 类型的属性，它们都是与绘制操作紧密相关的类。

&emsp;从前半部分与 RenderObject 绘制相关的接口，可以看出当沿着 RenderObject 的 child 链进行绘制时，在未遇到是绘制边界的 RenderObject 时，这一组连续的 RenderObject 对象会共用同一个 Layer，并把内容绘制在同一个 Canvas 上，当遇到是绘制边界的 RenderObject 时，就会结束当前的绘制记录并直接生成 Picture 保存在 Layer Tree 中，然后重新创建 PaintingContext 并添加新的 Layer 进行接下来的绘制，并且会把这新建的 Layer 和前一个 PaintingContext 的 Layer 拼接在一起，就这样如此往复下去，没有边界时就多个绘制在一起，有边界了就新建 Layer 继续把接下来的多个绘制在一起，如此往复。并会以此进行 Layer Tree 的构建。

&emsp;从后半部分与 Layer 绘制直接相关的接口，可以看出，新增 Layer 的情况并非只有遇到是绘制边界的 RenderObject 这样，还有遇到在 paint 函数中使用 addLayer/pushLayer/pushColorFilter/pushOpacity 的 RenderObject 时，也会结束当前的绘制记录并直接生成 Picture 保存在 Layer Tree 中，然后直接新增新的 Layer，并使用新的 Canvas 进行绘制，在绘制结束后直接把绘制结果生成 Picture 保存在 Layer Tree 中。

&emsp;然后还有一部分特殊的 pushClipXXX/pushTransform 它们会根据入参 needsCompositing 来决定是直接在当前 PaintingContext 的 canvas 上直接绘制，还是说和上面一样新增 Layer 再进行绘制，而这个入参 needsCompositing 的值就是 RenderObject 的 needsCompositing 或 alwaysNeedsCompositing getter 来决定的。

&emsp;Layer Tree 就是在上面的新增或者不新增 Layer 的过程中一步一步构建起来的，每次新增 Layer 的话都会把它和前一个 Layer 拼接起来。Layer Tree 呢管理所有 Layer 的组织架构，其中的 PictureLayer，即 PaintingContext 的 `PictureLayer? _currentLayer` 属性，它用来记录每次结束绘制记录时生成的 Picture，其中的 ContainerLayer，即 PaintingContext 的 `final ContainerLayer _containerLayer` 属性，它用来连接每一个子 Layer 节点，然后还有 ColorFilterLayer、OpacityLayer、TransformLayer、ClipRectLayer、ClipRRectLayer、ClipPathLayer 等 Layer 会组织一些特殊效果的 Layer。

&emsp;每次具体的绘制操作或者是绘制内容则是在当前的 PaintingContect 的当前的 Canvas 下进行，当有新 Layer 要添加时，则结束当前的绘制记录，生成所有绘制操作的 Picture 保存在 Layer Tree 中的 PictureLayer 节点中。

## 参考链接
**参考链接:🔗**
+ [Canvas class](https://api.flutter.dev/flutter/dart-ui/Canvas-class.html)
+ [PictureRecorder class](https://api.flutter.dev/flutter/dart-ui/PictureRecorder-class.html)
+ [Picture class](https://api.flutter.dev/flutter/dart-ui/Picture-class.html)
+ [Flutter 3.7 之快速理解 toImageSync 是什么？能做什么？](https://guoshuyu.cn/home/wx/Flutter-N18.html)
+ [Clip enum](https://api.flutter.dev/flutter/dart-ui/Clip.html)
