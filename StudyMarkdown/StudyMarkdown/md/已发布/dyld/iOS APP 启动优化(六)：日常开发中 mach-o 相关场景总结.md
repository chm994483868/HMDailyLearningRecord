# iOS APP 启动优化(六)：日常开发中 mach-o 相关场景总结  

&emsp;日常灵魂拷问，学习底层到底有没有用，很多人认为学习底层知识只是为了应付面试，日常开发中根本使用不到，事实真的是这样吗？其实我觉得那些对你学习底层进行 “冷嘲热讽” 的人，仅仅是因为他们不想学习或者他们遇到困难就退缩学不会，而打击你来寻找存在感罢了，今天我们就总结一些 mach-o 的知识在日常开发中的一些使用场景。来验证一下我们学习底层知识点到底有没有用！

## 在 mach-o 中插入自定义的 segment 和 section

&emsp;在前面学习 mach-o 和 dyld 的过程中，我们看到了 dyld 任意的加载 mach-o 文件中指定 segment 的各个 section 中的内容，那么，我们可不可以干预 Xcode 生成 mach-o 文件的过程呢？那么，有没有一种方式，可以允许我们在 Xcode Build 过程中动态的在 mach-o 中插入新的 segment 和 section 呢？答案是可以的，下面我们直接揭晓答案：使用 `__attribute__ section` 将指定的数据储存到指定的 segment 和 section 中。

&emsp;下面我们首先做一个知识点的延展，看一下 `__attribute__` 相关的信息，`__attribute__` 可以用来设置函数和变量的属性。下面我们看一下一些比较常用的 `gcc Attribute syntax`。

+ `__attribute__((format()))` 按照指定格式进行参数检查。
+ `__attribute__((__always_inline__))` 强制内联。
+ `__attribute__((deprecated("Use xxx: instead")` 这个可能是我们见的比较多的，用来标记某个方法已经被废弃了，需要用其它的方法代替。
+ `__attribute__((__unused__))` 标记函数或变量可能不会用到。
+ `__attribute__((visibility("visibility_type")))` 标记动态库符号是否可见，有以下取值：

1. `default` 符号可见，可导出。
2. `hidden` 符号隐藏，不可导出，只能在本动态库内调用。

+ `__attribute__((objc_designated_initializer))` 明确指定用于初始化的方法。一个优秀的设计，初始化接口可以有多个，但最终多个初始化初始化接口都会调用 `designed initializer` 方法。

+ `__attribute__((unavailable))`、`__attribute__((unavailable("Must use xxx: instead.")));` 标记方法被禁用，不能直接调用，但这并不意味着该方法不能被调用，在 Objective-C 中使用 runtime 依然可以调用。

+ `__attribute__((section("segment,section")))` 将一个指定的数据储存到我们需要的 segment 和 section 中。

+ `__attribute__((constructor))` 被 `attribute((constructor))` 标记的函数，会在 `main` 函数之前或动态库加载时执行。在 mach-o 中，被 `attribute((constructor))` 标记的函数会在 `_DATA` 段的 `__mod_init_func` 区中。当多个被标记 `attribute((constructor))` 的方法想要有顺序的执行，怎么办？`attribute((constructor))` 是支持优先级的：`_attribute((constructor(1)))`。

+ `__attribute__((destructor))` 和 `attribute((constructor))` 相反：被 `attribute((destructor))` 标记的函数，会在 `main` 函数退出或动态库卸载时执行。在 mach-o 中此类函数会放在 `_DATA` 段的 `__mod_term_func` 区中。

&emsp;下面我们看一个示例代码：

```c++
struct __COM_easeapi {
    int count;
    const char *name;
};

// easeapi_section 是一个 __COM_easeapi 类型的结构体变量，然后用 __attribute__ 修饰使其位于 __COM 段的 __easeapi 区中
volatile struct __COM_easeapi easeapi_section __attribute__ ((section ("__COM, __easeapi"))) = {255, "name"};
```

&emsp;`__attribute__ ((section ("segment, section")))` 只能声明 C （全局）函数、全局（静态）变量、Objective-C （全局）方法及属性。例如我们直接把其放在我们的 `main` 函数中使用，就会报这样的错误：`'section' attribute only applies to functions, global variables, Objective-C methods, and Objective-C properties`，

&emsp;由于我们需要存储指定的信息，典型的做法就是像上述示例中使用结构体变量。这种方式看似解决了问题，但是有诸多限制：

1. 新插入的 section 数据必须是静态或全局的，不能是运行时生成的。（不是动态数据，可以是全局函数的返回值。）
2. `__TEXT` 段由于是只读的，其限制更大，仅支持绝对寻址，所以也不能使用字符串指针。如下代码：

```c++
char *tempString __attribute__((section("__TEXT, __customSection"))) = (char *)"customSection string value";
int tempInt __attribute__((section("__TEXT, __customSection"))) = 5;
```

