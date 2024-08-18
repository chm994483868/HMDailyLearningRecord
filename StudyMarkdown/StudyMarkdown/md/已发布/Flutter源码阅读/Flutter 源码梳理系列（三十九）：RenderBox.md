# Flutter 源码梳理系列（三十九）：RenderBox

# RenderBox

&emsp;RenderBox：是一个在二维笛卡尔坐标系中的渲染对象。

&emsp;每个 RenderBox 的大小都由宽度和高度表示。每个 RenderBox 都有自己的坐标系（可以理解为 iOS 中的 Bounds），其左上角位于 (0, 0)。因此，RenderBox 的右下角位于 (width, height)。RenderBox 包含所有点，包括左上角，并延伸到但不包括右下角。

&emsp;通过向下传递一个 BoxConstraints 对象来执行 Box 布局。BoxConstraints 为子级的宽度和高度建立最小值和最大值。在确定其大小时，子级必须遵循其父级给予其的约束。（如果对 BoxConstraints 的定义以及其工作方式有点生疏的话，可以回看前面的 BoxConstraints 篇的内容。）（这段文档的对应的代码正是 RenderObject 的 layout 函数以及其众子类的 layout 函数的重写。）

```dart
// RenderObject 的 layout 函数：
void layout(Constraints constraints, { bool parentUsesSize = false }) { //... }
```

&emsp;这种协议足以表达许多常见的 Box 布局数据流。例如，要实现一个宽度输入，高度输出的数据流，可以使用一组具有严格宽度值的 Box 约束调用子级的布局函数（并为 parentUsesSize 参数传递 true）。在子级确定其高度后，使用子级的高度来确定父级的大小（size）。

## Writing a RenderBox subclass

&emsp;为了描述一个新的布局模型（layout model）、新的绘制模型（paint model）、新的点击测试模型（hit-testing model）或新的语义模型（semantics model），可以实现一个新的 RenderBox 子类，同时保持在 RenderBox 协议定义的笛卡尔空间中。

&emsp;如果要创建一个新的协议（非二维笛卡尔坐标系的），请考虑从 RenderObject 进行子类化。

### Constructors and properties of a new RenderBox subclass

&emsp;构造方法通常会为类的每个属性使用一个命名参数。然后将值传递给类的私有字段，并且构造方法会断言其正确性（例如，如果不应为 null，则断言它不是 null）。

&emsp;属性的形式为以下代码中的 getter/setter/field 组：

```dart
// getter
AxisDirection get axis => _axis;

// field
AxisDirection _axis = AxisDirection.down; // 或者在构造函数中初始化 _axis

// setter
set axis(AxisDirection value) {
  if (value == _axis) {
    return;
  }
  
  _axis = value;
  
  // 标记需要重新进行布局
  markNeedsLayout();
}
```

&emsp;通常，setter 会在最后调用 markNeedsLayout（如果布局使用该属性）或 markNeedsPaint（如果只有 painter 函数使用该属性）。没有必要同时调用这两个方法，因为 markNeedsLayout 包含了对 markNeedsPaint 的隐含调用。

&emsp;考虑布局和绘制操作是昂贵的；

&emsp;在调用 markNeedsLayout 或 markNeedsPaint 时应该谨慎。只有在布局（或绘制）实际上发生变化时才应该调用这两个方法。

### Children

&emsp;如果一个 RenderObject 是叶子节点，也就是说，它不能拥有任何子级节点，那么请忽略本节。（例如作为叶子节点的 RenderObject 示例包括：RenderImage、RenderParagraph 等。）

&emsp;对于带有子级节点的渲染对象，有四种可能的情况：

1. 仅有一个 RenderBox 类型的子级节点。在这种情况下，考虑继承自 RenderProxyBox（如果 RenderObject 的大小与此子级节点匹配）或 RenderShiftedBox（如果此子级节点会比 RenderBox 小且 RenderBox 会将此子级节点对齐在自身内部）。
```dart
// RenderProxyBox 是一个 RenderBox 的子类并且混入了 RenderObjectWithChildMixin 和 RenderProxyBoxMixin。
class RenderProxyBox extends RenderBox with RenderObjectWithChildMixin<RenderBox>, RenderProxyBoxMixin<RenderBox> { // ... }

// RenderShiftedBox 是一个 RenderBox 的抽象子类并且混入了 RenderObjectWithChildMixin。
abstract class RenderShiftedBox extends RenderBox with RenderObjectWithChildMixin<RenderBox> { // ... }
```

2. 仅有一个子级节点，但它并不是 RenderBox 类型的。使用 RenderObjectWithChildMixin 混入。（而使用 RenderObjectWithChildMixin 混入，可以直接理解为是 RenderObjectWithChildMixin 的子类，拥有一个 child 属性可以指向一个子级节点。）

3. 拥有一个子级节点列表（子级节点列表中：有 RenderObject/RenderBox 的子类类型）。使用 ContainerRenderObjectMixin 混入。（而使用 ContainerRenderObjectMixin 混入，可以直接理解为是 ContainerRenderObjectMixin 的子类，通过此继承便拥有了一对 firstChild 和 lastChild 指针，用于记录下以双向链表结构表示的子级节点列表。）

4. 更复杂的子级节点模型。

#### Using RenderProxyBox

&emsp;默认情况下，一个 RenderProxyBox 对象会根据其子级节点的大小调整自身大小，如果没有子级节点则会尽可能小；它会将所有的 hit testing 和绘制操作委托给子级节点，类似地，固有尺寸（intrinsic dimensions）和基线测量（baseline measurements）也会被代理给子级节点。（在 RenderProxyBoxMixin Mixin 中有这些逻辑的代码实现，可以看到全权被交由它的 child 属性实现。）

&emsp;RenderProxyBox 的子类只需重写 RenderBox 协议中相关的部分。例如，RenderOpacity 只需要重写 paint 方法（并根据 paint 方法的行为更新 alwaysNeedsCompositing，以及访问 visitChildrenForSemantics 方法，使得当子级节点不可见时，它会在辅助工具中隐藏子级节点），并添加一个 RenderOpacity.opacity 属性。

&emsp;RenderProxyBox 假设子级节点的大小与父级节点大小相同，并在位置 0,0 处。如果这不是真的，则应使用 RenderShiftedBox。

#### Using RenderShiftedBox

&emsp;默认情况下，RenderShiftedBox 的行为类似于 RenderProxyBox，但不假设子级节点位于 0,0 的位置（而是使用子级节点的 parentData 属性中记录的实际位置），也不提供默认的布局算法。（可以看到 RenderShiftedBox 的内部事由也都交给了自己的 child 属性实现，但是在实现时向比与 RenderProxyBox 多了 offset 的参与，其它几乎都是一样的。）

#### Kinds of children and child-specific data

&emsp;一个 RenderBox 不一定需要具有 RenderBox 子级。一个 RenderBox 的子级可以是另一个 RenderObject 的子类。可以参见 RenderObject 中的讨论。

&emsp;子级可以具有由父级拥有但存储在子级上的附加数据，使用 parentData 字段。用于该数据的类型必须是继承自 ParentData 的类。当子级 attached 时，setupParentData 方法用于初始化子级的 parentData 字段。

&emsp;按照惯例，具有 RenderBox 类型的子级的 RenderBox 对象其 parentData 会使用 BoxParentData 类，该类具有一个 BoxParentData.offset 属性，用于存储相对于父级的子级位置。(RenderProxyBox 不需要此偏移量，因此是此规则的一个例外。)

#### Using RenderObjectWithChildMixin

