# iOS 从源码解析Run Loop (四)：Source、Timer、Observer 创建以及添加到 mode 的过程

> &emsp;首先接上上篇由于文字个数限制没有放下的内容。

## \__CFRunLoopFindMode
&emsp;`__CFRunLoopFindMode` 函数根据 modeName 从 rl 的 _modes 中找到其对应的 CFRunLoopModeRef，如果找到的话则加锁并返回。如果未找到，并且 create 为真的话，则新建 __CFRunLoopMode 并添加到 rl 的 `_modes` 中，然后加锁并返回，如果 create 为假的话，则直接返回 NULL 不进行创建。
```c++
static CFRunLoopModeRef __CFRunLoopFindMode(CFRunLoopRef rl, CFStringRef modeName, Boolean create) {
    // 用于检查给定的进程是否被 fork
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
    
    // 从 rl->_modes 哈希表中找 &srlm 对应的 CFRunLoopModeRef（看到这里虽然是新建的 srlm，但是查找的时候其实是用的 srlm._name 的值调用哈希函数取哈希值来进行查找的）
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
    
    // 创建一个 CFRunLoopMode 实例并返回其地址
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
    // 如果添加失败的话，会直接 CRASH， 
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
    
    // 释放，rlm 被 rl->_modes 持有，并不会被销毁
    CFRelease(rlm);
    
    // 加锁，然后返回 rlm
    __CFRunLoopModeLock(rlm);    /* return mode locked */
    return rlm;
}
```
&emsp;其中 `ret = __CFPortSetInsert(rl->_wakeUpPort, rlm->_portSet)` 会把 run loop 对象的 `_wakeUpPort` 添加到每个 run loop mode 对象的 `_portSet` 端口集合里。即当一个 run loop 有多个 run loop mode 时，那么每个 run loop mode 都会有 run loop 的 `_wakeUpPort`。

&emsp;在 macOS 下 run loop mode 的 `_timerSource` 的计时器的回调事件内部会把 run loop mode 的 `_timerFired` 字段置为 true，表示计时器被触发。

&emsp;run loop mode 创建好了，看到 source/timer/observer 三者对应的 `_sources0`、`_sources1`、`_observers`、`_timers` 四个字段初始状态都是空，需要我们自己添加 run loop mode item，它们在代码层中对应的数据类型分别是: CFRunLoopSourceRef、CFRunLoopObserverRef、CFRunLoopTimerRef，那么就把它们放在下篇进行分析吧！

> &emsp;下面开始进入本篇内容。

## CFRunLoopSourceRef（struct \__CFRunLoopSource *）
&emsp;CFRunLoopSourceRef 是事件源（输入源），通过源码可以发现它内部的 `_context` 联合体中有两个成员变量 `version0` 和 `version1`，它们正分别对应了我们前面提到过多次的 source0 和 source1。
```c++
typedef struct __CFRunLoopSource * CFRunLoopSourceRef;

struct __CFRunLoopSource {
    CFRuntimeBase _base; // 所有 CF "instances" 都是从这个结构开始的
    uint32_t _bits;
    pthread_mutex_t _lock; // 互斥递归锁
    CFIndex _order; /* immutable */ source 的优先级，值越小，优先级越高
    CFMutableBagRef _runLoops; // run loop 集合
    union {
        CFRunLoopSourceContext version0; /* immutable, except invalidation */
        CFRunLoopSourceContext1 version1; /* immutable, except invalidation */
    } _context;
};
```
&emsp;当 \__CFRunLoopSource 表示 source0 的数据结构时 `_context` 中使用 `CFRunLoopSourceContext version0`，下面是 `CFRunLoopSourceContext` 的定义。 
```c++
// #if __LLP64__
//  typedef unsigned long long CFOptionFlags;
//  typedef unsigned long long CFHashCode;
//  typedef signed long long CFIndex;
// #else
//  typedef unsigned long CFOptionFlags;
//  typedef unsigned long CFHashCode;
//  typedef signed long CFIndex;
// #endif

typedef struct {
    CFIndex version;
    void * info; // source 的信息
    const void *(*retain)(const void *info); // retain 函数
    void (*release)(const void *info); // release 函数
    CFStringRef (*copyDescription)(const void *info); // 返回描述字符串的函数
    Boolean (*equal)(const void *info1, const void *info2); // 判断 source 对象是否相等的函数
    CFHashCode (*hash)(const void *info); // 哈希函数
    
    // 上面是 CFRunLoopSourceContext 和 CFRunLoopSourceContext1 的基础内容双方完全等同，
    // 两者的区别主要在下面，同时它们也表示了 source0 和 source1 的不同功能。
    
    void (*schedule)(void *info, CFRunLoopRef rl, CFStringRef mode); // 当 source0 加入到 run loop 时触发的回调函数（在下面的 CFRunLoopAddSource 函数中可看到其被调用了）
    void (*cancel)(void *info, CFRunLoopRef rl, CFStringRef mode); // 当 source0 从 run loop 中移除时触发的回调函数
    
    // source0 要执行的任务块，当 source0 事件被触发时的回调, 最后调用 __CFRUNLOOP_IS_CALLING_OUT_TO_A_SOURCE0_PERFORM_FUNCTION__ 函数来执行 perform(info)
    void (*perform)(void *info);
} CFRunLoopSourceContext;
```
&emsp;当 \__CFRunLoopSource 表示 source1 的数据结构时 `_context` 中使用 `CFRunLoopSourceContext1 version1`，下面是 `CFRunLoopSourceContext1` 的定义。
```c++
typedef struct {
    CFIndex version;
    void * info; // source 的信息
    const void *(*retain)(const void *info); // retain 函数
    void (*release)(const void *info); // release 函数
    CFStringRef (*copyDescription)(const void *info); // 返回描述字符串的函数
    Boolean (*equal)(const void *info1, const void *info2); // 判断 source 对象是否相等的函数
    CFHashCode (*hash)(const void *info); // 哈希函数
    
    // 上面是 CFRunLoopSourceContext 和 CFRunLoopSourceContext1 的基础内容双方完全等同，
    // 两者的区别主要在下面，同时它们也表示了 source0 和 source1 的不同功能。
    
#if (TARGET_OS_MAC && !(TARGET_OS_EMBEDDED || TARGET_OS_IPHONE)) || (TARGET_OS_EMBEDDED || TARGET_OS_IPHONE)
    mach_port_t (*getPort)(void *info); // getPort 函数指针，用于当 source 被添加到 run loop 中的时候，从该函数中获取具体的 mach_port_t 对象.
    void * (*perform)(void *msg, CFIndex size, CFAllocatorRef allocator, void *info); // perform 函数指针即指向 run loop 被唤醒后将要处理的事情
#else
    void * (*getPort)(void *info);
    void (*perform)(void *info);
#endif
} CFRunLoopSourceContext1;
```
&emsp;上面是 `__CFRunLoopSource` 相关的数据结构，下面我们看一下 source 的创建函数（先看一下 `CFRunLoopSource` 类的注册）。
### CFRunLoopSourceGetTypeID
&emsp;`__CFRunLoopSourceClass` 是一个 CFRuntimeClass 结构体实例，用来表示 CFRunLoopSource "类对象"。
```c++
static const CFRuntimeClass __CFRunLoopSourceClass = {
    _kCFRuntimeScannedObject,
    "CFRunLoopSource",
    NULL, // init
    NULL, // copy
    __CFRunLoopSourceDeallocate, // 销毁函数
    __CFRunLoopSourceEqual, // 判等函数
    __CFRunLoopSourceHash, // 哈希函数
    NULL,
    __CFRunLoopSourceCopyDescription // 描述函数
};
```
&emsp;`CFRunLoopSourceGetTypeID` 函数把 \__CFRunLoopSourceClass 注册到类表中，并返回其在类表中的索引。
```c++
CFTypeID CFRunLoopSourceGetTypeID(void) {
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{ 
        __kCFRunLoopSourceTypeID = _CFRuntimeRegisterClass(&__CFRunLoopSourceClass); 
    });
    return __kCFRunLoopSourceTypeID;
}
```
### CFRunLoopSourceCreate
&mesp;`CFRunLoopSourceCreate` 是根据入参 `context` 来创建 source0 或 source1。
```c++
CFRunLoopSourceRef CFRunLoopSourceCreate(CFAllocatorRef allocator, CFIndex order, CFRunLoopSourceContext *context) {
    // 用于检查给定的进程是否被 fork
    CHECK_FOR_FORK();
    
    // 局部变量
    CFRunLoopSourceRef memory;
    uint32_t size;
    
    // context 不能为 NULL，否则直接 crash
    if (NULL == context) CRASH("*** NULL context value passed to CFRunLoopSourceCreate(). (%d) ***", -1);
    
    // 计算 __CFRunLoopSource 结构除 CFRuntimeBase base 字段之外的内存空间长度
    size = sizeof(struct __CFRunLoopSource) - sizeof(CFRuntimeBase);
    
    // 创建 __CFRunLoopSource 实例并返回其指针
    memory = (CFRunLoopSourceRef)_CFRuntimeCreateInstance(allocator, CFRunLoopSourceGetTypeID(), size, NULL);
    if (NULL == memory) {
        return NULL;
    }
    
    // 设置 memory 的 _cfinfo 字段的值
    __CFSetValid(memory);
    
    // 设置 _bits 字段的值
    __CFRunLoopSourceUnsetSignaled(memory);
    
    // 初始化 _lock 为互斥递归锁
    __CFRunLoopLockInit(&memory->_lock);
    
    memory->_bits = 0;
    memory->_order = order; // order 赋值
    memory->_runLoops = NULL;
    
    size = 0;
    
    // 根据 context 判断是 source0 还是 source1，它们有不同的内存长度
    switch (context->version) {
        case 0:
            size = sizeof(CFRunLoopSourceContext);
        break;
        case 1:
            size = sizeof(CFRunLoopSourceContext1);
        break;
    }
    
    // 设置 memory 的 context 区域的内存值为 context
    objc_memmove_collectable(&memory->_context, context, size);
    
    // 如果 context 的 retain 函数不为 NULL，则对其 info 执行 retain 函数
    if (context->retain) {
        memory->_context.version0.info = (void *)context->retain(context->info);
    }
    
    return memory;
}
```
&emsp;基本就是申请空间，然后进行一些字段进行初始化。下面看一下 source 是如何添加到 mode 中的（先看一下 `__CFSetValid` 函数的实现）。

