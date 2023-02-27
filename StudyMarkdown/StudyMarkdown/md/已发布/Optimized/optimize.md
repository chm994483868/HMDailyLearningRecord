## Optimized





**参考链接:🔗**
+ [DYLD_PRINT_STATISTICS not working / Xcode 13.0 beta / iOS 15.0 beta 8](https://developer.apple.com/forums/thread/689581)
+ [ryan7cruise/YCSymbolTracker](https://github.com/ryan7cruise/YCSymbolTracker)
+ [iOS App启动优化（六）：实用党直接看这里](https://juejin.cn/post/6844904174287585287)
+ [iOS启动优化之——如何使用Xcode Log、App Launch、代码来计算启动时间 Launch Time](https://www.pudn.com/news/62cd30453662401f6fd47877.html)
+ [使用Instruments了解iOS应用启动时长（Xcode13）](https://www.iosprogrammer.tech/xcode/use-instruments-proj-launch-time/)
+ [Xcode 动态库环境变量——不止DYLD_PRINT_STATISTICS](https://juejin.cn/post/6969163142135971853)
+ [iOS启动优化之——如何使用MetricKit 来计算启动时间 Launch Time](https://blog.csdn.net/njafei/article/details/125726469)
+ [Instruments App Launch 启动时间查看](https://juejin.cn/post/7028788487281180709)

### 启动时间统计：

1. DYLD_PRINT_STATISTICS 已经失效了。
2. 

// 再详细总结一下启动时间的统计方式。

// 使用 Time Profile 查找每一个耗时的方法，右键 reveal in xcode 直接找到这个代码的位置，然后分析能不能把里面的内容转移到 App 首页加载完成以后，就是给它们找延后的位置。
// 发现大部分耗时久的函数都在众多的 +load 函数中，单单是 +load 函数，以非懒加载的方式执行，尽量减少 +load 方法使用，分析 +load 函数内部慢点原因，也是尽量把 +load 方法里面的内容放在启动以后再执行。
