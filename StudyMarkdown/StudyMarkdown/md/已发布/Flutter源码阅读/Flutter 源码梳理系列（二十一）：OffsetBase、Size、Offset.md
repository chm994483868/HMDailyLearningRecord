# Flutter 源码梳理系列（二十一）：OffsetBase、Offset、Size

# OffsetBase

&emsp;OffsetBase 一个基类，用于描述二维轴（x 轴和y 轴）对齐矢量的距离，Size 和 Offset 都是这种描述方式的实现。

## Constructors

&emsp;抽象常量构造函数。这个构造函数使得子类能够提供常量构造函数，以便它们可以在常量表达式中使用。第一个参数设置水平分量，第二个参数设置垂直分量。

```dart
abstract class OffsetBase {
  const OffsetBase(this._dx, this._dy);
  
  // ...
}
```

## `_dx` & `_dx`

&emsp;提供了两个 double 类型的属性，（可记录在 x 轴和 y 轴偏移值，也可记录在 x 轴和 y 轴与原点的距离值）。

```dart
  final double _dx;
  final double _dy;
```

## isInfinite

&emsp;是否是无限的♾️。

&emsp;如果两个组件（`_dx` & `_dx`）中任一组件为 double.infinity，则返回 true；如果两个组件（`_dx` & `_dx`）都是有限值（或负无穷大，或 NaN），则返回 false。

&emsp;这与将两个组件（`_dx` & `_dx`）均设置为 double.infinity 进行相等性比较不同。

&emsp;另请参阅：isFinite，如果两个组件（`_dx` & `_dx`）都是有限值（而且不是 NaN），则返回 true。

```dart
  bool get isInfinite => _dx >= double.infinity || _dy >= double.infinity;
```

## isFinite

&emsp;是否是有限的。

&emsp;检查两个组件（`_dx` & `_dx`）是否都是有限的（既不是无限的也不是 NaN）。

&emsp;另请参阅：isInfinite，如果其中一个组件等于正无穷大，则返回 true。

```dart
  bool get isFinite => _dx.isFinite && _dy.isFinite;
```

&emsp;然后下面👇提供了五个操作符，还是比较好理解和简单的，快速过一下即可。

## `operator <`

&emsp;小于运算符。将一个 Offset 或 Size 与另一个 Offset 或 Size 进行比较，如果左操作数的水平和垂直值都分别小于右操作数的水平和垂直值，则返回 true。否则返回 false。

&emsp;这是一种部分排序。有可能两个值既不小于、也不大于、也不等于另一个值。

```dart
  bool operator <(OffsetBase other) => _dx < other._dx && _dy < other._dy;
```

## `operator <=` 

&emsp;小于或等于运算符。将一个 Offset 或 Size 与另一个 Offset 或 Size 进行比较，如果左操作数的水平和垂直值都小于或等于右操作数的水平和垂直值，则返回 true。否则返回 false。

&emsp;这是一种偏序关系。可能存在两个值既不小于、也不大于、也不等于另一个值的情况。

```dart
  bool operator <=(OffsetBase other) => _dx <= other._dx && _dy <= other._dy;
```

## `operator >`

&emsp;大于操作符。将一个 Offset 或 Size 与另一个 Offset 或 Size 进行比较，如果左操作数的水平和垂直值分别大于右操作数的水平和垂直值，则返回 true。否则返回 false。

&emsp;这是一个部分排序。两个值之间可能既不小于，也不大于，也不等于另一个值。

```dart
  bool operator >(OffsetBase other) => _dx > other._dx && _dy > other._dy;
```

## `operator >=`

&emsp;大于或等于运算符。比较一个 Offset 或 Size 与另一个 Offset 或 Size，并在左操作数的水平和垂直值均大于或等于右操作数的水平和垂直值时返回 true。否则返回 false。

&emsp;这是一个部分排序。有可能两个值既不小于，也不大于，也不等于另一个值。

