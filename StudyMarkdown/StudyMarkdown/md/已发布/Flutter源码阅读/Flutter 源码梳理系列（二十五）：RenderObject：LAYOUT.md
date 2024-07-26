# Flutter 源码梳理系列（二十五）：RenderObject：LAYOUT

# RenderObject：LAYOUT section

&emsp;RenderObject 是 Render Tree 中的一个对象（或一个节点）。

&emsp;RenderObject 类层次结构是渲染库存在的核心原因。[RenderObjects?! | Decoding Flutter](https://www.youtube.com/watch?v=zmbmrw07qBc)

&emsp;RenderObject 具有父级，并且有一个称为 parentData 的 slot，父 RenderObject 可以在其中存储特定于子级的数据，例如子级位置。RenderObject 类还实现了基本的布局和绘制协议。（对这段文档进行解释：首先每个 RenderObject 对象都有一个 RenderObject? parent 指针，即自己的父级，然后还有一个 ParentData? parentData 属性，这个是它的父级传递来的数据，类型是 ParentData 的，可以被直接作用到自己身上，这里有点绕，从代码角度看其实就是我们之前在 ParentDataWidget 中见到的 `void applyParentData(RenderObject renderObject)` 函数，这个函数就是把当前 ParentDataWidget 对象中的数据沿着 Element Tree 应用到自己的子级 RenderObjectElement 节点的 RenderObject 上。例如我们日常使用的 Positioned Widget，它就是一个 ParentDataWidgt 子类，我们可以直接看它的 applyParentData 函数实现，就是把自己的 left/right/top/bottom 等这些数据赋值给 RenderObject 的 parentData 属性，然后标记 RenderObject 的父级需要重新布局（通过 renderObject.parent.markNeedsLayout））。

&emsp;然而，RenderObject 类：

+ 没有定义子级模型（例如，一个节点是否有零个、一个或多个子级）。（但是 Framework 提供了几个 RenderObject Mixin。）
+ 它也没有定义坐标系（例如，子级是在笛卡尔坐标中定位，还是在极坐标中定位等）。
+ 也没有特定的布局协议（例如，布局是宽度输入高度输出，约束输入尺寸输出，还是父级在子级布局之前或之后设置大小和位置等；或者子级是否允许读取其父级的 parentData slot）。

&emsp;RenderBox 作为 RenderObject 的直接子类其布局系统使用了笛卡尔坐标系（同 iOS，屏幕左上角是原点，X 轴向右增大，Y 轴向下增大）。

&emsp;RenderBox 是一个直接继承自 RenderObject 的抽象类。大部分的 RenderObjectWidget 都会使用 RenderBox 的子类作为它们的 RednerObjectWidget.createRenderObject 函数返回的 RenderObject，而不是直接使用 RenderObject 的子类。

&emsp;大概意思就是：RenderObject 是一个特别底层的类，只提供了一些最基础功能，然后以它为父类创建子类，例如 RenderBox，它可以更方便的帮助我们在 Flutter 中进行布局和绘制以及 hit testing。

## Lifecycle

&emsp;当不再需要 RenderObject 时，必须进行销毁。对象的创建者负责对其进行销毁。通常情况下，创建者是 RenderObjectElement，当它被卸载（unmout）时，该 Element 会销毁它创建的 RenderObject。（这里则可以在 RenderObjectElement.unmout 中找到对应的代码：当 RenderObjectElement 卸载时，它首先记录下自己的旧 widget，然后调用 Element.unmount 函数进行 Element 被卸载时的基本逻辑：1. 如果有 GlobalKey 的话，从 globalKeyRegistry 中把自己移除，2. 把自己的 widget 置为 null，3. 把自己的 dependencies 置为 null，4. 把自己的生命周期状态置为 defunct。然后回到 RenderObjectElement.mount，拿着前面记录的旧 widget，调用：oldWidget.didUnmountRenderObject(renderObject)，即向外传递 renderObject 被卸载了，这是 RenderObjectWidget 独有的，让上层有必要时也可以进行一些清理逻辑，然后调用 renderObject 属性的 dispose 函数，然后把自己的 renderObject 属性置为 null。）

&emsp;RenderObject 在调用 dispose 方法时负责清理所持有的任何昂贵资源，比如 Picture 或 Image 对象。这还包括 RenderObject 直接创建的任何 Layer。dispose 的基本实现将会将 layer 属性设为 null（在 RenderObject.dispose 函数中可见）。RenderObject 子类还必须将直接创建的任何其他 layer 也设置为 null。（同样遵循谁创建了资源，那么当自己要被销毁时也要记得释放自己的创建的资源，谁创建谁释放，看到这里，我们可以发现：StatefulElement 对象和 State 对象它们是一一对应，一同创建一同销毁，而到了 RenderObjedtElement 这里，同样的：RenderObjectElement 会持有自己的 RenderObject，它们也是一一对应的，一同创建（稍晚一点点，当 RenderObjectElement 挂载到 Element Tree 上后，会立即创建 RenderObject 对象，然后也是立即把整个 RenderObject 对象附加到 Render Tree 上去。）一同销毁，在 RenderObjectElement 的生命周期中 renderObject 属性一直都是同一个 RenderObject 对象，不会发生变化。）
                                     
## Writing a RenderObject subclass

&emsp;在大多数情况下，直接从 RenderObject 进行子类化是过度的，直接从 RenderBox 开始会是一个更好的起点。然而，如果一个 RenderObject 不想使用笛卡尔坐标系，那么它确实应该直接继承自 RenderObject。这允许它通过使用 Constraints 的一个新子类来定义自己的布局协议，而不是使用 BoxConstraints，并且可能使用一个全新的对象和值来表示输出结果，而不仅仅是一个 Size。这种增强的灵活性是以无法依赖 RenderBox 的特性为代价的。例如，RenderBox 实现了一个固有大小的协议，允许你测量子项而不完全布局，这样，如果子项的大小发生变化，父项将重新布局（以考虑子项的新尺寸）。这是一项微妙且容易出错的功能。

&emsp;编写 RenderBox 的大部分内容同样适用于编写 RenderObject，因此建议阅读 RenderBox 中的讨论以进行背景了解。主要的差异在于布局和点击测试，因为这些是 RenderBox 主要专门处理的方面。（如在 RenderObject 中 HitTestTarget.handleEvent 的实现内容是空的！）

### Layout

&emsp;一个布局协议以 Constraints 的子类开始。有关如何编写 Constraints 子类的更多信息，可以翻阅前面的 Constraints 部分的内容。

&emsp;performLayout 方法应该接受 constraints 并应用它们（这里指 layout 函数的 constraints 入参，同时 layout 函数内部也会把 constraints 赋值给 RenderObject 的 constraints 属性，这个 constraints 约束是由父级传递来的，子级必须要遵守的。）。布局算法的输出是设置在 render object 的字段上，用于描述 render object 的几何形状，以便父级布局使用。例如，对于 RenderBox，输出是 RenderBox.size 字段。只有在父级在调用子级的 layout 函数时将 parentUsesSize 参数指定为 true 时，父级才应该读取这个输出（例如：RenderConstrainedBox.performLayout 函数起了很好的示范作用，可以点击进去看看。）。

&emsp;每当 RenderObject 上的任何更改会影响该对象的布局时，都应调用 markNeedsLayout 方法标记此 RenderObject 需要重新布局。

### Hit Testing

&emsp;Hit testing 比布局更加灵活。没有可重写的方法，需要我们自己提供一个。

&emsp;我们的 hit testing 方法的一般行为应该类似于为 RenderBox 描述的行为。主要区别在于输入不一定是 Offset。当向 HitTestResult 添加条目时，你也可以使用不同的 HitTestEntry 子类。当调用 handleEvent 方法时，将传入与添加到 HitTestResult 中的相同对象，因此可以用于跟踪诸如点击的精确坐标等信息，无论新布局协议使用的坐标系是什么。

### Adapting from one protocol to another

&emsp;一般来说，Flutter Render Tree 的根是一个 RenderView 对象。这个对象只有一个子级（child），子级必须是一个 RenderBox。因此，如果你想在 Render Tree 中有一个自定义的 RenderObject 子类，你有两种选择：要么需要替换 RenderView 本身，要么需要一个 RenderBox，这个 RenderBox 以你的类作为它的子级（child）。（后者是更常见的情况。）

&emsp;这个 RenderBox 子类从 Box 协议转换为你的类的协议。

&emsp;具体来说，这意味着对于 hit testing，它重写 RenderBox.hitTest，并调用你的类中用于 hit testing 的任何方法。

&emsp;同样地，它重写 performLayout 来创建适合你的类的 Constraints 对象，并将其传递给子级（child）的 layout 方法。

### Layout interactions between render objects

&emsp;一般来说，RenderObject 的布局应该只取决于其子级（child）布局的输出，而且只有在 layout 调用中将 parentUsesSize 设置为 true 时才会如此。此外，如果设置为 true，则如果要渲染子级（child），则父级必须调用子级（child）的 layout，否则当子级（child）更改其布局输出时，父级将不会收到通知。

&emsp;可以设置 RenderObject 协议来传递额外的信息。例如，在 RenderBox 协议中，你可以查询子级们（children）的固有尺寸（intrinsic dimensions）和基线几何信息（baseline geometry）。但是，如果这样做的话，那么当父级在上一次布局阶段使用了额外信息时，子级必须在任何额外信息更改时调用父级的 markNeedsLayout 方法。关于如何实现这一点的示例，可参考 RenderBox.markNeedsLayout 方法。它重写了 RenderObject.markNeedsLayout 方法，以便当子级的几何信息更改时，如果父级已查询了 固有尺寸（intrinsic dimensions）和基线几何信息（baseline geometry），则会被标记为脏。

&emsp;OK，RenderObject 的文档就这么多，看起来其实还是挺绕的，比较它牵涉的内容真的很多。下面我们开始看它的源码，一层一层的用代码去解释它们的文档。

## Constructors

&emsp;看到 RenderObject 构造函数内部添加了 FlutterMemoryAllocations 的内容记录 RenderObject 对象的创建，以及对应的在 dispose 中记录 RenderObject 对象的销毁。这在之前的 State 和 Element 类中也见过相似的内容记录创建和销毁，当它们如果不是匹配出现的话就说明有内存泄漏了。

&emsp;这里看到 RenderObject 需要实现 HitTestTarget。HitTestTarget 只有一个函数：`void handleEvent(PointerEvent event, HitTestEntry<HitTestTarget> entry)`，而 RenderObject 对其只有一个空实现，交给具体的子类来实现具体内容。

```dart
abstract class RenderObject with DiagnosticableTreeMixin implements HitTestTarget {
  RenderObject() {
    // isRepaintBoundary 和 alwaysNeedsCompositing 默认都是 false，
    // 所以这里 _needsCompositing 默认也是 false。
    // _needsCompositing 表示当前 RenderObject 是否需要进行层合成。
    _needsCompositing = isRepaintBoundary || alwaysNeedsCompositing;
    
    // 默认是 false，表示当前的 RenderObject 对象，之前是否是绘制边界。
    _wasRepaintBoundary = isRepaintBoundary;
  }
```

&emsp;RenderObject 的构造函数中的几个属性都是超级复杂的内容，我们后面再展开。

## reassemble

&emsp;此函数执行后，会以入参 RenderObject 为根的整个 Render 子树被标记为 dirty，以进行布局、绘制等操作，以便可以看到热重载的效果，或者以便应用更改全局调试标志（例如 debugPaintSizeEnabled）的效果。

&emsp;这由 RendererBinding 在响应 ext.flutter.reassemble hook 时调用，该 hook 由开发工具使用，当应用程序代码已更改时，使 Widget Tree 获取任何已更改的实现。

&emsp;非常昂贵的操作，在除开发过程中不应调用。

```dart
  void reassemble() {
    // PipelineOwner 刷新管线的四个阶段，与此对应。
    
    // 如果当前 RenderObject 对象的重新布局边界属性：_relayoutBoundary，
    // 就是自己的话：则把自己添加到 owner._nodesNeedingLayout 列表里面去，
    // 并请求新的帧进行视觉刷新：owner!.requestVisualUpdate()。
    
    // 如果 _relayoutBoundary 不是自己的话，就调用自己 parent 的 markNeedsLayout，
    // 即往上传递需要重新布局的信号。
    markNeedsLayout();
    
    // 根据 parent 的情况，如果 parent 已经需要合成位更新，则直接 return，
    // 如果自己不是重绘边界，并且 parent 也不是重绘边界，则继续往 parent 中传递调用 markNeedsCompositingBitsUpdate 函数，
    // 否则的把自己添加到 owner._nodesNeedingCompositingBitsUpdate 集合中去，
    // 这里没有像 markNeedsLayout 一样进行请求新的帧进行视觉刷新，是因为其它的 flush 函数已经请求过了，自己就不需要了，
    // 因为 markNeedsCompositingBitsUpdate 和其它 flush 函数都是一起做的，它可以直接蹭别人的帧请求。
    markNeedsCompositingBitsUpdate();
    
    // 如果当前 RenderObject 对象就是重绘边界，
    // 则把自己添加到 owner._nodesNeedingPaint 列表里面去，
    // 并请求新的帧进行视觉刷新：owner!.requestVisualUpdate()。
    // 其它情况的话则同样是调用 parent 的 markNeedsPaint，往父级中传递。
    markNeedsPaint();
    
    // 添加到 owner!._nodesNeedingSemantics 中，标记进行语义化更新
    markNeedsSemanticsUpdate();
    
    // 递归在以当前 RenderObject 为根的整个 Render 子树上所有 RenderObject 对象执行 reassemble。
    visitChildren((RenderObject child) {
      child.reassemble();
    });
  }
```

## dispose

&emsp;释放由此 RenderObject 对象持有的任何资源。

&emsp;创建 RenderObject 的对象（RenderObjectElement）负责将其处理。如果这个 RenderObject 直接创建了任何 children，则也必须在此方法中处理这些 children。它不能处理由其他对象（比如 RenderObjectElement）创建的任何 children。当该 RenderObjectElement 卸载时，这些 children 将在那时被处理，这可能会延迟，如果 Element 移动到 Element Tree 的另一个部分。（在本帧结束时才会统一进行。）

&emsp;此方法的实现必须以调用继承方法结束，如 super.dispose()。即如果子类重写此 dispose 方法的话，需要先执行自己的自定义操作，然后最后再调用 super.dispose() 函数。

&emsp;在调用 dispose 后，RenderObject 将不再可用。（此函数类似 iOS 的 dealloc 函数，在对象内存释放前进行打扫清理工作。）

```dart
  @mustCallSuper
  void dispose() {
    _layerHandle.layer = null;
  }
```

&emsp;OK，看了 RenderObject 的两个常规的函数：reassemble 和 dispose。然后接下来是一部分跟 LAYOUT 相关的内容，后续还有 PAINTING 和 SEMANTICS 和 EVENTS HIT TESTING 等部分，我们慢慢展开。

&emsp;下面我们开始展开 RenderObject 中与 LAYOUT 部分相关的内容。

## parentData

&emsp;供父级 RenderObject 使用的数据。

&emsp;Parent data 被用于布局这个对象的渲染对象（通常是 Render Tree 中该对象的父对象）存储与自身相关的信息，以及任何其他节点可能清楚知道这些数据意味着什么的。Parent data 对于子对象来说是不透明的。

+ parentData 属性不能直接设置（parentData 属性不是私有的，在外部也可以设置），除非通过在父级（parent RenderObject）上调用 setupParentData 方法进行设置。
+ Parent data 可以在将子级 RenderObject 添加到父级 RenderObject 之前设置，方法是在未来的父级 RenderObject 上调用 setupParentData。
+ 使用 parent data 的约定取决于父级和子级之间使用的布局协议。例如，在 box 布局中，parent data 是完全不透明的，但在 sector（扇区）布局中，子级被允许读取 parent data 的一些字段。

&emsp;这里有点绕哦，还记得 RenderObjectElement.attachRenderObject 函数吗？当把 RenderObject 对象附加到 Render Tree 以后，会查找当前 RenderObjectElement 到祖先中最近的 RenderObjectElement 之间的 ParentDataElement 节点，把它们收集起来，然后循环对当前 RenderObject 对象执行更新 parentData 操作。即调用 ParentDataWidget 的 `void applyParentData(RenderObject renderObject);` 函数，内容就是更新 RenderObject 对象的 parentData 属性。
 
```dart
  ParentData? parentData;
```

### setupParentData

&emsp;重写此方法可以正确设置子级的 parentData 属性（的类型）。你可以调用此函数来在子级（child RenderObject）附加到父级的子级列表之前设置这个子级的 parentData 属性的默认值或者说是设置默认类型，例如在 RenderBox 中设置的是：`child.parentData = BoxParentData();`，它是 BoxParentData 类型的。而这里是在 RenderObject 基类中，parentData 默认是 ParentData 类型的。ParentData 类是所有 parent data 的基类。（如 BoxParentData 中有一个 Offset offset 属性，被用在子级在父级中的布局偏移值。如 Positioned 中使用的 StackParentData，它则有 top/right/bottom/left/width/height 属性来具体描述子级 RenderObject 在父级 RenderObject 中的位置。）

&emsp;当子级 RenderObject 将要附加到父级 RenderObject 时给这个子级 RenderObject 的 parentData 属性设置默认值。ParentData 是所有 parent data 的基类。

```dart
  void setupParentData(covariant RenderObject child) {
    if (child.parentData is! ParentData) {
      child.parentData = ParentData();
    }
  }
```

&emsp;牵涉到 Parent data 的内容还是较复杂的，后面我们学习 ContainerRenderObjectMixin 时再详细看，它的内部有较多的 parent data 的使用。

## depth

&emsp;在 Render Tree 中，此 RenderObject 对象的深度。（同 Element 对象的 depth 属性。）

&emsp;Render Tree 中 RenderObject 节点的深度随着向下遍历而单调递增：一个节点的深度总是大于其祖先的深度。对于兄弟节点之间的深度没有保证。

&emsp;子节点的深度可以比父节点的深度高出不止一个，因为深度值永远不会减少：重要的是它比父节点的深度更大。考虑一个树，有根节点 A，子节点 B，和孙子节点 C。最初，A 的深度为 0，B 的深度为 1，C 的深度为 2。如果 C 被移到成为 A 的子节点，B 的兄弟，则数字不会改变。C 的深度仍然是 2。

&emsp;节点的深度用于确保节点按深度顺序处理。深度是通过 adoptChild 和 dropChild 方法自动维护的。虽然 Element 和 RenderObject 对象都有 depth 属性，表示当前节点在树中的深度，不同于 Element 的 depth 会在 Element 节点挂载（Element.mount）到父级时自动 +1，RenderObject 则是在子级 RenderObject 附加到父级时，通过下面的 redepthChildren 方法更新这个子级 RenderObject 的 depth 属性。 

```dart
  int get depth => _depth;
  int _depth = 0;
```

## redepthChild

&emsp;将给定的子级 RenderObject 的深度调整为高于当前 RenderObject 自身的深度。只能在 redepthChildren 的重写中调用此方法。

```dart
  @protected
  void redepthChild(RenderObject child) {
    if (child._depth <= _depth) {
      child._depth = _depth + 1;
      
      child.redepthChildren();
    }
  }
```

## redepthChildren

&emsp;调整此 RenderObject 的子级 RenderObject 的深度（depth 属性的值），如果有的话。在具有子级的 RenderObject 子类中重写此方法，分别为每个子级调用 redepthChild。不要直接调用此方法。

```dart
  @protected
  void redepthChildren() { }
```

## parent

&emsp;在 Render Tree 中，此 RenderObject 的直接父级。Render Tree 中根节点的 parent 指向是 null。

&emsp;前向指针。

```dart
  RenderObject? get parent => _parent;
  RenderObject? _parent;
```

## adoptChild

&emsp;当一个 RenderObject （子类）实例对象决定将一个 RenderObject 对象作为自己的子级时调用。仅供 RenderObject 子类对象在更改其子级列表（也包括单个 子级的情况）时使用（注意这是修改 RenderObject 对象的子级时使用的函数，例如 SingleChildRenderObjectElement 的 renderObject 类型是：RenderObjectWithChildMixin，它只有一个子级：`ChildType? _child` 子级指针，在它的 Setter 函数中就用到这个 adoptChild 函数，而我们的 MultiChildRenderObjectElement 的 renderObject 类型是：ContainerRenderObjectMixin，它呢有子级列表：`ChildType? _firstChild;` 和 `ChildType? _lastChild;` 组成的链表，当向它的子级列表中加入新的子级时会直接调用这个 adoptChild 函数）。在其他情况下调用此方法将导致不一致的树形结构，并可能导致崩溃。

&emsp;这里我们根据一个 MultiChildRenderObjectElement 节点且其父级也是 MultiChildRenderObjectElement 为例，当把此 RenderObjectElement 节点被挂载到 Element Tree 上时，这个 adoptChild 函数被调用的时机：

1. 当 MultiChildRenderObjectElement 对象要挂载到 Element tree 上时，会调用 MultiChildRenderObjectElement.mount 函数。
2. MultiChildRenderObjectElement.mount 首先调用 RenderObjectElement.mount 函数。
3. RenderObjectElement.mount 首先调用 Element.mount 函数，把 MultiChildRenderObjectElement 对象挂载到 Element Tree 上。
4. Element.mount 执行完，回到 RenderObjectElement.mount 函数后，会创建 RenderObject 对象：`_renderObject = (widget as RenderObjectWidget).createRenderObject(this)`。
5. 此时创建的 RenderObject 类型便是一个：ContainerRenderObjectMixin 类型的 RenderObject 对象。
6. ContainerRenderObjectMixin 创建完毕会调用 RenderObjectElement.attachRenderObject 把此 RenderObject 附加到 Render Tree。
7. 在 RenderObjectElement.attachRenderObject 函数内部首先会往上找祖先节点中距离自己最近的类型是 RenderObjectElement 的节点，假设找到的也是一个 MultiChildRenderObjectElement 节点。
8. 找到后呢，就会调用它的 insertRenderObject 函数，此时就到了 MultiChildRenderObjectElement 的 insertRenderObject 函数。
9. MultiChildRenderObjectElement.insertRenderObject 函数就会调用 ContainerRenderObjectMixin.insert 函数。
10. 而 ContainerRenderObjectMixin.insert 函数内部便是调用：RenderObject.adoptChild 函数，即把这个新建的 RenderObject 附加到现有的 Render Tree 上。

&emsp;还有一点要注意一下，这个 adoptChild 函数是父级 RenderObject 调用的，并不是当前新建的 RenderObject 对象，它会作为 child 参数。例如当前一个 RenderObjectElement 节点挂载到 Element Tree 上，然后创建了对应的 RenderObject 对象，然后需要把此新建的 RenderObject 对象附加到 Render Tree 上去，此时就会查找此 RenderObjectElement 节点祖先中最近的 RenderObjectElement 节点，然后则是拿到此祖先 RenderObjectElement 节点的 RenderObject 对象，然后以前面新建的 RenderObject 对象为参数调用 adoptChild 函数，把这个新建的 RenderObject 对象作为自己的子级。

```dart
  @mustCallSuper
  @protected
  void adoptChild(RenderObject child) {
    // 给 RenderObject child 的 parentData 属性，赋一个默认值（或是一个默认类型）
    setupParentData(child);
    
    // ⬇️👇 然后是下面的三连击标记：
    
    // 当前 RenderObject 对象或其父级标记为需要重新布局。
    // 当前 RenderObject 对象或其父级标记为需要合成位更新。
    // 当前 RenderObject 对象或其父级标记为需要语义化更新。
    markNeedsLayout();
    markNeedsCompositingBitsUpdate();
    markNeedsSemanticsUpdate();
    
    // 把入参 child 的 parent 指针指向当前 RenderObject 对象。
    child._parent = this;
    
    // 如果当前的 RenderObject 对象的 _owner 属性不为 null，
    // 则把入参 child 的 _owner 也指向和父级 RenderObject 同样的 _owner 对象。
    // 这里 child.attach 内部，除了给 child._owner 赋值外，也会发起一轮类似上面的👆标记事件，
    // 会进行四组的判断是否把 child 标记：重新布局/合成位更新/重新绘制/语义化更新。
    //（上面是对 child 的直接父级进行的标记，这里则是对 child 的标记。合理！）
    if (attached) {
      child.attach(_owner!);
    }
    
    // 更新入参 child 的 depth 值
    redepthChild(child);
  }
```

## dropChild

&emsp;当一个 RenderObject （子类）实例对象决定另一个 RenderObject 对象不再是自己的子级时调用。仅供 RenderObject 子类对象在更改其子级列表时使用。在其他情况下调用此方法将导致不一致的树形结构，并可能导致崩溃。

&emsp;同上面的 adoptChild 函数，dropChild 是分离 RenderObject 对象的现有子级 RenderObject 对象时调用。

```dart
  @mustCallSuper
  @protected
  void dropChild(RenderObject child) {
    // 递归把 child 自己和所有 child 子级的 _relayoutBoundary 标识置为 null 
    child._cleanRelayoutBoundary();
    
    // 回调 child 的 parentData 的 detach 函数，
    // 并且把 parentData 置为 null。
    child.parentData!.detach();
    child.parentData = null;
    
    // 把 child 的 parent 指向置为 null
    child._parent = null;
    
    // 把 child 的 owner 指向置为 null
    if (attached) {
      child.detach();
    }
    
    // ⬇️👇 然后是下面的三连击标记：
    // 即不管是向当前 RenderObject 对象添加子级还是移除子级，都需要三连击标记。
    
    // 当前 RenderObject 对象或其父级标记为需要重新布局。
    // 当前 RenderObject 对象或其父级标记为需要合成位更新。
    // 当前 RenderObject 对象或其父级标记为需要语义化更新。
    markNeedsLayout();
    markNeedsCompositingBitsUpdate();
    markNeedsSemanticsUpdate();
  }
```

## visitChildren

&emsp;在 RenderObject 子类中重写此方法。对该 RenderObject 的每个直接子级调用入参 visitor。

```dart
  void visitChildren(RenderObjectVisitor visitor) { }
```

## owner

&emsp;这个 RenderObject 对象的 PipelineOwner 属性（如果未附加则为 null）。包含这个 RenderObject 对象的整个 Render Tree 上所有的节点将拥有相同的 owner。前面已学习 PipelineOwner，可以翻回去看看。

```dart
  PipelineOwner? get owner => _owner;
  PipelineOwner? _owner;
```

## attached

&emsp;此 RenderObject 对象所属的 Rener Tree 是否已附加到 PipelineOwner 中。在调用 attach 函数后（内部会对 owner 属性赋值），该值变为 true。在调用 detach 函数后（内部会把 owner 属性置为 null），该值变为 false。

```dart
  bool get attached => _owner != null;
```

## attach

&emsp;将该 RenderObject 对象标记为已附加到指定的 owner（即为当前的 RenderObject 对象的 owner 属性赋值。）。

&emsp;通常只从父级的 attach 方法中调用，并且由 owner 调用以将 Render Tree 的根标记为已附加。（例如在 adoptChild 函数中，当把子级 RenderObject 附加到父级时，也会调用 child.attach 函数，把当前的父级 RenderObject 的 owner 属性传入进去，子级和父级的 owner 属性都是同一个 PiplineOwner 对象。）

&emsp;具有子级的 RenderObject 子类应该重写此方法，在调用继承的方法后，如 super.attach(owner)，将所有子级附加到相同的 owner。（例如：RenderObjectWithChildMixin 和 ContainerRenderObjectMixin 都重写了此 attach 函数，把自己的所有 child 的 owner 属性也赋值为入参 owner。）

```dart
  @mustCallSuper
  void attach(PipelineOwner owner) {
    _owner = owner;
    
    // 如果节点在未附加时出现了脏数据，请确保在有 owner 可用时将其添加到相应的脏数据列表中。
    // 如果需要重新布局并且 _relayoutBoundary 重新布局边界不为 null。
    if (_needsLayout && _relayoutBoundary != null) {
      
      // 如果我们从未进行过任何布局，则不会进入此代码块；
      // scheduleInitialLayout() 将会处理这种情况。
      
      _needsLayout = false;
      markNeedsLayout();
    }
    
    // 是否需要合成位更新
    if (_needsCompositingBitsUpdate) {
      _needsCompositingBitsUpdate = false;
      
      markNeedsCompositingBitsUpdate();
    }
    
    // 如果需要重新绘制并且 _layerHandle.layer 不为 null
    if (_needsPaint && _layerHandle.layer != null) {
      
      // 如果我们从未绘制过任何内容，则不会进入此代码块；
      // scheduleInitialPaint() 将会处理这种情况。
      
      _needsPaint = false;
      markNeedsPaint();
    }
    
    // 如果需要语义化更新并且是语义化边界
    if (_needsSemanticsUpdate && _semanticsConfiguration.isSemanticBoundary) {
    
      // 如果我们从未更新过语义，则不会进入此代码块；
      // scheduleInitialSemantics() 将会处理这种情况。
      
      _needsSemanticsUpdate = false;
      markNeedsSemanticsUpdate();
    }
  }
```

## detach

&emsp;逻辑基本同上面的 attach 函数。

&emsp;将该 RenderObject 对象标记为与其 owner 属性分离。

&emsp;具有子级的子类应该重写此方法，在调用继承的方法后 detach 所有子级，如 super.detach()。

```dart
  @mustCallSuper
  void detach() {
    _owner = null;
  }
```

## `_needsLayout`

&emsp;标记是否需要重新布局。

```dart
  bool _needsLayout = true;
```

## `_relayoutBoundary`

&emsp;重新布局的边界，是一个可空的 RenderObject 指针。

&emsp;如果已知，表示包围此 RenderObject 对象的最近的布局边界。

&emsp;当一个 RenderObject 对象被标记为需要布局时，其父级可能也需要被标记为需要布局。一个 RenderObject 对象在重新布局时不需要父级重新布局的情况（因为其 Size 在重新布局时不会改变（传递来的约束没有变），或者因为其父级不会使用子级的大小进行自身的布局）被称为 "relayout boundary 重新布局边界"。

&emsp;这个属性在 layout 函数中设置，并由 markNeedsLayout 函数查询，以决定是否递归地标记父级也需要布局。

&emsp;该属性最初为 null，并且如果该 RenderObject 对象从 Render Tree 中移除（使用 dropChild 函数），它会再次变为 null；在此 RenderObject 最近被添加到 Render Tree 中后的第一次布局之前，它将保持为 null。在树中的某个祖先当前正在进行布局时，此属性也可以为 null，直到该 RenderObject 对象本身进行布局时。

&emsp;当 `_relayoutBoundary` 不为 null 时，重新布局边界要么是这个 RenderObject 对象自身，要么是其祖先之一，并且沿着该祖先的所有 RenderObject 对象在祖先链中具有相同的 `_relayoutBoundary`。等效地说：当 `_relayoutBoundary` 不为 null 时，重新布局边界要么是这个 RenderObject 对象自身，要么与其父级的相同。（因此 `_relayoutBoundary` 可以是 `null`、`this`，或者 `parent!._relayoutBoundary!`）

```dart
  RenderObject? _relayoutBoundary;
```

## constraints

&emsp;由最近父级 RenderObject 提供的布局约束，在父级中调用 child.layout 函数时会传递过来。

&emsp;如果尚未进行布局，访问此 getter 将引发 StateError 异常。

```dart
  @protected
  Constraints get constraints {
    if (_constraints == null) {
      throw StateError('A RenderObject does not have any constraints before it has been laid out.');
    }
    
    return _constraints!;
  }
  
  Constraints? _constraints;
```

## markNeedsLayout

&emsp;将该 RenderObject 对象的布局信息标记为脏，并根据此对象是否为重新布局边界（relayoutBoundary）来将其添加到其 PipelineOwner owner 的 `_nodesNeedingLayout` 列表中，或者推迟到其父级。

```dart
  void markNeedsLayout() {
    // 如果已经被标记为需要 Layout 了，直接 return 即可。
    if (_needsLayout) {
      return;
    }
    
    // 如果 _relayoutBoundary 为 null，则继续往上标记父级重新布局。
    // 即如果自己不是重新布局的边界，则继续往上找，直到向上找到最近的重新布局的边界。
    
    if (_relayoutBoundary == null) {
      _needsLayout = true;
      
      if (parent != null) {
        // _relayoutBoundary 在 RenderObject.layout 中被其祖先清除。
        // 保守地标记所有内容为脏，直到其达到最近的已知重新布局的边界。
        
        markParentNeedsLayout();
      }
      
      return;
    }
    
    // 如果重新布局边界不等于等于当前的 RenderObject 对象，则也往父级中去找。
    if (_relayoutBoundary != this) {
    
      // 从父级中去找重新布局边界，即调用父级的 markNeedsLayout 函数。
      markParentNeedsLayout();
      
    } else {
      // 如果走到这里的话，即自己就是自己的重新布局边界。
    
      _needsLayout = true;
      
      // 如果重新布局的边界就是自己，则把自己添加到 owner 需要重新布局的列表中去。
      // 并发出请求进行 "视觉更新" 的请求，以便在下一帧统一进行批量的处理重新布局。
      
      if (owner != null) {
        // 添加到 owner 的 _nodesNeedingLayout 列表中去。
        owner!._nodesNeedingLayout.add(this);
        
        // 请求进行视觉更新。
        owner!.requestVisualUpdate();
      }
    }
  }
```

### Background

&emsp;与在对 render object 进行写操作后急切地更新布局信息不同，我们会将布局信息标记为未更新，进而安排进行视觉更新。作为视觉更新的一部分，rendering pipeline 会更新 render object 的布局信息。

&emsp;该机制会批处理布局工作，使多个连续的写入合并在一起，减少了冗余计算。

&emsp;如果 render object 的父级指示在计算布局信息时使用其中一个 render object 子级的大小，则在为子级调用此函数时，还会标记父级也需要布局。在这种情况下，由于需要重新计算父级和子级的布局，因此只会通知流水线所有者（pipeline owner）有关父级；当父级重新布局时，它将调用子级的布局方法，因此也会对子级进行重新布局。有两种情况：

1. 父级和子级的布局信息各自独立，互不影响。
2. 父级需要子级的布局信息，那么当子级需要重新布局时，它会往上找，让父级进行重新布局，然后父级开始布局后，会连带着可子级也重新进行布局。

&emsp;RenderObject 的重新布局过程和 Element 的重建过程是一样的，它们都不是只对当前这一个 RenderObject/Element 对象操作，它们都是递归进行，会对整个以当前 RenderObject/Element 对象为根节点的整个子树🌲进行重新布局/重建。

### Special cases

&emsp;有些 RenderObject 的子类，特别是 RenderBox，有其他情况下需要通知父级子级的状态已经变脏（例如，子级的 固有尺寸（intrinsic dimensions）和基线几何信息（baseline geometry）发生变化的情况）。这些子类会重写 markNeedsLayout 方法，通常在正常情况下会调用 super.markNeedsLayout()，或者在需要对父级和子级都进行布局的情况下调用 markParentNeedsLayout 方法。

&emsp;如果 sizedByParent 已经改变，会调用 markNeedsLayoutForSizedByParentChange 方法，而不是调用 markNeedsLayout 方法。（在 markNeedsLayoutForSizedByParentChange 方法中也会直接标记父级需要更新布局，因为 sizedByParent 改变了，意味着之前的子级和父级的布局依赖关系要被改变了，所以需要子级和父级都根据现有的情况进行重新布局。）

## markParentNeedsLayout

&emsp;将当前 RenderObject 的布局信息标记为脏，并延迟到父级 RenderObject 处理。（父级重新布局会连带所有的子级也重新布局，当然也会有节流，比如两次调用传来的 Constraints 是一样的，则此子级不需要重新布局。）

&emsp;此函数只应该从 markNeedsLayout 或者 markNeedsLayoutForSizedByParentChange 的子类实现中调用，这些子类引入了更多原因以延迟处理脏布局到父级。（当自己不是重新布局边界的话，意味着父级和子级的之间有布局依赖，例如父级需要子级的 size、或者子级的 sizeByParent 属性发生了变化，此时都需要父级也进行重新布局。）

&emsp;只有在父级 RenderObject 不为 null 时才调用此函数。

```dart
  @protected
  void markParentNeedsLayout() {
    _needsLayout = true;
    
    final RenderObject parent = this.parent!;
    parent.markNeedsLayout();
  }
```

## markNeedsLayoutForSizedByParentChange

&emsp;将此 RenderObject 的布局信息标记为脏（类似于调用 markNeedsLayout），并且另外还处理任何必要的工作来处理 sizedByParent 已更改值的情况。

&emsp;每当 sizedByParent 可能已更改时应调用此方法。

&emsp;只有在 parent 不为 null 时才调用此方法。

```dart
  void markNeedsLayoutForSizedByParentChange() {
    markNeedsLayout();
    markParentNeedsLayout();
  }
```

## `_cleanChildRelayoutBoundary`

&emsp;在入参 RenderObject child 的子树中将 `_relayoutBoundary` 设置为 null，直到遇到重新布局边界。这是一个静态方法，用于减少在访问子级时的闭包分配（闭包创建）。

&emsp;这是一个在 RenderObject 对象的子级中递归调用的函数，递归退出的条件是遇到一个子级它的 `_relayoutBoundary` 属性不是自己。并不是在所有的子级中递归调用。

&emsp;即清理 child 的子级中 `_relayoutBoundary` 属性不是自己的 RenderObject 的 `_relayoutBoundary` 属性为 null。但是只要遇到 `_relayoutBoundary` 属性是自己的 RenderObject 就会立即停止清理。

```dart
  // ⚠️ 注意这是一个静态函数。
  static void _cleanChildRelayoutBoundary(RenderObject child) {
    // 如果入参 child 的 _relayoutBoundary 属性不是自己的话，才进入 if，否则什么也不做。
    if (child._relayoutBoundary != child) {
      
      // 在 child 的子级中递归调用 _cleanChildRelayoutBoundary 函数
      child.visitChildren(_cleanChildRelayoutBoundary);
      
      // 把它的 _relayoutBoundary 属性置空。
      child._relayoutBoundary = null;
    }
  }
```

## `_propagateRelayoutBoundaryToChild`

&emsp;这是一个静态方法，通过 visitChildren 减少闭包分配（闭包创建）。

&emsp;往入参 child 的子级中传播自己的 `_relayoutBoundary` 属性递归进行。注意这里类似上面的清理，一旦遇到子级的 `_relayoutBoundary` 属性是自己的 RenderObject 对象就会立即停止递归。

```dart
  static void _propagateRelayoutBoundaryToChild(RenderObject child) {
    // 如果入参 child 的 _relayoutBoundary 就是自己的话，直接 return，停止递归。
    if (child._relayoutBoundary == child) {
      return;
    }
    
    // 取得 child 的父级的 _relayoutBoundary 属性
    final RenderObject? parentRelayoutBoundary = child.parent?._relayoutBoundary;
    
    // 往 child 的子级传递自己的 _relayoutBoundary，注意这同样是只要遇到子级的 _relayoutBoundary 是自己时，立刻停止！🤚
    child._setRelayoutBoundary(parentRelayoutBoundary!);
  }
```

## `_setRelayoutBoundary`

&emsp;将此 RenderObject 对象的子级的 `_relayoutBoundary` 设置为入参 value，包括此 RenderObject 对象，但在此之后停止在重新布局边界处。（即遇到子级的 `_relayoutBoundary` 是自己时立刻停止。）

```dart
  void _setRelayoutBoundary(RenderObject value) {
    // 设置 _relayoutBoundary 属性值为入参 value
    _relayoutBoundary = value;
    
    // 然后在当前 RenderObject 对象的子级中递归调用 _propagateRelayoutBoundaryToChild 函数
    visitChildren(_propagateRelayoutBoundaryToChild);
  }
```

## scheduleInitialLayout

&emsp;通过调度首次布局来引导渲染管线（rendering pipeline）。需要将要渲染的对象附加到根 Rener Tree 上才能实现。

&emsp;可以查看 RenderView 来了解如何使用该函数的示例。在 Render Tree 根节点对应的 `_RawViewElement` Element 挂载到 Element Tree 上时，即在 `_RawViewElement.mount` 函数内，通过 `renderObject.prepareInitialFrame()` 直接调用 `scheduleInitialLayout();`。 即在 Render Tree 根节点附加到 Render Tree 后进行布局，为首帧内容做准备。

```dart
  void scheduleInitialLayout() {
    // 把自己的 _relayoutBoundary 重新布局边界设置为自己。
    _relayoutBoundary = this;
    
    // 把自己添加到 owner 的 _nodesNeedingLayout 列表中去。
    owner!._nodesNeedingLayout.add(this);
  }
```

## `_layoutWithoutResize`

&emsp;`_layoutWithoutResize`：正如其名，不调整大小的布局，能调用此函数的 RenderObject 对象都是自己是自己的 `_relayoutBoundary`。是的，没错，毕竟每个能加入到 PipelineOwner 的 `_nodesNeedingLayout` 列表中的 RenderObject 对象都是自己就是自己的重新布局边界。

&emsp;直接执行重新布局，并且标记需要语义化更新和需要重新绘制（在下一帧进行，当前帧进行重新布局）。

&emsp;`_layoutWithoutResize` 作为一个私有函数，全局唯一的被调用位置就是在 PipelineOwner.flushLayout 函数内。即当新帧到来，PipelineOwner 会刷新所有在上一帧收集的脏的需要重新布局的 RenderOjbect，然后就会遍历自己的 `_nodesNeedingLayout` 列表中的 RenderObject 对象调用它们的 `_layoutWithoutResize` 函数，执行 RenderObject 的布局函数：performLayout。

```dart
  @pragma('vm:notify-debugger-on-exception')
  void _layoutWithoutResize() {
    try {
      // 直接执行重新布局
      performLayout();
      
      // 标记需要语义化更新
      markNeedsSemanticsUpdate();
      
    } catch (e, stack) {
      _reportException('performLayout', e, stack);
    }
    
    // 设置不需要更新布局信息
    _needsLayout = false;
    
    // 然后标记需要进行重新绘制
    markNeedsPaint();
  }
```

## layout

&emsp;计算此 RenderObject 对象的布局信息。

&emsp;这个方法是父级 RenderObject 请求其子级（child）更新布局信息的主要入口点。父级传递一个约束对象 constraints，告知子级哪些布局方式是可接受的。子级必须遵守给定的约束。

&emsp;注意此函数是在父级 RenderObject 中调用的，例如：在 ConstrainedBox Widget 中，它创建的 RenderObject 是 RenderConstrainedBox 类型。在 RenderConstrainedBox.performLayout 函数内，它内部会调用：`child!.layout(_additionalConstraints.enforce(constraints), parentUsesSize: true);`，即在父级 RenderObject 的 performLayout 函数中调用 child 的 layout 函数。并且在所有的 RenderObject 的 performLayout 函数中都是这样处理的。

&emsp;所以当 RenderObject 对象进行布局时会调用自己的 performLayout 函数，然后 performLayout 内部则是这个父级 RenderObject 调用其子级的 layout 并传入约束参数 constraints 和自己是否要使用子级的 size 参数 parentUsesSize，而在子级的 layout 函数内，又会调用自己的 performLayout 函数，实际最后就是：layout 会一级一级的向自己的子级中递归调用（一级一级的向子级中传递约束 constraints），然后在子级的 layout 函数调用完成后，父级自己决定自己是否要用子级的 size，而这就是我们经常在其它文章中见到的：父级向子级中传递约束，子级向父级中传递 Size。

&emsp;如果父级在子级的布局期间读取信息，父级必须为 parentUsesSize 参数传递 true。在这种情况下，每当子级被标记为需要布局时，父级也会被标记为需要布局，因为父级的布局信息取决于子级的布局信息。如果父级使用默认值（false）作为 parentUsesSize 参数的值，则子级可以更改其布局信息（在给定约束条件 constraints 下）而不通知父级。（代码部分则是用 `_relayoutBoundary` 标识来完成这个逻辑，正如前面的 markNeedsLayout 函数内，会通过 `_relayoutBoundary` 的值进行判断，是否向上传递 parent 需要进行重新布局，当需要时会调用 markParentNeedsLayout 函数进行。）

&emsp;RenderObject 子类不应直接重写 layout 方法。相反，它们应该重写 performResize 和 performLayout。layout 方法将实际工作委托给 performResize 和 performLayout。

&emsp;父级的 performLayout 方法应无条件调用其所有子级的 layout。如果子级不需要执行任何工作来更新其布局信息，那么 layout 方法的责任（在这里实现）是提前返回。

```dart
  @pragma('vm:notify-debugger-on-exception')
  void layout(Constraints constraints, { bool parentUsesSize = false }) {
   
    // 最最重要的标识，判断当前 RenderObject 对象是否可以做自己的重新布局的边界，
    // 注意下面这些都是用 || 连接的，即只要有一个条件为真，那么就会返回真。
    
    // 1. parentUsesSize 为 false，即父级不使用子级的 size。
    // 2. sizedByParent 为 true，即子级使用父级传递来的约束 constraints 计算自己的 size。
    // 3. constraints 参数的最大最小宽相等且最大最小高也相等，即约束 size 大小固定时。
    // 4. parent 不是 RenderObject 时。（Render Tree 根节点）
    
    final bool isRelayoutBoundary = !parentUsesSize || sizedByParent || constraints.isTight || parent is! RenderObject;
    
    // 根据上面的 4 个条件确定当前 RenderObject 的重新布局边界是谁！ 
    
    // 如果重新布局边界是自己的话，RenderObject 的 _relayoutBoundary 属性就是自己，
    // 如果不是的话，_relayoutBoundary 就用父级的 _relayoutBoundary。
    final RenderObject relayoutBoundary = isRelayoutBoundary ? this : parent!._relayoutBoundary!;

    // 如果重新布局标识为 false 并且约束没变的话，就不需要重新布局，直接返回即可。
    if (!_needsLayout && constraints == _constraints) {
      
      // 如果之前的重新布局的边界 _relayoutBoundary 属性和当前计算出来的 relayoutBoundary 不同的话，
      // 则更新 _relayoutBoundary 属性的值。
      if (relayoutBoundary != _relayoutBoundary) {
      
        // ⚠️ 更新 _relayoutBoundary 属性为此 relayoutBoundary，
        // 并往子级中 _relayoutBoundary 非自己之前的子级中传递此 RenderObject 对象当前的 _relayoutBoundary 属性的值。
        _setRelayoutBoundary(relayoutBoundary);
      }
      
      // 重新布局标识为 false 并且新旧 constraints 相同，则直接 return 即可。
      return;
    }
    
    // 其它则是需要更新布局的情况！
    
    // 更新当前的 _constraints 属性的值，记录下当前由父级传递来的约束。
    _constraints = constraints;
    
    // 如果之前的 _relayoutBoundary 属性不为 null，并且当前计算出来的重新布局边界和之前的不相等，
    // 则清理自当前 RenderObject 对象起子级中的 _relayoutBoundary 不是自己的子级的 _relayoutBoundary 属性为 null。
    if (_relayoutBoundary != null && relayoutBoundary != _relayoutBoundary) {
      // 如果能进到这里，说明新的 relayoutBoundary 已经来了，旧的 _relayoutBoundary 要被抛弃了，
      // 然后则把自当前 RenderObject 对象起，
      // 依次向下子级中的 _relayoutBoundary 不是自己的子级的 _relayoutBoundary 属性置为 null。
      
      // 局部的 "重新布局边界" 已经改变，必须通知子级，以防它们也需要更新。
      // 否则，它们将会对后续的实际重新布局边界感到困惑。
      visitChildren(_cleanChildRelayoutBoundary);
    }
    
    // 更新当前重新布局边界 _relayoutBoundary 属性的值。
    _relayoutBoundary = relayoutBoundary;
    
    // 如果当前 RenderObject 的 size 是由父级决定的，则执行 size 更新。
    if (sizedByParent) {
      try {
      
        // RenderObject 对此函数为空实现，需要子类重写。
        // 子类的 performResize 实现内容则是根据其父级传递来的约束 _constraints 计算自己的大小，
        // 并保存在自己的 size 属性中。
        performResize();
        
      } catch (e, stack) {
        _reportException('performResize', e, stack);
      }
    }
    
    try {
      // 执行布局
      performLayout();
      
      // 标记需要语义化更新。
      markNeedsSemanticsUpdate();
    } catch (e, stack) {
      _reportException('performLayout', e, stack);
    }
    
    // 标记为不再需要重新布局
    _needsLayout = false;
    
    // 标记需要重绘。
    markNeedsPaint();
  }
```

## sizedByParent

&emsp;constraints 是否是 size 算法的唯一输入（特别是，子节点没有影响）。

&emsp;始终返回 false 是正确的，但如果要计算此 RenderObject 对象的 size，则返回 true 可能更有效率，因为如果约束不改变，我们就无需重新计算 size。

&emsp;通常，子类会始终返回相同的值。如果值可以改变，那么当它发生改变时，子类应确保调用 markNeedsLayoutForSizedByParentChange。

&emsp;在执行布局中，返回 true 的子类不应该改变此 RenderObject 对象的 size。取而代之的是，这项工作应该由 performResize 完成，或者对于 RenderBox 的子类来说，则应该在 RenderBox.computeDryLayout 中完成。

```dart
  @protected
  bool get sizedByParent => false;
```

## performResize 

&emsp;仅使用约束（`_constraints`）更新 RenderObject 对象的 size 属性。

&emsp;请不要直接调用此函数，应调用 layout 函数，此函数被 layout 函数调用。当在布局期间（layout 函数被调用时）确实需要由此 RenderObject 完成工作时，layout 函数才会调用此函数。父级提供的布局约束可以通过 constraints getter 获得。

&emsp;仅当 sizedByParent 为 true 时才会调用此函数。

&emsp;设置 sizedByParent 为 true 的子类应该重写此方法来计算它们的大小。RenderBox 的子类应该考虑重写 RenderBox.computeDryLayout 方法。

```dart
  @protected
  void performResize();
```

## performLayout

&emsp;执行此 RenderObject 的布局计算工作。

&emsp;请勿直接调用此函数：请调用 layout。当此 RenderObject 在布局期间需要执行实际工作时，会由 layout 调用此函数。父级提供的布局约束可以通过 constraints getter 获得。

&emsp;如果 sizedByParent 为 true，则此函数不应实际更改此 RenderObject 对象的 size。相反，该工作应由 performResize 完成。如果 sizedByParent 为 false，则此函数应同时更改此 RenderObject 的 size 并指示其子级进行布局。

&emsp;在实现此函数时，你必须对每个子级调用 layout，如果你的布局信息依赖于子级的布局信息，则设置 parentUsesSize 为 true。将 parentUsesSize 设置为 true 可确保如果子级进行重新布局，则此 RenderObject 也将执行重新布局。否则，子级可以更改其布局信息而不通知此 RenderObject。（通过 RenderObject 的 `_relayoutBoundary` 属性完成的这个逻辑，`_relayoutBoundary` 可以有三种情况：1. 是当前 RenderObject 对象自己。2. 为 null。3. 是父级的 relayoutBoundary 属性。）

```dart
  @protected
  void performLayout();
```

## invokeLayoutCallback

&emsp;允许对此对象的子级列表（和任何后代）以及由与此对象拥有相同 PipelineOwner 的 Render Tree 中的任何其他脏节点进行更改。回调参数会同步调用，并且只允许在回调的执行期间进行突变。

&emsp;这个功能允许在布局期间按需构建子级列表（例如，基于对象的 size），并在发生这种情况时移动节点以便能够处理 GlobalKey 的重新父级。同时仍然确保每个特定节点每帧只布局一次。

&emsp;调用此函数会禁用一些旨在捕获可能错误的断言。因此，通常不建议使用此函数。

&emsp;此函数只能在布局期间调用。

```dart
  @protected
  void invokeLayoutCallback<T extends Constraints>(LayoutCallback<T> callback) {
    _doingThisLayoutWithCallback = true;
    
    try {
      owner!._enableMutationsToDirtySubtrees(() { callback(constraints as T); });
    } finally {
      _doingThisLayoutWithCallback = false;
    }
  }
```

## RenderObject LAYOUT 总结

&emsp;至此 RenderObject 类中 Layout 部分的全部内容就看完了，内容还是特别多的，特别是其中的重新布局边界理解起来还是有点难的。

&emsp;那么到底什么是重新布局边界呢？其实也很简单，即如果一个 RenderObject 对象需要重新布局时，是否要连带自己的父级 RenderObject 对象也一起进行布局更新，如果不需要的话，那么此 RenderObject 对象自己就是重新布局边界，自己可以随便进行重新布局，而不会影响或者通知父级也进行重新布局。那如果要连带父级 RenderObject 也进行重新布局的话，则会往上找到一个最近的重新布局边界，然后对此边界以下的 Render 子树中的所有 RenderObject 进行重新布局，当然如果父级传递来的约束没有变得话子级也会提前结束重新布局的，并不是遍历到叶子节点为止。

&emsp;那么 RenderObject 对象如何判断自己是否是重新布局边界呢？如下标准，只要其中一个为 true 即可：

```dart
final bool isRelayoutBoundary = !parentUsesSize || sizedByParent || constraints.isTight || parent is! RenderObject;
```

1. 父级调用 layout 函数时传递的 parentUseSize 参数为 false，即父级不需要子级的 size。
2. 当前子级的 sizedByParent 为 true，即此子级的 size 由父级调用 layout 函数时传递来的 constraints 约束决定。
3. 父级调用 layout 函数时传递来的 constraints 约束是一个 Tight 约束，如果是 BoxConstraints 约束的话即最大最小宽度、最大最小高度固定，即父级直接指定了子级的 size。
4. 当前子级的 parent 不是 RenderObject。（大概只有 Render Tree 的根节点吗？）

&emsp;所以整体看下来，我们可以总结到：只要子级 RenderObject 的 size 改变不会影响父级 RenderObject 的话，那么这个子级 RenderObject 对象就可以做自己的重新布局边界。

&emsp;然后还有另外一个知识点，较难理解，就是我们在其它文章中会见到的：父级向子级中传递约束，子级向父级中传递大小。这个知识点则体现在 performLayout 和 layout 两个函数的相互调用中：

&emsp;当 RenderObject 对象进行布局时会调用自己的 performLayout 函数，然后 performLayout 内部则是这个父级 RenderObject 调用其子级的 layout 函数并传入约束参数 constraints 和自己是否要使用子级的 size 参数 parentUsesSize，而在子级的 layout 函数内，又会调用自己的 performLayout 函数，实际最后就是：layout 会一级一级的向自己的子级中递归调用（一级一级的向子级中传递约束 constraints），然后在子级的 layout 函数调用完成后，父级自己决定自己是否要用子级的 size，而这就是我们经常在其它文章中见到的：父级向子级中传递约束，子级向父级中传递大小。
  
&emsp;然后其它内容的话都比较简单，快速浏览即可。下一节我们看 RenderObject 的 PAINTING 部分。⛽️

## 参考链接
**参考链接:🔗**
+ [RenderObject class](https://api.flutter.dev/flutter/rendering/RenderObject-class.html)
+ [RenderObjects?! | Decoding Flutter](https://www.youtube.com/watch?v=zmbmrw07qBc)
+ [PipelineOwner class](https://api.flutter.dev/flutter/rendering/PipelineOwner-class.html)
+ [RendererBinding mixin](https://api.flutter.dev/flutter/rendering/RendererBinding-mixin.html)
+ [PipelineManifold class](https://api.flutter.dev/flutter/rendering/PipelineManifold-class.html)
+ [Listenable class](https://api.flutter.dev/flutter/foundation/Listenable-class.html)
+ [ChangeNotifier class](https://api.flutter.dev/flutter/foundation/ChangeNotifier-class.html)
