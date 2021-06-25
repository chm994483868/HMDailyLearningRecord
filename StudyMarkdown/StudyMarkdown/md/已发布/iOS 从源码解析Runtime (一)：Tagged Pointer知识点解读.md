# iOS 从源码解析Runtime (一)：Tagged Pointer 知识点解读

> &emsp;本来第一篇是 《iOS 从源码解析Runtime (二)：聚焦 objc_object、objc_class、isa》，但是当分析到 struct objc_object 的第一个函数 Class ISA() 的第一行实现代码时又看到了 ASSERT(!isTaggedPointer())，而且前面分析 weak 工作原理的时候也无数次看到 Tagged Pointer，觉的还是有必要再深入学习一下 Tagged Pointer。

## Tagged Pointer 由来
> &emsp;Tagged Pointer 是苹果为了在 64 位架构的处理器下节省内存占用和提高运行效率而提出的概念。它的本质是把一些占用内存较小的对象的数据直接放在指针的内存空间内，然后把这个指针直接作为对象使用，直接省去了为对象在堆区开辟空间的过程。

> &emsp;这里引出了一个疑问，“对象的内存都是位于堆区吗？” 是的。下面是我自己的推测：默认这里说的对象都是 NSObject 的子类，当深入看 + (id)alloc 函数时，可看到最后面开辟空间都是使用的 malloc（calloc 函数内部是调用 malloc 后再调用 bzero 置 0）函数，而 malloc 是 C 的运行库函数，向它申请的内存都是 C 运行库管理，采用堆的内存管理方式。该函数实际上会向操作系统申请内存，然后分配给请求者，同时其内部维护有它申请的内存的分配情况，以便管理其拥有的内存。

&emsp;2013 年 9 月，苹果首次在 `iOS` 平台推出了搭载 `64` 位架构处理器的 `iPhone`（`iPhone 5s`），为了节省内存和提高运行效率，提出了 `Tagged Pointer` 概念。下面我们逐步分析 `Tagged Pointer` 的优点以及结合源码分析它的实现。在 `objc-internal.h` 定义了 `OBJC_HAVE_TAGGED_POINTERS` 宏，表示在仅在 `__LP64__` 环境中支持 `Tagged Pointer`。
```c++
// Tagged pointer objects.
#if __LP64__
#define OBJC_HAVE_TAGGED_POINTERS 1
#endif
```
&emsp;指针变量的长度与地址总线有关。从 `32` 位系统架构切换到 `64` 位系统架构后，指针变量的长度也会由 `32` 位增加到 `64` 位。如果不考虑其它因素，`64` 位指针可表示的地址长度可达到 `2^64` 字节即 `2^34 TB`，以目前的设备的内存来看，使用 `8` 个字节存储一个地址数据，其实有很多位都是空余的，而 `Tagged Pointer` 正是为了把这些空余的空间利用起来。（例如，在 `iPhone` 真机下，在堆区创建一个 `NSObject` 对象，打印的它的地址，看到只占用了 `36` 位，剩下 `28` 位都是零。） 

## Tagged Pointer 内存占用
&emsp;明确一点，`NSInteger/NSUInteger` 是来自基本类型 `long/int`，`NSNumber`、`NSString`、`NSDate` 等都是继承自 `NSObject` 的子类。
```c++
#if __LP64__ || 0 || NS_BUILD_32_LIKE_64

// 在 64 位环境中，NSInteger 和 NSUInteger 占 8 个字节
typedef long NSInteger;
typedef unsigned long NSUInteger;
#else

// 在 32 位环境中，NSInteger 和 NSUInteger 占 4 个字节
typedef int NSInteger;
typedef unsigned int NSUInteger;
#endif
```
```objective-c
// NSNumber 继承自 NSObject，
// NSValue 继承自 NSObject，NSNumber 继承自 NSValue。
@interface NSNumber : NSValue
...
@end

@interface NSValue : NSObject <NSCopying, NSSecureCoding>
...
@end

// NSString 继承自 NSObject
@interface NSString : NSObject <NSCopying, NSMutableCopying, NSSecureCoding>
...
@end

// NSDate 继承自 NSObject
@interface NSDate : NSObject <NSCopying, NSSecureCoding>
...
@end
```
&emsp;在 `objc-runtime-new.h`，`CF` 要求所有对象至少为 `16` 个字节。（对象内部成员变量多为 8 字节对齐，如果最后对齐后对象内存小于 16 字节，则扩展为 16 字节。）
```c++
size_t instanceSize(size_t extraBytes) const {
    if (fastpath(cache.hasFastInstanceSize(extraBytes))) {
        return cache.fastInstanceSize(extraBytes);
    }

    size_t size = alignedInstanceSize() + extraBytes;
    
    // CF requires all objects be at least 16 bytes.
    if (size < 16) size = 16;
    
    return size;
}
```
&emsp;如果没有 `Tagged Pointer`，在 `32` 位环境中存储一个 `NSInteger` 类型的 `NSNumber` 实例对象的时候，需要系统在堆区为其分配 `8` 个字节（（理想状态）对象的 `isa` 指针 `4` 字节 + 存储的值 `4` 字节）空间，而到了 `64` 位环境，就会变成 `16` 个字节（（理想状态）对象的 `isa` 指针 `8` 字节 + 存储的值 `8` 字节），然后再加上指针变量在栈区的空间（`32` 位占 `4` 字节/ `64` 位占 `8` 字节），而如果此时 `NSNumber` 对象中仅存储了一个较小的数字，从 `32` 位切到  `64` 位环境即使在逻辑上没有任何改变的情况下，`NSNumber` 实例对象的内存占用也会直接翻一倍。
（在 `64` 位 `iOS` 真机环境下，`NSNumber` 实例对象中存放 `NSIntegerMax` 时，使用 `malloc_size` 函数，返回 `32`，即系统会为其开辟 `32` 字节的空间，一个 `NSObject` 实例对象系统会为其开辟 `16` 字节的空间。）

