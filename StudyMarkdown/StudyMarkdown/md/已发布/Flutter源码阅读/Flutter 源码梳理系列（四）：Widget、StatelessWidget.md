# Flutter 源码梳理系列（四）：Widget、StatelessWidget

&emsp;记住这句听了无数次的：Widget 用于描述 Element 的配置，后序我们会一路通过代码来验证这句话。

&emsp;`Widgets are the central class hierarchy in the Flutter framework.` -- 在 Flutter framework 中，Widget 是一个核心的类，它是构建 Flutter 应用程序界面的基本构建块，几乎所有的东西都是 Widget，从简单的文本到复杂的布局，甚至整个应用程序本身都是由 Widget 构成的。

&emsp;Widget 是 Flutter APP 的灵魂。

&emsp;一个 Widget 就是参与用户界面构建的一分子，且它是不可改变的。换句话说：Widget 对象一旦创建后就无法改变了。一个 widget 对象可以被 inflate(膨胀) 为 element 对象，后者管理底层的 render tree。

&emsp;Widget 本身没有可变状态（所有字段必须是 final）。如果你想将可变状态与 widget 关联起来，则需要使用 StatefulWidget，它会在被 inflate 为 element 并被纳入 Element 树中时创建一个 State 对象（通过 StatefulWidget.createState）。

&emsp;给定的 widget 对象可以零次或多次包含在 Element tree 中。特别地，给定的 widget 对象可以多次放置在 Element tree 中。每次将 widget 对象放置在树中时，它都会被 inflate 为一个 element 对象，这意味着一个被多次放置在树中的 widget 会被多次 inflate。(这里需要理解 `Element createElement()` 函数。已知 widget 对象和 element 对象是一一对应的，然后假如我们有一个 const 的全局 widget 变量(非使用 `GlobalKey`，如：`Text temp = const Text('123')`)，我们直接把它放在页面的多个地方，那么这个 temp widget 对象在每处都会生成一个不同的 element 对象实例。)

&emsp;key 属性控制一个 widget 如何替换树中的另一个 widget。如果两个 widget 对象的 runtimeType 和 key 属性分别为 operator == 相等，则新 widget 可通过更新 element 对象来替换掉旧的 widget 对象。否则，旧 element 对象可能将会从 element 树中移除，新 widget 对象则被 inflate 为一个新的 element 对象，并将此新 element 对象挂载到 element 树中。

&emsp;另请参见：

+ StatefulWidget 和 State，State 可用于在其生命周期内多次构建不同的 widget。
+ InheritedWidget，用于引入可以被后代 widget 读取的环境状态的 widget。
+ StatelessWidget，用于在特定配置和环境状态下始终以相同方式构建的 widget。

&emsp;看完注释晕晕的，那下面看代码吧，注释里面所有的要义都能用代码来解释：

# Widget 

&emsp;Widget 是所有 Widget 子类的抽象基类。

## @immutable 注解 

```dart
@immutable
abstract class Widget extends DiagnosticableTree { ... }
```

&emsp;Widget 是一个有 @immutable 注解的抽象类，表示 Widget（及其子类，以及 implements 和 with 的类）内的每个字段都必须是 final 修饰的。否则，Dart analyzer 默认将抛出警告，但不会抛出错误，当然也可提高 analyzer error 等级，让它强制报错(`must_be_immutable: error`)。final 关键字表示在属性初始化后便不能再为其赋值（修改），否则，Dart analyzer 将发出错误。从这里也可以印证我们见到无数次的那句：Widget 是不可变的，即 widget 对象自创建出来以后就是不可变的了。

&emsp;关于 DiagnosticableTree 它是用于提供调试信息相关的方法的，我们暂时不去分心去看了，后面抽时间再去看吧。

## key

&emsp;Widget 抽象基类仅有的一个字段 key，用来控制一个 widget 如何替换 element tree 中的另一个 widget。

&emsp;将 GlobalKey 用作 widget 对象的 key，允许该 widget 对应的 element 在 element tree 中移动（改变父级）而不会丢失状态。当发现一个新 widget（其 key 和 runtimeType 与先前位置相同位置的 widget 不匹配），但在先前一帧中树中的其他地方有一个具有相同 global key 的 widget，则该 widget 的 element 将移动到新位置。

