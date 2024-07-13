# Flutter 源码梳理系列（二十）：LeafRenderObjectElement、SingleChildRenderObjectElement、MultiChildRenderObjectElement

# LeafRenderObjectElement

&emsp;LeafRenderObjectElement：使用 LeafRenderObjectWidget 作为配置的 Element。

+ Object -> DiagnosticableTree -> Element -> RenderObjectElement -> LeafRenderObjectElement

&emsp;LeafRenderObjectElement 直接继承自 RenderObjectElement。

## Constructors

&emsp;创建一个 LeafRenderObjectElement，使用给定的 LeafRenderObjectWidget 作为其配置。

```dart
class LeafRenderObjectElement extends RenderObjectElement {
  LeafRenderObjectElement(LeafRenderObjectWidget super.widget);
  
  // ...
}
```

## Other

&emsp;然后下面是一组：forgetChild、insertRenderObjectChild、moveRenderObjectChild、removeRenderObjectChild 函数的空实现。

&emsp;试着全局搜了一下没找到有任何 LeafRenderObjectElement 的子类相关的内容，但是 LeafRenderObjectWidget 的子类倒是不少。后续遇到与其相关的内容后再展开。

```dart
  @override
  void forgetChild(Element child) {
    super.forgetChild(child);
  }

  @override
  void insertRenderObjectChild(RenderObject child, Object? slot) { }

  @override
  void moveRenderObjectChild(RenderObject child, Object? oldSlot, Object? newSlot) { }

  @override
  void removeRenderObjectChild(RenderObject child, Object? slot) { }
```

# SingleChildRenderObjectElement

&emsp;SingleChildRenderObjectElement：使用 SingleChildRenderObjectWidget 作为配置的 Element。

&emsp;child 是可选的。

&emsp;这个 Element 的子类可以用于 RenderObjectWidget，其 RenderObject 使用 RenderObjectWithChildMixin mixin。这样的 Widget 被期望继承自 SingleChildRenderObjectWidget。（RenderObjectWithChildMixin mixin 是 RenderObject 的子类，后续我们再深入学习。）

+ Object -> DiagnosticableTree -> Element -> RenderObjectElement -> SingleChildRenderObjectElement

&emsp;SingleChildRenderObjectElement 直接继承自 RenderObjectElement。

## Constructors

&emsp;创建一个 SingleChildRenderObjectElement，使用给定的 SingleChildRenderObjectWidget 作为其配置。

```dart
class SingleChildRenderObjectElement extends RenderObjectElement {
  SingleChildRenderObjectElement(SingleChildRenderObjectWidget super.widget);
  
  // ...
}
```

## `_child`

&emsp;同 ComponentElement 一样，也提供了一个 child 指针，用于指向自己的子级 Element。

```dart
  Element? _child;
```

## visitChildren

&emsp;对每个子级调用 visitor 函数。必须由支持拥有子级的 Element 子类重写此函数。

&emsp;重写 Element.visitChildren，同 ComponentElement 一样，遍历自己的 child。

```dart
  @override
  void visitChildren(ElementVisitor visitor) {
    if (_child != null) {
      visitor(_child!);
    }
  }
```

## forgetChild

&emsp;从 Element 的子 Element 列表中移除给定的子 Element，为了让该子 Element 在 Element Tree 的其他地方被重用做准备。

&emsp;这会更新子 Element 的模型，使得比如 visitChildren 不再遍历该子 Element。

&emsp;在调用此方法时，Element 仍将有一个有效的父级，并且子 Element 的 Element.slot 值将在该父级上下文中有效。调用此方法后，会调用 deactivateChild 来切断与这个对象的联系。

&emsp;更新的工作是负责更新或创建将取代该子 Element 的新子 Element。

&emsp;需要保证入参 child 和自己的 `_child` 属性相等。

```dart
  @override
  void forgetChild(Element child) {
    _child = null;
    super.forgetChild(child);
  }
```

## mount

&emsp;重写 RenderObjectElement.mount 函数。主要添加对 `_child` 的新建/更新/卸载。

```dart
  @override
  void mount(Element? parent, Object? newSlot) {
    super.mount(parent, newSlot);
    
    // 子级 Element 新建
    _child = updateChild(_child, (widget as SingleChildRenderObjectWidget).child, null);
  }
```

