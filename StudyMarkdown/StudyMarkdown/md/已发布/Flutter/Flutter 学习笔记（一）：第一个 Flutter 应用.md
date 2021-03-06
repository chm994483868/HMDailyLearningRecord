# Flutter 学习笔记（一）：第一个 Flutter 应用.md

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

```c++
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

&emsp;如果应用程序正在运行，请点击热重载按钮 (⚡️闪电图标：Hot Reload) 更新正在运行的应用程序。每次点击热重载或保存项目时，都会在正在运行的应用程序中随机选择不同的单词对，可看到模拟器中心显示不同的单词对。 这是因为 `wordPair` 变量是在 `build` 方法内部生成的，每次 `MaterialApp` 需要渲染时或者在 Flutter Inspector 中切换平台时 `build` 函数都会（重新）运行，此时 `final wordPair = new WordPair.random();` 便生成了新的 `wordPair` 变量。（我们可以把 `final wordPair = new WordPair.random();` 提出来，放到 `build` 函数的上面，再进行热重载，可发现模拟器屏幕中心的单词不再变化了。）

### 添加一个 有状态的部件（Stateful widget）

&emsp;在学习 `Stateful widget` 之前，我们再回顾一下上面的实例代码，其中最引我们瞩目的应该是 `class MyApp extends StatelessWidget {...}`，看到 `MyApp` 继承自 `StatelessWidget`，而我们这一小节的学习内容则主要与 `StatefulWidget` 类有关。  

&emsp;Stateless widgets 是不可变的，这意味着它们的属性不能改变，所有的值都是最终的。Stateful widgets 持有的状态可能在 widget 生命周期中发生变化。

&emsp;实现一个 Stateful widget 至少需要两个类：

1. 一个 `StatefulWidget` 类。
2. 一个 `State` 类。`StatefulWidget` 类本身是不变的，但是 `State` 类在 widget 生命周期中始终存在。

&emsp;在本小节中我们将添加一个继承自 `StatefulWidget` 类的子类：`RandomWords`，重写 `RandomWords` 类的 `createState` 函数，返回一个 `State` 的子类 `RandomWordsState` 的实例对象。`State` 类将最终为 widget 来维护建议的和喜欢的单词对。（一个 `WordPair` 数组存放建议的单词对，一个 `WordPair` 集合存放喜欢的单词对。）

&emsp;下面我们开始本小节的内容。

1. 添加有状态的 `RandomWords widget` 到 main.dart 文件的底部。它可以在 MyApp 之外的文件的任何位置使用，但是本示例将它放到了 main.dart 文件的底部。在 `RandomWords widget` 内部仅重写了它的 `createState` 函数，创建 `State` 类，其他没有做任何事情。下面我们分析一下 `createState` 函数。 

```c++
class RandomWords extends StatefulWidget {
  @override
  createState() => new RandomWordsState();
}
```

&emsp;`State<StatefulWidget> createState()` 函数用于在 widget 树的给定位置创建可变 `State`（函数返回值是 `State<StatefulWidget>`）。`StatefulWidget` 子类应重写此方法，并返回一个它们关联的 `State` 子类的新创建的实例。

```c++
@override
State<MyWidget> createState() => _MyWidgetState();
```

&emsp;framework 可以在 `StatefulWidget` 的生命周期内多次调用此方法。例如，如果 `widget` 在多个位置插入到 `widget` 树中，framework 将为每个位置创建一个单独的 `State` 对象。类似的，如果 `widget` 从树中移除并稍后再次插入到树中，framework 将再次调用 `createState` 函数，以创建新的 `State` 对象，从而简化 `State` 对象的生命周期。

2. 添加 `RandomWordsState` 类。该应用程序的大部分代码都在该类中，该类持有 `RandomWords widget` 的状态。这个类将保存随着用户滚动而无限增长的生成的随机单词对，以及用户点击选中的单词对，用户通过重复点击心形 ❤️ 图标来将单词对从列表中添加或删除。首先定义 `RandomWordsState` 类，下面我们会一步一步为其添加内容。

```c++
class RandomWordsState extends State<RandomWords> {
    // ...
}
```

3. 声明了 `RandomWordsState` 类以后，IDE 会提示我们 `RandomWordsState` 类缺少 `build` 方法，我们重写 `build` 方法，并把之前在 `MyApp` 中生成随机单词对的代码移动到 `RandomWordsState` 中来生成单词对。示例代码如下：

```c++
class RandomWordsState extends State<RandomWords> {
  @override
  Widget build(BuildContext context) {
    final wordPair = new WordPair.random();
    return new Text(wordPair.asPascalCase);
  }
}
```

4. 如下示例代码，修改之前的旧代码，把生成随机单词对的代码从 `MyApp` 移动到 `RandomWordsState` 中。

```c++
class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
  
    // ⬇️ 把此行创建 wordPair 局部变量注释掉
    // final wordPair = new WordPair.random();
    
    return new MaterialApp(
      title: 'Welcome to Flutter',
      home: new Scaffold(
        appBar: new AppBar(
          title: new Text('Welcome to Flutter'),
        ),
        body: new Center(
          // child: new Text('Hello World'),
          
          // ⬇️ 注释此行，使用下面的 RandomWords 实例对象，来返回一个随机单词对
          // child: new Text(wordPair.asPascalCase),
          child: new RandomWords(),
          
        ),
      ),
    );
  }
}
```

&emsp;重新启动应用程序，应用程序还是会和之前一样，每次热重载或者 command + s 保存程序，屏幕中心都会显示一个新的单词对。

&emsp;下面这一段是上面示例的完整代码：

```c++
import 'package:flutter/material.dart';
import 'package:english_words/english_words.dart';

