# iOS 从源码解析Runtime (十二)：聚焦objc_class(class_rw_t相关内容篇)

> &emsp;上篇我们详细分析了 `class_ro_t`，其中可能有点模糊的是  `class_data_bits_t bits` 的 `data` 函数和 `safe_ro` 函数，其中如果类是处于未实现完成（`RW_REALIZED`）状态时 `data` 函数返回的是 `class_ro_t *`，当类实现完成后返回的则是 `class_rw_t *`，且当类实现完成以后 `class_rw_t` 有一个 `ro` 函数来返回 `class_ro_t *`，那这是怎么回事呢，这篇我们就来详细分析一下。 ⛽️⛽️

## class_rw_t
&emsp;`class_rw_t` 的成员变量。
```c++
struct class_rw_t {
    // Be warned that Symbolication knows the layout of this structure.
    // 请注意，Symbolication 知道此结构的布局。
    
    // 在上篇的测试代码中：flags 打印看到是 2148007936
    // 转为二进制的话是只有 31 位和 19 位是 1，其它位全部都是 0，对应于:
    // class has started realizing but not yet completed it
    // #define RW_REALIZING (1<<19)
    // class_t->data is class_rw_t, not class_ro_t
    // #define RW_REALIZED (1<<31)
    
    uint32_t flags;
    
    //（控制台打印值为 1）
    uint16_t witness;
    
#if SUPPORT_INDEXED_ISA // isa 中保存 indexcls，大概是 watchOS 下才会用到
    uint16_t index;
#endif

    // std::atomic<uintptr_t>
    // 原子性 unsigned long
    
    // 执行如下命令，会打印 error:
    // (lldb) p $2->ro_or_rw_ext
    // error: no member named 'ro_or_rw_ext' in 'class_rw_t'
    
    // 在编译时会根据类定义生成类的 class_ro_t 数据，其中包含方法列表、属性列表、成员变量列表等等内容
    
    // ro_or_rw_ext 会有两种情况：
    // 1): 编译时值是 class_ro_t *
    // 2): 编译后类实现完成后值是 class_rw_ext_t *，而编译时的 class_ro_t * 作为 class_rw_ext_t 的 const class_ro_t *ro 成员变量保存
    explicit_atomic<uintptr_t> ro_or_rw_ext; // 变量名字对应与 class_ro_t or(或) class_rw_ext_t

    // 当前所属类的第一个子类
    // 测试时，定义了一个继承自 NSObject 的类，
    // 控制台打印看到它的 firstSubclass 是 nil
    Class firstSubclass;
    
    // 姊妹类、兄弟类
    // 测试时，定义了一个继承自 NSObject 的类，
    // 控制台打印看到 nextSiblingClass 是 NSUUID（好奇怪）
    
    // firstSubclass 和 nextSiblingClass 有超级重要的作用，后面会展开
    Class nextSiblingClass;
    ...
};
```
### class_rw_ext_t
```c++
struct class_rw_ext_t {
    // 特别关注 ro 这个成员变量。
    // 这个即是在类实现完成后，class_rw_t 中存放的编译器生成的 class_ro_t 数据。
    const class_ro_t *ro;
    
    // 在上一节 class_ro_t 中的：
    // 方法列表、属性列表、成员变量列表、协议列表的类型如下:
    // struct method_list_t : entsize_list_tt<method_t, method_list_t, 0x3>
    // struct property_list_t : entsize_list_tt<property_t, property_list_t, 0>
    // struct ivar_list_t : entsize_list_tt<ivar_t, ivar_list_t, 0>
    // struct protocol_list_t 
    
    // 到 class_rw_t 中就变为了:
    // class method_array_t : public list_array_tt<method_t, method_list_t>
    // class property_array_t : public list_array_tt<property_t, property_list_t>
    // class protocol_array_t : public list_array_tt<protocol_ref_t, protocol_list_t>
    
    // 这里先不着急，等下会详细分析它们所使用的新的数据结构: list_array_tt。
    
    // 方法列表
    method_array_t methods;
    // 属性列表
    property_array_t properties;
    // 协议列表
    protocol_array_t protocols;
    
    // 所属的类名
    char *demangledName;
    // 版本号
    uint32_t version;
};
```
### class_rw_t private 部分
&emsp;这里先分析一下 `class_rw_t` 的 `private` 部分。
```c++
struct class_rw_t {
    ...
private:
    // 使用 using 关键字声明一个 ro_or_rw_ext_t 类型:
    // objc::PointerUnion<const class_ro_t *, class_rw_ext_t *>
    //（可理解为一个指针联合体，系统只为其分配一个指针的内存空间，
    // 一次只能保存 class_ro_t 指针或者 class_rw_ext_t 指针）
    
    // 此时会发现 class_rw_t 一些端倪了。
    // 在 class_ro_t 中它是直接定义不同的成员变量来保存数据，
    // 而在 class_rw_t 中，它大概是用了一个中间人 struct class_rw_ext_t 来保存相关的数据。
    
    // 这里的数据存储根据类是否已经完成实现而分为两种情况：
    // 1): 类未实现完成时，ro_or_rw_ext 中存储的是 class_ro_t *
    // 2): 类已完成实现时，ro_or_rw_ext 中存储的是 class_rw_ext_t *，
    //     而 class_ro_t * 存储在 class_rw_ext_t 的 const class_ro_t *ro 成员变量中。
    
    // 类的 class_ro_t 类型的数据是在编译时就产生了。🌿
    
    using ro_or_rw_ext_t = objc::PointerUnion<const class_ro_t *, class_rw_ext_t *>;

    // 根据 ro_or_rw_ext 获得 ro_or_rw_ext_t 类型的值。
    //（可能是 class_ro_t * 或者 class_rw_ext_t *）
    const ro_or_rw_ext_t get_ro_or_rwe() const {
        return ro_or_rw_ext_t{ro_or_rw_ext};
    }
    
    // 以原子方式把入参 const class_ro_t *ro 保存到 ro_or_rw_ext 中
    void set_ro_or_rwe(const class_ro_t *ro) {
        ro_or_rw_ext_t{ro}.storeAt(ro_or_rw_ext, memory_order_relaxed);
    }
    
    // 先把入参 const class_ro_t *ro 赋值给入参 class_rw_ext_t *rwe 的 const class_ro_t *ro，
    // 然后以原子方式把入参 class_rw_ext_t *rwe 保存到 ro_or_rw_ext 中    
    void set_ro_or_rwe(class_rw_ext_t *rwe, const class_ro_t *ro) {
        // the release barrier is so that the class_rw_ext_t::ro 
        // initialization is visible to lockless readers.
        
        // 赋值
        rwe->ro = ro;
        // 原子方式保存
        ro_or_rw_ext_t{rwe}.storeAt(ro_or_rw_ext, memory_order_release);
    }
    
    // 此处仅声明 extAlloc 函数
    //（此函数的功能是进行 class_rw_ext_t 的初始化）
    class_rw_ext_t *extAlloc(const class_ro_t *ro, bool deep = false);
    
    // extAlloc 定义位于 objc-runtime-new.mm 中，主要完成 class_rw_ext_t 变量的创建，
    // 以及把其保存在 class_rw_t 的 ro_or_rw_ext 中。
    class_rw_ext_t *
    class_rw_t::extAlloc(const class_ro_t *ro, bool deepCopy)
    {
        // 加锁
        runtimeLock.assertLocked();
        
        // 申请空间
        auto rwe = objc::zalloc<class_rw_ext_t>();
        
        // class is a metaclass
        // #define RO_META (1<<0)
        // 标识是否是元类，如果是元类，则 version 是 7 否则是 0
        rwe->version = (ro->flags & RO_META) ? 7 : 0;

        // 把 ro 中的方法列表追加到 rw（class_rw_ext_t） 中
        //（attachLists 函数等下在分析 list_array_tt 时再进行详细分析）
        method_list_t *list = ro->baseMethods();
        if (list) {
        
            // 是否需要对 ro 的方法列表进行深拷贝，默认是 false
            if (deepCopy) list = list->duplicate();
            
            // 把 ro 的方法列表追加到 rwe 的方法列表中
            //（attachLists 函数在分析 list_array_tt 时再进行详细分析）
            //（注意 rwe->methods 的有两种形态，可能是指向单个列表的指针，
            // 或者是指向列表的指针数组（数组中放的是列表的指针））
            rwe->methods.attachLists(&list, 1);
        }

        // See comments in objc_duplicateClass property lists and
        // protocol lists historically have not been deep-copied.
        // 请参阅 objc_duplicateClass 属性列表和协议列表中的注释，历史上尚未进行过深度复制。
        
        // This is probably wrong and ought to be fixed some day.
        // 这可能是错误的，可能会在某天修改。
        
        // 把 ro 中的属性列表追加到 rw（class_rw_ext_t）中
        property_list_t *proplist = ro->baseProperties;
        if (proplist) {
            rwe->properties.attachLists(&proplist, 1);
        }

        // 把 ro 中的协议列表追加到 rw（class_rw_ext_t） 中
        protocol_list_t *protolist = ro->baseProtocols;
        if (protolist) {
            rwe->protocols.attachLists(&protolist, 1);
        }
        
        // 把 ro 赋值给 rw 的 const class_ro_t *ro，
        // 并以原子方式把 rw 存储到 class_rw_t 的 explicit_atomic<uintptr_t> ro_or_rw_ext 中
        set_ro_or_rwe(rwe, ro);
        
        // 返回 class_rw_ext_t *
        return rwe;
    }
    ...
};
```

