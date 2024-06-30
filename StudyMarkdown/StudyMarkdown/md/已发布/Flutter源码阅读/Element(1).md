# element

&emsp;看 Element 的声明，直接继承自 DiagnosticableTree 没啥特别的的，然后便是最重要的：Element 需要实现 BuildContext 抽象类中所有的抽象方法。而这个 BuildContext 就是我们在 StatelessWidget 的 build 和 State 的 build 函数中见了无数次的：BuildContext context 参数，其实 BuildContext 只是定了很多 getter 和抽象方法的一个抽象类，最终实现呢，全部落在了需要实现它的 Element 类身上，而实际在 Widget/State 的 build 函数中传递来的 context 参数便是它们对应的 Element 对象。

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

&emsp;它们是为了在 Flutter 应用中追踪内存分配的情况的。如果 kFlutterMemoryAllocationsEnabled 为 true，表示内存分配的追踪功能已经开启，则调用 FlutterMemoryAllocations.instance.dispatchObjectCreated 方法，将创建的对象相关信息传递给追踪系统。在这里将传递所属库（library）、类名（className）以及具体对象实例（object）。

```dart
// 默认情况下，在 debug 模式下该常量为 true，在 profile 和 release 模式下为 false。
// 要在 release 模式下启用，请传递编译标志：--dart-define=flutter.memory_allocations=true。
const bool kFlutterMemoryAllocationsEnabled = _kMemoryAllocations || kDebugMode;
```

&emsp;FlutterMemoryAllocations 是一个用于监听对象生命周期事件的类。FlutterMemoryAllocations 可以监听 Flutter framework 中 disposable 对象的创建和销毁事件。要监听其他对象事件，调用 FlutterMemoryAllocations.dispatchObjectEvent。使用该类时应该满足 kFlutterMemoryAllocationsEnabled 的条件，以确保在禁用内存分配时不会通过该类的代码增加应用程序的大小。这个类经过优化，适用于处理大量事件流和少量的添加或移除监听器。

&emsp;看了一下感觉此类就是一个监听器，用来监听对象的创建和销毁。例如在 framework.dart 中搜索：FlutterMemoryAllocations.instance，总共发现了四处调用，分别是针对 Element 和 State 对象的创建和销毁：

1. 在 Element 的构造函数处调用：dispatchObjectCreated，在 unmount 函数处调用 dispatchObjectDisposed。
2. 在 State 的 initState 函数处调用：dispatchObjectCreated，在 dispose 函数处调用 dispatchObjectDisposed。

&emsp;统计 Element 和 State 对象的创建和销毁，一一对应，至于其它的深入使用，后续再研究，感觉可以检测 Element 和 State 对象的内存泄漏问题。

## `_parent`

&emsp;element 对象有一个指向它的父级 element 的引用，在 mount 时会给 `_parent` 赋值，然后 ComponentElement 有 `Element? _child;`（StatelessElement/StatefulElement/ProxyElement 都是 ComponentElement 子类），SingleChildRenderObjectElement 有 `Element? _child;`，MultiChildRenderObjectElement 有 `late List<Element> _children;`，现在可以看出来了吧，element tree 其实是一个双向链表结构。 

```dart
  Element? _parent;
```

## `_notificationTree`

## operator ==

&emsp;用于比较两个 Element 的相等性，看代码的话，它加了 @nonVirtual 注解，表示所有的 Element 子类都不能重写此 operator ==，它们的对象比较时也都使用 Element 的 ==，然后是比较方式，是使用全局的 identical 函数，需要完全保证两个 Element 对象是同一个对象。

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

&emsp;同时并非所有的 element 对象的 slot 都有值，貌似只有 MultiChildRenderObjectElement 对象的 children 字段中的 element 对象才需要 slot 值。它表示此 element 节点在这个 MultiChildRenderObjectElement 父级节点中的位置。

```dart
  Object? get slot => _slot;
  Object? _slot;
```

