# Flutter 源码梳理系列（二十一.一）：Rect

# Rect

&emsp;Rect：一个相对于给定原点的不可变的、二维的、轴对齐的、浮点数矩形。

&emsp;可以使用 Rect 的其中一个构造函数或者使用 Offset 和 Size 以及 & 运算符来创建一个 Rect：

```dart
Rect myRect = const Offset(1.0, 2.0) & const Size(3.0, 4.0);
```

## Constructors

&emsp;下面是一组 Rect 的构造函数：

### fromLTRB

&emsp;根据矩形的左边界、顶部边界、右边界和底部边界构造一个矩形。

![image.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/924e23dff11d4716bde0fad935c9b72a~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1721527344&x-orig-sign=eeO9TGuY%2FsfhBphyXSfIxGuj690%3D)

```dart
  const Rect.fromLTRB(this.left, this.top, this.right, this.bottom);
```

### fromLTWH

&emsp;根据矩形的左边界、顶部边界、宽度和高度构建一个矩形。

&emsp;要根据一个 Offset 和一个 Size 构建一个 Rect，可以使用矩形构造运算符 &。如下：Offset.&。

![image.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/5bf350a2a9f342108f2579aad6d82233~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1721527750&x-orig-sign=3eqgq7n839Z2JyIj7O5HWLGgm44%3D)

```dart
  const Rect.fromLTWH(double left, double top, double width, double height) : this.fromLTRB(left, top, left + width, top + height);
  
  Rect operator &(Size other) => Rect.fromLTWH(dx, dy, other.width, other.height);
  
  // Rect myRect = Offset.zero & const Size(100.0, 100.0);
  // same as: Rect.fromLTWH(0.0, 0.0, 100.0, 100.0)
```

### fromCenter

&emsp;根据其中心点、宽度和高度构造一个矩形。

&emsp;假定 center 参数是相对于原点的偏移量。

![image.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/aa31bb35922d4c84b866498e284788ce~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1721528117&x-orig-sign=Snr%2FJbfbEdPFNG2MZip27BSc3wc%3D)

```dart
  Rect.fromCenter({ required Offset center, required double width, required double height }) : this.fromLTRB(
    center.dx - width / 2,
    center.dy - height / 2,
    center.dx + width / 2,
    center.dy + height / 2,
  );
```

### fromCircle

&emsp;构建一个包围给定圆的矩形。

&emsp;center 参数被假定为相对于原点的偏移量。

![image.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/705c00ea978d4f34adf5ada4f84438ee~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1721528236&x-orig-sign=AMiBMwjPdE7MyO30MTG%2FBwXjnjs%3D)

```dart
  Rect.fromCircle({ required Offset center, required double radius }) : this.fromCenter(
    center: center,
    width: radius * 2,
    height: radius * 2,
  );
```

### fromPoints

&emsp;构建一个最小的矩形，包围给定的偏移量，将它们视为从原点出发的向量。

![image.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/9d19712ef3f4400297c6ce1d6f081ce9~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1721528384&x-orig-sign=MRrefyGYU1FhWYgBHOkZoKv2HaM%3D)

```dart
  Rect.fromPoints(Offset a, Offset b) : this.fromLTRB(
    math.min(a.dx, b.dx),
    math.min(a.dy, b.dy),
    math.max(a.dx, b.dx),
    math.max(a.dy, b.dy),
  );
```

## left & top & right & bottom

&emsp;仅有的四个最重要的 double 类型的属性。左边界、顶部边界、右边界、底部边界。

```dart
  // 这个矩形左边界相对于 x 轴的偏移量。
  final double left;

  // 这个矩形顶部边界相对于 y 轴的偏移量。
  final double top;

  // 这个矩形右边界相对于 x 轴的偏移量。
  final double right;
  
  // 这个矩形底部边界相对于 y 轴的偏移量。
  final double bottom;
```

## width & height & size

&emsp;一些几何属性。

```dart
  // 这个矩形的左边界和右边界之间的距离。
  double get width => right - left;

  // 这个矩形的顶部边界和底部边界之间的距离。
  double get height => bottom - top;

  // 这个矩形的左上角和右下角之间的距离。
  Size get size => Size(width, height);
```

## hasNaN

&emsp;是否有任何维度是 `NaN`。

```dart
  bool get hasNaN => left.isNaN || top.isNaN || right.isNaN || bottom.isNaN;
```

