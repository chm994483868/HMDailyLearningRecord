# element

## mount

```dart
@mustCallSuper
void mount(
    Element? parent,
    Object? newSlot
)
```

&emsp;mount 用于将 element 对象添加到给定父元素 parent 对象的给定 slot newSlot 对象中。

&emsp;在新创建的 element 对象第一次添加到 element tree 中时，会调用此函数。使用这个方法来初始化依赖于父元素的状态。独立于父级的状态可以更容易地在构造函数中初始化。(意思把独立的字段提取出来后续再进行赋值，可以优化 element 对象快速创建)

&emsp;这个方法将 element 对象从 "initial" 生命周期状态转换为 "active" 生命周期状态。

&emsp;重写这个方法的子类可能还想要同时重写 update、visitChildren、RenderObjectElement.insertRenderObjectChild、RenderObjectElement.moveRenderObjectChild 和 RenderObjectElement.removeRenderObjectChild。（ComponentElement、RenderObjectElement、SingleChildRenderObjectElement、MultiChildRenderObjectElement 都重写了 mount 函数。）

&emsp;这个方法的实现应该以对继承方法的调用开始，如：super.mount(parent, newSlot)，同时从它的 @mustCallSuper 注解也可以看出，Element 子类重写此方法时，内部必须调用 super.mount。

```dart
  @mustCallSuper
  void mount(Element? parent, Object? newSlot) {
    // 开始有四个断言：element 的状态是 initial、_parent 为 null，入参 parent 的状态是 active，slot 为 null，
    // 四个断言，主要可确定到此 element 对象是一个初始状态。
    
    // 父级、slot、生命周期状态、depth 赋值
    _parent = parent;
    _slot = newSlot;
    _lifecycleState = _ElementLifecycle.active;
    _depth = _parent != null ? _parent!.depth + 1 : 1; // 父级深度加 1
    
    if (parent != null) {
      // 只有在 parent 非 null 的情况下才分配 owner。如果父级为 null（根节点），则应已分配 owner。
      // 请参阅 RootRenderObjectElement.assignOwner()。
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
    // 从 element tree 的根节点开始，所有的 InheritedElement 对象都会以它们的类型为 key 保存在 _inheritedElements 中，
    // InheritedElement 重写了此函数，如果是 InheritedElement 对象调用 _updateInheritance 的话，是把自己添加到 _parent?._inheritedElements 中并赋值给自己，
    // 如果是其它类型的 Element 对象的话，则只是把 _parent?._inheritedElements 赋值给自己。
    _updateInheritance();
    
    // 传递全局的 _notificationTree
    attachNotificationTree();
  }
```

&emsp;总结：mount 函数的内容还是很清晰的。然后全局搜索发现 mount 函数的调用位置只有一处：在 inflateWidget 函数内，final Element newChild = newWidget.createElement() 创建一个新的 element 对象后，然后直接调用 newChild.mount(this, newSlot)。即直接对一个新创建的 element 对象进行挂载，然后 mount 函数开头的四个断言也预示着 mount 函数只对新建的处于 initial 状态的 element 节点才能调用。

&emsp;然后就是有 GlobalKey 的 widget 对应的 element 对象会被全局收集起来，这里死记即可，当一个有 global key 的 element 节点被挂载到 elemet tree 上时就会同时把它收集在全局的 Map 中等待复用，复用的话，当对一个 widget 对象进行 inflateWidget 时，会首先判断有没有 global key 和能不能找到对应的 element，如果能找到的话，就不会调用新 widget 的 createElement 来创建新的 element 对象了。

&emsp;然后就是 owner/buildScope/inheritedElements/notificationTree 在整个 element tree 上每个节点的传递，Flutter framework 通过这种方式可以保证在任何 element 节点处都能以 O(1) 的复杂度找到要找的数据。

&emsp;暂时我们以 Element 为主，其它几个特别重要的类，如：BuildOwner、BuildScope 等等，我们后面再看。

## update

```dart
@mustCallSuper
void update(
    covariant Widget newWidget
)
```

&emsp;update 函数用于更改用于配置此 element 的 widget。

&emsp;当父级希望使用不同的 widget 来配置此 element 时，framework 会调用此函数。新的 widget 保证具有与旧 widget 件相同的 runtimeType。

&emsp;此函数仅在 Element 的 "active" 生命周期状态期间调用。开头的断言：newWidget 不能与 element 当前的 widget 是同一个，newWidget 和 element 当前的 widget 调用 Widget.canUpdate 返回 true。

```dart
  @mustCallSuper
  void update(covariant Widget newWidget) {
    _widget = newWidget;
  }
```

&emsp;Element 的 update 函数实现极其简单，仅有一个 `_widget` 赋值，然后 @mustCallSuper 注解表明所有的 Element 子类重写此函数的话需要调用 super.update。然后是几乎所有的 Element 子类都对 update 函数进行了重写，如果仅看 Element.update 内容就仅一个 `_widget` 赋值，但在 Element 子类中，实际需要进行整个 Element 子树的重建。例如 StatelessElement.update/StatefulElement.update/ProxyElement.update/SingleChildRenderObjectElement.update 等等，后面我们再详细展开。

## updateSlotForChild

&emsp;在其父级（Element 对象）中更改给定 child 所占用的 slot。

&emsp;当子项（child）从该元素的子项列表（children）中的一个位置移动到另一个位置时，由 MultiChildRenderObjectElement 和其他具有多个子项的 RenderObjectElement 子类调用。

