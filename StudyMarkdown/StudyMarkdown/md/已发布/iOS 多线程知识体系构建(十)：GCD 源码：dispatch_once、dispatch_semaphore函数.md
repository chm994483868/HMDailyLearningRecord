# iOS 多线程知识体系构建(十)：GCD 源码：dispatch_once、dispatch_semaphore函数

> &emsp;上一篇我们学习了 dispatch_async 和 dispatch_sync 函数，本篇我们开始学习 GCD  中 dispatch_once、 dispatch_semaphore 相关的函数。

&emsp;GCD 函数阅读过程中会涉及多个由大量宏定义组成的结构体的定义，需要一步一步进行宏展开才能更好的理解代码。

## dispatch_once
&emsp;`dispatch_once` 保证任务只会被执行一次，即使同时多线程调用也是线程安全的。常用于创建单例、`swizzeld method` 等功能。

&emsp;`dispatch_once` 函数调用函数的形式也类似 `dispatch_sync` 函数，内部也是直接调用后缀加 `_f` 的同名函数（我们日常使用的 block 调用方式被直接转为函数形式）。

&emsp;`dispatch_once` 是同步函数，会阻塞当前线程，直到 block 执行完成后返回，才会执行接下的语句（我们日常写的函数本来就是同步顺序执行的，可能看 `dispatch_sync` 和 `dispatch_async` 函数看的有点魔怔了，看到这种函数参数里面有个 block 的函数形式时，总是首先想想它会不会阻塞，会不会立刻返回或者要等 block 执行完成才会返回）。

&emsp;`dispatch_once` 不同于我们的日常的函数，它的 block 参数全局只能调用一次，即使在多线程的环境中也是全局只能执行一次，那么当多个线程同时调用 `dispatch_once` 时，系统是怎么加锁或者阻塞线程保证线程安全的呢？下面我们一起探究一下...
```c++
#ifdef __BLOCKS__
void
dispatch_once(dispatch_once_t *val, dispatch_block_t block)
{
    dispatch_once_f(val, block, _dispatch_Block_invoke(block));
}
#endif
```
### dispatch_once_t
&emsp;与 `dispatch_once` 函数一起使用的谓词，必须将其初始化为零。（静态和全局变量默认为零。）
```c++
DISPATCH_SWIFT3_UNAVAILABLE("Use lazily initialized globals instead")
typedef intptr_t dispatch_once_t;
```
### dispatch_once_gate_t
&emsp;`dispatch_gate_t` 是指向 `dispatch_gate_s` 结构体的指针，`dispatch_gate_s` 结构体仅有一个 `uint32_t` 类型的成员变量 `dgl_lock`。

&emsp;`dispatch_once_gate_t` 是指向 `dispatch_once_gate_s` 结构体的指针，`dispatch_once_gate_s` 结构体内部仅包含一个联合体。
```c++
typedef struct dispatch_gate_s {
    // typedef uint32_t dispatch_lock;
    dispatch_lock dgl_lock;
} dispatch_gate_s, *dispatch_gate_t;

typedef struct dispatch_once_gate_s {
    union {
        dispatch_gate_s dgo_gate;
        uintptr_t dgo_once;
    };
} dispatch_once_gate_s, *dispatch_once_gate_t;
```

### DLOCK_ONCE_DONE/DLOCK_ONCE_UNLOCKED
&emsp;`DLOCK_ONCE_UNLOCKED` 与 `DLOCK_ONCE_DONE` 对应，分别代表 `dispatch_once` 执行前后的标记状态。`DLOCK_ONCE_UNLOCKED` 用于标记 `dispatch_once` 还没有执行过，`DLOCK_ONCE_DONE` 用于标记 `dispatch_once` 已经执行完了。
```c++
#define DLOCK_ONCE_UNLOCKED   ((uintptr_t)0)
#define DLOCK_ONCE_DONE   (~(uintptr_t)0)
```

### dispatch_once_f
&emsp;根据 `val`（`dgo_once` 成员变量） 的值非零与否来判断是否执行 `dispatch_once_f` 提交的函数。
```c++
DISPATCH_NOINLINE
void
dispatch_once_f(dispatch_once_t *val, void *ctxt, dispatch_function_t func)
{
    // 把 val 转换为 dispatch_once_gate_t 类型
    dispatch_once_gate_t l = (dispatch_once_gate_t)val;

#if !DISPATCH_ONCE_INLINE_FASTPATH || DISPATCH_ONCE_USE_QUIESCENT_COUNTER
    // 原子性获取 l->dgo_once 的值
    uintptr_t v = os_atomic_load(&l->dgo_once, acquire);
    
    // 判断 v 的值是否是 DLOCK_ONCE_DONE（大概率是，表示 val 已经被赋值 DLOCK_ONCE_DONE 和 func 已经执行过了），是则直接返回
    if (likely(v == DLOCK_ONCE_DONE)) {
        return;
    }
    
#if DISPATCH_ONCE_USE_QUIESCENT_COUNTER
    // 不同的判定形式
    // 判断 v 是否还存在锁，如果存在就返回
    if (likely(DISPATCH_ONCE_IS_GEN(v))) {
        return _dispatch_once_mark_done_if_quiesced(l, v);
    }
#endif

#endif

    // 原子判断 l（l->dgo_once）是否非零，
    // 非零表示正在执行（或已经执行过了，如果执行过了则 l->dgo_once 值是 DLOCK_ONCE_DONE，在上面的 #if 中应该已经 return 了，或者下面的 _dispatch_once_wait 函数内部结束函数执行），
    // 零的话表示还没有执行过则进入 if 开始执行提交的函数
    
    // _dispatch_once_gate_tryenter 函数当 l（l->dgo_once）值是 NULL（0）时返回 YES，否则返回 NO，
    // 因为是原子操作所以这样也保证了多线程调用时，只有最早的一条线程进入 if，开始执行提交的函数，然后另外的线程则是执行下面的 _dispatch_once_wait 函数阻塞线程然后等待 _dispatch_once_callout 里面的唤醒操作。

    if (_dispatch_once_gate_tryenter(l)) {
        // 执行 dispatch_once_f 提交的函数
        return _dispatch_once_callout(l, ctxt, func);
    }
    
    // 线程阻塞等待 dispatch_function_t 提交的 func 执行完成或者内部判断 func 已经执行完成了，则直接 return。 
    // 如果是阻塞的话，当 func 执行完后 _dispatch_once_callout 内部会发出广播唤醒阻塞线程
    return _dispatch_once_wait(l);
}
```
&emsp;下面对 `dispatch_once_f` 函数中嵌套调用的函数进行分析。

