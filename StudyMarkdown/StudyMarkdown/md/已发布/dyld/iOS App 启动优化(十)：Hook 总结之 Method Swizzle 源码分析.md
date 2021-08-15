# iOS App 启动优化(十)：Hook 总结之 Method Swizzle 源码分析

&emsp;Objective-C 中的 Hook 又被称作 Method Swizzling，这是动态语言大都具有的特性。在 Objective-C 中经常会把 Hook 的逻辑写在 `+load` 方法中，这是利用它的调用时机较提前的性质。

## Method Swizzle 简述

&emsp;利用 OC 的 Runtime 特性，动态改变 `SEL`（选择子）和 `IMP`（方法实现）的对应关系，达到 OC 方法调用流程改变的目的。主要用于 OC 方法，常用的 API 有：

+ `method_exchangeImplementations` 交换函数的 imp。
+ `class_replaceMethod` 替换函数的 imp。
+ `method_getImplementation` 与 `method_setImplementation` 直接 get/set 函数的 imp。

## 从源码梳理 runtime 函数

&emsp;下面我们再次回到 objc4 源码中，对 Method Swizzle 涉及到的函数一探究竟。

### Method 结构回顾

&emsp;首先我们再看一下 OC 函数的数据结构 `struct Method`：

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

&emsp;`struct method_t` 便是我们的 OC 函数对应的数据结构，`SEL name` 是函数的选择子（或者直接理解为函数名），选择子中是不包含参数类型以及返回值类型的。`const char *types` 是完整的函数类型描述，对函数的返回值、所有参数（包含参数类型、长度、顺序）进行描述。`MethodListIMP imp` 是方法实现（或者理解为函数的地址）。

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
 
 * ⬇️⬇️⬇️
 *  \code 
 *  IMP imp1 = method_getImplementation(m1);
 *  IMP imp2 = method_getImplementation(m2);
 *  method_setImplementation(m1, imp2);
 *  method_setImplementation(m2, imp1);
 *  \endcode
 * ⬆️⬆️⬆️
 
 */
OBJC_EXPORT void
method_exchangeImplementations(Method _Nonnull m1, Method _Nonnull m2) 
    OBJC_AVAILABLE(10.5, 2.0, 9.0, 1.0, 2.0);
