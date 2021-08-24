# iOS App 启动优化(十一)：LLDB 文档：LLDB Quick Start Guide

## About LLDB and Xcode

&emsp;随着 Xcode 5 的发布，LLDB 调试器成为 OS X 上调试体验的基础。

![lldb_in_xc5_command_window_2x.png](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/333151bfbb784e1394d1396e978993cb~tplv-k3u1fbpfcp-watermark.image)

&emsp;LLDB 是 Apple 对 GDB 的 “从零开始” 替代品，它与 LLVM 编译器密切配合开发，为你带来最先进的调试以及流控制和数据检查方面的广泛功能。从 Xcode 5 开始，所有新的和预先存在的开发项目都会自动重新配置为使用 LLDB。

&emsp;标准的 LLDB 安装为你提供了一组广泛的命令，旨在与熟悉的 GDB 命令兼容。除了使用标准配置之外，你还可以轻松自定义 LLDB 以满足你的需求。

### At a Glance

&emsp;LLDB 与 Xcode 5 完全集成，用于源代码开发和构建和运行调试体验。您可以使用 Xcode UI 提供的控件和从 Xcode 调试控制台发出的命令访问其丰富的功能。

#### Understand LLDB Basics to Unlock Advanced Features（了解 LLDB 基础知识以解锁高级功能）

&emsp;通过 LLDB 命令语言，你可以使用 LLDB 的高级功能。命令语法规则且易于学习。许多命令都是通过包含的快捷方式表达的，从而节省你的时间和击键次数。并且你可以使用 LLDB 帮助系统快速检查和了解现有命令、快捷方式和命令选项的详细信息。

&emsp;你可以使用命令别名功能自定义 LLDB。你还可以通过使用 Python 脚本和 Python-LLDB 对象库来扩展 LLDB。

&emsp;**Relevant chapter:** Getting Started with LLDB

#### Use LLDB Equivalents for Common GDB Commands（使用 LLDB 等效项代替通用的 GDB 命令）

&emsp;LLDB 在交付时包括许多与 GDB 命令相同的命令别名。如果你已经有使用 GDB 命令的经验，你可以使用提供的表来查找 GDB 命令并找到 LLDB 等效项，包括规范和速记形式。

&emsp;**Relevant chapter:** GDB and LLDB Command Examples

#### Standalone LLDB Workflow

&emsp;你通常通过使用 Xcode 调试功能体验 LLDB，并使用 Xcode 控制台发出 LLDB 命令。但是，对于开源和其他非基于 GUI 的应用程序调试的开发，你可以从终端窗口使用 LLDB 作为传统的命令行调试器。

&emsp;了解 LLDB 如何作为命令行调试器工作，也可以帮助你在 Xcode 控制台中理解和使用 LLDB 的全部功能。

&emsp;**Relevant Chapter:** Using LLDB as a Standalone Debugger

### See Also

&emsp;要详细了解如何使用 Xcode 的调试功能，所有这些功能都由 LLDB 调试引擎提供支持，可参阅：WWDC 2013 session video for Tools #407 WWDC 2013: Debugging with Xcode。

&emsp;要查看最新的高级技术以帮助你有效地跟踪 LLDB 的错误，可参阅：WWDC 2013 session video for Tools #413 WWDC 2013: Advanced Debugging with LLDB.