+ 在 `64` 位环境下，非 `Tagged Pointer` 时，`NSNumber` 实例对象在堆区占用 `16` 字节（ `NSObject` 对象是 `16` 字节，`NSNumber` 对象实际占用 `32` 字节）+ 指针变量在栈区占用 `8` 字节空间，一共 `24` 字节空间。
+ 在 `64` 位环境下，使用 `Tagged Pointer` 时，`NSNumber` 对象在堆区占用 `0` 字节 + 指针变量在栈区占用 `8` 字节空间，一共 `8` 字节空间。

&emsp;**`Tagged Pointer` 减少了至少一半的内存占用。**

示例代码:
```c++
NSObject *objc = [[NSObject alloc] init];
NSNumber *number = [[[NSNumber alloc] initWithInt:1] copy];
// NSNumber *number = [[NSNumber alloc] initWithLong:NSIntegerMax];

NSLog(@"objc pointer: %zu malloc: %zu CLASS: %@ ADDRESS: %p", sizeof(objc), malloc_size(CFBridgingRetain(objc)), object_getClass(objc), objc);
NSLog(@"number pointer: %zu malloc: %zu CLASS: %@ ADDRESS: %p", sizeof(number), malloc_size(CFBridgingRetain(number)), object_getClass(number), number);

// 控制台打印:
objc pointer: 8 malloc: 16 CLASS: NSObject ADDRESS: 0x282f2c6e0
number pointer: 8 malloc: 0 CLASS: __NSCFNumber ADDRESS: 0xddb739a2fdf961f7 // 看这个地址值大概是在栈区
number pointer: 8 malloc: 32 CLASS: __NSCFNumber ADDRESS: 0x282d23da0 // 看这个地址值大概是在堆区
```

## 如何判断指针变量是 Tagged Pointer
### isTaggedPointer
&emsp;定义于 `objc-object.h` 的 `isTaggedPointer` 函数，用来判断一个指针变量是否是 `Tagged Pointer`。
```c++
inline bool 
objc_object::isTaggedPointer() 
{
    return _objc_isTaggedPointer(this);
}
```

### _objc_isTaggedPointer
&emsp;`_objc_isTaggedPointer` 是定义于 `objc-internal.h` 中的一个返回 `bool` 类型的静态内联函数。
> &emsp;Return true if ptr is a tagged pointer object. Does not check the validity of ptr's class.
>
> &emsp;如果 ptr 是一个 Tagged Pointer 返回 true。不检查 ptr 的 class 的有效性，这里只针对指针内存空间最高位或最低位是 1 还是 0。

```c++
static inline bool 
_objc_isTaggedPointer(const void * _Nullable ptr)
{
    // 直接把指针值强制转化为 unsigned long 然后和 _OBJC_TAG_MASK 做与操作的结果是否还等于 _OBJC_TAG_MASK
    return ((uintptr_t)ptr & _OBJC_TAG_MASK) == _OBJC_TAG_MASK;
}
```

### SUPPORT_TAGGED_POINTERS
&emsp;定义在 `objc-config.h`的 `SUPPORT_TAGGED_POINTERS` 表示在 `Objective-C 2.0` 和 `64` 位系统中可用 `Tagged Pointer`。
```c++
// Define SUPPORT_TAGGED_POINTERS=1 to enable tagged pointer objects Be sure to edit tagged pointer SPI in objc-internal.h as well.
#if !(__OBJC2__  &&  __LP64__)
#   define SUPPORT_TAGGED_POINTERS 0
#else
#   define SUPPORT_TAGGED_POINTERS 1
#endif
```

