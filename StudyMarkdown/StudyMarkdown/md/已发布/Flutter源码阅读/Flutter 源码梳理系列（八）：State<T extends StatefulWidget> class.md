# Flutter 源码梳理系列（八）：`State<T extends StatefulWidget> class`

&emsp;State 用来表示 StatefulWidget 的逻辑和内部状态。学习过程中目光主要聚集在 StatefulElement 类即可。相比 Element 类的学习简单了非常多。

# `_StateLifecycle`

&emsp;当启用断言时，会跟踪 State 对象的生命周期，`_StateLifecycle` 枚举定义了不同的生命周期。

+ created：已创建 State 对象。此时调用 State.initState。
+ initialized：State.initState 方法已被调用，但 State 对象尚未准备好 build。此时会调用 State.didChangeDependencies。
+ ready：State 对象已经准备好 build，而 State.dispose 还没有被调用。
+ defunct：已调用 State.dispose 方法并且 State 对象不再能够 build。

```dart
enum _StateLifecycle {
  created,
  initialized,
  ready,
  defunct,
}
```

# StateSetter

&emsp;State.setState 函数的签名。

```dart
// 签名无参数且不返回数据的回调函数
typedef VoidCallback = void Function();

typedef StateSetter = void Function(VoidCallback fn);
```

# State

&emsp;State 用来表示 StatefulWidget 的逻辑和内部状态。

&emsp;State 是（1）在构建 widget 时可以同步读取的信息，并且（2）在 widget 的生命周期中可能会发生变化。当发生这种状态变化时，Widget 实现者有责任确保 State 及时得到通知，使用 State.setState。

&emsp;当将 StatefulWidget 插入树中时，framework 会通过调用 StatefulWidget.createState 方法来创建 State 对象。由于给定的 StatefulWidget 实例可以被多次插入（例如，StatefulWidget 同时放置在树中的多个位置），可能会有多个与给定 StatefulWidget 实例相关联的 State 对象。同样，如果从树中移除 StatefulWidget，然后再次将其插入树中，framework 将再次调用 StatefulWidget.createState 来创建一个新的 State 对象，简化 State 对象的生命周期。

&emsp;State 对象有如下生命周期：

+ Framework 通过调用 StatefulWidget.createState 来创建一个 State 对象。（初见：StatefulElement 构造函数的初始化话列表调用 widget.createState() 函数创建 State 对象，即 State 对象是和 StatefulElement 一起创建的）

+ 新创建的 State 对象与一个 BuildContext 相关联。这种关联是永久的：State 对象永远不会改变其 BuildContext。然而，BuildContext 本身可以随着其子树在树中移动。在这一点上，State 对象被认为是已挂载的(mounted)。（BuildContext 即是 StatefulElement 对象，State 对象和 StatefulElement 对象是相互引用的。`_state`、`_widget`、`_element`，而且它们都会引用 `_widget`，`_widget` 可能会更新，但是 `_element` 是不会更新的。）

+ Framework 调用 initState。State 的子类应该重写 initState 方法，以执行依赖于 BuildContext 或 widget 的一次性初始化操作。当调用 initState 方法时，BuildContext 和 widget 分别作为 context 和 widget 属性可用。（当 StatefulElement 对象调用 `_firstBuild` 函数时，会直接调用 `state.initState()`，即初次构建时会回调 State 的 initState 函数，全生命周期内仅此一次。）

+ Framework 会调用 didChangeDependencies 方法。State 的子类应该重写 didChangeDependencies 方法来执行涉及 InheritedWidget 的初始化操作。如果调用 BuildContext.dependOnInheritedWidgetOfExactType 方法，如果后续继承的 InheritedWidget 发生变化或 widget 在树中移动，didChangeDependencies 方法将再次被调用。（当 StatefulElement 对象依赖了 InheritedElement 的话，当其更新时，此 StatefulElement 也会重建，并且 state 对象回调：didChangeDependencies 函数。）

