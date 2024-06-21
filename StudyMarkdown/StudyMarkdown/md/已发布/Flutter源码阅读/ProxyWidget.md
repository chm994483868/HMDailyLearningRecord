# ProxyWidget/InheritedWidget/ParentDataWidget

&emsp;ProxyWidget 通过接收一个 child widget 为参数构建内容，而不是自身构建一个新的 widget，它没有像 StatelessWidget 一样的 build 函数来构建新 widget。

&emsp;ProxyWidget 通常被用作其他 widget 的基类，比如 InheritedWidget 和 ParentDataWidget。这样的设计可以让我们更方便地实现一些复杂的 widget，让它们能够更好地与 Flutter 的布局系统和数据管理系统进行交互。（直白一点理解的话，仅仅是面向对象中的继承思想的使用，把同样的特征提取出来作为一个基类使用，而在这里 “同样的特征” 就是指：InheritedWidget 和 ParentDataWidget 都有一个 final Widget child 字段。）

&emsp;另外还可以参阅（下面是官方给其它 widget 下的定义，感觉蛮贴切的！）：

+ InheritedWidget，用于引入环境状态（ambient state）的 widget，可以被子代 widget 所读取。
+ ParentDataWidget，用于填充其子 widget 的 RenderObject.parentData 插槽的 widget，从而配置父 widget 的布局。
+ StatefulWidget和State，用于在其生命周期内可以多次构建不同内容的 widget。
+ StatelessWidget，用于在给定特定配置和 BuildContext 时总是以相同方式构建的 widget。
+ Widget，用于概述一般的 widget。

&emsp;下面看 ProxyWidget 的源码。

## ProxyWidget

&emsp;ProxyWidget 依然是直接继承自 Widget 的抽象基类，它添加了一个属于 ProxyWidget 子类的新字段：final Widget child，初始化 ProxyWidget 子类时必须传入 child 参数。

```dart
abstract class ProxyWidget extends Widget {
  /// Creates a widget that has exactly one child widget.
  /// 创建只有一个 child widget 的 widget。
  const ProxyWidget({ super.key, required this.child });
  // ...
}  
```

### child

&emsp;即是 Widget 的子类，那必然每个字段都要添加 final 修饰。

&emsp;树中位于此 widget 下方的 widget。此 widget 只能有一个子 widget。要布置多个子 widget，让此 widget 的子 widget 是 Row、Column 或 Stack 等拥有 children 属性的 widget，并将子 widget 提供给该 widget。

```dart
final Widget child;
```

&emsp;ProxyWidget 内容没啥看的，仅仅是直接继承自 Widget，然后添加一个新字段 child。而它最重要的作用仅是为了给：InheritedWidget 和 ParentDataWidget 两个超级重要的 widget 作基类使用。而这 ProxyWidget 仅仅是为了面向对象中继承基类的思想来的，哪怕直接让 InheritedWidget 和 ParentDataWidget 继承自 Widget，然后 InheritedWidget 和 ParentDataWidget 各自分别添加一个 final Widget child 也是可以的。

&emsp;下面看 ProxyWidget 的子类 InheritedWidget。

## InheritedWidget

&emsp;InheritedWidget 是用于高效传播信息到树中的 widget 的基类。InheritedWidget 提供了一种方式，可以在不同的 widget 之间有效地传递数据和信息。这种传递是自上而下的，即数据从父级 widget 传递到子级 widget，使得整个应用程序的信息传递更加高效和方便。

