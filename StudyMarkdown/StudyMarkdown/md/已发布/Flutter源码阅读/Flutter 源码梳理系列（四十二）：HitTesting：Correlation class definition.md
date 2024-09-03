# Flutter 源码梳理系列（四十二）：HitTesting：Correlation class definition

# `_TransformPart`

&emsp;`_TransformPart` 是一种可以通过 left-multiplication（左乘）矩阵应用的数据类型。

```dart
@immutable
abstract class _TransformPart {
  const _TransformPart();
  
  // ...
}
```

## multiply

&emsp;将这个 `_TransformPart` 从左侧应用到 `rhs` 上（当前 `_TransformPart` 在左边，入参 Matrix4 rhs 在右边）。这样应该工作得就好像这个 `_TransformPart` 首先被转换为一个矩阵，然后左乘到入参 `Matrix4 rhs` 上。举个例子，如果这个 `_TransformPart` 是一个向量 `v1`，它对应的矩阵是 `m1 = Matrix4.translation(v1)`，那么 `_VectorTransformPart(v1).multiply(rhs)` 的结果应该等于 `m1 * rhs`。

```dart
  Matrix4 multiply(Matrix4 rhs);
```

# `_MatrixTransformPart`

&emsp;

```dart
class _MatrixTransformPart extends _TransformPart {
  const _MatrixTransformPart(this.matrix);

  final Matrix4 matrix;
  
  // ...
}
```

## multiply

&emsp;

```dart
  @override
  Matrix4 multiply(Matrix4 rhs) {
    return matrix.multiplied(rhs);
  }
```

# `_OffsetTransformPart`

&emsp;

```dart
class _OffsetTransformPart extends _TransformPart {
  const _OffsetTransformPart(this.offset);

  final Offset offset;
  
  // ...
}
```

## multiply

&emsp;

```dart
  @override
  Matrix4 multiply(Matrix4 rhs) {
    return rhs.clone()..leftTranslate(offset.dx, offset.dy);
  }
```

&emsp;PointerEvent：触摸、触控笔或鼠标事件的基类。

&emsp;Pointer events 在屏幕坐标空间中运作，按逻辑像素（logical pixels）进行缩放。逻辑像素大致近似于每厘米 38 个像素，或每英寸 96 个像素。

&emsp;这样可以独立于设备的精确硬件特性来识别手势。特别是，可以根据大致的物理长度定义诸如触摸误差（参见 kTouchSlop）之类的功能，以便用户可以在高密度显示屏上以相同的距离移动手指，就像在低分辨率设备上移动手指一样。

&emsp;出于类似的原因，pointer events 不受渲染层中的任何 transforms 的影响。这意味着在应用于渲染内部移动之前，可能需要对增量进行缩放。例如，如果一个滚动列表显示为 2x 缩放，那么指针增量将需要按照相反的倍数进行缩放，以便列表在用户的手指上滚动时显示出来。

# BoxHitTestResult

&emsp;BoxHitTestResult：对 RenderBoxe 执行 hit test 的结果。该类的一个实例被提供给 RenderBox.hitTest，用于记录 hit test 的结果。

+ Object -> HitTestResult -> BoxHitTestResult

```dart
class BoxHitTestResult extends HitTestResult {
  // 创建一个用于在 [RenderBox] 上进行 hit testing 的空 hit test 结果。
  // (这里空的意思即是 HitTestResult 的 _path、_transforms、_localTransforms 三个属性被初始化为空列表。)
  BoxHitTestResult() : super();
  
  // ...
}
```

## wrap

&emsp;将 result 包装起来，创建一个 HitTestResult，该 result 实现了用于在 RenderBoxes 上进行 hit testing 的 BoxHitTestResult 协议。

&emsp;此方法由调整 RenderBox-world 和非 RenderBox-world 之间的 RenderObjects 使用，将 (subtype of) HitTestResult 转换为 BoxHitTestResult 进行在 RenderBoxes 上进行 hit testing。

添加到返回的 BoxHitTestResult 中的 HitTestEntry 实例也会被添加到包装的结果中（两者共享相同的底层数据结构来存储 HitTestEntry 实例）。

```dart
  BoxHitTestResult.wrap(super.result) : super.wrap();
```






## 参考链接
**参考链接:🔗**
+ [HitTestTarget class](https://api.flutter.dev/flutter/gestures/HitTestTarget-class.html)
+ [HitTestResult class](https://api.flutter.dev/flutter/gestures/HitTestResult-class.html)
+ [BoxHitTestResult class](https://api.flutter.dev/flutter/rendering/BoxHitTestResult-class.html)
+ [`HitTestEntry<T extends HitTestTarget> class`](https://api.flutter.dev/flutter/gestures/HitTestEntry-class.html)
+ [BoxHitTestEntry class](https://api.flutter.dev/flutter/rendering/BoxHitTestEntry-class.html)
+ [HitTestResult class](https://api.flutter.dev/flutter/gestures/HitTestResult-class.html)
