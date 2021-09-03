# iOS App 启动优化(十二)：LLDB 进阶

## LLDB 简述

> &emsp;什么是 REPL？R(read)、E(evaluate)、P(print)、L(loop)。“读取-求值-输出” 循环（英语：Read-Eval-Print Loop，简称 REPL），也被称作交互式顶层构件（英语：interactive toplevel），是一个简单的，交互式的编程环境。这个词常常用于指代一个 [Lisp](https://zh.wikipedia.org/wiki/LISP) 的交互式开发环境，也能指代命令行的模式。REPL 使得探索性的编程和调试更加便捷，因为 “读取-求值-输出” 循环通常会比经典的 “编辑-编译-运行-调试” 模式要更快。（有时候翻译成交互式解释器。就是你往 REPL 里输入一行代码，它立即给你执行结果。而不用像 C++, Java 那样通常需要编译才能看到运行结果，像 Python Ruby Scala 都是自带 REPL 的语言。）[读取﹣求值﹣输出循环 维基百科](https://zh.wikipedia.org/wiki/读取﹣求值﹣输出循环) [什么是REPL？](https://www.zhihu.com/question/53865469)

&emsp;[LLDB](https://lldb.llvm.org/resources/contributing.html) 是一个有着 REPL 特性并支持 C++、Python 插件的 **开源调试器**，LLDB 已被内置在 Xcode 中，Xcode 主窗口底部的控制台便是我们与 LLDB 交互的区域。LLDB 允许你在程序运行的特定时刻（某行代码、某个函数）暂停它（`br`），你可以查看变量的值（`p/po/wa`）、执行自定的指令（`e`），并且按照你所认为合适的步骤来操作程序（函数）的执行顺序，对程序进行流程控制。（这里有一个关于调试器如何工作的总体的解释。）相信每个人或多或少都在用 LLDB 来调试，比如 po 一个对象。LLDB 是非常强大的，且有内建的，完整的 Python 支持。今天我们除了介绍 LLDB 的一些进阶用法之外，还会详细介绍一下 facebook 开源的 lldb 插件 Chisel，它可以让你的调试更加高效，更加 Easy。

&emsp;[GDB to LLDB command map](https://lldb.llvm.org/use/map.html) 中的一组表格向我们非常好的介绍了 LLDB 调试器提供的几大块命令的总览（常规用法，估计大家都已经掌握了）。除此之外我们可以安装 [Chisel](https://github.com/facebook/chisel/wiki) 来体验 LLDB 更 “高级” 的用法，Chisel 是 facebook 开源的一组  LLDB 命令合集，用于协助我们 **调试 iOS 应用程序**。Chisel 里面的命令正是基于 LLDB 能支持 Python 脚本运行来做的，Chisel 每条命令对应的 Python 文件保存在 `/usr/local/Cellar/chisel/2.0.1/libexec` 路径下，熟悉 Python 的小伙伴可以试着读一下这些文件的内容（具体路径可能各人机器不同会有所区别，例如 Intel 的 mac 在 `/usr/local/Cellar/chisel/2.0.1/libexec` 路径下，m1 的 mac 在 `/opt/homebrew/Cellar/chisel/2.0.1/libexec` 路径下）。   

## LLDB 基础命令使用

&emsp;日常我们更多的可能是在 Xcode 中使用 LLDB，今天我们先在 **终端** 熟悉一下 LLDB 一些命令，然后再去 Xcode 中对部分 LLDB 命令实践。

&emsp;熟悉 LLDB 的小伙伴应该都用过 `help` 命令，如果有哪个命令不太懂的话，我们直接一个 `help <command>`，LLDB 便会打印出该命令的用途，甚至 `help help` 能教我们怎么使用 `help` 命令。

&emsp;下面我们启动终端，然后输入 `LLDB` 并回车，此时便会进入 `LLDB` 环境，然后我们输入 `help` 命令并回车，便会列出当前机器内的的


### help
&emsp;

&emsp;


## 内容规划

1. 介绍 chisel 安装、使用细节、官方提供的命令列表[wiki](https://github.com/facebook/chisel/wiki)、Custom Commands如何进行自定义命令。[chisel](https://github.com/facebook/chisel/blob/master/README.md)

![截屏2021-09-02 下午10.35.11.png](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/b70fa058010847499da535b95c4ba883~tplv-k3u1fbpfcp-watermark.image)
&emsp;Add the following line to ~/.lldbinit to load chisel when Xcode launches:
`command script import /usr/local/opt/chisel/libexec/fbchisellldb.py` 

2. LLDB 给的命令列表使用示例。

3. chisel 命令使用示例。（chisel 里面的命令都是基于 LLDB 支持的 python 脚本来做的，保存在 /opt/homebrew/Cellar/chisel/2.0.1/libexec 路径下。） 






## po 和 p

&emsp;`po` 是 `expression -O  --` 的缩写，`p` 是 `expression --` 的缩写。

```c++
(lldb) help po
     Evaluate an expression on the current thread.  Displays any returned value
     with formatting controlled by the type's author.  Expects 'raw' input (see
     'help raw-input'.)

Syntax: po <expr>

Command Options Usage:
  po <expr>


'po' is an abbreviation for 'expression -O  --'
```

```c++
(lldb) help p
     Evaluate an expression on the current thread.  Displays any returned value
     with LLDB's default formatting.  Expects 'raw' input (see 'help
     raw-input'.)

Syntax: p <expr>

Command Options Usage:
  p <expr>


'p' is an abbreviation for 'expression --'
(lldb) 
```


## 参考链接
**参考链接:🔗**
+ [LLDB Quick Start Guide](https://developer.apple.com/library/archive/documentation/IDEs/Conceptual/gdb_to_lldb_transition_guide/document/Introduction.html#//apple_ref/doc/uid/TP40012917-CH1-SW1)
+ [Advanced Debugging with Xcode and LLDB](https://developer.apple.com/videos/play/wwdc2018/412/)
+ [Xcode 10.2 Release Notes](https://developer.apple.com/documentation/xcode-release-notes/xcode-10_2-release-notes)
+ [小笨狼的LLDB技巧:chisel](https://www.jianshu.com/p/afaaacc55460)
+ [iOS开发中如何调试更高效：LLDB与Chisel](https://juejin.cn/post/6844904017416421390)
+ [与调试器共舞 - LLDB 的华尔兹](https://objccn.io/issue-19-2/)
+ [Chisel-LLDB命令插件，让调试更Easy](https://blog.cnbluebox.com/blog/2015/03/05/chisel/)
+ [LLDB高级调试+Cycript](https://juejin.cn/post/6844903990296068110#heading-5)
+ [iOS逆向 玩转LLDB调试](https://juejin.cn/post/6847902223926722573)
+ [iOS 常用调试方法：LLDB命令](https://juejin.cn/post/6844903794493358093)
+ [LLDB学习笔记](https://www.jianshu.com/p/e5cc0f83a4f0)
+ [iOS调试之chisel](https://www.jianshu.com/p/3eef81bf146d)
+ [iOS调试进阶-更高效的使用Xcode和LLDB](https://juejin.cn/post/6844903866345996296)
+ [Xcode10.2中LLDB增加的新特性](https://juejin.cn/post/6844903848771846157)

+ [WWDC 2018：效率提升爆表的 Xcode 和 LLDB 调试技巧](https://juejin.cn/post/6844903620329078791#heading-28)
+ [lldb-入坑指北（1）-给xcode添加批量启用禁用断点功能](https://cloud.tencent.com/developer/article/1729078)
+ [XCode LLDB调试小技巧基础篇提高篇汇编篇](https://cloud.tencent.com/developer/article/1013359)
+ [(轉)Xcode中LLDB的基本命令的使用（Swift）](https://www.itread01.com/p/357976.html)
+ [facebook/chisel](https://github.com/facebook/chisel/tree/master)

https://developer.apple.com/documentation/xcode-release-notes/xcode-10_2-release-notes
https://juejin.cn/post/6847902223926722573
https://juejin.cn/post/6844903647277481998
https://www.jianshu.com/p/7e2a61585352
https://www.jianshu.com/p/b2371dd4443b
https://www.jianshu.com/p/afaaacc55460
https://www.jianshu.com/p/e5cc0f83a4f0
https://www.jianshu.com/p/3eef81bf146d
https://juejin.cn/post/6844904017416421390#heading-27
https://lldb.llvm.org/use/map.html
https://objccn.io/issue-19-2/
https://juejin.cn/post/6844903990296068110#heading-5
https://github.com/facebook/chisel
https://blog.cnbluebox.com/blog/2015/03/05/chisel/