void main() { runApp(new MyApp()); }

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
  
    // final wordPair = new WordPair.random();
    
    return new MaterialApp(
      title: 'Welcome to Flutter',
      home: new Scaffold(
        appBar: new AppBar(
          title: new Text('Welcome to Flutter'),
        ),
        body: new Center(
          // child: new Text('Hello World'),
          // child: new Text(wordPair.asPascalCase),
          
          child: new RandomWords(),
        ),
      ),
    );
  }
}

class RandomWords extends StatefulWidget {
  @override
  createState() => new RandomWordsState();
}

class RandomWordsState extends State<RandomWords> {
  @override
  Widget build(BuildContext context) {
    final wordPair = new WordPair.random();
    return new Text(wordPair.asPascalCase);
  }
}
```

### 创建一个无限滚动的 ListView

&emsp;在上一节中，在 `RandomWordsState` 类的内部我们仅重写了它的 `build` 函数，来返回一个随机单词对。而在这一节中，我们继续扩展 `RandomWordsState` 类，以生成并显示单词对列表。当我们向上滑动时，`ListView` 将无限增长显示随机生成的单词对。

&emsp;`ListView` 的 `builder` 工厂构造函数允许我们按需创建一个懒加载的列表视图（`return new ListView.builder(...);`）。

1. 首先我们向上一节创建的 `RandomWordsState` 类中添加一个 `final _suggestions = <WordPair>[];` 数组用以保存建议的单词对，该变量名用下划线开头，在 Dart 语言中使用下划线做前缀标识符，会强制其变成私有的。另外添加一个 `_biggerFont` 变量来增大字体大小。

```c++
class RandomWordsState extends State<RandomWords> {
  final _suggestions = <WordPair>[];

