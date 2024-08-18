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
abstract class RenderObject with DiagnosticableTreeMixin implements HitTestTarget {
  /// 重写此方法以处理击中此 render object 的 PointerEvent。
  @override
  void handleEvent(PointerEvent event, covariant HitTestEntry entry) { }
}
```

# HitTestResult

&emsp;


# BoxHitTestResult

&emsp;

# HitTestEntry

&emsp;

# BoxHitTestEntry

&emsp;

# 







## 参考链接
**参考链接:🔗**
+ [HitTestTarget class](https://api.flutter.dev/flutter/gestures/HitTestTarget-class.html)
+ [HitTestResult class](https://api.flutter.dev/flutter/gestures/HitTestResult-class.html)
+ [BoxHitTestResult class](https://api.flutter.dev/flutter/rendering/BoxHitTestResult-class.html)
+ [`HitTestEntry<T extends HitTestTarget> class`](https://api.flutter.dev/flutter/gestures/HitTestEntry-class.html)
+ [BoxHitTestEntry class](https://api.flutter.dev/flutter/rendering/BoxHitTestEntry-class.html)
      
