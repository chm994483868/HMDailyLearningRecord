# Flutter 源码梳理系列（四十）：HitTesting：At first sight

# 前言

&emsp;在很久之前我们学习 RenderObject 时，已经看到过 RenderObject 需要实现 HitTestTarget 抽象接口类，而 HitTestTarget 抽象接口类仅有一个抽象函数：handleEvent，即每个实现 HitTestTarget 的类都需要实现自己的 handleEvent 抽象函数，而当事件到来时便会调用此 handleEvent 函数来处理这个事件。

&emsp;那这个 "处理事件" 是指什么事件呢？其实就是 handleEvent 抽象函数的 event 参数所限定的 PointerEvent 事件，而在 Flutter framework 中 PointerEvent 是作为 **触摸、触控笔或鼠标事件的基类** 而存在的，对应到移动端平台便是手指的触摸事件了，即 "处理事件" 便是处理我们的手指在触摸屏上的触摸事件，点击、滑动等等操作。

&emsp;所以看到这里我们便可以直白的理解：实现了 HitTestTarget 抽象接口类的类便具备了处理触摸事件的能力，而我们学了很久的 RenderObject 类便是一个这样的存在，它可以处理事件（或者说是可以响应事件）。然后在 Flutter framework 中进行全局搜索可发现仅有：RenderObject 和 TextSpan 实现了 HitTestTarget 抽象接口类。

&emsp;大前端中最重要的两件事：界面渲染和事件响应，关于界面渲染，我们前面的文章几乎都在讲这件事，然后后续再把布局模型相关的内容看完，几乎就把与界面渲染相关的内容都看一遍了，继续加油！而本篇呢我们则聚焦：事件响应，把 hit test 和 handle event 看个通透。

```dart
/// An object that can handle events.（HitTestTarget 表示了一个可以处理事件的对象。）
abstract interface class HitTestTarget {
  /// Override this method to receive events.（重写此方法以接收事件。）
  void handleEvent(PointerEvent event, HitTestEntry<HitTestTarget> entry);
}
```

&emsp;那么当一个触摸事件发生时，Flutter framework 是如何找到可以处理此事件的 RenderObject 的呢？而这就要引出 RenderBox 中的 hit testing（命中测试）了。在 RenderBox 中进行 hit test 正是通过下面一组函数进行的：

+ `bool hitTest(BoxHitTestResult result, { required Offset position }) { //... }`

```dart
  bool hitTest(BoxHitTestResult result, { required Offset position }) {
    // 如果 hit 的 position 位于当前 RenderBox 对象的 _size 内，则继续进行判断，否则直接返回 false。
    if (_size!.contains(position)) {
    
      // 由于 RenderBox 的 hitTestChildren 和 hitTestSelf 默认返回 false，所以不会进入此 if
      if (hitTestChildren(result, position: position) || hitTestSelf(position)) {
        // 当然 RenderBox 的子类可以根据自己的情况，而重写 hitTestChildren 和 hitTestSelf 函数。
        result.add(BoxHitTestEntry(this, position));
        
        // 即如果自己的任意一个子级 RenderBox 可以命中的话，则当前 RenderBox 也是可以命中！
        return true;
      }
    }
    
    return false;
  }
```

+ `bool hitTestSelf(Offset position) => false;`
+ `bool hitTestChildren(BoxHitTestResult result, { required Offset position }) => false;`

&emsp;首先 RenderBox 的 hitTestSelf 和 hitTestChildren 默认都是返回 false，则表示了会继续向当前 RenderBox 后方的兄弟级 RenderBox 中进行 hit test。

&emsp;RenderBox 的 hit testing 是通过 hitTest 方法进行的，而这个方法的默认实现委托给了 hitTestSelf 和 hitTestChildren，而 RenderBox 的这两个函数默认都是返回 false，所以在 RenderBoxd 的子类实现 hit testing 时，可以选择重写这两个方法，或者忽略它们，直接重写 hitTest 方法。

&emsp;hitTest 方法本身接收一个 Offset position 参数，如果当前 RenderBox 或其子级 RenderBox 中的一个可以命中（阻止了当前 RenderBox 后方的兄弟级 RenderBox 对象被点击），则必须返回 true；如果点击可以继续传递到这个 RenderBox 后方的其他 RenderBox 对象，则必须返回 false。

&emsp;对于每个 RenderBox 对象，应该使用相同的 HitTestResult 参数调用其子级 RenderBox 上的 hitTest 方法，并将 Offset position 转换为其子级 RenderBox 的坐标空间。默认实现委托给 hitTestChildren 来调用子级。

&emsp;RenderBoxContainerDefaultsMixin 提供了 RenderBoxContainerDefaultsMixin.defaultHitTestChildren 方法，假设子级 RenderBox 是轴对齐的、未被转换（transformed）并且根据 parentData 的 BoxParentData.offset 属性进行定位；更复杂的 RenderBox 子类可以相应地重写自己的 hitTestChildren 方法。

