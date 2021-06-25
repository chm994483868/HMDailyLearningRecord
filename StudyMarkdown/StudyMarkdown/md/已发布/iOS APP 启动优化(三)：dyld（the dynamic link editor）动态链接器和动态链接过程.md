# iOS APP 启动优化(三)：dyld（the dynamic link editor）动态链接器和动态链接过程

## 静态库与动态库

&emsp;TARGETS -> Build Phases -> Link Binary With Libraries -> (Add/Add Other...) 中我们可以添加多个系统库或我们自己的库，其中便可包含静态库和动态库。

&emsp;静态库通常以 .a .lib 或者 .framework 结尾，动态库以 .dylib .tbd .so .framework 结尾。（等等，.framework 可能是静态库也可能是动态库，后面我们会详细分析。）链接时，静态库会被完整的复制到可执行文件中，被多次使用就会有多份冗余拷贝，系统动态库链接时不复制，程序运行时由系统动态加载到内存中，供程序调用，系统只加载一次，多个程序共用，节省内存。

&emsp;Shift + command + n 创建 new project，在 Framework & library 中，Framework 选项默认是创建 Dynamic Library（动态库），Static Library 选项默认是创建 Static Library（静态库），创建完成的 Mach-O Type 的值告诉了我们他们对应的类型。 当然我们也能直接切换不同的 Mach-0 Type，如 Static Library 和 Dynamic Library 进行切换。而且从 Products 中看到默认情况下动态库是 .framework 后缀，静态库是 .a 后缀，同时还看到动态库是需要进行签名的，而静态库则不需要。

![截屏2021-05-09 11.00.43.png](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/bb53b5c4153b4ff6a47032975aabd997~tplv-k3u1fbpfcp-watermark.image)

![截屏2021-05-09 10.59.23.png](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/f48dc63bdc744453afeee353a127a7b7~tplv-k3u1fbpfcp-watermark.image)

![截屏2021-05-09 11.00.26.png](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/3ee1769190f44ad2a8a58db9692cfeae~tplv-k3u1fbpfcp-watermark.image)