```dart
  bool operator >=(OffsetBase other) => _dx >= other._dx && _dy >= other._dy;
```

## `operator ==`

&emsp;等号运算符。比较一个 Offset 或者 Size 与另一个 Offset 或者 Size，如果左操作数的水平和垂直值分别等于右操作数的水平和垂直值，则返回 true。否则返回 false。

&emsp;只要两个 OffsetBase 的变量的 dx 和 dy 都相等的话就是相等的。并不用一定要是同一个 OffsetBase 对象。

```dart
  @override
  bool operator ==(Object other) {
    return other is OffsetBase
        && other._dx == _dx
        && other._dy == _dy;
  }
```

## hashCode

&emsp;重写了 hashCode，与此 OffsetBase 对象已无关，只要关注 dx 和 dy 两个属性，只要两个属性相等，那么两个 OffsetBase 对象的哈希码就是相同的。

```dart
  @override
  int get hashCode => Object.hash(_dx, _dy);
```

## OffsetBase 总结

&emsp;主要提供了两个 double 类型的属性 `_dx` 和 `_dy` 来储存数值。OffsetBase 作为 Offset 和 Size 两个类的基类，在 Offset 类中 `_dx` 和 `_dy` 可以分别表示在 X 轴和 Y 轴的偏移值。在 Size 类中 `_dx` 和 `_dy` 可以分别表示宽度和高度的值。然后在 Offset 和 Size 两个类中就不需要其他存储值的属性了。

&emsp;然后再根据 `_dx` 和 `_dy` 两个属性的值是有限与无限，总结出来两个特征：`isInfinite` 和 `isFinite`，在 Offset 中可以表示无限偏移量，在 Size 中可表示尺寸无限大。

&emsp;其实感觉 OffsetBase 作为 Offset 的基类已经够了，但是这里还是把它作为 Size 的基类了，恰巧它们都需要两个属性来存储值，恰巧它们都有有限和无限两个特征。

&emsp;下面快速看一下 Offset 和 Size 两个在 RenderObject 的布局中要实际使用的类。

# Offset

&emsp;Offset：一个不可变的二维浮点偏移量（2D floating-point offset）。

&emsp;通常来说，偏移量可以有两种解释方式：

1. 作为在笛卡尔空间中距离一个单独维护的原点的点。例如，在 RenderBox 协议中，子级的左上角位置通常是从父级 Box 的左上角偏移（Offset）表示的。

2. 作为可以应用到坐标的矢量。例如，在绘制 RenderObject 时，父级会收到一个从屏幕原点的偏移量 Offset，它可以将这个偏移量添加到其子节点的偏移量上，从而找到从屏幕原点到每个子级的 Offset。

&emsp;因为一个特定的 Offset 可以在某一时刻被解释为一个含义，然后在后续的时刻被解释为另一个含义，所以同一个类用于这两种含义。

+ Object -> OffsetBase -> Offset。

## Constructors

&emsp;构建一个 Offset 对象。

```dart
class Offset extends OffsetBase {
  // 创建一个 Offset。第一个参数设置了 dx，即水平分量，第二个参数设置了 dy，即垂直分量。
  const Offset(super.dx, super.dy);
  
  // 根据给定的方向 direction 和距离 distance 创建一个 Offset。
  // 该方向 direction 是从正 x 轴顺时针的弧度值。
  // 距离 distance 可以被省略，以创建一个单位向量（距离 = 1.0）。
  factory Offset.fromDirection(double direction, [ double distance = 1.0 ]) {
    return Offset(distance * math.cos(direction), distance * math.sin(direction));
  }
  
  // ...
}
```

## dx & dy

&emsp;Offset 的 x 分量和 y 分量。

```dart
  double get dx => _dx;
  double get dy => _dy;
```

## distance

&emsp;偏移量的大小。如果你需要这个值来与另一个偏移量的距离进行比较，考虑使用 distanceSquared，因为计算起来更便宜。

