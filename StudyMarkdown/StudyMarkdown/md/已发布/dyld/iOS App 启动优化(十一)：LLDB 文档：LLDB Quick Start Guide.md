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
        <td>(lldb) thread step-over <br> (lldb) next <br> (lldb) n</td>
    </tr>
    <tr>
        <td colspan="2">在当前选定的线程中执行指令级单步。</td>
    </tr>
    <tr>
        <td>(gdb) stepi <br> (gdb) si</td>
        <td>(lldb) thread step-inst <br> (lldb) si</td>
    </tr>
    <tr>
        <td colspan="2">在当前选定的线程中执行指令级单步执行。</td>
    </tr>
    <tr>
        <td>(gdb) nexti <br> (gdb) ni</td>
        <td>(lldb) thread step-inst-over <br> (lldb) ni</td>
    </tr>
    <tr>
        <td colspan="2">跳出当前选定的 frame。</td>
    </tr>
    <tr>
        <td>(gdb) finish</td>
        <td>(lldb) thread step-out <br> (lldb) finish</td>
    </tr>
    <tr>
        <td colspan="2">每次停止时 backtrace 和 disassemble。</td>
    </tr>
    <tr>
        <td>—</td>
        <td>(lldb) target stop-hook add <br> Enter your stop hook command(s). Type 'DONE' to end. <br> > bt <br> > disassemble --pc <br> > DONE <br> Stop hook #1 added.</td>
    </tr>
</table>

### Breakpoint Commands

<table>
    <tr>
        <td>GDB</td>
        <td>LLDB</td>
    </tr>
    <tr>
        <td colspan="2">在所有名为 main 的函数上设置断点。</td>
    </tr>
    <tr>
        <td>(gdb) break main</td>
        <td>(lldb) breakpoint set --name main <br> (lldb) br s -n main <br> (lldb) b main</td>
    </tr>
    <tr>
        <td colspan="2">在文件 test.c 的第 12 行设置断点。</td>
    </tr>
    <tr>
        <td>(gdb) break test.c:12</td>
        <td>(lldb) breakpoint set --file test.c --line 12 <br> (lldb) br s -f test.c -l 12 <br> (lldb) b test.c:12</td>
    </tr>
    <tr>
        <td colspan="2">在所有 basename 为 main 的 C++ 方法上设置断点。</td>
    </tr>
    <tr>
        <td>(gdb) break main <br> （注意：这将中断任何名为 main 的 C 函数。）</td>
        <td>(lldb) breakpoint set --method main <br> (lldb) br s -M main</td>
    </tr>
    <tr>
        <td colspan="2">在 Objective-C 函数 -[NSString stringWithFormat:] 处设置断点。</td>
    </tr>
    <tr>
        <td>(gdb) break -[NSString stringWithFormat:]</td>
        <td>(lldb) breakpoint set --name "-[NSString stringWithFormat:]" <br> (lldb) b -[NSString stringWithFormat:]</td>
    </tr>
    <tr>
        <td colspan="2">在所有选择子为 count 的 Objective-C 方法上设置断点。</td>
    </tr>
    <tr>
        <td>(gdb) break count <br> （注意：这将中断任何名为 count 的 C 或 C++ 函数。）</td>
        <td>(lldb) breakpoint set --selector count <br> (lldb) br s -S count</td>
    </tr>
    <tr>
        <td colspan="2">通过函数名上的正则表达式设置断点。</td>
    </tr>
    <tr>
        <td>(gdb) rbreak regular-expression</td>
        <td>(lldb) breakpoint set --regex regular-expression <br> (lldb) br s -r regular-expression</td>
    </tr>
    <tr>
        <td colspan="2">通过源文件内容的正则表达式设置断点。</td>
    </tr>
    <tr>
        <td>(gdb) shell grep -e -n pattern source-file <br> (gdb) break source-file:CopyLineNumbers</td>
        <td>(lldb) breakpoint set --source-pattern regular-expression --file SourceFile <br> (lldb) br s -p regular-expression -f file</td>
    </tr>
    <tr>
        <td colspan="2">列出所有断点。</td>
    </tr>
    <tr>
        <td>(gdb) info break</td>
        <td>(lldb) breakpoint list <br> (lldb) br l</td>
    </tr>
    <tr>
        <td colspan="2">删除一个断点。</td>
    </tr>
    <tr>
        <td>(gdb) delete 1</td>
        <td>(lldb) breakpoint delete 1 <br> (lldb) br del 1</td>
    </tr>
