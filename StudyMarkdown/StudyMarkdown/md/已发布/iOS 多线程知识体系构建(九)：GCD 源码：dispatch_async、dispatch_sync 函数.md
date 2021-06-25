# iOS 多线程知识体系构建(九)：GCD 源码：dispatch_async、dispatch_sync 函数执行

> &emsp;上一篇我们学习了队列的创建，本篇我们开始学习 GCD  中的相关函数，首先从我们用的最多的 dispatch_async 和 dispatch_sync 开始。

&emsp;GCD 函数阅读过程中会涉及多个由大量宏定义组成的结构体的定义，需要一步一步进行宏展开才能更好的理解代码。

## dispatch_async
&emsp;当我们向队列提交任务时，无论 block 还是 function 形式，最终都会被封装为 `dispatch_continuation_s`，所以可以把它理解为描述任务内容的结构体，`dispatch_async` 函数内部会首先创建 `dispatch_continuation_s` 结构体。

&emsp;首先我们要知道一点，不管是用 `dispatch_async` 向队列中异步提交 block，还是用 `dispatch_async_f` 向队列中异步提交函数，都会把提交的任务包装成 `dispatch_continuation_s`，而在 `dispatch_continuation_s` 结构体中是使用一个函数指针（`dc_func`）来存储要执行的任务的，当提交的是 block 任务时 `dispatch_continuation_s` 内部存储的是 block 结构体定义的函数，而不是 block 本身。

&emsp;`dispatch_async` 函数在逻辑上可以分为两个部分，第一部分对 `work` 函数封装（`_dispatch_continuation_init`），第二部分是对线程的调用（`_dispatch_continuation_async`）。
```c++
void
dispatch_async(dispatch_queue_t dq, dispatch_block_t work)
{
    // 取得一个 dispatch_continuation_s (从缓存中获取或者新建)
    dispatch_continuation_t dc = _dispatch_continuation_alloc(); // 1⃣️ 
    
    // DC_FLAG_CONSUME 标记 dc 是设置在异步中的源程序（即表明 dispatch_continuation_s 是一个在异步中执行的任务）
    // #define DC_FLAG_CONSUME   0x004ul
    uintptr_t dc_flags = DC_FLAG_CONSUME; // 2⃣️
    
    // dispatch_qos_t 是一个 uint32_t 类型别名
    dispatch_qos_t qos;
    
    // 对 dispatch_continuations_s 结构体变量进行一系列初始化和配置
    qos = _dispatch_continuation_init(dc, dq, work, 0, dc_flags); // 3⃣️
    
    // 看到 dispatch_continuations_s 结构变量准备完毕后会嵌套调用 _dispatch_continuation_async 函数
    _dispatch_continuation_async(dq, dc, qos, dc->dc_flags);
}
```
&emsp;下面我们先分析一下 `dispatch_async` 函数内部涉及到的 `dispatch_continuation_s` 结构体变量的初始化和配置部分，然后再深入 `_dispatch_continuation_async` 函数学习，探究我们最常用的异步调用函数到底是怎么实现的。

