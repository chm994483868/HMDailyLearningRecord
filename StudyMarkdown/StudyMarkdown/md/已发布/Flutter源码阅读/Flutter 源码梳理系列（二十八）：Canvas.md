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

&emsp;注意到了吗？Canvas 的同名工厂构造函数返回的是 `_NativeCanvas`，它的定义如下：`base class _NativeCanvas extends NativeFieldWrapperClass1 implements Canvas { // ... }`。`_NativeCanvas` 同样是一个实现了 Canvas 所有抽象函数并继承自 NativeFieldWrapperClass1 的 base class，它的实现全部由 Flutter engine 实现。所以 Canvas 其实是 Flutter engine 层到 Flutter framework 层的桥接，Canvas 提供的 API 的真正实现在 engine 层，而在 framework 层中我们可以像其它普通的 framework 层的类一样使用 Canvas 的 API。 所以，我们暂时不分心去看它如何实现，我们只要专注于 Canvas 都提供了哪些功能 API 即可。

&emsp;其实同 Canvas 一样的还有：

+ Picture：`base class _NativePicture extends NativeFieldWrapperClass1 implements Picture { // ... }`
+ PictureRecorder：`base class _NativePictureRecorder extends NativeFieldWrapperClass1 implements PictureRecorder { // ... }`
+ Path：`base class _NativePath extends NativeFieldWrapperClass1 implements Path { //... }`
+ EngineLayer：`base class _NativeEngineLayer extends NativeFieldWrapperClass1 implements EngineLayer { // ... }`
+ Scene：`base class _NativeScene extends NativeFieldWrapperClass1 implements Scene { // ... }`
+ SceneBuilder：`base class _NativeSceneBuilder extends NativeFieldWrapperClass1 implements SceneBuilder { // ... }`

&emsp;看到了吗？它们都是属于 Flutter engine 层到 Flutter framework 层的桥接，它们在 framework 层为我们提供了可以无缝在其它 dart 类中可以使用的 API，但是它们的实现部分其实都在 engine 层。而且它们都是与绘制相关的内容，毕竟 Flutter 作为一个 UI 框架，它的绘制能力还是要来自当前所处的 Native 平台的。后续我们再对这些内容学习，目前的话我们专注于这些 API，看看它们都提供了哪些功能。

&emsp;OK，我们继续回到 Canvas 的源码。在开始之前呢，我们先看一个示例，来理解一下：Canvas 的 save 和 restore 的作用，关于它们的内容不太好理解。

&emsp;首先我们对它俩的内容解释一下，然后再看下面的示例代码。

&emsp;当我们使用 CustomPainter 绘制自定义图形时，我们可以使用 Canvas 的 save 和 restore 方法来保存和恢复绘制状态，并且 save 和 restore 必须是成对出现的，否则 IDE 会保存提醒我们。（save 和 restore 我们可以理解为是把当前绘制状态进行入栈和出栈，栈则是绘制栈。）

&emsp;save 方法会保存当前 Canvas 的矩阵状态、剪裁区域、图层以及绘制效果等信息。通过 save 方法保存状态后，我们可以进行一系列绘制操作，然后通过 restore 方法将 Canvas 恢复到距离此 restore 最近的 save 保存的状态，这样就可以避免影响到后续绘制操作。

&emsp;直白一点理解，就是当我们分阶段绘制我们的内容时，我们把每个阶段的绘制内容用 save 和 restore 给它包裹起来，那么各个绘制阶段就不会相互影响了，伪代码如下：