</table>

### Watchpoint Commands

<table>
    <tr>
        <td>GDB</td>
        <td>LLDB</td>
    </tr>
    <tr>
        <td colspan="2">在写入变量时在变量上设置 watchpoint。</td>
    </tr>
    <tr>
        <td>(gdb) watch global_var</td>
        <td>(lldb) watchpoint set variable global_var <br> (lldb) wa s v global_var</td>
    </tr>
    <tr>
        <td colspan="2">写入时在内存位置上设置 watchpoint。</td>
    </tr>
    <tr>
        <td>(gdb) watch -location g_char_ptr</td>
        <td>(lldb) watchpoint set expression -- my_ptr <br> (lldb) wa s e -- my_ptr <br> 注意：如果未指定 -x byte_size，则要监视的区域大小默认为指针大小。此命令采用 "raw" 输入，作为表达式返回一个指向区域开头的无符号整数，在选项终止符 (--) 之后进行评估。</td>
    </tr>
    <tr>
        <td colspan="2">在 watchpoint 上设置条件。</td>
    </tr>
    <tr>
        <td>-</td>
        <td>(lldb) watch set var global <br> (lldb) watchpoint modify -c '(global==5)' <br> c <br> ... <br> bt <br> * thread #1: tid = 0x1c03, 0x0000000100000ef5 a.out`modify + 21 at main.cpp:16, stop reason = watchpoint 1 <br> frame #0: 0x0000000100000ef5 a.out`modify + 21 at main.cpp:16 <br> frame #1: 0x0000000100000eac a.out`main + 108 at main.cpp:25 <br> frame #2: 0x00007fff8ac9c7e1 libdyld.dylib`start + 1 <br> (int32_t) global = 5</td>
    </tr>
    <tr>
        <td colspan="2">列出所有 watchpoints。</td>
    </tr>
    <tr>
        <td>(gdb) info break</td>
        <td>(lldb) watchpoint list <br> (lldb) watch l</td>
    </tr>
    <tr>
        <td colspan="2">删除一个 watchpoint。</td>
    </tr>
    <tr>
        <td>(gdb) delete 1</td>
        <td>(lldb) watchpoint delete 1 <br> (lldb) watch del 1</td>
    </tr>
    <tr>
        <td colspan="2"></td>
    </tr>
    <tr>
        <td></td>
        <td></td>
    </tr>
</table>

### Examining Variables

> &emsp;这里的帧（frame）是指方法的栈帧，例如在下面 -viewDidLoad 内。
   ```c++
  - (void)viewDidLoad {
      [super viewDidLoad];
      // Do any additional setup after loading the view.
      int a = 123;
      NSLog(@"%d", a);
  }
  
  // 在 NSLog 下一个断点，使用如下 LLDB 命令：
  
  (lldb) frame variable
  (ViewController *) self = 0x00007fc39a00a600
  (SEL) _cmd = "viewDidLoad"
  (int) a = 123
  (lldb) frame variable --no-args
  (int) a = 123
  (lldb) frame variable --no-locals
  (ViewController *) self = 0x00007fc39a00a600
  (SEL) _cmd = "viewDidLoad"
  (lldb) 
  ```

