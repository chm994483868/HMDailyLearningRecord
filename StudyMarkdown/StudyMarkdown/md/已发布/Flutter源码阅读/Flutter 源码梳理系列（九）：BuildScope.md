# Flutter 源码梳理系列（九）：BuildScope

&emsp;在接下来的内容之前呢，我们先来看一下 BuildScope 和 BuildOwner 的内容，为我们学习 Element 而铺路。但其实客观来讲，感觉还是先去学 Element 的内容，把整个 Element Tree 相关的内容搞懂了，再回过头来补充这些细节。

&emsp;在学习 Element 类时我们会看到有如下四个字段：

```dart
  @override
  BuildOwner? get owner => _owner;
  BuildOwner? _owner;
  
  BuildScope get buildScope => _parentBuildScope!;
  BuildScope? _parentBuildScope;
```

&emsp;然后在 Element 的 mount 函数中会看到它们直接是通过父级传值的，即 Element tree 上的所有 element 节点共用同一个 `_owner` 和 `_parentBuildScope`。

```dart
  @mustCallSuper
  void mount(Element? parent, Object? newSlot) {
    //...
    
    if (parent != null) {
      // Only assign ownership if the parent is non-null. If parent is null (the root node), the owner should have already been assigned.
      // See RootRenderObjectElement.assignOwner().
      _owner = parent.owner;
      _parentBuildScope = parent.buildScope;
    }
    
    //...
  }
```

&emsp;那么既然由父级而来，那么我们就有必要看一下 Element tree 的根节点从哪里来的了。在此之前我们先看一下 framework.dart 中定义的：`mixin RootElementMixin on Element`。

# RootElementMixin

&emsp;RootElementMixin 是 Element tree 根节点的 Mixin。只有 root elements 可以显式设置其 owner。所有其他 element 都从其 parent element 继承其 owner。RootElementMixin 直接混入了 Element 类，然后它添加了一个很重要的 assignOwner 函数。

```dart
mixin RootElementMixin on Element { //... }
```

## assignOwner

&emsp;assignOwner：设置 Element 对象的 owner。owner 将传播到该 element 的所有后代(所有的 element 子节点中)。owner 管理脏元素列表（已交由 BuildScope 替 owner 代管理：`final List<Element> _dirtyElements = <Element>[]`）。

&emsp;WidgetsBinding 引入了主 owner ：WidgetsBinding.buildOwner，并在调用 runApp 时将其分配给 Element Tree。该 binding 负责通过调用 build owner 的：BuildOwner.buildScope 来驱动 build pipeline。参见：WidgetsBinding.drawFrame。

&emsp;关于 Binding 相关的内容，太复杂了，我们暂时不深入看，我们目前的话主要追 `_owner` 和 `_parentBuildScope` 的起源。上面提到 “WidgetsBinding 引入了主 owner”，这句可在 WidgetsBinding 的 initInstances 函数里得到证实，直接创建了主 owner：`_buildOwner = BuildOwner();`。然后是 “该 binding 负责通过调用 build owner 的：BuildOwner.buildScope 来驱动 build pipeline。” 此句也极其重要，这里提到的 BuildOwner.buildScope 即是刷新当前 Scope 内的 Element 脏列表中的所有脏节点进行重建。

&emsp;其实看到这里可以发现 BuildScope 类其实就是 BuildOwner 的辅助类，替 BuildOwner 打理 element 脏列表相关的内容，例如收集脏 element 对象、统一对它们进行重建工作等，其实还发现之前是没有 BuildScope 类的，之前的脏列表 `final List<Element> _dirtyElements = <Element>[]` 直接就是 BuildOwner 类的属性，大概是哪个 Flutter 版本后把它抽出来到 BuildScope 类中统一管理。

```dart
  // ignore: use_setters_to_change_properties, (API predates enforcing the lint)
  void assignOwner(BuildOwner owner) {
    _owner = owner;
    _parentBuildScope = BuildScope();
  }
```

## mount

&emsp;RootElementMixin 重写了 Element.mount 函数。

```dart
  @override
  void mount(Element? parent, Object? newSlot) {
    // Root elements should never have parents.
    super.mount(parent, newSlot);
  }
```

