#  iOS底层Class结构之Cache_t
在之前的文章中探索过类的结构的相关信息，其中主要说了 superclss、isa 等，但是还有一个 cache_t，这个也是 Class 中极其重要的一部分。

在 objc4-779 之后的版本，cache_t 结构发生了变化，但是原理是一样的。
> （看到很多博主的的都是对旧版的解析，只有这个博主是用的最新的，所以一直在看博主的文章，本文中用的代码都是 objc4-781 中的，是目前苹果最新的代码。）

cache_t  是用来缓存 class 调用过的实例方法。但是其本身的工作机制和我们想象的是不一样的。
> 方法缓存原理：
> 1. 缓存第一个实例方法的时候，初始化一个 mask 容量为 3 的表，往里面存方法信息并记录缓存数量为 1.
> 2. 当有方法需要缓存的时候，如果需要缓存的方法数量 （当前缓存数量 + 1）<= 当前容量 + 1 的 3/4, 直接缓存并记录缓存数量 + 1
> 3. 当有方法需要缓存的时候，如果需要缓存的方法数量（当前缓存数量 + 1）> 当前容量的 3/4, 按照之前的容量 x ( (x + 1) * 2 - 1) 重新分配存储容量，并清除之前缓存的方法信息，然后再存储，并记录缓存数量为 1.（扩容以后并不会把以前的数据复制到新内存中，而是直接舍弃）

> mask 的机制
> 1.  初始化为 4 - 1
> 2. 当需要存储数量 > 当前容量 + 1 的 3/4，重新分配容量 （（当前容量 + 1）* 2 - 1 ）

**看 cache_t 源码结构之前延展学习 C++ 原子操作 std::atomic<T> :**
## std::atomic<T>
std::atomic<T> 模版类可以使对象操作为原子操作，避免多线程竞争问题。
std::atomic<T> 是模版类，一个模版类型为 T 的原子对象中封装了一个类型为 T 的值。
`template <class T> struct atomic;`
原子类型对象的主要特点就是从不同线程访问不会导致数据竞争（data race）。因此从不同线程访问某个原子对象是良性（well-defined）行为，而通常对于非原子类型而言，并发访问某个对象（如果不做任何同步操作）会导致未定义（undefined）行为发生。


