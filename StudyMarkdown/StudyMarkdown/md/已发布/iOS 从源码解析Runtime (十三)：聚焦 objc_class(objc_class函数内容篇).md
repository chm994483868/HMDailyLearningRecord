# iOS 从源码解析Runtime (十三)：聚焦 objc_class(objc_class函数内容篇)

> 前面已经忘记看了多少天了，终于把 `objc_object` 和 `objc_class` 的相关的数据结构都看完了，现在只剩下 `objc_class` 中定义的函数没有分析，那么就由本篇开始分析吧！⛽️⛽️

## objc_class 函数列表

### class_rw_t *data() const
&emsp;`data` 函数是直接调用的 `class_data_bits_t bits` 的 `data` 函数，内部实现的话也很简单，通过掩码 `#define FAST_DATA_MASK 0x00007ffffffffff8UL`（二进制第 `3`-`46` 位是 `1`，其他位都是 `0`） 从 `class_data_bits_t bits` 的 `uintptr_t bits`（`uintptr_t bits` 是 `struct class_data_bits_t` 仅有的一个成员变量）中取得 `class_rw_t` 指针。
```c++
class_rw_t *data() const {
    return bits.data();
}

// class_data_bits_t 中：
class_rw_t* data() const {
    return (class_rw_t *)(bits & FAST_DATA_MASK);
}
```
### void setData(class_rw_t *newData)
&emsp;同样 `setData` 函数调用的也是 `class_data_bits_t bits` 的 `setData` 函数，在 `struct class_data_bits_t` 的 `setData` 函数中， `uintptr_t bits` 首先和 `~FAST_DATA_MASK` 做与操作把掩码位置为 `0`，其它位保持不变，然后再和 `newData` 做或操作，把 `newData` 中值为 `1` 的位也设置到 `uintptr_t bits` 中。
```c++
void setData(class_rw_t *newData) {
    bits.setData(newData);
}
```
### void setInfo(uint32_t set)
&emsp;`setInfo` 函数调用的是 `class_rw_t` 的 `setFlags` 函数，通过原子的或操作把 `set` 中值为 `1` 的位也设置到 `class_rw_t` 的 `uint32_t flags` 中。（`flags` 和 `set` 都是 `uint32_t` 类型，都是 `32` 位。）
```c++
void setInfo(uint32_t set) {
    ASSERT(isFuture()  ||  isRealized());
    data()->setFlags(set);
}
```
### void clearInfo(uint32_t clear)
&emsp;`clearInfo` 函数调用的是 `class_rw_t` 的 `clearFlags` 函数，通过原子的与操作把 `~clear` 中值为 `0` 的位也设置到 `class_rw_t` 的 `uint32_t flags` 中。（`flags` 和 `clear` 都是 `uint32_t`  类型，都是 `32` 位。）
```c++
void clearInfo(uint32_t clear) {
    ASSERT(isFuture()  ||  isRealized());
    data()->clearFlags(clear);
}
```
### void changeInfo(uint32_t set, uint32_t clear)
&emsp;`changeInfo` 函数调用的同样也是 `class_rw_t` 的 `changeFlags` 函数，先取得 `class_rw_t` 的 `uint32_t flags` 赋值到一个临时变量 `uint32_t oldf` 中，然后 `oldf` 和 `uint32_t set` 做一个或操作，把 `set` 值为 `1` 的位也都设置到 `oldf` 中，然后再和 `~clear` 做与操作，把 `~clear` 中是 `0` 的位也都设置其中，并把 `!OSAtomicCompareAndSwap32Barrier(oldf, newf, (volatile int32_t *)&flags)` 作为 `do while` 循环的循环条件，保证 `uint32_t oldf` 的值都能写入 `uint32_t flags` 中。
```c++
// set and clear must not overlap
void changeInfo(uint32_t set, uint32_t clear) {
    ASSERT(isFuture()  ||  isRealized());
    ASSERT((set & clear) == 0);
    
    data()->changeFlags(set, clear);
}
```
### FAST_HAS_DEFAULT_RR/RW_HAS_DEFAULT_RR
&emsp;`FAST_HAS_DEFAULT_RR` 用以在 `__LP64__` 平台下判断 `objc_class` 的 `class_data_bits_t bits` 第二位的值是否为 `1`，以此表示该类或者父类是否有如下函数的默认实现。对应在 `非 __LP64` 平台下，则是使用 `RW_HAS_DEFAULT_RR`，且判断的位置发生了变化，`RW_HAS_DEFAULT_RR` 用以判断从 `objc_class` 的 `class_data_bits_t bits` 中取的得 `class_rw_t` 指针指向的 `class_rw_t` 实例的 `uint32_t flags` 的第 `14` 位的值是否为 `1`，以此表示该类或者父类是否有如下函数的默认实现：  

+ `retain/release/autorelease/retainCount`
+ `_tryRetain/_isDeallocating/retainWeakReference/allowsWeakReference`