&emsp;开头两个断言：当前 element 对象处于 active 状态，入参 child 的 `_parent`（父级）是自己。然后递归调用 visit 函数，主  

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

## updateSlot

&emsp;当 framework 需要更改该 Element 对象在其父级中占据的 slot 时，会被 updateSlotForChild 调用。

&emsp;三个断言也预示了 element 对象以及其父级都处于 active 状态，

```dart
  @protected
  @mustCallSuper
  void updateSlot(Object? newSlot) {    
    _slot = newSlot;
  }
```

## `_updateDepth`

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

## detachRenderObject

&emsp;主要是 RenderObjectElement 的重写。从 render object tree 中移除 renderObject。

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

## attachRenderObject

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

## `_retakeInactiveElement`

&emsp;根据入参 global key 从全局 map 中取出处于非活动状态的 element 对象。

&emsp;全局仅有一处调用，即在 inflateWidget 函数中，当拿着 newWidget 要进行 inflate 时，先判断下此 newWidget 有没有 global key，然后判断 global key 有没有对应的 element 对象，如果有的话可以取出来进行复用，而 `_retakeInactiveElement` 函数就是来取非活动状态的 element 对象的，注意是 非活动 状态的：Inactive。

```dart
  Element? _retakeInactiveElement(GlobalKey key, Widget newWidget) {
    // 取得 global key 对应的 element 对象
    final Element? element = key._currentElement;
    
    if (element == null) {
      return null;
    }
    
    // 如果新的 newWidget 不能更新 element 的话，即使取到了 element，也要返回 null
    if (!Widget.canUpdate(element.widget, newWidget)) {
      return null;
    }
    
    // 如果取到的 element 有父级的并且父级正是当前 element 的话，
    // 在开发模式下的话，会直接报错：在 widget tree 中，一次只能在一个 widget 上指定 GlobalKey。
    // 有 global key 的 widget 只能在树中出现一次。
    final Element? parent = element._parent;
    
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
      
      // 从元素的子列表中删除给定的子元素，以准备在元素树的其他地方被重用的子元素。
      parent.forgetChild(element);
      
      // 将给定元素移动到非活动元素列表中，并从渲染树中分离其渲染对象。
      parent.deactivateChild(element);
    }
    
    // 把 element 从全局的非活动元素表中移除
    owner!._inactiveElements.remove(element);
    
    return element;
  }
```

&emsp;又是一处跟 global key 相关的地方，要牢记：

+ 读取：在拿到新 widget 对象后，要 inflate 它时，如果新 widget 对象有 globl key 的话，则会去读取它对应的 element，尝试复用。
+ 保存：当挂载一个新的 element 对象时，如果对应的 widget 有 global key 的话，会把这个 global key 和 element 对象，保存在全局 map 中。 

## inflateWidget

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

&emsp;此函数返回的 element 对象已经被挂载，并且处于 "active" 生命周期状态。

```dart
@protected
  @pragma('dart2js:tryInline')
  @pragma('vm:prefer-inline')
  @pragma('wasm:prefer-inline')
  Element inflateWidget(Widget newWidget, Object? newSlot) {
    try {
      // 如果新 widget 对象有 global key
      final Key? key = newWidget.key;
      if (key is GlobalKey) {
        // 取得根据 global key 保存在全局 map 中的 element 对象
        final Element? newChild = _retakeInactiveElement(key, newWidget);
        
        if (newChild != null) {
        
          // 需要保证取得的 element._parent 是 null
          
          try {
            // 把取得的 element 激活（内容与挂载几乎一致）
            newChild._activateWithParent(this, newSlot);
          } catch (_) {
            // ...
          }
          
          // 然后沿着此 element 进行子树更新重建 
          final Element? updatedChild = updateChild(newChild, newWidget, newSlot);
          
          assert(newChild == updatedChild);
          return updatedChild!;
        }
      }
      
      // 如果没有 global key 的话，则比较正常的流程，
      // 根据新 widget 对象创建对应的 element 对象
      final Element newChild = newWidget.createElement();
      
      // 然后直接把新 element 对象挂载到 element tree 上，然后接下来进行 element 子树的新建，直到没有新 widget 为止。
      // 注意 mount 会被循环调用的过程，直到沿着这个 newWidget 把整个 element 子树构建完成。
      newChild.mount(this, newSlot);
      
      // 把当前这个 element 子节点返回
      return newChild;
    } finally {
        // ...
    }
  }
```

&emsp;inflateWidget 函数根据新 widget 对象，要么有 global key 的话找到对应的 element 进行复用，并进行子树更新重建。如果没有的话，则是创建一个对应的 element 对象，把它挂载到 element tree 上，然后进行整个子树的新建。

&emsp;看到这里，突然想到一个问题，鉴于 Element tree 是一层一层的构建的，直到没有新的 widget 对象了。它的那些跟 element 节点构建相关的函数一直被一层一层的重复调用，那如果 element tree 的层级特别深，那这相关函数的函数调用栈也越来越深，它们会栈溢出吗？

## deactivateChild

&emsp;deactivateChild 函数将入参 element 对象的 `_parent` 指向 null（原本是指向自己），将其移至一个全局的非活动元素列表（`_inactiveElements`），并将其 render object 从 render object tree 中分离。