#### __CFSetValid
&emsp;`__CFSetValid` 函数用来设置 CFRuntimeBase 的 `_cfinfo` 的值。
```c++
// 在 CFRunLoopSourceCreate 函数中是 __CFSetValid(memory); 这样的调用
// uint8_t _cfinfo[4];

#define CF_INFO_BITS (!!(__CF_BIG_ENDIAN__) * 3) // x86_64 macOS 下是 CF_INFO_BITS 的值是 0

#if defined(__BIG_ENDIAN__) // 大端模式
#define __CF_BIG_ENDIAN__ 1
#define __CF_LITTLE_ENDIAN__ 0
#endif

#if defined(__LITTLE_ENDIAN__) // 小端模式，x86_64 macOS 下是小端模式
#define __CF_LITTLE_ENDIAN__ 1
#define __CF_BIG_ENDIAN__ 0
#endif

CF_INLINE void __CFSetValid(void *cf) {
    // x86_64 macOS 下 CF_INFO_BITS 的值是 0
    // __CFBitfieldSetValue(((CFRuntimeBase *)cf)->_cfinfo[0], 3, 3, 1);
    __CFBitfieldSetValue(((CFRuntimeBase *)cf)->_cfinfo[CF_INFO_BITS], 3, 3, 1);
}
```
&emsp;那么 `__CFSetValid` 内部的调用就是 `__CFBitfieldSetValue(((CFRuntimeBase *)cf)->_cfinfo[0], 3, 3, 1);`，那么我们继续向下看 `__CFBitfieldSetValue` 宏定义。
```c++
// __CFBitfieldSetValue(((CFRuntimeBase *)cf)->_cfinfo[0], 3, 3, 1)
#define __CFBitfieldSetValue(V, N1, N2, X)   ((V) = ((V) & ~__CFBitfieldMask(N1, N2)) | (((X) << (N2)) & __CFBitfieldMask(N1, N2)))

// 宏转换如下：
// V = ((CFRuntimeBase *)cf)->_cfinfo[0]
// N1 = 3
// N2 = 3
// X = 1

// 为了能看的更清晰一点，这里先把宏转换中的 V 保留，我们把等号左边的内容转换完毕后再转换 V
((V) = ((V) & ~__CFBitfieldMask(3, 3)) | (((1) << (3)) & __CFBitfieldMask(3, 3))) // 1⃣️

#define __CFBitfieldMask(N1, N2)   ((((UInt32)~0UL) << (31UL - (N1) + (N2))) >> (31UL - N1))
// __CFBitfieldMask(3, 3) 
// ((((UInt32)~0UL) << (31UL - (3) + (3))) >> (31UL - 3))
// ((((UInt32)~0UL) << 31UL) >> (31UL - 3))
// 0b1000 0000 0000 0000 0000 0000 0000 0000 再右移 28 位
// 0b0000 0000 0000 0000 0000 0000 0001 0000
// 0x00000010UL

// __CFBitfieldMask(3, 3) 的值是 UInt32 类型的 0x00000010UL 

((V) = ((V) & ~0x00000010UL) | (((1) << (3)) & 0x00000010UL)) // 2⃣️
((V) = ((V) & 0xffffffefUL) | (0x00000008UL & 0x00000010UL)) // 3⃣️
((V) = ((V) & 0xffffffefUL) | 0x00000000UL) // 4⃣️
// 因为最右面是和 0 做或操作可直接忽略
((CFRuntimeBase *)cf)->_cfinfo[0] = ((CFRuntimeBase *)cf)->_cfinfo[0] & 0xffffffefUL // 5⃣️
// 前面学习 _CFRuntimeCreateInstance 函数时，我们知道 _cfinfo 中间两个字节保存的是类型 ID（类在类表中的索引），
// 然后前一个字节和后一个字节是 0，那么 _cfinfo[0] 低 8 位是 1110 1111
```
&emsp;`__CFRunLoopSourceUnsetSignaled(memory);` 调用内部同上 `__CFBitfieldSetValue(rls->_bits, 1, 1, 0);` 这里就不展开了。

