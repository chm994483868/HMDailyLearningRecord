# Flutter 源码梳理系列（十三）：Element：constructor -> updateChild

&emsp;看 Element 抽象类的声明，直接继承自 DiagnosticableTree 没啥特别的的，然后便是最重要的：Element 需要实现 BuildContext 抽象类中所有的抽象方法。而这个 BuildContext 就是我们在 StatelessWidget 的 build 和 State 的 build 函数中见了无数次的：BuildContext context 参数，其实 BuildContext 只是定了很多 getter 和抽象方法的一个抽象类，最终实现呢，全部落在了需要实现它的 Element 类身上，而实际在 Widget/State 的 build 函数中传递来的 context 参数便是它们对应的 Element 对象。

&emsp;BuildContext 类中定义了较多 getter 和抽象函数，我们先不去看，暂时还先看 Element 类，毕竟 Element 会实现 BuildContext 抽象类的所有要求。

```dart
abstract class Element extends DiagnosticableTree implements BuildContext {
    // ...
}
```

## constructor

&emsp;Element 的构造函数：创建一个使用给定 widget 作为其配置的 Element 对象，通常由 Widget 子类重写 Widget 的 `Element createElement()` 进行调用。几乎所有的 Widget 子类都重写了 createElement 函数，构建一个对应自己的 Element 对象。（毕竟它是一个抽象类的抽象函数，子类还是要进行自己实现此抽象函数的。）

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

&emsp;然后看到初始化列表，直接把 widget 参数赋值给了 Element 的 `_widget` 字段，从这里可以看出，element 对象会引用它对应的 widget 对象的，毕竟这 widget 是此 element 的配置信息嘛！被 Element 所引用很正常。

&emsp;Element 初始化列表下面跟的一段代码，看起来怪怪的。

&emsp;它们是为了在 Flutter 应用中追踪内存分配的情况的。如果 kFlutterMemoryAllocationsEnabled 为 true，表示内存分配的追踪功能已经开启，则调用 FlutterMemoryAllocations.instance.dispatchObjectCreated 方法，将创建的对象相关信息传递给追踪系统。在这里将传递所属库（library）、类名（className）以及具体对象实例（object）。而这里则正是追踪每一个 Element 对象的创建，然后在 Eelement 的 unmount 处还会追踪此 Eelment 对象的释放。

```dart
// 默认情况下，在 debug 模式下该常量为 true，在 profile 和 release 模式下为 false。
// 要在 release 模式下启用，请传递编译标志：--dart-define=flutter.memory_allocations=true。
const bool kFlutterMemoryAllocationsEnabled = _kMemoryAllocations || kDebugMode;
```

&emsp;FlutterMemoryAllocations 是一个用于监听对象生命周期事件的类。FlutterMemoryAllocations 可以监听 Flutter framework 中 disposable 对象的创建和销毁事件。要监听其他对象事件，调用 FlutterMemoryAllocations.dispatchObjectEvent。使用该类时应该满足 kFlutterMemoryAllocationsEnabled 的条件，以确保在禁用内存分配时不会通过该类的代码增加应用程序的大小。这个类经过优化，适用于处理大量事件流和少量的添加或移除监听器。

&emsp;看了一下感觉此类就是一个监听器，用来监听对象的创建和销毁。例如在 framework.dart 中搜索：FlutterMemoryAllocations.instance，总共发现了四处调用，分别是针对 Element 和 State 对象的创建和销毁：

1. 在 Element 的构造函数处调用：dispatchObjectCreated，在 unmount 函数处调用 dispatchObjectDisposed。
2. 在 State 的 initState 函数处调用：dispatchObjectCreated，在 dispose 函数处调用 dispatchObjectDisposed。

&emsp;统计 Element 和 State 对象的创建和销毁，一一对应，至于其它的深入使用，后续再研究，感觉可以配合检测 Element 和 State 对象的内存泄漏问题。

## `_parent`

&emsp;element 对象有一个指向它的父级 element 的引用，在 mount 时会给 `_parent` 赋值。

