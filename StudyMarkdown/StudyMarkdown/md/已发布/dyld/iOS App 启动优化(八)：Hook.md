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

&emsp;首先我们先看一下官方的描述：

&emsp;fishhook 是一个非常简单的库，它支持在 模拟器和设备上的 `iOS` 中运行的 Mach-O 二进制文件中动态地重新绑定符号。这提供了类似于在 `OS X` 上使用 `DYLD_INTERPOSE` 的功能。在 Facebook 上，我们发现它是一种很有用的方法，可以在 `libSystem` 中钩住调用（hook calls）以进行调试/跟踪（debugging/tracing）（for example, auditing for double-close issues with file descriptors）。

### fishhook 的使用方式

&emsp;fishhook 的使用方式非常简单，我们只需要把 `fishhook.h/fishhook.c` 文件拖入我们的项目中，然后就可以按如下方式重新绑定符号：

```c++
#import <dlfcn.h>

#import <UIKit/UIKit.h>

#import "AppDelegate.h"
#import "fishhook.h"
 
static int (*orig_close)(int);
static int (*orig_open)(const char *, int, ...);
 
int my_close(int fd) {
  printf("🤯🤯🤯 Calling real close(%d)\n", fd);
  
  return orig_close(fd);
}
 
int my_open(const char *path, int oflag, ...) {
  va_list ap = {0};
  mode_t mode = 0;
 
  if ((oflag & O_CREAT) != 0) {
    // mode only applies to O_CREAT
    va_start(ap, oflag);
    mode = va_arg(ap, int);
    va_end(ap);
    
    printf("🤯🤯🤯 Calling real open('%s', %d, %d)\n", path, oflag, mode);
    return orig_open(path, oflag, mode);
  } else {
  
    printf("🤯🤯🤯 Calling real open('%s', %d)\n", path, oflag);
    return orig_open(path, oflag, mode);
  }
}
 
int main(int argc, char * argv[])
{
  @autoreleasepool {
    
    // ⬇️⬇️⬇️ 这里我们把系统的 close 和 open 的符号和我们自己的 my_close 和 my_open 函数符号进行交换，
    // 那样下面的 open 和 close 函数调用时，就会执行我们的 my_open 和 my_open 函数
    rebind_symbols((struct rebinding[2]){{"close", my_close, (void *)&orig_close}, {"open", my_open, (void *)&orig_open}}, 2);
 
    // Open our own binary and print out first 4 bytes (which is the same for all Mach-O binaries on a given architecture)
    // 还记得 argv[0] 的，它就是我们当前程序的可执行文件的本地路径，然后直接读取它的前 4 个字节的内容，
    // 即对应于 mach_header 结构体中 magic 魔数，用来表示当前的 mach-o 格式的文件是一个什么类型的文件，
    // 如果我们对之前学习 mach-o 时还有印象的话，那么这里第一时间应该想到的就是 "可执行文件"
    
    printf("➡️➡️➡️ %s \n", argv[0]);
    
    int fd = open(argv[0], O_RDONLY);
    uint32_t magic_number = 0;
    read(fd, &magic_number, 4);
    
    printf("🤯🤯🤯 Mach-O Magic Number: %x \n", magic_number);
    
    close(fd);
 
    return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
  }
}
```

&emsp;运行上面的代码，我们的控制台就会看到如下输出：

```c++
// argv[0] 即我们的可执行文件的本地路径
➡️➡️➡️ /Users/hmc/Library/Developer/CoreSimulator/Devices/CC2922E4-A2DB-43DF-8B6F-D2987F683525/data/Containers/Bundle/Application/37AD7905-E15C-4039-905D-B474D67074AE/Test_ipa_simple.app/Test_ipa_simple

// int fd = open(argv[0], O_RDONLY); 调用系统的 open 函数，就会调用我们的自己的 my_open 函数
🤯🤯🤯 my_open Calling real open('/Users/hmc/Library/Developer/CoreSimulator/Devices/CC2922E4-A2DB-43DF-8B6F-D2987F683525/data/Containers/Bundle/Application/BD248843-0DA5-4D0F-91C5-7EBE5D97E687/Test_ipa_simple.app/Test_ipa_simple', 0)

// feedfacf 是我们前面学习 mach-o 时贼熟悉的一个魔数了，表示当前是一个 mach-o 格式的可执行文件
🤯🤯🤯 main Mach-O Magic Number: feedfacf

// close(fd); 同样，调用系统的 close 函数，就会调用我们自己的 my_close 函数
🤯🤯🤯 my_close Calling real close(3)
...

// 然后下面还有一堆的 my_open 和 my_close 的打印，是程序运行时其它的一些 open 和 close 的调用，感兴趣的话可以自己打印看看。 
```

&emsp;那么下面我们看一下 fishhook 内部是怎么工作的。

### fishhook How it works

&emsp;`dyld` 通过更新 Mach-O 二进制文件的 `__DATA` 段的特定部分中的指针来绑定 lazy 和 non-lazy 符号。 fishhook 通过确定传递给 `rebind_symbols` 的每个符号名称的更新位置，然后写出相应的替换来重新绑定这些符号。

&emsp;对于给定的 image 镜像，`__DATA` 段可能包含两个与动态符号绑定（dynamic symbol bindings）相关的 sections：`__nl_symbol_ptr` 和 `__la_symbol_ptr`。 `__nl_symbol_ptr` 是一个指向 non-lazily 绑定数据的指针数组（这些是在加载 library 时绑定的），而 `__la_symbol_ptr` 是一个指向导入函数（imported functions ）的指针数组，通常在第一次调用该符号时由名为 `dyld_stub_binder` 的例程填充（也可以告诉 `dyld` 在启动时绑定这些）。为了在这些 sections 之一中找到与特定位置相对应的符号名称，我们必须跳过几个间接层。对于两个相关的 sections，section headers（`struct section` 来自 <mach-o/loader.h>）提供了一个偏移量（在 `reserved1` 字段中）到所谓的间接符号表中。位于二进制文件的 `__LINKEDIT` 段中的间接符号表只是符号表（也在 `__LINKEDIT` 中）的索引数组，其顺序与 non-lazy 和 lazy 符号 sections 中的指针的顺序相同。因此，给定的 `struct section nl_symbol_ptr`，该 section 中第一个地址的符号表中对应的索引是 `indirect_symbol_table[nl_symbol_ptr->reserved1]`。符号表本身是一个 `struct nlist` 数组（参见 `<mach-o/nlist.h>`），每个 `nlist` 都包含一个指向 `__LINKEDIT` 中字符串表的索引，其中存储了实际的符号名称。因此，对于每个指针 `__nl_symbol_ptr` 和 `__la_symbol_ptr`，我们能够找到相应的符号，然后找到相应的字符串与请求的符号名称进行比较，如果有匹配项，我们将替换 section 中的指针。

&emsp;在 `lazy` 和 `non-lazy` 指针表中查找给定条目名称的过程如下所示：

![687474703a2f2f692e696d6775722e636f6d2f4856587148437a2e706e67-2.png](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/27cc376ebf5342c98659f61da97e9895~tplv-k3u1fbpfcp-watermark.image)

&emsp;好了，下面我们直接看源码：`fishhook.h/fishhook.c`。

### fishhook 源码解读

#### fishhook.h 

&emsp;










































## 参考链接
**参考链接:🔗**
+ [iOS逆向 RSA理论](https://juejin.cn/post/6844903989666906125)
+ [iOS逆向 HOOK原理之fishhook](https://juejin.cn/post/6845166890772332552)