&emsp;`dispatch_continuation_s` 结构体定义中内部使用的宏定义展开如下：
```c++
typedef struct dispatch_continuation_s {
    union {
        const void *do_vtable;
        uintptr_t dc_flags;
    };
    
    union {
        pthread_priority_t dc_priority;
        int dc_cache_cnt;
        uintptr_t dc_pad;
    };
    
    struct dispatch_continuation_s *volatile do_next; // 下一个任务
    struct voucher_s *dc_voucher;
    
    // typedef void (*dispatch_function_t)(void *_Nullable);
    
    dispatch_function_t dc_func; // 要执行的函数指针
    void *dc_ctxt; // 方法的上下文（参数）
    void *dc_data; // 相关数据
    void *dc_other; // 其它信息 
} *dispatch_continuation_t;
```
### _dispatch_continuation_alloc
&emsp;`_dispatch_continuation_alloc` 函数内部首先调用 `_dispatch_continuation_alloc_cacheonly` 函数从缓存中找 `dispatch_continuation_t`，如果找不到则调用 `_dispatch_continuation_alloc_from_heap` 函数在堆区新建一个 `dispatch_continuation_s`。
```c++
DISPATCH_ALWAYS_INLINE
static inline dispatch_continuation_t
_dispatch_continuation_alloc(void)
{
    dispatch_continuation_t dc =
            _dispatch_continuation_alloc_cacheonly();
    // 如果缓存中不存在则在堆区新建 dispatch_continuation_s
    if (unlikely(!dc)) {
        return _dispatch_continuation_alloc_from_heap();
    }
    return dc;
}
```
#### _dispatch_continuation_alloc_cacheonly
&emsp;`_dispatch_continuation_alloc_cacheonly` 函数内部调用 `_dispatch_thread_getspecific` 函数从当前线程获取根据 `dispatch_cache_key` 作为 key 保存的 `dispatch_continuation_t` 赋值给 `dc`，然后把 `dc` 的 `do_next` 作为新的 value 调用 `_dispatch_thread_setspecific` 函数保存在当前线程的存储空间中。（即更新当前缓存中可用的 `dispatch_continuation_t`）
```c++
DISPATCH_ALWAYS_INLINE
static inline dispatch_continuation_t
_dispatch_continuation_alloc_cacheonly(void)
{
    dispatch_continuation_t dc = (dispatch_continuation_t)
            _dispatch_thread_getspecific(dispatch_cache_key);

    // 更新 dispatch_cache_key 作为 key 保存在线程存储空间中的值
    if (likely(dc)) {
        _dispatch_thread_setspecific(dispatch_cache_key, dc->do_next);
    }
    return dc;
}

#define _dispatch_thread_getspecific(key) \
    (_dispatch_get_tsd_base()->key)
#define _dispatch_thread_setspecific(key, value) \
    (void)(_dispatch_get_tsd_base()->key = (value))
```
### DC_FLAG
&emsp;`DC_FLAG_CONSUME`：continuation resources 在运行时释放，表示 dispatch_continuation_s 是设置在异步或 non event_handler 源处理程序。
```c++
#define DC_FLAG_CONSUME   0x004ul
```
&emsp;·`DC_FLAG_BLOCK`：continuation function 是一个 block。
```c++
#define DC_FLAG_BLOCK   0x010ul
```
&emsp;`DC_FLAG_ALLOCATED`：bit 用于确保分配的 continuations 的  dc_flags 永远不会为 0。
```c++
#define DC_FLAG_ALLOCATED   0x100ul
```
&emsp;`dispatch_qos_t` 是一个 `uint32_t` 类型别名。
```c++
typedef uint32_t dispatch_qos_t;
```
### _dispatch_continuation_init
&emsp;`_dispatch_continuation_init` 函数是根据入参对 `dc` 进行初始化。
```c++
DISPATCH_ALWAYS_INLINE
static inline dispatch_qos_t
_dispatch_continuation_init(dispatch_continuation_t dc,
        dispatch_queue_class_t dqu, dispatch_block_t work,
        dispatch_block_flags_t flags, uintptr_t dc_flags)
{
    // 把入参 block 复制到堆区（内部是调用了 Block_copy 函数）
    void *ctxt = _dispatch_Block_copy(work);
    
    // 入参 dc_flags 是 DC_FLAG_CONSUME（0x004ul）
    // #define DC_FLAG_BLOCK   0x010ul
    // #define DC_FLAG_ALLOCATED   0x100ul
    
    // 即 dc_flags 等于 0x114ul，把上面的三个枚举合并在一起
    dc_flags |= DC_FLAG_BLOCK | DC_FLAG_ALLOCATED;
    
    // 判断 work block 的函数是否等于一个外联的函数指针
    if (unlikely(_dispatch_block_has_private_data(work))) {
        dc->dc_flags = dc_flags;
        dc->dc_ctxt = ctxt;
        // will initialize all fields but requires dc_flags & dc_ctxt to be set
        
        // 执行 dispatch_continuation_s 的慢速初始化
        return _dispatch_continuation_init_slow(dc, dqu, flags);
    }

    // 取出 block 的函数指针（将 work 的函数封装成 dispatch_function_t）
    dispatch_function_t func = _dispatch_Block_invoke(work);
    
    // dispatch_async 函数内部 dc_flags 参数默认值是 DC_FLAG_CONSUME
    if (dc_flags & DC_FLAG_CONSUME) {
        // _dispatch_call_block_and_release 调用并释放 block
        
        // 此时 _dispatch_call_block_and_release 就是 work
        func = _dispatch_call_block_and_release;
    }
    
    // 然后再进一步的设置 dispatch_continuation_s 结构体变量的一系列成员变量，（主要包括 voucher 和 priority）
    
    // ctxt 是指向堆区的 work 副本的指针
    // func 是函数
    return _dispatch_continuation_init_f(dc, dqu, ctxt, func, flags, dc_flags);
}
```
#### _dispatch_Block_copy
&emsp;`_dispatch_Block_copy` 内部调用 `Block_copy` 函数，把栈区 block 复制到堆区，或者堆区 block 引用加 1。
```c++
void *
(_dispatch_Block_copy)(void *db)
{
    dispatch_block_t rval;

    if (likely(db)) {
        while (unlikely(!(rval = Block_copy(db)))) {
            // 保证 block 复制成功
            _dispatch_temporary_resource_shortage();
        }
        return rval;
    }
    DISPATCH_CLIENT_CRASH(0, "NULL was passed where a block should have been");
}

DISPATCH_NOINLINE
void
_dispatch_temporary_resource_shortage(void)
{
    sleep(1);
    __asm__ __volatile__("");  // prevent tailcall
}
```
#### _dispatch_Block_invoke
&emsp;如果熟悉 block 内部的构造的话可知 `invoke` 是一个指向 block 要执行的函数的函数指针。（在我们定义 block 时，用 { } 扩起的所有表达式内容会构成一个完整的函数，它就是 block 要执行的函数，而这个 block 结构体中的 `invoke` 指针正是指向这个函数）