#### PointerUnion
&emsp;这里分析模版类 `objc::PointerUnion` 基于 `objc::PointerUnion<const class_ro_t *, class_rw_ext_t *>` 来进行。其中 `PT1` 是 `const class_ro_t *`（并且加了 `const`，表示 `class_ro_t` 内容不可被修改），`PT2` 是 `class_rw_ext_t *`。

```c++
// PT1: const class_ro_t *
// PT2: class_rw_ext_t *

template <class PT1, class PT2>
class PointerUnion {
    // 仅有一个成员变量 _value，
    // 只能保存 const class_ro_t * 或 class_rw_ext_t * 
    uintptr_t _value;

    // 两个断言，PT1 和 PT2 内存对齐
    static_assert(alignof(PT1) >= 2, "alignment requirement");
    static_assert(alignof(PT2) >= 2, "alignment requirement");

    // 定义结构体 IsPT1，内部仅有一个静态不可变 uintptr_t 类型的值为 0 的 Num。
    //（用于 _value 的类型判断, 表示此时是 class_ro_t *）
    struct IsPT1 {
      static const uintptr_t Num = 0;
    };
    
    // 定义结构体 IsPT2，内部仅有一个静态不可变 uintptr_t 类型的值为 1 的 Num。
    //（用于 _value 的类型判断，表示此时是 class_rw_ext_t *）
    struct IsPT2 {
      static const uintptr_t Num = 1;
    };
    
    template <typename T> struct UNION_DOESNT_CONTAIN_TYPE {};

    // 把 _value 最后一位置为 0 其它位保持不变的值 返回
    uintptr_t getPointer() const {
        return _value & ~1;
    }
    
    // 返回 _value 最后一位的值
    uintptr_t getTag() const {
        return _value & 1;
    }

public:
    // PointerUnion 的构造函数
    // 初始化列表原子操作，初始化 _value
    explicit PointerUnion(const std::atomic<uintptr_t> &raw)
    : _value(raw.load(std::memory_order_relaxed))
    { }
    
    // PT1 正常初始化
    PointerUnion(PT1 t) : _value((uintptr_t)t) { }
    
    // PT2 初始化时把 _value 的最后一位置为 1 
    PointerUnion(PT2 t) : _value((uintptr_t)t | 1) { }

    // 根据指定的 order 以原子方式把 raw 保存到 _value 中
    void storeAt(std::atomic<uintptr_t> &raw, std::memory_order order) const {
        raw.store(_value, order);
    }

    // 极重要的函数，在 class_rw_t 中判断 ro_or_rw_ext 当前是 class_rw_ext_t * 还是 class_ro_t *
    // is 函数在 class_rw_t 中调用时 T 使用的都是 class_rw_ext_t *，当 P1 和 P2 分别对应: 
    // const class_ro_t *, class_rw_ext_t * 时，Ty 可以转化为如下:
    // using Ty = typename 
    //            PointerUnionTypeSelector<const class_ro_t *,
                                           class_rw_ext_t *,
                                           IsPT1,
                                                    
                                           PointerUnionTypeSelector<class_rw_ext_t *,
                                                                    class_rw_ext_t *,
                                                                    IsPT2,
                                                                    UNION_DOESNT_CONTAIN_TYPE
                                                                    <class_rw_ext_t *>>
                                           >::Return;
    // (如果 T1 与 T2 相同，则 Ret 为 EQ 类型，否则为 NE 类型)
    // 如上，PointerUnionTypeSelector 的第四个模版参数 RET_NE 是：
    
    // PointerUnionTypeSelector<class_rw_ext_t *,
                                class_rw_ext_t *,
                                IsPT2,
                                UNION_DOESNT_CONTAIN_TYPE<class_rw_ext_t *>>
                                
    // 然后再执行一次比较，返回是 IsPT2，
    // IsPT2::Num 是 1，
    // getTag() 函数取 _value 第 1 位的值是 1 或者 0，
    // 根据 PointerUnion 的构造函数：PointerUnion(PT2 t) : _value((uintptr_t)t | 1) { }，
    // 可知当 _value 是 class_rw_ext_t * 时，_value 第 1 位是 1，
    // 即当 getTag() == Ty::Num 为真时，表示 _value 是 class_rw_ext_t *
    
    template <typename T>
    bool is() const {
        using Ty = typename PointerUnionTypeSelector<PT1, T, IsPT1, 
        PointerUnionTypeSelector<PT2, T, IsPT2, UNION_DOESNT_CONTAIN_TYPE<T>>>::Return;
        return getTag() == Ty::Num;
    }
    
    // 获取指针 class_ro_t 或者 class_rw_ext_t 指针
    template <typename T> T get() const {
        
      // 确保当前的类型和 T 是匹配的
      ASSERT(is<T>() && "Invalid accessor called");
      
      // getPointer 函数会把 _value 末尾置回 0
      return reinterpret_cast<T>(getPointer());
    }

    // 几乎同上，但是加了一层判断逻辑，
    // get 函数中如果当前 _value 类型和 T 不匹配的话，强制转换会返回错误类型的指针
    // dyn_cast 则始终都返回 T 类型的指针
    template <typename T> T dyn_cast() const {
      // 如果 T 和当前实际类型对应，则直接返回
      if (is<T>())
        return get<T>();
      
      // 否则返回 T 类型的值（调用了 T 类型的构造函数）
      return T();
    }
};
```

