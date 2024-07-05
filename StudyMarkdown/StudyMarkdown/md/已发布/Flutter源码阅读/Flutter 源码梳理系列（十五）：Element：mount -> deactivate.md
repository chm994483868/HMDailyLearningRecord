# Flutter 源码梳理系列（十五）：Element：mount -> deactivate

&emsp;前面梳理完了 updateChild 和 updateChildren 的内容，收获颇多。OK，下面我们继续，一鼓作气把 Element 中的内容全部看完。

# mount

```dart
@mustCallSuper
void mount(
    Element? parent,
    Object? newSlot
)
```

&emsp;mount 用于将 element 对象添加到给定父元素 parent 对象的给定 newSlot 中。

&emsp;当一个新创建的 element 对象首次添加到 Element tree 中时，framework 会调用这个函数。

&emsp;使用这个方法来初始化依赖于父级 Element 的状态。独立于父级的状态可以更容易地在构造函数中初始化。(意思把独立的字段提取出来后续再进行赋值，可以优化 Element 对象快速创建。)

&emsp;这个方法将 element 对象从 "initial" 生命周期状态转换为 "active" 生命周期状态。（element 对象默认就是 initial，然后调用 mount 函数后修改为了：active）

&emsp;重写这个方法的子类可能还想要同时重写:

+ update、
+ visitChildren、
+ RenderObjectElement.insertRenderObjectChild、
+ RenderObjectElement.moveRenderObjectChild、
+ RenderObjectElement.removeRenderObjectChild、

&emsp;ComponentElement、RenderObjectElement、SingleChildRenderObjectElement、MultiChildRenderObjectElement 都重写了 mount 函数，超级重要的函数。特别是 RenderObjectElement 重写 mount 函数，它会在 mount 函数中调用 `_renderObject = (widget as RenderObjectWidget).createRenderObject(this)`，创建自己的 RenderObject，超级重要，这里着重记忆一下，为我们后续学习 RenderObject 打基础。

&emsp;这个方法的实现应该以对继承方法的调用开始，如：super.mount(parent, newSlot)。看到有些函数重写时 super.xxx 写在函数开头，有些 super.xxx 写在函数结尾，这个真的需要注意一下，当我们要重写某个父类函数时，确实需要点进去看一下注释在决定 spuer.xxx 的位置。

&emsp;同时从它的 @mustCallSuper 注解也可以看出，Element 子类重写此方法时，内部必须调用 super.mount。

&emsp;mount 函数直白一点理解的话，大概就是往双向链表中添加节的感觉。

```dart
  @mustCallSuper
  void mount(Element? parent, Object? newSlot) {
    // 开始有四个断言：
    
    // 当前 element 对象的 _lifecycleState 是 initial、
    // _parent 为 null，
    // 入参 parent 的状态是 active，
    // 当前 element 对象的 slot 为 null，
    
    // 四个断言，主要可确定到此 element 对象是一个初始状态。
    
    // 父级，可以理解为双向链表中当前节点指向父节点
    _parent = parent;
    
    // slot、生命周期状态、depth 赋值
    _slot = newSlot;
    _lifecycleState = _ElementLifecycle.active;
    _depth = _parent != null ? _parent!.depth + 1 : 1; // 父级深度加 1
    
    if (parent != null) {
      // 只有在 parent 非 null 的情况下才分配 owner。
      // 如果父级为 null（根节点 RootElement，创建之初就以分配了 _owner 值），则应已分配 owner。
      // 可参阅 RootRenderObjectElement.assignOwner()。
      _owner = parent.owner;
      _parentBuildScope = parent.buildScope;
    }
    
    // 如果 widget 有 GlobalKey，则以 global key 为 Key，以此 element 为 Value，
    // 把它们保存在全局 Map 中：Map<GlobalKey, Element> _globalKeyRegistry 
    final Key? key = widget.key;
    if (key is GlobalKey) {
      owner!._registerGlobalKey(key, this);
    }
    
    // 传递全局的 PersistentHashMap<Type, InheritedElement>? _inheritedElements，
    // 从 element tree 的根节点开始，所有的 Element 链路上的 InheritedElement 对象都会以它们的类型为 key 保存在 _inheritedElements 中，
    // InheritedElement 则重写了此函数，如果是 InheritedElement 类型对象调用 _updateInheritance 的话，
    // 是把自己添加到 _parent?._inheritedElements 中并赋值给自己，
    // 如果是其它类型的 Element 对象的话，则只是把 _parent?._inheritedElements 赋值给自己。
    _updateInheritance();
    
    // 传递全局的 _notificationTree
    attachNotificationTree();
  }
```