&emsp;如果一个 RenderObject 只有一个子级节点，但该子级节点不是 RenderBox 类型，则 RenderObjectWithChildMixin 类会很有用。该类是一个泛型类，有一个类型参数，表示子级节点的类型。比如，如果你正在构建一个 RenderFoo 类，它接受一个 RenderBar 类型的子级对象，你可以按照以下方式使用这个 mixin：

```dart
class RenderFoo extends RenderBox with RenderObjectWithChildMixin<RenderBar> {
  // ...
}
```

&emsp;在这种情况下，由于 RenderFoo 类本身仍然是一个 RenderBox，因此仍然需要实现 RenderBox 的布局算法（layout algorithm），以及诸如固有尺寸（intrinsics and baselines）和基线、绘制和 hit testing 等功能。

#### Using ContainerRenderObjectMixin

&emsp;如果一个 RenderBox 可以有多个子级节点，那么 ContainerRenderObjectMixin 混合类可以用来处理样板代码。它使用一个链表来模拟子级列表，这种方式易于动态更改且可以高效地遍历。在这种模型下，随机访问是低效的；如果需要对子级进行随机访问，请查看更复杂子级模型的下一节。

&emsp;ContainerRenderObjectMixin 类有两个类型（泛型）参数。第一个是子级 RenderObject 的类型，第二个是它们的 parentData 类型。用于 parentData 的类本身必须混入 ContainerParentDataMixin 类；这是 ContainerRenderObjectMixin 存储子级链表的地方。一个 ParentData 类可以继承自 ContainerBoxParentData；这本质上是 BoxParentData 与 ContainerParentDataMixin 的混合。例如，如果一个 RenderFoo 类想要有一个 RenderBox 子级的链表，可以创建一个如下所示的 FooParentData 类：

```dart
class FooParentData extends ContainerBoxParentData<RenderBox> {
  // (any fields you might need for these children)
}

// ContainerBoxParentData ⬇️ 
abstract class ContainerBoxParentData<ChildType extends RenderObject> extends BoxParentData with ContainerParentDataMixin<ChildType> { }

// BoxParentData ⬇️ 
class BoxParentData extends ParentData {
  /// 在父级坐标系中绘制子级坐标系的偏移量。
  Offset offset = Offset.zero;

  @override
  String toString() => 'offset=$offset';
}

// ParentData ⬇️ 
class ParentData {
  /// 当 RenderObject 从 Render Tree 中移除时调用。
  @protected
  @mustCallSuper
  void detach() { }

  @override
  String toString() => '<none>';
}

// ContainerParentDataMixin ⏸️
mixin ContainerParentDataMixin<ChildType extends RenderObject> on ParentData {
  /// 父级的子级列表中的前一个兄弟节点。
  ChildType? previousSibling;
  /// 父级的子级列表中的下一个兄弟节点。
  ChildType? nextSibling;

  /// 清理兄弟指针
  @override
  void detach() {
    super.detach();
  }
}
```

&emsp;在 RenderBox 中使用 ContainerRenderObjectMixin 时，考虑混入 RenderBoxContainerDefaultsMixin，它提供了一组实用方法，用于实现 RenderBox 协议的常见部分（比如绘制子级）。

&emsp;因此，RenderFoo 类本身的声明应该如下所示：

```dart
// continuing from previous example...
class RenderFoo extends RenderBox with ContainerRenderObjectMixin<RenderBox, FooParentData>, RenderBoxContainerDefaultsMixin<RenderBox, FooParentData> {
  // ...
}
```

&emsp;在遍历子级（例如在布局过程中）时，通常会使用以下模式（在本例中假设子级都是 RenderBox 对象，并且此 RenderObject 使用 FooParentData 对象来存储其子级的 parentData 字段）。

```dart
// continuing from previous example...
RenderBox? child = firstChild;
while (child != null) {
  final FooParentData childParentData = child.parentData! as FooParentData;
  
  // ...operate on child and childParentData...
  assert(child.parentData == childParentData);
  
  // 这里，更新 child 指针时是使用 child 的 parentData 属性的 nextSibling 属性
  child = childParentData.nextSibling;
}
```

#### More complicated child models

&emsp;在 Flutter 中，除了上面提到的一个子级以及双向链表结构的子级列表，RenderObject 也可以有更复杂的子级模型，例如 a map of children keyed on an enum，or a 2D grid of efficiently randomly-accessible children，or multiple lists of children 等等。如果一个 RenderObject 有一个无法通过上述 Mixin 处理的模型，那么它必须自己实现 RenderObject 子级协议，步骤如下：

1. 每当删除一个子级时，使用 dropChild 方法丢弃该子级。
2. 每当添加一个子级时，使用 adoptChild 方法采用该子级。
3. 实现 attach 方法以便在每个子级上调用 attach。
4. 实现 detach 方法以便在每个子级上调用 detach。
5. 实现 redepthChildren 方法以便在每个子级上调用 redepthChild。
6. 实现 visitChildren 方法以便为每个子级调用其参数，通常按照绘制顺序（从后到前）。
7. 实现 debugDescribeChildren 方法以便为每个子级输出一个诊断节点。

&emsp;实现这七个要点基本上就是前面提到的两个 Mixin 类所做的全部工作。

### Layout

&emsp;RenderBox 类实现了布局算法。它们有一组给定的约束条件，并根据这些约束条件和可能有的其他输入（例如子级或属性）来确定自己的大小。

&emsp;在实现 RenderBox 子类时，需要做出一个选择。它是仅根据约束条件确定自己的大小，还是使用其他信息来确定自己的大小？纯粹基于约束条件确定大小的示例包括根据父级进行调整大小。

&emsp;仅基于约束条件确定大小允许系统进行一些重要的优化。采用这种方法的类应该重写 sizedByParent 方法返回 true，然后重写 computeDryLayout 方法来计算大小，仅使用约束条件，例如：

```dart
@override
bool get sizedByParent => true;

@override
Size computeDryLayout(BoxConstraints constraints) {
  return constraints.smallest;
}
```

&emsp;否则，大小是在 performLayout 函数中设置的。

&emsp;performLayout 函数是 RenderBox 在决定它们的大小（如果它们不是由父级进行设置）以及决定它们的子级应该位于何处时所使用的函数。

#### Layout of RenderBox children

&emsp;performLayout 函数应调用每个 RenderBox 子级的 layout 函数，向其传递一个描述子级可渲染约束的 BoxConstraints 对象。向子级传递 isTight 约束（参见 BoxConstraints.isTight）将允许 rendering library 应用一些优化，因为它知道如果约束是 isTight 的，即使子级自身的布局发生变化，子级的大小也不会改变。

&emsp;如果 performLayout 函数将使用子级的大小来影响布局的其他方面，例如如果 RenderBox 围绕子级自身大小调整自身大小，或者基于子级的大小定位多个子级，则必须为子级的布局函数指定 parentUsesSize 参数，并将其设置为 true。

&emsp;此标志将关闭一些优化；不依赖于子级大小的算法将更有效。特别是，依赖于子级的大小意味着如果子级被标记为需要重新布局，那么父级也可能会被标记为需要重新布局，除非父级向子级提供的约束是 isTight 约束。

&emsp;对于不从 RenderProxyBox 继承的 RenderBox 类，一旦它们布局了子级，还应通过设置每个子级的 parentData 对象的 BoxParentData.offset 属性，来定位它们。

#### Layout of non-RenderBox children

