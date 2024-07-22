# Flutter 源码梳理系列（二十八）：PaintingContext

# PaintingContext

&emsp;PaintingContext：一个画画的地方，绘制操作的上下文或者是为绘制操作提供环境的类。它是我们学习 Flutter 绘制相关内容的入门类。接下来我们会接触一大波绘制相关的类，为了防止我们没有头绪，所以在这里，我们先对这些类的功能进行总结，不然学习起来真的容易迷糊。

+ Offset
+ Size
+ Rect
+ Path
+ Paint
+ Canvas
+ PictureRecorder
+ Picture
+ ...

&emsp;OK，我们接下来继续学习 PaintingContext 类的内容以及它的源码。 

&emsp;与直接持有 Canvas 不同，RenderObjects 使用 PaintingContext 进行绘制。PaintingContext 中包含一个 Canvas（PaintingContext 类的 `Canvas? _canvas` 属性。），用于接收各个绘制操作（draw operations），并且还具有用于绘制子级 RenderObject 的函数（PaintingContext：void paintChild(RenderObject child, Offset offset)）。

&emsp;在绘制子级 RenderObject 时，由 PaintingContext 持有的 Canvas 可能会发生变化，因为在绘制子级 RenderObject 之前和之后发出的绘制操作（draw operations）可能会记录在单独的合成层（compositing layers）中。因此，在可能绘制子级 RenderObject 的操作之间不要保留对 Canvas 的引用。（这里牵涉到和 relayoutBoundary 类似的 repaintBoundary 知识点，我们后面再展开。）

&emsp;当使用 PaintingContext.repaintCompositedChild 和 pushLayer 时，会自动创建新的 PaintingContext 对象。

+ Object -> ClipContext -> PaintingContext

&emsp;看到 PaintingContext 直接继承自 ClipContext，下面我们先看一下 ClipContext 抽象类的内容。

## ClipContext

&emsp;ClipContext 抽象类的内容很简单，它只有一个 canvas getter 和 3 个非抽象函数，官方给它下的定义是：PaintingContext 使用的裁剪工具。然后 ClipContext 类的 3 个非抽象函数完成的功能分别是：

1. clipPathAndPaint：根据指定的 Path 对 Canvas canvas 进行裁剪。
2. clipRRectAndPaint：根据指定的圆角矩形 RRect 对 Canvas canvas 进行裁剪。
3. clipPathAndPaint：根据给定的 Rect 对 Canvas 进行裁剪。这么看的话，确实

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

+ Clip.antiAliasWithSaveLayer：比 antiAlias 慢得多，几乎不应该被使用。antiAliasWithSaveLayer 表示使用抗锯齿裁剪并紧接着保存图层（SaveLayer）。这种模式不仅可以进行抗锯齿裁剪，还会分配一个离屏缓冲区（offscreen buffer）。所有后续的绘制都会在该缓冲区上进行，最后再进行裁剪和合成返回。这种方法速度非常慢。它没有可能出现的锯齿边缘效果（具有抗锯齿的效果），但是由于引入了一个离屏缓冲区，会改变语义。（查看 https://github.com/flutter/flutter/issues/18057#issuecomment-394197336 以了解不使用 saveLayer 和使用 saveLayer 进行绘制之间的区别。）这种情况很少需要。可能需要这种情况的一种情形是：如果要在一个非常不同的背景颜色上叠加图像。但在这些情况下，考虑是否可以避免在一个地方叠加多种颜色（例如通过仅在图像缺席的地方存在背景颜色）。如果可以的话，抗锯齿就可以胜任并且速度更快。

&emsp;总结下来即根据裁剪情况和判断是否需要抗锯齿来使用哪个 Clip 的值，当处理圆或者弧时开启抗锯齿用 Clip.antiAlias，其它只需要裁剪不需要抗锯齿时使用 Clip.hardEdge 即可。

&emsp;然后是 `_clipAndPaint` 的第 3 个参数 Rect bounds，当使用 Clip.antiAliasWithSaveLayer 裁剪行为时作为 canvas.saveLayer 函数的参数。第 4 个参数则作为一个裁剪完成后的回调。

