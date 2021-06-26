# iOS Category 底层实现原理(一)：概念与数据结构
> &emsp;提到 category 不免要和 extension 进行比较，那么本篇就由 extension 开始吧！⛽️⛽️

## extension 延展
&emsp;`extension` 和 `category` 不同，`extension` 可以声明方法、属性和成员变量，但一般是私有方法、私有属性和私有成员变量。

### extension 存在形式
&emsp;`category` 拥有 `.h` 和 `.m` 文件，`extension` 则不然，`extension` 只存在一个 `.h` 文件，或者只能 “寄生” 在 `.m` 中（“寄生” 在 `.m` 中是我们最常见的存在形式）。

+ "寄生" 形式 
&emsp;比如，在 `BaseViewController.m` 文件中，可能会直接写一个 `extension`:
```objective-c
@interface BaseViewController () {
// 此处可定义私有成员变量
// ...
}

// 此处可定义私有属性
// ...
// 此处可定义私有方法
// ...
@end
```

+ 定义 .h 文件形式
&emsp;可以单独创建一个 `extension` 文件，`command` + `N` -> `Objective-C File`，`File Type` 选择 `Extension`，`Class` 输入要创建 `extension` 的类名，`File` 输入 `extension` 的名字，点击 `next` 后就会生成一个名字是 `类名+xxx.h` 的 `.h` 文件。

&emsp;下面示例是我们以 `.h` 文件的形式使用 `extension`。`CusObject+extension.h` 文件：
```c++
#import <Foundation/Foundation.h>
#import "CusObject.h"

NS_ASSUME_NONNULL_BEGIN

@interface CusObject () {
    // 通过 extension 添加成员变量
    NSString *name;
}

// 通过 extension 添加属性和方法
@property (nonatomic, copy) NSString *nameTwo;
- (void)testMethod_Extension;

@end

NS_ASSUME_NONNULL_END
```
&emsp;在 `CusObject.m` 中引入 `#import "CusObject+extension.h"`：
```objective-c
#import "CusObject.h"
#import "CusObject+extension.h"

@implementation CusObject

// 实现在 extension 中添加的方法，
// 并能正常访问成员变量和属性
-(void)testMethod_Extension {
    NSLog(@"%@", name);
    NSLog(@"%@", self.nameTwo);
}

- (void)dealloc {
    NSLog(@"🍀🍀🍀 CusObject deallocing");
}

@end
```

~~如果把 `#import "CusObject+extension.h"` 引入放在 `CusObject.m` 中，表示`extension` 中的成员变量、属性和方法都是私有的。~~

&emsp;如果把 `#import "CusObject+extension.h"` 引入放在 `CusObject.m` 中，则`extension` 中的成员变量、属性和方法只能在类内部使用。

&emsp;注意：如果把 `#import "CusObject+extension.h"` 放在 `CusObject.h` 最上面引入，会直接报错，这里有一个定义先后的问题，此时 `CusObject+extension.h` 处于 `CusObject` 类定义前面, `CusObject` 定义还没有完成，`extension` 必然无法找到 `CusObject`。

&emsp;我们可以把 `#import "CusObject+extension.h"`  放在下面，如下:
```c++
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface CusObject : NSObject

@end

NS_ASSUME_NONNULL_END

#import "CusObject+extension.h"
```
注意：⚠️⚠️ 
+ ~~无论在 `.m` 还是 `.h` 中引入 `extension`，`extension` 中定义的成员变量都是私有的。~~
+ ~~在 `.m` 中引入 `extension`，其中定义的成员变量、属性和方法都是私有的。~~

+ 在 `.m` 中引入 `extension`，其中定义的成员变量、属性和方法只能在类内部使用。
+ 在 `.h` 中引入 `extension`，属性和方法是公开的，成员变量默认是私有的，我们可以在前面添加 `@public` 可以变为公开，访问时要用 `->`。（`.` 和 `->` 的使用在 `C/C++` 和 `Objective-C` 中有一些区别，`OC` 是 `C` 的超集，但是这里它并没有和 `C` 完全相同）
+ 在 `.m` 中给类定义直接添加成员变量，在外部访问时会报错提示成员变量是 `protected` 的。同样也可加 `@public` 公开。

```c++
object->array = @[@(1), @(2)]; ❌❌ // Instance variable 'array' is protected
objc->name = @"chm"; ❌❌ // Instance variable 'name' is private
```

### extension 和 cateogry 区别
1. `extension` 可以添加成员变量，`category` 不能添加成员变量。运行时加载类到内存以后，才会加载分类，这时类的内存布局已经确定（编译器还会对成员变量顺序做出优化，保证遵循内存对齐原则下类占用内存容量最少），如果再去添加成员变量就会破坏类的内存布局。各个成员变量的访问地址是在编译时确定的，每个成员变量的地址偏移都是固定的（相对于类的起始地址的内存偏移（硬编码））。
2. `extension` 在编译期决议（就确定了是类的一部分），`category` 在运行期决议。`extension` 在编译期和头文件里的 `@interface` 以及实现文件里的 `@implement` 一起形成一个完整的类，`extension` 伴随类的产生而产生，亦随之一起消亡。  `category` 中的方法是在运行时决议的，没有实现也可以运行，而 `extension` 中的方法是在编译器检查的，没有实现会报错。
3. `extension` 一般用来隐藏类的私有信息，无法直接为系统的类扩展，但可以先创建系统类的子类再添加 `extension`。 
4. `category` 可以给系统提供的类添加分类。
5. `extension` 和 `category` 都可以添加属性，但是 `category` 中的属性不能生成对应的成员变量以及 `getter` 和 `setter` 方法的实现。
6. `extension` 不能像 `category` 那样拥有独立的实现部分（`@implementation` 部分），`extension` 所声明的方法必须依托对应类的实现部分来实现。

## Category 分类
&emsp;`category` 是 `Objective-C 2.0` 之后添加的语言特性，**它可以在不改变或不继承原类的情况下，动态地给类添加方法**。除此之外还有一些其他的应用场景:
1. 可以把类的的实现分开在几个不同的文件里面。这样做有几个显而易见的好处：

  + 可以减少单个文件的体积。
  + 可以把不同的功能组织到不同的 `category` 里面。
  + 可以由多个开发者共同完成一个类。
  + 可以按需加载想要的 `category`。
  + 声明私有方法。
  
2. 另外还衍生出 `category` 其他几个场景:

  + 模拟多继承（另外可以模拟多继承的还有 `protocol`）。
  + 把 `framework` 的私有方法公开。
  