&emsp;这里要先注意一下的点，slot 是一个 Object 类型，针对不同的 widget 它有不同的类型，不像是 iOS 中所有的 frame 都是 CGRect 一个类型的。

## depth

&emsp;depth 是一个简单的 int 值，表示 element 在 element tree 中的深度值，如果存在父级，则保证整数大于父级，树的根节点必须具有大于 0 的深度。

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

&emsp;mounted 已挂载标识，如果 Element 的 widget 字段不为 null，就表示已经挂载。

```dart
  @override
  Widget get widget => _widget!;
  Widget? _widget;
  
  @override
  bool get mounted => _widget != null;
```

## owner

&emsp;owner 是一个在 Element tree 上传递的全局对象，并且是管理 element 生命周期的对象。后面我们会详细看这个 BuildOwner 类型。

&mesp;在整个 element tree 上每个 element 节点都会持有它。

```dart
  @override
  BuildOwner? get owner => _owner;
  BuildOwner? _owner;
```

## buildScope

&emsp;当这个 Element 已经被 mounted 时，通常访问这个 getter 才是安全的。默认实现会返回父 Element 的 buildScope，因为在大多数情况下，一个 Element 一旦其祖先不再是脏的就可以准备重建。一个值得注意的例外是 LayoutBuilder 的子孙节点，在接收到约束之前不应该进行重建。LayoutBuilder 的 Element 重写了 buildScope 以确保其所有子孙节点在接收到约束之前都不会进行重建。如果你选择重写这个 getter 来建立自己的 BuildScope，需要在适当的时候手动调用 BuildOwner.buildScope，并传入你的 BuildScope 的根 Element，因为 Flutter framework 不会尝试注册或管理自定义的 BuildScope。如果你重写这个 getter，请始终返回相同的 BuildScope 实例。不支持在运行时更改此 getter 返回的值。updateChild 方法忽略 buildScope：如果父 Element 在一个具有不同 BuildScope 的子 Element 上调用 updateChild]，子 Element 可能仍然会重建。

