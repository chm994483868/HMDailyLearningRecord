# iOS 多线程知识体系构建(四)：GCD API（object.h、group.h、semaphore.h）解析篇

> &emsp;那么继续学习 dispath 中也挺重要的 <dispatch/object.h> 文件。

## dispatch_object_t
&emsp;`dispatch_object_t` 是所有调度对象（dispatch objects）的抽象基类型，且 `dispatch_object_t` 的具体定义在特定语言（Swift/Objective-C/C）下不同。调度对象通过调用 `dispatch_retain` 和 `dispatch_release` 进行引用计数管理。

&emsp;默认情况下，使用 Objective-C 编译器进行构建时，调度对象被声明为 Objective-C 类型。这使他们可以参与 ARC，通过 Blocks 运行时参与 RR（retain/release）管理以及通过静态分析器参与泄漏检查，并将它们添加到 Cocoa 集合（NSMutableArray、NSMutableDictionary...）中。详细信息可参考 <os/object.h>，下面会对 <os/object.h> 文件进行分析，特别是其中的 `OS_OBJECT_DECL_CLASS` 宏定义，分别针对不同的语言环境来定义不同的  `dispatch_object_t`。

&emsp;下面先看结论，然后再看 <os/object.h> 文件。

&emsp;`dispatch_object_t` 来自如下宏:
```c++
OS_OBJECT_DECL_CLASS(dispatch_object);
```
+ 在 Swift（在 Swift 中使用 Objective-C）下宏定义展开是:
```c++
OS_EXPORT OS_OBJECT_OBJC_RUNTIME_VISIBLE
@interface OS_dispatch_object : OS_object
- (instancetype)init OS_SWIFT_UNAVAILABLE("Unavailable in Swift");
@end
typedef OS_dispatch_object * dispatch_object_t
```
&emsp;`OS_dispatch_object` 是继承自 `OS_object` 的类，然后 `dispatch_object_t` 是一个指向 `OS_dispatch_object` 的指针。
+ 在 Objective-C 下宏定义展开是:
```c++
@protocol OS_dispatch_object <NSObject> 
@end
typedef NSObject<OS_dispatch_object> * dispatch_object_t;  
```
&emsp;`OS_dispatch_object` 是继承自 `NSObject` 协议的协议，并且为遵循该协议的 `NSObject` 实例对象类型的指针定义了一个 `dispatch_object_t` 的别名。
+ 在 C++ 下宏定义展开是:
```c++
typedef struct dispatch_object_s *dispatch_object_t
```
&emsp;`dispatch_object_t` 是一个指向 `dispatch_object_s` 结构体的指针。

&emsp;`dispatch_object_s` 的定义也很简单，就是一个很基本的 C++ 结构体定义：
（注释：调度对象不是 C++ 对象。尽管如此，我们至少可以使 C++ 知道类型兼容性。）
```c++
typedef struct dispatch_object_s {
private:
    dispatch_object_s(); // 构造函数
    ~dispatch_object_s(); // 析构函数
    dispatch_object_s(const dispatch_object_s &); // 复制构造函数
    void operator=(const dispatch_object_s &); // 赋值操作符
} *dispatch_object_t; // dispatch_object_t 是指向 dispatch_object_s 结构体的指针。
```
+ 在 C（Plain C）下是:
&emsp;`dispatch_object_t` 不再是一个指针而是一个联合体（union）：
```c++
typedef union {
    struct _os_object_s *_os_obj;
    struct dispatch_object_s *_do;
    struct dispatch_queue_s *_dq;
    struct dispatch_queue_attr_s *_dqa;
    struct dispatch_group_s *_dg;
    struct dispatch_source_s *_ds;
    struct dispatch_channel_s *_dch;
    struct dispatch_mach_s *_dm;
    struct dispatch_mach_msg_s *_dmsg;
    struct dispatch_semaphore_s *_dsema;
    struct dispatch_data_s *_ddata;
    struct dispatch_io_s *_dchannel;
} dispatch_object_t DISPATCH_TRANSPARENT_UNION;
```
&emsp;看到里面有很多我们陌生的结构体，这里暂时不进行解读，等到学习源码时我们自然能见到它们的定义。

