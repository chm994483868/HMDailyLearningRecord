#  Flutter 学习笔记（三）：Widgets 目录

&emsp;使用 Flutter 的一套的视觉、结构、平台、和交互式的 widgets，快速创建漂亮的 APP。

## 基础组件

&emsp;在构建 Flutter 应用程序之前，绝对要了解的 widgets。下面大概是一大组类似 iOS 中官方提供的 UI 控件，下面一起看一下吧。  

### Container

&emsp;[Container class](https://api.flutter.dev/flutter/widgets/Container-class.html) 将 common 绘制、定位和大小 widget 相结合的便利 widget。

&emsp;你是否有 widget 需要一些背景样式？也许是背景颜色或形状？还是一些尺寸限制？尝试将其包装在 Container widget 中。Container widget 可帮助你组成、装饰和定位子 widget。如果将 widget 包装在没有任何其他参数的 Container 小部件中，则不会发现外观上的任何差异。但是，如果添加 Color 参数，则你的子窗口 widget 将获得背景色。没有任何其他内容，Container 将根据其子 widget 自行调整大小。使用 Container 的 padding 属性在子 widget 和 Container 边界之间添加空白，并使用 margin 属性添加围绕 widget 的空白。使用 Decoration 属性可以在 Container 中添加一个形状，例如圆形。默认情况下 decoration 的大小是根据 Container 的*子项。在这种情况下，Container 对准圆形装饰最窄的参数--文本 widget 的高度。你可以像以前一样使用 padding 和 margin 来设计 decoration。使用 Alignment 属性，可以使子 widget 在 container 中对齐。设置对齐方式后，container 将展开以填充其父级的宽度和高度。你可以通过设置 container 的宽度和高度属性。或使用箱式布局模型来覆盖此设置。例如，使用 BoxConstraints，你的 container 可以扩展以填充给定的大小。你甚至可以将 transform 应用于 container。 

```c++
Container(
  child: Text('Less boring'),
  decoration: BoxDecoration(
    shape: BoxShape.circle,
    color: Colors.blue,
  ),
  margin: EdgeInsets.all(25.0),
  padding: EdgeInsets.all(40.0),
  alignment: Alignment.center,
  width: 200,
  height: 100,
  constraints: BoxConstraints.tightForFinite(
    width: 200,
  ),
  transform: Matrix4.rotationZ(0.05),
);
```

### Row

&emsp;[Row class](https://api.flutter.dev/flutter/widgets/Row-class.html) 在水平阵列（horizontal array）中显示其子项的 widget（在水平方向上排列子 widget 的列表）。要使子项扩展以填充可用的水平空间，请将子项包裹在 Expanded widget 中。Row widget 不会滚动（通常，如果 Row 中的子项多于 available room 的容量，则被认为是错误的）。如果你有 Row widget 并希望它们能够在空间不足的情况下滚动，请考虑使用 ListView。

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

### Column

&emsp;[Column class](https://api.flutter.dev/flutter/widgets/Column-class.html) 在垂直阵列（vertical array）中显示其子项的 widget（在垂直方向上排列子 widget 的列表）。要使子项扩展以填充可用的垂直空间，请将子项包裹在 Expanded widget 中。Column widget 不会滚动（并且通常认为 Column 中的子项多于 available room 的数量是错误的）。如果你有一行 widget 并希望它们能够在空间不足的情况下滚动，请考虑使用 ListView。

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

### Image

&emsp;[Image class](https://api.flutter.dev/flutter/widgets/Image-class.html) 显示图像的 widget。

&emsp;有时 App 需要显示图片，好在 Flutter 有个叫 Image 的 widget 正好派上用场。这 widget 能让图片显示于屏幕。Flutter 提取显示图像的方式有很多种。这图像可能来自你的 App 所保存的 Asset Bundle 使用 Image.asset，并提供 asset 名称。Image.asset 会根据设备的像素密度自动以适当版本显示文件。你只需要提供其他版本，并在 pubspec.yaml 加列。你也可以显示来自网络的图像。这得用上 Image.network()，并提供一个 URL。Flutter 将以加载缓存的方式显示图片。


```c++
Image.asset(
  'assets/images/dash.jpg',
)
```

```c++
flutter:
  assets:
    - images/dash.png
    - images/2x/dash.png
```

```c++
Image.network(
  'http://example.com/dash.jpg',
)
```

&emsp;为指定图像的各种方式提供了几个构造函数：

+ new Image，用于从 ImageProvider 获取图像。
+ new Image.asset，用于使用 key 从 AssetBundle 获取图像。
+ new Image.network，用于从 URL 获取图像。
+ new Image.file，用于从 file 中获取图像。
+ new Image.memory，用于从 Uint8List 获取图像。

&emsp;支持以下图像格式：JPEG、PNG、GIF、Animated GIF、WebP、Animated WebP、BMP 和 WBMP。底层平台（iOS/android）可能支持其他格式。 Flutter 会尝试调用平台 API 来解码无法识别的格式，如果平台 API 支持解码图像，Flutter 将能够渲染它。

&emsp;要自动执行 pixel-density-aware asset resolution，请使用 AssetImage 指定图像并确保 MaterialApp、WidgetsApp 或 MediaQuery widget 存在于 widget 树中的 Image widget 上方。

&emsp;图像是使用 paintImage 绘制的，它更详细地描述了这个类上各个字段的含义。

&emsp;默认构造函数可以与任何 ImageProvider 一起使用，例如 NetworkImage，以显示来自 Internet 的图像

```c++
const Image(
  image: NetworkImage('https://flutter.github.io/assets-for-api-docs/assets/widgets/owl.jpg'),
)
```

&emsp;为方便起见，Image Widget 还提供了几个构造函数来显示不同类型的图像。在此示例中，使用 Image.network 构造函数显示来自 Internet 的图像。

```c++
Image.network('https://flutter.github.io/assets-for-api-docs/assets/widgets/owl-2.jpg')
```

&emsp;Image.asset、Image.network、Image.file 和 Image.memory 构造函数允许通过 cacheWidth 和 cacheHeight 参数指定自定义解码大小。引擎会将图片解码到指定的大小，主要是为了减少 ImageCache 的内存使用。在 Web 平台上使用网络图片的情况下，cacheWidth 和 cacheHeight 参数会被忽略，因为 Web 引擎将网络图片的图片解码委托给 Web，不支持自定义解码大小。

### Text

## 参考链接
**参考链接:🔗**
+ [Widgets 目录](https://flutterchina.club/widgets/)