<table>
    <tr>
        <td>GDB</td>
        <td>LLDB</td>
    </tr>
    <tr>
        <td colspan="2">显示当前帧的参数和局部变量。</td>
    </tr>
    <tr>
        <td>(gdb) info args <br> and <br> (gdb) info locals</td>
        <td>(lldb) frame variable <br> (lldb) fr v</td>
    </tr>
    <tr>
        <td colspan="2">显示当前帧的局部变量。</td>
    </tr>
    <tr>
        <td>(gdb) info locals</td>
        <td>(lldb) frame variable --no-args <br> (lldb) fr v -a</td>
    </tr>
    <tr>
        <td colspan="2">显示局部变量 bar 的内容。</td>
    </tr>
    <tr>
        <td>(gdb) p bar</td>
        <td>(lldb) frame variable bar <br> (lldb) fr v bar <br> (lldb) p bar</td>
    </tr>
    <tr>
        <td colspan="2">显示格式化为十六进制的局部变量 bar 的内容。</td>
    </tr>
    <tr>
        <td>(gdb) p/x bar</td>
        <td>(lldb) frame variable --format x bar <br> (lldb) fr v -f x bar</td>
    </tr>
    <tr>
        <td colspan="2">显示全局变量 baz 的内容。</td>
    </tr>
    <tr>
        <td>(gdb) p baz</td>
        <td>(lldb) target variable baz <br> (lldb) ta v baz</td>
    </tr>
    <tr>
        <td colspan="2">显示当前源文件中定义的全局/静态变量。</td>
    </tr>
    <tr>
        <td>-</td>
        <td>(lldb) target variable <br> (lldb) ta v</td>
    </tr>
    <tr>
        <td colspan="2">每次停止时显示变量 argc 和 argv。</td>
    </tr>
    <tr>
        <td>(gdb) display argc <br> (gdb) display argv</td>
        <td>(lldb) target stop-hook add --one-liner "frame variable argc argv" <br> (lldb) ta st a -o "fr v argc argv" <br> (lldb) display argc <br> (lldb) display argv</td>
    </tr>
    <tr>
        <td colspan="2">仅在名为 main 的函数中停止时才显示变量 argc 和 argv。</td>
    </tr>
    <tr>
        <td>-</td>
        <td>(lldb) target stop-hook add --name main --one-liner "frame variable argc argv" <br> (lldb) ta st a -n main -o "fr v argc argv"</td>
    </tr>
    <tr>
        <td colspan="2">仅当你停在名为 MyClass 的 C 类中时才显示变量 *this。</td>
    </tr>
    <tr>
        <td>-</td>
        <td>(lldb) target stop-hook add --classname MyClass --one-liner "frame variable *this" <br> (lldb) ta st a -c MyClass -o "fr v *this"</td>
    </tr>
</table>

### Evaluating Expressions

<table>
    <tr>
        <td>GDB</td>
        <td>LLDB</td>
    </tr>
    <tr>
        <td colspan="2">评估当前帧中的广义表达式。</td>
    </tr>
    <tr>
        <td>(gdb) print (int) printf ("Print nine: %d.", 4 + 5) <br> 或者，如果你不想看到 void 返回：<br> (gdb) call (int) printf ("Print nine: %d.", 4 + 5)</td>
        <td>(lldb) expr (int) printf ("Print nine: %d.", 4 + 5) <br> 或者使用打印别名： <br> (lldb) print (int) printf ("Print nine: %d.", 4 + 5)</td>
    </tr>
    <tr>
        <td colspan="2">创建便利变量并为其赋值。</td>
    </tr>
    <tr>
        <td>(gdb) set $foo = 5 <br> (gdb) set variable $foo = 5 <br> Or use the print command: <br> (gdb) print $foo = 5 <br> Or use the call command: <br> (gdb) call $foo = 5 <br> To specify the type of the variable: <br> (gdb) set $foo = (unsigned int) 5</td>
        <td>LLDB 评估变量声明表达式，就像你在 C 中编写的那样：<br> (lldb) expr unsigned int $foo = 5</td>
    </tr>
    <tr>
        <td colspan="2">打印对象的 Objective-C 描述。</td>
    </tr>
    <tr>
        <td>(gdb) po [SomeClass returnAnObject]</td>
        <td>(lldb) expr -O -- [SomeClass returnAnObject] <br> Or use the po alias: <br> (lldb) po [SomeClass returnAnObject]</td>
    </tr>
    <tr>
        <td colspan="2">打印表达式结果的动态类型。</td>
    </tr>
    <tr>
        <td>(gdb) set print object 1 <br> (gdb) p someCPPObjectPtrOrReference <br> Note: Only for C++ objects.</td>
        <td>(lldb) expr -d run-target -- [SomeClass returnAnObject] <br> (lldb) expr -d run-target -- someCPPObjectPtrOrReference <br> Or set dynamic type printing as default: <br> (lldb) settings set target.prefer-dynamic run-target</td>
    </tr>
    <tr>
        <td colspan="2">调用函数以在函数的断点处停止。</td>
    </tr>
    <tr>
        <td>(gdb) set unwindonsignal 0 <br> (gdb) p function_with_a_breakpoint()</td>
        <td>(lldb) expr -u 0 -- function_with_a_breakpoint()</td>
    </tr>