## update

&emsp;重写 RenderObjectElement.update 函数。

```dart
  @override
  void update(SingleChildRenderObjectWidget newWidget) {
    // widget 属性已更新
    // renderObject 属性已更新
    super.update(newWidget);
    
    // 然后更新 _child，子级 Element 新建/更新/卸载
    _child = updateChild(_child, (widget as SingleChildRenderObjectWidget).child, null);
  }
```

## insertRenderObjectChild

&emsp;首先 SingleChildRenderObjectElement 的 renderObject 属性是 RenderObjectWithChildMixin 类型的，它有一个指向 RenderObject 的 child 指针，然后把入参 child RenderObject 赋值给它即可。由于仅有这一个 child 所以 slot 直接忽略即可。

&emsp;这样入参 RenderObject child 就被附加到 Render Tree 上去了。

```dart
  @override
  void insertRenderObjectChild(RenderObject child, Object? slot) {
    final RenderObjectWithChildMixin<RenderObject> renderObject = this.renderObject as RenderObjectWithChildMixin<RenderObject>;
    
    renderObject.child = child;
  }
```

## moveRenderObjectChild

&emsp;因为 SingleChildRenderObjectElement 的 renderObject 属性是 RenderObjectWithChildMixin，它只有一个 child，所以不存在 move RenderObject 行为，所以此函数内部实现为空。

```dart
  @override
  void moveRenderObjectChild(RenderObject child, Object? oldSlot, Object? newSlot) {
    // ...
  }
```

## removeRenderObjectChild

&emsp;从当前 SingleChildRenderObjectElement 的 renderObject 中移除给定的子节点 RenderObject。保证给定的子节点 RenderObject 已经被插入到指定的 slot，并且其 renderObject 作为其父级。

&emsp;需要保证入参 child RenderObjedt 是当前 SingleChildRenderObject 的 renderObject 属性的 child 属性。

```dart
  @override
  void removeRenderObjectChild(RenderObject child, Object? slot) {
    final RenderObjectWithChildMixin<RenderObject> renderObject = this.renderObject as RenderObjectWithChildMixin<RenderObject>;
    
    // 开始时 renderObject.child 等于入参 child
    // 然后直接把 renderObject 的 child 直接置 null 即可
    renderObject.child = null;
  }
```

## SingleChildRenderObjectElement 总结

&emsp;OK，SingleChildRenderObjectElement 的内容看完了，终于看到它的 `Element? _child` 指针了，指向自己的子级 Element 节点，继续延续 Element Tree 的构建，重写了 mount 函数，当把当前 SingleChildRenderObjectElement 对象挂载到 Element Tree 上后，直接读取 SingleChildRenderObjectElement 的 SingleChildRenderObjectWidget 的 child，调用 updateChild 构建 SingleChildRenderObjectElement 的 child。

&emsp;然后是关于 RenderObject 属性的：更新/插入/移动/移除 相关的操作有序的进行，内容都比较简单，快速浏览过一遍即可。

# MultiChildRenderObjectElement

&emsp;MultiChildRenderObjectElement：使用 MultiChildRenderObjectWidget 作为配置的 Element。

&emsp;这个 Element 子类可用于 RenderObjectWidget，其 RenderObject 使用 ContainerRenderObjectMixin mixin，parent data 类型实现 `ContainerParentDataMixin<RenderObject>` mixin。这样的 Widget 应该继承自 MultiChildRenderObjectWidget。（ContainerRenderObjectMixin mixin 是 RenderObject 的子类，后续我们再深入学习。）

+ Object -> DiagnosticableTree -> Element -> RenderObjectElement -> MultiChildRenderObjectElement

## Constructors

&emsp;创建一个 MultiChildRenderObjectElement，使用给定的 MultiChildRenderObjectWidget 作为其配置。

```dart
class MultiChildRenderObjectElement extends RenderObjectElement {
  MultiChildRenderObjectElement(MultiChildRenderObjectWidget super.widget);
  
  // ...
}
```

## renderObject

&emsp;重写 renderObject getter，转换为指定的 ContainerRenderObjectMixin 类型。

