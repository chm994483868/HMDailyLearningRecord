# iOS 从源码解析Runtime (二)：聚焦 isa、objc_object(ISA_BITFIELD相关内容篇)

> &emsp;`runtime` 是什么？在 `C` 语言中，将代码转换为可执行程序，一般要经历三个步骤，即编译、链接、运行。在链接的时候，方法的实现就已经确定好了。而在 `Objective-C` 中，却将一些在编译和链接过程中的工作，放到了运行阶段。也就是说，就算是一个编译好的 `.ipa` 包，在程序没有运行的时候，也不知道调用一个方法会发生什么。因此我们称 `Objective-C` 为一门动态语言。这样的设计使 `Objective-C` 变得灵活，甚至可以让我们在程序运行的时候，去动态修改一个方法的实现，而实现这一切的基础就是 `runtime`。简单来说，`runtime` 是一个库，这个库使我们可以在程序运行时创建对象、检查对象、修改类和对象的方法等等。
> 
> &emsp;2006 年，苹果发布了全新的 `Objective-C 2.0`，目前我们可以在苹果官网下载最新的 `objc4-781` 源码来阅读和调试。**在 `Obejective-C 2.0` 中类和对象等的实现进行了完全重写。** 虽然对当下而言 `Objective-C 1.0` 已经过时很久了，但是其相关代码对我们学习 `2.0` 还是具有极大的参考价值。（在源码内部 `1.0` 与 `2.0` 主要通过一些宏来作出区分）

&emsp;`struct objc_object` 和 `struct objc_class` 是 `iOS(OC)` 编写面向对象代码的基石。那今天我们就来详细的解析这两个结构体吧。

## objc_object
&emsp;`struct objc_object` 的定义位于 `objc-private.h`。在正式开始看 `objc_object` 之前我们先看下 `objc-private.h` 文件前面大概 50 行左右相关的代码。

+ **声明 `objc-privete.h` 头文件必须在其它头文件之前导入。**
&emsp;这样做是为了避免与其它地方定义的 `id` 和 `Class` 产生冲突。在 `Objective-C 1.0` 和 `2.0` 中，类和对象的结构体定义是不同的，在源码中我们能看到两处不同的 `objc_class` 和 `objc_object` 定义。

&emsp;我们可在 `runtime.h` 文件中看到 `/* Types */` 处的 `objc_class` 定义的那一部分代码被 `#if !OBJC_TYPES_DEFINED ... #endif` 所包裹，然后还有 `objc.h` 文件开头处的 `objc_object` 定义的代码也被 `#if !OBJC_TYPES_DEFINED ... #endif` 所包裹，表示两者只在 `Objective-C 1.0` 中使用。

&emsp;`Objective-C 2.0` 下正在使用的  `objc_object` 和 `objc_class` 定义分别位于 `objc-private.h` 和 `objc-runtime-new.h` 文件下。

&emsp;（2.0 下 `OBJC_TYPES_DEFINED` 的值为 1，且它的定义在 `objc-privete.h` 文件的开头处，因此声明 `objc-privete.h` 头文件必须在其它头文件之前导入。)

&emsp;（我们不通过源码看到的公开的 `runtime.h` 和 `objc.h` 头文件 里面的 `objc_object` 和 `objc_class` 都是过时的。）

```c++
/* 
 * Isolate ourselves from the definitions of id
 * and Class in the compiler and public headers.
 * 隔离 id 和 Class 在不同的编译器和头文件中的定义。
 */
#ifdef _OBJC_OBJC_H_
#error include objc-private.h before other headers
#endif

// 作为不同版本下定义类和对象的区分
#define OBJC_TYPES_DEFINED 1

// "为同名函数定不同的参数类型，大概是 id 类型参数与 void 替换"
#undef OBJC_OLD_DISPATCH_PROTOTYPES
#define OBJC_OLD_DISPATCH_PROTOTYPES 0
```

&emsp;在 `objc-api.h` 文件可看到如下代码，在 `__swift__` 为真的情况下，`OBJC_OLD_DISPATCH_PROTOTYPES` 会定为 1，其它情况下都是 0，在 `objc-privete.h` 文件中 `OBJC_OLD_DISPATCH_PROTOTYPES` 被定为 0。
```c++
/* OBJC_OLD_DISPATCH_PROTOTYPES == 0 
 * enforces the rule that the dispatch functions must
 * be cast to an appropriate function pointer type. 
 * 必须将 dispatch functions 强制转换为适当的函数指针类型。
 */
#if !defined(OBJC_OLD_DISPATCH_PROTOTYPES)
#   if __swift__
        // Existing Swift code expects IMP to be Comparable.
        // Variadic IMP is comparable via OpaquePointer; non-variadic IMP isn't.
        
        // 当前 Swift 代码期望 IMP 是遵守 Comparable，可以进行比较。
        // Variadic IMP 变量可以通过 OpaquePointer 进行比较，non-variadic IMP 不可以。
        
#       define OBJC_OLD_DISPATCH_PROTOTYPES 1
#   else
#       define OBJC_OLD_DISPATCH_PROTOTYPES 0
#   endif
#endif
```

示例代码一：
```c++
/// A pointer to the function of a method implementation. 
#if !OBJC_OLD_DISPATCH_PROTOTYPES
typedef void (*IMP)(void /* id, SEL, ... */ ); 
#else
typedef id _Nullable (*IMP)(id _Nonnull, SEL _Nonnull, ...); 
#endif
```
示例代码 二：
```c++
#if !OBJC_OLD_DISPATCH_PROTOTYPES
extern void _objc_msgForward_impcache(void);
#else
extern id _objc_msgForward_impcache(id, SEL, ...);
#endif
```
&emsp;看到同名函数，返回值 `void` 和 `id _Nullable (id 可空)` 做了替换，参数 `void` 和 `id, SEL` 做了替换，全局搜索可发现此行为只针对调度函数。

+ `runtime.h` 中的 `Objective-C 1.0` 下的 `objc_class` 定义。
```c++
/* Types */

#if !OBJC_TYPES_DEFINED

/// An opaque type that represents a method in a class definition.
typedef struct objc_method *Method;

/// An opaque type that represents an instance variable.
typedef struct objc_ivar *Ivar;

/// An opaque type that represents a category.
typedef struct objc_category *Category;

/// An opaque type that represents an Objective-C declared property.
typedef struct objc_property *objc_property_t;

struct objc_class {
    // 指向该类的元类（metaclass）指针，（元类永不可能为空，根元类的 isa 指向自己）
    Class _Nonnull isa  OBJC_ISA_AVAILABILITY;

#if !__OBJC2__
    // 指向父类的指针，（可空，根类的 super_class 指向 nil，根元类的 super_class 指向根类）
    Class _Nullable super_class                              OBJC2_UNAVAILABLE;
    // 类名
    const char * _Nonnull name                               OBJC2_UNAVAILABLE;
    // 类的版本信息
    long version                                             OBJC2_UNAVAILABLE;
    // 类信息，供运行时使用的一些标记位
    long info                                                OBJC2_UNAVAILABLE;
    // 该类的所有实例对象的大小
    long instance_size                                       OBJC2_UNAVAILABLE;
    // 指向该类成员变量列表的指针
    struct objc_ivar_list * _Nullable ivars                  OBJC2_UNAVAILABLE;
    // 指向该类方法（函数指针）列表指针的指针
    struct objc_method_list * _Nullable * _Nullable methodLists                    OBJC2_UNAVAILABLE;
    // 指向方法调用缓存的指针
    struct objc_cache * _Nonnull cache                       OBJC2_UNAVAILABLE;
    // 指向该类实现的协议列表的指针
    struct objc_protocol_list * _Nullable protocols          OBJC2_UNAVAILABLE;
#endif

} OBJC2_UNAVAILABLE;
/* Use `Class` instead of `struct objc_class *` */
```

