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

&emsp;







## 参考链接
**参考链接:🔗**
+ [Mac pro 找不到zshrc文件](https://www.jianshu.com/p/6e9d776836ab)
+ [编写您的第一个 Flutter App](https://flutterchina.club/get-started/codelab/)
+ [Libraries and visibility](https://dart.dev/guides/language/language-tour#libraries-and-visibility)



+ [深入浅出 Flutter Framework 之 Widget](https://juejin.cn/post/6844904152905023496)