&emsp;`_dispatch_Block_invoke` 是一个宏定义，即取得 block 结构体中的 `invoke` 成员变量。
```c++
typedef void(*BlockInvokeFunction)(void *, ...);
struct Block_layout {
    ...
    // 函数指针，指向 block 要执行的函数（即 block 定义中花括号中的表达式）
    BlockInvokeFunction invoke;
    ...
};

#define _dispatch_Block_invoke(bb) \
        ((dispatch_function_t)((struct Block_layout *)bb)->invoke)
```
#### _dispatch_block_has_private_data
&emsp;判断 `block` 的 `invoke` 函数指针指向是否是 `_dispatch_block_special_invoke`（一个外联的函数指针）。
```c++
DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_block_has_private_data(const dispatch_block_t block)
{
    return (_dispatch_Block_invoke(block) == _dispatch_block_special_invoke);
}
```
#### _dispatch_block_special_invoke
&emsp;一个外联的函数指针。
```c++
extern "C" {
// The compiler hides the name of the function it generates, and changes it if we try to reference it directly, but the linker still sees it.
extern void DISPATCH_BLOCK_SPECIAL_INVOKE(void *)
        __asm__(OS_STRINGIFY(__USER_LABEL_PREFIX__) "___dispatch_block_create_block_invoke");
void (*const _dispatch_block_special_invoke)(void*) = DISPATCH_BLOCK_SPECIAL_INVOKE;
}
```
#### _dispatch_call_block_and_release
&emsp;执行一个 block 然后释放 block。
```c++
void
_dispatch_call_block_and_release(void *block)
{
    void (^b)(void) = block;
    b(); // block 执行
    Block_release(b); // block 释放
}
```
#### _dispatch_continuation_init_f
&emsp;配置 `dispatch_continuation_s` 结构体变量的成员变量，将传入的参数对 `dc` 进行赋值，并执行 `_dispatch_continuation_voucher_set` 和 `_dispatch_continuation_priority_set` 函数。
```c++
DISPATCH_ALWAYS_INLINE
static inline dispatch_qos_t
_dispatch_continuation_init_f(dispatch_continuation_t dc,
        dispatch_queue_class_t dqu, void *ctxt, dispatch_function_t f,
        dispatch_block_flags_t flags, uintptr_t dc_flags)
{
    pthread_priority_t pp = 0;
    dc->dc_flags = dc_flags | DC_FLAG_ALLOCATED; // dc_flags 再添加一个新标记 #define DC_FLAG_ALLOCATED 0x100ul
    dc->dc_func = f; // 待执行的函数
    dc->dc_ctxt = ctxt;
    
    // in this context DISPATCH_BLOCK_HAS_PRIORITY means that the priority should not be propagated, only taken from the handler if it has one
    // 在此上下文中 DISPATCH_BLOCK_HAS_PRIORITY，这意味着不应传播优先级，只有在处理程序具有一个
    if (!(flags & DISPATCH_BLOCK_HAS_PRIORITY)) {
        pp = _dispatch_priority_propagate();
    }
    
    // 配置 voucher
    _dispatch_continuation_voucher_set(dc, flags);
    // 配置 priority
    return _dispatch_continuation_priority_set(dc, dqu, pp, flags);
}
```
&emsp;到这里任务（`dispatch_continuation_s`）的封装就完成了，下面看一下 `_dispatch_continuation_async` 函数的内容。

## _dispatch_continuation_async
&emsp;`dispatch_async` 函数内部把 `dispatch_continuation_s` 结构体变量准备好后调用 `_dispatch_continuation_async(dq, dc, qos, dc->dc_flags)`。
```c++
DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_continuation_async(dispatch_queue_class_t dqu,
        dispatch_continuation_t dc, dispatch_qos_t qos, uintptr_t dc_flags)
{
#if DISPATCH_INTROSPECTION
    if (!(dc_flags & DC_FLAG_NO_INTROSPECTION)) {
        _dispatch_trace_item_push(dqu, dc);
    }
#else
    (void)dc_flags;
#endif

    // 调用队列的 dq_push 函数
    return dx_push(dqu._dq, dc, qos);
}

// dx_push 是一个宏定义
#define dx_push(x, y, z) dx_vtable(x)->dq_push(x, y, z)

void (*const dq_push)(dispatch_queue_class_t, dispatch_object_t, dispatch_qos_t)
```
&emsp;`_dispatch_continuation_async` 函数内部使用了一个宏定义：`dx_push`，宏定义的内容是调用 `dqu`（`dispatch_queue_class_t`）的 vtable 的 `dq_push`（dq_push 是一个函数指针，是作为 vtable 的属性存在的，那么它是何时进行赋值的呢？）。

&emsp;全局搜索 `dq_push` 发现存在多处不同的队列进行赋值，例如根队列（`.dq_push = _dispatch_root_queue_push`）、主队列（`.dq_push = _dispatch_main_queue_push`）、并发队列（`.dq_push = _dispatch_lane_concurrent_push`）、串行队列（`.dq_push = _dispatch_lane_push`）等等，由于我们的自定义队列都是以根队列作为目标队列（任务大都是在根队列执行的），所以我们这里以 `_dispatch_root_queue_push` 为例进行学习。

