# Flutter 源码梳理系列（十六）：Element：unmount -> performRebuild

&emsp;前面梳理完了 Element.mount 到 Element.deactivate 的内容，收获颇多。OK，下面我们继续，一鼓作气把 Element 中的内容全部看完。

# unmount

&emsp;使当前 Element 对象从 "inactive" 生命周期状态过渡到 "defunct" 生命周期状态。

&emsp;当 framework 确定一个非活动 Element 将永远不会被重新激活时调用。在每帧结束时，framework 会对任何剩余的非活动 Element 对象调用 unmount，阻止非活动 Element 对象保持非活动状态超过一帧。

&emsp;在调用此方法之后，Element 对象将不会再次被添加到 Element Tree 中。

&emsp;此 Element 对象持有的任何资源应在此时释放。例如，RenderObjectElement.unmount 调用 RenderObject.dispose 并将其对 renderObject 的引用置为 null。

&emsp;此方法的实现应该以调用继承方法结束，如 super.unmount()。

&emsp;还记得 BuildOwner 学习时的 BuildOwner.finalizeTree 函数吗，即当前帧结束时，执行当前 BuildOwner 下的：`_inactiveElements._unmountAll`。

```dart
  @mustCallSuper
  void unmount() {
  
    // 当前 Element 对象的 widget 如果有 global key，则要从当前 BuildOwner 下的 _globalKeyRegistry 中移除：key -> element
    final Key? key = _widget?.key;
    if (key is GlobalKey) {
      owner!._unregisterGlobalKey(key, this);
    }
    
    // Release resources to reduce the severity of memory leaks caused by defunct, but accidentally retained Elements.
    
    // widget 置 null
    _widget = null;
    
    // _dependencies 里面是当前 element 对象所依赖的 InheritedElement 对象，把它置 null
    _dependencies = null;
    
    // 状态置为 defunct
    _lifecycleState = _ElementLifecycle.defunct;
  }
```

# findRenderObject

&emsp;该 Element 对象的当前 RenderObject。如果 element widget 是一个 RenderObjectWidget，则这是该 element 为自身创建的渲染对象。否则，这是第一个自己子节点 RenderObjectWidget 的 RenderObject。(如果当前 element 不是 RenderObjectElement 的话，会沿着 element tree 在自己的子树上找第一个 RenderObject)

&emsp;此方法只有在构建阶段完成后才会返回有效结果。因此，不能在 build 方法中调用它。应该只从互动事件处理程序（例如手势回调）或布局或绘制回调中调用它。如果 State.mounted 返回 false，则调用该方法也是无效的。(调用此函数时需要，当前 element 节点处于 active 状态)

&emsp;如果 render object 是 RenderBox（这是常见情况），则可以从 size getter 中获取 render object 的大小。这只在布局阶段之后才有效，因此应该只从绘制回调或互动事件处理程序（例如手势回调）中进行检查。

&emsp;有关帧不同阶段的详细信息，可参阅 WidgetsBinding.drawFrame 中的讨论。

&emsp;理论上，调用此方法的成本相对较高（树的深度为 O(N)），但实际上通常很便宜，因为树通常有许多 render object，因此到最近的 render object 的距离通常很短。

```dart
  @override
  RenderObject? findRenderObject() {
    return renderObject;
  }
```

&emsp;记住一点：如果自己没有 renderObject 的话，会继续往自己的子树上找，找到一个距离自己最近的。如果当前 element 对象不是 active 状态，则抛错。

# size

&emsp;如果 findRenderObject 返回的是 RenderBox，则返回它的 size 属性，否则返回 null。

&emsp;此 getter 仅在布局阶段完成后才会返回有效结果。因此，从 build 中调用此方法是无效的。应该在绘制回调或交互事件处理程序（例如手势回调）中调用它。

&emsp;如果 findRenderObject 实际返回的不是 RenderBox 的子类型的渲染对象（例如，RenderView），则此 getter 将在调试模式下引发异常，并在发布模式下返回 null。

&emsp;调用此 getter 理论上相对昂贵（在树的深度上为 O(N)），但在实践中通常很便宜，因为树通常具有许多渲染对象，因此距离最近的渲染对象通常很短。

```dart
  @override
  Size? get size {
    final RenderObject? renderObject = findRenderObject();
    
    if (renderObject is RenderBox) {
      return renderObject.size;
    }
    
    return null;
  }
```

# `_inheritedElements`

&emsp;记录从 Element Tree 根节点到当前 Element 节点的所有 InheritedElement 对象，起一个收集的作用，然后 Element Tree 上每个节点都会引用这个 PersistentHashMap，这样就方便了 InheritedElement 节点的子级找自己想要依赖的上级 InheritedElement 对象。