&emsp;一个 RenderBox 的子级不一定非要是 RenderBox 自身。如果它们使用另一个协议（如在 RenderObject 中讨论的），那么父级不会传递 BoxConstraints，而会传递适当的 Constraints 子类，而且父级也不会读取子级的大小，而是读取该布局协议的布局输出。parentUsesSize 标记仍然用来指示父级是否将读取该输出，并且如果子级具有 isTight 约束（由 Constraints.isTight 定义），则优化仍然会启动。

### Painting

&emsp;要描述 RenderBox 是如何绘制的，需要实现 paint 方法。它接收一个 PaintingContext 对象（绘制上下文）和一个 Offset。绘制上下文提供了影响 Layer Tree 的方法，以及一个可用于添加绘图命令的 PaintingContext.canvas。canvas 对象不应在对 PaintingContext 的方法调用之间缓存；每次调用 PaintingContext 的方法时，canvas 的标识可能会发生变化。Offset 指定了 RenderBox 左上角在 PaintingContext.canvas 的坐标系中的位置。

&emsp;要在 Canvas 上绘制文本，可以使用 TextPainter。

&emsp;要将 Image 绘制到 Canvas 上，可以使用 paintImage 方法。

&emsp;使用 PaintingContext 中引入新 Layer 的方法的 RenderBox 应该重写 alwaysNeedsCompositing getter 并将其设置为 true。如果对象有时需要 Layer 合成，有时则不需要，可以在某些情况下返回 true，在其他情况下返回 false。在这种情况下，每当返回值发生变化时，都需要调用 markNeedsCompositingBitsUpdate。（当添加或移除子级时，这会自动完成，因此如果 alwaysNeedsCompositing getter 的值仅基于子级的存在与否而改变，则无需显式调用它。）

&emsp;每当对象上发生任何更改导致 paint 方法绘制不同内容（但不会导致布局更改）时，对象应调用 markNeedsPaint。

#### Painting children

&emsp;paint 方法的 context 参数有一个 PaintingContext.paintChild 方法，应该为每个需要绘制的子级调用该方法。需要给该方法传递一个指向子级的引用，以及一个 Offset 对象，表示子级在父级中的位置。

&emsp;如果在 paint 方法中对绘制上下文应用转换(transform)（或者通常在给定的初始 offset 之外应用附加 offset），那么还应该重写 applyPaintTransform 方法。该方法必须以与在绘制给定子级之前转换绘制上下文和偏移值相同的方式来调整其所接收的矩阵。这将在 globalToLocal 和 localToGlobal 方法中使用。

#### Hit Tests

&emsp;用于 RenderBox 的 hit testing 是通过 hitTest 方法实现的。这个方法的默认实现委托给了 hitTestSelf 和 hitTestChildren。在实现 hit testing 时，你可以选择重写这两个方法，或者忽略它们，直接重写 hitTest 方法。

&emsp;hitTest 方法本身接收一个 Offset 参数，如果对象或其子对象中的一个已吸收了点击（阻止了下方的对象被点击），则必须返回 true；如果点击可以继续传递到这个对象下方的其他对象，则必须返回 false。

&emsp;对于每个子级 RenderBox，应该使用相同的 HitTestResult 参数调用子级上的 hitTest 方法，并将点转换为子级的坐标空间（以与 applyPaintTransform 方法相同的方式）。默认实现委托给 hitTestChildren 来调用子级。RenderBoxContainerDefaultsMixin 提供了 RenderBoxContainerDefaultsMixin.defaultHitTestChildren 方法，假设子对象是轴对齐的、未被转换（transformed）并且根据 parentData 的 BoxParentData.offset 属性进行定位；更复杂的 Box 可以相应地重写 hitTestChildren。

&emsp;如果对象被点击，则还应将自身添加到作为 hitTest 方法参数给出的 HitTestResult 对象中，使用 HitTestResult.add 方法。默认实现委托给 hitTestSelf 来确定 RenderBox 是否被点击。如果对象在子对象之前添加自身，则会导致它被视为在子对象上方。如果它在子对象之后添加自身，则会被视为在子对象下方。添加到 HitTestResult 对象的条目应该使用 BoxHitTestEntry 类。这些条目随后会按添加顺序被系统遍历，对于每个条目，将调用目标的 handleEvent 方法，传递 HitTestEntry 对象。

&emsp;Hit testing 不能依赖于绘画是否已发生。

### Semantics

&emsp;为了使一个 RenderBox 可访问，需要实现 describeApproximatePaintClip、visitChildrenForSemantics 和 describeSemanticsConfiguration 方法。默认实现适用于仅影响布局的对象，但表示交互组件或信息（图表、文本、图像等）的节点应提供更完整的实现。

### Intrinsics and Baselines

&emsp;layout、paint、hit testing and semantics protocols 对所有 RenderObject 都是通用的，但是对于 RenderBox 对象的话，必须实现两个额外的协议：固有尺寸（或者固有大小）和基线测量（intrinsic sizing and baseline measurements）。

&emsp;有四种方法用于实现固有尺寸，计算 RenderBox 的最小和最大固有宽度和高度。

&emsp; 如下这些方法的文档详细讨论了固有尺寸协议：（四个函数分别计算：最小固有宽度、最大固有宽度、最小固有高度、最大固有高度，正是与 BoxConstraints 约束的四个属性：minWidth、maxWidth、minWidth、maxHeight 相对应。）

+ computeMinIntrinsicWidth、
+ computeMaxIntrinsicWidth、
+ computeMinIntrinsicHeight、
+ computeMaxIntrinsicHeight。

&emsp;如果重写了这些方法中的任何一个，请确保在单元测试中将 debugCheckIntrinsicSizes 设置为 true，这将添加额外的检查来帮助验证实现。

&emsp;另外，如果 RenderBox 有任何子级，它必须实现 computeDistanceToActualBaseline。RenderProxyBox 提供了一个简单的实现，将其转发给子级；RenderShiftedBox 提供了一个实现，通过子级相对于父级的位置来偏移子级的基线信息。但是，如果不继承这两个类中的任何一个，则必须自己实现算法。

+ Object -> RenderObject -> RenderBox

&emsp;下面开始看一下 RenderBox 的源码实现。

```dart
abstract class RenderBox extends RenderObject {
  // ...
}
```

## setupParentData

&emsp;重写父类的 setupParentData 函数来正确设置子级的 parentData。可以调用该函数，在子级被添加到父级的子级列表之前，为子级设置 parentData。可以看到这里默认设置的是 BoxParentData 类型。

```dart
  @override
  void setupParentData(covariant RenderObject child) {
    // 如果 child 的 parentData 不是 BoxParentData 类型的话，设置 child.parentData 是 BoxParentData。
    if (child.parentData is! BoxParentData) {
      child.parentData = BoxParentData();
    }
  }
```

## `_LayoutCacheStorage`

&emsp;可看到 `_LayoutCacheStorage` 仅有 4 个 Map 类型的属性，分别用来存储：Intrinsic Dimensions、Dry Layout Sizes、Alphabetic Baseline、Ideo Baseline。

