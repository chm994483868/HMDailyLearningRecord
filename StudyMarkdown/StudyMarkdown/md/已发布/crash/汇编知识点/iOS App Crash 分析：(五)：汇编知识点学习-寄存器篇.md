# 汇编知识点学习临时文件（上面的那个文件坏了）

&emsp;本来本篇标题是《iOS App Crash 分析：(五)：函数堆栈获取解析》的，但是在整理汇编知识的过程中发现要复习的内容较多，所以现在修改为《iOS App Crash 分析：(五)：汇编知识点学习-寄存器篇》专注于之前学习的汇编知识点的巩固复习。

&emsp;汇编语言是使用 **助记符代替机器指令** 的一种编程语言，汇编指令和机器指令是一一对应的关系，所以理论上拿到二进制就可以进行反汇编。

&emsp;虽然我们日常开发大部分情况下不需要直接编写汇编指令，但是能看懂汇编指令能分析对应的代码逻辑对我们理解计算机运行逻辑还是有极大促进作用的（内功）。特别是当我们解决 Crash 问题时，利用汇编调试技巧进行反汇编更易使我们定位到问题根源。任何高级语言最终都会被编译成汇编，学习了解汇编的相关知识，可以更好的日常开发、学习探索中帮助我们更好的排查问题、理解底层运行的机制。
 
&emsp;学习函数调用栈相关的内容之前我们需要了解汇编相关的三个重要概念：寄存器、堆栈、指令集，其中寄存器、指令集在不同的架构下有不同的名字，但是基本概念都是一致的，这里我们以 x86 和 arm64 为例来学习。

## 寄存器概述

