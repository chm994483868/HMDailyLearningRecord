#  深入理解Objective-C中的Class
> **OC -> C/C++ -> 汇编 -> 机器码**

在 `main.m` 中输入如下，然后在终端中使用如下命令，将 `main.m` 中代码编译成 `C/C++` 代码（在当前文件夹下生成 `main.cpp`）：
```
clang -rewrite-objc main.m -o main.cpp // -o main.cpp 可以忽略
```
在 main.cpp 中发现下边的结构体，从 objc4 库的命名习惯可推断应该是 NSObject 的底层实现:
```
// IMPL 应该是 implementation 的缩写
// NSObject_IMPL => NSObject implementation 
struct NSObject_IMPL {
    Class isa;
};
```
在 **usr/include => objc => NSObject.h** 中看到其声明：
```
OBJC_AVAILABLE(10.0, 2.0, 9.0, 1.0, 2.0)
OBJC_ROOT_CLASS
OBJC_EXPORT
@interface NSObject <NSObject> {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wobjc-interface-ivars"
    Class isa  OBJC_ISA_AVAILABILITY;
#pragma clang diagnostic pop
}

// 暂时忽略前缀修饰，以及中间的消除警告的代码
// 以及 NSObject 协议里面的代理方法，以及实例方法和类方法

// 主要看 Class isa; 这个成员变量和上面 NSObject_IMPL 
// 结构体的成员变量如出一辙，这里进一步印证了 NSObject_IMPL 
// 结构体是 NSObject 的底层结构的推断。
@interface NSObject <NSObject> {
    Class isa  OBJC_ISA_AVAILABILITY;
}
```
按住 command 点击 Class 跳转到 **usr/include => objc => objc.h** 
中看到 Class：
```
/// An opaque type that represents an Objective-C class.
typedef struct objc_class *Class;
```
Class 是 `objc_class` 结构体指针，即 isa 实际上是一个指向 `objc_class` 的结构体的指针。
`objc_class` 是 Class 的底层结构。

## HHStaff HHManager