</table>

### Examining Thread State

<table>
    <tr>
        <td>GDB</td>
        <td>LLDB</td>
    </tr>
    <tr>
        <td colspan="2">显示当前线程的堆栈回溯。</td>
    </tr>
    <tr>
        <td>(gdb) bt</td>
        <td>(lldb) thread backtrace <br> (lldb) bt</td>
    </tr>
        <tr>
        <td colspan="2">显示所有线程的堆栈回溯。</td>
    </tr>
    <tr>
        <td>(gdb) thread apply all bt</td>
        <td>(lldb) thread backtrace all <br> (lldb) bt all</td>
    </tr>
        <tr>
        <td colspan="2">回溯当前线程的前五帧。</td>
    </tr>
    <tr>
        <td>(gdb) bt 5</td>
        <td>(lldb) thread backtrace -c 5 <br> (lldb) bt 5 (lldb-169 and later) <br> (lldb) bt -c 5 (lldb-168 and earlier)</td>
    </tr>
        <tr>
        <td colspan="2">按索引为当前线程选择不同的堆栈帧。</td>
    </tr>
    <tr>
        <td>(gdb) frame 12</td>
        <td>(lldb) frame select 12 <br> (lldb) fr s 12 <br> (lldb) f 12</td>
    </tr>
        <tr>
        <td colspan="2">列出有关当前线程中当前选定帧的信息。</td>
    </tr>
    <tr>
        <td>—</td>
        <td>(lldb) frame info</td>
    </tr>
        <tr>
        <td colspan="2">选择调用当前堆栈帧的堆栈帧。</td>
    </tr>
    <tr>
        <td>(gdb) up</td>
        <td>(lldb) up <br> (lldb) frame select --relative=1</td>
    </tr>
        <tr>
        <td colspan="2">选择当前堆栈帧调用的堆栈帧。(下一个栈帧)</td>
    </tr>
    <tr>
        <td>(gdb) down</td>
        <td>(lldb) down <br> (lldb) frame select --relative=-1 <br> (lldb) fr s -r-1</td>
    </tr>
        <tr>
        <td colspan="2">使用相对偏移量选择不同的堆栈帧。</td>
    </tr>
    <tr>
        <td>(gdb) up 2 <br> (gdb) down 3</td>
        <td>(lldb) frame select --relative 2 <br> (lldb) fr s -r2 <br> (lldb) frame select --relative -3 <br> (lldb) fr s -r-3</td>
    </tr>
        <tr>
        <td colspan="2">显示当前线程的 general-purpose registers（读出 general-purpose 寄存器的值）。</td>
    </tr>
    <tr>
        <td>(gdb) info registers</td>
        <td>(lldb) register read</td>
    </tr>
        <tr>
        <td colspan="2">向当前线程寄存器 rax 写下新的十进制值 123。</td>
    </tr>
    <tr>
        <td>(gdb) p $rax = 123</td>
        <td>(lldb) register write rax 123</td>
    </tr>
        <tr>
        <td colspan="2">在当前程序计数器（即 pc 寄存器）（指令指针）之前跳过 8 字节。（当前程序计数寄存器 pc 移动 8 个字节，到下一条指令）</td>
    </tr>
    <tr>
        <td>(gdb) jump *$pc+8</td>
        <td>(lldb) register write pc `$pc+8` <br> LLDB 命令使用背板来评估表达并插入缩放结果。</td>
    </tr>
        <tr>
        <td colspan="2">显示以有符号小数（正负）格式格式的当前线程的通用寄存器。</td>
    </tr>
    <tr>
        <td>—</td>
        <td>(lldb) register read --format i <br> (lldb) re r -f i <br> LLDB 现在支持 GDB 速记格式语法，但命令后不允许使用任何空间：<br> (lldb) register read/d <br> 注意：LLDB 尝试尽可能使用与 printf(3) 相同的格式字符。键入 help format 以查看格式指定器的完整列表。</td>
    </tr>
        <tr>
        <td colspan="2">显示当前线程的所有寄存器集合中的所有寄存器。</td>
    </tr>
    <tr>
        <td>(gdb) info all-registers</td>
        <td>(lldb) register read --all <br> (lldb) re r -a</td>
    </tr>
        <tr>
        <td colspan="2">在当前线程中显示名为 rax、rsp 和 rbp 的寄存器的值。</td>
    </tr>
    <tr>
        <td>(gdb) info all-registers rax rsp rbp</td>
        <td>(lldb) register read rax rsp rbp</td>
    </tr>
        <tr>
        <td colspan="2">在以二进制格式的当前线程中显示名为 rax 的寄存器的值。</td>
    </tr>
    <tr>
        <td>(gdb) p/t $rax</td>
        <td>(lldb) register read --format binary rax <br> (lldb) re r -f b rax <br> LLDB 现在支持 GDB 速记格式语法，但命令后不允许使用任何空间： <br> (lldb) memory read/4xw 0xbffff3c0 <br> (lldb) x/4xw 0xbffff3c0 <br> (lldb) memory read --gdb-format 4xw 0xbffff3c0</td>
    </tr>
        <tr>
        <td colspan="2">从表达 argv[0] 开始阅读内存。</td>
    </tr>
    <tr>
        <td>(gdb) x argv[0]</td>
        <td>(lldb) memory read `argv[0]` <br> 请注意，任何命令都可以使用任何表达的背板将缩放表达结果（只要 target 停止）与内联：<br> (lldb) memory read --size `sizeof(int)` `argv[0]`</td>
    </tr>
        <tr>
        <td colspan="2">从地址 0xbffff3c0 中阅读 512 字节的内存，并将结果保存为文本中的本地文件。</td>
    </tr>
    <tr>
        <td>(gdb) set logging on <br> (gdb) set logging file /tmp/mem.txt <br> (gdb) x/512bx 0xbffff3c0 <br> (gdb) set logging off</td>
        <td>(lldb) memory read --outfile /tmp/mem.txt --count 512 0xbffff3c0 <br> (lldb) me r -o/tmp/mem.txt -c512 0xbffff3c0 <br> (lldb) me r -o/tmp/mem.txt -c512 0xbffff3c0 <br> (lldb) x/512bx -o/tmp/mem.txt 0xbffff3c0</td>
    </tr>
        <tr>
        <td colspan="2">将二进制内存数据保存到从 0x1000 开始到 0x2000 结束的文件。</td>
    </tr>
    <tr>
        <td>(gdb) dump memory /tmp/mem.bin 0x1000 0x2000</td>
        <td>(lldb) memory read --outfile /tmp/mem.bin --binary 0x1000 0x1200 <br> (lldb) me r -o /tmp/mem.bin -b 0x1000 0x1200</td>
    </tr>
        <tr>
        <td colspan="2">Disassemble 当前 frame 的当前 function。（打印当前函数的汇编指令，并指到当前在那个指令的位置）</td>
    </tr>
    <tr>
        <td>(gdb) disassemble</td>
        <td>(lldb) disassemble --frame <br> (lldb) di -f</td>
    </tr>
        <tr>
        <td colspan="2">汇编所有类中名字为 main 的函数。</td>
    </tr>
    <tr>
        <td>(gdb) disassemble main</td>
        <td>(lldb) disassemble --name main <br> (lldb) di -n main</td>
    </tr>
        <tr>
        <td colspan="2">Disassemble an address range. 对指定地址范围的代码转化为汇编。</td>
    </tr>
    <tr>
        <td>(gdb) disassemble 0x1eb8 0x1ec3</td>
        <td>(lldb) disassemble --start-address 0x1eb8 --end-address 0x1ec3 <br> (lldb) di -s 0x1eb8 -e 0x1ec3</td>
    </tr>
        <tr>
        <td colspan="2">从给定地址汇编 20 条指令。</td>
    </tr>
    <tr>
        <td>(gdb) x/20i 0x1eb8</td>
        <td>(lldb) disassemble --start-address 0x1eb8 --count 20 <br> (lldb) di -s 0x1eb8 -c 20</td>
    </tr>
        <tr>
        <td colspan="2">显示当前帧的当前函数的混合源和反汇编。（就是输出源码并输出对应的汇编函数）</td>
    </tr>
    <tr>
        <td>—</td>
        <td>(lldb) disassemble --frame --mixed <br> (lldb) di -f -m</td>
    </tr>
        <tr>
        <td colspan="2">反汇编当前帧的当前函数并显示操作码字节。</td>
    </tr>
    <tr>
        <td>—</td>
        <td>(lldb) disassemble --frame --bytes <br> (lldb) di -f -b</td>
    </tr>
        <tr>
        <td colspan="2">反汇编当前帧的当前源代码行。</td>
    </tr>
    <tr>
        <td>—</td>
        <td>(lldb) disassemble --line <br> (lldb) di -l</td>
    </tr>
