# iOS 从 libclosure-74 源码来研究 Block 实现原理(2)

> 上篇分析了关于 `Block` 的各种理论，还没有触及 `libclosure-74` 的源码细节，本篇则针对上篇结束时提及的问题从源码角度来一一解读。⛽️⛽️

## 前期准备
&emsp;我们先创建一个如下 `NSObject` 的 `category`，并且在 `Compile Sources` 中把 `category` 的 `.m` 文件的 `Compiler Flags` 标记为 `-fno-objc-arc`，其中的 `retainCountForARC` 函数可以帮助我们在 `ARC` 下调用对象的 `retainCount` 方法，然后根据返回的引用计数值来判断外部对象是否被 `block` 持有了。
```c++
// NSObject+RetainCountForARC.h 
@interface NSObject (RetainCountForARC)

- (NSUInteger)retainCountForARC;

@end

// NSObject+RetainCountForARC.m
#import "NSObject+RetainCountForARC.h"
@implementation NSObject (RetainCountForARC)

- (NSUInteger)retainCountForARC {
    return [self retainCount];
}

@end
```
## `block.h` 
&emsp;`block.h` 声明了四个函数接口，以及两个 `block` 的初始类型。
### `_Block_copy`
```c++
// Create a heap based copy of a Block or simply add a reference to an existing one.
// This must be paired with Block_release to recover memory,
// even when running under Objective-C Garbage Collection.

// 创建基于堆的 block 副本，或仅添加对现有 block 的引用。
// 使用时必须与 Block_release 配对使用来恢复内存，
// 即使在 Objective-C 垃圾回收下运行也是如此。

BLOCK_EXPORT void *_Block_copy(const void *aBlock) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_3_2);
```
### `_Block_release`
```c++
// Lose the reference, and if heap based and last reference, recover the memory
// 释放引用，如果是堆 Block 且释放的是最后一个引用，释放引用后并释放内存。
//（类似 ARC 的 release 操作，先是减少引用计数，如果减少到 0 了，则执行 dealloc）

BLOCK_EXPORT void _Block_release(const void *aBlock) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_3_2);
```
### `_Block_object_assign`
```c++
// Used by the compiler. Do not call this function yourself.
// 由编译器主动调用，不要自己调用此函数。

BLOCK_EXPORT void _Block_object_assign(void *, const void *, const int) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_3_2);
```
### `_Block_object_dispose`
```c++
// Used by the compiler. Do not call this function yourself.
// 由编译器主动调用，不要自己调用此函数。
BLOCK_EXPORT void _Block_object_dispose(const void *, const int) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_3_2);
```
### `_NSConcreteGlobalBlock/_NSConcreteStackBlock`
```c++
BLOCK_EXPORT void * _NSConcreteGlobalBlock[32] __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_3_2);
BLOCK_EXPORT void * _NSConcreteStackBlock[32] __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_3_2);
```
&emsp;这里只把 `Global` 和 `Stack` 类型的 `Block` 的声明放在了 `.h` 文件中，`Block` 类型除了这两种之外，还有声明在 `Block_private.h` 中的多个其它类型。而这里仅仅把 `Global` 和 `Stack` 放出来，是因为当我们构建 `block` 时，`block` 的起始类型只可能是 `Global` 和 `Stack` 类型中的一种，然后其它几种 `Block` 类型都是 `_NSConcreteStackBlock` 类型的 `block` 执行复制（调用 `_Block_copy` 函数）时根据 `block` 定义内容不同来动态指定的。（`Global` 类型的 `block`，执行复制操作直接返回它自己）
&emsp;在上一篇我们使用 `clang` 转换 `block` 定义时已经见到过，`block` 所属的类型是由 `struct __block_impl` 的 `void *isa` 指针来指向的。下面我们对 `Block` 的 `6` 种类型来进行解析。

#### `Block 类型`
&emsp;在 `libclosure-74/Block_private.h Line 499` 中声明了所有 `Block` 类型：
```c++
// the raw data space for runtime classes for blocks class+meta used for stack, 
// malloc, and collectable based blocks.
// block 的 class + meta 的数据空间。

BLOCK_EXPORT void * _NSConcreteMallocBlock[32] __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_3_2);
BLOCK_EXPORT void * _NSConcreteAutoBlock[32] __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_3_2);
BLOCK_EXPORT void * _NSConcreteFinalizingBlock[32] __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_3_2);
BLOCK_EXPORT void * _NSConcreteWeakBlockVariable[32] __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_3_2);
// declared in Block.h
// BLOCK_EXPORT void * _NSConcreteGlobalBlock[32];
// BLOCK_EXPORT void * _NSConcreteStackBlock[32];
```
&emsp;其中 `_NSConcreteGlobalBlock`、`_NSConcreteStackBlock`、`_NSConcreteMallocBlock` 是三种最常见的类型，下面我们也会仔细分析这三种类型，另外的 `_NSConcreteFinalizingBlock`、`_NSConcreteAutoBlock`、`_NSConcreteWeakBlockVariable` 三种类型只在 `GC` 环境下使用。

+ 以下情况 `block` 会初始化为 `_NSConcreteGlobalBlock`：（要求此 `block` 以及其内部嵌套的 `block` 符合以下条件）
1. `MRC` 或者 `ARC` 下，未截获任何外部变量时。
```c++
{
    LGPerson *person = [[LGPerson alloc] init];
    
    // 内层 block 根据外层 block 输入的指数计算出 2 的幂并返回
    NSLog(@"%@", ^(int exponent){ return ^{ return pow(2, exponent); }(); });
    
    // 打印 🖨️：
    // <__NSGlobalBlock__: 0x106763048>
    
    // 内部嵌套一个 global Block，如果内部嵌套了一个 stack block 的话，外层 block 的类型也会是 stack block
    // 如 MRC 下：（MRC 下 block 截获外部的自动变量时类型时 Stack Block，ARC 下则是 Malloc Block）
    NSLog(@"%@", ^(int exponent){ ^{ NSLog(@"%@", person); }; });
    
    // 打印 🖨️：
    // MRC 下：<__NSStackBlock__: 0x7ffee5a230f0>
    // ARC 下：<__NSMallocBlock__: 0x600001cd7120> // 同样的代码在 xcode 11.3.1 上打印是 <__NSStackBlock__: 0x7ffee8775170>
    
    // 这里即使发生了赋值操作，因为右侧是 Global block，所以 blk 依然是 Global block
    double (^blk)(int exponent) = ^(int exponent){ return ^{ return pow(2, exponent); }(); };
    NSLog(@"%@", blk);
    NSLog(@"%f", blk(4));
    
    // 打印 🖨️：
    // <__NSGlobalBlock__: 0x106763088>
    // 16.000000
    
    NSLog(@"🍎🍎🍎:  %ld", [person retainCountForARC]);
    
    // 打印 🖨️：
    // MRC 下： 🍎🍎🍎:  1
    // ARC 下： 🍎🍎🍎:  2 // 被 block 持有 1 次
}

// 打印:
// ARC 下：🍀🍀🍀 LGPerson dealloc
// MRC 需要自己主动调用一次 release 操作
```
2. `MRC` 或 `ARC` 下仅截获 **全局变量**、**静态全局变量**、**静态局部变量** 时。
```c++
static int static_val = 14;
NSLog(@"%@", ^{ ^{ NSLog(@"%d", global_static_val); }; });
NSLog(@"%@", ^{ ^{ NSLog(@"%d", global_val); }; });
NSLog(@"%@", ^{ ^{ NSLog(@"%d", static_val); }; });
// 打印：
<__NSGlobalBlock__: 0x10bccc040>
<__NSGlobalBlock__: 0x10bccc060>
<__NSGlobalBlock__: 0x10bccc080>
```
+ 以下情况 `block` 会初始化为 `_NSConcreteStackBlock`：
1. `MRC` 下截获 **外部局部变量** 时。
```c++
{    
    LGPerson *person = [[LGPerson alloc] init];
    NSLog(@"%@", ^{ NSLog(@"%@", person);});
    NSLog(@"🍎🍎🍎:  %ld", [person retainCountForARC]);
}
// MRC 下打印：
// <__NSStackBlock__: 0x7ffee0c510f0>
// 🍎🍎🍎:  1 // MRC 下栈区 block 不持有外部局部变量

// ARC 下打印：
// <__NSMallocBlock__: 0x6000022126d0>
// 🍎🍎🍎:  2 // ARC 下堆区 block 持有外部局部变量
// 🍀🍀🍀 LGPerson dealloc
```
2. 在 `xcode 11.3.1` 也是 `_NSConcreteStackBlock`，同样的代码在 `xcode 12` 下是 `_NSConcreteMallocBlock`。

