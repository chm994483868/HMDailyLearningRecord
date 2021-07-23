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
> &emsp;在 iOS 上运行的 Mach-O 二进制文件中启用 **动态重新绑定符号** 的库。（仅限于系统的 C 函数）

&emsp;[fishhook](https://github.com/facebook/fishhook)

&emsp;首先我们先看一下官方的描述：

&emsp;fishhook 是一个非常简单的库，它支持在 模拟器和设备上的 `iOS` 中运行的 Mach-O 二进制文件中动态地重新绑定符号（仅限于系统的 C 函数）。这提供了类似于在 `OS X` 上使用 `DYLD_INTERPOSE` 的功能。在 Facebook 上，我们发现它是一种很有用的方法，可以在 `libSystem` 中钩住调用（hook calls）以进行调试/跟踪（debugging/tracing）（for example, auditing for double-close issues with file descriptors）。

### fishhook 的使用方式

&emsp;fishhook 的使用方式非常简单，我们只需要把 `fishhook.h/fishhook.c` 文件拖入我们的项目中，然后就可以按如下方式重新绑定符号：

```c++
#import <dlfcn.h>

#import <UIKit/UIKit.h>

#import "AppDelegate.h"
#import "fishhook.h"

// 这里声明两个静态全局的函数指针变量，分别用来记录系统函数 close 和 open 的地址
static int (*orig_close)(int);
static int (*orig_open)(const char *, int, ...);
 
int my_close(int fd) {
  printf("🤯🤯🤯 Calling real close(%d)\n", fd);
  
  // 经过下面 main 函数中的 rebind_symbols 调用后，这里的 orig_close 指针指向的地址就是系统的 close 函数 
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
    
    // 同上，这里的 orig_open 指向系统的 open 函数
    return orig_open(path, oflag, mode);
  } else {
    printf("🤯🤯🤯 Calling real open('%s', %d)\n", path, oflag);
    
    // 同上，这里的 orig_open 指向系统的 open 函数
    return orig_open(path, oflag, mode);
  }
}
 
int main(int argc, char * argv[])
{
  @autoreleasepool {
    
    // ⬇️⬇️⬇️ 这里是把系统的 close 和 open 函数的地址替换为我们自己的 my_close 和 my_open 函数，
    // 并且使用 orig_close 和 orig_open 两个静态全局变量记录系统的 close 和 open 的原始的函数地址。
    
    // 那样下面的 open 和 close 函数调用时，就会执行我们的 my_open 和 my_open 函数，然后它们内部又通过 orig_open 和 orig_close 来调用系统内原始的 open 和 close 函数
    rebind_symbols((struct rebinding[2]){{"close", my_close, (void *)&orig_close}, {"open", my_open, (void *)&orig_open}}, 2);
 
    // Open our own binary and print out first 4 bytes (which is the same for all Mach-O binaries on a given architecture)
    // 还记得 argv[0] 的，它就是我们当前程序的可执行文件的本地路径，然后直接读取它的前 4 个字节的内容，
    // 即对应于 mach_header 结构体中 magic 魔数，用来表示当前的 mach-o 格式的文件是一个什么类型的文件，（Mach-O Type）
    // 如果我们对之前学习 mach-o 时还有印象的话，那么这里第一时间应该想到的就是："可执行文件"。
    
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

// int fd = open(argv[0], O_RDONLY); 调用系统的 open 函数转变为调用我们自己的 my_open 函数
🤯🤯🤯 my_open Calling real open('/Users/hmc/Library/Developer/CoreSimulator/Devices/CC2922E4-A2DB-43DF-8B6F-D2987F683525/data/Containers/Bundle/Application/BD248843-0DA5-4D0F-91C5-7EBE5D97E687/Test_ipa_simple.app/Test_ipa_simple', 0)

// feedfacf 是我们前面学习 mach-o 时贼熟悉的一个魔数了，表示当前是一个 mach-o 格式的可执行文件
🤯🤯🤯 main Mach-O Magic Number: feedfacf

// close(fd); 同样，调用系统的 close 函数转变为调用我们自己的 my_close 函数
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

&emsp;`fishhook.h` 文件内容极少，只有一个结构体定义和两个函数声明。

##### struct rebinding

```c++
/*
 * A structure representing a particular intended rebinding from a symbol name to its replacement
 */
struct rebinding {
  const char *name;
  void *replacement;
  void **replaced;
};
```

&emsp;`rebinding` 表示从 symbol name（指 `name`） 到其替换（指 `replacement`）的特定预期重新绑定的 结构体。

&emsp;如我们上面的示例代码中：`(struct rebinding[2]){{"close", my_close, (void *)&orig_close}, {"open", my_open, (void *)&orig_open}}`，`rebinding` 是一个长度是 2 的 `rebinding` 结构体数组，其中第一个元素的字面量写法可转换为如下：

```c++
struct rebinding closeVariable;
closeVariable.name = "close";
closeVariable.replacement = my_close;
closeVariable.replaced = (void *)&orig_close;
```

&emsp;`rebinding` 结构体是用来定义替换函数使用的一个数据结构。`name` 成员变量表示要 Hook 的函数名称（ C 字符串），`replacement` 指针用来指定新的函数地址，即把 `name` 对应的函数的函数地址替换为 `replacement`（C 函数的名称就是一个函数指针，静态语言编译时就已确定。）`replaced` 是一个双重指针，只所以是这样是为了在函数内部直接修改外部指针变量的指向，用在 `rebinding` 结构体中就是为了记录 `name` 对应的原始函数的地址。 对应上面的 `closeVariable` 变量，当把其作为 `rebind_symbols` 函数的参数执行时，找到 `close`（它是一个系统函数）函数把它指向替换为 `my_close` 函数，`orig_close` 则是用来记录 `close` 函数的原始地址。

##### rebind_symbols

```c++
/*
 * For each rebinding in rebindings, rebinds references to external, 
 * indirect symbols with the specified name to instead point at replacement
 * for each image in the calling process as well as for all future images that are loaded by the process. 
 * If rebind_functions is called more than once, the symbols to rebind are added to the existing list of rebindings, 
 * and if a given symbol is rebound more than once, the later rebinding will take precedence.
 */
FISHHOOK_VISIBILITY
int rebind_symbols(struct rebinding rebindings[], size_t rebindings_nel);
```

&emsp;对于 rebindings 中的每次重新绑定，重新绑定对具有指定名称的外部间接符号的引用，以代替调用进程中的每个 image 以及该进程加载的所有 future images 的替换。如果 rebind_functions 被多次调用，则将要重新绑定的符号添加到现有的重新绑定列表中，如果给定符号被多次重新绑定，则后面的重新绑定将优先。（即后面的会覆盖前面的符号绑定）

##### rebind_symbols_image

```c++
/*
 * Rebinds as above, but only in the specified image. The header should point
 * to the mach-o header, the slide should be the slide offset. Others as above.
 */
FISHHOOK_VISIBILITY
int rebind_symbols_image(void *header,
                         intptr_t slide,
                         struct rebinding rebindings[],
                         size_t rebindings_nel);
```

&emsp;重新绑定同上，但是仅在指定的 image 中，`header` 参数指向该 mach-o 文件的 header，`slide` 参数是 slide offset，其他都同 `rebind_symbols` 函数。

&emsp;看上面的 `rebind_symbols` 和 `rebind_symbols_image` 两个函数的定义和注释还是有点迷糊，下面我们直接看 `fishhook.c` 中他们的函数定义。

#### fishhook.c

&emsp;`fishhook.c` 的内容也不多，我们也一起看一下，先看下它涉及的数据结构，然后再沿着函数调用流程看一下它内部每一个函数的执行过程。

&emsp;首先是一组我们比较熟悉的使用 `typedef` 来重命名的结构体，主要用于描述 mach-o 文件中的数据结构。（根据平台不同来使用 32/64 位的定义）

```c++
#ifdef __LP64__

typedef struct mach_header_64 mach_header_t; // ⬅️ 表示 mach-o 的 hader（头部）的结构体
typedef struct segment_command_64 segment_command_t; // ⬅️ 表示 mach-o 中 segment load command 的结构体
typedef struct section_64 section_t; // ⬅️ 表示 mach-o 中 segment 中的 sections 的结构体
typedef struct nlist_64 nlist_t; // ⬅️ 表示 mach-o 中 符号表条目结构。
#define LC_SEGMENT_ARCH_DEPENDENT LC_SEGMENT_64

#else

typedef struct mach_header mach_header_t;
typedef struct segment_command segment_command_t;
typedef struct section section_t;
typedef struct nlist nlist_t;
#define LC_SEGMENT_ARCH_DEPENDENT LC_SEGMENT

#endif

#ifndef SEG_DATA_CONST

#define SEG_DATA_CONST  "__DATA_CONST"

#endif
```

##### rebindings_entry

```c++
struct rebindings_entry {
  struct rebinding *rebindings;
  size_t rebindings_nel;
  struct rebindings_entry *next;
};

static struct rebindings_entry *_rebindings_head;
```

&emsp;`rebindings_entry` 结构体可以理解为一个链表节点的数据结构，`rebindings` 表示当前节点的内容，`rebindings_nel` 是链表长度（节点个数），`next` 是下一个节点。

&emsp;`_rebindings_head` 则是一个静态全局的 `rebindings_entry` 变量，用来记录.....。

##### rebind_symbols

&emsp;下面我们沿着 `rebind_symbols` 函数的调用流程来看下每一个函数的内部实现细节。

```c++
int rebind_symbols(struct rebinding rebindings[], size_t rebindings_nel) {

  // prepend_rebindings 函数的解析在下面 ⬇️⬇️
  int retval = prepend_rebindings(&_rebindings_head, rebindings, rebindings_nel);
  
  // 当 retval 返回 -1 的话表示 prepend_rebindings 函数内部调用 malloc 函数申请空间失败
  if (retval < 0) {
    return retval;
  }
  
  // If this was the first call, register callback for image additions (which is also invoked for existing images, otherwise, just run on existing images
  // 如果这是第一次调用，请注册 image 添加的回调。（第一次调用 rebind_symbols 时 _rebindings_head->next 会为 nil）
  // 这里用了一个 _dyld_register_func_for_add_image 函数，把 _rebind_symbols_for_image 函数注册为 image 添加时的回调。 
  
  // 如果 _rebindings_head->next 不存在时，即是第一次调用 rebind_symbols 函数
  if (!_rebindings_head->next) {
  
    // 把 _rebind_symbols_for_image 函数注册为 image 添加时的回调
    // ⬇️ _dyld_register_func_for_add_image 函数的详细分析在下面
    _dyld_register_func_for_add_image(_rebind_symbols_for_image);
    
  } else {
  
    // 
    uint32_t c = _dyld_image_count();
    
    // 
    for (uint32_t i = 0; i < c; i++) {
      _rebind_symbols_for_image(_dyld_get_image_header(i), _dyld_get_image_vmaddr_slide(i));
    }
    
  }
  
  return retval;
}
```

###### \_dyld_register_func_for_add_image

```c++
/*
 * The following functions allow you to install callbacks which will be called by dyld whenever an image is loaded or unloaded.  
 * During a call to _dyld_register_func_for_add_image() the callback func is called for every existing image. 
 * Later, it is called as each new image is loaded and bound (but initializers not yet run).  
 * The callback registered with _dyld_register_func_for_remove_image() is called after any terminators in an image are run and before the image is un-memory-mapped.
 */
 
extern void _dyld_register_func_for_add_image(void (*func)(const struct mach_header* mh, intptr_t vmaddr_slide))    __OSX_AVAILABLE_STARTING(__MAC_10_1, __IPHONE_2_0);
extern void _dyld_register_func_for_remove_image(void (*func)(const struct mach_header* mh, intptr_t vmaddr_slide)) __OSX_AVAILABLE_STARTING(__MAC_10_1, __IPHONE_2_0);
```

&emsp;以下函数允许你注册回调，每当加载或卸载 image 时，dyld 都会调用这些回调。在调用 `_dyld_register_func_for_add_image()` 期间，为每个现有 image 调用回调函数。稍后，在加载和绑定每个新 image 时调用它（但初始化程序尚未运行）。使用 `_dyld_register_func_for_remove_image()` 注册的回调在运行 image 中的任何终止符之后和 image 取消内存映射（`un-memory-mapped`）之前调用。

&emsp;看到这里我们第一时间大概会想到 `_dyld_objc_notify_register` 函数，他们都是为 `dyld` 注册某种情况的回调函数。这里的 `_dyld_register_func_for_add_image` 注册的回调函数有两个调用时机：

1. 调用完 `_dyld_register_func_for_add_image` 以后会直接遍历当前状态为 `image->getState() >= dyld_image_state_bound && image->getState() < dyld_image_state_terminated` 的 image 调用回调函数。
2. 把回调函数添加到 `sAddImageCallbacks`（它是一个静态全局的 `std::vector<ImageCallback>` 变量） 中，当后续有新 image 添加时进行调用。

&emsp;（在 `dyld` 源码中都能看到这些函数的定义，这里就不贴源码了。）

###### \_dyld_image_count \_dyld_get_image_header \_dyld_get_image_vmaddr_slide

&emsp;以下函数允许你遍历所有加载的 images。这不是线程安全的操作。另一个线程可以在迭代过程中添加或删除 image。这些例程的许多用途都可以通过调用 `dladdr()` 来代替，`dladdr()` 将返回 `mach_header` 和 image 名称，给定 image 中的地址。 `dladdr()` 是线程安全的。

```c++
/*
 * The following functions allow you to iterate through all loaded images.  
 * This is not a thread safe operation.  Another thread can add or remove an image during the iteration.  
 *
 * Many uses of these routines can be replace by a call to dladdr() which will return the mach_header and name of an image, given an address in the image. dladdr() is thread safe.
 */
 extern uint32_t                    _dyld_image_count(void)                              __OSX_AVAILABLE_STARTING(__MAC_10_1, __IPHONE_2_0);
 extern const struct mach_header*   _dyld_get_image_header(uint32_t image_index)         __OSX_AVAILABLE_STARTING(__MAC_10_1, __IPHONE_2_0);
 extern intptr_t                    _dyld_get_image_vmaddr_slide(uint32_t image_index) 
```

&emsp;

##### prepend_rebindings

```c++
static int prepend_rebindings(struct rebindings_entry **rebindings_head,
                              struct rebinding rebindings[],
                              size_t nel) {
                              
  // 调用 malloc 函数申请 24 个字节的空间，并把首地址强转为 struct rebindings_entry 指针。（sizeof(struct rebindings_entry) 值为 24）                              
  struct rebindings_entry *new_entry = (struct rebindings_entry *) malloc(sizeof(struct rebindings_entry));
  
  // 如果 malloc 申请空间失败，则返回 -1
  if (!new_entry) {
    return -1;
  }
  
  // 调用 malloc 函数申请空间，并把起始地址赋值给 new_entry 的 rebinings。（这里申请空间的长度是 rebinding 结构体的内存空间占用长度乘以总共的 rebinding 结构体的个数）
  new_entry->rebindings = (struct rebinding *) malloc(sizeof(struct rebinding) * nel);
  
  // 同上，如果 malloc 申请空间失败，则返回 -1（返回之前要把上面 malloc 申请的空间通过 new_entry 指针进行释放）
  if (!new_entry->rebindings) {
  
    // 释放 new_entry 
    free(new_entry);
    
    return -1;
  }
  
  // 调用 memcpy 函数把入参传入的 rebindings 数组中的所有 rebinding 元素逐字节的拷贝到 new_entry 的 rebindings 成员变量中去   
  memcpy(new_entry->rebindings, rebindings, sizeof(struct rebinding) * nel);
  
  // nel 是入参 rebindings 数组的长度，赋值给 new_entry 的 rebindings_nel 成员变量，也表示了当前 new_entry 中保存了多少个 rebinding 结构体 
  new_entry->rebindings_nel = nel;
  
  // new_entry 的 next 成员变量指向，第一个参数 *rebindings_head 
  new_entry->next = *rebindings_head;
  
  // 这里把 new_entry 赋值给了第一个参数。
  //（这里有个点，从前面调用可知，rebindings_head 参数其实是前面说过的 static struct rebindings_entry *_rebindings_head; 这个全局变量，
  // 然后 prepend_rebindings 函数每调用一次就构建一个 struct rebindings_entry *new_entry，然后把 new_entry 添加到 rebindings_head 这个链表的头部，
  // 然后再通过 *rebindings_head = new_entry; 保证 rebindings_head 一直是链表的头） 
  *rebindings_head = new_entry;
  
  return 0;
}
```

&emsp;`prepend_rebindings` 函数的内容看完了，它的内部就是构建一个 `struct rebindings_entry *new_entry` 变量，然后把入参 `struct rebinding rebindings[]` 数组中的元素直接复制到 `new_entry` 的 `struct rebinding *rebindings;` 中，然后把入参 `size_t nel` 赋值给 `new_entry` 的 `size_t rebindings_nel;`（`nel` 是 `rebindings` 数组的长度），然后最后最重要的是 `new_entry` 会被拼接到 `rebindings_head` 链表到头部，并更新 `rebindings_head` 的值，保证它还是当前链表的头部。






























## 参考链接
**参考链接:🔗**
+ [iOS逆向 RSA理论](https://juejin.cn/post/6844903989666906125)
+ [iOS逆向 HOOK原理之fishhook](https://juejin.cn/post/6845166890772332552)
+ [LXDZombieSniffer](https://github.com/sindrilin/LXDZombieSniffer)
+ [SDMagicHook](https://github.com/cloverapp1/SDMagicHook)
