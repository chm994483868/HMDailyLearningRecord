# Flutter 源码梳理系列（四十二）：HitTesting：Correlation class definition

# `_TransformPart`

&emsp;`_TransformPart` 是一种可以通过 left-multiplication（左乘）矩阵应用的数据类型。（left-multiplication 在代码方面则是表现为 `_TransformPart` 提供了一个 multiply 函数，入参是一个 Matrix4 rhs，并返回一个 Matrix4。）

&emsp;`_TransformPart` 是一个不可变的抽象类。

```dart
@immutable
abstract class _TransformPart {
  const _TransformPart();
  
  // ...
}
```

&emsp;然后下面是 `_TransformPart` 仅有的一个抽象函数 multiply，即为 `_TransformPart` 提供了一个左乘的概念。

## multiply

&emsp;将这个 `_TransformPart` 从左侧应用到 `rhs` 上（当前 `_TransformPart` 在左边，入参 Matrix4 rhs 在右边）。这样应该工作得就好像这个 `_TransformPart` 首先被转换为一个矩阵，然后左乘到入参 `Matrix4 rhs` 上。举个例子，如果这个 `_TransformPart` 是一个向量 `v1`，它对应的矩阵是 `m1 = Matrix4.translation(v1)`，那么 `_VectorTransformPart(v1).multiply(rhs)` 的结果应该等于 `m1 * rhs`。

```dart
  Matrix4 multiply(Matrix4 rhs);
```

&emsp;OK，`_TransformPart` 的内容就这么多，它作为一个抽象类是无法直接使用的，下面则是它的两个子类：`_MatrixTransformPart` 和 `_OffsetTransformPart`，分别添加了 final Matrix4 matrix 属性和 final Offset offset 属性。

# `_MatrixTransformPart`

&emsp;`_MatrixTransformPart` 直接继承自 `_TransformPart` 并添加了一个 final Matrix4 matrix 属性，作为一个 final 修饰的属性，它只在 `_MatrixTransformPart` 的构造函数调用时，即创建 `_MatrixTransformPart` 对象时进行赋值，并且后续不可再改变了。另一个角度看的话则是：创建 `_MatrixTransformPart` 对象时必须传入一个 Matrix4 matrix 参数。

```dart
class _MatrixTransformPart extends _TransformPart {
  const _MatrixTransformPart(this.matrix);

  final Matrix4 matrix;
  
  // ...
}
```

## multiply

&emsp;`_MatrixTransformPart` 实现了 `_TransformPart` 的抽象函数 multiply，内部则是直接调用 Matrix4 的 multiplied 函数，并把其返回值作为 multiply 函数的返回值。简单理解的话即是：两个 Matrix4 进行乘法运算。

```dart
  @override
  Matrix4 multiply(Matrix4 rhs) {
    return matrix.multiplied(rhs);
  }
```

&emsp;然后下面是 `_TransformPart` 的另一个子类：`_OffsetTransformPart`。

# `_OffsetTransformPart`

&emsp;不同于 `_MatrixTransformPart`，`_OffsetTransformPart` 类则是添加了一个 final Offset offset 属性。同样亦是创建 `_OffsetTransformPart` 对象时必须传入一个 Offset offset 参数。

```dart
class _OffsetTransformPart extends _TransformPart {
  const _OffsetTransformPart(this.offset);

  final Offset offset;
  
  // ...
}
```

## multiply

&emsp;`_OffsetTransformPart` 实现了 `_TransformPart` 的抽象函数 multiply，内部实现也很简单，首先克隆一份入参 Matrix4 rhs，然后调用 Matrix4 的 leftTranslate 函数，把入参 Matrix4 rhs 的克隆体偏移 final Offset offset。

```dart
  @override
  Matrix4 multiply(Matrix4 rhs) {
    return rhs.clone()..leftTranslate(offset.dx, offset.dy);
  }
```

&emsp;通过上面的 `_MatrixTransformPart` 和 `_OffsetTransformPart` 类实现 `_TransformPart` 的抽象函数：multiply，可以看到内部主要是用到了 Matrix4 的 multiplied 函数和 leftTranslate 函数。

&emsp;Matrix4 的 multiplied 函数内部则是调用 Matrix4 的 multiply 函数，它是最简单的矩阵乘法，还记得初中数学中学的矩阵的乘法吗？是的，没错就是我们当时在数学上学的矩阵的乘法的概念在代码上的实现而已。 

&emsp;Matrix4 的 leftTranslate 函数则是矩阵乘以左边的偏移量。leftTranslate 函数有三个参数：x/y/z，x 类型是 dynamic，y/z 类型是 double，因为是动态类型，所以 x 可以接收：3 种类型的值：Vector4/Vector3/double，然后则是参数分别乘以矩阵数组的 3/7/11/15 下标的值为整个矩阵数组更新值。

&emsp;目前先看到这里，后面我们会深入分析 Matrix4 矩阵相关的数据存储方式以及矩阵的各个运算的含义。

