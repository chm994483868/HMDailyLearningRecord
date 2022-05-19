# 标题暂时没有想好

## clang 概述

> &emsp;clang - the Clang C, C++, Objective-C, and Objective-C++ compiler

&emsp;Clang 是一个 C/C++/Objective-C/Objective-C++ 的编译器（编译器前端，主要处理一些和具体机器无关的针对语言的分析操作，LLVM 作为编译器后端），我们可以通过 `which clang` 命令看到 Clang 位于：/usr/bin/clang 大概是有直接集成在 macOS 中的，然后通过 `clang -v` 命令又看到 Xcode 中也集成了 Clang，InstalledDir: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang。

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

&emsp;(这里是我自己的一些想法，可能是错的：我们要对 "Clang 是一个编译器前端" 和 "clang 命令" 做出理解上的区别，说 Clang 是编译器前端直觉上好像给人说 Clang 只是编译器的一个组成部分，那么作为一个组成部分是不是就不能直接提供完整的编译、链接功能（这里是指把我们的程序从源码文件 "完全转换" 为可执行程序）？clang 命令则不是，它是集成了完整的编译、链接功能的，例如: 通过 `clang main.m` 命令我们可以直接得到一个名为 `a.out` 的可执行文件（a 是默认名，我们也可以通过 `-o` 选项指定自定义的可执行文件的名字），通过 `./a.out` 便可直接执行这个可执行文件，通过 `file a.out` 我们可得出类似信息：`a.out: Mach-O 64-bit executable x86_64`，即当前情况下 a.out 是一个 x86_64 架构下的 Mach-O 格式的可执行文件，然后我们也可以在 `clang` 命令后面指定不同的选项 `-E、-S、-c` 来分别查看完整编译、链接过程中的不同阶段的情况（以及指定阶段的输出文件）：预编译文件、汇编文件、目标文件，所以单指 Clang 可以说它是一个编译器或者编译器前端，clang 命令的话则是高度集成的，它可以分阶段或者完整的执行整个编译、链接过程。)

&emsp;接下来我们使用 `man clang` 命令来查看 Clang 的详细信息。

> &emsp;clang is a C, C++, and Objective-C compiler which encompasses preprocessing, parsing, optimization, code generation, assembly, and linking.  Depending on which high-level mode setting is passed, Clang will stop before doing a full link. While Clang is highly integrated, it is important to understand the stages of compilation, to understand how to invoke it.  These stages are:

&emsp;Clang 包括如下功能：preprocessing（预处理）、parsing（语法分析）、optimization（优化）、code generation（代码生成，AST 到 LLVM IR）、assembly（汇编）、linking（链接）：

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
