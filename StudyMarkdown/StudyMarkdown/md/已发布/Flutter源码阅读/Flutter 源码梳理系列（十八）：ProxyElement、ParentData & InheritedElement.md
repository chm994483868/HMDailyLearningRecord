# Flutter 源码梳理系列（十八）：ProxyElement、ParentData & InheritedElement

# ProxyElement

&emsp;ProxyElement：使用 ProxyWidget 作为配置的 Element。

+ Object -> DiagnosticableTree -> Element -> ComponentElement -> ProxyElement

&emsp;ProxyElement 直接继承自 ComponentElement。

## Constructors

&emsp;创建一个 ProxyElement，使用给定的 StatefulWidget 作为其配置。

```dart
abstract class ProxyElement extends ComponentElement {
  ProxyElement(ProxyWidget super.widget);
  
  // ...
}
```

## build

&emsp;StatelessElement 的 build 函数是调用自己的 StatelessWidget 的 build 函数，StatefulElement 的 build 函数是调用自己的 State 的 build 函数，ProxyElement 的 build 函数则是直接返回自己的 ProxyWidget 的 child 属性。

```dart
  @override
  Widget build() => (widget as ProxyWidget).child;
```

## update

&emsp;更新 ProxyElement 的 widget。

```dar
  @override
  void update(ProxyWidget newWidget) {
    // 记录旧的 widget
    final ProxyWidget oldWidget = widget as ProxyWidget;
    super.update(newWidget);
    
    // 默认调用 updated
    updated(oldWidget);
    
    // 同 StetelessElement/StatefulElement 一样，
    // 更新完 widget 以后，强制进行重建
    rebuild(force: true);
  }
```

## updated

&emsp;在 ProxyElement 的 Widget 发生更改时调用构建方法。默认情况下，会调用 notifyClients。子类可以重写此方法，以避免不必要地调用 notifyClients（例如，如果旧 Widget 和新 Widget 是等效的）。

```dart
  @protected
  void updated(covariant ProxyWidget oldWidget) {
    notifyClients(oldWidget);
  }
```

## notifyClients

&emsp;通知其他对象，与该 ProxyElement 关联的 Widget 已更改。在更新期间（通过 `updated` 调用），在更改与该 ProxyElement 关联的 Widget 之后但在重建此 ProxyElement 之前调用。

```dart
  @protected
  void notifyClients(covariant ProxyWidget oldWidget);
```

## ProxyElement 总结

&emsp;和 StatefulElement 比的话 ProxyElement 的内容并不多。当然还是比 StatelessElement 的内容多一些的，毕竟 StatelessElement 是最简单的 Element。当然和 StatelessElement 比较的话只多了：updated 和 notifyClients，且 notifyClients 还是一个抽象函数，目前看到现在此函数只是说在 ProxyElement 对象的 Widget 发生更改后，向外发出通知，具体 ProxyElement 的子类执行什么，由子类本身来决定。

&emsp;然后另外一个不同点就是 ProxyElement 的 build 函数，我们知道 build 函数的本质只是返回一个 Widget 对象，可以理解是当前位于 Widget Tree 的一个 Widget 节点的下一个 Widget 节点。其中 StatelessElement 的 build 函数是调用自己的 StatelessWidget 的 build 函数，StatefulElement 的 build 函数是调用自己的 State 的 build 函数，而 ProxyElement 的 build 函数则是直接返回自己的 child 属性（`final Widget child`）。

&emsp;下面我们继续，我们继续看 ProxyElement 的两个直接子类 ParentDataElement 和 InheritedElement，在它们中能找到跟 notifyClients 相关的内容。

# ParentDataElement

&emsp;ParentDataElement：使用 ParentDataWidget 作为配置的 Element。

+ Object -> DiagnosticableTree -> Element -> ComponentElement -> ProxyElement -> ParentDataElement

&emsp;ParentDataElement 直接继承自 ProxyElement。

## Constructors