```dart
  void _clipAndPaint(void Function(bool doAntiAlias) canvasClipCall,
                     Clip clipBehavior,
                     Rect bounds,
                     VoidCallback painter) {
                     
    // 首先对 canvas 当前状态进行保存，
    canvas.save();
    
    // 根据 clipBehavior 裁剪行为进行不同的裁剪
    switch (clipBehavior) {
      case Clip.none:
      
        // 完全无需裁剪
        break;
        
      case Clip.hardEdge:
      
        // 仅需要进行裁剪，并不进行抗锯齿，减少不必要的性能损耗
        canvasClipCall(false);
        
      case Clip.antiAlias:
      
        // 进行裁剪并且进行抗锯齿
        canvasClipCall(true);
        
      case Clip.antiAliasWithSaveLayer:
      
        // 进行裁剪并且进行抗锯齿，
        // 并且 执行 canvas.saveLayer，使得...
        canvasClipCall(true);
        canvas.saveLayer(bounds, Paint());
    }
    
    // canvas 进行裁剪后的回调
    painter();
    
    // 如果裁剪行为是 抗锯齿并且保存 Layer，则要多一次 canvas.restore，
    // 它使得 
    if (clipBehavior == Clip.antiAliasWithSaveLayer) {
      canvas.restore();
    }
    
    // 
    canvas.restore();
  }
```

&emsp;OK，`_clipAndPaint` 这个简单的工具函数看完了，看下下面都会使用它进行哪些裁剪行为。

### clipPathAndPaint

&emsp;根据 Clip clipBehavior 和 Path path 对 canvas 进行裁剪，然后进行绘制。在此之后，canvas 将恢复到裁剪之前的状态。（Rect bounds 参数是用于当 Clip clipBehavior 参数值是 Clip.antiAliasWithSaveLayer 时，Canvas.saveLayer 函数的边界。）

&emsp;可看到内部直接使用 Canvas.clipPath 函数。

```dart
  void clipPathAndPaint(Path path, Clip clipBehavior, Rect bounds, VoidCallback painter) {
  
    // 直接使用 canvas.clipPath(path, doAntiAlias: doAntiAlias)
    _clipAndPaint((bool doAntiAlias) => canvas.clipPath(path, doAntiAlias: doAntiAlias), clipBehavior, bounds, painter);
    
  }
```

### clipRRectAndPaint

&emsp;根据 Clip clipBehavior 和 RRect rrect 对 canvas 进行裁剪，然后进行绘制。在此之后，canvas 将恢复到裁剪之前的状态。（Rect bounds 参数是用于当 Clip clipBehavior 参数值是 Clip.antiAliasWithSaveLayer 时，Canvas.saveLayer 函数的边界。）

&emsp;可看到内部直接使用 Canvas.clipRRect 函数。

```dart
  void clipRRectAndPaint(RRect rrect, Clip clipBehavior, Rect bounds, VoidCallback painter) {
  
    // 直接使用 canvas.clipRRect(rrect, doAntiAlias: doAntiAlias)
    _clipAndPaint((bool doAntiAlias) => canvas.clipRRect(rrect, doAntiAlias: doAntiAlias), clipBehavior, bounds, painter);
    
  }
```

### clipRectAndPaint

&emsp;根据 Clip clipBehavior 和 Rect rect 对 canvas 进行裁剪，然后进行绘制。在此之后，canvas 将恢复到裁剪之前的状态。（Rect bounds 参数是用于当 Clip clipBehavior 参数值是 Clip.antiAliasWithSaveLayer 时，Canvas.saveLayer 函数的边界。）

&emsp;可看到内部直接使用 Canvas.clipRect 函数。

```dart
  void clipRectAndPaint(Rect rect, Clip clipBehavior, Rect bounds, VoidCallback painter) {
  
    // 直接使用 canvas.clipRect(rect, doAntiAlias: doAntiAlias)
    _clipAndPaint((bool doAntiAlias) => canvas.clipRect(rect, doAntiAlias: doAntiAlias), clipBehavior, bounds, painter);
    
  }
```

&emsp;OK，到这里 ClipContext 抽象类的内容就看完了，它确实是直接提供了三个非抽象的裁剪函数：根据路径裁剪、根据矩形裁剪、根据圆角矩形裁剪。作为 PaintContext 的裁剪工具，名副其实。下面我们则继续看 PaintContext 类的内容。

