# 浅学 Clang（一）：clang 指令介绍

## clang LLVM 概述

> &emsp;clang - the Clang C, C++, Objective-C, and Objective-C++ compiler

&emsp;首先是表示概念上的一词多义：Clang 和 LLVM 都会有多个表示概念，而大部分情况下大家会直接用 Clang 和 LLVM 称呼它们，此时我们需要根据上下文信息来推断 Clang 和 LLVM 所表达的含义。Clang 和 LLVM 有如下多个含义：Clang 项目、Clang 前端、Clang 驱动程序、Clang 命令、LLVM 核心（core）、LLVM 编译器/LLVM 项目/LLVM 编译器架构等等。LLVM 泛指 LLVM 编译框架，其中包含：前端、优化器、后端、汇编器、链接器，以及 libc++、JIT 等等。Clang 泛指 LLVM 编译器前端，Clang 是一个 C/C++/Objective-C/Objective-C++ 的编译器前端，主要处理一些和具体机器无关的针对语言的分析操作，LLVM 作为编译器后端，可进一步编译为与平台相关的机器语言，而 clang 命令/Clang 驱动程序 则驱动了 LLVM 整个编译过程中所使用的各个工具模块。

&emsp;通常我们在终端上使用的 clang 命令是 Clang 驱动程序，因为 LLVM 编译器框架内部包含众多的工作模块，此时需要一个驱动程序把整个编译器的功能块串起来，Clang Driver 正完成了这个任务，并且 clang 能够监控整个编译器的流程，能够调用到 Clang 和 LLVM 的各种库，最终实现编译的功能。[LLVM基本概念入门](https://zhuanlan.zhihu.com/p/140462815) 

&emsp;Clang（发音为 /ˈklæŋ/ 类似英文单词 clang）是一个 C、C++、Objective-C 和 Objective-C++ 编程语言的编译器前端。Clang 是 LLVM 编译器工具集的前端（front-end），目的是输出开发者编写的源代码对应的抽象语法树（Abstract Syntax Tree, AST），并将源代码编译成 LLVM Bitcode，接着在后端（back-end）使用 LLVM 编译成平台相关的机器语言。Clang 支持 C、C++、Objective C。Clang 项目（注意这里换称呼了）采用了 LLVM 作为其后端支撑，由 LLVM 2.6 开始，一起发布新版本。它的目标是提供一个 GNU 编译器套装的替代品，并支持 GNU 编译器大多数的编译设置以及非官方语言的扩展，作者是克里斯·拉特纳（Chris Lattner），在苹果公司的赞助支持下进行开发，而源代码许可是使用类 BSD 的伊利诺伊大学厄巴纳-香槟分校开源码许可。GNU Compiler Collection 缩写为 GCC，原名为 GNU C Compiler（GNU C 语言编译器），因为它原本只能处理 C 语言。GCC 在发布后也很快地得到扩展，变得可处理 C++，之后也变得可处理 Fortran、Pascal、Objective-C、Java、Ada，Go 与其他语言。[GCC](https://zh.wikipedia.org/wiki/GCC)）Clang 可以说是一个统称，包括 Clang 前端和 Clang 静态分析器等。[Clang](https://zh.wikipedia.org/wiki/Clang) Clang 项目在 2005 年由苹果公司发起。
  
&emsp;这里大家可能对 Clang 和 LLVM 的关系有些初步的认识了，可以看这下这：[深入研究Clang（一）Clang和LLVM的关系及整体架构](https://zhuanlan.zhihu.com/p/26223459) 和 [LLVM基本概念入门](https://zhuanlan.zhihu.com/p/140462815) 两篇文章加深理解。

&emsp;上面是我们对 Clang 和 LLVM 一些概念上的理解，下面我们对 Clang 的使用进行学习。 

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

&emsp;(这里是我自己的一些想法，~~可能是错的~~：我们要对 "Clang 是一个编译器前端" 和 "clang 命令" 做出理解上的区别，说 Clang 是编译器前端直觉上好像给人说 Clang 只是编译器的一个组成部分，那么作为一个组成部分是不是就不能直接提供完整的编译、链接功能（这里是指把我们的程序从源码文件 "完全转换" 为可执行程序），而只能提供其中一环的支持？clang 命令则不是，它是驱动了完整的编译、链接功能的，例如: 通过 `clang main.c` 命令我们可以直接得到一个名为 `a.out` 的可执行文件（a 是默认名，我们也可以通过 `-o` 选项输出自定义的可执行文件的名字），通过 `./a.out` 便可直接执行这个可执行文件，通过 `file a.out` 我们可得出此信息（当前 Intel 平台）：`a.out: Mach-O 64-bit executable x86_64`，即当前情况下 a.out 是一个 x86_64 架构下的 Mach-O 格式的可执行文件，然后我们也可以在 `clang` 命令后面指定不同的选项 `-E、-S、-c` 来分别查看完整编译、链接过程中的不同阶段的情况（以及指定阶段的输出文件）：预编译文件、汇编文件、目标文件，所以单指 Clang 可以说它是一个编译器或者编译器前端，clang 命令的话则是高度集成的，它可以分阶段或者完整的执行整个编译、链接过程。)

## man clang

&emsp;接下来我们使用 `man clang` 命令来查看 Clang 的详细信息。

> &emsp;clang is a C, C++, and Objective-C compiler which encompasses preprocessing, parsing, optimization, code generation, assembly, and linking.  Depending on which high-level mode setting is passed, Clang will stop before doing a full link. While Clang is highly integrated, it is important to understand the stages of compilation, to understand how to invoke it.  These stages are:
  &emsp;clang 是一个 C、C++ 和 Objective-C 编译器，它包括 preprocessing（预处理）、parsing（语法分析）、optimization（优化）、code generation（代码生成，AST 到 LLVM IR）、assembly（汇编）、linking（链接）。根据 clang 命令后面指定的选项，Clang 将在执行完整链接之前停止（即使用不同的 Stage Selection Options 分阶段执行编译链接工作）。虽然 Clang 是高度集成的，但重要的是要了解编译的各个阶段，了解如何调用它。

&emsp;下面我们跟着 `man clang` 输出的详细信息学习 Clang 的使用以及涉及的编译链接的各个阶段以及其中涉及的不同的选项参数的含义。

## Driver（驱动程序）

> &emsp;The clang executable is actually a small driver which controls the overall execution of other tools such as the compiler, assembler and linker.
  Typically you do not need to interact with the driver, but you transparently use it to run the other tools.
  
  "The clang executable" 是指我们所使用的 clang 命令所对应的 clang 可执行程序。

&emsp;Clang 可执行文件实际上是一个小驱动程序，它驱动其他工具（如编译器、汇编器和链接器等）的整体执行。通常，我们不需要与驱动程序交互，但可以透明地使用它来调用其他工具。

&emsp;Clang Driver 负责拼接编译器命令和 ld 命令。

## Preprocessing（预处理）

> &emsp;This stage handles tokenization of the input source file, macro expansion, #include expansion and handling of other preprocessor directives.  The output of this stage is typically called a ".i" (for C), ".ii" (for C++), ".mi" (for Objective-C), or ".mii" (for Objective-C++) file.

&emsp;此阶段处理输入源文件的标记化、宏定义展开、#include 引入展开（#include/#import 引入的文件递归插入到当前）以及其他预处理器指令的处理。此阶段的输出文件格式通常为：".i"（用于 C）、".ii"（用于 C++）、".mi"（用于 Objective-C）或 ".mii"（用于 Objective-C++）文件。

## Parsing and Semantic Analysis（语法分析和语义分析）

> &emsp;This stage parses the input file, translating preprocessor tokens into a parse tree. Once in the form of a parse tree, it applies semantic analysis to compute types for expressions as well and determine whether the code is well formed. This stage is responsible for generating most of the compiler warnings as well as parse errors. The output of this stage is an "Abstract Syntax Tree" (AST).

&emsp;此阶段分析源文件，将预处理器指令转换为语法分析树。 一旦采用语法分析树的形式，它就会将语义分析应用于表达式的计算类型，并确定代码是否格式正确。此阶段负责生成大多数编译器警告以及分析错误。此阶段的输出是 "Abstract Syntax Tree"（抽象语法树 AST）。

## Code Generation and Optimization（代码（LLVM IR）生成和优化）

> &emsp;This stage translates an AST into low-level intermediate code (known as "LLVM IR") and ultimately to machine code. This phase is responsible for optimizing the generated code and handling target-specific code generation. The output of this stage is typically called a ".s" file or "assembly" file. Clang also supports the use of an integrated assembler, in which the code generator produces object files directly. This avoids the overhead of generating the ".s" file and of calling the target assembler.
              
&emsp;此阶段将抽象语法树 AST 转换为 low-level 中间代码（被称为 "LLVM IR"），并最终转换为机器码（machine code）。此阶段负责优化生成的代码并处理特定目标的代码生成。此阶段的输出通常为 ".s" 文件或 "assembly" 汇编文件。Clang 还支持使用集成的汇编器（assembler），其中代码生成器（code generator）直接生成目标文件（object file ".o" 文件）。这避免了生成 ".s" 文件和调用目标汇编器的开销。（当然如果我们想要查看我们的源文件生成的汇编文件，也可以使用 -S 选项来指定编译到此阶段停下来）

## Assembler（汇编器）

> &emsp;This stage runs the target assembler to translate the output of the compiler into a target object file. The output of this stage is typically called a ".o" file or "object" file.

&emsp;此阶段运行目标汇编器以将编译器的输出转换为目标文件。此阶段的输出通常为 ".o" 文件或 "object" 目标文件。

## Linker

> &emsp;This stage runs the target linker to merge multiple object files into an executable or dynamic library. The output of this stage is typically called an "a.out", ".dylib" or ".so" file.

&emsp;此阶段运行目标链接器以将多个目标文件（.o）合并到可执行文件或动态库中。此阶段的输出通常为 "a.out"、".dylib" 或 ".so" 文件。

## Clang Static Analyzer

> &emsp;The Clang Static Analyzer is a tool that scans source code to try to find bugs through code analysis. This tool uses many parts of Clang and is built into the same driver.  Please see <https://clang-analyzer.llvm.org> for more details on how to use the static analyzer.

&emsp;Clang 静态分析器是一种扫描源代码以尝试通过代码分析来查找 bug 的工具。此工具使用 Clang 的许多部分，并内置于同一驱动程序中。有关如何使用静态分析器的更多详细信息，可参阅 [clang-analyzer](https://clang-analyzer.llvm.org)。

&emsp;上面看了编译各个阶段的分析，那么我们实际演示一下，我们使用 Xcode 创建一个 Command Line Tool 项目，语言选择 C 语言，然后我们 cd 到 main.c 文件所在文件夹下，执行 `clang -ccc-print-phases main.c` 指令，便可看到如下输出，看到一共经历了 5 个编译阶段，且和上面的阶段是完全对应的：（`-ccc-print-phases` 选项可用于打印编译的阶段信息，但实际并不进行编译过程）

```c++
➜  ~ cd /Users/hmc/Documents/GitHub/clang_test/compile_c/compile_c
➜  compile_c clang -ccc-print-phases main.c
               +- 0: input, "main.c", c
            +- 1: preprocessor, {0}, cpp-output
         +- 2: compiler, {1}, ir
      +- 3: backend, {2}, assembler
   +- 4: assembler, {3}, object
+- 5: linker, {4}, image
6: bind-arch, "x86_64", {5}, image
```

```c
#include <stdio.h>

int main(int argc, const char * argv[]) {
    // insert code here...
    printf("Hello, World!\n");
    return 0;
}
```

&emsp;接着我们可以执行 `clang -save-temps main.c` 指令，便可看到 main.c 所在文件夹下生成了如下文件：

```c++
a.out
main.bc
main.i
main.o
main.s
```

&emsp;它们便是编译过程中生成的全部临时文件（当然除了 a.out，它是最终的可执行文件），这些特定后缀的临时文件也正印证了上面我们所学习的编译过程。（`-save-temps` 选项用于保存中间编译结果）

## OPTIONS

### Stage Selection Options

&emsp;其中详细的解释来自于大佬的：[iOS程序员的自我修养-编译、链接过程（一）](https://juejin.cn/post/6844903912147795982)

+ `-E`: Run the preprocessor stage. 执行预处理阶段，处理源代码文件中的以 "#" 开头的预编译指令。执行 `clang -E main.c -o main.i` 指令，`-o main.i` 表示输出 `main.i` 预处理文件，如果我们不指定输出的话，则默认把预处理的结果文本直接全部打印在控制台。

&emsp;处理源代码文件中的以 "#" 开头的预编译指令，规则如下：

1. `#define` 删除并展开对应宏定义。
2. 处理所有的条件预编译指令。如 `#if/#ifdef/#else/#endif`。
3. `"#include/#import"` 包含的文件递归插入到此处。
4. 删除所有的注释 `//` 或 `/**/`。
5. 添加行号和文件名标识, 如 `# 1 "main.m"`，编译调试会用到。

+ `-fsyntax-only`: Run the preprocessor, parser and type checking stages. 防止编译器生成代码，只是语法级别的说明和修改。
+ `-S`: Run the previous stages as well as LLVM generation and optimization stages and target-specific code generation, producing an assembly file. 执行上面的 Preprocessing、Parsing and Semantic Analysis、Code Generation and Optimization 阶段并生成一个汇编文件。执行 `clang -S main.c -o main.s` 指令，`-o main.s` 表示输出 `main.s` 汇编文件，即使我们不指定输出，默认也会自动在当前文件夹下生成一个 `main.s` 文件。

&emsp;编译就是把上面得到的 `.i` 文件进行：词法分析、语法分析、静态分析、优化生成相应的汇编代码，得到 `.s` 文件。

1. 词法分析：源代码的字符序列分割成一个个 token（关键字、标识符、字面量、特殊符号），比如把标识符放到符号表。
2. 语法分析：生成抽象语法树 AST，此时运算符号的优先级确定了；有些符号具有多重含义也确定了，比如 `*` 是乘号还是对指针取内容；表达式不合法、括号不匹配等，都会报错。
3. 静态分析：分析类型声明和匹配问题。比如整型和字符串相加，肯定会报错。
4. 中间语言生成：CodeGen 根据 AST 自顶向下遍历逐步翻译成 LLVM IR，并且在编译期就可以确定的表达式进行优化，比如代码里 `t1=2+6`，可以优化 `t1=8`。
5. 目标代码生成与优化：根据中间语言生成依赖具体机器的汇编语言，并优化汇编语言。这个过程中，假如有变量且定义在同一个编译单元里，那给这个变量分配空间，确定变量的地址。假如变量或者函数不定义在这个编译单元，得到链接的时候，才能确定地址。
  
+ `-c`: Run all of the above, plus the assembler, generating a target ".o" object file. 运行上面所有阶段，直接生成一个 ".o" 目标文件。执行 `clang -c main.c -o main.o` 指令，`-o main.o` 表示输出 `main.o` 目标文件，即使我们不指定输出，默认也会自动在当前文件夹下生成一个 `main.o` 文件。

&emsp;汇编就是把上面得到的 `.s` 文件里的汇编指令一一翻译成机器指令。得到 `.o` 文件，也就是目标文件。

   
+ no stage selection option: If no stage selection option is specified, all stages above are run, and the linker is run to combine the results into an executable or shared library. 如果我们不指定编译阶段的选项，则直接运行上述所有阶段，并运行链接器（linker）以将编译结果链接到一个可执行文件或 shared library 中。执行 `clang main.c` 指令，默认可在当前文件夹下生成一个 `a.out` 文件，`a` 是默认名，我们也可用 `-o` 去指定一个我们想要的名字。

> &emsp;远古时代，一个程序只有一个源代码文件，导致程序的维护非常困难。现在程序都是分模块组成，比如一个 App，对应有多个源代码文件。每个源代码文件汇编成目标文件，根据上面流程 A 目标文件访问 B 目标文件的函数或者变量，是不知道地址的，链接就是要解决这个问题。链接过程主要包括地址和空间分配、符号决议和重定位。链接就是把目标文件（一个或多个）和需要的库（静态库/动态库）链接成可执行文件。

### Language Selection and Mode Options

+ `-x <language>`: Treat subsequent input files as having type language. 指定源文件是什么语言编写：C/C++/OpenCL/CUDA。
+ `-std=<standard>`: Specify the language standard to compile for. 指定要编译的语言标准。例如 C 采用的默认版本是：gnu17，C++ 采用的默认版本是：gnu++14，

&emsp;Supported values for the C language are:
...
   gnu99

      ISO C 1999 with GNU extensions
...
   gnu17

      ISO C 2017 with GNU extensions

&emsp;The default C language standard is gnu17, except on PS4, where it is gnu99. 默认的 C 语言标准是 gnu17，除了 PS4，它是 gnu99。（ISO C 2017 与 GNU 扩展）

&emsp;Supported values for the C++ language are:
...
   gnu++14

      ISO C++ 2014 with amendments and GNU extensions
...

&emsp;The default C++ language standard is gnu++14. 默认 C++ 语言标准是 gnu++14。（ISO C++ 2014 年修订和 GNU 扩展）

&emsp;Supported values for the OpenCL language are:
   cl1.0

      OpenCL 1.0
...

&emsp;The default OpenCL language standard is cl1.0. 默认的 OpenCL 语言标准是 cl1.0。

&emsp;Supported values for the CUDA language are:
   cuda

      NVIDIA CUDA(tm)

+ `-stdlib=<library>`: Specify the C++ standard library to use; supported options are libstdc++ and libc++. If not specified, platform default will be used. 指定要使用的 C++ 标准库; 支持的选项是 libstdc++ 和 libc++。如果未指定，将使用平台默认值。
+ `-rtlib=<library>`: Specify the compiler runtime library to use; supported options are libgcc and compiler-rt. If not specified, platform default will be used. 指定要使用的编译器运行时库; 支持的选项是 libgcc 和 compiler-rt。如果未指定，将使用平台默认值。
+ `-ansi`: Same as -std=c89.
+ `-ObjC, -ObjC++`: Treat source input files as Objective-C and Object-C++ inputs respectively. 将源输入文件分别视为 Objective-C 和 Object-C++ 输入。
+ `-trigraphs`: Enable trigraphs.
+ `-ffreestanding`: Indicate that the file should be compiled for a freestanding, not a hosted, environment. Note that it is assumed that a freestanding environment will additionally provide memcpy, memmove, memset and memcmp implementations, as these are needed for efficient codegen for many programs. 指示应针对独立环境（而非托管环境）编译文件。请注意，假设独立环境将额外提供 memcpy、memmove、memset 和 memcmp 实现，因为这些是许多程序的高效 codegen 所必需的。
+ `-fno-builtin`: Disable special handling and optimizations of builtin functions like strlen() and malloc(). 禁用内置函数（如 strlen() 和 malloc()）的特殊处理和优化。
+ `-fmath-errno`: Indicate that math functions should be treated as updating errno. 指示应将 math functions 视为 updating errno。
+ `-fpascal-strings`: Enable support for Pascal-style strings with "\pfoo". 启用对带有 "\pfoo" 的 Pascal-style 字符串的支持。
+ `-fms-extensions`: Enable support for Microsoft extensions. 启用对微软扩展的支持。
+ `-fmsc-version=`: Set `_MSC_VER`. Defaults to 1300 on Windows. Not set otherwise.
+ `-fborland-extensions`: Enable support for Borland extensions. 启用对 Borland 扩展的支持。
+ `-fwritable-strings`: Make all string literals default to writable.  This disables uniquing of strings and other optimizations. 将所有字符串文本默认为可写。 这将禁用字符串的统一和其他优化。
+ `-fblocks`: Enable the "Blocks" language feature. 启用 "Blocks" 语言功能。

### Target Selection Options

&emsp;Clang fully supports cross compilation as an inherent part of its design.  Depending on how your version of Clang is configured, it may have support for a number of cross compilers, or may only support a native target. Clang 完全支持交叉编译作为其设计的固有部分。根据 Clang 版本的配置方式，它可能支持许多交叉编译器，或者可能只支持本机。
+ `-arch <architecture>`: Specify the architecture to build for. 指定要为其构建的体系结构。
+ `-mmacosx-version-min=<version>`: When building for macOS, specify the minimum version supported by your application. 针对 macOS 进行构建时，请指定应用程序支持的最低版本。 
+ `-miphoneos-version-min`: When building for iPhone OS, specify the minimum version supported by your application. 针对 iOS 进行构建时，请指定应用程序支持的最低版本。
+ `--print-supported-cpus`: Print out a list of supported processors for the given target (specified through `--target=<architecture>` or `-arch <architecture>`). If no target is specified, the system default target will be used. 打印出给定 target 的受支持处理器列表。（通过 `--target=<architecture>` 或 `-arch<architecture>` 指定 target）如果未指定 target，则将使用系统默认 target。 
+ `-mcpu=?, -mtune=?`: Acts as an alias for `--print-supported-cpus`. 
+ `-march=<cpu>`: Specify that Clang should generate code for a specific processor family member and later.  For example, if you specify `-march=i486`, the compiler is allowed to generate instructions that are valid on i486 and later processors, but which may not exist on earlier ones. 指定 Clang 应为特定处理器系列成员及更高版本生成代码。例如，如果指定 `-march=i486`，则允许编译器生成在 i486 及更高版本的处理器上有效的指令，但在早期处理器上可能不存在的指令。

### Code Generation Options

> &emsp;`-O0, -O1, -O2, -O3, -Ofast, -Os, -Oz, -Og, -O, -O4`

&emsp;Specify which optimization level to use: 指定要使用的优化级别：

+ `-O0`: Means "no optimization": this level compiles the fastest and generates the most debuggable code. 表示 "no optimization" 无优化，此级别编译速度最快，生成可调试性最高的代码。
+ `-O1`: Somewhere between -O0 and -O2.
+ `-O2`: Moderate level of optimization which enables most optimizations. 中等级别的优化，可实现大多数优化。
+ `-O3`: Like -O2, except that it enables optimizations that take longer to perform or that may generate larger code (in an attempt to make the program run faster). 与 `-O2` 类似，只是它支持执行时间更长或可能生成更大代码的优化（试图使程序运行得更快）。
+ `-Ofast`: Enables all the optimizations from -O3 along with other aggressive optimizations that may violate strict compliance with language standards. 启用来自 `-O3` 的所有优化以及其他可能违反严格遵守语言标准的积极优化。
+ `-Os`: Like -O2 with extra optimizations to reduce code size. 像 `-O2` 一样，具有额外的优化以减小代码大小。
+ `-Oz`: Like -Os (and thus -O2), but reduces code size further. 像 `-Os`（and thus -O2）一样，但进一步减小了代码大小。
+ `-Og`: Like -O1. In future versions, this option might disable different optimizations in order to improve debuggability. 像 `-O1`。在将来的版本中，此选项可能会禁用不同的优化，以提高可调试性。
+ `-O`: Equivalent to -O1. 等效于 `-O1`。
+ `-O4`: and higher, Currently equivalent to `-O3`. 目前相当于 `-O3`。

> &emsp;`-g, -gline-tables-only, -gmodules`
       
&emsp;Control debug information output.  Note that Clang debug information works best at -O0.  When more than one option starting with -g is specified, the last one wins: 控制调试信息输出。 请注意，Clang 调试信息在 `-O0` 下效果最佳。 当指定了多个以 `-g` 开头的选项时，以最后一个选项优先： 

+ `-g`: Generate debug information. 生成 debug 信息。
+ `-gline-tables-only`: Generate only line table debug information. This allows for symbolicated backtraces with inlining information, but does not include any information about variables, their locations or types. 仅生成行表调试信息。这允许使用内联信息进行符号化的回溯跟踪，但不包括有关变量、其位置或类型的任何信息。
+ `-gmodules`: Generate debug information that contains external references to types defined in Clang modules or precompiled headers instead of emitting redundant debug type information into every object file.
    
    This option transparently switches the Clang module format to object file containers that hold the Clang module together with the debug information.  
    
    When compiling a program that uses Clang modules or precompiled headers, this option produces complete debug information with faster compile times and much smaller object files. 
    
    This option should not be used when building static libraries for distribution to other machines because the debug info will contain references to the module cache on the machine the object files in the library were built on.
     
+ `-fstandalone-debug -fno-standalone-debug`: Clang supports a number of optimizations to reduce the size of debug information in the binary. 

    They work based on the assumption that the debug type information can be spread out over multiple compilation units.
    
    For instance, Clang will not emit type definitions for types that are not needed by a module and could be replaced with a forward declaration.
    
    Further, Clang will only emit type info for a dynamic C++ class in the module that contains the vtable for the class.
    
    The `-fstandalone-debug` option turns off these optimizations. 
    
    This is useful when working with 3rd-party libraries that don't come with debug information.
    
    This is the default on Darwin.
    
    Note that Clang will never emit type information for types that are not referenced at all by the program.

+ `-feliminate-unused-debug-types`: By default, Clang does not emit type information for types that are defined but not used in a program.

    To retain the debug info for these unused types, the negation `-fno-eliminate-unused-debug-types` can be used.

+ `-fexceptions`: Enable generation of unwind information. This allows exceptions to be thrown through Clang compiled stack frames.  This is on by default in x86-64.

+ `-ftrapv`: Generate code to catch integer overflow errors.  Signed integer overflow is undefined in C. With this flag, extra code is generated to detect this and abort when it happens.
+ `-fvisibility`: This flag sets the default visibility level.
+ `-fcommon, -fno-common`: This flag specifies that variables without initializers get common linkage.  It can be disabled with `-fno-common`.
+ `-ftls-model=<model>`: Set the default thread-local storage (TLS) model to use for thread-local variables. Valid values are: "global-dynamic", "local-dynamic", "initial-exec" and "local-exec". The default is "global-dynamic". The default model can be overridden with the tls_model attribute. The compiler will try to choose a more efficient model if possible.
+ `-flto, -flto=full, -flto=thin, -emit-llvm`: Generate output files in LLVM formats, suitable for link time optimization.

    When used with `-S` this generates LLVM intermediate language assembly files, otherwise this generates LLVM bitcode format object files (which may be passed to the linker depending on the stage selection options).
    
    The default for `-flto` is "full", in which the LLVM bitcode is suitable for monolithic Link Time Optimization (LTO), where the linker merges all such modules into a single combined module for optimization. With "thin", ThinLTO compilation is invoked instead.
    
    NOTE: On Darwin, when using `-flto` along with `-g` and compiling and linking in separate steps, you also need to pass `-Wl, -object_path_lto, <lto-filename>.o` at the linking step to instruct the ld64 linker not to delete the temporary object file generated during Link Time Optimization (this flag is automatically passed to the linker by Clang if compilation and linking are done in a single step). This allows debugging the executable as well as generating the `.dSYM` bundle using `dsymutil(1)`.

### Driver Options

+ `-###`: Print (but do not run) the commands to run for this compilation. 打印此次编译活动所执行的命令以及选项，但实际并不进行编译运行。
+ `--help`: Display available options. 显示可用选项。
+ `-Qunused-arguments`: Do not emit any warnings for unused driver arguments. 不要对未使用的 Clang 驱动程序参数发出任何警告。

+ `-Wa,<args>`: Pass the comma separated arguments in args to the assembler. 将 `args` 中逗号分隔的参数传递给 assembler（汇编器）。  
+ `-Wl,<args>`: Pass the comma separated arguments in args to the linker. 将 `args` 中以逗号分隔的参数传递给 linker（链接器）。
+ `-Wp,<args>`: Pass the comma separated arguments in args to the preprocessor. 将 `args` 中逗号分隔的参数传递给 preprocessor（预处理器）。
 
+ `-fexceptions`: Enable generation of unwind information. This allows exceptions to be thrown through Clang compiled stack frames.  This is on by default in x86-64.
+ `-ftrapv`: Generate code to catch integer overflow errors.  Signed integer overflow is undefined in C. With this flag, extra code is generated to detect this and abort when it happens.
+ `-fvisibility`: This flag sets the default visibility level.
+ `-fcommon, -fno-common`: This flag specifies that variables without initializers get common linkage.  It can be disabled with `-fno-common`.
+ `-Xanalyzer <arg>`: Pass arg to the static analyzer. 将 `arg` 传递给静态分析器。
+ `-Xassembler <arg>`: Pass arg to the assembler. 将 `arg` 传递给汇编器。
+ `-Xlinker <arg>`: Pass arg to the linker. 将 `arg` 传递给链接器。
+ `-Xpreprocessor <arg>`: Pass arg to the preprocessor. 将 `arg` 传递给预处理器。
+ `-o <file>`: Write output to file. 将输出写入文件，例如：main.i、main.s、main.o、main.out...
+ `-print-file-name=<file>`: Print the full library path of file.
+ `-print-libgcc-file-name`: Print the library path for the currently used compiler runtime library (`libgcc.a` or `libclang_rt.builtins.*.a`).

```c++
➜  compile_c clang -print-libgcc-file-name libgcc.a
/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/13.0.0/lib/darwin21.4.0/libclang_rt.builtins-x86_64.a
```

+ `-print-prog-name=<name>`: Print the full program path of name. 打印 `name` 的完整程序路径。

```c++
➜  compile_c clang -print-prog-name=clang
/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang
➜  compile_c clang -print-prog-name=nm
/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/nm
➜  compile_c clang -print-prog-name=ls
/bin/ls
➜  compile_c clang -print-prog-name=cd
/usr/bin/cd
➜  compile_c clang -print-prog-name=dis
dis
➜  compile_c clang -print-prog-name=file
/usr/bin/file
```

+ `-print-search-dirs`: Print the paths used for finding libraries and programs.
+ `-save-temps`: Save intermediate compilation results. 保存中间编译结果。
+ `-save-stats, -save-stats=cwd, -save-stats=obj`: Save internal code generation (LLVM) statistics to a file in the current directory (`-save-stats/-save-stats=cwd`) or the directory of the output file (`-save-state=obj`). 将内部代码生成（LLVM）统计信息保存到当前目录中的文件中，例如会生成一个 main.stats 文件。

```c++
{
    "file-search.NumDirCacheMisses": 1,
    "file-search.NumDirLookups": 1,
    "file-search.NumFileCacheMisses": 1,
    "file-search.NumFileLookups": 1
}
```

+ `-integrated-as, -no-integrated-as`: Used to enable and disable, respectively, the use of the integrated assembler. Whether the integrated assembler is on by default is target dependent. 用于分别启用和禁用集成汇编器的使用。默认情况下，集成汇编器是否处于打开状态取决于 target。
+ `-time`: Time individual commands. 对单个命令进行计时。
+ `-ftime-report`: Print timing summary of each stage of compilation. 打印编译的每个阶段的计时摘要。
+ `-v`: Show commands to run and use verbose output. 显示要运行的命令并使用详细输出。

### Diagnostics Options

+ `-fshow-column, -fshow-source-location, -fcaret-diagnostics, -fdiagnostics-fixit-info, -fdiagnostics-parseable-fixits, -fdiagnostics-print-source-range-info, -fprint-source-range-info, -fdiagnostics-show-option, -fmessage-length`: These options control how Clang prints out information about diagnostics (errors and warnings). Please see the Clang User's Manual for more information. 这些选项控制 Clang 如何打印有关诊断的信息（错误和警告）。有关详细信息，请参阅 Clang 用户手册。

### Preprocessor Options

+ `-D<macroname>=<value>`: Adds an implicit #define into the predefines buffer which is read before the source file is preprocessed. 将隐式 `#define` 添加到预定义缓冲区中，该缓冲区在预处理源文件之前读取。
+ `-U<macroname>`: Adds an implicit #undef into the predefines buffer which is read before the source file is preprocessed. 将隐式 `#undef` 添加到预定义缓冲区中，该缓冲区在预处理源文件之前读取。
+ `-include <filename>`: Adds an implicit #include into the predefines buffer which is read before the source file is preprocessed. 将隐式 `#include` 添加到预定义缓冲区中，该缓冲区在预处理源文件之前读取。
+ `-I<directory>`: Add the specified directory to the search path for include files. 将指定的目录添加到包含文件的搜索路径中。
+ `-F<directory>`: Add the specified directory to the search path for framework include files. 将指定的目录添加到 framework 包含文件的搜索路径中。
+ `-nostdinc`: Do not search the standard system directories or compiler builtin directories for include files. 不要在标准系统目录或编译器内置目录中搜索包含文件。
+ `-nostdlibinc`: Do not search the standard system directories for include files, but do search compiler builtin include directories. 不要在标准系统目录中搜索 include 文件，但要搜索编译器内置包含目录。
+ `-nobuiltininc`: Do not search clang's builtin directory for include files. 不要在 clang 的内置目录中搜索 include 文件。

### ENVIRONMENT

+ `TMPDIR, TEMP, TMP`: These environment variables are checked, in order, for the location to write temporary files used during the compilation process. 将按顺序检查这些环境变量，以确定写入编译过程中使用的临时文件的位置。
+ `CPATH`: If this environment variable is present, it is treated as a delimited list of paths to be added to the default system include path list. The delimiter is the platform dependent delimiter, as used in the `PATH` environment variable. Empty components in the environment variable are ignored. 如果此环境变量存在，则将其视为要添加到默认系统包含路径列表中的分隔路径列表。分隔符是与平台相关的分隔符，如 PATH 环境变量中使用的分隔符。环境变量中的空组件将被忽略。
+ `C_INCLUDE_PATH, OBJC_INCLUDE_PATH, CPLUS_INCLUDE_PATH, OBJCPLUS_INCLUDE_PATH`: These environment variables specify additional paths, as for `CPATH`, which are only used when processing the appropriate language. 这些环境变量指定其他路径，如 `CPATH`，这些路径仅在处理适当的语言时使用。
+ `MACOSX_DEPLOYMENT_TARGET`: If `-mmacosx-version-min` is unspecified, the default deployment target is read from this environment variable. This option only affects Darwin targets.

&emsp;Apple Clang 1300.0.29.30.

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
+ [clang常用语法介绍](https://www.jianshu.com/p/96058bf1ecc2)