+ 以下情况 `block` 为转化为 `_NSConcreteMallocBlock`：
1. `MRC` 下 `_NSConcreteStackBlock` 调用 `copy` 函数（`_Block_copy`），`block` 的 `isa` 会被转换为 `_NSConcreteMallocBlock`（`result->isa = _NSConcreteMallocBlock;`）。
```c++
{
    LGPerson *person = [[LGPerson alloc] init];
    void (^blk)(void) = [^{ NSLog(@"%@", person);} copy];
    NSLog(@"%@", blk);
    NSLog(@"🍎🍎🍎:  %ld", [person retainCountForARC]);
}
// MRC 下打印：
// <__NSMallocBlock__: 0x600002822940>
// 🍎🍎🍎:  2 // 栈区 block 被复制到堆区时 持有 person 对象

// ARC 下打印：（即使不调用 copy 函数，也是同样的打印）
// <__NSMallocBlock__: 0x60000227f8a0>
// 🍎🍎🍎:  3 // 1): person 持有。 2): 等号右边 block 持有。 3): 等号左边 block 持有。
// 🍀🍀🍀 LGPerson dealloc
```
+ 在 `GC` 环境下，当 `block` 被复制时，如果 `block` 有 `ctors & dtors` 时，则会转换为 `_NSConcreteFinalizingBlock` 类型，反之，则会转换为 `_NSConcreteAutoBlock` 类型。
+ 在 `GC` 环境下，当对象被 `__weak __block` 修饰，且从栈复制到堆时，`block` 会被标记为 `_NSConcreteWeakBlockVariable` 类型。

## `Block_private.h`
&emsp;`Block_private.h` 中定义了 `struct Block_byref`，作为 `block` 的数据结构，对应上篇我们使用 `clang` 转换 `block` 得到的 `struct __main_block_impl_0`。定义了 `struct Block_byref`，作为 `__block` 变量的数据结构，对应上篇我们使用 `clang` 转换 `__block` 变量得到的 `struct __block_byref_val_0`。同时还定义了一些函数接口，下面进行详细分析。

