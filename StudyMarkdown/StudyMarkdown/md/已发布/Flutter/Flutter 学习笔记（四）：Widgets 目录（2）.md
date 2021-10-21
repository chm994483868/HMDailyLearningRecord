# Flutter 学习笔记（四）：Widgets 目录（2）

&emsp;使用 Flutter 的一套的视觉、结构、平台、和交互式的 widgets，快速创建漂亮的 APP。

## Cupertino(iOS 风格的 widget)

&emsp;用于当前 iOS 设计语言的美丽和高保真 widget。（它们用了 Cupertino 库比提诺作为前缀，它是美国一座城市的名字，也是苹果总部所在地，为什么不干脆直接以 Apple 为前缀呢😂）

### CupertinoActivityIndicator

&emsp;[CupertinoActivityIndicator class](https://docs.flutter.io/flutter/cupertino/CupertinoActivityIndicator-class.html) 顺时针旋转的 iOS 风格的活动指示器，即对应于 UIKit 中的 UIActivityIndicatorView，在 Flutter 中可以通过 radius 改变其大小，但是在 iOS 中比较麻烦，只能通过修改 UIActivityIndicatorView 的 transform 属性来改变其大小。

### CupertinoAlertDialog

&emsp;[CupertinoAlertDialog class](https://docs.flutter.io/flutter/cupertino/CupertinoAlertDialog-class.html) iOS风格的 alert dialog.

&emsp;你的应用是否具有用户需要做出重要决策的关键节点？你想提醒某人某事或者得到他们的输入吗？尝试使用 AlertDialog，根据你的首选设计，如果是 Material 的话可以使用 AlertDialog 或者使用 iOS 的 CupertinoAlertDialog 最基本的对话框是弹出窗口，包含一些内容和一些按钮，右上角是 title，后面是主要的 content，通常是文字，但也可能是动图，或者你想要的任何东西。用户应该清楚如何处理警报。因此请指定他们如何通过传递给 actions 参数的按钮来响应警报，如果你使用 Material，AlertDialog 会提供额外属性，像卡片一样，你可以更改对话框的 elevation，也可以更改 background color，而且，如果你真的想要花俏一点的话，可以使用 shape 参数修改对话框的形状，CupertinoAlertDialog 和 AlertDialog 分别与辅助方法 showCupertinoDialog 和 showDialog 配对，可以很好的显示对话框，ShowDialog 具有 context 和 builder 返回要显示的特定对话框，然后，你可以告诉它用户是否可以点击它的外部将其关闭，就这么简单。ShowDialog 和 AlertDialog 用 Material 的方式实现了对话框的执行，CupertinoAlertDialog 和 ShowCupertinoDialog 接受许多相同的参数并为 iOS 渲染。     

```c++
CupertinoAlertDialog(
  title: Text("Accept?"),
  content: Text("Do you accept?"),
  content: Image("dash.gif"),
  content: MyFancyWidget(),
  actions: [
    CupertinoDialogAction("No"),
    CupertinoDialogAction("Yes"),
  ],
);

AlertDialog(
  title: Text("Accept?"),
  content: Text("Do you accept?"),
  actions: [
    FlatButton("No"),
    FlatButton("Yes"),
  ],
  elevation: 24.0,
  backgroundColor: Colors.blue,
  shape: CirleBorder(),
};

showDialog(
  context: context,
  builder: (_) => AlertDialog(),
  barrierDismissible: false,
);
```

### CupertinoButton

&emsp;[CupertinoButton class](https://api.flutter.dev/flutter/cupertino/CupertinoButton-class.html) iOS 风格的 button。

### CupertinoDialogAction

&emsp;[CupertinoDialogAction class](https://api.flutter.dev/flutter/cupertino/CupertinoDialogAction-class.html) 通常用于CupertinoAlertDialog的一个button

### CupertinoSlider

&emsp;[CupertinoSlider class](https://api.flutter.dev/flutter/cupertino/CupertinoSlider-class.html) 从一个范围中选一个值，对应于 UISlider。

### CupertinoSwitch

&emsp;[CupertinoSwitch class](https://api.flutter.dev/flutter/cupertino/CupertinoSwitch-class.html) iOS 风格的开关，用于单一状态的开/关，对应于 UISwitch。

### CupertinoPageTransition

&emsp;[CupertinoPageTransition class](https://api.flutter.dev/flutter/cupertino/CupertinoPageTransition-class.html) 提供 iOS 风格的页面过渡动画。页面从右侧滑入，然后反向退出。当另一页进入覆盖它时，它也会以视差运动向左移动。

### CupertinoFullscreenDialogTransition

&emsp;[CupertinoFullscreenDialogTransition class](https://api.flutter.dev/flutter/cupertino/CupertinoFullscreenDialogTransition-class.html) 一个 iOS 风格的过渡，用于调用全屏对话框。例如，在创建新日历事件时，从底部引入下一个屏幕。

### CupertinoNavigationBar

&emsp;[CupertinoNavigationBar class](https://api.flutter.dev/flutter/cupertino/CupertinoNavigationBar-class.html) iOS 风格的导航栏. 通常和 CupertinoPageScaffold 一起使用。

### CupertinoTabBar

&emsp;[CupertinoTabBar class](https://api.flutter.dev/flutter/cupertino/CupertinoTabBar-class.html) iOS 风格的底部选项卡。 通常和 CupertinoTabScaffold 一起使用。

### CupertinoPageScaffold

&emsp;[CupertinoPageScaffold class](https://api.flutter.dev/flutter/cupertino/CupertinoPageScaffold-class.html) 一个 iOS 风格的页面的基本布局结构。包含内容和导航栏。

### CupertinoTabScaffold

&emsp;[CupertinoTabScaffold class](https://api.flutter.dev/flutter/cupertino/CupertinoTabScaffold-class.html) 标签式 iOS 应用程序的结构。将选项卡栏放在内容选项卡之上。

### CupertinoTabView

&emsp;[CupertinoTabView class](https://api.flutter.dev/flutter/cupertino/CupertinoTabView-class.html) 支持选项卡间并行导航项卡的根内容。通常与 CupertinoTabScaffolde 一起使用。

## Layout

&emsp;排列其它 widget 的columns、rows、grids 和其它的 layouts。

### 拥有单个子元素的布局 widget

#### Container

&emsp;[Container class](https://api.flutter.dev/flutter/widgets/Container-class.html) 一个拥有绘制、定位、调整大小的 widget。

#### Padding

&emsp;[Padding class](https://api.flutter.dev/flutter/widgets/Padding-class.html) 一个 widget, 会给其子 widget 添加指定的填充。()

```c++
const Card(
  child: Padding(
    padding: EdgeInsets.all(16.0),
    child: Text('Hello World!'),
  ),
)
```

#### Center

&emsp;[Center class](https://api.flutter.dev/flutter/widgets/Center-class.html) 将其子 widget 居中显示在自身内部的 widget。

&emsp;如果此 widget 的尺寸受到约束且 widthFactor 和 heightFactor 为空，则此 widget 将尽可能大。如果维度不受约束，且相应的大小因子为空，则 widget 将匹配该维度中其子维度的大小。如果大小因子为非空，则此 widget 的相应维度将是子维度和大小因子的乘积。例如，如果 widthFactor 为 2.0，则此 widget 的宽度将始终是其子部件宽度的两倍。

#### Align

&emsp;[Align class](https://api.flutter.dev/flutter/widgets/Align-class.html) 一个 widget，它可以将其子 widget 对齐，并可以根据子 widget 的大小自动调整大小。

```c++
Center(
  child: Container(
    height: 120.0,
    width: 120.0,
    color: Colors.blue[50],
    child: const Align(
      alignment: Alignment.topRight,
      child: FlutterLogo(
        size: 60,
      ),
    ),
  ),
)
```

#### FittedBox

&emsp;[FittedBox class](https://api.flutter.dev/flutter/widgets/FittedBox-class.html) 按自己的大小调整其子 widget 的大小和位置。

&emsp;大多数 Flutter widget 都是盒子，你可以将它们布置、堆叠、甚至相互嵌套，但是，当一个盒子不适合另一个盒子时会发生什么呢？为了解决这个问题，就有了 FittedBox，只需为其提供一个子级 widget 和一个 BoxFit，它描述如何使子级 widget 贴合自己，它将自动缩放或裁剪其子级 widget，BoxFit 枚举提供了一堆不同的贴合类型，例如 fitWidth、fill、甚至 none，还有一个对齐属性，告诉 FittedBox 如果有多余的空间如何对齐子级 

```c++
MyBlueRect(
  child: FittedBox(
    alignment: Alignment.centerLeft,
    fit: BoxFit.contain,
    child: MyDashPic(),
  ),
)
```

```c++
/// Flutter code sample for FittedBox

// In this example, the image is stretched to fill the entire [Container], which would not happen normally without using FittedBox.
// 在此示例中，image 被拉伸以填充整个 [Container]，如果不使用 FittedBox，通常不会发生。

import 'package:flutter/material.dart';

void main() => runApp(const MyApp());

/// This is the main application widget.
class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  static const String _title = 'Flutter Code Sample';

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: _title,
      home: Scaffold(
        appBar: AppBar(title: const Text(_title)),
        body: const Center(
          child: MyStatelessWidget(),
        ),
      ),
    );
  }
}

/// This is the stateless widget that the main application instantiates.
class MyStatelessWidget extends StatelessWidget {
  const MyStatelessWidget({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Container(
      height: 400,
      width: 300,
      color: Colors.red,
      child: FittedBox(
        child: Image.network(
            'https://flutter.github.io/assets-for-api-docs/assets/widgets/owl-2.jpg'),
        fit: BoxFit.fill,
      ),
    );
  }
}
```

#### AspectRatio

&emsp;[AspectRatio class](https://api.flutter.dev/flutter/widgets/AspectRatio-class.html) 一个 widget，试图将子 widget 的大小指定为某个特定的长宽比。

```c++
Expanded(
  child: Align(
    alignment: Alignment.bottomCenter,
    child: AspectRatio(
      aspectRatio: 3 / 2,
      child: MyWidget(),
    ),
  ),
)
```

#### ConstrainedBox

&emsp;[ConstrainedBox class](https://api.flutter.dev/flutter/widgets/ConstrainedBox-class.html) 对其子项施加附加约束的 widget。

```c++
ConstrainedBox(
  constraints: BoxConstraints(
    maxWidth: 40,
    maxHeight: 30,
  ),
  child: Container(),
)
```

&emsp;约束文本 widget 的最大宽度，使其跨越多行：

```c++
ConstrainedBox(
  constraints: BoxConstraints(
    maxWidth: 200,
  ),
  child: Text(
    'Delicious Candy',
    textAlign: TextAlign.center,
  ),
)
```

&emsp;可以通过约束其最小高度来拉伸凸起按钮的高度：

```c++
ConstrainedBox(
  constraints: BoxConstraints(
    minHeight: 100,
  ),
  child: RaisedButton(
    child: Text('Tap Me!'),
    onPressed: () {},
  ),
)
```

#### Baseline

&emsp;[Baseline class](https://api.flutter.dev/flutter/widgets/Baseline-class.html) 根据子项的基线对它们的位置进行定位的 widget。

#### FractionallySizedBox

&emsp;[FractionallySizedBox class](https://api.flutter.dev/flutter/widgets/FractionallySizedBox-class.html) 一个 widget，它把它的子项放在可用空间的一小部分。关于布局算法的更多细节，见 RenderFractionallySizedOverflowBox

```c++
Container(
  alignment: Alignment.center,
  child: FractionallySizedBox(
    widthFactor: 0.7,
    child: MyButton(
      child: Text('PRESS HERE'),
    ),
  ),
)
```

&emsp;小尺寸的空白也可以使用没有子项的 FractionallySizedBox:

```c++
FractionallySizedBox(
  heightFactor: 0.1,
)

Flexible(
  child: FractionallySizedBox(
    heightFactor: 0.1,
  ),
)
```

#### IntrinsicHeight

&emsp;[IntrinsicHeight class](https://api.flutter.dev/flutter/widgets/IntrinsicHeight-class.html) 一个 widget，它将它的子 widget 的高度调整其本身实际的高度。

#### IntrinsicWidth

&emsp;[IntrinsicWidth class](https://api.flutter.dev/flutter/widgets/IntrinsicWidth-class.html) 一个 widget，它将它的子 widget 的宽度调整其本身实际的宽度.

#### LimitedBox

&emsp;[LimitedBox class](https://api.flutter.dev/flutter/widgets/LimitedBox-class.html) 一个当其自身不受约束时才限制其大小的盒子。

```c++
ListView(
 children: [
   for (var i = 0; i < 10; i++)
     LimitedBox(
       maxHeight: 200,
       child: Container(
         color: randomColor(),
       ),
     ),
  ],
)
```

#### Offstage

&emsp;[Offstage class](https://api.flutter.dev/flutter/widgets/Offstage-class.html) 一个布局 widget，可以控制其子 widget 的显示和隐藏。

#### OverflowBox

&emsp;[OverflowBox class](https://api.flutter.dev/flutter/widgets/OverflowBox-class.html) 对其子项施加不同约束的 widget，它可能允许子项溢出父级。

#### SizedBox

&emsp;[SizedBox class](https://api.flutter.dev/flutter/widgets/SizedBox-class.html) 一个特定大小的盒子。这个 widget 强制它的孩子有一个特定的宽度和高度。如果宽度或高度为 NULL，则此 widget 将调整自身大小以匹配该维度中的孩子的大小。

```c++
SizedBox(
  width: 200,
  width: double.infinity,
  height: 100,
  height: double.infinity,
  child: MyButton(),
)

SizedBox.expand(
  child: MyButton(),
)
```

&emsp;可以使用 SizeBox 在 widget 之间添加间隙。

```c++
Column(
  children: [
    MyButton(),
    SizedBox(height: 200),
    OtherButton(),
  ],
)
```

#### SizedOverflowBox

&emsp;[SizedOverflowBox class](https://api.flutter.dev/flutter/widgets/SizedOverflowBox-class.html) 一个特定大小的 widget，但是会将它的原始约束传递给它的孩子，它可能会溢出。

#### Transform

&emsp;[Transform class](https://docs.flutter.io/flutter/widgets/Transform-class.html) 在绘制子 widget 之前应用转换的 widget。

```c++
Transform.rotate(
  angle: pi/4, // 45 deg
  child: MyIcon(),
)

Transform.scale(
  scale: 1.5,
  child: MyIcon(),
)

Transform.translate(
  offset: Offset(50, 50),
  child: MyIcon(),
)

Transform(
  transform: Matrix4.skewX(0.3),
  child: MyIcon(),
)
```

#### CustomSingleChildLayout

&emsp;[CustomSingleChildLayout class](https://api.flutter.dev/flutter/widgets/CustomSingleChildLayout-class.html) 一个自定义的拥有单个子 widget 的布局 widget。

### 拥有多个子元素的布局 widget

#### Row 

&emsp;[Row class](https://api.flutter.dev/flutter/widgets/Row-class.html) 在水平方向上排列子 widget 的列表。

&emsp;两个文本和一个图片在一行上从左到右显示：

```c++
Row(
  children: const <Widget>[
    Expanded(
      child: Text('Deliver features faster', textAlign: TextAlign.center),
    ),
    Expanded(
      child: Text('Craft beautiful UIs', textAlign: TextAlign.center),
    ),
    Expanded(
      child: FittedBox(
        fit: BoxFit.contain, // otherwise the logo will be tiny
        child: FlutterLogo(),
      ),
    ),
  ],
)
```

#### Column

&emsp;[Column class](https://api.flutter.dev/flutter/widgets/Column-class.html) 在垂直方向上排列子widget的列表。

&emsp;两个文本和一个图片在一列上从上到下显示：

```c++
Column(
  children: const <Widget>[
    Text('Deliver features faster'),
    Text('Craft beautiful UIs'),
    Expanded(
      child: FittedBox(
        fit: BoxFit.contain, // otherwise the logo will be tiny
        child: FlutterLogo(),
      ),
    ),
  ],
)
```

#### Stack

&emsp;[Stack class](https://api.flutter.dev/flutter/widgets/Stack-class.html) 可以允许其子 widget 简单的堆叠在一起。

&emsp;一组 widget 堆叠在一起：

```c++
Stack(
  children: <Widget>[
    Container(
      width: 100,
      height: 100,
      color: Colors.red,
    ),
    Container(
      width: 90,
      height: 90,
      color: Colors.green,
    ),
    Container(
      width: 80,
      height: 80,
      color: Colors.blue,
    ),
  ],
)
```

#### IndexedStack

&emsp;[IndexedStack class](https://api.flutter.dev/flutter/widgets/IndexedStack-class.html) 从一个子 widget 列表中显示单个孩子的 Stack。 

```c++
IndexedStack(
  index: _widgetIndex,
  children: [
    WidgetOne(),
    WidgetTwo(),
  ],
)

// elsewhere in the code
setState(
  () => _widgetIndex = 2);
```

#### Flow

&emsp;[Flow class](https://api.flutter.dev/flutter/widgets/Flow-class.html) 一个实现流式布局算法的 widget。

#### Table

&emsp;[Table class](https://api.flutter.dev/flutter/widgets/Table-class.html) 为其子 widget 使用表格布局算法的 widget。 

#### Wrap

&emsp;[Wrap class](https://api.flutter.dev/flutter/widgets/Wrap-class.html) 可以在水平或垂直方向多行显示其子 widget。

```c++
Wrap(
  spacing: 8.0, // gap between adjacent chips
  runSpacing: 4.0, // gap between lines
  children: <Widget>[
    Chip(
      avatar: CircleAvatar(backgroundColor: Colors.blue.shade900, child: const Text('AH')),
      label: const Text('Hamilton'),
    ),
    Chip(
      avatar: CircleAvatar(backgroundColor: Colors.blue.shade900, child: const Text('ML')),
      label: const Text('Lafayette'),
    ),
    Chip(
      avatar: CircleAvatar(backgroundColor: Colors.blue.shade900, child: const Text('HM')),
      label: const Text('Mulligan'),
    ),
    Chip(
      avatar: CircleAvatar(backgroundColor: Colors.blue.shade900, child: const Text('JL')),
      label: const Text('Laurens'),
    ),
  ],
)
```

#### ListBody

&emsp;[ListBody class](https://api.flutter.dev/flutter/widgets/ListBody-class.html) 一个 widget，它沿着一个给定的轴，顺序排列它的子元素。

#### ListView

&emsp;[ListView class](https://api.flutter.dev/flutter/widgets/ListView-class.html) 可滚动的列表控件。ListView 是最常用的滚动 widget，它在滚动方向上一个接一个地显示它的孩子。在纵轴上，孩子们被要求填充 ListView。

```c++
ListView(
  padding: const EdgeInsets.all(8),
  children: <Widget>[
    Container(
      height: 50,
      color: Colors.amber[600],
      child: const Center(child: Text('Entry A')),
    ),
    Container(
      height: 50,
      color: Colors.amber[500],
      child: const Center(child: Text('Entry B')),
    ),
    Container(
      height: 50,
      color: Colors.amber[100],
      child: const Center(child: Text('Entry C')),
    ),
  ],
)
```

#### CustomMultiChildLayout

&emsp;[CustomMultiChildLayout class](https://api.flutter.dev/flutter/widgets/CustomMultiChildLayout-class.html) 使用一个委托来对多个孩子进行设置大小和定位的小部件。

### Layout helpers

#### LayoutBuilder 

&emsp;[LayoutBuilder class](https://api.flutter.dev/flutter/widgets/LayoutBuilder-class.html) 构建一个可以依赖父窗口大小的 widget 树。

```c++
class HomeScreen extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return LayoutBuilder(
      builder: (context, constraints) {
      },
    );
  }
}
```

```c++
Widget build(BuildContext context) {
  return LayoutBuilder(
    builder: (context, constraints) {
      if (constraints.maxWidth < 600) {
        return MyOneColumnLayout();
      } else {
        return MyTwoColumnLayout();
      }
    },
  );
}
```

### 文本 Widget 

&emsp;文本显示和样式。

#### Text

&emsp;[Text class](https://api.flutter.dev/flutter/widgets/Text-class.html) 单一格式的文本。 

```c++
Text(
  'Hello, $_name! How are you?',
  textAlign: TextAlign.center,
  overflow: TextOverflow.ellipsis,
  style: const TextStyle(fontWeight: FontWeight.bold),
)
```

```c++
const Text.rich(
  TextSpan(
    text: 'Hello', // default text style
    children: <TextSpan>[
      TextSpan(text: ' beautiful ', style: TextStyle(fontStyle: FontStyle.italic)),
      TextSpan(text: 'world', style: TextStyle(fontWeight: FontWeight.bold)),
    ],
  ),
)
```

#### RichText

&emsp;[RichText class](https://api.flutter.dev/flutter/widgets/RichText-class.html) 一个富文本 Text，可以显示多种样式的 text。

&emsp;在其中加入 TextSpan 可以进行自定义显示以及交互（点击超链接跳转）等。

```c++
RichText(
  text: TextSpan(
    text: 'Hello ',
    style: DefaultTextStyle.of(context).style,
    children: const <TextSpan>[
      TextSpan(text: 'bold', style: TextStyle(fontWeight: FontWeight.bold)),
      TextSpan(text: ' world!'),
    ],
  ),
)
```

#### DefaultTextStyle 

&emsp;[DefaultTextStyle class](https://api.flutter.dev/flutter/widgets/DefaultTextStyle-class.html) 文字样式，用于指定 Text widget 的文字样式。

### Input 

&emsp;Material Components 和 Cupertino 中获取用户输入的 widget。

#### Form

&emsp;[Form class](https://api.flutter.dev/flutter/widgets/Form-class.html) 一个可选的、用于给多个 TextField 分组的 widget。

#### FormField

&emsp;[FormField<T> class](https://api.flutter.dev/flutter/widgets/FormField-class.html) 一个单独的表单字段。此 widget 维护表单字段的当前状态，以便在 UI 中直观地反映更新和验证错误。

#### RawKeyboardListener

&emsp;[RawKeyboardListener class](https://api.flutter.dev/flutter/widgets/RawKeyboardListener-class.html) 每当用户按下或释放键盘上的键时调用回调的 widget。

### 动画和Motion 

&emsp;在应用中使用动画。

#### AnimatedContainer

&emsp;[AnimatedContainer class](https://api.flutter.dev/flutter/widgets/AnimatedContainer-class.html) 在一段时间内逐渐改变其值的容器。

#### AnimatedCrossFade

&emsp;[AnimatedCrossFade class](https://api.flutter.dev/flutter/widgets/AnimatedCrossFade-class.html) 一个 widget，在两个孩子之间交叉淡入，并同时调整他们的尺寸。

```c++
AnimatedCrossFade(
  duration: const Duration(seconds: 3),
  firstChild: const FlutterLogo(style: FlutterLogoStyle.horizontal, size: 100.0),
  secondChild: const FlutterLogo(style: FlutterLogoStyle.stacked, size: 100.0),
  crossFadeState: _first ? CrossFadeState.showFirst : CrossFadeState.showSecond,
)
```

#### Hero

&emsp;[Hero class](https://api.flutter.dev/flutter/widgets/Hero-class.html) 将其子项标记为 hero 动画候选的 widget。

#### AnimatedBuilder

&emsp;[AnimatedBuilder class](https://api.flutter.dev/flutter/widgets/AnimatedBuilder-class.html) 用于构建动画的通用小部件。AnimatedBuilder 在有多个 widget 希望有一个动画作为一个较大的建造函数部分时会非常有用。要使用 AnimatedBuilder，只需构建 widget 并将其传给 builder 函数即可。

#### DecoratedBoxTransition

&emsp;[DecoratedBoxTransition class](https://api.flutter.dev/flutter/widgets/DecoratedBoxTransition-class.html) DecoratedBox 的动画版本，可以给它的 Decoration 不同属性使用动画。

#### FadeTransition

&emsp;[FadeTransition class](https://api.flutter.dev/flutter/widgets/FadeTransition-class.html) 对透明度使用动画的 widget。

#### PositionedTransition

&emsp;[PositionedTransition class](https://api.flutter.dev/flutter/widgets/PositionedTransition-class.html) Positioned 的动画版本，它需要一个特定的动画来将孩子的位置从动画的生命周期的起始位置移到结束位置。

#### RotationTransition

&emsp;[RotationTransition class](https://api.flutter.dev/flutter/widgets/RotationTransition-class.html) 对 widget 使用旋转动画。

#### ScaleTransition

&emsp;[ScaleTransition class](https://api.flutter.dev/flutter/widgets/ScaleTransition-class.html) 对 widget 使用缩放动画。

#### SizeTransition

&emsp;[SizeTransition class](https://api.flutter.dev/flutter/widgets/SizeTransition-class.html) 对自己的 size 和 clips 进行动画，并调整孩子。

#### SlideTransition

&emsp;[SlideTransition class](https://api.flutter.dev/flutter/widgets/SlideTransition-class.html) 对相对于其正常位置的某个位置之间使用动画。

#### AnimatedDefaultTextStyle

&emsp;[AnimatedDefaultTextStyle class](https://api.flutter.dev/flutter/widgets/AnimatedDefaultTextStyle-class.html) 在文本样式切换时使用动画。

#### AnimatedListState

&emsp;[AnimatedListState class](https://api.flutter.dev/flutter/widgets/AnimatedListState-class.html) 动画列表的 state。

#### AnimatedModalBarrier

&emsp;[AnimatedModalBarrier class](https://api.flutter.dev/flutter/widgets/AnimatedModalBarrier-class.html) 一个阻止用户与 widget 交互的 widget。

#### AnimatedOpacity

&emsp;[AnimatedOpacity class](https://api.flutter.dev/flutter/widgets/AnimatedOpacity-class.html) Opacity 的动画版本，在给定的透明度变化时，自动地在给定的一段时间内改变孩子的 Opacity。

#### AnimatedPhysicalModel

&emsp;[AnimatedPhysicalModel class](https://api.flutter.dev/flutter/widgets/AnimatedPhysicalModel-class.html) PhysicalModel 的动画版本。

#### AnimatedPositioned

&emsp;[AnimatedPositioned class](https://api.flutter.dev/flutter/widgets/AnimatedPositioned-class.html) 动画版本的 Positioned，每当给定位置的变化，自动在给定的时间内转换孩子的位置。

#### AnimatedSize

&emsp;[AnimatedSize class](https://api.flutter.dev/flutter/widgets/AnimatedSize-class.html) 动画 widget，当给定的孩子的大小变化时，它自动地在给定时间内转换它的大小。

#### AnimatedWidget

&emsp;[AnimatedWidget class](https://api.flutter.dev/flutter/widgets/AnimatedWidget-class.html) 当给定的 Listenable 改变值时，会重新构建该 widget。

#### AnimatedWidgetBaseState

&emsp;[AnimatedWidgetBaseState<T extends ImplicitlyAnimatedWidget> class](https://api.flutter.dev/flutter/widgets/AnimatedWidgetBaseState-class.html) 具有隐式动画的 widget 的基类。

### 交互模型

&emsp;响应触摸事件并将用户路由到不同的页面视图（View）。

#### LongPressDraggable 

&emsp;[LongPressDraggable<T extends Object> class](https://api.flutter.dev/flutter/widgets/LongPressDraggable-class.html) 可以使其子 widget 在长按时可拖动。

#### GestureDetector

&emsp;[GestureDetector class](https://api.flutter.dev/flutter/widgets/GestureDetector-class.html) 一个检测手势的 widget。

#### DragTarget

&emsp;[DragTarget<T extends Object> class](https://api.flutter.dev/flutter/widgets/DragTarget-class.html) 一个拖动的目标 widget，在完成拖动时它可以接收数据。

#### Dismissible

&emsp;[Dismissible class](https://api.flutter.dev/flutter/widgets/Dismissible-class.html) 可以在拖动时隐藏的 widget。

#### IgnorePointer

&emsp;[IgnorePointer class](https://api.flutter.dev/flutter/widgets/IgnorePointer-class.html) 在 hit test 中不可见的 widget。当 ignoring 为 true 时，此 widget 及其子树不响应事件。但它在布局过程中仍然消耗空间，并像往常一样绘制它的孩子。它是无法捕获事件对象、因为它在 RenderBox.hitTest 中返回 false。

#### AbsorbPointer

&emsp;[AbsorbPointer class](https://api.flutter.dev/flutter/widgets/AbsorbPointer-class.html) 在 hit test 期间吸收(拦截)事件。当 absorbing 为 true 时，此小部件阻止其子树通过终止命中测试来接收指针事件。它在布局过程中仍然消耗空间，并像往常一样绘制它的孩子。它只是防止其孩子成为事件命中目标，因为它从 RenderBox.hitTest 返回 true。

#### Navigator

&emsp;[Navigator class](https://api.flutter.dev/flutter/widgets/Navigator-class.html) 导航器，可以在多个页面(路由)栈之间跳转。

#### Scrollable

&emsp;[Scrollable class](https://api.flutter.dev/flutter/widgets/Scrollable-class.html) 实现了可滚动 widget 的交互模型，但不包含 UI 显示相关的逻辑。

### 样式

&emsp;管理应用的主题，使应用能够响应式的适应屏幕尺寸或添加填充。

#### Padding

&emsp;[Padding class](https://api.flutter.dev/flutter/widgets/Padding-class.html) 一个 widget, 会给其子 widget 添加指定的填充。

#### Theme

&emsp;[Theme class](https://api.flutter.dev/flutter/material/Theme-class.html) 将主题应用于子 widget。主题描述了应用选择的颜色和字体。

#### MediaQuery

&emsp;[MediaQuery class](https://api.flutter.dev/flutter/widgets/MediaQuery-class.html) 建立一个子树，在树中媒体查询解析不同的给定数据。

### 绘制和效果

&emsp;Widget 将视觉效果应用到其子组件，而不改变它们的布局、大小和位置。

#### Opacity

&emsp;[Opacity class](https://api.flutter.dev/flutter/widgets/Opacity-class.html) 使其子 widget 透明的 widget。

```c++
Opacity(
  opacity: _visible ? 1.0 : 0.0,
  child: const Text("Now you see me, now you don't!"),
)
```

#### Transform

&emsp;[Transform class](https://api.flutter.dev/flutter/widgets/Transform-class.html) 在绘制子 widget 之前应用转换的 widget。

#### DecoratedBox

&emsp;[DecoratedBox class](https://api.flutter.dev/flutter/widgets/DecoratedBox-class.html) 在孩子绘制之前或之后绘制装饰的 widget。

#### FractionalTranslation

&emsp;[FractionalTranslation class](https://api.flutter.dev/flutter/widgets/FractionalTranslation-class.html) 绘制盒子之前给其添加一个偏移转换。

#### RotatedBox

&emsp;[RotatedBox class](https://api.flutter.dev/flutter/widgets/RotatedBox-class.html) 可以延顺时针以 90 度的倍数旋转其子 widget。

&emsp;对比 Transform.rotate 而言，RotatedBox 只会影响渲染而不会影响布局，Transform.rotate 中的布局将表现好似未旋转的子部件仍占据着其原始位置，Transform.rotate 因此对动画和轻微旋转很有用，但对于像这样永久旋转的 box，请使用 RotatedBox。

```c++
const RotatedBox(
  quarterTurns: 3,
  child: Text('Hello World!'),
)
```

#### ClipOval 

&emsp;[ClipOval class](https://api.flutter.dev/flutter/widgets/ClipOval-class.html) 用椭圆剪辑其孩子的 widget。

#### ClipPath

&emsp;[ClipPath class](https://api.flutter.dev/flutter/widgets/ClipPath-class.html) 用 path 剪辑其孩子的 widget。

#### ClipRect

&emsp;[ClipRect class](https://api.flutter.dev/flutter/widgets/ClipRect-class.html) 用矩形剪辑其孩子的 widget。

#### CustomPaint

&emsp;[CustomPaint class](https://api.flutter.dev/flutter/widgets/CustomPaint-class.html) 提供一个画布的 widget，在绘制阶段可以在画布上绘制自定义图形。

#### BackdropFilter

&emsp;[BackdropFilter class](https://api.flutter.dev/flutter/widgets/BackdropFilter-class.html) 一个 widget，它将过滤器应用到现有的绘图内容，然后绘制孩子。这种效果是比较昂贵的，尤其是如果过滤器是 non-local，如 blur。


### 异步 Widgets

&emsp;Flutter 应用的异步模型。

#### FutureBuilder

&emsp;[FutureBuilder<T> class](https://api.flutter.dev/flutter/widgets/FutureBuilder-class.html) 基于与 Future 交互的最新快照来构建自身的 widget。

#### StreamBuilder

&emsp;[StreamBuilder<T> class](https://api.flutter.dev/flutter/widgets/StreamBuilder-class.html) 基于与流交互的最新快照构建自身的 widget。

### 可滚动的Widget

&emsp;滚动一个拥有多个子组件的父组件。

#### ListView

&emsp;[ListView class](https://api.flutter.dev/flutter/widgets/ListView-class.html) 一个可滚动的列表。

```c++
ListView(
  padding: const EdgeInsets.all(8),
  children: <Widget>[
    Container(
      height: 50,
      color: Colors.amber[600],
      child: const Center(child: Text('Entry A')),
    ),
    Container(
      height: 50,
      color: Colors.amber[500],
      child: const Center(child: Text('Entry B')),
    ),
    Container(
      height: 50,
      color: Colors.amber[100],
      child: const Center(child: Text('Entry C')),
    ),
  ],
)
```

#### NestedScrollView

&emsp;[NestedScrollView class](https://api.flutter.dev/flutter/widgets/NestedScrollView-class.html) 一个可以嵌套其它可滚动 widget 的 widget。

#### GridView

&emsp;[GridView class](https://api.flutter.dev/flutter/widgets/GridView-class.html) 一个可滚动的二维空间数组。

#### SingleChildScrollView

&emsp;[SingleChildScrollView class](https://api.flutter.dev/flutter/widgets/SingleChildScrollView-class.html) 有一个子 widget 的可滚动的 widget，子内容超过父容器时可以滚动。

#### Scrollable

&emsp;[Scrollable class](https://api.flutter.dev/flutter/widgets/Scrollable-class.html) 实现了可滚动 widget 的交互模型，但不包含 UI 显示相关的逻辑。

#### CustomScrollView

&emsp;[CustomScrollView class](https://api.flutter.dev/flutter/widgets/CustomScrollView-class.html) 一个使用 slivers 创建自定义的滚动效果的 ScrollView。

#### NotificationListener

&emsp;[NotificationListener<T extends Notification> class](https://api.flutter.dev/flutter/widgets/NotificationListener-class.html) 一个用来监听树上冒泡通知的 widget。

#### ScrollConfiguration

&emsp;[ScrollConfiguration class](https://api.flutter.dev/flutter/widgets/ScrollConfiguration-class.html) 控制可滚动组件在子树中的表现行为。

#### RefreshIndicator

&emsp;[RefreshIndicator class](https://api.flutter.dev/flutter/material/RefreshIndicator-class.html) Material Design 下拉刷新指示器，包装一个可滚动 widget。

```c++
ListView(
  physics: const AlwaysScrollableScrollPhysics(),
  children: ...
)
```

### 辅助功能 Widget

&emsp;给你的 App 添加辅助功能(这是一个正在进行的工作)。

#### Semantics

&emsp;[Semantics class](https://api.flutter.dev/flutter/widgets/Semantics-class.html) 一个 widget，用以描述 widget 树的具体语义。使用辅助工具、搜索引擎和其他语义分析软件来确定应用程序的含义。

#### MergeSemantics

&emsp;[MergeSemantics class](https://api.flutter.dev/flutter/widgets/MergeSemantics-class.html) 合并其后代语义的 widget。

```c++
MergeSemantics(
  child: Row(
    children: <Widget>[
      Checkbox(
        value: true,
        onChanged: (bool? value) {},
      ),
      const Text('Settings'),
    ],
  ),
)
```

#### ExcludeSemantics

&emsp;[ExcludeSemantics class](https://api.flutter.dev/flutter/widgets/ExcludeSemantics-class.html) 删除其后代所有语义的 widget。

## 参考链接
**参考链接:🔗**
+ [Widgets 目录](https://flutterchina.club/widgets/)