```c++
// Values for class_rw_t->flags (RW_*), cache_t->_flags (FAST_CACHE_*), 
// or class_t->bits (FAST_*).
// 当从 class_rw_t->flags 取值时，使用 RW_* 做前缀，
// 当从 cache_t->_flags 取值时，使用 FAST_CACHE_* 做前缀，
// 当从 class_data_bits_t->bits 取值时，使用 FAST_* 做前缀。 

// FAST_* and FAST_CACHE_* are stored on the class, 
// reducing pointer indirection.
// FAST_* 和 FAST_CACHE_* 前缀开头的值，
// 分别保存在 objc_class 的 class_data_bits_t bits 和 cache_t cache 两个成员变量中，直接减少了指针间接寻值，
// RW_* 前缀开头的值首先要从 class_data_bits_t bits 中找到 class_rw_t 的指针，然后根据指针再去寻 class_rw_t 的值。

#if __LP64__
...
// class or superclass has default retain/release/autorelease/retainCount/
//   _tryRetain/_isDeallocating/retainWeakReference/allowsWeakReference
// 类或者其父类
// 在 class_data_bits_t bits 的 uintptr_t bits 中判断。

#define FAST_HAS_DEFAULT_RR     (1UL<<2)
...
#else
...
// class or superclass has default retain/release/autorelease/retainCount/
//   _tryRetain/_isDeallocating/retainWeakReference/allowsWeakReference
// 类或者父类
// 在 class_rw_t 的 uint32_t flags 中判断。 

#define RW_HAS_DEFAULT_RR     (1<<14)
...
#endif
```
### hasCustomRR/setHasDefaultRR/setHasCustomRR
&emsp;在`__LP64__` 平台和其它平台下的判断、设置、清除 `objc_class` 的默认 `RR` 函数的标记位。
```c++
#if FAST_HAS_DEFAULT_RR

    // 直接判断 class_data_bits_t bits 的 uintptr_t bits 二进制表示的第 2 位的值是否为 1。
    bool hasCustomRR() const {
        return !bits.getBit(FAST_HAS_DEFAULT_RR);
    }
    
    // 以原子方式把 class_data_bits_t bits 的 uintptr_t bits 二进制表示的第 2 位设置为 1。
    void setHasDefaultRR() {
        bits.setBits(FAST_HAS_DEFAULT_RR);
    }
    
    // Default 和 Custom 是相反的，如果 objc_class 有 CustomRR 则没有 DefaultRR。
    // 以原子方式把 class_data_bits_t bits 的 uintptr_t bits 二进制表示的第 2 位设置为 0，
    // 表示 objc_class 有自定义的 RR 函数，
    // 即 objc_class 的 RR 函数已经被重载了。
    
    void setHasCustomRR() {
        bits.clearBits(FAST_HAS_DEFAULT_RR);
    }
#else

    // 直接判断 class_rw_t 的 flags 二进制表示的第 14 位的值是否为 1。
    bool hasCustomRR() const {
        return !(bits.data()->flags & RW_HAS_DEFAULT_RR);
    }
    
    // 以原子方式把 class_rw_t 的 uint32_t flags 二进制表示的第 14 位设置为 1。
    void setHasDefaultRR() {
        bits.data()->setFlags(RW_HAS_DEFAULT_RR);
    }
    
    // Default 和 Custom 是相反的，如果 objc_class 有 CustomRR 则没有 DefaultRR。
    // 以原子方式把 class_rw_t 的 uint32_t flags 二进制表示的第 14 位设置为 0，
    // 表示 objc_class 有自定义的 RR 函数，
    // 即 objc_class 的 RR 函数已经被重载了。
    
    void setHasCustomRR() {
        bits.data()->clearFlags(RW_HAS_DEFAULT_RR);
    }
#endif
```
### FAST_CACHE_HAS_DEFAULT_AWZ/RW_HAS_DEFAULT_AWZ
&emsp;`FAST_CACHE_HAS_DEFAULT_AWZ` 用以在 `__LP64__` 平台下判断 `objc_class` 的 `cache_t cache` 的 `uint16_t _flags` 二进制表示时第 `14` 位的值是否为 `1`，以此表示该类或者父类是否有 `alloc/allocWithZone` 函数的默认实现。（注意，这里和上面的 `RR` 不同，`RR` 是一组实例方法保存在类中，而 `alloc/allocWithZone` 是一组类方法保存在元类中。）而在 `非 __LP64__` 平台下，则是使用 `RW_HAS_DEFAULT_AWZ`，且判断的位置发生了变化，`RW_HAS_DEFAULT_AWZ` 用以判断从 `objc_class` 的 `class_data_bits_t bits` 中取得的 `class_rw_t` 指针指向的 `class_rw_t` 实例的 `uint32_t flags` 的第 `16` 位的值是否为 `1`，以此表示该类或者父类是否有 `alloc/allocWithZone` 函数的默认实现。 
```c++

// Values for class_rw_t->flags (RW_*), cache_t->_flags (FAST_CACHE_*), 
// or class_t->bits (FAST_*).
// 当从 class_rw_t->flags 取值时，使用 RW_* 做前缀，
// 当从 cache_t->_flags 取值时，使用 FAST_CACHE_* 做前缀，
// 当从 class_data_bits_t->bits 取值时，使用 FAST_* 做前缀 

// FAST_* and FAST_CACHE_* are stored on the class, 
// reducing pointer indirection.
// FAST_* 和 FAST_CACHE_* 前缀开头的值分别保存在
// objc_class 的 class_data_bits_t bits 和 cache_t cache 两个成员变量中，
// 直接减少了指针间接寻值，
// RW_* 前缀开头的值首先要从 class_data_bits_t bits 中找到 class_rw_t 的指针，
// 然后根据指针再去寻值。

#if __LP64__
...
// class or superclass has default alloc/allocWithZone: implementation.
// 类或父类有默认的 alloc/allocWithZone: 函数实现。
// Note this is is stored in the metaclass.
// 注意，alloc/allocWithZone: 都是类方法，它们都是保存在元类中的。

#define FAST_CACHE_HAS_DEFAULT_AWZ    (1<<14)
...
#else
...
// class or superclass has default alloc/allocWithZone: implementation.
// 类或父类有默认的 alloc/allocWithZone: 函数实现。
// Note this is is stored in the metaclass.
// 注意，alloc/allocWithZone: 都是类方法，它们都是保存在元类中的。

#define RW_HAS_DEFAULT_AWZ    (1<<16)
...

#endif
```
### hasCustomAWZ/setHasDefaultAWZ/setHasDefaultAWZ
&emsp;在 `__LP64__` 平台和其它平台下判断、设置、清除 `objc_class` 的默认 `AWZ` 函数的标记位。
```c++
#if FAST_CACHE_HAS_DEFAULT_AWZ
        
    // 直接判断 cache_t cache 的 uint16_t _flags 二进制表示的第 14 位的值是否为 1。
    bool hasCustomAWZ() const {
        return !cache.getBit(FAST_CACHE_HAS_DEFAULT_AWZ);
    }
    
    // 以原子方式把 cache_t cahce 的 uint16_t _flags 二进制表示的第 14 位设置为 1。
    void setHasDefaultAWZ() {
        cache.setBit(FAST_CACHE_HAS_DEFAULT_AWZ);
    }
    
    // Default 和 Custom 是相反的，如果 objc_class 有 CustomAWZ 则没有 DefaultAWZ。
    // 以原子方式把 cache_t cache 的 uint16_t _flags 二进制表示的第 14 位设置为 0，
    // 表示 objc_class 有自定义的 alloc/allocWithZone: 函数，
    // 即 objc_class 的 alloc/allocWithZone: 函数已经被重载了。
    
    void setHasCustomAWZ() {
        cache.clearBit(FAST_CACHE_HAS_DEFAULT_AWZ);
    }
#else
    // 直接判断 class_rw_t 的 flags 二进制表示的第 16 位的值是否为 1。
    bool hasCustomAWZ() const {
        return !(bits.data()->flags & RW_HAS_DEFAULT_AWZ);
    }
    
    // 以原子方式把 class_rw_t 的 uint32_t flags 二进制表示的第 16 位设置为 1。
    void setHasDefaultAWZ() {
        bits.data()->setFlags(RW_HAS_DEFAULT_AWZ);
    }
    
    // Default 和 Custom 是相反的，如果 objc_class 有 CustomAWZ 则没有 DefaultAWZ。
    // 以原子方式把 class_rw_t 的 uint32_t flags 二进制表示的第 16 位设置为 0，
    // 表示 objc_class 有自定义的 alloc/allocWithZone: 函数，
    // 即 objc_class 的 alloc/allocWithZone: 函数已经被重载了。
    
    void setHasCustomAWZ() {
        bits.data()->clearFlags(RW_HAS_DEFAULT_AWZ);
    }
#endif
```

### FAST_CACHE_HAS_DEFAULT_CORE/RW_HAS_DEFAULT_CORE
&emsp;`FAST_CACHE_HAS_DEFAULT_CORE` 用以在 `__LP64__` 平台下判断 `objc_class` 的 `cache_t cache` 的 `uint16_t _flags` 二进制表示时第 `15` 位的值是否为 `1`，以此表示该类或者父类是否有 `new/self/class/respondsToSelector/isKindOfClass` 函数的默认实现。而在 `非 __LP64__` 平台下，则是使用 `RW_HAS_DEFAULT_CORE`，且判断的位置发生了变化，`RW_HAS_DEFAULT_CORE` 用以判断从 `objc_class` 的  `class_data_bits_t bits` 中取得 `class_rw_t` 指针指向的 `class_rw_t` 实例的 `uint32_t flags` 的第 `13` 位的值是否为 `1`，以此表示该类或者父类是否有 `new/self/class/respondsToSelector/isKindOfClass` 函数的默认实现。

```c++
#if __LP64__
...
// class or superclass has default new/self/class/respondsToSelector/isKindOfClass
// 类或者父类有默认的 new/self/class/respondsToSelector/isKindOfClass

#define FAST_CACHE_HAS_DEFAULT_CORE   (1<<15)
...
#else
...
// class or superclass has default new/self/class/respondsToSelector/isKindOfClass
// 类或者父类有默认的 new/self/class/respondsToSelector/isKindOfClass

#define RW_HAS_DEFAULT_CORE   (1<<13)
...
#endif
```

