# Flutter 源码梳理系列（四十一）：HitTesting：Correlation class definition

# 前言

&emsp;上一篇我们分析了 hit test 过程中所涉及的函数调用流程，其中涉及到的类定义还是很多的，本篇我们对其中涉及到的类的内容做一个快速浏览，看看它们的定义中都有哪些内容。

&emsp;这次我们也反向来看。在上篇中我们知道最后在 GestureBinding.dispatchEvent 函数中会依次遍历 hitTestResult.path 中的 HitTestEntry，执行 entry.target 的 handleEvent 函数，而 entry.target 便是 HitTestTarget 类，所以下面我们先看一下 HitTestTarget。

# HitTestTarget

&emsp;HitTestTarget：表示可以处理 PointerEvent 的抽象接口类，仅有一个 handleEvent 抽象函数需要实现。如 RenderObject 及其所有子类都是 HitTestTarget，它们可根据自身情况实现 handleEvent 抽象函数，即它们都能处理 PointerEvent 事件。在 Flutter framework 中也仅有 RenderObject 及其子类可以处理 PointerEvent 事件。

```dart
abstract interface class HitTestTarget {
  /// 重写此方法以接收事件。
  void handleEvent(PointerEvent event, HitTestEntry<HitTestTarget> entry);
}
```

&emsp;RenderObject 抽象类实现了 HitTestTarget 的 handleEvent 函数，不过仅是一个空的实现。

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

&emsp;那么到了 RenderBox 这里呢？它重写 handleEvent 函数也仅是调用了 super 的 handleEvent 函数，也可以直接理解为是一个空实现，其它的 RenderBox 的子类则会实际根据自己的需要而重写 handleEvent 函数。

```dart
abstract class RenderBox extends RenderObject {
  // ...
  
  @override
  void handleEvent(PointerEvent event, BoxHitTestEntry entry) {
    // 这里 RenderBox 仅是调用 super.handleEvent 函数，即 RenderObject 的 handleEvent 函数
    super.handleEvent(event, entry);
  }
  
  // ...
}
```

&emsp;目前我们先不去看其它 RenderBox 子类的 handleEvent 函数的实现，接下来把目光放在 handleEvent 函数的两个参数：PointerEvent 和 HitTestEntry 上。

&emsp;首先是 BoxHitTestEntry entry 参数，其实就是执行 handleEvent 函数的 entry.target 的 entry：`entry.target.handleEvent(event.transformed(entry.transform), entry)`，在 GestureBinding 的 dispatchEvent 函数中，看到遍历 hitTestResult.path 时是直接把 HitTestEntry entry 作为了 entry.target.handleEvent 函数调用的 BoxHitTestEntry entry 参数，而 PointerEvent event 参数呢，则是本次要处理的事件，且在整个 hitTestResult.path 的遍历过程中调用每个 entry.target 的 handleEvent 事件所用的 PointerEvent event 参数都是同一个。

&emsp;下面看一下 HitTestEntry entry 以及它的 target 属性是什么。 

# HitTestEntry

&emsp;首先 HitTestEntry 是一个泛型类，而它的泛型 T 则被要求是继承自 HitTestTarget 的抽象类，在上面我们已经看到了 RenderObject 实现 HitTestTarget 的抽象函数，所以 HitTestEntry 的泛型 T 可以是 RenderObject、RenderBox 及其子类。或者可以直白一点的说所有的 RenderObject 都是可以作为 HitTestEntry 的泛型 T 的，同时这里也相当于是说明了：所有 RenderObject 都能作为 HitTest 的目标，所有的 RenderObject 都能进行 hit test，或者说是在 Flutter framework 中 hit test 是仅仅针对 RenderObject 进行的一个过程，其它类则不能进行 hit test。

&emsp;HitTestEntry：在有关特定 HitTestTarget 的 hit test 期间收集的数据。子类化 HitTestEntry 以从 hit test 阶段传递额外信息到事件（PointerEvent）传播阶段。 

## Constructors

&emsp;创建一个 HitTestEntry，可以看到当创建一个 HitTestEntry 对象时必须要传入 target 参数，而 target 参数的类型被要求是一个 HitTestTarget，即可以是一个 RenderObject、RenderBox 及其子类。

```dart
@optionalTypeArgs
class HitTestEntry<T extends HitTestTarget> {
  /// 创建一个 HitTestEntry，需要传入 target 参数。
  HitTestEntry(this.target);
  // ...
}
```

## target

&emsp;target 表示 hit test 命中测试过程中遇到的 HitTestTarget（即遇到的 RenderObject 对象）。

```dart
  final T target;

  @override
  String toString() => '${describeIdentity(this)}($target)';
```

## transform

&emsp;返回一个 Matrix4，描述了应该如何将传递给 HitTestEntry 的 PointerEvents 从屏幕的全局坐标空间转换为 target 的本地坐标空间。

