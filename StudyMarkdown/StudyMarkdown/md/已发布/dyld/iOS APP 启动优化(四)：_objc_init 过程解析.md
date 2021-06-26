# iOS APP 启动优化(四)：_objc_init 过程解析

&emsp;上一篇学习 dyld 涉及到 objc 中的 \_objc_init 函数，但是我们没有深入学习其涉及到的流程，那么就由本篇开始吧。

## \_objc_init
&emsp;在 objc/Source/objc-os.mm 中可找到 `void _objc_init(void)` 的定义。 

```c++
/***********************************************************************
* _objc_init
* Bootstrap initialization. Registers our image notifier with dyld.
* Called by libSystem BEFORE library initialization time
**********************************************************************/

void _objc_init(void)
{
    // initialized 局部静态变量，保证只初始化一次，下次再调用 _objc_init 则直接 return
    static bool initialized = false;
    if (initialized) return;
    initialized = true;
    
    // fixme defer initialization until an objc-using image is found?
    environ_init(); // 1⃣️ 环境变量初始化 
    
    tls_init(); // 2⃣️ 本地线程池
    static_init(); // 3⃣️ 系统级别的 C++ 构造函数调用
    runtime_init(); // 4⃣️ runtime 初始化
    exception_init(); // 5⃣️ 注册监听异常的回调
    cache_init(); // 6⃣️ cache 的初始化
    _imp_implementationWithBlock_init(); // 7⃣️ 对 imp 的 Block 标记进行初始化

    // 8⃣️ 注册回调通知，& 是引用类型的参数
    _dyld_objc_notify_register(&map_images, load_images, unmap_image);

#if __OBJC2__
    // 9⃣️ dyld 通知注册标记
    didCallDyldNotifyRegister = true;
#endif
}
```

&emsp;下面我们就详细看看 1⃣️ 到 9⃣️ 的具体实现。

## environ_init

&emsp;`environ_init` 方法就是进行环境变量的初始化。在项目运行之前我们可以在 Edit Scheme... -> Run -> Arguments -> Environment Variables 中添加环境变量以及对应的值，它们默认都是 NO，我们可以根据我们的需要来进行添加并把 value 设置为 YES。在 objc-env.h 文件中列出了所有的环境变量，其中它们分别以 OBJC_PRINT_、OBJC_DEBUG_、OBJC_DISABLE_ 为开头分了 3 块，分别针对 PRINT（打印）、DEBUG（调试）、DISABLE（禁用）的情况。环境变量的设置，可以帮助我们更快速的处理一些问题。例如添加 OBJC_PRINT_LOAD_METHODS 环境变量，控制台就会打印项目中所有的系统类、我们自己的类以及分类中的 `+load` 方法。

