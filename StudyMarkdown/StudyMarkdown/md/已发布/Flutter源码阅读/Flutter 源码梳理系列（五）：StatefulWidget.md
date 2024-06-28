# StatefulWidget

&emsp;首先我们要先有一个思想大纲：不管我们用 widget 构建的 UI 界面再怎么变化，无状态的 widget：`StatelessWidget` 还是有状态的 widget：`StatefulWidget`，它们 wiget 本身都是不可变的，直接理解为这个 `widget` 实例对象是内存中一个 `const` 变量即可，然后每次 `build` 函数执行时，都会完全新建一个 `widget` 实例对象（除去 flutter framework 优化的 const Widget，它们前后都是同一个 widget，执行 == 时，得到的是 true。）！当旧的 `widget` 实例对象不再有用时（UI 更新完毕后），那么旧的 `widget` 实例对象就可以被 GC 回收啦♻️！

&emsp;`StatefulWidget` 是一个具有可变状态(State)的 widget。(对应于 `StatelessWidget` 是一个不需要可变状态的 widget。)

&emsp;State 是一种信息，它可以在构建 widget 时同步读取，且可能在 widget 的生命周期内发生变化。widget 实现者有责任确保在状态更改时迅速通知 State，使用 `State.setState`。

&emsp;有状态 widget 描述了用户界面的一部分，通过构建一系列描述用户界面更具体的其他 widget 来实现。构建过程递归地继续，直到用户界面的描述完全具体化（例如，完全由描述具体渲染对象的 RenderObjectWidgets 组成）。

&emsp;有状态 widget 在描述的用户界面部分可以动态更改时非常有用，例如由于具有内部时钟驱动的状态或依赖于某些系统状态。对于仅取决于(widget)对象本身中的配置信息和 widget inflated 时的 `BuildContext` 的组合的情况，请考虑使用 StatelessWidget。（即非 state 情况还是用无状态 widget。）

&emsp;`StatefulWidget` 实例本身是不可变的，并将其可变状态存储在由 `createState` 方法创建的单独的 `State` 对象中，或者存储在 `State` 订阅的对象中，例如 `Stream` 或 `ChangeNotifier` 对象，这些对象的引用存储在 `StatefulWidget` 本身的 final 字段中。

&emsp;当 framework inflate `StatefulWidget` 时，会调用 `createState` 方法，这意味着如果同一个 widget 插入到树的多个位置，则可能会与多个 `State` 对象相关联。同样，如果一个 `StatefulWidget` 从树中移除，然后再次插入到树中，framework 将再次调用 `createState` 来创建一个新的 `State` 对象，简化 `State` 对象的生命周期。

&emsp;如果创建 `StatefulWidget` 的时候使用了 `GlobalKey` 作为其 `key`，当 `StatefulWidget` 从树的一个位置移动到另一个位置时，会保持相同的 `State` 对象。因为具有 `GlobalKey` 的 `widget` 只能在树中的一个位置使用，使用 `GlobalKey` 的 widget 最多只有一个关联 element。当将具有 `GlobalKey` 的 widget 从树中的一个位置移动到另一个位置时，framework 利用了这个特性，通过将与该 widget 关联的（唯一的）子树从旧位置移动到新位置（而不是在新位置重新创建子树）。与 `StatefulWidget` 相关的 `State` 对象会随着子树的其余部分一起移动，这意味着 `State` 对象在新位置中被重用（而不是重新创建）。但是，为了符合移动的条件，widget 必须在从旧位置删除的同时在同一个动画帧中插入到新位置。

## Performance considerations（性能方面的考虑）

&emsp;`StatefulWidget` 有两个主要的类别。

