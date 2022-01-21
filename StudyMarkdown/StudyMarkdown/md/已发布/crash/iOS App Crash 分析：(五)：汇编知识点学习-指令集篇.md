# iOS App Crash 分析：(五)：汇编知识点学习-指令集篇

&emsp;汇编语言作为机器语言的助记语言应运而生，当你深入的应用它时就会发现汇编语言其实并没有那么的复杂。在一个程序的机器指令中，大部分的指令所做的事情除了计算外就是将数据在寄存器与寄存器之间以及寄存器与内存之间进行移动。在高级语言中我们可以定义名字非常个性化的变量以及无限制数量的变量，而在低级语言中我们则只能使用那几个有限的寄存器来作为临时变量，以及像访问数组那样去访问内存地址。

&emsp;**我们称一个 CPU 里面所提供的所有的指令的集合称之为指令集（针对此 CPU 的指令集）。**

&emsp;不同的厂家以及不同的技术工艺和技术水平以及具体的设备上所实现的 CPU 的体系架构以及提供的功能也是有差异的。比如 ARM 指令架构体系的 CPU、x86 指令体系架构的 CPU、POWER-PC 指令架构体系的 CPU。这些不同体系的 CPU 因为架构完全不同导致所提供的指令和存储单元也完全不同。我们不可能让 ARM 指令直接在 x86 的 CPU 上执行。相同体系架构下的 CPU 指令则在一定程度上是可以相互兼容的，因为相同架构体系下的 CPU 的指令集是一致的(类比为接口一致，但是内部实现则不相同)，比如说 Intel 公司所生产的 x86 系列的 CPU 和 AMD 公司所生产的 x86 系列 CPU 所提供的指令集是相似和兼容的，他们之间的差别则只是在于内部的实现不同。

&emsp;CPU 指令集定义的是一个中央处理器所应该提供的基础功能的集合，它是 **一个标准**、是 **一个接口**、也是 **一个协议**。在软件开发中具有 **协议** 和 **接口** 定义的概念，无论是消费者还是提供者都需要遵循这个标准来进行编程和交互。提供者要实现接口所具有的功能，至于如何实现则是内部的事情，不对外暴露，消费者也不需要知道具体的实现细节，消费者则总是要按接口提供的功能方法并组合使用来完成某种功能。这种设计的思维对于硬件系统也是一样适用的。

&emsp;看到这里脑子里真的很糊，还是对下面的背景知识做一下铺垫，看一下：架构体系、指令集 到底是怎么定义的。

》〉》〉》〉》〉》〉 总结下面两篇文章的内容： 

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
