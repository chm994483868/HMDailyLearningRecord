# iOS 从源码解析Runtime (七)：聚焦 objc_object(dealloc、autorelease返回值优化篇)

> &emsp;上一篇我们非常非常详尽的分析了自动释放池的相关的源码，这篇我们继续学习 `objc_object` 剩余的函数，目前只剩下`rootDealloc` 和 `rootAutorelease` 的实现流程，本篇前面部分首先把 `rootDealloc` 的调用流程讲解一下，然后后面的重点都放在 `Autorelease` 对函数返回值的优化上。

## rootReleaseShouldDealloc
```c++
ALWAYS_INLINE bool 
objc_object::rootReleaseShouldDealloc()
{
    // 第五篇已经详细分析过 rootRelease 函数
    return rootRelease(false, false);
}
```
## rootDealloc
&emsp;对象 `Dealloc` 的内部实现。如下条件全部为真的话，可以直接调用 `free` 进行快速释放内存。

1. 对象的 `isa` 是优化的 `isa`。
2. 对象不存在弱引用。
3. 对象没有关联对象。
4. 对象没有自定义的 `C++` 的析构函数。
5. 对象的引用计数没有保存在 `SideTable` 中。

```c++
inline void
objc_object::rootDealloc()
{
    // 如果是 Tagged Pointer，则直接 return
    
    // 针对下面的 fixme:
    // 这里的大概 Tagged Pointer 的是不会执行到这里的，
    // 因为 dealloc 函数的调用是在 rootRelease 函数的最后通过
    // ((void(*)(objc_object *, SEL))objc_msgSend)(this, @selector(dealloc)) 来执行的，
    // 且正常情况下面我们不能主动调用 dealloc 函数，
    // 而 Tagged Pointer 调用 rootRelease 函数时会直接返回 false，
    // 所以很大概率下 Tagged Pointer 是走不到这里来的...
    
    if (isTaggedPointer()) return;  // fixme necessary? 是必要的吗？

    // 如下条件全部为真的话，可以直接调用 free 进行快速释放内存
    // 1. 对象的 isa 是优化的 isa。
    // 2. 对象不存在弱引用。
    // 3. 对象没有关联对象。
    // 4. 对象没有 C++ 的析构的函数。
    // 5. 对象的引用计数没有保存在 SideTable 中。
    if (fastpath(isa.nonpointer  &&  
                 !isa.weakly_referenced  &&  
                 !isa.has_assoc  &&  
                 !isa.has_cxx_dtor  &&  
                 !isa.has_sidetable_rc))
    {
        // 断言：1. 对象的引用计数没有保存在 SideTable 中 
        // 2. this 在 weak_table 中不存在弱引用
        // (内部详细就是判断 weak_table 的 weak_entries 中是否有 this 存在) 
        
        // 刚刚点进 sidetable_present 函数时发现，此函数只是 DEBUG 模式下的函数，
        // 然后再把模式切换到 Release 模式下时编译运行，再 command 点击 assert，
        // 看到宏定义是 #define assert(e) ((void)0)
        // 至此 看了这么久源码才发现，原来这个随处可见的断言只是针对 DEBUG 模式下使用的。
        
        assert(!sidetable_present());
        
        // 释放 this 的内存空间
        free(this);
    } 
    else {
        // 进入慢速释放的路径
        object_dispose((id)this);
    }
}
```
### object_dispose
```c++
id 
object_dispose(id obj)
{
    // 如果 obj 不存在，则直接返回 nil
    if (!obj) return nil;

    // 释放对象内存前的一些清理工作
    objc_destructInstance(obj); 
    
    // 释放对象内存
    free(obj);

    return nil;
}
```
### objc_destructInstance
```c++
/*
* objc_destructInstance
* Destroys an instance without freeing memory.
* 在对象释放内存之前清理对象相关的内容。

* Calls C++ destructors.
* 如果有 C++ 析构函数，则调用 C++ 析构函数。

* Calls ARC ivar cleanup.
* ARC ivar 的清理工作。（这个是指哪一部分清理工作？） 

* Removes associative references.
* 如果对象有关联对象的话，移除对象的关联对象。

* Returns `obj`. Does nothing if `obj` is nil.
* 返回 obj。 如果 obj 是 nil 的话则不执行任何操作。

*/
void *objc_destructInstance(id obj) 
{
    if (obj) {
        // Read all of the flags at once for performance.
        // 一次读取所有标志位以提高性能。
        
        // 是否有 C++ 析构函数 
        bool cxx = obj->hasCxxDtor();
        // 是否有关联对象
        bool assoc = obj->hasAssociatedObjects();

        // This order is important.
        // 下面的执行顺序很重要。
        
        // 如果有，则执行 C++ 析构函数
        if (cxx) object_cxxDestruct(obj);
        
        // 如果有，则移除关联对象。（具体实现可参考前面关联对象那篇文章）
        if (assoc) _object_remove_assocations(obj);
        
        // 清除对象的 Deallocating 状态，主要是对对象所处的 SideTable 进行清理工作 
        obj->clearDeallocating();
    }

    return obj;
}
```
### clearDeallocating
```c++
inline void 
objc_object::clearDeallocating()
{
    if (slowpath(!isa.nonpointer)) {
        // 对象的 isa 是非优化的 isa 
        
        // Slow path for raw pointer isa.
        // 针对 isa 是原始指针的对象的慢速执行路径
        
        // 1. 如果对象有弱引用，则调用 weak_clear_no_lock 函数执行清理工作，
        //    把对象的所有弱引用置为 nil，
        //    并对象的 weak_entry_t 移除（必要时还会缩小 weak_table_t 容量）
        // 2. 处理对象在 refcnts 中的 Bucket，
        //    ValueT 执行析构 KeyT 赋值为 TombstoneKey.
        sidetable_clearDeallocating();
    }
    else if (slowpath(isa.weakly_referenced  ||  isa.has_sidetable_rc)) {
        // Slow path for non-pointer isa with weak refs and/or side table data.
        // 同上 isa 是优化 isa 的对象的慢速执行路径
        
        // 包含的操作也基本完全相同
        clearDeallocating_slow();
    }

    // 可验证上面的 SideTable 的操作是否都完成了
    assert(!sidetable_present());
}
```

### sidetable_clearDeallocating
```c++
void 
objc_object::sidetable_clearDeallocating()
{
    // 从全局的 SideTables 中取出 SideTable
    SideTable& table = SideTables()[this];

    // clear any weak table items
    // 清除所有弱引用项（把弱引用置为 nil）
    
    // clear extra retain count and deallocating bit
    // 清除 SideTable 中的引用计数以及 deallocating 位
    
    // (fixme warn or abort if extra retain count == 0 ?)
    // (fixme 如果额外保留计数== 0，则发出警告或中止 ?)
    
    // 加锁
    table.lock();
    
    // 从 refcnts 中取出 this 对应的 BucketT（由 BucketT 构建的迭代器）
    RefcountMap::iterator it = table.refcnts.find(this);
    
    // 如果找到了
    if (it != table.refcnts.end()) {
    
        // ->second 取出 ValueT，最后一位是有无弱引用的标志位
        if (it->second & SIDE_TABLE_WEAKLY_REFERENCED) {
            // 具体实现可参考 weak 那几篇文章
            weak_clear_no_lock(&table.weak_table, (id)this);
        }
        
        // 把 this 对应的 BucketT "移除"（标记为移除）
        table.refcnts.erase(it);
    }
    table.unlock();
}
```
### clearDeallocating_slow
```c++
// Slow path of clearDeallocating() 
// for objects with nonpointer isa
// that were ever weakly referenced 
// or whose retain count ever overflowed to the side table.

NEVER_INLINE void
objc_object::clearDeallocating_slow()
{
    ASSERT(isa.nonpointer  &&  (isa.weakly_referenced || isa.has_sidetable_rc));

    SideTable& table = SideTables()[this];
    table.lock();
    
    // 同上，清理弱引用
    if (isa.weakly_referenced) {
        weak_clear_no_lock(&table.weak_table, (id)this);
    }
    
    // 同上，清理 refcnts 中的引用数据
    if (isa.has_sidetable_rc) {
        table.refcnts.erase(this);
    }
    table.unlock();
}
```
&emsp;至此 `rootDealloc` 函数涉及的全流程就分析完毕了，主要是在对象 `free` 之前做一些清理和收尾工作。（这里有一个疑问，`ViewController` 的 `strong` 属性的变量是在什么时候释放的？）

