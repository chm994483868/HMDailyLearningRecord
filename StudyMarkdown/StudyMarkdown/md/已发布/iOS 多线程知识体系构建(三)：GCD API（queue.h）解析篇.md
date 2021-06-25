# iOS 多线程知识体系构建(三)：GCD API（queue.h）解析篇

> &emsp;Grand Central Dispatch (GCD) 是 Apple 开发的一个多核编程的较新的解决方法。

> &emsp;Execute code concurrently on multicore hardware by submitting work to dispatch queues managed by the system.
> &emsp;通过提交工作到 dispatch 系统管理的队列（dispatch queues），在多核硬件上同时执行代码。主要用于优化应用程序以支持多核处理器以及其他对称多处理系统。可以理解为 Dispatch 队列封装了底层多核系统调度的操作，我们只需要关心对 Dispatch 队列的操作，不需要关心任务到底分配给哪个核心，甚至不需要关心任务在哪个线程执行（当然为了深入学习主线程和子线程是一定要研究的）。

## Grand Central Dispatch（GCD）
### GCD 概述
&emsp;Dispatch，也称为 Grand Central Dispatch（GCD），包含语言功能、运行时库和系统增强功能，这些功能为支持 macOS、iOS、watchOS 和 tvOS 中的多核硬件上的并发代码执行提供了系统的、全面的改进。

&emsp;对 BSD 子系统、Core Foundation 和 cococoa api 都进行了扩展，以使用这些增强功能来帮助系统和应用程序更快、更高效地运行，并提高响应能力。考虑一下单个应用程序有效地使用多个核心有多困难，更不用说在具有不同计算核心数量的不同计算机上或在多个应用程序竞争这些核心的环境中这样做有多困难。GCD 在系统级运行，可以更好地满足所有运行的应用程序的需求，以平衡的方式将它们与可用的系统资源相匹配。
### Dispatch Objects and ARC
> &emsp;GCD 的源码是开源的，我们首先学习完 GCD 的 API 使用以后，再直接深入它的源码一探究竟，这里先有个大致的认知概念就行。

&emsp;何谓 Dispatch Objects 呢？Dispatch Objects（调度对象）有多种类型，包括 `dispatch_queue_t`、`dispatch_group_t` 和 `dispatch_source_t`。基本 Dispatch Object 接口允许管理内存、暂停和恢复执行、定义对象上下文、记录任务数据等。

&emsp;默认情况下，使用 Objective-C 编译器构建 dispatch objects 时，它们被声明为 Objective-C 类型。此行为允许你采用 ARC 并启用静态分析器的内存泄漏检查。它还允许你将对象添加到 Cocoa 集合（NSMutableArray、NSMutableDictionary...）中。

&emsp;使用 Objective-C 编译器构建应用程序时，所有 dispatch objects 都是 Objective-C 对象。因此，启用自动引用计数（ARC）时，调度对象将自动保留和释放，就像任何其它 Objective-C 对象一样。如果未启用 ARC（在 MRC 下），需要使用 `dispatch_retain` 和 `dispatch_release` 函数（或 Objective-C 语义）来保留和释放 dispatch objects。不能使用 Core Foundation 中的 `retain` 和 `release`。

&emsp;如果你需要在启用了 ARC 的应用程序中使用 retain 和 release 语义（为了保持与现有代码的兼容性），可以通过在编译器标志中添加 DOS_OBJECT_USE_OBJC=0 来禁用基于 Objective-C 的 dispatch objects。

&emsp;如果需要在具有更高部署目标的已启用 ARC 的应用程序中使用 retain 和 release 语义（以保持与现有代码的兼容性），可以通过在编译器标志中添加 DOS_OBJECT_USE_OBJC = 0 来禁用基于 Objective-C 的 dispatch objects。

### GCD 中的类型
&emsp;为了深入理解 GCD，首先从我们日常使用 GCD API 中常见的类型入手，搞懂这些类型的具体定义有助于我们理解 GCD 的使用方式以及内部的实现逻辑。首先按住 command 点击 `dispatch_queue_t` 看到 queue.h 文件中的 `dispatch_queue_t` 是一个宏定义：`DISPATCH_DECL(dispatch_queue);`，看到小括号内没有 `_t`，那这个 `_t` 的小尾巴是从哪里来的呢？下面我们沿着 `DISPATCH_DECL` 宏的具体内容来看一下，涉及到的一系列宏都定义在 `usr/include/os/object.h` 文件中。

&emsp;**这里为了便于理解我们暂时只看 Objective-C 语言下的 GCD**。（下篇会把 Swift/C++/C 环境下的转换也都补上）

+ `DISPATCH_DECL` 宏定义：
```c++
#define DISPATCH_DECL(name) OS_OBJECT_DECL_SUBCLASS(name, dispatch_object)

DISPATCH_DECL(dispatch_queue) ➡️ OS_OBJECT_DECL_SUBCLASS(dispatch_queue, dispatch_object)
```
+ `OS_OBJECT_DECL_SUBCLASS` 宏定义：
```c++
#define OS_OBJECT_DECL_SUBCLASS(name, super) \
        OS_OBJECT_DECL_IMPL(name, <OS_OBJECT_CLASS(super)>)

OS_OBJECT_DECL_SUBCLASS(dispatch_queue, dispatch_object) ➡️ OS_OBJECT_DECL_IMPL(dispatch_queue, <OS_OBJECT_CLASS(dispatch_object)>)
```
+ `OS_OBJECT_CLASS` 宏定义：( `##` 运算符可以用于宏函数的替换部分。这个运算符把两个语言符号组合成单个语言符号，为宏扩展提供了一种连接实际变元的手段。)
```c++
#define OS_OBJECT_CLASS(name) OS_##name

<OS_OBJECT_CLASS(dispatch_object)> ➡️ <OS_dispatch_object>
OS_OBJECT_DECL_IMPL(dispatch_queue, <OS_OBJECT_CLASS(dispatch_object)>) ➡️ OS_OBJECT_DECL_IMPL(dispatch_queue, <OS_dispatch_object>)
```
+ `OS_OBJECT_DECL_IMPL` 宏定义：
```c++
#define OS_OBJECT_DECL_IMPL(name, ...) \
        OS_OBJECT_DECL_PROTOCOL(name, __VA_ARGS__) \
        typedef NSObject<OS_OBJECT_CLASS(name)> \
                * OS_OBJC_INDEPENDENT_CLASS name##_t
        
OS_OBJECT_DECL_IMPL(dispatch_queue, <OS_dispatch_object>) ➡️ 

    OS_OBJECT_DECL_PROTOCOL(dispatch_queue, <OS_dispatch_object>) \
    typedef NSObject<OS_dispatch_queue> \
            * OS_OBJC_INDEPENDENT_CLASS dispatch_queue_t
```
+ `OS_OBJC_INDEPENDENT_CLASS` 宏定义：
```c++
#if __has_attribute(objc_independent_class)
#define OS_OBJC_INDEPENDENT_CLASS __attribute__((objc_independent_class))
#endif // __has_attribute(objc_independent_class)

#ifndef OS_OBJC_INDEPENDENT_CLASS
#define OS_OBJC_INDEPENDENT_CLASS
#endif

OS_OBJECT_DECL_PROTOCOL(dispatch_queue, <OS_dispatch_object>) \
typedef NSObject<OS_dispatch_queue> \
        * OS_OBJC_INDEPENDENT_CLASS dispatch_queue_t ➡️

OS_OBJECT_DECL_PROTOCOL(dispatch_queue, <OS_dispatch_object>) \
typedef NSObject<OS_dispatch_queue> \
        * dispatch_queue_t
```
+ `OS_OBJECT_DECL_PROTOCOL` 宏定义：
```c++
#define OS_OBJECT_DECL_PROTOCOL(name, ...) \
        @protocol OS_OBJECT_CLASS(name) __VA_ARGS__ \
        @end

OS_OBJECT_DECL_PROTOCOL(dispatch_queue, <OS_dispatch_object>) \
typedef NSObject<OS_dispatch_queue> \
        * dispatch_queue_t ➡️

@protocol OS_dispatch_queue <OS_dispatch_object> \
@end \
typedef NSObject<OS_dispatch_queue> \
        * dispatch_queue_t
```
&emsp;连续的宏定义整理到这里 `DISPATCH_DECL(dispatch_queue);` 即为:
```c++
@protocol OS_dispatch_queue <OS_dispatch_object>
@end

typedef NSObject<OS_dispatch_queue> * dispatch_queue_t;
```
&emsp;`OS_dispatch_queue` 是继承自 `OS_dispatch_object` 协议的协议，并且为遵循该协议的 `NSObject` 实例对象类型的指针定义了一个 `dispatch_queue_t` 的别名，看到这里我们恍然大悟，例如我们整天使用的 `dispatch_queue_t globalQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);` 获取一个全局并发对象，而这个 `globalQueue` 其实就是一个遵循 `OS_dispatch_queue` 协议的 `NSObject` 实例对象指针（OC 下：`dispatch_queue_t` 是 NSObject 指针）。