&emsp;下面我们直接在 RootElementMixin.assignOwner 处打一个断点，看下 root Widget 和 root Element 的由来。

&emsp;简单分析一下函数堆栈，可以看出 RootWidget 正是我们的 Widget Tree 的根 Widget，而 RootWidget 有一个 `RootElement attach(BuildOwner owner, [ RootElement? element ])` 函数，正是可用来在 APP 启动时构建我们的 Element Tree 的根 Element 的（看到它的类型是：RootElement，直接继承自 Element 并 with 了 RootElementMixin：`class RootElement extends Element with RootElementMixin`），RootWidget 和 RootElement 两个类内容还是较简单的，后续我们再深入看。目前我们先看 RootWidget.attch 函数内容：

```dart
  RootElement attach(BuildOwner owner, [ RootElement? element ]) {
    if (element == null) {
      owner.lockState(() {
        // 由 RootWidget 对象创建 RootElement 对象，即我们的根 element
        element = createElement();
        
        // 然后这里就是我们要追寻的源头啦，因为 RootElement 混了 RootElementMixin，
        // 所以它取得了 assignOwner 函数，然后直接调用，这个 owner 入参就是 WidgetsBinding 对象的 _buildOwner 字段
        element!.assignOwner(owner);
      });
      
      // 然后接下来就是通过 buildScope 的回调进行根 element 的挂载！
      owner.buildScope(element!, () {
        element!.mount(/* parent */ null, /* slot */ null);
      });
      
    } else {
      element._newWidget = this;
      element.markNeedsBuild();
    }
    return element!;
  }
```

&emsp;OK，看完 attach 函数内容我们就清晰了，RootWidget 根 Widget 对象同样通过 createElement 创建了 RootElement 根 element 对象，然后根 element 对象调用 assignOwner 函数，直接传递了来自 WidgetsBinding 的 `_buildOwner` 实例对象和新建了一个 BuildScope 实例对象（BuildScope()），由此就构成了我们整个 Element tree 上所有节点共用的 `_owner` 和 `_parentBuildScope`。

&emsp;OK，看完了 BuildScope 的来源，接下来我们就看 BuildScope 类的内容吧。

# BuildScope

&emsp;BuildScope 是一个确定 BuildOwner.buildScope 操作范围的类（BuildOwner.buildScope：`void buildScope(Element context, [ VoidCallback? callback ])`）。BuildOwner.buildScope 方法重建所有与其 context 参数共享相同 Element.buildScope 的脏 Element，并跳过具有不同 Element.buildScope 的 element。默认情况下，Element 的 buildScope 与其 parent element 相同。

&emsp;可能看到这个官方文档又有点晕了，这里的核心内容主要在 BuildOwner.buildScope 函数内。通过直接在 BuildOwner.buildScope 函数打断点，发现传递来的 context 参数正是我们的根 elemnet：rootElement 对象，然后它下面的所有 element 节点共用同一个 buildScope 对象即：`final BuildScope buildScope = context.buildScope`（排除 LayoutBuilder 这种特殊情况），而这 BuildOwner.buildScope 函数正是取得入参 context 的 buildScope 来进行内部脏 element 列表的重建工作。

&emsp;特殊的 Element 可以重写 Element.buildScope 来为其后代创建一个隔离的构建范围。例如，LayoutBuilder 建立了自己的 BuildScope，这样没有后代 Element 可能在传入的约束未知之前提前重建。在 `class _LayoutBuilderElement<ConstraintType extends Constraints> extends RenderObjectElement` 类内部看到：`late final BuildScope _buildScope = BuildScope(scheduleRebuild: _scheduleRebuild);`，即它自己创建了自己的 BuildScope。

```dart
final class BuildScope {
  // 创建一个带有可选的 schedulebuild 回调的 BuildScope。scheduleRebuild 回调仅 _LayoutBuilderElement 用到。
  BuildScope({ this.scheduleRebuild });
  // ...
}
```

## scheduleRebuild

&emsp;一个可选的 VoidCallback，当这个 BuildScope 中的脏 element 列表每次被添加第一个脏 element 时，将会调用该回调。通常，这个回调表示需要在本帧的后续时间内调用 BuildOwner.buildScope 方法来重建这个 BuildScope 中的脏 element。如果这个 Scope 正在被 BuildOwner.buildScope 主动构建，那么它将 **不会** 被调用，因为当 BuildOwner.buildScope 返回时，BuildScope 将是干净的。