&emsp;在高级语言，如 Objective-C、C 和 C++ 里，操作对象是变量；在 ARM 汇编里，操作对象是寄存器（register）、内存和栈（stack）。其中寄存器可以看成 CPU 自带的变量，它们的数量一般是很有限的；当需要更多变量时，就可以把它们存放在内存中；不过，数量上去了，质量也下来了，对内存的操作比对寄存器的操作要慢的多。[iOS 应用逆向工程](http://product.m.dangdang.com/23674854.html)

&emsp;在一个程序的机器指令中，大部分的指令代码所做的事情除了计算外就是将数据在寄存器与寄存器之间以及寄存器与内存之间进行移动。在高级语言中我们可以定义非常个性化的变量以及无限制的变量，而在低级语言中我们则只能使用那几个有限的寄存器来作为临时变量，以及像访问数组那样去访问内存地址。[深入iOS系统底层之汇编语言](https://www.jianshu.com/p/ff8ed52bdd67)

&emsp;寄存器是 CPU 内部用来存放数据的一些小型存储器，用来暂时存放参与运算的数据和运算结果以及指令地址。（其实寄存器就是一种常用的时序逻辑电路，但这种时序逻辑电路只包含存储电路。寄存器的存储电路是由锁存器或触发器构成的，因为一个锁存器或触发器能存储 1 位二进制数，所以由 N 个锁存器或触发器可以构成 N 位寄存器。）

&emsp;在计算机领域，寄存器是 CPU 内部的元件，按功能划分包括：通用寄存器、专用寄存器和控制寄存器。寄存器拥有非常高的读写速度，所以在寄存器之间的数据传送非常快。寄存器有串行和并行两种数码存取方式。将 N 位二进制数一次存入寄存器或从寄存器中读出的方式称为并行方式。将 N 位二进制数以每次 1 位，分成 N 次存入寄存器或从寄存器读出，这种方式称为串行方式。并行方式只需一个时钟脉冲就可以完成数据操作，工作速度快，但需要 N 根输入和输出数据线。串行方式要使用几个时钟脉冲完成输入或输出操作，工作速度慢，但只需要一根输入或输出数据线，传输线少，适用于远距离传输。[寄存器-百度百科](https://baike.baidu.com/item/寄存器/187682?fr=aladdin)

## 不同类型架构下寄存器都有哪些

&emsp;在看具体的寄存器列表之前，我们先学习一下 LLDB 中的 register 命令。

### LLDB register 命令

&emsp;通过 `help register` 命令查看一下 `register` 命令的帮助信息。

```c++
(lldb) help register
Commands to access registers for the current thread and stack frame.

Syntax: register [read|write] ...

The following subcommands are supported:

      read  -- Dump the contents of one or more register values from the current frame.  If no register is specified, dumps them all.
      write -- Modify a single register value.

For more help on any particular subcommand, type 'help <command> <subcommand>'.
```

&emsp;register 命令用于读取或者修改当前时刻下当前线程的栈帧中寄存器存储的内容。它有两条子命令：读/写

+ read：从当前 frame Dump 一个或多个寄存器中存储的内容。如果未指定寄存器，则把它们全部 Dump 出来。直白一点理解就是读取寄存器中存储的内容直接打印出来。
+ write：修改指定寄存器中存储的内容。

&emsp;CPU 中最主要的部件是寄存器，可以通过改变寄存器的内容来实现对 CPU 的控制，不同 CPU，寄存器的个数和结构是不相同的。

&emsp;下面看一下 read/write 两个子命令后面都可以使用哪些选项：

```c++
(lldb) help regist read
Dump the contents of one or more register values from the current frame.  If no register is specified, dumps them all.

Syntax: register read <cmd-options> [<register-name> [<register-name> [...]]]

Command Options Usage:
  register read [-A] [-f <format>] [-G <gdb-format>] [-s <index>] [<register-name> [<register-name> [...]]]
  register read [-Aa] [-f <format>] [-G <gdb-format>] [<register-name> [<register-name> [...]]]

       -A ( --alternate ) Display register names using the alternate register name if there is one.

       -G <gdb-format> ( --gdb-format <gdb-format> ) Specify a format using a GDB format specifier string.

       -a ( --all ) Show all register sets.

       -f <format> ( --format <format> ) Specify a format to be used for display.

       -s <index> ( --set <index> ) Specify which register sets to dump by index.
     
     This command takes options and free-form arguments.  
     If your arguments resemble option specifiers (i.e., they start with a - or --), you must use ' -- ' between the end of the command options and the beginning of the
     arguments.
```

&emsp;-A/-a 选项都是打印出当前所有寄存器中存储的内容，其中 -A 使用 alternate 寄存器名称（如果有）显示寄存器名称，-a 则是显示寄存器的原始名字。

```c++
(lldb) help register write
Modify a single register value.

Syntax: register write <register-name> <value>
```

&emsp;register write 修改指定寄存器中存储的内容。

### 使用 LLDB register 命令查看不同平台的寄存器

&emsp;然后我们在 ARM64 和 x86 平台下看一下它们都有哪些寄存器。我们分别选择以模拟器（x86）或真机（ARM64）进入 LLDB 调试模式，然后在 Xcode 控制台使用 register read 命令查看打印内容。

#### ARM64 架构寄存器列表

&emsp;已知 iPhone 系列都是 ARM 架构的 CPU，且自 iPhone 5s 搭载的 64 位 A7 处理器以来开始全面进入 64 位的 arm64 架构。

```c++
- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
}
```

&emsp;这里我们使用 viewDidLoad 函数为示例代码，下面学习指令集时也以 viewDidLoad 函数为示例代码，在 viewDidLoad 函数处打一个断点，选中真机（iPhone X）然后运行程序，断点命中后进入 LLDB 调试模式，使用 `register read --all` 命令打印所有寄存器，可看到其中包括 General Purpose Registers、Floating Point Registers、Exception State Registers 三个分组的寄存器列表。

&emsp;浮点和向量寄存器：因为浮点数的存储以及其运算的特殊性，CPU 中专门提供浮点寄存器来处理浮点数。现在 CPU 支持向量运算（向量运算在图形处理相关的领域用的非常多），为了支持向量计算，系统也提供了众多的向量寄存器。

&emsp;这里我们只专注于 General Purpose Registers 分组，看到 ARM64 下有：

+ x0-x28（数据地址寄存器）
+ fp（x29 frame pointer 栈底寄存器）
+ lr（x30 link register 链接寄存器）
+ sp（x31 stack pointer 栈顶寄存器）
+ pc（x32 program counter 程序计数器）
+ cpsr（x33 状态寄存器 32 bit，上面的都是 64 bit）

&emsp;共 33 个 64 bit 和 1 个 32 bit 的 General Purpose Registers，w0-w28 表示 x0-x28 的低 32 bit。

&emsp;通用寄存器也称为数据地址寄存器。通常用来做数据计算的临时存储、累加、计数、地址保存等功能。定义这些寄存器的作用主要是用于在 CPU 指令中保存操作数，在 CPU 中当做一些常规变量来使用。

&emsp;w0-w28 表示 x0-x28 的低 32 位，因为 64 位 CPU 可以兼容 32 位，所以可以只使用 64 位寄存器的低 32 位。在 ARM64 架构下寄存器以 x 开头访问时是完整的 64 位，使用 w 开头访问时是低 32 位。

&emsp;通常，CPU 会先将内存中的数据存储到通用寄存器中，然后再对寄存器中的数据进行运算，假设内存中有块红色内存空间的值是 3，现在想把它的值加 1，并将结果存储到蓝色内存空间，CPU 首先会将红色内存空间的值放到 x0 寄存器中：mov x0, 红色内存空间
，然后让 x0 寄存器与 1 相加：add X0, 1，最后将值赋值给内存空间：mov 蓝色内存空间, x0。

```c++
(lldb) register read --all
General Purpose Registers:
        x0 = 0x00000001，02d08360  // w0 = 0x02d08360
        x1 = 0x00000001，9b043c57  // w1 = 0x9b043c57
        x2 = 0x00000000，00000001  // w2 = 0x00000001
        x3 = 0x00000001，6d59db90  // w3 = 0x6d59db90
        x4 = 0x00000000，00000010  // w4 = 0x00000010
        x5 = 0x00000000，00000020  // w5 = 0x00000020
        x6 = 0x00000001，6d59d890  // w6 = 0x6d59d890
        x7 = 0x00000000，00000000  // w7 = 0x00000000
        x8 = 0x00000001，9b043000  // w8 = 0x9b043000
        x9 = 0x00000000，00000000  // w9 = 0x00000000
       x10 = 0x00000000，0000002f  // w10 = 0x0000002f
       x11 = 0x00000001，03023258  // w11 = 0x03023258
       x12 = 0x00000000，00000025  // w12 = 0x00000025
       x13 = 0x00000000，00000000  // w13 = 0x00000000
       x14 = 0x00000000，00000000  // w14 = 0x00000000
       x15 = 0xffffffff，ffffffff  // w15 = 0xffffffff
       x16 = 0x00000001，0286d2b2  (void *)0x78e0000000010286  // w16 = 0x0286d2b2
       x17 = 0x00000001，02865f60  TEST_MENU`-[ViewController viewDidLoad] at ViewController.m:16 // w17 = 0x02865f60
       x18 = 0x00000000，00000000  // w18 = 0x00000000
       x19 = 0x00000001，02d08360  // w19 = 0x02d08360
       x20 = 0x00000000，00000000  // w20 = 0x00000000
       x21 = 0x00000001，f2d9c000  (void *)0x000000019efcba50  // w21 = 0xf2d9c000
       x22 = 0x00000001，9ba00157  // w22 = 0x9ba00157
       x23 = 0x00000001，9af2039d  // w23 = 0x9af2039d
       x24 = 0x00000000，00000000  // w24 = 0x00000000
       x25 = 0x00000001，f3987000  UIKitCore`_UIWindowSceneActivationSettings._pinchActivationScaleThreshold  // w25 = 0xf3987000
       x26 = 0x00000001，02d06280  // w26 = 0x02d06280
       x27 = 0x00000001，9b639957  // w27 = 0x9b639957
       x28 = 0x00000001，f84cf4f0  CoreFoundation`__NSArray0__struct  // w28 = 0xf84cf4f0
       
        fp = 0x000000016d59da00
        lr = 0x00000001830f26c8  UIKitCore`-[UIViewController _sendViewDidLoadWithAppearanceProxyObjectTaggingEnabled] + 104
        sp = 0x000000016d59d9e0
        pc = 0x0000000102865f74  TEST_MENU`-[ViewController viewDidLoad] + 20 at ViewController.m:17:5
      cpsr = 0x40000000
      
Floating Point Registers:
        ...

Exception State Registers:
       ...

```

&emsp;然后我们再使用 register read --alternate/register read -A 命令打印，看一下寄存器的备用名（alternate register name）。

```c++
(lldb) register read --alternate
General Purpose Registers:
      arg1 = 0x0000000102d08360
      arg2 = 0x000000019b043c57  
      arg3 = 0x0000000000000001
      arg4 = 0x000000016d59db90
      arg5 = 0x0000000000000010
      arg6 = 0x0000000000000020
      arg7 = 0x000000016d59d890
      arg8 = 0x0000000000000000
      ...
```

&emsp;可看到我们听过了很多次的说 x0-x7 是参数寄存器，这里也得到了印证 x0-x7 寄存器的备用名正是 arg1-arg8，如下我们直接读取 x0/arg1 打印的都是 x0，然后我们打印 self，看到其值也正是 x0 寄存器中保存的值，然后打印 `_cmd` 的值是 viewDidLoad，然后把 x1 寄存器的值强转为 SEL 后看到的也是 viewDidLoad，即当前 x0 x1 寄存器中存的值正是函数的前两个参数。

```c++
(lldb) register read x0
      x0 = 0x0000000102d08360
(lldb) register read arg1
      x0 = 0x0000000102d08360
(lldb) p self
(ViewController *) $0 = 0x0000000102d08360
(lldb) p _cmd
(SEL) $1 = "viewDidLoad"
(lldb) p (SEL)0x000000019b043c57
(SEL) $2 = "viewDidLoad"
```

&emsp;下面我们挑选 ARM64 中比较重要的寄存器来分析其功能/作用。(注意这里是 ARM64 架构，和 x86 架构是有一些重要区别的，例如当前函数结束后的返回地址，ARM64 下是保存在 lr 寄存器内，但是 lr 寄存器只有一个，连续函数调用的话会被覆盖，所有每次函数调用还是会把下一条指令的地址保存在栈低，而 x86 则是没有对应的 lr 寄存器，函数调用时则是直接保存在栈底。)

##### fp/sp

+ fp（x29 frame pointer 栈底寄存器），通常用作帧指针 fp（frame pointer），栈帧基址寄存器，指向当前函数栈帧的栈底。
+ sp（x31 stack pointer 栈顶寄存器，和 fp 对应，一个是栈底一个是栈顶，要牢记它们两个）在计算机科学中栈是非常重要的术语。sp 寄存器存放了一个指向栈顶的指针。

&emsp;栈的概念，在学习数据结构的时候就已经有了解，栈是一块具有后进先出功能的存储区域，在进行操作时我们总是只能将数据压入栈顶，或者将数据从栈顶弹出来。要维护一个栈区域就必须要提供 2 个寄存器，一个寄存器用来保存栈的基地址也就是栈的底部，而一个寄存器则用来保存栈的偏移也就是栈的顶部。

&emsp;在一般的系统中，我们都将栈的基地址设置在内存的高位，而将栈顶地址设置在内存的低位。因此每当有进栈操作时则将栈顶地址进行递减，而当有出栈操作时则将栈顶地址递增。栈的这种特性，使得它非常适合于保存函数中定义的局部变量，以及函数内调用函数的情况。在 x86 体系的 CPU 中，提供了一个专门的 rbp 寄存用来保存栈的基地址，同时提供一个专门的 rsp 寄存器来保存栈的栈顶地址，而 arm64 体系的 CPU 中则用 x29(fp) 寄存器来保存栈的基地址，同时提供一个 x31(sp) 寄存器来保存栈的栈顶地址。

##### lr

+ lr（x30 link register 链接寄存器），它存储的数据是方法 Caller 执行的最后一行指令的下一行的地址，它的作用也很好理解：当 Callee 执行完了之后要返回 Caller 继续向下执行，但是 CPU 要如何知道继续执行哪一行指令呢？正是靠 lr 寄存器保存了下一条指令的地址，Caller 方法/函数才能得以正常顺序继续执行。

##### pc

+ pc（x32 program counter 程序计数器、指令寄存器，不要被它的名字迷惑了，它可不是用来计数的），pc 寄存器里面保存的值是 CPU 接下来要执行的指令的地址。在 ARM64 中，进程是不能任意改写 pc 寄存器中的值的，它存放了下一条要执行的指令的地址。

> 我们知道程序代码是保存在内存中的，那 CPU 又是如何知道要执行哪一条保存在内存中的指令呢？这就是通过指令寄存器来完成的。
> 因为内存中的指令总是按线性序列保存的，CPU 只是按照编制好的程序来执行指令。因此 CPU 内提供一个指令寄存器来记录 CPU 下一条将要执行的指令的内存地址，这样每次执行完毕一条指令后，CPU 就根据指令寄存器中所记录的地址到内存中去读取指令并执行，同时又将下一条指令的内存地址保存到指令寄存器中，就这样就重复不断的处理来完成整个程序的执行。
>
> 但是这里面有两问题：
> 
> 前面不是说 CPU 内有高速缓存吗？怎么又说每次都去访问内存呢？而且保存还是内存的地址呢。 
> 这是没有问题的，指令寄存器中保存的确实是下一条指令在内存中的地址，但是操作系统除了将部分内存区域中的指令保存到高速缓存外还会建立一个内存地址到高速缓存地址之间的映射关系数据结构。因此即使是指令寄存器中保存的是内存地址，但是在指令真实执行时 CPU 就会根据指令寄存器中的内存地址以及内部建立的内存和高速缓存的映射关系来转化为指令在高速缓存中的地址来读取指令并执行。当然如果发现指令并不在高速缓存中时，CPU 就会触发一个中断并告诉操作系统，操作系统再根据特定的策略从内存中再次读取一块新的内存数据到高速缓存中，并覆盖掉原先保存在高速缓存中的内容，然后 CPU 再次读取高速缓存中的指令后继续执行。
> 如果说指令寄存器每次都是保存的顺序执行指令的话那么怎么去实现跳转逻辑呢？ 
> 答案是跳转指令和函数调用指令的存在。我们的用户态中的代码不能去人为的改变指令寄存器的值，也就是不能对指令寄存器进行赋值，因此默认情况下指令寄存器总是由 CPU 内部设置为下一条指令的地址，但是跳转指令和函数调用指令例外，这两条指令的主要作用就是用来改变指令寄存器的内容，正是因为跳转功能才使得我们的程序可以不只按顺序去执行而是具有条件执行和循环执行代码的能力。
> 
> 在 x86 体系的 CPU 中提供了一个 64 位的指令寄存器 rip，而在 ARM64 体系的 CPU 中则提供了一个 64 位的 pc 寄存器，它们两个的作用和功能是完全一样的。
> 需要再次强调的是指令寄存器保存的是下一条将要执行的指令的内存地址，而不是当前正在执行的指令的内存地址。[深入iOS系统底层之CPU寄存器](https://www.jianshu.com/p/6d7a57794122)

> 在内存/磁盘上，指令和数据没有任何区别，都是二进制数据。CPU 在工作时，将有的信息看作指令，有的看作数据，为同样的信息赋予了不同的意义，例如：1110 0000 0000 0011 0000 1000 1010 1010，可以当做数据 0xE003008AA，也可以当做指令 mov x0, x8，CPU 根据什么将内存中的信息看作指令呢？CPU 正是将 pc 指向的内存单元的内容看作指令，如果内存中的某段内容曾经被 CPU 执行过，那么它所在的内存单元必然被 pc 指向过。
> CPU 从何处执行指令，是由 pc 中的内容决定的，可以通过改变 pc 的内容来控制 CPU 执行目标指令，ARM64 提供了一个 mov 指令（传送指令），可以用来改变大部分寄存器的值，例如 mov x0, #10、mov x1, #20，但是，mov 指令并不能用于设置 pc 的值，ARM64 没有提供这样的功能。ARM64 提供了另外的指令来修改 pc 的值，这些指令统一称为转移指令，其中最简单的是 bl 指令。[iOS逆向 ：初识汇编](https://zhuanlan.zhihu.com/p/369071456)

> 高速缓存区域：iPhoneX 上搭载的 arm 处理器 A11，它的 1 级缓存的容量是 64kb，2 级缓存的容量是 8M。CPU 每执行一条指令前都需要从内存中将指令读取到 CPU 内存并执行，而寄存器的运行速度相比内存读写要快很多，为了性能，CPU 还集成了一个高速缓存区域。当程序运行时，先将要执行的指令代码以及数据复制到高速缓存中（由操作系统完成），然后 CPU 直接从高速缓存依次读取指令来执行。

&emsp;看到这里小伙伴是不是对 pc 和 lr 两个寄存器的作用和功能有点晕了呢？下面我们通过实例代码情况来看一下它们两者的区别。

![截屏2022-01-17 下午9.03.05.png](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/27aa6d93a69444888a0952360db9a3e2~tplv-k3u1fbpfcp-watermark.image?)

&emsp;首先是绿色线条圈出的地址，看到 pc 寄存器的值是 0x00000001003a1ff8，正是此时断点处的指令地址，它正是接下来要执行的指令，即 pc 寄存器保存的是下一条将要执行的指令的内存地址。

&emsp;然后是红色线条圈出的地址，从断点向上看最近的一条 bl 指令的下一条指令的地址是 0x1003a1ff0，然后此时 lr 寄存器保存的也是此值，即 lr 寄存器总是保存最后一次函数调用 b 指令的下一条指令的地址。

&emsp;再然后呢，我们猜一手 pc 和 lr 寄存器的值，我们手指按住 control 键，鼠标点击 Step into 键来单步执行一条指令，已知接下来 `bl 0x1003a2544` 指令会跳转到 0x1003a2544 处，那么 pc 寄存器存储的值会更新为 0x1003a2544，而 lr 寄存器存储的值会更新为 `bl 0x1003a2544` 指令下面一条指令的地址 0x1003a1ffc。    

![截屏2022-01-17 下午9.38.15.png](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/a54accaf205e4c0dad0b28adccb54e60~tplv-k3u1fbpfcp-watermark.image?)

&emsp;结果如图所示，完全对应到我们的猜测。小伙伴也可以自己单步执行汇编指令，然后看看对应的寄存器值的更新以加深印象。

&emsp;从上面的图中我们可以看出 pc 寄存器和 lr 寄存器所表示的意义：

+ pc 寄存器保存的是下一条将要执行的指令的内存地址，而不是当前正在执行的指令的内存地址。
+ lr 寄存器则保存着最后一次函数调用指令的下一条指令的内存地址。那么 lr 寄存器有什么作用吗？答案就是为了做函数调用栈跟踪，我们的程序在崩溃时能够将函数调用栈打印出来就是借助了 lr 寄存器来实现的。

&emsp;(这里插入一句 x86 下是没有像 lr 寄存器对应功能的寄存器的，它是依靠每次调用函数时把前一个函数的栈底（rbp/fp 寄存器保存到值）入栈做到的，这里我们通过上面的截图也看到了在 ARM64 体系下每条汇编指令的长度是 4 个字节，通过这个地址的偏移我们也可以读到很多想要的内容，具体用法我们在后续中进行学习。)

##### cpsr

+ cpsr（x33 状态寄存器），sprs 是状态寄存器，用于存放程序运行中一些状态标识。不同于高级编程语言里面的 if else，在汇编中就需要根据状态寄存器中的一些状态来控制分支的执行。状态寄存器又分为 The Current Program Status Register (CPSR) 和 The Saved Program Status Registers (SPSRs)。一般都是使用 CPSR，当发生异常时，CPSR 会存入 SPSR，当异常恢复，再拷贝回 CPSR。

&emsp;状态寄存器用来保存指令运行结果的一些信息，比如相加的结果是否溢出、结果是否为 0、以及是否是负数等。CPU 的某些指令会根据运行的结果来设置状态寄存器的状态位，而某些指令则是根据这些状态寄存器中的值来进行处理。比如一些条件跳转指令或者比较指令等等。我们在高级语言里面的条件判断最终在转化为机器指令时，机器指令就是根据状态寄存器里面的特殊位置来进行跳转的。在 x86 架构的 CPU 中提供了一个 64 bit 的 rflags 寄存器来作为状态寄存器；ARM64 架构的 CPU 则提供了一个 32 bit 的 cpsr 寄存器来作为状态寄存器。状态寄存器的内容由 CPU 内部进行置位，我们的程序中不能将某个数值赋值给状态寄存器。[深入iOS系统底层之CPU寄存器](https://www.jianshu.com/p/6d7a57794122)

&emsp;上面列出的都是我们在编程时会用到的寄存器，其实 CPU 内部还有很多专门用于控制的寄存器以及用于调试的寄存器，这些寄存器一般都提供给操作系统使用或者用于 CPU 内部调试使用。

#### x86 架构寄存器列表

&emsp;下面我们看一下 x86 架构下都有哪些寄存器。

&emsp;同样的操作，这次我们选择模拟器运行调试，在断点命中后在 Xcode 控制台输入 register read --all 命令，看到寄存器分组依然是：General Purpose Registers、Floating Point Registers、Exception State Registers，不过这次寄存器的名字完全发生了变化。

&emsp;General Purpose Registers 分组包括：rax、rbx、rcx、rdx、rdi、rsi、rbp、rsp、r8、r9、r10、r11、r12、r13、r14、r15、rip、rflags、cs、fs、gs。

&emsp;可看到 x86 和 ARM64 架构下 General Purpose Registers 分组的寄存器的名称完全不同，但其实它们的作用基本都是相同的。 

&emsp;同样的我们也使用 register read --alternate 命令查看寄存器的备用名（alternate register name），可看到如下对应关系：

+ rcx: arg4
+ rdx: arg3
+ rdi: arg1
+ rsi: arg2
+ r8: arg5
+ r9: arg6

+ rbp: fp
+ rsp: sp     

&emsp;其中 arg1-arg6 寄存器会用来存储函数参数，我们打印 self 和 `_cmd` 的值看到 rdi：arg1 保存的正是 viewDidLoad 函数的参数 self，rsi: arg2 保存的正是参数 viewDidLoad 的选择子。

&emsp;其中还有两个特别重要的寄存器，rbp: fp 和 rsp: sp 栈底和栈顶寄存器，ARM64 中的 fp 对应 x86 的 rbp，ARM64 中的 sp 对应 x86 的 rsp。 

```c++
(lldb) register read --all
General Purpose Registers:
       rax = 0x0000000000000000  // rax = 0x0000000000000000
       rbx = 0x00007fab1a20a9f0  // rbx = 0x00007fab1a20a9f0
       rcx = 0x0000000204d16600  dyld`_main_thread  // 4⃣️ arg4 = 0x0000000204d16600  dyld`_main_thread（寄存器残留值，寄存器是没有清空操作的，之前函数调用时用到的参数这里还会被残留下来）
       rdx = 0x000000000000010d  // 3⃣️ arg3 = 0x000000000000010d
       rdi = 0x00007fab1a20a9f0  // 1⃣️ arg1 = 0x00007fab1a20a9f0
       rsi = 0x0000000129c5d99d  "viewDidLoad"  // 2⃣️ arg2 = 0x0000000129c5d99d  "viewDidLoad"
       rbp = 0x000000030d789f70  // fp = 0x000000030d789f70
       rsp = 0x000000030d789f50  // sp = 0x000000030d789f50
        r8 = 0x000000010ec960b0  libsystem_pthread.dylib`_pthread_keys  // 5⃣️ arg5 = 0x000000010ec960b0  libsystem_pthread.dylib`_pthread_keys
        r9 = 0x00007fab1d009720  // 6⃣️ arg6 = 0x00007fab1d009720
       r10 = 0x0000000104a4949a  (void *)0xe9b80000000104a4 // r10 = 0x0000000104a4949a  (void *)0xe9b80000000104a4
       r11 = 0x0000000104a41d00  TEST_MENU`-[ViewController viewDidLoad] at ViewController.m:16  // r11 = 0x0000000104a41d00
       r12 = 0x0000000000000278  // r12 = 0x0000000000000278
       r13 = 0x000000010d4e43c0  libobjc.A.dylib`objc_msgSend  // r13 = 0x000000010d4e43c0  libobjc.A.dylib`objc_msgSend
       r14 = 0x0000000000000000  // r14 = 0x0000000000000000
       r15 = 0x000000010d4e43c0  libobjc.A.dylib`objc_msgSend  // r15 = 0x000000010d4e43c0  libobjc.A.dylib`objc_msgSend
       rip = 0x0000000104a41d10  TEST_MENU`-[ViewController viewDidLoad] + 16 at ViewController.m:17:5  // pc = 0x0000000104a41d10
    rflags = 0x0000000000000206  // flags = 0x0000000000000206
        cs = 0x000000000000002b  // cs = 0x000000000000002b
        fs = 0x0000000000000000  // fs = 0x0000000000000000
        gs = 0x0000000000000000  // gs = 0x0000000000000000
       ...

Floating Point Registers:
     ...

Exception State Registers:
    ...
    
(lldb) p self
(ViewController *) $0 = 0x00007fab1a20a9f0
(lldb) p (SEL)0x0000000129c5d99d
(SEL) $1 = "viewDidLoad"
```

&emsp;x86 架构的 General Purpose Registers 分组下有 21 个 64 bit 寄存器，然后 gs 寄存器后面列出的则是前面寄存器的低位，低 32 bit、低 16 bit、低 8 bit，如下对应表格：

| 64 bit | 低 32 bit | 低 16 bit | 低 8 bit |
| -- | -- | -- | -- |
| rax = 0x0000000000000000 | eax = 0x00000000 | ax = 0x0000 | ah = 0x00 al = 0x00 |
| rbx = 0x00007fba，8450，e1，20 | ebx = 0x8450e120 | bx = 0xe120 | bh = 0xe1 bl = 0x20 |
| rcx = 0x00000002，0263，36，00 | ecx = 0x02633600 | cx = 0x3600 | ch = 0x36 cl = 0x00 |
| rdx = 0x00000000，0000，01，0d | edx = 0x0000010d | dx = 0x010d | dh = 0x01 dl = 0x0d |
| rdi = 0x00007fba，8450，e1，20 | edi = 0x8450e120 | di = 0xe120 | dil = 0x20 |
| rsi = 0x00000001，2722，79，9d | esi = 0x2722799d | si = 0x799d | sil = 0x9d |
| rbp = 0x00000003，0893，bf，70 | ebp = 0x0893bf70 | bp = 0xbf70 | bpl = 0x70 |
| rsp = 0x00000003，0893，bf，50 | esp = 0x0893bf50 | sp = 0xbf50 | spl = 0x50 |
| r8 = 0x00000001，0c55，70，b0 | r8d = 0x0c5570b0 | r8w = 0x70b0 | r8l = 0xb0 |
| r9 = 0x00007fba，8383，7e，70 | r9d = 0x83837e70 | r9w = 0x7e70 | r9l = 0x70 |
| r10 = 0x00000001，022d，e4，9a | r10d = 0x022de49a | r10w = 0xe49a | r10l = 0x9a |
| r11 = 0x00000001，022d，6d，00 | r11d = 0x022d6d00 | r11w = 0x6d00 | r11l = 0x00 |
| r12 = 0x00000000，0000，02，78 | r12d = 0x00000278 | r12w = 0x0278 | r12l = 0x78 |
| r13 = 0x00000001，0ad7，93，c0 | r13d = 0x0ad793c0 | r13w = 0x93c0 | r13l = 0xc0 |
| r14 = 0x00000000，0000，00，00 | r14d = 0x00000000 | r14w = 0x0000 | r14l = 0x00 |
| r15 = 0x00000001，0ad7，93，c0 | r15d = 0x0ad793c0 | r15w = 0x93c0 | r15l = 0xc0 |
| rip = 0x00000001022d6d10 | - | - | - |
| rflags = 0x0000000000000206 | - | - | - |
| cs = 0x000000000000002b | - | - | - |
| fs = 0x0000000000000000 | - | - | - |
| gs = 0x0000000000000000 | - | - | - |

&emsp;看到这里我们零零散散的看了 x86 和 ARM64 架构下的常见寄存器，然后也知道寄存器位于 CPU 内部所起到的一些作用。

&emsp;那么怎么把 task、thread、thread state、register、stack frame 它们连起来呢，register 中保存的值正是 thread 的某个 state 下的值... 我们下篇继续！

## 参考链接
**参考链接:🔗**
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



