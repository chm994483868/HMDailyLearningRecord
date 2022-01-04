# iOS App Crash 分析：(五)：函数堆栈获取解析

&emsp;搞清楚函数调用栈是怎么获取的，就必须了解这个机制。

&emsp;函数调用栈有个大致的印象，栈帧图：

&emsp;首先了解寄存器，ARM64 有 34 个寄存器，其中 31 个通用寄存器、SP、PC、CPSR。调用约定指定他们其中的一些寄存器有特殊的用途，例如：

&emsp;x0-x28 通用寄存器。

&emsp;x29(FP) 通常用作帧指针 fp (frame pointer 寄存器) ，栈帧基址寄存器，指向当前函数栈帧的栈底。

&emsp;x30(LR) 

















## 参考链接
**参考链接:🔗**
+ [ios-crash-dump-analysis-book](https://github.com/faisalmemon/ios-crash-dump-analysis-book)
+ [ios-crash-dump-analysis-book/zh](https://faisalmemon.github.io/ios-crash-dump-analysis-book/zh/)
+ [iOS Crash/崩溃/异常 堆栈获取](https://www.jianshu.com/p/8ece78d71b3d)
+ [iOS堆栈信息解析（函数地址与符号关联）](https://www.jianshu.com/p/df5b08330afd)

+ [iOS 的崩溃捕获-堆栈符号化-崩溃分析](https://www.jianshu.com/p/302ed945e9cf)
+ [iOS/OSX Crash：捕捉异常](https://zhuanlan.zhihu.com/p/271282052)
