# Flutter 源码梳理系列（十九）：RenderObjectElement

# RenderObjectElement

&emsp;RenderObjectElement：使用 RenderObjectWidget 作为配置的 Element。

&emsp;RenderObjectElement 对象在 Render Tree 中有一个关联的 RenderObject，负责处理诸如布局、绘制和 hit testing 等具体操作。

&emsp;关于 RenderObjectElement 和 RenderObject 的内容看文档的话有点绕，但是呢我们可以用代码来理解这些注释。

&emsp;首先 RenderObjectElement 它作为 Element 的直接子类，它依然沿用我们之前学习的一堆 Element 子类的 Element Tree 体系，以及对应的一堆 Widget 子类的 Widget Tree 体系。我们在前面提到过很多次 Element 的 `_parent` 指针以及 Element 子类的 `_child` 指针，它们两个指针一起构建了 Element Tree：以双向（多向）链表结构为基础的树状结构🌲。

&emsp;其中我们提到了树形结构，但是呢我们基本把所有的目光都集中在了 `_parent` 和 `_child` 这两个 Element? 类型的指针上了，如果仅有它们两个指针是无法构建树形结构的，最多只是个双向链表结构。在下面学习的过程中我们会逐渐引出 MultiChildRenderObjectElement，它呢有一个 `late List<Element> _children` 属性，这个才是在把 Element Tree 扩展为树形结构的关键所在。而 MultiChildRenderObjectElement 就是 RenderObjectElement 的直接子类：`class MultiChildRenderObjectElement extends RenderObjectElement`。

&emsp;然后最最重要的来了，RenderObjectElement 都有一个 RenderObject 属性。如果先在我们大脑里面屏蔽 RenderObject 相关内容的话，我们是完全可以用之前学习的 Element 以及那一堆 Element 的子类的思想来理解 RenderObjectElement 的，它和它的子类依然是沿用着之前的 Element 节点挂载到 Element Tree 上的逻辑体系，一层一层的参与 Element Tree 的构建。

&emsp;而它的不同点就在于它们每个 RenderObjectElement 对象都会有一个 RenderObject 属性，而这些 RenderObject 对象呢，它们则会被单独拿出来构建一颗特殊的树：Render Tree，通过前面的学习我们可以知道：Widget Tree 和 Element Tree 它们中是每个节点一一对应，而到了 Render Tree 这里，则可以理解为从 Element Tree 的根节点开始遍历，每遇到一个 RenderObjectElement 节点的话就拿出它的 RenderObject 对象来，让他们这些 RenderObject 对象根据它们对应的 RenderObjectElement 的 祖先-子孙 关系，构建一颗 Render Tree，可想而知 Render Tree 的节点数量肯定是少于 Element Tree 的节点数量的。然后还有一个 RenderObject 子类：ContainerRenderObjectMixin，它可以包含一组 RenderObject 对象，然后整体看下来：Render Tree 依然是：以双向（多向）链表结构为基础的树状结构🌲。

&emsp;不过，当然 Render Tree 的构建不是我们上面描述的那样在 Element Tree 构建完成以后再遍历 Element 节点构建的，它是随着 Element Tree 一起构建的，如在 Element Tree 一层一层构建时，遇到了 RenderObjectElement 节点，当把它挂载到 Element Tree 上后，就会创建它对应的 RenderObject 对象出来，然后拿着这个 RenderObject 对象，并往上找到上一个最近的 RenderObject 的祖先，把这个新建的 RenderObject 节点附加到它下面，就这样 Render Tree 也是一层一层的跟着构建，所以可以理解为：Widget Tree、Element Tree、Render Tree 是整体一层一层一起构建的。

&emsp;所以在 RenderObjectElement 以及其子类的学习过程中最重要的内容就是它们的 RenderObject 以及 Render Tree 的构建。其它的关于 Element Tree 的部分我们还是可以直接用之前的 InheritedElement、ParentDataElement、StatefulElement 等 Element 子类做对应的知识点迁移。

&emsp;和 ComponentElement 进行对比。有关 Element 生命周期的详细信息，请参阅 Element 中的讨论。

## Writing a RenderObjectElement subclass