</table>

### Executable and Shared Library Query Commands

<table>
    <tr>
        <td>GDB</td>
        <td>LLDB</td>
    </tr>
    <tr>
        <td colspan="2">列出主要的可执行文件和所有相关的共享库。</td>
    </tr>
    <tr>
        <td>(gdb) info shared</td>
        <td>(lldb) image list</td>
    </tr>
    <tr>
        <td colspan="2">在可执行文件或任何共享库中查找原始地址的信息。</td>
    </tr>
    <tr>
        <td>(gdb) info symbol 0x1ec4</td>
        <td>(lldb) image lookup --address 0x1ec4 <br> (lldb) im loo -a 0x1ec4</td>
    </tr>
    <tr>
        <td colspan="2">在二进制文件中查找与正则表达式匹配的函数。</td>
    </tr>
    <tr>
        <td>(gdb) info function <FUNC_REGEX></td>
        <td>This one finds debug symbols: <br> (lldb) image lookup -r -n <FUNC_REGEX> <br> This one finds non-debug symbols: <br> (lldb) image lookup -r -s <FUNC_REGEX> <br> 提供二进制文件列表作为限制搜索的参数。</td>
    </tr>
    <tr>
        <td colspan="2">仅在 a.out 中查找地址信息。</td>
    </tr>
    <tr>
        <td>—</td>
        <td>(lldb) image lookup --address 0x1ec4 a.out <br> (lldb) im loo -a 0x1ec4 a.out</td>
    </tr>
    <tr>
        <td colspan="2">按名称查找类型 Point 的信息。</td>
    </tr>
    <tr>
        <td>(gdb) ptype Point</td>
        <td>(lldb) image lookup --type Point <br> (lldb) im loo -t Point</td>
    </tr>
    <tr>
        <td colspan="2">Dump 主可执行文件和任何共享库中的所有 sections。</td>
    </tr>
    <tr>
        <td>(gdb) maintenance info sections</td>
        <td>(lldb) image dump sections</td>
    </tr>
    <tr>
        <td colspan="2">Dump a.out 模块的所有 sections</td>
    </tr>
    <tr>
        <td>—</td>
        <td>(lldb) image dump sections a.out</td>
    </tr>
    <tr>
        <td colspan="2">Dump 主可执行文件和任何共享库中的所有 symbols。</td>
    </tr>
    <tr>
        <td>—</td>
        <td>(lldb) image dump symtab</td>
    </tr>
    <tr>
        <td colspan="2">Dump a.out 和 liba.so 中的所有 symbols</td>
    </tr>
    <tr>
        <td>—</td>
        <td>(lldb) image dump symtab a.out liba.so</td>
    </tr>