&emsp;下面看一下 <os/object.h> 文件。
## <os/object.h> 文件
>
>  @header
>  @preprocinfo
>  &emsp;By default, libSystem objects such as GCD and XPC objects are declared as
>  Objective-C types when building with an Objective-C compiler. This allows
>  them to participate in ARC, in RR management by the Blocks runtime and in
>  leaks checking by the static analyzer, and enables them to be added to Cocoa
>  collections.
> 
>  NOTE: this requires explicit cancellation of dispatch sources and xpc
>        connections whose handler blocks capture the source/connection object,
>        resp. ensuring that such captures do not form retain cycles (e.g. by
>        declaring the source as __weak).
> 
>  &emsp;To opt-out of this default behavior, add -DOS_OBJECT_USE_OBJC=0 to your
>  compiler flags.
> 
>  &emsp;This mode requires a platform with the modern Objective-C runtime, the
>  Objective-C GC compiler option to be disabled, and at least a Mac OS X 10.8
>  or iOS 6.0 deployment target.
> 
> &emsp;默认情况下，在使用 Objective-C 编译器进行构建时，libSystem 对象（例如 GCD 和 XPC 对象）被声明为 Objective-C 类型，这使他们可以参与 ARC，通过 Blocks 运行时参与 RR 管理以及通过静态分析器参与泄漏检查，并将它们添加到 Cocoa 集合中。
> 
> &emsp;注意：这需要显式取消 dispatch sources 和 xpc 连接来处理 blocks 捕获 source/connection 对象。 确保此类捕获不会形成循环引用（例如，通过将来源声明为 __weak）。
>
> &emsp;要选择退出此默认行为，请将 DOS_OBJECT_USE_OBJC = 0 添加到的编译器标志中即可。
> 
> &emsp;此模式要求平台具有现代的 Objective-C runtime，要禁用的 Objective-C GC 编译器选项，以及至少 Mac OS X 10.8 或 iOS 6.0 的版本要求。

### OS_OBJECT_HAVE_OBJC_SUPPORT
&emsp;`OS_OBJECT_HAVE_OBJC_SUPPORT` 仅在 macOS 10.8（i386 则是 10.12）以上或者 iOS 6.0 值为 1， 其它情况为 0。
### OS_OBJECT_USE_OBJC
&emsp;在 `OS_OBJECT_HAVE_OBJC_SUPPORT` 为 1 的情况下，在 macOS/iOS `__swift__` 情况下都是 1。
### OS_OBJECT_SWIFT3
&emsp;在 `__swift__` 宏存在时，`OS_OBJECT_SWIFT3` 都为 1。
### OS_OBJC_INDEPENDENT_CLASS
&emsp;`OS_OBJECT_USE_OBJC` 为 1 的情况下，存在 `objc_independent_class` 属性，则 `OS_OBJC_INDEPENDENT_CLASS` 是： `__attribute__((objc_independent_class))` 否则只是一个空的宏定义。
### OS_OBJECT_CLASS
&emsp;`#define OS_OBJECT_CLASS(name) OS_##name` 仅是为 `name` 添加一个 `OS_` 前缀，如 `OS_OBJECT_CLASS(object)` 宏展开是 `OS_object`。
### OS_OBJECT_DECL_PROTOCOL
&emsp;用于协议声明，`__VA_ARGS__` 是多参的宏展开时连续按序拼接各个参。
```c++
#define OS_OBJECT_DECL_PROTOCOL(name, ...) \
@protocol OS_OBJECT_CLASS(name) __VA_ARGS__ \
@end
```
### OS_OBJECT_CLASS_IMPLEMENTS_PROTOCOL_IMPL
&emsp;类声明并遵循指定的协议。
```c++
#define OS_OBJECT_CLASS_IMPLEMENTS_PROTOCOL_IMPL(name, proto) \
@interface name () <proto> \
@end
```
### OS_OBJECT_CLASS_IMPLEMENTS_PROTOCOL
&emsp;给 `name` 和 `proto` 添加 `OS_` 前缀。
```c++
#define OS_OBJECT_CLASS_IMPLEMENTS_PROTOCOL(name, proto) \
OS_OBJECT_CLASS_IMPLEMENTS_PROTOCOL_IMPL( \
        OS_OBJECT_CLASS(name), OS_OBJECT_CLASS(proto))
```
### OS_OBJECT_DECL_IMPL
&emsp;声明一个 `OS_name` 的协议，然后声明指向 `NSObject` 遵循 `OS_name` 协议的类型指针的别名 `name_t`。
```c++
#define OS_OBJECT_DECL_IMPL(name, ...) \
OS_OBJECT_DECL_PROTOCOL(name, __VA_ARGS__) \
typedef NSObject<OS_OBJECT_CLASS(name)> \
        * OS_OBJC_INDEPENDENT_CLASS name##_t
```
### OS_OBJECT_DECL_BASE
&emsp;声明 `OS_name` 类型，`name` 后面的参表示其继承的父类，然后有一个 `init` 函数。
```c++
#define OS_OBJECT_DECL_BASE(name, ...) \
        @interface OS_OBJECT_CLASS(name) : __VA_ARGS__ \
        - (instancetype)init OS_SWIFT_UNAVAILABLE("Unavailable in Swift"); \
        @end
```
### OS_OBJECT_DECL_IMPL_CLASS
&emsp;先声明一个类 `OS_name` 然后声明一个指向该类指针的别名 `name_t`。
```c++
#define OS_OBJECT_DECL_IMPL_CLASS(name, ...) \
        OS_OBJECT_DECL_BASE(name, ## __VA_ARGS__) \
        typedef OS_OBJECT_CLASS(name) \
                * OS_OBJC_INDEPENDENT_CLASS name##_t
```
### OS_OBJECT_DECL
&emsp;继承自 `<NSObject>` 协议的 `OS_name` 协议。
```c++
#define OS_OBJECT_DECL(name, ...) \
        OS_OBJECT_DECL_IMPL(name, <NSObject>)
```
### OS_OBJECT_DECL_SUBCLASS
&emsp;指定 `OS_name` 协议继承自 `OS_super` 协议。
```c++
#define OS_OBJECT_DECL_SUBCLASS(name, super) \
OS_OBJECT_DECL_IMPL(name, <OS_OBJECT_CLASS(super)>)
```
### OS_OBJECT_RETURNS_RETAINED
&emsp;如果存在 `ns_returns_retained` 属性，则 `OS_OBJECT_RETURNS_RETAINED` 宏定义为 `__attribute__((__ns_returns_retained__))`，否则仅是一个空的宏定义。
### OS_OBJECT_CONSUMED
&emsp;如果存在 `ns_consumed` 属性，则 `OS_OBJECT_CONSUMED` 宏定义为 `__attribute__((__ns_consumed__))`，否则仅是一个空的宏定义。
### OS_OBJECT_BRIDGE
&emsp;如果是 `objc_arc` 环境，则 `OS_OBJECT_BRIDGE` 宏定义为 `__bridge`，在 ARC 下对象类型转为 `void *` 时，需要加 `__bridge`，MRC 下则不需要。

