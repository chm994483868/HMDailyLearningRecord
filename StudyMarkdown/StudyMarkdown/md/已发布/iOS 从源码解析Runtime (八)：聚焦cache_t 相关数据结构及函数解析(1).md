# iOS 从源码解析Runtime (八)：聚焦cache_t 相关数据结构及函数解析(1)

> &emsp;前面连续几篇我们已经详细分析了 `objc_object` 的相关的所有源码，接下来几篇则开始分析定义于 `objc-runtime-new.h` 中的 `objc_class`，本篇先从 `struct objc_class : objc_object` 的 `cache_t cache` 开始，`cache_t` 主要实现方法缓存，帮助我们更快的找到方法地址进行调用。
  纵览 `objc-runtime-new.h` 文件真的超长，那我们就分块来学习，一起 ⛽️⛽️ 吧！

```c++
struct objc_class : objc_object {
// Class ISA; // objc_class 继承自 objc_object，所以其第一个成员变量其实是 isa_t isa 
Class superclass; // 父类指针
cache_t cache; // formerly cache pointer and vtable 以前缓存指针和虚函数表
...
};
```
> &emsp;`typedef uintptr_t SEL;` 在 `objc-runtime-new.h` 的 `198` 行，看到 `SEL`。 

&emsp;`cache` 是 `objc_class` 的第三个成员变量，类型是 `cache_t`。从数据结构角度及使用方法来看 `cache_t` 的话，它是一个 `SEL`  作为 `Key` ，`SEL + IMP(bucket_t)` 作为 `Value` 的散列表。为了对方法缓存先有一个大致的了解，我们首先解读一下 `objc-cache.mm` 文件开头的一大段注释内容。
```c++
/*
objc-cache.m

1. Method cache management 方法缓存管理
2. Cache flushing 缓存刷新
3. Cache garbage collection 缓存垃圾回收
4. Cache instrumentation 缓存检测
5. Dedicated allocator for large caches 大型缓存的专用分配器 (?)

/*
Method cache locking (GrP 2001-1-14)
For speed, objc_msgSend does not acquire any locks when it reads method caches. 
为了提高速度，objc_msgSend 在读取方法缓存时不获取任何锁。

Instead, all cache changes are performed
so that any objc_msgSend running concurrently with the cache mutator 
will not crash or hang or get an incorrect result from the cache. 
相反，将执行所有缓存更改，以便与缓存 mutator 并发运行的任何 objc_msgSend 都不会崩溃或挂起，
或者从缓存中获得不正确的结果。（以 std::atomic 完成所有的原子操作）

When cache memory becomes unused (e.g. the old cache after cache expansion), 
it is not immediately freed, because a concurrent objc_msgSend could still be using it. 
当缓存未使用时，（例如：缓存扩展后的旧缓存），它不会立即释放，因为并发的 objc_msgSend 可能仍在使用它。

Instead, the memory is disconnected from the data structures and placed on a garbage list. 
相反，内存与数据结构断开连接，并放在垃圾列表中。

The memory is now only accessible to instances of objc_msgSend that were
running when the memory was disconnected; 
内存现在只能访问断开内存时运行的 objc_msgSend 实例；

any further calls to objc_msgSend will not see the garbage memory because
the other data structures don't point to it anymore.
对 objc_msgSend 的任何进一步调用都不会看到垃圾内存，因为其他数据结构不再指向它。

The collecting_in_critical function checks the PC of all threads and returns FALSE when
all threads are found to be outside objc_msgSend. 
collecting_in_critical 函数检查所有线程的PC，当发现所有线程都在 objc_msgSend 之外时返回 FALSE。

This means any call to objc_msgSend that could have had access to the garbage has
finished or moved past the cache lookup stage, so it is safe to free the memory. 
这意味着可以访问垃圾的对 objc_msgSend 的任何调用都已完成或移动到缓存查找阶段，因此可以安全地释放内存。

All functions that modify cache data or structures must acquire the cacheUpdateLock
to prevent interference from concurrent modifications. 
所有修改缓存数据或结构的函数都必须获取 cacheUpdateLock，以防止并发修改的干扰。

The function that frees cache garbage must acquire the cacheUpdateLock and use
collecting_in_critical() to flush out cache readers.
释放缓存垃圾的函数必须获取 cacheUpdateLock，并使用 collecting_in_critical() 清除缓存读取器

The cacheUpdateLock is also used to protect the custom allocator used for large method cache blocks.
Cache readers (PC-checked by collecting_in_critical())
cacheUpdateLock 还用于保护用于大型方法缓存块的自定义分配器。缓存读取器（由 collecting_in_critical() 进行 PC 检查）

objc_msgSend
cache_getImp

Cache writers (hold cacheUpdateLock while reading or writing; not PC-checked)
(读取或写入时获取 cacheUpdateLock，不使用 PC-checked)

cache_fill         (acquires lock)(获取锁)
cache_expand       (only called from cache_fill)(仅从 cache_fill 调用)
cache_create       (only called from cache_expand)(仅从 cache_expand 调用)
bcopy               (only called from instrumented cache_expand)(仅从已检测的 cache_expand 调用)
flush_caches        (acquires lock)(获取锁)
cache_flush        (only called from cache_fill and flush_caches)(仅从 cache_fill 和 flush_caches 调用)
cache_collect_free (only called from cache_expand and cache_flush)(仅从 cache_expand 和 cache_flush 调用)

UNPROTECTED cache readers (NOT thread-safe; used for debug info only)(不是线程安全的；仅用于调试信息)
cache_print cache 打印
_class_printMethodCaches
_class_printDuplicateCacheEntries
_class_printMethodCacheStatistics
*/
```
&emsp;到这里就看完注释了，有点懵，下面还是把源码一行一行看完，然后再回顾上面的内容到底指的是什么。