## category 特点
1. `category` 只能给某个已有的类扩充方法，不能扩充成员变量。
2. `category` 中也可以添加属性，只不过 `@property` 只会生成 `setter` 和 `getter` 的声明，不会生成 `setter` 和 `getter` 的实现以及成员变量。
3. 如果 `category` 中的方法和类中的原用方法同名，运行时会优先调用 `category` 中的方法，也就是，`category` 中的方法会覆盖掉类中原有的方法，所以开发中尽量保证不要让分类中的方法和原有类中的方法名相同，避免出现这种情况的解决方案是给类的方法名统一添加前缀，比如 `category_`。
4. 如果多个 `category` 中存在同名的方法，运行时到底调用哪个方法由编译器决定，最后一个参与编译的方法会被调用。我们可以在 `Compile Sources` 中拖动不同分类的顺序来测试。
5. 调用优先级，`category` > 本类 > 父类。即优先调用 `category` 中的方法，然后调用本类方法，最后调用父类方法。注意：`category` 是在运行时添加的，不是在编译时。

注意：
+ `category` 的方法没有 “完全替换掉” 原来类已经有的方法，也就是说如果 `category` 和原来类都有 `methodA`，那么 `category` 附加完成之后，类的方法列表里会有两个 `methodA`。
+ `category` 的方法被放到了新方法列表的前面，而原来类的方法被放到了新方法列表的后面，这也就是我们平常所说的 `category` 的方法会 “覆盖” 掉原来类的同名方法，这是因为运行时在查找方法的时候是顺着方法列表的顺序查找的，它只要一找到对应名字的方法，就会罢休，殊不知后面可能还有一样名字的方法。

## 为什么 category 不能添加成员变量？
&emsp;`Objective-C` 中类是由 `Class` 类型来表示的，它实际上是一个指向 `objc_class` 结构体的指针，如下:
```c++
typedef struct objc_class *Class;
```
&emsp;`objc_class` 结构体定义如下:
```c++
// objc_class

struct objc_class : objc_object {
// Class ISA;
Class superclass;
cache_t cache;             // formerly cache pointer and vtable
class_data_bits_t bits;    // class_rw_t * plus custom rr/alloc flags

class_rw_t *data() const {
    return bits.data();
}

...
};

// class_data_bits_t

struct class_data_bits_t {
    friend objc_class;

    // Values are the FAST_ flags above.
    uintptr_t bits;
    ...
public:

    class_rw_t* data() const {
        return (class_rw_t *)(bits & FAST_DATA_MASK);
    }
    ...

    // Get the class's ro data, even in the presence of concurrent realization.
    // fixme this isn't really safe without a compiler barrier at least
    // and probably a memory barrier when realizeClass changes the data field
    const class_ro_t *safe_ro() {
        class_rw_t *maybe_rw = data();
        if (maybe_rw->flags & RW_REALIZED) {
            // maybe_rw is rw
            return maybe_rw->ro();
        } else {
            // maybe_rw is actually ro
            return (class_ro_t *)maybe_rw;
        }
    }
    ...
};

// class_rw_t

struct class_rw_t {
    // Be warned that Symbolication knows the layout of this structure.
    uint32_t flags;
    uint16_t witness;
#if SUPPORT_INDEXED_ISA
    uint16_t index;
#endif

    explicit_atomic<uintptr_t> ro_or_rw_ext;

    Class firstSubclass;
    Class nextSiblingClass;
    ...

public:
    ...

    const method_array_t methods() const {
        auto v = get_ro_or_rwe();
        if (v.is<class_rw_ext_t *>()) {
            return v.get<class_rw_ext_t *>()->methods;
        } else {
            return method_array_t{v.get<const class_ro_t *>()->baseMethods()};
        }
    }

    const property_array_t properties() const {
        auto v = get_ro_or_rwe();
        if (v.is<class_rw_ext_t *>()) {
            return v.get<class_rw_ext_t *>()->properties;
        } else {
            return property_array_t{v.get<const class_ro_t *>()->baseProperties};
        }
    }

    const protocol_array_t protocols() const {
        auto v = get_ro_or_rwe();
        if (v.is<class_rw_ext_t *>()) {
            return v.get<class_rw_ext_t *>()->protocols;
        } else {
            return protocol_array_t{v.get<const class_ro_t *>()->baseProtocols};
        }
    }
};

// class_ro_t

struct class_ro_t {
    uint32_t flags;
    uint32_t instanceStart;
    uint32_t instanceSize;
#ifdef __LP64__
    uint32_t reserved;
#endif

    const uint8_t * ivarLayout;
    
    const char * name;
    method_list_t * baseMethodList;
    protocol_list_t * baseProtocols;
    const ivar_list_t * ivars;

    const uint8_t * weakIvarLayout;
    property_list_t *baseProperties;

    ...

    method_list_t *baseMethods() const {
        return baseMethodList;
    }
    ...
};
```
&emsp;在上面一连串的数据结构定义中，`ivars` 是 `const ivar_list_t *`。在 `runtime` 中， `objc_class` 结构体大小是固定的，不可能往这个结构体中添加数据。且这里加了 `const` 修饰符，所以 `ivars` 指向一个固定的区域，不能修改成员变量值，也不能增加成员变量个数。

## category 中能添加属性吗？
&emsp;`category` 不能添加成员变量（`instance variables`），那到底能不能添加属性（`@property`）呢？

&emsp;从 `category` 的结构体开始分析:
&emsp;`category_t` 定义:
```c++
// classref_t is unremapped class_t*
typedef struct classref * classref_t;
```
```c++
struct category_t {
    const char *name;
    classref_t cls;
    struct method_list_t *instanceMethods;
    struct method_list_t *classMethods;
    struct protocol_list_t *protocols;
    struct property_list_t *instanceProperties;
    // Fields below this point are not always present on disk.
    struct property_list_t *_classProperties;

    method_list_t *methodsForMeta(bool isMeta) {
        if (isMeta) return classMethods;
        else return instanceMethods;
    }

    property_list_t *propertiesForMeta(bool isMeta, struct header_info *hi);
    
    protocol_list_t *protocolsForMeta(bool isMeta) {
        if (isMeta) return nullptr;
        else return protocols;
    }
};
```
&emsp;从 `category` 定义中可以看出 `category` 可以添加实例方法、类方法甚至可以实现协议、添加属性，同时也看到不能添加成员变量。
那为什么说不能添加属性呢？实际上，`category` 允许添加属性，可以使用 `@property` 添加，但是能添加 `@property` 不代表可以添加 “完整版的” 属性，通常我们说的添加属性是指编译器为我们生成了对应的成员变量和对应的 `setter` 和 `getter` 方法来存取属性。在 `category` 中虽说可以书写 `@property`，但是不会生成 \_成员变量，也不会生成所添加属性的 `getter` 和 `setter` 方法的实现，所以尽管添加了属性，也无法使用点语法调用 `setter` 和 `getter` 方法。（实际上，点语法可以写，只不过在运行时调用到这个方法时会报找不到方法的错误: `unrecognized selector sent to instance ....`）。我们此时可以通过 `associated object` 来为属性手动实现 `setter` 和 `getter` 存取方法。