</table>

### Miscellaneous

<table>
    <tr>
        <td>GDB</td>
        <td>LLDB</td>
    </tr>
    <tr>
        <td colspan="2">在屏幕上回显文本。</td>
    </tr>
    <tr>
        <td>(gdb) echo Here is some text\n</td>
        <td>(lldb) script print "Here is some text"</td>
    </tr>
    <tr>
        <td colspan="2">重新映射调试会话的源文件路径名。</td>
    </tr>
    <tr>
        <td>(gdb) set pathname-substitutions /buildbot/path /my/path</td>
        <td>(lldb) settings set target.source-map /buildbot/path /my/path <br> 注意：如果你的源文件不再位于与程序构建时相同的位置——也许程序是在不同的计算机上构建的——你需要告诉调试器如何在本地文件路径而不是构建系统文件路径。</td>
    </tr>
    <tr>
        <td colspan="2">提供一个 catchall 目录来搜索源文件。</td>
    </tr>
    <tr>
        <td>(gdb) directory /my/path</td>
        <td>(No equivalent command yet.)</td>
    </tr>
</table>

## Using LLDB as a Standalone Debugger（使用 LLDB 作为独立调试器）

&emsp;本章介绍基于终端（a basic Terminal）调试会话中的工作流程和操作。在适当的情况下，将 LLDB 操作与类似的 GDB 操作进行比较。