## CACHE_IMP_ENCODING/CACHE_MASK_STORAGE
&emsp;在进入 `cache_t/bucket_t` 内容之前，首先看两个宏定义，`CACHE_IMP_ENCODING` 表示在 `bucket_t` 中 `IMP` 的存储方式，`CACHE_MASK_STORAGE` 表示 `cache_t` 中掩码的位置。`struct bucket_t` 和 `struct cache_t` 里面的不同实现部分正是根据这两个宏来判断的。我们最关注的 `x86_64(mac)` 和 `arm64(iphone)` 两个平台下 `bucket_t` 中 `IMP` 都是以 `ISA` 与 `IMP` 异或的值存储。而掩码位置的话 `x86_64` 下是 `CACHE_MASK_STORAGE_OUTLINED` 没有掩码，`buckets` 散列数组和 `_mask` 以两个成员变量分别表示。在 `arm64` 下则是 `CACHE_MASK_STORAGE_HIGH_16` 高 `16` 位为掩码，散列数组和 `mask` 共同保存在 `_maskAndBuckets` 中。 
```c++
// 三种方法缓存存储 IMP 的方式：（bucket_t 中 _imp 成员变量存储 IMP 的方式）
// Determine how the method cache stores IMPs. 
// 确定方法缓存如何存储 IMPs. (IMP 是函数实现的指针，保存了函数实现的地址，根据 IMP 可以直接执行函数...)

// Method cache contains raw IMP. 方法缓存包含原始的 IMP（bucket_t 中 _imp 为 IMP）
#define CACHE_IMP_ENCODING_NONE 1 

// Method cache contains ISA ^ IMP. 方法缓存包含 ISA 与 IMP 的异或（bucket_t 中 _imp 是 IMP ^ ISA）
#define CACHE_IMP_ENCODING_ISA_XOR 2 

// Method cache contains ptrauth'd IMP. 
// 方法缓存包含指针验证的 IMP (bucket_t 中 _imp 是 ptrauth_auth_and_resign 函数计算的值)
#define CACHE_IMP_ENCODING_PTRAUTH 3 

// 上述三种方式的各在什么平台使用：

#if __PTRAUTH_INTRINSICS__ // 未找到该宏定义的值

// Always use ptrauth when it's supported. 当平台支持 __PTRAUTH_INTRINSICS__ 时总是使用指针验证
// 此时 CACHE_IMP_ENCODING 为 CACHE_IMP_ENCODING_PTRAUTH

#define CACHE_IMP_ENCODING CACHE_IMP_ENCODING_PTRAUTH

#elif defined(__arm__)

// 32-bit ARM uses no encoding. 32位 ARM 下不进行编码，直接使用原始 IMP(watchOS 下) 
#define CACHE_IMP_ENCODING CACHE_IMP_ENCODING_NONE

#else

// Everything else uses ISA ^ IMP. 其它情况下在方法缓存中存储 ISA 与 IMP 异或的值
#define CACHE_IMP_ENCODING CACHE_IMP_ENCODING_ISA_XOR

#endif
```
```c++
// CACHE 中掩码位置
#define CACHE_MASK_STORAGE_OUTLINED 1 // 没有掩码
#define CACHE_MASK_STORAGE_HIGH_16 2 // 高 16 位
#define CACHE_MASK_STORAGE_LOW_4 3 // 低 4 位

#if defined(__arm64__) && __LP64__ // 如果是 64 位 ARM 平台（iPhone 自 5s 起都是）

// 掩码存储在高 16 位
#define CACHE_MASK_STORAGE CACHE_MASK_STORAGE_HIGH_16 

#elif defined(__arm64__) && !__LP64__ // ARM 平台 非 64 位系统架构（watchOS 下）

// 掩码存储在低 4 位
#define CACHE_MASK_STORAGE CACHE_MASK_STORAGE_LOW_4

#else

// 不使用掩码的方式（_buckets 与 _mask 分别是两个变量，上面则是把 buckets 和 mask 合并保存在 _maskAndBuckets 中）
#define CACHE_MASK_STORAGE CACHE_MASK_STORAGE_OUTLINED

#endif
```
## bucket_t
&emsp;看到 `bucket_t` 一下想起了 `RefcountMap refcnts` 中保存对象引用计数时使用的数据结构 `typename BucketT = detail::DenseMapPair<KeyT, ValueT>>` 用于保存对象的地址和对象的引用计数。`bucket_t` 基本也是大致相同的作用，这里是把函数的 `SEL`  和函数的实现地址 `IMP` 保存在 `bucket_t` 这个结构体中。这里先看一下 `bucket_t` 定义的 `private` 部分:
```c++
struct bucket_t {
private:
    // 为了优化性能，针对 __arm64__ 和其它平台调换 _imp 和 _sel 两个成员变量的顺序
    
    // IMP-first is better for arm64e ptrauth and no worse for arm64.
    // IMP 在前时对 arm64e 的指针验证机制更好，对 arm64 也不差。
    
    // SEL-first is better for armv7* and i386 and x86_64.
    // SEL 在前时对 armv7* i386 x86_64 更好。
    
    // typedef unsigned long uintptr_t;
    // template <typename T> struct explicit_atomic : public std::atomic<T> { ... };
    // 禁止隐式转换，T 操作为原子操作，避免多线程竞争
    
    // 类型一模一样，这里是修改一下 _imp 和 _sel 的前后顺序
#if __arm64__
    explicit_atomic<uintptr_t> _imp;  
    explicit_atomic<SEL> _sel;
#else
    explicit_atomic<SEL> _sel;
    explicit_atomic<uintptr_t> _imp;
#endif
    
    // Compute the ptrauth signing modifier from &_imp, newSel, and cls.
    // 从 ＆_imp、newSel 和 cls 计算 ptrauth 签名修饰符。
    uintptr_t modifierForSEL(SEL newSel, Class cls) const {
        // 连续异或
        return (uintptr_t)&_imp ^ (uintptr_t)newSel ^ (uintptr_t)cls;
    }

    // Sign newImp, with &_imp, newSel, and cls as modifiers.
    // 签署 newImp，使用 ＆_imp、newSel 和 cls 作修改。
    uintptr_t encodeImp(IMP newImp, SEL newSel, Class cls) const {
        // 如果 newImp 为 nil，返回 0
        if (!newImp) return 0;
#if CACHE_IMP_ENCODING == CACHE_IMP_ENCODING_PTRAUTH
    // 如果 IMP 编码使用指针验证机制
        return (uintptr_t)
            ptrauth_auth_and_resign(newImp,
                                    ptrauth_key_function_pointer, 0,
                                    ptrauth_key_process_dependent_code,
                                    modifierForSEL(newSel, cls));
#elif CACHE_IMP_ENCODING == CACHE_IMP_ENCODING_ISA_XOR

        // IMP 与 Class 作异或的值
        return (uintptr_t)newImp ^ (uintptr_t)cls;
#elif CACHE_IMP_ENCODING == CACHE_IMP_ENCODING_NONE

        // 直接使用原始 IMP
        return (uintptr_t)newImp;
#else

#error Unknown method cache IMP encoding. // 未知方式

#endif
    }
...
};
```
&emsp;`bucket_t` 定义的 `public` 部分:
```c++
public:
    // 原子读取 _sel
    inline SEL sel() const { return _sel.load(memory_order::memory_order_relaxed); }

    // 根据 cls 从 bucket_t 实例中取得 _imp
    inline IMP imp(Class cls) const {
        // 首先原子读取 _imp
        uintptr_t imp = _imp.load(memory_order::memory_order_relaxed);
        
        // 如果 imp 不存在，则返回 nil
        if (!imp) return nil;
        
#if CACHE_IMP_ENCODING == CACHE_IMP_ENCODING_PTRAUTH
        // 原子读取 _sel
        SEL sel = _sel.load(memory_order::memory_order_relaxed);
        // 计算返回 IMP
        return (IMP)
            ptrauth_auth_and_resign((const void *)imp,
                                    ptrauth_key_process_dependent_code,
                                    modifierForSEL(sel, cls),
                                    ptrauth_key_function_pointer, 0);
#elif CACHE_IMP_ENCODING == CACHE_IMP_ENCODING_ISA_XOR

        // imp 与 cls 再进行一次异或，返回原值，得到 encodeImp 传入的 newImp（之所以是再，是因为 _imp 存储时就已经做过一次异或了）
        return (IMP)(imp ^ (uintptr_t)cls);
        
#elif CACHE_IMP_ENCODING == CACHE_IMP_ENCODING_NONE

        // 原始 IMP
        return (IMP)imp;
#else

#error Unknown method cache IMP encoding. // 未知

#endif
    }

    // enum Atomicity { Atomic = true, NotAtomic = false };
    // enum IMPEncoding { Encoded = true, Raw = false };
    
    // set 函数 的声明，按住 command 点击 set，可看到 set 函数定义在 objc-cache.mm 中
    template <Atomicity, IMPEncoding>
    void set(SEL newSel, IMP newImp, Class cls);
};
```
### set
&emsp;`set` 函数完成的功能是以原子方式完成 `bucket_t` 实例 `_imp` 和 `_sel` 成员变量的设置。 

