# Element

> &emsp;基于：Flutter 3.22.2 • 2024-06-06 09:51 • 761747bfc5 • stable

&emsp;下面开始学习整个 Flutter framework 最核心的一个类：Element。

&emsp;Element 是树中特定位置的 Widget 的实例化。

&emsp;Widget 描述如何配置子树，但同一个 widget 可以同时用于配置多个子树，因为 widget 是不可变的。一个 Element 代表了在树中的特定位置配置 widget 的使用。随着时间的推移，与给定 Element 相关联的 widget 可能会发生变化，例如，如果父 widget 重建并为此位置创建了一个新的 widget。

&emsp;Element 组成一棵树。大多数 element 都有一个唯一的 child，但是一些 widget（例如 RenderObjectElement 的子类）可以有多个 child（children，如：Row、Column 等）。

&emsp;Element 的生命周期如下：

+ Flutter framework 通过调用 Widget.createElement 在将作为 element 初始配置的 widget 上来创建一个 element。 (createElement 仅有的一次调用是在 Element 的 inflateWidget 中由 newWidget 调用：**final Element newChild = newWidget.createElement();**)

+ Flutter framework 框架调用 mount 方法，将新创建的 element 添加到树中的给定父级的给定 slot 中。mount 方法负责 inflate 任何子 widget，并根据需要调用 attachRenderObject 方法将任何关联的 render object 附加到 render object tree 中。(当上面的 newWidget.createElement() 调用完成返回 Element newChild，然后就会直接调用：**newChild.mount(this, newSlot);**，上面说的给定父级的给定的 slot，就是指调用 mount 函数传的两个参数 parent 和 newSlot：**void mount(Element? parent, Object? newSlot)**。而后半句根据需要调用 attachRenderObject 就是指的：RenderObjectElement 的 attachRenderObject 方法，在 RenderObjectElement 的 mount 中，它会直接调用：**void attachRenderObject(Object? newSlot)** 方法把 renderObject 附加到 render object tree 上去。)

+ 在这种情况下，该 element 被视为 "active"，可能会出现在屏幕上。

+ 在某些情况下，父级 widget 可能会决定更改用于配置此 element 的 widget，例如因为父级 widget 使用新 state 重建了。当发生这种情况时，Flutter framework 将使用新 widget 调用 update 函数。新 widget 将始终具有与旧 widget 相同的 runtimeType 和 key。如果父级 widget 希望在树中的此位置更改 widget 的 runtimeType 或 key，可以通过卸载此 element 并在此位置 inflate new widget 来实现。(update 函数是 Element 的一个函数，而且它由一个 @mustCallSuper 注解，字面意思的就是所有的 element 子类重写 update 函数时，必须要调用 super.update(newWidget)，这个 update 函数，几乎所有的 Element 子类都重写了，基本实现内容就是拿这个传来的 new widget 执行 Rebuild，调用整个 Element 最重要的：**Element? updateChild(Element? child, Widget? newWidget, Object? newSlot)** 函数。)

+ 在某些情况下，祖先 element(ancestor element)可能会决定将当前 element（或者中间祖先 element）从树上移除，祖先 element 通过调用 deactivateChild 函数来实现这个操作。当中间祖先 element 被移除时，该 element 的 render object 就会从 render tree 中移除，并将当前 element 添加到 owner 的不活跃元素列表(inactive elements)中，这会导致 Flutter framework 调用当前 element 的 deactivate 方法。

+ 在这种情况下，element 被认为是 "inactive"，不会出现在屏幕上。一个 element 只能保持在 inactive 状态直到当前动画帧结束。在动画帧结束时，任何仍然处于 inactive 状态的 element 将会被卸载。(即当前帧结束了，收集的那些依然处于非活动状态的 element 就可以被 GC 回收了，这个是对移除的 element 的优化复用机制，主导思想就是：如果 element 能复用就不进行新建。)  换句话说，如果一个 element 在当前帧没有在屏幕上展示出来，那么它将会被移除(unmounted)。

+ 如果一个 element 被重新加入到树中（例如，因为它或它的祖先之一使用的 global key 被重用了），Flutter framework 会从 owner 的非活动元素列表(list of inactive elements)中移除该 element，调用该 element 的 activate 函数，然后将该 element 的 render object 重新附加到 render tree 中。（在这一点上，该 element 再次被认为是 "active"，可能会出现在屏幕上。）