### _dispatch_root_queue_push
&emsp;`_dispatch_root_queue_push` 函数内部最终调用了 `_dispatch_root_queue_push_inline(rq, dou, dou, 1)` 函数。
```c++
DISPATCH_NOINLINE
void
_dispatch_root_queue_push(dispatch_queue_global_t rq, dispatch_object_t dou,
        dispatch_qos_t qos)
{
#if DISPATCH_USE_KEVENT_WORKQUEUE
    dispatch_deferred_items_t ddi = _dispatch_deferred_items_get();
    if (unlikely(ddi && ddi->ddi_can_stash)) {
        dispatch_object_t old_dou = ddi->ddi_stashed_dou;
        dispatch_priority_t rq_overcommit;
        rq_overcommit = rq->dq_priority & DISPATCH_PRIORITY_FLAG_OVERCOMMIT;

        if (likely(!old_dou._do || rq_overcommit)) {
            dispatch_queue_global_t old_rq = ddi->ddi_stashed_rq;
            dispatch_qos_t old_qos = ddi->ddi_stashed_qos;
            ddi->ddi_stashed_rq = rq;
            ddi->ddi_stashed_dou = dou;
            ddi->ddi_stashed_qos = qos;
            _dispatch_debug("deferring item %p, rq %p, qos %d",
                    dou._do, rq, qos);
            if (rq_overcommit) {
                ddi->ddi_can_stash = false;
            }
            if (likely(!old_dou._do)) {
                return;
            }
            // push the previously stashed item
            qos = old_qos;
            rq = old_rq;
            dou = old_dou;
        }
    }
#endif
#if HAVE_PTHREAD_WORKQUEUE_QOS
    if (_dispatch_root_queue_push_needs_override(rq, qos)) {
        return _dispatch_root_queue_push_override(rq, dou, qos);
    }
#else
    (void)qos;
#endif
    // 调用内联函数
    _dispatch_root_queue_push_inline(rq, dou, dou, 1);
}
```
### _dispatch_root_queue_push_inline
&emsp;`_dispatch_root_queue_push_inline` 内部则是调用 `_dispatch_root_queue_poke` 函数。
```c++
DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_root_queue_push_inline(dispatch_queue_global_t dq,
        dispatch_object_t _head, dispatch_object_t _tail, int n)
{
    struct dispatch_object_s *hd = _head._do, *tl = _tail._do;
    if (unlikely(os_mpsc_push_list(os_mpsc(dq, dq_items), hd, tl, do_next))) {
    
        // _dispatch_root_queue_poke
        return _dispatch_root_queue_poke(dq, n, 0);
    }
}
```
### _dispatch_root_queue_poke
&emsp;`_dispatch_root_queue_poke` 函数主要进行了一些容错判断，然后调用 `_dispatch_root_queue_poke_slow` 函数。
```c++
DISPATCH_NOINLINE
void
_dispatch_root_queue_poke(dispatch_queue_global_t dq, int n, int floor)
{
    if (!_dispatch_queue_class_probe(dq)) {
        return;
    }
#if !DISPATCH_USE_INTERNAL_WORKQUEUE
#if DISPATCH_USE_PTHREAD_POOL
    if (likely(dx_type(dq) == DISPATCH_QUEUE_GLOBAL_ROOT_TYPE))
#endif
    {
        if (unlikely(!os_atomic_cmpxchg2o(dq, dgq_pending, 0, n, relaxed))) {
            _dispatch_root_queue_debug("worker thread request still pending "
                    "for global queue: %p", dq);
            return;
        }
    }
#endif // !DISPATCH_USE_INTERNAL_WORKQUEUE

    // 上面是一些容错处理，主要是这里调用 _dispatch_root_queue_poke_slow 函数
    return _dispatch_root_queue_poke_slow(dq, n, floor);
}
```
### _dispatch_root_queue_poke_slow
```c++
DISPATCH_NOINLINE
static void
_dispatch_root_queue_poke_slow(dispatch_queue_global_t dq, int n, int floor)
{
    // n = 1
    int remaining = n;
    int r = ENOSYS;

    // 注册回调（内部调用了 dispatch_once_f，全局只会只会执行一次）
    _dispatch_root_queues_init();
    
    // DEGBUG 模式时的打印 __func__ 函数执行
    _dispatch_debug_root_queue(dq, __func__);
    
    // hook
    _dispatch_trace_runtime_event(worker_request, dq, (uint64_t)n);

#if !DISPATCH_USE_INTERNAL_WORKQUEUE
#if DISPATCH_USE_PTHREAD_ROOT_QUEUES
    if (dx_type(dq) == DISPATCH_QUEUE_GLOBAL_ROOT_TYPE)
#endif
    {
        _dispatch_root_queue_debug("requesting new worker thread for global "
                "queue: %p", dq);
        r = _pthread_workqueue_addthreads(remaining,
                _dispatch_priority_to_pp_prefer_fallback(dq->dq_priority));
        (void)dispatch_assume_zero(r);
        return;
    }
#endif // !DISPATCH_USE_INTERNAL_WORKQUEUE
#if DISPATCH_USE_PTHREAD_POOL
    dispatch_pthread_root_queue_context_t pqc = dq->do_ctxt;
    if (likely(pqc->dpq_thread_mediator.do_vtable)) {
        while (dispatch_semaphore_signal(&pqc->dpq_thread_mediator)) {
            _dispatch_root_queue_debug("signaled sleeping worker for "
                    "global queue: %p", dq);
            if (!--remaining) {
                return;
            }
        }
    }

    bool overcommit = dq->dq_priority & DISPATCH_PRIORITY_FLAG_OVERCOMMIT;
    if (overcommit) {
        os_atomic_add2o(dq, dgq_pending, remaining, relaxed);
    } else {
        if (!os_atomic_cmpxchg2o(dq, dgq_pending, 0, remaining, relaxed)) {
            _dispatch_root_queue_debug("worker thread request still pending for "
                    "global queue: %p", dq);
            return;
        }
    }

    int can_request, t_count;
    // seq_cst with atomic store to tail <rdar://problem/16932833>
    
    // 获取线程池的大小
    t_count = os_atomic_load2o(dq, dgq_thread_pool_size, ordered);
    do {
        // 计算可以请求的数量
        can_request = t_count < floor ? 0 : t_count - floor;
        if (remaining > can_request) {
            _dispatch_root_queue_debug("pthread pool reducing request from %d to %d",
                    remaining, can_request);
            os_atomic_sub2o(dq, dgq_pending, remaining - can_request, relaxed);
            remaining = can_request;
        }
        
        if (remaining == 0) {
            // 线程池无可用将会报错
            _dispatch_root_queue_debug("pthread pool is full for root queue: "
                    "%p", dq);
            return;
        }
    } while (!os_atomic_cmpxchgvw2o(dq, dgq_thread_pool_size, t_count,
            t_count - remaining, &t_count, acquire));

#if !defined(_WIN32)
    pthread_attr_t *attr = &pqc->dpq_thread_attr;
    pthread_t tid, *pthr = &tid;
#if DISPATCH_USE_MGR_THREAD && DISPATCH_USE_PTHREAD_ROOT_QUEUES
    if (unlikely(dq == &_dispatch_mgr_root_queue)) {
        pthr = _dispatch_mgr_root_queue_init();
    }
#endif
    do {
        _dispatch_retain(dq); // released in _dispatch_worker_thread
        
        // 开辟线程 pthread_create
        while ((r = pthread_create(pthr, attr, _dispatch_worker_thread, dq))) {
            if (r != EAGAIN) {
                (void)dispatch_assume_zero(r);
            }
            _dispatch_temporary_resource_shortage();
        }
        
    } while (--remaining);
#else // defined(_WIN32)
#if DISPATCH_USE_MGR_THREAD && DISPATCH_USE_PTHREAD_ROOT_QUEUES
    if (unlikely(dq == &_dispatch_mgr_root_queue)) {
        _dispatch_mgr_root_queue_init();
    }
#endif
    do {
        _dispatch_retain(dq); // released in _dispatch_worker_thread
#if DISPATCH_DEBUG
        unsigned dwStackSize = 0;
#else
        unsigned dwStackSize = 64 * 1024;
#endif
        uintptr_t hThread = 0;
        while (!(hThread = _beginthreadex(NULL, dwStackSize, _dispatch_worker_thread_thunk, dq, STACK_SIZE_PARAM_IS_A_RESERVATION, NULL))) {
            if (errno != EAGAIN) {
                (void)dispatch_assume(hThread);
            }
            _dispatch_temporary_resource_shortage();
        }
        if (_dispatch_mgr_sched.prio > _dispatch_mgr_sched.default_prio) {
            (void)dispatch_assume_zero(SetThreadPriority((HANDLE)hThread, _dispatch_mgr_sched.prio) == TRUE);
        }
        CloseHandle((HANDLE)hThread);
    } while (--remaining);
#endif // defined(_WIN32)
#else
    (void)floor;
#endif // DISPATCH_USE_PTHREAD_POOL
}
```
&emsp;根据代码可以知道，系统会获取线程池总数量和可以创建的数量，然后通过两个 `do while` 来进行动态的开辟线程。

