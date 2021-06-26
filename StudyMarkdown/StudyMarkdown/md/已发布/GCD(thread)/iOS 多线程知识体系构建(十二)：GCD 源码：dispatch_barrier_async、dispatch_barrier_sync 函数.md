# iOS 多线程知识体系构建(十二)：GCD 源码：dispatch_barrier_async 函数

> &emsp;本篇就来看看我们日常使用的 dispatch_barrier_async 的源码实现。

## dispatch_barrier_async
&emsp;`dispatch_barrier_async` 提交 barrier block 以在指定的调度队列上异步执行，同 `dispatch_async` 函数一样不会阻塞当前线程，此函数会直接返回并执行接下来的语句。

&emsp;`dispatch_barrier_async` 的作用是对异步添加到同一并发队列中的任务 block 作出 “排序”，如下测试代码：
```c++
- (void)viewDidLoad {
    [super viewDidLoad];
    
    dispatch_queue_t concurrentQueue = dispatch_queue_create("com.concurrent", DISPATCH_QUEUE_CONCURRENT);
    NSLog(@"🔞 START: %@", [NSThread currentThread]);
    
    dispatch_async(concurrentQueue, ^{ sleep(3); NSLog(@"🏃‍♀️ %@", [NSThread currentThread]);}); // ⬅️ 任务一
    dispatch_async(concurrentQueue, ^{ sleep(4); NSLog(@"🏃‍♀️🏃‍♀️ %@", [NSThread currentThread]);});// ⬅️ 任务二
    
    dispatch_barrier_async(concurrentQueue, ^{ sleep(3); NSLog(@"🚥🚥 %@", [NSThread currentThread]);}); // ⬅️ Barrie 任务
    
    dispatch_async(concurrentQueue, ^{ sleep(3); NSLog(@"🏃‍♀️🏃‍♀️🏃‍♀️ %@", [NSThread currentThread]);}); // ⬅️ 任务三
    dispatch_async(concurrentQueue, ^{ sleep(2); NSLog(@"🏃‍♀️🏃‍♀️🏃‍♀️🏃‍♀️ %@", [NSThread currentThread]);}); // ⬅️ 任务四
    NSLog(@"🔞 END: %@", [NSThread currentThread]);
}
```
&emsp;首先四个任务都不会阻塞主线程，两条 🔞 的打印会首先执行完毕，然后是任务一和任务二异步并发执行，当它们全部都执行完毕以后，开始异步执行 Barrie 任务，当 Barrie 任务 执行完毕以后，才开始异步并发执行任务三和任务四，这样就像在前两个任务和后两个任务之间插了一道无形的墙，使在 Barrie 任务之前添加的任务和之后添加的任务有了执行顺序，这就是 `dispatch_barrier_async` 函数的作用，可以在多个异步并发任务之间添加执行顺序。

&emsp;`dq` 参数是 Barrier block 提交到的目标调度队列，这里要注意把需要控制异步并发执行顺序的任务都添加到同一个自定义的并发队列 `dq` 中，同时注意不能使用 `dispatch_get_global_queue` API 获取的全局并发队列中（会导致 Barrier 失效，因为全局并发队列是系统创建的，苹果有时候会在全局并发队列中处理它自有任务，使用 barrier 函数阻塞全局并发队列无效），系统将在目标队列上保留引用，直到该 block 执行完成为止。

&emsp;`work` 参数是提交到目标调度队列的 block（该函数内部会代表调用者执行 `Block_copy` 和 `Block_release`）。
```c++
#ifdef __BLOCKS__
void
dispatch_barrier_async(dispatch_queue_t dq, dispatch_block_t work)
{
    // 取得一个 dispatch_continuation_s 结构体实例，用于封装 work
    dispatch_continuation_t dc = _dispatch_continuation_alloc();
    
    // continuation resources are freed on run this is set on async or for non event_handler source handlers
    // #define DC_FLAG_CONSUME  0x004ul
    // continuation acts as a barrier
    // #define DC_FLAG_BARRIER  0x002ul
    // DC_FLAG_CONSUME | DC_FLAG_BARRIER = 0x006ul
    
    // dc_flags 中添加 DC_FLAG_BARRIER 标记，标记此 work 是一个屏障 block，然后剩下的内容都和 dispatch_async 完全相同
    uintptr_t dc_flags = DC_FLAG_CONSUME | DC_FLAG_BARRIER;
    
    dispatch_qos_t qos;
    
    // 封装 work block 的内容以及任务执行时所处的队列等内容到 dc 中
    qos = _dispatch_continuation_init(dc, dq, work, 0, dc_flags);
    
    // 把封装好的 dispatch_continuation_s 进行异步调用
    _dispatch_continuation_async(dq, dc, qos, dc_flags);
}
#endif
```
&emsp;看到 `dispatch_barrier_async` 函数内部和 `dispatch_async` 相比在 `dc_flags` 赋值时添加了 `DC_FLAG_BARRIER` 标记，而此标记正是告知 `dispatch_continuation_s` 结构体中封装的 block 是一个 barrier block，其它的内容则和 `dispatch_async` 如出一辙。