## sidetable_lock
```c++
void 
objc_object::sidetable_lock()
{
    // SideTable 加锁
    SideTable& table = SideTables()[this];
    table.lock();
}
```
## sidetable_unlock
```c++
void 
objc_object::sidetable_unlock()
{
    // SideTable 解锁
    SideTable& table = SideTables()[this];
    table.unlock();
}
```
## MRC 函数返回值的释放操作
&emsp;一些结论：

&emsp;关于 `Autorelease`:
1. 在没有手加 `AutoreleasePool` 的情况下，`Autorelease` 对象是在当前的 `runloop` 迭代结束时释放的，而它能够释放的原因是系统在每个 `runloop` 迭代中都加入了自动释放池 `Push` 和 `Pop`。
2. `AutoreleasePool` 执行 `pop` 时完成的最重要的事情就是对池里的所有对象执行一次 `release` 操作。
3. `AutoreleasePool` 从不会 `retain` 放进池里的对象，它所做的唯一事情就是延迟释放，直白一点理解的话就是在池 `pop` 时 `release` 放进池里的对象。
4. 当对象连续调用 `autorelease` 函数时，对象会被放进自动释放池多次（对象放进自动释放池不会做重复检测），当自动释放池 `pop` 时会对对象调用对应次数的 `release` 操作，此时极有可能导致对象过度释放而使程序 `crash`。

&emsp;关于 `MRC` 下函数返回值放进自动释放池:
```c++
+ (CusPerson *)returnInstanceValue; {
    CusPerson *temp = [[[CusPerson alloc] init] autorelease]; // 情况 1
    CusPerson *temp = [[CusPerson alloc] init]; // 情况 2
    
    NSLog(@"%@ %p", NSStringFromSelector(_cmd), temp);
    return temp;
}
```
+ 情况 1 `temp` 调用 `autorelease` 函数被放进自动释放池。当我们在外部调用 `returnInstanceValue` 函数获取一个 `CusPerson` 对象并且不做 `retain` 操作时，我们不需要在调用 `returnInstanceValue` 函数的地方主动去执行一次 `release` 操作，`CusPerson` 对象在 `AutoreleasePool` 执行 `pop` 时会被 `release` 一次后得到正确销毁。
+ 情况 2 `temp` 对象没有放进自动释放池，需要在调用 `returnInstanceValue` 后当不再需要返回的对象时，对象要主动调用一次 `release` 或 `autorelease` 保证对象能正确释放。
+ 如果我们需要一直持有函数返回的对象，那么我们可以主动调用 `retain` 函数或者用一个 `retain/strong` 修饰的属性来接收函数返回值，当我们不使用对象时需要在合适的地方调用 `release` 操作，保证对象能正常销毁防止内存泄漏。
+ 成员变量默认是持有赋值给它的对象（默认是 \_\_strong 修饰的），属性的话根据不同的修饰符来决定是否持有赋值给它的对象。（`strong/retain/weak/unsafe_unretain`）。

&emsp;`ARC` 下情况则大不相同，下面我们开始分析。

## rootAutorelease
&emsp;看到 `if (prepareOptimizedReturn(ReturnAtPlus1)) return (id)this;` 并不是所有对象在调用 `rootAutorelease` 后都会被放进自动释放池的（`Tagged Pointer`  除外）。这里有一步优化操作，主要是针对函数返回值来做的，模拟出和 `MRC` 下函数返回值调用 `autorelease` 同等的效果，但是又不会真正的用到 `AutureleasePool`，这个优化的意思正是优化掉使用自动释放池的开销（优化为把对象保存在 `tls` 中，然后每次使用变量每次从 `tls` 读取）。
```c++
// Base autorelease implementation, ignoring overrides.
inline id 
objc_object::rootAutorelease()
{
    // 如果是 Tagged Pointer 则直接返回 (id)this
    if (isTaggedPointer()) return (id)this;
    
    // 如果 prepareOptimizedReturn(ReturnAtPlus1) 返回 true，则直接返回 (id)this 
    // (返回值还是入参本身，不用对其有什么纠结，
    // 目前还没看到什么时候优化操作是什么时候把对象放进 tls 中的)
    // 且这里传递的是 ReturnAtPlus1，表示优化时引用计数值加 1，
    // ReturnAtPlus1 会被保存在 tls 中。
    
    // 此函数的主要目的是判断是否能优化把对象放入自动释放池的操作，
    // 如果能的话会根据 ReturnAtPlus1 来进行优化，
    // 内部调用一个 callerAcceptsOptimizedReturn(__builtin_return_address(0)) 函数，
    
    // （它的目的就是判断 `objc_autoreleaseReturnValue` 的返回地址之后的汇编程序，
    //   是否存在对 `objc_retainAutoreleasedReturnValue` 
    //   或 `objc_unsafeClaimAutoreleasedReturnValue` 的调用，
    //   如果存在的话上层函数就可以对加入释放池的对象进行优化，
    //   不必将对象放入自动释放池，而是放在 `tls` 减少资源损耗。）
    
    // __builtin_return_address(0) 函数是找到当前函数的返回地址，
    // 并根据该地址继续找下面的汇编指令（x86_64 平台下的话）
    // 如果是 objc_retainAutoreleasedReturnValue 或者 
    // objc_unsafeClaimAutoreleasedReturnValue 的话表示可以进行优化
    // callerAcceptsOptimizedReturn 函数此时会返回 true，
    // 并会把 ReturnAtPlus1 根据 RETURN_DISPOSITION_KEY 保存到线程的存储空间内。
    
    if (prepareOptimizedReturn(ReturnAtPlus1)) return (id)this;
    
    // 否则正常调用 AutoreleasePoolPage::autorelease((id)this) 
    // 把 this 放进自动释放池
    // autorelease 实现的内容可参考上篇
    return rootAutorelease2();
}
```
### ReturnDisposition
&emsp;`ReturnDisposition` 代表优化设置，`ReturnAtPlus0` 即为优化时引用计数加 `0`，`ReturnAtPlus1` 即为优化时引用计数加 `1`。
```c++
enum ReturnDisposition : bool {
    ReturnAtPlus0 = false, 
    ReturnAtPlus1 = true
};
```
### RETURN_DISPOSITION_KEY
```c++
// Thread keys reserved by libc for our use.
// libc 保留供我们使用的线程 key。

// 如果是 #ifndef __ASSEMBLER__ 的情况下会有 #define __PTK_FRAMEWORK_OBJC_KEY0 40 这个宏定义

#if defined(__PTK_FRAMEWORK_OBJC_KEY0)

#   define SUPPORT_DIRECT_THREAD_KEYS 1 // 支持在线程存储空间内保存数据

// 这三个 key 暂时还没有见到在哪里使用
#   define TLS_DIRECT_KEY        ((tls_key_t)__PTK_FRAMEWORK_OBJC_KEY0) // #define __PTK_FRAMEWORK_OBJC_KEY0    40
#   define SYNC_DATA_DIRECT_KEY  ((tls_key_t)__PTK_FRAMEWORK_OBJC_KEY1) // #define __PTK_FRAMEWORK_OBJC_KEY1    41
#   define SYNC_COUNT_DIRECT_KEY ((tls_key_t)__PTK_FRAMEWORK_OBJC_KEY2) // #define __PTK_FRAMEWORK_OBJC_KEY2    42

// 从 tls 中获取 hotPage 使用 
#   define AUTORELEASE_POOL_KEY  ((tls_key_t)__PTK_FRAMEWORK_OBJC_KEY3) // #define __PTK_FRAMEWORK_OBJC_KEY3    43

// 只要是非 TARGET_OS_WIN32 平台下都支持优化 autoreleased 返回值
//（优化方案是把返回值放在 tls 中，避免加入到 autoreleasePool 中）
# if SUPPORT_RETURN_AUTORELEASE

// 从 tls 中获取 disposition，
// RETURN_DISPOSITION_KEY 对应的 value 是 ReturnDisposition 优化设置，
// 表示优化时引用计数加 0 或者 加 1
#   define RETURN_DISPOSITION_KEY ((tls_key_t)__PTK_FRAMEWORK_OBJC_KEY4) // #define __PTK_FRAMEWORK_OBJC_KEY4    44

# endif

#else

#   define SUPPORT_DIRECT_THREAD_KEYS 0

#endif

// Define SUPPORT_RETURN_AUTORELEASE to optimize autoreleased return values
// 定义 SUPPORT_RETURN_AUTORELEASE 以优化 autoreleased 返回值

#if TARGET_OS_WIN32

// TARGET_OS_WIN32 下不支持优化
#   define SUPPORT_RETURN_AUTORELEASE 0

#else

// 只要是非 TARGET_OS_WIN32 平台下都支持优化 autoreleased 返回值
//（优化方案是把返回值放在 tls 中，避免加入到 autoreleasePool 中）
#   define SUPPORT_RETURN_AUTORELEASE 1

#endif
```
### getReturnDisposition/setReturnDisposition
&emsp;这里又见到了 `tls_get_direct` 函数，已知它是运用 `Thread Local stroge (tls)` 机制在线程的存储空间里面根据 `key` 来获取对应的值，`static inline void tls_set_direct(tls_key_t k, void *value) ` 是根据 `key`，把 `value` 保存在 `tls` 中。（`tls` 涉及的内容太深了，这里先知悉其用法）

