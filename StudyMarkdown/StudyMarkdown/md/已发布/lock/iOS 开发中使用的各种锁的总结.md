# iOS 开发中使用的各种锁的总结

> &emsp;锁是常用的同步工具。一段代码段在同一个时间只能允许被有限个线程访问，比如一个线程 A 进入需要保护的代码之前添加简单的互斥锁，另一个线程 B 就无法访问这段保护代码了，只有等待前一个线程 A 执行完被保护的代码后解锁，B 线程才能访问被保护的代码段。本篇就来总结这些 iOS 开发中使用到的锁，包括 spinlock_t、os_unfair_lock、pthread_mutex_t、NSLock、NSRecursiveLock、NSCondition、NSConditionLock、@synchronized、dispatch_semaphore、pthread_rwlock_t。 

## spinlock_t
> &emsp;自旋锁，也只有加锁、解锁和尝试加锁三个方法。和 NSLock 不同的是 NSLock 请求加锁失败的话，会先轮询，但一秒后便会使线程进入 waiting 状态，等待唤醒。而 OSSpinLock 会一直轮询，等待时会消耗大量 CPU 资源，不适用于较长时间的任务。
> &emsp;使用 OSSpinLock 需要先引入 #import <libkern/OSAtomic.h>。看到 usr/include/libkern/OSSpinLockDeprecated.h 名字后面的 Deprecated 强烈的提示着我们 OSSpinLock 已经不赞成使用了。
> &emsp;查看 OSSpinLockDeprecated.h 文件内容 OSSPINLOCK_DEPRECATED_REPLACE_WITH(os_unfair_lock) 提示我们使用 os_unfair_lock 代替 OSSpinLock。
> &emsp;OSSpinLock 存在线程安全问题，它可能导致优先级反转问题，目前我们在任何情况下都不应该再使用它，我们可以使用 apple 在 iOS 10.0 后推出的 os_unfair_lock (作为 OSSpinLock 的替代) 。关于 os_unfair_lock 我们下一节展开学习。
### OSSpinLock API 简单使用
&emsp;`OSSpinLock API` 很简单，首先看下使用示例。
```objective-c
#import "ViewController.h"
#import <libkern/OSAtomic.h> // 引入 OSSpinLock

@interface ViewController ()

@property (nonatomic, assign) NSInteger sum;
@property (nonatomic, assign) OSSpinLock lock;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    self.lock = OS_SPINLOCK_INIT; // 初始化锁
    dispatch_queue_t globalQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0); // 取得一个全局并发队列
    self.sum = 0; // sum 从 0 开始

    dispatch_async(globalQueue, ^{ // 异步任务 1
        OSSpinLockLock(&_lock); // 获得锁
        for (unsigned int i = 0; i < 10000; ++i) {
            self.sum++;
        }
        NSLog(@"⏰⏰⏰ %ld", self.sum);
        OSSpinLockUnlock(&_lock); // 解锁
    });
    
    dispatch_async(globalQueue, ^{ // 异步任务 2
        OSSpinLockLock(&_lock); // 获得锁
        for (unsigned int i = 0; i < 10000; ++i) {
            self.sum++;
        }
        NSLog(@"⚽️⚽️⚽️ %ld", self.sum);
        OSSpinLockUnlock(&_lock); // 解锁
    });
}
@end

// 打印 🖨️：
⏰⏰⏰ 10000
⚽️⚽️⚽️ 20000

// 把 lock 注释后，运行多次可总结出如下三种不同情况，
// 其中只有一个任务达到了 10000 以上另一个是 10000 以下，另一种是两者都达到了 10000 以上

// 情况 1:
⏰⏰⏰ 9064
⚽️⚽️⚽️ 13708
// 情况 2:
⏰⏰⏰ 11326
⚽️⚽️⚽️ 9933
// 情况 3:
⏰⏰⏰ 10906
⚽️⚽️⚽️ 11903
...
```
> &emsp;sum 属性使用 atomic 或 nonatomic 时结果相同，atomic 虽是原子操作，但它不是线程安全的，它的原子性只是限于对它所修饰的变量在 setter 和 getter 时加锁而已，当遇到 self.sum++ 或者 self.sum = self.sum + 1 等等这种复合操作时，atomic 是完全保证不了线程安全的。

&emsp;在不加锁情况下打印的数字有一些有趣的点，这里分析一下：（假设在全局并发队列下的两个 `dispatch_async` 任务都开启了新线程，并把两条线分别命名为 “⏰线程” 和 “⚽️线程”）

1. 可以确定的是 ⏰线程 和 ⚽️线程 不会有任何一个可以打印 `20000`。
2. ⏰线程 和 ⚽️线程 两者的打印都到了 `10000` 以上。
3. ⏰线程 或 ⚽️线程 其中一个打印在 `10000` 以上一个在 `10000` 以下。

&emsp;情况 1 我们都能想到，因为 ⏰线程 和 ⚽️线程 是并发进行的，不会存在一个线程先把 `sum` 自增到 `10000` 然后另一个线程再把 `sum` 自增到 `20000`，只有加锁或者 `self.sum` 自增的任务在串行队列中执行才行。
&emsp;情况 2 我们可能也好理解，两者都打印到 `10000` 以上，可以分析为某个时间点 ⏰线程 持续自增，然后 ⚽️线程 在这个时间点后执行循环时 `sum` 已经大于它上一次循环时的值了，然后 ⏰线程 和 ⚽️线程 下 `sum` 的值都是以大于其上一次循环的值往下继续循环，最后两条线程的打印 `sum` 值都是大于 `10000` 的。
&emsp;情况 3 则理解比较麻烦，为什么其中一个可以小于 `10000`，可能是其中一个线程执行忽快忽慢造成的吗？ 还有如果被缩小一次，那不是会导致两条线程最终打印 `sum` 都会小于 `10000` 吗？可能是 `self.sum` 读取时是从寄存器或内存中读取造成的吗？想到了 `volatile` 关键字。（暂时先分析到这里，分析不下去了)

### OSSpinLockDeprecated.h 文件内容
&emsp;下面直接查看 `OSSpinLockDeprecated.h` 中的代码内容。

&emsp;上面示例代码中每一行与 `OSSpinLock` 相关的代码都会有这样一行警告 ⚠️⚠️ `'OSSpinLock' is deprecated: first deprecated in iOS 10.0 - Use os_unfair_lock() from <os/lock.h> instead` 。正是由下面的 `OSSPINLOCK_DEPRECATED` 所提示，在 4 大系统中都提示我们都不要再用 `OSSpinLock` 了。
```c++
#ifndef OSSPINLOCK_DEPRECATED

#define OSSPINLOCK_DEPRECATED 1
#define OSSPINLOCK_DEPRECATED_MSG(_r) "Use " #_r "() from <os/lock.h> instead"
#define OSSPINLOCK_DEPRECATED_REPLACE_WITH(_r) \
    __OS_AVAILABILITY_MSG(macosx, deprecated=10.12, OSSPINLOCK_DEPRECATED_MSG(_r)) \
    __OS_AVAILABILITY_MSG(ios, deprecated=10.0, OSSPINLOCK_DEPRECATED_MSG(_r)) \
    __OS_AVAILABILITY_MSG(tvos, deprecated=10.0, OSSPINLOCK_DEPRECATED_MSG(_r)) \
    __OS_AVAILABILITY_MSG(watchos, deprecated=3.0, OSSPINLOCK_DEPRECATED_MSG(_r))
    
#else

#undef OSSPINLOCK_DEPRECATED
#define OSSPINLOCK_DEPRECATED 0
#define OSSPINLOCK_DEPRECATED_REPLACE_WITH(_r)

#endif
```
&emsp;下面是不同情况下的 `OSSpinLock API` 实现:
1. `#if !(defined(OSSPINLOCK_USE_INLINED) && OSSPINLOCK_USE_INLINED)` 为真不使用内联时的原始 API：
+ `#define    OS_SPINLOCK_INIT    0` 初始化。
```c++
/*! @abstract The default value for an <code>OSSpinLock</code>. OSSpinLock 的默认值是 0（unlocked 状态）
    @discussion
    The convention is that unlocked is zero, locked is nonzero. 惯例是: unlocked 时是零，locked 时是非零
 */
#define    OS_SPINLOCK_INIT    0
```
+ `OSSpinLock` 数据类型。
```c++
/*! @abstract Data type for a spinlock. 自旋锁的数据类型是 int32_t
    @discussion
    You should always initialize a spinlock to {@link OS_SPINLOCK_INIT} before using it. 
    在使用一个自旋锁之前，我们应该总是先把它初始化为 OS_SPINLOCK_INIT。（其实是对它赋值为数字 0）
 */
typedef int32_t OSSpinLock OSSPINLOCK_DEPRECATED_REPLACE_WITH(os_unfair_lock);
```
+ `OSSpinLockTry` 尝试加锁，`bool` 类型的返回值表示是否加锁成功，即使加锁失败也不会阻塞线程。
```c++
/*! @abstract Locks a spinlock if it would not block. 如果一个 spinlock 未锁定，则锁定它。
    @result
    Returns <code>false</code> if the lock was already held by another thread,
    <code>true</code> if it took the lock successfully. 
    如果锁已经被另一个线程所持有则返回 false，否则返回 true 表示加锁成功。
 */
OSSPINLOCK_DEPRECATED_REPLACE_WITH(os_unfair_lock_trylock)
__OSX_AVAILABLE_STARTING(__MAC_10_4, __IPHONE_2_0)
bool    OSSpinLockTry( volatile OSSpinLock *__lock );
```
+ `OSSpinLockLock` 加锁。
```
/*! @abstract Locks a spinlock. 锁定一个 spinlock
    @discussion
    Although the lock operation spins, it employs various strategies to back
    off if the lock is held.
    尽管锁定操作旋转，（当加锁失败时会一直处于等待状态，一直到获取到锁为止，获取到锁之前会一直处于阻塞状态）
    它采用各种策略来支持如果加锁成功，则关闭旋转。
 */
OSSPINLOCK_DEPRECATED_REPLACE_WITH(os_unfair_lock_lock)
__OSX_AVAILABLE_STARTING(__MAC_10_4, __IPHONE_2_0)
void    OSSpinLockLock( volatile OSSpinLock *__lock );
```
+ `OSSpinLockUnlock` 解锁。
```
/*! @abstract Unlocks a spinlock */
OSSPINLOCK_DEPRECATED_REPLACE_WITH(os_unfair_lock_unlock)
__OSX_AVAILABLE_STARTING(__MAC_10_4, __IPHONE_2_0)
void    OSSpinLockUnlock( volatile OSSpinLock *__lock );
```
2. `OSSPINLOCK_USE_INLINED` 为真使用内联，内联实现是使用 `os_unfair_lock_t` 代替 `OSSpinLock`。
> &emsp;Inline implementations of the legacy OSSpinLock interfaces in terms of the of the <os/lock.h> primitives. Direct use of those primitives is preferred.
> &emsp;NOTE: the locked value of os_unfair_lock is implementation defined and subject to change, code that relies on the specific locked value used by the legacy OSSpinLock interface WILL break when using these inline implementations in terms of os_unfair_lock.
> 
> &emsp;就 <os/lock.h> 中的原始接口而言，此处是原始 OSSpinLock 接口的内联实现。最好直接使用这些 primitives。
> &emsp;NOTE: os_unfair_lock 的锁定值是实现定义的，可能会更改。当使用这些内联实现时，依赖于旧版 OSSpinLock 接口使用的特定锁定值的代码会中断 os_unfair_lock。

