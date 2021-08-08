# iOS App 启动优化(九)：fishhook 延展阅读

> &emsp;本篇标题是对 fishhook 延展阅读，实则是对 fishhook 涉及到的基础知识点进行统一学习和巩固，所以本篇还是对 mach-o 和 dyld 加载过程涉及的基础知识点的学习，只要把基础知识点都学好了，后面我们便有无限的可能！

## hook 概述

&emsp;hook：在进程中勾住某一个函数或者在计算机中钩住某一个进程，从而扩展程序的功能或者改变程序运行的流程。iOS 中 hook 使用场景：埋点、Crash 防护、应用加固、应用隔离等等。

## fishhook 为什么不能 hook 自定义函数（C 函数）

&emsp;运行测试如下代码，可发现我们定义的 `func` 函数并不能被 fishhook hook 到。（在这里可以学到一些函数位于不同分区的知识点）

```c++
#import "ViewController.h"
#import "fishhook.h"

// 原函数
static void func(void) {
    printf("♻️♻️♻️ %s \n", __func__);
}

// 新函数
static void hook_func(void) {
    printf("♻️♻️♻️ %s \n", __func__);
}

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    
    NSLog(@"✳️✳️✳️ NSLog: %p", NSLog);
    
    func();
    
    // hook func
    struct rebinding func_reb;
    func_reb.name = "func";
    func_reb.replacement = hook_func;
    func_reb.replaced = nil;

    // 定义需要 hook 的函数的结构体数组变量
    struct rebinding rebs[] = {func_reb};
    
    // 很简单，传递结构体数组地址及其成员变量数目
    rebind_symbols(rebs, 1);
    
    func();
}

// 控制台输出:
♻️♻️♻️ func 
♻️♻️♻️ func 
```

&emsp;通过上一篇的学习，我们已知的 fishhook 仅能 hook Lazy Symbol Pointers 和 Non-lazy Symbol Pointers 中的符号指针（这些符号指针所指向的符号都来自动态链接库），已知它们仅在 `(__DATA, __got)/(__DATA_CONST, __got)`、`(__DATA, __la_symbol_ptr)`、`(__DATA, __nl_symbol_ptr)` 这些 Section 中存在，而我们的自定义函数则是位于 `(_TEXT, __text)` Section 中的（是指函数定义，不是如前几个分区中是符号指针），我们已知的 `__DATA` 段的内容可读可写，而 `__TEXT` 段的内容只可读可执行，所以我们位于 `(__TEXT, __text)` Section 中的自定义函数仅仅可读可执行（可调用），且我们的自定义函数的调用是直接通过函数地址调用，并没有采用 Symbol Pointer 进行动态绑定，即写权限限制 + 通过地址直接调用两个原因下，fishhook 不能对我们的自定义函数进行 hook。

&emsp;下面我们通过不同的方式验证一下我的自定义函数位于 `__TEXT` 段中。

+ 使用 `image list` 取得当前进程的内存首地址，然后使用 `func` 函数的起始地址减去它，便可得到 `func` 函数在当前进程的 mach-o 二进制可执行文件中的偏移量，然后通过 MachOView 可视化找到该偏移量在 mach-o 二进制可执行文件中的位置并查看其内容。

&emsp;在 `func();` 处打一个断点，执行程序进入到该断点，然后 `p func` 打印：`(void (*)()) $0 = 0x000000010027d520 (TEST_Fishhook func at ViewController.m:12)` 看到 `func` 函数的地址是：`0x000000010027d520`，作为对比我们同样 `p NSLog` 打印来自 Foundation.framework 动态库中的 `NSLog` 函数的地址：`(void (*)(NSString * _Nonnull __strong, ...)) $1 = 0x00007fff20805d0d (Foundation NSLog)`，可看到 `NSLog` 的函数地址和 `func` 函数的地址完全不在一个 Level（因为 `NSLog` 根本不属于该进程，它是当前程序启动后被动态绑定的），然后使用 `image list -h` 打印出一组内存地址，它们便是当前进程和其依赖的各种库的内存地址，第一个地址是便是当前进程的内存地址：`[  0] 0x000000010027c000`，然后通过 `(lldb) p/x 0x000000010027d520-0x000000010027c000 (long) $1 = 0x0000000000001520` 打印可看到 `func` 函数在当前进程的 mach-o 二进制可执行文件中的偏移量是 `0x0000000000001520`，然后我们通过 MachOView 查找，可看到其位置在 `(__TEXT, __text)` Section 中。