+ `getReturnDisposition` 函数是取得 `RETURN_DISPOSITION_KEY` 在 `tls` 中保存的值。
+ `setReturnDisposition` 函数是以 `RETURN_DISPOSITION_KEY` 为 `key`，把 `disposition` 保存在 `tls` 中。 

```c++
static ALWAYS_INLINE ReturnDisposition 
getReturnDisposition() {
    return (ReturnDisposition)(uintptr_t)tls_get_direct(RETURN_DISPOSITION_KEY);
}
```

```c++
static ALWAYS_INLINE void 
setReturnDisposition(ReturnDisposition disposition) {
    // 根据 RETURN_DISPOSITION_KEY 把传入的 disposition 保存在线程的存储空间内 
    tls_set_direct(RETURN_DISPOSITION_KEY, (void*)(uintptr_t)disposition);
}
```

### __builtin_return_address
1. 这里函数返回地址不是函数返回值的地址是函数被调用后返回的地址，这里要从汇编的角度来理解。
  当我们的代码编译为汇编代码后，汇编指令从上到下一行一行来执行。
  比如我们在函数1 内部调用了函数 2，开始时根据汇编指令一条一条执行函数1，当执行到需要调用函数 2 时，
  假如此时是用一个 `callq` 指令跳转到了函数 2 去执行，当函数 2 执行完毕后返回的地址是接着刚刚 `callq` 指令的地址的，
  然后从函数 2 返回的地址处接着一条一条继续执行函数 1 的指令。
  （大概意思就是函数嵌套调用时，被嵌套调用的函数执行完毕后返回的地址就是接下来的要执行的指令的地址，（或者是一个固定的偏移位置，根据编译器不同情况不同））
2. `gcc` 默认不支持 `__builtin_return_address(LEVEL)` 的参数为非 `0`。好像只支持参数为 `0`。
3. `__builtin_return_address(0)` 的含义是，得到当前函数返回地址，即此函数被别的函数调用，然后此函数执行完毕后，返回，所谓返回地址就是那时候的地址。
4. `__builtin_return_address(1)` 的含义是，得到当前函数的调用者的返回地址。注意是调用者的返回地址，而不是函数起始地址。

