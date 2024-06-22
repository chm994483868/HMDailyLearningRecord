# RenderObjectWidget/LeafRenderObjectWidget/SingleChildRenderObjectWidget/MultiChildRenderWidget

&emsp;RenderObjectWidget 为 RenderObjectElement 提供配置，而 RenderObjectElement 包装 RenderObject，后者负责实际渲染应用程序。简单来说，RenderObjectWidget 是用来配置如何渲染应用程序的规则，RenderObject 则是负责实际渲染应用程序内容的工具。就像是 RenderObjectWidget 负责告诉 RenderObjectElement 如何完成渲染，而 RenderObjectElement 则告诉 RenderObject 具体怎么做。

&emsp;通常情况下，如果想要创建一个自定义的渲染对象（RenderObjectWidget），我们不直接继承 RenderObjectWidget，而是选择继承下面这些类之一，（它们都是 RenderObjectWidget 的直接子类。）：

+ LeafRenderObjectWidget: 如果 widget 没有子项，则使用。
+ SingleChildRenderObjectElement: 如果 widget 恰好有一个子项，则使用。
+ MultiChildRenderObjectWidget: 如果 widget 接受子项列表，则使用。
+ SlottedMultiChildRenderObjectWidget: 如果小部件将其子项组织在不同命名槽中，则使用。

&emsp;RenderObjectWidget 子类必须实现：createRenderObject 和 updateRenderObject 函数。

## RenderObjectWidget

&emsp;RenderObjectWidget 一个直接继承自 Widget 的抽象类，常量构造函数，还是那个样子，没什么特别的。

```dart
abstract class RenderObjectWidget extends Widget {
  /// Abstract const constructor. This constructor enables subclasses to provide
  /// const constructors so that they can be used in const expressions.
  // 抽象常量构造函数。此构造函数使子类能够提供常量构造函数，以便它们可以在常量表达式中使用。
  const RenderObjectWidget({ super.key });
  // ...
}
```

### createElement

&emsp;`@factory` 注解，表示不同的 RenderObjectWidget 子类都要实现这个抽象工厂方法，不同的 RenderObjectWidget 子类创建了不同的 RenderObjectElement 子类。

&emsp;RenderObjectWidget 总是会 inflate 为一个 RenderObjectElement 子类。

```dart
  @override
  @factory
  RenderObjectElement createElement();
```

&emsp;到目前为止，我看到的 Widget 子类都实现了自己的，createElement 函数，都分别创建自己对应类型的 Element。

+ Widget => StatelessWidget -> StatelessElement
+ Widget => StatefulWidget -> StatefulElement
+ Widget => ProxyWidget => `ParentDataWidget<T extends ParentData>` -> `ParentDataElement<T>`
+ Widget => ProxyWidget => InheritedWidget -> InheritedElement
+ Widget => RenderObjectWidget => LeafRenderObjectWidget -> LeafRenderObjectElement
+ Widget => RenderObjectWidget => SingleChildRenderObjectWidget -> SingleChildRenderObjectElement
+ Widget => RenderObjectWidget => MultiChildRenderObjectWidget -> MultiChildRenderObjectElement
+ Widget => RenderObjectWidget => SlottedMultiChildRenderObjectWidget -> SlottedMultiChildRenderObjectElement

&emsp;

### createRenderObject

&emsp;一个 RenderObjectWidget 抽象方法，需要 RenderObjectWidget 的各个子类实现。

&emsp;使用此 RenderObjectWidget 描述的配置创建此 RenderObjectWidget 表示的 RenderObject 类的一个实例。

&emsp;此方法不应该对 render object 的子项进行任何操作。相反，应该由重写 RenderObjectElement.mount 方法的方法处理，该方法在被该对象的 createElement 方法渲染的对象中调用。例如，参见 SingleChildRenderObjectElement.mount。

```dart
  @protected
  @factory
  RenderObject createRenderObject(BuildContext context);
```

### updateRenderObject

&emsp;一个 RenderObjectWidget 的非抽象方法，但是实现内容为空，由 RenderObjectWidget 的各个子类选择重写。

&emsp;将此 RenderObjectWidget 描述的配置复制到给定的 RenderObject 中，该 RenderObject 将与此对象的 createRenderObject 方法返回的相同类型。

&emsp;此方法不应对更新 render object 的子项执行任何操作。相反，应该由重写了此对象的 createElement 方法中渲染的对象上的 RenderObjectElement.update 方法处理。例如，请参阅 SingleChildRenderObjectElement.update。