&emsp;那么这个 `OS_dispatch_object` 协议怎么来的呢？可看到是来自 `OS_OBJECT_DECL_CLASS(dispatch_object);` 宏，下面对它所代表的具体内容进行解读。
```c++
/*
* By default, dispatch objects are declared as Objective-C types when building
* with an Objective-C compiler. This allows them to participate in ARC, in RR
* management by the Blocks runtime and in leaks checking by the static
* analyzer, and enables them to be added to Cocoa collections.
* See <os/object.h> for details.
*/

/*
 * 默认情况下，使用 Objective-C 编译器进行构建时，dispatch objects 被声明为 Objective-C 类型（NSObject）。
 * 这使他们可以参与 ARC，通过 Blocks 运行时参与 RR（retain/release）管理以及通过静态分析器参与泄漏检查，
 * 并将它们添加到 Cocoa 集合（NSMutableArray、NSMutableDictionary...）中。有关详细信息，请参见。
 */
OS_OBJECT_DECL_CLASS(dispatch_object);

// 如下代码可验证 dispatch_queue_t 添加到 OC 集合中:
dispatch_queue_t globalQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
NSMutableArray *array = [NSMutableArray array];
[array addObject:globalQueue];
NSLog(@"array:  %@", array);

// 调用 isKindOfClass 判断类型
NSLog(@"🍑🍑 %d", [globalQueue isKindOfClass:[NSObject class]]);
// 控制台如下打印：

2020-11-16 18:27:24.218954+0800 Simple_iOS[69017:9954014] array:  ( ⬅️ dispatch_queue_t 被添加到 NSMutableArray 中
    "<OS_dispatch_queue_global: com.apple.root.default-qos>"
)
2020-11-16 18:27:24.219246+0800 Simple_iOS[69017:9954014] 🍑🍑 1 ⬅️ dispatch_queue_t 是一个指向 NSObject 类型的指针
```
+ `OS_OBJECT_DECL_CLASS` 宏定义：
```c++
#define OS_OBJECT_DECL_CLASS(name) \
        OS_OBJECT_DECL(name)

OS_OBJECT_DECL_CLASS(dispatch_object) ➡️ OS_OBJECT_DECL(dispatch_object)
```
+ `OS_OBJECT_DECL` 宏定义：
```c++
#define OS_OBJECT_DECL(name, ...) \
        OS_OBJECT_DECL_IMPL(name, <NSObject>)

OS_OBJECT_DECL(dispatch_object) ➡️ OS_OBJECT_DECL_IMPL(dispatch_object, <NSObject>)
```
+ `OS_OBJECT_DECL_IMPL` 宏定义：
```c++
#define OS_OBJECT_DECL_IMPL(name, ...) \
        OS_OBJECT_DECL_PROTOCOL(name, __VA_ARGS__) \
        typedef NSObject<OS_OBJECT_CLASS(name)> \
                * OS_OBJC_INDEPENDENT_CLASS name##_t
                
OS_OBJECT_DECL_IMPL(dispatch_object, <NSObject>) ➡️

OS_OBJECT_DECL_PROTOCOL(dispatch_object, <NSObject>) \
typedef NSObject<OS_dispatch_object> \
        * OS_OBJC_INDEPENDENT_CLASS dispatch_object_t ➡️
        
@protocol OS_dispatch_object <NSObject> \
@end \
typedef NSObject<OS_dispatch_object> \
        * dispatch_object_t  
```
&emsp;连续的宏定义整理到这里 `OS_OBJECT_DECL_CLASS(dispatch_object);` 即为:
```c++
@protocol OS_dispatch_object <NSObject> 
@end

typedef NSObject<OS_dispatch_object> * dispatch_object_t;  
```
&emsp;`OS_dispatch_object` 是继承自 `NSObject` 协议的协议，并且为遵循该协议的 `NSObject` 实例对象类型的指针定义了一个 `dispatch_object_t` 的别名。（OC 下：`dispatch_object_t` 是 NSObject 指针）

&emsp;综上可知，宏定义 `OS_OBJECT_DECL_CLASS(name)` 会定义一个继承自 `NSObject` 协议的协议，协议的名称为固定的 `name` 添加 `OS_` 前缀，并且定义一个表示遵循该协议的 `NSObject` 实例对象类型的指针的别名，名称为 `name` 添加后缀 `_t`。

&emsp;由 `#define DISPATCH_DECL_SUBCLASS(name, base) OS_OBJECT_DECL_SUBCLASS(name, base)` 可知，还可以在定义一个协议时，指定其所继承的协议（上面两个则是默认继承自 `NSObject` 协议和 `OS_dispatch_object` 协议），但是在使用时，要保证指定的 `base` 协议是已经定义过的。

&emsp;**以下基于 Objective-C 环境解读。**（下篇会把 Swift/C++/C 环境下的转换也都补上）

#### dispatch_queue_t
&emsp;Dispatch 是用于通过简单但功能强大的 API 来表达并发性的抽象模型。在核心上，dispatch 提供了可以向其提交 blocks 的串行 FIFO 队列。提交给这些 dispatch queues 的 blocks 在系统完全管理的线程池上调用，无法保证将在哪个线程上调用 block（系统会自行从线程池取可用的线程）；但是，它保证一次只调用一个提交到 FIFO dispatch queue 的 block。当多个队列有要处理的块时，系统可以自由地分配额外的线程来并发地调用这些 blocks。当队列变为空时，这些线程将自动释放。

```c++
DISPATCH_DECL(dispatch_queue);
```
&emsp;转换宏定义后是：
```c++
@protocol OS_dispatch_queue <OS_dispatch_object>
@end

typedef NSObject<OS_dispatch_queue> * dispatch_queue_t;
```
&emsp;Dispatch queues 调用提交给它们的工作项。

&emsp;Dispatch queues 有多种形式，最常见的一种是调度串行队列（`dispatch_queue_serial_t`）。系统管理一个线程池，该线程池处理 dispatch queues 并调用提交给它们的工作项。从概念上讲，一个 dispatch queue 可以具有自己的执行线程，并且队列之间的交互是高度异步的。调度队列通过调用 dispatch_retain() 和 dispatch_release() 进行引用计数。提交给队列的待处理工作项也会保留对该队列的引用，直到它们完成为止。一旦释放了对队列的所有引用，系统将重新分配该队列（queue 被释放销毁）。
#### dispatch_queue_global_t
```c++
DISPATCH_DECL_SUBCLASS(dispatch_queue_global, dispatch_queue);
```
&emsp;转换宏定义后是：
```c++
@protocol OS_dispatch_queue_global <OS_dispatch_queue>
@end

typedef NSObject<OS_dispatch_queue_global> * dispatch_queue_global_t;
```
&emsp;`OS_dispatch_queue_global` 是继承自 `OS_dispatch_queue` 协议的协议，并且为遵循该协议的 `NSObject` 实例对象类型的指针定义了一个 `dispatch_queue_global_t` 的别名。

&emsp;调度全局并发队列（dispatch global concurrent queues）是围绕系统线程池的抽象，它调用提交到调度队列的工作项。

&emsp;调度全局并发队列（dispatch global concurrent queues）在系统管理的线程池之上提供优先级桶（这个大概是哈希桶，后续看源码时再分析），系统将根据需求和系统负载决定分配给这个池的线程数。特别是，系统会尝试为该资源保持良好的并发级别，并且当系统调用中有太多的现有工作线程阻塞时，将创建新线程。（NSThread 和 GCD 的一个重大区别，GCD 下线程都是系统自动创建分配的，而 NSThread 则是自己手动创建线程或者自己手动开启线程。）

&emsp;全局并发队列（global concurrent queues）是共享资源，因此，此资源的每个用户都有责任不向该池提交无限数量的工作，尤其是可能阻塞的工作，因为这可能导致系统产生大量线程（又名：线程爆炸 thread explosion）。

&emsp;提交到全局并发队列（global concurrent queues）的工作项相对于提交顺序没有排序保证，并且提交到这些队列的工作项可以并发调用（毕竟本质还是并发队列）。

&emsp;调度全局并发队列（dispatch global concurrent queues）是由 `dispatch_get_global_queue` 函数返回的已知全局对象，这些对象无法修改。 `dispatch_suspend`、`dispatch_resume`、`dispatch_set_context` 等等函数对此类型的队列调用无效。
#### dispatch_queue_serial_t
```c++
DISPATCH_DECL_SUBCLASS(dispatch_queue_serial, dispatch_queue);
```
&emsp;转换宏定义后是：
```c++
@protocol OS_dispatch_queue_serial <OS_dispatch_queue>
@end

typedef NSObject<OS_dispatch_queue_serial> * dispatch_queue_serial_t;
```
&emsp;`OS_dispatch_queue_serial` 是继承自 `OS_dispatch_queue` 协议的协议，并且为遵循该协议的 `NSObject` 实例对象类型的指针定义了一个 `dispatch_queue_serial_t` 的别名。

&emsp;调度串行队列（dispatch serial queues）调用以 FIFO 顺序串行提交给它们的工作项。

&emsp;调度串行队列（dispatch serial queues）是轻量级对象，可以向其提交工作项以 FIFO 顺序调用。串行队列一次只能调用一个工作项，但是独立的串行队列可以各自相对于彼此并发地调用其工作项。

&emsp;串行队列可以相互定位（`dispatch_set_target_queue`）（串行队列可以彼此作为目标）。队列层次结构底部的串行队列提供了一个排除上下文：在任何给定的时间，提交给这种层次结构中的任何队列的最多一个工作项将运行。这样的层次结构提供了一个自然的结构来组织应用程序子系统。

&emsp;通过将派生自 `DISPATCH_QUEUE_SERIAL` 的调度队列属性传递给 `dispatch_queue_create_with_target` 来创建串行队列。（串行队列的创建过程后续会通过源码来进行解读）

#### dispatch_queue_main_t
```c++
DISPATCH_DECL_SUBCLASS(dispatch_queue_main, dispatch_queue_serial);
```
&emsp;转换宏定义后是：
```c++
@protocol OS_dispatch_queue_main <OS_dispatch_queue_serial>
@end

typedef NSObject<OS_dispatch_queue_main> * dispatch_queue_main_t;
```
&emsp;`OS_dispatch_queue_main` 是继承自 `OS_dispatch_queue_serial` 协议的协议，并且为遵循该协议的 `NSObject` 实例对象类型的指针定义了一个 `dispatch_queue_main_t` 的别名。（看到这里发现主队列不愧是特殊的串行队列）

&emsp;`dispatch_queue_main_t` 是绑定到主线程的默认队列的类型。

&emsp;主队列是一个串行队列（`dispatch_queue_serial_t`），该队列绑定到应用程序的主线程。为了调用提交到主队列的工作项，应用程序必须调用 `dispatch_main`，`NSApplicationMain` 或在主线程上使用 `CFRunLoop`。

&emsp;主队列是一个众所周知的全局对象，它在进程初始化期间代表主线程自动创建，并由 `dispatch_get_main_queue` 返回，无法修改该对象。`dispatch_suspend`、`dispatch_resume`、`dispatch_set_context` 等等函数对此类型的队列调用无效（主队列只有一个，全局并发队列有多个）。
#### dispatch_queue_concurrent_t
```c++
DISPATCH_DECL_SUBCLASS(dispatch_queue_concurrent, dispatch_queue);
```
&emsp;转换宏定义后是：
```c++
@protocol OS_dispatch_queue_concurrent <OS_dispatch_queue>
@end

typedef NSObject<OS_dispatch_queue_concurrent> * dispatch_queue_concurrent_t;
```
&emsp;`OS_dispatch_queue_concurrent` 是继承自 `OS_dispatch_queue` 协议的协议，并且为遵循该协议的 `NSObject` 实例对象类型的指针定义了一个 `dispatch_queue_concurrent_t` 的别名。