### callerAcceptsOptimizedReturn
&emsp;这个函数针对不同的平台（`__x86__64__`/`__arm__`/`__arm64__`/`__i386`/`unknown`）有完全不同的实现。它的目的就是判断 `objc_autoreleaseReturnValue` 的返回地址之后的汇编程序，是否存在对 `objc_retainAutoreleasedReturnValue` 或 `objc_unsafeClaimAutoreleasedReturnValue` 的调用，如果存在的话上层函数就可以对加入释放池的对象进行优化，不必将对象放入自动释放池，而是放在 `tls` 减少资源损耗。
```c++
/*
  Fast handling of return through Cocoa's +0 autoreleasing convention.
  快速处理函数的返回值，不把函数的返回值放进自动释放池。
  
  The caller and callee cooperate to keep the returned object out of
  the autorelease pool and eliminate redundant retain/release pairs.
  调用方和被调用方合作将返回的对象保留在自动释放池之外，并消除多余的 retain/release 对。

  An optimized callee looks at the caller's
  instructions following the return. 
  一个可以被优化的被调用方，会在函数返回后查看调用方的接下来的指令。
  
  If the caller's instructions are also optimized
  then the callee skips
  all retain count operations: no autorelease, no retain/autorelease.
  如果调用方的指令也得到了优化，则被调用方将跳过所有保留计数操作：
  （autorelease retain/release）
  
  Instead it saves the result's current retain
  count (+0 or +1) in thread-local storage. 
  而是将结果的当前保留计数（+0 或 +1 此处是指 ReturnDisposition）
  保存在线程的存储空间中。（tls）
  
  If the caller does not look optimized then the callee
  performs autorelease or retain/autorelease as usual.
  如果调用方的指令看起来不能被优化，则被调用方
  将照常执行 autorelease 或 retain/autorelease。
  
  An optimized caller looks at the thread-local storage. 
  一个优化的调用者会查看线程的本地存储空间。
  
  If the result is set then it performs any retain or release needed to change 
  the result from the retain count left by the
  callee to the retain count desired by the caller.
  如果设置了结果，则它将执行将结果从被调用者留下的保留
  计数更改为调用者所需的保留计数所需的任何保留或释放操作。
  
  Otherwise the caller assumes the result is currently at +0 from an unoptimized
  callee and performs any retain needed for that case.
  否则，调用者会假设来自未优化的被调用者的结果当前为 +0，并执行该情况所需的任何 retain 操作。
  
  There are two optimized callees:
  这是两个优化的被调用者（有返回值的函数）：
  
    objc_autoreleaseReturnValue
      result is currently +1. The unoptimized path autoreleases it. 
      // 不能优化时调用：return objc_autorelease(obj);
      // + 1，未优化的执行路径是对它们执行 autorelease.
      
    objc_retainAutoreleaseReturnValue
      result is currently +0. The unoptimized path retains and autoreleases it. 
      // 不能优化时调用：return objc_autorelease(objc_retain(obj)); 
      // + 0，未优化的执行路径是对它执行 retains 和 autorelease.

  There are two optimized callers:
  这是两个优化的调用者（调用了 有返回值的函数 的函数）：
  
    objc_retainAutoreleasedReturnValue
      caller wants the value at +1. The unoptimized path retains it.
      // 不能优化时调用：return objc_retain(obj);
      // 调用者希望引用计数 +1。未优化路径对它执行 retain 操作。
      
    objc_unsafeClaimAutoreleasedReturnValue
      caller wants the value at +0 unsafely. The unoptimized path does nothing.
      // 不能优化时调用：objc_release(obj); return obj;
      // 调用者希望引用计数 +1 （不安全的）。未优化路径什么都不做。

  Example:

    Callee:
    被调用者：
      // compute ret at +1
      return objc_autoreleaseReturnValue(ret);
    
    Caller:
    调用者
      ret = callee();
      
      ret = objc_retainAutoreleasedReturnValue(ret);
      // use ret at +1 here

    Callee sees the optimized caller, sets TLS, and leaves the result at +1.
    如果是优化的被调用方，会把对象保存在 TLS 中，不用再保存在自动释放池中。
    
    Caller sees the TLS, clears it, and accepts the result at +1 as-is.
    调用方会从 TLS 中取处出结果使用。

  The callee's recognition of the optimized caller is architecture-dependent.
  被调用方对优化的调用方的判断识别取决于体系结构。（是否能进行优化，不同平台下不同的判断方式）
  
  // 这里涉及到了一些汇编知识，当我们的 OC 源码被转换为汇编代码后，
  // 他们是连续的且内存地址都是固定，例如调用 objc_autoreleaseReturnValue 指令后，
  // 后面是跟的 objc_retainAutoreleasedReturnValue 指令还是
  // objc_unsafeClaimAutoreleasedReturnValue 都是已经固定的，
  // 我们可以沿着 __builtin_return_address 返回的地址，
  // 接着往下探测汇编指令到底是哪一条，继而来判断是直接返回 obj 还是把 obj 放进自动释放池。
  
  x86_64: Callee looks for `mov rax, rdi` followed by a call or 
    jump instruction to objc_retainAutoreleasedReturnValue or 
    objc_unsafeClaimAutoreleasedReturnValue. 
  i386:  Callee looks for a magic nop `movl %ebp, %ebp`
  (frame pointer register 帧指针寄存器)
  armv7: Callee looks for a magic nop `mov r7, r7`
  (frame pointer register). 
  arm64: Callee looks for a magic nop `mov x29, x29`
  (frame pointer register). 

  Tagged pointer objects do participate in the optimized return scheme, 
 // 标记的指针对象确实参与了优化的返回方案
  
  because it saves message sends. 
  They are not entered in the autorelease pool in the unoptimized case.
  // 因为它节省了消息发送。在未优化的情况下，它们也不会放入到自动释放池中。
  
*/
# if __x86_64__

static ALWAYS_INLINE bool 
callerAcceptsOptimizedReturn(const void * const ra0)
{
    const uint8_t *ra1 = (const uint8_t *)ra0;
    const unaligned_uint16_t *ra2;
    const unaligned_uint32_t *ra4 = (const unaligned_uint32_t *)ra1;
    const void **sym;

#define PREFER_GOTPCREL 0
#if PREFER_GOTPCREL
    // 48 89 c7    movq  %rax,%rdi
    // ff 15       callq *symbol@GOTPCREL(%rip)
    if (*ra4 != 0xffc78948) {
        return false;
    }
    if (ra1[4] != 0x15) {
        return false;
    }
    ra1 += 3;
#else
    // 48 89 c7    movq  %rax,%rdi
    // e8          callq symbol
    if (*ra4 != 0xe8c78948) {
        return false;
    }
    ra1 += (long)*(const unaligned_int32_t *)(ra1 + 4) + 8l;
    ra2 = (const unaligned_uint16_t *)ra1;
    // ff 25       jmpq *symbol@DYLDMAGIC(%rip)
    if (*ra2 != 0x25ff) {
        return false;
    }
#endif
    ra1 += 6l + (long)*(const unaligned_int32_t *)(ra1 + 2);
    sym = (const void **)ra1;
    
    // 如果接下来不是 objc_retainAutoreleasedReturnValue 
    // 指令且不是 objc_unsafeClaimAutoreleasedReturnValue 指令，则表示不能进行优化，
    // 那么 rootAutorelease 函数就不能执行 return (id)this; 
    // 而是要执行 return rootAutorelease2(); 把对象放入自动释放池。
    // 接下来的几个平台的实在看不懂...😭
    if (*sym != objc_retainAutoreleasedReturnValue  &&  
        *sym != objc_unsafeClaimAutoreleasedReturnValue) 
    {
        return false;
    }

    return true;
}

// __x86_64__
# elif __arm__

static ALWAYS_INLINE bool 
callerAcceptsOptimizedReturn(const void *ra)
{
    // if the low bit is set, we're returning to thumb mode
    if ((uintptr_t)ra & 1) {
        // 3f 46          mov r7, r7
        // we mask off the low bit via subtraction
        // 16-bit instructions are well-aligned
        if (*(uint16_t *)((uint8_t *)ra - 1) == 0x463f) {
            return true;
        }
    } else {
        // 07 70 a0 e1    mov r7, r7
        // 32-bit instructions may be only 16-bit aligned
        if (*(unaligned_uint32_t *)ra == 0xe1a07007) {
            return true;
        }
    }
    return false;
}

// __arm__
# elif __arm64__

static ALWAYS_INLINE bool 
callerAcceptsOptimizedReturn(const void *ra)
{
    // fd 03 1d aa    mov fp, fp
    // arm64 instructions are well-aligned
    if (*(uint32_t *)ra == 0xaa1d03fd) {
        return true;
    }
    return false;
}

// __arm64__
# elif __i386__

static ALWAYS_INLINE bool 
callerAcceptsOptimizedReturn(const void *ra)
{
    // 89 ed    movl %ebp, %ebp
    if (*(unaligned_uint16_t *)ra == 0xed89) {
        return true;
    }
    return false;
}

// __i386__
# else

#warning unknown architecture

static ALWAYS_INLINE bool 
callerAcceptsOptimizedReturn(const void *ra)
{
    return false;
}

// unknown architecture
# endif
```

### prepareOptimizedReturn
```c++
// Try to prepare for optimized return with the given disposition (+0 or +1).
// 根据给定的 disposition（+0 或 +1）尝试准备优化返回值。

// Returns true if the optimized path is successful.
// 如果这条优化的路径是成功的则返回 true。

// Otherwise the return value must be retained and/or autoreleased as usual.
// 否则，返回值必须照常 retain 和/或 autorelease。

static ALWAYS_INLINE bool 
prepareOptimizedReturn(ReturnDisposition disposition)
{
    // 这里从 tls 中取得 RETURN_DISPOSITION_KEY 的值必须是 false(ReturnAtPlus0)，
    // 否则执行断言
    ASSERT(getReturnDisposition() == ReturnAtPlus0);
    
    // callerAcceptsOptimizedReturn 上面👆已经详细分析
    // __builtin_return_address(0) 上面👆已经详细分析
    if (callerAcceptsOptimizedReturn(__builtin_return_address(0))) {
        
        // 如果 disposition 是 true (ReturnAtPlus1 +1) 则保存在线程的存储空间内
        if (disposition) setReturnDisposition(disposition);
        
        // 返回 true
        return true;
    }

    return false;
}
```
&emsp;至此 `rootAutorelease` 函数已经看完了，`rootAutorelease` 函数分为两个分支，当对象能进行优化时，就直接返回，不能进行优化时就正常放进自动释放池。下面我们查找上面提及的 `objc_autoreleaseReturnValue` 等函数的调用时机。首先我们先看一下它们在 `NSObject.mm` 中的定义。

