# iOS App 启动优化(十)：Hook 总结之 Method Swizzle 源码分析

&emsp;Objective-C 中的 Hook 又被称作 Method Swizzling，这是动态语言大都具有的特性。在 Objective-C 中经常会把 Hook 的逻辑写在 `+load` 方法中，重写了 `+load` 方法的类和分类都被称为懒加载类、懒加载分类，它们会在 APP 启动时（这个时间阶段可以理解为：为 APP 启动做准备工作）、在 `main` 函数之前就会进行实现和加载（并调用 `+load` 方法），这个时机对 APP 启动而言是特别特别早的，这个阶段我们可以理解为 APP 正在启动或者正在为 APP 启动进行各项准备工作，此阶段 `+load` 方法被调用，其中我们添加的 Hook 逻辑也得到执行，然后等 APP 启动完成开始使用各个 OC 类、开始调用各个 OC 函数时，我们想要 Hook 的那些 OC 函数就都已经在我们的掌握了。

## Method Swizzle 简述

&emsp;利用 OC 的 Runtime 特性，动态改变 `SEL`（选择子）和 `IMP`（方法实现）的对应关系，达到 OC 方法调用流程改变的目的。主要用于 OC 方法，常用的 API 有：

+ `method_exchangeImplementations` 交换函数的 imp。
+ `class_replaceMethod` 替换函数的 imp。
+ `method_getImplementation` 与 `method_setImplementation` 直接 get/set 函数的 imp。
+ `class_addMethod` 向一个指定的类添加函数（如果添加成功，返回 YES，否则返回 NO，如果当前类已经存在该函数，则返回 NO）。

## 从源码梳理 Method Swizzle 相关的函数

&emsp;下面我们再次回到 objc4 源码中，对 Method Swizzle 涉及到的函数一探究竟。

### Method 数据结构回顾

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

&emsp;`struct method_t` 便是我们的 OC 函数对应的数据结构，`SEL name` 是函数的选择子（或者直接理解为函数名），选择子中是不包含参数类型以及返回值类型的（可以理解为参数标签字符串）。`const char *types` 是完整的函数类型描述，对函数的返回值、所有参数（包含参数类型、长度、顺序）进行描述。`SEL name` 和 `const char *types` 放在一起，我们便能完整推断出一个函数的名字、返回值类型、参数类型（或者理解为一个函数声明）。`MethodListIMP imp` 是方法实现（或者理解为函数的地址）。

### method_exchangeImplementations

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

### class_replaceMethod

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

&emsp;看到 `class_replaceMethod` 函数内部调用了 `addMethod` 函数，而这个 `addMethod` 函数是一个共用函数，在 `class_addMethod` 函数中也会调用 `addMethod`，不过它传递给 `addMethod` 函数的 `replace` 参数的值是 `NO`。

&emsp;暂时撇开判空和加锁代码后，两者是如下调用：

+ `class_replaceMethod`：`return addMethod(cls, name, imp, types ?: "", YES);`
+ `class_addMethod`：`return ! addMethod(cls, name, imp, types ?: "", NO);`

&emsp;`addMethod` 函数一个前面有 `!` 一个没有，是因为双方返回值不同，`class_replaceMethod` 返回函数的旧实现，`class_addMethod` 返回添加函数的结果 YES 或 NO。

&emsp;我们再讲一下 `addMethod` 函数中 `replace` 参数的意义：

&emsp;看到 `class_replaceMethod` 函数内部调用 `addMethod` 函数时，`addMethod` 函数的 `replace` 参数传递的是 `YES`，这里的 `replace` 参数表示的是：在 `addMethod` 函数内部如果 `cls` 已经存在 `name` 函数了，则是否要对 `name` 函数的旧实现进行替换。`class_replaceMethod` 函数自然就是为了替换函数，所以它调用 `addMethod` 函数时传的是 `YES`，表示如果 `cls` 类中已经存在 `name` 函数了则要把这个 `name` 函数的实现替换了，对比 `class_addMethod` 函数调用 `addMethod` 时 `replace` 参数传递的 `NO`，便是不进行替换。

