# Flutter 学习笔记（五）：Cookbook

## 使用主题共享颜色和字体样式

&emsp;为了在整个应用中 **共享颜色和字体样式**，我们可以使用主题。定义主题有两种方式：**全局主题** 或 **使用 Theme 来定义应用程序局部的颜色和字体样式**。事实上，全局主题只是由应用程序根 MaterialApp 创建的 Theme。

&emsp;定义一个主题后，我们可以在我们自己的 Widgets 中使用它。另外，Flutter 提供的 Material Widgets 将使用我们的主题为 AppBars、Buttons、Checkboxes 等设置背景颜色和字体样式。

### 创建应用主题

&emsp;为了整个应用程序中共享包含颜色和字体样式的主题，我们可以提供 ThemeData 给 MatrialApp 的构造函数。如果没有提供 theme，Flutter 将创建一个默认主题。

```c++
new MatrialApp(
    title: title,
    theme: new ThemeData(
        brightness: Brightness.dark,
        primaryColor: Colors.lightBlue[800],
        accentColor: Colors.cyan[600],
    ),
);
```

### 局部主题

&emsp;如果我们想在应用程序的一部分中覆盖应用程序的全局的主题，我们可以将要覆盖的部分封装在一个 Theme Widget 中。有两种方法可以解决这个问题：创建特有的 ThemeData 或扩展父主题。

#### 创建特有的 ThemeData

&emsp;如果我们不想继承任何应用程序的颜色或字体样式，我们可以通过 new ThemeData() 创建一个实例并将其传递给 Theme Widget。

```c++
new Theme(
    // Create a unique theme with "new ThemeData"
    data: new ThemeData(
        accentColor: Colors.yellow,
    ),
    child: new FloatingActionButton(
        onPressed: () {},
        child: new Icon(Icons.add),
    ),
);
```

#### 扩展父主题

&emsp;扩展父主题时无需覆盖所有的主题属性，我们可以通过使用 copyWith 方法来实现。

```c++
new Theme(
    // Find and Extend the parent theme using "copyWith". Please see the next section for more info on `Theme.of`.
    data: Theme.of(context).copyWith(accentColor: Colors.yellow),
    child: new FloatingActionButton(
        onPressed: null,
        child: new Icon(Icons.add),
    ),
);
```

### 使用主题

&emsp;现在我们已经定义了一个主题，我们可以在 Widget 的 build 方法中通过 Theme.of(context) 函数使用它！

&emsp;Theme.of(context) 将查找 Widget 树并返回树中最近的 Theme。如果我们的 Widget 之上有一个单独的 Theme 定义，则返回该值。如果不是，则返回 App 主题。事实上，FloatingActionButton 真是通过这种方式找到 accentColor 的！

&emsp;下面是一个示例：

```c++
new Container(
    color: Theme.of(context).accentColor,
    child: new Text(
        'Text with a background color',
        style: Theme.of(context).textTheme.title,
    ),
);
```

### 完整的例子

```c++
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';

void main() {
  runApp(new MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    // TODO: implement build
    final appName = 'Custom Themes';

    return new MaterialApp(
      title: appName,
      theme: new ThemeData(
        brightness: Brightness.dark,
        primaryColor: Colors.lightBlue[800],
        accentColor: Colors.cyan[600],
      ),
      home: new MyHomePage(
        title: appName,
      ),
    );
  }
}

class MyHomePage extends StatelessWidget {
  final String title;

  MyHomePage({Key? key, required this.title}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    // TODO: implement build
    return new Scaffold(
      appBar: new AppBar(
        title: new Text(title),
      ),
      body: new Center(
        child: new Container(
          color: Theme.of(context).accentColor,
          child: new Text(
            'Text with a background color',
            style: Theme.of(context).textTheme.headline6,
          ),
        ),
      ),
      floatingActionButton: new Theme(
        data: Theme.of(context).copyWith(accentColor: Colors.yellow),
        child: new FloatingActionButton(
          onPressed: null,
          child: new Icon(Icons.add),
        ),
      ),
    );
  }
}
```




## 参考链接
**参考链接:🔗**
+ [Widgets 目录](https://flutterchina.club/widgets/)
