# Clang 学习笔记

## clang 概述

> &emsp;clang - the Clang C, C++, Objective-C, and Objective-C++ compiler

&emsp;表示概念上的一词多义：Clang 和 LLVM 都会有多个概念，例如：Clang 项目、LLVM 编译器/LLVM 项目/LLVM 编译器架构（表示整个 LLVM 编译器框架的程序，包括了前端、优化器、后端、汇编器、链接器，以及 libc++、JIT 等）、LLVM 核心（core）、Clang 前端、Clang 驱动程序、Clang 命令等，而大部分情况下大家会直接用 Clang 和 LLVM 称呼，此时我们要根据上下文信息来推断此时所代表的含义。 

&emsp;Clang（Clang 前端）是一个 C/C++/Objective-C/Objective-C++ 的编译器前端（clang 编译器前端，主要处理一些和具体机器无关的针对语言的分析操作，LLVM 作为编译器后端，可进一步编译为与平台相关的机器语言），而 clang 命令/Clang 驱动程序 则驱动了 LLMM 编译器架构的各个功能模块进行完整的编译、链接等功能，clang 命令能够调用起来编译器的整个工作流程，所以 Clang 此时代表的含义不止是前端编译器是驱动程序：驱动了 LLVM 整个编译过程中所使用的各个工具模块。