&emsp;下面是一组在 Swift 中使用 ObjC 的宏，为了在 10.12 之前的 SDK 上向 Swift 中的 ObjC 对象提供向后部署，可以将 OS_object 类标记为 OS_OBJECT_OBJC_RUNTIME_VISIBLE。使用早于 OS X 10.12（iOS 10.0，tvOS 10.0，watchOS 3.0）的部署目标进行编译时，Swift 编译器将仅在运行时（使用 ObjC 运行时）引用此类型。

### OS_OBJECT_DECL_CLASS
&emsp;最重要的一条宏，涉及到不同语言环境下的定义，如开篇的 `OS_OBJECT_DECL_CLASS(dispatch_object)` 所示。
```c++
#if OS_OBJECT_SWIFT3

// 1⃣️：SWift 环境下
#define OS_OBJECT_DECL_CLASS(name) \
        OS_OBJECT_DECL_SUBCLASS_SWIFT(name, object)
        
#elif OS_OBJECT_USE_OBJC

// 2⃣️：Objective-c 环境下
#define OS_OBJECT_DECL_CLASS(name) \
        OS_OBJECT_DECL(name)
        
#else

// 3⃣️：C/C++ 环境下
#define OS_OBJECT_DECL_CLASS(name) \
        typedef struct name##_s *name##_t
        
#endif
```
### OS_OBJECT_GLOBAL_OBJECT
&emsp;桥接转化，如 ARC 下 `NSObject *` 转化为 `void *`。
```c++
#define OS_OBJECT_GLOBAL_OBJECT(type, object) ((OS_OBJECT_BRIDGE type)&(object))
```
### os_retain
&emsp;`os_retain` 增加 os_object 的引用计数。
```c++
API_AVAILABLE(macos(10.10), ios(8.0))
OS_EXPORT OS_SWIFT_UNAVAILABLE("Can't be used with ARC")
void*
os_retain(void *object);
#if OS_OBJECT_USE_OBJC
// ObjC 下则是定义成一个宏，调用 retain 函数
#undef os_retain
#define os_retain(object) [object retain]
#endif
```
&emsp;在具有现代 Objective-C 运行时的平台上，这完全等同于向对象发送 -[retain] 消息。

&emsp;`object` 要 retain 的对象。

&emsp;`result` return 保留的对象。
### os_release
&emsp;`os_release` 减少 os_object 的引用计数。
```c++
API_AVAILABLE(macos(10.10), ios(8.0))
OS_EXPORT
void OS_SWIFT_UNAVAILABLE("Can't be used with ARC")
os_release(void *object);
#if OS_OBJECT_USE_OBJC
// ObjC 下则是定义成一个宏，调用 release 函数
#undef os_release
#define os_release(object) [object release]
#endif
```
&emsp;在具有现代 Objective-C 运行时的平台上，这完全等同于向对象发送 -[release] 消息。

&emsp;`object` 要释放的对象。

&emsp;<os/object.h> 文件到这里就全部结束了，下面我们接着看 <dispatch/object.h> 文件。

&emsp;接下来是分别针对不同的情况（Swift/Objective-C/C++/C）定义了一些 DISPATCH 前缀开头的宏，而宏定义的内容其中 Swift/Objective-C 相关部分来自 <os/object.h> 中的宏，C++/C 部分是来自它们的语言环境。（例如 C++ 下的 `static_cast` 函数的调用、结构体的继承等，C 下的直接取地址强制转换、联合体的使用等）

## DISPATCH_DECL/DISPATCH_DECL_SUBCLASS/DISPATCH_GLOBAL_OBJECT
&emsp;`DISPATCH_DECL` 默认使用 `dispatch_object` 作为继承的父类，`DISPATCH_DECL_SUBCLASS` 则是自行指定父类，并且针对不同的语言环境作了不同的定义。

