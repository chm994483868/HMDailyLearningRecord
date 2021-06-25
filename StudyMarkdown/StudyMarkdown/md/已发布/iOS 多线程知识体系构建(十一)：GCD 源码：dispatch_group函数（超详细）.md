# iOS 多线程知识体系构建(十一)：GCD 源码：dispatch_group函数（超详细）

> &emsp;用法的话可以看前面的文章，本篇只看 dispatch_group 的数据结构和相关 API 的源码实现。

## dispatch_group
&emsp;dispatch_group 可以将一组 GCD 任务关联到一起，可以监听这一组所有任务的执行情况，当所有任务异步执行完毕后我们可以得到一个或多个回调通知（使用 `dispatch_group_notify` 添加几个就能执行几个回调通知）。

### dispatch_group_s
&emsp;`dispatch_group_s` 定义和 `dispatch_semaphore_s` 定义都是放在 semaphore_internal.h 文件中，而且该文件中仅包含它俩的内容，其实文件这样布局也是有用意的，因为它俩的内部实现有一些相似性，dispatch_group 在内部也会维护一个值，当调用 `dispatch_group_enter` 函数进行进组操作时（`dg_bits` - `0x0000000000000004ULL`），当调用 `dispatch_group_leave` 函数进行出组操作时（`dg_state` + `0x0000000000000004ULL`）时对该值进行操作（这里可以把 `dg_bits` 和 `dg_state` 理解为一个值），当该值达到临界值 0 时会做一些后续操作（`_dispatch_group_wake` 唤醒异步执行 `dispatch_group_notify` 函数添加的所有回调通知），且在使用过程中一定要谨记进组（enter）和出组（leave）必须保持平衡。

&emsp;上面使用了 “关联” 一词，这里需要特别强调一下，假设与 dispatch_group 关联的 GCD 任务是一个 block，dispatch_group 并不持有此 block，甚至 dispatch_group 与此 block 没有任何关系，dispatch_group 内部的那个值只是与 enter/leave 操作有关，GCD 任务只是借用了此值，例如在创建多个 GCD 异步任务之前调用多次 enter 操作，然后在每个 GCD 任务结束时调用 leave 操作，当这多个 GCD 异步任务都执行完毕，那么如果 dispatch_group 添加了回调通知，此时自会收到回调通知。即使我们使用 dispatch_group_async 创建多个 GCD 异步任务 block， 这些 GCD 任务 block 其实与 dispatch_group 也没有任何直接的关系。

&emsp;那么与这些 GCD 异步任务相比的话，我们使用 `dispatch_group_notify` 函数添加的多个回调通知的 block 则是被 dispatch_group 所完全拥有的，这些回调通知 block 会链接成一个链表，而 dispatch_group 实例则直接拥有此链表的头节点和尾节点。

&emsp;下面就一起看看 dispatch_group 的详细实现（`dg_bits` 和 `dg_state` 两个变量下面会细说，还有其实在之前的老版本中 dispatch_group 的内部是用 dispatch_semaphore 实现的，目前则是使用类似的思想各自独立实现，我们目前看的源码正是最新版本）。

&emsp;依然从基础的数据结构开始，`dispatch_group_t` 是指向 `dispatch_group_s` 结构体的指针，那么先看下 `dispatch_group_s` 结构体的定义。
```c++
DISPATCH_CLASS_DECL(group, OBJECT);
struct dispatch_group_s {
    DISPATCH_OBJECT_HEADER(group);
    DISPATCH_UNION_LE(
            uint64_t volatile dg_state,
            uint32_t dg_bits,
            uint32_t dg_gen
    ) DISPATCH_ATOMIC64_ALIGN;
    struct dispatch_continuation_s *volatile dg_notify_head;
    struct dispatch_continuation_s *volatile dg_notify_tail;
};
```
&emsp;宏定义展开如下:
```c++
struct dispatch_group_extra_vtable_s {
    unsigned long const do_type;
    void (*const do_dispose)(struct dispatch_group_s *, bool *allow_free);
    size_t (*const do_debug)(struct dispatch_group_s *, char *, size_t);
    void (*const do_invoke)(struct dispatch_group_s *, dispatch_invoke_context_t, dispatch_invoke_flags_t);
};

struct dispatch_group_vtable_s {
    // _OS_OBJECT_CLASS_HEADER();
    void (*_os_obj_xref_dispose)(_os_object_t);
    void (*_os_obj_dispose)(_os_object_t);
    
    struct dispatch_group_extra_vtable_s _os_obj_vtable;
};

// OS_OBJECT_CLASS_SYMBOL(dispatch_group)

extern const struct dispatch_group_vtable_s _OS_dispatch_group_vtable;
extern const struct dispatch_group_vtable_s OS_dispatch_group_class __asm__("__" OS_STRINGIFY(dispatch_group) "_vtable");

struct dispatch_group_s {
    struct dispatch_object_s _as_do[0];
    struct _os_object_s _as_os_obj[0];
    
    const struct dispatch_group_vtable_s *do_vtable; /* must be pointer-sized */
    
    int volatile do_ref_cnt;
    int volatile do_xref_cnt;
    
    struct dispatch_group_s *volatile do_next;
    struct dispatch_queue_s *do_targetq;
    void *do_ctxt;
    void *do_finalizer;
    
    // 可看到上半部分和其它 GCD 对象都是相同的，毕竟大家都是继承自 dispatch_object_s，重点是下面的新内容 
    
    union { 
        uint64_t volatile dg_state;  // leave 时加 DISPATCH_GROUP_VALUE_INTERVAL
        struct { 
            uint32_t dg_bits; // enter 时减 DISPATCH_GROUP_VALUE_INTERVAL
            
            // 主要用于 dispatch_group_wait 函数被调用后，
            // 当 dispath_group 处于 wait 状态时，结束等待的条件有两条：
            // 1): 当 dispatch_group 关联的 block 都执行完毕后，wait 状态结束
            // 2): 当到达了指定的等待时间后，即使关联的 block 没有执行完成，也结束 wait 状态 
            
            // 而当 dg_gen 不为 0 时，说明 dg_state 发生了进位，可表示 dispatch_group 关联的 block 都执行完毕了，
            // 如果 dispatch_group 此时处于 wait 状态的话就可以结束了，此时正对应上面结束 wait 状态的条件 1 中。
            uint32_t dg_gen;
        };
    } __attribute__((aligned(8)));
    
    // 下面两个成员变量比较特殊，它们分别是一个链表的头节点指针和尾节点指针
    // 调用 dispatch_group_notify 函数可添加当 dispatch_group 关联的 block 异步执行完成后的回调通知，
    // 多次调用 dispatch_group_notify 函数可添加多个回调事件（我们日常开发一般就用了一个回调事件，可能会忽略这个细节），
    // 而这些多个回调事件则会构成一个 dispatch_continuation_s 作为节点的链表，当 dispatch_group 中关联的 block 全部执行完成后，
    // 此链表中的 dispatch_continuation_s 都会得到异步执行。
    //（注意是异步，具体在哪个队列则根据 dispatch_group_notify 函数的入参决定，以及执行的优先级则根据队列的优先级决定）。
    
    struct dispatch_continuation_s *volatile dg_notify_head; // dispatch_continuation_s 链表的头部节点
    struct dispatch_continuation_s *volatile dg_notify_tail; // dispatch_continuation_s 链表的尾部节点
};
```
&emsp;`dg_bits` 和 `dg_state` 是联合体共享同一块内存空间的不同名的成员变量，进组和出组时减少和增加 `DISPATCH_GROUP_VALUE_INTERVAL` 操作的其实是同一个值，再详细一点的话是联合体共占用 64 bit 空间，其中 uint64_t 类型的 dg_state 可占完整 64 bit，然后 uint32_t 类型的 `dg_bits` 和  uint32_t 类型的 `dg_gen` 组成结构体共占用这 64 bit，其中 `dg_bits` 在 低 32 bit，`dg_gen` 在高 32 bit。