&emsp;`_dispatch_continuation_alloc` 函数返回值是一个 `dispatch_continuation_s` 结构体指针。它会首先去当前线程的 TSD 空间中根据一个全局 KEY（`dispatch_cache_key`）去取一个 `dispatch_continuation_t`，如果取得的话会把它直接返回，并且会更新 TSD 中 `dispatch_cache_key` 的 value，而这个 value 用的正是取到的 `dispatch_continuation_s` 结构体的 `do_next` 成员变量，这样就线程中的 `dispatch_continuation_t` 构成一个链表了。如果开始未取到的话则是调用在堆区创建一个 `dispatch_continuation_s`。

### _dispatch_continuation_async
&emsp;`_dispatch_continuation_async` 函数是把封装好的任务 `dispatch_continuation_s` 添加到指定的队列中进行异步调用。
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
    // dqu._dq 是把 dispatch_queue_class_t 转换为 struct dispatch_queue_s 指针
    return dx_push(dqu._dq, dc, qos);
}

#define dx_push(x, y, z) dx_vtable(x)->dq_push(x, y, z)
#define dx_vtable(x) (&(x)->do_vtable->_os_obj_vtable)

// 把 "return dx_push(dqu._dq, dc, qos);" 宏定义展开如下：
return (&(dqu._dq)->do_vtable->_os_obj_vtable)->dq_push(dqu._dq, dc, qos);
```
&emsp;`dx_push` 宏定义的全部展开的话看到是调用 `dispatch_queue_s` 的 `_os_obj_vtable`（ dispatch_queue_s 的操作函数列表）中的 `dq_push` 函数。全局搜索 `dq_push`，看到 init.c 文件中，queue_pthread_root  根队列的 `.dq_push = _dispatch_root_queue_push`，即根队列的 `dq_push` 函数指针指向了 `_dispatch_root_queue_push` 函数，下面一起看看 `_dispatch_root_queue_push` 函数的实现。

### _dispatch_root_queue_push
&emsp;`_dispatch_root_queue_push` 函数内部是调用一个内联函数 `_dispatch_root_queue_push_inline`。
```c++
// _dispatch_root_queue_push_inline(rq, dou, dou, 1);

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_root_queue_push_inline(dispatch_queue_global_t dq,
        dispatch_object_t _head, dispatch_object_t _tail, int n)
{
    // 把 _head 和 _tail 都转换为 struct dispatch_object_s 指针
    struct dispatch_object_s *hd = _head._do, *tl = _tail._do;
    
    if (unlikely(os_mpsc_push_list(os_mpsc(dq, dq_items), hd, tl, do_next))) {
        // n = 1
        
        // 队列和队列中的任务都准备好了，现在需要执行任务了，调用 _dispatch_root_queue_poke 函数
        return _dispatch_root_queue_poke(dq, n, 0);
    }
}
```
&emsp;`os_mpsc_push_list(os_mpsc(dq, dq_items), hd, tl, do_next)` 是一个较长的宏定义，我们把它展开。
```c++
#define os_mpsc(q, _ns, ...)   (q, _ns, __VA_ARGS__)

#define os_mpsc_push_list(Q, head, tail, _o_next)  ({ \
    os_mpsc_node_type(Q) _token; \
    _token = os_mpsc_push_update_tail(Q, tail, _o_next); \
    os_mpsc_push_update_prev(Q, _token, head, _o_next); \
    os_mpsc_push_was_empty(_token); \
})

