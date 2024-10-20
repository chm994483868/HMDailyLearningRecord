# Flutter 源码梳理系列（四十五）：HitTesting：Coordinate-space Transformations three

# 前言

&emsp;下面我们继续看 hit testing 最终会遇到的那些需要进行 Position 计算的特殊的 RenderBox 节点。 

# RenderAbsorbPointer.hitTest

&emsp;RenderAbsorbPointer 也是一个直接继承自 RenderProxyBox 的 RenderProxyBox 子类，也可以把它理解为一种有特殊作用的 RenderBox 节点，它的作用如它的名字：Absorb(吸收、吸纳) 一样，它用于取消往其子级中传递 hit testing，如果它的 `absorbing getter` 返回 true，则表示当 hit testing 进行到这个 RenderAbsorbPointer 节点时，就要结束 hit testing 了，不会再往其子级中进行 hit test 了，而结束 hit testing 的方式则是 RenderAbsorbPointer 自己进行 hitTest 函数的重写并直接返回 true/false，类比它的父类 RenderBox 的 hitTest 函数则是继续向子级中进行 hit testing，去找到更加具体的可以响应 hit test 的 RenderBox 节点。 

```dart
class RenderAbsorbPointer extends RenderProxyBox { //... }
```

&emsp;RenderAbsorbPointer 是一个在 hit testing 期间 "吸收" 指针事件的渲染对象。

&emsp;当 absorbing getter 返回 true 时，这个渲染对象通过在自身结束 hit testing 来阻止其子级接收指针事件。它在布局期间仍然占据空间，并按照通常方式呈现其子级。它只是阻止其子级成为 located events 的目标，因为其渲染对象在 hitTest 中返回 true。

&emsp;然后看一下 RenderAbsorbPointer 的 hitTest 函数的代码实现：如果 `absorbing getter` 返回 true，则直接返回 size.contains(position) 的值：true/false，如果 `absorbing getter` 返回 false 的话，则调用 super.hitTest 函数，继续向子级中传递 hit testing。

```dart
  @override
  bool hitTest(BoxHitTestResult result, { required Offset position }) {
    return absorbing
        ? size.contains(position)
        : super.hitTest(result, position: position);
  }
```

&emsp;看到当前函数堆栈中 this 指针指向的 RenderAbsorbPointer 实例对象的 `_absorbing` 属性值是 false，所以此处会继续向子级中传递 hit testing。

# `_RenderTheaterMixin.hitTestChildren`

&emsp;然后接下来又一个新的函数调用：`_RenderTheaterMixin.hitTestChildren`，看到此时的 this 指针指向的是 `_RenderTheater`，由于 `_RenderTheater` 混入了 `_RenderTheaterMixin`，且 `_RenderTheater` 自身并没有重写 hitTestChildren 函数，所以这里 this 调用到了它的父类 `_RenderTheaterMixin` 的 hitTestChildren 函数。

```dart
class _RenderTheater extends RenderBox with ContainerRenderObjectMixin<RenderBox, StackParentData>, 
                                            _RenderTheaterMixin { //... }
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
      
      // ⚠️ 函数指针，参数依然是 result 和 position，这里则是把它作为一个参数调用 BoxHitTestResult 的 addWithPaintOffset 函数。
      bool childHitTest(BoxHitTestResult result, Offset position) => localChild.hitTest(result, position: position);
      
      // ⚠️ 在 addWithPaintOffset 内部，首先对入参 position 进行 childParentData.offset 的偏移，然后再执行 childHitTest 函数。
      // 即对 Offset position 参数进行 StackParentData.offest 的偏移，然后再对结果坐标进行 hit testing。
      isHit = result.addWithPaintOffset(offset: childParentData.offset, position: position, hitTest: childHitTest);
    }
    
    return isHit;
  }
```

&emsp;那么这里的: **在 addWithPaintOffset 内部，首先对入参 position 进行 childParentData.offset 的偏移，然后再执行 childHitTest 函数。**怎么理解呢？其实也是很简单的，已知入参 position 是相对于屏幕坐标系的点，那么现在这个相对于屏幕有偏移的 RenderBox 节点进行 hit testing，那么就需要把入参 position 减去这个 RenderBox 节点的偏移量：StackParentData.offest，这样得到的 position 就是相对于当前进行 hit testing 的这个 RenderBox 节点的坐标系的点了。

