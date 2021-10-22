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

## 显示来自网上的图片

&emsp;显示图片是大多数移动应用程序的基础。Flutter 提供了 Image Widget 来显示不同类型的图片。

&emsp;为了处理来自 URL 的图像，请使用 Image.network 构造函数。

```c++
new Image.network(
  'https://raw.githubusercontent.com/flutter/website/master/_includes/code/layout/lakes/images/lake.jpg',
)
```

### Bonus: GIF 动画

&emsp;Image Widget 的一个惊艳的功能是：支持 GIF 动画！

```c++
new Image.network(
  'https://github.com/flutter/plugins/raw/master/packages/video_player/doc/demo_ipod.gif?raw=true',
)
```

### 占位图和缓存

&emsp;Image.network 默认不能处理一些高级功能，例如在下载完图片后加载或缓存图片到设备中后，使图片渐隐渐显。

#### 用占位符淡入图片

&emsp;当使用默认 Image widget 显示图片时，可能会注意到它们在加载完成后会直接显示到屏幕上。这可能会让用户产生视觉突兀。相反，如果最初显示一个占位图，然后在图像加载完显示时淡入，那么它会不会更好？我们可以使用 FadeInImage 来达到这个目的。

&emsp;FadeInImage 适用于任何类型的图片：内存、本地 Asset 或来自网上的图片。

&emsp;在下面的例子中，我们将使用 transparent_image 包作为一个简单的透明占位图。你也可以考虑按照 Assets 和图片指南使用本地资源来做占位图。

```c++
new FadeImage.memoryNetwork(
  placeholder: kTransparentImage,
  image: '',
);
```

&emsp;完整的例子:

&emsp;首先需要在 [pub.dartlang.org](https://pub.dev/flutter/packages) 搜索 `transparent_image` 软件包（看到其当前版本是：2.0.0），然后在 pubspec.yaml 文件中引入 `transparent_image: ^2.0.0`。

```c++
import 'package:flutter/material.dart';
import 'package:transparent_image/transparent_image.dart';

void main() {
  runApp(new MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    // TODO: implement build
    final title = 'Fade in images';

    return new MaterialApp(
      title: title,
      home: new Scaffold(
        appBar: new AppBar(
          title: new Text(title),
        ),
        body: new Stack(
          children: <Widget>[
            new Center(child: new CircularProgressIndicator()),
            new Center(
              child: new FadeInImage.memoryNetwork(
                placeholder: kTransparentImage,
                image: 'https://picsum.photos/250?image=9',
              ),
            )
          ]
        )
      )
    );
  }
}
```

#### 使用缓存图片






## 参考链接
**参考链接:🔗**
+ [Widgets 目录](https://flutterchina.club/widgets/)