```dart
  double get distance => math.sqrt(dx * dx + dy * dy);
```

## distanceSquared

&emsp;偏移量的幅度的平方。这比计算 distance 本身要更便宜。

```dart
  double get distanceSquared => dx * dx + dy * dy;
```

## direction

&emsp;这个偏移的角度以弧度表示，从正 x 轴顺时针开始，在 -pi 到 pi 的范围内，假设 x 轴的正值向右，y 轴的正值向下。当 dy 为零且 dx 为零或正值时，角度为零。

+ 从零到 pi/2 的值表示 dx 和 dy 的正值，即右下象限。
+ 从 pi/2 到 pi 的值表示 dx 的负值和 dy 的正值，即左下象限。
+ 从零到 -pi/2 的值表示 dx 的正值和 dy 的负值，即右上象限。
+ 从 -pi/2 到 -pi 的值表示 dx 和 dy 的负值，即左上象限。

&emsp;当 dy 为零且 dx 为负时，方向为 pi。当 dx 为零时，如果 dy 为正，则方向为 pi/2，如果 dy 为负，则方向为 -pi/2。

```dart
  double get direction => math.atan2(dy, dx);
```

## zero

&emsp;Offset 静态常量，零值。这可以用来表示坐标空间的原点。

```dart
  static const Offset zero = Offset(0.0, 0.0);
```

## infinite

&emsp;Offset 静态常量，具有无限 x 和 y 组件的偏移量。

&emsp;另请参阅：

+ isInfinite，用于检查任一分量是否为无限。
+ isFinite，用于检查两个分量是否都是有限的。

```dart
  static const Offset infinite = Offset(double.infinity, double.infinity);
```

## scale

&emsp;返回一个新的 Offset 对象，其中 x 分量按 scaleX 缩放，y 分量按 scaleY 缩放。

&emsp;如果两个缩放参数相同，则考虑使用 `*` 运算符代替：

```dart
Offset a = const Offset(10.0, 10.0);
Offset b = a * 2.0; // same as: a.scale(2.0, 2.0)
```

&emsp;如果两个参数是 -1，则考虑使用一元 `-` 运算符替代：

```dart
Offset a = const Offset(10.0, 10.0);
Offset b = -a; // same as: a.scale(-1.0, -1.0)
```

```dart
  Offset scale(double scaleX, double scaleY) => Offset(dx * scaleX, dy * scaleY);
```

## translate

&emsp;返回一个新的 Offset 对象，其中 x 分量增加了 translateX，y 分量增加了 translateY。

&emsp;如果参数来自另一个偏移量，请考虑使用 `+` 或 `-` 运算符替代：

```dart
Offset a = const Offset(10.0, 10.0);
Offset b = const Offset(10.0, 10.0);
Offset c = a + b; // same as: a.translate(b.dx, b.dy)
Offset d = a - b; // same as: a.translate(-b.dx, -b.dy)
```

```dart
  Offset translate(double translateX, double translateY) => Offset(dx + translateX, dy + translateY);
```

## `operator -`

&emsp;一元否定运算符。返回一个具有取反坐标的偏移量。如果 Offset 表示平面上的箭头，则此运算符将返回同一箭头，但指向相反方向。

```dart
  Offset operator -() => Offset(-dx, -dy);
```

## `-+*/~/%` 

&emsp;一组操作符。见名知意。

```dart
  Offset operator -(Offset other) => Offset(dx - other.dx, dy - other.dy);
  Offset operator +(Offset other) => Offset(dx + other.dx, dy + other.dy);
  Offset operator *(double operand) => Offset(dx * operand, dy * operand);
  Offset operator /(double operand) => Offset(dx / operand, dy / operand);
  Offset operator ~/(double operand) => Offset((dx ~/ operand).toDouble(), (dy ~/ operand).toDouble());
  Offset operator %(double operand) => Offset(dx % operand, dy % operand);
```

## `operator &`