### `Block_layout->flags`
&emsp;`struct Block_layout` 的成员变量 `flags` 类型是 `volatile int32_t` 共 `32` 位，下面的枚举值对应 `flags` 的某些位来标记 `Block_layout` 是否正在释放、引用计数的掩码、`block` 的类型、以及 `block` 的 `DESCRIPTOR` 信息等。
```c++
// Values for Block_layout->flags to describe block objects.
// 用于描述 block 对象的 Block_layout->flags 的值。

enum {
    BLOCK_DEALLOCATING =      (0x0001),  // runtime 用于运行时，标记 block 正在进行释放，
                                                    // 转化为二进制共 16 位，前 15 位是 0，最后一位是 1
                                                    
    BLOCK_REFCOUNT_MASK =     (0xfffe),  // runtime 用于运行时，block 引用计数的掩码
                                                    // 转化为二进制共 16 位，前 15 位是 1，最后一位是 0
                                                    
    // 16 位到 23 位 未使用
    
    BLOCK_NEEDS_FREE =        (1 << 24), // runtime 用于运行时，1 左移 24 位，第 24 位是 1，标识 block 是 堆 Block（24 位是 1）
    
    // 用于编译时，标识 block 有 copy dispose 助手
    
    // 判断 Block 是否有 copy_dispose 助手 即 description2 中的 copy 和 dispose 函数
    // 对应上篇 clang 转化中的 static struct __main_block_desc_0 中的
    // void (*copy)(struct __main_block_impl_0*, struct __main_block_impl_0*) 和
    // void (*dispose)(struct __main_block_impl_0*);
    
    BLOCK_HAS_COPY_DISPOSE =  (1 << 25), // compiler（25 位是 1）
    
    BLOCK_HAS_CTOR =          (1 << 26), // compiler: helpers have C++ code 标记 block 有 ctors & dtors（26 位是 1）
    BLOCK_IS_GC =             (1 << 27), // runtime 用于运行时，标记 block 是否处于 GC 环境（27 位是 1）
    
    BLOCK_IS_GLOBAL =         (1 << 28), // compiler 用于编译时，1 左移 28 位，第 28 位是 1，标识 block 是 全局 Block（28 位是 1）
    
    BLOCK_USE_STRET =         (1 << 29), // compiler: undefined if !BLOCK_HAS_SIGNATURE（29 位是 1）
    
    BLOCK_HAS_SIGNATURE  =    (1 << 30), // compiler 用于编译时，标记 block 有签名，可用于 block hook（30 位是 1）
    BLOCK_HAS_EXTENDED_LAYOUT=(1 << 31)  // compiler 用于编译时，标记 block 是否有延展布局（31 位是 1）
};
```
### `BLOCK_DESCRIPTOR`
&emsp;此处三个结构体各自包含一些描述信息或者功能，不同的 `block` 会包含其中一些或者全部，当包含某些描述信息或者功能时，它们会被追加到我们上篇看到的 `static struct __main_block_desc_0` 中，其中 `struct Block_descriptor_1` 所包含的 `reserved`（保留字段） 和 `size`（`block` 结构体所占用内存大小，如 `sizeof(struct __main_block_impl_0)`），它们两个是所有 `block` 都会包含的，然后像是 `struct Block_descriptor_2` 中的 `copy` 和 `dispose` 则是在 `block` 中截获外部对象类型或者 `__block` 变量时才会有的，且它们都是在编译时根据 `block` 的定义来确定的。
```c++
#define BLOCK_DESCRIPTOR_1 1

// block 结构体的默认描述信息
struct Block_descriptor_1 {
    uintptr_t reserved; // 保留字段
    uintptr_t size; // block 结构体的大小
};

#define BLOCK_DESCRIPTOR_2 1

// block 结构体包含 copy 和 dispose 函数
struct Block_descriptor_2 {
    // requires BLOCK_HAS_COPY_DISPOSE
    // Block_layout->flags & BLOCK_HAS_COPY_DISPOSE == 1
    
    // typedef void(*BlockCopyFunction)(void *, const void *);
    // typedef void(*BlockDisposeFunction)(const void *);
    
    BlockCopyFunction copy;
    BlockDisposeFunction dispose;
};

#define BLOCK_DESCRIPTOR_3 1

// block 结构体存在 signature 和 layout
struct Block_descriptor_3 {
    // requires BLOCK_HAS_SIGNATURE
    // Block_layout->flags & BLOCK_HAS_SIGNATURE == 1
    
    const char *signature;
    const char *layout;     // contents depend on BLOCK_HAS_EXTENDED_LAYOUT
};
```
### `struct Block_layout`
&emsp;`block` 的本质正是 `struct Block_layout`。
```c++
struct Block_layout {
    void *isa; // block 所属类型
    volatile int32_t flags; // contains ref count 包含引用计数等一些信息
    int32_t reserved; // block 的保留信息
    
    // typedef void(*BlockInvokeFunction)(void *, ...);
    BlockInvokeFunction invoke; // 函数指针，指向 block 要执行的函数（即 block 定义中花括号中的表达式）
    
    // 上篇中我们看到上面四个字段被综合放在了 struct __block_impl 中
    
    // block 附加描述信息，默认所有 block 都包含 Block_descriptor_1 中的内容
    struct Block_descriptor_1 *descriptor;
    
    // 主要保存了内存 size 大小以及 copy 和 dispose 函数的指针及签名和 layout 等信息，
    // 通过源码可发现，layout 中只包含了 Block_descriptor_1，
    // 并未包含 Block_descriptor_2 和 Block_descriptor_3，
    // 这是因为在捕获不同类型变量或者没用到外部变量时，编译器会改变结构体的结构，
    // 按需添加 Block_descriptor_2 和 Block_descriptor_3，
    // 所以才需要 BLOCK_HAS_COPY_DISPOSE 和 BLOCK_HAS_SIGNATURE 等枚举来判断
    
    // imported variables
    // capture 的外部变量，
    // 如果 block 表达式中截获了外部变量，block 结构体中就会有添加相应的成员变量
    // 如果是 __block 变量则添加对应结构体类型为其成员变量，
    // 非 __block 变量则是直接添加对应类型的成员变量。
    // 同时在 Block 的结构体初始化时将使用截获的值或者指针来初始化对应的成员变量。
};
```
### `Block_byref->flags`
&emsp;`struct Block_byref` 的成员变量 `flags` 类型是 `volatile int32_t` 共 `32` 位，下面的枚举值对应 `flags` 的某些位来标记 `Block_byref` 是否需要释放、是否有 `copy` 和 `dispose`、`Layout` 的掩码、`__block` 修饰的变量类型等等。
```c++
// Values for Block_byref->flags to describe __block variables.
// 用于描述 __block 变量的 Block_byref->flags 的值。

// 变量在被 __block 修饰时由编译器来生成 struct Block_byref 实例。 
enum {
    // Byref refcount must use the same bits as Block_layout's refcount.
    // Byref refcount 必须使用与 Block_layout 的 refcount 相同的位
    
    // BLOCK_DEALLOCATING =      (0x0001),  // runtime
    // BLOCK_REFCOUNT_MASK =     (0xfffe),  // runtime

    BLOCK_BYREF_LAYOUT_MASK =       (0xf << 28), // compiler 掩码 0b1111 左移 28 位
    BLOCK_BYREF_LAYOUT_EXTENDED =   (  1 << 28), // compiler
    BLOCK_BYREF_LAYOUT_NON_OBJECT = (  2 << 28), // compiler
    BLOCK_BYREF_LAYOUT_STRONG =     (  3 << 28), // compiler
    BLOCK_BYREF_LAYOUT_WEAK =       (  4 << 28), // compiler
    BLOCK_BYREF_LAYOUT_UNRETAINED = (  5 << 28), // compiler

    BLOCK_BYREF_IS_GC =             (  1 << 27), // runtime 用于运行时，表示当前时 GC 环境

    BLOCK_BYREF_HAS_COPY_DISPOSE =  (  1 << 25), // compiler 用于编译时，表示 Block_byref 含有 copy 和 dispose
    BLOCK_BYREF_NEEDS_FREE =        (  1 << 24), // runtime 用于运行时，表示是否需要进行释放操作
};
```
### `struct Block_byref`
&emsp;`__block` 变量的本质正是 `struct Block_byref`。
```c++
struct Block_byref {
    void *isa; // 指向父类，一般直接指向 0
    
    // __block 结构体实例在栈中时指向自己，
    // 截获 __block 变量的 栈区 block 执行 copy 后，
    // 栈中 __block 结构体实例的 __forwarding 指向堆中的 byref（__block 变量），
    // 堆中 __block 结构体实例的 __forwarding 指向自己
    
    struct Block_byref *forwarding; // 指向自己的指针 
    
    volatile int32_t flags; // contains ref count
    uint32_t size; // __block 结构体所占内存大小
};
```
### `struct Block_byref_2`
&emsp;这里有点像上面的 `block` 结构体的描述信息，根据 `block` 的定义追加不同的描述。这里是当 `__block` 修饰的不同类型时也是追加不同的功能，比如下面的 `Keep` 和 `Destroy` 操作，当 `__block` 修饰的是对象类型时就会追加到 `__block` 的结构体中，而如果修饰的是基本类型的话则不会添加这两个功能。
```c++
struct Block_byref_2 {
    // requires BLOCK_BYREF_HAS_COPY_DISPOSE
    // Block_byref->flags & BLOCK_BYREF_HAS_COPY_DISPOSE == 1
    
    // typedef void(*BlockByrefKeepFunction)(struct Block_byref*, struct Block_byref*);
    // typedef void(*BlockByrefDestroyFunction)(struct Block_byref *);
    
    // 在 ARC 下当截获 __block 对象类型变量的栈区 block 被复制到堆区时，__block 中的对象类型值引用计数会 + 1
    BlockByrefKeepFunction byref_keep;
    
    // 销毁 __block 变量
    BlockByrefDestroyFunction byref_destroy;
};
```
### `struct Block_byref_3`
&emsp;包含有布局扩展，当 `__block` 修饰不同类型的变量时，对应类型的成员变量会追加在 `struct Block_byref` 中。
```c++
struct Block_byref_3 {
    // requires BLOCK_BYREF_LAYOUT_EXTENDED
    // Block_byref->flags & BLOCK_BYREF_LAYOUT_EXTENDED == 1
    
    const char *layout;
};
```
### `Block 截获的外部变量类型`
&emsp;以下枚举值标识 `Block` 截获不同类型的外部变量。
```c++
// Runtime support functions used by compiler when generating copy/dispose helpers.
// 当编译器生成 copy/dispose helpers 时 Runtime 支持的函数.

// Values for _Block_object_assign() and _Block_object_dispose() parameters
// 作为 _Block_object_assign() 和 _Block_object_dispose() 函数的参数.

enum {
    // see function implementation for a more complete description of these fields and combinations.
    // 有关这些字段及其组合的更完整说明，请参见函数实现。
    
    // 0b11
    // 对象类型 
    BLOCK_FIELD_IS_OBJECT   =  3,  // id, NSObject, __attribute__((NSObject)), block, ...
    
    // 0b111
    // block 变量 
    BLOCK_FIELD_IS_BLOCK    =  7,  // a block variable
    
    // 0b1000
    // __block 说明符生成的结构体，持有 __block 变量的堆栈结构 
    BLOCK_FIELD_IS_BYREF    =  8,  // the on stack structure holding the __block variable
    
    // 0b10000
    // 被 __weak 修饰过的弱引用，只在 Block_byref 管理内部对象内存时使用
    // 也就是 __block __weak id; 仅使用 __weak 时，还是 BLOCK_FIELD_IS_OBJECT，
    // 即如果是对象类型，有没有添加 __weak 修饰都是一样的，都会生成 copy 助手
    BLOCK_FIELD_IS_WEAK     = 16,  // declared __weak, only used in byref copy helpers
    
    // 0b1000 0000
    // 在处理 Block_byref 内部对象内存的时候会加一个额外标记，配合上面的枚举一起使用
    BLOCK_BYREF_CALLER      = 128, // called from __block (byref) copy/dispose support routines.
};

enum {
    // 上述情况的整合，以下的任一中情况下编译器都会生成 copy_dispose 助手（即 copy 和 dispose 函数）
    
    BLOCK_ALL_COPY_DISPOSE_FLAGS = 
        BLOCK_FIELD_IS_OBJECT | BLOCK_FIELD_IS_BLOCK | BLOCK_FIELD_IS_BYREF |
        BLOCK_FIELD_IS_WEAK | BLOCK_BYREF_CALLER
};
```
## `runtime.cpp`
&emsp;`Block` 的核心内容的实现。
### `latching_incr_int`
&emsp;以原子方式增加 `block` 的引用计数，实参是 `Block_layout->flags`。
```c++
static int32_t latching_incr_int(volatile int32_t *where) {
    while (1) {
        // 取出旧值
        int32_t old_value = *where;
        
        // 如果 flags & BLOCK_REFCOUNT_MASK == BLOCK_REFCOUNT_MASK 表明 block 目前引用计数达到最大（0xfffe）。
        // BLOCK_REFCOUNT_MASK =     (0xfffe)
        // 0b1111 1111 1111 1110（10 进制值是 65534）
        if ((old_value & BLOCK_REFCOUNT_MASK) == BLOCK_REFCOUNT_MASK) {
            // 直接返回 BLOCK_REFCOUNT_MASK
            return BLOCK_REFCOUNT_MASK;
        }
        
        // 做一次原子性判断其值当前是否被其他线程改动，如果被改动就进入下一次循环直到改动结束后赋值。
        // OSAtomicCompareAndSwapInt 的作用就是在 where 取值与 old_value 相同时，将 old_value + 2 赋给 where。
        // 注: Block 的引用计数是以 flags 的 [1, 15] 位表示的，0 位表示的是 Block 正在进行释放。（BLOCK_DEALLOCATING = (0x0001)）
        // 以 2 为单位，即每次递增 0b10，[1, 15] 区间内每次递增 1
        
        if (OSAtomicCompareAndSwapInt(old_value, old_value+2, where)) {
            // 返回 block 新的引用计数值
            return old_value+2;
        }
    }
}
```
### `latching_incr_int_not_deallocating`
&emsp;以原子方式增加 `block` 的引用计数，实参是 `Block_layout->flags`，与上个函数不同的地方是，增加了判断 `block` 是否正在进行释放，`bool` 返回值表示是否可以增加 `block` 的引用计数。 
```c++
static bool latching_incr_int_not_deallocating(volatile int32_t *where) {
    while (1) {
        int32_t old_value = *where;
        
        // old_value & 0x0001 == 1
        if (old_value & BLOCK_DEALLOCATING) {
            // if deallocating we can't do this
            // 如果 block 正在进行释放，则返回 false 表示不能增加 block 的引用计数
            return false;
        }
        
        // old_value & 0xfffe == 0xfffe
        if ((old_value & BLOCK_REFCOUNT_MASK) == BLOCK_REFCOUNT_MASK) {
            // if latched, we're leaking this block, and we succeed.
            // 如果目前引用计数已达到 BLOCK_REFCOUNT_MASK，则返回 true。
            
            return true;
        }
        
        // 正常增加 block 的引用计数，并返回 true。
        if (OSAtomicCompareAndSwapInt(old_value, old_value+2, where)) {
            // otherwise, we must store a new retained value without the deallocating bit set.
            
            return true;
        }
    }
}
```
### `latching_decr_int_should_deallocate`
&emsp;以原子方式减少 `block` 的引用计数，实参是 `Block_layout->flags`，返回 `true` 表示 `block` 引用计数正常进行了 `-1`，如果返回 `false` 则表示当前 `block` 引用计数是 `BLOCK_REFCOUNT_MASK` 或者是 `0`。
```c++
// return should_deallocate?
// 实参传入 &aBlock->flags
// 是否减小 block 引用
static bool latching_decr_int_should_deallocate(volatile int32_t *where) {
    while (1) {
        int32_t old_value = *where;
        
        // 如果是引用计数为 0xfffe，直接返回 false.
        if ((old_value & BLOCK_REFCOUNT_MASK) == BLOCK_REFCOUNT_MASK) {
            return false; // latched high
        }
        
        // 如果引用计数为 0，直接返回 false.
        if ((old_value & BLOCK_REFCOUNT_MASK) == 0) {
            return false;   // underflow, latch low
        }
        
        // old_value 减去 0b10，并赋值给 new_value，
        // result 临时变量初始化为 false。
        int32_t new_value = old_value - 2;
        bool result = false;
        
        // 如果引用计数为 0x10，将其减 1 后为 0x1（即 BLOCK_DEALLOCATING 的值），表明 block 正在释放，返回 true
        // 如果 old_value & 0xffff == 0x10 
        if ((old_value & (BLOCK_REFCOUNT_MASK|BLOCK_DEALLOCATING)) == 2) {
            new_value = old_value - 1; // 0x1（即 BLOCK_DEALLOCATING 的值）
            result = true; // result 赋值为 true
        }

        // 以原子方式把 new_value 赋值给 where。
        if (OSAtomicCompareAndSwapInt(old_value, new_value, where)) {
            return result;
        }
    }
}
```
### `Framework Callback Routines`
&emsp;`block` 持有对象的 `retain`/`release`/`destructInstance` 操作，默认是空实现，我们主要是使用三个对应的函数指针，它们在编译过程中会根据 `block` 实际截获的类型来指向具体的函数，例如：当 `block` 截获的是对象类型时，`_Block_retain_object` 指针就可指向 `retain` 函数，来对 `block` 截获的对象实例做 `retain` 操作。它们具体的指向有编译器来控制，默认状态则都是空操作。
```c++
/*
Framework callback functions and their default implementations.
框架回调函数及其默认实现。
*/
#if !TARGET_OS_WIN32
#pragma mark Framework Callback Routines
#endif

static void _Block_retain_object_default(const void *ptr __unused) { } // block retain 持有的对象，默认为空
static void _Block_release_object_default(const void *ptr __unused) { } // block release 持有的对象，默认为空
static void _Block_destructInstance_default(const void *aBlock __unused) {} // block 废弃持有的对象时，默认为空

// 对应上面的函数指针
static void (*_Block_retain_object)(const void *ptr) = _Block_retain_object_default;
static void (*_Block_release_object)(const void *ptr) = _Block_release_object_default;
static void (*_Block_destructInstance) (const void *aBlock) = _Block_destructInstance_default;
```
### `_Block_descriptor_1`
&emsp;获取 `block` 的默认描述 `struct Block_descriptor_1`。
```c++
#if 0
static struct Block_descriptor_1 * _Block_descriptor_1(struct Block_layout *aBlock)
{
    return aBlock->descriptor;
}
#endif
```
### `_Block_descriptor_2`
&emsp;获取 `block` 的描述信息 `static struct Block_descriptor_2`，`copy` 和 `dispose` 函数。
```c++
static struct Block_descriptor_2 * _Block_descriptor_2(struct Block_layout *aBlock)
{
    // 根据 Block_layout->flags 来判断是否有 Block_descriptor_2
    // BLOCK_HAS_COPY_DISPOSE =  (1 << 25)，如果没有则直接返回 NULL
    if (! (aBlock->flags & BLOCK_HAS_COPY_DISPOSE)) return NULL;
    
    // 有无 Block_descriptor_2 内容是编译器在编译时根据 block 的定义来确定的，如果有的话会追加到 sruct Block_layout。
    // 查找时则是根据指针偏移来确定其指针位置，有就返回其指针。
    // 首先找到默认 descriptor 的位置，向后移动 sizeof(struct Block_descriptor_1) 的长度，
    uint8_t *desc = (uint8_t *)aBlock->descriptor;
    desc += sizeof(struct Block_descriptor_1); // 指针偏移
    
    return (struct Block_descriptor_2 *)desc;
}
```
### `_Block_descriptor_3`
&emsp;获取 `block` 的描述信息 `static struct Block_descriptor_3`，`signature` 和 `layout`。
```c++
static struct Block_descriptor_3 * _Block_descriptor_3(struct Block_layout *aBlock)
{
    // 根据 BLOCK_HAS_SIGNATURE 来判断是否有 Block_descriptor_3，方式完全同上.
    if (! (aBlock->flags & BLOCK_HAS_SIGNATURE)) return NULL;
    
    uint8_t *desc = (uint8_t *)aBlock->descriptor;
    desc += sizeof(struct Block_descriptor_1);
    
    // 根据 BLOCK_HAS_COPY_DISPOSE 判断是否有 Block_descriptor_2,
    // 如果有则指针偏移越过 Block_descriptor_2
    if (aBlock->flags & BLOCK_HAS_COPY_DISPOSE) {
        desc += sizeof(struct Block_descriptor_2);
    }
    return (struct Block_descriptor_3 *)desc;
}
```
### `_Block_call_copy_helper`
&emsp;执行 `block` 的 `copy` 函数。
```c++
static void _Block_call_copy_helper(void *result, struct Block_layout *aBlock)
{
    // 这里如果返回找到了 Block_descriptor_2，就执行它的 copy 函数，如果没有找到就直接 return。
    // 这个 copy 函数，可理解为上篇的 __main_block_copy_0 函数。
    struct Block_descriptor_2 *desc = _Block_descriptor_2(aBlock);
    if (!desc) return;

    (*desc->copy)(result, aBlock); // do fixup
}
```
### `_Block_call_dispose_helper`
&emsp;
```c++
static void _Block_call_dispose_helper(struct Block_layout *aBlock)
{
    // 这里同上面
    // 这里如果返回找到了 Block_descriptor_2，就执行它的 dispose 函数，如果没有找到就直接 return。
    // 这个 dispose 函数，可理解为上篇的 __main_block_dispose_0 函数
    struct Block_descriptor_2 *desc = _Block_descriptor_2(aBlock);
    if (!desc) return;

    (*desc->dispose)(aBlock);
}
```
### `_Block_copy`
&emsp;最最重要的 `_Block_copy` 函数，我们对 `block` 手动调用的 `copy` 函数，内部就是调用的 `_Block_copy` 函数。
1. 如果 `block` 为堆 `Block`，执行 `copy` 只是增加其引用计数，然后返回原 `Block`。
2. 如果 `block` 为 `GLOBAL Block`，执行 `copy` 操作不做任何处理直接返回自身。
3. 如果是栈 `Block` 则执行 `copy` 操作，把栈区 `Block` 复制到堆区，并把 `block` 截获的外部变量调用 `_Block_object_assign` 函数。（如果是对象类型的话增加其引用计数，如果是 `__block` 变量的话，同样从栈区复制到堆区。）
```c++
// Copy, or bump refcount, of a block.  If really copying, call the copy helper if present.
// 复制或增加 block 的引用计数。如果确实要复制，则调用 copy helper。（如果有）

void *_Block_copy(const void *arg) {
    // 声明一个 Block_layout 结构体类型的指针，如果传入的形参 arg 不存在，则 return NULL。
    struct Block_layout *aBlock;
    if (!arg) return NULL;
    
    // The following would be better done as a switch statement.
    // 以下最好作为 Switch 语句来完成。
    
    // arg 强转为 Block_layout 指针
    aBlock = (struct Block_layout *)arg;
    
    // BLOCK_REFCOUNT_MASK 栈 block
    // BLOCK_NEEDS_FREE 堆 block
    // BLOCK_IS_GLOBAL 全局 block
    
    // 如果 block 为堆 Block，执行 copy 只是增加其引用计数，然后返回原 Block。
    if (aBlock->flags & BLOCK_NEEDS_FREE) {
        // latches on high
        // 增加引用计数
        latching_incr_int(&aBlock->flags);
        
        return aBlock;
    }
    
    // 如果 block 为 GLOBAL Block，执行 copy 操作不做任何处理直接返回自身
    else if (aBlock->flags & BLOCK_IS_GLOBAL) {
        return aBlock;
    }
    else {
        // Its a stack block. Make a copy.
        // 如果是栈 Block 则执行 copy 操作，把栈区 Block 复制到堆区。
        
        // 按原 Block 的内存大小在堆区申请一块相同大小的内存，如果申请失败则返回NULL。
        struct Block_layout *result =
            (struct Block_layout *)malloc(aBlock->descriptor->size);
        if (!result) return NULL;
        
        // memmove() 用于复制位元，将 aBlock 的所有信息 copy 到 result 的位置上。
        // memmove 函数，如果旧空间和新空间有交集，那么以新空间为主，复制完毕，旧空间会被破坏。
        
        // 原型：void *memmove(void* dest, const void* src, size_t count );
        // 头文件：<string.h>
        // 功能：由 src 所指内存区域复制 count 个字节到 dest 所指内存区域。
        // 相关函数：memset、memcpy
        
        memmove(result, aBlock, aBlock->descriptor->size); // bitcopy first
        
#if __has_feature(ptrauth_calls)
        // Resign the invoke pointer as it uses address authentication.
        // 使用地址认证时，请放弃调用指针。
        
        result->invoke = aBlock->invoke;
#endif

        // reset refcount
        // 重制复制到堆区的 block 的引用计数
        
        // BLOCK_DEALLOCATING =      (0x0001),
        // BLOCK_REFCOUNT_MASK =     (0xfffe),
        // 它们两者 ｜ 一下就是: 0xffff
        
        // 将新Block的引用计数置零。
        // BLOCK_REFCOUNT_MASK|BLOCK_DEALLOCATING 就是 0xffff，
        // ~(0xffff)就是0x0000，
        // result->flags 和 0x0000 与等 就将 result->flags 的后 16 位全部置零。
        // 然后将新 Block 标识为 堆 Block 并将其引用计数置为 0b10。
        // ｜2 表示把 后 16 位置为 0x0002，表示引用计数为 0b 0000 0000 0000 0010
        
        result->flags &= ~(BLOCK_REFCOUNT_MASK|BLOCK_DEALLOCATING); // XXX not needed
        
        // BLOCK_NEEDS_FREE = (1 << 24) 把 block 标识为堆 block
        result->flags |= BLOCK_NEEDS_FREE | 2;  // logical refcount 1
        
        // 调用 block 内部的 copy 函数，
        // 当 block 截获外部对象类型或 __block 变量时，
        // 如上篇的 __main_block_desc_0 内部的 __main_block_copy_0 函数实现，
        // 内部 _Block_object_assign((void*)&dst->objc, (void*)src->objc, 8/*BLOCK_FIELD_IS_BYREF*/) 实现，
        // _Block_object_assign 内部会根据 block 截获的不同类型来做处理，例如，如果截获的是对象类型则执行 _Block_retain_object(object)，对象的引用计数 +1;
        // 如果是 __block 变量，则执行 _Block_byref_copy(object) 把其从栈区复制到堆区；如果是 block 的话则 _Block_copy(object) copy 操作；
        // 下面分析 _Block_object_assign 函数时再进行详细分析。
        
        _Block_call_copy_helper(result, aBlock);
        
        // Set isa last so memory analysis tools see a fully-initialized object.
        // 将 isa 指针置为 _NSConcreteMallocBlock。
        result->isa = _NSConcreteMallocBlock;
        
        return result;
    }
}
```
### `_Block_release`
&emsp;`block` 的释放操作，主要针对堆区 `block`，正常情况时引用计数 `-1`，如果需要释放 `block` 时，则也会先处理 `block` 截获的变量，然后最后释放 `block` 所占用的内存空间。如果 `block` 是栈区或者全局 `block`，调用 `_Block_release` 函数则直接 `return`。
```c++
// API entry point to release a copied Block.
// API 入口点以释放复制的 Block。（主要针对的是堆区 block 进行释放，栈区 block 由系统进行释放，全局 block 貌似不进行释放）

void _Block_release(const void *arg) {
    // 将 arg 强转为 Block_layout 指针，如果入参为空则直接 return。
    struct Block_layout *aBlock = (struct Block_layout *)arg;
    if (!aBlock) return;
    
    // 如果是全局 Block 则返回不做处理，global block 在程序结束时释放。
    if (aBlock->flags & BLOCK_IS_GLOBAL) return;
    
    // 如果入参不是堆 Block 则 return 不做处理。
    if (! (aBlock->flags & BLOCK_NEEDS_FREE)) return;
    
    // 返回 true，表示 block 需要进行释放，其它情况的话，则正常减少 block 的引用计数
    if (latching_decr_int_should_deallocate(&aBlock->flags)) {
    
        // 如果 aBlock 含有 copy_dispose 助手就执行 aBlock 中的 dispose 函数，与 copy 中的对应。
        // 实际调用 _Block_object_dispose 来处理 block 截获的变量，
        //（如果是对象类型，则 _Block_release_object(object) 引用计数 -1）
        //（如果是 __block 变量，则执行 _Block_byref_release(object)）
        //（如果是 block，则调用 _Block_release(object)）
        _Block_call_dispose_helper(aBlock);
        
        // block free 前的清理工作，类似 C++ 的析构函数。
        // _Block_destructInstance 默认为空，具体执行在编译时由编译器指向
        // _Block_destructInstance = callbacks->destructInstance;
        
        _Block_destructInstance(aBlock);
        
        // 释放 aBlock 内存
        free(aBlock);
    }
}
```
### `_Block_byref_copy`
&emsp;`struct Block_byref` 的复制操作，比较复杂的是把栈区的 `struct Block_byref` 复制到堆区，如果已经是堆区 `struct Block_byref` 的话只是增加它的引用计数。
```c++
static struct Block_byref *_Block_byref_copy(const void *arg) {
    // 强制转换 arg 为 Block_byref 指针
    struct Block_byref *src = (struct Block_byref *)arg;

    // 此时的操作是把这个栈区的 __block 变量复制到堆中去。
    //（此时把栈 block 复制到堆时连带的调用 _Block_byref_copy 函数）
    if ((src->forwarding->flags & BLOCK_REFCOUNT_MASK) == 0) {
        // src points to stack
        // 当前入参 arg 为栈区的 Block_byref 变量。
        
        // 在堆区申请一块与当前 Block_byref 相同大小的内存，并将 isa 指针置为 NULL。
        struct Block_byref *copy = (struct Block_byref *)malloc(src->size);
        copy->isa = NULL;
        
        // byref value 4 is logical refcount of 2: one for caller, one for stack
        // 将新 byref 的引用计数置为 0b100(逻辑值 2)，一份为调用方、一份为栈持有，所以引用计数为 2。
        // 同时把它标记为 堆区。
        copy->flags = src->flags | BLOCK_BYREF_NEEDS_FREE | 4;
        
        // 然后将当前 byref 和 malloc 的 byref 的 forwading 都指向 堆 byref，然后操作堆栈都是同一份东西。
        // 这两行特关键：正印证那一句，copy 发生后栈区 Block 的 __block 变量的 __forwarding 指向堆中的 __block 变量，
        // 堆中的 __block 的 __forwarding 指向自己
        copy->forwarding = copy; // patch heap copy to point to itself
        src->forwarding = copy;  // patch stack to point to heap copy
        
        // size 赋值
        copy->size = src->size;

        // 上面只是为 堆区的 Block_byref 申请空间并把 struct Block_byref 的内容进行复制，
        // 下面是把栈区的 Block_byref 的 struct Block_byref_2 和 struct Block_byref_3 中的内容追加到堆区 Block_byref 中。
        if (src->flags & BLOCK_BYREF_HAS_COPY_DISPOSE) {
            // Trust copy helper to copy everything of interest
            // If more than one field shows up in a byref block this is wrong XXX
            // 如果 src(入参) byref 含有内存管理的变量即有 copy_dispose 助手，执行此步。
            // src 和 copy 都进行指针偏移，分别找到各自的 Block_byref_2 位置的指针
        
            struct Block_byref_2 *src2 = (struct Block_byref_2 *)(src+1);
            struct Block_byref_2 *copy2 = (struct Block_byref_2 *)(copy+1);
            
            // 把 src2 的 Block_byref_2 的内容赋值给 copy2。
            copy2->byref_keep = src2->byref_keep;
            copy2->byref_destroy = src2->byref_destroy;
            
            // 判断是否还有 Block_byref_3，
            if (src->flags & BLOCK_BYREF_LAYOUT_EXTENDED) {
                // src2 和 copy2 都进行指针偏移，分别找到各自的 Block_byref_3 位置的指针
                struct Block_byref_3 *src3 = (struct Block_byref_3 *)(src2+1);
                struct Block_byref_3 *copy3 = (struct Block_byref_3*)(copy2+1);
                
                // 把 src3 的 Block_byref_3 的内容赋值给 copy3。
                copy3->layout = src3->layout;
            }
            
            // 执行 byref 的 byref_keep 函数(即 _Block_object_assign，不过会加上 BLOCK_BYREF_CALLER 标记)，管理捕获的对象内存。
            // 这里才是进行真的内容复制，例如 __block 截获的是 NSObject *objc 的话，
            // 新复制到堆区的 __block_byref_objc_0 的 NSObject *objc 也指向同一个 NSObject 对象，
            // 所以 byref_keep 函数是把 __block_byref_objc_0 的 NSObject *objc 成员变量的引用计数加 1，
            //（这个 NSObject 对象是我们原始的对象，这里只是被 block 截获了，由于被 block 持有，所有它的引用计数要增加）
            
            (*src2->byref_keep)(copy, src);
        }
        else {
            // Bitwise copy.
            // This copy includes Block_byref_3, if any.
            // 如果捕获的是普通变量，就没有 Block_byref_2，copy+1 和src+1 指向的就是 Block_byref_3，执行字节拷贝。
            // __block 修饰的是 int val = 10，则把 10 的值赋给堆中的 __block_byref_val_0 的 int val 成员变量。
            memmove(copy+1, src+1, src->size - sizeof(*src));
        }
    }
    
    // already copied to heap
    else if ((src->forwarding->flags & BLOCK_BYREF_NEEDS_FREE) == BLOCK_BYREF_NEEDS_FREE) {
        // 如果该 byref 是已经存在于堆，则只需要增加其引用计数。
        //（注意这里是增加 Block_byref 的引用计数，不要和上面的 NSObject *objc 的引用计数搞混了）
        latching_incr_int(&src->forwarding->flags);
    }
    
    // 返回 forwarding，也就是堆区的 __block 变量的地址。
    return src->forwarding;
}
```
### `_Block_byref_release`
&emsp;`struct Block_byref` 的释放操作，
```c++
static void _Block_byref_release(const void *arg) {
    // 强制转换 arg 为 Block_byref 指针
    struct Block_byref *byref = (struct Block_byref *)arg;

    // dereference the forwarding pointer since the compiler isn't doing this anymore (ever?)
    // 取消引用转发指针，因为编译器不再这样做了（曾经？）
    
    // 这里 byref->forwarding 可能还是 byref 自己，如果入参是没发生过复制的栈区的 struct Block_byref 的话，
    // 另外的情况是 byref->forwarding 是堆区的 struct Block_byref 指针，我们只针对这种情况，
    // 如果仅是栈区的 struct Block_byref 的话，则由系统处理其内存
    byref = byref->forwarding;
    
    // 判断 byref 是堆区 struct Block_byref
    if (byref->flags & BLOCK_BYREF_NEEDS_FREE) {
        // 取出 struct Block_byref 的引用计数
        int32_t refcount = byref->flags & BLOCK_REFCOUNT_MASK;
        os_assert(refcount);
        
        // 判断是否需要释放内存，也可能是只需要引用计数 -1 后还不为 0，例如还有别的 block 使用 Block_byref，此时还不能被废弃，
        // 引用计数 -1 后，如果需要释放了，则进入下面的 if
        if (latching_decr_int_should_deallocate(&byref->flags)) {
            // 如果 struct Block_byref 有 dispose 函数的话，
            if (byref->flags & BLOCK_BYREF_HAS_COPY_DISPOSE) {
                // 如果有 copy_dispose 助手就执行 byref_destroy 管理捕获的变量内存。
                
                // 指针偏移找到 Block_byref_2 指针
                struct Block_byref_2 *byref2 = (struct Block_byref_2 *)(byref+1);
                
                // byref_destroy，调用 _Block_object_dispose 函数，根据 __block 修饰的不同类型来处理，
                // 比如如果是对象类型的话则 _Block_release_object(object) 执行 release 操作。
                (*byref2->byref_destroy)(byref);
            }
            
            // 释放内存空间。
            free(byref);
        }
    }
}
```
### `_Block_tryRetain`
&emsp;`block` 的 `retain` 操作，可类比 `OC` 对象的引用计数处理。`block` 的引用计数保存在 `Block_layout->flags` 的 `[1, 15]` 位中。
```c++
bool _Block_tryRetain(const void *arg) {
    struct Block_layout *aBlock = (struct Block_layout *)arg;
    return latching_incr_int_not_deallocating(&aBlock->flags);
}
```
### `_Block_isDeallocating`
&emsp;判断 `block` 是否正在进行释放。标识位在 `Block_layout->flags` 的第 `0` 位。
```c++
bool _Block_isDeallocating(const void *arg) {
    struct Block_layout *aBlock = (struct Block_layout *)arg;
    return (aBlock->flags & BLOCK_DEALLOCATING) != 0;
}
```
### `_Block_object_assign`
&emsp;当栈区 `block` 复制到堆区时，以及栈区 `__block` 变量复制到堆区时，`block` 截获的外部变量和 `__block` 修饰的不同类型的变量的操作。
1. 如果是对象类型，则执行 `retain`。
2. 如果是 `block` 则执行 `_Block_copy`。
...