```dart
  final _LayoutCacheStorage _layoutCacheStorage = _LayoutCacheStorage();
  
  final class _LayoutCacheStorage {
  
  Map<(_IntrinsicDimension, double), double>? _cachedIntrinsicDimensions;
  Map<BoxConstraints, Size>? _cachedDryLayoutSizes;
  Map<BoxConstraints, BaselineOffset>? _cachedAlphabeticBaseline;
  Map<BoxConstraints, BaselineOffset>? _cachedIdeoBaseline;

  // Returns a boolean indicating whether the cache storage has cached intrinsics / dry layout data in it.
  bool clear() {
    final bool hasCache = (_cachedDryLayoutSizes?.isNotEmpty ?? false)
                       || (_cachedIntrinsicDimensions?.isNotEmpty ?? false)
                       || (_cachedAlphabeticBaseline?.isNotEmpty ?? false)
                       || (_cachedIdeoBaseline?.isNotEmpty ?? false);

    if (hasCache) {
      _cachedDryLayoutSizes?.clear();
      _cachedIntrinsicDimensions?.clear();
      _cachedAlphabeticBaseline?.clear();
      _cachedIdeoBaseline?.clear();
    }
    
    return hasCache;
  }
}
```

## `_computeIntrinsics`

&emsp;内部封装 `_computeWithTimeline` 函数，用于计算固有尺寸。

```dart
  Output _computeIntrinsics<Input extends Object, Output>(
    _CachedLayoutCalculation<Input, Output> type,
    Input input,
    Output Function(Input) computer,
  ) {
    // performResize 不应该依赖于传入约束之外的任何东西
    bool shouldCache = true;
    
    // release 模式仅用于调用 _computeWithTimeline 函数
    return shouldCache ? _computeWithTimeline(type, input, computer) : computer(input);
  }
```

## `_computeWithTimeline`

&emsp;从 `_layoutCacheStorage` 中读取缓存的数据。（Timeline 用于在 debug 模式下进行性能分析使用。）

```dart
  Output _computeWithTimeline<Input extends Object, Output>(
    _CachedLayoutCalculation<Input, Output> type,
    Input input,
    Output Function(Input) computer,
  ) {
    final Output result = type.memoize(_layoutCacheStorage, input, computer);
    return result;
  }
```

## getMinIntrinsicWidth

&emsp;返回此 RenderBox 能够具有的最小宽度，以便能够正确地绘制其内容，而不会发生内容裁剪。

&emsp;height 参数可能会提供一个特定的高度假设。给定的高度可以是无限的，这意味着请求在不受约束的环境中的固有宽度。给定的高度不应为负数或空。

&emsp;此函数只能在 RenderBox 子类上调用。调用此函数会将子级与父级关联，因此当子级的布局发生更改时，父级会收到通知（通过 markNeedsLayout）。

&emsp;调用此函数是昂贵的，因为可能导致 O(N^2) 的行为。不要重写此方法。相反，实现 computeMinIntrinsicWidth。

```dart
  @mustCallSuper
  double getMinIntrinsicWidth(double height) {
    return _computeIntrinsics(_IntrinsicDimension.minWidth, height, computeMinIntrinsicWidth);
  }
```

## computeMinIntrinsicWidth

&emsp;计算 getMinIntrinsicWidth 返回的值。不要直接调用此函数，而应调用 getMinIntrinsicWidth。

&emsp;在实现 performLayout 的子类中进行重写。这个方法应该返回此 RenderBox 可以具有的最小宽度，以便能够正确地在自身内部绘制其内容，而不会裁剪。

&emsp;如果布局算法独立于上下文（例如，它总是尝试成为特定大小），或者布局算法是宽度输入、高度输出，或者布局算法同时使用传入的宽度和高度约束（例如，它总是将自身大小设置为 BoxConstraints.biggest），那么应忽略 height 参数。

&emsp;如果布局算法严格是高度输入、宽度输出，或者是在宽度不受约束时是高度输入、宽度输出，那么 height 参数就是要使用的高度。

&emsp;height 参数永远不会是负数或 null。它可能是无穷大。

&emsp;如果此算法依赖于子级的固有尺寸，则应使用以 get 开头的函数来获取该子级的固有尺寸，而不是使用 compute。

&emsp;此函数不应返回负值或无限值。

### Examples

#### Text

&emsp;英文文本是宽度输入、高度输出算法的典型例子。因此，高度参数被忽略。

&emsp;考虑字符串 "Hello World"。最大固有宽度（从 computeMaxIntrinsicWidth 返回）将是没有换行的字符串的宽度。

&emsp;最小固有宽度将是最宽的单词的宽度，即 "Hello" 或 "World"。然而，如果文本在一个更窄的宽度下渲染，可能仍然不会溢出。例如，渲染可能会在单词的中间断开一半，如 "Hel⁞lo⁞Wor⁞ld"。然而，这不是正确的渲染方式，computeMinIntrinsicWidth 被定义为返回 Box 可能的最小宽度，而不会在其中无法正确绘制内容。

&emsp;因此，对于小于最小固有宽度的给定宽度，最小固有高度可能大于最小固有宽度的最小固有高度。

#### Viewports (e.g. scrolling lists)

&emsp;有些 RenderBox 是用于裁剪其子级的。例如，滚动列表的 render box 可能总是将自身大小调整为其父级大小（或者说，调整为最大的传入约束），而不考虑子对象的大小，然后裁剪子对象并根据当前滚动偏移量对它们进行定位。

&emsp;在这些情况下，固有尺寸仍然取决于子级，尽管布局算法以与子级无关的方式调整 Box 的大小。重要的是需要用来绘制 Box 内内容（在这种情况下是子级）的尺寸，而不是裁剪的尺寸。

#### When the intrinsic dimensions cannot be known

&emsp;有些情况下，RenderObject 没有有效的方法来计算它们的固有尺寸。例如，对于 lazy viewport 的每个子项重新实体化和测量可能成本过高（viewport 通常只实例化实际可见的子项），或者尺寸可能是由一个回调计算的，RenderObject 无法推理。

&emsp;在这种情况下，实际上返回有效答案可能是不可能的（或者至少是不切实际的）。在这种情况下，当 RenderObject.debugCheckingIntrinsics 为 false 且启用了断言时，固有函数应该抛出异常，并在其他情况下返回 0.0。

&emsp;可参见 LayoutBuilder 或 RenderViewportBase 的实现示例（特别是 RenderViewportBase.debugThrowIfNotCheckingIntrinsics）。

#### Aspect-ratio-driven boxes

&emsp;对于一些 Box，它们根据约束始终返回固有尺寸。对于这些 Box，当传入的高度或宽度参数是有限的时候，固有函数应返回适当的大小，将其视为在相应方向上的 tight 约束，同时将另一方向的约束视为无限制（unbounded）。这是因为计算最小固有宽度（computeMinIntrinsicWidth）和最小固有高度（computeMinIntrinsicHeight）的定义是基于尺寸可能的取值，而在这种情况下，这些 Box 只能有一种大小。

&emsp;当传入的参数不是有限的时候，它们应返回基于内容的实际固有尺寸，与任何其他 Box 一样。

```dart
  @protected
  double computeMinIntrinsicWidth(double height) {
    return 0.0;
  }
```

## getMaxIntrinsicWidth

&emsp;返回宽度超过该宽度时增加宽度不会减少首选高度的最小宽度。首选高度是在该宽度下调用 getMinIntrinsicHeight 时返回的值。

&emsp;height 参数可能给出一个特定的高度假定。给定的高度可以是无穷大，表示正在请求在无约束环境中的内在宽度。给定的高度不应为负数或 null。

&emsp;此函数应仅调用其子级。调用此函数会将子级与父级联系起来，因此当子的布局更改时，将通知父级（通过 markNeedsLayout）。

&emsp;调用此函数是昂贵的，因为其可能导致 O(N^2) 的行为。

&emsp;不要重写此方法。而应实现 computeMaxIntrinsicWidth。