+ 如果一个 element 在当前动画帧结束时没有重新加入到树中，那么 Flutter framework 会调用该 element 的 unmount 方法。

+ 在这种情况下，这个 element 被认为是 "defunct"，并且将来不会被加入到树中。换句话说，这个 element 已经被标记为不再需要，不会被使用到。

&mesp;OK，下面我们开始看 Element 的源码，说到底还是看代码的话，条理比较清晰，但是在正式看 Element 之前，我们先通过一个简单的示例代码，并通过打断点，看下函数堆栈。

&emsp;我们准备了一个极简单的页面，主要帮助我们梳理两个过程：

```dart
void main() {
  runApp(const MyUpdateApp());
}

class MyUpdateApp extends StatelessWidget {
  const MyUpdateApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
          appBar: AppBar(title: const Text('Element Study')),
          body: const Center(child: OneWidget())),
    );
  }
}

class OneWidget extends StatefulWidget {
  const OneWidget({super.key});

  @override
  State<StatefulWidget> createState() => _OneWidgetState();
}

class _OneWidgetState extends State<OneWidget> {
  void _click() {
    setState(() {});
  }

  @override
  Widget build(BuildContext context) {
    debugPrint('_OneWidgetState build');

    return ElevatedButton(onPressed: _click, child: const Text('Click me'));
  }
}
```

1. Widget 初次在页面上呈现的过程。
2. 当调用 setState 后，Widget 的更新过程。

&emsp;这两个过程中涉及的函数调用栈是我们的关注重点，它们会把 Element 的各个函数串联起来。

&emsp;虽然我们的示例代码只是看一个 Widget 层级较少的页面，但其实复杂 Widget 层级的构建流程是一样的，只是复杂 Widget 页面有更多的完全一样的重复构建过程而已，但其实只要我们能看懂一层的构建流程即可，再多的 Widget 层级每层的构建流程也都是一样的。

&emsp;当我们自己在 StatelessWidget 子类或者 State 子类重写的 build 函数意味着什么？首先从代码角度看 build 函数的话，它只是一个有着一个 BuildContext 参数和返回值 Widget 的普通函数，它实际跟我们自己写一个有参数和返回值 Widget 的函数没什么两样，那么每当 build 函数被调用的话，实际就会返回一个新的 Widget 对象了。

&emsp;我们继续思考，如果 build 函数被调用了，那么返回了一个新 Widget 对象后要做什么用呢？以日常我们自己创建的 Widget 子类初次构建为例，它意味着当前父级 element 要构建它的子 element 了，build 函数返回的 Widget 就是为创建这个新子 element 准备的，Widget 对象调用自己的 createElement 函数，便会创建引用自己的 element 对象(Element 构造函数初始化列表便把 Widget 对象赋值给自己的 `_widget` 字段)。

&emsp;那不妨再往前一点，Widget 对象是先于 Element 对象创建的，还记得 Widget 的抽象函数 Element createElement() 吗？是的，我们必是要先有了 Widget 对象才能调用它的 createElement 函数，创建一个 Element 对象出来。那再往前一点，APP 刚启动时呢？此时还没有任何 Element 呢，先有的第一个 Widget 对象是谁呢？第一个 Widget 对象必是我们传递给 runApp 函数的 const MyUpdateApp() 对象！





## element

&emsp;看 Element 的声明，直接继承自 DiagnosticableTree 没啥特别的的，然后便是最重要的：Element 需要实现 BuildContext 抽象类中所有的抽象方法。而这个 BuildContext 就是我们在 StatelessWidget 的 build 和 State 的 build 函数中见了无数次的：BuildContext context 参数，其实 BuildContext 只是定了很多 getter 和抽象方法的一个抽象类，最终实现呢，全部落在了需要实现它的 Element 类身上，而实际在 Widget/State 的 build 函数中传递来的 context 参数便是它们对应的 Element 对象。

&emsp;BuildContext 类中定义了较多 getter 和抽象函数，我们先不去看，暂时还先看 Element 类，毕竟 Element 会实现 BuildContext 抽象类的所有要求。