## 从 clang 编译文件来验证上面两个问题
&emsp;我们先用 `clang` 编译文件（这里建议大家在 `xcode` 和终端上自己试一下）。首先定义如下类 `CustomObject` 只声明一个属性:
```objective-c
// CustomObject.h
#import <Foundation/Foundation.h>
NS_ASSUME_NONNULL_BEGIN
@interface CustomObject : NSObject

@property (nonatomic, copy) NSString *customProperty;

@end
NS_ASSUME_NONNULL_END

// CustomObject.m
#import "CustomObject.h"
@implementation CustomObject
@end
```
&emsp;然后打开终端进入到 `CustomObject.m` 文件所在文件夹，执行 `clang -rewrite-objc CustomObject.m` 指令，然后生成 `CustomObject.cpp` 文件，查看它：

&emsp;`struct CustomObject_IMPL` 定义：
```c++
extern "C" unsigned long OBJC_IVAR_$_CustomObject$_customProperty;
struct CustomObject_IMPL {
    struct NSObject_IMPL NSObject_IVARS;
    NSString * _Nonnull _customProperty;
};

// @property (nonatomic, copy) NSString *customProperty;

/* @end */
```
&emsp;看到为我们增加了 `_customProperty` 成员变量，`NSObject_IVARS` 是每个继承自 `NSObject` 都会有的成员变量。
`@implementation CustomObject` 部分：
```c++
// @implementation CustomObject

static NSString * _Nonnull _I_CustomObject_customProperty(CustomObject * self, SEL _cmd) { return (*(NSString * _Nonnull *)((char *)self + OBJC_IVAR_$_CustomObject$_customProperty)); }
extern "C" __declspec(dllimport) void objc_setProperty (id, SEL, long, id, bool, bool);

static void _I_CustomObject_setCustomProperty_(CustomObject * self, SEL _cmd, NSString * _Nonnull customProperty) { objc_setProperty (self, _cmd, __OFFSETOFIVAR__(struct CustomObject, _customProperty), (id)customProperty, 0, 1); }
// @end
```
&emsp;看到我们的 `customProperty` 的 `setter` 和 `getter` 方法，到这里可印证：**类中添加属性编译器自动生成了成员变量和对应的 setter 和 getter 方法。**（这里刚好可以和 `category` 中不会生成作对比）
&emsp;接下来看 `getter` 函数的实现:
```c++
return (*(NSString * _Nonnull *)((char *)self + OBJC_IVAR_$_CustomObject$_customProperty));
```
&emsp;`self` 是我们的入参 `CustomObject * self`，然后它做了一个指针加法。这个 `OBJC_IVAR_$_CustomObject$_customProperty` 是 `_customProperty` 相对于 `self` 的指针偏移。
```c++
// 1 定义，其实它是一个 unsigned long 
extern "C" unsigned long OBJC_IVAR_$_CustomObject$_customProperty;

// 2 _customProperty 成员变量位置相对 struct CustomObject 的偏移
#define __OFFSETOFIVAR__(TYPE, MEMBER) ((long long) &((TYPE *)0)->MEMBER)
extern "C" unsigned long int OBJC_IVAR_$_CustomObject$_customProperty __attribute__ ((used, section ("__DATA,__objc_ivar"))) =
__OFFSETOFIVAR__(struct CustomObject, _customProperty);

// 3 成员变量列表，看到只有我们的 _customProperty
static struct /*_ivar_list_t*/ {
    unsigned int entsize;  // sizeof(struct _prop_t)
    unsigned int count;
    struct _ivar_t ivar_list[1];
} _OBJC_$_INSTANCE_VARIABLES_CustomObject __attribute__ ((used, section ("__DATA,__objc_const"))) = {
    sizeof(_ivar_t),
    1,
    {{(unsigned long int *)&OBJC_IVAR_$_CustomObject$_customProperty, "_customProperty", "@\"NSString\"", 3, 8}}
};

// _ivar_t 定义
struct _ivar_t {
    // 指向 ivar 偏移位置的指针
    unsigned long int *offset;  // pointer to ivar offset location
    const char *name;
    const char *type;
    unsigned int alignment;
    unsigned int  size;
};
```
&emsp;看到成员变量的访问是通过指针偏移来做的，而偏移距离都是结构体内存布局已经死死固定的。当 `category` 整合到它对应的类时，类的布局已固定，自然就不能再给它添加新的成员变量了。