```c++
/***********************************************************************
* environ_init
* Read environment variables that affect the runtime. 读取影响 runtime 的环境变量。
* Also print environment variable help, if requested. 如果有需要也会打印对我们有帮助的环境变量。
**********************************************************************/
void environ_init(void) 
{
    if (issetugid()) {
        // All environment variables are silently ignored when setuid or setgid.
        // 当 setuid 或 setgid 时，所有的环境变量是被静默忽略的。
        
        // This includes OBJC_HELP and OBJC_PRINT_OPTIONS themselves.
        // 这也包括 OBJC_HELP 和 OBJC_PRINT_OPTIONS。
        
        return; // ⬅️ 直接 return
    } 
    
    // 三个局部变量，默认是 false，然后在下面第一个 for 循环中判断是否把它们置为 true。
    bool PrintHelp = false;
    bool PrintOptions = false;
    bool maybeMallocDebugging = false;

    // Scan environ[] directly instead of calling getenv() a lot.
    // This optimizes the case where none are set.
    
    // 直接遍历扫描 environ[] 
    for (char **p = *_NSGetEnviron(); *p != nil; p++) {
        // 如果扫描到 "Malloc"、"DYLD"、"NSZombiesEnabled" 则把 maybeMallocDebugging 置为 true
        if (0 == strncmp(*p, "Malloc", 6)  ||  0 == strncmp(*p, "DYLD", 4)  ||  
            0 == strncmp(*p, "NSZombiesEnabled", 16))
        {
            maybeMallocDebugging = true;
        }
        
        // 如果是 "OBJC_" 打头的则直接跳过
        if (0 != strncmp(*p, "OBJC_", 5)) continue;
        
        // 如果扫描到 "OBJC_HELP=" 则把 PrintHelp 置为 true
        if (0 == strncmp(*p, "OBJC_HELP=", 10)) {
            PrintHelp = true;
            continue;
        }
        
        // 如果扫描到 "OBJC_PRINT_OPTIONS=" 则把 PrintOptions 置为 true
        if (0 == strncmp(*p, "OBJC_PRINT_OPTIONS=", 19)) {
            PrintOptions = true;
            continue;
        }
        
        // strchr 函数功能为在一个串中查找给定字符的第一个匹配之处。
        // 函数原型为：char *strchr(const char *str, int c)，
        // 即在参数 str 所指向的字符串中搜索第一次出现字符 c（一个无符号字符）的位置。
        // strchr 函数包含在 C 标准库 <string.h> 中。
        
        // 查找 p 中第一个 = 的位置 
        const char *value = strchr(*p, '=');
        if (!*value) continue;
        value++; // 然后这里 value 做了一次自增运算（因为 value 是一个 char 指针，所以 value 前进一个字节）
        
        // 这里是遍历 Settings 这个元素类型是 option_t 的全局不可变数组。
        // 在 objc-env.h 文件中列出了所有的 option_t 项。
        for (size_t i = 0; i < sizeof(Settings)/sizeof(Settings[0]); i++) {
            // ⚠️ 实话实说，下面这一段判断是否要赋值为 YES，没有完全看懂，
            // 大概是从 _NSGetEnviron 拿到环境变量，
            // 然后遍历来判断是否需要把 Settings 中存放的对应的 option_t 实例的 var 成员变量置为 YES。
            const option_t *opt = &Settings[i];
            if ((size_t)(value - *p) == 1+opt->envlen  &&  
                0 == strncmp(*p, opt->env, opt->envlen))
            {
                *opt->var = (0 == strcmp(value, "YES"));
                break;
            }
        }            
    }

    // Special case: enable some autorelease pool debugging
    // 特例：启用一些自动释放池调试
    // when some malloc debugging is enabled and OBJC_DEBUG_POOL_ALLOCATION is not set to something other than NO.
    // 当启用了某些 malloc 调试并且 OBJC_DEBUG_POOL_ALLOCATION 未设置为 NO 以外的值时。
    
    // 两个 if 判断是否把 DebugPoolAllocation 置为 true
    if (maybeMallocDebugging) {
        const char *insert = getenv("DYLD_INSERT_LIBRARIES");
        const char *zombie = getenv("NSZombiesEnabled");
        const char *pooldebug = getenv("OBJC_DEBUG_POOL_ALLOCATION");
        if ((getenv("MallocStackLogging")
             || getenv("MallocStackLoggingNoCompact")
             || (zombie && (*zombie == 'Y' || *zombie == 'y'))
             || (insert && strstr(insert, "libgmalloc")))
            &&
            (!pooldebug || 0 == strcmp(pooldebug, "YES")))
        {
            DebugPoolAllocation = true;
        }
    }

    // Print OBJC_HELP and OBJC_PRINT_OPTIONS output.
    // 打印 OBJC_HELP 和 OBJC_PRINT_OPTIONS 输出。
    if (PrintHelp  ||  PrintOptions) {
        // 下面两个 if 是输出一些提示信息
        if (PrintHelp) {
            // Objective-C 运行时调试。设置 variable=YES 以启用。
            _objc_inform("Objective-C runtime debugging. Set variable=YES to enable.");
            // OBJC_HELP：描述可用的环境变量
            _objc_inform("OBJC_HELP: describe available environment variables");
            if (PrintOptions) {
                // OBJC_HELP 已设置
                _objc_inform("OBJC_HELP is set");
            }
            // OBJC_PRINT_OPTIONS：列出设置了哪些选项
            _objc_inform("OBJC_PRINT_OPTIONS: list which options are set");
        }
        if (PrintOptions) {
            // OBJC_PRINT_OPTIONS 已设置
            _objc_inform("OBJC_PRINT_OPTIONS is set");
        }
        
        // 下面的 for 循环便是遍历打印 Settings 中的所有的环境变量的 env 以及对应的描述，
        // 然后是打印我们设置为 YES 的环境变量，表示我们对该环境变量进行了设置。
        for (size_t i = 0; i < sizeof(Settings)/sizeof(Settings[0]); i++) {
            const option_t *opt = &Settings[i];
            // 打印现有的环境变量，以及其对应的描述
            if (PrintHelp) _objc_inform("%s: %s", opt->env, opt->help);
            
            // 打印 var 的值设置为 YES 的环境变量，即告诉我们当前对哪些环境变量进行了设置
            if (PrintOptions && *opt->var) _objc_inform("%s is set", opt->env);
        }
    }
}
```

### struct option_t 和 Settings 数组

&emsp;`objc-env.h` 中就完全是一大组 `OPTION` 宏的使用，定义了一组 `option_t` 结构体实例，每一个 `option_t` 实例就用来表示一个环境变量。

&emsp;这里我们首先要明白 `#include "objc-env.h"` 的作用，在编译时编译器会把  `#include "objc-env.h"` 直接替换为 `objc-env.h` 文件中的一大组  `option_t` 实例，即这里的 `const option_t Settings[]` 数组便包含了 `objc-env.h` 中的所有的 `option_t` 结构体实例。

&emsp;这里我们摘出其中几个比较重要或者我们比较熟悉的环境变量，如：`OBJC_DISABLE_TAGGED_POINTERS` 表示是否禁用 NSNumber、NSString 等的 tagged pointer 指针优化、`OBJC_DISABLE_TAG_OBFUSCATION` 表示是否禁用 tagged pointer 的混淆、`OBJC_DISABLE_NONPOINTER_ISA` 表示是否禁用 non-pointer 的 isa 字段。

```c++
struct option_t {
    bool* var;
    const char *env;
    const char *help;
    size_t envlen;
};

const option_t Settings[] = {
#define OPTION(var, env, help) option_t{&var, #env, help, strlen(#env)}, 
#include "objc-env.h"
#undef OPTION
};
```

```c++
// OPTION(var, env, help)

OPTION( PrintImages,              OBJC_PRINT_IMAGES,               "log image and library names as they are loaded")
OPTION( PrintImageTimes,          OBJC_PRINT_IMAGE_TIMES,          "measure duration of image loading steps")
...
OPTION( DisableTaggedPointers,    OBJC_DISABLE_TAGGED_POINTERS,    "disable tagged pointer optimization of NSNumber et al.") 
OPTION( DisableTaggedPointerObfuscation, OBJC_DISABLE_TAG_OBFUSCATION,    "disable obfuscation of tagged pointers")
OPTION( DisableNonpointerIsa,     OBJC_DISABLE_NONPOINTER_ISA,     "disable non-pointer isa fields")
...
```

### 设置 OBJC_DISABLE_NONPOINTER_ISA

