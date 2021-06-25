# iOS 从源码解析Runtime (十一)：聚焦 objc_class(class_data_bits_t 内容篇)

> &emsp;`objc_class` 的 `cache_t cache` 成员变量中涉及方法缓存的内容终于完完整整分析完了，接下来我们继续分析 `class_data_bits_t data`。

## class_data_bits_t
&emsp;`class_data_bits_t data` 作为 `objc_class` 的第四个成员变量也是最后一个成员变量，它的作用正如它的名字 `data`，而它也正是存储管理了类相关的所有数据，例如我们上篇一直讲的当缓存未命中时就会去类的方法列表中继续查找，而这个方法列表正保存在 `data` 中，且方法列表不仅包含我们直接在类定义中编写的实例函数还包括我们在分类中为类添加的实例方法它们也会被追加到 `data` 中，等等 `data` 包含了太多数据和功能。下面我们一行一行来看它为类处理了哪些数据管理了类的哪些功能吧！⛽️⛽️
```c++
struct objc_class : objc_object {
    // Class ISA;
    Class superclass;
    cache_t cache;             // formerly cache pointer and vtable
    
    // 注释点明了核心 class_rw_t * 加重载 rr/alloc 标记
    class_data_bits_t bits;    // class_rw_t * plus custom rr/alloc flags
    ...
};
```
### class_data_bits_t 成员变量
&emsp;在 `struct class_data_bits_t` 定义中声明了 `objc_class` 为其友元类，`objc_class` 可以完全访问和调用 `class_data_bits_t` 的私有成员变量和私有方法。然后是仅有的一个成员变量 `uintptr_t bits`，这里之所以把它命名为 `bits` 也是有其意义的，它通过掩码的形式保存 `class_rw_t` 指针和是否是 `swift` 类等一些标志位。   
```c++
struct class_data_bits_t {
    friend objc_class;

    // Values are the FAST_ flags above.
    uintptr_t bits;
    ...
};
```
### class_data_bits_t private 部分
&emsp;代码中的注释真的超详细，这里就不展开说了。主要完成对 `bits` 的 `64` 位中指定位的设置和读取操作。
```c++
private:
    // 尾部的 const 表示该方法内部不会修改 class_data_bits_t 的内部数据
    // 这里返回值是一个 bool 类型，通过与操作来取出 bits 的指定位的值来进行判断
    bool getBit(uintptr_t bit) const
    {
        // 内部实现只有一个与操作，主要根据入参 bit(掩码) 来取得一些标识位
        // 如: 
        
        // class is a Swift class from the pre-stable Swift ABI
        // #define FAST_IS_SWIFT_LEGACY    (1UL<<0)
        // 使用 bit 的第 0 位来进行判断
        
        // class is a Swift class from the stable Swift ABI
        // bit 的 第 1 位 判断类是否是稳定的 Swift ABI 的 Swift 类
        // #define FAST_IS_SWIFT_STABLE (1UL<<1)
        
        return bits & bit;
    }

    // Atomically set the bits in `set` and clear the bits in `clear`.
    // 以原子方式设置 `set` 中的位，并清除 `clear` 中的位。
    // set and clear must not overlap.
    // 设置和清除不得重叠。
    
    // 以原子方式设置 bits 中的指定的标识位，然后再进行指定的标识位清理
    // （设置操作即是把指定的位置 1）
    // （清理操作即是把指定的位置 0）
    void setAndClearBits(uintptr_t set, uintptr_t clear)
    {
        // 断言，如果 set 和 clear 都是 1 的则没有执行的必要，直接执行断言
        ASSERT((set & clear) == 0);
        
        // 临时变量保存旧值
        uintptr_t oldBits;
        // 临时变量保存新值
        uintptr_t newBits;
        
        // do while 循环在 retain 和 release 里面我们已经见过了，
        // 主要是为了保证在多线程下 bits 位一定被正确设置了。
        
        // 循环条件：!StoreReleaseExclusive(&bits, oldBits, newBits)
        
        // 当 &bits 与 oldBits 相同时，把 newBits 复制到 &bits，
        // 并返回 true，由于前面的 ! 取反，此时会结束 do while 循环
        
        // 当 &bits 与 oldBits 不同时，把 oldBits 复制到 &bits，
        // 并返回 false，由于前面的 ! 取反，此时会继续 do while 循环
        
        do {
            // 以原子方式读取出 bits
            oldBits = LoadExclusive(&bits);
            
            // 这里根据运算优先级，其实也不是，是直接的小括号的最高优先级
            // 先拿 oldBits 和 set 做或操作保证所有 64 位的 1 都保留下来
            // 然后 ~clear 的操作已经把需要 clear 的位置为 0，然后无关的其他位都是 1
            // 最后和 ~clear 做与操作并把结果赋值给 newBits，
            // 此时的 newBits 和原始的 bits 比的话，正是把入参 set 位置为 1 把入参 clear 位置为 0 
            newBits = (oldBits | set) & ~clear;
            
        // while 循环正保证了 newBits 能正确的设置到的 bits 中
        } while (!StoreReleaseExclusive(&bits, oldBits, newBits)); 
    }

    // 以原子方式设置 bits 中指定的标识位
    void setBits(uintptr_t set) {
        __c11_atomic_fetch_or((_Atomic(uintptr_t) *)&bits, set, __ATOMIC_RELAXED);
    }
    
    // 以原子方式清理 bits 中指定的标识位
    void clearBits(uintptr_t clear) {
        __c11_atomic_fetch_and((_Atomic(uintptr_t) *)&bits, ~clear, __ATOMIC_RELAXED);
    }
```
### class_data_bits_t public 部分
```c++
public:
    
    // 最最最最重要的函数来啦，从 bits 中读出 class_rw_t 指针
    //（class_rw_t 的具体作用下面会详细讲解）
    
    // __LP64__: #define FAST_DATA_MASK 0x00007ffffffffff8UL
    // !__LP64__: #define FAST_DATA_MASK 0xfffffffcUL
    
    class_rw_t* data() const {
        // 与操作取出 class_rw_t 指针
        return (class_rw_t *)(bits & FAST_DATA_MASK);
    }
    
    // 上面是读取 class_rw_t 指针，那这里就是设置啦
    
    void setData(class_rw_t *newData)
    {   
        // 断言点明了应该何时调用 setData 来设置 class_rw_t 指针
        // 虽然这里有个 newData->flags 我们还没有遇到，但是不影响我们分析当前情况
        
        // class has started realizing but not yet completed it
        // RW_REALIZING 标识类已经开始实现但是还没有完成，即类正在实现过程中
        
        //（标识在第十九位）
        // #define RW_REALIZING (1<<19) 
        
        // class is unresolved future class
        // 类是尚未解决的未来类
        
        //（标识在第三十位）
        // #define RW_FUTURE (1<<30)
        
        // data() 不存在或者类处于 RW_REALIZING 或是 RW_FUTURE，否则执行断言
        ASSERT(!data()  ||  (newData->flags & (RW_REALIZING | RW_FUTURE)));
        
        // Set during realization or construction only. No locking needed.
        // setData 仅在 实现或构造期间 设置。不需要加锁。
        // Use a store-release fence because there may be
        // concurrent readers of data and data's contents.
        // 使用了一个 store-release fence 因为可能同时存在 数据读取器 和 数据内容读取器。
        
        // #define FAST_DATA_MASK 0x00007ffffffffff8UL
        
        // 首先是 bits 和 ~FAST_DATA_MASK 做一个与操作，
        // 即把 bits 中与 class_rw_t 指针相关的掩码位全部置为 0 同时保存其它位的值
        // 然后和 newData 做一个或操作，即把 newData 的地址有效位放到 bits 的对应的掩码位中
        // bits 的其它位则保持原值
        
        // 把结果赋值给 newBits
        uintptr_t newBits = (bits & ~FAST_DATA_MASK) | (uintptr_t)newData;
        
        // 线程围栏
        atomic_thread_fence(memory_order_release);
        
        bits = newBits;
    }

    // Get the class's ro data, even in the presence of concurrent realization.
    // 即使存在并发实现，也获取类的 ro 数据。
    
    // fixme this isn't really safe without a compiler barrier at
    // least and probably a memory barrier when realizeClass changes the data field.
    // fixme 这至少在没有编译器障碍的情况下并不是真正安全的，而当实现 Class 更改数据字段时可能没有内存障碍
    
    // 这里有一个点，大概我们之前已经见过 class_rw_t 和 class_ro_t 两者，
    // 它们两个一个是可读可写的，一个是只读的，
    // class_ro_t 中的内容都来自于 class 的定义中，当类没有实现完成时 data 函数返回的是 class_ro_t，
    // 当类实现完成后，class_ro_t 会赋值给 class_rw_t 的 ro 成员变量，
    // 且此时 data 函数返回也变为了 class_rw_t 指针。
    
    const class_ro_t *safe_ro() {
        // 取得 data，这里用了一个 maybe_rw 的临时变量名，
        // 因为此时分两种情况，如果类已经实现完成，则 data 函数返回的是 class_rw_t 指针，
        // 而如果类没有实现完成的话返回的是 class_ro_t 指针
        
        class_rw_t *maybe_rw = data();
        
        // #define RW_REALIZED (1<<31)
        // class_rw_t->flags 的第 31 位标识了类是否已经实现完成
        
        if (maybe_rw->flags & RW_REALIZED) {
            // maybe_rw is rw
            
            // 如果类已经实现完成的话把 maybe_rw->ro() 返回，
            // 正是 class_ro_t 指针
            
            return maybe_rw->ro();
        } else {
            // maybe_rw is actually ro
            // 如果类是未实现完成状态的话，此时 data 函数返回的就是 class_ro_t * 
            
            return (class_ro_t *)maybe_rw;
        }
    }
    
    // 设置当前类在类的全局列表中的索引，此函数只针对于 isa 中是保存类索引的情况（目前的话大概是适用于 watchOS）
    void setClassArrayIndex(unsigned Idx) {
    
#if SUPPORT_INDEXED_ISA // 只在 isa 中保存 indexcls 时适用
        // 0 is unused as then we can rely on zero-initialisation from calloc.
        // 0 未使用，因为我们可以依靠 calloc 的零初始化。
        
        ASSERT(Idx > 0);
        
        // 设置索引
        data()->index = Idx;
#endif

    }
    
    // 获取类索引，同样仅适用于 isa 中保存 indexcls 的情况
    unsigned classArrayIndex() {
#if SUPPORT_INDEXED_ISA

        return data()->index;
        
#else

        return 0;
        
#endif
    }

    // 下面是一组和 Swift 相关的内容，都是以掩码的形式设置标识位或者读取标识位
    
    // 是否是稳定 ABI 的 Swift 类
    bool isAnySwift() {
        return isSwiftStable() || isSwiftLegacy();
    }
    
    // class is a Swift class from the stable Swift ABI
    // 类是否是稳定的 Swift ABI 的 Swift 类
    bool isSwiftStable() {
        // #define FAST_IS_SWIFT_STABLE (1UL<<1)
        // FAST_IS_SWIFT_STABLE 和 bits 做与操作，
        // 即取出 bits 的第二位标识位的值
        
        return getBit(FAST_IS_SWIFT_STABLE);
    }
    
    // 设置 FAST_IS_SWIFT_STABLE 同时清理 FAST_IS_SWIFT_LEGACY
    void setIsSwiftStable() {
        setAndClearBits(FAST_IS_SWIFT_STABLE, FAST_IS_SWIFT_LEGACY);
    }

    // class is a Swift class from the pre-stable Swift ABI
    // 类是来自稳定的 Swift ABI 的 Swift 类
    bool isSwiftLegacy() {
        // #define FAST_IS_SWIFT_LEGACY (1UL<<0)
        // FAST_IS_SWIFT_LEGACY 和 bits 做与操作，
        // 即取出 bits 的第一位标识位的值
        
        return getBit(FAST_IS_SWIFT_LEGACY);
    }
    
    // 设置 FAST_IS_SWIFT_LEGACY 同时清理 FAST_IS_SWIFT_STABLE
    void setIsSwiftLegacy() {
        setAndClearBits(FAST_IS_SWIFT_LEGACY, FAST_IS_SWIFT_STABLE);
    }

    // fixme remove this once the Swift runtime uses the stable bits
    // fixme 一旦 Swift 运行时使用稳定位将其删除
    bool isSwiftStable_ButAllowLegacyForNow() {
        return isAnySwift();
    }
    
    // 当分析 class_ro_t 时，我们再详细分析此函数
    _objc_swiftMetadataInitializer swiftMetadataInitializer() {
        // This function is called on un-realized
        // classes without holding any locks.
        // 在未实现的类上调用此函数，无需持有任何锁。
        
        // Beware of races with other realizers.
        // 当心与其它 realizers 的竞态。
        
        // 当分析 class_ro_t 时，我们再详细分析此函数
        return safe_ro()->swiftMetadataInitializer();
    }
```
&emsp;`struct class_data_bits_t` 的内容看完了，接下来我们先看 `struct class_ro_t` 的内容。