### CFRunLoopAddSource
&emsp;`CFRunLoopAddSource` 函数是把 source 添加到 mode 中，不出意外，mode 参数是一个字符串。
```c++
void CFRunLoopAddSource(CFRunLoopRef rl, CFRunLoopSourceRef rls, CFStringRef modeName) {    /* DOES CALLOUT */
    // 用于检查给定的进程是否被 fork
    CHECK_FOR_FORK();
    
    // 如果 rl 被标记为正在进行释放，则直接返回。
    if (__CFRunLoopIsDeallocating(rl)) return;
    
    // 如果 rls 无效，则 return。
    if (!__CFIsValid(rls)) return;
    
    // 当 context.version0.schedule 不为 NULL 时，会被置为 true，用于标记在函数最后执行 schedule 函数，
    // schedule 是 source0 被添加到 mode 中后的回调函数。
    Boolean doVer0Callout = false;
    
    //（CFRunLoopRef）最外的 run loop 加锁，（它加锁的内部会嵌 run loop mode 的加锁或者 run loop source 的加锁）
    __CFRunLoopLock(rl);
    
    // 如果 modeName 是 kCFRunLoopCommonModes，即把 source 添加到 common mode 中
    if (modeName == kCFRunLoopCommonModes) {
        // 如果 rl 的 _commonModes 不为 NULL，则创建一个副本。
        CFSetRef set = rl->_commonModes ? CFSetCreateCopy(kCFAllocatorSystemDefault, rl->_commonModes) : NULL;
        
        // 如果 rl 的 _commonModeItems 为 NULL，即表示 common mode 模式下还没有任何 run loop mode item
        if (NULL == rl->_commonModeItems) {
            // 为 _commonModeItems 申请空间
            rl->_commonModeItems = CFSetCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeSetCallBacks);
        }
        
        // 把 rls 添加到 rl 的 common mode item 集合中
        CFSetAddValue(rl->_commonModeItems, rls);
        
        // 如果 rl 的 common modes 不为空，即表示当前 rl 已经有 mode 被标记为 common 了，这时需要把 common mode items 中的 item 都同步添加到被标记为 common 的 mode 中去，
        // 因为只要 rl 在这些 mode 下运行时，这些 run loop mode item 都能正常被回调。（例如 timer 只有在添加到 common mode 后才能在默认模式和滑动模式下时都能正常执行回调）
        if (NULL != set) {
            CFTypeRef context[2] = {rl, rls};
            /* add new item to all common-modes */
            // 调用 __CFRunLoopAddItemToCommonModes 函数把 rls 添加到 rl 的 common mode 中去
            CFSetApplyFunction(set, (__CFRunLoopAddItemToCommonModes), (void *)context);
            
            // 释放上面的临时变量 set
            CFRelease(set);
        }
    } else {
    // 即 modeName 当前是 common mode 之外的普通 mode
    
        // 在 rl 中找到该 run loop mode，（并且进行 mode 的加锁） 
        CFRunLoopModeRef rlm = __CFRunLoopFindMode(rl, modeName, true);
        
        // 如果 mode 不是空且 _sources0 是 NULL 的话，则为它们申请空间
        if (NULL != rlm && NULL == rlm->_sources0) {
            // 申请空间
            rlm->_sources0 = CFSetCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeSetCallBacks);
            rlm->_sources1 = CFSetCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeSetCallBacks);
            rlm->_portToV1SourceMap = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, NULL, NULL);
        }
        
        // 如果 rlm 不为 NULL，然后 _sources0 和 _sources1 都不包含 rlm
        if (NULL != rlm && !CFSetContainsValue(rlm->_sources0, rls) && !CFSetContainsValue(rlm->_sources1, rls)) {
            // 根据 rls 的 context 判断是 source0 还是 source1
            if (0 == rls->_context.version0.version) {
                // 如果 rls 是 source0 的话则直接添加到 _sources0 中（_sources0 中存放的都是开发者自己的 source）
                CFSetAddValue(rlm->_sources0, rls);
            } else if (1 == rls->_context.version0.version) {
                // 如果 rls 是 source1 的话直接添加到 _sources1 中（_sources1 比 _sources0 特殊，它主要存放系统的 source，下面还要处理端口的内容）
                CFSetAddValue(rlm->_sources1, rls);
                
                // 处理端口
                
                // 取出 source1 中的端口
                __CFPort src_port = rls->_context.version1.getPort(rls->_context.version1.info);
                if (CFPORT_NULL != src_port) {
                    // src_port 不为空的话，把端口保存到 rlm 中去。
                    // _portToV1SourceMap 和 _portSet 是 run loop mode 的两个成员变量：
                    // CFMutableDictionaryRef _portToV1SourceMap; // 存储了 Source1 的 port 与 source 的对应关系，key 是 mach_port_t，value 是 CFRunLoopSourceRef
                    // __CFPortSet _portSet; // 保存所有需要监听的 port，比如 _wakeUpPort，_timerPort，queuePort 都保存在这个集合中
                    
                    CFDictionarySetValue(rlm->_portToV1SourceMap, (const void *)(uintptr_t)src_port, rls);
                    __CFPortSetInsert(src_port, rlm->_portSet); 
                }
            }
            
            // ⬆️⬆️可看到目前只有 source0 和 source1
        
        //（__CFRunLoopSource）source 加锁
        __CFRunLoopSourceLock(rls);
        
        // 如果 rls 的 _runLoops 为 NULL 的话，则为 _runLoops 申请空间
        if (NULL == rls->_runLoops) {
            rls->_runLoops = CFBagCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeBagCallBacks); // sources retain run loops!
        }
        // 把 rl 添加到 rls 的 _runLoops 中
        CFBagAddValue(rls->_runLoops, rl);
        //（__CFRunLoopSource）source 解锁
        __CFRunLoopSourceUnlock(rls);
        
        // 如果 rls 是 source0，如果其 schedule 存在的，会在下面 rl 解锁后执行之，schedule 是 source 被添加到 mode 中的回调函数 
        if (0 == rls->_context.version0.version) {
            if (NULL != rls->_context.version0.schedule) {
                // 标记为 true，用于标记下面 rl 解锁后执行 schedule 回调
                doVer0Callout = true;
            }
        }
        }
        
        // 这个对应的加锁是在 __CFRunLoopFindMode(rl, modeName, true) 函数内找到 rlm 并返回之前加的。
        if (NULL != rlm) {
            //（CFRunLoopModeRef）run loop mode 解锁
            __CFRunLoopModeUnlock(rlm);
        }
    }
    
    //（CFRunLoopRef）最外的 run loop 解锁，（它加锁的内部会嵌 run loop mode 的加锁或者 run loop source 的加锁）
    __CFRunLoopUnlock(rl);
    
    // 回调 schedule 函数
    if (doVer0Callout) {
        // although it looses some protection for the source, we have no choice but to do this after unlocking
        // the run loop and mode locks, to avoid deadlocks where the source wants to take a lock which is already
        // held in another thread which is itself waiting for a run loop/mode lock
        // 尽管它为源代码失去了一些保护，但是我们别无选择，只能在解锁运行循环和模式锁之后执行此操作，
        // 以避免死锁，因为源代码要获取已经在另一个线程中持有的锁，而该线程本身正在等待运行循环/模式锁定。
        
        rls->_context.version0.schedule(rls->_context.version0.info, rl, modeName);    /* CALLOUT */
    }
}
```
&emsp;`CFRunLoopAddSource` 函数虽长但思路清晰，也都能和我们之前的结论对上，特别是当 source 添加到的 mode 是 common mode 时，会自动把 souce 同步到每个 common mode 中去。（注释已经极其清晰了，这里就不再总结了）