&emsp;下面我们演示一下 OBJC_DISABLE_NONPOINTER_ISA 的使用，我们在 Environment Variables 中添加 OBJC_DISABLE_NONPOINTER_ISA 并设置为 YES。（我们应该还记得如何判断实例对象的 isa 是 non-pointer 还是 pointer，即 uintptr_t nonpointer : 1，如果 isa 的第一位是 1 则表示它是 non-pointer 否则就是 pointer。）

&emsp;我们创建一个实例对象，然后分别在 OBJC_DISABLE_NONPOINTER_ISA 设置为 YES/NS 的情况下打印它的 isa 中的内容。

```c++
// ⬇️⬇️ OBJC_DISABLE_NONPOINTER_ISA 未设置或者设置为 NO
(lldb) x/4gx person
0x108baa1a0: 0x011d8001000080e9 0x0000000000000000
0x108baa1b0: 0x6b636950534e5b2d 0x426863756f547265
(lldb) p/t 0x011d8001000080e9
// ⬇️ isa 第一位是 1
(long) $1 = 0b0000000100011101100000000000000100000000000000001000000011101001

// ⬇️⬇️ OBJC_DISABLE_NONPOINTER_ISA 设置为 YES
(lldb) x/4gx person
0x108d04080: 0x00000001000080e8 0x0000000000000000
0x108d04090: 0x00000001a0080001 0x0000000100008028
(lldb) p/t 0x00000001000080e8
// ⬇️ isa 第一位是 0
(long) $1 = 0b0000000000000000000000000000000100000000000000001000000011101000
```

### 设置 OBJC_PRINT_LOAD_METHODS

&emsp;下面我们演示一下 OBJC_PRINT_LOAD_METHODS 的使用，我们在 Environment Variables 中添加 OBJC_PRINT_LOAD_METHODS 并设置为 YES。运行项目，可看到如下打印项目中所有的 load 方法。

```c++
objc[37659]: LOAD: category 'NSObject(NSObject)' scheduled for +load
objc[37659]: LOAD: +[NSObject(NSObject) load]

objc[37659]: LOAD: category 'NSObject(NSObject)' scheduled for +load
objc[37659]: LOAD: +[NSObject(NSObject) load]

objc[37659]: LOAD: class 'NSColor' scheduled for +load
objc[37659]: LOAD: class 'NSApplication' scheduled for +load
objc[37659]: LOAD: class 'NSBinder' scheduled for +load
objc[37659]: LOAD: class 'NSColorSpaceColor' scheduled for +load
objc[37659]: LOAD: class 'NSNextStepFrame' scheduled for +load
objc[37659]: LOAD: +[NSColor load]

objc[37659]: LOAD: +[NSApplication load]

objc[37659]: LOAD: +[NSBinder load]

objc[37659]: LOAD: +[NSColorSpaceColor load]

objc[37659]: LOAD: +[NSNextStepFrame load]

objc[37659]: LOAD: category 'NSError(FPAdditions)' scheduled for +load
objc[37659]: LOAD: +[NSError(FPAdditions) load]

objc[37659]: LOAD: class '_DKEventQuery' scheduled for +load
objc[37659]: LOAD: +[_DKEventQuery load]

objc[37659]: LOAD: class 'LGPerson' scheduled for +load
objc[37659]: LOAD: +[LGPerson load]
```

&emsp;环境变量的学习我们就到这里了，objc-env.h 中还有其他的一些环境变量的设置，这里就不一一演示了。

## tls_init

&emsp;`_objc_pthread_destroyspecific` 是线程的销毁函数。以 `TLS_DIRECT_KEY` 为 Key，在线程的本地存储空间中保存线程对应对销毁函数。（没有看到哪里体现的进行了线程池的初始化，TLS 应该是 Thread Local Storage 的缩写，即线程本地存储，这里大概是线程本地存储的初始化。）

```c++
void tls_init(void)
{
#if SUPPORT_DIRECT_THREAD_KEYS
    pthread_key_init_np(TLS_DIRECT_KEY, &_objc_pthread_destroyspecific);
#else
    _objc_pthread_key = tls_create(&_objc_pthread_destroyspecific);
#endif
}
```

## static_init

&emsp;运行 C++ 静态构造函数。libc 在 dyld 调用我们的静态构造函数之前调用 _objc_init()，所以我们必须自己做。

```c++
/***********************************************************************
* static_init
* Run C++ static constructor functions.
* libc calls _objc_init() before dyld would call our static constructors, so we have to do it ourselves.
**********************************************************************/
static void static_init()
{
    size_t count;
    auto inits = getLibobjcInitializers(&_mh_dylib_header, &count);
    for (size_t i = 0; i < count; i++) {
        inits[i]();
    }
}
```

&emsp;我们分析一下 `static_init` 函数的内容，其中的 `auto inits = getLibobjcInitializers(&_mh_dylib_header, &count);` 是从 `__objc_init_func` 区中取 `UnsignedInitializer` 的内容，取出以后就直接 for 循环遍历执行所有的 `UnsignedInitializer` 函数。

&emsp;`getLibobjcInitializers` 这个函数是用 `GETSECT` 宏定义的。（从名字上我们大概也能猜到它的功能：从区中取数据。）

```c++
GETSECT(getLibobjcInitializers, UnsignedInitializer, "__objc_init_func");

#define GETSECT(name, type, sectname)                                   \
    type *name(const headerType *mhdr, size_t *outCount) {              \
        return getDataSection<type>(mhdr, sectname, nil, outCount);     \
    }                                                                   \
    type *name(const header_info *hi, size_t *outCount) {               \
        return getDataSection<type>(hi->mhdr(), sectname, nil, outCount); \
    }
```

&emsp;把 `GETSECT(getLibobjcInitializers, UnsignedInitializer, "__objc_init_func")` 展开的话就是如下的两个函数定义。