```dart
  @mustCallSuper
  double getMaxIntrinsicWidth(double height) {    
    // getMaxIntrinsicWidth 的 height 参数不能为负或为空。
    return _computeIntrinsics(_IntrinsicDimension.maxWidth, height, computeMaxIntrinsicWidth);
  }
```

## computeMaxIntrinsicWidth

&emsp;计算 getMaxIntrinsicWidth 返回的值。请不要直接调用这个函数，而是调用 getMaxIntrinsicWidth。

&emspp在实现 performLayout 方法的子类中进行重写。这个方法应返回一个最小的宽度，超过这个宽度增加宽度不会导致首选高度减小。首选高度是根据该宽度计算出的 computeMinIntrinsicHeight 返回的值。

&emsp;如果布局算法严格地遵循高度对应宽度的原则，或者是在宽度没有限制时高度对应宽度，那么这个方法应返回与相同高度下 computeMinIntrinsicWidth 返回的值相同。

&emsp;否则，应忽略高度参数，并且返回的值应等于或大于 computeMinIntrinsicWidth 返回的值。

&emsp;高度参数永远不会是负数或空。它可以是无限大的。

&emsp;该方法返回的值可能并不精确匹配对象实际的尺寸。例如，一个总是使用 BoxConstraints.biggest 精确调整自身尺寸的 RenderBox 子类可能会比其最大固有尺寸更大。

&emsp;如果该算法依赖于子级的固有尺寸，则应使用以 get 开头的函数获得该子级的固有尺寸，而不是 compute。

&emsp;这个函数永远不应返回负数或无穷大。

```dart
  @visibleForOverriding
  @protected
  double computeMaxIntrinsicWidth(double height) {
    return 0.0;
  }
```

## getMinIntrinsicHeight

&emsp;返回此 RenderBox 能够具有的最小高度，以便在其中正确绘制其内容，而不会发生裁剪。

&emsp;width 参数可以提供一个特定的宽度来假设。给定的宽度可以是无限的，表示需要请求在不受限制的环境中的固有高度。给定的宽度不应为负数或空。

&emsp;此函数只应调用其子级。调用此函数会将子级与父级耦合，因此当子级的布局发生更改时，父级会收到通知（通过 markNeedsLayout）。

&emsp;调用此函数是昂贵的，因为它可能导致 O(N^2) 的行为。

&emsp;不要重写此方法。而是实现 computeMinIntrinsicHeight。

```dart
  @mustCallSuper
  double getMinIntrinsicHeight(double width) {
    return _computeIntrinsics(_IntrinsicDimension.minHeight, width, computeMinIntrinsicHeight);
  }
```

## computeMinIntrinsicHeight

&emsp;计算由 getMinIntrinsicHeight 返回的值。不要直接调用此函数，而是调用 getMinIntrinsicHeight。

&emsp;在子类中实现 performLayout 方法时需要进行重写。该方法应返回此 RenderBox 可以使用的最小高度，以便在不裁剪内容的情况下正确绘制其内容。

&emsp;如果布局算法与上下文无关（例如，它总是尝试成为特定大小），或者布局算法是高度输入宽度输出的，或者布局算法同时使用传入的高度和宽度约束（例如，它总是将自身尺寸设置为 BoxConstraints.biggest）,那么应该忽略宽度参数。

&emsp;如果布局算法严格按照宽度输入、高度输出的方式进行，或者在高度无约束时遵循宽度输入、高度输出的方式，则宽度参数将被使用。

&emsp;width 参数永远不会是负值或 null。它可以是无限的。

&emsp;如果此算法依赖于子级的固有尺寸，则应使用名称以 get 开头的函数来获取该子级的固有尺寸，而不是使用 compute。

&emsp;此函数不应返回负值或无限值。

```dart
  @visibleForOverriding
  @protected
  double computeMinIntrinsicHeight(double width) {
    return 0.0;
  }
```

## getMaxIntrinsicHeight

&emsp;返回超出该高度之后增加高度不会再减小首选宽度的最小高度。首选宽度是针对该高度返回的 getMinIntrinsicWidth 的值。

&emsp;width 参数可能提供了一个具体的宽度假设。给定的宽度可以是无限的，表示正在请求无约束环境下的内在高度。给定的宽度不应为负或为空。

&emsp;此函数应该只在子级上调用。调用此函数会将子级与父级耦合在一起，因此当子级的布局更改时，父级会收到通知（通过 markNeedsLayout）。

&emsp;调用此函数是昂贵的，因为它可能导致 O(N^2) 的行为。

&emsp;不要重写此方法。相反，请实现 computeMaxIntrinsicHeight。

```dart
  @mustCallSuper
  double getMaxIntrinsicHeight(double width) {
    return _computeIntrinsics(_IntrinsicDimension.maxHeight, width, computeMaxIntrinsicHeight);
  }
```

## computeMaxIntrinsicHeight

&emsp;计算 getMaxIntrinsicHeight 返回的值。不要直接调用这个函数，而是调用 getMaxIntrinsicHeight。

&emsp;在实现 performLayout 的子类中被重写。应当返回一个最小的高度，增加高度不会减少首选宽度的高度。首选宽度是对应该高度计算出的应该返回的值。

&emsp;如果布局算法严格遵循宽度输入、高度输出，或者在高度不受限制时是宽度输入、高度输出，那么这个函数应当返回与相同宽度对应的 computeMinIntrinsicHeight 一样的值。

&emsp;否则，应忽略宽度参数，返回的值应等于或大于 computeMinIntrinsicHeight 返回的值。

&emsp;宽度参数永远不会为负或为 null。它可能是无限大的。

&emsp;这个方法返回的值可能不会匹配对象实际占用的大小。例如，一个总是使用 BoxConstraints.biggest 精确调整自身大小的 RenderBox 子类可能会比其最大固有尺寸更大。

&emsp;如果算法依赖于子级的固有尺寸，那么应当使用以 get 开头的函数获取该子级的固有尺寸，而不是 compute 函数。

&emsp;这个函数不应返回负值或无限大值。

```dart
  @visibleForOverriding
  @protected
  double computeMaxIntrinsicHeight(double width) {
    return 0.0;
  }
```

## getDryLayout

&emsp;根据提供的 BoxConstraints 返回此 RenderBox 想要的 Size。

&emsp;此方法返回的 Size 保证与此 RenderBox 在相同约束条件下在布局时计算出的大小相同。

&emsp;此函数应仅在子级上调用。调用此函数会将子级与父级耦合，因此当子级的布局发生变化时，父级会被通知（通过 markNeedsLayout）。

&emsp;这种布局被称为 "dry" 布局，与 performLayout 执行的常规 "wet" 布局运行相反，因为它在不更改任何内部状态的情况下计算给定约束的所需大小。

&emsp;调用此函数昂贵，因为可能会导致 O(N^2) 的行为。

&emsp;不要重写此方法。而是实现 computeDryLayout。

```dart
  @mustCallSuper
  Size getDryLayout(covariant BoxConstraints constraints) {
    return _computeIntrinsics(_CachedLayoutCalculation.dryLayout, constraints, _computeDryLayout);
  }

  bool _computingThisDryLayout = false;
  
  Size _computeDryLayout(BoxConstraints constraints) {
    final Size result = computeDryLayout(constraints);
    return result;
  }
```

## computeDryLayout

&emsp;计算由 getDryLayout 返回的值。不要直接调用此函数，而是调用 getDryLayout。

&emsp;在实现 performLayout 或 performResize 的子类中重写此方法，或者在设置 sizedByParent 为 true 但未重写 performResize 时重写。此方法应返回该 RenderBox 在给定 BoxConstraints 的情况下希望得到的 Size。