&emsp;`DISPATCH_VTABLE_INSTANCE` 宏定义包裹的内容是 `dispatch_group_vtable_s` 结构体实例创建时一些成员变量的初始化，即 dispatch_group 的一些操作函数。（在 init.c 文件中 Dispatch object cluster 部分包含很多 GCD 对象的操作函数的的初始化）
```c++
// dispatch_group_extra_vtable_s 结构体中对应的成员变量的赋值
DISPATCH_VTABLE_INSTANCE(group,
    .do_type        = DISPATCH_GROUP_TYPE,
    .do_dispose     = _dispatch_group_dispose,
    .do_debug       = _dispatch_group_debug,
    .do_invoke      = _dispatch_object_no_invoke,
);
⬇️（宏展开）
DISPATCH_VTABLE_SUBCLASS_INSTANCE(group, group, __VA_ARGS__)
⬇️（宏展开）
OS_OBJECT_VTABLE_SUBCLASS_INSTANCE(dispatch_group, dispatch_group, _dispatch_xref_dispose, _dispatch_dispose, __VA_ARGS__)
⬇️（宏展开）
const struct dispatch_group_vtable_s OS_OBJECT_CLASS_SYMBOL(dispatch_group) = { \
    ._os_obj_xref_dispose = _dispatch_xref_dispose, \
    ._os_obj_dispose = _dispatch_dispose, \
    ._os_obj_vtable = { __VA_ARGS__ }, \
}
⬇️（宏展开）
const struct dispatch_group_vtable_s OS_dispatch_group_class = {
    ._os_obj_xref_dispose = _dispatch_xref_dispose,
    ._os_obj_dispose = _dispatch_dispose,
    ._os_obj_vtable = { 
        .do_type        = DISPATCH_GROUP_TYPE,
        .do_dispose     = _dispatch_group_dispose,
        .do_debug       = _dispatch_group_debug,
        .do_invoke      = _dispatch_object_no_invoke,
    }, 
}
```
### dispatch_group_create
&emsp;`dispatch_group_create` 函数用于创建可与 block 关联的 `dispatch_group_s` 结构体实例，此 `dispatch_group_s` 结构体实例可用于等待与它关联的所有 block 的异步执行完成。（使用 `dispatch_release` 释放 group 对象内存）

&emsp; GCD  任务 block 与 dispatch_group 关联的方式：  
+ 调用 `dispatch_group_enter` 表示一个 block 与 dispatch_group 关联，同时 block 执行完后要调用 `dispatch_group_leave` 表示解除关联，否则 `dispatch_group_s` 会永远等下去。
+ 调用 `dispatch_group_async` 函数与 block  关联，其实它是在内部封装了一对 enter 和 leave 操作。

&emsp;`dispatch_group_create` 函数内部直接调用了 `_dispatch_group_create_with_count` 并且入参为 0，表明目前没有 block 关联 dispatch_group。`_dispatch_group_create_and_enter` 函数则调用 `_dispatch_group_create_with_count` 入参为 1，表明有一个 block 关联 dispatch_group 操作。
```c++
dispatch_group_t
dispatch_group_create(void)
{
    // 入参为 0 调用 _dispatch_group_create_with_count 函数
    return _dispatch_group_create_with_count(0);
}

dispatch_group_t
_dispatch_group_create_and_enter(void)
{
    // 入参为 1，调用 _dispatch_group_create_with_count 函数，表示有一次 enter 操作
    return _dispatch_group_create_with_count(1);
}
```
&emsp;有符号数字和无符号数字的转换想必大家都比较熟悉了，不过开始看 `_dispatch_group_create_with_count` 函数实现之前我们还是先重温一下数值转换的知识点。

&emsp;在 dispatch_group 进行进组出组操作每次是用加减 4 （`DISPATCH_GROUP_VALUE_INTERVAL`）来记录的，并不是常见的加 1 减 1，然后起始值是从 uint32_t 的最小值 0 开始的，这里用了一个无符号数和有符号数的转换的小技巧，例如 dispatch_group 起始状态时 uint32_t 类型的 `dg_bits` 值为 0，然后第一个 enter 操作进来以后，把 uint32_t 类型的 `dg_bits` 从 0 减去 4，然后 -4 转换为 uint32_t 类型后值为 4294967292，然后 leave 操作时 `dg_bits` 加 4，即 4294967292 加 4，这样会使 uint32_t 类型值溢出然后 `dg_bits` 值就变回 0 了（uint32_t 类型的最小值），对应到 dispatch_group 中的逻辑原理即表示 `dg_bits` 达到临界值了，表示与组关联的 block 都执行完成了，可以执行后续的唤醒操作了。

&emsp;还有一点，`dg_bits` 使用 32 bit 空间对应使用 uint32_t 类型，然后 `DISPATCH_GROUP_VALUE_INTERVAL`（间隔）用 4 是因为 uint32_t 类型表示的数字个数刚好是 4 的整数倍吗，不过只要是 2 的幂都是整数倍，且 uint32_t 类型的数字即使以 4 为间隔表示的数字个数也完全足够使用了， 这里的还包括了掩码的使用，4 的二进制表示时后两位是 0，正好可以用来表示两个掩码位，仅后两位是 1 时分别对应 `DISPATCH_GROUP_HAS_NOTIFS` 和 `DISPATCH_GROUP_HAS_WAITERS` 两个宏，这两个宏的使用我们后续再讲。
```c++
#define DISPATCH_GROUP_VALUE_MASK       0x00000000fffffffcULL
#define DISPATCH_GROUP_VALUE_INTERVAL   0x0000000000000004ULL

#define DISPATCH_GROUP_VALUE_1          DISPATCH_GROUP_VALUE_MASK // 可表示此时 dispatch_group 关联了一个 block 
#define DISPATCH_GROUP_VALUE_MAX        DISPATCH_GROUP_VALUE_INTERVAL // 可表示 dispatch_group 关联的 block 达到了最大值，正常情况时应小于此值

#define DISPATCH_GROUP_HAS_NOTIFS       0x0000000000000002ULL // 表示 dispatch_group 是否有 notify 回调通知的掩码
#define DISPATCH_GROUP_HAS_WAITERS      0x0000000000000001ULL // 对应 dispatch_group_wait 函数的使用，表示 dispatch_group 是否处于等待状态的掩码

uint32_t max = (uint32_t)-1;
NSLog(@"⏰⏰：%u", UINT32_MAX);
NSLog(@"⏰⏰：%u", max);
max = max + 1;
NSLog(@"⏰⏰：%u", max);

// Implicit conversion from 'unsigned long long' to 'uint32_t' (aka 'unsigned int') changes value from 17179869180 to 4294967292
// 把 -1 转换为 uint32_t 后再转换为 ULL（无符号 long long） 然后乘以 0x0000000000000004ULL 后再强转为 uint32_t（也可以理解为 -4 转换为 uint32_t）
uint32_t dg_bits = (uint32_t)-1 * DISPATCH_GROUP_VALUE_INTERVAL;

NSLog(@"⏰⏰：%u", dg_bits);
NSLog(@"⏰⏰：%u", (uint32_t)-4);

// Implicit conversion from 'unsigned long long' to 'uint32_t' (aka 'unsigned int') changes value from 17179869176 to 4294967288
// 把 -2 转换为 uint32_t 后再转换为 ULL（无符号 long long） 然后乘以 0x0000000000000004ULL 后再强转为 uint32_t（也可以理解为 -8 转换为 uint32_t）
dg_bits = (uint32_t)-2 * DISPATCH_GROUP_VALUE_INTERVAL;

NSLog(@"⏰⏰：%u", dg_bits);

dg_bits = dg_bits - DISPATCH_GROUP_VALUE_INTERVAL;
NSLog(@"⏰⏰：%u", dg_bits);

// 打印结果:
⏰⏰：4294967295 // ⬅️ UINT32_MAX 无符号 32 位 int 的最大值（十六进制：0xFFFFFFFF）
⏰⏰：4294967295 // ⬅️ -1 强制转换为 uint32_t 的值即为 UINT32_MAX（十六进制：0xFFFFFFFF）
⏰⏰：0 // ⬅️ UINT32_MAX + 1 后溢出即切回到 uint32_t 类型的最小值 0（十六进制：0x00000000） 
⏰⏰：4294967292 // ⬅️ (uint32_t)-1 * DISPATCH_GROUP_VALUE_INTERVAL 计算的值（十六进制：0xFFFFFFFC）
⏰⏰：4294967292 // ⬅️ -4 强转为 uint32_t 类型的值，看到和上面 ⬆️ 是相等的（十六进制：0xFFFFFFFC）
⏰⏰：4294967288 // ⬅️ (uint32_t)-2 * DISPATCH_GROUP_VALUE_INTERVAL 的值也可以理解为 dg_bits 从 0 开始减了两次 4（十六进制：0xFFFFFFF8）
⏰⏰：4294967284 // ⬅️ 值为 dg_bits 再次减去 DISPATCH_GROUP_VALUE_INTERVAL（）（十六进制：0xFFFFFFF4）
```
#### _dispatch_group_create_with_count
&emsp;`_dispatch_group_create_with_count` 创建 `dispatch_group_s` 结构体，如果入参 `n` 大于 0，则指定为 `dg_bits` 的值为 `(uint32_t)-n * DISPATCH_GROUP_VALUE_INTERVAL`，并且 `dispatch_group_s` 的引用计数加 1（表示此 `dispatch_group_s` 实例当前正在被使用，不能进行销毁）。（`do_ref_cnt`）
```c++
DISPATCH_ALWAYS_INLINE
static inline dispatch_group_t
_dispatch_group_create_with_count(uint32_t n)
{
    // DISPATCH_VTABLE(group) 宏定义展开是 ➡️：&OS_dispatch_group_class
    
    // _dispatch_object_alloc 是为 dispatch_group_s 申请空间，然后用 &OS_dispatch_group_class 初始化，
    // &OS_dispatch_group_class 设置了 dispatch_group_t 的相关回调函数，如销毁函数 _dispatch_group_dispose 等。
    
    dispatch_group_t dg = _dispatch_object_alloc(DISPATCH_VTABLE(group),
            sizeof(struct dispatch_group_s));
    
    // #if DISPATCH_SIZEOF_PTR == 8
    // // the bottom nibble must not be zero, the rest of the bits should be random we sign extend 
    // // the 64-bit version so that a better instruction encoding is generated on Intel
    // #define DISPATCH_OBJECT_LISTLESS ((void *)0xffffffff89abcdef)
    // #else
    // #define DISPATCH_OBJECT_LISTLESS ((void *)0x89abcdef)
    // #endif
    
    // 表示链表的下一个节点，（目前赋一个初值 DISPATCH_OBJECT_LISTLESS）
    dg->do_next = DISPATCH_OBJECT_LISTLESS;
    
    // 目标队列（从全局的根队列数组 _dispatch_root_queues 中取默认 QOS 的队列）
    dg->do_targetq = _dispatch_get_default_queue(false);
    
    // ⬆️ 以上创建和成员赋值的内容完全同 dispatch_semaphore_s，可参考上篇，这里不再详细展开。 
    
    if (n) {
        // ⬇️ 以原子方式把 (uint32_t)-n * DISPATCH_GROUP_VALUE_INTERVAL 的值存储到 dg_bits 中，
        // n 表示 dg 关联的 block 数量。
        os_atomic_store2o(dg, dg_bits, (uint32_t)-n * DISPATCH_GROUP_VALUE_INTERVAL, relaxed);
        
        // ⬇️ 以原子方式把 1 保存到 do_ref_cnt 中（表示 dispatch_group 内部引用计数为 1，即目前有与组关联的 block 或者有任务进组了）
        os_atomic_store2o(dg, do_ref_cnt, 1, relaxed); // <rdar://22318411>
    }

    return dg;
}
```
&emsp;`os_atomic_store2o` 宏定义中 `f` 是 `p` 的成员变量，内部的 `atomic_store_explicit` 函数保证以原子方式把 `v` 的值存储到 `f` 中。
```c++
#define os_atomic_store2o(p, f, v, m) \
        os_atomic_store(&(p)->f, (v), m)

#define _os_atomic_c11_atomic(p) \
        ((__typeof__(*(p)) _Atomic *)(p))

#define os_atomic_store(p, v, m) \
        atomic_store_explicit(_os_atomic_c11_atomic(p), v, memory_order_##m)
```
### dispatch_group_enter
&emsp;`dispatch_group_enter` 表示 dispatch_group 已手动关联一个 block。