```dart
  @override
  ContainerRenderObjectMixin<RenderObject, ContainerParentDataMixin<RenderObject>> get renderObject {
    return super.renderObject as ContainerRenderObjectMixin<RenderObject, ContainerParentDataMixin<RenderObject>>;
  }
```

## children

&emsp;该 MultiChildRenderObjectElement 当前的子 Element 列表。此列表已经被筛选，隐藏了通过 forgetChild 方法被遗忘的 Element。（即过滤了位于 forgottenChildren 中的子 Element。）

```dart
  @protected
  @visibleForTesting
  Iterable<Element> get children => _children.where((Element child) => !_forgottenChildren.contains(child));

  late List<Element> _children;
```

## `_forgottenChildren`

&emsp;保存一组被遗忘的 children，以避免反复进行 O(n^2) 的工作，去逐步移除 children。

```dart
  final Set<Element> _forgottenChildren = HashSet<Element>();
```

## insertRenderObjectChild

&emsp;调用 ContainerRenderObjectMixin.insert 函数。

```dart
  @override
  void insertRenderObjectChild(RenderObject child, IndexedSlot<Element?> slot) {
    final ContainerRenderObjectMixin<RenderObject, ContainerParentDataMixin<RenderObject>> renderObject = this.renderObject;
    
    renderObject.insert(child, after: slot.value?.renderObject);
  }
```

## moveRenderObjectChild

&emsp;调用 ContainerRenderObjectMixin.move 函数。需要保证入参 child RenderObject 的 parent 是当前 MultiChildRenderObjectElement 的 renderObject。

```dart
  @override
  void moveRenderObjectChild(RenderObject child, IndexedSlot<Element?> oldSlot, IndexedSlot<Element?> newSlot) {
    final ContainerRenderObjectMixin<RenderObject, ContainerParentDataMixin<RenderObject>> renderObject = this.renderObject;
    
    renderObject.move(child, after: newSlot.value?.renderObject);
  }
```

## removeRenderObjectChild

&emsp;调用 ContainerRenderObjectMixin.remove 函数。需要保证入参 child RenderObject 的 parent 是当前 MultiChildRenderObjectElement 的 renderObject。

```dart
  @override
  void removeRenderObjectChild(RenderObject child, Object? slot) {
    final ContainerRenderObjectMixin<RenderObject, ContainerParentDataMixin<RenderObject>> renderObject = this.renderObject;
    renderObject.remove(child);
  }
```

## visitChildren

&emsp;直接遍历子 Element 列表中的非位于 forgottenChildren 中的 Element 对象，调用 visitor 函数。

```dart
  @override
  void visitChildren(ElementVisitor visitor) {
    for (final Element child in _children) {
    
      // 排除位于 forgottenChildren 中的子 Element
      if (!_forgottenChildren.contains(child)) {
        visitor(child);
      }
    }
  }
```

## forgetChild

&emsp;把指定的位于当前 children 中的某个 Element 对象添加到 `_forgottenChildren` 集合中。

```dart
  @override
  void forgetChild(Element child) {
    _forgottenChildren.add(child);
    super.forgetChild(child);
  }
```

## inflateWidget

&emsp;重写 Element.inflateWidget 函数，但是实际内容并没有添加什么。主要给下面的 mount 调用。

```dart
  @override
  Element inflateWidget(Widget newWidget, Object? newSlot) {
    final Element newChild = super.inflateWidget(newWidget, newSlot);
    
    return newChild;
  }
```

## mount

&emsp;超级重要的 mount 函数。首先调用 Element.mount 函数把自己挂载到 Element Tree 上去，然后调用 RenderObjectElement.mount 函数把 renderObject 附加到 Render Tree 上去。然后便是把 MultiChildRenderObjectWidget 的 children 中的所有 Widget 对象全部 inflate 为对应类型的 Element 对象（每一个 Element 对象都代表的是一个 Element 子树），然后把它们装在 List 中赋值给 MultiRenderObjectElement 的 children 属性。

