# Flutter 源码梳理系列（二十八）：Canvas

# Canvas

&emsp;Canvas：一个用于记录图形操作（graphical operations）的抽象类（接口）。

&emsp;Canvas 对象用于创建 Picture 对象，而 Picture 对象本身可以与 SceneBuilder 一起使用来构建 Scene。然而，在正常的使用中，这一切都由 Flutter framework 处理。（Picture 由创建 Canvas 对象时传入的 PictureRecorder 对象，调用 endRecording 产生。当在 Canvas 上完成一系列绘制后，调用 PictureRecorder.endRecording 结束绘制，便可得到 Picture 对象。）

&emsp;一个 canvas 具有一个当前的 transformation 矩阵（通过 Canvas 的 Float64List getTransform 函数可取得。），该矩阵应用于所有操作。最初，transformation 矩阵是 identity 矩阵。可以使用 translate、scale、rotate、skew 和 transform 方法来修改它。（即实现 Canvas 的平移、缩放、旋转 等操作。）

&emsp;一个 canvas 还有一个当前的裁剪区域（clip region）（通过 Canvas 的 Rect getLocalClipBounds 函数可取得。），该区域会应用到所有的操作中。初始情况下，裁剪区域（clip region）是无限的。可以通过使用 clipRect、clipRRect 和 clipPath 方法来修改它。

&emsp;当前的 transform 和 clip 可以通过由 save、saveLayer 和 restore 方法管理的堆栈进行保存和恢复。

## Use with the Flutter framework

&emsp;Flutter framework 中的 RendererBinding 提供了一个 hook 来创建 Canvas 对象 （RendererBinding.createCanvas），这使得测试能够 hook 到 scene 创建逻辑中。当创建一个将与 Scene 中的 PictureLayer 一起使用的 Canvas 时，在 Flutter framework 的 PaintingContext 中考虑调用 RendererBinding.createCanvas 而不是直接调用 Canvas.new 构造函数。（如在 PaintingContext 中，它 canvas 属性就是通过：`_canvas = RendererBinding.instance.createCanvas(_recorder!);` 创建的。）

&emsp;当使用 canvas 生成用于其他目的的位图（bitmap）时，例如使用 Picture.toImage 生成 PNG 图像时，则不适用这一点。

## Constructors

&emsp;创建一个 canvas，用于将图形操作（graphical operations）记录到给定的 PictureRecorder 中。

&emsp;影响完全位于给定的 cullRect 区域之外的像素的图形操作（graphical operations）可能会被实现丢弃。然而，如果一条指令部分在 cullRect 内部和外部绘制，实现可能会在这些边界外绘制。为了确保丢弃给定区域外的像素，考虑使用 clipRect。cullRect 是可选的；默认情况下，所有操作都被保留。

&emsp;要结束 PictureRecorder 的记录过程，请调用给定 PictureRecorder 的 PictureRecorder.endRecording。

```dart
abstract class Canvas {
  factory Canvas(PictureRecorder recorder, [ Rect? cullRect ]) = _NativeCanvas;
  
  // ...
}
```

&emsp;注意到了吗？Canvas 的同名工厂构造函数返回的是 `_NativeCanvas`，它的定义如下：`base class _NativeCanvas extends NativeFieldWrapperClass1 implements Canvas { // ... }`，它同样是一个实现了 Canvas 所有抽象函数并继承自 NativeFieldWrapperClass1 的 base class，它的实现全部由 native 实现。所以，我们暂时不分心去看它的内容，我们只要专注于 Canvas 都提供了哪些功能即可。（Canvas 抽象类中所有的函数都是抽象函数。）

## save

&emsp;在保存栈（save stack）上保存当前 transform 和 clip 的副本。调用 restore 以弹出保存栈（save stack）。

&emsp;另请参阅：

&emsp;saveLayer 也会执行相同的操作，同时还会将命令进行分组，直到匹配的 restore 出现为止。

```dart
  void save();
```

## saveLayer

&emsp;在保存栈（save stack）中保存 current transform 和 clip 的副本，然后创建一个新的组，随后的调用将成为该组的一部分。当稍后弹出保存栈（save stack）时，该组将被铺平成一个层（layer），并应用给定 Paint 对象的 Paint.colorFilter 和 Paint.blendMode。