## objc_retainAutoreleaseAndReturn
```c++
// Same as objc_retainAutorelease but suitable for tail-calling 
// if you don't want to push a frame before this point.
// 与 objc_retainAutorelease 相同但适用于尾部调用，
// if you don't want to push a frame before this point

__attribute__((noinline))
static id 
objc_retainAutoreleaseAndReturn(id obj)
{
    return objc_retainAutorelease(obj);
}
```
### objc_retainAutorelease
```c++
id
objc_retainAutorelease(id obj)
{
    // 这种操作可以理解为延迟对象释放，先 retain 然后放进释放池，
    // 当池 pop 时再执行 release 保证 obj 正常销毁不会导致内存泄露
    return objc_autorelease(objc_retain(obj));
}

```
## objc_autoreleaseReturnValue
```c++
// Prepare a value at +1 for return through a +0 autoreleasing convention.
// 准备 +1 处的值，以通过 +0 自动释放约定返回。
id 
objc_autoreleaseReturnValue(id obj)
{
    // 同上面的 rootAutorelease 函数
    if (prepareOptimizedReturn(ReturnAtPlus1)) return obj;
    
    // 一路判断最终依然调用 rootAutorelease2 函数
    return objc_autorelease(obj);
}
```
## objc_retainAutoreleaseReturnValue
```c++
// Prepare a value at +0 for return through a +0 autoreleasing convention.
// 准备一个 +0 的值以通过 +0 自动释放约定返回
id 
objc_retainAutoreleaseReturnValue(id obj)
{
    // 同上面的 prepareOptimizedReturn 函数调用，此处入参是 ReturnAtPlus0
    if (prepareOptimizedReturn(ReturnAtPlus0)) return obj;

    // not objc_autoreleaseReturnValue(objc_retain(obj)) 
    // because we don't need another optimization attempt
    // 没有 objc_autoreleaseReturnValue(objc_retain(obj)) 
    // 因为我们不需要其他优化尝试
    
    // 这种操作可以理解为延迟对象释放，先 retain 然后放进释放池，
    // 当池 pop 时再执行 release 保证 obj 正常销毁不会导致内存泄露
    return objc_retainAutoreleaseAndReturn(obj);
}
```
## objc_retainAutoreleasedReturnValue
```c++
// Accept a value returned through a +0
// autoreleasing convention for use at +1.
// 接受通过 +0 自动释放约定返回的值，以用于 +1
id
objc_retainAutoreleasedReturnValue(id obj)
{
    // 从 tls 中取出 ReturnDisposition
    if (acceptOptimizedReturn() == ReturnAtPlus1) return obj;

    // 对 obj 执行 retain 操作
    return objc_retain(obj);
}
```
### acceptOptimizedReturn
```c++
// Try to accept an optimized return.
// 尝试接受优化的返回值。

// Returns the disposition of the returned object (+0 or +1).
// 返回返回值的优化设置（+0 或 +1）。

// An un-optimized return is +0.
// 一个未优化的返回值返回 +0。

static ALWAYS_INLINE ReturnDisposition 
acceptOptimizedReturn()
{
    // 从 tls 中根据 RETURN_DISPOSITION_KEY 取得优化设置 
    ReturnDisposition disposition = getReturnDisposition();
    
    // reset to the unoptimized state
    // 重置为未优化状态
    setReturnDisposition(ReturnAtPlus0); 
    
    // 返回优化设置 （+0 或 +1）
    return disposition;
}
```
## objc_unsafeClaimAutoreleasedReturnValue
```c++
// Accept a value returned through a +0
// autoreleasing convention for use at +0.
// 接受通过 +0 自动释放约定返回的值，以用于 +0。
id
objc_unsafeClaimAutoreleasedReturnValue(id obj)
{
    // 如果是 ReturnAtPlus0 直接返回 obj
    if (acceptOptimizedReturn() == ReturnAtPlus0) return obj;

    return objc_releaseAndReturn(obj);
}
```
### objc_releaseAndReturn
```c++
// Same as objc_release but suitable for tail-calling 
// 与 objc_release 相同，但适用于尾部调用。

// if you need the value back and don't want
// to push a frame before this point.
// 如果您需要返回值，don't want to push a frame before this point

__attribute__((noinline))
static id 
objc_releaseAndReturn(id obj)
{
    // 执行 release，这里如果执行完 release 后 obj dealloc 了怎么办 ？
    objc_release(obj);
    
    // 返回 obj 
    return obj;
}
```
## objc_retainAutorelease/_objc_deallocOnMainThreadHelper...
```c++
id
objc_retainAutorelease(id obj)
{
    // 这种操作可以理解为延迟对象释放，先 retain 然后放进释放池，
    // 当池 pop 时再执行 release 保证 obj 正常销毁不会导致内存泄露
    return objc_autorelease(objc_retain(obj));
}

// dispatch_barrier_async_f(dispatch_get_main_queue(),
// self, _objc_deallocOnMainThreadHelper); 
void
_objc_deallocOnMainThreadHelper(void *context)
{
    id obj = (id)context;
    [obj dealloc];
}

// typedef const void* objc_objectptr_t; 只是一个 void* 类型

// convert objc_objectptr_t to id, callee must take ownership.
// 被调用方必须拥有所有权
id objc_retainedObject(objc_objectptr_t pointer) { return (id)pointer; }

// convert objc_objectptr_t to id, without ownership transfer.
// 没有所有权转让
id objc_unretainedObject(objc_objectptr_t pointer) { return (id)pointer; }

// convert id to objc_objectptr_t, no ownership transfer.
// 没有所有权转让
objc_objectptr_t objc_unretainedPointer(id object) { return object; }

// 在 map_images_nolock 函数内被调用
void arr_init(void) 
{
    // 自动释放池初始化
    AutoreleasePoolPage::init();
    
    // SideTablesMap 初始化
    SideTablesMap.init();
    
    // AssociationsManager 初始化
    _objc_associations_init();
}
```

## 验证结论
&emsp;何时函数返回值会被放进自动释放池？
```c++
// 准备一个 NSObject 的分类用于在 ARC 下查看对象的引用计数
// NSObject+Custom.m 文件，需要在 Compile Sources 中把
// NSObject+Custom.m Compiler Flags 置为 -fno-objc-arc
// NSObject+Custom.m 文件实现如下：（.h 仅包含 customRetainCount 声明）

#import "NSObject+Custom.h"
@implementation NSObject (Custom)
- (NSUInteger)customRetainCount {
    return self.retainCount;
}
@end
```
```c++
// 把 NSObject+Custom.h 引入 ViewController 中
// 把 _objc_autoreleasePoolPrint 如下声明放在 ViewController.m 文件顶部
OBJC_EXPORT void
_objc_autoreleasePoolPrint(void)
    OBJC_AVAILABLE(10.7, 5.0, 9.0, 1.0, 2.0);
    
// 在 viewDidLoad 中编写如如下函数
NSMutableArray *array = [NSMutableArray array];
NSLog(@"array: %p %ld", array, array.customRetainCount);
// 打印自动释放池内容
_objc_autoreleasePoolPrint();
```
&emsp;`ViewController` 标记为 `MRC` ：`array` `retainCount` 为 1，且根据 `array` 地址能在自动释放池里面找到 `array`。