&emsp;在函数前加 `OSSPINLOCK_INLINE` 告诉编译器尽最大努力保证被修饰的函数内联实现。
```c++
  #if __has_attribute(always_inline) // 尽最大努力保证函数内联实现
  #define OSSPINLOCK_INLINE static __inline
  #else
  #define OSSPINLOCK_INLINE static __inline __attribute__((__always_inline__))
  #endif

  #define OS_SPINLOCK_INIT 0 // 初始化为 0
  typedef int32_t OSSpinLock; // 类型依然是 int32_t

  #if  __has_extension(c_static_assert)
  // 如果 OSSpinLock 和 os_unfair_lock 内存长度不同，即类型不兼容，不能保证双方能正确的转换，直接断言。
  _Static_assert(sizeof(OSSpinLock) == sizeof(os_unfair_lock), "Incompatible os_unfair_lock type"); 
  #endif
```
+ `os_unfair_lock` 加锁。
```c++
  OSSPINLOCK_INLINE
  void
  OSSpinLockLock(volatile OSSpinLock *__lock)
  {
      // 转换为 os_unfair_lock_t。
      os_unfair_lock_t lock = (os_unfair_lock_t)__lock;
      return os_unfair_lock_lock(lock);
  }
```
+ `os_unfair_lock` 尝试加锁。
```c++
OSSPINLOCK_INLINE
bool
OSSpinLockTry(volatile OSSpinLock *__lock)
{
    // 转换为 os_unfair_lock_t。
    os_unfair_lock_t lock = (os_unfair_lock_t)__lock;
    return os_unfair_lock_trylock(lock);
}
```
+ `os_unfair_lock` 解锁。
```c++
OSSPINLOCK_INLINE
void
OSSpinLockUnlock(volatile OSSpinLock *__lock)
{
    // 转换为 os_unfair_lock_t。
    os_unfair_lock_t lock = (os_unfair_lock_t)__lock;
    return os_unfair_lock_unlock(lock);
}
```
&emsp;`#undef OSSPINLOCK_INLINE` 解除上面的宏定义。

3. 最后一种情况。
```c++
#define OS_SPINLOCK_INIT 0 // 初始化
typedef int32_t OSSpinLock OSSPINLOCK_DEPRECATED_REPLACE_WITH(os_unfair_lock); // 类型 int32_t
typedef volatile OSSpinLock *_os_nospin_lock_t
        OSSPINLOCK_DEPRECATED_REPLACE_WITH(os_unfair_lock_t); // 命名 _os_nospin_lock_t

OSSPINLOCK_DEPRECATED_REPLACE_WITH(os_unfair_lock_lock)
OS_NOSPIN_LOCK_AVAILABILITY
void _os_nospin_lock_lock(_os_nospin_lock_t lock); // 加锁
#undef OSSpinLockLock // 解除上面的原始 API 的加锁的宏定义
#define OSSpinLockLock(lock) _os_nospin_lock_lock(lock)

OSSPINLOCK_DEPRECATED_REPLACE_WITH(os_unfair_lock_trylock)
OS_NOSPIN_LOCK_AVAILABILITY
bool _os_nospin_lock_trylock(_os_nospin_lock_t lock); // 尝试加锁
#undef OSSpinLockTry // 解除上面的原始 API 的判断能否加锁的宏定义
#define OSSpinLockTry(lock) _os_nospin_lock_trylock(lock)

OSSPINLOCK_DEPRECATED_REPLACE_WITH(os_unfair_lock_unlock)
OS_NOSPIN_LOCK_AVAILABILITY
void _os_nospin_lock_unlock(_os_nospin_lock_t lock); // 解锁
#undef OSSpinLockUnlock // 解除上面的原始 API 的解锁的宏定义
#define OSSpinLockUnlock(lock) _os_nospin_lock_unlock(lock)
```
&emsp;至此 `OSSpinLockDeprecated.h` 文件代码结束，整体而言只有 4 条。

1. `OS_SPINLOCK_INIT` 初始化。
2. `OSSpinLockTry()` 尝试加锁，如果锁已经被另一个线程所持有则返回 false，否则返回 true，即使加锁失败也不会阻塞当前线程。
3. `OSSpinLockLock()` 加锁，加锁失败会一直等待，会阻塞当前线程。
4. `OSSpinLockUnlock` 解锁。

### OSSpinLock 的安全问题
&emsp;自旋锁 `OSSpinLock` 不是一个线程安全的锁，等待锁的线程会处于忙等（`busy-wait`）状态，一直占用着 `CPU` 资源。（类似一个 `while(1)` 循环一样，不停的查询锁的状态，注意区分 `runloop` 的机制，同样是阻塞，但是 `runloop` 是类似休眠的阻塞，不会耗费 `CPU` 资源，自旋锁的这种忙等机制使它相比其它锁效率更高，毕竟没有 **唤醒-休眠** 这些类似操作，从而能更快的处理事情。）自旋锁目前已经被废弃了，它可能会导致优先级反转。

&emsp;例如 `A/B` 两个线程，`A` 的优先级大于 `B` 的，我们的本意是 `A` 的任务优先执行，但是使用 `OSSpinLock` 后，如果是 `B` 优先访问了共享资源获得了锁并加锁，而 `A` 线程再去访问共享资源的时候锁就会处于忙等状态，由于 `A` 的优先级高它会一直占用 `CPU` 资源不会让出时间片，这样 `B` 一直不能获得 `CPU` 资源去执行任务，导致无法完成。