&emsp;[InheritedWidgets | Decoding Flutter](https://www.youtube.com/watch?v=og-vJqLzg2c)超级重要，务必观看！

&emsp;要从 build context 中获取特定类型的最近继承的 widget 的实例，请使用 BuildContext.dependOnInheritedWidgetOfExactType。当以这种方式引用时，Inherited Widget 会在其自身状态改变时导致 consumer rebuild。

&emsp;[A guide to Inherited Widgets - Flutter Widgets 101 Ep. 3](https://www.youtube.com/watch?v=Zbm3hjPjQMk)

&emsp;下面是一个名为 FrogColor 的继承自 InheritedWidget 的 widget：

```dart
class FrogColor extends InheritedWidget {
  const FrogColor({
    super.key,
    required this.color,
    required super.child,
  });

  final Color color;

  static FrogColor? maybeOf(BuildContext context) {
    return context.dependOnInheritedWidgetOfExactType<FrogColor>();
  }

  static FrogColor of(BuildContext context) {
    final FrogColor? result = maybeOf(context);
    assert(result != null, 'No FrogColor found in context');
    return result!;
  }

  @override
  bool updateShouldNotify(FrogColor oldWidget) => color != oldWidget.color;
}
```

### 实现 of 和 maybeOf 方法

&emsp;通常情况是在 InheritedWidget 上提供两个静态方法 of 和 maybeOf，这两个方法调用 BuildContext.dependOnInheritedWidgetOfExactType。这允许该类定义自己的后备逻辑，以防范围内没指定类型的 Widget。

&emsp;of 方法通常返回一个非空实例，并在未找到 InheritedWidget 时断言，maybeOf 方法返回一个可空实例，在未找到 InheritedWidget 时返回 null。通常情况下，of 方法通过在内部调用 maybeOf 实现。

&emsp;有时，of 和 maybeOf 方法返回的是一些数据，而不是 inherited widget 本身；例如，在这种情况下，它可以返回一种颜色而不是 FrogColor 小部件。

&emsp;偶尔，inherited widget 是另一个类的实现细节，因此是私有的。这种情况下，of 和 maybeOf 方法通常在公共类上实现。例如，Theme 被实现为一个构建私有的 inherited widget 的 StatelessWidget（class Theme extends StatelessWidget { // ... }），Theme.of 使用 BuildContext.dependOnInheritedWidgetOfExactType 寻找该私有 inherited widget，然后返回其中的 ThemeData。

&emsp;例如我们经常看到的 Theme.of(context) 的使用，而在 Theme 类的静态 of 函数内部其实是在 context 中查找一个私有的 inherited widget: `_InheritedTheme`。

```dart
Text('Example', style: Theme.of(context).textTheme.titleLarge,)

class Theme extends StatelessWidget {

// ...
static ThemeData of(BuildContext context) {
  final _InheritedTheme? inheritedTheme = context.dependOnInheritedWidgetOfExactType<_InheritedTheme>();
    ...
    
}

// ...

}
```

### 调用 of 或 maybeOf 方法

&emsp;当使用 of 或 maybeOf 方法时，context 必须是 InheritedWidget 的子孙（descendant），也就是说，context 必须是在 InheritedWidget 在树中的位置是 "below"。意思就是，在 context 必须是嵌套在 InheritedWidget 内部的。例如下面这个例子，使用的 context 是来自 Builder 的 context，而 Builder 是 FrogColor widget 的子 widget，所以这样是可以的。 

```dart
// continuing from previous example...
class MyPage extends StatelessWidget {
  const MyPage({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: FrogColor(
        color: Colors.green,
        child: Builder(
          builder: (BuildContext innerContext) {
            return Text(
              'Hello Frog',
              style: TextStyle(color: FrogColor.of(innerContext).color),
            );
          },
        ),
      ),
    );
  }
}
```

&emsp;下面看一个反例。在下面这个例子中，使用的 context 是来自 MyOtherPage widget，它是 FrogColor widget 的父级，所以这样不起作用，而且当调用 FrogColor.of 时会触发断言。

```dart
// continuing from previous example...

class MyOtherPage extends StatelessWidget {
  const MyOtherPage({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: FrogColor(
        color: Colors.green,
        child: Text(
          'Hello Frog',
          style: TextStyle(color: FrogColor.of(context).color),
        ),
      ),
    );
  }
}
```

&emsp;[InheritedWidget (Flutter Widget of the Week)](https://www.youtube.com/watch?v=1t-8rBCGBYw)

&emsp;另外可参考：

+ InheritedNotifier，一个 InheritedWidget 子类，其值可以是 Listenable，并且当值发送通知时，会通知依赖项。
+ InheritedModel，一个 InheritedWidget 子类，允许客户端订阅值的子 widget 的更改。

&emsp;好了，InheritedWidget 相关的文档都看完了，下面我们开始看 InheritedWidget 的源码。

### createElement

&emsp;

```dart
abstract class InheritedWidget extends ProxyWidget {
  /// Abstract const constructor. This constructor enables subclasses to provide
  /// const constructors so that they can be used in const expressions.
  // 抽象常量构造函数。该构造函数使得子类能够提供常量构造函数，从而可以在常量表达式中使用。
  const InheritedWidget({ super.key, required super.child });

  @override
  InheritedElement createElement() => InheritedElement(this);
  
  // ...
}
```

&emsp;

### updateShouldNotify

&emsp;

```dart
  @protected
  bool updateShouldNotify(covariant InheritedWidget oldWidget);
```

&emsp;


## 参考链接
**参考链接:🔗**
+ [widgets library - ProxyWidget class](https://api.flutter.dev/flutter/widgets/ProxyWidget-class.html)