+ `objc.h` 中的 `Objective-C 1.0` 下的 `objc_object` 定义
```c++
#if !OBJC_TYPES_DEFINED
/// An opaque type that represents an Objective-C class.
typedef struct objc_class *Class;

/// Represents an instance of a class.
struct objc_object {
    Class _Nonnull isa  OBJC_ISA_AVAILABILITY; // isa 还是原始的类指针
};

/// A pointer to an instance of a class.
typedef struct objc_object *id;
#endif
```
&emsp;在面向对象（`oop`）的编程语言中，每一个对象都是某个类的实例。在 `Objective-C` 中，所有对象的本质都是一个 `objc_object` 结构体，且每个实例对象的第一个成员变量都是 `Class isa`，指向该对象对应的类，每一个类描述了一系列它的实例对象的信息，包括对象占用内存大小、成员变量列表、该对象能执行的函数列表...等。

&emsp;**在一个类的实例对象的内存布局中，第一个成员变量是 `isa`，然后根据该对象所属类的继承体系依次对成员变量排序，排列顺序是: 根类的成员变量、父类的成员变量、最后才是自己的成员变量，且每个类定义中的成员变量（仅包含使用 `@property` 声明属性后由编译器生成的同名的 _成员变量）相互之间的顺序可能会与定义时的顺序不同，编译器会在内存对齐的原则下对类定义时的成员变量的顺序做出优化，保证内存占用最少。（还会涉及到 `.h` 中的成员变量和属性，`.m` 中 `extension` 中添加的成员变量和属性，它们之间的排序顺序）**

&emsp;验证代码:
```objective-c
// SubObject 类定义
@interface SubObject : BaseObject {
    NSArray *cus_array;
}

@property (nonatomic, assign) int cus_int;
@property (nonatomic, assign) double cus_dou;
@property (nonatomic, assign) int cus_int2;
@property (nonatomic, copy) NSString *cus_string;

@end

// 添加断点，控制台打印
(lldb) p *sub
(SubObject) $2 = {
  BaseObject = {
    NSObject = {
      isa = SubObject
    }
    baseString = nil
    _baseArray = nil
  }
  cus_array = nil
  _cus_int = 0
  _cus_int2 = 0
  _cus_dou = 0
  _cus_string = nil
}
```
&emsp;可看到 `NSObject` 的 `isa` 在最前面，然后是 `BaseObject` 的成员变量，最后才是 `SubObject` 的成员变量，然后注意 `_cus_int2` 跑到了 `_cus_dou` 前面，而在类定义时 `cus_dou` 属性是在 `cus_int2` 属性前面的。（由于内存对齐时不用再为 `double` 补位，这样至少减少了 4 个字节的内存浪费）

&emsp;这里大概又可以引出为什么不能动态的给类添加成员变量却可以添加方法？

&emsp;类的成员变量布局以及其实例对象大小在编译时就已确定，设想一下，如果 `Objective-C` 中允许给一个类动态添加成员变量，会带来一个问题：为基类动态增加成员变量会导致所有已创建出的子类实例都无法使用。
我们所说的 “类的实例”（对象），指的是一块内存区域，里面存储了 `isa` 指针和所有的成员变量。所以假如允许动态修改类已固定的成员变量的布局，那么那些已经创建出的对象就不符合类的定义了，就变成无效对象了。而方法的定义都是在类对象或元类对象中的，不管如何增删方法，都不会影响对象的内存布局，已经创建出的对象仍然可以正常使用。

+ `OBJC_ISA_AVAILABILITY` 
&emsp;在 `objc-api.h` 中的宏定义。表明在 `Objective-C 1.0` 中类型为 `Class` 的 `isa` 将在 `2.0` 中被弃用，在 `2.0` 中 `isa` 转变为 `union isa_t isa`，下面会详细分析。
```c++
/* OBJC_ISA_AVAILABILITY: `isa` will be deprecated or unavailable in the future
* isa 在未来将被弃用或者不可用。（这里可以理解为不能直接用 . 访问 isa 这个成员变量）
*/
#if !defined(OBJC_ISA_AVAILABILITY)
#   if __OBJC2__
#       define OBJC_ISA_AVAILABILITY  __attribute__((deprecated))
#   else
#       define OBJC_ISA_AVAILABILITY  /* still available */
#   endif
#endif
```
+ `OBJC2_UNAVAILABLE`
&emsp;在 `objc-api.h` 中的宏定义。表明在 `Objective-C 2.0` 中不可用，在 `macOS 10.5 iOS 2.0` 以及 `TVOS/WATCHOS/BRIDGEOS` 不可用。
```c++
/* OBJC2_UNAVAILABLE: unavailable in objc 2.0, deprecated in Leopard */
#if !defined(OBJC2_UNAVAILABLE)
#   if __OBJC2__
#       define OBJC2_UNAVAILABLE UNAVAILABLE_ATTRIBUTE
#   else
        /* plain C code also falls here, but this is close enough */
#       define OBJC2_UNAVAILABLE                                       \
            __OSX_DEPRECATED(10.5, 10.5, "not available in __OBJC2__") \
            __IOS_DEPRECATED(2.0, 2.0, "not available in __OBJC2__")   \
            __TVOS_UNAVAILABLE __WATCHOS_UNAVAILABLE __BRIDGEOS_UNAVAILABLE
#   endif
#endif
```
+ `SEL`
&emsp;在 `objc.h` 文件中定义的一个指向 `struct objc_selector` 的指针。
在 `objc4-781` 中查找不到 `objc_selector` 的具体定义，那这个 `objc_selector` 结构体具体是什么取决与使用 `GNU` 还是苹果的运行时， 在 `macOS` 中 `SEL` 其实被映射为一个 `C` 字符串，一个保存方法名字的字符串，它并不指向一个具体的方法实现（`IMP` 类型才是）。
`@selector(abc)` 返回的类型是 `SEL`，它作用是找到名字为 `abc` 的方法，对于所有的类，只要方法名是相同的，产生的 `@selector` 都是一样的。简而言之，你可以理解 `@selector()` 就是取指定名字的函数在类中的编号，它的行为基本可以等同 `C` 语言的中函数指针，只不过 `C` 语言中，可以把函数名直接赋给一个函数指针，而 `Objective-C` 的类不能直接应用函数指针，这样只能做一个 `@selector` 语法来取。
```c++
/// An opaque type that represents a method selector.
typedef struct objc_selector *SEL;
```
+ `IMP`
&emsp;在 `objc.h` 文件中定义的一个函数指针，指向方法调用时对应的函数实现。
```c++
// A pointer to the function of a method implementation. 
// 指向方法实现的指针。
#if !OBJC_OLD_DISPATCH_PROTOTYPES
typedef void (*IMP)(void /* id, SEL, ... */ ); 
#else
typedef id _Nullable (*IMP)(id _Nonnull, SEL _Nonnull, ...); 
#endif
```
&emsp;`OBJC_OLD_DISPATCH_PROTOTYPES` 默认为 0，在 `__swift__` 为真时是 1，会进行严格的参数匹配。`IMP` 指向的函数的前两个参数是默认参数 `id` 和 `SEL`，这对应了函数内部两个隐含参数 `self` 和 `_cmd`，这里的 `SEL` 和 `_cmd` 好理解，就是函数名。而 `id` 和 `self` ，对于实例方法来说， `self` 保存了当前对象的地址，对于类方法来说， `self` 保存了当前对应类对象的地址，后面的省略号即是参数列表。如下代码测试 `_cmd`: 
```objective-c
- (void)testCmd:(NSNumber *)num {
    NSLog(@"%ld", (long)num.integerValue);
    num = [NSNumber numberWithInteger:num.integerValue-1];
    if (num.integerValue > 0) {
        // 此处调用 _cmd
        [self performSelector:_cmd withObject:num];
    }
}
```
&emsp;控制台依次打印 5 4 3 2 1，即我们可以在方法内部用 `_cmd` 来调用方法自身。

