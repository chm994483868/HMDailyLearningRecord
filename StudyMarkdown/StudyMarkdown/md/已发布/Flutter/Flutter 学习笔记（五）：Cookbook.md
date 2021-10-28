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
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    const title = 'Fade in images';

    return MaterialApp(
      title: title,
      home: Scaffold(
        appBar: AppBar(
          title: const Text(title),
        ),
        body: Stack(
          children: <Widget>[
            const Center(child: CircularProgressIndicator()),
            Center(
              child: FadeInImage.memoryNetwork(
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

&emsp;在某些情况下，在从网上下载图片后缓存图片可能会很方便，以便它们可以脱机使用。为此，我们可以使用 cached_network_image 包来达到目的。

&emsp;除了缓存之外，cached_image_network 包在加载时还支持占位符和淡入淡出图片。

```c++
new CachedNetworkImage(
  imageUrl:'https://github.com/flutter/website/blob/master/_includes/code/layout/lakes/images/lake.jpg?raw=true',
);
```

##### 添加一个占位符

&emsp;cache_network_image 包允许我们使用任何 Widget 作为占位符！在这个例子中，我们将在图片加载时显示一个进度圈。

```c++
new CachedNetworkImage(
  placeholder: new CircularProgressIndicator(),
  imageUrl: 'https://github.com/flutter/website/blob/master/_includes/code/layout/lakes/images/lake.jpg?raw=true',
);
```

##### 完整的例子

&emsp;同样，我们需要先引入 `cached_network_image`。首先在 [pub.dartlang.org](https://pub.dev/flutter/packages) 搜索 `cached_network_image` 软件包（看到其当前版本是：3.1.0），然后在 pubspec.yaml 文件中引入 `cached_network_image: ^3.1.0`。

```c++
import 'package:flutter/material.dart';
import 'package:cached_network_image/cached_network_image.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    const title = 'Cached Images';

    return MaterialApp(
      title: title,
      home: Scaffold(
        appBar: AppBar(
          title: const Text(title),
        ),
        body: Stack(
          children: <Widget>[
            const Center(child: CircularProgressIndicator()),
            Center(
              child: CachedNetworkImage(
                imageUrl: 'https://picsum.photos/250?image=9',
              ),
            ),
          ]
        ),
      ),
    );
  }
}
```

## 创建一个基本 list

&emsp;显示数据列表是移动应用程序常见的需求。Flutter 包含的 ListView Widget，使列表变得轻而易举！

### 创建一个 ListView

&emsp;使用标准 ListView 构造函数非常适合仅包含少量条目的列表。我们使用内置的 ListTile Widget 来作为列表项。

```c++
import 'package:flutter/material.dart';

void main() => runApp(const MyApp());

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    const title = 'Basic List';

    return MaterialApp(
        title: title,
        home: Scaffold(
            appBar: AppBar(
              title: const Text(title),
            ),
            body: ListView(children: const <Widget>[
              ListTile(
                leading: Icon(Icons.map),
                title: Text('Maps'),
              ),
              ListTile(
                leading: Icon(Icons.photo_album),
                title: Text('Album'),
              ),
              ListTile(
                leading: Icon(Icons.phone),
                title: Text('Phone'),
              ),
            ])));
  }
}
```

## 创建一个水平 list

&emsp;有时，你可能想要创建一个水平滚动（而不是垂直滚动）的列表，ListView 本身就支持水平 list。在创建 ListView 时，设置 scrollDirection 为水平方向以覆盖默认的垂直方向。

```c++
import 'package:flutter/material.dart';

void main() => runApp(const MyApp());

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    const title = 'Horizontal List';

    return MaterialApp(
      title: title,
      home: Scaffold(
        appBar: AppBar(
          title: const Text(title),
        ),
        body: Container(
          margin: const EdgeInsets.symmetric(vertical: 20.0),
          height: 200.0,
          child: ListView(
            scrollDirection: Axis.horizontal,
            children: <Widget>[
              Container(
                width: 160.0,
                color: Colors.red,
              ),
              Container(
                width: 160.0,
                color: Colors.blue,
              ),
              Container(
                width: 160.0,
                color: Colors.green,
              ),
              Container(
                width: 160.0,
                color: Colors.yellow,
              ),
              Container(
                width: 160.0,
                color: Colors.orange,
              ),
            ],
          ),
        ),
      ),
    );
  }
}
```

## 使用长列表

&emsp;标准的 ListView 构造函数适用于小列表。为了处理包含大量数据的列表，最好使用 ListView.builder 构造函数。

&emsp;ListView 的构造函数需要一次创建所有项目，但 ListView.builder 的构造函数不需要，它将在列表项滚动到屏幕上时创建该列表项。

### 1. 创建一个数据源

&emsp;首先我们需要一个数据源，例如，你的数据源可能是消息列表、搜索结果或商店中的产品。大多数情况下，这些数据将来自互联网或数据库。

&emsp;在这个例子中，我们将使用 List.generate 构造函数生成拥有 10000 个字符串的列表。

```c++
final items = new List<String>.generate(10000, (i) => "Item $i");
```

### 2. 将数据源转换成 Widgets

&emsp;为了显示我们的字符串列表，我们需要将每个字符串展现为一个 Widget!

&emsp;这正是 ListView.builder 发挥作用的地方。在我们的例子中，我们将每一行显示一个字符串：

```c++
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';

void main() {
  runApp(MyApp(
    items: List<String>.generate(10000, (index) => "Item $index"),
  ));
}

class MyApp extends StatelessWidget {
  final List<String> items;

  const MyApp({Key? key, required this.items}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    const title = 'Long List';

    return MaterialApp(
      title: title,
      home: Scaffold(
        appBar: AppBar(
          title: const Text(title),
        ),
        body: ListView.builder(
          itemCount: items.length,
          itemBuilder: (context, index) {
            return ListTile(
              title: Text(items[index]),
            );
          },
        ),
      ),
    );
  }
}
```

## 使用不同类型的子项创建列表

&emsp;我们经常需要创建显示不同类型内容的列表。例如，我们可能正在制作一个列表，其中显示一个标题，后面跟着与该标题相关的几个子项，再后面是另一个标题，等等。

&emsp;如何用 Flutter 创建这样的结构呢？

### 步骤

1. 使用不同类型的数据创建数据源。
2. 将数据源转换为 Widgets 列表。

#### 1. 使用不同类型的数据创建数据源

&emsp;条目（子项）类型

&emsp;为了表示列表中的不同类型的条目，我们需要为每个类型的条目定义一个类。

&emsp;在这个例子中，我们将在一个应用程序上显示一个标题，后面跟着五条消息。因此，我们将创建三个类：ListItem、HeadingItem、和 MessageItem。

&emsp;创建 Item 列表

&emsp;大多数时候，我们会从互联网或本地数据库中读取数据，并将该数据转换成 item 的列表。

&emsp;对于这个例子，我们将生成一个 Item 列表来处理。该列表将包含一个标题、后五条消息，然后重复。

#### 2. 将数据源转换为 Widgets 列表

&emsp;为了将每个 item 转换为 Widget，我们将使用 ListView.builder 构造函数。

&emsp;通常，我们需要提供一个 builder 函数来检查我们正在处理的 item 类型，并返回该 item 类型对应的 Widget。

&emsp;在这个例子中，使用 is 关键字来检查我们正在处理的 item 的类型，这个速度很快，并会自动将每个 item 转换为适当的类型。但是，如果你更喜欢另一种模式，也有不同的方法可以解决这个问题！

```c++
import 'package:flutter/material.dart';
import 'package:flutter/foundation.dart';

void main() {
  runApp(MyApp(
    items: List<ListItem>.generate(
      1000,
      (index) => index % 6 == 0
          ? HeadingItem("Heading $index")
          : MessageItem("Sender $index", "Message body $index"),
    ),
  ));
}

class MyApp extends StatelessWidget {
  final List<ListItem> items;

  const MyApp({Key? key, required this.items}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    const title = "Mixed List";

    return MaterialApp(
      title: title,
      home: Scaffold(
        appBar: AppBar(
          title: const Text(title),
        ),
        body: ListView.builder(
          // Let the ListView know how many items it needs to build
          itemCount: items.length,
          // Provide a builder function. This is where the magic happens! We'll convert each item into a Widget based on the type of item it is.
          itemBuilder: (context, index) {
            final item = items[index];

            if (item is HeadingItem) {
              return ListTile(
                title: Text(
                  item.heading,
                  style: Theme.of(context).textTheme.headline3,
                ),
              );
            } else if (item is MessageItem) {
              return ListTile(
                title: Text(item.sender),
                subtitle: Text(item.body),
              );
            } else {
              return const ListTile(
                title: Text("PLACEHOLDER")
              );
            }
          },
        ),
      ),
    );
  }
}

// The base class for the different types of items the List can contain
abstract class ListItem {}

// A ListItem that contains data to display a heading
class HeadingItem implements ListItem {
  final String heading;

  HeadingItem(this.heading);
}

// A ListItem that contains data to display a message
class MessageItem implements ListItem {
  final String sender;
  final String body;

  MessageItem(this.sender, this.body);
}
```

## 创建一个 Grid List

&emsp;在某些情况下，你可能希望将 item 显示为网格，而不是一个普通列表。对于这个需求，我们可以使用 GridView Widget。

&emsp;使用网格的最简单方法是使用 GridView.count 构造函数，它允许我们指定行数和列数。

&emsp;在这个例子中，我们将生成一个包含 100 个 Widget 的 list，在网格中显示它们的索引。这将帮助我们观察 GridView 如何工作。

```c++
import 'package:flutter/material.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    const title = 'Grid List';

    return MaterialApp(
      title: title,
      home: Scaffold(
        appBar: AppBar(
          title: const Text(title),
        ),
        body: GridView.count(
          crossAxisCount: 3,
          children: List.generate(100, (index) {
            return Center(
              child: Text(
                'Item $index',
                style: Theme.of(context).textTheme.headline6,
              ),
            );
          }),
        ),
      )
    );
  }
}
```

## 处理点击

&emsp;我们不仅希望向用户展示信息，还希望我们的用户与我们的应用互动，那么如何响应用户的基本操作，如何点击和拖动？在 Flutter 中我们可以使用 GestureDetector Widget。假设我们想要创建一个自定义按钮，当点击时显示一个 SnackBar。如何解决这个问题？

### 步骤

1. 创建一个 button。
2. 把它包装在 GestureDetector 中并提供一个 onTap 回调。

### 注意

1. 虽然我们已经创建了一个自定义按钮来演示这些概念，但 Flutter 也提供了一些其它开箱即用的按钮：RaisedButton、FlatButton、CupertinoButton。

```c++
import 'package:flutter/material.dart';

void main() => runApp(const MyApp());

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    const title = 'Gesture Demo';

    return const MaterialApp(
      title: title,
      home: MyHomePage(title: title),
    );
  }
}

class MyHomePage extends StatelessWidget {
  final String title;

  const MyHomePage({Key? key, required this.title}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(title),
      ),
      body: const Center(child: MyButton()),
    );
  }
}

class MyButton extends StatelessWidget {
  const MyButton({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    // Our GestureDetector wraps our button
    return GestureDetector(
      // When the child is tapped, show a snackbar
      onTap: () {
        const snackBar = SnackBar(content: Text("Tap"));

        // ignore: deprecated_member_use
        Scaffold.of(context).showSnackBar(snackBar);
      },
      // Our Custom Button!
      child: Container(
        padding: const EdgeInsets.all(12.0),
        decoration: BoxDecoration(
          // ignore: deprecated_member_use
          color: Theme.of(context).buttonColor,
          borderRadius: BorderRadius.circular(8.0),
        ),
        child: const Text('My Button'),
      ),
    );
  }
}
```

## 添加 Material 触摸水波效果

&emsp;在设计应遵循 Material Design 指南的应用程序时，我们希望在点击时将水波动画添加到 Widgets。

&emsp;Flutter 提供了 InkWell Widget 来实现这个效果。

### 步骤

1. 创建一个可点击的 Widget。
2. 将它包裹在一个 InkWell 中来管理点击回调和水波动画。

```c++
import 'package:flutter/material.dart';

void main() => runApp(const MyApp());

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    const title = 'InkWell Demo';

    return const MaterialApp(
      title: title,
      home: MyHomePage(title: title),
    );
  }
}

class MyHomePage extends StatelessWidget {
  final String title;

  const MyHomePage({Key? key, required this.title}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(title),
      ),
      body: const Center(child: MyButton()),
    );
  }
}

class MyButton extends StatelessWidget {
  const MyButton({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return InkWell(
      onTap: () {
        // ignore: deprecated_member_use
        Scaffold.of(context).showSnackBar(const SnackBar(
          content: Text('Tap'),
        ));
      },
      child: Container(
        padding: const EdgeInsets.all(12.0),
        child: const Text('Flat Button'),
      ),
    );
  }
}
```

## 实现滑动关闭

&emsp;"滑动删除" 模式在移动应用中很常见。例如，如果我们正在编写一个电子邮件应用程序，我们希望允许我们的用户在列表中滑动电子邮件。当我们这样做时，我们需要将条目从收件箱移至垃圾箱。

&emsp;Flutter 通过提供 Dismissable Widget 使这项任务变的简单。

### 步骤

1. 创建 item 列表。
2. 将每个 item 包装在一个 Dismissable Widget 中。
3. 提供滑动背景提示。

#### 1. 创建 item 列表

&emsp;第一步是创建一个我们可以滑动的列表。

##### 创建数据源

&emsp;在我们的例子中，我们需要 20 个条目。为了简单起见，我们将生成一个字符串列表。

```c++
final items = new List<String>.generate(20, (i) => "Item ${i + 1}");
```

##### 将数据源转换为 List

&emsp;首先，我们将简单地在屏幕上的列表中显示每个项目（先不支持滑动）。

#### 2. 将每个 item 包装在 Dismissible Widget 中

&emsp;现在我们希望让用户能够将条目从列表中移除，用户删除一个条目后，我们需要从列表中删除该条目并显示一个 Snackbar。在实际的场景中，你可能需要执行更复杂的逻辑，例如从 Web 服务或数据库中删除条目。‘

&emsp;这时我们就可以使用 Dismissable。在下面的例子中，我们将更新 itemBuilder 函数以返回一个 Dismissable Widget。

#### 3. 提供滑动背景提示

&emsp;现在，我们的应用程序将允许用户从列表中滑动项目，但用户并不知道滑动后做了什么，所以，我们需要告诉用户滑动操作会移除条目。为此，我们将在滑动条目时显示提示。在下面的例子中，我们通过将背景设置为红色表示删除操作。

&emsp;为此，我们为 Dismissable 提供一个 background 参数。

&emsp;完整的例子：

```c++
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';

void main() {
  runApp(MyApp(
    items: List<String>.generate(20, (index) => "Item ${index + 1}"),
  ));
}

class MyApp extends StatelessWidget {
  final List<String> items;

  const MyApp({Key? key, required this.items}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    const title = 'Dismissing Items';

    return MaterialApp(
      title: title,
      home: Scaffold(
        appBar: AppBar(
          title: const Text(title),
        ),
        body: ListView.builder(
          itemCount: items.length,
          itemBuilder: (context, index) {
            final item = items[index];

            return Dismissible(
              // Each Dismissible must contain a Key. Keys allow Flutter to uniquely identify Widgets.
              key: Key(item),
              onDismissed: (direction) {
                items.removeAt(index);

                // ignore: deprecated_member_use
                Scaffold.of(context).showSnackBar(SnackBar(content: Text("$item dismissed")));
              },
              // Show a red background as the item is swiped away
              background: Container(color: Colors.red),
              child: ListTile(title: Text(item)),
            );
          },
        ),
      ),
    );
  }
}
```

## 导航到新页面并返回

&emsp;大多数应用程序包含多个页面。例如，我们可能有一个显示产品的页面，然后，用户可以点击产品，跳到该产品的详情页。

&emsp;在 Android 中，页面对应的是 Activity，在 iOS 中是 ViewController。而在 Flutter 中，页面只是一个 Widget！在 Flutter 中，我们可以使用 Navigator 在页面之间跳转。

### 步骤

1. 创建两个页面。
2. 调用 Navigator.push 导航到第二个页面。
3. 调用 Navigator.pop 返回第一个页面。

#### 1. 创建两个页面

&emsp;我们创建两个页面，每个页面包含一个按钮。点击第一个页面上的按钮将导航到第二个页面上。点击第二个页面上的按钮将返回到第一个页面上。

#### 2. 调用 Navigator.push 导航到第二个页面

&emsp;为了导航到新的页面，我们需要调用 Navigator.push 方法。该 push 方法将添加 Route 到由导航器管理的路由栈中！该 push 方法需要一个 Route，但 Route 从哪里来？我们可以创建自己的，或直接使用 MaterialPageRoute。MaterialPageRoute 很方便，因为它使用平台特定的动画跳转到新的页面（Android 和 iOS 屏幕切换动画会不同）。在 FirstScreen Widget 的 Build 方法中，我们添加 onPressed 回调。

#### 3. 调用 Navigator.pop 返回第一个页面

&emsp;现在我们在第二个屏幕上，我们如何关闭它并返回到第一个屏幕？使用 Navigator.pop 方法！该 pop 方法将 Route 从导航器管理的路由栈中移除当前路径。 

```c++
import 'package:flutter/material.dart';

void main() {
  runApp(const MaterialApp(
    title: 'Navigation Basics',
    home: FirstScreen(),
  ));
}

class FirstScreen extends StatelessWidget {
  const FirstScreen({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text("First Screen"),),
      body: Center(
        // ignore: deprecated_member_use
        child: RaisedButton(
          child: const Text("Launch new Screen"),
          onPressed: () {
            Navigator.push(
              context, 
              MaterialPageRoute(builder: (context) => const SecondScreen()),
            );
          },
        ),
      ),
    );
  }
}

class SecondScreen extends StatelessWidget {
  const SecondScreen({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text("Second Screen"),
        ),
        body: Center(
          // ignore: deprecated_member_use
          child: RaisedButton(
            onPressed: () {
              Navigator.pop(context);
            },
            child: const Text("Go back!"),
          ),
        ),
      );
  }
}
```

## 给新页面传值

&emsp;通常，我们不仅要导航到新的页面，还要将一些数据传给页面。例如，我们想要传一些关于我们点击的条目的信息。请记住：页面只是 Widgets，在这个例子中，我们将创建一个 Todos 列表。当点击一个 todo 时，我们将导航到一个显示关于待办事项信息到新页面（Widget）。

### Directions

1. 定义一个 Todo 类。
2. 显示 Todos 列表。
3. 创建一个显示待办事项详情的页面。
4. 导航并将数据传递到详情页。

#### 1. 定义一个 Todo 类

&emsp;首先，我们需要一种简单的方法来表示 Todos（待办事项）。在这个例子中，我们将创建一个包含两部分数据的类：标题和描述。

#### 2. 创建一个 Todos 列表

&emsp;其次，我们要显示一个 Todos 列表。在这个例子中，我们将生成 20 个待办事项并使用 ListView 显示它们。

#### 3. 创建一个显示待办事项（todo）详情的页面

&emsp;现在，我们将创建我们的第二个页面。页面的标题将包含待办事项的标题，页面正文将显示说明。由于这是一个普通的 StatelessWidget，我们只需要在创建页面时传递一个 Todo！然后，我们将使用给定的 Todo 来构建新的页面。

#### 4. 导航并将数据传递到详情页

&emsp;接下来，当用户点击我们列表中的待办事项时我们将导航到 DetailScreen，并将 Todo 传递给 DetailScreen。为了实现这一点，我们将实现 ListTile 的 onTap 回调。在我们的 onTap 回调中，我们将再次调用 Navigator.push 方法。

```c++
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';

class Todo {
  final String title;
  final String description;

  Todo(this.title, this.description);
}

void main() {
  runApp(MaterialApp(
    title: 'Passing Data',
    home: TodosScreen(
      todos: List.generate(
        20,
        (i) => Todo(
          'Todo $i',
          'A description of what needs to be done for Todo $i',
        ),
      ),
    ),
  ));
}

class TodosScreen extends StatelessWidget {
  final List<Todo> todos;

  const TodosScreen({Key? key, required this.todos}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Todos'),
      ),
      body: ListView.builder(
        itemCount: todos.length,
        itemBuilder: (context, index) {
          return ListTile(
            title: Text(todos[index].title),
            onTap: () {
              Navigator.push(
                context,
                MaterialPageRoute(
                  builder: (context) => DetailScreen(todo: todos[index]),
                ),
              );
            },
          );
        },
      ),
    );
  }
}

class DetailScreen extends StatelessWidget {
  final Todo todo;

  const DetailScreen({Key? key, required this.todo}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(todo.title),
      ),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Text(todo.description),
      ),
    );
  }
}
```

## 从新页面返回数据给上一个页面

&emsp;在某些情况下，我们可能想要从新页面返回数据。例如，假设我们导航到一个新页面，向用户呈现两个选项。当用户点击某个选项时，我们需要将用户选择通知给第一个页面，以便它能够处理这些信息！

&emsp;我们如何实现？使用 Navigator.pop！

### 步骤

1. 定义主页。
2. 添加一个打开选择页面的按钮。
3. 在选择页面上显示两个按钮。
4. 点击一个按钮时，关闭选择的页面。
5. 主页上弹出一个 snackbar 以显示用户的选择。

#### 1. 定义主页

&emsp;主页将显示一个按钮，点击后，它将打开选择页面！

#### 2. 添加一个打开选择页面的按钮

&emsp;现在，我们将创建我们的 SelectionButton。我们的选择按钮将会：

1. 点击时启动 SelectionScreen
2. 等待 SelectionScreen 返回结果

#### 3. 在选择页面上显示两个按钮

&emsp;现在，我们需要构建一个选择页面！它将包含两个按钮。当用户点击按钮时，应该关闭选择页面并让主页知道哪个按钮被点击！现在我们将定义 UI，并确定如何在下一步中返回数据。

#### 4. 点击一个按钮时，关闭选择的页面

&emsp;现在，我们完成两个按钮的 onPressed 回调。为了将数据返回到第一个页面，我们需要使用 Navigator.pop 方法。

&emsp;Navigator.pop 接受一个可选的（第二个）参数 result。如果我们返回结果，它将返回到一个 Future 到主页的 SelectionButton 中。

#### 5. 主页上弹出一个 snackbar 以显示用户的选择

&emsp;既然我们正在启动一个选择页面并等待结果，那么我们会想要对返回的信息进行一些操作。

&emsp;在这种情况下，我们将显示一个显示结果的 Snackbar。为此，我们将更新 SelectionButton 中的 _navigateAndDisplaySelection 方法。

```c++
import 'package:flutter/material.dart';

void main() {
  runApp(const MaterialApp(
    title: 'Returning Data',
    home: HomeScreen(),
  ));
}

class HomeScreen extends StatelessWidget {
  const HomeScreen({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Returning Data Demo'),
      ),
      body: const Center(child: SelectionButton()),
    );
  }
}

class SelectionButton extends StatelessWidget {
  const SelectionButton({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    // ignore: deprecated_member_use
    return RaisedButton(
      onPressed: () {
        _navigateAndDisplaySelection(context);
      },
      child: const Text('Pick an option, any option!'),
    );
  }

  // A method that launches the SelectionScreen and awaits the result from Navigator.pop!
  _navigateAndDisplaySelection(BuildContext context) async {
    // Navigator.push returns a Future that will complete after we call Navigator.pop on the Selection Screen!
    final result = await Navigator.push(
      context,
      MaterialPageRoute(builder: (context) => const SelectionScreen()),
    );

    // After the Selection Screen returns a result, show it in a Snackbar!
    // ignore: deprecated_member_use
    Scaffold.of(context).showSnackBar(SnackBar(content: Text("$result")));
  }
}

class SelectionScreen extends StatelessWidget {
  const SelectionScreen({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Pick an option'),
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            Padding(
              padding: const EdgeInsets.all(8.0),
              // ignore: deprecated_member_use
              child: RaisedButton(
                onPressed: () {
                  // Close the screen and return "Yep!" as the result
                  Navigator.pop(context, 'Yep!');
                },
                child: const Text('Yep!'),
              ),
            ),
            Padding(
              padding: const EdgeInsets.all(8.0),
              // ignore: deprecated_member_use
              child: RaisedButton(
                onPressed: () {
                  // Close the screen and return "Nope!" as the result
                  Navigator.pop(context, 'Nope.');
                },
                child: const Text('Nope.'),
              ),
            ),
          ],
        ),
      ),
    );
  }
}
``` 

## 从互联网上获取数据

&emsp;从大多数应用程序都需要从互联网上获取数据，Dart 和 Flutter 提供了相关工具！

> &emsp;官方文档示例使用的是 http package 发起简单的网路请求，但是 http package 功能较弱，很多常用功能都不支持。建议使用 [dio](https://github.com/flutterchina/dio) 来发起网络请求，它是一个强大易用的 dart http 请求库，支持 Restful API、FormData、拦截器、请求取消、Cookie 管理、文件上传/下载...


### 步骤

1. 添加 `http` package 依赖
2. 使用 `http` package 发出网请求
3. 将响应转为自定义的 Dart 对象
4. 获取并显示数据

#### 1. 添加 http package

&emsp;[http](https://pub.dartlang.org/packages/http) package 提供了从互联网获取数据的最简单方法。

```c++
dependencies:
  http: <latest_version>
```

#### 2. 发起网络请求

&emsp;在这个例子中，我们将使用 http.get 从 






























## 参考链接
**参考链接:🔗**
+ [Widgets 目录](https://flutterchina.club/widgets/)
+ [Cookbook](https://flutterchina.club/cookbook/)
