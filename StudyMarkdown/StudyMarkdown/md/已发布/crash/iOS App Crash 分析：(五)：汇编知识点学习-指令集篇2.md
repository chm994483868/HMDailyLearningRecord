# iOS App Crash 分析：(五)：汇编知识点学习-指令集篇

&emsp;汇编语言作为机器语言的助记语言应运而生，当你深入的应用它时就会发现汇编语言其实并没有那么的复杂。在一个程序所使用的机器指令中，大部分的指令所做的事情除了计算外就是将数据在寄存器与寄存器之间以及寄存器与内存之间进行移动。在高级语言中我们可以定义名字非常个性化的变量以及无限制数量的变量，而在低级语言中我们则只能使用那几个有限的寄存器来作为临时变量，以及像访问数组那样根据绝对地址/相对地址去访问内存中指定地址保存的数据。

## 指令集概述

&emsp;计算机指令就是指挥机器工作的指示和命令，程序就是一系列按一定顺序排列的指令，执行程序的过程就是计算机的工作过程。指令集是 CPU 中用来计算和控制计算机系统的一套指令的集合，而每一种新型的 CPU 在设计时就规定了一系列与其他硬件电路相配合的指令系统。而指令集的先进与否，也关系到 CPU 的性能发挥，它也是 CPU 性能体现的一个重要标志。指令的强弱也是 CPU 的重要指标，指令集是提高微处理器效率的最有效的工具之一。从现阶段的主流体系结构讲，指令集可分为复杂指令集和精简指令集两部分.