&emsp;调度并发队列（dispatch concurrent queues）会同时调用提交给它们的工作项，并接受屏障工作项的概念（and admit a notion of barrier workitems，（barrier 屏障是指调用 `dispatch_barrier_async` 函数，向队列提交工作项））。

&emsp;调度并发队列（dispatch concurrent queues）是可以向其提交常规和屏障工作项的轻量级对象。在排除其他任何类型的工作项目（按 FIFO 顺序）时，将调用屏障工作项目。（提交在 barrier 工作项之前的工作项并发执行完以后才会并发执行 barrier 工作项之后的工作项）。

&emsp;可以对同一并发队列以任何顺序并发调用常规工作项。但是，在调用之前提交的任何屏障工作项之前，不会调用常规工作项。

&emsp;换句话说，如果在 Dispatch 世界中串行队列等效于互斥锁，则并发队列等效于 reader-writer lock，其中常规项是读取器，而屏障是写入器。

&emsp;通过将派生自 `DISPATCH_QUEUE_CONCURRENT` 的调度队列属性传递给 `dispatch_queue_create_with_target` 来创建并发队列。

&emsp;注意事项：当调用优先级较低的常规工作项（readers）时，此时调度并发队列不会实现优先级反转避免，并且会阻止调用优先级较高的屏障（writer）。

#### dispatch_block_t
&emsp;日常使用 GCD 向队列提交的工作项都是这种名字是 `dispatch_block_t` 参数和返回值都是 `void` 的 Block。
```c++
typedef void (^dispatch_block_t)(void);
```
&emsp;提交给调度队列（dispatch queues）的 blocks 的类型，不带任何参数且没有返回值。当不使用 Objective-C ARC 进行构建时，分配到堆上或复制到堆上的 block 对象必须通过 `-[release]` 消息或 `Block_release()` 函数释放。

&emsp;以字面量形式声明的 block 分配空间存储在栈上。因此如下是一个无效的构建:
```c++
dispatch_block_t block;
if (x) {
    block = ^{ printf("true\n"); };
} else {
    block = ^{ printf("false\n"); };
}
block(); // unsafe!!!
```
&emsp;幕后发生的事情：
```c++
if (x) {
    struct Block __tmp_1 = ...; // setup details
    block = &__tmp_1;
} else {
    struct Block __tmp_2 = ...; // setup details
    block = &__tmp_2;
}
```
&emsp;如示例所示，栈变量的地址正在转义其分配范围。那是一个经典的C bug。相反，必须使用 `Block_copy()` 函数或通过发送 `-[copy]` 消息将 block 复制到堆中。(看到这里对 block 内部结构比较熟悉的同学感觉应该会很亲切。)

#### dispatch_async
&emsp;`dispatch_async` 提交一个 block 以在调度队列上异步执行。
```c++
#ifdef __BLOCKS__
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_async(dispatch_queue_t queue, dispatch_block_t block);
#endif
```
&emsp;`dispatch_async` 函数是用于将 block 提交到调度队列的基本机制。对 `dispatch_async` 函数的调用总是在 block 被提交后立即返回，而从不等待 block 被调用。（即我们熟悉的异步调用不会阻塞当前线程，因为 `dispatch_async` 函数提交 block 以后就立即返回了，对应的 `dispatch_sync` 函数调用则是等 block 执行结束以后才会返回。我们潜意识里可能觉的 `dispatch_async` 函数所提交的队列类型不同会影响该函数是否立即返回 ，这里 `dispatch_async` 函数是否立即返回和提交的队列类型是完全无关的，`dispatch_async` 函数不管是提交 block 到并发队列还是串行队列都会立即返回，不会阻塞当前线程。)

&emsp;目标队列（`dispatch_queue_t queue`）决定是串行调用该块还是同时调用提交到同一队列的其它块。（当 queue 是并发队列时会开启多条线程并发执行所有的 block，如果 queue 是串行队列（除了主队列）的话则是仅开辟一条线程串行执行所有的 block，如果主队列的话则是不开启线程直接在主线程中串行执行所有的 block），`dispatch_async` 函数提交 `block` 到不同的串行队列，则这些串行队列是相互并行处理的。（它们在不同的线程中并发执行串行队列中的 block）

&emsp;`queue`：block 提交到的目标调度队列。系统将在目标队列上保留引用，直到该 block 调用完成为止。在此参数中传递 `NULL` 的结果是不确定的。

&emsp;`block`：提交到目标调度队列的 block。该函数代表调用者执行 `Block_copy()` 和 `Block_release()` 函数。在此参数中传递 `NULL` 的结果是不确定的。

#### dispatch_function_t
&emsp;`dispatch_function_t` 是一个返回值是 void 参数是 void *（可空）的函数指针。
```c++
typedef void (*dispatch_function_t)(void *_Nullable);
```
#### dispatch_async_f
&emsp;`dispatch_async_f` 提交一个函数以在调度队列上异步执行。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NONNULL3 DISPATCH_NOTHROW
void
dispatch_async_f(dispatch_queue_t queue,
        void *_Nullable context, dispatch_function_t work);
```
&emsp;详细信息同上 `dispatch_async`。

&emsp;`queue`：函数被提交到的目标调度队列。系统将在目标队列上保留引用，直到函数执行完返回。在此参数中传递 `NULL` 的结果是不确定的。

&emsp;`context`：应用程序定义的上下文参数，以传递给函数，作为 `work` 函数执行时的参数。

&emsp;`work`：在目标队列上调用的应用程序定义的函数。传递给此函数的第一个参数是提供给 `dispatch_async_f` 的 `context` 参数。在此参数中传递 `NULL` 的结果是不确定的。
#### dispatch_sync
&emsp;`dispatch_sync` 提交一个 block 以在调度队列上同步执行。
```c++
#ifdef __BLOCKS__
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_sync(dispatch_queue_t queue, DISPATCH_NOESCAPE dispatch_block_t block);
#endif
```
&emsp;`dispatch_sync` 函数用于将工作项提交到一个 dispatch queue，像 `dispatch_async` 函数一样，但是 `dispatch_sync` 在工作项完成之前不会返回。(即 `dispatch_sync` 函数只有在提交到队列的 block 执行完成以后才会返回，会阻塞当前线程)。

&emsp;使用 `dispatch_sync` 函数提交到队列的工作项在调用时不会遵守该队列的某些队列属性（例如自动释放频率和 QOS 类）。

&emsp;针对当前队列调用 `dispatch_sync` 将导致死锁（dead-lock）（如在任何串行队列（包括主线程）中调用 `dispatch_sync` 函数提交 block 到当前串行队列，必死锁）。使用 `dispatch_sync` 也会遇到由于使用互斥锁而导致的多方死锁（multi-party dead-lock）问题，最好使用 `dispatch_async`。

&emsp;与 `dispatch_async` 不同，在目标队列上不执行保留。因为对这个函数的调用是同步的，所以 `dispatch_sync` 会 “借用” 调用者的引用。

&emsp;作为一种优化，`dispatch_sync` 在提交该工作项的线程上调用该工作项，除非所传递的队列是主队列或以其为目标的队列（参见 `dispatch_queue_main_t`，`dispatch_set_target_queue`）。

&emsp;`queue`：block 提交到的目标调度队列。在此参数中传递 `NULL` 的结果是不确定的。

&emsp;`block`：在目标调度队列上要调用的 block。在此参数中传递 `NULL` 的结果是不确定的。
#### dispatch_sync_f
&emsp;`dispatch_sync_f` 提交一个函数以在调度队列上同步执行。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NONNULL3 DISPATCH_NOTHROW
void
dispatch_sync_f(dispatch_queue_t queue,
        void *_Nullable context, dispatch_function_t work);
```
&emsp;详细信息同上 `dispatch_sync`。

&emsp;`queue`：函数提交到的目标调度队列。在此参数中传递 `NULL` 的结果是不确定的。

&emsp;`context`：应用程序定义的上下文参数，以传递给函数，作为 `work` 函数执行时的参数。

&emsp;`work`：在目标队列上调用的应用程序定义的函数。传递给此函数的第一个参数是提供给 `dispatch_sync_f` 的 `context` 参数。在此参数中传递 `NULL` 的结果是不确定的。
#### dispatch_async_and_wait
&emsp;`dispatch_async_and_wait` 提交一个 block 以在调度队列上同步执行。
```c++
#ifdef __BLOCKS__
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_async_and_wait(dispatch_queue_t queue,
        DISPATCH_NOESCAPE dispatch_block_t block);
#endif
```
&emsp;将工作项提交到调度队列，如 `dispatch_async`，但是 `dispatch_async_and_wait` 在工作项完成之前不会返回。像 `dispatch_sync` 系列的功能一样，`dispatch_async_and_wait` 也会发生死锁（参见 `dispatch_sync`）。

&emsp;但是，`dispatch_async_and_wait` 在两个基本方面与 `dispatch_sync` 系列的功能不同：它如何考虑队列属性以及如何选择调用工作项的执行上下文。

&emsp;使用 `dispatch_async_and_wait` 提交到队列的工作项在调用时会观察该队列的所有队列属性（包括自动释放频率或 QOS 类）。

&emsp;当 runtime 启动了一个线程来调用已经提交给指定队列的异步工作项时，该服务线程也将用于执行通过 `dispatch_async_and_wait` 提交给队列的同步工作。但是，如果 runtime 没有为指定的队列提供服务的线程（因为它没有排队的工作项，或者只有同步的工作项），那么 `dispatch_async_and_wait` 将在调用线程上调用工作项，类似于 `dispatch_sync` 系列。

&emsp;作为例外，如果提交工作的队列不以全局并发队列为目标（例如，因为它以主队列为目标），则调用 `dispatch_async_and_wait` 的线程将永远不会调用该工作项。

&emsp;换句话说，`dispatch_async_and_wait` 类似于将 `dispatch_block_create` 工作项提交到队列，然后等待它，如下面的代码示例所示。但是，当不需要新线程来执行工作项时，`dispatch_async_and_wait` 效率显著提高（因为它将使用提交线程的堆栈，而不需要堆分配）。
```c++
dispatch_block_t b = dispatch_block_create(0, block);
dispatch_async(queue, b);
dispatch_block_wait(b, DISPATCH_TIME_FOREVER);
Block_release(b);
```
&emsp;`queue`：block 提交到的目标调度队列。在此参数中传递 `NULL` 的结果是不确定的。

