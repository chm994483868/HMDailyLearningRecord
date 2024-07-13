# Flutter 源码梳理系列（十）：BuildOwner class

&emsp;BuildScope 的内容看完了，下面我们紧接着看一下 BuildOwner 类的内容。

&emsp;文档给 BuildOwner 下的定义是：Manager class for the widgets framework，widgets framework 的管理类，那么我们下面来看一下，它能不能承担得起这个称号。

&emsp;BuildOwner 类跟踪需要重建的 Widget，并处理适用于整个 Widget Tree 的其他任务，例如管理树的非活动 element 列表，并在调试时进行热重载时必要时触发 "reassemble" 命令。以上功能分别对应：

+ Element.markNeedsBuild 内部会调用 BuildOwner.scheduleBuildFor 把脏 Element 对象添加到当前 BuildOwner 下全局脏 Element 列表：BuildScope._dirtyElements 中，待下一帧进行重建。  
+ Element.deactivateChild 内部会把非活动 Element 对象添加到当前 BuildOwner 下全局非活动 Element 列表：BuildOwner._inactiveElements 中。
+ BuildOwner.reassemble 内部会调用 root.reassemble。root.reassemble 内部会递归调用 reassemble 把指定 element 对象以及它的所有子级 element 节点调用 markNeedsBuild，标记为为脏。

&emsp;以上提到的功能确实与 BuildOwner 类息息相关。

&emsp;main build owner 通常由 WidgetsBinding 拥有，并且与 build/layout/paint 流水线的其余部分一起由操作系统驱动。主 BuildOwner 由 WidgetsBinding 所拥有即上一篇我们学习 BuildScope 中，根 Element 节点的 owner 正是 WidgetsBinding.instance 的 `_buildOwner` 属性。

&emsp;也可以构建额外的 BuildOwner 来管理 off-screen widget tree。

&emsp;要将 BuildOwner 分配给 Element Tree，可以使用 RootElementMixin.assignOwner 方法，即我们上一篇分析的：RootWidget.attach 函数，当根 Element 节点创建完成后会直接调用 RootElementMixin.assignOwner 方法为它分配一个：`_owner` 属性。

&emsp;OK，下面我们看一下 BuildOwner 类的源码。

# BuildOwner

&emsp;BuildOwner 类的内容主要集中在脏 Element 列表中 Element 对象重建、非活动 Element 对象收集、GlobalKey - Element 对象收集等。

## Constructors

&emsp;创建一个管理 Widget 的对象。如果未指定 focusManager 参数，或者该参数为 null，这将会构建一个新的 FocusManager，并通过 FocusManager.registerGlobalHandlers 注册其全局输入处理程序，这将修改静态状态。如果调用者想要避免改变这种状态，可以在这里显式传递一个 focus manager。

&emsp;在 WidgetsBinding 中：`_buildOwner = BuildOwner()`，并没有传入 focusManager 参数，执行的默认行为。关于 FocusManager 相关的内容，我们后续再展开。

```dart
class BuildOwner {
  BuildOwner({ this.onBuildScheduled, FocusManager? focusManager }) :
      focusManager = focusManager ?? (FocusManager()..registerGlobalHandlers());
  //...
}
```

## onBuildScheduled

&emsp;当每次第一次在 buildScope 的脏 Element 列表中添加脏 Element 对象时执行的回调。每次是指第一次和后续每次脏 Element 列表中 Element 重建完成后，脏 Element 列表会被清空，然后再次被添加第一个脏 Element 对象时。

```dart
  VoidCallback? onBuildScheduled;
```

## `_inactiveElements`

&emsp;大家最期待的，全局非活动 Element 对象集合。这里同全局脏 Element 对象的管理一样，也是为其定义了一个私有类，这里是 `_InactiveElements`，它呢，有一个 `final Set<Element> _elements = HashSet<Element>()` 用来存储非活动 Element 对象。

&emsp;然后在 BuildOwner 内部则是通过这个 `_InactiveElements` 类型的 `_inactiveElements` 属性来管理非活动 Element 对象。

&emsp;`_InactiveElements` 类提供了：

1. `_unmount`：递归调用指定 element 对象以及其所有 child 的 unmount 函数，即对 element 以及整个子树的 element 对象调用 unmount 方法，即不能被再次激活复用的 Element 对象只能等待被 GC 回收♻️。