&emsp;创建一个 ParentDataElement，使用给定的 ParentDataWidget 作为其配置。

```dart
class ParentDataElement<T extends ParentData> extends ProxyElement {
  ParentDataElement(ParentDataWidget<T> super.widget);
  
  // ...
}
```

&emsp;ParentDataElement 的泛型 T 被要求是 ParentData 的子类，但其实 ParentData 是一个超级简单的类，仅有一个 detach 函数：

```dart
class ParentData {
  
  // 当 RenderObject 从树中移除时调用。
  @protected
  @mustCallSuper
  void detach() { }

  @override
  String toString() => '<none>';
}
```

## `_applyParentData`

&emsp;当前 ParentDataElement 对象沿着自己的 child 链找到第一 RenderObjectElement，然后调用 RenderObjectElement 的 `_updateParentData` 函数，入参是此 ParentDataElement 对象的 Widget 属性。

```dart
  void _applyParentData(ParentDataWidget<T> widget) {
    
    // 递归调用，直到找到第一个 RenderObjectElement 节点
    void applyParentDataToChild(Element child) {
      if (child is RenderObjectElement) {
      
        // _updateParentData 内部其实是调用入参 widget 的 applyParentData(renderObject) 函数。
        
        // 即内部其实是调用 ParentDataWidget 的 applyParentData 函数，而入参 renderObject 就是：
        // RenderObjectElement 的 renderObject 属性。而这个 RenderObjectElement 就是当前 ParentDataElement 
        // 对象沿着自己的 child 链找到的第一个 RenderObjectElement 类型的 Element 节点。
        child._updateParentData(widget);
        
      } else if (child.renderObjectAttachingChild != null) {
        // 继续 child 的 child 还有值则继续，直到找到这个 Element 链上的第一个 RenderObjectElement 类型的节点
        applyParentDataToChild(child.renderObjectAttachingChild!);
      }
    }
    
    if (renderObjectAttachingChild != null) {
      applyParentDataToChild(renderObjectAttachingChild!);
    }
  }
```

## applyWidgetOutOfTurn

&emsp;入参 newWidget 是 ParentDataWidget 类型的 Widget，还记得 ParentDataWidget 抽象类有一个抽象函数：`void applyParentData(RenderObject renderObject)` 吗？ParentDataWidget 的子类需要实现这个抽象函数。

&emsp;而 applyWidgetOutOfTurn 函数内部实现，其实就是调用入参 newWidget 的 ParentDataWidget.applyParentData 函数，而 ParentDataWidget.applyParentData 函数的参数是 RenderObject 类型，将最终由该元素负责的渲染对象的父级数据传递给它（就是由当前的 PerentDataElement 对象找到自己的第一个 RenderObjectElement 子级，把 parent data 传递给它的 RenderObject 属性。）。

&emsp;这允许修改 RenderObject 的 RenderObject.parentData 属性而不触发构建。一般来说这是不被推荐的，但在以下情况下是有意义的：

+ 构建（build）与布局（layout）目前正在进行中，但问题中的 ParentData 对布局没有影响，并且要应用的值在构建与布局之前无法确定（例如，它依赖于子元素的布局）。
+ 绘制（paint）目前正在进行中，但所涉及的 ParentData 对布局或绘制没有影响，并且要应用的值在绘制之前无法确定（例如，它取决于 compositing  阶段）。

&emsp;在任一种情况下，下一个构建（build）预计会导致此 Element 被配置为具有新的给定 Widget（或具有等效数据的 Widget）。

&emsp;只有那些对 ParentDataWidget.debugCanApplyOutOfTurn 返回 true 的 ParentDataWidget 可以通过这种方式应用。

&emsp;新的 widget 必须拥有与当前 widget 相同的子 widget。（需要保证入参 newWidget 的 child 属性和当前 ParentDataElement 对象的 Widget 的属性的 child 属性是相等的。）