&emsp;`block`：在目标调度队列上要调用的 block。在此参数中传递 `NULL` 的结果是不确定的。
#### dispatch_async_and_wait_f
&emsp;`dispatch_async_and_wait_f` 提交一个函数以在调度队列上同步执行。
```c++
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NONNULL3 DISPATCH_NOTHROW
void
dispatch_async_and_wait_f(dispatch_queue_t queue,
        void *_Nullable context, dispatch_function_t work);
```
&emsp;详细信息同上 `dispatch_async_and_wait`。

&emsp;`queue`：函数提交到的目标调度队列。在此参数中传递 `NULL` 的结果是不确定的。

&emsp;`context`：应用程序定义的上下文参数，以传递给函数，作为 `work` 函数执行时的参数。

&emsp;`work`：在目标队列上调用的应用程序定义的函数。传递给此函数的第一个参数是提供给 `dispatch_async_and_wait_f` 的 `context` 参数。在此参数中传递 `NULL` 的结果是不确定的。
#### DISPATCH_APPLY_AUTO
&emsp;把 0 强转为 `dispatch_queue_t`，作为一个常量使用。
&emsp;`DISPATCH_APPLY_AUTO_AVAILABLE` 宏定义在高于 macOS 10.9、高于 iOS 7.0 和任何 tvOS 或 watchOS 版本中值为 1，其它情况为 0。
```c++
#if DISPATCH_APPLY_AUTO_AVAILABLE
#define DISPATCH_APPLY_AUTO ((dispatch_queue_t _Nonnull)0) // 把 0 强转为 dispatch_queue_t
#endif
```
&emsp;传递给 `dispatch_apply` 或 `dispatch_apply_f` 的常数，以请求系统自动使用与当前线程的配置尽可能接近的工作线程。

&emsp;当提交一个 block 进行并行调用时，将此常量作为队列参数传递，将自动使用与调用方的服务质量最匹配的全局并发队列。(即 `dispatch_apply` 和 `dispatch_apply_f` 函数的第二个参数: `dispatch_queue_t queue` 使用 `DISPATCH_APPLY_AUTO`)

&emsp;注意事项：不应假设将实际使用哪个全局并发队列。使用此常量将向后部署到 macOS 10.9，iOS 7.0 和任何 tvOS 或 watchOS 版本。

#### dispatch_apply
&emsp;`dispatch_apply` 将一个 block 提交给调度队列以进行并行调用。(快速迭代)
```c++
#ifdef __BLOCKS__
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL3 DISPATCH_NOTHROW
void
dispatch_apply(size_t iterations,
        dispatch_queue_t DISPATCH_APPLY_QUEUE_ARG_NULLABILITY queue,
        DISPATCH_NOESCAPE void (^block)(size_t));
#endif
```
&emsp;将一个 block 提交给调度队列以进行并行调用。该函数在返回之前等待任务块完成，如果指定的队列是并发的，则该块可以并发调用，因此必须是可重入的安全块。

&emsp;每次调用该块都将传递当前的迭代索引。

&emsp;`iterations`：要执行的迭代次数。

&emsp;`queue`：block 提交到的调度队列。传递的首选值是 `DISPATCH_APPLY_AUTO`，以自动使用适合于调用线程的队列。

&emsp;`block`：要调用的 block 具有指定的迭代次数。在此参数中传递 `NULL` 的结果是不确定的。

> &emsp; 通常我们会用 for 循环遍历，但是 GCD 给我们提供了快速迭代的方法 dispatch_apply。dispatch_apply 按照指定的次数将指定的任务追加到指定的队列中，并等待全部队列执行结束。
> 如果是在串行队列中使用 dispatch_apply，那么就和 for 循环一样，按顺序同步执行。但是这样就体现不出快速迭代的意义了。
> 我们可以利用并发队列进行异步执行。比如说遍历 0~5 这 6 个数字，for 循环的做法是每次取出一个元素，逐个遍历。dispatch_apply 可以 在多个线程中同时（异步）遍历多个数字。还有一点，无论是在串行队列，还是并发队列中，dispatch_apply 都会等待全部任务执行完毕，这点就像是同步操作，也像是队列组中的 dispatch_group_wait 方法。因为是在并发队列中异步执行任务，所以各个任务的执行时间长短不定，最后结束顺序也不定。但是 apply---end 一定在最后执行。这是因为 dispatch_apply 方法会等待全部任务执行完毕。
#### dispatch_apply_f
&emsp;`dispatch_apply_f` 将一个函数提交给调度队列以进行并行调用。(快速迭代)
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL4 DISPATCH_NOTHROW
void
dispatch_apply_f(size_t iterations,
        dispatch_queue_t DISPATCH_APPLY_QUEUE_ARG_NULLABILITY queue,
        void *_Nullable context, void (*work)(void *_Nullable, size_t));
```
&emsp;详细信息同上 `dispatch_apply`。
#### dispatch_get_current_queue
&emsp;`dispatch_get_current_queue` 返回正在运行当前正在执行的块的队列。
```c++
API_DEPRECATED("unsupported interface", macos(10.6,10.9), ios(4.0,6.0)) // 已废弃，请勿再使用
DISPATCH_EXPORT DISPATCH_PURE DISPATCH_WARN_RESULT DISPATCH_NOTHROW
dispatch_queue_t
dispatch_get_current_queue(void);
```
&emsp;在已提交块的上下文之外调用 `dispatch_get_current_queue()` 时，它将返回默认的并发队列。

&emsp;建议仅用于调试和日志记录：代码不得对返回的队列进行任何假设，除非它是全局队列之一或代码自身创建的队列。如果队列不是 `dispatch_get_current_queue()` 返回的队列，则代码不能假定对该队列的同步执行不会出现死锁。

&emsp;在主线程上调用 `dispatch_get_current_queue()` 时，它可能会返回与 `dispatch_get_main_queue()` 相同的值，也可能不返回相同的值。比较两者并不是测试代码是否在主线程上执行的有效方法（参见 `dispatch_assert_queue` 和 `dispatch_assert_queue_not`）。

&emsp;返回当前队列。此功能已弃用，在以后的版本中将被删除。

#### dispatch_get_main_queue
&emsp;`dispatch_get_main_queue` 返回绑定到主线程的默认队列。(_dispatch_main_q 一个全局变量，程序启动时会自动构建主线程和主队列)
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT
struct dispatch_queue_s _dispatch_main_q; // _dispatch_main_q 是一个全局的 dispatch_queue_s 结构体变量

DISPATCH_INLINE DISPATCH_ALWAYS_INLINE DISPATCH_CONST DISPATCH_NOTHROW
dispatch_queue_main_t
dispatch_get_main_queue(void)
{
    // DISPATCH_GLOBAL_OBJECT 宏把 _dispatch_main_q 转化为 dispatch_queue_main_t 并返回。
    return DISPATCH_GLOBAL_OBJECT(dispatch_queue_main_t, _dispatch_main_q);
}

#define OS_OBJECT_BRIDGE __bridge

// 这个宏定义也很简单，只是简单的把 object 转化为 type 类型
#define DISPATCH_GLOBAL_OBJECT(type, object) ((OS_OBJECT_BRIDGE type)&(object))
```
&emsp;为了调用提交到主队列的块，应用程序必须调用 `dispatch_main()`、`NSApplicationMain()` 或在主线程上使用 `CFRunLoop`。

&emsp;主队列用于在应用程序上下文中与主线程和主 runloop 进行交互。

&emsp;由于主队列的行为不完全像常规串行队列，因此在非 UI 应用程序（守护程序）的进程中使用时，主队列可能会产生有害的副作用。对于此类过程，应避免使用主队列。

&emsp;返回主队列。在调用 `main()` 之前，该队列代表主线程自动创建。（`_dispatch_main_q`）

#### dispatch_queue_priority_t
&emsp;`dispatch_queue_priority` 的类型，表示队列的优先级。
```c++
#define DISPATCH_QUEUE_PRIORITY_HIGH 2
#define DISPATCH_QUEUE_PRIORITY_DEFAULT 0
#define DISPATCH_QUEUE_PRIORITY_LOW (-2)
#define DISPATCH_QUEUE_PRIORITY_BACKGROUND INT16_MIN

typedef long dispatch_queue_priority_t;
```
&emsp;`DISPATCH_QUEUE_PRIORITY_HIGH`：调度到队列的项目将以高优先级运行，即队列将在任何默认优先级或低优先级队列之前被调度执行。

&emsp;`DISPATCH_QUEUE_PRIORITY_DEFAULT`：调度到队列的项目将以默认优先级运行，即，在所有高优先级队列都已调度之后，但在任何低优先级队列都已调度之前，将调度该队列执行。

&emsp;`DISPATCH_QUEUE_PRIORITY_LOW`：调度到队列的项目将以低优先级运行，即，在所有默认优先级和高优先级队列都已调度之后，将调度该队列执行。

&emsp;`DISPATCH_QUEUE_PRIORITY_BACKGROUND`：调度到队列的项目将在后台优先级下运行，即在所有较高优先级的队列都已调度之后，将调度该队列执行，并且系统将在线程上以 `setpriority(2)` 的后台状态运行该队列上的项目（即磁盘 I/O 受到限制，线程的调度优先级设置为最低值）。

#### dispatch_get_global_queue
&emsp;`dispatch_get_global_queue` 返回给定服务质量（qos_class_t）（或者 dispatch_queue_priority_t 定义的优先级）类的众所周知的全局并发队列。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_CONST DISPATCH_WARN_RESULT DISPATCH_NOTHROW
dispatch_queue_global_t
dispatch_get_global_queue(long identifier, unsigned long flags);
```
&emsp;`identifier`：在 `qos_class_t` 中定义的服务质量等级或在 `dispatch_queue_priority_t` 中定义的优先级。

&emsp;建议使用服务质量类值来识别众所周知的全局并发队列：
+ `QOS_CLASS_USER_INTERACTIVE`
+ `QOS_CLASS_USER_INITIATED`
+ `QOS_CLASS_DEFAULT`
+ `QOS_CLASS_UTILITY`

&emsp;全局并发队列仍可以通过其优先级来标识，这些优先级映射到以下QOS类：
+ `DISPATCH_QUEUE_PRIORITY_HIGH:  QOS_CLASS_USER_INITIATED`
+ `DISPATCH_QUEUE_PRIORITY_DEFAULT:  QOS_CLASS_DEFAULT` 
+ `DISPATCH_QUEUE_PRIORITY_LOW:  QOS_CLASS_UTILITY`
+ `DISPATCH_QUEUE_PRIORITY_BACKGROUND:  QOS_CLASS_BACKGROUND`

&emsp;`flags`：保留以备将来使用。传递除零以外的任何值可能会导致返回 `NULL`，所以日常统一传 0 就好了。

&emsp;`result`：返回请求的全局队列，如果请求的全局队列不存在，则返回 `NULL`。

#### dispatch_queue_attr_t
&emsp;调度队列的属性。
```c++
DISPATCH_DECL(dispatch_queue_attr);
```
&emsp;转换宏定义后是：
```c++
@protocol OS_dispatch_queue_attr <OS_dispatch_object>
@end