```

&emsp;`method_exchangeImplementations` 函数的声明，交换两个函数的实现。`method_exchangeImplementations` 函数实现的功能和上面的示例代码中 `method_getImplementation/method_setImplementation` 的调用类似，但是 `method_exchangeImplementations` 具有原子性，是线程安全的。下面我们看一下它的函数定义：

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

    // 清空当前所有类的方法缓存（注意：是所有类的方法缓存，并不仅是 m1 或者 m2 所属的类）
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

&emsp;`class_replaceMethod` 函数用来替换给定类的方法的实现。`cls` 参数是要修改的类，`name` 参数是一个选择子，用于标识要替换其实现的方法，`types` 参数是描述方法的返回值以及参数类型的字符数组。

&emsp;由于 OC 函数必须至少接收两个参数 `self` 和 `_cmd`（即两个隐式参数），因此第二个和第三个字符必须是 `@:`（第一个字符是返回值类型）。此函数有一个 `IMP` 类型的返回值，表示的是原函数的实现，

> @note: 此函数以两种不同的方式运行：
> + 如果 `cls` 类中 `name` 标识的函数尚不存在，则添加它，就像调用了 `class_addMethod` 一样，新增函数的类型编码按给定的 `types` 使用。
> + 如果 `cls` 类中 `name` 标识的函数确实存在，则其 `IMP` 将被替换，就像调用了 `method_setImplementation` 一样。由 `types` 指定的类型编码被忽略。

&emsp;下面我们看一下 `class_replaceMethod` 函数的定义：

```c++
IMP 
class_replaceMethod(Class cls, SEL name, IMP imp, const char *types)
{
    if (!cls) return nil;
    
    // 加锁
    mutex_locker_t lock(runtimeLock);
    
    // 调用 addMethod 添加方法
    return addMethod(cls, name, imp, types ?: "", YES);
}
```

&emsp;下面我们看一下 `addMethod` 函数的定义：

```c++
/**********************************************************************
* addMethod
* fixme
* Locking: runtimeLock must be held by the caller
* Locking: runtimeLock 必须由调用者持有，即调用 addMethod 函数的前一个调用者必须加锁，也正如上面 class_replaceMethod 函数内部已经对 runtimeLock 加锁，
* 然后再调用的 addMethod 函数，且 runtimeLock 是一个全局锁，所以在其它函数中依然可以对其正常解锁。
**********************************************************************/
static IMP 
addMethod(Class cls, SEL name, IMP imp, const char *types, bool replace)
{
    // 前面 class_replaceMethod 函数调用中 replace 参数传递的为 YES
    
    // 这个 IMP 类型的 result 临时变量用于记录原始的函数实现，作为 addMethod 的函数返回值 
    IMP result = nil;
    
    // 断言：前一个调用是否已经对 runtimeLock 进行加锁
    runtimeLock.assertLocked();
    
    // 检查 cls 是否是一个已知类，根据所有已知类的列表检查给定的类。如果类未知，则会触发一个 _objc_fatal：Attempt to use unknown class。
    // 如果 runtime 知道该类（位于共享缓存内、加载图像的数据段内，或已使用 obj_allocateClassPair 分配），则返回 true。此查找的结果会缓存在类中的 "witness"(cls->data()->witness) 值中，该值在快速路径中方便检查。
    checkIsKnownClass(cls);
    
    // 断言：types 参数不能为 nil，在前面的调用中可看到对 types 参数的处理：types ?: "" 即默认传递："" 空字符串 
    ASSERT(types);
    
    // 断言： 
    ASSERT(cls->isRealized());

    method_t *m;
    if ((m = getMethodNoSuper_nolock(cls, name))) {
        // already exists
        if (!replace) {
            result = m->imp;
        } else {
            result = _method_setImplementation(cls, m, imp);
        }
    } else {
        auto rwe = cls->data()->extAllocIfNeeded();

        // fixme optimize
        method_list_t *newlist;
        newlist = (method_list_t *)calloc(sizeof(*newlist), 1);
        newlist->entsizeAndFlags = 
            (uint32_t)sizeof(method_t) | fixed_up_method_list;
        newlist->count = 1;
        newlist->first.name = name;
        newlist->first.types = strdupIfMutable(types);
        newlist->first.imp = imp;

        prepareMethodLists(cls, &newlist, 1, NO, NO);
        rwe->methods.attachLists(&newlist, 1);
        flushCaches(cls);

        result = nil;
    }

    return result;
}
```





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

+ [iOS逆向之Hopper进阶](https://www.jianshu.com/p/384dc5bc1cb4)
+ [十 iOS逆向- hopper disassembler](https://www.jianshu.com/p/20077ceb2f75?utm_campaign=maleskine&utm_content=note&utm_medium=seo_notes&utm_source=recommendation)
+ [Objective-C Method Swizzling](http://yulingtianxia.com/blog/2017/04/17/Objective-C-Method-Swizzling/)
+ [iOS 界的毒瘤：Method Swizzle](https://juejin.cn/post/6844903517979672590)
+ [iOS微信内存监控](https://wetest.qq.com/lab/view/367.html)
+ [通过修改GOT表，hook glibc函数](https://zhougy0717.github.io/2020/01/05/通过修改GOT表，hook_glibc函数/)
+ [dyld详解](https://www.dllhook.com/post/238.html)
+ [iOS hook C++ 尝试](https://juejin.cn/post/6844903966434656264)
+ [RSSwizzle](https://github.com/rabovik/RSSwizzle/)
+ [jrswizzle](https://github.com/rentzsch/jrswizzle)
+ [syx______](https://juejin.cn/user/2981531267132584)
+ [理解函数调用栈](https://juejin.cn/post/6917823435905662983)
+ [LLVM & Clang 入门](https://juejin.cn/post/6844903748435705864)
+ [AceDong](https://juejin.cn/user/3438928101906584)
+ [分析字节跳动解决OOM的在线Memory Graph技术实现](https://juejin.cn/post/6895583288451465230)
+ [iOS 逆向 - 重签应用调试与代码修改 (Hook)](https://juejin.cn/post/6844903978229039117#heading-14)
+ [iOS 逆向 - Hook / fishHook 原理与符号表](https://juejin.cn/post/6844903992904908814)
+ [iOS 页面渲染 - UIView & CALayer](https://mp.weixin.qq.com/s/ElGEsJoh3Y1-BWlvz1yJ9w)
+ [浅析快手iOS启动优化方式——动态库懒加载](https://mp.weixin.qq.com/s/gNc3uK5ILbXsO8jB1O-jnQ)
+ [dyld背后的故事&源码分析](https://juejin.cn/post/6844903782833192968)
+ [非越狱下 iOS代码注入&HOOK微信登录](https://juejin.cn/post/6844903775518359560)
+ [fishhook使用场景&源码分析](https://juejin.cn/post/6844903793008574477)

