#  iOS底层-isa结构(isa_t)
上一节学习了 isa 的指向。那么 isa 的结构具体是什么样的呢？从源码中来着手研究。
## 一、位域
isa 的结构是一个联合体+位域的形式。
示例：
坦克大战，定义方向有上下左右状态。
> 常见写法：添加 4 个变量。
```
@interface HHTank : NSObject

@property (nonatomic, assign) BOOL left;
@property (nonatomic, assign) BOOL right;
@property (nonatomic, assign) BOOL top;
@property (nonatomic, assign) BOOL bottom;

@end
```
> 位域的方式
定义联合体，只需要一个 char 的长度就可以表示 4 个方向
```
@interface HHTank : NSObject {
    @public
    union {
        uintptr_t direction;
        
        struct {
            uintptr_t left : 1;
            uintptr_t right : 1;
            uintptr_t top : 5; //这里定义为5  只是想说  长度可以根据不同的需求去自定义
            uintptr_t bottom : 1;
        };
        
    } _tankDirection;
}

@end
```
这样只需要对 left/right 等进行相应的赋值就可以满足需求.
具体赋值方法：
```
HHTank *tank = [HHTank new];

// 方法1：
tank->_tankDirection.direction = 0x81;
tank->_tankDirection.direction = 0b01111101;

// 方法2：
// tank->_tankDirection.left = YES; // YES 强转之后为 1，
// tank->_tankDirection.top = 31; // 0 ～ 32
// tank->_tankDirection.bottom = 0b1; // 二进制赋值

NSLog(@"🐶🐶🐶 left = %@ top = %@ right = %@ bottom = %@", @(tank->_tankDirection.left), @(tank->_tankDirection.top), @(tank->_tankDirection.right), @(tank->_tankDirection.bottom));

// 打印:
🐶🐶🐶 left = 1 top = 31 right = 0 bottom = 0
```
联合体的优势:
1. 联合体和结构体写法上有些类似，但是注意区分。
2. 联合体的所有信息公用一块内存，起到节省内存的作用。
位域的作用：
直观的表达取值范围，可以直接拿到相应的值。

## 二、isa 结构
可以从源码中找到相关内容:
位置 `Project Headers/objc-private.h/struct objc_object/initIsa`
 
 首先在 `alloc` 函数执行到开辟空间完毕后会执行 `initIsa` 函数。`initIsa` 是定义在 `struct objc_object` 结构体中的函数：
```
struct objc_object {
private:
    isa_t isa; // union isa_t 这里 isa 是一个联合体，不再是 struct objc_class * 的结构体指针。

public:

    // ISA() assumes this is NOT a tagged pointer object
    Class ISA();

    // rawISA() assumes this is NOT a tagged pointer object or a non pointer ISA
    Class rawISA();

    // getIsa() allows this to be a tagged pointer object
    Class getIsa();
    
    uintptr_t isaBits() const;
    
    // initIsa() should be used to init the isa of new objects only.
    // initIsa() 应该仅用于初始化新对象的 isa。
    
    // If this object already has an isa, use changeIsa() for correctness.
    // 如果此对象已经有 isa，则应该使用 changeIsa() 来确保正确性。
    
    // initInstanceIsa(): objects with no custom RR/AWZ
    // 没有自定义 RR/AWZ 的对象 （RR 表示 retain/release 等方法）
    
    // initClassIsa(): class objects
    // 类对象初始化 isa
    
    // initProtocolIsa(): protocol objects
    // 协议对象初始化 isa
    
    // initIsa(): other objects
    // 其他对象，指非指针且没有 C++ 析构函数 // initIsa(cls, false, false);
    // 后面两个参数默认是 false
    
    void initIsa(Class cls /*nonpointer=false*/);
    void initClassIsa(Class cls /*nonpointer=maybe*/);
    void initProtocolIsa(Class cls /*nonpointer=maybe*/);
    void initInstanceIsa(Class cls, bool hasCxxDtor);
    ...
}
```
initIsa 函数实现:
```
inline void 
objc_object::initIsa(Class cls, bool nonpointer, bool hasCxxDtor) 
{ 
    ASSERT(!isTaggedPointer()); 
    
    if (!nonpointer) {
        isa = isa_t((uintptr_t)cls);
    } else {
        ASSERT(!DisableNonpointerIsa);
        ASSERT(!cls->instancesRequireRawIsa());
        
        // isa_t 自动变量
        isa_t newisa(0);

#if SUPPORT_INDEXED_ISA
        ASSERT(cls->classArrayIndex() > 0);
        newisa.bits = ISA_INDEX_MAGIC_VALUE;
        // isa.magic is part of ISA_MAGIC_VALUE
        // isa.nonpointer is part of ISA_MAGIC_VALUE
        newisa.has_cxx_dtor = hasCxxDtor;
        newisa.indexcls = (uintptr_t)cls->classArrayIndex();
#else
        // 当前平台执行此分支
        // #   define ISA_MAGIC_VALUE 0x000001a000000001ULL
        // 初始化 bits
        newisa.bits = ISA_MAGIC_VALUE;
        // isa.magic is part of ISA_MAGIC_VALUE
        // isa.nonpointer is part of ISA_MAGIC_VALUE
        
        // 是否有 C++ 析构函数
        newisa.has_cxx_dtor = hasCxxDtor;
        
        // 类地址放在 shiftcls 里面
        newisa.shiftcls = (uintptr_t)cls >> 3;
#endif

        // This write must be performed in a single store in some cases
        // (for example when realizing a class because other threads
        // may simultaneously try to use the class).
        // fixme use atomics here to guarantee single-store and to
        // guarantee memory order w.r.t. the class index table
        // ...but not too atomic because we don't want to hurt instantiation
        
        // 赋值给 isa
        isa = newisa;
    }
}
```
isa 初始化完毕。
isa 是一个 union isa_t 类型的联合体，下面看一下 isa_t 的定义：
```

union isa_t {
    isa_t() { }
    isa_t(uintptr_t value) : bits(value) { }
    
    // 都占用 8 个字节
    Class cls; // struct objc_class *
    uintptr_t bits; // typedef unsigned long uintptr_t;
    
#if defined(ISA_BITFIELD)
    
    struct {
        ISA_BITFIELD;  // defined in isa.h
    };
    
#endif
};
```
`ISA_BITFIELD` 是 `isa.h` 文件里的宏定义，(根据架构不同，内容不同)，分别针对 PC 端 `__x86_64` 和 手机端 64 位 `__arm64__`。（大概针对的就是机器 CPU 的架构，那 ARM 芯片的 mac 电脑发售后，会不会针对它再定义不同的宏）