&emsp;Rectangle 构造函数操作符。将偏移量（Offset）和尺寸（Size）合并，形成一个 Rect。其左上角坐标是将此偏移量（左侧操作数）添加到原点得到的点，尺寸是右侧操作数。

```dart
// same as: Rect.fromLTWH(0.0, 0.0, 100.0, 100.0)
Rect myRect = Offset.zero & const Size(100.0, 100.0);
```

```dart
  Rect operator &(Size other) => Rect.fromLTWH(dx, dy, other.width, other.height);
```

## lerp

&emsp;在两个偏移量之间进行线性插值。

&emsp;如果其中一个偏移量为 null，这个函数将从 Offset.zero 开始插值。

&emsp;参数 t 代表时间轴上的位置，其中 0.0 表示插值尚未开始，返回a（或等效于a），1.0 表示插值已完成，返回 b（或等效于 b），其间的值表示插值在时间轴上处于 a 和 b 之间的相应点。插值可以超出 0.0 和 1.0，因此负值和大于 1.0 的值都是有效的（可以轻松地通过 Curves.elasticInOut 这样的曲线生成）。

&emsp;t 的值通常来自于 `Animation<double>`，例如 `AnimationController`。

```dart
  static Offset? lerp(Offset? a, Offset? b, double t) {
    if (b == null) {
      if (a == null) {
        return null;
      } else {
        return a * (1.0 - t);
      }
    } else {
      if (a == null) {
        return b * t;
      } else {
        return Offset(_lerpDouble(a.dx, b.dx, t), _lerpDouble(a.dy, b.dy, t));
      }
    }
  }
  
  // 在两个双精度之间线性插值。
  double _lerpDouble(double a, double b, double t) {
    return a * (1.0 - t) + b * t;
  }
```

## Offset 总结

&emsp;在子级 RenderObject 中表示相对于其父级 RenderObject 的偏移量，快速阅读即可。

# Size

&emsp;保存 2D floating-point size。你可以把它看作是原点的偏移量。

+ Object -> OffsetBase -> Size

## Constructors

&emsp;一组针对不同情况的构造函数。

```dart
class Size extends OffsetBase {
  // 使用给定的宽度和高度创建一个 Size。
  const Size(super.width, super.height);
  
  // 创建一个与另一个尺寸具有相同值的 Size 实例。
  // 被渲染库的 _DebugSize hack 使用。
  Size.copy(Size source) : super(source.width, source.height);

  // 创建一个正方形 Size，其宽度和高度是给定的尺寸。
  const Size.square(double dimension) : super(dimension, dimension); // ignore: use_super_parameters

  // 使用给定的宽度和 无限高度 创建一个 Size。
  const Size.fromWidth(double width) : super(width, double.infinity);

  // 使用给定的高度和 无限宽度 创建一个 Size。
  const Size.fromHeight(double height) : super(double.infinity, height);
  
  // 创建一个正方形尺寸，宽度和高度是给定尺寸的两倍。
  // 这是一个包含了给定半径圆的正方形。
  const Size.fromRadius(double radius) : super(radius * 2.0, radius * 2.0);
  
  // ...
}
```

## width & height

&emsp;表示这个尺寸的水平和垂直范围。

&emsp;直接读取从 OffsetBase 继承的两个属性 `_dx` 和 `_dy` 的值。这里可以理解为直接把这两个属性换了一个名字。

```dart
  double get width => _dx;
  double get height => _dy;
```

## aspectRatio

&emsp;该尺寸的宽高比。

&emsp;这将返回宽度除以高度得到的值。

&emsp;如果宽度为零，则结果将为零。如果高度为零（而宽度不为零），则结果将根据宽度的符号确定为 double.infinity 或 double.negativeInfinity。

&emsp;另请参阅：

+ AspectRatio，一个用于为子 Widget 指定特定宽高比的 Widget。
+ FittedBox，一个 Widget（在大多数模式下）尝试在更改大小的同时保持子 Widget 的宽高比。

