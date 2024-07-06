# Flutter 源码梳理系列（十七）：ComponentElement class

# ComonentElement 

&emsp;ComponentElement：一个组合其他 Element 的 Element。与直接创建 RenderObject 不同(与 RenderObjectElement 相对比)，ComponentElement 通过创建其他 Element 间接地创建 RenderObjects。

+ Object -> DiagnosticableTree -> Element -> ComponentElement

&emsp;ComponentElement 的直接子类：ProxyElement、StatelessElement、StatefulElement。

## Constructors

&emsp;ComponentElement(super.widget)：创建一个使用给定 widget 作为其配置的 ComponentElement 对象。

```dart
abstract class ComponentElement extends Element {
  ComponentElement(super.widget);
  
  // ...
}
```

## `_child`

&emsp;此 ComponentElement 元素的子元素（子节点、子级）。

```dart
  Element? _child;
```

## renderObjectAttachingChild

&emsp;返回此 ComponentElement 的子元素（`_child`），该子元素将向此 Element 的祖先插入一个 RenderObject 以构建渲染树。

&emsp;如果此 Element 没有任何需要将 RenderObject 附加到该 Element 的祖先中的子元素，则返回 null。因此，RenderObjectElement 将返回 null，因为其子元素将其 RenderObject 插入到 RenderObjectElement 本身而非 RenderObjectElement 的祖先中。

&emsp;此外，对于在其内部提升自己独立渲染树且不扩展祖先渲染树的 Element，可能会返回 null。

```dart
  @override
  Element? get renderObjectAttachingChild => _child;
```

## mount

&emsp;我们再熟悉的 mount 函数，ComponentElement 进行了重写，除了直接调用 super.mount(parent, newSlot)，还会执行：`_firstBuild`，即当 ComponentElement 对象挂载到 Element Tree 上后就要进行第一次构建了。因为它还要对自己的 build 函数返回的子级 Widget 进行构建 Element。

```dart
  @override
  void mount(Element? parent, Object? newSlot) {
    super.mount(parent, newSlot);
    
    _firstBuild();
  }
```

## `_firstBuild`

&emsp;StatefulElement 重写了这个方法，以便调用 state.didChangeDependencies 方法。内部的 rebuild 函数最终调用 performRebuild 方法。

```dart
  void _firstBuild() {
    // StatefulElement overrides this to also call state.didChangeDependencies.
    rebuild(); // This eventually calls performRebuild.
  }
```

## performRebuild

&emsp;调用 StatelessWidget 对象的 StatelessWidget.build 方法（对于 stateless widgets），或者调用 State 对象的 State.build 方法（对于 stateful widgets），然后更新 Widget Tree。

&emsp;在挂载(mount)过程中自动调用，以生成第一次构建（firstBuild），并在元素需要更新时通过重建调用（rebuild）。

```dart  
  @override
  @pragma('vm:notify-debugger-on-exception')
  void performRebuild() {
    Widget? built;
    
    try {
      built = build();
    } catch (e, stack) {
      // ...
    } finally {
      // 为了在调用 build() 后再标记元素为干净状态，我们延迟标记操作，
      // 这样在 build() 过程中调用 markNeedsBuild() 将会被忽略。
      
      // 通过 super.performRebuild() 把 _dirty 标记置为 false
      super.performRebuild(); // clears the "dirty" flag
    }
    try {
      _child = updateChild(_child, built, slot);
    } catch (e, stack) {
      // ...
    }
  }
```

## build

&emsp;子类应该重写这个函数，以实际调用适当的构建函数（例如 StatelessWidget.build 或 State.build）来构建它们的 Widget。

&emsp;注意 build 是 ComponentElement 的一个抽象函数，所有的 ComponentElement 子类要自己实现这个 build 函数。

```dart
  @protected
  Widget build();
```

## visitChildren

&emsp;对每个子组件调用传入的参数 visitor 函数。必须由支持拥有子组件的子类进行重写。在 Element 类中 visitChildren 函数是空的。

&emsp;访问子组件的顺序无法保证，但是随着时间推移应该是一致的。

&emsp;在构建期间调用此方法是危险的: 子组件列表可能仍在更新，因此在那时子组件可能尚未构建，或者可能是即将被替换的旧子组件。只有当可以证明子组件已经可用时，才应调用此方法。

```dart
  @override
  void visitChildren(ElementVisitor visitor) {
    if (_child != null) {
      visitor(_child!);
    }
  }
```

## forgetChild

&emsp;从元素的子元素列表中删除给定的子元素(在 ComponentElement 这里则是把自己的 `_child` 属性置为 null)，以准备将该子元素在元素树的其他地方重用。

&emsp;这将更新子元素模型，例如，visitChildren 不再遍历该子元素。

&emsp;在调用此方法时，元素仍将有一个有效的父元素，并且在该父元素的上下文中，子元素的 Element.slot 值也将是有效的。调用此方法后，将调用 deactivateChild 方法来切断与此对象的联系。

&emsp;这个 update 任务是负责更新或创建将替换此子元素的新子元素的。

```dart
  @override
  void forgetChild(Element child) {
    _child = null;
    
    super.forgetChild(child);
  }
```

## ComponentElement 总结