&emsp;此方法返回的大小必须与 RenderBox 在 performLayout（如果 sizedByParent 为 true，则为 performResize）中为自身计算的大小相匹配。

&emsp;如果此算法依赖于子级的大小，则应使用其 getDryLayout 方法获取该子级的大小。

&emsp;此布局称为 "dry" 布局，与由 performLayout 执行的常规 "wet" 布局运行相反，因为它在不更改任何内部状态的情况下计算给定约束的期望大小。

```dart
  @visibleForOverriding
  @protected
  Size computeDryLayout(covariant BoxConstraints constraints) {
    return Size.zero;
  }
```

## getDryBaseline

&emsp;对于给定的约束条件，返回从 RenderBox 的顶部到 Box 内容的第一个基线的距离，如果这个 RenderBox 没有任何基线，则返回 null。

&emsp;这个方法在底层调用 computeDryBaseline，然后缓存结果。RenderBox 的子类通常不会重写 getDryBaseline。相反，考虑重写 computeDryBaseline，使其返回与 getDistanceToActualBaseline 一致的基线位置。

&emsp;通常，这个方法被父 RenderBox 的 computeDryBaseline 或 computeDryLayout 实现调用，以获取 RenderBox 子级的基线位置。与 getDistanceToBaseline 不同，这个方法接受一个 BoxConstraints 作为参数，并计算基线位置，就好像 RenderBox 是由父级使用这个 BoxConstraints 进行布局的。

&emsp;方法名称中的 "dry" 表示这个方法，类似于 getDryLayout，被调用时没有观察到任何副作用，而不同于 "wet" 布局方法，比如 performLayout（它会改变这个 RenderBox 的大小，以及其子元素的偏移量，如果有的话）。由于这个方法不依赖于当前布局，不同于 getDistanceToBaseline，因此即使这个 RenderBox 的布局过时，调用这个方法也是可以的。

&emsp;类似于固有宽度/高度和 getDryLayout，将这个函数在 performLayout 中调用是昂贵的，因为它可能导致 O(N^2) 的布局性能，其中 N 是渲染子树中的渲染对象数量。通常，这个方法应该只被父 RenderBox 的 computeDryBaseline 或 computeDryLayout 实现调用。

```dart
  double? getDryBaseline(covariant BoxConstraints constraints, TextBaseline baseline) {
    final double? baselineOffset = _computeIntrinsics(_CachedLayoutCalculation.baseline, (constraints, baseline), _computeDryBaseline).offset;
    return baselineOffset;
  }

  bool _computingThisDryBaseline = false;
  
  BaselineOffset _computeDryBaseline((BoxConstraints, TextBaseline) pair) {
    final BaselineOffset result = BaselineOffset(computeDryBaseline(pair.$1, pair.$2));
    return result;
  }
```

## computeDryBaseline

&emsp;计算由 getDryBaseline 返回的数值。

&emsp;此方法仅用于重写，并且不应直接调用。要获取给定约束条件下的此 RenderBox 的假设基线位置，请调用 getDryBaseline。

&emsp;方法名称中的 "dry" 表示在调用时实现不能产生可观察的副作用。例如，它不得更改 RenderBox 的大小或其子级的绘制偏移，否则在调用 paint 时会导致 UI 更改，或在调用 hitTest 时会导致 hit testing 行为更改。此外，访问此 RenderBox 或子 RenderBox 的当前布局（包括访问 size 或 child.size）通常表示实现中存在 bug，因为当前布局通常是使用不同于作为第一个参数给定的约束集来计算的 BoxConstraints 计算出来的。要在此方法的实现中获取此 RenderBox 或子 RenderBox 的大小，请使用 getDryLayout 方法。

&emsp;实现必须返回一个表示从 Box 顶部到 Box 内容的第一个基线之间的距离的值，对于给定的约束，或者如果 RenderBox 没有基线则返回 null。当此 RenderBox 在相同状态下以相同布局约束进行布局时，它将返回 RenderBox.computeDistanceToActualBaseline 将返回的完全相同值。

&emsp;并非所有 RenderBox 都支持 dry baseLine 计算。例如，要计算 LayoutBuilder 的 dry baseLine，可能需要使用不同的约束调用其 builder，这可能会产生副作用，比如更新 Widget Tree，违反 "dry" 协议。在这种情况下，RenderBox 必须在断言中调用 debugCannotComputeDryLayout，并返回虚拟的 baseline 偏移值（例如 null）。

```dart
  @visibleForOverriding
  @protected
  double? computeDryBaseline(covariant BoxConstraints constraints, TextBaseline baseline) {
    return null;
  }
```

## size

&emsp;在布局过程中计算的此 RenderBox 的大小。

&emsp;每当此对象标记为需要布局时，此 size 值就会过时。在 performLayout 过程中，在调用子级的布局函数时，如果传递 true 给 parentUsesSize，那么就不要读取子级的大小。

&emsp;一个 RenderBox 的大小应该只在该 Box 的 performLayout 或 performResize 函数中设置。如果希望在这些函数之外更改 Box 的大小，请调用 markNeedsLayout 代替以安排对该 Box 的布局。

```dart
  // 这个 RenderBox 是否已经经历了布局并具有尺寸。
  bool get hasSize => _size != null;

  Size get size {
    return _size ?? (throw StateError('RenderBox was not laid out: $runtimeType#${shortHash(this)}'));
  }
  
  Size? _size;
  
  @protected
  set size(Size value) {
    _size = value;
  }
```

## getDistanceToBaseline

&emsp;将 RenderBox 的位置的 y 坐标到 Box 内容中第一个给定基线的 y 坐标之间的距离返回。

&emsp;在某些布局模型中使用，以便将相邻 Box 与共同基线对齐，而不考虑填充、字体大小差异等。如果没有基线，则此函数返回从 Box 位置的 y 坐标到 Box 底部的 y 坐标之间的距离（即，Box 的高度），除非调用者为 onlyReal 参数传递 true，在这种情况下，函数将返回 null。

&emsp;只能在对 Box 调用 layout 后调用此函数。只允许在父框的 performLayout 或 paint 函数中从该父框的父级调用此函数。

&emsp;在实现 RenderBox 的子类时，要重写基线计算，需要重写 computeDistanceToActualBaseline 函数。

```dart
  double? getDistanceToBaseline(TextBaseline baseline, { bool onlyReal = false }) {
    final double? result;
    
    try {
      result = getDistanceToActualBaseline(baseline);
    } finally {
      // ...
    }
    
    if (result == null && !onlyReal) {
      return size.height;
    }
    
    return result;
  }
```

## getDistanceToActualBaseline

&emsp;调用 computeDistanceToActualBaseline 并缓存结果。此函数只能从 getDistanceToBaseline 和 computeDistanceToActualBaseline 中调用。不要直接从这两个方法之外调用此函数。

```dart
  @protected
  @mustCallSuper
  double? getDistanceToActualBaseline(TextBaseline baseline) {
    return _computeIntrinsics(
      _CachedLayoutCalculation.baseline,
      (constraints, baseline),
      ((BoxConstraints, TextBaseline) pair) => BaselineOffset(computeDistanceToActualBaseline(pair.$2)),
    ).offset;
  }
```

## computeDistanceToActualBaseline

&emsp;返回 RenderBox 位置的 y 坐标与 Box 内容中第一个给定基线的 y 坐标之间的距离，如果有的话；否则返回 null。

&emsp;请勿直接调用此函数。如果需要从 performLayout 或 paint 调用的子级知道基线，请调用 getDistanceToBaseline。

