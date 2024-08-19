# Flutter 源码梳理系列（四十）：HitTesting

# HitTestTarget

&emsp;HitTestTarget：一个可以处理 PointerEvent 事件的对象。

```dart
abstract interface class HitTestTarget {
  /// 重写此方法以接收事件。
  void handleEvent(PointerEvent event, HitTestEntry<HitTestTarget> entry);
}
```

&emsp;RenderObject 实现了 HitTestTarget 的 handleEvent 函数，但是仅是一个空实现。

```dart
// RenderObject.handleEvent：

abstract class RenderObject with DiagnosticableTreeMixin implements HitTestTarget {
  // ...
  
  /// 重写此方法以处理击中此 render object 的 PointerEvent。
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

&emsp;首先 HitTestEntry 是一个泛型类，而它的泛型 T 则被要求是 HitTestTarget 或其子类。而上面我们学习时看到 RenderObject 实现了 HitTestTarget，即 RenderObject、RenderBox 都是 HitTestTarget 的子类。

&emsp;即这里的 HitTestEntry 泛型类的泛型参数 T 可以是：RenderObject、RenderBox 及其子类，或者可以直白一点的说所有的 RenderObject 都是可以作为 HitTestEntry 的泛型 T 的，其实同时这里也相当于是说明了，所有 RenderObject 都能作为 HitTest 的目标，所有的 RenderObject 都能进行 hitTest。

&emsp;HitTestEntry：在有关特定 HitTestTarget 的 hit test 期间收集的数据。子类化此对象（HitTestEntry）以从 hit test 阶段传递额外信息到事件（PointerEvent）传播阶段。 







# BoxHitTestEntry

&emsp;

# 

# HitTestResult

&emsp;


# BoxHitTestResult

&emsp;








## 参考链接
**参考链接:🔗**
+ [HitTestTarget class](https://api.flutter.dev/flutter/gestures/HitTestTarget-class.html)
+ [HitTestResult class](https://api.flutter.dev/flutter/gestures/HitTestResult-class.html)
+ [BoxHitTestResult class](https://api.flutter.dev/flutter/rendering/BoxHitTestResult-class.html)
+ [`HitTestEntry<T extends HitTestTarget> class`](https://api.flutter.dev/flutter/gestures/HitTestEntry-class.html)
+ [BoxHitTestEntry class](https://api.flutter.dev/flutter/rendering/BoxHitTestEntry-class.html)
      
