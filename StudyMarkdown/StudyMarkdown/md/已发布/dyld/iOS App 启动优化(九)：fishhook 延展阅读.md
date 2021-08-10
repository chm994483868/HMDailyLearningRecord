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

## Lazy Symbol Pointer 的动态绑定过程（dyld_stub_binder） 

&emsp;已知在 `(__DATA, __got)/(__DATA_CONST, __got)`、`(__DATA, __la_symbol_ptr)`、`(__DATA, __nl_symbol_ptr)` 这些 Section 中保存的分别是 Lazy Symbol Pointers 和 Non-Lazy Symbol Pointers，即分别为 Lazy Binding 指针表和 Non Lazy Binding 指针表。其中的 Lazy Binding 指针表比较特殊，我们从名字可以看出它是一个懒绑定指针表，当 mach-o 二进制可执行文件通过 dyld 加载时并没有在加载  Lazy Binding 指针表的过程中直接对其中的符号指针进行绑定（确定它们指向的符号地址），而是在第一次调用该符号指针对应的函数时，通过 PLT(Procedure Linkage Table) 来进行一次 Lazy Binding。下面我们通过如下示例代码进行验证。

```c++
#include <stdio.h>

int main(int argc, char * argv[]) {
    
    printf("♻️♻️♻️ %s \n", "hello world");
    printf("♻️♻️♻️ %s \n", "hello desgard");
    
    return 0;
}
```

&emsp;然后我们使用 Hopper Disassembler 查看其汇编实现如下：

```c++
_main:
0000000100002160         push       rbp
0000000100002161         mov        rbp, rsp
0000000100002164         sub        rsp, 0x20
0000000100002168         mov        dword [rbp+var_4], 0x0
000000010000216f         mov        dword [rbp+var_8], edi
0000000100002172         mov        qword [rbp+var_10], rsi
0000000100002176         lea        rdi, qword [aXe2x99xbbxefxb] ; argument "format" for method imp___stubs__printf, "\\xE2\\x99\\xBB\\xEF\\xB8\\x8F\\xE2\\x99\\xBB\\xEF\\xB8\\x8F\\xE2\\x99\\xBB\\xEF\\xB8\\x8F %s \\n"
000000010000217d         lea        rsi, qword [aHelloWorld]     ; "hello world"
0000000100002184         mov        al, 0x0
0000000100002186         call       imp___stubs__printf          ; printf
000000010000218b         lea        rdi, qword [aXe2x99xbbxefxb] ; argument "format" for method imp___stubs__printf, "\\xE2\\x99\\xBB\\xEF\\xB8\\x8F\\xE2\\x99\\xBB\\xEF\\xB8\\x8F\\xE2\\x99\\xBB\\xEF\\xB8\\x8F %s \\n"
0000000100002192         lea        rsi, qword [aHelloDesgard]   ; "hello desgard"
0000000100002199         mov        dword [rbp+var_14], eax
000000010000219c         mov        al, 0x0
000000010000219e         call       imp___stubs__printf          ; printf
00000001000021a3         xor        ecx, ecx
00000001000021a5         mov        dword [rbp+var_18], eax
00000001000021a8         mov        eax, ecx
00000001000021aa         add        rsp, 0x20
00000001000021ae         pop        rbp
00000001000021af         ret
   ; endp
```

&emsp;可看到调用 `printf` 函数的时候会触发 `call imp___stubs__printf ; printf` 指令，双击 `imp___stubs__printf` 进入其存储位置查看：

```c++
imp___stubs__printf:        // printf
0000000100002452         jmp        qword [_printf_ptr] ; _printf_ptr, _printf_ptr,_printf, CODE XREF=_main+38, _main+62
   ; endp
```

&emsp;然后再双击 `_printf_ptr` 可看到如下内容：

```c++
_printf_ptr:
0000000100008028         extern     _printf ; DATA XREF=imp___stubs__printf
```

&emsp;通过上面的汇编指令，我们看到 `imp___stubs__printf` 指针指向了 `0x0000000100008028`，即可以知道我们的 `(__DATA, __la_symbol_ptr)` Section 中的 `_printf` 这个指针在 mach-o 二进制可执行文件的偏移值是 `0x8028`。下面我们在两个 `printf` 方法前加上断点，然后使用 LLDB 对其进行调试：

&emsp;首先我们使用 `image list` 指令获得当前进程在内存中的首地址 `0x000000010b941000`。