+ `Method`
&emsp;位置在`objc.h` 文件。在 `Objective-C 1.0` 下，`Method` 被定义为一个指向 `struct objc_method` 的指针：
```c++
#if !OBJC_TYPES_DEFINED
// An opaque type that represents a method in a class definition.
// 表示类定义中的方法
typedef struct objc_method *Method;
#endif
```
&emsp;`struct objc_method` 在 `Objective-C 1.0` 定义如下:
```c++
struct objc_method {
    // 方法名称
    SEL _Nonnull method_name                                 OBJC2_UNAVAILABLE;
    
    // 方法类型是一个 char 指针，存储着方法的参数类型和返回值类型
    // 可通过 method_getTypeEncoding(method) 获取
    // 类似这样: @28@0:8i16@20、v16@0:8
    char * _Nullable method_types                            OBJC2_UNAVAILABLE;
    
    // 方法实现
    IMP _Nonnull method_imp                                  OBJC2_UNAVAILABLE;
}                                                            OBJC2_UNAVAILABLE;
```
&emsp;`Method` 的作用，相当于在 `SEL` 和 `IMP` 之间做了一个映射，当对一个对象发送消息时，通过 `SEL` 方法名找到其对应的函数实现 `IMP`，然后执行。

&emsp;注意:
&emsp;由于在类中查找方法时只是根据方法名（不包含参数和返回值）来查找的，因此在 `OC` 中同一块定义区域内，不能同时定义两个同名方法。（在分类中可以添加与类已有的方法同名的方法，会造成 "覆盖"。）如下代码则不能同时定义：（在 `C/C++` 中只要参数或者返回值类型不同就表示是不同的函数）
```c++
- (void)setWidth:(int)width;
- (void)setWidth:(double)width; // Duplicate declaration of method 'setWidth:'
```
+ `objc_method_list`
&emsp;`objc_method_list` 在 `Objective-C 1.0` 下的定义：
```c++
struct objc_method_list {
    struct objc_method_list * _Nullable obsolete             OBJC2_UNAVAILABLE;

    int method_count                                         OBJC2_UNAVAILABLE;
#ifdef __LP64__
    int space                                                OBJC2_UNAVAILABLE;
#endif
    /* variable length structure */
    struct objc_method method_list[1]                        OBJC2_UNAVAILABLE;
}                                                            OBJC2_UNAVAILABLE;
```
&emsp;`__LP64__` 表示当前平台（或者说运行环境）下 `long` 和 `poniter` 都是 64，表示当前操作系统是 `64` 位。在 `Mac` 终端直接执行 `cpp -dM /dev/null` 会打印出一大组当前设备的一些宏定义的值，我看到在我的机器下: `#define __LP64__ 1`、`#define __POINTER_WIDTH__ 64`、等等，列表很长，大家可以在终端执行一下试试。

&emsp;在上面 `Objective-C 1.0` 下 `objc_class` 定义中 `methodLists` 是一个 `struct objc_method_list` 类型的二级指针，其中每一个元素是一个数组，数组中的每一个元素是 `struct objc_method`。

+ `Ivar`
&emsp;在 `Objective-C 1.0` 下，`Ivar` 被定义为一个指向 `struct objc_ivar` 的指针：
```c++
#if !OBJC_TYPES_DEFINED
/// An opaque type that represents an instance variable.
typedef struct objc_ivar *Ivar;
#endif
```
+ `objc_ivar`
&emsp;`objc_ivar` 在 `Objective-C 1.0` 下的定义：
```c++
struct objc_ivar {
    char * _Nullable ivar_name                               OBJC2_UNAVAILABLE;
    char * _Nullable ivar_type                               OBJC2_UNAVAILABLE;
    // 这个 ivar_offset 很重要，等后面我们看 2.0 时再进行展开
    int ivar_offset                                          OBJC2_UNAVAILABLE;
#ifdef __LP64__
    int space                                                OBJC2_UNAVAILABLE;
#endif
}                                                            OBJC2_UNAVAILABLE;
```
+ `objc_ivar_list`
&emsp;`objc_ivar_list` 在 `Objective-C 1.0` 下的定义：
```c++
struct objc_ivar_list {
    int ivar_count                                           OBJC2_UNAVAILABLE;
#ifdef __LP64__
    int space                                                OBJC2_UNAVAILABLE;
#endif
    /* variable length structure */
    struct objc_ivar ivar_list[1]                            OBJC2_UNAVAILABLE;
}                                                            OBJC2_UNAVAILABLE;
```
&emsp;在上面 `Objective-C 1.0` 下 `objc_class` 定义中 `ivars` 是一个 `struct objc_ivar_list` 类型的指针。
`ivars` 中有一个数组，数组中每个元素是一个 `struct objc_ivar`。

+ `objc_property_t`
&emsp;在 `Objective-C 1.0` 下，`objc_property_t` 被定义为一个指向 `struct objc_property` 的指针：
```c++
#if !OBJC_TYPES_DEFINED
/// An opaque type that represents an Objective-C declared property.
typedef struct objc_property *objc_property_t;
#endif
```
&emsp;在 `objc4-781` 中未找到 `struct objc_property` 定义，倒是可以找一个与 `property` 相关的属性描述。

+ `objc_property_attribute_t`
```c++
/// Defines a property attribute
typedef struct {
    // 属性的名字
    const char * _Nonnull name;           /**< The name of the attribute */
    // 属性的值（通常是空的）
    const char * _Nonnull value;          /**< The value of the attribute (usually empty) */
} objc_property_attribute_t;
```
&emsp;`objc_property_attribute_t` 定义了属性的特性。

+ `Category`
&emsp;在 `Objective-C 1.0` 下，`Category` 被定义为一个指向 `struct objc_category` 的指针：
```c++
#if !OBJC_TYPES_DEFINED
/// An opaque type that represents a category.
typedef struct objc_category *Category;
#endif
```
+ `objc_category`
&emsp;`objc_category` 在 `Objective-C 1.0` 下的定义：
```c++
struct objc_category {
    char * _Nonnull category_name                            OBJC2_UNAVAILABLE;
    char * _Nonnull class_name                               OBJC2_UNAVAILABLE;
    struct objc_method_list * _Nullable instance_methods     OBJC2_UNAVAILABLE;
    struct objc_method_list * _Nullable class_methods        OBJC2_UNAVAILABLE;
    struct objc_protocol_list * _Nullable protocols          OBJC2_UNAVAILABLE;
}                                                            OBJC2_UNAVAILABLE;
```
&emsp;`objc_category` 包含分类中定义的实例方法和类方法，在程序启动时会由 `runtime` 动态追加到对应的类和元类中。
在 `objc_category` 中包含对象方法列表、类方法列表、协议列表。从这里我们也可以看出， `Category` 支持添加实例方法、类方法、协议，但不能保存成员变量。

注意:
&emsp;在 `Category` 中是可以添加属性的，但不会生成对应的成员变量、 `getter` 和 `setter` 。因此，调用 `Category` 中声明的属性时会报错。

&emsp;`Category` 在 `2.0` 和 `1.0` 中的差别还挺大的，后面我们会详细分析 `2.0` 下的 `Category`。