&emsp;一个使用这种方法的示例是 AutomaticKeepAlive widget。如果在其子孙节点的构建过程中收到通知，说它的子代必须保持活动状态，它将在回合之外应用一个 KeepAlive widget。这是安全的，因为根据定义，子代已经是活动的，因此这不会改变父节点在这一帧的行为。这比仅为更新 KeepAlive widget 而请求额外的帧更有效率。

```dart
  void applyWidgetOutOfTurn(ParentDataWidget<T> newWidget) {
    // 需要保证入参 newWidget 的 child 属性和当前 ParentDataElement 对象的 Widget 的属性的 child 属性是相等的
    _applyParentData(newWidget);
  }
```

## notifyClients

&emsp;在与该元素关联的 widget 发生更改之后但在重建该元素之前，在更新期间（通过 updated）调用，通知其他对象 widget 已经发生了变化。

&emsp;notifyClients 函数是 ProxyElement 中定义的抽象函数，然后 ParentDataElement 作为 ProxyElement 的直接子类，实现了此 notifyClients 函数，内部直接调用 `_applyParentData` 函数。

```dart
  @override
  void notifyClients(ParentDataWidget<T> oldWidget) {
    _applyParentData(widget as ParentDataWidget<T>);
  }
```

## ParentDataElement 总结

&emsp;看完 ParentDataElement 中对 notifyClients 函数的重写，可以发现它的目标全部集中在了 RenderObject 上，把 ParentDataElement 的 ParentDataWidget 中的 parent data 数据应用到子孙 RenderObjectElement 的 RenderObject 属性中去。下面看 InheritedElement 中的 notifyClients 它是一个超级重点，看懂了它，就会搞明白 InheritedWidget 是如何做到在数据发生变化时，通知依赖者进行更新重建的。 

# InheritedElement

&emsp;InheritedElement：使用 InheritedWidget 作为配置的 Element。

+ Object -> DiagnosticableTree -> Element -> ComponentElement -> ProxyElement -> InheritedElement

&emsp;InheritedElement 直接继承自 ProxyElement。

## Constructors

&emsp;创建一个 InheritedElement，使用给定的 InheritedWidget 作为其配置。

```dart
class InheritedElement extends ProxyElement {
  InheritedElement(InheritedWidget super.widget);
  
  // ...
}
```

## `_dependents`

&emsp;可以看到 `_dependents` 属性是一个 Key 是 Element，Value 是 Object? 的 Map，Value 是 Object? 也预示着它可以是 null，而最重要的点其实是在 Key 上，它是 Element 类型。其实根据此属性名字也可以看出来，它是用来存储当前 InheritedElement 对象的依赖者的，即哪些 Element 对象依赖了它，就会被存储在 `_dependents` 中。

```dart
  final Map<Element, Object?> _dependents = HashMap<Element, Object?>();
```

## `_updateInheritance`

&emsp;当 InheritedElement 节点被挂载到 Element Tree 时，会以自己的类型为 Key 以自己为 Value 添加到 `_inheritedElements` 中，然后在 Element Tree 中自己所在的链中传递。  

```dart
  @override
  void _updateInheritance() {
    final PersistentHashMap<Type, InheritedElement> incomingWidgets =
        _parent?._inheritedElements ?? const PersistentHashMap<Type, InheritedElement>.empty();
        
    _inheritedElements = incomingWidgets.put(widget.runtimeType, this);
  }
```

## getDependencies

&emsp;使用 setDependencies 记录的依赖者返回相应的依赖值。

&emsp;每个依赖者都映射到一个单一的对象值，该对象表示元素如何依赖于这个 InheritedElement。默认情况下，该值为 null，并且默认情况下依赖者会无条件重新构建。

&emsp;子类可以使用 updateDependencies 来管理这些值，这样它们就可以有选择地重新构建依赖者，然后在 notifyDependent 中通知它们。

&emsp;通常只有在 updateDependencies 的重写中才会调用此方法。

&emsp;另请参阅：

