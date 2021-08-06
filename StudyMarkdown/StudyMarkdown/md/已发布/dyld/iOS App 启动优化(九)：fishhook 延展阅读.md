# iOS App 启动优化(九)：fishhook 延展阅读

> &emsp;本篇看似是对 fishhook 的一个延展学习，实则是对 fishhook 涉及到的基础知识点进行统一学习和巩固，所以本篇还是对基础知识点的学习，只要把基础知识点都学好了，后面我们便有无限的可能！

## hook 概述

&emsp;hook：在进程中勾住某一个函数或者在计算机中钩住某一个进程，从而扩展程序的功能或者改变程序运行的流程。

## fishhook 为什么不能 hook 自定义函数

&emsp;运行测试如下代码，可发现我们定义的 `func` 函数并不能被 fishhook hook 到。

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
&emsp;通过上一篇的学习，我们已知的 fishhook 仅能 hook Lazy Symbol Pointers 和 Non-lazy Symbol Pointers 中的符号指针（动态链接库函数），已知它们仅在 `(__DATA, __got)/(__DATA_CONST, __got)`、`(__DATA, __la_symbol_ptr)`、`(__DATA, __nl_symbol_ptr)` 这些 Section 中存在，而我们的自定义函数则是直接在 `(_TEXT, __text)` Section 中存放的，下面我们通过不同的方式验证一下。

+ 使用 `image list` 取得当前进程的内存首地址，然后减去 `func` 函数的起始地址，便可得到 `func` 函数在当前进程的 mach-o 二进制可执行文件中的偏移量，然后通过 MachOView 可视化找到该偏移量在 mach-o 二进制可执行文件中的位置。

&emsp;在 `func();` 处打一个断点，执行程序进入到该断点，然后 `p func` 打印：`(void (*)()) $0 = 0x000000010027d520 (TEST_Fishhook func at ViewController.m:12)` 看到 `func` 函数的地址是：`0x000000010027d520`，然后使用 `image list -h` 打印出一组内存地址，它们便是当前进程和其依赖的各种库的内存地址，第一个地址是便是当前进程的内存地址：`[  0] 0x000000010027c000`，然后通过 `(lldb) p/x 0x000000010027d520-0x000000010027c000 (long) $1 = 0x0000000000001520` 打印可看到 `func` 函数在当前进程的 mach-o 二进制可执行文件中的偏移量是 `0x0000000000001520`，然后我们通过 MachOView 查找，可看到其位置在 `(__TEXT, __text)` Section 中。

![截屏2021-08-07 上午5.20.02.png](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/f4f720cd887347d18469b28e6defcfbe~tplv-k3u1fbpfcp-watermark.image)

![截屏2021-08-07 上午5.20.09.png](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/ed7397158eb8481bbc53e26b548ed75c~tplv-k3u1fbpfcp-watermark.image)

![截屏2021-08-07 上午5.19.10.png](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/bd23e873e67b4a9bad78e95aa31dc2e4~tplv-k3u1fbpfcp-watermark.image)

&emsp;下面我们再延伸一下，顺着  `0x0000000000001520` 的偏移往下读，可看到汇编跳转指令：`call 0x10000238c` 它便是 `printf` 的调用（`func` 函数定义内部是 `printf` 函数的调用），偏移量 `0x238c` 处是 `(__TEXT, __stubs)` 区，可看到 Value 标识的是 `_printf`。 

![截屏2021-08-07 上午5.43.31.png](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/31a8733b2b764f689d423d97a5193bca~tplv-k3u1fbpfcp-watermark.image)

+ 使用 Hopper Disassembler 验证上面的地址，可发现和我们上面计算以及打印的偏移量完全一致。（这里还牵涉到一个 Symbol Pointer 的懒加载问题，我们留在下一小节分析。）

![截屏2021-08-07 上午5.49.07.png](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/16c50506bcc3483dba467db56d270e10~tplv-k3u1fbpfcp-watermark.image)

![截屏2021-08-07 上午5.49.20.png](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/e80345eda36a44279c7cbdaf33997b08~tplv-k3u1fbpfcp-watermark.image)

![截屏2021-08-07 上午5.49.56.png](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/0596c6ad93a948aba917cba534c0c39a~tplv-k3u1fbpfcp-watermark.image)

![截屏2021-08-07 上午5.54.04.png](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/d76caf17f4954ba5b0683a527582b582~tplv-k3u1fbpfcp-watermark.image)

![截屏2021-08-07 上午5.54.29.png](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/93d572fc5bfb428d8b9ed2ae8a8b313e~tplv-k3u1fbpfcp-watermark.image)


## dyld_stub_binder 

&emsp;懒加载符号指针的绑定过程。


[LLDB Quick Start Guide](https://developer.apple.com/library/archive/documentation/IDEs/Conceptual/gdb_to_lldb_transition_guide/document/Introduction.html#//apple_ref/doc/uid/TP40012917-CH1-SW1)
[LLDB调试器使用简介](http://southpeak.github.io/2015/01/25/tool-lldb/)