&emsp;这里我们看 C++ 和 C 环境下（GCD 源码内部是此环境）。
+ C++ 环境下：
&emsp;上面我们我们已经看过 `struct dispatch_object_s` 声明，已知 `dispatch_object_t` 是指向 `struct dispatch_object_s` 的指针。
```c++
#define DISPATCH_DECL(name) \
typedef struct name##_s : public dispatch_object_s {} *name##_t
```
&emsp;如上篇中的 `DISPATCH_DECL(dispatch_queue)` 在此则转换为:
```c++
typedef struct dispatch_queue_s : public dispatch_object_s {} *dispatch_queue_t;
```
&emsp;即 `dispatch_queue_t` 是指向 `dispatch_queue_s` 结构体的指针，`dispatch_queue_s` 则是公开继承自 `dispatch_object_s` 结构体。 
```c++
#define DISPATCH_DECL_SUBCLASS(name, base) \
typedef struct name##_s : public base##_s {} *name##_t
```
&emsp;`DISPATCH_DECL_SUBCLASS` 则是指定父类，上面 `DISPATCH_DECL` 是默认继承自 `dispatch_object_s` 结构体。
+ C（Plain C）环境下：
&emsp;上面我们已经看到 `dispatch_object_t` 是一个联合体。
```c++
#define DISPATCH_DECL(name) typedef struct name##_s *name##_t
```
&emsp;如上篇中的 `DISPATCH_DECL(dispatch_queue)` 则直接转换为 `typedef struct dispatch_queue_s *dispatch_queue_t`，即 `dispatch_queue_t` 是指向 `dispatch_queue_s` 结构体的指针，对比 C++，此处的 `dispatch_queue_t` 仅是一个结构体。
```c++
#define DISPATCH_DECL_SUBCLASS(name, base) typedef base##_t name##_t
#define DISPATCH_GLOBAL_OBJECT(type, object) ((type)&(object))
```
&emsp;`DISPATCH_DECL_SUBCLASS` 指定指针指向类型，`DISPATCH_GLOBAL_OBJECT` 则是直接取地址后强制指针类型转换。
## _dispatch_object_validate
&emsp;这个函数的目的是把传进来的 `object` 的首地址取出来，然后强转为 `void *`。（把结构体的首元素的首地址转换为 `isa` 使用吗？）
```c++
DISPATCH_INLINE DISPATCH_ALWAYS_INLINE DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
_dispatch_object_validate(dispatch_object_t object)
{
    void *isa = *(void *volatile*)(OS_OBJECT_BRIDGE void*)object;
    (void)isa;
}
```
## dispatch_retain
&emsp;`dispatch_retain` 增加调度对象（dispatch object）的引用计数。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
DISPATCH_SWIFT_UNAVAILABLE("Can't be used with ARC")
void
dispatch_retain(dispatch_object_t object);
#if OS_OBJECT_USE_OBJC_RETAIN_RELEASE
#undef dispatch_retain
// 调用 retain 函数
#define dispatch_retain(object) \
        __extension__({ dispatch_object_t _o = (object); \
        _dispatch_object_validate(_o); (void)[_o retain]; })
#endif
```
&emsp;对 `dispatch_retain` 的调用必须与对 `dispatch_release` 的调用保持平衡。

&emsp;`object` 要保留的对象。在此参数中传递 `NULL` 的结果是未定义的。
## dispatch_release
&emsp;`dispatch_release` 减少调度对象的引用计数。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
DISPATCH_SWIFT_UNAVAILABLE("Can't be used with ARC")
void
dispatch_release(dispatch_object_t object);
#if OS_OBJECT_USE_OBJC_RETAIN_RELEASE
#undef dispatch_release
// 调用 release 函数
#define dispatch_release(object) \
        __extension__({ dispatch_object_t _o = (object); \
        _dispatch_object_validate(_o); [_o release]; })
#endif
```
&emsp;一旦释放了所有引用（即引用计数变为零），就会异步释放 dispatch object。系统不保证给定的客户端（client）是对给定对象的最后或唯一引用。

&emsp;`object` 要释放的对象。在此参数中传递 `NULL` 的结果是未定义的。

## dispatch_get_context
&emsp;`dispatch_get_context` 返回应用程序定义的对象的上下文。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_PURE DISPATCH_WARN_RESULT
DISPATCH_NOTHROW
void *_Nullable
dispatch_get_context(dispatch_object_t object);
```
&emsp;`object` 在此参数中传递 `NULL` 的结果是未定义的。

&emsp;`object` 的上下文；可以为 `NULL`。
## dispatch_set_context
&emsp;`dispatch_set_context` 将应用程序定义的上下文与对象相关联。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NOTHROW
void
dispatch_set_context(dispatch_object_t object, void *_Nullable context);
```
&emsp;`object` 在此参数中传递 `NULL`的结果是未定义的。

