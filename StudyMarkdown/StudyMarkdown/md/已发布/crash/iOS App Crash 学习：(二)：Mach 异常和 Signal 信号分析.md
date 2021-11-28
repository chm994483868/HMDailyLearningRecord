# iOS App Crash 学习：(二)：Mach 异常和 Signal 信号分析

&emsp;Objective-C 的异常处理是指通过 `@try` `@catch`（捕获） 或 `NSSetUncaughtExceptionHandler`（记录） 函数来捕获或记录异常（处理异常），但是这种处理方式对内存访问错误、重复释放等错误引起的 crash 是无能为力的（如野指针访问、MRC 下重复 release 等）。

⬇️⬇️⬇️⬇️⬇️⬇️ 这里需要注意一下： （如野指针访问、MRC 下重复 release 等） 这里是单纯的 Mach 异常（正常情况下都会转化为 signal 信号，但是比如收集到 mach 异常后，直接调用了 `exit()` 函数就会导致程序终止而没有产生对应的 signal 信号），还是 Mach 异常后会发送 signal 信号，后面要验证一下：

这种错误抛出的是 `signal`，所以需要专门做 `signal` 处理）不能得到 signal，如果要处理 signal 需要利用 unix 标准的 signal 机制，注册 `SIGABRT`、`SIGBUS`、`SIGSEGV` 等 signal 发生时的处理函数。

&emsp;例如我们编写如下代码，然后直接运行，程序会直接 crash 中止运行，然后 `NSLog(@"✳️✳️✳️ objc: %@", objc);` 行显示红色的错误信息：`Thread 1: EXC_BAD_ACCESS (code=1, address=0x3402e8d4c25c)` (objc 对象已经被释放，然后 NSLog 语句中又去访问 objc 已经被释放的内存，造成野指针访问) 指出我们的程序此时有一个 `EXC_BAD_ACCESS` 异常，导致退出，且此时可发现我们通过 `NSSetUncaughtExceptionHandler` 设置的 **未捕获异常处理函数** 在程序中止之前并没有得到执行！ 

```c++
__unsafe_unretained NSObject *objc = [[NSObject alloc] init];
NSLog(@"✳️✳️✳️ objc: %@", objc);
```

&emsp;在测试除零操作时（我们都知道 0 不能做除数😂）如下示例代码，发现运行结果与 Build Settings 的 Optimization Level 的选项值有关系，当我们选择 None[-O0] 时会 crash，报出：`Thread 1: EXC_ARITHMETIC (code=EXC_I386_DIV, subcode=0x0)` 错误，而在其他任意 Fast[-O, O1]、Faster[-O2]、Fastest[-O3]、Fastest,Smallest[-Os]、Fastest,Aggressive Optimizations[-Ofast]、Smallest,Aggressive Size Optimizations[-Oz] 选项下程序都正常运行没有 crash 退出，且每次运行 result 的值都是一个很大的随机数。

```c++
int a = 0;
int b = 1;
int result = b / a;
NSLog(@"🏵🏵🏵 %d", result);
```

&emsp;针对上述两段代码导致的 crash，我们在程序退出后在 xcode 底部的调试控制台输入 bt 指令并回车，可看到程序停止运行的原因分别如下：`EXC_ARITHMETIC`、`EXC_BAD_ACCESS`、`signal SIGABRT`。 这里还有一个小细节，就是程序退出大概是某条线程的退出，可以是主线程也可是某条子线程，当我们把上述代码放在一条子线程执行的话，便会看到控制台输出中最后一条是子线程的停止原因。

&emsp;除零操作（EXC_ARITHMETIC）：

```c++
// 当前在主线程 crash
(lldb) bt
* thread #1, queue = 'com.apple.main-thread', stop reason = EXC_ARITHMETIC (code=EXC_I386_DIV, subcode=0x0)
  * frame #0: 0x0000000102d06daf dSYMDemo`-[AppDelegate application:didFinishLaunchingWithOptions:](self=0x0000600002bb82c0, _cmd="application:didFinishLaunchingWithOptions:", application=0x00007fe1dbc06f50, launchOptions=0x0000000000000000) at AppDelegate.m:59:20
  ...
  