### hasCustomCore/setHasDefaultCore/setHasCustomCore
&emsp;在 `__LP64__` 平台和其它平台下判断、设置、清除 `objc_class` 的默认 `Core` 函数的标记位。
```c++
#if FAST_CACHE_HAS_DEFAULT_CORE
    // 直接判断 cache_t cache 的 uint16_t _flags 二进制表示的第 15 位的值是否为 1。
    bool hasCustomCore() const {
        return !cache.getBit(FAST_CACHE_HAS_DEFAULT_CORE);
    }
    
    // 以原子方式把 cache_t cache 的 uint16_t _flags 二进制表示的第 15 位设置为 1。
    void setHasDefaultCore() {
        return cache.setBit(FAST_CACHE_HAS_DEFAULT_CORE);
    }
    
    // Default 和 Custom 是相反的，如果 objc_calss 有 CustomCore 则没有 DefaultCore。
    // 以原子方式把 cache_t cache 的 uint16_t _flags 二进制表示的第 15 位设置为 0，
    // 表示 objc_calss 有自定义的 new/self/class/respondsToSelector/isKindOfClass 函数，
    // 即 objc_class 的 new/self/class/respondsToSelector/isKindOfClass 函数已经被重载了。 
    
    void setHasCustomCore() {
        return cache.clearBit(FAST_CACHE_HAS_DEFAULT_CORE);
    }
#else
    // 直接判断 class_rw_t 的 uint32_t flags 二进制表示的第 13 位的值是否为 1。
    bool hasCustomCore() const {
        return !(bits.data()->flags & RW_HAS_DEFAULT_CORE);
    }
    
    // 以原子方式把 class_rw_t 的 uint32_t flags 二进制表示的第 13 位设置为 1。
    void setHasDefaultCore() {
        bits.data()->setFlags(RW_HAS_DEFAULT_CORE);
    }
    
    // Default 和 Custom 是相反的，如果 objc_class 有 CustomCore 则没有 DefaultCore。
    // 以原子方式把 class_rw_t 的 uint32_t flags 二进制表示的第 13 位设置为 0，
    // 表示 objc_class 有自定义的 new/self/class/respondsToSelector/isKindOfClass 函数，
    // 即 objc_class 的 new/self/class/respondsToSelector/isKindOfClass 函数已经被重载了。
    void setHasCustomCore() {
        bits.data()->clearFlags(RW_HAS_DEFAULT_CORE);
    }
#endif
```

### FAST_CACHE_HAS_CXX_CTOR/RW_HAS_CXX_CTOR/FAST_CACHE_HAS_CXX_DTOR/RW_HAS_CXX_DTOR
&emsp;`FAST_CACHE_HAS_CXX_CTOR` 用以在 `__LP64__` 平台下判断 `objc_class` 的 `cache_t cache` 的 `uint16_t _flags` 二进制表示时第 `1` 位的值是否为 `1`，以此表示该类或者父类是否有 `.cxx_construct` 函数实现。而在 `非 __LP64__` 平台下，则是使用 `RW_HAS_CXX_CTOR`，且判断的位置发生了变化，`RW_HAS_CXX_CTOR` 用以判断从 `objc_class` 的 `class_data_bits_t bits` 中取得 `class_rw_t` 指针指向的 `class_rw_t` 实例的 `uint32_t flags` 的第 `18` 位的值是否为 `1`，以此表示该类或者父类是否有 `.cxx_construct` 函数实现。对应的 `FAST_CACHE_HAS_CXX_DTOR` 和 `RW_HAS_CXX_DTOR` 表示该类或者父类是否有 `.cxx_destruct` 函数实现。
这里需要注意的是在 `__LP64__ && __arm64__` 平台下 `FAST_CACHE_HAS_CXX_DTOR` 是 `1<<0`，而在 `__LP64__ && !__arm64__` 平台下 `FAST_CACHE_HAS_CXX_DTOR` 是 `1<<2`。 

```c++
#if __LP64__
...
#if __arm64__
// class or superclass has .cxx_construct/.cxx_destruct implementation。
// 类或者父类有 .cxx_construct/.cxx_destruct 函数实现。

// FAST_CACHE_HAS_CXX_DTOR is the first bit so that setting
// it in isa_t::has_cxx_dtor is a single bfi.
// FAST_CACHE_HAS_CXX_DTOR 是第一位，
// 因此在 isa_t::has_cxx_dtor 中进行设置仅是一个位字段。
// uintptr_t has_cxx_dtor      : 1;

// __LP64__ && __arm64__ 平台下

#define FAST_CACHE_HAS_CXX_DTOR       (1<<0)
#define FAST_CACHE_HAS_CXX_CTOR       (1<<1)
...
#else
...
// class or superclass has .cxx_construct/.cxx_destruct implementation.
// 类或者父类有 .cxx_construct/.cxx_destruct 函数实现。

// FAST_CACHE_HAS_CXX_DTOR is chosen to alias with isa_t::has_cxx_dtor.
// 选择 FAST_CACHE_HAS_CXX_DTOR 作为 isa_t::has_cxx_dtor 的别名。

// __LP64__ && !__arm64__ 平台下

#define FAST_CACHE_HAS_CXX_CTOR       (1<<1)
#define FAST_CACHE_HAS_CXX_DTOR       (1<<2)
#endif
...
#else
...
// class or superclass has .cxx_construct implementation.
// 类或者父类有 .cxx_construct 函数实现。
#define RW_HAS_CXX_CTOR       (1<<18)

// class or superclass has .cxx_destruct implementation
// 类或者父类有 .cxx_destruct 函数实现。 
#define RW_HAS_CXX_DTOR       (1<<17)
...
#endif
```
### hasCxxCtor/setHasCxxCtor/hasCxxDtor/setHasCxxDtor
&emsp;在 `__LP64__` 平台和其它平台下判断、设置（注意这里没有清除）`objc_class` 的 `.cxx_construct/.cxx_destruct` 函数实现的标记位。
```c++
#if FAST_CACHE_HAS_CXX_CTOR
    // 直接判断 cache_t cache 的 uint16_t _flags 二进制表示的第 1 位的值是否为 1。
    bool hasCxxCtor() {
        ASSERT(isRealized());
        return cache.getBit(FAST_CACHE_HAS_CXX_CTOR);
    }
    
    // 以原子方式把 cache_t cache 的 uint16_t _flags 二进制表示的第 1 位设置为 1。
    void setHasCxxCtor() {
        cache.setBit(FAST_CACHE_HAS_CXX_CTOR);
    }
#else
    // 直接判断 class_rw_t 的 flags 二进制表示的第 18 位的值是否为 1。
    bool hasCxxCtor() {
        ASSERT(isRealized());
        return bits.data()->flags & RW_HAS_CXX_CTOR;
    }
    
    // 以原子方式把 class_rw_t 的 uint32_t flags 二进制表示的第 18 位设置为 1。
    void setHasCxxCtor() {
        bits.data()->setFlags(RW_HAS_CXX_CTOR);
    }
#endif

#if FAST_CACHE_HAS_CXX_DTOR
    // 在 __LP64__ 的 __arm64__ 下是 0，在 !__arm64__ 下是 2。
    // 直接判断 cache_t cache 的 uint16_t _flags 二进制表示的第 0/2 位的值是否为 1。
    bool hasCxxDtor() {
        ASSERT(isRealized());
        return cache.getBit(FAST_CACHE_HAS_CXX_DTOR);
    }
    
    // 以原子方式把 cache_t cache 的 uint16_t _flags 二进制表示的第 0/2 位设置为 1。
    void setHasCxxDtor() {
        cache.setBit(FAST_CACHE_HAS_CXX_DTOR);
    }
#else
    // 直接判断 class_rw_t 的 uint32_t flags 二进制表示的第 17 位的值是否为 1。
    bool hasCxxDtor() {
        ASSERT(isRealized());
        return bits.data()->flags & RW_HAS_CXX_DTOR;
    }
    
    // 以原子方式把 class_rw_t 的 uint32_t flags 二进制表示的第 17 位设置为 1。
    void setHasCxxDtor() {
        bits.data()->setFlags(RW_HAS_CXX_DTOR);
    }
#endif
```
### FAST_CACHE_REQUIRES_RAW_ISA/RW_REQUIRES_RAW_ISA
&emsp;`FAST_CACHE_REQUIRES_RAW_ISA` 用以在 `__LP64__` 平台下判断 `objc_class` 的 `cache_t cache` 的 `uint16_t _flags` 二进制表示时第 `13` 位的值是否为 `1`，以此表示类实例对象（此处是指类对象，不是使用类构建的实例对象，一定要记得）是否需要原始的 `isa`。而在 `非 __LP64__` 且 `SUPPORT_NONPOINTER_ISA` 的平台下，则是使用 `RW_REQUIRES_RAW_ISA`，且判断的位置发生了变化，`RW_REQUIRES_RAW_ISA` 用以判断从 `objc_class` 的 `class_data_bits_t bits` 中取得 `class_rw_t` 指针指向的 `class_rw_t` 实例的 `uint32_t flags` 的第 `15` 位的值是否为 `1`，以此表示类实例对象（此处是指类对象，不是使用类构建的实例对象，一定要记得）是否需要原始的 `isa`。

