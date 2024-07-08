# Flutter 源码梳理系列（十七）：ComponentElement、Stateless & StatefulElement

# ComonentElement 

&emsp;ComponentElement：一个组合其他 Element 的 Element。与直接创建 RenderObject 不同(与 RenderObjectElement 相对比)，ComponentElement 通过创建其他 Element 间接地创建 RenderObjects。

+ Object -> DiagnosticableTree -> Element -> ComponentElement

&emsp;ComponentElement 的直接子类：ProxyElement、StatelessElement、StatefulElement。

## Constructors

&emsp;ComponentElement(super.widget)：创建一个使用给定 widget 作为其配置的 ComponentElement 对象。

```dart
abstract class ComponentElement extends Element {
  ComponentElement(super.widget);
  
  // ...
}
```

## `_child`

&emsp;此 ComponentElement 元素的子元素（子节点、子级）。

```dart
  Element? _child;
```

## renderObjectAttachingChild

&emsp;返回此 ComponentElement 的子元素（`_child`），该子元素将向此 Element 的祖先插入一个 RenderObject 以构建渲染树。

&emsp;如果此 Element 没有任何需要将 RenderObject 附加到该 Element 的祖先中的子元素，则返回 null。因此，RenderObjectElement 将返回 null，因为其子元素将其 RenderObject 插入到 RenderObjectElement 本身而非 RenderObjectElement 的祖先中。

&emsp;此外，对于在其内部提升自己独立渲染树且不扩展祖先渲染树的 Element，可能会返回 null。

```dart
  @override
  Element? get renderObjectAttachingChild => _child;
```

## mount

&emsp;我们再熟悉的 mount 函数，ComponentElement 进行了重写，除了直接调用 super.mount(parent, newSlot)，还会执行：`_firstBuild`，即当 ComponentElement 对象挂载到 Element Tree 上后就要进行第一次构建了。因为它还要对自己的 build 函数返回的子级 Widget 进行构建 Element。

```dart
  @override
  void mount(Element? parent, Object? newSlot) {
    super.mount(parent, newSlot);
    
    _firstBuild();
  }
```

## `_firstBuild`

&emsp;StatefulElement 重写了这个方法，以便在初次构建时调用 state.didChangeDependencies 方法。内部的 rebuild 函数最终调用 performRebuild 方法。

```dart
  void _firstBuild() {
    // StatefulElement overrides this to also call state.didChangeDependencies.
    rebuild(); // This eventually calls performRebuild.
  }
```

## performRebuild

&emsp;调用 StatelessWidget 对象的 StatelessWidget.build 方法（对于 stateless widgets），或者调用 State 对象的 State.build 方法（对于 stateful widgets），然后更新 Widget Tree。

&emsp;在挂载(mount)过程中自动调用，以生成第一次构建（firstBuild），并在元素需要更新时通过重建调用（rebuild）。

```dart  
  @override
  @pragma('vm:notify-debugger-on-exception')
  void performRebuild() {
    Widget? built;
    
    try {
      built = build();
    } catch (e, stack) {
      // ...
    } finally {
      // 为了在调用 build() 后再标记元素为干净状态，我们延迟标记操作，
      // 这样在 build() 过程中调用 markNeedsBuild() 将会被忽略。
      
      // 通过 super.performRebuild() 把 _dirty 标记置为 false
      super.performRebuild(); // clears the "dirty" flag
    }
    try {
      _child = updateChild(_child, built, slot);
    } catch (e, stack) {
      // ...
    }
  }
```

## build

&emsp;子类应该重写这个函数，以实际调用适当的构建函数（例如 StatelessWidget.build 或 State.build）来构建它们的 Widget。

&emsp;注意 build 是 ComponentElement 的一个抽象函数，所有的 ComponentElement 子类要自己实现这个 build 函数。

```dart
  @protected
  Widget build();
```

## visitChildren

&emsp;对每个子组件调用传入的参数 visitor 函数。必须由支持拥有子组件的子类进行重写。在 Element 类中 visitChildren 函数是空的。

&emsp;访问子组件的顺序无法保证，但是随着时间推移应该是一致的。