```c++
// When Blocks or Block_byrefs hold objects then their copy routine helpers use this entry point to do the assignment.
// 当 Blocks 或者 Block_byrefs 持有对象时，copy routine helpers 使用此入口点进行分配。
// 当个 Block 捕获了变量，Block 复制时，其捕获的这些变量也需要复制。
/// _Block_object_assign
/// @param destArg 执行 Block_copy() 后的 block 中的对象、block、或者 BYREF 指针的指针 （堆上 block 中的）
/// @param object copy 之前的变量指针 （栈上 block 中）
/// @param flags flags
void _Block_object_assign(void *destArg, const void *object, const int flags) {
    const void **dest = (const void **)destArg;
    
    switch (os_assumes(flags & BLOCK_ALL_COPY_DISPOSE_FLAGS)) {
      case BLOCK_FIELD_IS_OBJECT:
        /*
        id object = ...;
        [^{ object; } copy];
        */
        // 当block捕获的变量为 OC 对象时执行此步，
        // ARC 中引用计数由 强指针 来确定，
        // 此时 _Block_retain_object 指向 retain 函数，在编译时已由编译器改变指向。
        // 增加 object 引用计数
        _Block_retain_object(object);
        *dest = object;
        break;

      case BLOCK_FIELD_IS_BLOCK:
        /*
        void (^object)(void) = ...;
        [^{ object; } copy];
        */
        // 当 block 捕获的变量为另外一个 block 时执行此步，copy 一个新的 block 并赋值给 *dest。
        *dest = _Block_copy(object);
        break;
    
      case BLOCK_FIELD_IS_BYREF | BLOCK_FIELD_IS_WEAK:
      case BLOCK_FIELD_IS_BYREF:
        /*
         // copy the onstack __block container to the heap
         // Note this __weak is old GC-weak/MRC-unretained.
         // ARC-style __weak is handled by the copy helper directly.
         __block ... x;
         __weak __block ... x;
         [^{ x; } copy];
         */
         // 仅用 __block 修饰的变量和 __block 和 __weak 同时修饰的变量，执行同样的操作。
        // 当 block 捕获的变量为 __block 修饰的变量时会执行此步，执行 _Block_byref_copy 操作。
        *dest = _Block_byref_copy(object);
        break;
        
      case BLOCK_BYREF_CALLER | BLOCK_FIELD_IS_OBJECT:
      case BLOCK_BYREF_CALLER | BLOCK_FIELD_IS_BLOCK:
        /*
         // copy the actual field held in the __block container
         // Note this is MRC unretained __block only. 
         // ARC retained __block is handled by the copy helper directly.
         __block id object;
         __block void (^object)(void);
         [^{ object; } copy];
         */
        // 如果管理的是 __block 修饰的对象或者 block 的内存会执行此步，直接进行指针赋值。
        *dest = object;
        break;

      case BLOCK_BYREF_CALLER | BLOCK_FIELD_IS_OBJECT | BLOCK_FIELD_IS_WEAK:
      case BLOCK_BYREF_CALLER | BLOCK_FIELD_IS_BLOCK  | BLOCK_FIELD_IS_WEAK:
        /*
         // copy the actual field held in the __block container
         // Note this __weak is old GC-weak/MRC-unretained.
         // ARC-style __weak is handled by the copy helper directly.
         __weak __block id object;
         __weak __block void (^object)(void);
         [^{ object; } copy];
         */
        // 同时被 __weak 和 __block 修饰的对象或者 block 执行此步，也是直接进行指针赋值。
        *dest = object;
        break;

      default:
        break;
    }
}
```
### `_Block_object_dispose`
&emsp;`block` 释放或者 `__block` 变量释放时。
```c++
// 当 Block 释放时，其捕获了变量，此时也需要释放这些被 block 捕获的外部变量
// When Blocks or Block_byrefs hold objects their destroy helper routines call this
// entry point to help dispose of the contents
void _Block_object_dispose(const void *object, const int flags) {
    switch (os_assumes(flags & BLOCK_ALL_COPY_DISPOSE_FLAGS)) {
      case BLOCK_FIELD_IS_BYREF | BLOCK_FIELD_IS_WEAK:
      case BLOCK_FIELD_IS_BYREF:
        // get rid of the __block data structure held in a Block
        // 如果需要管理的变量为 byref，则执行该步。 __block 变量。
        _Block_byref_release(object);
        break;
      case BLOCK_FIELD_IS_BLOCK:
        // 如果是 block 则调用 _Block_release 释放 block，上面有讲。
        _Block_release(object);
        break;
      case BLOCK_FIELD_IS_OBJECT:
        // 如果是 OC 对象就进行 release，默认没有做操作，由 ARC 管理。
        _Block_release_object(object);
        break;
        
      // 如果是其他就不做处理，__block 修饰的变量只有一个强指针引用
      case BLOCK_BYREF_CALLER | BLOCK_FIELD_IS_OBJECT:
      case BLOCK_BYREF_CALLER | BLOCK_FIELD_IS_BLOCK:
      case BLOCK_BYREF_CALLER | BLOCK_FIELD_IS_OBJECT | BLOCK_FIELD_IS_WEAK:
      case BLOCK_BYREF_CALLER | BLOCK_FIELD_IS_BLOCK  | BLOCK_FIELD_IS_WEAK:
        break;
      default:
        break;
    }
}
```
&emsp;至此 `libclosure-74` 中的内容基本就分析完毕了，`signature` 和 `layout` 部分的内容，几乎用不到就不再进行分析了。

