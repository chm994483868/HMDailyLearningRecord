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