&emsp;第一个类别是在 `State.initState` 中分配资源并在 `State.dispose` 中释放资源的 Widget，但是它们不依赖于 InheritedWidgets 或调用 `State.setState`。这种 Widget 通常用于应用程序或页面的根部，通过 ChangeNotifiers、Streams 或其他类似对象与子 Widget 通信。遵循这种模式的 Stateful Widgets 相对较为廉价（从 CPU 和 GPU 循环来看），因为它们只构建一次然后不会更新。因此，它们可以拥有相对复杂和深层次的构建方法。

&emsp;第二类别是那些使用 `State.setState` 或依赖于 InheritedWidgets 的 Widgets。这些 Widget 在应用程序的生命周期中通常会被多次重建，因此重建此类 Widget 的影响最小化非常重要。（它们还可能使用 `State.initState` 或 `State.didChangeDependencies` 并分配资源，但重建是重要部分）。

&emsp;有几种技术可以用来最大程度地减轻重新构建 stateful widget 的影响：:

1. 将状态推到叶子节点。例如，如果你的页面有一个时钟，而不是将状态放在页面顶部并在时钟滴答声时重新构建整个页面，可以创建一个专用时钟 widget，只更新自身。（一句话总结：与 state 相关的子 widget 推到叶子节点或者提取出来，减少重新构建影响的范围。）

2. 尽量减少 build 方法及其创建的任何 widget 在传递上创建的节点数。理想情况下，有状态 widget 只会创建一个 widget，且该 widget 应为 RenderObjectWidget。（显然，这并非始终切实可行，但 widget 越接近这个理想，效率就会更高。）（一句话总结：尽量减少 widget 嵌套层级）

3. 如果子树不更改，可以缓存表示该子树的 widget，并在每次可以重复使用时重复使用。为此，将一个 widget 分配给一个 final 状态变量，并在 build 方法中重复使用。widget 被重复使用要比创建一个新的（但配置完全相同）widget 效率更高。另一种缓存策略是将 widget 的可变部分提取为 StatefulWidget，该 StatefulWidget 接受一个 child 参数。（一句话总结：使用 final widget 实例变量。）

4. 尽可能使用 const widget。（这相当于缓存 widget 并重复使用它，Flutter framework 对 const widget 进行优化。）

5. 避免更改创建的子树的深度或更改子树中任何 widget 的类型。例如，而不是返回 widget 或在 IgnorePointer 中包装 widget，应始终在 IgnorePointer 中包装 widget 并控制 `IgnorePointer.ignoring` 属性。这是因为更改子树的深度需要重新构建、布局和绘制整个子树，而仅更改属性将使 render tree 变化最小（例如，在 IgnorePointer 的情况下，根本不需要布局或重绘）。(一句话总结：尽量保证新旧 widget 调用 canUpdate 时返回 true。)

6. 如果由于某种原因必须更改深度，请考虑将子树的共同部分包装在具有在有状态 widget 的生命周期内保持一致的 `GlobalKey` 的 widget 中。如果没有其他 widget 可以方便地分配 key，则 `KeyedSubtree` widget 可能对此有用。（一句话总结：必要时可使用 `GlobalKey` 也可以优化）

7. 在尝试创建可重用的 UI 片段时，应优先使用 widget 而不是辅助函数。例如，如果使用一个辅助函数来构建一个 widget，那么调用 `State.setState` 将需要 Flutter 完全重新构建返回的包装 widget。如果使用 widget，Flutter 将能够高效地仅重新渲染真正需要更新的部分。更好的是，如果已创建的 widget 是 const，Flutter 将能够绕过大部分的重新构建工作。(一句话总结：同 stateless widget)

&emsp;可以看到 stateful widget 优化 build 的方法和 stateless widget 的基础做法是一样的，多出来的部分则是鉴于 stateful widget 重新构建的更加频繁而使用的优化技巧。 

&emsp;这是一个名为 YellowBird 的有状态 widget 子类的结构。

&emsp;在这个例子中，State 没有实际的状态。State 通常表示为私有成员字段。通常 widget 具有更多的构造函数参数，每个参数对应一个 final 属性。

