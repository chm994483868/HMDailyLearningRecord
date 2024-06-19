# Widget class

&emsp;Flutter 进阶！静下心来把源码和注释读一遍吧，彻底掌握 Flutter 开发！如果你静不下来心，那么去试试 Flutter 岗位的面试吧，解释不出来 Flutter 原理，甚至说不出来相关的类名，那么只能 “回去等通知了！”。 

&emsp;Widget 用于描述 Element 的配置。

&emsp;`Widgets are the central class hierarchy in the Flutter framework.` -- 在 Flutter framework 中，Widget 是一个核心的类层级结构，也就是说 Widgets 是构建 Flutter 应用程序界面的基本构建块。其实，几乎所有的东西都是 Widget，从简单的文本到复杂的布局，甚至整个应用程序本身都是由 Widget 构成的。Widget 是 Flutter framework 的灵魂。

&emsp;`A widget is an immutable description of part of a user interface.` -- 一个 Widget 是用户界面的一部分，且它是不可改变的描述。换句话说，Widget 是用户界面的一个组件，一旦创建后就无法改变了。Widgets 可以被 inflated 为 elements，后者管理底层的 render tree。

&emsp;Widgets 本身没有可变状态（所有字段必须是 final）。如果你想将可变状态与 widget 关联起来，则需要使用 StatefulWidget，它会在被 inflated 为 element 并被纳入树中时创建一个 State 对象（通过 StatefulWidget.createState）。

&emsp;给定的 widget 可以零次或多次包含在树中。特别地，给定的 widget 可以多次放置在树中。每次将 widget 放置在树中时，它都会被填充为一个 Element，这意味着一个被多次放置在树中的 widget 会被多次填充。(这里理解起来有些复杂，已知 Widget 和 Element 是一一对应的，然后假如我们有一个 const 的全局 Widget(`Text temp = const Text('123');`)，我们直接把它放在页面的多个地方，那么这个 temp Widget 在每处都会生成一个不同的 Element。)

&emsp;key 属性控制一个 widget 如何替换树中的另一个 widget。如果两个 widget 的 runtimeType 和 key 属性分别为 operator ==，则新 widget 通过更新底层 element（即通过调用 Element.update 与新 widget）来替换旧 widget。否则，旧 element 将从树中移除，新 widget 将被 inflated 为一个新 element，并将新 element 插入到树中。

&emsp;另请参见：StatefulWidget 和 State，用于可以在其生命周期内多次不同构建的 widgets。InheritedWidget，用于引入可以被后代 widgets 读取的环境状态的 widgets。StatelessWidget，用于在特定配置和环境状态下始终以相同方式构建的 widgets。

&emsp;看完注释晕晕的，那下面看代码吧，注释里面所有的要义其实都来自代码：

```dart
@immutable
abstract class Widget extends DiagnosticableTree { ... }
```

&emsp;`Widget` 是一个有 `@immutable` 注解的抽象类，表示 `Widget`（及其子类，以及 `implements` 和 `with` 的类）内的每个字段都必须是 `final`。否则，Dart analyzer 将抛出警告，但不会抛出错误。`final` 关键字表示在属性初始化后不能再为其赋值（修改）。否则，Dart analyzer 将发出错误。从这里也可以印证我们见到无数次的那句：`Widgets 是不可变的`。

&emsp;关于 `DiagnosticableTree` 它是用于提供调试信息相关的方法，我们暂时不去分心去看了，后面抽时间再去看吧。








## 参考链接
**参考链接:🔗**
+ [widgets library - Widget class](https://api.flutter.dev/flutter/widgets/Widget-class.html)
+ [immutable top-level constant](https://api.flutter.dev/flutter/meta/immutable-constant.html)
+ [Immutable in Dart and Flutter: Understanding, Usage, and Best Practices](https://medium.com/@yetesfadev/immutable-in-dart-and-flutter-understanding-usage-and-best-practices-742be5fa25ea)

// diagnostics.dart 文件内容待定