&emsp;CFRunLoopSource 的创建和添加就看完了，下面我们看 CFRunLoopObserverRef。（关于 CFRunLoopSource 中的 source0 和 souce1 的区别以及 source1 中的 port 相关的内容我们在下一篇再展开分析）

## CFRunLoopObserverRef（struct \__CFRunLoopObserver *）
&emsp;CFRunLoopObserverRef 是观察者，每个 observer 都包含了一个回调（函数指针），当 run loop 的状态发生变化时，观察者就能通过回调接收到这个变化。主要是用来向外界报告 run loop 当前的状态的更改。
```c++
typedef struct __CFRunLoopObserver * CFRunLoopObserverRef;

struct __CFRunLoopObserver {
    CFRuntimeBase _base; // 所有 CF "instances" 都是从这个结构开始的
    pthread_mutex_t _lock; // 互斥锁
    CFRunLoopRef _runLoop; // observer 所观察的 run loop
    CFIndex _rlCount; // observer 观察了多少个 run loop
    CFOptionFlags _activities; /* immutable */ // 所监听的事件，通过位异或，可以监听多种事件，_activities 用来说明要观察 runloop 的哪些状态，一旦指定了就不可变。
    CFIndex _order; /* immutable */ // observer 优先级
    
    // typedef void (*CFRunLoopObserverCallBack)(CFRunLoopObserverRef observer, CFRunLoopActivity activity, void *info);
    CFRunLoopObserverCallBack _callout; /* immutable */ // observer 回调函数，观察到 run loop 状态变化后的回调
    CFRunLoopObserverContext _context; /* immutable, except invalidation */ // observer 上下文
};
```
&emsp;observer 也包含一个回调函数，在监听的 run loop 状态出现时触发该回调函数。run loop 对 observer 的使用逻辑，基本与 timer 一致，都需要指定 callback 函数，然后通过 context 可传递参数。

&emsp;`CFRunLoopActivity` 是一组枚举值用于表示 run loop 的活动。
```c++
/* Run Loop Observer Activities */
typedef CF_OPTIONS(CFOptionFlags, CFRunLoopActivity) {
    kCFRunLoopEntry = (1UL << 0), // 进入 RunLoop 循环(这里其实还没进入)
    kCFRunLoopBeforeTimers = (1UL << 1), // Run Loop 要处理 timer 了
    kCFRunLoopBeforeSources = (1UL << 2), // Run Loop 要处理 source 了
    kCFRunLoopBeforeWaiting = (1UL << 5), // Run Loop 要休眠了
    kCFRunLoopAfterWaiting = (1UL << 6), // Run Loop 醒了
    kCFRunLoopExit = (1UL << 7), // Run Loop 退出（和 kCFRunLoopEntry 对应，Entry 和 Exit 在每次 Run Loop 循环中仅调用一次，用于表示即将进入循环和退出循环。）
    kCFRunLoopAllActivities = 0x0FFFFFFFU
};
```
&emsp;CFRunLoopObserverContext 的定义。
```c++
typedef struct {
    CFIndex version;
    void * info;
    
    const void *(*retain)(const void *info); // retain
    void (*release)(const void *info); // release 
    
    CFStringRef (*copyDescription)(const void *info); // 描述函数
} CFRunLoopObserverContext;
```
&emsp;然后接下来是 run loop observer 类对象，以及类注册。
```c++
CFTypeID CFRunLoopObserverGetTypeID(void) {
    static dispatch_once_t initOnce;
    
    dispatch_once(&initOnce, ^{ 
        // 类注册
        __kCFRunLoopObserverTypeID = _CFRuntimeRegisterClass(&__CFRunLoopObserverClass); 
    });
    
    return __kCFRunLoopObserverTypeID;
}
```
&emsp;`__CFRunLoopObserverClass` 是 run loop observer 的类对象。
```c++
static const CFRuntimeClass __CFRunLoopObserverClass = {
    0,
    "CFRunLoopObserver", // 名字
    NULL,      // init
    NULL,      // copy
    __CFRunLoopObserverDeallocate, // 销毁函数
    NULL,
    NULL,
    NULL,
    __CFRunLoopObserverCopyDescription // 描述函数
};
```
&emsp;好了，下面看 run loop observer 的创建函数。
### CFRunLoopObserverCreate
&emsp;`CFRunLoopObserverCreate` 函数用来创建带有函数回调的 CFRunLoopObserver 对象，有 6 个参数，我们首先看一下各个参数的含义。