## objc4-781 版本
**cache_t 结构**
```
struct cache_t {
#if CACHE_MASK_STORAGE == CACHE_MASK_STORAGE_OUTLINED
    /// 方法缓存数组（以散列表的形式进行存储）
    explicit_atomic<struct bucket_t *> _buckets; // 存储方法信息 8 
    /// 容量的临界值 （散列表长度 - 1）
    explicit_atomic<mask_t> _mask; // 留了多少个缓存位置 4
#elif CACHE_MASK_STORAGE == CACHE_MASK_STORAGE_HIGH_16
    explicit_atomic<uintptr_t> _maskAndBuckets;
    mask_t _mask_unused;
    
    // How much the mask is shifted by.
    static constexpr uintptr_t maskShift = 48;
    
    // Additional bits after the mask which must be zero. msgSend
    // takes advantage of these additional bits to construct the value
    // `mask << 4` from `_maskAndBuckets` in a single instruction.
    static constexpr uintptr_t maskZeroBits = 4;
    
    // The largest mask value we can store.
    static constexpr uintptr_t maxMask = ((uintptr_t)1 << (64 - maskShift)) - 1;
    
    // The mask applied to `_maskAndBuckets` to retrieve the buckets pointer.
    static constexpr uintptr_t bucketsMask = ((uintptr_t)1 << (maskShift - maskZeroBits)) - 1;
    
    // Ensure we have enough bits for the buckets pointer.
    static_assert(bucketsMask >= MACH_VM_MAX_ADDRESS, "Bucket field doesn't have enough bits for arbitrary pointers.");
#elif CACHE_MASK_STORAGE == CACHE_MASK_STORAGE_LOW_4
    // _maskAndBuckets stores the mask shift in the low 4 bits, and
    // the buckets pointer in the remainder of the value. The mask
    // shift is the value where (0xffff >> shift) produces the correct
    // mask. This is equal to 16 - log2(cache_size).
    explicit_atomic<uintptr_t> _maskAndBuckets;
    mask_t _mask_unused;

    static constexpr uintptr_t maskBits = 4;
    static constexpr uintptr_t maskMask = (1 << maskBits) - 1;
    static constexpr uintptr_t bucketsMask = ~maskMask;
#else
#error Unknown cache mask storage type.
#endif
    
#if __LP64__
    uint16_t _flags; // 2
#endif
    /// 表示已经缓存的方法的数量
    uint16_t _occupied; // 已缓存的数量 2

public:
    static bucket_t *emptyBuckets();
    
    struct bucket_t *buckets();
    mask_t mask();
    mask_t occupied();
    void incrementOccupied();
    void setBucketsAndMask(struct bucket_t *newBuckets, mask_t newMask);
    void initializeToEmpty();

    unsigned capacity();
    bool isConstantEmptyCache();
    bool canBeFreed();

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

#if FAST_CACHE_ALLOC_MASK
    bool hasFastInstanceSize(size_t extra) const
    {
        if (__builtin_constant_p(extra) && extra == 0) {
            return _flags & FAST_CACHE_ALLOC_MASK16;
        }
        return _flags & FAST_CACHE_ALLOC_MASK;
    }

    size_t fastInstanceSize(size_t extra) const
    {
        ASSERT(hasFastInstanceSize(extra));

        if (__builtin_constant_p(extra) && extra == 0) {
            return _flags & FAST_CACHE_ALLOC_MASK16;
        } else {
            size_t size = _flags & FAST_CACHE_ALLOC_MASK;
            // remove the FAST_CACHE_ALLOC_DELTA16 that was added
            // by setFastInstanceSize
            return align16(size + extra - FAST_CACHE_ALLOC_DELTA16);
        }
    }

    void setFastInstanceSize(size_t newSize)
    {
        // Set during realization or construction only. No locking needed.
        uint16_t newBits = _flags & ~FAST_CACHE_ALLOC_MASK;
        uint16_t sizeBits;

        // Adding FAST_CACHE_ALLOC_DELTA16 allows for FAST_CACHE_ALLOC_MASK16
        // to yield the proper 16byte aligned allocation size with a single mask
        sizeBits = word_align(newSize) + FAST_CACHE_ALLOC_DELTA16;
        sizeBits &= FAST_CACHE_ALLOC_MASK;
        if (newSize <= sizeBits) {
            newBits |= sizeBits;
        }
        _flags = newBits;
    }
#else
    bool hasFastInstanceSize(size_t extra) const {
        return false;
    }
    size_t fastInstanceSize(size_t extra) const {
        abort();
    }
    void setFastInstanceSize(size_t extra) {
        // nothing
    }
#endif

    static size_t bytesForCapacity(uint32_t cap);
    static struct bucket_t endMarker(struct bucket_t *b, uint32_t cap);

    void reallocate(mask_t oldCapacity, mask_t newCapacity, bool freeOld);
    void insert(Class cls, SEL sel, IMP imp, id receiver);

    static void bad_cache(id receiver, SEL sel, Class isa) __attribute__((noreturn, cold));
};
```
```
struct bucket_t {
private:
    // IMP-first is better for arm64e ptrauth and no worse for arm64.
    // SEL-first is better for armv7and i386 and x86_64.
#if __arm64__
    explicit_atomic<uintptr_t> _imp; // 获取方法实现
    explicit_atomic<SEL> _sel; // 以方法名为 key，选择子
#else
    explicit_atomic<SEL> _sel;
    explicit_atomic<uintptr_t> _imp;
#endif
....
};
```
定义 LGPerson 类，并实现几个方法并调用：
```
// .h
@interface LGPerson : NSObject

- (void)instanceMethod1;
- (void)instanceMethod2;
- (void)instanceMethod3;
- (void)instanceMethod4;
- (void)instanceMethod5;
- (void)instanceMethod6;
- (void)instanceMethod7;

@end

// main.m 中
LGPerson *person = [LGPerson alloc];
LGPerson *p = [person init]; // 在此处打断点

[p instanceMethod1];
[p instanceMethod2];
[p instanceMethod3];
[p instanceMethod4];
[p instanceMethod5];
[p instanceMethod6];
[p instanceMethod7];
```
通过控制台打印:
```
// 打印类信息
(lldb) p [person class]
(Class) $0 = LGPerson

// 拿到 cache 信息
(lldb) x/4gx $0
0x1000021d8: 0x00000001000021b0 （isa） 0x00000001003ee140 (superclass)
0x1000021e8: 0x0000000100677860 0x0002801000000003 (cache_t)
(lldb) p (cache_t *)0x1000021e8
(cache_t *) $1 = 0x00000001000021e8

// 查看 cache 具体有什么东西
(lldb) p *$1
(cache_t) $2 = {
  _buckets = {
    std::__1::atomic<bucket_t *> = 0x0000000100677860 {
      _sel = {
        std::__1::atomic<objc_selector *> = 0x00007fff70893e54
      }
      _imp = {
        std::__1::atomic<unsigned long> = 4041432
      }
    }
  }
  _mask = {
    std::__1::atomic<unsigned int> = 3
  }
  _flags = 32784
  _occupied = 2
}
```
这里就奇怪了，明明没有调用任何实例方法，为什么已经缓存了两个了呢？
继续打印 _buckets 信息：
```
(lldb) p (bucket_t *)$1.buckets()
(bucket_t *) $3 = 0x0000000100677860
  Fix-it applied, fixed expression was: 
    (bucket_t *)$1->buckets()
(lldb) p (bucket_t *)$1->buckets()
(bucket_t *) $4 = 0x0000000100677860
(lldb) p $4[0]
(bucket_t) $5 = {
  _sel = {
    std::__1::atomic<objc_selector *> = 0x00007fff70893e54
  }
  _imp = {
    std::__1::atomic<unsigned long> = 4041432
  }
}
(lldb) p $4[1]
(bucket_t) $6 = {
  _sel = {
    std::__1::atomic<objc_selector *> = 0x0000000000000000
  }
  _imp = {
    std::__1::atomic<unsigned long> = 0
  }
}
(lldb) p $4[2]
(bucket_t) $7 = {
  _sel = {
    std::__1::atomic<objc_selector *> = 0x0000000000000000
  }
  _imp = {
    std::__1::atomic<unsigned long> = 0
  }
}
(lldb) 
```
> 只看到 `$4[0]` 发现缓存的方法，但是无法看出究竟是缓存的什么方法。