&emsp;`ViewController` 标记为 `ARC` ：`array` `retainCount` 为 1，根据 `array` 地址不能在自动释放池里面找到 `array`。

&emsp;由于看不到 `[NSMutableArray array]` 内部实现，且重写 `NSMutableArray` 的 `dealloc` 会有很多干扰，所以整体看起来显得不够直观，那我们来定义一个自己的类来看。
```c++
// LGPerson.h 
#import <Foundation/Foundation.h>
NS_ASSUME_NONNULL_BEGIN
@interface LGPerson : NSObject
+ (LGPerson *)returnInstanceValue;
@end
NS_ASSUME_NONNULL_END

// LGPerson.m
#import "LGPerson.h"
#import "NSObject+Custom.h"
@implementation LGPerson
+ (LGPerson *)returnInstanceValue {
    LGPerson *temp = [[LGPerson alloc] init];
    NSLog(@"%@ %p %ld", NSStringFromSelector(_cmd), temp, temp.customRetainCount);
    return temp;
}
- (void)dealloc {
    NSLog(@"🍀🍀🍀 %@ LGPerson dealloc", self);
}
@end

// ViewControlle viewDidLoad 函数 中:
LGPerson *person = [LGPerson returnInstanceValue];
NSLog(@"person: %p %ld", person, person.customRetainCount);
_objc_autoreleasePoolPrint();
```
&emsp;`ViewController` 标记为 `MRC`，`LGPerson` 为 `ARC` 下打印结果:
```c++
returnInstanceValue 0x6000035405f0 1
person: 0x6000035405f0 1
objc[8620]: ##############
objc[8620]: AUTORELEASE POOLS for thread 0x10eb45dc0
objc[8620]: 572 releases pending.
objc[8620]: [0x7fa59d804000]  ................  PAGE (full)  (cold)
objc[8620]: [0x7fa59d804038]  ################  POOL 0x7fa59d804038
objc[8620]: [0x7fa59d804040]  ################  POOL 0x7fa59d804040
objc[8620]: [0x7fa59d804048]  ################  POOL 0x7fa59d804048
objc[8620]: [0x7fa59d804050]    0x600001949c80  UIApplicationSceneSettings
...
objc[8620]: [0x7fa59d037248]    0x6000035405f0  LGPerson // 在释放池的末尾能看到 person
objc[8620]: ##############
🍀🍀🍀 <LGPerson: 0x6000035405f0> LGPerson dealloc // dealloc 函数正常调用
```
&emsp;`ViewController` `LGPerson` 均为 `ARC` 下打印结果:
```c++
returnInstanceValue 0x600001b6afb0 1
person: 0x600001b6afb0 1 // 引用计数为 1
// AutoreleasePool 里面没有 person
🍀🍀🍀 <LGPerson: 0x600001b6afb0> LGPerson dealloc // dealloc 函数正常调用
```
&emsp;还有一种情况，我们把 `LGPerson` 修改如下，并在 `Compile Sources` 中把 `LGPerson.m` `Compiler Flags` 置为 `-fno-objc-arc`:
```c++
+ (LGPerson *)returnInstanceValue {
    LGPerson *temp = [[[LGPerson alloc] init] autorelease];
    NSLog(@"%@ %p %ld", NSStringFromSelector(_cmd), temp, temp.customRetainCount);
    return temp;
}

- (void)dealloc {
    [super dealloc];
    NSLog(@"🍀🍀🍀 LGPerson dealloc");
}
```
&emsp;`ViewController` `LGPerson` 均标记为 `MRC` 下打印结果:
```c++
returnInstanceValue 0x6000014266a0 1
person: 0x6000014266a0 1
objc[8741]: ##############
objc[8741]: AUTORELEASE POOLS for thread 0x10bbdddc0
objc[8741]: 583 releases pending.
objc[8741]: [0x7f9569804000]  ................  PAGE (full)  (cold)
objc[8741]: [0x7f9569804038]  ################  POOL 0x7f9569804038
objc[8741]: [0x7f9569804040]  ################  POOL 0x7f9569804040
objc[8741]: [0x7f9569804048]  ################  POOL 0x7f9569804048
objc[8741]: [0x7f9569804050]    0x60000383d900  UIApplicationSceneSettings
...
objc[8741]: [0x7f9569024298]    0x6000014266a0  LGPerson // person 放在自动释放池里
objc[8741]: [0x7f95690242a0]    0x600001669480  __NSCFString
objc[8741]: ##############
🍀🍀🍀 LGPerson dealloc // dealloc 函数正常调用
```
&emsp;看到 `LGPerson` 采用 `ARC` 时和 `MRC` 时要保持结果相同的话需要对 `temp` 调用 `autorelease` 函数。为了对比在 `MRC` 下对 `temp` 不执行 `autorelease` 的话，打印结果只有 `returnInstanceValue 0x600001470510 1` `person: 0x600001470510 1`，自动释放池里没有 `person` 且 `person` 没有执行 `dealloc`，内存泄漏了，需要我们在 `viewDidLoad` 中主动调用 `[person release]` 或 `[person autorelease]` 才能正确释放内存。

&emsp;`ARC` 的情况下 `person` 没有被放入自动释放池，`returnInstanceValue` 函数返回的对象还是正常使用了，那么表示 `temp` 对象在出了 `returnInstanceValue` 函数的右边花括号时的 `release` 操作并没有导致 `temp` 对象被释放销毁。那么这时为什么呢？那么我们进一步来看一些汇编代码。

&emsp;先抛出结论，对比 `ViewController` 标记为 `MRC`，`LGPerson` 标记为 `ARC` 和 `ViewController` `LGPerson` 均为 `ARC` 的两种情况。看到同样的 `LGPerson` 函数，当在 `MRC` 中调用时，函数返回的对象被放入了自动释放池。即当在 `MRC` 中调用 `ARC` 的函数时，函数返回值会被放入自动释放池，而在 `ARC` 调用时，函数返回对象正常使用，但是它没有被放入自动释放池，那程序是怎么来维护函数返回值的引用计数的呢 ？

### objc_autoreleaseReturnValue 调用时机
&emsp;用两种方式找到 `objc_autoreleaseReturnValue` 的调用时机：
```c++
+ (LGPerson *)returnInstanceValue {
    return [[LGPerson alloc] init]; // ⬅️ 在这行打断点，并勾选 
                                    // Debug -> Debug Workflow -> Always Show Disassembly
}
```
&emsp;执行程序可看到如下汇编：
```c++
Simple_iOS`+[LGPerson returnInstanceValue]:
    0x10c43af50 <+0>:  pushq  %rbp
    0x10c43af51 <+1>:  movq   %rsp, %rbp
    0x10c43af54 <+4>:  subq   $0x20, %rsp
    0x10c43af58 <+8>:  movq   %rdi, -0x8(%rbp)
    0x10c43af5c <+12>: movq   %rsi, -0x10(%rbp)