typedef NSObject<OS_dispatch_queue_attr> * dispatch_queue_attr_t;
```
&emsp;`OS_dispatch_queue_attr` 是继承自 `OS_dispatch_object` 协议的协议，并且为遵循该协议的 `NSObject` 实例对象类型的指针定义了一个 `dispatch_queue_attr_t` 的别名。（`dispatch_queue_attr_t` 具体是不是 NSObject 后面待确认）

#### dispatch_queue_attr_make_initially_inactive
&emsp;`dispatch_queue_attr_make_initially_inactive` 返回一个属性值，该值可提供给 `dispatch_queue_create` 或 `dispatch_queue_create_with_target`，以便使创建的队列最初处于非活动状态。
```c++
API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
DISPATCH_EXPORT DISPATCH_WARN_RESULT DISPATCH_PURE DISPATCH_NOTHROW
dispatch_queue_attr_t
dispatch_queue_attr_make_initially_inactive(
        dispatch_queue_attr_t _Nullable attr);
```
&emsp;调度队列可以在非活动状态下创建。必须先激活处于这种状态的队列，然后才能调用与其关联的任何 blocks。

&emsp;无法释放处于非活动状态的队列，必须在释放使用此属性创建的队列的最后一个引用之前调用 `dispatch_activate`。

&emsp;可以使用 `dispatch_set_target_queue` 更改处于非活动状态的队列的目标队列。一旦最初不活动的队列被激活，就不再允许更改目标队列。

&emsp;`attr`：队列属性值要与最初不活动的属性组合。

&emsp;`return`：返回可以提供给 `dispatch_queue_create` 和 `dispatch_queue_create_with_target` 的属性值。新值将 “attr” 参数指定的属性与最初处于非活动状态的属性结合在一起。

#### DISPATCH_QUEUE_SERIAL
&emsp;`DISPATCH_QUEUE_SERIAL` 宏定义，仅是一个 `NULL`，`dispatch_queue_t serialQueue = dispatch_queue_create("com.com", DISPATCH_QUEUE_SERIAL);` 日常创建串行队列必使用的一个宏，其实是 `NULL`。
```c++
#define DISPATCH_QUEUE_SERIAL NULL
```
&emsp;可用于创建以 FIFO 顺序串行调用块的调度队列的属性。(`dispatch_queue_serial_t`)

#### DISPATCH_QUEUE_SERIAL_INACTIVE
```c++
#define DISPATCH_QUEUE_SERIAL_INACTIVE \
        dispatch_queue_attr_make_initially_inactive(DISPATCH_QUEUE_SERIAL)
```
&emsp;可用于创建以 FIFO 顺序，顺序调用块的调度队列的属性，该属性最初是不活动的。

#### DISPATCH_QUEUE_CONCURRENT
&emsp;可用于创建调度队列的属性，该调度队列可同时调用块并支持通过调度屏障 API 提交的屏障块。(常规 block 和 barrier 的 block 任务块)
```c++
#define DISPATCH_GLOBAL_OBJECT(type, object) ((OS_OBJECT_BRIDGE type)&(object))

#define DISPATCH_QUEUE_CONCURRENT \
        DISPATCH_GLOBAL_OBJECT(dispatch_queue_attr_t, \
        _dispatch_queue_attr_concurrent)
API_AVAILABLE(macos(10.7), ios(4.3))
DISPATCH_EXPORT
struct dispatch_queue_attr_s _dispatch_queue_attr_concurrent; // 这里有一个 dispatch_queue_attr_s 结构体类型的全局变量。
```
&emsp;同上面类似，`DISPATCH_QUEUE_CONCURRENT` 宏定义是把全局变量 `_dispatch_queue_attr_concurrent` 强制转化为了 `dispatch_queue_attr_t`。

#### DISPATCH_QUEUE_CONCURRENT_INACTIVE
&emsp;可用于创建调度队列的属性，该属性可以同时调用块并支持通过调度屏障 API （`dispatch_barrier_async`）提交的屏障块，并且该属性最初是不活动的。
```c++
#define DISPATCH_QUEUE_CONCURRENT_INACTIVE \
        dispatch_queue_attr_make_initially_inactive(DISPATCH_QUEUE_CONCURRENT)
```
#### dispatch_autorelease_frequency_t
&emsp;`dispatch_autorelease_frequency_t` 传递给 `dispatch_queue_attr_make_with_autorelease_frequency` 函数的值。
```c++
// 枚举宏定义
#define DISPATCH_ENUM(name, type, ...) \
typedef enum : type { __VA_ARGS__ } __DISPATCH_ENUM_ATTR name##_t

DISPATCH_ENUM(dispatch_autorelease_frequency, unsigned long,
    DISPATCH_AUTORELEASE_FREQUENCY_INHERIT DISPATCH_ENUM_API_AVAILABLE(
            macos(10.12), ios(10.0), tvos(10.0), watchos(3.0)) = 0,
    DISPATCH_AUTORELEASE_FREQUENCY_WORK_ITEM DISPATCH_ENUM_API_AVAILABLE(
            macos(10.12), ios(10.0), tvos(10.0), watchos(3.0)) = 1,
    DISPATCH_AUTORELEASE_FREQUENCY_NEVER DISPATCH_ENUM_API_AVAILABLE(
            macos(10.12), ios(10.0), tvos(10.0), watchos(3.0)) = 2,
);
```
&emsp;`DISPATCH_AUTORELEASE_FREQUENCY_INHERIT`：具有这种自动释放频率（autorelease frequency）的调度队列将从其目标队列继承行为，这是手动创建的队列的默认行为。

&emsp;`DISPATCH_AUTORELEASE_FREQUENCY_WORK_ITEM`：具有这种自动释放频率（autorelease frequency）的调度队列在异步提交给它的每个块的执行周围 push 和 pop 一个自动释放池。参考 `dispatch_queue_attr_make_with_autorelease_frequency()`。

&emsp;`DISPATCH_AUTORELEASE_FREQUENCY_NEVER`：具有这种自动释放频率（autorelease frequency）的调度队列永远不会围绕异步执行提交给它的块的执行设置单个自动释放池，这是全局并发队列的行为。

#### dispatch_queue_attr_make_with_autorelease_frequency
&emsp;`dispatch_queue_attr_make_with_autorelease_frequency` 返回自动释放频率（autorelease frequency）设置为指定值的调度队列属性值。
```c++
API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
DISPATCH_EXPORT DISPATCH_WARN_RESULT DISPATCH_PURE DISPATCH_NOTHROW
dispatch_queue_attr_t
dispatch_queue_attr_make_with_autorelease_frequency(
        dispatch_queue_attr_t _Nullable attr,
        dispatch_autorelease_frequency_t frequency);
```
&emsp;当队列使用按工作项自动释放的频率（直接或从其目标队列继承）时，将异步提交到此队列的任何块（通过 `dispatch_async`，`dispatch_barrier_async`，`dispatch_group_notify` 等）执行。如果被单个Objective-C @autoreleasepool 范围包围。

&emsp;当队列使用每个工作项自动释放频率（直接或从其目标队列继承）时，异步提交到此队列的任何块（通过 `dispatch_async`、`dispatch_barrier_async`、`dispatch_group_notify` 等）都将被执行，就像被单独的 Objective-C `@autoreleasepool` 作用域包围一样。

&emsp;自动释放频率对同步提交到队列（通过 `dispatch_sync`，`dispatch_barrier_sync`）的块没有影响。

&emsp;全局并发队列具有 `DISPATCH_AUTORELEASE_FREQUENCY_NEVER` 行为。手动创建的调度队列默认情况下使用 `DISPATCH_AUTORELEASE_FREQUENCY_INHERIT`。

&emsp;使用此属性创建的队列在激活后无法更改目标队列。参考 `dispatch_set_target_queue` 和 `dispatch_activate`。

&emsp;`attr`：要与指定的自动释放频率或 `NULL` 组合的队列属性值。

&emsp;`frequency`：请求的自动释放频率。

&emsp;`return`：返回可以提供给 `dispatch_queue_create` 的属性值，如果请求的自动释放频率无效，则为 `NULL`。这个新值结合了 `attr` 参数指定的属性和所选的自动释放频率。

#### DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL
&emsp;`DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL` 使用此属性创建的调度队列按 FIFO 顺序串行调用块，并用相当于单个 Objective-C @autoreleasepool 作用域来包围异步提交给它的任何块的执行。
```c++
#define DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL \
        dispatch_queue_attr_make_with_autorelease_frequency(\
                DISPATCH_QUEUE_SERIAL, DISPATCH_AUTORELEASE_FREQUENCY_WORK_ITEM)
```
#### DISPATCH_QUEUE_CONCURRENT_WITH_AUTORELEASE_POOL
&emsp;`DISPATCH_QUEUE_CONCURRENT_WITH_AUTORELEASE_POOL` 使用此属性创建的调度队列可以并发调用块，并支持使用 dispatch barrier API 提交的 barrier blocks。它还包围了异步提交给它的任何块的执行，这些块相当于一个单独的 Objective-C @autoreleasepool。
```c++
#define DISPATCH_QUEUE_CONCURRENT_WITH_AUTORELEASE_POOL \
        dispatch_queue_attr_make_with_autorelease_frequency(\
                DISPATCH_QUEUE_CONCURRENT, DISPATCH_AUTORELEASE_FREQUENCY_WORK_ITEM)
```
#### dispatch_qos_class_t
&emsp;`qos_class_t` 类型的别名。
```c++
#if __has_include(<sys/qos.h>)
typedef qos_class_t dispatch_qos_class_t;
#else
typedef unsigned int dispatch_qos_class_t;
#endif
```
#### qos_class_t
&emsp;An abstract thread quality of service (QOS) classificatio。thread quality of service（QOS）的抽象种类。
```c++
// __QOS_ENUM 枚举宏定义
#define __QOS_ENUM(name, type, ...) enum { __VA_ARGS__ }; typedef type name##_t
#define __QOS_CLASS_AVAILABLE(...)

