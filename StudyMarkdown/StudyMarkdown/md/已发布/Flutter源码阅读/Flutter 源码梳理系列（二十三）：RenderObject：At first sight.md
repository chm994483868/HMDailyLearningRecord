# Flutter 源码梳理系列（二十三）：RenderObject：At first sight 

&emsp;在前面的学习过程中，我们学习了 Widget Tree、Element Tree、Notification Tree，现在我们要学习另一个树🌲了：Render Tree。

&emsp;已知 Widget Tree 中每个节点就是一个 Widget 对象，Element Tree 中每个节点就是一个 Element 对象，Notification Tree 中每个节点就是一个 NotificationNode 对象，而到了 Render Tree，它的每个节点就是一个 RenderObject 对象。那么既然如此，我们就先找一下 Render Tree 的根节点吧？

&emsp;还记得我们前面学习 RenderObjectElement.mount 函数时见到的：RenderObjectWidget.createRenderObject 函数的调用吗？那里便是 RenderObject 对象出生的地方。

```dart
_renderObject = (widget as RenderObjectWidget).createRenderObject(this);
```

&emsp;然后聚焦到 RenderObjectElement.mount 函数第一行依然是：super.mount(parent, newSlot)，即直接调用 Element.mount 函数。然后是针对 RenderObjectElement 的情况创建对应的 RenderObject 对象并附加到祖先 RenderObjectElement 对应的 RenderObject 上，来一层一层的构建 Render Tree。

&emsp;这种通过重写父类的函数，来添加本类的自定义操作的方式，我们在前面遇到过很多次。例如：我们学习 InheritedElement 时它重写 `Element._updateInheritance` 函数把自己添加到 `_inheritedElements` 中，其它非 InheritedElement Element 的 `_updateInheritance` 函数只是传递 `_inheritedElements` 的值。

&emsp;还有 NotifiableElementMixin 重写 Element.attachNotificationTree 函数构建 NotificationNode 节点，而其它非 NotifiableElementMixin Element 的 attachNotificationTree 函数只是传递 `_notificationTree` 的值。

&emsp;当 Element 节点的同样的阶段发生在不同类型的 Element 上时，子类 Element 可以通过重写父类的方法来添加自己的自定义的行为。这次则是到了 RenderObjectElement 以及其子类时，当它们把自身挂载到 Element Tree 上以后，它们会继续创建自己的 RenderObject 对象，然后把此 RenderObject 对象附加到 Render Tree 上去。 

&emsp;本篇呢，先简单对 RenderObject 进行一个预热，毕竟它涉及的内容太多了，我们直接去看的话不好消化，我们慢慢来。下面先看一下 Render Tree 的根节点在哪，然后再看 RenderObject 附加到 Render Tree 上时涉及到的一些函数，然后后续再一行一行的去看 RenderObject 的源码。

&emsp;首先我们去找 Render Tree 的根节点，这个过程也可以加深我们对 Flutter 项目的启动流程的理解。

# Entry

&emsp;首先我们直接在 RenderObjectElement.mount 函数的 `_renderObject = (widget as RenderObjectWidget).createRenderObject(this);` 行加一个断点，然后我们以 Debug 模式启动项目，此时会直接命中断点，然后我们点击：`Step Into` 按钮进入 createRenderObject 函数内部去，看看这第一个调用 createRenderObject 函数的 RenderObjectElement 会是谁。

&emsp;看到它跳转到了 `_RawViewInternal.createRenderObject` 函数：

```dart
class _RawViewInternal extends RenderObjectWidget {
  // ...
  @override
  RenderObject createRenderObject(BuildContext context) {
    return _deprecatedRenderView ?? RenderView(
      view: view,
    );
  }
  // ...
}
```

&emsp;可以看到第一个 RenderObject 对象正是这个命名为：`_deprecatedRenderView` 的变量。（它的名字叫 "弃用 RenderView"，它是在 v3.10.0-12.0.pre 之后被弃用，当前项目是 3.6.0-36.0.dev。）。

&emsp;`_deprecatedRenderView` 作为 `_RawViewInternal` 的一个属性，既然在 App 刚启动的时候就是有值的，那么既然这样，我们就直接在 `_RawViewInternal` 的构造函数处打一个断点，看一下它的属性：`_deprecatedRenderView` 值从哪来的。然后我们重新启动项目，命中断点，看到是在 RawView.build 函数内创建了 `_RawViewInternal` 对象并返回。

&emsp;本来想这样沿着断点查看堆栈，反向推导 Render Tree 根节点的创建过程的，但是其实不用这样。前面断点命中：`_renderObject = (widget as RenderObjectWidget).createRenderObject(this);` 时，其实在 RenderObjectElement.mount 函数中看到 this 指针指向的是：`_RawViewElement`，然后我们直接右键看它的类型定义，可看到：`class _RawViewElement extends RenderTreeRootElement { // ...}`，它是 RenderObjectElement 的子类，然后 this 的 depth 是 4，即 Element Tree 上第 4 个 Element 节点便是 Render Tree 根节点对应的 Element 了。

