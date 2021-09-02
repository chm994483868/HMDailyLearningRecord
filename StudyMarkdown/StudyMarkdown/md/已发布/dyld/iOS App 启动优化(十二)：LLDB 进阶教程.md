# iOS App 启动优化(十二)：LLDB 进阶

## LLDB 简述

> &emsp;什么是 REPL？R(read)、E(evaluate)、P(print)、L(loop)。“读取-求值-输出” 循环（英语：Read-Eval-Print Loop，简称 REPL），也被称做交互式顶层构件（英语：interactive toplevel），是一个简单的，交互式的编程环境。这个词常常用于指代一个 [Lisp](https://zh.wikipedia.org/wiki/LISP) 的交互式开发环境，也能指代命令行的模式。REPL 使得探索性的编程和调试更加便捷，因为 “读取-求值-输出” 循环通常会比经典的 “编辑-编译-运行-调试” 模式要更快。（有时候翻译成交互式解释器。就是你往 REPL 里输入一行代码，它立即给你执行结果。而不用像 C++, Java 那样通常需要编译才能看到运行结果，像 Python Ruby Scala 都是自带 REPL 的语言。）[读取﹣求值﹣输出循环 维基百科](https://zh.wikipedia.org/wiki/读取﹣求值﹣输出循环) [什么是REPL？](https://www.zhihu.com/question/53865469)


LLDB 是一个有着 REPL 的特性和 C++ ,Python 插件的开源调试器。LLDB 绑定在 Xcode 内部，存在于主窗口底部的控制台中。调试器允许你在程序运行的特定时暂停它，你可以查看变量的值，执行自定的指令，并且按照你所认为合适的步骤来操作程序的进展。(这里有一个关于调试器如何工作的总体的解释。)
相信每个人或多或少都在用LLDB来调试，比如po一个对象。LLDB的是非常强大的，且有内建的，完整的 Python 支持。今天我们主要介绍一个 facebook 开源的 lldb 插件 Chisel。可以让你的调试更Easy.

LLDB 是一个有着 REPL 的特性和 C++ ,Python 插件的开源调试器。LLDB 绑定在 Xcode 内部，存在于主窗口底部的控制台中。调试器允许你在程序运行的特定时暂停它，你可以查看变量的值，执行自定的指令，并且按照你所认为合适的步骤来操作程序的进展。(这里有一个关于调试器如何工作的总体的解释。)

你以前有可能已经使用过调试器，即使只是在 Xcode 的界面上加一些断点。但是通过一些小的技巧，你就可以做一些非常酷的事情。GDB to LLDB 参考是一个非常好的调试器可用命令的总览。你也可以安装 Chisel，它是一个开源的 LLDB 插件合辑，这会使调试变得更加有趣。


## 内容规划

1. 介绍 chisel 安装、使用细节、官方提供的命令列表[wiki](https://github.com/facebook/chisel/wiki)、Custom Commands如何进行自定义命令。[chisel](https://github.com/facebook/chisel/blob/master/README.md)
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