#### _dispatch_once_gate_tryenter
&emsp;`_dispatch_once_gate_tryenter` 函数原子性的判断 `l`（`l->dgo_once`）是否非零，非零表示 `dispatch_once_f` 提交的函数已经执行过了（或者正在执行），零的话还没有执行过。

&emsp;如果 `l`（`l->dgo_once`）是零的话，`_dispatch_once_gate_tryenter` 函数内部也会把 `l`（`l->dgo_once`） 赋值为当前线程的 ID（这里是一个临时赋值），在最后 `dispatch_once_f` 中提交的函数执行完成后 `_dispatch_once_gate_broadcast` 函数内部会把 `l`（`l->dgo_once`）赋值为 `DLOCK_ONCE_DONE`。（`_dispatch_lock_value_for_self` 函数是取出当前线程的 ID）

&emsp;这里藏一个点，就是每次执行 `_dispatch_once_gate_tryenter` 函数时 `l`（`l->dgo_once`）被赋值为当前线程的 ID，它对应了下面 `_dispatch_once_gate_broadcast` 函数内的 `v == value_self` 的判断，如果是单线程的调用 `dispatch_once_f` 的话，则是不存在其它线程被阻塞等待的，也就不需要线程唤醒的操作，而如果是多线程的环境下，`_dispatch_once_gate_tryenter` 函数会被不同线程调用，每次 `v` 都会被更新当前调用线程的 ID，而在 `_dispatch_once_gate_broadcast` 函数内部，`value_self` 的值是最初调用 `dispatch_once_f` 函数的线程的 ID，而 `v` 则是最后面调用 `dispatch_once_f` 函数的线程的 ID，且它们这些线程正在阻塞等待 `dispatch_once_f` 函数提交的函数执行完成，所以当 `dispatch_once_f` 提交的的函数执行完成后，需要对阻塞等待的线程进行唤醒操作。

```c++
DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_once_gate_tryenter(dispatch_once_gate_t l)
{
    // os_atomic_cmpxchg 原子性的判断 l->dgo_once 是否等于 DLOCK_ONCE_UNLOCKED（表示值为 0），若是 0 则赋值为当前线程 id
    // 如果 &l->dgo_once 的值为 NULL（0）则返回 YES，否则返回 NO
    return os_atomic_cmpxchg(&l->dgo_once, DLOCK_ONCE_UNLOCKED, (uintptr_t)_dispatch_lock_value_for_self(), relaxed);
}
```
##### os_atomic_cmpxchg
&emsp;`p` 变量相当于 `atomic_t` 类型的 `ptr` 指针用于获取当前内存访问制约规则 `m` 的值，用于对比旧值 `e`，若相等就赋新值 `v`，若不相等则把 `p` 内存空间里的值赋值给 `e`。
```c++
#define os_atomic_cmpxchg(p, e, v, m) \
        ({ _os_atomic_basetypeof(p) _r = (e); \
        atomic_compare_exchange_strong_explicit(_os_atomic_c11_atomic(p), \
        &_r, v, memory_order_##m, memory_order_relaxed); })
```
##### _dispatch_lock_value_for_self
&emsp;`_dispatch_lock_value_for_self` 取出当前线程的 ID，用于赋值给 `val`（`dgo_once` 成员变量）。（ `val` 在 `dispatch_once_f` 提交的函数执行完成之前会赋值为线程 ID，当提交的函数执行完成后会赋值为 `DLOCK_ONCE_DONE`，如我们为 `dispatch_once` 准备的 `static dispatch_once_t onceToken;`，在 `dispatch_once` 执行前打印 `onceToken` 值为 0，`onceToken` 初始值必须为 0，否则 `dispatch_once` 里的 block 不会执行，当 `dispatch_once` 执行完成后，打印 `onceToken`，它的值是 `-1`，如果我们手动把 `onceToken` 修改为 0，则可以再次执行 `dispatch_once` 提交的 block）。
```c++
DISPATCH_ALWAYS_INLINE
static inline dispatch_lock
_dispatch_lock_value_for_self(void)
{
    // _dispatch_tid_self() 为取出当前线程的ID
    return _dispatch_lock_value_from_tid(_dispatch_tid_self());
}
```
##### _dispatch_lock_value_from_tid
&emsp;`_dispatch_lock_value_from_tid` 函数内部仅是一个与操作。
```c++
DISPATCH_ALWAYS_INLINE
static inline dispatch_lock
_dispatch_lock_value_from_tid(dispatch_tid tid)
{
    // #define DLOCK_OWNER_MASK   ((dispatch_lock)0xfffffffc)
    return tid & DLOCK_OWNER_MASK;
}
```
&emsp;到这里与 `_dispatch_once_gate_tryenter` 相关的函数就看完了，根据 `_dispatch_once_gate_tryenter` 函数返回值，下面会有两个分支，一个是执行提交的函数，一个是提交的函数已经执行过了，执行接下来的 `_dispatch_once_wait(l)`  阻塞线程（提交的函数正在执行）或者结束函数调用（提交的函数已经执行完成）。（多线程环境下的同时调用，恰巧处于提交的函数正在执行，另一个线程的调用也进来了，那么后来的线程会阻塞等待，在提交的函数执行完成后该阻塞的线程会被唤醒），下面我们先看一下首次执行 `dispatch_once` 函数的过程。

#### _dispatch_once_callout
&emsp;`_dispatch_once_callout` 函数做了两件事，一是调用提交的函数，二是发出广播唤醒阻塞等待的线程。
```c++
// return _dispatch_once_callout(l, ctxt, func);

DISPATCH_NOINLINE
static void
_dispatch_once_callout(dispatch_once_gate_t l, void *ctxt,
        dispatch_function_t func)
{
    // _dispatch_client_callout 函数上篇已经看过了，内部实现很简单，就是执行函数 f(ctxt)，（func(ctxt)）
    _dispatch_client_callout(ctxt, func);
    
    // 广播唤醒阻塞的线程
    _dispatch_once_gate_broadcast(l);
}
```
##### _dispatch_client_callout
&emsp;执行 block，即调用 `f(ctxt)` 函数。