&emsp;上一篇呢我们看了 HitTestResult 的内容，它是用来记录 hit testing 结果的，它的 `_path` 属性会把一路上的 hitTest 返回 true 的 RenderObject 

<!--&emsp;PointerEvent：触摸、触控笔或鼠标事件的基类。-->
<!---->
<!--&emsp;Pointer events 在屏幕坐标空间中运作，按逻辑像素（logical pixels）进行缩放。逻辑像素大致近似于每厘米 38 个像素，或每英寸 96 个像素。-->
<!---->
<!--&emsp;这样可以独立于设备的精确硬件特性来识别手势。特别是，可以根据大致的物理长度定义诸如触摸误差（参见 kTouchSlop）之类的功能，以便用户可以在高密度显示屏上以相同的距离移动手指，就像在低分辨率设备上移动手指一样。-->
<!---->
<!--&emsp;出于类似的原因，pointer events 不受渲染层中的任何 transforms 的影响。这意味着在应用于渲染内部移动之前，可能需要对增量进行缩放。例如，如果一个滚动列表显示为 2x 缩放，那么指针增量将需要按照相反的倍数进行缩放，以便列表在用户的手指上滚动时显示出来。-->

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

&emsp;添加到返回的 BoxHitTestResult 中的 HitTestEntry 实例也会被添加到 wrap 的结果中（两者共享相同的底层数据结构来存储 HitTestEntry 实例）。

&esmp;当在 RenderObject 和 RenderBox 之间进行 hit testing 时，由于它们之间要完全共享同一个 HitTestResult，所以才有了此 wrap 函数，可以直白的把它理解为抹平 RenderObject 和 RenderBox 之间的 hit test result 差异。

```dart
  BoxHitTestResult.wrap(super.result) : super.wrap();
```

## addWithPaintTransform

&emsp;将 position 转换为子级 RenderObject 的本地坐标系，以便为子级 RenderObject 进行 hit testing。

&emsp;子级 RenderObject 的实际 hit testing 需要在提供的 hitTest 回调中实现，该回调会使用转换后的 position 作为参数进行调用。

&emsp;提供的 paint transform（描述了从子级到父级的三维变换）通过 PointerEvent.removePerspectiveTransform 处理，以移除 perspective component 并在用于将 position 从父级的坐标系转换为子级的坐标系之前进行反转。

&emsp;如果 transform 为 null，则将其视为 identity transform，并将 position 不加修改地提供给 hitTest 回调。如果无法反转 transform，则不会调用 hitTest 回调，并返回 false。否则，返回 hitTest 回调的返回值。

&emsp;position 参数可能为 null，将按原样转发给 hitTest 回调。如果子级与父级使用不同的 hit test protocol，且在该 protocol 中实际 hit testing 不需要 position，则将 position 设置为 null 可能很有用。

&emsp;该函数返回 hitTest 回调函数的返回值。

&emsp;当子节点和父节点的起始点不相同时，这种方法会在 RenderBox.hitTestChildren 中被使用。

```dart
abstract class RenderFoo extends RenderBox {
  final Matrix4 _effectiveTransform = Matrix4.rotationZ(50);

  @override
  void applyPaintTransform(RenderBox child, Matrix4 transform) {
    transform.multiply(_effectiveTransform);
  }

  @override
  bool hitTestChildren(BoxHitTestResult result, { required Offset position }) {
    return result.addWithPaintTransform(
      transform: _effectiveTransform,
      position: position,
      hitTest: (BoxHitTestResult result, Offset position) {
        return super.hitTestChildren(result, position: position);
      },
    );
  }
}
```

&emsp;下面👇是 addWithPaintTransform 函数本体：

```dart
  // 用于对 RenderBox 进行 hit testing 的方法签名。
  // 被 BoxHitTestResult.addWithPaintTransform 使用以对 RenderBox 的子级进行 hit test。
  typedef BoxHitTest = bool Function(BoxHitTestResult result, Offset position);

  bool addWithPaintTransform({
    required Matrix4? transform,
    required Offset position,
    required BoxHitTest hitTest,
  }) {
    if (transform != null) {
      transform = Matrix4.tryInvert(PointerEvent.removePerspectiveTransform(transform));
      
      if (transform == null) {
        // Objects are not visible on screen and cannot be hit-tested.
        // 对象在屏幕上是不可见的，不能进行 hit-tested
        return false;
      }
    }
    
    return addWithRawTransform(
      transform: transform,
      position: position,
      hitTest: hitTest,
    );
  }
```

## addWithPaintOffset

&emsp;用于对子级进行 hit testing 的便捷方法，这些子级由 Offset? offset 参数转换。

&emsp;子级的实际 hit testing 需要在提供的 BoxHitTest hitTest 回调中实现，该回调会以转换后的 Offset position 作为参数被调用。

&emsp;如果父级在 Offset? offset 处绘制子级，可以使用这个方法作为对 addWithPaintTransform 的方便替代。

