# iOS AssociatedObject 底层实现原理

> &emsp;话外音 C++ 的析构函数应该理解为内存释放前的清理工作，而不是内存释放，内存释放是使用的 free 函数，还有 OC 的 dealloc 也是，最终真正的释放内存函数是 free，dealloc 也可以理解为是 free 函数调用前做清理工作的。

## 前言
&emsp;使用 `Category` 为已经存在的类添加方法是我们很熟悉的常规操作，但是如果在 `Category` 中为类添加属性 `@property`，则编译器会立即给我们如下警告:
```c++
Property 'categoryProperty' requires method 'categoryProperty' to be defined - use @dynamic or provide a method implementation in this category.
Property 'categoryProperty' requires method 'setCategoryProperty:' to be defined - use @dynamic or provide a method implementation in this category
```
&emsp;提示我们需要手动为属性添加 `setter` `gettr` 方法或者使用 `@dynamic` 在运行时实现这些方法。

&emsp;**即明确的告诉我们在分类中 `@property` 并不会自动生成实例变量以及存取方法。**

&emsp;不是说好的使用 `@property`，编译器会自动帮我们生成下划线实例变量和对应的 `setter` 和 `getter` 方法吗。此机制只能在类定义中实现，因为在分类中，类的实例变量的布局已经固定，使用 `@property` 已经无法向固定的布局中添加新的实例变量，所以我们需要使用关联对象以及两个方法来模拟构成属性的三个要素。

&emsp;示例代码:
```c++
#import "HMObject.h"

NS_ASSUME_NONNULL_BEGIN

@interface HMObject (category)

// 在分类中添加一个属性
@property (nonatomic, copy) NSString *categoryProperty;

@end

NS_ASSUME_NONNULL_END
```

```c++
#import "HMObject+category.h"
#import <objc/runtime.h> 

@implementation HMObject (category)

- (NSString *)categoryProperty {
    // _cmd 代指当前方法的选择子，即 @selector(categoryProperty)
    return objc_getAssociatedObject(self, _cmd);
}

- (void)setCategoryProperty:(NSString *)categoryProperty {
    objc_setAssociatedObject(self,
                             @selector(categoryProperty),
                             categoryProperty,
                             OBJC_ASSOCIATION_COPY_NONATOMIC);
}

@end
```
&emsp;此时我们可以使用关联对象 `Associated Object` 来手动为 `categoryProperty` 添加存取方法，接下来我们对示例代码一步一步进行分析。

## 在类定义中使用 @property 
&emsp;在类定义中我们使用 `@property` 为类添加属性，如果不使用 `@dynamic` 标识该属性的话，编译器会自动帮我们生成一个名字为下划线加属性名的实例变量和该属性的 `setter` 和 `getter` 方法。我们编写如下代码:
```c++
// .h 中如下书写
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface HMObject : NSObject

@property (nonatomic, copy) NSString *cusProperty;

@end

NS_ASSUME_NONNULL_END

// .m 中什么都不做
#import "HMObject.h"
@implementation HMObject
// @dynamic cusProperty;

@end
```
&emsp;编译器会自动帮我们做如下三件事:

1. 添加实例变量 `_cusProperty`
2. 添加 `setter` 方法 `setCusProperty`
3. 添加 `getter` 方法 `cusProperty`

&emsp;即如下 `HMObject.m` 的代码实现：
```c++
#import "HMObject.h"

@implementation HMObject
// @dynamic cusProperty;
{
    NSString *_cusProperty;
}

- (void)setCusProperty:(NSString *)cusProperty {
    _cusProperty = cusProperty;
}

- (NSString *)cusProperty {
    return _cusProperty;
}

@end
```

### 验证 @property
&emsp;下面我们通过 `LLDB` 进行验证，首先我们把 `HMObject.m` 的代码都注释掉，只留下 `HMObject.h` 中的 `cusProperty` 属性。然后在 `main` 函数中编写如下代码：
```c++
Class cls = NSClassFromString(@"HMObject");
NSLog(@"%@", cls); // ⬅️ 这里打一个断点
```
&emsp;开始验证：

> &emsp;这里我们也可以使用 `runtime` 的 `class_copyPropertyList`、`class_copyMethodList`、`class_copyIvarList` 三个函数来分别获取 `HMObject` 的属性列表、方法列表和成员变量列表来验证编译器为我们自动生成了什么内容，但是这里我们采用一种更为简单的方法，仅通过控制台打印即可验证。