&emsp;然后对于 `class_replaceMethod` 函数和 `class_addMethod` 函数而言，如果当前 `cls` 类中都不存在 `name` 函数时，`class_replaceMethod` 函数和 `class_addMethod` 函数两者都是对 `cls` 类添加 `name` 函数，后面我们会对 `class_addMethod` 函数逐行分析。

&emsp;下面看一下 `class_addMethod` 函数的声明和定义。

### class_addMethod

&emsp;向指定类添加一个函数，添加成功则返回 YES，否则返回 NO，如果当前类已经存在该函数，则返回 NO。

```c++
/** 
 * Adds a new method to a class with a given name and implementation.
 * 
 * @param cls The class to which to add a method.
 * @param name A selector that specifies the name of the method being added.
 * @param imp A function which is the implementation of the new method. The function must take at least two arguments—self and _cmd.
 * @param types An array of characters that describe the types of the arguments to the method. 
 * 
 * @return YES if the method was added successfully, otherwise NO 
 *  (for example, the class already contains a method implementation with that name).
 *
 * @note class_addMethod will add an override of a superclass's implementation, 
 *  but will not replace an existing implementation in this class. 
 *  To change an existing implementation, use method_setImplementation.
 */
OBJC_EXPORT BOOL
class_addMethod(Class _Nullable cls, SEL _Nonnull name, IMP _Nonnull imp, 
                const char * _Nullable types) 
    OBJC_AVAILABLE(10.5, 2.0, 9.0, 1.0, 2.0);
```

&emsp;根据给定的 `name` 选择子、`types` 方法类型和 `imp` 方法实现，把此方法添加到指定的 `cls` 类。`imp` 参数是新方法的实现，该函数必须至少接受两个参数 `self` 和 `_cmd`。`types` 是描述方法参数类型的字符数组。

&emsp;`class_addMethod` 将添加超类实现的覆盖，但不会替换此类中的现有实现，要更改现有实现，请使用 `method_setImplementation`（即 `class_addMethod` 函数要么添加完成返回 YES，要么该类已经存在此函数，直接返回 NO）（这句话看完我们可能会有点懵，下面我们细致的分析一下）。

&emsp;下面我们先看完 `class_addMethod` 函数的定义代码，然后再详细拆分一下：**`class_addMethod` 将添加超类实现的覆盖，但不会替换此类中的现有实现，要更改现有实现，请使用 `method_setImplementation`（即 `class_addMethod` 函数要么添加完成返回 YES，要么该类已经存在此函数，直接返回 NO）。**

```c++
BOOL 
class_addMethod(Class cls, SEL name, IMP imp, const char *types)
{
    // 如果 cls 不存在则返回 NO
    if (!cls) return NO;

    // 加锁
    mutex_locker_t lock(runtimeLock);
    
    // 调用 addMethod 函数，对返回值取反（addMethod 函数返回函数实现时取反为 NO，返回 nil 时取反为 YES） 
    return ! addMethod(cls, name, imp, types ?: "", NO);
}
```

&emsp;下面我们逐行分析 `addMethod` 函数的定义。