2. `_unmountAll`：则是对排序后的所有非活动 Element 对象执行 unmount，不过这里的排序可以注意一下，它的排序规则和脏 element 列表重建时的排序刚好相反，它是先把 depth 大的子级 element 节点 unmount，然后再 unmount depth 小的父级 element 节点。（当前帧内，当脏 element 列表刷新重建完毕后，还位于非活动列表的 element 对象，被通过 BuildOwner.finalizeTree 调用此 `_inactiveElements._unmountAll` 函数，卸载非活动 element 对象。）

3. `_deactivateRecursively`：递归调用指定 element 对象以及其所有 child 的 deactivate 函数。即对 element 以及整个子树的 element 对象调用 deactivate 方法，是整个 element 子树失活。

4. `add`：使指定 element 子树失活后，加入 `final Set<Element> _elements = HashSet<Element>()` 内。（Element.deactivateChild 函数调用时，把这个失活的 element 对象通过 `owner!._inactiveElements.add(child)` 这个 add 函数添加到 `_elements` 中。）

5. `remove`：仅是从 `_elements` 中移除指定 element 对象：`_elements.remove(element)`。（在 Element.inflateWidget 时，如果 newWidget 有 globalKey 并能在 `WidgetsBinding.instance.buildOwner!._globalKeyRegistry[this]` 取得对应的 element 对象，则会取出它进行复用，然后通过 `owner!._inactiveElements.remove(element)` 这个 remove，把指定的 element 对象从 `_elements` 中移除。）

&emsp;`_InactiveElements` 类的内容都会简单，就是上面的函数和 `_elements` 集合最为重要，这里就不展开了。

```dart
  final _InactiveElements _inactiveElements = _InactiveElements();
```

## focusManager

&emsp;focusManager 是负责 focus tree 的对象。很少直接使用。相反，考虑使用 FocusScope.of 来获取给定 BuildContext 的 FocusScopeNode。

&emsp;有关更多详细信息，请参见 FocusManager。

&emsp;该字段将默认为通过 FocusManager.registerGlobalHandlers 注册其全局输入处理程序的 FocusManager。希望避免注册这些处理程序（并修改关联的静态状态）的调用方可以明确地将 focus manager 传递给 BuildOwner 构造函数。

&emsp;关于 FocusManager 的内容，我们后续再展开，这里只要有个印象：BuildOwner 关联的有个 FocusManager 要学习呢，别忘了。

```dart
  FocusManager focusManager;
```

## scheduleBuildFor

&emsp;将指定 element 对象添加到脏 element 列表中，这样当 WidgetsBinding.drawFrame 调用 buildScope 时，此 element 对象将重建。

```dart
  void scheduleBuildFor(Element element) {
    final BuildScope buildScope = element.buildScope;
    
    // 上面提到的第一次往脏 element 列表中添加脏 element 执行的回调
    if (!_scheduledFlushDirtyElements && onBuildScheduled != null) {
      _scheduledFlushDirtyElements = true;
      onBuildScheduled!();
    }
    
    // 直接通过 element 共享的 buildScope 调用：_scheduleBuildFor 函数。  
    buildScope._scheduleBuildFor(element);
  }
```

## lockState

&emsp;建立了一个范围，其中对 State.setState 的调用被禁止，并调用了给定的回调函数。这种机制用于确保，例如 State.dispose 不会调用 State.setState。

```dart
  void lockState(VoidCallback callback) {
    try {
      callback();
    } finally {

    }
  }
```

## buildScope

&emsp;建立一个更新 widget tree 的范围，并在必要时调用给定的回调。然后，按 depth 顺序重建所有被标记为脏的 element 对象，这些 element 对象正是通过上面的 scheduleBuildFor 方法标记的。（被添加在 element.buildScope._dirtyElements 中。）

&emsp;这种机制防止 build 方法间相互依赖，可能导致无限循环。(对 element.buildScope._dirtyElements 中的脏 element 对象排序后才进行子树重建)

&emsp;要在不执行其他工作的情况下刷新当前的脏 element 列表，可以在没有回调的情况下调用此函数。这是 framework 每帧都会执行的操作，在 WidgetsBinding.drawFrame 中。在 WidgetsBinding.drawFrame 内部正是通过：`buildOwner!.buildScope(rootElement!);` 调用到此函数的，`buildOwner` 所有的 element 节点共享。

&emsp;一次只能有一个 buildScope 处于活动状态。

```dart
  @pragma('vm:notify-debugger-on-exception')
  void buildScope(Element context, [ VoidCallback? callback ]) {
    final BuildScope buildScope = context.buildScope;
    if (callback == null && buildScope._dirtyElements.isEmpty) {
      return;
    }
    
    try {
      _scheduledFlushDirtyElements = true;
      buildScope._building = true;
      
      if (callback != null) {
        try {
          callback();
        } finally {
        
        }
      }
      
      buildScope._flushDirtyElements(debugBuildRoot: context);
    } finally {
      buildScope._building = false;
      _scheduledFlushDirtyElements = false;
    }
  }
```