1. 找到 `cls` 的 `bits`：
```c++
(lldb) x/5gx cls
0x1000022e8: 0x00000001000022c0 (isa) 0x00000001003ee140 (superclass)
0x1000022f8: 0x00000001003e84a0 0x0000001c00000000 (cache_t)
0x100002308: 0x0000000101850640 (bits)
```
2. 强制转换 `class_data_bits_t` 指针
```c++
(lldb) p (class_data_bits_t *)0x100002308
(class_data_bits_t *) $1 = 0x0000000100002308
```
3. 取得 `class_rw_t *`
```c++
(lldb) p $1->data()
(class_rw_t *) $2 = 0x0000000101850640
```
4. 取得 `class_ro_t *`
```c++
(lldb) p $2->ro
(const class_ro_t *) $3 = 0x0000000100002128
```
5. 打印 `ro` 内容
```c++
(lldb) p *$3
(const class_ro_t) $4 = {
  flags = 388
  instanceStart = 8
  instanceSize = 16
  reserved = 0
  ivarLayout = 0x0000000100000ee6 "\x01"
  name = 0x0000000100000edd "HMObject" // 类名
  baseMethodList = 0x0000000100002170 // 方法列表
  baseProtocols = 0x0000000000000000 // 遵循协议为空
  ivars = 0x00000001000021c0 // 成员变量
  weakIvarLayout = 0x0000000000000000
  baseProperties = 0x00000001000021e8 // 属性
  _swiftMetadataInitializer_NEVER_USE = {}
}
```
6. 打印 `ivars`
```c++
(lldb) p $4.ivars
(const ivar_list_t *const) $5 = 0x00000001000021c0
(lldb) p *$5
(const ivar_list_t) $6 = {
  entsize_list_tt<ivar_t, ivar_list_t, 0> = {
    entsizeAndFlags = 32
    count = 1 // 有 1 个成员变量
    first = {
      offset = 0x00000001000022b8
      // 看到名字为 _cusProperty 的成员变量
      name = 0x0000000100000ef6 "_cusProperty"
      type = 0x0000000100000f65 "@\"NSString\""
      alignment_raw = 3
      size = 8
    }
  }
}
```
7. 打印 `baseProperties`
```c++
(lldb) p $4.baseProperties
(property_list_t *const) $7 = 0x00000001000021e8
(lldb) p *$7
(property_list_t) $8 = {
  entsize_list_tt<property_t, property_list_t, 0> = {
    entsizeAndFlags = 16
    count = 1
    first = (name = "cusProperty", attributes = "T@\"NSString\",C,N,V_cusProperty")
  }
}
```
&emsp;看到只有一个名字是 `cusProperty` 的属性，属性的 `attributes` 是：`"T@\"NSString\",C,N,V_cusProperty"`

| code | meaning |
| ... | ... |
| T | 类型 |
| C | copy |
| N | nonatomic |
| V | 实例变量 |

&emsp;关于它的详细信息可参考 [《Objective-C Runtime Programming Guide》](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/ObjCRuntimeGuide/Articles/ocrtPropertyIntrospection.html)。