#if defined(__cplusplus) || defined(__OBJC__) || __LP64__
#if defined(__has_feature) && defined(__has_extension)
#if __has_feature(objc_fixed_enum) || __has_extension(cxx_strong_enums)
#undef __QOS_ENUM
#define __QOS_ENUM(name, type, ...) typedef enum : type { __VA_ARGS__ } name##_t
#endif
#endif
#if __has_feature(enumerator_attributes)
#undef __QOS_CLASS_AVAILABLE
#define __QOS_CLASS_AVAILABLE __API_AVAILABLE
#endif
#endif

// 具体的枚举值看这里
__QOS_ENUM(qos_class, unsigned int,
    QOS_CLASS_USER_INTERACTIVE
            __QOS_CLASS_AVAILABLE(macos(10.10), ios(8.0)) = 0x21,
    QOS_CLASS_USER_INITIATED
            __QOS_CLASS_AVAILABLE(macos(10.10), ios(8.0)) = 0x19,
    QOS_CLASS_DEFAULT
            __QOS_CLASS_AVAILABLE(macos(10.10), ios(8.0)) = 0x15,
    QOS_CLASS_UTILITY
            __QOS_CLASS_AVAILABLE(macos(10.10), ios(8.0)) = 0x11,
    QOS_CLASS_BACKGROUND
            __QOS_CLASS_AVAILABLE(macos(10.10), ios(8.0)) = 0x09,
    QOS_CLASS_UNSPECIFIED
            __QOS_CLASS_AVAILABLE(macos(10.10), ios(8.0)) = 0x00,
);
```
&emsp;为了便于记忆，姑且把 thread quality of service 翻译为线程服务质量。

&emsp;线程服务质量（QOS）类是 pthread、dispatch queue 或 NSOperation 预期执行的工作性质的有序抽象表示。每个类指定该频带的最大线程调度优先级（可与频带内的相对优先级偏移结合使用），以及计时器延迟、CPU吞吐量、I/O吞吐量、网络套接字流量管理行为等的服务质量特征。

&emsp;尽最大努力将可用的系统资源分配给每个 QOS 类。服务质量（Quality of service）降低仅发生在系统资源争用期间，与 QOS 等级成比例。也就是说，QOS 类代表用户发起的工作试图达到峰值吞吐量，而 QOS 类则试图实现峰值能量和热效率，即使在没有争用的情况下。最后，QOS 类的使用不允许线程取代可能应用于整个进程的任何限制。

+ `QOS_CLASS_USER_INTERACTIVE`：指示此线程执行的工作是用户交互。要求此类工作相对于系统上的其他工作具有较高的优先级。指定此 QOS 类是一个请求，要求即使在竞争中也要使用几乎所有可用的系统 CPU 和 I/O 带宽运行。这不是用于大型任务的高能效 QOS 类。该 QOS 类的使用应限于与用户的关键交互，如处理主事件循环上的事件、视图绘制、动画等。(可以直白的理解为告诉系统更高优先级的处理用户的交互事件)
+ `QOS_CLASS_USER_INITIATED`：指示此线程执行的工作是由用户启动的，并且用户可能正在等待结果。要求此类工作在低于关键用户交互性工作的优先级下运行，但要相对于系统上的其他工作更高。这不是用于大型任务的高能效 QOS 类。它的使用应限于持续时间足够短的操作，以使用户在等待结果时不太可能切换任务。典型的用户启动的工作将通过占位符内容或模式用户界面的显示来指示进度。
+ `QOS_CLASS_DEFAULT`：在没有更具体的 QOS 类信息的情况下，系统使用的默认 QOS 类。要求此类工作的优先级低于关键用户交互性（INTERACTIVE）和用户启动（INITIATED）的工作，但相对高于实用程序（UTILITY）和后台（BACKGROUND）任务。`pthread_create` 创建的没有指定 QOS 类属性的线程将默认为 `QOS_CLASS_DEFAULT`。此 QOS 类值不打算用作工作分类，它只应在传播或恢复系统提供的 QOS 类值时设置。
+ `QOS_CLASS_UTILITY`：指示由该线程执行的工作的 QOS 类可能由用户启动或未启动，并且用户不太可能立即等待结果。要求此类工作的优先级低于关键的用户交互性（INTERACTIVE）和用户启动（INITIATED）的工作，但要比低级别的系统维护任务高。使用此 QOS 类**表示工作应以节能高效的方式进行**。实用程序工作的进度可能会显示给用户，也可能不会显示给用户，但是这种工作的效果是用户可见的。
+ `QOS_CLASS_BACKGROUND`：指示该线程执行的工作的 QOS 类不是由用户启动的，并且用户可能不知道结果。这类工作要求优先于其他工作，这个 QOS 类的使用表明工作应该以最节能和最高效的方式运行。
+ `QOS_CLASS_UNSPECIFIED`：指示 QOS 类信息的缺失或删除。作为 API 返回值，可能表示线程或 pthread 属性配置为旧 API 不兼容或与 QOS 类系统冲突。
#### QOS_MIN_RELATIVE_PRIORITY
&emsp;可以在 QOS 类中指定的最小相对优先级。这些优先级仅在给定的 QOS 类内是相对的，并且仅对当前过程有意义。
```c++
#define QOS_MIN_RELATIVE_PRIORITY (-15)
```
#### dispatch_queue_attr_make_with_qos_class
&emsp;`dispatch_queue_attr_make_with_qos_class` 返回可以提供给 `dispatch_queue_create` 或 `dispatch_queue_create_with_target` 的属性值，以便为队列分配 QOS 类（形参：qos_class）和相对优先级（形参：relative_priority）。
```c++
API_AVAILABLE(macos(10.10), ios(8.0))
DISPATCH_EXPORT DISPATCH_WARN_RESULT DISPATCH_PURE DISPATCH_NOTHROW
dispatch_queue_attr_t
dispatch_queue_attr_make_with_qos_class(dispatch_queue_attr_t _Nullable attr,
        dispatch_qos_class_t qos_class, int relative_priority);
```
&emsp;如果以此方式指定，则 QOS 类和相对优先级优先于从调度队列的目标队列（如果有）继承的优先级，只要不会导致较低的 QOS 类和相对优先级即可。

&emsp;全局队列优先级映射到以下 QOS 类：
+ `DISPATCH_QUEUE_PRIORITY_HIGH`: `QOS_CLASS_USER_INITIATED`
+ `DISPATCH_QUEUE_PRIORITY_DEFAULT`: `QOS_CLASS_DEFAULT`
+ `DISPATCH_QUEUE_PRIORITY_LOW`: `QOS_CLASS_UTILITY`
+ `DISPATCH_QUEUE_PRIORITY_BACKGROUND`: `QOS_CLASS_BACKGROUND`

&emsp;例如：
```c++
dispatch_queue_t queue;
dispatch_queue_attr_t attr;

// 实参是 DISPATCH_QUEUE_SERIAL 和 QOS_CLASS_UTILITY
attr = dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL, QOS_CLASS_UTILITY, 0);

queue = dispatch_queue_create("com.example.myqueue", attr);
```
&emsp;以这种方式在队列上设置的 QOS 类和相对优先级对同步提交到队列的块没有影响（通过 `dispatch_sync`、`dispatch_barrier_sync`）。

&emsp;`attr`：要与 QOS 类组合的队列属性值，或者为 `NULL`。

&emsp;`qos_class`：QOS 类值，只能传递上面现有的枚举值，传递任何其他值都会导致返回 `NULL`。

&emsp;`relative_priority`：QOS 类中的相对优先级。该值是给定类别与最大支持的调度程序优先级的负偏移量，传递大于零或小于 `QOS_MIN_RELATIVE_PRIORITY`（-15）的值将导致返回 `NULL`。

&emsp;`return`：返回可以提供给 `dispatch_queue_create` 和 `dispatch_queue_create_with_target` 的属性值；如果请求了无效的 `QOS` 类，则返回 `NULL`。新值结合了 `attr` 参数指定的属性，新的 `QOS` 类（形参：qos_class）和相对优先级（形参：relative_priority）。

#### DISPATCH_TARGET_QUEUE_DEFAULT
&emsp;`DISPATCH_TARGET_QUEUE_DEFAULT` 传递给 `dispatch_queue_create_with_target`、`dispatch_set_target_queue` 和 `dispatch_source_create` 函数的常量，以指示应使用相关对象类型的默认目标队列。
```c++
#define DISPATCH_TARGET_QUEUE_DEFAULT NULL
```
#### dispatch_queue_create_with_target
&emsp;`dispatch_queue_create_with_target` 用指定的目标队列创建一个新的调度队列。
```c++
API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
DISPATCH_EXPORT DISPATCH_MALLOC DISPATCH_RETURNS_RETAINED DISPATCH_WARN_RESULT
DISPATCH_NOTHROW
dispatch_queue_t
dispatch_queue_create_with_target(const char *_Nullable label,
        dispatch_queue_attr_t _Nullable attr, dispatch_queue_t _Nullable target)
        DISPATCH_ALIAS_V2(dispatch_queue_create_with_target);
```
&emsp;使用 `DISPATCH_QUEUE_SERIAL` 或 `NULL` 属性创建的调度队列按 `FIFO` 顺序依次调用块。(串行队列)

&emsp;使用 `DISPATCH_QUEUE_CONCURRENT` 属性创建的调度队列可以并发调用块（类似于全局并发队列（`dispatch_get_global_queue` 函数获取的队列），但可能会有更多开销），并支持通过调度屏障 API （`dispatch_barrier_async` 函数）提交的屏障块（barrier blocks），例如实现有效的读写器方案（多读单写模型）。

&emsp;当不再需要调度队列时，应使用 `dispatch_release` 释放它。请注意，异步提交到队列的任何待处理块（pending blocks）都将保存对该队列的引用。因此，在所有待处理块（pending blocks）都完成之前，不会释放队列。

&emsp;使用 `dispatch_queue_create_with_target` 创建的队列不能更改其目标队列，除非创建的队列处于非活动状态（参阅 `dispatch_queue_attr_make_initially_inactive`），在这种情况下，可以更改目标队列，直到使用 `dispatch_activate` 激活新创建的队列为止。

&emsp;`label`：附加到队列的字符串标签，此参数是可选的，可以为 `NULL`。

&emsp;`attr`：预定义的属性，例如 `DISPATCH_QUEUE_SERIAL`，`DISPATCH_QUEUE_CONCURRENT` 或调用 `dispatch_queue_attr_make_with_ *` 函数的结果。

&emsp;`target`：新创建的队列的目标队列。目标队列被保留（retained），如果此参数为 `DISPATCH_TARGET_QUEUE_DEFAULT`，则将队列的目标队列设置为给定队列类型的默认目标队列。
#### dispatch_queue_create
&emsp;`dispatch_queue_create` 创建一个新的调度队列，可以向其提交块（blocks）。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_MALLOC DISPATCH_RETURNS_RETAINED DISPATCH_WARN_RESULT
DISPATCH_NOTHROW
dispatch_queue_t
dispatch_queue_create(const char *_Nullable label,
        dispatch_queue_attr_t _Nullable attr);
```
&emsp;使用 `DISPATCH_QUEUE_SERIAL` 或 `NULL` 属性创建的调度队列按 FIFO 顺序依次调用块。