&emsp;这允许我们创建复合效果（composite effects），例如使一组绘图命令（drawing commands）半透明。如果不使用 saveLayer，组中的每个部分将被单独绘制，因此它们重叠的部分将比它们不重叠的部分更暗。通过使用 saveLayer 将它们分组在一起，它们可以首先用不透明颜色绘制，然后可以使用 saveLayer 的 Paint 使整个组变为透明。

&emsp;调用 restore 以弹出保存栈（save stack）并将 Paint 应用到该组。

### Using saveLayer with clips

&emsp;当矩形裁剪操作（来自 clipRect）与光栅缓冲区（raster buffer）不是轴对齐的，或者当裁剪操作不是矩形的（例如因为是由 clipRRect 创建的圆角矩形裁剪，或者是由 clipPath 创建的任意复杂路径裁剪），裁剪的边缘需要进行抗锯齿处理。

&emsp;如果两个绘制调用在这样一个被裁剪区域的边缘重叠，没有使用 saveLayer，第一次绘制将首先与背景进行抗锯齿处理，然后第二次将与第一次绘制和背景混合后的结果进行抗锯齿处理。另一方面，如果在建立裁剪后立即使用 saveLayer，第二次绘制将覆盖第一次绘制的图层，因此当图层被剪切和合成时（调用 restore 时），第二次绘制将仅与背景进行抗锯齿处理。

&emsp;例如，这个 CustomPainter.paint 方法绘制了一个干净的白色圆角矩形：

```dart
void paint(Canvas canvas, Size size) {
  Rect rect = Offset.zero & size;
  
  canvas.save();
  canvas.clipRRect(RRect.fromRectXY(rect, 100.0, 100.0));
  canvas.saveLayer(rect, Paint());
  
  canvas.drawPaint(Paint()..color = Colors.red);
  canvas.drawPaint(Paint()..color = Colors.white);
  canvas.restore();
  canvas.restore();
}
```

&emsp;另一方面，此部件呈现红色轮廓，这是由红色绘制物与裁剪边缘处的背景进行抗锯齿处理后，然后白色绘制物与包括被裁剪的红色绘制物在内的背景进行类似的抗锯齿处理的结果。

```dart
void paint(Canvas canvas, Size size) {
  // (this example renders poorly, prefer the example above)
  Rect rect = Offset.zero & size;
  
  canvas.save();
  canvas.clipRRect(RRect.fromRectXY(rect, 100.0, 100.0));
  
  canvas.drawPaint(Paint()..color = Colors.red);
  canvas.drawPaint(Paint()..color = Colors.white);
  canvas.restore();
}
```

&emsp;如果裁剪只裁剪一个绘制操作，则这一点就无关紧要了。例如，以下 paint 方法绘制了一对干净的白色圆角矩形，即使裁剪不是在单独的图层上完成的：

```dart
void paint(Canvas canvas, Size size) {
  canvas.save();
  
  canvas.clipRRect(RRect.fromRectXY(Offset.zero & (size / 2.0), 50.0, 50.0));
  canvas.drawPaint(Paint()..color = Colors.white);
  
  canvas.restore();
  
  canvas.save();
  
  canvas.clipRRect(RRect.fromRectXY(size.center(Offset.zero) & (size / 2.0), 50.0, 50.0));
  canvas.drawPaint(Paint()..color = Colors.white);
  canvas.restore();
}
```

&emsp;（顺便提一句，与其像这样使用 clipRRect 和 drawPaint 来绘制圆角矩形，不如选择使用 drawRRect 方法。这些示例正在使用 drawPaint 作为 "complicated draw operations that will get clipped" 的代理，以说明这一点。）

### Performance considerations

&emsp;一般来说，saveLayer 的成本相对较高。

&emsp;GPU（图形处理器，负责处理图形的硬件）有多种不同的硬件架构，但大多涉及批处理命令并对其进行重新排序以提高性能。 当使用图层（layers）时，会导致渲染管线（rendering pipeline）必须切换渲染目标（从一个图层到另一个）。 渲染目标切换会刷新 GPU 的命令缓冲区，这通常意味着无法获得更大批处理的优化。 渲染目标切换还会生成大量的内存波动，因为 GPU 需要将当前帧缓冲区内容从专为写入优化的内存部分复制出来，然后在还原前一个渲染目标（图层）时需要将其复制回去。

&emsp;另请参阅：