&emsp;调用方负责将子元素从其子模型中移除。通常情况下，在对其子模型进行更新时，元素本身会调用 deactivateChild 方法；然而，在 GlobalKey 重新指定父级时，新父级会主动调用旧父级的 deactivateChild 方法，并首先使用 forgetChild 方法来使旧父级更新其子模型。

&emsp;调用方负责将入参 child Element 对象从其子树中移除（`_parent` 置为 null 即移除，一个 Element 对象加上它的 child，即一颗子树，或者一个 element 对象就可以理解为是一颗子树）。通常，deactivateChild 是由 Element 自身在更新其子树时调用的；然而，在 GlobalKey 重新父级化期间，新 Element 父级会主动调用原 Element 父级的 deactivateChild，首先使用 forgetChild 引起原 Element 父级更新其子树。

```dart
  @protected
  void deactivateChild(Element child) {
    // 断言：要保证调用此函数的 element 对象是入参 child 的父级。
    assert(child._parent == this);
    
    // 父级置为 null
    child._parent = null;
    // render object 从 render object tree 分离
    child.detachRenderObject();
    
    // 把 child 加入全局非活动对象列表中，同时内部 child 会调用 deactivate 函数
    owner!._inactiveElements.add(child); // this eventually calls child.deactivate()
  }
```

## `_activateWithParent`

&emsp;使用给定的父级 element 和 newSlot 激活当前 element 对象以及自己的子级。类似新 element 节点挂载的过程。

```dart
  void _activateWithParent(Element parent, Object? newSlot) {
    // 断言：element 对象当前要处于非活动状态
    assert(_lifecycleState == _ElementLifecycle.inactive);
    
    // 父级指向
    _parent = parent;
    // _owner 直接由父级 owner 赋值
    _owner = parent.owner;
    
    _updateDepth(_parent!.depth);
    _updateBuildScopeRecursively();
    
    // 递归激活自己以及所有的 child
    _activateRecursively(this);
    
    // 递归使 render object 都添加到 render object tree 中
    attachRenderObject(newSlot);
  }
```

## activate

&emsp;使当前 Element 对象从 "inactive" 生命周期状态过渡到 "active" 生命周期状态。

&emsp;当先前停用的 element 对象重新合并到 element tree 中时，framework 会调用此方法。framework 不会在 element 第一次变为活动状态（即从 "initial" 生命周期状态）时调用此方法。在那种情况下，framework 会调用 mount 方法。(例如从非活动列表中复用 element 节点)

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
    
    // 把父节点的 _inheritedElements 赋值给自己的。（原来如果是 element 对象自己调用 deactivate 的话，是不会把自己的 _parent 置为 null 的）
    // 当然如果是 element 对象调用自己的 deactivateChild 函数是指定的自己的子级失活的话，会把子级的 _parent 置为 null。
    // InheritedElement 重写了此函数，如果是 InheritedElement 对象调用 _updateInheritance 的话，是把自己添加到 _parent?._inheritedElements 中并赋值给自己，
    // 如果是其它类型的 Element 对象的话，则只是把 _parent?._inheritedElements 赋值给自己。
    _updateInheritance();
    
    // 把父节点的 _notificationTree 赋值给自己的。
    attachNotificationTree();
    
    // 如果 _dirty 为true，则会把此 element 对象添加到脏元素列表中，下一帧对它进行更新重建。
    if (_dirty) {
      owner!.scheduleBuildFor(this);
    }
    
    // 如果是 InheritedElement 对象的话，可能之前有依赖自己的 element 对象，
    // 如果有的话，也给自己打上脏标记，下一帧进行更新重建。
    //（实际执行内容与上面的 owner!.scheduleBuildFor(this); 一样，这里重写为 didChangeDependencies 是为了，
    // StatefulElement 的 state 的 didChangeDependencies 回调。）
    if (hadDependencies) {
      didChangeDependencies();
    }
  }
```

&emsp;过程类似 mount，当然不同点主要集中在可能要立即进行重建。

## deactivate

&emsp;使当前 Element 对象从 "active" 生命周期状态过渡到 "inactive" 生命周期状态。

&emsp;当先前处于活动状态的 element 被移动到非活动 element 列表时，framework 会调用此方法。在非活动状态下，element 将不会出现在屏幕上。element 对象只能保持在非活动状态直到当前动画帧结束。在动画帧结束时，如果 element 对象还没有被重新激活，framework 将 unmount 该 element 对象。

&emsp;这是由 deactivateChild 间接调用的。

&emsp;此方法的实现应该以调用继承方法结束，如 super.deactivate()。

```dart
  @mustCallSuper
  void deactivate() {
    if (_dependencies != null && _dependencies!.isNotEmpty) {
      for (final InheritedElement dependency in _dependencies!) {
        dependency.removeDependent(this);
      }
      
    // 出于方便起见，在这里我们实际上并不清除列表，即使它不再代表我们注册的内容。如果我们永远不会被重复使用，这都无所谓。但如果需要重新使用，我们会在 activate() 中清空列表。这样做的好处是让 Element 的 activate() 实现可以根据我们在这里是否有依赖关系来决定是否重建。
    }
    
    // 记录的从根节点到目前的所有 InheritedElement 置 null
    _inheritedElements = null;
    // 状态修改为 inactive
    _lifecycleState = _ElementLifecycle.inactive;
  }
