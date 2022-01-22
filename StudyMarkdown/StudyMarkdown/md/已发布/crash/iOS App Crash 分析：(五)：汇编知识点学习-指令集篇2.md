# iOS App Crash 分析：(五)：汇编知识点学习-指令集篇

&emsp;汇编语言作为机器语言的助记语言应运而生，当你深入的应用它时就会发现汇编语言其实并没有那么的复杂。在一个程序所使用的机器指令中，大部分的指令所做的事情除了计算外就是将数据在寄存器与寄存器之间以及寄存器与内存之间进行移动。在高级语言中我们可以定义名字非常个性化的变量以及无限制数量的变量，而在低级语言中我们则只能使用那几个有限的寄存器来作为临时变量，以及像访问数组那样根据绝对地址/相对地址去访问内存中指定地址保存的数据。

&emsp;下面我们对一些基础知识做一个铺垫。

&emsp;常见的指令集：

+ Intel：x86（1978），EM64T，MMX（1996.10.12 公布），SSE（1999.5.1 随 Pentium 3 CPU 时推出），SSE2（2000.11.1 随 Pentium 4 CPU 推出），SSE3（2004.2.1），SSSE3 (Super SSE3 2006.1.1)，SSE4A（2007.11.11），SSE4.1，SSE4.2，AVX（2008.3.1），AVX2（2011.6.13），AVX-512，VMX 等指令集。
  （SSE 指令集是 x86 架构下的一系列指令，一条指令可以实现多项数据运算，即 SIMD-Single Instruction Multiple Data。）
  