->  0x10c43af60 <+16>: movq   0x46f1(%rip), %rdi        ; (void *)0x000000010c43f788: LGPerson
    0x10c43af67 <+23>: callq  0x10c43b30c               ; symbol stub for: objc_alloc
    0x10c43af6c <+28>: movq   0x46d5(%rip), %rsi        ; "init"
    0x10c43af73 <+35>: movq   0x2086(%rip), %rdi        ; (void *)0x00007fff513f7780: objc_msgSend
    0x10c43af7a <+42>: movq   %rdi, -0x18(%rbp)
    0x10c43af7e <+46>: movq   %rax, %rdi
    0x10c43af81 <+49>: movq   -0x18(%rbp), %rax
    0x10c43af85 <+53>: callq  *%rax
    0x10c43af87 <+55>: movq   %rax, %rdi
    0x10c43af8a <+58>: addq   $0x20, %rsp
    0x10c43af8e <+62>: popq   %rbp
    0x10c43af8f <+63>: jmp    0x10c43b31e               ; symbol stub for: objc_autoreleaseReturnValue 
                                                        // 在函数末尾调用了 objc_autoreleaseReturnValue 函数
```
&emsp;第二种方式在终端执行: `clang -S -fobjc-arc LGPerson.m -o LGPerson.s` 指令，在当前文件夹下生成 `LGPerson.s` 文件，双击打开能看到 `returnInstanceValue` 函数被转成如下汇编代码：
```c++
    .p2align    4, 0x90         ## -- Begin function +[LGPerson returnInstanceValue]
"+[LGPerson returnInstanceValue]":      ## @"\01+[LGPerson returnInstanceValue]"
    .cfi_startproc
## %bb.0:
    pushq    %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq    %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq    $32, %rsp
    movq    %rdi, -8(%rbp)
    movq    %rsi, -16(%rbp)
    movq    L_OBJC_CLASSLIST_REFERENCES_$_(%rip), %rdi
    callq    _objc_alloc
    movq    L_OBJC_SELECTOR_REFERENCES_(%rip), %rsi
    movq    _objc_msgSend@GOTPCREL(%rip), %rdi
    movq    %rdi, -24(%rbp)         ## 8-byte Spill
    movq    %rax, %rdi
    movq    -24(%rbp), %rax         ## 8-byte Reload
    callq    *%rax
    movq    %rax, %rdi
    addq    $32, %rsp
    popq    %rbp
    jmp    _objc_autoreleaseReturnValue ## TAILCALL
    .cfi_endproc
                                        ## -- End function
```
&emsp;同样也在结尾处调用了 `_objc_autoreleaseReturnValue` 函数。为了对比我们使用 `clang -S -fno-objc-arc LGPerson.m -o LGPerson.s` 指令看下 `MRC` 下 `returnInstanceValue` 转换:
```c++
    .p2align    4, 0x90         ## -- Begin function +[LGPerson returnInstanceValue]
"+[LGPerson returnInstanceValue]":      ## @"\01+[LGPerson returnInstanceValue]"
    .cfi_startproc
## %bb.0:
    pushq    %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq    %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq    $16, %rsp
    movq    %rdi, -8(%rbp)
    movq    %rsi, -16(%rbp)
    movq    L_OBJC_CLASSLIST_REFERENCES_$_(%rip), %rsi
    movq    %rsi, %rdi
    callq    _objc_alloc_init
    addq    $16, %rsp
    popq    %rbp
    retq
    .cfi_endproc
                                        ## -- End function
```
&emsp;可发现在结尾处并不会调用 `_objc_autoreleaseReturnValue` 函数。看来在 `ARC` 下，当函数末尾返回函数返回值时，编译器会为我们插入一条 `_objc_autoreleaseReturnValue` 函数。前面我们已经详细分析过 `id objc_autoreleaseReturnValue(id obj) { ... }` 函数的实现，它有两个分支：

1. 符合优化返回值的条件把 `ReturnAtPlus1` 优化策略保存在 `tls` 中，然后直接返回 `objc`。（对应上面 `ViewController` 和 `LGPerson` 都是 `ARC` 时在 `viewDidLoad` 函数中调用 `returnInstanceValue` 函数并获得返回值，返回值并没有被放入自动释放池）
2. 不符合优化条件，直接执行 `objc_autorelease(obj)` 把 `obj` 放入自动释放池中。（对应上面 `ViewController` 处于 `MRC` 模式，然后 `LGPerson` 处于 `ARC` 模式，在 `viewDidLoad` 函数中调用 `returnInstanceValue` 函数，返回值会被放进自动释放池，那么它的 `retain/release` 就和我们熟知的 `MRC` 模式是一模一样的）

&emsp;这里我们只关注状况 1，这里我们还依稀记得在 `prepareOptimizedReturn` 函数中，判断是否可以对返回值进行优化的时候判断的条件是(x86_64) : 接下来主调函数是否调用 `objc_retainAutoreleasedReturnValue` 或 `objc_unsafeClaimAutoreleasedReturnValue` 函数，那么这两个函数的调用时机是什么时候呢？我们知道刚刚我们的 `returnInstanceValue` 函数调用完毕是回到了 `viewDidLoad` 函数，那么我们就汇编 `viewDidLoad` 函数。
```c++
// LGPerson.m 中 
+ (LGPerson *)returnInstanceValue {
    
    NSLog(@"🍎🍎🍎 %p", __builtin_return_address(0));
    
    return [[LGPerson alloc] init];
}

// viewDidLoad 中：
- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    LGPerson *person = [LGPerson returnInstanceValue];
    NSLog(@"1: %@", person); // ⬅️ 在这一行打断点
}

// 控制台打印:
🍎🍎🍎 0x107d72aca // 这是 returnInstanceValue 函数的返回值，我们可在汇编代码中找到它...
```
&emsp;可看到如下汇编输出:
```c++
...
0x107d72ab3 <+51>:  movq   0x7bfe(%rip), %rax        ; (void *)0x0000000107d7a898: LGPerson
0x107d72aba <+58>:  movq   0x7bcf(%rip), %rsi        ; "returnInstanceValue"
0x107d72ac1 <+65>:  movq   %rax, %rdi
0x107d72ac4 <+68>:  callq  *0x253e(%rip)             ; (void *)0x00007fff20173880: objc_msgSend

// 🍎🍎🍎 0x107d72aca 👈👈👈 这里，下面正是 objc_retainAutoreleasedReturnValue 函数，
// 刚好满足优化返回值的条件，不用把 obj 放入自动释放池了
0x107d72aca <+74>:  movq   %rax, %rdi 
0x107d72acd <+77>:  callq  0x107d732e8               ; symbol stub for: objc_retainAutoreleasedReturnValue // 调用 objc_retainAutoreleasedReturnValue 函数
0x107d72ad2 <+82>:  leaq   0x2547(%rip), %rcx        ; @"1: %@"
0x107d72ad9 <+89>:  movq   %rax, -0x28(%rbp)
->  0x107d72add <+93>:  movq   -0x28(%rbp), %rsi
0x107d72ae1 <+97>:  movq   %rcx, %rdi
0x107d72ae4 <+100>: movb   $0x0, %al
0x107d72ae6 <+102>: callq  0x107d7329a               ; symbol stub for: NSLog
...
```
&emsp;在分析 `objc_retainAutoreleasedReturnValue` 函数之前我们看下当 `ViewController` 是 `MRC` 模式时会有什么样汇编转换。
```c++
// 把 ViewController Compiler Flags 设置为 -fno-objc-arc

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    LGPerson *person = [LGPerson returnInstanceValue];
    NSLog(@"1: %@", person); // ⬅️ 在这行打断点
    NSLog(@"2: %@", person);
    NSLog(@"3: %@", person);
    NSLog(@"4: %@", person);
}

