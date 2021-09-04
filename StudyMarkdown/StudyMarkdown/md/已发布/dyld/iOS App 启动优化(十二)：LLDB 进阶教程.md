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

&emsp;在 LLDB 调试过程中，打印命令大概是我们用的最多的命令了，`print/prin/pri/p/po` 都能进行打印操作，LLDB 实际上会作前缀匹配，所以我们使用 `print/prin/pri/p` 是完全一样的，但是我们不能使用 `pr`，因为 LLDB 不能消除 `print` 和 `process` 的歧义，不过幸运的是我们可以直接使用 `p`，大概是打印命令用的比较多吧，LLDB 把 `p` 这个最简单的缩写/别名归给了打印命令使用。（如下我们使用 `help pr` 命令，会提示我们使用了不明确的命令名称，并列出了所有以 `pr` 开头的命令，来提示我们具体想要哪个命令。）

```c++
(lldb) help pr
Help requested with ambiguous command name, possible completions:
    process
    print
    present
    presponder
```

&emsp;下面我们区分一下 `print/prin/pri/p` 和 `po`，虽然它们都是打印命令，但是它们的打印格式并不相同。

&emsp;分别通过 `help print`、`help p`、`help po` 打印的结果，我们可以看到 `print/p` 是作用完全一样的命令，`print/p` 都是直接作为 `expression --` 命令的缩写来使用的，它们都是根据 LLDB 的默认格式来进行打印操作，而 `po` 则是 `expression -O  --` 的缩写，如果大家阅读的认真的话，应该还记得 `--` 是用来标记命令的选项结束的，`--` 前面是命令的选项，`--` 后面是命令的参数。`expression -O  --` 中的 `-O` 选项是 `--object-description` 的缩写：`-O ( --object-description ) Display using a language-specific description API, if possible.` 针对 Objective-C 的话，便是调用 `description` 实例函数或者类函数，即使用 `po` 命令进行打印时，它是根据当前语言环境，调用 `description` API 返回的结果进行打印操作。（在 OC 中我们重写 `+/-description` 便可得到自定义的打印结果）   

```c++
print    -- Evaluate an expression on the current thread.  Displays any returned value with LLDB's default formatting.
p        -- Evaluate an expression on the current thread.  Displays any returned value with LLDB's default formatting.
po       -- Evaluate an expression on the current thread.  Displays any returned value with formatting controlled by the type's author.
```

&emsp;`help print` 和 `help p` 都输出如下详细信息：

```c++

(lldb) help print
     Evaluate an expression on the current thread.  Displays any returned value with LLDB's default formatting.  Expects 'raw' input (see 'help raw-input'.)

Syntax: print <expr>

Command Options Usage:
  print <expr>


'print' is an abbreviation for 'expression --'
```

&emsp;`help po` 则输出如下详细信息：

```c++
(lldb) help po
     Evaluate an expression on the current thread.  Displays any returned value with formatting controlled by the type's author.  Expects 'raw' input (see 'help raw-input'.)

Syntax: po <expr>

Command Options Usage:
  po <expr>


'po' is an abbreviation for 'expression -O  --'
```

&emsp;根据详细信息我们可以看到 `p/po` 仅能在当前线程使用，也就是说我们仅能打印当前线程的变量，如果我们打印其它线程的变量的话会得到一个变量未定义的错误：`error: <user expression 0>:1:1: use of undeclared identifier 'xxx'`。（xxx 代指我们要打印的变量名）

























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