&emsp;在构建期间调用此方法是危险的: 子组件列表可能仍在更新，因此在那时子组件可能尚未构建，或者可能是即将被替换的旧子组件。只有当可以证明子组件已经可用时，才应调用此方法。

```dart
  @override
  void visitChildren(ElementVisitor visitor) {
    if (_child != null) {
      visitor(_child!);
    }
  }
```

## forgetChild

&emsp;从元素的子元素列表中删除给定的子元素(在 ComponentElement 这里则是把自己的 `_child` 属性置为 null)，以准备将该子元素在元素树的其他地方重用。

&emsp;这将更新子元素模型，例如，visitChildren 不再遍历该子元素。

&emsp;在调用此方法时，元素仍将有一个有效的父元素，并且在该父元素的上下文中，子元素的 Element.slot 值也将是有效的。调用此方法后，将调用 deactivateChild 方法来切断与此对象的联系。

&emsp;这个 update 任务是负责更新或创建将替换此子元素的新子元素的。

```dart
  @override
  void forgetChild(Element child) {
    _child = null;
    
    super.forgetChild(child);
  }
```

## ComponentElement 总结

&emsp;OK，ComponentElement 的内容就这么多，它作为一个直接继承自 Element 的抽象类，添加了一个超级重要的抽象方法，也是我们重写过无数次的方法：`Widget build()`，build 可以说是贯穿 Flutter framework 的始终，其实它是 ComponentElement 中定义的，然后还有一个超级重要的属性：`Element? _child`，在 SingleChildRenderObjectElement 中也有一个和此完全一样的属性。正是此 `_child` 和 Element 的 `Element? _parent` 属性，它们两个指针一起构建了 Element Tree：以双向链表结构为基础的树状结构🌲。 

&emsp;然后还有 ComponentElement 对 Element 的 mount、performRebuild 的重载，为接下来的一层一层的构建 Element Tree 奠定了基础。如 Element.mount 完成把当前这个单独的 Element 节点挂载到 Element Tree 上的任务，然后 ComponentElement 的 mount 函数则是在此 Element 节点挂载到树上后，继续进行此 Element 节点的 build，继续构建此 Element 节点的子级。ComponentElement.performRebuild 则实际完成这个任务，把此 Element 节点的子级赋值给它的 `_child` 属性，就这样递归的一级一级的完成以此 Element 节点为根节点的整个 Element 子树的构建。

&emsp;ProxyElement、StatelessElement、StatefulElement 它们作为 ComponentElement 的直接子类，继承了 ComponentElement 的 `_child` 和 `Widget build()` 函数，提供更加具体的实现内容。OK，我们继续往下看！

# StatelessElement

&emsp;StatelessElement：使用 StatelessWidget 作为配置的 Element。

+ Object -> DiagnosticableTree -> Element -> ComponentElement -> StatelessElement

&emsp;StatelessElement 直接继承自 ComponentElement。

## Constructors

&emsp;创建一个 StatelessElement，使用给定的 StatelessWidget 作为其配置。

```dart
class StatelessElement extends ComponentElement {
  StatelessElement(StatelessWidget super.widget);
  
  // ...
}
```

## build

&emsp;直接调用它的 StatelessWidget 的 build 函数，并返回其构建的 Widget。转了一圈终于回到了最初，回到了 StatelessWidget 的 build 函数：`Widget build(BuildContext context)`。

```dart
  @override
  Widget build() => (widget as StatelessWidget).build(this);
```

## update

&emsp;更改用于配置此 StatelessElement 的 StatelessWidget。

&emsp;当父级希望使用不同的 Widget 来配置此 Element 时，框架会调用此函数。新的 Widget 保证具有与旧 Widget 相同的 runtimeType。(执行此 update 函数前已经判断过了，如果不同就执行不到这里了。)

&emsp;此函数仅在 "active" 生命周期状态期间调用。

&emsp;在 Element.update 函数中仅有一行，把 `_widget` 属性赋值为入参 newWidget。在 StatelessElement 里还需要强制执行 StatelessElement 的重建，这毕竟 Widget 都已经发生变化了，肯定要进行强制重建。