```dart
  @protected
  void updateRenderObject(BuildContext context, covariant RenderObject renderObject) { }
```

### didUnmountRenderObject

&emsp;一个 RenderObjectWidget 的非抽象方法，但是实现内容为空，由 RenderObjectWidget 的各个子类选择重写。

&emsp;此 widget 先前关联的 render object 已从树中移除。给定的 RenderObject 将与此对象的 createRenderObject 返回的相同类型。

```dart
  @protected
  void didUnmountRenderObject(covariant RenderObject renderObject) { }
```

&emsp;到这里，RenderObjectWidget 的内容就看完了，它最突出的特点，就是：createRenderObject 抽象函数，需要它的各子类实现。看到这里可以验证：只有 RenderObjectWidget 才可以创建 RenderObject。其它的一些方法，我们后面再看。

&emsp;然后就是它的三个最主要的直接子类了：LeafRenderObjectWidget、SingleChildRenderObjectWidget、MultiChildRenderObjectWidget，它们三个的内容并不多，我们快速过一下。

## LeafRenderObjectWidget

&emsp;这是一个为没有子节点的 RenderObject 子类配置 RenderObjectWidgets 的超类。子类必须实现 createRenderObject 和 updateRenderObject。

```dart
abstract class LeafRenderObjectWidget extends RenderObjectWidget {
  /// Abstract const constructor. This constructor enables subclasses to provide
  /// const constructors so that they can be used in const expressions.
  // 抽象 const 构造函数。该构造函数使得子类能够提供 const 构造函数，以便在 const 表达式中使用。
  const LeafRenderObjectWidget({ super.key });

  @override
  LeafRenderObjectElement createElement() => LeafRenderObjectElement(this);
}
```

## SingleChildRenderObject

&emsp;一个为 RenderObjectWidgets 超类的类，用于配置只有一个子节点 slot 的 RenderObject 子类。

&emsp;分配给此 widget 的 render object 应该利用 RenderObjectWithChildMixin 实现一个 single-child 模型。该 mixin 公开了 RenderObjectWithChildMixin.child 属性，允许检索属于 widget 的 render object。

&emsp;子类必须实现 createRenderObject 和 updateRenderObject。

```dart
abstract class SingleChildRenderObjectWidget extends RenderObjectWidget {
  /// Abstract const constructor. This constructor enables subclasses to provide
  /// const constructors so that they can be used in const expressions.
  const SingleChildRenderObjectWidget({ super.key, this.child });
  
  // ...

  @override
  SingleChildRenderObjectElement createElement() => SingleChildRenderObjectElement(this);
}
```

### child

&emsp;树中位于此 widget 下方的 widget。

&emsp;此 widget 只能有一个 widget。要布局多个 widget，请让此 widget 的 child widget 是 Row、Column 或 Stack 等 widget，这些 widget 具有 children 属性，然后将 child widget 提供给该 widget。

```dart
  final Widget? child;
```

&emsp;同 LeafRenderObjectWidget 比多了：child 字段。

## MultiChildRenderObjectWidget

&emsp;这是一个为 RenderObject 子类配置 RenderObjectWidget 的超类，这些子类有一个子代列表(children)。 （这个超类只提供了该 children 的存储空间，实际上并没有提供更新逻辑。）

&emsp;子类必须使用混合了 ContainerRenderObjectMixin 的 RenderObject，这个 mixin 提供了访问容器渲染对象的子代（即子代 widget 所属的渲染对象）所需的功能。通常，子类将使用一个既混入 ContainerRenderObjectMixin 又混入 RenderBoxContainerDefaultsMixin 的 RenderBox。

&emsp;子类必须实现 createRenderObject 和 updateRenderObject。

&emsp;参见：

+ Stack，它使用 MultiChildRenderObjectWidget。
+ RenderStack，一个关联 render object 示例实现。
+ SlottedMultiChildRenderObjectWidget，它配置了一个 RenderObject，该 RenderObject 不是使用单个 children 列表，而是将其 child 组织在命名 slots 中。

```dart
abstract class MultiChildRenderObjectWidget extends RenderObjectWidget {
  /// Initializes fields for subclasses.
  const MultiChildRenderObjectWidget({ super.key, this.children = const <Widget>[] });
  
  // ...
  
  @override
  MultiChildRenderObjectElement createElement() => MultiChildRenderObjectElement(this);
}
```

