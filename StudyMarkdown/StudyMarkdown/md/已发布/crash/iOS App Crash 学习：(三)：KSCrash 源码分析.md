# iOS App Crash 学习：(三)：KSCrash 源码分析

&emsp;[kstenerud/KSCrash](https://github.com/kstenerud/KSCrash) The Ultimate Crash Reporter! 

## KSCrash 简述

&emsp;以下内容来自 KSCrash 首页描述。

&emsp;虽然现有的 crash reporters 确实报告了崩溃，但是他们可以做的还有很多。

&emsp;以下是 KSCrash 的一些主要功能：

+ 以支持离线重新符号化的方式在设备上进行符号化（对于许多功能已被编辑的 iOS 版本是必需的）。
+ 生成完整的 Apple 报告（KSCrashEmailReportStyleApple、KSCrashEmailReportStyleJSON），并填写每个字段。
+ 支持 32 位和 64 位模式。
+ 支持所有苹果设备，包括 Apple Watch。
+ 处理只能在 mach level 捕获的错误，例如堆栈溢出（stack overflow 只能在 mach level 捕获到，它不同于其他异常，它不会转化为对应的 Unix Signal（这里还不确定会不会转化），即使能转化，但是常规情况下 Unix signals 要在崩溃线程执行回调，此时由于堆栈溢出已经没有条件（堆栈空间）执行回调代码了）。
+ 跟踪未捕获的 C++ 异常的真正原因。


+ 处理崩溃处理程序本身（或用户崩溃处理程序回调）中的崩溃。







## KSCrash 使用过程

&emsp;在 `application:didFinishLaunchingWithOptions:` 函数中安装 **崩溃处理程序**。

```c++
- (BOOL) application:(__unused UIApplication *) application didFinishLaunchingWithOptions:(__unused NSDictionary *) launchOptions {

    // App 启动后开始安装 崩溃处理程序
    [self installCrashHandler];
    
    return YES;
}
```

&emsp;KSCrash 框架根据不同的发送日志的方式提供几种不同的 installation，它们都是 KSCrashInstallation 的子类，如下:

+ KSCrashInstallationStandard
+ KSCrashInstallationEmail
+ KSCrashInstallationHockey
+ KSCrashInstallationQuincy
+ KSCrashInstallationVictory

&emsp;我们只能选择其中一个 installation 使用。这里我们仅以标准 installation（KSCrashInstallationStandard）作为学习的主线。  

&emsp;`installCrashHandler` 函数第一行便是调用 `makeStandardInstallation` 函数，取得 KSCrashInstallationStandard 类的单例对象 `installation`，并为它的 url 属性赋值，此 url 会用在 App 启动时如果本地有崩溃 log 的话，会上传到此 url。

&emsp;接下来 KSCrashInstallationStandard 类的单例对象 `installation` 调用其 `install` 函数，此函数继承自父类 KSCrashInstallation，KSCrashInstallationStandard 作为子类并没有重写 `install` 函数，此函数的作用是安装 **崩溃处理程序**，即取得 KSCrash 类的单例对象。



















此操作应该尽早的完成，它会记录所有出现的崩溃，但是它并不会自动的去上传崩溃记录。

&emsp;这里指的 **崩溃处理程序** 是 KSCrash 类的单例对象。

```c++
- (void) installCrashHandler {
    // Create an installation (choose one)
    
    KSCrashInstallation* installation = [self makeStandardInstallation];
    
//    KSCrashInstallation* installation = [self makeEmailInstallation];
//    KSCrashInstallation* installation = [self makeHockeyInstallation];
//    KSCrashInstallation* installation = [self makeQuincyInstallation];
//    KSCrashInstallation *installation = [self makeVictoryInstallation];
    
    // Install the crash handler. This should be done as early as possible.
    // This will record any crashes that occur, but it doesn't automatically send them.
    [installation install];
    
    [KSCrash sharedInstance].deleteBehaviorAfterSendAll = KSCDeleteNever; // TODO: Remove this


    // Send all outstanding reports. You can do this any time; it doesn't need to happen right as the app launches.
    // Advanced-Example shows how to defer displaying the main view controller until crash reporting completes.
    [installation sendAllReportsWithCompletion:^(NSArray* reports, BOOL completed, NSError* error) {
        if (completed) {
            NSLog(@"🐹🐹🐹 Sent %d reports", (int)[reports count]);
        } else {
            NSLog(@"🐹🐹🐹 Failed to send reports: %@", error);
        }
    }];
}

- (KSCrashInstallation*) makeStandardInstallation {
    NSURL* url = [NSURL URLWithString:@"http://put.your.url.here"];
    
    KSCrashInstallationStandard* standard = [KSCrashInstallationStandard sharedInstance];
    standard.url = url;

    return standard;
}
```




+ [kstenerud/KSCrash](https://github.com/kstenerud/KSCrash)
+ [KSCrash源码分析](https://cloud.tencent.com/developer/article/1370201)
+ [iOS中Crash采集及PLCrashReporter使用](https://www.jianshu.com/p/930d7f77df6c)
+ [iOS KSCrash的使用](https://www.jianshu.com/p/d9ec5f3f144e)
+ [使用KSCrash进行崩溃日志的采集](https://www.jianshu.com/p/7847b7aaef0b)
+ [KSCrash翻译](https://www.jianshu.com/p/95102419c29b)
+ [iOS崩溃日志使用KSCrash收集、分析](https://www.jianshu.com/p/329684cf1e51)
+ [iOS开源库分析之KSCrash](https://xiaozhuanlan.com/topic/7193860452)
+ [KSCrash源码阅读（Monitors）](https://blog.csdn.net/qq_22389025/article/details/84784796)
+ [KSCrash崩溃收集原理浅析](https://www.it610.com/article/1191455498289913856.htm)
+ [KSCrash源码学习](https://www.jianshu.com/p/8c2dc3ce8545)
+ [KSCrash+Symbolicatecrash日志分析](https://www.jianshu.com/p/d88b39acea7d)

















BSBacktraceLogger 源码
KSCrash 源码
PLCrashReporter 源码
GYBootingProtection 源码

+ [iOS Crash/崩溃/异常 堆栈获取](https://www.jianshu.com/p/8ece78d71b3d)
+ [iOS中Crash采集及PLCrashReporter使用](https://www.jianshu.com/p/930d7f77df6c)
+ [iOS 启动连续闪退保护方案](https://blog.csdn.net/jiang314/article/details/52574307?utm_medium=distribute.pc_relevant.none-task-blog-2~default~baidujs_title~default-0.highlightwordscore&spm=1001.2101.3001.4242.1)
+ [iOS/OSX Crash：捕捉异常](https://zhuanlan.zhihu.com/p/271282052)





## 参考链接
**参考链接:🔗**
+ [iOS App 连续闪退时如何上报 crash 日志](https://zhuanlan.zhihu.com/p/35436876)













+ [Mach-维基百科](https://zh.wikipedia.org/wiki/Mach)
+ [iOS 异常信号思考](https://minosjy.com/2021/04/10/00/377/)
+ [Linux 多线程环境下 进程线程终止函数小结](https://www.cnblogs.com/biyeymyhjob/archive/2012/10/11/2720377.html)
+ [pthread_kill引发的争论](https://www.jianshu.com/p/756240e837dd)
+ [线程的信号pthread_kill()函数（线程四）](https://blog.csdn.net/littesss/article/details/71156793)
+ [原子操作atomic_fetch_add](https://www.jianshu.com/p/985fb2e9c201)
+ [iOS Crash 分析攻略](https://zhuanlan.zhihu.com/p/159301707)
+ [Handling unhandled exceptions and signals](https://www.cocoawithlove.com/2010/05/handling-unhandled-exceptions-and.html)
+ [Apple 源码文件下载列表](https://opensource.apple.com/tarballs/)
+ [iOS @try @catch异常机制](https://www.jianshu.com/p/f28b9b3f8e44)
+ [一文读懂崩溃原理](https://juejin.cn/post/6873868181635760142)
+ [软件测试之SDK开发(ios)——Mach捕获](https://blog.csdn.net/lfdanding/article/details/100024022)
+ [[史上最全] iOS Crash/崩溃/异常 捕获](https://www.jianshu.com/p/3f6775c02257)
+ [iOS Crash/崩溃/异常 堆栈获取](https://www.jianshu.com/p/8ece78d71b3d)
+ [KSCrash源码分析](https://cloud.tencent.com/developer/article/1370201)
+ [iOS线程通信和进程通信的例子（NSMachPort和NSTask，NSPipe）](https://blog.csdn.net/yxh265/article/details/51483822)
+ [iOS开发·RunLoop源码与用法完全解析(输入源，定时源，观察者，线程间通信，端口间通信，NSPort，NSMessagePort，NSMachPort，NSPortMessage)](https://sg.jianshu.io/p/07313bc6fd24)
+ [Delivering Notifications To Particular Threads](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/Notifications/Articles/Threading.html#//apple_ref/doc/uid/20001289-CEGJFDFG)
+ [iOS开发之线程间的MachPort通信与子线程中的Notification转发](https://cloud.tencent.com/developer/article/1018076)
+ [移动端监控体系之技术原理剖析](https://www.jianshu.com/p/8123fc17fe0e)
+ [iOS性能优化实践：头条抖音如何实现OOM崩溃率下降50%+](https://mp.weixin.qq.com/s?__biz=MzI1MzYzMjE0MQ==&mid=2247486858&idx=1&sn=ec5964b0248b3526836712b26ef1b077&chksm=e9d0c668dea74f7e1e16cd5d65d1436c28c18e80e32bbf9703771bd4e0563f64723294ba1324&cur_album_id=1590407423234719749&scene=189#wechat_redirect)
+ [iOS Crash之NSInvalidArgumentException](https://blog.csdn.net/skylin19840101/article/details/51941540)
+ [iOS调用reloadRowsAtIndexPaths Crash报异常NSInternalInconsistencyException](https://blog.csdn.net/sinat_27310637/article/details/62225658)
+ [iOS开发质量的那些事](https://zhuanlan.zhihu.com/p/21773994)
+ [NSException抛出异常&NSError简单介绍](https://www.jianshu.com/p/23913bbc4ee5)
+ [NSException:错误处理机制---调试中以及上架后的产品如何收集错误日志](https://blog.csdn.net/lcl130/article/details/41891273)
+ [Exception Programming Topics](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/Exceptions/Exceptions.html#//apple_ref/doc/uid/10000012-BAJGFBFB)
+ [iOS被开发者遗忘在角落的NSException-其实它很强大](https://www.jianshu.com/p/05aad21e319e)
+ [iOS runtime实用篇--和常见崩溃say good-bye！](https://www.jianshu.com/p/5d625f86bd02)
+ [异常处理NSException的使用（思维篇）](https://www.cnblogs.com/cchHers/p/15116833.html)
+ [异常统计- IOS 收集崩溃信息 NSEXCEPTION类](https://www.freesion.com/article/939519506/)
+ [NSException异常处理](https://www.cnblogs.com/fuland/p/3668004.html)
+ [iOS Crash之NSGenericException](https://blog.csdn.net/skylin19840101/article/details/51945558)
+ [iOS异常处理](https://www.jianshu.com/p/1e4d5421d29c)
+ [iOS异常处理](https://www.jianshu.com/p/59927211b745)
+ [iOS crash分类,Mach异常、Unix 信号和NSException 异常](https://blog.csdn.net/u014600626/article/details/119517507?spm=1001.2101.3001.6661.1&utm_medium=distribute.pc_relevant_t0.none-task-blog-2%7Edefault%7ECTRLIST%7Edefault-1.no_search_link&depth_1-utm_source=distribute.pc_relevant_t0.none-task-blog-2%7Edefault%7ECTRLIST%7Edefault-1.no_search_link)
+ [iOS Mach异常和signal信号](https://developer.aliyun.com/article/499180)