```dart
  double get aspectRatio {
    if (height != 0.0) {
      return width / height;
    }
    
    if (width > 0.0) {
      return double.infinity;
    }
    
    if (width < 0.0) {
      return double.negativeInfinity;
    }
    
    return 0.0;
  }
```

## zero

&emsp;Size 静态变量。一个空的尺寸，宽度为零，高度为零。

```dart
  static const Size zero = Size(0.0, 0.0);
```

## infinite

&emsp;Size 静态变量。一个宽度和高度都为无穷大的尺寸。

```dart
  static const Size infinite = Size(double.infinity, double.infinity);
```

## isEmpty

&emsp;这个尺寸是否包含了非零面积。负面积被视为空。

```dart
  bool get isEmpty => width <= 0.0 || height <= 0.0;
```

## `-+*/~/%`

```dart
  OffsetBase operator -(OffsetBase other) {
    // Size - Size 返回 Offset。
    // 如果 `sizeA - sizeB -> offsetA`，那么 `offsetA + sizeB -> sizeA
    if (other is Size) {
      return Offset(width - other.width, height - other.height);
    }
    
    // Size - Offset 返回 Size
    if (other is Offset) {
      return Size(width - other.dx, height - other.dy);
    }
    
    throw ArgumentError(other);
  }

  // 二进制加法运算符，用于将一个 Offset 加到一个 Size 上。
  // 返回一个 Size，它的宽度是左操作数（一个 Size）的宽度和右操作数（一个 Offset）的 Offset.dx 维度之和，
  // 高度是左操作数的高度和右操作数的 Offset.dy 维度之和。
  Size operator +(Offset other) => Size(width + other.dx, height + other.dy);

  // 乘法操作符。
  // 返回一个尺寸（Size），
  // 其维度是左侧操作数（一个 Size）的维度乘以标量右侧操作数（一个 double）。
  Size operator *(double operand) => Size(width * operand, height * operand);

  // 除法运算符。
  // 返回一个尺寸，其维度是左操作数（一个 Size）的维度除以标量右操作数（一个 double）。
  Size operator /(double operand) => Size(width / operand, height / operand);

  // 整数（截断）除法运算符。
  // 返回一个尺寸（Size），
  // 其维度是左操作数（一个 Size）的维度除以标量右操作数（一个 double），并向零舍入。
  Size operator ~/(double operand) => Size((width ~/ operand).toDouble(), (height ~/ operand).toDouble());

  /// Modulo (remainder) operator.
  ///
  /// Returns a [Size] whose dimensions are the remainder of dividing the
  /// left-hand-side operand (a [Size]) by the scalar right-hand-side operand (a
  /// [double]).
  
  // 取模（余数）运算符。
  // 返回一个尺寸（Size），
  // 其尺寸是将左操作数（一个 Size）除以标量右操作数（一个 double）得到的余数。
  Size operator %(double operand) => Size(width % operand, height % operand);
```

## shortestSide & longestSide

&emsp;宽度和高度的大小中较小的值和较大的值。

```dart
  double get shortestSide => math.min(width.abs(), height.abs());
  double get longestSide => math.max(width.abs(), height.abs());