### addMethod

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
    // 前面 class_replaceMethod 函数调用中 replace 参数传递的 YES
    // 前面 class_addMethod 函数调用中 replace 参数传递的 NO
     
    // 这个 IMP 类型的 result 临时变量用于记录原始的函数实现，作为 addMethod 的函数返回值 
    IMP result = nil;
    
    // 断言：前一个调用是否已经对 runtimeLock 进行加锁
    runtimeLock.assertLocked();
    
    // 检查 cls 是否是一个已知类，根据所有已知类的列表检查给定的类。如果类未知，则会触发一个 _objc_fatal：Attempt to use unknown class。
    // 如果 runtime 知道该类（位于共享缓存内、加载映像的数据段内，或已使用 obj_allocateClassPair 分配），则返回 true。
    // 此查找的结果会缓存在类的 "witness"(cls->data()->witness) 字段中，该值在快速路径中方便检查。
    checkIsKnownClass(cls);
    
    // 断言：types 参数不能为 nil，在前面的调用中可看到对 types 参数的处理：types ?: "" 即默认传递："" 空字符串 
    ASSERT(types);
    
    // 断言：cls 已经实现，这里需要两个条件为真：
    // 1. cls 不是 StubClass（isStubClass：1 <= isa && isa < 16） 
    // 2. data()->flags & RW_REALIZED 为真 
    ASSERT(cls->isRealized());

    method_t *m;
    
    // 下面分两种（三种）情况的处理：
    // 1. cls 类中存在名字为 name 的函数则进行替换函数实现/取出函数的实现
    //    1)：如果 replace 参数为 YES，则设置 name 函数的实现为新实现
    //    2)：如果 replace 参数为 NO，则取得 name 函数的旧实现
    // 2. cls 类中不存在名字为 name 的函数则进行添加函数
    
    if ((m = getMethodNoSuper_nolock(cls, name))) {
        // getMethodNoSuper_nolock 在 cls 类的函数列表中查找名为 name 的函数，
        // 注意这里仅在 cls 类的 cls->data()->methods() 中查找，并不会去 cls 的父类中查找，函数名中的 "NoSuper" 也指明了这一点，
        // getMethod_nolock 不同于 getMethodNoSuper_nolock，它便是一直沿着 cls 的 superclass 继承链去查找函数。 
        
        // 查找到 name 对应的函数会赋值给 m 这个局部变量。
        
        // already exists
        if (!replace) {
            // 如果 replace 参数为 NO 的话，仅将获取到的 m 的 imp 赋值给 result 作为 addMethod 函数的返回值
            result = m->imp;
        } else {
            // 把查找到的 cls 类中名字为 name 的函数的 imp 替换为 addMethod 函数入参 imp，并返回 name 函数的旧的 imp
            // (还包含两个隐式操作：刷新 cls 类的方法缓存 和 更新 cls 类的自定义 AWZ 和 RR 的标记)
            result = _method_setImplementation(cls, m, imp);
        }
    } else {
        // 下面便是为 cls 添加名为 name 的新函数。
        
        // 获取 cls 类的函数列表所在的位置，rwe 的类型是：class_rw_ext_t 或者 class_ro_t，当类实现以后，就从 ro 切换到 rw 去了
        auto rwe = cls->data()->extAllocIfNeeded();

        // fixme optimize 优化
        // 我们已知的类的函数列表大部分情况下是一个二维数组（当还是 ro 时是一维数组），下面就是往这个二维数组中追加内容
        
        method_list_t *newlist;
        
        // calloc 在内存的动态存储区中分配 sizeof(*newlist) 个长度为 1 的连续空间，并返回此空间的起始地址。
        //（或者理解为创建一个长度为 1 的 method_list_t 数组）
        newlist = (method_list_t *)calloc(sizeof(*newlist), 1);
        
        // 标记数组中每个元素所占用的内存长度
        newlist->entsizeAndFlags = (uint32_t)sizeof(method_t) | fixed_up_method_list;
        
        // 当前数组的长度
        newlist->count = 1;
        
        // newlist 中第一个元素就是我们要给 cls 类添加的函数
        newlist->first.name = name; // 函数选择子赋值
        
        //（strdupIfMutable 函数是对 types 字符串进行处理，
        // 如果 types 位于不可变的内存空间中，则不需要作任何处理，
        // 如果 types 所处内存空间是可变的，则对 types 字符串进行复制）
        
        newlist->first.types = strdupIfMutable(types); // 函数类型
        newlist->first.imp = imp; // 函数实现
        
        // 对 newlist 作一下准备工作：（如果需要修复的话）
        // 1): 如果 选择子 不在共享缓存中，则把选择子注册到选择子的共享缓存中去
        // 2): 对 newlist 中的函数根据其选择子进行排序（std::stable_sort(mlist->begin(), mlist->end(), sorter);）
        
        prepareMethodLists(cls, &newlist, 1, NO, NO);
        
        // 把 newlist 追加到 cls 的函数列表中去
        rwe->methods.attachLists(&newlist, 1);
        
        // 刷新 cls 类的方法缓存（刷新 cls 的缓存，它的元类，及其子类。如果 cls 为 nil 的话会刷新所有的类）
        flushCaches(cls);
        
        // 因为之前并不存在旧函数所以 result 赋值为 nil
        result = nil;
    }
    
    // 返回 result，如果 cls 类中存在名字为 name 的函数则返回 name 函数的旧实现，如果 cls 类中不存在名字为 name 的函数，则返回 nil 
    return result;
}
```

&emsp;`addMethod` 函数注释的超详细，这里就不在展开了，下面我们再总结一下下面两个函数：

+ `class_replaceMethod`：`return addMethod(cls, name, imp, types ?: "", YES);`
+ `class_addMethod`：`return ! addMethod(cls, name, imp, types ?: "", NO);`

&emsp;`class_replaceMethod`：如果类中存在要替换的函数的话则直接替换函数的实现，并把旧实现返回，如果类中不存在要替换的函数的话则直接为该类添加此函数并返回 nil，这个 nil 即可用来表示在调用 `class_replaceMethod` 之前，类中不存在要替换的函数。

&emsp;`class_addMethod`：如果类中不存在要添加的函数，则直接添加到类中，最后返回 ture，如果类中已经存在要添加的函数了，则返回 false。

&emsp;同时这里还有一个细节点，就是在查找函数在类中是否存在时，仅在该类的函数列表中去查找，并不会递归去查找其父类，这一点要谨记，然后上面提到的：**`class_addMethod` 将添加超类实现的覆盖，但不会替换此类中的现有实现，要更改现有实现，请使用 `method_setImplementation`（即 `class_addMethod` 函数要么添加完成返回 YES，要么该类已经存在此函数，直接返回 NO）。** 就可以得到解释了，首先 `class_addMethod` 函数仅在本类的方法列表中查找并不会去其父类中查找（这里一定要和消息的发送查找流程做出理解上区分），所以如果要添加的函数在本类中不存在，但是在其父类中存在同名函数的话，那么本类中依然会添加此函数，这样添加到本类中的函数便会覆盖了父类中的同名函数。

&emsp;下面再顺便两个方法实现的 Set/Get 函数，一个进行读一个进行写。

### method_getImplementation/method_setImplementation

&emsp;`method_getImplementation` 和 `method_setImplementation` 函数看名字大概就能猜到它们的功能，分别是获取一个函数实现和设置一个函数的实现，其中的读操作 `method_getImplementation` 的定义超级简单，就仅仅是返回 `Method` 结构体的 `imp` 成员变量。其中的写操作 `method_setImplementation` 函数内容稍微多点：首先它是线程安全的，执行过程中使用 `runtimeLock` 进行加锁，然后为函数设置新的实现，并返回旧的实现，然后会刷新当前类的方法缓存，调整 RR/AWZ 函数的自定义标志。

&emsp;`method_getImplementation` 函数声明：

```c++
/** 
 * Returns the implementation of a method.
 * 返回方法的实现。
 * 
 * @param m The method to inspect.
 * 
 * @return A function pointer of type IMP.
 */