```dart
  @override
  void update(StatelessWidget newWidget) {
    super.update(newWidget);
    
    // 对此 StatelessElement 对象进行强制重建。 
    rebuild(force: true);
  }
```

## StatelessElement 总结

&emsp;OK，StatelessElement 的内容就这么多，就这两个函数。主要实现了 ComponentElement 的 build 函数，即直接调用自己的 `widget`(StatelessWidget) 的 build 函数，返回创建的 Widget 对象即可。然后是重写了 Element.update 函数：当 widget 属性更新完毕以后肯定要对此 StatelessElement 节点进行强制重建工作。

# StatefulElement 

&emsp;StatefulElement：使用 StatefulWidget 作为配置的 Element。

+ Object -> DiagnosticableTree -> Element -> ComponentElement -> StatefulElement

&emsp;StatefulElement 直接继承自 ComponentElement。

## Constructors

&emsp;创建一个 StatefulElement，使用给定的 StatefulWidget 作为其配置。

&emsp;当创建一个 StatefulElement 对象时，同时直接调用 StatefulWidget 的 createState 函数为 StatefulElement 创建 State 对象，并直接赋值给 `_state` 属性，同时 state 对象的 `_element` 属性直接用 this(当前的 StatefulElement 对象赋值)，`_widget` 属性同时也用 StatefulWidget 赋值。至此：StatefulElement、State、StatefulWidget 三者的关系确定下来了：StatefulElement 和 State 之间相互引用，而它二者都引用同一个 StatefulWidget。

&emsp;另外一点便是当 StatefulWidget 对象调用 createElement 创建 StatefulElement 对象时，同时对应的 State 对象也会被一起创建出来，StatefulElement 对象和 State 对象是一一对应的，可谓是一起出生一起消亡。另外，当 StatefulElement 对象更新自己的 StatefulWidget 时，State 的 widget 属性也会一起更新，并且它俩保持同步始终指向同一个 StatefulWidget 对象。另外就是 State 对象和 StatefulElement 对象它二者的生命周期也是完全一样的。

```dart
class StatefulElement extends ComponentElement {
  StatefulElement(StatefulWidget widget)
      : _state = widget.createState(), super(widget) {
    state._element = this;
    state._widget = widget;
  }
  
  // ...
}
```

## build

&emsp;直接调用它的 state 属性的 build 函数，并返回其构建的 Widget。转了一圈终于回到了最初，回到了 `State<T extends StatefulWidget>`  的 build 函数：`Widget build(BuildContext context)`。

```dart
  @override
  Widget build() => state.build(this);
```

## state

&emsp;与 Element Tree 中此位置关联的 State 实例。The State instance associated with this location in the tree.(这是官方的解释，)

&emsp;State 对象与持有它们的 StatefulElement 对象之间是一对一的关系。State 对象由 StatefulElement 在挂载时创建。

```dart
  State<StatefulWidget> get state => _state!;
  State<StatefulWidget>? _state;
```

&emsp;OK，由此后续的 StatefulElement 重写的 ComponentElement/Element 的函数基本都是为了在其中插入 State 相关的操作，特别是和 StatelessElement 比较的话，StatelessElement 基本上都是原封不动的直接使用 ComponentElement/Element 中的函数。

&emsp;从下面开始，在 StatefulElement 中会重写 Element 的函数，只要在其中加入了 State 相关的操作的位置我们就添加一个序号，看到最后时会有多少个序号。

&emsp;看到这里有点想知道 Dart 中某个类的实例对象当调用类定义中的函数时是怎么执行的？如果是继承的父类的函数分为重写和非重写时又是怎么调用的？还有类中定义的静态函数是怎么调用的？直接用类名调用的时候是如何找到这个函数的？每个类都有一个全局唯一的类实例吗？还有某个类的实例对象中保存属性的值吗？是怎么保存的？如果是多个的时候是怎么排列的？

## reassemble

&emsp;在调试过程中，例如在热重载期间，每当应用程序被重新组装时都会调用此方法。

&emsp;该方法应重新运行任何依赖于全局状态的初始化逻辑，例如，从资源包加载图像（因为资源包可能已更改）。