&emsp;子类应重写此方法以提供它们基线的距离。在实现此方法时，通常有三种策略：

+ 对于使用 ContainerRenderObjectMixin 子模型的类，请考虑混入 RenderBoxContainerDefaultsMixin 类，并使用 RenderBoxContainerDefaultsMixin.defaultComputeDistanceToFirstActualBaseline。
+ 对于自行定义特定基线的类，请直接返回该值。
+ 对于希望推迟计算给其子级的类，请在子级上调用 getDistanceToActualBaseline（不是 computeDistanceToActualBaseline，即内部实现，也不是 getDistanceToBaseline，即此 API 的公共入口点）。

```dart
  @visibleForOverriding
  @protected
  double? computeDistanceToActualBaseline(TextBaseline baseline) {
    return null;
  }
```

## constraints

&emsp;最近从父级接收到的盒约束（BoxConstraints）。

```dart
  @override
  BoxConstraints get constraints => super.constraints as BoxConstraints;
```

## markNeedsLayout

&emsp;将这个 RenderBox 的布局信息标记为需要更新，并根据该对象是否为重新布局边界来将其注册到 PipelineOwner，或者交由父级对象处理。

### Background

&emsp;与在 RenderObject 中写入操作后急切更新布局信息不同，我们将布局信息标记为脏，从而安排进行视觉更新。在视觉更新过程中，PipelineOwner 会更新 RenderObject 的布局信息。

&emsp;这种机制会批处理布局工作，使多个连续的写操作合并在一起，从而消除冗余计算。

&emsp;如果 RenderObject 的父级指示在计算其布局信息时使用其 RenderObject 子级之一的大小，那么当为子级调用此函数时，还会标记父级为需要布局。在这种情况下，由于父级和子级都需要重新计算其布局，因此只有通知 PipelineOwner 有关父级；当父级进行布局时，它将调用子级的布局方法，从而也会对子级进行布局。

&emsp;一旦在 RenderObject 上调用了 markNeedsLayout，debugNeedsLayout 将为该 RenderObject 返回 true，直到 PipelineOwner 刚刚调用了 RenderObject 的布局方法为止。

### Special cases

&emsp;一些 RenderObject 的子类，尤其是 RenderBox，有其他情况下需要通知其父类子类是否被标记为 dirty （例如，子类的固有尺寸或基线发生变化时）。这些子类会重写 markNeedsLayout 方法，并在通常情况下调用 super.markNeedsLayout()，或者在父类需要进行布局的情况下调用 markParentNeedsLayout。

&emsp;如果 sizedByParent 已经改变，会调用 markNeedsLayoutForSizedByParentChange，而不是 markNeedsLayout。

```dart
  @override
  void markNeedsLayout() {

    // 如果 _layoutCacheStorage.clear 返回 true，则该 [RenderBox] 的布局将被父级的布局算法使用
    //（可能父级仅在绘制时使用了内在尺寸，但没有好的方法去检测，因此我们保守地假设它是一个布局依赖项）。
    
    // 一个 RenderObject 的 performLayout 实现可能会依赖于子级的基线位置或固有尺寸，
    // 即使它们之间存在重新布局边界。
    // _layoutCacheStorage 非空表示父级依赖于这个 RenderBox 的基线位置或固有尺寸，因此可能需要重新布局，不考虑重新布局边界。
    
    // 有些计算可能会失败（例如，dry baseline）。
    // 布局依赖关系仍然建立，但仅限于无法计算 dry baseline 的 RenderBox 到查询 dry baseline 的祖先之间。
    
    if (_layoutCacheStorage.clear() && parent != null) {
      markParentNeedsLayout();
      return;
    }
    
    super.markNeedsLayout();
  }
```

## performResize

&emsp;仅使用约束条件更新 RenderBox 的大小。

&emsp;不要直接调用此函数：调用 layout 代替。当在布局过程中需要该 RenderObject 实际工作时，layout 将调用此函数。父级提供的布局约束可通过 constraints 属性获取。

&emsp;只有当 sizedByParent 为 true 时，才会调用此函数。

&emsp;默认情况下，此方法将大小设置为使用当前约束条件调用 computeDryLayout 的结果。不要重写此方法，考虑重写 computeDryLayout。

```dart
  @override
  void performResize() {
    // 具有 sizedbypent = true 的子类的默认行为
    size = computeDryLayout(constraints);
  }
```

## performLayout

&emsp;执行此 RenderObject 的布局计算工作。

&emsp;不要直接调用此函数：请调用 layout。只有在布局过程中实际需要此 RenderObject 执行工作时，才会调用此函数。父级提供的布局约束可以通过 constraints getter 访问。

&emsp;如果 sizedByParent 为 true，则此函数不应该实际改变此 RenderObject 的尺寸。相反，这项工作应由 performResize 完成。如果 sizedByParent 为 false，则此函数应该改变此 RenderObject 的尺寸，并指示其子级进行布局。

&emsp;在实现此函数时，必须对每个子级调用 layout，并在 parentUsesSize 取为 true 时。如果父级布局信息取决于子级的布局信息，则需要将 parentUsesSize 取为 true。将 parentUsesSize 取为 true 可以确保如果子级发生布局更改，则此 RenderObject 也会进行布局。否则，子级可以更改其布局信息而不通知此 RenderObject。

```dart
  @override
  void performLayout() {
  // RenderBox 子类需要重写 performLayout 来设置大小和布局任何子元素，
  // 或者，将 sizedByParent 设置为 true，以便 performResize() 调整 RenderObject 的大小。
  }
```

## hitTest

&emsp;确定给定 position 处的 RenderBox 集合。

&emsp;如果这个 RenderBox 或其子级之一吸收了点击（阻止了这些对象下方的对象被点击），则返回 true，并将包含该点的任何 RenderBox 添加到给定的点击测试结果（BoxHitTestResult result）中。如果点击可以继续到这个对象下方的其他对象，则返回 false。

&emsp;调用方负责将 position 从全局坐标转换为相对于当前这个 RenderBox 原点的位置。这个 RenderBox 负责检查给定的 position 是否在其边界内。

&emsp;如果需要转换（transforming），调用方需要调用 BoxHitTestResult.addWithPaintTransform、BoxHitTestResult.addWithPaintOffset 或 BoxHitTestResult.addWithRawTransform 来记录所需的转换操作至 HitTestResult。这些方法还将帮助将转换应用于位置。

&emsp;Hit testing 需要布局是最新的，但不要求绘制是最新的。这意味着 RenderBox 在 hitTest 中可以依赖于 performLayout 已被调用，但不能依赖于 paint 已被调用。例如，一个 RenderBox 可能是 RenderOpacity 对象的子节点，当 RenderOpacity 的不透明度为零时，它会对其子节点进行 hitTest，即使它不会绘制其子节点。

```dart
  bool hitTest(BoxHitTestResult result, { required Offset position }) {
    if (_size!.contains(position)) {
    
      if (hitTestChildren(result, position: position) || hitTestSelf(position)) {
      
        result.add(BoxHitTestEntry(this, position));
        
        return true;
      }
    }
    
    return false;
  }
```

## hitTestSelf

&emsp;如果这个 RenderObject 可以被点击，即使其子级没有被点击，就重写这个方法。

&emsp;如果指定的 position 应该被视为对这个 RenderObject 的一个点击，则返回 true。

&emsp;调用方负责将 position 从全局坐标转换为相对于此 RenderBox 原点的位置。这个 RenderBox 负责检查给定的 position 是否在其边界内。