  final TextStyle _biggerFont = const TextStyle(fontSize: 18.0);
  ...
}
```

2. 向 `RandomWordsState` 类中添加一个 `_buildSuggestions()` 函数，此方法用来构建显示建议单词对的 `ListView`。`ListView` 类提供了一个 `builder` 属性，`itemBuilder` 值是一个匿名回调函数，它接受两个参数 `BuildContext, int`，`(context, i)`，`i` 是指行迭代器，迭代器从 0 开始，每调用一次该函数，`i` 就会自增 1，对于每个建议的单词对都会执行一次，该模型允许建议的单词对列表在用户滚动时无限增长。

```c++
class RandomWordsState extends State<RandomWords> {
  ...
  Widget _buildSuggestions() {
    return new ListView.builder(
      padding: const EdgeInsets.all(16.0),
      
      // 对于每个建议的单词对都会调用一次 itemBuilder，然后将单词对添加到 ListTile 行中，
      // 在偶数行，该函数会为单词对添加一个 ListTile row，
      // 在奇数行，该函数会添加一个分割线 widget，来分隔相邻的词对。
      // 注意，在小屏幕上，分割线看起来可能比较吃力。
      //（这里对比 iOS 中的 TableView，分割线是位于一个 Cell 上的，这里的则是 分割线 和 每个单词对都是一个 cell）
      
      itemBuilder: (context, i) {
        // 在每一列之前，添加一个 1 像素高的分隔线 widget（从 0 开始，isOdd 判断是否是奇数）
        if (i.isOdd) return new Divider();
        
        // 语法 `i ~/ 2` 表示 i 除以 2 的商（向下取整），返回值是整形（向下取整），比如 i 为：1，2，3，4，5 时结果则是：0，1，1，2，2，
        // 由此可以计算出 ListView 中减去分隔线后的实际单词对数量。
        final index = i ~/ 2;
        
        // 如果是建议单词列表中最后一个单词对，则进行扩容。
        if (index >= _suggestions.length) {
          // 接着再生成 10 个单词对，然后添加到 _suggestions 中。
          _suggestions.addAll(generateWordPairs().take(10));
        }
        
        // _buildRow 在下面进行解析。
        return _buildRow(_suggestions[index]);
      },
    );
  }
  ...
}

```

3. 对于每一个单词对，`_buildSuggestions` 函数都会调用一次 `_buildRow` 函数。这个函数在 `ListTile` 中显示每个新的单词对，这可以使我们在 `ListView` 中生成每个显示行，下面在 `RandomWordsState` 类中添加 `_buildRow` 函数。

```c++
class RandomWordsState extends State<RandomWords> {
  ...
  Widget _buildRow(WordPair pair) {
    return new ListTile(
      title: new Text(
        
        // 每行显示的文字，单词对转为驼峰命名形式
        pair.asPascalCase,
        // 字号是 18（`final TextStyle _biggerFont = const TextStyle(fontSize: 18.0);`）
        style: _biggerFont,
      ),
    );
  }
}
```

4. 下面我们更新 `RandomWordsState` 类的 `build` 函数中的内容，让其使用我们上面编写的 `_buildSuggestions()` 函数，不再直接仅仅生成一个单词对。

```c++
class RandomWordsState extends State<RandomWords> {
  ...
  @override
  Widget build(BuildContext context) {
    // final wordPair = new WordPair.random();
    // return new Text(wordPair.asPascalCase);
    
    // 这里返回 Sacffold 实例，把之前 MyApp 中的内容接管过来。
    return new Scaffold(
      // 导航条
      appBar: new AppBar(
        // 标题
        title: new Text('Startup Name Generator'),
      ),
      
      // body 这里则是调用 _buildSuggestions() 函数来构建一个 ListView
      body: _buildSuggestions(),
    );
  }
  
}
```

5. 更新 `MyApp` 类中 `build` 函数的内容。从 `MyApp` 中删除 `Scaffold` 和 `AppBar` 实例。这些将由 `RandomWordsState` 类来接管，这也使得在下一步中从一个屏幕导航到另一个屏幕时，可以更轻松的更改导航栏中的路由名称（导航条标题）。

```c++
class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    // final wordPair = new WordPair.random();
    return new MaterialApp(
      // title: 'Welcome to Flutter',
      // home: new Scaffold(
      //   appBar: new AppBar(
      //     title: new Text('Welcome to Flutter'),
      //   ),
      //   body: new Center(
      //     // child: new Text('Hello World'),
      //     // child: new Text(wordPair.asPascalCase),
      //     child: new RandomWords(),
      //   ),
      // ),

      title: 'Startup Name Generator',
      home: new RandomWords(),
    );
  }
}
```

&emsp;重启应用程序，我们将看到一个单词对列表，尽可能的向下滚动，我们可以无限滑动，能一直看到新的单词对生成。

### 添加交互

&emsp;在这一步中，我们将为 `ListView` 的每一行添加一个可点击的心形 ❤️ 图标。当我们点击 `ListView` 的条目时，会切换此条目的 “收藏” 状态，将该单词对添加或移除出 “收藏夹”。

1. 添加一个 `final _saved = new Set<WordPair>();` 集合到 `RandomWordsState` 类中，这个集合用来存储用户点击喜欢的单词对。使用 `Set` 比 `List` 更合适，`Set` 可以保证元素的唯一性。

```c++
class RandomWordsState extends State<RandomWords> {
  final _suggestions = <WordPair>[];