8. 打印 `baseMethodList`
```c++
(lldb) p $4.baseMethodList
(method_list_t *const) $9 = 0x0000000100002170
(lldb) p *$9
(method_list_t) $10 = {
  entsize_list_tt<method_t, method_list_t, 3> = {
    entsizeAndFlags = 26
    count = 3 // 有 3 个 method
    first = {
      // 第一个正是 cusProperty 的 getter 函数
      name = "cusProperty"
      types = 0x0000000100000f79 "@16@0:8"
      imp = 0x0000000100000c30 (KCObjcTest`-[HMObject cusProperty])
    }
  }
}
```
&emsp;看到方法的 `TypeEncoding` 如下:

&emsp;`types = 0x0000000100000f79 "@16@0:8"` 从左向右分别表示的含义是: `@` 表示返回类型是 `OC` 对象，16 表示所有参数总长度，再往后 `@` 表示第一个参数的类型，对应函数调用的 `self` 类型，0 表示从第 0 位开始，分隔号 : 表示第二个参数类型，对应 `SEL`，8 表示从第 8 位开始，因为前面的一个参数 `self` 占 8 个字节。下面开始是自定义参数，因为 `getter` 函数没有自定义函数，所以只有 `self` 和 `SEL` 参数就结束了。对应的函数原型正是 `objc_msgSend` 函数:
```c++
void
objc_msgSend(void /* id self, SEL op, ... */ )
```
9. 打印剩下的两个 `method`
```c++
(lldb) p $10.get(1)
(method_t) $11 = {
  name = "setCusProperty:"
  types = 0x0000000100000f81 "v24@0:8@16"
  imp = 0x0000000100000c60 (KCObjcTest`-[HMObject setCusProperty:])
}
(lldb) p $10.get(2)
(method_t) $12 = {
  name = ".cxx_destruct"
  types = 0x0000000100000f71 "v16@0:8"
  imp = 0x0000000100000c00 (KCObjcTest`-[HMObject .cxx_destruct])
}
```
&emsp;看到一个是 `cusProperty` 的 `setter` 函数，一个是 `C++` 的析构函数。

&emsp;为了做出对比，我们注释掉  `HMObject.h` 中的 `cusProperty` 属性，然后重走上面的流程，可打印出如下信息:
```c++
(lldb) x/5gx cls
0x100002240: 0x0000000100002218 0x00000001003ee140
0x100002250: 0x00000001003e84a0 0x0000001000000000
0x100002260: 0x00000001006696c0
(lldb) p (class_data_bits_t *)0x100002260
(class_data_bits_t *) $1 = 0x0000000100002260
(lldb) p $1->data()
(class_rw_t *) $2 = 0x00000001006696c0
(lldb) p $2->ro
(const class_ro_t *) $3 = 0x0000000100002118
(lldb) p *$3
(const class_ro_t) $4 = {
  flags = 128
  instanceStart = 8
  instanceSize = 8
  reserved = 0
  ivarLayout = 0x0000000000000000
  name = 0x0000000100000f22 "HMObject"
  baseMethodList = 0x0000000000000000
  baseProtocols = 0x0000000000000000
  ivars = 0x0000000000000000
  weakIvarLayout = 0x0000000000000000
  baseProperties = 0x0000000000000000
  _swiftMetadataInitializer_NEVER_USE = {}
}
(lldb) 
```
&emsp;可看到 `ivars`、`baseProperties` 和 `baseMethodList` 都是 `0x0000000000000000`，即编译器没有为 `HMObject` 生成属性、成员变量和函数。至此 `@property` 的作用可得到完整证明。

&emsp;`@property` 能够为我们自动生成实例变量以及存取方法，而这三者构成了属性这个类似于语法糖的概念，为我们提供了更便利的点语法来访问属性：

> &emsp;`self.property` 等价于 `[self property];`
> &emsp;`self.property = value;` 等价于 `[self setProperty:value];`

&emsp;习惯于 `C/C++` 结构体和结构体指针取结构体成员变量时使用 `.` 和 `->`。初见 `OC` 的点语法时有一丝疑问，`self` 明明是一个指针，访问它的成员变量时为什么可以用 `.` 呢？如果按 `C/C++` 的规则，不是应该使用 `self->_property` 吗？

&emsp;这里我们应与 `C/C++` 的点语法做出区别理解，`OC` 中点语法是用来帮助我们便捷访问属性的，在类内部我们可以使用 `_proerty`、`self->_propery` 和 `self.property` 三种方式访问同一个成员变量，区别在于使用 `self.property` 时是通过调用 `property` 的 `setter` 和 `getter` 来读取成员变量，而前两种则是直接读取，因此当我们重写属性的 `setter` 和 `getter` 并在内部做一些自定义操作时，我们一定要记得使用 `self.property` 来访问属性。

##  Associated Object
&emsp;我们使用 `objc_setAssociatedObject` 和 `objc_getAssociatedObject` 来分别模拟属性的存取方法，而使用关联对象模拟实例变量。`runtime.h` 中定义了如下三个与关联对象相关的函数接口:
```c++
/** 
 * Sets an associated value for a given object using a given key and association policy.
 * 使用给定的键和关联策略为给定的对象设置关联的值。
 * 
 * @param object The source object for the association.
 * 关联的源对象
 *
 * @param key The key for the association.
 * 关联的 key
 * @param value The value to associate with the key key for object. 
 * Pass nil to clear an existing association.
 * 与对象的键相关联的值。传递 nil 以清除现有的关联。
 *
 * @param policy The policy for the association. For possible values, see “Associative Object Behaviors.”
 * 关联策略
 * 
 * @see objc_setAssociatedObject
 * @see objc_removeAssociatedObjects
 */