> &emsp;通常我们在命令行上调用的 clang 工具，是 Clang 驱动程序，因为 LLVM 本质上只是一个编译器框架，所以需要一个驱动程序把整个编译器的功能串起来，clang 能够监控整个编译器的流程，即能够调用到 Clang 和 LLVM 的各种库，最终实现编译的功能。[LLVM基本概念入门](https://zhuanlan.zhihu.com/p/140462815) 

> &emsp;Clang（发音为 /ˈklæŋ/ 类似英文单字 clang）是一个 C、C++、Objective-C 和 Objective-C++ 编程语言的编译器前端。
  Clang 是 LLVM 编译器工具集的前端（front-end），目的是输出开发者编写的源代码对应的抽象语法树（Abstract Syntax Tree, AST），并将源代码编译成 LLVM Bitcode，接着在后端（back-end）使用 LLVM 编译成平台相关的机器语言。Clang 支持 C、C++、Objective C。
  Clang 项目（注意这里换称呼了）采用了 LLVM 作为其后端支撑，由 LLVM 2.6 开始，一起发布新版本。它的目标是提供一个 GNU 编译器套装的替代品，并支持 GNU 编译器大多数的编译设置以及非官方语言的扩展，作者是克里斯·拉特纳（Chris Lattner），在苹果公司的赞助支持下进行开发，而源代码许可是使用类 BSD 的伊利诺伊大学厄巴纳-香槟分校开源码许可。
  GNU Compiler Collection 缩写为 GCC，原名为 GNU C Compiler（GNU C 语言编译器），因为它原本只能处理 C 语言。GCC 在发布后也很快地得到扩展，变得可处理 C++，之后也变得可处理 Fortran、Pascal、Objective-C、Java、Ada，Go 与其他语言。[GCC](https://zh.wikipedia.org/wiki/GCC)）
  Clang 可以说是一个统称，包括 Clang 前端和 Clang 静态分析器等。[Clang](https://zh.wikipedia.org/wiki/Clang)
  Clang 项目在 2005 年由苹果公司发起。
  
&emsp;这里大家可能对 Clang 和 LLVM 的关系有些初步的认识了，可以看这下这：[深入研究Clang（一）Clang和LLVM的关系及整体架构](https://zhuanlan.zhihu.com/p/26223459) 和 [LLVM基本概念入门](https://zhuanlan.zhihu.com/p/140462815) 两篇文章加深理解。

&emsp;下面我们对 Clang 的使用进行学习。 

&emsp;我们可以通过 `which clang` 命令看到 Clang 位于：/usr/bin/clang 大概是有直接集成在 macOS 中的，然后通过 `clang -v` 命令又看到 Xcode 中也集成了 Clang，InstalledDir: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang。

```c++
hmc@localhost ~ % which clang
/usr/bin/clang
```

```c++
hmc@localhost ~ % clang -v
Apple clang version 13.0.0 (clang-1300.0.29.3)
Target: x86_64-apple-darwin21.1.0
Thread model: posix
InstalledDir: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin
```

&emsp;(这里是我自己的一些想法，~~可能是错的~~：我们要对 "Clang 是一个编译器前端" 和 "clang 命令" 做出理解上的区别，说 Clang 是编译器前端直觉上好像给人说 Clang 只是编译器的一个组成部分，那么作为一个组成部分是不是就不能直接提供完整的编译、链接功能（这里是指把我们的程序从源码文件 "完全转换" 为可执行程序），而只能提供其中一环的支持？clang 命令则不是，它是驱动了完整的编译、链接功能的，例如: 通过 `clang main.m` 命令我们可以直接得到一个名为 `a.out` 的可执行文件（a 是默认名，我们也可以通过 `-o` 选项输出自定义的可执行文件的名字），通过 `./a.out` 便可直接执行这个可执行文件，通过 `file a.out` 我们可得出此信息（当前 Intel 平台）：`a.out: Mach-O 64-bit executable x86_64`，即当前情况下 a.out 是一个 x86_64 架构下的 Mach-O 格式的可执行文件，然后我们也可以在 `clang` 命令后面指定不同的选项 `-E、-S、-c` 来分别查看完整编译、链接过程中的不同阶段的情况（以及指定阶段的输出文件）：预编译文件、汇编文件、目标文件，所以单指 Clang 可以说它是一个编译器或者编译器前端，clang 命令的话则是高度集成的，它可以分阶段或者完整的执行整个编译、链接过程。)

## man clang

&emsp;接下来我们使用 `man clang` 命令来查看 Clang 的详细信息。

> &emsp;clang is a C, C++, and Objective-C compiler which encompasses preprocessing, parsing, optimization, code generation, assembly, and linking.  Depending on which high-level mode setting is passed, Clang will stop before doing a full link. While Clang is highly integrated, it is important to understand the stages of compilation, to understand how to invoke it.  These stages are:
  &emsp;clang 是一个 C、C++ 和 Objective-C 编译器，它包括 preprocessing（预处理）、parsing（语法分析）、optimization（优化）、code generation（代码生成，AST 到 LLVM IR）、assembly（汇编）、linking（链接）。根据 clang 命令后面指定的选项，Clang 将在执行完整链接之前停止（即使用不同的 分部执行编译连接）。虽然 Clang 是高度集成的，但重要的是要了解编译的各个阶段，了解如何调用它。

&emsp;下面我们跟着 `man clang` 输出的详细信息学习 Clang 的使用涉及的编译的各个阶段。

## Driver（驱动程序）

> &emsp;The clang executable is actually a small driver which controls the overall execution of other tools such as the compiler, assembler and linker.
  Typically you do not need to interact with the driver, but you transparently use it to run the other tools.
  
  "The clang executable" 是指我们所使用的 clang 命令所对应的 clang 可执行程序。

&emsp;Clang 可执行文件实际上是一个小驱动程序，它驱动其他工具（如编译器、汇编器和链接器）的整体执行。通常，我们不需要与驱动程序交互，但可以透明地使用它来调用其他工具。

## Preprocessing（预处理）

> &emsp;This stage handles tokenization of the input source file, macro expansion, #include expansion and handling of other preprocessor directives.  The output of this stage is typically called a ".i" (for C), ".ii" (for C++), ".mi" (for Objective-C), or ".mii" (for Objective-C++) file.

&emsp;此阶段处理输入源文件的标记化、宏扩展、#include 扩展以及其他预处理器指令的处理。 此阶段的输出通常称为“.i”（用于 C）、“.ii”（用于C++）、“.mi”（用于 Objective-C）或“.mii”（用于 Objective-C++）文件。








+ 预处理（.i .ii .mi .mii 文件）、
+ 语法分析和语义分析（AST 抽象语法树）、
+ 代码生成（LLVM IR）和优化（.s）、
+ 汇编（.o）和链接（.out .dylib .so）。
 
&emsp;Clang 根据传递的 high-level mode setting（即使用 clang 命令时指定的那些选项），Clang 将在执行完整链接（full link）之前停止。clang 命令的提供的选项有如下几组：

+ Stage Selection Options（最重要的选项）

```c++
hmc@HMdeMac-mini compile_test % clang -ccc-print-phases main.m
               +- 0: input, "main.m", objective-c
            +- 1: preprocessor, {0}, objective-c-cpp-output
         +- 2: compiler, {1}, ir
      +- 3: backend, {2}, assembler
   +- 4: assembler, {3}, object
+- 5: linker, {4}, image
6: bind-arch, "x86_64", {5}, image
```

&emsp;虽然 Clang 是高度集成的，但重要的是要了解编译的各个阶段，了解如何调用它。 这些阶段是：


+ Driver：clang 可执行文件实际上是一个小驱动程序，它控制其他工具（如编译器、汇编器和链接器）的整体执行。通常，你不需要与驱动程序交互，但可以透明地使用它以运行其他工具。
+ Preprocessing（预处理）：此阶段处理输入源文件的标记化、宏扩展、#include 扩展和其他预处理器指令的处理。此阶段的输出通常称为 ".i"（用于 C）、".ii"（用于 C++）、".mi"（用于 Objective-C）或 ".mii"（用于 Objective-C++）文件。
+ Parsing and Semantic Analysis（语法分析和语义分析）：此阶段分析输入文件，将预处理器令牌转换为分析树。 一旦采用解析树的形式，它就会将语义分析应用于表达式的计算类型，并确定代码是否良好形成。此阶段负责生成大多数编译器警告以及分析错误。此阶段的输出是“抽象语法树”（AST）。
+ Code Generation and Optimization：此阶段将 AST 转换为低级中间代码（称为 "LLVM IR"），并最终转换为机器代码。 此阶段负责优化生成的代码并处理特定于目标的代码生成。此阶段的输出通常称为 ".s" 文件或汇编文件。Clang 还支持使用集成汇编程序，其中代码生成器直接生成对象文件。这避免了生成“.s”文件和调用目标汇编程序的开销。
+ Assembler：此阶段运行目标汇编程序以将编译器的输出转换为目标对象文件。此阶段的输出通常称为 “.o” 文件或“对象”文件。
+ Linker：此阶段运行目标链接器以将多个对象文件合并到可执行文件或动态库中。此阶段的输出通常称为 “a.out”、“.dylib” 或 “.so” 文件。
+ Clang Static Analyzer：Clang 静态分析器是一种扫描源代码以尝试通过代码分析来查找 bug 的工具。 此工具使用 Clang 的许多部分，并内置于同一驱动程序中。









## clang 和 GNU 的联系



## 参考链接
**参考链接:🔗**
+ [Clang与LLVM的关系](https://blog.csdn.net/u010164190/article/details/104901279)
+ [iOS程序员的自我修养-编译、链接过程（一）](https://juejin.cn/post/6844903912147795982)
+ [clang常用语法介绍](https://www.jianshu.com/p/96058bf1ecc2)
+ [Clang编译步骤及命令梳理](https://bbs.huaweicloud.com/blogs/314686?utm_source=zhihu&utm_medium=bbs-ex&utm_campaign=paas&utm_content=content)
+ [谁说不能与龙一起跳舞：Clang / LLVM (1)](https://zhuanlan.zhihu.com/p/21889573)
+ [LLVM基本概念入门](https://zhuanlan.zhihu.com/p/140462815)
+ [clang 源码导读（2）: clang driver 流程简介](https://cloud.tencent.com/developer/article/1803206)
+ [GNU GCC使用ld链接器进行链接的完整过程是怎样的？](https://www.zhihu.com/question/27386057)
+ [clang driver](https://www.jianshu.com/p/e816e0209827)
