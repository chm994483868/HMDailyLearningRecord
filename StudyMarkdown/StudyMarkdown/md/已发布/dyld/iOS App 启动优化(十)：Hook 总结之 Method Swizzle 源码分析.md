# iOS App 启动优化(十)：Hook 总结之 Method Swizzle 源码分析

&emsp;Objective-C 中的 Hook 又被称作 Method Swizzling，这是动态语言大都具有的特性。在 Objective-C 中经常会把 Hook 的逻辑写在 `+load` 方法中，这是利用它调用时机较提前的性质。

## Method Swizzle 简述

&emsp;利用 OC 的 Runtime 特性，动态改变 `SEL`（选择子）和 `IMP`（方法实现）的对应关系，达到 OC 方法调用流程改变的目的。主要用于 OC 方法，常用的 API 有：

+ `method_exchangeImplementations` 交换函数 imp
+ `class_replaceMethod` 替换方法
+ `method_getImplementation` 与 `method_setImplementation` 直接 get/set imp

## 从源码梳理 runtime 函数

&emsp;下面我们再次回到 objc4 源码中，对 Method Swizzle 涉及到的函数一探究竟。

### Method 结构回顾

```c++
typedef struct method_t *Method;
using MethodListIMP = IMP;

struct method_t {
    SEL name;
    const char *types;
    MethodListIMP imp;

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

&emsp;`method_t` 便是我们的 OC 函数对应的数据结构，`SEL name` 是函数的选择子（或者直接理解为函数名），选择子中是不包含参数类型的。`const char *types` 是完整的函数类型描述，对函数的返回值、所有参数（包含参数类型、长度、顺序）进行描述。`MethodListIMP imp` 是方法实现（或者理解为函数地址）。

## method_exchangeImplementations

&emsp;下面是 `method_exchangeImplementations` 函数的一个标准用法，交换两个函数的 `imp`，即交换两个函数的实现地址。

```c++
/** 
 * Exchanges the implementations of two methods.
 * 
 * @param m1 Method to exchange with second method.
 * @param m2 Method to exchange with first method.
 * 
 * @note This is an atomic version of the following:
 *  \code 
 *  IMP imp1 = method_getImplementation(m1);
 *  IMP imp2 = method_getImplementation(m2);
 *  method_setImplementation(m1, imp2);
 *  method_setImplementation(m2, imp1);
 *  \endcode
 */
OBJC_EXPORT void
method_exchangeImplementations(Method _Nonnull m1, Method _Nonnull m2) 
    OBJC_AVAILABLE(10.5, 2.0, 9.0, 1.0, 2.0);
```

&emsp;`method_exchangeImplementations` 函数的声明，交换两个 methods 的实现。`method_exchangeImplementations` 函数实现的工程和上面的示例代码类似，但是 `method_exchangeImplementations` 具有原子性，是线程安全的。下面我们看一下它的函数定义：

```c++
void method_exchangeImplementations(Method m1, Method m2)
{
    // 如果 m1 或者 m2 有一个不存在则直接返回
    if (!m1  ||  !m2) return;
    
    // 加锁（上面所说的原子性）
    mutex_locker_t lock(runtimeLock);
    
    // 交换 m1 和 m2 的 imp
    IMP m1_imp = m1->imp;
    m1->imp = m2->imp;
    m2->imp = m1_imp;


    // RR/AWZ updates are slow because class is unknown
    // Cache updates are slow because class is unknown
    // fixme build list of classes whose Methods are known externally?

    // 清空当前所有类的方法缓存（注意：是所有类的方法缓存）
    flushCaches(nil);

    // 当方法更改其 IMP 时更新自定义 RR 和 AWZ（retain、release、allocWithZone 函数）
    adjustCustomFlagsForMethodChange(nil, m1);
    adjustCustomFlagsForMethodChange(nil, m2);
}
```

&emsp;对两个 Method 的 `imp` 交换很简单，就是取值然后交换。`flushCaches(nil)` 则会清空当前所有类的方法缓存，这个操作还挺重的，所以这里也可理解为一个方法交换应尽早执行的原因。  

## class_replaceMethod

```c++
/** 
 * Replaces the implementation of a method for a given class.
 * 
 * @param cls The class you want to modify.
 * @param name A selector that identifies the method whose implementation you want to replace.
 * @param imp The new implementation for the method identified by name for the class identified by cls.
 * @param types An array of characters that describe the types of the arguments to the method. 
 *  Since the function must take at least two arguments—self and _cmd, the second and third characters
 *  must be “@:” (the first character is the return type).
 * 
 * @return The previous implementation of the method identified by \e name for the class identified by \e cls.
 * 
 * @note This function behaves in two different ways:
 *  - If the method identified by \e name does not yet exist, it is added as if \c class_addMethod were called. 
 *    The type encoding specified by \e types is used as given.
 *  - If the method identified by \e name does exist, its \c IMP is replaced as if \c method_setImplementation were called.
 *    The type encoding specified by \e types is ignored.
 */
OBJC_EXPORT IMP _Nullable
class_replaceMethod(Class _Nullable cls, SEL _Nonnull name, IMP _Nonnull imp, 
                    const char * _Nullable types) 
    OBJC_AVAILABLE(10.5, 2.0, 9.0, 1.0, 2.0);
```

&emsp;










## 参考链接
**参考链接:🔗**
+ [巧用符号表 - 探求 fishhook 原理（一）](https://www.desgard.com/iOS-Source-Probe/C/fishhook/%E5%B7%A7%E7%94%A8%E7%AC%A6%E5%8F%B7%E8%A1%A8%20-%20%E6%8E%A2%E6%B1%82%20fishhook%20%E5%8E%9F%E7%90%86%EF%BC%88%E4%B8%80%EF%BC%89.html)
+ [一文读懂fishhook原理](https://juejin.cn/post/6857699952563978247)
+ [fishhook的实现原理浅析](https://juejin.cn/post/6844903789783154702)
+ [fishhook使用场景&源码分析](https://juejin.cn/post/6844903793008574477)
+ [从fishhook第三方库学到的知识【有所得】](https://juejin.cn/post/6915680287049383944)
+ [iOS程序员的自我修养-fishhook原理（五）](https://juejin.cn/post/6844903926051897358)
+ [iOS 逆向 - Hook / fishHook 原理与符号表](https://juejin.cn/post/6844903992904908814)
+ [iOS逆向 RSA理论](https://juejin.cn/post/6844903989666906125)
+ [iOS逆向 HOOK原理之fishhook](https://juejin.cn/post/6845166890772332552)
+ [LXDZombieSniffer](https://github.com/sindrilin/LXDZombieSniffer)
+ [SDMagicHook](https://github.com/cloverapp1/SDMagicHook)