&emsp;大多数 RenderObject 使用的三种常见的子模型有：

+ 没有子项的 Leaf Render Object：LeafRenderObjectElement 类处理这种情况。
+ 一个子项：SingleChildRenderObjectElement 类处理这种情况。
+ 一系列子项的链表：MultiChildRenderObjectElement 类处理这种情况。

&emsp;然而，有时候 RenderObject 的子模型更加复杂。也许它有一个二维数组的子项。也许它按需构建子项。也许它包含多个列表。在这种情况下，配置了该 RenderObject 的 Widget 对应的 Element 将是 RenderObjectElement 的一个新子类。

&emsp;这样一个 RenderObjectElement 子类负责管理子节点，特别是该对象的 Element 子节点，以及对应 RenderObject 的 RenderObject 子项。

&emsp;这里主要提到的 RenderObjectElement 的三个子类，我们会在后面进行讲解，关于它们的内容还是比较好理解的，同样它们也是有分别对应的 Widget：LeafRenderObjectWidget、SingleChildRenderObjectWidget、MultiChildRenderObjectWidget，然后它们三个作为 RenderObjectWidget 的子类需要提供抽象函数：

+ `RenderObject createRenderObject(BuildContext context)`

&emsp;以及可以重写另外两个非抽象函数：

+ `void updateRenderObject(BuildContext context, covariant RenderObject renderObject) { }`
+ `void didUnmountRenderObject(covariant RenderObject renderObject) { }`

&emsp;创建 RenderObject、更新 RenderObject、RenderObject 已卸载三个函数，其中最重要的是 createRenderObject 函数，这里不同的 RenderObjectWidget 子类会创建不同的 RenderObject 对象，并且在创建完成后会直接赋值给对应的 RenderObjectElement 子类的 renderObject 属性。

&emsp;然后 RenderObject 类有 `RenderObject? _parent` 指针。RenderObject 的子类 RenderObjectWithChildMixin 有 `RenderObject _child` 指针。RenderObject 的子类 ContainerRenderObjectMixin 则有一个由 `RenderObject _firstChild` 和 `RenderObject _lastChild` 指针构造的 RenderObject 链表。看出来了吗？这和 Element 以及其子类构成的：以双向（多向）链表结构为基础的树状结构🌲是如出一辙的。

### Specializing the getters

&emsp;RenderObjectElement 对象在大部分时间里充当其 widget 和 renderObject 之间的中介。通常建议避免专门化 widget getter（指定类型的 Widget getter），而是在各个调用点处进行强制转换，以避免在此特定实现之外增加额外开销。

```dart
class FooElement extends RenderObjectElement {
  FooElement(super.widget);

  // 专门化 renderObject getter 是可以的，因为它不受性能影响。
  @override
  RenderFoo get renderObject => super.renderObject as RenderFoo;

  void _foo() {
    // 对于 Widget 的 getter 方法，我们更倾向于在本地执行类型转换，
    // 因为这样可以获得更好的整体性能，而且在不需要进行类型转换时会更快。
    final Foo foo = widget as Foo;
    // ...
  }

  // ...
}
```

### Slots

&emsp;每个子 RenderObjectElement 对应一个 RenderObject，应将其附加到该 RenderObjectElement 的 renderObject 属性作为子 RenderObject。（上面述说的：RenderObject 是 Render Tree 上的每个节点，且是一个双向链表结构。）

&emsp;然而，Element 的直接 child Element 可能并非最终产生其对应的实际 RenderObject 的 Element。例如，StatelessElement（StatelessWidget 的 Element）对应于其子 Element 的 RenderObject（由其 StatelessWidget.build 方法返回的其它子级 RenderObjectWidget 创建的）。（即我们上面提到的 Element Tree 中并非都是 RenderObjetElement 对象，而只有 RenderObjectElement 对象才有 RenderObject，当 RenderObjectElement 作为其它非 RenderObjectElement 对象的子级时，它的 RenderObject 对象附加到 Render Tree 上时，需要不断的往上找最近的祖先 RenderObjectElement。）