#### PointerUnionTypeSelector
```c++
// Ret will be EQ type if T1 is same as T2 or NE type otherwise.
// 如果 T1 与 T2 相同，则 Ret 为 EQ 类型，否则为 NE 类型。
template <typename T1, typename T2, typename RET_EQ, typename RET_NE>
struct PointerUnionTypeSelector {
  using Return = typename PointerUnionTypeSelectorReturn<RET_NE>::Return;
};

template <typename T> struct PointerUnionTypeSelectorReturn {
  using Return = T;
};
```

### class_rw_t public 部分
&emsp;分析 `class_rw_t` 的 `public` 部分。
```c++
struct class_rw_t {
...
public:
    // 以原子方式进行或操作设置 flags 指定位为 1。
    // 通过或操作把 set 中值是 1 的位同样设置到 flags 中，
    // 同时 flags 中值为 1 的位会保持原值。
    void setFlags(uint32_t set)
    {
        __c11_atomic_fetch_or((_Atomic(uint32_t) *)&flags, set, __ATOMIC_RELAXED);
    }

    // 以原子方式进行与操作设置 flags 指定位为 0。
    // 通过与操作把 ~clear 中值是 0 的位同样设置到 flags 中，
    // 同时 flags 中的其它位保持原值。
    void clearFlags(uint32_t clear) 
    {
        __c11_atomic_fetch_and((_Atomic(uint32_t) *)&flags, ~clear, __ATOMIC_RELAXED);
    }

    // set and clear must not overlap
    // 设置和清除不得重叠
    
    // 首先把 set 位置为 1，然后再进行 ~clear
    void changeFlags(uint32_t set, uint32_t clear) 
    {
        ASSERT((set & clear) == 0);

        uint32_t oldf, newf;
        do {
            oldf = flags;
            newf = (oldf | set) & ~clear;
            
        // bool OSAtomicCompareAndSwap32Barrier(int32_t __oldValue, 
        //                                      int32_t __newValue,
        //                                      volatile int32_t *__theValue );
        // 比较 __oldValue 是否与 __theValue 指针指向的内存位置的值匹配，如果匹配，
        // 则将 __newValue 的值存储到 __theValue 指向的内存位置，
        // 同时匹配时返回 true，否则返回 false。
        
        // 此 do while 循环只为保证 newf 的值正确保存到 flags 中。
        
        } while (!OSAtomicCompareAndSwap32Barrier(oldf, newf, (volatile int32_t *)&flags));
    }

    // 从 ro_or_rw_ext 中取得 class_rw_ext_t 指针
    class_rw_ext_t *ext() const {
        return get_ro_or_rwe().dyn_cast<class_rw_ext_t *>();
    }

    // 由 class_ro_t 构建一个 class_rw_ext_t，
    // 如果目前 ro_or_rw_ext 已经是 class_rw_ext_t 指针了，则直接返回
    // 如果目前 ro_or_rw_ext 是 class_ro_t 指针的话，
    // 根据 class_ro_t 的值构建 class_rw_ext_t 并把它的地址赋值给 class_rw_t 的 ro_or_rw_ext，
    // 且最后返回 class_rw_ext_t 指针。
    
    class_rw_ext_t *extAllocIfNeeded() {
        auto v = get_ro_or_rwe();
        if (fastpath(v.is<class_rw_ext_t *>())) {
            // 直接返回 class_rw_ext_t 指针
            return v.get<class_rw_ext_t *>();
        } else {
        
            // 构建 class_rw_ext_t 
            return extAlloc(v.get<const class_ro_t *>());
        }
    }

    // extAlloc 中 deepCopy 传递 true，ro 的方法列表会执行复制操作，
    
    // method_list_t *list = ro->baseMethods();
    // if (deepCopy) list = list->duplicate();
    
    // 且 rw 的 methods 中追加的是新复制的这份方法列表。
    
    class_rw_ext_t *deepCopy(const class_ro_t *ro) {
        return extAlloc(ro, true);
    }

    // 从 ro_or_rw_ext 中取得 class_ro_t 指针，
    // 如果此时 ro_or_rw_ext 中存放的是 class_rw_ext_t 指针，
    // 则返回 class_rw_ext_t 中的 const class_ro_t *ro
    const class_ro_t *ro() const {
        // 取得一个指针
        auto v = get_ro_or_rwe();
        
        if (slowpath(v.is<class_rw_ext_t *>())) {
            // 如果此时是 class_rw_ext_t 指针，则返回它的 ro
            return v.get<class_rw_ext_t *>()->ro;
        }
        
        // 如果此时正是 class_ro_t，则直接返回
        return v.get<const class_ro_t *>();
    }

    // 设置 ro，如果当前 ro_or_rw_ext 中保存的是 class_rw_ext_t 指针，
    // 则把 ro 赋值给 class_rw_ext_t 的 const class_ro_t *ro。
    
    // 如果此时 ro_or_rw_ext 中保存的是 class_ro_t *ro 的话，
    // 则以原子方式把入参 ro 保存到 ro_or_rw_ext 中。
    
    void set_ro(const class_ro_t *ro) {
        auto v = get_ro_or_rwe();
        if (v.is<class_rw_ext_t *>()) {
            v.get<class_rw_ext_t *>()->ro = ro;
        } else {
            set_ro_or_rwe(ro);
        }
    }

    // 方法列表获取
    // 1): class_rw_ext_t 的 method_array_t methods
    // 2): class_ro_t 的 method_list_t * baseMethodList 构建的 method_array_t
    
    const method_array_t methods() const {
        auto v = get_ro_or_rwe();
        if (v.is<class_rw_ext_t *>()) {
            return v.get<class_rw_ext_t *>()->methods;
        } else {
            return method_array_t{v.get<const class_ro_t *>()->baseMethods()};
        }
    }

    // 属性列表获取
    // 同上（class_ro_t 和 class_rw_ext_t）
    
    const property_array_t properties() const {
        auto v = get_ro_or_rwe();
        if (v.is<class_rw_ext_t *>()) {
            return v.get<class_rw_ext_t *>()->properties;
        } else {
            return property_array_t{v.get<const class_ro_t *>()->baseProperties};
        }
    }

    // 协议列表获取
    // 同上（class_ro_t 和 class_rw_ext_t）
    
    const protocol_array_t protocols() const {
        auto v = get_ro_or_rwe();
        if (v.is<class_rw_ext_t *>()) {
            return v.get<class_rw_ext_t *>()->protocols;
        } else {
            return protocol_array_t{v.get<const class_ro_t *>()->baseProtocols};
        }
    }
};
```
### list_array_tt
&emsp;下面我们来分析最后 `4` 个数据结构，`method_array_t`、`property_array_t`、`protocol_array_t` 和它们的父类 `list_array_tt`。
```c++
/*
* list_array_tt<Element, List>
* Generic implementation for metadata that can be augmented by categories.
* 可以按类别扩展的元数据的通用实现。
*
* 实际应用：
* 1. class method_array_t : public list_array_tt<method_t, method_list_t>
* 2. class property_array_t : public list_array_tt<property_t, property_list_t>
* 3. class protocol_array_t : public list_array_tt<protocol_ref_t, protocol_list_t>
*
* Element is the underlying metadata type (e.g. method_t)
* Element 是基础元数据类型（例如: method_t）
*
* List is the metadata's list type (e.g. method_list_t)
* List 是元数据的列表类型（例如: method_list_t）
* 
* A list_array_tt has one of three values:
* list_array_tt 具有如下三个值之一：
* 
* - empty（空）
* - a pointer to a single list（指向单个列表的指针）(array_t *)
* - an array of pointers to lists（指向列表的指针数组）(List* lists[0] 元素是 List * 的数组)
*
* countLists/beginLists/endLists iterate the metadata lists.
* countLists/beginLists/endLists 迭代元数据列表。
*
* count/begin/end iterate the underlying metadata elements.
* count/begin/end 迭代基础元数据元素。
*/
```
#### array_t
```c++
struct array_t {
    // count 是 lists 数组中 List * 的数量
    uint32_t count;
    
    // 数组中的元素是 List * (实际类型是 entsize_list_tt *)
    List* lists[0]; // 这里使用了 0 长度
    
    // 所占用的字节总数
    static size_t byteSize(uint32_t count) {
        return sizeof(array_t) + count*sizeof(lists[0]);
    }
    
    // 根据 count 计算所有的字节总数
    size_t byteSize() {
        return byteSize(count);
    }
};
```
#### list_array_tt protected
&emsp;`protected` 部分则是自定义的迭代器。
```c++
template <typename Element, typename List>
class list_array_tt {
...
 protected:
    class iterator {
        // 指向指针的指针，且中间夹了一个 const 修饰，
        // const 表示前半部分 List * 不可更改。
        // lists 是一个指向指针的指针，表示它指向的这个指针的指向不可更改。
        
        // 对应 array_t 中的 List* lists[0]
        List * const *lists;
        List * const *listsEnd;
        
        // 这里的 List 都是 entsize_list_tt 类型即 m 和 mEnd 是
        // entsize_list_tt::iterator 类型
        typename List::iterator m, mEnd;

     public:
        // 构造函数，初始化列表初始化 lists、listsEnd，
        // if 内部，*begin 是 entsize_list_tt 指针
        iterator(List *const *begin, List *const *end)
            : lists(begin), listsEnd(end)
        {
            if (begin != end) {
                // m 和 mEnd 分别是指向 List* lists[0] 数组中 *begin 列表的第一个元素和最后一个元素的迭代器
                m = (*begin)->begin();
                mEnd = (*begin)->end();
            }
        }
        
        // 重载 * 操作符
        const Element& operator * () const {
            return *m;
        }
        
        // 重载 * 操作符
        Element& operator * () {
            return *m;
        }

        // 重载 != 操作符
        bool operator != (const iterator& rhs) const {
            if (lists != rhs.lists) return true;
            
            // 表示此时还没有使用 array_t
            if (lists == listsEnd) return false;  // m is undefined
            
            if (m != rhs.m) return true;
            return false;
        }
        
        // 自增操作
        const iterator& operator ++ () {
            ASSERT(m != mEnd);
            
            // 这里是指迭代器指向的当前的方法列表的迭代器
            // (array_t 的 lists 中包含多个方法列表，每个列表迭代到 mEnd 后，会切换到下一个列表，并更新 m 和 mEnd)
            // 
            // entsize_list_tt::iterator 自增
            m++;
            
            if (m == mEnd) {
                // 如果当前已经迭代到当前方法列表的末尾
                ASSERT(lists != listsEnd);
                
                // 自增，切到 array_t 的 lists 数组中的下一个方法列表中
                lists++;
                
                if (lists != listsEnd) {
                    // 更新新的方法列表的 m 和 mEnd
                    m = (*lists)->begin();
                    mEnd = (*lists)->end();
                }
            }
            
            // 取出 iterator 的内容
            return *this;
        }
    };
    ...
};
```