&emsp;此函数仅在开发过程中调用。在发布构建中，ext.flutter.reassemble hook 不可用，因此此代码将永远不会执行。

&emsp;实现者不应依赖于在 hot reload source update、reassemble 和 build 之间的任何排序。在启动热重载之后，计时器（如动画）或附加到 isolate 中的调试会话可能会在调用 reassemble 之前触发重新装入的代码。期望 reassemble 之后的先决条件由 build 方法设置的代码必须能够顺利运行，例如，通过放弃而不是抛出异常。 也就是说，一旦 reassemble 被调用，build 将至少在其之后被调用一次。

&emsp;StatefulElement 重写 Element 的 reassemble 方法，添加了 state.reallemble 调用。

```dart
  @override
  void reassemble() {
    // 1️⃣ 在其中添加 state.reassemble
    state.reassemble();
    
    super.reassemble();
  }
```

## `_firstBuild`

&emsp;

```dart
  @override
  void _firstBuild() {
    // 2️⃣ 在其中加入 state.initState 的回调，这个回调对 State 对象而言超级重要！仅调用一次，我们可以进行一些 State 变量相关的初始化。
    // 这里有一个 debugCheckForReturnedFuture 的返回值，它在开发模式下没什么用，
    // 主要是提示我们 state.initState 只能是同步函数，不能添加 async，
    // 不要直接在 initState 中等待异步工作，而是调用一个单独的方法来完成这项工作，而不是等待它。
    final Object? debugCheckForReturnedFuture = state.initState() as dynamic;
    
    // 3️⃣ 然后是直接回调 state.didChangeDependencies 回调，
    // 在这里我们可以使用 InheritedWidget 中取得的数据进行重度的计算工作，如果有的话。
    
    // 注意当此 StatefulElement 对象依赖的 InheritedWidget 的数据发生变化时，state.didChangeDependencies 函数就会被重复调用，
    // 但是一般情况下我们可以在 build 中读取 StatefulElement 依赖的 InheritedWidget 中的数据，因为读取的时间复杂度特别低，特别快速，
    // 所以可以直接在 state.build 中调用 of 做这个事情，尽管 build 可能因为别的原因，调用的非常频繁，
    // 但是如果我们读取依赖的数据后要做大的计算量的事情的话，
    // 就可以把这个 of 读取动作移动到 state.didChangeDependencies 中去，
    // 这样就能保证大的计算量的事情只在依赖的数据发生变化时才会执行，（因为 state.didChangeDependencies 只在依赖数据发生变化时才会调用）
    // 当然如果没什么计算量的话，全部放在 build 中也是可以的。视情况而定。
    state.didChangeDependencies();
    
    // 然后则是直接调用 ComponentElement 的 firstBuild 函数。
    super._firstBuild();
  }
```

## performRebuild

&emsp;调用 StatelessWidget 对象的 StatelessWidget.build 方法（对于 stateless widget）或 State 对象的 State.build 方法（对于 stateful widget），然后更新 Widtet Tree。

&emsp;在挂载期间自动调用以生成第一个构建，并在元素需要更新时由 rebuild 调用。

&emsp;StatefulElement 重写此函数，仅为了 State。

```dart
  @override
  void performRebuild() {
    // 4️⃣ 插入 state.didChangeDependencies 函数调用，
    // 根据实际情况，只有依赖发生变化时才会被调用，
    // _didChangeDependencies 标识会记录依赖发生了变化。
    // 此标识由下面的 didChangeDependencies 函数赋值为 True，
    // 当建立了依赖关系的话，InheritedElement 会直接这个 StatefulElement 对象收集起来，
    // 然后当依赖数据发生变化时 InheritedElement 对象就可以直接调用这个 StatefulElement 对象的 didChangeDependencies 函数，
    // 做了两件事：
    // 1. 直接调用这个 StatefulElement 对象的 markNeedsBuild 函数，标记重建！
    // 2. 把 _didChangeDependencies 置为 true，然后 StatefulElement 进行重建时，就会根据这个标识调用 state.didChangeDependencies 了。
    if (_didChangeDependencies) {
      state.didChangeDependencies();
      
      // 本次调用完，直接置为 false，然后等待下一轮
      _didChangeDependencies = false;
    }
    
    // 然后继续调用 ComponentElement 的 performRebuild 函数。
    super.performRebuild();
  }
```