![截屏2024-10-16 23.30.32.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/4318eee3240a4bb6a43dacdbb1da6b20~tplv-73owjymdk6-jj-mark-v1:0:0:0:0:5o6Y6YeR5oqA5pyv56S-5Yy6IEAg6bOE6bG85LiN5oCVX-eJmeWMu-S4jeaAlQ==:q75.awebp?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1729179129&x-orig-sign=HbZp9NawCBXd1sXTCHgmCM5nTK8%3D)

&emsp;由于当前这个 this 指针指向的 `_RenderTheater` 的 childParentData.offset 的值是：Offset(0.0, 0.0)，所以我们暂时看不到什么效果，后面我们则会遇到 offset 有值的情况，我们继续。

# RenderOffstage.hitTest

&emsp;RenderOffstage 也是一个直接继承自 RenderProxyBox 的 RenderProxyBox 子类，也可以把它理解为一种有特殊作用的 RenderBox 节点，它的作用如它的名字：Offstage(译为不在舞台上的) 一样，它添加了一个 offstage 属性，用来指示是否正常展示子级，如果 RenderProxyBox 创建时入参 offstage 为 false 的话，则不展示子级了，它通过重写一众 Layout/Paint 的函数来实现此功能，当 offstage 为 true 的话，基本是完全不进行 布局 和 绘制了。

&emsp;同时也重写了 hitTest 函数，可看到如果 offstage 属性为 true 的话，hitTest 就直接返回 false 了。（offstage 译为：不在舞台上了。）

```dart
  @override
  bool hitTest(BoxHitTestResult result, { required Offset position }) {
    // 如果 offstage 属性为 true 的话，此 hitTest 函数直接返回 false，表示此时 自己 和 自己的子级都不会进行 hit testing 了。
    return !offstage && super.hitTest(result, position: position);
  }
```

&emsp;上面我们看到 `_RenderTheaterMixin.hitTestChildren` 函数中 childParentData.offset 的使用，但是 offset 的值是 Offset(0.0, 0.0) 的，下面则是到了 childParentData.offset 有值的情况！

# RenderBoxContainerDefaultsMixin.defaultHitTestChildren

&emsp;当 hit testing 进行到 this 指针指向的是 depth 是 26 的 RenderCustomMultiChildLayoutBox 时，看到一切都变得不一样起来。首先看下 RenderCustomMultiChildLayoutBox 的定义：

```dart
class RenderCustomMultiChildLayoutBox extends RenderBox with
  ContainerRenderObjectMixin<RenderBox, MultiChildLayoutParentData>,
  RenderBoxContainerDefaultsMixin<RenderBox, MultiChildLayoutParentData> {
  //...
}
```

&emsp;RenderCustomMultiChildLayoutBox 直接继承自 RenderBox 并混入了：`ContainerRenderObjectMixin<RenderBox, MultiChildLayoutParentData>` 和 `RenderBoxContainerDefaultsMixin<RenderBox, MultiChildLayoutParentData>`。

&emsp; RenderCustomMultiChildLayoutBox 是一个特殊的 RenderBox 子类，它将多个子级的布局推迟到委托对象（`MultiChildLayoutDelegate _delegate`）。委托对象可以确定每个子级的布局约束，并决定每个子级的位置。委托对象还可以确定父组件的大小，但是父组件的大小不能取决于子组件的大小。

&emsp;浏览 RenderCustomMultiChildLayoutBox 类的代码，看到它的构造函数必须要传入一个 delegate 参数：`required MultiChildLayoutDelegate delegate`，然后便是其它 RenderCustomMultiChildLayoutBox 类中重写的各个父类的函数，其内部实现几乎全部由 delegate 属性一手包办。这里我们主要关注它的 hitTestChildren 函数，看到内部是直接调用父类 RenderBoxContainerDefaultsMixin 的 defaultHitTestChildren 函数。