+ save，用于保存当前状态，但不为后续命令创建新图层。
+ BlendMode，讨论了使用 Paint.blendMode 与 saveLayer 的情况。

```dart
  void saveLayer(Rect? bounds, Paint paint);
```

## restore

&emsp;如果有内容要弹出，则弹出当前保存栈（save stack）。否则，不执行任何操作。

&emsp;使用 save 和 saveLayer 将 state push 到栈上。如果 state 是使用 saveLayer pushed 的，则此调用还将导致新图层（new layer）合成到上一个图层中。

```dart
  void restore();
```

## restoreToCount

&emsp;将保存栈（save stack）还原到之前的层级，就像从 getSaveCount 获取到的那样。如果 count 小于 1，则栈将被还原到初始状态。如果 count 大于当前的 getSaveCount，则不会发生任何操作。

&emsp;使用 save 和 saveLayer 将状态推入栈中。

&emsp;如果通过此调用恢复的状态堆栈级别中有任何是通过 saveLayer 推送的，则此调用还将导致这些层被合成到它们以前的层中。

```dart
  void restoreToCount(int count);
```

## getSaveCount

&emsp;返回保存栈上的项目数量，包括初始状态。这意味着对于一个干净的 canvas，它返回 1，每个 save 和 saveLayer 调用都会递增它，并且每个匹配的 restore 调用都会递减它。

&emsp;这个数字不会低于1。

```dart
  int getSaveCount();
```

## translate & scale & rotate & skew & transform

&emsp;下面一组函数，即对当前的 Canvas 整体进行：平移、缩放、旋转等操作。

&emsp;translate：将 current transform 添加到 translation 中，通过第一个参数 dx 水平移动坐标空间，通过第二个参数 dy 垂直移动坐标空间。（即在 x 轴和 y 轴平移。）

&emsp;scale：将一个与坐标轴对齐的缩放添加到 current transform 中，水平方向按第一个参数缩放，垂直方向按第二个参数缩放。如果未指定 sy，则 sx 将用于在两个方向上进行缩放。（即在 x 轴和 y 轴变大或者缩小。）

&emsp;rotate：将 current transform 添加旋转。参数为顺时针弧度。（即旋转 Canvas。）

&emsp;skew：在 current transform 中添加一个轴对齐的错切，第一个参数是水平方向上以距离单位顺时针围绕原点的倾斜，第二个参数是垂直方向上以距离单位顺时针围绕原点的倾斜。

&emsp;transform：将 current transform 乘以指定的 4⨉4 变换矩阵，该矩阵以列主序列为顺序以值列表的形式指定。

```dart
  void translate(double dx, double dy);
  void scale(double sx, [double? sy]);
  void rotate(double radians);
  void skew(double sx, double sy);
  void transform(Float64List matrix4);
```

## getTransform

&emsp;返回 current transform，包括自创建此 Canvas 对象以来执行的所有 transform 方法的组合结果，并遵循 save/restore 历史记录。

&emsp;可以改变 current transform 的方法包括 translate、scale、rotate、skew 和 transform。通过 restore 与其关联的 save 或 saveLayer 调用之前的相同值，restore 方法也可以修改 current transform。

```dart
  Float64List getTransform();
```

## clipRect

&emsp;将 clip region 减少到当前 clip region 和给定矩形（Rect rect）的交集部分。

![image.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/80a7fcac29744edf85498b028898c3c7~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1721702423&x-orig-sign=wW2ZuED7VoP1iU6ItbWaDLORKc0%3D)

&emsp;如果 doAntiAlias 为 true，则裁剪将会进行抗锯齿处理。

&emsp;如果多个绘制命令与裁剪边界相交，这可能会导致在裁剪边界处发生错误的混合。

&emsp;使用 ClipOp.difference 从 current clip 中减去提供的 Rect rect。

```dart
  void clipRect(Rect rect, { ClipOp clipOp = ClipOp.intersect, bool doAntiAlias = true });
```

### ClipOp enum

&emsp;定义新的 clip region 如何与现有的 clip region 合并。Canvas.clipRect 使用该属性。

+ ClipOp.difference：将新区域从现有区域中减去。
+ ClipOp.intersect：从现有区域中得到与新区域相交的部分。

## clipRRect

&emsp;将 clip region 减少为当前 clip region 与给定圆角矩形的交集。