&emsp;那么既然如此浅的深度，我们索性从 runApp 函数直接找这个 Render Tree 的根节点。

&emsp;有了前面 Widget 和 Element 的基础。我们知道 Widget 才是 Element 的源头，所以我们只要沿着 Widget Tree 找即可，遇到 StatelessWidget 的话直接看它的 build 函数返回的 Widget，遇到 StatefulWidget 的话直接看它的 State 的 build 函数返回的 Widget。只要我们找到第一个 RenderObjectWidget（或者其子类）便能找到第一个 RenderObjectElement（或者其子类），然后它的 renderObject 属性便就是 Render Tree 的根节点了。 

# runApp

&emsp;runApp 的 Widget app 参数即我们传给它的 Widget 对象，看到它被：`binding.wrapWithDefaultView(app)` 调用，那么我们直接去看这个 wrapWithDefaultView 函数。

```dart
void runApp(Widget app) {
  final WidgetsBinding binding = WidgetsFlutterBinding.ensureInitialized();
  _runWidget(binding.wrapWithDefaultView(app), binding, 'runApp');
}
```

# wrapWithDefaultView

&emsp;看到它直接返回了一个 View 对象，它是一个 StatefulWidget 类型的 Widget。然后它有两个参数：`pipelineOwner` 和 `renderView`，它们两个变量来自 RendererBinding：

```dart
  Widget wrapWithDefaultView(Widget rootWidget) {
    return View(
      view: platformDispatcher.implicitView!,
      deprecatedDoNotUseWillBeRemovedWithoutNoticePipelineOwner: pipelineOwner, // ⬅️⚠️
      deprecatedDoNotUseWillBeRemovedWithoutNoticeRenderView: renderView, // ⬅️⚠️
      child: rootWidget,
    );
  }
```

&emsp;RendererBinding 同 runApp 中的 WidgetsBinding 一样，也是有一个全局的单例对象。然后它的 `pipelineOwner` 和 `renderView` 属性都是有默认值的两个 late 变量。其实到这里就可以结束了，它们两个分别是 Render Tree 上所有节点指向的 `PipelineOwner? _owner` 和 Render Tree 的根节点。

```dart
  late final PipelineOwner pipelineOwner = PipelineOwner(
    onSemanticsOwnerCreated: () {
      (pipelineOwner.rootNode as RenderView?)?.scheduleInitialSemantics();
    },
    onSemanticsUpdate: (ui.SemanticsUpdate update) {
      (pipelineOwner.rootNode as RenderView?)?.updateSemantics(update);
    },
    onSemanticsOwnerDisposed: () {
      (pipelineOwner.rootNode as RenderView?)?.clearSemantics();
    }
  );
  
  late final RenderView renderView = _ReusableRenderView(
    view: platformDispatcher.implicitView!,
  );
```

&emsp;既然 `pipelineOwner` 和 `renderView` 被传到了 View 中，那么我们继续往 View 中看。看一看 View 的下级 Widget 是谁。

# View

&emsp;既然 View 是 StatefulWidget，那么我们直接看它的 State 的 build 函数即可，看到内部是返回一个 RawView 对象。并且这个 RawView 对象创建的时候把 `widget._deprecatedPipelineOwner` 和 `widget._deprecatedRenderView` 传递下去了。

&emsp;那么我们继续往 RawView 里面去看，它是一个 StatelessWidget。

```dart
  @override
  Widget build(BuildContext context) {
    return RawView(
      view: widget.view,
      deprecatedDoNotUseWillBeRemovedWithoutNoticePipelineOwner: widget._deprecatedPipelineOwner, // ⬅️⚠️
      deprecatedDoNotUseWillBeRemovedWithoutNoticeRenderView: widget._deprecatedRenderView, // ⬅️⚠️
      child: MediaQuery.fromView(
        view: widget.view,
        child: FocusTraversalGroup(
          policy: _policy,
          child: FocusScope.withExternalFocusNode(
            includeSemantics: false,
            focusScopeNode: _scopeNode,
            child: widget.child,
          ),
        ),
      ),
    );
  }
```

# RawView

&emsp;既然 RawView 是 StatelessWidget，那么我们直接看它的 build 函数即可，果然这里它直接返回了一个 `_RawViewInternal` 对象，并且在创建 `_RawViewInternal` 时把 `_deprecatedPipelineOwner` 和 `_deprecatedRenderView` 传递过去了。

&esmp;至此加上我们最前面没有提到的 RootWidget，其实到这里刚好是 4 级 Widget 了：1️⃣：RootWidget -> 2️⃣：View -> 3️⃣：RawView -> 4️⃣：`_RawViewInternal`，至此 4 级 Widget 对应的 depth 是 4 的 Element 也到了。

