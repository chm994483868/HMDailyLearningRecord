# Flutter 源码梳理系列（十九）：RenderObjectElement

# RenderObjectElement

&emsp;RenderObjectElement：使用 RenderObjectWidget 作为配置的 Element。

&emsp;RenderObjectElement 对象在渲染树中有一个关联的 RenderObject widget，负责处理诸如布局、绘制和 hit testing 等具体操作。

&emsp;和 ComponentElement 进行对比。有关 Element 生命周期的详细信息，请参阅 Element 中的讨论。

## Writing a RenderObjectElement subclass

&emsp;大多数 RenderObject 使用的三种常见的子模型有：

+ 没有子项的 Leaf Render Object：LeafRenderObjectElement 类处理这种情况。
+ 一个子项：SingleChildRenderObjectElement 类处理这种情况。
+ 一系列子项的链表：MultiChildRenderObjectElement 类处理这种情况。

&emsp;然而，有时候 Render Object 的子模型更加复杂。也许它有一个二维数组的子节点。也许它按需构建子节点。也许它包含多个列表。在这种情况下，配置了该 RenderObject 的 Widget 对应的 Element 将是 RenderObjectElement 的一个新子类。

&emsp;这样一个子类负责管理子节点，特别是该对象的 Element 子节点，以及对应 RenderObject 的 RenderObject 子节点。

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

&emsp;每个子 Element 对应一个 RenderObject，应将其附加到该 RenderObjectElement 的渲染对象（render object）作为 child RenderObject。

&emsp;然而，Element 的直接 child Element 可能并非最终产生其对应的实际 RenderObject 的 Element。例如，StatelessElement（StatelessWidget 的元素）对应于其子元素的 RenderObject（由其 StatelessWidget.build 方法返回的元素）。

&emsp;因此，每个子 Element 都会被分配一个 slot token。这是一个仅在该 RenderObjectElement 节点中私有的标识符。当最终生成 RenderObject 的子代准备将其附加到该节点的渲染对象（render object）时，它会将该 slot token 传回给该节点，从而使该节点能够廉价地识别应将子节点渲染对象放置在父渲染对象中的哪个位置，相对于其他子节点。

&emsp;当父 Element 调用 updateChild 来展开子 Element（请参见下一节）时，会确定子 Element 的 slot。可以通过调用 updateSlotForChild 来更新子 Element 的 slot。

### Updating children

&emsp;在 Element 的生命周期早期，Flutter framework 会调用 mount 方法。此方法应该为每个子 Element 调用 updateChild，传入该子 Element 的 Widget 和 slot，从而获取一个子 Element 的列表。

&emsp;随后，Flutter framework 会调用 update 方法。在这个方法中，RenderObjectElement 应该为每个子 Element 调用 updateChild，传入在 mount 期间获取的 Element，或者上次运行 update 时获取的 Element（以最近发生的为准），新的 Widget 以及 slot。这将为对象提供一个新的 Element 对象列表。

&emsp;在可能的情况下，update 方法应该尝试将上一次通行的元素映射到新通行的 Widget 上。例如，如果上一次通行的元素中有一个是通过特定 Key 配置的，而这次新通行中的一个 Widget 也有相同的 key，它们应该被配对，并且旧 Element 应该使用新 Widget 进行更新（并且还应该更新为新 Widget 新位置对应的占位符）。在这方面，updateChildren 方法可能是有用的。(关于这个 the last pass 不知道是什么意思，但是关于 updateChildren 的详细分析可以看之前的文章，有超详细的分析。)

&emsp;updateChild 应该按照它们的逻辑顺序为子 Element 调用。顺序可能很重要；例如，如果两个子元素在它们的构建方法中都使用 PageStorage 的 writeState 功能（而且都没有 Widget.key），那么第一个子元素写入的状态将被第二个子元素覆盖。

#### Dynamically determining the children during the build phase

&emsp;子组件不一定必须直接从该 Element 的 Widget 中获取。它们可以通过回调动态生成，或以其他更有创意的方式生成。

#### Dynamically determining the children during layout

&emsp;如果 Widget 在布局时间生成，那么在挂载和更新方法中生成它们是行不通的：在那时，该元素的渲染对象的布局尚未开始。相反，更新方法可以将渲染对象标记为需要布局（参见 RenderObject.markNeedsLayout），然后渲染对象的 RenderObject.performLayout 方法可以回调到元素，让它生成 Widget 并相应地调用 updateChild。

&emsp;在布局过程中，若渲染对象需要调用一个元素，则必须使用 RenderObject.invokeLayoutCallback。若一个元素在其更新方法之外需要调用 updateChild，则必须使用 BuildOwner.buildScope。

&emsp;在正常操作过程中，框架会提供比在布局期间进行构建时更多的检查。因此，使用具有布局时构建语义的 Widget 应该谨慎进行。

#### Handling errors when building

&emsp;如果一个元素调用一个构建器函数来获取它的子元素小部件，可能会发现构建过程抛出异常。这样的异常应该被捕获并使用 FlutterError.reportError 进行报告。如果在这种情况下需要一个子元素但构建器失败了，那么可以使用 ErrorWidget 的实例代替。

### Detaching children

&emsp;在使用 GlobalKeys 时，有可能会出现一个子元素在此元素更新之前被另一个元素主动移除的情况。（具体来说，当以特定 GlobalKey 为根的子树从这个元素移动到在构建阶段较早处理的元素时会发生这种情况。）当发生这种情况时，将调用该元素的 forgetChild 方法，该方法会带有对受影响子元素的引用。