在初始化 LGPerson 之前，通过 `Class cls = NSClassFromString(@"LGPerson");` 方式得到 cls，打印 cls 里面的缓存信息，发现 `mask = 0, _occupied = 0`，然后 `LGPerson *person = [LGPerson alloc];` 初始化之后，再打印，发现 `mask = 3, _occupied = 2`，也就是说在 alloc 过程中，进行了缓存操作。

断点往下走，调用了一个实例方法 `[person init]`，调用 `init` 之后，再打印 `cache` 信息，看到 `init` 被缓存到了 `LGPerson` 中，这也证明了 `即使是父类的方法，也会缓存在本类中`。

```
// 任何函数未执行时，只调用 NSClassFromString(@"LGPerson")
Class cls = NSClassFromString(@"LGPerson");
// 打印:
(cache_t) $3 = {
  _buckets = {
    std::__1::atomic<bucket_t *> = 0x00000001003e8490 {
      _sel = {
        std::__1::atomic<objc_selector *> = 0x0000000000000000
      }
      _imp = {
        std::__1::atomic<unsigned long> = 0
      }
    }
  }
  _mask = {
    std::__1::atomic<unsigned int> = 0 // 临界容量 0
  }
  _flags = 16
  _occupied = 0 // 已占用 0 
}

// 执行到 [persont init] 处
// 命令列表
p [person class]
x/4gx $0
p (cache_t *)0x1000021f0
p *$1

...
_mask = {
  std::__1::atomic<unsigned int> = 3 // 临界容量是 3
}
_flags = 32784
_occupied = 2 // 已占用是 2
...
// 执行到完 init 后
...
_mask = {
  std::__1::atomic<unsigned int> = 3 // 临界容量是 3
}
_flags = 32784
_occupied = 3 // 已占用是 3
...

// 使用上面 cls 
// 打印 _mask = 3 _occupied = 3
```

