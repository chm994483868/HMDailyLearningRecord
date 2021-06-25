# iOS 多线程知识体系构建(五)：GCD API（source.h、workloop.h、data.h）解析篇

> &emsp;那么继续学习 dispath 中也挺重要的 <dispatch/source.h> 文件。

## <dispatch/source.h>
&emsp;dispatch framework 提供了一套接口，用于监视低级系统对象（file descriptors（文件描述符）, Mach ports, signals, VFS nodes, etc.）的活动，并在此类活动发生时自动向 dispatch queues 提交事件处理程序块（event handler blocks）。这套接口称为 Dispatch Source API。
### dispatch_source_t
&emsp;`dispatch_source_t` 表示 dispatch sources 类型，调度源（dispatch sources）用于自动提交事件处理程序块（event handler blocks）到调度队列（dispatch queues）以响应外部事件。
```c++
DISPATCH_SOURCE_DECL(dispatch_source);
```
+ 在 Swift（在 Swift 中使用 Objective-C）下宏定义展开是:
```c++
OS_EXPORT OS_OBJECT_OBJC_RUNTIME_VISIBLE
@interface OS_dispatch_source : OS_dispatch_object
- (instancetype)init OS_SWIFT_UNAVAILABLE("Unavailable in Swift");
@end

typedef OS_dispatch_source * dispatch_source_t

@protocol OS_dispatch_source <NSObject>
@end

@interface OS_dispatch_source () <OS_dispatch_source>
@end
```
&emsp;`OS_dispatch_source` 是继承自 `OS_dispatch_object` 的类，然后 `dispatch_source_t` 是一个指向 `OS_dispatch_source` 的指针。
+ 在 Objective-C 下宏定义展开是:
```c++
@protocol OS_dispatch_source <OS_dispatch_object>
@end
typedef NSObject<OS_dispatch_source> * dispatch_source_t;
```
&emsp;`OS_dispatch_source` 是继承自 `OS_dispatch_object` 协议的协议，并且为遵循该协议的 `NSObject` 实例对象类型的指针定义了一个 `dispatch_source_t` 的别名。
+ 在 C++ 下宏定义展开是:
```c++
typedef struct dispatch_source_s : public dispatch_object_s {} * dispatch_source_t;
```
&emsp;`dispatch_source_t` 是一个指向 `dispatch_source_s` 结构体的指针。
+ 在 C（Plain C）下宏定义展开是:
```c++
typedef struct dispatch_source_s *dispatch_source_t
```
&emsp;`dispatch_source_t` 是指向 `struct dispatch_source_s` 的指针。
### dispatch_source_type_t
&emsp;`dispatch_source_type_t` 定义类型别名。此类型的常量表示调度源（dispatch source）正在监视的低级系统对象的类（class of low-level system object）。此类型的常量作为参数传递给 `dispatch_source_create` 函数并确定如何解释 handle 参数（handle argument ）（i.e. as a file descriptor（文件描述符）, mach port, signal number, process identifier, etc.）以及如何解释 mask 参数（mask argument）。
```c++
typedef const struct dispatch_source_type_s *dispatch_source_type_t;
```
### DISPATCH_EXPORT
&emsp;不同平台下的 `extern` 外联标识。
```c++
#if defined(_WIN32)
#if defined(__cplusplus)
#define DISPATCH_EXPORT extern "C" __declspec(dllimport)
#else
#define DISPATCH_EXPORT extern __declspec(dllimport)
#endif
#elif __GNUC__
#define DISPATCH_EXPORT extern __attribute__((visibility("default")))
#else
#define DISPATCH_EXPORT extern
#endif
```
### DISPATCH_SOURCE_TYPE_DECL
&emsp;先学习展开一下 `DISPATCH_SOURCE_TYPE_DECL` 宏定义，下面多处都要用到它。
```c++
DISPATCH_SOURCE_TYPE_DECL(data_add);
```
+ 在 Swift（在 Swift 中使用 Objective-C）下宏定义展开是:
```c++
extern struct dispatch_source_type_s _dispatch_source_type_data_add;
@protocol OS_dispatch_source_data_add <OS_dispatch_source>
@end

@interface OS_dispatch_source () <OS_dispatch_source_data_add>
@end
```
+ 在 Objective-C/C++/C 下宏定义展开是:
```c++
extern const struct dispatch_source_type_s _dispatch_source_type_data_add;
```
### DISPATCH_SOURCE_TYPE_DATA_ADD
&emsp;一种调度源（dispatch source），它合并通过调用 `dispatch_source_merge_data` 获得的数据。ADD 用于合并数据。句柄未使用（暂时传递零），mask 未使用（暂时传递零）。
```c++
#define DISPATCH_SOURCE_TYPE_DATA_ADD (&_dispatch_source_type_data_add)
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_SOURCE_TYPE_DECL(data_add);
```
### DISPATCH_SOURCE_TYPE_DATA_OR
&emsp;一种调度源（dispatch source），它合并通过调用 `dispatch_source_merge_data` 获得的数据。按位或进行合并数据。句柄未使用（暂时传递零），mask 未使用（暂时传递零）。
```c++
#define DISPATCH_SOURCE_TYPE_DATA_OR (&_dispatch_source_type_data_or)
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_SOURCE_TYPE_DECL(data_or);
```
### DISPATCH_SOURCE_TYPE_DATA_REPLACE
&emsp;一种调度源（dispatch source），它跟踪通过调用 `dispatch_source_merge_data` 获得的数据。新获得的数据值替换了尚未传递到源处理程序（source handler）的现有数据值。数据值为零将不调用源处理程序（source handler）。句柄未使用（暂时传递零），mask 未使用（暂时传递零）。
```c++
#define DISPATCH_SOURCE_TYPE_DATA_REPLACE (&_dispatch_source_type_data_replace)
API_AVAILABLE(macos(10.13), ios(11.0), tvos(11.0), watchos(4.0))
DISPATCH_SOURCE_TYPE_DECL(data_replace);
```
### DISPATCH_SOURCE_TYPE_MACH_SEND
&emsp;一种调度源（dispatch source），用于监视 Mach port 的 dead name 通知（发送权限不再具有任何相应的接收权限）。句柄（handle）是一个 Mach port，具有 send 或 send once right（mach_port_t）。mask 是 `dispatch_source_mach_send_flags_t` 中所需事件的 mask。
```c++
#define DISPATCH_SOURCE_TYPE_MACH_SEND (&_dispatch_source_type_mach_send)
API_AVAILABLE(macos(10.6), ios(4.0)) DISPATCH_LINUX_UNAVAILABLE()
DISPATCH_SOURCE_TYPE_DECL(mach_send);
```
### DISPATCH_SOURCE_TYPE_MACH_RECV
&emsp;一种调度源（dispatch source），用于监视 Mach port 中的挂起消息。句柄（handle）是具有接收权限（mach_port_t）的 Mach port。mask 是来自 `dispatch_source_mach_recv_flags_t` 中所需事件的 mask，但是当前未定义任何标志（现在传递零）。
```c++
#define DISPATCH_SOURCE_TYPE_MACH_RECV (&_dispatch_source_type_mach_recv)
API_AVAILABLE(macos(10.6), ios(4.0)) DISPATCH_LINUX_UNAVAILABLE()
DISPATCH_SOURCE_TYPE_DECL(mach_recv);
```
### DISPATCH_SOURCE_TYPE_MEMORYPRESSURE
&emsp;一种调度源（dispatch source），用于监视系统内存压力状况的变化。该句柄（handle）未使用（现在传递零）。mask 是来自 `dispatch_source_mach_recv_flags_t` 中所需事件的 mask。
```c++
#define DISPATCH_SOURCE_TYPE_MEMORYPRESSURE \
        (&_dispatch_source_type_memorypressure)
API_AVAILABLE(macos(10.9), ios(8.0)) DISPATCH_LINUX_UNAVAILABLE()
DISPATCH_SOURCE_TYPE_DECL(memorypressure);
```
### DISPATCH_SOURCE_TYPE_PROC
&emsp;一种调度源（dispatch source），用于监视外部进程中由 `dispatch_source_proc_flags_t` 定义的事件。句柄（handle）是进程标识符（pid_t）。mask 是来自 `dispatch_source_mach_recv_flags_t` 中所需事件的 mask。
```c++
#define DISPATCH_SOURCE_TYPE_PROC (&_dispatch_source_type_proc)
API_AVAILABLE(macos(10.6), ios(4.0)) DISPATCH_LINUX_UNAVAILABLE()
DISPATCH_SOURCE_TYPE_DECL(proc);
```
### DISPATCH_SOURCE_TYPE_READ
&emsp;一种调度源（dispatch source），用于监视文件描述符的待处理字节，以获取可读取的字节。句柄（handle）是文件描述符（int）。mask 未使用（现在传递零）。
```c++
#define DISPATCH_SOURCE_TYPE_READ (&_dispatch_source_type_read)
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_SOURCE_TYPE_DECL(read);
```
### DISPATCH_SOURCE_TYPE_SIGNAL
&emsp;监视当前进程以获取信号的调度源（dispatch source）。句柄（handle）是信号编号（int）。mask 未使用（现在传递零）。
```c++
#define DISPATCH_SOURCE_TYPE_SIGNAL (&_dispatch_source_type_signal)
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_SOURCE_TYPE_DECL(signal);
```
### DISPATCH_SOURCE_TYPE_TIMER
&emsp;基于计时器（based on a timer）提交（submits）事件处理程序块（event handler block）的调度源（dispatch source）。句柄（handle）未使用（现在传递零）。mask 指定要应用的来自 `dispatch_source_timer_flags_t` 的标志。
```c++
#define DISPATCH_SOURCE_TYPE_TIMER (&_dispatch_source_type_timer)
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_SOURCE_TYPE_DECL(timer);
```
### DISPATCH_SOURCE_TYPE_VNODE
&emsp;一种调度源（dispatch source），它监视由 `dispatch_source_vnode_flags_t` 定义的事件的文件描述符。句柄（handle）是文件描述符（int）。mask 是来自 `dispatch_source_vnode_flags_t` 的所需事件的 mask。
```c++
#define DISPATCH_SOURCE_TYPE_VNODE (&_dispatch_source_type_vnode)
API_AVAILABLE(macos(10.6), ios(4.0)) DISPATCH_LINUX_UNAVAILABLE()
DISPATCH_SOURCE_TYPE_DECL(vnode);
```
### DISPATCH_SOURCE_TYPE_WRITE
&emsp;一种调度源（dispatch source），它监视文件描述符以获取可用于写入字节的缓冲区空间。句柄（handle）是文件描述符（int）。mask 未使用（现在传递零）。
```c++
#define DISPATCH_SOURCE_TYPE_WRITE (&_dispatch_source_type_write)
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_SOURCE_TYPE_DECL(write);
```
### dispatch_source_mach_send_flags_t
&emsp;`dispatch_source_mach_send` 标志的类型。
```c++
// 表示与给定发送权限对应的接收权限已销毁。
#define DISPATCH_MACH_SEND_DEAD    0x1

typedef unsigned long dispatch_source_mach_send_flags_t;
```
### dispatch_source_mach_recv_flags_t
&emsp;`dispatch_source_mach_recv` 标志的类型。
```c++
typedef unsigned long dispatch_source_mach_recv_flags_t;
```
### dispatch_source_memorypressure_flags_t
&emsp;`dispatch_source_memorypressure` 标志的类型。

