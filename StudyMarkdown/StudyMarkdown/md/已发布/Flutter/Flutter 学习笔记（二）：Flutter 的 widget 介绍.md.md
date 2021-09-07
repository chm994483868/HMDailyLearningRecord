# Flutter 学习笔记（二）：Flutter Widget 框架介绍.md 

## widget 简述 

&emsp;Flutter Widget 采用现代响应式框架构建，这是从 [React](https://reactjs.org) 中获得的灵感，中心思想是用 widget 来构建界面上的 UI 元素。（说实话完全不了解 React，**响应式框架** 对我而言也是新词，对比 iOS 原生开发而言，大概理解为 RAC 在 iOS 中的使用，把数据和页面进行双向绑定。）

&emsp;Widget 描述了它们的视图在给定其当前配置和状态时应该看起来像什么。当 widget 的状态发生变化时，widget 会重新构建 UI，**Flutter 会对比前后变化的不同，以确定底层渲染树从一个状态到下一个状态所需的最小更改（译者语：类似于 React/Vue 中虚拟 Dom 的 diff 算法）。** 这里想到了 iOS 的重绘机制（想到了 `layoutSubviews/setNeedsLayout/layoutIfNeeded/drawRect:/setNeedsDisplay/setNeedsDisplayInRect:` 这些 API）：页面什么时候需要重绘以及重绘时机以及父子 View 是否都需要重绘以及是否仅需 View 的局部区域需要重绘等等，这里 widget 是否可以理解为 iOS 中的 UIView 呢？但是好像不同于 UIView 对象，在 iOS 中进行页面刷新重绘时我们的 UIView 及其子类对象并没有进行重新构建，不知道 **widget 会重新构建 UI** 作何理解，后续我们会对比原生开发继续深入学习。

## widget 示例

&emsp;如下一个极其简单的 Flutter 程序，在屏幕中仅显示一个 "Hello, world!" 文本，这里使用到了两个 widget，其中直接把 Center widget 传递给 `runApp` 函数。

```c++
import 'package:flutter/material.dart';

void main() {
  runApp(
    new Center(
      child: new Text(
        'Hello, world!',
        textDirection: TextDirection.ltr,
      ),
    ),
  );
}
```

### runApp 

&emsp;下面我们对 `runApp` 函数的官方文档进行一个学习：

```c++
void runApp(
        Widget app
)
```

&emsp;使给定的 widget inflate（使充气、使膨胀） 并附加到屏幕上。widget 在布局过程中受到约束，强制它填充整个屏幕。如果你希望将 widget 与屏幕的一侧（例如，顶部）对齐，请考虑使用 [Align](https://api.flutter.dev/flutter/widgets/Align-class.html) widget。如果你希望将 widget 居中，你也可以使用 [Center](https://api.flutter.dev/flutter/widgets/Center-class.html) widget。

&emsp;再次调用 `runApp` 时将会从屏幕上分离先前的 root widget，并将给定的 widget 附加到其位置。新的 widget tree 与先前的 widget tree 进行比较，任何差异都会应用于底层渲染树（underlying render tree），类似于调用 [State.setState](https://api.flutter.dev/flutter/widgets/State/setState.html) 后 [StatefulWidget](https://api.flutter.dev/flutter/widgets/StatefulWidget-class.html) 重建时发生的情况。

&emsp;如有必要，使用 [WidgetsFlutterBinding](https://api.flutter.dev/flutter/widgets/WidgetsFlutterBinding-class.html) 初始化绑定。

&emsp;好了，接下来我们继续分析 `runApp` 函数。

&emsp;该 `runApp` 函数接受给定的 widget 并将其作为 widget tree 的 root（对比原生的话可以理解为 APP 图层树的根节点），在上面实例代码中，widget tree 由两个 widget：`Center` 和 `Text` 组成（`Text` 作为 `Center` 的子 widget）。Flutter 框架会强制 root widget 覆盖整个屏幕，即 `Center` widget 会占满整个屏幕，这也意味着 `Text` widget 会位于屏幕中心，即 "Hello, world!" 文本会居中显示在屏幕上（`textDirection: TextDirection.ltr/rtl,` 是指文本方向是从左到右还是从右到左，且发现不指定此属性时，iOS App 是无法运行的，会报 `No Directionality widget found` 的错误，且我们进行 `ltr/rtl` 切换时，发现热重载无法生效了，只有点击重启，才能切换文本方向）。

&emsp;文本显示的方向需要在 `Text` 实例中指定，当使用 MaterialApp 时，文本的方向将自动设定，稍后将进行演示。

&emsp;在编写应用程序时，通常会创建新的 widget，这些  widget 是无状态的 [StatelessWidget](https://api.flutter.dev/flutter/widgets/StatelessWidget-class.html) 或者是有状态的 [StatefulWidget](https://api.flutter.dev/flutter/widgets/StatefulWidget-class.html)，具体的选择取决于你的 widget 是否需要管理一些状态（状态可以理解为一些需要持有化的数据）。`widget` 的主要工作是实现一个 [build](https://api.flutter.dev/flutter/widgets/StatelessWidget/build.html) 函数，用以构建自身。一个 widget 通常由一些较低级别 widget 组成。Flutter 框架将依次构建这些 widget，直到构建到最底层的子 widget 时，这些最底层的 widget 通常为 [RenderObject](https://api.flutter.dev/flutter/rendering/RenderObject-class.html)（渲染树中的一个对象，这里想到了 iOS 中的模型树和渲染树），它会计算并描述  widget 的几何形状。

### build method

&emsp;下面我们对 `build` 函数进行延展学习：

```c++

@protected
Widget build(
        BuildContext context
)
```

&emsp;描述此 widget 呈现的用户界面部分。当这个 widget 被插入到给定 `BuildContext`（widget 树中 widget 位置的句柄）的树中并且当这个 widget 的依赖关系发生变化（例如，这个小部件引用的 `InheritedWidget`（有效地沿树向下传播信息的 widget 基类）发生变化）时，Flutter 框架调用这个方法。这个方法可能会在每一帧中被调用，并且除了构建一个 widget 之外不应该有任何多余的操作。

&emsp;Fletter 框架使用此方法返回的 widget 替换此 widget 下方的子树，通过更新现有子树或删除子树并 inflating 新子树，取决于此方法返回的 widget 是否可以更新现有子树的 root，由调用 `Widget.canUpdate` 确定。

&emsp;`newWidget` 是否可用于更新当前以 `oldWidget` 作为其 configuration 的 Element。使用给定 widget 作为其 configuration 的 Element 可以更新为使用另一个 widget 作为其 configuration，当且仅当这两个小部件具有 `runtimeType` 和 key 属性为 `operator==` 时。

&emsp;如果 widget 没有 key（它们的 key 为 null），那么如果它们具有相同的类型，即使它们的 children 完全不同，它们也被认为是匹配的。

```c++
bool canUpdate(
        Widget oldWidget,
        Widget newWidget
)
```

&emsp;**Implementation:**

```c++
static bool canUpdate(Widget oldWidget, Widget newWidget) {
  return oldWidget.runtimeType == newWidget.runtimeType && oldWidget.key == newWidget.key;
}
```

&emsp;下面我们继续看 `build` 函数的内容。

&emsp;通常，`build` 实现会返回一个新创建的 constellation of widgets，这些 widgets 使用来自该 widget 的构造函数和给定的 `BuildContext` 的信息进行配置。

&emsp;给定的 `BuildContext` 包含有关构建此 widget 的树中位置的信息。例如，上下文为树中的这个位置提供了一组继承的 widget。如果 widget 在树周围移动，或者 widget 一次插入树中的多个位置，则可能会随着时间的推移使用多个不同的 `BuildContext` 参数构建给定 widget。

&emsp;此方法的实现必须仅依赖于：

+ widget 的字段，它们本身不得随时间变化，以及
+ 使用 `BuildContext.dependOnInheritedWidgetOfExactType` 从上下文获得的任何环境状态。

&emsp;如果 widget 的 `build` 方法依赖于其他任何东西，请改用 `StatefulWidget`。

&emsp;也可以看看，`StatelessWidget` 其中包含对性能考虑的讨论。

&emsp;**Implementation:**

```c++
@protected
Widget build(BuildContext context);
```

## 基础 Widget

&emsp;Flutter 有一套丰富、强大的基础 widget，其中以下是很常用的：

+ [Text](https://api.flutter.dev/flutter/widgets/Text-class.html)：该 widget 可以创建一个带格式的文本。（类似 iOS 中的 UILabel）
+ [Row](https://api.flutter.dev/flutter/widgets/Row-class.html)（在水平阵列中显示其子项的 widget）、[Column](https://api.flutter.dev/flutter/widgets/Column-class.html)（在垂直阵列中显示其子项的 widget）：这些具有弹性空间的布局类 widget 可让你在水平（`Row`）和垂直（`Column`）方向上创建灵活的布局。其设计是基于 web 开发中的 Flexbox 布局模型。
+ [Stack](https://api.flutter.dev/flutter/widgets/Stack-class.html)（相对于其框的边缘定位其子项的 widget）：取代线性布局（和 android 中的 LinearLayout 相似），`Stack` 允许子 widget 堆叠，你可以使用 [Positioned](https://api.flutter.dev/flutter/widgets/Positioned-class.html)（控制 `Stack` 的子项所在位置的 widget）来定位它们相对于 `Stack` 的上下左右四条边的位置。`Stack` 是基于 Web 开发中的绝对定位（absolute positioning）布局模型设计的。
+ [Container](https://api.flutter.dev/flutter/widgets/Container-class.html)（一个方便的 widget，结合了常见的绘画、定位和大小调整 widget）：`Container` 可让你创建矩形视觉元素。`Container` 可以装饰一个 [BoxDecoration](https://api.flutter.dev/flutter/painting/BoxDecoration-class.html)（关于如何绘制 box 的不可变描述），如 background、一个边框、或者一个阴影。`Container` 也可以具有边距（margins）、填充（padding）和应用于其大小的约束（constraints）。另外，`Container` 可以使用矩形在三维空间中对其进行变换。

&emsp;以下是一些简单的 widget，它们可以组合出其它的 widget：

```c++
import 'package:flutter/material.dart';

class MyAppBar extends StatelessWidget {
  MyAppBar({required this.title});

  // Widget 子类中的字段往往都会定义为 "final"

  final Widget title;

  @override
  Widget build(BuildContext context) {
    return new Container(
      height: 88.0, // 单位是逻辑上的像素（并非真实的像素，类似于浏览器中的像素）
      padding: const EdgeInsets.symmetric(horizontal: 8.0),
      decoration: new BoxDecoration(color: Colors.blue[500]),
      // Row 是水平方向的线性布局（linear layout）
      child: new Row(
        // 列表项的类型是 <widget>
        children: <Widget>[
          new IconButton(
            onPressed: null, // null 会禁用 button
            icon: new Icon(Icons.menu),
            tooltip: 'Navigation menu',
          ),
          // Expanded expands its child to fill the available space.(Expanded 扩展其子项以填充可用空间。)
          new Expanded(
            child: title,
          ),
          new IconButton(
            onPressed: null,
            icon: new Icon(Icons.search),
            tooltip: 'Search',
          ),
        ],
      ),
    );
  }
}

class MyScaffold extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    // Material 是 UI 呈现的 “一张纸”
    return new Material(
      // Column is 垂直方向的线性布局。
      child: new Column(
        children: <Widget>[
          new MyAppBar(
            title: new Text(
              'Example title',
              style: Theme.of(context).primaryTextTheme.bodyText1,
            ),
          ),
          new Expanded(
            child: new Center(
              child: new Text('Hello, world!'),
            ),
          ),
        ],
      ),
    );
  }
}

void main() {
  runApp(new MaterialApp(
    title: 'My app', // used by the OS task switcher
    home: new MyScaffold(),
  ));
}
```

&emsp;请确保在 pubspec.yaml 文件中，将 flutter 的值设置为：`uses-material-design: true`。这允许我们可以使用一组预定义 [Material icons](https://fonts.google.com/icons?selected=Material+Icons)。

```c++
name: startup_namer
description: A new Flutter project.

publish_to: 'none'

version: 1.0.0+1

environment:
  sdk: ">=2.12.0 <3.0.0"

dependencies:
  flutter:
    sdk: flutter

  cupertino_icons: ^1.0.2
  
  # 引入 english_words
  english_words: ^4.0.0

dev_dependencies:
  flutter_test:
    sdk: flutter

  flutter_lints: ^1.0.0

flutter:

  uses-material-design: true

```

&emsp;为了继承主题数据，widget 需要位于 [MaterialApp](https://api.flutter.dev/flutter/material/MaterialApp-class.html) 内才能正常显示，因此我们使用 `MaterialApp` 来运行该应用。如 `main` 函数所示：

```c++
void main() {
  runApp(new MaterialApp(
    title: 'My app', // used by the OS task switcher
    home: new MyScaffold(),
  ));
}
```

&emsp;下面我们对上面的一整段示例代码进行解读：

&emsp;在 `MyAppBar` 中创建一个 `Container`，高度是 88 像素（像素单位独立于设备，为逻辑像素），其左侧和右侧均有 8 像素的填充。在容器内部，`MyAppBar` 使用 `Row` 布局来排列其子项。中间的 title widget 被标记为 `Expanded`，这意味着它会填充尚未被其它子项占用的剩余可用空间。`Expanded` 可以拥有多个 `children`，然后使用 `flex` 参数来确定它们占用剩余空间的比例。

&emsp;`MyScaffold` 通过一个 `Column` widget，在垂直方向排列其子项。在 `Column` 的顶部，放置了一个 `MyAppBar` 实例，将一个 `Text` widget 作为其标题传递给 `MyAppBar`。将 widget 作为参数传递给其它 
widget 是一种强大的技术，可以让你创建各种复杂的 widget。最后，`MyScaffold` 使用了一个 `Expanded` 来填充剩余的空间，正中间包含一条 message。

## 使用 Material 组件

&emsp;Flutter 提供了许多 widgets，可帮助我们构建遵循 Material Design 的应用程序。Material 应用程序以 MaterialApp widget 开始，该 widget 在应用程序的根部创建了一些有用的 widget，其中包括一个 [Navigator](https://api.flutter.dev/flutter/widgets/Navigator-class.html)（使用堆栈规则管理一组子 widget 的 widget），它管理由字符串标识的 widget 栈（即页面路由栈）。Navigator 可以让你的应用程序在页面之间平滑的过渡。是否使用 MaterialApp 完全是可选的，但是使用它是一个很好的做法。

```c++
import 'package:flutter/material.dart';

void main() {
  runApp(new MaterialApp(
    title: 'Flutter Tutorial',
    home: new TutorialHome(),
  ));
}

class TutorialHome extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    // Scaffold 是 Material 中主要的布局组件.
    return new Scaffold(
      // 顶部的导航条
      appBar: new AppBar(
        // 导航条左边的菜单按钮 
        leading: new IconButton(
          icon: new Icon(Icons.menu),
          tooltip: 'Navigation menu',
          onPressed: null,
        ),

        // 导航条标题
        title: new Text('Example title'),

        // 导航条右边的搜索按钮
        actions: <Widget>[
          new IconButton(
            icon: new Icon(Icons.search),
            tooltip: 'Search',
            onPressed: null,
          ),
        ],
      ),

      // body 占屏幕的大部分（中间的一个 Hello, world! 文本）
      body: new Center(
        child: new Text('Hello, world!'),
      ),

      // 右下角浮动的加号按钮
      floatingActionButton: new FloatingActionButton(
        tooltip: 'Add', // used by assistive technologies
        child: new Icon(Icons.add),
        onPressed: null,
      ),
      
    );
  }
}
```

&emsp;现在我们已经从 MyAppBar 和 MyScaffold 切换到 [AppBar](https://api.flutter.dev/flutter/material/AppBar-class.html) 和 [Scaffold](https://api.flutter.dev/flutter/material/Scaffold-class.html) widget，我们的应用程序现在看起来已经有一些 Material 了！例如，导航条有一个阴影，标题文本会自动继承正确的样式。我们还在右下角添加了一个浮动操作按钮，以便进行相应的操作处理。

&emsp;我们再次将 widget 作为参数传递给其他 widget。该 Scaffold widget 需要许多不同的 widget 作为命名参数，其中的每一个被放置在 Scaffold 布局中相应的位置。同样，AppBar 中，我们给参数 leading、actions、title 分别传一个 widget。这种模式在整个 Flutter 框架中会经常出现。

## 处理手势

&emsp;大多数应用程序包括某种形式与系统的交互。构建交互式应用程序的第一步是检测输入手势。让我们通过一个简单的按钮来了解它的工作原理：

```c++
class MyButton extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return new GestureDetector(
      // 点击事件，打印 MyButton was tapped!
      onTap: () {
        print('MyButton was tapped!');
      },

      // widget 容器
      child: new Container(
        height: 36.0,
        padding: const EdgeInsets.all(8.0),
        margin: const EdgeInsets.symmetric(horizontal: 8.0),
        decoration: new BoxDecoration(
          borderRadius: new BorderRadius.circular(5.0),
          color: Colors.lightGreen[500],
        ),

        child: new Center(
          child: new Text('Engage'),
        ),
      ),
    );
  }
}
```

&emsp;该 [GestureDetector](https://api.flutter.dev/flutter/widgets/GestureDetector-class.html)（检测手势的 Widget）widget 并不具有显示效果，而是检测由用户做出的手势。当用户点击 [Container](https://api.flutter.dev/flutter/widgets/Container-class.html)（一个方便的 widget，结合了常见的绘画、定位和大小调整 widget）时，GestureDetector 会调用它的 onTap 回调，在回调中，将消息打印到控制台。你可以使用 GestureDetector 来检测各种输入手势，包括点击、拖动和缩放。

&emsp;许多 widget 都会使用一个 GestureDetector 为其他 widget 提供可选的回调。例如，IconButton、RaisedButton、和 FloatingActionButton，它们都有一个 onPressed 回调，它会在用户点击该 widget 时被触发。

## 根据用户输入改变 widget

&emsp;到目前为止，我们只使用了无状态的 widget。无状态 widget 从它们的父 widget 接收参数，它们被存储在 [final](https://www.dartlang.org/docs/dart-up-and-running/ch02.html#final-and-const) 修饰的成员变量中。当一个 widget 被要求构建时，它使用这些存储的值作为参数来构建 widget。

&emsp;下面我们对 final 和 const 关键字进行延展：

&emsp;如果你从不打算更改变量，请使用 final 或 const，而不是 var 或除了类型之外。 final 变量只能设置一次； const 变量是编译时常量。 （const 变量是隐式 final 的。）final top-level 或类变量在第一次使用时被初始化。

> &emsp;Note: Instance variables 可以是 final，但不能是 const。

&emsp;这是创建和设置 final 变量的示例：

```c++
final name = 'Bob'; // Without a type annotation
final String nickname = 'Bobby';
```

&emsp;你不能更改 final 变量的值：

```c++
name = 'Alice'; // Error: a final variable can only be set once.
```

&emsp;将 const 用于要成为编译时常量的变量（compile-time constants）。如果 const 变量在类级别，则将其标记为 static const。在声明变量的地方，将值设置为编译时常量，例如数字或字符串字面量、const 变量或对常量进行算术运算的结果：

```c++
const bar = 1000000; // Unit of pressure (dynes/cm2)
const double atm = 1.01325 * bar; // Standard atmosphere
```

&emsp;const 关键字不仅仅用于声明常量变量。你还可以使用它来创建常量值，以及声明创建常量值的构造函数。任何变量都可以有一个常数值。

```c++
var foo = const [];
final bar = const [];
const baz = []; // Equivalent to `const []`
```

&emsp;你可以从 const 声明的初始化表达式中省略 const，就像上面的 baz 一样。有关详细信息，请参阅 [see DON’T use const redundantly](https://dart.dev/guides/language/effective-dart/usage#dont-use-const-redundantly)。

&emsp;你可以更改 non-final、non-const 变量的值，即使它曾经有一个 const 值：

```c++
foo = [1, 2, 3]; // Was const []
```

&emsp;你不能改变 const 变量的值：

```c++
baz = [42]; // Error: Constant variables can't be assigned a value.
```

&emsp;你可以定义使用 [type checks and casts](https://dart.dev/guides/language/language-tour#type-test-operators)（is 和 as）的常量，[collection if](https://dart.dev/guides/language/language-tour#collection-operators) 和 [spread operators](https://dart.dev/guides/language/language-tour#spread-operator)（... and ...?）：

```c++
const Object i = 3; // Where i is a const Object with an int value...
const list = [i as int]; // Use a typecast.
const map = {if (i is int) i: 'int'}; // Use is and collection if.
const set = {if (list is List<int>) ...list}; // ...and a spread.
```

> &emsp;Note: 尽管无法修改 final 对象，但可以更改其字段。相比之下，const 对象及其字段不能改变：它们是不可变的。

&emsp;下面我们继续：

&emsp;为了构建更复杂的体验，例如，以更有趣的方式对用户输入做出反应，应用程序通常会携带一些状态。Flutter 使用 StatefulWidgets 来满足这种需求。StatefulWidgets 是特殊的 widget，它知道如何生成 State 对象，然后用它来保持状态。思考下面这个简单的例子，其中使用了前面提到的 RaisedButton：

```c++
class Counter extends StatefulWidget {
  // This class is the configuration for the state. It holds the values (in this nothing) provided by the parent and used by the build method of the State. Fields in a Widget subclass are always marked "final".
  // 这个类是 state 的 configuration。它保存由父级提供并由 State 的 build 方法使用的值（在此无）。Widget 子类中的字段始终标记为 final。   
  
  @override
  _CounterState createState() => new _CounterState();
}

class _CounterState extends State<Counter> {
  int _counter = 0;

  void _increment() {
    setState(() {
      // This call to setState tells the Flutter framework that something has changed in this State, which causes it to rerun the build method below so that the display can reflect the updated values.
      // If we changed _counter without calling setState(), then the build method would not be called again, and so nothing would appear to happen.
      
      // 这个对 setState 的调用告诉 Flutter 框架在这个 State 中发生了一些变化，这会导致它重新运行下面的 build 方法，以便显示可以反映更新后的值。如果我们在不调用 setState() 的情况下更改 _counter，则不会再次调用 build 方法，因此似乎什么也不会发生。
      
      _counter++;
    });
  }

  @override
  Widget build(BuildContext context) {
    // This method is rerun every time setState is called, for instance as done by the _increment method above.
    // The Flutter framework has been optimized to make rerunning build methods fast, so that you can just rebuild anything that needs updating rather than having to individually change instances of widgets.
    
    // 每次调用 setState 时都会重新运行此方法，例如上面的 _increment 方法。
    // Flutter 框架已经过优化，可以快速重新运行 build 方法，这样你就可以重新构建任何需要更新的东西，而不必单独更改 widget 的实例。
    
    return new Row(
      children: <Widget>[
        // 按钮点击调用上面的 _increment 函数，内部调用 setState  
        new RaisedButton(
          onPressed: _increment,
          child: new Text('Increment'),
        ),
        
        // Text widget 中显示 _counter 的值
        new Text('Count: $_counter'),
      ],
    );
  }
}
```

&emsp;你可能想知道为什么 StatefulWidget 和 State 是单独的对象。在 Flutter 中，这两种类型的对象具有不同的生命周期：widget 是临时对象，用于构建当前状态下的应用程序，而 State 对象在多次 build 之间保持不变，允许它们记录信息（状态）。

&emsp;上面的例子接受用户点击，并在点击时使用 _counter 自增，然后直接在其 build 方法中使用 _counter 值。在更复杂的应用程序中，widget 结构层次的不同部分可能有不同的职责；例如，一个 widget 可能呈现一个复杂的用户界面，其目标是收集特定信息（如日期或位置），而另一个 widget 可能会使用该信息来更改整体的显示。

&emsp;在 Flutter 中，事件流是 "向上" 传递的，而状态流是 "向下" 传递的（这类似于  React/Vue 中父子组件通信的方式：子 widget 到父 widget 是通过事件通信，而父到子是通过状态），重定向这一流程的共同父元素是 State。让我们看这个稍微复杂的例子是如何工作的：

```c++
// 一个 StatelessWidget，用于显示 count 的值（count 是用 final 修饰的，即只能记录 CounterDisplay 初始化时传进来的 count）
class CounterDisplay extends StatelessWidget {
  CounterDisplay({required this.count});

  final int count;

  @override
  Widget build(BuildContext context) {
    return new Text('Count: $count');
  }
}

// 同上，也是 StatelessWidget，onPressed 仅只能记录初始化传进来的值
class CounterIncrementor extends StatelessWidget {
  CounterIncrementor({required this.onPressed});

  final VoidCallback onPressed;

  @override
  Widget build(BuildContext context) {
    return new RaisedButton(
      onPressed: onPressed,
      child: new Text('Increment'),
    );
  }
}

// 使用上面两个 StatelessWidget 
class Counter extends StatefulWidget {
  @override
  _CounterState createState() => new _CounterState();
}

class _CounterState extends State<Counter> {
  int _counter = 0;

  // 每次调用 _increment 函数都会导致调用下面的 build 函数
  void _increment() {
    setState(() {
      ++_counter;
    });
  }

  @override
  Widget build(BuildContext context) {
    return new Row(children: <Widget>[
      new CounterIncrementor(onPressed: _increment),
      new CounterDisplay(count: _counter),
    ]);
  }
}
```

&emsp;注意我们是如何创建了两个新的无状态 widget 的。我们清晰的分离了显示计数器（CounterDisplay）和更改计数器（CounterIncrementor）的逻辑。尽管最终效果与前一个示例相同，但责任分离允许将复杂性逻辑封装在各个 widget 中，同时保持父项的简单性。











































## 参考链接
**参考链接:🔗**
+ [Mac pro 找不到zshrc文件](https://www.jianshu.com/p/6e9d776836ab)
+ [编写您的第一个 Flutter App](https://flutterchina.club/get-started/codelab/)
+ [Libraries and visibility](https://dart.dev/guides/language/language-tour#libraries-and-visibility)



+ [深入浅出 Flutter Framework 之 Widget](https://juejin.cn/post/6844904152905023496)