```dart
abstract class Element extends DiagnosticableTree implements BuildContext {
    // ...
}
```

### constructor

&emsp;Element 的构造函数：创建一个使用给定 widget 作为其配置的 Element 对象，通常由子 Widget 重写 Widget 的：`Element createElement()` 进行调用。几乎所有的子 Widget 都重写了 createElement 函数，构建一个子 Element。（毕竟它是一个抽象类的抽象函数，子类还是要进行自己实现此函数的。）

```dart
  Element(Widget widget)
    : _widget = widget {
    if (kFlutterMemoryAllocationsEnabled) {
      FlutterMemoryAllocations.instance.dispatchObjectCreated(
        library: _flutterWidgetsLibrary,
        className: '$Element',
        object: this,
      );
    }
  }
```

&emsp;然后看到初始化列表，直接把 widget 参数赋值给了 Element 的 `_widget` 字段，从这里可以看出，Element 会引用它对应的 Widget 的，毕竟这 Widget 是此 Element 的配置信息嘛！被 Element 所引用很正常。

&emsp;Element 初始化列表下面跟的一段代码，看起来怪怪的。

&emsp;它们是为了在 Flutter 应用中追踪内存分配的情况的。如果 kFlutterMemoryAllocationsEnabled 为 true，表示内存分配的追踪功能已经开启，则调用 FlutterMemoryAllocations.instance.dispatchObjectCreated 方法，将创建的对象相关信息传递给追踪系统。在这里将传递所属库（library）、类名（className）以及具体对象实例（object）。

```dart
// 默认情况下，在 debug 模式下该常量为 true，在 profile 和 release 模式下为 false。
// 要在 release 模式下启用，请传递编译标志：--dart-define=flutter.memory_allocations=true。
const bool kFlutterMemoryAllocationsEnabled = _kMemoryAllocations || kDebugMode;
```

&emsp;FlutterMemoryAllocations 是一个用于监听对象生命周期事件的类。FlutterMemoryAllocations 可以监听 Flutter framework 中 disposable 对象的创建和销毁事件。要监听其他对象事件，调用 FlutterMemoryAllocations.dispatchObjectEvent。使用该类时应该满足 kFlutterMemoryAllocationsEnabled 的条件，以确保在禁用内存分配时不会通过该类的代码增加应用程序的大小。这个类经过优化，适用于处理大量事件流和少量的添加或移除监听器。

&emsp;看了一下感觉此类就是一个监听器，用来监听对象的创建和销毁。例如在 framework.dart 中搜索：FlutterMemoryAllocations.instance，总共发现了四处调用，分别是针对 Element 和 State 的创建和销毁：

1. 在 Element 的构造函数处调用：dispatchObjectCreated，在 unmount 函数处调用 dispatchObjectDisposed。
2. 在 State 的 initState 函数处调用：dispatchObjectCreated，在 dispose 函数处调用 dispatchObjectDisposed。

&emsp;统计 Element 和 State 对象的创建和销毁，一一对应，至于其它的深入使用，后续再研究。

### `_parent`

&emsp;element 有一个指向它的父级 element 的引用，在 mount 时会给 `_parent` 赋值，然后 ComponentElement 有 `Element? _child;`（StatelessElement/StatefulElement/ProxyElement 都是 ComponentElement 子类），SingleChildRenderObjectElement 有 `Element? _child;`，MultiChildRenderObjectElement 有 `late List<Element> _children;`，现在可以看出来了吧，element tree 其实是一个双向链表结构。 

```dart
  Element? _parent;
```

### `_notificationTree`

### operator ==

&emsp;用于比较两个 Element 的相等性，看代码的话，它加了 @nonVirtual 注解，表示所有的 Element 子类都不能重写 operator ==，它们的对象比较时也都使用 Element 的 ==，然后是比较方式，是使用全局的 identical 函数，需要完全保证两个 Element 对象是同一个对象。

&emsp;看代码明明是 other 和 this 的比较，是两个 Element 的比较，但是注释中说的是 Widget 的比较：

&emsp;比较两个 widget 是否相等。当一个 widget 通过 operator == 与另一个 widget 比较相等时，假定更新是多余的，跳过更新该树分支的工作。通常不建议在任何具有 children widget 的 widget 上重写 operator ==，因为正确的实现将不得不转移到 children widget 的相等运算符，这是一个 O(N²) 的操作：每个 child widget 都需要遍历它的所有 children widget，树上的每一步。