&emsp;调用此函数表示一个 block 已通过 `dispatch_group_async` 以外的方式与 dispatch_group 建立关联，对该函数的调用必须与 `dispatch_group_leave` 保持平衡。
```c++
void
dispatch_group_enter(dispatch_group_t dg)
{
    // The value is decremented on a 32bits wide atomic so that the  carry
    // for the 0 -> -1 transition is not propagated to the upper 32bits.
    // dg_bits 是无符号 32 位 int，-1 和 0 的转换在 32 位 int 范围内，不会过渡到高位，影响 dg_gen 和 dg_state 的值
    
    // dg_bits 以原子方式减少 DISPATCH_GROUP_VALUE_INTERVAL，并返回 dg_bits 的旧值，表示 dispatch_group 增加了一个关联 block
    uint32_t old_bits = os_atomic_sub_orig2o(dg, dg_bits, DISPATCH_GROUP_VALUE_INTERVAL, acquire);
    
    // #define DISPATCH_GROUP_VALUE_MASK   0x00000000fffffffcULL 二进制表示 ➡️ 0b0000...11111100ULL
    // 拿 old_bits 和 DISPATCH_GROUP_VALUE_MASK 进行与操作，取出 dg_bits 的旧值，
    // old_bits 的二进制表示的后来两位是其它作用的掩码标记位，需要做这个与操作把它们置为 0，
    // old_value 可用来判断这次 enter 之前 dispatch_group 内部是否关联过 block。
    uint32_t old_value = old_bits & DISPATCH_GROUP_VALUE_MASK;
    
    if (unlikely(old_value == 0)) {
        // 表示此时调度组由未关联任何 block 的状态变换到了关联了一个 block 的状态，
        // 调用 _dispatch_retain 把 dg 的内部引用计数 +1 表明 dg 目前正在被使用，不能进行销毁。
        
        //（表示 dispatch_group 内部有 block 没执行完成即调度组正在被使用，
        // 此时 dispatch_group 不能进行释放，想到前面的信号量，
        // 如果 dsema_value 小于 dsema_orig 表示信号量实例正在被使用，此时释放信号量实例的话也会导致 crash，
        // 整体思想和我们的 NSObject 的引用计数原理是相同的，不同之处是内存泄漏不一定会 crash，而这里则是立即 crash，
        // 当然作为一名合格的开发绝对不能容许任何内存泄漏和崩溃 ！！！）
        
        _dispatch_retain(dg); // <rdar://problem/22318411>
    }
    
    // #define DISPATCH_GROUP_VALUE_INTERVAL   0x0000000000000004ULL 二进制表示 ➡️ 0b0000...00000100ULL 
    // #define DISPATCH_GROUP_VALUE_MAX   DISPATCH_GROUP_VALUE_INTERVAL
    
    // 如果 old_bits & DISPATCH_GROUP_VALUE_MASK 的结果等于 DISPATCH_GROUP_VALUE_MAX，即 old_bits 的值是 DISPATCH_GROUP_VALUE_INTERVAL。
    // 这里可以理解为上面 4294967292 每次减 4，一直往下减，直到溢出...
    // 表示 dispatch_group_enter 函数过度调用，则 crash。
    // DISPATCH_GROUP_VALUE_MAX = 0 + DISPATCH_GROUP_VALUE_INTERVAL; 
    
    if (unlikely(old_value == DISPATCH_GROUP_VALUE_MAX)) {
        DISPATCH_CLIENT_CRASH(old_bits, "Too many nested calls to dispatch_group_enter()");
    }
}
```
#### _dispatch_retain
&emsp;`_dispatch_retain` GCD 对象的引用计数加 1（`os_obj_ref_cnt` 的值）。
```c++
DISPATCH_ALWAYS_INLINE_NDEBUG
static inline void
_dispatch_retain(dispatch_object_t dou)
{
    (void)_os_object_retain_internal_n_inline(dou._os_obj, 1);
}
```
&emsp;`_os_object_retain_internal_n_inline` 的第一个参数是 `_os_object_t` 类型。`dou._os_obj` 的入参，正是我们很早就看过的 `dispatch_object_t` 透明联合体中的 `struct _os_object_s *_os_obj` 成员变量。
```c++
DISPATCH_ALWAYS_INLINE
static inline _os_object_t
_os_object_retain_internal_n_inline(_os_object_t obj, int n)
{
    int ref_cnt = _os_object_refcnt_add_orig(obj, n);
    
    // ref_cnt 是引用计数的原始值，如果原始值小于 0，则 crash 
    if (unlikely(ref_cnt < 0)) {
        _OS_OBJECT_CLIENT_CRASH("Resurrection of an object");
    }
    
    return obj;
}
```
&emsp;`_os_object_refcnt_add_orig` 是一个宏定义，以原子方式增加引用计数。（`os_obj_ref_cnt` 的值）
```c++
#define _os_object_refcnt_add_orig(o, n) \
        _os_atomic_refcnt_add_orig2o(o, os_obj_ref_cnt, n)
        
#define _os_atomic_refcnt_add_orig2o(o, m, n) \
        _os_atomic_refcnt_perform2o(o, m, add_orig, n, relaxed)

// #define _OS_OBJECT_GLOBAL_REFCNT INT_MAX

#define _os_atomic_refcnt_perform2o(o, f, op, n, m)   ({ \
        __typeof__(o) _o = (o); \
        int _ref_cnt = _o->f; \
        
        // 如果目前引用计数 _ref_cnt 不是 int 类型的最大值，则进行 +1 操作
        if (likely(_ref_cnt != _OS_OBJECT_GLOBAL_REFCNT)) { \
            _ref_cnt = os_atomic_##op##2o(_o, f, n, m); \
        } \
        _ref_cnt; \
    })
```
### dispatch_group_leave
&emsp;`dispatch_group_leave` 手动指示 dispatch_group 中的一个关联 block 已完成，或者说是一个 block 已解除关联。