&emsp;因此，每个子 Element 都会被分配一个 slot token。这是一个仅在该 RenderObjectElement 节点中私有的标识符。当最终生成 RenderObject 的子代准备将其附加到该 Element 的渲染对象（render object）时，它会将该 slot token 传回给该 Element，从而使该 Element 能够廉价地识别应将子 Element renderObject 对象放置在父级 RenderObject 中的哪个位置，相对于其他子级 RenderObject。（这个同 Element，当把 RenderObject 附加到 Render Tree 时也需要一个 slot，来表示它位于父级 RenderObject 的哪个位置。）

&emsp;当父 Element 调用 updateChild 来展开子 Element（请参见下一节）时，会确定子 Element 的 slot。可以通过调用 updateSlotForChild 来更新子 Element 的 slot。

### Updating children

&emsp;在 Element 的生命周期早期，Flutter framework 会调用 mount 方法。此方法应该为每个子 Element 调用 updateChild，传入该子 Element 的 Widget 和 slot，从而获取一个子 Element 的列表。

&emsp;随后，Flutter framework 会调用 update 方法。在这个方法中，RenderObjectElement 应该为每个子 Element 调用 updateChild，传入在 mount 期间获取的 Element，或者上次运行 update 时获取的 Element（以最近发生的为准），新的 Widget 以及 slot。这将为对象提供一个新的 Element 对象列表。

&emsp;在可能的情况下，update 方法应该尝试将上一次通行的 Element 映射到新通行的 Widget 上。例如，如果上一次通行的 Element 中有一个是通过特定 Key 配置的，而这次新通行中的一个 Widget 也有相同的 key，它们应该被配对，并且旧 Element 应该使用新 Widget 进行更新（并且还应该更新为新 Widget 新位置对应的占位符）。在这方面，updateChildren 方法可能是有用的。(关于这个 the last pass 不知道是什么意思，大概就是指 updateChildren 函数中传递来的旧 Element 列表中的 Element 对象。关于 updateChildren 的详细分析可以看之前的文章，有超详细的分析。)

&emsp;updateChild 应该按照它们的逻辑顺序为子 Element 调用。顺序可能很重要；例如，如果两个子 Element 在它们的构建方法中都使用 PageStorage 的 writeState 功能（而且都没有 Widget.key），那么第一个子 Element 写入的状态将被第二个子 Element 覆盖。

#### Dynamically determining the children during the build phase

&emsp;子 Widget 不一定必须直接从该 Element 的 Widget 中获取。它们可以通过回调动态生成，或以其他更有创意的方式生成。

#### Dynamically determining the children during layout

&emsp;如果 Widget 在布局时间生成，那么在 mount 和 update 方法中生成它们是行不通的：在那时，该 RenderObjectElement 的 RenderObject 的布局尚未开始。相反，update 方法可以将 RenderObject 标记为需要布局（参见 RenderObject.markNeedsLayout），然后 RenderObject 的 RenderObject.performLayout 方法可以回调到 RenderObjectElement，让它生成 Widget 并相应地调用 updateChild。

&emsp;在布局过程中，若 RenderObject 需要调用一个 Element，则必须使用 RenderObject.invokeLayoutCallback。若一个 Element 在其 update 方法之外需要调用 updateChild，则必须使用 BuildOwner.buildScope。

&emsp;在正常操作过程中，Flutter framework 会提供比在布局期间进行构建时更多的检查。因此，使用具有布局时构建语义的 Widget 应该谨慎进行。

#### Handling errors when building

&emsp;如果一个 Element 调用一个构建器函数（builder）来获取它的子 Element Widget，可能会发现构建过程抛出异常。这样的异常应该被捕获并使用 FlutterError.reportError 进行报告。如果在这种情况下需要一个子 Element 但构建器失败了，那么可以使用 ErrorWidget 的实例代替。

### Detaching children

&emsp;在使用 GlobalKey 时，有可能会出现一个子 Element 在此 Element 更新之前被另一个 Element 主动移除的情况。（具体来说，当以特定 GlobalKey 为根的子树从这个 Element 移动到在构建阶段较早处理的 Element 时会发生这种情况。）当发生这种情况时，将调用该 Element 的 forgetChild 方法，该方法会带有对受影响子 Element 的引用。

### Maintaining the render object tree