![image.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/fddd5bfe1db2406481c4b192012c6881~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1721702722&x-orig-sign=HFUs86zLGVE8p5%2FImWs6Jfefcdk%3D)

&emsp;如果 doAntiAlias 为 true，则裁剪将会进行抗锯齿处理。

&emsp;如果多个绘制命令与裁剪边界相交，这可能会导致在裁剪边界处发生错误的混合。

```dart
  void clipRRect(RRect rrect, {bool doAntiAlias = true});
```

## clipPath

&emsp;将 clip region 减小为当前 clip region 与给定路径的交集。

![image.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/985f0fd12c64428f968a2f42c2f2b568~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1721703020&x-orig-sign=ZSAbPU2O0K9%2BGVYXHBXV8G3%2Fyes%3D)

&emsp;如果 doAntiAlias 为 true，则裁剪将会进行抗锯齿处理。

```dart
  void clipPath(Path path, {bool doAntiAlias = true});
```

## getLocalClipBounds

&emsp;返回在当前 Canvas 对象的保存栈内执行的所有剪切方法组合结果的保存边界，以本地坐标空间计量，即在当前进行渲染操作的本地坐标空间下。

&emsp;组合的剪切结果在转回本地坐标空间之前会被舍入到整数像素边界，这考虑了渲染操作中的像素舍入，尤其是在抗锯齿时。因为 Picture 最终可能会被渲染到转换小部件或层上下文中的场景中，因此由于过早的舍入，结果可能会过于保守。结合使用 getDestinationClipBounds 方法、外部转换和真实设备坐标系中的舍入，将产生更准确的结果，但此值可能提供一个更便利的近似值，用于比较渲染操作与已建立的剪切操作。

&emsp;边界的保守估计是基于执行 ClipOp.intersect 与每个剪切方法的边界相交，可能会忽略使用 ClipOp.difference 执行的任何剪切方法。ClipOp 参数仅在 clipRect 方法上存在。

&emsp;为了理解边界估计如何保守，请考虑以下两个剪切方法调用：

```dart
void draw(Canvas canvas) {
  canvas.clipPath(Path()
    ..addRect(const Rect.fromLTRB(10, 10, 20, 20))
    ..addRect(const Rect.fromLTRB(80, 80, 100, 100)));
    
  canvas.clipPath(Path()
    ..addRect(const Rect.fromLTRB(80, 10, 100, 20))
    ..addRect(const Rect.fromLTRB(10, 80, 20, 100)));
  // ...
}
```

&emsp;执行这两个调用后，由于这两个路径没有重叠区域，因此没有区域可用于绘制。但在这种情况下，getLocalClipBounds 将返回一个从 10，10 到 100，100 的矩形，因为它只与两个路径对象的边界相交以获得其保守估计。

&emsp;裁剪边界不受任何封闭的 saveLayer 调用的边界影响, 因为引擎目前在渲染过程中不保证对这些边界的严格执行。

&emsp;能够改变当前裁剪的方法包括 clipRect、clipRRect 和 clipPath。restore 方法也可以通过将其还原为与其相关的 save 或 saveLayer 调用之前的值来修改当前的裁剪。

```dart
  Rect getLocalClipBounds();
```

## getDestinationClipBounds

&emsp;

```dart
  Rect getDestinationClipBounds();
```

## drawColor

&emsp;使用给定的颜色在 Canvas 上绘制，应用给定的 BlendMode，其中给定的 color 作为 source color，背景作为 destination color。

```dart
  void drawColor(Color color, BlendMode blendMode);
```

## drawLine

&emsp;使用给定的 Paint paint 在给定的点（Offset p1 和 Offset p2）之间绘制一条线。该线是 stroked 的，对于这次调用，忽略 Paint.style 的值。p1 和 p2 参数被解释为相对于原点的偏移量。

![image.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/1722ac22ca784972b45db368b37af77f~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1721703947&x-orig-sign=Ef9hakoJ%2Bsm1bkozxwD9MmGY014%3D)

```dart
  void drawLine(Offset p1, Offset p2, Paint paint);
```

## drawPaint

&emsp;使用给定的 Paint paint 填充 canvas。

&emsp;要使用单一 Color color 和 BlendMode blendMode 填充 canvas，请考虑使用 drawColor 方法。

```dart
  void drawPaint(Paint paint);
```

## drawRect & drawRRect & drawDRRect