### children

&emsp;树中此 Widget 下方的 widget。

&emsp;如果此列表将被修改，通常明智的做法是在每个子 Widget 上放置一个 Key，这样 framwwork 可以将旧配置匹配到新配置，从而维护底层的 render object。

&emsp;另外，在 Flutter 中，Widget 是不可变的，因此直接修改子项如 someMultiChildRenderObjectWidget.children.add(...) 或下面的示例代码将导致不正确的行为。每当修改 children 时，应提供一个新的列表对象。

&emsp;下面是错误示范代码：

```dart
// This code is incorrect.
class SomeWidgetState extends State<SomeWidget> {
  final List<Widget> _children = <Widget>[];

  void someHandler() {
    setState(() {
      _children.add(const ChildWidget());
    });
  }

  @override
  Widget build(BuildContext context) {
    // Reusing `List<Widget> _children` here is problematic.
    return Row(children: _children);
  }
}
```

&emsp;下面的代码纠正了上面提到的问题。

```dart
class SomeWidgetState extends State<SomeWidget> {
  final List<Widget> _children = <Widget>[];

  void someHandler() {
    setState(() {
      // The key here allows Flutter to reuse the underlying render objects even if the children list is recreated.
      // 这里的 key 是允许 Flutter 在 children 被重新创建时重复使用底层的 render object。(这里根据 key 可以复用 widget 对应的 element 和 render object)
      _children.add(ChildWidget(key: UniqueKey()));
    });
  }

  @override
  Widget build(BuildContext context) {
    // Always create a new list of children as a Widget is immutable.
    // 始终创建一个新的 children，因为 Widget 是不可变的。
    return Row(children: _children.toList());
  }
}
```

&emsp;同上面 SingleChildRenderObjectWidget 比，这里是一个 widget 列表。

```dart
  final List<Widget> children;
```

&emsp;至此 LeafRenderObjectWidget、SingleChildRenderObjectWidget、MultiChildRenderObjectWidget 三个 RenderObjectWidget 的抽象子类 的内容粗略看完了，官方文档里面也没有过多的介绍它们，我们自己看的话主要是基于 RenderObjectWidget 的抽象函数：createRenderObject，也即是再往后面的 RenderObjectWidget 子类必须要实现这个创建 Render object 的任务。看到这里可验证：Widget 和 Element 是一一对应的，不同的 Widget 都会实现自己的 createElement 函数，而只有 RenderObjectWidget 才能创建自己的 render object。

&emsp;看到这里的话，基本的抽象 Widget 基类，我们就看完了，它们的继承链如下：

+ Object => DiagnosticableTree => Widget => StatelessWidget
+ Object => DiagnosticableTree => Widget => StatefulWidget

+ Object => DiagnosticableTree => Widget => ProxyWidget => ParentDataWidget
+ Object => DiagnosticableTree => Widget => ProxyWidget => InheritedWidget

+ Object => DiagnosticableTree => Widget => RenderObjectWidget => LeafRenderObjectWidget
+ Object => DiagnosticableTree => Widget => RenderObjectWidget => SingleChildRenderObjectWidget
+ Object => DiagnosticableTree => Widget => RenderObjectWidget => MultiChildRenderObjectWidget
+ Object => DiagnosticableTree => Widget => RenderObjectWidget => SlottedMultiChildRenderObjectWidget

&emsp;粗略总结一下的话：StatefulWidget 子类需要创建自己的 State，ParentDataWidget 子类需要把 parent data 引用到 Render Object 上，InheritedWidget 子类可以把数据传递到远端，并且在更新时通知依赖也一起更新。其它的 RenderObjectWidget 子类则是各自构建自己的 Render Object。

&emsp;好了，下面我们继续，向 element 和 render object 发起冲锋。

## 参考链接
**参考链接:🔗**
+ [RenderObjectWidget class](https://api.flutter.dev/flutter/widgets/RenderObjectWidget-class.html)
+ [LeafRenderObjectWidget class](https://api.flutter.dev/flutter/widgets/LeafRenderObjectWidget-class.html)
+ [SingleChildRenderObjectWidget class](https://api.flutter.dev/flutter/widgets/SingleChildRenderObjectWidget-class.html)
+ [MultiChildRenderObjectWidget class](https://api.flutter.dev/flutter/widgets/MultiChildRenderObjectWidget-class.html)