```c++
#if __LP64__
...
// class's instances requires raw isa.
// 类实例需要 raw isa。

#define FAST_CACHE_REQUIRES_RAW_ISA   (1<<13)
...

#else
...
// class's instances requires raw isa.
// 类实例需要 raw isa。

#if SUPPORT_NONPOINTER_ISA
#define RW_REQUIRES_RAW_ISA   (1<<15)
#endif
...

#endif
```
### instancesRequireRawIsa/setInstancesRequireRawIsa
&emsp;在 `__LP64__` 平台和其它平台下判断、设置类实例（此处是指类对象，不是使用类构建的实例对象，一定要记得）需要原始 `isa` 的标记位。
```c++
#if FAST_CACHE_REQUIRES_RAW_ISA
    // 直接判断 cache_t cache 的 uint16_t _flags 二进制表示的第 13 位的值是否为 1。
    bool instancesRequireRawIsa() {
        return cache.getBit(FAST_CACHE_REQUIRES_RAW_ISA);
    }
    
    // 以原子方式把 cache_t cache 的 uint16_t _flags 二进制表示的第 13 位设置为 1。
    void setInstancesRequireRawIsa() {
        cache.setBit(FAST_CACHE_REQUIRES_RAW_ISA);
    }
#elif SUPPORT_NONPOINTER_ISA
    // 直接判断 class_rw_t 的 uint32_t flags 二进制表示的第 15 位的值是否为 1。
    bool instancesRequireRawIsa() {
        return bits.data()->flags & RW_REQUIRES_RAW_ISA;
    }
    
    // 以原子方式把 class_rw_t 的 uint32_t flags 二进制表示的第 15 位设置为 1。
    void setInstancesRequireRawIsa() {
        bits.data()->setFlags(RW_REQUIRES_RAW_ISA);
    }
#else
    // 当 isa 是原始 isa 时，直接返回 true。
    bool instancesRequireRawIsa() {
        return true;
    }
    void setInstancesRequireRawIsa() {
        // nothing
    }
#endif
```
+ 当 `! __LP64__` 平台下，所有掩码都存储在 `class_rw_t` 的 `uint32_t flags`。
+ `__LP64__` 平台下，`FAST_HAS_DEFAULT_RR` 存储在 `class_data_bits_t bits` 的 `uintptr_t bits`。（`1UL<<2`）（`retain/release/autorelease/retainCount/_tryRetain/_isDeallocating/retainWeakReference/allowsWeakReference`）
+ `__LP64__` 平台下，`FAST_CACHE_HAS_DEFAULT_AWZ` 存储在 `cache_t cache` 的 `uint16_t _flags` 下。（`1<<14`）（`alloc/allocWithZone:`）
+ `__LP64__` 平台下，`FAST_CACHE_HAS_DEFAULT_CORE` 存储在 `cache_t cache` 的 `uint16_t _flags` 下。（`1<<15`）（`new/self/class/respondsToSelector/isKindOfClass`）
+ `__LP64__` 平台下，`FAST_CACHE_HAS_CXX_CTOR` 存储在 `cache_t cache` 的 `uint16_t _flags` 下。（`1<<1`）（`.cxx_construct`）
+ `__LP64__` 平台下，`FAST_CACHE_HAS_CXX_DTOR` 存储在 `cache_t cache` 的 `uint16_t _flags` 下。（`1<<2` / `1<<0`）（`.cxx_destruct`）
+ `__LP64__` 平台下，`FAST_CACHE_REQUIRES_RAW_ISA` 存储在 `cache_t cache` 的 `uint16_t _flags` 下。（`1<<13`）（`requires raw isa`）

### void printInstancesRequireRawIsa(bool inherited)
&emsp;打印类对象需要原始 `isa`，当环境变量 `OBJC_PRINT_RAW_ISA` `Value` 为 `true` 时会调用该函数，`inherited` 表示该类是否是一个子类。
`OPTION( PrintRawIsa, OBJC_PRINT_RAW_ISA, "log classes that require raw pointer isa fields")`
```c++
void
objc_class::printInstancesRequireRawIsa(bool inherited)
{
    // 打印标识开启，否则执行断言
    ASSERT(PrintRawIsa);
    // 类对象需要原始的 isa，否则执行断言 
    ASSERT(instancesRequireRawIsa());
    
    // 控制台输出需要原始 isa 的类名等信息
    _objc_inform("RAW ISA:  %s%s%s", nameForLogging(), 
                 isMetaClass() ? " (meta)" : "", 
                 inherited ? " (inherited)" : "");
}
```

### void setInstancesRequireRawIsaRecursively(bool inherited = false)
&emsp;将此类及其所有子类标记为需要原始 `isa` 指针，标记函数 `setInstancesRequireRawIsa` 很简单，上面我们已经分析过了， 这里涉及到一个更重要的知识点，就是我们如何才能获取一个类的所有子类呢 ？这里正式使用到了 `struct class_rw_t` 的两个成员变量 `Class firstSubclass` 和 `Class nextSiblingClass`，下面我们跟着函数调用流程一起来分析一下吧。
```c++
/*
* Mark this class and all of its subclasses as requiring raw isa pointers.
* 将此类及其所有子类标记为需要原始 isa 指针。
*/
void objc_class::setInstancesRequireRawIsaRecursively(bool inherited)
{
    // struct objc_class 的函数内部的 this 自然是 objc_class *。
    
    // 取得 objc_class 指针。
    Class cls = (Class)this;
    
    // 加锁，加锁失败则执行断言。
    runtimeLock.assertLocked();
    
    // 如果此类已经被标记为需要原始 isa，则直接 return。
    if (instancesRequireRawIsa()) return;
    
    // 枚举一个类及其所有已实现的子类。
    //（foreach_realized_class_and_subclass 函数是我们要重点关注的对象，正是它获取了所有子类）
    foreach_realized_class_and_subclass(cls, [=](Class c){
        
        if (c->instancesRequireRawIsa()) {
            // 如果此类已经被标记为需要原始 isa，则直接 return false，跳过该类继续遍历下一个子类。
            return false;
        }
        
        // 把 c 标记为需要原始 isa。
        c->setInstancesRequireRawIsa();
        
        // 是否在控制台打印
        if (PrintRawIsa) c->printInstancesRequireRawIsa(inherited || c != cls);
        
        // return true 继续执行遍历。
        return true;
    });
}
```

