# Flutter 源码梳理系列（二十一）：RenderObject

# RenderObject

&emsp;RenderObject 是 Render Tree 中的一个对象。

&emsp;RenderObject 类层次结构是渲染库存在的核心原因。[RenderObjects?! | Decoding Flutter](https://www.youtube.com/watch?v=zmbmrw07qBc)

&emsp;RenderObject 具有父级，并且有一个称为 parentData 的 slot，父 RenderObject 可以在其中存储特定于子级的数据，例如子级位置。RenderObject 类还实现了基本的布局和绘制协议。（对这段文档进行解释：首先每个 RenderObject 对象都有一个 RenderObject? parent 指针，即自己的父级，然后还有一个 ParentData? parentData 属性，这个是它的父级传递来的数据，类型是 ParentData 的，可以被直接作用到自己身上，这里有点绕，从代码角度看其实就是我们之前在 ParentDataWidget 中见到的 `void applyParentData(RenderObject renderObject)` 函数，这个函数就是把当前 ParentDataWidget 对象中的数据应用到自己的子级 RenderObjectElement 节点的 RenderObject 上。例如我们日常使用的 Positioned Widget，它就是一个 ParentDataWidgt（子类），我们可以看看它的 applyParentData 函数实现，就是把自己的 left/right/top/bottom 等这些数据应用到 RenderObject 上，然后标记 RenderObject 的父级重新布局（markNeedsLayout）。）

&emsp;然而，RenderObject 类：

+ 没有定义子级模型（例如，一个节点是否有零个、一个或多个子级）。（但是提供了几个 RenderObject Mixin。）
+ 它也没有定义坐标系（例如，子级是在笛卡尔坐标中定位，还是在极坐标中定位等）。
+ 也没有特定的布局协议（例如，布局是宽度输入高度输出，约束输入尺寸输出，还是父级在子级布局之前或之后设置大小和位置等；或者子级是否允许读取其父级的 parentData slot）。

&emsp;RenderBox 子类布局系统使用了笛卡尔坐标系（同 iOS，屏幕左上角是原点，X 轴向右增大，Y 轴向下增大）。RenderBox 是一个直接继承自 RenderObject 的抽象类。大部分的 Widget 都会使用 RenderBox 作为它们的 render object，而不是直接使用 RenderObject。（`abstract class RenderBox extends RenderObject { // ... }`）

&emsp;大概意思就是：RenderObject 是一个特别底层的类，只提供了一些最基础功能，然后以它为基类创建子类，例如 RenderBox，它可以更方便的帮助我们在 Flutter 中进行布局和绘制。（后面我们会对 RenderBox 进行详细学习）

## Lifecycle

&emsp;当不再需要 RenderObject 时，必须进行销毁。对象的创建者负责对其进行销毁。通常情况下，创建者是 RenderObjectElement，当它被卸载（unmout）时，该 Element 会销毁它创建的 RenderObject。（这里则可以在 RenderObjectElement.unmout 中找到对应的代码：当 RenderObjectElement 卸载时，它首先记录下自己的旧 widget，然后调用 Element.unmount 函数进行 Element 节点被卸载是的基本逻辑：如果有 GlobalKey 的话，从 globalKeyRegistry 中把自己移除，把自己的 widget 置为 null，把自己的 dependencies 置为 null，把自己的生命周期置为 defunct。然后回到 RenderObjectElement.mount，拿着前面记录的旧 widget，调用：oldWidget.didUnmountRenderObject(renderObject)，即向外传递 renderObject 被卸载了，让上层有必要时也可以进行一些清理逻辑，然后调用 renderObject 的 dispose 函数，然后把自己的 renderObject 置为 null。）

&emsp;RenderObject 在调用 dispose 方法时负责清理所持有的任何昂贵资源，比如 Picture 或 Image 对象。这还包括 render object 直接创建的任何 Layer。dispose 的基本实现将会将 layer 属性设为 null（在 RenderObject.dispose 函数中可见）。RenderObject 子类还必须将直接创建的任何其他 layer 也设置为 null。（同样遵循谁创建了资源，那么当自己要被销毁时也要记得释放自己的创建的资源，谁创建谁释放。）

## Writing a RenderObject subclass

&emsp;在大多数情况下，直接从 RenderObject 进行子类化是过度的，直接从 RenderBox 开始会是一个更好的起点。然而，如果一个 render object 不想使用笛卡尔坐标系，那么它确实应该直接继承自 RenderObject。这允许它通过使用 Constraints 的一个新子类来定义自己的布局协议，而不是使用 BoxConstraints，并且可能使用一个全新的对象和值来表示输出结果，而不仅仅是一个 Size。这种增强的灵活性是以无法依赖 RenderBox 的特性为代价的。例如，RenderBox 实现了一个固有大小的协议，允许你测量子项而不完全布局，这样，如果子项的大小发生变化，父项将重新布局（以考虑子项的新尺寸）。这是一项微妙且容易出错的功能。(RenderBox 的内容我们后续会展开。)

&emsp;编写 RenderBox 的大部分内容同样适用于编写 RenderObject，因此建议阅读 RenderBox 中的讨论以进行背景了解。主要的差异在于布局和点击测试，因为这些是 RenderBox 主要专门处理的方面。

### Layout

&emsp;一个布局协议以 Constraints 的子类开始。有关如何编写 Constraints 子类的更多信息，请参阅 Constraints 部分的讨论。（后续我们会展开。）

&emsp;performLayout 方法应该接受 constraints 并应用它们（这里指 RenderObject 的 constraints 属性）。布局算法的输出是设置在 render object 的字段上，用于描述 render object 的几何形状，以便父级布局使用。例如，对于 RenderBox，输出是 RenderBox.size 字段。只有在父级在调用子级的布局时将 parentUsesSize 指定为 true 时，父级才应该读取这个输出。

&emsp;每当 render object 上的任何更改会影响该对象的布局时，都应调用 markNeedsLayout 方法标记需要重新布局。

### Hit Testing

&emsp;Hit testing 比布局更加灵活。没有可重写的方法，你需要自己提供一个。

&emsp;你的 hit testing 方法的一般行为应该类似于为 RenderBox 描述的行为。主要区别在于输入不一定是 Offset。当向 HitTestResult 添加条目时，你也可以使用不同的 HitTestEntry 子类。当调用 handleEvent 方法时，将传入与添加到 HitTestResult 中的相同对象，因此可以用于跟踪诸如击中的精确坐标等信息，无论新布局协议使用的坐标系是什么。（有点复杂，后面我们会深入学习）

### Adapting from one protocol to another

&emsp;一般来说，Flutter Render Object Tree 的根是一个 RenderView 对象。这个对象只有一个子级（child），子级必须是一个 RenderBox。因此，如果你想在 Render Tree 中有一个自定义的 RenderObject 子类，你有两种选择：要么需要替换 RenderView 本身，要么需要一个 RenderBox，这个 RenderBox 以你的类作为它的子级（child）。（后者是更常见的情况。）

&emsp;这个 RenderBox 子类从 box 协议转换为你的类的协议。

&emsp;具体来说，这意味着对于 hit testing，它重写 RenderBox.hitTest，并调用你的类中用于 hit testing 的任何方法。

&emsp;同样地，它重写 performLayout 来创建适合你的类的 Constraints 对象，并将其传递给子级（child）的 layout 方法。

### Layout interactions between render objects

&emsp;一般来说，render object 的布局应该只取决于其子级（child）布局的输出，而且只有在 layout 调用中将 parentUsesSize 设置为 true 时才会如此。此外，如果设置为 true，则如果要渲染子级（child），则父级必须调用子级（child）的 layout，否则当子级（child）更改其布局输出时，父级将不会收到通知。

&emsp;可以设置 render object 协议来传递额外的信息。例如，在 RenderBox 协议中，你可以查询子级们（children）的固有尺寸（intrinsic dimensions）和基线几何信息（baseline geometry）。但是，如果这样做的话，那么当父级在上一次布局阶段使用了额外信息时，子级必须在任何额外信息更改时调用父级的 markNeedsLayout 方法。关于如何实现这一点的示例，请参考 RenderBox.markNeedsLayout 方法。它重写了 RenderObject.markNeedsLayout 方法，以便当子级的几何信息更改时，如果父级已查询了固有或基线信息，则会被标记为脏。

&emsp;OK，RenderObject 的文档就这么多，下面我们开始看它的源码。

## Constructors

&emsp;看到 RenderObject 构造函数内部添加了 FlutterMemoryAllocations 的内容记录 RenderObject 对象的创建，以及对应的在 dispose 中记录 RenderObject 对象的销毁。这在之前的 State 和 Element 类中也见过相似的内容记录创建和销毁，当它们如果不是匹配出现的话就说明有内存泄漏了。

&emsp;这里看到 RenderObject 需要实现 HitTestTarget。HitTestTarget 只有一个函数：`void handleEvent(PointerEvent event, HitTestEntry<HitTestTarget> entry)`，而 RenderObject 对其只有一个空实现，交给具体的子类来实现具体内容。

```dart
abstract class RenderObject with DiagnosticableTreeMixin implements HitTestTarget {
  RenderObject() {
    // isRepaintBoundary 和 alwaysNeedsCompositing 默认都是 false，
    // 所以这里 _needsCompositing 默认也是 false
    _needsCompositing = isRepaintBoundary || alwaysNeedsCompositing;
    
    // 默认是 false
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
    markNeedsLayout();
    markNeedsCompositingBitsUpdate();
    markNeedsPaint();
    markNeedsSemanticsUpdate();
    
    // 递归在整个 Render 子树上所有 render object 执行 reassemble。
    visitChildren((RenderObject child) {
      child.reassemble();
    });
  }
```

## dispose

&emsp;释放由此 RenderObject 持有的任何资源。

&emsp;创建 RenderObject 的对象（RenderObjectElement）负责将其处理。如果这个 RenderObject 直接创建了任何 children，则也必须在此方法中处理这些 children。它不能处理由其他对象（比如 RenderObjectElement）创建的任何 children。当该 Element 卸载时，这些 children 将在那时被处理，这可能会延迟，如果 Element 移动到 Element Tree 的另一个部分。

&emsp;此方法的实现必须以调用继承方法结束，如 super.dispose()。

&emsp;在调用 dispose 后，render object 将不再可用。

```dart
  @mustCallSuper
  void dispose() {
    _layerHandle.layer = null;
  }
```

&emsp;OK，看了 RenderObject 的两个常规的函数：reassemble 和 dispose。然后接下来是一部分跟 Layout 相关的内容：

&emsp;LAYOUT。

## parentData

&emsp;供父级 render object 使用的数据。

&emsp;Parent data 被用于布局这个对象的渲染对象（通常是渲染树中该对象的父对象）存储与自身相关的信息，以及任何其他节点可能清楚知道这些数据意味着什么的。Parent data 对于子对象来说是不透明的。

+ parentData 属性不能直接设置（parentData 属性不是私有的，在外部也可以设置），除非通过在父级（parent RenderObject）上调用 setupParentData 方法进行设置。
+ 父节点的数据可以在将子节点添加到父节点之前设置，方法是在未来的父节点上调用 setupParentData。
+ 使用 parent data 的约定取决于父级和子级之间使用的布局协议。例如，在 box 布局中，parent data 是完全不透明的，但在 sector（扇区）布局中，子级被允许读取 parent data 的一些字段。

```dart
  ParentData? parentData;
```

## setupParentData

&emsp;重写此方法可以正确设置子级的 parent data。你可以调用此函数来在子级（child RenderObject）添加到父级的子级列表之前设置子级的 parent data。

&emsp;还记得

```dart
  void setupParentData(covariant RenderObject child) {
    if (child.parentData is! ParentData) {
      child.parentData = ParentData();
    }
  }
```

## depth

&emsp;在 Render Tree 中，此 render object 的深度。

&emsp;Render Tree 中节点的深度随着向下遍历而单调递增：一个节点的深度总是大于其祖先的深度。对于兄弟节点之间的深度没有保证。

&emsp;子节点的深度可以比父节点的深度高出不止一个，因为深度值永远不会减少：重要的是它比父节点的深度更大。考虑一个树，有根节点 A，子节点 B，和孙子节点 C。最初，A 的深度为 0，B 的深度为 1，C 的深度为 2。如果 C 被移到成为 A 的子节点，B 的兄弟，则数字不会改变。C 的深度仍然是 2。

&emsp;节点的深度用于确保节点按深度顺序处理。深度是通过 adoptChild 和 dropChild 方法自动维护的。

```dart
  int get depth => _depth;
  int _depth = 0;
```

## redepthChild

&emsp;将给定子节点的深度调整为高于此节点自身的深度。只能在 redepthChildren 的重写中调用此方法。

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

&emsp;调整此节点的子节点的深度，如果有的话。在具有子节点的子类中重写此方法，分别为每个子节点调用 redepthChild。不要直接调用此方法。

```dart
  @protected
  void redepthChildren() { }
```

## parent

&emsp;在 Render Tree 中，此 render object 的父级。Render Tree 中根节点的父级是 null。

```dart
  RenderObject? get parent => _parent;
  RenderObject? _parent;
```

## adoptChild

&emsp;当子类决定将一个 render object 作为子级时调用。仅供子类在更改其子级列表时使用。在其他情况下调用此方法将导致不一致的树形结构，并可能导致崩溃。

```dart
  @mustCallSuper
  @protected
  void adoptChild(RenderObject child) {
    setupParentData(child);
    
    markNeedsLayout();
    markNeedsCompositingBitsUpdate();
    markNeedsSemanticsUpdate();
    
    child._parent = this;
    
    if (attached) {
      child.attach(_owner!);
    }
    
    redepthChild(child);
  }
```

## dropChild

&emsp;当子类决定一个 render object 不再是子对象时调用。只能由子类在更改其子对象列表时使用。在其他情况下调用这个方法将导致不一致的树，可能会导致崩溃。

```dart
  @mustCallSuper
  @protected
  void dropChild(RenderObject child) {
    child._cleanRelayoutBoundary();
    
    child.parentData!.detach();
    child.parentData = null;
    child._parent = null;
    
    if (attached) {
      child.detach();
    }
    
    markNeedsLayout();
    markNeedsCompositingBitsUpdate();
    markNeedsSemanticsUpdate();
  }
```

## visitChildren

&emsp;在子类中重写此方法，对该 render object 的每个直接子对象调用入参 visitor。

```dart
  void visitChildren(RenderObjectVisitor visitor) { }
```

## owner

&emsp;这个 render object 的所有者（如果未附加则为 null）。包含这个 RenderObject 的整个 Render Tree 将拥有相同的 owner。（类似 Element 的 BuildOwner。）

```dart
  PipelineOwner? get owner => _owner;
  PipelineOwner? _owner;
```

## attached

&emsp;此 render object 所属的 Rener Tree 是否已附加到 PipelineOwner 中。在调用 attach 时，该值变为 true。在调用 detach 时，该值变为 false。

```dart
  bool get attached => _owner != null;
```

## attach

&emsp;将该 render object 标记为已附加到指定的 owner。

&emsp;通常只从父级的 attach 方法调用，并且由 owner 调用以将 Render Tree 的根标记为已附加。

&emsp;具有子项的子类应该重写此方法，在调用继承的方法后，如super.attach(owner)，将所有子项附加到相同的 owner。

```dart
  @mustCallSuper
  void attach(PipelineOwner owner) {
    _owner = owner;
    
    // 如果节点在未附加时出现了脏数据，请确保在有 owner 可用时将其添加到相应的脏数据列表中。
    if (_needsLayout && _relayoutBoundary != null) {
      
      // 如果我们从未进行过任何布局，则不要进入此代码块；
      // scheduleInitialLayout() 将会处理这种情况。
      _needsLayout = false;
      markNeedsLayout();
    }
    
    if (_needsCompositingBitsUpdate) {
      _needsCompositingBitsUpdate = false;
      markNeedsCompositingBitsUpdate();
    }
    
    if (_needsPaint && _layerHandle.layer != null) {
      
      // 如果我们从未绘制过任何内容，则不要进入这个代码块；
      // scheduleInitialPaint() 将会处理这种情况。
      _needsPaint = false;
      markNeedsPaint();
    }
    
    if (_needsSemanticsUpdate && _semanticsConfiguration.isSemanticBoundary) {
    
      // 如果我们从未更新过语义，则不要进入这个代码块。
      // scheduleInitialSemantics() 将会处理这种情况。
      _needsSemanticsUpdate = false;
      markNeedsSemanticsUpdate();
    }
  }
```

## detach

&emsp;将此 render object 标记为与其 PipelineOwner 分离。

&emsp;通常仅从父级的 detach 中调用，并且由 owner 标记树的根节点为 detached。

&emsp;具有子级的子类应该重写此方法，在调用继承的方法后 detach 所有子级，如 super.detach()。

```dart
  @mustCallSuper
  void detach() {
    _owner = null;
  }
```

## `_needsLayout`

&emsp;

```dart
  bool _needsLayout = true;
```

## `_relayoutBoundary`

&emsp;

```dart
  RenderObject? _relayoutBoundary;
```

## constraints

&emsp;最近由父组件提供的布局约束。

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

&emsp;将此 render object 的布局信息标记为脏，并根据此对象是否为重新布局边界（relayoutBoundary）来将其注册到其 PipelineOwner，或者推迟到其父级。

```dart
  void markNeedsLayout() {
    // 如果已经被标记为需要 Layout 了，直接 return 即可
    if (_needsLayout) {
      return;
    }
    
    // 如果重绘边界为 null，则继续往上标记父级重新布局
    if (_relayoutBoundary == null) {
      _needsLayout = true;
      
      if (parent != null) {
        // _relayoutBoundary 在 RenderObject.layout 中被其祖先清除。
        // 保守地标记所有内容为脏，直到其达到最近的已知重新布局的边界。
        markParentNeedsLayout();
      }
      
      return;
    }
    
    // 如果重绘边界不等于等于当前的 render object
    if (_relayoutBoundary != this) {
      markParentNeedsLayout();
    } else {
      _needsLayout = true;
      
      if (owner != null) {
        owner!._nodesNeedingLayout.add(this);
        owner!.requestVisualUpdate();
      }
    }
  }
```

### Background

&emsp;与在 render object 中写入后急切更新布局信息不同，我们会标记布局信息变脏，从而安排进行视觉更新。作为视觉更新的一部分，rendering pipeline 会更新 render object 的布局信息。

&emsp;该机制批处理布局工作，使多个连续的写入合并在一起，减少了冗余计算。

&emsp;如果 render object 的父级指示在计算布局信息时使用其中一个 render object 子级的大小，则在为子级调用此函数时，还会标记父级需要布局。在这种情况下，由于需要重新计算父级和子级的布局，因此只会通知流水线所有者（pipeline owner）有关父级；当父级重新布局时，它将调用子级的布局方法，因此也会对子级进行重新布局。

&emsp;一旦在 render object 上调用了 markNeedsLayout，那么在 render object 上调用 debugNeedsLayout 将返回 true，直到 pipeline owner 在 render objeect 上调用 layout 函数为止。

### Special cases

&emsp;有些 RenderObject 的子类，特别是 RenderBox，有其他情况下需要通知父节点子节点的状态已经变脏（例如，子节点的 固有尺寸（intrinsic dimensions）和基线几何信息（baseline geometry）发生变化的情况）。这些子类会重写 markNeedsLayout 方法，通常在正常情况下会调用 super.markNeedsLayout()，或者在需要对父节点和子节点都进行布局的情况下调用 markParentNeedsLayout 方法。

&emsp;如果 sizedByParent 已经改变，会调用 markNeedsLayoutForSizedByParentChange 方法，而不是调用 markNeedsLayout 方法。

## markParentNeedsLayout

&emsp;将此 render object 的布局信息标记为脏，并延迟到父级 render object 处理。

&emsp;此函数只应该从 markNeedsLayout 或者 markNeedsLayoutForSizedByParentChange 的子类实现中调用，这些子类引入了更多原因以延迟处理脏布局到父对象。

&emsp;只有在父级 render object 不为 null 时才调用此函数。

```dart
  @protected
  void markParentNeedsLayout() {
    _needsLayout = true;
    
    final RenderObject parent = this.parent!;
    parent.markNeedsLayout();
  }
```

## markNeedsLayoutForSizedByParentChange

&emsp;将此 render object 的布局信息标记为脏（类似于调用 markNeedsLayout），并且另外还处理任何必要的工作来处理 sizedByParent 已更改值的情况。

&emsp;每当 sizedByParent 可能已更改时应调用此方法。

&emsp;只有在 parent 不为 null 时才调用此方法。

```dart
  void markNeedsLayoutForSizedByParentChange() {
    markNeedsLayout();
    markParentNeedsLayout();
  }
```

## `_cleanRelayoutBoundary` & `_propagateRelayoutBoundary`

&emsp;清理和传播重新布局的边界。

```dart
  void _cleanRelayoutBoundary() {
    if (_relayoutBoundary != this) {
      _relayoutBoundary = null;
      visitChildren(_cleanChildRelayoutBoundary);
    }
  }

  void _propagateRelayoutBoundary() {
    if (_relayoutBoundary == this) {
      return;
    }
    
    final RenderObject? parentRelayoutBoundary = parent?._relayoutBoundary;
    
    if (parentRelayoutBoundary != _relayoutBoundary) {
      _relayoutBoundary = parentRelayoutBoundary;
      
      visitChildren(_propagateRelayoutBoundaryToChild);
    }
  }

  static void _cleanChildRelayoutBoundary(RenderObject child) {
    child._cleanRelayoutBoundary();
  }

  static void _propagateRelayoutBoundaryToChild(RenderObject child) {
    child._propagateRelayoutBoundary();
  }
```

## scheduleInitialLayout

&emsp;通过调度首次布局来引导 rendering pipeline。需要将要渲染的对象附加到根 Rener Tree 上才能实现。

&emsp;可以查看 RenderView 来了解如何使用该函数的示例。

```dart
  void scheduleInitialLayout() {
    _relayoutBoundary = this;
    owner!._nodesNeedingLayout.add(this);
  }
```

## `_layoutWithoutResize`

&emsp;

```dart
  @pragma('vm:notify-debugger-on-exception')
  void _layoutWithoutResize() {
    try {
      performLayout();
      markNeedsSemanticsUpdate();
    } catch (e, stack) {
      _reportException('performLayout', e, stack);
    }
    
    _needsLayout = false;
    markNeedsPaint();
  }
```

## layout

&emsp;计算此 render object 的布局信息。

&emsp;这个方法是父级 RenderObject 请求其子级（child）更新布局信息的主要入口点。父节点传递一个约束对象，告知子节点哪些布局方式是可接受的。子节点必须遵守给定的约束。

&emsp;如果父节点在子节点的布局期间读取信息，父节点必须为 parentUsesSize 传递 true。在这种情况下，每当子节点被标记为需要布局时，父节点也会被标记为需要布局，因为父节点的布局信息取决于子节点的布局信息。如果父节点使用默认值（false）作为 parentUsesSize，则子节点可以更改其布局信息（在给定约束条件下）而不通知父节点。

&emsp;子类不应直接重写 layout 方法。相反，它们应该重写 performResize 和 performLayout。layout 方法将实际工作委托给 performResize 和 performLayout。

&emsp;父节点的 performLayout 方法应无条件调用其所有子节点的 layout。如果子节点不需要执行任何工作来更新其布局信息，那么 layout 方法的责任（在这里实现）是提前返回。

```dart
  @pragma('vm:notify-debugger-on-exception')
  void layout(Constraints constraints, { bool parentUsesSize = false }) {
    final bool isRelayoutBoundary = !parentUsesSize || sizedByParent || constraints.isTight || parent is! RenderObject;
    
    final RenderObject relayoutBoundary = isRelayoutBoundary ? this : parent!._relayoutBoundary!;

    if (!_needsLayout && constraints == _constraints) {
      if (relayoutBoundary != _relayoutBoundary) {
        _relayoutBoundary = relayoutBoundary;
        visitChildren(_propagateRelayoutBoundaryToChild);
      }
      
      return;
    }
    
    _constraints = constraints;
    if (_relayoutBoundary != null && relayoutBoundary != _relayoutBoundary) {
      // The local relayout boundary has changed, must notify children in case
      // they also need updating. Otherwise, they will be confused about what
      // their actual relayout boundary is later.
      visitChildren(_cleanChildRelayoutBoundary);
    }
    
    _relayoutBoundary = relayoutBoundary;
    
    if (sizedByParent) {
      try {
        performResize();
      } catch (e, stack) {
        _reportException('performResize', e, stack);
      }
    }
    
    try {
      performLayout();
      markNeedsSemanticsUpdate();
    } catch (e, stack) {
      _reportException('performLayout', e, stack);
    }
    
    _needsLayout = false;
    markNeedsPaint();
  }
```

## sizedByParent

&emsp;constraints 是尺寸算法的唯一输入（特别是子节点不会影响）。

&emsp;始终返回 false 是正确的，但是当计算此 render object 的大小时，如果 constraints 不发生变化，则返回 true 可能更有效率。

&emsp;通常，子类会始终返回相同的值。如果值可以改变，那么当它发生改变时，子类应确保调用 markNeedsLayoutForSizedByParentChange。

&emsp;返回 true 的子类不得在 performLayout 中更改此 render object 的尺寸。相反，这项工作应该由 performResize 完成，或者对于 RenderBox 的子类来说，在 RenderBox.computeDryLayout 中完成。

```dart
  @protected
  bool get sizedByParent => false;
```

## performResize 

&emsp;使用仅 constraints 更新 render object 的大小。

&emsp;请不要直接调用此函数，应调用 layout 函数。只有在布局过程中需要此 render object 执行实际工作时，layout 函数才会调用此函数。你父级提供的布局约束可以通过 constraints getter 获得。

&emsp;仅当 sizedByParent 为 true 时才会调用此函数。

&emsp;设置 sizedByParent 为 true 的子类应该重写此方法来计算它们的大小。RenderBox 的子类应该考虑重写 RenderBox.computeDryLayout 方法。

```dart
  @protected
  void performResize();
```

## performLayout

&emsp;执行此 render object 的布局计算工作。

&emsp;请勿直接调用此函数：请调用 layout。当此 render object 在布局期间需要执行实际工作时，会由 layout 调用此函数。你的父级提供的布局约束可通过 constraints getter 获取。

&emsp;如果 sizedByParent 为 true，则此函数不应实际更改此 render object 的尺寸。相反，该工作应由 performResize 完成。如果 sizedByParent 为 false，则此函数应同时更改此 render object 的尺寸并指示其子级进行布局。

&emsp;在实现此函数时，你必须对每个子级调用 layout，如果你的布局信息依赖于子级的布局信息，则设置 parentUsesSize 为 true。将 parentUsesSize 设置为 true 可确保如果子级进行布局，则此 render object 也将执行布局。否则，子级可以更改其布局信息而不通知此 render object。

```dart
  @protected
  void performLayout();
```

## invokeLayoutCallback

&emsp;允许对此对象的子列表（和任何后代）以及由与此对象拥有相同 PipelineOwner 的渲染树中的任何其他脏节点进行更改。回调参数会同步调用，并且只允许在回调的执行期间进行突变。

&emsp;这个功能允许在布局期间按需构建子列表（例如，基于对象的大小），并在发生这种情况时移动节点以便能够处理 GlobalKey 的重新父级。同时仍然确保每个特定节点每帧只布局一次。

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






## 参考链接
**参考链接:🔗**
+ [RenderObject class](https://api.flutter.dev/flutter/rendering/RenderObject-class.html)