&emsp;`Thread-specific data（TSD）`是线程私有的数据，包含 TSD 的一些函数用于向线程（thread）对象中存储和获取数据。如 `CFRunLoopGetMain()` 函数，调用 `_CFRunLoopGet0()`，在其中即利用了 TSD 接口从 thread 中得到 runloop 对象。

&emsp;这里的 `_dispatch_get_tsd_base()` 也获取线程的私有数据。而 `_dispatch_get_unwind_tsd`、`_dispatch_set_unwind_tsd` 和 `_dispatch_free_unwind_tsd` 看来就是用于确保 `f(ctxt)` 执行的线程安全。
```c++
DISPATCH_NOINLINE
void
_dispatch_client_callout(void *ctxt, dispatch_function_t f)
{
    _dispatch_get_tsd_base();
    void *u = _dispatch_get_unwind_tsd();
    if (likely(!u)) return f(ctxt);
    _dispatch_set_unwind_tsd(NULL);
    
    f(ctxt); // 执行函数
    
    _dispatch_free_unwind_tsd();
    _dispatch_set_unwind_tsd(u);
}
```
##### _dispatch_once_gate_broadcast
&emsp;`_dispatch_once_gate_broadcast` 函数内部则是首先把 `l`（`dgo_once` 成员变量）原子性的赋值为 `DLOCK_ONCE_DONE`，表示提交的函数仅全局性的执行一次已经执行过了，然后是一句优化调用，如果 `v` 和 `value_self` 相等的话，表示目前是单线程在执行 `dispatch_once_f` 函数，提交的函数执行完了，但是不存在需要唤醒的阻塞线程，可以直接 return 了，如果不等的话则表示也有另外的线程在 `dispatch_once_f` 提交的函数正在执行的时候进来了，且这个 `v` 的值就是第二条线程的 ID，那么就需要执行 `_dispatch_gate_broadcast_slow` 来唤醒阻塞的线程。 
```c++
DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_once_gate_broadcast(dispatch_once_gate_t l)
{
    // 取出当前线程的 ID
    dispatch_lock value_self = _dispatch_lock_value_for_self();
    
    uintptr_t v;
    
#if DISPATCH_ONCE_USE_QUIESCENT_COUNTER
    v = _dispatch_once_mark_quiescing(l);
#else
    // 原子性的设置 l（dgo_once 成员变量）的值为 DLOCK_ONCE_DONE，并返回 l（dgo_once 成员变量）的原始值
    v = _dispatch_once_mark_done(l);
#endif
    
    // 这里为什么会有这一句呢？其实这是一句藏的很深的优化....
    
    // 如果是单线程执行 dispatch_once 的话则 v 等于 value_self，直接 return。
    // 如果是多线程执行 dispatch_once 的话则 v 可能不等于 value_self，需要执行接下来的 _dispatch_gate_broadcast_slow 唤醒阻塞的线程。
    if (likely((dispatch_lock)v == value_self)) return;
    
    // 唤醒阻塞的线程
    _dispatch_gate_broadcast_slow(&l->dgo_gate, (dispatch_lock)v);
}
```
##### _dispatch_once_mark_done
&emsp;原子性的设置 `&dgo->dgo_once` 的值为 `DLOCK_ONCE_DONE`，同时返回 `&dgo->dgo_once` 的旧值，此时，`dispatch_once` 即被标记为已执行过了。
```c++
DISPATCH_ALWAYS_INLINE
static inline uintptr_t
_dispatch_once_mark_done(dispatch_once_gate_t dgo)
{
    // 返回 &dgo->dgo_once 的旧值
    return os_atomic_xchg(&dgo->dgo_once, DLOCK_ONCE_DONE, release);
}
```
##### _dispatch_gate_broadcast_slow
```c++
void
_dispatch_gate_broadcast_slow(dispatch_gate_t dgl, dispatch_lock cur)
{
    if (unlikely(!_dispatch_lock_is_locked_by_self(cur))) {
        DISPATCH_CLIENT_CRASH(cur, "lock not owned by current thread");
    }

#if HAVE_UL_UNFAIR_LOCK
    // 唤醒线程
    _dispatch_unfair_lock_wake(&dgl->dgl_lock, ULF_WAKE_ALL);
#elif HAVE_FUTEX
    // 唤醒线程
    _dispatch_futex_wake(&dgl->dgl_lock, INT_MAX, FUTEX_PRIVATE_FLAG);
#else
    (void)dgl;
#endif
}
```
##### _dispatch_unfair_lock_wake
```c++
static void
_dispatch_unfair_lock_wake(uint32_t *uaddr, uint32_t flags)
{
    return _dlock_wake(uaddr, flags | UL_UNFAIR_LOCK);
}

static void
_dlock_wake(uint32_t *uaddr, uint32_t flags)
{
    int rc = __ulock_wake(flags | ULF_NO_ERRNO, uaddr, 0); // __ulock_wake 是必源的
    if (rc == 0 || rc == -ENOENT) return;
    DISPATCH_INTERNAL_CRASH(-rc, "ulock_wake() failed");
}
```
&emsp;到这里单线程第一次执行 `dispatch_once_f` 的流程就看完了，下面看一下另一个也超级重要的分支 `_dispatch_once_wait(l)`。

