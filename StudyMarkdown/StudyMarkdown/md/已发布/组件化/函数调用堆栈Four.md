# 函数调用堆栈

&emsp;在学习线程的相关知识时，我们一定接触过 NSThread 类，它有一个类属性：`@property (class, readonly, copy) NSArray<NSString *> *callStackSymbols` 用来获取当前线程的函数调用堆栈，该属性返回包含调用堆栈符号的数组，每个元素都是一个 NSString 对象，其值的格式由 backtrace_symbols() 函数确定。初看此属性给人眼前一亮，但是看到它仅是一个类属性时，我们仿佛意识到了什么，对，它有一个限制，此类属性返回值仅能描述调用此方法时当前线程的调用堆栈回溯。我们常见的在一个子线程中抓取主线程的函数调用堆栈的场景它就无法胜任了。例如：进行性能监控时通常会开一个子线程监控，如监控主线程的卡顿情况、CPU Usage 情况，当出现主线程卡顿、某个线程 CPU Usage 占用过高时，需要进行抓栈，那么就需要通过其它方式获取非当前线程的调用栈了。

> &emsp;在 Developer Documentation 中搜索 callStackSymbols 时，发现 NSException 类中也有一个同名的实例属性：`@property(readonly, copy) NSArray<NSString *> *callStackSymbols;` 此实例属性描述引发 Objective-C 异常时调用堆栈回溯的字符串数组。每个字符串的格式由 backtrace_symbols() 确定，感兴趣的小伙伴可以尝试在一个 Try-Catch 中制造一个 Objective-C 异常，打印一下 callStackSymbols 属性的内容试一下。

&emsp;继续开始之前我们需要补充一些 macOS 三种线程的关系：

1. `pthread_t pthread_self(void)` 返回的是 `pthread_t`，glibc 库的线程 id。实际上是线程控制块 tcb 首地址。(pthread_self 是 POSIX 标准中的接口，pthread_t 是 POSIX 标准中线程的类型) 
2. `syscall(SYS_gettid)` 内核级线程 id，系统唯一。该函数为系统调用函数，glibc 可能没有该函数声明，此时需要使用 syscall(SYS_gettid)。可以参考 [pthread_self() VS syscall(SYS_gettid)](https://www.cnblogs.com/alix-1988/p/14886847.html) 详细了解它们之间的关系。（此种线程不涉及本篇的知识点，可直接忽略）
3. 此种正是 macOS 中 Mach 内核中的线程（在 mach 中或者我们把线程理解为一个个端口，获取某条线程就是获取此线程的端口）：`mach_port_t mach_thread_self(void)` 直接获取线程端口 mach_port_t，另外一种是：首先 `pthread_t pthread_self(void)` 获取 POSIX 标准线程：`pthread_t`，然后通过 `mach_port_t pthread_mach_thread_np(pthread_t)` 把它转换为 Mach 线程（端口）：`mach_port_t`。`mach_port_t` 是 macOS 特有的 id，实际上不能说是 thread id，而应该当做是线程端口，它是 Mach 中表示线程的一种方式。
  
&emsp;上面的第 3 条获取 Mach 线程的方式中，其中涉及一些内存方面的问题，看到这个链接 [一个“反问”引发的内存反思](https://blog.csdn.net/killer1989/article/details/106674973) 我们可以详细阅读一下，增强一下对 `extern mach_port_t mach_thread_self(void);` 和 `mach_port_t pthread_mach_thread_np(pthread_t);` 函数的认识，其中 np 是 not posix 的首字母缩写。

```c++
/* return the mach thread bound to the pthread 返回绑定到 pthread 的 mach 线程 */
__API_AVAILABLE(macos(10.4), ios(2.0))
mach_port_t pthread_mach_thread_np(pthread_t);
```

&emsp;针对上述文章中提到的优化点，我们大概可以通过以下两种方式获取 Mach 线程（port）：

```c++
// 方式 1
mach_port_t thread_self(void) {
    // mach_thread_self 和 mach_port_deallocate 配对使用，使用完毕后立刻释放空间 
    mach_port_t thread_self = mach_thread_self();
    mach_port_deallocate(mach_task_self(), thread_self);
    return thread_self;
}

// 方式 2
mach_port_t thread_self(void) {
    return pthread_mach_thread_np(pthread_self());
}
```

&emsp;FP(x29) 栈底 SP 栈顶 PC 下一条指令 LR(x30) 函数返回后的下一个函数的第一条指令

@emsp;NSException:

```c++
NSException

Instance Property

callStackSymbols

An array containing the current call stack symbols.

@property(readonly, copy) NSArray<NSString *> *callStackSymbols;

Discussion
An array of strings describing the call stack backtrace at the moment the exception was first raised. The format of each string is determined by the backtrace_symbols() API
```

```c++
NSThread

Type Property

callStackSymbols

Returns an array containing the call stack symbols.

@property(class, readonly, copy) NSArray<NSString *> *callStackSymbols;

Return Value
An array containing the call stack symbols. Each element is an NSString object with a value in a format determined by the backtrace_symbols() function. For more information, see backtrace_symbols(3) macOS Developer Tools Manual Page.

Discussion
The return value describes the call stack backtrace of the current thread at the moment this method was called.
```

## 参考链接
**参考链接:🔗**
+ [iOS开发--探究iOS线程调用栈及符号化](https://blog.csdn.net/qq_36237037/article/details/107473867)
+ [iOS内存扫描工具实现](https://blog.csdn.net/liumazi/article/details/106417276)
+ [获取任意线程的调用栈](https://zhuanlan.zhihu.com/p/357039492)








+ [iOS中符号的那些事儿](https://juejin.cn/post/6844904164208689166)

+ [bestswifter/BSBacktraceLogger](https://github.com/bestswifter/BSBacktraceLogger)
+ [iOS——CPU监控](https://blog.csdn.net/cym_bj/article/details/109677752)
+ [深入解析Mac OS X & iOS 操作系统 学习笔记（十二）](https://www.jianshu.com/p/cc655bfdac13)
+ [一个“反问”引发的内存反思](https://blog.csdn.net/killer1989/article/details/106674973)
+ [BSBackTracelogger学习笔记](https://juejin.cn/post/6910791727670362125#heading-6)
+ [Swift堆栈信息获取](https://juejin.cn/post/6979138204154724382)
+ [MAC OS 的 mach_port_t 和 pthread_self()](https://blog.csdn.net/yxccc_914/article/details/79854603)
+ [iOS性能监控](https://www.jianshu.com/p/f04a1447aaa0?utm_campaign=shakespeare)
+ [syscall(SyS_gettid)是什么](https://blog.csdn.net/woainilixuhao/article/details/100144159)
+ [pthread_self() VS syscall(SYS_gettid)](https://www.cnblogs.com/alix-1988/p/14886847.html)



