# iOS App Crash 分析：(五)：函数堆栈获取解析

&emsp;虽然我们日常开发大部分情况下不需要直接编写汇编指令，但是能看懂汇编指令能分析对应的代码逻辑对我们理解计算机运行逻辑还是有极大促进作用的（内功）。特别是当我们解决 Crash 问题时，利用汇编调试技巧进行反汇编更易使我们定位到问题根源。
 
&emsp;学习函数调用栈相关的内容之前我们需要了解汇编相关的三个重要概念：寄存器、堆栈、指令集，其中寄存器、指令集在不同的架构下有不同的名字，但是基本概念都是一致的，这里我们以 x86 和 arm64 为例来学习。

## 寄存器概述

&emsp;寄存器是 CPU 内部用来存放数据的一些小型存储器，用来暂时存放参与运算的数据和运算结果。其实寄存器就是一种常用的时序逻辑电路，但这种时序逻辑电路只包含存储电路。寄存器的存储电路是由锁存器或触发器构成的，因为一个锁存器或触发器能存储 1 位二进制数，所以由 N 个锁存器或触发器可以构成 N 位寄存器。

&emsp;寄存器是 CPU 内的组成部分。寄存器是有限存储容量的高速存储部件，它们可用来暂存指令、数据和位址。

&emsp;在计算机领域，寄存器是 CPU 内部的元件，按功能划分包括：通用寄存器、专用寄存器和控制寄存器。寄存器拥有非常高的读写速度，所以在寄存器之间的数据传送非常快。

&emsp;寄存器有串行和并行两种数码存取方式。将 N 位二进制数一次存入寄存器或从寄存器中读出的方式称为并行方式。将 N 位二进制数以每次 1 位，分成 N 次存入寄存器或从寄存器读出，这种方式称为串行方式。