#### _dispatch_once_wait
&emsp;`_dispatch_once_wait` 函数中，`os_atomic_rmw_loop` 用于从操作系统底层获取状态，使用 `os_atomic_rmw_loop_give_up` 来执行返回操作，即不停查询 `&dgo->dgo_once` 的值，若变为 `DLOCK_ONCE_DONE` 则调用 `os_atomic_rmw_loop_give_up(return)` 退出等待。
```c++
void
_dispatch_once_wait(dispatch_once_gate_t dgo)
{
    // 获取当前线程的 ID
    dispatch_lock self = _dispatch_lock_value_for_self();
    uintptr_t old_v, new_v;
    
    // 取出 dgl_lock
    dispatch_lock *lock = &dgo->dgo_gate.dgl_lock;
    uint32_t timeout = 1;

    // 进入一个无限循环
    for (;;) {
        os_atomic_rmw_loop(&dgo->dgo_once, old_v, new_v, relaxed, {
            if (likely(old_v == DLOCK_ONCE_DONE)) { // 当 old_v 被 _dispatch_once_mark_done 中设置为 DLOCK_ONCE_DONE
            
                // ⬇️⬇️ 常规分支，dispatch_once_f 提交的函数已经执行完成，则直接结束函数执行
                os_atomic_rmw_loop_give_up(return);
            }
            
#if DISPATCH_ONCE_USE_QUIESCENT_COUNTER
            if (DISPATCH_ONCE_IS_GEN(old_v)) {
                os_atomic_rmw_loop_give_up({
                    os_atomic_thread_fence(acquire);
                    return _dispatch_once_mark_done_if_quiesced(dgo, old_v);
                });
            }
#endif
            // #define DLOCK_WAITERS_BIT   ((dispatch_lock)0x00000001)
            
            new_v = old_v | (uintptr_t)DLOCK_WAITERS_BIT;
            if (new_v == old_v) os_atomic_rmw_loop_give_up(break); // 跳出循环
        });
        if (unlikely(_dispatch_lock_is_locked_by((dispatch_lock)old_v, self))) {
            DISPATCH_CLIENT_CRASH(0, "trying to lock recursively");
        }
#if HAVE_UL_UNFAIR_LOCK
        _dispatch_unfair_lock_wait(lock, (dispatch_lock)new_v, 0,
                DLOCK_LOCK_NONE);
#elif HAVE_FUTEX
        _dispatch_futex_wait(lock, (dispatch_lock)new_v, NULL,
                FUTEX_PRIVATE_FLAG);
#else
        _dispatch_thread_switch(new_v, flags, timeout++);
#endif
        (void)timeout;
    }
}
```
##### os_atomic_rmw_loop
&emsp;`os_atomic_rmw_loop` 是一个宏定义，`__VA_ARGS__`  参数表示 do while 循环里的操作。
```c++
#define os_atomic_rmw_loop(p, ov, nv, m, ...)  ({ \
    bool _result = false; \
    __typeof__(p) _p = (p); \
    ov = os_atomic_load(_p, relaxed); \ // 原子读取 &dgo->dgo_once 的值
    do { \
        __VA_ARGS__; \
        _result = os_atomic_cmpxchgvw(_p, ov, nv, &ov, m); \
    } while (unlikely(!_result)); \
    _result; \
})
```
&emsp;看到这里 `dispatch_once` 的内容已经看完了，`_dispatch_once_wait` 函数内部是用了一个 do while 循环来阻塞等待 `&dgo->dgo_once` 的值被置为 `DLOCK_ONCE_DONE`，看到一些文章中说是用 `_dispatch_thread_semaphore_wait` 来阻塞线程，这里已经发生更新。

## dispatch_semaphore
&emsp;dispatch_semaphore 是 GCD 中最常见的操作，通常用于保证资源的多线程安全性和控制任务的并发数量。其本质实际上是基于 mach 内核的信号量接口来实现的。

### dispatch_semaphore_s
&emsp;`dispatch_semaphore_t` 是指向 `dispatch_semaphore_s` 结构体的指针。首先看一下基础的数据结构。
```c++
struct dispatch_queue_s;

DISPATCH_CLASS_DECL(semaphore, OBJECT);
struct dispatch_semaphore_s {
    DISPATCH_OBJECT_HEADER(semaphore);
    long volatile dsema_value;
    long dsema_orig;
    _dispatch_sema4_t dsema_sema;
};
```
&emsp;宏定义展开如下:
```c++
struct dispatch_semaphore_s;

// OS_OBJECT_CLASS_DECL(dispatch_semaphore, DISPATCH_OBJECT_VTABLE_HEADER(dispatch_semaphore))

struct dispatch_semaphore_extra_vtable_s {
    unsigned long const do_type;
    void (*const do_dispose)(struct dispatch_semaphore_s *, bool *allow_free);
    size_t (*const do_debug)(struct dispatch_semaphore_s *, char *, size_t);
    void (*const do_invoke)(struct dispatch_semaphore_s *, dispatch_invoke_context_t, dispatch_invoke_flags_t);
};

struct dispatch_semaphore_vtable_s {
    // _OS_OBJECT_CLASS_HEADER();
    void (*_os_obj_xref_dispose)(_os_object_t);
    void (*_os_obj_dispose)(_os_object_t);
    
    struct dispatch_semaphore_extra_vtable_s _os_obj_vtable;
};

// OS_OBJECT_CLASS_SYMBOL(dispatch_semaphore)

extern const struct dispatch_semaphore_vtable_s _OS_dispatch_semaphore_vtable;
extern const struct dispatch_semaphore_vtable_s OS_dispatch_semaphore_class __asm__("__" OS_STRINGIFY(dispatch_semaphore) "_vtable");

struct dispatch_semaphore_s {
    struct dispatch_object_s _as_do[0];
    struct _os_object_s _as_os_obj[0];
    
    const struct dispatch_semaphore_vtable_s *do_vtable; /* must be pointer-sized */
    
    int volatile do_ref_cnt;
    int volatile do_xref_cnt;
    
    struct dispatch_semaphore_s *volatile do_next;
    struct dispatch_queue_s *do_targetq;
    void *do_ctxt;
    void *do_finalizer;
    
    // 可看到上半部分和其它 GCD 对象都是相同的，毕竟大家都是继承自 dispatch_object_s，重点是下面两个新的成员变量
    // dsema_value 和 dsema_orig 是信号量执行任务的关键，执行一次 dispatch_semaphore_wait 操作，dsema_value 的值就做一次减操作
    
    long volatile dsema_value;
    long dsema_orig;
    _dispatch_sema4_t dsema_sema;
};
```
&emsp;`DISPATCH_VTABLE_INSTANCE` 宏定义包裹的内容是 `dispatch_semaphore_vtable_s` 结构体中的内容的初始化，即信号量的一些操作函数。（在 init.c 文件中 Dispatch object cluster 部分包含很多 GCD 对象的操作函数的的初始化）
```c++
// dispatch_semaphore_extra_vtable_s 结构体中对应的成员变量的赋值
DISPATCH_VTABLE_INSTANCE(semaphore,
    .do_type        = DISPATCH_SEMAPHORE_TYPE,
    .do_dispose     = _dispatch_semaphore_dispose,
    .do_debug       = _dispatch_semaphore_debug,
    .do_invoke      = _dispatch_object_no_invoke,
);
⬇️（宏展开）
DISPATCH_VTABLE_SUBCLASS_INSTANCE(semaphore, semaphore, __VA_ARGS__)
⬇️（宏展开）
OS_OBJECT_VTABLE_SUBCLASS_INSTANCE(dispatch_semaphore, dispatch_semaphore, _dispatch_xref_dispose, _dispatch_dispose, __VA_ARGS__)
⬇️（宏展开）
const struct dispatch_semaphore_vtable_s OS_OBJECT_CLASS_SYMBOL(dispatch_semaphore) = { \
    ._os_obj_xref_dispose = _dispatch_xref_dispose, \
    ._os_obj_dispose = _dispatch_dispose, \
    ._os_obj_vtable = { __VA_ARGS__ }, \
}
⬇️（宏展开）
const struct dispatch_semaphore_vtable_s OS_dispatch_semaphore_class = {
    ._os_obj_xref_dispose = _dispatch_xref_dispose,
    ._os_obj_dispose = _dispatch_dispose,
    ._os_obj_vtable = { 
        .do_type        = DISPATCH_SEMAPHORE_TYPE, // 类型
        .do_dispose     = _dispatch_semaphore_dispose, // dispose 函数赋值
        .do_debug       = _dispatch_semaphore_debug, // debug 赋值
        .do_invoke      = _dispatch_object_no_invoke, // invoke 函数赋值
    }, 
}
```
&emsp;`dispatch_semaphore_s` 结构体中：`dsema_orig` 是信号量的初始值，`dsema_value` 是信号量的当前值，信号量的相关 API 正是通过操作 `dsema_value` 来实现其功能的，`_dispatch_sema4_t` 是信号量的结构。