| 调用过实例方法数量 |0|1|2|3|4|5|6|7|8|
|---|---|---|---|---|---|---|---|---|---|
|_capacity|4|4|8|8|8|8|8|8|16|
|_mask|3|3|7|7|7|7|7|7|15|
|_occupied|2|3|1|2|3|4|5|6|1|

### 从源码看缓存机制
从哪里入手呢？先看 mask 是怎么来的
在 cache_t  中有一个 mask() 的方法，那就找这个方法

**第一个重点: 分配时容量和实际可用的容量**
**首先要记得 mask 始终是 capacity - 1，capacity 代表的是给 bucket_t * 分配的容量，比如起始分配的是 4，但是 mask 等于 capacity - 1,mask 是 3，occupied 代表当前缓存占用量即当前缓存了几个函数，那它就是几。**

**第二个重点: 扩容机制**
1. 缓存第一个方法时，初始化 capacity 为 4， mask 为 3，往里面存方法信息并把 occupied 置为 1.
2. 第一次之外的缓存方法时，首先判断 occupied + 1 <= capcity / 4 * 3，则不需要扩容，直接能进行缓存，并把 occupied 加 1。
3. 另外的情况就需要扩容了，新 capacity 等于 旧 capactiy * 2，然后 mask 依然置为 新 capacity - 1，新方法存入，且occupied 置为 1。 

