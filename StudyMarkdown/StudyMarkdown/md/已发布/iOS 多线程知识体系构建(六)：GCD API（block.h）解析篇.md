# iOS 多线程知识体系构建(六)：GCD API（block.h）解析篇

> &emsp;那么继续学习 dispath 中也挺重要的 <dispatch/block.h> 文件。

## <dispatch/block.h>
&emsp;Dispatch block objects.
### dispatch_block_flags_t
&emsp;`DISPATCH_OPTIONS` 宏定义内容，即不同平台或者环境下的枚举定义：
```c++
#if __has_feature(objc_fixed_enum) || __has_extension(cxx_strong_enums) || \
        __has_extension(cxx_fixed_enum) || defined(_WIN32)

#define DISPATCH_OPTIONS(name, type, ...) \
        typedef enum : type { __VA_ARGS__ } __DISPATCH_OPTIONS_ATTR __DISPATCH_ENUM_ATTR name##_t
        
#else

#define DISPATCH_OPTIONS(name, type, ...) \
        enum { __VA_ARGS__ } __DISPATCH_OPTIONS_ATTR __DISPATCH_ENUM_ATTR; typedef type name##_t
        
#endif // __has_feature(objc_fixed_enum) ...
```
&emsp;`dispatch_block_flags_t`:

&emsp;传递给 `dispatch_block_create*` 函数的标志。
+ `DISPATCH_BLOCK_BARRIER`：指示调度块对象（dispatch block object）在提交给 `DISPATCH_QUEUE_CONCURRENT` 队列时应充当屏障块（barrier block）的标志。有关详细信息，参考 `dispatch_barrier_async`。当直接调用调度块对象（dispatch block object）时，此标志无效。
+ `DISPATCH_BLOCK_DETACHED`：指示应该执行与当前执行上下文属性（例如 os_activity_t 和当前 IPC 请求的属性，如果有）无关的调度块对象（dispatch block object）的标志。关于 QoS 类别，其行为与 DISPATCH_BLOCK_NO_QOS 相同。如果直接调用，则块对象将在块主体的持续时间内从调用线程中删除其他属性（在应用分配给块对象的属性（如果有）之前）。如果提交给队列，则将使用队列的属性（或专门分配给该块对象的任何属性）执行该块对象。
+ `DISPATCH_BLOCK_ASSIGN_CURRENT`：指示应为调度块对象分配创建块对象时当前的执行上下文属性的标志。这适用于诸如 QOS 类，os_activity_t 的属性以及当前 IPC 请求的属性（如果有）。如果直接调用，则块对象将在块主体的持续时间内将这些属性应用于调用线程。如果将块对象提交到队列，则此标志替换将提交的块实例与提交时最新的执行上下文属性相关联的默认行为。如果使用 `DISPATCH_BLOCK_NO_QOS_CLASS` 或 `dispatch_block_create_with_qos_class` 分配了特定的 QOS 类，则该 QOS 类优先于此标志指示的 QOS 类分配。
+ `DISPATCH_BLOCK_NO_QOS_CLASS`：指示不应为调度块对象分配 QOS 类的标志。如果直接调用，则块对象将与调用线程的 QOS 类一起执行。如果将块对象提交到队列，这将替换默认行为，即在提交时将提交的块实例与当前的 QOS 类相关联。如果为特定的 QOS 类分配了 `dispatch_block_create_with_qos_class`，则忽略此标志。
+ `DISPATCH_BLOCK_INHERIT_QOS_CLASS`：指示执行提交到队列的调度块对象的标志应优先于分配给队列的 QOS 类，而不是分配给该块的 QOS 类（提交时与该块相关联的 resp。）。仅当所讨论的队列没有分配的 QOS 类时，才使用后者，只要这样做不会导致 QOS 类低于从队列的目标队列继承的 QOS 类。当将调度块对象提交到队列以进行异步执行时，此标志是默认设置；当直接调用调度块对象时，此标志无效。如果还传递了 `DISPATCH_BLOCK_ENFORCE_QOS_CLASS`，则将其忽略。
+ `DISPATCH_BLOCK_ENFORCE_QOS_CLASS`：指示执行提交到队列的调度块对象的标志应优先于分配给队列的 QOS 类，而不是分配给队列的 QOS 类，分配给该块的 QOS 类（在提交时与该块相关联）不会导致较低的 QOS 等级。当将调度块对象提交到队列以进行同步执行或直接调用调度块对象时，此标志是默认设置。
```c++
DISPATCH_OPTIONS(dispatch_block_flags, unsigned long,
    DISPATCH_BLOCK_BARRIER
            DISPATCH_ENUM_API_AVAILABLE(macos(10.10), ios(8.0)) = 0x1, // 二进制表示每次进一位，即能多个枚举值并存的枚举类型
    DISPATCH_BLOCK_DETACHED
            DISPATCH_ENUM_API_AVAILABLE(macos(10.10), ios(8.0)) = 0x2,
    DISPATCH_BLOCK_ASSIGN_CURRENT
            DISPATCH_ENUM_API_AVAILABLE(macos(10.10), ios(8.0)) = 0x4,
    DISPATCH_BLOCK_NO_QOS_CLASS
            DISPATCH_ENUM_API_AVAILABLE(macos(10.10), ios(8.0)) = 0x8,
    DISPATCH_BLOCK_INHERIT_QOS_CLASS
            DISPATCH_ENUM_API_AVAILABLE(macos(10.10), ios(8.0)) = 0x10,
    DISPATCH_BLOCK_ENFORCE_QOS_CLASS
            DISPATCH_ENUM_API_AVAILABLE(macos(10.10), ios(8.0)) = 0x20,
);
```
### dispatch_block_create
&emsp;根据现有块（existing block）和给定的标志（flags）在堆（heap）上创建一个新的调度块对象（dispatch block object）。
```c++
API_AVAILABLE(macos(10.10), ios(8.0))
DISPATCH_EXPORT DISPATCH_NONNULL2 DISPATCH_RETURNS_RETAINED_BLOCK
DISPATCH_WARN_RESULT DISPATCH_NOTHROW
dispatch_block_t
dispatch_block_create(dispatch_block_flags_t flags, dispatch_block_t block);
```
&emsp;提供的块被 `Block_copy` 到堆中，并由新创建的调度块对象保留。