&emsp;总结：mount 函数的内容还是很清晰的。然后全局搜索发现 mount 函数的调用位置只有一处：在 inflateWidget 函数内，final Element newChild = newWidget.createElement() 创建一个新的 element 对象后，然后直接调用 newChild.mount(this, newSlot)。即直接对一个新创建的 element 对象进行挂载，然后 mount 函数开头的四个断言也预示着 mount 函数只对新建的处于 initial 状态的 element 节点才能调用。

&emsp;然后就是有 GlobalKey 的 widget 对应的 element 对象会被全局收集起来，这里死记即可，当一个有 global key 的 element 节点被挂载到 elemet tree 上时就会同时把它收集在全局的 Map 中等待重用，重用的话，当对一个 widget 对象进行 inflateWidget 时，会首先判断有没有 global key 和能不能找到对应的 element，如果能找到的话，就不会调用新 widget 的 createElement 来创建新的 element 对象了，而是直接使用此现有的 global key 对应的 element，并且会把它挪到新的父级 Element 节点，即页面上不能同时出现两个同一 Global Key 对应的 Element 对象。

&emsp;然后就是 owner/buildScope/inheritedElements/notificationTree 在整个 element tree 上每个节点的传递，Flutter framework 通过这种方式可以保证在任何 element 节点处都能以 O(1) 的复杂度找到要找的数据。

&emsp;暂时我们以 Element 为主，其它几个特别重要的类，如：BuildOwner、BuildScope 等等，我们后面再看。（前面已经分析完啦，感兴趣的可以翻回去看看）

# update

```dart
@mustCallSuper
void update(
    covariant Widget newWidget
)
```

&emsp;update 函数用于更改用于配置此 element 的 widget 对象。

&emsp;当父级希望使用不同的 widget 来配置此 element 时，framework 会调用此函数。新的 widget 保证具有与旧 widget 件相同的 runtimeType。

&emsp;此函数仅在 Element 的 "active" 生命周期状态期间调用。开头的断言：newWidget 不能与 element 当前的 widget 是同一个，newWidget 和 element 当前的 widget 对象调用 Widget.canUpdate 的话返回 true。

```dart
  @mustCallSuper
  void update(covariant Widget newWidget) {
    _widget = newWidget;
  }
```

&emsp;Element.update 函数实现极其简单，仅有一个 `_widget` 赋值，然后 @mustCallSuper 注解表明所有的 Element 子类重写此函数的话需要调用 super.update。然后是几乎所有的 Element 子类都对 update 函数进行了重写，如果仅看 Element.update 内容就仅一个 `_widget` 赋值，但在 Element 子类重写中，实际需要进行整个以此 Element 对象为根节点的 Element 子树的重建。例如：

+ StatelessElement.update
+ StatefulElement.update
+ ProxyElement.update
+ RenderObjectElement.update
+ SingleChildRenderObjectElement.update
+ MultiChildRenderObjectElement.update

&emsp;等等，后面我们学习 Element 的子类时再详细展开。

&emsp;下面是两个根 Slot 相关的函数，主要与 RenderObjectElement 及其子类有关，后续我们再深入学习。

# updateSlotForChild

&emsp;在其父级（Element 对象）中更改给定 child 所占用的 slot。

&emsp;当子项（child）从该元素的子项列表（children）中的一个位置移动到另一个位置时，由 MultiChildRenderObjectElement 和其他具有多个子项的 RenderObjectElement 子类调用。

```dart
  @protected
  void updateSlotForChild(Element child, Object? newSlot) {
  
    void visit(Element element) {
      element.updateSlot(newSlot);
      
      // 在子树中找下一个 RenderObjectElement
      final Element? descendant = element.renderObjectAttachingChild;
      
      if (descendant != null) {
        visit(descendant);
      }
    }
    
    visit(child);
  }
```  

# updateSlot

&emsp;当 framework 需要更改该 Element 对象在其父级中占据的 slot 时，会被 updateSlotForChild 调用。RenderObjectElement 对此函数进行了重写。

```dart
  @protected
  @mustCallSuper
  void updateSlot(Object? newSlot) {    
    _slot = newSlot;
  }
```

# `_updateDepth`

&emsp;对 element 的所有 child 递归调用 `_updateDepth` 函数，更新它们的 `_depth` 值。