![截屏2021-08-07 上午5.20.02.png](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/f4f720cd887347d18469b28e6defcfbe~tplv-k3u1fbpfcp-watermark.image)

![截屏2021-08-07 上午5.20.09.png](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/ed7397158eb8481bbc53e26b548ed75c~tplv-k3u1fbpfcp-watermark.image)

![截屏2021-08-07 上午5.19.10.png](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/bd23e873e67b4a9bad78e95aa31dc2e4~tplv-k3u1fbpfcp-watermark.image)

&emsp;下面我们再延伸一下，顺着  `0x0000000000001520` 的偏移往下读，可看到汇编跳转指令：`call 0x10000238c` 它便是 `printf` 的调用（`func` 函数定义内部是 `printf` 函数的调用），偏移量 `0x238c` 处是 `(__TEXT, __stubs)` 区，可看到 Value 标识的是 `_printf`。（这里先预告一个结论：当前进程使用到的系统动态库中的函数会在进程启动时和被第一次调用时进行动态绑定。） 

![截屏2021-08-07 上午5.43.31.png](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/31a8733b2b764f689d423d97a5193bca~tplv-k3u1fbpfcp-watermark.image)

+ 使用 Hopper Disassembler 反汇编当前进程的 mach-o 二进制可执行文件，来验证上面的函数地址，可发现汇编指令的地址和我们上面计算打印出的偏移量完全一致。（这里还牵涉到一个 Symbol Pointer 的懒加载问题，我们留在下一小节中具体分析。）

&emsp;可看到 fishhook hook 前后，对 `_func` 函数的调用没有发生任何变化，即 fishhook 不能对我们的自定义函数进行 hook。

![截屏2021-08-07 上午5.49.07.png](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/16c50506bcc3483dba467db56d270e10~tplv-k3u1fbpfcp-watermark.image)

&emsp;然后我们双击 `_func` 函数（标签），跳转到 `_func` 函数的位置，发现它的偏移量是 `0x1520` 和上面我们手动计算到的一样，然后看到它内部对 `printf` 的调用其实是 `imp___stubs__printf` 的调用，`printf` 同 `NSLog` 函数类似，都是来自系统动态库中的函数（`printf` 函数在 MachOView 中看到是位于 `libSystem.B.dylib` 中，但是在进程中打印时却在 `libsystem_c.dylib` 中），可看到 `printf` 函数的调用方式和 `func` 函数完全不同，我们自定义的 `func` 函数我们能直接在当前进程的 mach-o 二进制文件中找到其函数定义的位置，而当前进程使用到的系统的动态库中的函数则是只在当前进程的 mach-o 二进制文件中有一个桩对应，在程序启动后则是调用 `dyld_stub_binder` 进行绑定。

![截屏2021-08-07 上午5.49.20.png](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/e80345eda36a44279c7cbdaf33997b08~tplv-k3u1fbpfcp-watermark.image)

&emsp;双击 `imp___stubs__printf` 跳转到 `imp___stubs__printf` 定义处，可看到它的偏移同样是 `0x238c`，此时再次双击 `_printf_ptr`。

![截屏2021-08-07 上午5.49.56.png](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/0596c6ad93a948aba917cba534c0c39a~tplv-k3u1fbpfcp-watermark.image)

&emsp;此时就来到了 `(__DATA, __la_symbol_ptr)` 的位置，其对应的正是 Lazy Symbol Pointers 中的 `_printf` 这个符号指针，走到这里我们就发现了对 `printf` 的调用是从 `_printf` 这个 Lazy Symbol Pointer 寻址调用的，并不像我们自己的自定义函数，调用时就直接拿到函数的地址进行调用，且这里我们看到 `_printf` 符号指针位于 `_DATA` 数据段中，数据段的内容可读可修改，因此这也是 fishhook 能够 hook 动态链接库 `Foundation.framework` 中的 `printf` 的一个重要原因。 

![截屏2021-08-07 上午5.53.55.png](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/4c215ca7c1b444169df64b58c0705f49~tplv-k3u1fbpfcp-watermark.image)

![截屏2021-08-07 上午5.54.04.png](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/d76caf17f4954ba5b0683a527582b582~tplv-k3u1fbpfcp-watermark.image)

### Lazy Sympol Pointers 和 Non-Lazy Symbol Pointers 中的符号指针的默认指向