```dart
  @override
  Widget build(BuildContext context) {
    return _RawViewInternal(
      view: view,
      deprecatedPipelineOwner: _deprecatedPipelineOwner, // ⬅️⚠️
      deprecatedRenderView: _deprecatedRenderView, // ⬅️⚠️
      builder: (BuildContext context, PipelineOwner owner) {
        return _ViewScope(
          view: view,
          child: _PipelineOwnerScope(
            pipelineOwner: owner,
            child: child,
          ),
        );
      },
    );
  }
```

&emsp;下面我们重点看 `_RawViewInternal`，它便是 Render Tree 根节点对应的 Widget。

# `_RawViewInternal`

&emsp;`_RawViewInternal` 是 RenderObjectWidget 的直接子类：`class _RawViewInternal extends RenderObjectWidget { // ...}`，它实现了 `createRenderObject` 函数，直接返回：`_deprecatedRenderView`，即从前面一路传递来的：RendererBinding 的 renderView 的属性。那么看到这里的话，找到根 RenderObject 了，那么下面看一下它是如何附加的。（或者是如何把它作为 Render tree 的根节点的。）

&emsp;既然是要找 RenderObject 附加，那便还是先找 Element.mount 挂载。`_RawViewInternal` 的 createElement 函数，看到它是返回一个 `_RawViewElement` 对象，`_RawViewElement` 是 RenderTreeRootElement 的直接子类：`class _RawViewElement extends RenderTreeRootElement { // ...}`，RenderTreeRootElement 是 RenderObjectElement 的直接抽象子类，并且它重写了 RenderObjectElement.attachRenderObject 函数，内部仅仅是一个 slot 赋值。这也可以理解，作为 Render Tree 的根节点对应的 Element，它是不需要做像 RenderObjectElement.attachRenderObject 里面的那些花里胡哨的事情的。

&emsp;RenderTreeRootElement 是一个用于管理 Render Tree 根部的 RenderObjectElement。与任何其他 RenderObjectElement 不同，它不尝试将其 renderObject 附加到最近的祖先 RenderObjectElement。相反，子类必须重写 attachRenderObject 和 detachRenderObject 方法，将 renderObject 附加/分离到管理 Render Tree 的任何实例（例如，通过将其分配给 PipelineOwner.rootNode）。然后其实是把这个任务交给了它的子类 `_RawViewElement`。

```dart
class _RawViewInternal extends RenderObjectWidget {
  // ...
  
  // 创建它对应的 Element
  @override
  RenderObjectElement createElement() => _RawViewElement(this);
  
  // 创建它对应的 RenerOjbect
  @override
  RenderObject createRenderObject(BuildContext context) {
    return _deprecatedRenderView ?? RenderView(
      view: view,
    );
  }
  // ...
}

// RenderTreeRootElement 内部很简单，全部是靠 _RawViewElement 完成本来属于 RenderTreeRootElement 的工作，
// _RawViewElement 直接继承自 RenderTreeRootElement，为它完成很多累活，
// 最重要的则是将 Render Tree 的根 RenderObject 对象附加到管理 Render Tree 的实例（PipelineOwner），即 PipelineOwner.rootNode 属性。
class _RawViewElement extends RenderTreeRootElement {
  // ...
}
```

&emsp;上面提到的 RenderTreeRootElement："它不尝试将其 renderObject 附加到最近的祖先 RenderObjectElement。相反，子类（`_RawViewElement`）必须重写 attachRenderObject 和 detachRenderObject 方法，..."，对应到代码里便是它重写 RenderObjectElement 的 attachRenderObject 和 detachRenderObject 方法去掉里面的复杂操作。

```dart
abstract class RenderTreeRootElement extends RenderObjectElement {
  // ...
  
  // 重写 RenderObjectElement 的 attachRenderObject 函数，仅保留一个 slot 赋值
  @override
  @mustCallSuper
  void attachRenderObject(Object? newSlot) {
    _slot = newSlot;
  }
  
  // 重写 RenderObjectElement 的 detachRenderObject 函数，仅保留一个 slot 置 null
  @override
  @mustCallSuper
  void detachRenderObject() {
    _slot = null;
  }
  // ...
}
```

&emsp;那么下面我们看一下如何把 Render Tree 的根节点附加到 PipelineOwner.rootNode 的，那要找的入口必然是根节点对应的 Element 对象执行挂载，然后再处理根节点。 

# `_RawViewElement.mount`

&emsp;然后我们直接看 `_RawViewElement` 的 mount 函数：