## Constructors

&emsp;创建一个 PaintingContext 对象。通常只会被 PaintingContext.repaintCompositedChild 和 pushLayer 调用。

```dart
class PaintingContext extends ClipContext {
  @protected
  PaintingContext(this._containerLayer, this.estimatedBounds);
  
  // ...
}
```

## `_containerLayer`

&emsp;

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

&emsp;静态函数。

&emsp;重绘给定的 RenderObject child。该 RenderObject 对象必须附加到一个 PipelineOwner，必须有一个合成的图层（composited layer），并且必须需要绘制（`child._needsPaint` 为 true）。该 RenderObject 对象的图层（如果有的话）会被重用，以及子树中不需要重新绘制的任何图层都将被重用。

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
    OffsetLayer? childLayer = child._layerHandle.layer as OffsetLayer?;
    
    if (childLayer == null) {

      final OffsetLayer layer = child.updateCompositedLayer(oldLayer: null);
      child._layerHandle.layer = childLayer = layer;
      
    } else {
      Offset? debugOldOffset;
      childLayer.removeAllChildren();
      
      final OffsetLayer updatedLayer = child.updateCompositedLayer(oldLayer: childLayer);
    }
    
    child._needsCompositedLayerUpdate = false;

    childContext ??= PaintingContext(childLayer, child.paintBounds);
    child._paintWithContext(childContext, Offset.zero);

    childContext.stopRecordingIfNeeded();
  }
```

## updateLayerProperties

&emsp;静态函数。

&emsp;更新 RenderObject child 的合成层而不重绘其子级。

&emsp;RenderObject 对象必须附加到 PipelineOwner，必须具有一个合成层，并且必须需要进行合成层的更新，但不需要进行绘制。RenderObject 对象的图层将会被重用，且不会导致任何子级重绘或它们的图层更新。

&emsp;另请参阅：

+ RenderObject.isRepaintBoundary，用于确定 RenderObject 是否具有合成层。

```dart
  static void updateLayerProperties(RenderObject child) {
    final OffsetLayer childLayer = child._layerHandle.layer! as OffsetLayer;
    
    final OffsetLayer updatedLayer = child.updateCompositedLayer(oldLayer: childLayer);
    child._needsCompositedLayerUpdate = false;
  }
```

## paintChild

&emsp;绘制子级 RenderObject。如果子级有自己的合成层，则该子级将合成到与此 PaintingContext 关联的 layer subtree 中。否则，该子级 RenderObject 将绘制到此 PaintingContext 的当前 PictureLayer 中。

```dart
  void paintChild(RenderObject child, Offset offset) {
    if (child.isRepaintBoundary) {
      stopRecordingIfNeeded();
      
      _compositeChild(child, offset);
      
    } else if (child._wasRepaintBoundary) {
      
      // 如果之前是绘制边界，但是现在不是了，需要把之前的合成层释放。
      child._layerHandle.layer = null;
      
      // 
      child._paintWithContext(this, offset);
    } else {
      // 
      child._paintWithContext(this, offset);
    }
  }
  
  void _compositeChild(RenderObject child, Offset offset) {
    // 为我们的 RenderObject child 创建一个图层，并将子组件绘制在图层中。
    if (child._needsPaint || !child._wasRepaintBoundary) {
      repaintCompositedChild(child, debugAlsoPaintedParent: true);
    } else {
      if (child._needsCompositedLayerUpdate) {
        updateLayerProperties(child);
      }
    }

    final OffsetLayer childOffsetLayer = child._layerHandle.layer! as OffsetLayer;
    childOffsetLayer.offset = offset;
    
    appendLayer(childOffsetLayer);
  }
```

## appendLayer

&emsp;向记录添加一个层，需要确保记录已经停止。请不要直接调用这个函数：而是应该调用 addLayer 或 pushLayer。当所有不是从画布生成的层都被添加时，这个函数会在内部被调用。

&emsp;需要自定义如何添加层的子类应该重写这个方法。

```dart
  @protected
  void appendLayer(Layer layer) {
    layer.remove();
    _containerLayer.append(layer);
  }
```

## `_isRecording`

&emsp;

```dart
  bool get _isRecording {
    final bool hasCanvas = _canvas != null;
    return hasCanvas;
  }
