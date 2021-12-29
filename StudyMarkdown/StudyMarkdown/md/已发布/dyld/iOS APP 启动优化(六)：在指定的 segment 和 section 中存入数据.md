# iOS APP 启动优化(六)：在指定的 segment 和 section 中存入数据  

&emsp;日常灵魂拷问，学习底层到底有没有用，很多人认为学习底层知识只是为了应付面试，日常开发中根本使用不到，事实真的是这样吗？今天我们就总结一些 mach-o 的知识点在日常开发中的一些使用场景，来验证一下我们学习底层知识点到底有没有用。

## 在指定的 segment 和 section 中存入数据

&emsp;在前面学习 mach-o 和 dyld 的过程中，我们看到了 dyld 任意的加载 mach-o 文件中指定 segment 的各个 section 中的内容，那么，我们可不可以干预 Xcode 生成 mach-o 文件的过程呢？那么，有没有一种方式，可以允许我们在 Xcode Build 过程中动态的在 mach-o 中插入新的 segment 和 section 呢？答案是可以的，下面我们直接揭晓答案：使用 `__attribute__ section` 将指定的数据储存到指定的 segment 和 section 中。

### \_\_attribute\_\_ 知识点扩展 

&emsp;下面我们首先做一个知识点的延展，看一下 `__attribute__` 相关的信息，`__attribute__` 可以用来设置函数属性（Function Attribute）、变量属性（Variable Attribute）和类型属性（Type Attribute）。它的书写特征是：`__attribute__` 前后都有两个下划线，并且后面会紧跟一对原括弧，括弧里面是相应的 `__attribute__` 参数，语法格式：`__attribute__((attribute-list))` 另外，它必须放于声明的尾部 `;` 之前。下面我们看一些比较常用的 `gcc Attribute syntax`。

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

#### \_\_attribute__((objc_root_class))