+ ComponentElement 有 `Element? _child;`（StatelessElement/StatefulElement/ProxyElement 都是 ComponentElement 子类）
+ SingleChildRenderObjectElement 有 `Element? _child;`
+ MultiChildRenderObjectElement 有 `late List<Element> _children;`

&emsp;它们涵盖了所有的 Element 对象，现在可以看出来了吧：element tree 其实是一个由双向链表构建的树状结构🌲。 

```dart
  Element? _parent;
```

## `_notificationTree`

&emsp;由 `_NotificationNode` 节点构建的 notification tree，前一篇有详细讲解，可以翻回去看看。

```dart
  _NotificationNode? _notificationTree;
```

## operator ==

&emsp;用于比较两个 Element 对象的相等性，看代码的话，它加了 @nonVirtual 注解，表示所有的 Element 子类都不能重写此 operator ==，所有 Element 子类对象比较时也都使用 Element 的 ==，然后是比较方式，是使用全局的 identical 函数，需要完全保证两个 Element 对象是同一个对象。

&emsp;当比较两个 widget 是否相等时，当一个 widget 通过 operator == 与另一个 widget 比较相等时，假定更新是多余的，跳过更新该树分支的工作。通常不建议在任何具有 children widget 的 widget 上重写 operator ==，因为正确的实现将不得不转移到 children widget 的相等运算符，这是一个 O(N²) 的操作：每个 child widget 都需要遍历它的所有 children widget，树上的每一步。

&emsp;如果重新构建 widget 比检查 widget 的参数是否相等更昂贵，而且预计 widget 经常会被相同的参数重新构建，那么有时一个 leaf widget（没有 children 的 widget）实现这个方法是合理的。但是通常情况下，如果已知 widget 不会更改，最好在 build 方法中缓存使用的 widget，这样效率更高。

```dart
  @nonVirtual
  @override
  // ignore: avoid_equals_and_hash_code_on_mutable_classes, hash_and_equals
  bool operator ==(Object other) => identical(this, other);
```

## slot

&emsp;slot 是由 element 父级设置的信息，用来定义子级 element 节点在其父级的子级列表（children）中的位置。当父级调用 updateChild 方法来 inflate 子 widget 时，子 widget 的 slot 位置会确定，等后面我们学习 RenderObjectElement 时再详细学习 slot 相关的内容。（在 Flutter 中只有能创建 RenderObject 的 RenderObjectWidget 才有机会绘制在屏幕上）

&emsp;同时并非所有的 element 对象的 slot 都有值，注意这是给子级列表（children）用的，即只有 MultiChildRenderObjectElement 对象的 children 字段中的 element 对象才需要 slot 值。它表示此 element 节点在这个 MultiChildRenderObjectElement 父级节点中的位置。其它情况的 Element 对象的 slot 都是 null。

&emsp;这里 slot 是使用的 Object? 类型，实际在 MultiChildRenderObjectElement 中，对其子级 Element 使用的是：IndexedSlot 类型，这里直接使用 Object? 也是为了处理不同的 slot 类型的情况。

```dart
  Object? get slot => _slot;
  Object? _slot;
```

&emsp;这里要先注意一下的点，slot 是一个 Object 类型，针对不同的 widget 它有不同的类型，不像是 iOS 中所有的 frame 都是 CGRect 一个类型的。

## depth

&emsp;depth 是一个简单的 int 值，表示 element 在 element tree 中的深度值，如果存在父级，则保证整数大于父级，树的根节点必须具有大于 0 的深度。（通过断点可以发现 RootElement 的 depth 值是 1）

&emsp;depth getter 内部有一个断言：如果 element 处于 `_ElementLifecycle.initial` 状态的话，会报错：Depth 信息仅在 element 已经 mount 后才可读取。

```dart
  int get depth {
    // ...
    return _depth;
  }
  late int _depth;
```

## widget/mounted

&emsp;widget 这里是 element 对象直接引用创建它的对应的 widget 对象。