&emsp;`context` 新的 client 定义对象的上下文。可能为 `NULL`。
## dispatch_set_finalizer_f
&emsp;`dispatch_set_finalizer_f` 为 dispatch object 设置终结（finalizer）函数。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NOTHROW
void
dispatch_set_finalizer_f(dispatch_object_t object,
        dispatch_function_t _Nullable finalizer);
```
&emsp;`object` 要修改的 dispatch object。在此参数中传递 `NULL` 的结果是未定义的。

&emsp;`finalizer` 终结（finalizer）函数的指针。

&emsp;在释放对对象的所有引用之后，将在对象的目标队列上调用调度对象的终结函数。应用程序可以使用此终结函数来释放与对象关联的任何资源，例如释放对象的上下文。传递给终结函数的 context 参数是在进行终结函数调用时调度对象的当前上下文。
## dispatch_activate
&emsp;`dispatch_activate` 激活指定的调度对象。
```c++
API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_activate(dispatch_object_t object);
```
&emsp;调度对象（例如队列（queues）和源（sources））可以在非活动状态下创建，必须先激活处于这种状态的对象，然后才能调用与它们关联的任何块（blocks）。

&emsp;可以使用 `dispatch_set_target_queue` 更改非活动对象的目标队列，一旦最初不活动的对象被激活，就不再允许更改目标队列。

&emsp;在活动对象上调用 `dispatch_activate` 无效，释放非活动对象上的最后一个引用计数是未定义的。

&emsp;`object` 要激活的对象。在此参数中传递 `NULL` 的结果是未定义的。
## dispatch_suspend
&emsp;`dispatch_suspend` 挂起调度对象上的块。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_suspend(dispatch_object_t object);
```
&emsp;挂起的对象将不会调用与其关联的任何块。与该对象关联的任何运行块（running block）完成之后，将发生对象的挂起。

&emsp;对 `dispatch_suspend` 的调用必须与对 `dispatch_resume` 的调用保持平衡。

&emsp;`object` 要暂停的对象。在此参数中传递 `NULL` 的结果是未定义的。
## dispatch_resume
&emsp;`dispatch_resume` 恢复调度对象上块的调用。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_resume(dispatch_object_t object);
```
&emsp;可以使用 `dispatch_suspend` 挂起 dispatch objects，这会增加内部挂起计数。 `dispatch_resume` 是逆运算，它消耗暂停计数。当最后一次暂停计数被消耗时，与该对象关联的块将再次被调用。

&emsp;出于向后兼容性的原因，在非活动且未暂停的调度源对象上的 `dispatch_resume` 与调用 `dispatch_activate` 具有相同的效果。对于新代码，首选使用 `dispatch_activate`。

&emsp;如果指定的对象的挂起计数为零并且不是非活动源，则此函数将导致断言并终止进程。

&emsp;`object` 要恢复的对象。在此参数中传递 `NULL` 的结果是未定义的。
## dispatch_set_qos_class_floor
```c++
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
DISPATCH_EXPORT DISPATCH_NOTHROW
void
dispatch_set_qos_class_floor(dispatch_object_t object,
        dispatch_qos_class_t qos_class, int relative_priority);
```
## dispatch_wait
&emsp;`dispatch_wait` 同步等待某个对象，或直到指定的超时时间已过。
```c++
DISPATCH_UNAVAILABLE
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NOTHROW
long
dispatch_wait(void *object, dispatch_time_t timeout);
#if __has_extension(c_generic_selections)
// 宏定义
#define dispatch_wait(object, timeout) \
        _Generic((object), \
            dispatch_block_t:dispatch_block_wait, \
            dispatch_group_t:dispatch_group_wait, \
            dispatch_semaphore_t:dispatch_semaphore_wait \
        )((object),(timeout))
#endif
```
&emsp;类型通用宏，根据第一个参数的类型，映射到 `dispatch_block_wait`，`dispatch_group_wait` 或 `dispatch_semaphore_wait` 此功能不适用于任何其他对象类型。

&emsp;`object` 要等待的对象。在此参数中传递 `NULL` 的结果是未定义的。

&emsp;`timeout` 何时超时，为方便起见，有 `DISPATCH_TIME_NOW` 和 `DISPATCH_TIME_FOREVER` 常量。

&emsp;`result` 成功返回零或错误返回非零（即超时）。
## dispatch_notify
&emsp;`dispatch_notify` 计划在完成指定对象的执行后将通知块提交给队列。
```c++
DISPATCH_UNAVAILABLE
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_notify(void *object, dispatch_object_t queue,
        dispatch_block_t notification_block);
#if __has_extension(c_generic_selections)
#define dispatch_notify(object, queue, notification_block) \
        _Generic((object), \
            dispatch_block_t:dispatch_block_notify, \
            dispatch_group_t:dispatch_group_notify \
        )((object),(queue), (notification_block))
