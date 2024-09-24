# Flutter 源码梳理系列（四十四）：HitTesting：Coordinate-space Transformations two

# `GestureBinding._flushPointerEventQueue` 

&emsp;接上篇，`_handlePointerDataPacket` 函数内会进行 PointerData 的数据处理，会把入参传递来的 `ui.PointerDataPacket packet` 参数的 data 属性：`final List<PointerData> data` 列表中的 PointerData 转换为对应类型的 PointerEvent，目前通过打印看到实际是转换了两个 PointerEvent 的子类实例对象：PointerAddedEvent 和 PointerDownEvent，并且它们两个的 postion 属性值都是：`Offset(194.7, 163.7)`。

&emsp;当把点击事件的数据转化为 PointerEvent 并收集在 `_pendingPointerEvents` 队列中，接下来便是循环遍历此队列中的 PointerEvent 对象了，即开始执行 `_flushPointerEventQueue` 函数。

```dart
  void _flushPointerEventQueue() {
  
    // 循环遍历 _pendingPointerEvents 中的 PointerEvent 对象，对此 Pointer 事件进行处理。
    while (_pendingPointerEvents.isNotEmpty) {
      handlePointerEvent(_pendingPointerEvents.removeFirst());
    }
  }
```

&emsp;在 `_flushPointerEventQueue` 内会对 `_pendingPointerEvents` 队列中的 PointerEvent 从头开始进行处理，在我们的实例代码中，看到第一个被处理的是 PointerDownEvent 实例对象。

# `GestureBinding.handlePointerEvent & GestureBinding._handlePointerEventImmediately`

&emsp;实际 handlePointerEvent 只是 `_handlePointerEventImmediately` 函数的超简单包装，在其内部添加了一些 debug 下的测试代码。

&emsp;handlePointerEvent 函数的官方注释是：将事件（PointerEvent event）分派给 hit test 在其 position 上找到的 target。这个方法根据事件类型（不同的 PointerEvent 子类，是不同的事件类型）将给定的事件发送给 dispatchEvent:（即：`void dispatchEvent(PointerEvent event, HitTestResult? hitTestResult)` 函数，hit test 结束后找到了 target，然后带着参数调用 dispatchEvent 函数。）。

```dart
  void handlePointerEvent(PointerEvent event) {
    // ⬇️ 测试代码。
    if (resamplingEnabled) {
      _resampler.addOrDispatch(event);
      _resampler.sample(samplingOffset, samplingClock);
      return;
    }

    // Stop resampler if resampling is not enabled. This is a no-op if resampling was never enabled.
    _resampler.stop();
    
    // ⬆️ 以上都是测试代码。
    
    // 实际立即执行：handlePointerEvent。
    _handlePointerEventImmediately(event);
  }
```

&emsp;实际直接调用 `_handlePointerEventImmediately` 函数，注意⚠️当前的 event 参数的类型是：PointerDownEvent。

```dart
  void _handlePointerEventImmediately(PointerEvent event) {
    HitTestResult? hitTestResult;
    
    if (event is PointerDownEvent ||
        event is PointerSignalEvent ||
        event is PointerHoverEvent ||
        event is PointerPanZoomStartEvent) {
        
      // 
      hitTestResult = HitTestResult();
      hitTestInView(hitTestResult, event.position, event.viewId);
      
      if (event is PointerDownEvent ||
          event is PointerPanZoomStartEvent) {
        
        // 缓存本次 hit test 的 TestResult。
        _hitTests[event.pointer] = hitTestResult;
      }
    } else if (event is PointerUpEvent ||
               event is PointerCancelEvent ||
               event is PointerPanZoomEndEvent) {
               
      //
      hitTestResult = _hitTests.remove(event.pointer);
    } else if (event.down ||
               event is PointerPanZoomUpdateEvent) {
      
      // 因为 pointer 向下发生的事件(如 PointerMoveEvent)应该被分派到它们最初的 PointerDownEvent 所在的相同位置，
      // 我们希望重用 pointer 向下时找到的 path，而不是每次得到这样的事件时都进行 hit test。
      
      // 即这里是把上次 hit test 缓存的 TestResult 取出来。
      hitTestResult = _hitTests[event.pointer];
    }

    if (hitTestResult != null ||
        event is PointerAddedEvent ||
        event is PointerRemovedEvent) {
        
      // 
      dispatchEvent(event, hitTestResult);
    }
  }
```


# RendererBinding.hitTestInView

# RenderView.hitTest

# RenderBox.hitTest






## 参考链接
**参考链接:🔗**
+ [HitTestTarget class](https://api.flutter.dev/flutter/gestures/HitTestTarget-class.html)
+ [HitTestResult class](https://api.flutter.dev/flutter/gestures/HitTestResult-class.html)
+ [BoxHitTestResult class](https://api.flutter.dev/flutter/rendering/BoxHitTestResult-class.html)
+ [`HitTestEntry<T extends HitTestTarget> class`](https://api.flutter.dev/flutter/gestures/HitTestEntry-class.html)
+ [BoxHitTestEntry class](https://api.flutter.dev/flutter/rendering/BoxHitTestEntry-class.html)
+ [HitTestResult class](https://api.flutter.dev/flutter/gestures/HitTestResult-class.html)
+ [PlatformDispatcher class](https://api.flutter.dev/flutter/dart-ui/PlatformDispatcher-class.html)
+ [Zone class](https://api.flutter.dev/flutter/dart-async/Zone-class.html)