&emsp;完整的函数调用栈大概精简如下：`dispatch_async` ➡️ `_dispatch_continuation_init` ➡️ `_dispatch_continuation_async` ➡️ `dx_push` ➡️ `dq_push`（`_dispatch_root_queue_push`）➡️ `_dispatch_root_queue_push_inline` ➡️ `_dispatch_root_queue_poke` ➡️ `_dispatch_root_queue_poke_slow` ➡️ `pthread_create`。

## dispatch_sync
&emsp;下面学习一下 `dispatch_sync` 函数的执行流程。
```c++
DISPATCH_NOINLINE
void
dispatch_sync(dispatch_queue_t dq, dispatch_block_t work)
{
    // #define DC_FLAG_BLOCK   0x010ul
    uintptr_t dc_flags = DC_FLAG_BLOCK;
    if (unlikely(_dispatch_block_has_private_data(work))) {
        return _dispatch_sync_block_with_privdata(dq, work, dc_flags);
    }
    
    // work block
    // _dispatch_Block_invoke(work) 的函数
    _dispatch_sync_f(dq, work, _dispatch_Block_invoke(work), dc_flags);
}
```
&emsp;`dispatch_sync` 函数内部调用 `_dispatch_sync_f` 函数。
### _dispatch_sync_f
&emsp;`_dispatch_sync_f` 内部仅有一行直接调用 `_dispatch_sync_f_inline` 函数。
```c++
DISPATCH_NOINLINE
static void
_dispatch_sync_f(dispatch_queue_t dq, void *ctxt, dispatch_function_t func, uintptr_t dc_flags)
{
    _dispatch_sync_f_inline(dq, ctxt, func, dc_flags);
}
```
### _dispatch_sync_f_inline
&emsp;`_dispatch_sync_f_inline` 函数实现中开局是一个 `dq->dq_width == 1` 的判断，上篇队列创建中我们知道串行队列的 `dq_width` 值为 1，自定义的并发队列的 `dq_width` 值为 `0xffeull`，根队列的 `dq_width` 值是 `0xfffull`，即如果 `dq` 参数是串行队列的话会执行 `_dispatch_barrier_sync_f(dq, ctxt, func, dc_flags)`，如果 `dq` 参数是并发队列的话，会执行接下来的函数。

```c++
DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_sync_f_inline(dispatch_queue_t dq, void *ctxt,
        dispatch_function_t func, uintptr_t dc_flags)
{
    // 学习上篇时我们知道串行队列的 dq_width 值为 1
    
    if (likely(dq->dq_width == 1)) {
        return _dispatch_barrier_sync_f(dq, ctxt, func, dc_flags);
    }
    
    // _DISPATCH_LANE_TYPE = 0x00000011, // meta-type for lanes
    // _DISPATCH_META_TYPE_MASK = 0x000000ff, // mask for object meta-types
    // #define dx_metatype(x) (dx_vtable(x)->do_type & _DISPATCH_META_TYPE_MASK)
    
    if (unlikely(dx_metatype(dq) != _DISPATCH_LANE_TYPE)) {
        DISPATCH_CLIENT_CRASH(0, "Queue type doesn't support dispatch_sync");
    }
    
    dispatch_lane_t dl = upcast(dq)._dl;
    
    // Global concurrent queues and queues bound to non-dispatch threads always fall into the slow case, see DISPATCH_ROOT_QUEUE_STATE_INIT_VALUE
    
    // 绑定到非调度线程（non-dispatch threads）的全局并发队列（global concurrent queues）和队列始终属于缓慢情况（slow case）
    if (unlikely(!_dispatch_queue_try_reserve_sync_width(dl))) {
        return _dispatch_sync_f_slow(dl, ctxt, func, 0, dl, dc_flags);
    }

    if (unlikely(dq->do_targetq->do_targetq)) {
        return _dispatch_sync_recurse(dl, ctxt, func, dc_flags);
    }
    
    _dispatch_introspection_sync_begin(dl);
    
    _dispatch_sync_invoke_and_complete(dl, ctxt, func DISPATCH_TRACE_ARG(
            _dispatch_trace_item_sync_push_pop(dq, ctxt, func, dc_flags)));
}
```
#### upcast
&emsp;入参转化为 `dispatch_object_t` 类型。
```c++
DISPATCH_ALWAYS_INLINE
static inline dispatch_object_t
upcast(dispatch_object_t dou)
{
    return dou;
}
```