&emsp;解释一下这里：首先我们知道 GlobalKey 对象是全局唯一的，然后整棵 element tree 呢共享同一个 BuildOwner 对象实例(全局的，后面再细看)，而在 BuildOwner 对象中呢有一个 map: `final Map<GlobalKey, Element> _globalKeyRegistry = <GlobalKey, Element>{};` 它就是用来保存：每一个全局唯一的 GlobalKey 对象和其对应的 Element 对象的。当对一个 widget 对象进行 inflate 时，判断到这个 widget 对象有 global key 的话，就会去取这个 global key 对应的 element 直接拿来用。

&emsp;通常，作为另一个 widget 的唯一子级的 widget 是不需要显式 key 的。大部分情况下我们是用不到 key 的，当我们有一组外观一样的 widget 并需要对它们排序、交换位置之类的操作时，才会需要用到 key。

```dart
final Key? key;
```

&emsp;下面继续往下看：createElement 抽象函数。

## createElement

&emsp;createElement 是一个抽象函数，那么 Widget 的所有非抽象子类都要自己实现此抽象函数。

&emsp;还看到它有两个注解:

1. @protected 注解表示该函数只能在当前类或子类中被访问，不能在类外被访问。(起到可访问范围控制的作用)
2. @factory 注解表示该函数是一个工厂方法，用于注释一个实例方法或静态方法 m。表示 m 必须要么是抽象的，要么必须返回一个新分配的对象或 null。此外，每一个实现或重写了 m 的方法都会隐式地带有相同的注解。

&emsp;createElement 函数在不同的 Widget 子类中实现时我们再看，@protected 和 @factory 两个注解提示着我们，createElement 只能在 widget 内部访问，不同的 Widget 非抽象子类会有自己的工厂实现。在 framework.dart 中所有的 widget 抽象子类排除 ProxyWidget 外，其它都分别实现了自己的 createElement 函数，分别返回不同的 Element 子类对象。 

```dart
  @protected
  @factory
  Element createElement();
```

&emsp;然后下面是两个与调试相关的方法：`String toStringShort() {...}`、`void debugFillProperties(DiagnosticPropertiesBuilder properties) {...}`。

## @override

&emsp;@override 注解用于标注在子类中重写了父类或接口中的方法（也可以标注字段，例如子类重写了父类中的一个字段）。通过使用 @override 注解，我们可以确保子类中的方法真的是在重写 父类或接口中的方法，如果不加此标注默认会提示警告。使用 @override 是一种很好的做法，因为它可以提高代码可读性，并有助于防止在 Dart 编程中子类化或实现接口时出现意外错误。

1. 如果子类重写父类方法没有加 @override 注解，Analyzer 会提示：`The member 'xxx' overrides an inherited member but isn't annotated with '@override'.`
2. 如果子类中一个方法加了 @override 注解，但是父类或接口中并没有这个方法，Analyzer 则提示：`The method doesn't override an inherited method.Try updating this class to match the superclass, or removing the override annotation.`

&emsp;看到 override 和其它面向对象语言的用法是完全一致，我们直接在大脑里面做知识对比迁移即可。

&emsp;默认情况下，注解只是一个小小的警告信息，如果我们需要强制执行注解的话，可以通过修改项目根目录下的 `analysis_options.yaml` 文件来强制执行它，例如：

```dart
analyzer:
  errors:
    annotate_overrides: error
    # other ....
```

&emsp;那么当我们重写父类方法而不加 @override 注解的话，会直接报错！我们可以根据自己的需要来自行调整注解的严重等级，如：error/warning/info。

## @nonVirtual

&emsp;下面是 operator == 和 int get hashCode，它们来自 Object 基类，在 Dart 中，每个类都隐式继承自 Object 基类。Object 基类定义了基本方法，例如：`toString()` 、`hashCode()`、 `==` 和 `runtimeType`，它们是适用于所有的 Dart 对象的。在 Dart OOP 中，理解这个默认超类至关重要。

&emsp;@nonVirtual 注解用于标注类 C 或 mixin M 中的实例成员(method、getter、setter、operator, or field) m。表示 m 不应在任何继承或 mixin C 或 M 的类中被重写。当我们强制重写时 Analyzer 会提示：`The member 'xxx' is declared non-virtual in 'CCC' and can't be overridden in subclasses. `

