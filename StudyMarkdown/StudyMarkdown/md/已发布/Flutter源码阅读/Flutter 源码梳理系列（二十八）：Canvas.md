# Flutter 源码梳理系列（二十八）：Canvas

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


## 参考链接
**参考链接:🔗**
+ [Canvas class](https://api.flutter.dev/flutter/dart-ui/Canvas-class.html)
+ [PictureRecorder class](https://api.flutter.dev/flutter/dart-ui/PictureRecorder-class.html)
+ [Picture class](https://api.flutter.dev/flutter/dart-ui/Picture-class.html)
+ [Flutter 3.7 之快速理解 toImageSync 是什么？能做什么？](https://guoshuyu.cn/home/wx/Flutter-N18.html)