## `_globalKeyRegistry`

&emsp;用来存储全局的：GlobalKey 和 对应的 Element 对象。

```dart
  final Map<GlobalKey, Element> _globalKeyRegistry = <GlobalKey, Element>{};
```

## globalKeyCount

&emsp;当前与由此 BuildOwner 构建的 Element 对象相关联的 GlobalKey 实例的数量。

```dart
  int get globalKeyCount => _globalKeyRegistry.length;
```

## `_registerGlobalKey`

&emsp;把一对 GlobalKey 和 Element 保存在 `_globalKeyRegistry` 中。

&emsp;⭐️在 Element.mount 函数中，如果 element 对象对应的 Widget 有 GlobalKey，则通过 `owner!._registerGlobalKey(key, this)` 把它们添加到 `_globalKeyRegistry` 中。

```dart
  void _registerGlobalKey(GlobalKey key, Element element) {
    _globalKeyRegistry[key] = element;
  }
```

## `_unregisterGlobalKey`

&emsp;把一对 GlobalKey 和 Element 从 `_globalKeyRegistry` 中移除。

&emsp;⭐️在 Element.unmount 函数中，当 element 对象要卸载了，如果 element 对象对应的 Widget 有 GlobalKey，则通过 `owner!._unregisterGlobalKey(key, this)` 把它们从 `_globalKeyRegistry` 中移除。 

```dart
  void _unregisterGlobalKey(GlobalKey key, Element element) {
    if (_globalKeyRegistry[key] == element) {
      _globalKeyRegistry.remove(key);
    }
  }
```

## `finalizeTree`

&emsp;这个便是之前见了很多次的，非活动的 Element 列表中，在此帧内如果没有被复用激活的话，剩余的非活动的 Element 对象就要被全部 unmount，然后等待 GC 回收♻️。

&emsp;此方法也是由 WidgetsBinding.drawFrame 调用。

```dart
  @override
  void drawFrame() {
   //...
   
    try {
      if (rootElement != null) {
        buildOwner!.buildScope(rootElement!);
      }
      super.drawFrame();
      buildOwner!.finalizeTree();
    }
    
   //...
  }
```

```dart
  @pragma('vm:notify-debugger-on-exception')
  void finalizeTree() {
    try {
      lockState(_inactiveElements._unmountAll); // this unregisters the GlobalKeys
    } catch (e, stack) {
    
    } finally {
    }
  }
```

## reassemble

&emsp;导致以给定 Element 对象为根的整个子树完全重建。当应用程序代码发生更改并正在热重载时，开发工具会使用这个功能，以使 Widget Tree 获取任何已更改的实现。

&emsp;这是昂贵的，不应该在非开发过程中调用。

&emsp;会递归对 Element root 所有的子级 element 对象调用 reassemble 函数。全部被标记为重建：`markNeedsBuild()`。

```dart
  void reassemble(Element root) {
    try {
      root.reassemble();
    } finally {

    }
  }
```

## BuildOwner 总结

&emsp;Ok，到此 BuildOwner 的内容就看完了，同 BuildScope 一样，自 RootElement 根节点出发在每个 element 节点中传递。

&emsp;三种超重要的全局存储：

1. `_inactiveElements._elements` 管理当前 BuildOwner 下非活动 element 对象。
2. `buildScope._dirtyElements` 管理当前 BuildOwner（或当前 BuildScope）下标记（markNeedsBuild）为脏的 element 列表。
3. `_globalKeyRegistry` 管理当前 BuildOwner 下所有的 GlobalKey - Element 对。

&emsp;再然后就是 WidgetsBinding.drawFrame 回调中，在当前帧 BuildOwner.buildScope 重建脏 element 元素，然后 BuildOwner.finalizeTree 把剩余的非活动的 element 对象 unmount。

&emsp;纵观整个 BuildOwner 参与的工作，确实称的上：Manager class for the widgets framework，widgets framework 的管理类！

## 参考链接
**参考链接:🔗**
+ [buildScope method](https://api.flutter.dev/flutter/widgets/BuildOwner/buildScope.html)
+ [Element class](https://api.flutter.dev/flutter/widgets/Element-class.html)
+ [Offstage class](https://api.flutter.dev/flutter/widgets/Offstage-class.html)
+ [BuildOwner class](https://api.flutter.dev/flutter/widgets/BuildOwner-class.html)