+ `Cache`
&emsp;在 `Objective-C 1.0` 下，`Cache` 被定义为一个指向 `struct objc_cache` 的指针：
```c++
typedef struct objc_cache *Cache                             OBJC2_UNAVAILABLE;
```
+ `objc_cache`
&emsp;`objc_cache` 在 `Objective-C 1.0` 下的定义：
```c++
#define CACHE_BUCKET_NAME(B)  ((B)->method_name)
#define CACHE_BUCKET_IMP(B)   ((B)->method_imp)
#define CACHE_BUCKET_VALID(B) (B)

// 哈希函数的宏定义
#ifndef __LP64__
#define CACHE_HASH(sel, mask) (((uintptr_t)(sel)>>2) & (mask))
#else
#define CACHE_HASH(sel, mask) (((unsigned int)((uintptr_t)(sel)>>3)) & (mask))
#endif

struct objc_cache {
    // 总容量 - 1
    unsigned int mask /* total = mask + 1 */                 OBJC2_UNAVAILABLE;
    // 当前的占用的总数
    unsigned int occupied                                    OBJC2_UNAVAILABLE;
    // struct objc_method 指针数组
    Method _Nullable buckets[1]                              OBJC2_UNAVAILABLE;
};
```
+ `Protocol`
```c++
#ifdef __OBJC__
@class Protocol;
#else
// Protocol 直接被定义为 objc_object ？
typedef struct objc_object Protocol;
#endif

@interface Protocol : NSObject
@end
```
+ `objc_protocol_list`
```c++
struct objc_protocol_list {
    struct objc_protocol_list * _Nullable next;
    // 长度
    long count;
    // Protocol 指针数组
    __unsafe_unretained Protocol * _Nullable list[1];
};
```
&emsp;**到这里由 `OBJC_TYPES_DEFINED` 宏定义涉及的 `Objective-C 1.0` 下的一些类型定义已经基本看了一遍，大致在脑中也有了一个脉络，下面继续看 `objc-private.h` 文件和接下的来 `Objective-C 2.0` 下类和对象的结构所做出的改变。**

+ `ASSERT(x)`
&emsp;在 `release` 模式下不会执行断言，同时也保证 `ASSERT(x)` 可正常编译。
```c++
// An assert that's disabled for release builds but still ensures the expression compiles.
#ifdef NDEBUG
#define ASSERT(x) (void)sizeof(!(x))
#else
#define ASSERT(x) assert(x)
#endif
```

&emsp;**以下正式进入 `Objective-C 2.0` 下类和对象的学习，下面所涉及的内容都是我们当下正在运用的内容!**