```

## unmount

&emsp;使当前 Element 对象从 "inactive" 生命周期状态过渡到 "defunct" 生命周期状态。

&emsp;当 framework 确定一个非活动元素将永远不会被重新激活时调用。在每个动画帧结束时，framework 会对任何剩余的非活动 element 对象调用 unmount，阻止非活动 element 对象保持非活动状态超过一个动画帧。

&emsp;在调用此方法之后，element 对象将不会再次被添加到 element tree 中。

&emsp;此 element 对象持有的任何资源应在此时释放。例如，RenderObjectElement.unmount 调用 RenderObject.dispose 并将其对 renderObject 的引用置为 null。

&emsp;此方法的实现应该以调用继承方法结束，如 super.unmount()。

```dart
  @mustCallSuper
  void unmount() {
    // 断言：注意调用 unmount 的 element 对象是由 inactive 非活动状态过来的
    assert(_lifecycleState == _ElementLifecycle.inactive);
    
    // 如果有 global key，则要从全局 map 中移除：key -> element
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

## findRenderObject

&emsp;该 element 对象的当前 RenderObject。如果 element widget 是一个 RenderObjectWidget，则这是该 element 为自身创建的渲染对象。否则，这是第一个自己子节点 RenderObjectWidget 的 RenderObject。(如果当前 element 不是 RenderObjectElement 的话，会沿着 element tree 在自己的子树上找第一个 RenderObject)

&emsp;此方法只有在构建阶段完成后才会返回有效结果。因此，不能在构建方法中调用它。应该只从互动事件处理程序（例如手势回调）或布局或绘制回调中调用它。如果 State.mounted 返回 false，则调用该方法也是无效的。(调用此函数时需要，当前 element 节点处于 active 状态)

&emsp;如果 render object 是 RenderBox（这是常见情况），则可以从 size getter 中获取 render object 的大小。这只在布局阶段之后才有效，因此应该只从绘制回调或互动事件处理程序（例如手势回调）中进行检查。

&emsp;有关帧不同阶段的详细信息，可参阅 WidgetsBinding.drawFrame 中的讨论。

&emsp;理论上，调用此方法的成本相对较高（树的深度为 O(N)），但实际上通常很便宜，因为树通常有许多 render object，因此到最近的 render object 的距离通常很短。

```dart
  @override
  RenderObject? findRenderObject() {
    assert(() {
      if (_lifecycleState != _ElementLifecycle.active) {
        throw FlutterError.fromParts(<DiagnosticsNode>[
          ErrorSummary('Cannot get renderObject of inactive element.'),
          ErrorDescription(
            'In order for an element to have a valid renderObject, it must be '
            'active, which means it is part of the tree.\n'
            'Instead, this element is in the $_lifecycleState state.\n'
            'If you called this method from a State object, consider guarding '
            'it with State.mounted.',
          ),
          describeElement('The findRenderObject() method was called for the following element'),
        ]);
      }
      return true;
    }());
    
    return renderObject;
  }
```

&emsp;记住一点：如果自己没有 renderObject 的话，会继续往自己的子树上找，找到一个距离自己最近的。如果当前 element 对象不是 active 状态，则抛错。

## size

&emsp;获取 findRenderObject 返回的 RenderBox 的大小。

&emsp;此 getter 仅在布局阶段完成后才会返回有效结果。因此，从构建方法中调用此方法是无效的。应该在绘制回调或交互事件处理程序（例如手势回调）中调用它。

&emsp;如果 findRenderObject 实际返回的不是 RenderBox 的子类型的渲染对象（例如，RenderView），则此 getter 将在调试模式下引发异常，并在发布模式下返回 null。

&emsp;调用此 getter 理论上相对昂贵（在树的深度上为 O(N)），但在实践中通常很便宜，因为树通常具有许多渲染对象，因此距离最近的渲染对象通常很短。

```dart
    if (renderObject is RenderBox) {
      return renderObject.size;
    }
    return null;
```

## `_inheritedElements`

&emsp;记录从 element tree 根节点到目前节点的所有 InheritedElement 对象，起一个收集的作用，然后 element tree 上每个节点都会引用这个 PersistentHashMap，这样就方便了 InheritedElement 节点的子级找自己想要依赖的上级 InheritedElement 对象。

&emsp;保证了 O(1) 的时间复杂度就可以找到指定类型的 InheritedWidget 对象。

```dart
  PersistentHashMap<Type, InheritedElement>? _inheritedElements;
```

## `_dependencies`

&emsp;记录下当前 element 节点依赖了哪些 InheritedElement 对象（它们都是自己的上级 element 节点）。

```dart
  Set<InheritedElement>? _dependencies;
```

## `_hadUnsatisfiedDependencies`

&emsp;未知。

```dart
  bool _hadUnsatisfiedDependencies = false;
```

&emsp;下面是一组跟 InheritedWidget/InheritedElement 相关的函数，如果搞清楚了它们，你就搞清楚了，InheritedWidget 的依赖更新机制。

## doesDependOnInheritedElement

&emsp;如果之前使用 ancestor 调用过 dependOnInheritedElement，则返回 true。

&emsp;因为如果之前 ancestor 调用过 dependOnInheritedElement 的话，它会被收集在 `_dependencies` 中。ancestor 是当前 element 节点的上级，都是 InheritedElement 对象，因为是自己的上级，所以被称为：“祖先”。

```dart
  @protected
  bool doesDependOnInheritedElement(InheritedElement ancestor) =>
      _dependencies != null && _dependencies!.contains(ancestor);
```

## dependOnInheritedElement

&emsp;将此 element 节点（构建上下文，之所以被称为 构建上下文，是因为在 Widget 那一层，开发者自己手写的代码层，element 是以 BuildContext context 的形式传过来的，其实它就是 widget 对应的 element 对象）注册到祖先（祖先指自己的上级 element 节点，因为 InheritedElement 是沿着 element tree 往上查找的）中，以便当祖先的 Widget 更改时，将重新构建此构建上下文。(即我们在通过 of 函数找到最近的 InheritedWidget 数据时，这个时候呢，也会把当前这个 element 对象呢也注册到 InheritedElement 的依赖者中，那样，当 InheritedElement 更新时，也就可以通知所有的依赖者更新重建啦！)

&emsp;返回值是 InheritedElement 对应的 InheritedWidget 对象。

&emsp;通常不直接调用此方法。大多数应用程序应该使用 dependOnInheritedWidgetOfExactType，找到适当的 InheritedElement 祖先后调用此方法。(即此方法呢，我们自己调用不到，它是给 dependOnInheritedWidgetOfExactType 函数调用的)

&emsp;有关何时可以调用 dependOnInheritedWidgetOfExactType 的所有资格也适用于此方法。（dependOnInheritedWidgetOfExactType 函数大家比较熟悉了，自定义 InheritedWidget 子类时，重写它的 of 函数时，内部要用这个 dependOnInheritedWidgetOfExactType 函数。）

&emsp;dependOnInheritedElement 函数内部做的事情呢，也特别清晰：

1. 找到当前 element 对象想要依赖的 InheritedElement 对象以后呢，就把它记录到自己的 `_dependencies` 字段中，记录下自己依赖了哪些 InheritedElement.
2. 而这个 InheritedElement 对象呢，也会把我们当前的 element 对象添加到自己的 `_dependents` 字段中，记录下哪些 element 对象依赖了自己，即记录下这些依赖者，方便后续更新重建。 

&emsp;然后其实还有个疑惑点：这个可 null 的 Object? aspect 参数是干啥用的，也暂时没见到哪里用到它了，后续我们研究一下。

```dart
  @override
  InheritedWidget dependOnInheritedElement(InheritedElement ancestor, { Object? aspect }) {
    // 找到当前 element 对象想要依赖的 InheritedElement 对象了以后呢，就把它记录到自己的 _dependencies 字段中
    _dependencies ??= HashSet<InheritedElement>();
    _dependencies!.add(ancestor);
    
    // 同时呢，InheritedElement 对象也记录下这些依赖自己的 element 对象。
    ancestor.updateDependencies(this, aspect);
    
    // 然后返回 InheritedElement 对象的 widget
    return ancestor.widget as InheritedWidget;
  }
```

## dependOnInheritedWidgetOfExactType

&emsp;dependOnInheritedWidgetOfExactType 函数我们应该挺熟悉的，在我们自定义 InheritedWidget 子类时，重写它的 of 函数时，内部必要用这个函数。下面看一下它的官方文档：

&emsp;返回给定类型 T 的最近的 widget，并在其上创建一个依赖（即把当前的 element 对象作为依赖者被它记录下来），如果找不到合适的 widget 则返回 null。

&emsp;找到的 widget 将是一个具体的 InheritedWidget 子类，调用 dependOnInheritedWidgetOfExactType 会将此构建上下文（element 对象）注册到返回的 widget 上。当该 widget 改变（或者引入了新的该类型的 widget，或者该 widget 被移除），此构建上下文将重新构建（此 element 对象会被标记为脏并进行新建），以便从该 widget 获取新值。

&emsp;通常会隐式地从 of() 静态方法中调用，例如 Theme.of。

&emsp;不应该在 widget 构造函数或 State.initState 方法中调用此方法，因为如果继承的值发生更改，这些方法将不会再次被调用。为确保在继承值更改时 widget 正确更新自身，只应该从构建方法、布局和绘制回调中直接或间接地调用此方法，或者从 State.didChangeDependencies（在 State.initState 之后立即调用）中调用。

&emsp;不应该从 State.dispose 中调用此方法，因为在那个时候 Element tree 不再稳定。要在该方法中引用祖先，请在 State.didChangeDependencies 中保存对祖先的引用。从 State.deactivate 中使用此方法是安全的，每当 widget 从树中移除时都会调用该方法。

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
    // 并且 _inheritedElements 在整个 Eelment tree 每个节点传递，所以这里呢，O(1) 的时间复杂度，就可以找到对应的 InheritedElement 对象。
    final InheritedElement? ancestor = _inheritedElements == null ? null : _inheritedElements![T];
    
    // 如果找到了对应类型的 InheritedElement 对象就直接调用上面的 dependOnInheritedElement 函数就好啦！
    if (ancestor != null) {
      return dependOnInheritedElement(ancestor, aspect: aspect) as T;
    }
    
    _hadUnsatisfiedDependencies = true;
    return null;
  }
