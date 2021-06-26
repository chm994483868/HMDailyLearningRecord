# iOS 从源码解析Runtime (五)：聚焦objc_object(retain、release、retaincount)

> 经过上面两篇 `DenseMap` 和 `DenseMapBase` 的分析，相信对存储 `objc_object` 的引用计数所用的数据结构已经极其清楚了，那么接下来继续分析 `objc_object` 剩下的函数吧。

## Optimized calls to retain/release methods

### id retain()
```c++
// Equivalent to calling [this retain], with shortcuts if there is no override
// 等效于调用 [this retain]，如果没有重载此函数，则能快捷执行（快速执行）
inline id 
objc_object::retain()
{
    // Tagged Pointer 不参与引用计数管理，它的内存在栈区，由系统自行管理
    ASSERT(!isTaggedPointer());

    // 如果没有重载 retain/release 函数，则调用根类的 rootRetain() 函数
    //（hasCustomRR() 函数定义在 objc_class 中，等下面分析 objc_class 时再对其进行详细分析）
    if (fastpath(!ISA()->hasCustomRR())) {
        return rootRetain();
    }

    // 如果重载了 retain 函数，则以 objc_msgSend 调用重载的 retain 函数
    return ((id(*)(objc_object *, SEL))objc_msgSend)(this, @selector(retain));
}
```
### void release()
```c++
// Equivalent to calling [this release], with shortcuts if there is no override
// 等效于调用 [this release]，如果没有重载此函数，则能快捷执行（快速执行）
inline void
objc_object::release()
{
    // Tagged Pointer 不参与引用计数管理，它的内存在栈区，由系统自行管理
    ASSERT(!isTaggedPointer());
    
    // 如果没有重载 retain/release 函数，则调用根类的 rootRelease() 函数
    //（hasCustomRR() 函数定义在 objc_class 中，等下面分析 objc_class 时再对其进行详细分析）
    if (fastpath(!ISA()->hasCustomRR())) {
        rootRelease();
        return;
    }

    // 如果重载了 release 函数，则 objc_msgSend 调用 release 函数
    ((void(*)(objc_object *, SEL))objc_msgSend)(this, @selector(release));
}
```
### id autorelease()
```c++
// Equivalent to [this autorelease], with shortcuts if there is no override
// 等效于调用[this autorelease]，如果没有重载此函数，则能快捷执行（快速执行）
inline id 
objc_object::autorelease()
{
    // Tagged Pointer 不参与引用计数管理，它的内存在栈区，由系统自行管理
    ASSERT(!isTaggedPointer());
    
    // 如果没有重载 retain/release 函数，则调用根类的 rootAutorelease() 函数
    //（hasCustomRR() 函数定义在 objc_class 中，等下面分析 objc_class 时再对其进行详细分析）
    if (fastpath(!ISA()->hasCustomRR())) {
        return rootAutorelease();
    }
    
    // 如果重载了 autorelease 函数，则 objc_msgSend 调用 autorelease 函数
    return ((id(*)(objc_object *, SEL))objc_msgSend)(this, @selector(autorelease));
}
```
## Implementations of retain/release methods

### id rootRetain()
> &emsp;Base retain implementation, ignoring overrides. This does not check isa.fast_rr; if there is an RR override then it was already called and it chose to call [super retain]. 
  tryRetain=true is the -_tryRetain path.
  handleOverflow=false is the frameless fast path.
  handleOverflow=true is the framed slow path including overflow to side table.
  The code is structured this way to prevent duplication.
> 
> &emsp;Base retain 函数实现，忽略重载。这不检查 isa.fast_rr; 如果存在 RR 重载，则它已经被调用，并选择调用 [super retain]。
  当 tryRetain=true 会执行 -_tryRetain 路径，会执行一个（return sidetable_tryRetain() ? (id)this : nil;）
  当 handleOverflow=false 时是 frameless 的快速路径。
  当 handleOverflow=true 时是一个 framed 的慢速路径会把溢出的引用计数转移到 SideTable 的 refcnts 中。
  当 SIDE_TABLE_DEALLOCATING 被标记时，会返回 nil，其它情况都返回 (id)this 
  以这种方式构造代码以防止重复。
```c++
ALWAYS_INLINE id 
objc_object::rootRetain()
{
    // tryRetain 和 handleOverflow 都传入的 false，不执行 -_tryRetain path. 
    // handleOverflow=false 处理 extra_rc++ 溢出的情况
    return rootRetain(false, false);
}
```
#### id rootRetain(bool tryRetain, bool handleOverflow)
&emsp;`tryRetain` 参数如其名，尝试持有，它涉及到的只有一个 `return sidetable_tryRetain() ? (id)this : nil;` 操作，只有当对象处于正在销毁状态时，才会返回  `false`。当对象的 `isa` 是原始指针时，对象的引用计数全部保存在 `SideTable` 的 `refcnts` 里面的。`sidetable_tryRetain` 函数只会在对象的 `isa` 是原始指针时才会被调用。

&emsp;`handleOverflow` 参数是处理 `newisa.extra_rc++ overflowed` 情况的，当溢出情况发生后，如果 `handleOverflow` 传入的是 `false` 时，则会调用 `return rootRetain_overflow(tryRetain)`，它只有一件事情，就是把 `handleOverflow` 传递为 `true` 再次调用 `rootRetain` 函数。所以无论如何，当引用计数溢出时都是必会进行处理的。

&emsp;当对象的 `isa` 是优化的 `isa` 且 `extra_rc` 溢出时，会把一部分引用计数转移到 `RefcountMap` 中，只是转移一部分，并不是 `extra_rc` 溢出以后，对象的引用计数全部交给 `RefcountMap` 管理了，每次溢出后会把 `extra_rc` 置为 `RC_HALF`，然后下次增加引用计数增加的还是 `extra_rc`，直到再次溢出再转移到 `RefcountMap` 中。大概是因为操作 `extra_rc` 的消耗要远低于操作 `RefcountMap`。

