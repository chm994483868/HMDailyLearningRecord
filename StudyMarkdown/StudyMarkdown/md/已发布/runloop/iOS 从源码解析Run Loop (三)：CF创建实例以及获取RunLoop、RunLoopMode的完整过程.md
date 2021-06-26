# iOS 从源码解析Run Loop (三)：CF创建实例以及获取RunLoop、RunLoopMode的完整过程

> &emsp;CFRunLoop.h 文件是 run loop 在 Core Foundation 下的最重要的头文件，与我们前面学习的 Cocoa Foundation 下的 NSRunLoop.h 文件相对应。NSRunLoop 的内容也正是对 \__CFRunLoop 的面向对象的简单封装，CFRunLoop.h 文件包含更多 run loop 的操作以及 run loop 涉及的部分底层数据结构的声明，\__CFRunLoop 结构则是 run loop 在 Core Foundation 下 C 语言的实现。本篇以 CFRunLoop.h 文件为入口通过 Apple 开源的 CF-1151.16 来深入学习 run loop。⛽️⛽️

## CFRunLoop Overview
&emsp;CFRunLoop 对象监视任务的输入源（sources of input），并在准备好进行处理时调度控制。输入源（input sources）的示例可能包括用户输入设备、网络连接、周期性或延时事件以及异步回调。

&emsp;run loop 可以监视三种类型的对象：sources（CFRunLoopSource）、timers（CFRunLoopTimer）和 observers（CFRunLoopObserver）。要在这些对象需要处理时接收回调，必须首先使用 `CFRunLoopAddSource`、`CFRunLoopAddTimer` 或 `CFRunLoopAddObserver` 将这些对象放入 run loop 中，以后也可以从 run loop 中删除它们（或使其 invalidate）以停止接收其回调。 

&emsp;添加到 run loop 的每个 source、timer 和 observer 必须与一个或多个 run loop modes 相关联。Modes 决定 run loop 在给定迭代期间处理哪些事件。每次 run loop 执行时，它都以特定 mode 执行。在该 mode 下，run loop 只处理与该 mode 关联的 sources、timers 和 observers 关联的事件。你可以将大多数 sources 分配给默认的 run loop mode（由 kCFRunLoopDefaultMode 常量指定），该 mode 用于在应用程序（或线程）空闲时处理事件。然而，系统定义了其它 modes，并且可以在其它 modes 下执行 run loop，以限制处理哪些 sources、timers 和 observers。因为 run loop modes 被简单地指定为字符串，所以你还可以定义自己的自定义 mode 来限制事件的处理。

&emsp;Core Foundation 定义了一种特殊的伪模式（pseudo-mode），称为 common modes，允许你将多个 mode 与给定的 source、timer 或 observer 关联起来。要指定 common modes，请在配置对象时为 mode 使用 kCFRunLoopCommonModes 常量。每个 run loop 都有自己独立的 set of common modes，默认 mode（kCFRunlopDefaultMode）始终是该 set 的成员。要向 set of common modes 添加 mode，请使用 `CFRunLoopAddCommonMode` 函数。

&emsp;每个线程只有一个 run loop。既不能创建（系统帮创建，不需要开发者自己手动创建）也不能销毁线程的 run loop（线程销毁时同时也会通过 TSD 使其对应的 run loop 销毁）。Core Foundation 会根据需要自动为你创建它。使用 `CFRunLoopGetCurrent` 获取当前线程的 run loop。调用 `CFRunLoopRun` 以默认模式运行当前线程的 run loop，直到 run loop 被 `CFRunLoopStop` 停止。也可以调用 `CFRunLoopRunInMode` 以指定的 mode 运行当前线程的 run loop 一段时间（或直到 run loop 停止）。只有在请求的模式至少有一个要监视的 source 或 timer 时，才能运行 run loop。

&emsp;Run loop 可以递归运行。你可以从任何运行循环 callout 中调用 `CFRunLoopRun` 或 `CFRunLoopRunInMode`，并在当前线程的调用堆栈上创建嵌套的运行循环激活（run loop activations）。在调用中可以运行的 modes 不受限制。你可以创建另一个在任何可用的运行循环模式下运行的运行循环激活，包括调用堆栈中已经运行的任何模式。（You can create another run loop activation running in any available run loop mode, including any modes already running higher in the call stack.）

&emsp;Cocoa 应用程序基于 CFRunLoop 来实现它们自己的高级事件循环（NSRunLoop）。Cocoa 编写应用程序时，可以将 sources、timers 和 observers 添加到它们的 run loop 对象和 modes 中。然后，对象将作为常规应用程序事件循环的一部分进行监视。使用 NSRunLoop 的 `getCFRunLoop` 方法获取相应的 CFRunLoopRef 类型。在 Carbon 应用程序中，使用 `getcfrunloopfrompeventloop` 函数。

&emsp;有关 run loop 的行为的更多信息，请参见 Threading Programming Guide 中的 Run Loops。（即上篇内容）

&emsp;以上是 CFRunLoop 文档的综述，估计对大家而言都是老生常谈的内容了。下面我们则深入源码，看看在代码层面是如何构建 run loop 体系的，即上面的一系列 run loop 概念内容与 run loop 源码实现是一一对应的，下面我们就从代码角度来理解 run loop 概念。

&emsp;Core Foundation 中的 CFRunLoop 都是 C API，提供了 run loop 相当丰富的接口，且都是线程安全的，NSRunLoop 是对 CFRunLoopRef 的封装，提供了面向对象的 API，非线程安全的。使用 NSRunLoop 的 `getCFRunLoop` 方法即可获取相应的 `CFRunLoopRef` 类型。

&emsp;下面我们对 Cocoa Foundation 和 Core Foundation 之间区别做一些拓展。