&emsp;visitChildren 函数，会遍历所有的 child。

```dart
  void _updateDepth(int parentDepth) {
    final int expectedDepth = parentDepth + 1;
    
    if (_depth < expectedDepth) {
      _depth = expectedDepth;
      
      // visitChildren 函数不同的 element 有不同的实现，
      // 会对 element 的 child 进行遍历调用入参的函数
      visitChildren((Element child) {
        child._updateDepth(expectedDepth);
      });
    }
  }
```

# detachRenderObject

&emsp;主要是 RenderObjectElement 的重写。从 Render Object Tree 中移除 renderObject。

&emsp;这个函数的默认实现会在每个子节点上递归调用 detachRenderObject。RenderObjectElement.detachRenderObject 重写此方法执行真正的工作，将 renderObject 从 render object tree 中移除。

```dart
  void detachRenderObject() {
  
    // 对 element 的 child 遍历调用入参函数
    visitChildren((Element child) {
      child.detachRenderObject();
    });
    
    _slot = null;
  }
```

# attachRenderObject

&emsp;将 renderObject 添加到由 newSlot 指定的位置处的 render object tree 中。

&emsp;该函数的默认实现会在每个子元素上递归调用 attachRenderObject。而 RenderObjectElement.attachRenderObject 的重写会实际执行将 renderObject 添加到 render object tree 中的工作。

&emsp;newSlot 参数指定了此元素新 slot 的值。

```dart
  void attachRenderObject(Object? newSlot) {
  
    visitChildren((Element child) {
      child.attachRenderObject(newSlot);
    });
    
    _slot = newSlot;
  }
```

# `_retakeInactiveElement`

&emsp;根据入参 global key 从全局 map 中取出处于非活动状态的 element 对象。(如何是活动对象的话，会让其失活以后在返回。)

&emsp;全局仅有一处调用，即在 inflateWidget 函数中，当拿着 newWidget 要进行 inflate 时，先判断下此 newWidget 有没有 global key，然后判断 global key 有没有对应的 element 对象，如果有的话可以取出来进行复用。会首先判断取得的 element 对象有没有 parent，如果有的话说明此 global key 对应的 element 对象目前正在 Element Tree 上，此时会对其进行失活（整个子树失活），然后再返回复用。（这个正对应：同一个 Global Key 对应的 Element 只能在 Element Tree 中出现一次。）

```dart
  Element? _retakeInactiveElement(GlobalKey key, Widget newWidget) {
    // 取得 global key 对应的 element 对象，
    // 这个是直接在当下 BuilcOwner 的：（WidgetsBinding.instance.buildOwner!）
    // final Map<GlobalKey, Element> _globalKeyRegistry = <GlobalKey, Element>{};
    // 取 GlobalKey 对应的 Element 即可。
    final Element? element = key._currentElement;
    
    // 不存在的话直接返回 null 即可
    if (element == null) {
      return null;
    }
    
    // 如果新的 newWidget 不能更新 element 的话，即使取到了 element，也要返回 null
    if (!Widget.canUpdate(element.widget, newWidget)) {
      return null;
    }
    
    // 如果取到的 element 有父级的并且父级正是当前 element 的话，
    // 在开发模式下的话，会直接报错：在 widget tree 中，一次只能在一个 widget 上指定同一个 GlobalKey。
    // 有 global key 的 widget 只能在树中出现一次。
    final Element? parent = element._parent;
    
    // 如果当前 global key 对应的 element 有 parent 的话，说明此 element 目前挂载在 Element tree 上，
    // 需要把它失活从 Element tree 上移除下来。
    if (parent != null) {
      assert(() {
        if (parent == this) {
          // 如果父级刚好就是当前节点的话，那同一个 global key 的 widget 在屏幕上已经存在了，不能出现两次！
          throw FlutterError.fromParts(<DiagnosticsNode>[
            ErrorSummary("A GlobalKey was used multiple times inside one widget's child list."),
            // ...
            ErrorDescription('A GlobalKey can only be specified on one widget at a time in the widget tree.'),
          ]);
        }
        
        parent.owner!._debugTrackElementThatWillNeedToBeRebuiltDueToGlobalKeyShenanigans(
          parent,
          key,
        );
        
        return true;
      }());
      
      // 断开 element 与 parent 的关联，为了把此 element 从 Element tree 上卸载下来
      parent.forgetChild(element);
      
      // 将给定元素移动到非活动元素列表中，并从渲染树中分离其渲染对象。
      // 把以 element 为根节点的 Element 子树整个的从 Element Tree 上卸载下来。
      parent.deactivateChild(element);
    }
    
    // 把 element 从全局的非活动 Element 表中移除，因为上面的 parent.deactivateChild(element) 调用，
    // 已经把 element 添加到了全局的非活动 Element 表，而此函数返回后，element 整棵子树会被重新激活，
    // 所以这里提前把 element 从全局的非活动 Element 表中移除
    owner!._inactiveElements.remove(element);
    
    return element;
  }
```

