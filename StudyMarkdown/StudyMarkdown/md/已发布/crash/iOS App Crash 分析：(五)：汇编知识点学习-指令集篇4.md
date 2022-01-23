# iOS App Crash 分析：(五)：汇编知识点学习-指令集篇

> &emsp;汇编语言作为机器语言（或机器指令）的助记语言应运而生，当你深入的应用它时就会发现汇编语言其实并没有那么的复杂。在一个程序所使用的机器指令中，大部分的指令所做的事情除了计算外就是将数据在寄存器与寄存器之间以及寄存器与内存之间进行移动。在高级语言中我们可以定义名字非常个性化的变量以及无限制数量的变量，而在低级语言中我们则只能使用那几个有限的寄存器来作为临时变量，以及像访问数组那样根据绝对地址/相对地址去访问内存中指定地址保存的数据。

&emsp;首先我们对指令集和 CPU 架构体系进行一个预览。

&emsp;常见的指令集有哪些：（主要是由生产 CPU 的两家公司 Intel 和 AMD 设计的）

+ Intel：
  1. x86（1978，起初是 16 位，后来为 80386 CPU 推出 IA-32：Intel Architecture 32-bit /i386 32 位）
  2. EM64T（Extended Memory 64 Technology，让现有的 x86 指令集能够执行 64 位代码，而且继续保持对 32 位代码的良好兼容。）
  3. MMX（1996.10.12 公布）（自 1996 年的 MMX 指令集以来，Intel 和 AMD 不断为 x86 体系添加新的 SIMD 指令集，包括：MMX、SSE 以及部分 AVX，它们都是对于数据的 CPU 加速处理的优化指令集。）
  4. SSE（1999.5.1 随 Pentium 3 CPU 时推出），SSE2（2000.11.1 随 Pentium 4 CPU 推出），SSE3（2004.2.1），SSSE3 (Super SSE3 2006.1.1)，SSE4.1（2006.9.27）...SSE4A（2007.11.11）
  5. AVX（2008.3.1），AVX2（2011.6.13），AVX-512，VMX 等指令集
  
  &emsp;以上均是在 x86 架构体系中使用的指令集。
  