+ 此时，State 对象已经完全初始化，Framework 可能会调用其 build 方法任意次数，以获取该子树用户界面的描述（build 函数返回的 Widget 对象，即用于构建 Element 子节点）。State 对象可以通过调用其 setState 方法自发地请求重新构建其子树，这表明它们的一些内部状态已经以可能影响该子树用户界面的方式发生了变化。（setState 对象调用后，即把当前 StatefulElement 标记为脏，在下一帧，此 StatefulElement 对象会进行重建）

+ 在此期间，父级 Widget 可能会重新构建，并请求在树中的该位置更新以显示一个具有相同 runtimeType 和 key 的新 Widget。当这种情况发生时，Framework 将会更新 `_widget` 属性以指向新的 widget 对象，然后调用 didUpdateWidget 方法，并将先前的 widget 作为参数传递(旧的 oldWidget)。State 对象应该重写 didUpdateWidget 来响应其关联 Widget 对象的更改（例如，启动隐式动画）。Framework 始终在调用 didUpdateWidget 后调用 build，这意味着在 didUpdateWidget 中进行的任何 setState 调用都是多余的。（即 StatefulElement 对象使用新 Widget 代替旧 Widget 的更新操作，后面详细看 StatefulElement.uptate 函数内容即可。）

+ 在开发过程中，如果发生热重载（不管是通过命令行的 flutter 工具按下 r 进行的，还是通过 IDE 进行的），则会调用 reassemble 方法。这提供了重新初始化在 initState 方法中准备的任何数据的机会。（即 reassemble 函数，除了回调 State.ressemble，Element.ressemble 也会执行，会标记 Element 对象为脏，并且会调用 Element 对象所有子级的 ressemble 函数。）

+ 如果包含 State 对象的子树从树中移除（例如，因为父级构建了一个具有不同 runtimeType 或 key 的 Widget），则 framework 会调用 deactivate 方法。子类应该重写此方法，以清理该对象与树中其他元素之间的任何链接（例如，如果你向祖先提供了指向子元素 RenderObject 的指针）。（即 StatefulElement 对象从 Element tree 中移除时，也会回调 State.deactivate。）

+ 此时，framework 可能会将此子树重新插入到树的另一个部分。如果发生这种情况，framework 将确保调用 build，以使 State 对象有机会适应其在树中的新位置。如果 framework 重新插入此子树，它将在从树中移除该子树的动画帧结束之前这样做。因此，State 对象可以推迟释放大多数资源，直到框架调用其 dispose 方法。（即 State 对象会跟着 StatefulElement 对象一起被复用，如果真的无法被复用的话，可以在 State.dispose 回调中清理资源，防止内存泄漏。）

+ 如果 framework 在当前动画帧结束之前没有重新插入此子树，框架将调用 dispose，这表示此 State 对象将不会再构建。子类应该重写此方法，以释放此对象保留的任何资源（例如，停止任何活动的动画）。（即在 State.dispose 中清理资源，防止内存泄漏。）

+ 在框架调用 dispose 后，State 对象被视为已卸载，并且 mounted 属性为 false。此时调用 setState 是错误的。此生命周期阶段是终端的：无法重新挂载已被处理的 State 对象。（处于等待被 GC 回收内存♻️。）

&emsp;OK，下面开始看 State 的源码。泛型 T 是 StatefulWidget 的子类。

```dart
@optionalTypeArgs
abstract class State<T extends StatefulWidget> with Diagnosticable {
  // ...
}
```

## widget

&emsp;Widget 不仅作为 Element 的配置，其实也是作为 State 的配置的。State 对象会直接引用创建它的 StatefulWidget 对象。

&emsp;一个 State 对象的配置是对应的 StatefulWidget 对象实例。这个属性在调用 initState 之前由 framework 进行初始化。如果父 widget 更新了树中此位置到一个新的 widget，而这个新的 widget 具有和当前配置相同的 runtimeType 和 Widget.key，framework 会更新这个属性指向新的 widget，然后调用 didUpdateWidget，将旧的 widget 作为参数传递进去。(在创建 State 对象时，已经把 Widget 对象赋值给它了，当 StatefulElement 更新时，会更新此 Widget 对象，并且会回调 State.didUpdateWidget 函数，可以拿到入参 oldWidget 添加一些消失动画。)

