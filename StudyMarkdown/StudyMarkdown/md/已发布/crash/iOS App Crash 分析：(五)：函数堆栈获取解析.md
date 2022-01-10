# iOS App Crash 分析：(五)：函数堆栈获取解析

&emsp;虽然我们日常开发大部分情况下不需要直接编写汇编指令，但是能看懂汇编指令能分析对应的代码逻辑对我们理解计算机运行逻辑还是有极大促进作用的（内功）。特别是当我们解决 Crash 问题时，利用汇编调试技巧进行反汇编更易使我们定位到问题根源。
 
&emsp;学习函数调用栈相关的内容之前我们需要了解汇编相关的三个重要概念：寄存器、堆栈、指令集，其中寄存器、指令集在不同的架构下有不同的名字，但是基本概念都是一致的，这里我们使用 x86 和 arm64 为例来学习。

## 寄存器

> &emsp;寄存器（Register）是计算机 CPU 内用来暂存指令、数据和地址的内部存储器。
> 寄存器的存贮容量有限，读写速度非常快。在计算机体系结构里，寄存器存储在已知时间点所做计算的中间结果，通过快速地访问数据来加速计算机程序的执行。
> &emsp;寄存器位于存储器层次结构的最顶端，也是CPU可以读写的最快的存储器，事实上所谓的暂存已经不像存储器，而是非常短暂的读写少量信息并马上用到，因为通常程序执行的步骤中，这期间就会一直使用它。寄存器通常都是以他们可以保存的比特数量来计量，举例来说，一个8位寄存器或32位寄存器。在中央处理器中，包含寄存器的部件有指令寄存器（IR）、程序计数器和累加器。寄存器现在都以寄存器数组的方式来实现，但是他们也可能使用单独的触发器、高速的核心存储器、薄膜存储器以及在数种机器上的其他方式来实现出来。
寄存器也可以指代由一个指令之输出或输入可以直接索引到的寄存器组群，这些寄存器的更确切的名称为“架构寄存器”。例如，x86指令集定义八个32位寄存器的集合，但一个实现x86指令集的CPU内部可能会有八个以上的寄存器。




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
(lldb) help register
Commands to access registers for the current thread and stack frame.

Syntax: register [read|write] ...

The following subcommands are supported:

      read  -- Dump the contents of one or more register values from the current frame.  If no register is specified, dumps them all.
      write -- Modify a single register value.

For more help on any particular subcommand, type 'help <command> <subcommand>'.
```

```c++
(lldb) help register read
Dump the contents of one or more register values from the current frame.  If no register is specified, dumps them all.

Syntax: register read <cmd-options> [<register-name> [<register-name> [...]]]

Command Options Usage:
  register read [-A] [-f <format>] [-G <gdb-format>] [-s <index>] [<register-name> [<register-name> [...]]]
  register read [-Aa] [-f <format>] [-G <gdb-format>] [<register-name> [<register-name> [...]]]

       -A ( --alternate )
            Display register names using the alternate register name if there is one.

       -G <gdb-format> ( --gdb-format <gdb-format> )
            Specify a format using a GDB format specifier string.

       -a ( --all )
            Show all register sets.

       -f <format> ( --format <format> )
            Specify a format to be used for display.

       -s <index> ( --set <index> )
            Specify which register sets to dump by index.
     
     This command takes options and free-form arguments.  
     If your arguments resemble option specifiers (i.e., they start with a - or --), you must use ' -- ' between the end of the command options and the beginning of the
     arguments.
```

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