&emsp;如果 RenderBox 对象被点击（即自己的 hitTest 函数返回 true 的话），则还应将自身添加到 hitTest 方法的 BoxHitTestResult result 参数中，使用 HitTestResult 的 add 方法进行添加。

&emsp;默认实现委托给 hitTestSelf 来确定 RenderBox 是否被点击。如果 RenderBox 对象在子级 RenderBox 对象之前添加自身，则会导致它被视为在子级 RenderBox 上方。如果它在子级 RenderBox 之后添加自身，则会被视为在子级 RenderBox 下方。

&emsp;添加到 HitTestResult 对象中的 Entry 应该使用 BoxHitTestEntry 类，这些 Entry 随后会按添加顺序被系统遍历，对于每个 Entry，将调用它的 target 的 handleEvent 方法，并在 handleEvent 被调用时传递 HitTestEntry 对象。

&emsp;下面在深入学习 hit testing 相关的类之前，我们首先捋捋 hit test 相关的函数调用堆栈，对此过程在实际的代码的执行上有一个脸熟。

&emsp;根据以上内容，我们可以把事件响应分为两个阶段：

1. 遍历 Render Tree 找到可以响应 hit 的 RenderBox，并以它为参数构建一个 BoxHitTestEntry，并收集到 HitTestResult 的 path 属性中。（可谓之 hit test 阶段）
2. 从前往后遍历被收集到的 BoxHitTestEntry，执行它们的 target（即 RenderBox 对象）的 handleEvent 函数。（可谓之 handle event 阶段）

# hit test

&emsp;下面我们以 Listener Widget 为例子，来研究一下上面两个阶段所涉及的函数调用堆栈。我们使用如下超简单的示例代码，当点击 'click me' 时，会在控制台打印：'onPointerDown 执行'。

```dart
import 'package:flutter/material.dart';

class MyHitTestAppWidget extends StatelessWidget {
  const MyHitTestAppWidget({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Hit Test Demo'),
        ),
        body: Center(
          child: Column(
            children: [
              Listener(
                child: const Text('click me'),
                onPointerDown: (p) {
                  debugPrint('onPointerDown 执行');
                },
              ),
            ],
          ),
        ),
      ),
    );
  }
}
```

&emsp;我们以 debug 模式调试此示例代码，并在 RenderProxyBoxWithHitTestBehavior 的 hitTest 函数处打一个断点，这里之所以在 RenderProxyBoxWithHitTestBehavior 处，是因为 Listener 的 createRenderObject 函数返回的 RenderPointerListener 正是继承自 RenderProxyBoxWithHitTestBehavior。然后我们点击 click me，断点会被正常命中，然后可以看到如下函数堆栈：

![截屏2024-08-26 00.49.10.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/b5dc8762f61e481ab1ad1f6313f8a127~tplv-73owjymdk6-jj-mark-v1:0:0:0:0:5o6Y6YeR5oqA5pyv56S-5Yy6IEAg6bOE6bG85LiN5oCVX-eJmeWMu-S4jeaAlQ==:q75.awebp?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1724691091&x-orig-sign=xvAanVlvFGSUvsv1qbvwu6UTj9Q%3D)

&emsp;可看到自 hooks.dart 中的：`_dispatchPointerDataPacket` 为起点，然后进入 GestureBinding 中，GestureBinding 实现了一组跟 hit test 有关的抽象接口类：HitTestable、HitTestDispatcher、HitTestTarget，除去我们目前在看的最简单的单点点击事件之外，还有复杂手势识别都会由 GestureBinding 统一分发管理，后续我们再对其进行深入研究。

```dart
mixin GestureBinding on BindingBase implements HitTestable, HitTestDispatcher, HitTestTarget { //... }
```

&emsp;然后是在 GestureBinding 的 `_handlePointerEventImmediately` 函数内，会创建一个 HitTestResult 对象，然后是直接调用 `hitTestInView` 函数，注意看这里是直接调用到了 RendererBinding 的 `hitTestInView` 函数中，之所以这里会调用到 RendererBinding 中是因为当前的 this 指针其实是 WidgetsFlutterBinding 对象，它同时混入了 GestureBinding 和 RendererBinding，且 RendererBinding 在 GestureBinding 的后面，即可以把 RendererBinding 看作是 GestureBinding 的子类，也正如 RendererBinding Mixin 定义所示，它可以看作是 GestureBinding 的子类，并且重写了 GestureBinding 的 hitTestInView 函数。