&emsp;被 hitTest 使用。如果你重写了 hitTest 并且没有调用这个函数，那么你就不需要实现这个函数。

```dart
  @protected
  bool hitTestSelf(Offset position) => false;
```

## hitTestChildren

&emsp;重写此方法以检查是否有任何子级位于给定 position。

&emsp;子类应该在指定 position 至少有一个子级报告击中时返回 true。

&emsp;通常应该按照反向绘制顺序进行子级的命中测试，以便在子级重叠的位置进行命中测试时，命中处于视觉 "最上方" 的子级（即，后绘制的子级）。

&emsp;调用方负责将 position 从全局坐标变换为相对于该 RenderBox 原点的位置。同样，该 RenderBox 负责对其子级调用 hitTest 时传递的 position 进行变换。

&emsp;如果需要进行变换，子类需要调用 BoxHitTestResult.addWithPaintTransform、BoxHitTestResult.addWithPaintOffset 或 BoxHitTestResult.addWithRawTransform 来记录 BoxHitTestResult 中所需的变换操作。这些方法还将帮助将变换应用到 position 上。

&emsp;被 hitTest 使用。如果重写 hitTest 并且不调用此函数，则无需实现此函数。

```dart
  @protected
  bool hitTestChildren(BoxHitTestResult result, { required Offset position }) => false;
```

## applyPaintTransform

&emsp;将从父坐标系到此 RenderBox 的坐标系的变换与给定的变换相乘。

&emsp;此函数用于在 Box 之间转换坐标系。在绘制过程中应用 transform 的子类应该重写此函数以将这些 transform 因素考虑在计算中。

&emsp;RenderBox 实现会负责根据布局期间确定的给定子级的位置并存储在子项的 parentData 的 BoxParentData.offset 属性中，来调整矩阵。

```dart
  @override
  void applyPaintTransform(RenderObject child, Matrix4 transform) {
    final BoxParentData childParentData = child.parentData! as BoxParentData;
    final Offset offset = childParentData.offset;
    
    transform.translate(offset.dx, offset.dy);
  }
```

## globalToLocal

&emsp;将给定的 point 从全局坐标系统中的逻辑像素转换为此 RenderBox 的本地坐标系统。

&emsp;该方法将 point 从屏幕上的位置反投影到 widget 上，这与 MatrixUtils.transformPoint 不同。

&emsp;如果从全局坐标到本地坐标的变换是退化的，则此函数返回 Offset.zero。

&emsp;如果 ancestor 不为 null，则此函数将给定 point 从 ancestor 的坐标系转换为本地坐标系统（ancestor 必须是此 RenderBox 的祖先）。

&emsp;此方法是基于 getTransformTo 实现的。

```dart
  Offset globalToLocal(Offset point, { RenderObject? ancestor }) {    
    // 我们希望找到对应于屏幕上给定点（s）的点（p），但也确保它实际上位于本地渲染平面上，
    // 这样在本地空间中进行视觉准确的手势处理时会有用。
    // 为此，我们不能简单地将 2D 屏幕点转换为 3D 本地空间，因为屏幕空间缺少深度分量|z|，
    // 因此有许多 3D 点与屏幕点对应。我们必须先将屏幕点反投影到渲染平面上，
    // 以找到与屏幕点对应的真实 3D 点。我们在本地空间在解除透视后进行正交反投影。渲染平面由渲染框偏移量（o）和 Z 轴（n）指定。
    // 通过找到视图向量（d）与本地 X-Y 平面的交点来进行反投影：(o-s).dot(n) == (p-s).dot(n)，(p-s) == |z|*d。
    
    final Matrix4 transform = getTransformTo(ancestor);
    final double det = transform.invert();
    
    if (det == 0.0) {
      return Offset.zero;
    }
    
    final Vector3 n = Vector3(0.0, 0.0, 1.0);
    final Vector3 i = transform.perspectiveTransform(Vector3(0.0, 0.0, 0.0));
    final Vector3 d = transform.perspectiveTransform(Vector3(0.0, 0.0, 1.0)) - i;
    final Vector3 s = transform.perspectiveTransform(Vector3(point.dx, point.dy, 0.0));
    final Vector3 p = s - d * (n.dot(s) / n.dot(d));
    
    return Offset(p.x, p.y);
  }
```

## localToGlobal

&emsp;将给定 point 从此 RenderBox 的本地坐标系转换为逻辑像素中的全局坐标系。

&emsp;如果 ancestor 非空，则此函数将给定 point 转换为 ancestor 的坐标系（ancestor 必须是此 RenderBox 的祖先），而不是转换为全局坐标系。

&emsp;该方法是通过 getTransformTo 实现的。如果变换矩阵 transform matrix 将给定 point 放在无穷远的线上（例如，当变换矩阵为零矩阵时），此方法将返回（NaN，NaN）。

```dart
  Offset localToGlobal(Offset point, { RenderObject? ancestor }) {
    return MatrixUtils.transformPoint(getTransformTo(ancestor), point);
  }
```

## paintBounds

&emsp;返回一个包含此 RenderBox 绘制的所有像素的矩形。

&emsp;绘制边界可能比 size 大或小，size 是此 RenderBox 在布局期间占据的空间量。例如，如果此 RenderBox 投射阴影，该阴影可能会延伸到布局期间为此 RenderBox 分配的空间之外。

&emsp;绘制边界用于调整此 RenderBox 绘制的缓冲区的大小。如果 RenderBox 尝试绘制超出其绘制边界之外的内容，则可能没有足够的内存分配来表示 RenderBox 的视觉外观，这可能导致未定义的行为。

&emsp;返回的绘制边界在此 RenderBox 的本地坐标系统中。

```dart
  @override
  Rect get paintBounds => Offset.zero & size;
```

## handleEvent

&emsp;重写此方法以处理命中此 RenderBox 的 PointerEvent。

&emsp;对于 RenderBox 对象，entry 参数是一个 BoxHitTestEntry。通过这个对象，可以确定 PointerDownEvent 在本地坐标中的位置。（这很有用，因为 PointerEvent.position 是在全局坐标中的。）

&emsp;此方法的实现应按如下方式调用 debugHandleEvent，以便它们支持 debugPaintPointersEnabled：

```dart
class RenderFoo extends RenderBox {
  // ...

  @override
  void handleEvent(PointerEvent event, HitTestEntry entry) {
    assert(debugHandleEvent(event, entry));
    // ... handle the event ...
  }

  // ...
}
```

```dart
  @override
  void handleEvent(PointerEvent event, BoxHitTestEntry entry) {
    super.handleEvent(event, entry);
  }
```

## RenderBox 总结

&emsp;RenderBox 的内容看完了，整体看下来其实内容还是不少的，其父类 RenderObject 为其提供了作为渲染对象的基础功能，然后到了 RenderBox 这里则是引入了二维笛卡尔坐标系，并把焦点放在了 Box 布局模型和 hit testing 上。其中关于固有尺寸和基线的计算系列函数以及 hit testing 系列函数是我们学习的重点，但 RenderBox 作为基类并没有并不作详细的函数实现，更详细的实现则需要我们在 RenderBox 的子类中继续探索。在 RenderBox 这里我们初次见识了基于最大、最小宽度和最大最小高度为基础的盒模型布局，后续几篇我们会把重点放在 RenderBox 相关类中，深入学习一下这个盒模型的内容。 

## 参考链接
**参考链接:🔗**
+ [RenderBox class](https://api.flutter.dev/flutter/rendering/RenderBox-class.html)