```

## canvas

&emsp;用于绘制的画布。当使用此上下文绘制子级时，当前画布可能会改变，这意味着保持对此 getter 返回的画布的引用是不稳定的。

```dart
  // Recording state
  PictureLayer? _currentLayer;
  ui.PictureRecorder? _recorder;
  Canvas? _canvas;

  /// The canvas on which to paint.
  ///
  /// The current canvas can change whenever you paint a child using this
  /// context, which means it's fragile to hold a reference to the canvas
  /// returned by this getter.
  @override
  Canvas get canvas {
    if (_canvas == null) {
      _startRecording();
    }
    
    return _canvas!;
  }

  void _startRecording() {
    assert(!_isRecording);
    
    _currentLayer = PictureLayer(estimatedBounds);
    _recorder = RendererBinding.instance.createPictureRecorder();
    _canvas = RendererBinding.instance.createCanvas(_recorder!);
    _containerLayer.append(_currentLayer!);
  }
```

## addCompositionCallback

&emsp;为当前上下文使用的 ContainerLayer 添加 CompositionCallback。当包含当前绘制上下文的层树进行合成或被分离且不会再次呈现时，将调用合成回调。无论层是通过保留渲染还是其他方式添加的，都会发生这种情况。

&emsp;合成回调对于推送一个本应尝试观察层树但实际上不影响合成的层很有用。例如，合成回调可用于观察当前容器层的总变换和裁剪，以确定绘制到其中的渲染对象是否可见。

&emsp;调用返回的回调将从合成回调中移除回调。

```dart
  VoidCallback addCompositionCallback(CompositionCallback callback) {
    return _containerLayer.addCompositionCallback(callback);
  }
```

## stopRecordingIfNeeded

&emsp;如果已经开始录制，请停止向画布录制。请不要直接调用此函数：此类中的函数会根据需要调用此方法。此函数会在内部调用以确保在添加图层或完成绘制结果之前停止录制。需要定制如何将录制保存到画布的子类应重写此方法以保存自定义画布录制的结果。

```dart
  @protected
  @mustCallSuper
  void stopRecordingIfNeeded() {
    if (!_isRecording) {
      return;
    }
    
    assert(() {
      if (debugRepaintRainbowEnabled) {
        final Paint paint = Paint()
          ..style = PaintingStyle.stroke
          ..strokeWidth = 6.0
          ..color = debugCurrentRepaintColor.toColor();
        canvas.drawRect(estimatedBounds.deflate(3.0), paint);
      }
      if (debugPaintLayerBordersEnabled) {
        final Paint paint = Paint()
          ..style = PaintingStyle.stroke
          ..strokeWidth = 1.0
          ..color = const Color(0xFFFF9800);
        canvas.drawRect(estimatedBounds, paint);
      }
      return true;
    }());
    _currentLayer!.picture = _recorder!.endRecording();
    _currentLayer = null;
    _recorder = null;
    _canvas = null;
  }

```







# PictureRecorder

&emsp;PictureRecorder：记录包含一系列图形操作（graphical operations）的图片（Picture）。

&emsp;要开始记录，构建一个 Canvas 来记录命令（即以一个 PictureRecorder 对象为参数构建一个 Canvas 对象）。要结束记录，使用 PictureRecorder.endRecording 方法。

&emsp;OK，下面看一下 PictureRecorder 的源码。

## Constructors

&emsp;创建一个新的空闲 PictureRecorder。要将其与 Canvas 对象关联并开始记录，将此 PictureRecorder 传递给 Canvas 构造函数即可。

&emsp;注意 PictureRecorder 是一个抽象类，是无法直接创建实例对象使用的，但是看这里：PictureRecorder() 已经被声明为一个工厂函数，它会直接返回一个 `_NativePictureRecorder` 对象，`_NativePictureRecorder` 是一个实现了 PictureRecorder 的子类，所以这里其实是直接返回一个 `_NativePictureRecorder` 实例对象，而并不是说拿着 PictureRecorder 抽象类来构建实例对象使用。

```dart
abstract class PictureRecorder {
  factory PictureRecorder() = _NativePictureRecorder;
  