#endif
```
&emsp;根据第一个参数的类型，映射到 `dispatch_block_notify` 或 `dispatch_group_notify` 的类型通用宏，此功能不适用于任何其他对象类型。

&emsp;`object` 要观察的对象。在此参数中传递 `NULL` 的结果是不确定的。

&emsp;`queue` 当观察对象完成时，将向其提交所提供的通知块的队列。

&emsp;`notification_block` 观察对象完成时要提交的块。
## dispatch_cancel
&emsp;`dispatch_cancel` 取消指定的对象。
```c++
DISPATCH_UNAVAILABLE
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_cancel(void *object);
#if __has_extension(c_generic_selections)
#define dispatch_cancel(object) \
        _Generic((object), \
            dispatch_block_t:dispatch_block_cancel, \
            dispatch_source_t:dispatch_source_cancel \
        )((object))
#endif
```
&emsp;根据第一个参数的类型映射到 `dispatch_block_cancel` 或 `dispatch_source_cancel` 的类型通用宏。此功能不适用于任何其他对象类型。

&emsp;`object` 要取消的对象。在此参数中传递 `NULL` 的结果是不确定的。
## dispatch_testcancel
&emsp;`dispatch_testcancel` 测试指定对象是否已被取消。
```c++
DISPATCH_UNAVAILABLE
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_WARN_RESULT DISPATCH_PURE
DISPATCH_NOTHROW
long
dispatch_testcancel(void *object);
#if __has_extension(c_generic_selections)
#define dispatch_testcancel(object) \
        _Generic((object), \
            dispatch_block_t:dispatch_block_testcancel, \
            dispatch_source_t:dispatch_source_testcancel \
        )((object))
#endif
```
&emsp;类型通用的宏，它映射到 `dispatch_block_testcancel` 或 `dispatch_source_testcancel`，具体取决于第一个参数的类型。此功能不适用于任何其他对象类型。

&emsp;`object` 要测试的对象。在此参数中传递 `NULL` 的结果是不确定的。
## dispatch_debug
&emsp;`dispatch_debug` 以编程方式记录有关调度对象的调试信息。
```c++
API_DEPRECATED("unsupported interface", macos(10.6,10.9), ios(4.0,6.0))
DISPATCH_EXPORT DISPATCH_NONNULL2 DISPATCH_NOTHROW DISPATCH_COLD
__attribute__((__format__(printf,2,3)))
void
dispatch_debug(dispatch_object_t object, const char *message, ...);
```
&emsp;以编程方式记录有关调度对象的调试信息。默认情况下，日志输出以通知级别发送到 syslog。在库的调试版本中，日志输出发送到 /var/tmp 中的文件。可以通过 `LIBDISPATCH_LOG` 环境变量配置日志输出目标。

&emsp;不建议使用此功能，以后的版本中将删除该功能。 Objective-C 调用者可以改用 `debugDescription`。

&emsp;<dispatch/object.h> 文件到这里就全部看完了。下面接着看另一个较简单的文件 <dispatch/group.h>。

## <dispatch/group.h>
&emsp;<dispatch/group.h> 文件的内容不多，仅仅包含几个与 `dispatch_group_t` 相关的函数，下面一起看一下吧。
### dispatch_group_t
&emsp;`dispatch_group_t` 表示提交给队列以进行异步调用的一组块。
```c++
DISPATCH_DECL(dispatch_group);
```
+ 在 Swift （在 Swift 中使用 Objective-C）下宏定义展开是:
```c++
OS_EXPORT OS_OBJECT_OBJC_RUNTIME_VISIBLE
@interface OS_dispatch_group : OS_dispatch_object
- (instancetype)init OS_SWIFT_UNAVAILABLE("Unavailable in Swift");
@end
typedef OS_dispatch_group * dispatch_group_t;
```
&emsp;`OS_dispatch_group` 是继承自 `OS_dispatch_object` 的类，然后 `dispatch_group_t` 是一个指向 `OS_dispatch_group` 的指针。
+ 在 Objective-C 下宏定义展开是:
```c++
@protocol OS_dispatch_group <OS_dispatch_object>
@end
typedef NSObject<OS_dispatch_group> * dispatch_group_t;
```
&emsp;`OS_dispatch_group` 是继承自 `OS_dispatch_object` 协议的协议，并且为遵循该协议的 `NSObject` 实例对象类型的指针定义了一个 `dispatch_group_t` 的别名。
+ 在 C++ 下宏定义展开是:
```c++
typedef struct dispatch_group_s : public dispatch_object_s {} * dispatch_group_t;
```
&emsp;`dispatch_group_t` 是一个指向 `dispatch_group_s` 结构体的指针。
+ 在 C（Plain C）下宏定义展开是:
```c++
typedef struct dispatch_group_s *dispatch_group_t
```
&emsp;`dispatch_group_t` 是指向 `struct dispatch_group_s` 的指针。
### dispatch_group_create
&emsp;`dispatch_group_create` 创建可以与块关联的新组。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_MALLOC DISPATCH_RETURNS_RETAINED DISPATCH_WARN_RESULT
DISPATCH_NOTHROW
dispatch_group_t
dispatch_group_create(void);
```
&emsp;此函数用于创建可与块关联的新组。调度组（dispatch group）可用于等待它引用的块的完成（所有的 blocks 异步执行完成）。使用 `dispatch_release` 释放组对象内存。

