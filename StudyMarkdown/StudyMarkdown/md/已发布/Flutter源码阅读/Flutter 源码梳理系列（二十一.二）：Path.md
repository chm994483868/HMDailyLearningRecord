# Flutter 源码梳理系列（二十一.二）：Path

# Path

&emsp;本篇来学习在 PaintingContext、Canvas、SceneBuilder 中都会用到的 Path。Path 和 Canvas 一样，也是 Flutter engine 层到 Flutter framework 层的桥接，Path 提供的 API 的真正实现在 engine 层，而在 framework 层中我们可以像其它普通的 framework 层的类一样使用 Path 的 API。且 Path 也是完全同 Canvas 一样的套路，在 Path 的同名工厂构造函数中直接返回 `_NativePath`，我们以为自己使用的是 Flutter framework 中一个 Path 类，其实它已经跑到了 engine 层。

&emsp;和学习 Canvas 时一样，我们也暂时不用纠结于 engine 是如何实现 Path 的一众 API 的，我们只要把目光聚焦在 Path API 提供了什么即可。看到 Path 我们大脑中估计会立即浮现出：这就是一个描述路径的类吧，估计也是像其它平台一样，提供直线、曲线、虚线以及闭合路径 等功能，也确实是这样的。在 Flutter 中 Path 最主要的作用就是当我们在 Canvas 中绘制自定义路径时，可以直接用 Path 进行描述，然后另外一个就是当我们对 Canvas 进行区域裁剪时，我们也可以使用 Path 参数来指定裁剪的区域。

&emsp;Path 中的内容整体还是比较简单的，我们快速浏览即可。下面过一遍 Path 的源码，首先是看它的介绍文档。
 
&emsp;Path 由多个子路径（sub-paths）和一个当前点（current point）组成。

&emsp;子路径（sub-paths）由各种类型的段组成，如直线（lines）、弧线（arcs）或贝塞尔曲线（beziers）。子路径（sub-paths）可以是开放的也可以是闭合的，并且可以相互交叉。

&emsp;闭合子路径（closed sub-paths）根据当前的 fillType，围绕着平面上的一个（可能是不连续的）区域。

&emsp;当前点（current point）最初位于原点。在每次向子路径（sub-path）添加一个段的操作之后，当前点（current point）被更新到该段的末尾。

&emsp;Path 可以使用 Canvas.drawPath 绘制在画布（canvases）上，也可以使用于 Canvas.clipPath 中创建裁剪区域（clip regions）。

&emsp;下面看一下 Path 的源码。

## Constructors

&emsp;`_NativePath` 是一个针对于当前平台的 base class（`base class _NativePath extends NativeFieldWrapperClass1 implements Path { // ... }`），`_NativePath` 类实现了 Path 抽象类中的所有抽象函数，并且在 Path 的同名工厂构造函数中直接返回。

```dart
abstract class Path {
  factory Path() = _NativePath;
  
  // 创建另一个 Path 的副本。
  // 拷贝这个副本快速且不需要额外内存，除非入参 source path 或此构造函数返回的 Path 被修改。
  factory Path.from(Path source) {
    
    // 创建一个 _NativePath 变量 clonedPath，然后直接调用 _clone 函数，
    // 把入参 source 克隆到 clonedPath。
    final _NativePath clonedPath = _NativePath._();
    (source as _NativePath)._clone(clonedPath);
    
    return clonedPath;
  }
  
  // ...
}
```

## fillType

&emsp;确定如何计算此 Path 的内部。默认值是：PathFillType.nonZero。

```dart
  PathFillType get fillType;
  set fillType(PathFillType value);
```

&emsp;PathFillType 是一个枚举，用来指示 Path 的填充类型，下面看下此枚举都有哪些值：

### PathFillType

&emsp;确定如何计算 Path 内部的环绕规则。这个枚举被 Path.fillType 属性使用。

+ Object -> Enum -> PathFillType

#### nonZero