&emsp;如果重新构建 widget 比检查 widget 的参数是否相等更昂贵，而且预计 widget 经常会被相同的参数重新构建，那么有时一个 leaf widget（没有 children 的 widget）实现这个方法是合理的。但是通常情况下，如果已知 widget 不会更改，最好在 build 方法中缓存使用的 widget，这样效率更高。

```dart
  @nonVirtual
  @override
  // ignore: avoid_equals_and_hash_code_on_mutable_classes, hash_and_equals
  bool operator ==(Object other) => identical(this, other);
```

### slot

&emsp;slot 类似我们在 iOS 原生中 View 的 frame，都是表示子级在父级中的位置。（不同的是 iOS 原生中一般我们画的每个 View 都会正常显示在画面上，而 Widget/Element 则不一定，只有 RenderObjectWidget 才有机会显示在屏幕上。）

&emsp;slot 是父级设置的信息，用来定义子级在其父级的子级列表中的位置。当父级调用 updateChild 方法来 inflate 子 widget 时，子 widget 的 slot 位置会确定。等后面我们学习 RenderObjectElement 时再详细学习 slot 相关的内容。（在 Flutter 中只有能创建 RenderObject 的 RenderObjectWidget 才有机会绘制在屏幕上。）

```dart
  Object? get slot => _slot;
  Object? _slot;
```

&emsp;这里要先注意一下的点，slot 是一个 Object 类型，针对不同的 widget 它有不同的类型，不像是 iOS 中所有的 frame 都是 CGRect 一个类型的。

### depth

&emsp;depth 是一个简单的 int 值，表示 element 在 element tree 中的深度值，如果存在父级，则保证整数大于父级，树的根节点必须具有大于 0 的深度。

&emsp;depth getter 内部有一个断言：如果 element 处于 `_ElementLifecycle.initial` 状态的话，会报错：Depth 信息仅在 element 已经 mount 后才可读取。

```dart
  int get depth {
    // ...
    return _depth;
  }
  late int _depth;
```

### widget/mounted

&emsp;widget 这里是 Element 直接引用了它对应的 widget。

&emsp;widget 表示该 element 的配置信息。应避免在 Element 的子类型中重写此字段，以提供一个更具体的 widget 类型（例如在 StatelessElement 中重写此字段为： StatelessWidget get widget，直接指定其为 StatelessWidget 类型）。相反，在需要更具体类型时，应在任何调用处进行类型转换。这样可以避免在构建阶段的 "getter" 中产生大量的类型转换开销，而在该过程中访问到这个字段 —— 而且不会使用更具体的 widget 的类型信息。

&emsp;mounted 已挂载标识，如果 Element 的 widget 字段不为 null，就表示已经挂载。

```dart
  @override
  Widget get widget => _widget!;
  Widget? _widget;
  
  @override
  bool get mounted => _widget != null;
```

### owner

&emsp;owner 是一个在 Element tree 上传递的全局对象，并且是管理 element 生命周期的对象。后面我们会详细看这个 BuildOwner 类型。

```dart
  @override
  BuildOwner? get owner => _owner;
  BuildOwner? _owner;
```

### buildScope

&emsp;当这个 Element 已经被 mounted 时，通常访问这个 getter 才是安全的。默认实现会返回父 Element 的 buildScope，因为在大多数情况下，一个 Element 一旦其祖先不再是脏的就可以准备重建。一个值得注意的例外是 LayoutBuilder 的子孙节点，在接收到约束之前不应该进行重建。LayoutBuilder 的 Element 重写了 buildScope 以确保其所有子孙节点在接收到约束之前都不会进行重建。如果你选择重写这个 getter 来建立自己的 BuildScope，需要在适当的时候手动调用 BuildOwner.buildScope，并传入你的 BuildScope 的根 Element，因为 Flutter framework 不会尝试注册或管理自定义的 BuildScope。如果你重写这个 getter，请始终返回相同的 BuildScope 实例。不支持在运行时更改此 getter 返回的值。updateChild 方法忽略 buildScope：如果父 Element 在一个具有不同 BuildScope 的子 Element 上调用 updateChild]，子 Element 可能仍然会重建。

