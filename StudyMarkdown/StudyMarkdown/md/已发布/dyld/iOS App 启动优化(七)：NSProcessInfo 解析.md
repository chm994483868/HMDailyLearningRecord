# iOS App 启动优化(七)：NSProcessInfo 解析

&emsp;NSProcessInfo 是我们统计 APP 启动时间时必会用到的一个类，下面我们就通过官方文档对它进行学习。

## NSProcessInfo

&emsp;NSProcessInfo 就是系统进程信息对象，其中包含一些方法，允许你设置或检索正在运行的应用程序（即进程）的各种类型的信息。

&emsp;A collection of information about the current process.（关于当前进程的一个信息集合）

### Overview

&emsp;每个进程都有一个共享的 `NSProcessInfo` 对象，称为进程信息代理（`NSProcessInfo *info = [NSProcessInfo processInfo];`）。

&emsp;进程信息代理可以返回参数（`arguments`）、环境变量（`environment `）、主机名（`hostName`）和进程名（`name`）等信息。`processInfo` 类方法返回当前进程的共享代理，即其对象发送消息的进程。例如，以下行返回 `NSProcessInfo` 对象，然后提供当前进程的名称：

```c++
NSString *processName = [[NSProcessInfo processInfo] processName];

// 打印：（当前项目的名字）
🤯🤯🤯 Test_ipa_simple
```

> Note
> `NSProcessInfo` 在 `macOS 10.7` 及更高版本中是线程安全的。（看到这里我们应该会意识到，`NSProcessInfo` 对象的属性和方法并不都是只读的，还有一些我们可以对其进行设置。在我们项目里面的话在任何地方都可以通过 `[NSProcessInfo processInfo]` 来取得当前进程信息对象。）

&emsp;`NSProcessInfo` 类还包括 `operatingSystem` 方法，该方法返回一个枚举常量，标识在其上执行进程的操作系统。

&emsp;如果 `NSProcessInfo` 对象无法将环境变量（environment variables）和命令行参数（command-line arguments）转换为 `Unicode` 作为 `UTF-8` 字符串，则它们会尝试解释用户默认 C 字符串编码中的环境变量和命令行参数。如果 Unicode 和 C 字符串转换都不起作用，`NSProcessInfo` 对象将忽略这些值。（这里描述的应该是使用 `-operatingSystemVersion` 或 `-isOperatingSystemAtLeastVersion:` 代替 `operatingSystem`）

### Managing Activities

&emsp;为了用户的利益，系统具有启发式以提高应用程序的电池寿命、性能和响应能力。你可以使用以下方法来管理或向系统提示你的应用程序有特殊要求的活动（Activities）：

+ `beginActivityWithOptions:reason:`
+ `endActivity:`
+ `performActivityWithOptions:reason:usingBlock:`

&emsp;作为对创建活动（activity）的响应，系统将禁用部分或全部启发式方法，以便你的应用程序可以快速完成，同时在用户需要时仍提供响应行为。

&emsp;当你的应用程序执行长时间运行的操作时，你可以使用活动（activities）。如果活动（activity ）可能需要不同的时间（例如，计算国际象棋游戏中的下一步），则应使用此 API。这将确保在数据量或用户计算机功能发生变化时的正确行为。你应该将你的活动归入两个主要类别之一：

1. 用户发起的：这些是​​用户明确开始的有限长度的活动。例如：导出或下载用户指定的文件。
2. 后台任务：这些是有限长度的活动，它们是应用程序正常操作的一部分，但不是由用户明确启动的。例如：自动保存、索引和自动下载文件。

&emsp;此外，如果你的应用程序需要高优先级 I/O，你可以包含 `NSActivityLatencyCritical` 标志（使用按位 OR）。你应该只将此标志用于确实需要高优先级的音频或视频录制等活动。

&emsp;如果你的活动在主线程上的事件回调中同步发生，则不需要使用此 API。

&emsp;请注意，长时间未能结束这些活动可能会对用户计算机的性能产生重大负面影响，因此请确保仅使用所需的最短时间。用户偏好（preferences）可能会覆盖你的的应用程序的请求。

&emsp;你还可以使用此 API 来控制自动终止（automatic termination）或突然终止（sudden termination）（请参阅 Sudden Termination）。例如：