&emsp;一旦一个后代生成了一个 RenderObject，它将会调用 insertRenderObjectChild 方法。如果后代的插槽（slot）发生更改，它将会调用 moveRenderObjectChild 方法。如果一个后代消失了，它将会调用 removeRenderObjectChild 方法。

&emsp;这三种方法应该相应地更新 Render Tree，分别从该 Element 自己的 RenderObject 中附加、移动和分离给定子 RenderObject。

### Walking the children

&emsp;如果一个 RenderObjectElement 对象有任何子元素 Element，它必须在实现 visitChildren 方法时将它们公开出来。这个方法被许多框架内部机制使用，所以应该快速。它也被测试框架和 debugDumpApp 使用。

+ Object -> DiagnosticableTree -> Element -> RenderObjectElement

&emsp;RenderObjectElement 直接继承自 Element。

&emsp;下面我们开始看 RenderObjectElement 的源码。

## Constructors

&emsp;创建一个 RenderObjectElement，使用给定的 RenderObjectWidget 作为其配置。

&emsp;直接继承自 Element，很常规的构造函数。

```dart
abstract class RenderObjectElement extends Element {
  RenderObjectElement(RenderObjectWidget super.widget);
  
  // ...
}
```

## renderObject

&emsp;该 RenderObjectElement 的底层 RenderObject。如果该 RenderObjectElement 已被卸载，这个 getter 会抛出异常。

&emsp;当 RenderObjectElement 节点挂载到 Element Tree 上时，通过调用 RenderObjectWidget.createRenderObject 函数创建，并赋值给该 RenderObjectElement 对象。

```dart
  @override
  RenderObject get renderObject {
    return _renderObject!;
  }
  
  RenderObject? _renderObject;
```

## renderObjectAttachingChild

&emsp;返回该 Element 的子 Element，该子 Element 将向该 Element 的祖先插入一个 RenderObject 以构建 Render Tree。默认返回 null，由各 RenderObjectElement 子类重写此 getter。

&emsp;如果该 Element 没有任何子 Element 需要将 RenderObject 附加到该 Element 的祖先中，则返回 null。因此，RenderObjectElement 将返回 null，因为其子 Element 将其 RenderObject 插入到 RenderObjectElement 本身而不是 RenderObjectElement 的祖先。

&emsp;此外，这可能对那些提升自己独立渲染树并且不扩展祖先渲染树的 Element 返回 null。

```dart
  @override
  Element? get renderObjectAttachingChild => null;
```

## `_ancestorRenderObjectElement`

&emsp;距离该 RenderObjecdtElement 最近的 RenderObjectElement 类型的祖先 RenderObjectElement，当调用 attachRenderObject 函数把当前 RenderObjectElement 的 RenderObject 附加到 Render Tree 时会对此 `_ancestorRenderObjectElement` 属性赋值。

```dart
  RenderObjectElement? _ancestorRenderObjectElement;
```

## `_findAncestorRenderObjectElement`

&emsp;沿着 parent 指针往上找，直到找到第一个 RenderObjectElement 为止，并把其返回。即离当前 Element 最近的 RenderObjectElement 类型祖先。

```dart
  RenderObjectElement? _findAncestorRenderObjectElement() {
    Element? ancestor = _parent;
    
    // 当遍历到根 Element 节点或者 ancestor 是 RenderObjectElement 类型则结束循环
    while (ancestor != null && ancestor is! RenderObjectElement) {
      // 沿着 _parent 指针继续向上，在实际场景下距离上一个 RenderObjectElement 对象是很近的，
      // 否则这个遍历下来 O(n) 的时间复杂度也太慢了
      ancestor = ancestor?._parent;
    }
        
    return ancestor as RenderObjectElement?;
  }
```

## `_findAncestorParentDataElements`

&emsp;沿着 parent 指针往上找直到遇到第一个 RenderObjectElement 节点为止，把所有的类型是 ParentDataElement 的 Element 收集起来并返回。

