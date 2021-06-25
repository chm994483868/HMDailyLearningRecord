# iOS 多线程知识体系构建(二)：Pthreads、NSThread篇

> &emsp;本篇首先来学习 iOS 多线程技术中的 Pthreads 技术。⛽️⛽️

> &emsp;可移植操作系统接口（英语：Portable Operating System Interface，缩写为POSIX）是 IEEE（电气和电子工程师协会）为要在各种 UNIX 操作系统上运行软件，而定义 API 的一系列互相关联的标准的总称，其正式称呼为 IEEE Std 1003，而国际标准名称为 ISO/IEC 9945。此标准源于一个大约开始于1985 年的项目。POSIX 这个名称是由理查德·斯托曼（RMS）应 IEEE 的要求而提议的一个易于记忆的名称。它基本上是 Portable Operating System Interface（可移植操作系统接口）的缩写，而 X 则表明其对 Unix API 的传承。--来自百度百科。

## Pthreads
&emsp;Pthreads 一般指 POSIX 线程。 POSIX 线程（POSIX Threads，常被缩写为 Pthreads）是 POSIX 的线程标准，定义了创建和操纵线程的一套 API。
### Pthreads 简介
&emsp;实现 POSIX 线程标准的库常被称作 Pthreads，一般用于 Unix-like POSIX  系统，如 Linux、Solaris、macOS。但是 Microsoft Windows 上的实现也存在，例如直接使用 Windows API 实现的第三方库 pthreads-w32，而利用 Windows 的 SFU/SUA 子系统，则可以使用微软提供的一部分原生 POSIX API。Pthreads 是一套通用的多线程的 API，可以在 Unix / Linux / Windows 等系统跨平台使用，使用 C  语言编写，需要程序员自己管理线程的生命周期，这里我们对它常用的 API 学习一下，等到后面学习 GCD  源码的时候都会用到。

&emsp;Pthreads 定义了一套 C 语言的类型、函数与常量，它以 pthread.h 头文件和一个线程库实现。Pthreads API 中大致共有 100 个函数调用，全都以 "pthread_" 开头，并可以分为四类：（这里我们只关注第一类：线程管理，其它三类关于锁的部分可以参考前面锁的文章）

1. 线程管理，例如创建线程，等待（join）线程，查询线程状态等。
2. 互斥锁（Mutex）：创建、摧毁、锁定、解锁、设置属性等操作。
3. 条件变量（Condition Variable）：创建、摧毁、等待、通知、设置与查询属性等操作。
4. 使用了互斥锁的线程间的同步管理