OBJC_EXPORT IMP _Nonnull
method_getImplementation(Method _Nonnull m) 
    OBJC_AVAILABLE(10.5, 2.0, 9.0, 1.0, 2.0);
```

&emsp;`method_getImplementation` 函数定义： 

```c++
IMP 
method_getImplementation(Method m)
{
    // 返回 imp
    return m ? m->imp : nil;
}
```

&emsp;`method_setImplementation` 函数声明：

```c++
/** 
 * Sets the implementation of a method.
 * 设置方法的实现。
 * 
 * @param m The method for which to set an implementation. 为其设置实现的方法。
 * @param imp The implemention to set to this method. 要设置为此方法的实现。
 * 
 * @return The previous implementation of the method. 方法的先前实现。
 */
OBJC_EXPORT IMP _Nonnull
method_setImplementation(Method _Nonnull m, IMP _Nonnull imp) 
    OBJC_AVAILABLE(10.5, 2.0, 9.0, 1.0, 2.0);
```

&emsp;`method_setImplementation` 函数定义：

```c++
IMP 
method_setImplementation(Method m, IMP imp)
{
    // Don't know the class - will be slow if RR/AWZ are affected
    // 如果是 RR/AWZ 函数的实现受到影响，需要执行更多操作，需要调整类的 Custom RR/AWZ 的标记
    
    // fixme build list of classes whose Methods are known externally?
    
    // 加锁
    mutex_locker_t lock(runtimeLock);
    
    // 调用 _method_setImplementation 函数，第一个所属类的参数传 nil 的话，会导致刷新当前所有类的方法缓存，调整当前类表中所有类的 RR/AWZ 标记
    return _method_setImplementation(Nil, m, imp);
}