```dart
class WidgetsFlutterBinding extends BindingBase with GestureBinding, SchedulerBinding, ServicesBinding, PaintingBinding, SemanticsBinding, RendererBinding, WidgetsBinding { //... }

mixin RendererBinding on BindingBase, ServicesBinding, SchedulerBinding, GestureBinding, SemanticsBinding, HitTestable { //... }
```

&emsp;RendererBinding 的 hitTestInView 函数内部也超简单，直接调用 `_viewIdToRenderView[viewId]` 的 hitTest 函数，那么这里的 `_viewIdToRenderView[viewId]` 是谁呢？其实正是我们的 Render Tree 的根节点，它的类型便是 RenderView，而 RenderView 的 hitTest 函数实现则是直接调用自己的 child 的 hitTest 函数：

```dart
  @override
  void hitTestInView(HitTestResult result, Offset position, int viewId) {
    _viewIdToRenderView[viewId]?.hitTest(result, position: position);
    super.hitTestInView(result, position, viewId);
  }
```

&emsp;RenderView 的 hitTest 函数实现如下：

```dart
  bool hitTest(HitTestResult result, { required Offset position }) {
    // 如果自己的 child 不为空，则调用 child 的 hitTest 函数
    if (child != null) {
      child!.hitTest(BoxHitTestResult.wrap(result), position: position);
    }
    
    // ⬆️ 上面的 child!.hitTest 执行会一路在子级 RenderObject 中递归执行，
    // 当上面我们的断点被命中时，函数堆栈正是还在这个 if 函数内部。
    
    // 以当前 Render Tree 根节点为参数构建一个 HitTestEntry 对象添加到 result 中
    result.add(HitTestEntry(this));
    
    // 然后默认返回 true
    return true;
  }
```

&emsp;然后自 RenderView 的 hitTest 函数开始，后续便是 RenderBox.hitTest 和 RenderProxyBoxMixin.hitTestChildren 一直在重复递归执行，可以看作是沿着 RenderBox 链一直进行 hit testing 直到我们的 RenderProxyBoxWithHitTestBehavior 的 hitTest，可以看到执行到此处时 this 指针是一个 RenderPointerListener 对象，它的 depth 是 10，正是我们示例代码中 Listener Widget 的 createRenderObject 函数返回的 RenderObject 对象。

&emsp;然后我们把 RenderProxyBoxWithHitTestBehavior 的 hitTest 函数处的断点取消，然后在 RenderView 的 hitTest 函数的 `return true;` 行打一个断点，然后点击 'Resume Program' 按钮，直接让程序执行到这个 `return true;` 断点处，然后着重看 result 变量：

![截屏2024-08-26 01.41.17.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/7380fd72d23f4d3798f7ee0c889ef3e2~tplv-73owjymdk6-jj-mark-v1:0:0:0:0:5o6Y6YeR5oqA5pyv56S-5Yy6IEAg6bOE6bG85LiN5oCVX-eJmeWMu-S4jeaAlQ==:q75.awebp?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1724694194&x-orig-sign=I%2F%2FETzZ1NiUJmIJdjEyF1%2FMM3cw%3D)

![截屏2024-08-26 01.41.33.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/75483bc98efa439ebea70512c90db3a9~tplv-73owjymdk6-jj-mark-v1:0:0:0:0:5o6Y6YeR5oqA5pyv56S-5Yy6IEAg6bOE6bG85LiN5oCVX-eJmeWMu-S4jeaAlQ==:q75.awebp?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1724694212&x-orig-sign=RD1oCegWOdia%2B3H640MTkhvxOtw%3D)

&emsp;可看到在这个 hit test 过程中，共收集了 30 个 Entry（即 30 个 RenderObject），其中第一个是我们的 'click me' 对应的 TextSpan，而最后一个则是我们的 Render Tree 的根节点，即可以把 hit test 理解为是深度优先遍历的，第一个加入 HitTestResult 的是 depth 最深的。

&emsp;然后我们继续回到 GestureBinding 的 `_handlePointerEventImmediately` 函数，可以看到函数的末尾会调用：`dispatchEvent(event, hitTestResult);`，而这便是上面分析的第二阶段：handle event 阶段的开始。

## 参考链接
**参考链接:🔗**
+ [HitTestTarget class](https://api.flutter.dev/flutter/gestures/HitTestTarget-class.html)
+ [HitTestResult class](https://api.flutter.dev/flutter/gestures/HitTestResult-class.html)
+ [BoxHitTestResult class](https://api.flutter.dev/flutter/rendering/BoxHitTestResult-class.html)
+ [`HitTestEntry<T extends HitTestTarget> class`](https://api.flutter.dev/flutter/gestures/HitTestEntry-class.html)
+ [BoxHitTestEntry class](https://api.flutter.dev/flutter/rendering/BoxHitTestEntry-class.html)
+ [HitTestResult class](https://api.flutter.dev/flutter/gestures/HitTestResult-class.html)
