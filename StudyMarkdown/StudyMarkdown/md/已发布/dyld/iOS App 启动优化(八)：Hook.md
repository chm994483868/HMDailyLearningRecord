# iOS App 启动优化(八)：Hook 总结

## Runtime API 回顾

1. objc_xxx 系列函数（objc_ 系列函数关注于宏观使用，如类与协议的空间分配、注册、注销 等操作）

| 函数名称 | 函数作用 |
| --- | --- |
| objc_getClass | 获取 Class 对象 |
| objc_getMetaClass | 获取 MetaClass 对象 |
| objc_allocateClassPair | 分配空间，创建类（仅在创建之后，注册之前 能够添加成员变量） |
| objc_registerClassPair | 注册一个类（注册后方可使用该类创建对象） |
| objc_disposeClassPair | 注销某个类 |
| objc_allocateProtocol | 开辟空间创建协议 |
| objc_registerProtocol | 注册一个协议 |
| objc_constructInstance | 构造一个实例对象（ARC 下无效） |
| objc_destructInstance | 析构一个实例对象（ARC 下无效） |
| objc_setAssociatedObject | 为实例对象关联对象 |
| objc_getAssociatedObject | 获取实例对象的关联对象 |
| objc_removeAssociatedObjects | 清空实例对象的所有关联对象 |

2. class_xxx 系列函数（class_ 系列函数关注于类的内部，如实例变量、属性、方法、协议等相关问题）

| 函数名称 | 函数作用 |
| --- | --- |
| class_addIvar | 为类添加实例变量 |
| class_addProperty | 为类添加属性 |
| class_addMethod | 为类添加方法 |
| class_addProtocol | 为类遵循协议 |
| class_replaceMethod | 替换类某方法的实现 |
| class_getName | 获取类名 |
| class_isMetaClass | 判断是否为元类 |
| objc_getProtocol | 获取某个协议 |
| objc_copyProtocolList | 拷贝在运行时中注册过的协议列表 |
| class_getSuperclass | 获取某类的父类 |
| class_setSuperclass | 设置某类的父类 |
| class_getProperty | 获取某类的属性 |
| class_getInstanceVariable | 获取实例变量 |
| class_getClassVariable | 获取类变量 |
| class_getInstanceMethod | 获取实例方法 |
| class_getClassMethod | 获取类方法 |
| class_getMethodImplementation | 获取方法的实现 |
| class_getInstanceSize | 获取类的实例的大小 |
| class_respondsToSelector | 判断类是否实现某方法 |
| class_conformsToProtocol | 判断类是否遵循某协议 |
| class_createInstance | 创建类的实例 |
| class_copyIvarList | 拷贝类的实例变量列表 |
| class_copyMethodList | 拷贝类的方法列表 |
| class_copyProtocolList | 拷贝类遵循的协议列表 |
| class_copyPropertyList | 拷贝类的属性列表 |

3. object_xxx 系列函数（object_ 系列函数关注于对象的角度，如实例变量）

| 函数名称 | 函数作用 |
| --- | --- |
| object_copy | 对象 copy (ARC 无效) |
| object_dispose | 对象释放 (ARC 无效) |
| object_getClassName | 获取对象的类名 |
| object_getClass | 获取对象的 Class |
| object_setClass | 设置对象的 Class |
| object_getIvar | 获取对象中实例变量的值 |
| object_setIvar | 设置对象中实例变量的值 |
| object_getInstanceVariable | 获取对象中实例变量的值 (ARC 中无效,使用 object_getIvar) |
| object_setInstanceVariable | 设置对象中实例变量的值 (ARC 中无效,使用 object_setIvar) |

4. method_xxx 系列函数（method_ 系列函数关注于方法内部，如方法的参数及返回值类型和方法的实现）

| 函数名称 | 函数作用 |
| --- | --- |
| method_getName | 获取方法名 |
| method_getImplementation | 获取方法的实现 |
| method_getTypeEncoding | 获取方法的类型编码 |
| method_getNumberOfArguments | 获取方法的参数个数 |
| method_copyReturnType | 拷贝方法的返回类型 |
| method_getReturnType | 获取方法的返回类型 |
| method_copyArgumentType | 拷贝方法的参数类型 |
| method_getArgumentType | 获取方法的参数类型 |
| method_getDescription | 获取方法的描述 |
| method_setImplementation | 设置方法的实现 |
| method_exchangeImplementations | 替换方法的实现 |

5. property_xxx 系列函数（property_ 系列函数关于与属性内部，如属性的特性等）

| 函数名称 | 函数作用 |
| --- | --- |
| property_getName | 获取属性名 |
| property_getAttributes | 获取属性的特性列表 |
| property_copyAttributeList | 拷贝属性的特性列表 |
| property_copyAttributeValue | 拷贝属性中某特性的值 |