&emsp;调用此函数表示一个关联 block 已完成，并且已通过 `dispatch_group_async` 以外的方式与 dispatch_group 解除了关联。
```c++
void
dispatch_group_leave(dispatch_group_t dg)
{
    // The value is incremented on a 64bits wide atomic so that the carry
    // for the -1 -> 0 transition increments the generation atomically.
    
    // 以原子方式增加 dg_state 的值，dg_bits 的内存空间是 dg_state 的低 32 bit，
    // 所以 dg_state + DISPATCH_GROUP_VALUE_INTERVAL 没有进位到 33 bit 时都可以理解为是 dg_bits + DISPATCH_GROUP_VALUE_INTERVAL。

    //（这里注意是把 dg_state 的旧值同时赋值给了 new_state 和 old_state 两个变量）
    uint64_t new_state, old_state = os_atomic_add_orig2o(dg, dg_state, DISPATCH_GROUP_VALUE_INTERVAL, release);
    
    // #define DISPATCH_GROUP_VALUE_MASK   0x00000000fffffffcULL ➡️ 0b0000...11111100ULL
    // #define DISPATCH_GROUP_VALUE_1   DISPATCH_GROUP_VALUE_MASK
    
    // dg_state 的旧值和 DISPATCH_GROUP_VALUE_MASK 进行与操作掩码取值，如果此时仅关联了一个 block 的话那么 dg_state 的旧值就是（十六进制：0xFFFFFFFC），
    //（那么上面的 os_atomic_add_orig2o 执行后，dg_state 的值是 0x0000000100000000ULL，
    // 因为它是 uint64_t 类型它会从最大的 uint32_t 继续进位，而不同于 dg_bits 的 uint32_t 类型溢出后为 0）
    // 如果 dg_state 旧值 old_state 等于 0xFFFFFFFC 则和 DISPATCH_GROUP_VALUE_MASK 与操作结果还是 0xFFFFFFFC
    uint32_t old_value = (uint32_t)(old_state & DISPATCH_GROUP_VALUE_MASK);
    
    if (unlikely(old_value == DISPATCH_GROUP_VALUE_1)) {
        // 如果 old_value 的值是 DISPATCH_GROUP_VALUE_1。
        
        // old_state 是 0x00000000fffffffcULL，DISPATCH_GROUP_VALUE_INTERVAL 的值是 0x0000000000000004ULL
        // 所以这里 old_state 是 uint64_t 类型，加 DISPATCH_GROUP_VALUE_INTERVAL 后不会发生溢出会产生正常的进位，old_state = 0x0000000100000000ULL
        old_state += DISPATCH_GROUP_VALUE_INTERVAL;
        
        // #define DISPATCH_GROUP_HAS_WAITERS   0x0000000000000001ULL
        // #define DISPATCH_GROUP_HAS_NOTIFS    0x0000000000000002ULL
        
        do {
            // new_state = 0x0000000100000000ULL
            new_state = old_state;
            
            if ((old_state & DISPATCH_GROUP_VALUE_MASK) == 0) {
                // 如果目前是仅关联了一个 block 而且是正常的 enter 和 leave 配对执行，则会执行这里。
                
                // 清理 new_state 中对应 DISPATCH_GROUP_HAS_WAITERS 的非零位的值，
                // 即把 new_state 二进制表示的倒数第一位置 0
                new_state &= ~DISPATCH_GROUP_HAS_WAITERS;
                
                // 清理 new_state 中对应 DISPATCH_GROUP_HAS_NOTIFS 的非零位的值，
                // 即把 new_state 二进制表示的倒数第二位置 0
                new_state &= ~DISPATCH_GROUP_HAS_NOTIFS;
            } else {
                // If the group was entered again since the atomic_add above,
                // we can't clear the waiters bit anymore as we don't know for
                // which generation the waiters are for
                
                // 清理 new_state 中对应 DISPATCH_GROUP_HAS_NOTIFS 的非零位的值，
                // 即把 new_state 二进制表示的倒数第二位置 0
                new_state &= ~DISPATCH_GROUP_HAS_NOTIFS;
            }
            
            // 如果目前是仅关联了一个 block 而且是正常的 enter 和 leave 配对执行，则会执行这里的 break，
            // 结束 do while 循环，执行下面的 _dispatch_group_wake 函数，唤醒异步执行 dispatch_group_notify 添加到指定队列中的回调通知。
            if (old_state == new_state) break;
            
        // 比较 dg_state 和 old_state 的值，如果相等则把 dg_state 的值存入 new_state 中，并返回 true，如果不相等则把 dg_state 的值存入 old_state 中，并返回 false。
        // unlikely(!os_atomic_cmpxchgv2o(dg, dg_state, old_state, new_state, &old_state, relaxed)) 表达式值为 false 时才会结束循环，否则继续循环，
        // 即 os_atomic_cmpxchgv2o(dg, dg_state, old_state, new_state, &old_state, relaxed) 返回 true 时才会结束循环，否则继续循环，
        // 即 dg_state 和 old_state 的值相等时才会结束循环，否则继续循环。
        
        //（正常 enter 和 leave 的话，此时 dg_state 和 old_state 的值都是 0x0000000100000000ULL，会结束循环）
        } while (unlikely(!os_atomic_cmpxchgv2o(dg, dg_state, old_state, new_state, &old_state, relaxed)));
                
        // 唤醒异步执行 dispatch_group_notify 添加到指定队列中的回调通知
        return _dispatch_group_wake(dg, old_state, true);
    }

    // 如果 old_value 为 0，而上面又进行了一个 dg_state + DISPATCH_GROUP_VALUE_INTERVAL 操作，此时就过度 leave 了，则 crash，
    // 例如创建好一个 dispatch_group 后直接调用 dispatch_group_leave 函数即会触发这个 crash。
    if (unlikely(old_value == 0)) {
        DISPATCH_CLIENT_CRASH((uintptr_t)old_value, "Unbalanced call to dispatch_group_leave()");
    }
}
```
#### os_atomic_add_orig2o
&emsp;`os_atomic_add_orig2o` 宏定义中 `f` 是 `p` 的成员变量，内部的 `atomic_fetch_add_explicit` 函数保证以原子方式把 `f` 的值增加 `v`。
```c++
#define os_atomic_add_orig2o(p, f, v, m) \
        os_atomic_add_orig(&(p)->f, (v), m)

#define os_atomic_add_orig(p, v, m) \
        _os_atomic_c11_op_orig((p), (v), m, add, +)
        
#define _os_atomic_c11_op_orig(p, v, m, o, op) \
        atomic_fetch_##o##_explicit(_os_atomic_c11_atomic(p), v, \
        memory_order_##m)
```
#### os_atomic_cmpxchgv2o
&emsp;`os_atomic_cmpxchgv2o` 宏定义中 `f` 是 `&(p)` 的成员变量，比较 `&(p)->f` 和 `e` 的值，如果相等则用 `&(p)->f` 的值替换 `v` 的值，如果不相等，则把 `&(p)->f` 的值存入 `e` 中。
```c++
#define os_atomic_cmpxchgv2o(p, f, e, v, g, m) \
        os_atomic_cmpxchgv(&(p)->f, (e), (v), (g), m)

#define _os_atomic_basetypeof(p) \
        __typeof__(atomic_load_explicit(_os_atomic_c11_atomic(p), memory_order_relaxed))

#define _os_atomic_c11_atomic(p) \
        ((__typeof__(*(p)) _Atomic *)(p))
        
#define os_atomic_cmpxchgv(p, e, v, g, m) \
        ({ _os_atomic_basetypeof(p) _r = (e); _Bool _b = \
        atomic_compare_exchange_strong_explicit(_os_atomic_c11_atomic(p), \
        &_r, v, memory_order_##m, memory_order_relaxed); *(g) = _r; _b; })
```
### dispatch_group_async
&emsp;`dispatch_group_async` 将一个 block 提交到指定的调度队列并进行异步调用，并将该 block 与给定的 dispatch_group 关联（其内部自动插入了 `dispatch_group_enter` 和 `dispatch_group_leave` 操作，相当于 `dispatch_async` 和 `dispatch_group_enter`、`dispatch_group_leave` 三个函数的一个封装）。