&emsp;保证了 O(1) 的时间复杂度就可以找到指定类型的 InheritedWidget 对象。

&emsp;由于 Element Tree 是一层一层构建的，所以轮到当前 Element 向上找指定类型的 InheritedElement 对象时，是以当前 Element 对象为起点开始，如果是自己的子级中有同类型的 InheritedElement 出现也不会影响当前的 Element 对象向上查找。 

```dart
  PersistentHashMap<Type, InheritedElement>? _inheritedElements;
```

# `_dependencies`

&emsp;记录下当前 Element 对象依赖了哪些 InheritedElement 对象（它们都是自己的父父级 element 节点），当它依赖的 InheritedElement 更新重建时所有依赖此 InheritedElement 对象的 Element 对象都会进行标记重建。

```dart
  Set<InheritedElement>? _dependencies;
```

# `_hadUnsatisfied`

&emsp;记录当前 Element 对象尝试找指定类型的 InheritedElement 时没有找到（有不满意的依赖关系）。

```dart
  bool _hadUnsatisfiedDependencies = false;
```

&emsp;下面是一组跟 InheritedWidget/InheritedElement 相关的函数，如果搞清楚了它们，你就搞清楚了，InheritedWidget 的依赖更新机制。

# doesDependOnInheritedElement

&emsp;如果之前使用入参 ancestor 调用过 dependOnInheritedElement 函数的话，会返回 true。（判断入参 ancestor 是否在自己的 `_dependencies` 集合中，自己依赖过哪些 InheritedElement 对象都会被记录在自己的 `_dependencies` 集合中。）

&emsp;因为如果之前 ancestor 调用过 dependOnInheritedElement 的话，它会被收集在 `_dependencies` 中。ancestor 是当前 element 节点的父父级的节点，都是 InheritedElement 对象，因为是自己的父父级别，所以可被称为：“祖先”。

```dart
  @protected
  bool doesDependOnInheritedElement(InheritedElement ancestor) =>
      _dependencies != null && _dependencies!.contains(ancestor);
```

# dependOnInheritedElement

&emsp;将此 Element 节点（构建上下文，之所以被称为 构建上下文，是因为在 Widget 那一层，开发者自己手写的代码层，Element 是以 BuildContext context 的形式传过来的，其实它就是 widget 对象对应的 element 对象）注册到祖先（祖先指自己的父父级 element 节点，因为 InheritedElement 是沿着 Element Tree 往上查找到的）中，以便当祖先的 Widget 更改时，将重新构建此 Element 节点。

&emsp;我们在通过 of 函数找到最近的 InheritedWidget 数据时，这个时候呢，也会把当前这个 Element 对象注册到 InheritedElement 的依赖者列表中（`_dependents`），那样，当 InheritedElement 更新时，也就可以通知所有的依赖者更新重建啦，InheritedElement 有一个依赖者列表（`_dependents`），直接标记列表中的 Element 对象重建即可。

&emsp;返回值是 InheritedElement 对应的 InheritedWidget 对象。

&emsp;通常不直接调用此方法。大多数应用程序应该使用 dependOnInheritedWidgetOfExactType，找到对应类型的 InheritedElement 后调用此方法。(即此方法呢，我们自己调用不到，它是给 dependOnInheritedWidgetOfExactType 函数调用的)

&emsp;有关何时可以调用 dependOnInheritedWidgetOfExactType 的所有资格也适用于此方法。（dependOnInheritedWidgetOfExactType 函数大家比较熟悉了，自定义 InheritedWidget 子类时，重写它的 of 函数时，内部要用这个 dependOnInheritedWidgetOfExactType 函数。）

&emsp;dependOnInheritedElement 函数内部做的事情呢，也特别清晰：

1. 找到当前 element 对象想要依赖的 InheritedElement 对象以后呢，就把它记录到自己的 `_dependencies` 字段中，记录下自己依赖了哪些 InheritedElement.
2. 而这个 InheritedElement 对象呢，也会把我们当前的 element 对象添加到自己的 `_dependents` 字段中，记录下哪些 element 对象依赖了自己，即记录下这些依赖者，方便后续更新重建。 

&emsp;然后其实还有个疑惑点：这个可 null 的 Object? aspect 参数是干啥用的，也暂时没见到哪里用到它了，后续我们研究一下。

```dart
  @override
  InheritedWidget dependOnInheritedElement(InheritedElement ancestor, { Object? aspect }) {
    // 找到当前 element 对象想要依赖的 InheritedElement 对象以后呢，就把它记录到自己的 _dependencies 字段中
    _dependencies ??= HashSet<InheritedElement>();
    _dependencies!.add(ancestor);
    
    // 同时呢，InheritedElement 对象也记录下这些依赖自己的 element 对象。
    ancestor.updateDependencies(this, aspect);
    
    // 然后返回 InheritedElement 对象的 widget
    return ancestor.widget as InheritedWidget;
  }
```

