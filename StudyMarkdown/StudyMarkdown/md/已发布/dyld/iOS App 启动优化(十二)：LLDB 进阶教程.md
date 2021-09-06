# iOS App 启动优化(十二)：LLDB Debugger (LLDB) 进阶

## LLDB Debugger (LLDB) 简述

> &emsp;什么是 REPL？R(read)、E(evaluate)、P(print)、L(loop)。“读取-求值-输出” 循环（英语：Read-Eval-Print Loop，简称 REPL），也被称作交互式顶层构件（英语：interactive toplevel），是一个简单的，交互式的编程环境。这个词常常用于指代一个 [Lisp](https://zh.wikipedia.org/wiki/LISP) 的交互式开发环境，也能指代命令行的模式。REPL 使得探索性的编程和调试更加便捷，因为 “读取-求值-输出” 循环通常会比经典的 “编辑-编译-运行-调试” 模式要更快。（有时候翻译成交互式解释器。就是你往 REPL 里输入一行代码，它立即给你执行结果。而不用像 C++, Java 那样通常需要编译才能看到运行结果，像 Python Ruby Scala 都是自带 REPL 的语言。）[读取﹣求值﹣输出循环 维基百科](https://zh.wikipedia.org/wiki/读取﹣求值﹣输出循环) [什么是REPL？](https://www.zhihu.com/question/53865469)

&emsp;[LLDB Debugger (LLDB)](https://lldb.llvm.org/resources/contributing.html) 是一个有着 REPL 特性并支持 C++、Python 插件的 **开源调试器**，LLDB 已被内置在 Xcode 中，Xcode 主窗口底部的控制台便是我们与 LLDB 交互的区域。LLDB 允许你在程序运行的特定时刻（如执行到某行代码、某个函数、某个变量被修改、target stop-hook 命中时）暂停它（`breakpoint/br/b/watchpoint/watch/wa`），你可以查看变量的值（`p/po/wa/frame/fr/target/ta`）、执行自定的指令（`expression/expr`），并且按照你所认为合适的步骤来操作程序（函数）的执行过程，对程序进行流程控制。（[How debuggers work: Part 1 - Basics](https://eli.thegreenplace.net/2011/01/23/how-debuggers-work-part-1.html) 是一个关于调试器如何工作的总体的解释。）

&emsp;[GDB to LLDB command map](https://lldb.llvm.org/use/map.html) 中的一组表格向我们非常好的介绍了 LLDB 调试器提供的几大块命令的总览（常规用法，估计大家都已经掌握了）。除此之外我们可以安装 [Chisel](https://github.com/facebook/chisel/wiki) 来体验 LLDB 更 “高级” 的用法，Chisel 是 facebook 开源的一组  LLDB 命令合集，用于协助我们 **调试 iOS 应用程序**。Chisel 里面的命令正是基于 LLDB 支持 Python 脚本解释器来运行的，Chisel 每条命令对应的 Python 文件保存在 `/usr/local/Cellar/chisel/2.0.1/libexec` 路径下，熟悉 Python 的小伙伴可以试着读一下这些文件的内容（具体路径可能各人机器不同会有所区别，例如 Intel 的 mac 在 `/usr/local/Cellar/chisel/2.0.1/libexec` 路径下，m1 的 mac 在 `/opt/homebrew/Cellar/chisel/2.0.1/libexec` 路径下）。

&emsp;上面我们说到 LLDB 已被内置在 Xcode 中，它是作为一个共享库放在 Xcode 的 SharedFrameworks 下面的，其完整路径是：`/Applications/Xcode.app/Contents/SharedFrameworks/LLDB.framework`。

> &emsp;LLDB 的调试接口本质上是一个 C++ 共享库，在 Mac 系统上，它被打包为 `LLDB.framework`（正常情况下，它存在于 `/Applications/Xcode.app/Contents/SharedFrameworks/LLDB.framework` 路径下），在类 unix 系统上，它是 `lldb.so`。 这些调试接口可以在 lldb 的脚本解释器内直接使用，或者可以被引入 lldb.py 模块 的 Python 脚本 使用。LLDB 本身支持用户自定义命令，比如通过脚本可以自定义一个 `pviews` 命令，该命令可以打印 APP 所有的视图（该命令已经在 Chisel 中实现）。[lldb-入坑指北（1）-给xcode添加批量启用禁用断点功能](https://cloud.tencent.com/developer/article/1729078)

&emsp;下面我们从 LLDB 的基础命令开始，一步一步深入学习 LLDB 调试器。

## LLDB 基础命令

&emsp;日常我们更多的可能是在 Xcode 中使用 LLDB，今天我们通过 **终端** 与 Xcode 结合使用来对 LLDB 进行学习。熟悉 LLDB 的小伙伴应该都用过 `help`，如果有哪个命令不太熟悉的话，我们直接一个 `help <command>` 回车，LLDB 便会打印出该命令的详细信息，甚至 `help help` 能教我们怎么使用 `help` 命令。下面我们启动终端，然后输入 `LLDB` 并回车，此时便会进入 `LLDB` 环境，然后我们再输入 `help` 命令并回车，便会列出一大组当前机器内可用的 LLDB 命令，看到我的机器把所有的 LLDB 命令分了三组：

+ Debugger commands
+ Current command abbreviations (type 'help command alias' for more info)（Debugger commands 中的一些命令的别名或缩写）
+ Current user-defined commands（用户自定义命令，由于我的机器已经安装了 Chisel，所以这里也直接列出来了 Chisel 提供的命令）

&emsp;下面我们就从最简单的 `help` 命令开始学习。

### help

&emsp;直接输入 `help` 命令，可显示所有 LLDB 命令的列表，输入 `help [<cmd-name>]` 便可列出 `<cmd-name>` 这个特定命令的详细信息。例如输入 `help help` 命令并回车，可看到如下描述：

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
            (相对 help 命令打印所有的命令列表，help -a 会隐藏 Current command abbreviations 分组的命令)

       -h ( --show-hidden-commands )(-h 是 --show-hidden-commands 选项的缩写)
            Include commands prefixed with an underscore.
            (同 help -h 和 help 相同，会把所有命令列出来)

       -u ( --hide-user-commands )(--hide-user-commands 是 -u 选项的缩写)
            Hide user-defined commands from the list.
            (相对 help 命令打印所有的命令列表，help -u 会隐藏 Current user-defined commands 分组的命令)
     
     This command takes options and free-form arguments.  If your arguments resemble option specifiers (i.e., they start with a - or --), you
     must use ' -- ' between the end of the command options and the beginning of the arguments.
     // 这里有很重要的一句：help 命令采用 options 和 free-form（自由格式的）参数。如果你的的参数类似于 option 说明符（例如，它们以 - 和 -- 开头），你必须在命令 options 的结尾和参数的开头之间使用 `--` 分隔。
     // 即当一个命令即有选项也有参数时，选项要放在前面，然后在它的结尾要插入 `--` 与后面的参数分隔开。
```

### print/po/p

&emsp;在 LLDB 调试过程中，打印命令大概是我们用的最多的命令了，`print/prin/pri/p/po` 都能进行打印操作（打印表达式的返回值，变量名也是一个表达式），LLDB 实际上会作前缀匹配，所以我们使用 `print/prin/pri/p` 是完全一样的，但是我们不能使用 `pr`，因为 LLDB 不能消除 `print` 和 `process` 的歧义，不过幸运的是我们可以直接使用 `p`，大概是打印命令用的比较多吧，LLDB 把 `p` 这个最简单的缩写/别名归给了打印命令使用。（如下我们使用 `help pr` 命令，会提示我们使用了不明确的命令名称，并列出了所有以 `pr` 开头的命令，来提示我们具体想要哪个命令。）

```c++
(lldb) help pr
Help requested with ambiguous command name, possible completions:
    process
    print
    present
    presponder
```

&emsp;下面我们区分一下 `print/prin/pri/p` 和 `po`，虽然它们都是打印命令，但是它们的打印格式并不相同。

&emsp;分别通过 `help print`、`help p`、`help po` 输出的帮助信息，我们可以看到 `print/p` 是作用完全一样的命令，`print/p` 都是 `expression --` 命令的缩写，它们都是根据 LLDB 的默认格式来进行打印操作，而 `po` 则是 `expression -O  --` 的缩写，如果大家阅读的认真的话，应该还记得 `--` 是用来标记命令的选项结束的，`--` 前面是命令的选项，`--` 后面是命令的参数。`expression -O  --` 中的 `-O` 选项是 `--object-description` 的缩写：`-O ( --object-description ) Display using a language-specific description API, if possible.` 针对 Objective-C 的话，便是调用 `description` 实例函数或者类函数，即使用 `po` 命令进行打印时，它是根据当前语言环境，调用 `description` API 返回的结果进行打印操作。（在 OC 中我们可以通过重写 `+/-description` 函数便可得到自定义的打印结果）   

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

&emsp;根据帮助信息我们可以看到 `p/po` 仅能在当前线程使用（`expression` 仅用于当前线程），也就是说我们仅能打印当前线程的表达式的返回值，如果我们打印其它线程的表达式的话会得到一个未定义的错误：`error: <user expression 0>:1:1: use of undeclared identifier 'xxx'`。（例如 xxx 代指我们要打印的变量名）

&emsp;如果我们使用过 `p` 命令的话，一定还记得每次 `p` 命令打印的结果中都会直接输出一个以 `$` 开头的变量，此变量的值便是我们要打印的表达式的返回值，即我们可以把此 `$` 开头的变量理解为我们使用 `p` 命令打印的结果值的变量，这些以 `$` 开头的变量都是存在于 LLDB 的命名空间中的，我们可以直接使用它（如果有看过之前文章的话，在类结构一章中，我们正是根据 `p` 命令输出的 `$` 开头的变量，一层一层根据 `$` 开头的变量的值进行强制转换来查看类的结构的）。

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

&emsp;关于单行和多行表达式，当我们要输入多行表达式时，我可以首先输入 `expression/expr/e` 命令，然后回车，即可进入 LLDB 多行表达式编辑模式，然后我们便开始输入自己想要的表达式，然后当我们要结束表达式时，我们在一个空行数输入回车，即结束多行表达式的编辑并执行。

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

### process  continue/continue/c

&emsp;


















```c++
(lldb) apropos continue
The following commands may relate to 'continue':
  process continue -- Continue execution of all threads in the current process.
  thread continue  -- Continue execution of the current target process.  One or more threads may be specified, by default all
                      threads continue.
  thread until     -- Continue until a line number or address is reached by the current or specified thread.  Stops when returning
                      from the current function as a safety measure.  The target line number(s) are given as arguments, and if
                      more than one is provided, stepping will stop when the first one is hit.
  c                -- Continue execution of all threads in the current process.
  continue         -- Continue execution of all threads in the current process.

The following settings variables may relate to 'continue': 


  target.process.thread.step-out-avoid-nodebug -- If true, when step-in/step-out/step-over leave the current frame, they will
                                                  continue to step out till they come to a function with debug information.
                                                  Passing a frame argument to step-out will override this option.
```

+ `n         -- Source level single step, stepping over calls.  Defaults to current thread unless specified.`
+ `next      -- Source level single step, stepping over calls.  Defaults to current thread unless specified.`

+ `ni        -- Instruction level single step, stepping over calls.  Defaults to current thread unless specified.`
+ `nexti     -- Instruction level single step, stepping over calls.  Defaults to current thread unless specified.`

+ `s         -- Source level single step, stepping into calls.  Defaults to current thread unless specified.`
+ `step      -- Source level single step, stepping into calls.  Defaults to current thread unless specified.`

+ `si        -- Instruction level single step, stepping into calls.  Defaults to current thread unless specified.`
+ `stepi     -- Instruction level single step, stepping into calls.  Defaults to current thread unless specified.`







+ `Activate breakpoints`
+ `Deactivate breakpoints`
+ `Pause program execution`
+ `Continue Program execution`
+ `Step over Step over instruction(hold Control) Step over thread(hold Control-Shift)`
+ `Step into Step into instruction(hold Control) Step into thread(hold Control-Shift)`
+ `Step out`































&emsp;


&emsp;

&emsp;


| 命令 | 用途 | 示例 |
| --- | --- | --- |
| apropos | List debugger commands related to a word or subject.（列出与 search-word 相关的调试命令） | Syntax: apropos <search-word> |


```c++
help
Debugger commands:
  apropos           -- List debugger commands related to a word or subject.（列出与单词或主题相关的调试器命令）
  breakpoint        -- Commands for operating on breakpoints (see 'help b' for
                       shorthand.)（用于操作断点的命令）
  command           -- Commands for managing custom LLDB commands.（用于管理自定义 LLDB commands 的命令。）
  disassemble       -- Disassemble specified instructions in the current
                       target.  Defaults to the current function for the
                       current thread and stack frame.（当前反汇编指定指令目标。默认为当前函数 当前线程和堆栈帧。）
  expression        -- Evaluate an expression on the current thread.  Displays
                       any returned value with LLDB's default formatting.
  frame             -- Commands for selecting and examing the current thread's
                       stack frames.
  gdb-remote        -- Connect to a process via remote GDB server.  If no host
                       is specifed, localhost is assumed.
  gui               -- Switch into the curses based GUI mode.
  help              -- Show a list of all debugger commands, or give details
                       about a specific command.
  kdp-remote        -- Connect to a process via remote KDP server.  If no UDP
                       port is specified, port 41139 is assumed.
  language          -- Commands specific to a source language.
  log               -- Commands controlling LLDB internal logging.
  memory            -- Commands for operating on memory in the current target
                       process.
  platform          -- Commands to manage and create platforms.
  plugin            -- Commands for managing LLDB plugins.
  process           -- Commands for interacting with processes on the current
                       platform.
  quit              -- Quit the LLDB debugger.
  register          -- Commands to access registers for the current thread and
                       stack frame.
  reproducer        -- Commands for manipulating reproducers. Reproducers make
                       it possible to capture full debug sessions with all its
                       dependencies. The resulting reproducer is used to replay
                       the debug session while debugging the debugger.
                       Because reproducers need the whole the debug session
                       from beginning to end, you need to launch the debugger
                       in capture or replay mode, commonly though the command
                       line driver.
                       Reproducers are unrelated record-replay debugging, as
                       you cannot interact with the debugger during replay.
  script            -- Invoke the script interpreter with provided code and
                       display any results.  Start the interactive interpreter
                       if no code is supplied.
  settings          -- Commands for managing LLDB settings.
  source            -- Commands for examining source code described by debug
                       information for the current target process.
  statistics        -- Print statistics about a debugging session
  target            -- Commands for operating on debugger targets.
  thread            -- Commands for operating on one or more threads in the
                       current process.
  type              -- Commands for operating on the type system.
  version           -- Show the LLDB debugger version.
  watchpoint        -- Commands for operating on watchpoints.
Current command abbreviations (type 'help command alias' for more info):
  add-dsym  -- Add a debug symbol file to one of the target's current modules
               by specifying a path to a debug symbols file, or using the
               options to specify a module to download symbols for.
  attach    -- Attach to process by ID or name.
  b         -- Set a breakpoint using one of several shorthand formats.
  bt        -- Show the current thread's call stack.  Any numeric argument
               displays at most that many frames.  The argument 'all' displays
               all threads.
  c         -- Continue execution of all threads in the current process.
  call      -- Evaluate an expression on the current thread.  Displays any
               returned value with LLDB's default formatting.
  continue  -- Continue execution of all threads in the current process.
  detach    -- Detach from the current target process.
  di        -- Disassemble specified instructions in the current target. 
               Defaults to the current function for the current thread and
               stack frame.
  dis       -- Disassemble specified instructions in the current target. 
               Defaults to the current function for the current thread and
               stack frame.
  display   -- Evaluate an expression at every stop (see 'help target
               stop-hook'.)
  down      -- Select a newer stack frame.  Defaults to moving one frame, a
               numeric argument can specify an arbitrary number.
  env       -- Shorthand for viewing and setting environment variables.
  exit      -- Quit the LLDB debugger.
  f         -- Select the current stack frame by index from within the current
               thread (see 'thread backtrace'.)
  file      -- Create a target using the argument as the main executable.
  finish    -- Finish executing the current stack frame and stop after
               returning.  Defaults to current thread unless specified.
  image     -- Commands for accessing information for one or more target
               modules.
  j         -- Set the program counter to a new address.
  jump      -- Set the program counter to a new address.
  kill      -- Terminate the current target process.
  l         -- List relevant source code using one of several shorthand formats.
  list      -- List relevant source code using one of several shorthand formats.
  n         -- Source level single step, stepping over calls.  Defaults to
               current thread unless specified.
  next      -- Source level single step, stepping over calls.  Defaults to
               current thread unless specified.
  nexti     -- Instruction level single step, stepping over calls.  Defaults to
               current thread unless specified.
  ni        -- Instruction level single step, stepping over calls.  Defaults to
               current thread unless specified.
  p         -- Evaluate an expression on the current thread.  Displays any
               returned value with LLDB's default formatting.
  parray    -- parray <COUNT> <EXPRESSION> -- lldb will evaluate EXPRESSION to
               get a typed-pointer-to-an-array in memory, and will display
               COUNT elements of that type from the array.
  po        -- Evaluate an expression on the current thread.  Displays any
               returned value with formatting controlled by the type's author.
  poarray   -- poarray <COUNT> <EXPRESSION> -- lldb will evaluate EXPRESSION to
               get the address of an array of COUNT objects in memory, and will
               call po on them.
  print     -- Evaluate an expression on the current thread.  Displays any
               returned value with LLDB's default formatting.
  q         -- Quit the LLDB debugger.
  r         -- Launch the executable in the debugger.
  rbreak    -- Sets a breakpoint or set of breakpoints in the executable.
  re        -- Commands to access registers for the current thread and stack
               frame.
  repl      -- Evaluate an expression on the current thread.  Displays any
               returned value with LLDB's default formatting.
  run       -- Launch the executable in the debugger.
  s         -- Source level single step, stepping into calls.  Defaults to
               current thread unless specified.
  shell     -- Run a shell command on the host.
  si        -- Instruction level single step, stepping into calls.  Defaults to
               current thread unless specified.
  sif       -- Step through the current block, stopping if you step directly
               into a function whose name matches the TargetFunctionName.
  step      -- Source level single step, stepping into calls.  Defaults to
               current thread unless specified.
  stepi     -- Instruction level single step, stepping into calls.  Defaults to
               current thread unless specified.
  t         -- Change the currently selected thread.
  tbreak    -- Set a one-shot breakpoint using one of several shorthand formats.
  undisplay -- Stop displaying expression at every stop (specified by stop-hook
               index.)
  up        -- Select an older stack frame.  Defaults to moving one frame, a
               numeric argument can specify an arbitrary number.
  v         -- Show variables for the current stack frame. Defaults to all
               arguments and local variables in scope. Names of argument,
               local, file static and file global variables can be specified.
               Children of aggregate variables can be specified such as
               'var->child.x'.  The -> and [] operators in 'frame variable' do
               not invoke operator overloads if they exist, but directly access
               the specified element.  If you want to trigger operator
               overloads use the expression command to print the variable
               instead.
               It is worth noting that except for overloaded operators, when
               printing local variables 'expr local_var' and 'frame var
               local_var' produce the same results.  However, 'frame variable'
               is more efficient, since it uses debug information and memory
               reads directly, rather than parsing and evaluating an
               expression, which may even involve JITing and running code in
               the target program.
  var       -- Show variables for the current stack frame. Defaults to all
               arguments and local variables in scope. Names of argument,
               local, file static and file global variables can be specified.
               Children of aggregate variables can be specified such as
               'var->child.x'.  The -> and [] operators in 'frame variable' do
               not invoke operator overloads if they exist, but directly access
               the specified element.  If you want to trigger operator
               overloads use the expression command to print the variable
               instead.
               It is worth noting that except for overloaded operators, when
               printing local variables 'expr local_var' and 'frame var
               local_var' produce the same results.  However, 'frame variable'
               is more efficient, since it uses debug information and memory
               reads directly, rather than parsing and evaluating an
               expression, which may even involve JITing and running code in
               the target program.
  vo        -- Show variables for the current stack frame. Defaults to all
               arguments and local variables in scope. Names of argument,
               local, file static and file global variables can be specified.
               Children of aggregate variables can be specified such as
               'var->child.x'.  The -> and [] operators in 'frame variable' do
               not invoke operator overloads if they exist, but directly access
               the specified element.  If you want to trigger operator
               overloads use the expression command to print the variable
               instead.
               It is worth noting that except for overloaded operators, when
               printing local variables 'expr local_var' and 'frame var
               local_var' produce the same results.  However, 'frame variable'
               is more efficient, since it uses debug information and memory
               reads directly, rather than parsing and evaluating an
               expression, which may even involve JITing and running code in
               the target program.
  x         -- Read from the memory of the current target process.
Current user-defined commands:
  alamborder    -- Put a border around views with an ambiguous layout
  alamunborder  -- Removes the border around views with an ambiguous layout
  bdisable      -- Disable a set of breakpoints for a regular expression
  benable       -- Enable a set of breakpoints for a regular expression
  binside       -- Set a breakpoint for a relative address within the
                   framework/library that's currently running. This does the
                   work of finding the offset for the framework/library and
                   sliding your address accordingly.
  bmessage      -- Set a breakpoint for a selector on a class, even if the
                   class itself doesn't override that selector. It walks the
                   hierarchy until it finds a class that does implement the
                   selector and sets a conditional breakpoint there.
  border        -- Draws a border around <viewOrLayer>. Color and width can be
                   optionally provided. Additionally depth can be provided in
                   order to recursively border subviews.
  caflush       -- Force Core Animation to flush. This will 'repaint' the UI
                   but also may mess with ongoing animations.
  copy          -- Copy data to your Mac.
  dcomponents   -- Set debugging options for components.
  dismiss       -- Dismiss a presented view controller.
  fa11y         -- Find the views whose accessibility labels match labelRegex
                   and puts the address of the first result on the clipboard.
  findinstances -- Find instances of specified ObjC classes.
  flicker       -- Quickly show and hide a view to quickly help visualize where
                   it is.
  fv            -- Find the views whose class names match classNameRegex and
                   puts the address of first on the clipboard.
  fvc           -- Find the view controllers whose class names match
                   classNameRegex and puts the address of first on the
                   clipboard.
  heapfrom      -- Show all nested heap pointers contained within a given
                   variable.
  hide          -- Hide a view or layer.
  mask          -- Add a transparent rectangle to the window to reveal a
                   possibly obscured or hidden view or layer's bounds
  mwarning      -- simulate a memory warning
  pa11y         -- Print accessibility labels of all views in hierarchy of
                   <aView>
  pa11yi        -- Print accessibility identifiers of all views in hierarchy of
                   <aView>
  pactions      -- Print the actions and targets of a control.
  paltrace      -- Print the Auto Layout trace for the given view. Defaults to
                   the key window.
  panim         -- Prints if the code is currently execution with a UIView
                   animation block.
  pbcopy        -- Print object and copy output to clipboard
  pblock        -- Print the block`s implementation address and signature
  pbundlepath   -- Print application's bundle directory path.
  pcells        -- Print the visible cells of the highest table view in the
                   hierarchy.
  pclass        -- Print the inheritance starting from an instance of any class.
  pcomponents   -- Print a recursive description of components found starting
                   from <aView>.
  pcurl         -- Print the NSURLRequest (HTTP) as curl command.
  pdata         -- Print the contents of NSData object as string.
  pdocspath     -- Print application's 'Documents' directory path.
  pinternals    -- Show the internals of an object by dereferencing it as a
                   pointer.
  pinvocation   -- Print the stack frame, receiver, and arguments of the
                   current invocation. It will fail to print all arguments if
                   any arguments are variadic (varargs).
  pivar         -- Print the value of an object's named instance variable.
  pjson         -- Print JSON representation of NSDictionary or NSArray object
  pkp           -- Print out the value of the key path expression using
                   -valueForKeyPath:
  pmethods      -- Print the class and instance methods of a class.
  poobjc        -- Print the expression result, with the expression run in an
                   ObjC++ context. (Shortcut for "expression -O -l ObjC++ -- " )
  pproperties   -- Print the properties of an instance or Class
  present       -- Present a view controller.
  presponder    -- Print the responder chain starting from a specific responder.
  psjson        -- Print JSON representation of Swift Dictionary or Swift Array
                   object
  ptv           -- Print the highest table view in the hierarchy.
  pvc           -- Print the recursion description of <aViewController>.
  pviews        -- Print the recursion description of <aView>.
  rcomponents   -- Synchronously reflow and update all components.
  sequence      -- Run commands in sequence, stopping on any error.
  setinput      -- Input text into text field or text view that is first
                   responder.
  settext       -- Set text on text on a view by accessibility id.
  show          -- Show a view or layer.
  slowanim      -- Slows down animations. Works on the iOS Simulator and a
                   device.
  taplog        -- Log tapped view to the console.
  uikit         -- Imports the UIKit module to get access to the types while in
                   lldb.
  unborder      -- Removes border around <viewOrLayer>.
  unmask        -- Remove mask from a view or layer
  unslowanim    -- Turn off slow animations.
  visualize     -- Open a UIImage, CGImageRef, UIView, CALayer, or
                   CVPixelBuffer in Preview.app on your Mac.
  vs            -- Interactively search for a view by walking the hierarchy.
  wivar         -- Set a watchpoint for an object's instance variable.
  xdebug        -- Print debug description the XCUIElement in human readable
                   format.
  xnoid         -- Print XCUIElement objects with label but without identifier.
  xobject       -- Print XCUIElement details.
  xtree         -- Print XCUIElement subtree.
  zzz           -- Executes specified lldb command after delay.
For more information on any command, type 'help <command-name>'.
(lldb) 

```

## 内容规划

1. 介绍 chisel 安装、使用细节、官方提供的命令列表[wiki](https://github.com/facebook/chisel/wiki)、Custom Commands如何进行自定义命令。[chisel](https://github.com/facebook/chisel/blob/master/README.md)

![截屏2021-09-02 下午10.35.11.png](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/b70fa058010847499da535b95c4ba883~tplv-k3u1fbpfcp-watermark.image)
&emsp;Add the following line to ~/.lldbinit to load chisel when Xcode launches:
`command script import /usr/local/opt/chisel/libexec/fbchisellldb.py` 

2. LLDB 给的命令列表使用示例。

3. chisel 命令使用示例。（chisel 里面的命令都是基于 LLDB 支持的 python 脚本来做的，保存在 /opt/homebrew/Cellar/chisel/2.0.1/libexec 路径下。） 



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
