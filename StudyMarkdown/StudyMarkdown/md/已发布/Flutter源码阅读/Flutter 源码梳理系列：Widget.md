# Widget/StatelessWidget/StatefulWidget

&emsp;Flutter 进阶！静下心来把源码和注释读一遍吧，彻底掌握 Flutter 开发！如果你静不下来心，那么去试试 Flutter 岗位的面试吧，解释不出来 Flutter 原理，甚至说不出来相关的类名，那么只能 “回去等通知了！”。 

&emsp;Widget 用于描述 Element 的配置。

&emsp;`Widgets are the central class hierarchy in the Flutter framework.` -- 在 Flutter framework 中，Widget 是一个核心的类层级结构，也就是说 Widgets 是构建 Flutter 应用程序界面的基本构建块。其实，几乎所有的东西都是 Widget，从简单的文本到复杂的布局，甚至整个应用程序本身都是由 Widget 构成的。Widget 是 Flutter framework 的灵魂。

&emsp;`A widget is an immutable description of part of a user interface.` -- 一个 widget 是用户界面的一部分，且它是不可改变的描述。换句话说，widget 是用户界面的一个组件（的构成描述），一旦创建后就无法改变了。Widgets 可以被 inflated 为 elements，后者管理底层的 render tree。

&emsp;Widgets 本身没有可变状态（所有字段必须是 final）。如果你想将可变状态与 widget 关联起来，则需要使用 StatefulWidget，它会在被 inflated 为 element 并被纳入树中时创建一个 State 对象（通过 StatefulWidget.createState）。

&emsp;给定的 widget 可以零次或多次包含在树中。特别地，给定的 widget 可以多次放置在树中。每次将 widget 放置在树中时，它都会被填充为一个 Element，这意味着一个被多次放置在树中的 widget 会被多次填充。(这里需要理解 `Element createElement();` 函数。已知 Widget 和 Element 是一一对应的，然后假如我们有一个 const 的全局 Widget(非使用 `GlobalKey`)(如：`Text temp = const Text('123');`)，我们直接把它放在页面的多个地方，那么这个 temp Widget 在每处都会生成一个不同的 Element 实例。)

&emsp;key 属性控制一个 widget 如何替换树中的另一个 widget。如果两个 widget 的 runtimeType 和 key 属性分别为 operator ==，则新 widget 通过更新底层 element（即通过调用 Element.update 与新 widget）来替换旧 widget。否则，旧 element 将从树中移除，新 widget 将被 inflated 为一个新 element，并将新 element 插入到树中。

&emsp;另请参见：StatefulWidget 和 State，用于可以在其生命周期内多次不同构建的 widgets。InheritedWidget，用于引入可以被后代 widgets 读取的环境状态的 widgets。StatelessWidget，用于在特定配置和环境状态下始终以相同方式构建的 widgets。

&emsp;看完注释晕晕的，那下面看代码吧，注释里面所有的要义其实都来自代码：

## Widget 

&emsp;所有 widget 的抽象基类。

### @immutable 注解 

```dart
@immutable
abstract class Widget extends DiagnosticableTree { ... }
```

&emsp;`Widget` 是一个有 `@immutable` 注解的抽象类，表示 `Widget`（及其子类，以及 `implements` 和 `with` 的类）内的每个字段都必须是 `final`。否则，Dart analyzer 默认将抛出警告，但不会抛出错误，也可提高 analyzer error 等级，让它强制报错(`must_be_immutable: error`)。`final` 关键字表示在属性初始化后不能再为其赋值（修改），否则，Dart analyzer 将发出错误。从这里也可以印证我们见到无数次的那句：`Widgets 是不可变的`。

&emsp;关于 `DiagnosticableTree` 它是用于提供调试信息相关的方法，我们暂时不去分心去看了，后面抽时间再去看吧。

### key

&emsp;`Widget` 抽象基类仅有的一个属性 `key`，用来控制一个 widget 如何替换树中的另一个 widget。