### OBJC_MSB_TAGGED_POINTERS
&emsp;`OBJC_MSB_TAGGED_POINTERS` 表示不同平台下字符串是低位优先排序（LSD）还是高位优先排序（MSD）。具体细节可参考:[《字符串低位优先排序（LSD）和高位优先排序（MSD）原理及 C++ 实现》](https://blog.csdn.net/weixin_41427400/article/details/79851043)
```c++
#if (TARGET_OS_OSX || TARGET_OS_IOSMAC) && __x86_64__
    // 64-bit Mac - tag bit is LSB
    // 在 64 位 Mac 下采用字符串低位优先排序（LSD）
#   define OBJC_MSB_TAGGED_POINTERS 0
#else
    // Everything else - tag bit is MSB
    // 其他情况下，都是采用字符串高位优先排序 (MSB)
#   define OBJC_MSB_TAGGED_POINTERS 1
#endif
```

### _OBJC_TAG_MASK
&emsp;`_OBJC_TAG_MASK` 表示在字符串高位优先排序的平台下指针变量的第 64 位标记该指针为 `Tagged Pointer`，在 字符串低位优先排序的平台下指针变量的第 1 位标记该指针为 `Tagged Pointer`。

&emsp;在 `iOS` 真机上判断是否是 `Tagged Pointer` 看指针的第 64 比特位是否是 1，在 `x86_64` 架构的 Mac 下看指针的第 1 个比特位是否是 1。（即在 iOS 中判断最高位，在 mac 中判断最低位）
```c++
#if OBJC_MSB_TAGGED_POINTERS
#   define _OBJC_TAG_MASK (1UL<<63)
...
#else
#   define _OBJC_TAG_MASK 1UL
...
#endif
```
示例代码:
```c++
// 在 iPhone 上运行
// Tagged Pointer
NSNumber *number1 = @1;
NSLog(@"number1 %p %@ %zu", number1, [number1 class], malloc_size(CFBridgingRetain(number1)));

NSNumber *number2 = @2;
NSLog(@"number2 %p %@ %zu", number2, [number2 class], malloc_size(CFBridgingRetain(number2)));

NSString *a = [[@"a" mutableCopy] copy];
NSLog(@"a %p %@ %zu", a, [a class], malloc_size(CFBridgingRetain(a)));

NSString *ab = [[@"ab" mutableCopy] copy];
NSLog(@"ab %p %@ %zu", ab, [ab class], malloc_size(CFBridgingRetain(ab)));

NSString *b = [NSString stringWithFormat:@"b"];
NSLog(@"b %p %@ %zu", b, [b class], malloc_size(CFBridgingRetain(b)));

NSString *c = [NSString stringWithFormat:@"c"];
NSLog(@"c %p %@ %zu", c, [c class], malloc_size(CFBridgingRetain(c)));

// 非 Tagged Pointer
NSNumber *number3 = [[NSNumber alloc] initWithInteger:NSIntegerMax];
NSLog(@"number3 %p %@ %zu", number3, [number3 class], malloc_size(CFBridgingRetain(number3)));

NSString *abcd__ = [NSString stringWithFormat:@"abcdefghijklmnopqrstuvwxyz"];
NSLog(@"abcd__ %p %@ %zu", abcd__, [abcd__ class], malloc_size(CFBridgingRetain(abcd__)));

// 控制台打印
number1 0xd3bc9b2fde3f08b4 __NSCFNumber 0 // 0xd... => 0b1101...
number2 0xd3bc9b2fde3f0884 __NSCFNumber 0 // 0xd... => 0b1101...
a 0xc3bc9b2fde3f0eb7 NSTaggedPointerString 0 // 0xc... => 0b1100...
ab 0xc3bc9b2fde392eb4 NSTaggedPointerString 0 // 0xc... => 0b1100...
b 0xc3bc9b2fde3f0e87 NSTaggedPointerString 0 // 0xc... => 0b1100...
c 0xc3bc9b2fde3f0e97 NSTaggedPointerString 0 // 0xc... => 0b1100...

number3 0x282bcc540 __NSCFNumber 32 // 没有占满 64 位，最高位都是 0
abcd__ 0x2805e3150 __NSCFString 48 // 没有占满 64 位，最高位都是 0
```
&emsp;分析打印结果，可看到所有 `Tagged Pointer` 的 `64` 位数据使用几乎都是满的，最高位都是 `1`，`malloc_size` 返回的都是 `0`，对比最后非 `Tagged Pointer` 系统没有为对象开辟空间。正常的 `Objective-C` 实例对象的第一个成员变量都是指向类对象内存地址的 `isa` 指针，通过打断点，可看到所有 `Tagged Pointer` 的 `isa` 都是 `0x0`，且当 `Tagged Pointer` 是 `NSNumber` 类型时，`class` 函数的打印依然是 `__NSCFNumber`，苹果并没有设计一个单独的 `Class` 来表示 `Tagged Pointer`，`NSString` 则打印的是 `NSTaggedPointerString`，那这里引出了另外一个问题，`Tagged Pointer` 又是怎么获取所属的类呢？

## 为何可通过设定最高位或最低位来标识 Tagged Pointer
&emsp;这是因为在分配内存的时候，都是按 `2` 的整数倍来分配的，这样分配出来的正常内存地址末位不可能为 `1`，通过将最低标识为 `1` ，就可以和其他正常指针做出区分。

&emsp;那么为什么最高位为 `1` ，也可以标识呢 ？（目前 iOS 设备的内存都是固定的，如 iPhone、iPad、iWatch 都是固定的，不像是 mac 产品我们可以自己加装内存条）这是因为 `64` 位操作系统，设备一般没有那么大的内存，所以内存地址一般只有 `48` 个左右有效位（`64` 位 `iOS` 堆区地址只使用了 `36` 位有效位），也就是说高位的 `16` 位左右都为 `0`，所以可以通过最高位标识为 `1` 来表示 `Tagged Pointer`。那么既然 `1` 位就可以标识 `Tagged Pointer` 了，其他的信息是干嘛的呢？我们可以想象的，首先要有一些 `bit` 位来表示这个指针对应的类型，不然拿到一个 `Tagged Pointer` 的时候我们不知道类型，就无法解析成对应的值。

## 如何从 Tagged Pointer 获取所属的类
&emsp;正常的 `Objective-C` 对象是通过 `isa` 和掩码 `ISA_MASK` 进行 `&` 运算得到类对象的内存地址的，那么 `Tagged Pointer` 又是怎样获取类对象的内存地址的呢？

&emsp;接着上面 `OBJC_HAVE_TAGGED_POINTERS` 宏定义继续往下看的话，看到枚举 `objc_tag_index_t`，表示可能成为 `Tagged Pointer` 的类有哪些。

### objc_tag_index_t
```c++
// Tagged pointer layout and usage is subject to change on different OS versions.
// Tagged pointer 的 layout 和用法可能会因不同的 OS 版本而异。

// Tag indexes 0..<7 have a 60-bit payload.
// 0..<7 的类型有 60 位的负载内容。
// Tag index 7 is reserved.
// 7 是保留位。
// Tag indexes 8..<264 have a 52-bit payload.
// 8..<264 的类型有 52 位负载内容。（其实是 8 到 19，19 到 264 之间的数字并没有在枚举值中列出来）
// Tag index 264 is reserved.
// 264 是保留位。
#if __has_feature(objc_fixed_enum)  ||  __cplusplus >= 201103L
enum objc_tag_index_t : uint16_t
#else
typedef uint16_t objc_tag_index_t;
enum
#endif
{
    // 60-bit payloads
    OBJC_TAG_NSAtom            = 0, 
    OBJC_TAG_1                 = 1, 
    OBJC_TAG_NSString          = 2, // NSString
    OBJC_TAG_NSNumber          = 3, // NSNumber
    OBJC_TAG_NSIndexPath       = 4, // NSIndexPath
    OBJC_TAG_NSManagedObjectID = 5, 
    OBJC_TAG_NSDate            = 6, // NSDate

    // 60-bit reserved
    // 保留位
    OBJC_TAG_RESERVED_7        = 7, 

    // 52-bit payloads
    OBJC_TAG_Photos_1          = 8,
    OBJC_TAG_Photos_2          = 9,
    OBJC_TAG_Photos_3          = 10,
    OBJC_TAG_Photos_4          = 11,
    OBJC_TAG_XPC_1             = 12,
    OBJC_TAG_XPC_2             = 13,
    OBJC_TAG_XPC_3             = 14,
    OBJC_TAG_XPC_4             = 15,
    OBJC_TAG_NSColor           = 16,
    OBJC_TAG_UIColor           = 17,
    OBJC_TAG_CGColor           = 18,
    OBJC_TAG_NSIndexSet        = 19,
    
    // 前 60 位负载内容
    OBJC_TAG_First60BitPayload = 0,
    // 后 60 位负载内容
    OBJC_TAG_Last60BitPayload  = 6, 
    // 前 52 位负载内容
    OBJC_TAG_First52BitPayload = 8, 
    // 后 52 位负载内容
    OBJC_TAG_Last52BitPayload  = 263, 
    
    // 保留位
    OBJC_TAG_RESERVED_264      = 264
};
#if __has_feature(objc_fixed_enum)  &&  !defined(__cplusplus)
typedef enum objc_tag_index_t objc_tag_index_t;
#endif
```

### _objc_taggedPointersEnabled
```c++
// Returns true if tagged pointers are enabled.
// The other functions below must not be called if tagged pointers are disabled.
// 如果启用了 Tagged Pointer，则返回 true。如果禁用了 Tagged Pointer，则不得调用以下其他函数。
static inline bool 
_objc_taggedPointersEnabled(void)
{
    // 外联值 objc_debug_taggedpointer_mask 在 SUPPORT_TAGGED_POINTERS 下， 
    // uintptr_t objc_debug_taggedpointer_mask = _OBJC_TAG_MASK;
    
    extern uintptr_t objc_debug_taggedpointer_mask;
    return (objc_debug_taggedpointer_mask != 0);
}
```
&emsp;在 `objc-runtime-new.mm` 有一段 `Tagged pointer objects` 的注释如下:
```c++
/*
* Tagged pointer objects.
*
* Tagged pointer objects store the class and the object value in the object pointer; 
* the "pointer" does not actually point to anything.
*
* Tagged pointer 指针对象将 class 和对象数据存储在对象指针中，指针实际上不指向任何东西。
* Tagged pointer objects currently use this representation:
* Tagged pointer 当前使用此表示形式:
*
* (LSB)(字符串低位优先排序，64 位的 mac 下)
*  1 bit   set if tagged, clear if ordinary object pointer // 设置是否标记，如果普通对象指针则清除
*  3 bits  tag index // 标记类型
* 60 bits  payload // 负载数据容量，（存储对象数据）
*
* (MSB)(64 位 iPhone 下)
* The tag index defines the object's class. 
* The payload format is defined by the object's class.
* tag index 表示对象所属的 class。负载格式由对象的 class 定义。
*
* If the tag index is 0b111, the tagged pointer object uses an "extended" representation, 
* allowing more classes but with smaller payloads:
*
* 如果 tag index 是 0b111(7), tagged pointer 对象使用 “扩展” 表示形式,
* 允许更多类，但有效载荷更小: 
* (LSB)(字符串低位优先排序，64 位的 mac 下)
*  1 bit   set if tagged, clear if ordinary object pointer // 设置是否标记，如果普通对象指针则清除
*  3 bits  0b111 
*  8 bits  extended tag index // 扩展的 tag index
* 52 bits  payload // 负载数据容量，此时只有 52 位
* (MSB)
*
* Some architectures reverse the MSB and LSB in these representations.
* 在这些表示中，某些体系结构反转了 MSB 和 LSB。
*
* This representation is subject to change. Representation-agnostic SPI is:
* objc-internal.h for class implementers.
* objc-gdb.h for debuggers.
*/
```
### _objc_decodeTaggedPointer
```c++
static inline uintptr_t
_objc_decodeTaggedPointer(const void * _Nullable ptr)
{
    return (uintptr_t)ptr ^ objc_debug_taggedpointer_obfuscator;
}
```
&emsp;解码 `Tagged Pointer`，就是与混淆器 `objc_debug_taggedpointer_obfuscator` 进行异或操作。

### _objc_getTaggedPointerTag
```c++
// Extract the tag value from the given tagged pointer object.
// Assumes ptr is a valid tagged pointer object.
// Does not check the validity of ptr's tag.
// 从给定的标记指针对象中提取标记值。
// 假定 ptr 是有效的带标记的指针对象。
// 不检查 ptr 标签的有效性。
static inline objc_tag_index_t 
_objc_getTaggedPointerTag(const void * _Nullable ptr);

static inline objc_tag_index_t 
_objc_getTaggedPointerTag(const void * _Nullable ptr) 
{
    // ASSERT(_objc_isTaggedPointer(ptr));
    uintptr_t value = _objc_decodeTaggedPointer(ptr);
    
    uintptr_t basicTag = (value >> _OBJC_TAG_INDEX_SHIFT) & _OBJC_TAG_INDEX_MASK;
    uintptr_t extTag =   (value >> _OBJC_TAG_EXT_INDEX_SHIFT) & _OBJC_TAG_EXT_INDEX_MASK;
    
    if (basicTag == _OBJC_TAG_INDEX_MASK) {
        return (objc_tag_index_t)(extTag + OBJC_TAG_First52BitPayload);
    } else {
        return (objc_tag_index_t)basicTag;
    }
}
```
&emsp;都是移位以及与操作。

### classSlotForBasicTagIndex
&emsp;在 `objc-runtime-new.mm` 定义，根据 `objc_tag_index_t` 返回 `Class` 指针。
```c++
// Returns a pointer to the class's storage in the tagged class arrays.
// Assumes the tag is a valid basic tag.

// 从存储 tagged class 的数组中返回一个指向 class 的指针。
// 假设 tag 是有效的 tag。

static Class *
classSlotForBasicTagIndex(objc_tag_index_t tag)
{
    uintptr_t tagObfuscator = ((objc_debug_taggedpointer_obfuscator
                                >> _OBJC_TAG_INDEX_SHIFT)
                               & _OBJC_TAG_INDEX_MASK);
    uintptr_t obfuscatedTag = tag ^ tagObfuscator;
    
    // Array index in objc_tag_classes includes the tagged bit itself
    // objc_tag_classes 中的数组索引包括标记位本身
    
#if SUPPORT_MSB_TAGGED_POINTERS
    return &objc_tag_classes[0x8 | obfuscatedTag];
#else
    return &objc_tag_classes[(obfuscatedTag << 1) | 1];
#endif
}
```

### classSlotForTagIndex
```c++
// Returns a pointer to the class's storage in the tagged class arrays, 
// or nil if the tag is out of range.
// 从存储 tagged class 的数组中返回一个指向 class 的指针。
// 如果 tag 在区间之外返回 nil。
static Class *  
classSlotForTagIndex(objc_tag_index_t tag)
{
    if (tag >= OBJC_TAG_First60BitPayload && tag <= OBJC_TAG_Last60BitPayload) {
        return classSlotForBasicTagIndex(tag);
    }

    if (tag >= OBJC_TAG_First52BitPayload && tag <= OBJC_TAG_Last52BitPayload) {
        int index = tag - OBJC_TAG_First52BitPayload;
        uintptr_t tagObfuscator = ((objc_debug_taggedpointer_obfuscator
                                    >> _OBJC_TAG_EXT_INDEX_SHIFT)
                                   & _OBJC_TAG_EXT_INDEX_MASK);
        return &objc_tag_ext_classes[index ^ tagObfuscator];
    }
    
    // 返回 nil
    return nil;
}
```

### objc_tag_classes
```c++
extern "C" { 
    extern Class objc_debug_taggedpointer_classes[_OBJC_TAG_SLOT_COUNT];
    ...
}
#define objc_tag_classes objc_debug_taggedpointer_classes
```
&emsp;全局搜索 `objc_tag_classes` 只能看到是一个外联 `Class` 数组。

### objc_debug_taggedpointer_obfuscator 和 initializeTaggedPointerObfuscator 函数
```c++
// 在 Private Header/objc-gdb.h 中的定义
// tagged pointers 通过与 objc_debug_taggedpointer_obfuscator 进行异或来混淆
// tagged pointers are obfuscated by XORing with a random value
// decoded_obj = (obj ^ obfuscator)
OBJC_EXPORT uintptr_t objc_debug_taggedpointer_obfuscator
    OBJC_AVAILABLE(10.14, 12.0, 12.0, 5.0, 3.0);
/*
* initializeTaggedPointerObfuscator
* Initialize objc_debug_taggedpointer_obfuscator with randomness.
* 用随机值初始化 objc_debug_taggedpointer_obfuscator。
*
* The tagged pointer obfuscator is intended to make it more difficult for an attacker to construct a particular object as a tagged pointer,
* tagged pointer 混淆器旨在使攻击者更难将一个特定对象构造为 tagged pointer,
* in the presence of a buffer overflow or other write control over some memory.
* 在缓冲区溢出或对某些内存进行其他写控制的情况下。
* The obfuscator is XORed with the tagged pointers when setting or retrieving payload values.
* 设置或检索有效载荷值时，混淆器会与 tagged pointers 进行异或。
* They are filled with randomness on first use.
* 首次使用时充满随机性。
*/
static void
initializeTaggedPointerObfuscator(void)
{
    // objc_debug_taggedpointer_obfuscator 是一个 unsigned long 类型的全局变量
    
    // 环境变量，禁用 tagged pointers 混淆 
    // OPTION( DisableTaggedPointerObfuscation, OBJC_DISABLE_TAG_OBFUSCATION, "disable obfuscation of tagged pointers")
    
    if (sdkIsOlderThan(10_14, 12_0, 12_0, 5_0, 3_0) ||
        // Set the obfuscator to zero for apps linked against older SDKs,
        // 对于与旧版SDK链接的应用，将 混淆器 设置为零，
        // in case they're relying on the tagged pointer representation.
        // 以防他们依赖 tagged pointer 表示。
        DisableTaggedPointerObfuscation) {
        objc_debug_taggedpointer_obfuscator = 0;
    } else {
        // Pull random data into the variable, then shift away all non-payload bits.
        // 将随机数据放入变量中，然后移走所有非有效位。
        arc4random_buf(&objc_debug_taggedpointer_obfuscator,
                       sizeof(objc_debug_taggedpointer_obfuscator));
                       
        // 然后和 ~_OBJC_TAG_MASK 作一次与操作 
        objc_debug_taggedpointer_obfuscator &= ~_OBJC_TAG_MASK;
    }
}
```
&emsp;主要看 `classSlotForBasicTagIndex` 函数，`objc_debug_taggedpointer_obfuscator` 是系统动态运行时创建的盐，每次运行都不一样，然后其他的操作就是根据不同的平台宏定义的值进行移位和进行位操作。

&emsp;验证示例:
```c++
// 引入 #import "objc-internal.h"
NSString *str1 = [NSString stringWithFormat:@"a"];
NSNumber *num1 = [NSNumber numberWithInteger:1];

NSLog(@"str1 class: %@", _objc_getClassForTag(_objc_getTaggedPointerTag((__bridge void *)str1)));
NSLog(@"num1 class: %@", _objc_getClassForTag(_objc_getTaggedPointerTag((__bridge void *)num1)));

// 打印结果:
str1 class: NSTaggedPointerString
num1 class: __NSCFNumber
```

## 获取 Tagged Pointer 的值
### _objc_getTaggedPointerValue 和 _objc_getTaggedPointerSignedValue
```c++
// Extract the payload from the given tagged pointer object.
// Assumes ptr is a valid tagged pointer object.
// The payload value is zero-extended.

// 从给定的 tagged pointer 对象中提取有效负载。
// 假定 ptr 是有效的 tagged pointer 对象。
// 有效负载值是零扩展的。

static inline uintptr_t
_objc_getTaggedPointerValue(const void * _Nullable ptr);

static inline uintptr_t
_objc_getTaggedPointerValue(const void * _Nullable ptr) 
{
    // ASSERT(_objc_isTaggedPointer(ptr));
    uintptr_t value = _objc_decodeTaggedPointer(ptr);
    uintptr_t basicTag = (value >> _OBJC_TAG_INDEX_SHIFT) & _OBJC_TAG_INDEX_MASK;
    if (basicTag == _OBJC_TAG_INDEX_MASK) {
        return (value << _OBJC_TAG_EXT_PAYLOAD_LSHIFT) >> _OBJC_TAG_EXT_PAYLOAD_RSHIFT;
    } else {
        return (value << _OBJC_TAG_PAYLOAD_LSHIFT) >> _OBJC_TAG_PAYLOAD_RSHIFT;
    }
}

static inline intptr_t
_objc_getTaggedPointerSignedValue(const void * _Nullable ptr) 
{
    // ASSERT(_objc_isTaggedPointer(ptr));
    uintptr_t value = _objc_decodeTaggedPointer(ptr);
    uintptr_t basicTag = (value >> _OBJC_TAG_INDEX_SHIFT) & _OBJC_TAG_INDEX_MASK;
    if (basicTag == _OBJC_TAG_INDEX_MASK) {
        return ((intptr_t)value << _OBJC_TAG_EXT_PAYLOAD_LSHIFT) >> _OBJC_TAG_EXT_PAYLOAD_RSHIFT;
    } else {
        return ((intptr_t)value << _OBJC_TAG_PAYLOAD_LSHIFT) >> _OBJC_TAG_PAYLOAD_RSHIFT;
    }
}
```
&emsp;函数实现都很简单，首先 `Tagged Pointer` 解码，与 `objc_debug_taggedpointer_obfuscator` 进行异或操作，然后根据不同平台的宏定义进行移位操作。

&emsp;示例代码:
```c++
// 引入 #import "objc-internal.h"
NSString *str1 = [NSString stringWithFormat:@"a"];
NSString *str2 = [NSString stringWithFormat:@"ab"];
NSString *str3 = [NSString stringWithFormat:@"abc"];

uintptr_t value1 = _objc_getTaggedPointerValue((__bridge void *)str1);
uintptr_t value2 = _objc_getTaggedPointerValue((__bridge void *)str2);
uintptr_t value3 = _objc_getTaggedPointerValue((__bridge void *)str3);

NSLog(@"value1: %lx", value1);
NSLog(@"value2: %lx", value2);
NSLog(@"value3: %lx", value3);

// 打印：
value1: 611
value2: 62612
value3: 6362613

NSNumber *num1 = [NSNumber numberWithInteger:11];
NSNumber *num2 = [NSNumber numberWithInteger:12];
NSNumber *num3 = [NSNumber numberWithInteger:13];

uintptr_t value1 = _objc_getTaggedPointerValue((__bridge void *)num1);
uintptr_t value2 = _objc_getTaggedPointerValue((__bridge void *)num2);
uintptr_t value3 = _objc_getTaggedPointerValue((__bridge void *)num3);

NSLog(@"value1: %lx", value1);
NSLog(@"value2: %lx", value2);
NSLog(@"value3: %lx", value3);

// 打印：
value1: b3
value2: c3
value3: d3
```
&emsp;第一组 `NSString` 的打印中：`0x61`、`0x62`、`0x63` 分别对应 `a`、`b`、`c` 的 `ASCII` 码，最后一位数字表示字符串长度。第二组 `NSNumber` 的打印中：`0xb`、`0xc`、`0xd` 分别对应 `11`、`12`、`13` 的 `ASCII` 码，后面的 `3` 大概对应 `enum objc_tag_index_t` 的 `OBJC_TAG_NSNumber = 3` 表示类型是 `OBJC_TAG_NSNumber`。

## Tagged Pointer 可存储的最大值
&emsp;根据前面的分析以及当 `Tagged Pointer` 是 `NSNumber` 类型时，在 x86_64 Mac 平台下:
```c++
NSNumber *number = [[NSNumber alloc] initWithInteger: pow(2, 55) - 2];;
NSLog(@"number %p %@ %zu", number, [number class], malloc_size(CFBridgingRetain(number)));
// 打印:
number 0x10063e330 __NSCFNumber 32

NSNumber *number = [[NSNumber alloc] initWithInteger: pow(2, 55) - 3];;
NSLog(@"number %p %@ %zu", number, [number class], malloc_size(CFBridgingRetain(number)));
// 打印:
number 0x21a60cf72f053d4b __NSCFNumber 0
```
&emsp;在 `x86_64 Mac` 平台下存储 `NSString` 类型的 `Tagged Pointer`，一个指针 `8` 个字节，`64` 个比特位，第 `1` 个比特位用于标记是否是 `Tagged Pointer`，第 `2~4` 比特位用于标记 `Tagged Pointer` 的指针类型，解码后的最后 `4` 个比特位用于标记 `value` 的长度，那么用于存储 `value` 的比特位只有 `56` 个了，此时如果每个字符用 `ASCII` 编码的话 `8` 个字符应该就不是 `Tagged Pointer` 了，但其实 `NSTaggedPointerString` 采用不同的编码方式：

1. 如果长度介于 `0` 到 `7`，直接用八位编码存储字符串。
2. 如果长度是 `8` 或 `9`，用六位编码存储字符串，使用编码表 `eilotrm.apdnsIc ufkMShjTRxgC4013bDNvwyUL2O856P-B79AFKEWV_zGJ/HYX`。
3. 如果长度是 `10` 或 `11`，用五位编码存储字符串,使用编码表 `eilotrm.apdnsIc ufkMShjTRxgC4013`。

&emsp;`@"aaaaaaaa"` 解码后的 `TaggedPointer` 值为 `0x2082082082088`，扣除最后 `4` 个比特位代表的长度，则为 `0x20820820820`，只有 `6` 个字节，但是因为长度为 `8`，需要进行分组解码，`6` 个比特位为一组，分组后为 `0x0808080808080808`，刚好 `8` 个字节，长度符合了。采用编码表 `eilotrm.apdnsIc ufkMShjTRxgC4013bDNvwyUL2O856P-B79AFKEWV_zGJ/HYX`，下标为`8` 的刚好是 `a`。

&emsp;`@"aaaaaaaaaa"` 解码后的 `TaggedPointer` 值为 `1084210842108a`，扣除最后 `4` 个比特位代表的长度，则为 `1084210842108`，只有 `6.5` 字节，但是因为长度为 `10`，需要进行分组解码，`5` 个比特位为一组，分组后为 `0x08080808080808080808`，刚好 `10` 个字节，长度符合了。采用编码表 `eilotrm.apdnsIc ufkMShjTRxgC4013`，下标为 `8` 的刚好是 `a`。

&emsp;在编码表中并没有看到 `+` 字符，使用 `+` 字符做个测试，`7` 个 `+` 应为 `NSTaggedPointerString`，而 `8` 个 `+` 则为普通的 `__NSCFString` 对象。

&emsp;关于字符串的存储可以参考: [《译】采用Tagged Pointer的字符串》](http://www.cocoachina.com/articles/13449)。

## 参考链接
**参考链接:🔗**
+ [malloc和calloc的差别](https://www.cnblogs.com/mfrbuaa/p/5383026.html)
+ [Objective-C 的 Tagged Pointer 实现](https://www.jianshu.com/p/58d00e910b1e)
+ [译】采用Tagged Pointer的字符串](http://www.cocoachina.com/articles/13449)
+ [TaggedPointer](https://www.jianshu.com/p/01153d2b28eb?utm_campaign=maleskine&utm_content=note&utm_medium=seo_notes&utm_source=recommendation)
+ [深入理解 Tagged Pointer](https://www.infoq.cn/article/deep-understanding-of-tagged-pointer)