```dart
class YellowBird extends StatefulWidget {
  const YellowBird({ super.key });

  @override
  State<YellowBird> createState() => _YellowBirdState();
}

class _YellowBirdState extends State<YellowBird> {
  @override
  Widget build(BuildContext context) {
    return Container(color: const Color(0xFFFFE306));
  }
}
```

&emsp;这个示例展示了一个更通用的 widget Bird，它可以接收一个颜色和一个子 widget，并且具有一些内部状态，其中有一个可以调用以改变状态的方法：

```dart
class Bird extends StatefulWidget {
  const Bird({
    super.key,
    this.color = const Color(0xFFFFE306),
    this.child,
  });

  final Color color;
  final Widget? child;

  @override
  State<Bird> createState() => _BirdState();
}

class _BirdState extends State<Bird> {
  double _size = 1.0;

  void grow() {
    setState(() { _size += 0.1; });
  }

  @override
  Widget build(BuildContext context) {
    return Container(
      color: widget.color,
      transform: Matrix4.diagonal3Values(_size, _size, 1.0),
      child: widget.child,
    );
  }
}
```

&emsp;按照惯例，widget 构造函数只使用命名参数。同样按照惯例，第一个参数是 key，而最后一个参数是 child、children 或相应的参数。

&emsp;ok，下面我们继续看 StatefulWidget 的源码：

&emsp;没什么特别的，依然是一个我们不能直接使用的抽象类，然后它直接继承自 `Widget`，默认传入父类的 `key` 实现了自己的 const 构造函数。

&emsp;那么它是怎么实现 `Widget` 中的 `createElement` 抽象函数的呢，下面继续往下看。

```dart
abstract class StatefulWidget extends Widget {
  /// Initializes [key] for subclasses.
  const StatefulWidget({ super.key });
  // ...
}
```

## createElement

&emsp;`@override` 注解表示了 `createElement` 是重写了父类函数。它创建一个 `StatefulElement` 来管理该 widget 在 element tree 中的位置。

&emsp;`StatefulWidget` 的子类通常不会重写此方法，如果需要子类重写的话就不会把它定义为非抽象函数了。

&emsp;后面我们再深入学习 `StatefulElement`，看到这里，可以验证一句：Widget 和 Element 是一一对应的。

```dart
  @override
  StatefulElement createElement() => StatefulElement(this);
```

&emsp;下面我们看 `StatefulWidget` 抽象类最重要的抽象函数：`createState`，我们重写了无数次的 `createState` 函数。

### createState

&emsp;`@protected` 和 `@factory` 两个注解，表示不同的 `StatefulWidget` 子类都要实现这个抽象工厂方法，不同的 `StatefulWidget` 子类创建不同的 `State` 子类。

&emsp;在树中的给定位置为该 widget 创建可变状态。子类应重写此方法，以返回其关联的 `State` 子类的新创建实例：(State 是一个范型抽象类，后面我们会学习。)

```dart
@override
State<SomeWidget> createState() => _SomeWidgetState();
```

&emsp;在 StatefulWidget 的生命周期中，framework 可以多次调用这个方法。例如，如果 widget 被插入到树中的多个位置，framework 将为每个位置创建一个单独的 `State` 对象。同样地，如果 widget 从树中移除，然后再次插入到树中，framework 会再次调用 `createState` 方法来创建一个新的 `State` 对象，简化 `State` 对象的生命周期。

```dart
  @protected
  @factory
  State createState();
```

&emsp;看到这里我们可以先对 widget 有一个小总结：

&emsp;特殊情况：

1. 针对那种全局的 const widget 常量，有 GlobalKey 的话，它只能与一对 element 和 state 对应，它不能在树中同时出现多次。

2. 针对那种全局的 const widget 常量，无 GlobalKey 的话，它能与多个独立的 element 和 state 分别对应，即一对多，它可以在树中同时出现多次。