// 不打断点直接运行的话看到控制台如下打印:
🍎🍎🍎 0x10b749aea
1: <LGPerson: 0x600003e181f0>
2: <LGPerson: 0x600003e181f0>
3: <LGPerson: 0x600003e181f0>
4: <LGPerson: 0x600003e181f0>
🍀🍀🍀 LGPerson dealloc // 这里的 dealloc 调用，
                        // 足以证明 MRC 时函数返回值我们手动调用 autorelease 

// 打断点时看汇编输出:
🍎🍎🍎 0x102c4daea
...
0x102c4dad3 <+51>:  movq   0x7b9e(%rip), %rax        ; (void *)0x0000000102c55850: LGPerson
0x102c4dada <+58>:  movq   0x7b6f(%rip), %rsi        ; "returnInstanceValue"
0x102c4dae1 <+65>:  movq   %rax, %rdi
0x102c4dae4 <+68>:  callq  *0x251e(%rip)             ; (void *)0x00007fff20173880: objc_msgSend
0x102c4daea <+74>:  leaq   0x252f(%rip), %rcx        ; @"1: %@" // 🍎🍎🍎 0x102c4daea

// 后续也没有看到 objc_retainAutoreleasedReturnValue 函数的调用。
0x102c4daf1 <+81>:  movq   %rax, -0x28(%rbp) 
->  0x102c4daf5 <+85>:  movq   -0x28(%rbp), %rsi
...
```
&emsp;不打断点时最后的 `dealloc` 调用，足以证明 `MRC` 时函数返回值需要我们手动调用 `autorelease` , `ARC` 时编译器帮我们调用，把函数返回值放进了自动释放池。

&emsp;这里我们回顾 `objc_retainAutoreleasedReturnValue`  函数的过程，它也有两个分支：
1. `if (acceptOptimizedReturn() == ReturnAtPlus1) return obj;` 对应上面的 `_objc_autoreleaseReturnValue` 正是把 `ReturnAtPlus1` 保存在线程的存储空间内，这里从线程中取到，所以这里是直接把 `obj` 返回了。（那么 `obj` 是保存在哪里了呢？别的文章都说是保存在 `tls` 中，自函数返回值没有放入自动释放池后，就被放在了 `tls` 中，然后在主调函数中再从 `tls` 中读取，但是目前我还是无法找到怎么证明的...唯一可看到的是当我们用一个 `strong` 的属性接收函数返回值时，然后后面每次用这个属性都会调用 `objc_retainAutoreleasedReturnValue` 函数，这里难道时每次都从 `tls` 中读值吗？）
```c++
@property (nonatomic, strong) LGPerson *person;
@property (nonatomic, strong) LGPerson *p2;

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    self.person = [LGPerson returnInstanceValue];
    self.p2 = self.person; // 调用 objc_retainAutoreleasedReturnValue
    
    NSLog(@"1: %@", self.person); // 调用 objc_retainAutoreleasedReturnValue
    NSLog(@"2: %@", self.person); // 调用 objc_retainAutoreleasedReturnValue
    NSLog(@"3: %@", self.person); // 调用 objc_retainAutoreleasedReturnValue
    NSLog(@"4: %@", self.person); // 调用 objc_retainAutoreleasedReturnValue
}
```

2. 另外就是 `return objc_retain(obj);` 此种情况对应的是: `LGPerson` 是 `MRC` 模式，函数返回值手动调用 `autorelease` 函数放进自动释放池，然后在 `ARC` 模式下的 `ViewController` 中调用 `LGPerson` 的有返回值的函数时。因为在 `ViewController` 我们不想变量在自动释放池 `pop` 时把对象释放了，那么需要对对象执行 `retain` 操作，正是编译器为我们做了。
```c++
// LGPerson MRC 模式下:
+ (LGPerson *)returnInstanceValue {
    NSLog(@"🍎🍎🍎 %p", __builtin_return_address(0));
    return [[[LGPerson alloc] init] autorelease];
}

// ViewController ARC 模式下:
- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    // 这里会调用 objc_retainAutoreleasedReturnValue 函数并走第 2 分支，对函数返回值执行 objc_retain 操作。
    LGPerson *person = [LGPerson returnInstanceValue];  
    
    NSLog(@"1: %@", person); // ⬅️ 在这里打断点
    NSLog(@"2: %@", person);
    NSLog(@"3: %@", person);
    NSLog(@"4: %@", person);
} // 这里出了右边大括号，会对 person 这个临时变量做一次 release 操作

// 控制台打印如下：
🍎🍎🍎 0x1064eaaba
1: <LGPerson: 0x6000011880a0>
2: <LGPerson: 0x6000011880a0>
3: <LGPerson: 0x6000011880a0>
4: <LGPerson: 0x6000011880a0>
🍀🍀🍀 LGPerson dealloc // 对象正常释放

// 汇编输出
...
0x108791aa3 <+51>:  movq   0x7bd6(%rip), %rax        ; (void *)0x0000000108799858: LGPerson
0x108791aaa <+58>:  movq   0x7ba7(%rip), %rsi        ; "returnInstanceValue"
0x108791ab1 <+65>:  movq   %rax, %rdi
0x108791ab4 <+68>:  callq  *0x254e(%rip)             ; (void *)0x00007fff20173880: objc_msgSend
0x108791aba <+74>:  movq   %rax, %rdi
0x108791abd <+77>:  callq  0x1087922de               ; symbol stub for: objc_retainAutoreleasedReturnValue // 对函数返回值调用 objc_retain
0x108791ac2 <+82>:  leaq   0x2557(%rip), %rcx        ; @"1: %@"
0x108791ac9 <+89>:  movq   %rax, -0x28(%rbp)
->  0x108791acd <+93>:  movq   -0x28(%rbp), %rsi
0x108791ad1 <+97>:  movq   %rcx, %rdi
0x108791ad4 <+100>: movb   $0x0, %al
0x108791ad6 <+102>: callq  0x10879228a               ; symbol stub for: NSLog
...
```
&emsp;到这里我们的 `objc_autoreleaseReturnValue` 和 `objc_retainAutoreleasedReturnValue` 函数就看完了。

&emsp;至此 `objc_object` 的代码就全部看完了。已经记不清花费了多少时间，但是整体对 `objc_object` 已经有了一个全面的认知，花多少时间都是超值的。⛽️⛽️

## 参考链接
**参考链接:🔗**
+ [黑幕背后的Autorelease](http://blog.sunnyxx.com/2014/10/15/behind-autorelease/)
+ [__builtin_return_address(LEVEL)](https://blog.csdn.net/dayancn/article/details/18899157)
+ [返回值的 Autorelease 和 编译器优化](https://www.jianshu.com/p/aae7c3bd2191)
+ [objc_autoreleaseReturnValue和objc_retainAutoreleasedReturnValue函数对ARC的优化](https://www.jianshu.com/p/2f05060fa377?spm=a2c4e.11153940.blogcont690541.7.575f3c89jeD8IS)
+ [ios – 为什么执行ARC的objc_autoreleaseReturnValue对于x86_64和ARM有所不同？](http://www.voidcn.com/article/p-npbahspk-bsx.html)
+ [https://zhuanlan.zhihu.com/p/161291275](https://zhuanlan.zhihu.com/p/161291275)
+ [操作系统内存管理(思维导图详解)](https://blog.csdn.net/hguisu/article/details/5713164)