```c++
UnsignedInitializer *getLibobjcInitializers(const headerType *mhdr, size_t *outCount) {
    return getDataSection<UnsignedInitializer>(mhdr, "__objc_init_func", nil, outCount);
}

UnsignedInitializer *getLibobjcInitializers(const header_info *hi, size_t *outCount) {
    return getDataSection<UnsignedInitializer>(hi->mhdr(), "__objc_init_func", nil, outCount);
}
```

## runtime_init

&emsp;`unattachedCategories` 是在 namespace objc 中定义的一个静态变量：`static UnattachedCategories unattachedCategories;`，`UnattachedCategories` 类的声明：`class UnattachedCategories : public ExplicitInitDenseMap<Class, category_list>` 它主要用来为类统计分类、追加分类到类、清除分类数据、清除类数据。

&emsp;`static ExplicitInitDenseSet<Class> allocatedClasses;`，`allocatedClasses` 是已使用 `objc_allocateClassPair` allocated 过的所有类（和元类）的表。 它也是在 namespace objc 中声明的一个静态变量。

&emsp;`objc::unattachedCategories.init(32)` 是初始化分类的存储容器，`objc::allocatedClasses.init()` 是初始化类的存储容器。这个方法作用就是进行初始化容器工作，后面会用到这些初始化的容器。

```c++
void runtime_init(void)
{
    objc::unattachedCategories.init(32);
    objc::allocatedClasses.init();
}
```

## exception_init

&emsp;初始化 libobjc 的异常处理系统。通过 `map_images` 调用。

&emsp;`exception_init` 函数内部就是对 `old_terminate` 这个静态全局的函数指针赋值，`set_terminate` 函数是一个入参和返回值都是 `terminate_handler`（参数和返回值都是 `void` 的函数指针）的函数。以 `_objc_terminate` 函数地址为参数，把返回值赋值给 `old_terminate`。

```c++
static void (*old_terminate)(void) = nil;
```

```c++
typedef void (*terminate_handler)();
_LIBCPP_FUNC_VIS terminate_handler set_terminate(terminate_handler) _NOEXCEPT;
```

```c++
/***********************************************************************
* exception_init
* Initialize libobjc's exception handling system. 
* Called by map_images().
**********************************************************************/
void exception_init(void)
{
    old_terminate = std::set_terminate(&_objc_terminate);
}
```

&emsp;`exception_init` 就是注册监听异常回调，系统方法在执行的过程中，出现异常触发中断，就会报出异常，如果我们在上层对这个方法处理，我们就能捕获这次异常。注意：是系统方法执行异常。我们可以在这里去监听系统异常，我们看下怎么处理。 我们看下 `_objc_terminate` 方法。

```c++
/***********************************************************************
* _objc_terminate
* Custom std::terminate handler.
*
* The uncaught exception callback is implemented as a std::terminate handler. 
* 1. Check if there's an active exception
* 2. If so, check if it's an Objective-C exception
* 3. If so, call our registered callback with the object.
* 4. Finally, call the previous terminate handler.
**********************************************************************/
static void (*old_terminate)(void) = nil;
static void _objc_terminate(void)
{
    // OPTION( PrintExceptions, OBJC_PRINT_EXCEPTIONS, "log exception handling")
    // OBJC_PRINT_EXCEPTIONS 是在 objc-env.h 中定义的环境变量，根据其值判断是否打印 log
    if (PrintExceptions) {
        _objc_inform("EXCEPTIONS: terminating");
    }

    if (! __cxa_current_exception_type()) {
        // No current exception.
        (*old_terminate)();
    }
    else {
        // There is a current exception. Check if it's an objc exception.
        // 当前有一个 exception。检查它是否是一个 objc exception。
        @try {
            __cxa_rethrow();
        } @catch (id e) {
            // It's an objc object. Call Foundation's handler, if any.
            // 它是一个 objc object。调用 Foundation 的处理程序（如果有）。
            (*uncaught_handler)((id)e);
            
            (*old_terminate)();
        } @catch (...) {
            // It's not an objc object. Continue to C++ terminate.
            // 它不是一个 objc object。进行 C++ terminate。
            
            (*old_terminate)();
        }
    }
}
```

&emsp;`@catch (id e)` 下面我们看到，如果捕获到 objc 对象，那就执行 `uncaught_handler` 函数。`uncaught_handler` 是一个静态全局函数。

```c++
// 入参是 id 返回值是 void 的指针
typedef void (*objc_uncaught_exception_handler)(id _Null_unspecified /* _Nonnull */ exception);

/***********************************************************************
* _objc_default_uncaught_exception_handler
* Default uncaught exception handler. Expected to be overridden by Foundation.
* 默认 uncaught exception 处理程序。预计将被 Foundation 重载。
**********************************************************************/
static void _objc_default_uncaught_exception_handler(id exception)
{
}
static objc_uncaught_exception_handler uncaught_handler = _objc_default_uncaught_exception_handler;
```

&emsp;我们看到 `uncaught_handler` 默认赋值为 `_objc_default_uncaught_exception_handler` 函数，而其实现其实是空的，即如果我们没有给 `uncaught_handler` 赋值的话，则就由系统处理。

&emsp;我们看下哪里会给 `uncaught_handler` 赋值，我们全局搜 `uncaught_handler`。

```c++
/***********************************************************************
* objc_setUncaughtExceptionHandler
* Set a handler for uncaught Objective-C exceptions.
* 为 uncaught Objective-C exceptions 设置处理程序。
* Returns the previous handler. 
* 返回值是以前的处理程序。
**********************************************************************/
objc_uncaught_exception_handler 
objc_setUncaughtExceptionHandler(objc_uncaught_exception_handler fn)
{
    // result 记录旧值，用于返回值
    objc_uncaught_exception_handler result = uncaught_handler;
    
    // 给 uncaught_handler 设置新值
    uncaught_handler = fn;
    
    // 返回旧值
    return result;
}
```