# dependOnInheritedWidgetOfExactType

&emsp;dependOnInheritedWidgetOfExactType 函数我们应该挺熟悉的，在我们自定义 InheritedWidget 子类时，重写它的 of 函数时，内部必要用这个函数。下面看一下它的文档：

&emsp;返回给定类型 T 的最近的 widget，并在其上创建一个依赖（即把当前的 element 对象作为依赖者被它记录下来），如果找不到合适的 widget 则返回 null。

&emsp;找到的 widget 将是一个具体的 InheritedWidget 子类，调用 dependOnInheritedWidgetOfExactType 会将此构建上下文（element 对象）注册到返回的 widget 上。当该 widget 改变（或者引入了新的该类型的 widget，或者该 widget 被移除），此构建上下文将重新构建（此 element 对象会被标记为脏并进行新建），以便从该 widget 获取新值。

&emsp;通常会隐式地从 of() 静态方法中调用，例如 Theme.of。

&emsp;不应该在 widget 构造函数或 State.initState 方法中调用此方法，因为如果继承的值发生更改，这些方法将不会再次被调用。为确保在继承值更改时 widget 正确更新自身，只应该从构建方法、布局和绘制回调中直接或间接地调用此方法，或者从 State.didChangeDependencies（在 State.initState 之后立即调用）中调用。

&emsp;不应该从 State.dispose 中调用此方法，因为在那个时候 Element Tree 不再稳定。要在该方法中引用祖先，请在 State.didChangeDependencies 中保存对祖先的引用。从 State.deactivate 中使用此方法是安全的，每当 widget 从树中移除时都会调用该方法。

&emsp;也可以从交互事件处理程序（例如手势回调）或计时器中调用此方法，以获取一个值一次，只要该值不会被缓存或后续重复使用。

&emsp;调用此方法的时间复杂度为 O(1)（常数因子较小），但将导致 widget 更频繁地重建。

&emsp;一旦一个 widget 通过调用此方法注册了对特定类型的依赖，它将在与该 widget 相关的更改发生时被重建，并且将调用 State.didChangeDependencies，直到该 widget 或其祖先下一次移动为止（例如，因为添加或移除了一个祖先）。