这里我们只看 __arm64__：
```
union isa_t {
    isa_t() { }
    isa_t(uintptr_t value) : bits(value) { }

    Class cls;
    uintptr_t bits;

    struct {
        uintptr_t nonpointer        : 1;                                       \
        uintptr_t has_assoc         : 1;                                       \
        uintptr_t has_cxx_dtor      : 1;                                       \
        uintptr_t shiftcls          : 33; /*MACH_VM_MAX_ADDRESS 0x1000000000*/ \
        uintptr_t magic             : 6;                                       \
        uintptr_t weakly_referenced : 1;                                       \
        uintptr_t deallocating      : 1;                                       \
        uintptr_t has_sidetable_rc  : 1;                                       \
        uintptr_t extra_rc          : 19
    };
};
```
对 isa 初始化的时候，对 bits 有一个默认的赋值 `#   define ISA_MAGIC_VALUE 0x000001a000000001ULL`，转换为二进制后是:

[ISA_MAGIC_VALUE](图片地址...)

bits 是 64 位（8 个字节），struct 是位域:
+ nonpointer
`nonpointer : 1;` (在 bits 的 64 位中，第 0 位用于 `nonpointer` 信息存储 )
表示是否对 isa 指针开启指针优化；0 代表纯 isa 指针，1 代表不止是类对象指针，还包含了类信息、对象的引用计数等；

+ has_assoc
`has_assoc : 1;` (在 bits 的 64 位中，第 1 位用于 `has_assoc` 信息存储)
表示有没有关联对象，0 没有，1 有

+ has_cxx_dtor
`has_cxx_dtor : 1;` (在 bits 的 64 位中，第 2 位用于 `has_cxx_dtor` 信息存储)
表示该对象是否有 C++ 或者 Objc 的析构器，如果有析构函数，则需要做析构逻辑，如果没有，则可以更快的释放对象。
+ shiftcls
`shiftcls : 33;` (在 bits 的 64 位中，第 3-35 位用于 `shiftcls` 信息存储)
表示存储类指针的值。开启指针优化的情况下，在 arm64 架构中用这 33 位空间来存储类指针。（即那个全局唯一的类对象的地址）
+ magic
`magic : 6;` (在 bits 的 64 位中，第 36-41 用于 `magic` 信息存储)
用于调试器判断当前对象是真的对象还是没有初始化的空间。
+ weakly_referenced
`weakly_referenced : 1;` (在 bits 的 64 位中，第 42 位用于 weakly_referenced 信息存储)
表示该对象是否被指向或者曾经被指向一个 ARC 弱引用变量，没有弱引用的对象可以更快的释放。
+ deallocating
`deallocating : 1;` (在 bits 的 64 位中，第 43 位用于 deallocating 信息存储)
标识该对象是否正在释放内存。
+ has_sidetable_rc
`has_sidetable_rc : 1;`
当对象引用计数大于 10 时，则需要借用该变量存储进位
+ extra_rc
`extra_rc : 19;`
当表示该对象的引用计数数值，实际上是引用计数值减 1，例如，如果对象的引用计数为 10，那么 extra_rc 为 9，如果引用计数大于 10，则需要使用上面提到的 has_sidetable_rc。

### 三、结构体（struct）与联合体（union）

union
1、可以定义多个成员，大小由最大的成员的大小决定。
2、成员共享同一块大小的内存，一次只能使用其中的一个成员。
3、对某一个成员赋值，会覆盖其他成员的值（也不奇怪，因为他们共享一块内存。但前提是成员所占字节数相同，当成员所占字节数不同时只会覆盖相应字节上的值，比如对char成员赋值就不会把整个int成员覆盖掉，因为char只占一个字节，而int占四个字节）
4、联合体的成员存放顺序是所有成员都从低地址开始存放的。
简而言之： union的特点：共用一块内存，大小由最长的那个成员决定
对某一个成员赋值，会影响其他成员的值
结构体
本质上是多个变量集合到一起，多个变量是同时存在的,互不影响。总体的大小是各个变量值所在内存大小的和（由于内存对齐的原则，总体大小总是>=这个和值）。

**参考链接:🔗**
[iOS底层-isa结构(isa_t)](https://www.jianshu.com/p/6b6bf1c27d8e)