## update

&emsp;除了同 StatelessElement 一样的强制重建外，依然是为了添加 State 相关的操作：

+ 与 StatefulElement 对象保持同步的更新 Widget 属性。
+ 更新了 Widget 对象后，还需要带着之前的旧 Widget 对象调用：State.didUpdateWidget 函数。

```dart
  @override
  void update(StatefulWidget newWidget) {
    // 这里直接调用 Element 的 update 函数，并不是调用 ComponentElement 的，
    // 因为 ComponentElement 根本没有重写此 update 函数。
    // 而 Element.update 仅作了一件事：把 newWidget 赋值给 _widget 属性。
    super.update(newWidget);
    
    // oldWidget 临时变量记录下当前 state 的 widget 属性的值，
    // 用于下面的 state.didUpdateWidget 回调的参数 
    final StatefulWidget oldWidget = state._widget!;
    
    // 5️⃣ 更新 state 的 widget 属性的值，保证 StatefulElement 和 State 的 widget 属性值一致
    state._widget = widget as StatefulWidget;
    
    // 不要直接在 didUpdateWidget 内部等待异步工作，而是调用一个单独的方法来完成这项工作，而不需要等待它。
    // 同样 debugCheckForReturnedFuture 返回值判断不要在让 state.didUpdateWidget 变成一个异步函数。
    // 6️⃣ 带着之前的旧 Widget 对象，回调 State.didUpdateWidget 函数。例如可以给这个旧 Widget 对象添加一个渐变的隐去动画。
    final Object? debugCheckForReturnedFuture = state.didUpdateWidget(oldWidget) as dynamic;
    
    // 强制进行此 StatefulElement 的重建工作
    rebuild(force: true);
  }
```

## activate

&emsp;除了在 activate 中添加 state.activate 之外，还有一个重要的操作：标记 StatefulElement 重建！

&emsp;注释给出了的解释是：由于 deactivate 函数中可能执行了一些资源的释放操作，所以这里当 StatefulElement 被重新激活后，需要进行 StatefulElement 的重建工作。

```dart
  @override
  void activate() {
    // ComponentElement 没有重写此 activate 函数，
    // 所以这里是直接调用 Element 的 activate 函数。
    super.activate();
    
    // 7️⃣ 添加 state.activate 函数。当 StatefulElement 对象执行 activate 函数时，
    // State 对象也需要执行它的 state.activate 函数，又一个 State 的回调函数被执行。
    state.activate();
    
    // 由于 State 可能已经观察到 deactivate() 并因此释放了在 build 中分配的资源，我们必须重新构建 Widget，以便其 State 可以重新分配其资源。
    markNeedsBuild();
  }
```

## deactivate

&emsp;StatefulElement 重写了 deactivate 函数，仅为了添加 state.deactivate 函数调用。

```dart
  @override
  void deactivate() {
    // 8️⃣ 添加 state.deactivate 函数，对 State 的 deactivate 函数进行回调。
    state.deactivate();
    
    // 同样，ComponentElement 没有重写此 deactivate 函数，
    // 所以这里调用的依然是 Element 的 deactivate 函数。
    super.deactivate();
  }
```

## unmount

&emsp;StatefulElement 重写 unmount 函数，首先直接执行 Element.unmount 函数，然后便是对自己的 state 属性的清理，以及 State 对象内部的资源的清理

```dart
  @override
  void unmount() {
    // ComponentElement 没有重写 unmount 函数，
    // 这里直接调用 Element 的 unmount 函数，清理 StatefulElement 相关的内容。
    super.unmount();
    
    // 9️⃣ 添加 state.dispose 函数，State 对象的生命也要走到终点了，回调此 State.dispose 函数，
    // 告诉上层也进行一些 State 相关资源的释放。
    state.dispose();
    
    // 断开 state 对象对当前 StatefulElement 对象的引用，
    // 同时下面也会断开 StatefulElement 对 State 对象的引用，
    // 保证它们两个对象都能正常释放。
    state._element = null;
    // 释放资源以减轻因 defunct 但意外保留的 Element 导致的内存泄漏的严重性。
    _state = null;
  }
```