+ `DISPATCH_MEMORYPRESSURE_NORMAL` 系统内存压力状况已恢复正常。
+ `DISPATCH_MEMORYPRESSURE_WARN` 系统内存压力状况已更改为警告。
+ `DISPATCH_MEMORYPRESSURE_CRITICAL` 系统内存压力状况已变为严重。
&emsp;内存压力升高是一种系统范围内的情况，为此源注册的应用程序应通过更改其将来的内存使用行为来作出反应，例如：通过减少新启动操作的缓存大小，直到内存压力恢复正常。
&emsp;注意：当系统内存压力进入提升状态时，应用程序不应遍历并丢弃现有缓存以进行过去的操作，因为这很可能会触发 VM 操作，从而进一步加剧系统内存压力。
```c++
#define DISPATCH_MEMORYPRESSURE_NORMAL        0x01
#define DISPATCH_MEMORYPRESSURE_WARN        0x02
#define DISPATCH_MEMORYPRESSURE_CRITICAL    0x04

typedef unsigned long dispatch_source_memorypressure_flags_t;
```
### dispatch_source_proc_flags_t
&emsp;`dispatch_source_proc` 标志的类型。

+ `DISPATCH_PROC_EXIT` 该进程已经退出（也许是 cleanly，也许不是）。
+ `DISPATCH_PROC_FORK` 该进程已创建一个或多个子进程。
+ `DISPATCH_PROC_EXEC` 通过 `exec *()` 或 `posix_spawn *()`，该进程已成为另一个可执行映像（executable image）。
+ `DISPATCH_PROC_SIGNAL` Unix 信号已传递到该进程。
```c++
#define DISPATCH_PROC_EXIT        0x80000000
#define DISPATCH_PROC_FORK        0x40000000
#define DISPATCH_PROC_EXEC        0x20000000
#define DISPATCH_PROC_SIGNAL    0x08000000

typedef unsigned long dispatch_source_proc_flags_t;
```
### dispatch_source_vnode_flags_t
&emsp;`dispatch_source_vnode` 标志的类型。