#### foreach_realized_class_and_subclass
```c++
// Enumerates a class and all of its realized subclasses.
// 枚举一个类及其所有已实现的子类。
static void
foreach_realized_class_and_subclass(Class top, bool (^code)(Class) __attribute((noescape)))
{
    unsigned int count = unreasonableClassCount();

    foreach_realized_class_and_subclass_2(top, count, false, code);
}
```

#### unreasonableClassCount
```c++
/*
* unreasonableClassCount
* Provides an upper bound for any iteration of classes,
* to prevent spins when runtime metadata is corrupted.
* 为类的任何迭代提供上限，以防止在运行时元数据损坏时发生死循环。
*/
static unsigned unreasonableClassCount()
{
    // 加锁
    runtimeLock.assertLocked();
    
    int base = NXCountMapTable(gdb_objc_realized_classes) +
    getPreoptimizedClassUnreasonableCount();

    // Provide lots of slack here. Some iterations touch metaclasses too.
    // 在此处提供大量的余地。一些迭代也涉及元类。
    // Some iterations backtrack (like realized class iteration).
    // 一些迭代回溯（例如实现的类迭代）。
    // We don't need an efficient bound, merely one that prevents spins.
    // 我们不需要有效的界限，只需防止旋转即可。
    
    return (base + 1) * 16;
}
```
#### foreach_realized_class_and_subclass_2
```c++
/*
* Class enumerators 类枚举器
* The passed in block returns `false` if subclasses can be skipped.
* 如果可以跳过子类，则传入的块将返回 "false"。
* Locking: runtimeLock must be held by the caller.
* runtimeLock 必须由调用方持有。
*/

// foreach_realized_class_and_subclass_2(top, count, false, code);
// top 是当前类，skip_metaclass 值是 false，code 就是我们枚举时的块 
// __attribute((noescape)) 想到了 Swift 中的闭包

static inline void
foreach_realized_class_and_subclass_2(Class top, unsigned &count,
                                      bool skip_metaclass,
                                      bool (^code)(Class) __attribute((noescape)))
{
    Class cls = top;

    runtimeLock.assertLocked();
    ASSERT(top);

    while (1) {
        if (--count == 0) {
            _objc_fatal("Memory corruption in class list.");
        }

        bool skip_subclasses;

        if (skip_metaclass && cls->isMetaClass()) {
            skip_subclasses = true;
        } else {
            skip_subclasses = !code(cls);
        }

        if (!skip_subclasses && cls->data()->firstSubclass) {
            cls = cls->data()->firstSubclass;
        } else {
            while (!cls->data()->nextSiblingClass  &&  cls != top) {
                cls = cls->superclass;
                if (--count == 0) {
                    _objc_fatal("Memory corruption in class list.");
                }
            }
            if (cls == top) break;
            cls = cls->data()->nextSiblingClass;
        }
    }
}
```
### bool canAllocNonpointer()
&emsp;表示 `objc_class` 的 `isa` 是非指针，即类对象不需要原始 `isa` 时，能根据该函数返回值设置 `isa_t isa` 的 `uintptr_t nonpointer : 1` 字段，标记该类的 `isa` 是非指针。

```c++
bool canAllocNonpointer() {
    ASSERT(!isFuture());
    return !instancesRequireRawIsa();
}
```

### bool isSwiftStable()
&emsp;调用 `class_data_bits_t bits` 的 `isSwiftStable` 函数，内部实现是通过与操作判断 `uintptr_t bits` 的二进制表示的第 `1` 位是否是 `1`，表示该类是否是有稳定的 `Swift ABI` 的 `Swift` 类。
```c++
// class is a Swift class from the stable Swift ABI.
// class 是一个有稳定的 Swift ABI 的 Swift 类。
// #define FAST_IS_SWIFT_STABLE    (1UL<<1)

bool isSwiftStable() {
    return bits.isSwiftStable();
}
```

### bool isSwiftLegacy()
&emsp;调用 `class_data_bits_t bits` 的 `isSwiftLegacy` 函数，内部实现是通过与操作判断 `uintptr_t bits` 的二进制表示的第 `0` 位是否是 `1`，表示该类是否是有稳定的 `Swift ABI` 的 `Swift` 类。（遗留的类）
```c++
// class is a Swift class from the pre-stable Swift ABI.
// class 是来自稳定的 Swift ABI 的 Swift 类。(遗留的类)
// #define FAST_IS_SWIFT_LEGACY    (1UL<<0)

bool isSwiftLegacy() {
    return bits.isSwiftLegacy();
}
```

### bool isAnySwift()
&emsp;调用 `class_data_bits_t bits` 的 `isAnySwift` 函数，`isSwiftStable` 或者 `isSwiftLegacy`。
```c++
bool isAnySwift() {
    return bits.isAnySwift();
}

// struct class_data_bits_t 的 isAnySwift 函数实现: 
bool isAnySwift() {
    return isSwiftStable() || isSwiftLegacy();
}
```

### bool isSwiftStable_ButAllowLegacyForNow()
&emsp;调用 `struct class_data_bits_t` 的 `isSwiftStable_ButAllowLegacyForNow` 函数。
```c++
bool isSwiftStable_ButAllowLegacyForNow() {
    return bits.isSwiftStable_ButAllowLegacyForNow();
}

// struct class_data_bits_t 的 isSwiftStable_ButAllowLegacyForNow 函数实现
// fixme remove this once the Swift runtime uses the stable bits.
// fixme 一旦 Swift runtime 使用稳定位将其删除。
bool isSwiftStable_ButAllowLegacyForNow() {
    return isAnySwift();
}

bool isAnySwift() {
    return isSwiftStable() || isSwiftLegacy();
}
```

### bool isStubClass() const
&emsp;全局搜索此函数发现只在 `objc_class` 的 `bool isRealized() const` 函数内调用了一次，它用于判断类对象是否已经实现完成。 
```c++
bool isStubClass() const {
    // isaBits 函数继承自 struct objc_object，
    // 它的实现只有一行就是取出 isa_t isa 的 uintptr_t bits 成员变量。
    // 如果 isa 是原始指针的话，那么取出的值是一个内存地址被强转为一个 unsigned long 值，
    // 即使 isa 不是原始的 isa，是一个优化的 isa，那它强转为 unsigned long 的值的话也会远大于 16 吧
    
    uintptr_t isa = (uintptr_t)isaBits();
    
    return 1 <= isa && isa < 16;
}
```