6. protocol_xxx 系列函数

| 函数名称 | 函数作用 |
| --- | --- |
| protocol_conformsToProtocol | 判断一个协议是否遵循另一个协议 |
| protocol_isEqual | 判断两个协议是否一致 |
| protocol_getName | 获取协议名称 |
| protocol_copyPropertyList | 拷贝协议的属性列表 |
| protocol_copyProtocolList | 拷贝某协议所遵循的协议列表 |
| protocol_copyMethodDescriptionList | 拷贝协议的方法列表 |
| protocol_addProtocol | 为一个协议遵循另一协议 |
| protocol_addProperty | 为协议添加属性 |
| protocol_getProperty | 获取协议中的某个属性 |
| protocol_addMethodDescription | 为协议添加方法描述 |
| protocol_getMethodDescription | 获取协议中某方法的描述 |

7. ivar_xxx 系列函数

| 函数名称 | 函数作用 |
| --- | --- |
| ivar_getName | 获取 Ivar 名称 |
| ivar_getTypeEncoding | 获取类型编码 |
| ivar_getOffset | 获取偏移量 |

8. sel_xxx 系列函数

| 函数名称 | 函数作用 |
| --- | --- |
| sel_getName | 获取名称 |
| sel_getUid | 获取 Uid |
| sel_registerName | 注册方法 |
| sel_isEqual | 判断方法是否相等 |

9. imp_xxx 系列函数

| 函数名称 | 函数作用 |
| --- | --- |
| imp_implementationWithBlock | 通过代码块创建 IMP |
| imp_getBlock | 获取函数指针中的代码块 |
| imp_removeBlock | 移除 IMP 中的代码块 |

## Hook 概述

### Hook 的定义

&emsp;Hook 翻译成中文为 “钩子” “挂钩”，在 iOS 逆向领域中指的是改变程序运行流程的一种技术，通过 Hook 可以让别人的程序执行自己所写的代码。

### Hook 的内容

&emsp;OC 函数、Block、C/C++ 函数 

### Hook 的方式

&emsp;在 iOS 中 Hook 技术有以下几种：

+ `Method Swizzling` 利用 OC 的 Runtime 特性，动态改变 `SEL`（选择子）和 `IMP`（方法实现）的对应关系，达到 OC 方法调用流程改变的目的。
+ `fishhook` 这是 FaceBook 提供的一个动态修改链接 Mach-O 文件的工具，利用 Mach-O 文件加载原理，通过修改懒加载和非懒加载两个表的指针达到 C 函数 Hook 的目的。
+ `Cydia Substrate` 原名为 `Mobile Substrate`，它的主要作用是针对 OC 方法、C 函数以及函数地址进行 Hook 操作，且安卓也能使用。

> &emsp;之前介绍过 `Method Swizzling`，OC 的 Runtime 特性让它有了 “黑魔法” 之称，同时也是局限性所在。

&emsp;三者的区别如下：
+ `Method Swizzling` 只适用于动态的 OC 方法（运行时确认函数实现地址）
+ `fishhook` 适用于静态的 C 方法（编译时确定函数实现地址）
+ `Cydia Substrate` 是一种强大的框架，只需要通过 Logos 语言（类似于正向开发）就可以进行 Hook，适用于 OC 方法、C 函数以及函数地址。

## fishhook 解读

> &emsp;A library that enables dynamically rebinding symbols in Mach-O binaries running on iOS.
> &emsp;在 iOS 上运行的 Mach-O 二进制文件中启用 **动态重新绑定符号** 的库。

&emsp;[fishhook](https://github.com/facebook/fishhook)

&emsp;fishhook 是一个非常简单的库，它支持在 模拟器和设备上的 `iOS` 上运行的 Mach-O 二进制文件中动态地重新绑定符号。这提供了类似于在 `OS X` 上使用 `DYLD_INTERPOSE` 的功能。在 Facebook 上，我们发现它是一种很有用的方法，可以在 `libSystem` 中钩住调用以进行调试/跟踪（例如，审计文件描述符的双重关闭问题）。

&emsp;`dyld` 通过更新 Mach-O 二进制文件的 `__DATA` 的特定部分的指针来绑定懒加载符号和非懒加载符号。fishhook 通过为传递给 rebind_符号的每个符号名确定要更新的位置，然后写出相应的替换来重新绑定这些符号。






## 参考链接
**参考链接:🔗**
+ [iOS逆向 RSA理论](https://juejin.cn/post/6844903989666906125)
+ [iOS逆向 HOOK原理之fishhook](https://juejin.cn/post/6845166890772332552)