&emsp;`objc_setUncaughtExceptionHandler` 方法为捕获 Objective-C 异常设置一个处理程序，并返回之前的处理程序。我们看其方法实现也是将传入的方法赋值给 `uncaught_handler`。

&emsp;在 `_objc_terminate` 函数中我们看到，当捕获到一个 Objective-C 异常时，会使用该异常对象（NSException）调用我们注册的回调（`uncaught_handler`），下面我们把系统提供的默认的实现为空的 `_objc_default_uncaught_exception_handler` 函数，使用 `objc_setUncaughtExceptionHandler` 替换为我们自己的函数。 

```c++
#import "HMUncaughtExceptionHandle.h"

@implementation HMUncaughtExceptionHandle

void TestExceptionHandlers(NSException *exception) {
    NSLog(@"🦷🦷🦷 %@ 🦷🦷🦷 %@", exception.name, exception.reason);
}

+ (void)installUncaughtSignalExceptionHandler {
    NSSetUncaughtExceptionHandler(&TestExceptionHandlers);
}

@end

#import <Foundation/Foundation.h>
#import "HMUncaughtExceptionHandle.h"

int main(int argc, const char * argv[]) {
    [HMUncaughtExceptionHandle installUncaughtSignalExceptionHandler];
    
    NSLog(@"🍀🍀🍀 %s", __func__);
    
    NSArray *tempArray = @[@(1), @(2), @(3)];
    NSLog(@"%@", tempArray[100]);
    
    return 0;
}
```

&emsp;我们把我们自定义的 `TestExceptionHandlers` 通过 `NSSetUncaughtExceptionHandler` 函数赋值给 `uncaught_handler`，当我们在 `main` 函数中主动触发一个数组越界的异常时，系统就会调用我们的 `TestExceptionHandlers` 函数。在 `installUncaughtSignalExceptionHandler` 函数中我们看到了 `NSSetUncaughtExceptionHandler` 函数，它是在 NSException.h 中声明的（`FOUNDATION_EXPORT void NSSetUncaughtExceptionHandler(NSUncaughtExceptionHandler * _Nullable);`），它的定义正对应了 `objc_setUncaughtExceptionHandler` 函数。

```c++
// ⬇️⬇️⬇️ 异常发生时调用了我们的 TestExceptionHandlers 函数
2021-06-11 08:50:03.385083+0800 KCObjc[22433:2951328] 🦷🦷🦷 NSRangeException 🦷🦷🦷 *** -[__NSArrayI objectAtIndexedSubscript:]: index 100 beyond bounds [0 .. 2]

2021-06-11 08:50:03.385154+0800 KCObjc[22433:2951328] *** Terminating app due to uncaught exception 'NSRangeException', reason: '*** -[__NSArrayI objectAtIndexedSubscript:]: index 100 beyond bounds [0 .. 2]'
*** First throw call stack:
(
    0   CoreFoundation                      0x00007fff20484083 __exceptionPreprocess + 242
    1   libobjc.A.dylib                     0x00007fff201bc17c objc_exception_throw + 48
    2   CoreFoundation                      0x00007fff20538c82 _CFThrowFormattedException + 194
    3   CoreFoundation                      0x00007fff203f7991 +[NSNull null] + 0
    4   KCObjc                              0x0000000100003d34 main + 292
    5   libdyld.dylib                       0x00007fff2032d621 start + 1
    6   ???                                 0x0000000000000001 0x0 + 1
)
libc++abi.dylib: terminating with uncaught exception of type NSException
*** Terminating app due to uncaught exception 'NSRangeException', reason: '*** -[__NSArrayI objectAtIndexedSubscript:]: index 100 beyond bounds [0 .. 2]'
terminating with uncaught exception of type NSException
```

&emsp;那么 `exception_init` 我们就看到这里，下面我们开始看下一个函数。

## cache_init

&emsp;`cache_init` 看名字我们能猜到它和缓存初始化有关，而这个 `cache` 指的就是方法缓存。`cache_init` 函数的定义正位于我们之前学习方法缓存时看了无数遍的 `objc-cache.mm` 文件。

&emsp;`HAVE_TASK_RESTARTABLE_RANGES` 是一个宏定义，在不同的平台下它的值是 0 或 1。

```c++
// Define HAVE_TASK_RESTARTABLE_RANGES to enable usage of task_restartable_ranges_synchronize()
// 启用 task_restartable_ranges_synchronize

#if TARGET_OS_SIMULATOR || defined(__i386__) || defined(__arm__) || !TARGET_OS_MAC
#   define HAVE_TASK_RESTARTABLE_RANGES 0
#else

// ⬇️⬇️⬇️
#   define HAVE_TASK_RESTARTABLE_RANGES 1
// ⬆️⬆️⬆️

#endif
```

&emsp;`objc_restartableRanges` 是一个全局的 `task_restartable_range_t` 数组。 

```c++
extern "C" task_restartable_range_t objc_restartableRanges[];

void cache_init()
{
#if HAVE_TASK_RESTARTABLE_RANGES
    // mach_msg_type_number_t 当前是 unsigned int 的别名，定义别名利于不同的平台做兼容
    mach_msg_type_number_t count = 0;
    // 同样 kern_return_t 当前是 int 的别名
    kern_return_t kr;

    // 统计 objc_restartableRanges 数组中存在 location 的 task_restartable_range_t 的数量
    while (objc_restartableRanges[count].location) {
        count++;
    }
    
    // Register a set of restartable ranges for the current task.
    kr = task_restartable_ranges_register(mach_task_self(),
                                          objc_restartableRanges, count);
    if (kr == KERN_SUCCESS) return;
    
    // 注册失败则停止运行
    _objc_fatal("task_restartable_ranges_register failed (result 0x%x: %s)", kr, mach_error_string(kr));
    
#endif // HAVE_TASK_RESTARTABLE_RANGES
}
```