&emsp;返回的调度块对象（dispatch block object）旨在通过 `dispatch_async` 和相关函数提交给调度队列，但也可以直接调用。两种操作都可以执行任意次，但只有第一次完成的调度块对象（dispatch block object）的执行才能用 `dispatch_block_wait` 等待，或用 `dispatch_block_notify` 来观察。

&emsp;如果将返回的调度块对象（dispatch block object）提交给调度队列（dispatch queue），则提交的块实例（block instance）将与提交时当前的 QOS 类相关联，除非以下标志之一在分配时分配了特定的 QOS 类（或没有 QOS 类）。块创建时间：
+ `DISPATCH_BLOCK_ASSIGN_CURRENT`
+ `DISPATCH_BLOCK_NO_QOS_CLASS`
+ `DISPATCH_BLOCK_DETACHED`
 
&emsp;块对象将与之一起执行的 QOS 类还取决于分配给队列的 QOS 类，以及以下哪个标志被指定或默认设置：
+ `DISPATCH_BLOCK_INHERIT_QOS_CLASS (default for asynchronous execution)`
+ `DISPATCH_BLOCK_ENFORCE_QOS_CLASS (default for synchronous execution)`
&emsp;(See description of dispatch_block_flags_t for details.)
 
&emsp;如果返回的调度块对象（dispatch block object）被直接提交到串行队列，并且被配置为使用特定的 QOS 类执行，那么系统将尽最大努力应用必要的 QOS 覆盖，以确保先前提交到串行队列的块在相同的 QOS 类或更高的 QOS 类中执行。
 