### Pthreads 使用
&emsp;在 iOS 中使用 Pthreads API 首先要引入头文件: `#import <pthread.h>`。我们先看一个最简单的开启线程的例子：
```c++
- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.

    // 定义一个 pthread_t 类型变量，为指向线程的指针，
    // 它实际类型是 struct _opaque_pthread_t 指针。
    pthread_t thread = NULL;
    
    // 用于在主线程内接收子线程任务执行完成后的返回值。（如果没有返回值则可忽略）
    void* thread_ret = NULL;
    
    // 准备一个变量用于传递参数
    NSObject *objc = [[NSObject alloc] init];
    NSLog(@"objc: %p", objc);
    
    // (__bridge void *)：在 C 和 OC 之间传递数据，需要使用 __bridge 进行桥接，桥接的目的就是为了告诉编译器如何管理内存。
    // 也可使用 CFBridgingRetain：pthread_create(&thread, NULL, run, (void *)CFBridgingRetain(objc))
    // 但是后要跟 CFBridgingRelease((__bridge void *)objc)，要不然会导致 objc 内存泄漏。
    
    // 直接传递 objc 会提示如下错误，并给了我们两种解决方法:
    // Implicit conversion of Objective-C pointer type 'NSObject *' to C pointer type 'void *' requires a bridged cast
    // Use __bridge to convert directly (no change in ownership)
    // Use CFBridgingRetain call to make an ARC object available as a +1 'void *'
    
    // MRC 中不需要使用桥接，能直接使用 objc。
    int result = pthread_create(&thread, NULL, run, (__bridge void *)(objc));
    
    if (result == 0) {
        NSLog(@"创建线程成功 🎉🎉");
    } else {
        NSLog(@"创建线程失败 ❌，失败编号: %d", result);
    }
    
    // 线程分离
    // 设置子线程的状态设置为 detached，进行线程分离，该线程运行结束后系统会进行处理并释放线程的所有资源，
    // 或者在子线程中添加 pthread_detach(pthread_self())，其中 pthread_self() 是获得当前线程自身
    pthread_detach(thread);
    
    // 线程合并
    // 另外一种方式是进行线程合并 pthread_join，看到下面的 run 函数了吗，它是有一个通用的返回值的 void *，
    // 那么我们如何接收它的返回值呢？
    
    // pthread_join(thread, (void**)&thread_ret);
    // NSLog(@"thread_ret: %p", thread_ret);
    
    // 上面的线程分离和线程合并我们必须选择一种，否则会发生线程资源泄漏。具体的细节在下面进行分析。
    
    NSLog(@"🧑‍💻 %@", [NSThread currentThread]);
}

void* run(void *param) {
    // sleep(2);

    NSLog(@"🏃‍♀️ %@ param: %p", [NSThread currentThread], param);
    
    // return param;
    return NULL;
}

// 控制台打印:
objc: 0x6000039da810
创建线程成功 🎉🎉
🧑‍💻 <NSThread: 0x600002ec6980>{number = 1, name = main} // 主线程

// 本地打印看到 run 函数是最后执行的，如果把 objc 换成自己的类，并重写 dealloc 函数，会在这行多一条: 🍀🍀🍀 TEMP dealloc 打印
// 当 run 函数执行时，objc 已经释放了，再在其内部打印 objc 不是野指针访问吗...？？？??

// 注释 pthread_join 内容，打开 pthread_detach 内容，多次执行会发现 🏃‍♀️ 和 🧑‍💻 打印顺序是不定的，即子线程是异步执行的，不会阻塞主线程。

// 注释 pthread_detach，打开 pthread_join 内容。pthread_join(thread, (void**)&thread_ret) 来获取 run 函数返回值的话，🧑‍💻 会等到 🏃‍♀️ 执行完成以后才会执行。
// 即 pthread_join 会阻塞主线程的执行，直到 run 函数执行完毕并返回。

🏃‍♀️ <NSThread: 0x600002ea36c0>{number = 6, name = (null)} param: 0x6000039da810 // run 函数内部的开启了子线程，obj 也被传递过来了
```
&emsp;`pthread_detach` 线程分离时的不同的打印顺序。
```c++
2020-11-13 16:30:44.152497+0800 Simple_iOS[50694:9047896] objc: 0x600001558a40
2020-11-13 16:30:44.152932+0800 Simple_iOS[50694:9048038] 🏃‍♀️ <NSThread: 0x6000002d4fc0>{number = 6, name = (null)} param: 0x600001558a40
2020-11-13 16:30:44.153245+0800 Simple_iOS[50694:9047896] 创建线程成功 🎉🎉
2020-11-13 16:30:44.155304+0800 Simple_iOS[50694:9047896] 🧑‍💻 <NSThread: 0x60000025acc0>{number = 1, name = main}
```
```c++
2020-11-13 16:31:23.100374+0800 Simple_iOS[50700:9048791] objc: 0x600003ab4490
2020-11-13 16:31:23.100671+0800 Simple_iOS[50700:9048791] 创建线程成功 🎉🎉
2020-11-13 16:31:23.100921+0800 Simple_iOS[50700:9048791] 🧑‍💻 <NSThread: 0x600002da3040>{number = 1, name = main}
2020-11-13 16:31:23.100770+0800 Simple_iOS[50700:9048878] 🏃‍♀️ <NSThread: 0x600002df9580>{number = 6, name = (null)} param: 0x600003ab4490
```
> &emsp;这里看一个题外话，看源码时经常遇到命名后缀有 _t/ref 的类型，如 spinlock_t、weak_table_t、weak_entry_t 等，后缀 t 是用来表示 struct 的，这是由于 C 中没有类的定义，它是面向过程的语言，在 C 语言中表示类型时是用结构体 struct，所以在后面加一个 _t 只是为了标识类型。[iOS多线程中的实际方案之一pthread](https://www.jianshu.com/p/cfc6e7d2316a)

> &emsp;在混合开发时，如果在 C 和 OC 之间传递数据，需要使用 \_\_bridge 进行桥接，桥接的目的就是为了告诉编译器如何管理内存，MRC 中不需要使用桥接；在 OC 中，如果是 ARC 开发，编译器会在编译时，根据代码结构，自动添加 retain/release/autorelease。但是，ARC 只负责管理 OC 部分的内存管理，而不负责 C 语言 代码的内存管理。因此，开发过程中，如果使用的 C 语言框架出现 retain/create/copy/new 等字样的函数，大多都需要 release，否则会出现内存泄漏，如上面的 CFBridgingRetain 和 CFBridgingRelease 配对使用。[iOS多线程中的实际方案之一pthread](https://www.jianshu.com/p/cfc6e7d2316a)

### pthread_t 定义
&emsp;`pthread_t` 是一个指向线程的指针，在 iOS 它是: `__darwin_pthread_t`。下面看一下源码定义:
```c++
typedef __darwin_pthread_t pthread_t;

typedef struct _opaque_pthread_t *__darwin_pthread_t;

struct _opaque_pthread_t {
    long __sig;
    struct __darwin_pthread_handler_rec  *__cleanup_stack;
    char __opaque[__PTHREAD_SIZE__];
};

struct __darwin_pthread_handler_rec {
    void (*__routine)(void *);    // Routine to call 线程的入口函数，即需要在新线程中执行的任务
    void *__arg;            // Argument to pass __routine 函数的参数
    struct __darwin_pthread_handler_rec *__next; // 可以理解为链表下一个节点的指针
};
```
&emsp;通过上面的代码一层一层递进：`pthread_t` 其实是 `_opaque_pthread_t` 结构体指针。

### pthread_create 线程创建
&emsp;`pthread_create` 是类 Unix 操作系统（Unix、Linux、Mac OS X等）的创建线程的函数。它的功能是创建线程（实际上就是确定调用该线程函数的入口点），在线程创建以后，就开始运行相关的线程函数。
`pthread_create` 的返回值: 若成功，返回 0；若出错，返回出错编号，并且 `pthread_t * __restrict` 中的内容未定义。下面看一下它的函数声明：

```c++
__API_AVAILABLE(macos(10.4), ios(2.0))
#if !_PTHREAD_SWIFT_IMPORTER_NULLABILITY_COMPAT
int pthread_create(pthread_t _Nullable * _Nonnull __restrict,
        const pthread_attr_t * _Nullable __restrict,
        void * _Nullable (* _Nonnull)(void * _Nullable),
        void * _Nullable __restrict);
#else
// 兼容 Swift
int pthread_create(pthread_t * __restrict,
        const pthread_attr_t * _Nullable __restrict,
        void *(* _Nonnull)(void *), void * _Nullable __restrict);
#endif // _PTHREAD_SWIFT_IMPORTER_NULLABILITY_COMPAT
```
&emsp;当线程创建成功时，由 `pthread_t * __restrict` 指向的内存单元被设置为新创建的线程的内容，`const pthread_attr_t * __restrict` 用于指定线程属性。新创建的线程从 `void * (* _Nonnull)(void *)` 函数地址开始运行，该函数指针指向的正是线程开启后的回调函数的起始地址，最后面的 `void * _Nullable __restrict` 则是作为它的参数，如果需要的参数不止一个，那么可以传递对象来包含不同的属性值进行传递。这种设计可以在线程创建之前就帮它准备好一些专有数据，最典型的用法就是使用 C++ 编程时的 this 指针。

&emsp;四个参数可精简总结如下:
1. 第一个参数为指向线程的指针。当一个新的线程创建成功之后，就会通过这个参数将线程的句柄返回给调用者，以便对这个线程进行管理。
2. 第二个参数用来设置线程属性，可传递 `NULL`。
3. 第三个参数是线程运行函数的起始地址。（即需要在新线程中执行的任务。该函数有一个返回值 `void *`，这个返回值可以通过 `pthread_join()` 接口获得）
4. 第四个参数是运行函数的参数，可传递 `NULL`。

### pthread_join 线程合并
&emsp;上面创建线程的内容我们大致已经清晰了，这里再回到我们的实例代码，首先打开关于 `pthread_join` 的注释，并把 `pthread_detach` 添加注释。运行程序可得到如下打印:
```c++
2020-11-13 16:33:09.516744+0800 Simple_iOS[50716:9050696] objc: 0x600000202bd0
2020-11-13 16:33:09.517021+0800 Simple_iOS[50716:9050696] 创建线程成功 🎉🎉
2020-11-13 16:33:11.518575+0800 Simple_iOS[50716:9050922] 🏃‍♀️ <NSThread: 0x600001580000>{number = 7, name = (null)} param: 0x600000202bd0
2020-11-13 16:33:11.519041+0800 Simple_iOS[50716:9050696] thread_ret: 0x600000202bd0
2020-11-13 16:33:11.519384+0800 Simple_iOS[50716:9050696] 🧑‍💻 <NSThread: 0x600001500fc0>{number = 1, name = main}
```
&emsp;保留了执行时间，看到 🎉🎉 和 🏃‍♀️ 执行时间相差 2 秒，这个 2 秒是 `run` 函数内部的 `sleep(2)` 执行时间，然后最后才打印 🧑‍💻。  可以明确的是 `thread` 阻塞了我们的主线程，等 `thread` 线程内部的 `run` 函数/任务执行完毕返回后，主线程才得以继续执行。

&emsp;这里为了做出对比可以试着运行 `pthread_detach` 的内容，看打印结果与 `pthread_join` 的区别。调用 `pthread_join(thread, (void**)&thread_ret)` 会把我们自己手动创建的 `thread` 线程与主线程进行合并。`pthread_join` 的第一个参数是新创建的 `thread` 线程句柄，第二个参数会去接收 `thread` 线程的返回值。`pthread_join` 会阻塞主进程的执行，直到合并的线程执行结束。由于 `thread` 线程在结束之后会将 `param` 返回，那么 `pthread_join` 获得的线程返回值自然也就是我们最初创建的 `objc`，输出结果中对象地址完全相同也证实了这一点(看到一些文章把线程合并描述为线程等待，例如我们这里是主线程等待 `thread` 线程，逻辑理解上和线程合并是完全一致的，要说明的一点是，一个线程不能被多个线程等待（合并），否则第一个接收到信号的线程成功返回，其余调用 `pthread_join` 的线程则返回错误代码 `ESRCH`)。看一下 `pthread_join` 函数的声明:
```c++
__API_AVAILABLE(macos(10.4), ios(2.0))
int pthread_join(pthread_t , void * _Nullable * _Nullable)
        __DARWIN_ALIAS_C(pthread_join);
```
&emsp;`pthread_join` 函数干了什么？什么是线程合并呢？

&emsp;我们首先要明确的一个问题就是什么是线程的合并。从前面的叙述中我们已经了解到了，`pthread_create` 负责创建了一个线程。那么线程也属于系统的资源，这跟内存没什么两样，而且线程本身也要占据一定的内存空间。众所周知的一个问题就是 C 或 C++ 编程中如果要通过 `malloc()` 或 `new` 分配了一块内存，就必须使用 `free()`或 `delete` 来回收这块内存，否则就会产生著名的内存泄漏问题。既然线程和内存没什么两样，那么有创建就必须得有回收，否则就会产生另外一个著名的资源泄漏问题，这同样也是一个严重的问题。那么线程的合并就是回收线程资源了。

&emsp;线程的合并是一种主动回收线程资源的方案。当一个进程或线程调用了针对其它线程的 `pthread_join` 函数，就是线程合并了。这个接口会阻塞调用进程或线程，直到被合并的线程结束为止。当被合并线程结束，`pthread_join` 接口就会回收这个线程的资源，并将这个线程的返回值返回给合并者。与线程合并相对应的另外一种线程资源回收机制是线程分离，调用接口是 `pthread_detach`，下面我们对线程分离进行分析。
### pthread_detach 线程分离
&emsp;首先看一下 `pthread_detach` 函数声明:
```c++
__API_AVAILABLE(macos(10.4), ios(2.0))
int pthread_detach(pthread_t);
```
&emsp;线程分离是将线程资源的回收工作交由系统自动来完成，也就是说当被分离的线程结束之后，系统会自动回收它的资源。因为线程分离是启动系统的自动回收机制，那么程序也就无法获得被分离线程的返回值，这就使得 `pthread_detach` 接口只要拥有一个参数就行了，那就是被分离线程句柄。

&emsp;**线程合并和线程分离都是用于回收线程资源的，可以根据不同的业务场景酌情使用。不管有什么理由，你都必须选择其中一种，否则就会引发资源泄漏的问题，这个问题与内存泄漏同样可怕。**

### pthread_attr_t 线程属性
&emsp;前面调用 `pthread_create` 函数创建线程时，第二个参数是设置线程的属性我们直接传了 `NULL`。当需要设置线程属性时我们可以传入一个 `pthread_attr_t` 指针（`pthread_attr_t` 实际是 `_opaque_pthread_attr_t` 结构体的别名）。

&emsp;为了给线程设置不同的属性，POSIX 定义了一系列属性设置函数，我们可以使用 `pthread_attr_init` 接口初始化线程属性结构体，使用 `pthread_attr_destroy` 接口来销毁线程属性结构体，以及与各个属性相关的 `pthread_attr_get XXX` / `pthread_attr_set XXX` 函数。

&emsp;首先我们看一下 `pthread_attr_t` 定义。
```c++
typedef __darwin_pthread_attr_t pthread_attr_t;

typedef struct _opaque_pthread_attr_t __darwin_pthread_attr_t;

struct _opaque_pthread_attr_t {
    long __sig;
    char __opaque[__PTHREAD_ATTR_SIZE__];
};
```
&emsp;`pthread_attr_init` 函数声明:
```c++
__API_AVAILABLE(macos(10.4), ios(2.0))
int pthread_attr_init(pthread_attr_t *);
```
&emsp;`pthread_attr_destroy` 函数声明:
```c++
__API_AVAILABLE(macos(10.4), ios(2.0))
int pthread_attr_destroy(pthread_attr_t *);
```
&emsp;在设置线程属性  `pthread_attr_t` 之前，通常先调用 `pthread_attr_init` 来初始化，之后来调用相应的属性设置函数，在看具体的属性设置函数之前，我们先看一下线程都有哪些属性。

&emsp;那么线程都有哪些属性呢？在 pthread.h 文件的函数列表中，我们可看到在 apple 平台（iOS/macOS）下苹果遵循 POSIX 线程标准实现了线程的如下属性:
+ 分离属性
+ 绑定属性（scope 属性）
+ 满栈警戒区属性
+ 堆栈大小属性
+ 堆栈地址
+ 调度属性（包括算法、调度优先级、继承权）
&emsp;下面来分别详细介绍这些属性。

#### 分离属性
&emsp;首先在 pthread.h 文件中能看到两个与分离属性相关的设置和读取接口: `pthread_attr_setdetachstate`、`pthread_attr_getdetachstate`。
```c++
__API_AVAILABLE(macos(10.4), ios(2.0))
int pthread_attr_setdetachstate(pthread_attr_t *, int);

__API_AVAILABLE(macos(10.4), ios(2.0))
int pthread_attr_getdetachstate(const pthread_attr_t *, int *);
```
&emsp;前面说过线程能够被合并和分离，分离属性就是让线程在创建之前就决定它应该是分离的。如果设置了这个属性，就没有必要调用 `pthread_join` 或 `pthread_detach` 来回收线程资源了。`pthread_attr_setdetachstate` 函数的第二个参数有两个取值，`PTHREAD_CREATE_DETACHED`（分离的）和 `PTHREAD_CREATE_JOINABLE`（可合并，也是默认属性），它们是两个宏定义，定义在 pthread.h 文件顶部:
```c++
#define PTHREAD_CREATE_JOINABLE      1
#define PTHREAD_CREATE_DETACHED      2
```
&emsp;分离属性设置示例如下:
```c++
...
pthread_attr_t attr;
pthread_attr_init(&attr);
pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

int result = pthread_create(&thread, &attr, run, (__bridge void *)(objc));
...
```
#### 绑定属性（scope 属性）
&emsp;在 pthread.h 文件中与绑定属性（scope 属性）相关的 API 和宏定义如下:
```c++
__API_AVAILABLE(macos(10.4), ios(2.0))
int pthread_attr_setscope(pthread_attr_t *, int); // 设置

__API_AVAILABLE(macos(10.4), ios(2.0))
int pthread_attr_getscope(const pthread_attr_t * __restrict, int * __restrict); // 读取

/* We only support PTHREAD_SCOPE_SYSTEM */ // 在 iOS/macOS 下都不支持 PTHREAD_SCOPE_PROCESS
#define PTHREAD_SCOPE_SYSTEM         1 // 绑定
#define PTHREAD_SCOPE_PROCESS        2 // 非绑定
```

&emsp;由于目前该属性的理解有限，这边就先把 Linux 大佬的原文摘录贴出了。

> &emsp;说到这个绑定属性，就不得不提起另外一个概念：轻进程（ Light Weight Process，简称 LWP）。轻进程和 Linux 系统的内核线程拥有相同的概念，属于内核的调度实体。一个轻进程可以控制一个或多个线程。默认情况下，对于一个拥有 n 个线程的程序，启动多少轻进程，由哪些轻进程来控制哪些线程由操作系统来控制，这种状态被称为非绑定的。那么绑定的含义就很好理解了，只要指定了某个线程 “绑” 在某个轻进程上，就可以称之为绑定的了。被绑定的线程具有较高的响应速度，因为操作系统的调度主体是轻进程，绑定线程可以保证在需要的时候它总有一个轻进程可用。绑定属性就是干这个用的。
设置绑定属性的接口是 pthread_attr_setscope()，它的完整定义是：int pthread_attr_setscope(pthread_attr_t *attr, int scope);
它有两个参数，第一个就是线程属性对象的指针，第二个就是绑定类型，拥有两个取值：PTHREAD_SCOPE_SYSTEM（绑定的）和 PTHREAD_SCOPE_PROCESS（非绑定的）。
>  &emsp;不知道你是否在这里发现了本文的矛盾之处。就是这个绑定属性跟我们之前说的 NPTL 有矛盾之处。在介绍 NPTL 的时候就说过业界有一种 m:n 的线程方案，就跟这个绑定属性有关。但是笔者还说过 NPTL 因为 Linux 的 “蠢” 没有采取这种方案，而是采用了“1:1” 的方案。这也就是说，Linux 的线程永远都是绑定。对，Linux 的线程永远都是绑定的，所以 PTHREAD_SCOPE_PROCESS 在 Linux 中不管用，而且会返回 ENOTSUP 错误。
既然 Linux 并不支持线程的非绑定，为什么还要提供这个接口呢？答案就是兼容！因为 Linux 的 NTPL 是号称 POSIX 标准兼容的，而绑定属性正是 POSIX 标准所要求的，所以提供了这个接口。如果读者们只是在 Linux 下编写多线程程序，可以完全忽略这个属性。如果哪天你遇到了支持这种特性的系统，别忘了我曾经跟你说起过这玩意儿：）[在Linux中使用线程](https://blog.csdn.net/jiajun2001/article/details/12624923)

> &emsp;设置线程 __scope 属性。scope 属性表示线程间竞争 CPU 的范围，也就是说线程优先级的有效范围。POSIX 的标准中定义了两个值： PTHREAD_SCOPE_SYSTEM 和 PTHREAD_SCOPE_PROCESS ，前者表示与系统中所有线程一起竞争 CPU 时间，后者表示仅与同进程中的线程竞争 CPU。默认为 PTHREAD_SCOPE_PROCESS。 目前 Linux Threads 仅实现了 PTHREAD_SCOPE_SYSTEM 一值。[线程属性pthread_attr_t简介](https://blog.csdn.net/hudashi/article/details/7709413)

#### 堆栈大小属性
&emsp;在 pthread.h 文件中与堆栈大小属性相关的 API 如下:
```c++
__API_AVAILABLE(macos(10.4), ios(2.0))
int pthread_attr_setstacksize(pthread_attr_t *, size_t);

__API_AVAILABLE(macos(10.4), ios(2.0))
int pthread_attr_getstacksize(const pthread_attr_t * __restrict, size_t * __restrict);
```
&emsp;从前面的这些例子中可以了解到，子线程的主函数与主线程的 `viewDidLoad` 函数有一个很相似的特性，那就是可以拥有局部变量。虽然同一个进程的线程之间是共享内存空间的，但是它的局部变量确并不共享。原因就是局部变量存储在堆栈中，而不同的线程拥有不同的堆栈。Linux 系统为每个线程默认分配了 8 MB 的堆栈空间，如果觉得这个空间不够用，可以通过修改线程的堆栈大小属性进行扩容。修改线程堆栈大小属性的接口是 `pthread_attr_setstacksize`，它的第二个参数就是堆栈大小了，以字节为单位。需要注意的是，线程堆栈不能小于 16 KB，而且尽量按 4 KB（32 位系统）或 2 MB（64 位系统）的整数倍分配，也就是内存页面大小的整数倍。此外，修改线程堆栈大小是有风险的，如果你不清楚你在做什么，最好别动它。

#### 满栈警戒区属性
&emsp;在 pthread.h 文件中与满栈警戒区属性相关的 API 如下:
```c++
__API_AVAILABLE(macos(10.4), ios(2.0))
int pthread_attr_setguardsize(pthread_attr_t *, size_t);

__API_AVAILABLE(macos(10.4), ios(2.0))
int pthread_attr_getguardsize(const pthread_attr_t * __restrict, size_t * __restrict);
```
&emsp;既然线程是有堆栈的，而且还有大小限制，那么就一定会出现将堆栈用满的情况。线程的堆栈用满是非常危险的事情，因为这可能会导致对内核空间的破坏，一旦被有心人士所利用，后果也不堪设想。为了防治这类事情的发生，Linux 为线程堆栈设置了一个满栈警戒区。这个区域一般就是一个页面，属于线程堆栈的一个扩展区域。一旦有代码访问了这个区域，就会发出 SIGSEGV 信号进行通知。

&emsp;虽然满栈警戒区可以起到安全作用，但是也有弊病，就是会白白浪费掉内存空间，对于内存紧张的系统会使系统变得很慢。所有就有了关闭这个警戒区的需求。同时，如果我们修改了线程堆栈的大小，那么系统会认为我们会自己管理堆栈，也会将警戒区取消掉，如果有需要就要开启它。
修改满栈警戒区属性的接口是 `pthread_attr_setguardsize`，它的第二个参数就是警戒区大小了，以字节为单位。与设置线程堆栈大小属性相仿，应该尽量按照 4KB 或 2MB 的整数倍来分配。当设置警戒区大小为 0 时，就关闭了这个警戒区。虽然栈满警戒区需要浪费掉一点内存，但是能够极大的提高安全性，所以这点损失是值得的。而且一旦修改了线程堆栈的大小，一定要记得同时设置这个警戒区。

#### 调度属性（包括算法、调度优先级、继承权）
&emsp;在 pthread.h/pthread_impl.h 文件中与调度属性（包括算法、调度优先级、继承权）相关的 API 和宏定义 如下:
```c++
__API_AVAILABLE(macos(10.4), ios(2.0))
int pthread_attr_setschedpolicy(pthread_attr_t *, int); // 设置调度算法（策略）

__API_AVAILABLE(macos(10.4), ios(2.0))
int pthread_attr_getschedpolicy(const pthread_attr_t * __restrict, int * __restrict); // 读取调度算法（策略）

// 三种策略
#define SCHED_OTHER                1 // 其它
#define SCHED_FIFO                 4 // 先进先出
#define SCHED_RR                   2 // 轮询
```
```c++
#ifndef __POSIX_LIB__
struct sched_param { int sched_priority;  char __opaque[__SCHED_PARAM_SIZE__]; };
#endif

__API_AVAILABLE(macos(10.4), ios(2.0))
int pthread_attr_setschedparam(pthread_attr_t * __restrict,
        const struct sched_param * __restrict); // 设置线程优先级

__API_AVAILABLE(macos(10.4), ios(2.0))
int pthread_attr_getschedparam(const pthread_attr_t * __restrict,
        struct sched_param * __restrict); // 读取线程优先级
```
```c++
__API_AVAILABLE(macos(10.4), ios(2.0))
int pthread_attr_setinheritsched(pthread_attr_t *, int); // 设置继承权

__API_AVAILABLE(macos(10.4), ios(2.0))
int pthread_attr_getinheritsched(const pthread_attr_t * __restrict, int * __restrict); // 读取继承权

#define PTHREAD_INHERIT_SCHED        1 // 拥有继承权
#define PTHREAD_EXPLICIT_SCHED       2 // 放弃继承权
```
&emsp;线程的调度属性有三个，分别是：算法、优先级和继承权。

&emsp;Linux 提供的线程调度算法有三个：轮询、先进先出和其它。其中轮询和先进先出调度算法是 POSIX 标准所规定，而其他则代表采用 Linux 自己认为更合适的调度算法，所以默认的调度算法也就是其它了。轮询和先进先出调度算法都属于实时调度算法。轮询指的是时间片轮转，当线程的时间片用完，系统将重新分配时间片，并将它放置在**就绪队列尾部**，这样可以保证具有相同优先级的轮询任务获得公平的 CPU 占用时间；先进先出就是先到先服务，一旦线程占用了 CPU 则一直运行，直到有更高优先级的线程出现或自己放弃。

&emsp;设置线程调度算法的接口是 `pthread_attr_setschedpolicy`，它的第二个参数有三个取值：`SCHED_RR`（轮询）、`SCHED_FIFO`（先进先出）和 `SCHED_OTHER`（其它）。
Linux 的**线程优先级**与**进程的优先级**不一样，进程优先级前面一篇有解读。Linux 的线程优先级是从 1 到 99 的数值，数值越大代表优先级越高。而且要注意的是，只有采用 `SHCED_RR` 或 `SCHED_FIFO` 调度算法时，优先级才有效。对于采用 `SCHED_OTHER` 调度算法的线程，其优先级恒为 `0`。

&emsp;设置线程优先级的接口是 `pthread_attr_setschedparam`，`sched_param` 结构体的 `sched_priority` 字段就是线程的优先级了。

&emsp;此外，即便采用 `SCHED_RR` 或 `SCHED_FIFO` 调度算法，线程优先级也不是随便就能设置的。首先，进程必须是以 `root` 账号运行的；其次，还需要放弃线程的继承权。什么是继承权呢？就是当创建新的线程时，新线程要继承父线程（创建者线程）的调度属性。如果不希望新线程继承父线程的调度属性，就要放弃继承权。

&emsp;设置线程继承权的接口是 `pthread_attr_setinheritsched`，它的第二个参数有两个取值：`PTHREAD_INHERIT_SCHED`（拥有继承权）和 `PTHREAD_EXPLICIT_SCHED`（放弃继承权），新线程在默认情况下是拥有继承权。

&emsp;好了，线程属性先介绍到这里，由于没有找到 iOS/macOS 的材料，这里借用了 Linux 下 POSIX 线程标准，尽管平台不同，但是基本理解和处理方式都是相同的，所以并不妨碍我们对线程属性进行理解和学习。下面我们继续学习 pthread.h 文件中的其它接口。

### pthread_kill 向指定线程发送一个信号
&emsp;`pthread_kill` 用于向指定的 thread 发送信号。在创建的线程中使用 signal(SIGKILL, sig_handler) 处理信号，如果给一个线程发送了 SIGQUIT，但线程却没有实现 signal 处理函数，则整个进程退出。函数声明如下:
```c++
__API_AVAILABLE(macos(10.4), ios(2.0))
int pthread_kill(pthread_t, int);
```
&emsp;参数一是指定的要向它发送信号的线程，参数二表示传递的 signal 参数，一般都是大于 0 的，这时系统默认或者自定义的都是有相应的处理程序。常用信号量宏可以在 #import <signal.h> 查看，signal 为 0 时，是一个被保留的信号，一般用这个保留的信号测试线程是否存在。

&emsp;`pthread_kill` 函数返回值：
+ 0: 调用成功
+ ESRCH: 线程不存在
+ EINVAL: 信号不合法
+ 测试线程是否存在/终止的方法

&emsp;如果线程内不对信号进行处理，则调用默认的处理程式，如 SIGQUIT 信号会退出终止线程，SIGKILL会杀死线程等等。可以调用 `signal(SIGQUIT, sig_process_routine)` 来自定义信号的处理程序。

### pthread_cancel 中断指定线程的运行
&emsp;`pthread_cancel` 发送终止信号给指定的 thread 线程，如果成功则返回 0，否则为非 0 值，发送成功并不意味着 thread 会终止。函数声明如下:
```c++
__API_AVAILABLE(macos(10.4), ios(2.0))
int pthread_cancel(pthread_t) __DARWIN_ALIAS(pthread_cancel);
```
&emsp;若是在整个程序退出时，要终止各个线程，应该在成功发送 CANCEL 指令后，使用 pthread_join 函数，等待指定的线程已经完全退出以后，再继续执行，否则，很容易产生 “段错误”。

### pthread_setcancelstate 设置本线程对 Cancel 信号的反应
```c++
#define PTHREAD_CANCEL_ENABLE        0x01  /* Cancel takes place at next cancellation point // Cancel 发生在下一个取消点 */ 
#define PTHREAD_CANCEL_DISABLE       0x00  /* Cancel postponed // Cancel 推迟 */

__API_AVAILABLE(macos(10.4), ios(2.0))
int pthread_setcancelstate(int state, int * _Nullable oldstate)
        __DARWIN_ALIAS(pthread_setcancelstate);
```
&emsp;设置本线程对 Cancel 信号的反应，state 有两种值：`PTHREAD_CANCEL_ENABLE` 和 `PTHREAD_CANCEL_DISABLE`，分别表示收到信号后设为 CANCLED 状态和忽略 CANCEL 信号继续运行；old_state 如果不为 NULL 则存入原来的 Cancel 状态以便恢复。

&emsp;`PTHREAD_CANCEL_ENABLE`：表示可以接收处理取消信号，设置线程状态为 CANCEl，并终止任务执行。

&emsp;`PTHREAD_CANCEL_DISABLE`：忽略取消信号，继续执行任务。
### pthread_setcanceltype 设置本线程取消动作的执行时机
```c++
#define PTHREAD_CANCEL_DEFERRED      0x02  /* Cancel waits until cancellation point // Cancel 等待直到取消点 */
#define PTHREAD_CANCEL_ASYNCHRONOUS  0x00  /* Cancel occurs immediately // Cancel 立即发生*/

__API_AVAILABLE(macos(10.4), ios(2.0))
int pthread_setcanceltype(int type, int * _Nullable oldtype)
        __DARWIN_ALIAS(pthread_setcanceltype);
```
&emsp;设置本线程取消动作的执行时机，type 有两种取值：`PTHREAD_CANCEL_DEFERRED` 和 `PTHREAD_CANCEL_ASYNCHRONOUS`，仅当 Cancel 状态为 Enable 时有效，分别表示收到信号后继续运行至下一个取消点再退出 （推荐做法，因为在终止线程之前必须要处理好内存回收防止内存泄漏，而手动设置取消点这种方式就可以让我们很自由的处理内存回收时机）和立即执行取消动作（退出）（不推荐这样操作,可能造成内存泄漏等问题）；oldtype 如果不为 NULL 则存入原来的取消动作类型值。

&emsp;此函数应该在线程开始时执行，若线程内部有任何资源申请等操作，应该选择 `PTHREAD_CANCEL_DEFERRED` 的设定，然后在退出点（`pthread_testcancel` 用于定义退出点）进行线程退出。

### pthread_testcancel 
```c++
__API_AVAILABLE(macos(10.4), ios(2.0))
void pthread_testcancel(void) __DARWIN_ALIAS(pthread_testcancel);
```
&emsp;检查本线程是否处于 Canceld 状态，如果是，则进行取消动作，否则直接返回。 此函数在线程内执行，执行的位置就是线程退出的位置，在执行此函数以前，线程内部的相关资源申请一定要释放掉，否则很容易造成内存泄露。

&emsp;线程取消的方法是向目标线程发 Cancel 信号，但如何处理 Cancel 信号则由目标线程自己决定，或者忽略、或者立即终止、或者继续运行至 Cancelation-point（取消点），由不同的 Cancelation 状态决定。

&emsp;线程接收到 CANCEL 信号的缺省处理（即 `pthread_create` 创建线程的缺省状态）是继续运行至取消点，也就是说设置一个 CANCELED 状态，线程继续运行，只有运行至 Cancelation-point 的时候才会退出。

### pthread_self 获取当前线程本身
```c++
__API_AVAILABLE(macos(10.4), ios(2.0))
pthread_t pthread_self(void);
```
&emsp;在线程内部获取当前线程本身。如 `[NSThread currentThread]`。
### pthread_equal 比较两个线程是否相等
```c++
__API_AVAILABLE(macos(10.4), ios(2.0))
int pthread_equal(pthread_t _Nullable, pthread_t _Nullable);
```
&emsp;对两个线程进行对比。
### pthread_exit 终止当前进程
```c++
__API_AVAILABLE(macos(10.4), ios(2.0))
void pthread_exit(void * _Nullable) __dead2;
```
&emsp;线程通过调用 `pthread_exit` 函数终止执行，就如同进程在结束时调用 `exit` 函数一样。这个函数的作用是，终止调用它的线程并返回一个指向某个对象的指针。
### 线程本地存储
&emsp;在学习自动释放池对函数返回值优化、空自动释放池占位等等知识点时，我们遇到过线程本地存储的概念（Thread Local Storage）。

&emsp;同一进程内线程之间可以共享内存地址空间，线程之间的数据交换可以非常快捷，这是线程最显著的优点。但是多个线程访问共享数据，需要昂贵的同步开销（加锁），也容易造成与同步相关的 BUG，更麻烦的是有些数据根本就不希望被共享，这又是缺点。

&emsp;C 程序库中的 errno 是个最典型的一个例子。errno 是一个全局变量，会保存最后一个系统调用的错误代码。在单线程环境并不会出现什么问题。但是在多线程环境，由于所有线程都会有可能修改 errno，这就很难确定 errno 代表的到底是哪个系统调用的错误代码了。这就是有名的 “非线程安全（Non Thread-Safe）” 的。

&emsp;此外，从现代技术角度看，在很多时候使用多线程的目的并不是为了对共享数据进行并行处理。更多是由于多核心 CPU 技术的引入，为了充分利用 CPU 资源而进行并行运算（不互相干扰）。换句话说，大多数情况下每个线程只会关心自己的数据而不需要与别人同步。

&emsp;为了解决这些问题，可以有很多种方案。比如使用不同名称的全局变量。但是像 errno 这种名称已经固定了的全局变量就没办法了。在前面的内容中提到在线程堆栈中分配局部变量是不在线程间共享的。但是它有一个弊病，就是线程内部的其它函数很难访问到。目前解决这个问题的简便易行的方案是线程本地存储，即 Thread Local Storage，简称 TLS。利用 TLS，errno 所反映的就是本线程内最后一个系统调用的错误代码了，也就是线程安全的了。
Linux（iOS/macOS）提供了对 TLS 的完整支持，通过下面这些接口来实现：
```c++
typedef __darwin_pthread_key_t pthread_key_t;
typedef unsigned long __darwin_pthread_key_t;

__API_AVAILABLE(macos(10.4), ios(2.0))
int pthread_key_create(pthread_key_t *, void (* _Nullable)(void *));

__API_AVAILABLE(macos(10.4), ios(2.0))
int pthread_key_delete(pthread_key_t);

__API_AVAILABLE(macos(10.4), ios(2.0))
int pthread_setspecific(pthread_key_t , const void * _Nullable);

__API_AVAILABLE(macos(10.4), ios(2.0))
void* _Nullable pthread_getspecific(pthread_key_t);
```
&emsp;`pthread_key_create` 接口用于创建一个线程本地存储区。第一个参数用来返回这个存储区的句柄，需要使用一个全局变量保存，以便所有线程都能访问到。第二个参数是线程本地数据的回收函数指针，如果希望自己控制线程本地数据的生命周期，这个参数可以传递 NULL。

&emsp;`pthread_key_delete` 接口用于回收线程本地存储区。其唯一的参数就要回收的存储区的句柄。

&emsp;`pthread_getspecific` 和 `pthread_setspecific` 这两个接口分别用于获取和设置线程本地存储区的数据。这两个接口在不同的线程下会有不同的结果不同（相同的线程下就会有相同的结果），这也就是线程本地存储的关键所在。
## NSThread
&emsp;`NSThread` 是一个继承自 `NSObject` 并用来管理和操作线程的类。学习完 `Pthreads`，再来看 `NSThread` 真是倍感亲切呀，貌似现在看到 `NS` 前缀的类都会倍感亲切，它们几乎都是继承自 `NSObject` 的类，并且在 `ARC` 的加持下最后的释放销毁都由编译器为我们做了，我们尽管创建使用就好了，再加上它们几乎同一的使用逻辑也使我们易学习易上手易使用。下面我们正式进入 `NSThread` 的学习。

&emsp;一个 `NSThread` 对象会对应一个线程，与 `Pthreads` 相比，它以更加面向对象的方式来操作和管理线程，尽管还是需要我们自己手动管理线程的生命周期，但是此时仅限于创建，我们这里可以把创建线程的过程理解为创建 `NSThread` 对象，至于最后任务执行结束，线程资源的回收，系统都会帮我们处理，所以相比 GCD 来说还不是最易用的，GCD 的使用过程中，我们可以完全不考虑线程的创建和销毁，GCD 的使用和源码部分留在下篇，那么我们首先来看下 `NSThread` 的使用吧！⛽️⛽️

### NSThread 创建和启动线程
&emsp;NSThread.h 文件中列出了所有 NSThread 创建和启动线程的方式，并且提供了以 selector 或者 block 的形式在线程中执行函数（或者说是任务），同时在文件底部还提供了一个 `NSObject` 的 `NSThreadPerformAdditions` 分类，它列举了一组实例方法，分别在主线程、后台线程或是指定线程中执行函数（或说是任务），即我们可以使用任何继承自 `NSObject` 的类的对象来使用这些与多线程相关的 API，大大方便了我们在开发中使用多线程来执行任务。下面看一下 NSThread 的使用示例：

+ 使用 NSThread 的 `alloc` 和 `init` 方法显式创建 NSThread 对象（创建线程），然后调用 `start` 函数启动线程。
```c++
- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.

    // 创建一个对象作为线程函数的参数，看到 run 函数中打印对象地址是完全一致的
    NSObject *objc = [[NSObject alloc] init];
    NSLog(@"objc: %p", objc);
    
    // 1. 创建 NSThread 对象（创建线程）
    NSThread *thread = [[NSThread alloc] initWithTarget:self selector:@selector(run:) object:objc];
    
    // 设置线程名
    [thread setName:@"thread"];
    // 设置优先级，优先级从 0 到 1，1 最高
    [thread setThreadPriority:0.9];
    
    // 2. 启动线程
    [thread start];
    //（不会阻塞当前线程，不用等 run 函数执行完成才执行接下来的指令）
    
    // 1. 创建 NSThread 对象（创建线程），线程执行的任务以 block 的形式，代码更加紧凑
    NSThread *thread2 = [[NSThread alloc] initWithBlock:^{
        sleep(2);
        NSLog(@"🙆‍♂️ %@", [NSThread currentThread]);
    }];
    // 2. 启动线程
    [thread2 start];
    //（不会阻塞当前线程，不用等 block 执行完成才执行接下来的指令）
    
    NSLog(@"🧑‍💻 %@", [NSThread currentThread]);
}

- (void)run:(NSObject *)param {
    sleep(3);
    NSLog(@"🏃‍♀️ %@ param: %p", [NSThread currentThread], param);
}

// 打印:
2020-11-15 11:35:30.178059+0800 Simple_iOS[57531:9389410] objc: 0x6000024e42e0
2020-11-15 11:35:30.178355+0800 Simple_iOS[57531:9389410] 🧑‍💻 <NSThread: 0x6000033ea7c0>{number = 1, name = main} ⬅️ NSThread 是主线程
2020-11-15 11:35:32.183285+0800 Simple_iOS[57531:9389593] 🙆‍♂️ <NSThread: 0x6000033ba900>{number = 8, name = (null)} ⬅️ NSThread 是子线程，看到打印时间和 🧑‍💻 相差 2 秒
2020-11-15 11:35:33.180177+0800 Simple_iOS[57531:9389592] 🏃‍♀️ <NSThread: 0x6000033bba80>{number = 7, name = (null)} param: 0x6000024e42e0 ⬅️ NSThread 是子线程，看到是 3 秒后的打印
```
+ 使用 NSThread 的类方法显式的创建线程并会立刻自动启动线程（对比上面不需要再调用 `start` 函数）。
```c++
- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    // 创建一个对象作为线程函数的参数，看到 run 函数中打印对象地址是完全一致的
    NSObject *objc = [[TEMP alloc] init];
    NSLog(@"objc: %p", objc);
    
    // 函数名以 detach 开头，可联想到上面 Pthreads 中的线程分离。
    
    // 线程执行的任务以 selector 形式
    [NSThread detachNewThreadSelector:@selector(run:) toTarget:self withObject:objc];
    //（不会阻塞当前线程，不用等 run 函数执行完成才执行接下来的指令）
    
    // 线程执行的任务以 block 形式，代码更加紧凑
    [NSThread detachNewThreadWithBlock:^{
        sleep(2);
        NSLog(@"🙆‍♂️ %@", [NSThread currentThread]);
    }];
    //（不会阻塞当前线程，不用等 block 执行完成才执行接下来的指令）
    
    NSLog(@"🧑‍💻 %@", [NSThread currentThread]);
}

- (void)run:(NSObject *)param {
    sleep(3);
    NSLog(@"🏃‍♀️ %@ param: %p", [NSThread currentThread], param);
}

// 打印:
2020-11-15 11:47:15.823097+0800 Simple_iOS[57662:9395974] objc: 0x600003904690
2020-11-15 11:47:15.823579+0800 Simple_iOS[57662:9395974] 🧑‍💻 <NSThread: 0x600002e0cb40>{number = 1, name = main} ⬅️ NSThread 是主线程
2020-11-15 11:47:17.825466+0800 Simple_iOS[57662:9396067] 🙆‍♂️ <NSThread: 0x600002eb2140>{number = 9, name = (null)} ⬅️ NSThread 是子线程，看到打印时间和 🧑‍💻 相差 2 秒
2020-11-15 11:47:18.828209+0800 Simple_iOS[57662:9396066] 🏃‍♀️ <NSThread: 0x600002e0ce40>{number = 7, name = (null)} param: 0x600003904690 ⬅️ NSThread 是子线程，看到是 3 秒后的打印
2020-11-15 11:47:18.828532+0800 Simple_iOS[57662:9396066] 🍀🍀🍀 TEMP dealloc ⬅️ 这里传递了一个自己的类的对象，并重写了 dealloc 函数，看到 viewDidLoad 函数中的 TEMP 临时对象的销毁延迟到了 run 函数执行完毕
```
+ 隐式创建并启动线程。
```c++
- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    // 创建一个对象作为线程函数的参数，看到 run 函数中打印对象地址是完全一致的
    NSObject *objc = [[TEMP alloc] init];
    NSLog(@"objc: %p", objc);
    
    [self performSelectorInBackground:@selector(run:) withObject:objc];
    //（不会阻塞当前线程，不用等 run 函数执行完成才执行接下来的指令）
    
    NSLog(@"🧑‍💻 %@", [NSThread currentThread]);
}

- (void)run:(NSObject *)param {
    sleep(3);
    NSLog(@"🏃‍♀️ %@ param: %p", [NSThread currentThread], param);
}

// 打印:
2020-11-15 12:12:34.071785+0800 Simple_iOS[57919:9413855] objc: 0x6000036648f0
2020-11-15 12:12:34.072102+0800 Simple_iOS[57919:9413855] 🧑‍💻 <NSThread: 0x60000217d040>{number = 1, name = main} ⬅️ NSThread 是主线程
2020-11-15 12:12:37.431160+0800 Simple_iOS[57919:9414023] 🏃‍♀️ <NSThread: 0x6000021132c0>{number = 6, name = (null)} param: 0x6000036648f0 ⬅️ NSThread 是子线程，看到是 3 秒后的打印
2020-11-15 12:12:37.431515+0800 Simple_iOS[57919:9414023] 🍀🍀🍀 TEMP dealloc ⬅️ 这里传递了一个自己的类的对象，并重写了 dealloc 函数，看到 viewDidLoad 函数中的 TEMP 临时对象的销毁延迟到了 run 函数执行完毕
```
&emsp;以上就是 NSThread 创建线程的方式，只有使用 NSThread 的 `alloc` 和 `init` 方法创建的线程才会返回具体的线程实例，也就是说如果想要对线程做更多的控制，比如添加线程的名字、更改优先级等操作，要使用第一种方式来创建线程，但是此种方法需要使用 `start` 方法来手动启动线程。

### NSThread.h 文件 API 分析
&emsp;下面对 NSThread.h 文件中的属性或者函数进行分析。
#### currentThread
```c++
@property (class, readonly, strong) NSThread *currentThread;
```
&emsp;这里看到 `class` 真的被震惊到了，这个....这个....，这是类属性吗？做了这么多年 iOS 开发，第一次关注到这个属性修饰符里面竟然可以添加一个 `class` ，而且这个 `[NSThread currentThread]` 几乎每天都在使用的判断当前线程的方式，却从没关注过它是一个类属性，一直以为它是一个类函数，哭了。

&emsp;返回值代表当前执行线程的线程对象。(返回值表示当前执行线程的线程对象。)
#### isMultiThreaded
```c++
+ (BOOL)isMultiThreaded;
```
&emsp;这个函数第一次遇到，它针对的是当前的应用程序，返回应用程序是否为多线程。在 main.m 文件的 `main` 函数中会返回 0，如果使用 `pthread_create` 创建一个线程则会返回 1，在 `viewDidLoad` 函数中会直接返回 1。

&emsp;返回值如果应用程序是多线程，则为 YES，否则为 NO。

&emsp;官方文档的解释是：如果使用 `detachNewThreadSelector:toTarget:withObject:` 或 `start` 从主线程分离了线程，则认为该应用程序是多线程的。如果使用非 Cocoa API（例如 POSIX 或 Multiprocessing Services API ）在应用程序中分离了线程，则此方法仍可能返回 NO。分离的线程不必当前正在运行，应用程序才被认为是多线程的--此方法仅指示是否已产生单个线程。

#### threadDictionary
```c++
@property (readonly, retain) NSMutableDictionary *threadDictionary;

// 使用示例
// 在上面示例代码的 run 函数中添加如下代码:

NSLog(@"🏃‍♀️ %@", [[NSThread currentThread] threadDictionary]);

NSMutableDictionary *dic = [[NSThread currentThread] threadDictionary];
[dic setObject:param forKey:@"KEY"];

NSLog(@"🏃‍♀️ %@", [[NSThread currentThread] threadDictionary]);

// 控制台打印:
2020-11-15 15:12:58.245199+0800 Simple_iOS[59662:9494051] 🏃‍♀️ <NSThread: 0x600001c244c0>{number = 7, name = (null)} param: 0x600000baaf90
2020-11-15 15:12:58.245562+0800 Simple_iOS[59662:9494051] 🏃‍♀️ {
}
2020-11-15 15:12:58.245928+0800 Simple_iOS[59662:9494051] 🏃‍♀️ {
    KEY = "<TEMP: 0x600000baaf90>"; ⬅️ 把 param 保存在了线程的 threadDictionary 中
}
```
&emsp;取得一个线程的只读的 threadDictionary，取得以后我们可以往里面保存自己的内容。

&emsp;你可以使用返回的字典来存储特定于线程的数据。在对 NSThread 对象进行任何操作期​​间都不会使用线程字典，它只是一个可以存储任何有趣数据的地方。例如，Foundation 使用它来存储线程的默认 NSConnection 和 NSAssertionHandler 实例。你可以为字典定义自己的键。
#### sleepUntilDate:
```c++
+ (void)sleepUntilDate:(NSDate *)date;
```
&emsp;阻塞当前线程直到指定的时间，参数 date 也表示了线程恢复处理的时间。 线程被阻塞时，不会发生 runloop 处理。
#### sleepForTimeInterval:
```c++
+ (void)sleepForTimeInterval:(NSTimeInterval)ti;
```
&emsp;在给定的时间间隔内休眠线程。
#### exit
```c++
+ (void)exit;
```
&emsp;终止当前线程。

&emsp;此方法使用于 currentThread 类方法访问的当前线程。在退出线程之前，此方法将 `NSThreadWillExitNotification` 与退出线程发送到默认通知中心。因为通知是同步传递的，所以保证 `NSThreadWillExitNotification` 的所有观察者都可以在线程退出之前接收到通知。应该避免调用此方法，因为它不会使线程有机会清理在执行过程中分配的任何资源。
#### threadPriority
```c++
+ (double)threadPriority;
```
&emsp;返回当前线程的优先级。返回值表示当前线程的优先级，由 0.0 到 1.0 之间的浮点数指定，其中 1.0 是最高优先级。

&emsp;此范围内的优先级映射到操作系统的优先级值。 “typical” 线程优先级可能是 0.5，但是由于优先级是由内核确定的，因此不能保证此值实际上是多少。
#### setThreadPriority:
```c++
+ (BOOL)setThreadPriority:(double)p;
```
&emsp;设置当前线程的优先级。如果优先级分配成功，则为 YES，否则为 NO。p 使用从 0.0 到 1.0 的浮点数指定的新优先级，其中1.0是最高优先级。
#### threadPriority
```c++
@property double threadPriority API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0)); // To be deprecated; use qualityOfService below
```
&emsp;接收者的优先级。线程的优先级，由 0.0 到 1.0 之间的浮点数指定，其中 1.0 是最高优先级。此范围内的优先级映射到操作系统的优先级值。 “typical” 线程优先级可能是 0.5，但是由于优先级是由内核确定的，因此不能保证此值实际上是多少。
#### qualityOfService
```c++
@property NSQualityOfService qualityOfService API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0)); // read-only after the thread is started
```
#### callStackReturnAddresses
```c++
@property (class, readonly, copy) NSArray<NSNumber *> *callStackReturnAddresses API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
```
&emsp;返回一个包含调用堆栈返回地址的数组。包含调用堆栈返回地址的数组。每个元素都是一个包含 NSUInteger 值的NSNumber对象。
#### callStackSymbols
```c++
@property (class, readonly, copy) NSArray<NSString *> *callStackSymbols API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
```
&emsp;返回一个包含调用堆栈符号的数组。返回值是包含调用堆栈符号的数组。每个元素都是一个 NSString 对象，其值的格式由 `backtrace_symbols()` 函数确定。有关更多信息，参见 backtrace_symbols(3) macOS 开发人员工具手册页。

&emsp;返回值描述了在调用此方法时当前线程的调用堆栈回溯。
#### name
```c++
@property (nullable, copy) NSString *name API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
```
&emsp;线程的名字。
#### stackSize
```c++
@property NSUInteger stackSize API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
```
&emsp;线程的堆栈大小，以字节为单位。该值必须以字节为单位，并且为 4KB 的倍数。若要更改堆栈大小，必须在启动线程之前设置此属性。在线程启动后设置堆栈大小会更改属性大小（这由 `stackSize` 方法反映），但不会影响为线程预留的实际页面数。
#### isMainThread
```c++
@property (readonly) BOOL isMainThread API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
```
&emsp;一个布尔值，指示接收方是否为主线程。如果接收方是主线程则为 YES，否则为 NO。
#### isMainThread
```c++
@property (class, readonly) BOOL isMainThread API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0)); // reports whether current thread is main
```
&emsp;返回一个布尔值，该值指示当前线程是否为主线程。如果当前线程是主线程，则为YES，否则为NO。
#### mainThread
```c++
@property (class, readonly, strong) NSThread *mainThread API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
```
&emsp;返回代表主线程的 NSThread 对象。
#### executing
```c++
@property (readonly, getter=isExecuting) BOOL executing API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
```
&emsp;一个布尔值，指示接收者（线程）是否正在执行。如果接收者正在执行，则为 YES，否则为 NO。
#### finished
```c++
@property (readonly, getter=isFinished) BOOL finished API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
```
&emsp;一个布尔值，指示接收方是否已完成执行。如果接收方完成执行，则为 YES，否则为 NO。
#### cancelled
```c++
@property (readonly, getter=isCancelled) BOOL cancelled API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
```
&emsp;一个布尔值，指示接收者是否取消。如果已取消接收器，则为 YES，否则为 NO。如果你的线程支持取消，则它应定期检查此属性，并在返回 YES 时退出。
#### cancel
```c++
- (void)cancel API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
```
&emsp;更改接收器的取消状态以指示它应该退出（标识为取消状态，并不是执行取消操作）。

&emsp;此方法的语义与用于 NSOperation 的语义相同。此方法在接收器中设置状态信息，然后由 canceled 属性反映出来。支持取消的线程应定期调用 canceled 方法以确定该线程实际上是否已取消，如果已经被标识为取消则退出。有关取消和操作对象的更多信息，参见 NSOperation，NSOperation 会在下篇进行学习。
#### start
```c++
- (void)start API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
```
&emsp;启动线程。（如在上面示例代码 `[thread start]` 添加 `[thread cancel]`，则 `thread` 线程不会再执行）

&emsp;此方法异步产生新线程，并在新线程上调用接收者的 `main` 方法。一旦线程开始执行，则 `executing` 属性返回 YES，这可能在 `start` 方法返回之后发生。

&emsp;如果使用 `target` 和 `selector` 初始化了接收器（NSThread 对象），则默认的 `main` 方法将自动调用该 `selector`。

&emsp;如果此线程是应用程序中分离的第一个线程，则此方法将 object 为 nil 的 `NSWillBecomeMultiThreadedNotification` 通知发布到默认通知中心。
#### main
```c++
- (void)main API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));    // thread body method
```
&emsp;线程的主要入口点例程。

&emsp;此方法的默认实现采用用于初始化 NSThread 的 `target` 和 `selector`，并在指定的 `target` 上调用 `selector`。如果你子类化 NSThread，则可以重写此方法并将其用于实现线程的 main body。如果这样做，则无需调用 `super`。

&emsp;你永远不要直接调用此方法。你应该始终通过调用 `start` 方法来启动线程。

&emsp;至此，我们的 NSThread 类的所有代码就看完了，还是挺清晰的哦。

### NSObject + NSThreadPerformAdditions
&emsp;下面是 NSObject 的一个分类，同时也是 `NSThread.h` 文件的最后一部分。其中的几个在主线程、指定线程和后台线程执行任务，还挺重要的，一起来看看吧！⛽️⛽️
#### performSelectorOnMainThread:withObject:waitUntilDone:modes: 
```c++
- (void)performSelectorOnMainThread:(SEL)aSelector withObject:(nullable id)arg waitUntilDone:(BOOL)wait modes:(nullable NSArray<NSString *> *)array;
```
&emsp;使用指定的模式在主线程上调用接收方的方法。

&emsp;`aSelector`: 一个选择器，用于标识要调用的方法。该方法不应有明显的返回值，并且应采用 id 类型的单个参数或不带参数。

&emsp;`arg`: 调用时传递给 `aSelector` 的参数。如果该方法不接受参数，则传递 `nil`。

&emsp;`wait`: 一个布尔值，指定当前线程是否在主线程上的接收器上执行指定的选择器之后才阻塞。指定 YES 是阻止该线程；否则，请指定 NO 以使此方法立即返回。如果当前线程也是主线程，并且你传递 YES，则立即执行该消息，否则将执行队列排队，以使其下次通过 runloop 运行。

&emsp;`array`: 字符串数组，标识允许执行指定选择器的模式。该数组必须至少包含一个字符串。如果为该参数指定 nil 或空数组，则此方法将返回而不执行指定的选择器。

&emsp;你可以使用此方法将消息传递到应用程序的主线程。主线程包含应用程序的主 runloop，并且是 NSApplication 对象接收事件的地方。在这种情况下，消息是你要在线程上执行的当前对象的方法。

&emsp;此方法使用 array 参数中指定的 runloop 模式，将消息在主线程的 runloop 中排队。作为其正常 runloop 处理的一部分，主线程使消息出队（假定它正在以指定的模式之一运行）并调用所需的方法。假设每个选择器的关联 runloop 模式相同，那么从同一线程对该方法的多次调用会导致相应的选择器排队，并以与调用相同的顺序执行。如果为每个选择器指定不同的模式，则其关联模式与当前 runloop 模式不匹配的所有选择器都将被跳过，直到 runloop 随后在该模式下执行。

&emsp;你无法取消使用此方法排队的消息。如果要取消当前线程上的消息的选项，则必须使用 `performSelector:withObject:afterDelay:` 或 `performSelector:withObject:afterDelay:inModes:` 方法。

&emsp;该方法向其当前上下文的 runloop 进行注册，并依赖于定期运行的 runloop 才能正确执行。一个常见的上下文是调用 dispatch queue 时调用，可能会调用此方法并最终向不是定期自动运行的 runloop 注册。如果在一个 dispatch queue 上运行时需要这种功能，则应使用 `dispatch_after` 和相关方法来获取所需的行为。
#### performSelectorOnMainThread:withObject:waitUntilDone:
```c++
- (void)performSelectorOnMainThread:(SEL)aSelector withObject:(nullable id)arg waitUntilDone:(BOOL)wait;
// equivalent to the first method with kCFRunLoopCommonModes
```
&emsp;使用默认模式（`kCFRunLoopCommonModes`）在主线程上调用接收方的方法。
#### performSelector:onThread:withObject:waitUntilDone:modes:
```c++
- (void)performSelector:(SEL)aSelector onThread:(NSThread *)thr withObject:(nullable id)arg waitUntilDone:(BOOL)wait modes:(nullable NSArray<NSString *> *)array API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
```
&emsp;使用指定的模式在指定的线程上调用接收方（任意的 NSObject 或其子类的对象）的方法。
#### performSelector:onThread:withObject:waitUntilDone:
```c++
- (void)performSelector:(SEL)aSelector onThread:(NSThread *)thr withObject:(nullable id)arg waitUntilDone:(BOOL)wait API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
// equivalent to the first method with kCFRunLoopCommonModes
```
&emsp;使用默认模式（`kCFRunLoopCommonModes`）在指定线程上调用接收方的方法。
#### performSelectorInBackground:withObject:
```c++
- (void)performSelectorInBackground:(SEL)aSelector withObject:(nullable id)arg API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
```
&emsp;在新的后台线程上调用接收方的方法。

&emsp;`aSelector`: 一个选择器，用于标识要调用的方法。该方法不应有明显的返回值，并且应采用 id 类型的单个参数或不带参数。

&emsp;`arg`: 调用时传递给方法的参数。如果该方法不接受参数，则传递 `nil`。

&emsp;此方法在你的应用程序中创建一个新线程，如果尚未将其置于多线程模式，则将其置于多线程模式。由 `aSelector` 表示的方法必须像在程序中创建任何其他新线程一样设置线程环境。

## 参考链接
**参考链接:🔗**
+ [pthread-百度百科词条](https://baike.baidu.com/item/POSIX线程?fromtitle=Pthread&fromid=4623312)
+ [pthread_create-百度百科词条](https://baike.baidu.com/item/pthread_create/5139072?fr=aladdin)
+ [pthread_cancel-百度百科词条](https://baike.baidu.com/item/pthread_cancel)
+ [在Linux中使用线程](https://blog.csdn.net/jiajun2001/article/details/12624923)
+ [线程属性pthread_attr_t简介](https://blog.csdn.net/hudashi/article/details/7709413)
+ [iOS多线程：『pthread、NSThread』详尽总结](https://juejin.im/post/6844903556009443335)
+ [iOS 多线程系列 -- pthread](https://www.jianshu.com/p/291598217865)
+ [iOS底层原理总结 - pthreads](https://www.jianshu.com/p/4434f18c5a95)
+ [C语言多线程pthread库相关函数说明](https://www.cnblogs.com/mq0036/p/3710475.html)
+ [iOS多线程中的实际方案之一pthread](https://www.jianshu.com/p/cfc6e7d2316a)
+ [iOS---多线程实现方案一 (pthread、NSThread)](https://www.cnblogs.com/fengmin/p/5548399.html)
