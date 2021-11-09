# iOS App Crash 分析：(二)：dsym 文件讲解

## DWARF 概述

&emsp;DWARF 是一种被广泛使用的标准化调试数据格式 [Debugging data format](https://en.wikipedia.org/wiki/Debugging_data_format)。

&emsp;DWARF 最初是与 ELF[Executable and Linkable Format](https://en.wikipedia.org/wiki/Executable_and_Linkable_Format) 一起设计的（ELF 是类 Unix 操作系统的可执行二进制文件标准格式，如 Linux 的主要可执行文件格式就是 ELF，macOS 的主要可执行文件格式是 Mach-O），尽管 DWARF 是一种独立于 [object file](https://en.wikipedia.org/wiki/Object_file) 的格式（这里的意思是即使 DWARF 最初是与 ELF 一起设计的，但是 DWARF 是独立与目标文件格式的，它并不是和 ELF 绑定的）。

&emsp;DWARF 这个名字是对 ELF 的 [medieval fantasy](https://en.wikipedia.org/wiki/Historical_fantasy#Medieval_fantasy) 补充，没有官方意义，尽管后来提出是 Debugging With Arbitrary Record Formats 或 Debugging With Attributed Record Formats 的首字母缩写（使用任意记录格式调试/使用属性化记录格式调试）。[Debugging data format](https://en.wikipedia.org/wiki/Debugging_data_format)

+ DWARF 是许多 **编译器** 和 **调试器** 用于支持源码级调试的 **调试文件格式**（debugging file format）（在开发中除了源码级调试还有汇编指令级调试），它满足了许多过程语言的要求，如 C、C++ 和 Fortran，并且可以扩展到其他语言。
+ DWARF 是独立于架构的，适用于任何处理器或操作系统。它广泛应用于 Unix、Linux 和其他操作系统，以及单机环境中（stand-alone environments）。[The DWARF Debugging Standard](http://dwarfstd.org)

&emsp;一个调试器的任务是尽可能以自然、可理解的方式，向程序员提供 **执行程序的一个概观**，同时 **允许对其执行进行多样各种不同的控制**，这意味着在本质上，调试器必须 **逆向许多编译器精心制作的变换**，把程序的 **数据及状态** 转换回到这个程序源代码里程序员原来使用的措辞（terms）。DWARF 调试数据格式便可为这个过程服务。

&emsp;关于 DWARF 调试格式的内容还有很多。例如它的发展历程，当前已经到达 DWARF 5（2017 年发布）。例如它的设计模型它内部的块结构，它是如何描述几乎任何机器架构上的过程编程语言的，它是如何紧凑的表示可执行程序与源代码关系的。等等内容，在下面的章节中我们会随机梳理一下，毕竟网络上有大篇的相关文档。

&emsp;下面我们主要把关注点放在 iOS 日常开发工作中与 DWARF 有接触的一些点上（.dSYM）。

&emsp;~~本篇的重心我们放在 Xcode 中 Build Options 中 Debug Information Format 选择 DWARF with dSYM File 选项中生成的 .dSYM 文件，下面我们通过 .dSYM 文件来一起学习 DWARF 和 .dSYM 文件内部结构，然后学习如何从 crash log 中追踪解析错误日志。~~

&emsp;下面我们从一个 iOS 示例项目开始学习。

## Xcode：Debug Information Format

&emsp;首先我们使用 Xcode 创建一个名为 dSYMDemo 的 iOS 项目，然后在其 Build Settings 中直接搜索 DWARF，我们便可看到 Build Options -> Debug Information Format 选项，其中在 Debug 模式下默认值是 DWARF，在 Release 模式下默认值是 DWARF with dSYM File，然后我们也可以直接把 Debug 模式时的 DWARF 设置为 DWARF with dSYM File，然后运行项目便可在 ~/Library/Developer/Xcode/DerivedData/dSYMDemo-aewxczjzradnxqbkowrhyregmryo/Build/Products/Debug-iphonesimulator 路径（以本机实际路径为准）下生成 dSYMDemo.app 和 dSYMDemo.app.dSYM 两个文件，其中的 dSYMDemo.app 文件我们在学习 mach-o 时已经详细研究过，本篇我们主要来研究 dSYMDemo.app.dSYM 文件（即在 debug 模式下，通过修改 Build Options -> Debug Information Format 选项，在 debug 模式下 build 时同时也生成一份 .dSYM 文件出来）。

&emsp;这里我们发现当 Debug Information Format 设置为 DWARF 时仅生成一个 .app 文件，如果设置为 DWARF with dSYM File 则会同时生成一个 .app 文件一个 .dSYM 文件，那我们是不是会有一个疑惑呢，当设置了 Debug Information Format 为 DWARF 时，那调试文件（调试数据）去哪了呢？既然都让我们选择了调试信息格式，那这调试文件（调试信息）总会生成的吧？






&emsp;.dSYM 文件，乍一看这个后缀名好像好复杂，其实不然，它仅起一个类似 “聚合” 的作用，它和 .app 文件一样，**它们都仅是一个带后缀的文件夹形式的文件**，所以我们直接把它们看作文件夹就好，在 macOS 中直接右键显示包内容即可直接看到其文件夹内部的内容。下面会直接 `cd` 进入 .dSYM 文件内部查看其核心文件内容。（dSYM 这四个字母便是 Debug Symbols（调试符号）的缩写）

&emsp;在前面学习 mach-o 时我们多次使用过 file 命令，下面我们依然使用 file 命令来查看文件（类型）的详细信息。（可以在控制台输入 file --help 指令并回车，查看 file 命令的更多详细信息）

&emsp;下面我们再回顾一下 iOS 日常开发中经常遇到的几个文件后缀：.xcarchive .ipa .app .dSYM .plist。（在 macOS 的文件系统中我们选中指定文件然后在右边的简介中已经列出了此文件的简要的种类信息，当我们选中 .dSYM 文件右键显示简介时，还会有一个 “附赠信息”，它会直接列出当前 .dSYM 文件的 uuid，帮助 dwarfdump -uuid 命令做了它要做的事情。）

```c++
hmc@bogon Debug-iphonesimulator % dwarfdump -uuid dSYM_Demo.app.dSYM
UUID: E54BEE35-F931-3C61-B045-D729AE9E8F02 (x86_64) dSYM_Demo.app.dSYM/Contents/Resources/DWARF/dSYM_Demo
```

+ .xcarchive：Xcode Archive（带后缀的文件夹形式的文件），file 命令查看：`file dSYMDemo.xcarchive: dSYMDemo.xcarchive: directory`
+ .ipa：iOS 软件包归档（一个 zip 文件，可以直接用 unzip 命令解压）file 命令查看：`file dSYMDemo.ipa: dSYMDemo.ipa: Zip archive data, at least v1.0 to extract`
+ .app：应用程序(Intel)（带后缀的文件夹形式的文件）file 命令查看：`file dSYMDemo.app: dSYMDemo.app: directory`
+ .dSYM：Archived Debug Symbols（带后缀的文件夹形式的文件）file 命令查看：`file dSYMDemo.app.dSYM: dSYMDemo.app.dSYM: directory`
+ .plist：Property List，file 命令查看：`file ExportOptions.plist: ExportOptions.plist: XML 1.0 document text, ASCII text`

|  | macOS 文件系统显示 | file 命令查看文件类型 |
| -- | -- |-- |
|  .xcarchive | Xcode Archive（带后缀的文件夹形式的文件） | dSYMDemo.xcarchive: directory |
|  .ipa | iOS 软件包归档（一个 zip 文件，可以直接用 unzip 命令解压） | dSYMDemo.ipa: Zip archive data, at least v1.0 to extract |
|  .app | 应用程序(Intel)（带后缀的文件夹形式的文件） | dSYMDemo.app: directory |
|  .dSYM | Archived Debug Symbols（带后缀的文件夹形式的文件） | dSYMDemo.app.dSYM: directory |
|  .plist | Property List | ExportOptions.plist: XML 1.0 document text, ASCII text |

&emsp;既然 dSYMDemo.app.dSYM 是一个文件夹，那么我们继续看其内部的内容，这里除了选中文件右键显示包内容查看，还可以先使用 [tree](http://mama.indstate.edu/users/ice/tree/) 命令来一览 dSYMDemo.app.dSYM 文件的内部文件层级。（tree 命令可以使用 Homebrew 安装：brew install tree）

```c++
hmc@bogon Debug-iphonesimulator % tree dSYMDemo.app.dSYM 
dSYMDemo.app.dSYM
└── Contents
    ├── Info.plist
    └── Resources
        └── DWARF
            └── dSYMDemo

3 directories, 2 files
```

&emsp;可看到 dSYMDemo.app.dSYM 内部有个 3 个文件夹，2 个文件，其中最核心的便是 DWARF 文件夹下的 dSYMDemo 文件，它没有任何后缀名。 

### 生成 .dSYM 文件

&emsp;除了分别以 Debug 和 Release 模式运行 dSYMDemo 项目能分别在 Debug-iphonesimulator/Release-iphonesimulator 文件夹下生成 dSYMDemo.app.dSYM 文件外，还有一个用到最多的地方（毕竟 .dSYM 主要用来符号化线上收集到的 crash log），此处取得的 .dSYM 文件可以理解为我们打包项目的 “存根” 文件，我们直接 Archive dSYMDemo 项目，然后选中 ~/Library/Developer/Xcode/Archives 路径下的 `dSYMDemo 2021-9-24, 08.2512.xcarchive` 文件右键显示包内容，在其 dSYMs 路径下也会生成一份 dSYMDemo.app.dSYM 文件。下面我们使用 macOS 下的 file 命令来看一下这个 dSYMDemo.app.dSYM 文件到底是个什么文件。

&emsp;这里我们以 Release-iphonesimulator 下的 dSYMDemo.app.dSYM 文件为例：

```c++
hmc@bogon Release-iphonesimulator % file dSYMDemo.app.dSYM 
dSYMDemo.app.dSYM: directory
```

&emsp;乍一看觉得 .dSYM 后缀很特别，可看到它其实和 .app 后缀一样，也就仅是一个文件夹而已。那么我们直接右键 dSYMDemo.app.dSYM 显示包内容，在其 DWARF 路径下有一个 dSYMDemo 文件，它便是 dSYMDemo.app.dSYM 的核心，我们同样使用 file 命令查看它的文件类型。

```c++
hmc@bogon DWARF % file dSYMDemo 
dSYMDemo: Mach-O universal binary with 2 architectures: [x86_64:Mach-O 64-bit dSYM companion file x86_64] [arm64:Mach-O 64-bit dSYM companion file arm64]
dSYMDemo (for architecture x86_64):    Mach-O 64-bit dSYM companion file x86_64
dSYMDemo (for architecture arm64):    Mach-O 64-bit dSYM companion file arm64
```

&emsp;可看到它是一个 Fat Mach-O 文件，分别对应 x86_64 和 arm64 两个架构。












&emsp;调试数据格式是 **存储有关汇编计算机程序的信息** 供高级调试者使用的一种手段。现代调试数据格式存储了足够的信息，以便进行源级调试。
&emsp;高级调试器需要有关变量、类型、常数、子常规等的信息，因此他们可以在机器级存储和源语言构造之间进行翻译。此类信息也可以由其他软件工具使用。信息必须由编译器生成，并由链接器存储在可执行文件或动态库中。
&emsp;某些对象文件格式包括调试信息，但其他对象可以使用通用调试数据格式，如 stabs 和 DWARF。

&emsp;一般来说，debug 模式构建的 App 会把 Debug 符号表存储在编译好的二进制中，而 release 模式构建的 App 会把 Debug 符号表存储在 .dSYM 文件中以节省二进制体积。

&emsp;在每一次的编译中，Debug 符号表和 App 的二进制通过构建时的 UUID 相互关联。每次构建时都会生成新的唯一标识 UUID，不论源码是否相同仅有 UUID 保持一致的 .dSYM 文件（即使相同的源码，不同次的 build 对应都不同），才能用于解析其堆栈信息。

&emsp;DWARF，即 Debug With Arbitrary Record Format ，是一个标准调试信息格式，即调试信息。单独保存下来就是dSYM文件，即 Debug Symbol File 。使用 MachOView 打开一个二进制文件，就能看到很多 DWARF 的 section，如 __DWARF,__debug_str, __DWARF,__debug_info, __DWARF,__debug_names 等。

## dSYM 文件概述

&emsp;dSYM 是内存地址与函数名、文件名和行号的映射表，一般用来进行崩溃日志分析。

&emsp;<起始地址> <结束地址> <函数> [<文件名: 行号>]

### DWARF 

&emsp;**DWARF** 是一种被众多编译器和调试器使用的用于支持 **源码级别** 调试的 **调试文件格式**，该格式是一个固定的数据格式。dSYM 就是按照 **DWARF** 格式保存调试信息的文件，也就是说 dSYM 是一个文件。

&emsp;**DWARF** 是一种调试信息格式，通常用于源码级别调试，也可用于从运行时地址还原源码对应的符号以及行号的工具（如：atos）。

&emsp;**Strip Linked Product** **Deployment Postprocessing**

## symbolicatecrash

## atos 单/多行符号化

## dwarfdump



## 参考链接
**参考链接:🔗**
+ [漫谈 iOS Crash 收集框架](https://mp.weixin.qq.com/s/hOOzVzJ-nAtkQ8iD-8wVGg)
+ [iOS崩溃异常处理(NSUncaughtExceptionHandler)](https://www.jianshu.com/p/2a8b6c9b5a59)

+ [DWARF调试格式的简介](https://blog.csdn.net/wuhui_gdnt/article/details/7283483/)
+ [iOS查看UUID](https://blog.csdn.net/bianhuanshizhe/article/details/50338797)
+ [使用 Setfile 命令修改 MacOS 文件创建时间 (creation date)，（非 touch 命令），附 Linux 文件时间属性介绍](https://ld246.com/article/1592910201129)







+ [iOS 符号解析重构之路](https://mp.weixin.qq.com/s/TVRYXhiOXIsMmXZo9GmEVA)
+ [dSYM文件的汇编分析](https://juejin.cn/post/6925618080941146125)
+ [iOS的调试文件dSYM与DWARF](https://juejin.cn/post/6983302313586884616)
+ [iOS开发符号表(dSYM)知识总结](http://www.cloudchou.com/android/post-992.html?utm_source=tuicool&utm_medium=referral)
+ [DWARF文件初探——提取轻量符号表](https://juejin.cn/post/6950227054931476516)
+ [dwarf简介](https://blog.csdn.net/helloworlddm/article/details/76785319)
+ [调试 DWARF 和 STAB 格式](https://blog.csdn.net/weixin_34014277/article/details/93052964?utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7EOPENSEARCH%7Edefault-14.no_search_link&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7EOPENSEARCH%7Edefault-14.no_search_link)
+ [mac额外安装命令tree](https://www.jianshu.com/p/75b125ac19d5)
+ [iOS中符号的那些事儿](https://juejin.cn/post/6844904164208689166)
+ [ios dSYM 符号化](https://juejin.cn/post/6995853234975801380)
+ [Bugly iOS 符号表配置](https://bugly.qq.com/docs/user-guide/symbol-configuration-ios/?v=20170912151050#_2)
+ [深入理解Symbol](https://blog.csdn.net/Hello_Hwc/article/details/103330564)

1. 代码健壮性差、单元测试覆盖率低：发生异常
2. 发生异常时的统计上报。
  + 异常分类：`NSSetUncaughtExceptionHandle` 和 `void (*signal(int, void (*)(int)))(int)` 捕获异常。

&emsp;NSException 类学习

```c++
/*
typedef void NSUncaughtExceptionHandler(NSException *exception);

// 获取当前的异常处理函数
FOUNDATION_EXPORT NSUncaughtExceptionHandler * _Nullable NSGetUncaughtExceptionHandler(void);
 
// 设置当前的异常处理函数
FOUNDATION_EXPORT void NSSetUncaughtExceptionHandler(NSUncaughtExceptionHandler * _Nullable);
*/

void uncaughtExceptionHandler(NSException *exception) {
    // 
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    
    // 测试
    // NSException *exception = [[NSException alloc] init];
    
    NSUncaughtExceptionHandler *currentHandler = NSGetUncaughtExceptionHandler();
    NSSetUncaughtExceptionHandler(&uncaughtExceptionHandler);

    return YES;
}
```

&emsp;这里还发现一个点，即使发生了异常，当调用到 `uncaughtExceptionHandler` 函数时，它内部的代码不执行完毕的话，程序是不会退出的，所以我们可以在此函数中把异常数据写入本地，并在下次程序启动时把异常数据上传到服务器去。

```c++
void uncaughtExceptionHandler(NSException *exception) {
    for (int i = 0; i < 5; ++i) {
        sleep(1);
        NSLog(@"🐔🐔🐔 %d", i);
    }
}
```

```c++
2021-10-20 17:25:31.650557+0800 dSYMDemo[39533:359760] 🏵🏵🏵 数组要越界了...
2021-10-20 17:25:32.659840+0800 dSYMDemo[39533:359760] 🏵🏵🏵 0
2021-10-20 17:25:33.661376+0800 dSYMDemo[39533:359760] 🏵🏵🏵 1
2021-10-20 17:25:34.662952+0800 dSYMDemo[39533:359760] 🏵🏵🏵 2
2021-10-20 17:25:35.664537+0800 dSYMDemo[39533:359760] 🏵🏵🏵 3
2021-10-20 17:25:36.666269+0800 dSYMDemo[39533:359760] 🏵🏵🏵 4
2021-10-20 17:25:36.668223+0800 dSYMDemo[39533:359760] *** Terminating app due to uncaught exception 'NSRangeException', reason: '*** -[__NSArrayI objectAtIndexedSubscript:]: index 3 beyond bounds [0 .. 2]'
*** First throw call stack:
(
    0   CoreFoundation                      0x00007fff20421af6 __exceptionPreprocess + 242
    1   libobjc.A.dylib                     0x00007fff20177e78 objc_exception_throw + 48
    2   CoreFoundation                      0x00007fff2049e77f _CFThrowFormattedException + 194
    3   CoreFoundation                      0x00007fff2044341d +[__NSArrayI allocWithZone:] + 0
    4   dSYMDemo                            0x000000010210cccb -[AppDelegate function3] + 107
    5   dSYMDemo                            0x000000010210cc55 -[AppDelegate function2] + 53
    6   dSYMDemo                            0x000000010210cc15 -[AppDelegate function1] + 53
    7   dSYMDemo                            0x000000010210cba4 -[AppDelegate application:didFinishLaunchingWithOptions:] + 132
    8   UIKitCore                           0x00007fff24692fdd -[UIApplication _handleDelegateCallbacksWithOptions:isSuspended:restoreState:] + 232
    9   UIKitCore                           0x00007fff24694b5f -[UIApplication _callInitializationDelegatesWithActions:forCanvas:payload:fromOriginatingProcess:] + 3919
    10  UIKitCore                           0x00007fff2469a56d -[UIApplication _runWithMainScene:transitionContext:completion:] + 1237
    11  UIKitCore                           0x00007fff23cc3730 -[_UISceneLifecycleMultiplexer completeApplicationLaunchWithFBSScene:transitionContext:] + 179
    12  UIKitCore                           0x00007fff2469695a -[UIApplication _compellApplicationLaunchToCompleteUnconditionally] + 59
    13  UIKitCore                           0x00007fff24696ce9 -[UIApplication _run] + 898
    14  UIKitCore                           0x00007fff2469bba8 UIApplicationMain + 101
    15  dSYMDemo                            0x000000010210cff2 main + 114
    16  libdyld.dylib                       0x00007fff2025a3e9 start + 1
)
libc++abi.dylib: terminating with uncaught exception of type NSException
```