&emsp;下面我们 `clang` 编译 `category` 文件：`NSObject+customCategory.h` 文件：
```c++
#import <Foundation/Foundation.h>
NS_ASSUME_NONNULL_BEGIN
@interface NSObject (customCategory)

@property (nonatomic, copy) NSString *categoryProperty_one;
@property (nonatomic, strong) NSMutableArray *categoryProperty_two;

- (void)customInstanceMethod_one;
- (void)customInstanceMethod_two;
+ (void)customClassMethod_one;
+ (void)customClassMethod_two;

@end
NS_ASSUME_NONNULL_END
```
&emsp;`NSObject+customCategory.m` 文件：
```c++
#import "NSObject+customCategory.h"
@implementation NSObject (customCategory)
- (void)customInstanceMethod_one {
    NSLog(@"🧑‍🍳 %@ invokeing", NSStringFromSelector(_cmd));
}
- (void)customInstanceMethod_two {
    NSLog(@"🧑‍🍳 %@ invokeing", NSStringFromSelector(_cmd));
}
+ (void)customClassMethod_one {
    NSLog(@"🧑‍🍳 %@ invokeing", NSStringFromSelector(_cmd));
}
+ (void)customClassMethod_two {
    NSLog(@"🧑‍🍳 %@ invokeing", NSStringFromSelector(_cmd));
}
@end
```
&emsp;浏览摘录 `NSObject+customCategory.cpp` 文件:
```c++
// @implementation NSObject (customCategory)
static void _I_NSObject_customCategory_customInstanceMethod_one(NSObject * self, SEL _cmd) {
    NSLog((NSString *)&__NSConstantStringImpl__var_folders_24_5w9yv8jx63bgfg69gvgclmm40000gn_T_NSObject_customCategory_740f85_mi_0, NSStringFromSelector(_cmd));
}
static void _I_NSObject_customCategory_customInstanceMethod_two(NSObject * self, SEL _cmd) {
    NSLog((NSString *)&__NSConstantStringImpl__var_folders_24_5w9yv8jx63bgfg69gvgclmm40000gn_T_NSObject_customCategory_740f85_mi_1, NSStringFromSelector(_cmd));
}
static void _C_NSObject_customCategory_customClassMethod_one(Class self, SEL _cmd) {
    NSLog((NSString *)&__NSConstantStringImpl__var_folders_24_5w9yv8jx63bgfg69gvgclmm40000gn_T_NSObject_customCategory_740f85_mi_2, NSStringFromSelector(_cmd));
}
static void _C_NSObject_customCategory_customClassMethod_two(Class self, SEL _cmd) {
    NSLog((NSString *)&__NSConstantStringImpl__var_folders_24_5w9yv8jx63bgfg69gvgclmm40000gn_T_NSObject_customCategory_740f85_mi_3, NSStringFromSelector(_cmd));
}
// @end
```
&emsp;看到只有我们的两个实例方法和两个类方法，没有添加成员变量也没有任何属性的 `setter` 和 `getter` 方法。这里即可印证：**category 不能添加属性。**
```c++
// 两个实例方法
static struct /*_method_list_t*/ {
    unsigned int entsize;  // sizeof(struct _objc_method)
    unsigned int method_count;
    struct _objc_method method_list[2];
} _OBJC_$_CATEGORY_INSTANCE_METHODS_NSObject_$_customCategory __attribute__ ((used, section ("__DATA,__objc_const"))) = {
    sizeof(_objc_method),
    2,
    {{(struct objc_selector *)"customInstanceMethod_one", "v16@0:8", (void *)_I_NSObject_customCategory_customInstanceMethod_one},
    {(struct objc_selector *)"customInstanceMethod_two", "v16@0:8", (void *)_I_NSObject_customCategory_customInstanceMethod_two}}
};

// 两个类方法
static struct /*_method_list_t*/ {
    unsigned int entsize;  // sizeof(struct _objc_method)
    unsigned int method_count;
    struct _objc_method method_list[2];
} _OBJC_$_CATEGORY_CLASS_METHODS_NSObject_$_customCategory __attribute__ ((used, section ("__DATA,__objc_const"))) = {
    sizeof(_objc_method),
    2,
    {{(struct objc_selector *)"customClassMethod_one", "v16@0:8", (void *)_C_NSObject_customCategory_customClassMethod_one},
    {(struct objc_selector *)"customClassMethod_two", "v16@0:8", (void *)_C_NSObject_customCategory_customClassMethod_two}}
};

// 两个属性
static struct /*_prop_list_t*/ {
    unsigned int entsize;  // sizeof(struct _prop_t)
    unsigned int count_of_properties;
    struct _prop_t prop_list[2];
} _OBJC_$_PROP_LIST_NSObject_$_customCategory __attribute__ ((used, section ("__DATA,__objc_const"))) = {
    sizeof(_prop_t),
    2,
    {{"categoryProperty_one","T@\"NSString\",C,N"},
    {"categoryProperty_two","T@\"NSMutableArray\",&,N"}}
};
```
&emsp;看到类方法、实例方法和属性的结构体：
```c++
static struct _category_t _OBJC_$_CATEGORY_NSObject_$_customCategory __attribute__ ((used, section ("__DATA,__objc_const"))) = 
{
    "NSObject",
    0, // &OBJC_CLASS_$_NSObject,
    (const struct _method_list_t *)&_OBJC_$_CATEGORY_INSTANCE_METHODS_NSObject_$_customCategory,
    (const struct _method_list_t *)&_OBJC_$_CATEGORY_CLASS_METHODS_NSObject_$_customCategory,
    0,
    (const struct _prop_list_t *)&_OBJC_$_PROP_LIST_NSObject_$_customCategory,
};
```
&emsp;以上三者构成 `_category_t` 结构体实例。

## category 原理
> 即使我们不引入 `category` 的头文件，`category` 中的方法也会被添加进主类中，我们可以通 `performSelector:` 等方式对 `category` 中的方法进行调用: 
  + 将 `category` 和它的主类（或元类）注册到哈希表中，形成映射关系。（`ExplicitInitDenseMap<Class, category_list>`）
  + 如果主类（或元类）已实现，那么重建它的方法列表。

## category 相关数据结构
&emsp;到这里突然有些茫然，不知道从哪里入手，已知 `category` 是在 `runtime` 初始化时开始加载的，这里涉及到 `runtime` 的加载流程，暂且不表。我们还是先来一层一层剥开相关的数据结构。

&emsp;可绘制这样一个关系图：
![Category 数据结构](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/40aed4dd4a944043b4e2bc0c4b1fe6bf~tplv-k3u1fbpfcp-zoom-1.image)

### category_t
```c++
typedef struct category_t *Category;

// classref_t is unremapped class_t*
typedef struct classref * classref_t;

struct category_t {
    const char *name; // 分类的名字
    classref_t cls; // 所属的类 
    struct method_list_t *instanceMethods; // 实例方法列表
    struct method_list_t *classMethods; // 类方法列表
    struct protocol_list_t *protocols; // 协议列表
    struct property_list_t *instanceProperties; // 实例属性列表
    
    // Fields below this point are not always present on disk.
    struct property_list_t *_classProperties; // 类属性？
    
    // 返回 类/元类 方法列表
    method_list_t *methodsForMeta(bool isMeta) {
        if (isMeta) return classMethods;
        else return instanceMethods;
    }

    property_list_t *propertiesForMeta(bool isMeta, struct header_info *hi);
    
    // 协议列表，元类没有协议列表
    protocol_list_t *protocolsForMeta(bool isMeta) {
        // 这里如果是元类的话会返回 nullptr，
        // 但是在 load_categories_nolock 函数
        // 中有貌似把 protocols 添加到元类的迹象
        // 但是在 attachCategories 函数
        // 中 protocolsForMeta 函数返回 nullptr
        // 应该是没有实际添加
        if (isMeta) return nullptr;
        else return protocols;
    }
};

/*
* category_t::propertiesForMeta
* Return a category's instance or class properties.
* 返回 category 的实例或类的属性。
* hi is the image containing the category.
* hi 是包含 category 的镜像（images）。
*/
property_list_t *
category_t::propertiesForMeta(bool isMeta, struct header_info *hi)
{
    if (!isMeta) return instanceProperties;
    else if (hi->info()->hasCategoryClassProperties()) return _classProperties;
    else return nil;
}
```