+ `DISPATCH_VNODE_DELETE` filesystem 对象已从 namespace 中删除。
+ `DISPATCH_VNODE_WRITE` filesystem 对象数据已更改。
+ `DISPATCH_VNODE_EXTEND` filesystem 对象的大小已更改。
+ `DISPATCH_VNODE_ATTRIB` filesystem 对象 metadata 已更改。
+ `DISPATCH_VNODE_LINK` filesystem 对象 link计数已更改。
+ `DISPATCH_VNODE_RENAME` filesystem 对象在 namespace 中被重命名。
+ `DISPATCH_VNODE_REVOKE` filesystem 对象被 revoked。
+ `DISPATCH_VNODE_FUNLOCK` filesystem 对象已解锁。

```c++
#define DISPATCH_VNODE_DELETE    0x1
#define DISPATCH_VNODE_WRITE    0x2
#define DISPATCH_VNODE_EXTEND    0x4
#define DISPATCH_VNODE_ATTRIB    0x8
#define DISPATCH_VNODE_LINK        0x10
#define DISPATCH_VNODE_RENAME    0x20
#define DISPATCH_VNODE_REVOKE    0x40
#define DISPATCH_VNODE_FUNLOCK    0x100

typedef unsigned long dispatch_source_vnode_flags_t;
```
### dispatch_source_timer_flags_t
&emsp;`dispatch_source_timer` 标志的类型。
+ `DISPATCH_TIMER_STRICT` 指定系统应尽最大努力严格遵守通过 `dispatch_source_set_timer` 为计时器指定的 leeway value，even if that value is smaller than the default leeway value that would be applied to the timer otherwise. 即使指定了此标志，也会有 minimal amount of leeway 应用于计时器。注意：使用此标志可能会覆盖系统采用的节电（power-saving）技术，并导致更高的功耗，因此必须谨慎使用它，并且仅在绝对必要时使用。
```c++
#define DISPATCH_TIMER_STRICT 0x1

typedef unsigned long dispatch_source_timer_flags_t;
```
### dispatch_source_create
&emsp;`dispatch_source_create` 创建一个新的调度源（dispatch source）来监视低级系统对象（low-level system objects），并根据事件自动将处理程序块（handler block）提交给调度队列（dispatch queue）。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_MALLOC DISPATCH_RETURNS_RETAINED DISPATCH_WARN_RESULT
DISPATCH_NOTHROW
dispatch_source_t
dispatch_source_create(dispatch_source_type_t type,
    uintptr_t handle,
    unsigned long mask,
    dispatch_queue_t _Nullable queue);
```
&emsp;Dispatch sources 不可重入。在调度源被挂起或事件处理程序块当前正在执行时，接收到的任何事件都将在调度源恢复或事件处理程序块返回后合并和传递。

&emsp;`Dispatch sources` 在非活动状态下创建。创建源并设置任何所需的属性（即处理程序，上下文等）之后，必须调用 `dispatch_activate` 才能开始事件传递。

&emsp;一旦被激活，就不允许在源上调用 `dispatch_set_target_queue`（参阅 `dispatch_activate` 和 `dispatch_set_target_queue`）。

&emsp;出于向后兼容性的原因，在非活动且未暂停的源上的 `dispatch_resume` 与调用 `dispatch_activate` 具有相同的效果。对于新代码，首选使用 `dispatch_activate`。

&emsp;`type`：声明调度源的类型。必须是已定义的 `dispatch_source_type_t` 常量之一。

&emsp;`handle`：要监视的基础系统句柄（handle）。此参数的解释由 `type` 参数中提供的常量确定。

&emsp;`mask`：指定所需事件的标志 mask。此参数的解释由 `type` 参数中提供的常量确定。

&emsp;`queue`：事件处理程序块（event handler block）将提交到的调度队列。如果队列为 `DISPATCH_TARGET_QUEUE_DEFAULT`，则源（source）将事件处理程序块提交到默认优先级全局队列。

&emsp;`result`：新创建的调度源（dispatch source）。如果传递了无效的参数，则为 `NULL`。
### dispatch_source_set_event_handler
&emsp;为给定的调度源（dispatch source）设置事件处理程序块（event handler block）。
```c++
#ifdef __BLOCKS__
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NOTHROW
void
dispatch_source_set_event_handler(dispatch_source_t source,
    dispatch_block_t _Nullable handler);
#endif /* __BLOCKS__ */
```
&emsp;`source`：要进行修改的调度源。在此参数中传递 `NULL` 的结果是未定义的

&emsp;`handler`：事件处理程序块将提交到源的目标队列。
### dispatch_source_set_event_handler_f
&emsp;为给定的调度源设置事件处理函数。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NOTHROW
void
dispatch_source_set_event_handler_f(dispatch_source_t source,
    dispatch_function_t _Nullable handler);
```
&emsp;`handler`：事件处理程序函数提交到源的目标队列。传递给事件处理程序（函数）的 context 参数是设置事件处理程序时当前调度源的上下文。

### dispatch_source_set_cancel_handler
&emsp;为给定的调度源设置取消处理程序块（cancellation handler block）。
```c++
#ifdef __BLOCKS__
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NOTHROW
void
dispatch_source_set_cancel_handler(dispatch_source_t source,
    dispatch_block_t _Nullable handler);
#endif /* __BLOCKS__ */
```
&emsp;一旦系统释放了对源基础句柄的所有引用，并且返回了源的事件处理程序块，则取消处理程序（如果已指定）将被提交到源的目标队列，以响应对 `dispatch_source_cancel` 的调用。

&emsp;IMPORTANT：file descriptor 和基于 mach port 的源需要源取消（source cancellation）和取消处理程序（a cancellation handler），以便安全地关闭描述符或销毁端口。在调用取消处理程序之前关闭描述符或端口可能会导致竞态。如果在源的事件处理程序仍在运行时为新描述符分配了与最近关闭的描述符相同的值，则事件处理程序可能会将数据读/写到错误的描述符。

&emsp;`handler`：取消处理程序块将提交到源的目标队列。
### dispatch_source_set_cancel_handler_f
&emsp;设置给定调度源的取消处理函数。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NOTHROW
void
dispatch_source_set_cancel_handler_f(dispatch_source_t source,
    dispatch_function_t _Nullable handler);
```
&emsp;同上 `dispatch_source_set_cancel_handler` 函数。
### dispatch_source_cancel
&emsp;异步取消调度源（dispatch source），以防止进一步调用其事件处理程序块。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_source_cancel(dispatch_source_t source);
```
&emsp;取消操作（dispatch_source_cancel）将阻止对指定调度源的事件处理程序块（event handler block）的任何进一步调用，但不会中断已在进行中的事件处理程序块。