```
typedef uint32_t mask_t; // mask_t 是无符号 32 位 int，占 4 个字节

// memory_order::memory_order_relaxed
// Relaxed ordering: 在单个线程内，所有原子操作是顺序进行的。按照什么顺序？基本上就是代码顺序（sequenced-before）。这就是唯一的限制了！两个来自不同线程的原子操作是什么顺序？两个字：任意。

mask_t cache_t::mask() 
{
    return _mask.load(memory_order::memory_order_relaxed);
}

```
```
// objc-cache.mm 文件开头注释
 Method cache locking (GrP 2001-1-14)

For speed, objc_msgSend does not acquire any locks when it reads method caches.
为了提高速度，objc_msgSend在读取方法缓存时不获取任何锁。

Instead, all cache changes are performed so that any objc_msgSend running concurrently with the cache mutator will not crash or hang or get an incorrect result from the cache.
相反，将执行所有高速缓存更改，以便与高速缓存更改器同时运行的任何objc_msgSend都不会崩溃或挂起，或从高速缓存中获取错误的结果。

When cache memory becomes unused (e.g. the old cache after cache expansion), it is not immediately freed, because a concurrent objc_msgSend could still be using it.
当缓存内存未使用时（例如，缓存扩展后的旧缓存），它不会立即被释放，因为并发objc_msgSend可能仍在使用它。

Instead, the memory is disconnected from the data structures and placed on a garbage list.
而是将内存与数据结构断开连接，并将其放置在垃圾清单上。

The memory is now only accessible to instances of objc_msgSend that were running when the memory was disconnected; any further calls to objc_msgSend will not see the garbage memory because the other data structures don't point to it anymore.
现在，只有断开内存时正在运行的objc_msgSend实例才能访问该内存；进一步调用objc_msgSend将看不到垃圾内存，因为其他数据结构不再指向该内存。

The collecting_in_critical function checks the PC of all threads and returns FALSE when all threads are found to be outside objc_msgSend.
collect_in_critical 函数检查 PC 的所有线程，并在发现所有线程不在objc_msgSend之外时返回FALSE。

This means any call to objc_msgSend that could have had access to the garbage has finished or moved past the cache lookup stage, so it is safe to free the memory.
这意味着对objc_msgSend的所有本可以访问垃圾的调用都已完成或移到了高速缓存查找阶段，因此可以安全地释放内存。

All functions that modify cache data or structures must acquire the cacheUpdateLock to prevent interference from concurrent modifications.
所有修改缓存数据或结构的函数都必须获取 cacheUpdateLock，以防止并发修改造成干扰。

The function that frees cache garbage must acquire the cacheUpdateLock and use collecting_in_critical() to flush out cache readers.
释放缓存垃圾的函数必须获取 cacheUpdateLock 并使用 collection_in_critical（）刷新缓存读取器。

The cacheUpdateLock is also used to protect the custom allocator used for large method cache blocks.
cacheUpdateLock还用于保护用于大型方法缓存块的自定义分配器。

Cache readers (PC-checked by collecting_in_critical())
objc_msgSend*
cache_getImp

Cache writers (hold cacheUpdateLock while reading or writing; not PC-checked)
cache_fill         (acquires lock)
cache_expand       (only called from cache_fill) 
cache_create       (only called from cache_expand)
bcopy               (only called from instrumented cache_expand)
flush_caches        (acquires lock)
cache_flush        (only called from cache_fill and flush_caches)
cache_collect_free (only called from cache_expand and cache_flush)

UNPROTECTED cache readers (NOT thread-safe; used for debug info only)
cache_print

_class_printMethodCaches
_class_printDuplicateCacheEntries
_class_printMethodCacheStatistics

```
入口，`cache_fill`:
```
void cache_fill(Class cls, SEL sel, IMP imp, id receiver)
{
    runtimeLock.assertLocked();

#if !DEBUG_TASK_THREADS
    // Never cache before +initialize is done
    // 在完成初始化之前不要进行缓存
    if (cls->isInitialized()) {
        cache_t *cache = getCache(cls);
// #if CONFIG_USE_CACHE_LOCK
//        mutex_locker_t lock(cacheUpdateLock);
// #endif
        cache->insert(cls, sel, imp, receiver);
    }
// #else
//    _collecting_in_critical();
// #endif
}

// 取得类的缓存 struct cache_t *
cache_t *getCache(Class cls) 
{
    ASSERT(cls);
    return &cls->cache;
}

// 步骤 1 插入缓存信息
ALWAYS_INLINE
void cache_t::insert(Class cls, SEL sel, IMP imp, id receiver) // 插入缓存
{
#if CONFIG_USE_CACHE_LOCK
    cacheUpdateLock.assertLocked();
#else
    runtimeLock.assertLocked(); // 这里使用 runtimeLock
#endif

    ASSERT(sel != 0 && cls->isInitialized());

    // Use the cache as-is if it is less than 3/4 full
    // 如果缓存未满 3/4，则按原样使用它 （
    // 按照第一次缓存来走流程）
    // occupied() = 0 newOccupied = 1
    mask_t newOccupied = occupied() + 1; // occupied 由 0 加 1
    
    unsigned oldCapacity = capacity(), capacity = oldCapacity;
    if (slowpath(isConstantEmptyCache())) { // 如果缓存是空的，这里很可能为 假，然后用的 slowpath
        // 如果是第一次进来，则为 cache_t 开辟空间 
        // Cache is read-only. Replace it. // 因为缓存是只读，直接替换
        if (!capacity) capacity = INIT_CACHE_SIZE; // 如果 capacity 是 0，初始化为 4 1<<2，且只能是 2 的次方。
        // 根据当前数据重新分配内存(bucket_t *newBuckets = allocateBuckets(newCapacity);)
        // 且再往里面 (setBucketsAndMask(newBuckets, newCapacity - 1);) 把 _occupied 置为 0
        reallocate(oldCapacity, capacity, /* freeOld */false);
    }
    else if (fastpath(newOccupied + CACHE_END_MARKER <= capacity / 4 * 3)) {
        // Cache is less than 3/4 full. Use it as-is.
        // 如果占用 newOccupied(旧占用量加 1) 小于等于容量加 1 的四分之三，就不用做其他操作
    }
    else {
        // 现在需要扩容了
        // 如果新占用数大于容量 + 1 的 3/4，就需要重新分配
        // 重新赋值为 capacity 的 2 倍，（capacity 是 mask + 1），所以是 8、16、32 等
        capacity = capacity ? capacity * 2 : INIT_CACHE_SIZE;
        if (capacity > MAX_CACHE_SIZE) {
            capacity = MAX_CACHE_SIZE;
        }
        // 根据当前数据重新分配 bucket_t *，存储新的数据，并清除已有缓存
        reallocate(oldCapacity, capacity, true);
    }

    bucket_t *b = buckets();
    mask_t m = capacity - 1;
    mask_t begin = cache_hash(sel, m);
    mask_t i = begin;

    // Scan for the first unused slot and insert there.
    // There is guaranteed to be an empty slot because the
    // minimum size is 4 and we resized at 3/4 full.
    do {
        if (fastpath(b[i].sel() == 0)) { // 如果没有找到需要缓存的方法
            incrementOccupied(); // _occupied ++
            b[i].set<Atomic, Encoded>(sel, imp, cls); // 混存方法
            return;
        }
        if (b[i].sel() == sel) {
            // The entry was added to the cache by some other thread
            // before we grabbed the cacheUpdateLock.
            // 如果找到了需要缓存的方法，即表示已经存在缓存中了，则直接 return。
            return;
        }
    } while (fastpath((i = cache_next(i, m)) != begin)); // 继续循环

    cache_t::bad_cache(receiver, (SEL)sel, cls);
}
```
```
ALWAYS_INLINE
void cache_t::reallocate(mask_t oldCapacity, mask_t newCapacity, bool freeOld)
{
    bucket_t *oldBuckets = buckets();
    bucket_t *newBuckets = allocateBuckets(newCapacity);

    // Cache's old contents are not propagated. 
    // This is thought to save cache memory at the cost of extra cache fills.
    // fixme re-measure this

    ASSERT(newCapacity > 0);
    ASSERT((uintptr_t)(mask_t)(newCapacity-1) == newCapacity-1);
    // 这个方法是重新赋值 buckets、mask 和 occupied 
    // mask 的值是 capacity - 1，所以最后新值是 4 -1, 8 - 1, 16 - 1 等等
    
    setBucketsAndMask(newBuckets, newCapacity - 1);
    
    if (freeOld) { // 释放之前的 buckets
        cache_collect_free(oldBuckets, oldCapacity);
    }
}
```
```
void cache_t::setBucketsAndMask(struct bucket_t *newBuckets, mask_t newMask)
{
    // objc_msgSend uses mask and buckets with no locks.
    // It is safe for objc_msgSend to see new buckets but old mask.
    // (It will get a cache miss but not overrun the buckets' bounds).
    // It is unsafe for objc_msgSend to see old buckets and new mask.
    // Therefore we write new buckets, wait a lot, then write new mask.
    // objc_msgSend reads mask first, then buckets.

#ifdef __arm__
    // ensure other threads see buckets contents before buckets pointer
    mega_barrier();

    _buckets.store(newBuckets, memory_order::memory_order_relaxed);
    
    // ensure other threads see new buckets before new mask
    mega_barrier();
    
    _mask.store(newMask, memory_order::memory_order_relaxed);
    _occupied = 0;
#elif __x86_64__ || i386
    // ensure other threads see buckets contents before buckets pointer
    _buckets.store(newBuckets, memory_order::memory_order_release);
    
    // ensure other threads see new buckets before new mask
    _mask.store(newMask, memory_order::memory_order_release);
    _occupied = 0;
#else
#error Don't know how to do setBucketsAndMask on this architecture.
#endif
}
```

750 不想再看分析了！！！

二个版本的区别：
781 是上来就判断需要不需要扩容如果需要就直接扩容，然后才进行缓存判断。
750 则是先判断当前有没有缓存，如果有的话直接返回了，没有的话再按需扩容。


**参考链接:🔗**
[C++11 并发指南六( <atomic> 类型详解二 std::atomic )](https://www.cnblogs.com/haippy/p/3301408.html)
[如何理解 C++11 的六种 memory order？](https://www.zhihu.com/question/24301047)