&emsp;`result` 新创建的组，如果失败，则为 `NULL`。
### dispatch_group_async
&emsp;`dispatch_group_async` 将一个块提交到调度队列，并将该块与给定的调度组关联。
```c++
#ifdef __BLOCKS__
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_group_async(dispatch_group_t group,
    dispatch_queue_t queue,
    dispatch_block_t block);
#endif /* __BLOCKS__ */
```
&emsp;将一个块提交到调度队列，并将该块与给定的调度组关联。调度组可用于等待其引用的块的完成。

&emsp;`group`：与提交的块关联的调度组。在此参数中传递 `NULL` 的结果是未定义的。

&emsp;`queue`：块将提交到该调度队列以进行异步调用。

&emsp;`block`：该块异步执行。
### dispatch_group_async_f
&emsp;`dispatch_group_async_f` 将函数提交给调度队列，并将该函数与给定的调度组关联。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NONNULL2 DISPATCH_NONNULL4
DISPATCH_NOTHROW
void
dispatch_group_async_f(dispatch_group_t group,
    dispatch_queue_t queue,
    void *_Nullable context,
    dispatch_function_t work);
```
&emsp;同上 `dispatch_group_async` 只是把 block 换为了函数。
### dispatch_group_wait
&emsp;`dispatch_group_wait` 同步等待，直到与一个组关联的所有块都已完成，或者直到指定的超时时间过去为止。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
long
dispatch_group_wait(dispatch_group_t group, dispatch_time_t timeout);
```
&emsp;该函数等待与给定调度组关联的块的完成，并在所有块完成或指定的超时时间结束后返回。（阻塞直到函数返回）

&emsp;如果没有与调度组关联的块（即该组为空），则此函数将立即返回。

&emsp;从多个线程同时使用同一调度组调用此函数的结果是不确定的。

&emsp;成功返回此函数后，调度组为空。可以使用 `dispatch_release` 释放它，也可以将其重新用于其他块。

&emsp;`group`：等待调度组。在此参数中传递 `NULL` 的结果是未定义的。

&emsp;`timeout`：何时超时（dispatch_time_t）。有 `DISPATCH_TIME_NOW` 和 `DISPATCH_TIME_FOREVER` 常量。

&emsp;`result`：成功返回零（与该组关联的所有块在指定的时间内完成），错误返回非零（即超时）。
### dispatch_group_notify
&emsp;`dispatch_group_notify` 当与组相关联的所有块都已完成时，计划将块提交到队列（即当与组相关联的所有块都已完成时，提交到 `queue` 的 `block` 将执行）。
```c++
#ifdef __BLOCKS__
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_group_notify(dispatch_group_t group,
    dispatch_queue_t queue,
    dispatch_block_t block);
#endif /* __BLOCKS__ */
```
&emsp;如果没有块与调度组相关联（即该组为空），则通知块将立即提交。

&emsp;通知块（`block`）提交到目标队列（`queue`）时，该组将为空。该组可以通过 `dispatch_release` 释放，也可以重新用于其他操作。

&emsp;`group`：观察的调度组。在此参数中传递 `NULL` 的结果是未定义的。

&emsp;`queue`：组完成后，将向其提交所提供的块的队列。

&emsp;`block`：组完成后要提交的 block。
### dispatch_group_notify_f
&emsp;`dispatch_group_notify_f` 与一个组关联的所有块均已完成后，计划将函数提交给队列。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NONNULL2 DISPATCH_NONNULL4
DISPATCH_NOTHROW
void
dispatch_group_notify_f(dispatch_group_t group,
    dispatch_queue_t queue,
    void *_Nullable context,
    dispatch_function_t work);
```
&emsp;同上 `dispatch_group_notify` 只是把 block 换为了函数。
### dispatch_group_enter
&emsp;`dispatch_group_enter` 表示组已手动输入块。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_group_enter(dispatch_group_t group);
```
&emsp;调用此函数表示另一个块已通过 `dispatch_group_async` 以外的其他方式加入了该组。对该函数的调用必须与 `dispatch_group_leave` 平衡。

