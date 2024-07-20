# Flutter 源码梳理系列（二十八）：PaintingContext

# PaintingContext

&emsp;PaintingContext：一个画画的地方。

&emsp;与直接持有 Canvas 不同，RenderObjects 使用 PaintingContext 进行绘制。PaintingContext 中包含一个 Canvas，用于接收各个绘制操作（draw operations），并且还具有用于绘制子级 RenderObject 的函数（PaintingContext：void paintChild(RenderObject child, Offset offset)）。

&emsp;在绘制子级 RenderObject 时，由 PaintingContext 持有的 Canvas 可能会发生变化，因为在绘制子级 RenderObject 之前和之后发出的绘制操作（draw operations）可能会记录在单独的合成层（compositing layers）中。因此，在可能绘制子级 RenderObject 的操作之间不要保留对 Canvas 的引用。（这里牵涉到和 relayoutBoundary 类似的 repaintBoundary 知识点，我们后面再展开。）

&emsp;当使用 PaintingContext.repaintCompositedChild 和 pushLayer 时，会自动创建新的 PaintingContext 对象。

+ Object -> ClipContext -> PaintingContext

## Constructors

&emsp;







# Canvas

&emsp;Canvas：一个用于记录图形操作（graphical operations）的接口。

&emsp;Canvas 对象用于创建 Picture 对象，而 Picture 对象本身可以与 SceneBuilder 一起使用来构建 Scene。然而，在正常的使用中，这一切都由 Flutter framework 处理。

&emsp;一个 canvas 具有一个当前的 transformation 矩阵，该矩阵应用于所有操作。最初，transformation 矩阵是 identity 矩阵。可以使用 translate、scale、rotate、skew 和 transform 方法来修改它。

&emsp;一个 canvas 还有一个当前的裁剪区域（clip region），该区域会应用到所有的操作中。初始情况下，裁剪区域（clip region）是无限的。可以通过使用 clipRect、clipRRect 和 clipPath 方法来修改它。

&emsp;当前的 transform 和 clip 可以通过由 save、saveLayer 和 restore 方法管理的堆栈进行保存和恢复。

## Use with the Flutter framework

&emsp;Flutter framework 中的 RendererBinding 提供了一个 hook 来创建 Canvas 对象 （RendererBinding.createCanvas），这使得测试能够 hook 到 scene 创建逻辑中。当创建一个将与 Scene 中的 PictureLayer 一起使用的 Canvas 时，在 Flutter framework 的上下文中考虑调用 RendererBinding.createCanvas 而不是直接调用 Canvas.new 构造函数。

&emsp;当使用 canvas 生成用于其他目的的位图（bitmap）时，例如使用 Picture.toImage 生成 PNG 图像时，则不适用这一点。

# Constructors

&emsp;创建一个 canvas，用于将图形操作（graphical operations）记录到给定的 PictureRecorder 中。

&emsp;影响完全位于给定的 cullRect 区域之外的像素的图形操作可能会被实现丢弃。然而，如果一条指令部分在 cullRect 内部和外部绘制，实现可能会在这些边界外绘制。为了确保丢弃给定区域外的像素，考虑使用 clipRect。cullRect 是可选的；默认情况下，所有操作都被保留。

&emsp;要结束 PictureRecorder 的记录过程，请调用给定 PictureRecorder 的 PictureRecorder.endRecording。

```dart
abstract class Canvas {
  factory Canvas(PictureRecorder recorder, [ Rect? cullRect ]) = _NativeCanvas;
  
  // ...
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