&emsp;我们接着上一段的内容继续描述。真正的 `printf` 函数定义位于系统动态链接库 `libSystem.B.dylib` 中，所以对当前进程来说 `printf` 是一个外部符号，由于 `ASLR` 偏移量，动态链接库每次加载到内存的地址是不定的，所以需要可执行文件加载到内存时对符号表中符号进行重新绑定（Lazy/Non_Lazy），以修正符号表中每个 `nlist_64` 结构体的 `n_value` 的值，而这里便是把 `printf` 对应的符号表中的 `nlist_64` 结构体的 `n_value` 的值替换为 `printf` 在 `libSystem.B.dylib` 中的地址（或者说是 `printf` 函数定义在内存中的地址更准确一点）。 Lazy Symbol Pointers 中的符号指针指向的符号第一次使用时才进行重绑定，Non-Lazy Symbol Pointers 中的符号指针指向的符号则是在程序启动时就进行符号重绑定，且在程序的 mach-o 二进制文件中，Lazy Symbol Pointer 默认指向 `dyld_stub_binder`，而 Non-Lazy Symbol Pointer 则默认指向 `0x000000`。下面我们通过 MachOView 来看一下。

![截屏2021-08-08 下午12.30.44.png](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/6a08fb2428634f0cb195d917393338b4~tplv-k3u1fbpfcp-watermark.image)

![截屏2021-08-08 下午12.39.16.png](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/b8a90c2a50a043fea3c26f1c606678f8~tplv-k3u1fbpfcp-watermark.image)

&emsp;如图所示可看到 Non-Lazy Symbol Pointers 中的所有的符号指针默认都是指向 `0000000000000000`，符号表中 `objc_msgSend` 符号的 `n_value` 的值也为 `0000000000000000`，程序启动时会对符号表中当前进程中所有的 Non-Lazy Symbol Pointers 对应的符号进行重绑定，同时 Non-Lazy Symbol Pointer 也会修改正确的指向。  

&emsp;Lazy Symbol Pointers 中的符号指针则默认都会指向到 `dyld_stub_binder` 中去。

![截屏2021-08-08 下午12.57.11.png](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/d45032fd54cd47d5a30753fe1202aca5~tplv-k3u1fbpfcp-watermark.image)

&emsp;如图可看到 Lazy Symbol Pointers 中的 `_printf` 符号指针指向 `0x10000246E`，且位于 `_printf` 前后不同位置的符号指针的指向几乎都是顺序排列的 `0x100002400`、`0x100002478`、`0x100002482` 等等。

![截屏2021-08-08 下午12.56.57.png](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/09530b568bf9419baa3ede5d8aa1fda1~tplv-k3u1fbpfcp-watermark.image)

&emsp;如图可看到 `0x10000246E` 位于 `(__TEXT, __stub_helper)` Section 中，然后汇编指令转换看到都有一个跳转指令：`jmp 0x1000023a0`。其中已知 `0x2478` 是 `_strcmp` 符号指针的指向，`0x245A` 是 `_malloc` 符号指针的指向，`0x2450` 是 `_free` 符号指针的指向，然后我们上下翻动 `(__TEXT, __stub_helper)` Section 中的内容，可看到每个指向这里的 Lazy Symbol Pointers 中的符号指针默认都会跳转到 `0x1000023a0` 这个地址。 

![截屏2021-08-08 下午12.57.56.png](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/799db8ec4b084185b3a1e526455166af~tplv-k3u1fbpfcp-watermark.image)

&emsp;`0x23A0` 是 `(__TEXT, __stub_helper)` Section 的起始位置，其中的 `jmp qword ptr [rip + 0xec71]` 便是跳转到 `dyld_stub_binder`，`dyld_stub_binder` 便是 dyld 进行桩绑定，即 dyld 进行符号绑定。

## Lazy Symbol Pointer 的动态绑定过程 dyld_stub_binder 

&emsp;



















## 参考链接
**参考链接:🔗**
+ [LLDB Quick Start Guide](https://developer.apple.com/library/archive/documentation/IDEs/Conceptual/gdb_to_lldb_transition_guide/document/Introduction.html#//apple_ref/doc/uid/TP40012917-CH1-SW1)
+ [LLDB调试器使用简介](http://southpeak.github.io/2015/01/25/tool-lldb/)
+ [十 iOS逆向- hopper disassembler](https://www.jianshu.com/p/20077ceb2f75)
+ [iOS逆向之Hopper进阶](https://www.jianshu.com/p/384dc5bc1cb4)
+ [一文读懂fishhook原理](https://juejin.cn/post/6857699952563978247)
