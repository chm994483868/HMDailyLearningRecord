# Shell 学习笔记

## Shell 概述

&emsp;操作系统的外壳：操作系统是由多个功能模块组成的功能庞大、复杂的软件系统。任何通用的操作系统，都要提供一个称为 shell（外壳）的程序（shell 这个单词的原意是 "外壳"，跟 kernel（内核）相对应，比喻内核外面的一层，即用户跟内核交互的对话界面），用户（操作人员）使用这个程序来操作计算机系统进行工作。DOS 中有一个程序 command.com，这个程序在 DOS 中称为命令解释器，也就是 DOS 系统的 shell。[汇编语言（第 3 版）](https://book.douban.com/subject/25726019/)

&emsp;在 Linux 中，shell 是一个用 C 语言编写的程序。用户输入命令交给 shell 处理（例如：cd、dir、type 等），shell 是一个命令解释器，是一个工具箱，shell 将相应的操作传递给内核（Kernel），内核把处理的结果输出给用户。shell 也是操作系统中的一个软件，它包含在 Linux 内核的外面，为用户和内核之间的交互提供了一个接口，目前最流行的 shell 为 bash shell（GUN Bourne-Again Shell）。用户的命令用 shell 去解释，再发送给 Linxu 内核，shell 接收系统方回应的输出并显示其到屏幕中。

> 具体来说，Shell 这个词有多种含义。
  首先，Shell 是一个程序，提供一个与用户对话的环境。这个环境只有一个命令提示符，让用户从键盘输入命令，所以又称为命令行环境（command line interface，简写为 CLI）。Shell 接收到用户输入的命令，将命令送入操作系统执行，并将结果返回给用户。本书中，除非特别指明，Shell 指的就是命令行环境。
  
  其次，Shell 是一个命令解释器，解释用户输入的命令。它支持变量、条件判断、循环操作等语法，所以用户可以用 Shell 命令写出各种小程序，又称为脚本（script）。这些脚本都通过 Shell 的解释执行，而不通过编译。
  
  最后，Shell 是一个工具箱，提供了各种小工具，供用户方便地使用操作系统的功能。[Bash 简介](https://wangdoc.com/bash/intro.html)

&emsp;在 macOS 下打开终端程序便进入了 shell 命令行环境，在 macOS Monterey 中默认是 zsh，我们也可以直接执行 `bash` 命令切换到 bash，退出则可以使用 `exit` 命令，或者同时按下 control + d。`bash --version` 可以查看当前 bash 的版本。也可以切换到 bash 后使用 `echo $BASH_VERSION` 查看。

```c++
hmc@localhost ~ % bash

The default interactive shell is now zsh.
To update your account to use zsh, please run `chsh -s /bin/zsh`.
For more details, please visit https://support.apple.com/kb/HT208050.
bash-3.2$ exit
exit
hmc@localhost ~ % 

...
bash-3.2$ echo $BASH_VERSION
3.2.57(1)-release
...

hmc@localhost ~ % bash --version
GNU bash, version 3.2.57(1)-release (x86_64-apple-darwin21)
Copyright (C) 2007 Free Software Foundation, Inc.
```

&emsp;`pwd` 可以查看当前所在的目录。

```c++
hmc@localhost ~ % pwd
/Users/hmc
```

## Shell 类型

&emsp;使用 `cat /etc/shells` 命令，直接打印 /etc 目录下 shells 文稿文件中的内容，可看到 macOS 系统自带如下一系列 shell。

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

&emsp;然后我们可直接在 /bin 目录下看到不同类型 shell 程序（解释器）的可执行文件。

![截屏2022-02-09 上午6.39.58.png](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/5336ffa338b64a429512cc391ffb54ac~tplv-k3u1fbpfcp-watermark.image?)

&emsp;下面是不同类型 shell 的一些简述，关于各 shell 的详细信息我们可以在终端中使用 `man bash`、`man csh`、`man dash`、`man ksh`、`man sh`、`man tcsh`、`man zsh` 命令查看。

+ sh（全称 Bourne Shell），是 UNIX 最初使用的 shell，而且在每种 UNIX 上都可以使用。Bourne Shell 在 shell 编程方便相当优秀，但在处理与用户的交互方面做得不如其他几种 shell。
+ bash（全称 Bourne Again Shell），是 Linux 和 Mac 的默认 shell（命令行环境）（bash 是 macOS Mojave 及更低版本中的默认 Shell，从 macOS Catalina 版开始，zsh (Z shell) 是所有新建用户帐户的默认 Shell），现在最流行，也是大多数 Linux 系统默认的 shell。它是 Bourne Shell 的扩展，与 Bourne Shell 完全兼容，并且在 Bourne Shell 的基础上增加了很多特性，可以提供命令补全，命令编辑和命令历史等功能。它还包含了很多 C Shell 和 Korn Shell 中的优点，有灵活和强大的编辑接口，同时又很友好的用户界面。[在 Mac 上将 zsh 用作默认 Shell](https://support.apple.com/zh-cn/HT208050)
+ csh（全称 C Shell），是一种比 Bourne Shell 更适合的变种 shell，它的语法与 C 语言很相似。
+ ksh（全称 Korn Shell），集合了 C Shell 和 Bourne Shell 的优点并且和 Bourne Shell 完全兼容。
+ Tcsh 是 Linux 提供的 C Shell 的一个扩展版本。Tcsh 包括命令行编辑，可编程单词补全，拼写校正，历史命令替换，作业控制和类似 C 语言的语法，它不仅和 Bash Shell 提示符兼容，而且还提供比 Bash Shell 更多的提示符参数。
+ pdksh，是 Linux 系统提供的 ksh 的扩展。pdksh 支持人物控制，可以在命令行上挂起，后台执行，唤醒或终止程序。

&emsp;Linux 的 Shell 种类众多，常见的有：

+ Bourne Shell（/usr/bin/sh 或 /bin/sh）
+ **Bourne Again Shell（/bin/bash）**（现在最流行，也是大多数 Linux 系统默认的 Shell。）
+ C Shell（/usr/bin/csh）
+ K Shell（/usr/bin/ksh）
+ Shell for Root（/sbin/sh）
...

&emsp;在一般情况下，人们并不区分 Bourne Shell 和 Bourne Again Shell，所以，像 `#!/bin/sh`，它同样也可以改为 `#!/bin/bash`。`#!` 告诉当前的命令行环境，其后所指定的程序（程序的路径）即是解释此 shell 脚本文件的 shell 程序（shell 解释器），`#!` 是一个约定的标记，它告诉系统当前这个 shell 脚本需要什么解释器来执行，即使用哪一种 shell 程序来执行此 shell 脚本。

&emsp;那么有没有一种情况，`#!` 后面指定的 shell 程序不存在呢？或者指定路径时指定错了，不在 /bin 目录中，则 shell 脚本就无法执行了。为了保险，也可以写成下面这样：

```c++
#!/usr/bin/env bash
``` 

&emsp;上面命令使用 `env` 命令（这个命令总是在 /usr/bin 目录）直接返回 bash 程序的位置，前提是 bash 的路径是在 `$PATH` 里面。

&emsp;`env` 命令总是指向 /usr/bin/env 文件，或者说，这个二进制文件总是在目录 /usr/bin。

&emsp;`#!/usr/bin/env NAME` 这个语法的意思是，让 shell 查找 `$PATH` 环境变量里面第一个匹配的 NAME。如果你不知道某个命令的具体路径，或者希望兼容其他用户的机器，这样的写法就很有用。

&emsp;如果直接在执行 shell 脚本时指定使用哪个 shell 程序，则会忽略 shell 脚本顶部指定的 `#!/xxx`，例如：`/bin/bash FileName.sh` 会直接使用 `bash` 程序执行 FileName.sh 脚本。

&emsp;使用 `echo $SHELL` 可以查看当前运行的 shell。

```c++
➜  ~ echo $SHELL
/bin/zsh
➜  ~ 
```

## Shell 脚本

&emsp;shell 既是一种命令语言，又是一种程序设计语言。那么什么是 shell 脚本呢？脚本就是 shell 命令组成的文件，这些命令都是可执行程序的名字，脚本不用编译即可通过解释器解释运行。它是一种解释型语言，我们可以用 shell 脚本操作执行动作，用脚本判定命令的执行条件，用脚本来实现动作的批量执行。

&emsp;shell 脚本（shell script），是一种为 shell 编写的脚本程序。大家口中所说的 shell 通常都是指 shell 脚本，但我们自己则要知道，shell 和 shell script 是两个不同的概念。由于习惯的原因，简洁起见，后续出现的 "shell 编程" 都是指 shell 脚本编程，不是指开发 shell 自身。

&emsp;shell 脚本编程跟 JavaScript、php 编程一样，只要有一个能编写代码的文本编辑器和一个能解释执行的脚本解释器就可以了。

## 运行 Shell 脚本有不同的方式

1. 作为可执行程序。将 shell 脚本保存为 FileName.sh，并 cd 到 FileName.sh 所在目录：

```c++
chmod +x ./FileName.sh  # 使脚本具有执行权限
./FileName.sh  # 执行脚本
```

&emsp;注意，一定要写成 ./FileName.sh，而不是 FileName.sh，运行其它二进制的程序也一样，直接写 FileName.sh，linux 系统会去 PATH 里寻找有没有叫 FileName.sh 的，而只有 /bin, /sbin, /usr/bin，/usr/sbin 等在 PATH 里，你的当前目录通常不在 PATH 里，所以写成 FileName.sh 是会找不到的，要用 ./FileName.sh 告诉系统说，就在当前目录找。

&emsp;仅使用 . 时，打开一个子 shell 来读取并执行 FileName 中的命令，该 FileName 文件需要 "执行权限"，运行一个 shell 脚本时会启动另一个命令解释器。

&emsp;chmod + x file 加上执行权限，否则会提示无执行权限。注意执行脚本时候或者全目录，或者 ./FileName.sh ，如果不加的话，linux 默认会从 PATH 里去找该 FileName.sh。

2. 作为解释器参数。这种运行方式是直接运行解释器，其参数就是 shell 脚本的文件名，如：

```c++
/bin/sh FileName.sh
/bin/php FileName.php
```

&emsp;打开一个子 shell 来读取并执行 FileName 中的命令。该 FileName 文件可以无 "执行权限"。运行一个 shell 脚本时会启动另一个命令解释器。

&emsp;这种方式运行 shell 脚本，不需要在 shell 脚本文件第一行指定解释器信息，写了也没用，它会直接使用上面指令中指定的解释器。

> 所以脚本其实就是短小的、用来让计算机自动化完成一系列工作的程序，这类程序可以用文本编辑器修改，不需要编译，通常是解释运行的。

3. source 命令用法。

```c++
source FileName
```

&emsp;在当前 shell 环境下读取并执行 FileName 脚本。该 FileName 文件可以无 "执行权限"。`source` 有一个简写形式，可以使用一个点（.）来表示。

&emsp;`source` 命令最大的特点是在当前 shell 执行脚本，不像直接执行脚本时，会新建一个子 shell。所以 `source` 命令执行脚本时，不需要 export 变量。

## shell(bash) 的基本使用

&emsp;下面我们以 bash 为例来学习 shell 的知识。 

&emsp;波浪线扩展。波浪线 `~` 会自动扩展成当前用户的主目录。

```c++
➜  ~ echo ~
/Users/hmc
➜  ~ cd ~/foo
cd: no such file or directory: /Users/hmc/foo
➜  ~ cd ~/Postman
➜  Postman 
```

&emsp;`~user` 表示扩展成用户 `user` 的主目录，`~user` 会根据波浪线后面的用户名，返回该用户的主目录。如果 `~user` 的 `user` 是不存在的用户名，则波浪号扩展不起作用。 

```c++
bash-3.2$ echo ~
/Users/hmc
bash-3.2$ echo ~foo
~foo
bash-3.2$ echo ~hmc
/Users/hmc
bash-3.2$ echo ~root
/var/root
bash-3.2$ 
```

&emsp;`~+` 会扩展成当前所在的目录，等同于 `pwd` 命令。

```c++
bash-3.2$ echo ~+
/Users/hmc
bash-3.2$ cd ~/Postman
bash-3.2$ echo ~+
/Users/hmc/Postman
bash-3.2$ pwd
/Users/hmc/Postman
bash-3.2$ 
```

&emsp;Bash 将美元符号 `$` 开头的词元视为变量，将其扩展成变量值。变量名除了放在美元符号后面，也可以放在 `${}` 里面。

&emsp;Bash 变量分为环境变量和自定义变量两类。

&emsp;脚本（script）就是包含一系列命令的一个文本文件。Shell 读取这个文件，依次执行里面的所有命令，就好像这些命令是直接输入到命令行一样。所有能够在命令行完成的任务，都能够用脚本完成。

&emsp;脚本的好处是可以重复使用，也可以指定在特定场合自动调用，比如系统启动或关闭时自动执行脚本。

&emsp;if 和 then 写在同一行时，需要分号分隔，分号是 bash 的命令分隔符，它们也可以写成两行，这时不需要分号。

&emsp;上面的 `declare` 命令不仅会输出函数名，还会输出所有定义。输出顺序是按照函数名的字母表顺序。由于会输出很多内容，最好通过管道命令配合 more 或 less 使用。












1⃣️

+ [在 Mac 上将 zsh 用作默认 Shell](https://support.apple.com/zh-cn/HT208050)
+ [macOS各个文件夹的作用](https://www.jianshu.com/p/0c08f2c7748d)
+ [MacOS 修改系统环境变量$Path](https://www.mzh.ren/macos-system-environment-variables.html)

&emsp;今日学习安排：先把 [在 Mac 上将 zsh 用作默认 Shell]、[MacOS 修改系统环境变量$Path]、[macOS各个文件夹的作用] 文章看完，然后整理一下 shell 概述部分，然后开始专心学习 阮一峰老师 的 bash shell 教程。

2⃣️

&emsp;今日学习安排：Shell 的相关知识点已经铺垫完成，接下来阅读 阮一峰老师 的 bash 教程的正文部分。








## 参考链接
**参考链接:🔗**
+ [Bash 脚本教程](https://wangdoc.com/bash/)
+ [快乐的 Linux 命令行](http://billie66.github.io/TLCL/index.html)
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