&emsp;这个仅由 `_LayoutBuilderElement` 会用到。

```dart
  // scheduleRebuild 是否被调用
  bool _buildScheduled = false;
  
  // 是否 BuildOwner.buildScope 在这个 buildScope 中正在运行
  bool _building = false;
  
  final VoidCallback? scheduleRebuild;
```

## `_dirtyElementsNeedsResorting`

&emsp;在构建过程中更多 element 变为脏 element，需要重新对 `_dirtyElements` 进行排序。这是为了保持 `Element._sort` 定义的排序顺序。当 buildScope 不在积极重新构建 Widget tree 时，此字段设置为 null。

&emsp;这里的意思是，当已经开始对脏 element 列表中的 element 对象进行重建了，但是在重建过程中又有其它的脏 element 对象进入脏 element 列表了，例如：markNeedsBuild 函数被调用时就是把 element 对象加入脏 element 列表中，此时会需要对脏 element 列表中的 element 进行重新排序，超级重要。

&emsp;首先要理解对某个 element 节点进行重建意味着什么，它意味着对自这个 element 节点起的整个子树的重建。

&emsp;当对脏 element 列表中的 element 对象进行重建时，并不是按照列表顺序一个一个来的，这里的每个 element 对象都是一棵 Element 子树，如果不对它们进行排序会造成重建的资源浪费，例如：子级先重建了，父级后进行重建时，又对子级进行重建。这里会根据 element 节点的 depth 和 dirty 来进行排序，保证从父级往子级重建。具体的排序规则我们下面会看到。

```dart
  bool? _dirtyElementsNeedsResorting;
```

## `_dirtyElements`

&emsp;存储当前 BuildScope 内所有被标记为脏的 Element 对象。

```
  final List<Element> _dirtyElements = <Element>[];
```

## `_scheduleBuildFor`

&emsp;把 element 入参添加到脏 element 列表中去。

```dart
  @pragma('dart2js:tryInline')
  @pragma('vm:prefer-inline')
  @pragma('wasm:prefer-inline')
  void _scheduleBuildFor(Element element) {
    assert(identical(element.buildScope, this));
    
    // 把入参 element 添加脏列表中去
    if (!element._inDirtyList) {
      _dirtyElements.add(element);
      element._inDirtyList = true;
    }
    
    // 每次第一次调用 _scheduleBuildFor 时，才会执行 scheduleRebuild 回调，
    // _buildScheduled 标识在每次脏列表清空以后才会被重置为 false
    if (!_buildScheduled && !_building) {
      _buildScheduled = true;
      scheduleRebuild?.call();
    }
    
    // 当有新的脏 element 加入列表了，就需要标记为需要重新排序。
    if (_dirtyElementsNeedsResorting != null) {
      _dirtyElementsNeedsResorting = true;
    }
  }
```

## `_tryRebuild`

&emsp;重建入参 Element element，需要保证当前 element 在脏列表中，并且 element 的 buildScope 字段就是当前的这个 BuildScope。

```dart
  @pragma('dart2js:tryInline')
  @pragma('vm:prefer-inline')
  @pragma('wasm:prefer-inline')
  @pragma('vm:notify-debugger-on-exception')
  void _tryRebuild(Element element) {
    try {
      element.rebuild();
    } catch (e, stack) {

    }
  }
```
## Element._sort

&emsp;位于 Element 中的一个静态的函数，主要用于 BuildScope 中脏 element 列表 `_dirtyElements` 的 element 排序使用。

&emsp;根据这个排序规则可以看出：

1. depth 值小的 element 排在前面（即那些父级 element 排在子级 element 前面去）
2. depth 相等时，dirty 的排在前面。 