### bool isUnfixedBackwardDeployingStableSwift()
```c++
// Swift stable ABI built for old deployment targets looks weird.
// 为旧的部署目标构建的 Swift 稳定的 ABI 看起来很奇怪。

// The is-legacy bit is set for compatibility with old libobjc.
// 设置 is-legacy 位是为了与旧的 libobjc 兼容。

// We are on a "new" deployment target so we need to rewrite that bit.
// 我们的部署目标是“新的”，因此我们需要重写一下。

// These stable-with-legacy-bit classes are distinguished from real
// legacy classes using another bit in the Swift data.
// 这些具有传统稳定性的类使用 Swift 数据中的另一位与实际的传统类区分开.
// (ClassFlags::IsSwiftPreStableABI)

bool isUnfixedBackwardDeployingStableSwift() {
    // Only classes marked as Swift legacy need apply.
    // 仅需要标记为 Swift legacy 的类适用。
    if (!bits.isSwiftLegacy()) return false;

    // Check the true legacy vs stable distinguisher.
    // 检查真正的传统 vs 稳定的区分符。
    
    // The low bit of Swift's ClassFlags is SET for true 
    // legacy and UNSET for stable pretending to be legacy.
    // Swift 的 ClassFlags 的低位设置为 SET（表示真正的遗留物），而 UNSET（低位）用于稳定地假装为遗留物。
    
    // 用 & 取出 class_data_bits_t bits 的地址加 1 是什么操作，
    // struct class_data_bits 只有一个成员变量 uintptr_t bits，
    // 那对 class_data_bits_t bits 取地址取出也是 uintptr_t bits 的地址，
    // 通过前面的分析我们已知 uintptr_t bits 的第 0 位是标记 #define FAST_IS_SWIFT_LEGACY (1UL<<0),
    // 这里的加 1 操作....
    
    // 对 bits 的地址加 1,
    uint32_t swiftClassFlags = *(uint32_t *)(&bits + 1);
    
    bool isActuallySwiftLegacy = bool(swiftClassFlags & 1);
    return !isActuallySwiftLegacy;
}
```

### fixupBackwardDeployingStableSwift
```c++
void fixupBackwardDeployingStableSwift() {
    if (isUnfixedBackwardDeployingStableSwift()) {
        // Class really is stable Swift, pretending to be pre-stable.
        // 类确实是稳定的 Swift，假装是稳定的。
        // Fix its lie.
        // 修正谎言
        
        // 把 class_data_bits_t bits 的 uintptr_t bits 的 #define FAST_IS_SWIFT_STABLE (1UL<<1)
        // 置为 1, 并把 #define FAST_IS_SWIFT_LEGACY (1UL<<0) 置为 0。
        bits.setIsSwiftStable();
    }
}

// 暂时不理解是干啥用的
_objc_swiftMetadataInitializer swiftMetadataInitializer() {
    return bits.swiftMetadataInitializer();
}
```

**下面的一些掩码相关的操作我们开始用到 `struct class_ro_t` 的 `uint32_t flags`了！它们的宏定义都是以 `RO_` 开头的。**

### RO_IS_ARC/RO_HAS_WEAK_WITHOUT_ARC
&emsp;在 `struct class_ro_t` 的 `uint32_t flags` 中使用的掩码。
```c++
// class compiled with ARC
// 由 ARC 编译的类
#define RO_IS_ARC             (1<<7)
// class is not ARC but has ARC-style weak ivar layout.
// 类不是 ARC，但具有 ARC 风格的 weak ivar 布局。
#define RO_HAS_WEAK_WITHOUT_ARC (1<<9)
```
### bool hasAutomaticIvars()
&emsp;从 `class_data_bits_t bits` 中取出 `class_rw_t` 指针，然后从 `struct class_rw_t` 中取出 `explicit_atomic<uintptr_t> ro_or_rw_ext` 对应的 `class_rw_ext_t` 指针，然后从 `struct class_rw_ext_t` 中取出 `const class_ro_t *ro`，然后取出 `uint32_t flags` 和 （`RO_IS_ARC | RO_HAS_WEAK_WITHOUT_ARC` （二进制表示第 `7` 位和第 `9` 位是 `1`，其它位都是 `0`））做与操作。
```c++
// Return YES if the class's ivars are managed by ARC, or the class is MRC but has ARC-style weak ivars.
// 如果类的 ivars 由 ARC 管理，或者该类是 MRC 但具有 ARC 样式的 weak ivars，则返回 YES。
// (weak 修饰符是可以在 MRC 中使用的，weak 是和 ARC 一起推出的，
// 根据之前 weak 的实现原理也可知它的实现流程和 ARC 或者 MRC 是完全没有关系的。)

bool hasAutomaticIvars() {
    return data()->ro()->flags & (RO_IS_ARC | RO_HAS_WEAK_WITHOUT_ARC);
}
```
### bool isARC()
&emsp;同上，最后取出 `class_ro_t` 的 `uint32_t flags` 和 `RO_IS_ARC` 做与操作。
```c++
// Return YES if the class's ivars are managed by ARC.
// 如果类的 ivar 由 ARC 管理，则返回 YES。
bool isARC() {
    return data()->ro()->flags & RO_IS_ARC;
}
```

### RW_FORBIDS_ASSOCIATED_OBJECTS
&emsp;禁止类的实例对象进行关联对象的掩码，看到它前缀是 `RW` 开始的，表示它用在 `struct class_rw_t` 的 `uint32_t flags` 中。（`AssociatedObject` 的实现原理可以参考之前的文章）
```c++
// class does not allow associated objects on its instances.
// 类不允许在其实例上使用 关联对象。
#define RW_FORBIDS_ASSOCIATED_OBJECTS       (1<<20)
```

### bool forbidsAssociatedObjects()
&emsp;禁止该类的实例对象进行 `AssociatedObject`。从 `class_data_bits_t bits` 中取出 `class_rw_t` 指针，然后从 `struct class_rw_t` 中取出 `uint32_t flags` 和 `RW_FORBIDS_ASSOCIATED_OBJECTS`（第 `20` 位值为 `1`）与操作的结果。
```c++
bool forbidsAssociatedObjects() {
    // class_rw_t 的 flags 做与操作
    return (data()->flags & RW_FORBIDS_ASSOCIATED_OBJECTS);
}
```
### instancesHaveAssociatedObjects/setInstancesHaveAssociatedObjects
&emsp;在 `struct class_rw_t` 的 `uint32_t flags` 做掩码操作。
```c++
// class instances may have associative references.
// 类实例可能具有关联引用。
#define RW_INSTANCES_HAVE_ASSOCIATED_OBJECTS (1<<22)

#if SUPPORT_NONPOINTER_ISA
    // Tracked in non-pointer isas; not tracked otherwise
#else
    bool instancesHaveAssociatedObjects() {
        // this may be an unrealized future class in the CF-bridged case.
        // 在 CF 桥接的情况下，这可能是未实现的未来 class。
        ASSERT(isFuture()  ||  isRealized());
        // class_rw_t 的 flags
        return data()->flags & RW_INSTANCES_HAVE_ASSOCIATED_OBJECTS;
    }

    void setInstancesHaveAssociatedObjects() {
        // this may be an unrealized future class in the CF-bridged case.
        // 在CF桥接的情况下，这可能是未实现的未来 class。
        ASSERT(isFuture()  ||  isRealized());
        // class_rw_t 的 flags
        setInfo(RW_INSTANCES_HAVE_ASSOCIATED_OBJECTS);
    }
#endif
```
### shouldGrowCache/setShouldGrowCache
```c++
// 默认为 true
bool shouldGrowCache() {
    return true;
}

void setShouldGrowCache(bool) {
    // fixme good or bad for memory use?
}
```
### RW_INITIALIZING
&emsp;判断 `objc_class` 是否正在进行初始化的掩码。判断位置在 `struct class_rw_t` 的 `uint32_t flags` 中。
```c++
// class is initializing
// class 正在初始化 （class_rw_t flags 的 第 28 位）
#define RW_INITIALIZING       (1<<28)
```
### bool isInitializing()
&esmp;`RW_INITIALIZING` 是 `RW` 前缀开头，可直接联想到其判断位置在 `struct class_rw_t` 的 `uint32_t flags` 中，与前面的一些判断相比这里 `objc_class` 的位置发生了变化，前面我们所有的判断都是在当前的 `objc_class` 中进行的，而此处的判断要转移到当前 `objc_class` 的元类中，元类的类型也是 `struct objc_class`，所以它们同样也有 `class_data_bits_t bits`、`cache_t cache` 等成员变量，这里 `isInitializing` 函数使用的正是元类的 `class_data_bits_t bits` 成员变量。`getMeta` 函数是取得当前 `objc_class` 的元类，然后 `data`  函数从元类的 `class_data_bits_t bits` 中取得 `class_rw_t` 指针，然后取得 `struct class_rw_t` 的 `uint32_t flags` 和 `RW_INITIALIZING` 做与操作，取得 `flags` 二进制表示的第 `28` 位的值作为结果返回。

