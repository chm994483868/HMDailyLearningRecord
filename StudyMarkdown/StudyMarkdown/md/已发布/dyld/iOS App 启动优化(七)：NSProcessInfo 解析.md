# iOS App 启动优化(七)：NSProcessInfo 解析

&emsp;NSProcessInfo 是我们统计 APP 启动时间时必会用到的一个类，下面我们就通过官方文档对它进行学习。

/Users/hmc/Documents/GitHub/APPLE_开源代码/objc4_debug/objc4-781

















&emsp;做逆向和静态分析的时候必会看到的文件格式。

&emsp;每个进程都会被分配一个虚拟地址空间，进程寻址的范围就是在这个虚拟地址空间进行的，虚拟地址到物理地址之间有一个映射表进行管理。

&emsp;编译器或任何创建 Mach-O 文件的工具都可以定义额外的节名。这些额外的名称没有出现在表 1 中。

&emsp;在Mach-O文件中的每个section都包含类型和一组属性标记。在中间对象文件中，这个类型和属性决定了静态连接器怎么将section拷贝到最终产品中。对象文件分析工具（例如otool）用类型和属性决定怎么读取和现实这些section。有些section类型和属性是动态连接器用到的。


## Hello World 和编译器

### 预处理

+ 符号化（Tokenization）
+ 宏定义的展开
+ `#include` 的展开

### 语法和语义分析

+ 将符号化后的内容转化为一棵解析树（parse tree）
+ 解析树做语义分析
+ 输出一棵抽象语法树（Abstract Syntax Tree*(AST)）

### 生成代码和优化

+ 将 AST 转换为更低级的中间码（LLVM IR）
+ 对生成的中间码做优化
+ 生成特定目标代码
+ 输出汇编代码

### 汇编器

+ 将汇编代码转换为目标对象文件

### 链接器

+ 将多个目标对象文件合并为一个可执行文件（或者一个动态库）



任意的片段

使用链接符号 -sectcreate 我们可以给可执行文件以 section 的方式添加任意的数据。这就是如何将一个 Info.plist 文件添加到一个独立的可执行文件中的方法。Info.plist 文件中的数据需要放入到 __TEXT segment 里面的一个 __info_plist section 中。可以将 -sectcreate segname sectname file 传递给链接器（通过将下面的内容传递给 clang）：

-Wl,-sectcreate,__TEXT,__info_plist,path/to/Info.plist
同样，-sectalign 规定了对其方式。如果你添加的是一个全新的 segment，那么需要通过 -segprot 来规定 segment 的保护方式 (读/写/可执行)。这些所有内容在链接器的帮助文档中都有，例如 ld(1)。

我们可以利用定义在 /usr/include/mach-o/getsect.h 中的函数 getsectdata() 得到 section，例如 getsectdata() 可以得到指向 section 数据的一个指针，并返回相关 section 的长度。


&emsp;阅读 kyson 老师的 runtime 的专栏。

&emsp;**今天早上的任务就是把 kyson 老师的 runtime 文章全部看完。**

&emsp;






## 参考链接
**参考链接:🔗**
+ [iOS 启动优化 + 监控实践](https://juejin.cn/post/6844904194877587469)
+ [深入理解MachO数据解析规则](https://juejin.cn/post/6947843156163428383)
+ [探秘 Mach-O 文件](http://hawk0620.github.io/blog/2018/03/22/study-mach-o-file/)
+ [深入剖析Macho (1)](http://satanwoo.github.io/2017/06/13/Macho-1/)
+ [Mach-O 可执行文件](https://objccn.io/issue-6-3/)
+ [巧用nm命令](https://zhuanlan.zhihu.com/p/52984601)

[](https://github.com/zjh171/RuntimeSample)

[](https://xiaozhuanlan.com/runtime)

[](https://blog.csdn.net/jasonblog/article/details/49909209)

[](http://hawk0620.github.io/blog/2018/03/22/study-mach-o-file/)

[](https://easeapi.com/blog/blog/57-ios-dumpdecrypted.html)

[](https://blog.csdn.net/lovechris00/article/details/81561627)

[](https://juejin.cn/post/6844904194877587469)

[](https://www.jianshu.com/p/782c0eb7bc10)

[](https://www.jianshu.com/u/58e5946c7e09)

[](https://juejin.cn/post/6947843156163428383)

[](https://mp.weixin.qq.com/s/vt2LjEbgYsnU1ZI5P9atRw)

[](https://blog.csdn.net/weixin_30463341/article/details/99201551)

[](https://www.cnblogs.com/zhanggui/p/9991455.html)

[](https://objccn.io/issue-6-3/)

+ [iOS dyld详解](https://zhangyu.blog.csdn.net/article/details/92835911?utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromMachineLearnPai2%7Edefault-4.control&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromMachineLearnPai2%7Edefault-4.control)


## 模仿 static_init 调用构造函数

&emsp;在前面的 \_objc_init 过程解析中我们详细分析了 static_init 函数，已知它是

[iOS开发之runtime（11）：Mach-O 犹抱琵琶半遮面](https://xiaozhuanlan.com/topic/0328479651)


&emsp;全局搜索 \__objc_init_func 

## iOS 启动优化 + 监控实践

&emsp;但是每个版本排查启动增量会耗费不少时间,想做一个自动化的启动监控流程来降低这方面的时间成本。

+ 启动流程、
+ 如何优化、
+ push 启动优化、
+ 二进制重排、
+ 后续计划

[iOS 启动优化 + 监控实践](https://juejin.cn/post/6844904194877587469)


&emsp;NSProcessInfo 是我们统计 APP 启动时间时必会用到的一个类，下面我们就通过官方文档对它进行学习。

/Users/hmc/Documents/GitHub/APPLE_开源代码/objc4_debug/objc4-781

&emsp;做逆向和静态分析的时候必会看到的文件格式。

&emsp;每个进程都会被分配一个虚拟地址空间，进程寻址的范围就是在这个虚拟地址空间进行的，虚拟地址到物理地址之间有一个映射表进行管理。

&emsp;编译器或任何创建 Mach-O 文件的工具都可以定义额外的节名。这些额外的名称没有出现在表 1 中。

&emsp;在Mach-O文件中的每个section都包含类型和一组属性标记。在中间对象文件中，这个类型和属性决定了静态连接器怎么将section拷贝到最终产品中。对象文件分析工具（例如otool）用类型和属性决定怎么读取和现实这些section。有些section类型和属性是动态连接器用到的。

## 加载过程


&emsp;当你点击一个 icon 启动应用程序的时候，系统在内部大致做了如下几件事：

+ 内核（OS Kernel）创建一个进程，分配虚拟的进程空间等等，加载动态链接器。
+ 通过动态链接器加载主二进制程序引用的库、绑定符号。
+ 启动程序

&emsp;struct mach_header_64 这个结构体代表的都是 Mach-O 文件的一些元信息，它的作用是让内核在读取该文件创建虚拟进程空间的时候，检查文件的合法性以及当前硬件的特性是否能支持程序的运行。