```dart
  List<ParentDataElement<ParentData>> _findAncestorParentDataElements() {
    Element? ancestor = _parent;
    
    final List<ParentDataElement<ParentData>> result = <ParentDataElement<ParentData>>[];

    // 多个 ParentDataWidget 可以贡献 ParentData，但有一些约束。
    // 1. ParentData 只能由唯一的 ParentDataWidget 类型写入。例如，两个尝试写入相同子项的 KeepAlive ParentDataWidget 是不允许的。

    // 2. 每个贡献的 ParentDataWidget 必须贡献一个独特的 ParentData 类型，以免 ParentData 被覆盖。例如，如果第一个检查被 KeepAlive ParentDataWidget 的子类替代，那么就不能存在两个同时写入类型为 KeepAliveParentDataMixin 的 ParentDataWidget。

    // 3. ParentData 本身必须与所有写入它的 ParentDataWidget 兼容。举例来说，TwoDimensionalViewportParentData 使用 KeepAliveParentDataMixin，因此它可以兼容 KeepAlive，以及另一个父级数据类型为 TwoDimensionalViewportParentData 或其子类的 ParentDataWidget。 第一和第二种情况在此处得到验证。第三种情况在 debugIsValidRenderObject 中得到验证。
    
    while (ancestor != null && ancestor is! RenderObjectElement) {
    
      if (ancestor is ParentDataElement<ParentData>) {
        result.add(ancestor);
      }
      
      ancestor = ancestor._parent;
    }
    
    return result;
  }
```

## mount

&emsp;重写 Element.mount 函数。主要针对 RenderObject 对象的构建和附加到 Render Tree 中。

&emsp;可以看到之前的 Element 学习中，它们在 mount 后都是执行构建，而在 RenderObjectElement 这里，当把其挂载到 Element Tree 上以后，做的主要的事情就都聚焦到了 RenderObject 上，首先是调用 RenderObjectWidget 的 createRenderObject 函数创建 RenderObject 对象，并直接赋值给自己的 renderObject 属性上。然后便是把此 RenderObject 对象附加到 Render Tree 中。

&emsp;在 SingleChildRenderObjectElement 和 MultiChildRenderObjectElement 中都会重写此函数，执行完 RenderObjecteleElement.mount 后，会继续构建它们的子级 Element。

```dart
  @override
  void mount(Element? parent, Object? newSlot) {
    super.mount(parent, newSlot);
    
    // 构建 RenderObject 对象，并把它添加到 Render Tree 中
    _renderObject = (widget as RenderObjectWidget).createRenderObject(this);
    attachRenderObject(newSlot);
    
    super.performRebuild(); // clears the "dirty" flag
  }
```

## update

&emsp;重写 Element.update 函数。更新完 widget 后，也要进行 RenderObject 的更新。然后再进行重建。

```dart
  @override
  void update(covariant RenderObjectWidget newWidget) {
    super.update(newWidget);
    
    _performRebuild(); // calls widget.updateRenderObject()
  }
```

## performRebuild

&emsp;重写 Element.performRebuild 函数。内部只管调用自己的私有函数 `_performRebuild`。

```dart
  @override
  void performRebuild() { // ignore: must_call_super, _performRebuild calls super.
    _performRebuild(); // calls widget.updateRenderObject()
  }
```

## `_performRebuild`

&emsp;主要目光在 RenderObject 更新。

```dart
  @pragma('dart2js:tryInline')
  @pragma('vm:prefer-inline')
  @pragma('wasm:prefer-inline')
  void _performRebuild() {
    // 主要目光依然是集中在 RenderObject 上，更新 RenderObject
    (widget as RenderObjectWidget).updateRenderObject(this, renderObject);
    
    // 在这里再调用 Element.performRebuild 函数，
    // 内部的话仅是把脏标记置为 false
    super.performRebuild(); // clears the "dirty" flag
  }
```

## deactivate

&emsp;内部同样仅是调用 Element.deactivate。

```dart
  @override
  void deactivate() {
    super.deactivate();
  }
```

## unmount

&emsp;重写 Element.unmount 函数。主要目光在 RenderObject 上，因为 RenderObjectElement 对象再也没有机会在挂载到 Element Tree 上了，所以这里也需要对其 RenderObject 对象进行资源清理。这里有点类似 StatefulElement 卸载时对 State 对象的资源清理。