// 当前在子线程 crash
(lldb) bt all
  thread #1, queue = 'com.apple.main-thread'
    frame #0: 0x00007fff203b69a4 CoreFoundation`__CFStringHash + 151
    ...
    
* thread #8, queue = 'com.apple.root.default-qos', stop reason = EXC_ARITHMETIC (code=EXC_I386_DIV, subcode=0x0)
  * frame #0: 0x00000001029a0daf dSYMDemo`__57-[AppDelegate application:didFinishLaunchingWithOptions:]_block_invoke(.block_descriptor=0x00000001029a6048) at AppDelegate.m:60:24
  ...
```

&emsp;野指针访问（EXC_BAD_ACCESS）：

```c++
// 当前在主线程 crash
(lldb) bt
* thread #1, queue = 'com.apple.main-thread', stop reason = EXC_BAD_ACCESS (code=1, address=0x539e0d66c11c)
    frame #0: 0x00007fff20190d18 libobjc.A.dylib`objc_opt_respondsToSelector + 16
    ...

// 当前在子线程 crash
thread #1, queue = 'com.apple.main-thread'
  frame #0: 0x00007fff2468f57e UIKitCore`+[_UIBackgroundTaskInfo backgroundTaskAssertionQueue]
  ...
  
* thread #4, queue = 'com.apple.root.default-qos', stop reason = EXC_BAD_ACCESS (code=1, address=0xbc637211426c)
    frame #0: 0x00007fff20190d18 libobjc.A.dylib`objc_opt_respondsToSelector + 16
    ...
```

&emsp;附加一个数组越界的 crash 作为对比（它最终是 `abort` 函数调用 `pthread_kill` 发送了一个 `SIGABRT` 信号停止某条线程后，连带程序中止）：

```c++
// 当前在主线程 crash
(lldb) bt 
* thread #1, queue = 'com.apple.main-thread', stop reason = signal SIGABRT
    frame #0: 0x00007fff61131462 libsystem_kernel.dylib`__pthread_kill + 10
    frame #1: 0x00007fff6116a610 libsystem_pthread.dylib`pthread_kill + 263
    frame #2: 0x00007fff200fab94 libsystem_c.dylib`abort + 120
    ...

// 当前在子线程 crash
(lldb) bt all
  thread #1, queue = 'com.apple.main-thread'
    frame #0: 0x00007fff204318a9 CoreFoundation`-[NSSet anyObject]
    ...

* thread #5, queue = 'com.apple.root.default-qos', stop reason = signal SIGABRT
  * frame #0: 0x00007fff61131462 libsystem_kernel.dylib`__pthread_kill + 10
    frame #1: 0x00007fff6116a610 libsystem_pthread.dylib`pthread_kill + 263
    frame #2: 0x00007fff200fab94 libsystem_c.dylib`abort + 120
    ...
```

&emsp;Objective-C 的异常如果不做任何处理的话（try catch 捕获处理），最终便会触发程序中止退出，此时造成退出的原因是程序向自身发送了 `SIGABRT` 信号。（对于未捕获的 Objective-C 异常，我们可以通过 `NSSetUncaughtExceptionHandler` 函数设置 **未捕获异常处理函数** 在其中记录存储异常日志，然后在 APP 下次启动时进行上传（**未捕获异常处理函数** 函数执行完毕后，程序也同样会被终止，此时没有机会给我们进行网络请求上传数据），如果异常日志记录得当，然后再配合一些异常发生时用户的操作行为数据，那么可以分析和解决大部分的崩溃问题。）

&emsp;上篇我们已经分析过 Objective-C 的异常捕获处理，下面我们开始详细学习 mach 异常和 signal 处理。

```c++
#import "AppDelegate.h"
#import <execinfo.h>

