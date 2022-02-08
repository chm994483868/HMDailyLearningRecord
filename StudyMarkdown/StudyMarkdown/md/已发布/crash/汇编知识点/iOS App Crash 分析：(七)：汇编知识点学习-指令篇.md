# iOS App Crash 分析：(七)：汇编知识点学习-指令篇

&emsp;本篇来学习具体的指令。

&emsp;


## 参考链接
**参考链接:🔗**
+ [Intel x86](https://baike.baidu.com/item/Intel%20x86/1012845?fromtitle=x86&fromid=6150538)
+ [x86:SIMD指令集发展历程表（MMX、SSE、AVX 等）](https://blog.csdn.net/weixin_34122604/article/details/86271850?spm=1001.2101.3001.6650.1&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7ERate-1.pc_relevant_default&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7ERate-1.pc_relevant_default&utm_relevant_index=2)
+ [什么是i386,x86和x64, 一文了解处理器架构](https://baike.baidu.com/tashuo/browse/content?id=5d5bb1f8a73bd4495d7b21a5&lemmaId=433177&fromLemmaModule=pcBottom&lemmaTitle=Intel%2080386)
+ [深入iOS系统底层系列文章目录](https://www.jianshu.com/p/139f0899335d)
+ [Man Page(man otool)](https://wangwangok.gitbooks.io/mac-terminal-tool/content/otool.html)
+ [iOS逆向 ：初识汇编](https://www.jianshu.com/p/139f0899335d)
+ [iOS Crash/崩溃/异常 堆栈获取](https://www.jianshu.com/p/8ece78d71b3d)
+ [iOS堆栈信息解析（函数地址与符号关联）](https://www.jianshu.com/p/df5b08330afd)
+ [谈谈iOS堆栈那些事](https://joey520.github.io/2020/03/15/谈谈msgSend为什么不会出现在堆栈中/)
+ [iOS 的崩溃捕获-堆栈符号化-崩溃分析](https://www.jianshu.com/p/302ed945e9cf)
+ [函数栈的实现原理](https://segmentfault.com/a/1190000017151354)
+ [函数调用栈 剖析＋图解[转]](https://www.jianshu.com/p/78e01e513120)
+ [[转载]C语言函数调用栈](https://www.jianshu.com/p/c89d243b8276)
+ [BSBackTracelogger学习笔记](https://juejin.cn/post/6910791727670362125)
+ [汇编过程调用是怎样操作栈的？](https://www.zhihu.com/question/49410551/answer/115870825)
+ [ARM三级流水线](https://blog.csdn.net/qq_34127958/article/details/72791382)



## 任务

1. Shell/Python/JavaScript 脚本语言学习。
2. SwiftUI 和 Flutter 学习。
3. [ios-crash-dump-analysis-book/zh](https://faisalmemon.github.io/ios-crash-dump-analysis-book/zh/)
4. Mach 内核/汇编语言/逆向 三本书要读。
5. 汇编语言/函数调用栈回溯。