```

## getInheritedWidgetOfExactType

&emsp;对 getElementForInheritedWidgetOfExactType 的简单封装。

&emsp;返回给定的 InheritedWidget 子类 T 的最近的 widget，如果找不到适当的祖先，则返回 null。

&emsp;**这个方法不像更典型的 dependOnInheritedWidgetOfExactType 那样引入依赖关系，所以如果 InheritedWidget 发生变化，这个上下文不会重新构建。这个功能适用于那些不常见的情况，其中不希望有依赖关系。**

&emsp;这个方法不应该从 State.dispose 中调用，因为此时元素树已不稳定。要在该方法中引用祖先，请在 State.didChangeDependencies 中保存对祖先的引用。可以安全地从 State.deactivate 中使用这个方法，该方法在 widget 从树中移除时调用。

&emsp;还可以从交互事件处理程序（例如手势回调）或计时器中调用这个方法，以获取值一次，只要那个值不被缓存或以后重复使用。

&emsp;调用这个方法的时间复杂度是 O(1)，有一个小的常数因子。

```dart
  @override
  T? getInheritedWidgetOfExactType<T extends InheritedWidget>() {
    return getElementForInheritedWidgetOfExactType<T>()?.widget as T?;
  }
```

## getElementForInheritedWidgetOfExactType

&emsp;看代码仿佛是只做了 dependOnInheritedWidgetOfExactType 函数一半的事情，从 `_inheritedElements` 读取一下数据就返回了，不包括任何依赖关系注册。

&emsp;返回与给定的 InheritedWidget 子类 T 的最近的 widget 相对应的 element 对象，该类型必须是一个具体的 InheritedWidget 子类。如果找不到这样的 element，则返回 null。

&emsp;调用此方法的时间复杂度为 O(1)，且有一个很小的常量因子。

&emsp;这个方法不会像 dependOnInheritedWidgetOfExactType 一样在目标上建立依赖关系。

&emsp;这个方法不应该从 State.dispose 中调用，因为此时元素树已不稳定。要从这个方法引用祖先，需要在 State.didChangeDependencies 中调用 dependOnInheritedWidgetOfExactType 来保存对祖先的引用。可以在 State.deactivate 中使用这个方法，因为每当小部件从树中移除时会调用 State.deactivate。

```dart
  @override
  InheritedElement? getElementForInheritedWidgetOfExactType<T extends InheritedWidget>() {
    assert(_debugCheckStateIsActiveForAncestorLookup());
    final InheritedElement? ancestor = _inheritedElements == null ? null : _inheritedElements![T];
    return ancestor;
  }