## dependOnInheritedElement

&emsp;在 State.initState 完成之前调用：`dependOnInheritedWidgetOfExactType<$targetType>` 或者 `dependOnInheritedElement` 函数都是错误的。在 State.dispose 之后调用：`dependOnInheritedWidgetOfExactType<$targetType>` 或者 `dependOnInheritedElement` 也是错误的。

&emsp;在 StatefulElement 中重写 dependOnInheritedElement 函数，其实对生产环境而言没有任何变化，它主要是用于开发阶段添加 State 状态相关的断言的。在 State 对象的错误状态调用了：`dependOnInheritedWidgetOfExactType<$targetType>` 或者 `dependOnInheritedElement` 函数，命中断言，直接抛错给开发者以警示！

&emsp;删除 StatefulElemet.dependOnInheritedElement 中的断言后，发现内部仅是对父类的调用。 

```dart
  @override
  InheritedWidget dependOnInheritedElement(Element ancestor, { Object? aspect }) {
    return super.dependOnInheritedElement(ancestor as InheritedElement, aspect: aspect);
  }
```

## `_didChangeDependencies`

&emsp;标识此 StatefulElement 对象依赖的 InheritedElement 对象的 Widget 的数据发生变化了。

```dart
  bool _didChangeDependencies = false;
```

## didChangeDependencies

&emsp;当此 StatefulElement 对象依赖的 InheritedElement 对象的 Widget 的数据发生变化时，且需要通知所有的依赖者时(InheritedWidget.updateShouldNotify 返回 true)，则 InheritedElement 对象会调用所有依赖者的 didChangeDependencies 函数。

```dart
  @override
  void didChangeDependencies() {
    // ComponentElement 没有重写 didChangeDependencies 函数，
    // 所以这里直接调用的是 Element.didChangeDependencies 函数。
    // 内部比较简单就是直接调用 Element 对象的 markNeedsBuild 函数，
    // 把其标记为脏，并添加到脏 Element 列表中去，下一帧 Element 对象会进行重建。 
    super.didChangeDependencies();
    
    // 然后主要的是把 _didChangeDependencies 置为 true，这样 就可以保证在 StatefulElement 对象进行重建时，
    // State 的 didChangeDependencies 得到回调。
    _didChangeDependencies = true;
  }
```

## StatefulElement 总结

&emsp;OK，到此 StatefulElement 的内容就全部看完了，跟仅仅重写了两个函数（build、update）的 StatelessElement 对比，StatefulElement 重写太多东西，但整体看下来可发现 StatefulElement 重写的内容几乎都是围绕它的 State 属性进行的。

&emsp;特别是跟 State 的生命周期和回调函数相关的内容，我们要重点关注，因为它真的与我们的日常开发息息相关。搞清楚 State 对象的每个回调函数里面能做什么，以及 State 对象 dispose 回调执行时在里面做资源清理防止内存泄漏，更是我们日常开发中要遵守的黄金法则。那么现在让你说说 State 对象相关的生命周期函数和回调函数你能清晰的进行描述吗？

&emsp;首先是 State 对象的出生：当 StatefulWidget 对象调用自己的 newWidget.createElement() 时，会进行 StatefulElement 对象的创建，伴随着 StatefulElement 对象的出生，同时 StatefulWidget 对象也会调用自己的 widget.createState() 函数，此时 State 对象也便出生了。

&emsp;当 StatefulElement 对象创建完毕后，就要把它挂载到 Element Tree 上去了，此时 StatefulElement.mount 函数执行，当 StatfuleElement 通过 Element.mount 挂载好后，就要进行 StatefulElement 对象的第一次构建了：调用 StatefulElement._firstbuild 函数，在它内部呢会首先进行 Statte.initState 和 State.didChangeDependencies 两个 State 回调函数的一前一后的调用，然后便是 StatefulElement 调用 performRebuild 真的进行第一次构建，会第一次调用 StatefulElement.build 函数，而它内部呢则是调用 State.build 函数，此时 State 的 build 函数就被得到调用了。当然除了这第一次的构建，后续 StatefulElement 可能会进行很多次的重新构建，它的每次重新构建都会执行 State.build，即 State 的 build 函数在 State 生命周期中可能会被调用很多次。