[x86:SIMD指令集发展历程表（MMX、SSE、AVX 等）](https://blog.csdn.net/weixin_34122604/article/details/86271850?spm=1001.2101.3001.6650.1&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7ERate-1.pc_relevant_default&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7ERate-1.pc_relevant_default&utm_relevant_index=2)

+ AMD：
  1. x86
  2. x86-64（1999，AMD64/EM64T/x64）
  3. 3D-Now!（1998.1.1，重点提高了 AMD 公司 K6 系列 CPU 对 3D 图形的处理能力。）

+ ARM:
  1. armv7 
  2. armv7s
  3. arm64
  ...
  
&emsp;常见的 CPU 架构体系：

+ Inter x86（x86、i386、x86-64/x64）架构
+ ARM ARM 架构
+ IBM PowerPC 架构

&emsp;看到这里可以发现部分指令集和架构名是同名的，如 Intel 8086 CPU 它首次采用 x86 架构，运行的指令集也直接称为 x86 指令集，然后到后来的 Intel 和 AMD "合作" 把 32 位的 x86 架构扩展到 64 位推出 x86-64 架构，它执行 x86-64 指令集。所以架构和指令集的关系是什么呢？拿 x86 架构举例来说，自 1978 年 Intel 8086 CPU 首度推出以来，x86 架构也一起跟着诞生，然后直到如今，Intel 和不同厂商（AMD）也在不断推进发展 x86 架构，它们相继推出越来越强大的 x86 架构的 CPU，同时在 x86 架构下运行的指令集也不断丰富，指令集以自己的力量和角度不断增强和丰富着 CPU 的功能和性能等。起初时架构和指令集是同名的，随着后来时间的发展，后续新增的指令集都有着自己的名字例如 Intel 的 SSE、AVX 系列指令集，AMD 的 3D-Now! 指令集，那么换一种说法呢，我们也可以说在 x86（i386）架构下执行的指令集都是 x86 指令集，在 x86-64 架构下执行的指令集都是 x86-64 指令集。（也有说处理器架构主要指的是处理器使用指令集，总之我们可理解为架构和指令集是相互的强绑定关系。） 

+ 以 Intel 为例，其最初与 IBM 在 1978 年推出个人计算机使用的处理器芯片为 8086，16 位处理器，大约有 29k 个晶体管，该芯片使用的处理器架构即为最初的 x86 指令集。
+ 随后因为由于 Intel 的 16 位处理器其性能无法应对 AMD 同时代推出的处理器，在此基础上推出了 i386 处理器，也叫做 80386，指令集依然是 x86 指令集，不同的是 i386 处理器是 32 位处理器，具有 275K 个晶体管。
+ Intel 1989 年又相继推出了 i486 ，在其 386 基础上增加了晶体管数量，提高了浮点数性能，其指令集也是 x86 指令集。
+ 随后 Intel 在 1993 年便进入奔腾时代 (Pentium)，也是 Intel 第一个正式 CPU 品牌，奔腾处理器完全向下兼容，其处理器架构也是以 x86 为基础 ，主要在晶体管数量和主频上有了大幅度提高，奔腾处理器 12 年的产品周期内一共有 6 代处理器，按照发布时间分别为 奔腾(1993年)，奔腾 Pro(1995年), 奔腾/MMX (1997年), 奔腾2(1997年)，奔腾3(1999年), 奔腾4(2000年)，奔腾4E (2004年)。
+ 2016 年 Intel 开始进入酷睿时代，典型的消费级产品为大众所熟知的 Core i3, Core i5, Core i7, Core i9 ，这些产品依然采用 x86/x86-64 架构，并增加了多核心处理器，超线程技术，补充了更多的指令处理复杂运算，这些处理器以及 AMD 的同代产品速龙(2003 年上市)，锐龙(2017 年上市) 其核心指令集一直没有太大变化，x86 一直是霸占桌面处理器市场的存在。

&emsp;[什么是i386,x86和x64, 一文了解处理器架构](https://baike.baidu.com/tashuo/browse/content?id=5d5bb1f8a73bd4495d7b21a5&lemmaId=433177&fromLemmaModule=pcBottom&lemmaTitle=Intel%2080386)

&emsp;下面我们对上面涉及到的一些名词进行学习。

## 指令集（指令系统）概述

&emsp;**在计算机中，指示计算机硬件执行某种运算、处理功能的命令称为指令。指令是计算机运行的最小的功能单位，而硬件的作用是完成每条指令规定的功能。程序就是一系列按一定顺序排列的指令，执行程序的过程就是计算机的工作过程。**

&emsp;指令集是 CPU 中用来计算和控制计算机系统的一套指令的集合，而每一种新型的 CPU 在设计时就规定了一系列与其它硬件电路（显卡、网卡、声卡等）相配合的指令系统，而指令集的先进与否，也关系到 CPU 的性能发挥，它也是 CPU 性能体现的一个重要标志。指令的强弱也是 CPU 的重要指标，指令集是提高微处理器效率的最有效的工具之一。从现阶段的主流体系结构讲，指令集可分为复杂指令集（x86）和精简指令集（ARM）两部分。

&emsp;计算机的指令系统是指一台计算机上全部指令的集合，也称计算机的指令集。指令系统包括指令格式、寻址方式和数据形式。一台计算机的指令系统反映了该计算机的全部功能，机器类型不同，其指令系统也不同，因而功能也不同。指令系统的设置和机器的硬件结构密切相关，一台计算机要有较好的性能，必须设计功能齐全、通用性强、内含丰富的指令系统，这就需要复杂的硬件结构来支持。

&emsp;常见的指令集有：

+ Intel 推出的 x86，EM64T，MMX，SSE（随 Pentium 3 CPU 时推出），SSE2（随 Pentium 4 CPU 推出），SSE3，SSSE3 (Super SSE3)，SSE4A，SSE4.1，SSE4.2，AVX，AVX2，AVX-512，VMX 等指令集；
+ AMD 推出的 x86，x86-64，3D-Now!（重点提高了 AMD 公司 K6 系列 CPU 对 3D 图形的处理能力）指令集。

&emsp;[指令集](https://baike.baidu.com/item/指令集)

### x86/x86-64 指令集

&emsp;x86 指令集是 Intel 为其第一块 16 位 CPU(i8086) 专门开发的，IBM 1981 年推出的世界第一台 PC 机中的 CPU—i8088（i8086 简化版）使用的也是 x86 指令集，同时电脑中为提高浮点数据处理能力而增加的 x87 芯片系列数学协处理器则另外使用 x87 指令集。（Intel 推出 CPU，IBM 推出搭载该 CPU 的 PC 机。）

&emsp;x86 是当前最成功的 CPU 架构/指令集。以后就将 x86 指令集和 x87 指令集统称为 x86 指令集。虽然随着 CPU 技术的不断发展，Intel 陆续研制出更新型的 i80386、i80486，但为了保证电脑能继续运行以往开发的各类应用程序以保护和继承丰富的软件资源，所以 Intel 公司所生产的所有 CPU 仍然继续使用 x86 指令集，所以它的 CPU 仍属于 x86 系列。

&emsp;由于 Intel x86 系列及其兼容 CPU 都使用 x86 指令集，所以就形成了庞大的 x86 系列及兼容 CPU 阵容。

&emsp;到 2002 年，由于 32 位特性的长度，x86（IA-32）架构开始到达某些设计上的极限，例如要处理大量的信息储存时大于 4GB 会有困难。Intel 原本已经决定在 64 位的时代完全地舍弃 x86（IA-32）兼容性，推出新的架构称为 IA-64 技术作为它的 Itanium 处理器产品线的基础。IA-64 与 x86（IA-32） 的软件天生不兼容；它使用各种模拟形式来运行 x86（IA-32）的软件，不过，以模拟方式来运行的效率十分低下，并且会影响其他程序的运行。AMD 公司则主动把 32 位 x86（IA-32）扩充为 64 位。它以一个称为 AMD64 的架构出现（在重命名前也称为 x86-64），且以这个技术为基础的第一个产品是单内核的 Opteron 和 Athlon 64 处理器家族。由于 AMD 的 64 位处理器产品线首先进入市场，且微软也不愿意为 Intel 和 AMD 开发两套不同的 64 位操作系统，Intel 也被迫采纳 AMD64 指令集且增加某些新的扩充到他们自己的产品，命名为 EM64T 架构（显然他们不想承认这些指令集是来自它的主要对手），EM64T 后来被 Intel 正式更名为 Intel 64(也就是 x64 指令集)。

&emsp;看到这里可以发现，即使 16 位和 32 位 x86 架构和指令集都是 Intel 推出的，但是在 64 位的 x86 架构 "标准" 制定中，其它厂商（AMD）依然可以有自己的声音。

### RISC（精简指令集）/CISC（复杂指令集）

&emsp;所谓指令的复杂度就是指 CPU 指令集中所提供的指令的数量、指令寻址模式、指令参数、以及 CPU 内部的架构设计的复杂度、以及指令本身所占据的字节数等来进行划分的一种方式。

&emsp;一般有两种类型的分类：复杂指令集、精简指令集。（指令本身所占据的字节数是个重点，x86-64 架构体系指令长度是变长的从 1 个到 15 个字节不等，而 ARM64 架构体系指令的长度是固定的 4 个字节，这里为后续以不同的方式回溯函数调用堆栈埋下伏笔。）

1. CISC 指令集。CISC 的英文全称为 "Complex Instruction Set Computer"，即 "复杂指令系统计算机"，从计算机诞生以来，人们一直沿用 CISC 指令集方式。早期的桌面软件是按 CISC 设计的，并一直沿续到现在。目前，桌面计算机流行的 x86 架构体系即使用 CISC。在 CISC 微处理器中，程序的各条指令是按顺序串行执行的，每条指令中的各个操作也是按顺序串行执行的。顺序执行的优点是控制简单，但计算机各部分的利用率不高，执行速度慢。CISC 架构的服务器主要以 x86/x64 架构（Intel Architecture）为主，而且多数为中低档服务器所采用。

2. RISC 指令集。RISC 的英文全称为 "Reduced Instruction Set Computer"，即 "精简指令集计算机"。相比 CISC（复杂指令集）而言，RISC 的指令格式统一，种类比较少，寻址方式也比复杂指令集少，这样一来，它能够以更快的速度执行操作。使用 RISC 指令集的体系结构主要有 ARM、MIPS。RISC 起源于 80 年代的 MIPS 主机(即 RISC 机)，RISC 机中采用的微处理器统称 RISC 处理器。目前的智能移动设备中的 CPU 几乎都采用 RISC 指令集，比较有代表的就是 ARM 指令集和 POWER-PC 指令集。

### ARM 指令集

&emsp;1978 年 12 月 5 日，物理学家赫尔曼·豪泽（Hermann Hauser）和工程师 Chris Curry，在英国剑桥创办了 CPU 公司（Cambridge Processing Unit），主要业务是为当地市场供应电子设备。1979 年，CPU 公司改名为 Acorn 公司。起初，Acorn 公司打算使用摩托罗拉公司的 16 位芯片，但是发现这种芯片太慢也太贵。"一台售价 500 英镑的机器，不可能使用价格 100 英镑的 CPU！"，他们转而向 Intel 公司索要 80286 芯片的设计资料，但是遭到拒绝，于是被迫自行研发。

&emsp;1985 年，Roger Wilson 和 Steve Furber 设计了他们自己的第一代 32 位、6M Hz 的处理器，并用它做出了一台 RISC 指令集的计算机，简称 ARM（Acorn RISC Machine），这就是 ARM 这个名字的由来。ARM 处理器本身是 32 位设计，但也配备 16 位指令集（Thumb）。

&emsp;ARM 处理器是英国 Acorn 有限公司设计的低功耗成本的第一款 RISC 微处理器，全称为 Advanced RISC Machine。ARM 微处理器在较新的体系结构中支持两种指令集：ARM 指令集和 Thumb 指令集，其中，ARM 指令为 32 位的长度，Thumb 指令为 16 位长度。Thumb 指令集为 ARM 指令集的功能子集，但与等价的 ARM 代码相比较，可节省 30%～40% 以上的存储空间，同时具备 32 位代码的所有优点。[ARM （ARM处理器）](https://baike.baidu.com/item/ARM/7518299)

## CPU 架构

&emsp;CPU 架构是 CPU 厂商给属于同一系列的 CPU 产品定的一个规范，主要目的是为了区分不同类型的 CPU。市面上的 CPU 分类主要分有两大阵营，一个是 Intel、AMD 为首的复杂指令集 CPU，另一个是以 IBM、ARM 为首的精简指令集 CPU。两个不同品牌的 CPU，其产品的架构也不相同，例如：Intel、AMD 的 CPU 是 x86 架构的，而 IBM 公司的 CPU 是 PowerPC 架构，ARM 公司是 ARM 架构。[处理器架构](https://baike.baidu.com/item/处理器架构/8535061)

### Intel x86（架构）

&emsp;Intel 8086 是英特尔公司于 1978 年推出的 16 位微处理器，x86 架构也是随 Intel 8086 微处理器首度出现。

&emsp;**x86（x86 架构）泛指一系列基于 Intel 8086 且向后兼容的中央处理器指令集架构。** Intel 8086 在三年后为 IBM PC 所选用，之后 x86 便成为了个人计算机的标准平台，成为了历来最成功的 CPU 架构。

&emsp;Intel 在早期以 80x86 这样的数字格式来命名处理器，包括 Intel 8086、80186、80286、80386 以及 80486，由于以 "86" 作为结尾，因此其架构被称为 "x86"。由于数字并不能作为注册商标，因此 Intel 及其竞争者均在后续新一代处理器中使用可注册的名称，例如：奔腾系列（Pentium）、酷睿系列（Core）、锐龙系列（Ryzen，由 AMD 推出）。

&emsp;虽然 x86 架构由 Intel 公司推出，但是 Intel 之外其他公司也有制造 x86 架构的处理器，其中最成功的制造商为 AMD，其早先产品 Athlon 系列处理器的市场份额仅次于 Intel Pentium。

&emsp;Intel 8086 是 16 位处理器，直到 1985 年 32 位的 80386 的开发，这个架构都维持是 16 位。接着一系列的处理器表示了 32 位架构的细微改进，推出了数种的扩充，直到 2003 年 AMD 对于这个架构发展了 64 位的扩充，并命名为 AMD64。后来 Intel 也推出了与之兼容的处理器，并命名为 Intel 64。两者一般被统称为 x86-64 或 x64，开创了 x86 的 64 位时代。

> &emsp;x86 的 32 位架构一般又被称作 IA-32、i386（随 Intel 80386 CPU 推出），全名为 "Intel Architecture, 32-bit"。其 64 位架构由 AMD 率先推出，并被称为 "AMD64"。之后也被 Intel 采用，被其称为 "Intel 64"。一般也被称作 "x86-64"、"x64"。[X86架构](https://baike.baidu.com/item/X86架构/7470217)

### MIPS 架构 

&emsp;MIPS 架构（英语：MIPS architecture，为 Microprocessor without interlocked piped stages architecture 的缩写），是一种采取精简指令集（RISC）的处理器架构，1981 年出现，由 MIPS 科技公司开发并授权，广泛被使用在许多电子产品、网络设备、个人娱乐装置与商业装置上。最早的 MIPS 架构是 32 位，最新的版本已经变成 64 位。

&emsp;目前国内的龙芯 CPU，采用的就是 MIPS 指令集。[MIPS](https://baike.baidu.com/item/MIPS)

## 按指令流和数据流对 CPU 体系进行划分

&emsp;前面我们提到 Intel 和 AMD 不断为 x86 体系添加新的 SIMD 指令集，包括：MMX、SSE 以及部分 AVX，它们都是对于数据的 CPU 加速处理的优化指令集。其中 SIMD 全称 Single Instruction Multiple Data，即单指令多数据，采用一个指令流处理多个数据流，此外还有 SISD、MISD、MIMD。 

&emsp;按指令流和数据流来进行分类的依据是 CPU 的一条指令可以同时处理多少条数据，或者一条数据同时被多少条指令处理，以及在一个 CPU 时间周期内可以同时执行多少条指令等规则来划分的。因此可以划分为如下四种：

+ 单指令流单数据流机器（SISD）
  SISD 机器是一种传统的串行计算机，它的硬件不支持任何形式的并行计算，所有的指令都是串行执行。并且在某个时钟周期内，CPU 只能处理一个数据流。因此这种机器被称作单指令流单数据流机器。早期的计算机都是 SISD 机器，如冯诺.依曼架构，如 IBM PC 机，早期的巨型机和许多 8 位的家用机等。

+ 单指令流多数据流机器（SIMD）
  SIMD 是采用一个指令流处理多个数据流。这类机器在数字信号处理、图像处理、以及多媒体信息处理等领域非常有效。Intel 处理器实现的 MMXTM、SSE（Streaming SIMD Extensions）、SSE2 及 SSE3 扩展指令集，都能在单个时钟周期内处理多个数据单元。也就是说我们现在用的单核计算机基本上都属于 SIMD 机器。

+ 多指令流单数据流机器（MISD）
  MISD 是采用多个指令流来处理单个数据流。由于实际情况中，采用多指令流处理多数据流才是更有效的方法，因此 MISD 只是作为理论模型出现，没有投入到实际应用之中。

+ 多指令流多数据流机器（MIMD）
  MIMD 机器可以同时执行多个指令流，这些指令流分别对不同数据流进行操作。最新的多核计算平台就属于 MIMD 的范畴，例如 Intel 和 AMD 的双核处理器等都属于 MIMD。
  
> &emsp;[1.1.2 多核CPU硬件架构介绍](https://book.51cto.com/art/201004/197196.htm)
  
&emsp;以加法指令为例，单指令单数据（SISD）的 CPU 对加法指令译码后，执行部件先访问内存，取得第一个操作数，之后再一次访问内存，取得第二个操作数，随后才能进行求和运算。而在 SIMD 型的 CPU 中，指令译码后几个执行部件同时访问内存，一次性获得所有操作数进行运算。这个特点使 SIMD 特别适合于多媒体应用等数据密集型运算。如：AMD 公司引以为豪的 3D NOW! 技术实质就是 SIMD，这使 K6-2、雷鸟、毒龙处理器在音频解码、视频回放、3D 游戏等应用中显示出优异的性能。

&emsp;上面一些 CPU、CPU 架构、CPU 指令集的知识铺垫完成了，接下来我们继续学习汇编语言。

## 





## 参考链接
**参考链接:🔗**
+ [Intel x86](https://baike.baidu.com/item/Intel%20x86/1012845?fromtitle=x86&fromid=6150538)
+ [x86:SIMD指令集发展历程表（MMX、SSE、AVX 等）](https://blog.csdn.net/weixin_34122604/article/details/86271850?spm=1001.2101.3001.6650.1&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7ERate-1.pc_relevant_default&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7ERate-1.pc_relevant_default&utm_relevant_index=2)
+ [什么是i386,x86和x64, 一文了解处理器架构](https://baike.baidu.com/tashuo/browse/content?id=5d5bb1f8a73bd4495d7b21a5&lemmaId=433177&fromLemmaModule=pcBottom&lemmaTitle=Intel%2080386)
+ [深入iOS系统底层系列文章目录](https://www.jianshu.com/p/139f0899335d)
+ [iOS逆向 ：初识汇编](https://www.jianshu.com/p/139f0899335d)
+ [iOS Crash/崩溃/异常 堆栈获取](https://www.jianshu.com/p/8ece78d71b3d)
+ [iOS堆栈信息解析（函数地址与符号关联）](https://www.jianshu.com/p/df5b08330afd)
+ [谈谈iOS堆栈那些事](https://joey520.github.io/2020/03/15/谈谈msgSend为什么不会出现在堆栈中/)
+ [iOS 的崩溃捕获-堆栈符号化-崩溃分析](https://www.jianshu.com/p/302ed945e9cf)
+ [函数栈的实现原理](https://segmentfault.com/a/1190000017151354)
+ [函数调用栈 剖析＋图解[转]](https://www.jianshu.com/p/78e01e513120)
+ [[转载]C语言函数调用栈](https://www.jianshu.com/p/c89d243b8276)
+ [BSBackTracelogger学习笔记](https://juejin.cn/post/6910791727670362125)
+ [汇编过程调用是怎样操作栈的？](https://www.zhihu.com/question/49410551/answer/115870825)
+ [ARM三级流水线](https://blog.csdn.net/qq_34127958/article/details/72791382)



## 任务

1. Shell/Python/JavaScript 脚本语言学习。
2. SwiftUI 和 Flutter 学习。
3. [ios-crash-dump-analysis-book/zh](https://faisalmemon.github.io/ios-crash-dump-analysis-book/zh/)
4. Mach 内核/汇编语言/逆向 三本书要读。
5. 汇编语言/函数调用栈回溯。