&emsp;又是一处跟 global key 相关的地方，要牢记：

+ 读取：在拿到新 widget 对象后，要 inflate 它时，如果新 widget 对象有 globl key 的话，则会去读取它对应的 element，尝试复用。
+ 保存：在挂载一个新的 element 对象时，如果对应的 widget 有 global key 的话，会把这个 global key 和 element 对象，保存在全局 map 中。 

# inflateWidget

```dart
@protected
Element inflateWidget(
    Widget newWidget,
    Object? newSlot
)
```

&emsp;inflateWidget 用于为给定的 newWidget 对象创建一个 Element 对象，并将其作为当前这个 element 对象的 child 添加到指定的 newSlot 中。

&emsp;通常由 updateChild 方法调用，但是子类也可以直接调用该方法，以便更精细地控制创建 element 对象。

&emsp;如果给定的 newWidget 对象具有 global key 并且已经存在具有该 global key 的 widget 的 element 对象，则此函数将重用该 element 对象（可能是从 element tree 中的另一个位置嫁接过来，或者从未活动 element 列表中重新激活），而不是创建一个新 element 对象。

&emsp;newSlot 参数指定该元素 slot 的新值。

&emsp;此函数返回的 element 对象已经被挂载到 Element Tree 中，并且处于 "active" 生命周期状态。

```dart
@protected
  @pragma('dart2js:tryInline')
  @pragma('vm:prefer-inline')
  @pragma('wasm:prefer-inline')
  Element inflateWidget(Widget newWidget, Object? newSlot) {
    try {
      // 如果入参 newWidget 有 global key 的话则会去尝试读取并复用，
      // 如果当前是 newWidget 初次创建 Element 的话，会在挂载时把它注册起来，
      // 以便后续的复用。
      final Key? key = newWidget.key;
      
      if (key is GlobalKey) {
        // 取得根据 global key 保存在全局 map 中的 element 对象
        final Element? newChild = _retakeInactiveElement(key, newWidget);
        
        if (newChild != null) {
        
          // 需要保证取得的 element._parent 是 null
          try {
            // 把取得的 element 对象激活，
            // 其实是整个的把以 newChild 为根节点的整个子树激活，并挂载在当前节点下，
            // 当前节点是它的父节点。
            newChild._activateWithParent(this, newSlot);
          } catch (_) {
            // ...
          }
          
          // 通过前面的 _retakeInactiveElement 函数得知，如果能走到这里就表示 newWidget 是可以更新 newChild 的，
          // 然后沿着 newWidget 对此 newChild 进行 Element 子树的重建。
          final Element? updatedChild = updateChild(newChild, newWidget, newSlot);
          
          return updatedChild!;
        }
      }
      
      // 如果没有 global key 的话，则比较正常的流程，
      // 使用入参 newWidget 创建对应的 element 对象。
      final Element newChild = newWidget.createElement();
      
      // 直接把 newChild 对象挂载到 Element Tree 上，继续根据 newWidget 的情况一层一层的新建 element 子树，直到没有新 widget 为止。
      // 这里的话，只要 newWidget 后续还有新的 Widget 子级， mount 就会被递归调用，直到沿着这个 newWidget 把整个 element 子树构建完成。
      newChild.mount(this, newSlot);
      
      // 把当前这个 element 子节点返回
      return newChild;
    } finally {
        // ...
    }
  }
```

&emsp;inflateWidget 函数根据入参 newWidget 对象，要么有 global key 的话找到对应的 element 进行复用，并进行子树更新重建。如果没有的话，则是创建一个对应的 element 对象，把它挂载到 Element Tree 上，然后进行整个 Element 子树的新建。

&emsp;看到这里，突然想到一个问题，鉴于 Element Tree 是一层一层的构建的，直到没有新的 widget 对象为止。那那些跟 Element 节点构建相关的函数一直被一层一层的重复调用，那如果 Element Tree 的层级特别深，那这相关函数的函数调用栈也越来越深，它们会栈溢出吗？