+ updateDependencies，每次使用 dependOnInheritedWidgetOfExactType 创建依赖者时都会调用该方法。
+ setDependencies，为依赖者设置依赖值。
+ notifyDependent，它可以被重写以使用依赖者的依赖值来决定是否需要重新构建依赖者。
+ InheritedModel，这是一个使用此方法管理依赖值的类的示例。

```dart
  @protected
  Object? getDependencies(Element dependent) {
    return _dependents[dependent];
  }
```

## setDependencies

&emsp;设置 dependent 的 getDependencies value 返回的值。

&emsp;每个 dependent 元素都映射到一个表示该元素如何依赖于此 InheritedElement 的单个对象值。updateDependencies 方法默认将此值设置为 null，以便无条件重新构建 dependent 元素。

&emsp;子类可以使用 updateDependencies 来管理这些值，以便它们可以有选择地在 notifyDependent 中重新构建 dependents。

&emsp;通常只在 updateDependencies 的重写中调用此方法。

&emsp;另请参阅：

+ updateDependencies，每次使用 dependOnInheritedWidgetOfExactType 创建一个依赖者时都会调用此方法。
+ getDependencies，返回 dependent Element 的当前值。
+ notifyDependent，可以被重写以使用 dependent 的 getDependencies 值来决定是否需要重新构建 dependent。
+ InheritedModel，这是一个使用此方法来管理依赖值的示例类。

```dart
  @protected
  void setDependencies(Element dependent, Object? value) {
    _dependents[dependent] = value;
  }
```

## updateDependencies

&emsp;在添加新的依赖者时被 dependOnInheritedWidgetOfExactType 调用。

&ems;每个依赖者 Element 可以通过 setDependencies 映射到单个对象值。该方法可以使用 getDependencies 查找现有的依赖者。

&emsp;默认情况下，此方法将依赖值设置为 null。这只用于记录对依赖者 dependent 的无条件依赖。

&emsp;子类可以管理自己的依赖者值，以便它们可以有选择地重建执行 notifyDependent 的依赖者。

&emsp;另请参见：

+ getDependencies，返回 dependent Element 的当前值。
+ setDependencies，为依赖者 Element 设置值。
+ notifyDependent，可以被重写以使用 dependent 的 getDependencies 值来决定是否需要重新构建 dependent。
+ InheritedModel，这是一个使用此方法来管理依赖值的示例类。

```dart
  @protected
  void updateDependencies(Element dependent, Object? aspect) {
    setDependencies(dependent, null);
  }
```

## notifyDependent

&emsp;对于每个依赖者，notifyClients 会调用该方法。

&emsp;默认情况下，会调用 dependent.didChangeDependencies()。（还记得吗？StatefulElement 会通过此，调用 State.didChangeDependencies 函数。）

&emsp;子类可以重写此方法，根据 getDependencies 的值有选择地调用 didChangeDependencies

```dart
  void notifyDependent(covariant InheritedWidget oldWidget, Element dependent) {
    dependent.didChangeDependencies();
  }
```

## removeDependent

&emsp;由 Element.deactivate 调用，从该 InheritedElement 中移除所提供的依赖者 Element。

&emsp;在依赖者被移除后，当该 InheritedElement 通知其依赖者时，Element.didChangeDependencies 将不再在其上调用。

&emsp;子类可以重写此方法以释放为给定依赖者保留的任何资源。

```dart
  @protected
  @mustCallSuper
  void removeDependent(Element dependent) {
    _dependents.remove(dependent);
  }
```

## updated

&emsp;如果 InheritedWidget.updateShouldNotify 返回 true，则调用所有依赖者 Element 的 Element.didChangeDependencies 方法。（即我们在 InheritedWidget 中重写 updateShouldNotify 函数，返回 Bool 来指示哪些 InheritedWidget 的数据发生变化了，才会通知依赖者进行重建。）

&emsp;在 build 之前立即调用 update。

&emsp;调用 notifyClients 来实际触发通知。

```dart
  @override
  void updated(InheritedWidget oldWidget) {
    if ((widget as InheritedWidget).updateShouldNotify(oldWidget)) {
      super.updated(oldWidget);
    }
  }
```