&emsp;有关使用 LLDB Python 脚本和其他高级功能的更多信息，请访问 [The LLDB Debugger](https://lldb.llvm.org)。

## Getting Started with LLDB

&emsp;LLDB 是一个命令行调试环境，其功能类似于 GDB。 LLDB 为 Xcode 提供了底层调试环境，其中包括调试区域中的控制台，用于在 Xcode IDE 环境中直接访问 LLDB 命令。

&emsp;本章简要解释了 LLDB 语法和命令功能，告知你命令别名功能的使用，并向你介绍 LLDB 帮助系统。

### LLDB Command Structure

&emsp;所有开始使用 LLDB 的用户都应该了解 LLDB 命令结构和语法，以便挖掘 LLDB 的潜力并了解如何从中获得最大收益。在许多情况下，LLDB 提供的命令（例如 `list` 和 `b`）的工作方式与 GDB 命令一样，并且使有经验的 GDB 用户更容易学习 LLDB。

#### Understanding Command Syntax

&emsp;LLDB 命令语法自始至终都是结构化和规则的。 LLDB 命令都是以下形式：

```c++
<command> [<subcommand> [<subcommand>...]] <action> [-options [option-value]] [argument [argument...]]
```

&emsp;命令（`Command`）和子命令（`subcommand`）是 LLDB 调试器对象的名称。命令和子命令按层次排列：一个特定的命令对象为它后面的子命令对象创建上下文，它再次为下一个子命令提供上下文，依此类推。
行动（`action`）是你要在组合调试器对象（它前面的 command subcommand.. 实体）的上下文中执行的操作。选项（`options`）是 `action` 修饰符，通常带有一个值。根据正在使用的命令的上下文，参数代表各种不同的东西。例如：使用 `process launch` 命令启动进程，该上下文中的参数是你在命令行中输入的内容，就像你在调试会话之外调用进程一样。

&emsp;LLDB 命令行解析在命令执行之前完成。命令（`Commands`）、子命令（`subcommands`）、选项（`options`）、选项值（`option values`）和参数（`arguments`）都是用空格分隔的，双引号用于保护选项值（`option values`）和参数（`arguments`）中的空格。如果需要将反斜杠或双引号字符放入参数（`argument`）中，请在参数（`argument`）中在该字符前加上反斜杠。 LLDB 等效地使用单引号和双引号，允许轻松编写命令行的双引号部分。例如：

```c++
(lldb) command [subcommand] -option "some \"quoted\" string"
```

&emsp;也可以写成：

```c++
(lldb) command [subcommand] -option 'some "quoted" string'
```

&emsp;这种命令解析设计有助于使所有命令的 LLDB 命令语法规则和统一。 （对于 GDB 用户，这也意味着你可能必须在 LLDB 中引用一些在 GDB 中不必引用的参数。）

&emsp;作为一个简单的 LLDB 命令示例，要在文件 `test.c` 的第 12 行设置断点，请输入：

```c++
(lldb) breakpoint set --file test.c --line 12
```

&emsp;似乎与此模型不同的命令（例如，`print` 或 `b`）通常是由命令别名机制创建的自定义命令形式，这在下面 Command Aliases and Help 一节中进行了讨论。

#### Using Command Options

&emsp;LLDB 中的命令选项（`command options`）具有规范（也称为 “discoverable” ）形式和缩写形式。例如，这里是 `breakpoint set` 命令的命令选项的部分列表，在括号中列出了规范形式：

```c++
breakpoint set
       -M <method> ( --method <method> )
       -S <selector> ( --selector <selector> )
       -b <function-name> ( --basename <function-name> )
       -f <filename> ( --file <filename> )
       -l <linenum> ( --line <linenum> )
       -n <function-name> ( --name <function-name> )
…
```

&emsp;选项（`options`）可以在命令后面的命令行中以任何顺序放置。如果参数以连字符 (`-`) 开头，则通过添加选项终止信号：双连字符 (`--`)，向 LLDB 指示你已完成当前命令的选项。例如，如果你想启动一个进程并为 `process launch` 命令提供 `--stop-at-entry` 选项，并希望使用参数 `-program_arg_1` 值和 `-program_arg_2` 值启动相同的进程，请输入：

```c++
(lldb) process launch --stop-at-entry -- -program_arg_1 value -program_arg_2 value
```

#### Using Raw Commands

&emsp;LLDB 命令解析器支持 “原始” 命令（“raw” commands），其中在删除命令选项后，命令字符串的其余部分将未经解释地传递给命令。这对于参数可能是一些复杂表达式的命令很方便，这些表达式用反斜杠保护起来很笨拙。例如，`expression` 命令是一个原始命令。

&emsp;当你通过 `help` 查找命令时，命令的输出会告诉你该命令是否为原始命令，以便你知道会发生什么。

&emsp;原始命令（`raw commands`）仍然可以有选项，如果你的命令字符串中有破折号，你可以通过在命令名称之后但在命令字符串之前放置选项终止 (`--`) 来指示这些不是选项标记。

#### Using Command Completion in LLDB

&emsp;LLDB 支持源文件名（`source file names`）、符号名（`symbol names`）、文件名（`file names`）等的命令补全。通过在命令行上输入制表符来启动终端窗口中的补全。 Xcode 控制台中的补全与源代码编辑器中的补全的工作方式相同：在键入第三个字符后会自动弹出补全，并且可以通过 `Esc (Escape)` 键手动调用补全弹出窗口。此外，Xcode 控制台中的补全遵循 `Editing` 面板中指定的 `Xcode Text Editing` 首选项。

&emsp;命令中的各个选项可以有不同的补全符。例如，`breakpoint` 中的 `--file <path>` 选项补全源文件，`--shlib <path>` 选项补全当前加载的共享库，等等。该行为可能非常具体，例如：如果你指定 `--shlib <path>`，并且正在 `--file <path>` 上补全，LLDB 仅列出 `--shlib <path>` 指定的共享库中的源文件。

#### Comparing LLDB with GDB

&emsp;与 GDB 相比，使用 LLDB 时，LLDB 命令解析器的命令行解析和统一性有所不同。 LLDB 命令语法有时会迫使你更明确地说明你的意图，但它在使用中更为常规。

&emsp;例如，设置断点是一种常见的操作。在 GDB 中，要设置断点，你可以输入以下内容在 `foo.c` 的第 12 行中断：

```c++
(gdb) break foo.c:12
```

&emsp;你可以输入以下内容来中断函数 foo：

```c++
(gdb) break foo
```

&emsp;GDB 中可以使用更复杂的 `break` 表达式。例如：`(gdb) break foo.c::foo`，意思是：“在文件 `foo.c` 中的函数 `foo` 中设置断点”。但是在某些时候，GDB 语法变得复杂并限制了 GDB 的功能，尤其是在 C++ 中，可能没有可靠的方法来指定要中断的函数。之所以出现这些缺陷，是因为 GDB 命令行语法由一组可能相互矛盾的特殊表达式解析器支持。

&emsp;相比之下，LLDB 断点（`breakpoint`）命令在其表达式中只需要一种简单、直接的方法，并提供智能自动完成的优点以及在更复杂的情况下设置断点的能力。要在 LLDB 中设置相同的文件和行断点，请输入：

```c++
(lldb) breakpoint set --file foo.c --line 12
```

&emsp;要在 LLDB 中名为 `foo` 的函数上设置断点，请输入：

```c++
(lldb) breakpoint set --name foo
```

&emsp;在 LLDB 中按名称设置断点比在 GDB 中更强大，因为你可以指定要按方法名称在函数上设置断点。要在所有名为 foo 的 C++ 方法上设置断点，请输入：

```c++
(lldb) breakpoint set --method foo
```

&emsp;要在名为 `alignLeftEdges:` 的 Objective-C 选择子上设置断点，请输入：

```c++
(lldb) breakpoint set --selector alignLeftEdges:
```

&emsp;你可以使用 `--shlib <path>` 表达式将任何断点限制在特定的可执行静像内（`executable image`）。

```c++
(lldb) breakpoint set --shlib foo.dylib --name foo
```

&emsp;本节中介绍的 LLDB 命令使用可发现的命令名称和选项的规范形式，可能看起来有些冗长。然而，就像在 GDB 中一样，LLDB 命令解释器对命令名称进行最短唯一字符串匹配，创建一个缩写的命令形式。例如，以下两个命令行表达式演示了相同的命令：

```c++
(lldb) breakpoint set --name "-[SKTGraphicView alignLeftEdges:]"
(lldb) br s --name "-[SKTGraphicView alignLeftEdges:]"
```

&emsp;同样，你可以将最短唯一字符串匹配与缩写选项格式结合起来以减少击键次数。将两者结合使用可以进一步减少命令行表达式。例如：

```c++
(lldb) breakpoint set --shlib foo.dylib --name foo
```

&emsp;变为：

```c++
(lldb) br s -s foo.dylib -n foo
```

&emsp;使用 LLDB 的这些特性提供了与使用 GDB 时几乎相同的 “速记” 感觉和简洁性。查看 Breakpoint Commands 和 GDB and LLDB Command Examples 中的其他部分，了解有关最短唯一字符串匹配和使用缩写形式选项如何节省击键次数的更多示例。

> &emsp;Note：你还可以使用 Command Aliases and Help 中讨论的命令别名（`command alias`），使常用的命令行表达式易于记忆，并且只需几次击键即可输入。

#### Scripting with Python

&emsp;对于高级用户，LLDB 具有可使用 script 命令访问的内置 Python 解释器。调试器的所有功能都可以作为 Python 解释器中的类使用。因此，可以通过使用 LLDB-Python 库编写 Python 函数，然后将脚本加载到正在运行的会话中，使用脚本命令访问它们来实现 GDB 中使用 define 命令引入的更复杂的命令。有关 LLDB-Python 库的更多信息，请访问 LLDB 调试器网站的 LLDB Python 参考和 LLDB Python 脚本示例部分。

### Command Aliases and Help

&emsp;现在你了解了 LLDB 语法和命令行动态，将注意力转向 LLDB 的两个非常有用的功能——命令别名和帮助系统。

#### Understanding Command Aliases

&emsp;使用 LLDB 中的命令别名机制为常用命令构造别名。例如，如果你重复输入：

```c++
(lldb) breakpoint set --file foo.c --line 12
```

&emsp;你可以使用以下命令构造别名：

```c++
(lldb) command alias bfl breakpoint set -f %1 -l %2
```

&emsp;它可以让你输入这个命令：

```c++
(lldb) bfl foo.c 12
```

&emsp;因为命令别名在各种情况下都很有用，所以你应该熟悉它们的构造。有关命令别名构造、限制和语法的完整说明，请使用 LLDB help 系统。输入：

```c++
(lldb) help command alias
```

> &emsp;Note: 在其默认配置中，一些常用命令的别名已添加到 LLDB（例如，`step`、`next` 和 `continue`），但并未尝试构建详尽的别名集。根据 LLDB 开发团队的经验，将基本命令设置为唯一的一两个字母，然后学习这些序列，而不是用大量别名填充命名空间，然后必须键入它们更方便。

  然而，用户可以按照自己喜欢的任何方式自由自定义 LLDB 命令集。 LLDB 在启动时读取文件 `~/.lldbinit`。这是一个文件，用于存储定义时使用 `command alias` 命令创建的别名。 LLDB 帮助系统读取此初始化文件并显示别名，以便你可以轻松提醒自己已设置的内容。要查看所有当前定义的别名及其定义，请使用 `help -a` 命令并在帮助输出的末尾找到当前定义的别名，从以下内容开始：
  ```c++
  ...
  The following is a list of your current command abbreviations (see 'help command alias' for more info): ...

 ```

#### Using LLDB Help

&emsp;探索 LLDB Help 系统，以更广泛地了解 LLDB 必须提供的内容并查看 LLDB 命令构造的详细信息。熟悉 help 命令可以让你访问帮助系统中的大量命令文档。

&emsp;对 help 命令的简单调用将返回所有顶级 LLDB 命令的列表。例如，这里是部分清单：

```c++
(lldb) help
The following is a list of built-in, permanent debugger commands:
 
_regexp-attach    -- Attach to a process id if in decimal, otherwise treat the
argument as a process name to attach to.
_regexp-break     -- Set a breakpoint using a regular expression to specify the
                     location, where <linenum> is in decimal and <address> is
                     in hex.
_regexp-bt        -- Show a backtrace.  An optional argument is accepted; if
                     that argument is a number, it specifies the number of
                     frames to display.  If that argument is 'all', full
                     backtraces of all threads are displayed.
 … and so forth …
```

&emsp;调用 `help` 后跟任何命令会列出该命令的帮助条目以及任何子命令、选项和参数。通过在所有命令上迭代调用 help <command> <subcommand> [<subcommand>...]，你可以遍历整个 LLDB 命令层次结构。

&emsp;当使用选项 `--show-aliases`（简称 `-a`）调用时，`help` 命令显示包括所有当前命令别名。

```c++
(lldb) help -a
```

> &emsp;Note：`help -a` 生成的列表包括所有提供的命令别名和 GDB 命令模拟以及你定义的任何命令别名。

> &emsp;Important：流行需求中包含的一个别名是 GDB break 命令的弱模拟。它不会做 GDB break 命令所做的所有事情（例如，它不处理像 break foo.c::bar 这样的表达式）。相反，它处理更常见的情况，并使有经验的 GDB 用户更容易过渡到使用 LLDB。 GDB 中断仿真默认情况下也别名为 b，与 GDB 约定匹配。
为了学习和练习本机的 LLDB 命令集，这些提供的默认值将 GDB 中断模拟置于其余 LLDB 断点命令的方式中。在所有命令别名的一般情况下，如果你不喜欢提供的别名之一，或者如果它妨碍了，你可以轻松摆脱它。以 GDB 中断仿真器别名 b 为例，以下命令删除提供的 GDB b 仿真别名：

```c++
(lldb) command unalias b
```

&emsp;你还可以运行：

```c++
(lldb) command alias b breakpoint
```

&emsp;它允许你仅使用 b 运行本机 LLDB breakpoint 命令。

&emsp;探索 LLDB 中可用内容的一种更直接的方法是使用 `apropos` 命令：它在 LLDB 帮助文档中搜索一个单词，并为每个匹配的命令转储一个摘要帮助字符串。例如：

```c++
(lldb) apropos file
The following commands may relate to 'file':
…
log enable                     -- Enable logging for a single log channel.
memory read                    -- Read from the memory of the process being
                                  debugged.
memory write                   -- Write to the memory of the process being
                                  debugged.
platform process launch        -- Launch a new process on a remote platform.
platform select                -- Create a platform if needed and select it as
                                  the current platform.
plugin load                    -- Import a dylib that implements an LLDB
                                  plugin.
process launch                 -- Launch the executable in the debugger.
process load                   -- Load a shared library into the current
                                  process.
source                         -- A set of commands for accessing source file
                                  information
… and so forth …
```

&emsp;使用带有命令别名的 `help` 命令来了解它们的构造。例如，键入以下命令以查看提供的 GDB b 仿真及其实现的说明：

```c++
(lldb) help b
…
'b' is an abbreviation for '_regexp-break'
```

&emsp;通过研究命令 `break` 命令 `add`（用于 Setting Breakpoints）显示了 `help` 命令的另一个功能。为了演示它，你应该执行命令并检查帮助系统结果：

```c++
(lldb) help break command add
Add a set of commands to a breakpoint, to be executed whenever the breakpoint is hit.
 
Syntax: breakpoint command add <cmd-options> <breakpt-id>
etc...
```

&emsp;在帮助输出中在尖括号中指定的命令参数（例如 `<breakpt-id>`）表明该参数是通用参数类型，通过查询命令系统可以获得进一步的帮助。在这种情况下，要了解有关 `<breakpt-id>` 的更多信息，请输入：

```c++
(lldb) help <breakpt-id>
 
<breakpt-id> -- Breakpoint IDs consist major and minor numbers; the major
etc...
```

&emsp;经常使用 `help` 并浏览 LLDB 帮助文档是熟悉 LLDB 功能范围的好方法。

## GDB and LLDB Command Examples

&emsp;本章中的表格列出了常用的 GDB 命令并提供了等效的 LLDB 命令和替代形式。还列出了 LLDB 中内置的 GDB 兼容性别名。

&emsp;请注意，完整的 LLDB 命令名称可以通过唯一的短格式进行匹配，可以替代使用。例如，可以使用 br se 代替 breakpoint set。

### Execution Commands

<table>
    <tr>
        <td>GDB</td>
        <td>LLDB</td>
    </tr>
    <tr>
        <td colspan="2">启动一个没有参数的进程。</td>
    </tr>
    <tr>
        <td>(gdb) run <br> (gdb) r</td>
        <td>(lldb) process launch <br> (lldb) run <br> (lldb) r</td>
    </tr>
    <tr>
        <td colspan="2">使用参数 <args> 启动进程。</td>
    </tr>
    <tr>
        <td>(gdb) run <args> <br> (gdb) r <args></td>
        <td>(lldb) process launch -- <args> <br> (lldb) r <args></td>
    </tr>
    <tr>
        <td colspan="2">使用参数 1 2 3 启动进程 a.out，而不必每次都提供 args。</td>
    </tr>
    <tr>
        <td>% gdb --args a.out 1 2 3 <br> (gdb) run <br> ... <br> (gdb) run <br> ...</td>
        <td>% lldb -- a.out 1 2 3 <br> (lldb) run <br> ... <br> (lldb) run <br> ...</td>
    </tr>
    <tr>
        <td colspan="2">在新的终端窗口中启动带有参数的进程（仅限 OS X）。</td>
    </tr>
    <tr>
        <td>-</td>
        <td>(lldb) process launch --tty -- <args> <br> (lldb) pro la -t -- <args></td>
    </tr>
    <tr>
        <td colspan="2">在现有终端窗口 /dev/ttys006（仅限 OS X）中使用参数启动进程。</td>
    </tr>
    <tr>
        <td>-</td>
        <td>(lldb) process launch --tty=/dev/ttys006 -- <args> <br> (lldb) pro la -t/dev/ttys006 -- <args></td>
    </tr>
    <tr>
        <td colspan="2">在启动之前为进程设置环境变量。</td>
    </tr>
    <tr>
        <td>(gdb) set env DEBUG 1</td>
        <td>(lldb) settings set target.env-vars DEBUG=1 <br> (lldb) set se target.env-vars DEBUG=1</td>
    </tr>
        <td colspan="2">在一个命令中为进程和启动进程设置环境变量。</td>
    <tr>
        <td></td>
        <td>(lldb) process launch -v DEBUG=1</td>
    </tr>
    <tr>
        <td colspan="2">附加到进程 ID 为 123 的进程。</td>
    </tr>
    <tr>
        <td>(gdb) attach 123</td>
        <td>(lldb) process attach --pid 123 <br> (lldb) attach -p 123</td>
    </tr>
    <tr>
        <td colspan="2">附加到名为 a.out 的进程。</td>
    </tr>
    <tr>
        <td>(gdb) attach a.out</td>
        <td>(lldb) process attach --name a.out <br> (lldb) pro at -n a.out</td>
    </tr>
    <tr>
        <td colspan="2">等待名为 a.out 的进程启动并附加。</td>
    </tr>
    <tr>
        <td>(gdb) attach -waitfor a.out</td>
        <td>(lldb) process attach --name a.out --waitfor <br> (lldb) pro at -n a.out -w</td>
    </tr>
    <tr>
        <td colspan="2">附加到在系统 eorgadd 上运行的远程 GDB 协议服务器，端口 8000。</td>
    </tr>
    <tr>
        <td>(gdb) target remote eorgadd:8000</td>
        <td>(lldb) gdb-remote eorgadd:8000</td>
    </tr>
    <tr>
        <td colspan="2">附加到在本地系统上运行的远程 GDB 协议服务器，端口 8000。</td>
    </tr>
    <tr>
        <td>(gdb) target remote localhost:8000</td>
        <td>(lldb) gdb-remote 8000</td>
    </tr>
    <tr>
        <td colspan="2">在系统 eorgadd 上以 kdp 模式附加到 Darwin 内核。</td>
    </tr>
    <tr>
        <td>(gdb) kdp-reattach eorgadd</td>
        <td>(lldb) kdp-remote eorgadd</td>
    </tr>
    <tr>
        <td colspan="2">在当前选定的线程中执行源级单步。</td>
    </tr>
    <tr>
        <td>(gdb) step <br> (gdb) s</td>
        <td>(lldb) thread step-in <br> (lldb) step <br> (lldb) s</td>
    </tr>
    <tr>
        <td colspan="2">在当前选定的线程中执行源级单步执行。</td>
    </tr>
    <tr>
        <td>(gdb) next <br> (gdb) n</td>
        <td colspan="2">(lldb) thread step-over <br> (lldb) next <br> (lldb) n</td>
    </tr>
    <tr>
        <td colspan="2">在当前选定的线程中执行指令级单步。</td>
    </tr>
    <tr>
        <td>(gdb) stepi <br> (gdb) si</td>
        <td>(lldb) thread step-inst <br> (lldb) si</td>
    </tr>
    <tr>
        <td>在当前选定的线程中执行指令级单步执行。</td>
    </tr>
    <tr>
        <td>(gdb) nexti <br> (gdb) ni</td>
        <td>(lldb) thread step-inst-over <br> (lldb) ni</td>
    </tr>
    <tr>
        <td>跳出当前选定的 frame。</td>
    </tr>
    <tr>
        <td>(gdb) finish</td>
        <td>(lldb) thread step-out <br> (lldb) finish</td>
    </tr>
    <tr>
        <td>每次停止时 backtrace 和 disassemble。</td>
    </tr>
    <tr>
        <td>—</td>
        <td>(lldb) target stop-hook add <br> Enter your stop hook command(s). Type 'DONE' to end. <br> > bt <br> > disassemble --pc <br> > DONE <br> Stop hook #1 added.</td>
    </tr>
</table>
































## 参考链接
**参考链接:🔗**
+ [iOS 启动优化 + 监控实践](https://juejin.cn/post/6844904194877587469) // [已读完]（有多处实践方法可借鉴使用，后续重读进行摘录总结实践）

+ [哈啰出行iOS App首屏秒开优化](https://juejin.cn/post/6948990967324082183) // [已读完]（主要介绍了一些思想，极少的实践）
+ [抖音研发实践：基于二进制文件重排的解决方案 APP启动速度提升超15%](https://mp.weixin.qq.com/s/Drmmx5JtjG3UtTFksL6Q8Q) // [正在进行中...]
+ [iOS App冷启动治理：来自美团外卖的实践](https://juejin.cn/post/6844903733231353863)  // 未开始
+ [APP启动时间最精确的记录方式](https://juejin.cn/post/6844903997153755150)  // 未开始
+ [iOS如何获取当前时间--看我就够了](https://juejin.cn/post/6905671622037307405)  // 未开始
+ [启动优化](https://juejin.cn/post/6983513854546444296)  // 未开始
+ [iOS 优化篇 - 启动优化之Clang插桩实现二进制重排](https://juejin.cn/post/6844904130406793224#heading-29)  // 未开始
+ [懒人版二进制重排](https://juejin.cn/post/6844904192193085448#heading-7)  // 未开始
+ [我是如何让微博绿洲的启动速度提升30%的](https://juejin.cn/post/6844904143111323661)  // 未开始
+ [App性能优化小结](https://juejin.cn/post/6844903704886247431)  // 未开始
+ [美团 iOS 工程 zsource 命令背后的那些事儿](https://tech.meituan.com/2019/08/08/the-things-behind-the-ios-project-zsource-command.html) // 未开始
+ [iOS美团同款"ZSource"二进制调试实现](https://juejin.cn/post/6847897745987125262)
+ [iOS编译速度如何稳定提高10倍以上之一](https://juejin.cn/post/6903407900006449160)
+ [iOS编译速度如何稳定提高10倍以上之二](https://juejin.cn/post/6903408514778497031#heading-35)