OBJC_EXPORT void
objc_setAssociatedObject(id _Nonnull object, const void * _Nonnull key,
                         id _Nullable value, objc_AssociationPolicy policy)
    OBJC_AVAILABLE(10.6, 3.1, 9.0, 1.0, 2.0);
```

```c++
/** 
 * Returns the value associated with a given object for a given key.
 * 返回与给定键的给定对象关联的值
 * 
 * @param object The source object for the association.
 * 关联的源对象
 * @param key The key for the association.
 * 关联的 key
 * 
 * @return The value associated with the key \e key for \e object.
 * 
 * @see objc_setAssociatedObject
 */
OBJC_EXPORT id _Nullable
objc_getAssociatedObject(id _Nonnull object, const void * _Nonnull key)
    OBJC_AVAILABLE(10.6, 3.1, 9.0, 1.0, 2.0);
```

```c++
/** 
 * Removes all associations for a given object.
 * 删除给定对象的所有关联
 * 
 * @param object An object that maintains associated objects.
 * 
 * @note The main purpose of this function is to make it easy to return an object
 * to a "pristine state”. You should not use this function for general removal of
 * associations from objects, since it also removes associations that other clients
 * may have added to the object. Typically you should use \c objc_setAssociatedObject
 * with a nil value to clear an association.
 *
 * 意指此函数会一下删除对象全部的关联对象，如果我们想要删除指定的关联对象，
 * 应该使用 objc_setAssociatedObject 函数把 value 参数传递 nil 即可。
 *
 * 此功能的主要目的是使对象轻松返回“原始状态”，因此不应从该对象中普遍删除关联，
 * 因为它还会删除其他 clients 可能已添加到该对象的关联。
 * 通常，你应该将 objc_setAssociatedObject 与 nil 一起使用以清除指定关联。
 * 
 * @see objc_setAssociatedObject
 * @see objc_getAssociatedObject
 */
OBJC_EXPORT void
objc_removeAssociatedObjects(id _Nonnull object)
    OBJC_AVAILABLE(10.6, 3.1, 9.0, 1.0, 2.0);
```

### const void *key
&emsp;存取函数中的参数 `key` 我们都使用了 `@selector(categoryProperty)`，其实也可以使用静态指针 `static void *` 类型的参数来代替，不过这里强烈建议使用 `@selector(categoryProperty)` 作为 `key` 传入，因为这种方法省略了声明参数的代码，并且能很好地保证 `key` 的唯一性。
 
### objc_AssociationPolicy policy
&emsp;`policy` 代表关联策略:
```c++
/**
 * Policies related to associative references.
 * These are options to objc_setAssociatedObject()
 */
typedef OBJC_ENUM(uintptr_t, objc_AssociationPolicy) {
    /**< Specifies a weak reference to the associated object. */
    OBJC_ASSOCIATION_ASSIGN = 0,    
    
    /**< Specifies a strong reference to the associated object. 
    *   The association is not made atomically. */
    OBJC_ASSOCIATION_RETAIN_NONATOMIC = 1, 
    
    /**< Specifies that the associated object is copied. 
    *   The association is not made atomically. */
    OBJC_ASSOCIATION_COPY_NONATOMIC = 3,
    
    /**< Specifies a strong reference to the associated object.
    *   The association is made atomically. */
    OBJC_ASSOCIATION_RETAIN = 01401,
    
    /**< Specifies that the associated object is copied.
    *   The association is made atomically. */
    OBJC_ASSOCIATION_COPY = 01403          
};
```
&emsp;注释已经解释的很清楚了，即不同的策略对应不同的修饰符:

| objc_AssociationPolicy | 修饰符 |
| ... | ... |
| OBJC_ASSOCIATION_ASSIGN | assign |
| OBJC_ASSOCIATION_RETAIN_NONATOMIC | nonatomic、strong |
| OBJC_ASSOCIATION_COPY_NONATOMIC | nonatomic、copy |
| OBJC_ASSOCIATION_RETAIN | atomic, strong |
| OBJC_ASSOCIATION_COPY | atomic, copy |

&emsp;`objc-references.mm` 文件包含了所有的核心操作，首先来分析相关的数据结构。

### ObjcAssociation
&emsp;`associated object` 机制中用于保存 **关联策略** 和 **关联值**。
```c++
class ObjcAssociation {
    // typedef unsigned long uintptr_t;
    uintptr_t _policy; // 关联策略
    id _value; // 关联值
public:
    // 构造函数，初始化列表初始化 policy 和 value
    ObjcAssociation(uintptr_t policy, id value) : _policy(policy), _value(value) {}
    // 构造函数，初始化列表，policy 初始化为 0, value 初始化为 nil 
    ObjcAssociation() : _policy(0), _value(nil) {}
    