> &emsp;《不再安全的 OSSpinLock》原文: 新版 iOS 中，系统维护了 5 个不同的线程优先级/QoS: background，utility，default，user-initiated，user-interactive。高优先级线程始终会在低优先级线程前执行，一个线程不会受到比它更低优先级线程的干扰。这种线程调度算法会产生潜在的优先级反转问题，从而破坏了 spin lock。
具体来说，如果一个低优先级的线程获得锁并访问共享资源，这时一个高优先级的线程也尝试获得这个锁，它会处于 spin lock 的忙等状态从而占用大量 CPU。此时低优先级线程无法与高优先级线程争夺 CPU 时间，从而导致任务迟迟完不成、无法释放 lock。这并不只是理论上的问题，libobjc 已经遇到了很多次这个问题了，于是苹果的工程师停用了 OSSpinLock。
苹果工程师 Greg Parker 提到，对于这个问题，一种解决方案是用 truly unbounded backoff 算法，这能避免 livelock 问题，但如果系统负载高时，它仍有可能将高优先级的线程阻塞数十秒之久；另一种方案是使用 handoff lock 算法，这也是 libobjc 目前正在使用的。锁的持有者会把线程 ID 保存到锁内部，锁的等待者会临时贡献出它的优先级来避免优先级反转的问题。理论上这种模式会在比较复杂的多锁条件下产生问题，但实践上目前还一切都好。
libobjc 里用的是 Mach 内核的 thread_switch() 然后传递了一个 mach thread port 来避免优先级反转，另外它还用了一个私有的参数选项，所以开发者无法自己实现这个锁。另一方面，由于二进制兼容问题，OSSpinLock 也不能有改动。
最终的结论就是，除非开发者能保证访问锁的线程全部都处于同一优先级，否则 iOS 系统中所有类型的自旋锁都不能再使用了。-[《不再安全的 OSSpinLock》](https://blog.ibireme.com/2016/01/16/spinlock_is_unsafe_in_ios/)

## os_unfair_lock
> &emsp;`os_unfair_lock` 设计宗旨是用于替换 `OSSpinLock`，从 `iOS 10` 之后开始支持，跟 `OSSpinLock` 不同，等待 `os_unfair_lock` 的线程会处于休眠状态（类似 `Runloop` 那样），不是忙等（`busy-wait`）。
### os_unfair_lock 引子
&emsp;看到 `struct SideTable` 定义中第一个成员变量是 `spinlock_t slock;`， 这里展开对 `spinlock_t` 的学习。
```c++
struct SideTable {
    spinlock_t slock;
    ...
};
```
&emsp;`spinlock_t` 其实是使用 `using` 声明的一个模版类。
```c++
#if DEBUG
#   define LOCKDEBUG 1
#else
#   define LOCKDEBUG 0
#endif

template <bool Debug> class mutex_tt;
using spinlock_t = mutex_tt<LOCKDEBUG>;
```
&emsp;所以 `spinlock_t` 其实是一个互斥锁，与它的名字自旋锁是不符的，其实以前它是 `OSSpinLock`，因为其优先级反转导致的安全问题而被遗弃了。
```c++
template <bool Debug>
class mutex_tt : nocopy_t { // 继承自 nocopy_t
    os_unfair_lock mLock;
 public:
    constexpr mutex_tt() : mLock(OS_UNFAIR_LOCK_INIT) {
        lockdebug_remember_mutex(this);
    }
    ...
};
```
&emsp;`nocopy_t` 正如其名，删除编译器默认生成的复制构造函数和赋值操作符，而构造函数和析构函数则依然使用编译器默认生成的。
```c++
// Mix-in for classes that must not be copied.
// 构造函数 和 析构函数 使用编译器默认生成的，删除 复制构造函数 和 赋值操作符。
class nocopy_t {
  private:
    nocopy_t(const nocopy_t&) = delete;
    const nocopy_t& operator=(const nocopy_t&) = delete;
  protected:
    constexpr nocopy_t() = default;
    ~nocopy_t() = default;
};
```
&emsp;`mute_tt` 类的第一个成员变量是: `os_unfair_lock mLock`。
### os_unfair_lock 正片
&emsp;在 `usr/include/os/lock.h` 中看到 `os_unfair_lock` 的定义，使用 `os_unfair_lock` 首先需要引入 `#import <os/lock.h>` 。
### os_unfair_lock API 简单使用
&emsp;`os_unfair_lock API` 很简单，首先看下使用示例。
```c++
#import "ViewController.h"
#import <os/lock.h> // os_unfair_lock

@interface ViewController ()
@property (nonatomic, assign) NSInteger sum;
@property (nonatomic, assign) os_unfair_lock unfairL;
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    _sum = 100; // 只是给自动生成的 _sum 成员变量赋值，不会调用 sum 的 setter 函数
    self->_sum = 1000; // 只是给自动生成的 _sum 成员变量赋值，不会调用 sum 的 setter 函数
    
    // 一定要区分 self. 中的 .，它和 C/C++ 中的 . 是不一样的，OC 中的 . 是调用 getter/setter 函数。
    // 开始一直疑惑 self.xxx，self 是一个指针，不是应该使用 self->xxx 吗?
    // 在 OC 中，应该是 self->_xxx，_xxx 是 xxx 属性自动生成的对应的成员变量 _xxx
    // self 是一个结构体指针，所以访问指针的成员变量，只能是 self->_xxx，不能是 self->xxx
    
    // 等号左边的 "self.unfairL = xxx" 相当于调用 unfairL 的 setter 函数给它赋值
    // 即 [self setUnfairL:OS_UNFAIR_LOCK_INIT];
    
    // 等号右边的 "xxx = self.unfaiL" 或者 "self.unfairL" 的使用，
    // 相当于调用 unfairL 的 getter 函数，读取它的值
    // 相当于调用 getter 函数：[self unfairL]
    
    /*
     // os_unfair_lock 是一个结构体
     typedef struct os_unfair_lock_s {
     uint32_t _os_unfair_lock_opaque;
     } os_unfair_lock, *os_unfair_lock_t;
     */
     
    self.unfairL = OS_UNFAIR_LOCK_INIT; // 初始化
    dispatch_queue_t globalQueue_DEFAULT = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    self.sum = 0;
    
    __weak typeof(self) _self = self;
    dispatch_async(globalQueue_DEFAULT, ^{
        __strong typeof(_self) self = _self;

        // 不是使用 &self.unfairL，
        // 这样使用相当于 &[self unfairL]
        // 不能这样取地址
        // &[self unfairL]，
        // 报错: Cannot take the address of an rvalue of type 'os_unfair_lock'
        // 报错: 不能获取类型为 "os_unfair_lock" 的右值的地址
        // &self.unfairL;
        // 报错: Address of property expression requested
        // 只能使用 &self->_unfairL
        // 先拿到成员变量 _unfairL，然后再取地址
        
        os_unfair_lock_lock(&self->_unfairL); // 加锁
        // os_unfair_lock_lock(&self->_unfairL); // 重复加锁会直接 crash
        
        for (unsigned int i = 0; i < 10000; ++i) {
            self.sum++;
        }
        os_unfair_lock_unlock(&self->_unfairL); // 解锁
        NSLog(@"⏰⏰⏰ %ld", self.sum);
    });

    dispatch_async(globalQueue_DEFAULT, ^{
        __strong typeof(_self) self = _self;
        os_unfair_lock_lock(&self->_unfairL); // 加锁
        for (unsigned int i = 0; i < 10000; ++i) {
            self.sum++;
        }
        os_unfair_lock_unlock(&self->_unfairL); // 解锁
        NSLog(@"⚽️⚽️⚽️ %ld", self.sum);
    });
}
@end

// 打印:
⚽️⚽️⚽️ 10000
⏰⏰⏰ 20000
```
### lock.h 文件内容
&emsp;首先是一个宏定义告诉我们 `os_unfair_lock` 出现的时机。看到 `os_unfair_lock` 是在 `iOS 10.0` 以后首次出现的。
```c++
#define OS_LOCK_API_VERSION 20160309
#define OS_UNFAIR_LOCK_AVAILABILITY \
__API_AVAILABLE(macos(10.12), ios(10.0), tvos(10.0), watchos(3.0))
```
> &emsp;/*!
> * @typedef os_unfair_lock
> *
> * @abstract
> * Low-level lock that allows waiters to block efficiently on contention.
> *
> * In general, higher level synchronization primitives such as those provided by
> * the pthread or dispatch subsystems should be preferred.
> *
> * The values stored in the lock should be considered opaque and implementation
> * defined, they contain thread ownership information that the system may use
> * to attempt to resolve priority inversions.
> *
> * This lock must be unlocked from the same thread that locked it, attempts to
> * unlock from a different thread will cause an assertion aborting the process.
> *
> * This lock must not be accessed from multiple processes or threads via shared
> * or multiply-mapped memory, the lock implementation relies on the address of
> * the lock value and owning process.
> *
> * Must be initialized with OS_UNFAIR_LOCK_INIT
> *
> * @discussion
> * Replacement for the deprecated OSSpinLock. Does not spin on contention but
> * waits in the kernel to be woken up by an unlock.
> *
> * As with OSSpinLock there is no attempt at fairness or lock ordering, e.g. an
> * unlocker can potentially immediately reacquire the lock before a woken up
> * waiter gets an opportunity to attempt to acquire the lock. This may be
> * advantageous for performance reasons, but also makes starvation of waiters a
> * possibility.
> */
&emsp;对以上摘要内容进行总结，大概包括以下 4 点：
1. `os_unfair_lock` 是一个低等级锁。一些高等级的锁才应该是我们日常开发中的首选。
2. 必须使用加锁时的同一个线程来进行解锁，尝试使用不同的线程来解锁将导致断言中止进程。
3. 锁里面包含线程所有权信息来解决优先级反转问题。
4. 不能通过共享或多重映射内存从多个进程或线程访问此锁，锁的实现依赖于锁值的地址和所属进程。
5. 必须使用 `OS_UNFAIR_LOCK_INIT` 进行初始化。

&emsp;`os_unfair_lock_s` 结构，typedef 定义别名，`os_unfair_lock` 是一个 `os_unfair_lock_s` 结构体，`os_unfair_lock_t` 是一个 `os_unfair_lock_s` 指针，该结构体内部就一个 `uint32_t _os_unfair_lock_opaque` 成员变量。
```c++
OS_UNFAIR_LOCK_AVAILABILITY
typedef struct os_unfair_lock_s {
    uint32_t _os_unfair_lock_opaque;
} os_unfair_lock, *os_unfair_lock_t;
```
&emsp;针对不同的平台或者 `C++` 版本以不同的方式来进行初始化 `(os_unfair_lock){0}`。
1. `(os_unfair_lock){0}`
2. `os_unfair_lock{}`
3. `os_unfair_lock()`
4. `{0}`
```c++
#ifndef OS_UNFAIR_LOCK_INIT
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define OS_UNFAIR_LOCK_INIT ((os_unfair_lock){0}) // ⬅️
#elif defined(__cplusplus) && __cplusplus >= 201103L
#define OS_UNFAIR_LOCK_INIT (os_unfair_lock{}) // ⬅️
#elif defined(__cplusplus)
#define OS_UNFAIR_LOCK_INIT (os_unfair_lock()) // ⬅️
#else
#define OS_UNFAIR_LOCK_INIT {0} // ⬅️
#endif
#endif // OS_UNFAIR_LOCK_INIT
```
+  `os_unfair_lock_lock` 加锁。
```c++
/*!
 * @function os_unfair_lock_lock
 *
 * @abstract
 * Locks an os_unfair_lock. // 锁定一个 os_unfair_lock
 *
 * @param lock
 * Pointer to an os_unfair_lock. // 参数是一个 os_unfair_lock 指针
 */
OS_UNFAIR_LOCK_AVAILABILITY
OS_EXPORT OS_NOTHROW OS_NONNULL_ALL
void os_unfair_lock_lock(os_unfair_lock_t lock);
```
+ `os_unfair_lock_trylock` 尝试加锁。
```c++
/*!
 * @function os_unfair_lock_trylock
 *
 * @abstract
 * Locks an os_unfair_lock if it is not already locked.
 * 锁定一个 os_unfair_lock，如果它是之前尚未锁定的。
 *
 * @discussion
 * It is invalid to surround this function with a retry loop, if this function
 * returns false, the program must be able to proceed without having acquired
 * the lock, or it must call os_unfair_lock_lock() directly (a retry loop around
 * os_unfair_lock_trylock() amounts to an inefficient implementation of
 * os_unfair_lock_lock() that hides the lock waiter from the system and prevents
 * resolution of priority inversions).
 * 如果此函数返回 false，则用重试循环包围此函数是无效的，程序必须能够有能力处理这种没有获得锁的情况保证程序正常运行，
 * 或者必须直接调用 os_unfair_lock_lock()（os_unfair_lock_lock 会使线程阻塞一直到获得锁为止）。
 * （围绕 os_unfair_lock_trylock() 的重试循环等于 os_unfair_lock_lock() 的低效实现，
 * 该实现将 lock waiter 从系统中隐藏并解决了优先级反转问题）
 * 
 * @param lock
 * Pointer to an os_unfair_lock.
 * 参数是一个指向 os_unfair_lock 的指针。
 *
 * @result
 * Returns true if the lock was succesfully locked and false if the lock was already locked.
 * 锁定成功返回 true，如果之前已经被锁定则返回 false。
 * 
 */
OS_UNFAIR_LOCK_AVAILABILITY
OS_EXPORT OS_NOTHROW OS_WARN_RESULT OS_NONNULL_ALL
bool os_unfair_lock_trylock(os_unfair_lock_t lock);
```
+ `os_unfair_lock_unlock` 解锁。
```c++
/*!
 * @function os_unfair_lock_unlock
 *
 * @abstract
 * Unlocks an os_unfair_lock. // 解锁
 *
 * @param lock
 * Pointer to an os_unfair_lock.
 */
OS_UNFAIR_LOCK_AVAILABILITY
OS_EXPORT OS_NOTHROW OS_NONNULL_ALL
void os_unfair_lock_unlock(os_unfair_lock_t lock);
```
+ `os_unfair_lock_assert_owner` 判断当前线程是否是 `os_unfair_lock` 的所有者，否则触发断言。
```c++
/*!
 * @function os_unfair_lock_assert_owner
 *
 * @abstract
 * Asserts that the calling thread is the current owner of the specified unfair lock.
 *
 * @discussion
 * If the lock is currently owned by the calling thread, this function returns. 
 * 如果锁当前由调用线程所拥有，则此函数正常执行返回。
 *
 * If the lock is unlocked or owned by a different thread, this function asserts and terminates the process.
 * 如果锁是未锁定或者由另一个线程所拥有，则执行断言。
 *
 * @param lock
 * Pointer to an os_unfair_lock.
 */
OS_UNFAIR_LOCK_AVAILABILITY
OS_EXPORT OS_NOTHROW OS_NONNULL_ALL
void os_unfair_lock_assert_owner(os_unfair_lock_t lock);
```
+ `os_unfair_lock_assert_not_owner` 与上相反，如果当前线程是指定 `os_unfair_lock` 的所有者则触发断言。
```c++
/*!
 * @function os_unfair_lock_assert_not_owner
 *
 * @abstract
 * Asserts that the calling thread is not the current owner of the specified unfair lock.
 *
 * @discussion
 * If the lock is unlocked or owned by a different thread, this function returns.
 *
 * If the lock is currently owned by the current thread, this function assertsand terminates the process.
 *
 * @param lock
 * Pointer to an os_unfair_lock.
 */
OS_UNFAIR_LOCK_AVAILABILITY
OS_EXPORT OS_NOTHROW OS_NONNULL_ALL
void os_unfair_lock_assert_not_owner(os_unfair_lock_t lock);
```
+ 测试 `os_unfair_lock_assert_owner` 和 `os_unfair_lock_assert_not_owner`。
```c++
dispatch_async(globalQueue_DEFAULT, ^{
    os_unfair_lock_assert_owner(&self->_unfairL);
});
os_unfair_lock_assert_not_owner(&self->_unfairL);
```
## pthread_mutex_t
&emsp;`pthread_mutex_t` 是 C 语言下多线程互斥锁的方式，是跨平台使用的锁，等待锁的线程会处于休眠状态，可根据不同的属性配置把 `pthread_mutex_t` 初始化为不同类型的锁，例如：互斥锁、递归锁、条件锁。当使用递归锁时，允许同一个线程重复进行加锁，另一个线程访问时就会等待，这样可以保证多线程时访问共用资源的安全性。`pthread_mutex_t` 使用时首先要引入头文件 `#import <pthread.h>`。
```c++
PTHREAD_MUTEX_NORMAL // 缺省类型，也就是普通类型，当一个线程加锁后，其余请求锁的线程将形成一个队列，并在解锁后先进先出原则获得锁。
PTHREAD_MUTEX_ERRORCHECK // 检错锁，如果同一个线程请求同一个锁，则返回 EDEADLK，否则与普通锁类型动作相同。这样就保证当不允许多次加锁时不会出现嵌套情况下的死锁
PTHREAD_MUTEX_RECURSIVE // 递归锁，允许同一个线程对同一锁成功获得多次，并通过多次 unlock 解锁。
PTHREAD_MUTEX_DEFAULT // 适应锁，动作最简单的锁类型，仅等待解锁后重新竞争，没有等待队列。
```
&emsp;`pthread_mutex_trylock` 和 `trylock` 不同，`trylock` 返回的是 `YES` 和 `NO`，`pthread_mutex_trylock` 加锁成功返回的是 `0`，失败返回的是错误提示码。
### pthread_mutex_t 简单使用
&emsp;`pthread_mutex_t` 初始化时使用不同的 `pthread_mutexattr_t` 可获得不同类型的锁。
#### 互斥锁（ PTHREAD_MUTEX_DEFAULT 或 PTHREAD_MUTEX_NORMAL ）
```c++
#import "ViewController.h"
#import <pthread.h> // pthread_mutex_t

@interface ViewController ()

@property (nonatomic, assign) NSInteger sum;
@property (nonatomic, assign) pthread_mutex_t lock;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.sum = 0;
    dispatch_queue_t global_queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);

    // 1. 互斥锁，默认状态为互斥锁
    // 初始化属性
    pthread_mutexattr_t att;
    pthread_mutexattr_init(&att);
    
    // 设置属性，描述锁是什么类型
    pthread_mutexattr_settype(&att, PTHREAD_MUTEX_DEFAULT);
    
    // 初始化锁
    pthread_mutex_init(&self->_lock, &att);
    // 销毁属性
    pthread_mutexattr_destroy(&att);

    __weak typeof(self) _self = self;
    dispatch_async(global_queue, ^{
        __strong typeof(_self) self = _self;
        if (!self) return;
    
        pthread_mutex_lock(&self->_lock);
        for (unsigned int i = 0; i < 10000; ++i) {
            self.sum++;
        }
        pthread_mutex_unlock(&self->_lock);
        
        NSLog(@"😵😵😵 %ld", (long)self.sum);
    });
    
    dispatch_async(global_queue, ^{
        __strong typeof(_self) self = _self;
        if (!self) return;
        
        pthread_mutex_lock(&self->_lock);
        for (unsigned int i = 0; i < 10000; ++i) {
            self.sum++;
        }
        pthread_mutex_unlock(&self->_lock);

        NSLog(@"👿👿👿 %ld", (long)self.sum);
    });
}

#pragma mark - dealloc

- (void)dealloc {
    NSLog(@"🧑‍🎤🧑‍🎤🧑‍🎤 dealloc 同时释放🔒...");
    // 销毁锁
    pthread_mutex_destroy(&self->_lock);
}

@end

// 打印 🖨️：
😵😵😵 10000
👿👿👿 20000
🧑‍🎤🧑‍🎤🧑‍🎤 dealloc 同时释放🔒...
```
#### 递归锁（ PTHREAD_MUTEX_RECURSIVE ）
```c++
#import "ViewController.h"
#import <pthread.h> // pthread_mutex_t

static int count = 3;
@interface ViewController ()

@property (nonatomic, assign) NSInteger sum;
@property (nonatomic, assign) pthread_mutex_t recursivelock;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.sum = 0;
    dispatch_queue_t global_queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    
    // 2. 递归锁（PTHREAD_MUTEX_RECURSIVE）
    pthread_mutexattr_t recursiveAtt;
    pthread_mutexattr_init(&recursiveAtt);
    
    // 设置属性，描述锁是什么类型
    pthread_mutexattr_settype(&recursiveAtt, PTHREAD_MUTEX_RECURSIVE);
    
    // 初始化锁
    pthread_mutex_init(&self->_recursivelock, &recursiveAtt);
    // 销毁属性
    pthread_mutexattr_destroy(&recursiveAtt);
    
    __weak typeof(self) _self = self;
    dispatch_async(global_queue, ^{
        __strong typeof(_self) self = _self;
        if (!self) return;
        
        pthread_mutex_lock(&self->_recursivelock);
        for (unsigned int i = 0; i < 10000; ++i) {
            self.sum++;
        }
        pthread_mutex_unlock(&self->_recursivelock);

        NSLog(@"😵😵😵 %ld", (long)self.sum);
    });
    
    dispatch_async(global_queue, ^{
        __strong typeof(_self) self = _self;
        if (!self) return;
        
        // 递归锁验证
        [self recursiveAction];
    });
    
    dispatch_async(global_queue, ^{
        __strong typeof(_self) self = _self;
        if (!self) return;
        
        pthread_mutex_lock(&self->_recursivelock);
        for (unsigned int i = 0; i < 10000; ++i) {
            self.sum++;
        }
        pthread_mutex_lock(&self->_recursivelock);
        
        NSLog(@"👿👿👿 %ld", (long)self.sum);
    });
    
    dispatch_async(global_queue, ^{
        __strong typeof(_self) self = _self;
        if (!self) return;
        
        // 递归锁验证
        [self recursiveAction];
    });
}

#pragma mark - Private Methods
- (void)recursiveAction {
    pthread_mutex_lock(&self->_recursivelock);
    
    NSLog(@"😓😓😓 count = %d", count);
    if (count > 0) {
        count--;
        [self recursiveAction];
    }

    // else { // 如果是单线程的话，这里加一个递归出口没有任何问题
    // return;
    // }
    
    pthread_mutex_unlock(&self->_recursivelock);
    count = 3;
}

#pragma mark - dealloc
- (void)dealloc {
    NSLog(@"🧑‍🎤🧑‍🎤🧑‍🎤 dealloc 同时销毁🔒...");
    pthread_mutex_destroy(&self->_recursivelock);
}

@end

// 打印 🖨️:
😵😵😵 10000
😓😓😓 count = 3
😓😓😓 count = 2
😓😓😓 count = 1
😓😓😓 count = 0

👿👿👿 20000
😓😓😓 count = 3
😓😓😓 count = 2
😓😓😓 count = 1
😓😓😓 count = 0

🧑‍🎤🧑‍🎤🧑‍🎤 dealloc 同时销毁🔒...
```
#### 条件锁
&emsp;首先设定以下场景，两条线程 `A` 和 `B`，`A` 线程中执行删除数组元素，`B` 线程中执行添加数组元素，由于不知道哪个线程会先执行，所以需要加锁实现，只有在添加之后才能执行删除操作，为互斥锁添加条件可以实现。通过此方法可以实现线程依赖。
```c++
#import "ViewController.h"

#import <pthread.h> // pthread_mutex_t

@interface ViewController ()

@property (nonatomic, strong) NSMutableArray *dataArr;
@property (nonatomic, assign) pthread_mutex_t lock;
@property (nonatomic, assign) pthread_cond_t condition;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    // 初始化数组
    self.dataArr = [NSMutableArray array];
    
    // 初始化锁
    pthread_mutexattr_t att;
    pthread_mutexattr_init(&att);
    pthread_mutexattr_settype(&att, PTHREAD_MUTEX_DEFAULT);
    pthread_mutex_init(&self->_lock, &att);
    pthread_mutexattr_destroy(&att);

    // 初始化条件
    pthread_cond_init(&self->_condition, NULL);
    
    dispatch_queue_t global_DEFAULT = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    dispatch_queue_t global_HIGH = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0);

    __weak typeof(self) _self = self;
    
    dispatch_async(global_HIGH, ^{
        __strong typeof(_self) self = _self;
        if (!self) return;
        
        pthread_mutex_lock(&self->_lock);
        NSLog(@"🧑‍💻🧑‍💻🧑‍💻 delete begin");
        
        if (self.dataArr.count < 1) {
            pthread_cond_wait(&self->_condition, &self->_lock);
        }
        
        [self.dataArr removeLastObject];
        NSLog(@"数组执行删除元素操作");
        pthread_mutex_unlock(&self->_lock);
    });
    
    dispatch_async(global_DEFAULT, ^{
        __strong typeof(_self) self = _self;
        if (!self) return;
        
        pthread_mutex_lock(&self->_lock);
        NSLog(@"🧑‍💻🧑‍💻🧑‍💻 add begin");
        
        [self.dataArr addObject:@"CHM"];
        pthread_cond_signal(&self->_condition);
        
        NSLog(@"数组执行添加元素操作");
        pthread_mutex_unlock(&self->_lock);
    });

    NSThread *deThread = [[NSThread alloc] initWithTarget:self selector:@selector(deleteObj) object:nil];
    [deThread start];

    // sleep 1 秒，确保删除元素的线程先获得锁
    sleep(1);

    NSThread *addThread = [[NSThread alloc] initWithTarget:self selector:@selector(addObj) object:nil];
    [addThread start];
}

#pragma mark - Private Methods

- (void)deleteObj {
    pthread_mutex_lock(&self->_lock);

    NSLog(@"🧑‍💻🧑‍💻🧑‍💻 delete begin");
    // 添加判断，如果没有数据则添加条件
    
    if (self.dataArr.count < 1) {
        // 添加条件，如果数组为空，则添加等待线程休眠，将锁让出，这里会将锁让出去，所以下面的 addObj 线程才能获得锁
        // 接收到信号时会再次加锁，然后继续向下执行
        pthread_cond_wait(&self->_condition, &self->_lock);
    }
    
    [self.dataArr removeLastObject];
    NSLog(@"数组执行删除元素操作");

    pthread_mutex_unlock(&self->_lock);
}

- (void)addObj {
    pthread_mutex_lock(&self->_lock);

    NSLog(@"🧑‍💻🧑‍💻🧑‍💻 add begin");
    [self.dataArr addObject:@"HTI"];
    
    // 发送信号，说明已经添加元素了
    pthread_cond_signal(&self->_condition);
    
    NSLog(@"数组执行添加元素操作");
    pthread_mutex_unlock(&self->_lock);
}

#pragma mark - dealloc

- (void)dealloc {
    NSLog(@"🧑‍🎤🧑‍🎤🧑‍🎤 dealloc 同时释放🔒...");
    
    pthread_mutex_destroy(&self->_lock);
    pthread_cond_destroy(&self->_condition);
}

@end

// 打印 🖨️:
🧑‍💻🧑‍💻🧑‍💻 delete begin
🧑‍💻🧑‍💻🧑‍💻 add begin
数组执行添加元素操作
数组执行删除元素操作
🧑‍🎤🧑‍🎤🧑‍🎤 dealloc 同时释放🔒...
```
## NSLock
> &emsp;继承自 NSObject 并遵循 NSLocking 协议，lock 方法加锁，unlock 方法解锁，tryLock 尝试并加锁，如果返回 true 表示加锁成功，返回 false 表示加锁失败，谨记返回的 BOOL 表示加锁动作的成功或失败，并不是能不能加锁，即使加锁失败也会不会阻塞当前线程。lockBeforeDate: 是在指定的 Date 之前尝试加锁，如果在指定的时间之前都不能加锁，则返回 NO，且会阻塞当前线程。大概可以使用在：先预估上一个临界区的代码执行完毕需要多少时间，然后在这个时间之后为另一个代码段来加锁。

1. 基于 `mutex` 基本锁的封装，更加面向对象，等待锁的线程会处于休眠状态。
2. 遵守 `NSLocking` 协议，`NSLocking` 协议中仅有两个方法 `-(void)lock` 和 `-(void)unlock`。
3. 可能会用到的方法: 
  1. 初始化跟其他 `OC` 对象一样，直接进行 `alloc` 和 `init` 操作。
  2. `-(void)lock;` 加锁。
  3. `-(void)unlock;` 解锁。
  4. `-(BOOL)tryLock;` 尝试加锁。
  5. `-(BOOL)lockBeforeDate:(NSDate *)limit;` 在某一个时间点之前等待加锁。
4. 在主线程连续调用 `[self.lock lock]` 会导致主线程死锁。
5. 在主线程没有获取 `Lock` 的情况下和在获取 `Lock` 的情况下，连续两次 ` [self.lock unlock]` 都不会发生异常。（其他的锁可能连续解锁的情况下会导致 `crash`，还没有来的及测试）
6. 在子线程连续 `[self.lock lock]` 会导致死锁，同时别的子线获取 `self.lock` 则会一直等待下去。
7. 同时子线程死锁会导致 `ViewController` 不释放。

### NSLock 使用
```c++
@interface ViewController ()

@property (nonatomic, assign) NSInteger sum;
@property (nonatomic, strong) NSLock *lock;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.sum = 0;
    self.lock = [[NSLock alloc] init];
    dispatch_queue_t global_queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    
    __weak typeof(self) _self = self;
    dispatch_async(global_queue, ^{
        __strong typeof(_self) self = _self;
        if (!self) return;
        
        // 如果此处加锁失败，则阻塞当前线程，下面的代码不会执行，
        // 直到等到 lock 被其他线程释放了，它可以加锁了，才会接着执行下面的代码
        [self.lock lock];
        for (unsigned int i = 0; i < 10000; ++i) {
            self.sum++;
        }
        [self.lock unlock];
        
        NSLog(@"👿👿👿 %ld", (long)self.sum);
    });
    
    dispatch_async(global_queue, ^{
        __strong typeof(_self) self = _self;
        if (!self) return;
        
        // 如果此处加锁失败，则阻塞当前线程，下面的代码不会执行，
        // 直到等到 lock 被其他线程释放了，它可以加锁了，才会接着执行下面的代码
        [self.lock lock];
        for (unsigned int i = 0; i < 10000; ++i) {
            self.sum++;
        }
        [self.lock unlock];
        
        NSLog(@"😵😵😵 %ld", (long)self.sum);
    });
}

#pragma mark - dealloc
- (void)dealloc {
    NSLog(@"🧑‍🎤🧑‍🎤🧑‍🎤 dealloc 同时释放🔒...");
}

@end

// 打印结果:
😵😵😵 20000
👿👿👿 10000
🧑‍🎤🧑‍🎤🧑‍🎤 dealloc 同时释放🔒...

__weak typeof(self) _self = self;
// 线程 1
dispatch_async(global_queue, ^{
    __strong typeof(_self) self = _self;
    if (!self) return;

    [self.lock lock];
    for (unsigned int i = 0; i < 10000; ++i) {
        self.sum++;
    }
    sleep(3);
    [self.lock unlock];
    NSLog(@"👿👿👿 %ld", (long)self.sum);
});

// 线程 2
dispatch_async(global_queue, ^{
    __strong typeof(_self) self = _self;
    if (!self) return;
    sleep(1); // 保证让线程 1 先获得锁
    
    // 如果此处用 1，则在这个时间点不能获得锁
    // 如果是用大于 2 的数字，则能获得锁
    // 且这个 if 函数是会阻塞当前线程的
    if ([self.lock lockBeforeDate: [NSDate dateWithTimeIntervalSinceNow:1]]) {
        for (unsigned int i = 0; i < 10000; ++i) {
            self.sum++;
        }
        [self.lock unlock];
    } else {
        NSLog(@"lockBeforeDate 失败，会直接来到这里吗，会不阻塞当前线程吗？");
    }
    
    NSLog(@"😵😵😵 %ld", (long)self.sum);
});
```
&emsp;`[self.lock lockBeforeDate: [NSDate dateWithTimeIntervalSinceNow:1]]`，lockBeforeDate: 方法会在指定 Date 之前尝试加锁，且这个过程是会阻塞线程 2 的，如果在指定时间之前都不能加锁，则返回 false，在指定时间之前能加锁，则返回 true。
_priv 和 name，检测各个阶段，_priv 一直是 NULL。name 是用来标识的，用来输出的时候作为 lock 的名称。如果是三个线程，那么一个线程在加锁的时候，其余请求锁的的线程将形成一个等待队列，按先进先出原则，这个结果可以通过修改线程优先级进行测试得出。

## NSRecursiveLock
> &emsp;NSRecursiveLock 是递归锁，和 NSLock 的区别在于，它可以在同一个线程中重复加锁也不会导致死锁。NSRecursiveLock 会记录加锁和解锁的次数，当二者次数相等时，此线程才会释放锁，其它线程才可以上锁成功。

1. 同 `NSLock` 一样，也是基于 `mutex` 的封装，不过是基于 `mutex` 递归锁的封装，所以这是一个递归锁。
2. 遵守 `NSLocking` 协议，`NSLocking` 协议中仅有两个方法 `-(void)lock` 和 `-(void)unlock`。
3. 可能会用到的方法: 
  1. 继承自 NSObject，所以初始化跟其他 OC 对象一样，直接进行 alloc 和 init 操作。
  2. `-(void)lock;` 加锁
  3. `-(void)unlock;` 解锁
  4. `-(BOOL)tryLock;` 尝试加锁
  5. `-(BOOL)lockBeforeDate:(NSDate *)limit;` 在某一个时间点之前等待加锁。
4. 递归锁是可以在同一线程连续调用 `lock` 不会直接导致阻塞死锁，但是依然要执行相等次数的 `unlock`。不然异步线程再获取该递归锁会导致该异步线程阻塞死锁。
5. 递归锁允许同一线程多次加锁，不同线程进入加锁入口会处于等待状态，需要等待上一个线程解锁完成才能进入加锁状态。

### NSRecursiveLock 使用
&emsp;其实是实现上面 `pthread_mutex_t` 和 `PTHREAD_MUTEX_RECURSIVE` 完成的递归锁场景，只是这里使用 `NSRecursiveLock` `API` 更加精简，使用起来更加简单方便。
```c++
#import "ViewController.h"

static int count = 3;

@interface ViewController ()

@property (nonatomic, assign) NSInteger sum;
@property (nonatomic, strong) NSLock *lock;
@property (nonatomic, strong) NSRecursiveLock *recursiveLock;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    self.sum = 0;
    self.recursiveLock = [[NSRecursiveLock alloc] init];
    dispatch_queue_t global_queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    
    __weak typeof(self) _self = self;
    dispatch_async(global_queue, ^{
        __strong typeof(_self) self = _self;
        if (!self) return;
    
        [self.recursiveLock lock];
        for (unsigned int i = 0; i < 10000; ++i) {
            self.sum++;
        }
        [self.recursiveLock unlock];
        
        NSLog(@"👿👿👿 %ld", (long)self.sum);
    });
    
    dispatch_async(global_queue, ^{
        __strong typeof(_self) self = _self;
        if (!self) return;
        
        [self recursiveAction];
    });
    
    dispatch_async(global_queue, ^{
        __strong typeof(_self) self = _self;
        if (!self) return;
        
        [self.recursiveLock lock];
        for (unsigned int i = 0; i < 10000; ++i) {
            self.sum++;
        }
        [self.recursiveLock unlock];
        
        NSLog(@"😵😵😵 %ld", (long)self.sum);
    });
    
    dispatch_async(global_queue, ^{
        __strong typeof(_self) self = _self;
        if (!self) return;
        
        [self recursiveAction];
    });
}

#pragma mark - Private Methods

- (void)recursiveAction {
    [self.recursiveLock lock];
    NSLog(@"😓😓😓 count = %d", count);
    if (count > 0) {
        count--;
        [self recursiveAction];
    }

    // else { // 如果是单线程的话，这里加一个递归出口没有任何问题
    // return;
    // }

    [self.recursiveLock unlock];
    count = 3;
}

#pragma mark - dealloc
- (void)dealloc {
    NSLog(@"🧑‍🎤🧑‍🎤🧑‍🎤 dealloc 同时释放🔒...");
}

@end
// 打印结果:
😓😓😓 count = 3
👿👿👿 10000
😓😓😓 count = 2
😓😓😓 count = 1
😓😓😓 count = 0

😵😵😵 20000
😓😓😓 count = 3
😓😓😓 count = 2
😓😓😓 count = 1
😓😓😓 count = 0
```
```c++
NSRecursiveLock *lock = [[NSRecursiveLock alloc] init];
dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
  static void (^RecursieveBlock)(int);
  
  RecursiveBlock = ^(int value) {
      [lock lock];
      if (value > 0) {
          NSLog(@"value: %d", value);
          
          RecursiveBlock(value - 1);
      }
      [lock unlock];
  };
  
  RecursiveBlock(2);
});
```
&emsp;如上示例，如果用 NSLock 的话，lock 先锁上，但未执行解锁的时候，就会进入递归的下一层，并再次请求上锁，阻塞了该线程，线程被阻塞了，自然后面的解锁代码就永远不会执行，而形成了死锁。而 NSRecursiveLock 递归锁就是为了解决这个问题。

## NSCondition
> &emsp;NSCondition 的对象实际上作为一个锁和一个线程检查器，锁上之后其它线程也能上锁，而之后可以根据条件决定是否继续运行线程，即线程是否要进入 waiting 状态，经测试，NSCondition 并不会像上文的那些锁一样，先轮询，而是直接进入 waiting 状态，当其它线程中的该锁执行 signal 或者 broadcast 方法时，线程被唤醒，继续运行之后的方法。也就是使用 NSCondition 的模型为: 1. 锁定条件对象。2. 测试是否可以安全的履行接下来的任务。如果布尔值为假，调用条件对象的 wait 或者 waitUntilDate: 方法来阻塞线程。再从这些方法返回，则转到步骤 2 重新测试你的布尔值。（继续等待信号和重新测试，直到可以安全的履行接下来的任务，waitUntilDate: 方法有个等待时间限制，指定的时间到了，则返回 NO，继续运行接下来的任务。而等待信号，既线程执行 [lock signal] 发送的信号。其中 signal 和 broadcast 方法的区别在于，signal 只是一个信号量，只能唤醒一个等待的线程，想唤醒多个就得**多次调用**，而 broadcast 可以唤醒所有在等待的线程，如果没有等待的线程，这两个方法都没有作用。）

1. 基于 `mutex` 基础锁和 `cont` 条件的封装，所以它是互斥锁且自带条件，等待锁的线程休眠。
2. 遵守 `NSLocking` 协议，`NSLocking` 协议中仅有两个方法 `-(void)lock` 和 `-(void)unlock`。
3. 可能会用到的方法
  1. 初始化跟其它 OC 对象一样，直接进行 `alloc` 和 `init` 操作。
  2. `-(void)lock;` 加锁
  3. `-(void)unlock;` 解锁
  4. `-(BOOL)tryLock;` 尝试加锁
  5. `-(BOOL)lockBeforeDate:(NSDate *)limit;` 在某一个时间点之前等待加锁
  6. `-(void)wait;` 等待条件（进入休眠的同时放开锁，被唤醒的同时再次加锁）
  7. `-(void)signal;` 发送信号激活等待该条件的线程，切记线程收到后是从 wait 状态开始的
  8. `- (void)broadcast;` 发送广播信号激活等待该条件的所有线程，切记线程收到后是从 wait 状态开始的
### NSCondition 使用
```c++
#import "ViewController.h"

@interface ViewController ()

@property (nonatomic, strong) NSMutableArray *dataArr;
@property (nonatomic, strong) NSCondition *condition;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    // 初始化数组
    self.dataArr = [NSMutableArray array];
    
    self.condition = [[NSCondition alloc] init];
    dispatch_queue_t global_queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    
    __weak typeof(self) _self = self;
    dispatch_async(global_queue, ^{
        __strong typeof(_self) self = _self;
        if (!self) return;
        
        [self deleteObj];
    });
    
    dispatch_async(global_queue, ^{
        __strong typeof(_self) self = _self;
        if (!self) return;
        
        [self deleteObj];
    });
    
    // sleep 0.5 秒，确保删除元素的操作先取得锁
    sleep(0.5);
    
    dispatch_async(global_queue, ^{
        __strong typeof(_self) self = _self;
        if (!self) return;
        
        [self addObj];
    });
}

#pragma mark - Private Methods

- (void)deleteObj {
    [self.condition lock];
    NSLog(@"🧑‍💻🧑‍💻🧑‍💻 delete begin");
    
    // 添加判断，如果没有数据则添加条件
    if (self.dataArr.count < 1) {
        // 添加条件，如果数组为空，则添加等待线程休眠，将锁让出，这里会将锁让出去，所以下面的 addObj 线程才能获得锁
        // 接收到信号时会再次加锁，然后继续向下执行
        
        NSLog(@"下面是进入 wait...");
        [self.condition wait];
        
        // 当 broadcast 过来的时候还是继续往下执行，
        // 切记不是从 deleteObj 函数头部开始的，是从这里开始的
        // 所以当第一个异步删除数组元素后，第二个异步进来时数组已经空了
        NSLog(@"接收到 broadcast 或 signal 后的函数起点");
    }
    
    NSLog(@"%@", self.dataArr);
    [self.dataArr removeLastObject];
    NSLog(@"🧑‍💻🧑‍💻🧑‍💻 数组执行删除元素操作");
    [self.condition unlock];
}

- (void)addObj {
    [self.condition lock];
    NSLog(@"🧑‍💻🧑‍💻🧑‍💻 add begin");
    
    [self.dataArr addObject:@"CHM"];
    
    // 发送信号，说明已经添加元素了
    // [self.condition signal];
    // 通知所有符合条件的线程
    [self.condition broadcast];
    
    NSLog(@"🧑‍💻🧑‍💻🧑‍💻 数组执行添加元素操作");
    [self.condition unlock];
}

#pragma mark - dealloc
- (void)dealloc {
    NSLog(@"🧑‍🎤🧑‍🎤🧑‍🎤 dealloc 同时释放🔒...");
}
@end

// 打印结果:

// 这里两个异步线程执行都 [self.condition lock]，都正常进入了，
// 并没有因为 self.condition 先被一条线程获取加锁了而另一条线程处于阻塞等待状态， 

🧑‍💻🧑‍💻🧑‍💻 delete begin
下面是进入 wait...
🧑‍💻🧑‍💻🧑‍💻 delete begin
下面是进入 wait...

🧑‍💻🧑‍💻🧑‍💻 add begin
🧑‍💻🧑‍💻🧑‍💻 数组执行添加元素操作
接收到 broadcast 或 signal 后的函数起点
(
    CHM
)
🧑‍💻🧑‍💻🧑‍💻 数组执行删除元素操作
接收到 broadcast 或 signal 后的函数起点
(
)
🧑‍💻🧑‍💻🧑‍💻 数组执行删除元素操作
🧑‍🎤🧑‍🎤🧑‍🎤 dealloc 同时释放🔒...
```
## NSConditionLock
> &emsp;NSConditionLock 和 NSLock 类似，同样是继承自 NSObject 和遵循 NSLocking 协议，加解锁 try 等方法都类似，只是多了一个 condition 属性，以及每个操作都多了一个关于 condition 属性的方法，例如 tryLock 和 tryLockWhenCondition:，NSConditionLock 可以称为条件锁。只有 condition 参数与初始化的时候相等或者上次解锁后设置的 condition 相等，lock 才能正确的进行加锁操作。unlockWithCondition: 并不是当 condition 符合条件时才解锁，而是解锁之后，修改 condition 的值为入参，当使用 unlock 解锁时， condition 的值保持不变。如果初始化用 init，则 condition 默认值为 0。lockWhenCondition: 和 lock 方法类似，加锁失败会阻塞当前线程，一直等下去，直到能加锁成功。tryLockWhenCondition: 和 tryLock 类似，表示尝试加锁，即使加锁失败也不会阻塞当前线程，但是同时满足 lock 是空闲状态并且 condition 符合条件才能尝试加锁成功。从上面看出，NSConditionLock 还可以实现任务之间的依赖。

1. 基于 `NSCondition` 的进一步封装，可以更加高级的设置条件值。
  > &emsp;假设有这样的场景，三个线程 A B C，执行完 A  线程后才能执行 B，执行完 B 线程后执行 C，就是为线程之间的执行添加依赖，`NSConditionLock` 可以方便的完成这个功能。
2. 遵守 `NSLocking` 协议，`NSLocking` 协议中仅有两个方法 `-(void)lock` 和 `-(void)unlock`。
3. 可能用到的方法：
  1. 初始化跟其他 OC 对象一样，直接 `alloc` 和 `initWithCondition:(NSInteger)condition` 操作；（如果使用 `init` 方法，则 `condition` 默认为 0）。
  2. 有一个属性是 `@property(readonly) NSInteger condition;` 用来设置条件值，如果不设定，则默认为零。
  3. `-(void)lock;` 直接加锁。
  4. `-(void)lockWhenCondition:(NSInteger)condition;` 根据 `condition` 值加锁，如果入参和当前的 `condition` 不等则不加。
  5. `-(void)unlockWithCondition:(NSInteger)condition;` 解锁, 并设定 `condition` 的值为入参。
  6. `-(BOOL)tryLock;` 尝试加锁。
  7. `-(BOOL)lockBeforeDate:(NSDate *)limit;` 在某一个时间点之前等待加锁。
  
### NSConditionLock 使用
```c++
#import "ViewController.h"

@interface ViewController ()

@property (nonatomic, strong) NSConditionLock *lock;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.lock = [[NSConditionLock alloc] initWithCondition:0];
    [self createThreads];
}

#pragma mark - Private Methods

- (void)createThreads {
    // 需要执行的顺序为 A-B-C，但是因为在子线程中所以我们不能确定谁先执行，添加 sleep 使问题更突出点，c 线程先启动然后是 b 然后是 a。
    NSThread *c = [[NSThread alloc] initWithTarget:self selector:@selector(threadC) object:nil];
    [c start];
    sleep(0.2);
    
    NSThread *b = [[NSThread alloc] initWithTarget:self selector:@selector(threadB) object:nil];
    [b start];
    sleep(0.2);
    
    NSThread *a = [[NSThread alloc] initWithTarget:self selector:@selector(threadA) object:nil];
    [a start];
}

- (void)threadA {
    NSLog(@"A begin");
    [self.lock lockWhenCondition:0]; // 此时 Condition 值为 0 才能加锁成功，因为 Condition 初始值是 0，所以只有 A 能加锁成功
    NSLog(@"A threadExcute");
    [self.lock unlockWithCondition:1]; // 解锁并把 Condition 设置为 1
    // [self unlock]; // 如果此处使用 unlock，则导致 B C 线程死锁，且导致 ViewController 不释放
}

- (void)threadB {
    NSLog(@"B begin");
    [self.lock lockWhenCondition:1]; // 此时 Condition 值为 1 才能加锁成功
    NSLog(@"B threadExcute");
    [self.lock unlockWithCondition:2]; // 解锁并把 Condition 设置为 2
}

- (void)threadC {
    NSLog(@"C begin");
    [self.lock lockWhenCondition:2]; // 此时 Condition 值为 2 才能加锁成功
    NSLog(@"C threadExcute");
    [self.lock unlock]; // 解锁
}

#pragma mark - dealloc
- (void)dealloc {
    NSLog(@"🧑‍🎤🧑‍🎤🧑‍🎤 dealloc 同时释放🔒...");
}

// 打印结果:
// 虽然启动顺序是 C B A，但是执行顺序是 A B C，正是由 Condition 条件控制的，只有 Condition 匹配才能加锁成功，否则一直阻塞等待
C begin
B begin
A begin

A threadExcute
B threadExcute
C threadExcute
🧑‍🎤🧑‍🎤🧑‍🎤 dealloc 同时释放🔒...
```
1. `[self.lock unlock];` 执行后 `condition` 保持不变，依然是初始化的值或者是上次执行 `lockWhenCondition:` 时的值。 
2. A B C 3 条线程必须都执行加锁和解锁后 `ViewController` 才能正常释放，除了最后一条线程可以直接使用 `unlock` 执行解锁外，前两条线程 `unlockWithCondition:` 的入参 `condition` 的值必须和 `NSConditionLock` 当前的 `condition` 的值匹配起来。保证每条线程都 `lock` 和 `unlock`，无法正常执行时都会导致线程阻塞等待，`ViewController` 不会释放。
3. 在同一线程连续 `[self.lock lockWhenCondition:1];` 会直接阻塞死锁，不管用的 `condition` 是否和当前锁的 `condition` 相等，都会导致阻塞死锁。

## NSLocking、NSLock、NSConditionLock、NSRecursiveLock、NSCondition 定义
```c++
#import <Foundation/NSObject.h>

@class NSDate;

NS_ASSUME_NONNULL_BEGIN

// NSLocking 协议，上面提到锁的类型只要是 NS 开头的都会遵守此协议
@protocol NSLocking // 看到 NSLocking 协议只有加锁和解锁两个协议方法

- (void)lock;
- (void)unlock;

@end

@interface NSLock : NSObject <NSLocking> { // NSLock 是继承自 NSObject 并遵守 NSLocking 协议
@private
    void *_priv;
}

- (BOOL)tryLock; // 尝试加锁，返回 true 表示加锁成功
- (BOOL)lockBeforeDate:(NSDate *)limit; // 在某个 NSDate 之前加锁

@property (nullable, copy) NSString *name API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));

@end

// 条件锁
@interface NSConditionLock : NSObject <NSLocking> { // 继承自 NSObject 并遵守 NSLocking 协议
@private
    void *_priv;
}

- (instancetype)initWithCondition:(NSInteger)condition NS_DESIGNATED_INITIALIZER;

@property (readonly) NSInteger condition; // 只读的 condition 属性
- (void)lockWhenCondition:(NSInteger)condition; // 根据 condition 值加锁, 如果值不满足, 则不加;

- (BOOL)tryLock;
- (BOOL)tryLockWhenCondition:(NSInteger)condition; 

- (void)unlockWithCondition:(NSInteger)condition; // 解锁, 并设定 condition 的值;
- (BOOL)lockBeforeDate:(NSDate *)limit; // 在某一个时间点之前等待加锁
- (BOOL)lockWhenCondition:(NSInteger)condition beforeDate:(NSDate *)limit;

@property (nullable, copy) NSString *name API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));

@end

// 递归锁
@interface NSRecursiveLock : NSObject <NSLocking> { // 继承自 NSObject 并遵守 NSLocking 协议
@private
    void *_priv;
}

- (BOOL)tryLock; // 尝试加锁，返回 true 表示加锁成功
- (BOOL)lockBeforeDate:(NSDate *)limit; // 在某个 NSDate 之前加锁

@property (nullable, copy) NSString *name API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));

@end

API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0))
@interface NSCondition : NSObject <NSLocking> { // 继承自 NSObject 并遵守 NSLocking 协议
@private
    void *_priv;
}

- (void)wait; // 添加等待，线程休眠，并将锁让出
- (BOOL)waitUntilDate:(NSDate *)limit; // 某个 NSDate 
- (void)signal; // 发送信号，告知等待的线程，条件满足了
- (void)broadcast; // 通知所有符合条件的线程，（通知所有在等待的线程）

@property (nullable, copy) NSString *name API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));

@end

NS_ASSUME_NONNULL_END
```
## @synchronized
> &emsp;@synchronized(object) 指令使用的 object 为该锁的唯一标识，只有当标识相同时，才满足互斥，所以如果线程 2 中的 @synchronized(self) 改为 @synchronized(self.view)，则线程 2 就不会被阻塞，@synchronized 指令实现锁的优点就是我们不需要在代码中显式的创建锁对象，便可以实现锁的机制，但作为一种预防措施，@synchronized 块会隐式的添加一个异常处理例程来保护代码，**该处理例程会在异常抛出的时候自动释放互斥锁**。@synchronized 还有一个好处就是不用担心忘记解锁了。如果在 @synchronized(object) {} 内部 object 被释放或被设为 nil，从测试结果来看，不会产生问题，但如果 object 一开始就是 nil，则失去了加锁的功能。不过虽然 nil 不行，但是 [NSNull null] 是可以的。

1. `objc4-750` 版本之前（`iOS 12` 之前）`@synchronized` 是一个基于 `pthread_mutex_t` 封装的递归锁，之后实现则发生了改变，底层的封装变为了 `os_unfair_lock`。下面验证它，在 `@synchronized` 打断点，并且打开 `Debug-> Debug Workflow -> Always Show Disassembly`:
```c++
#pragma mark - Private Methods

- (void)recuresiveAction {
    // ➡️ 在下面 @synchronized 上打断点  
    @synchronized ([self class]) {
        NSLog(@"🌰🌰🌰 count = %d", count);
        if (count > 0) {
            count--;
            
            [self recuresiveAction];
        }
    }
}

// 汇编 objc_Simple`-[ViewController recuresiveAction]:
...
0x10868fc4b <+43>:  callq  0x108690360               ; symbol stub for: objc_sync_enter // 👈 看到调用了 objc_sync_enter 函数
...
0x10868fcc7 <+167>: callq  0x108690366               ; symbol stub for: objc_sync_exit // 👈 看到调用了 objc_sync_exit 函数
...
```
&emsp;看到 `@synchronized` 调用了 `objc_sync_enter` 和 `objc_sync_exit` 函数，下面从 `objc4-781` 中看一下这两个函数的实现，`objc_sync_exit` 和 `objc_sync_enter` 函数都位于 `objc-sync.mm`。
```c++
// End synchronizing on 'obj'. 
// Returns OBJC_SYNC_SUCCESS or OBJC_SYNC_NOT_OWNING_THREAD_ERROR
int objc_sync_exit(id obj)
{
    int result = OBJC_SYNC_SUCCESS;
    
    if (obj) {
        SyncData* data = id2data(obj, RELEASE); 
        if (!data) {
            result = OBJC_SYNC_NOT_OWNING_THREAD_ERROR;
        } else {
            bool okay = data->mutex.tryUnlock(); // 尝试解锁，返回 true 表示解锁成功，否则表示失败
            if (!okay) {
                result = OBJC_SYNC_NOT_OWNING_THREAD_ERROR;
            }
        }
    } else {
        // @synchronized(nil) does nothing
    }

    return result;
}
```
```c++
// Begin synchronizing on 'obj'. 
// Allocates recursive mutex associated with 'obj' if needed.
// Returns OBJC_SYNC_SUCCESS once lock is acquired.  
int objc_sync_enter(id obj)
{
    int result = OBJC_SYNC_SUCCESS;

    if (obj) {
        // 根据传入的对象，来获取一个锁，所以使用 @synchronized 时传入对象很重要
        SyncData* data = id2data(obj, ACQUIRE);
        ASSERT(data);
        data->mutex.lock(); // 这里使用 data 的 mutex 成员变量执行 lock
    } else {
        // @synchronized(nil) does nothing
        // 传入 nil 则什么也不做
        if (DebugNilSync) {
            _objc_inform("NIL SYNC DEBUG: @synchronized(nil); set a breakpoint on objc_sync_nil to debug");
        }
        objc_sync_nil();
    }
    
    return result;
}
```
`SyncData` 定义:
```c++
typedef struct alignas(CacheLineSize) SyncData {
    struct SyncData* nextData;
    DisguisedPtr<objc_object> object;
    int32_t threadCount;  // number of THREADS using this block
    recursive_mutex_t mutex;
} SyncData;
```
`recursive_mutex_t` 是使用 `using` 关键字声明的模版类：`using recursive_mutex_t = recursive_mutex_tt<LOCKDEBUG>;` 下面看一下 `recursive_mutex_tt` 底层结构:
```c++
template <bool Debug>
class recursive_mutex_tt : nocopy_t {
    // 底层封装的是 os_unfair_recursive_lock
    os_unfair_recursive_lock mLock;

  public:
    constexpr recursive_mutex_tt() : mLock(OS_UNFAIR_RECURSIVE_LOCK_INIT) {
        lockdebug_remember_recursive_mutex(this);
    }

    constexpr recursive_mutex_tt(const fork_unsafe_lock_t unsafe)
        : mLock(OS_UNFAIR_RECURSIVE_LOCK_INIT)
    { }

    void lock()
    {
        lockdebug_recursive_mutex_lock(this);
        os_unfair_recursive_lock_lock(&mLock);
    }
    ...
  };
```
`objc4-723` 中 `recursive_mutex_tt` 定义:
```c++
 // 在 objc4-723 版本中 recursive_mutex_tt 的底层结构为
 class recursive_mutex_tt : nocopy_t {
     // 底层封装的是互斥锁 pthread_mutex_t
     pthread_mutex_t mLock;

   public:
     recursive_mutex_tt() : mLock(PTHREAD_RECURSIVE_MUTEX_INITIALIZER) {
         lockdebug_remember_recursive_mutex(this);
     }

     recursive_mutex_tt(const fork_unsafe_lock_t unsafe)
         : mLock(PTHREAD_RECURSIVE_MUTEX_INITIALIZER)
     { }
 ...
 }
```
继续查看 `os_unfair_recursive_lock` 底层实现:
```c++
/*!
* @typedef os_unfair_recursive_lock
*
* @abstract
* Low-level lock that allows waiters to block efficiently on contention.
*
* @discussion
* See os_unfair_lock.
*
*/
OS_UNFAIR_RECURSIVE_LOCK_AVAILABILITY
typedef struct os_unfair_recursive_lock_s {

    os_unfair_lock ourl_lock; // 底层为互斥锁 os_unfair_lock 
    uint32_t ourl_count; // 因为 @synchronized 为递归锁，所以需要记录加锁次数
    
} os_unfair_recursive_lock, *os_unfair_recursive_lock_t;
```
&emsp;到这里可以确认了底层是 `os_unfair_lock`。 然后我们还注意到 `OS_UNFAIR_RECURSIVE_LOCK_AVAILABILITY`:
```c++
/*! @group os_unfair_recursive_lock SPI
 *
 * @abstract
 * Similar to os_unfair_lock, but recursive.
 * 与 os_unfair_lock 相似，但是是递归的。
 *
 * @discussion
 * Must be initialized with OS_UNFAIR_RECURSIVE_LOCK_INIT
 * 必须使用 OS_UNFAIR_RECURSIVE_LOCK_INIT 进行初始化
 */

#define OS_UNFAIR_RECURSIVE_LOCK_AVAILABILITY \
        __OSX_AVAILABLE(10.14) __IOS_AVAILABLE(12.0) \
        __TVOS_AVAILABLE(12.0) __WATCHOS_AVAILABLE(5.0)
```
&emsp;这里表明是 `iOS 12.0` 之后才是出现的。至此可验证 `iOS 12.0` 后 `@synchronized` 是一个封装了 `os_unfair_lock` 的递归锁（`os_unfair_recursive_lock`）。

2. `@synchronized(obj){...}` 传入一个对象 `obj` 进行加锁，如果传入空，则不执行操作。
### @synchronized 使用
```c++
#import "ViewController.h"

static int count = 3;

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    dispatch_queue_t global_queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    __weak typeof(self) _self = self;
    dispatch_async(global_queue, ^{
        __strong typeof(_self) self = _self;
        if (!self) return;
            
        [self recuresiveAction];
    });
}