### class_ro_t
```c++
struct class_ro_t {
    // 通过掩码保存的一些标志位
    uint32_t flags;
    
    // 当父类大小发生变化时，调整子类的实例对象的大小。
    // 具体内容可查看 moveIvars 函数
    // （自己成员变量的起始偏移量，会有一些继承自父类的成员变量）
    uint32_t instanceStart;
    
    // 根据内存对齐计算成员变量从前到后所占用的内存大小，
    // 不过没有进行总体的内存对齐，例如最后一个成员变量是 char 时，
    // 则最后只是加 1，instanceSize 的值是一个奇数，
    // 再进行一个整体 8/4 字节对齐就好了，
    //（__LP64__ 平台下 8 字节对齐，其它则是 4 字节对齐）
        
    // objc_class 的 alignedInstanceSize 函数，
    // 完成了这最后一步的整体内存对齐。
        
    uint32_t instanceSize;
    
    // 仅在 64 位系统架构下的包含的保留位
#ifdef __LP64__
    uint32_t reserved;
#endif

    // 记录了哪些是 strong 的 ivar
    const uint8_t * ivarLayout;
    
    // name = 0x0000000100000f93 "LGPerson"
    // name 应该是类名
    const char * name;
    
    // 实例方法列表
    method_list_t * baseMethodList;
    // 协议列表
    protocol_list_t * baseProtocols;
    // 成员变量列表
    const ivar_list_t * ivars;
    
    // 记录了哪些是 weak 的 ivar
    const uint8_t * weakIvarLayout;
    // 属性列表
    property_list_t *baseProperties;

    // This field exists only when RO_HAS_SWIFT_INITIALIZER is set.
    // 仅当设置了 RO_HAS_SWIFT_INITIALIZER 时，此字段才存在。
    // 回调函数指针数组
    // 从 Objective-C 回调 Swift，以执行 Swift 类初始化
    _objc_swiftMetadataInitializer __ptrauth_objc_method_list_imp _swiftMetadataInitializer_NEVER_USE[0];

    // 返回 0 索引大概是用于返回起始地址
    _objc_swiftMetadataInitializer swiftMetadataInitializer() const {
        // class has ro field for Swift metadata initializer callback.
        // 类具有用于 Swift 元数据初始化程序回调的 ro 字段。
        // #define RO_HAS_SWIFT_INITIALIZER (1<<6) // flags 的第 6 位是一个标识位
        
        if (flags & RO_HAS_SWIFT_INITIALIZER) {
            return _swiftMetadataInitializer_NEVER_USE[0];
        } else {
            return nil;
        }
    }

    // 返回方法列表
    method_list_t *baseMethods() const {
        return baseMethodList;
    }
    
    // 完全复制一份 class_ro_t 
    class_ro_t *duplicate() const {
        if (flags & RO_HAS_SWIFT_INITIALIZER) {
            // class has ro field for Swift metadata initializer callback.
            // 类具有用于 Swift 元数据初始化程序回调的 ro 字段。
            // #define RO_HAS_SWIFT_INITIALIZER (1<<6)
            
            // 附加 _swiftMetadataInitializer_NEVER_USE 数组的赋值
            size_t size = sizeof(*this) + sizeof(_swiftMetadataInitializer_NEVER_USE[0]);
            class_ro_t *ro = (class_ro_t *)memdup(this, size);
            ro->_swiftMetadataInitializer_NEVER_USE[0] = this->_swiftMetadataInitializer_NEVER_USE[0];
            return ro;
        } else {
            // 直接进行内存复制
            size_t size = sizeof(*this);
            class_ro_t *ro = (class_ro_t *)memdup(this, size);
            return ro;
        }
    }
};
```
#### _objc_swiftMetadataInitializer
```c++
/** 
 * Callback from Objective-C to Swift to perform Swift class initialization.
 * 从 Objective-C 回调到 Swift 以执行 Swift 类初始化。
 */
#if !(TARGET_OS_OSX && __i386__)
typedef Class _Nullable
(*_objc_swiftMetadataInitializer)(Class _Nonnull cls, void * _Nullable arg);
#endif
```
#### memdup
```c++
static inline void *
memdup(const void *mem, size_t len)
{
    // 申请空间
    void *dup = malloc(len);
    // 复制内容
    memcpy(dup, mem, len);
    // 返回起始地址
    return dup;
}
```
#### entsize_list_tt
&emsp;在分析方法列表、成员变量列表、属性列表之前先看一个重要的数据结构 `entsize_list_tt`，它可理解为一个数据容器，拥有自己的迭代器用于遍历元素。 
（`ent` 应该是 `entry` 的缩写）
```c++
/*
* entsize_list_tt<Element, List, FlagMask>
* Generic implementation of an array of non-fragile structs.
* non-fragile 结构体数组的通用实现。
*
* 它的三个模版参数 Element、List、FlagMask 分别表示:
*
* Element is the struct type (e.g. method_t)
* Element 是结构体类型，如: method_t

* List is the specialization of entsize_list_tt (e.g. method_list_t)
* List 是 entsize_list_tt 指定类型，如: method_list_t

* FlagMask is used to stash extra bits in the entsize field
*   (e.g. method list fixup markers)
* FlagMask 用于在 entsize 字段中存储多余的位。
*（如: 方法列表被修改的标记位）
*/
template <typename Element, typename List, uint32_t FlagMask>
struct entsize_list_tt {
    // entsize（entry 的大小） 和 Flags 以掩码形式保存在 entsizeAndFlags 中
    uint32_t entsizeAndFlags;
    
    // entsize_list_tt 的容量
    uint32_t count;
    
    // 首个元素
    Element first;
    
    // method_list_t 定义中 FlagMask 的值是: 0x3
    // ivar_list_t 定义中 FlagMask 的值是: 0
    // property_list_t 定义中 FlagMask 的值是: 0
    
    // 元素的大小（entry 的大小）
    uint32_t entsize() const {
        return entsizeAndFlags & ~FlagMask;
    }
    
    // 从  entsizeAndFlags 中取出 flags
    uint32_t flags() const {
        return entsizeAndFlags & FlagMask;
    }

    // 返回指定索引的元素的的引用，orEnd 表示 i 可以等于 count，
    // 当 i 等于 count 时返回最后一个元素的后面的位置。
    Element& getOrEnd(uint32_t i) const {
        // 断言，i 不能超过 count
        ASSERT(i <= count);
        // 首先取出 first 地址（强转为 uint8_t 指针），然后指针偏移 i * ensize() 个字节长度，
        // 然后转换为 Element 指针，然后解引用取出指针指向内容作为 Element 引用返回。
        return *(Element *)((uint8_t *)&first + i*entsize()); 
    }

    // 在索引范围内返回 Element 引用
    Element& get(uint32_t i) const { 
        ASSERT(i < count);
        return getOrEnd(i);
    }
    
    // entsize_list_tt 占用的内存总大小，以字节为单位
    size_t byteSize() const {
        return byteSize(entsize(), count);
    }
    
    // entsize 单个元素的内存大小，count 是总的元素的个数
    static size_t byteSize(uint32_t entsize, uint32_t count) {
        // 首先算出 struct entsize_list_tt 的内存大小，
        // 即 uint32_t entsizeAndFlags + uint32_t count + Element first
        // 三个成员变量的总长度，然后加上元素数组的总的内存大小。
        return sizeof(entsize_list_tt) + (count-1)*entsize;
    }

    // 复制 List（method_list_t、ivar_list_t、property_list_t）
    List *duplicate() const {
        // 申请 byteSize() 个长度为 1 的连续的内存空间并置为 0
        auto *dup = (List *)calloc(this->byteSize(), 1);
        
        // 成员变量赋值
        dup->entsizeAndFlags = this->entsizeAndFlags;
        dup->count = this->count;
        
        // 从原数据的 begin() 到 end() 的内容复制到以 dup->begin() 为起始地址的空间内
        std::copy(begin(), end(), dup->begin());
        return dup;
    }
    
    // 自定义的迭代器的声明，实现在下面
    struct iterator;
    
    // 起始位置的迭代器
    const iterator begin() const {
        // static_cast 是一个 c++ 运算符，功能是把一个表达式转换为某种类型，
        // 但没有运行时类型检查来保证转换的安全性。
        // 把 this 强制转换为 const List *
        // 0 对应下面 iterator 的构造函数实现可知，
        // 把 element 指向第 1 个元素
        
        // 即返回指向容器第一个元素的迭代器
        return iterator(*static_cast<const List*>(this), 0); 
    }
    
    // 同上，少了两个 const 修饰，前面的 const 表示函数返回值为 const 不可变
    // 后面的 const 表示函数执行过程中不改变原始对象里的内容
    iterator begin() { 
        return iterator(*static_cast<const List*>(this), 0); 
    }
    
    // 返回指向 entsize_list_tt 最后一个元素的后面的迭代器，
    // 注意这里不是指向最后一个元素，而是指向最后一个的后面。
    const iterator end() const { 
        return iterator(*static_cast<const List*>(this), count); 
    }
    
    // 同上，去掉了两个 const 限制
    iterator end() { 
        return iterator(*static_cast<const List*>(this), count); 
    }
    
    // entsize_list_tt 的自定义的迭代器实现
    struct iterator {
        // 元素（entry）的大小
        uint32_t entsize;
        
        // 当前迭代器对应的索引
        uint32_t index;  // keeping track of this saves a divide in operator-
        
        // 当前迭代器对应的元素指针
        Element* element;

        // 声明类型别名
        typedef std::random_access_iterator_tag iterator_category;
        typedef Element value_type;
        typedef ptrdiff_t difference_type;
        typedef Element* pointer;
        typedef Element& reference;
        
        // 构造函数
        iterator() { }
        
        // 构造函数
        // start 默认值是 0 即 index 默认从 0 开始，element 默认指向第一个元素
        iterator(const List& list, uint32_t start = 0)
            : entsize(list.entsize())
            , index(start)
            , element(&list.getOrEnd(start))
        { }

        // 下面是一系列重载操作符
        // +=
        const iterator& operator += (ptrdiff_t delta) {
            // 指针向后偏移
            element = (Element*)((uint8_t *)element + delta*entsize);
            // 更新 index
            index += (int32_t)delta;
            // 返回 *this
            return *this;
        }
        
        // -=
        const iterator& operator -= (ptrdiff_t delta) {
            // 指针向前偏移
            element = (Element*)((uint8_t *)element - delta*entsize);
            // 更新 index
            index -= (int32_t)delta;
            // 返回 *this
            return *this;
        }
        
        // 以下都是 += 和 -= 的应用
        // +
        const iterator operator + (ptrdiff_t delta) const {
            return iterator(*this) += delta;
        }
        // -
        const iterator operator - (ptrdiff_t delta) const {
            return iterator(*this) -= delta;
        }
        
        // ++
        iterator& operator ++ () { *this += 1; return *this; }
        // --
        iterator& operator -- () { *this -= 1; return *this; }
        // ++（int）
        iterator operator ++ (int) {
            iterator result(*this); *this += 1; return result;
        }
        // --（int）
        iterator operator -- (int) {
            iterator result(*this); *this -= 1; return result;
        }
        // 两个迭代器的之间的距离
        ptrdiff_t operator - (const iterator& rhs) const {
            return (ptrdiff_t)this->index - (ptrdiff_t)rhs.index;
        }

        // 返回元素指针或引用
        // 解引用操作符
        Element& operator * () const { return *element; }
        // 
        Element* operator -> () const { return element; }
        // 
        operator Element& () const { return *element; }

        // 判等，看到的是直接比较 element 的地址
        // 哦哦，不一定是直接的地址比较，== 可能被模版抽象类型 Element 重载
        bool operator == (const iterator& rhs) const {
            return this->element == rhs.element;
        }
        
        // 不等
        bool operator != (const iterator& rhs) const {
            return this->element != rhs.element;
        }
        
        // 大概是前后位置比较
        // 小于
        bool operator < (const iterator& rhs) const {
            return this->element < rhs.element;
        }
        // 大于
        bool operator > (const iterator& rhs) const {
            return this->element > rhs.element;
        }
    };
};
```
&emsp;`entsize_list_tt` 源码全部看完了，还挺清晰的，注释极其详细，这里就不展开分析了，接下来分析 `entsize_list_tt` 的子类: `method_list_t`、`ivar_list_t`、`property_list_t`。

