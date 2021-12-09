# iOS App Crash 学习：(三)：KSCrash 源码分析













## 参考链接
**参考链接:🔗**
+ [iOS App 连续闪退时如何上报 crash 日志](https://zhuanlan.zhihu.com/p/35436876)
+ 













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