```dart
  T get widget => _widget!;
  T? _widget;
```

## `_debugLifecycleState`

&emsp;默认就是 created 已创建的状态。

&emsp;表示这个 State 对象当前所处的生命周期阶段。当启用断言时，该字段会被 framework 使用，以验证 State 对象是否按照一定的顺序进行生命周期变化的。

```dart
  _StateLifecycle _debugLifecycleState = _StateLifecycle.created;
```

## context

&emsp;获取 State 对象的上下文，即 State 对象对应的 StatefulElement 对象。

&emsp;此小部件构建的树中的位置。

&emsp;framework 在使用 StatefulWidget.createState 创建 State 对象后，在调用 initState 之前将其与 BuildContext 关联起来。这种关联是永久的：State 对象不会改变其 BuildContext。但是，BuildContext 本身可以在树中移动。

&emsp;在调用 dispose 后，framework 会切断 State 对象与 BuildContext 的连接。

```dart
  BuildContext get context {
    return _element!;
  }
```

## `_element`

&emsp;在 State 对象创建后直接赋值其 StatefulElement 对象：`state._element = this;`。

```dart
  StatefulElement? _element;
```

## mounted

&emsp;表示当前这个 State 对象是否在树中。

&emsp;在创建 State 对象之后，在调用 initState 之前，framework 会通过将其与 BuildContext 关联来 "mount" State 对象。State 对象保持挂载状态，直到 framework 调用 dispose，之后 framework 将不再要求 State 对象再次构建。

&emsp;在 mounted 为 true 之前调用 setState 是一个错误。

```dart
  bool get mounted => _element != null;
```

## initState

&emsp;当此对象被插入到树中时调用。(即 StatefulElement 对象插入 Element tree 中)

&emsp;framework 会为创建的每个 State 对象仅调用此方法一次。

&emsp;重写此方法以执行依赖于此对象插入树的位置（即 context）或用于配置此对象的小部件（即 widget）的初始化操作。

&emsp;如果 State 的 build 方法依赖于一个可以改变状态的对象，例如 ChangeNotifier 或 Stream，或者一些其他可以订阅以接收通知的对象，那么请确保在 initState、didUpdateWidget 和 dispose 中正确地订阅和取消订阅：

1. 在 initState 中，订阅对象。
2. 在 didUpdateWidget 中，取消订阅旧对象并订阅新对象（如果更新后的 Widget 配置需要替换对象）。
3. 在 dispose 中，取消订阅对象。

&emsp;不应该在此方法中使用 BuildContext.dependOnInheritedWidgetOfExactType。然而，didChangeDependencies 会在此方法之后立即调用，可以在那里使用 BuildContext.dependOnInheritedWidgetOfExactType。

&emsp;此方法的实现应该从调用继承方法开始，如 super.initState()。

```dart
  @protected
  @mustCallSuper
  void initState() {
    // ...
  }
```

## didUpdateWidget

&emsp;当 Widget 的配置发生变化时被调用。

&emsp;如果父级 Widget 重建并请求在树中的此位置更新以显示一个具有相同 runtimeType 和 key 的新 widget，则 framework 将更新此 State 对象的 widget 属性以引用新 widget，然后用先前的 widget 作为参数调用此方法。

&emsp;重写此方法以在 Widget 更改时做出响应（例如，开始隐式动画）。

&emsp;在调用 didUpdateWidget 后，framework 总是会调用 build，这意味着在 didUpdateWidget 中对 setState 的任何调用都是多余的。

&emsp;如果一个 State 的 build 方法依赖于一个可以自身改变状态的对象，例如 ChangeNotifier 或 Stream，或者其他可以订阅以接收通知的对象，则务必在 initState、didUpdateWidget 和 dispose 中正确订阅和取消订阅：