&emsp;widget 表示该 element 的配置信息。应避免在 Element 的子类型中重写此字段，以提供一个更具体的 widget 类型（例如在 StatelessElement 中重写此字段为： StatelessWidget get widget，直接指定其为 StatelessWidget 类型）。相反，在需要更具体类型时，应在任何调用处进行类型转换。这样可以避免在构建阶段的 "getter" 中产生大量的类型转换开销，而在该过程中访问到这个字段 —— 而且不会使用更具体的 widget 的类型信息。（即 `_widget` 字段还保持 `Widget?` 类型即可，当需要具体类型信息时，使用类型转换）

&emsp;毕竟每个 Element 对象的 widget 字段都是 Widget 的子类，当需要类型信息时再进行转换。

&emsp;mounted 已挂载标识，如果 Element 的 widget 字段不为 null，就表示已经挂载。

```dart
  @override
  Widget get widget => _widget!;
  Widget? _widget;
  
  @override
  bool get mounted => _widget != null;
```

## owner

&emsp;owner 是一个在 Element tree 上传递的全局对象，并且是管理 Widget Framework 生命周期的对象。后面我们会详细看这个 BuildOwner 类型。(在前面的已经讲过啦，可以翻回去看看，极其重要的 owner，一定要学会它)

&mesp;在整个 element tree 上每个 element 节点都会持有它。

```dart
  @override
  BuildOwner? get owner => _owner;
  BuildOwner? _owner;
```

## buildScope

&emsp;当这个 Element 已经被 mounted 时，通常访问这个 getter 才是安全的。默认实现会返回父 Element 的 buildScope，因为在大多数情况下，一个 Element 一旦其祖先不再是脏的就可以准备重建。

&emsp;一个值得注意的例外是 LayoutBuilder 的子孙节点，在接收到约束之前不应该进行重建。LayoutBuilder 的 Element 重写了 buildScope 以确保其所有子孙节点在接收到约束之前都不会进行重建。如果你选择重写这个 getter 来建立自己的 BuildScope，需要在适当的时候手动调用 BuildOwner.buildScope，并传入你的 BuildScope 的根 Element，因为 Flutter framework 不会尝试注册或管理自定义的 BuildScope。如果你重写这个 getter，请始终返回相同的 BuildScope 实例。不支持在运行时更改此 getter 返回的值。

&emsp;updateChild 方法忽略 buildScope：如果父 Element 在一个具有不同 BuildScope 的子 Element 上调用 updateChild，子 Element 可能仍然会重建。

&emsp;前面有 BuildScope 的详细讲解，可以翻回去看看。

```dart
  BuildScope get buildScope => _parentBuildScope!;
  // 父 Element build scope 的缓存值。当该 Element mount 或 reparent 时，缓存会被更新。
  BuildScope? _parentBuildScope;
```

## renderObject

&emsp;当前 element 或者此 element 下的子级 element 的 render object。(即不仅在此 element 找，而且会顺着 element 链继续往下找。)

&emsp;如果此 element 是 RenderObjectElement 的话，那么直接返回它的 renderObject 就可以了。否则，此 getter 会沿 element 树向下查找，直到找到一个 RenderObjectElement 返回它的 renderObject 为止。

&emsp;当然还有沿着 element 树找不到 render object 的情况。element 树中的某些位置没有对应的 render object 的，在这种情况下，此 getter 返回 null。这可能发生在 element 位于 View 之外的情况下，因为只有位于 view 中的 element 子树有与之关联的 render tree。

```dart
  RenderObject? get renderObject {
    Element? current = this;
    
    while (current != null) {
      if (current._lifecycleState == _ElementLifecycle.defunct) {
        break;
      } else if (current is RenderObjectElement) {
        // 如果当前的 Element 对象是 RenderObjectElement 的话，直接返回它的 renderObject 即可
        return current.renderObject;
      } else {
        // 这里则是遍历往当前 element 的 child 中继续去查找
        current = current.renderObjectAttachingChild;
      }
    }
    return null;
  }
```

## renderObjectAttachingChild

&emsp;返回此 element 的子级 element，该子 element 将在此 element 的祖先中插入一个 RenderObject，以构建 render 树。