#### _dispatch_barrier_sync_f
&emsp;`_dispatch_barrier_sync_f` 函数内部也是仅调用了 `_dispatch_barrier_sync_f_inline` 函数。
```c++
DISPATCH_NOINLINE
static void
_dispatch_barrier_sync_f(dispatch_queue_t dq, void *ctxt,
        dispatch_function_t func, uintptr_t dc_flags)
{
    _dispatch_barrier_sync_f_inline(dq, ctxt, func, dc_flags);
}
```

##### _dispatch_barrier_sync_f_inline
```c++
DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_barrier_sync_f_inline(dispatch_queue_t dq, void *ctxt,
        dispatch_function_t func, uintptr_t dc_flags)
{
    // 获取当前线程的 ID（TLS 技术保存在线程的存储空间内）
    dispatch_tid tid = _dispatch_tid_self();

    if (unlikely(dx_metatype(dq) != _DISPATCH_LANE_TYPE)) {
        DISPATCH_CLIENT_CRASH(0, "Queue type doesn't support dispatch_sync");
    }

    dispatch_lane_t dl = upcast(dq)._dl;
    
    // The more correct thing to do would be to merge the qos of the
    // thread that just acquired the barrier lock into the queue state.
    // （更加正确的做法是将刚获得屏障锁的线程的质量合并到队列状态。）
    //
    // However this is too expensive for the fast path, so skip doing it.
    // The chosen tradeoff is that if an enqueue on a lower priority thread
    // contends with this fast path, this thread may receive a useless override.
    //
    // 但是，这对于快速路径而言太昂贵了，因此请跳过此步骤。
    // 选择的权衡是，如果优先级较低的线程上的队列与此快速路径竞争，则此线程可能会收到无用的覆盖。
    // 
    // Global concurrent queues and queues bound to non-dispatch threads always
    // fall into the slow case, see DISPATCH_ROOT_QUEUE_STATE_INIT_VALUE
    // 全局并发队列和绑定到非调度线程的队列总是处于慢速状态，
    
    // 死锁
    if (unlikely(!_dispatch_queue_try_acquire_barrier_sync(dl, tid))) {
        return _dispatch_sync_f_slow(dl, ctxt, func, DC_FLAG_BARRIER, dl,
                DC_FLAG_BARRIER | dc_flags);
    }

    if (unlikely(dl->do_targetq->do_targetq)) {
        return _dispatch_sync_recurse(dl, ctxt, func,
                DC_FLAG_BARRIER | dc_flags);
    }
    _dispatch_introspection_sync_begin(dl);
    
    // 执行 block
    _dispatch_lane_barrier_sync_invoke_and_complete(dl, ctxt, func
            DISPATCH_TRACE_ARG(_dispatch_trace_item_sync_push_pop(
                    dq, ctxt, func, dc_flags | DC_FLAG_BARRIER)));
}
```
&emsp;在日常开发中我们知道如果当前是串行队列然后调用 `dispatch_sync` 函数把一个任务添加到当前的串行队列则必然会发生死锁（同步函数中如果当前正在执行的队列和等待的是同一个队列，形成相互等待的局面，则会造成死锁），而发生死锁的原因正存放在 `_dispatch_queue_try_acquire_barrier_sync(dl, tid)` 函数调用中。

&emsp;当我们在 `viewDidLoad` 中写下如下函数:
```c++
dispatch_sync(mainQueue, ^{
    NSLog(@"✈️✈️✈️ crash");
});
```
&emsp;运行后必然发生崩溃，左侧的函数调用栈可看到: `0 __DISPATCH_WAIT_FOR_QUEUE__` ⬅️ `1 _dispatch_sync_f_slow` ⬅️ `2 -[ViewController viewDidLoad]`..., 可看到是 `__DISPATCH_WAIT_FOR_QUEUE__` 函数发生了 crash。

&emsp;下面沿着 `_dispatch_queue_try_acquire_barrier_sync` 函数的调用看下去。
```c++
DISPATCH_ALWAYS_INLINE DISPATCH_WARN_RESULT
static inline bool
_dispatch_queue_try_acquire_barrier_sync(dispatch_queue_class_t dq, uint32_t tid)
{
    return _dispatch_queue_try_acquire_barrier_sync_and_suspend(dq._dl, tid, 0);
}
```
&emsp;直接调用了 `_dispatch_queue_try_acquire_barrier_sync_and_suspend` 函数。