&emsp;如果两个 widget 的 `runtimeType` 和 `key` 属性分别是 `operator ==`，则新 widget 通过更新底层 element（即通过使用新 widget 做参数调用 `Element.update` 函数）来替换旧 widget。否则，旧 element 将从树中移除，新 widget 将被 inflated 到一个新 element 中，并将新 element 插入到树中。

&emsp;此外，将 `GlobalKey` 用作 widget 的 `key` 允许该 widget 对应的 element 在树中移动（改变父级）而不会丢失状态。当发现一个新 widget（其 `key` 和 `runtimeType` 与先前位置相同位置的 widget 不匹配），但在先前一帧中树中的其他地方有一个具有相同 global key 的 widget，则该 widget 的 element 将移动到新位置。

&emsp;解释一下这里：首先我们知道 `GlobalKey` 是全局唯一的，然后整棵 element 树呢共享同一个 `BuildOwner` 实例(全局的，后面再细看)，而在 `BuildOwner` 中呢有一个 map: `final Map<GlobalKey, Element> _globalKeyRegistry = <GlobalKey, Element>{};` 它呢就是用来保存：每一个全局唯一的 `GlobalKey` 和其对应的 `Element` 的。当对一个 widget 进行 inflate 时，判断到这个 widget 有 global key 的话，就会去取这个 global key 对应的 element 直接拿来用。

&emsp;通常，作为另一个 widget 的唯一子级的 widget 是不需要显式 key 的。（当讲 key 时，我们再细看，什么开发场景下才需要用 key。）

```dart
final Key? key;
```

&emsp;下面继续往下看：`createElement` 抽象函数。

### createElement

&emsp;一眼就能看出 `createElement` 是一个抽象函数，那么 `Widget` 的所有子类都要各自自己实现它。

&emsp;还看到它有两个注解:

1. `@protected` 注解表示该函数只能在当前类或子类中被访问，不能在类外被访问。(起到可访问范围控制的作用)
2. `@factory` 注解表示该函数是一个工厂方法，用于注释一个实例方法或静态方法 m。表示 m 必须要么是抽象的，要么必须返回一个新分配的对象或 null。此外，每一个实现或重写了 m 的方法都会隐式地带有相同的注解。

&emsp;`createElement` 函数在不同的 `Widget` 子类中我们再细看，`@protected` 和 `@factory` 两个注解也提示着我们，`createElement` 只能在 widget 内部访问，不同的 `Widget` 子类会有自己的工厂实现。在 framework.dart 中所有的 `widget` 抽象子类排除 `ProxyWidget` 外，其它都分别实现了自己的 `createElement` 函数，分别返回不同的 `Element` 子类。 

```dart
  @protected
  @factory
  Element createElement();
```

&emsp;然后下面是两个与调试相关的方法：`String toStringShort() {...}`、`void debugFillProperties(DiagnosticPropertiesBuilder properties) {...}`。

### @override

&emsp;`@override` 注解用于标注在子类中重写了 父类或接口 中的方法（字段也可以，例如子类重写了父类中的一个字段。）。通过使用 `@override` 注解，我们可以确保子类中的方法真的是在重写 父类 或 接口 中的方法，如果不是会编译报错。使用 `@override` 是一种很好的做法，因为它可以提高代码可读性，并有助于防止在 Flutter 中子类化或实现接口时出现意外错误。

1. 如果子类重写父类方法没有加 `@override` 注解 Analyzer 会提示：`The member 'xxx' overrides an inherited member but isn't annotated with '@override'.`
2. 如果子类中一个方法加了 `@override` 注解，但是 父类或接口 中并没有这个方法 Analyzer 则提示：`The method doesn't override an inherited method.Try updating this class to match the superclass, or removing the override annotation.`

&emsp;看到 override 和其它面向对象语言的用法是完全一致，我们直接在大脑里面做知识对比迁移即可。

&emsp;默认情况下，注解只是一个小小的警告信息，如果我们需要强制执行注解的话，可以通过修改项目根目录下的 `analysis_options.yaml` 文件来强制执行它，例如：

```dart
analyzer:
  errors:
    annotate_overrides: error
    # other ....
```

&emsp;那么当我们重写父类方法而不加 `@override` 注解的话，会直接报错！我们可以根据自己的需要来自行调整注解的严重等级，如：error/warning/info。

