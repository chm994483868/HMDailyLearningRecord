# Flutter 源码梳理系列（四十）：HitTesting

# HitTestTarget

&emsp;HitTestTarget：一个可以处理 PointerEvent 事件的对象。

```dart
abstract interface class HitTestTarget {
  /// 重写此方法以接收事件。
  void handleEvent(PointerEvent event, HitTestEntry<HitTestTarget> entry);
}
```

&emsp;RenderObject 抽象类实现了 HitTestTarget 的 handleEvent 函数，但是仅是一个空实现。

```dart
// RenderObject.handleEvent：

abstract class RenderObject with DiagnosticableTreeMixin implements HitTestTarget {
  // ...
  
  /// 重写此方法以处理击中此 RenderObject 时的 PointerEvent 事件
  @override
  void handleEvent(PointerEvent event, covariant HitTestEntry entry) { }
  
  // ...
}
```

&emsp;那么到了 RenderBox 这里呢？仅调用了 super 的 handleEvent 函数，也可以直接理解为是一个空实现，其它的 RenderBox 的子类会实际根据自己的情况而重写 handleEvent 函数。

```dart
abstract class RenderBox extends RenderObject {
  // ...
  
  @override
  void handleEvent(PointerEvent event, BoxHitTestEntry entry) {
    super.handleEvent(event, entry);
  }
  
  // ...
}
```

&emsp;目前我们先不去看其它 RenderBox 子类的 handleEvent 函数的实现，接下来把目光放在 handleEvent 函数的两个参数：PointerEvent 和 HitTestEntry 上。

# HitTestEntry

&emsp;首先 HitTestEntry 是一个泛型类，而它的泛型 T 则被要求是继承自 HitTestTarget，在上面我们已经看到 RenderObject 实现了 HitTestTarget，即 RenderObject、RenderBox 都可谓是继承自 HitTestTarget。

&emsp;即这里的 HitTestEntry 泛型类的泛型参数 T 可以是：RenderObject、RenderBox 及其子类，或者可以直白一点的说所有的 RenderObject 都是可以作为 HitTestEntry 的泛型 T 的，其实同时这里也相当于是说明了，所有 RenderObject 都能作为 HitTest 的目标，所有的 RenderObject 都能进行 hitTest。

&emsp;HitTestEntry：在有关特定 HitTestTarget 的 hit test 期间收集的数据。子类化此对象（HitTestEntry）以从 hit test 阶段传递额外信息到事件（PointerEvent）传播阶段。 

## Constructors

&emsp;创建一个 hit test entry，可以看到当创建一个 HitTestEntry 对象时必须要传入 target 参数：一个 HitTestTarget 子类对象，即可以是一个 RenderObject 对象。

```dart
@optionalTypeArgs
class HitTestEntry<T extends HitTestTarget> {
  /// 创建一个 hit test entry.
  HitTestEntry(this.target);
  // ...
}
```

## target

&emsp;target 表示 hit test 命中测试过程中遇到的 HitTestTarget。（即遇到的 RenderObject 对象。）

```dart
  /// The [HitTestTarget] encountered during the hit test.
  final T target;

  @override
  String toString() => '${describeIdentity(this)}($target)';
```

## transform

&emsp;返回一个矩阵，描述了应该如何将传递给 HitTestEntry 的 PointerEvents 从屏幕的全局坐标空间转换为 target 的本地坐标空间。

&emsp;另外可参考：BoxHitTestResult.addWithPaintTransform，它在 hit testing 期间用于构建此转换。

```dart
  Matrix4? get transform => _transform;
  Matrix4? _transform;
```

## HitTestEntry 总结

&emsp;下面看第一个 HitTestEntry 的子类，也是最重要的一个子类。

# BoxHitTestEntry

&emsp;BoxHitTestEntry 是 RenderBox 所使用的一个 hit test entry。

## Constructors

&emsp;可以看到 BoxHitTestEntry 的泛型 T 被要求是 RenderBox 或其子类，并且它的构造函数在原仅有 target 参数的基础上添加了另外一个 localPosition 参数。

```dart
class BoxHitTestEntry extends HitTestEntry<RenderBox> {
  /// 创建一个 box hit test entry.
  BoxHitTestEntry(super.target, this.localPosition);
  // ...
}
```

## localPosition

&emsp;localPosition 表示在 target 的本地坐标系中的 hit test 位置。

```dart
  final Offset localPosition;
```

## BoxHitTestEntry 总结

&emsp; // .......

# HitTestResult

&emsp;HitTestResult：表示执行 hit test 的结果。

## Constructors

&emsp;创建一个空的 hit test result，这里之所以说它是空的，是因为在 HitTestResult 的初始化列表中对它的三个 List 类型的属性都设置了一个空值。

```dart
class HitTestResult {
  /// 创建一个空的 hit test result.
  HitTestResult()
     : _path = <HitTestEntry>[],
       _transforms = <Matrix4>[Matrix4.identity()],
       _localTransforms = <_TransformPart>[];
  // ...
}
```

## HitTestResult.wrap

&emsp;将结果（通常是 HitTestResult 的子类型）进行封装，以创建一个通用的 HitTestResult。添加到返回的 HitTestResult 的 HitTestEntry 也会添加到封装的结果中（它们共享同一底层数据结构来存储 HitTestEntry）。

```dart
  HitTestResult.wrap(HitTestResult result)
     : _path = result._path,
       _transforms = result._transforms,
       _localTransforms = result._localTransforms;
```

## path

&emsp;注意 path 是一个 final 属性。

&emsp;在 hit test 期间记录的 HitTestEntry 对象的不可修改列表。path 中的第一个条目是最具体的，通常是正在进行 hit test 的 Render Tree 上的叶子节点对应的 HitTestEntry。事件传播从最具体的（即第一个）HitTestEntry 开始，并按顺序通过 path 进行。

```dart
  Iterable<HitTestEntry> get path => _path;
  final List<HitTestEntry> _path;
```

## `_transforms`

&emsp;一个 transform 的堆栈。从全局到当前对象的变换部件堆栈分为 2 部分：* `_transforms` 指的是全局矩阵，意味着它们已被祖先相乘，因此是相对于全局坐标空间的。* `_localTransforms` 是相对于父级坐标空间的本地变换部件。当添加新的变换部件时，它们会被附加到 `_localTransforms` 中，仅当使用时才会被转换为全局变换并移动到 `_transforms` 中。

```dart
  final List<Matrix4> _transforms;
```



# BoxHitTestResult

&emsp;








## 参考链接
**参考链接:🔗**
+ [HitTestTarget class](https://api.flutter.dev/flutter/gestures/HitTestTarget-class.html)
+ [HitTestResult class](https://api.flutter.dev/flutter/gestures/HitTestResult-class.html)
+ [BoxHitTestResult class](https://api.flutter.dev/flutter/rendering/BoxHitTestResult-class.html)
+ [`HitTestEntry<T extends HitTestTarget> class`](https://api.flutter.dev/flutter/gestures/HitTestEntry-class.html)
+ [BoxHitTestEntry class](https://api.flutter.dev/flutter/rendering/BoxHitTestEntry-class.html)
+ [HitTestResult class](https://api.flutter.dev/flutter/gestures/HitTestResult-class.html)