&emsp;[X86架构](https://baike.baidu.com/item/X86架构/7470217)
&emsp;[指令集](https://baike.baidu.com/item/指令集/238130?fr=aladdin)


&emsp;理解指令集和 CPU 架构的关系：

&emsp;某种 CPU 架构类型会提供自己的一套指令集系统，然后提供了某一套规定指令集的 CPU 会被称为一个 CPU 架构，或者某个厂商遵守现有的某个 CPU 架构的指令集而生产了一种 CPU，那么就是这种 CPU 也是某种 CPU 架构的 CPU，比如 AMD 公司采用英特尔公司设计的 x86 架构生产的 CPU。







&emsp;x86 架构（The X86 architecture）是微处理器执行的计算机语言指令集，指一个 intel 通用计算机系列的标准编号缩写，也标识一套通用的计算机指令集合。

&emsp;**我们称一个 CPU（或者 CPU 架构）所提供的所有可用的指令的集合为指令集（针对此特定 CPU 架构的指令集，不同 CPU 架构使用不同的指令集）。**

## CPU 架构历史概述

&emsp;

&emsp;不同的厂家以及不同的技术工艺和技术水平以及具体的设备上所实现的 CPU 的体系架构以及提供的功能也是有差异的。比如 ARM 指令架构体系的 CPU、x86 指令架构体系的 CPU、POWER-PC 指令架构体系的 CPU。这些不同架构体系的 CPU 因为架构完全不同导致所提供的指令和存储单元也完全不同。我们不可能让 ARM 指令直接在 x86 的 CPU 上执行。相同体系架构下的 CPU 指令则在一定程度上是可以相互兼容的，因为相同架构体系下的 CPU 的指令集是一致的(类比为接口一致，但是内部实现则不相同)，比如说 Intel 公司所生产的 x86 系列的 CPU 和 AMD 公司所生产的 x86 系列 CPU 所提供的指令集是相似和兼容的，他们之间的差别则只是在于内部的实现不同。

&emsp;CPU 指令集定义的是一个中央处理器所应该提供的基础功能的集合，它是 **一个标准**、是 **一个接口**、也是 **一个协议**。在软件开发中具有 **协议** 和 **接口** 定义的概念，无论是消费者还是提供者都需要遵循这个标准来进行编程和交互。提供者要实现接口所具有的功能，至于如何实现则是内部的事情，不对外暴露，消费者也不需要知道具体的实现细节，消费者则总是要按接口提供的功能方法并组合使用来完成某种功能。这种设计的思维对于硬件系统也是一样适用的。

> &emsp;x86 一般指 Intel x86，它是英特尔公司于 1978 年推出的 16 位微处理器。x86 泛指一系列基于 Intel 8086 且向后兼容的中央处理器指令集架构。
> &emsp;Intel 在早期以 80x86 这样的数字格式来命名处理器，包括 Intel 8086、80186、80286、80386 以及 80486，由于以 “86” 作为结尾，因此其架构被称为 "x86"。由于数字并不能作为注册商标，因此 Intel 及其竞争者（AMD）均在新一代处理器使用可注册的名称，如奔腾（Pentium）、酷睿（Core）、锐龙（Ryzen，由 AMD 推出）。所以看到这里 x86 中的字母 x，它的读音似乎应该是 "叉"，而不是英文字母 x/eks/。
> &emsp;x86 的 32位架构一般又被称作 IA-32，全名为 "Intel Architecture, 32-bit"。其 64 位架构由 AMD 率先推出，并被称为 "AMD64"。之后也被 Intel 采用，被其称为 "Intel 64"。一般也被称作 "x86-64"、"x64"。
> &emsp;**64 位架构：**
> &emsp;到 2002 年，由于 32 位特性的长度，x86 的架构开始到达某些设计的极限。导致要处理大量的信息储存大于 4GB 会有困难，像是在数据库或是影片编辑上可以发现。
> &emsp;Intel 原本已经决定在 64 位的时代完全地舍弃 x86 兼容性，推出新的架构称为 IA-64 技术作为他的 Itanium 处理器产品线的基础。IA-64 与 x86 的软件天生不兼容，它使用各种模拟形式来运行 x86 的软件，不过，以模拟方式来运行的效率十分低下，并且会影响其他程序的运行。
> &emsp;AMD 主动把 32 位 x86（或称为 IA-32）扩充为 64 位。它以一个称为 AMD64 的架构出现（在重命名前也称为 x86-64），且以这个技术为基础的第一个产品是单内核的 Opteron 和 Athlon 64 处理器家族。由于 AMD 的 64 位处理器产品线首先进入市场，且微软也不愿意为 Intel 和 AMD 开发两套不同的 64 位操作系统，Intel 也被迫采纳 AMD64 指令集且增加某些新的扩充到他们自己的产品，命名为 EM64T 架构（显然他们不想承认这些指令集是来自它的主要对手），EM64T 后来被 Intel 正式更名为 Intel 64。
> &emsp;这是由非 Intel 的制造商所发起和设计的第一次重大的 x86 架构升级。也许更重要的，它也是第一次 Intel 实际上从外部来源接受这项本质的技术。[Intel x86](https://baike.baidu.com/item/Intel%20x86/1012845?fromtitle=x86&fromid=6150538)
 
 
 
 
 
 
 
 
 
 
 

+ [Intel x86](https://baike.baidu.com/item/Intel%20x86/1012845?fromtitle=x86&fromid=6150538)
+ [什么是i386,x86和x64, 一文了解处理器架构](https://baike.baidu.com/tashuo/browse/content?id=5d5bb1f8a73bd4495d7b21a5&lemmaId=433177&fromLemmaModule=pcBottom&lemmaTitle=Intel%2080386)















&emsp;一般情况下某种 CPU 指令集通常都是由某些设计或者生产 CPU 的公司（Intel、ARM：Acorn RISC Machine）或者某些标准组织共同定义而形成。

&emsp;那么目前市面上有哪些主流的 CPU 指令集或 CPU 架构体系呢？

+ x86/x64（x86 架构体系在 32 位时代叫做  x86-64、x64、i386 ，毕竟当前 64 位已经成为主流）

> Intel x86 是英特尔公司于 1978 年推出的 16 位微处理器。
> x86 泛指一系列基于 Intel 8086 且向后兼容的中央处理器指令集架构。


&emsp;所谓指令的复杂度就是指 CPU 指令集中所提供的指令的数量、指令寻址模式、指令参数、以及 CPU 内部的架构设计的复杂度、以及指令本身所占据的字节数等来进行划分的一种方式，一般有两种类型的分类：（指令本身所占据的字节数是个重点，x86-64 架构体系指令长度是变长的从 1 个到 15 个字节不等，而 ARM64 架构体系指令的长度是固定的 4 个字节，这里为后续以不同的方式回溯函数调用堆栈埋下伏笔。）

1. CISC 指令集。CISC 的英文全称为 "Complex Instruction Set Computer"，即 “复杂指令系统计算机”，从计算机诞生以来，人们一直沿用CISC指令集方式。早期的桌面软件是按CISC设计的，并一直沿续到现在。目前，桌面计算机流行的x86体系结构即使用CISC。在CISC微处理器中，程序的各条指令是按顺序串行执行的，每条指令中的各个操作也是按顺序串行执行的。顺序执行的优点是控制简单，但计算机各部分的利用率不高，执行速度慢。CISC架构的服务器主要以x86/x64架构(Intel Architecture)为主，而且多数为中低档服务器所采用。

2. RISC指令集。RISC的英文全称为“Reduced Instruction Set Computer”，即“精简指令集计算机”，是一种执行较少类型计算机指令的微处理器，起源于80年代的MIPS主机(即RISC机)，RISC机中采用的微处理器统称RISC处理器。这样一来，它能够以更快的速度执行操作(每秒执行更多百万条指令，即MIPS)。目前的智能移动设备中的CPU几乎都采用RISC指令集，比较有代表的就是ARM指令集和POWER-PC指令集。

&emsp;objc-msg-x86_64.s 和 objc-msg-arm64.s 文件学习。



## 参考链接
**参考链接:🔗**
+ [Intel x86](https://baike.baidu.com/item/Intel%20x86/1012845?fromtitle=x86&fromid=6150538)
+ [什么是i386,x86和x64, 一文了解处理器架构](https://baike.baidu.com/tashuo/browse/content?id=5d5bb1f8a73bd4495d7b21a5&lemmaId=433177&fromLemmaModule=pcBottom&lemmaTitle=Intel%2080386)
+ [iOS逆向 ：初识汇编](https://zhuanlan.zhihu.com/p/369071456)
+ [深入iOS系统底层之程序中的汇编代码](https://www.jianshu.com/p/f649285668cd)
+ [ios-crash-dump-analysis-book](https://github.com/faisalmemon/ios-crash-dump-analysis-book)
+ [ios-crash-dump-analysis-book/zh](https://faisalmemon.github.io/ios-crash-dump-analysis-book/zh/)
+ [iOS Crash/崩溃/异常 堆栈获取](https://www.jianshu.com/p/8ece78d71b3d)
+ [iOS堆栈信息解析（函数地址与符号关联）](https://www.jianshu.com/p/df5b08330afd)
+ [Mach微内核简介](https://wangkejie.com/iOS/kernelarchitecture/mach.html)
+ [Mach Overview](https://developer.apple.com/library/archive/documentation/Darwin/Conceptual/KernelProgramming/Mach/Mach.html)
+ [谈谈iOS堆栈那些事](https://joey520.github.io/2020/03/15/谈谈msgSend为什么不会出现在堆栈中/)
+ [iOS 的崩溃捕获-堆栈符号化-崩溃分析](https://www.jianshu.com/p/302ed945e9cf)
+ [函数栈的实现原理](https://segmentfault.com/a/1190000017151354)
+ [函数调用栈 剖析＋图解[转]](https://www.jianshu.com/p/78e01e513120)
+ [[转载]C语言函数调用栈](https://www.jianshu.com/p/c89d243b8276)
+ [BSBackTracelogger学习笔记](https://juejin.cn/post/6910791727670362125)
+ [如何定位Obj-C野指针随机Crash(一)：先提高野指针Crash率](https://cloud.tencent.com/developer/article/1070505)
+ [如何定位Obj-C野指针随机Crash(二)：让非必现Crash变成必现](https://cloud.tencent.com/developer/article/1070512)
+ [如何定位Obj-C野指针随机Crash(三)：如何让Crash自报家门](https://cloud.tencent.com/developer/article/1070528)
+ [iOS/OSX Crash：捕捉异常](https://zhuanlan.zhihu.com/p/271282052)
+ [汇编过程调用是怎样操作栈的？](https://www.zhihu.com/question/49410551/answer/115870825)
+ [ARM三级流水线](https://blog.csdn.net/qq_34127958/article/details/72791382)