    // 复制构造函数采用默认
    ObjcAssociation(const ObjcAssociation &other) = default;
    // 赋值操作符采用默认
    ObjcAssociation &operator=(const ObjcAssociation &other) = default;
    
    // 和 other 交换 policy 和 value
    ObjcAssociation(ObjcAssociation &&other) : ObjcAssociation() {
        swap(other);
    }
    inline void swap(ObjcAssociation &other) {
        std::swap(_policy, other._policy);
        std::swap(_value, other._value);
    }

    // 内联函数获取 _policy
    inline uintptr_t policy() const { return _policy; }
    // 内联函数获取 _value
    inline id value() const { return _value; }
    
    // 在 SETTER 时使用根据关联策略判断是否需要持有 value
    inline void acquireValue() {
        if (_value) {
            switch (_policy & 0xFF) {
            case OBJC_ASSOCIATION_SETTER_RETAIN:
                // retain
                _value = objc_retain(_value);
                break;
            case OBJC_ASSOCIATION_SETTER_COPY:
                // copy
                _value = ((id(*)(id, SEL))objc_msgSend)(_value, @selector(copy));
                break;
            }
        }
    }
    
    // 在 SETTER 时使用：与上面的 acquireValue 函数对应，释放旧值 value 
    inline void releaseHeldValue() {
        if (_value && (_policy & OBJC_ASSOCIATION_SETTER_RETAIN)) {
            // release 减少引用计数
            objc_release(_value);
        }
    }

    // 在 GETTER 时使用：根据关联策略判断是否对关联值进行 retain 操作
    inline void retainReturnedValue() {
        if (_value && (_policy & OBJC_ASSOCIATION_GETTER_RETAIN)) {
            objc_retain(_value);
        }
    }
    
    // 在 GETTER 时使用：判断是否需要放进自动释放池
    inline id autoreleaseReturnedValue() {
        if (slowpath(_value && (_policy & OBJC_ASSOCIATION_GETTER_AUTORELEASE))) {
            return objc_autorelease(_value);
        }
        return _value;
    }
};
```

### ObjectAssociationMap
```c++
typedef DenseMap<const void *, ObjcAssociation> ObjectAssociationMap;
```
&emsp;`DenseMap` 这里不再展开，把 `ObjectAssociationMap` 理解为一个 `key` 是 `const void *`，`value` 是 `ObjcAssociation` 的哈希表即可。

### AssociationsHashMap
```c++
typedef DenseMap<DisguisedPtr<objc_object>, ObjectAssociationMap> AssociationsHashMap;
```
&emsp;同上，把 `AssociationsHashMap` 理解为一个 `key` 是 `DisguisedPtr<objc_object>` `value` 是 `ObjectAssociationMap` 的哈希表即可。`DisguisedPtr<objc_object>` 可理解为把 `objc_object` 地址伪装为一个整数。可参考:[iOS weak 底层实现原理(一)：SideTable|s、weak_table_t、weak_entry_t 等数据结构](https://juejin.im/post/6865468675940417550)

### AssociationsManager
&emsp;`AssociationsManager` 的类定义不复杂，从数据结构角度来看的话它是作为一个 `key` 是 `DisguisedPtr<objc_object>` `value` 是 `ObjectAssociationMap` 的哈希表来用的，这么看它好像和上面的 `AssociationsHashMap` 有些重合，其实它内部正是存储了一个局部静态的 `AssociationsHashMap` 用来存储程序中所有的关联对象。

#### AssociationsManagerLock
```c++
spinlock_t AssociationsManagerLock;
```
&emsp;一个全局的自旋锁（互斥锁），保证 `AssociationsManager` 中对 `AssociationsHashMap` 操作的线程安全。
```c++
// class AssociationsManager manages a lock / hash table singleton pair.
// Allocating an instance acquires the lock