&emsp;还有一个点这里要注意一下，把入参 block `db` 封装成 `dispatch_continuation_t`  `dc` 的过程中，会把 `dc_flags` 设置为 `DC_FLAG_CONSUME | DC_FLAG_GROUP_ASYNC`，这里的 `DC_FLAG_GROUP_ASYNC` 标志关系到 `dc` 执行的时候调用的具体函数（这里的提交的任务的 block 和 dispatch_group 关联的点就在这里，`dc` 执行时会调用 `_dispatch_continuation_with_group_invoke(dc)`，而我们日常使用的 `dispatch_async` 函数提交的异步任务的 block 执行的时候调用的是 `_dispatch_client_callout(dc->dc_ctxt, dc->dc_func)` 函数，它们正是根据 `dc_flags` 中的 `DC_FLAG_GROUP_ASYNC` 标识来区分的，具体细节在下面的 `_dispatch_continuation_invoke_inline` 函数中介绍）。
```c++
#ifdef __BLOCKS__
void
dispatch_group_async(dispatch_group_t dg, dispatch_queue_t dq,
        dispatch_block_t db)
{
    // 从缓存中取一个 dispatch_continuation_t 或者新建一个 dispatch_continuation_t 返回赋值给 dc。
    dispatch_continuation_t dc = _dispatch_continuation_alloc();
    
    // 这里的 DC_FLAG_GROUP_ASYNC 的标记很重要，是它标记了 dispatch_continuation 中的函数异步执行时具体调用哪个函数。
    uintptr_t dc_flags = DC_FLAG_CONSUME | DC_FLAG_GROUP_ASYNC;
    
    // 优先级
    dispatch_qos_t qos;
    
    // 配置 dsn，（db block 转换为函数）
    qos = _dispatch_continuation_init(dc, dq, db, 0, dc_flags);
    
    // 调用 _dispatch_continuation_group_async 函数异步执行提交到 dq 的 db
    _dispatch_continuation_group_async(dg, dq, dc, qos);
}
#endif
```
#### _dispatch_continuation_group_async
&emsp;`_dispatch_continuation_group_async` 函数内部调用的函数很清晰，首先调用 enter 表示 block 与 dispatch_group 建立关联，然后把 dispatch_group 赋值给 dispatch_continuation 的 `dc_data` 成员变量，这里的用途是当执行完 dispatch_continuation 中的函数后从 `dc_data` 中读取到 dispatch_group，然后对此 dispatch_group 进行一次出组 leave 操作（详情看下面的 `_dispatch_continuation_with_group_invoke` 函数），正是和这里的 enter 操作平衡的，然后就是我们比较熟悉的 `_dispatch_continuation_async` 函数提交任务到队列中进行异步调用。
```c++
DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_continuation_group_async(dispatch_group_t dg, dispatch_queue_t dq,
        dispatch_continuation_t dc, dispatch_qos_t qos)
{
    // 调用 dispatch_group_enter 表示与一个 block 建立关联
    dispatch_group_enter(dg);
    
    // 把 dg 赋值给了 dc 的 dc_data 成员变量，当 dc 中的函数执行完成后，从 dc_data 中读出 dg 执行 leave 操作，正是和上面的 enter 操作对应。
    dc->dc_data = dg;
    
    // 在指定队列中异步执行 dc
    _dispatch_continuation_async(dq, dc, qos, dc->dc_flags);
}
```
##### _dispatch_continuation_async
&emsp;`_dispatch_continuation_async` 把 GCD 任务封装成的 `dc` 提交到队列中异步执行。
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

    // 调用队列的 dx_push 函数，并示把任务放入到指定的队列中
    return dx_push(dqu._dq, dc, qos);
}

// dx_push 是一个宏定义
#define dx_push(x, y, z) dx_vtable(x)->dq_push(x, y, z)
void (*const dq_push)(dispatch_queue_class_t, dispatch_object_t, dispatch_qos_t)
```
##### _dispatch_continuation_invoke_inline
&emsp;`_dispatch_continuation_invoke_inline` 是 `dispatch_continuation_t` 被调用时执行的函数。当 `dc->dc_flags` 包含 `DC_FLAG_GROUP_ASYNC` 时，执行的是 `_dispatch_continuation_with_group_invoke` 函数。
```c++
DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_continuation_invoke_inline(dispatch_object_t dou,
        dispatch_invoke_flags_t flags, dispatch_queue_class_t dqu)
{
    dispatch_continuation_t dc = dou._dc, dc1;
    dispatch_invoke_with_autoreleasepool(flags, {
        uintptr_t dc_flags = dc->dc_flags;
        // Add the item back to the cache before calling the function. This
        // allows the 'hot' continuation to be used for a quick callback.
        //
        // The ccache version is per-thread.
        // Therefore, the object has not been reused yet.
        // This generates better assembly.
        _dispatch_continuation_voucher_adopt(dc, dc_flags);
        if (!(dc_flags & DC_FLAG_NO_INTROSPECTION)) {
            _dispatch_trace_item_pop(dqu, dou);
        }
        if (dc_flags & DC_FLAG_CONSUME) {
            dc1 = _dispatch_continuation_free_cacheonly(dc);
        } else {
            dc1 = NULL;
        }
        
        // 当 dc_flags 包含 DC_FLAG_GROUP_ASYNC 时，执行的是 _dispatch_continuation_with_group_invoke 函数，
        // else 里面的 _dispatch_client_callout 是我们日常 dispatch_async 函数提交的任务执行时调用的函数。
        
        if (unlikely(dc_flags & DC_FLAG_GROUP_ASYNC)) {
            _dispatch_continuation_with_group_invoke(dc);
        } else {
            _dispatch_client_callout(dc->dc_ctxt, dc->dc_func);
            _dispatch_trace_item_complete(dc);
        }
        
        if (unlikely(dc1)) {
            _dispatch_continuation_free_to_cache_limit(dc1);
        }
    });
    _dispatch_perfmon_workitem_inc();
}
```
##### _dispatch_continuation_with_group_invoke
&emsp;`_dispatch_continuation_with_group_invoke` 是 `dispatch_group_async` 提交的异步任务执行时调用的函数。`_dispatch_continuation_with_group_invoke` 函数内的 `dc->dc_data` 正是上面 `_dispatch_continuation_group_async` 函数中赋值的 dispatch_group，然后 `type == DISPATCH_GROUP_TYPE` 为真，`_dispatch_client_callout` 函数执行我们的 `dispatch_group_async` 函数传递的 block，然后是下面的 `dispatch_group_leave((dispatch_group_t)dou)` 出组操作，正对应了上面 `_dispatch_continuation_group_async` 函数中的 `dispatch_group_enter(dg)` 进组操作。
```c++
DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_continuation_with_group_invoke(dispatch_continuation_t dc)
{
    // dou 我们上面的赋值的 dg
    struct dispatch_object_s *dou = dc->dc_data;
    // dou 的类型是 DISPATCH_GROUP_TYPE
    unsigned long type = dx_type(dou);
    
    if (type == DISPATCH_GROUP_TYPE) {
        // 执行 GCD 任务
        _dispatch_client_callout(dc->dc_ctxt, dc->dc_func);
        _dispatch_trace_item_complete(dc);
        
        // 执行 leave 操作，和上面的 enter 操作对应
        dispatch_group_leave((dispatch_group_t)dou);
    } else {
        DISPATCH_INTERNAL_CRASH(dx_type(dou), "Unexpected object type");
    }
}
```
&emsp;至此 `dispatch_group_async` 函数就看完了，和我们自己手动调用 enter、leave、dispatch_async 相比轻松了不少，可以让我们更专注于 GCD 任务的编写。

### dispatch_group_notify
&emsp;`dispatch_group_notify` 函数，当与 dispatch_group 相关联的所有 block 都已完成时，计划将 `db` 提交到队列 `dq`（即当与 dispatch_group 相关联的所有 block 都已完成时，notify 添加的回调通知将得到执行）。如果没有 block 与 dispatch_group 相关联，则通知块 `db` 将立即提交。如下代码中通知块 `db` 将立即被调用。
```c++
dispatch_group_t group = dispatch_group_create();
// dispatch_group_notify 提交的回调 block 立即得到执行
dispatch_group_notify(group, globalQueue, ^{
    NSLog(@"🏃‍♀️ %@", [NSThread currentThread]);
});
// 控制台打印:
 🏃‍♀️ <NSThread: 0x600000fcbe00>{number = 5, name = (null)}