&emsp;`group`：要更新的调度组。在此参数中传递 `NULL` 的结果是未定义的。
### dispatch_group_leave
&emsp;`dispatch_group_leave` 手动指示组中的块已完成。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_group_leave(dispatch_group_t group);
```
&emsp;调用此函数表示块已完成，并且已通过 `dispatch_group_async` 以外的其他方式离开了调度组。

&emsp;`group`：要更新的调度组。在此参数中传递 `NULL` 的结果是未定义的。

&emsp;<dispatch/group.h> 文件到这里就全部看完了。下面接着看另一个较简单的文件 <dispatch/semaphore.h>。
## <dispatch/semaphore.h>
&emsp;<dispatch/semaphore.h> 文件的内容不多，仅仅包含几个与 `dispatch_semaphore_t` 相关的函数，下面一起看一下吧。
### dispatch_semaphore_t
&emsp;`dispatch_semaphore_t` 表示计数信号量。主要用来控制并发任务的数量。
```c++
DISPATCH_DECL(dispatch_semaphore); // DISPATCH_DECL(dispatch_group);
```
+ 在 Swift（在 Swift 中使用 Objective-C） 下宏定义展开是:
```c++
OS_EXPORT OS_OBJECT_OBJC_RUNTIME_VISIBLE
@interface OS_dispatch_semaphore : OS_dispatch_object
- (instancetype)init OS_SWIFT_UNAVAILABLE("Unavailable in Swift");
@end
typedef OS_dispatch_semaphore * dispatch_semaphore_t;
```
&emsp;`OS_dispatch_semaphore` 是继承自 `OS_dispatch_object` 的类，然后 `dispatch_semaphore_t` 是一个指向 `OS_dispatch_semaphore` 的指针。
+ 在 Objective-C 下宏定义展开是:
```c++
@protocol OS_dispatch_semaphore <OS_dispatch_object>
@end
typedef NSObject<OS_dispatch_semaphore> * dispatch_semaphore_t;
```
&emsp;`OS_dispatch_semaphore` 是继承自 `OS_dispatch_object` 协议的协议，并且为遵循该协议的 `NSObject` 实例对象类型的指针定义了一个 `dispatch_semaphore_t` 的别名。
+ 在 C++ 下宏定义展开是:
```c++
typedef struct dispatch_semaphore_s : public dispatch_object_s {} * dispatch_semaphore_t;
```
&emsp;`dispatch_semaphore_t` 是一个指向 `dispatch_semaphore_s` 结构体的指针。
+ 在 C（Plain C）下宏定义展开是:
```c++
typedef struct dispatch_semaphore_s *dispatch_semaphore_t
```
&emsp;`dispatch_semaphore_t` 是指向 `struct dispatch_semaphore_s` 的指针。
### dispatch_semaphore_create
&emsp;`dispatch_semaphore_create` 用初始值（`long value`）创建新的计数信号量。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_MALLOC DISPATCH_RETURNS_RETAINED DISPATCH_WARN_RESULT
DISPATCH_NOTHROW
dispatch_semaphore_t
dispatch_semaphore_create(long value);
```
&emsp;当两个线程需要协调特定事件的完成时，将值传递为零非常有用。传递大于零的值对于管理有限的资源池非常有用，该资源池的大小等于该值。

&emsp;`value`：信号量的起始值。传递小于零的值将导致返回 `NULL`。

&emsp;`result`：新创建的信号量，失败时为 `NULL`。
### dispatch_semaphore_wait
&emsp;`dispatch_semaphore_wait` 等待（减少）信号量。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
long
dispatch_semaphore_wait(dispatch_semaphore_t dsema, dispatch_time_t timeout);
```
&emsp;减少计数信号量。如果结果值小于零，此函数将等待信号出现，然后返回。（可以使总信号量减 1，信号总量小于 0 时就会一直等待（阻塞所在线程），否则就可以正常执行。）

&emsp;`dsema`：信号量。在此参数中传递 `NULL` 的结果是未定义的。

&emsp;`timeout`：何时超时（dispatch_time）。为方便起见，有 `DISPATCH_TIME_NOW` 和 `DISPATCH_TIME_FOREVER` 常量。

&emsp;`result`：成功返回零，如果发生超时则返回非零。
### dispatch_semaphore_signal
&emsp;`dispatch_semaphore_signal` 发信号（增加）信号量。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
long
dispatch_semaphore_signal(dispatch_semaphore_t dsema);
```
&emsp;增加计数信号量。如果先前的值小于零，则此函数在返回之前唤醒等待的线程。

&emsp;`dsema`：在此参数中传递 `NULL` 的结果是未定义的。

&emsp;`result`：如果线程被唤醒，此函数将返回非零值。否则，返回零。

&emsp;<dispatch/semaphore.h> 文件到这里就全部看完了。

## 参考链接
**参考链接:🔗**
+ [swift-corelibs-libdispatch-main](https://github.com/apple/swift-corelibs-libdispatch)
+ [Dispatch 官方文档](https://developer.apple.com/documentation/dispatch?language=objc)
+ [iOS libdispatch浅析](https://juejin.im/post/6844904143174238221)
+ [GCD--百度百科词条](https://baike.baidu.com/item/GCD/2104053?fr=aladdin)
+ [iOS多线程：『GCD』详尽总结](https://juejin.im/post/6844903566398717960)
+ [iOS底层学习 - 多线程之GCD初探](https://juejin.im/post/6844904096973979656)
+ [GCD 中的类型](https://blog.csdn.net/u011374318/article/details/87870585)
+ [iOS Objective-C GCD之queue（队列）篇](https://www.jianshu.com/p/d0017f74f9ca)
+ [变态的libDispatch结构分析-object结构](https://blog.csdn.net/passerbysrs/article/details/18223845)