```
// HHStaff.h
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface HHStaff : NSObject {
    NSString *name;
}

- (void)doInstanceStaffWork; // 对象方法
+ (void)doClassStaffWork; // 类方法

@end

NS_ASSUME_NONNULL_END

// HHStaff.m
#import "HHStaff.h"

@implementation HHStaff

- (void)doInstanceStaffWork { // 对象方法
    NSLog(@"✳️✳️✳️ %s", __FUNCTION__);
}

+ (void)doClassStaffWork { // 类方法
    NSLog(@"✳️✳️✳️ %s", __FUNCTION__);
}

@end

// HHManager.h
#import "HHStaff.h"

NS_ASSUME_NONNULL_BEGIN

@interface HHManager : HHStaff {
    NSInteger officeNum;
}

- (void)doInstanceManagerWork;
+ (void)doClassManagerWork;

@end

NS_ASSUME_NONNULL_END

// HHManager.m
#import "HHManager.h"

@implementation HHManager

- (void)doInstanceManagerWork {
    NSLog(@"✳️✳️✳️ %s", __FUNCTION__);
}

+ (void)doClassManagerWork {
    NSLog(@"✳️✳️✳️ %s", __FUNCTION__);
}

@end

// clang -rewrite-objc main.m =》main.cpp

struct NSObject_IMPL {
    Class isa;
};

struct HHStaff_IMPL {
    struct NSObject_IMPL NSObject_IVARS;
    NSString *name;
};

struct HHManager_IMPL {
    struct HHStaff_IMPL HHStaff_IVARS;
    NSInteger officeNum;
};

// 合并继承体系, HHManager_IMPL 实际实现如下：
struct HHManager_IMPL {
    Class isa;
    NSString *name;
    NSInteger officeNum;
};
```
**isa 来自 NSObject，因为大部分类都是直接或间接继承自 NSObject 的，所以可以认为每一个对象都包含了一个 isa 指针。**
## OC 中的 3 种对象关系
+ 实例对象(instance)。实例对象在内存中存储的信息包括：isa 指针 和 类定义中的成员变量对应的值，如 `NSString* name` 成员变量的值是: `@"CHM"`。 
+ 类对象(Class)。类对象中包含的信息，成员变量信息，指的是成员变量的描述信息。如 `HHManager_IMPL` 中有：`Class isa`、`NSString* name`、`NSInteger officeNum` 三个成员变量。
+ 元类对象(meta-class)。元类对象的存储结构与类对象相似，只不过只有 `isa`、`superclass` 和 类方法有值，其他均为空。
```
HHStaff *staffA = [[HHStaff alloc] init];
HHStaff *staffB = [[HHStaff alloc] init];

NSLog(@"♻️♻️♻️ 实例对象: %p - %p", staffA, staffB);

// @protocol NSObject
// - (Class)class OBJC_SWIFT_UNAVAILABLE("use 'type(of: anObject)' instead");
// @end
Class staffClassA = [staffA class]; // 属于 NSObject 协议的代理方法

// runtime.h 中的函数
Class staffClassB = objc_getClass(object_getClassName(staffB));
Class staffClassB2 = object_getClass(staffB);

@interface NSObject <NSObject>
+ (Class)class OBJC_SWIFT_UNAVAILABLE("use 'aClass.self' instead");
@end
Class staffClassC = [HHStaff class]; // 属于 NSObject 类定义中的类方法

NSLog(@"♻️♻️♻️ 类 对象: %p - %p - %p - %p", staffClassA, staffClassB, staffClassB2, staffClassC);

Class staffMetaClassA = object_getClass(staffClassA);
Class staffMetaClassB = object_getClass(staffClassB);

NSLog(@"♻️♻️♻️ 元类对象: %p - %p", staffMetaClassA, staffMetaClassB);

// 打印结果:
♻️♻️♻️ 实例对象: 0x1039addc0 - 0x1039ace80
♻️♻️♻️ 类 对象: 0x100003890 - 0x100003890 - 0x100003890 - 0x100003890
♻️♻️♻️ 元类对象: 0x100003868 - 0x100003868
```
```
/** 
 * Returns the class of an object.
 * 返回对象的类。
 * 
 * @param obj The object you want to inspect.
 * @param obj 你想要获取其 Class 的实例对象。
 *（包括实例对象和类对象，入参是类对象时返回的是它的元类）
 * 
 * @return The class object of which object is an instance, or Nil if object is nil.
 * 入参为 Nil 时返回 nil
 */
OBJC_EXPORT Class _Nullable
object_getClass(id _Nullable obj) 
    OBJC_AVAILABLE(10.5, 2.0, 9.0, 1.0, 2.0);
```
```
/** 
 * Returns the class definition of a specified class.
 * 返回指定类的类定义。
 * 
 * @param name The name of the class to look up.
 * 要查找的类的名称。
 *
 * @return The Class object for the named class, or \c nil
 *  if the class is not registered with the Objective-C runtime.
 * 
 * @note \c objc_getClass is different from \c objc_lookUpClass in that if the class
 *  is not registered, \c objc_getClass calls the class handler callback and then checks
 *  a second time to see whether the class is registered. \c objc_lookUpClass does 
 *  not call the class handler callback.
 * 
 * @warning Earlier implementations of this function (prior to OS X v10.0)
 *  terminate the program if the class does not exist.
 */
OBJC_EXPORT Class _Nullable
objc_getClass(const char * _Nonnull name)
    OBJC_AVAILABLE(10.0, 2.0, 9.0, 1.0, 2.0);
```
由打印结果可知：实例对象可以有多个，类对象和元类对象各自只有一个。

### isa 和 superclass
通过上一小节，我们知道类里边的信息并不是存在一个地方，而是分开存放在实例对象、类对象和元类对象里面。这些对象依靠 isa 和 superclass 指针连系起来。
#### isa
`isa` 用来连系一个类的实例对象、类对象和元类对象。
目前可以看到的是:
|-| isa 类型|
|---|---|
| 实例对象 | Class |
| 类对象 | isa_t（union）|
| 元类对象 | isa_t（union）|
#### superclass
superclass 是用来在继承体系中搜寻父类的。（只有类对象和元类对象有 superclas(struct objc_class *) ）。
+ 对于类对象：子类（HHManager）的类对象的 superclass 指向父类（HHStaff）的类对象，父类的类对象的 superclass 指向它的父类的类对象。
+ 对于元类对象：子类（HHManager）的元类对象的 superclass 指向父类（HHStaff）的元类对象，父类的元类对象的 superclass 指向它的父类的元类对象。