#pragma mark - Private Methods

- (void)recuresiveAction {
    @synchronized ([self class]) {
        NSLog(@"🌰🌰🌰 count = %d", count);
        if (count > 0) {
            count--;
            
            [self recuresiveAction];
        }
    }
}

#pragma mark - dealloc
- (void)dealloc {
    NSLog(@"🧑‍🎤🧑‍🎤🧑‍🎤 dealloc 同时释放🔒...");
}

@end

// 打印结果:
🌰🌰🌰 count = 3
🌰🌰🌰 count = 2
🌰🌰🌰 count = 1
🌰🌰🌰 count = 0

🧑‍🎤🧑‍🎤🧑‍🎤 dealloc 同时释放🔒...
```
## dispatch_semaphore
> &emsp;dispatch_semaphore 是 GCD 用来同步的一种方式，与他相关的只有三个函数，一个是创建信号量，一个是等待信号量，一个是发送信号。
dispatch_semaphore 和 NSCondition 类似，都是一种基于信号的同步方式，但 NSCondition 信号只能发送，不能保存（如果没有线程在等待，则发送的信号会失效）。而 dispatch_semaphore 能保存发送的信号。dispatch_semaphore 的核心是 dispatch_semaphore_t 类型的信号量。
> #emsp;dispatch_semaphore_create(1) 方法可以创建一个 dispatch_semaphore_t 类型的信号量，设定信号量的初始化值为 1。注意，这里的传入参数必须大于等于 0，否则 dispatch_semaphore 会返回 NULL。
> &emsp;dispatch_semaphore_wait(signal, overTime) 方法会判断 signal 的信号值是否大于 0，大于 0 不会阻塞线程，消耗掉一个信号，执行后续任务。如果信号值为 0，该线程会和 NSCondition 一样直接进入 waiting 状态，等待其他线程发送信号唤醒线程去执行后续任务，或者当 overTime 时限到了，也会执行后续任务。
> &emsp;dispatch_semaphore_signal(signal) 发送信号，如果没有等待的线程调用信号，则使 signal 信号值加 1（做到对信号的保存）。一个 dispatch_semaphore_wait（signal, overTime）方法会去对应一个 dispatch_semaphore_signal(signal) 看起来像 NSLock 的 lock 和 unlock，其实可以这样理解，区别只在于有信号量这个参数，lock unlock 只能同一时间，一个线程访问被保护的临界区，而如果 dispatcch_semaphore 的信号量初始值为 x，则可以有 x 个线程同时访问被保护的临界区。

1. 本来是用于控制线程的最大并发数量，我们将并发数量设置为 `1` 也可以认为是加锁的功能。
2. 可能会用到的方法：
  1. 初始化 `dispatch_semaphore_create()` 传入的值为最大并发数量，设置为 `1` 则达到加锁效果。
  2. 判断信号量的值 `dispatch_semaphore_wait()` 如果大于 `0`，则可以继续往下执行（同时信号量的值减去 `1`），如果信号量的值为 `0`，则线程进入休眠状态等待（此方法的第二个参数就是设置要等多久，一般是使用永久 `DISPATCH_TIME_FOREVER`）。
  3. 释放信号量 `dispatch_semaphore_signal()` 同时使信号量的值加上 `1`。
  
### dispatch_semaphore 使用
```c++
#import "ViewController.h"