#define os_mpsc_node_type(Q) _os_atomic_basetypeof(_os_mpsc_head Q)

#define _os_mpsc_head(q, _ns, ...)   &(q)->_ns##_head ##__VA_ARGS__

// os_mpsc_push_list((dq, dq_items), hd, tl, do_next) 宏定义展开:
// Q: (dq, dq_items)
// head: hd
// tail: tl
// _o_next: do_next

({
    _os_atomic_basetypeof(&(dq)->dq_items_head) _token;
    _token = os_mpsc_push_update_tail((dq, dq_items), tl, do_next);
    os_mpsc_push_update_prev((dq, dq_items), _token, hd, do_next);
    os_mpsc_push_was_empty(_token);
});
```
&emsp;看到这里我们就明白了，`_dispatch_root_queue_push_inline` 内部的 `os_mpsc_push_list` 宏的使用把我们最开始封装的 `dispatch_continuation_s` 追加到队列的 `dq_items_tail` 中。`dq_items_tail` 是 `dispatch_lane_s` 结构体中以 `dispatch_continuation_t` 为节点的链表的尾节点的一个指针。

&emsp;如果对 `dq_items_head` 和 `dq_items_tail` 不熟悉的话可以复习一下 `dispatch_lane_s` 的定义。
```c++
typedef struct dispatch_lane_s {
    // 表示 dispatch_lane_s 继承的父类 dispatch_queue_s、dispatch_object_s、_os_object_s
    struct dispatch_queue_s _as_dq[0];
    struct dispatch_object_s _as_do[0];
    struct _os_object_s _as_os_obj[0];
    
    const struct dispatch_lane_vtable_s *do_vtable; /* must be pointer-sized */
    int volatile do_ref_cnt;
    int volatile do_xref_cnt;
    
    struct dispatch_lane_s *volatile do_next;
    struct dispatch_queue_s *do_targetq;
    void *do_ctxt;
    void *do_finalizer
    
    union { 
        uint64_t volatile dq_state;
        struct {
            dispatch_lock dq_state_lock;
            uint32_t dq_state_bits;
        };
    };
    
    /* LP64 global queue cacheline boundary */
    unsigned long dq_serialnum;
    const char *dq_label;
    
    union { 
        uint32_t volatile dq_atomic_flags;
        struct {
            const uint16_t dq_width; // 队列的宽度（串行队列为 1，并发队列大于 1）
            const uint16_t __dq_opaque2;
        };
    };
    
    dispatch_priority_t dq_priority;
    union {
        struct dispatch_queue_specific_head_s *dq_specific_head;
        struct dispatch_source_refs_s *ds_refs;
        struct dispatch_timer_source_refs_s *ds_timer_refs;
        struct dispatch_mach_recv_refs_s *dm_recv_refs;
        struct dispatch_channel_callbacks_s const *dch_callbacks;
    };
    int volatile dq_sref_cnt;
    
    dispatch_unfair_lock_s dq_sidelock; // 锁
    struct dispatch_object_s *volatile dq_items_tail; // dispatch_continuation_t 链表的尾
    struct dispatch_object_s *volatile dq_items_head; // dispatch_continuation_t 链表的头
    uint32_t dq_side_suspend_cnt // 挂起次数
    
} DISPATCH_ATOMIC64_ALIGN *dispatch_lane_t;
```
&emsp;那么到这里看到我们的任务的 block 已经追加到队列中，那么它们从哪里开始执行的呢？是下面的 `_dispatch_root_queue_poke` 函数。

### _dispatch_root_queue_poke
&emsp;`_dispatch_root_queue_poke` 函数前面都是一些判断容错，如判断 `dq_items_tail` 是否为空即判断入参队列中是否添加了任务等等，然后函数最后调用了 `_dispatch_root_queue_poke_slow` 函数。
```c++
// 根据前面一系列调用，可知入参 n = 1，floor = 0 
// _dispatch_root_queue_poke_slow(dq, 1, 0);

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
    
    ...
    