&emsp;如果此 element 没有任何需要将 RenderObject 附加到此 element 的祖先的子 element，则返回 null。因此，RenderObjectElement 将返回 null，因为其子 Element 将其 RenderObject 插入到 RenderObjectElement 本身而不是其祖先中。

&emsp;此外，对于 hoist 它们自己独立渲染树并且不扩展祖先渲染树的 Element，可能会返回 null。

&emsp;Element 类的 visitChildren 函数是个空实现，但是 

+ ComponentElement/
+ SingleChildRenderObjectElement/
+ MultiChildRenderObjectElement 

&emsp;分别重写了它，并且返回它们的 `_child`，MultiChildRenderObjectElement 的话则是遍历它的：`_children`。

&emsp;Element 并没有 `_child` 字段，但是它的子类添加了此字段，然后配合 `_parent` 字段，可以看出 element tree 是一个由双向链表构建的树状结构🌲。

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

## `_lifecycleState`

&emsp;这被用来验证 element 对象以有秩序的方式在生命周期中移动。

&emsp;表示 element 对象目前处于生命周期的哪个状态，值是 `_ElementLifecycle` 枚举。

```dart
enum _ElementLifecycle {
  initial, // 初创建出来时
  active, // 已经挂载在 Element Tree 上
  inactive, // 被失活了，等待重新激活或者被回收
  defunct, // 无法再被重新激活了，只能等待卸载回收了
}

  _ElementLifecycle _lifecycleState = _ElementLifecycle.initial;
```

## visitChildren

&emsp;这是一个比较特殊的函数，它的参数 visitor 是一个函数：参数是 Element，返回值是 void 的函数：`typedef ElementVisitor = void Function(Element element);`。

&emsp;而且这个函数比较特殊，在 Element 中它是一个空实现，而在有 child 字段的 Element 子类中要重写它，比如：ComponentElement 重写了 visitChildren 函数，当它的 `_child` 不为 null 时，调用 `vistor(_child)`。然后 SingleChildRenderObjectElement 和 ComponentElement 一样的重写，MultiChildRenderObjectElement 则是对自己 `_children` 中的每个 Element child 调用 `vistor(child)`。

&emesp;此函数可以直白的理解为：访问 element 的每个 child。分别以 element 的 child 们为入参调用 visitor 函数。

```dart
  void visitChildren(ElementVisitor visitor) { }
```

## visitChildElements

&emsp;仅仅是为了实现 BuildContext 抽象类中的定义的 visitChildElements 抽象函数。其内部仅仅是封装的 visitChildren 函数调用。

```dart
  @override
  void visitChildElements(ElementVisitor visitor) {
    // 直接调用 visitChildren 函数，仅此而已
    visitChildren(visitor);
  }
```

&emsp;下面我们进入 Element 的核心函数之一：updateChild，超级重要。

## updateChild

&emsp;在 updateChild 函数前有三个很重要的注解，它们的目的都是想优化 updateChild 函数的调用为内联调用。不过此内联优化的场景并不是针对的 Flutter 生产环境。

+ @pragma('dart2js:tryInline') 向 dart2js(Dart-to-JavaScript compiler) 建议在优化期间应优先考虑内联 updateChild 函数。
+ @pragma('vm:prefer-inline') 向 Dart VM 建议在优化期间应优先考虑内联 updateChild 函数。
+ @pragma('wasm:prefer-inline') 向 WebAssembly 建议在优化期间应优先考虑内联 updateChild 函数。

&emsp;貌似有点可惜，此内联优化并不是针对 native APP 生产环境。

&emsp;updateChild 函数的作用：使用给定的新配置（新 Widget 对象）更新给定的 element 对象（也有可能是新建或者移除 Element）。该方法是 widget system 的核心。每当我们根据更新的配置要添加、更新或删除 element 时，就会调用它。

&emsp;newSlot 参数指定了 element slot 的新值。

&emsp;如果 Element? child 入参为 null，而 Widget? newWidget 入参不为 null，则表示我们需要根据此 newWidget 创建新的 element 对象（实际呢是整个 Element 子树）。