@interface ViewController ()

@property (nonatomic, assign) NSInteger sum;
@property (nonatomic, strong) dispatch_semaphore_t semaphore;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.sum = 0;
    self.semaphore = dispatch_semaphore_create(1);
    
    dispatch_queue_t global_queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    
    __weak typeof(self) _self = self;
    dispatch_async(global_queue, ^{
        __strong typeof(_self) self = _self;
        if (!self) return;
        
        dispatch_semaphore_wait(self.semaphore, DISPATCH_TIME_FOREVER);
        for (unsigned int i = 0; i < 10000; ++i) {
            self.sum++;
        }
        dispatch_semaphore_signal(self.semaphore);
        NSLog(@"🍐🍐🍐 %ld", (long)self.sum);
    });
    
    dispatch_async(global_queue, ^{
        __strong typeof(_self) self = _self;
        if (!self) return;
        
        dispatch_semaphore_wait(self.semaphore, DISPATCH_TIME_FOREVER);
        for (unsigned int i = 0; i < 10000; ++i) {
            self.sum++;
        }
        dispatch_semaphore_signal(self.semaphore);
        NSLog(@"🍎🍎🍎 %ld", (long)self.sum);
    });
}

#pragma mark - dealloc
- (void)dealloc {
    NSLog(@"🧑‍🎤🧑‍🎤🧑‍🎤 dealloc 同时释放🔒...");
}