# deactivateChild

&emsp;deactivateChild 函数将入参 element 对象的 `_parent` 指向 null（原本是指向自己，即断开与父级 Element 的关联。），将其放入一个全局的非活动元素列表（`_inactiveElements`），并将其所有 RenderObject 从 RenderObject Tree 中分离。（或者说是把以入参 Element 为根节点的整个 Element 子树从 Element Tree 上卸载，并把所有的 Element 节点都设置为 inactive 非活动状态。）

&emsp;调用方负责将子元素从其子模型中移除。通常情况下，在对其子模型进行更新时，元素本身会调用 deactivateChild 方法；然而，在 GlobalKey 重新指定父级时，新父级会主动调用旧父级的 deactivateChild 方法，并首先使用 forgetChild 方法来使旧父级更新其子模型。

&emsp;调用方负责将入参 child Element 对象从其子树中移除（`_parent` 置为 null 即移除，一个 Element 对象加上它的 child，即一颗子树，或者一个 element 对象就可以理解为是一颗子树）。通常，deactivateChild 是由 Element 自身在更新其子树时调用的；然而，在 GlobalKey 重新父级化期间，新 Element 父级会主动调用原 Element 父级的 deactivateChild，首先使用 forgetChild 引起原 Element 父级更新其子树。

```dart
  @protected
  void deactivateChild(Element child) {
    // child 父级置为 null，把它断开与当前 Elemet 对象的关联
    child._parent = null;
    // 把 child Element 子树中所有的 RenderObject 从 RenderObject Tree 分离
    child.detachRenderObject();
    
    // 把 child 加入全局非活动对象列表中，同时内部 child 会调用 deactivate 函数，
    // 内部会递归调用 child 的所有子级 element 对象的 deactivate 函数，使它们所有处于 inactive 状态。
    owner!._inactiveElements.add(child); // this eventually calls child.deactivate()
  }
```

# `_activateWithParent`

&emsp;使用给定的父级 element 和 newSlot 激活当前 element 对象以及它的所有子级 element 对象，类似新 element 节点挂载的过程。（或者说是把整个以自己为根节点的 Element 子树激活，并挂载在入参 parent 节点处。）

```dart
  void _activateWithParent(Element parent, Object? newSlot) {
    // 当前 Element 对象父级指向入参 parent
    _parent = parent;
    // _owner 直接由父级 owner 赋值
    _owner = parent.owner;
    
    // 沿着父级更新 depth
    _updateDepth(_parent!.depth);
    // 递归传递 _parentBuildScope = _parent?.buildScope
    _updateBuildScopeRecursively();
    
    // 递归调用所有子级 Element 节点的 activate 函数
    _activateRecursively(this);
    
    // 递归使所有 RenderObject 都添加到 RenderObject tree 中
    attachRenderObject(newSlot);
  }
```

# activate

&emsp;使当前 Element 对象从 "inactive" 生命周期状态过渡到 "active" 生命周期状态。

&emsp;当先前停用的 element 对象重新合并到 element tree 中时，framework 会对它以及它的所有子级 Eleemnt 对象调用此方法。framework 不会在 element 第一次变为活动状态（即从 "initial" 生命周期状态）时调用此方法。在那种情况下，framework 会调用 mount 方法。(例如从非活动列表中复用 element 节点)

&emsp;此方法的实现应该以调用继承方法的方式开始，例如 super.activate()，如 @mustCallSuper 注解提示，子类重写此方法时需要先调用：super.activate()。