  // ...
```

## isRecording

&emsp;表示这个 PictureRecorder 对象当前是否正在记录命令（Canvas 的绘制操作（graphical operations）或者 绘图命令）。

&emsp;具体来说，如果 Canvas 对象已经被创建用于记录命令且尚未通过调用 endRecording 方法结束记录，则返回 true；如果这个 PictureRecorder 尚未与 Canvas 相关联，或者 endRecording 方法已经被调用，则返回 false。

```dart
  bool get isRecording;
```

## endRecording

&emsp;完成记录图形操作（graphical operations），即结束图形操作（graphical operations）记录。

&emsp;返回一个包含到目前为止已记录的图形操作（graphical operations）的图片（一个 Picture 对象）。调用此函数后，PictureRecorder 对象和 Canvas 对象都会失效，无法继续使用。

```dart
  Picture endRecording();
```

# Picture

&emsp;Picture：一个表示一系列记录的绘图操作的对象。

&emsp;要创建一个 Picture，可以使用 PictureRecorder。

&emspp可以使用 SceneBuilder 将 Picture 放置在一个 Scene 中，通过 SceneBuilder.addPicture 方法。也可以使用 Canvas.drawPicture 方法将 Picture 绘制到 Canvas 中。

&emsp;OK，下面看一下 Picture 的源码。

## onCreate

&emsp;⚠️ 注意这是一个 Picture 类的静态属性。

&emsp;一个被调用来报告 Picture 创建的回调函数。（在 Flutter 的 flutter/foundation.dart 中更倾向于使用 MemoryAllocations，而不是直接使用 onCreate，因为 MemoryAllocations 允许多个回调函数。）

```dart
// Picture 生命周期事件签名，入参为 Picture 返回值是 void
typedef PictureEventCallback = void Function(Picture picture);
```

```dart
  static PictureEventCallback? onCreate;
```

## onDispose

&emsp;⚠️ 注意这是一个 Picture 类的静态属性。

&emsp;一个被调用来报告 Picture 释放的回调函数。

```dart
  static PictureEventCallback? onDispose;
```

## toImage

&emsp;异步的从这个 Picture 对象创建一张图像（Image）。

&emsp;返回的 Image 将具有 width 个像素宽和 height 个像素高。该 Image 在 0（left），0（top），width（right），height（bottom）边界内被栅格化。超出这些边界的内容将被裁剪。

&emsp;注意返回值是 Future，这是一个异步生成 Image 对象。下面还有一个同步的生成 Image 对象。

```dart
  Future<Image> toImage(int width, int height);
```

## toImageSync

&emsp;同步地从这个 Picture 对象创建一张图像（Image）。

&emsp;返回的 Image 将具有 width 个像素宽和 height 个像素高。该 Image 在 0（left），0（top），width（right），height（bottom）边界内被栅格化。超出这些边界的内容将被裁剪。

&emsp;Image 对象是同步创建和返回的，但是是异步栅格化的。如果栅格化失败，当 Image 绘制到 Canvas 上时会抛出异常。

&emsp;如果有 GPU 上下文可用，此 Image 将被创建为 GPU 驻留，并且不会被复制回 host。这意味着绘制此 Image 将更高效。

&emsp;如果没有 GPU 上下文可用，该 Image 将在 CPU 上栅格化。

```dart
  Image toImageSync(int width, int height);
```

## dispose

&emsp;调用此方法释放该 Picture 对象使用的资源。在调用此方法后，该 Picture 对象将不再可用。

```dart
  void dispose();
```

## approximateBytesUsed

&emsp;返回为此 Picture 对象分配的大约字节数。实际 Picture 的大小可能会更大，特别是如果它包含对 image 或其他大对象的引用。

```dart
  int get approximateBytesUsed;
```

## Picture 总结

&emsp;















## 参考链接
**参考链接:🔗**
+ [Canvas class](https://api.flutter.dev/flutter/dart-ui/Canvas-class.html)
+ [PictureRecorder class](https://api.flutter.dev/flutter/dart-ui/PictureRecorder-class.html)
+ [Picture class](https://api.flutter.dev/flutter/dart-ui/Picture-class.html)
+ [Flutter 3.7 之快速理解 toImageSync 是什么？能做什么？](https://guoshuyu.cn/home/wx/Flutter-N18.html)
+ [Clip enum](https://api.flutter.dev/flutter/dart-ui/Clip.html)