```

## attachNotificationTree

&emsp;在 Element.mount 和 Element.activate 中被调用，将该元素注册到通知树中。

&emsp;仅暴露此方法是为了能够实现 NotifiableElementMixin。希望响应通知的 Element 子类应该使用混入方式来实现。

&emsp;另请参见：NotificationListener，一个允许监听通知的小部件。后续我们进入深入研究。

&emsp;目前看只是简单的像其它字段一样，直接把父级的 `_notificationTree` 字段赋值给自己。

```dart
  @protected
  void attachNotificationTree() {
    _notificationTree = _parent?._notificationTree;
  }
```

## `_updateInheritance`

&emsp;在 Element.mount 和 Element.activate 中都会调用此函数。

&emsp;在整个 element tree 中，每个 element 节点都引用同一个：`_inheritedElements`。同时还有一个特别重要的点：InheritedElement 重写了此函数，如果是 InheritedElement 对象调用 `_updateInheritance` 的话，是把自己添加到 `_parent?._inheritedElements` 中并赋值给自己，如果是其它类型的 Element 对象的话，则只是把 `_parent?._inheritedElements` 赋值给自己。

```dart
  void _updateInheritance() {
    assert(_lifecycleState == _ElementLifecycle.active);
    _inheritedElements = _parent?._inheritedElements;
  }
```

&emsp;OK，下面是一组跟 "Ancestor" 相关的函数，首先我们可以先对 Ancestor 词有一个心理建设，它代表的意思其实很简单：就是当前 element 节点的上级，我们可以沿着 `_parent` 往链表上级走，也就是沿着 element 的双向链表往上走，往上的那些节点，便可以被称为 "祖先"。

&emsp;然后下面一组函数便是找祖先：找指定类型的 Widget、找指定类型的 State、找根 State、找指定类型的 RenderObject。

## findAncestorWidgetOfExactType

&emsp;返回给定类型 T 的最近的祖先 Widget，该类型必须是一个具体的 Widget 子类的类型。代码也超级简单，沿着自己的 `_parent` 往上查找即可。

&emsp;一般来说，dependOnInheritedWidgetOfExactType 更有用，因为 InheritedWidget 在更改时会触发依赖者重建。当用于交互事件处理程序（例如手势回调）或执行一次性任务（例如断言你是否具有特定类型的 widget 作为祖先）时，此方法是合适的。Widget 的 build 方法的返回值不应该依赖于此方法返回的值，因为如果此方法的返回值发生更改，那么构建上下文(Build Context)将不会重新构建。这可能导致 build 方法中使用的数据发生更改，但 widget 却没有重新构建(rebuild)。

&emsp;调用此方法比较昂贵（树深度为 O(N)）。只有在从此 Widget 到所需祖先之间的距离已知为较小且有界时才调用此方法。(毕竟它是沿着 element 链往上一个一个查的，复杂页面的 element 的 depth 都是很大的。)

&emsp;在 State.deactivate 或 State.dispose 中不应调用此方法，因为在此时 element tree 不再稳定。要在这些方法中引用祖先，请通过在 State.didChangeDependencies 中调用 findAncestorWidgetOfExactType 来保存对祖先的引用。

&emsp;如果在此上下文的祖先中没有出现请求类型的 widget，则返回 null。

```dart
  @override
  T? findAncestorWidgetOfExactType<T extends Widget>() {
    Element? ancestor = _parent;
    
    // 如果 runtimeType 类型不匹配，则沿着 _parent 链继续往上找即可
    while (ancestor != null && ancestor.widget.runtimeType != T) {
      ancestor = ancestor._parent;
    }
    
    return ancestor?.widget as T?;
  }
