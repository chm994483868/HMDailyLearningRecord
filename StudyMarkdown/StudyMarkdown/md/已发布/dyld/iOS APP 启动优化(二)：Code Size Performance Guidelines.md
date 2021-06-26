# iOS APP 启动优化(二)：Code Size Performance Guidelines

> &emsp;**Retired Document**
> &emsp;**Important:** This document may not represent best practices for current development. Links to downloads and other resources may no longer be valid. （虽然针对当前开发可能已经不是最佳实践，但是依然具有其参考价值）

## Introduction to Code Size Performance Guidelines

&emsp;在程序性能方面，内存使用率和效率之间有明显的相关性。应用程序占用的内存越多，效率就越低。更多的内存意味着更多的内存分配、更多的代码和更多潜在的分页活动的可能性。

&emsp;本文档的主题的重点是减少可执行代码。减少代码占用不仅仅是在编译器中启用代码优化的问题，尽管这确实有帮助。你还可以通过组织代码来减少代码占用空间，以便在任何给定时间仅将最少数量的必需函数存储在内存中。你可以通过分析代码来实现此优化。

&emsp;减少应用程序分配的内存量对于减少内存占用也很重要；Performance Documentation 中的 [Memory Usage Performance Guidelines](https://developer.apple.com/library/archive/documentation/Performance/Conceptual/ManagingMemory/ManagingMemory.html#//apple_ref/doc/uid/10000160i) 中包含了这些信息。

### Organization of This Document

&emsp;本编程主题（文档）包含以下内容：

+ Overview of the Mach-O Executable Format 介绍如何使用 Mach-O 可执行格式的组织来提高代码效率。
+ Managing Code Size 描述可用于减小可执行文件总体大小的几个编译器选项。
+ Improving Locality of Reference 描述如何配置和重组代码以缩短代码段（code segments）的加载时间。
+ Reducing Shared Memory Pages 描述减小 \_\_DATA 段大小的方法。
+ Minimizing Your Exported Symbols 显示如何识别和消除代码中不必要的符号信息（symbol information）。

## Overview of the Mach-O Executable Format

&emsp;Mach-O 是 OS X 中二进制文件的 native 可执行格式，是 shipping code 的首选格式。可执行格式决定二进制文件中的代码（code）和数据（data）读入内存的顺序。代码和数据的顺序会影响内存使用和分页活动（paging activity），因此会直接影响程序的性能。

&emsp;Mach-O 二进制文件被组织成段（segments）。每个段包含一个或多个 sections。不同类型的代码或数据进入每个 section。Segments 总是从页（page）边界开始，但 sections 不一定是页对齐的（page-aligned）。Segment 的大小由它包含的所有 sections 中的字节数来度量，并向上舍入到下一个虚拟内存页的边界（virtual memory page boundary）。因此，一个 segment 总是 4096 字节或 4 KB 的倍数，其中 4096 字节是最小大小。

&emsp;Mach-O 可执行文件的 segments 和 sections 根据其预期用途命名。Segment 名称的约定是使用前有双下划线的所有大写字母组成（例如：\_\_TEXT）；Section 名称的约定是使用前有双下划线的所有小写字母组成（例如：\_\_text）。

&emsp;Mach-O 可执行文件中有几个可能的 segments，但是只有两个与性能有关：\_\_TEXT segment 和 \_\_DATA segment。

### The \_\_TEXT Segment: Read Only

&emsp;\_\_TEXT segment 是一个只读区域，包含可执行代码和常量数据。按照惯例，编译器工具创建的每个可执行文件至少有一个只读 \_\_TEXT segment。由于该 segment 是只读的，内核可以将可执行文件中的 \_\_TEXT segment 直接映射（map）到内存中一次。当 segment 映射到内存中时，它可以在对其内容感兴趣的所有进程之间共享。（这主要是 frameworks 和 shared libraries 的情况。）只读属性还意味着组成 \_\_TEXT segment 的页不必保存到备份存储。如果内核需要释放物理内存，它可以丢弃一个或多个 \_\_TEXT 页，并在需要时从磁盘重新读取它们。

&emsp;表 1 列出了可以出现在 \_\_TEXT segment 中的一些更重要的 sections。有关 segments 的完整列表，请参阅 Mach-O Runtime Architecture。

&emsp;**Table 1**  Major sections in the \_\_TEXT segment

| Section | Description |
| --- | --- |
| \_\_text | The compiled machine code for the executable（可执行文件的已编译机器码） |
| \_\_const | The general constant data for the executable（可执行文件的常规常量数据） |
| \_\_cstring | Literal string constants (quoted strings in source code) 字面量字符串常量（源代码中带引号的字符串） |
| \_\_picsymbol_stub | Position-independent code stub routines used by the dynamic linker (dyld) 动态链接器（dyld）使用的与位置无关的 code stub routines |

### The \_\_DATA Segment: Read/Write

&emsp;\_\_DATA segment 包含可执行文件的非常量数据。此 segment 既可读又可写。因为它是可写的，所以 framework 或其他 shared library 的 \_\_DATA segment 在逻辑上是为每个与 library 链接的进程复制的。当内存页可读写时，内核将它们标记为 copy-on-write。此技术延迟复制页（page），直到共享该页的某个进程尝试写入该页。当发生这种情况时，内核会为该进程创建一个页（page）的私有副本。

&emsp;\_\_DATA segment 有许多 sections，其中一些 sections 仅由动态链接器（dynamic linker）使用。表 2 列出了 \_\_DATA segment 中可能出现的一些更重要的 sections。有关 segments 的完整列表，请参阅 Mach-O Runtime Architecture。

&emsp;**Table 2** Major sections of the \_\_DATA segment

| Section | Description |
| --- | --- |
| \_\_data | Initialized global variables (for example int a = 1; or static int a = 1;). 初始化的全局变量 |
| \_\_const | Constant data needing relocation (for example, char * const p = "foo";). 需要重定位的常量数据 |
| \_\_bss | Uninitialized static variables (for example, static int a;). 未初始化的静态变量 |
| \_\_common | Uninitialized external globals (for example, int a; outside function blocks). 未初始化的外部全局变量 |
| \_\_dyld | A placeholder section, used by the dynamic linker. 动态链接器使用的占位符部分 |
| \_\_la_symbol_ptr | “Lazy” symbol pointers. Symbol pointers for each undefined function called by the executable. “Lazy” 符号指针。可执行文件调用的每个未定义函数的符号指针 |
| \_\_nl_symbol_ptr | “Non lazy” symbol pointers. Symbol pointers for each undefined data symbol referenced by the executable. “Non lazy” 符号指针。可执行文件引用的每个未定义数据符号的符号指针 |

### Mach-O Performance Implications

&emsp;Mach-O 可执行文件的 \_\_TEXT 和 \_\_DATA 的组成对性能有直接影响。优化这些 segments 的技术和目标是不同的。然而，他们有一个共同的目标：提高内存的使用效率。

&emsp;大多数典型的 Mach-O 文件都由可执行代码组成，这些代码占据了 \_\_TEXT 中的 \_\_text section。如上面 The \_\_TEXT Segment: Read Only 中所述，\_\_TEXT segment 是只读的，直接映射到可执行文件。因此，如果内核需要回收某些 \_\_text 页所占用的物理内存，它不必将这些页保存到备份存储区，并在以后对它们进行分页。它只需要释放内存，当以后引用代码时，从磁盘读回它。虽然这比交换成本更低，因为它涉及一个磁盘访问而不是两个磁盘访问，所以它仍然是昂贵的，特别是在必须从磁盘重新创建许多页的情况下。

&emsp;改善这种情况的一种方法是通过过程重新排序（procedure reordering）来改善代码的引用位置，如 [Improving Locality of Reference](https://developer.apple.com/library/archive/documentation/Performance/Conceptual/CodeFootprint/Articles/ImprovingLocality.html#//apple_ref/doc/uid/20001862-CJBJFIDD) 中所述。这项技术根据方法和函数的执行顺序、调用频率以及调用频率将它们组合在一起。如果 \_\_text section 组中的页以这种方式正常工作，则不太可能释放它们并多次读回。例如，如果将所有启动时初始化函数放在一个或两个页上，则不必在这些初始化发生后重新创建页。

&emsp;与 \_\_TEXT segment 不同，\_\_DATA segment 可以写入，因此 \_\_DATA segment 中的页不可共享。frameworks 中的非常量全局变量（non-constant global variables）可能会对性能产生影响，因为与 framework 链接的每个进程（process）都有自己的变量副本。这个问题的主要解决方案是将尽可能多的非常量全局变量移到 \_\_TEXT 中的 \_\_const section，方法是声明它们为 const。[Reducing Shared Memory Pages](https://developer.apple.com/library/archive/documentation/Performance/Conceptual/CodeFootprint/Articles/SharedPages.html#//apple_ref/doc/uid/20001863-CJBJFIDD) 描述了这一点和相关的技术。对于应用程序来说，这通常不是问题，因为应用程序中的 \_\_DATA section 不与其他应用程序共享。

&emsp;编译器将不同类型的非常量全局数据（nonconstant global data）存储在 \_\_DATA segment 的不同 sections 中。这些类型的数据是未初始化的静态数据和符号（uninitialized static data and symbols），它们与未声明为 extern 的 ANSI C “tentative definition” 概念一致。未初始化的静态数据（Uninitialized static data）位于 \_\_DATA segment 的 \_\_bss section。临时定义符号（tentative-definition symbols）位于 \_\_DATA segment 的 \_\_common section。

&emsp;ANSI C 和 C++ 标准规定系统必须将未初始化静态变量（uninitialized static variables）设置为零。（其他类型的未初始化数据保持未初始化状态）由于未初始化的静态变量和临时定义符号（tentative-definition symbols）存储在分开的 sections 中，系统需要对它们进行不同的处理。但是，当变量位于不同的 sections 时，它们更有可能最终出现在不同的内存页上，因此可以分别进行换入和换出操作，从而使你的代码运行速度更慢。这些问题的解决方案（如 [Reducing Shared Memory Pages](https://developer.apple.com/library/archive/documentation/Performance/Conceptual/CodeFootprint/Articles/SharedPages.html#//apple_ref/doc/uid/20001863-CJBJFIDD) 中所述）是将非常量全局数据（non-constant global data）合并到 \_\_DATA segment 的一个 section 中。

## Managing Code Size

&emsp;GCC 编译器支持各种优化代码的选项。这些技术中的大多数都会根据你的需要生成更少的代码或更快的代码（less code or faster code）。当你准备发布软件时，你应该尝试这些技术，看看哪些技术对你的代码最有好处。

### Compiler-Level Optimizations

&emsp;当你的项目代码稳定下来时，你应该开始试验用于优化代码的基本 GCC 选项。GCC 编译器支持优化选项，允许你选择是使用较小的二进制大小（smaller binary size）、更快的代码（faster code）还是更快的构建时间（faster build times）。

&emsp;对于新项目，Xcode 会自动禁用开发构建样式的优化（optimizations for the development build style），并选择 “fastest, smallest” 部署构建样式（“fastest, smallest” option for the deployment build style）的选项。任何类型的代码优化都会导致生成时间变慢，因为优化过程涉及到额外的工作。如果你的代码正在更改（如在开发周期中所做的那样），则你不希望启用优化。在开发周期即将结束时，部署构建样式（deployment build style）可以为你指示最终产品的大小。

&emsp;Table 1 列出了 Xcode 中可用的优化级别（optimization levels）。当你选择其中一个选项时，Xcode 会将给定组或文件的相应标志传递给 GCC 编译器（ Xcode passes the appropriate flags to the GCC compiler for the given group or files）。这些选项可在 target-level 或作为 build style 的一部分可见。有关项目的构建设置的信息，请参阅 Xcode Help。

&emsp;Table 1  GCC compiler optimization options
| Xcode Setting | Description |
| --- | --- |
| None | 编译器不会尝试优化代码。当你专注于解决逻辑错误并且需要快速编译时，请在开发过程中使用此选项。 Do not use this option for shipping your executable。 |
| Fast | 编译器执行简单的优化以提高代码性能，同时最小化对编译时间的影响。此选项在编译期间也会使用更多内存。 |
| Faster | 执行几乎所有不需要 space-time trade-off 的受支持优化。使用此选项时编译器不执行循环展开（loop unrolling）或函数内联（function inlining）。此选项增加编译时间和生成代码的性能。 |
| Fastest | 执行所有优化以提高生成代码的速度。当编译器执行积极的函数内联时，此选项可以增加生成代码的大小。<br>通常不建议使用此选项。有关详细信息，请参见避免过多的函数内联（Avoid Excessive Function Inlining）。 |
| Fastest, smallest | 执行通常不会增加代码大小的所有优化。这是传送代码（shipping code）的首选选项，因为它使可执行文件的内存占用更小。 |

&emsp;与任何性能增强一样，不要假设哪个选项会给你带来最佳效果。你应该始终衡量你尝试的每个优化的结果。例如，“Fastest” 选项可能会为特定模块生成速度极快的代码，但这样做通常会以牺牲可执行文件的大小为代价。如果代码需要在运行时从磁盘中传入，则你从代码生成中获得的任何速度优势都很容易丢失。（如果代码需要在运行时从磁盘分页，那么从代码生成中获得的任何速度优势都很容易丢失。）

### Additional Optimizations

&emsp;除了代码级（code-level）优化之外，你还可以使用一些附加技术在 module level 组织代码。以下各节介绍这些技术。

#### Dead Strip Your Code

&emsp;对于静态链接（statically-linked）的可执行文件，dead-code stripping 是从可执行文件中删除未引用代码的过程。dead-stripping 背后的思想是，如果代码未被引用，就不能使用它，因此在可执行文件中就不需要它。删除 dead code 可以减少可执行文件的大小，并有助于减少分页（reduce paging）。

&emsp;从 Xcode Tools 版本 1.5 开始，静态链接器（static linker）（ld）支持可执行文件的 dead stripping。你可以直接从 Xcode 或通过向静态链接器（static linker）传递适当的命令行选项来启用此功能。

&emsp;要在 Xcode 中启用 dead-code stripping，请执行以下操作：

1. 选择你的 target。
2. 打开 Inspector 或 Get Info 窗口并选择 Build 选项卡。
3. 在链接设置（Linking settings）中，启用 Dead Code Stripping 选项。

&emsp;TARGETS -> Build Settings -> 搜索 Linking -> Dead Code Stripping 设置为 YES/NO（默认是 YES）。

4. 在 Code Generation settings 中，将 Level of Debug Symbols 选项设置为 All Symbols。
&emsp;TARGETS -> Build Settings -> 搜索 All Symbols -> Strip Style 设置为 All Symbols/Non-Global Symbols/Debugging Symbols（默认是 All Symbols）。

&emsp;要从命令行启用 dead-code stripping，请将 -dead_strip 选项传递给 ld。还应该将 -gfull 选项传递给 GCC，以便为代码生成一组完整的调试符号（debugging symbols）。链接器（linker）使用这个额外的调试信息对可执行文件进行 dead strip。

> &emsp;Note: 建议使用 “All Symbols” 或 -gfull 选项，即使你不打算 dead strip。尽管该选项生成较大的中间文件（intermediate files），但通常会生成较小的可执行文件，因为链接器（linker）能够更有效地删除重复的符号信息。

&emsp;如果不想删除任何未使用的函数，至少应该将它们隔离在 \_\_TEXT segment 的一个单独部分中。将未使用的函数移到 common section 可以改进代码引用的局部性，并降低它们被加载到内存中的可能性。有关如何在 common section 中对函数进行分组的详细信息，请参见 Improving Locality of Reference。

#### Strip Symbol Information

&emsp;调试符号（debugging symbols）和动态绑定信息（dynamic-binding information）可能会占用大量空间，并且占可执行文件大小的很大一部分。在 shipping 代码之前，应该去掉所有不需要的符号。

&emsp;要从可执行文件中删除调试符号（debugging symbols），请将 Xcode build-style 设置更改为 “Deployment”，然后 rebuild 可执行文件。如果你愿意，还可以按目标（target）生成调试符号。有关构建样式（build styles）和目标设置（target settings）的详细信息，请参阅Xcode Help。

&emsp;要从可执行文件中手动删除动态绑定符号（dynamic-binding symbols），请使用 strip tool。此工具删除动态链接器（dynamic linker）在运行时通常用于绑定外部符号的符号信息。删除不希望动态绑定的函数的符号会减少可执行文件的大小，并减少动态链接器必须绑定的符号数。通常，你会使用此命令而不使用任何选项来删除非外部符号（non-external symbols），如以下示例所示：

```c++
% cd ~/MyApp/MyApp.app/Contents/MacOS
% strip MyApp
```

&emsp;此命令相当于使用 -u 和 -r 选项运行 strip。它删除所有标记为非外部的符号，但不删除标记为外部的符号。

&emsp;手动剥离动态绑定符号的另一种方法是使用导出文件来限制在构建时导出的符号。导出文件标识运行时从可执行文件中可用的特定符号。有关创建导出文件的详细信息，请参见 Minimizing Your Exported Symbols。

&emsp;An alternative to stripping out dynamic-binding symbols manually is to use an exports file to limit the symbols exported at build time. An exports file identifies the specific symbols available at runtime from your executable. For more information on creating an exports file, see Minimizing Your Exported Symbols.

#### Eliminate C++ Exception Handling Overhead

&emsp;当抛出异常时，C++ 运行库必须能够将堆栈展开回第一匹配 catch 块的点。为此，GCC  编译器为每个可能引发异常的函数生成堆栈展开信息。此展开信息存储在可执行文件中，并描述堆栈上的对象。这些信息使得在抛出异常时调用这些对象的析构函数来清除它们成为可能。

&emsp;即使你的代码不抛出异常，GCC 编译器仍然会为 C++ 代码生成默认的堆栈展开信息。如果你广泛使用异常，这个额外的代码会显著增加可执行文件的大小。

##### Disabling Exceptions

&emsp;通过禁用目标的 “Enable C++ Exceptions” 构建选项，可以禁用 XCoad 中的异常处理。从命令行，将 -fno-exceptions 选项传递给编译器。此选项删除函数的堆栈展开信息。但是，仍然必须从代码中删除任何 try、catch 和 throw 语句。

##### Selectively Disabling Exceptions

&emsp;如果代码在某些地方使用异常，而不是在任何地方使用异常，则可以通过向方法声明中添加空的异常规范来显式标识不需要展开信息的方法。例如，在下面的代码中，编译器必须为 my_function 生成堆栈展开信息，理由是 my_other_function 或它调用的函数可能引发异常。

```c++
extern int my_other_function (int a, int b);
int my_function (int a, int b)
{
   return my_other_function (a, b);
}
```

&emsp;但是，如果你知道 my_other_function 函数不能抛出异常，你可以通过在函数声明中包含空异常规范（throw ()）来向编译器发出信号。因此，你可以如下声明前面的函数：

```c++
extern int foo (int a, int b) throw ();
int my_function (int a, int b) throw ()
{
   return foo (a, b);
}
```

##### Minimizing Exception Use（尽量减少异常捕获的使用）

&emsp;在编写代码时，请仔细考虑异常的使用。异常应该用来表示异常情况，也就是说，它们应该用来报告你没有预料到的问题。如果从文件读取时出现文件结束错误（end-of-file error），则不希望抛出异常，因为这是一种已知类型的错误，可以轻松处理。如果你试图读取一个已知已打开的文件，并且被告知该文件 ID 无效，那么你可能希望抛出一个异常。

#### Avoid Excessive Function Inlining（避免内联函数使用过度）

&emsp;尽管内联函数（inline functions）在某些情况下可以提高速度，但如果使用过度，它们也会降低 OS X 上的性能。内联函数消除了调用函数的开销，但是通过用代码的副本（copy of the code）替换每个函数调用来实现。如果内联函数经常被调用，那么这些额外的代码会很快累积起来，使可执行文件膨胀并导致分页问题。

&emsp;如果使用得当，内联函数可以节省时间，并且对代码占用的影响最小。请记住，内联函数的代码通常应该非常短，很少调用。如果在函数中执行代码所需的时间少于调用函数所需的时间，则函数是内联的最佳候选函数。一般来说，这意味着一个内联函数的代码应该不超过几行。你还应该确保从代码中尽可能少的地方调用函数。即使是一个很短的函数，如果在几十个或几百个地方内联使用，也会导致过度膨胀。

&emsp;另外，你应该知道，一般应该避免使用 GCC 的 “Fastest” 优化级别。在这个优化级别上，编译器会积极地尝试创建内联函数，即使对于没有标记为内联的函数也是如此。不幸的是，这样做会大大增加可执行文件的大小，并由于分页而导致更糟糕的性能问题。

#### Build Frameworks as a Single Module

&emsp;大多数共享库（shared libraries）不需要 Mach-O 运行时的模块特性（module features）。此外，跨模块调用产生的开销与跨库调用相同。因此，你应该将项目的所有中间对象文件链接到一个模块中。

&emsp;要合并对象文件，必须在链接阶段（link phase）将 -r 选项传递给 ld。如果你使用 Xcode 来构建代码，那么默认情况下这是为你完成的。

## Improving Locality of Reference

&emsp;对应用程序性能的一个重要改进是减少应用程序在任何给定时间使用的虚拟内存页（virtual memory pages）的数量。这组页（set of pages）称为工作集（working set），由应用程序代码（application code）和运行时数据（runtime data）组成。减少内存中数据的数量（in-memory data）是算法的一个功能（is a function of your algorithms），但是减少内存中代码的数量（in-memory code）可以通过一个称为分散加载（scatter loading）的处理来实现。这种技术也被称为改进代码引用的局部性（improving the locality of reference）。

&emsp;通常，方法和函数的编译代码是由源文件以生成的二进制文件组织的。（通常，编译的方法和函数代码由生成的二进制文件中的源文件组织。）分散加载（scatter loading）会更改此组织，而是将相关方法和函数分组在一起，而与这些方法和函数的原始位置无关。这个过程允许内核将活动应用程序（active application）最常引用的可执行页保存在尽可能小的内存空间中。这不仅减少了应用程序的占用空间，还降低了这些页面被调出（大概是指内存紧张时被回收）的可能性。

> &emsp;Important:通常应该等到开发周期的很晚才分散加载应用程序。在开发过程中，代码往往会四处移动，这会使以前的评测结果无效。

### Profiling Code With gprof（通过 gprof 分析代码）

&emsp;根据运行时收集的分析数据
，gprof 生成程序的 execution profile。被调用例程的效果包含在每个调用方的 profile 中。profile 数据取自 call graph profile file(gmon.out 默认情况下），它是由程序编译创建的，并与 -pg 选项链接。可执行文件中的符号表（symbol table）与 call graph profile file 相关联。如果指定了多个 profile file，gprof 输出将显示给定 profile files 中 profile 信息的总和。

&emsp;gprof 工具有很多用途，包括：

+ Sampler application 工作不好的情况，例如 command-line tools 或在短时间后退出的应用程序
+ 在这种情况下，你需要一个包含给定程序中可能调用的所有代码的 call graph，而不是周期性地对调用进行采样
+ 需要更改代码的 link order 以优化代码局部性的情况

#### Generating Profiling Data

&emsp;在分析应用程序之前，必须将项目设置为 generate profiling information。要为 Xcode 项目生成 profiling information，必须修改 target 或 build-style settings，以包含 “Generate profiling code” 选项。（位置在 TARGETS -> Build Settings -> Build Options -> Generate profiling code(YES/NO)）(有关启用 target 和 build-style settings 的信息，请参见 Xcode Help）

&emsp;程序内的 profiling code 生成一个名为 gmon.out 且包含 profiling information 的文件。
(通常，此文件放在当前工作目录中。）若要分析此文件中的数据，请在调用 gprof 之前将其复制到包含可执行文件的目录中，或只是指定路径到 gmon.out 当你运行 gprof 时。

&emsp;除了分析你自己的代码之外，你可以通过与 Carbon 和 Cocoa frameworks 这些框架的 profile versions 进行链接，找出它们花费了多少时间。
为此，请将 DYLD_IMAGE_SUFFIX 设置添加到 target 或 build style，并将其值设置为 \_profile。dynamic linker 将此后缀与 framework 名称相结合，以针对 framework 的 profile version 进行链接。要确定哪些 frameworks 支持 profiling （概要分析），请查看 frameworks 本身。例如，Carbon library 附带了 profile 和 debug 版本。

&emsp;Note: libraries 的 profile 和 debug 版本是作为 developer tools package 的一部分安装的，在用户系统上可能不可用。确保你的 shipping executable 没有链接到这些库之一。

#### Generating Order Files

&emsp;order file 包含一个有序的 lines 序列，每一 line 由一个 source file name 和一个 symbol name 组成，用冒号分隔，没有其他空格。每一 line 表示要放置在可执行文件 section 中的 block。如果手动修改 order file，则必须完全遵循此格式，以便 linker 可以处理该 order file。如果 object file 的 name:symbol name pair 并不完全是 linker 看到的名称，它会尽最大努力将名称与被 linked 的 objects 匹配起来。

&emsp;procedure 重新排序的 order file 中的 lines 由 an object filename 和 procedure name（function、method 或其他 symbol）组成。order file 中列出 procedures（程序） 的顺序表示它们链接到可执行文件的 \_\_text section 的顺序。

&emsp;要从使用 program 生成的 profiling data 创建 order file，请使用 -S 选项运行 gprof（请参阅 gprof 的手册页）。例如：

```c++
gprof -S MyApp.profile/MyApp gmon.out
```

&emsp;-S 选项生成四个相互排斥的 order files：

| gmon.order | 基于 profiling call graph 的 “closest is best” 分析进行排序。经常互相 call 的 call 放在一起。 |
| callf.order | Routines 按对每个 Routine 的调用次数排序，首先最大调用次数。 |
| callo.order | 按照调用顺序对 Routines 进行排序 |
| time.order | 按花费的 CPU 时间对 Routines 进行排序，花最多时间的 Routine 放在第一。 |

&emsp;你应该尝试使用这些 files 中的每一个，看看哪些 file 提供了最大的性能改进（如果有的话）。
请参阅 Using pagestuff to Examine Pages on Disk，以便讨论如何衡量 ordering 结果。

&emsp;这些 order files 只包含 profiling 期间使用的那些 procedures。linker 跟踪缺失的 procedures（程序），并在 order files 中列出的程序之后以默认顺序将它们链接起来。
仅当项目目录（project directory）包含由 linker 的 -whatsloaded 选项生成的文件时，才会在 order file 中生成 library functions 的 static names（静态名称）；有关详细信息，请参见 Creating a Default Order File。

&emsp;gprof-S 选项不适用于已使用 order file 链接的可执行文件。

#### Fixing Up Your Order Files

&emsp;生成 order files 后，你应该仔细检查它们并确保它们是正确的。在许多情况下，你需要手动编辑 order files，包括：

+ 可执行文件包含汇编语言文件（assembly-language files）。
+ 你 profiled（分析）了一个 stripped 的可执行文件。
+ 你的可执行文件包含未使用 -g 选项编译的文件。
+ 你的项目定义内部标签（defines internal labels）（通常用于 goto 语句）。
+ 你希望保留特定 object file 中例程的顺序（order of routines）。

&emsp;如果 symbol 的定义位于 an assembly file、a stripped executable file 或 a file compiled without the -g option，gprof 将从 order file 中的 symbol’s entry 中忽略 source file name。如果项目使用此类 files，则必须手动编辑 order file 并添加适当的 source filenames。或者，你可以完全删除 symbol references，以强制以默认顺序 linked 相应的 routines。

&emsp;如果代码包含 internal labels，则必须从 order files 中删除这些 labels；否则，定义 label 的函数将在链接阶段被 split apart。可以通过在 assembly files 前面加上字符串 L_ 来防止将 internal labels 包含在 assembly files 中（You can prevent the inclusion of internal labels in assembly files altogether by prefixing them with the string L_）。汇编程序将带有此前缀的符号解释为特定函数的本地符号，并将其剥离（strips）以防止其他工具（如 gprof ）访问。（想起 objc-msg-arm64.s 文件中的 L 前缀的汇编指令标签）

&emsp;要保留特定 object file 中 routines 的顺序，请使用特殊符号 .section_all。例如，如果 object file foo.o 来自 assembly source，并且你希望 link 所有 routines 而不重新排序它们，请删除对 foo.o 的所有现有引用，并在 order file 中插入以下行：

```c++
foo.o:.section_all
```

&emsp;此选项对于从 assembly source 编译的 object files 或没有 source 的 object files 非常有用。

#### Linking with an Order File

&emsp;生成 order file 后，可以使用 -sectorder 和 -e start 选项链接程序：

```c++
cc -o outputFile inputFile.o … -sectorder __TEXT __text orderFile -e start
```

&emsp;要在 Xcode 项目中使用 order file，请在项目的 Deployment build style 中修改 “Other Linker Flags” 选项。Add the text `-sectorder __TEXT __text` orderFile to this setting to specify your order file.

&emsp;如果任何 inputFile 是 library 而不是 object file，则可能需要在链接之前编辑 order file，以将对 object file 的所有引用替换为对相应 library file 的引用。同样，linker 尽最大努力使 order file 中的名称与其正在编辑的 sources 匹配。

&emsp;使用这些选项，linker 构造可执行文件 outputFile，以便从 input files 的 \_\_text sections 构造 \_\_TEXT segment 的 \_\_text section 的内容。linker 按照 orderFile 中列出的顺序排列 input files 中的 routines。

&emsp;当 linker 处理 order file 时，它将 object-file 和 symbol-name pairs 未在 order file 中列出的 procedures 放入 outputFile 的 \_\_text section。它以默认顺序链接这些符号。多次列出的 object-file 和 symbol-name pairs 总是生成警告，并且使用该 pair 的第一次出现。

&emsp;默认情况下，linker 打印 linked objects 中不在 order file 中的 symbol names 数、顺序文件中不在链接对象中的符号名称数以及它尝试匹配的不明确符号名称数的摘要。要请求这些符号的详细列表，请使用 -sectorder_detail 选项。

&emsp;linker 的 -e start 选项保留 executable 的入口点（entry point）。start 符号（注意，缺少前导“ _”）在 C runtime shared library 的 /usr/bin/crt1.o 中定义；它表示程序中正常链接时的第一个 text 地址。当 reorder procedures 时，必须使用此选项来修复入口点。另一种方法是将 /usr/lib/crt1.o:start 或 /usr/lib/crt1.o:section_all 作为 order file 的第一行。

#### Limitations of gprof Order Files

&emsp;gprof 生成的 .order 文件只包含在运行可执行文件时 called 或 sampled 的那些函数。为了使 library functions 正确地出现在 order file 中，linker 生成的 whatsloaded 文件应该存在于工作目录中。

&emsp;-S 选项不适用于已与 order file 链接的可执行文件。

&emsp;生成 gmon.order 文件可能需要很长时间—可以使用 -x 参数抑制（suppressed）它。

&emsp;下列项目的文件名将丢失：

+ 不使用 -g 参数编译的文件
+ 从 assembly-language source 生成的 routines 
+ 删除了调试符号的可执行文件（如 strip 工具）（executables that have had their debugging symbols removed (as with the strip tool)）

### Profiling With the Monitor Functions

&emsp;文件 /usr/include/monitor.h 声明了一组函数，你可以使用这些函数以编程方式分析代码的特定部分。你可以使用这些函数仅为代码的某些部分或所有代码收集统计信息。然后可以使用 gprof 工具从生成的文件中构建调用图（call graph）和其他性能分析数据（performance analysis data）。Listing 1 展示了如何使用 monitor 函数。

&emsp;Listing 1  Using monitor functions
```c++
#include <monitor.h>
 
    /* To start profiling: */
    moninit();
    moncontrol(1);
 
    /* To stop, and dump to a file */
    moncontrol(0);
    monoutput("/tmp/myprofiledata.out");
    monreset();
```

### Organizing Code at Compile Time

&emsp;GCC 编译器允许你在声明的任何函数或变量上指定属性（attributes）。section 属性可让你告诉 GCC 你要放置一段特定的代码的哪个 segment 和 section。

&emsp;Warning: 除非你了解 Mach-O 可执行文件的结构，并且知道在相应的 segments 中放置 functions 和 data 的规则，否则不要使用 section 属性。将 function 或 global variable 放在不适当的 section 可能会中断程序。

&emsp;section 属性接受几个参数，这些参数控制结果代码的放置位置。至少，必须为要放置的代码指定 segment 和 section 名称。其他 options 也可用，在 GCC 文档中有描述。

&emsp;下面的列表显示了如何对函数使用 section 属性。在本例中，section 属性被添加到函数的前向声明中。该属性告诉编译器将函数放置在可执行文件的特定 \_\_text section。

```c++
void MyFunction (int a) __attribute__((section("__TEXT,__text.10")));
```

&emsp;下面的列表显示了一些如何使用 section 属性组织全局变量的示例。

```c++
extern const int x __attribute__((section("__TEXT,__my_const")));
const int x=2;
 
extern char foo_string[] __attribute__((section("__DATA,__my_data")));
char foo_string[] = "My text string\n";
```

&emsp;有关指定 section 属性的详细信息，请参阅 /Developer/documentation/DeveloperTools/gcc3 中的 GCC 编译器文档。

### Reordering the __text Section

&emsp;如 Mach-O 可执行文件格式概述中所述，\_\_TEXT segment 保存程序的 actual code 和 read-only 部分。按照惯例，编译器工具将 Mach-O 对象文件（扩展名为 .O）中的 procedures 放在 \_\_TEXT segment 的 \_\_text section。

&emsp;当程序运行时，\_\_text section 的 page 会按需加载到内存中，因为这些 pages 上的 routines 会被使用。代码按照它在给定源文件中出现的顺序链接到 \_\_text section，源文件按照它们在 linker command line 中列出的顺序（或在 Xcode 中指定的顺序）链接。因此，来自第一个 object file 的代码从头到尾被链接，接着是来自第二个文件和第三个文件的代码，依此类推。

&emsp;以 source file 声明顺序加载 code 很少是最优的。例如，假设代码中的某些方法或函数被重复调用，而其他方法或函数很少使用。对 procedures 进行重新排序，将常用代码放在 \_\_text section 的开头，可以最大限度地减少应用程序使用的平均页数（average number of pages），从而减少分页活动（paging activity）。

&emsp;作为另一个例子，假设代码定义的所有对象都同时初始化。因为每个类的 initialization routine 都是在 a separate source file 中定义的，所以 initialization code 通常分布在 \_\_text section。通过对所有类连续初始化代码的重新排序，可以减少需要读入的页数，从而提高初始化性能。应用程序只需要少量包含初始化代码的 pages，而不需要大量 pages，每个 page 只包含一小部分初始化代码。

#### Reordering Procedures

&emsp;根据应用程序的大小（size）和复杂性（complexity），你应该采用一种排序代码的策略，以最大程度地提高可执行文件的性能。与大多数性能调优一样，测量和重新调整程序顺序花费的时间越多，节省的内存就越多。通过运行应用程序并按调用频率（call frequency）对 routines 进行排序，可以很容易地获得良好的初次排序（a good first-cut ordering）。下面列出了此策略（strategy）的步骤，并在以下各节中进行了详细说明：

1. 构建应用程序的 a profile version。此步骤生成一个可执行文件，其中包含分析和重新排序过程中使用的符号。
2. 运行并使用应用程序创建 a set of profile data。执行一系列功能测试，或让某人在测试期间使用该程序。

> Important: 为了获得最佳结果，请关注最典型的使用模式。避免使用应用程序的所有特性，否则 profile data 可能会被 diluted（稀释）。 For example, focus on launch time and the time to activate and deactivate your main window. Do not bring up ancillary window.

3. 创建 order files。Order files 按优化顺序（optimized order）列出 procedures。linker 使用 order files 对可执行文件中的 procedures 重新排序。
4. 使用 order files 运行 linker。这将创建一个可执行文件，其中的 procedures 链接到 order file 中指定的 \_\_text section。

&emsp;For information on profiling your code and generating and linking an order file, see Profiling Code With gprof.

#### Procedure Reordering for Large Programs

&emsp;对于许多程序来说，由上述步骤生成的顺序比无序过程带来了实质性的改进。对于一个功能很少的简单应用程序，这样的排序代表了通过 procedure 重新排序获得的大部分收益。然而，更大的应用程序和其他大型程序可以从额外的分析中获益匪浅。虽然基于调用频率（call frequency）或调用图（call graph）的 order files 是一个很好的开始，但你可以利用对应用程序结构的了解来进一步减少 virtual-memory working set。

##### Creating a Default Order File

&emsp;如果要使用上述技术以外的其他技术对应用程序的 procedures 进行重新排序，则可以跳过分析步骤，从列出应用程序所有 routines 的默认 order file 开始。一旦你有了一个合适形式的 routines 列表，你就可以手动或使用你选择的排序技术重新排列条目。然后可以将生成的 order file 与 linker 的 -sectorder 选项一起使用，如 Linking with an Order File 中所述。

&emsp;要创建默认 order file，请首先使用 -whatsloaded 选项运行 linker：

```c++
cc -o outputFile inputFile.o -whatsloaded > loadedFile
```

&emsp;这将创建一个 loadingFile 文件，该文件列出了可执行文件（包括 frameworks 或其他 libraries 中的任何文件）中加载的 object files。-whatsLoad 选项还可以用于确保 gprof -S 生成的 order files 包含静态库（static libraries）中 procedures 的名称。

&emsp;使用 loadedFile 文件，可以使用 -onjls 选项和 \_\_TEXT \_\_text 参数运行 nm：

```c++
nm -onjls __TEXT __text `cat loadedFile` > orderFile
```

&emsp;orderFile 文件的内容是 text section 的 symbol table。procedures 在符号表中以其默认链接顺序列出。你可以重新排列此文件中的条目，以更改要链接 procedures 的顺序，然后按照 Linking with an Order File 所述运行 linker。

##### Using pagestuff to Examine Pages on Disk

&emsp;Pagestuff 工具通过告诉你在给定的时间可能在内存中加载了可执行文件的哪些 pages，可以帮助你评估 procedure ordering 的有效性。本节简要介绍如何使用此工具。有关更多信息，请参阅 pagestuff man page。

&emsp;Pagestuff 工具打印出特定可执行代码 page 上的 symbols。以下是命令的语法：

```c++
pagestuff filename [pageNumber | -a]
```

&emsp;pagestuff 的输出是包含在 filename 的 pageNumber 页码上的页中的 procedures 列表。 要查看文件的所有 pages，请使用 -a 选项代替页码。此输出允许你确定与内存中的文件相关联的每个页是否已优化。如果不是这样，你可以重新排列 order file 中的条目并再次链接可执行文件，以最大限度地提高性能。例如，将两个相关 procedures 移到一起，使它们链接在同一页上。完善排序可能需要几个链接和调整周期。（Perfecting the ordering may require several cycles of linking and tuning.）

##### Grouping Routines According to Usage

&emsp;为什么要为应用程序的各个操作生成 profile data？该策略基于以下假设：大型应用程序有三组常规 routines：

+ Hot routines 在应用程序的最常见用法期间运行。这些通常是原始 routines，它为应用程序的 features（例如，访问文档的数据结构的 routines）或实现应用程序的核心 features 的 routines 提供基础，例如在字处理器中实现打字的 routines。这些 routines 应该聚集在同一组 pages 中。
+ Warm routines 实现应用程序的特定 features。Warm routines 通常与用户偶尔执行的特定 features 相关联（例如启动、打印或导入图形）。因为这些 routines 经常被合理地使用，所以将它们聚集在同一个小 pages 集中，这样它们就可以快速加载。但是，由于用户不访问此功能的时间很长，因此这些 routines 不应位于 hot category 中。
+ 在应用程序中很少使用 Cold routines。Cold routines 实现模糊 features 或覆盖边界或错误情况。将这些 routines 组合在一起，以避免在 hot or warm page 上浪费空间。

&emsp;在任何给定的时间，你都应该期望大多数 hot pages 是驻留的，而对于用户当前使用的功能，你应该期望 hot pages 是驻留的。只有极少数情况下，cold page 才是常驻的。

&emsp;为了实现这种理想的排序，需要收集大量的 profile data sets。首先，收集 hot routines。如上所述，编译应用程序进行分析（profiling），启动它，然后使用程序。使用 gprof -S，从 profile data 生成一个以调用频率排序的 order file，称为 hot.order。

&emsp;创建 hot order file 后，为用户偶尔使用的 features 创建 order files，例如仅在启动应用程序时运行的 routines。打印、打开文档、导入图像和使用各种 non-document windows 和工具是用户偶尔使用但不连续使用的 features 的其他示例，是拥有自己的 order files 的良好候选。建议在分析 feature 之后命名这些 order files（例如，feature.order）。

&emsp;最后，要生成所有 routines 的列表，请构建一个 “default” order 文件 default.order（如 Reordering Procedures 中所述）。

&emsp;一旦有了这些 order files，就可以使用 Listing 2 中所示的代码来组合它们。可以使用此列表构建 command-line utility，该 utility 可以删除 order files 中的重复行，同时保留原始数据的顺序。

Listing 2  Code for Unique.c

```c++
//
//  unique
//
//  A command for combining files while removing
//  duplicate lines of text. The order of other lines of text
//  in the input files is preserved.
//  unique 是在删除重复的文本行的同时合并文件的命令。将保留输入文件中其他行文本的顺序。
//
//  Build using this command line:
//
//  cc -ObjC -O -o unique -framework Foundation Unique.c
//
//  Note that “unique” differs from the BSD command “uniq” in that
//  “uniq” combines duplicate adjacent lines, while “unique” does not
//  require duplicate lines to be adjacent. “unique” is also spelled
//  correctly.
//  请注意，“unique” 与 BSD 命令 “uniq” 的不同之处在于，
//  “uniq” 组合了重复的相邻行，而 “unique” 不要求重复行相邻 “unique” 的拼写也正确。
 
#import <stdio.h>
#import <string.h>
#import <Foundation/NSSet.h>
#import <Foundation/NSData.h>
 
#define kBufferSize 8*1024
 
void ProcessFile(FILE *fp)
{
    char buf[ kBufferSize ];
 
    static id theSet = nil;
 
    if( theSet == nil )
    {
        theSet = [[NSMutableSet alloc] init];
    }
 
    while( fgets(buf, kBufferSize, fp) )
    {
        id dataForString;
 
        dataForString = [[NSData alloc] initWithBytes:buf length:strlen(buf)];
 
        if( ! [theSet containsObject:dataForString] )
        {
            [theSet addObject:dataForString];
            fputs(buf, stdout);
        }
 
        [dataForString release];
    }
}
 
int main( int argc, char *argv[] )
{
    int     i;
    FILE *  theFile;
    int     status = 0;
 
    if( argc > 1 )
    {
        for( i = 1; i < argc; i++ )
        {
            if( theFile = fopen( argv[i], "r" ) )
            {
                ProcessFile( theFile );
                fclose( theFile );
            }
            else
            {
                fprintf( stderr, "Could not open ‘%s’\n", argv[i] );
                status = 1;
                break;
            }
        }
    }
    else
    {
        ProcessFile( stdin );
    }
 
    return status;
}
```

&emsp;一旦构建，你将使用该程序生成最终 order file，其语法如下所示：

```c++
unique hot.order feature1.order ... featureN.order default.order > final.order
```

&emsp;当然，排序的真正测试是减少分页 I/O 的数量。运行应用程序，使用不同的功能，并检查排序文件在不同条件下的性能。你可以使用 top 工具（以及其他工具）来度量分页性能。

##### Finding That One Last Hot Routine

&emsp;重新排序后，通常会在一个页区域中包含一些 cold routines，这些 routines 通常在文本排序（text ordering）结束时就很少使用。然而，一到两个 hot routines 可能会从裂缝中滑出，落在这个 cold 的区域。这是一个代价高昂的错误，因为使用其中一个 hot routines 现在需要驻留整个 page，而这个 page 中充满了不太可能使用的 cold routines。

&emsp;检查可执行文件的 cold pages 是否未被意外地分页。查找在应用程序 text segment 的 cold region 中具有 high-page offsets 的常驻页（pages that are resident）。如果有一个不需要的页，你需要找出调用该页上的 routine。一种方法是在接触该页的特定操作期间进行概要分析，并使用 grep 工具在概要分析程序输出中搜索驻留在该页上的 routines。或者，一种快速识别页被触摸位置的方法是在 gdb 调试器下运行应用程序，并使用 Mach call vm_protect 来禁止对该页的所有访问：

```c++
(gdb) p vm_protect(task_self(), startpage_addr, vm_page_size, FALSE, 0);
```

&emsp;清除页保护后，对该页的任何访问都会导致内存错误，从而中断调试器（debugger）中的程序。此时，你只需查看函数调用堆栈（使用 bt 命令）即可了解调用 routine 的原因。

### Reordering Other Sections

&emsp;可以使用 linker 的 -sectorder 选项来组织可执行文件的大多数 sections 中的 blocks。偶尔可能受益（occasionally benefit）于重新排列的 sections 是 literal sections，例如 \_\_TEXT segment 的 \_\_cstring section 或 \_\_DATA segment 的 \_\_data section。

#### Reordering Literal Sections

&emsp;使用 ld 和 otool 工具可以最容易地生成 literal sections 的 order file 中的行。对于 literal sections，otool 为每种类型的 literal section 创建特定类型的 order file：

+ 对于 C 字符串 literal sections，order-file 格式是每行一个文本 C 字符串（C 字符串中允许使用 ANSI C 转义序列）。例如，如下一行：

```c++
Hello world\n
```

+ 对于 4 字节的 literal sections，order-file 格式是一个 32 位十六进制数，每行前导 0x，行的其余部分作为注释处理。例如，如下一行：

```c++
0x3f8ccccd (1.10000002384185790000e+00)
```

+ 对于 8 字节的 literal sections，order file 行由每行两个 32 位十六进制数组成，每行用空格分隔，每个空格都有一个前导 0x，其余的行作为注释处理。例如，如下一行：

```c++
0x3ff00000 0x00000000 (1.00000000000000000000e+00)
```

+ 对于 literal pointer sections，order file 中的行的格式表示指针，每行一个。文字指针由 segment name、文字指针的 section name 和文字本身表示。它们之间用冒号分隔，没有多余的空格。例如，如下一行：

```c++
__OBJC:__selector_strs:new
```

+ 对于所有的 literal sections，order file 中的每一行都被简单地输入到 literal section，并以 order 文件 的顺序出现在输出文件（output file）中。不进行检查，以查看文字是否在加载的对象中。

&emsp;要对 literal section 重新排序，首先使用 ld -whatsloaded 选项创建一个 “whatsloaded” 文件，如 Creating a Default Order File 中所述。然后，使用适当的 options、segment 和 section 名称以及 filenames 运行 otool。otool 的输出是指定 section 的默认 order file。例如，以下命令行生成一个 order file，其中列出了文件 cstring_order 中的 \_\_TEXT segment 的 \_\_cstring section 的默认加载顺序：

```c++
otool -X -v -s __TEXT __cstring `cat whatsloaded` > cstring_order
```

&emsp;一旦创建了文件 cstring_order，就可以编辑该文件并重新排列其条目以优化引用的位置。例如，你可以将程序最常用的文字字符串（例如出现在用户界面中的标签）放在文件的开头。要在可执行文件中生成所需的加载顺序，请使用以下命令：

```c++
cc -o hello hello.o -sectorder __TEXT __cstring  cstring_order
```

#### Reordering Data Sections

&emsp;目前没有工具来测量对 data symbols 的代码引用。但是，你可能知道程序的数据引用模式，并且可以通过将很少使用的特性的数据与其他数据分离来节省一些成本。实现 \_\_data section 重新排序的一种方法是按大小对数据进行排序，这样小的数据项最终会出现在尽可能少的页上。例如，如果一个较大的数据项跨两个页放置，而两个较小的数据项共享其中的每一个页，则必须对较大的数据项进行分页才能访问较小的数据项。按大小重新排序数据可以最大限度地降低这种低效率。因为这些数据通常需要写入虚拟内存备份存储区（virtual-memory backing store），所以在某些程序中这可能是一个很大的节省。

&emsp;要重新排序 \_\_data section，请首先创建一个 order file，按你希望链接的顺序列出 source files 和 symbols（在 Generating Order Files 的开头描述了 order file entries）。然后，使用 -sectorder 命令行选项链接程序：

```c++
cc -o outputFile inputFile.o … -sectorder __DATA __data orderFile -e start
```

&emsp;要在 Xcode 项目中使用 order file，请在项目的 Deployment build style 中修改 “Other Linker Flags” 选项。将 text -sectorder \_\_DATA \_\_data orderFile 添加到此设置以指定你的 order file。

### Reordering Assembly Language Code

&emsp;在重新排序用汇编语言编写的 routines 时，需要记住的一些附加准则：

+ temporary labels in assembly code

&emsp;在手动编码的汇编代码中，请注意分支到临时标签的分支会在非临时标签上分支。例如，如果使用以 “L” 开头的标签或 d 标签（其中 d 是数字），如本例所示

```c++
foo: b 1f
    ...
bar: ...
1:   ...
```

&emsp;生成的程序将无法正确链接或执行，因为只有 foo 和 bar 符号才能将其放入对象文件的符号表中。对临时标签 1 的引用被编译为偏移量；结果，没有为指令 b 1f 生成重定位条目。如果链接器没有将与符号栏关联的块直接放在与 foo 关联的块之后，则 1f 的分支将不会到达正确的位置。因为没有重定位条目，链接器不知道如何修复分支。解决此问题的源代码更改是将标签 1 更改为非临时标签（例如 bar 1）。通过将包含手工编码的汇编代码的对象文件链接为一个整体，而无需重新排序，可以避免这些文件出现问题。

+ the pseudo-symbol .section_start

&emsp;如果任何输入文件中指定的 section 的大小为非零，并且没有具有其 section 开头值的符号，则链接器将伪符号 .section_start 用作与节中第一个块关联的符号名。此符号的目的是处理其符号不会持久存在于对象文件中的文字常量。因为文字字符串和浮点常量都在文字部分，所以这对 Apple 编译器来说不是问题。你可能会看到汇编语言程序或非 Apple 编译器使用的此符号。但是，你不应该对这些代码重新排序，而应该将整个文件链接起来，而不必重新排序（请参见 Linking with an Order File）。

## Reducing Shared Memory Pages

&emsp;如 Overview of the Mach-O Executable Format 所述，Mach-O 二进制文件的 \_\_DATA segment 中的数据是可写的，因此是可共享的（通过 copy-on-write）。在内存不足的情况下，可写数据会增加可能需要写入磁盘的页数，从而降低分页性能。对于 frameworks，可写数据最初是共享的，但有可能被复制到每个进程的内存空间。

&emsp;减少可执行文件中的 dynamic 或 non-constant data 会对性能产生重大影响，特别是对于 frameworks，以下部分将向你展示如何减少可执行文件的 \_\_DATA segment 的大小，从而减少 shared memory pages 的数量。

### Declaring Data as const

&emsp;使 \_\_DATA segment 变小的最简单方法是将范围更广的数据标记为常量。大多数时候，很容易将数据标记为常量。例如，如果你永远不会修改数组中的元素，则应在数组声明中包含 const 关键字，如下所示：

```c++
const int fibonacci_table[8] = {1, 1, 2, 3, 5, 8, 13, 21};
```

&emsp;记住将指针标记为常量（适当时）。在下面的示例中，字符串 “a” 和 “b” 是常量，但数组指针 foo 不是：

```c++
static const char *foo[] = {"a", "b"};
foo[1] = "c";       // OK: foo[1] is not constant.
```

&emsp;要将整个声明标记为常量，需要将 const 关键字添加到指针以使指针为常量。在以下示例中，数组及其内容都是常量：

```c++
static const char *const foo[] = {"a", "b"};
foo[1] = "c";       // NOT OK: foo[1] is constant.
```

&emsp;有时你可能需要重写代码来分离常量数据。下面的示例包含一个结构数组，其中只有一个字段声明为 const。因为整个数组没有声明为 const，所以它存储在 \_\_DATA segment 中。

```c++
const char *const imageNames[100] = { "FooImage", /* . . . */ };
NSImage *imageInstances[100] = { nil, /* . . . */ };
```

&emsp;如果未初始化的数据项包含指针，则编译器无法将该项存储在 \_\_TEXT segment 中。字符串结束于 \_\_TEXT segment 的 \_\_cstring section，但数据项的其余部分（包括指向字符串的指针）结束于 \_\_DATA segment 的 const section。在下面的示例中，daythedule 将在 \_\_TEXT 和 \_\_DATA segments 之间拆分，即使它是 constant：

```c++
struct daytime {
    const int value;
    const char *const name;
};
 
const struct daytime daytimeTable[] = {
    {1, "dawn"},
    {2, "day"},
    {3, "dusk"},
    {4, "night"}
};
```

&emsp;要将整个数组放入 \_\_TEXT segment，必须重写此结构，使其使用固定大小的 char 数组而不是字符串指针，如以下示例所示：

```c++
struct daytime {
    const int value;
    const char name[6];
};
 
const struct daytime daytimeTable[] = {
    {1, {'d', 'a', 'w', 'n', '\0'}},
    {2, {'d', 'a', 'y', '\0'}},
    {3, {'d', 'u', 's', 'k', '\0'}},
    {4, {'n', 'i', 'g', 'h', 't', '\0'}}
};
```

&emsp;不幸的是，如果字符串的大小千差万别，就没有好的解决方案，因为这种解决方案会留下大量未使用的空间。

&emsp;数组被分成两段，因为编译器总是在 \_\_TEXT segment 的 \_\_cstring section 存储常量字符串。如果编译器将数组的其余部分存储在 \_\_DATA segment 的 \_\_data section 中，则字符串和指向字符串的指针可能会出现在不同的页上。如果发生这种情况，系统将不得不用新地址更新指向字符串的指针，而如果指针位于 \_\_TEXT segment 中，则不能这样做，因为 \_\_TEXT segment 被标记为只读。因此指向字符串的指针以及数组的其余部分必须存储在 \_\_DATA segment 的 const section。\_\_const section 是为声明为 const 的数据保留的，这些数据不能放在 \_\_TEXT segment 中。

### Initializing Static Data

&emsp;正如 Overview of the Mach-O Executable Format 中指出的，编译器将未初始化的静态数据存储在 \_\_DATA segment 的 \_\_bss section，并将初始化的数据存储在 \_\_data section。如果 \_\_bss section 中只有少量静态数据，则可能需要考虑将其移到 \_\_data section。将数据存储在两个不同的 sections 会增加可执行文件使用的内存页的数量，从而增加分页的可能性。

&emsp;合并 \_\_bss 和 \_\_data sections 的目的是减少应用程序使用的内存页数。如果将数据移到 \_\_data section 会增加该 section 中的内存页数，则此技术没有任何好处。事实上，添加到 \_\_data section 的页会增加在启动时读取和初始化数据所花费的时间。

&emsp;假设你声明以下静态变量：

```c++
static int x;
static short conv_table[128];
```

&emsp;要将这些变量移到可执行文件的 \_\_DATA segment 的 \_\_data section 中，请将定义更改为：

```c++
static int x = 0;
static short conv_table[128] = {0};
```

### Avoiding Tentative-Definition Symbols

&emsp;编译器将遇到的任何重复符号放入 \_\_DATA segment 的 \_\_common section（请参阅 Overview of the Mach-O Executable Format）。这里的问题与未初始化的静态变量相同。如果一个可执行文件的非常量全局数据分布在多个 sections 中，则这些数据更有可能位于不同的内存页上；因此，页可能必须单独交换。\_\_common section 的目标与 \_\_bss section 的目标相同：如果在可执行文件中有少量数据，则将其从可执行文件中删除。

&emsp;a tentative-definition symbol 的 common source 是头文件中该符号的定义。通常，头声明一个符号，但不包括该符号的定义；而是在实现文件中提供定义。但是出现在头文件中的定义会导致代码或数据出现在包含头文件的每个实现文件中。这个问题的解决方案是确保头文件只包含声明，而不包含定义。

&emsp;对于函数，你显然会在头文件中声明该函数的原型，并将该函数的定义放在实现文件中。对于全局变量和数据结构，应该执行类似的操作。与其在头文件中定义变量，不如在实现文件中定义并适当初始化它。然后，在头文件中声明该变量，在声明前面加上 extern 关键字。这种技术将变量定义本地化为一个文件，同时仍然允许从其他文件访问该变量。

&emsp;当意外地导入同一头文件两次时，还可以获得 tentative-definition symbols。为确保不执行此操作，请包含预处理器指令以禁止包含已包含的文件。因此，在头文件中，你将拥有以下代码：

```c++
#ifndef MYHEADER_H
#define MYHEADER_H
// Your header file declarations. . .
#endif
```

&emsp;然后，如果要包含该头文件，请按以下方式包含它：

```c++
#ifndef MYHEADER_H
#include "MyHeader.h"
#endif
```

### Analyzing Mach-O Executables

&emsp;你可以使用多种工具来确定非常量数据占用了多少内存。这些工具报告数据使用的各个方面。

&emsp;在应用程序或 framework 运行时，使用 size 和 pagestuff 工具查看各种 data sections 有多大以及它们包含哪些符号。需要查找的内容包括：

+ 要查找包含大量非常量数据的可执行文件，请检查 \_\_DATA segment 中包含大 \_\_data sections 的文件。
+ 检查 \_\_bss 和 \_\_common sections 是否存在可以删除或移动到 \_\_data section 的变量和符号。
+ 若要查找虽然声明为常量，但编译器无法将其视为常量的数据，请检查 \_\_DATA segment 中是否存在可执行文件或带有 \_\_const section 的对象文件。

&emsp;\_\_DATA segment 中一些较大的内存消耗者是已初始化但未声明常量的固定大小全局数组。有时可以通过在源代码中搜索 “[]={” 来找到这些表。

&emsp;你还可以让编译器帮助你找到哪些数组可以设置为常量。将 const 放在所有你怀疑为只读的初始化数组前面，然后重新编译。如果一个数组不是真正的只读的，它将不会编译。删除有问题的常量并重试。

## Minimizing Your Exported Symbols

&emsp;如果应用程序或 framework 具有公共接口，则应将导出的符号限制为接口所需的符号。导出的符号占用可执行文件的空间，应尽可能减少。这不仅减少了可执行文件的大小，还减少了动态链接器（dynamic linker）的工作量。

&emsp;默认情况下，Xcode 从项目中导出所有符号。你可以使用下面的信息来标识和消除不希望导出的符号。

### Identifying Exported Symbols

&emsp;要查看应用程序导出的符号，请使用 nm 工具。此工具读取可执行文件的符号表，并显示你请求的符号信息。可以查看所有符号，也可以只查看可执行代码特定段中的符号。例如，要仅显示外部可用的全局符号，可以在命令行上指定 -g 选项。

&emsp;要查看详细的符号信息，请使用 -m 选项运行 nm。此选项的输出告诉你符号的类型以及它是外部的还是本地的（非外部的）。例如，要查看 TextEdit 应用程序的详细符号信息，可以使用 nm，如下所示：

```c++
%cd /Applications/TextEdit.app/Contents/MacOS
% nm -m TextEdit
```

&emsp;结果输出的一部分可能如下所示：

```c++
9005cea4 (prebound undefined [lazy bound]) external _abort (from libSystem)
9000a5c0 (prebound undefined [lazy bound]) external _atexit (from libSystem)
90009380 (prebound undefined [lazy bound]) external _calloc (from libSystem)
00018d14 (__DATA,__common) [referenced dynamically] external _catch_exception_raise
00018d18 (__DATA,__common) [referenced dynamically] external _catch_exception_raise_state
00018d1c (__DATA,__common) [referenced dynamically] external _catch_exception_raise_state_identity
```

&emsp;在此模式下，nm 根据符号显示各种信息。对于驻留在 \_\_TEXT segment 中的函数和其他代码，nm 显示预绑定信息和源库。对于 \_\_DATA segment 中的信息，nm 显示符号的特定 section 及其链接。对于所有符号，nm 显示符号是外部的还是本地的。

### Limiting Your Exported Symbols

&emsp;如果知道要从项目中导出的符号，则应创建一个导出文件，并将该文件添加到项目的链接器设置中。导出文件是一个纯文本文件，其中包含要使外部调用者可用的符号的名称。每个符号必须单独列在一行上。前导和尾随空格不被视为符号名称的一部分。以 # 符号开头的行将被忽略。

&emsp;要在 Xcode 项目中包含导出文件，请修改项目的 target or build-style settings。将 “Exported symbols file” 设置的值设置为导出文件的名称。Xcode 将适当的选项传递给静态链接器。

&emsp;要从命令行导出符号列表，请将 -exported_symbols_list 选项添加到链接器命令。也可以导出所有符号，然后限制特定列表，而不是导出特定的符号列表。要限制特定的符号列表，请在链接器命令中使用 -unexported_symbols_list 选项。

&emsp;请注意，运行时库导出的符号必须显式包含在导出文件中，才能正确启动应用程序。要收集这些符号的列表，请在没有导出文件的情况下链接代码，然后从终端执行 nm -m 命令。从生成的输出中，收集所有标记为外部的、不属于代码一部分的符号，并将它们添加到导出文件中。

### Limiting Exports Using GCC 4.0

&emsp;GCC4.0 支持单个符号的自定义可见性属性。此外，编译器还提供编译时标志，允许你为已编译文件的所有符号设置默认可见性。

&emsp;有关使用 GCC 4 的新符号可见性特征的信息，请参见 C++ Runtime Environment Programming Guide 中的 “Controlling Symbol Visibility“。

## 参考链接
**参考链接:🔗**
+ [Code Size Performance Guidelines](https://developer.apple.com/library/archive/documentation/Performance/Conceptual/CodeFootprint/CodeFootprint.html#//apple_ref/doc/uid/10000149-SW1)
+ [Memory Usage Performance Guidelines](https://developer.apple.com/library/archive/documentation/Performance/Conceptual/ManagingMemory/ManagingMemory.html#//apple_ref/doc/uid/10000160-SW1)