## zero

&emsp;静态常量 zero。四边分别与左边、顶边、右边和底边都重合的矩形，边长均为零。

```dart
  static const Rect zero = Rect.fromLTRB(0.0, 0.0, 0.0, 0.0);
```

## largest

&emsp;一个覆盖整个坐标空间的矩形。

&emsp;这个矩形覆盖从 -1e9,-1e9 到 1e9,1e9 的空间。这是图形操作有效的空间。

```dart
  static const double _giantScalar = 1.0E+9; // matches kGiantRect from layer.h
  static const Rect largest = Rect.fromLTRB(-_giantScalar, -_giantScalar, _giantScalar, _giantScalar);
```

## isInfinite & isFinite & isEmpty

&emsp;isInfinite：但凡有一个值是无限的就是无限的。isFinite：必须所有值都是有限的才是有限的。

&emsp;isEmpty：即只要一个方向的大值小于等于小值，即数学中的：一个宽度或者高度为 0 的矩形，那么它的面积肯定也是零。

```dart
  // 这个矩形的任何坐标是否等于正无穷大。
  // 为了与 Offset 和 Size 保持一致而包含在内。
  bool get isInfinite {
    return left >= double.infinity
        || top >= double.infinity
        || right >= double.infinity
        || bottom >= double.infinity;
  }

  // 这个矩形的所有坐标是否都是有限的。
  bool get isFinite => left.isFinite && top.isFinite && right.isFinite && bottom.isFinite;
  
  // 这个矩形是否包含非零面积。负面积被视为空。
  bool get isEmpty => left >= right || top >= bottom;
```

## shift

&emsp;返回一个根据给定偏移量 offset 平移的新矩形。

&emsp;如果要通过单独的 x 和 y 分量而不是通过一个 Offset 来平移一个矩形，请考虑使用 translate。

```dart
  Rect shift(Offset offset) {
    return Rect.fromLTRB(left + offset.dx, top + offset.dy, right + offset.dx, bottom + offset.dy);
  }
```

## translate

&emsp;返回一个新的矩形，其中 translateX 添加到 x 分量中，translateY 添加到 y 分量中。

&emsp;要通过 Offset 而不是单独的 x 和 y 分量平移矩形，请考虑使用 shift。

```dart
  Rect translate(double translateX, double translateY) {
    return Rect.fromLTRB(left + translateX, top + translateY, right + translateX, bottom + translateY);
  }
```

## inflate

&emsp;返回一个新的矩形，其边缘向外移动给定的增量。

&emsp;即可以理解为绕着原来的旧矩形扩大了一圈（delta）。（left 和 top 减去 delta，right 和 bottom 加上 delta。）

```dart
  Rect inflate(double delta) {
    return Rect.fromLTRB(left - delta, top - delta, right + delta, bottom + delta);
  }
```

## deflate

&emsp;返回一个边缘向内移动给定偏移量的新矩形。

&emsp;刚好与 inflate 相反，即可以理解为绕着原来的旧矩形缩小了一圈（-delta）。（left 和 top 加上 delta，right 和 bottom 减去 delta。）

```dart
  Rect deflate(double delta) => inflate(-delta);
```

## intersect

&emsp;返回一个新的矩形，它是给定矩形和该矩形的交集。这两个矩形必须重叠才有意义。如果两个矩形不重叠，则生成的矩形将具有负宽度或高度。

```dart
  Rect intersect(Rect other) {
    return Rect.fromLTRB(
      math.max(left, other.left),
      math.max(top, other.top),
      math.min(right, other.right),
      math.min(bottom, other.bottom)
    );
  }
```

## expandToInclude

&emsp;返回一个新的矩形，该矩形是包含此矩形和给定矩形的边界框。

```dart
  Rect expandToInclude(Rect other) {
    return Rect.fromLTRB(
        math.min(left, other.left),
        math.min(top, other.top),
        math.max(right, other.right),
        math.max(bottom, other.bottom),
    );
  }
```

## overlaps

&emsp;该矩形是否与其他矩形有非零重叠区域。

&emsp;如果一个矩形的右边界小于等于一个矩形左边界，那肯定这个矩形整体都是在另一个矩形的左边了，它们两个肯定不会重叠。其它判断如此一样。