&emsp;`flags`：块对象（block object）的配置标志。传递的值与 `dispatch_block_flags_t` 中的标志不是按位或的结果将返回 `NULL`。
 
&emsp;`block`：从中创建调度块对象（dispatch block object）的块。
 
&emsp;`result`：新创建的调度块对象，或者 `NULL`。当不使用 Objective-C ARC 构建时，必须使用 `-[release]` 消息或 `Block_release` 函数来进行释放。
### dispatch_block_create_with_qos_class
&emsp;根据现有块（existing block）和给定的标志（flags）在堆（heap）上创建一个新的调度块对象（dispatch block object），并为其分配指定的 QOS 类（`qos_class`）和相对优先级（`relative_priority`）。
```c++
API_AVAILABLE(macos(10.10), ios(8.0))
DISPATCH_EXPORT DISPATCH_NONNULL4 DISPATCH_RETURNS_RETAINED_BLOCK
DISPATCH_WARN_RESULT DISPATCH_NOTHROW
dispatch_block_t
dispatch_block_create_with_qos_class(dispatch_block_flags_t flags,
        dispatch_qos_class_t qos_class, int relative_priority,
        dispatch_block_t block);
```
&emsp;提供的块被 `Block_copy` 到堆中，并由新创建的调度块对象（dispatch block object）保留。

&emsp;返回的调度块对象（dispatch block object）旨在通过 `dispatch_async` 和相关函数提交给调度队列，但也可以直接调用。两种操作都可以执行任意次，但只有第一次完成的调度块对象（dispatch block object）的执行才能用 `dispatch_block_wait` 等待，或用 `dispatch_block_notify` 来观察。

&emsp;如果直接调用，则返回的调度块对象（dispatch block object）将与分配的 QOS 类一起执行，只要它不会导致其 QOS 类比调用线程上当前的类低。

&emsp;如果将返回的调度块对象提交到调度队列，则将执行的 QOS 类取决于分配给该块的 QOS 类，分配给该队列的 QOS 类以及以下哪个标志被指定或默认设置。

&emsp;如果将返回的调度块对象（dispatch block object）提交到调度队列（dispatch queue），则执行该对象所用的 QOS 类取决于分配给块的QOS类、分配给队列的 QOS 类以及指定或默认为以下哪个标志：
+ `DISPATCH_BLOCK_INHERIT_QOS_CLASS: default for asynchronous execution`
+ `DISPATCH_BLOCK_ENFORCE_QOS_CLASS: default for synchronous execution`
&emsp;(See description of dispatch_block_flags_t for details.)

&emsp;如果返回的调度块对象（dispatch block object）被直接提交到串行队列，并且被配置为使用特定的 QOS 类执行，那么系统将尽最大努力应用必要的 QOS 覆盖，以确保先前提交到串行队列的块在相同的 QOS 类或更高的 QOS 类中执行。

&emsp;`flags`：块对象（block object）的配置标志。传递的值与 `dispatch_block_flags_t` 中的标志不是按位或的结果将返回 `NULL`。

&emsp;`qos_class`：QOS 类的值。

+ `QOS_CLASS_USER_INTERACTIVE`
+ `QOS_CLASS_USER_INITIATED`
+ `QOS_CLASS_DEFAULT`
+ `QOS_CLASS_UTILITY`
+ `QOS_CLASS_BACKGROUND`
+ `QOS_CLASS_UNSPECIFIED`

&emsp;传递 `QOS_CLASS_UNSPECIFIED` 等效于指定 `DISPATCH_BLOCK_NO_QOS_CLASS` 标志。传递任何其他值都会导致返回 `NULL`。

&emsp;`relative_priority`：QOS 类中的相对优先级。该值是给定类别与最大支持的调度程序优先级的负偏移量。传递大于零或小于 `QOS_MIN_RELATIVE_PRIORITY` 的值将导致返回 `NULL`。