```c++
(lldb) image list
[  0] 7FE17B7A-D271-3133-9A56-A92A3780D8BC 0x000000010b941000 /Users/hmc/Library/Developer/Xcode/DerivedData/TEST_Fishhook-guvclcyaszalpmdldofnoxqksebw/Build/Products/Debug-iphonesimulator/TEST_Fishhook.app/TEST_Fishhook 
...
```

&emsp;然后我们使用 `memory read 0x000000010b941000+0x8028` 指令查看 `_printf` 符号指针的指向内容，已知 iOS 是小端模式，可知 `_printf` 符号指针指向 `0x010b94349a`

```c++
(lldb) memory read 0x000000010b941000+0x8028
0x10b949028: 9a 34 94 0b 01 00 00 00 81 00 00 00 28 00 00 00  .4..........(...
0x10b949038: 28 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  (...............
(lldb) 
```

&emsp;然后我们使用 `dis -s 0x010b94349a` 把此地址的数据转换为汇编指令，看到其中有一个 `jmp 0x10b943458` 指令跳转。

```c++
(lldb) dis -s 0x010b94349a
    0x10b94349a: pushq  $0x91
    0x10b94349f: jmp    0x10b943458
    0x10b9434a4: jbe    0x10b94350f               ; ""
    0x10b9434a6: ja     0x10b9434ed               ; "ector:"
    0x10b9434a9: imull  $0x72006461, 0x6f(%rsp,%rcx,2), %esp ; imm = 0x72006461 
    0x10b9434b1: outsl  (%rsi), %dx
    0x10b9434b2: insb   %dx, %es:(%rdi)
    0x10b9434b3: addb   %ch, %gs:0x6e(%rcx)
(lldb) 
```

&emsp;然后我们使用 `dis -s 0x10b943458` 查看 `0x10b943458` 中的内容，可看到其中对 `dyld_stub_binder` 的调用，`dyld_stub_binder` 方法的作用简单来讲就是计算对应的函数地址进行绑定，之后进而调用对应函数。

```c++
(lldb) dis -s 0x10b943458
    0x10b943458: leaq   0x7019(%rip), %r11        ; _dyld_private
    0x10b94345f: pushq  %r11
    0x10b943461: jmpq   *0x1ba9(%rip)             ; (void *)0x00007fff2025cbb4: dyld_stub_binder
    0x10b943467: nop    
    0x10b943468: pushq  $0x0
    0x10b94346d: jmp    0x10b943458
    0x10b943472: pushq  $0x12
(lldb) 
```

&emsp;此时我们再单步执行，到达第二个断点，看到控制台输出了 `♻️♻️♻️ hello world ` 此时表示我们第一次调用 `printf` 函数结束了。此时我们再次调用 `memory read 0x000000010b941000+0x8028` 查看 `_printf` 符号指针指向的内容。 

```c++
(lldb) memory read 0x000000010b941000+0x8028
0x10b949028: e8 f4 0b 20 ff 7f 00 00 81 00 00 00 28 00 00 00  ... ........(...
0x10b949038: 28 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  (...............
(lldb) 
```

&emsp;可看到此时 `_printf` 符号指针指向 `0x7fff200bf4e8`，然后我们使用 `dis -s 0x7fff200bf4e8` 查看内容，此时便可看到 `_printf` 符号指针此时便指向 `libsystem_c.dylib printf` 函数了。

```c++
(lldb) dis -s 0x7fff200bf4e8
libsystem_c.dylib`printf:
    0x7fff200bf4e8 <+0>:  pushq  %rbp
    0x7fff200bf4e9 <+1>:  movq   %rsp, %rbp
    0x7fff200bf4ec <+4>:  pushq  %r14
    0x7fff200bf4ee <+6>:  pushq  %rbx
    0x7fff200bf4ef <+7>:  subq   $0xd0, %rsp
    0x7fff200bf4f6 <+14>: movq   %rdi, %r14
    0x7fff200bf4f9 <+17>: testb  %al, %al
    0x7fff200bf4fb <+19>: je     0x7fff200bf526            ; <+62>
    0x7fff200bf4fd <+21>: movaps %xmm0, -0xb0(%rbp)