&emsp;大多数情况下，你通过 Xcode 调试功能间接使用 LLDB 调试器，并使用 Xcode 控制台窗格发出 LLDB 命令。但是对于开源和其他基于非 GUI 的应用程序调试的开发，LLDB 在终端窗口中用作传统的命令行调试器。要将 LLDB 用作命令行调试器，你应该了解如何：

+ 加载调试进程
+ 将正在运行的进程附加到 LLDB
+ 设置 breakpoints 和 watchpoints
+ 控制进程执行
+ 在被调试的进程中导航
+ 检查变量的状态和值
+ 执行替代代码（alternative code）

&emsp;Xcode IDE 通过将 LLDB 完全集成到带有图形控件的源代码编辑、构建和 "run for debugging" 循环中，使许多这些操作自动化。从命令行了解这些操作的工作方式还有助于你在 Xcode 控制台窗格中理解和使用 LLDB 调试器的全部功能。

### Specifying the Program to Debug（指定要调试的程序）

&emsp;首先，你需要将程序设置为调试。与 GDB 一样，你可以启动 LLDB 并使用命令行指定要调试的文件。键入：

```c++
$ lldb /Projects/Sketch/build/Debug/Sketch.app
Current executable set to '/Projects/Sketch/build/Debug/Sketch.app' (x86_64).
```

![截屏2021-08-26 07.53.38.png](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/1c549a216d4545d3b95636f5d8a57e16~tplv-k3u1fbpfcp-watermark.image)

&emsp;或者，你可以在可执行文件已经运行后使用 file 命令指定要调试的可执行文件：

```c++
$ lldb
(lldb) file /Projects/Sketch/build/Debug/Sketch.app
Current executable set to '/Projects/Sketch/build/Debug/Sketch.app' (x86_64).
```

![截屏2021-08-26 07.53.42.png](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/19804ba1f39e41928c648297d31c7673~tplv-k3u1fbpfcp-watermark.image)

### Setting Breakpoints

&emsp;接下来，你可能希望设置断点以在进程启动后开始调试。LLDB Command Structure 中简要讨论了设置断点。要查看断点设置的所有选项，请使用 `help breakpoint set`。例如，键入以下内容以在任何使用名为 `alignLeftEdges:` 的方法时设置断点：

```c++
(lldb) breakpoint set --selector alignLeftEdges:
Breakpoint created: 1: name = 'alignLeftEdges:', locations = 1, resolved = 1
```

&emsp;要找出你设置了哪些断点，请键入 `breakpoint list` 命令并检查它返回的内容，如下所示：

```c++
(lldb) breakpoint list
Current breakpoints:
1: name = 'alignLeftEdges:', locations = 1, resolved = 1
  1.1: where = Sketch`-[SKTGraphicView alignLeftEdges:] + 33 at /Projects/Sketch/SKTGraphicView.m:1405, address = 0x0000000100010d5b, resolved, hit count = 0