#### list_array_tt private 部分
```c++
template <typename Element, typename List>
class list_array_tt {
 ...
 
 // 刚刚两块都是定义在 list_array_tt 中的独立部分 struct array_t 和 class iterator。
 // 下面开始是 class list_array_tt 自己的内容
 
 private:
    union {
        // 联合体，包含两种情况：
        // list_array_tt 存储一个 entsize_list_tt 指针，保存一组内容（如只有一组 method_t）。
        
        List* list;
        
        // list_array_tt 存储一个 array_t 指针，array_t 中是 entsize_list_tt * lists[0]，
        // 存储 entsize_list_tt * 的数组。
        //（如多组 method_t。如给某个类编写了多个 category，每个 category 的实例方法数组会被一组一组追加进来，
        // 而不是说不同 category 的实例方法统一追加到一个大数组中）
        
        uintptr_t arrayAndFlag;
    };

    // 第 1 位标识是指向单个列表的指针还是指向列表的指针数组。
    // 如果是 1 表示是指向列表的指针数组，即使用 array_t。
    bool hasArray() const {
        return arrayAndFlag & 1;
    }

    // arrayAndFlag 第 1 位置为 0，其它位保持不变，然后强转为 array_t *
    // （第 1 位只是用做标识位，真正使用 arrayAndFlag 的值时需要把第 1 位置回 0）
    array_t *array() const {
        return (array_t *)(arrayAndFlag & ~1);
    }
    
    // 把 array_t *array 强转为 uintptr_t，
    // 并把第 1 位置为 0，标识当前 list_array_t 内部数据使用的是 array_t
    void setArray(array_t *array) {
        arrayAndFlag = (uintptr_t)array | 1;
    }
    ...
};
```
#### list_array_tt public 部分
```c++
template <typename Element, typename List>
class list_array_tt {
 ...
 public:
    // 构造函数
    list_array_tt() : list(nullptr) { }
    list_array_tt(List *l) : list(l) { }

    // 统计所有元素的个数，注意这里是所有 Element 的个数
    uint32_t count() const {
        uint32_t result = 0;
        for (auto lists = beginLists(), end = endLists(); 
             lists != end;
             ++lists)
        {
            // 例如使用 entsize_list_tt -> count，统计 lists 中每个方法列表中的 method_t 的数量
            result += (*lists)->count;
        }
        return result;
    }

    // begin 迭代器
    iterator begin() const {
        return iterator(beginLists(), endLists());
    }

    // end 迭代器
    iterator end() const {
        List * const *e = endLists();
        return iterator(e, e);
    }

    // 例如方法列表数量（属性列表数量，协议列表数量等）（例如: method_array_t 下方法列表的数量）
    uint32_t countLists() {
        if (hasArray()) {
            // 如果此时是 array_t，则返回其 count
            return array()->count;
        } else if (list) {
            // 如果此时是指向单个列表的指针，则仅有 1 个方法列表
            return 1;
        } else {
            // 其它为 0
            return 0;
        }
    }

    // Lists 的起始地址
    List* const * beginLists() const {
        if (hasArray()) {
            // 如果此时是指向列表的指针数组，则直接返回 lists
            return array()->lists;
        } else {
            // 如果此时是指向单个列表的指针，则 & 取出其地址返回
            return &list;
        }
    }

    // Lists 的 end 位置（注意 end 位置是指最后一个元素的后面，不是指最后一个元素）
    List* const * endLists() const {
        if (hasArray()) {
            // 指针偏移
            return array()->lists + array()->count;
        } else if (list) {
            // &list + 1 偏移到整个指针的后面
            return &list + 1;
        } else {
            // 为空时，如果 list 为空，那对空取地址应该是 0x0
            return &list;
        }
    }
    
    // 附加 Lists，这里分三种情况
    void attachLists(List* const * addedLists, uint32_t addedCount) {
        // 如果 addedCount 为 0，则直接返回
        if (addedCount == 0) return;

        if (hasArray()) {
            // 1): 如果目前是指向列表的指针数组，即把 addedLists 追加到 List* lists[0] 数组中
            // many lists -> many lists
            
            uint32_t oldCount = array()->count;
            uint32_t newCount = oldCount + addedCount;
            
            // 内部 realloc 函数申请  空间，同时 setArray 函数把第 1 位置为 1，作为标记
            setArray((array_t *)realloc(array(), array_t::byteSize(newCount)));
            
            // 更新 count 值
            array()->count = newCount;
            
            // 把原始的 array()->lists 移动到了后面的内存空间中，
            // 前面空出了 [array()->lists, array()->lists + addedCount] 的空间
            memmove(array()->lists + addedCount, array()->lists, 
                    oldCount * sizeof(array()->lists[0]));
            
            // 把要新追加的 addedLists 添加到上面预留出的空间
            //（这里证明了分类中添加的同名函数会 "覆盖" 类定义中的原始函数）
            memcpy(array()->lists, addedLists, 
                   addedCount * sizeof(array()->lists[0]));
        }
        else if (!list  &&  addedCount == 1) {
            // 2): 如果目前 lists 不存在并且 addedCount 等于 1，则直接把 addedLists 赋值给 list 
            // 此时只保存一个方法列表
            // 0 lists -> 1 list
            
            list = addedLists[0];
        } 
        else {
            // 3): 如果目前是指向单个列表的指针需要转化为指向列表的指针数组
            // 1 list -> many lists
            
            List* oldList = list;
            uint32_t oldCount = oldList ? 1 : 0;
            uint32_t newCount = oldCount + addedCount;
            
            // 内部 realloc 函数申请空间，setArray 函数把第 1 位置为 1，作为标记
            // 因为是新开辟空间，所以用的 malloc
            setArray((array_t *)malloc(array_t::byteSize(newCount)));
            
            // 更新 count
            array()->count = newCount;
            
            // 这里同样也是把 oldList 放在后面
            if (oldList) array()->lists[addedCount] = oldList;
            
            // 把要新追加的 addedLists 复制到 array()->lists 起始的空间内
            // 复制
            // 复制
            // 复制
            memcpy(array()->lists, addedLists, 
                   addedCount * sizeof(array()->lists[0]));
        }
    }

    // 释放内存空间
    void tryFree() {
        if (hasArray()) {
            // 如果当前是指向列表的指针数组，首先进行循环释放列表
            for (uint32_t i = 0; i < array()->count; i++) {
                try_free(array()->lists[i]);
            }
            
            // 最后释放 array()
            try_free(array());
        }
        else if (list) {
            // 如果当前仅有一个方法列表
            
            // static void try_free(const void *p) 
            // {
            //    if (p && malloc_size(p)) free((void *)p);
            // }
        
            // 如果 p 不为空且系统为其开辟了空间则执行 free 函数
            
            try_free(list);
        }
    }

    // 复制一份 list_array_t 
    template<typename Result>
    Result duplicate() {
        Result result;

        if (hasArray()) {
            array_t *a = array();
            result.setArray((array_t *)memdup(a, a->byteSize()));
            
            for (uint32_t i = 0; i < a->count; i++) {
                // 循环方法列表进行复制
                result.array()->lists[i] = a->lists[i]->duplicate();
            }
            
        } else if (list) {
            // 如果仅有一个方法列表，则直接复制后进行赋值
            result.list = list->duplicate();
        } else {
            // 空
            result.list = nil;
        }
        
        return result;
    }
};
```
### try_free
```c++
static void try_free(const void *p) 
{
    // free 函数
    if (p && malloc_size(p)) free((void *)p);
}
```
### method_array_t
```c++
class method_array_t : 
    public list_array_tt<method_t, method_list_t> 
{
    // 类型定义
    typedef list_array_tt<method_t, method_list_t> Super;

 public:
    method_array_t() : Super() { }
    method_array_t(method_list_t *l) : Super(l) { }

    // category 添加的函数的起始地址，由于 category 函数会追加到函数列表的最前面，
    // 所以 beginLists 就是 beginCategoryMethodLists
    method_list_t * const *beginCategoryMethodLists() const {
        return beginLists();
    }
    
    // 分类添加的函数的结束地址，实现在 objc-runtime-new.mm 中
    method_list_t * const *endCategoryMethodLists(Class cls) const;

    // 复制一份
    method_array_t duplicate() {
        return Super::duplicate<method_array_t>();
    }
};
```
#### endCategoryMethodLists
```c++
method_list_t * const *method_array_t::endCategoryMethodLists(Class cls) const
{
    auto mlists = beginLists();
    auto mlistsEnd = endLists();
    
    if (mlists == mlistsEnd  ||  !cls->data()->ro()->baseMethods())
    {
        // No methods, or no base methods. 
        // 没有方法，也没有基本方法。
        
        // Everything here is a category method.
        // 这里的一切都是分类方法。
        
        return mlistsEnd;
    }
    
    // Have base methods. Category methods are everything except the last method list.
    // 有基本方法。分类方法是除最后一个方法列表以外的所有内容。
    
    return mlistsEnd - 1;
}
```
### property_array_t
```c++
class property_array_t : 
    public list_array_tt<property_t, property_list_t> 
{
    // 类型声明
    typedef list_array_tt<property_t, property_list_t> Super;

 public:
    // 构造函数
    property_array_t() : Super() { }
    property_array_t(property_list_t *l) : Super(l) { }

    // 复制一份
    property_array_t duplicate() {
        return Super::duplicate<property_array_t>();
    }
};
```
### protocol_array_t
```c++
class protocol_array_t : 
    public list_array_tt<protocol_ref_t, protocol_list_t> 
{
    // 类型声明
    typedef list_array_tt<protocol_ref_t, protocol_list_t> Super;

 public:
    // 构造函数
    protocol_array_t() : Super() { }
    protocol_array_t(protocol_list_t *l) : Super(l) { }

    // 复制一份
    protocol_array_t duplicate() {
        return Super::duplicate<protocol_array_t>();
    }
};
```
&emsp;至此我们的 `objc-runtime-new.h` 文件的除 `objc_class` 函数之外的内容就全部解析完毕了。内容还挺多的，但是总体下来结构还是挺清晰的，下篇继续分析 `objc_class` 的函数部分，一起⛽️⛽️ 吧！

## 参考链接
**参考链接:🔗**
+ [iOS之LLDB常用命令](https://juejin.im/post/6869621360415637518)
+ [并发编程—OSAtomic原子操作](https://blog.csdn.net/pjk1129/article/details/44779831)