&emsp;循环结束的条件是: `!StoreExclusive(&isa.bits, oldisa.bits, newisa.bits)`，这句代码意思是，`&isa.bits` 和 `oldisa.bits` 进行原子比较字节逐位相等的话，则把 `newisa.bits` 复制(同 `std::memcpy` 函数)到 `&isa.bits` 中，并且返回 `true`。如果 `&isa.bits` 和 `oldisa.bits` 不相等的话，则把 `&isa.bits`  中的内容加载到 `oldisa.bits` 中。`StoreExclusive` 函数内部是封装 `__c11_atomic_compare_exchange_weak((_Atomic(uintptr_t) *)dst, &oldvalue, value, __ATOMIC_RELAXED, __ATOMIC_RELAXED)` 函数，可参考：[`atomic_compare_exchange_weak`](https://zh.cppreference.com/w/cpp/atomic/atomic_compare_exchange)。
所以这里的循环的真实目的就是为了把: `newisa.bits` 复制到 `&isa.bits` 中，保证对象 `bits` 中的数据能正确的进行修改。

```c++
// handleOverflow 参数看似是一个 bool 类型的表示是否处理上溢出，
// 其实当溢出发生了的话是一定会进行处理的，如果 handleOverflow 为 false，
// 那么它会借一个 rootRetain_overflow 函数，并再次调用 rootRetain 函数，
// 并把 handleOverflow 参数传递 true。 

ALWAYS_INLINE id 
objc_object::rootRetain(bool tryRetain, bool handleOverflow)
{
    // 如果是 Tagged Pointer 则直接返回 this (Tagged Pointer 不参与引用计数管理，它的内存在栈区，由系统处理)
    if (isTaggedPointer()) return (id)this;
    
    // 临时变量，标记 SideTable 是否加锁
    bool sideTableLocked = false;
    // 临时变量，标记是否需要把引用计数迁移到 SideTable 中
    bool transcribeToSideTable = false;

    // 记录 objc_object 之前的 isa
    isa_t oldisa;
    // 记录 objc_object 修改后的 isa
    isa_t newisa;
    
    // 循环结束的条件是 slowpath(!StoreExclusive(&isa.bits, oldisa.bits, newisa.bits))
    // StoreExclusive 函数，如果 &isa.bits 与 oldisa.bits 的内存内容相同，则返回 true，并把 newisa.bits 复制到 &isa.bits，
    // 否则返回 false，并把 &isa.bits 的内容加载到 oldisa.bits 中。
    // 即 do-while 的循环条件是指，&isa.bits 与 oldisa.bits 内容不同，如果它们内容不同，则一直进行循环，
    // 循环的最终目的就是把 newisa.bits 复制到 &isa.bits 中。
    // return __c11_atomic_compare_exchange_weak((_Atomic(uintptr_t) *)dst,
    //                                          &oldvalue, value, __ATOMIC_RELAXED, __ATOMIC_RELAXED)
    
    // _Bool atomic_compare_exchange_weak( volatile A *obj, C* expected, C desired );
    // 定义于头文件 <stdatomic.h>
    // 原子地比较 obj 所指向对象的内存的内容与 expected 所指向的内存的内容，若它们相等，则以 desired 替换前者（进行读修改写操作）。
    // 否则，将 obj 所指向的实际内存内容加载到 *expected （进行加载操作）。
    
    do {
        // 默认不需要转移引用计数到 SideTable
        transcribeToSideTable = false;
        
        // x86_64 平台下
        // C atomic_load( const volatile A* obj );
        // 定义于头文件 <stdatomic.h>
        // 以原子方式加载并返回 obj 指向的原子变量的当前值。该操作是原子读取操作。
        // return  __c11_atomic_load((_Atomic(uintptr_t) *)src, __ATOMIC_RELAXED);
        
        // 以原子方式读取 &isa.bits。（&为取地址） 
        oldisa = LoadExclusive(&isa.bits);
        
        // 赋值给 newisa（第一次进来时 &isa.bits, oldisa.bits, newisa.bits 三者是完全相同的）
        newisa = oldisa;
        
        // 如果 newisa 不是优化的 isa (元类的 isa 是原始的 isa (Class cls))
        if (slowpath(!newisa.nonpointer)) {
            
            // 在 mac、arm64e 下不执行任何操作，只在 arm64 下执行 __builtin_arm_clrex();
            // 在 arm64 平台下，清除对 &isa.bits 的独占访问标记。
            ClearExclusive(&isa.bits);
            
            // 如果是元类则直接返回 this，元类对象都是全局唯一的，不参与引用计数管理
            if (rawISA()->isMetaClass()) return (id)this;
            
            // 如果不需要 tryRetain 并且当前 SideTable 处于加锁状态，则进行解锁
            if (!tryRetain && sideTableLocked) sidetable_unlock();
            
            // 此处两种情况都是针对的原始指针类型的 isa，此时的对象的引用计数都保存在 SideTable 中
            if (tryRetain) {
              // 如果需要 tryRetain 则调用 sidetable_tryRetain 函数，并根据结果返回 this 或者 nil。
              // 执行此行之前是不需要在当前函数对 SideTable 加锁的
              
              // sidetable_tryRetain 返回 false 表示对象已被标记为正在释放，
              // 所以此时再执行 retain 操作是没有意义的，所以返回 nil。
              return sidetable_tryRetain() ? (id)this : nil;
              
            } else { 
              // 如果不需要 tryRetain 则调用 sidetable_retain()
              return sidetable_retain();
            }
        }
        
        // don't check newisa.fast_rr; we already called any RR overrides
        // 不要检查 newisa.fast_rr; 我们已经调用所有 RR 的重载。
        
        // 如果 tryRetain 为真并且 objc_object 被标记为正在释放 (newisa.deallocating)，则返回 nil
        if (slowpath(tryRetain && newisa.deallocating)) {
            
            // 在 mac、arm64e 下不执行任何操作，只在 arm64 下执行 __builtin_arm_clrex();
            // 在 arm64 平台下，清除对 &isa.bits 的独占访问标记。
            ClearExclusive(&isa.bits);
            
            // 如果不需要 tryRetain 并且当前 SideTable 处于加锁状态，则进行解锁
            if (!tryRetain && sideTableLocked) sidetable_unlock();
            
            // 返回 nil
            return nil;
        }
        
        // 下面就是 isa 为 nonpointer，并且没有被标记为正在释放的对象
        uintptr_t carry;
        // bits extra_rc 自增
        
        // x86_64 平台下:
        // # define RC_ONE (1ULL<<56)
        // uintptr_t extra_rc : 8
        // extra_rc 内容位于 56~64 位
        
        newisa.bits = addc(newisa.bits, RC_ONE, 0, &carry);  // extra_rc++
        
        // 如果 carry 为 true，表示要处理引用计数溢出的情况
        if (slowpath(carry)) {
            // newisa.extra_rc++ overflowed
            
            // 如果 handleOverflow 为 false，
            // 则调用 rootRetain_overflow(tryRetain) 它的作用就是把 handleOverflow 传为 true
            // 再次调用 rootRetain 函数，目的就是 extra_rc 发生溢出时，我们一定要处理
            if (!handleOverflow) {
                
                // 在 mac、arm64e 下不执行任何操作，只在 arm64 下执行 __builtin_arm_clrex();
                // 在 arm64 平台下，清除对 &isa.bits 的独占访问标记。
                ClearExclusive(&isa.bits);
                
                return rootRetain_overflow(tryRetain);
            }
            
            // Leave half of the retain counts inline and prepare to
            // copy the other half to the side table.
            // 将 retain count 的一半留在 inline，并准备将另一半复制到 SideTable.
            
            // 此时表明需要操作 SideTable 了！
            
            // SideTable 加锁，接下来需要操作 refcnts 
            // 如果 tryRetain 为 false 并且 sideTableLocked 为 false，则 SideTable 加锁
            if (!tryRetain && !sideTableLocked) sidetable_lock();
            
            // 标记 SideTable 已经加锁
            // 整个函数只有这里把 sideTableLocked 置为 true
            sideTableLocked = true;
            // 标记需要把引用计数转移到 SideTable 中
            transcribeToSideTable = true;
            
            // x86_64 平台下：
            // uintptr_t extra_rc : 8
            // # define RC_HALF  (1ULL<<7) 二进制表示为: 0b 1000,0000
            // extra_rc 总共 8 位，现在把它置为 RC_HALF，表示 extra_rc 溢出
            newisa.extra_rc = RC_HALF;
            
            // 把 has_sidetable_rc 标记为 true，表示 extra_rc 已经存不下该对象的引用计数，
            // 需要扩张到 SideTable 中
            newisa.has_sidetable_rc = true;
        }
    } while (slowpath(!StoreExclusive(&isa.bits, oldisa.bits, newisa.bits)));

    if (slowpath(transcribeToSideTable)) {
        // Copy the other half of the retain counts to the side table.
        // 复制 retain count 的另一半到 SideTable 中。
        sidetable_addExtraRC_nolock(RC_HALF);
    }

    // 如果 tryRetain 为 false 并且 sideTableLocked 为 true，则 SideTable 解锁
    if (slowpath(!tryRetain && sideTableLocked)) sidetable_unlock();
    
    // 返回 this 
    return (id)this;
}
```
&emsp;这里增加引用计数的情况中，前两种比较普通。

1. 当对象的 `isa` 是非优化的 `isa` 时，对象的引用计数全部保存在 `SideTable` 中，当要增加引用计数时就调用 `sidetable_tryRetain/sidetable_retain` 增加 `SideTable` 中的引用计数。
2. 当对象的 `isa` 是优化的 `isa` 且对象的引用计数保存在 `extra_rc` 字段中且加 1 后未溢出时，此时也是比较清晰的，执行完加 1 后，函数也直接 `return (id)this` 结束了。
3. 只有第三种情况比较特殊，当对象的 `isa` 是优化的 `isa` 且对象的引用计数保存在 `extra_rc` 中，此时 `extra_rc++` 后发生溢出，此时会把 `extra_rc` 赋值为 `RC_HALF`，把 `has_sidetable_rc` 赋值为 `true`，然后调用 `sidetable_addExtraRC_nolock(RC_HALF)`。其实疑问就发生在这里，如果对象的 `extra_rc` 中的引用计数已经溢出过了，并转移到了 `SideTable` 中一部分，此时 `extra_rc` 是被置为了 `RC_HALF`，那下次增加对象的引用计数时，并不是直接去 `SideTable` 中增加引用计数，其实是增加 `extra_rc` 中的值，直到增加到再次溢出时才会跑到 `SideTable` 中增加引用计数。这里还挺迷惑的，觉的最好的解释应该是尽量在 `extra_rc` 字段中增加引用计数，少去操作 `SideTable`，毕竟操作 `SideTable` 还要加锁解锁，还要哈希查找等，整体消耗肯定是大于直接操作 `extra_rc` 字段的。

#### LoadExclusive、ClearExclusive、StoreExclusive、StoreReleaseExclusive
&emsp;这四个函数主要用来进行原子读写(修改)操作。在 `objc-os.h` 的定义可看到在不同平台下它们的实现是不同的。首先是 `__arm64__ && !__arm64e__`，它针对的平台是从 `iPhone 5s` 开始到 `A12` 之前，已知 `A12` 开始是属于 `__arm64e__` 架构。

+ `ldrex` 可从内存加载数据，如果物理地址有共享 `TLB` 属性，则 `ldrex` 会将该物理地址标记为由当前处理器独占访问，并且会清除该处理器对其他任何物理地址的任何独占访问标记。否则，会标记：执行处理器已经标记了一个物理地址，但访问尚未完毕。清除标记时使用 `clrex` 指令。

+ `strex` 可在一定条件下向内存存储数据。条件具体如下：
  1. 如果物理地址没有共享 `TLB `属性，且执行处理器有一个已标记但尚未访问完毕的物理地址，那么将会进行存储，清除该标记，并在 `Rd` 中返回值 0。
  2. 如果物理地址没有共享 `TLB` 属性，且执行处理器也没有已标记但尚未访问完毕的物理地址，那么将不会进行存储，而会在 `Rd` 中返回值 1。
  3. 如果物理地址有共享 `TLB` 属性，且已被标记为由执行处理器独占访问，那么将进行存储，清除该标记，并在 `Rd` 中返回值 0。
  4. 如果物理地址有共享 `TLB` 属性，但没有标记为由执行处理器独占访问，那么不会进行存储，且会在 `Rd` 中返回值 1。

+ `stlex` ...

+ `clrex` 该指令的作用就是在独占访问结束时，清除 `cpu` 中本地处理器针对某块内存区域的独占访问标志（核中的某个状态寄存器），以防在未清除时的其他操作，对系统产生影响。

由于对 `ARM` 相关内容属于完全未知，具体内容可参考: 
[Linux内核同步机制之（一）：原子操作](http://www.wowotech.net/linux_kenrel/atomic.html) 
[arm架构的独占读写指令ldrex和strex的使用详解（原子操作和自旋锁实现的基本原理）](https://blog.csdn.net/duanlove/article/details/8212123) 
[【解答】arm架构的linux内核中，clrex指令的作用是什么，内核中什么时候才会用到？](https://blog.csdn.net/qianlong4526888/article/details/8536922)


&emsp;在非 `arm64` 平台下，例如 `mac` 的 `x86_64` 架构下则都是基于 `C++11` 后推出的 `atomic` 操作来实现的。
+ `template< class T > T atomic_load( const std::atomic<T>* obj ) noexcept` 原子地获得 `obj` 所指向的值。
+ `template< class T > bool atomic_compare_exchange_weak( std::atomic<T>* obj, typename std::atomic<T>::value_type* expected, typename std::atomic<T>::value_type desired ) noexcept;` 原子地比较 `obj` 所指向对象与 `expected` 所指向对象的对象表示 (`C++20` 前)值表示 (`C++20` 起)，若它们逐位相等，则以 `desired` 替换前者（进行读修改写操作）。否则，将 `obj` 所指向对象的实际值加载到 `*expected` （进行加载操作），复制如同以 `std::memcpy` 进行，返回值的话，当 `obj` 与 `expected` 相等时返回 `true`，反之返回 `false`。

具体内容可参考:
[atomic_load](https://zh.cppreference.com/w/cpp/atomic/atomic_load)
[atomic_compare_exchange](https://zh.cppreference.com/w/cpp/atomic/atomic_compare_exchange)

#### SIDE_TABLE_WEAKLY_REFERENCED 等等标志位
&emsp;我们首先要清楚一件很重要的事情，当对象的 `isa` 是原始类指针时，在 `SideTable` 的 `RefcountMap refcnts` 中取出 `objc_object` 对应的 `size_t` 的值并不是单纯的对象的引用计数这一个数字，它是明确有一些标志位存在的，且有些标志位所代表的含义与 `isa` 是非指针的 `objc_object` 的 `isa_t isa` 中的一些位是相同的。所以这里我们不能形成定式思维，觉的这些标志位只存在于 `isa_t isa` 中。

&emsp;如下列举当对象的 `isa` 是原始指针时，一些标志位所代表的含义：

+ `SIDE_TABLE_WEAKLY_REFERENCED` 是 `size_t` 的第 0 位，表示该对象是否有弱引用。（此时是针对 `isa` 是原始指针的对象，对应于 `isa` 是非指针时，`x86_64` 下 `isa_t isa` 的 `uintptr_t weakly_referenced : 1;` 字段）   
+ `SIDE_TABLE_DEALLOCATING` 是 `size_t` 的第 1 位，表示对象是否正在进行释放。（同上，对应于 `x86_64` 下 `isa_t isa` 的 `uintptr_t deallocating : 1;` 字段）
+ `SIDE_TABLE_RC_ONE` 是 `size_t` 的第 2 位，这时才正式开始表示该对象的引用计数。(当对象的 `isa` 是非指针时，引用计数也是从第 2 位开始的，但是它的前两位不做任何标记位使用，只是单纯的被舍弃了。)
+ `SIDE_TABLE_RC_PINNED` 在 `__LP64__` `64` 位系统架构下是 `size_t` 的第 63 位，`32` 位系统架构下是第 31 位，也就是 `size_t` 的最后一位，表示在 `SideTable` 中的引用计数溢出。（大概是不会存在一个对象的引用计数大到连 `SideTable` 都存不下的吧）
+ `SIDE_TABLE_RC_SHIFT` 帮助我们从 `size_t` 中拿出真实引用计数用的，即从第 2 位开始，后面的数值都表示对象的引用计数了。
+ `SIDE_TABLE_FLAG_MASK` 是 `SIDE_TABLE_RC_ONE` 的值减 1，它的二进制表示是 `0b11` 后两位是 `1`，其它位都是 `0`，做掩码使用。

```c++
// The order of these bits is important.
#define SIDE_TABLE_WEAKLY_REFERENCED (1UL<<0) // 0b1
#define SIDE_TABLE_DEALLOCATING      (1UL<<1) // 0b10 // MSB-ward of weak bit
#define SIDE_TABLE_RC_ONE            (1UL<<2) // 0b100 // MSB-ward of deallocating bit
#define SIDE_TABLE_RC_PINNED         (1UL<<(WORD_BITS-1)) // 第 63/31 位是 1，其它位都是 0

#define SIDE_TABLE_RC_SHIFT 2 // 表示 SIDE_TABLE_RC_ONE 左移的距离
#define SIDE_TABLE_FLAG_MASK (SIDE_TABLE_RC_ONE-1) // 0b011 后两位是 1，其它位都是 0，做掩码使用
```
#### sidetable_tryRetain
&emsp;**此函数只能在 `objc_object` 使用非优化 `isa` 的情况下调用。**
它有个 `bool` 类型的返回值，当对象被标记为 `SIDE_TABLE_DEALLOCATING` （正在进行释放）时才会返回 `false`，其它情况下都是正常进行 `retain` 并返回 `true`。
```c++
bool
objc_object::sidetable_tryRetain()
{

// 如果当前平台支持 isa 优化
#if SUPPORT_NONPOINTER_ISA
    // 如果 isa 是优化的 isa 则直接执行断言，
    // sidetable_tryRetain 函数只能在对象的 isa 是原始指针时调用（Class cls）
    ASSERT(!isa.nonpointer);
#endif

    // 从全局的 SideTalbes 中找到 this 所处的 SideTable
    SideTable& table = SideTables()[this];

    // NO SPINLOCK HERE
    // 这里没有 SPINLOCK
    
    // _objc_rootTryRetain() is called exclusively by _objc_loadWeak(), 
    // which already acquired the lock on our behalf.
    // _objc_rootTryRetain() 仅由 _objc_loadWeak() 独占调用，已经代表我们获得了锁。

    // fixme can't do this efficiently with os_lock_handoff_s
    // fixme os_lock_handoff_s 无法有效地做到这一点
    // if (table.slock == 0) {
    //     _objc_fatal("Do not call -_tryRetain.");
    // }

    // 默认值为 true，如果该对象正在进行释放，会被置为 false
    bool result = true;
    
   // it 的类型是: std::pair<DenseMapIterator<std::pair<Disguised<objc_object>, size_t>>, bool>
   // try_emplace 处理两种情况：
   // 1. 如果 this 在 refcnts 中还不存在，则给 this 在 buckets 中找一个 BucketT，
   //    KeyT 放 this， ValueT 放 SIDE_TABLE_RC_ONE，然后使用这个 BucketT 构建一个 iterator，
   //    然后用这个 iterator 和 true 构造一个 std::pair<iterator, true> 返回。
   // 2. 如果 this 在 refcnts 中已经存在了，则用 this 对应的 BucketT 构建一个 iterator,
   //    然后用这个 iterator 和 false 构造一个 std::pair<iterator, false> 返回。
    auto it = table.refcnts.try_emplace(this, SIDE_TABLE_RC_ONE);
    
    // refcnt 是引用计数值的引用。
    // it.first 是 DenseMapIterator，它的操作符 -> 被重写了返回的是 DenseMpaIterator 的 Ptr 成员变量，
    // 然后 Ptr 的类型是 BucketT 指针, 然后这里的  ->second 其实就是 BucketT->second，其实就是 size_t，正是保存的对象的引用计数数据。
    auto &refcnt = it.first->second;
    
    if (it.second) {
        // 如果 it.second 为 true，表示 this 第一次放进 refcnts 中，
        // 且 BucketT.second 已经被置为 SIDE_TABLE_RC_ONE，其它也不需要任何操作了。
        // there was no entry
    } else if (refcnt & SIDE_TABLE_DEALLOCATING) { 
        // 表示对象正在进行释放，result 置为 false 就好了
        result = false;
    } else if (! (refcnt & SIDE_TABLE_RC_PINNED)) { 
        // refcnt & SIDE_TABLE_RC_PINNED 值为 false 的话表示，
        // rcfcnts 中 保存 this 的 BucketT 的 size_t 还没有溢出，还可正常进行自增操作保存 this 的引用计数，
        // refcnt 加 SIDE_TABLE_RC_ONE
        refcnt += SIDE_TABLE_RC_ONE;
    }

    return result;
}
```
#### sidetable_retain
&emsp;**此函数只能在 `objc_object` 使用非优化 `isa` 的情况下调用。**
```c++
id
objc_object::sidetable_retain()
{

// 如果当前平台支持 isa 优化
#if SUPPORT_NONPOINTER_ISA
    // 如果 isa 是优化的 isa 则直接执行断言，
    // sidetable_retain 函数只能在对象的 isa 是原始指针时调用（Class cls）
    ASSERT(!isa.nonpointer);
#endif

    // 从全局的 SideTalbes 中找到 this 所处的 SideTable
    SideTable& table = SideTables()[this];
    
    // SideTable 加锁
    table.lock();
    
    // 这里是调用 DenseMapBase 的 operator[]，找到 this 所对应的 BucketT 然后返回 Bucket.second 即 this 的引用计数数据
    
    // 这里有一个迷惑点， 如果在 refcnts 中未找到 this 对应的 BucketT 的话，会调用 InsertIntoBucket 函数为 this 构建一个 BucketT，
    // 只是这里没有传递 size_t 那 BucketT 的 size_t 的初始值是什么呢？
    // 当对象的 isa 是原始指针时，对象的引用计数全部都存放在 refcnts 中，那么在对象刚创建好时就会把对象放到 refcnts 中吗？
    // 还有对象是什么时候放进 SideTable 的？都还没有遇到，等一点一点的深入 objc4-781 应该都能看到的吧...
    size_t& refcntStorage = table.refcnts[this];
    
    if (! (refcntStorage & SIDE_TABLE_RC_PINNED)) {
        // refcnt & SIDE_TABLE_RC_PINNED 值为 false 的话表示，
        // rcfcnts 中 保存 this 的 BucketT 的 size_t 还没有溢出，还可正常进行自增操作保存 this 的引用计数
        // refcnt 加 SIDE_TABLE_RC_ONE
        refcntStorage += SIDE_TABLE_RC_ONE;
    }
    
    // SideTable 解锁
    table.unlock();

    return (id)this;
}
```
#### rootRetain_overflow
&emsp;`rootRetain_overflow` 函数内部是调用了 `rootRetain(tryRetain, true)`，`handleOverflow` 函数传递的是 `true`，即当溢出发生时递归调用 `rootRetain`，去执行 `sidetable_addExtraRC_nolock` 函数，去处理溢出，把引用计数转移到 `SideTable` 中去。

```c++
NEVER_INLINE id 
objc_object::rootRetain_overflow(bool tryRetain)
{
    return rootRetain(tryRetain, true);
}
```
#### sidetable_addExtraRC_nolock
&emsp;`refcnts` 中引用计数溢出则返回 `true`，正常情况下加 `1`，返回 `false`。
```c++
// Move some retain counts to the side table from the isa field.
// Returns true if the object is now pinned.
// 将一些引用计数从 isa 字段转移到 SideTable 中。
// 如果 SideTable.refcnts (size_t 类型) 溢出了则会返回 true。

bool 
objc_object::sidetable_addExtraRC_nolock(size_t delta_rc)
{
    // 如果 isa.nonpointer 为真，即 objc_object 的 isa 是优化的 isa，则正常往下执行
    ASSERT(isa.nonpointer);
    
    // 从全局的 SideTalbes 中找到 this 所处的 SideTable
    SideTable& table = SideTables()[this];

    // 从 refcnts 中取出对象的引用计数，如果此时对象还没有插入哈希桶的话，
    // 会直接为 this 在哈希桶中找一个位置插入，那么在函数 InsertIntoBucket 中不传递 Value
    // 的时候，对象的 this 的所在的 BucketT 的 second 的初始值是多少呢？（size_t 的初始值是 0 吗？） 
    size_t& refcntStorage = table.refcnts[this];
    size_t oldRefcnt = refcntStorage;
    
    // isa-side bits should not be set here
    // isa-side bits 不应在此处设置
    // 这里表明从 SideTable 中取出的 size_t 的后两位是
    // 不能包含 SIDE_TABLE_DEALLOCATING 和 SIDE_TABLE_WEAKLY_REFERENCED 的
    // 它们两位必须都是 0，不然会执行如下断言
    ASSERT((oldRefcnt & SIDE_TABLE_DEALLOCATING) == 0);
    ASSERT((oldRefcnt & SIDE_TABLE_WEAKLY_REFERENCED) == 0);

    // 如果已经被标记为溢出，则直接 return true;
    if (oldRefcnt & SIDE_TABLE_RC_PINNED) return true;

    // 加 1
    uintptr_t carry;
    size_t newRefcnt = addc(oldRefcnt, delta_rc << SIDE_TABLE_RC_SHIFT, 0, &carry);
    
    // 如果 carry 为 true，表示溢出了
    // oldRefcnt & SIDE_TABLE_FLAG_MASK => oldRefcnt & 0b011
    
    if (carry) {
        // refcntStorage 最高位为 1，然后最后两位保持原值，其它位都是 0
        
        // 这里不是说 SideTable 中取出的 refcnt 的后两位不都一定要是 0 吗，不然上面的断言会执行。
        // SIDE_TABLE_RC_PINNED 只有最高位是 1，然后 oldRefcnt 的最低两位是 0 与 0b011 做与操作
        // 最后两位还是 0
        // 所以 refcntStorage 的值就是 最高位是 1， 然后剩余位都是 oldRefcnt 的值
        // 且刚刚 oldRefcnt 加 1 后溢出了则，refcntStorage 的值就是除了最后两位是 0 以外，其他位都是 1
        refcntStorage = SIDE_TABLE_RC_PINNED | (oldRefcnt & SIDE_TABLE_FLAG_MASK);
        
        // 溢出了,此时要返回 true
        return true;
    } else {
        // refcnts 中的引用计数正常加 1，没有溢出，此时返回 false
        refcntStorage = newRefcnt;
        return false;
    }
}
```
&emsp;下面开始是 release 函数：

### bool rootRelease()
```c++
// Base release implementation, ignoring overrides.
// 基本的 release 函数实现，忽略重载。

// Does not call -dealloc.
// 不调用 -dealloc 函数。

// Returns true if the object should now be deallocated.
// 如果现在应该释放对象，那么返回 true。

// This does not check isa.fast_rr; if there is an RR override then 
// it was already called and it chose to call [super release].
// 这不检查 isa.fast_rr; 如果有 retain/release 函数被重载了，则它已经被调用了，
// 并且它选择调用 [super release]。
// 
// 对应不同的执行逻辑，和上面的 rootRetain 函数有类似之处。
// 
// handleUnderflow=false is the frameless fast path. // 不处理下溢出
// 处理下溢出，涉及 SideTable 引用计数的数据的借用处理
// handleUnderflow=true is the framed slow path including side table borrow

// The code is structured this way to prevent duplication.
// 以这种方式构造代码防止重复。

ALWAYS_INLINE bool 
objc_object::rootRelease()
{
    // 调用下面有两个参数的 rootRelease，
    // 两个参数分别代表是否执行销毁/是否处理溢出
    return rootRelease(true, false);
}
```
#### bool rootRelease(bool performDealloc, bool handleUnderflow)
```c++
// handleUnderflow 参数看似是一个 bool 类型的表示是否处理下溢出，
// 当溢出发生了的话是必须要处理的，如果 handleUnderflow 为 false，
// 那么它会借一个 rootRelease_underflow 函数，并再次调用 rootRelease 函数，
// 并把 handleUnderflow 参数传递 true。

ALWAYS_INLINE bool 
objc_object::rootRelease(bool performDealloc, bool handleUnderflow)
{   
    // 如果是 Tagged Pointer 直接返回 false，Tagged Pointer 不参与引用计数处理，它内存位于栈区，由系统处理
    if (isTaggedPointer()) return false;

    // 标记 SideTable 是否加锁了
    bool sideTableLocked = false;

    // 临时变量存放旧的 isa
    isa_t oldisa;
    // 临时变量存放字段修改后的 isa
    isa_t newisa;

 retry:
    do {
        // 以原子方式读到 &isa.bits 的数据
        oldisa = LoadExclusive(&isa.bits);
        // 把 oldisa 赋值给 newisa，此时 isa.bits/oldisa/newisa 三者是相同的
        newisa = oldisa;
        
        if (slowpath(!newisa.nonpointer)) {
            // 如果对象的 isa 只是原始指针 （Class isa/Class cls）
            
            // __arm64__ && !__arm642__ 平台下，取消 &isa.bits 的独占访问标记
            // x86_64 下什么都不需要做，对它而言上面的 LoadExclusive 也只是一个原子读取 (atomic_load)
            ClearExclusive(&isa.bits);
            
            // 如果当前对象是元类对象，则直接返回 false 
            if (rawISA()->isMetaClass()) return false;
            
            // 如果当前 SideTable 加锁了则进行解锁
            if (sideTableLocked) sidetable_unlock();
            
            // 只针对 isa 是原始 Class cls 的对象调用的 sidetable_release 函数
            return sidetable_release(performDealloc);
        }
        
        // don't check newisa.fast_rr; we already called any RR overrides
        // 不要检查 newisa.fast_rr; 我们之前已经调用过所有 RR 的重载
        
        // extra_rc-- 
        uintptr_t carry;
        newisa.bits = subc(newisa.bits, RC_ONE, 0, &carry);  // extra_rc--
        
        // 如果发生了下溢出的话，要进行处理，如果没有发生的话就是结束循环，解锁并执行 return false;
        if (slowpath(carry)) {
            // don't ClearExclusive()
            // 不执行 ClearExclusive()
            // 这里直接 goto 到 underflow 中去处理溢出
            goto underflow;
        }
        
    // 这里结束循环的方式同 rootRetain 函数，都是为了保证 isa.bits 能正确修改
    // StoreExclusive 和 StoreReleaseExclusive 的区别在于 memory_order_relaxed 和 memory_order_release
    // 可参考 https://en.cppreference.com/w/cpp/atomic/memory_order
    
    // 当 &isa.bits 与 oldisa.bits 相同时，把 newisa.bits 复制给 &isa.bits，并返回 true
    // 当 &isa.bits 与 oldisa.bits 不同时，
    // 把 oldisa.bits 复制给 &isa.bits, 并返回 false （此时会继续进行 do wehile 循环）
    } while (slowpath(!StoreReleaseExclusive(&isa.bits, oldisa.bits, newisa.bits)));
    
    // 如果未下溢出的话，不需要 goto underflow，如果 Sidetable 加锁了，
    // 则进行解锁，然后返回 false，函数执行结束
    if (slowpath(sideTableLocked)) sidetable_unlock();
    return false;

 underflow:
    // newisa.extra_rc-- underflowed: borrow from side table or deallocate
    // newisa.extra_rc-- 发生溢出时，有两种方式进行处理：
    // 1. 如果 SideTable 中有保存对象的引用计数的话可以从 SideTable 中借用
    // 2. 如果 SideTable 中没有保存对象的引用计数的话，表示对象需要执行销毁了

    // abandon newisa to undo the decrement
    newisa = oldisa;

    if (slowpath(newisa.has_sidetable_rc)) {
        // 如果 newisa.has_sidetable_rc 为 true，表示在 SideTable 中有保存对象的引用计数
        if (!handleUnderflow) {
            ClearExclusive(&isa.bits);
            
            // 如果 handleUnderflow 为 false，则调用 rootRelease_underflow，“递归” 调用 rootRelease 处理溢出
            return rootRelease_underflow(performDealloc);
        }

        // Transfer retain count from side table to inline storage.
        // 将 retain count 从 SideTable 中转移到 isa.extra_rc 中保存。

        if (!sideTableLocked) {
            // 如果 SideTable 未加锁
            
            // 同上，清除独占标记
            ClearExclusive(&isa.bits);
            
            // 给 SideTable 加锁
            sidetable_lock();
            // 并把加锁标记置为 true
            sideTableLocked = true;
            
            // Need to start over to avoid a race against the nonpointer -> raw pointer transition.
            
            // 回到 retry
            goto retry;
        }

        // Try to remove some retain counts from the side table.
        // 尝试从 SideTable 中移除一些引用计数。
        
        // 是从 SideTable 借一些引用计数出来，borrowed 是借到的值，可能是 0，也可能是 RC_HALF
        // refcnts 中保存的引用计数是 RC_HALF 的整数倍，
        // 每次 retain 溢出时都是往 refcnts 中转移 RC_HALF，
        // 剩下的 RC_HALF 放在 extra_rc 字段中
        size_t borrowed = sidetable_subExtraRC_nolock(RC_HALF);

        // To avoid races, has_sidetable_rc must remain set even if the side table count is now zero.
        // 为了避免竞态，即使 SideTable 计数现在为零，也必须保持 has_sidetable_rc 之前的设置。
        
        if (borrowed > 0) {
            // borrowed 表示从 SideTable 借到引用计数了
            
            // Side table retain count decreased.
            // SideTable 引用计数 减少。
            // Try to add them to the inline count.
            // 尝试将借来的引用计数增加到 extra_rc 中。
            
            // 赋值。（包含减 1 的操作）
            newisa.extra_rc = borrowed - 1;  // redo the original decrement too
            
            // 原子保存修改后的 isa.bits
            bool stored = StoreReleaseExclusive(&isa.bits, 
                                                oldisa.bits, newisa.bits);
            if (!stored) {
                // 如果失败的话
                
                // Inline update failed. 
                // extra_rc 更新失败。
                
                // Try it again right now. 
                // This prevents livelock on LL/SC architectures where the side
                // table access itself may have dropped the reservation.
                // 立即进行重试。
                // 这样可以防止在 LL/SC体系结构上发生 livelock，在这种情况下 SideTable 访问本身可能已取消预留。
                // 活锁可参考: https://www.zhihu.com/question/20566246
                
                isa_t oldisa2 = LoadExclusive(&isa.bits);
                isa_t newisa2 = oldisa2;
                
                if (newisa2.nonpointer) {
                    uintptr_t overflow;
                    // 把借来的引用计数增加到 extra_rc 中
                    newisa2.bits = 
                        addc(newisa2.bits, RC_ONE * (borrowed-1), 0, &overflow);
                    if (!overflow) {
                        // 如果还是失败的话，下面 goto retry 再重来
                        stored = StoreReleaseExclusive(&isa.bits, oldisa2.bits, 
                                                       newisa2.bits);
                    }
                }
            }

            if (!stored) {
                // 如果还是失败了。
                // Inline update failed.
                // Put the retains back in the side table.
                // 把从 SideTable 借来的引用计数还放回到 SideTable 中去。
                
                sidetable_addExtraRC_nolock(borrowed);
                
                // 然后直接 goto retry; 进行全盘重试
                goto retry;
            }

            // Decrement successful after borrowing from side table.
            // 减去从 SideTable 借来的引用计数成功。
            
            // This decrement cannot be the deallocating decrement
            // - the side table lock and has_sidetable_rc bit
            // ensure that if everyone else tried to -release while we worked, 
            // the last one would block.
            
            // 解锁
            sidetable_unlock();
            // 返回 false 
            return false;
        }
        else {
            // SideTable 是空的，执行 dealloc 分支
            // Side table is empty after all. Fall-through to the dealloc path.
        }
    }

    // Really deallocate.
    // 执行销毁。

    if (slowpath(newisa.deallocating)) {
        // 如果对象已经被标记了正在执行释放...
        // 这里又进行释放，明显是发生了过度释放...
        
        // 清除独占标记
        ClearExclusive(&isa.bits);
        
        // 如果 SideTable 加锁了则进行解锁
        if (sideTableLocked) sidetable_unlock();
        // 调用 overrelease_error，crash 报错...
        // 对象在销毁的过程中过度释放；中断 objc_overrelease_during_dealloc_error 进行调试
        return overrelease_error();
        // does not actually return
    }
    
    // 把对象的 isa 的 deallocating 置为 true。isa 的又一个字段被设置了，越来的越多的字段被发现设置位置了。 
    newisa.deallocating = true;
    
    // 设置 &isa.bits，如果失败，则 goto retry;
    if (!StoreExclusive(&isa.bits, oldisa.bits, newisa.bits)) goto retry;

    // 如果加锁了，则进行解锁。
    if (slowpath(sideTableLocked)) sidetable_unlock();

    // 这个函数以当前的水平实在是看不懂呀...
    __c11_atomic_thread_fence(__ATOMIC_ACQUIRE);

    if (performDealloc) {
        // 如果 performDealloc 为 true，则以消息发送的方式调用 dealloc 
        ((void(*)(objc_object *, SEL))objc_msgSend)(this, @selector(dealloc));
    }
    
    return true;
}
```
#### sidetable_release
```c++
// return uintptr_t instead of bool so that the various raw-isa -release paths all return zero in eax
// 返回 uintptr_t 而不是 bool，以便各种 raw-isa -release路径在 eax 中都返回零

uintptr_t
objc_object::sidetable_release(bool performDealloc)
{
// 如果当前平台支持 isa 优化
#if SUPPORT_NONPOINTER_ISA
    // 如果 isa 是优化的 isa 则直接执行断言，
    // sidetable_release 函数只能在对象的 isa 是原始 isa 时调用（Class cls）
    ASSERT(!isa.nonpointer);
#endif
    
    // 从全局的 SideTalbes 中找到 this 所处的 SideTable
    SideTable& table = SideTables()[this];
    
    // 临时变量，标记是否需要执行 dealloc
    bool do_dealloc = false;
    
    // 加锁
    table.lock();
    
    // it 的类型是: std::pair<DenseMapIterator<std::pair<Disguised<objc_object>, size_t>>, bool>
    // try_emplace 处理两种情况：
    // 1. 如果 this 在 refcnts 中还不存在，则给 this 在 buckets 中找一个 BucketT，
    //    KeyT 放 this， ValueT 放 SIDE_TABLE_DEALLOCATING，然后使用这个 BucketT 构建一个 iterator，
    //    然后用这个 iterator 和 true 构造一个 std::pair<iterator, true> 返回。
    // 2. 如果 this 在 refcnts 中已经存在了，则用 this 对应的 BucketT 构建一个 iterator,
    //    然后用这个 iterator 和 false 构造一个 std::pair<iterator, false> 返回。
    auto it = table.refcnts.try_emplace(this, SIDE_TABLE_DEALLOCATING);
    
    // refcnt 是引用计数值的引用。
    // it.first 是 DenseMapIterator，它的操作符 -> 被重写了返回的是 DenseMpaIterator 的 Ptr 成员变量，
    // 然后 Ptr 的类型是 BucketT 指针,
    // 然后这里的 ->second 其实就是 BucketT->second，其实就是 size_t，正是保存的对象的引用计数数据。
    auto &refcnt = it.first->second;
    
    if (it.second) {
        // 如果 it.second 为 true，表示 this 第一次放进 refcnts 中，且 BucketT.second 已经被置为 SIDE_TABLE_DEALLOCATING，
        // 标记为需要执行 dealloc
        do_dealloc = true;
    } else if (refcnt < SIDE_TABLE_DEALLOCATING) {
        // SIDE_TABLE_WEAKLY_REFERENCED may be set. Don't change it.
        // 如果 refcnt < SIDE_TABLE_DEALLOCATING，那可能的情况就是 SIDE_TABLE_WEAKLY_REFERENCED 或者为 0
        // 标记为需要执行 dealloc
        do_dealloc = true;
        
        // 与 SIDE_TABLE_DEALLOCATING 执行或操作，表示把 refcnt 标记为 DEALLOCATING
        refcnt |= SIDE_TABLE_DEALLOCATING;
    } else if (! (refcnt & SIDE_TABLE_RC_PINNED)) {
        // refcnt & SIDE_TABLE_RC_PINNED 值为 false 的话表示，
        // rcfcnts 中保存的 this 对应的 BucketT 的 size_t 还没有溢出，还可正常进行操作存储 this 的引用计数
        // refcnt 减去 SIDE_TABLE_RC_ONE
        refcnt -= SIDE_TABLE_RC_ONE;
    }
    
    // 解锁
    table.unlock();
    
    if (do_dealloc  &&  performDealloc) {
        // 如果 do_dealloc 被标记为需要 dealloc 并且入参 performDealloc 为 true，
        // 则以 objc_msgSend 消息发送的方式调用对象的 dealloc 方法
        ((void(*)(objc_object *, SEL))objc_msgSend)(this, @selector(dealloc));
    }
    
    return do_dealloc;
}
```
#### rootRelease_underflow
```c++
NEVER_INLINE uintptr_t
objc_object::rootRelease_underflow(bool performDealloc)
{
    // handleUnderflow 为 true，来处理溢出情况
    return rootRelease(performDealloc, true);
}
```
#### sidetable_subExtraRC_nolock
```c++
// Move some retain counts from the side table to the isa field.
// 将一些引用计数从 SideTable 移动到 isa 的 extra_rc 中。
// Returns the actual count subtracted, which may be less than the request.
// 返回减去的实际计数，可能少于要求。

size_t 
objc_object::sidetable_subExtraRC_nolock(size_t delta_rc)
{
    // 对象的 isa 必须是非指针
    ASSERT(isa.nonpointer);
    // 取得 SideTable 
    SideTable& table = SideTables()[this];

    // it 的类型是: DenseMapIterator<  std::pair<Disguised<objc_object>, size_t>  >
    // 找到 this 对应的 BucketT，并构建一个 DenseMapItertor
    RefcountMap::iterator it = table.refcnts.find(this);
    
    // DenseMapIterator 的 operator== 重写了，内部比较的是两个迭代器的 Ptr 成员变量
    // 如果 it == table.refcnts.end() 表示未找到 this 对应的 BucketT
    // it->second == 0，表示 BucketT 的 size_t 是 0
    if (it == table.refcnts.end()  ||  it->second == 0) {
        // Side table retain count is zero. Can't borrow.
        // SideTable 的引用计数是 0。不能借
        return 0;
    }
    
    // 取得 refcnts 中对象的引用计数
    size_t oldRefcnt = it->second;

    // 又见到在 retain 中已经见过的断言判断
    // 从 SideTable 中取出的引用计数的数据，第一位和第二位必须是 0
    
    // isa-side bits should not be set here
    ASSERT((oldRefcnt & SIDE_TABLE_DEALLOCATING) == 0);
    ASSERT((oldRefcnt & SIDE_TABLE_WEAKLY_REFERENCED) == 0);
    
    // 从 oldRefcnt 中减去入参要借用的引用计数的数值
    // 这里的减操作是不会有负数的，因为 BucketT 最小值 VlaueT 是 RC_HALF
    size_t newRefcnt = oldRefcnt - (delta_rc << SIDE_TABLE_RC_SHIFT);
    
    // 断言：减操作以后，oldRefcnt 大于 newRefcnt，那如果 delta_rc 为 0 呢？oldRefcnt 会等于 newRefcnt
    // 还有减完 newRefcnt 可能是 0，也就是意味着 SideTable 不再保存对象的引用计数了，此时 has_sidetable_rc 应该还是 true
    ASSERT(oldRefcnt > newRefcnt);  // shouldn't underflow
    
    // 更新 SideTable 中的引用计数数据
    it->second = newRefcnt;
    // 返回借的值
    return delta_rc;
}
```

&emsp;**与aotorelease 相关的内容留到下篇再写。**

### rootTryRetain
```c++
ALWAYS_INLINE bool 
objc_object::rootTryRetain()
{
    return rootRetain(true, false) ? true : false;
}
```
### rootReleaseShouldDealloc
```c++
ALWAYS_INLINE bool 
objc_object::rootReleaseShouldDealloc()
{
    return rootRelease(false, false);
}
```
### rootRetainCount
&emsp;如果对象的 `isa` 是非指针的话，引用计数同时在 `extra_rc` 字段和 `SideTable` 中保存，要求它们的和。如果对象的 `isa` 是原始 `isa` 的话，对象的引用计数数据只保存在 `SideTable` 中。
```c++
inline uintptr_t 
objc_object::rootRetainCount()
{
    // 如果是 Tagged Pointer 的话，获取它的引用计数则直接返回 (uintptr_t)this
    if (isTaggedPointer()) return (uintptr_t)this;
    
    // 加锁
    sidetable_lock();
    
    // 以原子方式加载 &isa.bits 数据
    isa_t bits = LoadExclusive(&isa.bits);
    // 如果是 __arm64__ && !__arm64e__ 平台下，要清除独占标记
    ClearExclusive(&isa.bits);
    
    if (bits.nonpointer) {
        // 如果对象的 isa 是非指针的话，引用计数同时在 extra_rc 字段和 SideTable 中保存，要求它们的和
        // 这里加 1， 是因为 extra_rc 存储的是对象本身之外的引用计数的数量
        uintptr_t rc = 1 + bits.extra_rc;
        
        // 如果 has_sidetable_rc 位为 1，则表示在 SideTable 中也保存有对象的引用计数数据
        if (bits.has_sidetable_rc) {
            // 找到对象的在 SideTable 中的引用计数并增加到 rc 中
            rc += sidetable_getExtraRC_nolock();
        }
        // 解锁
        sidetable_unlock();
        // 返回 rc
        return rc;
    }

    sidetable_unlock();
    // 如果对象的 isa 是原始 isa 的话，对象的引用计数数据只保存在 SideTable 中
    return sidetable_retainCount();
}
```
#### sidetable_getExtraRC_nolock
```c++
size_t 
objc_object::sidetable_getExtraRC_nolock()
{
    // 此函数只限定 isa 是非指针的对象调用
    ASSERT(isa.nonpointer);
    
    // 从全局的 SideTables 中找到 this 所处的 SideTable
    SideTable& table = SideTables()[this];
    // 查找对象的引用计数
    RefcountMap::iterator it = table.refcnts.find(this);
    // 如果未找到，返回 0
    if (it == table.refcnts.end()) return 0;
    // 如果找到了做一次右移操作，后两位是预留的标记位
    else return it->second >> SIDE_TABLE_RC_SHIFT;
}
```
#### sidetable_retainCount
```c++
uintptr_t
objc_object::sidetable_retainCount()
{
    // 找到 this 所在的 SideTable
    SideTable& table = SideTables()[this];

    // refcnt_result 初始为 1，因为 SideTable 中存储的是对象本身之外的引用计数的数量
    size_t refcnt_result = 1;
    
    // 加锁
    table.lock();
    
    // 在 refcnts 中查找对象的引用计数
    RefcountMap::iterator it = table.refcnts.find(this);
    if (it != table.refcnts.end()) {
        // this is valid for SIDE_TABLE_RC_PINNED too
        // 这也对 SIDE_TABLE_RC_PINNED 有效
        
        // 移位并增加到 refcnt_result
        refcnt_result += it->second >> SIDE_TABLE_RC_SHIFT;
    }
    
    // 解锁
    table.unlock();
    return refcnt_result;
}
```
### rootIsDeallocating
```c++
inline bool 
objc_object::rootIsDeallocating()
{
    // 如果是 Tagged Pointer 直接返回 false
    if (isTaggedPointer()) return false;
    
    // 如果 isa 是非指针的对象，则直接返回 isa 的 deallocating 字段
    // 该字段保存的正是对象是否正在进行释放
    if (isa.nonpointer) return isa.deallocating;
    
    // 如果 isa 是原始指针的对象，它的是否正在释放状态是保存在 refcnts 中的引用计数数据的第二位中
    return sidetable_isDeallocating();
}
```
#### sidetable_isDeallocating
```c++
bool 
objc_object::sidetable_isDeallocating()
{
    // 在全局的 SideTables 中找到对象所处的 SideTable
    SideTable& table = SideTables()[this];

    // NO SPINLOCK HERE
    // 此处没有 SPINLOCK 
    
    // _objc_rootIsDeallocating() is called exclusively by _objc_storeWeak(), 
    // which already acquired the lock on our behalf.
    // _objc_rootIsDeallocating() 函数仅由 _objc_storeWeak() 函数独占调用，在调用之前已经获得了锁。
    
    // fixme can't do this efficiently with os_lock_handoff_s
    // fixme os_lock_handoff_s 无法有效地做到这一点
    // if (table.slock == 0) {
    //     _objc_fatal("Do not call -_isDeallocating.");
    // }

    // 在 refcnts 中查找 this 的引用计数数据
    RefcountMap::iterator it = table.refcnts.find(this);
    // 如果 it 不等于 end()，并且引用计数数据的第二位是 1， 则表明对象正在进行释放
    return (it != table.refcnts.end()) && (it->second & SIDE_TABLE_DEALLOCATING);
}
```
### clearDeallocating
&emsp;对象释放时的清理操作，这里涉及到：

1. 如果对象有弱引用的话，则对象释放了要把那些弱引用置为 `nil`。
2. 要从 `refcnts` 中清除对象，把存放对象引用计数数据的 `BucketT` 的 `second` 执行析构操作，然后把 `first` 置为 `TombstoneKey`。

涉及到的函数调用流程还挺长的，我们来一步一步看。
```c++
inline void 
objc_object::clearDeallocating()
{
    if (slowpath(!isa.nonpointer)) {
        // Slow path for raw pointer isa.
        // isa 是原始指针的对象
        sidetable_clearDeallocating();
    }
    else if (slowpath(isa.weakly_referenced  ||  isa.has_sidetable_rc)) {
        // Slow path for non-pointer isa with weak refs and/or side table data.
        // 针对 isa 是非指针的对象，isa.weakly_referenced 为真，或者 isa.has_sidetable_rc 为真
        clearDeallocating_slow();
    }

    assert(!sidetable_present());
}
```
#### sidetable_clearDeallocating
```c++
void 
objc_object::sidetable_clearDeallocating()
{
    SideTable& table = SideTables()[this];

    // clear any weak table items
    // 清除所有的弱引用项
    
    // clear extra retain count and deallocating bit
    // 清除 extra retain count 和 deallocating 标志位
    // (fixme warn or abort if extra retain count == 0 ?)
    
    // 加锁
    table.lock();
    // 在 refcnts 中查找 BucketT
    RefcountMap::iterator it = table.refcnts.find(this);
    if (it != table.refcnts.end()) {
        // 如果引用计数数据的第一位是 1，则表示该对象存在弱引用
        if (it->second & SIDE_TABLE_WEAKLY_REFERENCED) {
        // 调用 weak_clear_no_lock 函数首先根据 this 找到 this 在 table.weak_table 中对应的 weak_entry_t
        // 然后查找 weak_entry_t 中的 referrers 的数量已经起始地址，然后循环遍历把每个 referrer 置为 nil
        // 然后把 weak_entry_t 从 weak_table 中移除，并判断是否需要紧缩 weak_table 容量
        weak_clear_no_lock(&table.weak_table, (id)this);
        }
        
        // 把 it 指向的 BucketT second 执行析构，把 first 置为 TombstoneKey 即把对象的 BucketT 从 Buckets 中移除。
        // NumEntries 减 1，NumTombstones 加 1，并判断是否需要紧缩 Buckets 容量。
        table.refcnts.erase(it);
    }
    // 解锁
    table.unlock();
}
```
#### clearDeallocating_slow
```c++
NEVER_INLINE void
objc_object::clearDeallocating_slow()
{
    // 断言条件：isa 是非指针的对象，
    // isa.weakly_referenced（有弱引用） 或者 isa.has_sidetable_rc（SideTable 中保存有引用计数） 为真
    ASSERT(isa.nonpointer  &&  (isa.weakly_referenced || isa.has_sidetable_rc));
    
    // 在全局 SideTables 中找到 this 所在的 SideTable 
    SideTable& table = SideTables()[this];
    // 加锁
    table.lock();
    if (isa.weakly_referenced) {
        // 如果该对象有弱引用
        
        // 调用 weak_clear_no_lock 函数首先根据 this 找到 this 在 table.weak_table 中对应的 weak_entry_t
        // 然后查找 weak_entry_t 中的 referrers 的数量已经起始地址，然后循环遍历把每个 referrer 置为 nil
        // 然后把 weak_entry_t 从 weak_table 中移除，并判断是否需要紧缩 weak_table 容量
        weak_clear_no_lock(&table.weak_table, (id)this);
    }
    
    if (isa.has_sidetable_rc) {
        // 如果该对象在 SideTable 中有保存引用计数，要把它从 Buckets 中移除
        // 找到 this 对应的 BucketT，把 BucketT second 执行析构，
        // 把 first 置为 TombstoneKey 即把对象的 BucketT 从 Buckets 中移除。
        // NumEntries 减 1，NumTombstones 加 1，并判断是否需要紧缩 Buckets 容量。
        table.refcnts.erase(this);
    }
    
    // 解锁
    table.unlock();
}
```
&emsp;到这里 `objc_object` 的 `retain` `release` `retainCount` 的操作就完全通了一遍了，由于这里已经篇幅过长了，下面的内容下篇继续分析。

## 参考链接
**参考链接:🔗**
+ [atomic_compare_exchange_weak](https://en.cppreference.com/w/c/atomic/atomic_compare_exchange)
+ [atomic_load, atomic_load_explicit](https://en.cppreference.com/w/c/atomic/atomic_load)
+ [从源码角度看苹果是如何实现 retainCount、retain 和 release 的](https://juejin.im/post/6844903847131889677)
+ [Perfect forwarding and universal references in C++](https://eli.thegreenplace.net/2014/perfect-forwarding-and-universal-references-in-c/)
+ [操作系统内存管理(思维导图详解)](https://blog.csdn.net/hguisu/article/details/5713164)
+ [内存管理](https://www.jianshu.com/p/8d742a44f0da)