&emsp;在这里直接标注 Widget 类，即表示所有的 Widget 子类都直接使用 Object 基类的 hashCode 和 == 操作符，不要再进行重写。这里有一个关于不能重写 == 的讨论，有兴趣的话也可以读读：[Document why Widget.operator== is marked as non-virtual](https://github.com/flutter/flutter/issues/49490)[mark widget == and hashCode as nonVirtual](https://github.com/flutter/flutter/pull/46900)

```dart
  @override
  @nonVirtual
  bool operator ==(Object other) => super == other;

  @override
  @nonVirtual
  int get hashCode => super.hashCode;
```

&emsp;这里可以先按默认的其它语言的常识来理解：`operator ==`(判等)、`int get hashCode`(获取一个对象的哈希值)。在 iOS 原生中可看到对象哈希值是用对象的十六进制地址为参数调用哈希函数算出来的，在 iOS 中，在一个对象的生命周期中对象的地址是不会发生变化的，不像 Dart 的 GC 机制，进行 标记-整理 时，对象的地址是会发生变化的，那以在 Dart 中以什么作为计算一个对象的哈希值的依据呢？

## canUpdate

&emsp;canUpdate 是 Widget 类的一个静态函数，完全不牵涉任何继承和重写，全局就这一个。那么理解起来就比较简单了，而且它的实现也很简单，主要是它的用途场景，我们需要好好理解一下。记得初学 Flutter 时还以为这个函数是用来判断 Widget 是不是能更新呢？后来看发现每个 Widget 对象基本可以理解为都是一个常量，根本不存在更新一说，简直贻笑大方。

&emsp;canUpdate 函数用来判断入参 newWidget 对象能否用来更新当前以入参 oldWidget 对象作为其配置的 element 对象。一个将给定 widget 对象用作其配置的 element 对象可以被更新为使用另一个 widget 对象作为其配置，只有当这两个 widget 对象的 runtimeType 和 key 属性是 operator == 相等时才可以。如果这两个 widget 对象都没有 key（它们的 key 为 null），那么它们将被视为匹配，即使它们的子 widget 完全不同。例如我们有一个 Row widget 对象实例，那么不管它的 children 内部是什么样的，Row widget 对象实例返回的 runtimeType 总是：Row。（当 canUpdate 返回 true 时，只是当前这个 element 对象得到了复用，接下来会对此 element 进行重建工作，它的子级 element 对象根据子级 widget 情况可能会进行：更新/移除/新建 操作。）

&emsp;所以 canUpdate 是用来判断是否能能用新 widget 对象替换 element 的旧 widget 的（element 对象都是会直接引用 inflated 它们的 widget 对象的）。而且所有的 Widget 以及其子类都是用的这一个静态函数来判断的，而且它们只比较 runtimeType 和 key，比较条件还是比较宽松的，这样可以尽可能最大程度的复用当前这个 element 节点。然后如果以面向对象的角度看 Widget 这个抽象基类的话，它仅有一个 key 属性，然后它的一众子类中属性也是较少的，可以看出 widget 对象们还都是比较轻量化的，这样也利于 build 函数中频繁的创建它们以及 Dart GC 可以快速的回收它们占用的内存空间。

```dart
  static bool canUpdate(Widget oldWidget, Widget newWidget) {
    return oldWidget.runtimeType == newWidget.runtimeType
        && oldWidget.key == newWidget.key;
  }
```

## `_debugConcreteSubtype`

&emsp;`_debugConcreteSubtype` 是 Widget 抽象类的最后一个私有静态函数，duebug 用。`_debugConcreteSubtype` 返回特定 widget 对象具体子类型的数字编码。这在 Element.updateChild 中使用，用于确定热重载是否修改了已挂载 element 的超类(即：element 节点是否由 StatelessElement 换成了 StatefulElement，反之亦然)。每个 Widget 的编码必须与 `Element._debugConcreteSubtype` 中对应的 Element 编码匹配。内部极其简单，如果 widget 是 StatefulWidget 子类则返回 1，如果是 StatelessWidget 子类则返回 2，其它情况返回 0。它会与 `Element._debugConcreteSubtype` 配合使用，如果 element 是 StatefulElement 子类则返回 1，如果是 StatelessElement 子类则返回 2，其它情况返回 0。即：StatelessWidget 对应 StatelessElement，StatefulWidget 对应 StatefulElement。

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

## Widget 总结

&emsp;至此 Widget 这个抽象基类的源码就看完了，总结一下：

1. Widget 是一个 @immutable 注解的抽象基类，预示着 Widget 及其所有子类的所有字段都必须是 final 修饰的，至此奠定了所有 Widget 对象都是不可变的基础。
2. 每个 widget 对象都有一个可 null 的 key 字段，可以用来控制现有 element tree 上 widget 对应的 element 是否可以被复用，注意这个仅代表这一个 element 对象是否可以被复用，至于此 element 的子级 element 是否可以被复用，就得继续看子级 widget 的情况了。此 element 对象被复用后，会继续进行它的重建工作。
3. Widget 有一个抽象工厂函数 Element createElement()，预示着所有的 Wiget 非抽象子类需要自己实现这个 createElement 函数，这也预示了，所有的 element 对象都是有一个对应的 widget 实例的。
4. Widget 有一个静态函数 canUpdate 用来判断，如果新旧 widget 的 runtimeType 和 key 相等，那么就可以直接用新 widget 更新现有的 element 对象，而不用创建新的，然后当使用新的 widget 对象更新 element 对象完毕后，旧的 widget 就可以进行 Dart GC 回收了♻️。

&emsp;下面进入 StatelessWidget，它直接继承自 Widget 的抽象类。

# StatelessWidget

&emsp;StatelessWidget 一个不需要可变状态的 widget。

&emsp;Stateless widget 通过构建其他更具体描述用户界面的 widget 集合来描述用户界面的一部分。构建过程会递归地继续，直到用户界面的描述完全具体化（例如，完全由 RenderObjectWidget 组成，它们描述具体的 RenderObject）。这个递归就是指我们经常看到的构建 Element tree 和 RenderObject tree 的过程，Flutter 会根据我们编码好的 widget 组合进行递归构建。 

&emsp;简单来说，Stateless widget 是一个不需要可变状态的 widget，在 Flutter 中用于构建用户界面。它通过构建一系列描述用户界面的其他 widget 来描述用户界面的一部分。这个构建过程会递归继续，直到用户界面的描述完全具体化，即全部由描述具体渲染对象的 RenderObjectWidget 构成。

&emsp;如果你的用户界面描述不依赖于除了自身配置信息和所在上下文之外的任何其他内容，那么 Stateless widget 就非常有用。如果你的 widget 可能会动态改变，比如内部有一个时钟驱动的状态，或依赖于某些系统状态，那么你应该考虑使用有状态组件：StatefulWidget。

## Performance considerations

&emsp;StatelessWidget 的 build 方法通常只在三种情况下被调用，当每次被调用时都会返回一个新的 widget 对象：

1. 当 widget 第一次插入到树中时（可以理解为由 widget 对象构建对应的 element 对象并挂载到 element tree 中）。
2. 当 widget 的父级更改其配置时（查看 Element.rebuild，父级改变时会对子级 widget 进行重建工作）。
3. 以及当它所依赖的 InheritedWidget 发生变化时。

&emsp;如果一个 widget 的父级经常会改变 widget 的配置，或者它依赖的 inherited widget 经常会变化，那么优化 build 方法的性能非常重要，以保持流畅的渲染性能。

&emsp;有几种技巧可以用来减小重建 stateless widget 的影响：

1. 尽量减少通过 build 方法和其创建的任何 widget 进行的传递性节点数量。例如，不要使用复杂的 Row、Column、Padding 和 SizedBox 来将单个子级 widget 以特别精致的方式定位，考虑只使用 Align 或 CustomSingleChildLayout。不要通过多个 Container 和 Decoration 的复杂分层来绘制恰到好处的图形效果，可以考虑使用单个 CustomPaint widget。(一句话总结：尽量减少 widget 嵌套的层级，因为一层 widget 就会被构建为一层 element，widget 层级深的话，对应的 element tree 也会很深)

2. 尽可能使用 const widget（系统提供的 Widget 子类），当自己自定义 Widget 的子类时，尽量给它提供 const 构造函数，以便该 widget 的用户可以使用 const 构造函数创建 widget 对象。（一句话总结：使用 const widget。这个是 Flutter framework 级别的优化，当使用 const widget 时，新旧 widget 是同一个（不再执行 element 的重建或新建过程，必要的话仅仅更新一下当前 element 的 slot 即可），可以保证调用 element 对象的 updateChild 函数时，child.widget == newWidget 相等，无需进行 element 的重建行为。）[Better Performance with const Widgets in Flutter](https://medium.com/@Ruben.Aster/better-performance-with-const-widgets-in-flutter-50d60d9fe482)

&emsp;还有这篇，超级详细介绍了 const widget 相关内容，一定要阅读：[Unlocking Efficiency: When and Why to Use const with Constant Constructors in Flutter](https://www.dhiwise.com/post/why-flutter-prefer-const-with-constant-constructor)。

&emsp;深入理解 element 的更新机制的话，会深深感受到 const 的妙处。如下的示例代码，如果 EmbedChildWidget 添加 const 的话，那么在 ChildWidget 执行 build 的时候，EmbedChildWidget（以及对应的 element）对象不必进行重建，如果我们去掉 const 的话，每次 ChildWidget build 的时候，EmbedChildWidget 对象也会跟着执行 build，这就造成性能浪费了！

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

3. 考虑将频繁重建的 stateless widget 重构为 stateful widget，这样它就可以使用 StatefulWidget 中描述的一些优化技术，比如缓存子树的常见部分以及在更改树结构时使用 GlobalKey。（一句话总结：使用 stateful widget 进行优化。）

4. 如果由于使用 InheritedWidgets 而导致 widget 可能经常重建，考虑将 stateless widget 重构为多个 widget，将其中易发生变化的部分推送到叶子节点。例如：不要构建一个包含四个 widget 的树，内部 widget 依赖于 Theme，考虑将构建内部 widget 的 build 函数的部分提取到自定义的 widget 子类中，这样只有内部 widget 在 Theme 更改时需要重建。（一句话总结：将依赖于 InheritedWidget 导致需要重建的 widget 提取出来，封装为一个新 widget 子类。）

5. 当尝试创建可重用的 UI widget 时，最好使用 widget 子类而不是辅助函数。比如，如果使用一个辅助函数来构建一个 widget 对象，当调用 State.setState 方法时，Flutter 需要重新构建返回的包裹 widget。但如果使用 Widget 子类，Flutter 将能够有效地重新渲染只有那些真正需要更新的部分。更好的是，如果创建的 widget 对象是 const 类型，Flutter 将跳过大部分重建工作。（一句话总结：创建可重用的 UI 时，封装一个新 widget 子类使用，而不是直接使用一个辅助函数返回 widget 对象，类似👆。）

&emsp;[Widgets vs helper methods | Decoding Flutter](https://www.youtube.com/watch?v=IOyq-eTRhvo) 此视频更多地解释了为什么 const 构造函数很重要，以及为什么抽取一个 Widget 子类比使用辅助方法返回 widget 对象更好，强烈建议观看一下。

&emsp;下面是一个名为 GreenFrog 的 stateless widget 的结构。通常，正常的 widget 子类会有更多的构造函数参数，每个参数对应一个 final 属性。

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

&emsp;OK，下面我们继续看 StatelessWidget 的源码：

&emsp;没什么特别的，依然是一个我们不能直接使用的抽象类，然后它直接继承自 Widget，默认传入父类的 key 实现了自己的 const 构造函数。

&emsp;那么它是怎么实现 Widget 类的 createElement 抽象函数的呢，下面继续往下看。

```dart
abstract class StatelessWidget extends Widget {
  /// Initializes [key] for subclasses.
  const StatelessWidget({ super.key });
  ...
}
```

## createElement

&emsp;@override 注解表示了 createElement 是重写了父类函数，它创建一个 StatelessElement 来管理该 widget 在 element tree 中的位置。至此，createElement 抽象函数就变为非抽象函数了，那么 StatelessWidget 子类就不必实现自己的 createElement 函数啦。

&emsp;StatelessWidget 的子类通常不会重写此方法，如果需要子类重写的话就不会把它定义为非抽象函数了。

&emsp;后面我们再深入学习 StatelessElement，看到这里，可以验证一句：Widget 和 Element 是一一对应的。

```dart
  @override
  StatelessElement createElement() => StatelessElement(this);
```

&emsp;下面我们看 StatelessWidget 抽象类最重要的抽象函数：build，我们自己定义的 StatelessWidget 子类，重写了无数次的 build 函数。

## build

&emsp;build 描述了此 widget 所代表的用户界面部分。即我们在这里组织我们的 UI 页面。

&emsp;当此 widget(带指此 widget 对象 inflate 的 element 对象) 被挂载到给定的 element tree 中时，Flutter framework 会调用此方法，以及当此 widget 引用的 InheritedWidget 发生变化时（例如，此 widget 引用的 InheritedWidget 发生变化）。该方法可能在每一帧中被调用，所以除了在其函数内部构建 widget 对象 之外不应具有任何副作用。（build 函数调用超级频繁，我们只在内部组织构建 widget 对象返回即可，不要再执行其它附加操作。）

&emsp;Flutter framework 将此 widget 下方的子树替换为此方法返回的 widget 子树，具体取决于此方法返回的 widget 对象是否可以更新现有子树的根节点，这取决于调用 Widget.canUpdate 时的结果。替换可以通过更新现有子树或者删除子树并 inflated 新子树来完成。

&emsp;通常情况下，build 函数实现会返回一个新创建的 widget 集合，这些 widget 对象根据此 widget 的 build 函数和给定的 BuildContext 中的信息进行配置。例如平时开发中，当我们继承 StatelessWidget 定义一个自己的 Widget 时，例如上面示例代码中的 Frog widget，我们可以自己给它们添加一些 final 字段，例如颜色、字符串文本等，然后在 build 函数内部，我们就可以使用这些颜色、字符串文本，以及它的 context 参数传递来的数据来构建 widget 了。（context 是由当前 widget 的父级传递下来的。）

&emsp;给定的 BuildContext context 参数包含关于在构建此 widget 的树中的位置的信息。例如，context 参数提供了此树位置的 inherited widgets 集合。如果 widget 在树中移动，或者如果 widget 同时插入到树中的多个位置，那么给定 widget 可能会多次使用多个不同的 BuildContext 参数进行构建。

&emsp;此方法的实现必须仅依赖于：widget 的字段，widget 的字段本身不能随时间变化，任何通过使用 BuildContext.dependOnInheritedWidgetOfExactType 从上下文获取的环境状态也不能变化。

&emsp;如果一个 widget 的 build 方法需要依赖于其他内容，请改用 StatefulWidget。

```dart
  @protected
  Widget build(BuildContext context);
```

&emsp;以面向对象的思想来看 build 函数，它的每次被调用，就是创建一个新的 widget 对象，那么是不是就代表之前的旧的 widget 对象要进 Dart GC 回收了呢？鉴于 widget 的频繁创建和销毁，flutter 正是从 GC 层面进行优化的，提高 widget 的创建和销毁性能。

## StatelessWidget 总结

&emsp;StatelessWidget 抽象类内部的内容很少，一共就俩函数，但是极其重要！关于它的非抽象函数 createElement 直接返回一个 StatelessElement 对象，我们可以先有一个印象：首先 StatelessWidget 是一个抽象类，但是 createElement 函数并没有定义为抽象函数，而是给它定义为了一个非抽象函数，直接返回一个 StatelessElement 实例，这也预示了 StatelessWidget 子类不必重写 createElement 函数，使用 StatelessWidget 类的即可。后面我们学习 Element 时再详细看 StatelessElement。

&emsp;最最重要的 build 函数，首先它是一个抽象函数，需要 StatelessWidget 的子类自己实现，然后它有一个 @protected 注解，也预示了它只能在内部使用。在我们自己的继承自 StatelessWidget 的自定义 StatelessWidget 子类中已经重写过无数次 build 函数了，应该是比较熟悉的。

&emsp;那么关键的来了，在 StatelessWidget 的注释中官方给了我们两个比较重要的关注点：

1. build 函数的调用时机（每次 build 函数的调用，都代表进行了 widget 重建）。
2. 优化 StatelessWidget 重建的性能。

&emsp;三种情况下 StatelessWidget 的 build 会被调用：

1. 由 widget 构建的对应的 element 对象挂载到 element tree 上后，会直接进行子级 widget 构建。（仅一次）
2. 当 widget 的父级进行重建时，会对所有子 widget 进行递归重建。（多次）
3. 当 InheritedWidget 发生变化时，所有依赖的它的 widget 会进行重建。（多次）

&emsp;优化 StatelessWidget 重建的 5 个技巧：

1. 尽量减少 build 函数内 widget 的嵌套层级。
2. 为自定义 StatelessWidget 子类提供一个 const 构造函数。（当使用 const 构造函数时，在编译期即可确定此变量是 const 的，可进行更多的优化。）
3. 必要时使用 StatefulWidget 进行替换，使用 StatefulWidget 的优化技巧。
4. 将依赖于 InheritedWidget 的子级 widget 提取出来，减少对旁边无关 widget 的影响。
5. 创建可重用的 UI 时，封装一个新 widget 子类使用，而不是直接使用一个辅助函数返回 widget 对象。（例如，当我们的 widget 嵌套的过多时，我们可能会定义个辅助函数返回 widget 对象，不要这样，直接封装为一个 widget 子类使用。例如把 ListView 的 item 提取出来，封装为一个 widget 子类使用。）

&emsp;好了，StatelessWidget 看到这里，初看 Flutter 的声明式 UI 时，一层一层 Widget 叠在一起挺复杂的，不要被它迷惑，我们还是完全可以用面向对象的角度去理解他们，然后再融会贯通它的组合大于继承的 Dart 语言技巧。

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