```

## findAncestorStateOfType

&emsp;返回给定类型为 T 实例的最近祖先 StatefulWidget widget 的 State 对象。

&emsp;不应该在 build 方法中使用该方法，因为如果此方法返回的值发生更改，则构建上下文不会重新构建。通常情况下，对于这种情况，更适合使用 dependOnInheritedWidgetOfExactType。该方法对于以一次性方式更改祖先 widget 的状态很有用，例如，使祖先滚动列表滚动到此构建上下文的 widget 中，或者响应用户交互移动焦点。

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

## findRootAncestorStateOfType

&emsp;返回在给定类型 T 的实例中，是 StatefulWidget widget 的最远祖先的 State 对象。

&emsp;与 findAncestorStateOfType 的功能相同，但会继续访问后续祖先，直到没有剩余类型为 T 实例的元素。然后返回找到的最后一个。

&emsp;该操作的时间复杂度也是 O(N)，N 是整个 widget 树，而不是子树。

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
      
      ancestor = ancestor._parent;
    }
    
    return statefulAncestor?.state as T?;
  }
```

## findAncestorRenderObjectOfType

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

## visitAncestorElements

&emsp;从此构建上下文的 widget 的父级开始逐级遍历祖先，对每个祖先调用参数。

&emsp;回调函数将获得对应祖先 widget 的 Element 对象的引用。当遍历到根 widget 或者回调函数返回 false 时，遍历将停止。回调函数不能返回 null。

&emsp;这对于检查 widget 树非常有用。

&emsp;调用此方法相对较昂贵（在树的深度为 N 时为 O(N)）。

&emsp;不应该在 State.deactivate 或 State.dispose 中调用此方法，因为在那时 element tree 已不再稳定。要在这些方法中引用祖先，可以通过在 State.didChangeDependencies 中调用 visitAncestorElements 保存对祖先的引用。

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

## didChangeDependencies

&emsp;当此 element 对象的一个依赖项（即此 element 对象依赖的 InheritedElement）发生变化时调用。

&emsp;dependOnInheritedWidgetOfExactType 将此 element 对象注册为依赖于给定类型的 InheritedElement 对象。当树中的此位置的该类型信息发生变化时（例如，因为 InheritedElement 更新为新的 InheritedWidget 且 InheritedWidget.updateShouldNotify 返回 true），framework 会调用此函数来通知此 element 对象发生变化。

&emsp;实现内容也很简单，把当前 element 对象标记为重建即可。

```dart
  @mustCallSuper
  void didChangeDependencies() {
    markNeedsBuild();
  }
```

## dirty

&emsp;如果 element 对象已被标记为需要重建，则 dirty getter 返回 true。

&emsp;当 element 对象首次创建以及在调用 markNeedsBuild 后，该标志为 true。该标志在 performRebuild 实现中被重置为 false。

&emsp;注意在 element 对象首次创建后此 dirty 标识也会置为 true。当 element 对象调用 performRebuild 函数开始执行重建时会把此标识置为 false。

```dart
  bool get dirty => _dirty;
  bool _dirty = true;
```

&emsp;OK，下面还有三个超重要的函数：markNeedsBuild、rebuild、performRebuild 三个超重要的函数，看完以后 Element 类就可以完结了，继续加油！

## markNeedsBuild

&emsp;将该 element 对象标记为脏元素，并将其添加到下一帧中需要重建的全局 Widget 列表中。

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
    
    // 把当前 element 对象添加到全局的：final List<Element> _dirtyElements = <Element>[]; 脏元素列表中，等待下一帧一起重建
    owner!.scheduleBuildFor(this);
  }