&emsp;我们全局搜索 `objc_restartableRanges` 可看到，在 `_collecting_in_critical` 函数中有看到有对其的遍历读取。`_collecting_in_critical` 函数在前面的方法缓存中我们有学习过，用于判断当前是否可以对旧的方法缓存（扩容后的旧的方法缓存表）进行收集释放，如果某个线程当前正在执行缓存读取函数，则返回 TRUE。当缓存读取功能正在进行时，不允许收集 cache garbage，因为它可能仍在使用 garbage memory。即当前有其它线程正在读取使用我们的旧的方法缓存表时，此时不能对旧的方法缓存表进行内存释放。

&emsp;`_objc_restartableRanges` 被方法调度缓存代码（method dispatch caching code）用来确定是否有任何线程在缓存中主动进行调度。

## \_imp_implementationWithBlock_init

&emsp;初始化 trampoline machinery。通常这什么都不做，因为一切都是惰性初始化的，但对于某些进程，我们急切地加载 trampolines dylib。

&emsp;在某些进程中急切地加载 libobjc-trampolines.dylib。一些程序（最显着的是旧版本的嵌入式 Chromium 使用的 QtWebEngineProcess）启用了一个高度限制性的沙盒配置文件，它会阻止访问该 dylib。如果有任何东西调用了 `imp_implementationWithBlock`（就像 AppKit 开始做的那样），那么我们将在尝试加载它时崩溃。在启用 sandbox 配置文件并阻止它之前，在此处加载它会对其进行设置。

&emsp;`Trampolines` 是一个 `TrampolinePointerWrapper` 类型的静态全局变量。

```c++
static TrampolinePointerWrapper Trampolines;
```

```c++
/// Initialize the trampoline machinery. Normally this does nothing, as
/// everything is initialized lazily, but for certain processes we eagerly load
/// the trampolines dylib.
void
_imp_implementationWithBlock_init(void)
{
#if TARGET_OS_OSX
    // Eagerly load libobjc-trampolines.dylib in certain processes. Some
    // programs (most notably QtWebEngineProcess used by older versions of
    // embedded Chromium) enable a highly restrictive sandbox profile which
    // blocks access to that dylib. If anything calls
    // imp_implementationWithBlock (as AppKit has started doing) then we'll
    // crash trying to load it. Loading it here sets it up before the sandbox
    // profile is enabled and blocks it.
    //
    // This fixes EA Origin (rdar://problem/50813789)
    // and Steam (rdar://problem/55286131)
    if (__progname &&
        (strcmp(__progname, "QtWebEngineProcess") == 0 ||
         strcmp(__progname, "Steam Helper") == 0)) {
        Trampolines.Initialize();
    }
#endif
}
```

&emsp;`_imp_implementationWithBlock_init` 方法实现可以看到这个是在 OS 下执行，这个方法就是对 imp 的 Block 标记进行初始化。

&emsp;下面我们对 `imp_implementationWithBlock` 函数进行延伸学习。在 runtime.h 中能看到它的声明如下：

```c++
/** 
 * Creates a pointer to a function that will call the block when the method is called.
 * 创建一个指向将在调用函数时执行入参 block 的函数的指针。
 * 
 * @param block The block that implements this method. Its signature should be: method_return_type ^(id self, method_args...). 
 *  The selector is not available as a parameter to this block.
 *  The block is copied with \c Block_copy().
 *
 * 参数 block 实现返回值 IMP 的 block。它的签名应该是：method_return_type ^(id self, method_args...)。（忽略了 _cmd）
 * selector 不可用作此 block 的参数。该 block 会被使用 Block_copy() 进行复制。
 * 
 * @return The IMP that calls this block. Must be disposed of with \c imp_removeBlock.
 * 返回值是调用参数 block 的 IMP。必须当需要 block 和 IMP 解除关联时用 imp_removeBlock 处理。
 */
OBJC_EXPORT IMP _Nonnull
imp_implementationWithBlock(id _Nonnull block)
    OBJC_AVAILABLE(10.7, 4.3, 9.0, 1.0, 2.0);
```

&emsp;下面我们看下它的定义，在 objc-block-trampolines.mm 中可看到其定义如下：

```c++
IMP imp_implementationWithBlock(id block) 
{
    // Block object must be copied outside runtimeLock because it performs arbitrary work.
    block = Block_copy(block);

    // Trampolines must be initialized outside runtimeLock because it calls dlopen().
    Trampolines.Initialize();
    
    // 加锁
    mutex_locker_t lock(runtimeLock);

    return _imp_implementationWithBlockNoCopy(block);
}
```

&emsp;看完 `imp_implementationWithBlock` 函数我们可能一脸懵逼，下面我们就详细看下使用示例就明白了。

