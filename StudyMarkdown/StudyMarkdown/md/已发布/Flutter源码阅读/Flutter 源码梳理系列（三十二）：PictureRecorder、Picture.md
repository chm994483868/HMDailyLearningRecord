# Flutter 源码梳理系列（三十二）：PictureRecorder、Picture

&emsp;在前面学习时，我们从来没有详细讲解过 PictureRecorder 和 Picture 的内容。其实主要是鉴于 PictureRecorder 和 Picture 是 Flutter framework 层到 Engine 层的桥接，它们真正的功能实现是在 Engine 层，在 Framework 层只能简单浏览一下它们的 API 看它们实现了什么功能而已。

&emsp;与 Canvas 类似，PictureRecorder、Picture 的实现类分别是：`_NativePictureRecorder` 和 `_NativePicture`。

# PictureRecorder

&emsp;PictureRecorder：记录包含一系列图形操作（graphical operations）的图片（Picture）。

&emsp;要开始记录，构建一个 Canvas 来记录命令（即以一个 PictureRecorder 对象为参数构建一个 Canvas 对象）。要结束记录，则使用 PictureRecorder.endRecording 方法。

&emsp;如 PaintingContext 类中的 `_startRecording` 函数内部一样，当构建 Canvas 对象时，需要传入 `ui.PictureRecorder? _recorder`。

```dart
  void _startRecording() {
  
    // ...
    _recorder = RendererBinding.instance.createPictureRecorder();
    _canvas = RendererBinding.instance.createCanvas(_recorder!);
    // ...
  }
```

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

### `_NativePictureRecorder.endRecording`

&emsp;在 `_NativePictureRecorder` 类中 endRecording 函数有一部分我们可以看懂的实现，如下：

```dart
  @override
  Picture endRecording() {
    // _NativePictureRecorder 有一个 _NativeCanvas? _canvas，即它会持有使用自己为参数创建的 Canvas，
    // 并且新创建的 Canvas 也会持有这个 _NativePictureRecorder，它俩会双向持有。
    
    // 如果 _canvas 属性为 null，则直接抛错，毕竟一个 _NativePictureRecorder 对象连 Canvas 都没有，
    // 就想生成 Picture，有点想多了，直接抛错即可。
    // 这也提示我们没有和 Canvas 绑定时是不能调用 _NativePictureRecorder 对象的 endRecording 函数的。
    if (_canvas == null) {
      throw StateError('PictureRecorder did not start recording.');
    }
    
    // 然后接下来为生成 Picture 对象做准备。
    
    // 准备一个 _NativePicture 对象，然后调用在 Engine 层实现的 _endRecording 函数，
    // 处理 _NativePicture 对象的事宜。
    final _NativePicture picture = _NativePicture._();
    _endRecording(picture);
    
    // 生成 _NativePicture 对象以后就无事了，
    // 就断开 _NativeCanvas 和 _NativePictureRecorder 的相互引用，
    // 没有了 _canvas 那么 _recorder 也要作废了。
    _canvas!._recorder = null;
    _canvas = null;
    
    // 我们在这里调用处理程序，而不是在 Picture 构造函数中调用，因为我们希望通过处理程序可以使用 picture.approximateBytesUsed。
    
    // 这里会回调一个 Picture 类的静态回调 onCreate。
    // 还有一个 Picture 类的静态回调 onDispose 函数，会在 Picture 类的 dispose 函数中，
    // 进行回调：Picture.onDispose?.call(this);
    Picture.onCreate?.call(picture);
    
    return picture;
  }
```

## PictureRecorder 总结

&emsp;PictureRecorder 的 API 内容很少，到这里就结束了，我们只要记住两个点就可以了：

1. 搭配 Canvas 使用，当创建 Canvas 对象时需要传入一个 PictureRecorder 对象。
2. 它的 endRecording 方法会把记录到的所有图形操作生成一个 Picture 对象，并且在此函数调用后：Canvas 和 PictureRecorder 对象就都失效了。

&emsp;OK，下面我们看一下 Picture 的内容。

# Picture

&emsp;Picture：一个表示一系列记录的绘图操作的对象。

&emsp;要创建一个 Picture，可以使用 PictureRecorder。

&emsp;可以使用 SceneBuilder 将 Picture 放置在一个 Scene 中，通过 SceneBuilder.addPicture 方法。也可以使用 Canvas.drawPicture 方法将 Picture 绘制到 Canvas 中。

&emsp;OK，下面看一下 Picture 的源码。

## onCreate

&emsp;⚠️ 注意这是一个 Picture 类的静态属性。

&emsp;一个被调用来报告 Picture 创建的回调函数。（在 Flutter 的 flutter/foundation.dart 中更倾向于使用 MemoryAllocations，而不是直接使用 onCreate，因为 MemoryAllocations 允许多个回调函数。）

&emsp;此回调可以在 `_NativePictureRecorder` 的 endRecording 函数中看到被调用了：`Picture.onCreate?.call(picture);`。

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

&emsp;除了两个 Picture 类的静态回调：onCreate 和 onDispose 外，还有一对异步/同步生成 Image 的接口，关于此一对接口可以先通过大佬的文章学习：[Flutter 3.7 之快速理解 toImageSync 是什么？能做什么？](https://guoshuyu.cn/home/wx/Flutter-N18.html)。

## 参考链接
**参考链接:🔗**
+ [Canvas class](https://api.flutter.dev/flutter/dart-ui/Canvas-class.html)
+ [PictureRecorder class](https://api.flutter.dev/flutter/dart-ui/PictureRecorder-class.html)
+ [Picture class](https://api.flutter.dev/flutter/dart-ui/Picture-class.html)
+ [Flutter 3.7 之快速理解 toImageSync 是什么？能做什么？](https://guoshuyu.cn/home/wx/Flutter-N18.html)
