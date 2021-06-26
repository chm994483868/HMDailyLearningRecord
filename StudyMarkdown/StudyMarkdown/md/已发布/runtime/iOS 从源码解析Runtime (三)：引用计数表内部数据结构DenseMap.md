# iOS 从源码解析Runtime (三)：引用计数表内部数据结构objc::DenseMap

> &emsp;在 [iOS weak 底层实现原理(一)：SideTable|s、weak_table_t、weak_entry_t 等数据结构](https://juejin.im/post/6865468675940417550) 已经解读过 `struct SideTable`，但是当时重点放在了 `weak` 相关内容上，由于较少涉及对象的引用计数相关内容，所以没有详细解读其中的 `RefcountMap refcnts`，那么就由本篇来解读。

## RefcountMap refcnts
&emsp;`refcnts`（应该是 `reference count` 的缩写）是 `struct SideTable` 的一个成员变量，它作为一张散列表来保存对象的引用计数。`RefcountMap` 类型定义如下，看到 `RefcountMap` 的实际类型是 `objc::DenseMap` 模版类。
```c++
// RefcountMap disguises its pointers because we
// don't want the table to act as a root for `leaks`.

// RefcountMap 伪装了它的指针，因为我们不希望该表充当 leaks 的根。
typedef objc::DenseMap<DisguisedPtr<objc_object>,size_t,RefcountMapValuePurgeable> RefcountMap;
```

## DenseMap
```c++
// 看到 DenseMap 的超长模版定义，不免有些头皮发麻，下面我们分析的时候就根据它的模版参数的顺序来一个一个解析 ⛽️
template <typename KeyT, typename ValueT,
          typename ValueInfoT = DenseMapValueInfo<ValueT>,
          typename KeyInfoT = DenseMapInfo<KeyT>,
          typename BucketT = detail::DenseMapPair<KeyT, ValueT>>
class DenseMap : public DenseMapBase<DenseMap<KeyT, ValueT, ValueInfoT, KeyInfoT, BucketT>,
                                     KeyT, 
                                     ValueT, 
                                     ValueInfoT, 
                                     KeyInfoT, 
                                     BucketT> { ... };
```
&emsp;只看 `RefcountMap` 的 `typedef` 语句的话，我们可以直白的把 `RefcountMap` 理解为一个 `key` 是我们的对象指针 `value` 是该对象的引用计数（`size_t`）的哈希表。

&emsp;深入下去 `DenseMap` 涉及的数据结构真的超多，为了秉持完成 `runtime` 每行代码都要看的通透，那我们硬着头看下去。`DenseMap` 是在 `llvm` 中用的非常广泛的数据结构，它本身的实现是一个基于`Quadratic probing`（二次探查）的散列表，键值对本身是 `std::pair<KeyT, ValueT>`。`DenseMap` 有四个成员变量: `Buckets`、`NumEntries`、`NumTombstones`、`NumBuckets` 分别用于表示散列桶的起始地址（一块连续的内存）、已存储的数据的个数、`Tombstone` 个数（表示之前存储的有值，现在被 `erase` 了的个数，初始化是置为 `EmptyKey`，当 `erase` 后被置为 `TombstoneKey`（TombstoneKey 的值是 EmptyKey - 1）。二次探查法删除数据时需要设置 `deleted` 标识）、桶的总个数。

&emsp;`DenseMap` 继承自 `DenseMapBase`，`DenseMapBase` 是 `2012` 年 `Chandler Carruth` 添加的，为了实现 `SmallDenseMap`，将 `DenseMap` 的哈希逻辑抽象到了 `DenseMapBase` 中，而内存管理的逻辑留在了 `DenseMap` 和 `SmallDenseMap` 实现。

&emsp;`DenseMap` 的前三个模版参数是:  
1. `DisguisedPtr<objc_object>` 伪装 `objc_object` 指针。
2. `size_t` 表示引用计数的值。
3. `RefcountMapValuePurgeable` 一个结构体，只定义了一个静态内联函数 `isPurgeable`，入参为 `0` 时返回 `true`，否则返回 `false`。

### RefcountMapValuePurgeable 和 DenseMapValueInfo
&emsp;`RefcountMapValuePurgeable` 是在 `NSObject.mm` 文件中 `RefcountMap` 上面定义的一个结构体，直接作为了 `DenseMap` 的第三个模版参数，在 `llvm-DenseMapInfo.h` 中 `class DenseMap` 定义中该位置的模版参数是有一个默认值的: `DenseMapValueInfo`，它的内部也是只有一个静态内联函数 `isPurgeable` 但它是直接返回 `false`。
```c++
// 在 NSObject.mm 中
struct RefcountMapValuePurgeable {
    static inline bool isPurgeable(size_t x) {
        return x == 0;
    }
};

// 默认
template<typename T>
struct DenseMapValueInfo {
    // 作为 DenseMap 的默认模版参数时 T 的类型是 size_t 
    static inline bool isPurgeable(const T &value) {
        return false;
    }
};
```

### DenseMapInfo
&emsp;`typename KeyInfoT = DenseMapInfo<KeyT>`。`DenseMapInfo` 是一个模版结构体，其内部只有四个静态函数，分别用于 `empty key`、`tombstone key` 以及哈希值的计算，它定义在 `llvm-DenseMapInfo.h` 中，该文件只有 `200` 行，文件前面的注释 `This file defines DenseMapInfo traits for DenseMap.` (该文件用来定义 `DenseMap` 的 `DenseMapInfo` 特征，取自 `llvmCore-3425.0.31`，（后期会深入学习 `LLVM`）。) 表明其核心作用，文件下面提供了针对常见类型的 `DenseMapInfo` 的特化版本，例如指针类型、整型等，这里我们主要使用 `DenseMapInfo<DisguisedPtr<T>>`。

&emsp;关于模版内部实现，对于 `getEmptyKey` 函数实现，基本上返回的都是模版参数所能表示的最大值，`getTombstoneKey` 都是 `getEmptyKey` 再减 1。

&emsp;针对 `DenseMapInfo<T*>` 和 `DenseMapInfo<DisguisedPtr<T>>` 版本，它们的 `getHashValue` 函数内部实现都是直接调用了 `objc4-781` 全局通用的指针哈希函数 `ptr_hash`。其它例如 `DenseMapInfo<long>` 、 `DenseMapInfo<int>` 等它们的 `getHashValue(const long& Val)` 函数都是直接返回 `Val * 37UL` 的值，哈希值的计算都是仅用了这个乘法计算，每个 `hash seed` 都是 `37`，哈希函数中普遍都使用质数作为哈希种子，质数能够有效的避免哈希碰撞的发生，这里选择 `37` 大概是在测试过程中有比较好的性能表现。

```c++
template<typename T>
struct DenseMapInfo {
  //static inline T getEmptyKey();
  //static inline T getTombstoneKey();
  //static unsigned getHashValue(const T &Val);
  //static bool isEqual(const T &LHS, const T &RHS);
};
```
&emsp;下面是针对 `struct DenseMapInfo` 的特化版本：

#### DenseMapInfo<T*>
```c++
// Provide DenseMapInfo for all pointers.
// 为所有的指针提供 DenseMapInfo

template<typename T>
struct DenseMapInfo<T*> {
  static inline T* getEmptyKey() {
    
    // static_cast <type-id>( expression )
    // 把 expression 转换为 type-id 类型，但没有运行时类型检查来保证转换的安全性 
    
    // reinterpret_cast<type-id> (expression)
    // 操作符修改了操作数类型，但仅仅是重新解释了给出的对象的比特模型而没有进行二进制转换。
    
    // -1 转化为 unsigned long 的最大值 18446744073709551615
    uintptr_t Val = static_cast<uintptr_t>(-1);
    
    // 把 18446744073709551615 转换为 T 指针
    return reinterpret_cast<T*>(Val);
  }
  static inline T* getTombstoneKey() {
    // -2 转化为 18446744073709551614
    uintptr_t Val = static_cast<uintptr_t>(-2);
    
    // 把 18446744073709551614 转化为 T 指针
    return reinterpret_cast<T*>(Val);
  }
  static unsigned getHashValue(const T *PtrVal) {
      // 指针哈希函数
      return ptr_hash((uintptr_t)PtrVal);
  }
  
  // 判断是否相等(T 类型可能重写 ==)
  static bool isEqual(const T *LHS, const T *RHS) { return LHS == RHS; }
};
```
&emsp;`static_cast` 和 `reinterpret_cast` 的区别可参考: [《reinterpret_cast》](https://baike.baidu.com/item/reinterpret_cast/9303204?fr=aladdin)。

#### DenseMapInfo<DisguisedPtr<T>>
```c++
// Provide DenseMapInfo for disguised pointers.
// 为伪装的指针提供 DenseMapInfo。基本和 T* 保持相同

template<typename T>
struct DenseMapInfo<DisguisedPtr<T>> {
  static inline DisguisedPtr<T> getEmptyKey() {
  
    // DisguisedPtr 内部: DisguisedPtr(T* ptr) : value(disguise(ptr)) { }
    // static uintptr_t disguise(T* ptr) { return -(uintptr_t)ptr; }
    
    // (uintptr_t)-1 是 unsigned long 的最大值，又被 -(uintptr_t)ptr 转化为 1, 即 DisguisedPtr 的 value 值为 1
    
    return DisguisedPtr<T>((T*)(uintptr_t)-1);
  }
  static inline DisguisedPtr<T> getTombstoneKey() {
  
    // 同上 DisguisedPtr 的 value 值为 2
    return DisguisedPtr<T>((T*)(uintptr_t)-2);
  }
  static unsigned getHashValue(const T *PtrVal) {
      // 指针 hash 函数
      return ptr_hash((uintptr_t)PtrVal);
  }
  static bool isEqual(const DisguisedPtr<T> &LHS, const DisguisedPtr<T> &RHS) {
      // 判等
      return LHS == RHS; 
  }
};
```

#### DenseMapInfo<const char*>
```c++
// Provide DenseMapInfo for cstrings.
// 为 cstrings 提供 DenseMapInfo。

template<> struct DenseMapInfo<const char*> {
  static inline const char* getEmptyKey() {
    // typedef __darwin_intptr_t intptr_t;
    // typedef long __darwin_intptr_t;
    // 把 -1 转化为 const char *
    return reinterpret_cast<const char *>((intptr_t)-1); 
  }
  static inline const char* getTombstoneKey() { 
    // 把 -2 转化为 const char *
    return reinterpret_cast<const char *>((intptr_t)-2); 
  }
  static unsigned getHashValue(const char* const &Val) { 
    // 哈希函数，下面解析
    return _objc_strhash(Val); 
  }
  static bool isEqual(const char* const &LHS, const char* const &RHS) {
    if (LHS == RHS) {
      return true;
    }
    
    // 任一值为 getEmptyKey 或 getTombstoneKey 都返回 false
    if (LHS == getEmptyKey() || RHS == getEmptyKey()) {
      return false;
    }
    if (LHS == getTombstoneKey() || RHS == getTombstoneKey()) {
      return false;
    }
    
    // 字符串比较
    return 0 == strcmp(LHS, RHS);
  }
};
```

#### _objc_strhash
```c++
static __inline uint32_t _objc_strhash(const char *s) {
    uint32_t hash = 0;
    for (;;) {
    
    // 从 s 起点开始每次读取一个字节的数据
    int a = *s++;
    
    if (0 == a) break;
    // 每次把 hash 的值左移 8 位给 a 留出空间，再加 a
    // 再加 hash
    hash += (hash << 8) + a;
    }
    return hash;
}
```

#### DenseMapInfo<char>
&emsp;下面的一组 `DenseMapInfo<unsigned>`、`DenseMapInfo<unsigned long>`、`DenseMapInfo<unsigned long long>`、`DenseMapInfo<int>`、`DenseMapInfo<long>`、`DenseMapInfo<long long>` 几乎都一模一样，`getEmptyKey` 都是取该抽象类型的最大值，`getTombstoneKey` 都是最大值减 1，`getHashValue` 都是乘以 `37`，`isEqual` 函数都是直接直接 `==`。

#### DenseMapInfo<std::pair<T, U> >
```c++
// Provide DenseMapInfo for all pairs whose members have info.

template<typename T, typename U>
struct DenseMapInfo<std::pair<T, U> > {
  typedef std::pair<T, U> Pair;
  
  typedef DenseMapInfo<T> FirstInfo;
  typedef DenseMapInfo<U> SecondInfo;

  static inline Pair getEmptyKey() {
    // FirstInfo::getEmptyKey() 和 SecondInfo::getEmptyKey() 构成 std::pair 返回
    return std::make_pair(FirstInfo::getEmptyKey(),
                          SecondInfo::getEmptyKey());
  }
  
  static inline Pair getTombstoneKey() {
    // FirstInfo::getTombstoneKey() 和 SecondInfo::getTombstoneKey() 构成 std::pair 返回
    return std::make_pair(FirstInfo::getTombstoneKey(),
                          SecondInfo::getTombstoneKey());
  }
  
  static unsigned getHashValue(const Pair& PairVal) {
  
   // 把 first 的哈希值(32位 int)左移 32 位和 second 的哈希值(32位 int)做或运算，
   // 即把 first 和 second 的哈希值合并到一个 64 位 int 中。
    uint64_t key = (uint64_t)FirstInfo::getHashValue(PairVal.first) << 32
          | (uint64_t)SecondInfo::getHashValue(PairVal.second);
    
    // 然后那上面的 64 位 int 做 移位 取反 相加 异或 操作
    key += ~(key << 32);
    key ^= (key >> 22);
    key += ~(key << 13);
    key ^= (key >> 8);
    key += (key << 3);
    key ^= (key >> 15);
    key += ~(key << 27);
    key ^= (key >> 31);
    
    return (unsigned)key;
  }
  
  // 判等
  static bool isEqual(const Pair &LHS, const Pair &RHS) {
    return FirstInfo::isEqual(LHS.first, RHS.first) &&
           SecondInfo::isEqual(LHS.second, RHS.second);
  }
};
```

#### std::pair<T, U>
```c++
template <class _T1, class _T2>
struct _LIBCPP_TEMPLATE_VIS pair {
    typedef _T1 first_type;
    typedef _T2 second_type;

    _T1 first;
    _T2 second;

#if !defined(_LIBCPP_CXX03_LANG)
    pair(pair const&) = default;
    pair(pair&&) = default;
#else
  // Use the implicitly declared copy constructor in C++03
#endif
...
};
```
&emsp;`std::pair` 是一个结构体模板，其可于一个单元内存储两个相异对象，是 `std::tuple` 的拥有两个元素的特殊情况。一般来说，`pair` 可以封装任意类型的对象，可以生成各种不同的 `std::pair<T1, T2>` 对象，可以是数组对象或者包含 `std::pair<T1,T2>` 的 `vector` 容器。`pair` 还可以封装两个序列容器或两个序列容器的指针。（想到了 `Swift` 中的元组）具体细节可参考：[STL std::pair基本用法](https://www.cnblogs.com/phillee/p/12099504.html)

### detail::DenseMapPair<KeyT, ValueT>
&emsp;第五个模版参数。
```c++
// We extend a pair to allow users to override the bucket
// type with their own implementation without requiring two members.
// 我们扩展了 pair，允许用户使用自己的实现覆盖存储桶类型，而无需两个成员。

// 公开继承自 std::pair
template <typename KeyT, typename ValueT>
struct DenseMapPair : public std::pair<KeyT, ValueT> {

  // FIXME: Switch to inheriting constructors when we drop support for older
  // clang versions.
  // 当我们放弃对较旧的 clang 版本的支持时，请切换到继承构造函数。
  
  // NOTE: This default constructor is declared with '{}' rather than
  // '= default' to work around a separate bug in clang-3.8. 
  // This can also go when we switch to inheriting constructors.
  // 此默认构造函数使用 '{}' 而不是 '= default' 声明，以解决 clang-3.8 中的一个单独的错误。
  // 当我们切换到继承构造函数时，这也可以进行。
  
  DenseMapPair() {}

  // 初始化列表内使用 Key 和 Value 初始化 std::pair
  DenseMapPair(const KeyT &Key, const ValueT &Value)
      : std::pair<KeyT, ValueT>(Key, Value) {}

  // KeyT && ValueT && 通用引用
  // 初始化列表初始化 std::pair
  DenseMapPair(KeyT &&Key, ValueT &&Value)
      : std::pair<KeyT, ValueT>(std::move(Key), std::move(Value)) {}

  // 实现调用函数去推导正确的模板函数版本，以下两个函数大概都是保证模版特化正常...
  template <typename AltKeyT, typename AltValueT>
  DenseMapPair(AltKeyT &&AltKey, AltValueT &&AltValue,
               typename std::enable_if<
                   std::is_convertible<AltKeyT, KeyT>::value &&
                   std::is_convertible<AltValueT, ValueT>::value>::type * = 0)
      : std::pair<KeyT, ValueT>(std::forward<AltKeyT>(AltKey),
                                std::forward<AltValueT>(AltValue)) {}

  template <typename AltPairT>
  DenseMapPair(AltPairT &&AltPair,
               typename std::enable_if<std::is_convertible<
                   AltPairT, std::pair<KeyT, ValueT>>::value>::type * = 0)
      : std::pair<KeyT, ValueT>(std::forward<AltPairT>(AltPair)) {}

  // 返回 first 的引用
  KeyT &getFirst() { return std::pair<KeyT, ValueT>::first; }
  
  // 返回 const first 的引用
  const KeyT &getFirst() const { return std::pair<KeyT, ValueT>::first; }
  
  // 返回 second 引用
  ValueT &getSecond() { return std::pair<KeyT, ValueT>::second; }
  
  // 返回 const second 引用
  const ValueT &getSecond() const { return std::pair<KeyT, ValueT>::second; }
};
```
&emsp;`std::move` 右值引用可具体参考：[C++右值引用（std::move）](https://zhuanlan.zhihu.com/p/94588204)。涉及到大量 `C++ 11` 相关的内容，但总体还是继承 `std::pair` 新建 `struct DenseMapPair` 方便我们使用 `std::pair` 特性。

&emsp;分析 `DenseMap` 的代码实现，`DenseMap` 的内存管理，主要是通过 `operator new` 分配内存，通过 `operator delete` 释放内存。
```c++
template <typename KeyT, typename ValueT,
          typename ValueInfoT = DenseMapValueInfo<ValueT>,
          typename KeyInfoT = DenseMapInfo<KeyT>,
          typename BucketT = detail::DenseMapPair<KeyT, ValueT>>
class DenseMap : public DenseMapBase<DenseMap<KeyT, ValueT, ValueInfoT, KeyInfoT, BucketT>,
                                     KeyT, ValueT, ValueInfoT, KeyInfoT, BucketT> {
                                       
  // 把 DenseMapBase 定为 DenseMap 的友元类，这样 DenseMapBase 就能完全访问 DenseMap 的私有成员变量私有函数等
  friend class DenseMapBase<DenseMap, KeyT, ValueT, ValueInfoT, KeyInfoT, BucketT>;
  
  // Lift some types from the dependent base class into this class for
  // simplicity of referring to them.
  // 为了简化引用，将某些类型从依赖基类提升到此类，主要为了方便的在子类里面使用父类。
  
  // 这里主要为了在 DenseMap 里面调用 DenseMapBase 的函数
  using BaseT = DenseMapBase<DenseMap, KeyT, ValueT, ValueInfoT, KeyInfoT, BucketT>;
  
  // 下面是 4 个成员变量
  
  // 散列桶的起始地址（一块连续的内存）
  BucketT *Buckets;
  
  // 已存储的数据的个数
  unsigned NumEntries;
  
  // Tombstone 个数（二次探查法删除数据时需要设置 deleted 标识）
  unsigned NumTombstones;
  
  // 桶的总个数
  unsigned NumBuckets;

public:
  /// Create a DenseMap wth an optional \p InitialReserve that guarantee that
  /// this number of elements can be inserted in the map without grow()
  /// 使用可选的 InitialReserve 指定长度创建一个 DenseMap，以确保可以将这些数量的元素插入到 map 中，而无需调用 grow（）。
  
  /// explicit 禁止隐式转换
  explicit DenseMap(unsigned InitialReserve = 0) { init(InitialReserve); }

  // 复制构造函数
  DenseMap(const DenseMap &other) : BaseT() {
    // 入参为 0，把 4 个成员变量全部置为 0
    init(0);
    
    // other 的内容复制到 this 
    copyFrom(other);
  }

  // 复制构造函数，入参是 DenseMap &&
  DenseMap(DenseMap &&other) : BaseT() {
    init(0);
    
    // 交换 other 和 this
    swap(other);
  }

  // 大概是用一个起点和一个终点来初始化 DenseMap 吗？
  template<typename InputIt>
  DenseMap(const InputIt &I, const InputIt &E) {
    // 初始长度
    init(std::distance(I, E));
    // 插入
    this->insert(I, E);
  }
  
  // 由一组 typename BaseT::value_type 初始化 DenseMap
  DenseMap(std::initializer_list<typename BaseT::value_type> Vals) {
    // 申请空间并进行初始化
    init(Vals.size());
    
    // 从 Vals 起点到终点把 BaseT::value_type 插入到 this 中
    //（因为这是初始化函数所以不需要进行扩容机制）
    // insert 函数
    // template <class _Pp,
    //          class = typename enable_if<is_constructible<value_type,
    //          _Pp>::value>::type>
    //     _LIBCPP_INLINE_VISIBILITY
    //     pair<iterator, bool> insert(_Pp&& __p)
    //         {return __tree_.__insert_unique(_VSTD::forward<_Pp>(__p));}
    // 使用的 map 类的插入函数
    this->insert(Vals.begin(), Vals.end());
  }

  // 析构函数
  ~DenseMap() {
    // 循环析构 Buckets 里面的 BucketT
    this->destroyAll();
    
    // 释放内存
    operator delete(Buckets);
  }

  // 交换
  void swap(DenseMap& RHS) {
    std::swap(Buckets, RHS.Buckets);
    std::swap(NumEntries, RHS.NumEntries);
    std::swap(NumTombstones, RHS.NumTombstones);
    std::swap(NumBuckets, RHS.NumBuckets);
  }
                                       
  // 重载赋值操作符
  DenseMap& operator=(const DenseMap& other) {
    if (&other != this)
      // 先释放旧值，再申请空间，然后把 other 复制到 this 
      copyFrom(other);
      
    return *this;
  }

  // 重载赋值操作符
  DenseMap& operator=(DenseMap &&other) {
    // 循环析构 Buckets 里面的 BucketT
    this->destroyAll();
    
    // 释放 Buckets 的内存
    operator delete(Buckets);
    
    // 都置为 0
    init(0);
    
    // 交换
    swap(other);
    
    return *this;
  }

  // copy 函数
  void copyFrom(const DenseMap& other) {
    // 循环析构 Buckets 里面的 BucketT
    this->destroyAll();
    
    // 释放 Buckets 的内存
    operator delete(Buckets);
    
    // 申请空间
    if (allocateBuckets(other.NumBuckets)) {
      // 调用 DenseMapBase 的复制函数
      this->BaseT::copyFrom(other);
    } else {
      // 申请空间失败的话把成员变量都置为 0
      NumEntries = 0;
      NumTombstones = 0;
    }
  }

  // DenseMap 的初始化分为如下三步：
  // 针对初始元素数，计算初始最小桶的数量
  // 针对桶的个数，分配内存
  // 初始化
  
  // 由于 DenseMap 对桶的数量有两个标准：
  // 桶的数量必须是2次幂
  // 如果 DenseMap 的 load factor > 3/4 或者空桶数量 < 1/8，
  // 则说明需要增加桶的数量
  
  // 为了满足这两个标准，getMinBucketToReserveForEntries() 首先将元素数量 * 4/3，
  // 然后计算大于元素数量 * 4/3 + 1 的最小的 2 次幂，计算 2 次幂的方法为 NextPowerOf2()。
  
  // 为桶分配内存的方法是 allocateBuckets()，
  // 该方法就是调用 operator new() 分配一块堆内存，用于存放数据。
  // 最后是信息的初始化，初始化空桶的方法是 initEmpty()。
  
  void init(unsigned InitNumEntries) {
    // 获取需要分配的桶数
    auto InitBuckets = BaseT::getMinBucketToReserveForEntries(InitNumEntries);
    
    // new(sizeof(BucketT) * NumBuckets) 申请空间，如果成功返回 true
    if (allocateBuckets(InitBuckets)) {
    
      // 执行 DenseMapBase 的 initEmpty 函数
      this->BaseT::initEmpty();
    } else {
      NumEntries = 0;
      NumTombstones = 0;
    }
  }

  // 扩容
  // DenseMap 在初始化阶段，会进行初始桶数量的计算，桶的分配，以及 empty key 的初始化。
  // 当桶的数量不够时，标准是 load factor > 3/4 或者空桶数量 < 1/8，说明需要分配新的桶来存储数据。
  // 为 DenseMap 增加桶数量的方法是 grow()。
  // 增长过程和 std::vector 很相似，分为计算新的桶数量并分配内存，拷贝数据，释放旧的桶。
  // 计算桶的数量同样使用的是 NextPowerOf2() 方法。
  
  void grow(unsigned AtLeast) {
    unsigned OldNumBuckets = NumBuckets;
    BucketT *OldBuckets = Buckets;

    // 计算新的桶数量并分配内存
    allocateBuckets(std::max<unsigned>(MIN_BUCKETS, static_cast<unsigned>(NextPowerOf2(AtLeast-1))));
    
    ASSERT(Buckets);
    if (!OldBuckets) {
      this->BaseT::initEmpty();
      return;
    }

    // 拷贝数据
    this->moveFromOldBuckets(OldBuckets, OldBuckets+OldNumBuckets);

    // Free the old table.
    // 释放旧的桶
    operator delete(OldBuckets);
  }

  // 清理
  // 清理操作是由 shrink_and_clear() 方法实现的，主要是将重新分配一块内存，
  // 然后进行初始化，然后将原有的内存释放，类似于容器中的 clear() 方法。
  void shrink_and_clear() {
    unsigned OldNumEntries = NumEntries;
    this->destroyAll();

    // Reduce the number of buckets.
    // 减少桶的数量。
    unsigned NewNumBuckets = 0;
    if (OldNumEntries)
      NewNumBuckets = std::max(MIN_BUCKETS, 1 << (Log2_32_Ceil(OldNumEntries) + 1));
    
    if (NewNumBuckets == NumBuckets) {
    
      // 初始化
      this->BaseT::initEmpty();
      
      return;
    }
    
    // 释放旧数据
    operator delete(Buckets);
    
    // 重新分配一块内存，然后进行初始化
    init(NewNumBuckets);
  }

private:
  // 成员变量的赋值和取值
  unsigned getNumEntries() const {
    return NumEntries;
  }

  void setNumEntries(unsigned Num) {
    NumEntries = Num;
  }

  unsigned getNumTombstones() const {
    return NumTombstones;
  }

  void setNumTombstones(unsigned Num) {
    NumTombstones = Num;
  }

  BucketT *getBuckets() const {
    return Buckets;
  }

  unsigned getNumBuckets() const {
    return NumBuckets;
  }

  // 为 Buckets 申请内存空间
  bool allocateBuckets(unsigned Num) {
    NumBuckets = Num;
    
    if (NumBuckets == 0) {
      Buckets = nullptr;
      return false;
    }
    
    // 申请 sizeof(BucketT) * NumBuckets 个字节的空间
    Buckets = static_cast<BucketT*>(operator new(sizeof(BucketT) * NumBuckets));
    
    return true;
  }
};
```
&emsp;接下来详细分析 `DenseMap` 里的每个函数。

#### void init(unsigned InitNumEntries)
&emsp;初始化。
```c++
void init(unsigned InitNumEntries) {
  // 调用 DenseMapBase 的 getMinBucketToReserveForEntries 
  // 根据入参 InitNumEntries 获取需要分配的桶数
  // 这里的返回值是大于 (InitNumEntries * 4 / 3 + 1) 的最小的 2 次幂
  // 例如：InitNumEntries 是 5 则返回 8，是 10 则返回 16
  auto InitBuckets = BaseT::getMinBucketToReserveForEntries(InitNumEntries);
  
  // 根据计算出的 InitBuckets 申请对应数量字节的空间并返回 true 失败则返回 false
  // 并在过程中给 NumBuckets 和 Buckets 两个成员变量赋值了
  // new(sizeof(BucketT) * NumBuckets)
  if (allocateBuckets(InitBuckets)) {
  
    // 执行 DenseMapBase 的 initEmpty 函数
    this->BaseT::initEmpty();
    
  } else {
    // 如果失败的话把另外两个成员变量 NumEntries 和 NumTombstones 置为 0
    NumEntries = 0;
    NumTombstones = 0;
  }
}
```
##### unsigned getMinBucketToReserveForEntries(unsigned NumEntries)
```c++
/// Returns the number of buckets to allocate to ensure that
/// the DenseMap can accommodate \p NumEntries without need to grow().
// 返回分配的存储桶数，以确保 DenseMap 可以容纳 NumEntries 个 entry 而无需 grow()。
// 当总容量使用超过 3/4 或者剩余容量少于 1/8 时，需要调用 grow() 进行扩容

unsigned getMinBucketToReserveForEntries(unsigned NumEntries) {
  // Ensure that "NumEntries * 4 < NumBuckets * 3"
  // 确保 NumEntries 小于 NumBuckets 的四分之三
  
  // 如果为 0 则直接 return
  if (NumEntries == 0)
    return 0;
    
  // +1 is required because of the strict equality.
  // +1 是必需的，因为严格相等。
  // For example if NumEntries is 48, we need to return 401.
  // 例如，如果 NumEntries 为 48，则需要返回 401。
  // 这里注释是不是写错了，NumEntries 是 48 是返回 64。
  
  return NextPowerOf2(NumEntries * 4 / 3 + 1);
}
```

##### inline uint64_t NextPowerOf2(uint64_t A)
```c++
/// NextPowerOf2 - Returns the next power of two
/// (in 64-bits) that is strictly greater than A. 
/// Returns zero on overflow.
/// 返回严格大于 A 的 2 的下一个幂（64 位），溢出时返回零。

// 1
// 0x100000
// 0x010000 => 0x110000

// 2
// 0x110000
// 0x001100 => 0x111100

// 4
// 0x111100
// 0x001111 => 0x111111

...

// return 0x111111 + 1 

// 把 A 的二进制位从最高位到最低位全部置为 1，然后再加 1，
// 刚好就是大于 A 的最小的 2 的幂。
// （如果 A 值较小，所有位都是 1 后，后面的移位或操作都是浪费的）
inline uint64_t NextPowerOf2(uint64_t A) {
  A |= (A >> 1);
  A |= (A >> 2);
  A |= (A >> 4);
  A |= (A >> 8);
  A |= (A >> 16);
  A |= (A >> 32);
  return A + 1;
}
```

##### bool allocateBuckets(unsigned Num)
```c++
bool allocateBuckets(unsigned Num) {
  // 这里就给 NumBuckets 成员变量赋值了
  NumBuckets = Num;
  // 如果为 0，则 Buckets 置为 nullptr 并返回 false
  if (NumBuckets == 0) {
    Buckets = nullptr;
    return false;
  }
  
  // 申请 sizeof(BucketT) * NumBuckets 个字节的空间
  // 并转换为 BucketT 指针把起始地址赋给 Buckets
  Buckets = static_cast<BucketT*>(operator new(sizeof(BucketT) * NumBuckets));
  
  return true;
}
```

##### void initEmpty()
```c++
void initEmpty() {
  // 由于此函数是被 DenseMap 调用，所以 this 还是 DenseMap 实例
  
  // 把 NumEntries 和 NumTombstones 置为 0
  setNumEntries(0);
  setNumTombstones(0);

  // 断言
  // 由于 NumBuckets 是 2 的幂，那么转换为二进制则末尾数字必不为 0
  // 例如 NumBuckets 是 8
  // 8 => 0b1000 减 1 后是 0b0111，它们与的结果必为 0
  ASSERT((getNumBuckets() & (getNumBuckets()-1)) == 0 &&
         "# initial buckets must be a power of two!");
         
  // 获取 DenseMapInfo 的 EmptyKey 
  // 对应我的模版参数的话是:
  // static inline DisguisedPtr<T> getEmptyKey() {
  //   return DisguisedPtr<T>((T*)(uintptr_t)-1);
  // }
  const KeyT EmptyKey = getEmptyKey();
  
  // 对应我们的模版参数的话
  // KeyT 是 DisguisedPtr<objc_object> 
  // 这里循环对 Buckets 里的 BucketT 进行初始化
  for (BucketT *B = getBuckets(), *E = getBucketsEnd(); B != E; ++B)
    ::new (&B->getFirst()) KeyT(EmptyKey);
}
```

##### getEmptyKey
```c++
static const KeyT getEmptyKey() {
  // DenseMapBase, DerivedT 必须是类
  // 且 DerivedT 是 DenseMapBase 的子类或同类否则返回 false
  static_assert(std::is_base_of<DenseMapBase, DerivedT>::value,
                "Must pass the derived type to this template!");
  
  // 这里就用到了 DenseMapInfo<KeyT> 了
  // 对应我们使用的是 struct DenseMapInfo<DisguisedPtr<T>> {...};
  return KeyInfoT::getEmptyKey();
}
```

##### BucketT *getBuckets() const
```c++
// 取得 Buckets 起始地址
BucketT *getBuckets() const {
  return Buckets;
}
```

##### const BucketT *getBucketsEnd() const
```c++
// 指针偏移，指到 Buckets 末尾
const BucketT *getBucketsEnd() const {
  return getBuckets() + getNumBuckets();
}
```

#### void copyFrom(const DenseMap& other)
```c++
void copyFrom(const DenseMap& other) {
  // 循环析构 Buckets 里面的 BucketT
  this->destroyAll();
  
  // 释放内存
  operator delete(Buckets);
  
  // 根据 other 的桶的数量申请空间
  if (allocateBuckets(other.NumBuckets)) {
    // 调用 DenseMapBase 的 copyFrom 函数
    // 把 other 的数据复制到 this 中
    this->BaseT::copyFrom(other);
  } else {
    // 申请失败的话把另外的两个成员变量
    // NumEntries 和 NumTombstones 置为 0
    NumEntries = 0;
    NumTombstones = 0;
  }
}
```

##### void destroyAll()
```c++
void destroyAll() {
  // 如果为空则直接 return
  if (getNumBuckets() == 0) // Nothing to do.
    return;
    
  // 取得 EmptyKey 和 TombstoneKey
  const KeyT EmptyKey = getEmptyKey(), TombstoneKey = getTombstoneKey();
  
  // 桶里面的数据类型如下：
  // typename BucketT = detail::DenseMapPair<KeyT, ValueT>>
  // DenseMapPair 继承自 std::pair
  // 对它 first 和 second 两个成员变量分别执行析构函数
  
  for (BucketT *P = getBuckets(), *E = getBucketsEnd(); P != E; ++P) {
    if (!KeyInfoT::isEqual(P->getFirst(), EmptyKey) &&
        !KeyInfoT::isEqual(P->getFirst(), TombstoneKey))
      P->getSecond().~ValueT();
    P->getFirst().~KeyT();
  }
}
```

##### DenseMapBase:: void copyFrom()
```c++
template <typename OtherBaseT>
void copyFrom(
    const DenseMapBase<OtherBaseT, KeyT, ValueT, ValueInfoT, KeyInfoT, BucketT> &other) {
  // 两条断言，other 不能与 this 相同
  // other 和 this 的桶的容量必须相等
  ASSERT(&other != this);
  ASSERT(getNumBuckets() == other.getNumBuckets());

  // 成员变量赋值
  setNumEntries(other.getNumEntries());
  setNumTombstones(other.getNumTombstones());

  // 判断 KeyT 和 ValueT 是否能进行 copy
  // 如果可以直接进行字节复制
  if (is_trivially_copyable<KeyT>::value &&
      is_trivially_copyable<ValueT>::value)
    memcpy(reinterpret_cast<void *>(getBuckets()), other.getBuckets(),
           getNumBuckets() * sizeof(BucketT));
  else
  // 循环进行赋值
    for (size_t i = 0; i < getNumBuckets(); ++i) {
      ::new (&getBuckets()[i].getFirst())
          KeyT(other.getBuckets()[i].getFirst());
          
      if (!KeyInfoT::isEqual(getBuckets()[i].getFirst(), getEmptyKey()) &&
          !KeyInfoT::isEqual(getBuckets()[i].getFirst(), getTombstoneKey()))
        ::new (&getBuckets()[i].getSecond())
            ValueT(other.getBuckets()[i].getSecond());
    }
}
```

#### void grow(unsigned AtLeast)
```c++
void grow(unsigned AtLeast) {
  // 记录旧的桶的个数
  unsigned OldNumBuckets = NumBuckets;
  // 记录旧的桶的起点地址
  BucketT *OldBuckets = Buckets;

  // 根据新的桶的数量并分配内存
  // #define MIN_BUCKETS 4 桶容量最新用的是 4
  // 取得 MIN_BUCKETS 和 NextPowerOf2(AtLeast-1) 的最大值
  allocateBuckets(std::max<unsigned>(MIN_BUCKETS, static_cast<unsigned>(NextPowerOf2(AtLeast-1))));
  ASSERT(Buckets);

  // 如果没有旧桶数据的话，把 this 初始化一下就可以 return 了
  if (!OldBuckets) {
    this->BaseT::initEmpty();
    return;
  }

  // 把旧数据从起点到终点都移动到上面申请到新空间内
  this->moveFromOldBuckets(OldBuckets, OldBuckets+OldNumBuckets);

  // Free the old table.
  // 释放旧桶的内存
  operator delete(OldBuckets);
}
```

##### void moveFromOldBuckets(BucketT *OldBucketsBegin, BucketT *OldBucketsEnd)
```c++
void moveFromOldBuckets(BucketT *OldBucketsBegin, BucketT *OldBucketsEnd) {
  // 把新的空间初始化
  // 成员变量置为 0
  // 循环把 BucketT 的 KeyT 置为 EmptyKey (来自 DenseMapInfo<DisguisedPtr<objc_object>> 的 getEmptyKey() 函数)
  initEmpty();

  // Insert all the old elements.
  // 插入所有旧数据。
  
  const KeyT EmptyKey = getEmptyKey();
  const KeyT TombstoneKey = getTombstoneKey();
  
  for (BucketT *B = OldBucketsBegin, *E = OldBucketsEnd; B != E; ++B) {
  
    // 这里终于用到了 RefcountMapValuePurgeable
    // isPurgeable 函数，只有入参为 0 的时候才会返回 true
    
    if (ValueInfoT::isPurgeable(B->getSecond())) {
      // 为真时表示，BucketT 的 ValueT 是 0
      // 表示某个对象的引用计数为0，此时已经不需要保存该对数据了
      // Free the value.
      // 释放 ValueT，循环的尾部还有一个 B->getFirst().~KeyT()  
      B->getSecond().~ValueT();
    } else if (!KeyInfoT::isEqual(B->getFirst(), EmptyKey) &&
        !KeyInfoT::isEqual(B->getFirst(), TombstoneKey)) {
      // Insert the key/value into the new table.
      // 插入 key/value 到新表中。
      
      BucketT *DestBucket;
      bool FoundVal = LookupBucketFor(B->getFirst(), DestBucket);
      (void)FoundVal; // silence warning.
      ASSERT(!FoundVal && "Key already in new map?");
      
      // 分别把 KeyT 和 ValueT 移动到 DestBucket 中
      DestBucket->getFirst() = std::move(B->getFirst());
      ::new (&DestBucket->getSecond()) ValueT(std::move(B->getSecond()));
      
      // NumEntries 加 1
      incrementNumEntries();

      // Free the value.
      // 释放旧值。
      B->getSecond().~ValueT();
    }
    B->getFirst().~KeyT();
  }
}
```

##### bool LookupBucketFor()
&emsp;从哈希表中取 `BucketT`。
```c++
/// LookupBucketFor - Lookup the appropriate bucket for Val, returning it in FoundBucket.
/// 查找适合 Val 的 bucket，并在 FoundBucket 中将其返回.

/// If the bucket contains the key and a value, this returns true,
/// otherwise it returns a bucket with an empty marker or tombstone and returns false.
/// 如果 bucket 包含键和值，则返回true，否则返回带有空标记或 逻辑删除 的 bucket 并返回 false。

template<typename LookupKeyT>
bool LookupBucketFor(const LookupKeyT &Val,
                     const BucketT *&FoundBucket) const {
  // 取得 Buckets 起点
  const BucketT *BucketsPtr = getBuckets();
  // 取得 NumBuckets 
  const unsigned NumBuckets = getNumBuckets();

  // 如果为空则置为 nullptr 并返回 false
  if (NumBuckets == 0) {
    FoundBucket = nullptr;
    return false;
  }

  // FoundTombstone - Keep track of whether we find a tombstone while probing.
  // 跟踪在探测时是否找到 tombstone。
  // 标记
  const BucketT *FoundTombstone = nullptr;
  // 取得 EmptyKey 和 TombstoneKey
  const KeyT EmptyKey = getEmptyKey();
  const KeyT TombstoneKey = getTombstoneKey();
  // 断言
  // Empty/Tombstone value 不应该插入 map 中 
  assert(!KeyInfoT::isEqual(Val, EmptyKey) &&
         !KeyInfoT::isEqual(Val, TombstoneKey) &&
         "Empty/Tombstone value shouldn't be inserted into map!");

  // val: KeyT 取得 KeyT 的哈希值 
  unsigned BucketNo = getHashValue(Val) & (NumBuckets-1);
  unsigned ProbeAmt = 1;
  
  while (true) {
  
    // 从起点移动到 哈希值 宽的位置，如果没有发生哈希碰撞可能就是要找的 BucketT 
    const BucketT *ThisBucket = BucketsPtr + BucketNo;
    
    // 如果直接就找到了要找的 BucketT，那么提取它，并返回 true 
    // Found Val's bucket?  If so, return it.
    if (LLVM_LIKELY(KeyInfoT::isEqual(Val, ThisBucket->getFirst()))) {
      FoundBucket = ThisBucket;
      return true;
    }

    // If we found an empty bucket, the key doesn't exist in the set.
    // Insert it and return the default value.
    // 如果我们发现一个空存储桶，则该键在集合中不存在。
    // 插入并返回默认值。（这里没看到插入事件呀？）
    
    if (LLVM_LIKELY(KeyInfoT::isEqual(ThisBucket->getFirst(), EmptyKey))) {
      // If we've already seen a tombstone while probing, 
      // fill it in instead of the empty bucket we eventually probed to.
      // 如果我们在探测时已经看到了 tombstone，请填写它，而不是我们最终探测到的空桶。
      FoundBucket = FoundTombstone ? FoundTombstone : ThisBucket;
      return false;
    }

    // If this is a tombstone, remember it.  
    // If Val ends up not in the map, 
    // we prefer to return it than something that would require more probing.
    // 如果这是一个 tombstone，请记住它。
    // 如果 Val 最终不在 map 中，我们宁愿返回它，而不是需要更多探测的东西。
    // Ditto for zero values.
    // 同上零值。
    if (KeyInfoT::isEqual(ThisBucket->getFirst(), TombstoneKey) &&
        !FoundTombstone)
      // Remember the first tombstone found.
      // 记住找到的第一个 tombstone
      FoundTombstone = ThisBucket;
      
    // 如果 isPurgeable 返回 true
    if (ValueInfoT::isPurgeable(ThisBucket->getSecond())  &&  !FoundTombstone)
      FoundTombstone = ThisBucket;

    // Otherwise, it's a hash collision or a tombstone, 
    // continue quadratic probing.
    // 否则，它是哈希冲突或 tombstone，继续进行二次探测。
    if (ProbeAmt > NumBuckets) {
      FatalCorruptHashTables(BucketsPtr, NumBuckets);
    }
    
    // ProbeAmt 做了自增
    BucketNo += ProbeAmt++;
    BucketNo &= (NumBuckets-1);
  }
}
```

##### void incrementNumEntries()
```c++
// 加 1
void incrementNumEntries() {
  setNumEntries(getNumEntries() + 1);
}
```

#### void shrink_and_clear()
```c++
void shrink_and_clear() {
  // 记录旧的 NumEntries。
  unsigned OldNumEntries = NumEntries;
  // 循环析构 Buckets 里面的 BucketT
  this->destroyAll();

  // Reduce the number of buckets.
  // 减少桶的数量。
  unsigned NewNumBuckets = 0;
  if (OldNumEntries)
    // #define MIN_BUCKETS 4 最小减到 4
    // 取得缩小后的容量
    NewNumBuckets = std::max(MIN_BUCKETS, 1 << (Log2_32_Ceil(OldNumEntries) + 1));
  
  if (NewNumBuckets == NumBuckets) {
    // 初始化为 0 状态
    this->BaseT::initEmpty();
    return;
  }
  
  // 释放旧数据
  operator delete(Buckets);
  
  // 根据 NewNumBuckets 申请空间并进行初始化
  init(NewNumBuckets);
}
```

##### inline unsigned Log2_32_Ceil(uint32_t Value)
```c++
/// Log2_32_Ceil - This function returns the ceil log base 2 of the specified value, 
/// 32 if the value is zero. (32 bit edition).
/// Ex. Log2_32_Ceil(32) == 5, Log2_32_Ceil(1) == 0, Log2_32_Ceil(6) == 3

// 该函数返回指定值的以 2 为基的 log 值并向上取整。如果该值是 0，则返回 32。
// 例如：Log2_32_Ceil(32) 结果是 5，Log2_32_Ceil(1) 结果是 0，
// Log2_32_Ceil(6) 结果是 3

inline unsigned Log2_32_Ceil(uint32_t Value) {
  return 32-CountLeadingZeros_32(Value-1);
}
```
&emsp;到这里 `DenseMap` 的代码就全部分析完了。包含哈希表的根据指定容量申请空间（`allocateBuckets`）初始化（`init`）、复制（`copyFrom`）、扩容（`grow`）、收缩（`shrink_and_clear`）。下面我们进入 `DenseMapBase` 的分析。

## 参考链接
**参考链接:🔗**
+ [llvm中的数据结构及内存分配策略 - DenseMap](https://blog.csdn.net/dashuniuniu/article/details/80043852)
+ [构造哈希表之二次探测法](https://blog.csdn.net/xyzbaihaiping/article/details/51607770)
+ [Objective-C 引用计数原理](http://yulingtianxia.com/blog/2015/12/06/The-Principle-of-Refenrence-Counting/)
+ [C++语法之友元函数、友元类](https://ityongzhen.github.io/C++语法之友元函数、友元类.html/#more)
+ [static_cast](https://baike.baidu.com/item/static_cast/4472966?fr=aladdin)
+ [reinterpret_cast](https://baike.baidu.com/item/reinterpret_cast/9303204?fr=aladdin)
+ [浅谈std::forward](https://zhuanlan.zhihu.com/p/92486757)
+ [C++11 std::move和std::forward](https://www.jianshu.com/p/b90d1091a4ff)
+ [实现 std::is_convertible](https://zhuanlan.zhihu.com/p/98384465)
+ [C++中的explicit关键字](https://blog.csdn.net/chollima/article/details/3486230?utm_medium=distribute.pc_relevant.none-task-blog-BlogCommendFromMachineLearnPai2-2.add_param_isCf&depth_1-utm_source=distribute.pc_relevant.none-task-blog-BlogCommendFromMachineLearnPai2-2.add_param_isCf)
+ [C++ 内存管理之重载operator new 和operator delete](https://blog.csdn.net/u014303647/article/details/80328317)
+ [实现 std::is_base_of](https://zhuanlan.zhihu.com/p/98106799)
+ [C++：std::initializer_list](https://blog.csdn.net/dragongiri/article/details/87712958)
