# 标题暂时没有想好

## clang 概述

> &emsp;clang - the Clang C, C++, Objective-C, and Objective-C++ compiler

&emsp;clang 是 C/C++/Objective-C/Objective-C++ 的编译器前端（主要处理一些和具体机器无关的针对语言的分析操作），我们可以通过 `which clang` 命令看到 clang 位于：/usr/bin/clang，大概是直接集成在 macOS 中的，接下来我们使用 `man clang` 命令来查看 clang 的详细信息。

&emsp;clang 是一个 C、C++ 和 Objective-C 编译器，它包括预处理（.i .ii .mi .mii 文件）、语法分析和语义分析（AST 抽象语法树）、代码生成（LLVM IR）和优化（.s）、汇编（.o）和链接（.out .dylib .so）。 根据传递的高级模式设置，Clang 将在执行完整链接之前停止。虽然 Clang 是高度集成的，但重要的是要了解编译的各个阶段，了解如何调用它。

+ Driver：clang 可执行文件实际上是一个小驱动程序，它控制其他工具（如编译器、汇编器和链接器）的整体执行。通常，你不需要与驱动程序交互，但可以透明地使用它以运行其他工具。
+ Preprocessing（预处理）：此阶段处理输入源文件的标记化、宏扩展、#include 扩展和其他预处理器指令的处理。此阶段的输出通常称为 ".i"（用于 C）、".ii"（用于 C++）、".mi"（用于 Objective-C）或 ".mii"（用于 Objective-C++）文件。
+ Parsing and Semantic Analysis（语法分析和语义分析）：此阶段分析输入文件，将预处理器令牌转换为分析树。 一旦采用解析树的形式，它就会将语义分析应用于表达式的计算类型，并确定代码是否良好形成。此阶段负责生成大多数编译器警告以及分析错误。此阶段的输出是“抽象语法树”（AST）。
+ Code Generation and Optimization：此阶段将 AST 转换为低级中间代码（称为 "LLVM IR"），并最终转换为机器代码。 此阶段负责优化生成的代码并处理特定于目标的代码生成。此阶段的输出通常称为 ".s" 文件或汇编文件。Clang 还支持使用集成汇编程序，其中代码生成器直接生成对象文件。这避免了生成“.s”文件和调用目标汇编程序的开销。
+ Assembler：此阶段运行目标汇编程序以将编译器的输出转换为目标对象文件。此阶段的输出通常称为 “.o” 文件或“对象”文件。
+ Linker：此阶段运行目标链接器以将多个对象文件合并到可执行文件或动态库中。此阶段的输出通常称为 “a.out”、“.dylib” 或 “.so” 文件。
+ Clang Static Analyzer：Clang 静态分析器是一种扫描源代码以尝试通过代码分析来查找 bug 的工具。 此工具使用 Clang 的许多部分，并内置于同一驱动程序中。





## 参考链接
**参考链接:🔗**
+ [Clang与LLVM的关系](https://blog.csdn.net/u010164190/article/details/104901279)
+ [iOS程序员的自我修养-编译、链接过程（一）](https://juejin.cn/post/6844903912147795982)

