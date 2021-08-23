#  Flutter 学习笔记：第一个 Flutter 应用

## 入门: 在macOS上搭建Flutter开发环境

### 一、获取Flutter SDK

&emsp;去 flutter 官网下载其最新可用的安装包：[Flutter SDK releases](https://flutter.dev/docs/development/tools/sdk/releases#macos)，或者去 Flutter 的 Github 仓库去下载：[flutter/flutter](https://github.com/flutter/flutter/releases)。

### 二、更新环境变量

&emsp;添加 flutter 相关工具到 path 中：

```c++
export PATH=`pwd`/flutter/bin:$PATH
```

&emsp;此代码只能暂时针对当前命令行窗口设置 PATH 环境变量，要想永久将 Flutter 添加到 PATH 中需要更新环境变量，以便你可以运行 flutter 命令在任何终端会话中。

1. 打开（或创建）`.bash_profile`，此文件在不同的机器上可能文件路径不同，例如在我的电脑上在此路径：`/Users/hmc/.bash_profile`。如果 `.bash_profile` 文件不存在的话可以使用 `vim .bash_profile` 指令自行创建一个。
2. 往 `.bash_profile` 文件中添加以下行并更改 `PATH_TO_FLUTTER_GIT_DIRECTORY` 为下载 Flutter SDK 到本地的路径，例如我的 Flutter SDK 的本地路径是：`/Users/hmc/Documents/GitHub/flutter`。

```c++
export PUB_HOSTED_URL=https://pub.flutter-io.cn // 国内用户需要设置
export FLUTTER_STORAGE_BASE_URL=https://storage.flutter-io.cn // 国内用户需要设置
export PATH=PATH_TO_FLUTTER_GIT_DIRECTORY/flutter/bin:$PATH // 我的机器：export PATH=/Users/hmc/Documents/GitHub/flutter/bin:$PATH
```

3. 打开一个终端窗口运行 `source /Users/hmc/.bash_profile`（`.bash_profile` 文件的实际路径大家以自己的机器为准）。
4. 如果终端使用的是 zsh，则终端启动时 `~/.bash_profile` 文件将不会被加载，解决办法就是修改 `~/.zshrc`（我本机的路径：`/Users/hmc/.zshrc`），在其中添加：`source ~/.bash_profile`。如果 `.zshrc` 文件不存在的话，可使用如下命令创建：

```c++
touch .zshrc
open -e .zshrc
```
&emsp;然后在其中输入：`source ~/.bash_profile` 并保存，然后输入 `source .zshrc` 命令刷新环境使环境变量生效。

### 三、运行 flutter doctor

&emsp;运行 `flutter doctor` 命令查看是否需要安装其它依赖项并完成 Flutter 环境的整体安装。`flutter doctor` 命令检查你的环境并在终端窗口中显示报告，Dart SDK 已经在捆绑在 Flutter 里了，没有必要单独安装 Dart。仔细检查命令行输出以获取可能需要安装的其他软件或进一步需要执行的任务（以粗体显示）。

&emsp;如果大家当前已经有 iOS 开发环境的话，此时 Flutter 环境基本已经配置完成了，通过上述配置，我本机运行 `flutter doctor` 命令，输出如下：

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

&emsp;看到 android 环境还需要配置，后续如果需要再进行。Flutter 的 iOS 环境便已经配好了。

## 起步: 配置编辑器

&emsp;




































## 参考链接
**参考链接:🔗**
+ [Mac pro 找不到zshrc文件](https://www.jianshu.com/p/6e9d776836ab)