&emsp;这里我们再延伸一个可能被我们忽略了，但是还挺重要的知识点。我们大概一直都知道的 NSObject 作为根类（root_class），它的父类是 nil，我们日常使用的每个类都是 NSObject 的子类（NSProxy 除外，它是另外一个根类，它仅遵循 NSObject 协议，并不继承 NSObject 类）那么我们能不能自己创建一个不继承 NSObject 的类来使用呢？这篇文章 [不使用 NSOBJECT 的 OBJECTIVE-C CLASS](https://uranusjr.com/blog/post/53/objective-c-class-without-nsobject/) 会给我们答案。

&emsp;作为根类使用的类会使用 `NS_ROOT_CLASS` 宏来声明，例如：

+ NSProxy

```c++
NS_ROOT_CLASS
@interface NSProxy <NSObject> {
    __ptrauth_objc_isa_pointer Class    isa;
}
...
```

+ NSObject

```c++
OBJC_AVAILABLE(10.0, 2.0, 9.0, 1.0, 2.0)
OBJC_ROOT_CLASS // ⬅️ 这里有一个 OBJC_ROOT_CLASS 宏
OBJC_EXPORT
@interface NSObject <NSObject> {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wobjc-interface-ivars"
    Class isa  OBJC_ISA_AVAILABILITY;
#pragma clang diagnostic pop
}
```

&emsp;我们看一下 `OBJC_ROOT_CLASS` 宏的定义，它其实就是 `__attribute__((objc_root_class))`。

```c++
#if !defined(OBJC_ROOT_CLASS)
#   if __has_attribute(objc_root_class)
#       define OBJC_ROOT_CLASS __attribute__((objc_root_class))
#   else
#       define OBJC_ROOT_CLASS
#   endif
#endif
```

### \_\_attribute\_\_ ((section ("segment, section"))) 使用

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
            
            printf("😮😮😮 dli_fname:%s\n", info.dli_fname);
            printf("😮😮😮 dli_fbase:%p\n", info.dli_fbase);
            printf("😮😮😮 dli_sname:%s\n", info.dli_sname);
            printf("😮😮😮 dli_saddr:%p\n", info.dli_saddr);
            
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
// header 信息
😮😮😮 dli_fname:/Users/hmc/Library/Developer/Xcode/DerivedData/objc-efzravoaasjkrvghpezsjgrtdmuy/Build/Products/Debug/KCObjc
😮😮😮 dli_fbase:0x100000000
😮😮😮 dli_sname:GCC_except_table1
😮😮😮 dli_saddr:0x100003d0c

 ✳️ __DATA, __customSection1
 ✳️ __DATA, __customSection2
 ✳️✳️ __CUSTOMSEGMENT, __customSection1
 ✳️✳️ __CUSTOMSEGMENT, __customSection2
 ✳️✳️✳️ 5
```

> &emsp;有人会觉得，设置 section 的数据的意义是什么，也许在底层库的设计中可能会用到，但我们的日常开发中有使用场景吗？答案是肯定的。
> &emsp;这主要是由其特性决定的：设置 section 的时机在 main 函数之前。这么靠前的位置，其实可能帮助我们做一些管理性的工作，比如 APP 的启动器管理：在任何一个想要独立启动的模块中，声明其模块名，并写入对应的 section 中，那么 APP 启动时，就可以通过访问指定 section 中的内容来实现加载启动模块的功能。[iOS开发之runtime（12）：深入 Mach-O](https://xiaozhuanlan.com/topic/9204153876)

### dladdr 介绍

&emsp;示例代码中 `Dl_info` 结构体和 `dladdr` 函数我们可能比较陌生，它们两者都是在 dlfcn.h 中声明。上面 `main` 函数开头的 `if (machHeader == NULL) { ... }` 中正是使用 `dladdr` 来获取 header，然后拿到 header 以后作为 `getsectiondata` 函数的参数， 去取指定段和区中的数据。 

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

&emsp;下面我们对 `dladdr` 进行学习，`dladdr` 方法可以用来获取一个函数所在的模块、名称以及地址。下面我们继续看一个示例，这个示例是使用 `dladdr` 方法获取 `NSArray` 类的 `description` 函数的 `dl_info`  信息。

```c++
#import <dlfcn.h>
#include <objc/objc.h>
#include <objc/runtime.h>
#include <stdio.h>

int main(int argc, const char * argv[]) {

//    /*
//     * Structure filled in by dladdr().
//     */
//    typedef struct dl_info {
//            const char      *dli_fname;     /* Pathname of shared object */
//            void            *dli_fbase;     /* Base address of shared object */
//            const char      *dli_sname;     /* Name of nearest symbol */
//            void            *dli_saddr;     /* Address of nearest symbol */
//    } Dl_info;
//
//    extern int dladdr(const void *, Dl_info *);
    
    Dl_info info;
    IMP imp = class_getMethodImplementation(objc_getClass("NSArray"), sel_registerName("description"));
    
    printf("✳️✳️✳️ pointer %p\n", imp);
    
    if (dladdr((const void *)(imp), &info)) {
        printf("✳️✳️✳️ dli_fname: %s\n", info.dli_fname);
        printf("✳️✳️✳️ dli_fbase: %p\n", info.dli_fbase);
        printf("✳️✳️✳️ dli_sname: %s\n", info.dli_sname);
        printf("✳️✳️✳️ dli_saddr: %p\n", info.dli_saddr);
    } else {
        printf("error: can't find that symbol.\n");
    }
    
    return 0;
}

// ⬇️ 控制台打印内容如下：
✳️✳️✳️ pointer 0x7fff203f44dd
✳️✳️✳️ dli_fname: /System/Library/Frameworks/CoreFoundation.framework/Versions/A/CoreFoundation
✳️✳️✳️ dli_fbase: 0x7fff20387000
✳️✳️✳️ dli_sname: -[NSArray description]
✳️✳️✳️ dli_saddr: 0x7fff203f44dd
```

&emsp;如控制台打印，我们仅需要将 `NSArray` 类的 `description` 函数的 `IMP` 作为参数传递给 `dladdr` 函数，它就能获取到此 `IMP` 所在的模块、对应的函数的名称以及地址，所以我们可以通过这种方式来判断一个函数是不是被非法修改了。

&emsp;那么我们下面看一个验证函数是否被修改的例子：

```c++
// 🤯 要用到的头文件
#include <objc/runtime.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#import <dlfcn.h>

static inline BOOL validate_methods(const char *cls,const char *fname) __attribute__ ((always_inline));

BOOL validate_methods(const char *cls, const char *fname) {
    // 根据类名获取类对象
    Class aClass = objc_getClass(cls);

    // 用于记录 aClass 类的方法列表
    Method *methods;
    // 用于记录方法列表数量
    unsigned int nMethods;
    // 获取指定
    Dl_info info;
    // 用于记录 method 的 IMP
    IMP imp;

    char buf[128];
    Method m;

    if (!aClass) return NO;

    // 🤯 获取 aClass 的所有方法
    methods = class_copyMethodList(aClass, &nMethods);

    // 🤯 循环验证方法列表中的每个 method
    while (nMethods--) {
        m = methods[nMethods];

        printf("✳️✳️✳️ validating [%s %s]\n", (const char *)class_getName(aClass), (const char *)method_getName(m));
        
        // 🤯 取得函数的 IMP
        imp = method_getImplementation(m);
        // imp = class_getMethodImplementation(aClass, sel_registerName("allObjects"));
        
        if (!imp) {
            // IMP 不存在的话报错并 return
            printf("✳️✳️✳️ error: method_getImplementation(%s) failed\n", (const char *)method_getName(m));

            free(methods);
            return NO;
        }
        
        // 🤯 imp 做参数，通过 dladdr 函数获取 imp 的信息
        if (!dladdr((const void *)imp, &info)) {
            // 获取失败的话报错并 return
            printf("✳️✳️✳️ error: dladdr() failed for %s\n", (const char *)method_getName(m));

            free(methods);
            return NO;
        }

        // 🤯 Validate image path（验证（比较）函数所在的模块名，如果不同的话，则 goto 语句执行 FAIL 中的内容，打印 info 的信息）
        if (strcmp(info.dli_fname, fname)) {
            goto FAIL;
        }

        // 🤯 通过 dladdr 函数取得的函数名不为 NULL，且也不等于 <redacted> 时，否则打印一句 "✳️✳️✳️ <redacted>" 继续下个循环
        //（<redacted> 涉及一些符号化相关的知识点，后续我们再进行详细学习）
        if (info.dli_sname != NULL && strcmp(info.dli_sname, "<redacted>") != 0) {
            
            // 🤯 我们先看一下 snprintf 函数的定义，它是一个 C 库函数。
            // 🤯 C 库函数 int snprintf(char *str, size_t size, const char *format, ...) 
            // 设将可变参数 (...) 按照 format 格式化成字符串，并将字符串复制到 str 中，size 为要写入的字符的最大数目，超过 size 会被截断。
            // 返回值：
            // 1. 如果格式化后的字符串长度小于等于 size，则会把字符串全部复制到 str 中，并给其后添加一个字符串结束符 \0；
            // 2. 如果格式化后的字符串长度大于 size，超过 size 的部分会被截断，只将其中的 (size-1) 个字符复制到 str 中，并给其后添加一个字符串结束符 \0，返回值为欲写入的字符串长度。
            
            // 🤯 Validate class name in symbol
            
            // 获取 aClass 类对象的名字，然后按 "[%s " 这个格式保存在 buf 中（buf 是前面声明的长度是 128 的 char 数组）
            snprintf(buf, sizeof(buf), "[%s ", (const char *)class_getName(aClass));
            
            // 这里的字符串比较。dli_sname 是一个 const char *，它加 1 后应该是 info.dli_saddr 吧？
            if (strncmp(info.dli_sname + 1, buf, strlen(buf))) {
            
                // 获取 aClass 类对象的名字，然后按 "[%s(" 这个格式保存在 buf 中（buf 是前面声明的长度是 128 的 char 数组）
                // 跟上面类似，只是格式发生了变化 
                snprintf(buf, sizeof(buf), "[%s(", (const char *)class_getName(aClass));
                
                // 字符串比较。info.dli_sname + 1 应该是 info.dli_saddr 吧？
                if (strncmp(info.dli_sname + 1, buf, strlen(buf))) {
                    goto FAIL;
                }
            }

            // 🤯 Validate selector in symbol
            
            // 获取 m 方法的名字，然后按 " %s]" 这个格式保存在 buf 中（buf 是前面声明的长度是 128 的 char 数组）
            snprintf(buf, sizeof(buf), " %s]", (const char *)method_getName(m));

            if (strncmp(info.dli_sname + (strlen(info.dli_sname) - strlen(buf)), buf, strlen(buf))) {
                goto FAIL;
            }
            
        } else {
            printf("✳️✳️✳️ <redacted> \n");
        }
    }

    return YES;

FAIL:
    printf("🥶🥶🥶 method %s failed integrity test: \n", (const char *)method_getName(m));
    printf("🥶🥶🥶   dli_fname:%s\n", info.dli_fname);
    printf("🥶🥶🥶   dli_sname:%s\n", info.dli_sname);
    printf("🥶🥶🥶   dli_fbase:%p\n", info.dli_fbase);
    printf("🥶🥶🥶   dli_saddr:%p\n", info.dli_saddr);
    free(methods);
    return NO;
}
```

&emsp;然后我们便可以通过前面示例中取得的 NSArray 所在的模块路径，调用 `validate_methods` 函数：`validate_methods("NSArray", "/System/Library/Frameworks/CoreFoundation.framework/Versions/A/CoreFoundation");` 由于打印的内容太长了，这里就不复制粘贴了，感兴趣的小伙伴可以把代码粘贴出来自己试一下。

&emsp;这里 [iOS安全–验证函数地址，检测是否被替换，反注入](http://www.alonemonkey.com/ioss-validate-address.html) 是原文，看原文应该会更加清晰一些！

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

&emsp;下面列出真实的参考链接 🔗，上面是以前的其他文章的参考链接 🔗：

+ [iOS安全：修改Mach-O](https://easeapi.com/blog/blog/70-modify-Mach-O.html)
+ [6.33 Declaring Attributes of Functions](https://gcc.gnu.org/onlinedocs/gcc/Function-Attributes.html)
+ [iOS开发之runtime（12）：深入 Mach-O](https://xiaozhuanlan.com/topic/9204153876)
+ [iOS开发之runtime（16）：设置/获取section数据详解](https://xiaozhuanlan.com/topic/8932604571)
+ [不使用 NSOBJECT 的 OBJECTIVE-C CLASS](https://uranusjr.com/blog/post/53/objective-c-class-without-nsobject/)
+ [iOS安全–验证函数地址，检测是否被替换，反注入](http://www.alonemonkey.com/ioss-validate-address.html)
+ [AloneMonkey](http://blog.alonemonkey.com)