[x86:SIMD指令集发展历程表（MMX、SSE、AVX 等）](https://blog.csdn.net/weixin_34122604/article/details/86271850?spm=1001.2101.3001.6650.1&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7ERate-1.pc_relevant_default&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7ERate-1.pc_relevant_default&utm_relevant_index=2)

+ AMD：x86，x86-64，3D-Now!（1998.1.1）（重点提高了 AMD 公司 K6 系列 CPU 对 3D 图形的处理能力）指令集。

&emsp;常见的 CPU 架构：

+ Inter x86 架构
+ ARM armv7、armv8、arm64、arm64e 架构
+ IBM PowerPC 架构

## 指令集（指令系统）概述

&emsp;计算机指令就是指挥机器工作的指示和命令，程序就是一系列按一定顺序排列的指令，执行程序的过程就是计算机的工作过程。

&emsp;指令集是 CPU 中用来计算和控制计算机系统的一套指令的集合，而每一种新型的 CPU 在设计时就规定了一系列与其他硬件电路相配合的指令系统，而指令集的先进与否，也关系到 CPU 的性能发挥，它也是 CPU 性能体现的一个重要标志。指令的强弱也是 CPU 的重要指标，指令集是提高微处理器效率的最有效的工具之一。从现阶段的主流体系结构讲，指令集可分为复杂指令集和精简指令集两部分。

&emsp;**在计算机中，指示计算机硬件执行某种运算、处理功能的命令称为指令。指令是计算机运行的最小的功能单位，而硬件的作用是完成每条指令规定的功能。**

&emsp;一台计算机上（或者说计算机内部的 CPU 上）全部指令的集合，就是这台计算机的指令系统。指令系统也称指令集，是这台计算机全部功能的体现。

&emsp;人们设计计算机首要考虑的是它拥有的功能，也就是首先要按功能档次设计指令集，然后按指令集的要求在硬件上实现。指令系统不仅仅是指令的集合，还包括全部指令的指令格式、寻址方式和数据形式。所以，各计算机执行的指令系统不仅决定了机器所要求的能力，而且也决定了指令的格式和机器的结构。反过来说，不同结构的机器和不同的指令格式应该具有与之相匹配的指令系统。为此，设计指令系统时，要对指令格式、类型及操作功能给予应有的重视。软件是为了使用计算机而编写的各种系统和用户的程序，程序由一个序列的计算机指令组成。从这个角度上说，指令是用于设计程序的一种计算机语言单位。

&emsp;计算机的指令系统是指一台计算机上全部指令的集合，也称计算机的指令集。指令系统包括指令格式、寻址方式和数据形式。一台计算机的指令系统反映了该计算机的全部功能，机器类型不同，其指令系统也不同，因而功能也不同。指令系统的设置和机器的硬件结构密切相关，一台计算机要有较好的性能，必须设计功能齐全、通用性强、内含丰富的指令系统，这就需要复杂的硬件结构来支持。

&emsp;常见的指令集有：Intel 的 x86，EM64T，MMX，SSE（随 Pentium 3 CPU 时推出），SSE2（随 Pentium 4 CPU 推出），SSE3，SSSE3 (Super SSE3)，SSE4A，SSE4.1，SSE4.2，AVX，AVX2，AVX-512，VMX 等指令集；和 AMD 的 x86，x86-64，3D-Now!（重点提高了 AMD 公司 K6 系列 CPU 对 3D 图形的处理能力）指令集。[指令集](https://baike.baidu.com/item/指令集/238130?fr=aladdin)

### x86/x86-64 指令集

&emsp;x86 指令集是 Intel 为其第一块 16 位 CPU(i8086) 专门开发的，IBM 1981 年推出的世界第一台 PC 机中的 CPU—i8088（i8086 简化版）使用的也是 x86 指令集，同时电脑中为提高浮点数据处理能力而增加的 x87 芯片系列数学协处理器则另外使用 x87 指令集。（Intel 推出 CPU，IBM 推出搭载该 CPU 的 PC 机。）

&emsp;x86 是当前最成功的 CPU 架构/指令集。以后就将 x86 指令集和 x87 指令集统称为 x86 指令集。虽然随着 CPU 技术的不断发展，Intel 陆续研制出更新型的 i80386、i80486，但为了保证电脑能继续运行以往开发的各类应用程序以保护和继承丰富的软件资源，所以 Intel 公司所生产的所有 CPU 仍然继续使用 x86 指令集，所以它的 CPU 仍属于 x86 系列。

&emsp;由于 Intel x86 系列及其兼容 CPU 都使用 x86 指令集，所以就形成了庞大的 x86 系列及兼容 CPU 阵容。

&emsp;到 2002 年，由于 32 位特性的长度，x86 架构开始到达某些设计上的极限，例如要处理大量的信息储存大于 4GB 会有困难。Intel 原本已经决定在 64 位的时代完全地舍弃 x86 兼容性，推出新的架构称为 IA-64 技术作为它的 Itanium 处理器产品线的基础（x86 也称 IA-32）。IA-64 与 x86 的软件天生不兼容；它使用各种模拟形式来运行 x86 的软件，不过，以模拟方式来运行的效率十分低下，并且会影响其他程序的运行。AMD 公司则主动把 32 位 x86（IA-32）扩充为 64 位。它以一个称为 AMD64 的架构出现（在重命名前也称为 x86-64），且以这个技术为基础的第一个产品是单内核的 Opteron 和 Athlon 64 处理器家族。由于 AMD 的 64 位处理器产品线首先进入市场，且微软也不愿意为 Intel 和 AMD 开发两套不同的 64 位操作系统，Intel 也被迫采纳 AMD64 指令集且增加某些新的扩充到他们自己的产品，命名为 EM64T 架构（显然他们不想承认这些指令集是来自它的主要对手），EM64T 后来被 Intel 正式更名为 Intel 64(也就是 x64 指令集)。

### RISC（精简指令集）/CISC（复杂指令集）

&emsp;所谓指令的复杂度就是指 CPU 指令集中所提供的指令的数量、指令寻址模式、指令参数、以及 CPU 内部的架构设计的复杂度、以及指令本身所占据的字节数等来进行划分的一种方式。

&emsp;一般有两种类型的分类：复杂指令集、精简指令集。（指令本身所占据的字节数是个重点，x86-64 架构体系指令长度是变长的从 1 个到 15 个字节不等，而 ARM64 架构体系指令的长度是固定的 4 个字节，这里为后续以不同的方式回溯函数调用堆栈埋下伏笔。）

1. CISC 指令集。CISC 的英文全称为 "Complex Instruction Set Computer"，即 "复杂指令系统计算机"，从计算机诞生以来，人们一直沿用 CISC 指令集方式。早期的桌面软件是按 CISC 设计的，并一直沿续到现在。目前，桌面计算机流行的 x86 体系结构即使用 CISC。在 CISC 微处理器中，程序的各条指令是按顺序串行执行的，每条指令中的各个操作也是按顺序串行执行的。顺序执行的优点是控制简单，但计算机各部分的利用率不高，执行速度慢。CISC 架构的服务器主要以 x86/x64 架构（Intel Architecture）为主，而且多数为中低档服务器所采用。

2. RISC 指令集。RISC 的英文全称为 "Reduced Instruction Set Computer"，即 "精简指令集计算机"。相比 CISC（复杂指令集）而言，RISC 的指令格式统一，种类比较少，寻址方式也比复杂指令集少，这样一来，它能够以更快的速度执行操作。使用 RISC 指令集的体系结构主要有 ARM、MIPS。RISC 起源于 80 年代的 MIPS 主机(即 RISC 机)，RISC 机中采用的微处理器统称 RISC 处理器。目前的智能移动设备中的 CPU 几乎都采用 RISC 指令集，比较有代表的就是 ARM 指令集和 POWER-PC 指令集。

## CPU 架构

&emsp;CPU架构是CPU厂商给属于同一系列的CPU产品定的一个规范，主要目的是为了区分不同类型CPU的重要标示。市面上的CPU分类主要分有两大阵营，一个是intel、AMD为首的复杂指令集CPU，另一个是以IBM、ARM为首的精简指令集CPU。两个不同品牌的CPU，其产品的架构也不相同，例如，Intel、AMD的CPU是X86架构的，而IBM公司的CPU是PowerPC架构，ARM公司是ARM架构。[处理器架构](https://baike.baidu.com/item/处理器架构/8535061)

### Intel x86（架构）

&emsp;Intel x86（Intel 8086）是英特尔公司于 1978 年推出的 16 位微处理器，x86 架构也是随 Intel 8086 微处理器首度出现。

&emsp;**x86（x86 架构）泛指一系列基于 Intel 8086 且向后兼容的中央处理器指令集架构。** Intel 8086 在三年后为 IBM PC 所选用，之后 x86 便成为了个人计算机的标准平台，成为了历来最成功的 CPU 架构。

&emsp;Intel 在早期以 80x86 这样的数字格式来命名处理器，包括 Intel 8086、80186、80286、80386 以及 80486，由于以 "86" 作为结尾，因此其架构被称为 "x86"。由于数字并不能作为注册商标，因此 Intel 及其竞争者均在新一代处理器使用可注册的名称，如奔腾（Pentium）、酷睿（Core）、锐龙（Ryzen，AMD 推出）。

&emsp;Intel 之外其他公司也有制造 x86 架构的处理器，其中最成功的制造商为 AMD，其早先产品 Athlon 系列处理器的市场份额仅次于 Intel Pentium。

&emsp;Intel 8086 是 16 位处理器，直到 1985 年 32 位的 80386 的开发，这个架构都维持是 16 位。接着一系列的处理器表示了 32 位架构的细微改进，推出了数种的扩充，直到 2003 年 AMD 对于这个架构发展了 64 位的扩充，并命名为 AMD64。后来 Intel 也推出了与之兼容的处理器，并命名为 Intel 64。两者一般被统称为 x86-64 或 x64，开创了 x86 的 64 位时代。

&emsp;x86 的 32 位架构一般又被称作 IA-32，全名为 "Intel Architecture, 32-bit"。其 64 位架构由 AMD 率先推出，并被称为 "AMD64"。之后也被 Intel 采用，被其称为 "Intel 64"。一般也被称作 "x86-64"、"x64"。[X86架构](https://baike.baidu.com/item/X86架构/7470217)

## CPU 架构历史概述

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