&emsp;drawRect：使用给定的 Paint 绘制一个矩形。矩形是填充的还是描边（或两者同时）由 Paint.style 控制。

![image.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/50f5f316f2bc4f419ce9b44c22a34661~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1721704508&x-orig-sign=6Pebw382cyRCmJmbnTvdOSsRXjA%3D)

&emsp;drawRRect：使用给定的 Paint 绘制一个圆角矩形。矩形是填充的还是描边（或两者同时）由 Paint.style 控制。

![image.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/113ffa3f2d67401b820782dda9b6e3e1~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1721704612&x-orig-sign=TFrQ77yy7Fx8Jiu2kCJry0chXFc%3D)

&emsp;drawDRRect：使用给定的 Paint 绘制由两个圆角矩形之间的差形成的形状。这个形状是填充的还是描边（或两者同时）由 Paint.style 控制。这个形状几乎但又完全不像一个环形。

```dart
  void drawRect(Rect rect, Paint paint);
  void drawRRect(RRect rrect, Paint paint);
  void drawDRRect(RRect outer, RRect inner, Paint paint);
```

## drawOval & drawCircle & drawArc

&emsp;drawOval：绘制一个轴对齐的椭圆，将给定的轴对齐矩形填满，并使用给定的 Paint paint 进行绘制。椭圆是填充的还是描边（或两者同时）由 Paint.style 控制。

![image.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/ce4a516d7e0b45a986337a392bc75c63~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1721704947&x-orig-sign=L5XbLJaIHtZJ6cQ3iC0b0nKJ8YU%3D)

&emsp;drawCircle：在给定的点 Offset c 为圆心，用第二个参数 double radius 指定的半径，用第三个参数指定的 Paint paint 绘制一个圆。圆是填充的还是描边（或两者同时）由 Paint.style 控制。

![image.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/a2dd8bd7c02d446eb41d5f6acb472a4c~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1721705070&x-orig-sign=vUt87vEHJIiYN61q%2BQNArwTcQtA%3D)

&emsp;drawArc：绘制一个按比例缩放以适应给定矩形 Rect rect 内部的弧线。它从椭圆上的 startAngle 弧度开始，沿椭圆到达 startAngle + sweepAngle 弧度，其中零弧度是椭圆右侧的点，穿过与矩形中心相交的水平线，正角度沿着椭圆顺时针旋转。 如果 useCenter 为 true，则将弧线闭合回中心，形成一个圆形扇区。 否则，弧线不闭合，形成一个圆形段。

&emsp;这种方法针对绘制弧线进行了优化，应该比 Path.arcTo 方法更快。

![image.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/bf2df539d145410b92021b2b1703679b~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1721705385&x-orig-sign=T6LlQ8ljHqlI40I4nuUnJCPzJf8%3D)

```dart
  void drawOval(Rect rect, Paint paint);
  void drawCircle(Offset c, double radius, Paint paint);
  void drawArc(Rect rect, double startAngle, double sweepAngle, bool useCenter, Paint paint);
```

## drawPath

&emsp;使用给定的 Paint paint 绘制给定的 Path path。

&emsp;绘制的形状是填充的还是描边（或两者同时）由 Paint.style 控制。如果 Path path 被填充，那么 Path path 内的子路径（sub-paths）会隐式关闭（参见 Path.close）。

```dart
  void drawPath(Path path, Paint paint);
```

## drawImage & drawImageRect & drawImageNine

&emsp;drawImage：使用给定的 Paint paint 将给定的 Image image 画在 canvas 上，其左上角位于给定的 Offset offset 处。图像使用给定的 Paint paint 混合到 canvas 中。

&emsp;drawImageRect：将由 Rect src 参数描述的给定图像 Image image 的子集绘制到由 Rect dst 参数指定的轴对齐矩形中的 canvas 中。这可能会通过应用滤镜的一半宽度的范围从 src 矩形的外部进行采样。对该方法进行多次调用并使用不同参数（来自相同图像）可以批量处理为一个调用 drawAtlas 以提高性能。

&emsp;drawImageNine：使用给定的 Paint paint，在 canvas 上绘制给定的图片 Image image。图片被分为九个部分，通过绘制两条水平线和两条垂直线来描述，其中 Rect center 参数描述了这四条线相交形成的矩形。（这形成了一个 3x3 的区域网格，中心区域由 Rect center 参数描述。）在目标矩形 Rect dst 描述的四个角落中，绘制四个角落的区域，不进行缩放。 剩下的五个区域通过拉伸来绘制，以使它们完全覆盖目标矩形 Rect dst，同时保持它们的相对位置。

