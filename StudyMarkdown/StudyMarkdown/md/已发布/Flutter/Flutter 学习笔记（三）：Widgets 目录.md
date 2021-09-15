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

&emsp;有时 App 需要显示图片，好在 Flutter 有个叫 Image 的 widget 正好派上用场。这 widget 能让图片显示于屏幕。Flutter 提取显示图像的方式有很多种。这图像可能来自你的 App 所保存的 Asset Bundle 使用 Image.asset，并提供 asset 名称。Image.asset 会根据设备的像素密度自动以适当版本显示文件。你只需要提供其他版本，并在 pubspec.yaml 加列。你也可以显示来自网络的图像。这得用上 Image.network()，并提供一个 URL。Flutter 将以加载缓存的方式显示图片（Web images are cached  automatically）。请记住，网络图片的加载速度，可能不及 Asset 中的图片且用户得联网才能看到初始图像。你可提交 loadingBuilder 让框架反复调用以便从网络下载图片。loadingBuilder 内含下载进度参数 progress，你可用来让用户知道实际图像还要多久才会下载完毕并显示。你还可以选择显示来自用户设备的图档，这部分可利用 Image.file，而且以字节数组存于内存的图像，也可藉此显示于屏幕，用 Image.memory 就可办到。无论图像档来源为何，都可用参数加以配置。设定属性 width and/or height，便能确定图像的显示规格，且能避免加载布局时出现难看的抖动。设置图片的 fit 属性以分配其刻入布局的空间。默认选项为 contain 但还有其他选项，包括可拉伸图片的 fill，而 cover 就是不拉伸。你还可以使用 color 和 colorBlendMode 为图像来套色（类似原生的 tint color）。有很多选择。别忘了为图像加上语义化标签 semanticLabel。这些将显示在辅助技术中，并使你的应用更易于访问。Flutter 支持 JPEG、PNG、GIF、WebP、位图和 WBMP。你也可以显示 GIF 和 WebP 动画。

```c++
Image.network(
  'https://example.com/dash.jpg',
  loadingBuilder: (context, child, progress) {
    return progress == null
        ? child
        : LinearProgressIndicator(...);
  },
)
```

```c++
Image.file(
  '/path/to/dash.jpg',
)
```

```c++
Image.memory(
  myUint8List,
)
```

```c++
Image.asset(
  'assets/images/dash.jpg',
  width: 200,
  height: 400,
  fit: BoxFit.contain,
  color: Colors.red,
  colorBlendMode: BlendMode.darken,
  semanticLabel: 'Dash mascot',
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

&emsp;[Text class](https://api.flutter.dev/flutter/widgets/Text-class.html) 单一格式的文本。Text widget 显示具有单一样式的文本字符串。根据布局约束，字符串可能会跨越多行，也可能全部显示在同一行上。style 参数是可选的。省略时，文本将使用最近的 enclosing DefaultTextStyle 中的样式。如果给定 style 的 TextStyle.inherit 属性为 true（默认值），则给定 style 将与最接近的 enclosing DefaultTextStyle 合并。这种合并行为很有用，例如，在使用默认字体系列和大小时使文本加粗。

```c++
Text(
  'Hello, $_name! How are you?',
  textAlign: TextAlign.center,
  overflow: TextOverflow.ellipsis,
  style: const TextStyle(fontWeight: FontWeight.bold),
)
```

&emsp;使用 Text.rich 构造函数，Text widget 可以显示具有不同样式 TextSpans 的段落。下面的示例以不同的样式为每个单词显示 “Hello beautiful world”。

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

&emsp;交互性：

&emsp;要使 Text 对触摸事件作出反应，请将其包装在 GestureDetector.onTap 处理程序的 GestureDetector 小部件中。

&emsp;在 material design 应用程序中，请考虑使用 TextButton，或者如果这不合适，至少使用 InkWell 而不是 GestureDetector。

&emsp;要使文本部分具有交互性，请使用 RichText 并指定 TapGestureRecognizer 作为文本相关部分的 TextSpan.recognizer。

### Icon

&emsp;[Icon class](https://api.flutter.dev/flutter/widgets/Icon-class.html) 使用 IconData 中描述的字体的字形绘制的图形图标 widget，例如 Icons 中 material 的预定义 IconData。 

&emsp;Icons 不是交互式的。对于交互式图标，请考虑材质的 IconButton。

```c++
Row(
  mainAxisAlignment: MainAxisAlignment.spaceAround,
  children: const <Widget>[
    Icon(
      Icons.favorite,
      color: Colors.pink,
      size: 24.0,
      semanticLabel: 'Text to announce in accessibility modes',
    ),
    Icon(
      Icons.audiotrack,
      color: Colors.green,
      size: 30.0,
    ),
    Icon(
      Icons.beach_access,
      color: Colors.blue,
      size: 36.0,
    ),
  ],
)
```

### RaisedButton

&emsp;[RaisedButton class](https://api.flutter.dev/flutter/material/RaisedButton-class.html) Material Design 中的 button， 一个凸起的材质矩形按钮。 

&emsp;This class is deprecated, please use ElevatedButton instead. 已废弃，用 ElevatedButton 代替。

&emsp;[ElevatedButton class](https://api.flutter.dev/flutter/material/ElevatedButton-class.html) 使用升高的按钮为原本大部分为平面的布局添加维度，例如在长而繁忙的内容列表中，或在广阔的空间中。避免在对话框或卡片等已经提升的内容上使用提升的按钮。

### Scaffold

&emsp;[Scaffold class](https://api.flutter.dev/flutter/material/Scaffold-class.html) Material Design 布局结构的基本实现。此 widget 类提供了用于显示 drawer、snackbar 和底部 sheet 的 API。 

&emsp;[AppBar 基础](https://flutterchina.club/catalog/samples/basic-app-bar/)

&emsp;[选项卡式的AppBar](https://flutterchina.club/catalog/samples/tabbed-app-bar/)

### Appbar

&emsp;[AppBar class](https://api.flutter.dev/flutter/material/AppBar-class.html) 一个Material Design 应用程序栏，由工具栏和其他可能的widget（如 TabBar 和 FlexibleSpaceBar ）组成。

&emsp;[具有自定义底部widget的AppBar](https://flutterchina.club/catalog/samples/app-bar-bottom/)

### FlutterLogo

&emsp;[FlutterLogo class](https://api.flutter.dev/flutter/material/FlutterLogo-class.html) Flutter logo, 以 widge t形式，这个 widget 遵从 IconTheme。

### Placeholder

&emsp;[Placeholder class](https://api.flutter.dev/flutter/widgets/Placeholder-class.html) 一个绘制了一个盒子的 widget，代表日后有 widget 将会被添加到该盒子中。这个 widget 在开发过程中很有用，可以指示界面尚未完成。











## 参考链接
**参考链接:🔗**
+ [Widgets 目录](https://flutterchina.club/widgets/)