&emsp;`allocator`：用于为新对象分配内存的分配器。传递 NULL 或 kCFAllocatorDefault 以使用当前的默认分配器。

&emsp;`activities`：标识运行循环活动阶段的标志集，在此期间应调用观察者。上面 CFRunLoopActivity 列出了不同的运行状态。要在运行循环的多个阶段调用观察者，请使用按位或运算符组合 CFRunLoopActivity 的值。

&emsp;`repeats`：一个标志，标识在运行循环中是仅一次还是每次调用观察者。如果 repeats 为 false，则即使安排在运行循环内的多个阶段调用观察者，观察者也将在调用一次之后失效。

&emsp;`order`：优先级索引，指示运行循环观察者的处理顺序。在给定的运行循环模式下，在同一活动阶段中安排了多个运行循环观察者时，将按此参数的升序对观察者进行处理。除非有其他原因，否则传递 0。

&emsp;`callout`：观察者运行时调用的回调函数。

&emsp;`context`：为运行循环观察者保存上下文信息的结构。该函数将信息从结构中复制出来，因此上下文所指向的内存不需要在函数调用之后继续存在。如果观察者不需要上下文的信息指针来跟踪状态，则可以为 NULL。

&emsp;返回值：新的 CFRunLoopObserver 对象。所有权遵循所有权策略中描述的创建规则。
```c++
CF_EXPORT CFRunLoopObserverRef CFRunLoopObserverCreate(CFAllocatorRef allocator,
                                                       CFOptionFlags activities,
                                                       Boolean repeats,
                                                       CFIndex order,
                                                       CFRunLoopObserverCallBack callout,
                                                       CFRunLoopObserverContext *context);
```
&emsp;好了，下面我们一起看一下该函数的实现。
```c++
CFRunLoopObserverRef CFRunLoopObserverCreate(CFAllocatorRef allocator,
                                             CFOptionFlags activities,
                                             Boolean repeats,
                                             CFIndex order,
                                             CFRunLoopObserverCallBack callout,
                                             CFRunLoopObserverContext *context) {
    // 用于检查给定的进程是否被 fork
    CHECK_FOR_FORK();
    
    // struct __CFRunLoopObserver 结构体指针
    CFRunLoopObserverRef memory;
    UInt32 size;
    // 计算 __CFRunLoopObserver 结构除 CFRuntimeBase base 字段之外的内存空间长度
    size = sizeof(struct __CFRunLoopObserver) - sizeof(CFRuntimeBase);
    
    // 创建 __CFRunLoopObserver 实例并返回其指针
    memory = (CFRunLoopObserverRef)_CFRuntimeCreateInstance(allocator, CFRunLoopObserverGetTypeID(), size, NULL);
    if (NULL == memory) {
        return NULL;
    }
    
    // 设置 memory 的 _cfinfo 字段的值
    __CFSetValid(memory);
    
    // 设置 memory 的 _cfinfo 字段的值，表示当前还没有开始观察 run loop。
    __CFRunLoopObserverUnsetFiring(memory);
    
    if (repeats) {
        // 设置 memory 的 _cfinfo 字段的值，表示重复观察 run loop 状态变化
        // __CFBitfieldSetValue(((CFRuntimeBase *)rlo)->_cfinfo[CF_INFO_BITS], 1, 1, 1);
        
        __CFRunLoopObserverSetRepeats(memory);
    } else {
        // 设置 memory 的 _cfinfo 字段的值，表示仅观察一次 run loop 状态变化
        // __CFBitfieldSetValue(((CFRuntimeBase *)rlo)->_cfinfo[CF_INFO_BITS], 1, 1, 0);
        
        __CFRunLoopObserverUnsetRepeats(memory);
    }
    
    // 初始化 _lock 为互斥递归锁
    __CFRunLoopLockInit(&memory->_lock);
    
    // 连续 5 个字段赋值
    memory->_runLoop = NULL;
    memory->_rlCount = 0;
    memory->_activities = activities;
    memory->_order = order;
    memory->_callout = callout;
    
    if (context) {
        // 根据 context 是否有 retain 函数，决定是否持有 info
        if (context->retain) {
            memory->_context.info = (void *)context->retain(context->info);
        } else {
            memory->_context.info = context->info;
        }
        
        // 配置 _context 字段的内容
        memory->_context.retain = context->retain;
        memory->_context.release = context->release;
        memory->_context.copyDescription = context->copyDescription;
    } else {
        // 如果入参 context 为 NULL，则 _context 字段的内容都置 0
        memory->_context.info = 0;
        memory->_context.retain = 0;
        memory->_context.release = 0;
        memory->_context.copyDescription = 0;
    }
    
    // 返回 struct __CFRunLoopObserver 结构体指针
    return memory;
}
```
&emsp;运行循环观察器不会自动添加到运行循环中。要将观察者添加到运行循环中，使用 CFRunLoopAddObserver。观察者只能注册到一个运行循环，尽管可以将其添加到该运行循环中的多个运行循环模式。