void mySignalHandler(int signal) {
    NSMutableString *mstr = [[NSMutableString alloc] init];
    [mstr appendString:@"Stack:\n"];
    
    void *callstack[128];
    int frames = backtrace(callstack, 128);
    char ** strs = backtrace_symbols(callstack, frames);
    
    printf("🏵🏵🏵 char ** 怎么打印：%p", strs);
    
//    kill(<#pid_t#>, <#int#>)
    
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    
    // 信号量截断
    signal(SIGABRT, mySignalHandler);
    signal(SIGILL, mySignalHandler);
    signal(SIGSEGV, mySignalHandler);
    signal(SIGFPE, mySignalHandler);
    signal(SIGBUS, mySignalHandler);
    signal(SIGPIPE, mySignalHandler);
    
    return YES;
}
```

&emsp;SignalHandler 不要在 debug 环境下测试。因为系统的 debug 会优先去拦截。我们要运行一次后，关闭 debug 状态。应该直接在模拟器上点击我们 build 上去的 App 去运行。而 UncaughtExceptionHandler 可以在调试状态下捕捉。

&emsp;学习 Crash 捕获相关的 **Mach 异常** 和 **signal 信号处理**。

> &emsp;**Mach 为 XNU 的微内核，Mach 异常为最底层的内核级异常。在 iOS 系统中，底层 Crash 先触发 Mach 异常，然后再转换为对应的 Signal 信号**。

&emsp;Darwin 是 macOS 和 iOS 的操作系统，而 XNU 是 Darwin 操作系统的内核部分。XNU 是混合内核，兼具宏内核和微内核的特性，而 Mach 即为其微内核。





### Mach 异常 

######### 总结 Mach 知识点：⬇️

&emsp;Mach（微内核）涉及到的知识点有点多，所以这里我们首先稍微梳理铺垫一下，大概会涉及到：Mach、XNU、BSD、Darwin、Kernel、GUI、NeXTSTEP、macOS 等之间的一些联系或者关系。

&emsp;对 Mach 的维基百科的的介绍进行总结：

&emsp;[BSD](https://zh.wikipedia.org/wiki/BSD)伯克利软件包：Berkeley Software Distribution，缩写：BSD，也被称为伯克利 Unix 或 Berkeley Unix，是一个派生自 Unix（类 Unix）的操作系统，1970 年代由伯克利加州大学的学生比尔·乔伊开创，也被用来代表其派生出的各种包。

&emsp;[Mach](https://zh.wikipedia.org/wiki/Mach)（国际发音：[mʌk]）是一个由卡内基梅隆大学开发的计算机操作系统微内核，Mach 开发项目在卡内基梅隆大学从 1985 年运行到 1994 年，到 Mach 3.0 版结束，其他还有许多人继续 Mach 的研究。Mach 的开发是为了取代 BSD 的 UNIX 核心，所以是许多新的操作系统的设计基础。Mach 的研究至今似乎是停止了，虽然有许多商业化操作系统，如 NEXTSTEP 与 OPENSTEP，特别是 Mac OS X（使用 XNU 核心）都是使用 Mach 或其派生系统。

&emsp;[GUI](https://zh.wikipedia.org/wiki/图形用户界面) 图形用户界面：Graphical User Interface，缩写：GUI，是指采用图形方式显示的计算机操作用户界面。与早期计算机使用的命令行界面相比，除了降低用户的操作负担之外，对于新用户而言，图形界面对于用户来说在视觉上更易于接受，学习成本大幅下降，也让电脑的大众化得以实现。

&emsp;[NeXTSTEP](https://zh.wikipedia.org/wiki/NeXTSTEP)（又写作 NeXTstep、NeXTStep、NEXTSTEP)是由 NeXT.Inc 所开发的操作系统。NeXT 是乔布斯在 1985 年离开苹果公司后所创立的公司。这套系统是以 Mach 和 BSD 为基础，以 Objective-C 作为原生语言，具有很先进的 GUI。1.0 版推出时间是在 1989 年 9 月 18 日。后来苹果电脑在 1997 年 2 月将 NeXT 买下，成为 Mac OS X 的基础。

&emsp;[macOS](https://zh.wikipedia.org/wiki/MacOS)/ˌmækʔoʊˈɛs/ 是苹果公司推出的基于 GUI 的操作系统，为麦金塔（Macintosh，简称 Mac）系列电脑的主操作系统。Classic Mac OS（操作系统，简称 Mac OS，注意这里是没有 X 的）所指的是苹果公司从 1984 年至 2001 年间为麦金塔系列电脑所开发的一系列操作系统，始于 System 1，终结于 Mac OS 9，1997 年，史蒂夫·乔布斯重回苹果公司，经过为期四年的开发，苹果公司于 2001 年以新的操作系统 Mac OS X 取代了 Classic Mac OS。它保留了 Classic Mac OS 的大部分 GUI 设计元素，并且应用程序框架为了兼容性而存在着一些重叠，但这两个操作系统的起源和结构以及底层代码完全不同。简单来说，Mac OS X 它是 Mac OS 版本 10 的分支，然而它与早期发行的 Mac OS 相比，在 Mac OS 的历史上是彻底走向独立发展的。自 2001 年推出起，Mac OS X 这个名字随着时间的推移也发生了一些变化，2001 年至 2011 年间称作 Mac OS X，2012 年至 2015 年称 OS X，2016 年 6 月，苹果公司宣布 OS X 更名为 macOS，以便与苹果其他操作系统 iOS、watchOS 和 tvOS 保持统一的命名风格。

&emsp;[POSIX](https://zh.wikipedia.org/wiki/可移植操作系统接口) 可移植操作系统接口：Portable Operating System Interface，缩写：POSIX，是 IEEE（电气电子工程师学会）为要在各种 UNIX 操作系统上运行软件，而定义 API 的一系列互相关联的标准的总称，其正式称呼为 IEEE Std 1003，而国际标准名称为 ISO/IEC 9945。此标准源于一个大约开始于 1985 年的项目。POSIX 这个名称是由理查德·斯托曼（RMS）应 IEEE 的要求而提议的一个易于记忆的名称。它基本上是 Portable Operating System Interface（可移植操作系统接口）的缩写，而最后一个字母 X 则表明其对 Unix API 的传承。

&emsp;Mac OS X 是与先前的 Mac OS 彻底地分离开来的一个操作系统，它的底层代码与先前版本完全不同。Mac OS X 新的核心名为 Darwin，是一套开放源码、符合 POSIX 标准的操作系统，伴随着标准的 Unix 命令行与其强大的应用工具。macOS 包含两个主要的部分：

1. 核心：名为 Darwin，是以 BSD 源代码和 Mach 微核心为基础，由苹果公司和独立开发者社群合作开发
2. GUI：由苹果公司开发，名为 Aqua 的专利的图形用户界面。（Aqua 是 macOS（旧称 Mac OS X 和 OS X）的 GUI 之商标名称）

&emsp;[IPC](https://zh.wikipedia.org/wiki/行程間通訊) 进程间通信：Inter-Process Communication，缩写：IPC，指至少两个进程或线程间传送数据或信号的一些技术或方法。IPC 对**微内核**和 nano 内核的设计过程非常重要。 微内核减少了内核提供的功能数量，然后通过 IPC 与服务器通信获得这些功能，与普通的宏内核相比，IPC 的数量大幅增加。

&emsp;[系统调用](https://zh.wikipedia.org/wiki/系统调用) 在电脑中，系统调用（英语：system call），指运行在用户空间的程序向操作系统内核请求需要更高权限运行的服务。系统调用提供用户程序与操作系统之间的接口。大多数系统交互式操作需求在内核态执行。如设备 IO 操作或者进程间通信。操作系统的进程空间可分为用户空间（用户态）和内核空间（内核态），它们需要不同的执行权限，其中系统调用运行在内核空间。系统调用和普通库函数调用非常相似，只是系统调用由操作系统内核提供，运行于内核态，而普通的库函数调用由函数库或用户自己提供，运行于用户态。（通过中断实现内核态和用户态的切换，后续还需要学习一下。）

&emsp;[Kernel](https://zh.wikipedia.org/wiki/内核) 内核/核心：Kernel，在计算机科学中是一个用来**管理**软件发出的资料 I/O（输入与输出）要求的电脑程序，将这些要求转译为资料处理的指令并交由中央处理器（CPU）及电脑中其他电子组件进行处理，是现代操作系统中最基本的部分。它是为众多应用程序提供对计算机硬件的安全访问的一部分软件，这种访问是有限的，并由内核决定一个程序在什么时候对某部分硬件操作多长时间。直接对硬件操作是非常复杂的，所以内核通常提供一种硬件抽象的方法，来完成这些操作。有了这个，通过 **进程间通信机制（IPC）** 及 **系统调用**，应用进程可间接控制所需的硬件资源（特别是处理器及 IO 设备）。（内核大概可理解为提供了应用进程和计算机硬件之间的桥梁）

![截屏2021-11-24 下午11.10.44.png](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/e663e3c00bf842c4a203f79effbad3d5~tplv-k3u1fbpfcp-watermark.image?)

&emsp;电脑软硬之间的架构与关系图，可以看到内核进行的是应用软件和计算机硬件的交互工作。

&emsp;内核在设计上可以概分为宏内核与微内核两大架构。在宏内核与微内核之间，进行妥协的设计，这称为混合内核，但是混合内核能否被列为第三大架构，目前仍然有争议。

&emsp;[宏内核](https://zh.wikipedia.org/wiki/整塊性核心) 宏内核结构在硬件之上，定义了一个高阶的抽象接口，应用一组原语（或者叫系统调用（System call））来实现操作系统的功能，例如进程管理，文件系统，和存储管理等等，这些功能由多个运行在内核态的模块来完成。尽管每一个模块都是单独地服务这些操作，但是内核代码是高度集成的，而且难以编写正确。因为所有的模块都在同一个内核空间上运行，一个很小的 bug 都会使整个系统崩溃。然而，如果开发顺利，宏内核结构就可以从运行效率上得到好处。宏内核结构是非常有吸引力的一种设计，由于在同一个地址空间上实现所有复杂的低阶操作系统控制代码的效率会比在不同地址空间上实现更高些。

&emsp;[微内核](https://zh.wikipedia.org/wiki/微內核) 微内核结构由一个非常简单的硬件抽象层和一组比较关键的原语或系统调用组成；这些原语，仅仅包括了创建一个系统必需的几个部分；如线程管理，地址空间和进程间通信等。微内核的目标是将系统服务的实现和系统的基本操作规则分离开来。例如，进程的输入/输出锁定服务可以由运行在微核之外的一个服务组件来提供。这些非常模块化的用户态服务器用于完成操作系统中比较高级的操作，这样的设计使内核中最内核的部分的设计更简单。一个服务组件的失效并不会导致整个系统的崩溃，内核需要做的，仅仅是重新启动这个组件，而不必影响其它的部分。

&emsp;[混合内核](https://zh.wikipedia.org/wiki/混合核心) Hybrid kernel，又称为混合式核心、混合内核，是指一种操作系统内核架构。传统上的操作系统内核可以分为宏内核（Monolithic kernel）与微核心（Micro kernel）两大基本架构，混合核心结合了这两种核心架构，混合核心的基本设计理念，是以微核心架构来设计操作系统核心，但在实现上则采用宏内核的做法。混合核心实质上是微核心，只不过它让一些微核结构执行在用户空间的代码执行在核心空间，这样让核心的执行效率更高些。这是一种妥协做法，设计者参考了微核心结构的系统执行速度不佳的理论。

&emsp;[XNU](https://zh.wikipedia.org/wiki/XNU) XNU 是一个由苹果电脑开发用于 macOS 操作系统的操作系统内核。它是 Darwin 操作系统的一部分，跟随着 Darwin 一同作为自由及开放源代码软件被发布。它还是 iOS、tvOS 和 watchOS 操作系统的内核。XNU 是 X is Not Unix 的缩写。...................

&emsp;[Darwin](https://zh.wikipedia.org/wiki/Darwin_(操作系统)) Darwin 是由苹果公司于 2000 年所发布的一个开放源代码操作系统（2003 年 7 月，苹果在 APSL（Apple Public Source License）的 2.0 版本下发布了 Darwin）。Darwin 是 macOS 和 iOS 操作环境的操作系统部分。Darwin 是一种类 Unix 操作系统，它的内核是 XNU（XNU 是混合内核设计，使其具备了微内核的灵活性和宏内核的性能），其以微核心为基础的核心架构来实现 Mach，而操作系统的服务和用户空间工具则以 BSD 为基础。类似其他类 Unix 操作系统，Darwin 也有对称多处理器的优点，高性能的网络设施和支持多种集成的文件系统。集成 Mach 到 XNU 内核的好处是可携性，或者是在不同形式的系统使用软件的能力。举例来说，一个操作系统核心集成了 Mach 微核心，能够提供多种不同 CPU 架构的二进制格式到一个单一的文件（例如 x86 和 PowerPC），这是因为它使用了 Mach-O 的二进制格式。（到这里就和我们之前学习 Mach-O 的知识点联系起来了）

&emsp;在 mac 电脑的命令终端中执行 uname -r 命令将显示 Darwin 版本号，执行 uname -v 命令将显示 XNU 构建版本的字符串，其中包括 Darwin 的版本号（uname 后面可跟 -a -m -n -p -r -s -v）。执行 system_profiler SPSoftwareDataType 命令将显示 mac 电脑的 software 信息。如下：

```c++
➜  ~ uname -r
21.1.0
➜  ~ uname -v
Darwin Kernel Version 21.1.0: Wed Oct 13 17:33:23 PDT 2021; root:xnu-8019.41.5~1/RELEASE_X86_64
➜  ~ 
```

```c++
hmc@localhost ~ % system_profiler SPSoftwareDataType
Software:

    System Software Overview:

      System Version: macOS 11.2.2 (20D80)
      Kernel Version: Darwin 20.3.0
      Boot Volume: Macintosh HD
      Boot Mode: Normal
      Computer Name: HM的Mac mini
      User Name: HM C (hmc)
      Secure Virtual Memory: Enabled
      System Integrity Protection: Enabled
      Time since boot: 17:56