```dart
class MyPainter extends CustomPainter {
  @override
  void paint(Canvas canvas, Size size) {
    
    canvas.save();
    // 在此处进行第 1️⃣ 阶段的绘制
    
    // 例如这里是在第 1️⃣ 阶段绘制内部，
    // 下面这个 translate 调用直接把 canvas 的绘制状态向下平移了 100，
    // 但是由于这里被完整的 save 和 restore 包围着呢，
    // 那么到了下面的第 2️⃣ 阶段时，它不会受此向下偏移 100 的影响，
    // 第 2️⃣ 阶段还是会从 (0, 0) 原点处开始绘制。
    
    canvas.translate(0, 100);
    
    // 而在这个第 1️⃣ 阶段 translate 下面的绘制内容，
    // 则都会因为这个 translate 而整体向下平移 100。
    
    canvas.restore(); // 把第一阶段的绘制状态 pop 出了绘制堆栈，这样就不影响后续的绘制了。
    
    // 下面我们可以从 canvas 的最初态继续我们的绘制了。
    
    canvas.save();
    
    // 在此处进行第 2️⃣ 阶段的绘制
    
    canvas.restore();
    
    // 如果后续没有分阶段的绘制了，
    // 我们也可以省略 save 和 restore，
    // 直接在此进行第 3️⃣ 阶段的绘制
  }
}
```

&emsp;当然如果我们没有那么多绘制阶段，只想一次给它绘制完毕的话，那么直接省略 save 和 restore 也是可以的。但是如果在绘制时没有正确使用 save 和 restore 方法来保存和恢复 Canvas 的绘制状态，可能会导致绘制效果出现意外的结果，或者影响到后续的绘制操作。没有正确保存和恢复 Canvas 绘制状态可能会导致以下问题：

1. 绘制效果叠加：如果在绘制过程中改变了 Canvas 的状态（如平移、旋转、缩放、图层等），而没有在后续绘制完成后恢复状态，可能导致后续的绘制操作受到之前的状态影响，从而出现意外的绘制效果。

2. 剪裁区域错误：如果在绘制过程中修改了 Canvas 的剪裁区域，但没有恢复，可能会导致后续的绘制操作受到错误的剪裁，从而绘制内容被裁剪。

3. 性能问题：频繁修改 Canvas 状态而没有正确保存和恢复可能会影响性能，不必要的状态变化增加了绘制的开销。

&emsp;因此，为了确保绘制的正确性和性能，建议在需要修改 Canvas 的绘制状态时使用 save 方法保存绘制状态在绘制栈中，在绘制完成后使用 restore 方法恢复绘制状态，从而保持绘制的独立性、隔离性和正确性。

&emsp;如下是一个完整的示例，我们可以一键粘贴到我们的 IDE 里面运行调试一下：

```dart
import 'package:flutter/material.dart';
import 'package:flutter/rendering.dart';

void main() {
  runApp(const MyCustomPainterApp());
}

class MyCustomPainterApp extends StatelessWidget {
  const MyCustomPainterApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: const Text('CustomPainter Example')),
        body: Center(
          child: CustomPaint(size: MediaQuery.of(context).size, painter: MyPainter()),
        ),
      ),
    );
  }
}

class MyPainter extends CustomPainter {
  @override
  void paint(Canvas canvas, Size size) {
    debugPrint('      初始状态:${canvas.getSaveCount()} ${canvas.getTransform()}');

    // 第 1️⃣ 阶段绘制，保存 Canvas 的状态。
    canvas.save();
    debugPrint('第一次 save 后:${canvas.getSaveCount()} ${canvas.getTransform()}');

    // 把 canvas 向下平移 100
    canvas.translate(0, 100);
    debugPrint(' translate 后:${canvas.getSaveCount()} ${canvas.getTransform()}');

    // 首先绘制一个：位于原点，宽高分别是 100 的红色正方形。
    // 但是由于上面👆canvas 向下平移了 100，所以红色正方形的位置在 (0, 100) 处
    canvas.drawRect(
        const Rect.fromLTWH(0, 0, 100, 100), Paint()..color = Colors.red);

    // 第 1️⃣ 阶段绘制结束了，恢复 Canvas 的状态。
    canvas.restore();
    debugPrint('阶段一 restore:${canvas.getSaveCount()} ${canvas.getTransform()}');

    // 第 2️⃣ 阶段绘制
    canvas.save();

    // 这里回到了 canvas 的初始态了，可以继续进行其他绘制操作，不受之前的绘制影响
    // 绘制一个圆心在 (150, 150)，直径是 100 的颜色是蓝色的圆
    canvas.drawCircle(const Offset(150, 150), 50, Paint()..color = Colors.blue);

    canvas.restore();
  }

  @override
  bool shouldRepaint(covariant CustomPainter oldDelegate) => false;
}

// log 输出如下：
// 初始态，然后下面第一个 restore 调用后，会恢复到此状态。看到初始 saveCount 绘制栈是 2。
flutter:       初始状态:2 [1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 115.0, 0.0, 1.0]

flutter: 第一次 save 后:3 [1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 115.0, 0.0, 1.0]
flutter:  translate 后:3 [1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 215.0, 0.0, 1.0]

// 可看到此处 canvas 又恢复了初始态。
flutter: 阶段一 restore:2 [1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 115.0, 0.0, 1.0]
```

