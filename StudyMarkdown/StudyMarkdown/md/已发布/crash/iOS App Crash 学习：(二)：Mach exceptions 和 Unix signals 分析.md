# iOS App Crash 学习：(二)：Mach exceptions 和 Unix signals 分析

## 摘要

&emsp;Objective-C 异常是应用层面的异常，我们可以通过 `@try` `@catch`（捕获）或 `NSSetUncaughtExceptionHandler`（记录）函数来捕获或记录异常（处理异常），这里之所以说 Objective-C 异常是应用层面的异常是因为当发生 Objective-C 异常时只是一个 NSException 对象被 raise/@throw 仅在未捕获处理此 NSException 对象的情况下最终才会导致进程中止，且最终进程中止的过程是系统调用了 abort 函数，abort 内部调用了 (void)pthread_kill(pthread_self(), SIGABRT) 向当前线程发送了 SIGABRT 信号导致了进程中止[abort.c](https://opensource.apple.com/source/Libc/Libc-1439.141.1/stdlib/FreeBSD/abort.c.auto.html)。而 Objective-C 异常之外的例如对内存访问错误、重复释放等错误引起的 Mach 异常需要通过其他方式进行处理（如野指针访问、MRC 下重复 release 等会造成 EXC_BAD_ACCESS 类型的 Mach 异常导致进程中止。直接调用 abort 函数其内部调用 pthread_kill 对当前线程发出 SIGABRT 信号后进程被中止，或者我们也可以自己手动调用 pthread_kill(pthread_self(), SIGSEGV) 中止进程此时我们便可以收到 Signal 回调），本篇我们便开始学习 Mach 异常处理和 signal 信号处理。

&emsp;Objective-C 异常处理过程：

+ 生成异常对象: @try 中出现异常，系统会生成一个 NSException 异常对象，该对象提交到系统中，系统就会抛出异常；
+ 异常处理流程: 运行环境接收到 NSException 异常对象时，如果存在能处理该异常对象的 @catch 代码块，就将该异常对象交给 @catch 处理，该过程就是捕获异常，如果没有 @catch 代码块处理异常，程序就会终止（abort()）；
+ @catch 代码块捕获过程: 运行环境接收到异常对象时, 会依次判断该异常对象类型是否是 @catch 代码块中异常或其子类实例, 如果匹配成功, 被匹配的 @catch 就会处理该异常, 都则就会跟下一个 @catch 代码块对比;
+ @catch 处理异常: 系统将 NSException 异常对象传递给 @catch 形参，@catch 通过该形参获取 NSException 异常对象详细信息，可进行一些处理后使进程继续执行，也可以调用 raise/@throw 继续抛出 NSException 异常对象使用程序终止。

## Objective-C 异常、Mach 异常、Signal 信号之间的一些联系 

&emsp;Objective-C 异常（NSException）是应用层面的异常，它与其他两者的最大区别就是 Mach 异常与 Unix 信号是硬件层面的异常，NSException 是软件层面的异常，且它们三者中两者之间有一些迁移转化关系。

+ 当发生 Objective-C 异常，且不进行捕获时，最终程序会因当前线程收到 `SIGABRT` 信号而终止，此时我们只能使用 try catch 或 NSSetUncaughtExceptionHandler 来记录处理 NSException 异常，而最终程序终止时抛出的 `SIGABRT` 信号，我们使用 `signal(SIGABRT, SignalHandler);` 是抓取不到 Signal 回调的，如果我们自己在程序中手动调用 abort() 则可以抓取到 `SIGABRT` 信号。（NSException -> Signal）

+ 一般情况下 Mach 异常会转换成 Signal，但是比如收集到 Mach 异常后，直接调用了 `exit()` 函数就会导致进程退出而没有产生对应的 signal 信号，又或者 Mach 异常还没转换成 Signal，程序就已经被杀死了（如死循环导致的内存溢出），这时候就无法捕获 Signal 了。（Mach -> Signal）
+ 有些异常不会经过 Mach Exception，也不会被 NSException 捕获，只能通过 Signal 捕获，原因是底层直接调用了 `__pthread_kill` 函数直接向某条线程发送了 Signal。

&emsp;如果要处理 signal 需要利用 unix 标准的 signal 机制，注册 `SIGABRT`、`SIGBUS`、`SIGSEGV` 等 signal 发生时的处理函数。

## Mach 异常示例 

&emsp;我们编写如下代码，然后直接运行，程序会直接 crash 中止运行，然后 `NSLog(@"✳️✳️✳️ objc: %@", objc);` 行显示红色的错误信息：`Thread 1: EXC_BAD_ACCESS (code=1, address=0x3402e8d4c25c)` (objc 对象已经被释放，然后 NSLog 语句中又去访问 objc 已经被释放的内存，造成野指针访问) 指出我们的程序此时有一个 `EXC_BAD_ACCESS` 异常，它就是一个标准的 Mach 异常导致进程退出，且此时可发现我们通过 `NSSetUncaughtExceptionHandler` 设置的 **未捕获异常处理函数** 在进程中止之前并没有得到执行，以及通过 `void(*signal(int, void (*)(int)))(int)` 设置的 Signal 信号处理函数也没有得到执行，因为此时的异常就仅是一个 Mach 异常，我们使用 Objective-C 异常处理和 Signal 信号处理是抓不到它的。 

```c++
__unsafe_unretained NSObject *objc = [[NSObject alloc] init];
NSLog(@"✳️✳️✳️ objc: %@", objc);
```

&emsp;在测试除零操作时（我们都知道 0 不能做除数）如下示例代码，发现运行结果与 Build Settings 的 Optimization Level 的选项值有关系，当我们选择 None[-O0] 时会 crash，报出：`Thread 1: EXC_ARITHMETIC (code=EXC_I386_DIV, subcode=0x0)` 异常，此时也是一个标准的 Mach 异常，而在其他任意 Fast[-O, O1]、Faster[-O2]、Fastest[-O3]、Fastest,Smallest[-Os]、Fastest,Aggressive Optimizations[-Ofast]、Smallest,Aggressive Size Optimizations[-Oz] 选项下程序都正常运行没有 crash 退出，且每次运行 result 的值都是一个很大的随机数。

```c++
int a = 0;
int b = 1;
int result = b / a;
NSLog(@"🏵🏵🏵 %d", result);
```

&emsp;针对上述两段代码导致的 crash，我们在程序退出后在 xcode 底部的调试控制台输入 bt 指令并回车，可看到程序停止运行的原因分别是：`EXC_BAD_ACCESS`、`EXC_ARITHMETIC`，作为对比，数组越界访问时则是：`signal SIGABRT`， 这里还有一个小细节，就是程序退出其实是某条线程的退出导致的，可以是主线程也可是某条子线程，当我们把上述代码放在一条子线程执行的话，便会看到控制台输出中是一条子线程的停止原因。

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

&emsp;Objective-C 的异常如果不做任何处理的话（try catch 捕获处理），最终便会触发程序中止退出，此时造成退出的原因是程序向自身发送了 `SIGABRT` 信号。（对于未捕获的 Objective-C 异常，我们可以通过 `NSSetUncaughtExceptionHandler` 函数设置 **未捕获异常处理函数** 在其中记录存储异常日志，然后在 APP 下次启动时进行上传（**未捕获异常处理函数** 函数执行完毕后，程序也同样会被中止，此时没有机会给我们进行网络请求上传数据），如果异常日志记录得当，然后再配合一些异常发生时用户的操作行为数据，那么可以分析和解决大部分的崩溃问题。）

## Mach 概述（Mach 是什么） 

&emsp;Mach（微内核）涉及到的知识点有点多，所以这里我们首先稍微梳理铺垫一下，大概会涉及到：BSD、Mach、GUI、NeXTSTEP、macOS、POSIX、IPC、system call、Kernel、宏内核、微内核、混合内核、XNU、Darwin 等以及他们之间的一些联系或者关系。

&emsp;下面我们对维基百科中对以上名词的介绍进行摘录：

&emsp;[BSD](https://zh.wikipedia.org/wiki/BSD)伯克利软件包：Berkeley Software Distribution，缩写：BSD，也被称为伯克利 Unix 或 Berkeley Unix，是一个派生自 Unix（类 Unix）的操作系统，1970 年代由伯克利加州大学的学生比尔·乔伊开创，也被用来代表其派生出的各种包。

&emsp;[Mach](https://zh.wikipedia.org/wiki/Mach)（国际发音：[mʌk]）是一个由卡内基梅隆大学开发的计算机操作系统微内核，Mach 开发项目在卡内基梅隆大学从 1985 年运行到 1994 年，到 Mach 3.0 版结束，其他还有许多人继续 Mach 的研究。Mach 的开发是为了取代 BSD 的 UNIX 核心，所以是许多新的操作系统的设计基础。Mach 的研究至今似乎是停止了，虽然有许多商业化操作系统，如 NEXTSTEP 与 OPENSTEP，特别是 Mac OS X（使用 XNU 核心）都是使用 Mach 或其派生系统。

&emsp;[GUI](https://zh.wikipedia.org/wiki/图形用户界面) 图形用户界面：Graphical User Interface，缩写：GUI，是指采用图形方式显示的计算机操作用户界面。与早期计算机使用的命令行界面相比，除了降低用户的操作负担之外，对于新用户而言，图形界面对于用户来说在视觉上更易于接受，学习成本大幅下降，也让电脑的大众化得以实现。

&emsp;[NeXTSTEP](https://zh.wikipedia.org/wiki/NeXTSTEP)（又写作 NeXTstep、NeXTStep、NEXTSTEP)是由 NeXT.Inc 所开发的操作系统。NeXT 是乔布斯在 1985 年离开苹果公司后所创立的公司。这套系统是以 Mach 和 BSD 为基础，以 Objective-C 作为原生语言，具有很先进的 GUI。1.0 版推出时间是在 1989 年 9 月 18 日。后来苹果电脑在 1997 年 2 月将 NeXT 买下，成为 Mac OS X 的基础。

&emsp;[macOS](https://zh.wikipedia.org/wiki/MacOS)/ˌmækʔoʊˈɛs/ 是苹果公司推出的基于 GUI 的操作系统，为麦金塔（Macintosh，简称 Mac）系列电脑的主操作系统。Classic Mac OS（操作系统，简称 Mac OS，注意这里是没有 X 的）所指的是苹果公司从 1984 年至 2001 年间为麦金塔系列电脑所开发的一系列操作系统，始于 System 1，终结于 Mac OS 9，1997 年，史蒂夫·乔布斯重回苹果公司，经过为期四年的开发，苹果公司于 2001 年以新的操作系统 Mac OS X 取代了 Classic Mac OS。它保留了 Classic Mac OS 的大部分 GUI 设计元素，并且应用程序框架为了兼容性而存在着一些重叠，但这两个操作系统的起源和结构以及底层代码完全不同。简单来说，Mac OS X 它是 Mac OS 版本 10 的分支，然而它与早期发行的 Mac OS 相比，在 Mac OS 的历史上是彻底走向独立发展的。自 2001 年推出起，Mac OS X 这个名字随着时间的推移也发生了一些变化，2001 年至 2011 年间称作 Mac OS X，2012 年至 2015 年称 OS X，2016 年 6 月，苹果公司宣布 OS X 更名为 macOS，以便与苹果其他操作系统 iOS、watchOS 和 tvOS 保持统一的命名风格。

&emsp;这里补一张 macOS 架构图：

![截屏2021-12-09 上午12.03.48.png](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/c2746a6b912a451382441c50261dd936~tplv-k3u1fbpfcp-watermark.image?)

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

&emsp;[XNU](https://zh.wikipedia.org/wiki/XNU) XNU 是一个由苹果电脑开发用于 macOS 操作系统的操作系统内核。它是 Darwin 操作系统的一部分，跟随着 Darwin 一同作为自由及开放源代码软件被发布。它还是 iOS、tvOS 和 watchOS 操作系统的内核。XNU 是 X is Not Unix 的缩写。XNU 最早是 NeXT 公司为了 NeXTSTEP 操作系统而发展的。它是一种混合式核心（Hybrid kernel），结合了由卡内基美隆大学发展的 Mach 2.5 版，4.3 BSD，与称为 Driver Kit 的面向对象程序设计应用程序界面。在苹果电脑收购 NeXT 公司之后，XNU 的 Mach 微内核被升级到 Mach 3.0，BSD 的部分升级至 FreeBSD，Driver Kit 则改成 I/O Kit，一套以 C++ 撰写的应用程序界面。XNU 是一个混合内核，将宏内核与微内核两者的特性兼收并蓄，以期同时拥有两种内核的优点————比如在微内核中提高操作系统模块化程度以及让操作系统更多的部分接受内存保护的消息传递机制，和宏内核在高负荷下表现的高性能。到 2007 年为止，XNU 支持单核和具有对称多处理的 ARM，IA-32 和 x86-64 处理器。在第 10 版（即 Mac OS X 10.6）之后，不再支持 PowerPC。

&emsp;**XNU 中的 Mach:** XNU 内核以一个被深度定制的 Mach 3.0 内核作为基础。如此这般，它便可以把操作系统的核心部分作为独立的进程运行，由此带来极大的灵活性（Mach 核心之上可平行运行多个操作系统）。但是因为 **内核态/用户态的上下文切换** 会额外消耗时间，同时内核与服务进程之间的消息传递也会降低运行效率，所以这种设计通常会降低性能。为了提高效率，在 Mac OS X 中 BSD 部分与 Mach 一起内建于核心部分。深度定制的 “混合” Mach 3.0 内核与传统 BSD 内核聚变一体的产物就是一个 “混合” 内核，同时具有两者的优点与缺点。

&emsp;[Darwin](https://zh.wikipedia.org/wiki/Darwin_(操作系统)) Darwin 是由苹果公司于 2000 年所发布的一个开放源代码操作系统（2003 年 7 月，苹果在 APSL（Apple Public Source License）的 2.0 版本下发布了 Darwin）。Darwin 是 macOS 和 iOS 操作环境的操作系统部分。Darwin 是一种类 Unix 操作系统，它的内核是 XNU（XNU 是混合内核设计，使其具备了微内核的灵活性和宏内核的性能），其以微核心为基础的核心架构来实现 Mach，而操作系统的服务和用户空间工具则以 BSD 为基础。类似其他类 Unix 操作系统，Darwin 也有对称多处理器的优点，高性能的网络设施和支持多种集成的文件系统。集成 Mach 到 XNU 内核的好处是可携性，或者是在不同形式的系统使用软件的能力。举例来说，一个操作系统核心集成了 Mach 微核心，能够提供多种不同 CPU 架构的二进制格式到一个单一的文件（例如 x86 和 PowerPC），这是因为它使用了 Mach-O 的二进制格式。（到这里就和我们之前学习 Mach-O 的知识点联系起来了）

&emsp;在 mac 电脑的命令终端中执行 `uname -r` 命令将显示 Darwin 版本号，执行 `uname -v` 命令将显示 XNU 构建版本的字符串，其中包括 Darwin 的版本号（uname 后面可跟 -a -m -n -p -r -s -v）。执行 `system_profiler SPSoftwareDataType` 命令将显示 mac 电脑的 software 信息。如下输出：

```c++
➜  ~ uname -r
21.1.0
➜  ~ uname -v
Darwin Kernel Version 21.1.0: Wed Oct 13 17:33:23 PDT 2021; root:xnu-8019.41.5~1/RELEASE_X86_64
➜  ~ 
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

&emsp;看到这里的话我们大概就可以对 Mach 的位置进行一个总结了：Darwin 是 macOS 和 iOS 操作环境的操作系统部分，它的内核是 XNU，XNU 是混合内核设计，使其具备了微内核的灵活性和宏内核的性能，而 XNU 内核的微内核部分便是一个被深度定制的 Mach 3.0 内核，所以看到这里我们便可理解那句 **Mach 异常为最底层的内核级异常**。

### 从《Kernel Programming Guide》中学习 Mach 

&emsp;下面我们再过一下 [《Kernel Programming Guide》](https://developer.apple.com/library/archive/documentation/Darwin/Conceptual/KernelProgramming/Mach/Mach.html?spm=a2c6h.12873639.0.0.15ee7113vyXhuI#//apple_ref/doc/uid/TP30000905-CH209-TPXREF102) 文档中的 Mach 概述部分，加深对 Mach 微内核的理解。

#### Mach Overview

&emsp;OS X 内核的基本服务和原语（fundamental services and primitives）基于 Mach 3.0。Apple 已经修改并扩展了 Mach，以更好地满足 OS X 的功能和性能目标。Mach 3.0 最初被设想为一个简单，可扩展的通信微内核。它能够作为独立内核运行，而其他传统操作系统服务（如 I/O、文件系统和网络堆栈）则作为用户模式服务运行。

&emsp;但是，在 OS X 中，Mach 是与其他内核组件链接到单个内核地址空间中的，这主要是为了性能。在链接组件之间进行直接调用比在单独的任务之间发送消息或执行远程过程调用 remote procedure calls（RPC） 要快得多，这种模块化结构使系统比单片内核所允许的更强大，更具可扩展性，而不会受到纯微内核的性能损失（因通信造成的性能损失）。

&emsp;因此，在 OSX 中，Mach 主要不是客户端和服务器之间的通信枢纽。相反，它的价值在于它的抽象性、可扩展性和灵活性。特别是，Mach 提供了:

+ 以通信通道（communication channels）（例如 ports）作为对象引用的基于对象的 API
+ 高度并行执行，包括抢占式调度线程和对 SMP 的支持
+ 灵活的调度框架，支持实时使用
+ 一整套 IPC 原语，包括消息传递、RPC、同步和通知
+ 支持大型虚拟地址空间、共享内存区域和由持久存储支持的内存对象
+ 经验证的可扩展性和可移植性，例如跨指令集体系结构和在分布式环境中
+ 安全和资源管理是设计的基本原则；所有资源都是虚拟化的

#### Mach Kernel Abstractions

&emsp;Mach 提供了一小部分抽象（Abstractions），这些抽象（Abstractions）被设计为既简单又强大。以下是主要的内核抽象（Kernel Abstractions）：

+ Tasks。资源所有权的单位；每个任务由一个虚拟地址空间、一个端口权限命名空间和一个或多个线程组成。（类似于 process）
+ Threads。任务中的 CPU 执行单位。
+ Address space。Mach 与内存管理器一起实现了 sparse 虚拟地址空间和共享内存的概念。
+ Memory objects。内存管理的内部单元。内存对象包括命名的条目和区域；它们是可能映射到地址空间的潜在持久数据的表示。
+ Ports。安全的单工通信通道，只能通过发送和接收功能（称为端口权限）访问。
+ IPC。消息队列、远程过程调用、通知、信号量和锁集。
+ Time。时钟、计时器和等待。

&emsp;在 trap 级别，大多数 Mach 抽象的接口由发送到和来自表示这些对象的内核端口的消息组成。trap-level 接口（如 mach_msg_overwrite_trap）和消息格式本身在正常使用中由 Mach 接口生成器（MIG）抽象。MIG 用于根据对基于消息的 API 的描述，编译这些 API 的过程接口。

#### Tasks and Threads

&emsp;OS X 进程和 POSIX 线程（pthreads）分别在 Mach 任务和线程之上实现。线程是任务中的控制流点，存在一个任务，用于为其包含的线程提供资源，进行此拆分是为了提供并行性和资源共享。

&emsp;A thread

+ 是 task 中控制流的点。
+ 可以访问包含 task 的所有元素。
+ 与其他线程并行执行（可能），甚至同一任务中的线程。
+ 具有最小的状态信息，以实现低开销。

&emsp;A task

+ 是系统资源的集合。这些资源（地址空间除外）由端口引用。如果端口的权限是这样分配的，则这些资源可以与其他任务共享。
+ 提供一个大的、可能稀疏的地址空间，由虚拟地址引用。该空间的一部分可以通过继承或外部内存管理来共享。
+ 包含一定数量的线程。

&emsp;请注意，任务本身没有生命周期，只有线程执行指令。当说 "任务 Y 做 X" 时，真正的意思是 "任务 Y 中包含的线程做 X"。

&emsp;任务是一个相当昂贵的实体，它是一个资源集合。任务中的所有线程共享所有内容。如果没有显式操作（尽管操作通常很简单），两个任务将不会共享任何内容，并且一些资源（例如端口接收权限）根本无法在两个任务之间共享。
 
&emsp;线程是一个相当轻量级的实体。它的创建成本相当低，而且操作开销也很低。这是正确的，因为线程几乎没有状态信息（主要是它的寄存器状态）。它所拥有的任务承担着资源管理的负担。在多处理器计算机上，任务中的多个线程可以并行执行。即使并行性不是目标，多个线程也有一个优势，即每个线程都可以使用同步编程风格，而不是使用一个线程尝试提供多个服务来尝试异步编程。

&emsp;线程是基本的计算实体。一个线程只属于一个定义其虚拟地址空间的任务。要影响地址空间的结构或引用地址空间以外的任何资源，线程必须执行特殊的陷阱指令，该指令使内核代表线程执行操作，或代表线程向某个代理发送消息。通常，这些陷阱操作与包含线程的任务相关联的资源。可以请求内核来操作这些实体：创建它们、删除它们，并影响它们的状态。

&emsp;Mach 为线程调度策略提供了一个灵活的框架。OS X 的早期版本支持分时和固定优先级策略。提高和降低分时线程的优先级，以平衡其资源消耗与其他分时线程之间的关系。

&emsp;固定优先级线程执行一段时间，然后放在具有相同优先级的线程队列的末尾。将固定优先级线程的量子级别设置为无穷大将允许该线程运行，直到它阻塞，或者直到它被更高优先级的线程抢占。高优先级实时线程通常是固定优先级的。

&emsp;OS X 还为实时性能提供时间约束调度。此调度允许你指定线程必须在特定时间段内获得特定时间量。

&emsp;Mach 调度在 [Mach Scheduling and Thread Interfaces](https://developer.apple.com/library/archive/documentation/Darwin/Conceptual/KernelProgramming/scheduler/scheduler.html#//apple_ref/doc/uid/TP30000905-CH211-BEHJDFCA) 中进一步描述。

#### Ports, Port Rights, Port Sets, and Port Namespaces

&emsp;除了任务的虚拟地址空间之外，所有其他 Mach 资源都是通过称为端口（port）的间接寻址级别访问的。端口是请求服务的客户端和提供服务的服务器之间单向通信通道的端点。如果要为此类服务请求提供回复，则必须使用第二个端口。这相当于 UNIX 术语中的（单向）管道。

&emsp;在大多数情况下，由端口访问的资源（即由它命名的）称为对象。大多数由端口命名的对象都有一个接收器和（可能）多个发送器。也就是说，对于消息队列这样的典型对象，只有一个接收端口和至少一个发送端口。

&emsp;对象提供的服务由接收发送到对象的请求的管理器确定。因此，内核是与内核提供的对象关联的端口的接收器，而与任务提供的对象关联的端口的接收器是提供这些对象的任务。

&emsp;对于为任务提供的对象命名的端口，可以将该端口的请求接收者更改为不同的任务，例如通过在消息中将该端口传递给该任务。单个任务可能有多个引用其支持的资源的端口。因此，任何给定的实体都可以有多个表示它的端口，每个端口都表示不同的允许操作集。例如，许多对象具有名称端口和控制端口（有时称为特权端口）。访问控制端口允许操作对象；对名称端口的访问只是命名对象，以便你可以获取有关该对象的信息或对其执行其他非特权操作。

&emsp;任务具有以特定方式访问端口的权限（发送、接收、发送一次）；这些被称为端口权限。只能通过右键访问端口。端口通常用于授予客户端对 Mach 内对象的访问权限。有权发送到对象的 IPC 端口表示有权以规定的方式操作对象。因此，端口权限所有权是 Mach 的基本安全机制。拥有对某个对象的权限就是拥有访问或操作该对象的能力。

&emsp;可以通过 IPC 在任务之间复制和移动端口权限。这样做实际上会将功能传递给某些对象或服务器。

&emsp;端口引用的一种对象类型是端口集。顾名思义，端口集是一组端口权限，当从该组的任何成员接收消息或事件时，可以将其视为单个单元。端口集允许一个线程等待多个消息和事件源，例如在工作循环中。

&emsp;传统上，在 Mach 中，由端口表示的通信通道始终是消息队列。但是，OS X 支持其他类型的通信通道，这些新类型的 IPC 对象也由端口和端口权限表示。有关消息和其他 IPC 类型的更多详细信息，请参阅进程间通信 （IPC） 部分。

&emsp;端口和端口权限没有允许直接操作任意端口或权限的系统范围的名称。仅当任务在其 port 命名空间中具有端口权限时，任务才能操作端口。端口权限由端口名指定，端口名是 32 位端口命名空间的整数索引。每个任务都有一个与之关联的端口命名空间。

&emsp;当另一个任务将端口权限显式插入其命名空间时，当任务在消息中接收权限时，通过创建返回对象权限的对象，以及通过 Mach 调用某些特殊端口（mach_thread_self、mach_task_self 和 mach_reply_port）时，任务会获取端口权限。

#### Memory Management

&emsp;与大多数现代操作系统一样，Mach 为大型稀疏的虚拟地址空间提供寻址。运行时访问是通过虚拟地址进行的，这些虚拟地址可能与尝试访问的初始时间物理内存中的位置不对应。Mach 负责获取请求的虚拟地址，并在物理内存中为其分配相应的位置。它通过需求分页来实现这一点。

&emsp;将内存对象映射到虚拟地址空间的范围时，将使用数据填充该范围。地址空间中的所有数据最终都通过内存对象提供。在物理内存中建立页面时，Mach 会向内存对象（寻呼机）的所有者询问页面的内容，并在回收页面之前将可能修改的数据返回给寻呼机。OS X 包括两个内建寻呼机，即默认寻呼机和 vnode 寻呼机。

&emsp;默认寻呼机处理非持久内存，称为匿名内存。匿名内存初始化为零，并且仅在任务的生命周期内存中存在。vnode 寻呼机将文件映射到内存对象。Mach 将接口导出到内存对象，以允许用户模式任务提供其内容。此接口称为外部内存管理接口或 EMMI。

&emsp;内存管理子系统导出称为命名条目或命名内存条目的虚拟内存句柄。与大多数内核资源一样，这些资源由端口表示。具有命名的内存输入句柄允许所有者映射基础虚拟内存对象或传递将基础对象映射到其他对象的权限。在两个不同的任务中映射命名条目会导致在两个任务之间产生一个共享内存窗口，从而为建立共享内存提供了一种灵活的方法。

&emsp;从 OS X v10.1 开始，EMMI 系统得到了增强，以支持 "无端口" EMMI。在传统的 EMMI 中，为每个内存区域创建两个 Mach 端口，同样为每个缓存的 vnode 创建两个端口。无端口 EMMI 在其初始实现中将其替换为直接内存引用（基本上是指针）。在将来的版本中，端口将用于与内核外部的寻呼机进行通信，同时使用直接引用与驻留在内核空间中的寻呼机进行通信。这些更改的最终结果是，无端口 EMMI 的早期版本不支持在内核空间之外运行的寻呼机。预计此支持将在将来的版本中恢复。

&emsp;虚拟内存空间的地址范围也可以通过直接分配（使用 vm_allocate）来填充。基础虚拟内存对象是匿名的，并由默认寻呼机提供支持。地址空间的共享范围也可以通过继承进行设置。创建新任务时，将从父级克隆这些任务。此克隆也与基础内存地址空间有关。对象的映射部分可以作为副本继承，也可以作为共享继承，或者根本不继承，具体取决于与映射关联的属性。Mach 实践一种称为写入时复制的延迟复制形式，以优化任务创建时继承副本的性能。

&emsp;不是直接复制范围，而是通过受保护的共享实现写入时复制优化。这两个任务共享要复制的内存，但具有只读访问权限。当任一任务尝试修改范围的一部分时，此时将复制该部分。这种对内存副本的延迟评估是一项重要的优化，它允许在多个方面进行简化，尤其是消息传递 API。

&emsp;Mach 通过导出命名区域提供了另一种形式的共享。命名区域是命名条目的一种形式，但不是由虚拟内存对象支持，而是由虚拟映射片段支持。此片段可能包含到大量虚拟内存对象的映射。它可以映射到其他虚拟映射中，从而不仅提供了一种继承一组虚拟内存对象，还继承其现有映射关系的方法。此功能在任务设置中提供了显著的优化，例如，在共享用于共享库的地址空间的复杂区域时。

#### Interprocess Communication (IPC)

&emsp;任务之间的通信是 Mach philosophy（哲学）的重要元素。Mach 支持客户端/服务器系统结构，其中任务（客户端）通过通信通道发送的消息向其他任务（服务器）发出请求来访问服务。

&emsp;这些通信信道在 Mach 中的端点称为 ports，而 port rights 表示使用该信道的权限。Mach 提供的 IPC 形式包括:

+ message queues
+ semaphores
+ notifications
+ lock sets
+ remote procedure calls (RPCs)

&emsp;端口所表示的 IPC 对象的类型决定了该端口上允许的操作，以及数据传输的发生方式（以及是否发生）。

> Important: OS X 中的 IPC 设备处于过渡状态。在系统的早期版本中，并非所有这些 IPC 类型都可以实现。

&emsp;有两个根本不同的 Mach API 用于端口的原始操作 — mach_ipc 系列和 mach_msg 系列。在合理范围内，两个系列都可以与任何 IPC 对象一起使用，但是在新代码中首选 mach_ipc 调用。mach_ipc 调用在适当的情况下维护状态信息，以支持事务的概念。旧代码支持 mach_msg 调用，但已弃用，它们是 stateless。

#### IPC Transactions and Event Dispatching

&emsp;当线程调用 mach_ipc_dispatch 时，它会重复处理在注册端口集上传入的事件。这些事件可以是来自 RPC 对象的参数块（作为客户端调用的结果）、正在获取的锁定对象（由于某些其他线程释放锁的结果）、正在发布的通知或信号量，或者来自传统消息队列的消息。

&emsp;这些事件通过 mach_msg_dispatch 的标注进行处理。某些事件意味着在标注的生存期内存在事务。对于锁，状态是锁的所有权。当标注返回时，将释放锁。对于远程过程调用，状态是客户端的标识、参数块和应答端口。当标注返回时，将发送答复。当标注返回时，事务（如果有）完成，线程等待下一个事件。mach_ipc_dispatch 设施旨在支持工作循环。

#### Message Queues

&emsp;最初，Mach 中进程间通信的唯一样式是消息队列。只有一个任务可以保留表示消息队列的端口的接收权限。允许此任务从端口队列接收（读取）消息。多个任务可以拥有对端口的权限，这些权限允许它们将消息发送（写入）到队列中。

&emsp;一个任务通过构建包含一组数据元素的数据结构，然后在它拥有发送权限的端口上执行消息发送操作，与另一个任务进行通信。稍后，具有该端口接收权限的任务将执行消息接收操作。

&emsp;消息可能包含以下部分或全部内容：

+ 纯数据
+ 内存范围的副本
+ 端口权限
+ 内核隐式属性，例如发送方的安全令牌

&emsp;消息传输是一个异步操作。消息在逻辑上复制到接收任务中，可能具有写入时复制优化。接收任务中的多个线程可以尝试从给定端口接收消息，但只有一个线程可以接收任何给定消息。

#### Semaphores

&emsp;信号量 IPC 对象支持等待、提交和提交所有操作。这些是计数信号量，因为如果该信号量的等待队列中当前没有线程正在等待，则将保存（计数）帖子。post all 操作将唤醒所有当前正在等待的线程。

#### Notifications

&emsp;与信号量一样，通知对象也支持发布和等待操作，但添加了状态字段。状态是在创建通知对象时定义的固定大小、固定格式的字段。每个帖子更新状态字段，每个帖子都覆盖了一个状态。

#### Locks

&emsp;锁是提供对关键部分的互斥访问的对象。锁的主要接口是面向事务的（请参见 IPC 事务和事件调度）。在事务期间，线程持有锁。当它从事务返回时，将释放锁。

#### Remote Procedure Call (RPC) Objects

&emsp;顾名思义，RPC 对象旨在促进和优化远程过程调用。RPC 对象的主要接口是面向事务的（请参见 IPC 事务和事件调度）

&emsp;创建 RPC 对象时，将定义一组参数块格式。当客户端进行 RPC（对象上的发送）时，它会导致在对象上创建预定义格式之一的消息并排队，然后最终传递给服务器（接收方）。当服务器从事务返回时，应答将返回给发送方。Mach 尝试通过使用客户端的资源执行服务器来优化事务，这称为线程迁移。

#### Time Management

&emsp;以 Mach 为单位的传统时间抽象是时钟，它提供了一组基于 mach_timespec_t 的异步报警服务。有一个或多个时钟对象，每个对象定义一个单调递增的时间值，以纳秒为单位表示。实时时钟是内置的，是最重要的，但系统中可能有其他时钟用于其他时间概念。时钟支持获取当前时间、在给定时间段内休眠、设置闹钟（在给定时间发送的通知）等操作。

&emsp;mach_timespec_t API 在 OS X 中已弃用。较新和首选的 API 基于计时器对象，这些对象又使用 AbsoluteTime 作为基本数据类型。AbsoluteTime 是一种依赖于计算机的类型，通常基于平台本机时基。提供了例程，用于将 AbsoluteTime 值与其他数据类型（如纳秒）相互转换。定时器对象支持异步、无漂移通知、取消和过早警报。它们比时钟更有效，并且允许更高的分辨率。

&emsp;文档还是蛮晦涩的，只能先试着去理解了，上面提到 Mach 通信使用的 port，如果大家还有印象的话，在 Runloop 的学习中我们见到过很多次 port 端口，Runloop 的唤醒等操作，都是通过 port 来通信完成的，CFRunLoopSource 中的 Source1 内部基于 port 来实现的。(Source1：包含了一个 mach_port 和一个回调（函数指针），被用于通过内核和其他线程相互发送消息，这种 Source 能主动唤醒 RunLoop 的线程。)

## Mach 中的通信机制：port

&emsp;这一小节我们学习下 port(端口)，提到这个我们大概最先想到的就是 runloop 中的基于 port 的 source1 以及 iOS 中基于 port 的线程间通信。这里我们从简单着手，先不着眼于 mach_port_t，首先我们看下在 cocoa 中使用的 NSMachPort，下面我们通过一些示例代码回顾一下在 iOS 中使用 NSMachPort 进行线程间通信。

### NSMachPort 使用示例

&emsp;NSMachPort 是 NSPort 的一个子类，它封装了 mach port，是 macOS 中的基本通信端口，NSMachPort 类的 `@property (readonly) uint32_t machPort` 属性便是取得 NSMachPort 对象对应的 mach port。NSMachPort 只允许本地（在同一台机器上）通信。附带类 NSSocketPort 允许本地和远程分布式对象通信，但是对于本地情况，可能比 NSMachPort 更昂贵。要有效地使用 NSMachPort，你应该熟悉 mach ports、port 访问权限和 mach messages。

&emsp;NSMachPort 的工作方式其实是将 NSMachPort 的对象添加到一个线程所对应的 RunLoop 中，并给 NSMachPort 对象设置相应的代理。在其他线程中调用该 NSMachPort 对象发消息时会在 NSMachPort 所关联的线程中执行相关的代理方法。示例代码如下：

```c++
#import "ViewController.h"
#import "MyWorkClass.h"

@interface ViewController () <NSPortDelegate>

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    // 1. 在主线程创建一个 port 对象，然后直接把此 port 对象传到子线程去，子线程通过此 port 对象发送消息给主线程
    NSPort *myPort = [NSMachPort port];
    
    // 2. 设置 port 对象的回调代理
    myPort.delegate = self;
    
    // 3. 把 port 对象加入到主线程的 runloop 中，用来接收消息
    [[NSRunLoop currentRunLoop] addPort:myPort forMode:NSDefaultRunLoopMode];
    
    NSLog(@"🍀🍀🍀 添加到主线程 runloop 中的 port 对象：%@", myPort);
    
    // 4. 创建一个子线程，并把传入主线程 runloop 中的 port 对象传递到子线程中去
    MyWorkClass *work = [[MyWorkClass alloc] init];
    [NSThread detachNewThreadSelector:@selector(launchThreadWithPort:) toTarget:work withObject:myPort];
}

// This is the delegate method that subclasses should send to their delegates, unless the subclass has something more specific that it wants to try to send first
- (void)handlePortMessage:(NSMessagePort *)message {
    NSLog(@"🍀🍀🍀 接到 子线程 通过 port 传递来的消息：%@ 当前在：%@", message, [NSThread currentThread]);
    
    // 1. 消息 id
    NSUInteger msgID = [[message valueForKeyPath:@"msgid"] integerValue];
    
    // 2. 当前主线程的 port
    NSPort *localPort = [message valueForKeyPath:@"localPort"];
    
    NSLog(@"🍀🍀🍀 接到 子线程 通过 port 传递来的消息，localPort: %@", localPort);
    
    // 3. 接收到消息的 port（来自其它线程）
    NSPort *remotePort = [message valueForKeyPath:@"remotePort"];
    
    NSLog(@"🍀🍀🍀 接到 子线程 通过 port 传递来的消息，remotePort: %@", remotePort);
    
    if (msgID == 100) {
        // 向子线程的 port 发送消息
//        [remotePort sendBeforeDate:[NSDate date] msgid:200 components:nil from:localPort reserved:0];
        
    } else if (msgID == 200) {
        NSLog(@"操作 2 ... \n");
        
    }
}

- (void)dealloc {
    NSLog(@"🍀🍀🍀 %@", @"ViewController");
}

@end

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface MyWorkClass : NSObject

- (void)launchThreadWithPort:(NSPort *)port;

@end

NS_ASSUME_NONNULL_END

#import "MyWorkClass.h"

@interface MyWorkClass () <NSMachPortDelegate>

@property (nonatomic, strong) NSPort *remotePort;

@property (nonatomic, strong) NSPort *myPort;

@end

@implementation MyWorkClass

- (void)launchThreadWithPort:(NSPort *)port {
    @autoreleasepool {
        // 1. 保存主线程传来的 port 对象，然后通过此 port 对象可以向主线程发送消息
        self.remotePort = port;
        
        // 2. 设置子线程的名字
        [[NSThread currentThread] setName:@"MYWORKERCLASSTHREAD"];
        
        // 3. 开启 runloop
        [[NSRunLoop currentRunLoop] run];
        
        // 4. 创建自己的 port
        self.myPort = [NSPort port];
        
        NSLog(@"🍀🍀🍀 子线程：%@ 添加到子线程 runloop 的 port 对象：%@", [NSThread currentThread], self.myPort);
        
        // 5. 给自己的 port 设置代理
        self.myPort.delegate = self;
        
        // 6. 将自己的 port 添加到当前线程的 runloop 中
        // 作用 1：防止当前线程的 runloop 退出（如果子线程的 runloop 中没有任何 timer/source/observer 则会自动退出）
        // 作用 2：可以用来接收主线程通过此 port 对象发送过来的消息
        [[NSRunLoop currentRunLoop] addPort:self.myPort forMode:NSDefaultRunLoopMode];
        
        // 7. 从子线程向主线程发送消息：
        // 首先我们使用的是主线程 runloop 中的 port 对象发送消息，并且我们把上面创建的并放入子线程 runloop 中的 port 对象传递到主线程去，
        // 那么后续主线程便可以通过此 port 对象向子线程发送消息，看到这里我们也发现了 port 对象是单向通信的
        [self sendPortMessage];
    }
}

- (void)sendPortMessage {
    NSMutableArray *array = [[NSMutableArray alloc] initWithArray:@[@"1", @"2"]];
    // 发送消息到主线程，操作 1
    [self.remotePort sendBeforeDate:[NSDate date] msgid:100 components:array from:self.myPort reserved:0];
}

- (void)handlePortMessage:(NSPortMessage *)message {
    NSLog(@"🍀🍀🍀 接收主线程发送来的消息！");
    
    // 处理主线程发送来的消息，例如停止子线程的 runloop 等操作
}

@end
```

&emsp;上面的示例代码中我们演示了 NSMachPort 的使用，NSMachPort 以面向对象的思想对 mach_port_t 进行封装，简化了 port 的使用。与直接系统调用不同，这里的用户进程是先向内核申请一个 port 的访问许可，然后利用 IPC 机制向这个 port 发送消息。虽说发送消息的操作同样是系统调用，但 Mach 内核的工作形式有些不同——handler 的工作可以交由其他进程实现。

## Mach 异常产生的流程

&emsp;在《深入解析 Mac OS X & iOS 操作系统》一书中介绍了系统对异常处理的流程，以及如下一张示意图：

![截屏2021-12-06 下午10.12.36.png](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/a8c7656632024714aa1b2ec7df67234d~tplv-k3u1fbpfcp-watermark.image?)

&emsp;以及详细的异常机制，硬件异常/软件异常等，这里就不再摘录了。

## Mach 异常类型有哪些

&emsp;我们可以在 [xnu 版本列表](https://opensource.apple.com/tarballs/xnu/) 下载最新的 XNU 内核源码，当前最新的版本是 xnu-7195.141.2。Mach 异常的类型便被定义在 xnu-7195.141.2/osfmk/mach/exception_types.h 中。

&emsp;下面我们列举 exception_types.h 中几个比较常见的 Mach 异常类型：

+ EXC_BAD_ACCESS

```c++
/*
 *    Machine-independent exception definitions.
 */

/* Could not access memory. */
/* Code contains kern_return_t describing error. */
/* Subcode contains bad memory address. */
#define EXC_BAD_ACCESS          1       
```

&emsp;通常由于访问了不该访问的内存导致。EXC_BAD_ACCESS 通常是由于访问了不该访问的内存导致，根据情况不同它会分别转换为两种类型的信号：当 mach code 等于 KERN_INVALID_ADDRESS 时转换为 SIGSEGV 其他情况则转换为 SIGBUG。

&emsp;SIGSEGV：对应的 mach code 是 KERN_INVALID_ADDRESS（无效地址访问），即通常我们说的野指针。通常是试图访问未分配给自己的内存，或试图往没有写权限的内存地址写数据。`*((int*)(0x1234)) = 122;` 可以制造一个 `EXC_BAD_ACCESS(SIGSEGV)` 异常。

&emsp;SIGBUG：非法地址，包括内存地址对齐(alignment)出错。比如访问一个四个字长的整数, 但其地址不是 4 的倍数。它与 SIGSEGV 的区别在于后者是由于对合法存储地址的非法访问触发的(如访问不属于自己存储空间或只读存储空间)。`char *s = "hello world"; *s = 'H';` 可以制造一个 EXC_BAD_ACCESS(SIGBUS) 异常。

&emsp;栈溢出会导致 SIGSEGV 信号，但是在 mach 层捕获的时候对应的 mach code 是 KERN_PROTECTION_FAILURE(地址保护错误)，因为栈溢出会访问到栈顶部的保护页。在 mach 层捕获异常转换到对应的信号需要考虑这种情况。

+ EXC_BAD_INSTRUCTION

```c++
/* Instruction failed */
/* Illegal or undefined instruction or operand */
#define EXC_BAD_INSTRUCTION     2       
```

&emsp;此类异常通常由于线程执行非法指令导致。EXC_BAD_INSTRUCTION 是指令相关的异常，通常是尝试执行非法的指令，对应的信号是 SIGILL 表示 Illeague instruction。

+ EXC_ARITHMETIC

```c++
/* Arithmetic exception */
/* Exact nature of exception is in code field */
#define EXC_ARITHMETIC          3       
```

&emsp;算术异常，除零错误会抛出此类异常。EXC_ARITHMETIC 是算术相关的异常，在发生致命的算术运算错误时发出。不仅包括浮点运算错误，还包括溢出及除数为 0 等其它所有的算术的错误。对应的信号是 SIGFPE。

+ EXC_EMULATION

```c++
/* Emulation instruction */
/* Emulation support instruction encountered */
/* Details in code and subcode fields    */
#define EXC_EMULATION           4       
```

&emsp;EXC_EMULATION 是硬件相关的异常，对应的信号是 SIGEMT，几乎碰不到这种信号。

+ EXC_SOFTWARE

```c++
/* Software generated exception */
/* Exact exception is in code field. */

/* Codes 0 - 0xFFFF reserved to hardware */
/* Codes 0x10000 - 0x1FFFF reserved for OS emulation (Unix) */

#define EXC_SOFTWARE            5       
```

&emsp;EXC_SOFTWARE 是软件相关的异常，它会分别转换为四种不同类型的信号：SIGSYS、SIGPIPE、SIGABRT 和 SIGKILL。

&emsp;SIGSYS：对应的 mach code 是 EXC_UNIX_BAD_SYSCALL，通常是非法的系统调用。

&emsp;SIGPIPE：管道破裂，这个信号通常在进程间通信产生。比如采用 FIFO(管道)通信的两个进程，读管道没打开或者意外终止就往管道写，写进程会收到 SIGPIPE 信号。此外用 Socket 通信的两个进程，写进程在写 Socket 的时候，读进程已经终止。对应的 mach code 是 EXC_UNIX_BAD_PIPE。

&emsp;SIGABRT：是调用 abort 生成的信号。通常是因为应用层发生 NSException 异常，并且没有被捕获，导致程序向自身发送了 SIGABRT 信号而崩溃。abort 函数最终会调用 `(void)pthread_kill(pthread_self(), SIGABRT);`。对应的 mach code 是 EXC_UNIX_ABORT。

&emsp;SIGKILL：用来立即结束程序的运行，该信号不能被阻塞、处理和忽略。对应的 mach code 是 EXC_SOFT_SIGNAL。

+ EXC_BREAKPOINT

```c++
/* Trace, breakpoint, etc. */
/* Details in code field. */
#define EXC_BREAKPOINT          6       
```

&emsp;EXC_BREAKPOINT 是由断点指令或其它 trap 指令产生，由 debugger 使用，对应的信号是 SIGTRAP。

## Mach 异常捕获

&emsp;上面我们看到了 Mach 使用 port 进行线程间通信，而捕获 Mach 异常也正是基于 port 的通信机制来做的，我们可以通过 Mach 提供的 API 实现注册自定义 port（thread 类型/task 类型/host 类型），替换内核接收 Mach 异常消息的 port，然后利用 mach_msg 函数接收异常消息，最后利用 mach_msg 函数将异常消息转发出去，不影响原有的流程。

&emsp;这里替换内核接收 Mach 异常消息的 port 涉及到三个重要函数，我们能分别在 host、task、thread 三者中设置 port。

+ 为 host 层一个或多个异常类型设置异常处理程序。如果没有 task 或特定于 thread 的异常处理程序，或者这些处理程序返回错误，则会为 host 上的所有 thread 调用这些处理程序：[host_set_exception_ports](https://opensource.apple.com/source/xnu/xnu-7195.141.2/osfmk/mach/host_priv.defs.auto.html)
 
+ 为指定 task 设置异常端口：[task_set_exception_ports](https://opensource.apple.com/source/xnu/xnu-7195.141.2/osfmk/man/task_set_exception_ports.html)
+ 为指定 thread 设置异常端口：[thread_set_exception_ports](https://opensource.apple.com/source/xnu/xnu-7195.141.2/osfmk/man/thread_set_exception_ports.html)

&emsp;这里还有一些注意点：`thread_set_exception_ports` 只能针对特定线程，例如我们在 `thread_set_exception_ports` 设置了主线程，那么在子线程的发生的 mach 异常通过我们设置的 port 是无法收到回调的，此时仅能收到主线程发生的 Mach 异常，而 `task_set_exception_ports` 则可以收到当前进程的所有 Mach 异常，不区分是哪个线程发生了 Mach 异常。

&emsp;下面是捕获 Mach 异常的示例代码：

```c++
#import "AppDelegate.h"

#import <pthread.h>
#import <mach/mach_init.h>
#import <mach/mach_port.h>
#import <mach/task.h>
#import <mach/message.h>
#import <mach/thread_act.h>
#import <mach/host_priv.h>

@interface AppDelegate ()

@end

@implementation AppDelegate

/// 注册捕获异常的端口
// 自定义端口号
mach_port_name_t myExceptionPort = 10086;

- (void)catchMACHExceptions {
    // 用自定义端口号初始化一个端口
    mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &myExceptionPort);
    // 向端口插入发送权限
    mach_port_insert_right(mach_task_self(), myExceptionPort, myExceptionPort, MACH_MSG_TYPE_MAKE_SEND);
    // 设置 Mach 异常的种类
    exception_mask_t excMask = EXC_MASK_BAD_ACCESS | EXC_MASK_BAD_INSTRUCTION | EXC_MASK_ARITHMETIC | EXC_MASK_SOFTWARE;
    
    // 设置内核接收 Mach 异常消息的 thread Port
    thread_set_exception_ports(mach_thread_self(), excMask, myExceptionPort, EXCEPTION_DEFAULT, MACHINE_THREAD_STATE);
//    task_set_exception_ports(mach_task_self(), excMask, myExceptionPort, EXCEPTION_DEFAULT, MACHINE_THREAD_STATE);
//    host_set_exception_ports(<#host_priv_t host_priv#>, <#exception_mask_t exception_mask#>, <#mach_port_t new_port#>, <#exception_behavior_t behavior#>, <#thread_state_flavor_t new_flavor#>)
    
    // 新建一个线程处理异常消息
    pthread_t thread;
    pthread_create(&thread, NULL, exc_handler, NULL);
}

/// 接收异常消息
static void *exc_handler(void *ignored) {
    // 结果
    mach_msg_return_t rc;
    // 内核将发送给我们的异常消息的格式，参考 ux_handler() [bsd / uxkern / ux_exception.c] 中对异常消息的定义
    typedef struct {
        mach_msg_header_t Head;
        // start of the kernel processed data
        mach_msg_body_t msgh_body;
        mach_msg_port_descriptor_t thread;
        mach_msg_port_descriptor_t task;
        // end of the kernel processed data
        NDR_record_t NDR;
        exception_type_t exception;
        mach_msg_type_number_t codeCnt;
        integer_t code[2];
        int flavor;
        mach_msg_type_number_t old_stateCnt;
        natural_t old_state[144];
    } exc_msg_t;
    
    // 消息处理循环，这里的死循环不会有问题，因为 exc_handler 函数运行在一个独立的子线程中，而且 mach_msg 函数也是会阻塞的。
    for (;;) {
        exc_msg_t exc;
        
        // 这里会阻塞，直到接收到 exception message，或者线程被中断
        rc = mach_msg(&exc.Head, MACH_RCV_MSG | MACH_RCV_LARGE, 0, sizeof(exc_msg_t), myExceptionPort, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
        if (rc != MACH_MSG_SUCCESS) {
            //
            break;
        };
        
        // 打印异常消息
        NSLog(@"🍀🍀🍀 CatchMACHExceptions %d. Exception : %d Flavor: %d. Code %d/%d. State count is %d", exc.Head.msgh_id, exc.exception, exc.flavor, exc.code[0], exc.code[1], exc.old_stateCnt);
        
        // 定义转发出去的消息类型
        struct rep_msg {
            mach_msg_header_t Head;
            NDR_record_t NDR;
            kern_return_t RetCode;
        } rep_msg;
        rep_msg.Head = exc.Head;
        rep_msg.NDR = exc.NDR;
        rep_msg.RetCode = KERN_FAILURE;
        kern_return_t result;
        if (rc == MACH_MSG_SUCCESS) {
            // 将异常消息再转发出去
            result = mach_msg(&rep_msg.Head, MACH_SEND_MSG, sizeof(rep_msg), 0, MACH_PORT_NULL, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
        }
    }
    
    return NULL;
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    
    // 自定义捕获 Mach 异常
    [self catchMACHExceptions];
    
    // ARC 下会发生 EXC_BAD_ACCESS 异常，这里要注意一下，只有我们关闭 xcode 的 Debug executable 选项才能收到 exc_handler 回调
    __unsafe_unretained NSObject *objc = [[NSObject alloc] init];
    NSLog(@"✳️✳️✳️ objc: %@", objc);
    
    return YES;
}

@end
```

&emsp;运行代码，我们能看到控制台有如下类似的打印：

```c++
🍀🍀🍀 CatchMACHExceptions 2401. Exception : 1 Flavor: 0. Code 13/0. State count is 8
```

&emsp;这里还发现在 m1 mac 下需要关闭下面的 Debug executable 现象后才能捕获到 Mach 异常，在 intel mac 下开启与关闭 Debug executable 选项都能捕获到 Mach 异常。

![截屏2021-12-07 下午9.41.34.png](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/b0545d10a7c94cc3b0b43394e3a527ad~tplv-k3u1fbpfcp-watermark.image?)

## Unix signals

&emsp;上面的示例代码中，我们使用第一条 `mach_msg` 捕获到了 Mach 异常，然后我们第二条 `mach_msg` 继续将 Mach 异常消息转发出去，那么转发出去的 Mach 异常消息会去哪里呢？它会被转换为对应的 UNIX 信号（在上面的 Mach 异常类型中我们已经介绍了 Mach 异常类型与 Unix 信号的对应关系，下面我们会更具体的看一下）。通过上面的 Mach 异常流程图，Mach 异常在 Mach 层被捕获并抛出后，会在 BSD 层被 `catch_mach_exception_raise` 处理，并通过 `ux_exception` 将异常转换为对应的 UNIX 信号，并通过 `threadsignal` 将信号投递到出错线程，iOS 中的 POSIX API 就是通过 Mach 之上的 BSD 层实现的。我们可以看下 `ux_exception` 函数实现，其中有 Mach 异常类型和 UNIX signals 的对应关系。

### Mach 异常转换为 Unix signal

&emsp;ux_exception 函数中明确的对应关系：

```c++
/*
 * Translate Mach exceptions to UNIX signals.
 *
 * ux_exception translates a mach exception, code and subcode to a signal.  Calls machine_exception (machine dependent) to attempt translation first.
 */
static int
ux_exception(int                        exception,
    mach_exception_code_t      code,
    mach_exception_subcode_t   subcode)
{
    int machine_signal = 0;

    /* Try machine-dependent translation first. */
    if ((machine_signal = machine_exception(exception, code, subcode)) != 0) {
        return machine_signal;
    }

    switch (exception) {
    case EXC_BAD_ACCESS:
        if (code == KERN_INVALID_ADDRESS) {
            return SIGSEGV;
        } else {
            return SIGBUS;
        }

    case EXC_BAD_INSTRUCTION:
        return SIGILL;

    case EXC_ARITHMETIC:
        return SIGFPE;

    case EXC_EMULATION:
        return SIGEMT;

    case EXC_SOFTWARE:
        switch (code) {
        case EXC_UNIX_BAD_SYSCALL:
            return SIGSYS;
        case EXC_UNIX_BAD_PIPE:
            return SIGPIPE;
        case EXC_UNIX_ABORT:
            return SIGABRT;
        case EXC_SOFT_SIGNAL:
            return SIGKILL;
        }
        break;

    case EXC_BREAKPOINT:
        return SIGTRAP;
    }

    return 0;
}
```

&emsp;以表格的形式展示，大家更清晰一些：

<table>
    <tr>
        <td>Mach Exception Type</td>
        <td>Unix Signal</td>
    </tr>
    <tr>
        <td rowspan="2">EXC_BAD_ACCESS</td>
        <td>SIGSEGV</td>
    </tr>
    <tr>
        <td>SIGBUS</td>
    </tr>
    <tr>
        <td>EXC_BAD_INSTRUCTION</td>
        <td>SIGILL</td>
    </tr>
    <tr>
        <td>EXC_ARITHMETIC</td>
        <td>SIGFPE</td>
    </tr>
    <tr>
        <td>EXC_EMULATION</td>
        <td>SIGEMT</td>
    </tr>
    <tr>
        <td rowspan="4">EXC_SOFTWARE</td>
        <td>SIGSYS(EXC_UNIX_BAD_SYSCALL)</td>
    </tr>
    <tr>
        <td>SIGPIPE(EXC_UNIX_BAD_PIPE)</td>
    </tr>
    <tr>
        <td>SIGABRT(EXC_UNIX_ABORT)</td>
    </tr>
    <tr>
        <td>SIGKILL(EXC_SOFT_SIGNAL)</td>
    </tr>
    <tr>
        <td>EXC_BREAKPOINT</td>
        <td>SIGTRAP</td>
    </tr>
</table>

### Unix signals 有哪些

&emsp;所有的 Unix signals 值被定义在 xnu-7195.141.2/bsd/machine/signal.h 中。

```c++
#define SIGHUP  1       /* hangup */
#define SIGINT  2       /* interrupt */
#define SIGQUIT 3       /* quit */
#define SIGILL  4       /* illegal instruction (not reset when caught) */
#define SIGTRAP 5       /* trace trap (not reset when caught) */
#define SIGABRT 6       /* abort() */
#if  (defined(_POSIX_C_SOURCE) && !defined(_DARWIN_C_SOURCE))
#define SIGPOLL 7       /* pollable event ([XSR] generated, not supported) */
#else   /* (!_POSIX_C_SOURCE || _DARWIN_C_SOURCE) */
#define SIGIOT  SIGABRT /* compatibility */
#define SIGEMT  7       /* EMT instruction */
#endif  /* (!_POSIX_C_SOURCE || _DARWIN_C_SOURCE) */
#define SIGFPE  8       /* floating point exception */
#define SIGKILL 9       /* kill (cannot be caught or ignored) */
#define SIGBUS  10      /* bus error */
#define SIGSEGV 11      /* segmentation violation */
#define SIGSYS  12      /* bad argument to system call */
#define SIGPIPE 13      /* write on a pipe with no one to read it */
#define SIGALRM 14      /* alarm clock */
#define SIGTERM 15      /* software termination signal from kill */
#define SIGURG  16      /* urgent condition on IO channel */
#define SIGSTOP 17      /* sendable stop signal not from tty */
#define SIGTSTP 18      /* stop signal from tty */
#define SIGCONT 19      /* continue a stopped process */
#define SIGCHLD 20      /* to parent on child stop or exit */
#define SIGTTIN 21      /* to readers pgrp upon background tty read */
#define SIGTTOU 22      /* like TTIN for output if (tp->t_local&LTOSTOP) */
#if  (!defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE))
#define SIGIO   23      /* input/output possible signal */
#endif
#define SIGXCPU 24      /* exceeded CPU time limit */
#define SIGXFSZ 25      /* exceeded file size limit */
#define SIGVTALRM 26    /* virtual time alarm */
#define SIGPROF 27      /* profiling time alarm */
#if  (!defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE))
#define SIGWINCH 28     /* window size changes */
#define SIGINFO 29      /* information request */
#endif
#define SIGUSR1 30      /* user defined signal 1 */
#define SIGUSR2 31      /* user defined signal 2 */
```

&emsp;

### Unix Signal 捕获

&emsp;这里摘录 [Handling unhandled exceptions and signals](https://www.cocoawithlove.com/2010/05/handling-unhandled-exceptions-and.html) 中的示例代码，做了一些微小的修改：

```c++
#import "UncaughtExceptionHandler.h"

#import <UIKit/UIDevice.h>
#import <libkern/OSAtomic.h>
#import <execinfo.h>
#import <stdatomic.h>

NSString * const UncaughtExceptionHandlerSignalExceptionName = @"UncaughtExceptionHandlerSignalExceptionName";
NSString * const UncaughtExceptionHandlerSignalKey = @"UncaughtExceptionHandlerSignalKey";
NSString * const UncaughtExceptionHandlerAddressesKey = @"UncaughtExceptionHandlerAddressesKey";
NSString * const UncaughtExceptionHandlerFileKey = @"UncaughtExceptionHandlerFileKey";

atomic_int UncaughtExceptionCount = 0;
const int32_t UncaughtExceptionMaximum = 10;

// 这里异常发生时跳过函数调用堆栈中的 4 个 frame，如下 4 个：
/*
 "0   dSYMDemo                            0x00000001042541eb +[UncaughtExceptionHandler backtrace] + 59",
 "1   dSYMDemo                            0x0000000104253edc mySignalHandler + 76",
 "2   libsystem_platform.dylib            0x000000010e774e2d _sigtramp + 29",
 "3   ???                                 0x0000600002932720 0x0 + 105553159464736",
*/
const NSInteger UncaughtExceptionHandlerSkipAddressCount = 4;
//const NSInteger UncaughtExceptionHandlerReportAddressCount = 5;

void mySignalHandler(int signal);

@implementation UncaughtExceptionHandler

+ (void)installUncaughtExceptionHandler {
    // 将之前注册的 未捕获异常处理函数 取出并备份，防止覆盖
    previousUncaughtExceptionHandler = NSGetUncaughtExceptionHandler();
    // Objective-C 异常捕获（越界、参数无效等）
    NSSetUncaughtExceptionHandler(&UncaughtExceptionHandlers);
    
    // 信号量截断，当抛出信号时会回调 MySignalHandler 函数
    signal(SIGABRT, mySignalHandler);
    signal(SIGILL, mySignalHandler);
    signal(SIGSEGV, mySignalHandler);
    signal(SIGFPE, mySignalHandler);
    signal(SIGBUS, mySignalHandler);
    signal(SIGPIPE, mySignalHandler);
}

+ (void)setSignalHandlerInAdvance {
    struct sigaction act;
    // 当 sa_flags 设为 SA_SIGINFO 时，设定 sa_sigaction 来指定信号处理函数
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = test_signal_action_handler;
    sigaction(SIGABRT, &act, NULL);
}

static void test_signal_action_handler(int signo, siginfo_t *si, void *ucontext) {
    NSLog(@"🏵🏵🏵 [sigaction handler] - handle signal: %d", signo);
    
    // handle siginfo_t
    NSLog(@"🏵🏵🏵 siginfo: {\n si_signo: %d,\n si_errno: %d,\n si_code: %d,\n si_pid: %d,\n si_uid: %d,\n si_status: %d,\n si_value: %d\n }",
          si->si_signo,
          si->si_errno,
          si->si_code,
          si->si_pid,
          si->si_uid,
          si->si_status,
          si->si_value.sival_int);
}

// 获取函数堆栈信息
+ (NSArray *)backtrace {
    void* callstack[128];
    
    // 用于获取当前线程的函数调用堆栈，返回实际获取的指针个数
    int frames = backtrace(callstack, 128);
    // 从 backtrace 函数获取的信息转化为一个字符串数组
    char **strs = backtrace_symbols(callstack, frames);
    
    NSMutableArray *backtrace = [NSMutableArray arrayWithCapacity:frames];
    
    // 越过最前面的 4 个 frame
    if (frames > UncaughtExceptionHandlerSkipAddressCount) {
        for (int i = UncaughtExceptionHandlerSkipAddressCount; i < frames; ++i) {
            [backtrace addObject:[NSString stringWithUTF8String:strs[i]]];
        }
    }
    
    NSLog(@"🏵🏵🏵 异常发生时的堆栈：%@", backtrace);
    
    free(strs);
    
    return backtrace;
}

- (void)saveCreash:(NSException *)exception file:(NSString *)file {
    // 异常发生时的堆栈信息
    NSArray *stackArray = [exception callStackSymbols];
    if (!stackArray || stackArray.count <= 0) {
        stackArray = [exception.userInfo objectForKey:UncaughtExceptionHandlerAddressesKey];
    }
    
    // 出现异常的原因
    NSString *reason = [exception reason];
    // 异常名称
    NSString *name = [exception name];
    
    NSString * _libPath  = [[NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES) objectAtIndex:0] stringByAppendingPathComponent:file];
    
    if (![[NSFileManager defaultManager] fileExistsAtPath:_libPath]){
        [[NSFileManager defaultManager] createDirectoryAtPath:_libPath withIntermediateDirectories:YES attributes:nil error:nil];
    }
    
    NSDate* date = [NSDate dateWithTimeIntervalSinceNow:0];
    NSTimeInterval a = [date timeIntervalSince1970];
    NSString *timeString = [NSString stringWithFormat:@"%f", a];
    
    NSString * savePath = [_libPath stringByAppendingFormat:@"/error%@.log", timeString];
    NSString *exceptionInfo = [NSString stringWithFormat:@"Exception reason：%@\nException name：%@\nException stack：%@", name, reason, stackArray];
    BOOL sucess = [exceptionInfo writeToFile:savePath atomically:YES encoding:NSUTF8StringEncoding error:nil];
    
    NSLog(@"🏵🏵🏵 保存崩溃日志 sucess:%d, %@", sucess, savePath);
}

// 异常处理方法
- (void)handleException:(NSException *)exception {
    NSDictionary *userInfo = [exception userInfo];
    [self saveCreash:exception file:[userInfo objectForKey:UncaughtExceptionHandlerFileKey]];
    
    // 这里也可以强行再次运行 runloop 防止程序中止，但是完全没有必要，因为当前程序已经处于完全不可用状态
//    CFRunLoopRef runLoop = CFRunLoopGetCurrent();
//    CFArrayRef allModes = CFRunLoopCopyAllModes(runLoop);
//
//    while (!dismissed) {
//        for (NSString *mode in (__bridge NSArray *)allModes) {
//            CFRunLoopRunInMode((CFStringRef)mode, 0.001, false);
//        }
//    }
    
    r0 ~ r30 共 31 个寄存器，每个寄存器是 8 个字节 64 位
    r31 第 32 个寄存器是 zero register 
    r29 fp frame pointer
    r30 lr link register 
    
    x31 zero register zr XZR/WZR 64/32 位
    sp 就是 x31 SP/WSP
    pc 当前执行的指令的地址
    cpsr spsrs fpsr fpcr 
     
    NSSetUncaughtExceptionHandler(NULL);
    signal(SIGABRT, SIG_DFL);
    signal(SIGILL, SIG_DFL);
    signal(SIGSEGV, SIG_DFL);
    signal(SIGFPE, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
    signal(SIGPIPE, SIG_DFL);
    
    if ([[exception name] isEqual:UncaughtExceptionHandlerSignalExceptionName]) {
        int signalNumber = [[[exception userInfo] objectForKey:UncaughtExceptionHandlerSignalKey] intValue];
        
        NSLog(@"🏵🏵🏵 抓到 signal 异常：%d", signalNumber);
        
        // 如果是 signal 异常
        kill(getpid(), [[[exception userInfo] objectForKey:UncaughtExceptionHandlerSignalKey] intValue]);
    } else {
        NSLog(@"🏵🏵🏵 抓到 Objective-C 异常：%@", exception);
        
        // 如果是 Objective-C 异常
        [exception raise];
        
        // 在自己的异常处理操作完毕后，调用先前别人注册的未捕获异常处理函数，并把原始的 exception 进行传递
        if (previousUncaughtExceptionHandler) {
            previousUncaughtExceptionHandler(exception);
        } else {
            // 如果是 Objective-C 异常
            kill(getpid(), SIGKILL);
        }
    }
}

// 获取应用信息
NSString* getAppInfo(void) {
    NSString *appInfo = [NSString stringWithFormat:@"App : %@ %@(%@) Device : %@ OS Version : %@ %@",
                         [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleDisplayName"],
                         [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleShortVersionString"],
                         [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleVersion"],
                         [UIDevice currentDevice].model,
                         [UIDevice currentDevice].systemName,
                         [UIDevice currentDevice].systemVersion];
    return appInfo;
}

static NSUncaughtExceptionHandler *previousUncaughtExceptionHandler = NULL;

// NSSetUncaughtExceptionHandler 捕获异常的调用方法，利用 NSSetUncaughtExceptionHandler，当程序异常退出的时候，可以先进行处理，然后做一些自定义的动作
void UncaughtExceptionHandlers (NSException *exception) {
    // 原子自增 1
    int32_t exceptionCount = atomic_fetch_add(&UncaughtExceptionCount, 1);
    if (exceptionCount > UncaughtExceptionMaximum) { return; }
    
    // 异常发生时的函数堆栈
    NSArray *callStack = [UncaughtExceptionHandler backtrace];
    
    // 组装 userInfo 数据
    NSMutableDictionary *userInfo = [NSMutableDictionary dictionaryWithDictionary:[exception userInfo]];
    [userInfo setObject:callStack forKey:UncaughtExceptionHandlerAddressesKey];
    [userInfo setObject:@"Objective-C Crash" forKey:UncaughtExceptionHandlerFileKey];
    
    NSException *medianException = [NSException exceptionWithName:[exception name]
                                                           reason:[exception reason]
                                                         userInfo:userInfo];
    
    // Objective-C 异常和 signal 都放在 handleException: 函数中进行处理
    [[[UncaughtExceptionHandler alloc] init] performSelectorOnMainThread:@selector(handleException:) withObject:medianException waitUntilDone:YES];
}

// Signal 处理方法
void mySignalHandler(int signal) {
    // 原子自增 1
    int32_t exceptionCount = atomic_fetch_add(&UncaughtExceptionCount, 1);
    if (exceptionCount > UncaughtExceptionMaximum) { return; }
    
    // 异常发生时的函数堆栈
    NSArray *callStack = [UncaughtExceptionHandler backtrace];
    
    // 组装 userInfo 数据
    NSMutableDictionary *userInfo = [NSMutableDictionary dictionaryWithObject:[NSNumber numberWithInt:signal] forKey:UncaughtExceptionHandlerSignalKey];
    [userInfo setObject:callStack forKey:UncaughtExceptionHandlerAddressesKey];
    [userInfo setObject:@"Signal Crash" forKey:UncaughtExceptionHandlerFileKey];
    
    // 构建一个 NSException 对象
    NSException *medianException = [NSException exceptionWithName:UncaughtExceptionHandlerSignalExceptionName
                                                     reason:[NSString stringWithFormat:NSLocalizedString(@"Signal %d was raised.\n" @"%@", nil), signal, getAppInfo()]
                                                   userInfo:userInfo];
    
    // Objective-C 异常和 signal 都放在 handleException: 函数中进行处理
    [[[UncaughtExceptionHandler alloc] init] performSelectorOnMainThread:@selector(handleException:) withObject:medianException  waitUntilDone:YES];
}

@end
```

&emsp;backtrace & backtrace_symbols 函数：

&emsp;这里我们做一个延展：示例代码中 `+ (NSArray *)backtrace {...}` 函数用来获取当前函数的回溯信息，即异常发生时的函数调用堆栈。其中用到了 `backtrace` 和 `backtrace_symbols` 函数：

```c++
int backtrace(void**,int) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);

char** backtrace_symbols(void* const*,int) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
void backtrace_symbols_fd(void* const*,int,int) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
```

&emsp;backtrace 函数用来获取程序中当前函数的回溯信息，即一系列的函数调用关系，获取到的信息被放在参数 `void**` 中。`void**` 是一个数组指针，数组的每个元素保存着每一级被调用函数的返回地址。参数 `int` 指定了 `void**` 中可存放的返回地址的数量。如果函数实际的回溯层级数大于 `int`，则 `void**` 中只能存放最近的函数调用关系，所以，想要得到完整的回溯信息，就要确保 `void**` 参数足够大。

&emsp;backtrace 函数的返回值为 `void**` 中的条目数量，这个值不一定等于 `int`，因为如果为得到完整回溯信息而将 `int` 设置的足够大，则该函数的返回值为 `void**` 中实际得到的返回地址数量。
 
&emsp;通过 backtrace 函数得到 `void**` 之后，backtrace_symbols 可以将其中的返回地址都对应到具体的函数名，参数 `int` 为 `void**` 中的条目数。backtrace_symbols 函数可以将每一个返回值都翻译成 "函数名 + 函数内偏移量 + 函数返回值"，这样就可以更直观的获得函数的调用关系。经过翻译后的函数回溯信息放到 backtrace_symbols 的返回值中，如果失败则返回 NULL。需要注意，返回值本身是在 backtrace_symbols 函数内部进行 malloc 的，所以必须在后续显式地 free 掉。
 
&emsp;backtrace_symbols_fd 的 `void* const*` 和 `int` 参数和 backtrace_symbols 函数相同，只是它翻译后的函数回溯信息不是放到返回值中，而是一行一行的放到文件描述符 fd 对应的文件中。

&emsp;然后再加上我们上面的 Mach 异常的捕获代码：

```c++
- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    
    // Objective-C 异常处理/signal 信号处理
    [UncaughtExceptionHandler installUncaughtExceptionHandler];
    // 自定义捕获 Mach 异常
    [self catchMACHExceptions];
}
```

&emsp;我们便可以捕获三种情况下的异常。

&emsp;除了上面的关闭 Debug executable 选项，我们还可以通过在 LLDB 中关闭拦截，而收到 Signal 的回调。Xcode Debug 模式运行 App 时，App 进程 signal 被 LLDB Debugger 调试器捕获，我们可以使用 LLDB 调试命令，将指定 signal 处理抛到用户层处理，方便调试。 

```c++
(lldb) process handle --notify true
Do you really want to update all the signals?: [y/N] y
...
(lldb) process handle --stop false
Do you really want to update all the signals?: [y/N] y
...
(lldb) process handle --pass true
Do you really want to update all the signals?: [y/N] y
NAME         PASS   STOP   NOTIFY
===========  =====  =====  ======
SIGHUP       true   false  true 
SIGINT       true   false  true 
SIGQUIT      true   false  true 
SIGILL       true   false  true 
SIGTRAP      true   false  true 
SIGABRT      true   false  true 
SIGEMT       true   false  true 
SIGFPE       true   false  true 
SIGKILL      true   false  true 
SIGBUS       true   false  true 
SIGSEGV      true   false  true 
SIGSYS       true   false  true 
SIGPIPE      true   false  true 
SIGALRM      true   false  true 
SIGTERM      true   false  true 
SIGURG       true   false  true 
SIGSTOP      true   false  true 
SIGTSTP      true   false  true 
SIGCONT      true   false  true 
SIGCHLD      true   false  true 
SIGTTIN      true   false  true 
SIGTTOU      true   false  true 
SIGIO        true   false  true 
SIGXCPU      true   false  true 
SIGXFSZ      true   false  true 
SIGVTALRM    true   false  true 
SIGPROF      true   false  true 
SIGWINCH     true   false  true 
SIGINFO      true   false  true 
SIGUSR1      true   false  true 
SIGUSR2      true   false  true 
```

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

## 总结 Objective-C 异常、Mach 异常、Unix Signals

&emsp;未捕获的 Objective-C 异常最终会导致程序向自身发送了 SIGABRT 信号而中止，此时我们并不能捕获到 SIGABRT 信号，如果我们手动调用 `(void)pthread_kill(pthread_self(), SIGABRT)/kill(getpid(), SIGABRT)` 则可以收到 SIGABRT 信号，且它们都是应用级异常，所以 Mach 异常的流程是不会走的。

&emsp;一般情况下 Mach 异常和 Objective-C 异常最终都会转换为 Unix signals，但是还有一些特殊情况，例如 EXC_GUARD 异常，还有发生栈溢出（Stackoverflow）时，Unix signals 在崩溃线程回调，但是已经没有条件(栈空间)再执行回调代码了。

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
+ [Apple 源码文件下载列表](https://opensource.apple.com/tarballs/)
+ [iOS @try @catch异常机制](https://www.jianshu.com/p/f28b9b3f8e44)
+ [一文读懂崩溃原理](https://juejin.cn/post/6873868181635760142)
+ [软件测试之SDK开发(ios)——Mach捕获](https://blog.csdn.net/lfdanding/article/details/100024022)
+ [[史上最全] iOS Crash/崩溃/异常 捕获](https://www.jianshu.com/p/3f6775c02257)
+ [iOS Crash/崩溃/异常 堆栈获取](https://www.jianshu.com/p/8ece78d71b3d)
+ [KSCrash源码分析](https://cloud.tencent.com/developer/article/1370201)
+ [iOS线程通信和进程通信的例子（NSMachPort和NSTask，NSPipe）](https://blog.csdn.net/yxh265/article/details/51483822)
+ [iOS开发·RunLoop源码与用法完全解析(输入源，定时源，观察者，线程间通信，端口间通信，NSPort，NSMessagePort，NSMachPort，NSPortMessage)](https://sg.jianshu.io/p/07313bc6fd24)
+ [Delivering Notifications To Particular Threads](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/Notifications/Articles/Threading.html#//apple_ref/doc/uid/20001289-CEGJFDFG)
+ [iOS开发之线程间的MachPort通信与子线程中的Notification转发](https://cloud.tencent.com/developer/article/1018076)
+ [移动端监控体系之技术原理剖析](https://www.jianshu.com/p/8123fc17fe0e)
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