&emsp;这里也可以重温下此视频：[InheritedWidgets | Decoding Flutter](https://www.youtube.com/watch?v=og-vJqLzg2c)超级重要，务必观看，涉及在何时何处使用 dependOnInheritedWidgetOfExactType，以及 InheritedWidget 是如何让其依赖者进行重建的！

&emsp;当 T 是一个支持部分更新的 InheritedWidget 子类，比如 InheritedModel 时，aspect 参数才会被使用。它指定了此上下文依赖的继承 widget 的 "aspect"。

&emsp;原来 aspect 参数是给，如：InheritedModel 类使用的，后续我们研究一下。 

```dart
  @override
  T? dependOnInheritedWidgetOfExactType<T extends InheritedWidget>({Object? aspect}) {
    // _inheritedElements 字段内呢是根据 InheritedWidget 的类型来保存对应的 InheritedElement 对象呢，
    // 并且 _inheritedElements 在整个 Eelment tree 中传递，所以这里呢，O(1) 的时间复杂度，就可以找到对应的 InheritedElement 对象。
    final InheritedElement? ancestor = _inheritedElements == null ? null : _inheritedElements![T];
    
    // 如果找到了对应类型的 InheritedElement 对象就直接调用上面的 dependOnInheritedElement 函数就好啦！
    if (ancestor != null) {
      return dependOnInheritedElement(ancestor, aspect: aspect) as T;
    }
    
    _hadUnsatisfiedDependencies = true;
    return null;
  }
```

# getInheritedWidgetOfExactType

&emsp;对 getElementForInheritedWidgetOfExactType 的简单封装。

&emsp;返回给定的 InheritedWidget 子类 T 的最近的 widget，如果找不到适当的祖先，则返回 null。

&emsp;**这个方法不像更典型的 dependOnInheritedWidgetOfExactType 那样引入依赖关系，所以如果 InheritedWidget 发生变化，这个上下文不会重新构建。这个功能适用于那些不常见的情况，其中不希望有依赖关系。**

&emsp;这个方法不应该从 State.dispose 中调用，因为此时 Element Tree 已不稳定。要在该方法中引用祖先，请在 State.didChangeDependencies 中保存对祖先的引用。可以安全地从 State.deactivate 中使用这个方法，该方法在 widget 从树中移除时调用。

&emsp;还可以从交互事件处理程序（例如手势回调）或计时器中调用这个方法，以获取值一次，只要那个值不被缓存或以后重复使用。

&emsp;调用这个方法的时间复杂度是 O(1)，有一个小的常数因子。

```dart
  @override
  T? getInheritedWidgetOfExactType<T extends InheritedWidget>() {
    return getElementForInheritedWidgetOfExactType<T>()?.widget as T?;
  }
```

# getElementForInheritedWidgetOfExactType

&emsp;看代码仿佛是只做了 dependOnInheritedWidgetOfExactType 函数一半的事情，从 `_inheritedElements` 读取一下数据就返回了，不包括任何在依赖者和被依赖者中存储依赖关系。

&emsp;返回与给定的 InheritedWidget 子类 T 的最近的 widget 相对应的 element 对象，该类型必须是一个具体的 InheritedWidget 子类。如果找不到这样的 element，则返回 null。调用此方法的时间复杂度为 O(1)，且有一个很小的常量因子。（在 Element Tree 一层一层的构建过程中，已经把所有的 InheritedElement 存储在全局 inheritedElements 中了。）

&emsp;这个方法不会像 dependOnInheritedWidgetOfExactType 一样在依赖者和被依赖者上建立依赖关系。

&emsp;这个方法不应该从 State.dispose 中调用，因为此时 Element Tree 已不稳定。要从这个方法引用祖先，需要在 State.didChangeDependencies 中调用 dependOnInheritedWidgetOfExactType 来保存对祖先的引用。可以在 State.deactivate 中使用这个方法，因为每当 Widget 从树中移除时会调用 State.deactivate。

```dart
  @override
  InheritedElement? getElementForInheritedWidgetOfExactType<T extends InheritedWidget>() {
    final InheritedElement? ancestor = _inheritedElements == null ? null : _inheritedElements![T];
    
    return ancestor;
  }
```

# attachNotificationTree

&emsp;在 Element.mount 和 Element.activate 中被调用，将当前 Element 对象注册到 Notification Tree 中。

&emsp;仅暴露此方法是为了能够实现 NotifiableElementMixin。希望响应通知的 Element 子类应该使用混入方式来实现。

&emsp;另请参见：NotificationListener，一个允许监听通知的小部件。后续我们进入深入研究。（前面已经全面解析过 NotificationListener 的工作原理，可以翻回去看看。）

&emsp;非 NotifiableElementMixin 节点时，直接把父级的 `_notificationTree` 字段赋值给自己，当遇到 NotifiableElementMixin 节点时，会以自己和父级构建一个新的 Notification Tree 节点。

```dart
  @protected
  void attachNotificationTree() {
    _notificationTree = _parent?._notificationTree;
  }
```

# `_updateInheritance`

&emsp;在 Element.mount 和 Element.activate 中都会调用此函数。

&emsp;在整个 element tree 中，每个 element 节点都引用同一个：`_inheritedElements`。同时还有一个特别重要的点：InheritedElement 重写了此函数，如果是 InheritedElement 对象调用 `_updateInheritance` 的话，是把自己添加到 `_parent?._inheritedElements` 中并赋值给自己，如果是其它类型的 Element 对象的话，则只是把 `_parent?._inheritedElements` 赋值给自己。

```dart
  void _updateInheritance() {
    assert(_lifecycleState == _ElementLifecycle.active);
    _inheritedElements = _parent?._inheritedElements;
  }
```

&emsp;OK，下面是一组跟 "Ancestor" 相关的函数，首先我们可以先对 Ancestor 词有一个心理建设，它代表的意思其实很简单：就是当前 element 节点的父级或父父级，我们可以沿着 `_parent` 往链表上级走，也就是沿着 element 的双向链表往上走，往上的那些节点，便可以被称为 "祖先"。

&emsp;然后下面一组函数便是找祖先：找指定类型的 Widget、找指定类型的 State、找根 State、找指定类型的 RenderObject。经历了前面 `_inheritedElements`、`_notificationTree` 的结构，虽然介绍它们时会说它们是向上沿着 Element 链查找的，但其实它们的结构都是精心设计过的，时间复杂度都很低，`_inheritedElements` 更是降到了 O(1)，而 `_notificationTree` 也是整个链上有几个 NotifiableElementMixin，时间复杂度就是 O(几)。而下面的一组查祖先的函数，则不是了哦，它们就是真的沿着 parent Element 链往上查的，最差的时间复杂度是：O(n)。 
 
# findAncestorWidgetOfExactType

&emsp;返回给定类型 T 的最近的祖先 Widget，该类型必须是一个具体的 Widget 子类的类型。代码也超级简单，沿着自己的 `_parent` 往上查找即可。

&emsp;一般来说，dependOnInheritedWidgetOfExactType 更有用，因为 InheritedWidget 在更改时会触发依赖者的重建。当用于交互事件处理程序（例如手势回调）或执行一次性任务（例如断言你是否具有特定类型的 widget 作为祖先）时，此方法是合适的。

&emsp;Widget 的 build 方法的返回值不应该依赖于此方法返回的值，因为如果此方法的返回值发生更改，那么构建上下文(BuildContext context)将不会重新构建。这可能导致 build 方法中使用的数据发生更改，但 widget 却没有重新构建(rebuild)。

&emsp;调用此方法比较昂贵（树深度为 O(N)）。只有在从此 Widget 到所需祖先之间的距离已知为较小且有界时才调用此方法。(毕竟它是沿着 element 链往上一个一个查的，复杂页面的 element 的 depth 都是很大的。)

&emsp;在 State.deactivate 或 State.dispose 中不应调用此方法，因为在此时 element tree 不再稳定。要在这些方法中引用祖先，请通过在 State.didChangeDependencies 中调用 findAncestorWidgetOfExactType 来保存对祖先的引用。

&emsp;如果在此上下文的祖先中没有出现请求类型的 widget，则返回 null。

```dart
  @override
  T? findAncestorWidgetOfExactType<T extends Widget>() {
    Element? ancestor = _parent;
    
    // 如果 runtimeType 类型不匹配，则沿着 _parent 链继续往上找，
    // 直到找到 ancestor 为 null 或 ancestor.widget.runtimeType == T 为止。
    while (ancestor != null && ancestor.widget.runtimeType != T) {
      ancestor = ancestor._parent;
    }
    
    return ancestor?.widget as T?;
  }
```

# findAncestorStateOfType

&emsp;同上，但是这一次是查找指定类型的 State。返回给定类型为 T 实例的最近祖先 StatefulWidget 的 State 对象。

&emsp;不应该在 build 方法中使用该方法，因为如果此方法返回的值发生更改，则构建上下文(BuildContext context)不会重新构建。通常情况下，对于这种情况，更适合使用 dependOnInheritedWidgetOfExactType。该方法对于以一次性方式更改祖先 widget 的状态很有用，例如，使祖先滚动列表滚动到此构建上下文的 widget 中，或者响应用户交互移动焦点。

&emsp;一般来说，考虑使用触发祖先中状态更改的回调，而不是使用本方法隐含的命令式风格。这通常会导致更易维护和可重用的代码，因为它将 widget 解耦。

&emsp;调用此方法比较昂贵（树深度为 O(N)）。只有在从此 Widget 到所需祖先之间的距离已知为较小且有界时才调用此方法。(毕竟它是沿着 element 链往上一个一个查的，复杂页面的 element 的 depth 都是很大的。)

&emsp;在 State.deactivate 或 State.dispose 中不应调用此方法，因为在此时 element tree 不再稳定。要在这些方法中引用祖先，请通过在 State.didChangeDependencies 中调用 findAncestorStateOfType 保存对祖先的引用。

```dart
  @override
  T? findAncestorStateOfType<T extends State<StatefulWidget>>() {
    Element? ancestor = _parent;
    
    // 沿着 _parent 找 StatefulElement 对象和它的 state 字段是否是要找的类型
    while (ancestor != null) {
      if (ancestor is StatefulElement && ancestor.state is T) {
        break;
      }
      
      ancestor = ancestor._parent;
    }
    
    // 找到对应的 StatefulElement 节点了，返回它的 state 字段即可
    final StatefulElement? statefulAncestor = ancestor as StatefulElement?;
    
    return statefulAncestor?.state as T?;
  }
```

# findRootAncestorStateOfType

&emsp;返回在给定类型 T 的实例中，是 StatefulWidget 的最远祖先的 State 对象。

&emsp;与 findAncestorStateOfType 的功能相同，但会继续访问后续祖先，直到没有剩余类型为 T 实例的元素。然后返回找到的最后一个。

&emsp;该操作的时间复杂度也是 O(N)，N 是整个 widget 树，而不是子树。它会一直遍历到当前 Element 的根节点才会结束。

```dart
  @override
  T? findRootAncestorStateOfType<T extends State<StatefulWidget>>() {
    Element? ancestor = _parent;
    StatefulElement? statefulAncestor;
    
    // 和 findAncestorStateOfType 的不同之处，一直沿着 _parent 往上，直到为 null 为止，
    // 用 statefulAncestor 变量记录下这一路上最后一个 State 类型满足要求的 StatefulElement 节点。
    while (ancestor != null) {
      
      // 遇到符合类型要求的就记录下，直到最后一个
      if (ancestor is StatefulElement && ancestor.state is T) {
        statefulAncestor = ancestor;
      }
      
      // 更新上一层继续
      ancestor = ancestor._parent;
    }
    
    return statefulAncestor?.state as T?;
  }
```

# findAncestorRenderObjectOfType

&emsp;返回给定类型 T 的最近祖先 RenderObjectWidget widget 的 RenderObject 对象。

&emsp;在构建方法中不应该使用此方法，因为如果此方法返回的值发生更改，构建上下文将不会重新构建。通常情况下，在这种情况下 dependOnInheritedWidgetOfExactType 更为合适。此方法仅在某些特殊情况下有用，其中一个 widget 需要使其祖先改变其布局或绘制行为。例如，Material 使用此方法使 InkWell widget 可以触发 Material 的实际渲染对象上的涟漪效果。

&emsp;调用此方法比较昂贵（树深度为 O(N)）。只有在从此 Widget 到所需祖先之间的距离已知为较小且有界时才调用此方法。(毕竟它是沿着 element 链往上一个一个查的，复杂页面的 element 的 depth 都是很大的。)

&emsp;不应该在 State.deactivate 或 State.dispose 中调用此方法，因为在那个时候 element tree 已不再稳定。要从这些方法中引用祖先，请通过在 State.didChangeDependencies 中调用 findAncestorRenderObjectOfType 来保存对祖先的引用。

```dart
  @override
  T? findAncestorRenderObjectOfType<T extends RenderObject>() {
    Element? ancestor = _parent;
    
    // 同样沿着 _parent 往上找
    while (ancestor != null) {
    
      // 找到节点类型是 RenderObjectElement 并且 renderObject 是指定类型的 renderObject，返回即可
      if (ancestor is RenderObjectElement && ancestor.renderObject is T) {
        return ancestor.renderObject as T;
      }
      
      ancestor = ancestor._parent;
    }
    
    return null;
  }
```

# visitAncestorElements

&emsp;从此构建上下文的 widget 的父级开始逐级遍历祖先，对每个祖先调用参数。

&emsp;回调函数将获得对应祖先 widget 的 Element 对象的引用。当遍历到根 widget 或者回调函数返回 false 时，遍历将停止。回调函数不能返回 null。

&emsp;这对于检查 widget 树非常有用。

&emsp;调用此方法相对较昂贵（在树的深度为 N 时为 O(N)）。

&emsp;不应该在 State.deactivate 或 State.dispose 中调用此方法，因为在那时 Element Tree 已不再稳定。要在这些方法中引用祖先，可以通过在 State.didChangeDependencies 中调用 visitAncestorElements 保存对祖先的引用。

```dart
  typedef ConditionalElementVisitor = bool Function(Element element);

  @override
  void visitAncestorElements(ConditionalElementVisitor visitor) {
    Element? ancestor = _parent;
    
    // 沿着 _parent，依次 element 节点执行 visitor 函数
    while (ancestor != null && visitor(ancestor)) {
      ancestor = ancestor._parent;
    }
  }
```

&emsp;OK，上面一组跟 Ancestor 相关的函数就看完了，都很简单，就是沿着 parent 指针进行遍历。主要是在真实场景中使用它们时，注意时机，过早和过晚都不要。

&emsp;下面的 didChangeDependencies 函数则是重点，直接会告诉我们 InheritedElement 是如何让依赖者重建的，下面我们一起看一下。

# didChangeDependencies

&emsp;当此 Element 对象的一个依赖项（即此 element 对象依赖的 InheritedElement）发生变化时调用。

&emsp;dependOnInheritedWidgetOfExactType 将此 element 对象注册为依赖于给定类型的 InheritedElement 对象。当树中的此位置的该类型信息发生变化时（例如，因为 InheritedElement 更新为新的 InheritedWidget 且 InheritedWidget.updateShouldNotify 返回 true），framework 会调用此函数来通知此 element 对象发生变化。

&emsp;此函数最重要的调用位置在 InheritedElement 中，它会遍历所有依赖自己的依赖者的 didChangeDependencies 函数，告诉依赖者们依赖发生变化了，你们看着办吧！

&emsp;实现内容也很简单，把当前 element 对象标记为重建即可。

```dart
  @mustCallSuper
  void didChangeDependencies() {
    markNeedsBuild();
  }
```

# dirty

&emsp;如果 element 对象已被标记为需要重建，则 dirty getter 返回 true。

&emsp;当 element 对象首次创建以及在调用 markNeedsBuild 后，会把此标识设置为 true，该标识在 performRebuild 实现中会再被设置为 false，因为 performRebuild 表示就要真的进行重建了，所以此 Element 对象不再是脏的了。

&emsp;注意在 element 对象首次创建后此 dirty 标识也会置为 true。当 element 对象调用 performRebuild 函数开始执行重建时会把此标识置为 false。

```dart
  bool get dirty => _dirty;
  bool _dirty = true;
```

&emsp;OK，下面还有三个超重要的函数：markNeedsBuild、rebuild、performRebuild 三个超重要的函数，根据它们函数的实际内容也可以理解为：标记重建 -> 发起重建 -> 执行重建，看完它们以后 Element 类就可以完结了，继续加油！

# markNeedsBuild

&emsp;将该 Element 对象标记为脏元素，并将其添加到下一帧中集体都会进行重建的 Widget 列表中。

&emsp;由于在同一帧中两次构建一个 element 对象是低效的，因此应用程序和 Widget 应该被设计成只在帧开始之前的事件处理程序中标记 Widget 为脏状态，而不是在构建过程中标记。

```dart
  void markNeedsBuild() {
    if (_lifecycleState != _ElementLifecycle.active) {
      return;
    }
    
    if (dirty) {
      return;
    }
    
    _dirty = true;
    
    // 把当前 element 对象添加到全局的：final List<Element> _dirtyElements = <Element>[] 脏元素列表中，等待下一帧所有脏 Element 一起重建，
    // 之前学习 BuildOwner 时有详细讲解过整个过程，可以翻回去看看。
    owner!.scheduleBuildFor(this);
  }
```

# rebuild

```dart
  void rebuild({bool force = false}) { // ... }
```

&emsp;简单概括 rebuild：先进行一些判断然后实际调用 performRebuild 进行重建工作，并且 Element 的一众子类都没有重写这个 rebuild 函数，所以那些 element 子类对象都会调用到这里。

&emsp;官方对 rebuild 函数进行了详细注释，下面一起看一下：

&emsp;rebuild 调用会使 widget 进行重建。在开发模式下中，还会验证各种不变性，生产环境的话其实内部仅有一行：performRebuild() 调用。下面是 rebuild 函数被调用的一些场景：

+ 当 BuildOwner.scheduleBuildFor 被调用标记此 element 对象为脏时，在下一帧会由 BuildOwner 调用此方法；
+ 在 Element 初被创建出来时由 mount 调用；
+ 当 Widget 需要进行更新时由 update 调用；

&emsp;Element 对象仅在 dirty 为 true 时才会进行重建。若要无论 dirty 标志如何都进行重建，请将入参 force 设置为 true。此 force 参数很有用，例如在 Element 的 widget update 过程中，此时 element 对象的 dirty 标识是 false，此时直接通过 rebuild(force: true) 调用强制进行重建会比较方便。

## When rebuilds happen

### Terminology

&emsp;Widget 代表 Element 的配置。每个 Element 都有一个 Widget，存储在 Element.widget 字段中。术语 "widget" 通常在严格意义上更正确的时候使用，而应该使用 "element"。

&emsp;虽然一个 Element 具有当前 Widget，但是随着时间的推移，该 Widget 可能被其他 Widget 替换。例如，支持 ColoredBox 的 Element 可能首先有一个其 Widget 为 ColoredBox，其 ColoredBox.color 为蓝色，然后稍后将被赋予一个新的 ColoredBox，其颜色为绿色。

&emsp;在任何特定时间点，同一树中的多个 Element 可能具有相同的 Widget。例如，具有绿色的相同 ColoredBox 可能在 Widget 树中的多个位置同时使用，每个位置由不同的 Element 支持。

### Marking an element dirty

&emsp;在帧之间，一个 Element 可能会被标记为脏。这可能是由于各种原因造成的，包括以下情况：

+ StatefulWidget 的 State 可能通过调用 State.setState 方法导致其 Element 被标记为脏。
+ 当 InheritedWidget 发生改变时，之前订阅它的 Element 会被标记为脏。
+ 在热重载期间，每个元素都会被标记为脏（使用 Element.reassemble）。

### Rebuilding

&emsp;在下一帧中重新构建脏 Element。这是如何做到的确取决于 Element 的类型。

+ StatelessElement 通过使用其 widget 的 StatelessWidget.build 方法进行重建。
+ StatefulElement 通过使用其 widget 的 state 的 State.build 方法进行重建。
+ RenderObjectElement 通过更新其 RenderObject 进行重建。

&emsp;在许多情况下，重建的最终结果是单个子 widget 或者（对于 MultiChildRenderObjectElements）一组子 widget。

&emsp;这些子 widget 用于更新 Element 的子 element 的 widget 属性。如果新的 Widget 具有相同的类型和 key，则认为它对应于现有的 Element。（**对于 MultiChildRenderObjectElements，即使顺序发生变化，也会努力跟踪 widget；** 请参阅 RenderObjectElement.updateChildren。可翻回去看看之前的 updateChildren 解析，超详细。）

&emsp;如果之前没有对应的子 element，那么将创建一个新的 Element（使用 Widget.createElement）；然后递归构建该 Element。(注意这里的递归，整个子树构建完成才会结束。)

&emsp;如果以前已经有一个子 element，但构建未提供相应的 Widget 以更新它，则旧子 element 将被丢弃（或者，在涉及 GlobalKey 重新添加到 element tree 中的情况下，被重新使用在其他地方。）。

&emsp;然而，最常见的情况是有一个对应的先前子 element。这是通过要求子 element 使用新的子 widget 更新自己来处理的。对于 StatefulElement，这就是触发 State.didUpdateWidget 的情况。

### Not rebuilding

&emsp;在 Element 被告知使用新的 Widget 更新自己之前，会使用 operator == 来比较新旧 Widget 对象。

&emsp;通常情况下，这相当于使用 identical 进行比较，以查看这两个 Widget 对象是否确实是完全相同的实例。如果它们是相同的实例，并且 Element 并没有因为其他原因已被标记为需要更新，则 Element 将跳过更新自身，因为它可以确信更新自身或其子级将没有任何价值。(const 构造函数发挥作用的地方！)

&emsp;强烈建议避免在 Widget 对象上覆盖 operator ==。虽然这样做可能会改善性能，但实际上，对于非叶子 Widget，这会导致 O(N²) 的行为。这是因为必要时比较将包括比较子 Widget，并且如果这些子 Widget 也实现了 operator ==，则最终将导致对 Widget 子树的完全遍历……然后在树的每个级别上都重复这个操作。然后在实践中发现，重新构建会更加廉价。此外，如果应用程序中使用的 Widget 的任何子类实现了 operator ==，那么编译器无法在任何地方内联比较，因为必须将调用视为虚拟的，以防实例碰巧是重写了 operator 的实例。（意思就是不要重写 Widget 的 operator ==，这可能导致沿着 Widget 子树一个一个的比较两个 Widget 的子 Widget 是否都相等。）

&emsp;相反，避免不必要的重构的最佳方式是缓存从 State.build 返回的 Widget 对象，以便每帧使用相同的 Widget 对象，直到它们发生变化。存在多种机制来鼓励这样做：例如，const widgets 是一种形式的自动缓存（如果使用 const 常量表达式构造 Widget 对象，则每次使用相同参数构建时都返回同一个 Widget 对象实例）。

&emsp;另一个例子是 AnimatedBuilder.child 属性，它允许子树中非动画部分保持静态，即使 AnimatedBuilder.builder 回调重新创建其他组件。(这个后续要深入研究一下)

```dart
  @pragma('dart2js:tryInline')
  @pragma('vm:prefer-inline')
  @pragma('wasm:prefer-inline')
  void rebuild({bool force = false}) {
    // 1. 如果当前 element 对象的状态不是 active
    // 2. 或者非 dirty 且 force 参数为 false
    // 则直接 return，不进行重建。
    if (_lifecycleState != _ElementLifecycle.active || (!_dirty && !force)) {
      return;
    }
    
    try {
      // 最重要的内容，实际执行重建时，调用的是 performRebuild 函数哦
      performRebuild();
      
    } finally {
    }
  }
```

# performRebuild

&emsp;在适当的检查之后，使 widget 更新自身。在 rebuild 被调用后调用。Element 类的基本实现只是清除了 Element 对象的 dirty 标志，而其它的 Element 子类都对其进行了重写。特别重要，后面学习 Element 子类时我们再一一展开。

```dart
  @protected
  @mustCallSuper
  void performRebuild() {
    _dirty = false;
  }
```

# Element 总结

&emsp;OK，到这里 Element 类的内容就看完了，一共分了 5 篇内容。Element 真的是一个超级大类，感觉它是 Flutter framework 第一重要的类应该是没有异议的。

&emsp;



## 参考链接
**参考链接:🔗**
+ [Element class](https://api.flutter.dev/flutter/widgets/Element-class.html)
+ [kFlutterMemoryAllocationsEnabled top-level constant](https://api.flutter.dev/flutter/foundation/kFlutterMemoryAllocationsEnabled-constant.html)
+ [Flutter | Understanding the MemoryAllocations](https://medium.com/@maciejbrzezinski/flutter-what-is-memoryallocations-1ee2eb0a8670)
+ [Flutter | How to create a custom binding and inject your own BuildOwner](https://medium.com/@maciejbrzezinski/flutter-how-to-create-a-custom-binding-and-inject-your-own-buildowner-f60ef320537b)
+ [Support for WebAssembly (Wasm)](https://docs.flutter.dev/platform-integration/web/wasm)