@end

// 打印结果:
🍐🍐🍐 10000
🍎🍎🍎 20000
🧑‍🎤🧑‍🎤🧑‍🎤 dealloc 同时释放🔒...
```
## pthread_rwlock_t
&emsp;学习 `pthread_rwlock_t` 读写锁之前，首先引入一个问题：“如何实现一个多读单写的模型？”，需求如下:
+ 同时可以有多个线程读取。
+ 同时只能有一个线程写入。
+ 同时只能执行读取或者写入的一种。
&emsp;首先想到的就是我们的 `pthread_rwlock_t`。
1. 读取加锁可以同时多个线程进行，写入同时只能一个线程进行，等待的线程处于休眠状态。
2. 可能会用到的方法：
  1. `pthread_rwlock_init()` 初始化一个读写锁
  
  2. `pthread_rwlock_rdlock()` 读写锁的读取加锁
  3. `pthread_rwlock_wrlock()` 读写锁的写入加锁
  
  4. `pthread_rwlock_unlock()` 解锁
  5. `pthread_rwlock_destroy()` 销毁锁
### pthread_rwlock_t 使用
&emsp;代码示例，测试代码主要看，打印读取可以同时出现几个，打印写入同时只会出现一个。
```c++
#import "ViewController.h"
#import <pthread.h>