> &emsp;Core Foundation 框架 (CoreFoundation.framework) 是一组 C 语言接口，它们为 iOS 应用程序提供基本数据管理和服务功能。该框架支持进行管理的数据以及可提供的服务：群体数据类型 (数组、集合等)、程序包、字符串管理、日期和时间管理、原始数据块管理、偏好管理、URL 及数据流操作、线程和 Run Loop、端口和 soket 通讯。
> &emsp;Core Foundation 框架和 Cocoa Foundation 框架紧密相关，它们为相同功能提供接口，但 Cocoa Foundation 框架提供 Objective-C 接口。如果你将 Cocoa Foundation 对象和 Core Foundation 类型掺杂使用，则可利用两个框架之间的 “toll-free bridging”。所谓的 Toll-free bridging 是说你可以在某个框架的方法或函数同时使用 Core Foundatio 和 Cocoa Foundation 框架中的某些类型。很多数据类型支持这一特性，其中包括群体和字符串数据类型。每个框架的类和类型描述都会对某个对象是否为 toll-free bridged，应和什么对象桥接进行说明。
>
> &emsp;下面看一下Objective-C 指针与 Core Foundation 指针之间的转换规则：
>
> &emsp;ARC 仅管理 Objective-C 指针（retain、release、autorelease），不管理 Core Foundation 指针，CF 指针需要我们手动的 CFRetain 和 CFRelease 来管理（对应 MRC 时的 retain/release），CF 中没有 autorelease。
> &emsp;Cocoa Foundation 指针与 Core Foundation指针转换，需要考虑的是**所指向对象所有权的归属**。ARC 提供了 3 个修饰符来管理。
> &emsp;1. \__bridge，什么也不做，仅仅是转换。此种情况下：
> &emsp;    1.1：从 Cocoa 转换到 Core，需要手动 CFRetain，否则，Cocoa 指针释放后，传出去的指针则无效。
> &emsp;    1.2：从 Core 转换到 Cocoa，需要手动 CFRelease，否则，Cocoa 指针释放后，对象引用计数仍为 1，不会被销毁。
> &emsp;2. \__bridge_retained，转换后自动调用 CFRetain，即帮助自动解决上述 1.1 的情形。
> &emsp;（\__bridge_retained or CFBridgingRetain，ARC 把对象所有权转出，需 Core Foundation 处理。）
> &emsp;3. \__bridge_transfer，转换后自动调用 CFRelease，即帮助自动解决上述 1.2 的情形。
> &emsp;（\__bridge_transfer or CFBridgingRelease，Core Foundation 把对象所有权交给 ARC，由 ARC 自动处理。）
> &emsp;[Cocoa Foundation和 Core Foundation之间数据转换（桥接 \__bridge）](https://www.cnblogs.com/qingpeng/p/4568239.html)

## Run Loop 数据结构
&emsp;首先看到 CFRunLoop.h 文件的内容被包裹在 `CF_IMPLICIT_BRIDGING_ENABLED` 和 `CF_IMPLICIT_BRIDGING_DISABLED` 两个宏之间， 它们是一对表示隐式桥接转换的宏。
```c++
#ifndef CF_IMPLICIT_BRIDGING_ENABLED
#if __has_feature(arc_cf_code_audited)
#define CF_IMPLICIT_BRIDGING_ENABLED _Pragma("clang arc_cf_code_audited begin")
#else
#define CF_IMPLICIT_BRIDGING_ENABLED
#endif
#endif
```

```c++
#ifndef CF_IMPLICIT_BRIDGING_DISABLED
#if __has_feature(arc_cf_code_audited)
#define CF_IMPLICIT_BRIDGING_DISABLED _Pragma("clang arc_cf_code_audited end")
#else
#define CF_IMPLICIT_BRIDGING_DISABLED
#endif
#endif
```
&emsp;CF_BRIDGED_MUTABLE_TYPE 宏表示 Cocoa 和 Core Foundation 下对应的免费桥接转换类型。如 id 和 struct \__CFRunLoop，id 和 struct \__CFRunLoopSource，NSTimer 和 struct \__CFRunLoopTimer。 
```c++
#if __has_attribute(objc_bridge)
    #define CF_BRIDGED_MUTABLE_TYPE(T)    __attribute__((objc_bridge_mutable(T)))
#else
    #define CF_BRIDGED_MUTABLE_TYPE(T)
#endif
```
&emsp;然后看到几个重要的 typedef 声明。Core Founction 中 run loop 相关的数据结构有：CFRunLoopRef、CFRunLoopSourceRef、CFRunLoopObserverRef、CFRunLoopTimerRef 等等。

### CFRunLoopRef（struct \__CFRunLoop *）
&emsp;在 Core Foundation 下 \__CFRunLoop 结构是 Run Loop 对应的数据结构，对应 Cocoa 中的 NSRunLoop 类。CFRunLoopRef 则是指向 \__CFRunLoop 结构体的指针。
```c++
typedef struct __CFRunLoop * CFRunLoopRef;

struct __CFRunLoop { 
    CFRuntimeBase _base; // 所有 CF "instances" 都是从这个结构开始的
    pthread_mutex_t _lock; /* locked for accessing mode list */ 锁定以访问 mode list（CFMutableSetRef _modes）
    
    // typedef mach_port_t __CFPort;
    // 唤醒 run loop 的端口，这个是 run loop 原理的关键所在，可通过 port 来触发 CFRunLoopWakeUp 函数
    __CFPort _wakeUpPort; // used for CFRunLoopWakeUp 手动唤醒 run loop 的端口。初始化 run loop 时设置，仅用于 CFRunLoopWakeUp，CFRunLoopWakeUp 函数会向 _wakeUpPort 发送一条消息
    
    Boolean _unused; // 标记是否使用过
    
    //_perRunData 是 run loop 每次运行都会重置的一个数据结构，这里的重置是指再创建一个 _per_run_data 实例赋值给 rl->_perRunData，
    // 并不是简单的把 _perRunData 的每个成员变量重新赋值。（volatile 防止编译器自行优化，每次读值都去寄存器里面读取）
    volatile _per_run_data *_perRunData; // reset for runs of the run loop 
    
    pthread_t _pthread; // run loop 所对应的线程，二者是一一对应的。（之前在学习线程时并没有在线程的结构体中看到有 run loop 相关的字段，其实线程的 run loop 是存在了 TSD 中，当然如果是线程有获取 run loop 的话）
    uint32_t _winthread; // Windows 下记录 run loop 对象创建时所处的线程的 ID
    
    CFMutableSetRef _commonModes; // 存储字符串（而非 runloopMode 对象）的容器，对应着所有标记为 common 的 mode。
    CFMutableSetRef _commonModeItems; // 存储 modeItem 对象的容器，对应于所有标记为 common 的 mode 下的 Item（即 source、timer、observer）
    CFRunLoopModeRef _currentMode; // 当前运行的 Run Loop Mode 实例的指针（typedef struct __CFRunLoopMode *CFRunLoopModeRef）
    CFMutableSetRef _modes; // 集合，存储的是 CFRunLoopModeRef
    
    // 链表头指针，该链表保存了所有需要被 run loop 执行的 block。
    // 外部通过调用 CFRunLoopPerformBlock 函数来向链表中添加一个 block 节点。
    // run loop 会在 CFRunLoopDoBlock 时遍历该链表，逐一执行 block。
    struct _block_item *_blocks_head;
    
    // 链表尾指针，之所以有尾指针，是为了降低增加 block 节点时的时间复杂度。
    //（例如有新节点插入时，只有头节点的话，要从头节点遍历才能找到尾节点，现在已经有尾节点不需要遍历了，则时间复杂度从 O(n) 降到了 O(1）。)
    struct _block_item *_blocks_tail;
    
    // 绝对时间是自参考日期到参考日期（纪元）为 2001 年 1 月 1 日 00:00:00 的时间间隔。
    // typedef double CFTimeInterval;
    // typedef CFTimeInterval CFAbsoluteTime;
    
    CFAbsoluteTime _runTime; // 运行时间点
    CFAbsoluteTime _sleepTime; // 休眠时间点
    
    // 所有 "CF objects" 的基础 "type" 及其上的多态函数（polymorphic functions）
    // typedef const void * CFTypeRef;
    CFTypeRef _counterpart;
};
```
#### CFRuntimeBase
&emsp;所有 CF "instances" 都是从这个结构开始的，如 \__CFBoolean、\__CFString、\__CFDate、\__CFURL 等。不要直接引用这些字段--它们是供 CF 使用的，可以在没有警告的情况下添加或删除或更改格式。不能保证从一个版本到另一个版本使用此结构的二进制兼容性。
```c++
typedef struct __CFRuntimeBase {
    uintptr_t _cfisa; // 类型
    uint8_t _cfinfo[4]; // 表示 run loop 状态如：Sleeping/Deallocating 等等很多信息
#if __LP64__
    uint32_t _rc; // 引用计数
#endif
} CFRuntimeBase;

struct __CFBoolean {
    CFRuntimeBase _base;
};

struct __CFString {
    CFRuntimeBase base;
    union {    // In many cases the allocated structs are smaller than these
        struct __inline1 {
    ...
};
```
#### \_per_run_data
&emsp;重置 run loop 时用的数据结构，每次 run loop 运行后都会重置 \_perRunData 的值。
```c++
typedef struct _per_run_data {
    uint32_t a;
    uint32_t b;
    uint32_t stopped; // run loop 是否停止的标记
    uint32_t ignoreWakeUps; // run loop 是否忽略唤醒的标记
} _per_run_data;
```
#### \_block_item
&emsp;需要被 run loop 执行的 block 链表中的节点数据结构。
```c++
struct _block_item {
    struct _block_item *_next; // 指向下一个节点
    
    // typedef const void * CFTypeRef; 
    // CFString 或 CFSet 类型，也就是说一个 block 可能对应单个或多个 mode 
    CFTypeRef _mode; // CFString or CFSet
    
    void (^_block)(void); // 真正要执行的 block 本体
};
```
&emsp;上面是 CFRunLoopRef 涉及的相关数据结构，特别是其中与 mode 相关的 \_modes、\_commonModes、\_commonModeItems 三个成员变量都是  CFMutableSetRef 可变集合类型，也正对应了前面的一些结论，一个 run loop 对应多个 mode，一个 mode 下可以包含多个 modeItem（更详细的内容在下面的 \__CFRunLoopMode 结构中）。既然 run loop 包含多个 mode 那么它定可以在不同的 mode 下运行，run loop 一次只能在一个 mode 下运行，如果想要切换 mode，只能退出 run loop，然后再根据指定的 mode 运行 run loop，这样可以是使不同的 mode 下的 modeItem 相互隔离，不会相互影响。

## CF 中的 "Class" 和 Instances
&emsp;下面我们需要从 Cocoa Foundation 的 runtime 的角度来理解 Core Foundation 中的 "类" 和 "对象" 是如何构建的。

&emsp;首先在 Core Foundation 中所有的 "类对象" 都是一个静态全局 const 变量，然后所有的 "类对象" 会动态注册在 Core Foundation 的一个全局 "类表" 中，同时每个 "类" 还有一个对应的静态全局变量的索引，当 "类" 注册完成时会为其赋值，以后都用其从 "类表" 中读取 "类对象"，然后我们可以使用 "类对象" 构建对应的 "类的对象（类的实例）"。

### CFRuntimeClass
&emsp;CFRuntimeClass 是 Core Foundation 的 "基类"。`version` 字段表示不同的版本，系统有固定的 4 个枚举值。下面先看 CFRuntimeClass 的声明，然后我们再看 Core Foundation 中的 CFRunLoop 类对象，会以 CFRunLoop 类注册为例进行验证。
```c++
enum { // Version field constants
    _kCFRuntimeScannedObject =     (1UL << 0),
    _kCFRuntimeResourcefulObject = (1UL << 2),  // tells CFRuntime to make use of the reclaim field 标示 CFRuntime 可使用 __CFRuntimeClass 的 reclaim 字段
    _kCFRuntimeCustomRefCount =    (1UL << 3),  // tells CFRuntime to make use of the refcount field 标示 CFRuntime 可使用 __CFRuntimeClass 的 refcount 字段
    _kCFRuntimeRequiresAlignment = (1UL << 4),  // tells CFRuntime to make use of the requiredAlignment field 标示 CFRuntime 可使用 __CFRuntimeClass 的 requiredAlignment 字段
};

typedef struct __CFRuntimeClass {
    CFIndex version; // 位标记值，该字段不同的位代表不同的含义
    const char *className; // must be a pure ASCII string, nul-terminated 类名
    
    // typedef const void * CFTypeRef;
    void (*init)(CFTypeRef cf); // 初始化函数
    CFTypeRef (*copy)(CFAllocatorRef allocator, CFTypeRef cf); // copy 函数
    void (*finalize)(CFTypeRef cf); // 释放内存时的清理函数，同 C++ 的析构函数，同 OC 的 dealloc 函数
    Boolean (*equal)(CFTypeRef cf1, CFTypeRef cf2); // 判等
    CFHashCode (*hash)(CFTypeRef cf); // 哈希函数
    
    // 这里的两个函数都是返回 retain 后的字符串，可对应理解为 NSObject 的 description 和 debugDescription 函数，
    //（返回当前实例的描述字符串）
    CFStringRef (*copyFormattingDesc)(CFTypeRef cf, CFDictionaryRef formatOptions); // return str with retain
    CFStringRef (*copyDebugDesc)(CFTypeRef cf); // return str with retain

#define CF_RECLAIM_AVAILABLE 1
    // 当 version 字段值和 _kCFRuntimeResourcefulObject 与操作为真时，指示应使用此字段。
    void (*reclaim)(CFTypeRef cf); // Or in _kCFRuntimeResourcefulObject in the .version to indicate this field should be used

#define CF_REFCOUNT_AVAILABLE 1
    // 当 version 字段值和 _kCFRuntimeCustomRefCount 与操作为真时，指示应使用此字段。
    
    // 当 version 字段值和 _kCFRuntimeCustomRefCount 与操作为真时，此字段必须为非 NULL。
    // - 如果回调在 'op' 中传递 1，则应增加 'cf' 的引用计数并返回 0
    // - 如果回调在 'op' 中传递了 0，则应返回 'cf' 的引用计数，最多 32 位
    // - 如果回调在 'op' 中传递 -1，则应减少 'cf' 的引用计数；
    // 如果现在为零，则应清理并释放'cf'（除非该进程在 GC 下运行，并且 CF 不会为你分配内存，否则不会调用上述 finalize 回调；如果在 GC 下运行，则 finalize 应该执行拆除对象并释放对象内存）；然后返回 0
    // 记住要使用饱和算术逻辑并在 ref 计数达到 UINT32_MAX 时停止递增和递减，否则你将遇到安全漏洞
    // 请记住，引用计数的递增/递减必须线程安全/原子地完成
    // 类创建函数应使用自定义引用计数 1 创建/初始化对象
    // 不要尝试将 CFRuntimeBase 中的任何位用于你的引用计数；将其存储在 CF 对象的其他字段中。
    
    uint32_t (*refcount)(intptr_t op, CFTypeRef cf); // Or in _kCFRuntimeCustomRefCount in the .version to indicate this field should be used
    
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#define CF_REQUIRED_ALIGNMENT_AVAILABLE 1
    // 当 version 字段值和 _kCFRuntimeRequiresAlignment 与操作为真时，指示应使用此字段。
    
    // 在这种情况下，_CFRuntimeCreateInstance() 的分配器将被忽略；如果这小于系统支持的最小对齐方式，则将获得更高的对齐方式；
    // 如果这不是系统支持的对齐方式（例如，大多数系统仅支持 2 的幂，或者如果它太高），则结果（结果）将取决于 CF 或系统决定。
    uintptr_t requiredAlignment; // Or in _kCFRuntimeRequiresAlignment in the .version field to indicate this field should be used; 

} CFRuntimeClass;
```
#### \__CFRunLoopClass
&emsp;全局搜 `static const CFRuntimeClass` 可看到一众 Core Foundation 下的 "类对象"，如 \__CFArrayClass、\__CFDateClass、\__CFSetClass、\__CFStringClass 等等。\__CFRunLoopClass 是 CFRunLoop "类对象"。
```c++
static const CFRuntimeClass __CFRunLoopClass = {
    0,
    "CFRunLoop", // 类名正是 CFRunLoop
    NULL,      // init
    NULL,      // copy
    __CFRunLoopDeallocate, // CFRunLoop 类实例的销毁函数，做的事情就是 CFRunLoop 对象销毁时的清理工作，等下我们会进行分析，这个函数还挺重要的，可以帮我们理解 run loop 的结构（这里相当于忽略父类的销毁函数，然后子类重写了自己的销毁函数）
    NULL,
    NULL,
    NULL,
    __CFRunLoopCopyDescription // 对应上面的 CFStringRef (*copyDebugDesc)(CFTypeRef cf); 函数指针赋值，DEBUG 模式下 CFRuntimeClass 实例的描述函数，返回对当前的 CFRunLoop 实例进行描述的字符串，等下我们也分析下。
    
    // __CFRunLoopDeallocate 和 __CFRunLoopCopyDescription 准备留在 run loop 对象创建时再分析，这样正好和 run loop mode 和 run loop mode item 相关的内容对应起来。
};
```
&emsp;可看到 \__CFRunLoopClass 变量是一个 CFRuntimeClass 结构体实例，`className` 是 `CFRunLoop`。

### Core Foundation 注册类（\_CFRuntimeRegisterClass）
&emsp;`_CFRuntimeRegisterClass` 函数把指定的类对象注册到 Core Foundation 的静态全局 "类表" 中，全局搜索 `_CFRuntimeRegisterClass` 函数可看到每个它的调用都被包裹在 `dispatch_once` 中，保证每个 "类" 全局注册一次。我们以 \__CFRunLoopClass 类注册为例继续向下看。
```c++
#if __LLP64__
typedef unsigned long long CFTypeID;
#else
typedef unsigned long CFTypeID;
#endif

// __kCFRunLoopTypeID __kCFRunLoopModeTypeID 初始值都是 0，当执行 CFRunLoopGetTypeID 函数后，会被重新赋值
static CFTypeID __kCFRunLoopModeTypeID = _kCFRuntimeNotATypeID;
static CFTypeID __kCFRunLoopTypeID = _kCFRuntimeNotATypeID;
enum {
    _kCFRuntimeNotATypeID = 0
};

CFTypeID CFRunLoopGetTypeID(void) {
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{ 
        __kCFRunLoopTypeID = _CFRuntimeRegisterClass(&__CFRunLoopClass); 
        __kCFRunLoopModeTypeID = _CFRuntimeRegisterClass(&__CFRunLoopModeClass); 
    });
    return __kCFRunLoopTypeID;
}
```
&emsp;`CFRunLoopGetTypeID` 函数内部完成了 CFRunLoop 和 CFRunLoopMode 类的注册，\__kCFRunLoopTypeID 和 \__kCFRunLoopModeTypeID 都是静态全局变量，分别是它们在类表中的索引，后续构建 CFRunLoop 实例时，都通过 \__kCFRunLoopTypeID 去类表中查找 "类对象"。下面我们看一下 `_CFRuntimeRegisterClass` 函数，它的内部比较简单，就是把类对象的地址放在类表中，然后返回其索引，并且可看到每个类都会有一个 static CFTypeID 类型的全局变量用来记录每个类对象的地址在类表中的索引。（看到这里忽然想到了看 NSObject 时，runtime 会为类的每个成员变量准备一个静态全局值用来记录每个成员变量的偏移量，以加快成员变量的查找以及读取。）
```c++
CFTypeID _CFRuntimeRegisterClass(const CFRuntimeClass * const cls) {
    // className must be pure ASCII string, non-null
    // 一组判断
    
    // 类对象 version 字段中 _kCFRuntimeCustomRefCount 位为真但是 refcount 字段不存在则返回 _kCFRuntimeNotATypeID。（0）
    if ((cls->version & _kCFRuntimeCustomRefCount) && !cls->refcount) {
       CFLog(kCFLogLevelWarning, CFSTR("*** _CFRuntimeRegisterClass() given inconsistent class '%s'.  Program will crash soon."), cls->className);
       return _kCFRuntimeNotATypeID;
    }
    
    // 在 DEPLOYMENT_TARGET_MACOSX，macOS 下 __CFBigRuntimeFunnel 互斥锁。
    // #define CFLockInit ((pthread_mutex_t)PTHREAD_ERRORCHECK_MUTEX_INITIALIZER)
    // static CFLock_t __CFBigRuntimeFunnel = CFLockInit;
    // #define __CFLock(LP) ({ (void)pthread_mutex_lock(LP); })
    
    // 加锁
    __CFLock(&__CFBigRuntimeFunnel);
    
    // CF_PRIVATE int32_t __CFRuntimeClassTableCount = 0; // 每次用新类注册就递增
    // #define __CFMaxRuntimeTypes    65535 // 
    if (__CFMaxRuntimeTypes <= __CFRuntimeClassTableCount) {
        CFLog(kCFLogLevelWarning, CFSTR("*** CoreFoundation class table full; registration failing for class '%s'.  Program will crash soon."), cls->className);
        
        // 如果类表满了，则解锁并返回 _kCFRuntimeNotATypeID
        __CFUnlock(&__CFBigRuntimeFunnel);
        return _kCFRuntimeNotATypeID;
    }
    
    // #define __CFRuntimeClassTableSize 1024  // 1024 和 65535，那最大只能注册 1024 个类
    if (__CFRuntimeClassTableSize <= __CFRuntimeClassTableCount) {
        CFLog(kCFLogLevelWarning, CFSTR("*** CoreFoundation class table full; registration failing for class '%s'.  Program will crash soon."), cls->className);
        
        // 如果类表满了，则解锁并返回 _kCFRuntimeNotATypeID
        __CFUnlock(&__CFBigRuntimeFunnel);
        return _kCFRuntimeNotATypeID;
    }
    
    // 全局类表，是一个长度为 1024 的 CFRuntimeClass 指针的数组
    // CF_PRIVATE CFRuntimeClass * __CFRuntimeClassTable[__CFRuntimeClassTableSize] = {0};
    
    // 然后直接把类对象的地址放在数组里面，并 __CFRuntimeClassTableCount 自增
    __CFRuntimeClassTable[__CFRuntimeClassTableCount++] = (CFRuntimeClass *)cls;
    
    // typeID 就是入参类 cls 在类表中索引 
    CFTypeID typeID = __CFRuntimeClassTableCount - 1;
    
    // 解锁
    __CFUnlock(&__CFBigRuntimeFunnel);
    
    // 返回入参 cls 在类表中索引 
    return typeID;
}
```
&emsp;至此 CFRunLoop 类就注册完成了。
### \__CFAllocator
&emsp;那么下面就是创建 CFRunLoop 的类实例了，使用到了 `_CFRuntimeCreateInstance` 函数，分析该函数之前我们需要先看所有 CF 类中的默认分配器 `kCFAllocatorSystemDefault`。`kCFAllocatorSystemDefault` 是一个指向静态全局的  struct __CFAllocator 结构体实例的指针。下面先看一下 __CFAllocator 的定义，它内部包含一堆函数指针，用来存储一堆相关的处理函数的具体实现的地址，例如 malloc、free、realloc等函数，看到它的各个成员变量和 struct _malloc_zone_t 几乎相同，struct _malloc_zone_t 的定义可在 libmalloc 库中查看。
```c++
struct __CFAllocator {
    CFRuntimeBase _base; // 所有 CF "instances" 都是从这个结构开始的
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI

    // CFAllocator structure must match struct _malloc_zone_t! 
    // The first two reserved fields in struct _malloc_zone_t are for us with CFRuntimeBase.
    // CFAllocator 结构必须匹配 struct _malloc_zone_t!
    // struct _malloc_zone_t 中的前两个保留字段供我们使用 CFRuntimeBase. 
    
    // 返回块的大小；如果不在此 zone 中，则返回 0；否则，返回 0。必须快速，尤其是对于 negative answers
    size_t (*size)(struct _malloc_zone_t *zone, const void *ptr);
    
    void *(*malloc)(struct _malloc_zone_t *zone, size_t size); // malloc 函数指针
    void *(*calloc)(struct _malloc_zone_t *zone, size_t num_items, size_t size); // 与 malloc 相同，但返回的块设置为零
    void *(*valloc)(struct _malloc_zone_t *zone, size_t size); // 与 malloc 相同，但返回的块设置为零，并保证是页面对齐的
    void (*free)(struct _malloc_zone_t *zone, void *ptr); // 释放函数
    void *(*realloc)(struct _malloc_zone_t *zone, void *ptr, size_t size); // 重新分配
    void (*destroy)(struct _malloc_zone_t *zone); // 销毁 zone，所有内存被回收
    const char *zone_name; // zone 的名字

    // 可选的批处理回调；可能为 NULL
    // 给定大小（size），返回能够保持该大小的指针；返回分配的指针数（可能为 0 或小于 num_requested）
    //（可能与下面的 free 对应，申请一组指定大小的指针）
    unsigned (*batch_malloc)(struct _malloc_zone_t *zone, size_t size, void **results, unsigned num_requested); 
    // 释放 to_be_freed 中的所有指针；请注意，在此过程中，to_be_freed 可能会被覆盖
    void (*batch_free)(struct _malloc_zone_t *zone, void **to_be_freed, unsigned num_to_be_freed); 

    struct malloc_introspection_t *introspect;
    unsigned version;
    
    // 对齐的内存分配。callback 可能为 NULL。
    void *(*memalign)(struct _malloc_zone_t *zone, size_t alignment, size_t size);
    
    // 释放已知在区域（zone）中并且已知具有给定大小（size）的指针（ptr）。callback 可能为 NULL。
    void (*free_definite_size)(struct _malloc_zone_t *zone, void *ptr, size_t size);
#endif

    // typedef const struct __CFAllocator * CFAllocatorRef;
    CFAllocatorRef _allocator;
    CFAllocatorContext _context; // 上下文
};
```

#### kCFAllocatorSystemDefault
&emsp;kCFAllocatorSystemDefault 是一个全局的系统默认的分配器，内部各个函数指针类型的成员变量都有具体的函数来赋值。看到 CF 还准备了另外两个分配器 __kCFAllocatorMalloc 和 __kCFAllocatorMallocZone。下面我们看一下  kCFAllocatorSystemDefault 的各个成员变量的初值都是什么。
```c++
// typedef const struct __CFAllocator * CFAllocatorRef;
const CFAllocatorRef kCFAllocatorSystemDefault = &__kCFAllocatorSystemDefault;

#if __BIG_ENDIAN__
#define INIT_CFRUNTIME_BASE(...) {0, {0, 0, 0, 0x80}}
#else
#define INIT_CFRUNTIME_BASE(...) {0, {0x80, 0, 0, 0}}
#endif

static struct __CFAllocator __kCFAllocatorSystemDefault = {
    INIT_CFRUNTIME_BASE(), // 系统准备的默认值，用于初始化 CFRuntimeBase _base，所有 CF "instances" 都是从这个结构开始的
    
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
    
    // 下面一组固定的函数名赋值，函数都有在 CFBase.c 中有具体的实现
    __CFAllocatorCustomSize,
    __CFAllocatorCustomMalloc,
    __CFAllocatorCustomCalloc,
    __CFAllocatorCustomValloc,
    __CFAllocatorCustomFree,
    __CFAllocatorCustomRealloc,
    __CFAllocatorNullDestroy,
    
    "kCFAllocatorSystemDefault", // zone_name zone 的名字
    NULL, // 两个批处理 batch_malloc 置 NULL 
    NULL, // batch_free 置 NULL
    &__CFAllocatorZoneIntrospect, // zone 的内省函数，其内部也是一组函数指针。
    6, // version 
    NULL,
    NULL,
#endif

    NULL, // _allocator
    
    // __CFAllocatorSystemAllocate 内部实现是 malloc(size)
    // __CFAllocatorSystemReallocate 内部实现是 realloc(ptr,size)
    // __CFAllocatorSystemDeallocate 内部实现是 free(ptr)
    
    {0, NULL, NULL, NULL, NULL, __CFAllocatorSystemAllocate, __CFAllocatorSystemReallocate, __CFAllocatorSystemDeallocate, NULL}
};
```
### \_CFRuntimeCreateInstance 创建
&emsp;CFRunLoop 类对象和系统默认分配器都看完了，那下面三行是构建 CFRunLoop 类的实例。
```c++
CFRunLoopRef loop = NULL;
uint32_t size = sizeof(struct __CFRunLoop) - sizeof(CFRuntimeBase); // 为实例分配的额外字节数（即 __CFRunLoop 超出 CFRuntimeBase 部分成员变量所需的字节数），等内部具体申请空间时还会加回来。
loop = (CFRunLoopRef)_CFRuntimeCreateInstance(kCFAllocatorSystemDefault, CFRunLoopGetTypeID(), size, NULL);
```
&emsp;下面我们分析一下 `_CFRuntimeCreateInstance` 函数，这里是 `_CFRuntimeCreateInstance` 函数的声明。
```c++
// 使用给定的分配器，创建由给定 CFTypeID 指定的类的新 CF 实例，并返回它。如果分配器（kCFAllocatorSystemDefault）返回 NULL，则此函数返回 NULL。
// CFRuntimeBase 结构在返回实例的开始处初始化。
// extraBytes 是为实例分配的额外字节数（超出 CFRuntimeBase 所需的字节数）。
// 如果指定的 CFTypeID 对于 CF 运行时是未知的，则此函数返回 NULL。
// 除了基址头（CFRuntimeBase）之外，新内存的任何部分都没有初始化（例如，多余的字节不是零）。
// 使用此函数创建的所有实例只能通过使用 CFRelease() 函数来销毁——不能直接使用 CFAllocatorDeallocate() 销毁实例，即使在类的初始化或创建函数中也是如此。 为 category 参数传递 NULL。

CF_EXPORT CFTypeRef _CFRuntimeCreateInstance(CFAllocatorRef allocator, CFTypeID typeID, CFIndex extraBytes, unsigned char *category);
```
&emsp;`_CFRuntimeCreateInstance` 函数很长，下面对它进行详细分析，看似很长，其实没做多少事情，主要是为 CF 对象申请空间，然后对 CFRuntimeBase 的 `_cfinfo` 赋值，如果其 `init` 函数不为空的话，则调用，上面看到我们的 \__CFRunLoopClass 类对象仅有 `__CFRunLoopDeallocate` 和 `__CFRunLoopCopyDescription`（销毁函数和描述函数）。这也印证了声明部分的注释：“除了基址头（CFRuntimeBase）之外，新内存的任何部分都没有初始化（例如，多余的字节不是零）。”。
```c++
CFTypeRef _CFRuntimeCreateInstance(CFAllocatorRef allocator, CFTypeID typeID, CFIndex extraBytes, unsigned char *category) {
    // #define HALT do { DebugBreak(); abort(); __builtin_unreachable(); } while (0)
    // 枚举用一个 do {...} while(0) 包裹，这样构造后的宏定义不会受到大括号、分号等的影响，总是会按你期望的方式调用运行。
    // 如果入参 typeID 超过的类表的总长度，则直接中止程序运行
    if (__CFRuntimeClassTableSize <= typeID) HALT;
    
    // 断言，typeID 不等于 0。（0 表示未初始化的类型ID）
    CFAssert1(typeID != _kCFRuntimeNotATypeID, __kCFLogAssertion, "%s(): Uninitialized type id", __PRETTY_FUNCTION__);
    
    // 根据 typeID 从类表中找到类对象的指针（这里我们以 __CFRunLoopClass 类对象为例）
    CFRuntimeClass *cls = __CFRuntimeClassTable[typeID];
    
    // 如果类指针为 NULl 则返回 NULL（这里我们以 __CFRunLoopClass 类对象为例）
    if (NULL == cls) {
        return NULL;
    }
    
    // 如果 cls 的 viersion 的 _kCFRuntimeRequiresAlignment 位的值为 1，则分配器使用 kCFAllocatorSystemDefault
    if (cls->version & _kCFRuntimeRequiresAlignment) {
        allocator = kCFAllocatorSystemDefault;
    }
    
    // !! 把一个任意类型转换为 Boolean 类型。
    // 是否使用 CFRuntime 的 refcount 字段
    Boolean customRC = !!(cls->version & _kCFRuntimeCustomRefCount);
    
    // 如果 customRC 为真但是类对象的 refcount 为空，即没有提供引用计数操作函数，则返回 NULL
    if (customRC && !cls->refcount) {
        CFLog(kCFLogLevelWarning, CFSTR("*** _CFRuntimeCreateInstance() found inconsistent class '%s'."), cls->className);
        return NULL;
    }
    
    // 如果 allocator 为 NULL，则从当前线程的 TSD 中查找，如果未找到则返回 kCFAllocatorSystemDefault
    CFAllocatorRef realAllocator = (NULL == allocator) ? __CFGetDefaultAllocator() : allocator;
    
    // kCFAllocatorNull 并不是一个 NULL，是一个几乎所有成员变量的值都是 NULL 的结构体实例，其中的赋值的函数内部实现都是为空或者返回 NULL 
    if (kCFAllocatorNull == realAllocator) {
        return NULL;
    }
    
    // 判断 realAllocator 是否是 kCFAllocatorSystemDefault
    Boolean usesSystemDefaultAllocator = _CFAllocatorIsSystemDefault(realAllocator);
    
    // 如果 cls 指定了内存对齐的长度，则返回，否则返回 16
    size_t align = (cls->version & _kCFRuntimeRequiresAlignment) ? cls->requiredAlignment : 16;
    
    // 计算实例需要的总长度，当分配器不是 kCFAllocatorSystemDefault 时，增加一个指针的长度。
    //（前面减的 sizeof(CFRuntimeBase) 又加回来了）
    CFIndex size = sizeof(CFRuntimeBase) + extraBytes + (usesSystemDefaultAllocator ? 0 : sizeof(CFAllocatorRef));
    
    // 然后整体进行 16 字节对齐（先加 15 然后把后 4 bit 置为 0）
    size = (size + 0xF) & ~0xF;    // CF objects are multiples of 16 in size
    
    // CFType version 0 objects are unscanned by default since they don't have write-barriers and hard retain their innards
    // CFType version 1 objects are scanned and use hand coded write-barriers to store collectable storage within
    
    // 到这里就都判断准备完成了，下面开始申请空间
    
    CFRuntimeBase *memory = NULL;
    if (cls->version & _kCFRuntimeRequiresAlignment) {
        // #define malloc_zone_memalign(zone,align,size) malloc(size)
        // 使用自定义的对齐长度，申请空间
        memory = malloc_zone_memalign(malloc_default_zone(), align, size);
    } else {
        // 调用 CFAllocatorAllocate 申请空间。
        //（CFAllocatorAllocate 函数内部挺长的，大概是整体是为了保证内存申请成功，这里不再展开了）
        memory = (CFRuntimeBase *)CFAllocatorAllocate(allocator, size, CF_GET_COLLECTABLE_MEMORY_TYPE(cls));
    }
    
    // 如果申请失败，则返回 NULL
    if (NULL == memory) {
        return NULL;
    }
    
    // 判断是否把 memory 内存空间全部置为 0
    if (!kCFUseCollectableAllocator || !CF_IS_COLLECTABLE_ALLOCATOR(allocator) || !(CF_GET_COLLECTABLE_MEMORY_TYPE(cls) & __kCFAllocatorGCScannedMemory)) {
        memset(memory, 0, size);
    }
    
    // 大概是做最后一次 Allocation 事件统计
    if (__CFOASafe && category) {
        __CFSetLastAllocationEventName(memory, (char *)category);
    } else if (__CFOASafe) {
        __CFSetLastAllocationEventName(memory, (char *)cls->className);
    }
    
    // 判断 realAllocator 是否是 kCFAllocatorSystemDefault（以 __CFRunLoopClass 类对象为例时，不执行 if 里面的内容）
    if (!usesSystemDefaultAllocator) {
        // add space to hold allocator ref for non-standard allocators.
        // (this screws up 8 byte alignment but seems to work)
        
        // CFRetain realAllocator 后赋值给 memory。
        *(CFAllocatorRef *)((char *)memory) = (CFAllocatorRef)CFRetain(realAllocator);
        // memory 移动一个指针的距离
        memory = (CFRuntimeBase *)((char *)memory + sizeof(CFAllocatorRef));
    }
    
    // 临时变量，记录引用计数
    uint32_t rc = 0;
    
#if __LP64__
    // 在 x86_64 macOS 下，以 _CFRuntimeCreateInstance(kCFAllocatorSystemDefault, CFRunLoopGetTypeID(), size, NULL) 调用为例：
    // kCFUseCollectableAllocator 值为 NO
    if (!kCFUseCollectableAllocator || (1 && 1)) {
        // 现在 memory 是 CFRuntimeBase 指针，把其 _rc 字段置为 1
        memory->_rc = 1;
    }
    // customRC 为 NO
    if (customRC) {
        memory->_rc = 0xFFFFFFFFU;
        rc = 0xFF;
    }
#else
    if (!kCFUseCollectableAllocator || (1 && 1)) {
        rc = 1;
    }
    if (customRC) {
        rc = 0xFF;
    }
#endif
    // cfinfop 指向 _cfinfo 字段（ uint8_t _cfinfo[4]; ） 
    uint32_t *cfinfop = (uint32_t *)&(memory->_cfinfo);
    
    // 在 x86_64 macOS 下，以 _CFRuntimeCreateInstance(kCFAllocatorSystemDefault, CFRunLoopGetTypeID(), size, NULL) 调用为例：
    // rc = 0
    // customRC = NO
    // typeID 是 __kCFRunLoopTypeID（CFRunLoop 类在类表中的索引）
    // usesSystemDefaultAllocator = true
    // 0 | 0x0 | ((uint32_t)typeID << 8) | 0b1000 0000
    // 这里大概是把 CFRunLoop 类的索引放在 CFRuntimeBase 的 _cfinfo 字段中，[1, 1024] 需要两个字节，
    // _cfinfo 是长度为 4 的 uint8_t 数组，即中间两个字节用来保存类的索引。
    
    *cfinfop = (uint32_t)((rc << 24) | (customRC ? 0x800000 : 0x0) | ((uint32_t)typeID << 8) | (usesSystemDefaultAllocator ? 0x80 : 0x00));
    memory->_cfisa = 0;
    
    // 如果类对象的 init 字段不为 NULL，则调用该函数。
    // （__CFRunLoopClass 类对象 init 是 NULL）
    if (NULL != cls->init) {
        (cls->init)(memory);
    }
    
    return memory;
}
```
&emsp;至此 `_CFRuntimeCreateInstance` 函数就看完了，申请实例长度的空间，然后初始化 `CFRuntimeBase _base` 字段，`_rc` 引用计数置为 1， 然后 `_cfinfo` 字段的量中间两个字节用来保存类索引，`_cfisa` 字段是 0，并没有明确指向，然后 CFRunLoop 类对象的 `init` 函数指针又是 NULL，可能小伙伴会奇怪了，那 CFRunLoop 后面的一众成员变量什么时候初始化呢，不要着急，它们是在 `_CFRuntimeCreateInstance` 函数返回 CFRunLoop 实例后进行初始化的，下面会细讲。

&emsp;至此以 CFRunLoop 类为例，CF 下创建实例的过程就全部结束了，下面我们开始看怎么获取线程的 run loop 对象。（`static const CFRuntimeClass __CFRunLoopModeClass` 是 CFRunLoopMode 的类对象，等下我们再学习。）

&emsp;下面看两个超级重要的函数（其实是一个函数），获取主线程的 run loop 和获取当前线程（子线程）的 run loop。

## CFRunLoopGetMain/CFRunLoopGetCurrent 获取 Run Loop
&emsp;`CFRunLoopGetMain/CFRunLoopGetCurrent` 函数可分别用于获取主线程的 run loop 和获取当前线程（子线程）的 run loop。main run loop 使用一个静态变量 \__main 存储，子线程的 run loop 会保存在当前线程的 TSD 中。两者在第一次获取 run loop 时都会调用 \_CFRunLoopGet0 函数根据线程的 pthread_t 对象从静态全局变量 \__CFRunLoops（static CFMutableDictionaryRef）中获取，如果获取不到的话则新建 run loop 对象，并根据线程的 pthread_t 保存在静态全局变量 \__CFRunLoops（static CFMutableDictionaryRef）中，方便后续读取。
```c++
CFRunLoopRef CFRunLoopGetMain(void) {
    // 用于检查给定的进程是否被 fork
    CHECK_FOR_FORK();
    
    // __main 是一个静态变量，只能初始化一次，用于保存主线程关联的 run loop 对象
    static CFRunLoopRef __main = NULL; // no retain needed
    
    // 只有第一次获取 main run loop 时 __main 值为 NULL，
    // 然后从静态全局的 CFMutableDictionaryRef __CFRunLoops 中根据主线程查找 main run loop，
    // 赋值给 __main，以后再获取 main run loop，即可直接返回 __main。 
    //（主线程和 main run loop 都是全局唯一的，pthread_main_thread_np() 获取主线程）
    
    if (!__main) __main = _CFRunLoopGet0(pthread_main_thread_np()); // no CAS needed
    
    // 返回 main run loop
    return __main;
}

CFRunLoopRef CFRunLoopGetCurrent(void) {
    // 用于检查给定的进程是否被 fork
    CHECK_FOR_FORK();
    
    // 从当前线程的 TSD 中获取其 run loop，
    // 如果未找到的话则去静态全局的 CFMutableDictionaryRef __CFRunLoops 中根据 pthread_t 查找 run loop
    CFRunLoopRef rl = (CFRunLoopRef)_CFGetTSD(__CFTSDKeyRunLoop);
    if (rl) return rl;
    
    // TSD 中未找到当前线程的 run loop 的话，即是第一次获取当前线程 run loop 的情况，
    // 系统会为当前线程创建一个 run loop，会把它存入当前线程的 TSD 中同时也会存入静态全局变量 __CFRunLoops 中。
    
    // pthread_self() 获取当前线程的 pthread_t。
    
    // 去静态全局的 CFMutableDictionaryRef __CFRunLoops 中根据 pthread_t 查找当前线程的 run loop，
    // 如果找不到的话则会为当前线程进行创建 run loop。
    return _CFRunLoopGet0(pthread_self());
}
```
#### _CFRunLoopGet0
&emsp;\_CFRunLoopGet0 函数，可以通过当前线程的 pthread_t 来获取其 run loop 对象，如果没有则新创建一个 run loop 对象。创建之后，将 run loop 对象保存在静态全局 \__CFRunLoops 中，同时还会保存在当前线程的 TSD 中。
```c++
// 静态全局的 CFMutableDictionaryRef，key 是 pthread_t，value 是 CFRunLoopRef。
static CFMutableDictionaryRef __CFRunLoops = NULL;

// #if DEPLOYMENT_TARGET_MACOSX
// typedef pthread_mutex_t CFLock_t; 在 macOS 下 CFLock_t 是互斥锁

// 用于访问 __CFRunLoops 时加锁
static CFLock_t loopsLock = CFLockInit;

// should only be called by Foundation
// t==0 is a synonym for "main thread" that always works 
// t 为 0 等同于获取主线程的 run loop

// 外联函数，根据入参 pthread_t t 获取该线程的 run loop
CF_EXPORT CFRunLoopRef _CFRunLoopGet0(pthread_t t) {
    // static pthread_t kNilPthreadT = (pthread_t)0;
    // 如果 t 是 nil，则把 t 赋值为主线程
    if (pthread_equal(t, kNilPthreadT)) {
        t = pthread_main_thread_np();
    }
    
    // macOS 下 __CFLock 是互斥锁加锁
    // #define __CFLock(LP) ({ (void)pthread_mutex_lock(LP); })
    // macOS 下 __CFUnlock 是互斥锁解锁
    // #define __CFUnlock(LP) ({ (void)pthread_mutex_unlock(LP); })
    
    // 加锁
    __CFLock(&loopsLock);
    // 第一次调用时，__CFRunLoops 不存在则进行新建，并且会直接为主线程创建一个 run loop，并保存进 __CFRunLoops 中
    if (!__CFRunLoops) {
    
        // 解锁，（先加锁，如果 __CFRunLoops 为 nil，则立即进行了解锁，在多线程环境下，可能会存在多个线程同时进入到下面的 dict 创建）
        __CFUnlock(&loopsLock);
        
        // 创建 CFMutableDictionaryRef
        CFMutableDictionaryRef dict = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, NULL, &kCFTypeDictionaryValueCallBacks);
        // 根据主线程的 pthread_t 创建 run loop
        CFRunLoopRef mainLoop = __CFRunLoopCreate(pthread_main_thread_np());
    
        // #define pthreadPointer(a) a
        // 把 mainLoop 根据主线程的 pthread_t 作为 key 保存在 dict 中
        CFDictionarySetValue(dict, pthreadPointer(pthread_main_thread_np()), mainLoop);
        
        // InterlockedCompareExchangePointer 函数是进行原子性的比较和交换指针指向的值。
        // 把 dst 内存地址中存储的值与 oldp 进行比较，如果相等，则用 newp 指向的内存地址与 dst 内存地址中存储的值进行交换。
        // 返回值是 dst 内存地址中存储的值。
        // CF_EXPORT bool OSAtomicCompareAndSwapPtrBarrier(void *oldp, void *newp, void *volatile *dst) 
        // { 
        //     return oldp == InterlockedCompareExchangePointer(dst, newp, oldp);
        // }
        
        // 原子性的比较交换内存空间中值，如果 &__CFRunLoops 存储的值是 NULL 的话，把 dict 指向的内存地址与 &__CFRunLoops 内存中的值进行交换，并返回 True。
        // 当 &__CFRunLoops 内存空间中的值不是 NULL 时，不发生交换，返回 false，此时进入会 if，执行释放 dict 操作。
        if (!OSAtomicCompareAndSwapPtrBarrier(NULL, dict, (void * volatile *)&__CFRunLoops)) {
        
            // 🔒🔒
            // 在多线程环境下，假如这里有 1 2 3 三条线程入参 t 都是主线程，那么同一时间它们可能都走到这个 if 这里，只有 1 准确的把 dict 的值保存在 __CFRunLoops 中以后，
            // 剩下的 2 3 线程由于判断时 &__CFRunLoops 存储的不再是 NULL，则会进入这个 if，执行 dict 的释放操作并且销毁 dict。
            
            // 释放 dict 
            CFRelease(dict);
        }
        
        // 🔒🔒
        // 多线程环境下，对应到上面的 1 线程时，由于 __CFRunLoops 持有了 mainLoop，所以下面的 mainLoop 的释放操作，只是对应上面的创建操作做一次释放，并不会销毁 mainLoop。
        // 而在 2 3 线程下，由于 dict 被释放销毁，dict 不再持有 mainLoop 了，所以针对 2 3 线程下 mainLoop 则会被释放并销毁。
        
        // 释放 mainLoop，这里 __CFRunLoops 已经持有 mainLoop，这里的 release 并不会导致 mainLoop 对象被销毁。
        CFRelease(mainLoop);
        
        // 加锁 
        __CFLock(&loopsLock);
    }
    
    // 根据线程的 pthread_t 从 __CFRunLoops 中获取其对应的 run loop
    CFRunLoopRef loop = (CFRunLoopRef)CFDictionaryGetValue(__CFRunLoops, pthreadPointer(t));
    
    // 解锁
    __CFUnlock(&loopsLock);
    
    // 如果 loop 不存在，则新建 run loop，例如子线程第一次获取 run loop 时都会走到这里，需要为它创建 run loop。
    if (!loop) {
        // 根据线程创建 run loop
        CFRunLoopRef newLoop = __CFRunLoopCreate(t);
        // 加锁
        __CFLock(&loopsLock);
        
        // 再次判断 __CFRunLoops 中是否有线程 t 的 run loop，因为 "if (!loop)" 判断上面进行了解锁，可能在多线程的场景下前面一条线程已经创建好了该入参 t 的 run loop 并保存在 __CFRunLoops 中。
        //（开始思考时思维固定在了即使在多线程环境下，由于每条线程的入参 t 都是它们自己当前线程，所以即使多条线程同时进来，由于它们各自创建自己的 run loop，那么这里就根本不需要再二次判断 loop 是否存在，
        // 其实我们应该这样思考，假如有三条线程同时进来，然后它们的入参 t 是同一个线程的情况，就必须进行再次的 loop 是否为 nil 的判断了。）
        
        // 🔒🔒
        // 例如线程 1 2 3 同时进来，分别创建了三次 newLoop，假设线程 1 首先执行完成后并解锁，那么 __CFRunLoops 中已经存在 t 对应的 run loop 了，
        // 此时线程 2 再走到这里的时候，取得的 loop 便是有值的了，这时候不再需要存入 __CFRunLoops 中了，只需要继续往下走释放并销毁 newLoop 就好了，线程 3 也是同样。
        
        // 这里还有一点时，为什么要先创建 newLoop 后加锁呢，这样在多线程的情况下会存在创建多个 newLoop 的情况，如果把 newLoop 的创建放在下面的 "if (!loop)" 内部的话，
        // 需要把 "CFRelease(newLoop);" 也提到这个 "if (!loop)" 内部去，这样就会导致在 __CFLock(&loopsLock)/__CFUnlock(&loopsLock) 中间插入一条 "CFRelease(newLoop);"，
        // 可是即使是这样，newLoop 也只是会执行 release 操作，但是并不会执行销毁操作呀，那么什么时候 CFRunLoopDeallocate 会执行呢 ？
        
        loop = (CFRunLoopRef)CFDictionaryGetValue(__CFRunLoops, pthreadPointer(t));
        if (!loop) {
            // 把 newLoop 根据线程 t 保存在 __CFRunLoops 中
            CFDictionarySetValue(__CFRunLoops, pthreadPointer(t), newLoop);
            // 赋值给 loop
            loop = newLoop;
        }
        
        // don't release run loops inside the loopsLock, because CFRunLoopDeallocate may end up taking it.
        // 不要在 loopsLock 内部释放运行循环，因为 CFRunLoopDeallocate 最终可能会占用它
        
        // 解锁
        __CFUnlock(&loopsLock);
        
        // 放入 __CFRunLoops 时，__CFRunLoops 会持有 newLoop，这里的 release 只是对应 __CFRunLoopCreate(t) 时的引用计数 + 1
        CFRelease(newLoop);
    }
    
    // 这里判断入参线程 t 是否就是当前线程，如果是的话则可以直接把 loop 保存在当前线程的 TSD 中。
    if (pthread_equal(t, pthread_self())) {
        // loop 存入 TSD 中，方便 CFRunLoopGetCurrent 函数直接从当前线程的 TSD 读取到线程的 run loop 就可以返回了，不用再调用 _CFRunLoopGet0 函数。
        _CFSetTSD(__CFTSDKeyRunLoop, (void *)loop, NULL);
        
        // 从 TSD 中根据 __CFTSDKeyRunLoopCntr 取 run loop 的退出函数（__CFFinalizeRunLoop 函数，每个线程退出时都会调用，做一些清理和释放工作，
        // 最重要的如果线程的 run loop 存在的话会把其从 __CFRunLoops 中移除并进行释放等工作，后面我们详细分析 __CFFinalizeRunLoop 函数）
        if (0 == _CFGetTSD(__CFTSDKeyRunLoopCntr)) {
            // 注册一个回调 __CFFinalizeRunLoop，当线程销毁时，顺便销毁其 run loop 对象。
            _CFSetTSD(__CFTSDKeyRunLoopCntr, (void *)(PTHREAD_DESTRUCTOR_ITERATIONS-1), (void (*)(void *))__CFFinalizeRunLoop);
        }
    }
    return loop;
}
```
&emsp;Note:感觉有一点理解是以前没有过的（大概就是日常单线程写多了，现在专注看多线程的内容有了一些新的认识），日常对函数调用的惯性使然，当函数内部需要操作什么数据时我们就把什么数据作为参数传入（全局变量以及和函数调用时同处一个作用域的变量除外），由于线程和它的 run loop 对象一一对应，那么当一个函数内部需要操作当前线程的 run loop 对象时不需要通过参数传入，可以直接通过 `CFRunLoopRef rl = (CFRunLoopRef)CFDictionaryGetValue(__CFRunLoops, pthreadPointer(pthread_self()));` 取得当前线程的 run loop 对象（这里其实还是一个 `__CFRunLoops` 全局变量读值😂），还有一点，同一个函数在不同的线程执行那么在函数内部能直接通过 `pthread_self()` 函数获取当前线程的线程对象。`pthread_self` 一个不需要任何参数的函数只要执行一下就能获取当前线程的线程对象，感觉还挺有意思的！（对操作系统的内容几乎一无所知，觉得后续需要补充一下）

&emsp;下面我们看一下 `__CFRunLoopCreate` 创建 run loop 的函数。
##### \__CFRunLoopCreate
&emsp;`__CFRunLoopCreate` 函数入参是一个线程，返回值是一个 run loop，正如其名，完成的功能就是为线程创建 run loop。
```c++
static CFRunLoopRef __CFRunLoopCreate(pthread_t t) {
    CFRunLoopRef loop = NULL;
    CFRunLoopModeRef rlm;
    
    // sizeof(CFRuntimeBase) 在 x86_64 macOS 下是 16，CFRuntimeBase 是所有 CF 类都包含的字段。
    
    // 减去 sizeof(CFRuntimeBase) 得到的 size 是为了计算出 extraBytes 的大小，即计算出 CFRuntimeBase 之外的扩展空间的大小，
    // 因为 CFRuntimeBase 是所有 CF 类都包含的字段，__CFRunLoop 结构体中是包含 CFRuntimeBase _base 成员变量的，
    // 所以要减去 sizeof(CFRuntimeBase) 得到 __CFRunLoop 结构体中剩余成员变量占用的空间。
    uint32_t size = sizeof(struct __CFRunLoop) - sizeof(CFRuntimeBase);
    
    // CFRunLoopGetTypeID() 内部调用 dispatch_once 在 CF 运行时中注册两个新类 run loop（CFRunLoop）和 run loop mode（CFRunLoopMode），并返回 __kCFRunLoopTypeID。
    // 然后 _CFRuntimeCreateInstance 函数根据 __kCFRunLoopTypeID 构建一个 run loop 对象并返回赋值给 loop。
    //（注册新类是把全局的 run loop "类对象" 和 run loop mode "类对象" 放进全局的类表 __CFRuntimeClassTable 中，
    // 其中 __kCFRunLoopTypeID 实际值是 run loop "类对象" 在 __CFRuntimeClassTable 类表中的索引。）
    loop = (CFRunLoopRef)_CFRuntimeCreateInstance(kCFAllocatorSystemDefault, CFRunLoopGetTypeID(), size, NULL);
    
    // 如果创建失败，则返回 NULL。
    if (NULL == loop) {
        return NULL;
    }
    
    // 初始化 loop 的 _perRunData。
    (void)__CFRunLoopPushPerRunData(loop);
    
    // 初始化 loop 的 pthread_mutex_t _lock 为一个互斥递归锁。
    //（__CFRunLoopLockInit 内部使用的 PTHREAD_MUTEX_RECURSIVE 表示递归锁，允许同一个线程对同一锁加锁多次，且需要对应次数的解锁操作）
    __CFRunLoopLockInit(&loop->_lock);
    
    // 给 loop 的 _wakeUpPort 唤醒端口赋值，这里 loop 会持有 port 对象，在合适的时候需要进行一次释放。
    loop->_wakeUpPort = __CFPortAllocate();
    if (CFPORT_NULL == loop->_wakeUpPort) HALT; // 这里超级重要，_wakeUpPort 必须创建成功并且直接关乎到 run loop 的唤醒，如果创建失败的话则直接 crash。
    
    // 设置 loop 的 _perRunData->ignoreWakeUps 为 0x57414B45，
    // 前面 __CFRunLoopPushPerRunData 初始化时 _perRunData->ignoreWakeUps 的值是 0x00000000。
    // 0x57414B45 表示忽略，0x00000000 表示不忽略。
    __CFRunLoopSetIgnoreWakeUps(loop);
    
    // _commonModes 是 CFMutableSetRef 类型，CFSetCreateMutable 是为其申请空间，这里 loop 会持有返回的 CFSet 实例，在合适的时候 loop 要对其进行一次释放。 
    loop->_commonModes = CFSetCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeSetCallBacks);
    // 把 kCFRunLoopDefaultMode 添加到 loop 的 _commonModes 集合中，
    // 同时也验证了 _commonModes 中存放的是 mode 对应的字符串（"kCFRunLoopDefaultMode"）并不是 CFRunLoopModeRef，
    // 同时也验证了 loop 创建时就会直接把默认的 mode 标记为 common。
    CFSetAddValue(loop->_commonModes, kCFRunLoopDefaultMode);
    
    // loop 的其它一些成员变量赋初值为 NULL
    loop->_commonModeItems = NULL;
    loop->_currentMode = NULL;
    // 同上面的 _commonModes，也是为 _modes 申请空间，这里 loop 会持有返回的 CFSet 实例，在合适的时候 loop 要对其进行一次释放。
    loop->_modes = CFSetCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeSetCallBacks);
    loop->_blocks_head = NULL;
    loop->_blocks_tail = NULL;
    loop->_counterpart = NULL;
    
    // 把 pthread_t t 赋值给 loop 的 _pthread 成员变量。
    loop->_pthread = t;
    
#if DEPLOYMENT_TARGET_WINDOWS
    // Windows 下会获取当前线程的 ID 赋值给 loop 的 _winthread
    loop->_winthread = GetCurrentThreadId();
#else
    loop->_winthread = 0;
#endif

    // __CFRunLoopFindMode(CFRunLoopRef rl, CFStringRef modeName, Boolean create)
    // __CFRunLoopFindMode 函数根据 modeName 从 rl 的 _modes 中找到其对应的 CFRunLoopModeRef，如果找到的话则加锁并返回，
    // 如果未找到，并且 create 为真的话，则新建 __CFRunLoopMode 加锁并返回，如果 create 为假的话，则返回 NULL。
    // 具体拆开讲解准备留在 CFRunLoopModeRef 章节。
    
    // 此处是构建一个 _name 是 kCFRunLoopDefaultMode 的 mode 赋值给 rlm，最后会把 rlm 添加到 loop 的 _modes 中。
    
    // 函数返回时会调用 __CFRunLoopModeLock(rlm) 进行加锁，然后对应下面 if 中的 __CFRunLoopModeUnlock(rlm) 解锁。
    //（内部加锁是：pthread_mutex_lock(&(rlm->_lock))，解锁是：pthread_mutex_unlock(&(rlm->_lock))。）
    
    //（关于 rlm 的 _portSet：）
    //（会把 loop 的 _wakeUpPort 添加到 rlm 的 _portSet 中）
    // (rlm->_timerPort = mk_timer_create()，然后把 _timerPort 也添加到 rlm 的 _portSet 中)
    //（还有一个 queuePort 也添加到 rlm 的 _portSet 中）
    
    rlm = __CFRunLoopFindMode(loop, kCFRunLoopDefaultMode, true);
    if (NULL != rlm) __CFRunLoopModeUnlock(rlm);
    
    return loop;
}
```
&emsp;`__CFRunLoopCreate` 函数整体看下来涉及的细节和函数调用还挺多的。首先是 `_CFRuntimeCreateInstance` 函数调用中的参数：`CFRunLoopGetTypeID()` 该函数内部使用全局只会进行一次的在 Core Foundation 运行时中为我们注册两个类 run loop（CFRunLoop）和 run loop mode（CFRunLoopMode），并返回 `__kCFRunLoopTypeID` 指定 `_CFRuntimeCreateInstance` 函数构建的是 CFRunLoop 的实例。

&emsp;然后 run loop 的实例 loop 创建好以后是对 loop 的一些成员变量进行初始化。

+ 初始化 loop 的 `_perRunData`。
+ 初始化 loop 的 `pthread_mutex_t _lock`，`_lock` 的初始时属性用的 `PTHREAD_MUTEX_RECURSIVE`，即 `_lock` 为一个互斥递归锁。
+ 给 loop 的 `_wakeUpPort`（唤醒端口）赋初值（`__CFPortAllocate()`）。
+ 设置 loop 的 `_perRunData->ignoreWakeUps` 为 `0x57414B45`，前面 `__CFRunLoopPushPerRunData` 初始化时 `_perRunData->ignoreWakeUps` 的值是 `0x00000000`。
+ 初始化 loop 的 `_commonModes` 并把默认 mode 的字符串（"kCFRunLoopDefaultMode"）添加到 `_commonModes` 中，即把默认 mode 标记为 common。
+ 初始化 loop 的 `_modes`，并把构建好的 `CFRunLoopModeRef rlm` 添加到 `_modes` 中。
+ 把 `pthread_t t` 赋值给 loop 的 `_pthread` 成员变量。（`Windows` 下会获取当前线程的 ID 赋值给 loop 的 `_winthread`）

##### \__CFRunLoopPushPerRunData
&emsp;`__CFRunLoopPushPerRunData` 初始化 run loop 的 `_perRunData`，并返回 `_perRunData` 的旧值。每次 run loop 运行会重置 `_perRunData`（重新为 \_perRunData 创建 \_per_run_data 实例）。`_perRunData->stopped` 表示是否停止的标记，停止则设置为 `0x53544F50`，运行则为 `0x0`。`_perRunData->ignoreWakeUps` 表示是否忽略唤醒的标记，忽略则设置为 `0x57414B45`，不忽略则为`0x0`。
```c++
// (void)__CFRunLoopPushPerRunData(loop);
CF_INLINE volatile _per_run_data *__CFRunLoopPushPerRunData(CFRunLoopRef rl) {
    // previous 记录旧值
    volatile _per_run_data *previous = rl->_perRunData;
    // 为入参 run loop 新建 _perRunData
    rl->_perRunData = (volatile _per_run_data *)CFAllocatorAllocate(kCFAllocatorSystemDefault, sizeof(_per_run_data), 0); // 创建 _per_run_data 实例
    
    rl->_perRunData->a = 0x4346524C;
    rl->_perRunData->b = 0x4346524C; // 'CFRL'
    rl->_perRunData->stopped = 0x00000000; 
    rl->_perRunData->ignoreWakeUps = 0x00000000; // 在 __CFRunLoopCreate 函数中，接下来会设置 loop 的 _perRunData->ignoreWakeUps 为 0x57414B45
    
    // 返回旧值
    return previous;
}
```
&emsp;`__CFRunLoopPushPerRunData` 源码下面是一组 `_perRunData` 相关的函数。
```c++
// 把 previous（旧值）赋值给 rl 的 _perRunData
CF_INLINE void __CFRunLoopPopPerRunData(CFRunLoopRef rl, volatile _per_run_data *previous) {
    // 如果当前 rl 的 _perRunData 有值，则销毁它。
    if (rl->_perRunData) CFAllocatorDeallocate(kCFAllocatorSystemDefault, (void *)rl->_perRunData);
    
    // 把 previous 赋值给 rl->_perRunData。
    rl->_perRunData = previous;
}

// 判断 run loop 是否已停止，如果 stopped 的值是 0x00000000，则返回 false，表示没有停止，
// 若是其它值则表示停止，即 stopped 的值非零表示停止，零表示正在运行。
CF_INLINE Boolean __CFRunLoopIsStopped(CFRunLoopRef rl) {
    return (rl->_perRunData->stopped) ? true : false;
}

// 设置 run loop 已停止，直接把 stopped 赋值为 0x53544F50。
CF_INLINE void __CFRunLoopSetStopped(CFRunLoopRef rl) {
    rl->_perRunData->stopped = 0x53544F50;    // 'STOP'
}

// 未停止
CF_INLINE void __CFRunLoopUnsetStopped(CFRunLoopRef rl) {
    rl->_perRunData->stopped = 0x0;
}

// 判断是否忽略 WakeUp，ignoreWakeUps 非零表示忽略，零表示不忽略。
CF_INLINE Boolean __CFRunLoopIsIgnoringWakeUps(CFRunLoopRef rl) {
    return (rl->_perRunData->ignoreWakeUps) ? true : false;    
}

// 直接把 ignoreWakeUps 赋值为 0x57414B45，非零表示忽略。
CF_INLINE void __CFRunLoopSetIgnoreWakeUps(CFRunLoopRef rl) {
    rl->_perRunData->ignoreWakeUps = 0x57414B45; // 'WAKE'
}

// 0x0
CF_INLINE void __CFRunLoopUnsetIgnoreWakeUps(CFRunLoopRef rl) {
    rl->_perRunData->ignoreWakeUps = 0x0;
}
```
#### CHECK_FOR_FORK
&emsp;Forking is a system call where a process creates a copy of itself. CHECK_FOR_FORK is a boolean value in the code which checks whether the given process was forked.（Forking 是系统调用，其中进程创建其自身的副本。 CHECK_FOR_FORK 是代码中的布尔值，用于检查给定的进程是否被分叉。）[What's the meaning of CHECK_FOR_FORK?](https://stackoverflow.com/questions/47260563/whats-the-meaning-of-check-for-fork)
```c++
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI
extern uint8_t __CF120293;
extern uint8_t __CF120290;
extern void __THE_PROCESS_HAS_FORKED_AND_YOU_CANNOT_USE_THIS_COREFOUNDATION_FUNCTIONALITY___YOU_MUST_EXEC__(void);
#define CHECK_FOR_FORK() do { __CF120290 = true; if (__CF120293) __THE_PROCESS_HAS_FORKED_AND_YOU_CANNOT_USE_THIS_COREFOUNDATION_FUNCTIONALITY___YOU_MUST_EXEC__(); } while (0)
#endif

#if !defined(CHECK_FOR_FORK)
#define CHECK_FOR_FORK() do { } while (0)
#endif

CF_PRIVATE void __THE_PROCESS_HAS_FORKED_AND_YOU_CANNOT_USE_THIS_COREFOUNDATION_FUNCTIONALITY___YOU_MUST_EXEC__(void) {
    write(2, EXEC_WARNING_STRING_1, sizeof(EXEC_WARNING_STRING_1) - 1);
    write(2, EXEC_WARNING_STRING_2, sizeof(EXEC_WARNING_STRING_2) - 1);
//    HALT;
}
```
&emsp;看到这里 run loop 创建的相关的内容就看完了，上面的我们预留的两个函数 \__CFRunLoopDeallocate 是 run loop 的销毁函数， \__CFRunLoopCopyDescription 是返回 run loop 对象的描述，下面我们来一起看一下。

### \__CFRunLoopDeallocate
```c++
static void __CFRunLoopDeallocate(CFTypeRef cf) {
    // 指针转换为 CFRunLoopRef 类型
    CFRunLoopRef rl = (CFRunLoopRef)cf;

    // 如果是想要错误销毁主线程的 run loop 则直接中止程序运行
    if (_CFRunLoopGet0b(pthread_main_thread_np()) == cf) HALT;

    /* We try to keep the run loop in a valid state as long as possible, since sources may have non-retained references to the run loop.
       Another reason is that we don't want to lock the run loop for callback reasons, if we can get away without that.  We start by eliminating the sources, since they are the most likely to call back into the run loop during their "cancellation". Common mode items will be removed from the mode indirectly by the following three lines. */
       
    // 我们尝试将运行循环尽可能长时间地保持在有效状态，因为 sources 可能对运行循环具有非保留（non-retained references）的引用。
    // 另一个原因是，如果没有回调，我们不想出于回调原因而锁定运行循环。
    // 我们从消除源开始，因为它们最有可能在其 "cancellation" 期间回调到运行循环中。
    // Common mode items 将从 mode 中移除，通过以下三行间接模式。
    
    // #define CF_INFO_BITS (!!(__CF_BIG_ENDIAN__) * 3) // 0 
    // #define __CF_BIG_ENDIAN__ 0 // macOS 下是小端模式
    // __CFBitfieldSetValue(((CFRuntimeBase *)rl)->_cfinfo[CF_INFO_BITS], 2, 2, 1) // 
    // #define __CFBitfieldSetValue(V, N1, N2, X)   ((V) = ((V) & ~__CFBitfieldMask(N1, N2)) | (((X) << (N2)) & __CFBitfieldMask(N1, N2)))
    // #define __CFBitfieldMask(N1, N2)   ((((UInt32)~0UL) << (31UL - (N1) + (N2))) >> (31UL - N1))
    
    // 把 rl 标记为正在销毁
    __CFRunLoopSetDeallocating(rl);
    
    // 如果 rl 的 _modes 不为 NULL，则
    if (NULL != rl->_modes) {
        // 删除引用
        CFSetApplyFunction(rl->_modes, (__CFRunLoopCleanseSources), rl); // remove references to rl
        // 销毁 sources
        CFSetApplyFunction(rl->_modes, (__CFRunLoopDeallocateSources), rl);
        // 销毁 run loop observer 
        CFSetApplyFunction(rl->_modes, (__CFRunLoopDeallocateObservers), rl);
        // 销毁 timer 
        CFSetApplyFunction(rl->_modes, (__CFRunLoopDeallocateTimers), rl);
    }
    
    // 加锁
    __CFRunLoopLock(rl);
    
    // 取得 rl 的 block 链表的头节点
    struct _block_item *item = rl->_blocks_head;
    
    // 遍历释放 block 持有的 mode 以及 block 本身
    while (item) {
        struct _block_item *curr = item;
        item = item->_next;
        
        // 释放 block 持有的 _mode
        CFRelease(curr->_mode);
        // 释放 block
        Block_release(curr->_block);
        // 释放内存空间
        free(curr);
    }
    
    // CF 下的 release
    // rl 会持有 _commonModeItems、_commonModes、_modes 所以这里要进行一次释放
    if (NULL != rl->_commonModeItems) {
        CFRelease(rl->_commonModeItems);
    }
    
    // 释放 _commonModes
    if (NULL != rl->_commonModes) {
        CFRelease(rl->_commonModes);
    }
    
    // 释放 _modes
    if (NULL != rl->_modes) {
        CFRelease(rl->_modes);
    }
    
    // 释放唤醒端口
    __CFPortFree(rl->_wakeUpPort);
    // #define CFPORT_NULL MACH_PORT_NULL
    // 并置为 MACH_PORT_NULL
    rl->_wakeUpPort = CFPORT_NULL;
    
    // 释放 rl 当前的 _perRunData，并把 _perRunData 置为 NULL
    __CFRunLoopPopPerRunData(rl, NULL);
    
    // 解锁
    __CFRunLoopUnlock(rl);
    
    // 销毁 rl 中的互斥递归锁
    pthread_mutex_destroy(&rl->_lock);
    
    // sizeof(CFRuntimeBase) 是 16。
    // 把 cf 内存空间除开头 16 个字节外，后面的每个自己都置为 0x8C
    memset((char *)cf + sizeof(CFRuntimeBase), 0x8C, sizeof(struct __CFRunLoop) - sizeof(CFRuntimeBase));
}
```
### \__CFRunLoopCopyDescription
```c++
static CFStringRef __CFRunLoopCopyDescription(CFTypeRef cf) {
    // 指针转换为 CFRunLoopRef 类型
    CFRunLoopRef rl = (CFRunLoopRef)cf;
    
    // 创建一个 CFMutableStringRef，__CFRunLoopCopyDescription 函数的调用者需要释放返回值，否则会发生内存泄漏
    CFMutableStringRef result;
    result = CFStringCreateMutable(kCFAllocatorSystemDefault, 0);
    
#if DEPLOYMENT_TARGET_WINDOWS
    CFStringAppendFormat(result, NULL, CFSTR("<CFRunLoop %p [%p]>{wakeup port = 0x%x, stopped = %s, ignoreWakeUps = %s, \ncurrent mode = %@,\n"), cf, CFGetAllocator(cf), rl->_wakeUpPort, __CFRunLoopIsStopped(rl) ? "true" : "false", __CFRunLoopIsIgnoringWakeUps(rl) ? "true" : "false", rl->_currentMode ? rl->_currentMode->_name : CFSTR("(none)"));
#else
    // 拼接字符串
    // cf 的地址、cf 的分配器地址、是否停止、是否忽略唤醒、当前 mode 的名字
    CFStringAppendFormat(result, NULL, CFSTR("<CFRunLoop %p [%p]>{wakeup port = 0x%x, stopped = %s, ignoreWakeUps = %s, \ncurrent mode = %@,\n"), cf, CFGetAllocator(cf), rl->_wakeUpPort, __CFRunLoopIsStopped(rl) ? "true" : "false", __CFRunLoopIsIgnoringWakeUps(rl) ? "true" : "false", rl->_currentMode ? rl->_currentMode->_name : CFSTR("(none)"));
#endif

    // 再接着拼接 _commonModes、_commonModeItems、_modes
    CFStringAppendFormat(result, NULL, CFSTR("common modes = %@,\ncommon mode items = %@,\nmodes = %@}\n"), rl->_commonModes, rl->_commonModeItems, rl->_modes);
    return result;
}
```
&emsp;看到这里 run loop 创建以及销毁的相关的内容就看完了，其中比较重要的 `__CFRunLoopFindMode` 函数，留在下 CFRunLoopModeRef 节再分析。

### CFRunLoopModeRef（struct \__CFRunLoopMode *）
&emsp;每次 run loop 开始 run 的时候，都必须指定一个 mode，称为 run loop mode（运行循环模式）。mode 指定了在这次 run 中，run loop 可以处理的任务，对于不属于当前 mode 的任务，则需要切换 run loop 至对应 mode 下，再重新调用 run 方法，才能够被处理，这样也保证了不同 mode 的 source/timer/observer 互不影响，使不同 mode 下的数据做到相互隔离的。下面我们就从代码层面看下 mode 的数据结构、CFRunLoopMode 类对象及一些相关的函数。

&emsp;`__CFRunLoopModeClass` run loop mode 的 "类对象"，
```c++
static const CFRuntimeClass __CFRunLoopModeClass = {
    0,
    "CFRunLoopMode",
    NULL,      // init
    NULL,      // copy
    __CFRunLoopModeDeallocate, // 销毁函数
    __CFRunLoopModeEqual, // 比较函数
    __CFRunLoopModeHash, // 哈希函数（很重要，从 run loop 的 _modes 哈希表中查找 run loop mode ）
    NULL, // 
    __CFRunLoopModeCopyDescription // 描述函数
};
```
&emsp;`__CFRunLoopModeHash` 哈希函数，取 run loop mode 的名字的哈希值。
```c++
static CFHashCode __CFRunLoopModeHash(CFTypeRef cf) {
    CFRunLoopModeRef rlm = (CFRunLoopModeRef)cf;
    return CFHash(rlm->_name);
}
```
&emsp;`__CFRunLoopModeEqual` run loop mode 的判等函数也是对其 `_name` 进行比较。
```c++
static Boolean __CFRunLoopModeEqual(CFTypeRef cf1, CFTypeRef cf2) {
    CFRunLoopModeRef rlm1 = (CFRunLoopModeRef)cf1;
    CFRunLoopModeRef rlm2 = (CFRunLoopModeRef)cf2;
    return CFEqual(rlm1->_name, rlm2->_name);
}
```
&emsp;`__CFRunLoopMode` 定义。
```c++
typedef struct __CFRunLoopMode *CFRunLoopModeRef;

struct __CFRunLoopMode {
    CFRuntimeBase _base; // 所有 CF "instances" 都是从这个结构开始的
    pthread_mutex_t _lock; /* must have the run loop locked before locking this */ 必须在锁定之前将 run loop 锁定，即加锁前需要 run loop 对象先加锁
    CFStringRef _name; // mode 的一个字符串名称
    Boolean _stopped; // 标记了 run loop 的运行状态，实际上并非如此简单，还有前面的 _per_run_data。
    char _padding[3]; 
    
    // _sources0、_sources1、_observers、_timers 都是集合类型，里面都是 mode item，即一个 mode 包含多个 mode item
    CFMutableSetRef _sources0; // sources0 事件集合（之所以用集合是为了保证每个元素唯一）
    CFMutableSetRef _sources1; // sources1 事件集合
    
    CFMutableArrayRef _observers; // run loop observer 观察者数组
    CFMutableArrayRef _timers; // 计时器数组
    
    CFMutableDictionaryRef _portToV1SourceMap; // 存储了 Source1 的 port 与 source 的对应关系，key 是 mach_port_t，value 是 CFRunLoopSourceRef
    __CFPortSet _portSet; // 保存所有需要监听的 port，比如 _wakeUpPort，_timerPort，queuePort 都保存在这个集合中
    CFIndex _observerMask; // 添加 obsever 时设置 _observerMask 为 observer 的 _activities（CFRunLoopActivity 状态）
    
    // DEPLOYMENT_TARGET_MACOSX 表示部署在 maxOS 下
    // #if DEPLOYMENT_TARGET_MACOSX
    //  #define USE_DISPATCH_SOURCE_FOR_TIMERS 1
    //  #define USE_MK_TIMER_TOO 1
    // #else
    //  #define USE_DISPATCH_SOURCE_FOR_TIMERS 0
    //  #define USE_MK_TIMER_TOO 1
    // #endif
    
    // 在 maxOS 下 USE_DISPATCH_SOURCE_FOR_TIMERS 和 USE_MK_TIMER_TOO 都为真。
    
#if USE_DISPATCH_SOURCE_FOR_TIMERS
    // 使用 dispatch_source 表示 timer
    dispatch_source_t _timerSource; // GCD 计时器
    dispatch_queue_t _queue; // 队列
    Boolean _timerFired; // set to true by the source when a timer has fired 计时器触发时由 source 设置为 true，在 _timerSource 的回调事件中值会置为 true，即标记为 timer 被触发。
    Boolean _dispatchTimerArmed;
#endif

#if USE_MK_TIMER_TOO
    // 使用 MK 表示 timer 
    mach_port_t _timerPort; // MK_TIMER 的 port
    Boolean _mkTimerArmed;
#endif

#if DEPLOYMENT_TARGET_WINDOWS
    DWORD _msgQMask;
    void (*_msgPump)(void);
#endif

    // timer 软临界点
    uint64_t _timerSoftDeadline; /* TSR */
    // timer 硬临界点
    uint64_t _timerHardDeadline; /* TSR */
};
```
&emsp;看完了 run loop mode 的数据结构定义，那么我们分析下 `__CFRunLoopFindMode` 函数，正是通过它得到一个 run loop mode 对象。通常我们接触到的 run loop mode 只有 kCFRunLoopDefaultMode 和 UITrackingRunLoopMode，前面看到 run loop 创建时会通过 `__CFRunLoopFindMode` 函数取得一个默认 mode（kCFRunLoopDefaultMode），并把它添加到 run loop 对象的 \_modes 中。

#### \__CFRunLoopFindMode
&emsp;`__CFRunLoopFindMode` 函数根据 modeName 从 rl 的 _modes 中找到其对应的 CFRunLoopModeRef，如果找到的话则加锁并返回。如果未找到，并且 create 为真的话，则新建 \_\_CFRunLoopMode 加锁并返回，如果 create 为假的话，则返回 NULL。
```c++
static CFRunLoopModeRef __CFRunLoopFindMode(CFRunLoopRef rl, CFStringRef modeName, Boolean create) {
    // 用于检查给定的进程是否被分叉
    CHECK_FOR_FORK();
    
    // struct __CFRunLoopMode 结构体指针
    CFRunLoopModeRef rlm;
    
    // 创建一个 struct __CFRunLoopMode 结构体实例，
    // 并调用 memset 把 srlm 内存空间全部置为 0。
    struct __CFRunLoopMode srlm;
    memset(&srlm, 0, sizeof(srlm));
    
    // __kCFRunLoopModeTypeID 现在正是表示 CFRunLoopMode 类，实际值是 run loop mode 类在全局类表 __CFRuntimeClassTable 中的索引。
    
    // 前面 __CFRunLoopCreate 函数内部会调用 CFRunLoopGetTypeID() 函数，
    // 其内部是全局执行一次在 CF 运行时中注册两个新类 run loop（CFRunLoop）和 run loop mode（CFRunLoopMode），
    // 其中 __kCFRunLoopModeTypeID = _CFRuntimeRegisterClass(&__CFRunLoopModeClass)，那么 __kCFRunLoopModeTypeID 此时便是 run loop mode 类在全局类表中的索引。
    //（__CFRunLoopModeClass 可以理解为一个静态全局的 "类对象"（实际值是一个），_CFRuntimeRegisterClass 函数正是把它放进一个全局的 __CFRuntimeClassTable 类表中。）

    // 本身 srlm 是一片空白内存，现在相当于把 srlm 设置为一个 run loop mode 类的对象。 
    //（实际就是设置 CFRuntimeBase 的 _cfinfo 成员变量，srlm 里面目前包含的内容就是 run loop mode 的类信息。）
    _CFRuntimeSetInstanceTypeIDAndIsa(&srlm, __kCFRunLoopModeTypeID);
    
    // 把 srlm 的 mode 名称设置为入参 modeName
    srlm._name = modeName;
    
    // 从 rl->_modes 哈希表中找 &srlm 对应的 CFRunLoopModeRef
    rlm = (CFRunLoopModeRef)CFSetGetValue(rl->_modes, &srlm);
    
    // 如果找到了则加锁，然后返回 rlm。
    if (NULL != rlm) {
        __CFRunLoopModeLock(rlm);
        return rlm;
    }
    
    // 如果没有找到，并且 create 值为 false，则表示不进行创建，直接返回 NULL。
    if (!create) {
        return NULL;
    }
    
    // 创建一个 CFRunLoopMode 对并返回其地址
    rlm = (CFRunLoopModeRef)_CFRuntimeCreateInstance(kCFAllocatorSystemDefault, __kCFRunLoopModeTypeID, sizeof(struct __CFRunLoopMode) - sizeof(CFRuntimeBase), NULL);
    
    // 如果 rlm 创建失败，则返回 NULL
    if (NULL == rlm) {
        return NULL;
    }
    
    // 初始化 rlm 的 pthread_mutex_t _lock 为一个互斥递归锁。
    //（__CFRunLoopLockInit 内部使用的 PTHREAD_MUTEX_RECURSIVE 表示递归锁，允许同一个线程对同一锁加锁多次，且需要对应次数的解锁操作）
    __CFRunLoopLockInit(&rlm->_lock);
    
    // 初始化 _name
    rlm->_name = CFStringCreateCopy(kCFAllocatorSystemDefault, modeName);
    
    // 下面是一组成员变量的初始赋值
    rlm->_stopped = false;
    rlm->_portToV1SourceMap = NULL;
    
    // _sources0、_sources1、_observers、_timers 初始状态都是空的
    rlm->_sources0 = NULL;
    rlm->_sources1 = NULL;
    rlm->_observers = NULL;
    rlm->_timers = NULL;
    
    rlm->_observerMask = 0;
    rlm->_portSet = __CFPortSetAllocate(); // CFSet 申请空间初始化
    rlm->_timerSoftDeadline = UINT64_MAX;
    rlm->_timerHardDeadline = UINT64_MAX;
    
    // ret 是一个临时变量初始值是 KERN_SUCCESS，用来表示向 rlm->_portSet 中添加 port 时的结果，
    // 如果添加失败的话，会直接 CRASH 
    kern_return_t ret = KERN_SUCCESS;
    
#if USE_DISPATCH_SOURCE_FOR_TIMERS
    // macOS 下，使用 dispatch_source 构造 timer
    
    // _timerFired 首先赋值为 false，然后在 timer 的回调函数执行的时候会赋值为 true
    rlm->_timerFired = false;
    
    // 队列
    rlm->_queue = _dispatch_runloop_root_queue_create_4CF("Run Loop Mode Queue", 0);
    
    // 构建 queuePort，入参是 mode 的 _queue
    mach_port_t queuePort = _dispatch_runloop_root_queue_get_port_4CF(rlm->_queue);
    
    // 如果 queuePort 为 NULL，则 crash。（无法创建运行循环模式队列端口。）
    if (queuePort == MACH_PORT_NULL) CRASH("*** Unable to create run loop mode queue port. (%d) ***", -1);
    
    // 构建 dispatch_source 类型使用的是 DISPATCH_SOURCE_TYPE_TIMER，表示是一个 timer
    rlm->_timerSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, rlm->_queue);
    
    // 这里为了在下面的 block 内部修改 _timerFired 的值，用了一个 __block 指针变量。（觉的如果这里只是改值，感觉用指针就够了可以不用 __block 修饰）
    // 当 _timerSource（计时器）回调时会执行这个 block。
    __block Boolean *timerFiredPointer = &(rlm->_timerFired);
    dispatch_source_set_event_handler(rlm->_timerSource, ^{
        *timerFiredPointer = true;
    });
    
    // Set timer to far out there. The unique leeway makes this timer easy to spot in debug output.
    // 将计时器设置在远处。独特的回旋余地使该计时器易于发现调试输出。（从 DISPATCH_TIME_FOREVER 启动，DISPATCH_TIME_FOREVER 为时间间隔）
    _dispatch_source_set_runloop_timer_4CF(rlm->_timerSource, DISPATCH_TIME_FOREVER, DISPATCH_TIME_FOREVER, 321);
    // 启动
    dispatch_resume(rlm->_timerSource);
    
    // 把运行循环模式队列端口 queuePort 添加到 rlm 的 _portSet（端口集合）中。
    ret = __CFPortSetInsert(queuePort, rlm->_portSet);
    // 如果添加失败则 crash。（无法将计时器端口插入端口集中。）
    if (KERN_SUCCESS != ret) CRASH("*** Unable to insert timer port into port set. (%d) ***", ret);
#endif

#if USE_MK_TIMER_TOO
    // mk 构造 timer
    
    // 构建 timer 端口
    rlm->_timerPort = mk_timer_create();
    // 同样把 rlm 的 _timerPort 添加到 rlm 的 _portSet（端口集合）中。
    ret = __CFPortSetInsert(rlm->_timerPort, rlm->_portSet);
    // 如果添加失败则 crash。（无法将计时器端口插入端口集中。）
    if (KERN_SUCCESS != ret) CRASH("*** Unable to insert timer port into port set. (%d) ***", ret);
#endif
    
    // 然后这里把 rl 的 _wakeUpPort 也添加到 rlm 的 _portSet（端口集合）中。
    //（这里要特别注意一下，run loop 的 _wakeUpPort 会被插入到所有 mode 的 _portSet 中。）
    ret = __CFPortSetInsert(rl->_wakeUpPort, rlm->_portSet);
    // 如果添加失败则 crash。（无法将唤醒端口插入端口集中。）
    if (KERN_SUCCESS != ret) CRASH("*** Unable to insert wake up port into port set. (%d) ***", ret);
    
#if DEPLOYMENT_TARGET_WINDOWS
    rlm->_msgQMask = 0;
    rlm->_msgPump = NULL;
#endif

    // 这里把 rlm 添加到 rl 的 _modes 中，
    //（本质是把 rlm 添加到 _modes 哈希表中）
    CFSetAddValue(rl->_modes, rlm);
    
    // 释放，rlm 已被 rl->_modes 持有，并不会被销毁只是减少引用计数
    CFRelease(rlm);
    
    // 加锁，然后返回 rlm
    __CFRunLoopModeLock(rlm);    /* return mode locked */
    return rlm;
}
```
&emsp;其中 `ret = __CFPortSetInsert(rl->_wakeUpPort, rlm->_portSet)` 会把 run loop 对象的 `_wakeUpPort` 添加到每个 run loop mode 对象的 `_portSet` 端口集合里。即当一个 run loop 有多个 run loop mode 时，那么每个 run loop mode 都会有 run loop 的 `_wakeUpPort`。

&emsp;在 macOS 下 run loop mode 的 `_timerSource` 的计时器的回调事件内部会把 run loop mode 的 `_timerFired` 字段置为 true，表示计时器被触发。

&emsp;run loop mode 创建好了，看到 source/timer/observer 三者对应的 `_sources0`、`_sources1`、`_observers`、`_timers` 四个字段初始状态都是空，需要我们自己添加 run loop mode item，它们在代码层中对应的数据类型分别是: CFRunLoopSourceRef、CFRunLoopObserverRef、CFRunLoopTimerRef，那么就把它们放在下篇进行分析吧！

## 参考链接
**参考链接:🔗**
+ [runloop 源码](https://opensource.apple.com/tarballs/CF/)
+ [Run Loops 官方文档](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/Multithreading/RunLoopManagement/RunLoopManagement.html#//apple_ref/doc/uid/10000057i-CH16-SW1)
+ [iOS RunLoop完全指南](https://blog.csdn.net/u013378438/article/details/80239686)
+ [iOS源码解析: runloop的底层数据结构](https://juejin.cn/post/6844904090330234894)
+ [iOS源码解析: runloop的运行原理](https://juejin.cn/post/6844904090166624270)
+ [深入理解RunLoop](https://blog.ibireme.com/2015/05/18/runloop/)
+ [iOS底层学习 - 深入RunLoop](https://juejin.cn/post/6844903973665636360)
+ [一份走心的runloop源码分析](https://cloud.tencent.com/developer/article/1633329)
+ [NSRunLoop](https://www.cnblogs.com/wsnb/p/4753685.html)
+ [iOS刨根问底-深入理解RunLoop](https://www.cnblogs.com/kenshincui/p/6823841.html)
+ [RunLoop总结与面试](https://www.jianshu.com/p/3ccde737d3f3)
+ [Runloop-实际开发你想用的应用场景](https://juejin.cn/post/6889769418541252615)
+ [RunLoop 源码阅读](https://juejin.cn/post/6844903592369848328#heading-17)
+ [do {...} while (0) 在宏定义中的作用](https://www.cnblogs.com/lanxuezaipiao/p/3535626.html)
+ [CFRunLoop 源码学习笔记(CF-1151.16)](https://www.cnblogs.com/chengsh/p/8629605.html)
+ [操作系统大端模式和小端模式](https://www.cnblogs.com/wuyuankun/p/3930829.html)