#### _dispatch_sema4_t/_DSEMA4_POLICY_FIFO
&emsp;在不同的平台和环境下 `_dispatch_sema4_t` 使用了不同的类型。（具体类型在 libdispatch 源码中未找到）

&emsp;`_DSEMA4_POLICY_FIFO` 在下面的 `_dispatch_sema4_init` 函数调用中会用到。
```c++
#if USE_MACH_SEM

  typedef semaphore_t _dispatch_sema4_t;
  #define _DSEMA4_POLICY_FIFO  SYNC_POLICY_FIFO
  
#elif USE_POSIX_SEM

  typedef sem_t _dispatch_sema4_t;
  #define _DSEMA4_POLICY_FIFO 0
  
#elif USE_WIN32_SEM

  typedef HANDLE _dispatch_sema4_t;
  #define _DSEMA4_POLICY_FIFO 0
  
#else

#error "port has to implement _dispatch_sema4_t"

#endif
```
&emsp;下面看一下 `dispatch_semaphore_s` 相关 API 的源码实现。

### dispatch_semaphore_create
&emsp;`dispatch_semaphore_create` 用初始值（`long value`）创建新的计数信号量。

&emsp;当两个线程需要协调特定事件的完成时，将值传递为零非常有用。传递大于零的值对于管理有限的资源池非常有用，该资源池的大小等于该值（例如我们有多个文件要从服务器下载下来，然后用 dispatch_semaphore 限制只能并发五条线程（`dispatch_semaphore_create(5)`）进行下载）。

&emsp;参数 `value`：信号量的起始值，传递小于零的值将导致返回 `NULL`。返回值 `result`：新创建的信号量，失败时为 `NULL`。
```c++
dispatch_semaphore_t
dispatch_semaphore_create(long value)
{
    // 指向 dispatch_semaphore_s 结构体的指针
    dispatch_semaphore_t dsema;

    // If the internal value is negative, then the absolute of the value is equal
    // to the number of waiting threads. 
    // Therefore it is bogus to initialize the semaphore with a negative value.
    
    if (value < 0) {
        // #define DISPATCH_BAD_INPUT   ((void *_Nonnull)0)
        // 如果 value 值小于 0，则直接返回 0
        return DISPATCH_BAD_INPUT;
    }

    // DISPATCH_VTABLE(semaphore) ➡️ &OS_dispatch_semaphore_class
    // _dispatch_object_alloc 是为 dispatch_semaphore_s 申请空间，然后用 &OS_dispatch_semaphore_class 初始化，
    // &OS_dispatch_semaphore_class 设置了 dispatch_semaphore_t 的相关回调函数，如销毁函数 _dispatch_semaphore_dispose 等
    dsema = _dispatch_object_alloc(DISPATCH_VTABLE(semaphore),
            sizeof(struct dispatch_semaphore_s));
    
    // #if DISPATCH_SIZEOF_PTR == 8
    // // the bottom nibble must not be zero, the rest of the bits should be random we sign extend the 64-bit version so that a better instruction encoding is generated on Intel
    // #define DISPATCH_OBJECT_LISTLESS ((void *)0xffffffff89abcdef)
    // #else
    // #define DISPATCH_OBJECT_LISTLESS ((void *)0x89abcdef)
    // #endif
    
    // 表示链表的下一个节点
    dsema->do_next = DISPATCH_OBJECT_LISTLESS;
    
    // 目标队列（从全局的队列数组 _dispatch_root_queues 中取默认队列）
    dsema->do_targetq = _dispatch_get_default_queue(false);
    
    dsema->dsema_value = value; // 当前值（当前是初始值）
    
    // _DSEMA4_POLICY_FIFO 表示先进先出策略吗 ？
    _dispatch_sema4_init(&dsema->dsema_sema, _DSEMA4_POLICY_FIFO);
    
    dsema->dsema_orig = value; // 初始值
    return dsema;
}
```
#### _dispatch_get_default_queue
&emsp;从全局的根队列中取默认 QOS 的队列，当 `overcommit` 为 `true` 时取 `com.apple.root.default-qos.overcommit` 队列，为 `false` 时取 `com.apple.root.default-qos` 队列。
```c++
#define _dispatch_get_default_queue(overcommit) \
        _dispatch_root_queues[DISPATCH_ROOT_QUEUE_IDX_DEFAULT_QOS + \
                !!(overcommit)]._as_dq
```