@interface ViewController ()

@property (nonatomic, assign) pthread_rwlock_t lock;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.

    [self rwlockType];
}

#pragma mark - Private methods
- (void)rwlockType {
    pthread_rwlock_init(&self->_lock, NULL);
    
    dispatch_queue_t globalQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    __weak typeof(self) _self = self;
    for (unsigned int i = 0; i < 100; ++i) {
        // 同时创建多个线程进行写入操作
        dispatch_async(globalQueue, ^{
            __weak typeof(_self) self = _self;
            if (!self) return;
            
            [self lockWriteAction];
        });
        
        dispatch_async(globalQueue, ^{
            __weak typeof(_self) self = _self;
            if (!self) return;
            
            [self lockWriteAction];
        });
        
        dispatch_async(globalQueue, ^{
            __weak typeof(_self) self = _self;
            if (!self) return;
            
            [self lockWriteAction];
        });
        
        // 同时创建多个线程进行读操作
        dispatch_async(globalQueue, ^{
            __strong typeof(_self) self = _self;
            if (!self) return;
            
            [self lockReadAction];
        });
        
        dispatch_async(globalQueue, ^{
            __strong typeof(_self) self = _self;
            if (!self) return;
            
            [self lockReadAction];
        });
        
        dispatch_async(globalQueue, ^{
            __strong typeof(_self) self = _self;
            if (!self) return;
            
            [self lockReadAction];
        });
    }
}