### CFRunLoopAddObserver
```c++
void CFRunLoopAddObserver(CFRunLoopRef rl, CFRunLoopObserverRef rlo, CFStringRef modeName) {
    // 用于检查给定的进程是否被 fork
    CHECK_FOR_FORK();
    
    // struct __CFRunLoopMode 指针
    CFRunLoopModeRef rlm;
    
    // 如果 rl 已经被标记为正在销毁，则 return（ rl->_cfinfo[CF_INFO_BITS] ）
    if (__CFRunLoopIsDeallocating(rl)) return;
    
    // rlo 无效则 return， 
    // 或者 rlo 的 _runLoop 不为 NULL 且 _runLoop 不等于 rl 的话直接 return
    if (!__CFIsValid(rlo) || (NULL != rlo->_runLoop && rlo->_runLoop != rl)) return;
    
    // run loop 加锁
    __CFRunLoopLock(rl);
    
    // 如果 modeName 是 kCFRunLoopCommonModes 的话，则需要把 rlo 添加到所有被标记为 common 的 mode 下，
    // 这样只要 rl 是 common mode 运行时都能得到观察。
    if (modeName == kCFRunLoopCommonModes) {
        // 取得 rl 的 _commonModes
        CFSetRef set = rl->_commonModes ? CFSetCreateCopy(kCFAllocatorSystemDefault, rl->_commonModes) : NULL;
        
        // 如果 rl 的 _commonModeItems 为 NULL，则为其申请空间
        if (NULL == rl->_commonModeItems) {
            rl->_commonModeItems = CFSetCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeSetCallBacks);
        }
        
        // 把 rlo 添加到 rl 的 _commonModeItems 中
        CFSetAddValue(rl->_commonModeItems, rlo);
        
        // 如果当前 rl 有标记为 common 的 mode，则把 rlo 添加到它们的 run loop mode 的 _observers 中
        if (NULL != set) {
            CFTypeRef context[2] = {rl, rlo};
            /* add new item to all common-modes */

            // 调用 __CFRunLoopAddItemToCommonModes 函数把 rlo 添加到 rl 的 common mode 中去
            CFSetApplyFunction(set, (__CFRunLoopAddItemToCommonModes), (void *)context);
            
            // 释放上面的临时变量 set
            CFRelease(set);
        }
    } else {
    // 即 modeName 当前是 common mode 之外的普通 mode
    
        // 在 rl 中找到该 run loop mode，（并且进行 mode 的加锁） 
        rlm = __CFRunLoopFindMode(rl, modeName, true);
        
        // 如果 mode 不是空且 _observers 是 NULL 的话，则为它们申请空间
        if (NULL != rlm && NULL == rlm->_observers) {
            // 为 rlm 的 _observers 申请空间
            rlm->_observers = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
        }
        
        // 如果 rlm 不为空，并且当前 rlm 的 _observers 不包含 rlo
        if (NULL != rlm && !CFArrayContainsValue(rlm->_observers, CFRangeMake(0, CFArrayGetCount(rlm->_observers)), rlo)) {
            // inserted 标记 rlo 是否插入成功
            Boolean inserted = false;
            
            // for 循环是从后向前便利，在 rlm 的 _observers 数组中找到第一个 _order 小于等于 rlo 的 run loop observer
            for (CFIndex idx = CFArrayGetCount(rlm->_observers); idx--; ) {
                // 找到指定位置的 CFRunLoopObserverRef
                CFRunLoopObserverRef obs = (CFRunLoopObserverRef)CFArrayGetValueAtIndex(rlm->_observers, idx);
                
                // 比较 _order
                if (obs->_order <= rlo->_order) {
                    // 插入
                    CFArrayInsertValueAtIndex(rlm->_observers, idx + 1, rlo);
                    
                    // 插入成功跳出 for 循环
                    inserted = true;
                    break;
                }
            }
            
            // 如果循环结束还是没找到比 rlo 的 _order 小的 run loop observer 的话，把 rlo 插入到 rlm 的 _observers 的 0 下标处
            if (!inserted) {
                CFArrayInsertValueAtIndex(rlm->_observers, 0, rlo);
            }
            
            // 把 rlo 观察的 run loop 状态添加到 rlm->_observerMask 中（rlm->_observerMask 不同位的 1 表示观察 run loop 不同的状态）
            rlm->_observerMask |= rlo->_activities;
            
            // __CFRunLoopObserverSchedule 函数实现在下面 ⬇️
            __CFRunLoopObserverSchedule(rlo, rl, rlm);
        }
        
        // 如果 rlm 不为空，则把 rlm = __CFRunLoopFindMode(rl, modeName, true); 中的 rlm 的加锁进行对应的解锁
        if (NULL != rlm) {
            __CFRunLoopModeUnlock(rlm);
        }
    }
    
    // run loop 解锁
    __CFRunLoopUnlock(rl);
}

static void __CFRunLoopObserverSchedule(CFRunLoopObserverRef rlo, CFRunLoopRef rl, CFRunLoopModeRef rlm) {
    // CFRunLoopObserver 加锁
    __CFRunLoopObserverLock(rlo);
    // 如果 rlo 的 _rlCount 为 0，则把 rlo 的 _runLoop 赋值为 rl
    if (0 == rlo->_rlCount) {
        rlo->_runLoop = rl;
    }
    // _rlCount 自增
    rlo->_rlCount++;
    
    // CFRunLoopObserver 解锁
    __CFRunLoopObserverUnlock(rlo);
}
```
&emsp;CFRunLoopAddObserver 的创建和添加就看完了，创建就是构建实例然后进行赋值，添加的话则和 CFRunLoopSource 不尽相同，CFRunLoopAddObserver 同样会依赖于特定的 run loop mode，所以一旦 run loop 运行于其它 mode，则需要的状态就会无法监听到。

&emsp;下面我们开始看 CFRunLoopTimerRef。
## CFRunLoopTimerRef（struct \__CFRunLoopTimer *）
&emsp;NSTimer 是与 run loop 息息相关的，CFRunLoopTimerRef 与 NSTimer 是可以 toll-free bridged（免费桥接转换）的。当 timer 加到 run loop 的时候，run loop 会注册对应的触发时间点，时间到了，run loop 若处于休眠则会被唤醒，执行 timer 对应的回调函数。
```c++
typedef struct CF_BRIDGED_MUTABLE_TYPE(NSTimer) __CFRunLoopTimer * CFRunLoopTimerRef;

struct __CFRunLoopTimer {
    CFRuntimeBase _base; // 所有 CF "instances" 都是从这个结构开始的
    uint16_t _bits; // 标记 timer 的状态
    pthread_mutex_t _lock; // 互斥锁
    CFRunLoopRef _runLoop; // timer 对应的 run loop，注册在哪个 run loop 中
    CFMutableSetRef _rlModes; // timer 对应的 run loop modes，内部保存的也是 run loop mode 的名字，也验证了 timer 可以在多个 run loop mode 中使用
    CFAbsoluteTime _nextFireDate; // timer 的下次触发时机，每次触发后都会再次设置该值
    CFTimeInterval _interval; /* immutable */ // timer 的时间间隔
    
    // _tolerance 默认值是 0.0， 然后 CFRunLoopTimerSetTolerance 函数中 当入参小于 0 的话，设置 tolerance = 0.0，
    // 如果大于 0 的话，设置 rlt->_tolerance = MIN(tolerance, rlt->_interval / 2)，即控制容忍的偏差值最大为 _interval 的一半。
    CFTimeInterval _tolerance; /* mutable */ // timer 的允许时间偏差
    
    uint64_t _fireTSR; /* TSR units */ // timer 本次被触发的时间点
    CFIndex _order; /* immutable */ // timer 优先级
    
    // typedef void (*CFRunLoopTimerCallBack)(CFRunLoopTimerRef timer, void *info);
    CFRunLoopTimerCallBack _callout; /* immutable */ // timer 回调
    CFRunLoopTimerContext _context; /* immutable, except invalidation */ // timer 上下文，可用于传递参数到 timer 对象的回调函数中。
};
```
&emsp;CFRunLoopTimerContext 的定义。
```c++
typedef struct {
    CFIndex version;
    void * info;
    const void *(*retain)(const void *info); // retain
    void (*release)(const void *info); // release
     
    CFStringRef (*copyDescription)(const void *info); // 描述函数
} CFRunLoopTimerContext;
```
&emsp;然后接下来是 run loop timer 类对象，以及类注册。
```c++
CFTypeID CFRunLoopTimerGetTypeID(void) {
    static dispatch_once_t initOnce;
    dispatch_once(&initOnce, ^{ 
        // 类注册
        __kCFRunLoopTimerTypeID = _CFRuntimeRegisterClass(&__CFRunLoopTimerClass); 
    });
    return __kCFRunLoopTimerTypeID;
}
```
&emsp;`__CFRunLoopTimerClass` 是 run loop timer 的类对象。
```c++
static const CFRuntimeClass __CFRunLoopTimerClass = {
    0,
    "CFRunLoopTimer", // 名字
    NULL,      // init
    NULL,      // copy
    __CFRunLoopTimerDeallocate, // 销毁函数
    NULL,    // equal
    NULL,
    NULL,
    __CFRunLoopTimerCopyDescription // 描述函数
};
```
&emsp;好了，下面看 run loop timer 的创建函数。