```
&emsp;(下载了 [Sketch.app](https://www.sketch.com/apps/)，试了一下发现 `alignLeftEdges:` 这个函数已经不存在了，然后又用 MachOView 查看了 Sketch 的二进制文件，发现对其自定义的选择子加断点并不能找到，但是对它所使用到的系统库中的函数加断点可以正常加上。)

```c++
2: name = 'alignLeft:', locations = 6
  2.1: where = WebKit`-[WKView alignLeft:], address = WebKit[0x00007fff3cad71f3], unresolved, hit count = 0 
  2.2: where = WebKit`-[WKWebView(WKImplementationMac) alignLeft:], address = WebKit[0x00007fff3cadb9f7], unresolved, hit count = 0 
  2.3: where = AppKit`-[NSText alignLeft:], address = AppKit[0x00007fff23320dbf], unresolved, hit count = 0 
  2.4: where = AppKit`-[NSTextView alignLeft:], address = AppKit[0x00007fff23348112], unresolved, hit count = 0 
  2.5: where = WebKitLegacy`-[WebHTMLView alignLeft:], address = WebKitLegacy[0x00007fff34fe7e60], unresolved, hit count = 0 
  2.6: where = WebKitLegacy`-[WebView(WebViewEditingActions) alignLeft:], address = WebKitLegacy[0x00007fff350025f0], unresolved, hit count = 0 
```

![截屏2021-08-26 08.55.50.png](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/630b7268686b4abbb0c16b1f6c0b5273~tplv-k3u1fbpfcp-watermark.image)

&emsp;在 LLDB 中，断点有两个部分：断点的逻辑规范，这是用户提供给 `breakpoint set` 命令的内容，以及代码中与该规范匹配的位置。例如，按选择子（`--selector`）中断在程序中的类中实现该选择子的所有方法上设置一个断点。类似地，如果文件和行被内联包含在代码的不同位置，则文件和行断点可能会导致多个位置。

&emsp;`breakpoint list` 命令输出提供的一条信息是逻辑断点（logical breakpoint）的个数，以及每个逻辑断点（logical breakpoint）所处的类以及具体的内存地址以及命中次数。两者由句点 (.) 连接，例如上面：2: 和 2.1:~2.6: 中的内容，表明了断点名和断点个数，然后 2.1:~2.6: 中的全部内容。

&emsp;因为逻辑断点仍然有效，如果加载另一个包含 `alignLeftEdges:` 选择子实现的共享库，新位置将添加到断点 1（即，在新加载的选择子上设置 1.2 断点）。

&emsp;断点列表中的另一条信息是断点位置是否已解析。当一个位置对应的文件地址被加载到正在调试的程序中时，它就会被解析。例如，如果你在稍后卸载的共享库中设置断点，该断点位置将保留但不再解析。

&emsp;LLDB acts like GDB with the command:

```c++
(gdb) set breakpoint pending on
```

&emsp;像 GDB 一样，LLDB 总是从你的规范中创建一个断点，即使它没有找到任何与规范匹配的位置。要确定表达式是否已解析，请使用 `breakpoint list` 检查位置字段。当你设置断点时，LLDB 会将断点报告为挂起（pending）。通过查看处于挂起（pending）状态的断点，你可以通过检查 `breakpoint set` 输出来确定在未找到任何位置时定义断点时是否存在拼写错误。例如：

```c++
(lldb) breakpoint set --file foo.c --line 12
Breakpoint created: 2: file ='foo.c', line = 12, locations = 0 (pending)
WARNING: Unable to resolve breakpoint to any actual locations.
```

&emsp;无论是在逻辑断点生成的所有位置上，还是在逻辑断点解析到的任何特定位置上，你都可以使用断点触发的命令删除、禁用、设置条件和忽略计数。例如，如果你想添加一个命令来在 LLDB 遇到编号为 1.1 的断点时打印回溯，请执行以下命令：

```c++
(lldb) breakpoint command add 1.1
Enter your debugger command(s). Type 'DONE' to end.
> bt
> DONE
```

&emsp;默认情况下，`breakpoint command add` 命令采用 LLDB 命令行命令。要明确指定此默认值，请传递 `--command` 选项（`breakpoint command add --command ...`）。如果你使用 Python 脚本实现断点命令，请使用 `--script` 选项。 LLDB 帮助系统具有解释 `breakpoint command add` 的大量信息。

&emsp;



















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