**延伸:**

## `MRC` 下 `__block` 可以防止循环引用
1. `MRC` 下栈区 `block` 不持有 `person`，堆区 `block` 是会持有 `person`。
2. `ARC` 下栈区 `block` 和 堆区 `block` 都会持有 `person`。
```c++
@property (nonatomic, copy) BLOCK blk;

LGPerson *person = [[LGPerson alloc] init];
NSLog(@"%@", person);

NSLog(@"%@", ^{ NSLog(@"%@", person); });

self.blk = ^{ NSLog(@"%@", person); };
NSLog(@"%@", self.blk);
NSLog(@"person 引用计数：%lu", (unsigned long)[person customRetainCount]);
// MRC 下打印结果:
<LGPerson: 0x60000243f950>
<__NSStackBlock__: 0x7ffeee55e170>
<__NSMallocBlock__: 0x600002879680>
person 引用计数：2

// ARC 下打印结果:
<LGPerson: 0x600003be0ab0>
<__NSStackBlock__: 0x7ffeee45d170>
<__NSMallocBlock__: 0x60000379c270>
person 引用计数：4

// 把 blk 修改为 @property (nonatomic, weak) BLOCK blk;
// MRC 下打印结果:
<LGPerson: 0x600000140430>
<__NSStackBlock__: 0x7ffeedc9a170>
<__NSStackBlock__: 0x7ffeedc9a148>
person 引用计数：1

// ARC 下打印结果:
<LGPerson: 0x600001edaf20>
<__NSStackBlock__: 0x7ffee0c27170>
<__NSStackBlock__: 0x7ffee0c27148>
person 引用计数：3
🍀🍀🍀 <LGPerson: 0x600001edaf20> LGPerson dealloc
```
3. `MRC` 下栈区 `block` 不持有 `__block person`，堆区 `block` 也不持有 `__block person`。
4. `ARC` 下栈区 `block` 不持有 `__block person`，堆区 `block` 会持有 `__block person`。
```c++
@property (nonatomic, copy) BLOCK blk;

LGPerson __block *person = [[LGPerson alloc] init];
NSLog(@"%@", person);

NSLog(@"%@", ^{ NSLog(@"%@", person); });

self.blk = ^{ NSLog(@"%@", person); };
NSLog(@"%@", self.blk);
NSLog(@"person 引用计数：%lu", (unsigned long)[person customRetainCount]);
// MRC 下打印结果:
<LGPerson: 0x6000015a23a0>
<__NSStackBlock__: 0x7ffee602a138>
<__NSMallocBlock__: 0x6000019e8420>
person 引用计数：1

// ARC 下打印结果:
<LGPerson: 0x600000971470>
<__NSStackBlock__: 0x7ffee6176138>
<__NSMallocBlock__: 0x6000005027f0>
person 引用计数：1

// 把 blk 修改为 @property (nonatomic, weak) BLOCK blk;
// MRC 下打印结果:
<LGPerson: 0x6000017adc90>
<__NSStackBlock__: 0x7ffee454d138>
<__NSStackBlock__: 0x7ffee454d110>
person 引用计数：1

// ARC 下打印结果:
<LGPerson: 0x600002e21920>
<__NSStackBlock__: 0x7ffeebe23138>
<__NSStackBlock__: 0x7ffeebe23110>
person 引用计数：1
🍀🍀🍀 <LGPerson: 0x600002e21920> LGPerson dealloc
```
## `dispatch_async` 的 `block` 中是否该使用 `_weak self`