```dart
  Matrix4? get transform => _transform;
  Matrix4? _transform;
```

&emsp;HitTestEntry 的内容就这么多，下面看第一个 HitTestEntry 的子类，也是最重要的一个子类。

# BoxHitTestEntry

&emsp;BoxHitTestEntry 是 RenderBox 所使用的一个 HitTestEntry。

## Constructors

&emsp;可以看到 BoxHitTestEntry 的泛型 T 被要求是 RenderBox 或其子类，并且它的构造函数在原仅有 target 参数的基础上添加了另外一个 localPosition 参数。

```dart
class BoxHitTestEntry extends HitTestEntry<RenderBox> {
  /// 创建一个 BoxHitTestEntry 对象，并需要传入 target 和 localPosition 参数
  BoxHitTestEntry(super.target, this.localPosition);
  // ...
}
```

## localPosition

&emsp;localPosition 表示在 target 的本地坐标系中进行 hit test 时的 Offset。

```dart
  final Offset localPosition;
```

&emsp;BoxHitTestEntry 的内容就这么多。

&emsp;下面我们看最重要的 HitTestResult 类，它的首次使用出现在 GestureBinding 的 `_handlePointerEventImmediately` 函数中，当自 Render tree 的根节点开始进行 hit test 之前，就需要创建一个 HitTestResult 对象，用它来记录本次 hit test 的结果。

```dart
HitTestResult? hitTestResult;
// ...
hitTestResult = HitTestResult();
hitTestInView(hitTestResult, event.position, event.viewId);
// ...
```

&emsp;可看到直接以 hitTestResult 为参数调用 hitTestInView 函数，开始本次的 hit test 过程。

# HitTestResult

&emsp;HitTestResult：表示执行 hit test 的结果。

## Constructors

&emsp;创建一个空的 HitTestResult，这里之所以说它是空的，是因为在此构造函数的初始化列表中 HitTestResult 会对它的三个 List 类型的属性设置一个空的 List。

+ `_path = <HitTestEntry>[]`
+ `_transforms = <Matrix4>[Matrix4.identity()],`
+ `_localTransforms = <_TransformPart>[]`

```dart
class HitTestResult {
  /// 创建一个空的 HitTestResult 对象
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

&emsp;注意 path 是一个 `List<HitTestEntry>` 类型的 final 属性，它是在 hit test 期间记录 HitTestEntry 对象的不可修改列表。path 中的第一个条目是最具体的，通常是正在进行 hit test 的 Render Tree 上的叶子节点对应的 HitTestEntry。事件传播则从最具体的（即第一个）HitTestEntry 开始，并按顺序通过 path 进行。

&emsp;如上篇示例代码中第一个被加入到 hitTestResult.path 的便是我们的 'click me' 对应的 TextSpan，而最后一个则是我们的 Render Tree 的根节点，即可以把 hit test 理解为是深度优先遍历的，第一个加入 HitTestResult 的是 depth 最深的。

```dart
  Iterable<HitTestEntry> get path => _path;
  final List<HitTestEntry> _path;
```

## `_transforms` & `_localTransforms`

&emsp;一个 `_TransformPart` 的堆栈。

&emsp;从全局到当前对象的 `_TransformPart` 堆栈分为两部分存储：

+ `_transforms` 是全局 `_TransformPart`，意味着它们已经乘以了祖先矩阵，因此相对于全局坐标空间。
+ `_localTransforms` 是本地 `_TransformPart`，相对于父级的坐标空间。

&emsp;当新的 `_TransformPart` 被添加时，它们会被追加到 `_localTransforms` 中，只有当它们被使用时，才会被转换为全局 `_TransformPart` 并移动到 `_transforms` 中。

```dart
  final List<Matrix4> _transforms;
  final List<_TransformPart> _localTransforms;
```

## `_globalizeTransforms`

&emsp;将 `_localTransforms` 中的所有 `_TransformPart` 全局化，并将它们移动到 `_transforms` 中。

```dart
  void _globalizeTransforms() {
    // 如果当前 _localTransforms list 为空，则直接 return
    if (_localTransforms.isEmpty) {
      return;
    }
    
    // 取出 _transforms 的最后一个矩阵
    Matrix4 last = _transforms.last;
    
    // for in 循环，遍历 _localTransforms list 中的所有 _TransformPart 与 last 进行相乘
    for (final _TransformPart part in _localTransforms) {
      last = part.multiply(last);
      
      // 然后把相乘结果存入 _transforms 中去
      _transforms.add(last);
    }
    
    // 由于已经把 _localTransforms 中的 _TransformPart 全部转换到 _transforms 中去了，所以把 _localTransforms 直接清空即可
    _localTransforms.clear();
  }
  
  
  Matrix4 get _lastTransform {
    _globalizeTransforms();
    return _transforms.last;
  }