#### _dispatch_object_alloc
&emsp;为 GCD 对象申请空间并初始化。
```c++
void *
_dispatch_object_alloc(const void *vtable, size_t size)
{
#if OS_OBJECT_HAVE_OBJC1
    const struct dispatch_object_vtable_s *_vtable = vtable;
    dispatch_object_t dou;
    dou._os_obj = _os_object_alloc_realized(_vtable->_os_obj_objc_isa, size);
    dou._do->do_vtable = vtable;
    return dou._do;
#else
    return _os_object_alloc_realized(vtable, size);
#endif
}
```
&emsp;内部调用了 `_os_object_alloc_realized` 函数，下面看一下它的定义。

##### _os_object_alloc_realized
&emsp;核心在 `calloc` 函数申请空间，并赋值。
```c++
inline _os_object_t
_os_object_alloc_realized(const void *cls, size_t size)
{
    _os_object_t obj;
    dispatch_assert(size >= sizeof(struct _os_object_s));
    
    // while 循环只是为了申请空间成功，核心还是在 calloc 函数中
    while (unlikely(!(obj = calloc(1u, size)))) {
        _dispatch_temporary_resource_shortage();
    }
    
    obj->os_obj_isa = cls;
    return obj;
}
```
### dispatch_semaphore_wait
&emsp;`dispatch_semaphore_wait` 等待（减少）信号量。
```c++
long
dispatch_semaphore_wait(dispatch_semaphore_t dsema, dispatch_time_t timeout)
{
    // 原子操作 dsema 的成员变量 dsema_value 的值减 1
    long value = os_atomic_dec2o(dsema, dsema_value, acquire);
    
    // 如果减 1 后仍然大于等于 0，则直接 return 
    if (likely(value >= 0)) {
        return 0;
    }
    
    // 如果小于 0，则调用 _dispatch_semaphore_wait_slow 函数进行阻塞等待
    return _dispatch_semaphore_wait_slow(dsema, timeout);
}
```
&emsp;减少计数信号量，如果结果值小于零，此函数将等待信号出现，然后返回。（可以使总信号量减 1，信号总量小于 0 时就会一直等待（阻塞所在线程），否则就可以正常执行。）`dsema`：信号量，在此参数中传递 `NULL` 的结果是未定义的。`timeout`：何时超时（dispatch_time），为方便起见，有 `DISPATCH_TIME_NOW` 和 `DISPATCH_TIME_FOREVER` 常量。函数返回值 `result`，成功返回零，如果发生超时则返回非零（`_DSEMA4_TIMEOUT`）。

#### os_atomic_dec2o
&emsp;`os_atomic_dec2o` 是对原子操作 -1 的封装。
```c++
#define os_atomic_dec2o(p, f, m) \
        os_atomic_sub2o(p, f, 1, m)
        
#define os_atomic_sub2o(p, f, v, m) \
        os_atomic_sub(&(p)->f, (v), m)
        
#define os_atomic_sub(p, v, m) \
        _os_atomic_c11_op((p), (v), m, sub, -)
        
#define _os_atomic_c11_op(p, v, m, o, op) \
        ({ _os_atomic_basetypeof(p) _v = (v), _r = \
        atomic_fetch_##o##_explicit(_os_atomic_c11_atomic(p), _v, \
        memory_order_##m); (__typeof__(_r))(_r op _v); })
```
#### _dispatch_semaphore_wait_slow
```c++
DISPATCH_NOINLINE
static long
_dispatch_semaphore_wait_slow(dispatch_semaphore_t dsema,
        dispatch_time_t timeout)
{
    long orig;
    
    // 为 &dsema->dsema_sema 赋值
    _dispatch_sema4_create(&dsema->dsema_sema, _DSEMA4_POLICY_FIFO);
    
    switch (timeout) {
    
    // 如果 timeout 是一个特定时间的话调用 _dispatch_sema4_timedwait 进行 timeout 时间的等待
    default:
        if (!_dispatch_sema4_timedwait(&dsema->dsema_sema, timeout)) {
            break;
        }
        // Fall through and try to undo what the fast path did to dsema->dsema_value
        
    // 如果 timeout 参数是 DISPATCH_TIME_NOW
    case DISPATCH_TIME_NOW:
        orig = dsema->dsema_value;
        while (orig < 0) {
        
            // dsema_value 加 1 抵消掉 dispatch_semaphore_wait 函数中的减 1 操作
            if (os_atomic_cmpxchgvw2o(dsema, dsema_value, orig, orig + 1,
                    &orig, relaxed)) {
                // 返回超时
                return _DSEMA4_TIMEOUT();
            }
        }
        // Another thread called semaphore_signal().
        // Fall through and drain the wakeup.
    
    // 如果 timeout 参数是 DISPATCH_TIME_FOREVER 的话调用 _dispatch_sema4_wait 一直等待，直到得到 signal 信号
    case DISPATCH_TIME_FOREVER:
        _dispatch_sema4_wait(&dsema->dsema_sema);
        break;
    }
    
    return 0;
}
```