```dart
  /// Returns result < 0 when [a] < [b], result == 0 when [a] == [b], result > 0 when [a] > [b].
  static int _sort(Element a, Element b) {
    final int diff = a.depth - b.depth;
    
    // If depths are not equal, return the difference.
    if (diff != 0) {
      return diff;
    }
    
    // If the `dirty` values are not equal, sort with non-dirty elements being less than dirty elements.
    final bool isBDirty = b.dirty;
    if (a.dirty != isBDirty) {
      return isBDirty ? -1 : 1;
    }
    
    // Otherwise, `depth`s and `dirty`s are equal.
    return 0;
  }
```

## `_flushDirtyElements`

&emsp;首先根据 Element._sort 对脏 element 列表进行排序，然后才能对脏 element 列表中的脏 element 进行重建，并且在重建过程中会实时的对脏 element 列表进行排序以及获取下一个最适合进行重建的 element 对象。

```dart
  @pragma('vm:notify-debugger-on-exception')
  void _flushDirtyElements({ required Element debugBuildRoot }) {
    // 根据 element 的 depth 和 dirty 对脏表中的 element 进行排序，
    // 父级排在子级前面，平级时，脏的排在非脏前面
    _dirtyElements.sort(Element._sort);
    
    // 当在重建过程中有新的脏 element 加入则置为 true
    _dirtyElementsNeedsResorting = false;
    
    try {
      // 通过 _dirtyElementIndexAfter 确切取得下一个需要重建的 element 节点
      for (int index = 0; index < _dirtyElements.length; index = _dirtyElementIndexAfter(index)) {
        final Element element = _dirtyElements[index];
        
        // 确保是同一个 BuildScope 的 Element 节点才进行重建
        if (identical(element.buildScope, this)) {
          _tryRebuild(element);
        }
      }
    } finally {
      for (final Element element in _dirtyElements) {
        if (identical(element.buildScope, this)) {
          // 重建完成后，把 element._inDirtyList 置为 false
          element._inDirtyList = false;
        }
      }
      
      // 清空脏列表
      _dirtyElements.clear();
      _dirtyElementsNeedsResorting = null;
      
      // 标记开始新的一轮脏 element 收集
      _buildScheduled = false;
    }
  }
```

## `_dirtyElementIndexAfter`

&emsp;当对一个 element 子树进行重建后，每个 element 节点的 dirty 会置为 false。（处于活动状态或者 dirty 为 true 的节点才会重建。）

```dart
  @pragma('dart2js:tryInline')
  @pragma('vm:prefer-inline')
  @pragma('wasm:prefer-inline')
  int _dirtyElementIndexAfter(int index) {
    // 如果上一个 element 对象重建完成后，没有新的脏 element 加入脏列表，则 +1 即可，继续下个 element 重建
    if (!_dirtyElementsNeedsResorting!) {
      return index + 1;
    }
    
    // 如果有新的脏元素加入
    index += 1;
    
    // 重新进行排序
    _dirtyElements.sort(Element._sort);
    // 标记置为 false
    _dirtyElementsNeedsResorting = false;
    
    // 往前遍历看还没有 dirty 为 true 的 elelment 节点
    while (index > 0 && _dirtyElements[index - 1].dirty) {
      // It is possible for previously dirty but inactive widgets to move right in the list.
      // We therefore have to move the index left in the list to account for this.
      // We don't know how many could have moved. However, we do know that the only possible
      // change to the list is that nodes that were previously to the left of the index have
      // now moved to be to the right of the right-most cleaned node, and we do know that
      // all the clean nodes were to the left of the index. So we move the index left
      // until just after the right-most clean node.
      
      // 之前被标记为脏状态但是处于非活动状态的 Widget 可能会在列表中向右移动。
      // 因此，我们必须将索引向左移动以进行调整。
      // 我们不知道可能有多少个 widget 已经移动。
      // 不过，我们知道列表的唯一可能变化是之前处于索引左侧的节点现在已经移动到最右侧的 已清理节点（dirty 为 false）右侧，
      // 而且所有的 已清理节点（dirty 为 false）都在索引左侧。因此，我们将索引向左移动，直到刚好位于最右侧的 已清理节点（dirty 为 false） 之后。
      index -= 1;
    }
    
    return index;
  }
```

## BuildScope 总结

&emsp;



## 参考链接
**参考链接:🔗**
+ [BuildOwner class](https://api.flutter.dev/flutter/widgets/BuildOwner-class.html#widgets.BuildOwner.1)