```dart
  @override
  bool hitTestChildren(BoxHitTestResult result, { required Offset position }) {
    
    // 直接调用父类 RenderBoxContainerDefaultsMixin 的 defaultHitTestChildren 函数。
    return defaultHitTestChildren(result, position: position);
  }
```

&emsp;然后看一下 RenderBoxContainerDefaultsMixin 的定义，看到它是一个继承自 ContainerRenderObjectMixin 的 mixin，它为具有由 ContainerRenderObjectMixin mixin 管理的子级的 RenderBox 提供有用的默认行为(通过 4 个 default 函数进行)，其中最重要的便是 defaultHitTestChildren 函数了。

```dart
mixin RenderBoxContainerDefaultsMixin<ChildType extends RenderBox, ParentDataType extends ContainerBoxParentData<ChildType>> implements ContainerRenderObjectMixin<ChildType, ParentDataType> {
  
    double? defaultComputeDistanceToFirstActualBaseline(TextBaseline baseline) { //... }
    double? defaultComputeDistanceToHighestActualBaseline(TextBaseline baseline) { //... }
    
    // 👇
    bool defaultHitTestChildren(BoxHitTestResult result, { required Offset position }) { //... }
    // 👆
    
    void defaultPaint(PaintingContext context, Offset offset) { //... }
    List<ChildType> getChildrenAsList() { //... }
}
```

&emsp;下面看一下 RenderBoxContainerDefaultsMixin.defaultHitTestChildren 函数的实现：**通过向后遍历子级列表，在每个子级上执行一个 hit testing，一旦第一个子级报告包含给定的 position，就停止遍历，返回是否有任何子级包含给定的 position。**

```dart
  bool defaultHitTestChildren(BoxHitTestResult result, { required Offset position }) {
    ChildType? child = lastChild;
    
    while (child != null) {
      // The x, y parameters have the top left of the node's box as the origin.
      
      // 首先读取当前 child 的 parentData 数据，当前 childParentData 的类型是 MultiChildLayoutParentData。 
      final ParentDataType childParentData = child.parentData! as ParentDataType;
      
      // 调用 BoxHitTestResult.addWithPaintOffset 函数，⚠️ 注意这里的 childParentData.offset，
      // 当前它的值是：Offset(0.0, 115.0)。是的，没错，此时需要对我们前面一路看来的基于屏幕坐标系的 position 点下手操作了。
      final bool isHit = result.addWithPaintOffset(
        offset: childParentData.offset,
        position: position,
        hitTest: (BoxHitTestResult result, Offset transformed) {
          return child!.hitTest(result, position: transformed);
        },
      );
      
      // 任何一个 child 被 hit 后，就直接 return，不会再向后遍历进行 hit testing 了。
      if (isHit) {
        return true;
      }
      
      // 更新 child，向下一个子级节点进行 hit testing。
      child = childParentData.previousSibling;
    }
    
    return false;
  }
```

&emsp;下面我们专注看一下 RenderBoxContainerDefaultsMixin.defaultHitTestChildren 函数执行时，各个变量的值。

+ this: 可看到当前 this 指向的类型是：RenderCustomMultiChildLayoutBox，它的 depth 是：26，它的 size 是：Size(393.0, 852.0)，它的 constraints 是：`BoxConstraints` 类型，其中 minWidth & maxWidth 是：393.0，minHeight & maxHeight 是：852.0。它的 childCount 值是 4，即有 4 个子级节点。
+ position: 在当前屏幕坐标系内以逻辑像素为单位的值是：Offset(193.3, 161.7)。（由于在梳理代码过程中，要多次调试运行项目，所以并不能保证每次鼠标都能正好点中我们实例代码中黄色区域的正中心位置，所以这里会有一丁点的偏差。）
+ child: 可看到它是 this 的第一个子级节点，此时 child 指向的类型是：RenderPositionedBox，它的 depth 是：27，它的 size 是：Size(393.0， 737.0)，它的 constraints 是：`_BodyBoxConstraints` 类型，其中 minWidth：0，maxWidth：393.0，minHeight：0，maxHeight：737.0，bottomWidgetsHeight：0，appBarHeight：115.0，materialBannerHeight：0，特别注意一下：appBarHeight 的值是 115。
+ childParentData: 即 child.parentData 的值，可看到此时 child 的 parentData 属性是有值的，类型是 MultiChildLayoutParentData，且它的 offset 属性的值是：Offset(0.0, 115.0)，id 属性是：`_ScaffoldSlot.body`，而此处的 115.0 的值正对应了上面：appBarHeight：115。