&emsp;如果 Widget? newWidget 入参为 null，而 Element? child 入参为 null，则需要将这现有的 child（element 节点）移除（实际呢是整个 Element 子树），因为它不再具有配置了。

&emsp;如果两者均不为 null，则需要将 Element? child 的配置更新为 Widget? newWidget。如果 Widget? newWidget 可以提供给 Element? child（由 Widget.canUpdate 决定），则提供。否则，Element? child 需要被失活处理并为新配置 Widget? newWidget 创建新的 Element 对象（实际呢是整个 Element 子树）。

&emsp;如果两者均为 null，则我们既没有子 element 了，也不需要新建子 element 了，因此我们不做任何操作。

&emsp;updateChild 方法返回新的 child element 对象（如果必须创建一个），或者传入的 child element 对象（如果只需要更新子元素），或 null（如果移除了子 element 且没有需要新建的 element）。

&emsp;下表总结了上述内容:

  |                     | **newWidget == null**  | **newWidget != null**   |
  | :-----------------: | :--------------------- | :---------------------- |
  |  **child == null**  |  Returns null.         |  Returns new Element. |
  |  **child != null**  |  Old child is removed, returns null. | Old child updated if possible, returns child or new Element. |

&emsp;只有在 newWidget 不为 null 时才会使用 newSlot 参数。如果 child 为 null（或者旧 child 无法更新），那么通过 inflateWidget 将 newSlot 分配给为 child 创建的新的 Element 对象。如果 child 不为 null（并且旧 child 可以更新），那么通过 updateSlotForChild 给予 newSlot 来更新其 slot，以防它在上次构建后发生移动。

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
        // 1️⃣1️⃣：child 不为 null，但是 widget 已经无了，所以也需要把这旧 element 失活了并等待内存回收♻️。
    
        // deactivateChild 函数所有的 Element 子类仅用 Element 类的这个，所有子类都没有重写它。
        
        // deactivateChild 把这个指定的 Element child 对象失活，主要做了三件事：
        // 1. 把 child 的 _parent 置为 null。⚠️（重要的点）
        // 2. 再把以 child 为根节点的整个子树上的所有 RenderObjectElement 的 render object 从 RenderObject tree 上分离。
        
        // 3. 再是把 child 对象放入 owner!._inactiveElements 这个当前 BuildOwner 下的非活动 Element Set 中，
        // owner!._inactiveElements.add(child)
        // _elements.add(element)
        // 
        // add 实际内部超复杂，它内部是递归调用 child 的 deactivate 函数，
        // 对 child 的所有子级 Element 对象（通过 element.visitChildren 遍访所有 child 对象）调用 deactivate，
        // 或者说是对以此 child 为根节点的 Element 子树上所有的 Element 节点调用 deactivate 失活函数。
        // deactivate 函数呢：如果是 StatefulElement 的话会加一条调用：state.deactivate，
        // 其它的 Element 子类的话，都是调用 Element.deactivate：
        // 1️⃣. 如果 element 对象依赖了 InheriteElement 的话，遍历此 element 对象的 _dependencies 列表，
        // 把此 element 对象从 InheritedElement 对象的依赖者列表 _dependents 中，移除此 element 对象，
        // 注意，有点绕哦：意思就是如果 element 对象依赖了 InheritedElement，那么就找到具体的 InheritedElement 对象，
        // 把我们这个要失活的 element 对象从 InheritedElement 对象的依赖者列表中移除掉。
        // 2️⃣. 把此 element 的 _inheritedElements 指向 null，状态修改为 inactive。
        
        // 所以整体看下来，感觉对 child 的所有子级 element 节点做的事情并不多，
        // 依然维持了以 child 为根节点的 Element 子树，
        // 但是会断开 child 对其 parent 的引用，并把它加入到当前 BuildOwner 下全局的失活 Element 列表中，
        // 其它的话则是从 RenderObject Tree 上分离下所有 RenderObject，
        // 以及解开对 InheritedElement 的依赖。
        deactivateChild(child);
      }
      
      // 到啦叶子末端啦，已经没有新的 Widget 要构建啦（旧 element 已经被失活啦，如果有的话），updateChild 返回 null 即可
      return null;
    }

    // 其它情况⬇️
    
    // 临时变量，主要用来记录返回值，针对下面这些 newWidget 不为 null 的情况，（必有 Element 要返回。）
    // updateChild 函数的以下部分：要么返回新建的 element 对象，要么返回得到更新的入参 child 对象。
    final Element newChild;
    
    if (child != null) {
      // 2️⃣：针对 child 不为 null 的情况，此 if 内尽量尝试对 child 进行更新，实在不行时才会进行新建 Element 对象。
      
      bool hasSameSuperclass = true;
      // 原代码这里有一段 hasSameSuperclass 的解释，
      // 主要为了防止热重载时，StatefulWidget 和 StatelessWidget 的相互替换导致问题，
      // 而且断言 assert 代码在生产环境时会被移除，为了减少理解负担，
      // 我们就直接给它删掉，把 hasSameSuperclass 理解为常量 true 即可。 
      
      if (child.widget == newWidget) {
        // 2️⃣1️⃣：我们之前学习 const 时已经看过 flutter 对它的优化了，
        // 如果 newWidget 是被 const 构造函数构建，并且加了 const 修饰那么新旧 widget 就是同一个对象，
        // 这里如果 newSlot 与 child element 对象的 slot 不同的话只需更新 child 的 slot 即可。
        if (child.slot != newSlot) {
          updateSlotForChild(child, newSlot);
        }
        
        // 记录下，还是入参 child 对象返回即可
        newChild = child;
      } else if (Widget.canUpdate(child.widget, newWidget)) {
        // 2️⃣2️⃣：如果 newWidget 和旧 widget：runtimeType 和 key 一样的话，那么可以直接更新 child 即可，不需新建 Element 对象。
        
        // 如需要更新 slot，则更新 slot
        if (child.slot != newSlot) {
          updateSlotForChild(child, newSlot);
        }
        
        // Element 类中 update 函数仅是 newWidget 替换 child 的 _widget 属性，
        // 而在 Element 子类中，除了调用 super.update(newWidget) 还，还会调用：rebuild(force: true)，
        // 会强制进行以 child 为根节点的 Element 子树的重建。
        
        // 即会对 child 的 Element 子树进行完整的重建工作，
        // update 函数链路很长的，并非只是简单更新下 child 这独一个 Element 节点的 _widget 而已。
        // StatefulElement 的话还会更新下自己 state 的 _widget，以及回调它的 state.didUpdateWidget 函数，然后：rebuild(force: true)，
        // 然后：ProxyElement、InheritedElement、RenderObjectElement、SingleChildRenderObjectElement、MultiChildRenderObjectElement，
        // 都对 update 进行了重写，后续学习这些 Element 子类时再深入。
        
        // 目前只要记得：它们都会对以 child 为根节的 Element 子树进行重建即可。
        child.update(newWidget);

        newChild = child;
      } else {
        // 2️⃣3️⃣：针对 child 不能用 newWidget 进行更新的情况，只能新建 element （element 子树）了，
        // 要先把旧的 element 给它失活了。 
        deactivateChild(child);
        
        // 使用 newWidget 和 newSlot 开始构建 child 原有父级 element 下的完整的 element 子树，
        // 可以理解为自 child 的原父级 element 为根节点，开始构建它下面的整个 element 子树。
        newChild = inflateWidget(newWidget, newSlot);
      }
    } else {
      // 3️⃣：针对 child 为 null 的情况，表示完整的进行 element 子树的构建！
      newChild = inflateWidget(newWidget, newSlot);
    }

    // 把新的 element 对象或者是更新过的 child element 对象返回即可
    return newChild;
  }