```dart
  @override
  void mount(Element? parent, Object? newSlot) {
    // 当前 Element 节点挂载在 Element Tree 上，
    // 当前 Element 节点的 renderObject 附加到 Render Tree 上
    super.mount(parent, newSlot);
    
    // 取得 MultiChildRenderObjectWidget 对象
    final MultiChildRenderObjectWidget multiChildRenderObjectWidget = widget as MultiChildRenderObjectWidget;
    
    // 准备一个同 multiChildRenderObjectWidget.children 长度相同的 List，内部用 _NullElement.instance 占位
    final List<Element> children = List<Element>.filled(multiChildRenderObjectWidget.children.length, _NullElement.instance);
    
    // 为每个子 Element 节点的 slot 的 value 值，IndexedSlot 的内容还记得吗？不记得的话翻翻前面的 updateChildren 篇，
    // 每个子 Element 的 slot 类型是 IndexedSlot，它的 index 值记录此子 Element 在 children 中的索引值，
    // value 值是一个 Element? 类型，会用来记录当前子 Element 前面的一个子 Element，
    // 这个 previousChild 值就是用来记录前面的一个子 Element 的。
    Element? previousChild;
    
    // 遍历把所有的 widget.children 中的 Widget 对象都 inflate 为对应类型的 Element 对象（每个都一个 Element 子树） 
    for (int i = 0; i < children.length; i += 1) {
      final Element newChild = inflateWidget(multiChildRenderObjectWidget.children[i], IndexedSlot<Element?>(i, previousChild));
      
      children[i] = newChild;
      previousChild = newChild;
    }
    
    // 然后赋值给 _children 属性
    _children = children;
  }
```

## update

&emsp;超级重要的 update 函数。首先调用 Element.update 完成 widget 的更新，然后调用 RenderObjectWidget.updateRenderObject 完成 renderObject 的更新，然后便是最为出名的 Element.updateChildren 的调用，用新的 Widget 列表，更新旧的 Element 列表。

```dart
  @override
  void update(MultiChildRenderObjectWidget newWidget) {
    // 完成 _widget 属性的更新
    // 完成 RenderObjectWidget.updateRenderObject 的更新
    super.update(newWidget);
    
    // 取得新的 Wieget 属性
    final MultiChildRenderObjectWidget multiChildRenderObjectWidget = widget as MultiChildRenderObjectWidget;

    // 然后 Element.updateChildren 的调用，可以翻前面的文章，有超级详细的解析。
    _children = updateChildren(_children, multiChildRenderObjectWidget.children, forgottenChildren: _forgottenChildren);
    
    // 然后把 _forgottenChildren 集合清空即可。
    _forgottenChildren.clear();
  }
```

## MultiChildRenderObjectElement 总结

&emsp;OK，MultiChildRenderObjectElement 的内容看完了，终于看到它的 `late List<Element> _children` 列表了，一组子 Element 节点，正是这一组子 Element 节点，把单调的 Element 双向链表扩展为了树形结构。

&emsp;重点内容聚焦在了 MultiChildRenderObjectElement.mount 函数：当把当前 MultiChildRenderObjectElement 节点挂载到 Element Tree 上以后，便开始遍历 MultiChildRenderObjectWidget 中的一组 Widget 对象，分别进行子 Element Tree 的构建。

&emsp;然后还有 MultiChildRenderObjectElement.update 中对一组旧 Element 对象的重建，用到了 Element.updateChildren 函数，然后其中的对同一个父级的一组 Element 对象有 key 时的复用逻辑，让人印象深刻，如果我们想要搞懂 LocalKey 相关的内容，Element.updateChildren 是必学的内容，前面有超详细的解析，可以翻回去看看。

&emsp;然后还有 ContainerRenderObjectMixin 相关的 RenderObject 的 insert/move/remove 相关的内容，我们到 RenderObject 篇再进行详细解读。

## 参考链接
**参考链接:🔗**
+ [RenderObjectElement class](https://api.flutter.dev/flutter/widgets/RenderObjectElement-class.html)
+ [LeafRenderObjectElement class](https://api.flutter.dev/flutter/widgets/LeafRenderObjectElement-class.html)
+ [SingleChildRenderObjectElement class](https://api.flutter.dev/flutter/widgets/SingleChildRenderObjectElement-class.html)
+ [MultiChildRenderObjectElement class](https://api.flutter.dev/flutter/widgets/MultiChildRenderObjectElement-class.html)