&emsp;当 StatefulElement 对象的父级进行重建时，会传递来新的 Widget 对象来更新现有的 StatefulElement 对象，如果新的 Widget 对象是 StatefulWidget 类型，并且可以更新 现有的 StatefulElement 对象时，此时会调用 StatefulElement 的 update 函数，而在 StatefulElement 的 update 函数内部，则会以旧的 StatefulWidget 对象为参数调用 State.didUpdateWidget 函数，即 State 的 didUpdateWidget 回调函数得到调用。同 State.build 一样，在 StatefulElement 对象的生命周期中 State.didUpdateWidget 也可能会被调用多次。

&emsp;上面提到在 StatefulElement 的生命周期中可能会进行很多次的重新构建，它每次进行重新构建的原因不尽相同，其中有一种引发 StatefulElement 对象进行重新构建的原因比较特殊，就是如果当前 StatefulElement 对象依赖了某种类型的 InheritedElement 的话，当这个 InheritedElement 对象的 Widget 的数据发生变化的话，此 StatefulElement 对象会被标记重建，然后当下一帧 StatefulElement 对象将要执行重建的时候，在 StatefulElement 的 performRebuild 函数中会判断是否是因为 StatefulElement 对象的依赖变了导致的重建，如果是的话，会先调用 State.didChangeDependencies 函数，即 State 的 didChangeDependencies 回调函数得到执行，同样的，在 StatefulElement 对象的生命周期中，只要是依赖发生了变化就会被标记重建，即在 StatefulElement 对象的生命周期中 State.didChangeDependencies 也可能会执行多次，且每次它都会先于 State.build 执行。

&emsp;然后还有另外三个比较特殊的回调：State.activate、State.deactivate、State.dispose。

&emsp;上面提到当 StatefulElement 对象的父级（parent）进行重建时，会传递来新的 Widget 对象来更新现有的 StatefulElement 对象，如果新 Widget 对象为 null 的话，那么会把当前的 StatefulElement 对象进行失活处理，在失活过程中会调用 StatefulElement 对象的 deactivate 函数，此时便也会调用 State.deactivate 函数。如果新的 Widget 对象的不能更新现有的 StatefulElement 对象的话，同样也会执行它的失活过程，也会调用到 State.deactivate 函数。

&emsp;当 StatefulElement 对象被失活以后呢，并不会立即执行 unmount，它在下一帧中还有可能被重新激活：假如当前已失活的 StatefulElement 对象的 StatefulWidget 对象有 GlobalKey，并且在下一帧中有和此已失活的 StatefulElement 对象的 StatefulWidget 对象相同的 GlobalKey 的其它 StatefulWidget 对象进行 inflate，则可以取出此 StatefulElement 对象进行复用，并会调用 StatefulElement.activate 使其重新激活，在此过程中 State.activate 也会被调用。当然如果本帧中以失活的 StatefulElement 对象没有被重新激活的机会，则会在本帧结束之前，统一被进行 unmount，StatefulElement.unmout 会被调用，它的内部会调用到 State.dispose，即最终 State.dispose 被执行，后续 State 对象就等待被 GC 回收了。另外如果 StatefulElement 所处的子树，其父级 Element 节点有以上类似 GlobalKey 的 Widget，也有可能会同样顺带着 StatefulElement 进行 StatefulElement.deactivate 和 StatefulElement.activate 的过程。
 
 &emsp;StatefulElement 就总结到这里吧，如果你想搞通 State 相关的内容，记得一定要能捋通 StatefulElement 相关的逻辑。

## 参考链接
**参考链接:🔗**
+ [ComponentElement class](https://api.flutter.dev/flutter/widgets/ComponentElement-class.html)
+ [StatelessElement class](https://api.flutter.dev/flutter/widgets/StatelessElement-class.html)
+ [StatefulElement class](https://api.flutter.dev/flutter/widgets/StatefulElement-class.html)