### CFRunLoopTimerCreate
&emsp;`CFRunLoopTimerCreate` 用函数回调创建一个新的 CFRunLoopTimer 对象，有 7 个参数，我们首先看一下各个参数的含义。

&emsp;`allocator`：用于为新对象分配内存的分配器。传递 NULL 或 kCFAllocatorDefault 以使用当前的默认分配器。

&emsp;`fireDate`：计时器应首先触发的时间。如果有实现原因，可以通过计时器对触发日期的精细精度（最多为亚毫秒）进行调整。

&emsp;`interval`：计时器的触发间隔。如果为 0 或负数，计时器将触发一次，然后自动失效。如果存在实施原因，可以通过计时器对间隔的精细精度（最多为亚毫秒）进行调整。

&emsp;`flags`：目前被忽略。传递 0 以获得将来的兼容性。

&emsp;`order`：优先级索引，指示运行循环计时器的处理顺序。运行循环计时器当前忽略此参数。传递 0。

&emsp;`callout`：计时器触发时调用的回调函数。

&emsp;`context`：保存运行循环计时器的上下文信息的结构。该函数将信息从结构中复制出来，因此上下文所指向的内存不需要在函数调用之后继续存在。如果回调函数不需要上下文的信息指针来跟踪状态，则可以为 NULL。
```c++
CFRunLoopTimerRef CFRunLoopTimerCreate(CFAllocatorRef allocator,
                                       CFAbsoluteTime fireDate,
                                       CFTimeInterval interval,
                                       CFOptionFlags flags,
                                       CFIndex order,
                                       CFRunLoopTimerCallBack callout,
                                       CFRunLoopTimerContext *context) {
    // 用于检查给定的进程是否被 fork
    CHECK_FOR_FORK();
    
    // 如果 interval 为 NaN，则 crash
    if (isnan(interval)) {
        CRSetCrashLogMessage("NaN was used as an interval for a CFRunLoopTimer");
        HALT;
    }
    
    // struct __CFRunLoopTimer 结构体指针
    CFRunLoopTimerRef memory;
    UInt32 size;
    // 计算 __CFRunLoopTimer 结构除 CFRuntimeBase base 字段之外的内存空间长度
    size = sizeof(struct __CFRunLoopTimer) - sizeof(CFRuntimeBase);
    
    // 创建 __CFRunLoopTimer 实例并返回其指针
    memory = (CFRunLoopTimerRef)_CFRuntimeCreateInstance(allocator, CFRunLoopTimerGetTypeID(), size, NULL);
    if (NULL == memory) {
        return NULL;
    }
    
    // 设置 memory 的 _cfinfo 字段的值
    __CFSetValid(memory);
    
    // 设置 memory 的 _cfinfo 字段的值，表示当前 timer 还没有开始执行
    __CFRunLoopTimerUnsetFiring(memory);
    
    // 初始化 _lock 为互斥递归锁
    __CFRunLoopLockInit(&memory->_lock);
    
    // _runLoop 置为 NULL
    memory->_runLoop = NULL;
    // 为 timer 的 _rlModes 申请空间
    memory->_rlModes = CFSetCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeSetCallBacks);
    // 优先级，等下把 timer 添加到 mode 的时候会用到
    memory->_order = order;
    
    // 如果 interval 小于 0.0，则置为 0
    if (interval < 0.0) interval = 0.0;
    
    // 时间间隔赋值
    memory->_interval = interval;
    memory->_tolerance = 0.0;
    
    // 下面这一部分代码保证计时器第一次触发的时间点正常。
    // #define TIMER_DATE_LIMIT    4039289856.0
    // 如果入参 fireDate 过大，则置为 TIMER_DATE_LIMIT
    if (TIMER_DATE_LIMIT < fireDate) fireDate = TIMER_DATE_LIMIT;
    
    // 下次触发的时间
    memory->_nextFireDate = fireDate;
    memory->_fireTSR = 0ULL;
    
    // 取得当前时间
    uint64_t now2 = mach_absolute_time();
    CFAbsoluteTime now1 = CFAbsoluteTimeGetCurrent();
    
    if (fireDate < now1) {
        // 如果第一次触发的时间已经过了，则把 _fireTSR 置为当前
        memory->_fireTSR = now2;
    } else if (TIMER_INTERVAL_LIMIT < fireDate - now1) {
        // 如果第一次触发的时间点与当前是时间差距超过了 TIMER_INTERVAL_LIMIT，则把 _fireTSR 置为 TIMER_INTERVAL_LIMIT
        memory->_fireTSR = now2 + __CFTimeIntervalToTSR(TIMER_INTERVAL_LIMIT);
    } else {
        // 这里则是正常的，如果第一次触发的时间还没有到，则把触发时间设置为当前时间和第一次触发时间点的差值
        memory->_fireTSR = now2 + __CFTimeIntervalToTSR(fireDate - now1);
    }
    
    // 回调函数
    memory->_callout = callout;
    
    // context
    if (NULL != context) {
        // 如果 context 不为 NULL
        
        // 是否 retain info
        if (context->retain) {
            memory->_context.info = (void *)context->retain(context->info);
        } else {
            memory->_context.info = context->info;
        }
        
        // 进行赋值
        memory->_context.retain = context->retain;
        memory->_context.release = context->release;
        memory->_context.copyDescription = context->copyDescription;
    } else {
        // 置 0
        memory->_context.info = 0;
        memory->_context.retain = 0;
        memory->_context.release = 0;
        memory->_context.copyDescription = 0;
    }
    return memory;
}
```
&emsp;计时器需要先添加到运行循环模式，然后才能启动。要将计时器添加到运行循环中，使用 CFRunLoopAddTimer  函数。计时器一次只能注册到一个运行循环中，尽管它可以在该运行循环中处于多种模式。