##### _dispatch_queue_try_acquire_barrier_sync_and_suspend
&emsp;`_dispatch_queue_try_acquire_barrier_sync_and_suspend` 函数通过 `os_atomic_rmw_loop2o` 函数回调，从 OS 底层获取到了状态信息，并返回。
```c++
/* Used by _dispatch_barrier_{try,} sync
 *
 * Note, this fails if any of e:1 or dl!=0, but that allows this code to be 
 * a simple cmpxchg which is significantly faster on Intel, and makes a 
 * significant difference on the uncontended codepath.
 *
 * See discussion for DISPATCH_QUEUE_DIRTY in queue_internal.h
 *
 * Initial state must be `completely idle`
 * Final state forces { ib:1, qf:1, w:0 }
 */
 
DISPATCH_ALWAYS_INLINE DISPATCH_WARN_RESULT
static inline bool
_dispatch_queue_try_acquire_barrier_sync_and_suspend(dispatch_lane_t dq,
        uint32_t tid, uint64_t suspend_count)
{
    // #define DISPATCH_QUEUE_STATE_INIT_VALUE(width) \
    //         ((DISPATCH_QUEUE_WIDTH_FULL - (width)) << DISPATCH_QUEUE_WIDTH_SHIFT)
    // 如果 dq->dq_width 值为 1，则 init = 0xfffull << 41
    
    uint64_t init  = DISPATCH_QUEUE_STATE_INIT_VALUE(dq->dq_width);
    
    // #define DISPATCH_QUEUE_WIDTH_FULL_BIT   0x0020000000000000ull
    // #define DISPATCH_QUEUE_IN_BARRIER   0x0040000000000000ull // 当队列当前正在执行 barrier 时，此位置 1
    // #define DISPATCH_QUEUE_SUSPEND_INTERVAL   0x0400000000000000ull
    
    uint64_t value = DISPATCH_QUEUE_WIDTH_FULL_BIT | DISPATCH_QUEUE_IN_BARRIER |
            _dispatch_lock_value_from_tid(tid) |
            (suspend_count * DISPATCH_QUEUE_SUSPEND_INTERVAL);
            
    uint64_t old_state, new_state;
    
    // #define DISPATCH_QUEUE_ROLE_MASK   0x0000003000000000ull
    // #define os_atomic_rmw_loop2o(p, f, ov, nv, m, ...) os_atomic_rmw_loop(&(p)->f, ov, nv, m, __VA_ARGS__)
    
    // 从底层获取信息 -- 状态信息 - 当前队列 - 线程
    return os_atomic_rmw_loop2o(dq, dq_state, old_state, new_state, acquire, {
        uint64_t role = old_state & DISPATCH_QUEUE_ROLE_MASK;
        if (old_state != (init | role)) {
            os_atomic_rmw_loop_give_up(break);
        }
        new_state = value | role;
    });
}
```
&emsp;`_dispatch_lock_value_from_tid` 位操作取得 `dispatch_lock`。
```c++
DISPATCH_ALWAYS_INLINE
static inline dispatch_lock
_dispatch_lock_value_from_tid(dispatch_tid tid)
{
    return tid & DLOCK_OWNER_MASK;
}
```
&emsp;在 `_dispatch_barrier_sync_f_inline` 函数中，如果执行 `_dispatch_sync_f_slow` 的话，下面看一下 `_dispatch_sync_f_slow` 函数的内容。

##### _dispatch_sync_f_slow
&emsp;`_dispatch_sync_f_slow` 函数内部看到了 `__DISPATCH_WAIT_FOR_QUEUE__(&dsc, dq)` 函数的身影。

&emsp;`_dispatch_sync_f_slow` 函数中生成了一些任务的信息，然后通过 `_dispatch_trace_item_push` 来进行压栈操作，从而存放在我们的同步队列中（FIFO），从而实现函数的执行。
```c++
DISPATCH_NOINLINE
static void
_dispatch_sync_f_slow(dispatch_queue_class_t top_dqu, void *ctxt,
        dispatch_function_t func, uintptr_t top_dc_flags,
        dispatch_queue_class_t dqu, uintptr_t dc_flags)
{
    dispatch_queue_t top_dq = top_dqu._dq;
    dispatch_queue_t dq = dqu._dq;
    if (unlikely(!dq->do_targetq)) {
        return _dispatch_sync_function_invoke(dq, ctxt, func);
    }

    pthread_priority_t pp = _dispatch_get_priority();
    
    // 任务信息
    struct dispatch_sync_context_s dsc = {
        .dc_flags    = DC_FLAG_SYNC_WAITER | dc_flags,
        .dc_func     = _dispatch_async_and_wait_invoke,
        .dc_ctxt     = &dsc,
        .dc_other    = top_dq,
        .dc_priority = pp | _PTHREAD_PRIORITY_ENFORCE_FLAG,
        .dc_voucher  = _voucher_get(),
        .dsc_func    = func,
        .dsc_ctxt    = ctxt,
        .dsc_waiter  = _dispatch_tid_self(),
    };
    
    // 压栈
    _dispatch_trace_item_push(top_dq, &dsc);
    
    // ⬇️⬇️⬇️
    __DISPATCH_WAIT_FOR_QUEUE__(&dsc, dq);

    if (dsc.dsc_func == NULL) {
        // dsc_func being cleared means that the block ran on another thread ie.
        // case (2) as listed in _dispatch_async_and_wait_f_slow.
        dispatch_queue_t stop_dq = dsc.dc_other;
        return _dispatch_sync_complete_recurse(top_dq, stop_dq, top_dc_flags);
    }

    _dispatch_introspection_sync_begin(top_dq);
    _dispatch_trace_item_pop(top_dq, &dsc);
    _dispatch_sync_invoke_and_complete_recurse(top_dq, ctxt, func,top_dc_flags
            DISPATCH_TRACE_ARG(&dsc));
}
```
##### `__DISPATCH_WAIT_FOR_QUEUE__`
&emsp;`__DISPATCH_WAIT_FOR_QUEUE__` 函数中，它会获取到队列的状态，看其是否为等待状态，然后调用 `_dq_state_drain_locked_by` 中的异或运算，判断队列和线程的等待状态，如果两者都在等待，那么就会返回 YES，从而造成死锁的崩溃。
```c++
DISPATCH_NOINLINE
static void
__DISPATCH_WAIT_FOR_QUEUE__(dispatch_sync_context_t dsc, dispatch_queue_t dq)
{
    // 获取队列的状态，看是否是处于等待状态
    uint64_t dq_state = _dispatch_wait_prepare(dq);
    
    if (unlikely(_dq_state_drain_locked_by(dq_state, dsc->dsc_waiter))) {
        DISPATCH_CLIENT_CRASH((uintptr_t)dq_state,
                "dispatch_sync called on queue "
                "already owned by current thread");
    }
    ...
}
```
##### _dq_state_drain_locked_by
```c++
DISPATCH_ALWAYS_INLINE
static inline bool
_dq_state_drain_locked_by(uint64_t dq_state, dispatch_tid tid)
{
    return _dispatch_lock_is_locked_by((dispatch_lock)dq_state, tid);
}
```
```c++
DISPATCH_ALWAYS_INLINE
static inline bool
_dispatch_lock_is_locked_by(dispatch_lock lock_value, dispatch_tid tid)
{
    // #define DISPATCH_QUEUE_DRAIN_OWNER_MASK   ((uint64_t)DLOCK_OWNER_MASK)
    // equivalent to _dispatch_lock_owner(lock_value) == tid
    
    // lock_value 为队列状态，tid 为线程 id
    // ^ (异或运算法) 两个相同就会出现 0 否则为 1
    return ((lock_value ^ tid) & DLOCK_OWNER_MASK) == 0;
}
```
&emsp;`_dispatch_sync` 首先获取当前线程的 `tid`，然后获取到系统底层返回的 `status`，然后获取到队列的等待状态和 `tid` 比较，如果相同，则表示正在死锁，从而崩溃。