```

## rebuild

&emsp;简单概括 rebuild 的话，进行一些判断然后实际调用 performRebuild 进行重建，并且 Element 的一众子类都没有重写这个 rebuild 函数，所以实际那些 element 对象还是都会调用到这里。

&emsp;rebuild 调用会使 widget 进行更新。在调试构建中，还会验证各种不变性。

+ 当 BuildOwner.scheduleBuildFor 被调用标记此 element 对象为脏时，由 BuildOwner 调用此方法；
+ 在第一次构建元素时由 mount 调用；
+ 当小部件发生更改时由 update 调用；

&emsp;该方法仅在 dirty 为 true 时进行重建。若要无论 dirty 标志如何都重新构建，请将 force 设置为 true。在 update 过程中，dirty 为 false 时强制重新构建会很方便。

### When rebuilds happen

#### Terminology

&emsp;Widget 代表 Element 的配置。每个 Element 都有一个 Widget，存储在 Element.widget 字段中。术语 "widget" 通常在严格意义上更正确的时候使用，而应该使用 "element"。

&emsp;虽然一个 Element 具有当前 Widget，但是随着时间的推移，该 Widget 可能被其他 Widget 替换。例如，支持 ColoredBox 的 Element 可能首先有一个其 Widget 为 ColoredBox，其 ColoredBox.color 为蓝色，然后稍后将被赋予一个新的 ColoredBox，其颜色为绿色。

&emsp;在任何特定时间点，同一树中的多个 Element 可能具有相同的 Widget。例如，具有绿色的相同 ColoredBox 可能在 Widget 树中的多个位置同时使用，每个位置由不同的 Element 支持。

#### Marking an element dirty

&emsp;在帧之间，一个 Element 可能会被标记为脏。这可能是由于各种原因造成的，包括以下情况：

+ StatefulWidget 的 State 可能通过调用 State.setState 方法导致其 Element 被标记为脏。
+ 当 InheritedWidget 发生改变时，之前订阅它的 Element 会被标记为脏。
+ 在热重载期间，每个元素都会被标记为脏（使用 Element.reassemble）。

#### Rebuilding

&emsp;在下一帧中重新构建脏 Element。这是如何做到的确取决于 Element 的类型。

+ StatelessElement 通过使用其 widget 的 StatelessWidget.build 方法进行重建。
+ StatefulElement 通过使用其 widget 的 state 的 State.build 方法进行重建。
+ RenderObjectElement 通过更新其 RenderObject 进行重建。

&emsp;在许多情况下，重建的最终结果是单个子 widget 或者（对于 MultiChildRenderObjectElements）一组子 widget。

&emsp;这些子 widget 用于更新 Element 的子 element 的 widget 属性。如果新的 Widget 具有相同的类型和 key，则认为它对应于现有的 Element。（对于 MultiChildRenderObjectElements，即使顺序发生变化，也会努力跟踪 widget；请参阅 RenderObjectElement.updateChildren。）

&emsp;如果之前没有对应的子 element，那么将创建一个新的 Element（使用 Widget.createElement）；然后递归构建该 Element。(注意这里的递归，整个子树构建完成才会结束。)

&emsp;如果以前已经有一个子 element，但构建未提供相应的 Widget 以更新它，则旧子 element 将被丢弃（或者，在涉及 GlobalKey 重新添加到 element tree 中的情况下，被重新使用在其他地方。）。

&emsp;然而，最常见的情况是有一个对应的先前子 element。这是通过要求子 element 使用新的子 widget 更新自己来处理的。对于 StatefulElement，这就是触发 State.didUpdateWidget 的情况。

#### Not rebuilding

&emsp;在 Element 被告知使用新的 Widget 更新自己之前，会使用 operator == 来比较新旧 Widget 对象。

&emsp;通常情况下，这相当于使用 identical 进行比较，以查看这两个 Widget 对象是否确实是完全相同的实例。如果它们是相同的实例，并且 Element 并没有因为其他原因已被标记为需要更新，则 Element 将跳过更新自身，因为它可以确信更新自身或其子级将没有任何价值。(const 构造函数发挥作用的地方！)

&emsp;强烈建议避免在 Widget 对象上覆盖 operator ==。虽然这样做可能会改善性能，但实际上，对于非叶子 Widget，这会导致 O(N²) 的行为。这是因为必要时比较将包括比较子 Widget，并且如果这些子 Widget 也实现了 operator ==，则最终将导致对 Widget 树的完全遍历……然后在树的每个级别上都重复这个操作。实践中，重新构建更加廉价。（此外，如果应用程序中使用的 Widget 的任何子类实现了 operator ==，那么编译器无法在任何地方内联比较，因为必须将调用视为虚拟的，以防实例碰巧是重写了 operator 的实例。）

&emsp;相反，避免不必要的重构的最佳方式是缓存从 State.build 返回的小部件，以便每帧使用相同的小部件，直到它们发生变化。存在多种机制来鼓励这样做：例如，const widgets 是一种形式的自动缓存（如果使用 const 关键字构造小部件，则每次使用相同参数构建时都返回同一实例）。

&emsp;另一个例子是 AnimatedBuilder.child 属性，它允许子树中非动画部分保持静态，即使 AnimatedBuilder.builder 回调重新创建其他组件。(这个后续要深入研究一下)

```dart
  @pragma('dart2js:tryInline')
  @pragma('vm:prefer-inline')
  @pragma('wasm:prefer-inline')
  void rebuild({bool force = false}) {
    // 1. 如果当前 element 对象的状态不是 active 2. 或者非 _dirty 且 非 force
    // 直接 return。
    if (_lifecycleState != _ElementLifecycle.active || (!_dirty && !force)) {
      return;
    }
    
    try {
      // 最重要的内容，实际执行重建，调用：performRebuild 函数
      performRebuild();
    } finally {
    }
  }
```

## performRebuild

&emsp;在适当的检查之后，使 widget 更新自身。在 rebuild 被调用后调用。Element 基本实现只是清除了 dirty 标志。不同的 Element 子类都对其进行了重写。

```dart
  @protected
  @mustCallSuper
  void performRebuild() {
    _dirty = false;
  }
```

# Element 总结

&emsp;







## 参考链接
**参考链接:🔗**
+ [Element class](https://api.flutter.dev/flutter/widgets/Element-class.html)
+ [kFlutterMemoryAllocationsEnabled top-level constant](https://api.flutter.dev/flutter/foundation/kFlutterMemoryAllocationsEnabled-constant.html)
+ [Flutter | Understanding the MemoryAllocations](https://medium.com/@maciejbrzezinski/flutter-what-is-memoryallocations-1ee2eb0a8670)
+ [Flutter | How to create a custom binding and inject your own BuildOwner](https://medium.com/@maciejbrzezinski/flutter-how-to-create-a-custom-binding-and-inject-your-own-buildowner-f60ef320537b)
+ [Support for WebAssembly (Wasm)](https://docs.flutter.dev/platform-integration/web/wasm)