&emsp;`block`：从中创建调度块对象的块。

&emsp;`result`：新创建的调度块对象，或者 `NULL`。当不使用 Objective-C ARC 构建时，必须使用 `-[release]` 消息或 `Block_release` 函数来进行释放。
### dispatch_block_perform
&emsp;从指定的块（`block`）和标志（`flags`）创建，同步执行（synchronously execute）和释放（release）调度块对象（dispatch block object）。
```c++
API_AVAILABLE(macos(10.10), ios(8.0))
DISPATCH_EXPORT DISPATCH_NONNULL2 DISPATCH_NOTHROW
void
dispatch_block_perform(dispatch_block_flags_t flags,
        DISPATCH_NOESCAPE dispatch_block_t block);
```
&emsp;行为与序列相同，如下:
```c++
dispatch_block_t b = dispatch_block_create(flags, block);
b();
Block_release(b);
```
&emsp;但可以通过内部方式更有效地实现，而无需复制指定块（block）到堆中或分配创建新的块（block）对象。

&emsp;`flags`：临时块对象的配置标志。传递与 `dispatch_block_flags_t` 中的标志不是按位或的值，结果将是未定义的。

&emsp;`block`：从中创建临时块对象的块。
### dispatch_block_wait
&emsp;同步等待（阻塞），直到指定的分发块对象的执行完成或指定的超时时间结束为止。
```c++
API_AVAILABLE(macos(10.10), ios(8.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NOTHROW
long
dispatch_block_wait(dispatch_block_t block, dispatch_time_t timeout);
```
&emsp;如果块对象的执行已经完成，该函数将立即返回。

&emsp;不能使用此接口等待同一块对象的多次执行；为此，请使用 `dispatch_group_wait`。单个调度块对象（dispatch block object）可以等待一次并执行一次，也可以执行任意次数。任何其他组合的行为都是未定义的。即使取消（`dispatch_block_cancel`）表示该块的代码从不运行，向调度队列的提交也被视为执行。

&emsp;从多个线程同时使用同一个调度块对象同时调用此函数的结果是不确定的，但请注意，这样做将违反上一段中描述的规则。

&emsp;如果此函数的返回值指示已超过指定的超时，则该调用不算作允许的一次等待。

&emsp;如果在调用此函数时，指定的调度块对象已直接提交到串行队列，则系统将尽最大努力应用必要的 QOS 覆盖，以确保该块和任何较早提交给该串行队列的块在调用 `dispatch_block_wait` 的线程的 QOS 类（或更高版本）上执行。

&emsp;`block`：要等待的调度块对象。传递 `NULL` 或未由 `dispatch_block_create*` 函数之一返回的块对象的结果是未定义的。

&emsp;`timeout`：何时超时（dispatch_time）。为方便起见，有 `DISPATCH_TIME_NOW` 和 `DISPATCH_TIME_FOREVER` 常量。

&emsp;`result`：成功返回零（在指定的超时内完成调度块对象），错误返回非零（即超时）。
### dispatch_block_notify
&emsp;计划在完成指定调度块对象（block）的执行后将通知块（notification_block）提交给队列。
```c++
API_AVAILABLE(macos(10.10), ios(8.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_block_notify(dispatch_block_t block, dispatch_queue_t queue,
        dispatch_block_t notification_block);
```
&emsp;如果观察到的块对象（block）的执行已经完成，则此函数将立即提交通知块（notification_block）。

&emsp;使用此接口无法通知同一块对象的多次执行，请为此目的使用 `dispatch_group_notify`。

&emsp;单个分发块对象（single dispatch block object）可以被观察一次或多次并执行一次，也可以执行任意次。任何其他组合的行为均未定义。即使取消（`dispatch_block_cancel`）表示该块的代码从不运行，向调度队列的提交也被视为执行。