### CFRunLoopAddTimer
&emsp;`CFRunLoopAddTimer` 将 CFRunLoopTimer 对象添加到 run loop mode。`modeName` 是要将计时器添加到的 rl 的运行循环模式。使用常数 kCFRunLoopCommonModes 将计时器添加到所有 common mode 监视的对象集中。

&emsp;尽管可以将一个运行循环计时器一次添加到一个运行循环中的多个运行循环模式中，但它一次只能在一个运行循环中注册。如果 rl 已经在模式下包含计时器，则此功能不执行任何操作。
```c++
void CFRunLoopAddTimer(CFRunLoopRef rl, CFRunLoopTimerRef rlt, CFStringRef modeName) {    
    // 用于检查给定的进程是否被 fork
    CHECK_FOR_FORK();
    
    // 如果 rl 被标记为正在进行释放，则直接返回。
    if (__CFRunLoopIsDeallocating(rl)) return;
    
    // rlt 无效则 return， 
    // 或者 rlt 的 _runLoop 不为 NULL 且 _runLoop 不等于 rl 的话直接 return
    if (!__CFIsValid(rlt) || (NULL != rlt->_runLoop && rlt->_runLoop != rl)) return;
    
    // run loop 加锁
    __CFRunLoopLock(rl);
    
    // 如果 rlt 添加到 common mode 中
    if (modeName == kCFRunLoopCommonModes) {
        // 取得 rl 的 _commonModes 的副本
        CFSetRef set = rl->_commonModes ? CFSetCreateCopy(kCFAllocatorSystemDefault, rl->_commonModes) : NULL;
        
        // 如果 rl 的 _commonModeItems 为 NULL，则为其申请内存
        if (NULL == rl->_commonModeItems) {
            rl->_commonModeItems = CFSetCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeSetCallBacks);
        }
        
        // 把 rlt 添加到 rl 的 _commonModeItems
        CFSetAddValue(rl->_commonModeItems, rlt);
        
        // 如果 rl 存在 common mode
        if (NULL != set) {
            CFTypeRef context[2] = {rl, rlt};
            /* add new item to all common-modes */
            // 调用 __CFRunLoopAddItemToCommonModes 函数把 rlt 添加到 rl 被标记为 common 的 mode 中
            CFSetApplyFunction(set, (__CFRunLoopAddItemToCommonModes), (void *)context);
            
            // 释放上面的临时变量 set
            CFRelease(set);
        }
    } else {
    // 即 modeName 当前是 common mode 之外的普通 mode
    
        // 在 rl 中找到该 run loop mode，（并且进行 mode 的加锁） 
        CFRunLoopModeRef rlm = __CFRunLoopFindMode(rl, modeName, true);
        
        // 如果找到了 rlm
        if (NULL != rlm) {
            // 如果 rlm 的 _timers 为 NULL 的话，则为其申请内存
            if (NULL == rlm->_timers) {
                CFArrayCallBacks cb = kCFTypeArrayCallBacks;
                cb.equal = NULL;
                
                // 为 _timers 申请内存
                rlm->_timers = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &cb);
            }
        }
        
        // 如果 rlm 不为 NULL，且 rlt 的 _rlModes 不包含 rlm
        if (NULL != rlm && !CFSetContainsValue(rlt->_rlModes, rlm->_name)) {
            // rlt 加锁
            __CFRunLoopTimerLock(rlt);

            if (NULL == rlt->_runLoop) {
                // 如果 rlt->_runLoop 为 NULL，则为 rlt 的 _runLoop 赋值
                rlt->_runLoop = rl;
            } else if (rl != rlt->_runLoop) {
                // 如果 rlt 的 _runLoop 不为 rl，即此时 rlt 的 _runLoop 有值，一般 rlt 创建好后 _runLoop 字段是 NULL，
                // 则纷纷解锁后直接 return（即如果 timer 被添加过一个 run loop 了，就不能再被添加到别的 run loop 了。）
                
                __CFRunLoopTimerUnlock(rlt);
                __CFRunLoopModeUnlock(rlm);
                __CFRunLoopUnlock(rl);
                
                // 直接return
                return;
            }
            
            // 把 rlm 的 _name 添加到 rlt 的 _rlModes 中。
            //（也验证了 CFMutableSetRef _rlModes; timer 对应的 run loop modes，内部保存的也是 run loop mode 的名字，也验证了 timer 可以在多个 run loop mode 中使用）
            CFSetAddValue(rlt->_rlModes, rlm->_name);
            
            // timer 解锁
            __CFRunLoopTimerUnlock(rlt);
            
            // static CFLock_t __CFRLTFireTSRLock = CFLockInit;
            // 加锁
            __CFRunLoopTimerFireTSRLock();
            // 把 rlt 添加到 rlm 的 _timers 中
            __CFRepositionTimerInMode(rlm, rlt, false);
            // 解锁
            __CFRunLoopTimerFireTSRUnlock();
            
            // CFSystemVersionLion = 7,        /* 10.7 */
            if (!_CFExecutableLinkedOnOrAfter(CFSystemVersionLion)) {
                // Normally we don't do this on behalf of clients, but for backwards compatibility due to the change in timer handling...
                // 通常我们不代表客户这样做，而是由于计时器处理方式的变化而向后兼容...
                if (rl != CFRunLoopGetCurrent()) CFRunLoopWakeUp(rl);
            }
        }
        
        // 这个对应的加锁是在 __CFRunLoopFindMode(rl, modeName, true) 函数内找到 rlm 并返回之前加的。
        if (NULL != rlm) {
            __CFRunLoopModeUnlock(rlm);
        }
    }
    
    // run loop 解锁
    __CFRunLoopUnlock(rl);
}
```
&emsp;看到这里 CFRunLoopSourceRef、CFRunLoopObserverRef、CFRunLoopTimerRef 的创建以及添加就都看完了，其实还是蛮清晰的，下篇我们开始看 run loop 的运行相关的内容。⛽️⛽️

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
+ [CFBag](https://nshipster.cn/cfbag/)