class AssociationsManager {
    // Storage 模版类名
    using Storage = ExplicitInitDenseMap<DisguisedPtr<objc_object>, ObjectAssociationMap>;
    // 静态变量 _mapStoreage，用于存储 AssociationsHashMap 数据
    static Storage _mapStorage;
    
public:
    // 构造函数 获取全局的 AssociationsManagerLock 加锁
    AssociationsManager()   { AssociationsManagerLock.lock(); }
    // 析构函数 AssociationsManagerLock 解锁
    ~AssociationsManager()  { AssociationsManagerLock.unlock(); }

    // 返回内部的保存的 AssociationsHashMap，
    AssociationsHashMap &get() {
        return _mapStorage.get();
    }
    
    // init 初始化函数实现只是 调用 Storage 的 init 函数
    static void init() {
        _mapStorage.init();
    }
};

// 其实这里有点想不明白，明明 AssociationsManager 已经定义了公开函数 get 获取内部 _mapStorage 的数据，

// 为什么这里在类定义外面还写了这句代码 ？（这是又把 _mapStorage 声明为全局变量吗？）
AssociationsManager::Storage AssociationsManager::_mapStorage;
```
&emsp;管理 `AssociationsHashMap` 静态变量。

&emsp;总结:

1. 通过 `AssociationsManager` 的 `get` 函数取得一个全局唯一 `AssociationsHashMap`。
2. 根据我们的原始对象的 `DisguisedPtr<objc_object>` 从 `AssociationsHashMap` 取得 `ObjectAssociationMap`。
3. 根据我们指定的关联 `key`(`const void *key`) 从 `ObjectAssociationMap` 取得 `ObjcAssociation`。
4. `ObjcAssociation` 的两个成员变量，保存我们的关联策略 `_policy` 和关联值 `_value`。

示例图:
![associated object相关数据结构](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/cc5e47b5525c4739994bd95bcc78fc43~tplv-k3u1fbpfcp-zoom-1.image)

### objc_setAssociatedObject
```c++
void
objc_setAssociatedObject(id object, const void *key, id value, objc_AssociationPolicy policy)
{
    SetAssocHook.get()(object, key, value, policy);
}
```
&emsp;`SetAssocHook`
```c++
static ChainedHookFunction<objc_hook_setAssociatedObject> SetAssocHook{_base_objc_setAssociatedObject};
```
&emsp;`_base_objc_setAssociatedObject`
```c++
static void
_base_objc_setAssociatedObject(id object, const void *key, id value, objc_AssociationPolicy policy)
{
  _object_set_associative_reference(object, key, value, policy);
}
```
&emsp;`forbidsAssociatedObjects`（表示是否允许某个类的实例对象关联对象）
```c++
// class does not allow associated objects on its instances
#define RW_FORBIDS_ASSOCIATED_OBJECTS       (1<<20)

bool forbidsAssociatedObjects() {
    return (data()->flags & RW_FORBIDS_ASSOCIATED_OBJECTS);
}
```
&emsp;`try_emplace`
```c++
// Inserts key,value pair into the map if the key isn't already in the map.
// 如果 key value 键值对在 map 中不存在则把它们插入 map