<center class="half">
  <img src="https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/e5571c680c1d4724b96433ead37b5e9c~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1721794209&x-orig-sign=dbC7zmH%2FlEPp3PsZFARNax7ymek%3D" width="200"/><img src="https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/20a50894d4564aec8d2d14e12bbf295d~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1721794311&x-orig-sign=tjPTEEDR853EOv6NDCFa%2FTy%2B9pI%3D" width="200"/>
</center>

&emsp;OK，接下来我们继续看 Canvas 的源码。

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

&emsp;目前看下来，只能注意到 savaLayer 相对于 save 而言多了 Layer 的新建。saveLayer 会创建新的 Layer，而 save 则是还在当前 Layer 下进行绘制。然后还有一个抗锯齿的处理的不同。

```dart
  void saveLayer(Rect? bounds, Paint paint);
```

## restore

&emsp;如果有内容要弹出，则弹出当前保存栈（save stack）。否则，不执行任何操作。

&emsp;使用 save 和 saveLayer 将 canvas 状态 push 到栈上。如果 state 是使用 saveLayer pushed 的，则此调用还将导致新图层（new layer）合成到上一个图层中。

```dart
  void restore();
```

## restoreToCount

&emsp;将保存栈（save stack）还原到之前的层级，就像从 getSaveCount 获取到的那样。如果 count 小于 1，则栈将被还原到初始状态。如果 count 大于当前的 getSaveCount，则不会发生任何操作。

&emsp;使用 save 和 saveLayer 将 canvas 状态推入栈中。

&emsp;如果通过此调用恢复的 canvas 状态堆栈级别中有任何是通过 saveLayer 推送的，则此调用还将导致这些层被合成到它们以前的层中。

```dart
  void restoreToCount(int count);
```

## getSaveCount

&emsp;返回保存栈内的项目数量，包括初始状态。这意味着对于一个干净的 canvas，它返回 1，每个 save 和 saveLayer 调用都会递增它，并且每个匹配的 restore 调用都会递减它。

&emsp;这个数字不会低于1。

```dart
  int getSaveCount();
```

&emsp;OK，上面就是跟 Canvas 相关的绘制状态堆栈保存的全部函数了，下面则是 Canvas 提供的一系列绘制 API 了，如绘制路径、文本、图像等等来实现自定义的绘制，以及平移、旋转、缩放等等变换效果。

## translate & scale & rotate & skew & transform

&emsp;下面一组函数，即对本绘制阶段内后续的绘制内容整体进行：平移、缩放、旋转等操作。（注意是对后续的绘制操作产生影响，比如我们绘制了一个正方形，我们想要它旋转 30 度，那么我们就需要先调用：canvas.rotate(30)，然后在绘制正方形。还有它是对后续整体的绘制内容进行处理，例如本次绘制阶段我们画了一个正方向和一个圆形，那么平移操作就是让它们整体进行平移，而不是单独对正方形或者圆形平移，如果平移操作需要单独处理的话，则需要把它们拆分到不同的绘制阶段。）

&emsp;translate：将 current transform 添加到 translation 中，通过第一个参数 dx 水平移动坐标空间，通过第二个参数 dy 垂直移动坐标空间。（即在 x 轴和 y 轴平移。）

&emsp;scale：将一个与坐标轴对齐的缩放添加到 current transform 中，水平方向按第一个参数缩放，垂直方向按第二个参数缩放。如果未指定 sy，则 sx 将用于在两个方向上进行缩放。（即在 x 轴和 y 轴变大或者缩小。）