```

&emsp;updateChild 函数的内容还是比较清晰的，针对三个参数 Element? child、Widget? newWidget、Object? newSlot 值不同的情况进行不同的处理。

&emsp;我们看到其中一种最省事的情况：child 参数不为 null，且 child.widget 和 newWidget 相等时，必要的话仅需更新下 child 的 slot（一般情况下不需要），完全不需要对 child 子树进行重建，直接返回 child 对象即可。而这个最省事的情况就是对应了官方推荐的优化技巧：提取封装 widget 子类，声明 const 构造函数，使用 const 常量表达式。

&emsp;如果新旧 Widget 不相等，但是 child 对象可以被复用时，会使用 newWidget 进行更新，然后对其下级 Element 子树进行重建，有可能可以完整的复用整个子树，也有可能只能复用到某个节点后续需要新建子树，视 widget 子级来决定。当不可被复用时，则是新建接下来的整个 element 子树。

&emsp;还有注意 child.update(newWidget) 并不是说只是更新下当前这个 child 节点对象，它是会沿着当前这个 child 节点链一直往下子级进行重建的，同样是对 child 子树的重建，同样是直到末端没有新 widget 对象了为止，只是说本次复用了这个 child Element 对象而已。

&emsp;还有我们应该也注意到了，貌似以上 updateChild 内的流程好像都是针对一根 Element 节点 **链** 进行的，过程中我们还没有怎么深入讲过 MultiChildRenderObjectElement 和 MultichildRenderObjectWidget 相关的调用呢，它们可都是有一组子级的：`final List<Widget> children` / `late List<Element> _children`，那么当涉及 MultiChildRenderObjectWidget 的一组子级 widget 时，该怎么进行更新和新建呢？那么接下来我们看它们所涉及到的最核心的 updateChildren 函数。 

&emsp;首先看一下这两个函数的定义：

+ `Element? updateChild(Element? child, Widget? newWidget, Object? newSlot)`
+ `List<Element> updateChildren(List<Element> oldChildren, List<Widget> newWidgets, { Set<Element>? forgottenChildren, List<Object?>? slots })`  

&emsp;观察一下会发现：updateChildren 的参数 `List<Element> oldChildren` 和返回值都是没有 ？的，它们都不是可选的，当 updateChildren 被调用时，入参 oldChildren 都是有值的，即：当每次调用 updateChildren 函数时都是对现有的一组 Element 对象使用 newWidgets 对其进行更新。

&emsp;而 updateChild 则是包括：新建/更新/移除 element 子树三种情况的，特别是入参 Element? child 为 null 和入参 Widget? newWidget 不为 null 时，会沿着这个 newWidget 创建新的 element 子树，然后对比 updateChildren 函数的话，updateChildren 被调用时入参 oldChildren 必是有值的，它是对 oldChildren 中现有的一组 element 对象进行更新操作，或者是说对现有的 `List<Element> oldChildren` 中的一组 Element 子树进行更新操作。

&emsp;所以 updateChildren 函数的出发点：就是对一组现有的 element 节点进行更新操作，然后在 framework.dart 进行搜索，可以发现 updateChildren 函数仅有的一处调用是在 MultiChildRenderObjectElement 的 update 函数中。

&emsp;OK，接下来我们正式开始看 updateChildren 的内容吧，它是 Element 类中最复杂的一个函数，翻过这座大山⛰️以后，其它 Element 类的内容我们就可以一马平川啦！⛽️

## 参考链接
**参考链接:🔗**
+ [Element class](https://api.flutter.dev/flutter/widgets/Element-class.html)
+ [kFlutterMemoryAllocationsEnabled top-level constant](https://api.flutter.dev/flutter/foundation/kFlutterMemoryAllocationsEnabled-constant.html)
+ [Flutter | Understanding the MemoryAllocations](https://medium.com/@maciejbrzezinski/flutter-what-is-memoryallocations-1ee2eb0a8670)
+ [Flutter | How to create a custom binding and inject your own BuildOwner](https://medium.com/@maciejbrzezinski/flutter-how-to-create-a-custom-binding-and-inject-your-own-buildowner-f60ef320537b)
+ [Support for WebAssembly (Wasm)](https://docs.flutter.dev/platform-integration/web/wasm)