+ `Class`
`Class` 是指向 `struct objc_class` 的指针。
```c++
typedef struct objc_class *Class;
```
+ `id`
`id` 是指向 `struct objc_object` 的指针。
```c++
typedef struct objc_object *id;
```
+ `__OBJC__`
> `__OBJC__`
  This macro is defined, with value 1, when the Objective-C compiler is in use. You can use `__OBJC__` to test whether a header is compiled by a C compiler or a Objective-C compiler. [Common Predefined Macros](https://gcc.gnu.org/onlinedocs/gcc-3.3/cpp/Common-Predefined-Macros.html)

&emsp;`__OBJC__` 在 `Objective-C` 编译器中被预定义为 1，我们可以使用该宏来判断头文件是通过 `C` 编译器还是 `Objective-C` 编译器进行编译。

+ `__OBJC2__`
&emsp;定义在 `objc-config.h` 中：
```c++
// Define __OBJC2__ for the benefit of our asm files.
#ifndef __OBJC2__
#   if TARGET_OS_OSX  &&  !TARGET_OS_IOSMAC  &&  __i386__
        // 表示在 32 位 Mac 下依然使用旧 ABI
        // old ABI
#   else
#       define __OBJC2__ 1
#   endif
#endif
```
&emsp;下面接着看 `objc_object`。`objc_object` 仅有一个 `isa_t isa` 成员变量。
```c++
struct objc_object {
private:
    isa_t isa;
public:
    ...
};
```

## isa
```c++
#include "isa.h"
union isa_t {
    // 构造函数
    isa_t() { }
    // 初始化列表，初始 bits 的值
    isa_t(uintptr_t value) : bits(value) { }
    
    // cls 和 bits 共同使用一块 8 字节空间，
    // 当使用 bits 时，会使用位域，不同的位表示不同的内容 ISA_BITFIELD
    
    // 兼容以前的 isa，在 isa 中只存放一个指向 objc_class 的指针
    Class cls;
    // typedef unsigned long uintptr_t;
    uintptr_t bits;
    
    // ISA_BITFIELD 在 x86_64/arm64 下有不同的定义
#if defined(ISA_BITFIELD)
    struct {
        ISA_BITFIELD;  // defined in isa.h
    };
#endif
};
```
&emsp;`isa` 的类型是 `union isa_t`，它有两个成员变量 `Class cls` 和 `uintptr_t bits` 共用同一块 8 字节的内存空间，其中 `bits` 采用位域的机制来保存当前对象所属类之外的更多信息。`ISA_BITFIELD` 宏定义位于 `isa.h` 文件，一起来看下。

### 苹果设备架构梳理
&emsp;在学习 `isa.h` 之前我们首先对 `iPhone` 和 `Mac` 的架构做一下梳理。
+ `armv6/armv7/armv7s/arm64(armv8)/arm64e(armv8)` 是 `iPhone` 的 `ARM` 处理器的指令集。（`A` 系列 `CPU` 芯片）
+ `i386/x86_64` 是 `Mac` 的 `intel` 处理器的指令集。（同时也是我们在 `Xcode` 中使用的 `iPhone` 模拟器的指令集）

> `i386` 架构是 `intel` 通用微处理器 `32` 位处理器。
  `x86_64` 架构是 `x86` 架构的 `64` 位处理器。 
  `armv6/armv7/armv7s` 架构是 `32` 位处理器。
  `arm64(armv8)/arm64e(armv8)` 架构是 `64` 位处理器。（ `iPhone 5s` 开始全部转向 `64` 位（`__LP64__`））

### isa.h 文件
&emsp; `isa.h` - `C` 和 `assembly` 代码的 `isa` 字段的定义。

#### SUPPORT_PACKED_ISA
```c++
// Define SUPPORT_PACKED_ISA=1 on platforms that store the class in
the isa field as a maskable pointer with other data around it.
// SUPPORT_PACKED_ISA = 1 的平台将类信息与其他数据一起存储在 isa 的字段中，
// 把 isa 作为一个 maskable pointer。（并通过掩码的方式读取 isa 保存的不同的内容）

#if (!__LP64__  ||  TARGET_OS_WIN32  || (TARGET_OS_SIMULATOR && !TARGET_OS_IOSMAC))
#   define SUPPORT_PACKED_ISA 0
#else
#   define SUPPORT_PACKED_ISA 1
#endif
```
&emsp;表示平台是否支持在 `isa` 指针中插入 `Class` 指针之外的信息，可以理解为把类信息和其它的一些信息打包放在 `isa` 中。如果支持会将 `Class` 信息放入 `isa_t` 中定义的 `bits` 位域的 `shiftcls` 中，读取 `Class` 信息的话则是以掩码的方式。其它位中则放一些其它信息。如果不支持的话，那么 `union isa_t` 中的 `bits/cls` 两个成员变量共用的内存空间就只是保存一个 `Class` 指针。

&emsp;下列平台下不支持：

1. 32 位处理器。（它使用的是 `SUPPORT_INDEXED_ISA`）
2. `os` 是 `win32`。
3. 在模拟器中且 `!TARGET_OS_IOSMAC`。（ `TARGET_OS_IOSMAC` 不知道是什么平台）

#### SUPPORT_INDEXED_ISA
```c++
// Define SUPPORT_INDEXED_ISA = 1 on platforms that store the
// class in the isa field as an index into a class table.
// Note, keep this in sync with any .s files which also define it.
// Be sure to edit objc-abi.h as well.

// SUPPORT_INDEXED_ISA = 1 的平台将类在全局 class table 中的 index 和其他数据一起存储在 isa 的字段中，
// 把 isa 作为一个 maskable pointer。（并通过掩码的方式读取 isa 保存的不同的内容）

// 注意，与任何定义它的 .s 文件保持同步。确保同时编辑 objc-abi.h 文件。
#if __ARM_ARCH_7K__ >= 2  ||  (__arm64__ && !__LP64__)
#   define SUPPORT_INDEXED_ISA 1
#else
#   define SUPPORT_INDEXED_ISA 0
#endif
```
&emsp;和上面的 `SUPPORT_PACKED_IA` 基本完全相同，不同的是它保存的是 `Class` 在全局 `class table` 中的索引，位域名是 `indexcls`，它支持的平台仅限于 `armv7k` 或 `arm64_32`，目前的话会在 `watchOS` 下会使用。

#### SUPPORT_NONPOINTER_ISA
```c++
// Define SUPPORT_NONPOINTER_ISA = 1 on any platform that may
// store something in the isa field that is not a raw pointer.

// SUPPORT_NONPOINTER_ISA = 1 的平台表示在 isa 的字段中保存非原始指针的内容。
// (优化后的 isa （union isa_t 类型）)

#if !SUPPORT_INDEXED_ISA  &&  !SUPPORT_PACKED_ISA
#   define SUPPORT_NONPOINTER_ISA 0
#else
#   define SUPPORT_NONPOINTER_ISA 1
#endif
```
&emsp;标记是否支持优化的 `isa` 指针（`isa` 中除 `Class` 指针外，可以保存更多信息）。那如何判断是否支持优化的 `isa` 指针呢？

1. 首先只要支持 `SUPPORT_PACKED_ISA` 或 `SUPPORT_INDEXED_ISA` 任何一个的情况下都是支持 `SUPPORT_NONPOINTER_ISA` 的。
2. 已知在自 `iPhone 5s` `arm64` 架构以后的 `iPhone` 中 `SUPPORT_PACKED_ISA` 都为 1，`SUPPORT_INDEXED_ISA` 为 0，则其 `SUPPORT_NONPOINTER_ISA` 为 1。
3. 在 `Edit Scheme... -> Run -> Environment Variables` 中添加 `OBJC_DISABLE_NONPOINTER_ISA` 值为 `NO` (在 `objc-env.h` 文件我们可以看到 `OPTION( DisableNonpointerIsa, OBJC_DISABLE_NONPOINTER_ISA, "disable non-pointer isa fields")` ) 可关闭指针优化。

&emsp;那怎么判断一个 `isa` 是优化的 `isa`（也就是 `nonpointer`）呢？
也是极其简单的，在 `SUPPORT_PACKED_ISA` 或 `SUPPORT_INDEXED_ISA` 平台下，只需要判断 `isa` 的第一位就可以了，如果是 `1` 则表示是优化的 `isa` 否则就不是。

&emsp;那么看完以上 3 个宏定义，`isa.h` 文件中的宏定义我们就自然理解分为 3 大块了（由于我们的 `objc4-781` 是在 `Mac` 下运行的，那自然我们会更关注 `x84_64` 一些）。

#### ISA_BITFIELD
&emsp;`isa_t isa` 不同位段下保存的信息不同。

##### __arm64__ 下
```c++
#   define ISA_MASK        0x0000000ffffffff8ULL
#   define ISA_MAGIC_MASK  0x000003f000000001ULL
#   define ISA_MAGIC_VALUE 0x000001a000000001ULL
#   define ISA_BITFIELD                                                      \
      // 表示 isa 中只是存放的 Class cls 指针还是包含更多信息的 bits
      uintptr_t nonpointer        : 1;                                       \
      // 标记该对象是否有关联对象，如果没有的话对象能更快的销毁，
      // 如果有的话销毁前会调用 _object_remove_assocations 函数根据关联策略循环释放每个关联对象
      uintptr_t has_assoc         : 1;                                       \
      // 标记该对象所属类是否有自定义的 C++ 析构函数，如果没有的话对象能更快销毁，
      // 如果有的话对象销毁前会调用 object_cxxDestruct 函数去执行该类的析构函数
      uintptr_t has_cxx_dtor      : 1;                                       \
      // isa & ISA_MASK 得出该实例对象所属的的类的地址
      uintptr_t shiftcls          : 33; /*MACH_VM_MAX_ADDRESS 0x1000000000*/ \
      // 用于调试器判断当前对象是真的对象还是没有初始化的空间
      uintptr_t magic             : 6;                                       \
      // 标记该对象是否有弱引用，如果没有的话对象能更快销毁，
      // 如果有的话对象销毁前会调用 weak_clear_no_lock 函数把该对象的弱引用置为 nil，
      // 并调用 weak_entry_remove 把对象的 entry 从 weak_table 中移除
      uintptr_t weakly_referenced : 1;                                       \
      // 标记该对象是否正在执行销毁
      uintptr_t deallocating      : 1;                                       \
      // 标记 refcnts 中是否也有保存实例对象的引用计数，当 extra_rc 溢出时会把一部分引用计数保存到 refcnts 中去，
      uintptr_t has_sidetable_rc  : 1;                                       \
      // 保存该对象的引用计数 -1 的值（未溢出之前）
      uintptr_t extra_rc          : 19 // 最大保存 2^19 - 1，觉得这个值很大呀,  mac 下是 2^8 - 1 = 255
#   define RC_ONE   (1ULL<<45)
#   define RC_HALF  (1ULL<<18)
```

##### __x86_64__ 下
```c++
#   define ISA_MASK        0x00007ffffffffff8ULL
#   define ISA_MAGIC_MASK  0x001f800000000001ULL
#   define ISA_MAGIC_VALUE 0x001d800000000001ULL
#   define ISA_BITFIELD                                                        \
      uintptr_t nonpointer        : 1;                                         \
      uintptr_t has_assoc         : 1;                                         \
      uintptr_t has_cxx_dtor      : 1;                                         \
      uintptr_t shiftcls          : 44; /*MACH_VM_MAX_ADDRESS 0x7fffffe00000*/ \
      uintptr_t magic             : 6;                                         \
      uintptr_t weakly_referenced : 1;                                         \
      uintptr_t deallocating      : 1;                                         \
      uintptr_t has_sidetable_rc  : 1;                                         \
      uintptr_t extra_rc          : 8
#   define RC_ONE   (1ULL<<56)
#   define RC_HALF  (1ULL<<7)
```

##### armv7k or arm64_32 下
```c++
#   define ISA_INDEX_IS_NPI_BIT  0
#   define ISA_INDEX_IS_NPI_MASK 0x00000001
#   define ISA_INDEX_MASK        0x0001FFFC
#   define ISA_INDEX_SHIFT       2
#   define ISA_INDEX_BITS        15
#   define ISA_INDEX_COUNT       (1 << ISA_INDEX_BITS)
#   define ISA_INDEX_MAGIC_MASK  0x001E0001
#   define ISA_INDEX_MAGIC_VALUE 0x001C0001
#   define ISA_BITFIELD                         \
      uintptr_t nonpointer        : 1;          \
      uintptr_t has_assoc         : 1;          \
      uintptr_t indexcls          : 15;         \ // 在 class table 中的索引
      uintptr_t magic             : 4;          \
      uintptr_t has_cxx_dtor      : 1;          \
      uintptr_t weakly_referenced : 1;          \
      uintptr_t deallocating      : 1;          \
      uintptr_t has_sidetable_rc  : 1;          \
      uintptr_t extra_rc          : 7
#   define RC_ONE   (1ULL<<25)
#   define RC_HALF  (1ULL<<6)
```

## struct objc_object 中关于 ISA_BITFIELD 的函数
&emsp;`struct objc_object` 中定义的函数过多，由于篇幅限制，这里先解析一部分，剩下的会再开新篇介绍。

### Class ISA()
```c++
// ISA() assumes this is NOT a tagged pointer object
// 假定不是 tagged pointer 对象时调用该函数
inline Class 
objc_object::ISA() 
{
    // 如果是 tagged pointer 则直接执行断言
    ASSERT(!isTaggedPointer()); 
#if SUPPORT_INDEXED_ISA
    // 支持在 isa 中保存类的索引的情况下
    
    if (isa.nonpointer) {
        uintptr_t slot = isa.indexcls;
        
        // 根据索引返回 class table 中的 Class
        return classForIndex((unsigned)slot);
    }
    
    // 如果是非优化指针直接返回 isa 中的 bits，由于 isa 是 union 所以 (Class)isa.bits 和 (Class)isa.cls 的值是一样的。
    return (Class)isa.bits;
#else
    // 从 shiftcls 位域取得 Class 指针，这个是我们平时用到的最多的，我们日常使用的实例对象获取所属的类都是通过这种方式。
    return (Class)(isa.bits & ISA_MASK);
#endif
}
```

### Class rawISA()
```c++
// rawISA() assumes this is NOT a tagged pointer object or a non pointer ISA
// 返回原始 isa 中的类指针
inline Class
objc_object::rawISA()
{
    ASSERT(!isTaggedPointer() && !isa.nonpointer);
    // 同 return (Class)isa.cls;
    return (Class)isa.bits;
}
```

### Class getIsa()
```c++
// getIsa() allows this to be a tagged pointer object
// 允许 tagged pointer 对象调用的获取 isa 的函数
inline Class 
objc_object::getIsa() 
{
    // 如果是非 TaggedPointer 则直接调用 ISA()
    if (fastpath(!isTaggedPointer())) return ISA();

    // 从 Tagged Pointer 取得所属类，可参考上一篇深入学习 Tagged Pointer
    extern objc_class OBJC_CLASS_$___NSUnrecognizedTaggedPointer;
    uintptr_t slot, ptr = (uintptr_t)this;
    Class cls;

    slot = (ptr >> _OBJC_TAG_SLOT_SHIFT) & _OBJC_TAG_SLOT_MASK;
    
    // Tagged Pointer 从指针中提出 tag，然后从 objc_tag_classes 取出对应的类
    cls = objc_tag_classes[slot];
    if (slowpath(cls == (Class)&OBJC_CLASS_$___NSUnrecognizedTaggedPointer)) {
        slot = (ptr >> _OBJC_TAG_EXT_SLOT_SHIFT) & _OBJC_TAG_EXT_SLOT_MASK;
        cls = objc_tag_ext_classes[slot];
    }
    return cls;
}
```

### uintptr_t isaBits() const
```c++
inline uintptr_t
objc_object::isaBits() const
{   
    // 直接返回 union isa_t 的 bits
    return isa.bits;
}
```

### initIsa
&emsp;下面是一组针对不同类型的 objc_object（子类）来初始化其 isa 的方法，例如初始化实例对象的 isa、初始化类对象的 isa、初始化协议对象的 isa、和初始化其他类型对象的 isa。
```c++
// initIsa() should be used to init the isa of new objects only.
// If this object already has an isa, use changeIsa() for correctness.

// initIsa() 应该仅仅用于初始化一个新对象的 isa.
// 如果该对象已经有了 isa，应该使用 changeIsa() 来保持正确性。

// initInstanceIsa(): objects with no custom RR/AWZ
// initInstanceIsa(): 对象没有自定义的 RR/AWZ 函数

// initClassIsa(): class objects
// initClassIsa(): 初始化类对象的 isa

// initProtocolIsa(): protocol objects
// initProtocolIsa(): 协议对象

// initIsa(): other objects
// initIsa(): 其他对象

void initIsa(Class cls /*nonpointer=false*/);
void initClassIsa(Class cls /*nonpointer=maybe*/);
void initProtocolIsa(Class cls /*nonpointer=maybe*/);
void initInstanceIsa(Class cls, bool hasCxxDtor);

// initIsa
inline void 
objc_object::initIsa(Class cls)
{
    // nonpointer false 表示不使用非指针
    // hascxxDtor false 表示没有 C++ 析构函数
    initIsa(cls, false, false);
}

// initClassIsa
inline void 
objc_object::initClassIsa(Class cls)
{
    // OPTION( DisableNonpointerIsa, OBJC_DISABLE_NONPOINTER_ISA, "disable non-pointer isa fields")
    // 环境变量开启了禁用非指针 isa 字段
    
    // cache.getBit(FAST_CACHE_REQUIRES_RAW_ISA);
    // 从 cache 中获取是否需要原始指针的标记值
    
    if (DisableNonpointerIsa  ||  cls->instancesRequireRawIsa()) {
        // 此时 isa 还是原始的 class 指针
        initIsa(cls, false/*not nonpointer*/, false);
    } else {
        initIsa(cls, true/*nonpointer*/, false);
    }
}

// initProtocolIsa
inline void
objc_object::initProtocolIsa(Class cls)
{
    return initClassIsa(cls);
}

// initInstanceIsa
inline void 
objc_object::initInstanceIsa(Class cls, bool hasCxxDtor)
{
    // 不需要原始 isa
    ASSERT(!cls->instancesRequireRawIsa());
    
    // 是否包含 C++ 析构函数 调用的 objc_class 中的函数，保存在 cache 成员变量下 
    ASSERT(hasCxxDtor == cls->hasCxxDtor());

    initIsa(cls, true, hasCxxDtor);
}

// initIsa
inline void 
objc_object::initIsa(Class cls, bool nonpointer, bool hasCxxDtor) 
{
    // 如果是 Tagged Pointer 则执行断言
    ASSERT(!isTaggedPointer()); 
    
    if (!nonpointer) {
        // 如果不是非指针，则表明 isa 中仅存一个 Class 指针
        isa = isa_t((uintptr_t)cls);
    } else {
        // 两条断言
        // 没有环境变量没有禁用非指针
        // cls 不需要一个原始的 isa(原始 isa 是指仅存一个 Class 的 isa)
        ASSERT(!DisableNonpointerIsa);
        ASSERT(!cls->instancesRequireRawIsa());
        
        // 初始化一个 union isa_t 的实例
        isa_t newisa(0);

#if SUPPORT_INDEXED_ISA
        // class 保存在 class table 中忽略
        ASSERT(cls->classArrayIndex() > 0);
        
        // 初始化
        newisa.bits = ISA_INDEX_MAGIC_VALUE;
        
        // isa.magic is part of ISA_MAGIC_VALUE
        // isa.nonpointer is part of ISA_MAGIC_VALUE
        
        newisa.has_cxx_dtor = hasCxxDtor;
        // index 来自 class_data_bits_t 中
        newisa.indexcls = (uintptr_t)cls->classArrayIndex();
#else
        // __x86_64__: # define ISA_MAGIC_VALUE 0x001d800000000001ULL
        // __arm64__: # define ISA_MAGIC_VALUE 0x000001a000000001ULL
        
        // 初始化 bits 
        newisa.bits = ISA_MAGIC_VALUE;
        
        // magic 和 nonpointer 使用 ISA_MAGIC_VALUE 中的原始值
        // isa.magic is part of ISA_MAGIC_VALUE
        // isa.nonpointer is part of ISA_MAGIC_VALUE
        
        // 是否有 C++ 析构函数
        newisa.has_cxx_dtor = hasCxxDtor;
        
        // shiftcls 赋值为 cls 左移 3 位，为前面 nonpointer has_assoc has_cxx_dtor 三个字段撇开位置
        newisa.shiftcls = (uintptr_t)cls >> 3;
#endif
        // This write must be performed in a single store in some cases
        // 在某些情况下，此写操作必须在单个存储中执行
        
        // (for example when realizing a class because other
        // threads may simultaneously try to use the class).
        // (例如在实现一个类时，因为其他线程可能同时尝试使用该类). (指实现类对象时，不是指创建实例对象时)
        // fixme use atomics here to guarantee single-store and to guarantee memory order
        // w.r.t. the class index table...
        // but not too atomic because we don't want to hurt instantiation
        // 在这里使用原子来保证单存储并保证存储器顺序w.r.t.类索引表...但是不要太原子，因为我们不想伤害实例化
        // 把 newisa 赋值给 isa
        
        isa = newisa;
    }
}
```
&emsp;以上用于初始化类对象或者实例对象的 `isa`。实例对象的话大概率在 `callalloc` 函数中，为对象开辟完空间后，要对对象的 `isa` 进行初始化。

### changeIsa
&emsp;changeIsa 函数包含的内容较多，涉及到不同类型的 isa 类型的转换，例如当 newCls 参数的 isa 是类指针或者优化的 ias 时，它可能转化为 isa 是类指针或者是优化的 isa，然后优化的 isa 又包含是保存的类索引（indexcls）还是直接保存的类地址（shiftcls），而且包含引用计数的转移等内容。
```c++
inline Class 
objc_object::changeIsa(Class newCls)
{
    // This is almost always true but there are 
    // enough edge cases that we can't assert it.
    // 这几乎总是正确的，但是有足够的边缘情况我们无法断言。
    // assert(newCls->isFuture()  || 
    //        newCls->isInitializing()  ||  newCls->isInitialized());
    
    // 如果是 tagged pointer 则执行断言
    ASSERT(!isTaggedPointer()); 
    
    // 临时变量
    isa_t oldisa;
    isa_t newisa;

    bool sideTableLocked = false;
    bool transcribeToSideTable = false;

    do {
        transcribeToSideTable = false;
        
        // __c11_atomic_load((_Atomic(uintptr_t) *)src, __ATOMIC_RELAXED);
        // atomic_load(const volatile A * obj);
        // 以原子方式加载并返回指向的原子变量的当前值 obj。该操作是原子读取操作。
        // 以原子方式从 isa 中读取 bits 并赋给 oldisa
        oldisa = LoadExclusive(&isa.bits);
        
        // oldisa.bits 等于 0 或者 oldisa 是非指针
        // 并且 newCls 不是未实现的 future class 且 newCls 实例不需要原始 isa
        // 即新 isa 和 旧 isa 都是优化后的非指针
        if ((oldisa.bits == 0  ||  oldisa.nonpointer)  &&
            !newCls->isFuture()  &&  newCls->canAllocNonpointer())
        {
            // 0 -> nonpointer
            // nonpointer -> nonpointer
            // 包含两种情况
            // 1. 从 0 转换为 nonpointer 
            // 2. 从 nonpointer 转换为 nonpointer
            
#if SUPPORT_INDEXED_ISA
            // 忽略
            if (oldisa.bits == 0) newisa.bits = ISA_INDEX_MAGIC_VALUE;
            else newisa = oldisa;
            // isa.magic is part of ISA_MAGIC_VALUE
            // isa.nonpointer is part of ISA_MAGIC_VALUE
            newisa.has_cxx_dtor = newCls->hasCxxDtor();
            ASSERT(newCls->classArrayIndex() > 0);
            newisa.indexcls = (uintptr_t)newCls->classArrayIndex();
#else
            // 如果是 0，则用宏 ISA_MAGIC_VALUE 初始化
            // 否则把 isa.bits 赋给 newisa
            if (oldisa.bits == 0) newisa.bits = ISA_MAGIC_VALUE;
            else newisa = oldisa;
            
            // isa.magic is part of ISA_MAGIC_VALUE
            // isa.nonpointer is part of ISA_MAGIC_VALUE
            
            // 从 newCls 中取出 hasCxxDtor
            newisa.has_cxx_dtor = newCls->hasCxxDtor();
            // 从 newCls 中取出 shiftcls
            newisa.shiftcls = (uintptr_t)newCls >> 3;
#endif
        }
        else if (oldisa.nonpointer) {
            // nonpointer -> raw pointer
            // 包含一种情况
            // 1. 从 nonpointer 转换为 raw Pointer
            
            // Need to copy retain count et al to side table.
            // 需要将引用计数等内容复制到 SideTable.
            
            // Acquire side table lock before setting isa to
            // prevent races such as concurrent -release.
            // 在设置 isa 之前要从 SideTable 获取锁防止诸如并发 -release 之类的竞态。
            
            // 加锁
            if (!sideTableLocked) sidetable_lock();
            
            sideTableLocked = true;
            transcribeToSideTable = true;
            
            // 赋值 cls （isa_t 内部指保存一个 Class）
            newisa.cls = newCls;
        }
        else {
            // raw pointer -> raw pointer
            // 包含一种情况
            // 1. 从 raw Pointer 转换为 raw Pointer
            
            // 只需要一个 cls 赋值
            newisa.cls = newCls;
        }
    // __c11_atomic_compare_exchange_weak((_Atomic(uintptr_t) *)dst,
    // &oldvalue, value, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
    // 保证修改成功
    } while (!StoreExclusive(&isa.bits, oldisa.bits, newisa.bits));

    // 只针对: 从 nonpointer 转换为 raw Pointer 这种情况
    // 需要将引用计数等内容复制到 SideTable.
    if (transcribeToSideTable) {
        // Copy oldisa's retain count et al to side table.
        // 复制 oldisa 的引用计数等数据到 SideTable
        // oldisa.has_assoc: nothing to do
        // oldisa.has_cxx_dtor: nothing to do
        sidetable_moveExtraRC_nolock(oldisa.extra_rc, 
                                     oldisa.deallocating, 
                                     oldisa.weakly_referenced);
    }

    // 只针对: 从 nonpointer 转换为 raw Pointer 这种情况
    // 解锁
    if (sideTableLocked) sidetable_unlock();

    // 返回 Class 
    if (oldisa.nonpointer) {
#if SUPPORT_INDEXED_ISA
        return classForIndex(oldisa.indexcls);
#else
        return (Class)((uintptr_t)oldisa.shiftcls << 3);
#endif
    }
    else {
        return oldisa.cls;
    }
}
```
&emsp;修改 `isa`，包含多种不同的情况:

1. 从 `0` 转换为 `nonpointer`
2. 从 `nonpointer` 转换为 `nonpointer`
3. 从 `nonpointer` 转换为 `raw Pointer`
4. 从 `raw Pointer` 转换为 `raw Pointer`

### sidetable_moveExtraRC_nolock
```c++
// Move the entire retain count to the side table, as well as isDeallocating and weaklyReferenced.
// 将全部引用计数以及 isDeallocating 和 weakReferenced 移到 SideTable。
void 
objc_object::sidetable_moveExtraRC_nolock(size_t extra_rc, 
                                          bool isDeallocating, 
                                          bool weaklyReferenced)
{
    // 如果不是非指针，则执行断言，应该已经更改为原始指针了
    ASSERT(!isa.nonpointer);        // should already be changed to raw pointer
    
    // 首先取得对象所处的 SideTable
    SideTable& table = SideTables()[this];

    // 从 SideTable 的 refcnts 中取出对象的引用计数
    size_t& refcntStorage = table.refcnts[this];
    
    // 旧对象的引用计数
    size_t oldRefcnt = refcntStorage;
    
    // not deallocating - that was in the isa
    // 不是在 deallocating
    ASSERT((oldRefcnt & SIDE_TABLE_DEALLOCATING) == 0);  
    ASSERT((oldRefcnt & SIDE_TABLE_WEAKLY_REFERENCED) == 0);  

    // 由 nonpointer -> raw pointer
    uintptr_t carry;
    
    // 把原对象 refcnts 中的引用计数和 extra_rc 中的引用计数相加
    size_t refcnt = addc(oldRefcnt, extra_rc << SIDE_TABLE_RC_SHIFT, 0, &carry);
    
    // 标记溢出
    if (carry) refcnt = SIDE_TABLE_RC_PINNED;
    // 标记正在释放
    if (isDeallocating) refcnt |= SIDE_TABLE_DEALLOCATING;
    // 标记包含弱引用
    if (weaklyReferenced) refcnt |= SIDE_TABLE_WEAKLY_REFERENCED;

    refcntStorage = refcnt;
}
```
&emsp;至此，`struct objc_object` 中关于初始化和修改 `isa` 的函数就都看完了，看到针对 `union isa_t` 中的 `bits` 中不同的位域进行赋值。然后下面一组函数，仍然是对位域中不同的值进行读取或者修改，下面一起来看吧。

### hasNonpointerIsa
&emsp;读取 `union isa_t` 的 `bits` 中 `ISA_BITFIELD` 中的 `nonpointer`。
```c++
inline bool 
objc_object::hasNonpointerIsa()
{
    return isa.nonpointer;
}
```

### isTaggedPointer
&emsp;`isTaggedPointer`、`isBasicTaggedPointer`、`isExtTaggedPointer` 三个函数可参考上篇 `Tagged Pointer`。

### isClass
```c++
inline bool
objc_object::isClass()
{
    // 如果是 Tagged Pointer 直接返回 false
    if (isTaggedPointer()) return false;
    // 首先取出对象所属的 Class，然后调用 objc_class 的 isMetaClass 函数
    
    return ISA()->isMetaClass();
}
    
    // 判断是否是元类
    bool isMetaClass() {
        ASSERT(this);
        ASSERT(isRealized());
#if FAST_CACHE_META
        // 下篇学习 objc_class 时再详细分析
        return cache.getBit(FAST_CACHE_META);
#else
        return data()->flags & RW_META;
#endif
    }
```
&emsp;所以此函数是判断对象所属的类是否是元类。

&emsp;**下面是两个特别重要也且常用的与关联对象相关的函数：**

### hasAssociatedObjects
```c++
inline bool
objc_object::hasAssociatedObjects()
{
    // 如果对象是 Tagged Pointer 则返回 true，对这个 true 还挺迷惑的
    if (isTaggedPointer()) return true;
    
    // 如果是非指针则读取 has_assoc 位
    if (isa.nonpointer) return isa.has_assoc;
    
    // 如果 isa 是 objc_class 指针则默认返回 true
    return true;
}
```
&emsp;判断对象是否有关联对象，实现很简单，非 `Tagged Pointer` 对象的标识存储在 `union isa_t` 的 `bits` 的 `has_assoc` 位。

### setHasAssociatedObjects
```c++
inline void
objc_object::setHasAssociatedObjects()
{
    // 如果是 Tagged Pointer 直接返回
    if (isTaggedPointer()) return;

 retry:
    // 原子读取 isa.bits
    isa_t oldisa = LoadExclusive(&isa.bits);
    isa_t newisa = oldisa;
    if (!newisa.nonpointer  ||  newisa.has_assoc) {
        ClearExclusive(&isa.bits); // 对应 LoadExclusive，（可理解为加锁和解锁配对使用）
        
        // 如果 isa 是 objc_class 指针或者 has_assoc 为 true，则什么也不做。
        
        return;
    }
    
    // 标记为 true
    newisa.has_assoc = true;
    
    // 保证修改成功，否则 goto 到 retry
    if (!StoreExclusive(&isa.bits, oldisa.bits, newisa.bits)) goto retry;
}
```
&emsp;设置关联对象标志位，如果已经设置为 `true` 则什么都不做，否则设置为 `true`。

&emsp;如果一个对象曾经有关联对象，当所有关联对象都移除后 `has_assoc` 位置不设置为 `false`，当对象释放时会调用 `_object_remove_assocations` 函数，会不会导致性能浪费呢？ 

### ClearExclusive
&emsp;`LoadExclusive` 和 `ClearExclusive` 配对使用，可理解为加锁解锁配对。
```c++
#if __arm64__ && !__arm64e__

...
static ALWAYS_INLINE
void
ClearExclusive(uintptr_t *dst __unused)
{
    __builtin_arm_clrex();
}

#else
...
static ALWAYS_INLINE
void
ClearExclusive(uintptr_t *dst __unused)
{
}

#endif
```

&emsp;**下面是两个特别重要也且常用的与弱引用相关的函数：**

### isWeaklyReferenced
&emsp;判断对象是否存在弱引用。
```c++
inline bool
objc_object::isWeaklyReferenced()
{
    // 如果是 Tagged Pointer 执行断言
    ASSERT(!isTaggedPointer());
    
    // 如果是非指针则返回 weakly_referenced 标记位
    if (isa.nonpointer) return isa.weakly_referenced;
    
    // 其他情况调用 sidetable_isWeaklyReferenced
    //（当 isa 是 objc_class 指针时，对象的弱引用标识位在 SideTable 的 refcnts 中）
    else return sidetable_isWeaklyReferenced();
}
```
### sidetable_isWeaklyReferenced
&emsp;`isa` 是原始类指针的对象的是否有弱引用的标识在 `refcnts` 中。
```c++
bool 
objc_object::sidetable_isWeaklyReferenced()
{
    bool result = false;
    
    // 取得对象所处的 SideTable
    SideTable& table = SideTables()[this];
    // 加锁
    table.lock();
    
    RefcountMap::iterator it = table.refcnts.find(this);
    // 判断当前对象是否存在 SideTable 的 refcnts 中
    if (it != table.refcnts.end()) {
        // 如果存在 
        // it->second 是引用计数 与 SIDE_TABLE_WEAKLY_REFERENCED 进行与操作
        // 引用计数值的第 1 位是弱引用的标识位哦
        result = it->second & SIDE_TABLE_WEAKLY_REFERENCED;
    }
    
    // 解锁
    table.unlock();

    return result;
}
```
### hasCxxDtor
```c++
// object may have -.cxx_destruct implementation?
inline bool
objc_object::hasCxxDtor()
{
    // 如果是 Tagged Pointer 则执行断言
    ASSERT(!isTaggedPointer());
    
    // 如果是非指针则读取 has_cxx_dtor 位
    if (isa.nonpointer) return isa.has_cxx_dtor;
    
    // 其他情况则调用调用类的 hasCxxDtor 函数
    else return isa.cls->hasCxxDtor();
}

// 当 FAST_CACHE_HAS_CXX_DTOR 存在时表示 C++ 析构函数标识位直接保存在类的 cache 成员变量中。
#if FAST_CACHE_HAS_CXX_DTOR
    bool hasCxxDtor() {
        ASSERT(isRealized());
        return cache.getBit(FAST_CACHE_HAS_CXX_DTOR);
    }
    void setHasCxxDtor() {
        cache.setBit(FAST_CACHE_HAS_CXX_DTOR);
    }
#else
...
#endif
```
&emsp;下面开始是 `struct objc_object` 中与 `retain`/`release` 函数相关的一大组函数，由于涉及 `RefcountMap refcnts` 的内容，可能要开一篇分支来解读 `refcnts`，并且到这里篇幅已经过长了，那么下篇见吧。

## 参考链接
**参考链接:🔗**
+ [Objective-C 1.0 中类与对象的定义](https://kangzubin.com/objc1.0-class-object/)
+ [苹果架构分类](https://www.jianshu.com/p/63420dfb217c)
+ [Object-C 中的Selector 概念](https://www.cnblogs.com/geek6/p/4106199.html)
+ [C语言中文开发手册:atomic_load/atomic_compare_exchange_weak](https://www.php.cn/manual/view/34155.html)
+ [操作系统内存管理(思维导图详解)](https://blog.csdn.net/hguisu/article/details/5713164)
+ [内存管理](https://www.jianshu.com/p/8d742a44f0da)