hmc@localhost ~ % 
```

```c++
➜  ~ system_profiler SPSoftwareDataType
Software:

    System Software Overview:

      System Version: macOS 12.0.1 (21A559)
      Kernel Version: Darwin 21.1.0
      Boot Volume: Macintosh HD
      Boot Mode: Normal
      Computer Name: HM的MacBook Pro
      User Name: HM C (hmc)
      Secure Virtual Memory: Enabled
      System Integrity Protection: Disabled
      Time since boot: 2:25

➜  ~ 
```

![截屏2021-11-20 09.09.20.png](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/eec1dc7e8fe34a82979793b2f6e2463b~tplv-k3u1fbpfcp-watermark.image?)

![截屏2021-11-21 上午8.47.18.png](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/146836b6c7d04a4da5d316b085dafafa~tplv-k3u1fbpfcp-watermark.image?)

&emsp;













&emsp;Mach 引入了 port 的概念用以表示双向的 IPC (进程间通信 Inter-Process Communication），它就像 UNIX 下的文件一样拥有权限信息，使得其安全模型非常接近 UNIX。并且，Mach 使得任何进程都可以拥有一般系统中内核才有的权限，从而允许用户进程实现与硬件交互等操作。

&emsp;Port 机制在 IPC 中的应用该是 Mach 与其他传统内核的一大分野。在 UNIX 下，用户进程调用内核只能通过系统调用或陷入（trap）。用户进程使用一个库安排好数据的位置，然后软件触发一个中断，内核在初始化时会为所有中断设置 handler，因此程序触发中断的时候，控制权就转移到了内核，在一些必要的检查之后即可得以进一步操作。
在 Mach 下，这就交给了 IPC 系统。与直接系统调用不同，这里的用户进程是先向内核申请一个 port 的访问许可，然后利用 IPC 机制向这个 port 发送消息。虽说发送消息的操作同样是系统调用，但 Mach 内核的工作形式有些不同——handler 的工作可以交由其他进程实现。


```c++
(lldb) help process handle
     Manage LLDB handling of OS signals for the current target process. Defaults to showing current policy.