```c++
bool isInitializing() {
    return getMeta()->data()->flags & RW_INITIALIZING;
}
```

### void setInitialized()
&emsp;标记该类初始化完成。
```c++
/*
* Locking: write-locks runtimeLock
* 此过程需要加锁
*/
void 
objc_class::setInitialized()
{
    // 记录当前类的元类的临时变量
    Class metacls;
    // 记录当前类的临时变量
    Class cls;

    // 如果当前类是元类，则执行断言
    ASSERT(!isMetaClass());

    // 当前类
    cls = (Class)this;
    // 元类
    metacls = cls->ISA();

    // 加锁
    mutex_locker_t lock(runtimeLock);

    // Special cases:
    // 特别情况
    // - NSObject AWZ  class methods are default.
    // - NSObject RR   class and instance methods are default.
    // - NSObject Core class and instance methods are default.
    
    // adjustCustomFlagsForMethodChange() also knows these special cases.
    // attachMethodLists() also knows these special cases.

    objc::AWZScanner::scanInitializedClass(cls, metacls);
    objc::RRScanner::scanInitializedClass(cls, metacls);
    objc::CoreScanner::scanInitializedClass(cls, metacls);

    // Update the +initialize flags.
    // 更新 +initialize 字段。
    // Do this last.
    // 设置元类的 RW_INITIALIZED 标记位，并清除 RW_INITIALIZING 标记位
    //（struct class_rw_t 的 uint32_t flags 成员变量）
    metacls->changeInfo(RW_INITIALIZED, RW_INITIALIZING);
}
```
### bool isLoadable()
&emsp;默认为 `true`。
```c++
bool isLoadable() {
    ASSERT(isRealized());
    // any class registered for +load is definitely loadable.
    return true;
}
```
### IMP getLoadMethod()
&emsp;获取一个类的 `+load` 函数，首先我们要对 `+load` 函数和别的函数做出一些理解上的区别，首先我们在任何时候都不应该自己主动去调用 `+load` 函数，它是由系统自动调用的，且它被系统调用时是直接通过它的函数地址调用的，它是不走 `objc_msgSend` 消息发送流程的。当我们在自己的类定义中添加了 `+load` 函数，编译过程中编译器会把它存储在元类的 `struct class_ro_t` 的 `method_list_t * baseMethodList` 成员变量中。那么 `category` 中的 `+load` 函数在编译过程中会被放在哪里呢？

```c++
/*
* objc_class::getLoadMethod
* fixme
* Called only from add_class_to_loadable_list.
* 仅从 add_class_to_loadable_list 调用。
* Locking: runtimeLock must be read- or write-locked by the caller.
* Locking: runtimeLock 必须是读写锁。
*/
IMP 
objc_class::getLoadMethod()
{
    // 加锁
    runtimeLock.assertLocked();

    // 临时变量，const 表示 mlist 指向的 method_list_t 不可变。
    const method_list_t *mlist;

    // 当前 objc_class 和其元类 已经实现完成，
    // 当前 objc_class 不能是元类，且它的 ISA() 是元类
    ASSERT(isRealized());
    ASSERT(ISA()->isRealized());
    ASSERT(!isMetaClass());
    ASSERT(ISA()->isMetaClass());

    // 获取当前类的元类的 class_ro_t 的 method_list_t * baseMethodList
    mlist = ISA()->data()->ro()->baseMethods();
    
    // 如果 mlist 存在，则遍历找到 +load 函数
    if (mlist) {
        for (const auto& meth : *mlist) {
            // 取得函数的名字 (SEL)
            const char *name = sel_cname(meth.name);
            // 如果是 +load 函数，则直接返回 +load 函数的 imp
            if (0 == strcmp(name, "load")) {
                return meth.imp;
            }
        }
    }
    // 如果没有找到则返回 nil
    return nil;
}
```
### RW_REALIZED
&emsp;在 `struct class_rw_t` 的 `uint32_t flags` 二进制表示的第 `31` 位
```c++
#define RW_REALIZED           (1<<31)
```

### bool isRealized() const
&emsp;`struct class_rw_t` 的 `uint32_t flags` 二进制表示的第 `31` 位和 `RW_REALIZED` 与操作的结果。 
```c++
// Locking: To prevent concurrent realization, hold runtimeLock.
// Locking: 为了防止并发实现，需要加锁。
bool isRealized() const {
    return !isStubClass() && (data()->flags & RW_REALIZED);
}
```

### RW_FUTURE
```c++
// class is unresolved future class.
// class 是未解决的 future class。
#define RW_FUTURE             (1<<30)
```

### bool isFuture() const
&emsp;`struct class_rw_t` 的 `uint32_t flags` 二进制表示的第 `30` 位和 `RW_REALIZED` 与操作的结果。 
```c++
// Returns true if this is an unrealized future class.
// 如果这是未实现的 future class，则返回true。
// Locking: To prevent concurrent realization, hold runtimeLock.
// Locking: 为了防止并发实现，需要加锁。
bool isFuture() const {
    return data()->flags & RW_FUTURE;
}
```

### FAST_CACHE_META/RW_META/RO_META
&emsp;在 `__LP64__` 平台下标识 `objc_class` 是否是元类的值在 `cache_t cache` 中，其它情况则是在 `struct class_rw_t` 的 `uint32_t flags` 中（需要根据指针进行寻址）。
```c++
// class is a metaclass.
// 类是元类。
#define RO_META               (1<<0)
// class is a metaclass (copied from ro).
// 类是元类。
#define RW_META               RO_META // (1<<0)

#if __LP64__
...
#if __arm64__
...
// Denormalized RO_META to avoid an indirection.
// 为避免间接化，对 RO_META 进行了规范化处理。
#define FAST_CACHE_META               (1<<2)
#else
// Denormalized RO_META to avoid an indirection.
// 为避免间接化，对 RO_META 进行了规范化处理。
#define FAST_CACHE_META               (1<<0)
...
#endif
...
#else
...
#endif
```

### bool isMetaClass()
&emsp;如果 `FAST_CACHE_META` 存在，则从 `cache_t cache` 的 `uint16_t _flags` 二进制表示的第 `2/0` 位判断当前 `objc_class` 是否是元类。其它情况则从 `class_data_bits_t bits` 中取得 `class_rw_t` 指针指向的 `class_rw_t` 实例的 `uint32_t flags` 二进制表示的第 `0` 位进行判断。
```c++
    bool isMetaClass() {
        ASSERT(this);
        ASSERT(isRealized());
        
#if FAST_CACHE_META
        return cache.getBit(FAST_CACHE_META);
#else
        return data()->flags & RW_META;
#endif
    }
```