&emsp;使用 `DISPATCH_QUEUE_CONCURRENT` 属性创建的调度队列可以并发调用块（类似于全局并发队列（`dispatch_get_global_queue` 函数获取的队列），但可能会有更多开销），并支持通过调度屏障 API （`dispatch_barrier_async` 函数）提交的屏障块（blocks），例如实现有效的读写器方案（多读单写模型）。

&emsp;当不再需要调度队列时，应使用 `dispatch_release` 释放它。请注意，异步提交到队列的任何待处理块（pending blocks）都将保存对该队列的引用。因此，在所有待处理块（pending blocks）都完成之前，不会释放队列。

&emsp;通过将`dispatch_queue_attr_make_with_qos_class` 函数的结果传递给此函数的 attr 参数，可以为新创建的队列指定服务质量类（`dispatch_qos_class_t qos_class`）和相对优先级（`int relative_priority`）。这样指定的服务质量等级优先于新创建的调度队列的目标队列（如果有）的服务质量等级，只要这不会导致较低的 QOS 类和相对优先级。

&emsp;如果未指定服务质量等级，则新创建的调度队列的目标队列是**默认优先级的全局并发队列**。

&emsp;`label`：附加到队列的字符串标签。此参数是可选的，可以为 `NULL`。

&emsp;`attr`：预定义的属性，例如 `DISPATCH_QUEUE_SERIAL`，`DISPATCH_QUEUE_CONCURRENT` 或调用 `dispatch_queue_attr_make_with_ *` 函数的结果。

&emsp;`result`：返回新创建的 dispatch queue。
#### DISPATCH_CURRENT_QUEUE_LABEL
&emsp;传递给 `dispatch_queue_get_label` 函数的常量，以检索当前队列的标签。
```c++
#define DISPATCH_CURRENT_QUEUE_LABEL NULL
```
#### dispatch_queue_get_label
&emsp;`dispatch_queue_get_label` 返回在创建队列时指定的给定队列的标签，如果指定了 `NULL` 标签，则返回空字符串。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_PURE DISPATCH_WARN_RESULT DISPATCH_NOTHROW
const char *
dispatch_queue_get_label(dispatch_queue_t _Nullable queue);
```
&emsp;传递 `DISPATCH_CURRENT_QUEUE_LABEL` 将返回当前队列的标签。

&emsp;`queue` 要查询的队列，或 `DISPATCH_CURRENT_QUEUE_LABEL`。
#### dispatch_queue_get_qos_class
&emsp;`dispatch_queue_get_qos_class` 返回给定队列的 QOS 类和相对优先级（传入一个 `int` 指针 `relative_priority_ptr` 来获取相对优先级，QOS 类则是函数返回 `dispatch_qos_class_t` 类型的值）。
```c++
API_AVAILABLE(macos(10.10), ios(8.0))
DISPATCH_EXPORT DISPATCH_WARN_RESULT DISPATCH_NONNULL1 DISPATCH_NOTHROW
dispatch_qos_class_t
dispatch_queue_get_qos_class(dispatch_queue_t queue,
        int *_Nullable relative_priority_ptr);
```
&emsp;如果给定队列是使用从 `dispatch_queue_attr_make_with_qos_class` 返回的属性值创建的，则此函数返回当时指定的 QOS 类和相对优先级；对于任何其他属性值，它将返回 `QOS_CLASS_UNSPECIFIED` 的 QOS 类和相对优先级 0。

&emsp;如果给定队列是全局队列之一，则此函数返回在 `dispatch_get_global_queue` 下记录的已分配 QOS 类值，相对优先级为 0；否则，返回 0。对于主队列，它返回由 `qos_class_main` 提供的 QOS 值和相对优先级 0。

&emsp;`queue`：要查询的队列。

&emsp;`relative_priority_ptr`：指向 `int` 变量的指针，该变量将用 QOS 类中的相对优先级偏移或 `NULL` 填充。
#### dispatch_set_target_queue
&emsp;`dispatch_set_target_queue` 设置给定对象的目标队列。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NOTHROW
void
dispatch_set_target_queue(dispatch_object_t object,
        dispatch_queue_t _Nullable queue);
```
&emsp;对象的目标队列负责处理对象。

&emsp;如果在创建时未为调度队列指定服务质量等级和相对优先级，则调度队列的服务质量等级将从其目标队列继承。 `dispatch_get_global_queue` 函数可用于获取特定服务质量类的目标队列，但是建议改为使用 `dispatch_queue_attr_make_with_qos_class`。

&emsp;提交到目标队列是另一个串行队列的串行队列的块不会与提交到目标队列或具有相同目标队列的任何其他队列的块同时调用。

&emsp;将循环引入目标队列的层次结构的结果是不确定的。

&emsp;调度源的目标队列指定将其**事件处理程序**和**取消处理程序**块提交到的位置。

&emsp;调度 I/O 通道的目标队列指定在何处执行其 I/O 操作。如果通道的目标队列的优先级设置为 `DISPATCH_QUEUE_PRIORITY_BACKGROUND`，则当存在 I/O 争用时，将限制该队列上 `dispatch_io_read` 或 `dispatch_io_write` 执行的 I/O 操作。

&emsp;对于所有其他调度对象类型，目标队列的唯一功能是确定在何处调用对象的终结函数（object's finalizer function）。

&emsp;通常，更改对象的目标队列是异步操作，不会立即生效，也不会影响已经与指定对象关联的块。

&emsp;但是，如果在调用 `dispatch_set_target_queue` 时某个对象处于非活动状态，则目标队列更改将立即生效，并将影响已经与指定对象关联的块。激活最初不活动的对象后，调用 `dispatch_set_target_queue` 会导致声明并终止过程。

&emsp;如果调度队列处于活动状态并被其他调度对象作为目标，则更改其目标队列将导致未定义的行为。

&emsp;`object`：要修改的对象。在此参数中传递 `NULL` 的结果是不确定的。

&emsp;`queue`：对象的新目标队列。保留队列，并释放先前的目标队列（如果有）。如果 `queue` 是 `DISPATCH_TARGET_QUEUE_DEFAULT`，则将给定对象类型的对象目标队列设置为默认目标队列。
#### dispatch_main
&emsp;`dispatch_main` 执行提交到主队列的块。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NOTHROW DISPATCH_NORETURN
void
dispatch_main(void);
```
&emsp;此函数 “驻留” 主线程，并等待将块提交到主队列，该函数从不返回。

&emsp;在主线程上调用 `NSApplicationMain` 或 `CFRunLoopRun` 的应用程序无需调用 `dispatch_main`。
#### dispatch_time_t
&emsp;`dispatch_time_t` 时间的某种抽象表示；其中零表示 “现在”，而 `DISPATCH_TIME_FOREVER` 表示 “无穷大”，其间的每个值都是不透明的编码。
```c++
typedef uint64_t dispatch_time_t;
```
#### dispatch_after
&emsp;`dispatch_after` 安排一个 block 在指定时间后在给定队列上执行。
```c++
#ifdef __BLOCKS__
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL2 DISPATCH_NONNULL3 DISPATCH_NOTHROW
void
dispatch_after(dispatch_time_t when, dispatch_queue_t queue,
        dispatch_block_t block);
#endif

// 示例：
// 在 serialQueue 队列中 2 秒后执行
dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC), serialQueue, ^{
    NSLog(@"🚥🚥 %@", [NSThread currentThread]);
});
```
&emsp;支持将 `DISPATCH_TIME_NOW` 作为 “when” 参数传递，但不如调用 `dispatch_async` 那样理想。传递 `DISPATCH_TIME_FOREVER` 是不确定的。

&emsp;`when`：`dispatch_time` 或 `dispatch_walltime` 返回的时间里程碑。

&emsp;`queue`：给定块将在指定时间提交到的队列。在此参数中传递 `NULL` 的结果是不确定的。

&emsp;`block`：要执行的代码块。在此参数中传递 `NULL` 的结果是不确定的。
#### dispatch_after_f
&emsp;`dispatch_after_f` 安排一个函数在指定时间后在给定队列上执行，`context` 做为其参数。
```c++
API_AVAILABLE(macos(10.6), ios(4.0))
DISPATCH_EXPORT DISPATCH_NONNULL2 DISPATCH_NONNULL4 DISPATCH_NOTHROW
void
dispatch_after_f(dispatch_time_t when, dispatch_queue_t queue,
        void *_Nullable context, dispatch_function_t work);