#### method_list_t
&emsp;`method_list_t` 是在 `class_ro_t` 中用于保存方法列表的数据结构。它继承自 `entsize_list_tt`。
```c++
// Two bits of entsize are used for fixup markers.
// entsize 的后两位用于固定标记。
struct method_list_t : entsize_list_tt<method_t, method_list_t, 0x3> {
    bool isUniqued() const;
    bool isFixedUp() const;
    void setFixedUp();

    // 返回在 method_list_t 中指定的 meth 的 index
    //（指针距离除以元素宽度）
    uint32_t indexOfMethod(const method_t *meth) const {
        uint32_t i = (uint32_t)(((uintptr_t)meth - (uintptr_t)this) / entsize());
        ASSERT(i < count);
        return i;
    }
};
```
#### method_t
&emsp;方法的数据结构。
```c++
struct method_t {
    SEL name; // 方法名、选择子
    const char *types; // 方法类型
    
    // using MethodListIMP = IMP;
    MethodListIMP imp; // 方法实现

    // 根据选择子的地址进行排序
    struct SortBySELAddress :
        public std::binary_function<const method_t&, const method_t&, bool>
    {
        bool operator() (const method_t& lhs, const method_t& rhs)
        { return lhs.name < rhs.name; }
    };
};
```
可参考 [stl 中 std::binary_function 的使用](https://blog.csdn.net/tangaowen/article/details/7547475)

#### isUniqued、isFixedUp、setFixedUp
&emsp;三个函数主要涉及到 `method_list_t` 的 `flags` 标志位。
```c++
/*
Low two bits of mlist->entsize is used as the fixed-up marker.
mlist->entsize（其实是 entsizeAndFlags）的低两位用作固定标记。

PREOPTIMIZED VERSION（预优化版本）:

  Method lists from shared cache are 1 (uniqued) or 3 (uniqued and sorted).
  来自 shared cache 中的 Method lists 是 1(唯一) 或 3 (唯一且排序)。
  
  Runtime fixed-up method lists get 3.
  Runtime 固定方法列表得到 3。
  (这里是指 method_list_t 定义中，继承 entsize_list_tt 的模版参数 FlagMask 传递的是 0x3 吗 ？)
  
UN-PREOPTIMIZED VERSION（未优化版本）:

  Method lists from shared cache are 1 (uniqued) or 3 (uniqued and sorted).
  来自 shared cache 中的 Method lists 是 1(唯一) 或 3 (唯一且排序)。
  
  Shared cache's sorting and uniquing are not trusted,
  but do affect the location of the selector name string.
  Shared cache 的排序和唯一性不受信任，但会影响 选择器名称字符串的 位置。
  
  Runtime fixed-up method lists get 2.
  运行时固定方法列表得到 2。
  ...
  下面一部分注释是描述 protocol->flags 的，等分析 protocol_list_t 时再进行解读。
*/

// 静态全局变量 fixed_up_method_list 值为 3
static uint32_t fixed_up_method_list = 3;
// 静态全局变量 uniqued_method_list 值为 1
static uint32_t uniqued_method_list = 1;

// 使 Shared Cache 优化无效 
void
disableSharedCacheOptimizations(void)
{
    // fixed_up_method_list 置为 2
    fixed_up_method_list = 2;
    
    // It is safe to set uniqued method lists to 0 as we'll never call it
    // unless the method list was already in need of being fixed up.
    // 将 uniqued_method_list 设置为 0 是安全的，因为除非已经需要修复方法列表，
    // 否则我们永远都不会调用它。
    uniqued_method_list = 0;
    
    // protocol_list_t 运用
    fixed_up_protocol = PROTOCOL_FIXED_UP_1 | PROTOCOL_FIXED_UP_2;
    // Its safe to just set canonical protocol to 0 as we'll never call
    // clearIsCanonical() unless isCanonical() returned true, which can't happen
    // with a 0 mask
    canonical_protocol = 0;
}

bool method_list_t::isUniqued() const {
    // 判断 flags 是否和当前的 uniqued_method_list 相同，
    // 如果相等则返回 true，都否则返回 false。
    return (flags() & uniqued_method_list) != 0;
}

bool method_list_t::isFixedUp() const {
    // 判断 flags 是否和当前的 fixed_up_method_list 相等
    return flags() == fixed_up_method_list;
}

// 设置 method_list_t 的 entsizeAndFlags  
void method_list_t::setFixedUp() {
    // 加锁，加锁失败则执行断言
    runtimeLock.assertLocked();
    
    ASSERT(!isFixedUp());
    
    // entsize(): entsizeAndFlags & ~FlagMask 除了低两位其它位保持不变
    entsizeAndFlags = entsize() | fixed_up_method_list;
}
```
#### ivar_list_t
&emsp;`ivr_list_t` 是 `class_ro_t` 中保存成员变量的数据结构，同样继承自 `entsize_list_tt`，`FlagMask` 模版参数是 `0`。
```c++
struct ivar_list_t : entsize_list_tt<ivar_t, ivar_list_t, 0> {
    // 判断当前的 ivar_list_t 中是否包含指定的 ivar
    bool containsIvar(Ivar ivar) const {
        // 地址比较
        
        // 迭代器的 * 已被重载如下:
        // Element& operator * () const { return *element; }
        
        // 所以 *begin() 返回的是 ivar_t
        // typedef struct ivar_t *Ivar; // 入参是 Ivar，Ivar 是 ivar_t 指针
        // 所以这里要加一个 & 取出 ivar_t 地址即表示 Ivar。   
        
        return (ivar >= (Ivar)&*begin()  &&  ivar < (Ivar)&*end());
    }
};
```

#### ivar_t
```c++
struct ivar_t {

#if __x86_64__
    // *offset was originally 64-bit on some x86_64 platforms.
    // *offset 最初在某些 x86_64 平台上为 64 位。
    
    // We read and write only 32 bits of it.
    // 我们只读取和写入 32 位
    
    // Some metadata provides all 64 bits. This is harmless for unsigned
    // little-endian values.
    // 一些元数据提供所有 64 位。这对于无符号的 Little-endian (小端存储) 值无害。
    
    // Some code uses all 64 bits. class_addIvar() over-allocates the
    // offset for their benefit.
    // 一些代码使用所有 64 位。class_addIvar() 为他们的利益过度分配了偏移量。
#endif

    // 首先这里要和结构体中成员变量的偏移距离做出理解上的区别。

    // offset 它是一个指针，那它指向谁呢，它指向一个全局变量，
    // 编译器为每个类的每个成员变量都分配了一个全局变量，用于存储该成员变量的偏移值。

    // 如果 offset 仅是成员变量偏移距离的一个整数的话，能想到的成员变量的读取可能是这样的：
    // 当我们读取成员变量时从实例对象的 isa 找到类，然后 data() 找到 class_rw_t 
    // 然后在找到 class_ro_t 然后再找到 ivar_list_t，
    // 然后再比较 ivar_t 的 name 和我们要访问的成员变量的名字是否相同，然后再读出 int 类型的 offset,
    // 再进行 self + offset 指针偏移找到这个成员变量。

    // 现在当我们访问一个成员变量时，只需要 self + *offset 就可以了。
    // 下面会单独讲解 offset 指针。
    
    int32_t *offset;
    
    // 成员变量名称（如果类中有属性的话，编译器会自动生成 _属性名 的成员变量）
    const char *name;
    
    // 成员变量类型
    const char *type;
    
    // alignment is sometimes -1; use alignment() instead
    // 对齐有时为 -1，使用 alignment() 代替。
    
    // 原始对齐值是 2^alignment_raw 的值
    // alignment_raw 应该叫做对齐值的指数
    uint32_t alignment_raw;
    
    // 成员变量的类型的大小
    uint32_t size;

    // #ifdef __LP64__
    // #   define WORD_SHIFT 3UL
    // #else
    // #   define WORD_SHIFT 2UL
    // #endif
    
    uint32_t alignment() const {
    
        // 应该没有类型的 alignment_raw 会是 uint32_t 类型的最大值吧！
        // WORD_SHIFT 在 __LP64__ 下是 3，表示 8 字节对齐，
        // 在非 __LP64__ 下是 2^2 = 4 字节对齐。
        if (alignment_raw == ~(uint32_t)0) return 1U << WORD_SHIFT;
        
        // 2^alignment_raw 
        return 1 << alignment_raw;
    }
};
```

##### 验证 int32_t *offset
&emsp;我们首先定义一个 `LGPerson` 类：
```c++
// 定义一个 LGPerson 类
// LGPerson.h 如下，.m 为空即可
@interface LGPerson : NSObject
@property (nonatomic, strong) NSMutableArray *arr;
@end
```
&emsp;然后在终端执行 `clang -rewrite-objc LGPerson.m` 生成 `LGPerson.cpp`。 
&emsp;摘录 `LGPerson.cpp`:
&emsp;`ivar_list_t` 如下，`arr` 为仅有的成员变量，它对应的 `ivar_t` 初始化部分 `int32_t *offset` 值使用了 `(unsigned long int *)&OBJC_IVAR_$_LGPerson$_arr`。
```c++
static struct /*_ivar_list_t*/ {
    unsigned int entsize;  // sizeof(struct _prop_t)
    unsigned int count;
    struct _ivar_t ivar_list[1];
} _OBJC_$_INSTANCE_VARIABLES_LGPerson __attribute__ ((used, section ("__DATA,__objc_const"))) = {
    sizeof(_ivar_t),
    1,
    {{(unsigned long int *)&OBJC_IVAR_$_LGPerson$_arr, "_arr", "@\"NSMutableArray\"", 3, 8}}
};
```
&emsp;在 `LGPerson.cpp` 文件中全局搜索 `OBJC_IVAR_$_LGPerson$_arr` 有如下结果:
```c++
// 全局变量声明和定义赋值
extern "C" unsigned long OBJC_IVAR_$_LGPerson$_arr;
extern "C" unsigned long int 
           OBJC_IVAR_$_LGPerson$_arr 
           __attribute__ ((used, section ("__DATA,__objc_ivar"))) = 
           __OFFSETOFIVAR__(struct LGPerson, _arr);

// arr 的 setter getter 函数，看到都是直接使用了 OBJC_IVAR_$_LGPerson$_arr
static NSMutableArray * _Nonnull _I_LGPerson_arr(LGPerson * self, SEL _cmd) { 
    return (*(NSMutableArray * _Nonnull *)((char *)self + OBJC_IVAR_$_LGPerson$_arr)); 
}

static void _I_LGPerson_setArr_(LGPerson * self, SEL _cmd, NSMutableArray * _Nonnull arr) { 
    (*(NSMutableArray * _Nonnull *)((char *)self + OBJC_IVAR_$_LGPerson$_arr)) = arr; 
}
```
#### property_list_t
&emsp;`property_list_t` 是 `class_ro_t` 中保存属性的数据结构，同样继承自 `entsize_list_tt`，`FlagMask` 模版参数是 `0`。
```c++
struct property_list_t : entsize_list_tt<property_t, property_list_t, 0> {
};
```
#### property_t
&emsp;看到 `property_t` 极其简单，编译器会自动生成一个对应的 _属性名的成员变量保存在 `ivars` 中。
```c++
struct property_t {
    // 属性名字
    const char *name;
    
    // 属性的 attributes，例如：
    
    // @property (nonatomic, strong) NSObject *object;
    // object 的 attributes："T@\"NSObject\",&,N,V_object"
    //
    // @property (nonatomic, copy) NSArray *array2;
    // array2 的 attributes："T@\"NSArray\",C,N,V_array2"
    // 
    
    const char *attributes;
};
```
#### protocol_ref_t
```c++
// 实际用于 protocol_t 的指针
typedef uintptr_t protocol_ref_t;  // protocol_t *, but unremapped
```
#### protocol_list_t
```c++
struct protocol_list_t {
    // count is pointer-sized by accident.
    
    // list 的长度
    uintptr_t count;
    
    // 实际是 protocol_t 数组
    protocol_ref_t list[0]; // variable-size

    // protocol_list_t 所占用的内存大小
    size_t byteSize() const {
        return sizeof(*this) + count*sizeof(list[0]);
    }
    
    // 完全的内存空间进行复制
    protocol_list_t *duplicate() const {
        return (protocol_list_t *)memdup(this, this->byteSize());
    }
    
    // 迭代器和 const 迭代器
    typedef protocol_ref_t* iterator;
    typedef const protocol_ref_t* const_iterator;
    
    // 后面的 const 表示不会改变 protocol_list_t 内的数据
    const_iterator begin() const {
        return list;
    }
    
    iterator begin() {
        return list;
    }
    
    // list 最后一个元素的后面。
    const_iterator end() const {
        return list + count;
    }
    
    iterator end() {
        return list + count;
    }
};
```
#### protocol_t
```c++
struct protocol_t : objc_object {
    // 协议名
    const char *mangledName;
    
    // 所属的还是所继承的协议
    // 例如定义了一个：
    // @protocol CustomProtocol <NSObject>
    // ...
    // @end
    // 然后通过层层打印发现 CustomProtocol 的 protocols 是 NSObject
    struct protocol_list_t *protocols;
    
    // 协议中的实例方法
    method_list_t *instanceMethods;
    
    // 协议中的类方法
    method_list_t *classMethods;
    
    // 协议中的可选的实例方法
    method_list_t *optionalInstanceMethods;
    
    // 协议中可选的类方法
    method_list_t *optionalClassMethods;
    
    // 协议中声明的属性
    property_list_t *instanceProperties;
    
    // 这个 size 大概是整个 protocol_t 内容大小
    // 协议中声明了 一个实例方法一个类方法
    // 一个可选的实例方法一个可选的类方法，外加一个 NSObject 类型的属性
    // 打印时，size 的值是 96
    uint32_t size;   // sizeof(protocol_t)
    
    uint32_t flags;
    
    // Fields below this point are not always present on disk.
    // 低于此点的字段并不总是存在于磁盘上。
    
    const char **_extendedMethodTypes;
    
    const char *_demangledName;
    property_list_t *_classProperties;

    const char *demangledName();

    // 协议名
    const char *nameForLogging() {
        return demangledName();
    }

    // 下面一部分内容等详细分析 protocol 时再解析。
    bool isFixedUp() const;
    void setFixedUp();

    bool isCanonical() const;
    void clearIsCanonical();

#   define HAS_FIELD(f) (size >= offsetof(protocol_t, f) + sizeof(f))

    bool hasExtendedMethodTypesField() const {
        return HAS_FIELD(_extendedMethodTypes);
    }
    bool hasDemangledNameField() const {
        return HAS_FIELD(_demangledName);
    }
    bool hasClassPropertiesField() const {
        return HAS_FIELD(_classProperties);
    }

#   undef HAS_FIELD

    const char **extendedMethodTypes() const {
        return hasExtendedMethodTypesField() ? _extendedMethodTypes : nil;
    }

    property_list_t *classProperties() const {
        return hasClassPropertiesField() ? _classProperties : nil;
    }
};
```
&emsp;到这里 `class_ro_t` 的内容就看完了，都是一些简单的数据结构，下篇我们开始分析 `class_rw_t`。

## 参考链接
**参考链接:🔗**
+ [iOS之LLDB常用命令](https://juejin.im/post/6869621360415637518)