> `blcok` 中截获 `self`，一般会延长 `self` 的生命周期（至少到 `block` 释放后，才会释放 `self`）。如果 `self` 同时持有 `block`，则会导致循环引用。在日常 `block`  开发中我们的重点都放在了预防循环引用上，而循环引用之外的延长 `self` 的生命周期是很容易忽略的一个点。

### 延长 `self` 的生命周期
🌰 一：
```objective-c
dispatch_async(globalQueue_DEFAULT, ^{
    // do something
    
    // 下面在主队列里面要执行的 block 捕获了 self，self 的生命周期被延长，
    // 直到 block 被释放后才会释放被其 retain 的 self。
    
    // 假如这里做了一个超长等待时间的网络请求，请求响应里面使用了 self（self.model = xxxxxx），
    // 而由于网络请求过长，用户点击了返回按钮 VC 执行了 pop，
    // 但是由此处的 block 持有 VC，所以 VC 并不能得到释放。 
    
    dispatch_async(dispatch_get_main_queue(), ^{
        self.view.backgroundColor = [UIColor redColor];
    });
});
```

🌰 二：
```objective-c
// 下面在并行队列里面要执行的 block 没有 retain self
__weak typeof(self) _self = self;
dispatch_async(globalQueue_DEFAULT, ^{
    // 保证在下面的执行过程中 self 不会被释放，执行结束后 self 会执行一次 release。
    
    // 在 ARC 下，这里看似前面的 __wek 和这里的 __strong 相互抵消了，
    // 这里 __strong 的 self，在出了下面的右边花括号时，会执行一次 release 操作。 
    // 且只有此 block 执行的时候 _self 有值那么此处的 __strong self 才会有值，
    // 否则下面的 if 判断就直接 return 了。
    
    __strong typeof(_self) self = _self;
    if (!self) return;
    
    // do something
    // ...
    
    dispatch_async(dispatch_get_main_queue(), ^{
        // 此时如果能进来，表示此时 self 是存在的
        self.view.backgroundColor = [UIColor redColor];
    });
});
```
&emsp;当在 `dispatch_async` 的异步线程的 `block` 中捕获到 `self` 时，`self` 会被 `retained`，当 `block` 执行完毕后 `block` 释放销毁，同时才会释放它所 `retain` 的 `self`。这意味着：当 `block` 执行完毕后，`self` 如果没有别的强引用时它的生命周期才会结束。
上面例一中的第二 个 `block` 是在主队列中，它保证了 `self` 一直存活着当这个 `block` 被执行的时候。而此时在程序中存在潜在危险的就是：**延长了 `self` 的生命周期。**

