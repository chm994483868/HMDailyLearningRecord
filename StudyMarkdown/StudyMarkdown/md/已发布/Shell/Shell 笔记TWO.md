# Shell 笔记

## Shell 概述

&emsp;Shell 是一个 C 语言编写的脚本语言，它是用户与 Linux 的桥梁，用户输入命令交给 Shell 处理，shell 是一个命令解释器，是一个工具箱，Shell 将相应的操作传递给内核（Kernel），内核把处理的结果输出给用户。

&emsp;Shell 是一个用 C 语言编写的程序，它是用户使用 Linux 的桥梁。**Shell 既是一种命令语言，又是一种程序设计语言。**

&emsp;shell 也是操作系统中的一个软件，它包含在 Linux 内核的外面，为用户和内核之间的交互提供了一个接口，目前最流行的 shell 为 bash shell（GUN Bourne-Again Shell）。用户的命令用 shell 去解释，再发送给 Linxu 内核，shell 接收系统方回应的输出并显示其到屏幕中。

## Shell 类型

&emsp;sh(全称 Bourne Shell)，是 UNIX 最初使用的 shell，而且在每种 UNIX 上都可以使用。

&emsp;Bourne Shell 在 shell 编程方便相当优秀，但在处理与用户的交互方便作得不如其他几种 shell。

&emsp;bash（全称 Bourne Again Shell），Linux 默认的，它是 Bourne Shell 的扩展。

&emsp;与 Bourne Shell 完全兼容，并且在 Bourne Shell 的基础上增加了很多特性。可以提供命令补全，命令编辑和命令历史等功能。它还包含了很多 C Shell 和 Korn Shell 中的优点，有灵活和强大的编辑接口，同时又很友好的用户界面。

&emsp;csh(全称 C Shell)，是一种比 Bourne Shell 更适合的变种 Shell，它的语法与 C 语言很相似。

&emsp;Tcsh 是 Linux 提供的 C Shell 的一个扩展版本。

&emsp;Tcsh 包括命令行编辑，可编程单词补全，拼写校正，历史命令替换，作业控制和类似 C 语言的语法，它不仅和 Bash Shell 提示符兼容，而且还提供比 Bash Shell 更多的提示符参数。ksh(全称 Korn Shell)，集合了 C Shell 和 Bourne Shell 的优点并且和 Bourne Shell 完全兼容。

&emsp;pdksh，是 Linux 系统提供的 ksh 的扩展。

&emsp;pdksh 支持人物控制，可以在命令行上挂起，后台执行，唤醒或终止程序。

## Shell 脚本

&emsp;什么是 shell 脚本？脚本就是 shell 命令组成的文件，这些命令都是可执行程序的名字，脚本不用编译即可通过解释器解释运行。它是一种解释型语言，我们可以用 shell 脚本操存执行动作，用脚本判定命令的执行条件，用脚本来实现动作的批量执行。

&emsp;Shell 脚本（shell script），是一种为 shell 编写的脚本程序。大家口中所说的 shell 通常都是指 shell 脚本，但我们自己则要知道，shell 和 shell script 是两个不同的概念。由于习惯的原因，简洁起见，本文出现的 "shell 编程" 都是指 shell 脚本编程，不是指开发 shell 自身。

&emsp;Shell 脚本编程跟 JavaScript、php 编程一样，只要有一个能编写代码的文本编辑器和一个能解释执行的脚本解释器就可以了。

&emsp;使用 `cat /etc/shells` 可看到 m1 的 macMini 系统自带如下 shell 解释器。

```c++
hmc@localhost ~ % cat /etc/shells
# List of acceptable shells for chpass(1).
# Ftpd will not allow users to connect who are not using one of these shells.

/bin/bash
/bin/csh
/bin/dash
/bin/ksh
/bin/sh
/bin/tcsh
/bin/zsh
```

https://apptrailers.itunes.apple.com/apple-assets-us-std-000001/PurpleVideo7/v4/ff/9b/d4/ff9bd4a2-2762-baac-1237-df7a81f2dbdb/P37356270_default.m3u8