&emsp;rotate：将 current transform 添加旋转。参数为顺时针弧度。

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

&emsp;OK，Canvas 中与变换相关的 API 看完了，下面则是一系列与裁剪相关的 API。

## clipRect

&emsp;将 clip region 减少到当前 clip region 和给定矩形（Rect rect）的交集部分。

![image.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/80a7fcac29744edf85498b028898c3c7~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1721702423&x-orig-sign=wW2ZuED7VoP1iU6ItbWaDLORKc0%3D)

&emsp;如果 doAntiAlias 为 true，则裁剪边界将会进行抗锯齿处理。

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

&emsp;将 clip region 减少到当前 clip region 与给定圆角矩形（RRect rrect）的交集部分。

![image.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/fddd5bfe1db2406481c4b192012c6881~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1721702722&x-orig-sign=HFUs86zLGVE8p5%2FImWs6Jfefcdk%3D)

&emsp;如果 doAntiAlias 为 true，则裁剪边界将会进行抗锯齿处理。

&emsp;如果多个绘制命令与裁剪边界相交，这可能会导致在裁剪边界处发生错误的混合。

```dart
  void clipRRect(RRect rrect, {bool doAntiAlias = true});
```

## clipPath

&emsp;将 clip region 减小到当前 clip region 与给定路径（Path path）的交集部分。

![image.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/985f0fd12c64428f968a2f42c2f2b568~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1721703020&x-orig-sign=ZSAbPU2O0K9%2BGVYXHBXV8G3%2Fyes%3D)

&emsp;如果 doAntiAlias 为 true，则裁剪边界将会进行抗锯齿处理。

```dart
  void clipPath(Path path, {bool doAntiAlias = true});
```

## getLocalClipBounds

&emsp;返回在当前 Canvas 对象的保存栈内执行的所有裁剪方法组合结果的保存边界，以本地坐标空间计量，即在当前进行渲染操作的本地坐标空间下。

&emsp;组合的裁剪结果在转回本地坐标空间之前会被舍入到整数像素边界，这考虑了渲染操作中的像素舍入，尤其是在抗锯齿时。因为 Picture 最终可能会被渲染到 transforming widgets 或 layers 上下文中的场景中，因此由于过早的舍入，结果可能会过于保守。结合使用 getDestinationClipBounds 方法、外部转换和真实设备坐标系中的舍入，将产生更准确的结果，但此值可能提供一个更便利的近似值，用于比较渲染操作与已建立的裁剪操作。

&emsp;边界的保守估计是基于执行 ClipOp.intersect 与每个裁剪方法的边界相交，可能会忽略使用 ClipOp.difference 执行的任何裁剪方法。ClipOp 参数仅在 clipRect 方法上存在。

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

&emsp;返回在当前 Canvas 对象的保存栈内执行的所有 clip 方法组合结果的保守边界，以目标坐标空间中的度量为准，该坐标空间将渲染 Picture。

```dart
  Rect getDestinationClipBounds();
```

&emsp;Ok，Canvas 裁剪相关的 API 结束了，下面是一组绘制功能的 API。

## drawColor

&emsp;使用给定的颜色在 Canvas 上绘制，应用给定的 BlendMode，其中给定的 color 作为 source color，背景作为 destination color。

```dart
  void drawColor(Color color, BlendMode blendMode);
```

## drawLine

&emsp;使用给定的 Paint paint 在给定的点（Offset p1 和 Offset p2）之间绘制一条线。该线是 stroked 的，对于这次调用，忽略 Paint.style 的值。Offset p1 和 Offset p2 参数被解释为相对于原点的偏移量。

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

&emsp;下面则是一组把图片绘制到 canvas 中的 API。

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

## Canvas 总结

&emsp;OK，Canvas 的内容看完了，首先是它的初始化需要传入一个 PictureRecorder 用于记录在此 Canvas 中进行的绘制操作，并在绘制结束时可通过 PictureRecorder.endRecording 取得 Picture。取得 Picture 对象后可通过 SceneBuilder.addPicture 把此 Picture 添加到 Scene 中。其后通过 window.render 将 Scene 送入 Engine 层，最终经 GPU 光栅化后显示在屏幕上。

