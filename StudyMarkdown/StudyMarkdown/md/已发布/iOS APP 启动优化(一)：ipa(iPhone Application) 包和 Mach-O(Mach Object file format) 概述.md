# iOS APP 启动优化(一)：ipa(iPhone application archive) 包和 Mach-O(Mach Object file format) 概述

> &emsp;IPA 后缀的文件是 iOS 系统的软件包，全称为 iPhone application archive。通常情况下，IPA 文件都是使用苹果公司的 FairPlayDRM 技术进行加密保护的。每个 IPA 文件都是 ARM 架构（部分 mac 下应该是 intel 的 x86 架构）的可执行文件以及该应用的资源文件的打包文件，只能安装在 iPhone、iPod Touch、iPad 以及使用 Apple Silicon 平台的 Mac 上。该文件可以通过修改后缀名为 zip 后，进行解压缩，查看其软件包中的内容。[IPA文件-维基百科](https://zh.wikipedia.org/wiki/IPA文件)
 
> &emsp;数字版权管理（英语：Digital rights management，缩写为 DRM）是一系列访问控制技术，通常用于控制数字内容和设备在被销售之后的使用过程。DRM 有时也称为拷贝保护、复制控制、技术保护措施等，但这些称呼存在争议。许多数字出版社和软件厂商都使用了 DRM，例如亚马逊、AT&T、AOL、Apple Inc.、Netflix、Google[7]、BBC、微软、Sony、Valve Corporation 等。[数字版权管理-维基百科](https://zh.wikipedia.org/wiki/数字版权管理)

## 解压 .ipa 文件查看其内容并引出 Mach-O 格式

&emsp;相信每一位 iOS 开发者都进行过打包测试，当我们把 Ad Hoc 或者 App Store Connect 的包导出到本地时会看到一个 xxx.ipa 文件，ipa 是 iPhone Application Archive 的缩写。实际上 xxx.ipa 只是一个变相的 zip 压缩包，我们可以把 xxx.ipa 文件直接通过 unzip 命令进行解压。

&emsp;我们直接新建一个命名为 Test_ipa_Simple 的空白 iOS App，直接进行 Archive 后并导出 Test_ipa_Simple.ipa 文件查看它的内部结构。在终端执行 unzip Test_ipa_Simple.ipa 解压之后，会有一个 Payload 目录，而 Payload 里则是一个看似是文件的 Test_ipa_Simple.app，而实际上它又是一个目录（文件夹），或者说是一个完整的 App Bundle。其中 Base.lproj 中是我们的 Main.storyboard 和 LaunchScreen.storyboard 的内容，然后是 embedded.mobileprovision（描述文件）和 PkgInfo、Info.plist、_CodeSignature 用于描述 App 的一些信息，然后我们要重点关注的便是当前这个目录里面体积最大的文件 Test_ipa_Simple，它是和我们的 ipa 包同名的一个[二进制文件](https://www.zhihu.com/question/19971994)，然后用 file 命令查看它的文件类型是一个在 arm64 处理器架构下的可执行（executable）文件，格式则是 Mach-O，其他还存在 FAT 格式的 Mach-O 文件（可直白的理解为胖的 Mach-O 文件），它们是支持多个架构的二进制文件的顺序组合，例如这里取 `/bin/ls` 路径下的系统文件 `ls` 作为示例，使用 file 命令对它进行查看，可看到它是一个 FAT 文件，它包含 x86_64 和 arm64e 两个架构（这里是 m1 Mac 下的 `ls` 文件），即这里的 `ls` 是一个支持 x86_64 和 arm64e 两种处理器架构的通用二进制文件，里面包含的两部分都是 Mach-O 格式的 64-bit 可执行文件。在了解了二进制文件的数据结构以后，一切就都显得没有秘密了。（下面是终端执行记录，可大致浏览一下）

```c++
// file 指令可看出 ls 的文件类型：
hmc@HMdeMac-mini Desktop % file ls
ls: Mach-O universal binary with 2 architectures: [x86_64:Mach-O 64-bit executable x86_64] [arm64e:Mach-O 64-bit executable arm64e]
ls (for architecture x86_64):    Mach-O 64-bit executable x86_64
ls (for architecture arm64e):    Mach-O 64-bit executable arm64e
hmc@HMdeMac-mini Desktop % 
```

```c++
hmc@bogon Test_ipa_Simple 2021-04-09 08-10-25 % unzip Test_ipa_Simple.ipa 
Archive:  Test_ipa_Simple.ipa
   creating: Payload/
   creating: Payload/Test_ipa_Simple.app/
   creating: Payload/Test_ipa_Simple.app/_CodeSignature/
  inflating: Payload/Test_ipa_Simple.app/_CodeSignature/CodeResources  
  inflating: Payload/Test_ipa_Simple.app/Test_ipa_Simple  
   creating: Payload/Test_ipa_Simple.app/Base.lproj/
   creating: Payload/Test_ipa_Simple.app/Base.lproj/Main.storyboardc/
  inflating: Payload/Test_ipa_Simple.app/Base.lproj/Main.storyboardc/UIViewController-BYZ-38-t0r.nib  
  inflating: Payload/Test_ipa_Simple.app/Base.lproj/Main.storyboardc/BYZ-38-t0r-view-8bC-Xf-vdC.nib  
  inflating: Payload/Test_ipa_Simple.app/Base.lproj/Main.storyboardc/Info.plist  
   creating: Payload/Test_ipa_Simple.app/Base.lproj/LaunchScreen.storyboardc/
  inflating: Payload/Test_ipa_Simple.app/Base.lproj/LaunchScreen.storyboardc/01J-lp-oVM-view-Ze5-6b-2t3.nib  
  inflating: Payload/Test_ipa_Simple.app/Base.lproj/LaunchScreen.storyboardc/UIViewController-01J-lp-oVM.nib  
  inflating: Payload/Test_ipa_Simple.app/Base.lproj/LaunchScreen.storyboardc/Info.plist  
  inflating: Payload/Test_ipa_Simple.app/embedded.mobileprovision  
  inflating: Payload/Test_ipa_Simple.app/Info.plist  
  inflating: Payload/Test_ipa_Simple.app/PkgInfo  
hmc@bogon Test_ipa_Simple 2021-04-09 08-10-25 % cd Payload 
hmc@bogon Payload % ls
Test_ipa_Simple.app
hmc@bogon Payload % cd Test_ipa_Simple.app 
hmc@bogon Test_ipa_Simple.app % ls -lht
total 240
drwxr-xr-x  4 hmc  staff   128B  4  9 08:10 Base.lproj
-rw-r--r--@ 1 hmc  staff   3.0K  4  9 08:10 Info.plist
-rw-r--r--  1 hmc  staff     8B  4  9 08:10 PkgInfo
-rwxr-xr-x  1 hmc  staff    86K  4  9 08:10 Test_ipa_Simple
drwxr-xr-x  3 hmc  staff    96B  4  9 08:10 _CodeSignature
-rw-r--r--  1 hmc  staff    20K  4  9 08:10 embedded.mobileprovision
hmc@bogon Test_ipa_Simple.app % file Test_ipa_Simple 
Test_ipa_Simple: Mach-O 64-bit executable arm64
```

## Mach-O 格式概述

> &emsp;Mach-O 为 Mach Object 文件格式的缩写，全称为 Mach Object File Format 它是一种用于可执行文件、目标代码、动态库、内核转储的文件格式。作为 a.out 格式的替代者，Mach-O 提供了更强的扩展性，并提升了符号表中信息的访问速度。
Mach-O 曾经为大部分基于 Mach 核心的操作系统所使用。NeXTSTEP、Darwin 和 Mac OS X 等系统使用这种格式作为其原生可执行档、库和目标代码的格式。而同样使用 GNU Mach 作为其微内核的 GNU Hurd 系统则使用 ELF 而非 Mach-O 作为其标准的二进制文件格式。[Mach-O-维基百科](https://zh.wikipedia.org/wiki/Mach-O)

&emsp;在 Xcode -> Build Setting -> Mach-O Type 中，Xcode 直接给我们列出了下面几种类型，看名字的话我们大概可以猜一下他们分别对应什么类型：

+ Executable（应用的主要二进制）
+ Dynamic Library（动态链接库（又称 DSO 或 DLL））
+ Bundle（不能被链接的 Dylib，只能在运行时使用 dlopen( ) 加载，可当做 macOS 的插件）
+ Static Library（静态链接库）
+ Relocatable Object File（可重定向文件类型）

&emsp;如果我们新建 iOS App 的话 Mach-O Type 默认就是 Executable，如果新建 Framework 或 Static Library 则 Mach-O Type 分别默认是  Dynamic Library 和 Static Library，如果我们同时选中 Include Tests，创建出的 TARGETS 中的 Tests 和 UITests 的 Mach-O Type 默认是 Bundle。

&emsp;实际上在 [apple/darwin-xnu](https://github.com/apple/darwin-xnu) 的 darwin-xnu/EXTERNAL_HEADERS/mach-o/loader.h 中定义了一组宏来表示不同的 Mach-O Type，如 `#define MH_EXECUTE 0x2 /* demand paged executable file */`、`#define MH_DYLIB 0x6 /* dynamically bound shared library */`、`#define MH_BUNDLE 0x8 /* dynamically bound bundle file */`、`#define MH_OBJECT 0x1 /* relocatable object file */` 等（它们分别对应上面的 Mach-O Type）。在数据结构层面这一组不同的宏正用于为 struct mach_header_64 的 filetype 字段赋值，来表示当前 Mach-O 的不同类型，等下面我们具体分析 Mach-O 结构的时候再来详细分析这些宏值所代表的含义。

&emsp;在 [Code Size Performance Guidelines](https://developer.apple.com/library/archive/documentation/Performance/Conceptual/CodeFootprint/CodeFootprint.html#//apple_ref/doc/uid/10000149-SW1) 文档中的 [Overview of the Mach-O Executable Format](https://developer.apple.com/library/archive/documentation/Performance/Conceptual/CodeFootprint/Articles/MachOOverview.html#//apple_ref/doc/uid/20001860-BAJGJEJC) 章节提到了 Mach-O 格式，并描述了如何组织 Mach-O executable format 来提高代码的效率，下面我们先看下这一节的原文。

&emsp;Mach-O 是 OS X 中二进制文件的 native 可执行格式，是 shipping code 的首选格式。可执行格式决定二进制文件中的代码（code）和数据（data）读入内存的顺序。代码和数据的顺序会影响内存使用和分页活动（paging activity），因此会直接影响程序的性能。

&emsp;Mach-O 二进制文件被组织成段（segments）。每个段包含一个或多个 sections。不同类型的代码或数据进入每个 section。Segments 总是从页（page）边界开始，但 sections 不一定是页对齐的（page-aligned）。Segment 的大小由它包含的所有 sections 中的字节数来度量，并向上舍入到下一个虚拟内存页的边界（virtual memory page boundary）。因此，一个 segment 总是 4096 字节或 4096 字节的倍数，其中 4096 字节是最小大小。

&emsp;Mach-O 可执行文件的 segments 和 sections 根据其预期用途命名。Segment 名称的约定是使用前有双下划线的所有大写字母组成（例如：\_\_TEXT）；Section 名称的约定是使用前有双下划线的所有小写字母组成（例如：\_\_text）。

&emsp;Mach-O 可执行文件中有几个可能的 segments，但是只有两个与性能有关：\_\_TEXT segment 和 \_\_DATA segment。

+ The \_\_TEXT Segment: Read Only

&emsp;\_\_TEXT segment 是一个只读区域，包含可执行代码和常量数据。按照惯例，编译器工具创建的每个可执行文件至少有一个只读 \_\_TEXT segment。由于该 segment 是只读的，内核可以将可执行文件中的 \_\_TEXT segment 直接映射（map）到内存中一次。当 segment 映射到内存中时，它可以在对其内容感兴趣的所有进程之间共享。（这主要是 frameworks 和 shared libraries 的情况。）只读属性还意味着组成 \_\_TEXT segment 的页不必保存到备份存储。如果内核需要释放物理内存，它可以丢弃一个或多个 \_\_TEXT 页，并在需要时从磁盘重新读取它们。

&emsp;表 1 列出了可以出现在 \_\_TEXT segment 中的一些更重要的 sections。有关 segments 的完整列表，请参阅 Mach-O Runtime Architecture。

&emsp;Table 1  Major sections in the __TEXT segment

| Section | Description |
| --- | --- |
| \_\_text | The compiled machine code for the executable（可执行文件的已编译机器码） |
| \_\_const | The general constant data for the executable（可执行文件的常规常量数据） |
| \_\_cstring | Literal string constants (quoted strings in source code) 字面量字符串常量（源代码中带引号的字符串） |
| \_\_picsymbol_stub | Position-independent code stub routines used by the dynamic linker (dyld) 动态链接器（dyld）使用的与位置无关的 code stub routines |

+ The __DATA Segment: Read/Write

&emsp;\_\_DATA segment 包含可执行文件的非常量数据。此 segment 既可读又可写。因为它是可写的，所以 framework 或其他 shared library 的 \_\_DATA segment 在逻辑上是为每个与 library 链接的进程复制的。当内存页可读写时，内核将它们标记为 copy-on-write。此技术延迟复制页（page），直到共享该页的某个进程尝试写入该页。当发生这种情况时，内核会为该进程创建一个页（page）的私有副本。

&emsp;\_\_DATA segment 有许多 sections，其中一些 sections 仅由动态链接器（dynamic linker）使用。表 2 列出了 \_\_DATA segment 中可能出现的一些更重要的 sections。有关 segments 的完整列表，请参阅 Mach-O Runtime Architecture。

&emsp;Table 2  Major sections of the __DATA segment

| Section | Description |
| --- | --- |
| \_\_data | Initialized global variables (for example int a = 1; or static int a = 1;). 初始化的全局变量 |
| \_\_const | Constant data needing relocation (for example, char * const p = "foo";). 需要重定位的常量数据 |
| \_\_bss | Uninitialized static variables (for example, static int a;). 未初始化的静态变量 |
| \_\_common | Uninitialized external globals (for example, int a; outside function blocks). 未初始化的外部全局变量 |
| \_\_dyld | A placeholder section, used by the dynamic linker. 动态链接器使用的占位符部分 |
| \_\_la_symbol_ptr | “Lazy” symbol pointers. Symbol pointers for each undefined function called by the executable. “Lazy” 符号指针。可执行文件调用的每个未定义函数的符号指针 |
| \_\_nl_symbol_ptr | “Non lazy” symbol pointers. Symbol pointers for each undefined data symbol referenced by the executable. “Non lazy” 符号指针。可执行文件引用的每个未定义数据符号的符号指针 |

+ Mach-O Performance Implications

&emsp;Mach-O 可执行文件的 \_\_TEXT 和 \_\_DATA 的组成对性能有直接影响。优化这些 segments 的技术和目标是不同的。然而，他们有一个共同的目标：提高内存的使用效率。

&emsp;大多数典型的 Mach-O 文件都由可执行代码组成，这些代码占据了 \_\_TEXT 中的 \_\_text section。如上面 The \_\_TEXT Segment: Read Only 中所述，\_\_TEXT segment 是只读的，直接映射到可执行文件。因此，如果内核需要回收某些 \_\_text 页所占用的物理内存，它不必将这些页保存到备份存储区，并在以后对它们进行分页。它只需要释放内存，当以后引用代码时，从磁盘读回它。虽然这比交换成本更低，因为它涉及一个磁盘访问而不是两个磁盘访问，所以它仍然是昂贵的，特别是在必须从磁盘重新创建许多页的情况下。

&emsp;改善这种情况的一种方法是通过过程重新排序（procedure reordering）来改善代码的引用位置，如 [Improving Locality of Reference](https://developer.apple.com/library/archive/documentation/Performance/Conceptual/CodeFootprint/Articles/ImprovingLocality.html#//apple_ref/doc/uid/20001862-CJBJFIDD) 中所述。这项技术根据方法和函数的执行顺序、调用频率以及调用频率将它们组合在一起。如果 \_\_text section 组中的页以这种方式正常工作，则不太可能释放它们并多次读回。例如，如果将所有启动时初始化函数放在一个或两个页上，则不必在这些初始化发生后重新创建页。

&emsp;与 \_\_TEXT segment 不同，\_\_DATA segment 可以写入，因此 \_\_DATA segment 中的页不可共享。frameworks 中的非常量全局变量（non-constant global variables）可能会对性能产生影响，因为与 framework 链接的每个进程（process）都有自己的变量副本。这个问题的主要解决方案是将尽可能多的非常量全局变量移到 \_\_TEXT 中的 \_\_const section，方法是声明它们为 const。[Reducing Shared Memory Pages](https://developer.apple.com/library/archive/documentation/Performance/Conceptual/CodeFootprint/Articles/SharedPages.html#//apple_ref/doc/uid/20001863-CJBJFIDD) 描述了这一点和相关的技术。对于应用程序来说，这通常不是问题，因为应用程序中的 \_\_DATA section 不与其他应用程序共享。

&emsp;编译器将不同类型的非常量全局数据（nonconstant global data）存储在 \_\_DATA segment 的不同 sections 中。这些类型的数据是未初始化的静态数据和符号（uninitialized static data and symbols），它们与未声明为 extern 的 ANSI C “tentative definition” 概念一致。未初始化的静态数据（Uninitialized static data）位于 \_\_DATA segment 的 \_\_bss section。临时定义符号（tentative-definition symbols）位于 \_\_DATA segment 的 \_\_common section。

&emsp;ANSI C 和 C++ 标准规定系统必须将未初始化静态变量（uninitialized static variables）设置为零。（其他类型的未初始化数据保持未初始化状态）由于未初始化的静态变量和临时定义符号（tentative-definition symbols）存储在分开的 sections 中，系统需要对它们进行不同的处理。但是，当变量位于不同的 sections 时，它们更有可能最终出现在不同的内存页上，因此可以分别进行换入和换出操作，从而使你的代码运行速度更慢。这些问题的解决方案（如 [Reducing Shared Memory Pages](https://developer.apple.com/library/archive/documentation/Performance/Conceptual/CodeFootprint/Articles/SharedPages.html#//apple_ref/doc/uid/20001863-CJBJFIDD) 中所述）是将非常量全局数据（non-constant global data）合并到 \_\_DATA segment 的一个 section 中。

&emsp;以上是 Overview of the Mach-O Executable Format 章节中的全部内容，可能我们对其中的 segment 和 section 还不太熟悉，下面我们会进行更详细的解读。

## Mach-O 文件内部构成

&emsp;下面我们结合 [apple/darwin-xnu](https://github.com/apple/darwin-xnu) 中的源码来分析 Mach-O 二进制文件的内部构成，首先看一张大家都在用的官方的图片。

![d06ff3536b6369f4652b6a5b862f9ced.png](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/ffa97f6d060e441a8d83d1bacc58f190~tplv-k3u1fbpfcp-watermark.image)

&emsp;从图上我们能明显看出 Mach-O 文件的数据主体分为三大部分：分别是 Header（头部）、Load commands（加载命令）、Data（最终的数据），可看到完全对应到上一节中提到的 “Mach-O 二进制文件被组织成多个段（segments），每个段包含一个或多个 sections”。 

### Header（Mach-O 头部）

&emsp;Mach-O 文件的 Header 部分对应的数据结构定义在 darwin-xnu/EXTERNAL_HEADERS/mach-o/loader.h 中，struct mach_header 和 struct mach_header_64 分别对应 32-bit architectures 和 64-bit architectures。（对于 32/64-bit architectures，32/64 位的 mach header 都出现在 Mach-O 文件的最开头。）

```c++
struct mach_header_64 {
    uint32_t    magic;        /* mach magic number identifier */
    cpu_type_t    cputype;    /* cpu specifier */
    cpu_subtype_t    cpusubtype;    /* machine specifier */
    uint32_t    filetype;    /* type of file */
    uint32_t    ncmds;        /* number of load commands */
    uint32_t    sizeofcmds;    /* the size of all the load commands */
    uint32_t    flags;        /* flags */
    uint32_t    reserved;    /* reserved */
};
```

&emsp;观察 mach_header_64 结构体各个字段的名字，可看到 header 部分存放的是当前 Mach-O 文件的一些概述信息，例如：支持的 CPU 类型（架构）、支持的 CPU 子类型、文件类型（对应上面的 Mach-O Type）、Load commands 的数量、Load commands 的大小等内容。

+ magic 是 mach 的魔法数标识，Test_ipa_Simple 的 magic 是 MH_MAGIC_64，该值是 loader.h 中的一个宏：`#define MH_MAGIC_64 0xfeedfacf` 用于表示 64 位的 mach 魔法数（64-bit mach magic number）。

&emsp;这里牵涉到一个 magic number（魔数）的概念。对于一个二进制文件来说，其对应的类型可以在其最初几个字节来标识出来，即 “魔数”。例如我们特别熟悉的 png 格式的图片，使用 xxd 命令查看前 8 个字节的内容 `00000000: 8950 4e47 0d0a 1a0a 0000 000d 4948 4452  .PNG........IHDR` 我们可识别出它是一张 png 格式的图片，再例如常见的 shell 脚本文件前 8 个字节的内容 `00000000: 6563 686f 2022 7e7e 7e7e 7e7e 7e7e 7e7e  echo "~~~~~~~~~~`。

+ filetype 表示 Mach-O Type，这个可以有很多类型，静态库（.a）、单个目标文件（.o）都可以通过这个类型标识来区分。（可执行文件、符号文件(DSYM)、内核扩展等）
+ ncmds 表示 Load commands 加载命令的数量。
+ sizeofcmds 表示 Load commands 加载命令所占的大小。
+ flags 不同的位表示不同的标识信息，比如 TWOLEVEL 是指符号都是两级格式的，符号自身 + 加上自己所在的单元，PIE 标识是位置无关的。

&emsp;这里我们可以通过几种不同方式来查看 Test_ipa_Simple 文件 header 中各个字段的具体值。

1. 通过 `otool -v -h Test_ipa_Simple` 可查看上面 Test_ipa_Simple 文件的 header 中的内容，去掉 `-v` 则是各字段的原始数值。看到其中有我们较为熟悉的 cputype 是 ARM64、filetype 是可执行文件（EXECUTE）。

```c++
hmc@bogon Test_ipa_Simple.app % otool -h Test_ipa_Simple
Test_ipa_Simple:
Mach header
      magic  cputype cpusubtype  caps    filetype ncmds sizeofcmds      flags
 0xfeedfacf 16777228          0  0x00           2    22       2800 0x00200085
 
+++++++++++++++++++++++++++++++++++++++++++

hmc@HMdeMac-mini Test_ipa_Simple.app % otool -v -h Test_ipa_Simple        
Test_ipa_Simple:
Mach header
      magic  cputype cpusubtype  caps    filetype ncmds sizeofcmds      flags
MH_MAGIC_64    ARM64        ALL  0x00     EXECUTE    22       2800   NOUNDEFS DYLDLINK TWOLEVEL PIE
hmc@HMdeMac-mini Test_ipa_Simple.app % 
```

&emsp;这里 flags 中的几个值我们可以直接在 loader.h 里面找到，然后它们对应的值进行按位 & 以后得到的值正是：0x00200085。

```c++
// 目标文件没有未定义的符号
#define MH_NOUNDEFS 0x1 /* the object file has no undefined references */

// 目标文件是动态链接输入文件，不能被再次静态链接
#define MH_DYLDLINK 0x4 /* the object file is input for the dynamic linker and can't be staticly link edited again */

// 只读 segments 和 可读写 segments 分离
#define MH_SPLIT_SEGS 0x20 /* the file has its read-only and read-write segments split */

#define MH_TWOLEVEL 0x80 /* the image is using two-level name space bindings */
#define MH_PIE 0x200000 /* When this bit is set, the OS will load the main executable at a random address. Only used in MH_EXECUTE filetypes. */
```

> &emsp;简单总结一下就是 Headers 能帮助校验 Mach-O 合法性和定位文件的运行环境。[探秘 Mach-O 文件](http://hawk0620.github.io/blog/2018/03/22/study-mach-o-file/)

2. 通过 [MachOView](https://github.com/fangshufeng/MachOView) 工具查看。 

![截屏2021-04-16 08.45.44.png](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/1c07afe370ea4fd08615393af1adf057~tplv-k3u1fbpfcp-watermark.image)

3. 直接使用 xxd 命令读取以十六进制读取二进制文件的内容。（这里看到 magic 值是 0xcffaedfe，同一个文件上面使用 otool 和 MachOView 看到的值是 0xfeedfacf）`#define MH_CIGAM_64 0xcffaedfe /* NXSwapInt(MH_MAGIC_64) */`

![截屏2021-04-16 08.51.00.png](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/dc0b2f9d65974ce5a778f975888c07a4~tplv-k3u1fbpfcp-watermark.image)

### Load commands

&emsp;Header 中的数据已经说明了整个 Mach-O 文件的基本信息，但是整个 Mach-O 中最重要的还是 Load commands，Header 之后就是 Load commands，其占用的内存和加载命令的总数在 Header 中已经指出。它说明了操作系统应当如何加载 Mach-O 文件中的数据（描述了怎样加载每个 Segment 的信息），对系统内核加载器和动态链接器起指导作用。

1. 它描述了文件中数据的具体组织结构。
2. 它也说明了进程启动后，对应的内存空间结构是如何组织的。

&emsp;load commands "specify both the logical structure of the file and the layout of the file in virtual memory". load commands “既指定文件的逻辑结构，也指定文件在虚拟内存中的布局”。 

&emsp;Mach-O 文件的 Load commands 部分对应的数据结构定义在 darwin-xnu/EXTERNAL_HEADERS/mach-o/loader.h 中：struct load_command，该结构体仅有两个成员变量：`uint32_t cmd` 和 `uint32_t cmdsize`。

&emsp;Load commands 直接跟在 mach_header 后面。所有 commands 的总大小由 mach_header 中的 sizeofcmds 字段给出。所有 load commands 的前两个字段必须是 cmd 和 cmdsize。cmd 字段用表示该 command 类型的常量填充。每个 command 类型都有一个特定的 structure。cmdsize 字段是以字节为单位的特定 load command structure 的大小，再加上它后面作为 load command 一部分的任何内容（i.e. section structures, strings, etc.）。要前进到下一个 load command，可以将 cmdsize 添加到当前 load command 的偏移量或指针中。32 位体系结构的 cmdsize 必须是 4 字节的倍数，而 64 位体系结构的 cmdsize 必须是 8 字节的倍数（这永远是所有 load commands 的最大对齐方式）。padded bytes 必须为零。目标文件中的所有表也必须遵循这些规则，以便可以对文件进行内存映射。否则，指向这些表的指针将无法在某些机器上正常工作。With all padding zeroed like objects will compare byte for byte.

```c++
struct load_command {
    uint32_t cmd;        /* type of load command */
    uint32_t cmdsize;    /* total size of command in bytes */
};
```

&emsp;cmd 字段指示 load command 的类型，cmdsize 字段主要用于计算出下一条 load command 的位置，即从本条 load command 的起始位置再偏移其 cmdsize 后便是下一条 load command 的起始位置。

#### cmdsize

&emsp;下面我们摘出 LC_SEGMENT_64(__TEXT)、LC_SEGMENT_64(__DATA)、LC_SEGMENT_64(__LINKEDIT) 三条 load command，可看到它们的前两个字段都是 cmd 和 cmdsize，然后加上后面的内容构成本条完整的 load command。然后每条 load command 的起始地址加上 command size 后的偏移是下一条 load command 的起始地址。

![截屏2021-04-20 08.58.05.png](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/5f6f96ee6c564c808d972165bfd3b9da~tplv-k3u1fbpfcp-watermark.image)

![截屏2021-04-20 08.58.15.png](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/778bb7aa82ab4dd7a7c9365c85ae49c8~tplv-k3u1fbpfcp-watermark.image)

![截屏2021-04-20 08.58.24.png](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/b9070785fe1a4900a94e6e7ad0e02575~tplv-k3u1fbpfcp-watermark.image)

&emsp;上面的起始地址 + Command Size：0x00000068 + 792 = 0x00000380 + 1112 = 0x000007D8。

#### type of load command

&emsp;cmd 字段指示 load command 的类型，在 loader.h 中定义了一系列枚举来表示不同类型的加载命令。
```c++
/* Constants for the cmd field of all load commands, the type */

// 将 segment（段）映射到进程的内存空间
#define LC_SEGMENT_64 0x19 /* 64-bit segment of this file to be mapped */

// 二进制文件 id，与符号表 uuid 对应，可用作符号表匹配
#define LC_UUID 0x1b /* the uuid */

// 加载动态链接器
#define LC_LOAD_DYLINKER 0xe /* load a dynamic linker */

// 描述在 __LINKEDIT 段的哪里找字符串表、符号表
#define LC_SYMTAB 0x2 /* link-edit stab symbol table info */

// 代码签名
#define LC_CODE_SIGNATURE 0x1d /* local of code signature */

// 其他的暂时就不一一列举了
。。。
```

#### Segment

&emsp;定义在 loader.h 中的 struct segment_command。

&emsp;segment load command 指示要将此文件的一部分映射到 task's（进程的） 地址空间中。vmsize 是内存中此 segment 的大小，可能等于或大于从该文件映射的量 filesize。文件映射从 fileoff 开始到内存段的开头 vmaddr。段的其余内存（如果有的话）按需分配并用 0 填充。segment 的最大虚拟内存保护和初始虚拟内存保护由 maxprot 和 initprot 字段指定。如果 segment 具有 sections，那么 section structures 直接遵循 segment command，其大小将反映在 cmdsize 中。

```c++
struct segment_command_64 { /* for 64-bit architectures */
    uint32_t    cmd;        /* LC_SEGMENT_64 */
    uint32_t    cmdsize;    /* includes sizeof section_64 structs */
    char        segname[16];    /* segment name */
    uint64_t    vmaddr;        /* memory address of this segment */
    uint64_t    vmsize;        /* memory size of this segment */
    uint64_t    fileoff;    /* file offset of this segment */
    uint64_t    filesize;    /* amount to map from the file */
    vm_prot_t    maxprot;    /* maximum VM protection */
    vm_prot_t    initprot;    /* initial VM protection */
    uint32_t    nsects;        /* number of sections in segment */
    uint32_t    flags;        /* flags */
};
```

+ cmd 是上面一小节的 type of load command。
+ segname[16] 段的名字，前面我们见到过 \_\_TEXT、\_\_DATA、\_\_PAGEZERO、\_\_LINKEDIT，两个下划线开头然后所有的字母都是大写。在 loader.h 中依然可以找到它们的定义。

```c++
/* The currently known segment names and the section names in those segments */

// 可执行文件捕获空指针的段 
#define SEG_PAGEZERO "__PAGEZERO" /* the pagezero segment which has no */
                                  /* protections and catches NULL */
                                  /* references for MH_EXECUTE files */
                                  
// 代码段，只读数据段 
#define SEG_TEXT "__TEXT" /* the tradition UNIX text segment */

// 数据段 
#define SEG_DATA "__DATA" /* the tradition UNIX data segment */

// 包含动态链接器所需的符号、字符串表等数据 
#define SEG_LINKEDIT "__LINKEDIT" /* the segment containing all structs */
                                  /* created and maintained by the link */
                                  /* editor.  Created with -seglinkedit */
                                  /* option to ld(1) for MH_EXECUTE and */
                                  /* FVMLIB file types only */
```

+ vmaddr 段的虚拟内存地址（未偏移），由于 ALSR，程序会在进程加上一段偏移量（slide），段的真实地址 = vm address + slide。
+ vmsize 段的虚拟内存大小。
+ fileoff 段在文件的偏移。
+ filesize 段在文件的大小。
+ nsects 段中包含多少个 section。
 
#### Section 

&emsp;定义在 loader.h 中的 struct section_64。

&emsp;一个 segment 由零个或多个 sections 组成。Non-MH_OBJECT 文件的所有 segments 中都有相应的 sections，并由 link editor 生成时填充到指定的 segment 对齐。MH_EXECUTE 和 MH_FVMLIB 格式文件的第一 segment 在其第一 section 之前包含目标文件的 mach_header 和 load commands。0 填充部分总是在其 segment （\_\_PAGEZERO）中的最后一个（在所有格式中）。这允许将归 0 的 segment 填充映射到内存中，其中可能存在 0 填充 sections。gigabyte 的 0 填充 sections，那些 section 类型为 S_GB_ZEROFILL，只能位于具有这种类型的部分的 segment 中。然后将这些 segments 放置在所有其他 segments 之后。

&emsp;MH_OBJECT 格式将其所有 sections 放在一个 segment 中以实现紧凑性。没有填充到指定的 segment 边界，并且 mach_header 和 load commands 不是该 segment 的一部分。 link editor 将合并具有相同段名称 sectname，进入相同段，segname 的段。结果 section 与合并 section 的最大对齐方式对齐，并且是新 section 的对齐方式。合并后的 sections 与合并后的 sections 中的原始对齐方式对齐。任何用于获得指定对齐的填充字节都为 0。

&emsp;mach object files 的 section 结构的 reloff 和 nreloc 字段引用的 relocation entries 的格式在头文件 <reloc.h> 中描述。

```c++
struct section_64 { /* for 64-bit architectures */
    char        sectname[16];    /* name of this section */
    char        segname[16];    /* segment this section goes in */
    uint64_t    addr;        /* memory address of this section */
    uint64_t    size;        /* size in bytes of this section */
    uint32_t    offset;        /* file offset of this section */
    uint32_t    align;        /* section alignment (power of 2) */
    uint32_t    reloff;        /* file offset of relocation entries */
    uint32_t    nreloc;        /* number of relocation entries */
    uint32_t    flags;        /* flags (section type and attributes)*/
    uint32_t    reserved1;    /* reserved (for offset or index) */
    uint32_t    reserved2;    /* reserved (for count or sizeof) */
    uint32_t    reserved3;    /* reserved */
};
```
+ segname[16] 当前 section 所在的 segment 的名字。
+ sectname[16] section 的名字。前面的学习过程中我们可能对以下几个 sections 比较眼熟了。

1. \_\_Text.\_\_text 主程序代码
2. \_\_Text.\_\_cstring c 字符串
3. \_\_Text.\_\_stubs 桩代码
4. \_\_Text.\_\_stub_helper
5. \_\_Data.\_\_data 初始化可变的数据
6. \_\_Data.\_\_objc_imageinfo 镜像信息 ，在运行时初始化时 objc_init，调用 load_images 加载新的镜像到 infolist 中
7. \_\_Data.\_\_la_symbol_ptr
8. \_\_Data.\_\_nl_symbol_ptr
9. \_\_Data.\_\_objc_classlist 类列表
10. \_\_Data.\_\_objc_classrefs 引用的类

&emsp;同样这里我们也通过几种不同的方式来查看 Test_ipa_Simple 文件中 Load commands 部分的一些详细内容。

&emsp;我们可以用 `otool -l Test_ipa_Simple` 来查看 Test_ipa_Simple 这个 Mach-O 文件中的 Load commands（加载命令）。（上面通过 Test_ipa_Simple 的 header 部分的 ncmds 字段我们知道它一共有 22 条加载命令（包含加载 \_\_PAGEZERO 段的话是 23 条加载命令），但是内容过长了这里就仅列出 Load command 0 和 Load command 1 的内容，它们两个都是 LC_SEGMENT_64） 

```c++
hmc@bogon Test_ipa_Simple.app % otool -l Test_ipa_Simple 
Test_ipa_Simple:
Load command 0 // ⬅️ 加载命令 0
      cmd LC_SEGMENT_64
  cmdsize 72
  segname __PAGEZERO // ⬅️ PAGEZERO 段
   vmaddr 0x0000000000000000
   vmsize 0x0000000100000000
  fileoff 0
 filesize 0
  maxprot ---
 initprot ---
   nsects 0
    flags (none)
Load command 1 // ⬅️ 加载命令 1
      cmd LC_SEGMENT_64
  cmdsize 792
  segname __TEXT // ⬅️ TEXT 段
   vmaddr 0x0000000100000000
   vmsize 0x0000000000008000
  fileoff 0
 filesize 32768
  maxprot r-x // ⬅️ 仅有读权限
 initprot r-x
   nsects 9 // ⬅️ 告诉我们 TEXT 段有 9 个 section 
    flags (none)
Section // ⬇️ 下面便是对 TEXT 段 9 个区的描述（0 区）
  sectname __text // ⬅️ 区名 __text 
   segname __TEXT // ⬅️ 段名 __TEXT (这里也完全对应我们上面阅读 Overview of the Mach-O Executable Format 文档中的内容)
      addr 0x000000010000621c
      size 0x00000000000001fc
    offset 25116
     align 2^2 (4)
    reloff 0
    nreloc 0
      type S_REGULAR
attributes PURE_INSTRUCTIONS SOME_INSTRUCTIONS
 reserved1 0
 reserved2 0
Section // ⬅️ (1 区)
  sectname __stubs // ⬅️ 区名 __stubs
   segname __TEXT // ⬅️ 段名 __TEXT
      addr 0x0000000100006418
      size 0x000000000000009c
    offset 25624
     align 2^2 (4)
    reloff 0
    nreloc 0
      type S_SYMBOL_STUBS
attributes PURE_INSTRUCTIONS SOME_INSTRUCTIONS
 reserved1 0 (index into indirect symbol table)
 reserved2 12 (size of stubs)
Section // ⬅️ (2 区)
  sectname __stub_helper // ⬅️ 区名 __stub_helper
   segname __TEXT // ⬅️ 段名 __TEXT
      addr 0x00000001000064b4
      size 0x00000000000000b4
    offset 25780
     align 2^2 (4)
    reloff 0
    nreloc 0
      type S_REGULAR
attributes PURE_INSTRUCTIONS SOME_INSTRUCTIONS
 reserved1 0
 reserved2 0
Section // ⬅️ (3 区)
  sectname __objc_methlist // ⬅️ 区名 __objc_methlist
   segname __TEXT // ⬅️ 段名 __TEXT
      addr 0x0000000100006568
      size 0x00000000000000bc
    offset 25960
     align 2^3 (8)
    reloff 0
    nreloc 0
      type S_REGULAR
attributes (none)
 reserved1 0
 reserved2 0
Section // ⬅️ (4 区)
  sectname __objc_methname // ⬅️ 区名 __objc_methname
   segname __TEXT // ⬅️ 段名 __TEXT
      addr 0x0000000100006624
      size 0x0000000000000d68
    offset 26148
     align 2^0 (1)
    reloff 0
    nreloc 0
      type S_CSTRING_LITERALS
attributes (none)
 reserved1 0
 reserved2 0
Section // ⬅️ (5 区)
  sectname __objc_classname // ⬅️ 区名 __objc_classname
   segname __TEXT // ⬅️ 段名 __TEXT
      addr 0x000000010000738c
      size 0x0000000000000070
    offset 29580
     align 2^0 (1)
    reloff 0
    nreloc 0
      type S_CSTRING_LITERALS
attributes (none)
 reserved1 0
 reserved2 0
Section // ⬅️ (6 区)
  sectname __objc_methtype // ⬅️ 区名 __objc_methtype
   segname __TEXT // ⬅️ 段名 __TEXT
      addr 0x00000001000073fc
      size 0x0000000000000b0f
    offset 29692
     align 2^0 (1)
    reloff 0
    nreloc 0
      type S_CSTRING_LITERALS
attributes (none)
 reserved1 0
 reserved2 0
Section // ⬅️ (7 区)
  sectname __cstring // ⬅️ 区名 __cstring
   segname __TEXT // ⬅️ 段名 __TEXT
      addr 0x0000000100007f0b
      size 0x0000000000000090
    offset 32523
     align 2^0 (1)
    reloff 0
    nreloc 0
      type S_CSTRING_LITERALS
attributes (none)
 reserved1 0
 reserved2 0
Section // ⬅️ (8 区)
  sectname __unwind_info // ⬅️ 区名 __unwind_info
   segname __TEXT // ⬅️ 段名 __TEXT
      addr 0x0000000100007f9c
      size 0x0000000000000064
    offset 32668
     align 2^2 (4)
    reloff 0
    nreloc 0
      type S_REGULAR
attributes (none)
 reserved1 0
 reserved2 0
Load command 2 // ⬇️ 其它的加载命令
...
```

&emsp;上面是加载 \_\_PAGE_ZERO 和 \_\_TEXT 两个 segment 的 Load command 的全部内容。\_\_PAGE_ZERO 是一段 “空白” 数据区，这段数据没有任何读写运行权限，方便捕捉总线错误（SIGBUS）。\_\_TEXT 则是主体代码段，我们注意到其中的 r-x，不包含 w 写权限，这是为了避免代码逻辑被肆意篡改。

&emsp;加载命令 LC_MAIN 会声明整个程序的入口地址，保证进程启动后能够正常的开始整个应用程序的运行。（我们程序的 main 函数。）

```c++
...
Load command 12 // ⬅️ 加载命令 12
       cmd LC_MAIN // ⬅️ LC_MAIN
   cmdsize 24
  entryoff 25424
 stacksize 0
Load command 13
...
```

&emsp;下面的表格我们列出 Mach-O 文件 Test_ipa_simple 中的全部 23 条 Load commands 的名字以及它们对应的段名和包含的区名。

| Load command | cmd | segname | sections | name |
| --- | --- | --- | --- | --- |
| 0 | LC_SEGMENT_64 | \_\_PAGEZERO | _ | _ |
| 1 | LC_SEGMENT_64 | \_\_TEXT | \_\_text<br>\_\_stubs<br>\_\_stub_helper<br>\_\_objc_methlist<br>\_\_objc_methname<br>\_\_objc_classname<br>\_\_objc_methtype<br>\_\_cstring<br>\_\_unwind_info | _ |
| 2 | LC_SEGMENT_64 | \_\_DATA_CONST | \_\_got<br>\_\_cfstring<br>\_\_objc_classlist<br>\_\_objc_protolist<br>\_\_objc_imageinfo | _ |
| 3 | LC_SEGMENT_64 | \_\_DATA | \_\_la_symbol_ptr<br>\_\_objc_const<br>\_\_objc_selrefs<br>\_\_objc_classrefs<br>\_\_objc_superrefs<br>\_\_objc_ivar<br>\_\_objc_data<br>\_\_data | _ |
| 4 | LC_SEGMENT_64 | \_\_LINKEDIT | _ | _ |
| 5 | LC_DYLD_INFO_ONLY | _ | _ | _ |
| 6 | LC_SYMTAB | _ | _ | _ |
| 7 | LC_DYSYMTAB | _ | _ | _ |
| 8 | LC_LOAD_DYLINKER | _ | _ | /usr/lib/dyld (offset 12) |
| 9 | LC_UUID | _ | _ | _ |
| 10 | LC_BUILD_VERSION | _ | _ | _ |
| 11 | LC_SOURCE_VERSION | _ | _ | _ |
| 12 | LC_MAIN | _ | _ | _ |
| 13 | LC_ENCRYPTION_INFO_64 | _ | _ | _ |
| 14 | LC_LOAD_DYLIB(Foundation) | _ | _ | /System/Library/Frameworks/Foundation.framework/Foundation (offset 24) |
| 15 | LC_LOAD_DYLIB(libobjc.A.dylib) | _ | _ | /usr/lib/libobjc.A.dylib (offset 24) |
| 16 | LC_LOAD_DYLIB(libSystem.B.dylib) | _ | _ | /usr/lib/libSystem.B.dylib (offset 24) |
| 17 | LC_LOAD_DYLIB(CoreFoundation) | _ | _ | /System/Library/Frameworks/CoreFoundation.framework/CoreFoundation (offset 24) |
| 18 | LC_LOAD_DYLIB(UIKit) | _ | _ | /System/Library/Frameworks/UIKit.framework/UIKit (offset 24) |
| 19 | LC_RPATH | _ | _ | _ |
| 20 | LC_FUNCTION_STARTS | _ | _ | _ |
| 21 | LC_DATA_IN_CODE | _ | _ | _ |
| 22 | LC_CODE_SIGNATURE | _ | _ | _ |

&emsp;使用 MachOView 查看的话 23 条 Load commands 是这样的。
             
![截屏2021-04-18 下午4.10.55.png](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/615183ec70fc43b8b51463a2c8b847f1~tplv-k3u1fbpfcp-watermark.image)

### Data
&emsp;至于 Data 部分，在了解了头部和加载命令后，就没什么特别可说的了。Data 是最原始的编译数据，主要是程序的指令（代码）和数据，里面包含了 Objective-C 的类信息、常量等，它们的排布完全依照 Load Commands 的描述，包含 Load commands 中提到的各个段（Segments）的数据。Load Commands 到 Data 的箭头，Data 的位置是由 Load Commands 指定的。

## 参考链接
**参考链接:🔗**
+ [MachOView工具](https://www.jianshu.com/p/2092d2d374e5)
+ [查看二进制文件](https://www.cnblogs.com/skydragon/p/7200173.html)
+ [iOS App启动优化（一）—— 了解App的启动流程](https://juejin.cn/post/6844903968837992461)
+ [了解iOS上的可执行文件和Mach-O格式](http://www.cocoachina.com/articles/10988)
+ [探秘 Mach-O 文件](http://hawk0620.github.io/blog/2018/03/22/study-mach-o-file/)
+ [Apple 操作系统可执行文件 Mach-O](https://xiaozhuanlan.com/topic/1895704362)
+ [iOS开发之runtime（11）：Mach-O 犹抱琵琶半遮面](https://xiaozhuanlan.com/topic/0328479651)
+ [iOS开发之runtime（12）：深入 Mach-O](https://xiaozhuanlan.com/topic/9204153876)
+ [Overview of the Mach-O Executable Format](https://developer.apple.com/library/archive/documentation/Performance/Conceptual/CodeFootprint/Articles/Articles/Articles/MachOOverview.html#//apple_ref/doc/uid/20001860-BAJGJEJC)
+ [iOS安全：Mach-O Type](https://easeapi.com/blog/blog/23.html)
+ [apple/darwin-xnu](https://github.com/apple/darwin-xnu) 
+ [Mac 命令 - otool](https://blog.csdn.net/lovechris00/article/details/81561627)
+ [iOS 启动优化 + 监控实践](https://juejin.cn/post/6844904194877587469)
+ [dyld背后的故事&源码分析](https://juejin.cn/post/6844903782833192968)
+ [Mac OS X ABI Mach-O File Format Reference（Mach-O文件格式参考](https://www.jianshu.com/p/f10f916a9a63)
+ [aidansteele/osx-abi-macho-file-format-reference](https://github.com/aidansteele/osx-abi-macho-file-format-reference)
+ [The Nitty Gritty of “Hello World” on macOS](https://www.reinterpretcast.com/hello-world-mach-o)
+ [深入理解MachO数据解析规则](https://juejin.cn/post/6947843156163428383)
+ [图解 Mach-O 中的 got](https://mp.weixin.qq.com/s/vt2LjEbgYsnU1ZI5P9atRw)