// The value is constructed in-place if the key is not in the map,
// otherwise it is not moved.
template <typename... Ts>
std::pair<iterator, bool> try_emplace(const KeyT &Key, Ts &&... Args) {
  BucketT *TheBucket;
  // 已存在
  if (LookupBucketFor(Key, TheBucket))
    return std::make_pair(
             makeIterator(TheBucket, getBucketsEnd(), true),
             false); // Already in map.

  // Otherwise, insert the new element.
  // 不存在，则插入新元素
  TheBucket = InsertIntoBucket(TheBucket, Key, std::forward<Ts>(Args)...);
  return std::make_pair(
           makeIterator(TheBucket, getBucketsEnd(), true),
           true);
}
```
&emsp;`setHasAssociatedObjects` 设置对象的 `uintptr_t has_assoc : 1;` 位，标记该对象有关联对象，该对象 `dealloc` 时要进行清理工作。
```c++
inline void
objc_object::setHasAssociatedObjects()
{
    if (isTaggedPointer()) return;

 retry:
    isa_t oldisa = LoadExclusive(&isa.bits);
    isa_t newisa = oldisa;
    
    // 如果已经被标记为有关联对象或者对象的 isa 是原始指针则直接返回就可以了。（LoadExclusive 和 ClearExclusive 是配对使用的，可以理解为加锁和解锁。）
    if (!newisa.nonpointer  ||  newisa.has_assoc) {
        ClearExclusive(&isa.bits);
        return;
    }
    
    // has_assoc 为置为 true，表示该对象存在关联对象。
    newisa.has_assoc = true;
    
    // 保证原子赋值的成功。
    if (!StoreExclusive(&isa.bits, oldisa.bits, newisa.bits)) goto retry;
}
```
&emsp;`_object_set_associative_reference`
```c++
void
_object_set_associative_reference(id object, const void *key, id value, uintptr_t policy)
{
    // This code used to work when nil was passed for object and key. Some code
    // probably relies on that to not crash. Check and handle it explicitly.
    // rdar://problem/44094390
    if (!object && !value) return; // 判空对象和关联值都为 nil 则 return

    // 判断该类是否允许关联对象
    if (object->getIsa()->forbidsAssociatedObjects())
        _objc_fatal("objc_setAssociatedObject called on instance (%p) of class %s which does not allow associated objects", object, object_getClassName(object));

    // 伪装 object 指针为 disguised
    DisguisedPtr<objc_object> disguised{(objc_object *)object};
    // 根据入参创建一个 association (关联策略和关联值)
    ObjcAssociation association{policy, value};

    // retain the new value (if any) outside the lock.
    // 在加锁之前根据关联策略判断是否 retain/copy 入参 value 
    association.acquireValue();

    {
        // 创建 mananger 临时变量
        // 这里还有一步连带操作
        // 在其构造函数中 AssociationsManagerLock.lock() 加锁
        AssociationsManager manager;
        // 取得全局的 AssociationsHashMap
        AssociationsHashMap &associations(manager.get());

        if (value) {
            // 这里 DenseMap 对我们而言是一个黑盒，这里只要看 try_emplace 函数
            
            // 在全局 AssociationsHashMap 中尝试插入 <DisguisedPtr<objc_object>, ObjectAssociationMap> 
            // 返回值类型是 std::pair<iterator, bool>
            auto refs_result = associations.try_emplace(disguised, ObjectAssociationMap{});
            // 如果新插入成功
            if (refs_result.second) {
                /* it's the first association we make */
                // 第一次建立 association
                // 设置 uintptr_t has_assoc : 1; 位，标记该对象存在关联对象 
                object->setHasAssociatedObjects();
            }

            /* establish or replace the association */
            // 重建或者替换 association
            auto &refs = refs_result.first->second;
            
            auto result = refs.try_emplace(key, std::move(association));
            if (!result.second) {
                // 替换
                // 如果之前有旧值的话把旧值的成员变量交换到 association
                // 然后在 函数执行结束时把旧值根据对应的关联策略判断执行 release
                association.swap(result.first->second);
            }
        } else {
            // value 为 nil 的情况，表示要把之前的关联对象置为 nil
            // 也可理解为移除指定的关联对象
            auto refs_it = associations.find(disguised);
            if (refs_it != associations.end()) {
                auto &refs = refs_it->second;
                auto it = refs.find(key);
                if (it != refs.end()) {
                    association.swap(it->second);
                    // 清除指定的关联对象
                    refs.erase(it);
                    // 如果当前 object 的关联对象为空了，则同时从全局的 AssociationsHashMap
                    // 中移除该对象
                    if (refs.size() == 0) {
                        associations.erase(refs_it);
                    }
                }
            }
        }
        
        // 析构 mananger 临时变量
        // 这里还有一步连带操作
        // 在其析构函数中 AssociationsManagerLock.unlock() 解锁
    }

    // release the old value (outside of the lock).
    // 开始时 retain 的是新入参的 value, 这里释放的是旧值，association 内部的 value 已经被替换了
    association.releaseHeldValue();
}
```
&emsp;函数执行过程中有两种情况:
+ `value != nil` 新增/更新关联对象的值
+ `value == nil` 删除一个关联对象

函数流程图：
![_object_set_associative_reference流程图](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/1c4350153140465b86d3ad44d1f917f2~tplv-k3u1fbpfcp-zoom-1.image)

&emsp;如果看通了上面的 `_object_set_associative_reference` 则看  `_object_get_associative_reference` 是很容易看懂的。

### objc_getAssociatedObject
```c++
id
objc_getAssociatedObject(id object, const void *key)
{
    return _object_get_associative_reference(object, key);
}
```
`_object_get_associative_reference`
```c++
id
_object_get_associative_reference(id object, const void *key)
{
    // 局部变量
    ObjcAssociation association{};

    {
        // 加锁
        AssociationsManager manager;
        // 取得全局唯一的 AssociationsHashMap
        AssociationsHashMap &associations(manager.get());
        
        // 从全局的 AssociationsHashMap 中取得对象对应的 ObjectAssociationMap
        AssociationsHashMap::iterator i = associations.find((objc_object *)object);
        if (i != associations.end()) {
            // 如果存在
            ObjectAssociationMap &refs = i->second;
            // 从 ObjectAssocationMap 中取得 key 对应的 ObjcAssociation 
            ObjectAssociationMap::iterator j = refs.find(key);
            if (j != refs.end()) {
                // 如果存在
                association = j->second;
                // 根据关联策略判断是否需要对 _value 执行 retain 操作
                association.retainReturnedValue();
            }
        }
        
        // 解锁
    }
    // 返回 _value 并根据关联策略判断是否需要放入自动释放池
    return association.autoreleaseReturnedValue();
}
```
### objc_removeAssociatedObjects
&emsp;`hasAssociatedObjects`
```c++
inline bool
objc_object::hasAssociatedObjects()
{
    if (isTaggedPointer()) return true;
    if (isa.nonpointer) return isa.has_assoc;
    return true;
}
```
&emsp;`objc_removeAssociatedObjects`
```c++
void objc_removeAssociatedObjects(id object) 
{
    // 对象不为空，且 has_assoc 标记为 true，表示该对象有关联对象
    if (object && object->hasAssociatedObjects()) {
        _object_remove_assocations(object);
    }
}
```
&emsp;`_object_remove_assocations`
```c++
// Unlike setting/getting an associated reference, 
// this function is performance sensitive because
// of raw isa objects (such as OS Objects) that can't
// track whether they have associated objects.

