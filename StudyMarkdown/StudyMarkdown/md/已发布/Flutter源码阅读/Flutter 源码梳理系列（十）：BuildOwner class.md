# Flutter 源码梳理系列（九）：BuildOwner class

&emsp;BuildScope 的内容看完了，下面我们紧接着看一下 BuildOwner 类的内容。

&emsp;文档给 BuildOwner 下的定义是：Manager class for the widgets framework，widgets framework 的管理类，那么我们下面来看一下，它能不能承担得起这个称号。

&emsp;BuildOwner 类跟踪需要重建的 Widget，并处理适用于整个 Widget Tree 的其他任务，例如管理树的非活动 element 列表，并在调试时进行热重载时必要时触发 "reassemble" 命令。以上功能分别对应：

+ Element.markNeedsBuild 内部就是调用 BuildOwner.scheduleBuildFor 把脏 Element 添加到全局脏 Element 列表中 BuildScope._dirtyElements，待下一帧进行重建。  
+ Element.deactivateChild 内部就是被非活动 Element 对象添加全局非活动 Element 列表中：BuildOwner._inactiveElements。
+ BuildOwner.reassemble 内部就是调用 root.reassemble()。

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
2. `_unmountAll`：则是对排序后的所有非活动 Element 对象执行 unmount。
3. `_deactivateRecursively`：递归调用指定 element 对象以及其所有 child 的 deactivate 函数。即对 element 以及整个子树的 element 对象调用 deactivate 方法，是整个 element 子树失活。
4. `add`：使指定 element 子树失活后，加入 `final Set<Element> _elements = HashSet<Element>()` 内。
5. `remove`：仅是从 `_elements` 中移除指定 element 对象：`_elements.remove(element)`。

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
    assert(_debugStateLockLevel >= 0);
    assert(() {
      _debugStateLockLevel += 1;
      return true;
    }());
    try {
      callback();
    } finally {
      assert(() {
        _debugStateLockLevel -= 1;
        return true;
      }());
    }
    assert(_debugStateLockLevel >= 0);
  }
```

## 







## 参考链接
**参考链接:🔗**
+ [Element class](https://api.flutter.dev/flutter/widgets/Element-class.html)
+ [Offstage class](https://api.flutter.dev/flutter/widgets/Offstage-class.html)