&emsp;有点晕，后面再来看。

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
        return current.renderObject;
      } else {
        // 这里则是遍历往当前 element 的 child 中去找
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

## `_lifecycleState`

&emsp;这被用来验证 element 对象以有秩序的方式在生命周期中移动。

&emsp;表示 element 目前处于生命周期的哪个状态，值是 `_ElementLifecycle` 枚举。

```dart
enum _ElementLifecycle {
  initial,
  active,
  inactive,
  defunct,
}

  _ElementLifecycle _lifecycleState = _ElementLifecycle.initial;
```

## visitChildren

&emsp;这是一个比较特殊的函数，它的参数 visitor 是一个参数是 Element，返回值是 void 的函数：`typedef ElementVisitor = void Function(Element element);`。

&emsp;而且这个函数比较特殊，在 Element 中它是一个空实现，而在有 child 字段的 Element 子类中要重写它，比如：ComponentElement 重写了 visitChildren 函数，当它的 `_child` 不为 null 时，调用 `vistor(_child)`。然后 SingleChildRenderObjectElement 和 ComponentElement 一样的重写，MultiChildRenderObjectElement 则是对自己 `_children` 中的每个 Element child 调用 `vistor(child)`。

&emesp;此函数可以直白的理解为：访问 element 的 child。分别以 element 的 child 们为入参调用 visitor 函数。

```dart
  void visitChildren(ElementVisitor visitor) { }
```

## visitChildElements

&emsp;仅仅是为了实现 BuildContext 抽象类的 visitChildElements 抽象函数，内部仅仅是封装的 visitChildren 函数调用。

```dart
  @override
  void visitChildElements(ElementVisitor visitor) {
    visitChildren(visitor);
  }
```

&emsp;下面我们进入 Element 的核心函数之一：updateChild，超级重要。

## updateChild

&emsp;在 updateChild 函数前有三个很重要的注解，它们都是想优化 updateChild 函数的调用为内联调用。不过此内联优化的场景并不是针对的 Flutter 生成环境。

+ @pragma('dart2js:tryInline') 向 dart2js(Dart-to-JavaScript compiler) 建议在优化期间应优先考虑内联 updateChild 函数。
+ @pragma('vm:prefer-inline') 向 Dart VM 建议在优化期间应优先考虑内联 updateChild 函数。
+ @pragma('wasm:prefer-inline') 向 WebAssembly 建议在优化期间应优先考虑内联 updateChild 函数。

&emsp;貌似有点可惜，此内联优化并不是针对 native APP 生产环境。

&emsp;updateChild 函数的作用：使用给定的新配置更新给定的子 element（也有可能是新建或者移除 Element）。该方法是 widget system 的核心。每当我们根据更新的配置要添加、更新或删除子 element 时，就会调用它。

&emsp;newSlot 参数指定了 element slot 的新值。

&emsp;如果子 element 为 null（即 Element? child 入参为 null），而 newWidget 不为 null（即 Widget? newWidget 入参不为 null），则表示我们需要创建一个配置是 newWidget 的新 element 对象。

&emsp;如果 newWidget 为 null（即 Widget? newWidget 入参不为 null），而子 element 不为 null（即 Element? child 入参为 null），则需要将这现有的 child（element 节点）移除，因为它不再具有配置了。

&emsp;如果两者均不为 null，则需要将子 element 的配置更新为 newWidget 给出的新配置。如果 newWidget 可以提供给现有子 element（由 Widget.canUpdate 决定），则提供。否则，旧子 element 需要被处理并为新配置（newWidget）创建新的 child Element 对象。

&emsp;如果两者均为 null，则我们既没有子 element 了，也不需要新建子 element 了，因此我们不做任何操作。

&emsp;updateChild 方法返回新的 child element 对象（如果必须创建一个），或者传入的 child element 对象（如果只需要更新子元素），或 null（如果移除了子 element 且没有需要新建的 element）。

&emsp;下表总结了上述内容:

  |                     | **newWidget == null**  | **newWidget != null**   |
  | :-----------------: | :--------------------- | :---------------------- |
  |  **child == null**  |  Returns null.         |  Returns new Element. |
  |  **child != null**  |  Old child is removed, returns null. | Old child updated if possible, returns child or new Element. |

&emsp;只有在 newWidget 不为 null 时才会使用 newSlot 参数。如果 child 为 null（或者旧 child 无法更新），那么通过 inflateWidget 将 newSlot 分配给为 child 创建的新 Element 对象。如果 child 不为 null（并且旧 child 可以更新），那么通过 updateSlotForChild 给予 newSlot 来更新其 slot，以防它在上次构建后发生移动。

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
    
        // deactivateChild 函数所有的 Element 类仅用这一个，子类都没有重写。
        // deactivateChild 把这个指定的 Element child 对象失活，主要做了三件事：
        // 1. 把 child 的 _parent 置为 null。
        // 2. 把 child 的 render object 从 render tree 上分离。
        // 3. 把 child 放入 owner!._inactiveElements 这个全局的 非活动 Element Set 中，等待被复用。
        deactivateChild(child);
      }
      
      // 旧 element 已经被移除啦，updateChild 返回 null 即可
      return null;
    }

    // 临时变量，主要用来记录返回值，针对下面这些 newWidget 不为 null 的情况，
    // updateChild 函数要么返回新建的 element 对象，要么返回得到更新的 child 入参。
    final Element newChild;
    
    if (child != null) {
      // 2️⃣：针对 child 不为 null 的情况，此 if 内尽量尝试对 child 进行更新，实在不行时才进行新建。
      
      bool hasSameSuperclass = true;
      // 原代码这里有一段 hasSameSuperclass 的解释，
      // 主要为了防止热重载时，StatefulWidget 和 StatelessWidget 的相互替换导致问题，
      // 而且断言 assert 代码在生产环境时会被移除，为了减少理解负担，
      // 我们就直接给它删掉，把 hasSameSuperclass 理解为常量 true 即可。 
      
      if (hasSameSuperclass && child.widget == newWidget) {
        // 2️⃣1️⃣：我们之前学习 const 时已经看过 flutter 对它的优化了，
        // 如果 newWidget 是被 const 构造函数构建，并且加了 const 修饰那么新旧 widget 就是同一个对象，
        // 这里如果 newSlot 与 child element 对象的 slot 不同的话只需更新 child 的 slot 即可。
        if (child.slot != newSlot) {
          updateSlotForChild(child, newSlot);
        }
        
        // 记录下，还是 child Element 对象返回
        newChild = child;
      } else if (hasSameSuperclass && Widget.canUpdate(child.widget, newWidget)) {
        // 2️⃣2️⃣：如果 newWidget 和旧 widget：runtimeType 和 key 一样的话，那么可以直接更新 child 即可，不需新建。
        
        // 如需要更新 slot，则更新 slot
        if (child.slot != newSlot) {
          updateSlotForChild(child, newSlot);
        }
        
        // 此处原有一段 FlutterTimeline 的代码，生产环境无用，暂时删掉不看。
        
        // newWidget 仅需更新 child 即可，更新完 child 后，
        // 会强制进行 child 子级 element 的重建：rebuild(force: true)，
        // 即会对 child 的 Element 子树进行完整的重建工作，
        // update 函数链路很长的，并非只是简单更新下 child 这独一个 Element 节点的内容而已。
        child.update(newWidget);

        newChild = child;
      } else {
        // 2️⃣3️⃣：针对 child 不能用 newWidget 进行更新的情况，只能新建 element 了，
        // 要把旧的 element 给它失活了。 
        deactivateChild(child);
        
        // 使用 newWidget 和 newSlot 开始构建 child 原有父级 element 下的完整的 element 子树，
        // 可以理解为自 child 的父级 element 开始，开始构建它下面的整个 element 子树。
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

&emsp;我们看到其中一种最省事的情况：

&emsp;新旧 Widget 相等时，widget 不进行任何操作（此 newWidget 的 build 也不会被执行），没有任何开销，当然如果新旧 slot 不同的话，会更新下 element 的 slot。而这个最省事的情况就是对应了官方推荐的优化技巧：提取封装 widget 子类，声明 const 构造函数，使用时添加 const 修饰使用常量表达式。

&emsp;child element 对象可以被复用时，会使用 newWidget 进行更新，然后对其下级 Element 子树进行重建，有可能可以完整的复用整个子树，也有可能只能复用到某个节点后需要新建，视 widget 子级来决定。当不可被复用时，则是新建接下来的整个 element 子树。

&emsp;这里注意 update child 的更新并不是只针对当前这一个 element 节点的，它是沿着当前这个 element 节点链一直往下子级进行的，直到最末端没有新 widget 对象了为止。

&emsp;还有我们应该也注意到了，以上 update child 整个流程都是针对一根节点 **链** 进行的，过程中还没有怎么讲过 MultiChildRenderObjectElement 和 MultichildRenderObjectWidget，它们可都是一个点有一组子级的：`final List<Widget> children` / `late List<Element> _children`，那么当涉及 MultiChildRenderObjectWidget 的一组子级 widget 时，怎么进行更新和新建呢？那么接下来看它涉及的最核心的 updateChildren 函数。 

&emsp;还有一个特别重要的点要注意到，首先看这两个函数的定义：

+ `Element? updateChild(Element? child, Widget? newWidget, Object? newSlot)`
+ `List<Element> updateChildren(List<Element> oldChildren, List<Widget> newWidgets, { Set<Element>? forgottenChildren, List<Object?>? slots })`  

&emsp;观察一下会发现：updateChildren 的 `List<Element> oldChildren` 是没有 ？的，它不是可选的，它每次都是有值的。这里最重要的一个点：updateChild 是包括：新建/更新/移除 element 子级三种情况的，特别是 `Element? child` 为 null 时，会进行新的 element 子树的构建，对于新建 element 子树这一点，在 updateChildren 函数调用中是不存在的，当它被调用时必是对现有的 `List<Element> oldChildren` 中的一组 element 节点进行更新操作（这个更新是针对单个 element 节点进行，直接调用的 updateChild 函数）。

&emsp;所以 updateChildren 函数的出发点：就是对现有的 MultiChildRenderObjectElement 节点的一组现有的子级 element 节点进行更新操作。然后可以大胆的进行一个全局搜索，可以发现 updateChildren 函数全局仅有的一处调用：在 MultiChildRenderObjectElement 的 update 函数中的调用。

&emsp;OK，接下来正式开始看 updateChildren 的内容吧，它是 Element 类中最复杂的一个函数，翻过这座大山⛰️以后，其它 Element 类的内容我们就可以一马平川啦！⛽️

## updateChildren

```dart
@protected
List<Element> updateChildren(
    List<Element> oldChildren,
    List<Widget> newWidgets,
    {Set<Element>? forgottenChildren,
        List<Object?>? slots}
)
```

&emsp;updateChildren 函数是针对 MultiChildRenderObjectElement 对象的更新函数。updateChildren 使用新的 widget 更新此 element 的现有的 children(子级 element 列表)。

&emsp;尝试使用给定的 newWidgets（一组不同类型的 Widget 对象列表）更新给定的 oldChildren（一组不同类型的 Element 对象列表，每个 Element 对象都是一个 Element 子树），根据需要删除过时的 element 并引入新 element，然后返回新的 element 列表。

&emsp;在此函数执行期间，不得修改 oldChildren 列表（它内部是旧的 element 对象）。如果调用方希望在此函数在栈上时从 oldChildren 中移除元素，调用方可以提供一个 forgottenChildren 参数。每当此函数从 oldChildren 中读取 element 对象时，此函数首先检查该 element 对象是否在 forgottenChildren 集合中，如果是，则该函数会假设该 element 对象不在 oldChildren 中，直接 break 跳过。

&emsp;上面一段描述有点晕，我们来分析一下，通过代码可以看出 oldChildren 中的 element 对象，如果也出现在 forgottenChildren 集合中的话，则会直接跳过对 oldChildren 的遍历，通过函数内部的 replaceWithNullIfForgotten 函数进行判断。同时还有一个操作，从 newWidgets 和 oldChildren 的同一个下标位分别取得 widget 对象和 element 对象，如果这个 widget 对象不可以更新 element 对象时（通过 Widget.canUpdate 判断返回 false）也会直接跳过对 oldChildren 的遍历。）

&emsp;此函数是对 updateChild 的方便包装，后者会更新每个单独的 element 对象。如果 slots 非空，则调用 updateChild 函数时的 newSlot 参数的值将从该 slots 列表中按照当前处理的 widget 在 newWidget 列表中对应的索引检索（newWidget 和 slot 必须具有相同的长度）。如果 slots 为空的话，则将新建一个 `IndexedSlot<Element>` 对象作为 newSlot 参数的值。在这种情况下，IndexedSlot 对象的 index 字段设置为当前处理的 widget 在 newWidget 列表中的索引值（index），IndexedSlot 对象的 value 字段设置为该列表中前一个 widget 对象的 Element 对象（若是第一个元素的话，则为 null）。（这里的意思就是给 newChildren 列表中的每个 element 对象配置一个 slot 值，这个 slot 值是 IndexedSlot 类型的，而这个 IndexedSlot 类其实也是蛮简单的，它共有两个字段，一个字段是 index 可标识每个 element 元素在 newChildren 列表中的索引，还有一个 value 字段，可标识该 element 对象前面的那个 element 对象。）

 &emsp;当一个 element 对象的 slot 发生变化时，其关联的 renderObject 需要移动到其父级的 children 中的新位置。如果该 RenderObject 将其子元素组织成链表（如 ContainerRenderObjectMixin 所做的），则可以通过在与 slot 对象中的 IndexedSlot.value 相关的 Element 的 RenderObject 之后重新将子 RenderObject 插入到列表中来实现这一点。(关于 RenderObject 的内容后续再进行补充。)
 
 &emsp;下面我们先分心一下，看一下 IndexedSlot 这个泛型类的内容。
 
### IndexedSlot
 
&emsp;`IndexedSlot<T extends Element?>` 用于表示 MultiChildRenderObjectElement 子级（children List 中的 Element）的 Element 的 slot 值。

&emsp;对于 MultiChildRenderObjectElement 的 slot 来说，它包括一个索引，用来标识占据该 slot 的子节点在 MultiChildRenderObjectElement 的子节点列表中的位置，以及一个任意值，可以进一步定义占据该 slot 的子节点在其父节点的子节点列表中的位置（如上面在 updateChildren 中，把此任意值设置为了此子节点前面那个节点）。

&emsp;直接看 IndexedSlot 类的代码，它还是特别简单的，有一个 int value 字段和一个泛型 T value 字段，然后重写了 operator == 和 hashCode，由 index 和 value 共同决定 IndexedSlot 对象的判等和生成哈希码。其它就没有任何内容了。 

```dart
@immutable
class IndexedSlot<T extends Element?> {
  /// Creates an [IndexedSlot] with the provided [index] and slot [value].
  const IndexedSlot(this.index, this.value);

  // 用于定义占用此 slot 的子节点在其父节点的子列表中的位置的信息
  final T value;

  // 父节点的子节点列表中此 slot 的索引
  final int index;

  @override
  bool operator ==(Object other) {
    if (other.runtimeType != runtimeType) {
      return false;
    }
    return other is IndexedSlot
        && index == other.index
        && value == other.value;
  }

  @override
  int get hashCode => Object.hash(index, value);
}
```
&emsp;然后下面还有一段较复杂的官方注释，讲解 Flutter 中关于管理子元素顺序的问题，一起来看一下：

&emsp;在 MultiChildRenderObjectElement 中 子元素的顺序不仅取决于前一个兄弟节点，还取决于它在列表中的位置。当子元素在列表中的位置发生变化时，即使它的前一个兄弟节点没有变化，它的渲染位置也可能需要移动。举个例子，原来顺序是 `[e1, e2, e3, e4]`，变为 `[e1, e3, e4, e2]` 时，元素 e4 的前一个兄弟依然是 e3，但是由于在列表中的位置变了，它的渲染位置需要移动到 e2 之前。为了触发这种移动，当子元素在父节点的子元素列表中的位置发生变化时，需要为元素分配一个新的位置，即 `IndexedSlot<Element>` 类型的对象。这样可以确保父节点的渲染对象知道子元素需要移动到链表的哪个位置，同时确定它的新的前一个兄弟节点。

```dart
  @protected
  List<Element> updateChildren(List<Element> oldChildren, List<Widget> newWidgets, { Set<Element>? forgottenChildren, List<Object?>? slots }) {
  
    // 嵌套函数，如果 child 入参包含在 forgottenChildren 集合中，则返回 null
    Element? replaceWithNullIfForgotten(Element child) {
      return forgottenChildren != null && forgottenChildren.contains(child) ? null : child;
    }
    
    // 如果 slots 入参不为 null，则返回 newChildIndex 索引处的 slot，否则创建一个 IndexedSlot 对象返回，
    // IndexedSlot 类也贼简单，仅仅是有两个 final 字段的类：final T value 和 final int index，
    // 把两个入参 newChildIndex 和 previousChild 绑定在一起
    Object? slotFor(int newChildIndex, Element? previousChild) {
      return slots != null
          ? slots[newChildIndex]
          : IndexedSlot<Element?>(newChildIndex, previousChild);
    }
    
    // 上面是两个嵌套函数的定义，下面开始看 updateChildren 函数的主要内容。
    
    // 这段代码尝试将新的 widget 列表（newWidgets）与旧的 element 列表（oldChildren）进行差异比较，并生成一个新的 element 列表作为该 Element 的新子元素列表(MultiChildRenderObjectElement 的 `_children` 字段)。调用此方法的对象应相应更也新其 render object。
    
    // 根据参数值的情况，它尝试优化的情况包括：
    // - 旧列表为空（oldChildren 为空）
    // - 列表相同（newWidgets 中 widget 与 oldChildren 中 element 匹配，可以用 widget 对象更新 element 对象）
    // - 在列表中仅在一个地方插入或移除一个或多个 widget。（）
    
    // 如果具有 key 的 widget 在两个列表中都存在，它们将会被同步。没有 key 的 widget 可能会被同步，但不能保证。
    // 一般的方法是将整个新列表向后同步，步骤如下：
    // 1. 从顶部开始遍历列表，同步节点，直到没有匹配的节点为止。
    // 2. 从底部开始遍历列表，不同步节点，直到没有匹配的节点为止。我们将在最后同步这些节点。我们现在不同步它们，因为我们希望按顺序从头到尾同步所有节点。
    
    // 到目前为止，我们已经将旧列表和新列表缩小到节点不再匹配的地步。
    
    // 3. 遍历旧列表的缩小部分，以获取键的列表，并将空值与非键控项同步。
    // 4. 将新列表的缩小部分向前移动：
    // * 使用空值同步非键控项目
    // * 如果存在源，则将带有密钥的项目与源同步，否则将其与 null 同步。
    // 5. 再次遍历列表底部，同步节点。
    // 6. 将空值与仍然挂载的键列表中的任何项目同步。

    int newChildrenTop = 0;
    int oldChildrenTop = 0;
    int newChildrenBottom = newWidgets.length - 1;
    int oldChildrenBottom = oldChildren.length - 1;

    final List<Element> newChildren = List<Element>.filled(newWidgets.length, _NullElement.instance);

    Element? previousChild;
    
    // Update the top of the list.
    while ((oldChildrenTop <= oldChildrenBottom) && (newChildrenTop <= newChildrenBottom)) {
      final Element? oldChild = replaceWithNullIfForgotten(oldChildren[oldChildrenTop]);
      final Widget newWidget = newWidgets[newChildrenTop];
      
      assert(oldChild == null || oldChild._lifecycleState == _ElementLifecycle.active);
      
      if (oldChild == null || !Widget.canUpdate(oldChild.widget, newWidget)) {
        break;
      }
      
      final Element newChild = updateChild(oldChild, newWidget, slotFor(newChildrenTop, previousChild))!;
      
      assert(newChild._lifecycleState == _ElementLifecycle.active);
      
      newChildren[newChildrenTop] = newChild;
      previousChild = newChild;
      
      newChildrenTop += 1;
      oldChildrenTop += 1;
    }

    // Scan the bottom of the list.
    while ((oldChildrenTop <= oldChildrenBottom) && (newChildrenTop <= newChildrenBottom)) {
      final Element? oldChild = replaceWithNullIfForgotten(oldChildren[oldChildrenBottom]);
      final Widget newWidget = newWidgets[newChildrenBottom];
      
      assert(oldChild == null || oldChild._lifecycleState == _ElementLifecycle.active);
      
      if (oldChild == null || !Widget.canUpdate(oldChild.widget, newWidget)) {
        break;
      }
      
      oldChildrenBottom -= 1;
      newChildrenBottom -= 1;
    }

    // Scan the old children in the middle of the list.
    final bool haveOldChildren = oldChildrenTop <= oldChildrenBottom;
    Map<Key, Element>? oldKeyedChildren;
    if (haveOldChildren) {
      oldKeyedChildren = <Key, Element>{};
      while (oldChildrenTop <= oldChildrenBottom) {
        final Element? oldChild = replaceWithNullIfForgotten(oldChildren[oldChildrenTop]);
        assert(oldChild == null || oldChild._lifecycleState == _ElementLifecycle.active);
        if (oldChild != null) {
          if (oldChild.widget.key != null) {
            oldKeyedChildren[oldChild.widget.key!] = oldChild;
          } else {
            deactivateChild(oldChild);
          }
        }
        oldChildrenTop += 1;
      }
    }

    // Update the middle of the list.
    while (newChildrenTop <= newChildrenBottom) {
      Element? oldChild;
      final Widget newWidget = newWidgets[newChildrenTop];
      if (haveOldChildren) {
        final Key? key = newWidget.key;
        if (key != null) {
          oldChild = oldKeyedChildren![key];
          if (oldChild != null) {
            if (Widget.canUpdate(oldChild.widget, newWidget)) {
              // we found a match!
              // remove it from oldKeyedChildren so we don't unsync it later
              oldKeyedChildren.remove(key);
            } else {
              // Not a match, let's pretend we didn't see it for now.
              oldChild = null;
            }
          }
        }
      }
      assert(oldChild == null || Widget.canUpdate(oldChild.widget, newWidget));
      final Element newChild = updateChild(oldChild, newWidget, slotFor(newChildrenTop, previousChild))!;
      assert(newChild._lifecycleState == _ElementLifecycle.active);
      assert(oldChild == newChild || oldChild == null || oldChild._lifecycleState != _ElementLifecycle.active);
      newChildren[newChildrenTop] = newChild;
      previousChild = newChild;
      newChildrenTop += 1;
    }
    
    // We've scanned the whole list.
    assert(oldChildrenTop == oldChildrenBottom + 1);
    assert(newChildrenTop == newChildrenBottom + 1);
    assert(newWidgets.length - newChildrenTop == oldChildren.length - oldChildrenTop);
    newChildrenBottom = newWidgets.length - 1;
    oldChildrenBottom = oldChildren.length - 1;
    
    // Update the bottom of the list.
    while ((oldChildrenTop <= oldChildrenBottom) && (newChildrenTop <= newChildrenBottom)) {
      final Element oldChild = oldChildren[oldChildrenTop];
      assert(replaceWithNullIfForgotten(oldChild) != null);
      assert(oldChild._lifecycleState == _ElementLifecycle.active);
      final Widget newWidget = newWidgets[newChildrenTop];
      assert(Widget.canUpdate(oldChild.widget, newWidget));
      final Element newChild = updateChild(oldChild, newWidget, slotFor(newChildrenTop, previousChild))!;
      assert(newChild._lifecycleState == _ElementLifecycle.active);
      assert(oldChild == newChild || oldChild._lifecycleState != _ElementLifecycle.active);
      newChildren[newChildrenTop] = newChild;
      previousChild = newChild;
      newChildrenTop += 1;
      oldChildrenTop += 1;
    }
    
    // Clean up any of the remaining middle nodes from the old list.
    if (haveOldChildren && oldKeyedChildren!.isNotEmpty) {
      for (final Element oldChild in oldKeyedChildren.values) {
        if (forgottenChildren == null || !forgottenChildren.contains(oldChild)) {
          deactivateChild(oldChild);
        }
      }
    }
    assert(newChildren.every((Element element) => element is! _NullElement));
    return newChildren;
```

&emsp;先放放...

## 参考链接
**参考链接:🔗**
+ [Element class](https://api.flutter.dev/flutter/widgets/Element-class.html)
+ [kFlutterMemoryAllocationsEnabled top-level constant](https://api.flutter.dev/flutter/foundation/kFlutterMemoryAllocationsEnabled-constant.html)
+ [Flutter | Understanding the MemoryAllocations](https://medium.com/@maciejbrzezinski/flutter-what-is-memoryallocations-1ee2eb0a8670)
+ [Flutter | How to create a custom binding and inject your own BuildOwner](https://medium.com/@maciejbrzezinski/flutter-how-to-create-a-custom-binding-and-inject-your-own-buildowner-f60ef320537b)
+ [Support for WebAssembly (Wasm)](https://docs.flutter.dev/platform-integration/web/wasm)