&emsp;OK，ComponentElement 的内容就这么多，它作为一个直接继承自 Element 的抽象类，添加了一个超级重要的抽象方法，也是我们重写过无数次的方法：`Widget build()`，build 可以说是贯穿 Flutter framework 的始终，其实它是 ComponentElement 中定义的，然后还有一个超级重要的属性：`Element? _child`，在 SingleChildRenderObjectElement 中也有一个和此完全一样的属性。正是此 `_child` 和 Element 的 `Element? _parent` 属性，它们两个指针一起构建了 Element Tree：以双向链表结构为基础的树状结构🌲。 

&emsp;然后还有 ComponentElement 对 Element 的 mount、performRebuild 的重载，为接下来的一层一层的构建 Element Tree 奠定了基础。如 Element.mount 完成把当前这个单独的 Element 节点挂载到 Element Tree 上的任务，然后 ComponentElement 的 mount 函数则是在此 Element 节点挂载到树上后，继续进行此 Element 节点的 build，继续构建此 Element 节点的子级。ComponentElement.performRebuild 则实际完成这个任务，把此 Element 节点的子级赋值给它的 `_child` 属性，就这样递归的一级一级的完成以此 Element 节点为根节点的整个 Element 子树的构建。

&emsp;ProxyElement、StatelessElement、StatefulElement 它们作为 ComponentElement 的直接子类，继承了 ComponentElement 的 `_child` 和 `Widget build()` 函数，提供更加具体的实现内容。OK，我们继续往下看！

# StatelessElement

&emsp;StatelessElement：使用 StatelessWidget 作为配置的 Element。

+ Object -> DiagnosticableTree -> Element -> ComponentElement -> StatelessElement

&emsp;StatelessElement 直接继承自 ComponentElement。

## Constructors

&emsp;创建一个 StatelessElement，使用给定的 StatelessWidget 作为其配置。

```dart
class StatelessElement extends ComponentElement {
  StatelessElement(StatelessWidget super.widget);
  
  // ...
}
```

## build

&emsp;直接调用它的 StatelessWidget 的 build 函数，并返回其构建的 Widget。转了一圈终于回到了最初，回到了 StatelessWidget 的 build 函数：`Widget build(BuildContext context)`。

```dart
  @override
  Widget build() => (widget as StatelessWidget).build(this);
```

## update

&emsp;更改用于配置此 StatelessElement 的 StatelessWidget。

&emsp;当父级希望使用不同的 Widget 来配置此 Element 时，框架会调用此函数。新的 Widget 保证具有与旧 Widget 相同的 runtimeType。(执行此 update 函数前已经判断过了，如果不同就执行不到这里了。)

&emsp;此函数仅在 "active" 生命周期状态期间调用。

&emsp;在 Element.update 函数中仅有一行，把 `_widget` 属性赋值为入参 newWidget。在 StatelessElement 里还需要强制执行 StatelessElement 的重建，这毕竟 Widget 都已经发生变化了，肯定要进行强制重建。

```dart
  @override
  void update(StatelessWidget newWidget) {
    super.update(newWidget);
    
    // 对此 StatelessElement 对象进行强制重建。 
    rebuild(force: true);
  }
```

## StatelessElement 总结

&emsp;OK，StatelessElement 的内容就这么多，就这两个函数。主要实现了 ComponentElement 的 build 函数，即直接调用自己的 `widget`(StatelessWidget) 的 build 函数，返回创建的 Widget 对象即可。然后是重写了 Element.update 函数：当 widget 属性更新完毕以后肯定要对此 StatelessElement 节点进行强制重建工作。

# StatefulElement 

&emsp;StatefulElement：使用 StatefulWidget 作为配置的 Element。

+ Object -> DiagnosticableTree -> Element -> ComponentElement -> StatefulElement

&emsp;StatefulElement 直接继承自 ComponentElement。

## Constructors

&emsp;创建一个 StatefulElement，使用给定的 StatefulWidget 作为其配置。

&emsp;当创建一个 StatefulElement 对象时，同时直接调用 StatefulWidget 的 createState 函数为 StatefulElement 创建 State 对象，并直接赋值给 `_state` 属性，同时 state 对象的 `_element` 属性直接用 this(当前的 StatefulElement 对象赋值)，`_widget` 属性同时也用 StatefulWidget 赋值。至此：StatefulElement、State、StatefulWidget 三者的关系确定下来了：StatefulElement 和 State 之间相互引用，而它二者都引用同一个 StatefulWidget。

&emsp;另外一点便是当 StatefulWidget 对象调用 createElement 创建 StatefulElement 对象时，同时对应的 State 对象也会被一起创建出来。另外，当 StatefulElement 对象更新自己的 StatefulWidget 时，State 的 widget 属性也会一起更新，并且它俩保持同步始终指向同一个 StatefulWidget 对象。另外就是 State 对象和 StatefulElement 对象它二者的生命周期是完全一样的。

```dart
class StatefulElement extends ComponentElement {
  StatefulElement(StatefulWidget widget)
      : _state = widget.createState(), super(widget) {
    state._element = this;
    state._widget = widget;
  }
  
  // ...
}
```

## build

&emsp;

```dart
  @override
  Widget build() => state.build(this);
```



## 参考链接
**参考链接:🔗**
+ [ComponentElement class](https://api.flutter.dev/flutter/widgets/ComponentElement-class.html)
+ [StatelessElement class](https://api.flutter.dev/flutter/widgets/StatelessElement-class.html)
+ [StatefulElement class](https://api.flutter.dev/flutter/widgets/StatefulElement-class.html)
