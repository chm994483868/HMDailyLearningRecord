# Flutter 学习笔记（三）：Widgets 目录（2）

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

#### 

&emsp;[]() 

#### 

&emsp;[]() 

#### 

&emsp;[]() 

#### 

&emsp;[]() 

#### 

&emsp;[]() 

#### 

&emsp;[]() 

#### 

&emsp;[]() 

#### 

&emsp;[]() 

#### 

&emsp;[]() 

#### 

&emsp;[]() 

#### 

&emsp;[]() 

#### 

&emsp;[]() 

#### 

&emsp;[]() 











## 参考链接
**参考链接:🔗**
+ [Widgets 目录](https://flutterchina.club/widgets/)