/***********************************************************************
* method_setImplementation
* Sets this method's implementation to imp.The previous implementation is returned.
* 将此方法的实现设置为 imp。返回先前的实现。
**********************************************************************/
static IMP 
_method_setImplementation(Class cls, method_t *m, IMP imp)
{   
    // 断言：调用前是否已经对 runtimeLock 加锁
    runtimeLock.assertLocked();
    
    // 判空操作
    if (!m) return nil;
    if (!imp) return nil;

    // 记录方法的旧实现，替换方法的新实现
    IMP old = m->imp;
    m->imp = imp;

    // Cache updates are slow if cls is nil (i.e. unknown)
    // 如果 cls 为 nil，则缓存更新很慢（需要对当前所有类刷新方法缓存）
    
    // RR/AWZ updates are slow if cls is nil (i.e. unknown)
    // 如果 cls 为 nil，则 RR/AWZ 更新很慢
    
    // fixme build list of classes whose Methods are known externally?
    // fixme 构建其方法在外部已知的类的列表？
    
    // 如果 cls 不为 nil，则只刷新 cls 以及 cls 已实现的子类的方法缓存
    flushCaches(cls);
    
    // 当方法更改其 IMP 时更新自定义的 RR 和 AWZ 的标记
    adjustCustomFlagsForMethodChange(cls, m);

    return old;
}
```

&emsp;在设置方法实现的过程中，涉及到的方法缓存刷新和调整 RR/AWZ 的函数标记可能会让我们有一点懵，这里需要我们对 OC 的方法缓存机制和 OC alloc 过程有一定的认识，前面的文章有分析这部分的内容，不熟悉的小伙伴可以去翻翻看一下。 

&emsp;上面我们对 Method Swizzle 使用到的部分 runtime 函数进行了完整的分析，下面我们看一下 Method Swizzle 的方案实践。

&emsp;其实看到这里的话，如果在  Method Swizzle  中不考虑继承关系，只是单纯的 修改/交换 当前类的一些函数，不考虑继承自父类函数的话，其实它们还蛮简单的，但是这样也就失去了 **继承的精髓**，前面我们学习 runtime 时，实例函数/类函数、子类/父类/元类、函数列表 等等，它们的关系一定要了然于胸，不然看后续的 Method Swizzle 实践会在理解上有一点吃力。

+ 实例函数位于类中。
+ 类函数位于元类中。
+ **子类继承了父类，在子类中不重写父类函数的话，那么子类可调用的父类的函数还是在位于父类的函数列表中，如果在子类的定义中重写了父类的函数，那么子类中便也有了一份和父类同名的函数。** （一定要谨记这句）

## Method Swizzle 实践

&emsp;这里分为两种情况，1): 要 Hook 的函数在类中不存在（不存在时我们首先进行添加）。2): 要 Hook 的函数在类中已存在（则可直接进行 hook 交换）。

### 示例 1

&emsp;下面代码实现的功能就是在交换 original 函数和 swizzled 函数的 imp。

```c++
// 放在 +load 函数中进行，dispatch_once 保证全局只调用一次。
+ (void)load {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        Class aClass = [self class];
        
        // original 选择子
        SEL originalSelector = @selector(method_original:);
        // swizzled 选择子
        SEL swizzledSelector = @selector(method_swizzle:);
        
        // 查找 original 函数（注意：class_getInstanceMethod 会沿着继承链在当前类以及父类中递归查找实例函数结构体（类函数则是在元类中查找））
        Method originalMethod = class_getInstanceMethod(aClass, originalSelector);
        // 查找 swizzled 函数（在当前类及其父类中查找函数结构体）
        Method swizzledMethod = class_getInstanceMethod(aClass, swizzledSelector);
        
        // 在 aClass 中添加原函数，如果 didAddMethod 为 true 表示在 aClass 中添加 original 函数成功（注意 original 函数直接添加的 swizzledMethod 函数的实现）
        // 如果 didAddMethod 为 false 则表示在本类中已经存在 original 函数了。（注意这里是本类，class_addMethod 只是针对的本类，即使在父类中存在 original 同名的函数，
        // class_addMethod 还是会往本类中添加 original 函数，此时便会覆盖了父类中的 original 函数）
        
        BOOL didAddMethod = class_addMethod(aClass,
                                            originalSelector,
                                            method_getImplementation(swizzledMethod),
                                            method_getTypeEncoding(swizzledMethod));
                                            
        if (didAddMethod) {
            // 添加成功的话，仅剩下把 swizzled 函数的实现替换成 original 函数的实现
            class_replaceMethod(aClass,
                                swizzledSelector,
                                method_getImplementation(originalMethod),
                                method_getTypeEncoding(originalMethod));
        } else {
            // 原本已经存在 original 和 swizzled 函数的话，直接交换它们俩的实现
            method_exchangeImplementations(originalMethod, swizzledMethod);
        }
    });
}
```

&emsp;首先查找 originalSelector 选择子和 swizzledSelector 选择子在 aClass 中对应的函数结构 originalMethod 和 swizzledMethod，然后调用 class_addMethod 函数向 aClass 中添加 originalSelector 选择子对应的函数，并且将其 imp 映射为 swizzledMethod 函数的 imp，class_addMethod 函数执行时，如果 aClass 中已经存在  originalSelector 选择子对应的函数，则不做任何操作 class_addMethod 函数的返回值为 false， 如果 aClass 中不存在 originalSelector 选择子对应的函数的话，则在 aClass 中添加 originalSelector 选择子对应的函数，函数 imp 是 swizzledMethod 函数的 imp，且 class_addMethod 函数的返回值为 true。接着如果 didAddMethod 为真，则需要把 aClass 中 swizzledSelector 选择子对应的函数的 imp 替换为 originalMethod 函数的 imp，如果 didAddMethod 为假，则表明 cClass 中原本已经有 originalMethod 和 swizzledMethod 函数了，则调用 method_exchangeImplementations 函数交换它俩的 imp。（例如在自定义 UIViewController 中有一个 viewDidLoad 函数和一个我们自定义的 swizzledViewDidLoad 函数，然后我们在自定义 UIViewController 的 +load 函数中交换 viewDidLoad 和 swizzledViewDidLoad 两个函数的 imp）

> &emsp;有时为了避免方法命名冲突和参数 `_cmd` 被篡改，也会使用下面这种『静态方法版本』的 Method Swizzle。CaptainHook 中的宏定义也是采用这种方式，比较推荐：[Objective-C-Method-Swizzling](http://yulingtianxia.com/blog/2017/04/17/Objective-C-Method-Swizzling/) (这里大佬说的 **避免方法命名冲突和参数 `_cmd` 被篡改** 这两个点涉及的场景没有理解到，暂时只能把下面的的代码读通了😭)

```c++
// 类型命名 IMPPointer 表示指向 IMP 的指针
typedef IMP *IMPPointer;