![截屏2024-10-20 11.10.24.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/9ca364ebb35c4f07a80a5a93a18594fb~tplv-73owjymdk6-jj-mark-v1:0:0:0:0:5o6Y6YeR5oqA5pyv56S-5Yy6IEAg6bOE6bG85LiN5oCVX-eJmeWMu-S4jeaAlQ==:q75.awebp?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1729480513&x-orig-sign=WrBlkVsdAidfSIZqHA9RaWxhvWE%3D)

![截屏2024-10-20 10.15.10.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/16872966c6114b66b289664ba8178926~tplv-73owjymdk6-jj-mark-v1:0:0:0:0:5o6Y6YeR5oqA5pyv56S-5Yy6IEAg6bOE6bG85LiN5oCVX-eJmeWMu-S4jeaAlQ==:q75.awebp?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1729477016&x-orig-sign=N7WcU8vmUvVZpV3RyWK8yJu87mM%3D)

&emsp;然后继续向下执行 BoxHitTestResult.addWithPaintOffset 函数，可看到 `position - offset` 执行后，便是对：Offset(193.3, 46.7) 进行 hit testing 了。

![截屏2024-10-20 11.19.17.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/ce19b1064f3d4ea2a02701c88b100cbb~tplv-73owjymdk6-jj-mark-v1:0:0:0:0:5o6Y6YeR5oqA5pyv56S-5Yy6IEAg6bOE6bG85LiN5oCVX-eJmeWMu-S4jeaAlQ==:q75.awebp?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1729480865&x-orig-sign=W6ejgUwIm8ROG2C6wl0WMNWlxmA%3D)

&emsp;hit testing 进行到此处可发现，首先由 Y 轴减去了偏移 115.0，即后续的 hit testing 就是从 Y 轴 偏移 115 开始了。

# RenderShiftedBox.hitTestChildren

&emsp;当执行到 RenderShiftedBox.hitTestChildren 时，this 指针指向 depth 是 27 的 RenderPositionBox 时，可看到要计算 X 轴的偏移了。其实这个 depth 是 27 的 RenderPositionBox 就是我们上面看到的 depth 是 26 的 RenderCustomMultiChildLayoutBox 的 4 个子级节点中的第一个子级节点。 

&emsp;由于 RenderPositionedBox 直接继承自 RenderAligningShiftedBox，然后 RenderAligningShiftedBox 又直接继承自 RenderShiftedBox，而只有 RenderShiftedBox 重写实现了 hitTestChildren 函数，所以当 RenderPositionBox 实例对象调用 hitTestChildren 函数时，就调用到了 RenderShiftedBox.hitTestChildren 中。

```dart
class RenderPositionedBox extends RenderAligningShiftedBox { //... }
abstract class RenderAligningShiftedBox extends RenderShiftedBox { //... }
abstract class RenderShiftedBox extends RenderBox with RenderObjectWithChildMixin<RenderBox> { //... }
```

&emsp;可看到 RenderPositionBox 仅有一个子级，而子级 child 的 parentData 是：BoxParentData 类型，值是：Offset(146.5, 0.0)。child 的 size 是：Size(100.0, 737.0)。

&emsp;当 RenderShiftedBox.hitTestChildren 执行时，position 入参是上一步计算出来的：Offset(193.3, 46.7)。然后 RenderShiftedBox.hitTestChildren 函数内部读取到的 child.parentData 的值的 offset 属性是：Offset(146.5, 0.0)，而这个 146.5 怎么计算出来的的呢？其实正是：（393.0 - 100）/ 2.0 得出的，即屏幕的宽度减去我们的黄色区域的宽度 100，然后除以 2 可得。