##### _dispatch_sema4_create
&emsp;`&dsema->dsema_sema` 如果为 NULL 的话则进行赋值。
```c++
DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_sema4_create(_dispatch_sema4_t *sema, int policy)
{
    // #define _dispatch_sema4_is_created(sema)   (*(sema) != MACH_PORT_NULL)
    
    // 如果 sema 为 NULL，则调用 _dispatch_sema4_create_slow 为 sema 赋值
    if (!_dispatch_sema4_is_created(sema)) {
    
        // 从缓存读取或者新建
        _dispatch_sema4_create_slow(sema, policy);
    }
}
```
&emsp;如果 `DISPATCH_USE_OS_SEMAPHORE_CACHE` 为真并且 `policy` 为 `_DSEMA4_POLICY_FIFO`，则调用 `os_get_cached_semaphore` 从缓存中取得一个 `_dispatch_sema4_t` 赋值给 `s4`，否则调用 `semaphore_create` 新建一个 `_dispatch_sema4_t` 赋值给 `s4`。
```c++
void
_dispatch_sema4_create_slow(_dispatch_sema4_t *s4, int policy)
{
    semaphore_t tmp = MACH_PORT_NULL;

    _dispatch_fork_becomes_unsafe();

    // lazily allocate the semaphore port

    // Someday:
    // 1) Switch to a doubly-linked FIFO in user-space.
    // 2) User-space timers for the timeout.

#if DISPATCH_USE_OS_SEMAPHORE_CACHE
    if (policy == _DSEMA4_POLICY_FIFO) {
        tmp = (_dispatch_sema4_t)os_get_cached_semaphore();
        
        // 如果 s4 等于 MACH_PORT_NULL 则把 tmp 赋值给它
        if (!os_atomic_cmpxchg(s4, MACH_PORT_NULL, tmp, relaxed)) {
        
            // 如果 s4 不为 MACH_PORT_NULL 则把它加入缓存
            os_put_cached_semaphore((os_semaphore_t)tmp);
        }
        return;
    }
#endif
    
    // 新建 kern_return_t
    kern_return_t kr = semaphore_create(mach_task_self(), &tmp, policy, 0);
    DISPATCH_SEMAPHORE_VERIFY_KR(kr);

    // 原子赋值
    if (!os_atomic_cmpxchg(s4, MACH_PORT_NULL, tmp, relaxed)) {
        kr = semaphore_destroy(mach_task_self(), tmp);
        DISPATCH_SEMAPHORE_VERIFY_KR(kr);
    }
}
```
##### _dispatch_sema4_wait
&emsp;当 `timeout` 是 `DISPATCH_TIME_FOREVER` 时，do while 循环一直等下去，直到 `sema` 的值被修改为不等于 `KERN_ABORTED`。
```c++
void
_dispatch_sema4_wait(_dispatch_sema4_t *sema)
{
    kern_return_t kr;
    do {
        kr = semaphore_wait(*sema);
    } while (kr == KERN_ABORTED);
    
    DISPATCH_SEMAPHORE_VERIFY_KR(kr);
}
```
##### _dispatch_sema4_timedwait
&emsp;当 `timeout` 是一个指定的时间的话，则循环等待直到超时，或者发出了 `signal` 信号，`sema` 值被修改。
```c++
bool
_dispatch_sema4_timedwait(_dispatch_sema4_t *sema, dispatch_time_t timeout)
{
    mach_timespec_t _timeout;
    kern_return_t kr;

    do {
        // 取时间的差值
        uint64_t nsec = _dispatch_timeout(timeout);
        
        _timeout.tv_sec = (__typeof__(_timeout.tv_sec))(nsec / NSEC_PER_SEC);
        _timeout.tv_nsec = (__typeof__(_timeout.tv_nsec))(nsec % NSEC_PER_SEC);
        
        kr = semaphore_timedwait(*sema, _timeout);
    } while (unlikely(kr == KERN_ABORTED));

    if (kr == KERN_OPERATION_TIMED_OUT) {
        return true;
    }
    
    DISPATCH_SEMAPHORE_VERIFY_KR(kr);
    
    return false;
}
```
&emsp;其中调用了 mach 内核的信号量接口 `semaphore_wait` 和 `semaphore_timedwait` 进行 wait 操作。所以，GCD 的信号量实际上是基于 mach 内核的信号量接口来实现。`semaphore_timedwait` 函数即可以指定超时时间。

### dispatch_semaphore_signal
&emsp;`dispatch_semaphore_signal` 发信号（增加）信号量。如果先前的值小于零，则此函数在返回之前唤醒等待的线程。如果线程被唤醒，此函数将返回非零值。否则，返回零。
```c++
long
dispatch_semaphore_signal(dispatch_semaphore_t dsema)
{
    // 原子操作 dsema 的成员变量 dsema_value 的值加 1
    long value = os_atomic_inc2o(dsema, dsema_value, release);
    
    if (likely(value > 0)) {
        // 如果 value 大于 0 表示目前没有线程需要唤醒，直接 return 0
        return 0;
    }
    
    // 如果过度释放，导致 value 的值一直增加到 LONG_MIN（溢出），则 crash 
    if (unlikely(value == LONG_MIN)) {
        DISPATCH_CLIENT_CRASH(value,
                "Unbalanced call to dispatch_semaphore_signal()");
    }
    
    // value 小于等于 0 时，表示目前有线程需要唤醒
    return _dispatch_semaphore_signal_slow(dsema);
}
```
&emsp;测试 `value == LONG_MIN` 并没有发生 crash。
```c++
dispatch_semaphore_t semaphore = dispatch_semaphore_create(LONG_MAX);
dispatch_semaphore_signal(semaphore);

// 控制台打印
(lldb) po semaphore
<OS_dispatch_semaphore: semaphore[0x600000ed79d0] = { xref = 1, ref = 1, port = 0x0, value = 9223372036854775807, orig = 9223372036854775807 }>

(lldb) po semaphore
<OS_dispatch_semaphore: semaphore[0x600000ed79d0] = { xref = 1, ref = 1, port = 0x0, value = -9223372036854775808, orig = 9223372036854775807 }> ⬅️ value 的值是 LONG_MIN，并没有 crash
```
#### os_atomic_inc2o
&emsp;`os_atomic_inc2o` 是对原子操作 +1 的封装。
```c++
#define os_atomic_inc2o(p, f, m) \
        os_atomic_add2o(p, f, 1, m)

#define os_atomic_add2o(p, f, v, m) \
        os_atomic_add(&(p)->f, (v), m)
        
#define os_atomic_add(p, v, m) \
        _os_atomic_c11_op((p), (v), m, add, +)  
        
#define _os_atomic_c11_op(p, v, m, o, op) \
        ({ _os_atomic_basetypeof(p) _v = (v), _r = \
        atomic_fetch_##o##_explicit(_os_atomic_c11_atomic(p), _v, \
        memory_order_##m); (__typeof__(_r))(_r op _v); })
```
#### _dispatch_semaphore_signal_slow
&emsp;内部调用 `_dispatch_sema4_signal(&dsema->dsema_sema, 1)` 唤醒一条线程。
```c++
DISPATCH_NOINLINE
long
_dispatch_semaphore_signal_slow(dispatch_semaphore_t dsema)
{
    _dispatch_sema4_create(&dsema->dsema_sema, _DSEMA4_POLICY_FIFO);
    
    // count 传 1，唤醒一条线程
    _dispatch_sema4_signal(&dsema->dsema_sema, 1);
    return 1;
}
```
##### _dispatch_sema4_signal
&emsp;`semaphore_signal` 能够唤醒一个在 `semaphore_wait` 中等待的线程。如果有多个等待线程，则根据线程优先级来唤醒。
```c++
void
_dispatch_sema4_signal(_dispatch_sema4_t *sema, long count)
{
    do {
        // semaphore_signal 唤醒线程
        kern_return_t kr = semaphore_signal(*sema);
        DISPATCH_SEMAPHORE_VERIFY_KR(kr);
    } while (--count);
}
```
&emsp;下面看一下 semaphore.c 文件中与信号量有关的最后一个函数。