&emsp;Canvas 的绘制结果想要显示到屏幕上后续还有很多路要走。后面我们会逐步学习。当前的话我们的主要目标是理解 Canvas 的功能定位（在渲染管线上处于哪个环节。），以及提供了哪些基础的绘制接口。

&emsp;关于 Canvas 的工厂构造函数中出现的 `_NativeCanvas`：`base class _NativeCanvas extends NativeFieldWrapperClass1 implements Canvas { // ... }`，它才是完成 Canvas 绘制操作的实现者，它位于 engine 层，它是由 C++ 实现的，都是比较复杂的，但是呢在 framework 层 Canvas 为我们提供了良好的绘制操作的接口，让我们得以在 framework 层可以轻松的使用自定义绘制功能，后续学习 RenderObject 的绘制流程时我们更能体现到 Canvas 良好接口设计的价值。

&emsp;那么 Canvas 的绘制接口则可以细分为如下几个部分：

1. save/saveLayer/restore：确保绘制的正确性和性能，在需要修改 Canvas 的绘制状态时使用 save 方法保存绘制状态在绘制栈中，在绘制完成后使用 restore 方法恢复绘制状态，从而保持绘制的独立性、隔离性和正确性。
2. translate/scale/rotate/skew/transform：支持矩阵变换（transformation matrix）：平移/缩放/旋转/倾斜，它们将作用于其后在该 Canvas 上进行的绘制操作。
3. clipRect/clipRRect/clipPath：支持区域裁剪(clip region)，它们将作用于其后在该 Canvas 上进行的绘制操作。
4. 下面则是一系列的绘制函数：

+ drawColor: 在 Canvas 上填充指定颜色。
+ drawPaint: 在 Canvas 上绘制 Paint 对象。
+ drawLine: 在 Canvas 上绘制直线。
+ drawRect: 在 Canvas 上绘制矩形。
+ drawRRect: 在 Canvas 上绘制圆角矩形。
+ drawDRRect: 在 Canvas 上绘制双圆角矩形。
+ drawOval: 在 Canvas 上绘制椭圆形。
+ drawCircle: 在 Canvas 上绘制圆形。
+ drawArc: 在 Canvas 上绘制弧线。
+ drawPath: 在 Canvas 上绘制路径。
+ drawImage: 在 Canvas 上绘制指定图片。
+ drawImageRect: 在 Canvas 上绘制指定图片的指定区域。
+ drawImageNine: 在 Canvas 上绘制九宫格方式拉伸图片。
+ drawPicture: 在 Canvas 上绘制 Picture 对象。
+ drawParagraph: 在 Canvas 上绘制文本段落。
+ drawPoints: 在 Canvas 上绘制点集。
+ drawRawPoints: 在 Canvas 上绘制原始点集。
+ drawVertices: 在 Canvas 上绘制顶点集合。
+ drawAtlas: 在 Canvas 上绘制图集（纹理集合）中的图像。
+ drawRawAtlas: 在 Canvas 上绘制原始图集中的图像。
+ drawShadow: 在 Canvas 上绘制阴影效果。

&emsp;Canvas 的内容学习到这里，我们下篇继续。

## 参考链接
**参考链接:🔗**
+ [Canvas class](https://api.flutter.dev/flutter/dart-ui/Canvas-class.html)
+ [PictureRecorder class](https://api.flutter.dev/flutter/dart-ui/PictureRecorder-class.html)
+ [Picture class](https://api.flutter.dev/flutter/dart-ui/Picture-class.html)
+ [Flutter 3.7 之快速理解 toImageSync 是什么？能做什么？](https://guoshuyu.cn/home/wx/Flutter-N18.html)
+ [ClipOp enum](https://api.flutter.dev/flutter/dart-ui/ClipOp.html)
+ [Canvas.drawVertices — Incredibly fast, incredibly low-level, incredibly fun](https://www.youtube.com/watch?v=pD38Yyz7N2E)