```
&emsp;可参考 `dispatch_after` 函数，只是把 block 换成了函数。 
#### Dispatch Barrier API
&emsp;Dispatch barrier API 是一种机制，用于将屏障块（barrier blocks）提交给调度队列，类似于 dispatch_async/dispatch_sync API。它可以实现有效的读取器/写入器方案。

&emsp;屏障块仅在​​提交给使用 `DISPATCH_QUEUE_CONCURRENT` 属性创建的队列时才表现出特殊的行为。在这样的队列上，屏障块将不会运行，直到更早提交给队列的所有块都已完成，并且屏障块之后提交给队列的任何块都将不会运行，直到屏障块已完成。

&emsp;当提交到**全局队列**或未使用 `DISPATCH_QUEUE_CONCURRENT` 属性创建的队列时，屏障块的行为与使用 dispatch_async/dispatch_sync API 提交的块相同。（如果使用 `dispatch_async` 和 `dispatch_barrier_async` 提交到 `dispatch_get_global_queue` 取得的 queue，则并发执行，失去屏障的功能。）

#### dispatch_barrier_async
&emsp;`dispatch_barrier_async` 提交 barrier block 以在调度队列上异步执行。（同 `dispatch_async` 不会阻塞当前线程，直接返回执行接下来的语句，但是后添加的 block 则是等到 barrier block 执行完成后才会开始执行。）
```c++
#ifdef __BLOCKS__
API_AVAILABLE(macos(10.7), ios(4.3))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_barrier_async(dispatch_queue_t queue, dispatch_block_t block);
#endif
```
&emsp;将一个块提交到诸如 `dispatch_async` 之类的调度队列中，但将该块标记为屏障（barrier）（仅与 `DISPATCH_QUEUE_CONCURRENT` 队列相关）。

&emsp;`queue`：块提交到的目标调度队列。系统将在目标队列上保留引用，直到该块完成为止。在此参数中传递 `NULL` 的结果是不确定的。

&emsp;`block`：提交到目标调度队列的块。该函数代表调用者执行 `Block_copy` 和 `Block_release`。在此参数中传递 `NULL` 的结果是不确定的。

#### dispatch_barrier_async_f
&emsp;`dispatch_barrier_async_f` 同上，只是把 `block` 换成了函数。
```c++
API_AVAILABLE(macos(10.7), ios(4.3))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NONNULL3 DISPATCH_NOTHROW
void
dispatch_barrier_async_f(dispatch_queue_t queue,
        void *_Nullable context, dispatch_function_t work);
```

#### dispatch_barrier_sync
&emsp;`dispatch_barrier_sync` 提交屏障块（barrier block）以在调度队列上**同步执行**（会阻塞当前线程，直到 barrier block 执行完成才会返回）。
```c++
#ifdef __BLOCKS__
API_AVAILABLE(macos(10.7), ios(4.3))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_barrier_sync(dispatch_queue_t queue,
        DISPATCH_NOESCAPE dispatch_block_t block);
#endif
```
&emsp;将一个块提交到诸如 `dispatch_sync`之类的调度队列中（阻塞当前线程，直到 block 执行完毕才会返回），但将该块标记为屏障（仅与 `DISPATCH_QUEUE_CONCURRENT` 队列相关）。

&emsp;`queue`：块提交到的目标调度队列。在此参数中传递 `NULL` 的结果是不确定的。

&emsp;`block`：在目标调度队列上要调用的块。在此参数中传递 `NULL` 的结果是不确定的。

#### dispatch_barrier_sync_f
&emsp;同 `dispatch_barrier_sync`，仅是把 block 替换为了函数。
```c++
API_AVAILABLE(macos(10.7), ios(4.3))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NONNULL3 DISPATCH_NOTHROW
void
dispatch_barrier_sync_f(dispatch_queue_t queue,
        void *_Nullable context, dispatch_function_t work);
```
#### dispatch_barrier_async_and_wait
&emsp;`dispatch_barrier_async_and_wait` 提交一个块以在调度队列上同步执行。
```c++
#ifdef __BLOCKS__
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
void
dispatch_barrier_async_and_wait(dispatch_queue_t queue,
        DISPATCH_NOESCAPE dispatch_block_t block);
#endif
```
&emsp;将一个块提交到诸如 `dispatch_async_and_wait` 之类的调度队列中，但将该块标记为屏障（barrier）（仅与 `DISPATCH_QUEUE_CONCURRENT` 队列相关）。

#### dispatch_barrier_async_and_wait_f
&emsp;同 `dispatch_barrier_async_and_wait`，仅是把 block 替换为了函数。 
```c++
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NONNULL3 DISPATCH_NOTHROW
void
dispatch_barrier_async_and_wait_f(dispatch_queue_t queue,
        void *_Nullable context, dispatch_function_t work);
```
#### Dispatch queue-specific contexts
&emsp;这个API允许不同的子系统将上下文与共享队列关联起来，而不会有冲突的风险，并且可以从目标队列层次结构中该队列或其任何子队列上执行的块检索上下文。

#### dispatch_queue_set_specific
&emsp;`dispatch_queue_set_specific` 将子系统特定的上下文与调度队列相关联，以获得子系统特有的 key（这里 key 参数类型是 `const void *` （指针指向可以变，但是指向的内容不能通过该指针修改））。
```c++
API_AVAILABLE(macos(10.7), ios(5.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_NOTHROW
void
dispatch_queue_set_specific(dispatch_queue_t queue, const void *key,
        void *_Nullable context, dispatch_function_t _Nullable destructor);
```
&emsp;当为同一个 key 设置了新的上下文时，或者在释放了对队列的所有引用之后，将使用默认优先级全局并发队列上的上下文调用指定的析构函数。
 
&emsp;`queue`：调度队列进行修改。在此参数中传递 `NULL` 的结果是不确定的。

&emsp;`key`：要为其设置上下文的键，通常是指向特定于子系统的静态变量的指针。 key 只作为指针进行比较，从不取消引用。不建议直接传递字符串常量，保留 `NULL` 键，并忽略为其设置上下文的尝试。

&emsp;`context`：对象的新的特定于子系统的上下文。这可能为 `NULL`。

&emsp;`destructor`：析构函数的指针。这可以为 `NULL`，如果 `context` 为 `NULL`，则将其忽略。

#### dispatch_queue_get_specific
&emsp;`dispatch_queue_get_specific` 返回与调度队列相关联的子系统特定上下文，用于子系统唯一的键。
```c++
API_AVAILABLE(macos(10.7), ios(5.0))
DISPATCH_EXPORT DISPATCH_NONNULL1 DISPATCH_PURE DISPATCH_WARN_RESULT
DISPATCH_NOTHROW
void *_Nullable
dispatch_queue_get_specific(dispatch_queue_t queue, const void *key);
```
&emsp;如果已在指定队列上设置了指定键，则返回该键的上下文。

&emsp;`key`：获取上下文的键，通常是指向特定于子系统的静态变量的指针。key 仅作为指针进行比较，而不会取消引用，不建议直接传递字符串常量。

&emsp;`result`：指定键的上下文；如果未找到上下文，则为 `NULL`。

#### dispatch_get_specific
&emsp;`dispatch_get_specific` 返回子系统唯一的 key 的当前特定于子系统的上下文。
```c++
API_AVAILABLE(macos(10.7), ios(5.0))
DISPATCH_EXPORT DISPATCH_PURE DISPATCH_WARN_RESULT DISPATCH_NOTHROW
void *_Nullable
dispatch_get_specific(const void *key);
```
&emsp;从队列上执行的块调用时，如果指定键已在队列上设置，则返回该键的上下文；否则，返回在队列的目标队列上执行的 `dispatch_get_specific` 的结果；如果当前队列是全局并发队列，则返回 `NULL`。

&emsp;`key`：获取上下文的键，通常是指向特定于子系统的静态变量的指针。key 仅作为指针进行比较，而不会取消引用，不建议直接传递字符串常量。

&emsp;`result`: 指定 key 的上下文；如果未找到上下文，则为 `NULL`。

#### Dispatch assertion API
&emsp;Dispatch assertion API 在运行时断言代码正在给定队列的上下文中执行（或从其执行）。它可用于从保护资源的适当队列中检查访问资源的块是否这样做。它还可以用于验证如果在给定队列上运行可能导致死锁的块永远不会在该队列上执行。

#### dispatch_assert_queue
&emsp;`dispatch_assert_queue` 验证当前块是否在给定的调度队列上执行。
```c++
API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
DISPATCH_EXPORT DISPATCH_NONNULL1
void
dispatch_assert_queue(dispatch_queue_t queue)
        DISPATCH_ALIAS_V2(dispatch_assert_queue);
```
&emsp;某些代码希望在特定的调度队列上运行，此函数验证该期望为真。

&emsp;如果当前正在执行的块已提交给​​指定队列或任何以它为目标的队列（参阅 `dispatch_set_target_queue`），则此函数返回。

&emsp;如果当前执行的块是使用同步 API 提交的（`dispatch_sync`，`dispatch_barrier_sync` 等），则也会（递归地）评估提交块的上下文。如果发现同步提交的块本身已提交到指定队列或任何以它为目标的队列，则此函数返回。

&emsp;否则，此函数将声明：将解释记录到系统日志并终止应用程序。

&emsp;将 `dispatch_get_main_queue` 的结果传递给此函数可验证当前块是否已提交到主队列或提交给它的队列，或者是否正在主线程上运行（在任何上下文中）。

&emsp;当在提交的块的上下文之外（例如，从使用 `pthread_create` 手动创建的线程的上下文中）调用 `dispatch_assert_queue` 时，此函数还将声明并终止应用程序。

&emsp;`queue`：当前块应在其上运行的调度队列。在此参数中传递 `NULL` 的结果是不确定的。
#### dispatch_assert_queue_barrier
&emsp;`dispatch_assert_queue_barrier` 验证当前块是否在给定的调度队列上执行，并且该块充当该队列上的屏障（barrier）。
```c++
API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
DISPATCH_EXPORT DISPATCH_NONNULL1
void
dispatch_assert_queue_barrier(dispatch_queue_t queue);
```
&emsp;行为与 `dispatch_assert_queue` 完全一样，另外还要检查当前块是否充当指定队列上的屏障，如果指定队列是串行的，则始终为 true（参见 `DISPATCH_BLOCK_BARRIER` 或 `dispatch_barrier_async`）。

&emsp;`queue`：当前块应作为屏障运行的调度队列。在此参数中传递 `NULL` 的结果是不确定的。
#### dispatch_assert_queue_not
&emsp;`dispatch_assert_queue_not` 验证当前块不在给定调度队列上执行。
```c++
API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
DISPATCH_EXPORT DISPATCH_NONNULL1
void
dispatch_assert_queue_not(dispatch_queue_t queue)
        DISPATCH_ALIAS_V2(dispatch_assert_queue_not);
```
&emsp;等效于 `dispatch_assert_queue`，但相等性测试却相反。这意味着它将在 `dispatch_assert_queue` 返回时终止应用程序，反之亦然。
 
&emsp;`queue`：当前块不应在其上运行的调度队列。在此参数中传递 `NULL` 的结果是不确定的。
 
&emsp;`dispatch_assert_queue_debug`、`dispatch_assert_queue_barrier_debug`、`dispatch_assert_queue_not_debug` 仅在 `DEBUG` 模式下可用。

&emsp;至此 <dispatch/queue.h> 文件看完了，作为 dispatch 中最大的一个文件，包含的信息还是挺多的，需要耐心学习。⛽️⛽️

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