// swizzle 函数声明（原函数要替换为的函数）
static void MethodSwizzle(id self, SEL _cmd, id arg1);

// original 函数指针（用来记录替换发生后原函数的实现）
static void (*MethodOriginal)(id self, SEL _cmd, id arg1);

// 交换发生后，当调用 @selector(originalMethod:) 选择子对应的函数时，会调用 MethodSwizzle 函数
static void MethodSwizzle(id self, SEL _cmd, id arg1) {
    // do custom work
    // 这里可以放一些我们自己的自定义操作
    
    // 然后下面接着通过 MethodOriginal 这个 IMP 指针调用 @selector(originalMethod:) 选择子对应的原始的函数实现！
    MethodOriginal(self, _cmd, arg1);
}

BOOL class_swizzleMethodAndStore(Class class, SEL original, IMP replacement, IMPPointer store) {
    IMP imp = NULL;
    
    // 从 class 以及继承体系中获取 original 选择子对应的函数结构体
    Method method = class_getInstanceMethod(class, original);
    
    // 如果能找到的话，则把 original 选择子对应的函数结构体的 imp 替换为 replacement 这个 imp
    if (method) {
        // method 的类型编码
        const char *type = method_getTypeEncoding(method);
        
        // 替换 original 选择子对应的函数结构体的 imp
        imp = class_replaceMethod(class, original, replacement, type);
        
        // 这里如果上面的 Method method = class_getInstanceMethod(class, original); 获得函数来自 class 的父类，则这个 imp 会是 nil，
        // 如果这句没看懂的话应该回顾上面的 class_replaceMethod 函数定义，它替换的是当前类的函数列表中的函数，如果没找到的话则不会去递归其父类。
        
        // 如果 imp 为空的话，则把 class 父类中的 original 选择子对应的函数的 imp 赋值给 imp 局部变量 
        if (!imp) {
            imp = method_getImplementation(method);
        }
    }
    
    // 如果未找到 original 选择子对应的函数结构体的话，这个 if 内的赋值不会执行。
    // 如果找到了 original 选择子对应的函数结构体的，这里可能是两种情况：
    // 1): 在 class 中找到的
    // 2): 在 class 的父类中找到的
    // 然后不管是在哪里找到的，这里的 imp 变量记录的就是这个 original 选择子对应的函数结构体的 imp，然后把它赋值给 *store，
    // 即把  original 选择子对应的函数的原始的实现记录在 MethodOriginal 指针中。 
    if (imp && store) { *store = imp; }
    
    // 如果 original 选择子对应的函数结构体不存在的话，这里会直接返回 NO，存在的话则是返回 YES
    return (imp != NULL);
}