- (void)lockReadAction {
    pthread_rwlock_rdlock(&self->_lock);
    sleep(1);
    NSLog(@"RWLock read action %@", [NSThread currentThread]);
    pthread_rwlock_unlock(&self->_lock);
}

- (void)lockWriteAction {
    pthread_rwlock_wrlock(&self->_lock);
    sleep(1);
    NSLog(@"RWLock Write Action %@", [NSThread currentThread]);
    pthread_rwlock_unlock(&self->_lock);
}

#pragma mark - dealloc

-(void)dealloc {
    NSLog(@"🚚🚚🚚 deallocing...");
    
    pthread_rwlock_destroy(&self->_lock);
}

@end
// 打印结果: 可看到每次 write 操作同一个时间只执行一次，每次执行 write 操作至少相差 1 的时间，而 read 操作，几乎三次读取完全同一时刻进行
2020-08-23 21:56:47.918292+0800 algorithm_OC[17138:583665] RWLock Write Action <NSThread: 0x600001d45440>{number = 6, name = (null)}
2020-08-23 21:56:48.918953+0800 algorithm_OC[17138:583666] RWLock Write Action <NSThread: 0x600001d58740>{number = 4, name = (null)}
2020-08-23 21:56:49.924037+0800 algorithm_OC[17138:583667] RWLock Write Action <NSThread: 0x600001d06440>{number = 3, name = (null)}

2020-08-23 21:56:50.927716+0800 algorithm_OC[17138:583697] RWLock read action <NSThread: 0x600001d00d40>{number = 10, name = (null)}
2020-08-23 21:56:50.927716+0800 algorithm_OC[17138:583696] RWLock read action <NSThread: 0x600001d864c0>{number = 8, name = (null)}
2020-08-23 21:56:50.927721+0800 algorithm_OC[17138:583698] RWLock read action <NSThread: 0x600001da4b40>{number = 9, name = (null)}
...
```
### dispatch_barrier_async 实现多读单写
1. 传入的并发队列必须是手动创建的，`dispatch_queue_create()` 方式，如果传入串行队列或者通过 `dispatch_get_global_queue()` 方式创建，则 `dispatch_barrier_async` 的作用就跟 `dispatch_async` 变的一样。
2. 可能会用到的方法：
  1. `dispatch_queue_create()` 创建并发队列
  2. `dispatch_barrier_async()` 异步栅栏
### dispatch_barrier_async 使用
```c++
#import "ViewController.h"

@interface ViewController ()

@property (nonatomic, strong) dispatch_queue_t queue;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    [self barrierAsyncType];
}

#pragma mark - Private methods
- (void)barrierAsyncType {
    self.queue = dispatch_queue_create("rw_queue", DISPATCH_QUEUE_CONCURRENT);
    for (unsigned int i = 0; i < 100; ++i) {
    
        // 同时创建多个线程进行写入操作
        [self barrierWriteAction];
        [self barrierWriteAction];
        [self barrierWriteAction];
        
        // 同时创建多个线程进行读取操作
        [self barrierReadAction];
        [self barrierReadAction];
        [self barrierReadAction];
    }
}

- (void)barrierReadAction {
    dispatch_async(self.queue, ^{
        sleep(1);
        NSLog(@"barrier Read Action %@", [NSThread currentThread]);
    });
}

- (void)barrierWriteAction {
    // 写操作使用 dispatch_barrier_async
    dispatch_barrier_async(self.queue, ^{
        sleep(1);
        NSLog(@"barrier Write Action %@", [NSThread currentThread]);
    });
}

@end

// 打印结果: 从打印时间可以看出，write 操作是依序进行的，每次间隔 1 秒，而 read 操作则是 3 条线程同时进行读操作
2020-08-23 22:25:14.144265+0800 algorithm_OC[17695:604062] barrier Write Action <NSThread: 0x6000012a0180>{number = 5, name = (null)}
2020-08-23 22:25:15.148017+0800 algorithm_OC[17695:604062] barrier Write Action <NSThread: 0x6000012a0180>{number = 5, name = (null)}
2020-08-23 22:25:16.151869+0800 algorithm_OC[17695:604062] barrier Write Action <NSThread: 0x6000012a0180>{number = 5, name = (null)}

2020-08-23 22:25:17.156004+0800 algorithm_OC[17695:604062] barrier Read Action <NSThread: 0x6000012a0180>{number = 5, name = (null)}
2020-08-23 22:25:17.156040+0800 algorithm_OC[17695:604063] barrier Read Action <NSThread: 0x600001230340>{number = 6, name = (null)}
2020-08-23 22:25:17.156023+0800 algorithm_OC[17695:604065] barrier Read Action <NSThread: 0x6000012e6300>{number = 3, name = (null)}
...
```
## 总结
&emsp;锁粗略的效率排序（不同的锁可能更擅长不同的场景）

1. `os_unfair_lock` (`iOS 10` 之后)
2. `OSSpinLock` (`iOS 10` 之前)
3. `dispatch_semaphore` (`iOS` 版本兼容性好)
4. `pthread_mutex_t` (`iOS` 版本兼容性好)
5. `NSLock` (基于 `pthread_mutex_t` 封装)
6. `NSCondition` (基于 `pthread_mutex_t` 封装)
7. `pthread_mutex_t(recursive)` 递归锁的优先推荐
8. `NSRecursiveLock` (基于 `pthread_mutex_t` 封装)
9. `NSConditionLock` (基于 `NSCondition` 封装)
10. `@synchronized`
  1. `iOS 12` 之前基于 `pthread_mutex_t` 封装
  2. `iOS 12` 之后基于 `os_unfair_lock` 封装（iOS 12 之后它的效率应该不是最低，应该在 3/4 左右）

&emsp;自旋锁和互斥锁的取舍
&emsp;自旋锁和互斥锁怎么选择，其实这个问题已经没有什么意义，因为自旋锁 `OSSpinLock` 在 `iOS 10` 之后已经废弃了，而它的替换方案 `os_unfair_lock` 是互斥锁，但是我们仍然做一下对比:
**自旋锁:**
+ 预计线程需要等待的时间较短
+ 多核处理器
+ `CPU` 的资源不紧张
**互斥锁:**
+ 预计线程需要等待的时间较长
+ 单核处理器
+ 临界区（加锁解锁之间的部分）有 I/O 操作

**其它:**
加锁和解锁的实现一定要配对出现，不然当前线程就会一直阻塞下去。

## 参考链接
**参考链接:🔗**
+ [自旋锁](https://baike.baidu.com/item/自旋锁/9137985?fr=aladdin)
+ [不再安全的 OSSpinLock](https://blog.ibireme.com/2016/01/16/spinlock_is_unsafe_in_ios/)
+ [iOS 锁 部分一](https://www.jianshu.com/p/8ce323dbc491)
+ [如何深入理解 iOS 开发中的锁？](https://zhuanlan.zhihu.com/p/148788634)
+ [iOS 常见知识点（三）：Lock](https://www.jianshu.com/p/ddbe44064ca4)
+ [iOS锁-OSSpinLock与os_unfair_lock](https://www.jianshu.com/p/40adc41735b6)
+ [os_unfair_lock pthread_mutex](https://www.jianshu.com/p/6ff0dfe719bf)
+ [iOS 锁 部分一](https://www.jianshu.com/p/8ce323dbc491)
+ [iOS 锁 部分二](https://www.jianshu.com/p/d0fd5a5869e5)
+ [iOS 锁 部分三](https://www.jianshu.com/p/b6509683876c)
+ [iOS中保证线程安全的几种方式与性能对比](https://www.jianshu.com/p/938d68ed832c)
+ [关于 @synchronized，这儿比你想知道的还要多](http://yulingtianxia.com/blog/2015/11/01/More-than-you-want-to-know-about-synchronized/)