&emsp;并行方式只需一个时钟脉冲就可以完成数据操作，工作速度快，但需要 N 根输入和输出数据线。串行方式要使用几个时钟脉冲完成输入或输出操作，工作速度慢，但只需要一根输入或输出数据线，传输线少，适用于远距离传输。[寄存器-百度百科](https://baike.baidu.com/item/寄存器/187682?fr=aladdin)

## 寄存器类型

&emsp;在看具体的寄存器之前，我们先学习一下 LLDB 中的 register 命令。

### LLDB register 命令

```c++
(lldb) help register
Commands to access registers for the current thread and stack frame.

Syntax: register [read|write] ...

The following subcommands are supported:

      read  -- Dump the contents of one or more register values from the current frame.  If no register is specified, dumps them all.
      write -- Modify a single register value.

For more help on any particular subcommand, type 'help <command> <subcommand>'.
```

&emsp;register 命令用于访问当前线程和 stack frame 的寄存器的命令。它有两条子命令：

+ read：从当前 frame Dump 一个或多个寄存器中存储的内容。如果未指定寄存器，则把它们全部 Dump 出来。直白一点理解就是读取寄存器中存储的内容直接打印出来。
+ write：修改单个寄存器的值。

&emsp;下面看一下两个子命令后面都可以使用哪些选项：

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

&emsp;-A/-a 选项都是打印出当前所有寄存器，其中 -A 使用 alternate 寄存器名称（如果有）显示寄存器名称，-a 显示所有寄存器的集合。

```c++
(lldb) help register write
Modify a single register value.

Syntax: register write <register-name> <value>
```

&emsp;修改指定寄存器的值。

### 使用 LLDB register 命令查看不同平台的寄存器

&emsp;然后我们在 ARM64 和 x86 平台下看一下它们都有哪些寄存器。我们分别选择以模拟器或真机进入 LLDB 调试模式，然后在 Xcode 控制台使用 register read 命令查看打印内容。

#### ARM64

&emsp;已知 iPhone 系列都是 ARM 架构的 CPU，且自 iPhone 5s 以来开始进入 64 位的 arm64 架构。

&emsp;这里我们使用 viewDidLoad 函数为示例代码，下面学习指令集时也以 viewDidLoad 函数为示例代码，在 viewDidLoad 函数处打一个断点，选中真机（iPhone X）然后运行程序，断点命中后进入 LLDB 调试模式，使用 `register read --all` 命令打印所有寄存器，可看到其中包括：General Purpose Registers、Floating Point Registers、Exception State Registers 三个分组。

&emsp;这里我们只专注于 General Purpose Registers 分组，看到 ARM64 下有：x0-x28（通用寄存器）、fp（x29 frame pointer 栈底寄存器）、lr（x30 link register 链接寄存器）、sp（x31 stack pointer 栈顶寄存器）、pc（x32 program counter 程序计数器）、cpsr（x33 状态寄存器）共 34 个 64 bit 的 General Purpose Registers，w0-w28 表示 x0-x28 的低 32 bit。

```c++
(lldb) register read --all
General Purpose Registers:
        x0 = 0x0000000102d08360  // w0 = 0x02d08360
        x1 = 0x000000019b043c57  // w1 = 0x9b043c57
        x2 = 0x0000000000000001  // w2 = 0x00000001
        x3 = 0x000000016d59db90  // w3 = 0x6d59db90
        x4 = 0x0000000000000010  // w4 = 0x00000010
        x5 = 0x0000000000000020  // w5 = 0x00000020
        x6 = 0x000000016d59d890  // w6 = 0x6d59d890
        x7 = 0x0000000000000000  // w7 = 0x00000000
        x8 = 0x000000019b043000  // w8 = 0x9b043000
        x9 = 0x0000000000000000  // w9 = 0x00000000
       x10 = 0x000000000000002f  // w10 = 0x0000002f
       x11 = 0x0000000103023258  // w11 = 0x03023258
       x12 = 0x0000000000000025  // w12 = 0x00000025
       x13 = 0x0000000000000000  // w13 = 0x00000000
       x14 = 0x0000000000000000  // w14 = 0x00000000
       x15 = 0xffffffffffffffff  // w15 = 0xffffffff
       x16 = 0x000000010286d2b2  (void *)0x78e0000000010286  // w16 = 0x0286d2b2
       x17 = 0x0000000102865f60  TEST_MENU`-[ViewController viewDidLoad] at ViewController.m:16 // w17 = 0x02865f60
       x18 = 0x0000000000000000  // w18 = 0x00000000
       x19 = 0x0000000102d08360  // w19 = 0x02d08360
       x20 = 0x0000000000000000  // w20 = 0x00000000
       x21 = 0x00000001f2d9c000  (void *)0x000000019efcba50  // w21 = 0xf2d9c000
       x22 = 0x000000019ba00157  // w22 = 0x9ba00157
       x23 = 0x000000019af2039d  // w23 = 0x9af2039d
       x24 = 0x0000000000000000  // w24 = 0x00000000
       x25 = 0x00000001f3987000  UIKitCore`_UIWindowSceneActivationSettings._pinchActivationScaleThreshold  // w25 = 0xf3987000
       x26 = 0x0000000102d06280  // w26 = 0x02d06280
       x27 = 0x000000019b639957  // w27 = 0x9b639957
       x28 = 0x00000001f84cf4f0  CoreFoundation`__NSArray0__struct  // w28 = 0xf84cf4f0
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

&emsp;可看到我们听过了很多次的说 x0-x7 是参数寄存器，这里也得到了印证 x0-x7 寄存器的备用名正是 arg1-arg8，如下我们直接读取 x0/arg1 打印的都是 x0，然后我们打印 self，看到其值也正是 x0 寄存器中保存的值，然后打印 `_cmd` 的值是 viewDidLoad，然后把 x1 寄存器的值强转为 SEL 后看到的也是 viewDidLoad，即当前 x0 x1 寄存器中存的值正是当前的函数参数。

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

#### x86

&emsp;下面我们看一下 x86 架构下都有哪些寄存器。

&emsp;同样的操作，这次我们选择模拟器运行调试，在断点命中后在 Xcode 控制台输入 register read --all 命令，看到寄存器分组依然是：General Purpose Registers、Floating Point Registers、Exception State Registers，不过这次寄存器的名字完全发生了变化。

&emsp;General Purpose Registers : rax、rbx、rcx、rdx、rdi、rsi、rbp、rsp、r8、r9、r10、r11、r12、r13、r14、r15、rip、rflags、cs、fs、gs。

```c++
(lldb) register read --all
General Purpose Registers:
       rax = 0x0000000000000000
       rbx = 0x00007fba8450e120
       rcx = 0x0000000202633600  dyld`_main_thread
       rdx = 0x000000000000010d
       rdi = 0x00007fba8450e120
       rsi = 0x000000012722799d  "viewDidLoad"
       rbp = 0x000000030893bf70
       rsp = 0x000000030893bf50
        r8 = 0x000000010c5570b0  libsystem_pthread.dylib`_pthread_keys
        r9 = 0x00007fba83837e70
       r10 = 0x00000001022de49a  (void *)0x89b800000001022d
       r11 = 0x00000001022d6d00  TEST_MENU`-[ViewController viewDidLoad] at ViewController.m:16
       r12 = 0x0000000000000278
       r13 = 0x000000010ad793c0  libobjc.A.dylib`objc_msgSend
       r14 = 0x0000000000000000
       r15 = 0x000000010ad793c0  libobjc.A.dylib`objc_msgSend
       rip = 0x00000001022d6d10  TEST_MENU`-[ViewController viewDidLoad] + 16 at ViewController.m:17:5
    rflags = 0x0000000000000206
        cs = 0x000000000000002b
        fs = 0x0000000000000000
        gs = 0x0000000000000000
       ...

Floating Point Registers:
        ...

Exception State Registers:
        ...
```

| 64 bit | 低 32 bit | 低 16 bit | 低 8 bit |
| -- | -- | -- | -- |
| rax = 0x0000000000000000 | eax = 0x00000000 | ax = 0x0000 | ah = 0x00 al = 0x00 |
| rbx = 0x00007fba8450e120 | ebx = 0x8450e120 | bx = 0xe120 | bh = 0xe1 bl = 0x20 |
| rcx = 0x0000000202633600 | ecx = 0x02633600 | cx = 0x3600 | ch = 0x36 cl = 0x00 |
| rdx = 0x000000000000010d | edx = 0x0000010d | dx = 0x010d | dh = 0x01 dl = 0x0d |
| rdi = 0x00007fba8450e120 | edi = 0x8450e120 | di = 0xe120 | dil = 0x20 |
| rsi = 0x000000012722799d | esi = 0x2722799d | si = 0x799d | sil = 0x9d |
| rbp = 0x000000030893bf70 | ebp = 0x0893bf70 | bp = 0xbf70 | bpl = 0x70 |
| rsp = 0x000000030893bf50 | esp = 0x0893bf50 | sp = 0xbf50 | spl = 0x50 |
| r8 = 0x000000010c5570b0 | r8d = 0x0c5570b0 | r8w = 0x70b0 | r8l = 0xb0 |
| r9 = 0x00007fba83837e70 | r9d = 0x83837e70 | r9w = 0x7e70 | r9l = 0x70 |
| r10 = 0x00000001022de49a | r10d = 0x022de49a | r10w = 0xe49a | r10l = 0x9a |
| r11 = 0x00000001022d6d00 | r11d = 0x022d6d00 | r11w = 0x6d00 | r11l = 0x00 |
| r12 = 0x0000000000000278 | r12d = 0x00000278 | r12w = 0x0278 | r12l = 0x78 |
| r13 = 0x000000010ad793c0 | r13d = 0x0ad793c0 | r13w = 0x93c0 | r13l = 0xc0 |
| r14 = 0x0000000000000000 | r14d = 0x00000000 | r14w = 0x0000 | r14l = 0x00 |
| r15 = 0x000000010ad793c0 | r15d = 0x0ad793c0 | r15w = 0x93c0 | r15l = 0xc0 |
| rip = 0x00000001022d6d10 | - | - | - |
| rflags = 0x0000000000000206 | - | - | - |
| cs = 0x000000000000002b | - | - | - |
| fs = 0x0000000000000000 | - | - | - |
| gs = 0x0000000000000000 | - | - | - |


```c++
(lldb) register read --alternate
General Purpose Registers:
       rax = 0x0000000000000000
       rbx = 0x00007fac55909bf0
      arg4 = 0x000000010d1c1600  dyld`_main_thread
      arg3 = 0x000000000000010d
      arg1 = 0x00007fac55909bf0
      arg2 = 0x00007fff6c456797
        fp = 0x00007ff7b6a17f80
        sp = 0x00007ff7b6a17f60
      arg5 = 0x00007fff862a40c0  libsystem_pthread.dylib`_pthread_keys
      arg6 = 0x00007fac5a80d400
       r10 = 0x00000001094ed3f2  (void *)0xdd3800000001094e
       r11 = 0x00000001094e5ec0  TEST_MENU`-[ViewController viewDidLoad] at ViewController.m:16
       r12 = 0x0000000000000278
       r13 = 0x00007fff201803c0  libobjc.A.dylib`objc_msgSend
       r14 = 0x0000000000000000
       r15 = 0x00007fff201803c0  libobjc.A.dylib`objc_msgSend
        pc = 0x00000001094e5ed0  TEST_MENU`-[ViewController viewDidLoad] + 16 at ViewController.m:17:5
     flags = 0x0000000000000206
        cs = 0x000000000000002b
        fs = 0x0000000000000000
        gs = 0x0000000000000000

(lldb) p 0x00007fac55909bf0
(long) $2 = 140378146642928
(lldb) p self
(ViewController *) $3 = 0x00007fac55909bf0
(lldb) p (SEL)0x00007fff6c456797
(SEL) $4 = "viewDidLoad"
```

&emsp;
















```c++
- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
}
```





```c++
#if __LP64__
// true arm64

#define SUPPORT_TAGGED_POINTERS 1
#define PTR .quad
#define PTRSIZE 8
#define PTRSHIFT 3  // 1<<PTRSHIFT == PTRSIZE
// "p" registers are pointer-sized
#define UXTP UXTX
#define p0  x0
#define p1  x1
#define p2  x2
#define p3  x3
#define p4  x4
#define p5  x5
#define p6  x6
#define p7  x7
#define p8  x8
#define p9  x9
#define p10 x10
#define p11 x11
#define p12 x12
#define p13 x13
#define p14 x14
#define p15 x15
#define p16 x16
#define p17 x17

// true arm64
#else
// arm64_32

#define SUPPORT_TAGGED_POINTERS 0
#define PTR .long
#define PTRSIZE 4
#define PTRSHIFT 2  // 1<<PTRSHIFT == PTRSIZE
// "p" registers are pointer-sized
#define UXTP UXTW
#define p0  w0
#define p1  w1
#define p2  w2
#define p3  w3
#define p4  w4
#define p5  w5
#define p6  w6
#define p7  w7
#define p8  w8
#define p9  w9
#define p10 w10
#define p11 w11
#define p12 w12
#define p13 w13
#define p14 w14
#define p15 w15
#define p16 w16
#define p17 w17

// arm64_32
#endif
```

&emsp;


```c++
(lldb) register read -A
General Purpose Registers:
       rax = 0x0000000000000000
       rbx = 0x00007f83746262e0
      arg4 = 0x0000000203227600  dyld`_main_thread
      arg3 = 0x000000000000010e
      arg1 = 0x00007f83746262e0
      arg2 = 0x0000000127ebe99d  "viewDidLoad"
        fp = 0x000000030a1baf70
        sp = 0x000000030a1baf50
      arg5 = 0x000000010d1ee0b0  libsystem_pthread.dylib`_pthread_keys
      arg6 = 0x00007f8374831140
       r10 = 0x0000000102f6f362  (void *)0xf9b80000000102f6
       r11 = 0x0000000102f62220  TEST_MENU`-[ViewController viewDidLoad] at ViewController.m:33
       r12 = 0x0000000000000278
       r13 = 0x000000010ba103c0  libobjc.A.dylib`objc_msgSend
       r14 = 0x0000000000000000
       r15 = 0x000000010ba103c0  libobjc.A.dylib`objc_msgSend
        pc = 0x0000000102f62230  TEST_MENU`-[ViewController viewDidLoad] + 16 at ViewController.m:34:5
     flags = 0x0000000000000206
        cs = 0x000000000000002b
        fs = 0x0000000000000000
        gs = 0x0000000000000000

```




&emsp;搞清楚函数调用栈是怎么获取的，就必须了解这个机制。

&emsp;函数调用栈有个大致的印象，栈帧图：

&emsp;首先了解寄存器，ARM64 有 34 个寄存器，其中 31 个通用寄存器、SP、PC、CPSR。调用约定指定他们其中的一些寄存器有特殊的用途，例如：

&emsp;x0-x28 通用寄存器。

&emsp;x29(FP) 通常用作帧指针 fp (frame pointer 寄存器) ，栈帧基址寄存器，指向当前函数栈帧的栈底。

&emsp;x30(LR) 链接寄存器（link register）。它保存了当目前函数返回时下一个函数的地址。

&emsp;SP 栈指针（stack pointer）存放指向栈顶的指针，使用 SP/WSP 来进行对 SP 寄存器的访问。

&emsp;PC 程序计数器（program counter）它存放了当前执行指令的地址，在每个指令执行完成后会自动增加。

&emsp;CPSR 状态寄存器

```c++
/* Get the current mach thread ID.
 * mach_thread_self() receives a send right for the thread port which needs to be deallocated to balance the reference count. This function takes care of all of that for you.
 *
 * @return The current thread ID.
 */
KSThread ksthread_self(void);

KSThread ksthread_self()
{
    thread_t thread_self = mach_thread_self();
    mach_port_deallocate(mach_task_self(), thread_self);
    return (KSThread)thread_self;
}
```

&emsp;可以看到其它的寄存器也还有一些不相干的信息，说明寄存器不会每个堆栈执行完都全部清理，一般只需要使用时候正确取值即可。

&emsp;因此可知调用链就是一个链表结构。帧的地址就是栈基址寄存器的地址。而每个帧的返回地址指针就是上一个栈的基址寄存器。

&emsp;内核为了能在出现异常时回溯调用过程，会把整个调用链的堆栈保存下来。

&emsp;其中之一避免调用栈的开销，因此 runtime 把它以纯汇编的形式实现，相当于内嵌在当前的调用栈里执行，即它共用的是当前调用栈的空间，而不需要新开辟一个调用栈。









## 参考链接
**参考链接:🔗**
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