```dart
  void drawImage(Image image, Offset offset, Paint paint);
  void drawImageRect(Image image, Rect src, Rect dst, Paint paint);
  void drawImageNine(Image image, Rect center, Rect dst, Paint paint);
```

## drawPicture

&emsp;将给定的图片 Picture picture 绘制到 canvas 上。要创建 Picture picture，可参见 PictureRecorder。

```dart
  void drawPicture(Picture picture);
```

## drawParagraph

&emsp;将给定段落中的文本 Paragraph paragraph 绘制到给定的偏移位置 Offset offset 上。

&emsp;必须先在段落对象 Paragraph paragraph 上调用 Paragraph.layout。

&emsp;要对齐文本，请在传递给 ParagraphBuilder.new 构造函数的 ParagraphStyle 对象上设置 textAlign。

&emsp;如果文本是左对齐或两端对齐的，左边距将位于偏移参数的 Offset.dx 坐标指定的位置。

&emsp;如果文本是右对齐或两端对齐的，右边距将在由给定给 Paragraph.layout 的 ParagraphConstraints.width 和偏移参数的 Offset.dx 坐标相加得到的位置描述。

&emsp;如果文本是居中的，则居中轴将在由给定给 Paragraph.layout 的 ParagraphConstraints.width 的一半与偏移参数的 Offset.dx 坐标相加得到的位置描述的位置上。

```dart
  void drawParagraph(Paragraph paragraph, Offset offset);
```

## drawPoints & drawRawPoints

&emsp;drawPoints：根据给定的 PointMode pointMode，绘制一个点序列。`List<Offset> points` 参数被解释为相对于原点的偏移量。对于每个点（PointMode.points）或线（PointMode.lines 或 PointMode.polygon），都使用 Paint paint，而忽略 Paint.style。

&emsp;drawRawPoints：根据给定的 PointMode pointMode，绘制一系列点。`Float32List points` 参数被解释为一对浮点数的列表，其中每对表示相对于原点的 x 和 y 偏移量。对于每个点（PointMode.points）或线（PointMode.lines 或 PointMode.polygon），都使用 Paint paint，而忽略 Paint.style。

```dart
  void drawPoints(PointMode pointMode, List<Offset> points, Paint paint);
  void drawRawPoints(PointMode pointMode, Float32List points, Paint paint);
```

### PointMode enum

&emsp;PointMode 枚举定义了绘制一组点时对点列表的解释方式。被 Canvas.drawPoints 和 Canvas.drawRawPoints 方法使用。

+ PointMode.points：将每个点单独绘制。如果 Paint.strokeCap 为 StrokeCap.round，那么每个点将绘制为直径为 Paint.strokeWidth 的圆形否则，每个点将绘制为边长为 Paint.strokeWidth 的轴对齐正方形。（由点点组成的线。）
+ PointMode.lines：将每对点序列绘制为一条线段。如果点的数量是奇数，则忽略最后一个点。（由线段组成的线。）
+ PointMode.polygon：将整个点序列绘制为一条线。（直接把所有的点串起来。）

## drawVertices

&emsp;将一组 Vertices 绘制为一个或多个三角形。

&emsp;Paint.color 属性指定了要用于三角形的默认颜色。

&emsp;如果设置了 Paint.shader 属性，则会完全覆盖 Paint.color，用指定的 ImageShader、Gradient 或其他 Shader 替换颜色。