&emsp;有点晕，后面再来看。

```dart
  BuildScope get buildScope => _parentBuildScope!;
  // 父 Element build scope 的缓存值。当该 Element mount 或 reparent 时，缓存会被更新。
  BuildScope? _parentBuildScope;
```

### renderObject

&emsp;当前 element 或者此 element 下的 element 的 render object。(即不仅在此 element 找，而且会顺着 element 链继续往下找。)

&emsp;如果此 element 是 RenderObjectElement 的话，那么直接返回它的 renderObject 就可以了。否则，此 getter 会沿 element 树向下查找，直到找到一个 RenderObjectElement 返回它的 renderObject 为止。

&emsp;当然还有沿着 element 树找不到 render object 的情况。element 树中的某些位置没有对应的 render object 的，在这种情况下，此 getter 返回 null。这可能发生在 element 位于 View 之外的情况下，因为只有位于 view 中的 element 子树有与之关联的 render tree。

```dart
  RenderObject? get renderObject {
    Element? current = this;
    while (current != null) {
      if (current._lifecycleState == _ElementLifecycle.defunct) {
        break;
      } else if (current is RenderObjectElement) {
        return current.renderObject;
      } else {
        // 这里则是遍历往当前 element 的 child 中去找
        current = current.renderObjectAttachingChild;
      }
    }
    return null;
  }
```

### renderObjectAttachingChild

&emsp;返回此 element 的子 element，该子 element 将在此 element 的祖先中插入一个 RenderObject，以构建 render 树。

&emsp;如果此 element 没有任何需要将 RenderObject 附加到此 element 的祖先的子 element，则返回 null。因此，RenderObjectElement 将返回 null，因为其子 Element 将其 RenderObject 插入到 RenderObjectElement 本身而不是其祖先中。

&emsp;此外，对于 hoist 它们自己独立渲染树并且不扩展祖先渲染树的 Element，可能会返回 null。

&emsp;Element 的 visitChildren 函数是个空实现，但是 ComponentElement/SingleChildRenderObjectElement/MultiChildRenderObjectElement 实现了它，并且返回它们的 `_child`，MultiChildRenderObjectElement 的话则是遍历它的：`_children`。

&emsp;Element 并没有 `_child` 字段，但是它的子类添加了此字段，然后配合 `_parent` 字段，可以看出 element tree 是一个双向链表结构。

```dart
  @protected
  Element? get renderObjectAttachingChild {
    Element? next;
    visitChildren((Element child) {
      next = child;
    });
    return next;
  }
```

### `_lifecycleState`

&emsp;这被用来验证 element 对象以有秩序的方式在生命周期中移动。

&emsp;表示 element 目前处于生命周期的哪个状态。

```dart
enum _ElementLifecycle {
  initial,
  active,
  inactive,
  defunct,
}

  _ElementLifecycle _lifecycleState = _ElementLifecycle.initial;
```

### visitChildren

&emsp;这是一个比较特殊的函数，它的参数 visitor 是一个参数是 Element，返回值是 void 的函数：`typedef ElementVisitor = void Function(Element element);`。

&emsp;而且这个函数比较特殊，在 Element 中它是一个空实现，而在有 child 的 Element 子类中要重写它，比如：ComponentElement 重写了 visitChildren：当它的 `_child` 不为 null 时，调用 `vistor(_child)`。然后 SingleChildRenderObjectElement 和 ComponentElement 一样的重写，MultiChildRenderObjectElement 则是对自己 `_children` 中的每个 Element child 调用 `vistor(child)`。

```dart
  void visitChildren(ElementVisitor visitor) { }
```

### visitChildElements

&emsp;仅仅是为了实现 BuildContext 的 visitChildElements 抽象函数，内部是封装的 visitChildren 函数。

```dart
  @override
  void visitChildElements(ElementVisitor visitor) {
    visitChildren(visitor);
  }
```

&emsp;下面我们进入 Element 的核心函数之一：updateChild，超级重要。

### updateChild

&emsp;在 updateChild 函数前有三个很重要的注解：