```dart
  @override
  void unmount() {
    final RenderObjectWidget oldWidget = widget as RenderObjectWidget;
    super.unmount();
    
    // 调用 RenderObjectWidget 的 didUnmountRenderObject 函数
    oldWidget.didUnmountRenderObject(renderObject);
    
    // renderObject 调用 dispose，并把 renderObject 属性置为 null
    _renderObject!.dispose();
    _renderObject = null;
  }
```

## `_updateParentData`

&emsp;更新 ParentData 数据。在当前的 RenderObject 对象上应用 `ParentDataWidget<ParentData>` 的数据。

```dart
  void _updateParentData(ParentDataWidget<ParentData> parentDataWidget) {
    parentDataWidget.applyParentData(renderObject);
  }
```

## updateSlot

&emsp;更新当前 RenderObjectElement 在其祖先中的 Slot。

```dart
  @override
  void updateSlot(Object? newSlot) {
    // 临时变量 oldSlot 记录下之前的旧的 slot 数据
    final Object? oldSlot = slot;
    
    super.updateSlot(newSlot);
    
    // 通过祖先 RenderObjectElement 调用 moveRenderObjectChild 函数，
    // 把当前的 RenderObject 对象移动到新的 newSlot 去。
    _ancestorRenderObjectElement?.moveRenderObjectChild(renderObject, oldSlot, slot);
  }
```

## attachRenderObject

&emsp;将 renderObject 附加到祖先 RenderObject 的指定 newSlot 处。

&emsp;此函数在 Element.attachRenderObject 中的默认实现会在每个子 Element 上递归调用 attachRenderObject 函数。而这个使用场景主要是用在有 GlobalKey 的 newWidget 可以复用 Element 时，把此 Elemet 子树激活时，在 Element 子树上递归调用。即把 Element 子树上的 RenderObject 重新附加到 Render Tree 上。  

&emsp;RenderObjectElement.attachRenderObject 重写了实际将 renderObject 附加到 Render Tree 中的工作。

&emsp;newSlot 参数指定了该 RenderObjectElement 的 RenderObject 在其父级 RenderObject 中的的 slot 值。

```dart
  @override
  void attachRenderObject(Object? newSlot) {
    _slot = newSlot;
    
    // 找到距离当前 RenderObjectElement 最近的祖先 RenderObjectElement
    _ancestorRenderObjectElement = _findAncestorRenderObjectElement();
    
    // 然后把当前 RenderObjectElement 的 renderObject 插入到祖先 RenderObjectElement 的 RenderObject 中去，
    // 把 RenderObject 对象附加到 Render Tree 上时和 Element 挂载到 Element Tree 上类似，
    // 就代码角度而言的话，就是 paernt、child 等 RenderObject 指针的赋值。
    _ancestorRenderObjectElement?.insertRenderObjectChild(renderObject, newSlot);
    
    // 找到当前节点到最近的 RenderObjectElement 祖先节点中所有的 ParentDataElement 节点
    final List<ParentDataElement<ParentData>> parentDataElements = _findAncestorParentDataElements();
    
    // 遍历这些 ParentDataElement 把它们的 ParentDataWidget 中的数据都应用到此 RenderObjectElement 对象的 RenderObject 属性
    for (final ParentDataElement<ParentData> parentDataElement in parentDataElements) {
      _updateParentData(parentDataElement.widget as ParentDataWidget<ParentData>);
    }
  }
```

## detachRenderObject

&emsp;从 Render Tree 中移除 renderObject。

&emsp;此函数在 Element.detachRenderObject 中的默认实现会在每个子 Element 上递归调用 detachRenderObject 函数。使用场景在 Element 节点被失活时，在此 Element 子树上的节点上都调用 detachRenderObject 函数。即把此 Element 子树上的 RenderObject 都从 Render Tree 中分离。

&emsp;而到了 RenderObjectElement 这里 RenderObjectElement.detachRenderObject 则是进行了重写实际执行将 renderObject 从 Render Tree 中移除的工作。

&emsp;此函数由 deactivateChild 调用。

```dart
  @override
  void detachRenderObject() {
    // 从父级 RenderObject 中移除当前 renderObject
    if (_ancestorRenderObjectElement != null) {
      _ancestorRenderObjectElement!.removeRenderObjectChild(renderObject, slot);
      _ancestorRenderObjectElement = null;
    }
    
    _slot = null;
  }
```

