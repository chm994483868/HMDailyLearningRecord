# iOS APP 启动优化(六)：mach-o 文件使用场景总结  

&emsp;日常灵魂拷问，学习底层到底有没有用，很多人认为学习底层知识只是为了应付面试，日常开发中根本使用不到，事实真的是这样吗？其实我觉得那些对你学习底层进行 “冷嘲热讽” 的人，仅仅是因为他们不想学习或者他们遇到困难 就退缩学不会，而打击你来寻找存在感罢了，今天我们就总结一些 mach-o 的知识在日常开发中的一些使用场景。来验证一下我们学习底层知识点到底有没有用！

&emsp;设置 section 的数据的意义是什么呢？

## 模仿 static_init 调用构造函数

&emsp;在前面的 \_objc_init 过程解析中我们详细分析了 static_init 函数，已知它是

[iOS开发之runtime（11）：Mach-O 犹抱琵琶半遮面](https://xiaozhuanlan.com/topic/0328479651)


&emsp;全局搜索 \__objc_init_func 






## 参考链接
**参考链接:🔗**
+ [iOS dyld加载流程](https://www.jianshu.com/p/bda67b2a3465)
+ [dyld和ObjC的关联](https://www.jianshu.com/p/3cad4212892a)
+ [OC底层原理之-类的加载过程-上（ _objc_init实现原理）](https://juejin.cn/post/6883118074426294285)
+ [线程本地存储TLS(Thread Local Storage)的原理和实现——分类和原理](https://www.cnblogs.com/zhoug2020/p/6497709.html)
+ [imp_implementationWithBlock()的实现机制](https://www.jianshu.com/p/c52bc284e9c7)
+ [iOS 底层原理之—dyld 与 objc 的关联](https://www.yuque.com/ioser/spb08a/alu6tz)
+ [dyld-832.7.3](https://opensource.apple.com/tarballs/dyld/)
+ [OC底层原理之-App启动过程（dyld加载流程）](https://juejin.cn/post/6876773824491159565)
+ [iOS中的dyld缓存是什么？](https://blog.csdn.net/gaoyuqiang30/article/details/52536168)
+ [iOS进阶之底层原理-应用程序加载（dyld加载流程、类与分类的加载）](https://blog.csdn.net/hengsf123456/article/details/116205004?utm_medium=distribute.pc_relevant.none-task-blog-baidujs_title-4&spm=1001.2101.3001.4242)
+ [iOS应用程序在进入main函数前做了什么？](https://www.jianshu.com/p/73d63220d4f1)
+ [dyld加载应用启动原理详解](https://www.jianshu.com/p/1b9ca38b8b9f)
+ [iOS里的动态库和静态库](https://www.jianshu.com/p/42891fb90304)
+ [Xcode 中的链接路径问题](https://www.jianshu.com/p/cd614e080078)
+ [iOS 利用 Framework 进行动态更新](https://nixwang.com/2015/11/09/ios-dynamic-update/)
+ [命名空间namespace ，以及重复定义的问题解析](https://blog.csdn.net/u014357799/article/details/79121340)
+ [C++ 命名空间namespace](https://www.jianshu.com/p/30e960717ef1)
+ [一文了解 Xcode 生成「静态库」和「动态库」 的流程](https://mp.weixin.qq.com/s/WH8emrMpLeVW-LfGwN09cw)
+ [Hook static initializers](https://blog.csdn.net/majiakun1/article/details/99413403)

**之所以没有进alloc而是进了objc_alloc，查资料说的是在编译期的时候如果符号绑定失败了就会触发一个这样的修复操作，调用fixupMessageRef方法，明显的能看到 if (msg->sel == SEL_alloc) , msg->imp = (IMP)&objc_alloc，将alloc方法和objc_alloc方法进行交换。**