```dart
  @override
  void mount(Element? parent, Object? newSlot) {
    // 直接调用 RenderObjectElement.mount 函数，
    // 但是 RenderObjectElement.mount 内部的 attachRenderObject 函数调用，
    // 调用的则是 RenderTreeRootElement.attachRenderObject 函数，即只进行简单的 slot 赋值。
    super.mount(parent, newSlot); 
    
    // 这里则是直接把 RendererBinding.renderView 赋值给 RendererBinding.pipelineOwner 的 rootNode 属性。
    
    // 由于这里是一个赋值操作，所以还会调用 PiplineOwner 的 rootNode Setter，
    // 在 rootNode 的 Setter 里面有一个极重要的 `_rootNode?.attach(this);` 函数调用，
    // 内部会实现把 RendererBinding.pipelineOwner 赋值给 RendererBinding.renderView 的 PipelineOwner? _owner 属性 
    _effectivePipelineOwner.rootNode = renderObject;
    
    // 下面牵涉到逻辑较深，我们后面再进行研究。
    // 主要牵涉：RendererBinding、PipelineOwner、以及 layout 和 paint。 
    _attachView();
    
    // 和其它 Element 一样，
    // 这里继续回归到 Element Tree 的构建，当前 Element 对象挂载完成后，继续找到子级 Widget 进行 Element 子树构建
    _updateChild();
    
    renderObject.prepareInitialFrame();
    if (_effectivePipelineOwner.semanticsOwner != null) {
      renderObject.scheduleInitialSemantics();
    }
  }
```

&emsp;下面是 PipelineOwner 的 rootNode Setter，完成 RenderObject 的 `PipelineOwner? _owner` 属性赋值。

&emsp;看到这里的话其实有一个变化了，在之前我们学习 Widget 和 Element 时几乎完全没有用到 Setter，但是到了 RenderObject 这里我们要注意 Setter 的使用，有时候我们分析函数调用连续不上时，可以仔细看一下属性的赋值，看有没有添加 Setter，会在它里面添加函数调用，例如下面二处：

+ PipelineOwner rootNode Setter
+ RenderObjectWithChildMixin child Setter

&emsp;在这两个 Setter 内部都有极重要的流程函数的调用。例如下面的 rootNode Setter 里面调用 attach 函数。

```dart
  set rootNode(RenderObject? value) {
    if (_rootNode == value) {
      return;
    }
    
    // 首先进行一个清理释放之前的引用后，再进行新的赋值。
    // 内部时把之前的旧 _rootNode 引用的 owner 置为 null，
    // 即之前的旧 _rootNode 不再引用当前的 PipelineOwner 对象了。
    _rootNode?.detach();
    
    // 这里便是把我们的 Render Tree 的根节点赋值给 PipelineOwner 了。
    _rootNode = value;
    
    // 最终调用到 RenderObject.attach 函数，
    // 把此 RendererBinding.pipelineOwner 对象赋值给 RendererBinding.renderView 的 PipelineOwner? _owner 属性，
    // 并且后续 Render Tree 一层一层构建，所有的节点的 PipelineOwner? _owner 属性，都是这个 RendererBinding.pipelineOwner，
    // 即所有的 RenderObject 节点共用这个 PipelineOwner 对象。
    _rootNode?.attach(this);
  }
```

# Render Tree rootNode

&emsp;至此我们看到 Render Tree 的根节点（RendererBinding.renderView），赋值给了 RendererBinding.pipelineOwner.rootNode 属性，这便完成了根节点的附加工作。这其中牵涉的 RendererBinding 是一个重点。

&emsp;RendererBinding 是 Render Tree 和 Flutter 引擎之间的粘合剂。RendererBinding 管理多个独立的 Render Tree。每个 Render Tree 由一个必须通过 addRenderView 添加到 binding 中才会在帧生成、点击测试等过程中被考虑的 RenderView 作为根节点。此外，Render Tree 必须由一个属于以 rootPipelineOwner 为根的 Pipeline Owner Tree 的 PipelineOwner 管理。

&emsp;以上述方式向此 RendererBinding 添加 PipelineOwners 和 RenderViews 是更高级抽象的责任。例如，Widgets 库引入了 View widget，它向此 binding 注册其 RenderView 和 PipelineOwner。

&emsp;接下来的学习越来越复杂了！

## 参考链接
**参考链接:🔗**
+ [PipelineOwner class](https://api.flutter.dev/flutter/rendering/PipelineOwner-class.html)
+ [14.2 Element、BuildContext和RenderObject](https://book.flutterchina.club/chapter14/element_buildcontext.html#_14-2-1-element)
+ [Flutter启动流程和渲染管线](https://book.flutterchina.club/chapter14/flutter_app_startup.html#_14-3-1-应用启动)
+ [FlutterView class](https://api.flutter.dev/flutter/dart-ui/FlutterView-class.html)
+ [RendererBinding mixin](https://api.flutter.dev/flutter/rendering/RendererBinding-mixin.html)