> &emsp;如果我们创建的 framework 是动态库，那么我们直接在工程里使用的时候会报错：Reason: Image Not Found。需要在工程的 General 里的 Frameworks, Libraries, and Embedded Content 添加这个动态库并设置其 Embed 为 Embed & Sign 才能使用。
因为我们创建的这个动态库其实也不能给其他程序使用的，而你的 App Extension 和 APP 之间是需要使用这个动态库的。这个动态库可以 App Extension 和 APP 之间共用一份（App 和 Extension 的 Bundle 是共享的），因此苹果又把这种 Framework 称为 Embedded Framework，而我把这个动态库称为伪动态库。[iOS里的动态库和静态库](https://www.jianshu.com/p/42891fb90304)

&emsp;这里继续依我们的 Test_ipa_Simple 为例，并把上面我们自己构建的动态库 `DYLIB` 和 静态库 `STATICLIB` 导入  Test_ipa_Simple 中，直接运行的话会报如下找不到 `DYLIB.framework` 我们把其 Embed 置为 Embed & Sign 即可正常运行，如果报找不到 STATICLIB 的话，则是在 Build Settings 的 Library Search Paths 和 Header Search Paths 中正确的导入 STATICLIB 及 .h 的路径。（同时为了作为对比，我们在 Build Phases -> Link Binary With Libraries 中导入 `WebKit.framework`。）

```c++
dyld: Library not loaded: @rpath/DYLIB.framework/DYLIB
  Referenced from: /Users/hmc/Library/Developer/CoreSimulator/Devices/4E072E27-E586-4E81-A693-A02A3ED83DEC/data/Containers/Bundle/Application/1208BD23-B788-4BF7-A4CE-49FBA99BA330/Test_ipa_Simple.app/Test_ipa_Simple
  Reason: image not found
```

```c++
hmc@bogon Test_ipa_Simple.app % file Test_ipa_Simple 
Test_ipa_Simple: Mach-O 64-bit executable arm64
```

```c++
hmc@bogon DYLIB.framework % file DYLIB 
DYLIB: Mach-O 64-bit dynamically linked shared library arm64
```

```c++
hmc@bogon Debug-iphoneos % file libSTATICLIB.a 
libSTATICLIB.a: current ar archive random library
```

&emsp;我们创建的动态库和系统的动态库有什么区别呢？

1. 我们导入到项目中的我们自己创建的动态库是在我们自己应用的 .app 目录里面，只能自己的 App Extension 和 APP 使用。
2. 我们导入到项目中的系统的动态库是在系统目录里面，所有的程序都能使用。

&emsp;（我们在模拟器上运行的时候用 `NSBundle *bundel = [[NSBundle mainBundle] bundlePath];` 就能得到 .app 的路径，在第一篇中我们有详细讲解 .ipa 和 .app 目录中的内容，这里不再展开。）

&emsp;我们自己创建的动态库就在 .app 目录下的 Framework 文件夹里，对 Test_ipa_Simple 进行 Archive，导出并解压 Test_ipa_Simple.ipa，进入 Test_ipa_Simple.app 文件夹:

![截屏2021-05-09 14.21.36.png](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/8f1d41b6bb554260abdf9017440e98ad~tplv-k3u1fbpfcp-watermark.image)

&emsp;下面我们可以通过 MachOView 来验证一下 Test_ipa_Simple.app 文件夹中的 Test_ipa_Simple 可执行文件中的动态库（WebKit 和 DYLID）的链接地址。（@rpth 表示的其实就是 .app 下的 Framework 文件夹。）

![截屏2021-05-09 14.47.22.png](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/89315ecbe30b4da4b1143391269bc6a7~tplv-k3u1fbpfcp-watermark.image)

![截屏2021-05-09 14.47.32.png](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/64864747351f417fba7af6b28920b532~tplv-k3u1fbpfcp-watermark.image)

&emsp;系统在加载动态库时，会检查 framework 的签名，签名中必须包含 Team Identifier 并且 framework 和 host app 的 Team Identifier 必须一致。可以使用 `codesign -dv Test_ipa_Simple.app` 和 `codesign -dv DYLIB.framework` 来进行验证。

+ .framework 为什么既是静态库又是动态库 ？

> &emsp;系统的 .framework 是动态库，我们自己建立的.framework 一般都是静态库。但是现在你用 xcode 创建 Framework 的时候默认是动态库（Mach-O Type 默认是 Dynamic Library），一般打包成 SDK 给别人用的话都使用的是静态库，可以修改 Build Settings 的 Mach-O Type 为 Static Library。

+ 什么是 framework ?

> &emsp;Framework 是 Cocoa/Cocoa Touch 程序中使用的一种资源打包方式，可以将代码文件、头文件、资源文件、说明文档等集中在一起，方便开发者使用。一般如果是静态 Framework 的话，资源打包进 Framework 是读取不了的。静态 Framework 和 .a 文件都是编译进可执行文件里面的。只有动态 Framework 能在 .app 下面的 Framework 文件夹下看到，并读取 .framework 里的资源文件。
>
> &emsp;Cocoa/Cocoa Touch 开发框架本身提供了大量的 Framework，比如 Foundation.framework / UIKit.framework / AppKit.framework 等。需要注意的是，这些 framework 无一例外都是动态库。
>
> &emsp;平时我们用的第三方 SDK 的 framework 都是静态库，真正的动态库是上不了 AppStore 的(iOS 8 之后能上 AppStore，因为有个 App Extension，需要动态库支持)。

&emsp;我们用 use_frameworks! 生成的 pod 里面，pods 这个 PROJECT 下面会为每一个 pod 生成一个 target，比如有一个 pod 叫做 AFNetworking，那么就会有一个叫 AFNetworking 的 target，最后这个 target 生成的就是 AFNetworking.framework。

### 关于 use_frameworks!

&emsp;在使用 CocoaPods 的时候在 Podfile 里加入 use_frameworks! ，那么在编译的时候就会默认生成动态库，我们能看到每个源码 Pod 都会在 Pods 工程下面生成一个对应的动态库 Framework 的 target，我们能在这个 target 的 Build Settings -> Mach-O Type 看到默认设置是 Dynamic Library，也就是会生成一个动态 Framework，我们能在 Products 下面看到每一个 Pod 对应生成的动态库。

![截屏2021-05-10 08.32.00.png](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/7e4bca1afb7844c5b81bac837ab20687~tplv-k3u1fbpfcp-watermark.image)

&emsp;这些生成的动态库将链接到主项目给主工程使用，但是我们上面说过动态库需要在主工程 target 的 General -> Frameworks, Libraries, and Embedded Content 添加这个动态库并设置其 Embed 为 Embed & Sign 才能使用，而我们并没有在 Frameworks, Libraries, and Embedded Content 中看到这些动态库。那这是怎么回事呢，其实是 cocoapods 已经执行了脚本把这些动态库嵌入到了 .app 的 Framework 目录下，相当于在 Frameworks, Libraries, and Embedded Content 加入了这些动态库，我们能在主工程 target 的 Build Phase -> [CP]Embed Pods Frameworks 里看到执行的脚本。（"${PODS_ROOT}/Target Support Files/Pods-Test_ipa_Simple/Pods-Test_ipa_Simple-frameworks.sh"）

![截屏2021-05-10 08.22.43.png](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/b3c70781d38645aba9736a352ce5b513~tplv-k3u1fbpfcp-watermark.image)

&emsp;所以 Pod 默认是生成动态库，然后嵌入到 .app 下面的 Framework 文件夹里。我们去 Pods 工程的 target 里把 Build Settings -> Mach-O Type 设置为 Static Library。那么生成的就是静态库，但是 cocoapods 也会把它嵌入到 .app 的 Framework 目录下，而因为它是静态库，所以会报错：unrecognized selector sent to instanceunrecognized selector sent to instance 。[iOS里的动态库和静态库](https://www.jianshu.com/p/42891fb90304)

&emsp;动态库和静态的知识我们就延伸到这里吧，下面我们继续学习 **链接器** 相关的内容。

## 一组函数的执行顺序

```c++
// main.m 代码如下：

__attribute__((constructor)) void main_front() {
    printf("🦁🦁🦁 %s 执行 \n", __func__);
}

__attribute__((destructor)) void main_back() {
    printf("🦁🦁🦁 %s 执行 \n", __func__);
}

int main(int argc, char * argv[]) {
    NSLog(@"🦁🦁🦁 %s 执行", __func__);
    
//    NSString * appDelegateClassName;
//    @autoreleasepool {
//        // Setup code that might create autoreleased objects goes here.
//        appDelegateClassName = NSStringFromClass([AppDelegate class]);
//    }
//    return UIApplicationMain(argc, argv, nil, appDelegateClassName);
    
    return 0;
}

// ViewController.m 代码如下：

@implementation ViewController

+ (void)load {
    NSLog(@"🦁🦁🦁 %s 执行", __func__);
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
}

@end

// 运行后控制台打印如下：

2021-05-07 14:46:45.238651+0800 Test_ipa_Simple[43277:456220] 🦁🦁🦁 +[ViewController load] 执行
🦁🦁🦁 main_front 执行 
2021-05-07 14:46:45.242218+0800 Test_ipa_Simple[43277:456220] 🦁🦁🦁 main 执行
🦁🦁🦁 main_back 执行 
```

&emsp;根据控制台打印，可以看到 load 函数最先执行，然后是 constructor 属性修饰的 main_front 函数执行，然后是 main 函数执行，最后是 destructor 属性修饰的 main_back 函数执行。

&emsp;\_\_attribute__ 可以设置函数属性(Function Attribute)、变量属性(Variable Attribute)和类型属性(Type Attribute)。\_\_attribute__ 前后都有两个下划线，并且后面会紧跟一对原括弧，括弧里面是相应的 \_\_attribute__ 参数，\_\_attribute__ 语法格式为：`__attribute__((attribute-list))`。

&emsp;若函数被设定为 `constructor` 属性，则该函数会在 main 函数执行之前被自动的执行。类似的，若函数被设定为 `destructor` 属性，则该函数会在 main 函数执行之后或者 exit 被调用后被自动的执行。

&emsp;我们知道 .h、.m 的类在程序运行时先进行预编译，之后进行编译，编译完成后会进行汇编，在汇编结束后会进入一个阶段叫链接（把所有的代码链接到我们的程序中），最后会生成一个可执行文件。

&emsp;下面我们将了解 App 运行需要加载依赖库，需要加载 .h、.m 文件，那么谁来决定加载这些东西的先后顺序呢？这就是我们今天要说的主角 dyld（链接器）。就是由它来决定加载内容的先后顺序。

&emsp;app：images（镜像文件）-> dyld：读到内存（也就是加表里），启动主程序 - 进行 link - 一些必要对象的初始化（runtime，libsysteminit，OS_init 的初始化）。

&emsp;下面我们的目光聚焦在两个点上：链接器本身和链接过程的解读。

### Dyld 探索

&emsp;macOS 的 dyld 程序位置在 `/usr/lib/dyld`   

![截屏2021-05-12 08.08.33.png](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/909ef5653e4c40479d3b43a437dcb9e7~tplv-k3u1fbpfcp-watermark.image)

```c++
hmc@bogon Simple % file dyld
dyld: Mach-O universal binary with 3 architectures: [x86_64:Mach-O 64-bit dynamic linker x86_64] [i386:Mach-O dynamic linker i386] [arm64e]
dyld (for architecture x86_64):    Mach-O 64-bit dynamic linker x86_64
dyld (for architecture i386):    Mach-O dynamic linker i386
dyld (for architecture arm64e):    Mach-O 64-bit dynamic linker arm64e
```

&emsp;可以看到我电脑里面的 dyld 是一个 fat Mach-O 文件，同时集合了三个平台 x86_64、i386、arm64e。 

&emsp;dyld 是英文 the dynamic link editor 的简写，翻译过来就是动态链接器，是苹果操作系统的一个重要的组成部分。在 iOS/macOS 系统中，仅有很少量的进程只需要内核就能完成加载，基本上所有的进程都是动态链接的，所以 Mach-O 镜像文件中会有很多对外部的库和符号的引用，但是这些引用并不能直接用，在启动时还必须要通过这些引用进行内容的填补，这个填补工作就是由动态链接器 dyld 来完成的，也就是符号绑定。系统内核在加载 Mach-O 文件时，都需要用 dyld 链接程序，将程序加载到内存中。

&emsp;在编写项目时，我们大概最先接触到的可执行的代码是 main 和 load 函数，当我们不重写某个类的 load 函数时，大概会觉得 main 是我们 APP 的入口函数，当我们重写了某个类的 load 函数后，我们又已知的 load 函数是在 main 之前执行的。（上一小节我们也有说过 \_\_attribute__((constructor)) 修饰的 C  函数也会在 main 之前执行）那么从这里可以看出到我们的 APP 真的执行到 main 函数之前其实已经做了一些 APP 的 加载操作，那具体都有哪些呢，我们可以在 load 函数中打断点，然后打印出函数调用堆栈来发现一些端倪。如下图所示：

&emsp;在模拟器下的截图，其中的 sim 表示当前是在 TARGET_OS_SIMULATOR 环境下：

![截屏2021-05-13 08.11.38.png](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/89b62441b6d646b39966c7e2bf52abdb~tplv-k3u1fbpfcp-watermark.image)

```c++
(lldb) bt
* thread #1, queue = 'com.apple.main-thread', stop reason = breakpoint 1.1
    frame #0: 0x0000000100a769c7 Test_ipa_Simple`+[ViewController load](self=ViewController, _cmd="load") at ViewController.m:17:5
    frame #1: 0x00007fff201804e3 libobjc.A.dylib`load_images + 1442
    frame #2: 0x0000000108cb5e54 dyld_sim`dyld::notifySingle(dyld_image_states, ImageLoader const*, ImageLoader::InitializerTimingList*) + 425
    frame #3: 0x0000000108cc4887 dyld_sim`ImageLoader::recursiveInitialization(ImageLoader::LinkContext const&, unsigned int, char const*, ImageLoader::InitializerTimingList&, ImageLoader::UninitedUpwards&) + 437
    frame #4: 0x0000000108cc2bb0 dyld_sim`ImageLoader::processInitializers(ImageLoader::LinkContext const&, unsigned int, ImageLoader::InitializerTimingList&, ImageLoader::UninitedUpwards&) + 188
    frame #5: 0x0000000108cc2c50 dyld_sim`ImageLoader::runInitializers(ImageLoader::LinkContext const&, ImageLoader::InitializerTimingList&) + 82
    frame #6: 0x0000000108cb62a9 dyld_sim`dyld::initializeMainExecutable() + 199
    frame #7: 0x0000000108cbad50 dyld_sim`dyld::_main(macho_header const*, unsigned long, int, char const**, char const**, char const**, unsigned long*) + 4431
    frame #8: 0x0000000108cb51c7 dyld_sim`start_sim + 122
    frame #9: 0x0000000200dea57a dyld`dyld::useSimulatorDyld(int, macho_header const*, char const*, int, char const**, char const**, char const**, unsigned long*, unsigned long*) + 2093
    frame #10: 0x0000000200de7df3 dyld`dyld::_main(macho_header const*, unsigned long, int, char const**, char const**, char const**, unsigned long*) + 1199
    frame #11: 0x0000000200de222b dyld`dyldbootstrap::start(dyld3::MachOLoaded const*, int, char const**, dyld3::MachOLoaded const*, unsigned long*) + 457
  * frame #12: 0x0000000200de2025 dyld`_dyld_start + 37
(lldb) 
```

&emsp;在真机下的截图，相比较与模拟器环境看到是少了 dyld\`dyld::useSimulatorDyld 和 dyld_sim\`start_sim 调用（切换到模拟器环境），后序的函数调用基本都是一样的，除了运行环境不同外（dyld_sim / dyld）。 

![截屏2021-05-15 08.06.39.png](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/297c64db8ca44b99999fbd81146e4c3e~tplv-k3u1fbpfcp-watermark.image)

```c++
(lldb) bt
* thread #1, queue = 'com.apple.main-thread', stop reason = breakpoint 1.1
  * frame #0: 0x00000001043f19c0 Test_ipa_Simple`+[ViewController load](self=ViewController, _cmd="load") at ViewController.m:17:5
    frame #1: 0x00000001a2bc925c libobjc.A.dylib`load_images + 944
    frame #2: 0x00000001046ea21c dyld`dyld::notifySingle(dyld_image_states, ImageLoader const*, ImageLoader::InitializerTimingList*) + 464
    frame #3: 0x00000001046fb5e8 dyld`ImageLoader::recursiveInitialization(ImageLoader::LinkContext const&, unsigned int, char const*, ImageLoader::InitializerTimingList&, ImageLoader::UninitedUpwards&) + 512
    frame #4: 0x00000001046f9878 dyld`ImageLoader::processInitializers(ImageLoader::LinkContext const&, unsigned int, ImageLoader::InitializerTimingList&, ImageLoader::UninitedUpwards&) + 184
    frame #5: 0x00000001046f9940 dyld`ImageLoader::runInitializers(ImageLoader::LinkContext const&, ImageLoader::InitializerTimingList&) + 92
    frame #6: 0x00000001046ea6d8 dyld`dyld::initializeMainExecutable() + 216
    frame #7: 0x00000001046ef928 dyld`dyld::_main(macho_header const*, unsigned long, int, char const**, char const**, char const**, unsigned long*) + 5216
    frame #8: 0x00000001046e9208 dyld`dyldbootstrap::start(dyld3::MachOLoaded const*, int, char const**, dyld3::MachOLoaded const*, unsigned long*) + 396
    frame #9: 0x00000001046e9038 dyld`_dyld_start + 56
(lldb) 
```

&emsp;可以看到从 \_dyld_start 函数开始直到 +[ViewController load] 函数，中间的函数调用栈都集中在了 dyld/dyld_sim。（最后的 libobjc.A.dylib`load_images 调用，后面我们会详细分析）下面我们可以通过 [dyld 的源码](https://opensource.apple.com/tarballs/dyld/) 来一一分析上面函数调用堆栈中出现的函数。

#### \_dyld_start
&emsp;\_dyld_start 是汇编函数，这里我们只看 \_\_arm64__ && !TARGET_OS_SIMULATOR 平台下的。

```c++
#if __arm64__ && !TARGET_OS_SIMULATOR
    .text
    .align 2
    .globl __dyld_start
__dyld_start:
    mov     x28, sp // mov 数据传送指令 x28 -> sp
    and     sp, x28, #~15        // force 16-byte alignment of stack and 逻辑与指令 ((x28 & #~15) & sp) -> sp
    mov    x0, #0
    mov    x1, #0
    stp    x1, x0, [sp, #-16]!    // make aligned terminating frame
    mov    fp, sp            // set up fp to point to terminating frame
    sub    sp, sp, #16             // make room for local variables sub 减法指令 
    
#if __LP64__
    ldr     x0, [x28]               // get app's mh into x0
    ldr     x1, [x28, #8]           // get argc into x1 (kernel passes 32-bit int argc as 64-bits on stack to keep alignment)
    add     x2, x28, #16            // get argv into x2
#else
    ldr     w0, [x28]               // get app's mh into x0
    ldr     w1, [x28, #4]           // get argc into x1 (kernel passes 32-bit int argc as 64-bits on stack to keep alignment)
    add     w2, w28, #8             // get argv into x2
#endif

    adrp    x3,___dso_handle@page
    add     x3,x3,___dso_handle@pageoff // get dyld's mh in to x4
    mov    x4,sp                   // x5 has &startGlue
    
    // ⬇️⬇️⬇️⬇️⬇️ 这里调用 dyldbootstrap::start 是一个入口  
    // call dyldbootstrap::start(app_mh, argc, argv, dyld_mh, &startGlue)
    
    bl    __ZN13dyldbootstrap5startEPKN5dyld311MachOLoadedEiPPKcS3_Pm
    mov    x16,x0                  // save entry point address in x16
    
#if __LP64__
    ldr     x1, [sp]
#else
    ldr     w1, [sp]
#endif

    cmp    x1, #0
    b.ne    Lnew

    // LC_UNIXTHREAD way, clean up stack and jump to result
#if __LP64__
    add    sp, x28, #8             // restore unaligned stack pointer without app mh
#else
    add    sp, x28, #4             // restore unaligned stack pointer without app mh
#endif

#if __arm64e__
    braaz   x16                     // jump to the program's entry point
#else
    br      x16                     // jump to the program's entry point
#endif

    // LC_MAIN case, set up stack for call to main()
Lnew:    mov    lr, x1            // simulate return address into _start in libdyld.dylib

#if __LP64__
    ldr    x0, [x28, #8]       // main param1 = argc
    add    x1, x28, #16        // main param2 = argv
    add    x2, x1, x0, lsl #3
    add    x2, x2, #8          // main param3 = &env[0]
    mov    x3, x2
Lapple:    ldr    x4, [x3]
    add    x3, x3, #8
#else
    ldr    w0, [x28, #4]       // main param1 = argc
    add    x1, x28, #8         // main param2 = argv
    add    x2, x1, x0, lsl #2
    add    x2, x2, #4          // main param3 = &env[0]
    mov    x3, x2
Lapple:    ldr    w4, [x3]
    add    x3, x3, #4
#endif

    cmp    x4, #0
    b.ne    Lapple            // main param4 = apple
    
#if __arm64e__
    braaz   x16
#else
    br      x16
#endif

#endif // __arm64__ && !TARGET_OS_SIMULATOR
```

#### dyldbootstrap::start
&emsp;然后看到汇编函数 \_\_dyld_start 内部调用了 `dyldbootstrap::start(app_mh, argc, argv, dyld_mh, &startGlue)` 函数，即 dyldbootstrap 命名空间中的 start 函数，namespace dyldbootstrap 定义在 dyldInitialization.cpp 中，它的内容超简单，内部就定义了 start 和 rebaseDyld 两个函数，从命名空间的名字中我们已经能猜到一些它的作用：用来进行 dyld 的初始化，将 dyld 引导到可运行状态（Code to bootstrap dyld into a runnable state）。下面我们一起看下其中的 start 的函数。

```c++
//
//  This is code to bootstrap dyld.  This work in normally done for a program by dyld and crt.
//  In dyld we have to do this manually.
//
uintptr_t start(const dyld3::MachOLoaded* appsMachHeader, int argc, const char* argv[],
                const dyld3::MachOLoaded* dyldsMachHeader, uintptr_t* startGlue)
{

    // Emit kdebug tracepoint to indicate dyld bootstrap has started <rdar://46878536>
    // 发出 kdebug tracepoint 以指示 dyld bootstrap 已启动
    dyld3::kdebug_trace_dyld_marker(DBG_DYLD_TIMING_BOOTSTRAP_START, 0, 0, 0, 0);

    // if kernel had to slide dyld, we need to fix up load sensitive locations
    // we have to do this before using any global variables
    rebaseDyld(dyldsMachHeader); // 用于重定位（设置虚拟地址偏移，这里的偏移主要用于重定向）

    // kernel sets up env pointer to be just past end of agv array
    // 内核将 env 指针设置为刚好超出 agv 数组的末尾
    const char** envp = &argv[argc+1];
    
    // kernel sets up apple pointer to be just past end of envp array
    // 内核将 apple 指针设置为刚好超出 envp 数组的末尾
    const char** apple = envp;
    while(*apple != NULL) { ++apple; }
    ++apple;

    // set up random value for stack canary
    // 为 stack canary 设置随机值
    __guard_setup(apple);

#if DYLD_INITIALIZER_SUPPORT // 前面 DYLD_INITIALIZER_SUPPORT 宏的值是 0，所以这里 #if 内部的内容并不会执行 
    // run all C++ initializers inside dyld
    // 在 dyld 中运行所有 C++ 初始化器
    //（这里可以参考 《Hook static initializers》：https://blog.csdn.net/majiakun1/article/details/99413403）
    //（帮助我们了解学习 C++ initializers）
    runDyldInitializers(argc, argv, envp, apple);
#endif

    _subsystem_init(apple);

    // now that we are done bootstrapping dyld, call dyld's main
    // 现在我们完成了 bootstrapping dyld，调用 dyld 的 main（进入 dyld 的主函数）
    uintptr_t appsSlide = appsMachHeader->getSlide();
    return dyld::_main((macho_header*)appsMachHeader, appsSlide, argc, argv, envp, apple, startGlue);
}
```

&emsp;在 `start` 函数中 `appsMachHeader` 和 `dyldsMachHeader` 两个参数的类型是 `const dyld3::MachOLoaded*`（它们两个参数可以理解为我们程序的可执行文件和 dyld 程序的 header 的地址），在 dyld/dyld3/MachOLoaded.h 文件中可看到命名空间 dyld3 中定义的 `struct VIS_HIDDEN MachOLoaded : public MachOFile`，即 `MachOLoaded` 结构体公开继承自 `MachOFile` 结构体，在 dyld/dyld3/MachOFile.h 文件中可看到命名空间 dyld3 中定义的 `struct VIS_HIDDEN MachOFile : mach_header`，即 `MachOFile` 结构体继承自 `mach_header` 结构体。

&emsp;在 `return dyld::_main((macho_header*)appsMachHeader, appsSlide, argc, argv, envp, apple, startGlue);` 中，我们看到 `appsMachHeader` 被强转为了 `macho_header*`，那我们接着则看下 `macho_header` 的定义。在 dyld/src/ImageLoader.h 中可看到在 \_\_LP64__ 下 `macho_header` 公开继承自 `mach_header_64` 其他平台则是继承自 `mach_header`（它们的名字仅差一个 `0`），`mach_header` 在前一篇 《iOS APP 启动优化(一)：ipa(iPhone application archive) 包和 Mach-O(Mach Object file format) 概述》中我们有详细分析过：

```c++
#if __LP64__
    struct macho_header                : public mach_header_64  {};
    struct macho_nlist                : public nlist_64  {};    
#else
    struct macho_header                : public mach_header  {};
    struct macho_nlist                : public nlist  {};    
#endif
```

> &emsp;Mach-O 文件的 Header 部分对应的数据结构定义在 darwin-xnu/EXTERNAL_HEADERS/mach-o/loader.h 中，`struct mach_header` 和 `struct mach_header_64` 分别对应 32-bit architectures 和 64-bit architectures。（对于 32/64-bit architectures，32/64 位的 mach header 都出现在 Mach-O 文件的最开头。）

```c++
struct mach_header_64 {
    uint32_t    magic;        /* mach magic number identifier */
    cpu_type_t    cputype;    /* cpu specifier */
    cpu_subtype_t    cpusubtype;    /* machine specifier */
    uint32_t    filetype;    /* type of file */
    uint32_t    ncmds;        /* number of load commands */
    uint32_t    sizeofcmds;    /* the size of all the load commands */
    uint32_t    flags;        /* flags */
    uint32_t    reserved;    /* reserved */
};
```

&emsp;综上，`MachOLoaded -> MachOFile -> mach_header`。MachOFile 继承 mach_header 使其拥有 mach_header 结构体中所有的成员变量，然后 MachOFile 定义中则声明了一大组针对 Mach-O 的 Header 的函数，例如获取架构名、CPU 类型等。MachOLoaded 继承自 MachOFile 其定义中则声明了一组加载 Mach-O 的 Header 的函数。 

#### dyld::_main 

&emsp;下面我们接着看 `dyld::_main` 函数。首先是根据函数调用方式可以看到 \_main 函数是属于 dyld 命名空间的，在 dyld/src/dyld2.cpp 中可看到 namespace dyld 的定义，在 dyld2.h 和 dyld2.cpp 中可看到分别进行了 `uintptr_t _main(const macho_header* mainExecutableMH, uintptr_t mainExecutableSlide, int argc, const char* argv[], const char* envp[], const char* apple[], uintptr_t* startGlue)` 的声明和定义。

&emsp;首先是 \_main 函数的注释：

> &emsp;Entry point for dyld.  The kernel loads dyld and jumps to __dyld_start which sets up some registers and call this function.
> Returns address of main() in target program which __dyld_start jumps to
>
> &emsp;dyld 的入口点。内核加载 dyld 并跳到 \_\_dyld_start 设置一些寄存器并调用此函数。返回目标程序中的 main() 地址，\_\_dyld_start 跳到该地址。

&emsp;下面我们沿着 \_main 函数的定义，来分析 \_main 函数，由于该函数定义内部根据不同的平台、不同的架构作了不同的定义和调用，所以函数定义超长，总共有 800 多行，这里只对必要的代码段进行摘录分析，其中最重要的部分则是分析函数返回值 `uintptr_t result` 在函数内部的赋值情况。

&emsp;在 dyld/src/dyld2.cpp 的 7117 行和 7127 行看看到如下代码：

```c++
...
// find entry point for main executable
result = (uintptr_t)sMainExecutable->getEntryFromLC_MAIN();

...
// main executable uses LC_UNIXTHREAD, dyld needs to let "start" in program set up for main()
result = (uintptr_t)sMainExecutable->getEntryFromLC_UNIXTHREAD();
...
```

&emsp;`sMainExecutable` 是一个全局变量：`static ImageLoaderMachO* sMainExecutable = NULL;`，在 dyld/src/dyld2.cpp 的 6861 行可看到对其进行实例化。

```c++
// instantiate ImageLoader for main executable
sMainExecutable = instantiateFromLoadedImage(mainExecutableMH, mainExecutableSlide, sExecPath);
gLinkContext.mainExecutable = sMainExecutable;
gLinkContext.mainExecutableCodeSigned = hasCodeSignatureLoadCommand(mainExecutableMH);
```

##### ImageLoaderMachO

&emsp;这里我们首先看一下 `ImageLoaderMachO` 类（ImageLoaderMachO is a subclass of ImageLoader which loads mach-o format files.）的定义，`instantiateFromLoadedImage` 函数返回一个 `ImageLoaderMachO` 指针，在 dyld/src/ImageLoaderMachO.h 中可看到 `class ImageLoaderMachO : public ImageLoader` 的定义，`ImageLoaderMachO` 类公开继承自 `ImageLoader` 类。`ImageLoader` 是一个抽象基类。为了支持加载特定的可执行文件格式，可以创建 `ImageLoader` 的一个具体子类。对于使用中的每个可执行文件（dynamic shared object），将实例化一个 `ImageLoader`。`ImageLoader` 基类负责将 images 链接在一起，但它对任何特定的文件格式一无所知，主要由其特定子类来实现。如 `ImageLoaderMachO` 是 `ImageLoader` 的特定子类，可加载 mach-o 格式的文件。（例如还有 `class ImageLoaderMegaDylib : public ImageLoader` ImageLoaderMegaDylib is the concrete subclass of ImageLoader which represents all dylibs in the shared cache.）

##### instantiateFromLoadedImage

&emsp;下面我们接着看 `instantiateFromLoadedImage` 函数实现，根据入参 `const macho_header* mh` 它内部直接调用 `ImageLoaderMachO` 的 `instantiateMainExecutable` 函数进行主可执行文件的实例化（即创建 ImageLoader 对象）。对于程序中需要的依赖库、插入库，会创建一个对应的 image 对象，对这些 image 进行链接，调用各 image 的初始化方法等等，包括对 runtime 的初始化。然后将 image 加载到 imagelist 中，所以我们在 xcode 中使用 image list 命令查看的第一个便是我们的 mach-o，最后返回根据我们的主可执行文件创建的 ImageLoader 对象的地址，即这里 `sMainExecutable` 就是创建后的主程序。  

```c++
// The kernel maps in main executable before dyld gets control.  We need to 
// make an ImageLoader* for the already mapped in main executable.
static ImageLoaderMachO* instantiateFromLoadedImage(const macho_header* mh, uintptr_t slide, const char* path)
{
    // try mach-o loader
    // isCompatibleMachO 是检查 mach-o 的 subtype 是否支持当前的 cpu 
//    if ( isCompatibleMachO((const uint8_t*)mh, path) ) {
        ImageLoader* image = ImageLoaderMachO::instantiateMainExecutable(mh, slide, path, gLinkContext);
        
        // 将 image 加载到 imagelist 中，所以我们在 xcode 中使用 image list 命令查看的第一个便是我们的 mach-o
        addImage(image);
        
        return (ImageLoaderMachO*)image;
//    }
    
//    throw "main executable not a known format";
}
```

##### ImageLoaderMachO::instantiateMainExecutable 

&emsp;下面我们看一下 `ImageLoaderMachO::instantiateMainExecutable` 函数的定义，它的功能便是为 main executable 创建 image。

```c++
// create image for main executable
ImageLoader* ImageLoaderMachO::instantiateMainExecutable(const macho_header* mh, uintptr_t slide, const char* path, const LinkContext& context)
{
    //dyld::log("ImageLoader=%ld, ImageLoaderMachO=%ld, ImageLoaderMachOClassic=%ld, ImageLoaderMachOCompressed=%ld\n",
    //    sizeof(ImageLoader), sizeof(ImageLoaderMachO), sizeof(ImageLoaderMachOClassic), sizeof(ImageLoaderMachOCompressed));
    bool compressed;
    unsigned int segCount;
    unsigned int libCount;
    const linkedit_data_command* codeSigCmd;
    const encryption_info_command* encryptCmd;
    sniffLoadCommands(mh, path, false, &compressed, &segCount, &libCount, context, &codeSigCmd, &encryptCmd);
    // instantiate concrete class based on content of load commands
    if ( compressed ) 
        return ImageLoaderMachOCompressed::instantiateMainExecutable(mh, slide, path, segCount, libCount, context);
    else
#if SUPPORT_CLASSIC_MACHO
        return ImageLoaderMachOClassic::instantiateMainExecutable(mh, slide, path, segCount, libCount, context);
#else
        throw "missing LC_DYLD_INFO load command";
#endif
}
```

&emsp;其中的 `sniffLoadCommands` 函数，它也是 `ImageLoaderMachO` 类的一个函数。

&emsp;`class ImageLoaderMachOCompressed : public ImageLoaderMachO` 即 `ImageLoaderMachOCompressed` 是 `ImageLoaderMachO` 的子类：ImageLoaderMachOCompressed is the concrete subclass of ImageLoader which loads mach-o files that use the compressed LINKEDIT format。

##### sniffLoadCommands

&emsp;下面我们看一下 `sniffLoadCommands` 函数的定义，此函数过长，我们只看它的部分内容。

```c++
// determine if this mach-o file has classic or compressed LINKEDIT and number of segments it has
void ImageLoaderMachO::sniffLoadCommands(const macho_header* mh, const char* path, bool inCache, bool* compressed,
                                            unsigned int* segCount, unsigned int* libCount, const LinkContext& context,
                                            const linkedit_data_command** codeSigCmd,
                                            const encryption_info_command** encryptCmd)
{
    *compressed = false;
    *segCount = 0;
    *libCount = 0;
    *codeSigCmd = NULL;
    *encryptCmd = NULL;

    const uint32_t cmd_count = mh->ncmds;
    const uint32_t sizeofcmds = mh->sizeofcmds;
    ...
```

&emsp;确定此 mach-o 文件是  classic 或者 compressed LINKEDIT 且确定 mach-o 可执行文件的 segments 的数量。然后我们可以看到对 mach-o 文件中的 Load Commands 中各个段的确认，如 LC_DYLD_INFO、LC_DYLD_INFO_ONLY、LC_LOAD_DYLIB、LC_SEGMENT_64、LC_CODE_SIGNATURE 等等。

```c++
...
switch (cmd->cmd) {
    case LC_DYLD_INFO:
    case LC_DYLD_INFO_ONLY:
        if ( cmd->cmdsize != sizeof(dyld_info_command) )
            throw "malformed mach-o image: LC_DYLD_INFO size wrong";
        dyldInfoCmd = (struct dyld_info_command*)cmd;
        *compressed = true;
        break;
    case LC_DYLD_CHAINED_FIXUPS:
        if ( cmd->cmdsize != sizeof(linkedit_data_command) )
            throw "malformed mach-o image: LC_DYLD_CHAINED_FIXUPS size wrong";
        chainedFixupsCmd = (struct linkedit_data_command*)cmd;
        *compressed = true;
        break;
    case LC_DYLD_EXPORTS_TRIE:
        if ( cmd->cmdsize != sizeof(linkedit_data_command) )
            throw "malformed mach-o image: LC_DYLD_EXPORTS_TRIE size wrong";
        exportsTrieCmd = (struct linkedit_data_command*)cmd;
        break;
    case LC_SEGMENT_COMMAND:
        segCmd = (struct macho_segment_command*)cmd;
...        
```

&emsp;`sniffLoadCommands(mh, path, false, &compressed, &segCount, &libCount, context, &codeSigCmd, &encryptCmd);` 函数调用我们就看到这里，然后下面的 `return ImageLoaderMachOCompressed::instantiateMainExecutable(mh, slide, path, segCount, libCount, context);` 和 `return ImageLoaderMachOClassic::instantiateMainExecutable(mh, slide, path, segCount, libCount, context);` 则都是调用 ImageLoaderMachO 的构造函数，创建 ImageLoaderMachO 对象。

&emsp;`sMainExecutable` 创建完毕，我们接着分析 `dyld::_main` 函数。

#### getHostInfo

&emsp;调用 getHostInfo(mainExecutableMH, mainExecutableSlide); 函数来获取 Mach-O 头部信息中的当前运行架构信息，仅是为了给 sHostCPU 和 sHostCPUsubtype 两个全局变量赋值。getHostInfo 函数虽然有两个参数 mainExecutableMH 和 mainExecutableSlide 但是实际都只是为了在 __x86_64__ && !TARGET_OS_SIMULATOR 下使用的。

```c++
static void getHostInfo(const macho_header* mainExecutableMH, uintptr_t mainExecutableSlide)
{
#if CPU_SUBTYPES_SUPPORTED
#if __ARM_ARCH_7K__
    sHostCPU        = CPU_TYPE_ARM;
    sHostCPUsubtype = CPU_SUBTYPE_ARM_V7K;
#elif __ARM_ARCH_7A__
    sHostCPU        = CPU_TYPE_ARM;
    sHostCPUsubtype = CPU_SUBTYPE_ARM_V7;
#elif __ARM_ARCH_6K__
    sHostCPU        = CPU_TYPE_ARM;
    sHostCPUsubtype = CPU_SUBTYPE_ARM_V6;
#elif __ARM_ARCH_7F__
    sHostCPU        = CPU_TYPE_ARM;
    sHostCPUsubtype = CPU_SUBTYPE_ARM_V7F;
#elif __ARM_ARCH_7S__
    sHostCPU        = CPU_TYPE_ARM;
    sHostCPUsubtype = CPU_SUBTYPE_ARM_V7S;
#elif __ARM64_ARCH_8_32__
    sHostCPU        = CPU_TYPE_ARM64_32;
    sHostCPUsubtype = CPU_SUBTYPE_ARM64_32_V8;
#elif __arm64e__
    sHostCPU        = CPU_TYPE_ARM64;
    sHostCPUsubtype = CPU_SUBTYPE_ARM64E;
#elif __arm64__
    sHostCPU        = CPU_TYPE_ARM64;
    sHostCPUsubtype = CPU_SUBTYPE_ARM64_V8;
#else
    struct host_basic_info info;
    mach_msg_type_number_t count = HOST_BASIC_INFO_COUNT;
    mach_port_t hostPort = mach_host_self();
    kern_return_t result = host_info(hostPort, HOST_BASIC_INFO, (host_info_t)&info, &count);
    if ( result != KERN_SUCCESS )
        throw "host_info() failed";
    sHostCPU        = info.cpu_type;
    sHostCPUsubtype = info.cpu_subtype;
    mach_port_deallocate(mach_task_self(), hostPort);
  #if __x86_64__
      // host_info returns CPU_TYPE_I386 even for x86_64.  Override that here so that
      // we don't need to mask the cpu type later.
      sHostCPU = CPU_TYPE_X86_64;
    #if !TARGET_OS_SIMULATOR
      sHaswell = (sHostCPUsubtype == CPU_SUBTYPE_X86_64_H);
      // <rdar://problem/18528074> x86_64h: Fall back to the x86_64 slice if an app requires GC.
      if ( sHaswell ) {
        if ( isGCProgram(mainExecutableMH, mainExecutableSlide) ) {
            // When running a GC program on a haswell machine, don't use and 'h slices
            sHostCPUsubtype = CPU_SUBTYPE_X86_64_ALL;
            sHaswell = false;
            gLinkContext.sharedRegionMode = ImageLoader::kDontUseSharedRegion;
        }
      }
    #endif
  #endif
#endif
#endif
}
```

#### forEachSupportedPlatform

&emsp;在此块区域中我们看到了我们的老朋友 block 在 C/C++ 函数中的使用。

&emsp;判断 `mainExecutableMH` 支持的平台以及当前的版本信息等。

```c++
// Set the platform ID in the all image infos so debuggers can tell the process type
// FIXME: This can all be removed once we make the kernel handle it in rdar://43369446
// The host may not have the platform field in its struct, but there's space for it in the padding, so always set it
{
    __block bool platformFound = false;
    ((dyld3::MachOFile*)mainExecutableMH)->forEachSupportedPlatform(^(dyld3::Platform platform, uint32_t minOS, uint32_t sdk) {
        if (platformFound) {
            halt("MH_EXECUTE binaries may only specify one platform");
        }
        gProcessInfo->platform = (uint32_t)platform;
        platformFound = true;
    });
    if (gProcessInfo->platform == (uint32_t)dyld3::Platform::unknown) {
        // There were no platforms found in the binary. This may occur on macOS for alternate toolchains and old binaries.
        // It should never occur on any of our embedded platforms.
#if TARGET_OS_OSX
        gProcessInfo->platform = (uint32_t)dyld3::Platform::macOS;
#else
        halt("MH_EXECUTE binaries must specify a minimum supported OS version");
#endif
    }
}
...
```

#### setContext

&emsp;`setContext` 是一个静态全局函数，主要为 `ImageLoader::LinkContext gLinkContext;` 这个全局变量的各项属性以及函数指针赋值。设置 crash 以及 log 地址，设置上下文信息。 
```c++
CRSetCrashLogMessage("dyld: launch started");

setContext(mainExecutableMH, argc, argv, envp, apple);
```

#### configureProcessRestrictions

&emsp;设置环境变量，envp 就是 \_main 函数的参数，它是所有环境变量的数组，就是将环境变量插入进去。
```c++
configureProcessRestrictions(mainExecutableMH, envp);
```

#### checkSharedRegionDisable

&emsp;对共享缓存进行处理。 
```c++
// load shared cache
checkSharedRegionDisable((dyld3::MachOLoaded*)mainExecutableMH, mainExecutableSlide);
```

#### instantiateFromLoadedImage 

&emsp;主程序的初始化。
```c++
// instantiate ImageLoader for main executable
sMainExecutable = instantiateFromLoadedImage(mainExecutableMH, mainExecutableSlide, sExecPath);
gLinkContext.mainExecutable = sMainExecutable;
gLinkContext.mainExecutableCodeSigned = hasCodeSignatureLoadCommand(mainExecutableMH);
```

#### loadInsertedDylib

&emsp;插入动态库。

```c++
// load any inserted libraries
if ( sEnv.DYLD_INSERT_LIBRARIES != NULL ) {
    for (const char* const* lib = sEnv.DYLD_INSERT_LIBRARIES; *lib != NULL; ++lib) 
        loadInsertedDylib(*lib);
}
```

#### link

&emsp;link 主程序。

```c++
link(sMainExecutable, sEnv.DYLD_BIND_AT_LAUNCH, true, ImageLoader::RPathChain(NULL, NULL), -1);
sMainExecutable->setNeverUnloadRecursive();
```

&emsp;link 我们所有的 image（通过上面两个可以知道，必须先 link 主程序，然后在 link 所有的 image） 

```c++
// link any inserted libraries
// do this after linking main executable so that any dylibs pulled in by inserted 
// dylibs (e.g. libSystem) will not be in front of dylibs the program uses
if ( sInsertedDylibCount > 0 ) {
    for(unsigned int i=0; i < sInsertedDylibCount; ++i) {
        ImageLoader* image = sAllImages[i+1];
        link(image, sEnv.DYLD_BIND_AT_LAUNCH, true, ImageLoader::RPathChain(NULL, NULL), -1);
        image->setNeverUnloadRecursive();
    }
    if ( gLinkContext.allowInterposing ) {
        // only INSERTED libraries can interpose
        // register interposing info after all inserted libraries are bound so chaining works
        for(unsigned int i=0; i < sInsertedDylibCount; ++i) {
            ImageLoader* image = sAllImages[i+1];
            image->registerInterposing(gLinkContext);
        }
    }
}
```

#### weakBind

&emsp;绑定弱符号。

```c++
// <rdar://problem/12186933> do weak binding only after all inserted images linked
sMainExecutable->weakBind(gLinkContext);
```

#### initializeMainExecutable

&emsp;执行所有的初始化方法。

```c++
// run all initializers
initializeMainExecutable(); 
```

#### notifyMonitoringDyldMain

&emsp;查找 main 函数入口

```c++
// notify any montoring proccesses that this process is about to enter main()
notifyMonitoringDyldMain();
```

&emsp;设置运行环境 -> 加载共享缓存 -> 实例化主程序 -> 插入加载动态库 -> 连接主程序 -> 链接插入的动态库 -> 执行弱符号绑定 -> 执行初始化方法 -> 查找入口并返回（） 

### initializeMainExecutable

&emsp;上面我们分析了 main 的总体流程，其中 `initializeMainExecutable` 函数进行了所有的 initializers，下面我们详细学习一下 `initializeMainExecutable` 函数。

```c++
void initializeMainExecutable()
{
    // record that we've reached this step（记录，我们已经达到了这一步）
    gLinkContext.startedInitializingMainExecutable = true;

    // run initialzers for any inserted dylibs（为任何插入的 dylibs 运行初始化器）
    ImageLoader::InitializerTimingList initializerTimes[allImagesCount()];
    initializerTimes[0].count = 0;
    const size_t rootCount = sImageRoots.size();
    if ( rootCount > 1 ) {
        for(size_t i=1; i < rootCount; ++i) {
            sImageRoots[i]->runInitializers(gLinkContext, initializerTimes[0]);
        }
    }
    
    // run initializers for main executable and everything it brings up 
    sMainExecutable->runInitializers(gLinkContext, initializerTimes[0]);
    
    // register cxa_atexit() handler to run static terminators in all loaded images when this process exits
    if ( gLibSystemHelpers != NULL ) 
        (*gLibSystemHelpers->cxa_atexit)(&runAllStaticTerminators, NULL, NULL);

    // dump info if requested
    if ( sEnv.DYLD_PRINT_STATISTICS )
        ImageLoader::printStatistics((unsigned int)allImagesCount(), initializerTimes[0]);
    if ( sEnv.DYLD_PRINT_STATISTICS_DETAILS )
        ImageLoaderMachO::printStatisticsDetails((unsigned int)allImagesCount(), initializerTimes[0]);
}
```

&emsp;`gLinkContext` 是一个 `ImageLoader::LinkContext gLinkContext;` 类型的全局变量，LinkContext 是在 class ImageLoader 中定义的一个结构体，其中定义了很多函数指针和成员变量，来记录和处理 Link 的上下文。其中 `bool startedInitializingMainExecutable;` 则是用来记录标记 MainExecutable 开始进行 Initializing 了，这里是直接把它的值置为 true。

&emsp;`InitializerTimingList` 也是在 class ImageLoader 中定义的一个挺简单的结构体。用来记录 Initializer 所花费的时间。    

```c++
struct InitializerTimingList
{
    uintptr_t    count;
    struct {
        const char*        shortName;
        uint64_t        initTime;
    } images[1];

    void addTime(const char* name, uint64_t time);
};

void ImageLoader::InitializerTimingList::addTime(const char* name, uint64_t time)
{
    for (int i=0; i < count; ++i) {
        if ( strcmp(images[i].shortName, name) == 0 ) {
            images[i].initTime += time;
            return;
        }
    }
    images[count].initTime = time;
    images[count].shortName = name;
    ++count;
}
```

&emsp;下面是 `runInitializers` 函数，同样是 class ImageLoader 中定义的一个函数。

```c++
void ImageLoader::runInitializers(const LinkContext& context, InitializerTimingList& timingInfo)
{
    uint64_t t1 = mach_absolute_time(); // ⬅️ 计时开始
    mach_port_t thisThread = mach_thread_self();
    ImageLoader::UninitedUpwards up;
    up.count = 1;
    up.imagesAndPaths[0] = { this, this->getPath() };
    
    // ⬇️⬇️⬇️⬇️⬇️⬇️⬇️⬇️
    processInitializers(context, thisThread, timingInfo, up);
    
    context.notifyBatch(dyld_image_state_initialized, false); // ⬅️ 大概是通知初始化完成  
    mach_port_deallocate(mach_task_self(), thisThread); // ⬅️ deallocate 任务
    uint64_t t2 = mach_absolute_time(); // ⬅️ 计时结束
    fgTotalInitTime += (t2 - t1);
}
```

&emsp;在 runInitializers 中我们看到了两个老面孔，在学习 GCD 源码时见过的 `mach_absolute_time` 和 `mach_thread_self` 一个用来统计初始化时间，一个用来记录当前线程。 

&emsp;`UninitedUpwards` 也是 class ImageLoader 中定义的一个超简单的结构体，其中的成员变量 `std::pair<ImageLoader*, const char*> imagesAndPaths[1];` 一个值记录 ImageLoader 的地址，另一个值记录该 ImageLoader 的路径。 

```c++
struct UninitedUpwards
{
    uintptr_t count;
    std::pair<ImageLoader*, const char*> imagesAndPaths[1];
};
```

&emsp;可看到 `processInitializers(context, thisThread, timingInfo, up);` 是其中最重要的函数，下面来一起看看。

#### processInitializers

&emsp;处理初始化过程，看到 `processInitializers` 中 `recursiveInitialization` 函数的递归调用，之所以递归调用，是因为我们的动态库或者静态库会引入其它类库，而且表是个多表结构，所以需要递归实例化。 

```c++
// <rdar://problem/14412057> upward dylib initializers can be run too soon
// To handle dangling dylibs which are upward linked but not downward, all upward linked dylibs
// have their initialization postponed until after the recursion through downward dylibs
// has completed.
void ImageLoader::processInitializers(const LinkContext& context, mach_port_t thisThread,
                                     InitializerTimingList& timingInfo, ImageLoader::UninitedUpwards& images)
{
    uint32_t maxImageCount = context.imageCount()+2;
    ImageLoader::UninitedUpwards upsBuffer[maxImageCount];
    ImageLoader::UninitedUpwards& ups = upsBuffer[0];
    ups.count = 0;
    
    // Calling recursive init on all images in images list, building a new list of uninitialized upward dependencies.
    for (uintptr_t i=0; i < images.count; ++i) {
        images.imagesAndPaths[i].first->recursiveInitialization(context, thisThread, images.imagesAndPaths[i].second, timingInfo, ups);
    }
    
    // If any upward dependencies remain, init them.
    if ( ups.count > 0 )
        processInitializers(context, thisThread, timingInfo, ups);
}
```

&emsp;`images.imagesAndPaths[i].first` 是 ImageLoader 指针（`ImageLoader *`），即调用 class ImageLoader 的 `recursiveInitialization` 函数，下面我们看一下 `recursiveInitialization` 函数的定义。

#### recursiveInitialization

&emsp;`recursiveInitialization` 函数定义较长，其中比较重要的是 // initialize lower level libraries first 下的 for 循环，进行循环判断是否都加载过，没有的话就再执行 `dependentImage->recursiveInitialization` 因为我们前面说的动态库可能会依赖其它库。 

```c++
void ImageLoader::recursiveInitialization(const LinkContext& context, mach_port_t this_thread, const char* pathToInitialize,
                                          InitializerTimingList& timingInfo, UninitedUpwards& uninitUps)
{
    recursive_lock lock_info(this_thread);
    recursiveSpinLock(lock_info);

    if ( fState < dyld_image_state_dependents_initialized-1 ) {
        uint8_t oldState = fState;
        // break cycles
        fState = dyld_image_state_dependents_initialized-1;
        try {
            // initialize lower level libraries first
            for(unsigned int i=0; i < libraryCount(); ++i) {
                ImageLoader* dependentImage = libImage(i);
                if ( dependentImage != NULL ) {
                    // don't try to initialize stuff "above" me yet
                    if ( libIsUpward(i) ) {
                        uninitUps.imagesAndPaths[uninitUps.count] = { dependentImage, libPath(i) };
                        uninitUps.count++;
                    }
                    else if ( dependentImage->fDepth >= fDepth ) {
                        dependentImage->recursiveInitialization(context, this_thread, libPath(i), timingInfo, uninitUps);
                    }
                }
            }
            
            // record termination order
            if ( this->needsTermination() )
                context.terminationRecorder(this);

            // let objc know we are about to initialize this image
            uint64_t t1 = mach_absolute_time(); // ⬅️ 起点计时
            fState = dyld_image_state_dependents_initialized;
            oldState = fState;
            context.notifySingle(dyld_image_state_dependents_initialized, this, &timingInfo);
            
            // initialize this image
            bool hasInitializers = this->doInitialization(context);

            // let anyone know we finished initializing this image
            fState = dyld_image_state_initialized;
            oldState = fState;
            context.notifySingle(dyld_image_state_initialized, this, NULL);
            
            if ( hasInitializers ) {
                uint64_t t2 = mach_absolute_time(); // ⬅️ 终点计时
                timingInfo.addTime(this->getShortName(), t2-t1);
            }
        }
        catch (const char* msg) {
            // this image is not initialized
            fState = oldState;
            recursiveSpinUnLock();
            throw;
        }
    }
    
    recursiveSpinUnLock();
}
```

&emsp;然后再往下是 // let objc know we are about to initialize this image 部分的的内容，它们才是 `recursiveInitialization` 函数的核心，我们首先看一下 `context.notifySingle(dyld_image_state_dependents_initialized, this, &timingInfo);` 函数的调用，首先这里我们一直往上追溯的话可发现 context 参数即在 `initializeMainExecutable` 函数中传入的 `ImageLoader::LinkContext gLinkContext;` 这个全局变量，然后在 dyld/src/dyld2.cpp 文件中的 `static void setContext(const macho_header* mainExecutableMH, int argc, const char* argv[], const char* envp[], const char* apple[])` 静态全局函数中，`gLinkContext.notifySingle = &notifySingle;` 即 `recursiveInitialization`  函数中调用的 `context.notifySingle` 即 `gLinkContext.notifySingle` 即 dyld/src/dyld2.cpp 中的 `&notifySingle` 函数。

&emsp;然后我们直接在 dyld2.cpp 中搜索 `notifySingle` 函数， 它是一个静态全局函数，由于该函数实现过长，那么我们只看其中的核心：

```c++
(*sNotifyObjCInit)(image->getRealPath(), image->machHeader());
```

&emsp;sNotifyObjCInit 是一个静态全局变量，是一个名字是 `_dyld_objc_notify_init` 的函数指针，`_dyld_objc_notify_init` 是一个返回值为 void 两个参数分别是 const char * 和 const struct mach_header * 的函数指针：

```c++
typedef void (*_dyld_objc_notify_init)(const char* path, const struct mach_header* mh);

static _dyld_objc_notify_init sNotifyObjCInit;
```

&emsp;然后同样在 dyld2.cpp 文件中搜索，可看到在 `registerObjCNotifiers` 函数中，有对 `sNotifyObjCInit` 这个全局变量赋值。

```c++
void registerObjCNotifiers(_dyld_objc_notify_mapped mapped, _dyld_objc_notify_init init, _dyld_objc_notify_unmapped unmapped)
{
    // record functions to call
    sNotifyObjCMapped    = mapped;
    
    // ⬇️⬇️⬇️⬇️⬇️
    sNotifyObjCInit        = init; // ⬅️
    // ⬆️⬆️⬆️⬆️⬆️
    
    sNotifyObjCUnmapped = unmapped;

    // call 'mapped' function with all images mapped so far
    try {
        notifyBatchPartial(dyld_image_state_bound, true, NULL, false, true);
    }
    catch (const char* msg) {
        // ignore request to abort during registration
    }

    // <rdar://problem/32209809> call 'init' function on all images already init'ed (below libSystem)
    for (std::vector<ImageLoader*>::iterator it=sAllImages.begin(); it != sAllImages.end(); it++) {
        ImageLoader* image = *it;
        if ( (image->getState() == dyld_image_state_initialized) && image->notifyObjC() ) {
            dyld3::ScopedTimer timer(DBG_DYLD_TIMING_OBJC_INIT, (uint64_t)image->machHeader(), 0, 0);
            
            // ⬇️⬇️⬇️⬇️⬇️⬇️ 
            (*sNotifyObjCInit)(image->getRealPath(), image->machHeader());
        }
    }
}
```

```c++
typedef void (*_dyld_objc_notify_mapped)(unsigned count, const char* const paths[], const struct mach_header* const mh[]);
typedef void (*_dyld_objc_notify_init)(const char* path, const struct mach_header* mh);
typedef void (*_dyld_objc_notify_unmapped)(const char* path, const struct mach_header* mh);
```

&emsp;我们看到 `registerObjCNotifiers` 函数的 `_dyld_objc_notify_init init` 参数会直接赋值给 `sNotifyObjCInit`，并在下面的 for 循环中进行调用，那么什么时候调用 `registerObjCNotifiers` 函数呢？`_dyld_objc_notify_init init` 的实参又是什么呢？我们全局搜索 `registerObjCNotifiers` 函数。（其实看到这里，看到 registerObjCNotifiers 函数的形参我们可能会有一点印象了，之前看 objc 的源码时的 \_objc_init 函数中涉及到 image 部分。）

&emsp;在 dyld/src/dyldAPIs.cpp 中，`_dyld_objc_notify_register` 函数内部调用了 `registerObjCNotifiers` 函数（属于 namespace dyld）。

```c++
void _dyld_objc_notify_register(_dyld_objc_notify_mapped    mapped,
                                _dyld_objc_notify_init      init,
                                _dyld_objc_notify_unmapped  unmapped)
{
    dyld::registerObjCNotifiers(mapped, init, unmapped);
}
```

&emsp;这下就连上了 `_dyld_objc_notify_register` 函数在 objc4 源码中也有调用过，并且就在 `_objc_init` 函数中。下面我们先看一下 `_dyld_objc_notify_register` 函数的声明。  

```c++
//
// Note: only for use by objc runtime
// Register handlers to be called when objc images are mapped, unmapped, and initialized.
// Dyld will call back the "mapped" function with an array of images that contain an objc-image-info section.
// Those images that are dylibs will have the ref-counts automatically bumped, so objc will no longer need to call dlopen() on them to keep them from being unloaded.  
// During the call to _dyld_objc_notify_register(), dyld will call the "mapped" function with already loaded objc images.  
// During any later dlopen() call, dyld will also call the "mapped" function.  
// Dyld will call the "init" function when dyld would be called initializers in that image.  
// This is when objc calls any +load methods in that image.

void _dyld_objc_notify_register(_dyld_objc_notify_mapped    mapped,
                                _dyld_objc_notify_init      init,
                                _dyld_objc_notify_unmapped  unmapped);
```

&emsp;`_dyld_objc_notify_register` 函数仅供 objc runtime 使用。注册在 mapped、unmapped 和 initialized objc images 时要调用的处理程序。Dyld 将使用包含 objc-image-info section 的 images 数组回调 mapped 函数。那些 dylib 的 images 将自动增加引用计数，因此 objc 将不再需要对它们调用 dlopen() 以防止它们被卸载。在调用 \_dyld_objc_notify_register() 期间，dyld 将使用已加载的 objc images 调用 mapped 函数。在以后的任何 dlopen() 调用中，dyld 还将调用 mapped 函数。当 dyld 在该 image 中调用 initializers 时，Dyld 将调用 init 函数。这是当 objc 调用 image 中的任何 +load 方法时。

&emsp;下面我们在 objc4-781 中搜一下 `_dyld_objc_notify_register` 函数，在 `_objc_init` 中我们看到了它的身影。

```c++
void _objc_init(void)
{
    // initialized 作为一个局部静态变量，只能初始化一次，保证 _objc_init 全局只执行一次 
    static bool initialized = false;
    if (initialized) return;
    initialized = true;
    
    // fixme defer initialization until an objc-using image is found?
    environ_init();
    tls_init();
    static_init();
    runtime_init();
    exception_init();
    cache_init();
    _imp_implementationWithBlock_init();
    
    // ⬇️⬇️⬇️⬇️
    _dyld_objc_notify_register(&map_images, load_images, unmap_image);

#if __OBJC2__
    didCallDyldNotifyRegister = true;
#endif
}
```

&emsp;那么追了这么久，上面的 init 形数对应的实参便是 `load_images` 函数。总结下：`_dyld_objc_notify_register` 相当于一个回调函数，也就是该方法的调用是在 `_objc_init` 调用的方法里调用,类推就是调用初始化方法才回去调用 `_dyld_objc_notify_register`。下面我们回到前面的 `bool hasInitializers = this->doInitialization(context);` 调用。

```c++
bool ImageLoaderMachO::doInitialization(const LinkContext& context)
{
    CRSetCrashLogMessage2(this->getPath());

    // ⬇️⬇️⬇️⬇️
    // mach-o has -init and static initializers
    doImageInit(context);
    
    doModInitFunctions(context);
    
    CRSetCrashLogMessage2(NULL);
    
    return (fHasDashInit || fHasInitializers);
}
```

&emsp;其中的核心是 `doImageInit(context);` 调用。

```c++
void ImageLoaderMachO::doImageInit(const LinkContext& context)
{
    if ( fHasDashInit ) {
        const uint32_t cmd_count = ((macho_header*)fMachOData)->ncmds; // ⬅️ load command 的数量
        const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
        const struct load_command* cmd = cmds;
        for (uint32_t i = 0; i < cmd_count; ++i) {
            switch (cmd->cmd) {
                case LC_ROUTINES_COMMAND:
                    Initializer func = (Initializer)(((struct macho_routines_command*)cmd)->init_address + fSlide);
#if __has_feature(ptrauth_calls)
                    func = (Initializer)__builtin_ptrauth_sign_unauthenticated((void*)func, ptrauth_key_asia, 0);
#endif
                    // <rdar://problem/8543820&9228031> verify initializers are in image
                    if ( ! this->containsAddress(stripPointer((void*)func)) ) {
                        dyld::throwf("initializer function %p not in mapped image for %s\n", func, this->getPath());
                    }
                    if ( ! dyld::gProcessInfo->libSystemInitialized ) {
                        // <rdar://problem/17973316> libSystem initializer must run first
                        dyld::throwf("-init function in image (%s) that does not link with libSystem.dylib\n", this->getPath());
                    }
                    if ( context.verboseInit )
                        dyld::log("dyld: calling -init function %p in %s\n", func, this->getPath());
                    {
                        dyld3::ScopedTimer(DBG_DYLD_TIMING_STATIC_INITIALIZER, (uint64_t)fMachOData, (uint64_t)func, 0);
                        func(context.argc, context.argv, context.envp, context.apple, &context.programVars);
                    }
                    break;
            }
            cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
        }
    }
}
```

&emsp;看到其中就是遍历 load command，找到其中 LC_ROUTINES_COMMAND 类型的 load command 然后通过内存地址偏移找到要执行的方法的地址并执行。（`Initializer func = (Initializer)(((struct macho_routines_command*)cmd)->init_address + fSlide);`） 其中的 `if ( ! dyld::gProcessInfo->libSystemInitialized )` 是判断 libSystem 必须先初始化，否则就直接抛错。总结：上面我们研究了初始化的过程，最后是由内存地址不断平移拿到初始化方法进行调用。

&emsp;这样我们最开始的 bt 指令的截图中出现的函数就都浏览一遍了：`_dyld_start` -> `dyldbootstrap::start` -> `dyld::_main` -> `dyld::initializeMainExecutable` -> `ImageLoader::runInitializers` -> `ImageLoader::processInitializers` -> `ImageLoader::recursiveInitialization` -> `dyld::notifySingle` -> `libobjc.A.dylib \` `load_images`。

&emsp;下面我们开始验证 initializeMainExecutable 的流程！

&emsp;我们在 objc4-781 源码的 `_objc_init` 处打一个断点并运行，可看到如下的堆栈信息。

![截屏2021-06-06 上午11.30.09.png](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/b43633835e814c2aa4b09cb23304494d~tplv-k3u1fbpfcp-watermark.image)

&emsp;我们看到了 `libSystem_initializer` 函数的调用，我们去下载源码：[Libsystem](https://opensource.apple.com/tarballs/Libsystem/)，打开源码，全局搜索 `libSystem_initializer`，可在 Libsystem/init.c 中看到 `libSystem_initializer` 函数的定义如下（只摘录一部分）：

```c++
// libsyscall_initializer() initializes all of libSystem.dylib
// <rdar://problem/4892197>
__attribute__((constructor))
static void
libSystem_initializer(int argc,
              const char* argv[],
              const char* envp[],
              const char* apple[],
              const struct ProgramVars* vars)
{
    static const struct _libkernel_functions libkernel_funcs = {
        .version = 4,
        // V1 functions
#if !TARGET_OS_DRIVERKIT
        .dlsym = dlsym,
#endif
        .malloc = malloc,
        .free = free,
        .realloc = realloc,
        ._pthread_exit_if_canceled = _pthread_exit_if_canceled,
        ...
```

&emsp;下面我们摘录其中比较重要的内容：

```c++
// 对内核的初始化
__libkernel_init(&libkernel_funcs, envp, apple, vars);

// 对平台的初始化
__libplatform_init(NULL, envp, apple, vars);

// 对线程的初始化（初始化后我们的 GCD 才能使用）
__pthread_init(&libpthread_funcs, envp, apple, vars);

// 对 libc 的初始化
_libc_initializer(&libc_funcs, envp, apple, vars);

// 对 malloc 初始化
// TODO: Move __malloc_init before __libc_init after breaking malloc's upward link to Libc
// Note that __malloc_init() will also initialize ASAN when it is present
__malloc_init(apple);

// 对 dyld 进行初始化（dyld_start 是 dyld 并没有初始化，dyld 也是一个库）
_dyld_initializer();

// 对 libdispatch 进行初始化。上面的堆栈信息中我们也看到了 libdispatch 的初始化
libdispatch_init();

_libxpc_initializer();
```

&emsp;我们可看到 `libdispatch_init` 是在 `libdispatch.dylib` 中，我们去下载源码：[libdispatch](https://opensource.apple.com/tarballs/libdispatch/)，打开源码，全局搜索 `libdispatch_init`，可在 libdispatch/Dispatch Source/queue.c 中看到 `libdispatch_init` 函数的定义如下（只摘录一部分）：

```c++
...
frame #1: 0x00000001002e989f libdispatch.dylib`_os_object_init + 13
frame #2: 0x00000001002faa13 libdispatch.dylib`libdispatch_init + 285
...
```

```c++
DISPATCH_EXPORT DISPATCH_NOTHROW
void
libdispatch_init(void)
{
    dispatch_assert(sizeof(struct dispatch_apply_s) <=
            DISPATCH_CONTINUATION_SIZE);

    if (_dispatch_getenv_bool("LIBDISPATCH_STRICT", false)) {
        _dispatch_mode |= DISPATCH_MODE_STRICT;
    }
    ...
...
```

&emsp;我们一直沿着 `libdispatch_init` 的定义往下看，临近定义结束处，会有 `_dispatch_thread` 的一些 creat 操作（GCD 相关）。

```c++
...
_dispatch_hw_config_init();
_dispatch_time_init();
_dispatch_vtable_init();

// ⬇️⬇️⬇️⬇️⬇️⬇️
_os_object_init();

_voucher_init();
_dispatch_introspection_init();
}
```

&emsp;看到其中 `_os_object_init` 的调用，我们全局搜一下可在 libdispatch/Dispatch Source/object.m 中看到其定义。 

```c++
void
_os_object_init(void)
{
    // ⬇️⬇️⬇️
    _objc_init();
    
    Block_callbacks_RR callbacks = {
        sizeof(Block_callbacks_RR),
        (void (*)(const void *))&objc_retain,
        (void (*)(const void *))&objc_release,
        (void (*)(const void *))&_os_objc_destructInstance
    };
    
    _Block_use_RR2(&callbacks);
#if DISPATCH_COCOA_COMPAT
    const char *v = getenv("OBJC_DEBUG_MISSING_POOLS");
    if (v) _os_object_debug_missing_pools = _dispatch_parse_bool(v);
    v = getenv("DISPATCH_DEBUG_MISSING_POOLS");
    if (v) _os_object_debug_missing_pools = _dispatch_parse_bool(v);
    v = getenv("LIBDISPATCH_DEBUG_MISSING_POOLS");
    if (v) _os_object_debug_missing_pools = _dispatch_parse_bool(v);
#endif
}
```

&emsp;我们看到 `_os_object_init` 函数定义第一行就是 `_objc_init` 调用，也就是从 `_os_object_init` 跳入到 `_objc_init` 进入 runtime 的初始化，上面我们讲了 `_objc_init` 会调用 `_dyld_objc_notify_register`，然后就对 `sNotifyObjCInit` 赋值。

```c++
...
// initialize this image
bool hasInitializers = this->doInitialization(context);

// let anyone know we finished initializing this image
fState = dyld_image_state_initialized;
oldState = fState;
context.notifySingle(dyld_image_state_initialized, this, NULL);
...
```

&emsp;我们在 `doInitialization` 方法调用之后再进行 `notifySingle`，而 `notifySingle` 就会跳到 `sNotifyObjCInit`，`sNotifyObjCInit()` 才会执行。

&emsp;文章开头处我们看到 load 方法是最先执行的，在之前的文章中我们有详细分析过 +load 的执行，如果还有印象的话一定记得它的入口 `load_imags` 函数。这正和我们上面的分析联系起来了，在 objc-781 源码中，它最先走的是 `objc_init`，它最后会调用 `_dyld_objc_notify_register` 传入 `load_images`，而 `load_images` 内部的 `prepare_load_methods` 和 `call_load_methods` 完成了整个项目中父类、子类、分类中的所有 +load 函数的调用。

![截屏2021-06-06 下午4.19.52.png](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/ba2304f261cf446bb6434d0c82dceb66~tplv-k3u1fbpfcp-watermark.image)

&emsp;文章开头处我们看到 C++ 静态方法是在 +load 方法后面调用的，我们现在看一下它的调用时机。前面分析 `ImageLoaderMachO` 时只看了 `doImageInit` 而没有分析 `doModInitFunctions` 函数。

```c++
// ⬇️⬇️⬇️⬇️
// mach-o has -init and static initializers
doImageInit(context);
doModInitFunctions(context);
```

&emsp;在 dyld/src/ImageLoaderMachO.cpp 中我们可看到 `void ImageLoaderMachO::doModInitFunctions(const LinkContext& context)` 的定义，由于其过长，这里我们只摘录看下其内容。

```c++
void ImageLoaderMachO::doModInitFunctions(const LinkContext& context)
{
    if ( fHasInitializers ) {
        const uint32_t cmd_count = ((macho_header*)fMachOData)->ncmds;
        const struct load_command* const cmds = (struct load_command*)&fMachOData[sizeof(macho_header)];
        const struct load_command* cmd = cmds;
        for (uint32_t i = 0; i < cmd_count; ++i) {
            if ( cmd->cmd == LC_SEGMENT_COMMAND ) {
                const struct macho_segment_command* seg = (struct macho_segment_command*)cmd;
                const struct macho_section* const sectionsStart = (struct macho_section*)((char*)seg + sizeof(struct macho_segment_command));
                const struct macho_section* const sectionsEnd = &sectionsStart[seg->nsects];
                for (const struct macho_section* sect=sectionsStart; sect < sectionsEnd; ++sect) {
                    const uint8_t type = sect->flags & SECTION_TYPE;
                    ...
...
```

&emsp;首先遍历找到类型是 `LC_SEGMENT_COMMAND` 的 Load command，然后遍历该段中类型是 `S_MOD_INIT_FUNC_POINTERS` 和 `S_INIT_FUNC_OFFSETS` 的区，然后便利其中的 `Initializer` 并执行。

```c++
typedef void (*Initializer)(int argc, const char* argv[], const char* envp[], const char* apple[], const ProgramVars* vars);
```

&emsp;我们在之前写的 `main_front` 函数中打一个断点，运行代码，使用 bt 查看其函数调用堆栈。

![截屏2021-06-06 下午5.07.34.png](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/9c10e651383549459241198765b5fac2~tplv-k3u1fbpfcp-watermark.image)

&emsp;可看到 `main_front` 正是在 `ImageLoaderMachO::doModInitFunctions(ImageLoader::LinkContext const&)` 下执行的，也正说明了 C++ 的静态方法就是在执行 `doModInitFunctions` 下执行的。

&emsp;我们在 `main_front` 中打一个断点，然后打开 Debug -> Debug Workflow -> Always Show Disassembly 选项，运行项目。

```c++
Test_ipa_simple`main_front:
    0x10fc7bf60 <+0>:  pushq  %rbp
    0x10fc7bf61 <+1>:  movq   %rsp, %rbp
    0x10fc7bf64 <+4>:  leaq   0x2105(%rip), %rax        ; @
    0x10fc7bf6b <+11>: movq   %rax, %rdi
    0x10fc7bf6e <+14>: leaq   0x1e39(%rip), %rsi        ; "main_front"
    0x10fc7bf75 <+21>: movb   $0x0, %al
    0x10fc7bf77 <+23>: callq  0x10fc7c334               ; symbol stub for: NSLog
->  0x10fc7bf7c <+28>: popq   %rbp
    0x10fc7bf7d <+29>: retq   
```

&emsp;我们下一步等 `main_front` 执行完，再 Continue Program Execution 直接到 main 函数中：

```c++
Test_ipa_simple`main:
    0x102838100 <+0>:  pushq  %rbp
    0x102838101 <+1>:  movq   %rsp, %rbp
    0x102838104 <+4>:  subq   $0x10, %rsp
    0x102838108 <+8>:  leaq   0x1f79(%rip), %rax        ; @
    0x10283810f <+15>: movl   $0x0, -0x4(%rbp)
    0x102838116 <+22>: movl   %edi, -0x8(%rbp)
    0x102838119 <+25>: movq   %rsi, -0x10(%rbp)
->  0x10283811d <+29>: movq   %rax, %rdi
    0x102838120 <+32>: movb   $0x0, %al
    0x102838122 <+34>: callq  0x1028383d4               ; symbol stub for: NSLog
    0x102838127 <+39>: xorl   %eax, %eax
    0x102838129 <+41>: addq   $0x10, %rsp
    0x10283812d <+45>: popq   %rbp
    0x10283812e <+46>: retq   
```

&emsp;`main` 函数是被编译到内存中的，而且是固定写死的，编译器找到 `main` 函数会加载到内存中，如果我们修改 `main` 函数的名字则会报如下错误: `ld: entry point (_main) undefined. for architecture x86_64`，告诉我们找不到 `main` 函数，这部分其实在 `dyld` 源码中也有所体现，下面我们搜下 `_dyld_start` 看下不同平台下对 `main` 函数的调用。 

```c++
#if __i386__ && !TARGET_OS_SIMULATOR
...
         # LC_MAIN case, set up stack for call to main()
Lnew:    movl    4(%ebp),%ebx
...
#endif /* __i386__  && !TARGET_OS_SIMULATOR*/


#if __x86_64__ && !TARGET_OS_SIMULATOR
...
         # LC_MAIN case, set up stack for call to main()
Lnew:    addq    $16,%rsp    # remove local variables
...
#endif /* __x86_64__ && !TARGET_OS_SIMULATOR*/


#if __arm__
...
         // LC_MAIN case, set up stack for call to main()
Lnew:    mov    lr, r5            // simulate return address into _start in libdyld
...
#endif /* __arm__ */


#if __arm64__ && !TARGET_OS_SIMULATOR
...
         // LC_MAIN case, set up stack for call to main()
Lnew:    mov    lr, x1            // simulate return address into _start in libdyld.dylib
...
#endif // __arm64__ && !TARGET_OS_SIMULATOR
```

&emsp;





















## 参考链接
**参考链接:🔗**
+ [dyld-832.7.3](https://opensource.apple.com/tarballs/dyld/)
+ [OC底层原理之-App启动过程（dyld加载流程）](https://juejin.cn/post/6876773824491159565)
+ [iOS中的dyld缓存是什么？](https://blog.csdn.net/gaoyuqiang30/article/details/52536168)
+ [iOS进阶之底层原理-应用程序加载（dyld加载流程、类与分类的加载）](https://blog.csdn.net/hengsf123456/article/details/116205004?utm_medium=distribute.pc_relevant.none-task-blog-baidujs_title-4&spm=1001.2101.3001.4242)
+ [iOS应用程序在进入main函数前做了什么？](https://www.jianshu.com/p/73d63220d4f1)
+ [dyld加载应用启动原理详解](https://www.jianshu.com/p/1b9ca38b8b9f)
+ [iOS里的动态库和静态库](https://www.jianshu.com/p/42891fb90304)
+ [Xcode 中的链接路径问题](https://www.jianshu.com/p/cd614e080078)
+ [iOS 利用 Framework 进行动态更新](https://nixwang.com/2015/11/09/ios-dynamic-update/)
+ [命名空间namespace ，以及重复定义的问题解析](https://blog.csdn.net/u014357799/article/details/79121340)
+ [C++ 命名空间namespace](https://www.jianshu.com/p/30e960717ef1)
+ [一文了解 Xcode 生成「静态库」和「动态库」 的流程](https://mp.weixin.qq.com/s/WH8emrMpLeVW-LfGwN09cw)
+ [Hook static initializers](https://blog.csdn.net/majiakun1/article/details/99413403)
+ [iOS逆向 dyld流程](https://juejin.cn/post/6844904202242637837)
+ [OC 底层探索 13、类的加载1 - dyld和objc的关联](https://www.cnblogs.com/zhangzhang-y/p/13806192.html)