&emsp;一旦源的事件处理程序完成，取消处理程序将提交到源的目标队列，这表明现在可以安全地关闭源的句柄（i.e. file descriptor or mach port）。
### dispatch_source_testcancel
&emsp;测试给定的调度源是否已取消。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_WARN_RESULT DISPATCH_PURE
DISPATCH_NOTHROW
long
dispatch_source_testcancel(dispatch_source_t source);
```
&emsp;`result`：取消则非零，未取消则为零。
### dispatch_source_get_handle
&emsp;返回与此调度源关联的基础系统句柄（underlying system handle）。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_WARN_RESULT DISPATCH_PURE
DISPATCH_NOTHROW
uintptr_t
dispatch_source_get_handle(dispatch_source_t source);
```
&emsp;返回值应根据调度源的类型进行解释，并且可以是以下句柄之一:
```c++
DISPATCH_SOURCE_TYPE_DATA_ADD:        n/a
DISPATCH_SOURCE_TYPE_DATA_OR:         n/a
DISPATCH_SOURCE_TYPE_DATA_REPLACE:    n/a
DISPATCH_SOURCE_TYPE_MACH_SEND:       mach port (mach_port_t)
DISPATCH_SOURCE_TYPE_MACH_RECV:       mach port (mach_port_t)
DISPATCH_SOURCE_TYPE_MEMORYPRESSURE   n/a
DISPATCH_SOURCE_TYPE_PROC:            process identifier (pid_t)
DISPATCH_SOURCE_TYPE_READ:            file descriptor (int)
DISPATCH_SOURCE_TYPE_SIGNAL:          signal number (int)
DISPATCH_SOURCE_TYPE_TIMER:           n/a
DISPATCH_SOURCE_TYPE_VNODE:           file descriptor (int)
DISPATCH_SOURCE_TYPE_WRITE:           file descriptor (int)
```
### dispatch_source_get_mask
&emsp;返回由调度源监视的事件的 mask。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_WARN_RESULT DISPATCH_PURE
DISPATCH_NOTHROW
unsigned long
dispatch_source_get_mask(dispatch_source_t source);
```
&emsp;`result`：返回值应根据调度源的类型进行解释，并且可以是以下 flag 之一：
```c++
DISPATCH_SOURCE_TYPE_DATA_ADD:        n/a
DISPATCH_SOURCE_TYPE_DATA_OR:         n/a
DISPATCH_SOURCE_TYPE_DATA_REPLACE:    n/a
DISPATCH_SOURCE_TYPE_MACH_SEND:       dispatch_source_mach_send_flags_t
DISPATCH_SOURCE_TYPE_MACH_RECV:       dispatch_source_mach_recv_flags_t
DISPATCH_SOURCE_TYPE_MEMORYPRESSURE   dispatch_source_memorypressure_flags_t
DISPATCH_SOURCE_TYPE_PROC:            dispatch_source_proc_flags_t
DISPATCH_SOURCE_TYPE_READ:            n/a
DISPATCH_SOURCE_TYPE_SIGNAL:          n/a
DISPATCH_SOURCE_TYPE_TIMER:           dispatch_source_timer_flags_t
DISPATCH_SOURCE_TYPE_VNODE:           dispatch_source_vnode_flags_t
DISPATCH_SOURCE_TYPE_WRITE:           n/a
```
### dispatch_source_get_data
&emsp;返回调度源的待处理数据。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_WARN_RESULT DISPATCH_PURE
DISPATCH_NOTHROW
unsigned long
dispatch_source_get_data(dispatch_source_t source);
```
&emsp;该函数旨在从事件处理程序块中调用。在事件处理程序回调之外调用此函数的结果是未定义的。

&emsp;`result`：返回值应根据调度源的类型进行解释，并且可以是以下之一：
```c++
DISPATCH_SOURCE_TYPE_DATA_ADD:        application defined data
DISPATCH_SOURCE_TYPE_DATA_OR:         application defined data
DISPATCH_SOURCE_TYPE_DATA_REPLACE:    application defined data
DISPATCH_SOURCE_TYPE_MACH_SEND:       dispatch_source_mach_send_flags_t
DISPATCH_SOURCE_TYPE_MACH_RECV:       dispatch_source_mach_recv_flags_t
DISPATCH_SOURCE_TYPE_MEMORYPRESSURE   dispatch_source_memorypressure_flags_t
DISPATCH_SOURCE_TYPE_PROC:            dispatch_source_proc_flags_t
DISPATCH_SOURCE_TYPE_READ:            estimated bytes available to read
DISPATCH_SOURCE_TYPE_SIGNAL:          number of signals delivered since the last handler invocation
DISPATCH_SOURCE_TYPE_TIMER:           number of times the timer has fired since the last handler invocation
DISPATCH_SOURCE_TYPE_VNODE:           dispatch_source_vnode_flags_t
DISPATCH_SOURCE_TYPE_WRITE:           estimated buffer space available
```
### dispatch_source_merge_data
&emsp;将数据合并到类型为 `DISPATCH_SOURCE_TYPE_DATA_ADD`，`DISPATCH_SOURCE_TYPE_DATA_OR` 或 `DISPATCH_SOURCE_TYPE_DATA_REPLACE` 的调度源中，并将其事件处理程序块提交到其目标队列。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_source_merge_data(dispatch_source_t source, unsigned long value);
```
&emsp;`value`：使用调度源类型指定的逻辑 OR 或 ADD 与待处理数据合并的值。零值无效并且也不会导致事件处理程序块的提交。

&emsp;这里插入一下，看一下 <dispatch/time.h> 文件的两个函数。
### dispatch_time
&emsp;相对于默认时钟或 wall time clock（墙上时钟）的当前值，创建一个 `dispatch_time_t`，或修改现有的 `dispatch_time_t`。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_WARN_RESULT DISPATCH_NOTHROW
dispatch_time_t
dispatch_time(dispatch_time_t when, int64_t delta);
```
&emsp;在 Apple 平台上，默认时钟基于 `mach_absolute_time`。

&emsp;`when`：可选的 `dispatch_time_t`，用于添加纳秒。如果传递了 `DISPATCH_TIME_NOW`，则 `dispatch_time` 将使用默认时钟（该时钟基于 Apple 平台上的 `mach_absolute_time`）。如果使用 `DISPATCH_WALLTIME_NOW`，则 `dispatch_time` 将使用 `gettimeofday(3)` 返回的值。 `dispatch_time(DISPATCH_WALLTIME_NOW，delta)` 等效于 `dispatch_walltime(NULL，delta)`。