+ @pragma('dart2js:tryInline') 向 dart2js(Dart-to-JavaScript compiler) 建议在优化期间应优先考虑内联 updateChild 函数。
+ @pragma('vm:prefer-inline') 向 Dart VM 建议在优化期间应优先考虑内联 updateChild 函数。
+ @pragma('wasm:prefer-inline') 向 WebAssembly 建议在优化期间应优先考虑内联 updateChild 函数。

&emsp;貌似有点可惜，此内联优化并不是针对 native APP 生产环境。

&emsp;updateChild 函数的作用：使用给定的新配置更新给定的子 element。该方法是 widget system 的核心。每当我们根据更新的配置要添加、更新或删除子 element 时，就会调用它。

&emsp;newSlot 参数指定了 element slot 的新值。

&emsp;如果子 element 为 null（Element? child 参数），而 newWidget 不为 null（Widget? newWidget 参数），则表示我们有一个需要创建 Element 并配置为 newWidget 的新 child Element。

&emsp;如果 newWidget 为 null（Widget? newWidget 参数），而子 element 不为 null（Element? child 参数），则需要将其移除，因为它不再具有配置。

&emsp;如果两者均不为 null，则需要将子 element 的配置更新为 newWidget 给出的新配置。如果 newWidget 可以提供给现有子 element（由 Widget.canUpdate 决定），则提供。否则，旧子 element 需要被处理并为新配置创建新的 child Element。

&emsp;如果两者均为 null，则我们既没有子 element，也将不会有子 element，因此我们不做任何操作。

&emsp;updateChild 方法返回新的 child element（如果必须创建一个），或者传入的 child element（如果只需要更新子元素），或 null（如果移除了子 element 且没有替换）。

&emsp;下表总结了上述内容:

  |                     | **newWidget == null**  | **newWidget != null**   |
  | :-----------------: | :--------------------- | :---------------------- |
  |  **child == null**  |  Returns null.         |  Returns new Element. |
  |  **child != null**  |  Old child is removed, returns null. | Old child updated if possible, returns child or new Element. |

&emsp;只有在 newWidget 不为 null 时才会使用 newSlot 参数。如果 child 为 null（或者旧 child 无法更新），那么通过 inflateWidget 将 newSlot 分配给为 child 创建的新 Element。如果 child 不为 null（并且旧 child 可以更新），那么通过 updateSlotForChild 给予 newSlot 来更新其 slot，以防它在上次构建后发生移动。

&emsp;好了，针对上述的 5 种情况，我们看代码：