&emsp;内部由符号边交叉的 non-zero 和定义。对于给定点，如果从该点到无限远处画一条线，这条线穿过顺时针绕点的线的次数与逆时针绕点的线的次数不同，那么该点被认为在路径的内部。[Nonzero-rule](https://en.wikipedia.org/wiki/Nonzero-rule)

#### evenOdd

&emsp;内部是由奇数边交叉定义的。对于给定点，如果从该点到无限远处画出的一条线穿过一条奇数条线，则将该点视为在路径的内部。[Even-odd_rule](https://en.wikipedia.org/wiki/Even-odd_rule)

&emsp;我们可以直接看参考链接，体会下 PathFillType.nonZero 和 PathFillType.evenOdd 的区别。下面继续看 Path 的内容。

## moveTo & relativeMoveTo & lineTo & relativeLineTo

```dart
  // 在给定坐标处开始一个新的子路径（sub-path）。
  //（可以理解为设定（x, y）为当前点，后续便从这新当前点开始画路径。）
  void moveTo(double x, double y);

  // 从当前点开始，在给定的偏移量处开始一个新的子路径（sub-path）。
  //（可以理解为更新当前点为：旧的当前点 + (dx, dy)，后续便从这新当前点开始画路径。）
  void relativeMoveTo(double dx, double dy);

  // 向当前点添加一条以当前点为起点，以这入参指定点 (x, y) 为终点的线段。
  void lineTo(double x, double y);

  // 从当前点开始，添加一条直线段到距当前点给定偏移量的点。
  // (可以理解为以当前点为起点，然后以 "当前点 + (dx, dy)" 为终点的线段。)
  void relativeLineTo(double dx, double dy);
```

## quadraticBezierTo & relativeQuadraticBezierTo

&emsp;quadraticBezierTo：添加一个二次贝塞尔曲线段，从当前点曲线到给定点（x2,y2），使用控制点（x1,y1）。

![image.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/8affe12d1dcb4633a67ab9e947d202c0~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1721578789&x-orig-sign=Xfvwrn%2FbyX8XC4%2FEcWcRm2wF0ik%3D)

```dart
  void quadraticBezierTo(double x1, double y1, double x2, double y2);
```

&emsp;relativeQuadraticBezierTo：添加一个二次贝塞尔曲线段，这个曲线从当前点到从当前点偏移 (x2, y2) 的点（"当前点 + (x2, y2)"）之间的路径，控制点位于从当前点偏移 (x1, y1) 的位置。

```dart
  void relativeQuadraticBezierTo(double x1, double y1, double x2, double y2);
```

## cubicTo & relativeCubicTo

&emsp;cubicTo：添加一个三次贝塞尔曲线段，从当前点曲线到给定点 (x3, y3)，使用控制点 (x1, y1) 和 (x2, y2)。

![image.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/665d2833867f404f9acd0f5b2cdfdfee~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1721579111&x-orig-sign=%2FBz23ZaMpBV%2B%2FamOy87Fgk7P7Bk%3D)

```dart
  void cubicTo(double x1, double y1, double x2, double y2, double x3, double y3);
```

&emsp;relativeCubicTo：添加一个三次贝塞尔曲线段，该曲线从当前点到距当前点偏移为 (x3, y3) 的点，使用从当前点偏移为 (x1,y1) 和 (x2,y2) 的控制点。

```dart
  void relativeCubicTo(double x1, double y1, double x2, double y2, double x3, double y3);
```

## conicTo & relativeConicTo

&emsp;conicTo：添加一个贝塞尔曲线段，从当前点曲线到给定点 (x2, y2)，使用控制点 (x1, y1) 和权重 w。如果权重大于 1，则曲线是一个双曲线；如果权重等于 1，则是一个抛物线；如果小于 1，则是一个椭圆。

![image.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/cb8623dc7f4d43e9a4fc88bd1335f79a~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1721579988&x-orig-sign=r3UTd2XE6650EKuSWJxY8Z9dGKk%3D)

```dart
  void conicTo(double x1, double y1, double x2, double y2, double w);
```

&emsp;relativeConicTo：添加一个贝塞尔曲线段，从当前点曲线到从当前点偏移为 (x2, y2) 的点，使用从当前点偏移为 (x1, y1) 的控制点和权重 w。如果权重大于 1，则曲线是一个双曲线；如果权重等于 1，则是一个抛物线；如果小于 1，则是一个椭圆。

```dart
  void relativeConicTo(double x1, double y1, double x2, double y2, double w);
```

## arcTo

&emsp;如果 forceMoveTo 参数为 false，则添加一条直线段和一条弧段。

&emsp;如果 forceMoveTo 参数为 true，则开始一个由弧段组成的新子路径。

&emsp;无论哪种情况，弧段由沿着给定矩形边界的椭圆的弧组成，从 startAngle 弧度开始绕椭圆到 startAngle + sweepAngle 弧度结束，其中零弧度是通过矩形中心的水平线穿过椭圆右侧的点，正角度顺时针绕椭圆。

&emsp;如果 forceMoveTo 为 false，则添加的线段从当前点开始，终止于弧的起点。

```dart
  void arcTo(Rect rect, double startAngle, double sweepAngle, bool forceMoveTo);
```

## arcToPoint & relativeArcToPoint

&emsp;追加最多四个圆锥曲线，通过权重描述一个半径为 radius 且沿 rotation（以度为单位，顺时针）旋转的椭圆。

&emsp;第一条曲线从路径中的上一个点开始，最后一个点以 arcEnd 结束。根据顺时针和 largeArc 确定的方向，这些曲线沿着路径前进，这样扫过的角度始终小于 360 度。

&emsp;如果两个半径都为零或路径中的上一个点是 arcEnd，则追加一个简单的直线。如果两个半径都大于零但太小而无法描述一条弧线，则将这些半径缩放以适合路径中的最后一个点。

```dart
  void arcToPoint(Offset arcEnd, {
    Radius radius = Radius.zero,
    double rotation = 0.0,
    bool largeArc = false,
    bool clockwise = true,
  });
```

&emsp;追加最多四个共轭曲线，它们被加权以描述一个半径为 radius 并按照 rotation（以角度表示，顺时针）旋转的椭圆。

&emsp;最后的路径点由（px，py）描述。

&emsp;第一条曲线从路径中的最后一个点开始，最后一条曲线以 arcEndDelta.dx + px 和 arcEndDelta.dy + py 结束。曲线沿着由 clockwise 和 largeArc 决定的方向进行，使得扫描角度始终小于 360 度。

&emsp;如果半径为零，或者 arcEndDelta.dx 和 arcEndDelta.dy 都为零，则追加一条简单的直线。如果两者均大于零但太小以描述一条弧时，则将半径缩放以适应路径中的最后一个点。

```dart
  void relativeArcToPoint(
    Offset arcEndDelta, {
    Radius radius = Radius.zero,
    double rotation = 0.0,
    bool largeArc = false,
    bool clockwise = true,
  });
```

## addRect & addOval

&emsp;addRect：添加一个新的子路径（sub-path），由四条线组成，勾勒出给定的矩形。

&emsp;addOval：添加一个新的子路径（sub-path），该子路径（sub-path）由形成填充给定矩形的椭圆的曲线组成。要添加一个圆，可以将一个适当的矩形作为椭圆。可以使用 Rect.fromCircle 来轻松描述圆的中心偏移和半径

```dart
  void addRect(Rect rect);
  void addOval(Rect oval);
```

## addArc

&emsp;添加一个新的子路径（sub-path），其中包含一个弧段，该弧段由限定的矩形边界内椭圆的边缘形成，从弧度 startAngle 开始围绕椭圆到弧度 startAngle + sweepAngle 结束，其中 0 弧度是横穿矩形中心的水平线的椭圆右侧点，正角度顺时针围绕椭圆。

![image.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/b153313c769c42599d59ffbcb1acd2bf~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1721581806&x-orig-sign=aX9ZD7v0IMroeRepiBQOyq93C5E%3D)

![image.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/a2a5494fdd1f41a49120c145ab7398c9~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1721581829&x-orig-sign=HOk9TX%2BpUsHHlYiM8g9RinyMKFM%3D)

```dart
  void addArc(Rect oval, double startAngle, double sweepAngle);
```

## addPolygon

&emsp;添加一个新的子路径（sub-path），该子路径（sub-path）由连接给定点的线段序列组成。

&emsp如果 close 参数为 true，则会添加一个最终线段，连接最后一个点和第一个点。

&emsp;points 参数被解释为相对于原点的偏移量。

```dart
  void addPolygon(List<Offset> points, bool close);
```

## addRRect

&emsp;添加一个新的子路径（sub-path），该子路径（sub-path）由组成所描述的圆角矩形所需的直线和曲线组成。

```dart
  void addRRect(RRect rrect);
```

## addPath

&emsp;将 Path 的子路径（sub-paths）按指定的偏移量添加到此 Path 中。（可以理解为复制一遍已经绘制的路径，然后整体根据 offset 进行偏移。）

&emsp;如果指定了 matrix4，那么在通过给定的偏移量平移矩阵后，Path 将通过该矩阵进行变换。该矩阵是按列主序存储的 4x4 矩阵。

```dart
  void addPath(Path path, Offset offset, {Float64List? matrix4});
```

## extendWithPath

&emsp;将 Path 的子路径（sub-paths）按照偏移量 offset 添加到此 Path 中。当前子路径（sub-paths）将与 Path 的第一个子路径（sub-paths）延长，并在必要时用 lineTo 连接它们。（与上面的 addPath 类似，只是最后可以会把新增的 Path 已旧的 Path 连接起来。）

&emsp;如果指定了 matrix4，那么在矩阵被偏移量平移之后，Path 将按照此矩阵进行变换。该矩阵是按列主序存储的 4x4 矩阵。

```dart
  void extendWithPath(Path path, Offset offset, {Float64List? matrix4});
```

## close & reset

&emsp;close：关闭最后一个子路径（sub-paths），就好像从当前点（current point）到该子路径的第一个点（Path 的起点）画了一条直线一样。（即把最后一个点和第一个点直接用一条直线连接起来。）

&emsp;reset：清除 Path 对象中的所有子路径（sub-paths），将其恢复为创建时的初始状态。当前点（current point）将被重置为原点。

```dart
  void close();
  void reset();
```

## contains & shift & transform

&emsp;contains：测试给定点（Offset point）是否在 Path 内部。(也就是说，如果该 Path 被用于 Canvas.clipPath，该点是否在 Path 的可见部分内，还是说被裁剪掉了。) Offset point 参数被解释为相对于原点的偏移量。如果 point 在 Path 内部则返回 true，否则返回 false。

&emsp;shift：返回 Path 的副本（一个新的 Path，和之前的函数那种在既有的 Path 添加新的子路径（sub-paths）是不同的。），其中每个子路径的所有线段都按给定的偏移量（Offset offset）平移。

&emsp;transform：使用给定的矩阵转换所有子路径（sub-paths）的所有线段，返回 Path 的副本。

```dart
  bool contains(Offset point);
  Path shift(Offset offset);
  Path transform(Float64List matrix4);
```

## getBounds

&emsp;计算此 Path 的边界矩形。（可以理解为在 Path 外面画一个矩形，然后刚好把 Path 包围在里面。）

&emsp;包含仅具有相同直线上的轴对齐点的 Path 将没有 area，因此对于这样的路径，Rect.isEmpty 将返回true。考虑改为检查 rect.width + rect.height > 0.0，或者使用 computeMetrics API 来检查 Path 长度。

&emsp;对于许多更复杂的路径，bounds 可能会不准确。例如，当 Path 包含一个圆时，用于计算边界的点是圆的隐含控制点，它们形成一个围绕圆的正方形；如果应用了 transform 对圆进行变换，则该正方形会旋转，因此（轴对齐、非旋转的）边界框最终会严重高估圆覆盖的实际 area。

```dart
  Rect getBounds();
```

&emsp;下面是 Path 中最后一部分内容，两个比较不好理解的静态函数，我们一起来看一下。

## combine

&emsp;静态函数：根据给定的操作方式（PathOperation operation）将这两条 Path 合并。

&emsp;得到的 Path 将由互不重叠的轮廓构成。在可能的情况下，曲线顺序会降低，使得立方曲线可以变为二次曲线，并且二次曲线可以变为直线

```dart
  static Path combine(PathOperation operation, Path path1, Path path2) {
    final _NativePath path = _NativePath();
    if (path._op(path1 as _NativePath, path2 as _NativePath, operation.index)) {
      return path;
    }
    
    throw StateError('Path.combine() failed.  This may be due an invalid path; in particular, check for NaN values.');
  }
```

## computeMetrics

&emsp;


## Path 总结

&emsp;OK，Path 的内容到这里就看完了，首先是 Path 是一个抽象类，然后它内部的所有函数都是抽象函数，甚至于它的同名构造函数都是返回一个 NativePath（`factory Path() = _NativePath;`），至于 NativePath 是谁？它定义是这样的：`base class _NativePath extends NativeFieldWrapperClass1 implements Path { // ...}`，可以看到它是完全实现了 Path 抽象函数的一个继承自 NativeFieldWrapperClass1 的类，它的内容主要通过 Flutter engine 在 native 层实现。那里面牵涉的内容比较多，我们后续再学习。目前的话只要我们理解 Path 的功能以及它的一些常见函数，然后后面学习 RenderObject 的绘制时，看到 Path 参数不晕即可。

&emsp;Path 相关的 API 都比较好理解，快速浏览即可。 





## 参考链接
**参考链接:🔗**
+ [Path class](https://api.flutter.dev/flutter/dart-ui/Path-class.html)
+ [Class modifiers](https://dart.dev/language/class-modifiers#base)
+ [Flutter and Native Libraries: Boost Performance with Dart FFI](https://blog.flutter.wtf/dart-ffi/)
+ [PathFillType enum](https://api.flutter.dev/flutter/dart-ui/PathFillType.html)
+ [Flutter学习：使用CustomPaint绘制路径](https://juejin.cn/post/7083304661645541390#heading-1)
