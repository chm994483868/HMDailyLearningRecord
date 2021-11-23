# iOS App Crash 学习：(二)：Mach 异常和 Signal 信号分析

&emsp;Objective-C 的异常处理是指通过 `@try` `@catch`（捕获） 或 `NSSetUncaughtExceptionHandler`（记录） 函数来捕获或记录异常，但是这种处理方式对内存访问错误、重复释放等错误引起的 crash 是无能为力的（如野指针访问、MRC 下重复 release 等）。

⬇️⬇️⬇️⬇️⬇️⬇️ 这里需要注意一下： （如野指针访问、MRC 下重复 release 等） 这里是单纯的 Mach 异常（正常情况下都会转化为 signal 信号，但是比如收集到 mach 异常后，直接调用了 `exit()` 函数就会导致程序终止而没有产生对应的 signal 信号），还是 Mach 异常后会发送 signal 信号，后面要验证一下：

这种错误抛出的是 `signal`，所以需要专门做 `signal` 处理）不能得到 signal，如果要处理 signal 需要利用 unix 标准的 signal 机制，注册 `SIGABRT`、`SIGBUS`、`SIGSEGV` 等 signal 发生时的处理函数。

&emsp;例如我们编写如下代码，然后直接运行，程序会直接 crash 中止运行，然后 `NSLog(@"✳️✳️✳️ objc: %@", objc);` 行显示红色的错误信息：`Thread 1: EXC_BAD_ACCESS (code=1, address=0x3402e8d4c25c)` (objc 对象已经被释放，然后 NSLog 语句中又去访问 objc 已经被释放的内存) 指出我们的程序此时有一个 `EXC_BAD_ACCESS` 异常，导致退出，且此时可发现我们通过 `NSSetUncaughtExceptionHandler` 设置的 **未捕获异常处理函数** 在程序中止之前并没有得到执行！ 

```c++
__unsafe_unretained NSObject *objc = [[NSObject alloc] init];
NSLog(@"✳️✳️✳️ objc: %@", objc);
```

&emsp;在测试除零操作时（我们都知道 0 不能做除数😂）如下示例代码，发现运行结果与 Build Settings 的 Optimization Level 的选项值有关系，当我们选择 None[-O0] 时会 crash，报出：`Thread 1: EXC_ARITHMETIC (code=EXC_I386_DIV, subcode=0x0)` 错误，而在其他任意 Fast[-O, O1]、Faster[-O2]、Fastest[-O3]、Fastest,Smallest[-Os]、Fastest,Aggressive Optimizations[-Ofast]、Smallest,Aggressive Size Optimizations[-Oz] 选项下程序都正常运行没有 crash 退出，且每次运行 result 的值都是一个很大的随机数。

```c++
int a = 0;
int b = 1;
int result = b / a;
NSLog(@"🏵🏵🏵 %d", result);
```

&emsp;针对上述两段代码导致的 crash，我们在程序退出后在 xcode 底部的调试控制台输入 bt 指令并回车，可看到程序停止运行的原因分别如下：`EXC_ARITHMETIC`、`EXC_BAD_ACCESS`、`signal SIGABRT`。 这里还有一个小细节，就是程序退出大概是某条线程的退出：主线程或子线程，当我们把上述代码放在一条子线程的执行的话，便会看到子线程的停止原因。

&emsp;除零操作：

```c++
// 当前在主线程
(lldb) bt
* thread #1, queue = 'com.apple.main-thread', stop reason = EXC_ARITHMETIC (code=EXC_I386_DIV, subcode=0x0)
  * frame #0: 0x0000000102d06daf dSYMDemo`-[AppDelegate application:didFinishLaunchingWithOptions:](self=0x0000600002bb82c0, _cmd="application:didFinishLaunchingWithOptions:", application=0x00007fe1dbc06f50, launchOptions=0x0000000000000000) at AppDelegate.m:59:20
  ...
  
// 当前在子线程
(lldb) bt all
  thread #1, queue = 'com.apple.main-thread'
    frame #0: 0x00007fff203b69a4 CoreFoundation`__CFStringHash + 151
    ...
    
* thread #8, queue = 'com.apple.root.default-qos', stop reason = EXC_ARITHMETIC (code=EXC_I386_DIV, subcode=0x0)
  * frame #0: 0x00000001029a0daf dSYMDemo`__57-[AppDelegate application:didFinishLaunchingWithOptions:]_block_invoke(.block_descriptor=0x00000001029a6048) at AppDelegate.m:60:24
  ...
```

&emsp;野指针访问：

```c++
// 当前在主线程
(lldb) bt
* thread #1, queue = 'com.apple.main-thread', stop reason = EXC_BAD_ACCESS (code=1, address=0x539e0d66c11c)
    frame #0: 0x00007fff20190d18 libobjc.A.dylib`objc_opt_respondsToSelector + 16
    ...

// 当前在子线程
thread #1, queue = 'com.apple.main-thread'
  frame #0: 0x00007fff2468f57e UIKitCore`+[_UIBackgroundTaskInfo backgroundTaskAssertionQueue]
  ...
  
* thread #4, queue = 'com.apple.root.default-qos', stop reason = EXC_BAD_ACCESS (code=1, address=0xbc637211426c)
    frame #0: 0x00007fff20190d18 libobjc.A.dylib`objc_opt_respondsToSelector + 16
    ...