```c++

// 1⃣️ Answerer.h
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface Answerer : NSObject

@end

NS_ASSUME_NONNULL_END

// 2⃣️ Answerer.m
#import "Answerer.h"

@implementation Answerer

@end

// 3⃣️ Answerer+DynamicallyProvidedMethod.h
#import "Answerer.h"

NS_ASSUME_NONNULL_BEGIN

@interface Answerer (DynamicallyProvidedMethod)

- (int)answerForThis:(int)a andThat:(int)b;
- (void)boogityBoo:(float)c;

@end

NS_ASSUME_NONNULL_END

// 4⃣️ Answerer+DynamicallyProvidedMethod.m
#import "Answerer+DynamicallyProvidedMethod.h"

@implementation Answerer (DynamicallyProvidedMethod)

@end

// 5⃣️ main
#import <Foundation/Foundation.h>
#import "Answerer.h"
#import "Answerer+DynamicallyProvidedMethod.h"
#import <objc/runtime.h>

int main(int argc, const char * argv[]) {
    
    // 创建一个 block，block 的第一个 id 类型的参数对应我们 OC 函数默认的 self 参数
    int (^impyBlock)(id, int, int) = ^(id _self, int a, int b) {
        return a + b;
    };
    
    // 从 block 创建一个 IMP
    int (*impyFunct)(id, SEL, int, int) = (void*)imp_implementationWithBlock(impyBlock);
    
    // 调用 block
    NSLog(@"🍀🍀🍀 impyBlock: %d + %d = %d", 20, 22, impyBlock(nil, 20, 22));
    // 调用 imp
    NSLog(@"🍀🍀🍀 impyFunct: %d + %d = %d", 20, 22, impyFunct(nil, NULL, 20, 22));
    
    // 获取我们接下来要修改的类的实例
    Answerer *answerer = [[Answerer alloc] init];
    
    // 将 impyFunct 动态添加到 Answerer 中, 然后调用它
    class_addMethod([Answerer class], @selector(answerForThis:andThat:), (IMP)impyFunct, "i@:ii");
    NSLog(@"🍀🍀🍀 Method: %d + %d = %d", 20, 20, [answerer answerForThis:20 andThat:20]);
    
    SEL _sel = @selector(boogityBoo:);
    float k = 5.0;
    IMP boo = imp_implementationWithBlock(^(id _self, float c) {
        NSLog(@"🍀🍀🍀 Executing [%@ - %@%f] %f", [_self class], NSStringFromSelector(_sel), c, c * k);
    });
    class_addMethod([Answerer class], _sel, boo, "v@:f");
    
    [answerer boogityBoo:3.1415];
    
    return 0;
}

// 6⃣️ 控制台打印如下：

2021-06-12 16:30:05.558548+0800 KCObjc[1543:50241] 🍀🍀🍀 impyBlock: 20 + 22 = 42
2021-06-12 16:30:05.558636+0800 KCObjc[1543:50241] 🍀🍀🍀 impyFunct: 20 + 22 = 42
2021-06-12 16:30:05.558695+0800 KCObjc[1543:50241] 🍀🍀🍀 Method: 20 + 20 = 40
2021-06-12 16:30:05.558787+0800 KCObjc[1543:50241] 🍀🍀🍀 Executing [Answerer - boogityBoo:3.141500] 15.707500
```

&emsp;更详细内容可以参考这篇文章: [imp_implementationWithBlock()的实现机制](https://www.jianshu.com/p/c52bc284e9c7)。

## \_dyld_objc_notify_register

```c++
_dyld_objc_notify_register(&map_images, load_images, unmap_image);
```

&emsp;可看到 `_dyld_objc_notify_register` 的参数有三个，分别是 `&map_images`、`load_images`、`unmap_image`，在前面一篇 dyld 的文章中我们有提到 `_dyld_objc_notify_register` 它是用来注册一些回调函数。

&emsp;我们从 `dyld_priv.h` 中看一下 `_dyld_objc_notify_register` 函数的声明。

&emsp;`_dyld_objc_notify_register` 函数仅供 objc runtime 使用，注册当 mapped、unmapped 和 initialized objc images 时要调用的处理程序。Dyld 将使用包含 `objc-image-info` section 的 images 数组回调 `mapped` 函数。那些 dylibs 的 images 将自动增加引用计数，因此 objc 将不再需要对它们调用 `dlopen()` 以防止它们被卸载。在调用 `_dyld_objc_notify_register()` 期间，dyld 将使用已加载的 objc images 调用 `mapped` 函数。在以后的任何 `dlopen()` 调用中，dyld 还将调用 `mapped` 函数。当 dyld 在该 image 中调用 initializers 时，Dyld 将调用 `init` 函数。这是当 objc 调用该 image 中的任何 `+load` 方法时。

```c++
//
// Note: only for use by objc runtime
// Register handlers to be called when objc images are mapped, unmapped, and initialized.
// Dyld will call back the "mapped" function with an array of images that contain an objc-image-info section.
// Those images that are dylibs will have the ref-counts automatically bumped, so objc will no longer need to call dlopen() on them to keep them from being unloaded.
// During the call to _dyld_objc_notify_register(), dyld will call the "mapped" function with already loaded objc images.
// During any later dlopen() call, dyld will also call the "mapped" function.
// Dyld will call the "init" function when dyld would be called initializers in that image.
// This is when objc calls any +load methods in that image.
//
void _dyld_objc_notify_register(_dyld_objc_notify_mapped    mapped,
                                _dyld_objc_notify_init      init,
                                _dyld_objc_notify_unmapped  unmapped);
```

&emsp;下面我们从 dyld 源码中，看一下 `_dyld_objc_notify_register` 函数的定义。

```c++
void _dyld_objc_notify_register(_dyld_objc_notify_mapped    mapped,
                                _dyld_objc_notify_init      init,
                                _dyld_objc_notify_unmapped  unmapped)
{
    dyld::registerObjCNotifiers(mapped, init, unmapped);
}
```

&emsp;看到 `_dyld_objc_notify_register` 函数内部是直接调用 `dyld::registerObjCNotifiers` 函数，而 `dyld::registerObjCNotifiers` 内部则是调用 `mappd` 函数初始化所有的 images，然后调用所有初始化后的 images 的 `init` 函数。也就是当 objc 的准备工作都已经完成（objc_init  函数结尾处），此时调用 `_dyld_objc_notify_register` 告诉 dyld 可以进行类的加载，于是 dyld 进行类的加载。 

&emsp;下面我们开始分析极其重要的三个函数 `map_images`、`load_images`、`unmap_image` 函数，由于本篇篇幅过长了，我们就留到下篇文章进行分析吧！继续加油哦！🎉🎉🎉

## 参考链接
**参考链接:🔗**
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



```c++
if ( sEnv.DYLD_PRINT_OPTS )
    printOptions(argv);
if ( sEnv.DYLD_PRINT_ENV ) 
    printEnvironmentVariables(envp);
```

&emsp;此处是判断是否设置了环境变量，如果设置了，那么 xcode 就会在控制台打印相关的详细信息。（在 Edit Scheme... -> Run -> Arguments -> Environment Variables 进行添加） 

&emsp;当添加了 DYLD_PRINT_OPTS 时，会在控制台输出可执行文件的位置。
```c++
opt[0] = "/Users/hmc/Library/Developer/CoreSimulator/Devices/4E072E27-E586-4E81-A693-A02A3ED83DEC/data/Containers/Bundle/Application/ECDA091A-1610-49D2-8BC0-B41A58BC76EC/Test_ipa_Simple.app/Test_ipa_Simple"
```

&emsp;当添加了 DYLD_PRINT_ENV 时，会在控制台输出用户级别、插入的动态库、动态库的路径、模拟器的信息等等一系列的信息，由于内容过多这里就粘贴出来了。

## LLDB 常用命令

1. p po p/x p/o p/t p/d p/c
2. expression 修改参数
3. call 
4. x x/4gx x/4xg
5. image list
6. image lookup --address+地址
7. thread list
8. thread backtrace（bt）bt all
9. thread return frame variable
10. register read register read/x

## clang 

&emsp;clang:Clang 是一个 C++ 编写、基于 LLVM、发布于 LLVM BSD 许可证下的 C/C++/Objective-C/Objective-C++ 编译器。它与 GNU C 语言规范几乎完全兼容（当然，也有部分不兼容的内容， 包括编译命令选项也会有点差异），并在此基础上增加了额外的语法特性，比如 C 函数重载（通过 \_ attribute_((overloadable)) 来修饰函数)，其目标(之一)就是超越 GCC。