### @nonVirtual

&emsp;下面是 `operator ==` 和 `int get hashCode`，它们来自 `Object`，在 Dart 中，每个类都隐式继承自 `Object` 类。`Object` 类定义了基本方法，例如：`toString()` 、`hashCode()`、 `==` 和 `runtimeType`，它们是适用于所有 Dart 对象。在 Dart OOP 中，理解这个默认超类至关重要。

&emsp;`@nonVirtual` 注解用于标注类 C 或 mixin M 中的实例成员(method、getter、setter、operator, or field) m。表示 m 不应在任何继承或 mixin C 或 M 的类中被重写。当我们强制重写时 Analyzer 会提示：`The member 'xxx' is declared non-virtual in 'CCC' and can't be overridden in subclasses. `

&emsp;在这里直接标注 `Widget`，即表示所有的 `Widget` 子类都直接使用 `Object` 的 `hashCode` 和 `==` 操作符，不要再进行重写。这里有一个关于不能重写 `==` 的讨论，有兴趣的话也可以读读：[Document why Widget.operator== is marked as non-virtual](https://github.com/flutter/flutter/issues/49490)[mark widget == and hashCode as nonVirtual](https://github.com/flutter/flutter/pull/46900)

```dart
  @override
  @nonVirtual
  bool operator ==(Object other) => super == other;

  @override
  @nonVirtual
  int get hashCode => super.hashCode;
```

&emsp;这里可以先按默认的其它语言的常识来理解：`operator ==`(判等)、`int get hashCode`(一个对象的哈希值)。在 iOS 原生中可看到对象哈希值是用对象的十六进制地址为参数调用哈希函数算出来的，在 iOS 中，在一个对象的生命周期中对象的地址是不会发生变化的，不像 Dart 的 GC 机制，进行 标记-整理 时，对象的地址是会发生变化的，那以什么作为计算一个对象的哈希值的依据呢？后面学习 `Dart Object` 时再进行深入研究。

### canUpdate

&emsp;`canUpdate` 是 `Widget` 类的一个静态函数，完全不牵涉任何继承和重写，全局就这一个。那么理解起来就比较简单了，而且它的实现也很简单，主要是它的用途场景，我们需要好好理解一下。记得初学 Flutter 时还以为这个函数是用来判断 `Widget` 是不是能更新呢？简直贻笑大方。

&emsp;`canUpdate` 函数用来判断 `Widget newWidget` 能否用来更新当前以 `Widget oldWidget` 作为其配置的 `Element`。一个将给定 widget 用作其配置的 element 可以被更新为使用另一个 widget 作为其配置，只有当这两个 widget 的 `runtimeType` 和 `key` 属性是 `operator ==` 时才可以。如果这两个 widget 没有 `key`（它们的 `key` 为 `null`），那么它们将被视为匹配，即使它们的子组件完全不同。例如我们有一个 `Row` widget 实例，那么不管它的 `children` 内部是什么样的，`Row` widget 实例返回的 `runtimeType` 总是：`Row`。

&emsp;所以 `canUpdate` 是用来判断是否能用新 widget 替换 element 的旧 widget 的（element 都是会直接引用 inflated 它们时的 widget 的。）。而且所有的 Widget 以及其子类都是用的这一个静态函数来判断的，而且它们只比较 `runtimeType` 和 `key`，比较条件还是比较宽松的，这样可以尽可能最大程度的复用 element 的。然后如果以面向对象的角度看 `Widget` 这个抽象基类的话，它仅有一个 `key` 属性，然后它的一众子类中属性也是较少的，可以看出它们都是比较轻量化的，这样也利于 `build` 函数中频繁的创建它们以及 Dart GC 可以快速的回收它们占用的内存空间。

```dart
  static bool canUpdate(Widget oldWidget, Widget newWidget) {
    return oldWidget.runtimeType == newWidget.runtimeType
        && oldWidget.key == newWidget.key;
  }
```

### `_debugConcreteSubtype`

&emsp;`_debugConcreteSubtype` 是 `Widget` 的最后一个私有静态函数，duebug 用。`_debugConcreteSubtype` 返回特定 `Widget` 具体子类型的数字编码。这在 `Element.updateChild` 中使用，用于确定热重载是否修改了已挂载 element 配置的超类。每个 `Widget` 的编码必须与 `Element._debugConcreteSubtype` 中对应的 `Element` 编码匹配。内部极其简单，如果 widget 是 StatefulWidget 子类则返回 1，如果是 StatelessWidget 子类则返回 2，其它情况返回 0。它会与 `Element._debugConcreteSubtype` 配合使用，如果 element 是 StatefulElement 子类则返回 1，如果是 StatelessElement 子类则返回 2，其它情况返回 0。

```dart
  static int _debugConcreteSubtype(Widget widget) {
    return widget is StatefulWidget ? 1 :
           widget is StatelessWidget ? 2 :
           0;
  }
  
  static int _debugConcreteSubtype(Element element) {
    return element is StatefulElement ? 1 :
           element is StatelessElement ? 2 :
           0;
  }
```

### 总结 Widget

&emsp;至此 `Widget` 这个抽象基类的源码就看完了，总结一下：

1. `Widget` 是一个 `@immutable` 注解的抽象基类，预示着 `Widget` 及其所有子类的所有字段都必须是 `final` 修饰的，至此奠定了所有 Widgets 都是不可变的。
2. 每个 widget 都一个可选的 `key`，可以用来控制现有 element tree 上 widget 对应的 element 是否还可以继续用。
3. `Widget` 有一个抽象工厂函数 `Element createElement();`，预示这所有的 wiget 子类需要自己实现这个 `createElement` 函数，这也预示了，所有的 widget 实例都是与 element 是一一对应的。
4. `Widget` 有一个静态函数 `canUpdate` 用来判断，如果新旧 widget 的 `runtimeType` 和 `key` 相等，那么就可以直接用新 widget 更新现有的 element，而不用创建新的，然后当使用新的 widget 更新 element 完毕后，旧的 widget 就可以进行 Dart GC 回收了♻️。

&emsp;下面进入 `StatelessWidget`，它直接继承自 `Widget` 的抽象类。

## StatelessWidget

&emsp;`StatelessWidget` 一个不需要可变状态的 widget。

&emsp;Stateless widget 通过构建其他更具体描述用户界面的 widget 集合来描述用户界面的一部分。构建过程会递归地继续，直到用户界面的描述完全具体化（例如，完全由 RenderObjectWidgets 组成，它们描述具体的 RenderObject）。这个递归就是指我们经常看到的构建 Element tree 和 RenderObject tree 的过程，Flutter 会根据我们编码好的 widgets 进行递归构建。 

&emsp;简单来说，Stateless widget 是一个不需要可变状态的 widget，在 Flutter 中用于构建用户界面。它通过构建一系列描述用户界面的其他小部件来描述用户界面的一部分。这个构建过程会递归继续，直到用户界面的描述完全具体化，即全部由描述具体渲染对象的 RenderObjectWidgets 构成。

&emsp;如果你的用户界面描述不依赖于除了自身配置信息和所在上下文之外的任何其他内容，那么 Stateless widget 就非常有用。如果你的 widget 可能会动态改变，比如内部有一个时钟驱动的状态，或依赖于某些系统状态，那么你应该考虑使用有状态组件 `StatefulWidget`。

### Performance considerations（性能方面的考虑）

&emsp;`StatelessWidget` 的 `build` 方法通常只在三种情况下被调用：

1. 当 widget 第一次插入到树中时（可以理解为由 widget 构建对应的 element 并插入 element tree 中）。
2. 当 widget 的父级更改其配置时（查看 `Element.rebuild`，父级改变时会对子 widget 进行重建）。
3. 以及当它所依赖的 `InheritedWidget` 发生变化时。

&emsp;如果一个 widget 的父级经常会改变 widget 的配置，或者它依赖的 inherited widget 经常会变化，那么优化 `build` 方法的性能非常重要，以保持流畅的渲染性能。

&emsp;有几种技巧可以用来减小重建 stateless widget 的影响：

1. 尽量减少通过 build 方法和其创建的任何 widget 进行的传递性节点数量。例如，不要使用复杂的 Rows、Columns、Paddings 和 SizedBox 来将单个子项以特别精致的方式定位，考虑只使用 Align 或 CustomSingleChildLayout。不要通过多个 Containers 和 Decorations 的复杂分层来绘制恰到好处的图形效果，可以考虑使用单个 CustomPaint 小部件。(一句话总结：尽量减少 widget 嵌套的层级。)
2. 尽可能使用 const widget，并为该 widget 提供一个 const 构造函数，以便该 widget 的用户也可以这样做。（一句话总结：使用 const widget。这个是 Flutter framework 级别的优化，当使用 const widget 时，新旧 widget 是同一个（且不再执行 widget 的 build 过程），可以保证在 element 的 updateChild 时，child.widget == newWidget 相等，优化更新行为。达到直接复用 widget 的目的。）[Better Performance with const Widgets in Flutter](https://medium.com/@Ruben.Aster/better-performance-with-const-widgets-in-flutter-50d60d9fe482)

&emsp;还有这篇，超级详细介绍了 const widget 相关内容，一定要阅读：[Unlocking Efficiency: When and Why to Use const with Constant Constructors in Flutter](https://www.dhiwise.com/post/why-flutter-prefer-const-with-constant-constructor)。

&emsp;如下的示例代码，如果 EmbedChildWidget 添加 const 的话，那么在 ChildWidget build 的时候，EmbedChildWidget 不必进行重建，如果我们去掉 const 的话，每次 ChildWidget build 的时候，EmbedChildWidget 也会跟着执行 build，这就造成性能浪费了！

```dart
class ChildWidget extends StatelessWidget {
  const ChildWidget({super.key});

  @override
  Widget build(BuildContext context) {
    final data = MyInheritedWidget.of(context).data;
    debugPrint('ChildWidget build 执行！');

    return Column(
      children: [
        Text('Data from InheritedWidget: $data'),
        
        // 注意此处添加了 const
        const EmbedChildWidget(),
      ],
    );
  }
}

class EmbedChildWidget extends StatelessWidget {
  const EmbedChildWidget({super.key});

  @override
  Widget build(BuildContext context) {
    debugPrint('EmbedChildWidget build 执行！');

    return const Text('123');
  }
}
```

3. 考虑将频繁重建的 stateless widget 重构为 stateful widget，这样它就可以使用 StatefulWidget 中描述的一些技术，比如缓存子树的常见部分以及在更改树结构时使用 GlobalKey。（一句话总结：使用 stateful widget 进行优化。）
4. 如果由于使用 InheritedWidgets 而导致 widget 可能经常重建，考虑将 stateless widget 重构为多个 widget，其中会将发生变化的部分推送到叶子节点。例如：不要构建一个包含四个 widget 的树，内部 widget 依赖于 Theme，考虑将构建内部 widget 的 build 函数的部分提取到自己的 widget 中，这样只有内部 widget 在 Theme 更改时需要重建。（一句话总结：将依赖于 InheritedWidgets 导致需要重建的 widget 提取出来，封装为一个新 widget。）
5. 当尝试创建可重用的 UI widget 时，最好使用 widget 而不是辅助函数。比如，如果使用一个辅助函数来构建一个 widget，当调用 State.setState 方法时，Flutter 需要重新构建返回的包裹小部件。但如果使用 Widget，Flutter 将能够有效地重新渲染只有那些真正需要更新的部分。更好的是，如果创建的 widget 是 const 类型，Flutter 将跳过大部分重建工作。（一句话总结：创建可重用的 UI 时，封装一个新 widget 使用，而不是直接使用一个辅助函数返回 widget，类似👆。）

&emsp;[Widgets vs helper methods | Decoding Flutter](https://www.youtube.com/watch?v=IOyq-eTRhvo) 此视频更多地解释了为什么 const 构造函数很重要，以及为什么抽取一个 Widget 比使用辅助方法返回 widget 更好，强烈建议观看一下。

&emsp;下面是一个名为 GreenFrog 的 stateless widget 的结构。通常，正常的 widget 会有更多的构造函数参数，每个参数对应一个 final 属性。

```dart
class GreenFrog extends StatelessWidget {
  const GreenFrog({ super.key });

  @override
  Widget build(BuildContext context) {
    return Container(color: const Color(0xFF2DBD3A));
  }
}
```

&emsp;下一个示例展示了一个更通用的 widget Frog，可以设置 color 和 child。

```dart
class Frog extends StatelessWidget {
  const Frog({
    super.key,
    this.color = const Color(0xFF2DBD3A),
    this.child,
  });

  final Color color;
  final Widget? child;

  @override
  Widget build(BuildContext context) {
    return ColoredBox(color: color, child: child);
  }
}
```

&emsp;按照惯例，widget 构造函数只使用命名参数。同样按照惯例，第一个参数是 key，而最后一个参数是 child、children，或者其它等效选项。

&emsp;ok，下面我们继续看 StatelessWidget 的源码：

&emsp;没什么特别的，依然是一个我们不能直接使用的抽象类，然后它直接继承自 `Widget`，默认传入父类的 `key` 实现了自己的 const 构造函数。

&emsp;那么它是怎么实现 `Widget` 中的 `createElement` 抽象函数的呢，下面继续往下看。

```dart
abstract class StatelessWidget extends Widget {
  /// Initializes [key] for subclasses.
  const StatelessWidget({ super.key });
  ...
}
```

### createElement

&emsp;`@override` 注解表示了 `createElement` 是重写了父类函数。它创建一个 `StatelessElement` 来管理该 widget 在 element tree 中的位置。

&emsp;`StatelessWidget` 的子类通常不会重写此方法，如果需要子类重写的话就不会把它定义为非抽象函数了。

&emsp;后面我们再深入学习 `StatelessElement`，看到这里，可以验证一句：Widget 和 Element 是一一对应的。

```dart
  @override
  StatelessElement createElement() => StatelessElement(this);
```

&emsp;下面我们看 `StatelessWidget` 抽象类最重要的抽象函数：`build`，我们重写了无数次的 `build` 函数。

### build

&emsp;`build` 描述了此 widget 所代表的用户界面部分。即我们在这里组织我们的 UI 页面。

&emsp;当此 widget 被插入到给定的 BuildContext 中的树(即 element tree)中时，Flutter framework 会调用此方法，以及当此 widget 引用的 InheritedWidget 发生变化时（例如，此 widget 引用的 InheritedWidget 发生变化）。该方法可能在每一帧中被调用，并且除了在其函数内部构建 widget 之外不应具有任何副作用。（build 调用超级频繁，我们只在内部组织构建 widget 即可，不要再执行其它操作。）

&emsp;Flutter framework 将此 widget 下方的子树替换为此方法返回的 widget，具体取决于此方法返回的小部件是否可以更新现有子树的根节点，这取决于调用 `Widget.canUpdate` 时的结果。替换可以通过更新现有子树或者删除子树并 inflated 新子树来完成。

&emsp;通常情况下，`build` 函数实现会返回一个新创建的 widget 集合，这些 widget 根据此 widget 的构造函数和给定的 `BuildContext` 中的信息进行配置。例如平时开发中，当我们继承 `StatelessWidget` 定义一个自己的 Widget 时，例如上面示例代码中的 `Frog` widget，我们可以自己给它们添加一些 `final` 字段，例如颜色、字符串文本等，然后在 `build` 函数内部，我们就可以使用这些颜色、字符串文本，以及它的 `context` 参数传递来的数据来构建 widget 了。（context 是由当前 widget 的父级传递下来的。）

&emsp;给定的 `BuildContext context` 参数包含关于在构建此 widget 的树中的位置的信息。例如，`context` 参数提供了此树位置的 inherited widgets 集合。如果 widget 在树中移动，或者如果 widget 同时插入到树中的多个位置，那么给定 widget 可能会多次使用多个不同的 `BuildContext` 参数进行构建。

&emsp;此方法的实现必须仅依赖于：widget 的字段，widget 的字段本身不能随时间变化，任何通过使用 `BuildContext.dependOnInheritedWidgetOfExactType` 从上下文获取的环境状态也不能变化。

&emsp;如果一个 widget 的 build 方法需要依赖于其他内容，请改用 `StatefulWidget`。

```dart
  @protected
  Widget build(BuildContext context);
```

&emsp;以面向对象的思想来看 `build` 函数，它的每次被调用，就是创建一个新的 widget 对象，那么是不是就代表之前的旧的 widget 对象要进 Dart GC 回收了呢？鉴于 widget 的频繁创建和销毁，flutter 正是从 GC 层面进行优化的，提高 widget 的创建和销毁性能。

### 总结 StatelessWidget

&emsp;StatelessWidget 抽象类内部的内容很少，一共就俩函数，但是极其重要！关于它的非抽象函数 `createElement` 直接返回一个 `StatelessElement` 对象，我们可以先有一个印象：首先 `StatelessWidget` 是一个抽象类，但是 `createElement` 函数并没有定义为抽象函数，而是给它定义为了一个非抽象函数，直接返回一个 `StatelessElement` 实例，这也预示了 `StatelessWidget` 子类不要重写 `createElement` 函数，使用 `StatelessWidget` 的即可。后面我们学习 element 时再详细看 `StatelessElement`。

&emsp;最最重要的 `build` 函数，首先它是一个抽象函数，需要 `StatelessWidget` 的子类自己实现，然后它有一个 `@protected` 注解，也预示了它只能在内部使用。在我们自己的继承自 `StatelessWidget` 的 widget 中已经重写过无数次 `build` 函数了，应该是比较熟悉的。

&emsp;那么关键的来了，在 `StatelessWidget` 的注释中官方给了我们两个比较重要的关注点：

1. `build` 函数的调用时机（每次 `build` 函数的调用，都代表进行了 widget 重建）。
2. 优化 `StatelessWidget` 重建的性能。

&emsp;三种情况下 `StatelessWidget` 的 `build` 会被调用：

1. 由 widget 构建的对应的 element 进行初次构建时。（仅一次）
2. 当 widget 的父级进行重建时，会对所有子 widget 进行递归重建。（多次）
3. 当 InheritedWidget 发生变化时，所有依赖的它的 widget 会进行重建。（多次）

&emsp;优化 `StatelessWidget` 重建的 5 个技巧：

1. 尽量减少 `build` 函数内 widget 的嵌套层级。
2. 为该 `StatelessWidget` 提供一个 const 构造函数。（在编译期即可确定一个变量是 const 的，可进行更多的优化。）
3. 必要时使用 `StatefulWidget` 进行替换，使用 `StatefulWidget` 优化技巧。
4. 将依赖于 InheritedWidget 的子 widget 提取出来，减少对旁边无关 widget 的影响。
5. 创建可重用的 UI 时，封装一个新 widget 使用，而不是直接使用一个辅助函数返回 widget。（例如，当我们的 widget 嵌套的过多时，我们可能会创建个函数返回 widget，不要这样，直接封装为一个子 widget 使用。例如把 ListView 的 item 提取出来，封装为一个子 widget 使用。）

&emsp;好了，StatelessWidget 看到这里，我们继续向下：StatefulWidget。

&emsp;初看 Flutter 的声明式 UI 时，一层一层 Widget 叠在一起挺复杂的，不要被它迷惑，我们还是完全可以用面向对象的角度去理解他们，然后再融会贯通它的组合大于继承的 Dart 语言技巧。

## StatefulWidget

&emsp;首先我们要先有一个思想大纲：不管我们用 widget 构建的 UI 界面再怎么变化，无状态的 widget：`StatelessWidget` 还是有状态的 widget：`StatefulWidget`，它们 wiget 本身都是不可变的，直接理解为这个 `widget` 实例对象是内存中一个 `const` 变量即可，然后每次 `build` 函数执行时，都会完全新建一个 `widget` 实例对象（除去 flutter framework 优化的 const Widget，它们前后都是同一个 widget，执行 == 时，得到的是 true。）！当旧的 `widget` 实例对象不再有用时（UI 更新完毕后），那么旧的 `widget` 实例对象就可以被 GC 回收啦♻️！

&emsp;`StatefulWidget` 是一个具有可变状态(State)的 widget。(对应于 `StatelessWidget` 是一个不需要可变状态的 widget。)

&emsp;State 是一种信息，它可以在构建 widget 时同步读取，且可能在 widget 的生命周期内发生变化。widget 实现者有责任确保在状态更改时迅速通知 State，使用 `State.setState`。

&emsp;有状态 widget 描述了用户界面的一部分，通过构建一系列描述用户界面更具体的其他 widget 来实现。构建过程递归地继续，直到用户界面的描述完全具体化（例如，完全由描述具体渲染对象的 RenderObjectWidgets 组成）。

&emsp;有状态 widget 在描述的用户界面部分可以动态更改时非常有用，例如由于具有内部时钟驱动的状态或依赖于某些系统状态。对于仅取决于(widget)对象本身中的配置信息和 widget inflated 时的 `BuildContext` 的组合的情况，请考虑使用 StatelessWidget。（即非 state 情况还是用无状态 widget。）

&emsp;`StatefulWidget` 实例本身是不可变的，并将其可变状态存储在由 `createState` 方法创建的单独的 `State` 对象中，或者存储在 `State` 订阅的对象中，例如 `Stream` 或 `ChangeNotifier` 对象，这些对象的引用存储在 `StatefulWidget` 本身的 final 字段中。

&emsp;当 framework inflate `StatefulWidget` 时，会调用 `createState` 方法，这意味着如果同一个 widget 插入到树的多个位置，则可能会与多个 `State` 对象相关联。同样，如果一个 `StatefulWidget` 从树中移除，然后再次插入到树中，framework 将再次调用 `createState` 来创建一个新的 `State` 对象，简化 `State` 对象的生命周期。

&emsp;如果创建 `StatefulWidget` 的时候使用了 `GlobalKey` 作为其 `key`，当 `StatefulWidget` 从树的一个位置移动到另一个位置时，会保持相同的 `State` 对象。因为具有 `GlobalKey` 的 `widget` 只能在树中的一个位置使用，使用 `GlobalKey` 的 widget 最多只有一个关联 element。当将具有 `GlobalKey` 的 widget 从树中的一个位置移动到另一个位置时，framework 利用了这个特性，通过将与该 widget 关联的（唯一的）子树从旧位置移动到新位置（而不是在新位置重新创建子树）。与 `StatefulWidget` 相关的 `State` 对象会随着子树的其余部分一起移动，这意味着 `State` 对象在新位置中被重用（而不是重新创建）。但是，为了符合移动的条件，widget 必须在从旧位置删除的同时在同一个动画帧中插入到新位置。

### Performance considerations（性能方面的考虑）

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

### createElement

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

### 总结 StatefulWidget

&emsp;StatefulWidget 抽象类内部的内容很少，一共就俩函数，但是极其重要！关于它的非抽象函数 `createElement` 直接返回一个 `StatefulElement` 对象，我们可以先有一个印象：首先 `StatefulWidget` 是一个抽象类，但是 `createElement` 函数并没有定义为抽象函数，而是给它定义为了一个非抽象函数，直接返回一个 `StatefulElement` 实例，这也预示了 `StatefulWidget` 子类不要重写 `createElement` 函数，使用 `StatefulWidget` 的即可。后面我们学习 element 时再详细看 `StatefulElement`。

&emsp;然后它最重要的抽象工厂函数 `createState` 交给它的子类来实现。

&emsp;StatefulWidget 的注释中官方依据 State 不同的接收数据的方式给它分了两个类别（1. 通过 ChangeNotifiers、Streams 或其他类似对象传递数据。2. 依赖于 InheritedWidgets 传递数据，这类重建比较频繁，要着重优化。），同时又针对 StatefulWidget 频繁重建的行为给了一些优化建议，总主旨依然是如何把重建影响范围缩到最小，建议我们日常都以这些主旨做开发。

&emsp;好了，本篇先到这里，详细看了：Widget、StatelessWidget、StatefulWidget，那么我们下篇继续。

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