(lldb) 
```

&emsp;即我们的 Lazy Binding 指针在第一调用时通过 `dyld_stub_binder` 函数对其进行正确的绑定，并进行调用，然后后续再对其调用就是直接调用 Lazy Symbol Pointer 所指向的函数了。 

## ASLR 简述

&emsp;ASLR(Address Space Layout Randomization，地址空间布局随机化)，是一种针对缓冲区溢出的安全保护技术。借助 ASLR，mach-o 二进制可执行文件每次加载到内存的起始地址都会随机变化。目前大部分主流操作系统都已经实现了 ASLR，如 Windows Vista、Linux 2.6.12、Mac OS X 10.7、iOS 4.3 以及 Android 4.0 均从此版本开始支持 ASLR。

&emsp;简单说，ASLR 使得渗透（基于缓冲区溢出）攻击的难度明显提升，增加了系统的安全性。但是，对于不是搞安全/逆向的 Programmer 来说，在调试程序时这就略显蛋疼。控制变量是调试阶段的一大原则。

## PIC 简述

&emsp;苹果为了能在 mach-O 二进制可执行文件中访问外部函数（系统动态链接库/共享缓存库中的函数），采用了一个技术，叫做 PIC（Position-independent code 位置代码独立）技术。

&emsp;C 语言是静态的，也就是说在编译的时候就已经确定了函数的地址，而系统动态库中的函数并不会直接编译进 mach-o 二进制可执行文件中，所以在 mach-o 二进制可执行文件没有启动之前，是无法确定引用到的系统动态库中的函数的地址的，只有在 mach-o 二进制文件启动运行的时候才能借由 dyld 对使用到的系统动态库中的函数进行绑定，确认其函数地址。所以为了这些引用到的系统动态库中函数，苹果针对 mach-o 二进制文件提供了 PIC 技术，首先在 mach-o 二进制可执行文件中添加了 Lazy Symbol Pointers 懒加载符号指针表和 Non-Lazy Symbol Pointers 非懒加载符号指针表，它们中记录的都是使用到的系统动态库中的符号，在 mach-o 二进制可执行文件启动之前 Lazy Symbol Pointer 默认指向 `dyld_stub_binder`，而 Non-Lazy Symbol Pointer 则默认指向 `0x000000`，启动之后，则通过 dyld 对这些符号指针进行绑定，为它们赋值动态缓存库中确定的函数地址。

&emsp;当应用程序想要调用 mach-o 二进制文件外部的函数时，或者说是如果 mach-o 二进制文件内部需要调用系统的库函数时，mach-o 文件会在 `__DATA` 段中建立对应的符号指针表，这些符号指针指向外部函数，dyld 会对它们进行动态绑定将它们指向正确的函数地址。

&emsp;所以这么看的话，mach-o 二进制可执行文件中使用到的系统动态库中的 C 函数在 mach-o 二进制可执行文件启动运行进内存的时候也有了动态的表现，C 在内部函数的时候是静态的，在编译后函数的地址就确定了，但是，引用到的外部的函数是不能确定的，也就是说 C 的底层也有了动态特性。fishhook 正是借助于这一点，fishhook 的原理其实就是，将指向系统方法（外部函数）的符号指针重新进行绑定指向内部的函数。这样就把系统方法与自己定义的方法进行了交换。这也就是为什么 C 的内部函数修改不了，自定义的函数修改不了，只能修改 mach-o 外部【共享缓存库中】的函数。  

> &emsp;大家都知道 OC 的方法之所以可以 HOOK 是因为它的运行时特性，OC 的方法调用在底层都是 msg_send（id,SEL）的形式，这为我们提供了交换方法实现（IMP）的机会，但 C 函数在编译链接时就确定了函数指针的地址偏移量（Offset），这个偏移量在编译好的可执行文件中是固定的，而可执行文件每次被重新装载到内存中时被系统分配的起始地址（在 lldb 中用命令 image List 获取）是不断变化的。运行中的静态函数指针地址其实就等于上述 Offset + mach-o 文件在内存中的首地址。（我们知道 C 函数是静态的，也就是说在编译的时候，编译器就知道了它的实现地址，这也是为什么 C 函数只写函数声明调用时会报错。）
> &emsp;既然 C 函数的指针地址是相对固定且不可修改的，那么 fishhook 又是怎么实现 对 C 函数的 HOOK 呢？其实内部/自定义的 C 函数 fishhook 也 HOOK 不了，它只能 HOOK Mach-O 外部（共享缓存库中）的函数。fishhook 利用了 MachO 的动态绑定机制，苹果的共享缓存库不会被编译进我们的 MachO 文件，而是在动态链接时才去重新绑定。苹果采用了PIC（Position-independent code）技术成功让 C 的底层也能有动态的表现。
>  + 编译时在 Mach-O 文件 _DATA 段的符号表中为每一个被引用的系统 C 函数建立一个指针（8 字节的数据，放的全是 0），这个指针用于动态绑定时重定位到共享库中的函数实现。
>  + 在运行时当系统 C 函数被第一次调用时会动态绑定一次，然后将 Mach-O 中的 _DATA 段符号表中对应的指针，指向外部函数（其在共享库中的实际内存地址）。
>  
>  &emsp;fishhook 正是利用了 PIC 技术做了这么两个操作：
>  
>  + 将指向系统方法（外部函数）的指针重新进行绑定指向内部函数/自定义 C 函数。
>  + 将内部函数的指针在动态链接时指向系统方法的地址。
>  
>  &emsp;这样就把系统方法与自己定义的方法进行了交换，达到 HOOK 系统 C 函数（共享库中的）的目的。[fishhook的实现原理浅析](https://juejin.cn/post/6844903789783154702)


> &emsp;由于 iOS 系统中的 UIKit / Foundation 系统库每个应用都会通过 dyld 加载到内存中, 因此, 为了节约空间, 苹果将这些系统库放在了一个地方 : 动态库共享缓存区（dyld shared cache）。（Mac OS 一样有）。因此 , 类似 NSLog 的函数实现地址，并不会也不可能会在我们自己的工程的 Mach-O 中，那么我们的工程想要调用 NSLog 方法，如何能找到其真实的实现地址呢?
> &emsp;其流程如下：
>
> + 在工程编译时，所产生的 Mach-O 可执行文件中会预留出一段空间, 这个空间其实就是符号表（和懒加载和非懒加载符号指针表），存放在 _DATA 数据段中（因为 _DATA 段在运行时是可读可写的） 
> + 编译时：工程中所有引用了共享缓存区中的系统库方法 , 其指向的地址设置成符号地址，（例如工程中有一个 NSLog，那么编译时就会在 Mach-O 中创建一个 NSLog 的符号（以及在懒加载符号指针表中，有一个符号指针等待着指向 NSLog 函数地址），工程中的 NSLog 就指向这个符号） 
> + 运行时：当 dyld 将应用加载到内存中时，根据 load commands 中列出的需要加载哪些库文件，去做绑定的操作（以 NSLog 为例，dyld 就会去找到 Foundation 中 NSLog 的真实地址写到 _DATA 段的符号表中 NSLog 的符号上面（以及懒加载符号指针表中 NSLog 符号指针内）） 
>
> &emsp;这个过程被称为 PIC 技术。（Position Independent Code：位置代码独立） 
> &emsp;那么了解了系统函数的整个加载过程 , 我们再来看 fishhook 的函数名称 :
> rebind_symbols :: 重绑定符号 也就简单明了了.
> &emsp;其原理就是 :
>
> 将编译后系统库函数所指向的符号，在运行时重绑定到用户指定的函数地址，然后将原系统函数的真实地址赋值到用户指定的指针上。
>
> &emsp;那么再回头看自定义的 C 函数为什么 hook 不了 ?
> &emsp;那答案就很简单了 :
>
> &emsp;自定义 C 函数实际地址就在自己的 Mach-O 内 , 并没有符号指针和重绑定的过程。
> &emsp;编译时就已经确定了，并没有办法操作。 [iOS 逆向 - Hook / fishHook 原理与符号表](https://juejin.cn/post/6844903992904908814)


## 总结

&emsp;至此，fishhook 相关的内容就全部搞懂了，说实话收获巨大，特别是对 mach-o 文件的了解更加深入了，说实话 fishhook 总共 200 行的源码并不复杂，复杂就复杂在对 mach-o 符号相关的部分的结构要了如指掌，以及对动态链接库的重绑定机制也要了如指掌，感谢 fishhook，感谢 facebook！

&emsp;把 **运行时库的链接过程** 推迟到了 **运行时** 再进行，这就是 **动态链接（Dynamic Linking）** 的基本思想。

## 参考链接
**参考链接:🔗**
+ [LLDB Quick Start Guide](https://developer.apple.com/library/archive/documentation/IDEs/Conceptual/gdb_to_lldb_transition_guide/document/Introduction.html#//apple_ref/doc/uid/TP40012917-CH1-SW1)
+ [LLDB调试器使用简介](http://southpeak.github.io/2015/01/25/tool-lldb/)
+ [十 iOS逆向- hopper disassembler](https://www.jianshu.com/p/20077ceb2f75)
+ [iOS逆向之Hopper进阶](https://www.jianshu.com/p/384dc5bc1cb4)
+ [一文读懂fishhook原理](https://juejin.cn/post/6857699952563978247)
+ [地址空间布局随机化ASLR](https://www.jianshu.com/p/d3f094b4443d)