  final _saved = new Set<WordPair>();

  final TextStyle _biggerFont = const TextStyle(fontSize: 18.0);
  ...
}
```

2. 在 `_buildRow()` 方法中添加一个 `final alreadySaved = _saved.contains(pair);` 来检查确保单词对还没有被添加到收藏夹中。

```c++
Widget _buildRow(WordPair pair) {
  final alreadySaved = _saved.contains(pair);
  ...
}
```

3. 同时在 `_buildRow()` 中，添加一个心形 ❤️ 图标到 `ListView` 以启用收藏功能。接下来就可以给心形 ❤️ 图标添加交互能力了。

```c++
Widget _buildRow(WordPair pair) {
  final alreadySaved = _saved.contains(pair);

  return new ListTile(
    title: new Text(
      pair.asPascalCase,
      style: _biggerFont,
    ),
    
    // 添加心形图标
    trailing: new Icon(
      // 并根据当前的单词对是否已经被收藏，心形图标显示为不同的样子（空心的表示未收藏，实心红色表示已经收藏）
      alreadySaved ? Icons.favorite : Icons.favorite_border,
      color: alreadySaved ? Colors.red : null,
    ),
  );
}
```

4. 重启应用，可看到每一行单词对的右边都有一个空心的心形图标，此时它们还没有交互事件。

5. 在 `_buildRow` 中让心形图标变的可以点击（注意这里的交互事件是添加在 ListView 的每一行上面的）。如果单词对已经添加到收藏中，再次点击将其从收藏夹中删除。当每行单词对被点击时，函数调用 `setState()` 通知框架状态已经改变。

```c++
Widget _buildRow(WordPair pair) {
  final alreadySaved = _saved.contains(pair);

  return new ListTile(
    title: new Text(
      pair.asPascalCase,
      style: _biggerFont,
    ),
    
    // 添加心形图标
    trailing: new Icon(
      // 并根据当前的单词对是否已经被收藏，心形图标显示为不同的样子（空心的表示未收藏，实心红色表示已经收藏）
      alreadySaved ? Icons.favorite : Icons.favorite_border,
      color: alreadySaved ? Colors.red : null,
    ),
    
    // ListView 的每行添加交互事件
    onTap: () {
      setState(() {
        // 点击事件，如果当前单词对已经被收藏了则把其从 _saved 集合中移除，否则添加到 _saved 集合中
        if (alreadySaved) {
        
          // 移除
          _saved.remove(pair);
        } else {
        
          // 添加
          _saved.add(pair);
        }
      });
    },
  );
}
```

> &emsp;Note: 在 Flutter 的响应式风格的框架中，调用 `setState()` 函数会为 `State` 对象触发 `build` 方法，从而导致对 UI 的更新。

### 导航到新页面

&emsp;在这一节中，我们将添加一个显示收藏夹内容的页面（在 Flutter 中称为路由（route）），并将学习如何在主路由和新路由之间导航（切换页面）。

&emsp;在 Flutter 中，导航器管理应用程序的路由栈，将路由推入（push）到导航器的栈中，将会显示更新为该路由页面。从导航器的栈中弹出（pop）路由，将显示返回到前一个路由。

1. 在 `RandomWordsState` 类的 `build` 方法中为 `AppBar` 添加一个列表图标，当用户点击列表图标时，包含收藏夹的新路由页面入栈显示。

> &emsp;Note: 某些 widget 属性需要单个 widget（child），而其它一些属性，如果 action，需要一组 widgets（children），用方括号 [] 表示。

&emsp;将该图标及其相应的操作添加到 `build` 方法中：

```c++
class RandomWordsState extends State<RandomWords> {
  ...
  @override
  Widget build(BuildContext context) {
    // final wordPair = new WordPair.random();
    // return new Text(wordPair.asPascalCase);

    return new Scaffold(
      appBar: new AppBar(
        title: new Text('Startup Name Generator'),
        
        // actions 是一组交互，我们这里仅需要一个跳转到收藏夹路由的交互图标
        actions: <Widget>[
          // 收藏夹按钮，点击时调用 _pushSaved 函数，它的图标样式是 Icons.list
          new IconButton(onPressed: _pushSaved, icon: new Icon(Icons.list)),
        ],
        
      ),
      body: _buildSuggestions(),
    );
  }
  ...
}
```

2. 向 `RandomWordsState` 类中添加 `_pushSaved()` 函数。

```c++
class RandomWordsState extends State<RandomWords> {
  ...
  void _pushSaved() {
    // ...
  }
}
```

&emsp;热重载应用，导航栏右侧会显示一个列表按钮样子的图标，现在点击它还不会有任何反应，因为 `_pushSaved()` 函数还没有添加内容。

3. 当用户点击导航栏中右侧的列表图标时，建立一个路由并将其推入到导航管理器的栈中，此操作会切换页面以显示新路由。新页面的内容在 `MaterialPageRoute` 的 `builder` 属性中构建，`builder` 是一个匿名函数。添加 `Navigator.push` 调用，这会使路由入栈（以后路由入栈均指推入到导航管理器的栈）。

```c++
void _pushSaved() {
    Navigator.of(context).push(
    );
}
```

4. 添加 `MaterialPageRoute` 及其 `builder`。现在，添加生成 `ListTile` 行的代码。`ListTile` 的 `divideTiles()` 方法在每个 `ListTile` 之间添加 1 像素的分割线。该 `divided` 变量持有最终的列表项。

```c++
void _pushSaved() {
  Navigator.of(context).push(
  
    new MaterialPageRoute(
      builder: (context) {
      
        // 遍历 _saved 中收集的每个单词对
        final tiles = _saved.map(
          (pair) {
            // 根据每个单词对构建一个 ListTile 行
            return new ListTile(
              title: new Text(
                pair.asPascalCase,
                style: _biggerFont,
              ),
            );
            
          },
        );
        
        // 在每个 ListTile 之间添加 1 像素的分割线
        final divided = ListTile.divideTiles(
          context: context,
          
          // 上面构建的内容
          tiles: tiles,
        ).toList();
      },
    ),
    
  );
}
```

5. `builder` 返回一个 `Scaffold`，其中包含名为 `Saved Suggestions` 的新路由的导航条。新路由的 `body` 由包含 `ListTiles` 行的 `ListView` 组成，每行之间通过一个分隔线分隔。

```c++
void _pushSaved() {
  Navigator.of(context).push(
  
    new MaterialPageRoute(
      builder: (context) {
      
        // 遍历 _saved 中收集的每个单词对
        final tiles = _saved.map(
          (pair) {
            // 根据每个单词对构建一个 ListTile 行
            return new ListTile(
              title: new Text(
                pair.asPascalCase,
                style: _biggerFont,
              ),
            );
            
          },
        );
        
        // 在每个 ListTile 之间添加 1 像素的分割线
        final divided = ListTile.divideTiles(
          context: context,
          
          // 上面构建的内容
          tiles: tiles,
        ).toList();
        
        // 新路由页面
        return new Scaffold(
          appBar: new AppBar(
            title: new Text('Saved Suggestions'),
          ),
          
          // ListView 
          body: new ListView(children: divided),
        );
      },
    ),
    
  );
}
```

6. 热重载应用程序，收藏一些单词对，并点击导航栏上的列表图标，在新路由页面中显示收藏的内容。注意，导航器会在导航栏中添加一个 “返回” 按钮，我们不必显式实现 `Navigator.pop`，点击返回按钮时便能回到主页路由。

### 总结

&emsp;至此我们便学会了一个简单的可滚动、可交互、可路由的 Flutter 应用程序了，同时我们对 Flutter 应该也有一个大致的了解了，相比原生而言它的开发效率可太高了，热重载也太爱了，原生动辄该一行代码都要重新编译运行实在太“拉胯”了！那么本篇就到这里吧， 后续我们开始深入学习 Flutter！⛽️⛽️ 🎉🎉🎉

## 参考链接
**参考链接:🔗**
+ [Mac pro 找不到zshrc文件](https://www.jianshu.com/p/6e9d776836ab)
+ [编写您的第一个 Flutter App](https://flutterchina.club/get-started/codelab/)
+ [Libraries and visibility](https://dart.dev/guides/language/language-tour#libraries-and-visibility)