**假装这里有一张继承关系和 isa 、superclass 指向的经典图**

```
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        // insert code here...
        // 创建 HHManager 实例对象
        HHManager *mgr = [[HHManager alloc] init];
        NSLog(@"🕐🕐🕐 %@", mgr);
        NSLog(@"🕐🕐🕐 %p", &mgr);
        [mgr doInstanceStaffWork];
    }
    
    return 0;
}
// 打印：
🕐🕐🕐 <HHManager: 0x1028050a0> // 实例对象位于堆区
🕐🕐🕐 0x7ffeefbff5c0 // 对象指针，可以看出指针是位于栈区的
📢📢📢 -[HHStaff doInstanceStaffWork] self = <HHManager: 0x1028050a0> // 父类函数执行，这里的 self 入参是 HHManager 实例对象
```
对象方法存放在类对象里面，所以首先根据 mgr 的 isa 指针找到它的类对象，然后在类对象的方法列表里边查找这个方法，发现找不到，接着根据类对象的 superclass 指针找到父类的类对象，然后在父类的类对象里边查找该方法，如果还找不到就根据父类的 superclass 指针沿着继承体系继续向上找，直到根类，如果还是找不到就会执行消息转发的流程。
如果是类方法则通过类对象的 isa 指针找到元类对象，然后就依照类似查找对象方法的方式查找类方法，只不过这次是在元类对象的继承体系里边查找。
上边的逻辑省略了一个非常重要的缓存问题，即在每一级查找时，都会先去查找缓存，然后才去查找方法列表。找到之后，也会在缓存里边存一份（即使是在父类的类对象或元类对象里边找到的，也要始终缓存在当前类对象或元类对象里），以便提高查找效率。

#### 验证特例:
Root class(meta) 根元类的 superclass 指向根类，根类的 superclass 指向 nil，根元类的 isa 指向自己。
```
// 1. 这里用 HHStaff 类以调用类方法的方式执行
// 最终执行了基类的类对象里边存储的对象方法。
[HHStaff doInstanceWork];
// 2. 这里用 HHStaff 实例对象以调用实例方法的方式执行
HHStaff *staff = [[HHStaff alloc] init];
[staff doInstanceWork];

// 打印：
📢📢📢 -[NSObject(Extern) doInstanceWork] self = HHStaff
📢📢📢 -[NSObject(Extern) doInstanceWork] self = <HHStaff: 0x100501960>
```
```
OBJC_EXPORT id _Nullable
objc_msgSend(id _Nullable self, SEL _Nonnull op, ...)
    OBJC_AVAILABLE(10.0, 2.0, 9.0, 1.0, 2.0);
```
`[HHStaff doInstanceWork]` 最终执行了基类的类对象里边存储的对象方法，可以这么来理解，OC 的方法调用经编译后都会被转成这样的函数调用:
`objc_msgSend(object, @select(methodName))`, 这里并没有指明是`类方法`还是`对象方法`，也就是不关心是对象方法还是类方法，如果 object 是实例对象，就会去类对象里查找方法，如果 object 是类对象，就会去元类对象里边查找。这里的 C 函数执行只关心 object 类型。

如果在 NSObject 的 Extern 里面实现，如下的函数：
```
- (void)doInstanceWork;
+ (void)doInstanceWork;
```
那么 `[HHStaff doInstanceWork]` 在根元类的方法列表里面就找到了对应的函数实现了。

当只有 `- (void)doInstanceWork;` 时，函数开始执行时，从 HHStaff 的元类的方法列表开始查找，一直沿着 superclass 继承体系向上查找，直到找到 NSObject 类对象，因为当前继承体系下根元类的 superclass 指向 NSObject 类对象。

这里还有个知识点，我们在写 OC 函数时是严格区分实例函数和类函数的，编译以后，其实这里 类对象的方法列表中存放的实例函数和元类方法列表中存放的类函数都是完全一样的。
**我们完全可以把它们当作 C 函数来看待，即使是在继承体系中查找函数时，我们潜意识里面也应该这样认定。**