```dart
  @protected
  @pragma('dart2js:tryInline')
  @pragma('vm:prefer-inline')
  @pragma('wasm:prefer-inline')
  Element? updateChild(Element? child, Widget? newWidget, Object? newSlot) {
    if (newWidget == null) {
    // 1️⃣：针对 newWidget 为 null 时，如果 child 不为空，则现在已经不需要它了，直接把它失活了先。
    
      if (child != null) {
        // 1️⃣1️⃣：child 不为 null，但是 widget 已经无了，所以也需要把这旧 element 失活等待内存回收♻️。
    
        // deactivateChild 函数所有的 Element 仅有这一个，子类都没有重写。
        // deactivateChild 把这个指定的 Element child 失活，主要做了三件事：
        // 1. 把 child 的 _parent 置为 null。
        // 2. 把 child 的 render object 从 render tree 上分离。
        // 3. 把 child 放入 owner!._inactiveElements 这个全局的 非活动 Element Set 中，等待被复用。
        deactivateChild(child);
      }
      
      // 旧 element 已经被移除啦，updateChild 返回 null 即可
      return null;
    }

    // 临时变量，主要用来记录返回值，针对下面这些 newWidget 不为 null 的情况，
    // updateChild 函数要么返回新建的 element，要么返回得到更新的旧 element。
    final Element newChild;
    
    if (child != null) {
      // 2️⃣：针对 child 不为 null 的情况，此 if 内尽量尝试对 child 进行更新，实在不行时才进行新建。
      
      bool hasSameSuperclass = true;
      // 原代码这里有一段 hasSameSuperclass 的解释，
      // 主要为了防止热重载时，StatefulWidget 和 StatelessWidget 的相互替换导致问题，
      // 而且断言 assert 代码在生产环境时会被移除，为了减少理解负担，我们就直接给它删掉了。
      
      if (hasSameSuperclass && child.widget == newWidget) {
        // 2️⃣1️⃣：我们之前学习 const 时已经看过 flutter 对它的优化了，
        // 如果 newWidget 是被 const 构造函数构建，并且加了 const 修饰那么新旧 widget 就是同一个对象，
        // 这里如果 newSlot 与 child element 的 slot 不同的话只需更新 child 的 slot 即可。
        if (child.slot != newSlot) {
          updateSlotForChild(child, newSlot);
        }
        
        // 记录下，还是旧 element 返回
        newChild = child;
      } else if (hasSameSuperclass && Widget.canUpdate(child.widget, newWidget)) {
        // 2️⃣2️⃣：如果 newWidget 和旧 widget：runtimeType 和 key 一样的话，那么可以直接更新 child 即可，不需新建。
        
        // 如需要更新 slot，则更新 slot
        if (child.slot != newSlot) {
          updateSlotForChild(child, newSlot);
        }
        
        // 此处原有一段 FlutterTimeline 的代码，生产环境无用，暂时删掉不看。
        
        // newWidget 仅需更新 child 即可
        child.update(newWidget);

        newChild = child;
      } else {
        // 2️⃣3️⃣：针对 child 不能用 newWidget 进行更新的情况，只能新建 element 了，
        // 把旧的 element 给它失活了。 
        deactivateChild(child);
        
        // 使用 newWidget 和 newSlot 开始构建新的子 element tree
        newChild = inflateWidget(newWidget, newSlot);
      }
    } else {
      // 3️⃣：针对 child 为 null 的情况，只能进行新子 element tree 的构建了。
      newChild = inflateWidget(newWidget, newSlot);
    }

    // 把新的 element 或者是更新过的旧 element 返回即可
    return newChild;
  }
```

&emsp;updateChild 函数的内容还是比较清晰的，针对三个参数 Element? child、Widget? newWidget、Object? newSlot 值不同的情况进行不同的处理。

&emsp;我们看到其中一种最省事的情况：新旧 Widget 相等时，widget 不进行任何操作（此 newWidget 的 build 也不会被执行），没有任何开销，仅有的一点是新旧 slot 不同的话，会更新下 element 的 slot。而这个最省事的情况就是对应了官方推荐的优化技巧：提取封装子 widget，声明 const 构造函数，使用时添加 const 修饰使用常量表达式。

&emsp;Element.updateChild -> StatefulElement.update -> Element.rebuild -> StatefulElement.performRebuild -> ComponentElement.performRebuild -> StatefulElement.build -> `_ThreeWidgetState.build`。

1. BuildOwner.buildScope -> `BuildScope._flushDirtyElements` -> `BuildScope._tryRebuild` -> Element.rebuild -> StatefulElement.performRebuild(ComponentElement.performRebuild) -> StatefulElement.build -> `_OneWidgetState.build` -> 调完这个 build 就拿到返回到新 widget 啦，接下来就是调用 element 的：`_child = updateChild(_child, built, slot);` 啦，这个 build 返回的 widget 就是当前 element 的下一个节点的 widget，然后沿着 element 链顺序向下更新 element 节点。

2. 所以到这里就要理解 StatelessWidget/State 的 build 函数返回的 widget 就是当前 element 节点的下个节点的 widget，即：`_child._widget` 就是这个 build 返回的 widget 了。




## 参考链接
**参考链接:🔗**
+ [Element class](https://api.flutter.dev/flutter/widgets/Element-class.html)
+ [kFlutterMemoryAllocationsEnabled top-level constant](https://api.flutter.dev/flutter/foundation/kFlutterMemoryAllocationsEnabled-constant.html)
+ [Flutter | Understanding the MemoryAllocations](https://medium.com/@maciejbrzezinski/flutter-what-is-memoryallocations-1ee2eb0a8670)
+ [Flutter | How to create a custom binding and inject your own BuildOwner](https://medium.com/@maciejbrzezinski/flutter-how-to-create-a-custom-binding-and-inject-your-own-buildowner-f60ef320537b)
+ [Support for WebAssembly (Wasm)](https://docs.flutter.dev/platform-integration/web/wasm)
+ []()