#if DISPATCH_USE_PTHREAD_POOL
    dispatch_pthread_root_queue_context_t pqc = dq->do_ctxt;
    if (likely(pqc->dpq_thread_mediator.do_vtable)) {
        // 循环调用 dispatch_semaphore_signal 发送信号量，增加 dsema_value 的值
        while (dispatch_semaphore_signal(&pqc->dpq_thread_mediator)) {
            _dispatch_root_queue_debug("signaled sleeping worker for "
                    "global queue: %p", dq);
            if (!--remaining) {
                return;
            }
        }
    }
    
    // 根据队列的优先级判断是否可以 overcommit
    bool overcommit = dq->dq_priority & DISPATCH_PRIORITY_FLAG_OVERCOMMIT;
    if (overcommit) {
        // 原子增加
        os_atomic_add2o(dq, dgq_pending, remaining, relaxed);
    } else {
        // 原子比较
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
        
        // 如果剩余大于可以请求的数量，则原子减少 dgq_pending
        if (remaining > can_request) {
            _dispatch_root_queue_debug("pthread pool reducing request from %d to %d", remaining, can_request);
            os_atomic_sub2o(dq, dgq_pending, remaining - can_request, relaxed);
            // 更新 remaining 的值
            remaining = can_request;
        }
        
        if (remaining == 0) {
            // 线程池无可用将会报错
            _dispatch_root_queue_debug("pthread pool is full for root queue: " "%p", dq);
            return;
        }
    } while (!os_atomic_cmpxchgvw2o(dq, dgq_thread_pool_size, t_count, t_count - remaining, &t_count, acquire));

#if !defined(_WIN32)

    // 线程属性
    pthread_attr_t *attr = &pqc->dpq_thread_attr;
    pthread_t tid, *pthr = &tid;
    
#if DISPATCH_USE_MGR_THREAD && DISPATCH_USE_PTHREAD_ROOT_QUEUES
    if (unlikely(dq == &_dispatch_mgr_root_queue)) {
        pthr = _dispatch_mgr_root_queue_init();
    }
#endif
    
    do {
        // 增加 dq 的引用计数
        _dispatch_retain(dq); // released in _dispatch_worker_thread
        
        // 创建线程 pthread_create
        while ((r = pthread_create(pthr, attr, _dispatch_worker_thread, dq))) {
            if (r != EAGAIN) {
                (void)dispatch_assume_zero(r);
            }
            _dispatch_temporary_resource_shortage();
        }
        
    } while (--remaining);
#else // defined(_WIN32)