1. 在 initState 中，订阅对象。
2. 在 didUpdateWidget 中取消订阅旧对象并订阅新对象（如果更新后的 Widget 配置需要替换对象）。
3. 在 dispose 中，取消订阅对象。

&emsp;此方法的实现应该以调用继承方法开始，如 super.didUpdateWidget(oldWidget) 中所示。

```dart
  @mustCallSuper
  @protected
  void didUpdateWidget(covariant T oldWidget) { }
```

## reassemble

&emsp;在调试期间重新组装应用程序时调用，例如在热重载期间。

&emsp;这个方法应重新运行任何依赖全局状态的初始化逻辑，例如从资源包中加载图像（因为资源包可能已经发生了变化）。

&emsp;这个函数只会在开发环境中被调用。在发布版本中，ext.flutter.reassemble 钩子不可用，因此这段代码将永远不会执行。

```dart
  @protected
  @mustCallSuper
  void reassemble() { }
```

## setState

&emsp;通知 framework，该 State 对象的内部状态已更改。

&emsp;每当更改 State 对象的内部状态时，请在传递给 setState 的函数中进行更改：

```dart
setState(() { _myState = newValue; });
```

&emsp;提供的回调函数会立即同步调用。它不能返回一个 Future（回调函数不能是异步的），因为这样会不清楚实际上状态何时被设置。

&emsp;调用 setState 通知 framework，此 State 对象的内部状态已经以可能影响此子树中的用户界面的方式发生了改变，这会导致 framework 为该 State 对象安排一个构建。

&emsp;如果你直接更改状态而不调用 setState，则 framework 可能不会安排一个构建，这个子树的用户界面可能不会更新以反映新的状态。

&emsp;通常建议只在实际更改状态时使用 setState 方法来包装，而不是任何可能与更改相关的计算。例如，这里一个被 build 函数使用的值被增加，然后更改被写入磁盘，但只有增量被包含在 setState 中：

```dart
Future<void> _incrementCounter() async {
  setState(() {
    _counter++;
  });
  
  Directory directory = await getApplicationDocumentsDirectory(); // from path_provider package
  final String dirName = directory.path;
  await File('$dirName/counter.txt').writeAsString('$_counter');
}
```

&emsp;有时，已更改的状态位于另一个不由 Widget 状态拥有的对象中，但是 Widget 仍然需要更新以响应新状态。这在使用 Listenables（例如 AnimationControllers）时尤其常见。

&emsp;在这种情况下，在传递给 setState 的回调中留下一个注释，解释哪些状态已更改是一个良好的实践：

```dart
void _update() {
  setState(() { /* The animation changed. */ });
}

//...
animation.addListener(_update);
```

&emsp;在 framework 调用 dispose 后调用此方法是错误的。你可以通过检查 mounted 属性是否为 true 来确定是否可以调用此方法。尽管如此，最好的做法是取消可能触发 setState 的任何工作，而不仅仅是在调用 setState 之前检查是否已经 mounted，否则会浪费 CPU 周期。

### Design discussion

&emsp;这个 API 的最初版本是一个叫做 markNeedsBuild 的方法，以保持与 RenderObject.markNeedsLayout、RenderObject.markNeedsPaint 等方法的一致性。

&emsp;然而，Flutter 框架的早期用户测试表明，人们会比必要的情况更频繁地调用 markNeedsBuild() 方法。实质上，人们使用它就像一个幸运符号，每当他们不确定是否需要调用它时，他们就会调用它，以防万一。

&emsp;自然地，这导致了应用程序的性能问题。

&emsp;当 API 被改为接受一个回调函数时，这种做法大大减少了。一个假设是，通过促使开发人员在回调中实际更新其状态，引导开发人员更仔细地考虑到底是什么被更新，从而提高了他们对于调用方法的适当时机的理解。

&emsp;实际上，setState 方法的实现非常简单：它同步执行所提供的回调，然后调用 Element.markNeedsBuild。

### Performance considerations