### _dispatch_semaphore_dispose
&emsp;信号量的销毁函数。
```c++
void
_dispatch_semaphore_dispose(dispatch_object_t dou,
        DISPATCH_UNUSED bool *allow_free)
{
    dispatch_semaphore_t dsema = dou._dsema;

    // 容错判断，如果当前 dsema_value 小于 dsema_orig，表示信号量还正在使用，不能进行销毁，如下代码会导致此 crash 
    // dispatch_semaphore_t sema = dispatch_semaphore_create(1); // 创建 value = 1，orig = 1
    // dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER); // value = 0，orig = 1
    // sema = dispatch_semaphore_create(1); // 赋值导致原始 dispatch_semaphore_s 释放，但是此时 orig 是 1，value 是 0 则直接 crash
    
    if (dsema->dsema_value < dsema->dsema_orig) {
        DISPATCH_CLIENT_CRASH(dsema->dsema_orig - dsema->dsema_value,
                "Semaphore object deallocated while in use");
    }

    _dispatch_sema4_dispose(&dsema->dsema_sema, _DSEMA4_POLICY_FIFO);
}
```
#### _dispatch_sema4_dispose
```c++
DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_sema4_dispose(_dispatch_sema4_t *sema, int policy)
{
    // 如果 sema 存在则调用 _dispatch_sema4_dispose_slow 函数
    if (_dispatch_sema4_is_created(sema)) {
        _dispatch_sema4_dispose_slow(sema, policy);
    }
}
```
##### _dispatch_sema4_dispose_slow
&emsp;如果 `DISPATCH_USE_OS_SEMAPHORE_CACHE` 为真并且 `policy` 为 `_DSEMA4_POLICY_FIFO`，则调用 `os_put_cached_semaphore` 把 `sema` 放入缓存中，否则，调用 mach 内核的 `semaphore_destroy` 函数进行信号量的销毁。
```c++
void
_dispatch_sema4_dispose_slow(_dispatch_sema4_t *sema, int policy)
{
    semaphore_t sema_port = *sema;
    *sema = MACH_PORT_DEAD;
    
#if DISPATCH_USE_OS_SEMAPHORE_CACHE
    if (policy == _DSEMA4_POLICY_FIFO) {
    
        // 放入缓存
        return os_put_cached_semaphore((os_semaphore_t)sema_port);
    }
#endif

    // 调用 semaphore_destroy 销毁
    kern_return_t kr = semaphore_destroy(mach_task_self(), sema_port);
    DISPATCH_SEMAPHORE_VERIFY_KR(kr);
}
```
&emsp;到这里信号量相关的常用 API 的源码就看完了，主要根据 `dsema_value` 来进行阻塞和唤醒线程。

## 参考链接
**参考链接:🔗**
+ [libdispatch苹果源码](https://opensource.apple.com/tarballs/libdispatch/)
+ [GCD源码分析1 —— 开篇](http://lingyuncxb.com/2018/01/31/GCD源码分析1%20——%20开篇/)
+ [扒了扒libdispatch源码](http://joeleee.github.io/2017/02/21/005.扒了扒libdispatch源码/)
+ [GCD源码分析](https://developer.aliyun.com/article/61328)
+ [关于GCD开发的一些事儿](https://www.jianshu.com/p/f9e01c69a46f)
+ [GCD 深入理解：第一部分](https://github.com/nixzhu/dev-blog/blob/master/2014-04-19-grand-central-dispatch-in-depth-part-1.md)
+ [dispatch_once 详解](https://www.jianshu.com/p/4fd27f1db63d)
+ [透明联合类型](http://nanjingabcdefg.is-programmer.com/posts/23951.html)
+ [变态的libDispatch结构分析-dispatch_object_s](https://blog.csdn.net/passerbysrs/article/details/18228333?utm_source=blogxgwz2)
+ [深入浅出 GCD 之基础篇](https://xiaozhuanlan.com/topic/9168375240)
+ [从源码分析Swift多线程—DispatchGroup](http://leevcan.com/2020/05/30/从源码分析Swift多线程—DispatchGroup/)
+ [GCD源码分析（一）](https://www.jianshu.com/p/bd629d25dc2e)
+ [GCD-源码分析](https://www.jianshu.com/p/866b6e903a2d)
+ [GCD底层源码分析](https://www.jianshu.com/p/4ef55563cd14)
+ [GCD源码吐血分析(1)——GCD Queue](https://blog.csdn.net/u013378438/article/details/81031938)
+ [c/c++:计算可变参数宏 __VA_ARGS__ 的参数个数](https://blog.csdn.net/10km/article/details/80760533)
+ [OC底层探索(二十一)GCD异步、GCD同步、单例、信号量、调度组、栅栏函数等底层分析](https://blog.csdn.net/weixin_40918107/article/details/109520980)
+ [iOS源码解析: GCD的信号量semaphore](https://juejin.cn/post/6844904122370490375)