&emsp;`delta`：纳秒级添加。
### dispatch_walltime
&emsp;使用壁钟（wall clock）创建一个 `dispatch_time_t`。
```c++

_STRUCT_TIMESPEC
{
    __darwin_time_t tv_sec;
    long            tv_nsec;
};

API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_WARN_RESULT DISPATCH_NOTHROW
dispatch_time_t
dispatch_walltime(const struct timespec *_Nullable when, int64_t delta);
```
&emsp;在 macOS 上，wall clock 基于 `gettimeofday(3)`。

&emsp;`when`：要添加时间的结构 `timespec`。如果传递了 `NULL`，则 `dispatch_walltime` 将使用 `gettimeofday(3)` 的结果。`dispatch_walltime(NULL，delta)` 返回与 `dispatch_time(DISPATCH_WALLTIME_NOW，delta)` 相同的值。
### dispatch_source_set_timer
&emsp;设置 timer source 的开始时间，间隔和回程值（leeway value）。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_source_set_timer(dispatch_source_t source,
    dispatch_time_t start,
    uint64_t interval,
    uint64_t leeway);
```
&emsp;此函数返回后，将清除先前计时器值累积的所有未决源数据；计时器的下一次触发将在 `start` 时发生，此后每隔 `interval` 纳秒，直到计时器源被取消。

&emsp;系统可能会延迟计时器的任何触发时间，以改善功耗和系统性能。允许延迟的上限可以使用 `leeway` 参数进行配置，下限在系统的控制下。

&emsp;对于 `start` 时的初始计时器触发，允许延迟的上限设置为 `leeway` 纳秒。对于随后的计时器以 `start + N * interval` 触发的情况，上限为 `MIN（leeway，interval / 2）`。

&emsp;允许延迟的下限可能随过程状态（例如应用程序 UI 的可见性）而变化。如果指定的计时器源是使用 `DISPATCH_TIMER_STRICT` 的 mask 创建的，则系统将尽最大努力严格遵守所提供的 `leeway` 值，即使该值小于当前下限。请注意，即使指定了此标志，也希望有最小的延迟量。

&emsp;`start` 参数还确定将使用哪个时钟作为计时器：如果 `start` 是 `DISPATCH_TIME_NOW` 或由 `dispatch_time(3)` 创建的，则计时器基于正常运行时间（从 Apple 平台上的 `mach_absolute_time` 获取） 。如果使用 `dispatch_walltime(3)` 创建了 `start`，则计时器基于 `gettimeofday(3)`。

&emsp;如果 timer source 已被取消，则调用此函数无效。

&emsp;`start`：计时器的开始时间。参考 `dispatch_time()` 和 `dispatch_walltime()`。

&emsp;`interval`：计时器的纳秒间隔。将 `DISPATCH_TIME_FOREVER` 用于一键式计时器（a one-shot timer）。

&emsp;`leeway`：timer 的纳秒 leeway。
### dispatch_source_set_registration_handler
&emsp;设置给定调度源的注册处理程序块（registration handler block）。
```c++
#ifdef __BLOCKS__
API_AVAILABLE(macos(10.7), ios(4.3))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NOTHROW
void
dispatch_source_set_registration_handler(dispatch_source_t source,
    dispatch_block_t _Nullable handler);
#endif /* __BLOCKS__ */
```
&emsp;一旦相应的 `kevent` 已在源中的初始 `dispatch_resume` 之后向系统注册，注册处理程序（如果已指定）将被提交到源的目标队列。

&emsp;如果在设置注册处理程序时已经注册了源，则会立即调用注册处理程序。

&emsp;`handler`：注册处理程序块将提交到源的目标队列。
### dispatch_source_set_registration_handler_f
&emsp;设置给定调度源（dispatch source）的注册处理函数。
```c++
API_AVAILABLE(macos(10.7), ios(4.3))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NOTHROW
void
dispatch_source_set_registration_handler_f(dispatch_source_t source,
    dispatch_function_t _Nullable handler);
```
&emsp;同上 `dispatch_source_set_registration_handler`。

&emsp;<dispatch/source.h> 文件到这里就全部看完了。下面接着看另一个文件 <dispatch/workloop.h>。
## <dispatch/workloop.h>
&emsp;调度工作循环（dispatch workloops），是 `dispatch_queue_t` 的子类。
### dispatch_workloop_t
&emsp;调度工作循环（dispatch workloops）按优先级调用提交给它们的工作项。（`dispatch_workloop_t` 继承自 `dispatch_queue_t`）。
```c++
DISPATCH_DECL_SUBCLASS(dispatch_workloop, dispatch_queue);
```
+ 在 Swift （在 Swift 中使用 Objective-C）下宏定义展开是:
```c++
OS_EXPORT OS_OBJECT_OBJC_RUNTIME_VISIBLE
@interface OS_dispatch_workloop : OS_dispatch_queue
- (instancetype)init OS_SWIFT_UNAVAILABLE("Unavailable in Swift");
@end

typedef OS_dispatch_workloop * dispatch_workloop_t;
```
&emsp;`OS_dispatch_workloop` 是继承自 `OS_dispatch_queue` 的类，然后 `dispatch_workloop_t` 是指向 `OS_dispatch_workloop` 的指针。
+ 在 Objective-C 下宏定义展开是:
```c++
@protocol OS_dispatch_workloop <OS_dispatch_queue>
@end