&emsp;对于 Offset? offset 参数为 null 的情况，offset 会被视为提供了 Offset.zero 值。

&emsp;该函数返回 hitTest 回调的返回值。

&emsp;另可参阅：

+ addWithPaintTransform，该方法采用通用的绘画变换矩阵（a generic paint transform matrix），并更详细地记录了此 API 的预期用法。

```dart
  bool addWithPaintOffset({
    required Offset? offset,
    required Offset position,
    required BoxHitTest hitTest,
  }) {
    final Offset transformedPosition = offset == null ? position : position - offset;
    
    // 与下面👇的 popTransform 对应 
    if (offset != null) {
      pushOffset(-offset);
    }
    
    final bool isHit = hitTest(this, transformedPosition);
    
    // 与上面👆的 pushOffset 对应
    if (offset != null) {
      popTransform();
    }
    
    return isHit;
  }
```

## addWithRawTransform

&emsp;将 Offset position 转换为子级的本地坐标系，用于对子级进行 hit testing。

&emsp;子级的实际 hit testing 需要在提供的 BoxHitTest hitTest 回调中实现，该回调使用转换后的 Offset position 作为参数调用。

&emsp;与 addWithPaintTransform 不同，提供的 Matrix4? transform 直接用于转换 Offset position，没有任何预处理。

&emsp;如果 transform 为 null，则会将其视为 identity transform，并将 Offset position 不加以处理直接提供给 BoxHitTest hitTest 回调。

&emsp;该函数返回 BoxHitTest hitTest 回调的返回值。

&emsp;另可参阅：

+ addWithPaintTransform，它实现了相同的功能，但采用绘制变换矩阵（paint transform matrix）。

```dart
  bool addWithRawTransform({
    required Matrix4? transform,
    required Offset position,
    required BoxHitTest hitTest,
  }) {
    // 
    final Offset transformedPosition = transform == null ?
        position : MatrixUtils.transformPoint(transform, position);
    
    // 与下面👇的 popTransform 对应 
    if (transform != null) {
      pushTransform(transform);
    }
    
    final bool isHit = hitTest(this, transformedPosition);
    
    // 与上面👆的 pushTransform 对应
    if (transform != null) {
      popTransform();
    }
    
    return isHit;
  }
```

## addWithOutOfBandPosition

&emsp;手动管理 position 变换逻辑时添加 hit testing 的传递方法。

&emsp;子级的实际 hit testing 需要在提供的 BoxHitTestWithOutOfBandPosition hitTest 回调中实现。position 的处理需要由调用者处理。

&emsp;该函数返回 BoxHitTestWithOutOfBandPosition hitTest 回调的返回值。

&emsp;应该将 paintOffset、paintTransform 或 rawTransform 传递给该方法以更新 hit testing 堆栈。

+ paintOffset 具有传递给 addWithPaintOffset 的 offset 语义。
+ paintTransform 具有传递给 addWithPaintTransform 的 transform 语义，除了它必须可逆；确保这一点是调用者的责任。
+ rawTransform 具有传递给 addWithRawTransform 的 transform 语义。其中必须有且仅有一个是非空的。

&emsp;另请参阅：

+ addWithPaintTransform，它接受一个通用的 paint transform matrix，并详细记录了此 API 的预期使用情况。

```dart
  bool addWithOutOfBandPosition({
    Offset? paintOffset,
    Matrix4? paintTransform,
    Matrix4? rawTransform,
    required BoxHitTestWithOutOfBandPosition hitTest,
  }) {    
    if (paintOffset != null) {
    
      // 与下面👇的 popTransform 对应
      pushOffset(-paintOffset);
    } else if (rawTransform != null) {
      
      // 与下面👇的 popTransform 对应
      pushTransform(rawTransform);
    } else {
      paintTransform = Matrix4.tryInvert(PointerEvent.removePerspectiveTransform(paintTransform!));
      
      // 与下面👇的 popTransform 对应 
      pushTransform(paintTransform!);
    }
    
    final bool isHit = hitTest(this);
    
    // 与上面👆的 popTransform 对应
    popTransform();
    
    return isHit;
  }
```





## 参考链接
**参考链接:🔗**
+ [HitTestTarget class](https://api.flutter.dev/flutter/gestures/HitTestTarget-class.html)
+ [HitTestResult class](https://api.flutter.dev/flutter/gestures/HitTestResult-class.html)
+ [BoxHitTestResult class](https://api.flutter.dev/flutter/rendering/BoxHitTestResult-class.html)
+ [`HitTestEntry<T extends HitTestTarget> class`](https://api.flutter.dev/flutter/gestures/HitTestEntry-class.html)
+ [BoxHitTestEntry class](https://api.flutter.dev/flutter/rendering/BoxHitTestEntry-class.html)
+ [HitTestResult class](https://api.flutter.dev/flutter/gestures/HitTestResult-class.html)