// 与 setting/getting 关联引用不同，此函数对性能敏感，
// 因为原始的 isa 对象（例如 OS 对象）无法跟踪它们是否具有关联的对象。
void
_object_remove_assocations(id object)
{
    // 对象对应的 ObjectAssociationMap
    ObjectAssociationMap refs{};

    {
        // 加锁
        AssociationsManager manager;
        // 取得全局的 AssociationsHashMap
        AssociationsHashMap &associations(manager.get());
        
        // 取得对象的对应 ObjectAssociationMap，里面包含所有的 (key, ObjcAssociation)
        AssociationsHashMap::iterator i = associations.find((objc_object *)object);
        if (i != associations.end()) {
            // 把 i->second 的内容都转入 refs 对象中
            refs.swap(i->second);
            // 从全局 AssociationsHashMap 移除对象的 ObjectAssociationMap
            associations.erase(i);
        }
        
        // 解锁
    }

    // release everything (outside of the lock).
    // 遍历对象的 ObjectAssociationMap 中的 (key, ObjcAssociation)
    // 对 ObjcAssociation 的 _value 根据 _policy 进行释放
    for (auto &i: refs) {
        i.second.releaseHeldValue();
    }
}
```

## 关联对象的本质
&emsp;在分类中到底能否实现属性？首先要知道属性是什么，属性的概念决定了这个问题的答案。

+ 如果把属性理解为通过方法访问的实例变量，那这个问题的答案就是不能，因为分类不能为类增加额外的实例变量。
+ 如果属性只是一个存取方法以及存储值的容器的集合，那么分类可以实现属性。

&emsp;分类中对属性的实现其实只是实现了一个看起来像属性的接口而已。

## 参考链接
**参考链接:🔗**
+ [关联对象 AssociatedObject 完全解析](https://draveness.me/ao/)
+ [iOS_@property 属性的本质是什么?](https://www.jianshu.com/p/7ddefcfba3cb)
+ [C++11的6种内存序总结__std::memory_order_acquire_等](https://blog.csdn.net/mw_nice/article/details/84861651)
+ [【iOS】Category的成员变量](https://www.jianshu.com/p/0aada6304090)