```dart
  bool overlaps(Rect other) {
    if (right <= other.left || other.right <= left) {
      return false;
    }
    
    if (bottom <= other.top || other.bottom <= top) {
      return false;
    }
    
    return true;
  }
```

## shortestSide & longestSide

&emsp;宽度或者高度中较大/较小的那个值。

```dart
  // 这个矩形的宽度和高度两者中较小的那个数值。
  double get shortestSide => math.min(width.abs(), height.abs());

  // 这个矩形的宽度和高度两者中较大的那个数值。
  double get longestSide => math.max(width.abs(), height.abs());
```

## topLeft & topCenter & topRight

&emsp;顶部左、中、右的偏移量。

```dart
  // 这个矩形顶部边界和左侧边界交叉点的偏移量。
  Offset get topLeft => Offset(left, top);

  // 这个矩形顶部边界的中心的偏移。
  Offset get topCenter => Offset(left + width / 2.0, top);

  // 这个矩形顶部边界和右侧边界交叉点的偏移量。
  Offset get topRight => Offset(right, top);
```

## centerLeft & center & centerRight

&esmp;中部左、中、右的偏移量。

```dart
  // 这个矩形左边界中心位置的偏移量。
  Offset get centerLeft => Offset(left, top + height / 2.0);

  // 这个矩形的左右和上下边界的中点的偏移量。
  Offset get center => Offset(left + width / 2.0, top + height / 2.0);

  // 这个矩形右边界中心位置的偏移量。
  Offset get centerRight => Offset(right, top + height / 2.0);
```

## bottomLeft & bottomCenter & bottomRight

```dart
  // 这个矩形底部边界和左侧边界交叉点的偏移量。
  Offset get bottomLeft => Offset(left, bottom);

  
  // 这个矩形底部边界的中心的偏移。
  Offset get bottomCenter => Offset(left + width / 2.0, bottom);

  // 这个矩形底部边界和右侧边界交叉点的偏移量。
  Offset get bottomRight => Offset(right, bottom);
```

## contains

&emsp;判断由给定偏移量指定的点（假定相对于原点）是否位于此矩形的左侧、右侧、上侧和下侧边界之间。

&emsp;矩形包括其上边界和左边界，但不包括其下边界和右边界。

```dart
  bool contains(Offset offset) {
    return offset.dx >= left && offset.dx < right && offset.dy >= top && offset.dy < bottom;
  }
```

## lerp

&emsp;在两个矩形之间进行线性插值。

&emsp;如果其中一个矩形为 null，则会使用 Rect.zero 作为替代。

&emsp;参数 t 表示在时间轴上的位置，0.0 表示插值尚未开始，返回a（或等价于a），1.0 表示插值已完成，返回b（或等价于b），介于两者之间的值表示插值在时间轴上的相应位置。插值可以在 0.0 和 1.0 之外进行外推，因此负值和大于 1.0 的值都是有效的（并且可以通过诸如 Curves.elasticInOut 的曲线轻松生成）。

&emsp;通常，t 的值是从 `Animation<double>`（如 AnimationController）中获取的。

&emsp;后面我们学习 Flutter 动画时会深入此部分。

```dart
  static Rect? lerp(Rect? a, Rect? b, double t) {
    if (b == null) {
      if (a == null) {
        return null;
      } else {
        final double k = 1.0 - t;
        return Rect.fromLTRB(a.left * k, a.top * k, a.right * k, a.bottom * k);
      }
    } else {
      if (a == null) {
        return Rect.fromLTRB(b.left * t, b.top * t, b.right * t, b.bottom * t);
      } else {
        return Rect.fromLTRB(
          _lerpDouble(a.left, b.left, t),
          _lerpDouble(a.top, b.top, t),
          _lerpDouble(a.right, b.right, t),
          _lerpDouble(a.bottom, b.bottom, t),
        );
      }
    }
  }
```

## Rect 总结

&emsp;OK，内容都比较简单，主要是围绕四个 double 类型的属性：left、top、right、bottom 展开，它们分别表示在 x 轴和 y 轴距离原点的一个数值，然后我们可以在大脑里面以这四个值构建出一个矩形来，再配合我们前面学习的 Size 和 Offset，为我们后续学习 Flutter 绘制、布局相关的内容时打下基础。

## 参考链接
**参考链接:🔗**
+ [Rect class](https://api.flutter.dev/flutter/dart-ui/Rect-class.html)