## Class 的结构
类中的方法、属性、协议等重要信息都存放在 `类对象` 和 `元类对象` 里面，且这两者的结构相同，都是 Class 类型的，而 Class 的结构体实际是 `struct objc_class`，此节我们的目的就是要弄清楚 `struct objc_class` 的结构。

在 `Project Headers => objc-runtime-new.h`  (P1244) 中可看到 `struct objc_class` 的定义，这个结构体定义超级长，差不多有 400 多行：
```
struct objc_class : objc_object {
// Class ISA; // 可增加一个 isa_t isa; 继承自 objc_object
Class superclass; 
cache_t cache; // 1. 方法缓存  // formerly cache pointer and vtable
class_data_bits_t bits;    // class_rw_t * plus custom rr/alloc flags

class_rw_t *data() const { // 2. 超级重要的的 class_rw_t
    return bits.data();
}

// 忽略下面的函数🥺...

}
```
看到 objc_class 继承自 objc_object:
在 `Project Headers => objc-private.h` (P82) 中看到 `struct objc_object` 的定义，由于函数较多这里先看的它成员变量定义:
```
struct objc_object {
private:
    isa_t isa; // 3. ias 私有成员变量

public:

    // ISA() assumes this is NOT a tagged pointer object
    Class ISA(); // 不是一个 tagged pointer 对象

    // rawISA() assumes this is NOT a tagged pointer object or a non pointer ISA
    Class rawISA(); // 返回的可能是一个 tagged pointer 对象

    // getIsa() allows this to be a tagged pointer object
    Class getIsa(); // 返回的是一个 tagged pointer 对象
    
    // 忽略下面的函数🥺...
    
}
```

`struct objc_class` 方法定义中涉及这几个重要的宏，来标识不同情况下该函数是否存在：

**FAST_HAS_DEFAULT_RR**
```
// class or superclass has default retain/release/autorelease/retainCount/
//   _tryRetain/_isDeallocating/retainWeakReference/allowsWeakReference
#define FAST_HAS_DEFAULT_RR     (1UL<<2)
```
**FAST_CACHE_HAS_DEFAULT_AWZ**
```
// AWZ 是 allocWithZone 的缩写
// class or superclass has default alloc/allocWithZone: implementation
// Note this is is stored in the metaclass.
#define FAST_CACHE_HAS_DEFAULT_AWZ    (1<<14)
```
**FAST_CACHE_HAS_DEFAULT_CORE**
```
// class or superclass has default new/self/class/respondsToSelector/isKindOfClass
#define FAST_CACHE_HAS_DEFAULT_CORE   (1<<15)
```
**FAST_CACHE_HAS_CXX_CTOR**
**FAST_CACHE_HAS_CXX_DTOR**
// 是否有 C++ 的构造和析构函数
```
// class or superclass has .cxx_construct/.cxx_destruct implementation
//   FAST_CACHE_HAS_CXX_DTOR is chosen to alias with isa_t::has_cxx_dtor
#define FAST_CACHE_HAS_CXX_CTOR       (1<<1)
#define FAST_CACHE_HAS_CXX_DTOR       (1<<2)
```
**FAST_CACHE_REQUIRES_RAW_ISA**
```
// class's instances requires raw isa
#define FAST_CACHE_REQUIRES_RAW_ISA   (1<<13)
```
**RW_FORBIDS_ASSOCIATED_OBJECTS**
// 不允许类实例关联对象
```
// class does not allow associated objects on its instances
#define RW_FORBIDS_ASSOCIATED_OBJECTS       (1<<20)
```
**RW_INITIALIZING**
```
// class is initializing
#define RW_INITIALIZING       (1<<28)
```

等等...

### struct objc_class 中的 cache_t cache;
位于 `Project Headers => objc-runtime-new.h` (P267)


**参考链接:**
[深入理解 Objective-C ☞ Class](https://www.jianshu.com/p/241e8be676a9?utm_campaign=maleskine&utm_content=note&utm_medium=reader_share&utm_source=weixin)