&emsp;调用该函数的直接开销很小，并且预计每帧最多调用一次，因此这种开销在任何情况下都是可以忽略的。尽管如此，最好还是要避免在紧密循环中多次冗余调用该函数，因为这涉及创建闭包并执行它。该方法是幂等的，每帧每个 State 多次调用它也没有好处。

&emsp;然而，造成调用该函数的间接成本很高：它会导致 Widget 重建，可能触发整个以该 Widget 为根的子树的重建，并进一步触发整个相应的 RenderObject 子树的布局和重绘。

&emsp;因此，只有在 build 方法将因检测到的任何状态更改而使其结果有意义地改变时，才应调用该方法。

&emsp;OK，setState 函数的注释就这么多，它的实际执行的内容也很好：同步执行传入的闭包，然后把当前 Element 标记为脏，等待下一帧此 Element 对象的重建。它内部的几个断言也告诉了我们调用 setState 函数的注意事项：

+ State 对象已执行 dispose 后不能调用 setState。
+ State 对象刚创建，还未挂载到树上时不能调用 setState。
+ 调用 setState 传入的 VoidCallback 不能是个异步事件。

```dart
  @protected
  void setState(VoidCallback fn) {
    final Object? result = fn() as dynamic;
    _element!.markNeedsBuild();
  }
```

## deactivate

&emsp;当这个对象从树中移除时调用。(由 StatefulElement 的 deactivate 调用。)

&emsp;在将这个 State 对象从树中移除时，framework 会调用这个方法。在某些情况下，framework 会重新将这个 State 对象重新插入到树的另一个部分（例如，如果包含这个 State 对象的子树由于使用 GlobalKey 而从树中的一个位置移动到另一个位置）。如果发生这种情况，framework 将调用 activate 方法，让 State 对象有机会重新获取在 deactivate 中释放的任何资源。然后还会调用 build 方法，让 State 对象有机会适应树中的新位置。如果 framework 确实重新插入这个子树，它会在从树中移除子树的动画帧结束之前这样做。因此，State 对象可以推迟释放大多数资源，直到 framework 调用它们的 dispose 方法。

&emsp;子类应该重写这个方法，清理这个对象与树中其他元素之间的任何链接（例如，如果你向祖先提供了指向后代 RenderObject 的指针）。

&emsp;这个方法的实现应该以调用继承的方法结束，例如 super.deactivate()。

&emsp;当永久从树中移除 Widget 后会调用 dispose。

```dart
  @protected
  @mustCallSuper
  void deactivate() { }
```

## activate

&emsp;在通过 deactivate 方法移除后，当此 State 对象重新插入到树中时会调用此方法。

&emsp;在大多数情况下，当一个 State 对象被停用后，它不会被重新插入到树中，其 dispose 方法将会被调用以表示它可以被 GC 回收♻️。

&emsp;然而，在一些情况下，一个 State 对象在被停用后，framework 会将其重新插入到树的另一个部分（例如，如果包含该 State 对象的子树由于使用 GlobalKey 而从树的一个位置移植到另一个位置）。如果发生这种情况，框架将调用 activate 方法，让 State 对象有机会重新获取在 deactivate 中释放的任何资源。然后还会调用 build 方法，让对象有机会适应树中的新位置。如果框架重新插入这个子树，它将会在子树从树中移除的动画帧结束前这样做。因此，State 对象可以推迟释放大多数资源，直到框架调用它们的 dispose 方法。

&emsp;框架不会在第一次将 State 对象插入到树时调用此方法。相反，在那种情况下，框架会调用 initState。

&emsp;此方法的实现应该以调用继承方法开始，就像 super.activate() 一样。

```dart
  @protected
  @mustCallSuper
  void activate() { }
```

## dispose

&emsp;当此 State 对象永久从树中移除时调用。

&emsp;当 framework 调用此方法时，表示此 State 对象将永远不会再构建(不会再被复用了，可以进入 GC 回收流程了)。在 framework 调用 dispose 后，State 对象被视为已卸载，mounted 属性为 false。在此时调用 setState 是错误的。这个生命周期阶段是终止的：无法重新挂载已被 dispose 的 State 对象。