```

&emsp;附加一个数组越界的 crash 作为对比（它是 `abort` 函数调用 `pthread_kill` 发送了一个 `SIGABRT` 信号后程序中止）：

```c++
// 当前在主线程
(lldb) bt 
* thread #1, queue = 'com.apple.main-thread', stop reason = signal SIGABRT
    frame #0: 0x00007fff61131462 libsystem_kernel.dylib`__pthread_kill + 10
    frame #1: 0x00007fff6116a610 libsystem_pthread.dylib`pthread_kill + 263
    frame #2: 0x00007fff200fab94 libsystem_c.dylib`abort + 120
    ...

// 当前在子线程
(lldb) bt all
  thread #1, queue = 'com.apple.main-thread'
    frame #0: 0x00007fff204318a9 CoreFoundation`-[NSSet anyObject]
    ...

* thread #5, queue = 'com.apple.root.default-qos', stop reason = signal SIGABRT
  * frame #0: 0x00007fff61131462 libsystem_kernel.dylib`__pthread_kill + 10
    frame #1: 0x00007fff6116a610 libsystem_pthread.dylib`pthread_kill + 263
    frame #2: 0x00007fff200fab94 libsystem_c.dylib`abort + 120
    ...
```

&emsp;Objective-C 的异常如果不做任何处理的话（try catch 捕获处理），最终便会触发程序中止退出，此时造成退出的原因是程序向自身发送了 `SIGABRT` 信号。（对于未捕获的 Objective-C 异常，我们可以通过 `NSSetUncaughtExceptionHandler` 函数设置 **未捕获异常处理函数** 在其中记录存储异常日志，然后在 APP 下次启动时进行上传（**未捕获异常处理函数** 函数执行完毕后，程序也同样会被终止，此时没有机会给我们进行网络请求上传数据），如果异常日志记录得当，然后再配合一些异常发生时用户的操作行为数据，那么可以分析和解决大部分的崩溃问题。）

&emsp;上篇我们已经分析过 Objective-C 的异常捕获处理，下面我们开始详细学习 mach 异常和 signal 处理。

```c++
#import "AppDelegate.h"
#import <execinfo.h>

void mySignalHandler(int signal) {
    NSMutableString *mstr = [[NSMutableString alloc] init];
    [mstr appendString:@"Stack:\n"];
    
    void *callstack[128];
    int frames = backtrace(callstack, 128);
    char ** strs = backtrace_symbols(callstack, frames);
    
    printf("🏵🏵🏵 char ** 怎么打印：%p", strs);
    
//    kill(<#pid_t#>, <#int#>)
    
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    
    // 信号量截断
    signal(SIGABRT, mySignalHandler);
    signal(SIGILL, mySignalHandler);
    signal(SIGSEGV, mySignalHandler);
    signal(SIGFPE, mySignalHandler);
    signal(SIGBUS, mySignalHandler);
    signal(SIGPIPE, mySignalHandler);
    
    return YES;
}
```

&emsp;SignalHandler 不要在 debug 环境下测试。因为系统的 debug 会优先去拦截。我们要运行一次后，关闭 debug 状态。应该直接在模拟器上点击我们 build 上去的 App  去运行。而 UncaughtExceptionHandler 可以在调试状态下捕捉。

&emsp;学习 Crash 捕获相关的 **Mach 异常** 和 **signal 信号处理**。

> &emsp;**Mach 为 XNU 的微内核，Mach 异常为最底层的内核级异常。在 iOS 系统中，底层 Crash 先触发 Mach 异常，然后再转换为对应的 Signal 信号**。

&emsp;Darwin 是 macOS 和 iOS 的操作系统，而 XNU 是 Darwin 操作系统的内核部分。XNU 是混合内核，兼具宏内核和微内核的特性，而 Mach 即为其微内核。Mac 可执行 `system_profiler SPSoftwareDataType` 命令查看当前设备的 Darwin 版本号。

![截屏2021-11-20 09.09.20.png](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/eec1dc7e8fe34a82979793b2f6e2463b~tplv-k3u1fbpfcp-watermark.image?)

![截屏2021-11-21 上午8.47.18.png](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/146836b6c7d04a4da5d316b085dafafa~tplv-k3u1fbpfcp-watermark.image?)


### Mach 异常 

######### 总结 Mach 知识点：⬇️



















## 参考链接
**参考链接:🔗**
+ [Mach-维基百科](https://zh.wikipedia.org/wiki/Mach)
+ [iOS 异常信号思考](https://minosjy.com/2021/04/10/00/377/)
+ [Linux 多线程环境下 进程线程终止函数小结](https://www.cnblogs.com/biyeymyhjob/archive/2012/10/11/2720377.html)
+ [pthread_kill引发的争论](https://www.jianshu.com/p/756240e837dd)
+ [线程的信号pthread_kill()函数（线程四）](https://blog.csdn.net/littesss/article/details/71156793)
+ [原子操作atomic_fetch_add](https://www.jianshu.com/p/985fb2e9c201)
+ [iOS Crash 分析攻略](https://zhuanlan.zhihu.com/p/159301707)


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


