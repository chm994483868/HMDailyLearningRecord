# Flutter 源码梳理系列（四十五）：HitTesting：Coordinate-space Transformations three

# RenderAbsorbPointer.hitTest

&emsp;RenderAbsorbPointer 也是一个直接继承自 RenderProxyBox 的 RenderProxyBox 子类，可以把它理解为一种特殊的 RenderBox。它用于取消往其子级中传递 hit testing，如果它的 absorbing getter 返回 true，则表示当 hit testing 进行到这个 RenderAbsorbPointer 节点时，就要结束 hit testing 了，不会再往其子级中进行 hit test 了，而结束 hit testing 的方式则是重写 hitTest 函数，直接返回 true/false，而不是像 RenderBox.hitTest 中那样，继续向子级中进行 hit testing 探测，找到更加具体的可以响应 hit test 的 RenderBox 节点。 

```dart
class RenderAbsorbPointer extends RenderProxyBox { //... }
```

&emsp;RenderAbsorbPointer 是一个在 hit testing 期间 "吸收" 指针事件的渲染对象。

&emsp;当 absorbing getter 返回 true 时，这个渲染对象通过在自身结束 hit testing 来阻止其子级接收指针事件。它在布局期间仍然占据空间，并按照通常方式呈现其子级。它只是阻止其子级成为 located events 的目标，因为其渲染对象在 hitTest 中返回 true。

&emsp;然后看一下 RenderAbsorbPointer 的 hitTest 函数的代码实现，如果 absorbing getter 返回 true，则直接返回 size.contains(position) 的值 true/false，如果 absorbing getter 返回 false 的话，则调用 super.hitTest 函数，继续向子级中传递 hit testing。

```dart
  @override
  bool hitTest(BoxHitTestResult result, { required Offset position }) {
    return absorbing
        ? size.contains(position)
        : super.hitTest(result, position: position);
  }
```

&emsp;看到当前函数堆栈中 this 指针指向的 RenderAbsorbPointer 实例对象的 `_absorbing` 属性值是 false，所以此处继续向子级中传递 hit testing 探测。

# `_RenderTheaterMixin.hitTestChildren`

&emsp;然后接下来又一个新的函数调用：`_RenderTheaterMixin.hitTestChildren`，看到此时的 this 指针指向的是 `_RenderTheater`，由于 `_RenderTheater` 混入了 `_RenderTheaterMixin`，且 `_RenderTheater` 自身并没有重写 hitTestChildren 函数，所以这里调用到了它的父类 `_RenderTheaterMixin` 的 hitTestChildren 函数。

```dart
class _RenderTheater extends RenderBox with ContainerRenderObjectMixin<RenderBox, StackParentData>, _RenderTheaterMixin { //... }
```

&emsp;`_RenderTheaterMixin` 是一个 RenderBox 子类，其大小根据其父级大小自动调整，实现堆栈布局算法（stack layout algorithm），并在给定的 `theater` 中渲染其子级节点。

```dart
mixin _RenderTheaterMixin on RenderBox { //... }
```

&emsp;然后是 `_RenderTheaterMixin` 的 hitTestChildren 函数实现，可看到里面对 BoxHitTestResult.addWithPaintOffset 的调用。

```dart
  @override
  bool hitTestChildren(BoxHitTestResult result, { required Offset position }) {
    
    // 所有的子级组成的迭代器。
    final Iterator<RenderBox> iterator = _childrenInHitTestOrder().iterator;
    
    // 标记是否已经被命中的临时变量。
    bool isHit = false;
    
    // 遍历所有的子级，未命中并且还有子级，则继续向子级中进行 hit testing。
    while (!isHit && iterator.moveNext()) {
      final RenderBox child = iterator.current;
      
      // 取得子级的 parentData，注意这里取得的 StackParentData 是当前 child 的 ParentData。
      final StackParentData childParentData = child.parentData! as StackParentData;
      
      final RenderBox localChild = child;
      
      // 函数指针，参数依然是 result 和 position，这里则是把它作为一个参数调用 BoxHitTestResult 的 addWithPaintOffset 函数。
      bool childHitTest(BoxHitTestResult result, Offset position) => localChild.hitTest(result, position: position);
      
      // 在 addWithPaintOffset 内部，首先对入参 position 进行 childParentData.offset 的偏移，然后再执行 childHitTest 函数。
      // 即对 Offset position 参数进行 StackParentData.offest 的偏移，然后再对结果坐标进行 hit testing。
      isHit = result.addWithPaintOffset(offset: childParentData.offset, position: position, hitTest: childHitTest);
    }
    
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
+ [PlatformDispatcher class](https://api.flutter.dev/flutter/dart-ui/PlatformDispatcher-class.html)
+ [Zone class](https://api.flutter.dev/flutter/dart-async/Zone-class.html)
+ [RenderAbsorbPointer class](https://api.flutter.dev/flutter/rendering/RenderAbsorbPointer-class.html)