```

&emsp;通知块 `db` 提交到目标队列 `dq` 时，该 dispatch_group 关联的 block 将为空，或者说只有该 dispatch_group 关联的 block 为空时，通知块 `db` 才会提交到目标队列 `dq`。此时可以通过 `dispatch_release` 释放 dispatch_group，也可以重新用于其他操作。

&emsp;`dispatch_group_notify` 函数不会阻塞当前线程，此函数会立即返回，如果我们想阻塞当前线程，想要等 dispatch_group 中关联的 block 全部执行完成后才执行接下来的操作时，可以使用 `dispatch_group_wait` 函数并指定具体的等待时间（`DISPATCH_TIME_FOREVER`）。
```c++
#ifdef __BLOCKS__
void
dispatch_group_notify(dispatch_group_t dg, dispatch_queue_t dq,
        dispatch_block_t db)
{
    // 从缓存中取一个 dispatch_continuation_t 或者新建一个 dispatch_continuation_t 返回
    dispatch_continuation_t dsn = _dispatch_continuation_alloc();
    // 配置 dsn，即用 dispatch_continuation_s 封装 db。（db 转换为函数）
    _dispatch_continuation_init(dsn, dq, db, 0, DC_FLAG_CONSUME);
    
    // 调用 _dispatch_group_notify 函数
    _dispatch_group_notify(dg, dq, dsn);
}
#endif
```
#### _dispatch_group_notify
&emsp;
```c++
DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_group_notify(dispatch_group_t dg, dispatch_queue_t dq,
        dispatch_continuation_t dsn)
{
    uint64_t old_state, new_state;
    dispatch_continuation_t prev;

    // dispatch_continuation_t 的 dc_data 成员变量被赋值为 dispatch_continuation_s 执行时所在的队列 
    dsn->dc_data = dq;
    
    // dq 队列引用计数 +1，因为有新的 dsn 要在这个 dq 中执行了（`os_obj_ref_cnt` 的值 +1）
    _dispatch_retain(dq);

    //    prev =  ({
    //        // 以下都是原子操作:
    //        _os_atomic_basetypeof(&(dg)->dg_notify_head) _tl = (dsn); // 类型转换
    //        // 把 dsn 的 do_next 置为 NULL，防止错误数据
    //        os_atomic_store(&(_tl)->do_next, (NULL), relaxed);
    //        // 入参 dsn 存储到 dg 的成员变量 dg_notify_tail 中，并返回之前的旧的 dg_notify_tail
    //        atomic_exchange_explicit(_os_atomic_c11_atomic(&(dg)->dg_notify_tail), _tl, memory_order_release);
    //    });
    
    // 把 dsn 存储到 dg 的 dg_notify_tail 成员变量中，并返回之前的旧 dg_notify_tail，
    // 这个 dg_notify_tail 是一个指针，用来指向 dg 的 notify 回调函数链表的尾节点。
    prev = os_mpsc_push_update_tail(os_mpsc(dg, dg_notify), dsn, do_next);
    
    // #define os_mpsc_push_was_empty(prev) ((prev) == NULL)
    
    // 如果 prev 为 NULL，表示 dg 是第一次添加 notify 回调函数，则再次增加 dg 的引用计数（os_obj_ref_cnt + 1），
    // 前面我们还看到 dg 在第一次执行 enter 时也会增加一次引用计数（os_obj_ref_cnt + 1）。
    if (os_mpsc_push_was_empty(prev)) _dispatch_retain(dg);
    
    //    ({
    //        // prev 是指向 notify 回调函数链表的尾节点的一个指针
    //        _os_atomic_basetypeof(&(dg)->dg_notify_head) _prev = (prev);
    //        if (likely(_prev)) {
    //            // 如果之前的尾节点存在，则把 dsn 存储到之前尾节点的 do_next 中，即进行了链表拼接
    //            (void)os_atomic_store(&(_prev)->do_next, ((dsn)), relaxed);
    //        } else {
    //            // 如果之前尾节点不存在，则表示链表为空，则 dsn 就是头节点了，并存储到 dg 的 dg_notify_head 成员变量中
    //            (void)os_atomic_store(&(dg)->dg_notify_head, (dsn), relaxed);
    //        }
    //    });
    
    // 把 dsn 拼接到 dg 的 notify 回调函数链表中，或者是第一次的话，则把 dsn 作为 notify 回调函数链表的头节点
    os_mpsc_push_update_prev(os_mpsc(dg, dg_notify), prev, dsn, do_next);
    
    if (os_mpsc_push_was_empty(prev)) {
        // 如果 prev 不为 NULL 的话，表示 dg 有 notify 回调函数存在。
    
        // os_atomic_rmw_loop2o 是一个宏定义，内部包裹了一个 do while 循环，
        // 直到 old_state == 0 时跳出循环执行 _dispatch_group_wake 函数唤醒执行 notify 链表中的回调通知，
        // 即对应我们上文中的 dispatch_group_leave 函数中 dg_bits 的值回到 0 表示 dispatch_group 中关联的 block 都执行完了。
        
        // 大概逻辑是这样，这里不再拆开宏定义分析了，具体拆开如下面的 os_atomic_rmw_loop2o 宏分析，
        // 实在太杀时间了，低头一小时，抬头一小时...😭😭
        
        // 只要记得这里是用一个 do while 循环等待，每次循环以原子方式读取状态值（dg_bits），
        // 直到 0 状态，去执行 _dispatch_group_wake 唤醒函数把 notify 链表中的函数提交到指定的队列异步执行就好了！⛽️⛽️
        
        os_atomic_rmw_loop2o(dg, dg_state, old_state, new_state, release, {
            // #define DISPATCH_GROUP_HAS_NOTIFS   0x0000000000000002ULL
            
            // 这里挺重要的一个点，把 new_state 的二进制表示的倒数第二位置为 1，
            // 表示 dg 存在 notify 回调函数。
            new_state = old_state | DISPATCH_GROUP_HAS_NOTIFS;
            
            if ((uint32_t)old_state == 0) {
            
                // 跳出循环执行 _dispatch_group_wake 函数，把 notify 回调函数链表中的任务提交到指定的队列中执行
                os_atomic_rmw_loop_give_up({
                    return _dispatch_group_wake(dg, new_state, false);
                });
            }
        });
    }
}
```
##### os_mpsc_push_update_tail
&emsp;`os_mpsc_push_update_tail` 是一个宏定义，主要是对 dispatch_group 的成员变量 `dg_notify_tail` 进行更新，即把新入参的 `dispatch_continuation_t dsn` 存储到 `dg_notify_tail` 中，并返回之前的 `dg_notify_tail`。

&emsp;下面是涉及到的宏定义，想要看一行代码的含义，要把下面的涉及的宏定义全部展开，只能感叹...🍎🐂🍺...
```c++
#define os_mpsc(q, _ns, ...)   (q, _ns, __VA_ARGS__)
#define os_mpsc_node_type(Q) _os_atomic_basetypeof(_os_mpsc_head Q)

#define _os_mpsc_head(q, _ns, ...)   &(q)->_ns##_head ##__VA_ARGS__
#define _os_mpsc_tail(q, _ns, ...)   &(q)->_ns##_tail ##__VA_ARGS__
#define _os_atomic_c11_atomic(p) \
        ((__typeof__(*(p)) _Atomic *)(p))

#define os_atomic_store2o(p, f, v, m) \
        os_atomic_store(&(p)->f, (v), m)
        
#define os_atomic_xchg(p, v, m) \
        atomic_exchange_explicit(_os_atomic_c11_atomic(p), v, memory_order_##m)

#define _os_atomic_basetypeof(p) \
        __typeof__(atomic_load_explicit(_os_atomic_c11_atomic(p), memory_order_relaxed))
        
// Returns true when the queue was empty and the head must be set
#define os_mpsc_push_update_tail(Q, tail, _o_next)  ({ \
        os_mpsc_node_type(Q) _tl = (tail); \
        os_atomic_store2o(_tl, _o_next, NULL, relaxed); \
        os_atomic_xchg(_os_mpsc_tail Q, _tl, release); \
    })