### method_t
&emsp;方法的数据结构，很简单。
```c++
struct method_t {
    SEL name; // 方法名、选择子
    const char *types; // 方法类型
    
    // using MethodListIMP = IMP;
    MethodListIMP imp; // 方法实现

    // 根据选择子的地址进行排序
    struct SortBySELAddress :
        public std::binary_function<const method_t&,
                                    const method_t&, bool>
    {
        bool operator() (const method_t& lhs,
                         const method_t& rhs)
        { return lhs.name < rhs.name; }
    };
};
```
&emsp;可参考 [stl 中 std::binary_function 的使用](https://blog.csdn.net/tangaowen/article/details/7547475)

### entsize_list_tt 
&emsp;下面先看一下超长的 `entsize_list_tt`，它可理解为一个数据容器，拥有自己的迭代器用于遍历所有元素。（`ent` 应该是 `entry` 的缩写）
```c++
/***********************************************************************
* entsize_list_tt<Element, List, FlagMask>
* Generic implementation of an array of non-fragile structs.
*
* Element is the struct type (e.g. method_t)
* Element 是结构体类型，如: method_t

* List is the specialization of entsize_list_tt (e.g. method_list_t)
* List 是 entsize_list_tt 指定类型，如: method_list_t

* FlagMask is used to stash extra bits in the entsize field
*   (e.g. method list fixup markers)
* 标记位
* FlagMask 用于将多余的位藏匿在 entsize 字段中
* 如: 方法列表修复标记
**********************************************************************/
template <typename Element, typename List, uint32_t FlagMask>
struct entsize_list_tt {
    uint32_t entsizeAndFlags;
    // 容器的容量
    uint32_t count;
    // 第一个元素
    Element first;
    
    // 元素的大小
    uint32_t entsize() const {
        return entsizeAndFlags & ~FlagMask;
    }
    
    // 取出 flags
    uint32_t flags() const {
        return entsizeAndFlags & FlagMask;
    }

    // 根据索引返回指定元素的的引用，这 i 可以等于 count
    // 意思是可以返回最后一个元素的后面
    Element& getOrEnd(uint32_t i) const {
        // 断言，i 不能超过 count
        ASSERT(i <= count);
        // 首先取出 first 地址然后指针偏移 i * ensize() 个长度
        // 然后转换为 Element 指针，然后取出指针指向内容返回
        // 返回类型是 Element 引用
        return *(Element *)((uint8_t *)&first + i*entsize()); 
    }
    
    // 在索引范围内返回 Element 引用
    Element& get(uint32_t i) const { 
        ASSERT(i < count);
        return getOrEnd(i);
    }
    
    // 容器占用的内存总长度，以字节为单位
    size_t byteSize() const {
        return byteSize(entsize(), count);
    }
    
    // entsize 单个元素的内存长度，count 是元素的个数
    static size_t byteSize(uint32_t entsize, uint32_t count) {
        // 首先算出 struct entsize_list_tt 的长度：
        // uint32_t entsizeAndFlags + uint32_t count + Element first
        // 三个成员变量的总长度，然后加上 (count - 1) 个元素的长度
        return sizeof(entsize_list_tt) + (count-1)*entsize;
    }

    // 复制一份 List
    List *duplicate() const {
        // 开辟 byteSize() 长度空间并置为 1
        auto *dup = (List *)calloc(this->byteSize(), 1);
        // 成员变量赋值
        dup->entsizeAndFlags = this->entsizeAndFlags;
        dup->count = this->count;
        // 原数据的从 begin() 到 end() 的内容复制到以 dup->begin()
        // 为起始地址的空间内
        std::copy(begin(), end(), dup->begin());
        return dup;
    }
    
    // 自定义的迭代器的声明，实现在下面
    struct iterator;
    
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
    
    // 即返回指向容器最后一个元素的后面的迭代器，
    // 注意这里不是指向最后一个元素，
    // 而是指向最后一个的后面
    const iterator end() const { 
        return iterator(*static_cast<const List*>(this), count); 
    }
    
    // 同上，去掉了两个 const 限制
    iterator end() { 
        return iterator(*static_cast<const List*>(this), count); 
    }
    
    // 下面是自定义的迭代器
    struct iterator {
        // 每个元素的大小
        uint32_t entsize;
        // 当前迭代器的索引
        uint32_t index;  // keeping track of this saves a divide in operator-
        // 元素指针
        Element* element;

        // 类型定义
        typedef std::random_access_iterator_tag iterator_category;
        typedef Element value_type;
        typedef ptrdiff_t difference_type;
        typedef Element* pointer;
        typedef Element& reference;
        
        // 构造函数
        iterator() { }
        
        // 构造函数
        iterator(const List& list, uint32_t start = 0)
            : entsize(list.entsize())
            , index(start)
            , element(&list.getOrEnd(start))
        { }

        // 重载操作符
        const iterator& operator += (ptrdiff_t delta) {
            // 指针偏移
            element = (Element*)((uint8_t *)element + delta*entsize);
            // 更新 index
            index += (int32_t)delta;
            // 返回 *this
            return *this;
        }
        
        const iterator& operator -= (ptrdiff_t delta) {
            element = (Element*)((uint8_t *)element - delta*entsize);
            index -= (int32_t)delta;
            return *this;
        }
        
        // 以下都是 += 和 -= 的应用
        const iterator operator + (ptrdiff_t delta) const {
            return iterator(*this) += delta;
        }
        const iterator operator - (ptrdiff_t delta) const {
            return iterator(*this) -= delta;
        }

        iterator& operator ++ () { *this += 1; return *this; }
        iterator& operator -- () { *this -= 1; return *this; }
        iterator operator ++ (int) {
            iterator result(*this); *this += 1; return result;
        }
        iterator operator -- (int) {
            iterator result(*this); *this -= 1; return result;
        }
        
        // 两个迭代器的之间的距离
        ptrdiff_t operator - (const iterator& rhs) const {
            return (ptrdiff_t)this->index - (ptrdiff_t)rhs.index;
        }

        // 返回元素指针或引用
        Element& operator * () const { return *element; }
        Element* operator -> () const { return element; }
        operator Element& () const { return *element; }

        // 判等，看到的是直接比较 element 的地址
        // 哦哦，也不是，== 可能被抽象类型 Element 重载
        bool operator == (const iterator& rhs) const {
            return this->element == rhs.element;
        }
        // 不等
        bool operator != (const iterator& rhs) const {
            return this->element != rhs.element;
        }
        
        // 比较
        bool operator < (const iterator& rhs) const {
            return this->element < rhs.element;
        }
        bool operator > (const iterator& rhs) const {
            return this->element > rhs.element;
        }
    };
};
```

### method_list_t
```c++
// Two bits of entsize are used for fixup markers.
// entsize 的后两位用于固定标记
struct method_list_t : entsize_list_tt<method_t, method_list_t, 0x3> {
    bool isUniqued() const;
    bool isFixedUp() const;
    void setFixedUp();
    
    // 返回指定 meth 的 index
    //（指针距离除以元素宽度）
    uint32_t indexOfMethod(const method_t *meth) const {
        uint32_t i = 
            (uint32_t)(((uintptr_t)meth - (uintptr_t)this) / entsize());
        ASSERT(i < count);
        return i;
    }
};
```
&emsp;在 `objc-runtime-new.mm` 看下 `method_list_t` 的函数实现:
```c++
static uint32_t uniqued_method_list = 1;
bool method_list_t::isUniqued() const {
    return (flags() & uniqued_method_list) != 0;
}

static uint32_t fixed_up_method_list = 3;
bool method_list_t::isFixedUp() const {
    return flags() == fixed_up_method_list;
}

void method_list_t::setFixedUp() {
    runtimeLock.assertLocked();
    ASSERT(!isFixedUp());
    entsizeAndFlags = entsize() | fixed_up_method_list;
}
```

```c++
/*
  Low two bits of mlist->entsize is used as the fixed-up marker.
  method_list_t 的 entsize 的低两位用作固定标记。
  
  PREOPTIMIZED VERSION:
  预优化版本：
  
    Method lists from shared cache are 1 (uniqued) or 3 (uniqued and sorted).
    来自 shared cache 的 Method lists 为 1（唯一）或 3（唯一且已排序）
    
    (Protocol method lists are not sorted because of their extra parallel data)
    Runtime fixed-up method lists get 3.
    指 method_list_t 继承 entsize_list_tt 的模版参数 FlagMask hardcode 是 0x3
    
  UN-PREOPTIMIZED VERSION:
  未预优化版本：
  
    Method lists from shared cache are 1 (uniqued) or 3 (uniqued and sorted)
    来自 shared cache 的 Method lists 为 1（唯一）或 3（唯一且已排序）
    
    Shared cache's sorting and uniquing are not trusted, 
    but do affect the location of the selector name string.
    
    Runtime fixed-up method lists get 2.
*/
// 静态全局变量
static uint32_t fixed_up_method_list = 3;
static uint32_t uniqued_method_list = 1;
```
&emsp;`method_list_t` 的 `FlagMask` 是 `0x3`，即二进制: `0b11`，`FlagMask` 会在把 `category` 的方法追加到类前调用 `prepareMethodLists` 函数里面用到，用于判断是否需要把方法列表调整为 `uniqued and sorted`。

### protocol_list_t
```c++
struct protocol_list_t {
    // count is pointer-sized by accident.
    // count 是指针宽度
    
    uintptr_t count;
    
    // typedef uintptr_t protocol_ref_t;  // protocol_t *, but unremapped
    // protocol_ref_t 为 protocol_t * 
    // 此处虽然数组长度用的 0，不过它是运行期可变的
    // 其实是 C99 中的一种写法，允许我们在运行期动态的申请内存。
    protocol_ref_t list[0]; // variable-size
    
    // 字节容量，同 entsize_list_tt，但是这里 count 没有减 1
    // 因为数组初始用的 0 
    size_t byteSize() const {
        return sizeof(*this) + count*sizeof(list[0]);
    }

//    static inline void *
//    memdup(const void *mem, size_t len)
//    {
//        void *dup = malloc(len);
//        memcpy(dup, mem, len);
//        return dup;
//    }

// void *memcpy(void *destin, void *source, unsigned n);
// 从源 source 所指的内存地址的起始位置开始拷贝 n 个字节到目标 destin 所指的内存地址的起始位置中。

    // 复制函数
    protocol_list_t *duplicate() const {
        return (protocol_list_t *)memdup(this, this->byteSize());
    }

    // 类型定义
    typedef protocol_ref_t* iterator;
    typedef const protocol_ref_t* const_iterator;
    
    // begin 指针
    const_iterator begin() const {
        return list;
    }
    
    iterator begin() {
        return list;
    }
    
    // 结束位置指针
    const_iterator end() const {
        return list + count;
    }
    iterator end() {
        return list + count;
    }
};
```

### property_list_t
```c++
struct property_list_t : entsize_list_tt<property_t, property_list_t, 0> {
};
```
&emsp;继承自 `entsize_list_tt`，它的 `FlagMask` `hardcode` 是 `0`。

### property_t
```c++
struct property_t {
    const char *name;
    const char *attributes;
};
```

### locstamped_category_t
```c++
struct locstamped_category_t {
    category_t *cat;
    // header 数据
    struct header_info *hi;
};
```

### category_list
```c++
// class nocopy_t 构造函数和析构函数使用编译器默认生成的，删除复制构造函数和赋值函数
class category_list : nocopy_t {
    // 联合体变量 _u
    union {
        // lc 与下面的 struct 构成，
        // is_array 表示一个数组还是只是一个 locstamped_category_t
        locstamped_category_t lc; // 占 16 个字节
        
        struct {
            // locstamped_category_t 指针
            locstamped_category_t *array; // 8 个字节，下面 8 个字节
            
            // 根据数据量切换不同的存储形态。类似 weak_entry_t 的数据结构，
            // 开始先用定长为 4 的数组保存弱引用指针，然后大于 4 以后切换为哈希数组保存，
            // 也类似 class_rw_ext_t 中的方法列表，是保存一个方法列表指针，还是保存一个数组每个数组元素都是一个方法列表指针
            
            // this aliases with locstamped_category_t::hi
            // which is an aliased pointer
            // 位域
            uint32_t is_array :  1;
            uint32_t count    : 31;
            uint32_t size     : 32;
        };
    } _u;

public:
    // 构造函数
    // _u 初始化列表 lc 和 struct 都为 nullptr
    category_list() : _u{{nullptr, nullptr}} { }
    
    // _u lc 初始化
    category_list(locstamped_category_t lc) : _u{{lc}} { }
    
    // 入参 category_list &&
    category_list(category_list &&other) : category_list() {
        std::swap(_u, other._u);
    }
    
    // 析构函数
    ~category_list()
    {
        if (_u.is_array) {
            free(_u.array);
        }
    }

    // conunt 表示的是 category_t 的数量
    uint32_t count() const
    {
        if (_u.is_array) return _u.count;
        return _u.lc.cat ? 1 : 0;
    }

    // 内存容量
    // sizeof(locstamped_category_t) 应该是 16
    uint32_t arrayByteSize(uint32_t size) const
    {
        return sizeof(locstamped_category_t) * size;
    }
    
    // locstamped_category_t 指针
    const locstamped_category_t *array() const
    {
        return _u.is_array ? _u.array : &_u.lc;
    }

    // 拼接
    void append(locstamped_category_t lc)
    {
        if (_u.is_array) {
            // 如果是数组
            if (_u.count == _u.size) {
                // 如果已经存满了
                // 扩容
                // Have a typical malloc growth:
                // - size <=  8: grow by 2
                // - size <= 16: grow by 4
                // - size <= 32: grow by 8
                // ... etc
                _u.size += _u.size < 8 ? 2 : 1 << (fls(_u.size) - 2);
                _u.array = (locstamped_category_t *)reallocf(_u.array, arrayByteSize(_u.size));
            }
            // 依次向数组里面放 locstamped_category_t
            _u.array[_u.count++] = lc;
        } else if (_u.lc.cat == NULL) {
            // 如果还没有保存任何数据，使用 lc 成员变量
            _u.lc = lc;
        } else {
            // 由原始的一个 locstamped_category_t lc 转变为指针数组存储 locstamped_category_t
            locstamped_category_t *arr = (locstamped_category_t *)malloc(arrayByteSize(2));
            arr[0] = _u.lc;
            arr[1] = lc;

            _u.array = arr;
            _u.is_array = true;
            _u.count = 2;
            _u.size = 2;
        }
    }

    // 擦除，（只需要清除内容，并不需要释放原始的 16 字节的空间）
    void erase(category_t *cat)
    {
        if (_u.is_array) {
            // 如果已经以数组形式保存，则遍历
            for (int i = 0; i < _u.count; i++) {
                if (_u.array[i].cat == cat) {
                    // shift entries to preserve list order
                    // 移动数组，删除 cat
                    memmove(&_u.array[i], &_u.array[i+1], arrayByteSize(_u.count - i - 1));
                    return;
                }
            }
        } else if (_u.lc.cat == cat) {
            // 如果只有一个 cat，则置为 nil
            _u.lc.cat = NULL;
            _u.lc.hi = NULL;
        }
    }
};
```

### UnattachedCategories
```c++
// unattachedCategories 是一个静态全局变量，隶属于 namespace objc，存放未追加到类的分类数据。
static UnattachedCategories unattachedCategories;
```

```c++
// 一个公开继承自 ExplicitInitDenseMap<Class, category_list> 的类
// 抽象参数分别是 Class、category_list
// 从数据结构及使用角度，可以把它理解 key 是 Class value 是 category_list 的哈希表
class UnattachedCategories : public ExplicitInitDenseMap<Class, category_list>
{
public:
    // 向指定的 cls 中添加 locstamped_category_t 
    void addForClass(locstamped_category_t lc, Class cls)
    {
        runtimeLock.assertLocked();

        if (slowpath(PrintConnecting)) {
            _objc_inform("CLASS: found category %c%s(%s)",
                         cls->isMetaClass() ? '+' : '-',
                         cls->nameForLogging(), lc.cat->name);
        }

        //  向 unattachedCategories 添加 <Class, category_list>
        auto result = get().try_emplace(cls, lc);
        if (!result.second) {
            // 如果 cls 已经存在 category_list，则把 lc 添加到 category_list 的数组中
            // 这里 append 是 category_list 的 append 函数
            // result.first->second 即是 cls 对应的 category_list
            result.first->second.append(lc);
        }
    }

    // 把 previously 的 categories 数据添加到 cls 上
    void attachToClass(Class cls, Class previously, int flags)
    {
        runtimeLock.assertLocked();
        ASSERT((flags & ATTACH_CLASS) ||
               (flags & ATTACH_METACLASS) ||
               (flags & ATTACH_CLASS_AND_METACLASS));

        auto &map = get();
        auto it = map.find(previously);

        if (it != map.end()) {
            category_list &list = it->second;
            if (flags & ATTACH_CLASS_AND_METACLASS) {
                int otherFlags = flags & ~ATTACH_CLASS_AND_METACLASS;
                
                // attachCategories 函数追加分类内容到类中去，下篇详细解析此函数
                
                attachCategories(cls, list.array(), list.count(), otherFlags | ATTACH_CLASS);
                attachCategories(cls->ISA(), list.array(), list.count(), otherFlags | ATTACH_METACLASS);
            } else {
                attachCategories(cls, list.array(), list.count(), flags);
            }
            map.erase(it);
        }
    }

    void eraseCategoryForClass(category_t *cat, Class cls)
    {
        runtimeLock.assertLocked();

        auto &map = get();
        auto it = map.find(cls);
        if (it != map.end()) {
            category_list &list = it->second;
            // 移除 category_list 中保存的 cat（locstamped_category_t）
            list.erase(cat);
            if (list.count() == 0) {
                // 如果 category_list 空了，则把 <Class, category_list> 移除
                map.erase(it);
            }
        }
    }

    void eraseClass(Class cls)
    {
        runtimeLock.assertLocked();
        // 删除指定 cls 的 <Class, category_list>
        get().erase(cls);
    }
};
```
&emsp;到这里 `category_t` 相关的数据结构基本看完了，并不复杂。在之前我们用 `clang` 编译我们的类文件和分类文件的时候，已经看到生成的 `_category_t` 结构体，下面我们再解读一下 `clang` 后的 `.cpp` 文件内容：

### \_OBJC_$_CATEGORY_INSTANCE_METHODS_NSObject_$_customCategory
&emsp;编译器生成实例方法列表保存在 **DATA段的** `objc_const` `section` 里（`struct /*_method_list_t*/`）。 
```c++
static struct /*_method_list_t*/ {
    unsigned int entsize;  // sizeof(struct _objc_method)
    unsigned int method_count;
    struct _objc_method method_list[2];
} _OBJC_$_CATEGORY_INSTANCE_METHODS_NSObject_$_customCategory __attribute__ ((used, section ("__DATA,__objc_const"))) = {
    sizeof(_objc_method),
    2,
    {{(struct objc_selector *)"customInstanceMethod_one", "v16@0:8", (void *)_I_NSObject_customCategory_customInstanceMethod_one},
    {(struct objc_selector *)"customInstanceMethod_two", "v16@0:8", (void *)_I_NSObject_customCategory_customInstanceMethod_two}}
};
```

### \_OBJC_$_CATEGORY_CLASS_METHODS_NSObject_$_customCategory
&emsp;编译器生成类方法列表保存在 **DATA段的** `objc_const` `section` 里（`struct /*_method_list_t*/`）。
```c++
static struct /*_method_list_t*/ {
    unsigned int entsize;  // sizeof(struct _objc_method)
    unsigned int method_count;
    struct _objc_method method_list[2];
} _OBJC_$_CATEGORY_CLASS_METHODS_NSObject_$_customCategory __attribute__ ((used, section ("__DATA,__objc_const"))) = {
    sizeof(_objc_method),
    2,
    {{(struct objc_selector *)"customClassMethod_one", "v16@0:8", (void *)_C_NSObject_customCategory_customClassMethod_one},
    {(struct objc_selector *)"customClassMethod_two", "v16@0:8", (void *)_C_NSObject_customCategory_customClassMethod_two}}
};
```

### \_OBJC_$_PROP_LIST_NSObject_$_customCategory
&emsp;编译器生成属性列表保存在 **DATA段的** `objc_const` `section` 里（`struct /*_prop_list_t*/`）。
```c++
static struct /*_prop_list_t*/ {
    unsigned int entsize;  // sizeof(struct _prop_t)
    unsigned int count_of_properties;
    struct _prop_t prop_list[2];
} _OBJC_$_PROP_LIST_NSObject_$_customCategory __attribute__ ((used, section ("__DATA,__objc_const"))) = {
    sizeof(_prop_t),
    2,
    {{"categoryProperty_one","T@\"NSString\",C,N"},
    {"categoryProperty_two","T@\"NSMutableArray\",&,N"}}
};
```
&emsp;还有一个需要注意到的事实就是 `category` 的名字用来给各种列表以及后面的 `category` 结构体本身命名，而且有 `static` 来修饰，所以在同一个编译单元里我们的 `category` 名不能重复，否则会出现编译错误。

### \_OBJC_$_CATEGORY_NSObject_$_customCategory
&emsp;编译器生成 `_category_t` 本身 `_OBJC_$_CATEGORY_NSObject_$_customCategory` 并用前面生成的实例方法、类方法、属性列表来初始化。还用 `OBJC_CLASS_$_NSObject` 来动态指定 `_OBJC_$_CATEGORY_NSObject_$_customCategory` 所属的类。
```c++
extern "C" __declspec(dllimport) struct _class_t OBJC_CLASS_$_NSObject;

static struct _category_t _OBJC_$_CATEGORY_NSObject_$_customCategory __attribute__ ((used, section ("__DATA,__objc_const"))) = 
{
    "NSObject",
    0, // &OBJC_CLASS_$_NSObject,
    (const struct _method_list_t *)&_OBJC_$_CATEGORY_INSTANCE_METHODS_NSObject_$_customCategory,
    (const struct _method_list_t *)&_OBJC_$_CATEGORY_CLASS_METHODS_NSObject_$_customCategory,
    0,
    (const struct _prop_list_t *)&_OBJC_$_PROP_LIST_NSObject_$_customCategory,
};

// 设置 cls
static void OBJC_CATEGORY_SETUP_$_NSObject_$_customCategory(void ) {
    _OBJC_$_CATEGORY_NSObject_$_customCategory.cls = &OBJC_CLASS_$_NSObject;
}

#pragma section(".objc_inithooks$B", long, read, write)
__declspec(allocate(".objc_inithooks$B")) static void *OBJC_CATEGORY_SETUP[] = {
    (void *)&OBJC_CATEGORY_SETUP_$_NSObject_$_customCategory,
};
```

### L_OBJC_LABEL_CATEGORY_$
&emsp;最后，编译器在 **DATA段下的** `objc_catlist` `section` 里保存了一个长度为 1 的 `struct _category_t *` 数组 `L_OBJC_LABEL_CATEGORY_$`，如果有多个 `category`，会生成对应长度的数组，用于运行期 `category` 的加载，到这里编译器的工作就接近尾声了。
```c++
static struct _category_t *L_OBJC_LABEL_CATEGORY_$ [1] __attribute__((used, section ("__DATA, __objc_catlist,regular,no_dead_strip")))= {
    &_OBJC_$_CATEGORY_NSObject_$_customCategory,
};
```
&emsp;这时我们大概会有一个疑问，这些准备好的的 `_category_t` 数据什么时候附加到类上去呢？或者是存放在内存哪里等着我们去调用它里面的实例函数或类函数呢？**已知分类数据是会全部追加到类本身上去的。** 不是类似 `weak` 机制或者 `associated object` 机制等，再另外准备哈希表存放数据，然后根据对象地址去查询处理数据等这样的模式。

&emsp;下面我们就开始研究分类的数据是如何追加到本类上去的。

## 参考链接
**参考链接:🔗**
+ [结合 category 工作原理分析 OC2.0 中的 runtime](http://www.cocoachina.com/articles/17293)
+ [深入理解Objective-C：Category](https://tech.meituan.com/2015/03/03/diveintocategory.html)
+ [iOS 捋一捋Category加载流程及+load](https://www.jianshu.com/p/fd176e806cf3)
+ [iOS开发之runtime（17）：_dyld_objc_notify_register方法介绍](https://xiaozhuanlan.com/topic/6453890217)
+ [iOS开发之runtime(27): _read_images 浅析](https://xiaozhuanlan.com/topic/1452730698)