```c++
id activity = [[NSProcessInfo processInfo] beginActivityWithOptions:NSActivityAutomaticTerminationDisabled reason:@"Good Reason"];

// Perform some work

[[NSProcessInfo processInfo] endActivity:activity];
```

相当于:

```c++
[[NSProcessInfo processInfo] disableAutomaticTermination:@"Good Reason"];

// Perform some work

[[NSProcessInfo processInfo] enableAutomaticTermination:@"Good Reason"];
```

&emsp;由于此 API 返回一个对象，因此与使用自动终止 API 相比，将开始和结束配对可能更容易——如果在 `endActivity:` 调用之前释放对象，则活动将自动结束。

&emsp;该 API 还提供了一种机制来禁用系统范围的空闲睡眠（system-wide idle sleep）和显示空闲睡眠（display idle sleep）。这些会对用户体验产生很大的影响，所以一定不要忘记结束禁用睡眠的活动（包括 `NSActivityUserInitiated`）。

### Sudden Termination








&emsp;

```c++
@interface NSProcessInfo : NSObject {
@private
    NSDictionary    *environment;
    NSArray        *arguments;
    NSString        *hostName;
    NSString        *name;
    NSInteger        automaticTerminationOptOutCounter;
}
```

+  @property (readonly, copy) NSArray<NSString *> *arguments; 传入 main 函数中的参数 (可在 Edit Scheme... -> Run -> Arguments -> Arguments Passed On Launch 中添加变量: `{"name":"iOS","arme":"参数"}` )
+ @property (readonly, copy) NSString *hostName; 域名
+ @property (copy) NSString *processName; 进程名称
+ @property (readonly) int processIdentifier; 进程 ID
+ @property (readonly, copy) NSString *globallyUniqueString; 进程全球唯一编号
+ @property (readonly, copy) NSString *operatingSystemVersionString; @property (readonly) NSOperatingSystemVersion operatingSystemVersion API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0)); 系统版本号
+ @property (readonly) NSTimeInterval systemUptime API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0)); 时间段：设备上一次开机至今时间段 



### NSProcessInfo + NSProcessInfoPowerState

&emsp;`NSProcessInfo` 的 `NSProcessInfoPowerState` 分类仅有一个 `lowPowerModeEnabled` 属性。

&emsp;检索系统当前是否设置了低功耗模式。在低功耗模式未知或不受支持的系统上，从 `lowPowerModeEnabled` 属性返回的值始终为 `NO`。

```c++
@interface NSProcessInfo (NSProcessInfoPowerState)

// Retrieve the current setting of the system for the low power mode setting.
// On systems where the low power mode is unknown or unsupported,
// the value returned from the lowPowerModeEnabled property is always NO.

@property (readonly, getter=isLowPowerModeEnabled) BOOL lowPowerModeEnabled API_AVAILABLE(ios(9.0), watchos(2.0), tvos(9.0)) API_UNAVAILABLE(macos);

@end
```


## 参考链接
**参考链接:🔗**
+ [iOS 启动优化 + 监控实践](https://juejin.cn/post/6844904194877587469) // 进行中...

+ [哈啰出行iOS App首屏秒开优化](https://juejin.cn/post/6948990967324082183) // 未开始
+ [抖音研发实践：基于二进制文件重排的解决方案 APP启动速度提升超15%](https://mp.weixin.qq.com/s/Drmmx5JtjG3UtTFksL6Q8Q) // 未开始
+ [iOS App冷启动治理：来自美团外卖的实践](https://juejin.cn/post/6844903733231353863)  // 未开始
+ [APP启动时间最精确的记录方式](https://juejin.cn/post/6844903997153755150)  // 未开始
+ [iOS如何获取当前时间--看我就够了](https://juejin.cn/post/6905671622037307405)  // 未开始
+ [启动优化](https://juejin.cn/post/6983513854546444296)  // 未开始
+ [iOS 优化篇 - 启动优化之Clang插桩实现二进制重排](https://juejin.cn/post/6844904130406793224#heading-29)  // 未开始
+ [懒人版二进制重排](https://juejin.cn/post/6844904192193085448#heading-7)  // 未开始
+ [我是如何让微博绿洲的启动速度提升30%的](https://juejin.cn/post/6844904143111323661)  // 未开始
+ [App性能优化小结](https://juejin.cn/post/6844903704886247431)  // 未开始














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
+ [抖音品质建设 - iOS启动优化《实战篇》](https://juejin.cn/post/6921508850684133390)
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