&emsp;`tempInt` 能正常保存到 `__TEXT` 段的 `__customSection` 区中，也能正常读取到，而 `tempString` 的话则会直接报：`Absolute addressing not allowed in arm64 code but used in '_string5' referencing 'cstring'`。

&emsp;`__attribute__ ((section ("segment, section")))` 其中 `segment` 可以是已知的段名，也可以是我们自定义的段名，然后读取时保证一致就好了。

&emsp;`__attribute__ section` 的方式实际上是 mach-o 加载到内存后填充数据的，并不能直接填充至 mach-o 文件中的，例如上面示例代码中我们使用自定义的 `segment` 名字，然后打包后使用 MachOView 查看我们的可执行文件的结构，并不会有我们自定义的段，同样的我们使用现用的 `__TEXT` 和 `__DATA` 段，也不会添加新的 `section` 区。

&emsp;下面我们看另一位大佬的示例代码，看下如何读取我们放在指定段和区中的值。[iOS开发之runtime（12）：深入 Mach-O](https://xiaozhuanlan.com/topic/9204153876)

```c++
#import <Foundation/Foundation.h>

#import <dlfcn.h>
#import <mach-o/getsect.h>

#ifndef __LP64__
#define mach_header mach_header
#else
#define mach_header mach_header_64
#endif

const struct mach_header *machHeader = NULL;
static NSString *configuration = @"";

// 写入 __DATA, __customSection
char *string1 __attribute__((section("__DATA, __customSection"))) = (char *)"__DATA, __customSection1";
char *string2 __attribute__((section("__DATA, __customSection"))) = (char *)"__DATA, __customSection2";

// 写入 __CUSTOMSEGMENT, __customSection
char *string3 __attribute__((section("__CUSTOMSEGMENT, __customSection"))) = (char *)"__CUSTOMSEGMENT, __customSection1";
char *string4 __attribute__((section("__CUSTOMSEGMENT, __customSection"))) = (char *)"__CUSTOMSEGMENT, __customSection2";

// 在 __TEXT, __customSection 中写入字符串，则会报如下错误：
// Absolute addressing not allowed in arm64 code but used in '_string5' referencing 'cstring'

//char *string5 __attribute__((section("__TEXT, __customSection"))) = (char *)"customSection string value";

// 写入 __TEXT, __customSection
int tempInt __attribute__((section("__TEXT, __customSection"))) = 5;

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        
        // ⬇️ 直接在 main 函数中使用 __attribute__ section 会报如下错误：
        // 'section' attribute only applies to functions, global variables, Objective-C methods, and Objective-C properties
        // int tempInt2 __attribute__((section("__TEXT, __customSection"))) = 5;
        
        if (machHeader == NULL) {
            Dl_info info;
            dladdr((__bridge const void *)(configuration), &info);
            machHeader = (struct mach_header_64 *)info.dli_fbase;
        }
        
        unsigned long byteCount = 0;
        uintptr_t *data = (uintptr_t *)getsectiondata(machHeader, "__DATA", "__customSection", &byteCount);
        NSUInteger counter = byteCount/sizeof(void*);
        
        for (NSUInteger idx = 0; idx < counter; ++idx) {
            char *string = (char *)data[idx];
            NSString *str = [NSString stringWithUTF8String:string];
            NSLog(@"✳️ %@", str);
        }
        
        unsigned long byteCount1 = 0;
        uintptr_t *data1 = (uintptr_t *)getsectiondata(machHeader, "__CUSTOMSEGMENT", "__customSection", &byteCount1);
        NSUInteger counter1 = byteCount/sizeof(void*);
        
        for (NSUInteger idx = 0; idx < counter1; ++idx) {
            char *string = (char *)data1[idx];
            NSString *str = [NSString stringWithUTF8String:string];
            NSLog(@"✳️✳️ %@", str);
        }
        
        unsigned long byteCount2 = 0;
        uintptr_t *data2 = (uintptr_t *)getsectiondata(machHeader, "__TEXT", "__customSection", &byteCount2);
        NSUInteger counter2 = byteCount2/sizeof(int);
        
        for (NSUInteger idx = 0; idx < counter2; ++idx) {
            int intTemp = (int)data2[idx];
            NSLog(@"✳️✳️✳️ %d", intTemp);
        }
    }
    
    return 0;
}

// ⬇️ 控制台打印:
 ✳️ __DATA, __customSection1
 ✳️ __DATA, __customSection2
 ✳️✳️ __CUSTOMSEGMENT, __customSection1
 ✳️✳️ __CUSTOMSEGMENT, __customSection2
 ✳️✳️✳️ 5
```

&emsp;其中 `Dl_info` 结构体和 `dladdr` 函数我们可能比较陌生，它们两者都是在 dlfcn.h 中声明。上面 `main` 函数开头的 `if (machHeader == NULL) { ... }` 中的代码正是使用 `dladdr` 来获取 header，然后拿到 header 以后作为 `getsectiondata` 函数的参数。 

```c++
/*
 * Structure filled in by dladdr().
 */
typedef struct dl_info {
        const char      *dli_fname;     /* Pathname of shared object */
        void            *dli_fbase;     /* Base address of shared object */
        const char      *dli_sname;     /* Name of nearest symbol */
        void            *dli_saddr;     /* Address of nearest symbol */
} Dl_info;

extern int dladdr(const void *, Dl_info *);
```

















&emsp;设置 section 的数据的意义是什么呢？

## 模仿 static_init 调用构造函数

&emsp;在前面的 \_objc_init 过程解析中我们详细分析了 static_init 函数，已知它是

[iOS开发之runtime（11）：Mach-O 犹抱琵琶半遮面](https://xiaozhuanlan.com/topic/0328479651)


&emsp;全局搜索 \__objc_init_func 


## 设置 section 中的内容

&emsp;有人会觉得，设置 section 的数据的意义是什么，也许在底层库的设计中可能会用到，但我们的日常开发中有使用场景吗？这主要由其特性决定的：设置 section 的时机在 main 函数之前，这么靠前的位置，其实可能帮助我们做一些管理的工作，比如 APP 的启动器管理：在任何一个想要独立启动的模块中，声明其模块名，并写入相应的 section 中，那么 APP 启动时，就可以通过访问指定 section 中的内容来实现加载启动模块的功能。

```c++
#import <Foundation/Foundation.h>
#import <dlfcn.h>

#ifndef __LP64__
#define mach_header mach_header
#else
#define mach_header mach_header_64
#endif

const struct mach_header *machHeader = NULL;
static NSString *configuration = @"";


char *kString __attribute__((section("__DATA,__customSection"))) = (char *)"kyson.cn";
char *kString1 __attribute__((section("__DATA,__customSection"))) = (char *)"kyson.cn1";
char *kString2 __attribute__((section("__DATA,__customSection"))) = (char *)"kyson.cn2";
char *kString3 __attribute__((section("__DATA,__customSection"))) = (char *)"kyson.cn3";
char *kString4 __attribute__((section("__DATA,__customSection"))) = (char *)"kyson.cn4";

int main(int argc, const char * argv[]) {
//    @autoreleasepool {
//        // insert code here...
//        NSLog(@"🤯🤯🤯");
//    }
    NSLog(@"🤯🤯🤯");
//    [HMUncaughtExceptionHandle installUncaughtSignalExceptionHandler];
//    NSArray *tempArray = @[@(1), @(2), @(3)];
//    NSLog(@"🦁🦁🦁 %@", tempArray[100]);
    
    //LGPerson *person = [[LGPerson alloc] init];
    //person.name = @"小花";
    //person.sex = @"男";
    //person.age = 8;
    
    @autoreleasepool {
        
        if (machHeader == NULL) {
            Dl_info info;
            dladdr((__bridge const void *)(configuration), &info);
            machHeader = (struct mach_header_64 *)info.dli_fbase;
        }
        
        unsigned long byteCount = 0;
        uintptr_t *data = (uintptr_t *)getsectiondata(machHeader, "__DATA", "__customSection", &byteCount);
        NSUInteger counter = byteCount/sizeof(void*);
        for (NSUInteger idx = 0; idx < counter; ++idx) {
            char *string = (char *)data[idx];
            NSString *str = [NSString stringWithUTF8String:string];
            NSLog(@"✳️✳️✳️ %@", str);
        }
    }
    
    return 0;
}
```

## 修改 mach-o 

&emsp;使用 \_\_attribute__ section。

[iOS 安全：修改 mach-o](https://easeapi.com/blog/blog/70-modify-Mach-O.html)

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


+ [iOS 启动优化 + 监控实践](https://juejin.cn/post/6844904194877587469)
+ [深入理解MachO数据解析规则](https://juejin.cn/post/6947843156163428383)
+ [探秘 Mach-O 文件](http://hawk0620.github.io/blog/2018/03/22/study-mach-o-file/)
+ [深入剖析Macho (1)](http://satanwoo.github.io/2017/06/13/Macho-1/)

&emsp;下面列出真实的参考链接 🔗：
+ [iOS安全：修改Mach-O](https://easeapi.com/blog/blog/70-modify-Mach-O.html)
+ [6.33 Declaring Attributes of Functions](https://gcc.gnu.org/onlinedocs/gcc/Function-Attributes.html)

+ [iOS开发之runtime（16）：设置/获取section数据详解](https://xiaozhuanlan.com/topic/8932604571)
+ [iOS安全–验证函数地址，检测是否被替换，反注入](http://www.alonemonkey.com/ioss-validate-address.html)
+ [AloneMonkey](http://blog.alonemonkey.com)