&emsp;子类应该重写此方法来释放该对象保留的任何资源（例如，停止任何活动的动画）。

&emsp;如果一个 State 的 build 方法依赖于一个可以改变状态的对象，例如一个 ChangeNotifier 或 Stream，或者其他一些可以订阅以接收通知的对象，则确保在 initState、didUpdateWidget 和 dispose 中正确订阅和取消订阅：

1. 在 initState 中，订阅该对象。
2. 在 didUpdateWidget 中，从旧对象取消订阅并订阅新对象（如果更新的 widget 配置需要替换该对象）。
3. 在 dispose 中，从对象取消订阅。

&emsp;此方法的实现应该以调用继承方法结束，如 super.dispose()。

### Caveats

&emsp;这个方法在某些开发者可能期望它被调用的时机并不会被触发，比如应用程序关闭或通过平台 Native 方法关闭时。

&emsp;应用程序关闭：无法预测应用程序关闭将在何时发生。例如，用户的电池可能会着火，或用户可能会将设备掉入游泳池，或者操作系统可能会由于内存压力而单方面终止应用程序进程。

&emsp;应用程序应该负责确保即使在快速的未经安排的终止情况下，它们仍然表现良好。

&emsp;为了人为导致整个 Widget 树被释放，可以考虑使用像 SizedBox.shrink 这样的 Widget 调用 runApp。

&emsp;要监听平台关闭消息（以及其他生命周期更改），可以考虑使用 AppLifecycleListener API。

### Dismissing Flutter UI via platform native methods

&emsp;一个应用程序可能同时包含 Flutter UI 和非 Flutter UI。如果应用程序调用非 Flutter 方法来移除基于 Flutter 的 UI，比如使用平台 Native API 来操作平台 Native 的导航栈，那么 framework 就不知道开发人员是否打算迅速释放资源。Widget 树仍然被加载并准备着，在下次显示时可以立即渲染。