typedef NSObject<OS_dispatch_workloop> * dispatch_workloop_t;
```
&emsp;`OS_dispatch_workloop` 是继承自 `OS_dispatch_queue` 协议的协议，并且为遵循该协议的 `NSObject` 实例对象类型的指针定义了一个 `dispatch_workloop_t` 的别名。
+ 在 C++ 下宏定义展开是:
```c++
typedef struct dispatch_workloop_s : public dispatch_queue_s {} *dispatch_workloop_t;
```
&emsp;`dispatch_workloop_t` 是一个指向 `dispatch_workloop_s` 结构体的指针。
+ 在 C（Plain C）下宏定义展开是:
```c++
typedef struct dispatch_queue_t *dispatch_workloop_t
```
&emsp;`dispatch_group_t` 是指向 `struct dispatch_group_s` 的指针。

&emsp;调度工作循环（dispatch workloop）是 `dispatch_queue_t` 的一种形式，它是优先排序的队列（使用提交的工作项的 QOS 类作为排序依据）。

&emsp;在每次调用 workitem 之间，workloop 将评估是否有更高优先级的工作项直接提交给 workloop 或任何以 workloop 为目标的队列，并首先执行这些工作项。

&emsp;针对 workloop 的 serial queues 维护其工作项的 FIFO 执行。但是，workloop 可以基于它们的优先级，将提交给以其为目标的独立串行队列（independent serial queues）的工作项彼此重新排序，同时保留关于每个串行队列的 FIFO 执行。

&emsp;dispatch workloop 是 `dispatch_queue_t` 的 “subclass” ，可以将其传递给所有接受 dispatch queue 的 API，但 `dispatch_sync` 系列中的函数除外。 `dispatch_async_and_wait` 必须用于 workloop 对象。以 workloop 为目标的队列上的 `dispatch_sync` 系列函数仍被允许，但出于性能原因不建议使用。
### dispatch_workloop_create
&emsp;创建一个新的调度工作循环（dispatch workloop），可以向其提交工作项（workitems）。
```c++
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
DISPATCH_EXPORT DISPATCH_MALLOC DISPATCH_RETURNS_RETAINED DISPATCH_WARN_RESULT
DISPATCH_NOTHROW
dispatch_workloop_t
dispatch_workloop_create(const char *_Nullable label);
```
&emsp;`label`：附加到工作循环（workloop）的字符串标签。

&emsp;`result`：新创建的调度工作循环（dispatch workloop）。
### dispatch_workloop_create_inactive
&emsp;创建一个可以设置后续激活（setup and then activated）的新的非活动调度工作循环（dispatch workloop）。
```c++
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
DISPATCH_EXPORT DISPATCH_MALLOC DISPATCH_RETURNS_RETAINED DISPATCH_WARN_RESULT
DISPATCH_NOTHROW
dispatch_workloop_t
dispatch_workloop_create_inactive(const char *_Nullable label);
```
&emsp;创建一个不活动的 workloop 可以使其在激活之前接受进一步的配置，并可以向其提交工作项。

&emsp;将工作项（workitems）提交到无效的工作循环（inactive workloop）是未定义的，这将导致过程终止。

&emsp;`label`：附加到 workloop 的字符串标签。

&emsp;`result`：新创建的调度工作循环。
### dispatch_workloop_set_autorelease_frequency
&emsp;设置 workloop 的自动释放频率（autorelease frequency）。
```c++
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_workloop_set_autorelease_frequency(dispatch_workloop_t workloop,
        dispatch_autorelease_frequency_t frequency);
```
&emsp;可参考 `dispatch_queue_attr_make_with_autorelease_frequency`，workloop 的默认策略是 `DISPATCH_AUTORELEASE_FREQUENCY_WORK_ITEM`。

&emsp;`workloop`：dispatch workloop 进行修改。该 workloop 必须是非活动的，传递激活的对象是不确定的，并且将导致进程终止。

&emsp;<dispatch/workloop.h> 文件到这里就全部看完了。下面接着看另一个文件 <dispatch/once.h>。
## <dispatch/once.h>
&emsp;
### dispatch_once_t
&emsp;看到 `dispatch_once_t` 仅是 long 的别名。
```c++
typedef __darwin_intptr_t       intptr_t;
typedef long                    __darwin_intptr_t;

DISPATCH_SWIFT3_UNAVAILABLE("Use lazily initialized globals instead")
typedef intptr_t dispatch_once_t;
```
&emsp;与 `dispatch_once` 一起使用的谓词，必须将其初始化为零。注意：静态和全局变量默认为零。
### dispatch_once
&emsp;一次只能执行一次块（block）。
```c++
#ifdef __BLOCKS__
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
DISPATCH_SWIFT3_UNAVAILABLE("Use lazily initialized globals instead")
void
dispatch_once(dispatch_once_t *predicate,
        DISPATCH_NOESCAPE dispatch_block_t block);
```
&emsp;`prddicate`：指向 `dispatch_once_t` 的指针，用于测试该 block 是否已完成。（这里我们常使用 `static onceToken;` 静态和全局变量默认为零。）

&emsp;`block`：该 block 全局仅执行一次。

&emsp;在使用或测试由该块初始化的任何变量之前，请始终调用 `dispatch_once`。

#### DISPATCH_ONCE_INLINE_FASTPATH
&emsp;在目前的 Mac iPhone 主流机器，（或者 apple 的主流平台下）下此值应该都是 1，那么将使用如下的内联 `_dispatch_once`。

#### DISPATCH_EXPECT/dispatch_compiler_barrier
&emsp;`__builtin_expect` 这个指令是 GCC 引入的，作用是允许程序员将最有可能执行的分支告诉编译器，这个指令的写法为：`__builtin_expect(EXP, N)`，意思是：`EXP == N` 的概率很大，然后 CPU 会预取该分支的指令，这样 CPU 流水线就会很大概率减少了 CPU 等待取指令的耗时，从而提高 CPU 的效率。

&emsp;`dispatch_compiler_barrier` 内存屏障。
```c++
#if __GNUC__
#define DISPATCH_EXPECT(x, v) __builtin_expect((x), (v)) 
#define dispatch_compiler_barrier()  __asm__ __volatile__("" ::: "memory")
#else
#define DISPATCH_EXPECT(x, v) (x)
#define dispatch_compiler_barrier()  do { } while (0)
#endif
```
#### DISPATCH_COMPILER_CAN_ASSUME
```c++
#if __has_builtin(__builtin_assume)
#define DISPATCH_COMPILER_CAN_ASSUME(expr) __builtin_assume(expr)
#else
#define DISPATCH_COMPILER_CAN_ASSUME(expr) ((void)(expr))
#endif
```
#### _dispatch_once
```c++
#if defined(__x86_64__) || defined(__i386__) || defined(__s390x__)
#define DISPATCH_ONCE_INLINE_FASTPATH 1 
#elif defined(__APPLE__)
#define DISPATCH_ONCE_INLINE_FASTPATH 1
#else
#define DISPATCH_ONCE_INLINE_FASTPATH 0
#endif

#if DISPATCH_ONCE_INLINE_FASTPATH
DISPATCH_INLINE DISPATCH_ALWAYS_INLINE DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
DISPATCH_SWIFT3_UNAVAILABLE("Use lazily initialized globals instead")
void
_dispatch_once(dispatch_once_t *predicate,
        DISPATCH_NOESCAPE dispatch_block_t block)
{
    // DISPATCH_EXPECT(*predicate, ~0l) 表示很大概率 *predicate 的值是 ~0l，并返回 *predicate 的值
    if (DISPATCH_EXPECT(*predicate, ~0l) != ~0l) {
        // 当 *predicate 等于 0 时，调用 dispatch_once 函数
        dispatch_once(predicate, block);
    } else {
        // 否则，执行这里仅是 
        dispatch_compiler_barrier();
    }
    DISPATCH_COMPILER_CAN_ASSUME(*predicate == ~0l);
}
#undef dispatch_once
#define dispatch_once _dispatch_once
#endif
#endif
```
&emsp;后面的 `dispatch_once_f` 和 `_dispatch_once_f` 仅是把 `dispatch_block_t` 更换为 `dispatch_function_t`，执行逻辑与 `dispatch_once` 完全相同。

&emsp;<dispatch/once.h> 文件到这里就全部看完了。下面接着看另一个文件 <dispatch/data.h>。
## <dispatch/data.h>
&emsp;调度数据对象（dispatch data objects）描述了可以由系统或应用程序管理的内存的连续或稀疏区域。调度数据对象（Dispatch data objects）是不可变的，任何直接访问由调度对象表示的内存区域都不得修改该内存。

### dispatch_data_t
&emsp;代表内存区域（memory regions）的调度对象（dispatch object）。
```c++
DISPATCH_DATA_DECL(dispatch_data);
```
+ 在 Swift（在 Swift 中使用 Objective-C）下宏定义展开是:
```c++
OS_EXPORT OS_OBJECT_OBJC_RUNTIME_VISIBLE
@interface OS_dispatch_data : NSObject
- (instancetype)init OS_SWIFT_UNAVAILABLE("Unavailable in Swift");
@end