`memory_order` 的值可参考: [《如何理解 C++11 的六种 memory order？》](https://www.zhihu.com/question/24301047)
```c++
// 非 __arm64__ 平台下(x86_64 下):
template<Atomicity atomicity, IMPEncoding impEncoding>
void bucket_t::set(SEL newSel, IMP newImp, Class cls)
{
    // 当前 bucket_t 实例的 _sel 为 0 或者与传入的 newSel 相同
    // DEBUG 下 bucket_t 的 _sel 和 newSel 不同就会执行断言 ？
    
    ASSERT(_sel.load(memory_order::memory_order_relaxed) == 0 ||
           _sel.load(memory_order::memory_order_relaxed) == newSel);

    // objc_msgSend uses sel and imp with no locks.
    // objc_msgSend 使用 sel 和 imp 不会加锁。
    
    // It is safe for objc_msgSend to see new imp but NULL sel 
    // objc_msgssend 可以安全地看到新的 imp 和 NULL 的 sel。
    
    // (It will get a cache miss but not dispatch to the wrong place.
    //  它将导致缓存未命中，但不会分派到错误的位置。)
    
    // It is unsafe for objc_msgSend to see old imp and new sel.
    // objc_msgSend 查看旧的 imp 和新的 sel 是不安全的。
    
    // Therefore we write new imp, wait a lot, then write new sel.
    // 因此，我们首先写入新的 imp，等一下，然后再写入新的 sel。
    
    // 根据 impEncoding 判断 新 IMP 是需要做异或求值还是直接使用
    uintptr_t newIMP = (impEncoding == Encoded
                        ? encodeImp(newImp, newSel, cls)
                        : (uintptr_t)newImp);

    if (atomicity == Atomic) {
        // 如果是 Atomic
        // 首先把 newIMP 存储到 _imp
        _imp.store(newIMP, memory_order::memory_order_relaxed);
        
        // _sel 是 0 时：
        if (_sel.load(memory_order::memory_order_relaxed) != newSel) {
        // 如果当前 _sel 与 newSel 不同，则根据不同的平台来设置 _sel
        
#ifdef __arm__
            // barrier
            mega_barrier();
            _sel.store(newSel, memory_order::memory_order_relaxed);
#elif __x86_64__ || __i386__
            _sel.store(newSel, memory_order::memory_order_release);
#else

// 不知道如何在此架构上执行 bucket_t::set。
#error Don't know how to do bucket_t::set on this architecture.

#endif
        }
    } else {
        // 原子保存 _imp
        _imp.store(newIMP, memory_order::memory_order_relaxed);
        // 原子保存 _sel
        _sel.store(newSel, memory_order::memory_order_relaxed);
    }
}
```
&emsp;首先要把 `newImp` 写入，`__arm64__` 下 `set` 函数的实现涉及一个 `__asm__` 好像涉及到 `ARM` 的内存排序内存屏障啥的看不懂。
`struct bucket_t` 到这里就结束了，主要用来保存函数的 `SEL` 和 `IMP`（`IMP` 根据不同的编码方式来保存）。 

## cache_t
&emsp;`cache_t` 是作为一个散列数组来缓存方法的。先看下 `cache_t` 定义的 `private` 部分:

### mask_t:
```c++
#if __LP64__

// x86_64 & arm64 asm are less efficient with 16-bits
// x86_64 和 arm64 asm 的 16 位效率较低

typedef uint32_t mask_t; // 32 位 4 字节 int
#else

typedef uint16_t mask_t; // 16 位 2 字节 int
#endif
```
### struct cache_t private
```c++
struct cache_t {
#if CACHE_MASK_STORAGE == CACHE_MASK_STORAGE_OUTLINED
    // 如果没有掩码的话
    
    // _buckets 是 struct bucket_t 类型的数组
    // 方法的缓存数组（以散列表的形式存储 bucket_t）
    explicit_atomic<struct bucket_t *> _buckets;
    
    // _buckets 的数组长度 -1（容量的临界值）
    explicit_atomic<mask_t> _mask;
    
#elif CACHE_MASK_STORAGE == CACHE_MASK_STORAGE_HIGH_16
    // 高 16 位是掩码的平台（iPhone 5s 以后的真机）
    
    // 掩码和 Buckets 指针共同保存在 uintptr_t 类型的 _maskAndBuckets 中
    explicit_atomic<uintptr_t> _maskAndBuckets;
    
    mask_t _mask_unused; // 未使用的容量
    
    // How much the mask is shifted by.
    // 高 16 位是 mask，即 _maskAndBuckets 右移 48 位得到 mask
    static constexpr uintptr_t maskShift = 48;
    
    // Additional bits after the mask which must be zero. 
    // msgSend takes advantage of these additional bits to construct the value `mask << 4` from `_maskAndBuckets` in a single instruction.
    // 掩码后的其他位必须为零。
    // msgSend 利用这些额外的位在单个指令中从 _maskAndBuckets 构造了值 mask<< 4
    static constexpr uintptr_t maskZeroBits = 4;
    
    // The largest mask value we can store.
    // 我们可以保存的最大的 mask 值。
    // (64 - maskShiift) 即掩码位数，然后 1 左移掩码位数后再 减 1 即 16 位能保存的最大二进制值
    //（16 位 1，其余位都是 0 的数值）
    static constexpr uintptr_t maxMask = ((uintptr_t)1 << (64 - maskShift)) - 1;
    
    // The mask applied to `_maskAndBuckets` to retrieve the buckets pointer.
    // 应用于 _maskAndBuckets 的掩码，以获取 buckets 指针。
    // 1 左移 44(48 - 4) 位后再 减 1（44 位 1，其余都是 0 的数值）
    static constexpr uintptr_t bucketsMask = ((uintptr_t)1 << (maskShift - maskZeroBits)) - 1;
    
    // Ensure we have enough bits for the buckets pointer.
    // 确保我们有足够的位用于存储 buckets 指针。
    static_assert(bucketsMask >= MACH_VM_MAX_ADDRESS, "Bucket field doesn't have enough bits for arbitrary pointers.");
    
#elif CACHE_MASK_STORAGE == CACHE_MASK_STORAGE_LOW_4
    // _maskAndBuckets stores the mask shift in the low 4 bits, and the buckets pointer in the remainder of the value. 
    // The mask shift is the value where (0xffff >> shift) produces the correct mask.
    // This is equal to 16 - log2(cache_size).
    // _maskAndBuckets 将掩码移位存储在低 4 位中，并将 buckets 指针存储在该值的其余部分中。
    // 掩码 shift 是（0xffff >> shift）产生正确掩码的值。
    // 等于 16 - log2(cache_size)
    
    // 几乎同上
    explicit_atomic<uintptr_t> _maskAndBuckets;
    mask_t _mask_unused;

    static constexpr uintptr_t maskBits = 4;
    static constexpr uintptr_t maskMask = (1 << maskBits) - 1;
    static constexpr uintptr_t bucketsMask = ~maskMask;
#else

#error Unknown cache mask storage type. // 未知掩码存储类型

#endif

#if __LP64__
    // 如果是 64 位环境的话会多一个 _flags 标志位
    uint16_t _flags;
#endif

    uint16_t _occupied; // 缓存数组的已占用量
...
};
```
### struct cache_t public
&emsp;`cache_t` 定义的 `public` 部分: `cache_t` 的实现部分也是涉及到不同的平台下不同的实现，这里只分析 `CACHE_MASK_STORAGE_OUTLINED(x86_64)` 和 `CACHE_MASK_STORAGE_HIGH_16 (__arm64__ && __LP64__)` 两个平台的实现。

#### emptyBuckets
&emsp;一个指向 `_objc_empty_cache` 的 `bucket_t` 指针，用来指示当前类的缓存指向空缓存。（`_objc_empty_cache` 是一个外联变量）
```c++
// OBJC2 不可见
struct objc_cache {
    unsigned int mask /* total = mask + 1 */                 OBJC2_UNAVAILABLE;
    unsigned int occupied                                    OBJC2_UNAVAILABLE;
    Method _Nullable buckets[1]                              OBJC2_UNAVAILABLE;
};

OBJC_EXPORT struct objc_cache _objc_empty_cache OBJC_AVAILABLE(10.0, 2.0, 9.0, 1.0, 2.0);

// 位于 objc-cache-old.mm 中
// 静态的空缓存，所有类最初都指向此缓存。
// 发送第一条消息时，它在缓存中未命中，当缓存新增时，它会检查这种情况，并使用 malloc 而不是 realloc。
// 这避免了在 Messenger 中检查 NULL 缓存的需要。
struct objc_cache _objc_empty_cache =
{
    0,        // mask
    0,        // occupied
    { NULL }  // buckets
};

// CACHE_MASK_STORAGE_OUTLINED 下
struct bucket_t *cache_t::emptyBuckets()
{
    // 直接使用 & 取 _objc_empty_cache 的地址并返回，
    // _objc_empty_cache 是一个全局变量，用来标记当前类的缓存是一个空缓存。
    return (bucket_t *)&_objc_empty_cache;
}

// CACHE_MASK_STORAGE_HIGH_16 下（看到和 OUTLINED 完全一样）
struct bucket_t *cache_t::emptyBuckets()
{
    return (bucket_t *)&_objc_empty_cache;
}
```
#### buckets
&emsp;散列数组的起始地址。
```c++
// CACHE_MASK_STORAGE_OUTLINED 下
// 没有任何嗦头，原子加载 _buckets 并返回
struct bucket_t *cache_t::buckets() 
{
    // 原子加载 _buckets 并返回
    return _buckets.load(memory_order::memory_order_relaxed);
}

// CACHE_MASK_STORAGE_HIGH_16 下
// 也是没有任何嗦头，原子加载 _maskAndBuckets，然后与 bucketsMask 掩码与操作并把结果返回
struct bucket_t *cache_t::buckets()
{
    // 原子加载 _maskAndBuckets
    uintptr_t maskAndBuckets = _maskAndBuckets.load(memory_order::memory_order_relaxed);
    // 然后与 bucketsMask 做与操作并返回结果。
    //（bucketsMask 的值是低 44 位是 1，其它位全部是 0，与操作取出 maskAndBuckets 低 44 位的值）
    return (bucket_t *)(maskAndBuckets & bucketsMask);
}
```

#### mask
&emsp;`_buckets` 的数组长度 -1（容量的临界值）。
```c++
// CACHE_MASK_STORAGE_OUTLINED
mask_t cache_t::mask() 
{
    // 没有任何嗦头，原子加载 _mask 并返回
    return _mask.load(memory_order::memory_order_relaxed);
}

// CACHE_MASK_STORAGE_HIGH_16
mask_t cache_t::mask()
{
    // 原子加载 _maskAndBuckets
    uintptr_t maskAndBuckets = _maskAndBuckets.load(memory_order::memory_order_relaxed);
    // maskAndBuckets 左移 48 位即得到了 mask，并返回此值。（高 16 位保存 mask）
    return maskAndBuckets >> maskShift;
}

#if __LP64__
typedef uint32_t mask_t;  // x86_64 & arm64 asm are less efficient with 16-bits
#else
typedef uint16_t mask_t;
#endif

typedef uintptr_t SEL;
```
&emsp;这里有一个点，在 `CACHE_MASK_STORAGE_HIGH_16` 时是 `__LP64__` 平台，`mask_t` 在 `__LP64__` 下是 `uint32_t`，多出了 `16` 位空间，`mask` 只需要 `16` 位就足够保存。注释给出的解释是: " `x86_64` 和 `arm64` `asm` 的 `16` 位效率较低。"

#### occupied/incrementOccupied
```c++
mask_t cache_t::occupied() 
{
    return _occupied; // 返回 _occupied
}

void cache_t::incrementOccupied() 
{
    _occupied++; // _occupied 自增
}
```
#### setBucketsAndMask
&emsp;设置 `_buckets` 与 `_mask` 的值，`CACHE_MASK_STORAGE_OUTLINED` 模式只需要分别以原子方式设置两个成员变量的值即可，`CACHE_MASK_STORAGE_HIGH_16` 模式需要把两个值做位操作合并在一起然后以原子方式保存在 `_maskAndBuckets` 中。同时以上两种情况都会顺便把 `_occupied` 设置为 `0`。
```c++
// CACHE_MASK_STORAGE_OUTLINED
void cache_t::setBucketsAndMask(struct bucket_t *newBuckets, mask_t newMask)
{
    // objc_msgSend uses mask and buckets with no locks.
    // objc_msgSend 使用 mask 和 buckets 不会进行加锁。
    
    // It is safe for objc_msgSend to see new buckets but old mask.
    // 对于 objc_msgSend 来说，看到新的 buckets 和旧的 mask 是安全的。
    
    // (It will get a cache miss but not overrun the buckets' bounds).
    // (它将获得缓存未命中，但不会超出存储桶的界限。)
    
    // It is unsafe for objc_msgSend to see old buckets and new mask.
    // objc_msgSend 查看旧 buckets 和新 mask 是不安全的。
    
    // Therefore we write new buckets, wait a lot, then write new mask.
    // 所以我们先写入新的 buckets，写入完成后，再写入新的 mask。
    
    // objc_msgSend reads mask first, then buckets.
    // objc_msgSend 首先读取 mask，然后读取 buckets。

#ifdef __arm__
    // ensure other threads see buckets contents before buckets pointer
    // 确保其他线程在 buckets 指针之前看到 buckets 内容
    mega_barrier();

    _buckets.store(newBuckets, memory_order::memory_order_relaxed);
    
    // ensure other threads see new buckets before new mask
    // 确保其他线程在新 mask 之前看到新 buckets
    mega_barrier();
    
    _mask.store(newMask, memory_order::memory_order_relaxed);
    _occupied = 0; // _occupied 置为 0
#elif __x86_64__ || i386
    // ensure other threads see buckets contents before buckets pointer
    // 确保其他线程在 buckets 指针之前看到 buckets 内容
    // 以原子方式保存 _buckets
    _buckets.store(newBuckets, memory_order::memory_order_release);
    
    // ensure other threads see new buckets before new mask
    // 确保其他线程在新 mask 之前看到新 buckets
    // 以原子方式保存 _mask
    _mask.store(newMask, memory_order::memory_order_release);
    _occupied = 0; // _occupied 置为 0
#else

// 不知道如何在此架构上执行 setBucketsAndMask。
#error Don't know how to do setBucketsAndMask on this architecture.

#endif
}

// CACHE_MASK_STORAGE_HIGH_16
void cache_t::setBucketsAndMask(struct bucket_t *newBuckets, mask_t newMask)
{
    // 转为 unsigned long
    uintptr_t buckets = (uintptr_t)newBuckets;
    uintptr_t mask = (uintptr_t)newMask;
    
    // 断言: buckets 小于等于 buckets 的掩码（bucketsMask 的值低 44 位全为 1，其它位是 0）
    ASSERT(buckets <= bucketsMask);
    // 断言: mask 小于等于 mask 的最大值（maxMask 的值低 16 位全为 1，其它位是 0）
    ASSERT(mask <= maxMask);
    
    // newMask 左移 48 位然后与 newBuckets 做或操作，
    // 因为 newBuckets 高 16 位全部是 0，所以 newMask 左移 16 的值与 newBuckets 做或操作时依然保持不变
    // 把结果以原子方式保存在 _maskAndBuckets 中  
    _maskAndBuckets.store(((uintptr_t)newMask << maskShift) | (uintptr_t)newBuckets, std::memory_order_relaxed);
    // 把 _occupied 置为 0
    _occupied = 0;
}
```
#### initializeToEmpty
```c++
// bzero
// 头文件：#include <string.h>
// 函数原型：void bzero (void *s, int n);
// 功能：将字符串 s 的前 n 个字节置为 0，一般来说 n 通常取 sizeof(s)，将整块空间清零

// CACHE_MASK_STORAGE_OUTLINED
void cache_t::initializeToEmpty()
{
    // 把 this 的内存清零
    bzero(this, sizeof(*this));
    // 以原子方式把 _objc_empty_cache 的地址存储在 _buckets 中
    _buckets.store((bucket_t *)&_objc_empty_cache, memory_order::memory_order_relaxed);
}

// CACHE_MASK_STORAGE_HIGH_16
void cache_t::initializeToEmpty()
{
    // 把 this 的内存清零
    bzero(this, sizeof(*this));
    // 把 _objc_empty_cache 的地址转换为 uintptr_t然后以原子方式把其存储在 _maskAndBuckets 中 
    _maskAndBuckets.store((uintptr_t)&_objc_empty_cache, std::memory_order_relaxed);
}
```
&emsp;两种模式下都是把 `_objc_empty_cache` 的地址取出用于设置 `_buckets/_maskAndBuckets`，两种模式下也都对应上面的 `emptyBuckets` 函数，取出 `(bucket_t *)&_objc_empty_cache` 返回。  
#### canBeFreed
&emsp;`canBeFreed` 函数只有下面一种实现，看名字我们大概也能猜出此函数的作用，正式判断能不能释放 `cache_t`。
```c++
bool cache_t::canBeFreed()
{
    // 调用 isConstantEmptyCache 函数，如果它返回 true，
    // 则表明 cache_t 的 buckets 当前正是那些准备的标记 emptyBuckets 的静态值。
    
    //（当 capacity 小于 capacity 时，
    // isConstantEmptyCache 函数内部的 emptyBucketsForCapacity 函数返回的都是:
    // cache_t::emptyBuckets() 全局的 (bucket_t *)&_objc_empty_cache 值），则不能进行释放，
    
    // 我们自己申请的有效的方法缓存内容，才可根据情况进行释放。
    
    return !isConstantEmptyCache();
}
```
#### isConstantEmptyCache
&emsp;看完下面的 `emptyBucketsForCapacity` 实现才知道 `isConstantEmptyCache` 中 `Constant` 的含义。
```c++
bool cache_t::isConstantEmptyCache()
{
    // occupied() 函数很简单就是获取 _occupied 成员变量的值然后直接返回，
    // _occupied 表示散列表中已占用的容量
    // 此处要求 occupied() 为 0 并且 buckets() 等于 emptyBucketsForCapacity(capacity(), false)
    // emptyBucketsForCapacity 函数则是根据 capacity() 去找其对应的 emptyBuckets，
    // 且这些 emptyBuckets 地址都是固定的，
    // 它们是作标记用的静态值，如果此时 buckets 正是这些个静态值，说明此时 cache_t 是一个空缓存。
    return 
        occupied() == 0  &&  
        buckets() == emptyBucketsForCapacity(capacity(), false); 
        // 且这里用了 false 则下面不执行 emptyBucketsList 相关的申请空间的逻辑，
        // 会直接 if (!allocate) return nil;
}
```
#### capacity
```c++
// mask 是临界值，加 1 后就是散列表的容量
unsigned cache_t::capacity()
{
    return mask() ? mask()+1 : 0; 
}
```
### emptyBucketsForCapacity
&emsp;根据入参 `capacity`，返回一个指定 `capacity` 容量的空的散列表，返回的这个 `bucket_t *` 是  `static bucket_t **emptyBucketsList` 这个静态变量指定下标的值，当 `capacity` 位于指定的区间时，返回的 `bucket_t *` 都是相同的。如果 `capacity` 超出了现有的容量界限，则会对 `emptyBucketsList` 进行扩容。
例如：`capacity` 值在 `[8，15]` 之内时，通过 `index = log2u(capacity)` 计算的 `index` 值都是相同的，那么调用 `emptyBucketsForCapacity` 函数返回的都是相同的 `emptyBucketsList[index]`。由于这里有 `EMPTY_BYTES` 限制，所以至少 `capacity`  大于 `9/1025` 才会使用到 `emptyBucketsList` 相关的逻辑，内部 `index` 是从 `3/10`（2 的 3 次方是 8，2 的 10 次方是 1024） 开始的。其它的情况则一律返回 `cache_t::emptyBuckets()`。
```c++
bucket_t *emptyBucketsForCapacity(mask_t capacity, bool allocate = true)
{
#if CONFIG_USE_CACHE_LOCK
    cacheUpdateLock.assertLocked();
#else
    runtimeLock.assertLocked(); // 走此分支，加锁（加锁失败会执行断言）
#endif

    // buckets 总字节占用（sizeof(bucket_t) * capacity）
    size_t bytes = cache_t::bytesForCapacity(capacity);
    
    // Use _objc_empty_cache if the buckets is small enough.
    // 如果 buckets 足够小的话使用 _objc_empty_cache。
    if (bytes <= EMPTY_BYTES) {
        // 小于 ((8+1)*16) 主要针对的是 DEBUG 模式下。
        // 小于 ((1024+1)*16) 非 DEBUG 模式（后面的乘以 16 是因为 sizeof(bucket_t) == 16）
        //（觉得这个 1025 的容量就已经很大大了，可能很难超过，大概率这里就直接返回 cache_t::emptyBuckets() 了）
        return cache_t::emptyBuckets();
    }

    // Use shared empty buckets allocated on the heap.
    // 使用在堆上分配的 shared empty buckets。
    
    // 静态的 bucket_t **，下次再进入 emptyBucketsForCapacity 函数的话依然是保持上次的值
    // 且返回值正是 emptyBucketsList[index]，就是说调用 emptyBucketsForCapacity 获取就是一个静态的定值
    static bucket_t **emptyBucketsList = nil;
    
    // 静态的 mask_t (uint32_t)，下次再进入 emptyBucketsForCapacity 函数的话依然是保持上次的值
    static mask_t emptyBucketsListCount = 0;
    
    // ⚠️
    // template <typename T>
    // static inline T log2u(T x) {
    //     return (x<2) ? 0 : log2u(x>>1)+1;
    // }
    
    // log2u 计算的是小于等于 x 的最大的 2 幂的指数
    // x 在 [8，15] 区间内，大于等于 2^3，所以返回值为 3
    // x 在 [16, 31] 区间内，大于等于 2^4, 所以返回值为 4
    
    mask_t index = log2u(capacity);

    if (index >= emptyBucketsListCount) {
        if (!allocate) return nil;

        // index + 1 此值还没有看出来是什么意思
        mask_t newListCount = index + 1;
        
        // capacity 大于 9/1026 那么 bytes 大于 9 * 16/1026 * 16，16 Kb 也可太大了
        // 分配 bytes 个长度为 1 的连续内存空间，且内存初始化为 0
        bucket_t *newBuckets = (bucket_t *)calloc(bytes, 1);
        
        // ⚠️
        // extern void *realloc(void *mem_address, unsigned int newsize);
        //（数据类型*）realloc（要改变内存大小的指针名，新的大小）
        // 新的大小可大可小，如果新的大小大于原内存大小，则新分配部分不会被初始化；如果新的大小小于原内存大小，可能会导致数据丢失。
        // 注意事项: 重分配成功旧内存会被自动释放，旧指针变成了野指针。
        // 返回值: 如果重新分配成功则返回指向被分配内存的指针，否则返回空指针 NULL。
        
        // 先判断当前的指针是否有足够的连续空间，如果有，扩大 mem_address 指向的地址，并且将 mem_address 返回，
        // 如果空间不够，先按照 newsize 指定的大小分配空间，将原有数据从头到尾拷贝到新分配的内存区域，
        // 而后释放原来 mem_address 所指内存区域（注意：原来指针是自动释放，不需要使用 free），
        // 同时返回新分配的内存区域的首地址，即重新分配存储器块的地址。
        
        // 对 emptyBucketsList 进行扩容
        emptyBucketsList = (bucket_t**)
            realloc(emptyBucketsList, newListCount * sizeof(bucket_t *));
            
        // Share newBuckets for every un-allocated size smaller than index.
        // 对于每个小于索引的未分配大小，Share newBucket。
        // The array is therefore always fully populated.
        // 因此，array 始终总是完全填充。
        
        // 把新扩容的 emptyBucketsList 的新位置上都放上 newBuckets
        for (mask_t i = emptyBucketsListCount; i < newListCount; i++) {
            // 把新扩容的 emptyBucketsList 的 新位置上都放上 newBuckets
            emptyBucketsList[i] = newBuckets;
        }
        
        // 更新 emptyBucketsListCount，且 emptyBucketsListCount 是函数内的静态局部变量，
        // 函数进来 emptyBucketsListCount 都保持上次的值
        emptyBucketsListCount = newListCount;
        
        // OPTION( PrintCaches, OBJC_PRINT_CACHE_SETUP, "log processing of method caches")
        if (PrintCaches) {
            // 如果开启了 OBJC_PRINT_CACHE_SETUP 则打印 
            _objc_inform("CACHES: new empty buckets at %p (capacity %zu)", 
                         newBuckets, (size_t)capacity);
        }
    }

    return emptyBucketsList[index];
}
```
#### CONFIG_USE_CACHE_LOCK
&emsp;`emptyBucketsForCapacity` 函数实现第一行就是一个 `CONFIG_USE_CACHE_LOCK` 宏定义，它是用来标志 `emptyBucketsForCapacity` 函数使用 `cacheUpdateLock` 还是 `runtimeLock`，注意这里针对的是 `Objective-C` 的版本，`__OBJC2__` 下使用的是 `runtimeLock` 否则使用 `cacheUpdateLock`。
```c++
// OBJC_INSTRUMENTED controls whether message dispatching is dynamically monitored.
// OBJC_INSTRUMENTED 控制是否动态监视消息调度。

// Monitoring introduces substantial overhead.
// 监控会带来大量开销。

// NOTE: To define this condition, do so in the build command, NOT by uncommenting the line here.  
// NOTE: 若要定义此条件，请在 build 命令中执行此操作，而不是取消下面 OBJC_INSTRUMENTED 的注释。

// This is because objc-class.h heeds this condition, but objc-class.h can not 
// #include this file (objc-config.h) because objc-class.h is public and objc-config.h is not.
// 这是因为 objc-class.h 注意到了这个条件，但是 objc-class.h 不能包括这个文件（objc-config.h），
// 因为 objc-class.h 是公共的，而 objc-config.h 不是。

//#define OBJC_INSTRUMENTED

// --------------- 以上与 CONFIG_USE_CACHE_LOCK 无关

// In __OBJC2__, the runtimeLock is a mutex always held hence the cache 
// lock is redundant and can be elided.
// 在 __OBJC2__ 中，runtimeLock 是 "始终保持" 的互斥锁，因此 cache lock 是多余的，可以忽略。
// (始终保持那里的意思是指 runtime Lock 始终是一个互斥锁吗？)

// If the runtime lock ever becomes a rwlock again, the cache lock would need to be used again.
// 如果 runtime lock 再次变为 rwlock，则需要再次使用 cache lock。

// 可在 objc-runtime-new.mm 中看到如下定义，表明 cacheUpdateLock 只是一个在旧版本中使用的锁
// #if CONFIG_USE_CACHE_LOCK
// mutex_t cacheUpdateLock;
// #endif

#if __OBJC2__

#define CONFIG_USE_CACHE_LOCK 0 // Objective-C 2.0 下是 0 

#else

#define CONFIG_USE_CACHE_LOCK 1 //  其他情况下是 1

#endif

// 对应与如下调用：
#if CONFIG_USE_CACHE_LOCK
    cacheUpdateLock.assertLocked();
#else
    // extern mutex_t runtimeLock; 在 objc-locks-new.h 中一个外联声明
    // mutex_t runtimeLock; 在 objc-runtime-new.mm Line 75 定义（类型是互斥锁） 
    
    // assertLocked 进行加锁，如果加锁失败是导致断言
    runtimeLock.assertLocked(); // 在 __OBJC2__ 下是使用 runtimeLock
#endif
```
#### bytesForCapacity
&emsp;`bucket_t` 散列数组的总的内存占用（以字节为单位）。
```c++
size_t cache_t::bytesForCapacity(uint32_t cap)
{   
    // 总容量乘以每个 bucket_t 的字节大小
    return sizeof(bucket_t) * cap;
}
```
#### EMPTY_BYTES
```c++
// EMPTY_BYTES includes space for a cache end marker bucket.
// EMPTY_BYTES 是包括 缓存结束标记 bucket 的空间。

// This end marker doesn't actually have the wrap-around pointer 
// because cache scans always find an empty bucket before they might wrap.
// 这个结束标记实际上没有 wrap-around 指针，
// 因为缓存扫描总是在可能进行换行之前找到一个空的 bucket。(因为 buckets 的扩容机制)

// 1024 buckets is fairly common.
// 1024 bukcets 很常见。

// 一个 bucket_t 的实例变量的大小应该是 16 字节。

#if DEBUG
    // Use a smaller size to exercise heap-allocated empty caches.
    // 使用较小的容量来执行堆分配的空缓存。
#   define EMPTY_BYTES ((8+1)*16)
#else
#   define EMPTY_BYTES ((1024+1)*16)
#endif
```
### getBit/setBit/clearBit
&emsp;针对 `__LP64__` 平台下的 `_flags` 的操作。[atomic_fetch_or/atomic_fetch_and](https://en.cppreference.com/w/cpp/atomic/atomic_fetch_or)
```c++
#if __LP64__
    bool getBit(uint16_t flags) const {
        return _flags & flags;
    }
    void setBit(uint16_t set) {
        __c11_atomic_fetch_or((_Atomic(uint16_t) *)&_flags, set, __ATOMIC_RELAXED);
    }
    void clearBit(uint16_t clear) {
        __c11_atomic_fetch_and((_Atomic(uint16_t) *)&_flags, ~clear, __ATOMIC_RELAXED);
    }
#endif
```
### FAST_CACHE_ALLOC_MASK
```c++
// Fast Alloc fields:
// This stores the word-aligned size of instances + "ALLOC_DELTA16", 
// or 0 if the instance size doesn't fit.
// 它存储实例的字对齐大小 + "ALLOC_DELTA16"，如果实例大小不适合，则存储 0。

// These bits occupy the same bits than in the instance size, 
// so that the size can be extracted with a simple mask operation.
// 这些位占用与实例大小相同的位，因此可以通过简单的掩码操作提取大小。

// FAST_CACHE_ALLOC_MASK16 allows to extract the instance size rounded
// rounded up to the next 16 byte boundary, which is a fastpath for _objc_rootAllocWithZone()
// FAST_CACHE_ALLOC_MASK16 允许提取四舍五入到下一个 16 字节边界的实例大小，
// 这是 _objc_rootAllocWithZone() 的快速路径

#define FAST_CACHE_ALLOC_MASK         0x1ff8 // 0b0001 1111 1111 1000
#define FAST_CACHE_ALLOC_MASK16       0x1ff0 // 0b0001 1111 1111 0000
#define FAST_CACHE_ALLOC_DELTA16      0x0008 // 0b0000 0000 0000 1000
```
### hasFastInstanceSize/fastInstanceSize/setFastInstanceSize
&emsp;在 `__LP64__` 平台下，`cache_t` 多了一个 `uint16_t _flags`。以下函数是根据 `_flags` 中的一些标志位做出不同的处理。这些个函数内容都是给 `objc_class` 用的，本篇的内容是针对的都是方法缓存的学习，等到 `objc_class` 篇再详细分析下面的函数。
```c++
#if FAST_CACHE_ALLOC_MASK
    bool hasFastInstanceSize(size_t extra) const
    {
        if (__builtin_constant_p(extra) && extra == 0) {
            // 如果 extra 在编译时是常量并且值为 0，则 _flags & 0b0001 1111 1111 0000 的值返回，
            // 判断 cache_t 是否有快速实例化的大小
            return _flags & FAST_CACHE_ALLOC_MASK16;
        }
        // 返回 _flags & 0b0001 1111 1111 1000 的值
        return _flags & FAST_CACHE_ALLOC_MASK;
    }

    // 快速实例化的大小
    size_t fastInstanceSize(size_t extra) const
    {   
        // 断言
        ASSERT(hasFastInstanceSize(extra));

        if (__builtin_constant_p(extra) && extra == 0) {
            // 如果 extra 在编译时是常量并且值为 0，则直接返回 _flags & 0b0001 1111 1111 0000 的值
            return _flags & FAST_CACHE_ALLOC_MASK16;
        } else {
            // size = _flags & 0b0001 1111 1111 1000
            size_t size = _flags & FAST_CACHE_ALLOC_MASK;
            
            // remove the FAST_CACHE_ALLOC_DELTA16 that was added by setFastInstanceSize
            // 删除由 setFastInstanceSize 添加的 FAST_CACHE_ALLOC_DELTA16
            
            // static inline size_t align16(size_t x) {
            //     return (x + size_t(15)) & ~size_t(15);
            // }
            // align16 函数是计算大于等于 x 的最小的 16 的倍数，即计算 16 字节对齐时的长度
            // 0b1000 8
            // 0b1111 15
            // 0b0001 0111  8 + 15 = 23
            // &
            // 0b0000 ~15
            // 0b0001 0000 16
            
            // 16 字节对齐
            return align16(size + extra - FAST_CACHE_ALLOC_DELTA16);
        }
    }

    // 设置快速实例化的大小
    void setFastInstanceSize(size_t newSize)
    {
        // Set during realization or construction only. No locking needed.
        // 仅在 实现 或 构造 期间设置。不需要加锁。
        // #define FAST_CACHE_ALLOC_MASK         0x1ff8 // 0b0001 1111 1111 1000
        // _flags & 0b1110 0000 0000 0111
        
        uint16_t newBits = _flags & ~FAST_CACHE_ALLOC_MASK;
        uint16_t sizeBits;

        // Adding FAST_CACHE_ALLOC_DELTA16 allows for FAST_CACHE_ALLOC_MASK16 to yield
        // the proper 16byte aligned allocation size with a single mask.
        // 添加 FAST_CACHE_ALLOC_DELTA16 允许 FAST_CACHE_ALLOC_MASK16 通过单个掩码产生正确的 16 字节对齐的分配大小。
        
        // #ifdef __LP64__
        // #   define WORD_MASK 7UL
        // #else
        // #   define WORD_MASK 3UL
        // #endif
        
        // 0b0101 5
        // 0b0111 7
        // 0b1100 5 + 7
        // 0b1000 ~7
        // &
        // 0b1000 // 8
        
        // static inline size_t word_align(size_t x) {
        //     return (x + WORD_MASK) & ~WORD_MASK;
        // }
        // word_align 函数是进行字对齐，即 8 字节对齐
        
        // newSize 8 字节对齐 
        sizeBits = word_align(newSize) + FAST_CACHE_ALLOC_DELTA16;
        
        // 与操作
        sizeBits &= FAST_CACHE_ALLOC_MASK;
        
        if (newSize <= sizeBits) {
            // 或操作
            newBits |= sizeBits;
        }
        
        // _flags 赋值
        _flags = newBits;
    }
#else
    // 不支持 FAST_CACHE_ALLOC_MASK 时，返回 false
    bool hasFastInstanceSize(size_t extra) const {
        return false;
    }
    size_t fastInstanceSize(size_t extra) const {
        // 直接 abort
        abort();
    }
    void setFastInstanceSize(size_t extra) {
        // nothing
    }
#endif
```
#### __builtin_constant_p
&emsp;`__builtin_constant_p` 是编译器 `gcc` 内置函数，用于判断一个值是否为编译时常量，如果是常数，函数返回 `1 `，否则返回 `0`。此内置函数的典型用法是在宏中用于手动编译时优化。
```c++
// 在 main.m 中做如下测试：
printf("😊😊 %d\n", __builtin_constant_p(101)); // 打印: 😊😊 1

constexpr int a = 12 * 13;
printf("😊😊 %d\n", __builtin_constant_p(a)); // 打印: 😊😊 1

int a = 12 * 13;
printf("😊😊 %d\n", __builtin_constant_p(a)); // 打印: 😊😊 0
```
### endMarker
```c++
// CACHE_END_MARKER 值为 1 时，定义 endMarker 函数
bucket_t *cache_t::endMarker(struct bucket_t *b, uint32_t cap)
{
    // 最后一个 bucket_t 的指针，-1 是从内存末尾再前移一个 bucket_t 的宽度，
    // 这里是先把 bucket_t 指针转化为一个 unsigned long，然后加上 cap 的字节总数，
    // 然后转化为 bucket_t 指针，然后再退一个指针的宽度，即 cache_t 哈希数组的最后一个 bucket_t 的位置。
    return (bucket_t *)((uintptr_t)b + bytesForCapacity(cap)) - 1;
}
```
#### CACHE_END_MARKER
&emsp;标记是否支持 `cache_t` 的 `buckets` 散列数组的内存末尾标记。
```c++
#if __arm__  ||  __x86_64__  ||  __i386__

// objc_msgSend has few registers available.
// objc_msgSend 可用寄存器很少。

// Cache scan increments and wraps at special end-marking bucket.
// 缓存扫描增量包裹在特殊的末端标记桶上。

#define CACHE_END_MARKER 1 // 定为 1

static inline mask_t cache_next(mask_t i, mask_t mask) {
    return (i+1) & mask;
}

#elif __arm64__

// objc_msgSend has lots of registers available.
// objc_msgSend 有很多可用的寄存器。

// Cache scan decrements. No end marker needed.
// 缓存扫描减量，无需结束标记。

#define CACHE_END_MARKER 0 // 定为 0

static inline mask_t cache_next(mask_t i, mask_t mask) {
    return i ? i-1 : mask;
}

#else

// 未知的架构
#error unknown architecture

#endif
```
### reallocate
```c++
ALWAYS_INLINE
void cache_t::reallocate(mask_t oldCapacity, mask_t newCapacity, bool freeOld)
{
    // 一个临时变量用于记录旧的散列表
    bucket_t *oldBuckets = buckets();
    
    // 为新散列表申请指定容量的空间
    bucket_t *newBuckets = allocateBuckets(newCapacity);

    // Cache's old contents are not propagated.
    // 缓存的旧内容不会传播。
    
    // This is thought to save cache memory at the cost of extra cache fills.
    // 这被认为是以额外的缓存填充为代价来节省缓存内存的。
    // fixme re-measure this 重新测量

    ASSERT(newCapacity > 0);
    ASSERT((uintptr_t)(mask_t)(newCapacity-1) == newCapacity-1);
    
    // 设置 buckets 和 mask
    setBucketsAndMask(newBuckets, newCapacity - 1);
    
    if (freeOld) {
        // 这里不是立即释放旧的 buckets，而是将旧的 buckets 添加到存放旧散列表的列表中，以便稍后释放，注意这里是稍后释放。
        cache_collect_free(oldBuckets, oldCapacity);
    }
}
```
#### allocateBuckets
```c++
#if CACHE_END_MARKER

bucket_t *allocateBuckets(mask_t newCapacity)
{
    // Allocate one extra bucket to mark the end of the list.
    // 分配一个额外的 bucket 以标记列表的末尾。
    
    // This can't overflow mask_t because newCapacity is a power of 2.
    // 因为 newCapacity 是 2 的幂，所以它不会溢出 mask_t。
    
    // 申请 sizeof(bucket_t) * newCapacity 个长度为 1 的连续内存空间，且内存初始化为 0
    bucket_t *newBuckets = (bucket_t *)
        calloc(cache_t::bytesForCapacity(newCapacity), 1);

    // end 标记的 bucket_t
    bucket_t *end = cache_t::endMarker(newBuckets, newCapacity);

#if __arm__
    // arm 32位架构下
    // End marker's sel is 1 and imp points BEFORE the first bucket.
    // 结束标记的 sel 为1，imp 指向第一个 bucket 之前。
    
    // This saves an instruction in objc_msgSend.
    // 这会将指令保存在 objc_msgSend 中。
    
    // bucket_t 的 set 函数，设置 _sel 和 _imp，_imp 设置为了 (newBuckets - 1)
    // _sel 设置为 1
    end->set<NotAtomic, Raw>((SEL)(uintptr_t)1, (IMP)(newBuckets - 1), nil);
#else
    // 其他
    // End marker's sel is 1 and imp points to the first bucket.
    // 结束标记的 sel 为1，imp 指向第一个存储桶。
    end->set<NotAtomic, Raw>((SEL)(uintptr_t)1, (IMP)newBuckets, nil);
#endif
    
    // 缓存容量统计
    if (PrintCaches) recordNewCache(newCapacity);

    return newBuckets;
}

#else

bucket_t *allocateBuckets(mask_t newCapacity)
{
    // 缓存容量统计
    if (PrintCaches) recordNewCache(newCapacity);
    
    // 申请 sizeof(bucket_t) * newCapacity 个长度为 1 的连续内存空间，且内存初始化为 0 
    return (bucket_t *)calloc(cache_t::bytesForCapacity(newCapacity), 1);
}

#endif
```
#### cache_collect_free
&emsp;`cache_collect_free` 函数声明在 `objc-cache.mm` 文件顶部，定义在 `objc-cache.mm` 的 `Line 977`。
```c++
/*
cache_collect_free.

Add the specified malloc'd memory to the list of them to free at some later point.
将指定的已分配内存（待释放的方法列表）添加到它们的列表中，以便稍后释放。

size is used for the collection threshold. It does not have to be precisely the block's size.
size 用于收集阈值。它不必精确地是 块 的大小。

Cache locks: cacheUpdateLock must be held by the caller.
cacheUpdateLock 必须由调用方持有。需要加锁。（__objc2__ 下使用的是 runtimeLock）

*/
static void cache_collect_free(bucket_t *data, mask_t capacity)
{
#if CONFIG_USE_CACHE_LOCK
    cacheUpdateLock.assertLocked();
#else
    runtimeLock.assertLocked(); // 加锁，加锁失败执行断言
#endif

    // 记录等待释放的容量 
    if (PrintCaches) recordDeadCache(capacity);

    // 为 garbage 准备空间，需要时进行扩容
    _garbage_make_room ();
    
    // 增加 garbage_byte_size 的值
    garbage_byte_size += cache_t::bytesForCapacity(capacity);
    
    // 把旧的 buckets 放进 garbage_refs 中，garbage_count 并自增 1
    garbage_refs[garbage_count++] = data;
    
    // 尝试去释放累积的旧缓存（bucket_t）
    cache_collect(false);
}
```
#### _garbage_make_room
&emsp;同样 `_garbage_make_room` 函数声明在 `objc-cache.mm` 顶部，定义在 `objc-cache.mm` 的 `Line 947`。
```c++
/*
_garbage_make_room.  
Ensure that there is enough room for at least one more ref in the garbage.
确保 garbage 中有足够的空间容纳至少一个引用。
*/

// amount of memory represented by all refs in the garbage.
// garbage 中所有引用所表示的内存量。
static size_t garbage_byte_size = 0;

// do not empty the garbage until garbage_byte_size gets at least this big.
// 在 garbage 中字节大小（garbage_byte_size）至少达到这么大（garbage_threshold）之前，不要清空 garbage。
static size_t garbage_threshold = 32*1024;

// table of refs to free.
// bucket_t **
static bucket_t **garbage_refs = 0;

// current number of refs in garbage_refs.
// 当前 garbage_refs 中 bucket_t * 的数量。
static size_t garbage_count = 0;

// capacity of current garbage_refs.
// 当前 garbage_refs 的容量。
static size_t garbage_max = 0;

// capacity of initial garbage_refs
// 初始 garbage_refs 的容量。
enum {
    INIT_GARBAGE_COUNT = 128 // 8 个 bucket_t
};

static void _garbage_make_room(void)
{
    static int first = 1; // 静态局部变量，下次进来 first 依然是上次的值

    // Create the collection table the first time it is needed
    // 第一次需要时创建收集表
    if (first)
    {
        first = 0; // 此处置为 0 后，以后调用 _garbage_make_room 再也不会进到这个 if
        // 申请初始空间
        // 申请 INIT_GARBAGE_COUNT * sizeof(void *) 字节个空间。
        // (malloc 不会对空间进行初始化，会保持申请时的垃圾数据)
        garbage_refs = (bucket_t**)malloc(INIT_GARBAGE_COUNT * sizeof(void *));
        
        // 当前 garbage_refs 的容量是 INIT_GARBAGE_COUNT
        garbage_max = INIT_GARBAGE_COUNT;
    }

    // Double the table if it is full
    // 如果当前 garbage_refs 中 refs 的数量等于 garbage_max 就对 garbage_refs 扩容为当前的 2 倍
    else if (garbage_count == garbage_max)
    {
        // garbage_refs 扩容为 2 倍
        garbage_refs = (bucket_t**)
            realloc(garbage_refs, garbage_max * 2 * sizeof(void *));
        // 更新 garbage_max 为 2 倍
        garbage_max *= 2;
    }
}
```
#### cache_collect
```c++
/*
cache_collect.  
Try to free accumulated dead caches.
尝试释放累积的死缓存。

collectALot tries harder to free memory.
collectALot 如果为 true 则即使 garbage_byte_size 未达到阀值也会去释放内存（旧的 bucket_t）。

Cache locks: cacheUpdateLock must be held by the caller.
cacheUpdateLock 必须由调用方持有，需要加锁。（__objc2__ 下使用的是 runtimeLock）
*/
void cache_collect(bool collectALot)
{
#if CONFIG_USE_CACHE_LOCK
    cacheUpdateLock.assertLocked();
#else
    runtimeLock.assertLocked(); // 加锁，加锁失败会执行断言
#endif

    // Done if the garbage is not full
    // 如果 garbage 未满，则返回
    // 32 * 1024
    // 未达到释放阀值，且 collectALot 为 false
    if (garbage_byte_size < garbage_threshold && !collectALot) {
        return;
    }

    // Synchronize collection with objc_msgSend and other cache readers.
    // objc_msgSend 和其他 缓存读取器 同步收集。
    if (!collectALot) {
    
        if (_collecting_in_critical ()) {
            // objc_msgSend (or other cache reader) is currently looking in the cache and might still be using some garbage.
            // objc_msgSend（或其他缓存读取器）当前正在缓存中查找，并且可能仍在使用某些 garbage。
            // 打印
            if (PrintCaches) {
                _objc_inform ("CACHES: not collecting; "
                              "objc_msgSend in progress");
            }
            // 直接 return
            return;
        }
    } 
    else {
        // No excuses.
        // 一直循环直到 _collecting_in_critical 为 false.
        while (_collecting_in_critical());
    }

    // No cache readers in progress - garbage is now deletable.
    // 没有正在进行中的 缓存读取器 现在可以删除 garbage 了。

    // Log our progress
    // Log
    if (PrintCaches) {
        cache_collections++; // 自增
        // 打印 garbage_byte_size garbage 使用的字节 cache_allocations 分配了多少 cache_t
        _objc_inform ("CACHES: COLLECTING %zu bytes (%zu allocations, %zu collections)", garbage_byte_size, cache_allocations, cache_collections);
    }
    
    // Dispose all refs now in the garbage.
    // 处理 garbage 中的所有 refs。
    
    // Erase each entry so debugging tools don't see stale pointers.
    // 擦除每个条目，以便调试工具不会看到过时的指针。
    
    // 循环释放 garbage_refs 中的 bucket_t *
    while (garbage_count--) {
        auto dead = garbage_refs[garbage_count];
        garbage_refs[garbage_count] = nil;
        free(dead);
    }
    
    // Clear the garbage count and total size indicator.
    // garbage_count 和 garbage_byte_size 置 0。
    
    garbage_count = 0;
    garbage_byte_size = 0;

    // 打印 Cache statistics 中的内容
    if (PrintCaches) {
        size_t i;
        size_t total_count = 0;
        size_t total_size = 0;

        for (i = 0; i < countof(cache_counts); i++) {
            int count = cache_counts[i];
            int slots = 1 << i;
            size_t size = count * slots * sizeof(bucket_t);

            if (!count) continue;

            _objc_inform("CACHES: %4d slots: %4d caches, %6zu bytes", 
                         slots, count, size);

            total_count += count;
            total_size += size;
        }

        _objc_inform("CACHES:      total: %4zu caches, %6zu bytes", 
                     total_count, total_size);
    }
}
```
#### _collecting_in_critical
&emsp;同样 `_collecting_in_critical` 函数声明在 `objc-cache.mm` 顶部，定义在 `objc-cache.mm` 的 `Line 838`。
返回 `true` 表示 `objc_msgSend`（或其他缓存读取器（`cache reader`））当前正在缓存中查找，并且可能仍在使用某些 `garbage`。返回 `false` 的话表示 `garbage` 中的 `bucket_t` 没有被在使用。

```c++
static int _collecting_in_critical(void)
{
#if TARGET_OS_WIN32 // 如果是 TARGET_OS_WIN32 则一直返回 true
    return TRUE;
#elif HAVE_TASK_RESTARTABLE_RANGES (arm64 和 x86_64 都支持)
    // Only use restartable ranges if we registered them earlier.
    // 如果我们较早注册它们，请仅使用 restartable ranges。
    // #if HAVE_TASK_RESTARTABLE_RANGES
    // static bool shouldUseRestartableRanges = true;
    // #endif
    
    if (shouldUseRestartableRanges) {
        // typedef int kern_return_t;
        kern_return_t kr = task_restartable_ranges_synchronize(mach_task_self());
        
        if (kr == KERN_SUCCESS) return FALSE; // return FALSE 表示 garbage 没有被在使用，此时处于可清空状态。
        （如果 collectALot 为真则表示必须清空，如果是不可清空状态则会一直等待，上面的 while (_collecting_in_critical()) ; ）
        
        _objc_fatal("task_restartable_ranges_synchronize failed (result 0x%x: %s)",
                    kr, mach_error_string(kr));
    }
#endif // !HAVE_TASK_RESTARTABLE_RANGES

    // Fallthrough if we didn't use restartable ranges.
    // 如果我们不使用 restartable ranges，则会失败。

    thread_act_port_array_t threads;
    unsigned number;
    unsigned count;
    kern_return_t ret;
    int result;

    // objc_thread_self: (pthread_t)tls_get_direct(_PTHREAD_TSD_SLOT_PTHREAD_SELF);
    // 从线程的存储空间读取
    
    mach_port_t mythread = pthread_mach_thread_np(objc_thread_self());

    // Get a list of all the threads in the current task.
    // 获取当前任务中所有线程的列表。
#if !DEBUG_TASK_THREADS
    ret = task_threads(mach_task_self(), &threads, &number);
#else
    ret = objc_task_threads(mach_task_self(), &threads, &number);
#endif

    if (ret != KERN_SUCCESS) {
        // See DEBUG_TASK_THREADS below to help debug this.
        // 请参阅下面的 DEBUG_TASK_THREADS 来帮助调试。
        
        _objc_fatal("task_threads failed (result 0x%x)\n", ret);
    }

    // Check whether any thread is in the cache lookup code.
    // 检查缓存查找代码中是否有线程。
    
    result = FALSE;
    for (count = 0; count < number; count++)
    {
        int region;
        uintptr_t pc;

        // Don't bother checking ourselves.
        // 不要打扰自己
        if (threads[count] == mythread)
            continue;

        // Find out where thread is executing.
        // 找出线程在哪里执行。
        pc = _get_pc_for_thread (threads[count]);

        // Check for bad status, and if so, assume the worse (can't collect).
        // 检查状态是否良好，如果是，则假设情况更糟（无法收集）。
        if (pc == PC_SENTINEL)
        {
            result = TRUE;
            goto done;
        }
        
        // Check whether it is in the cache lookup code.
        // 检查它是否在缓存查找代码中。
        for (region = 0; objc_restartableRanges[region].location != 0; region++)
        {
            uint64_t loc = objc_restartableRanges[region].location;
            if ((pc > loc) &&
                (pc - loc < (uint64_t)objc_restartableRanges[region].length))
            {
                result = TRUE;
                goto done;
            }
        }
    }

 done:
    // Deallocate the port rights for the threads
    // 取消分配线程的端口权限
    for (count = 0; count < number; count++) {
        mach_port_deallocate(mach_task_self (), threads[count]);
    }

    // Deallocate the thread list
    // 取消分配线程列表
    vm_deallocate (mach_task_self (), (vm_address_t) threads, sizeof(threads[0]) * number);

    // Return our finding
    // 返回我们的发现
    return result;
}
```
##### HAVE_TASK_RESTARTABLE_RANGES
```c++
// Define HAVE_TASK_RESTARTABLE_RANGES to enable usage of task_restartable_ranges_synchronize().
// 定义 HAVE_TASK_RESTARTABLE_RANGES 以启用使用 task_restartable_ranges_synchronize() 函数。

#if TARGET_OS_SIMULATOR || defined(__i386__) || defined(__arm__) || !TARGET_OS_MAC
#   define HAVE_TASK_RESTARTABLE_RANGES 0
#else
#   define HAVE_TASK_RESTARTABLE_RANGES 1
#endif
```
##### task_restartable_ranges_synchronize
```c++
/*!
 * @function task_restartable_ranges_synchronize
 *
 * @brief
 * Require for all threads in the task to reset their PC if within a restartable range.
 * 如果在可重新启动的范围内，则要求任务中的所有线程重置其 PC。
 *
 * @param task
 * The task to operate on (needs to be current task)
 * 要执行的任务（需要是当前任务）
 * @returns
 * - KERN_SUCCESS
 * - KERN_FAILURE if the task isn't the current one 如果任务不是当前任务
 */
extern kern_return_t task_restartable_ranges_synchronize(task_t task);
```
&emsp;`cache_t` 的内容先分析到这里，下篇我们接着来看 `cache_t` 剩余的 `insert` 和 `bad_cache`，以及最重要的 `objc-cache.h` 文件中声明的一系列方法，正是它们完整构成了方法缓存的实现。 

## 参考链接
**参考链接:🔗**
+ [方法查找流程 objc_msg_arm64.s](https://www.jianshu.com/p/a8668b81c5d6)
+ [OC 底层探索 09、objc_msgSend 流程 1-缓存查找](https://www.cnblogs.com/zhangzhang-y/p/13704597.html)
+ [汇编指令解读](https://blog.csdn.net/peeno/article/details/53068412)