### bool isMetaClassMaybeUnrealized()
```c++
// Like isMetaClass, but also valid on un-realized classes.
// 类似于 isMetaClass，但在未实现的类上也有效。
bool isMetaClassMaybeUnrealized() {
    static_assert(offsetof(class_rw_t, flags) == offsetof(class_ro_t, flags), "flags alias");
    static_assert(RO_META == RW_META, "flags alias");
    
    // 同上，从 struct class_rw_t 的 uint32_t flags 中判断 
    return data()->flags & RW_META;
}
```

### Class getMeta()
&emsp;取得当前类的元类。
```c++
// NOT identical to this->ISA when this is a metaclass.
Class getMeta() {
    // 如果当前类是元类，则直接返回 this，
    // 如果不是元类，则调用 objc_object::ISA() 函数取得元类。
    //（多半是从 isa 中根据掩码取出类的地址：(Class)(isa.bits & ISA_MASK)）
    
    if (isMetaClass()) return (Class)this;
    else return this->ISA();
}
```
### bool isRootClass()
&emsp;判断一个类是否是根类，只是判断一个类的 `superclass` 是否为 `nil`。
+ 根类的父类是 `nil`，根类的元类是根元类。
+ 根元类的父类是根类，根元类的元类是自己。
```c++
bool isRootClass() {
    // 如果 objc_class 的 superclass 是 nil，则该类是根类。 
    return superclass == nil;
}
```
### bool isRootMetaclass()
&emsp;根元类的元类指向自己。
```c++
bool isRootMetaclass() {
    // 如果一个 objc_class 的元类是自己的话，那么只可能是根元类。
    return ISA() == (Class)this;
}
```

### const char *mangledName()
&emsp;伪装一个类的名字，看到其是保存在 `struct class_ro_t` 的 `const char * name` 内。
```c++
const char *mangledName() { 
    // fixme can't assert locks here
    // fixme 不能在这里断言锁
    
    ASSERT(this);

    if (isRealized()  ||  isFuture()) {
        // 如果类已经实现了则 data() 返回的是 class_rw_t *，
        // 然后再从 class_rw_ext_t * 返回 const class_ro_t *ro，
        // 然后再返回 struct class_ro_t 的 const char * name。
        return data()->ro()->name;
    } else {
        // data() 返回的是 class_ro_t *，
        // 然后返回 const char * name。
        return ((const class_ro_t *)data())->name;
    }
}

const char *demangledName(bool needsLock);
const char *nameForLogging();
```

### word_align
&emsp;根据入参 `x` 进行 `8/4` 字节对齐。
```c++
#ifdef __LP64__
...
#   define WORD_MASK 7UL
...
#else
...
#   define WORD_MASK 3UL
...
#endif

static inline uint32_t word_align(uint32_t x) {
    // __LP64__ 下 WORD_MASK 7UL :
    // x = 12        => 0b 0000 0000 0000 0000 0000 0000 0000 1100 
    // WORD_MASK     => 0b 0000 0000 0000 0000 0000 0000 0000 0111 
    // ~WORD_MASK    => 0b 1111 1111 1111 1111 1111 1111 1111 1000
    // x + WORD_MASK => 0b 0000 0000 0000 0000 0000 0000 0001 0011
    // & ~WORD_MASK  => 0b 0000 0000 0000 0000 0000 0000 0001 0000
    
    // 16
    return (x + WORD_MASK) & ~WORD_MASK;
}
```
### uint32_t unalignedInstanceStart() const
&emsp;`unalignedInstanceStart` 只是表示第一个成员变量的所占用的字节大小吗？（如之前定义的一个继承自 `NSObject` 的类的 `instanceStart` 的值是 `8`）
```c++
// May be unaligned depending on class's ivars.
// 根据 class 的 ivars 可能没有内存对齐。
uint32_t unalignedInstanceStart() const {
    ASSERT(isRealized());
    
    // 直接返回 struct class_ro_t 的 uint32_t instanceStart
    return data()->ro()->instanceStart;
}
```
### uint32_t alignedInstanceStart() const
```c++
// Class's instance start rounded up to a pointer-size boundary.
// 类的实例开始四舍五入到指针大小的边界。
// This is used for ARC layout bitmaps.
// 这用于 ARC 布局位图。

uint32_t alignedInstanceStart() const {
    // 调用 word_align 工具函数，计算一个大于等于入参的最小的 8 的倍数。
    // 如 x = 7 返回 8
    // 如 x = 8 返回 8
    // 如 x = 12 返回 16 
    // ...
    return word_align(unalignedInstanceStart());
}
```
### uint32_t unalignedInstanceSize() const
```c++
// May be unaligned depending on class's ivars.
// 根据 class 的 ivars 可能没有内存对齐。
uint32_t unalignedInstanceSize() const {
    ASSERT(isRealized());
    
    // 直接返回 struct class_ro_t 的 uint32_t instanceSize
    // 根据内存对齐计算成员变量从前到后所占用的内存大小，
    // 不过没有进行总体的内存对齐，例如最后一个成员变量是 char 时，
    // 则最后只是加 1，instanceSize 的值是一个奇数，
    // 再进行一个整体 8 字节对齐就好了，
    // alignedInstanceSize 函数，完成了这最后一步的整体内存对齐
    return data()->ro()->instanceSize;
}
```
### uint32_t alignedInstanceSize() const
```c++
// Class's ivar size rounded up to a pointer-size boundary.
// 类的 ivar 大小四舍五入到指针大小的边界。
//（对 instanceSize 进行 8 字节对齐）
uint32_t alignedInstanceSize() const {
    // __LP64__ 平台下 8 字节对齐，其它则是 4 字节对齐
    return word_align(unalignedInstanceSize());
}
```
### size_t instanceSize(size_t extraBytes) const
&emsp;
```c++
size_t instanceSize(size_t extraBytes) const {
    // 从 cache_t cache 的 uint16_t _flags 判断是否能快速获取实例大小
    if (fastpath(cache.hasFastInstanceSize(extraBytes))) {
        return cache.fastInstanceSize(extraBytes);
    }

    // 正常调用 alignedInstanceSize 并添加需要延展的 bytes。
    size_t size = alignedInstanceSize() + extraBytes;
    
    // CF requires all objects be at least 16 bytes.
    // CF要求所有对象至少为 16 个字节。
    
    if (size < 16) size = 16;
    
    return size;
}
```
### void setInstanceSize(uint32_t newSize)
&emsp;`cache.setFastInstanceSize(newSize)` 把类实例的大小放在了 `cache_t cache` 的 `uint16_t _flags` 中，方便进行快速获取，减少了 `class_rw_t` 指针的寻址。
```c++
void setInstanceSize(uint32_t newSize) {
    ASSERT(isRealized());
    ASSERT(data()->flags & RW_REALIZING);
    
    // 取得 class_ro_t
    auto ro = data()->ro();
    
    if (newSize != ro->instanceSize) {
        // 如果 newSize 不等于 ro->instanceSize
        ASSERT(data()->flags & RW_COPIED_RO);
        // 更新 instanceSize 
        *const_cast<uint32_t *>(&ro->instanceSize) = newSize;
    }
    
    // 把 newSize 保存在 cache_t cache 的 uint16_t _flags 中，方便进行快速获取，减少了 class_rw_t 指针的寻址
    cache.setFastInstanceSize(newSize);
}
```
### chooseClassArrayIndex/setClassArrayIndex/classArrayIndex
&emsp;关于类在全局类表中的索引，这里不再展开了。

&emsp;至此，`objc_class` 的所有函数就全部看完了。

## 参考链接
**参考链接:🔗**
+ [iOS之LLDB常用命令](https://juejin.im/post/6869621360415637518)