&emsp;看到这里我们找到了串行队列死锁的原因，那么正常执行的 `dispatch_sync` 函数执行是如何执行的，我们返回前面的流程继续往下看。

&emsp;在 `_dispatch_barrier_sync_f_inline` 函数中，正常会调用 `_dispatch_lane_barrier_sync_invoke_and_complete` 函数。

#### _dispatch_lane_barrier_sync_invoke_and_complete
```c++
/*
 * For queues we can cheat and inline the unlock code, 
 * which is invalid for objects with a more complex state machine (sources or mach channels)
 */
DISPATCH_NOINLINE
static void
_dispatch_lane_barrier_sync_invoke_and_complete(dispatch_lane_t dq,
        void *ctxt, dispatch_function_t func DISPATCH_TRACE_ARG(void *dc))
{
    _dispatch_sync_function_invoke_inline(dq, ctxt, func);
    ...
}
```
#### _dispatch_sync_function_invoke_inline
&emsp;`_dispatch_sync_function_invoke_inline` 函数内部的 `_dispatch_client_callout` 函数执行任务。
```c++
DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_sync_function_invoke_inline(dispatch_queue_class_t dq, void *ctxt,
        dispatch_function_t func)
{
    dispatch_thread_frame_s dtf;
    _dispatch_thread_frame_push(&dtf, dq);
    
    // func(ctxt)
    _dispatch_client_callout(ctxt, func);
    
    _dispatch_perfmon_workitem_inc();
    _dispatch_thread_frame_pop(&dtf);
}
```
#### _dispatch_client_callout
```c++

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_client_callout(void *ctxt, dispatch_function_t f)
{
    return f(ctxt); // ⬅️
}

DISPATCH_NOINLINE
void
_dispatch_client_callout(void *ctxt, dispatch_function_t f)
{
    _dispatch_get_tsd_base();
    void *u = _dispatch_get_unwind_tsd();
    if (likely(!u)) return f(ctxt);
    _dispatch_set_unwind_tsd(NULL);
    f(ctxt); // ⬅️
    _dispatch_free_unwind_tsd();
    _dispatch_set_unwind_tsd(u);
}

void
_dispatch_client_callout(void *ctxt, dispatch_function_t f)
{
    @try {
        return f(ctxt); // ⬅️
    }
    @catch (...) {
        objc_terminate();
    }
}
```
&emsp;看到这里当队列是串行队列时 `dispatch_sync` 的 block 得到了执行。

&emsp;当 `dispatch_sync` 把任务提交到串行队列时，完整的函数调用栈大概精简如下：`dispatch_sync` ➡️ `_dispatch_sync_f` ➡️ `_dispatch_sync_f_inline` ➡️ `_dispatch_barrier_sync_f` ➡️ `_dispatch_barrier_sync_f_inline` ➡️ `_dispatch_lane_barrier_sync_invoke_and_complete` ➡️ `_dispatch_sync_function_invoke_inline` ➡️ `_dispatch_client_callout` ➡️ `f(ctxt)`。

&emsp;下面我们看一下当提交到并行队列时 `dispatch_sync` 的  `_dispatch_sync_invoke_and_complete` 执行分支。
```c++
DISPATCH_NOINLINE
static void
_dispatch_sync_invoke_and_complete(dispatch_lane_t dq, void *ctxt,
        dispatch_function_t func DISPATCH_TRACE_ARG(void *dc))
{
    _dispatch_sync_function_invoke_inline(dq, ctxt, func);
    _dispatch_trace_item_complete(dc);
    _dispatch_lane_non_barrier_complete(dq, 0);
}
```
&emsp;`_dispatch_sync_function_invoke_inline` 函数定义其中 `_dispatch_client_callout(ctxt, func);` 执行 block 任务：
```c++
DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_sync_function_invoke_inline(dispatch_queue_class_t dq, void *ctxt,
        dispatch_function_t func)
{
    dispatch_thread_frame_s dtf;
    _dispatch_thread_frame_push(&dtf, dq);
    _dispatch_client_callout(ctxt, func); // 执行
    _dispatch_perfmon_workitem_inc();
    _dispatch_thread_frame_pop(&dtf);
}
```

&emsp;当 `dispatch_sync` 把任务提交到并发队列时，完整的函数调用栈大概精简如下：`dispatch_sync` ➡️ `_dispatch_sync_f` ➡️ `_dispatch_sync_f_inline` ➡️ `_dispatch_sync_invoke_and_complete` ➡️ `_dispatch_sync_function_invoke_inline` ➡️ `_dispatch_client_callout` ➡️ `f(ctxt)`。

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