&emsp;blendMode 参数用于控制 Vertices 中的颜色如何与绘制的颜色组合。如果 Vertices 中没有指定颜色，则 blendMode 不起作用。如果 Vertices 中有颜色，则从绘制的 Paint.shader 或 Paint.color 中取到的颜色会与 Vertices 中指定的颜色使用 blendMode 参数混合。在这种混合情况下，paint 参数中的颜色被视为源（source），而顶点中的颜色被视为目标（destination）。BlendMode.dst 忽略 paint，仅使用顶点的颜色；BlendMode.src 忽略顶点的颜色，仅使用 paint 中的颜色。[BlendMode enum](https://api.flutter.dev/flutter/dart-ui/BlendMode.html) 中有各个 BlendMode 模式的展示效果，可以跳转过去看看。

&emsp;所有参数都不能为 null。

&emsp;另请参阅：

+ Vertices.new，用于创建要在画布上绘制的顶点集。
+ Vertices.raw，使用类型数据列表创建顶点，而不是使用未编码的列表。

```dart
  void drawVertices(Vertices vertices, BlendMode blendMode, Paint paint);
```

## drawAtlas & drawRawAtlas

&emsp;drawAtlas：将 Image atlas 的许多部分（- the atlas - ）绘制到 canvas 上。

&emsp;当想要将 Image 的许多部分绘制到 canvas 上时，例如在使用 sprites 或 zooming 时，此方法可以实现优化。它比多次调用 drawImageRect 更高效，并提供了更多功能，可以单独对每个 image part（图像部分）进行变换，如旋转或缩放，并使用纯色混合或调制这些部分。

&emsp;该方法接受一个 Rect 对象的列表，每个 Rect 对象都定义独立绘制的 Image atlas 的一部分。每个 Rect 与 transforms 列表中的 RSTransform 条目关联，该条目定义了用于绘制图像部分的位置、旋转和（均匀的）缩放。每个 Rect 还可以与可选的 Color 关联，该颜色将与关联的图像部分在混合结果绘制到 canvas 之前使用 blendMode 进行合成。完整操作可以拆分为:

+ 使用 `List<Rect> rects` 参数中的条目指定图像的每个矩形部分与 `List<Color>? colors` 列表中的关联条目使用 `BlendMode? blendMode` 参数进行混合（如果指定了颜色）。在此操作中，图像部分将被视为操作的源（source），并且关联的颜色将被视为目标（destination）。
+ 使用 `List<RSTransform> transforms` 列表中关联的条目中表达的平移、旋转和缩放属性，将第一步的结果混合到 canvas 上，使用参数 `Paint paint` 对象的属性。 

&emsp;如果需要操作的第一阶段是将图像的每个部分与一种颜色混合，则颜色和 blendMode 参数都不能为 null，且颜色列表中必须有每个图像部分的条目。如果不需要该阶段，则颜色参数可以是 null 或空列表，blendMode 参数也可以是 null。

&emsp;可选的 `Rect? cullRect` 参数可以提供一个估计值，用于比较由图集的所有组件渲染的坐标的边界，并与 clip 进行快速拒绝操作，如果不相交的话。

&emsp;`List<RSTransform> transforms` 和 `List<Rect> rects` 列表的长度必须相等，如果 `List<Color>? colors` 参数不为空，则它必须为空或与其他两个列表具有相同的长度。

&emsp;drawRawAtlas 与 drawAtlas 完成同样的功能，仅仅是参数类型不同。

```dart
  void drawAtlas(Image atlas,
                 List<RSTransform> transforms,
                 List<Rect> rects,
                 List<Color>? colors,
                 BlendMode? blendMode,
                 Rect? cullRect,
                 Paint paint);

  void drawRawAtlas(Image atlas,
                    Float32List rstTransforms,
                    Float32List rects,
                    Int32List? colors,
                    BlendMode? blendMode,
                    Rect? cullRect,
                    Paint paint);
```

## drawShadow

&emsp;根据给定的 Path path 绘制阴影。(material elevation)

&emsp;若遮挡物体不是不透明的，则 transparentOccluder 参数应为 true。这些参数不得为 null。

```dart
  void drawShadow(Path path, Color color, double elevation, bool transparentOccluder);
```












## 参考链接
**参考链接:🔗**
+ [Canvas class](https://api.flutter.dev/flutter/dart-ui/Canvas-class.html)
+ [PictureRecorder class](https://api.flutter.dev/flutter/dart-ui/PictureRecorder-class.html)
+ [Picture class](https://api.flutter.dev/flutter/dart-ui/Picture-class.html)
+ [Flutter 3.7 之快速理解 toImageSync 是什么？能做什么？](https://guoshuyu.cn/home/wx/Flutter-N18.html)
+ [ClipOp enum](https://api.flutter.dev/flutter/dart-ui/ClipOp.html)
+ [Canvas.drawVertices — Incredibly fast, incredibly low-level, incredibly fun](https://www.youtube.com/watch?v=pD38Yyz7N2E)