## iOS 内存五大分区

1. 栈区

&emsp;又称堆栈 ，由编译器自动分配释放，是用户存放程序临时创建的局部变量，也就是说我们函数括弧“{}” 中定义的变量(但不包括 static 声明的变量, static 意味着在数据段中存放变量)。除此以外, 在函数被调用时,其参数也会被压入发起调用的进程栈中, 并且待到调用结束后, 函数的返回值 也会被存放回栈中。由于 栈的后进先出特点,所以 栈 特别方便用来保存/恢复调用现场。从这个意义上讲,我们可以把 堆栈 看成一个寄存、交换临时数据的内存区。

&emsp;栈 是向低地址扩展的数据结构，是一块连续的内存区域

2. 堆区

&emsp;由程序员分配释放，分配方式类似于链表，是向高地址扩展的数据结构，是不连续的内存区域。用于存放进程运行中被动态分配的内存段，堆区的大小并不固定，可动态扩张或缩减。当进程调用 alloc 等函数分配内存时，新分配的内存就被动态添加到堆上（堆被扩张）；当利用 realse 释放内存时，被释放的内存从堆中被剔除（堆被缩减）。如果应用程序没有释放掉，操作系统会自动回收。变量通过 new、alloc、malloc、realloc 分配的内存块就存放在堆区。

3. 全局/静态区

+ 全局/静态区 是存放全局变量和静态变量的。
+ 已初始化的全局变量和静态变量存放在一块区域。
+ 未初始化的全局变量和静态变量在相邻的另一块区域。
+ 由 static 修饰的变量会成为静态变量，该变量的内存由全局/静态区在编译阶段完成分配，且仅分配一次。
+ static 可以修饰局部变量也可以修饰全局变量。
+ 全局/静态区 的内存在编译阶段完成分配，程序运行时会一直存在内存中，只有当程序结束后才会由操作系统释放。

4. 常量区

+ 常量区 是一块比较特殊的存储区，常量区里面存放的是常量，常量字符串就存放在常量区。
+ 常量区 的内存在编译阶段完成分配，程序运行时会一直存在内存中，只有当程序结束后才会由操作系统释放。

5. 代码区

&emsp;代码区 是用来存放可执行文件的操作指令（存放函数的二进制代码），其实就是存放程序的所有代码。代码区 需要防止在运行时被非法修改，所以只准许读取操作，而不允许写入（修改）操作——它是不可写的。


// duishanji4822ee@163.com
// Heiye2121

// 赢赢转使用的苹果开发者账号
// shanghaiguwan@163.com
// 1Q@w3e4r5t

// 18780334870

// 快买他使用的苹果开发者账号
// cugme9@163.com Ww115511
// qwe999 oo123-oo456-oo789 => 密保顺序对应 朋友工作父母
// 1998/8/8

// cugme9@163.com 当前密码：Cwq17150198837 绑定的手机号码：17150198837

// 富富转使用的苹果开发者账号
// feiquhui407300@126.com // 绑定手机号码：17150198837
// feiquhui407300@126.com    Ass112211
// bu3309    香港    宝马    火箭 => 密保顺序对应 朋友工作父母
// 1990/1/1

// feiquhui407300@126.com 当前密码：Cwq17150198837 绑定的手机号码：17150198837

// qwe999
// Cwq17150198837

// 好好做新买账号的原始信息
// 账号ruhan32106@21cn.com   密码Knn12355
// 密保  ooo  ppp   qqq => 密保顺序对应 朋友工作父母
// 日期1997/11/24    Nsr9455613

// ruhan32106@21cn.com 当前密码：Cwq17150198837 绑定的手机号码：18611404599