```dart
  @mustCallSuper
  void activate() {
    // 有可能依赖了上层的 InheritedElement 对象
    final bool hadDependencies = (_dependencies != null && _dependencies!.isNotEmpty) || _hadUnsatisfiedDependencies;
    
    // 状态修改为 active
    _lifecycleState = _ElementLifecycle.active;
    
    // 我们在 deactivate 中注销了我们的依赖项，但从未清除列表。
    // 由于我们将被重用，现在让我们清除我们的列表。
    _dependencies?.clear();
    
    _hadUnsatisfiedDependencies = false;
    
    // 把父节点的 _inheritedElements 赋值给自己的。
    //（原来如果是 element 对象自己调用 deactivate 的话，是不会把自己的 _parent 置为 null 的，
    // 其实只会把当前子树的根节点的 _parent 置为 null。）
    
    // 如果是 element 对象调用自己的 deactivateChild 函数，把指定的自己的子级失活的话，会把子级的 _parent 置为 null。
    
    // InheritedElement 重写了此函数，如果是 InheritedElement 对象调用 _updateInheritance 的话，
    // 是把自己添加到 _parent?._inheritedElements 中并赋值给自己，
    // 如果是其它类型的 Element 对象的话，则只是把 _parent?._inheritedElements 赋值给自己。
    _updateInheritance();
    
    // 混入了 NotifiableElementMixin 的 Element 算是重写了此函数，
    // NotifiableElementMixin 重写了此函数，如果是 NotifiableElementMixin 对象调用 attachNotificationTree 的话，
    // 会用自己构建一个 _NotificationNode 节点拼接在 notification tree 中，
    / 其它类型的 Element 对象则是把父节点的 _notificationTree 赋值给自己的。
    attachNotificationTree();
    
    // 如果 _dirty 为true，则会把此 element 对象添加到脏 Element 列表中，下一帧对它进行重建。
    if (_dirty) {
      owner!.scheduleBuildFor(this);
    }
    
    // 如果之前有依赖的 InheritedElement 对象的话，
    // 如果有的话，调用自己的 markNeedsBuild 给自己打上脏标记，下一帧进行更新重建。
    //（实际执行内容与上面的 owner!.scheduleBuildFor(this) 一样，
    // 这里设定为 didChangeDependencies 函数，是为了让 StatefulElement 类重写此函数，
    // 当 StatefulElement 对象执行到这里时，可以给自己的 _didChangeDependencies = true，
    // 然后下一帧重建时，也能回调到 state 对象的 didChangeDependencies 回调函数。）
    if (hadDependencies) {
      didChangeDependencies();
    }
  }
```

&emsp;过程类似 mount，当然不同点主要集中在可能要立即进行重建（有 dirty 标记或者有依赖 InheritedElement 对象）。

# deactivate

&emsp;使当前 Element 对象从 "active" 生命周期状态过渡到 "inactive" 生命周期状态。

&emsp;当先前处于活动状态的 element 被移动到非活动 element 列表时，framework 会对它的以及它的所有子级 Eleemnt 对象调用此方法。在非活动状态下，element 将不会出现在屏幕上。element 对象只能保持在非活动状态直到当前帧结束。在动画帧结束时，如果 element 对象还没有被重新激活，framework 将 unmount 该 element 对象。（会对 element 对象的所有子级 element 对象调用 unmount 函数。）

&emsp;这是由 deactivateChild 间接调用的。

&emsp;此方法的实现应该以调用继承方法结束，如 super.deactivate()。

```dart
  @mustCallSuper
  void deactivate() {
    // 如果之前依赖过 InheritedElemet 对象的话，现在遍历那些 InheritedElement 对象，
    // 把自己从那些 InheritedElement 对象的依赖者列表中移除自己。
    if (_dependencies != null && _dependencies!.isNotEmpty) {
      for (final InheritedElement dependency in _dependencies!) {
        dependency.removeDependent(this);
      }
      
    // 出于方便起见，在这里我们实际上并不清除列表，即使它不再代表我们注册的内容。如果我们永远不会被重复使用，这都无所谓。但如果需要重新使用，我们会在 activate() 中清空列表。这样做的好处是让 Element 的 activate() 实现可以根据我们在这里是否有依赖关系来决定是否重建。
    }
    
    // 记录的从根节点到目前的所有 InheritedElement 对象列表置为 null
    _inheritedElements = null;
    // 状态修改为 inactive
    _lifecycleState = _ElementLifecycle.inactive;
  }
```

&emsp;OK，篇幅有限，本篇就先到这里吧，从 mount 函数到 deactivate 函数。

## 参考链接
**参考链接:🔗**
+ [Element class](https://api.flutter.dev/flutter/widgets/Element-class.html)
+ [kFlutterMemoryAllocationsEnabled top-level constant](https://api.flutter.dev/flutter/foundation/kFlutterMemoryAllocationsEnabled-constant.html)
+ [Flutter | Understanding the MemoryAllocations](https://medium.com/@maciejbrzezinski/flutter-what-is-memoryallocations-1ee2eb0a8670)
+ [Flutter | How to create a custom binding and inject your own BuildOwner](https://medium.com/@maciejbrzezinski/flutter-how-to-create-a-custom-binding-and-inject-your-own-buildowner-f60ef320537b)
+ [Support for WebAssembly (Wasm)](https://docs.flutter.dev/platform-integration/web/wasm)
