# iOS App 启动优化(八)：fishhook 源码逐行分析

## 理解 fishhook 实现原理所需要的一些前置知识点

&emsp;首先我们在学习 fishhook 实现原理之前一定要有 mach-o 的知识，可参考之前的文章：[iOS APP 启动优化(一)：ipa 包和 Mach-O( Mach Object File Format)概述](https://juejin.cn/post/6952503696945070116)。

&emsp;[facebook/fishhook](https://github.com/facebook/fishhook) 是 Facebook 开源的一个 iOS 库，它可以动态修改进程（进程：mach-o 格式的可执行文件在 iOS 上运行时）中 `(__DATA, __got)`、 `(__DATA, __la_symbol_ptr)`、`(__DATA, __nl_symbol_ptr)` Section 中符号指针（Non-Lazy Symbol Pointers 和 Lazy Symbol Pointers 类型的符号指针），已知这些 Section 中存放的内容是 Non-Lazy Symbol Pointers 和 Lazy Symbol Pointers，而 fishhook 修改的便是其中 Symbol Pointer 的指向（指向我们自定义的函数或者其他任意函数）。

### 不知道有用还是无用的知识点

&emsp;关于 Non-Lazy Symbol Pointers，目前可见的的在 `(__DATA, __got)`、`(__DATA, __nl_symbol_ptr)` 两个 Section 中存放，其中 `(__DATA, __nl_symbol_ptr)` 区在 `main.m` 函数类似如下内容时会在可执行文件（mach-o 二进制文件）中存在：

```c++
#import <stdio.h>

int main(int argc, char * argv[]) {
    printf("%s\n", "hello world");
    printf("%s\n", "hello desgard");
    return 0;
}
```

&emsp;而我们 OC 项目的话，可执行文件中仅存在 `(__DATA, __got)` 这个分区用来存放 Non-Lazy Symbol Pointers。这些分区中的内容是 Non-Lazy Symbol Pointers，其实是通过它们对应的 Load command 的 `flags` 字段中的 `S_NON_LAZY_SYMBOL_POINTERS` 来标记识别的。作为对比 `(__DATA, __la_symbol_ptr)` 对应的 Load command 的 `flags` 字段中是 `S_LAZY_SYMBOL_POINTERS` 来标记的，预示了 `(__DATA, __la_symbol_ptr)` 中存放的是 Lazy Symbol Pointers。

### fishhook 的局限性

&emsp;这里用的标题虽然是 “局限性”，不过也不能称之为局限性，毕竟每种工具都有其适用范围，这里大概应该称之为 fishhook 的使用范围，或者说是 fishhook 都能 hook 哪些函数。

&emsp;fishhook 能 hook 的是 Non-Lazy Symbol Pointers 和 Lazy Symbol Pointers，已知 `(__DATA, __got)`、`(__DATA, __la_symbol_ptr)`、`(__DATA, __nl_symbol_ptr)` Section 中存放的 Symbol Pointer 符合要求，通过前面 Mach-0 知识的学习我们已知这些区中仅会存放我们引入到项目中的系统动态库的 C 函数，而我们自己的自定义函数（C/OC）是没有对应的 Symbol Pointer 保存在这些区中的，它们的定义是存储在 `(__TEXT, __text)` Section 中，然后有对应的符号在 Symbol Table 中，那么看到这里我们自然也想到了 fishhook 的局限性，它仅仅能用来 hook 系统动态库中的 C 函数。

~~&emsp;前面介绍了 fishhook 是用来 hook C 函数的，这里其实还有一个前提，就是它只能 hook 系统的 C 函数，并不能 hook 我们自己写的自定义 C 函数。C 函数是静态的，在编译时就已经确定了函数地址（函数实现地址在 mach-o 本地文件中），而系统的 C 函数则存在着动态的部分，那么为什么系统级别的 C 函数存在着的动态的部分是什么呢？这就要说到 PIC（position-independent code） 技术，又叫做 位置独立代码/位置无关代码，是为了系统 C 函数在编译时期能够确认一个地址的一种技术手段。~~

~~&emsp;编译时在 mach-o 文件中预留一段空间 -- 符号表（`__DATA` 段中），dyld 把应用加载到内存中时（此时在 load command 中会依赖 Foundation），在符号表中找到了 `NSLog` 函数，就会进行链接绑定 -- 将 Foundation 中 NSLog 的真实地址赋值到 `__DATA` 段的 `NSLog` 符号上。而自定义的 C 函数是不会生成符号表的，直接就是一个函数地址，所以 fishhook 的局限性就在于只有符号表内的符号才可以进行 hook（重新绑定符号）。~~

## fishhook 解读

### fishhook 官方介绍

&emsp;首先我们看一下官方的描述：

&emsp;A library that enables dynamically rebinding symbols in Mach-O binaries running on iOS. 在 iOS 上运行的 Mach-O 二进制文件中启用 **动态重新绑定符号** 的库。（仅限于系统动态库中的 C 函数）

&emsp;fishhook 是一个非常简单的库，它支持在模拟器和设备上的 iOS 系统中运行的 Mach-O 二进制文件中动态地重新绑定符号（仅限于系统动态库中的 C 函数）。这提供了类似于在 OS X 上使用 [DYLD_INTERPOSE](https://opensource.apple.com/source/dyld/dyld-210.2.3/include/mach-o/dyld-interposing.h) 的功能（下面的示例代码中， fishhook 的使用方式确实和它很像）。在 Facebook 上，我们发现它是一种很有用的方法，可以在 `libSystem` 中钩住调用（hook calls）以进行调试/跟踪（debugging/tracing）（for example, auditing for double-close issues with file descriptors）。

&emsp;这里我们摘录一下在 OS X 上使用的 `DYLD_INTERPOSE`：

```c++
#if !defined(_DYLD_INTERPOSING_H_)
#define _DYLD_INTERPOSING_H_

/*
 *  Example:
 *
 *  static
 *  int
 *  my_open(const char* path, int flags, mode_t mode)
 *  {
 *    int value;
 *
 *    // do stuff before open (including changing the arguments)
 *
 *    value = open(path, flags, mode);
 *
 *    // do stuff after open (including changing the return value(s))
 *
 *    return value;
 *  }
 *  DYLD_INTERPOSE(my_open, open)
 */

#define DYLD_INTERPOSE(_replacement,_replacee) \
   __attribute__((used)) static struct{ const void* replacement; const void* replacee; } _interpose_##_replacee \
            __attribute__ ((section ("__DATA,__interpose"))) = { (const void*)(unsigned long)&_replacement, (const void*)(unsigned long)&_replacee };

#endif
```

### fishhook 使用方式

&emsp;fishhook 的使用方式非常简单，我们只需要把 `fishhook.h/fishhook.c` 文件拖入我们的项目中，然后就可以按如下示例代码重新绑定符号：

```c++
#import <dlfcn.h>

#import <UIKit/UIKit.h>

#import "AppDelegate.h"
#import "fishhook.h"

// 这里声明两个静态全局的函数指针变量，分别用来记录系统原始的 close 和 open 函数的地址
static int (*orig_close)(int);
static int (*orig_open)(const char *, int, ...);
 
int my_close(int fd) {
  printf("🤯🤯🤯 Calling real close(%d)\n", fd);
  
  // 经过下面 main 函数中的 rebind_symbols 调用后，orig_close 这个函数指针指向的就是系统原始的 close 函数，即这里便是调用系统原始的 close 函数。 
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
    
    // 同上，orig_open 这个函数指针指向系统原始的 open 函数
    return orig_open(path, oflag, mode);
  } else {
    printf("🤯🤯🤯 Calling real open('%s', %d)\n", path, oflag);
    
    // 同上，orig_open 这个函数指针指向系统原始的 open 函数
    return orig_open(path, oflag, mode);
  }
}
 
int main(int argc, char * argv[])
{
  @autoreleasepool {
    
    // 把指向 close 和 open 函数的 Symbol Pointer 的指向替换为我们自定义的 my_close 和 my_open 函数，
    // 并且使用 orig_close 和 orig_open 两个 静态全局函数指针变量 记录系统的 close 和 open 函数地址，
    // 那样下面的 open 和 close 函数调用时，就会执行我们自定义的 my_open 和 my_close 函数，然后 my_open 和 my_close 内部又通过 orig_open 和 orig_close 函数指针来调用系统原始的 open 和 close 函数。
    
    rebind_symbols((struct rebinding[2]){{"close", my_close, (void *)&orig_close}, {"open", my_open, (void *)&orig_open}}, 2);
 
    // Open our own binary and print out first 4 bytes (which is the same for all Mach-O binaries on a given architecture)
    // 还记得 argv[0] 的，它就是我们当前进程的可执行文件的本地路径，然后直接读取它的前 4 个字节的内容，
    // 即对应于 mach_header 结构体中 magic 魔数，用来表示当前的 mach-o 格式的文件是一个什么类型的文件，（Mach-O Type）
    // 如果我们对之前学习 mach-o 时还有印象的话，那么这里第一时间应该想到的就是："64 位可执行文件"。
    
    printf("➡️➡️➡️ %s \n", argv[0]);
    
    // 打开文件
    int fd = open(argv[0], O_RDONLY);
    
    // 读取前 4 个字节放在 magic_number 变量中
    uint32_t magic_number = 0;
    read(fd, &magic_number, 4);
    
    // 这里就会打印出我们熟悉的 feedfacf 魔数
    printf("🤯🤯🤯 Mach-O Magic Number: %x \n", magic_number);
    
    // 关闭文件
    close(fd);
 
    return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
  }
}
```

&emsp;运行上面的代码，我们的控制台就会看到如下输出：

```c++
// argv[0] 即我们当前进程的可执行文件的本地路径
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

### 通过 LLDB 调试 Lazy Symbol Pointer 绑定过程以及 fishhook 进行 hook 的结果 

&emsp;下面我们通过 LLDB 调试一下 `(__DATA, __la_symbol_ptr)` 中指向 `_open` 这个符号的 Lazy Symbol Pointer 被绑定的过程，以及经过 fishhook 处理后这个 Lazy Symbol Pointer 指向发生变化的过程。

&emsp;下面我们捋一下 `_open` 指针的一系列指向，这样不仅可以帮助我们更好的理解 mach-o，同时还能提高我们对指针的认识！（开始之前我以为自己对指针已经极其熟悉了，妹想到，自己还是太年轻了！）

![截屏2021-08-03 下午10.30.13.png](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/d58de98c44404eceb844743f8140977f~tplv-k3u1fbpfcp-watermark.image)

&emsp;首先是第一张图，位于 `(__DATA, __la_symbol_ptr)` 中的一个指针（Indirect Pointer 间接指针），其距 mach-o 二进制文件起始偏移 `0x000C0E0` 个字节，假设 mach-o 二进制文件的起始地址是 `0x100000000`，即此间接指针的地址是：`0x10000C0E0`，然后这个地址下连续 8 个字节内存放的值是 `0x000000010000609E`，到这里可以理解为：**一个起始地址是 0x10000C0E0 的指针，指向 0x10000609E 这个地址！**，那到这里有同学可能会问了，那这个 `_open` 是哪里来的呢？（指针的本质就是一段 8 个字节的空间，里面存了一些内容，正常情况下都是存一个地址，那这个地址怎么就和 `_open` 字符串联系起来了呢？）不急，我们下面慢慢看。首先我们把 MachOView 上翻一下，找找 `0x10000609E` 这个位置放的是什么，我们看到 `0x10000609E` 这个位置是在 `(__TEXT, __stub_helper)` 区中，到这里我们也验证了一个结论: `(__DATA, __la_symbol_ptr)` 区中的 Symbol Pointer 起始时是指向 `(__TEXT, __stub_helper)` 区的。

![截屏2021-08-03 下午10.58.28.png](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/0b602e1db1d346aba624e72bd209ef45~tplv-k3u1fbpfcp-watermark.image)

&emsp;`(__TEXT, __stub_helper)` 区的知识点，我们暂时不往下延伸，它其实超重要的，我们暂时留在后面分析。这里我们先看上面一个问题：`0x10000C0E0` 这个地址的指针是怎么就和 `_open` 连接起来的呢？这里我们首先要看一下 `LC_DYSYMTAB` 这个 Load Command：

![截屏2021-08-03 下午11.15.20.png](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/aa9eb57435b9485a9af38daea49e85c9~tplv-k3u1fbpfcp-watermark.image)

&emsp;从图片中可以看到 `LC_DYSYMTAB` 这个 Loac Command 的 IndSym Table Offset 的值是 `0x00011D20`，那么可以计算得出 Dynamic Symbol Table 的位置应该是在 `0x100011d20` 处。果然我们从上往下看 mach-o 二进制文件，基本在最底部看到了 Dynamic Symbol Table，它的 Offset 的值正是 `0x0011D20` 和上图中看到的值一致，且 Dynamic Symbol Table 中存放的全都是 Indirect Symbols，此时我们可以在里面一通找，我们要找的便是位于 `(__DATA, __la_symbol_ptr)` 区中，且 Indirect Address 是 `0x10000COE0` 的一个 Symbol，如下图，我们真的找到了，并且我们又看到了我们熟悉的 `_open` 字符，我们看到此 Symbol 在 Symbol Table 中的 Index 是 `0x0000014C (#332)`。（Indirect Address 的值正是这些 Symbol 相对 mach-o 二进制文件起点的偏移值 + mach-o 二进制文件的起点值，也就是这些 Symbol 所在的 Section 的位置）

![截屏2021-08-03 下午11.27.30.png](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/dfa7382ec5874f83b08d12790872306f~tplv-k3u1fbpfcp-watermark.image)

&emsp;好了，这里我们也不墨迹，直接找到 Symbol Table 的索引 332 处，可发现这里是我们的 `_open` 符号的内容。那么这里的 `_open` 字符串到底来自哪里呢？这里我们也从图中看到了，Symbol Table 的 #332 处的这个符号的名字字符串在 String Table 的 Index 是 #1171。 

![截屏2021-08-03 下午11.35.40.png](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/34e535cdea3b4da7befde5384d0de2e0~tplv-k3u1fbpfcp-watermark.image)

&emsp;好了，这里我们也不啰嗦了，直接找到 String Table 的 #1171 这个索引处：

![截屏2021-08-03 下午11.41.33.png](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/48011037acf7437197a7cb72b7a4dd01~tplv-k3u1fbpfcp-watermark.image)

&emsp;舒服了，绕了一大圈，我们终于找到了位于 `(__DATA, __la_symbol_ptr)` 中的 `0x10000C0E0` 这个指针指向的是 `_open` 这个符号！而 `_open` 这个字符串也正是来自 String Table。

&emsp;好了，`_open` 这个符号指针的 `_open` 字符串的来源我们搞清楚了，下面我们接着看 `_open` 这个符号指针怎么和符号绑定起来的。

&emsp;首先在 `__DATA, __la_symbol_ptr` 中有一个指向 `_open` 的懒加载符号指针。当我们使用 open 函数时 dyld 才会对 open 这个符号指针进行正确的绑定，还有对符号表中的 `_open` 地址进行正确的绑定，我们已知 `_open` 函数是来自系统的动态库。

> &emsp;这里我们先看一下 `image` 指令的知识点，`image list` 指令可列出当前进程在内存中的地址以及本地路径，以及当前进程所依赖的 shared library image 在内存中的地址和本地路径。
  
  ```c++
  (lldb) image
       Commands for accessing information for one or more target modules.

  Syntax: target modules <sub-command> ...

  The following subcommands are supported:

        add          -- Add a new module to the current target's modules.
        dump         -- Commands for dumping information about one or more target
                        modules.
        list         -- List current executable and dependent shared library
                        images.
        load         -- Set the load addresses for one or more sections in a
                        target module.
        lookup       -- Look up information within executable and dependent
                        shared library images.
        search-paths -- Commands for managing module search paths for a target.
        show-unwind  -- Show synthesized unwind instructions for a function.

  For more help on any particular subcommand, type 'help <command> <subcommand>'.
  ```

&emsp;为了便于我们验证 `hook` 的过程，这里我们对上面的示例代码做一下调整，用如下代码作为演示使用。

```c++
int main(int argc, char * argv[]) {
    
    // ⬇️ 在此行打一个断点 
    NSLog(@"🎃🎃🎃 %p", open);
    
    int fd = open(argv[0], O_RDONLY);
    uint32_t magic_number = 0;
    read(fd, &magic_number, 4);
    printf("🤯🤯🤯 %s Mach-O Magic Number: %x \n", __func__, magic_number);
    close(fd);
    
    rebind_symbols((struct rebinding[2]){{"close", my_close, (void *)&orig_close}, {"open", my_open, (void *)&orig_open}}, 2);
    
    NSString * appDelegateClassName;
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
        appDelegateClassName = NSStringFromClass([AppDelegate class]);
    }
    
    return UIApplicationMain(argc, argv, nil, appDelegateClassName);
}
```

&emsp;执行上面的代码，并进入第一个断点：

1. 通过 `image list` 可打印当前进程和其依赖的 shared library image 镜像被加载到内存的地址。而第一个便是我们当前进程的内存首地址：`0x000000010b35f000`。（也发现有时候第一个是 `/usr/lib/dyld `，这里我们要看准后面的路径，我们需要的仅是当前进程在内存中的首地址。）

```c++
[  0] BAAF897A-1463-3D9E-BDFE-EA61525D3435 0x000000010b35f000 /Users/hmc/Library/Developer/Xcode/DerivedData/Test_ipa_simple-hfabjfhaswcxjleagxtdjjvbnnhi/Build/Products/Debug-iphonesimulator/Test_ipa_simple.app/Test_ipa_simple 
```

![截屏2021-07-28 下午9.57.03.png](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/9204fe2073fa4701aa30457eafa04091~tplv-k3u1fbpfcp-watermark.image)

2. 右键 Products 文件夹中的 Test_ipa_simple.app 点击 Show in Finder，然后选中 Test_ipa_simple.app 右键显示包内容，然后用 MachOView 打开 Test_ipa_simple 二进制文件，可在此 mach-o 文件的 `Section64(__DATA,__la_symbol_ptr)` 区中 `Lazy Symbol Pointers` 中看到 `open` 符号指针相对 mach-o 二进制文件的起点为偏移量：`0x000C0D8`，然后通过当前进程在内存中的首地址 + `open` 符号指针在二进制文件的偏移量取得 `open` 符号指针在内存的地址：`0x000000010b35f000` + `0x000C0D8` = `0x10B36B0D8`。

![截屏2021-07-28 下午10.03.40.png](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/e7af7a526417469eafa174c91a8244c9~tplv-k3u1fbpfcp-watermark.image)

3. 通过 `memory read` 直接读取我们上面计算得出的 `open` 符号指针在内存中的地址（即读出该指针地址内保存的值，也即为该指针的指向），已知 iOS 是小端模式，所以这里我们需要把该指针的 8 个字节中的内容进行倒着读，即为：`0x010b3650d0`。

![截屏2021-07-28 下午10.09.34.png](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/5a5de5494903476f90e86e5b75ea0486~tplv-k3u1fbpfcp-watermark.image)

4. 下面通过 `dis -s` 查看上面 `0x010b3650d0` 地址中的汇编代码。由于 `open` 是一个懒加载符号，且此时 `open` 函数还没有被使用过，此时并没有被绑定。（当前应该是 `(__TEXT, __stub_helper)` 的一个位置）

![截屏2021-07-28 下午10.30.28.png](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/6eb2ac4031b9468ca1f9a674f1a73554~tplv-k3u1fbpfcp-watermark.image)

5. 然后我们单步向下执行，通过 `NSLog(@"🎃🎃🎃 %p", open);` 语句后我们打印出了 `open` 函数指针所指向的地址：`0x7fff61133e65` 这个地址便是我们当前进程引用到的系统的 `open` 函数的真正地址，然后我们可以通过 `dis -s 0x7fff61133e65` 查看该地址的汇编代码来进行验证，可看到此地址下正是 `libsystem_kernel.dylib open` 函数。

![截屏2021-07-28 下午10.38.30.png](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/7c964b158e584b29b8d5408ff4072555~tplv-k3u1fbpfcp-watermark.image)

6. 那么此时我们再执行一下 `memory read 0x10B36B0D8` 查看一下我们的 `open` 符号指针内指向的内容有没有发生变化，通过打印发现，它依然指向 `0x010b3650d0`，即我们的 `open` 符号指针的指向还是没有变化。（它的正确的指向应该是 `0x7fff61133e65`）

![截屏2021-07-28 下午10.46.04.png](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/912fe686ba614a34b64ceef9b09f56fb~tplv-k3u1fbpfcp-watermark.image)

7. 那么到这里大家可能会有疑问了，这个 `open` 符号指针什么时候才能被绑定呢、才能真正指向系统的 `open` 函数呢？上面我们也已经看到了系统的 `open` 函数的地址是 `0x7fff61133e65`，即当我们执行 `memory read 0x10B36B0D8` 指令输出 `0x7fff61133e65` 的时候，就是 `open` 符号指针被正确绑定了。看上面的代码区我们看到当前我们的单步执行到 `int fd = open(argv[0], O_RDONLY);` 此时我们再往下单步执行一下，然后再执行 `memory read 0x10B36B0D8` 指令便可看到 `0x10B36B0D8` 地址中保存的正是 `0x7fff61133e65`，即当我们第一次调用 `open` 函数的时候，`Section64(__DATA,__la_symbol_ptr)` 的 `Lazy Symbol Pointers` 中的 `open` 符号指针才真正指向系统的 `open` 函数。

![截屏2021-07-28 下午11.05.28.png](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/e2f11823cff54136bf501a4d6aee1a35~tplv-k3u1fbpfcp-watermark.image)

8. 那么看到这里我们的 `open` 符号指针就被正确绑定了，那也该轮到我们的 fishhook 出场了，看它怎么 hook open 函数，把 `open` 符号指针的指向给修改了，指向我们自己的函数。我们连续单步往下执行，直到执行到 `rebind_symbols` 下面一行，然后我们再执行 `memory read 0x10B36B0D8` 指令，可看到我们的 `open` 符号指针的指向变化了，现在指向了 `0x010b363f60`，然后我们再执行 `dis -s 0x010b363f60` 转换为汇编代码，可看到此时 `open` 符号指针指向了 `Test_ipa_simple my_open` 函数，即此时 `open` 符号指针指向了我们自定义的 `my_open` 函数。

![截屏2021-07-28 下午11.16.00.png](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/0e8677b70b71454bad68acc2fcc271f9~tplv-k3u1fbpfcp-watermark.image)

9. 此时还有一个点就是我们的系统的 `open` 函数哪去了呢？即当前我们的 `orig_open` 函数指针正指向我们系统的 `open` 函数，可看到当前 `orig_open` 函数指针指向的地址正是：`0x00007fff61133e65`，即为我们上面看到的系统的 `open` 函数的地址。即此时通过 `orig_open` 函数指针便可调用我们系统的 `open` 函数。

![截屏2021-07-28 下午11.21.14.png](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/086e922516c649efa7867b36ddc9c8dc~tplv-k3u1fbpfcp-watermark.image)

&emsp;这样我们就通过 LLDB 把 `_open` 这个 Symbol Pointer 绑定过程和 fishhook 实现 hook 的结果看完了，具体的执行细节，我们在下面的章节一一分析。

### Symbol Pointer 到 Symbol 的过程

&emsp;在下面看 fishhook 内部是怎么工作之前，我们首先看一个其它的知识点。我们在 `main` 函数中打印 `NSLog` 函数的地址（`NSLog(@"🎃🎃🎃 %p", NSLog);` 控制台输出：`🎃🎃🎃 0x7fff20805d0d`），我们多次打印，或者删除 APP 后重新运行打印，可看到 `NSLog` 函数的地址一直都是固定的。（之所以不变，是因为 `NSLog` 函数是来自系统中名为 `Foundation` 的共享动态库，而 `Foundation` 这个系统的共享动态库位于共享缓存中，它可以被多个进程所引用，所以即使在另外一个进程中我们打印 `NSLog` 函数的地址，可以发现打印的地址都是一样的。）

&emsp;下面我们借助 [fangshufeng/MachOView](https://github.com/fangshufeng/MachOView) 来直面 mach-o 文件，在其中查找函数实现地址，这里以 `NSLog` 函数为例。

&emsp;下面我们根据字符串对应在符号表中的指针，找到其在共享库的函数实现。

1. 在 `Section64(__DATA, __la_symbol_ptr)` 的 `Lazy Symbol Pointers` 中， `_NSLog` 位于第一个，下标为 0。

![截屏2021-07-25 下午8.48.49.png](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/136ba2d1e68d41228829f3bc0356ed14~tplv-k3u1fbpfcp-watermark.image)

2. 在 `Dynamic Symbol Table` 的 `Indirect Symbols` 中，`_NSLog` 也是位于第一个，不过它是 `(__TEXT, __stubs)` 区中的 `_NSLog` Symbol Pointer，然后我们可以顺序看  `Dynamic Symbol Table` 中的 `Indirect Symbol`，可发现这里存放有 `(__DATA, __got)` Section 中的 `_NSLog` Symbol Pointer，然后最后才是 `(__DATA, __la_symbol_ptr)` Section 中的 `_NSLog` Symbol Pointer，然后它们三者在 `Indirect Symbols` 中的 Data 的值都是：`0x111`（十进制是：273），即它们三者在 Symbol Table 中的索引是一样的，即它们三者都指向 Symbol Table 中的同一个 Symbol。 

![截屏2021-07-25 下午8.51.17.png](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/629957a693744c7e9de6ac2d6e285d65~tplv-k3u1fbpfcp-watermark.image)

3. 然后我们拿着上面得到的索引 273 在 `Symbol Table` 的 `Symbols` 中查找，我们看着 Offset 的值一直向上滚动，直到滚动到 #273，我们便找到了 `_NSLog`，此时我们看到 `_NSLog` 条目的 Data 的值是：`Ox000000E1`，此值对应了 `_NSLog` 在 `String Table Index` 中的偏移值。

![截屏2021-07-25 下午9.08.35.png](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/c366c4e8c21f40f18e81fe0f374c9380~tplv-k3u1fbpfcp-watermark.image)

4. 最后在 `String Table` 中计算表头（`Ox00011D00`）+ 偏移量（`Ox000000E1`）得到：`Ox00011DE1`，即找到了 `NSLog` 的函数名字符串在 `String Table` 的位置。

![截屏2021-07-25 下午9.27.58.png](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/bdd80417d7e34900afabb3fdeca4b4ee~tplv-k3u1fbpfcp-watermark.image)

&emsp;以上查找 `NSLog` 符号的流程总结下来便是：`Section64(__DATA, __la_symbol_ptr)`：`Lazy Symbol Pointers` -> `Dynamic Symbol Table`：`Indirect Symbols`（`Data` 转换为 10 进制作为一个下标使用）-> `Symbol Table`：`Symbols`（`Data` 作为一个偏移量使用）-> `String Table`，请对此过程保持一个印象，下面的 fishhook 的实现代码中，对 rebinding 中 name 的查找的过程和此过程完全一致。（如果理解了此查找过程，那么对下面 fishhook 给出的示例图便可以一目了然！）

&emsp;下面我们继续学习 fishhook 看看它是如何工作的。

### fishhook How it works

&emsp;下面是官方文档中对 fishhook 进行 hook 过程的一个介绍：

&emsp;`dyld` 通过更新 Mach-O 二进制文件的 `__DATA` 段的特定部分中的指针来绑定 lazy 和 non-lazy 符号。 fishhook 通过确定传递给 `rebind_symbols` 的每个符号名称的更新位置，然后写出相应的替换来重新绑定这些符号。

&emsp;对于给定的 image 镜像，`__DATA` 段可能包含两个与动态符号绑定（dynamic symbol bindings）相关的 sections：`(__DATA, __nl_symbol_ptr)` 和 `(__DATA, __la_symbol_ptr)`。 `__nl_symbol_ptr` 是一个指向 non-lazily 绑定数据的指针数组（这些是在加载 library 时绑定的），而 `__la_symbol_ptr` 是一个指向导入函数（imported functions ）的指针数组，通常在第一次调用该符号时由名为 `dyld_stub_binder` 的例程填充（也可以告诉 `dyld` 在启动时绑定这些）。为了在这些 sections 之一中找到与特定位置相对应的符号名称，我们必须跳过几个间接层。对于两个相关的 sections，section headers（`struct section` 来自 <mach-o/loader.h>）提供了一个偏移量（在 `reserved1` 字段中）到所谓的间接符号表中。位于二进制文件的 `__LINKEDIT` 段中的间接符号表只是符号表（也在 `__LINKEDIT` 中）的索引数组，其顺序与 non-lazy 和 lazy 符号 sections 中的指针的顺序相同。因此，给定的 `struct section nl_symbol_ptr`，该 section 中第一个地址的符号表中对应的索引是 `indirect_symbol_table[nl_symbol_ptr->reserved1]`。符号表本身是一个 `struct nlist` 数组（参见 `<mach-o/nlist.h>`），每个 `nlist` 都包含一个指向 `__LINKEDIT` 中字符串表的索引，其中存储了实际的符号名称。因此，对于每个指针 `__nl_symbol_ptr` 和 `__la_symbol_ptr`，我们能够找到相应的符号，然后找到相应的字符串与请求的符号名称进行比较，如果有匹配项，我们将替换 section 中的指针。

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

&emsp;`rebinding` 结构体是用来定义替换函数使用的一个数据结构。`name` 成员变量表示要 Hook 的函数名称（ C 字符串），`replacement` 指针用来指定新的函数地址，即把 `name` 对应的函数的函数地址替换为 `replacement`（C 函数的名称就是一个函数指针，静态语言编译时就已确定。）`replaced` 是一个二维指针，之所以是这样是为了在函数内部直接修改外部指针变量的指向，用在 `rebinding` 结构体中就是为了记录 `name` 对应的原始函数的地址。 对应上面示例代码段的 `closeVariable` 变量，当把其作为 `rebind_symbols` 函数的参数执行时，找到 `close`（它是一个系统函数）符号指针，把它指向符号表中的 `my_close` 函数，`orig_close` 则是用来记录 `close` 函数的原始地址。

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

&emsp;重新绑定同上，但是仅在指定的 image 中，`header` 参数指向该 mach-o 文件的 header，`slide` 参数指定 image 在虚拟内存中的 slide offset，其他都同 `rebind_symbols` 函数。

&emsp;看上面的 `rebind_symbols` 和 `rebind_symbols_image` 两个函数的定义和注释还是有点迷糊，下面我们直接看 `fishhook.c` 中他们的函数定义。

#### fishhook.c

&emsp;`fishhook.c` 的内容也不多，我们一起看一下，先看下它涉及的数据结构，然后再沿着函数调用流程看一下它内部每一个函数的执行过程。

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

&emsp;`rebindings_entry` 结构体作用为一个单向链表节点的数据结构使用（链表的内容就是我们上面的 `rebinding` 结构体），`rebindings` 表示当前节点的内容，`rebindings_nel` 是当前链表长度（节点个数），`next` 指向下一个节点。

&emsp;`_rebindings_head` 则是一个静态全局的 `rebindings_entry` 指针，它作为一个链表的头使用，而链表的每个节点中保存的内容就是我们每次调用 `rebind_symbols` 函数传进来的 `rebinding` 数组，调用一次 `rebind_symbols` 函数就是构建一个 `rebindings_entry` 节点，且这个新节点不是拼接在链表尾部的，这个新节点的 `next` 直接指向 `_rebindings_head`，然后把 `_rebindings_head` 更新为这个新节点的地址，即每次有新节点进来它都是直接拼接在原始链表头部的。

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
  
    // 取得由 dyld 映射的当前 image 的数量
    uint32_t c = _dyld_image_count();
    
    // 遍历当前的所有 image，把 _rebindings_head 链表中的所有的节点中保存的 rebinding 在 image 的 segment load command 中进行查找绑定
    for (uint32_t i = 0; i < c; i++) {
      // ⬇️ _rebind_symbols_for_image 函数在下面的详细分析（在当前 image 中进行查找绑定）
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

&emsp;以下函数允许你注册回调，每当加载或卸载 image 时，dyld 都会调用这些回调。在调用 `_dyld_register_func_for_add_image()` 期间，也会为每个现有 image 调用回调函数。稍后，在加载和绑定每个新 image 时也会调用之前注册的回调函数（但初始化程序尚未运行）。使用 `_dyld_register_func_for_remove_image()` 注册的回调在运行 image 中的任何终止符之后和 image 取消内存映射（`un-memory-mapped`）之前调用。

&emsp;看到这里我们第一时间大概会想到 `_dyld_objc_notify_register` 函数，他们都是为 `dyld` 注册某种情况的回调函数。这里的 `_dyld_register_func_for_add_image` 注册的回调函数有两个调用时机：

1. 调用完 `_dyld_register_func_for_add_image` 以后会直接遍历当前状态为 `image->getState() >= dyld_image_state_bound && image->getState() < dyld_image_state_terminated` 的 image 调用回调函数。
2. 当后续有新 image 添加到 `sAddImageCallbacks`（它是一个静态全局的 `std::vector<ImageCallback>` 变量） 时会调用回调函数。

&emsp;（在 `dyld` 源码中都能看到这些函数的定义，这里就不贴源码了。）

###### \_dyld_image_count \_dyld_get_image_header \_dyld_get_image_vmaddr_slide

&emsp;以下函数帮助你遍历所有加载的 images。这不是线程安全的操作，另一个线程可能在本线程迭代过程中添加或删除 image。这些例程的许多用途都可以通过调用 `dladdr()` 来代替，`dladdr()` 将返回 `mach_header` 和 image 名称，给定 image 中的地址。 `dladdr()` 是线程安全的。

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

&emsp;`_dyld_image_count` 返回由 `dyld` 映射的当前 image 的数量。Note: 使用这个
count 迭代所有 images 不是线程安全的，因为另一个线程
可能在迭代过程中添加或删除 image。

&emsp;`_dyld_get_image_header` 返回指向由 `image_index` 索引的 image 的 `mach_header` 头的指针。如果 `image_index` 超出范围，则返回 NULL。

&emsp;`_dyld_get_image_vmaddr_slide` 返回由 `image_index` 索引的 image 的虚拟内存地址滑动量。如果 `image_index` 超出范围返回零。

&emsp;好了，关于 dyld 的一些背景知识都看完了，下面我们接着看 fishhook.c 中的代码。

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

&emsp;下面我们看 fishhook 的核心函数，也是上面 `rebind_symbols` 函数的核心函数：`_rebind_symbols_for_image`。

##### \_rebind_symbols_for_image

```c++
static void _rebind_symbols_for_image(const struct mach_header *header,
                                      intptr_t slide) {
    rebind_symbols_for_image(_rebindings_head, header, slide);
}
```

&emsp;emmm... 内部调用 `rebind_symbols_for_image` 函数（名字去掉 `_`），然后把 `_rebindings_head` 这个全局变量作为第一个参数传入。下面开始我们详细看一下 `rebind_symbols_for_image` 函数的定义。

##### rebind_symbols_for_image

&emsp;在开始看 `rebind_symbols_for_image` 函数之前，我们先看几张截图，`rebind_symbols_for_image` 函数便是要遍历形参 `header` 对应的 image（镜像）中的所有 Load commands 并查找到如下三个 Load command：

&emsp;`LC_SEGMENT_64(__LINKEDIT)`：

![截屏2021-07-31 上午4.39.48.png](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/eff8806aa8984edc81fcff5bb411df7d~tplv-k3u1fbpfcp-watermark.image)

&emsp;`LC_SYMTAB`：

![截屏2021-07-31 上午4.39.54.png](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/489acf000bff418cad5517eb13f91f62~tplv-k3u1fbpfcp-watermark.image)

&emsp;`LC_DYSYMTAB`：

![截屏2021-07-31 上午4.39.59.png](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/c2f0a91da45940928c7940ff51584a4a~tplv-k3u1fbpfcp-watermark.image)

&emsp;最后一张截图是 `String Table`（字符串表），用于记录符号表中符号的名字，`rebinding` 结构体中的 `name` 字符串就是要与 `String Table` 中的符号名字字符串来比较的。

![截屏2021-07-31 10.25.48.png](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/d7a374664d6d4617bb8a5efdc614aac8~tplv-k3u1fbpfcp-watermark.image)

```c++
static void rebind_symbols_for_image(struct rebindings_entry *rebindings,
                                     const struct mach_header *header,
                                     intptr_t slide) {
  // 这里是使用 dladdr 函数来判断传入的 header 地址是否有对应的 image 存在 
  Dl_info info;
  if (dladdr(header, &info) == 0) {
    return;
  }
  
  // 下面是 4 个局部变量，分别用来记录从 mach-o 中的找到的指定的 segment load command
  
  // cur_seg_cmd 仅用于记录每次循环时的 segment load command
  segment_command_t *cur_seg_cmd;
  
  // 分别用于记录找到的：LC_SEGMENT_64(__LINKEDIT)、LC_SYMTAB、LC_DYSYMTAB 三个 Load command
  segment_command_t *linkedit_segment = NULL;
  struct symtab_command* symtab_cmd = NULL;
  struct dysymtab_command* dysymtab_cmd = NULL;

  // 指针偏移，越过 mach-o 的 mach header 部分，直接到达 load command 部分的首地址
  uintptr_t cur = (uintptr_t)header + sizeof(mach_header_t);
  
  // 遍历 mach-o 文件中 load commands 中的每条 segment load command，
  // 分别找到类型是 LC_SEGMENT_ARCH_DEPENDENT(name 是 __LINKEDIT)、LC_SYMTAB、LC_DYSYMTAB 的 segment load command
  for (uint i = 0; i < header->ncmds; i++, cur += cur_seg_cmd->cmdsize) {
    cur_seg_cmd = (segment_command_t *)cur;
    
    // #define LC_SEGMENT_ARCH_DEPENDENT LC_SEGMENT_64
    // 首先判断 Load command 的类型是 LC_SEGMENT_64，然后判断其 segname 是否是 __LINKEDIT，如果是即找到了 LC_SEGMENT_64(__LINKEDIT)。 
    if (cur_seg_cmd->cmd == LC_SEGMENT_ARCH_DEPENDENT) {
    
      // #define SEG_LINKEDIT "__LINKEDIT"
      if (strcmp(cur_seg_cmd->segname, SEG_LINKEDIT) == 0) {
      
        // SEG_LINKEDIT：the segment containing all structs created and maintained by the link editor. Created with -seglinkedit option to ld(1) for MH_EXECUTE and FVMLIB file types only
        // SEG_LINKEDIT：包含由 link editor 创建和维护的所有结构的 segment。使用 -seglinkedit 选项创建 ld(1) 仅适用于 MH_EXECUTE 和 FVMLIB 文件类型
        linkedit_segment = cur_seg_cmd;
      }
      
    } else if (cur_seg_cmd->cmd == LC_SYMTAB) {
      // #define LC_SYMTAB 0x2 /* link-edit stab symbol table info */
      // LC_SYMTAB：link-edit stab symbol table info
      
      // 记录 LC_SYMTAB 这条 Load command，它内部记录了 Symbol Table 和 String Table 的偏移量
      symtab_cmd = (struct symtab_command*)cur_seg_cmd;
      
    } else if (cur_seg_cmd->cmd == LC_DYSYMTAB) {
      // #define LC_DYSYMTAB 0xb /* dynamic link-edit symbol table info */
      // LC_DYSYMTAB：dynamic link-edit symbol table info
      
      // 记录 LC_DYSYMTAB 这条 Load command，它内部记录了 Dynamic Symbol Table 的偏移量
      dysymtab_cmd = (struct dysymtab_command*)cur_seg_cmd;
      
    }
  }

  // 如果上面循环遍历中有任何一个 segment load command 没找到则直接 return 结束
  if (!symtab_cmd || !dysymtab_cmd || !linkedit_segment || !dysymtab_cmd->nindirectsyms) {
    return;
  }

  // Find base symbol/string table addresses
  // 找到 symbol/string table 的基址
  //（观察上面的 LC_SYMTAB 截图可看到其中的 Symbol Table Offset 和 String Table Offset 两个字段，我们便可以根据这两个字段的值，做地址偏移，便可找到下面的 Symbol Table 和 String Table 两个表）
  
  // 这里有一个知识点，我们如何找到进程的起始地址，在上面 "通过 LLDB 调试 Lazy Symbol Pointer 绑定过程以及 fishhook 进行 hook 的结果" 一节中，
  // 我们在控制台通过 image list 可得当前进程在内存中的地址，那么这里的 linkedit_base 的值是什么呢？(image 在内存中的地址)
  // 我们可以直接在此行打断点，然后和我们通过 image list 取得的地址进行比较，当入参 header 是我们当前进程的 image 时，发现它们的值是相等的。（测试时同为：0x0000000106730000 这里就不截图了，小伙伴可以自己打印看一下）
  
  // 那么下面我们分析一下为什么通过：(uintptr_t)slide + linkedit_segment->vmaddr - linkedit_segment->fileoff 就能得到当前进程 image 的内存地址呢？
  
  // uint64_t fileoff; /* file offset of this segment */ fileoff 字段表示 segment 在 mach-o 二进制文件的偏移
  // 进程的起始地址 = __LINKEDIT.VM_Address - __LINKEDIT.File_Offset + silde 的改变值
  // LC_SEGMENT_64(__LINKEDIT) 段的虚拟地址，然后减去此段在 mach-o 二进制文件中的文件偏移，即回到了 mach-o 二进制文件的起始位置，然后再加上 silde 随机值，变得到了 image 的起始地址
  
  // (这里多次打印，可发现入参 header 的值和 linkedit_base 的值，大部分情况下都是相等的)
  
  uintptr_t linkedit_base = (uintptr_t)slide + linkedit_segment->vmaddr - linkedit_segment->fileoff;
  
  // ⬇️⬇️⬇️ 下面三个局部变量纷纷对应了我们上面验证了多次的：Symbol Table、String Table、Dynamic Symbol Table，三张表！
  
  // 符号表的地址 = 基址 + 符号表偏移量（已知符号表就是一个 nlist_t 结构体数组）
  nlist_t *symtab = (nlist_t *)(linkedit_base + symtab_cmd->symoff);
  
  // 字符串表的地址 = 基址 + 字符串表偏移量
  char *strtab = (char *)(linkedit_base + symtab_cmd->stroff);

  // Get indirect symbol table (array of uint32_t indices into symbol table)
  // 动态符号表地址 = 基址 + 动态符号表偏移量
  uint32_t *indirect_symtab = (uint32_t *)(linkedit_base + dysymtab_cmd->indirectsymoff);
   
  // 指针偏移，越过 mach-o 的 mach header 部分，直接到达 load command 部分的首地址，并赋值给 cur
  cur = (uintptr_t)header + sizeof(mach_header_t);
  
  // 再次对 segment load command 进行遍历，这里便是查找 __DATA 和 __DATA_CONST 段中的 SECTION_TYPE 是 S_LAZY_SYMBOL_POINTERS 和 S_NON_LAZY_SYMBOL_POINTERS 的区，
  // 已知的便是我们再熟悉不过的 (__DATA_CONST, __got)、(__DATA, __la_symbol_ptr)、(__DATA, __nl_symbol_ptr) 这三个 Section 符合要求。
  for (uint i = 0; i < header->ncmds; i++, cur += cur_seg_cmd->cmdsize) {
    cur_seg_cmd = (segment_command_t *)cur;
    
    // 遍历只需要查找类型是 LC_SEGMENT_64 名字是 __DATA 或者 __DATA_CONST 的 Load command，其它的 Load command 则直接跳过 
    // #define LC_SEGMENT_ARCH_DEPENDENT LC_SEGMENT_64
    
    if (cur_seg_cmd->cmd == LC_SEGMENT_ARCH_DEPENDENT) {
      
      // #define SEG_DATA "__DATA" /* the tradition UNIX data segment */
      // #define SEG_DATA_CONST  "__DATA_CONST"
      
      if (strcmp(cur_seg_cmd->segname, SEG_DATA) != 0 && strcmp(cur_seg_cmd->segname, SEG_DATA_CONST) != 0) {
        continue;
      }
      
      // 下面便是遍历 __DATA/__DATA_CONST 段中的 sections，便可找到其中的 _la_symbol_ptr、__nl_symbol_ptr、__got 三个区！
      
      for (uint j = 0; j < cur_seg_cmd->nsects; j++) {
        section_t *sect = (section_t *)(cur + sizeof(segment_command_t)) + j;
        
        // #define S_LAZY_SYMBOL_POINTERS 0x7 /* section with only lazy symbol pointers */
        // #define S_NON_LAZY_SYMBOL_POINTERS 0x6 /* section with only non-lazy symbol pointers */
        
        // (__DATA_CONST, __got)、(__DATA, __la_symbol_ptr)、(__DATA_CONST, __got) 三个区调用 perform_rebinding_with_section 函数
        
        if ((sect->flags & SECTION_TYPE) == S_LAZY_SYMBOL_POINTERS) {
          perform_rebinding_with_section(rebindings, sect, slide, symtab, strtab, indirect_symtab);
        }
        
        if ((sect->flags & SECTION_TYPE) == S_NON_LAZY_SYMBOL_POINTERS) {
          perform_rebinding_with_section(rebindings, sect, slide, symtab, strtab, indirect_symtab);
        }
      } 
    }
  }
}
```

&emsp;`rebind_symbols_for_image` 函数的内部流程很清晰，就是找到 mach-o 文件中的 `lazy symbol pointers` 和 `non-lazy symbol pointers` 符号指针去调用 `perform_rebinding_with_section` 函数。  

&emsp;下面我们看一下 `perform_rebinding_with_section` 函数的内容。

```c++
/*
 * An indirect symbol table entry is simply a 32bit index into the symbol table to the symbol that the pointer or stub is refering to.  Unless it is for a non-lazy symbol pointer section for a defined symbol which strip(1) as removed.  In which case it has the value INDIRECT_SYMBOL_LOCAL.  If the symbol was also absolute INDIRECT_SYMBOL_ABS is or'ed with that.
 */
#define INDIRECT_SYMBOL_LOCAL    0x80000000
#define INDIRECT_SYMBOL_ABS    0x40000000
```

##### perform_rebinding_with_section

```c++
static void perform_rebinding_with_section(struct rebindings_entry *rebindings,
                                           section_t *section,
                                           intptr_t slide,
                                           nlist_t *symtab,
                                           char *strtab,
                                           uint32_t *indirect_symtab) {
                                           
  // #define SEG_DATA_CONST "__DATA_CONST"
  // isDataConst 用于标记入参 section 是否是属于 __DATA_CONST 段的区
  
  const bool isDataConst = strcmp(section->segname, SEG_DATA_CONST) == 0;
  
  // nl_symbol_ptr 和 la_symbol_ptr section 中的 reserved1 字段指明它们各自在 indirect symbol table 中的起始索引，
  // 这里 indirect_symtab 就是 Indirect Symbols 的起始地址，我们前面讲过，Indirect Symbols 中的符号，便是 (__TEXT, __stubs) 中的 Symbol Stubs 和 (_DATA_CONST, __got) 中的 Non-Lazy Symbol Pointers 和 (__DATA, __la_symbol_ptr) 中的 Lazy Symbol Pointers 依次排列的。
  // 例如上面一张图，在 Dynamic Symbol Table 中的 Indirect Symbols 中：
  // [#0 ~ #33] 是 (__TEXT, __stubs) 中的 Symbol Stubs
  // [#34 ~ #39] 是 (_DATA_CONST, __got) 中的 Non-Lazy Symbol Pointers
  // [#40 ~ #73] 是 (__DATA, __la_symbol_ptr) 中的 Lazy Symbol Pointers
  // 然后我们可以别看到 (_DATA_CONST, __got) Load command 中 reserved1 字段的值是 Indirect Sym Index 且值是 34，
  // 然后 (__DATA, __la_symbol_ptr) Load command 中的 reserved1 字段的值是 Indirect Sym Index 且值是 40。
  
  // 以上面的知识点做铺垫，我们便可以理解下面的：indirect_symtab + section->reserved1 的值便是 (_DATA_CONST, __got) 或 (__DATA, __la_symbol_ptr) 中的符号指针在 Indirect Symbols 中的起始点，
  // 而这里的每个起始点是一个 uint32_t 的数组，每 4 个字节存放的内容便是 (_DATA_CONST, __got) 或 (__DATA, __la_symbol_ptr) 中的一个符号指针对应的符号在 Symbol Table 中的索引。
  
  // 1⃣️1⃣️1⃣️
  uint32_t *indirect_symbol_indices = indirect_symtab + section->reserved1;
  
  // slide + section-> addr 就是 (_DATA_CONST, __got) 或 (__DATA, __la_symbol_ptr) 中的符号指针数组的起始地址，然后就可以通过 indirect_symbol_bindings 遍历所有的符号指针了
  
  // 2⃣️2⃣️2⃣️
  void **indirect_symbol_bindings = (void **)((uintptr_t)slide + section->addr);
  
  // typedef int vm_prot_t;
  // #define VM_PROT_READ ((vm_prot_t) 0x01) /* read permission */
  vm_prot_t oldProtection = VM_PROT_READ;
  
  // 如果是来自 __DATA_CONST 段的 section，由于 __DATA_CONST 段如其名，其只能读，不能写即不能修改，所以这里要修改它的权限，把它变为可修改。
  if (isDataConst) {
    oldProtection = get_protection(rebindings);
    // 修改权限为可读可写
    mprotect(indirect_symbol_bindings, section->size, PROT_READ | PROT_WRITE);
  }
  
  // 遍历  (_DATA_CONST, __got) 或 (__DATA, __la_symbol_ptr) section 里的每一个符号指针（即遍历 Lazy Symbol Pointers 和 Non-Lazy Symbol Pointers 中的每个 Symbol Pointer，for 循环中 sizeof(void *) 正是计算一个指针的长度） 
  for (uint i = 0; i < section->size / sizeof(void *); i++) {
  
    // 找到符号在符号表中的下标，Indirect Symbols 中内容的是符号在 Symbol Table 中的下标的数组，通过此下标，我们便能字 Symbols Table 中找到此下标对应的符号
    
    // 1⃣️1⃣️1⃣️
    uint32_t symtab_index = indirect_symbol_indices[i];
    
    if (symtab_index == INDIRECT_SYMBOL_ABS || symtab_index == INDIRECT_SYMBOL_LOCAL ||
        symtab_index == (INDIRECT_SYMBOL_LOCAL   | INDIRECT_SYMBOL_ABS)) {
      continue;
    }
    
    // 以 symtab_index 作为下标，访问 symbol table，找到了对应的符号，然后 strtab_offset 值便是此符号的名字字符串在 String Table 中的首字符索引
    uint32_t strtab_offset = symtab[symtab_index].n_un.n_strx;
    
    // String Table 的首地址偏移，便获取到 symbol_name
    char *symbol_name = strtab + strtab_offset;
    
    // 判断符号的名字字符串是否有两个字符，为啥是两个，因为前面有一个 _，所以符号的名字长度最少是 1  
    bool symbol_name_longer_than_1 = symbol_name[0] && symbol_name[1];
    
    // 这里的双层循环，外层循环是遍历符号指针，内层循环则是遍历我们调用 rebind_symbols 函数构建的链表遍历链表中的 rebinding 数组，看看能不能找到我们要 hook 的函数
    // 下面开始遍历链表中的 rebinding 进行 hook，把链表的 头 赋值给 cur
    
    struct rebindings_entry *cur = rebindings;
    
    // 外层的 while 循环用来遍历 rebindings_entry 链表
    while (cur) {
     
      // 内层的 for 循环遍历链表每个节点的 rebinding 数组
      for (uint j = 0; j < cur->rebindings_nel; j++) {
        
        // 判断 符号名字长度超过 1，以及符号的名字与 rebinding 的 name 相同
        if (symbol_name_longer_than_1 && strcmp(&symbol_name[1], cur->rebindings[j].name) == 0) {
        
          // 如果 rebinding 的 replaced 不为 NULL，这里 cur->rebindings[j].replaced 是一个二维指针，要进行判空操作，
          // 并且当前符号指针指向的符号和 hook 要替换的函数不一致，才有必要进行 hook。
          if (cur->rebindings[j].replaced != NULL && indirect_symbol_bindings[i] != cur->rebindings[j].replacement) {
          
            // rebinding 的 replaced 记录原始符号对应的函数实现（rebindings[j].replaced 保存 indirect_symbol_bindings[i] 的函数地址） 
            
            *(cur->rebindings[j].replaced) = indirect_symbol_bindings[i];
          }
          
          // 把本次循环的符号指针指向的函数替换为我们 rebinding 中准备的替换函数 replacement 
          
          // 2⃣️2⃣️2⃣️
          indirect_symbol_bindings[i] = cur->rebindings[j].replacement;
          
          // goto 语句，跳到下面的 symbol_loop 处，这里是指当前这个 符号指针 已经被替换了，开始查找下一个 符号指针
          goto symbol_loop;
        }
        
      }
      
      // 更新 cur 为链表的下一个节点
      cur = cur->next;
    }
    
  symbol_loop:;
  }
  
  // 如果 section 是来自 __DATA_CONST 段，上面修改了它的读写权限，所以这里需要把它修改回去！
  if (isDataConst) {
    int protection = 0;
    if (oldProtection & VM_PROT_READ) {
      protection |= PROT_READ;
    }
    if (oldProtection & VM_PROT_WRITE) {
      protection |= PROT_WRITE;
    }
    if (oldProtection & VM_PROT_EXECUTE) {
      protection |= PROT_EXEC;
    }
    mprotect(indirect_symbol_bindings, section->size, protection);
  }
  
}
```

&emsp;`perform_rebinding_with_section` 函数内部就是遍历  `(_DATA_CONST, __got)` 、 `(__DATA, __la_symbol_ptr)` 、`(__DATA, __nl_symbol_ptr)`  section 里的每一个符号指针（即遍历 Lazy Symbol Pointers 和 Non-Lazy Symbol Pointers 中的每个 Symbol Pointer）找到与 `rebindings_entry` 链表中的 `rebinding` 结构体的 `name` 字段相同的符号指针，然后把符号指针指向的符号对应的函数地址记录在 `replaced` 中，并把原符号指针指向的函数，替换为 `rebinding` 结构体的  `replacement` 字段。

&emsp;至此 fishhook 的全部实现过程我们就看完了！🎉🎉🎉

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