&emsp;Linux 的 Shell 种类众多，常见的有：

+ Bourne Shell（/usr/bin/sh 或 /bin/sh）
+ **Bourne Again Shell（/bin/bash）**（现在最流行，也是大多数 Linux 系统默认的 Shell。）
+ C Shell（/usr/bin/csh）
+ K Shell（/usr/bin/ksh）
+ Shell for Root（/sbin/sh）
...

&emsp;在一般情况下，人们并不区分 Bourne Shell 和 Bourne Again Shell，所以，像 `#!/bin/sh`，它同样也可以改为 `#!/bin/bash`。`#!` 告诉系统其后路径所指定的程序即是解释此 Shell 脚本文件的 Shell 程序，`#!` 是一个约定的标记，它告诉系统这个 Shell 脚本需要什么解释器来执行，即使用哪一种 Shell。

## 运行 Shell 脚本有不同的方式

1. 作为可执行程序，将 shell 脚本保存为 xxx.sh，并 cd 到相应目录，

```c++
chmod +x ./xxx.sh  # 使脚本具有执行权限
./xxx.sh  # 执行脚本
```

&emsp;注意，一定要写成 ./xxx.sh，而不是 xxx.sh，运行其它二进制的程序也一样，直接写 xxx.sh，linux 系统会去 PATH 里寻找有没有叫 xxx.sh 的，而只有 /bin, /sbin, /usr/bin，/usr/sbin 等在 PATH 里，你的当前目录通常不在 PATH 里，所以写成 xxx.sh 是会找不到命令的，要用 ./xxx.sh 告诉系统说，就在当前目录找。

&emsp;仅使用 . 时，打开一个子 shell 来读取并执行 FileName 中命令，该 FileName 文件需要 "执行权限"。运行一个 shell 脚本时会启动另一个命令解释器。

2. 作为解释器参数。这种运行方式是，直接运行解释器，其参数就是 shell 脚本的文件名，如：

```c++
/bin/sh FileName.sh
/bin/php FileName.php
```

&emsp;打开一个子 shell 来读取并执行 FileName 中命令。该 FileName 文件可以无 "执行权限"。运行一个 shell 脚本时会启动另一个命令解释器。

&emsp;这种方式运行 shell 脚本，不需要在 shell 脚本文件第一行指定解释器信息，写了也没用，它会直接使用上面指令中指定的解释器。

&emsp;chmod + x file 加上执行权限，否则会提示无执行权限。注意执行脚本时候或者全目录，或者 ./file.sh ，如果不加的话，linux 默认会从 PATH 里去找该 file.sh。

> 所以脚本其实就是短小的、用来让计算机自动化完成一系列工作的程序，这类程序可以用文本编辑器修改，不需要编译，通常是解释运行的。

3. source 命令用法。

```c++
source FileName
```

&emsp;在当前 bash 环境下读取并执行 FileName 中的命令。该 FileName 文件可以无 "执行权限"。该命令通常用命令 . 来替代。
















## 参考链接
**参考链接:🔗**
+ [Bash 脚本教程](https://wangdoc.com/bash/)
+ [shell（shell简介）](https://www.cnblogs.com/du-z/p/10959013.html)
+ [shell简介](https://blog.csdn.net/qq_43943846/article/details/88014144)
+ [Shell介绍](https://zhuanlan.zhihu.com/p/115642982)
+ [shell学习笔记](https://zhuanlan.zhihu.com/p/143730100)
+ [shell基本指南](https://zhuanlan.zhihu.com/p/105102573)
+ [sh/bash/csh/Tcsh/ksh/pdksh等shell本质区别](https://blog.csdn.net/dream_an/article/details/50548936)
+ [快，学会 shell](https://zhuanlan.zhihu.com/p/75373580)
+ [Shell 编程入门](https://zhuanlan.zhihu.com/p/97566547)
+ [浅谈shell 基础，思想和技巧](https://zhuanlan.zhihu.com/p/129268123)
+ [在 Mac 上将 zsh 用作默认 Shell](https://support.apple.com/zh-cn/HT208050)