## notifyClients

&emsp;通过调用 Element.didChangeDependencies，通知所有依赖于此 InheritedWidget 的 widget 已更改。

&emsp;此方法只能在构建阶段调用。通常情况下，当 InheritedWidget 重新构建时，例如通过  InheritedWidget 上方调用 State.setState，此方法会被自动调用。

&emsp;另请参阅：

+ InheritedNotifier，InheritedWidget 的一个子类，当其 Listenable 发送通知时也会调用此方法。

```dart
  @override
  void notifyClients(InheritedWidget oldWidget) {
    for (final Element dependent in _dependents.keys) {
      notifyDependent(oldWidget, dependent);
    }
  }
```

## InheritedElement 总结

&emsp;这下终于看清 ProxyElement.notifyClients 这个抽象函数的用意了。

&emsp;首先是 ProxyElement.notifyClients 函数的调用时机，全部逻辑体现在 ProxyElement.update 函数内：在 ProxyElement 进行更新其 Widget 时，首先会更新 ProxyElement 对象的 `_widget` 属性值，然后以 ProxyElement 的旧 `_widget` 属性值为参数调用 updated 函数，而在 updated 函数内部则是以旧 `_widget` 为参数调用 ProxyElement.notifyClients 函数，然后在最后才是调用 rebuild(force: true)。

&emsp;即可以理解 ProxyElement.notifyClients 函数的调用时机在 ProxyElement 的 Widget 属性赋新值之后，在 ProxyElement 执行强制重建之前。

&emsp;然后则是 ProxyElement 的子类对 notifyClients 这个抽象函数的不同实现：在 ParentDataElement 中它用于向其子孙 RenderObjectElement 节点的 renderObject 属性应用 ParentDataWidget 中的 parent data。而在 InheritedElement 中它则用于遍历当前所有的依赖者 Element 执行它们的 didChangeDependencies，而 Element.didChangeDependencies 的默认实现就是调用 Element.markNeedsBuild 函数，即所有依赖者 Element 被标记重建。

&emsp;然后就是 InheritedElement 实现这些逻辑的细节了：首先 InheritedElement 使用 `Map<Element, Object?> _dependents` 属性来记录每个依赖此 InheritedElement 对象的依赖者 Element 以及其依赖值。然后还添加了 获取、设置、更新、移除 `_dependents` 中依赖者 Element 的接口，供外部使用。然后最重要的还重写了 ProxyElement.updated 函数，在其中以旧 Widget 为参数调用 InheritedWidgt.updateShouldNotify 函数，根据其 bool 返回值决定是否通知依赖者 Element，而我们则可以通过重写 InheritedWidget 的 updateShouldNotify 函数来根据具体情况，哪些数据发生了变化了才指示 InheritedElement 通知其依赖者 Element 进行更新重建。（InheritedWidget.updateShouldNotify 是一个抽象函数，我们必须在 InheritedWidget 子类中重写此函数。）     

&emsp;然后还有一个超级重要的 `InheritedElement._updateInheritance` 函数，InheritedElement 重写了 `_updateInheritance` 函数，在 Element 的默认实现中此函数只是把父 Element 节点的 `_inheritedElements` 传递给子 Element 节点，而到了 InheritedElement 时，它则是读取父 Element 节点的 `_inheritedElements` 属性，并把自己添加到 `_inheritedElements` 中，这样就保证了我们可以在 Element 链中向上以 O(1) 的时间复杂度读取到距离当前 Element 节点最近的指定类型的 InheritedElement。

## 参考链接
**参考链接:🔗**
+ [ComponentElement class](https://api.flutter.dev/flutter/widgets/ComponentElement-class.html)
+ [ProxyElement class](https://api.flutter.dev/flutter/widgets/ProxyElement-class.html)
+ [InheritedElement class](https://api.flutter.dev/flutter/widgets/InheritedElement-class.html)