...

    do {
        // 增加 dq 的引用计数
        _dispatch_retain(dq); // released in _dispatch_worker_thread
        
#if DISPATCH_DEBUG
        unsigned dwStackSize = 0;
#else
        unsigned dwStackSize = 64 * 1024;
#endif

        uintptr_t hThread = 0;
        // 
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

### _dispatch_root_queues_init

&emsp;(这里的代码真的看不懂了，暂时只是把其中的一些点记录下来)。

&emsp;`_dispatch_root_queues_init` 函数内部调用 `dispatch_once_f` 函数执行了全局只执行一次的 `_dispatch_root_queues_init_once` 函数，然后在内部设置了不同事务（任务循环、内核事务）的调用句柄都是 `_dispatch_worker_thread2`，即当队列中的任务开始执行时是调用 `_dispatch_worker_thread2` 函数。

&emsp;在 `_dispatch_root_queues_init_once` 函数开头会进行一个 for 循环，循环次数是根队列的数量 `DISPATCH_ROOT_QUEUE_COUNT`（12），每次循环执行一个:
```c++
#define DISPATCH_WORKQ_MAX_PTHREAD_COUNT 255
int thread_pool_size = DISPATCH_WORKQ_MAX_PTHREAD_COUNT;
// 以及根据是否可以 overcommit 和活动的 CPU 数量 active_cpus，动态调整线程池的容量

_dispatch_root_queue_init_pthread_pool(&_dispatch_root_queues[i], 0, _dispatch_root_queues[i].dq_priority);
```
&emsp;看到内部是调用 `semaphore_create`。

&emsp;下面看一下 `_dispatch_worker_thread2` 函数，已经被添加到队列中的 dc 是如何执行的。

### _dispatch_worker_thread2
&emsp;`_dispatch_worker_thread2` 函数内部根据优先级从根队列数组中取出一个队列，原子减 1 队列的 `dgq_pending`，关键点在于调用 `_dispatch_root_queue_drain` 函数。
```c++
_dispatch_root_queue_drain(dq, dq->dq_priority, DISPATCH_INVOKE_WORKER_DRAIN | DISPATCH_INVOKE_REDIRECTING_DRAIN);
```
### _dispatch_root_queue_drain
&emsp;`_dispatch_root_queue_drain` 函数名后面的 drain 想到了自动释放池。
```c++
DISPATCH_NOT_TAIL_CALLED // prevent tailcall (for Instrument DTrace probe)
static void
_dispatch_root_queue_drain(dispatch_queue_global_t dq,
        dispatch_priority_t pri, dispatch_invoke_flags_t flags)
{
...
    // 循环遍历返回 item 内部好像会根据一些算法遍历 dq_items_head 和 dq_items_tail，
    // 然后调用 _dispatch_continuation_pop_inline 函数执行。
    
    while (likely(item = _dispatch_root_queue_drain_one(dq))) {
        // 根据优先级重置队列中任务的顺序吗？
        if (reset) _dispatch_wqthread_override_reset();
        
        // 执行 dc
        _dispatch_continuation_pop_inline(item, &dic, flags, dq);
        
        reset = _dispatch_reset_basepri_override();
        if (unlikely(_dispatch_queue_drain_should_narrow(&dic))) {
            break;
        }
    }
...
}
```
### _dispatch_continuation_pop_inline
```c++
DISPATCH_ALWAYS_INLINE_NDEBUG
static inline void
_dispatch_continuation_pop_inline(dispatch_object_t dou,
        dispatch_invoke_context_t dic, dispatch_invoke_flags_t flags,
        dispatch_queue_class_t dqu)
{
...
    // dc 执行方式，根据队列的类型调用 dx_invoke 或者调用 _dispatch_continuation_invoke_inline 函数
    if (_dispatch_object_has_vtable(dou)) {
        dx_invoke(dou._dq, dic, flags);
    } else {
        _dispatch_continuation_invoke_inline(dou, flags, dqu);
    }
...
}
```
### _dispatch_continuation_invoke_inline
```c++
DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_continuation_invoke_inline(dispatch_object_t dou,
        dispatch_invoke_flags_t flags, dispatch_queue_class_t dqu)
{
    // 这是个逗号运算符吗？
    dispatch_continuation_t dc = dou._dc, dc1;
    
    dispatch_invoke_with_autoreleasepool(flags, {
        // dc 标识
        uintptr_t dc_flags = dc->dc_flags;
        
        _dispatch_continuation_voucher_adopt(dc, dc_flags);
        if (!(dc_flags & DC_FLAG_NO_INTROSPECTION)) {
            _dispatch_trace_item_pop(dqu, dou);
        }
        
        // 判断是否释放队列中的 dispatch_continuation_t 缓存
        if (dc_flags & DC_FLAG_CONSUME) {
            dc1 = _dispatch_continuation_free_cacheonly(dc);
        } else {
            dc1 = NULL;
        }
        
        // dispatch_group 关联的 block 执行和普通的异步提交的 block 的执行
        
        if (unlikely(dc_flags & DC_FLAG_GROUP_ASYNC)) {
            // dispatch_group 中讲过的与 dispatch_group 关联的 block 执行时调用的函数
            _dispatch_continuation_with_group_invoke(dc);
        } else {
            // 执行 dc 的 dc_func
            _dispatch_client_callout(dc->dc_ctxt, dc->dc_func);
            _dispatch_trace_item_complete(dc);
        }
        
        // 释放队列中的 dispatch_continuation_t 缓存
        if (unlikely(dc1)) {
            _dispatch_continuation_free_to_cache_limit(dc1);
        }
    });
    _dispatch_perfmon_workitem_inc();
}
```
&emsp;看到这里仿佛是在看 `dispatch_async` 的执行流程，重点应该是在 `_dispatch_continuation_pop_inline` 函数中，当 `dispatch_barrier_async` 添加 barrier block 到队列以后，当队列的任务需要执行时可能运用的是 `dx_invoke(dou._dq, dic, flags)`?


&emsp;未完待续...


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
+ [深入浅出 GCD 之 dispatch_queue](https://xiaozhuanlan.com/topic/7193856240)


