# Flutter 学习笔记（三）：Widgets 目录（2）

&emsp;使用 Flutter 的一套的视觉、结构、平台、和交互式的 widgets，快速创建漂亮的 APP。

## Cupertino(iOS 风格的 widget)

&emsp;用于当前 iOS 设计语言的美丽和高保真 widget。（它们用了 Cupertino 库比提诺 作为前缀，它是美国一座城市的名字。）

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

&emsp;[]() 按自己的大小调整其子 widget 的大小和位置。


















## 参考链接
**参考链接:🔗**
+ [Widgets 目录](https://flutterchina.club/widgets/)