&emsp;那么 Offset(193.3, 46.7) 减去 Offset(146.5, 0.0) 可得：Offset(46.8, 46.7)，即我们验证的找了一路的 Offset(50.0, 50.0) 终于找到啦！而这个减法操作，同样是在 BoxHitTestResult.addWithPaintOffset 进行的。

![截屏2024-10-20 12.14.38.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/e7ada69329b240b3b3e8bce872df5146~tplv-73owjymdk6-jj-mark-v1:0:0:0:0:5o6Y6YeR5oqA5pyv56S-5Yy6IEAg6bOE6bG85LiN5oCVX-eJmeWMu-S4jeaAlQ==:q75.awebp?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1729484224&x-orig-sign=uom2H7ew%2BB%2FoGX0kgCUM9%2FCpGNY%3D)

![截屏2024-10-20 12.18.16.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/40b76700843d4d20b68db0ab86fa7684~tplv-73owjymdk6-jj-mark-v1:0:0:0:0:5o6Y6YeR5oqA5pyv56S-5Yy6IEAg6bOE6bG85LiN5oCVX-eJmeWMu-S4jeaAlQ==:q75.awebp?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1729484418&x-orig-sign=YGQ9%2B7XPouAUg%2BrPZdLtvk9QYPQ%3D)

```dart
  @override
  bool hitTestChildren(BoxHitTestResult result, { required Offset position }) {
    final RenderBox? child = this.child;
    
    if (child != null) {
    
      // 取得 child 的 parentData 数据。此时可看到 childParentData.offset 的值是：Offset(146.5, 0.0)。
      final BoxParentData childParentData = child.parentData! as BoxParentData;
      
      // 然后 result.addWithPaintOffset 内减去 Offset(146.5, 0.0) 这个偏移，便得到新的 hit teeing 使用的 positon：Offset(46.8, 46.7)。
      return result.addWithPaintOffset(
        offset: childParentData.offset,
        position: position,
        hitTest: (BoxHitTestResult result, Offset transformed) {
          return child.hitTest(result, position: transformed);
        },
      );
      
    }
    return false;
  }
```

&emsp;最终我们不远万里终于找到了距离我们示例代码中黄色区域最中心的（50, 50）的点了，可看到在整个过程中并没有什么难点，就仅仅是读取 RenderBox 在布局阶段计算好的 parentData 数据，然后对其中坐标值进行计算，一层一层逼近点击事件发生时的坐标点。

![截屏2024-10-20 12.21.53.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/26401ddb8cd74fe6853165e8eec4e1d0~tplv-73owjymdk6-jj-mark-v1:0:0:0:0:5o6Y6YeR5oqA5pyv56S-5Yy6IEAg6bOE6bG85LiN5oCVX-eJmeWMu-S4jeaAlQ==:q75.awebp?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1729484637&x-orig-sign=YLtPzB4p%2FzFwAK7t6C6fm5mTx8s%3D)

# RenderProxyBoxWithHitTestBehavior.hitTest

&emsp;轮回。可看到 RenderProxyBoxWithHitTestBehavior.hitTest 被再次执行了，最终执行到了 depth 是 31 的 `_RenderColoredBox` 节点，它的 size 是：Size(100, 100)，而此时调用 hitTest 函数入参 position 是 Offset(46.8, 46.7)，所以此时 hitTest 函数内的 `size.contains(position)` 的值是 true，表示是可以响应点击事件的，而且此时此 RenderBox 节点的 child 属性是 null，表示 hit testing 要到底啦！

&emsp;**最终我们不远万里终于找到了距离我们示例代码中黄色区域最中心的（50, 50）的点了，可看到在整个过程中并没有什么难点，就仅仅是读取 RenderBox 在布局阶段计算好的 parentData 数据，然后对其中坐标值进行偏移计算，一层一层逼近点击事件发生时的坐标点，最终确定一路上的经历了哪些 RenderBox 节点，并把它们以 BoxHitTestEntry 的形式收集起来，然后再统一对它们进行调度，执行它们的 handleEvent 函数！**

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