```

## add 

&emsp;将一个 HitTestEntry 添加到 `_path` 中。新的 HitTestEntry 会被添加到 `_path` 的末尾，这意味着 HitTestEntry 应该按照从最具体到最不具体的顺序进行添加，通常是在进行 hit test 的树结构向上遍历的过程中。

```dart
  void add(HitTestEntry entry) {
    // 并且把 _transforms 列表中的最后一个 Matrix4 赋值给 entry 的 _transform
    entry._transform = _lastTransform;
    
    _path.add(entry);
  }
```

## pushTransform

&emsp;将一个新的 Matrix4 transform 添加到 `_localTransforms` 列表中，该 Matrix4 transform 将被应用到所有后续通过 add 添加的 HitTestEntry 中，直到通过 popTransform 移除此 Matrix4 transform 为止。

&emsp;这个方法只能被 HitTestResult 子类使用，HitTestResult 子类必须为它们的用户提供特定于坐标空间的公共包装器来调用此功能（可参考 BoxHitTestResult.addWithPaintTransform 方法作为一个例子）。

&emsp;提供的 Matrix4 transform 应描述如何将 PointerEvents 从调用方法的坐标空间转换到其子级 RenderObject 的坐标空间。在大多数情况下，transform 是从应用 RenderObject.applyPaintTransform 的反转结果中移除 perspective component 而得到的。

&emsp;如果提供的 Matrix4 transform 是一个平移矩阵（translation matrix），那么使用具有平移偏移量（translation offset）的 pushOffset 方法要快得多。

&emsp;在对子级 RenderObject 进行 hit test 之前，HitTestables 需要通过子类定义的便利方法间接调用此方法，以移除与父级 RenderObject 不同源点的子级 RenderObject 的 transform。hit testing 完子级 RenderObject 后，需要调用 popTransform 来移除特定于子级 RenderObject 的 transform。

&emsp;另请参阅:

+ pushOffset，类似于 pushTransform，但仅限于平移，在这种情况下更快。
+ BoxHitTestResult.addWithPaintTransform，这是对 RenderBox 进行 hit test 的公共包装器。

```dart
  @protected
  void pushTransform(Matrix4 transform) {
    // 仅仅是把 Matrix4 transform 通过 _MatrixTransformPart 转换为一个 _TransformPart，并添加到 _localTransforms 列表中
    _localTransforms.add(_MatrixTransformPart(transform));
  }
```

## pushOffset

&emsp;添加一个新的 translation offset，通过 `_OffsetTransformPart` 函数把入参 Offset offset 转换为 `_TransformPart` 并添加到 `_localTransforms` 列表中。该 Offset offset 将被应用到所有后续通过 add 添加的 HitTestEntry 中，直到通过 popTransform 将其移除为止。

```dart
  @protected
  void pushOffset(Offset offset) {
    // 同上面的 pushTransform 函数，向 _localTransforms 列表中添加一个 _TransformPart
    _localTransforms.add(_OffsetTransformPart(offset));
  }
```

## popTransform

&emsp;移除通过 pushTransform 或 pushOffset 添加到 `_localTransforms` 列表中的最后一个 `_TransformPart`。

&emsp;这个方法只能由 HitTestResult 子类使用，HitTestResult 子类必须为他们的用户提供特定于坐标空间的公共包装函数，这些函数围绕这个功能进行封装（例如，可以参考 BoxHitTestResult.addWithPaintTransform）。

&emsp;这个方法必须在对需要调用 pushTransform 或 pushOffset 的子级 RenderObject 进行 hit test 之后调用。

```dart
  @protected
  void popTransform() {
    if (_localTransforms.isNotEmpty) {
      _localTransforms.removeLast();
    } else {
      _transforms.removeLast();
    }
  }
```

&emsp;OK，HitTestResult 就这么多内容。本篇我们看了三个 hit test 过程中最基础的数据结构：HitTestTarget、HitTestEntry、HitTestResult。下篇我们继续，则把重点放在 PointerEvent 是如何从屏幕的全局坐标空间转换为 target 的本地坐标空间。

## 参考链接
**参考链接:🔗**
+ [HitTestTarget class](https://api.flutter.dev/flutter/gestures/HitTestTarget-class.html)
+ [HitTestResult class](https://api.flutter.dev/flutter/gestures/HitTestResult-class.html)
+ [BoxHitTestResult class](https://api.flutter.dev/flutter/rendering/BoxHitTestResult-class.html)
+ [`HitTestEntry<T extends HitTestTarget> class`](https://api.flutter.dev/flutter/gestures/HitTestEntry-class.html)
+ [BoxHitTestEntry class](https://api.flutter.dev/flutter/rendering/BoxHitTestEntry-class.html)
+ [HitTestResult class](https://api.flutter.dev/flutter/gestures/HitTestResult-class.html)