typedef OS_dispatch_data * dispatch_data_t;
```
&emsp;`OS_dispatch_data` 是继承自 `NSObject` 的类，然后 `dispatch_data_t` 是指向 `OS_dispatch_data` 的指针。
+ 在 Objective-C 下宏定义展开是:
```c++
@protocol OS_dispatch_data <OS_dispatch_object>
@end

typedef NSObject<OS_dispatch_data> * dispatch_data_t;
```
&emsp;`OS_dispatch_data` 是继承自 `OS_dispatch_object` 协议的协议，并且为遵循该协议的 `NSObject` 实例对象类型的指针定义了一个 `dispatch_data_t` 的别名。
+ 在 C++ 下宏定义展开是:
```c++
typedef struct dispatch_data_s : public dispatch_object_s {} *dispatch_data_t;
```
&emsp;`dispatch_data_t` 是一个指向 `dispatch_data_s` 结构体的指针。
+ 在 C（Plain C）下宏定义展开是:
```c++
typedef struct dispatch_data_s *dispatch_data_t;
```
&emsp;`dispatch_data_t` 是指向 `struct dispatch_data_s` 的指针。
### dispatch_data_empty
&emsp;表示零长度（zero-length）存储区域（memory region）的单例分发数据对象（singleton dispatch data object）。
```c++
#define dispatch_data_empty \
        DISPATCH_GLOBAL_OBJECT(dispatch_data_t, _dispatch_data_empty)
API_AVAILABLE(macos(10.7), ios(5.0))
DISPATCH_EXPORT struct dispatch_data_s _dispatch_data_empty;
```
### DISPATCH_DATA_DESTRUCTOR_DEFAULT
&emsp;调度数据对象（dispatch data objects）的默认析构函数。在创建数据对象（data object）时使用，以指示应将提供的缓冲区复制到系统管理的内部存储器中。
```c++
#define DISPATCH_DATA_DESTRUCTOR_DEFAULT NULL
```
### DISPATCH_DATA_DESTRUCTOR_TYPE_DECL
&emsp;根据是否是 `__BLOCKS__` 环境来转换 `_dispatch_data_destructor_##name` 为 `dispatch_block_t` 或者 `dispatch_function_t`。
```c++
#ifdef __BLOCKS__
/*! @parseOnly */
#define DISPATCH_DATA_DESTRUCTOR_TYPE_DECL(name) \
    DISPATCH_EXPORT const dispatch_block_t _dispatch_data_destructor_##name
#else
#define DISPATCH_DATA_DESTRUCTOR_TYPE_DECL(name) \
    DISPATCH_EXPORT const dispatch_function_t \
    _dispatch_data_destructor_##name
#endif /* __BLOCKS__ */
```
### DISPATCH_DATA_DESTRUCTOR_FREE
&emsp;从 malloc 的缓冲区创建的调度数据对象（dispatch data objects）的析构函数。在创建数据对象（data object）时使用，以指示所提供的缓冲区是由 `malloc` 系列函数分配的，应使用 `free` 销毁。
```c++
#define DISPATCH_DATA_DESTRUCTOR_FREE (_dispatch_data_destructor_free)
API_AVAILABLE(macos(10.7), ios(5.0))
DISPATCH_DATA_DESTRUCTOR_TYPE_DECL(free);
```
### DISPATCH_DATA_DESTRUCTOR_MUNMAP
&emsp;从需要使用 `munmap` 释放的缓冲区，创建的调度数据对象（dispatch data objects）的析构函数。
```c++
#define DISPATCH_DATA_DESTRUCTOR_MUNMAP (_dispatch_data_destructor_munmap)
API_AVAILABLE(macos(10.9), ios(7.0))
DISPATCH_DATA_DESTRUCTOR_TYPE_DECL(munmap);
```
### dispatch_data_create
&emsp;从给定的连续内存缓冲区（buffer）中创建一个调度数据对象（dispatch data object）。如果提供了非默认的析构函数（non-default destructor），则缓冲区所有权归调用者所有（即不会复制字节）。数据对象（data object）的最新（last release）版本将导致在指定队列上调用指定的析构函数以释放缓冲区。
```c++
API_AVAILABLE(macos(10.7), ios(5.0))
DISPATCH_EXPORT DISPATCH_RETURNS_RETAINED DISPATCH_WARN_RESULT DISPATCH_NOTHROW
dispatch_data_t
dispatch_data_create(const void *buffer,
    size_t size,
    dispatch_queue_t _Nullable queue,
    dispatch_block_t _Nullable destructor);
#endif /* __BLOCKS__ */
```
&emsp;如果提供了 `DISPATCH_DATA_DESTRUCTOR_FREE` 析构函数，则将通过 `free` 释放缓冲区，并且忽略队列参数。

&emsp;如果提供了 `DISPATCH_DATA_DESTRUCTOR_DEFAULT` 析构函数，则数据对象的创建会将缓冲区复制到系统管理的内部存储器中。

&emsp;`buffer`：连续的数据缓冲区。

&emsp;`size`：连续数据缓冲区的大小。

&emsp;`queue`：析构函数应提交的队列。

&emsp;`destructor`：析构函数负责在不再需要时释放数据。

&emsp;`result`：新创建的调度数据对象。
### dispatch_data_get_size
&emsp;返回由指定调度数据对象（dispatch data object）表示的内存区域的逻辑大小。
```c++
API_AVAILABLE(macos(10.7), ios(5.0))
DISPATCH_EXPORT DISPATCH_PURE DISPATCH_NONNULL1 DISPATCH_NOTHROW
size_t
dispatch_data_get_size(dispatch_data_t data);
```
&emsp;`data`：要查询的调度数据对象（dispatch data object）。