// 返回 YES 表示替换成功，返回 NO 表示原函数不存在，无法进行替换
+ (BOOL)swizzle:(SEL)original with:(IMP)replacement store:(IMPPointer)store {
    return class_swizzleMethodAndStore(self, original, replacement, store);
}

// 在 +load 函数中进行
+ (void)load {
    // 把当前类的 originalMethod 选择子对应的函数替换为 MethodSwizzle，然后在 MethodOriginal 这个 IMP 指针中记录 originalMethod 选择子对应的函数的原始实现
    // (IMP)MethodSwizzle 把函数地址强转为 IMP
    [self swizzle:@selector(originalMethod:) with:(IMP)MethodSwizzle store:(IMP *)&MethodOriginal];
}
```

&emsp;`[self swizzle:@selector(originalMethod:) with:(IMP)MethodSwizzle store:(IMP *)&MethodOriginal];` 函数，把当前类的 originalMethod 选择子对应的函数替换为 MethodSwizzle，然后在 MethodOriginal 这个 IMP 指针中记录 originalMethod 选择子对应的函数的原始实现。 

### 示例 2
 
&emsp;类似示例 1 中第一段示例代码，但是缺少了往本类中添加函数。
 
```c++
+ (void)load
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        Class aClass = [self class];
        
        // method_original 和 method_swizzle 选择子
        SEL originalSelector = @selector(method_original:);
        SEL swizzledSelector = @selector(method_swizzle:);
        
        // 这里是拿着 originalSelector 和 swizzledMethod 两个选择子去 aClass 及其父类中查找函数，
        // 注意 class_getInstanceMethod 函数是沿着继承链，去本类及其父类的函数列表中递归查找函数，
        // 所以这里返回的 originalMethod 和 swizzledMethod 函数的位置都是不固定的，它们可能来自本类，可能来自父类。
        Method originalMethod = class_getInstanceMethod(aClass, originalSelector);
        Method swizzledMethod = class_getInstanceMethod(aClass, swizzledSelector);
        
        // 直接交换 originalMethod 和 swizzledMethod 两个函数的 imp
        method_exchangeImplementations(originalMethod, swizzledMethod);
    });
}
```

&emsp;这段代码是直接交换 IMP，这样做是很危险的，危险的来源就如注释中所说：调用 `class_getInstanceMethod` 函数取得的函数结构体，我们根本不知道是来自本类还是父类，这段代码便直接对他们进行 IMP 交换了，这里和示例 1 中的第一段代码对比的话，缺少了 “函数来源” 的判断，示例 1 中的第一段代码借用 `class_addMethod` 函数来判断本类中有没有原始函数，如果没有的话会进行添加，这样就避免在 Hook 本类的原始函数的过程，不会影响到父类。当然如果本类中存在原始函数，则示例 1 和示例 2 执行结果就一模一样了。那么如果本类不存在原始函数，原始函数来自于父类呢：如果本类中没有实现原始函数，`class_getInstanceMethod` 返回的是其某个父类的 Method 对象，这样 `method_exchangeImplementations` 就把父类的原始函数的 imp 跟本类的 Swizzle 函数实现交换了。这样其它父类及其其他子类的原始函数调用就可能会出问题，甚至 Crash。

&emsp;那么 Objective-C Method Swizzling 的背景知识就学到这里，后续涉及的使用场景，后续我们再进行分析。

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




/////// 网页备份
http://southpeak.github.io/2015/01/25/tool-lldb/
http://yulingtianxia.com/blog/2017/04/17/Objective-C-Method-Swizzling/
https://github.com/nygard/class-dump/commits/master
https://zhuanlan.zhihu.com/p/304443154
https://segmentfault.com/a/1190000007799086
https://github.com/huanxsd?tab=repositories
https://www.cnblogs.com/17bdw/p/12239505.html
https://hex-rays.com/cgi-bin/quote.cgi
https://github.com/aidansteele
https://zhangbuhuai.com/post/macho-structure.html
https://juejin.cn/post/6844903889523884039
https://juejin.cn/user/3438928103236920/posts
https://juejin.cn/post/6844903992904908814
https://juejin.cn/post/6844903912147795982
https://juejin.cn/post/6844903912143585288
https://juejin.cn/post/6844903922654511112
https://juejin.cn/post/6844903926051897358
http://southpeak.github.io/2015/01/25/tool-lldb/
https://developer.apple.com/library/archive/documentation/IDEs/Conceptual/gdb_to_lldb_transition_guide/document/Introduction.html#//apple_ref/doc/uid/TP40012917-CH1-SW1
https://www.jianshu.com/p/e1e7c3c2e773
https://blog.csdn.net/tobebetterprogrammer/article/details/51374046
https://www.jianshu.com/p/0a27ab6ad5e1
https://www.jianshu.com/p/cac59b17bb55
https://juejin.cn/post/6844903582655840270