```

&emsp;下面这些便利方法相当于在从给定原点和此大小构造的 Rect 上调用同名方法。

## topLeft/topCenter/topRight/centerLeft/center/centerRight/bottomLeft/bottomCenter/bottomRight

```dart
  // 从给定的 Offset 到矩形的顶部和左边缘的交点的偏移量
  //（此 Offset 被解释为左上角），以及此 Size。
  Offset topLeft(Offset origin) => origin;
  
  // 给定的偏移量（被解释为矩形的左上角）到矩形顶部边缘中心的偏移量。
  Offset topCenter(Offset origin) => Offset(origin.dx + width / 2.0, origin.dy);
  
  // 给定偏移量描述的矩形的顶部和右侧边缘的交点的偏移量（这个偏移量被解释为左上角）
  // 和这个大小之间的矩形描述。
  Offset topRight(Offset origin) => Offset(origin.dx + width, origin.dy);

  // 给定偏移量描述的矩形左边缘中心的偏移量（将其解释为左上角）和此大小。
  Offset centerLeft(Offset origin) => Offset(origin.dx, origin.dy + height / 2.0);
  
  // 给定偏移量描述的矩形的中点到左右边缘以及顶部和底部边缘之间的偏移量
  //（该偏移量被解释为矩形的左上角）和此尺寸。
  Offset center(Offset origin) => Offset(origin.dx + width / 2.0, origin.dy + height / 2.0);
  
  // 给定的偏移量（被解释为矩形的左上角）和这个尺寸描述的矩形的右边缘中心点的偏移量。
  Offset centerRight(Offset origin) => Offset(origin.dx + width, origin.dy + height / 2.0);
  
  // 给定的偏移量（被解释为左上角）描述的矩形的底部和左边缘的交叉点的偏移量和这个尺寸。
  Offset bottomLeft(Offset origin) => Offset(origin.dx, origin.dy + height);
  
  // 矩形底边中心点到给定偏移量描述的矩形的中心的偏移量（偏移量被解释为左上角）和这个大小。
  Offset bottomCenter(Offset origin) => Offset(origin.dx + width / 2.0, origin.dy + height);
  
  // 给定的偏移量描述了一个矩形的位置（被解释为左上角），
  // 这个矩形的底部和右侧边缘的交点偏移量。
  Offset bottomRight(Offset origin) => Offset(origin.dx + width, origin.dy + height);
```

## contains

&emsp;判断由给定偏移量指定的点（假定相对于大小的左上角）是否位于此大小的矩形的左右和上下边界之间。

&emsp;矩形包含其顶部和左侧边缘，但不包括底部和右侧边缘。

```dart
  bool contains(Offset offset) {
    return offset.dx >= 0.0 && offset.dx < width && offset.dy >= 0.0 && offset.dy < height;
  }
```

## flipped

&emsp;交换宽度和高度的尺寸。

```dart
  Size get flipped => Size(height, width);
```

## lerp

&emsp;在两个尺寸之间进行线性插值

&emsp;如果其中一个尺寸为 null，则此函数会从 Size.zero 开始插值。

&emsp;t 参数表示时间轴上的位置，0.0 表示插值未开始，返回a（或与a等效）；1.0 表示插值已完成，返回b（或与b等效）；介于0.0和1.0之间的值表示插值在时间轴上处于a和b之间相应位置。可以对插值进行超出0.0和1.0的外推，因此负值和大于1.0的值是有效的（并且可以轻松通过弹性曲线（例如 Curves.elasticInOut）生成）。

&emsp;t 的值通常从 `Animation<double>`（例如 AnimationController）获取。

```dart
  static Size? lerp(Size? a, Size? b, double t) {
    if (b == null) {
      if (a == null) {
        return null;
      } else {
        return a * (1.0 - t);
      }
    } else {
      if (a == null) {
        return b * t;
      } else {
        return Size(_lerpDouble(a.width, b.width, t), _lerpDouble(a.height, b.height, t));
      }
    }
  }
  
 double _lerpDouble(double a, double b, double t) {
   return a * (1.0 - t) + b * t;
 }
```

## Size 总结

&emsp;如其名，表示尺寸大小。直接继承自 OffsetBase，然后把从 OffsetBase 继承的两个属性 `_dx` 和 `_dy` 名字修改为 width 和 height。快速浏览即可，为后面的 Constraints 学习打下基础。

## 参考链接
**参考链接:🔗**
+ [Size class](https://api.flutter.dev/flutter/dart-ui/Size-class.html)
+ [OffsetBase class](https://api.flutter.dev/flutter/dart-ui/OffsetBase-class.html)
+ [Offset class](https://api.flutter.dev/flutter/dart-ui/Offset-class.html)