Syntax: process handle <cmd-options> [<unix-signal> [<unix-signal> [...]]]

Command Options Usage:
  process handle [-n <boolean>] [-p <boolean>] [-s <boolean>] [<unix-signal> [<unix-signal> [...]]]

       -n <boolean> ( --notify <boolean> )
            Whether or not the debugger should notify the user if the signal is received.

       -p <boolean> ( --pass <boolean> )
            Whether or not the signal should be passed to the process.

       -s <boolean> ( --stop <boolean> )
            Whether or not the process should be stopped if the signal is received.

If no signals are specified, update them all.  If no update option is specified, list the current values.
     
     This command takes options and free-form arguments.  If your arguments
     resemble option specifiers (i.e., they start with a - or --), you must use
     ' -- ' between the end of the command options and the beginning of the
     arguments.
```


&emsp;`UncaughtExceptionHandlers` 函数执行结束后， abort() -> pthread_kill 抛出的 `SIGABRT` 信号，使用 `signal(SIGABRT, MySignalHandler);` 捕获不到！ 



## 参考链接
**参考链接:🔗**
+ [Mach-维基百科](https://zh.wikipedia.org/wiki/Mach)
+ [iOS 异常信号思考](https://minosjy.com/2021/04/10/00/377/)
+ [Linux 多线程环境下 进程线程终止函数小结](https://www.cnblogs.com/biyeymyhjob/archive/2012/10/11/2720377.html)
+ [pthread_kill引发的争论](https://www.jianshu.com/p/756240e837dd)
+ [线程的信号pthread_kill()函数（线程四）](https://blog.csdn.net/littesss/article/details/71156793)
+ [原子操作atomic_fetch_add](https://www.jianshu.com/p/985fb2e9c201)
+ [iOS Crash 分析攻略](https://zhuanlan.zhihu.com/p/159301707)
+ [Handling unhandled exceptions and signals](https://www.cocoawithlove.com/2010/05/handling-unhandled-exceptions-and.html)


+ [iOS性能优化实践：头条抖音如何实现OOM崩溃率下降50%+](https://mp.weixin.qq.com/s?__biz=MzI1MzYzMjE0MQ==&mid=2247486858&idx=1&sn=ec5964b0248b3526836712b26ef1b077&chksm=e9d0c668dea74f7e1e16cd5d65d1436c28c18e80e32bbf9703771bd4e0563f64723294ba1324&cur_album_id=1590407423234719749&scene=189#wechat_redirect)




+ [iOS Crash之NSInvalidArgumentException](https://blog.csdn.net/skylin19840101/article/details/51941540)
+ [iOS调用reloadRowsAtIndexPaths Crash报异常NSInternalInconsistencyException](https://blog.csdn.net/sinat_27310637/article/details/62225658)
+ [iOS开发质量的那些事](https://zhuanlan.zhihu.com/p/21773994)
+ [NSException抛出异常&NSError简单介绍](https://www.jianshu.com/p/23913bbc4ee5)
+ [NSException:错误处理机制---调试中以及上架后的产品如何收集错误日志](https://blog.csdn.net/lcl130/article/details/41891273)
+ [Exception Programming Topics](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/Exceptions/Exceptions.html#//apple_ref/doc/uid/10000012-BAJGFBFB)
+ [iOS被开发者遗忘在角落的NSException-其实它很强大](https://www.jianshu.com/p/05aad21e319e)
+ [iOS runtime实用篇--和常见崩溃say good-bye！](https://www.jianshu.com/p/5d625f86bd02)
+ [异常处理NSException的使用（思维篇）](https://www.cnblogs.com/cchHers/p/15116833.html)
+ [异常统计- IOS 收集崩溃信息 NSEXCEPTION类](https://www.freesion.com/article/939519506/)
+ [NSException异常处理](https://www.cnblogs.com/fuland/p/3668004.html)
+ [iOS Crash之NSGenericException](https://blog.csdn.net/skylin19840101/article/details/51945558)
+ [iOS异常处理](https://www.jianshu.com/p/1e4d5421d29c)
+ [iOS异常处理](https://www.jianshu.com/p/59927211b745)
+ [iOS crash分类,Mach异常、Unix 信号和NSException 异常](https://blog.csdn.net/u014600626/article/details/119517507?spm=1001.2101.3001.6661.1&utm_medium=distribute.pc_relevant_t0.none-task-blog-2%7Edefault%7ECTRLIST%7Edefault-1.no_search_link&depth_1-utm_source=distribute.pc_relevant_t0.none-task-blog-2%7Edefault%7ECTRLIST%7Edefault-1.no_search_link)
+ [iOS Mach异常和signal信号](https://developer.aliyun.com/article/499180)