&emsp;如果为单个块对象计划了多个通知块，则没有定义将通知块提交到其关联队列的顺序。

&emsp;`block`：要观察的调度块对象。传递 `NULL` 或未由 `dispatch_block_create*` 函数之一返回的块对象的结果是不确定的。

&emsp;`queue`：当观察到的块完成时，将向其提交所提供的通知块的队列。

&emsp;`notification_block`：观察的块对象（`block`）完成时要提交的通知块。
### dispatch_block_cancel
&emsp;异步取消指定的调度块对象（dispatch block object）。
```c++
API_AVAILABLE(macos(10.10), ios(8.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_block_cancel(dispatch_block_t block);
```
&emsp;取消会使调度块对象（dispatch block object）的任何将来执行立即返回，但不影响已经在进行中的块对象的任何执行。

&emsp;与该块对象相关联的任何资源的释放将被延迟，直到下一次尝试执行该块对象（或已完成的任何执行完成）为止。

&emsp;注意：需要注意确保可以取消的块对象不会捕获需要执行块体才能释放的任何资源（例如，块体调用 `free`（用 `malloc` 分配的内存））。如果由于取消而从不执行块体，则这些资源将被泄漏。

&emsp;`block`：要取消的调度块对象。传递 `NULL` 或未由 `dispatch_block_create*` 函数之一返回的块对象的结果是不确定的。
### dispatch_block_testcancel
&emsp;测试给定的调度块对象（dispatch block object）是否已取消。
```c++
API_AVAILABLE(macos(10.10), ios(8.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_WARN_RESULT DISPATCH_PURE
DISPATCH_NOTHROW
long
dispatch_block_testcancel(dispatch_block_t block);
```
&emsp;`block`：要测试的调度块对象。传递 `NULL` 或未由 `dispatch_block_create*` 函数之一返回的块对象的结果是未定义的。

&emsp;`result`：如果取消，则返回非零；如果未取消，返回零。

&emsp;<dispatch/block.h> 文件到这里就全部看完了。

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




&emsp;<dispatch/block.h> 文件到这里就全部看完了。下面接着看另一个文件 <dispatch/io.h>，
## <dispatch/io.h>
&emsp;Dispatch I/O 对文件描述符（file descriptors）提供流和随机访问异步读取和写入操作。可以从文件描述符（file descriptor）中将一个或多个 dispatch I/O channels 创建为 `DISPATCH_IO_STREAM` 类型或 `DISPATCH_IO_RANDOM` 类型。创建通道后，应用程序可以安排异步读取和写入操作。

&emsp;应用程序可以在 dispatch I/O channel 上设置策略，以指示长时间运行的操作所需的 I/O 处理程序频率。

&emsp;Dispatch I/O 还为 I/O 缓冲区提供了内存管理模型，可避免在通道之间进行管道传输时不必要的数据复制。Dispatch I/O 监视应用程序的整体内存压力和 I/O 访问模式，以优化资源利用率。


1. 在 Block_descriptor_3 结构体中有一个 signature 成员变量表示 block 的签名信息，里面主要包含 block 的参数和返回值等信息。
2. _Block_copy 函数里面有清晰的描述，malloc 函数在堆区申请空间，然后调用 memmove 函数进行字节复制把栈区的 block 复制到堆区。
3. 经过 __block 修饰的变量已经变为了结构体，此时再修改变量修改的是结构体中的同名成员变量。
4. 在 ARC 下用 strong/copy 修饰 block 属性时，该属性的 setter 函数内部看到调用的都是 _objc_setProperty_nonatomic_copy 函数即 block 都会发生复制操作，用 weak 的话则是 _objc_storeWeak 函数。所以 ARC 下用 strong 和 copy 修饰 block 是一样的，但是不能用 weak。
5. 在 block 释放时会调用一个 _Block_object_dispose 函数根据 block 捕获的不同的变量做不同的释放操作。