&emsp;`result`：数据对象（data object）表示的字节数。
### dispatch_data_create_map
```c++
API_AVAILABLE(macos(10.7), ios(5.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_RETURNS_RETAINED
DISPATCH_WARN_RESULT DISPATCH_NOTHROW
dispatch_data_t
dispatch_data_create_map(dispatch_data_t data,
    const void *_Nullable *_Nullable buffer_ptr,
    size_t *_Nullable size_ptr);
```
&emsp;将指定的调度数据对象（dispatch data object）表示的内存映射为单个连续的内存区域，并返回表示该内存区域的新数据对象。如果提供了对指针和大小变量的非 `NULL` 引用，则将使用该区域的位置和范围填充它们。这些允许对表示的内存进行直接读取访问，但是仅在释放返回的对象之前才有效。在 ARC 下，如果对象被保存在一个自动存储的变量（局部变量）中，则需要注意确保在通过指针访问内存之前编译器不会释放它。

&emsp;`data`：要映射的调度数据对象（dispatch data object）。

&emsp;`buffer_ptr`：指向指针变量的指针，该指针变量将使用映射的连续内存区域的位置或 `NULL` 填充。

&emsp;`size_ptr`：指向要用映射的连续内存区域的大小或 `NULL` 填充的 size_t 变量的指针。

&emsp;`result`：新创建的调度数据对象（dispatch data object）。
### dispatch_data_create_concat
&emsp;返回一个表示**指定数据对象（data objects）的串联**的新调度数据对象（dispatch data objec）。调用返回后，这些对象可以由应用程序释放（但是，在新创建的对象也被释放之前，系统可能不会释放由它们描述的内存区域）。
```c++
API_AVAILABLE(macos(10.7), ios(5.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_RETURNS_RETAINED
DISPATCH_WARN_RESULT DISPATCH_NOTHROW
dispatch_data_t
dispatch_data_create_concat(dispatch_data_t data1, dispatch_data_t data2);
```
&emsp;`data1`：表示要放置在新创建对象内存区域开头的数据对象。

&emsp;`data2`：表示要放置在新创建对象内存区域末尾的数据对象。

&emsp;`result`：一个新创建的对象，表示 `data1` 和 `data2` 对象的串联。
### dispatch_data_create_subrange
&emsp;返回表示指定数据对象子范围的新调度数据对象（dispatch data object），该对象可能在调用返回后由应用程序释放（但是，在新创建的对象也已释放之前，系统可能不会释放该对象描述的内存区域）。
```c++
API_AVAILABLE(macos(10.7), ios(5.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_RETURNS_RETAINED
DISPATCH_WARN_RESULT DISPATCH_NOTHROW
dispatch_data_t
dispatch_data_create_subrange(dispatch_data_t data,
    size_t offset,
    size_t length);
```
&emsp;`data`：表示要创建其内存区域子范围的数据对象。

&emsp;`offset`：数据对象的偏移量表示子范围开始处。

&emsp;`length`：范围的长度。

&emsp;`result`：一个新创建的对象，代表数据对象的指定子范围。
### dispatch_data_applier_t
&emsp;表示为一个数据对象（data object）中的每个连续内存区域调用的块。
```c++
typedef bool (^dispatch_data_applier_t)(dispatch_data_t region,
    size_t offset,
    const void *buffer,
    size_t size);
```
&emsp;`region`：表示当前区域的数据对象。

&emsp;`offset`：当前区域到数据对象开始的逻辑偏移量。

&emsp;`buffer`：当前区域的内存位置。

&emsp;`size`：当前区域的内存大小。

&emsp;`result`：一个布尔值，指示是否应继续遍历。
### dispatch_data_apply
&emsp;以逻辑顺序遍历指定的调度数据对象（dispatch data object）表示的内存区域，并为遇到的每个连续内存区域调用一次指定的块（block）。
```c++
API_AVAILABLE(macos(10.7), ios(5.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
bool
dispatch_data_apply(dispatch_data_t data,
    DISPATCH_NOESCAPE dispatch_data_applier_t applier);
#endif /* __BLOCKS__ */
```
&emsp;块的每次调用都会传递一个数据对象，该对象表示当前区域及其逻辑偏移量，以及该区域的存储位置和范围。它们允许对内存区域的直接读取访问，但是仅在释放传入的区域对象之前才有效。请注意，区域对象在块返回时由系统释放，如果块返回后需要区域对象或关联的内存位置，则应用程序有责任保留该对象。

&emsp;`data`：要遍历的数据对象。

&emsp;`applier`：数据对象中每个连续存储区域要调用的块。

&emsp;`result`：一个布尔值，指示遍历是否成功完成。
### dispatch_data_copy_region
&emsp;在由指定对象（`data`）表示的区域中查找包含指定位置（location）的连续内存区域，并返回表示该区域的内部调度数据对象（dispatch data object）的副本（函数返回值）及其在指定对象中的逻辑偏移量（用一个 size_t * 记录）。
```c++
API_AVAILABLE(macos(10.7), ios(5.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NONNULL3 DISPATCH_RETURNS_RETAINED
DISPATCH_WARN_RESULT DISPATCH_NOTHROW
dispatch_data_t
dispatch_data_copy_region(dispatch_data_t data,
    size_t location,
    size_t *offset_ptr);
```
&emsp;`data`：要查询的调度数据对象。

&emsp;`location`：要查询的数据对象中的逻辑位置。

&emsp;`offset_ptr`：指向 size_t 变量的指针，指针包含的值是返回的区域对象到查询的数据对象的起始位置的逻辑偏移量。

&emsp;`result`：新创建的调度数据对象（dispatch data object）。

&emsp;<dispatch/data.h> 文件到这里就全部看完了。下面接着看另一个文件 <dispatch/block.h>，下面一篇就是 GCD API 的最后一篇了，然后就开始扣源码了，其实看到这里我自己也不知道在看什么，看懂的大概就是函数命名、函数参数或者说是仅看懂了函数声明，很多 API 的注释都是不知道在说什么，因为从来都没有用过，貌似只有第一篇中我们比较常用 API 能看懂，可能是强迫症吧必须把 API 都看一遍，或者是还不知道怎么开始扣 GCD 的源码，这里想到了之前刚开始扣 objc4-781 源码的时候，也是不知道怎么看怎么规划，然后就找博客找文章几乎把相关的都看了一遍，后来就未知未觉的进入 objc4 的源码世界。总之，加油！加油！一定要搞清楚线程、队列、任务、以及它们之间的协作和调度关系。⛽️⛽️

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
+ [__builtin_expect 说明](https://www.jianshu.com/p/2684613a300f)
+ [内存屏障(__asm__ __volatile__("": : :"memory"))](https://blog.csdn.net/whycold/article/details/24549571)
