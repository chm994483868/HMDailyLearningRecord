#  Flutter 学习笔记：第一个 Flutter 应用

## 入门: 在 macOS 上搭建 Flutter 开发环境

### 一、获取Flutter SDK

&emsp;去 flutter 官网下载其最新可用的安装包：[Flutter SDK releases](https://flutter.dev/docs/development/tools/sdk/releases#macos)，或者去 Flutter 的 Github 仓库去下载：[flutter/flutter](https://github.com/flutter/flutter/releases)。

### 二、更新环境变量

&emsp;添加 flutter 相关工具到 path 中：

```c++
export PATH=`pwd`/flutter/bin:$PATH
```

&emsp;此代码只能暂时针对当前命令行窗口设置 PATH 环境变量，要想永久将 Flutter 添加到 PATH 中需要更新环境变量，以便你可以运行 flutter 命令在任何终端会话中。

1. 打开（或创建）`.bash_profile`，此文件在不同的机器上可能文件路径不同，例如在我的电脑上在此路径：`/Users/hmc/.bash_profile`。如果 `.bash_profile` 文件不存在的话可以 `cd` 到 `Users/xxx(你当前的用户名)` 路径下，然后使用 `vim .bash_profile` 命令自行创建一个。
2. 往 `.bash_profile` 文件中添加以下行并更改 `PATH_TO_FLUTTER_GIT_DIRECTORY` 为下载 Flutter SDK 到本地的路径，例如我的 Flutter SDK 的本地路径是：`/Users/hmc/Documents/GitHub/flutter`，下面示例中的第 3 行则修改为：`export PATH=/Users/hmc/Documents/GitHub/flutter/bin:$PATH`。

```c++
export PUB_HOSTED_URL=https://pub.flutter-io.cn // 国内用户需要设置
export FLUTTER_STORAGE_BASE_URL=https://storage.flutter-io.cn // 国内用户需要设置
export PATH=PATH_TO_FLUTTER_GIT_DIRECTORY/flutter/bin:$PATH
```

3. 打开一个终端窗口执行 `source /Users/hmc/.bash_profile` 命令进行刷新（`.bash_profile` 文件的实际路径大家以自己的机器为准）。
4. 如果终端使用的是 zsh，则终端启动时 `~/.bash_profile` 文件将不会被加载，解决办法就是修改 `~/.zshrc`（我本机的路径是：`/Users/hmc/.zshrc`），在其中添加：`source ~/.bash_profile`。如果 `.zshrc` 文件不存在的话，可使用如下命令创建：

```c++
touch .zshrc
open -e .zshrc
```

&emsp;然后在其中输入：`source ~/.bash_profile` 并保存，然后输入 `source .zshrc` 命令刷新环境使环境变量生效。

### 三、运行 flutter doctor

&emsp;运行 `flutter doctor` 命令查看是否需要安装其它依赖项并完成 Flutter 环境的整体安装。`flutter doctor` 命令检查你的环境并在终端窗口中显示报告，Dart SDK 已经捆绑在 Flutter 里了，没有必要单独安装 Dart。仔细检查命令行输出以获取可能需要安装的其他软件或进一步需要执行的任务（以粗体显示）。

&emsp;如果大家当前已经有 iOS 开发环境的话，那么配置到这里，Flutter 环境基本就配置完成了，通过上述配置，我本机运行 `flutter doctor` 命令（此命令第一次执行会比较慢），输出如下：

```c++
hmc@localhost ~ % flutter doctor
Doctor summary (to see all details, run flutter doctor -v):
[✓] Flutter (Channel master, 2.5.0-7.0.pre.185, on macOS 11.4 20F5046g
    darwin-x64, locale zh-Hans-CN)
[✗] Android toolchain - develop for Android devices
    ✗ Unable to locate Android SDK.
      Install Android Studio from:
      https://developer.android.com/studio/index.html
      On first launch it will assist you in installing the Android SDK
      components.
      (or visit https://flutter.dev/docs/get-started/install/macos#android-setup
      for detailed instructions).
      If the Android SDK has been installed to a custom location, please use
      `flutter config --android-sdk` to update to that location.

[✓] Xcode - develop for iOS and macOS (Xcode 12.4)
[✓] Chrome - develop for the web
[!] Android Studio (not installed)
[✓] VS Code (version 1.42.1)
[✓] Connected device (2 available)

! Doctor found issues in 2 categories.
hmc@localhost ~ % 
```

&emsp;看到 android 环境还需要配置，后续如果需要我们再进行。至此Flutter 的 iOS 环境便已经配好了。

## 起步: 配置编辑器

### 一、安装 Visual Studio Code
&emsp;IDE 部分我们直接选择 [Visual Studio Code](https://code.visualstudio.com) 需要 1.20.1或更高版本，当前 VS Code 版本已经到 1.59，下载安装完 VS Code 以后我们需要安装 Flutter 插件。

### 二、安装 Flutter 插件

&emsp;启动 VS Code，shift + Command + p 调出 VS Code 的命令面板，输入 install，然后选择 Extensions: Install Extensions，在搜索框里输入 flutter，在搜索列表结果中选择：Flutter Flutter support and debugger for Visual Studio Code. 并点击 install 按钮，安装完成选择 OK 重新启动 VS Code。

### 三、通过 Flutter Doctor 验证设置

&emsp;shift + Command + p 调出 VS Code 的命令面板，输入 doctor，然后选择：Flutter: Run Flutter Doctor，这个指令需要执行一小会，然后查找输出，它会详细列出你的机器当前的 Flutter 版本、本地路径、来源、引擎版本、Dart 版本、Xcode 版本及位置、CocoaPods 版本、Chrome 位置、VS Code 版本及位置、插件版本、连接的设备 等等的详细信息。我的机器如下（没有安装安卓环境）

```c++
[flutter] flutter doctor -v
[✓] Flutter (Channel master, 2.5.0-7.0.pre.185, on macOS 11.4 20F5046g darwin-x64, locale zh-Hans-CN)
    • Flutter version 2.5.0-7.0.pre.185 at /Users/hmc/Documents/GitHub/flutter
    • Upstream repository https://github.com/flutter/flutter.git
    • Framework revision 2526cb07cb (2 days ago), 2021-08-21 23:42:01 -0400
    • Engine revision 4783663ee4
    • Dart version 2.15.0 (build 2.15.0-41.0.dev)
    • Pub download mirror https://pub.flutter-io.cn
    • Flutter download mirror https://storage.flutter-io.cn

[✗] Android toolchain - develop for Android devices
    ✗ Unable to locate Android SDK.
      Install Android Studio from: https://developer.android.com/studio/index.html
      On first launch it will assist you in installing the Android SDK components.
      (or visit https://flutter.dev/docs/get-started/install/macos#android-setup for detailed instructions).
      If the Android SDK has been installed to a custom location, please use
      `flutter config --android-sdk` to update to that location.


[✓] Xcode - develop for iOS and macOS (Xcode 12.4)
    • Xcode at /Applications/Xcode.app/Contents/Developer
    • CocoaPods version 1.10.1

[✓] Chrome - develop for the web
    • Chrome at /Applications/Google Chrome.app/Contents/MacOS/Google Chrome

[!] Android Studio (not installed)
    • Android Studio not found; download from https://developer.android.com/studio/index.html
      (or visit https://flutter.dev/docs/get-started/install/macos#android-setup for detailed instructions).

[✓] VS Code (version 1.42.1)
    • VS Code at /Users/hmc/Downloads/Visual Studio Code-2.app/Contents
    • Flutter extension version 3.8.1

[✓] Connected device (3 available)
    • iPhone 12 (mobile)     • FF9BFB96-8FF4-4AD6-98B8-1C8889653AF0 • ios            • com.apple.CoreSimulator.SimRuntime.iOS-14-4 (simulator)
    • iPhone 12 Pro (mobile) • CC2922E4-A2DB-43DF-8B6F-D2987F683525 • ios            • com.apple.CoreSimulator.SimRuntime.iOS-14-4 (simulator)
    • Chrome (web)           • chrome                               • web-javascript • Google Chrome 92.0.4515.159

! Doctor found issues in 2 categories.
exit code 0
```

&emsp;至此 IDE 配置完成。

## 起步: 简单体验

&emsp;本节从我们的模板创建一个新的 Flutter 应用程序，运行它，并学习如何使用 Hot Reload（热重载） 进行更新重载。

### 一、创建 Flutter 新应用

&emsp;shift + Command + p 调出 VS Code 的命令面板，输入 flutter，然后选择 Flutter: New Project，然后在输入框中输入项目名称（如：FirstFlutterDemo），然后按回车键，选择项目的本地路径，项目创建完成，VS Code 会默认选中并打开 main.dart 文件，确保在 VS Code 的右下角选择了目标设备，然后按 F5 键或调用 Debug > Start Debugging，等待应用程序启动，如果一切正常，在应用程序创建启动成功后，应该在设备或模拟器上看到应用程序截图如下：

![截屏2021-08-24 下午9.53.42.png](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/6a96395a6f904204b7b0a45c360f2cb1~tplv-k3u1fbpfcp-watermark.image)

### 二、热重载

&emsp;Flutter 可以通过 热重载（hot reload） 实现快速的开发周期，热重载就是无需重启应用程序（原生开发，修改任意一行代码都要重新编译运行才能生效）就能实时加载修改后的代码，并且不会丢失状态。简单的对代码进行更改，然后告诉 IDE 或命令行工具你需要重新加载（点击 Hot Reload 按钮），你就会在你的设备或模拟器上看到更改。如在 main.dart 文件中修改 `You have pushed the button this many times:` 字符串内容，然后点击 command + s 保存，此时便立刻能在模拟器上看到更新的字符串。

## 正式编写第一个 Flutter App

&emsp;新建一个命名为 startup_namer 的 Flutter 项目。直接删除 lib/main.dart 中的全部代码，然后替换为如下代码并运行，它仅仅显示一个标题是 `Welcome to Flutter` 的蓝色导航条和屏幕中心的 `Hello World` 文本。 

```c++
// 引入 material.dart
import 'package:flutter/material.dart';

// => 符号是 Dart 中单行函数/方法的简写，同 void main() { runApp(new MyApp()); }
// runApp 函数的参数是一个 MyApp 实例
void main() => runApp(new MyApp());

// MyApp 类继承自 StatelessWidget
class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return new MaterialApp(
      title: 'Welcome to Flutter',
      
      // Scaffold 是来自 Material 库中
      home: new Scaffold(
        // 导航栏
        appBar: new AppBar(
          // 导航栏标题
          title: new Text('Welcome to Flutter'),
        ),
        
        // 主屏幕内容
        body: new Center(
          // 文本 widget
          child: new Text('Hello World'),
        ),
      ),
    );
  }
}
```

&emsp;以上示例是创建了一个 Material APP。（Material 是一种标准的移动端和 web 端的视觉设计语言。 Flutter 提供了一套丰富的 Material widgets。）

&emsp;`MyApp` 类继承自 `StatelessWidget`，这将会使应用本身也成为一个 widget。 在 Flutter 中，大多数东西都是 widget，包括对齐（`alignment`）、填充（`padding`）和布局（`layout`）。

&emsp;`Scaffold` 是 Material library 库中提供的一个 widget，它提供了默认的导航栏（`appBar`）、标题（`title`）和包含主屏幕 widget 树的 `body` 属性。widget 树可以很复杂。widget 的主要工作是提供一个 `build()` 方法来描述如何根据其他较低级别的 widget 来显示自己。

&emsp;示例中的 `body` 的 widget 树中包含了一个 `Center` widget，`Center` widget又包含一个 `Text` widget 作为其子 widget。 `Center` widget 可以将其子 widget 树对齐到屏幕中心。

### 使用外部包（package）

&emsp;在上面的实例中，我们使用了 `flutter/material.dart` 这个 默认的 package，下面我们尝试导入其他自定义的 package。(本节以引入 english_words 4.0.0 package 为例，english_words 是用于处理英语单词的实用程序。计算音节，生成听起来不错的单词组合，并提供按用法排名前 5000 的英语单词。)

&emsp;在 [pub.dartlang.org](https://pub.dev/flutter/packages) 中我们能看到有很多不同功能的 package，我们在其中搜索到：[english_words 4.0.0](https://pub.dev/packages/english_words)。

#### 一、修改 pubspec.yaml 文件指定要引入的 package 

&emsp;pubspec.yaml 文件用来管理 Flutter 应用程序的 assets（资源，如图片、package 等）。下面我们在 pubspec.yaml 文件中，将 `english_words: ^4.0.0` 添加到 `dependencies:` 下面，作为当前程序的一个依赖项。当前 `startup_namer` 这个 Flutter 应用程序的 pubspec.yaml 文件的完整内容如下：

```c++
name: startup_namer
description: A new Flutter project.

# 以下行可防止使用 `flutter pub publish` 意外地将 package 发布到 pub.dev。这是 private packages 的首选项。
publish_to: 'none' # 如果你希望发布到 pub.dev，请删除此行

# 下面定义了应用程序的版本号和构建号。
# 版本号是按点分隔的三个数字，如 1.2.43，后跟可选的构建号用一个 + 分开。
# 版本和 builder 号都可以通过分别指定 --build-name 和 --build-number 来覆盖在 flutter 构建中。
#
# 在 Android 中，build-name 用作 versionName，而 build-number 用作 versionCode。
# Read more about Android versioning at https://developer.android.com/studio/publish/versioning
#
# 在 iOS 中，build-name 用作 CFBundleShortVersionString，而 build-number 用作 CFBundleVersion。
# Read more about iOS versioning at https://developer.apple.com/library/archive/documentation/General/Reference/InfoPlistKeyReference/Articles/CoreFoundationKeys.html
version: 1.0.0+1

environment:
  sdk: ">=2.12.0 <3.0.0"

# dependencies 指定你的 package 需要的其他 packages 才能工作。要自动将你的 package dependencies 升级到最新版本，请考虑运行：flutter pub upgrade --major-versions 指令。
# 或者，可以通过将下面的版本编号更改为 pub.dev 上提供的最新版本来手动更新 dependencies。要查看哪些 dependencies 有较新的版本可用，请运行：flutter pub outdated 指令。
dependencies:
  flutter:
    sdk: flutter

  # 下面将 Cupertino Icons 字体添加到你的应用程序中。与 iOS 样式图标的 CupertinoIcons 类一起使用。
  cupertino_icons: ^1.0.2
  
  # 引入 english_words
  english_words: ^4.0.0

dev_dependencies:
  flutter_test:
    sdk: flutter
  
  # 下面的 "flutter_lints" package 包含一组推荐的 lints，以鼓励良好的编码实践。
  # package 提供的 lint set 在位于 package root 的 "analysis_options.yaml" 文件中激活。
  # 有关停用特定 lint rules 和激活其他 rules 的信息，请参阅该文件。
  flutter_lints: ^1.0.0

# 有关此文件的通用 Dart 部分的信息，see the following page: https://dart.dev/tools/pub/pubspec

# 以下部分特定于 Flutter。
flutter:

  # 以下行可确保将 Material Icons font 包含在你的应用程序中，以便你可以在 material Icons class 中使用 icons。
  uses-material-design: true

  # 要将 assets（一般是图片） 添加到你的应用程序，请添加 assets 部分，如下所示：
  # assets:
  #   - images/a_dot_burr.jpeg
  #   - images/a_dot_ham.jpeg

  # image asset 可以引用一个或多个特定于分辨率的 "variants"（变体），see https://flutter.dev/assets-and-images/#resolution-aware.

  # 有关从 package dependencies 中添加 assets 的详细信息，see https://flutter.dev/assets-and-images/#from-packages 
  
  # 要在应用程序中添加自定义字体，请在此处添加此 "flutter" 部分的字体部分。此列表中的每个条目应有一个带有字体 family 的 "family" 键，以及一个带有列表的 "fonts" 键，该键提供字体的 asset 和其他描述符。
  # For example:
  # fonts:
  #   - family: Schyler
  #     fonts:
  #       - asset: fonts/Schyler-Regular.ttf
  #       - asset: fonts/Schyler-Italic.ttf
  #         style: italic
  #   - family: Trajan Pro
  #     fonts:
  #       - asset: fonts/TrajanPro.ttf
  #       - asset: fonts/TrajanPro_Bold.ttf
  #         weight: 700
  #
  # 有关 package dependencies 中字体的详细信息 see https://flutter.dev/custom-fonts/#from-packages
```

&emsp;(analysis_options.yaml 文件后面再讲，此部分先把引入 package 讲完。)

&emsp;在 `pubspec.yaml` 文件中引入了 `english_words: ^4.0.0`，除了可以引入依赖项之外，`pubspec.yaml` 还提供了许多其他的功能。

&emsp;`english_words` 后面的 `^4.0.0` 直接指明了这个 package 的版本。当没有指定版本号时我们可以使用 `flutter pub upgrade --major-versions` 指令把依赖的 package 升级到最新版本，也可以手动修改版本号为一个指定版本，指定为当前最新版本来进行手动更新。要查看哪些依赖的 package 有较新的版本可用时可运行：`flutter pub outdated` 指令查看。我们当前是最新的，所以打印没有发现。

```c++
hmc@bogon startup_namer % flutter pub outdated
Showing outdated packages.
[*] indicates versions that are not the latest available.

Found no outdated packages
hmc@bogon startup_namer % 
```

&emsp;运行 `flutter pub upgrade --major-versions` 指令会有如下打印。看到 `cupertino_icons` 最新是 `1.0.3` 了，由于在 `pubspec.yaml` 文件中我们直接指定了 `^1.0.2` 所以它并没有被直接更新。还有 `flutter_lints` 最新的是 `1.0.4`，然后在 `pubspec.yaml` 文件中指定的是 `^1.0.0`，也不会进行强制更新。

```c++
hmc@bogon startup_namer % flutter pub upgrade --major-versions
Resolving dependencies...
  async 2.8.2
  boolean_selector 2.1.0
  characters 1.1.0
  charcode 1.3.1
  clock 1.1.0
  collection 1.15.0
  cupertino_icons 1.0.3
  english_words 4.0.0
  fake_async 1.2.0
  flutter 0.0.0 from sdk flutter
  flutter_lints 1.0.4
  flutter_test 0.0.0 from sdk flutter
  lints 1.0.1
  matcher 0.12.11
  meta 1.7.0
  path 1.8.0
  sky_engine 0.0.99 from sdk flutter
  source_span 1.8.1
  stack_trace 1.10.0
  stream_channel 2.1.0
  string_scanner 1.1.0
  term_glyph 1.2.0
  test_api 0.4.3
  typed_data 1.3.0
  vector_math 2.1.0
No dependencies changed.

No changes to pubspec.yaml!
hmc@bogon startup_namer % 
```

&emsp;然后 `pubspec.yaml` 文件中也指定了版本号和构建号，然后还有引入图片资源、字体资源的内容。

#### 二、获取 package

&emsp;运行 `flutter packages get` / `flutter pub get` 来拉取 pubspec.yaml 文件中引入的 package，它们会被下载到 `/Users/hmc/.pub-cache/hosted/pub.flutter-io.cn` 路径中。可以看到有如下输出：

```c++
hmc@bogon startup_namer % flutter packages get
Running "flutter pub get" in startup_namer...                      565ms
hmc@bogon startup_namer % flutter pub get
Running "flutter pub get" in startup_namer...                      579ms
hmc@bogon startup_namer % 
```

#### 三、在代码中引入 package

&emsp;在 `lib/main.dart` 文件的顶部我们可以引入要使用的 package，如下：

```c++
import 'package:flutter/material.dart';
import 'package:english_words/english_words.dart';
```

&emsp;这里还有一个点，上面我们导入了 `english_words/english_words.dart` 当我们下面的代码不使用其中的内容时，它们会呈现为灰色的，它可以指示我们导入的库尚未使用（到目前为止）。

#### 四、使用导入的 package

&emsp;下面我们使用 `english_words` package 中的函数生成的英文单词字符串来替代 `Hello World`。

> &emsp;Tip: **驼峰命名法**（称为 "upper camel case" 或 "Pascal case"）, 表示字符串中的每个单词（包括第一个单词）都以大写字母开头。所以，"uppercamelcase" 变成 "UpperCamelCase"。

&emsp;代码修改如下：

```dart
import 'package:flutter/material.dart';
// ⬇️ 引入 english_words 中的内容
import 'package:english_words/english_words.dart';

void main() { runApp(new MyApp()); }

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
  
    // ⬇️ 调用 WordPair 类的 random 函数，生成一个随机单词对 
    final wordPair = new WordPair.random();
    
    return new MaterialApp(
      title: 'Welcome to Flutter',
      home: new Scaffold(
        appBar: new AppBar(
          title: new Text('Welcome to Flutter'),
        ),
        body: new Center(
        
          // ⬇️ 把 "Hello World" 注释了，然后 wordPair 转换为驼峰命名法，显示到屏幕中心
          // child: new Text('Hello World'),
          child: new Text(wordPair.asPascalCase),
          
        ),
      ),
    );
  }
}
```

#### 五、热重载测试 package 的使用

&emsp;如果应用程序正在运行，请使用热重载按钮 (lightning bolt icon) 更新正在运行的应用程序。每次单击热重载或保存项目时，都会在正在运行的应用程序中随机选择不同的单词对。 这是因为单词对是在 build 方法内部生成的。每次MaterialApp需要渲染时或者在Flutter Inspector中切换平台时 build 都会运行.






```c++
export HOMEBREW_BOTTLE_DOMAIN=https://mirrors.tuna.tsinghua.edu.cn/homebrew-bottles
export PUB_HOSTED_URL=https://pub.flutter-io.cn
export FLUTTER_STORAGE_BASE_URL=https://storage.flutter-io.cn
export PATH=/Users/hmc/Documents/GitHub/flutter/bin:$PATH
```


## 参考链接
**参考链接:🔗**
+ [Mac pro 找不到zshrc文件](https://www.jianshu.com/p/6e9d776836ab)