如果你明确的不希望延长 `UIViewController` 对象的生命周期，而是当 `block` 被执行的时候去检查 `UIViewController` 对象到底是否存在，那么应该使用 `_weak typedef(self) _self = self` 防止 `self` 被 `block`  `reatain` 。同时需要注意的是 `block` 最后都会被执行，不管 `UIViewController` 是否存活。

### 在并行队列的异步操作 `block` 内部，释放 `retain` 的 `UI` 对象
```objective-c
dispatch_async(globalQueue_DEFAULT, ^{
    // self 假如在此处捕获的 self 是一个 UI 对象，且此 block 是该 UI 对象的最后一个持有者，一些操作使该 UI 对象被释放，由于此时在非主线程，且 此时 UI 对象的 dealloc 里面有一些 UI 操作，由于 UI 操作必须在主线程进行，但是此时是在非主线程，所以会导致 crash (怎么才能模拟出这种场景呢😖)
};
```

## 参考链接
**参考链接:🔗**
+ [libclosure-74](https://opensource.apple.com/source/libclosure/libclosure-74/)
+ [深入理解Block之Block的类型](https://www.jianshu.com/p/0855b68d1c1d)
+ [深入研究Block捕获外部变量和__block实现原理](https://www.jianshu.com/p/ee9756f3d5f6)
+ [【iOS】Block Hook概念+BlockHook第三方库分析（基本原理已完结，补充libffi方法解释）](https://blog.csdn.net/qq_32792839/article/details/99842250)
+ [最详细的block底层](https://www.cnblogs.com/MrTao/archive/2004/01/13/6824967.html)
+ [dispatch_async的block中是否该使用_weak self](https://www.jianshu.com/p/c374b7727d79)
+ [dispatch_async的block里面需要__weak self 吗？ #41](https://github.com/ibireme/YYKit/issues/41)
+ [线程安全类的设计](https://objccn.io/issue-2-4/)