```
&emsp;`prev = os_mpsc_push_update_tail(os_mpsc(dg, dg_notify), dsn, do_next)` 宏定义展开如下:
```c++
prev =  ({
    // 以下都是原子操作:
    _os_atomic_basetypeof(&(dg)->dg_notify_head) _tl = (dsn); // 类型转换
    // 入参 dsn 会作为新的尾节点，这是是把 dsn 的 do_next 置为 NULL，防止错误数据
    os_atomic_store(&(_tl)->do_next, (NULL), relaxed);
    // 入参 dsn 存储到 dispatch_group 的 dg_notify_tail 节点，并返回之前的旧的尾节点
    atomic_exchange_explicit(_os_atomic_c11_atomic(&(dg)->dg_notify_tail), _tl, memory_order_release);
});
```
##### os_mpsc_push_update_prev
&emsp;`os_mpsc_push_update_prev` 是一个宏定义，当 dispatch_group 的  notify 回调函数链表为空时，把入参的 `dispatch_continuation_t dsn` 存储到 `dg_notify_head` 中，如果是链表不为空，则把入参 `dispatch_continuation_t dsn` 拼接到链表中。
```c++
#define os_mpsc_push_update_prev(Q, prev, head, _o_next)  ({ \
    os_mpsc_node_type(Q) _prev = (prev); \
    if (likely(_prev)) { \
        (void)os_atomic_store2o(_prev, _o_next, (head), relaxed); \
    } else { \
        (void)os_atomic_store(_os_mpsc_head Q, (head), relaxed); \
    } \
})
```
&emsp;`os_mpsc_push_update_prev((dg, dg_notify), prev, dsn, do_next)` 宏定义展开如下：
```c++
({
    // prev 是 notify 回调函数链表的尾节点
    _os_atomic_basetypeof(&(dg)->dg_notify_head) _prev = (prev);
    if (likely(_prev)) {
        // 如果之前的尾节点存在，则把 dsn 存储到之前尾节点的 do_next 中
        (void)os_atomic_store(&(_prev)->do_next, ((dsn)), relaxed);
    } else {
        // 如果之前尾节点不存在，则表示链表为空，则 dsn 就是头节点了，并则存储到 dg 的 dg_notify_head 成员变量中
        (void)os_atomic_store(&(dg)->dg_notify_head, (dsn), relaxed);
    }
});
```
##### os_atomic_rmw_loop2o
&emsp;`os_atomic_rmw_loop2o` 宏定义内部其实包裹了一个完整的 do while，然后是 do while 循环内部是一个原子操作进行读值比较赋值直到可以跳出循环。
```c++
#define os_atomic_rmw_loop2o(p, f, ov, nv, m, ...) \
        os_atomic_rmw_loop(&(p)->f, ov, nv, m, __VA_ARGS__)
        
#define os_atomic_cmpxchgvw(p, e, v, g, m) \
        ({ _os_atomic_basetypeof(p) _r = (e); _Bool _b = \
        atomic_compare_exchange_weak_explicit(_os_atomic_c11_atomic(p), \
        &_r, v, memory_order_##m, memory_order_relaxed); *(g) = _r;  _b; })

#define os_atomic_rmw_loop(p, ov, nv, m, ...)  ({ \
    bool _result = false; \
    __typeof__(p) _p = (p); \
    ov = os_atomic_load(_p, relaxed); \
    do { \
        __VA_ARGS__; \
        _result = os_atomic_cmpxchgvw(_p, ov, nv, &ov, m); \
    } while (unlikely(!_result)); \
    _result; \
})
```
&emsp;上面 `_dispatch_group_notify` 函数中，`os_atomic_rmw_loop2o` 的宏定义使用，大概可以拆成下面一段：
```c++
({
    bool _result = false;
    __typeof__(dg) _dg = (dg);
    
    dg_state = os_atomic_load(_dg, relaxed);
    
    do {
        new_state = old_state | DISPATCH_GROUP_HAS_NOTIFS;
        if ((uint32_t)old_state == 0) {
            os_atomic_rmw_loop_give_up({
                return _dispatch_group_wake(dg, new_state, false);
            });
        }
        
        _result = ({ _os_atomic_basetypeof(_dg) _r = (dg_state); 
                     _Bool _b = atomic_compare_exchange_weak_explicit(_os_atomic_c11_atomic(_dg), &_r, old_state, memory_order_release, memory_order_relaxed); 
                     *(&dg_state) = _r;  
                     _b; 
                  });
    } while (unlikely(!_result));
    _result;
})
```
### dispatch_group_wait
&emsp;`dispatch_group_wait` 函数同步等待直到与 dispatch_group 关联的所有 block 都异步执行完成或者直到指定的超时时间过去为止，才会返回。

&emsp;如果没有与 dispatch_group 关联的 block，则此函数将立即返回。

&emsp;从多个线程同时使用同一 dispatch_group 调用此函数的结果是不确定的。

&emsp;成功返回此函数后，dispatch_group 关联的 block 为空，可以使用 `dispatch_release` 释放 dispatch_group，也可以将其重新用于其它 block。

&emsp;`timeout`：指定何时超时（dispatch_time_t）。有 `DISPATCH_TIME_NOW` 和 `DISPATCH_TIME_FOREVER` 常量。

&emsp;`result`：成功返回零（与该组关联的所有块在指定的时间内完成），错误返回非零（即超时）。
```c++
long
dispatch_group_wait(dispatch_group_t dg, dispatch_time_t timeout)
{
    uint64_t old_state, new_state;
    
    // 使用同上面的 os_atomic_rmw_loop2o 宏定义，内部是一个 do while 循环，
    // 每次循环都从本地原子取值，判断 dispatch_group 所处的状态，
    // 是否关联的 block 都异步执行完毕了。 
    
    os_atomic_rmw_loop2o(dg, dg_state, old_state, new_state, relaxed, {
        // #define DISPATCH_GROUP_VALUE_MASK   0x00000000fffffffcULL
        
        // 表示关联的 block 为 0 或者关联的 block 都执行完毕了，则直接 return 0，
        //（函数返回，停止阻塞当前线程。）
        if ((old_state & DISPATCH_GROUP_VALUE_MASK) == 0) {
            // 跳出循环并返回 0
            os_atomic_rmw_loop_give_up_with_fence(acquire, return 0);
        }
        
        // 如果 timeout 等于 0，则立即跳出循环并返回 _DSEMA4_TIMEOUT()，
        // 指定等待时间为 0，则函数返回，并返回超时提示，
        //（继续向下执行，停止阻塞当前线程。）
        if (unlikely(timeout == 0)) {
            // 跳出循环并返回 _DSEMA4_TIMEOUT() 超时
            os_atomic_rmw_loop_give_up(return _DSEMA4_TIMEOUT());
        }
        
        // #define DISPATCH_GROUP_HAS_WAITERS   0x0000000000000001ULL
        new_state = old_state | DISPATCH_GROUP_HAS_WAITERS;
        
        // 表示目前需要等待，至少等到关联的 block 都执行完毕或者等到指定时间超时 
        if (unlikely(old_state & DISPATCH_GROUP_HAS_WAITERS)) {
            // 跳出循环，执行下面的 _dispatch_group_wait_slow 函数
            os_atomic_rmw_loop_give_up(break);
        }
    });

    return _dispatch_group_wait_slow(dg, _dg_state_gen(new_state), timeout);
}
```
#### _dispatch_group_wait_slow
&emsp; 
```c++
DISPATCH_NOINLINE
static long
_dispatch_group_wait_slow(dispatch_group_t dg, uint32_t gen,
        dispatch_time_t timeout)
{
    // for 死循环，等待内部的条件满足时 return，否则一直进行死循环
    for (;;) {
        // 比较等待，内部是根据指定的时间进行时间等待，并根据 &dg->dg_gen 值判断是否关联的 block 都异步执行完毕了。
        // 这里牵涉到 dg_state 的进位，当 dg_bits 溢出时会进位到 dg_gen 中，此时 dg_gen 不再是 0，可表示关联的 block 都执行完毕了。
        int rc = _dispatch_wait_on_address(&dg->dg_gen, gen, timeout, 0);
        
        // 表示 dispatch_group 关联的 block 都异步执行完毕了，return 0
        if (likely(gen != os_atomic_load2o(dg, dg_gen, acquire))) {
            return 0;
        }
        
        // 等到超过指定时间了，return _DSEMA4_TIMEOUT() 超时
        if (rc == ETIMEDOUT) {
            return _DSEMA4_TIMEOUT();
        }
    }
}
```
&emsp;下面我们学习上面 `dispatch_group_leave` 和 `dispatch_group_notify` 函数中都曾使用过的 `_dispatch_group_wake` 函数，正是它完成了唤醒，并把所有的 notify 回调函数提交到指定的队列中异步执行。

### _dispatch_group_wake
&emsp;`_dispatch_group_wake` 把  notify 回调函数链表中的所有的函数提交到指定的队列中异步执行，`needs_release` 表示是否需要释放所有关联 block 异步执行完成、所有的 notify 回调函数执行完成的 dispatch_group 对象。`dg_state` 则是 dispatch_group 的状态，包含目前的关联的 block 数量等信息。
```c++
DISPATCH_NOINLINE
static void
_dispatch_group_wake(dispatch_group_t dg, uint64_t dg_state, bool needs_release)
{
    // dispatch_group 对象的引用计数是否需要 -1
    uint16_t refs = needs_release ? 1 : 0; // <rdar://problem/22318411>
    
    // #define DISPATCH_GROUP_HAS_NOTIFS   0x0000000000000002ULL // 用来判断 dispatch_group 是否存在 notify 函数的掩码
    
    // 这里如果 dg_state & 0x0000000000000002ULL 结果不为 0，即表示 dg 存在 notify 回调函数
    if (dg_state & DISPATCH_GROUP_HAS_NOTIFS) {
    
        dispatch_continuation_t dc, next_dc, tail;

        // Snapshot before anything is notified/woken <rdar://problem/8554546>
        
        // 取出 dg 的 notify 回调函数链表的头 
        dc = os_mpsc_capture_snapshot(os_mpsc(dg, dg_notify), &tail);
        
        do {
            // 取出 dc 创建时指定的队列，对应 _dispatch_group_notify 函数中的 dsn->dc_data = dq 赋值操作
            dispatch_queue_t dsn_queue = (dispatch_queue_t)dc->dc_data;
            
            // 取得下一个节点
            next_dc = os_mpsc_pop_snapshot_head(dc, tail, do_next);
            
            // 根据各队列的优先级异步执行 notify 链表中的函数
            _dispatch_continuation_async(dsn_queue, dc, _dispatch_qos_from_pp(dc->dc_priority), dc->dc_flags);
                    
            // 释放 notify 函数执行时的队列 dsn_queue（os_obj_ref_cnt - 1）
            _dispatch_release(dsn_queue);
            
        // 当 next_dc 为 NULL 时，跳出循环
        } while ((dc = next_dc));

        // 这里的 refs 计数增加 1 正对应了 _dispatch_group_notify 函数中，
        // 当第一次给 dispatch_group 添加 notify 函数时的引用计数加 1，_dispatch_retain(dg)
        // 代码执行到这里时 dg 的所有 notify 函数都执行完毕了。 
        //（统计 dispatch_group 的引用计数需要减小的值）
        refs++;
    }
    
    // #define DISPATCH_GROUP_HAS_WAITERS   0x0000000000000001ULL
    
    // 根据 &dg->dg_gen 的值判断是否处于阻塞状态
    if (dg_state & DISPATCH_GROUP_HAS_WAITERS) {
        _dispatch_wake_by_address(&dg->dg_gen);
    }

    // 根据 refs 判断是否需要释放 dg（执行 os_obj_ref_cnt - refs），当 os_obj_ref_cnt 的值小于 0 时，可销毁 dg。
    // 如果 needs_release 为真，并且 dg 有 notify 函数时，会执行 os_obj_ref_cnt - 2
    // 如果 needs_release 为假，但是 dg 有 notify 函数时，会执行 os_obj_ref_cnt - 1
    // 如果 needs_release 为假，且 dg 无 notify 函数时，不执行操作
    if (refs) _dispatch_release_n(dg, refs);
}
```
#### os_mpsc_capture_snapshot
&emsp;`os_mpsc_capture_snapshot` 宏定义，用来取 notify 函数链表的头节点。
```c++
#define os_mpsc_get_head(Q)  ({ \
    __typeof__(_os_mpsc_head Q) __n = _os_mpsc_head Q; \
    os_mpsc_node_type(Q) _node; \
    _node = os_atomic_load(__n, dependency); \
    if (unlikely(_node == NULL)) { \
        _node = _dispatch_wait_for_enqueuer((void **)__n); \
    } \
    _node; \
})

