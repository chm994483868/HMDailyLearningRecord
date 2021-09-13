# iOS App 启动优化(十二)：LLDB Debugger (LLDB) 进阶

## LLDB Debugger (LLDB) 简述

> &emsp;什么是 REPL？R(read)、E(evaluate)、P(print)、L(loop)。“读取-求值-输出” 循环（英语：Read-Eval-Print Loop，简称 REPL），也被称作交互式顶层构件（interactive toplevel），是一个简单的、交互式的编程环境。这个词常常用于指代一个 [Lisp](https://zh.wikipedia.org/wiki/LISP) 的交互式开发环境，也能指代命令行的模式。REPL 使得 **探索性的编程和调试** 更加便捷，因为 “读取-求值-输出” 循环通常会比经典的 “编辑-编译-运行-调试” 模式要更快。（有时候翻译成交互式解释器。就是你往 REPL 里输入一行代码，它立即给你执行结果。而不用像 C++, Java 那样通常需要编译才能看到运行结果，像 Python Ruby Scala 都是自带 REPL 的语言。）[读取﹣求值﹣输出循环 维基百科](https://zh.wikipedia.org/wiki/读取﹣求值﹣输出循环) [什么是REPL？](https://www.zhihu.com/question/53865469)

&emsp;[LLDB Debugger (LLDB)](https://lldb.llvm.org/resources/contributing.html) 是一个有着 REPL 特性并支持 C++、Python 插件的 **开源调试器**，LLDB 已被内置在 Xcode 中，Xcode 主窗口底部的控制台便是我们与 LLDB 交互的区域。LLDB 允许你在程序运行的特定时刻（如执行到某行代码、某个函数，某个变量被修改、target stop-hook 命中时）暂停它（`breakpoint/br/b/watchpoint/watch/wa`），你可以查看变量的值（`p/po/wa/frame/fr/target/ta`）、执行自定的表达式（`expression/expr`），并且按照你所认为合适的步骤来操作程序（函数）的执行过程，对进程进行流程控制。（[How debuggers work: Part 1 - Basics](https://eli.thegreenplace.net/2011/01/23/how-debuggers-work-part-1.html) 是一个关于调试器如何工作的总体的解释。）

&emsp;[GDB to LLDB command map](https://lldb.llvm.org/use/map.html) 中的一组表格向我们非常好的介绍了 LLDB 调试器提供的几大块命令的总览（常规用法，估计大家都已经掌握了）。除此之外我们可以安装 [Chisel](https://github.com/facebook/chisel/wiki) 来体验 LLDB 更 “高级” 的用法，Chisel 是 facebook 开源的一组  LLDB 命令合集，用于协助我们 **调试 iOS 应用程序**。Chisel 里面的命令正是基于 LLDB 支持 Python 脚本解释器来运行的，Chisel 每条命令对应的 Python 文件保存在 `/usr/local/Cellar/chisel/2.0.1/libexec` 路径下，熟悉 Python 的小伙伴可以试着读一下这些文件的内容（具体路径可能各人机器不同会有所区别，例如 Intel 的 mac 在 `/usr/local/Cellar/chisel/2.0.1/libexec` 路径下，m1 的 mac 在 `/opt/homebrew/Cellar/chisel/2.0.1/libexec` 路径下）。

&emsp;上面我们说到 LLDB 已被内置在 Xcode 中，它是作为一个共享库放在 Xcode 的 SharedFrameworks 下面的，其完整路径是：`/Applications/Xcode.app/Contents/SharedFrameworks/LLDB.framework`。

> &emsp;LLDB 的调试接口本质上是一个 C++ 共享库，在 Mac 系统上，它被打包为 `LLDB.framework`（正常情况下，它存在于 `/Applications/Xcode.app/Contents/SharedFrameworks/LLDB.framework` 路径下），在类 unix 系统上，它是 `lldb.so`（so 是此类系统的共享对象的后缀，so 大概是 shared object 的缩写，在 iOS 和 macOS 中把它们称作系统库、共享库（shared library），在 《程序员的自我修养》那本书中都称作的是：共享对象，所表达的意义是一样的）。 这些调试接口可以在 lldb 的脚本解释器内直接使用，或者可以被引入 lldb.py 模块 的 Python 脚本 使用。LLDB 本身支持用户自定义命令，比如通过脚本可以自定义一个 `pviews` 命令，该命令可以打印 APP 当前的所有视图（该命令已经在 Chisel 中实现）。[lldb-入坑指北（1）-给xcode添加批量启用禁用断点功能](https://cloud.tencent.com/developer/article/1729078)

&emsp;下面我们从 LLDB 的基础命令开始，一步一步深入学习 LLDB 调试器。

## LLDB 基础命令

&emsp;日常我们更多的可能是在 Xcode 中使用 LLDB，今天我们通过 **终端** 与 Xcode 两种方式来对 LLDB 进行学习。熟悉 LLDB 的小伙伴应该都用过 `help`，如果有哪个命令不太熟悉的话，我们直接一个 `help <command>` 回车，LLDB 便会打印出该命令的详细信息，甚至 `help help` 能教我们怎么使用 `help` 命令。下面我们启动终端，然后输入 `LLDB` 并回车，此时便会进入 `LLDB` 环境，然后我们再输入 `help` 命令并回车，便会列出一大组当前机器内可用的 LLDB 命令，看到我的机器把所有的 LLDB 命令分了三组：

+ Debugger commands（LLDB 原始支持的几大块调试命令（例如：breakpoint 断点相关、process 进程相关、thread 线程相关、watchpoint 变量观察点相关））
+ Current command abbreviations (type 'help command alias' for more info)（Debugger commands 中的一些命令或者子命令的别名或缩写）
+ Current user-defined commands（用户自定义调试命令，由于我的机器已经安装了 Chisel，所以这里也直接列出来了 Chisel 提供的调试命令（python 脚本））

&emsp;下面我们就从最简单的 `help` 命令开始学习。

### help

&emsp;直接输入 `help` 命令，可显示所有 LLDB 命令的列表，输入 `help [<cmd-name>]` 便可列出 `<cmd-name>` 这个特定命令的详细信息。例如输入 `help help` 命令并回车，可看到如下描述：

&emsp;下面 `-a`、`-h`、`-u` 选项的用法可直接忽略，感觉没啥实际用处。

```c++
help help
     Show a list of all debugger commands, or give details about a specific command.
     (显示所有调试器命令的列表，或提供有关特定命令的详细信息。)

Syntax: help [<cmd-name>]

Command Options Usage（help 命令后面可跟的命令选项用法）:

  // help 不仅可以查看单个命令的详细信息，还可以查看子命令的详细信息，
  // 例如：help breakpoint（查看 breakpoint 命令的详细信息），help breakpoint clear（查看 breakpoint clear 命令的详细信息）
  help [-ahu] [<cmd-name> [<cmd-name> [...]]]

       -a ( --hide-aliases )( -a 是 --hide-aliases 选项的缩写)
            Hide aliases in the command list.
            (相对 help 命令打印所有的命令列表，help -a 会隐藏 help 列出的三组命令中的 Current command abbreviations 组的命令)

       -h ( --show-hidden-commands )(-h 是 --show-hidden-commands 选项的缩写)
            Include commands prefixed with an underscore.
            (help -h 和 help 相同，会把所有命令列出来)

       -u ( --hide-user-commands )(--hide-user-commands 是 -u 选项的缩写)
            Hide user-defined commands from the list.
            (相对 help 命令打印所有的命令列表，help -u 会隐藏 help 列出的三组命令中的 Current user-defined commands 分组的命令)
     
     This command takes options and free-form arguments.  If your arguments resemble option specifiers (i.e., they start with a - or --), you
     must use ' -- ' between the end of the command options and the beginning of the arguments.
     // 这里有很重要的一句：help 命令采用 options 和 free-form（自由格式的）参数。如果你的的参数类似于 option 说明符（例如，它们以 - 和 -- 开头），你必须在命令 options 的结尾和参数的开头之间使用 `--` 分隔。
     // 即当一个命令即有选项也有参数时，选项要放在前面，然后在它的结尾要插入 `--` 与后面的参数分隔开。
```

### print/po/p

&emsp;在 LLDB 调试过程中，打印命令大概是我们用的最多的命令了，`print/prin/pri/p  /po` 都是打印命令，打印命令的本质是执行打印命令后面的表达式并输出执行结果，任意的一个变量名也是一个表达式，例如我们在 LLDB 调试窗口中执行如下命令:

+ `p a` （a 是一个变量名，这里便是执行 a 这个表达式并输出结果，即为 a 的值）
+ `p a + 2` （这里便是执行 a + 2 这个表达式并输出结果，即为 a + 2 的值）

&emsp;LLDB 实际上会作前缀匹配，所以我们使用 `print/prin/pri/p` 是完全一样的，但是我们不能使用 `pr`，因为 LLDB 不能消除 `print` 和 `process` 两者的歧义，不过幸运的是我们可以直接使用 `p`（大概在 LLDB 调试时 `p` 打印命令用的是最多的），LLDB 把 `p` 这个最简单的 缩写/别名 归给了 **打印命令** 使用。（如下我们使用 `help pr` 命令，会提示我们使用了不明确的命令名称，并列出了所有以 `pr` 开头的命令，来提示我们具体想要查看哪个命令的详细信息。）

```c++
(lldb) help pr
Help requested with ambiguous command name, possible completions:
    process
    print
    present
    presponder
```

&emsp;下面我们区分一下 `print/prin/pri/p` 和 `po`，虽然它们都是打印命令，但是它们的打印格式并不相同。

&emsp;分别通过 `help print`、`help p`、`help po` 输出的帮助信息，我们可以看到 `print/p` 是作用完全一样的命令，`print/p` 都是 `expression --` 命令的缩写，它们都是根据 LLDB 的默认格式来进行打印操作，而 `po` 则是 `expression -O  --` 的缩写，如果大家阅读的认真的话，应该还记得 `--` 是用来标记命令的选项结束的，`--` 前面是命令的选项，`--` 后面是命令的参数。`expression -O  --` 中的 `-O` 选项是 `--object-description` 的缩写：`-O ( --object-description ) Display using a language-specific description API, if possible.` 即针对 Objective-C 的话，便是调用 `description` 实例函数或者类函数，即使用 `po` 命令进行打印时，它是根据当前语言环境，调用 `description` API 返回的结果进行打印操作。（在 OC 中我们可以通过重写 `+/-description` 函数便可得到自定义的打印结果。）   

&emsp;即看到这里面我们便明白了 `print/prin/pri/po/p` 都是调用 `expression` 命令，执行表达式（变量也是一个表达式）并输出表达式的返回值。

```c++
print    -- Evaluate an expression on the current thread.  Displays any returned value with LLDB's default formatting.
p        -- Evaluate an expression on the current thread.  Displays any returned value with LLDB's default formatting.
po       -- Evaluate an expression on the current thread.  Displays any returned value with formatting controlled by the type's author.
```

&emsp;`help print` 和 `help p` 都输出如下帮助信息：

```c++

(lldb) help print
     Evaluate an expression on the current thread.  Displays any returned value with LLDB's default formatting.  Expects 'raw' input (see 'help raw-input'.)

Syntax: print <expr>

Command Options Usage:
  print <expr>


'print' is an abbreviation for 'expression --'
```

&emsp;`help po` 则输出如下帮助信息：

```c++
(lldb) help po
     Evaluate an expression on the current thread.  Displays any returned value with formatting controlled by the type's author.  Expects 'raw' input (see 'help raw-input'.)

Syntax: po <expr>

Command Options Usage:
  po <expr>


'po' is an abbreviation for 'expression -O  --'
```

&emsp;根据帮助信息我们可以看到 `p/po` 仅能在当前线程使用（`expression` 仅用于当前线程），也就是说我们仅能打印当前线程的表达式的返回值，如果我们打印其它线程的表达式的话会得到一个未定义的错误：`error: <user expression 0>:1:1: use of undeclared identifier 'xxx'`，`xxx` 代指我们要打印的变量名。如下示例代码：

```c++
...
int main_a = 10;
NSLog(@"%d", main_a);

dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
    int local_variable = 11;
    NSLog(@"🎉🎉🎉 local_variable = %d", local_variable);
});
...

// 我们在 dispatch_async 内部定义一个断点，执行程序，命中断点时，我们执行 p main_a 命令，便打印如下错误：

(lldb) p main_a
error: <user expression 0>:1:1: use of undeclared identifier 'main_a'
main_a
^

```

&emsp;如果我们使用过 `p` 命令的话，一定还记得每次 `p` 命令打印的结果中都会直接输出一个以 `$` 开头的变量，此变量的值便是我们要打印的表达式的返回值，即我们可以把此 `$` 开头的变量理解为我们使用 `p` 命令打印的结果值的变量，这些以 `$` 开头的变量都是存在于 LLDB 的命名空间中的，在后续的命令操作中我们可以直接根据它们的名字使用它们（如果有看过之前文章的话，在类结构一章中，我们正是根据 `p` 命令输出的 `$` 开头的变量，一层一层根据 `$` 开头的变量的值进行强制转换来查看类的结构的）。这里的变量名之所以使用 `$` 开头也对应了，下面要讲的在 LLDB 的命名空间中自定义变量，变量名需要以 `$` 开头。

```c++
(lldb) p 123
(int) $0 = 123

(lldb) p $0 + 7
(int) $1 = 130

(lldb) p $1
(int) $1 = 130
```

&emsp;在 `p` 命令的详细信息中我们还看到了：`Displays any returned value with LLDB's default formatting.` 提到了以 LLDB 的默认格式来输出返回值，那么除了 LLDB 的默认格式，还有其它格式吗？如果是这样问那必定是有的，大概我们用的最多的便是 `p/x`：以十六进制输出返回值，`p/t`：以二进制输出返回值（字母 t 代表 two），`p/d`：以有符号十进制输出返回值，`p/u`：以无符号十进制输出返回值。我们还可以使用 `p/c` 打印字符，或者 `p/s` 打印以空字符（`\0`）结尾的字符串。如下简单示例打印：

```c++
// 默认格式
(lldb) p 123
(int) $0 = 123

// 十六进制
(lldb) p/x 123
(int) $1 = 0x0000007b

// 二进制
(lldb) p/t 123
(int) $2 = 0b00000000000000000000000001111011
(lldb)

// 整数转字符
(lldb) p/c 65
(int) $9 = A\0\0\0

// 字符串打印
(lldb) p/s "qwer"
(const char [5]) $10 = "qwer"
```

&emsp;[10.5 Output Formats](https://sourceware.org/gdb/onlinedocs/gdb/Output-Formats.html) 中有较完整的打印格式列出，感兴趣的小伙伴可以阅读一下。 

### expression

&emsp;通过上面的学习我们已知，`p` 是 `expression --` 的缩写，`po` 是 `expression -O  --` 的缩写，那么本小节我们来单独学一下 `expression` 命令，`expr` 是其缩写。

&emsp;在之前学习或使用 `po/p` 命令时，我们的目光更多的是聚焦在它的打印功能上，难道它只能打印表达式（变量也属于表达式）的返回值吗？当然不是，即然是打印表达式的返回值，那么如果表达式是一个函数、是一个赋值语句等形式呢？如果要打印表达式的返回值，那么是不是说要连带着执行了表达式才能得到表示式的返回值呢？那如果表达式执行过程中修改了我们程序中的变量的值呢？修改的值会保留在变量中吗？答案是会的，不仅会修改 LLDB 命令空间中变量的值，同时还会修改我们程序中的变量的值。如下示例：

```c++
dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
    int local_variable = 11; // ⬅️ 此行打一个断点进行 LLDB 调试
    NSLog(@"🎉🎉🎉 local_variable = %d", local_variable);
});

// 控制台输出：

(lldb) expression local_variable
(int) $0 = 11

(lldb) expression $0 = $0 + 11
(int) $1 = 22

(lldb) expression local_variable
(int) $2 = 22

(lldb) expression local_variable = 33
(int) $3 = 33

(lldb) expression local_variable
(int) $4 = 33

(lldb) expression $2
(int) $2 = 22

// ⬇️ step 执行到下一条语句，可看到 NSLog 输出 local_variable 变量的值已经是 33
(lldb) step
2021-09-05 10:32:55.667121+0800 TEST_Fishhook[29717:14465661] 🎉🎉🎉 local_variable = 33
```

&emsp;通过上面的示例我们是不是会想到了什么，我们可以直接通过 LLDB 命令来改变程序中变量的值，而不需要我们重新编译运行程序，这样我们是不是就可以直接在调试的过程中修改变量的值进而改变程序的执行流程了，而且我们完全不需要重新编译运行程序。

&emsp;上面一小段我们聚焦于使用 `expression` 来修改程序中变量的值（`po/p` 也能作同样的事，毕竟它们都是 `expression` 相关的缩写），那么我们能不能在 LLDB 中创建变量呢？`expression` 后面直接跟变量声明表达式是不是就创建一个变量了呢？答案是肯定的，例如我们在程序中使用 `int a = 2;` 来声明一个变量一样，我们在 LLDB 中使用 `expression` 命令也能做同样的事情，不过为了能在 LLDB 的命令空间中继续使用声明的变量，变量的名称必须以 `$` 开头。如下使用示例：

```c++
// 声明一个 int 类型的变量 $a
(lldb) e int $a = 2

// 打印 $a 变量与 19 相乘的结果值
(lldb) p $a * 19
(int) $0 = 38

// 声明一个数组变量 $array
(lldb) e NSArray *$array = @[@"Saturday", @"Sunday", @"Monday"]

// 打印 $array 的长度
(lldb) p [$array count]
(NSUInteger) $1 = 3

// 取出 $array 数组中下标为 0 的变量并把所有字符转为大写
(lldb) po [[$array objectAtIndex:0] uppercaseString]
SATURDAY

// 取出 $array 数组中下标为 2 的变量，是一个字符串，然后取出该字符串的第一个字符，由于 LLDB 无法确定返回值的类型，所以这里直接给了一个错误
(lldb) p [[$array objectAtIndex:$a] characterAtIndex:0]
error: <user expression 5>:1:28: no known method '-characterAtIndex:'; cast the message send to the method's return type
[[$array objectAtIndex:$a] characterAtIndex:0]
~~~~~~~~~~~~~~~~~~~~~~~~~~~^~~~~~~~~~~~~~~~~~~

// 标明返回值类型便能得到正确的打印
(lldb) p (char)[[$array objectAtIndex:$a] characterAtIndex:0]
(char) $3 = 'M'

// 把字符转为整数输出
(lldb) p/d (char)[[$array objectAtIndex:$a] characterAtIndex:0]
(char) $4 = 77
```

&emsp;到这里我们看到了 `expression` 可以进行打印、修改、声明变量。下面我们看一下 `help expression` 命令输出的关于 `expression` 命令的详细信息，其中涉及的命令选项较多，这里就不一一列举了，下面主要看下其中几个比较重要的信息。

#### Single and multi-line expressions（单行/多行表达式）

&emsp;关于单行和多行表达式，当我们要输入多行表达式时，我们可以首先输入 `expression/expr/e` 命令，然后回车，即可进入 LLDB 多行表达式编辑模式，然后我们便开始输入自己想要的表达式，然后当我们要结束表达式时，我们在一个空行处输入回车，即结束多行表达式的编辑并执行我们输入的多行表达式。

```c++
Single and multi-line expressions:

    The expression provided on the command line must be a complete expression with no newlines.  To evaluate a multi-line expression, hit a return after an empty expression, and lldb will enter the
    multi-line expression editor. Hit return on an empty line to end the multi-line expression.
    
```

&emsp;如下使用示例：

```c++
// 输入 expression/expr/e 后直接回车便进入 LLDB 多行表达式编辑模式，如果不输入任何内容，再回车便退出编辑模式
(lldb) expression
Enter expressions, then terminate with an empty line to evaluate:
1 

(lldb) expr
Enter expressions, then terminate with an empty line to evaluate:
1

(lldb) e
Enter expressions, then terminate with an empty line to evaluate:
1

// 进入 LLDB 多行表达式编辑模式，回车再回车便退出编辑模式并执行
(lldb) e 
Enter expressions, then terminate with an empty line to evaluate:
1 local_variable = local_variable + 10
2 
(int) $0 = 21

// local_variable 变量的值已经被修改为 21
(lldb) p local_variable
(int) $1 = 21 
```

#### Timeouts（表达式执行超时时间）

&emsp;如果表达式可以静态计算（无需运行代码），那么它将直接执行。否则，默认情况下，表达式将在当前线程上运行，但超时时间很短：当前为 0.25 秒。如果在那段时间内没有返回，表达式执行将被中断并把所有线程运行时恢复。你可以使用 `-a` 选项禁用对所有线程的重试。你也可以使用 `-t` 选项设置更短的超时时间。

```c++
Timeouts:

    If the expression can be evaluated statically (without running code) then it will be.  Otherwise, by default the expression will run on the current thread with a short timeout: currently .25
    seconds.  If it doesn't return in that time, the evaluation will be interrupted and resumed with all threads running.  You can use the -a option to disable retrying on all threads.  You can use the
    -t option to set a shorter timeout.
```

#### User defined variables（用户在 LLDB 中自定义变量）

&emsp;为方便起见，你可以定义自己的变量或在后续表达式中使用。定义它们的方式与在 C 语言中定义变量的方式相同。如果定义的变量的名字的第一个字符是 `$`，那么该变量的值将在后面的表达式中可用，否则它仅在当前表达式中可用。

```c++
User defined variables:

    You can define your own variables for convenience or to be used in subsequent expressions.  You define them the same way you would define variables in C.  If the first character of your user
    defined variable is a $, then the variable's value will be available in future expressions, otherwise it will just be available in the current expression.
```

#### Continuing evaluation after a breakpoint（断点命中后继续执行）

```c++
Continuing evaluation after a breakpoint:

    If the "-i false" option is used, and execution is interrupted by a breakpoint hit, once you are done with your investigation, you can either remove the expression execution frames from the stack
    with "thread return -x" or if you are still interested in the expression result you can issue the "continue" command and the expression evaluation will complete and the expression result will be
    available using the "thread.completed-expression" key in the thread format.
```

&emsp;下面是 `expr` 的一些使用示例：

```c++
Examples:

    // 赋值
    expr my_struct->a = my_array[3]
    
    // 计算
    expr -f bin -- (index * 8) + 5
    
    // 声明变量（在接下来的整个 LLDB 环境中都可用）
    expr unsigned int $foo = 5
    
    // 声明变量（仅在当前行可用）
    expr char c[] = \"foo\"; c[0]
     
     // 由于此命令采用 `raw` 输入，因此如果你使用任何命令选项，则必须在命令选项的结尾和原始输入的开头之间使用 `--`。如：`expr -f bin -- (index * 8) + 5`
     Important Note: Because this command takes 'raw' input, if you use any command options you must use ' -- ' between the end of the command options and the beginning of the raw input.
```

### thread/process/frame

&emsp;当我们在 Xcode 中运行程序，并命中我们在程序中添加的断点时，此时进程执行便暂停到我们的断点处（此时进程中的其他线程都会暂停，`thread list` 可列出当前所有线程，其中标星的是当前线程）。Xcode 底部的控制台便会进入 LLDB 调试模式，调试条上的  `Pause program execution/Continue Program execution`：暂停/继续 按钮，此时会处于 `Continue` 状态，点击它我们的进程便会继续无休止的执行下去直到结束或者再次命中我们的程序中的下一个断点。当我们的进程进入 LLDB 调试状态时，暂停/继续 按钮右边的三个按钮也会变成高亮的可点击状态（这三个按钮只有程序进入 LLDB 调试模式后才会变成可点击状态，程序正常运行时它们都是灰色不可点击的） ，此时我们便有了四个可以用来控制程序执行流程的按钮（如果加上旁边的 激活/关闭 所有断点的按钮的话，那我们便有了 5 个可以控制程序执行流程的按钮）。依次如下按钮：

1. `Activate breakpoints/Deactivate breakpoints`（激活/失活所有断点）
2. `Pause program execution/Continue Program execution`（暂停/继续 程序执行，对应的 LLDB 命令是：`process interrupt/process continue` ）
3. `Step over/Step over instruction(hold Control)/Step over thread(hold Control-Shift)`
  + `Step over`：直接点击按钮进行源码级别的单行代码步进
  + `Step over instruction(hold Control)`：按住键盘上的 Control 按钮点击此按钮进行汇编指令级别的单行指令步进，且其它线程也都会执行。
  + `Step over thread(hold Control-Shift)`：同时按住键盘上的 Control 和 Shift 按钮点击此按钮进行源码级别的单行代码步进，仅执行当前线程并继续暂停其他的线程，上面的两者则都是继续执行所有线程，所以这个 `thread` 级别的 `Step over` 可以保证调试只在当前线程进行，屏蔽其他线程对当前线程的影响。
4. `Step into/Step into instruction(hold Control)/Step into thread(hold Control-Shift)`（同上，源码级别、汇编指令级别、仅执行当前线程。和 `Step over` 不同的是，当单行代码是一个函数调用时，点击此按钮会进入函数内部，其他则基本和 `Step over` 相同。）
5. `Step out`

&emsp;下面我们对这些按钮进行详细学习：

&emsp;`Activate breakpoints/Deactivate breakpoints`：激活/失活 全部断点，例如当我们想要关闭所有断点想要程序直接执行下去，看它最终呈现的页面效果时，我们可以先使用此按钮失活所有断点，然后点击 `Continue Program execution` 按钮即可。在 LLDB 调试器中我们可以使用 `breakpoint disable` 和 `breakpoint enable` 达到同样的效果（有一点细微差别，感兴趣的小伙伴可以自己试一下）。

```c++
(lldb) breakpoint enable
All breakpoints enabled. (7 breakpoints)
(lldb) breakpoint disable
All breakpoints disabled. (7 breakpoints)
```

&emsp;`Pause program execution/Continue Program execution`：暂停/继续 执行程序（对应的 LLDB 命令是：`process interrupt/process continue`）。当程序处于暂停状态时，点击此按钮可使程序继续执行下去，直到遇到下一个断点，或者没有下一个断点的话程序一直执行下去。在 LLDB 调试器中我们可以使用 `process continue/continue/c` 可达到同样的效果（`continue/c` 是 `process continue` 的缩写）。它们后面还可以跟一个 `-i` 选项，下面是 `c` 命令的帮助信息：

&emsp;（下面帮助信息中指出了 `c` 是继续执行当前进程的 **所有线程**，那有没有只执行当前线程的命令呢？答案是有的，下面我们会揭晓。）

```c++
(lldb) help c
     Continue execution of all threads in the current process.
     继续执行当前进程中的所有线程。

Syntax: c <cmd-options>

Command Options Usage:
  c [-i <unsigned-integer>]

       -i <unsigned-integer> ( --ignore-count <unsigned-integer> )
            Ignore <N> crossings of the breakpoint (if it exists) for the currently selected thread.
            忽略当前选定线程的断点（如果存在）的 <N> 个交叉点。

'c' is an abbreviation for 'process continue'
```

&emsp;`-i` 参数没看太懂什么意思，只找到一个链接：[Repeating Command in LLDB](https://stackoverflow.com/questions/64639184/repeating-command-in-lldb)，大概是跳过指定个数的（重复的）断点，例如代码使用示例：

```c++
int j = 0;

while (true) {
    j++;
    NSLog(@"%d", j);
}

// 我们在 while 处打一个断点，然后运行程序后断点被命中，在 LLDB 调试器中输入: c -i 3

// 控制台打印：
(lldb) c -i 3
Process 85687 resuming
2021-09-08 09:44:24.859226+0800 TEST_Fishhook[85687:1017243] 1
2021-09-08 09:44:24.866394+0800 TEST_Fishhook[85687:1017243] 2
2021-09-08 09:44:24.873266+0800 TEST_Fishhook[85687:1017243] 3
2021-09-08 09:44:24.878565+0800 TEST_Fishhook[85687:1017243] 4
(lldb) 
```

&emsp;接下来我们看下一个按钮的功能：

&emsp;`Step over/Step over instruction(hold Control)/Step over thread(hold Control-Shift)` 会以黑盒的方式执行一行代码。即使这行代码是一个函数调用的话也是直接执行，并不会跳进函数内部，对比 `Step into/Step into instruction(hold Control)/Step into thread(hold Control-Shift)` 的话，它则是可以跳进（单行代码）所调用的函数内部，当然仅限于我们自己的自定义的函数，系统那些闭源的函数我们是无法进入的。在 LLDB 调试中我们可以使用 `thread step-over/next/n` 命令（`next/n` 是 `thread step-over` 的缩写）达到同样的效果。

```c++
n         -- Source level single step, stepping over calls.  Defaults to current thread unless specified.
next      -- Source level single step, stepping over calls.  Defaults to current thread unless specified.
nexti     -- Instruction level single step, stepping over calls.  Defaults to current thread unless specified.
ni        -- Instruction level single step, stepping over calls.  Defaults to current thread unless specified.
```

&emsp;`n/next` 是源码级别，且会跳过函数调用。`nexti/ni` 是汇编指令级别的，且同样会跳过函数调用。  

&emsp;`Step into/Step into instruction(hold Control)/Step into thread(hold Control-Shift)` 对比上面的一个按钮，如果你想跳进一个函数调用进行调试或者查看它内部的执行情况便可以使用此按钮，如果当前行不是函数调用时，`Step over/Step into` 的执行效果是一样的。在 LLDB 调试中我们可以使用 `thread step-in/thread step-inst/step-inst-over/s/step/si/stepi` 命令达到同样的效果。

```c++
thread step-in        -- Source level single step, stepping into calls.  Defaults to current thread unless specified.
thread step-inst      -- Instruction level single step, stepping into calls.  Defaults to current thread unless specified.
thread step-inst-over -- Instruction level single step, stepping over calls.  Defaults to current thread unless specified.

s                     -- Source level single step, stepping into calls.  Defaults to current thread unless specified.
step                  -- Source level single step, stepping into calls.  Defaults to current thread unless specified.
si                    -- Instruction level single step, stepping into calls.  Defaults to current thread unless specified.
stepi                 -- Instruction level single step, stepping into calls.  Defaults to current thread unless specified.
```

&emsp;`Step out` 如果我们使用 `Step into` 进入了一个函数调用，但是它的内容特别长，我们不想一次一次重复点击 `Step into/Step over` 按钮直到函数执行完成，我们想快速把当前这个函数执行完成，那么我们就可以点击 `Step out` 按钮来完成此操作。`Step out` 会继续执行到下一个返回语句（直到一个堆栈帧结束）然后再次停止。在 LLDB 调试中我们可以使用 `thread step-out/finish` 命令达到同样的效果。

```c++
thread step-out       -- Finish executing the current stack frame and stop after returning.  Defaults to current thread unless specified.
finish                -- Finish executing the current stack frame and stop after returning.  Defaults to current thread unless specified.
```

&emsp;除了上面 Xcode 直接提供的调试按钮所对应的调试命令外，还有一个特别有用的命令：`thread return`，它有一个可选参数，在执行时它会把可选参数加载进返回寄存器中，然后立刻执行返回命令，跳出当前栈帧。这意味着这函数的剩余部分 **不会被执行**。这会给 ARC 的引用计数造成一些问题，或者会使函数内的清理部分失效。但是在函数开头执行这个命令，是个非常好的隔离这个函数、伪造函数返回值的方式。

```c++
(lldb) help thread return
     Prematurely return from a stack frame, short-circuiting execution of newer frames and optionally yielding a specified value.  Defaults to the exiting the current stack frame.  Expects 'raw' input
     (see 'help raw-input'.)

Syntax: thread return

Command Options Usage:
  thread return [-x] -- [<expr>]
  thread return [<expr>]

       -x ( --from-expression )
            Return from the innermost expression evaluation.
     
     Important Note: Because this command takes 'raw' input, if you use any command options you must use ' -- ' between the end of the command options and the beginning of the raw input.
(lldb) 
```

&emsp;上面我们使用到的命令基本都是位于 `thread` 命令下的，其中还涉及到 `frame` 命令（如查看当前函数调用栈帧内容等）、`process` 命令（控制进程继续执行等）。下面我们直接列出它们的子命令以加深学习印象。

#### thread

&emsp;`thread`：用于在当前进程中的一个或多个线程上操作的命令。

```c++
(lldb) help thread
     Commands for operating on one or more threads in the current process.

Syntax: thread <subcommand> [<subcommand-options>]

The following subcommands are supported:

      backtrace      -- Show thread call stacks.  Defaults to the current thread, thread indexes can be specified as arguments.
                        Use the thread-index "all" to see all threads.
                        Use the thread-index "unique" to see threads grouped by unique call stacks.
                        Use 'settings set frame-format' to customize the printing of frames in the backtrace and 'settings set thread-format' to customize the thread header.
                        显示线程调用堆栈。默认为当前线程，也可以指定线程，使用线程索引作为参数。
                        使用线程索引 "all" 查看所有线程。
                        使用线程索引 "unique" 查看按唯一调用堆栈分组的线程。
                        使用 'settings set frame-format' 自定义回溯中帧的打印，使用 'settings set thread-format' 自定义线程标题。
                        
      continue       -- Continue execution of the current target process.  One or more threads may be specified, by default all threads continue.
                        继续执行当前目标进程。可以指定一个或多个线程，默认情况下所有线程都继续执行。(thread continue <thread-index> [<thread-index> [...]])
                    
      exception      -- Display the current exception object for a thread. Defaults to the current thread.
                        显示线程的当前异常对象。默认为当前线程。
    
      info           -- Show an extended summary of one or more threads.  Defaults to the current thread.
                        显示一个或多个线程的扩展摘要。默认为当前线程。
                        
      jump           -- Sets the program counter to a new address.
                        将程序计数器(pc 寄存器)设置为新地址。
                        
      list           -- Show a summary of each thread in the current target process.  Use 'settings set thread-format' to customize the individual thread listings.
                        显示当前目标进程中每个线程的摘要。使用 'settings set thread-format' 来自定义单个线程列表。
                        
      plan           -- Commands for managing thread plans that control execution.
                        用于管理控制执行的线程计划的命令。
      
      return         -- Prematurely return from a stack frame, short-circuiting execution of newer frames and optionally yielding a specified value.  Defaults to the exiting the current stack frame. 
                        Expects 'raw' input (see 'help raw-input'.)
                        过早地从栈帧返回，短路执行较新的栈帧，并可选指定返回值。 默认退出当前栈帧。（例如我们使用 thread step-in 命令进入了一个返回值是 bool 的函数，此时我们直接输入 thread return NO 命令回车，那么这次函数调用便立即结束，且返回 NO。）
                        
      select         -- Change the currently selected thread.
                        更改当前选定的线程。
                        
      step-in        -- Source level single step, stepping into calls.  Defaults to current thread unless specified.
                        源级单步，步入函数调用。 除非指定，否则默认在当前线程。
      
      step-inst      -- Instruction level single step, stepping into calls.  Defaults to current thread unless specified.
                        指令级别单步，步入函数调用。 除非指定，否则默认在当前线程。
      
      step-inst-over -- Instruction level single step, stepping over calls.  Defaults to current thread unless specified.
                        指令级别单步，跨过函数调用。 除非指定，否则默认在当前线程。
      
      step-out       -- Finish executing the current stack frame and stop after returning.  Defaults to current thread unless specified.
                        完成执行当前栈帧（函数调用）并在返回后停止。 除非指定，否则默认在当前线程。
      
      step-over      -- Source level single step, stepping over calls.  Defaults to current thread unless specified.
                        源级单步，跨过函数调用。 除非指定，否则默认在当前线程。
      
      step-scripted  -- Step as instructed by the script class passed in the -C option.  You can also specify a dictionary of key (-k) and value (-v) pairs that will be used to populate an
                        SBStructuredData Dictionary, which will be passed to the constructor of the class implementing the scripted step.  See the Python Reference for more details.
                        按照 -C 选项中通过的脚本类指示的步骤。还可以指定 key（-k）和 value（-v）对的 dictionary，用于填充 SBStructuredData Dictionary，它将被传递给实现脚本化步骤的类的构造函数。
                        
      until          -- Continue until a line number or address is reached by the current or specified thread.  Stops when returning from the current function as a safety measure.  The target line
                        number(s) are given as arguments, and if more than one is provided, stepping will stop when the first one is hit.
                        继续，直到当前或指定线程到达行号或地址。作为安全措施，从当前函数返回时停止。target 行号作为参数给出，如果提供了多个行号，则当第一个行号被击中时，步进将停止。

For more help on any particular subcommand, type 'help <command> <subcommand>'.
```

#### process

&emsp;`process` 与当前平台上的进程进行交互的命令。

```c++
(lldb) help process
     Commands for interacting with processes on the current platform.

Syntax: process <subcommand> [<subcommand-options>]

The following subcommands are supported:

      attach    -- Attach to a process.
                   附加到进程。
                   
      connect   -- Connect to a remote debug service.
                   连接到远程调试服务。
                   
      continue  -- Continue execution of all threads in the current process.
                   继续执行当前进程中的所有线程。
                   
      detach    -- Detach from the current target process.
                   分离当前目标进程。
      
      handle    -- Manage LLDB handling of OS signals for the current target process.  Defaults to showing current policy.
                   管理当前目标进程的操作系统信号的 LLDB 处理。 默认显示当前策略。
      
      interrupt -- Interrupt the current target process.
                   中断当前目标进程。
      
      kill      -- Terminate the current target process.
                   终止当前目标进程。
      
      launch    -- Launch the executable in the debugger.
                   在调试器中启动可执行程序。
      
      load      -- Load a shared library into the current process.
                   将共享库加载到当前进程中。
      
      plugin    -- Send a custom command to the current target process plug-in.
                   将自定义命令发送到当前目标进程插件。
      
      save-core -- Save the current process as a core file using an appropriate file type.
                   使用适当的文件类型将当前进程保存为 core file。
      
      signal    -- Send a UNIX signal to the current target process.
                   向当前目标进程发送 UNIX 信号。
      
      status    -- Show status and stop location for the current target process.
                   显示当前目标进程的状态和停止位置。
      
      unload    -- Unload a shared library from the current process using the index returned by a previous call to "process load".
                   使用以前调用的 "process load" 返回的索引从当前进程卸载共享库。

For more help on any particular subcommand, type 'help <command> <subcommand>'.
```

#### frame

&emsp;`frame` 用于选择和检查当前线程堆栈帧的命令。

```c++
(lldb) help frame
     Commands for selecting and examing the current thread's stack frames.

Syntax: frame <subcommand> [<subcommand-options>]

The following subcommands are supported:

      info       -- List information about the current stack frame in the current thread.
                    列出当前线程中当前堆栈帧的信息。
      
      recognizer -- Commands for editing and viewing frame recognizers.
                    用于编辑和查看帧识别器的命令。
      
      select     -- Select the current stack frame by index from within the current thread (see 'thread backtrace'.)
                    从当前线程中按索引选择当前堆栈帧（参见 'thread backtrace'）。
      
      variable   -- Show variables for the current stack frame. Defaults to all arguments and local variables in scope. Names of argument, local, file static and file global variables can be specified.
                    Children of aggregate variables can be specified such as 'var->child.x'.  The -> and [] operators in 'frame variable' do not invoke operator overloads if they exist, but directly
                    access the specified element.  If you want to trigger operator overloads use the expression command to print the variable instead.
                    It is worth noting that except for overloaded operators, when printing local variables 'expr local_var' and 'frame var local_var' produce the same results.  However, 'frame
                    variable' is more efficient, since it uses debug information and memory reads directly, rather than parsing and evaluating an expression, which may even involve JITing and running
                    code in the target program.
                    显示当前堆栈帧的变量。默认是作用域内的所有参数和局部变量。可以指定 argument、local、file static 和 file global 变量的名称。可以指定聚合变量的子级，例如 'var->child.x'。'frame variable' 中的 -> 和 [] 运算符如果存在，则不会调用运算符重载，而是直接访问指定的元素。如果你想触发操作符重载，请使用 'expression' 命令打印变量。值得注意的是，除了重载操作符外，打印本地变量 'expr local_var' 和 'frame var local_var' 时也会产生相同的结果。然而，'frame
                    variable' 更有效，因为它直接使用调试信息和内存读取，而不是解析和计算表达式，甚至可能涉及 JITing 和运行目标程序中的代码。

For more help on any particular subcommand, type 'help <command> <subcommand>'.
```

### breakpoint

&emsp;在日常开发中，我们把断点作为一个停止程序运行，检查当前状态，追踪 bug 的方式。在 Xcode 中进行可视化的断点调试非常的方便，感觉比使用 LLDB 的命令要方便很多，那么我们接下来看一下 Xcode 中的可视化的断点操作与 LLDB 调试器中断点命令的对应关系。

#### breakpoint list/disable/enable/delete

&emsp;点击 Xcode 左侧面板的 Show the Breakpoint navigator 按钮，我们便切换到了一个可以快速管理所有断点（仅限于我们在 Xcode 中使用图形界面添加的断点，不包含我们使用 LLDB 命令添加的断点）的面板。在 LLDB 调试窗口中我们可以使用 `breakpoint list/br li` 命令列出所有的断点，包括我们使用 Xcode 图形界面添加的和使用 LLDB 命令添加的所有断点。还有如下的 LLDB breakpoint 命令，在 Xcode 中清点按钮即可做到同样的效果：

+ `breakpoint disable [<breakpt-id | breakpt-id-list>]` 关闭断点
+ `breakpoint enable [<breakpt-id | breakpt-id-list>]` 打开断点
+ `breakpoint delete <cmd-options> [<breakpt-id | breakpt-id-list>]` 删除断点

&emsp;如下使用示例：

```c++
(lldb) br li
Current breakpoints:
1: file = '/Users/hmc/Documents/iOSSample/TEST_Fishhook/TEST_Fishhook/main.m', line = 29, exact_match = 0, locations = 1, resolved = 1, hit count = 1

  1.1: where = TEST_Fishhook`main + 34 at main.m:29:29, address = 0x0000000102a2df32, resolved, hit count = 1 

2: file = '/Users/hmc/Documents/iOSSample/TEST_Fishhook/TEST_Fishhook/main.m', line = 30, exact_match = 0, locations = 1, resolved = 1, hit count = 0

  2.1: where = TEST_Fishhook`main + 56 at main.m:30:29, address = 0x0000000102a2df48, resolved, hit count = 0 

(lldb) br dis 1
1 breakpoints disabled.
(lldb) br li
Current breakpoints:

// ⬇️ 看到断点 1 已经被置为 disabled

1: file = '/Users/hmc/Documents/iOSSample/TEST_Fishhook/TEST_Fishhook/main.m', line = 29, exact_match = 0, locations = 1 Options: disabled 

  1.1: where = TEST_Fishhook`main + 34 at main.m:29:29, address = 0x0000000102a2df32, unresolved, hit count = 1 

2: file = '/Users/hmc/Documents/iOSSample/TEST_Fishhook/TEST_Fishhook/main.m', line = 30, exact_match = 0, locations = 1, resolved = 1, hit count = 0

  2.1: where = TEST_Fishhook`main + 56 at main.m:30:29, address = 0x0000000102a2df48, resolved, hit count = 0 

(lldb) br del 1
1 breakpoints deleted; 0 breakpoint locations disabled.
(lldb) br li
Current breakpoints:

// ⬇️ 看到断点 1 已经被删除
 
2: file = '/Users/hmc/Documents/iOSSample/TEST_Fishhook/TEST_Fishhook/main.m', line = 30, exact_match = 0, locations = 1, resolved = 1, hit count = 0

  2.1: where = TEST_Fishhook`main + 56 at main.m:30:29, address = 0x0000000102a2df48, resolved, hit count = 0 

(lldb) 
```

#### 创建断点

&emsp;在 LLDB 调试器中设置断点，可以使用 `breakpoint set <cmd-options>` 命令，如下示例在 main.m 文件的 35 行设置一个断点（`br` 是 `breakpoint` 的缩写形式）：

```c++
(lldb) breakpoint set -f main.m -l 35
Breakpoint 3: where = TEST_Fishhook`main + 146 at main.m:35:29, address = 0x0000000102a2dfa2

(lldb) br set -f main.m -l 35
Breakpoint 5: where = TEST_Fishhook`main + 146 at main.m:35:29, address = 0x0000000102a2dfa2
```

&emsp;我们也可以使用 `b` 命令：

```c++
(lldb) help b
     Set a breakpoint using one of several shorthand formats.  Expects 'raw' input (see 'help raw-input'.)

Syntax: 
_regexp-break <filename>:<linenum>
              main.c:12             // Break at line 12 of main.c

_regexp-break <linenum>
              12                    // Break at line 12 of current file

_regexp-break 0x<address>
              0x1234000             // Break at address 0x1234000

_regexp-break <name>
              main                  // Break in 'main' after the prologue

_regexp-break &<name>
              &main                 // Break at first instruction in 'main'

_regexp-break <module>`<name>
              libc.so`malloc        // Break in 'malloc' from 'libc.so'

_regexp-break /<source-regex>/
              /break here/          // Break on source lines in current file
                                    // containing text 'break here'.


'b' is an abbreviation for '_regexp-break'
```

&emsp;`b main.c:35`  同样也是在 main.m 文件的 35 行设置一个断点：

```c++
(lldb) b main.m:35
Breakpoint 6: where = TEST_Fishhook`main + 146 at main.m:35:29, address = 0x0000000102a2dfa2
```

&emsp;`b isEven` 在一个符号（C 语言函数）上创建断点，完全不用指定在哪一行：

```c++
(lldb) b isEven
Breakpoint 7: where = TEST_Fishhook`isEven + 11 at main.m:12:9, address = 0x0000000102a2e08b
```

&emsp;OC 方法也可以：

```c++
(lldb) breakpoint set -F "-[NSArray objectAtIndex:]"
Breakpoint 8: where = CoreFoundation`-[NSArray objectAtIndex:], address = 0x00007fff204a2f77

(lldb) b -[NSArray objectAtIndex:]
Breakpoint 9: where = CoreFoundation`-[NSArray objectAtIndex:], address = 0x00007fff204a2f77

(lldb) breakpoint set -F "+[NSSet setWithObject:]"
Breakpoint 10: where = CoreFoundation`+[NSSet setWithObject:], address = 0x00007fff20434b0f

(lldb) b +[NSSet setWithObject:]
Breakpoint 11: where = CoreFoundation`+[NSSet setWithObject:], address = 0x00007fff20434b0f
```

&emsp;下面我们看一下在 Xcode 的图形界面上如何添加断点，我们可以点击 Breakpoint navigator 底部的 + 按钮，然后选择 Symbolic Breakpoint...，然后我们可以给此断点设置需要命中的符号、所属模块、命中条件、忽略次数（如第几次调用该符号时才命中此断点）、断点命中时的活动、以及断点命中后执行完活动后是否继续执行进程：

![截屏2021-09-12 下午8.54.24.png](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/ed3083f34f3e40ad9ad05fda440ab015~tplv-k3u1fbpfcp-watermark.image?)

![截屏2021-09-12 下午9.02.15.png](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/7db17f991c2941adb51475cd08f1dd0c~tplv-k3u1fbpfcp-watermark.image?)

&emsp;例如我们在上面的弹框中的 Symbol 输入框中输入 `-[NSArray objectAtIndex:]`，那么进程每次调用此函数时都会命中这个断点，包括我们自己的调用以及系统的调用。

&emsp;其中命中条件和忽略次数，如下代码，我们在 `if (i % 2 == 0) {` 行添加一个断点，那么命中条件我们可以添加 `i == 99`，当 i 等于 99 的时候才命中此断点，忽略次数初始值是 0，表示不忽略，如输入 5，可表示前 5 次调用都忽略，后续调用才会命中此断点。

```c++
static BOOL isEven(int i) {
    if (i % 2 == 0) {
        NSLog(@"✳️✳️✳️ %d is even!", i);
        return YES;
    }
    
    NSLog(@"✳️✳️✳️ %d is odd!", i);
    return NO;
}
```

&emsp;断点命中时的活动，例如每次命中时打印 i 的值（执行 `p i` 命令）:

![截屏2021-09-12 下午9.15.28.png](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/4b60cfdd0edc4eb2acaaabfbe429851d~tplv-k3u1fbpfcp-watermark.image?)

&emsp;除了断点命中时执行 LLDB 命令，我们还可以添加别的一些活动：执行 shell 命令、在控制台打印 log 信息或者直接用语音读出 log 信息、执行 Apple 脚本、捕捉 GPU 栈帧、以及播放声音（播放声音这个可太搞笑了）。

![截屏2021-09-12 下午9.16.26.png](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/47f2e3ae44b044bdaa6d122bced1dcaf~tplv-k3u1fbpfcp-watermark.image?)

&emsp;在 LLDB 中使用命令也可以这样做，如下：

```c++
(lldb) breakpoint set -F isEven
Breakpoint 2: where = TEST_Fishhook`isEven + 11 at main.m:12:9, address = 0x0000000107ee508b
(lldb) breakpoint modify -c 'i == 99' 2
(lldb) breakpoint command add 2
Enter your debugger command(s).  Type 'DONE' to end.
> p i
> DONE
(lldb) br li 2
2: name = 'isEven', locations = 1, resolved = 1, hit count = 0
    Breakpoint commands:
      p i

Condition: i == 99

  2.1: where = TEST_Fishhook`isEven + 11 at main.m:12:9, address = 0x0000000107ee508b, resolved, hit count = 0 

(lldb) 
```

&emsp;看编辑断点弹出窗口的底部，会看到一个选项： Automatically continue after evaluation actions 。它仅仅是一个选择框，但是却很强大。选中它，调试器会运行断点中所有的命令，然后继续运行。看起来就像没有执行任何断点一样 (除非断点太多，运行需要一段时间，拖慢了程序执行)。

&emsp;这个选项框的效果和让最后断点的最后一个行为是 `process continue` 一样。选框只是让这个操作变得更简单。使用 LLDB 命令也可以达到同样的效果：

```c++
(lldb) breakpoint set -F isEven
Breakpoint 3: where = TEST_Fishhook`isEven + 11 at main.m:12:9, address = 0x0000000107ee508b
(lldb) breakpoint command add 3
Enter your debugger command(s).  Type 'DONE' to end.
> continue
> DONE
(lldb) br li 3
3: name = 'isEven', locations = 1, resolved = 1, hit count = 0
    Breakpoint commands:
      continue

  3.1: where = TEST_Fishhook`isEven + 11 at main.m:12:9, address = 0x0000000107ee508b, resolved, hit count = 0 

(lldb) 
```

&emsp;执行断点后自动继续运行，允许你完全通过断点来修改程序！你可以在某一行停止，运行一个 `expression` 命令来改变变量，然后继续运行。或者在某个函数第一行添加断点，然后运行一个 `thread return 9` 命令，直接结束此函数的调用并返回一个自己设定的值。

```c++
(lldb) help breakpoint
     Commands for operating on breakpoints (see 'help b' for shorthand.)

Syntax: breakpoint <subcommand> [<command-options>]

The following subcommands are supported:

      clear   -- Delete or disable breakpoints matching the specified source file and line.
                 删除或禁用与指定源文件和行匹配的断点。
      
      command -- Commands for adding, removing and listing LLDB commands executed when a breakpoint is hit.
                 当断点被命中时 执行添加、删除和列出 LLDB 命令的 命令。（即添加当断点被命中时，我们想要执行的命令）
      
      delete  -- Delete the specified breakpoint(s).  If no breakpoints are specified, delete them all.
                 删除指定的断点。 如果没有指定断点，则删除全部断点。
      
      disable -- Disable the specified breakpoint(s) without deleting them.  If none are specified, disable all breakpoints.
                 在不删除它们的情况下禁用指定的断点。 如果没有指定，则禁用所有断点。
      
      enable  -- Enable the specified disabled breakpoint(s). If no breakpoints are specified, enable all of them.
                 启用指定的禁用断点。如果没有指定断点，则启用所有断点。
      
      list    -- List some or all breakpoints at configurable levels of detail.
                 将部分或全部断点列在可配置的细节级别。
      
      modify  -- Modify the options on a breakpoint or set of breakpoints in the executable.  If no breakpoint is specified, acts on the last created breakpoint.  With the exception of -e, -d and -i,
                 passing an empty argument clears the modification.
                 修改可执行中的断点或一组断点上的选项。 如果没有指定断点，则在最后创建的断点上起作用。 除了 -e， -d 和 -i 通过一个空的参数清除修改。
                 
      name    -- Commands to manage name tags for breakpoints.
                 管理断点 名称标签 的命令。
      
      read    -- Read and set the breakpoints previously saved to a file with "breakpoint write".
                 阅读并设置以前保存到 "breakpoint write" 的文件中的断点。
      
      set     -- Sets a breakpoint or set of breakpoints in the executable.
                 在可执行程序中设置一个断点或一组断点。
      
      write   -- Write the breakpoints listed to a file that can be read in with "breakpoint read".  If given no arguments, writes all breakpoints.
                 将列出的断点写入可用 "breakpoint read" 读取的文件中。 如果没有参数，则编写所有断点。

For more help on any particular subcommand, type 'help <command> <subcommand>'.
```

 &emsp;至此断点相关的命令就看到这里，下面我们看一些 LLDB 在 iOS App 的视图层级的一些用法。
 
 ## 在 iOS App 中进行 LLDB 调试
 
 &emap;前面我们学习了 LLDB 调试条上的 暂停/继续 执行程序的按钮，其中我们的主要关注点放在了继续按钮上，现在我们试试运行 iOS App 后点击暂停按钮试试。
 
 ### 打印当前 App 视图层级  
 
 &emsp;点击暂停按钮后可看到我们的 iOS App 定位到了主线程的 `mach_msg_trap` 处，并且我们的 Xcode 的控制台进入了 LLDB 调试模式。点击暂停按钮后会暂停 iOS App 的运行，就如同执行了 `process interrupt` 命令，因为 LLDB 总是在 Xcode 背后运行的。虽然此时也进入了 LLDB 调试模式，但是我们的 iOS App 并没有暂停在我们自己编写的特定的函数或者特定的代码处，所以我们能做的事情并不多，但是我们可以试着去访问 iOS App 的全局变量。如下，我们可以通过 `keyWindow` 来访问我们 iOS App 的视图层级：

```c++
(lldb) po [[[UIApplication sharedApplication] keyWindow] recursiveDescription]
<UIWindow: 0x7fb4555062a0; frame = (0 0; 428 926); gestureRecognizers = <NSArray: 0x600000164ab0>; layer = <UIWindowLayer: 0x600000f665c0>>
   | <UITransitionView: 0x7fb457005d60; frame = (0 0; 428 926); autoresize = W+H; layer = <CALayer: 0x600000f41140>>
   |    | <UIDropShadowView: 0x7fb457006680; frame = (0 0; 428 926); autoresize = W+H; layer = <CALayer: 0x600000f41da0>>
   |    |    | <UILayoutContainerView: 0x7fb45720a9e0; frame = (0 0; 428 926); clipsToBounds = YES; autoresize = W+H; gestureRecognizers = <NSArray: 0x60000011fea0>; layer = <CALayer: 0x600000f08740>>
   |    |    |    | <UINavigationTransitionView: 0x7fb45720b7e0; frame = (0 0; 428 926); clipsToBounds = YES; autoresize = W+H; layer = <CALayer: 0x600000f08880>>
   |    |    |    |    | <UIViewControllerWrapperView: 0x7fb457509d60; frame = (0 0; 428 926); autoresize = W+H; layer = <CALayer: 0x600000f51d40>>
   |    |    |    |    |    | <UIView: 0x7fb4575092e0; frame = (0 0; 428 926); autoresize = W+H; layer = <CALayer: 0x600000f51e00>>
   |    |    |    |    |    |    | <UILabel: 0x7fb457509450; frame = (182 453; 64.3333 20.3333); text = 'CENTER'; opaque = NO; autoresize = RM+BM; userInteractionEnabled = NO; layer = <_UILabelLayer: 0x600002c506e0>>
   |    |    |    | <UINavigationBar: 0x7fb45741dce0; frame = (0 47; 428 44); opaque = NO; autoresize = W; layer = <CALayer: 0x600000f31480>>
   |    |    |    |    | <_UIBarBackground: 0x7fb45740af90; frame = (0 -47; 428 91); userInteractionEnabled = NO; layer = <CALayer: 0x600000f315e0>>
   |    |    |    |    |    | <UIVisualEffectView: 0x7fb45700f790; frame = (0 0; 428 91); layer = <CALayer: 0x600000f12060>> effect=none
   |    |    |    |    |    |    | <_UIVisualEffectBackdropView: 0x7fb45700ff30; frame = (0 0; 428 91); autoresize = W+H; userInteractionEnabled = NO; layer = <UICABackdropLayer: 0x600000f12300>>
   |    |    |    |    |    | <_UIBarBackgroundShadowView: 0x7fb45700fb60; frame = (0 91; 428 0.333333); layer = <CALayer: 0x600000f12160>> clientRequestedContentView effect=none
   |    |    |    |    |    |    | <_UIBarBackgroundShadowContentImageView: 0x7fb4557180d0; frame = (0 0; 428 0.333333); autoresize = W+H; userInteractionEnabled = NO; layer = <CALayer: 0x600000f3bf40>>
   |    |    |    |    | <_UINavigationBarContentView: 0x7fb457415480; frame = (0 0; 428 44); layer = <CALayer: 0x600000f31600>> layout=0x7fb45741ac30
   |    |    |    |    | <UIView: 0x7fb45700ce70; frame = (0 0; 0 0); userInteractionEnabled = NO; layer = <CALayer: 0x600000f10040>>
```

### 在 LLDB 中调试 App UI 

&emsp;通过上面视图层级的输出我们可以直接通过内存地址获得我们的 text 是 CENTER 的 UILabel，下面我们在 LLDB  命名空间中创建一个变量来取得此 UILabel：

```c++
(lldb) expression UILabel *$myLabel = (UILabel *)0x7fb457509450
```

&emsp;然后我们改变 myLabel 的背景颜色：

```c++
(lldb) expression [$myLabel setBackgroundColor: [UIColor blueColor]]
```

&emsp;执行完此命令，我们的 Label 的背景颜色并没有发生变化，因为改变的内容需要被发送到渲染服务中，这样我们屏幕上的显示的内容才能得到刷新，此时需要我们可以再次点击 LLDB 调试条上的 继续 按钮，继续运行我们的 iOS App，我们的 App 界面才能得到刷新，我们的 Label 的背景色才能变成蓝色。 

&emsp;渲染服务实际上是另外一个进程（被称作：backboardd），这就是说即使我们当前的 App 进程被暂停了，但是 backboardd 还是在继续运行着，这意味着我们可以运行下面的命令，而不是继续运行我们的程序：

```c++
(lldb) expression [CATransaction flush]

error: <user expression 3>:1:16: no known method '+flush'; cast the message send to the method's return type
[CATransaction flush]
~~~~~~~~~~~~~~~^~~~~~

(lldb) expression (void)[CATransaction flush] // ⬅️ 需要在函数调用前添加一个返回值类型，否则会报错
```

&emsp;执行 `expression (void)[CATransaction flush]` 后，我们的 iOS App 仍处于暂停状态，我们仍处于 LLDB 调试模式，但是不管是在模拟器上还是真机上我们都能看到我们的 Label 的背景颜色变为了蓝色，即我们的 iOS App 的 UI 得到了实时更新。
 
&emsp;下面我们再做一个更大的更新，我们在当前 VC push 出一个新的 VC。
 
```c++
// 取得当前程序的根控制器（导航控制器）
(lldb) expression id $nvc = [[[UIApplication sharedApplication] keyWindow] rootViewController]

// 创建一个新控制器
(lldb) expression id $vc = [UIViewController new]
(lldb) expression (void)[[$vc view] setBackgroundColor: [UIColor yellowColor]]
(lldb) expression (void)[$vc setTitle:@"New!"]

(lldb) expression (void)[$nvc pushViewController:$vc animated:YES]

// 渲染服务
(lldb) expression (void)[CATransaction flush]
```

&emsp;下面我们通过 LLDB 调试查找一个按钮的点击事件。

&emsp;首先取得一个 $myButton 变量，可以通过 `po [[[UIApplication sharedApplication] keyWindow] recursiveDescription]` 在 UI 层取出，或者是停在一个断点时取得的一个局部变量，接下来我们找到 $myButton 按钮的点击事件： 

```c++
(lldb) expression id $myButton = (id)0x7ff1e9412bc0

(lldb) po [$myButton allTargets]
{(
    <ViewController: 0x7ff1ec0087c0>
)}

(lldb) po [$myButton actionsForTarget:(id)0x7ff1ec0087c0 forControlEvent:0]
<__NSArrayM 0x6000022dd050>(
buttonAction:
)
```

&emsp;然后我们可以在 `-[ViewController buttonAction:]` 设置一个符号断点，当按钮被点击时就会命中此断点。
 
 &emsp;观察实例变量的变化，假设我们的一个 UIView，不知道为什么它的 `_layer` 实例变量被重写了 (糟糕)。因为有可能并不涉及到方法，我们不能使用符号断点。相反的，我们想监视什么时候这个地址被写入。首先，我们需要找到 `_layer` 这个变量在对象上的相对位置：
 
```c++
 (lldb) p (ptrdiff_t)ivar_getOffset((CALayer *)class_getInstanceVariable([UIView class], "_layer"))
 (ptrdiff_t) $3 = 40
``` 

&emsp;现在我们知道 `$myView + 40` 是 `_layer` 实例变量的内存地址：

```c++
 (lldb) expression id $myView = (id)0x7ff1e9412340
 (lldb) watchpoint set expression -- (int *)$myView + 40
 Watchpoint created: Watchpoint 1: addr = 0x7ff1e94123e0 size = 8 state = enabled type = w
     new value: 0x0000000000000000
 (lldb) watchpoint list
 Number of supported hardware watchpoints: 4
 Current watchpoints:
 Watchpoint 1: addr = 0x7ff1e94123e0 size = 8 state = enabled type = w
     new value: 0x0000000000000000
``` 

&emsp;非重写方法的符号断点：

> &emsp;假设你想知道 -[ViewController viewDidAppear:] 什么时候被调用。如果这个方法并没有在 MyViewController 中实现，而是在其父类中实现的，该怎么办呢？试着设置一个断点，会出现以下结果：
  ```c++
  (lldb) b -[ViewController viewDidAppear:]
  Breakpoint 2: no locations (pending).
  WARNING:  Unable to resolve breakpoint to any actual locations.
  ```
  因为 LLDB 会查找一个符号，但是实际在这个类上却找不到，所以断点也永远不会触发。你需要做的是为断点设置一个条件 [self isKindOfClass:[MyViewController class]]，然后把断点放在 UIViewController 上。正常情况下这样设置一个条件可以正常工作。但是这里不会，因为我们没有父类的实现。viewDidAppear: 是苹果实现的方法，因此没有它的符号；在方法内没有 self 。如果想在符号断点上使用 self，你必须知道它在哪里 (它可能在寄存器上，也可能在栈上；在 x86 上，你可以在 $esp+4 找到它)。但是这是很痛苦的，因为现在你必须至少知道四种体系结构 (x86，x86-64，armv7，armv64)。想象你需要花多少时间去学习命令集以及它们每一个的调用约定，然后正确的写一个在你的超类上设置断点并且条件正确的命令。幸运的是，这个在 Chisel 被解决了。这被成为 bmessage：
  ```c++
  (lldb) bmessage -[ViewController viewDidAppear:]
  Setting a breakpoint at -[UIViewController viewDidAppear:] with condition (void*)object_getClass((id)$rdi) == 0x0000000105154570
  Breakpoint 1: where = UIKitCore`-[UIViewController viewDidAppear:], address = 0x00007fff23f6968e
  ```

## chisel 概述

&emsp;chisel 可以使用 `brew install chisel` 安装，然后根据提示把一行类似 `command script import /usr/local/opt/chisel/libexec/fbchisellldb.py` 的命令添加到 `~/.lldbinit` 文件中，如果 `.lldbinit` 文件不存在的话，我们可以自己创建一个（路径类似：/Users/hmc/.lldbinit），`.lldbinit` 中的内容会在 Xcode 启动时执行，上面一行便是在 Xcode 启动时加载 chisel。 

![截屏2021-09-02 下午10.35.11.png](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/b70fa058010847499da535b95c4ba883~tplv-k3u1fbpfcp-watermark.image)

&emsp;`command script import /usr/local/opt/chisel/libexec/fbchisellldb.py` 中的 `command` 正是 LLDB 中的一个命令，它是用来管理 LLDB 自定义命令的命令。

```c++
(lldb) command
     Commands for managing custom LLDB commands.

Syntax: command <subcommand> [<subcommand-options>]

The following subcommands are supported:

      alias   -- Define a custom command in terms of an existing command. 
                 Expects 'raw' input (see 'help raw-input'.)
                 根据现有命令定义自定义命令。（为一个命令起个别名）
                 
      delete  -- Delete one or more custom commands defined by 'command regex'.
                 删除由 'command regex' 定义的一个或多个自定义命令。
      
      history -- Dump the history of commands in this session.
                 Commands in the history list can be run again using
                 "!<INDEX>".   "!-<OFFSET>" will re-run the command that is
                 <OFFSET> commands from the end of the list (counting the
                 current command).
                 
      regex   -- Define a custom command in terms of existing commands by matching regular expressions.
                 通过匹配常规表达式，根据现有命令定义自定义命令。
                 
      script  -- Commands for managing custom commands implemented by interpreter scripts.
                 由 interpreter scripts 实施的自定义命令管理命令。
      
      source  -- Read and execute LLDB commands from the file <filename>.
                 从文件中读取和执行 LLDB 命令<filename>。
      
      unalias -- Delete one or more custom commands defined by 'command alias'.
                 删除由 'command alias' 定义的一个或多个自定义命令。

For more help on any particular subcommand, type 'help <command> <subcommand>'.
```

&emsp;`command script import` 用来在 LLDB 中导入脚本模块。

```c++
(lldb) help command script import
     Import a scripting module in LLDB.

Syntax: command script import <cmd-options> <filename> [<filename> [...]]

Command Options Usage:
  command script import [-r] <filename> [<filename> [...]]

       -r ( --allow-reload )
            Allow the script to be loaded even if it was already loaded before.
            This argument exists for backwards compatibility, but reloading is always allowed, whether you specify it or not.
            即使脚本以前已经加载，也允许加载脚本。此参数存在向后兼容性，但无论你是否指定，重加载始终允许。
```

&emsp;`command script import /usr/local/opt/chisel/libexec/fbchisellldb.py` 此行命令的作用便是把 `/usr/local/opt/chisel/libexec/fbchisellldb.py` 中的脚本导入到 LLDB 中。那么 chisel 提供了多少命令呢，如下：

```c++
Current user-defined commands:
  alamborder    -- Put a border around views with an ambiguous layout
  alamunborder  -- Removes the border around views with an ambiguous layout
  bdisable      -- Disable a set of breakpoints for a regular expression
  benable       -- Enable a set of breakpoints for a regular expression
  binside       -- Set a breakpoint for a relative address within the framework/library that's currently running. This does the work of finding the offset for the framework/library and sliding your
                   address accordingly.
  bmessage      -- Set a breakpoint for a selector on a class, even if the class itself doesn't override that selector. It walks the hierarchy until it finds a class that does implement the selector
                   and sets a conditional breakpoint there.
  border        -- Draws a border around <viewOrLayer>. Color and width can be optionally provided. Additionally depth can be provided in order to recursively border subviews.
  caflush       -- Force Core Animation to flush. This will 'repaint' the UI but also may mess with ongoing animations.
  copy          -- Copy data to your Mac.
  dcomponents   -- Set debugging options for components.
  dismiss       -- Dismiss a presented view controller.
  fa11y         -- Find the views whose accessibility labels match labelRegex and puts the address of the first result on the clipboard.
  findinstances -- Find instances of specified ObjC classes.
  flicker       -- Quickly show and hide a view to quickly help visualize where it is.
  fv            -- Find the views whose class names match classNameRegex and puts the address of first on the clipboard.
  fvc           -- Find the view controllers whose class names match classNameRegex and puts the address of first on the clipboard.
  heapfrom      -- Show all nested heap pointers contained within a given variable.
  hide          -- Hide a view or layer.
  mask          -- Add a transparent rectangle to the window to reveal a possibly obscured or hidden view or layer's bounds
  mwarning      -- simulate a memory warning
  pa11y         -- Print accessibility labels of all views in hierarchy of <aView>
  pa11yi        -- Print accessibility identifiers of all views in hierarchy of <aView>
  pactions      -- Print the actions and targets of a control.
  paltrace      -- Print the Auto Layout trace for the given view. Defaults to the key window.
  panim         -- Prints if the code is currently execution with a UIView animation block.
  pbcopy        -- Print object and copy output to clipboard
  pblock        -- Print the block`s implementation address and signature
  pbundlepath   -- Print application's bundle directory path.
  pcells        -- Print the visible cells of the highest table view in the hierarchy.
  pclass        -- Print the inheritance starting from an instance of any class.
  pcomponents   -- Print a recursive description of components found starting from <aView>.
  pcurl         -- Print the NSURLRequest (HTTP) as curl command.
  pdata         -- Print the contents of NSData object as string.
  pdocspath     -- Print application's 'Documents' directory path.
  pinternals    -- Show the internals of an object by dereferencing it as a pointer.
  pinvocation   -- Print the stack frame, receiver, and arguments of the current invocation. It will fail to print all arguments if any arguments are variadic (varargs).
  pivar         -- Print the value of an object's named instance variable.
  pjson         -- Print JSON representation of NSDictionary or NSArray object
  pkp           -- Print out the value of the key path expression using -valueForKeyPath:
  pmethods      -- Print the class and instance methods of a class.
  poobjc        -- Print the expression result, with the expression run in an ObjC++ context. (Shortcut for "expression -O -l ObjC++ -- " )
  pproperties   -- Print the properties of an instance or Class
  present       -- Present a view controller.
  presponder    -- Print the responder chain starting from a specific responder.
  psjson        -- Print JSON representation of Swift Dictionary or Swift Array object
  ptv           -- Print the highest table view in the hierarchy.
  pvc           -- Print the recursion description of <aViewController>.
  pviews        -- Print the recursion description of <aView>.
  rcomponents   -- Synchronously reflow and update all components.
  sequence      -- Run commands in sequence, stopping on any error.
  setinput      -- Input text into text field or text view that is first responder.
  settext       -- Set text on text on a view by accessibility id.
  show          -- Show a view or layer.
  slowanim      -- Slows down animations. Works on the iOS Simulator and a device.
  taplog        -- Log tapped view to the console.
  uikit         -- Imports the UIKit module to get access to the types while in lldb.
  unborder      -- Removes border around <viewOrLayer>.
  unmask        -- Remove mask from a view or layer
  unslowanim    -- Turn off slow animations.
  visualize     -- Open a UIImage, CGImageRef, UIView, CALayer, or CVPixelBuffer in Preview.app on your Mac.
  vs            -- Interactively search for a view by walking the hierarchy.
  wivar         -- Set a watchpoint for an object's instance variable.
  xdebug        -- Print debug description the XCUIElement in human readable format.
  xnoid         -- Print XCUIElement objects with label but without identifier.
  xobject       -- Print XCUIElement details.
  xtree         -- Print XCUIElement subtree.
  zzz           -- Executes specified lldb command after delay.
```

&emsp;那么我们下一节再详细列举 chisel 提供的命令的作用吧！

 
## 内容规划

1. 介绍 chisel 安装、使用细节、官方提供的命令列表[wiki](https://github.com/facebook/chisel/wiki)、Custom Commands如何进行自定义命令。[chisel](https://github.com/facebook/chisel/blob/master/README.md)

![截屏2021-09-02 下午10.35.11.png](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/b70fa058010847499da535b95c4ba883~tplv-k3u1fbpfcp-watermark.image)
&emsp;Add the following line to ~/.lldbinit to load chisel when Xcode launches:
`command script import /usr/local/opt/chisel/libexec/fbchisellldb.py` 

2. LLDB 给的命令列表使用示例。

3. chisel 命令使用示例。（chisel 里面的命令都是基于 LLDB 支持的 python 脚本来做的，保存在 /opt/homebrew/Cellar/chisel/2.0.1/libexec 路径下。） 



## 参考链接
**参考链接:🔗**
+ [Debugging with Xcode](https://developer.apple.com/library/archive/documentation/DeveloperTools/Conceptual/debugging_with_xcode/chapters/about_debugging_w_xcode.html#//apple_ref/doc/uid/TP40015022-CH10-SW1)
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
+ [LLDB 不是只有 po](https://medium.com/@yenchiayou/lldb-不是只有-po-563597c78faf)
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

+ [Step Over Thread和Step Into Thread命令有什么用处？](http://cn.voidcc.com/question/p-tjpujtpj-bez.html)
+ [对 Xcode 菜单选项的详细探索](https://blog.csdn.net/zhanglizhi111/article/details/52326744)