## insertRenderObjectChild

&emsp;将给定的当前 RenderObjectElement 的 RenderObject 插入到父级 RenderObject 的指定 slot。

&emsp;slot 的语义由当前 RenderObjectElement 来确定。例如，如果该 RenderObjectElement 只有一个子 Element，例如当前是 SingleChildRenderObjectElement 的话，则 slot 应始终为 null，在 SingleChildRenderObjectElement.insertRenderObjectChild 函数内部会直接忽略这个 slot 参数。如果该 RenderObjectElement 有一组子 Element，则上一个兄弟 Element 包装在 IndexedSlot 中，是 IndexedSlot slot 构造函数的 value 参数值。例如 MultiChildRenderObjectElement.insertRenderObjectChild 函数内部使用 slot 参数。

```dart
  @protected
  void insertRenderObjectChild(covariant RenderObject child,
                               covariant Object? slot);
```

## moveRenderObjectChild

&emsp;将给定的 RenderObject child 从给定的旧 slot 移动到给定的新 solt。在 SingleChildRenderObjectElement 中此函数内部为空，因为 SingleChildRenderObjectElement 就一个 RenderObject，且它的 slot 为 null，它没有什么移动不移动的操作可言。

&emsp;保证给定的入参 child RenderObject 的父级，是当前的 RenderObjectElement 的 renderObject 属性。

&emsp;只有在 updateChild 可能会使用具有不同于先前给定的 Slot 的 slot 与现有 Element 子 Element 一起被调用时，才会调用此方法。例如，MultiChildRenderObjectElement 这样做。SingleChildRenderObjectElement 不会这样做（因为 slot 始终为 null）。具有每个子节点始终具有相同 slot 的特定 slot 集的元素（并且不会将位于不同 slot 中的子节点相互比较，以更新另一个 slot 中的元素）永远不会调用这种方法。

```dart
  @protected
  void moveRenderObjectChild(covariant RenderObject child,
                             covariant Object? oldSlot,
                             covariant Object? newSlot);
```

## removeRenderObjectChild

&emsp;从当前的 RenderObjectElement 的 renderObject 中移除指定的 renderObject。确保给定的子项已经插入到给定的 slot 中，并且其 renderObject 为其父级。即需要保证入参 RenderObject child 的 parent 指针指向的正是当前 RenderObjectelement 的 renderObject 属性。或者保证当前 RenderObjectElement 的 renderObject 属性的 child 指针，指向的正是入参 RenderObject child。

```dart
  @protected
  void removeRenderObjectChild(covariant RenderObject child,
                               covariant Object? slot);
```

## RenderObjectElement 总结

&emsp;OK，到这里 RenderObjectElement 的内容就看完了。几乎全部内容都是围绕 RenderObject 展开的，它相比于其他 ComponentElement 系 Element，而言多了一个 renderObject 属性，而正是它们的 RenderObject 构建了完整的 Render Tree。而 Render Tree 的构建类似于 Element 的 parent/child 指针赋值来构建，这里则是通过 RenderObject 的 parent/child 指针来进行，同样是一个以双向（多向）链表结构为基础的树状结构🌲。

&emsp;还有其中的沿着 Element 的 `_parent` 指针向上查找 RenderObjectElement 和 ParentDataElement 节点的操作让人印象深刻。OK，就先到这里吧，后续的内容细节我们在 SingleChildRenderObjectElement 和 MultiChildRenderObjectElement 中再展开。

## 参考链接
**参考链接:🔗**
+ [RenderObjectElement class](https://api.flutter.dev/flutter/widgets/RenderObjectElement-class.html)
+ [LeafRenderObjectElement class](https://api.flutter.dev/flutter/widgets/LeafRenderObjectElement-class.html)
+ [SingleChildRenderObjectElement class](https://api.flutter.dev/flutter/widgets/SingleChildRenderObjectElement-class.html)
+ [MultiChildRenderObjectElement class](https://api.flutter.dev/flutter/widgets/MultiChildRenderObjectElement-class.html)