#define os_atomic_xchg(p, v, m) \
        atomic_exchange_explicit(_os_atomic_c11_atomic(p), v, memory_order_##m)

#define os_mpsc_capture_snapshot(Q, tail)  ({ \
    os_mpsc_node_type(Q) _head = os_mpsc_get_head(Q); \
    os_atomic_store(_os_mpsc_head Q, NULL, relaxed); \
    /* 22708742: set tail to NULL with release, so that NULL write */ \
    /* to head above doesn't clobber head from concurrent enqueuer */ \
    *(tail) = os_atomic_xchg(_os_mpsc_tail Q, NULL, release); \
    _head; \
})
```
#### _dispatch_release_n
&emsp;`_dispatch_release_n` 根据入参减少 GCD “对象” 的内部的引用计数。
```c++
DISPATCH_ALWAYS_INLINE_NDEBUG
static inline void
_dispatch_release_n(dispatch_object_t dou, int n)
{
    _os_object_release_internal_n_inline(dou._os_obj, n);
}
```
&emsp;`_os_object_release_internal_n_inline`:
```c++
DISPATCH_ALWAYS_INLINE
static inline void
_os_object_release_internal_n_inline(_os_object_t obj, int n)
{
    // 以原子方式对 obj 的 os_obj_ref_cnt 减少 n
    int ref_cnt = _os_object_refcnt_sub(obj, n);
    
    //  如果大于等于 0 表示该 GCD 对象还正在被使用，直接 return 
    if (likely(ref_cnt >= 0)) {
        return;
    }
    
    // 如果小于 -1 的话，表示发生了错误 GCD 对象被过度 release
    if (unlikely(ref_cnt < -1)) {
        _OS_OBJECT_CLIENT_CRASH("Over-release of an object");
    }
    
    // 看到只有 DEBUG 模式下才会操作 os_obj_xref_cnt 成员变量
#if DISPATCH_DEBUG
    int xref_cnt = obj->os_obj_xref_cnt;
    if (unlikely(xref_cnt >= 0)) {
        DISPATCH_INTERNAL_CRASH(xref_cnt,
                "Release while external references exist");
    }
#endif

    // _os_object_refcnt_dispose_barrier() is in _os_object_dispose()
    return _os_object_dispose(obj);
}
```
&emsp;`_os_object_refcnt_sub` 是一个宏，对 GCD 对象的 `os_obj_ref_cnt` 以原子方式减少 `n`。
```c++
#define _os_object_refcnt_sub(o, n) \
        _os_atomic_refcnt_sub2o(o, os_obj_ref_cnt, n)
        
#define _os_atomic_refcnt_sub2o(o, m, n) \
        _os_atomic_refcnt_perform2o(o, m, sub, n, release)

#define _OS_OBJECT_GLOBAL_REFCNT INT_MAX

#define _os_atomic_refcnt_perform2o(o, f, op, n, m)   ({ \
    __typeof__(o) _o = (o); \
    int _ref_cnt = _o->f; \
    if (likely(_ref_cnt != _OS_OBJECT_GLOBAL_REFCNT)) { \
        _ref_cnt = os_atomic_##op##2o(_o, f, n, m); \
    } \
    _ref_cnt; \
})
```
### _os_object_dispose
&emsp;`_os_object_dispose` 函数是 GCD 对象的销毁函数。
```c++
#define _os_object_refcnt_dispose_barrier(o) \
        _os_atomic_refcnt_dispose_barrier2o(o, os_obj_ref_cnt)

#define _os_atomic_refcnt_dispose_barrier2o(o, m) \
        (void)os_atomic_load2o(o, m, acquire)

void
_os_object_dispose(_os_object_t obj)
{
    // 以原子方式读取 os_obj_ref_cnt 的值
    _os_object_refcnt_dispose_barrier(obj);
    
    // 如果 GCD 对象包含指定的销毁函数 _os_obj_dispose，则执行自己的销毁函数
    if (likely(obj->os_obj_isa->_os_obj_dispose)) {
        return obj->os_obj_isa->_os_obj_dispose(obj);
    }
    
    // 销毁 obj
    return _os_object_dealloc(obj);
}

void
_os_object_dealloc(_os_object_t obj)
{
    *((void *volatile*)&obj->os_obj_isa) = (void *)0x200;
    // free 释放 obj 的内存空间
    return free(obj);
}
```
&emsp;`_os_object_dealloc` 销毁函数也对应了 GCD 对象的创建函数 `_os_object_alloc_realized`。
```c++
inline _os_object_t
_os_object_alloc_realized(const void *cls, size_t size)
{
    _os_object_t obj;
    dispatch_assert(size >= sizeof(struct _os_object_s));
    
    // while 循环保证 calloc 申请内存空间成功
    while (unlikely(!(obj = calloc(1u, size)))) {
        _dispatch_temporary_resource_shortage();
    }
    
    obj->os_obj_isa = cls;
    return obj;
}
```

&emsp;至此 dispatch_group 的内容就看完了，细节很多，知识点很多，需要耐心取一点一点解读！⛽️⛽️

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