&emsp;普通情况（一般我们很少把 widget 提取出来，作为一个全局常量）：

1. widget 和 element、state 是一一对应的，当新 widget 能直接更新旧 widget 的话， 会直接更新 element 和 state 所引用的 widget，element 和 state 对象都直接得到复用。

2. 上面注释提到 framework 会再次调用 `createState` 方法来创建一个新的 `State` 对象，这种情况还没遇到过，遇到了再看...

&emsp;至此，StatefulWidget 的源码看完了，我们总结一下。

## 总结 StatefulWidget

&emsp;StatefulWidget 抽象类内部的内容很少，一共就俩函数，但是极其重要！关于它的非抽象函数 `createElement` 直接返回一个 `StatefulElement` 对象，我们可以先有一个印象：首先 `StatefulWidget` 是一个抽象类，但是 `createElement` 函数并没有定义为抽象函数，而是给它定义为了一个非抽象函数，直接返回一个 `StatefulElement` 实例，这也预示了 `StatefulWidget` 子类不要重写 `createElement` 函数，使用 `StatefulWidget` 的即可。后面我们学习 element 时再详细看 `StatefulElement`。

&emsp;然后它最重要的抽象工厂函数 `createState` 交给它的子类来实现。

&emsp;StatefulWidget 的注释中官方依据 State 不同的接收数据的方式给它分了两个类别（1. 通过 ChangeNotifiers、Streams 或其他类似对象传递数据。2. 依赖于 InheritedWidgets 传递数据，这类重建比较频繁，要着重优化。），同时又针对 StatefulWidget 频繁重建的行为给了一些优化建议，总主旨依然是如何把重建影响范围缩到最小，建议我们日常都以这些主旨做开发。

&emsp;好了，本篇先到这里，详细看了：StatefulWidget，那么我们下篇继续。

## 参考链接
**参考链接:🔗**
+ [widgets library - Widget class](https://api.flutter.dev/flutter/widgets/Widget-class.html)
+ [immutable top-level constant](https://api.flutter.dev/flutter/meta/immutable-constant.html)
+ [Better Performance with const Widgets in Flutter](https://medium.com/@Ruben.Aster/better-performance-with-const-widgets-in-flutter-50d60d9fe482)
+ [Immutable in Dart and Flutter: Understanding, Usage, and Best Practices](https://medium.com/@yetesfadev/immutable-in-dart-and-flutter-understanding-usage-and-best-practices-742be5fa25ea)
+ [Protected keyword in Dart](https://medium.com/@nijatnamazzade/protected-keyword-in-dart-b8b8ef024c89)
+ [How to Use Override Annotation and the super Keyword in Dart.](https://blog.devgenius.io/how-to-use-override-annotation-and-the-super-keyword-in-dart-9f9d9df326bb)
+ [What is the purpose of @override in Flutter?](https://tekzy.net/blog/what-is-the-purpose-of-override-in-flutter/)
+ [Dart OOP: Method Overriding, Field Overriding, Super Keyword, Super Constructor, and Object Class (Dart OOP Part 5)](https://medium.com/@wafiqmuhaz/dart-oop-method-overriding-field-overriding-super-keyword-super-constructor-and-object-class-fab8bfbfc902)
+ [Dart Generics: Generic Classes, Generic Functions, and Bounded Type Parameters (Dart Generics Part 1)](https://medium.com/@wafiqmuhaz/dart-generics-generic-classes-generic-functions-and-bounded-type-parameters-dart-generics-part-a75099a193c5)
+ [annotate_overrides](https://dart.dev/tools/linter-rules/annotate_overrides)
+ [Unlocking Efficiency: When and Why to Use const with Constant Constructors in Flutter](https://www.dhiwise.com/post/why-flutter-prefer-const-with-constant-constructor)
+ [Why use const in Flutter Dart?](https://medium.com/@calvin.kamardi/why-use-const-in-flutter-dart-34f3496baaf9)