&emsp;请参考用于引导应用程序的方法（例如 runApp 或 runWidget）来了解如何更积极地释放资源的建议。[runApp](https://api.flutter.dev/flutter/widgets/runApp.html) [runWidget](https://api.flutter.dev/flutter/widgets/runWidget.html)

```dart
  @protected
  @mustCallSuper
  void dispose() {
    // ...
  }
```

## build

&emsp;描述了由该 Widget 表示的用户界面的部分。

&emsp;Framework 在许多不同情况下调用此方法。例如：

+ 在调用 initState 之后。
+ 在调用 didUpdateWidget 之后。
+ 在接收到调用 setState 后。
+ 在此 State 对象的某个依赖项更改后（例如，之前构建中引用的 InheritedWidget 更改后）。
+ 在调用 deactivate 并将 State 对象重新插入树的另一个位置后。

&emsp;该方法可能在每一帧中都被调用，不应该产生除了构建 Widget 之外的任何副作用。

&emsp;Framework 使用此方法返回的 Widget 对象替换此 Widget 下方的子树，可能通过更新现有子树或通过移除子树并 inflate 一个新的子树来完成，具体取决于调用 Widget.canUpdate 来确定此方法返回的 Widget 是否可以更新现有子树的根。

&emsp;通常的实现会返回一组新创建的 Widget，这些 Widget 使用了来自此 Widget 构造函数、给定的 BuildContext 和此 State 对象的内部状态的信息。

&emsp;给定的 BuildContext 包含了有关该 Widget 正在构建的树中位置的信息。例如，该上下文为该树中此位置提供了一组 InheritedWidget。BuildContext 参数始终与此 State 对象的 context 属性相同，并且在该对象的生命周期内将保持不变。BuildContext 参数在这里提供冗余，以使此方法与 WidgetBuilder 的签名匹配。

### Design discussion

&emsp;为什么 build 方法放在 State 上，而不是 StatefulWidget 上？

&emsp;将一个 Widget build(BuildContext context) 方法放在 State 上，而不是在 StatefulWidget 上放一个 Widget build(BuildContext context, State state) 方法，可以在子类化 StatefulWidget 时给开发者更多的灵活性。

&emsp;例如，AnimatedWidget 是 StatefulWidget 的一个子类，它引入了一个抽象的 Widget build(BuildContext context) 方法供其子类实现。如果 StatefulWidget 已经有了一个接受 State 参数的 build 方法，那么 AnimatedWidget 就必须强制要求其子类提供 State 对象，即使其 State 对象是 AnimatedWidget 的内部实现细节。

&emsp;在概念上，StatelessWidget 也可以以类似的方式实现为 StatefulWidget 的子类。如果 build 方法在 StatefulWidget 上而不是 State 上的话，那就不可能了。

&emsp;将 build 函数放在 State 上而不是放在 StatefulWidget 上还有助于避免与隐式捕获 this 关联的一类错误。如果在 StatefulWidget 的 build 函数中定义一个闭包，那么该闭包会隐式捕获 this，即当前 Widget 对象实例，并且会使该实例的（不可变的）字段在作用域中：

```dart
// (this is not valid Flutter code)
class MyButton extends StatefulWidgetX {
  MyButton({super.key, required this.color});

  final Color color;

  @override
  Widget build(BuildContext context, State state) {
    return SpecialWidget(
      handler: () { print('color: $color'); },
    );
  }
}
```

&emsp;例如，假设父级使用 blue 构建了一个名为 MyButton 的按钮，这时 print 函数中的 $color 就指代 blue，这是符合预期的。现在，假设父级修改为 green 再次构建 MyButton。第一次构建时创建的闭包仍然隐含地指向原始的 Widget，因此 $color 仍然打印蓝色，即使部件已更新为绿色；如果该闭包生存时间超出其部件的生存周期，它将打印过时的信息。

&emsp;相比之下，在 State 对象上的 build 函数中，闭包在构建过程中隐含地捕获了 State 实例而不是 Widget 实例：

```dart
class MyButton extends StatefulWidget {
  const MyButton({super.key, this.color = Colors.teal});

  final Color color;
  // ...
}

class MyButtonState extends State<MyButton> {
  // ...
  
  @override
  Widget build(BuildContext context) {
    return SpecialWidget(
      handler: () { print('color: ${widget.color}'); },
    );
  }
}
```

&emsp;现在，当父组件使用 green 重建 MyButton 时，第一次构建生成的闭包仍然引用 State 对象，该对象跨重建保留，并且 framework 已更新该 State 对象的 widget 属性，以引用新的 MyButton 实例，而 ${widget.color} 打印为绿色，正如预期的那样。

&emsp;说实话，这里没看懂闭包捕获的问题，有小伙伴能解答一下吗？

```dart
  @protected
  Widget build(BuildContext context);
```

## didChangeDependencies

&emsp;当此 State 对象的依赖项发生更改时调用。

&emsp;例如，如果前一次对 build 的调用引用了后来发生更改的 InheritedWidget，那么 framework 将调用此方法来通知此对象发生更改。

&emsp;此方法也会在 initState 之后立即调用。可以在此方法中安全地调用 BuildContext.dependOnInheritedWidgetOfExactType。

&emsp;子类很少重写此方法，因为 framework 始终在依赖项更改后调用 build。一些子类确实会重写此方法，因为它们在其依赖项更改时需要执行一些昂贵的工作（例如，网络获取），并且这些工作对于每次 build 来说都太昂贵了。

&emsp;再梳理一下当依赖的 InheritedElement 发生更新时是如何回调到 State 的 didChangeDependencies 的：

1. InheritedElement 会直接调用依赖者的 didChangeDependencies 函数。
2. StatefulElement 的 didChangeDependencies 函数内：先直接调用 `super.didChangeDependencies()`（其内部只有一句：把此 element 标记为需要重建：`markNeedsBuild()`），然后把 StatefulElement 对象的 `_didChangeDependencies = true` 标记为 true。
3. 下一帧，当 StatefulElement 对象执行 performRebuild 时，会根据 `_didChangeDependencies` 是否为 true 调用 `state.didChangeDependencies()`。

&emsp;这样就回调到 State.didChangeDependencies 了。

&emsp;如果是其它类型的 Element 依赖 InheritedElement 的话，如：StatelessElement，它呢就没有必要重写 didChangeDependencies 函数，它是直接使用 Element 父类的即可，内部仅一句 `markNeedsBuild()`，把自己标记为脏，然后到下一帧执行重建即可。 

```dart
  @protected
  @mustCallSuper
  void didChangeDependencies() { }
```

## State 总结

&emsp;因为已经读完了 Element 类的内容，所以再读 State 的内容的话还是比较清晰快速的。再加上 State 的内容几乎和 StatefulElement 和 StatefulWidget 绑定在一起，所以看起来比较清晰。

&emsp;一对 State 对象和 StatefulElement 对象是一生都绑定在一起的。不会像它们引用的 widget 对象一样，发生变化。

&emsp;然后再下面最重要两点：1. State 对象的生命周期。2. setState 都干了什么。

&emsp;State 对象的生命周期：

1. StatefulElement 对象创建时，构造函数的初始化列表处：`_state = widget.createState()`，即同时创建了 State 对象。
2. StatefulElement 对象创建好后，就要挂载到 Element tree 上了，会调用它的 mount 函数，然后挂载好后，就要进行初次 build 了，即调用：` _firstBuild()`，然后进入 StatefulElement 的 `_firstBuild` 函数，第一句就是调用：`state.initState()`。1️⃣：initState 执行。
3. 还是 StatefulElement 的 `_firstBuild` 函数，紧接着调用：`state.didChangeDependencies()`。2️⃣：didChangeDependencies 执行。
4. StatefulElement 真正开始构建 `performRebuild` 执行，`state.build()` 执行。3️⃣：build 执行。（后续 StatefulElement 对象如果重建，此 state.build 都会执行。）
5. 当 StatefulElement 对象同类型 Widget 更新时，会执行 update 函数，内部调用：`state.didUpdateWidget(oldWidget)`。4️⃣：didUpdateWidget 执行。
6. 除了初次构建时调用 `state.didChangeDependencies()`，当 StatefulElement 依赖了 InheritedElement 的话，当 InheritedElement 更新时，StatefulElement 也会被通知重建，当 `performRebuild` 执行时，会判断是否是依赖更新（`_didChangeDependencies`），再次调用：`state.didChangeDependencies()`。2️⃣：didChangeDependencies 执行。
7. 当 StatefulElement 对象从 Element tree 移除时，别调用 deactivate，同时调用：`state.deactivate()`。5️⃣：deactivate 执行。
8. 如果 StatefulElement 对象被复用再次被激活时，同时调用：`state.activate()`。6️⃣：activate 执行。
9. 当 StatefulElement 对象再也用不到了，执行 unmount 时，同时调用：`state.dispose()`。7️⃣：dispose 执行。

&emsp;然后是 setState 都干了什么？内部的话仅是两行代码：`final Object? result = fn() as dynamic;` 和 `_element!.markNeedsBuild()`，首先执行传入的回调，更新 State 对象的数据，然后把对应的 StatefulElement 对象标记为脏，并把它添加到全局的脏列表里：`_dirtyElements.add(element)`，等待着下一帧进行重建。 

&emsp;OK，`State<T extends StatefulWidget> class` 类内容看完了，如果已经对 Element 构建和更新流程很熟悉的话，那么 State 的内容看起来也是清晰异常。 

## 参考链接
**参考链接:🔗**
+ [`State<T extends StatefulWidget> class`](https://api.flutter.dev/flutter/widgets/Element-class.html)
+ [StatefulElement class](https://api.flutter.dev/flutter/widgets/StatefulElement-class.html)
+ [StatelessElement class](https://api.flutter.dev/flutter/widgets/StatelessElement-class.html)