### Maintaining the render object tree

&emsp;一旦一个后代生成了一个渲染对象，它将会调用 insertRenderObjectChild 方法。如果后代的插槽（slot）发生更改，它将会调用 moveRenderObjectChild 方法。如果一个后代消失了，它将会调用 removeRenderObjectChild 方法。

&emsp;这三种方法应该相应地更新渲染树，分别从该元素自己的渲染对象中附加、移动和分离给定子渲染对象。

### Walking the children

&emsp;如果一个 RenderObjectElement 对象有任何子元素 Elements，它必须在实现 visitChildren 方法时将它们公开出来。这个方法被许多框架内部机制使用，所以应该快速。它也被测试框架和 debugDumpApp 使用。

+ Object -> DiagnosticableTree -> Element -> RenderObjectElement

&emsp;RenderObjectElement 直接继承自 Element。

## Constructors

&emsp;创建一个 RenderObjectElement，使用给定的 RenderObjectWidget 作为其配置。

```dart
abstract class RenderObjectElement extends Element {
  RenderObjectElement(RenderObjectWidget super.widget);
  
  // ...
}
```

## renderObject

&emsp;该 RenderObjectElement 的底层 RenderObject。如果该 RenderObjectElement 已被卸载，这个 getter 会抛出异常。

```dart
  @override
  RenderObject get renderObject {
    return _renderObject!;
  }
  
  RenderObject? _renderObject;
```

## renderObjectAttachingChild

&emsp;返回该 Element 的子元素，该子元素将向该 Element 的祖先插入一个 RenderObject 以构建渲染树。

&emsp;如果该 Element 没有任何子元素需要将 RenderObject 附加到该 Element 的祖先中，则返回 null。因此，RenderObjectElement 将返回 null，因为其子元素将其 RenderObjects 插入到 RenderObjectElement 本身而不是 RenderObjectElement 的祖先。

&emsp;此外，这可能对那些提升自己独立渲染树并且不扩展祖先渲染树的 Element 返回 null。

```dart
  @override
  Element? get renderObjectAttachingChild => null;
```

## `_ancestorRenderObjectElement`

&emsp;

```dart
  RenderObjectElement? _ancestorRenderObjectElement;
```

## `_findAncestorRenderObjectElement`

&emsp;沿着 parent 指针往上找，直到找到第一个 RenderObjectElement 为止，并把其返回。

```dart
  RenderObjectElement? _findAncestorRenderObjectElement() {
    Element? ancestor = _parent;
    
    while (ancestor != null && ancestor is! RenderObjectElement) {
      // 在调试模式中，我们会检查祖先是否接受 RenderObjects 以便在 attachRenderObject 中生成更好的错误消息。
      // 在发布模式中，我们假设只构建正确的树（即 debugExpectsRenderObjectForSlot 总是返回 true ），不会进行显式检查。
      ancestor = ancestor?._parent;
    }
    
    return ancestor as RenderObjectElement?;
  }
```

## `_findAncestorParentDataElements`

&emsp;

```dart
  List<ParentDataElement<ParentData>> _findAncestorParentDataElements() {
    Element? ancestor = _parent;
    
    final List<ParentDataElement<ParentData>> result = <ParentDataElement<ParentData>>[];
    final Set<Type> debugAncestorTypes = <Type>{};
    final Set<Type> debugParentDataTypes = <Type>{};
    final List<Type> debugAncestorCulprits = <Type>[];

    // More than one ParentDataWidget can contribute ParentData, but there are
    // some constraints.
    // 1. ParentData can only be written by unique ParentDataWidget types.
    //    For example, two KeepAlive ParentDataWidgets trying to write to the
    //    same child is not allowed.
    // 2. Each contributing ParentDataWidget must contribute to a unique
    //    ParentData type, less ParentData be overwritten.
    //    For example, there cannot be two ParentDataWidgets that both write
    //    ParentData of type KeepAliveParentDataMixin, if the first check was
    //    subverted by a subclassing of the KeepAlive ParentDataWidget.
    // 3. The ParentData itself must be compatible with all ParentDataWidgets
    //    writing to it.
    //    For example, TwoDimensionalViewportParentData uses the
    //    KeepAliveParentDataMixin, so it could be compatible with both
    //    KeepAlive, and another ParentDataWidget with ParentData type
    //    TwoDimensionalViewportParentData or a subclass thereof.
    // The first and second cases are verified here. The third is verified in
    // debugIsValidRenderObject.

    while (ancestor != null && ancestor is! RenderObjectElement) {
      if (ancestor is ParentDataElement<ParentData>) {
        assert((ParentDataElement<ParentData> ancestor) {
          if (!debugAncestorTypes.add(ancestor.runtimeType) || !debugParentDataTypes.add(ancestor.debugParentDataType)) {
            debugAncestorCulprits.add(ancestor.runtimeType);
          }
          return true;
        }(ancestor));
        result.add(ancestor);
      }
      ancestor = ancestor._parent;
    }
    assert(() {
      if (result.isEmpty || ancestor == null) {
        return true;
      }
      // Validate points 1 and 2 from above.
      _debugCheckCompetingAncestors(
        result,
        debugAncestorTypes,
        debugParentDataTypes,
        debugAncestorCulprits,
      );
      return true;
    }());
    return result;
  }
```










## 参考链接
**参考链接:🔗**
+ [ComponentElement class](https://api.flutter.dev/flutter/widgets/ComponentElement-class.html)
+ [ProxyElement class](https://api.flutter.dev/flutter/widgets/ProxyElement-class.html)
