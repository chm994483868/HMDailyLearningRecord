# iOS App Crash 分析：(一)：dsym文件讲解

## DWARF

&emsp;DWARF 是许多编译器和调试器用来支持源码级调试的 **调试文件格式**。它满足了许多过程语言（例如 C、C++ 和 Fortran）的要求，并且可以扩展到其他语言。 DWARF 是独立于架构的，适用于任何处理器或操作系统。它广泛用于 Unix、macOS、Linux 等操作系统，以及单机环境。[The DWARF Debugging Standard](http://dwarfstd.org)

&emsp;DWARF 是一种被广泛使用的标准化 [Debugging data format](https://en.wikipedia.org/wiki/Debugging_data_format)。DWARF 最初是与 [Executable and Linkable Format (ELF)](https://en.wikipedia.org/wiki/Executable_and_Linkable_Format) 一起设计的，尽管它独立于 [object file](https://en.wikipedia.org/wiki/Object_file) 格式。DWARF 这个名字是对 “ELF” 的 [medieval fantasy](https://en.wikipedia.org/wiki/Historical_fantasy#Medieval_fantasy) 补充，没有官方意义，尽管后来提出了 "Debugging With Arbitrary Record Formats" 的首字母缩写。

+ [Debugging data format](https://en.wikipedia.org/wiki/Debugging_data_format)

调试数据格式是存储有关汇编计算机程序的信息供高级调试者使用的一种手段。现代调试数据格式存储了足够的信息，以便进行源级调试。

高级调试器需要有关变量、类型、常数、子常规等的信息，因此他们可以在机器级存储和源语言构造之间进行翻译。此类信息也可以由其他软件工具使用。信息必须由编译器生成，并由链接器存储在可执行文件或动态库中。

某些对象文件格式包括调试信息，但其他对象可以使用通用调试数据格式，如刺和矮人。



## dSYM 文件概述

&emsp;dSYM 是内存地址与函数名、文件名和行号的映射表，一般用来进行崩溃日志分析。<起始地址> <结束地址> <函数> [<文件名: 行号>]

### DWARF 

&emsp;**DWARF** 是一种被众多编译器和调试器使用的用于支持 **源码级别** 调试的 **调试文件格式**，该格式是一个固定的数据格式。dSYM 就是按照 **DWARF** 格式保存调试信息的文件，也就是说 dSYM 是一个文件。

&emsp;**DWARF** 是一种调试信息格式，通常用于源码级别调试，也可用于从运行时地址还原源码对应的符号以及行号的工具（如：atos）。

&emsp;**Strip Linked Product** **Deployment Postprocessing**

## symbolicatecrash

## atos 单/多行符号化

## dwarfdump



## 参考链接
**参考链接:🔗**
+ [iOS 符号解析重构之路](https://mp.weixin.qq.com/s/TVRYXhiOXIsMmXZo9GmEVA)
+ [dSYM文件的汇编分析](https://juejin.cn/post/6925618080941146125)
+ [iOS的调试文件dSYM与DWARF](https://juejin.cn/post/6983302313586884616)
