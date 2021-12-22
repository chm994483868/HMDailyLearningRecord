# iOS App Crash 学习：(三)：KSCrash 源码分析

&emsp;[kstenerud/KSCrash](https://github.com/kstenerud/KSCrash) The Ultimate Crash Reporter! 

## KSCrash 简述

&emsp;以下内容来自 KSCrash 首页描述。

### Another crash reporter? Why? 

&emsp;虽然现有的 crash reporters 确实报告了崩溃，但是他们可以做的还有很多。

&emsp;以下是 KSCrash 的一些主要功能：

+ 以支持离线重新符号化的方式在设备上进行符号化（对于许多功能已被编辑的 iOS 版本是必需的）。
+ 生成完整的 Apple 报告（KSCrashEmailReportStyleApple、KSCrashEmailReportStyleJSON），并填写每个字段。
+ 支持 32 位和 64 位模式。
+ 支持所有苹果设备，包括 Apple Watch。
+ 能处理只能在 mach level 捕获的错误，例如堆栈溢出（stack overflow 只能在 mach level 捕获到，它不同于其他异常，它不会转化为对应的 Unix Signal（这里还不确定会不会转化为 Unix Signal），即使能转化，但是常规情况下 Unix signals 要在崩溃线程执行回调，此时由于堆栈溢出已经没有条件（堆栈空间）执行回调函数了）。
+ 跟踪未捕获的 C++ 异常的真正原因。
+ 在崩溃处理程序本身（或用户崩溃处理程序回调）中处理崩溃。
+ 检测 zombie（deallocated）object 访问尝试。
+ 在 zombies 或内存损坏的情况下恢复丢失的 NSException 消息。
+ introspects 寄存器和堆栈中的对象（C 字符串和 Objective-C 对象，包括 ivars）。
+ 提取有关异常引用的对象的信息（例如 "发送到实例 0xa26d9a0 无法识别的选择器"）。
+ 其可插拔的服务器报告体系结构使其可以轻松适应任何 API 服务。（邮件、上传服务器、Hockey、Quincy、Victory）
+ dumps 堆栈内容。
+ diagnoses 崩溃原因（Crash Doctor）。
+ 以 JSON 格式记录大量超出 Apple 崩溃报告范围的信息。
+ 支持包含程序员提供的额外数据（before and during a crash）。

### KSCrash 处理以下类型的崩溃

+ Mach kernel exceptions（Mach 内核异常）
+ Fatal signals（Unix Signals）
+ C++ exceptions（C++ 异常）
+ Objective-C exceptions（Objective-C 异常） 
+ Main thread deadlock（experimental）主线程死锁检测（实验性质）
+ Custom crashes（e.g. from scripting languages）自定义异常

### KSCrash can report to the following servers:

+ Hockey
+ QuincyKit
+ Victory
+ Email

### C++ Exception Handling

&emsp;没错！通常，如果你的应用由于未捕获的 C++ exception 而终止，你得到的只是： 

```c++
Thread 0 name:  Dispatch queue: com.apple.main-thread
Thread 0 Crashed:
0   libsystem_kernel.dylib          0x9750ea6a 0x974fa000 + 84586 (__pthread_kill + 10)
1   libsystem_sim_c.dylib           0x04d56578 0x4d0f000 + 292216 (abort + 137)
2   libc++abi.dylib                 0x04ed6f78 0x4ed4000 + 12152 (abort_message + 102)
3   libc++abi.dylib                 0x04ed4a20 0x4ed4000 + 2592 (_ZL17default_terminatev + 29)
4   libobjc.A.dylib                 0x013110d0 0x130b000 + 24784 (_ZL15_objc_terminatev + 109)
5   libc++abi.dylib                 0x04ed4a60 0x4ed4000 + 2656 (_ZL19safe_handler_callerPFvvE + 8)
6   libc++abi.dylib                 0x04ed4ac8 0x4ed4000 + 2760 (_ZSt9terminatev + 18)
7   libc++abi.dylib                 0x04ed5c48 0x4ed4000 + 7240 (__cxa_rethrow + 77)
8   libobjc.A.dylib                 0x01310fb8 0x130b000 + 24504 (objc_exception_rethrow + 42)
9   CoreFoundation                  0x01f2af98 0x1ef9000 + 204696 (CFRunLoopRunSpecific + 360)
...
```

&emsp;无法跟踪异常是什么或从哪里抛出！

&emsp;现在，使用 KSCrash，你可以获得未捕获的异常类型、描述以及它从何处抛出：

```c++
Application Specific Information:
*** Terminating app due to uncaught exception 'MyException', reason: 'Something bad happened...'

Thread 0 name:  Dispatch queue: com.apple.main-thread
Thread 0 Crashed:
0   Crash-Tester                    0x0000ad80 0x1000 + 40320 (-[Crasher throwUncaughtCPPException] + 0)
1   Crash-Tester                    0x0000842e 0x1000 + 29742 (__32-[AppDelegate(UI) crashCommands]_block_invoke343 + 78)
2   Crash-Tester                    0x00009523 0x1000 + 34083 (-[CommandEntry executeWithViewController:] + 67)
3   Crash-Tester                    0x00009c0a 0x1000 + 35850 (-[CommandTVC tableView:didSelectRowAtIndexPath:] + 154)
4   UIKit                           0x0016f285 0xb4000 + 766597 (-[UITableView _selectRowAtIndexPath:animated:scrollPosition:notifyDelegate:] + 1194)
5   UIKit                           0x0016f4ed 0xb4000 + 767213 (-[UITableView _userSelectRowAtPendingSelectionIndexPath:] + 201)
6   Foundation                      0x00b795b3 0xb6e000 + 46515 (__NSFireDelayedPerform + 380)
7   CoreFoundation                  0x01f45376 0x1efa000 + 308086 (__CFRUNLOOP_IS_CALLING_OUT_TO_A_TIMER_CALLBACK_FUNCTION__ + 22)
8   CoreFoundation                  0x01f44e06 0x1efa000 + 306694 (__CFRunLoopDoTimer + 534)
9   CoreFoundation                  0x01f2ca82 0x1efa000 + 207490 (__CFRunLoopRun + 1810)
10  CoreFoundation                  0x01f2bf44 0x1efa000 + 204612 (CFRunLoopRunSpecific + 276)
...
```

### Custom Crashes & Stack Traces

&emsp;You can now report your own custom crashes and stack traces (think scripting languages):

```c++
- (void) reportUserException:(NSString*) name
                  reason:(NSString*) reason
              lineOfCode:(NSString*) lineOfCode
              stackTrace:(NSArray*) stackTrace
        terminateProgram:(BOOL) terminateProgram;
```

&emsp;See KSCrash.h for details.

### Unstable Features

&emsp;The following features should be considered "unstable" and are disabled by default:

+ Deadlock detection

## How to Use KSCrash

1. 将 framework 添加到项目中（或将 KSCrash 项目添加为依赖项）。
2. 将以下系统 frameworks 和 libraries 添加到你的项目中：

+ libc++.dylib (libc++.tbd in newer versions)
+ libz.dylib (libz.tbd in newer versions)
+ MessageUI.framework (iOS only)
+ SystemConfiguration.framework

3. Add the flag "-ObjC" to Other Linker Flags in your Build Settings。
4. Add the following to your `[application: didFinishLaunchingWithOptions:]` method in your app delegate:

```c++
#import <KSCrash/KSCrash.h>

// Include to use the standard reporter.
#import <KSCrash/KSCrashInstallationStandard.h>

// Include to use Quincy or Hockey.
#import <KSCrash/KSCrashInstallationQuincyHockey.h>

// Include to use the email reporter.
#import <KSCrash/KSCrashInstallationEmail.h>

// Include to use Victory.
#import <KSCrash/KSCrashInstallationVictory.h>

- (BOOL)application:(UIApplication*) application didFinishLaunchingWithOptions:(NSDictionary*) launchOptions {
KSCrashInstallationStandard* installation = [KSCrashInstallationStandard sharedInstance];
installation.url = [NSURL URLWithString:@"http://put.your.url.here"];

// OR:

KSCrashInstallationQuincy* installation = [KSCrashInstallationQuincy sharedInstance];
installation.url = [NSURL URLWithString:@"http://put.your.url.here"];

// OR:

KSCrashInstallationHockey* installation = [KSCrashInstallationHockey sharedInstance];
installation.appIdentifier = @"PUT_YOUR_HOCKEY_APP_ID_HERE";

// OR:

KSCrashInstallationEmail* installation = [KSCrashInstallationEmail sharedInstance];
installation.recipients = @[@"some@email.address"];

// Optional (Email): Send Apple-style reports instead of JSON
[installation setReportStyle:KSCrashEmailReportStyleApple useDefaultFilenameFormat:YES]; 

// Optional: Add an alert confirmation (recommended for email installation)（使用 KSCrashInstallationEmail 时，本地有崩溃日志时，弹出提示框是否发送邮件）
[installation addConditionalAlertWithTitle:@"Crash Detected"
                                 message:@"The app crashed last time it was launched. Send a crash report?"
                               yesAnswer:@"Sure!"
                                noAnswer:@"No thanks"];

// OR:

KSCrashInstallationVictory* installation = [KSCrashInstallationVictory sharedInstance];
installation.url = [NSURL URLWithString:@"https://put.your.url.here/api/v1/crash/<application key>"];

[installation install];
    …
}
```

&emsp;这将安装 crash monitor system（该系统拦截崩溃并将报告存储到磁盘）。请注意，你可以并且可能希望为各种 installations 设置其他属性。

&emsp;Once you're ready to send any outstanding crash reports, call the following:

```c++
[installation sendAllReportsWithCompletion:^(NSArray *filteredReports, BOOL completed, NSError *error) {
    // Stuff to do when report sending is complete（报告发送完成后要做的事情）
    ...
}];
```

## Recommended Reading

&emsp;如果可能，你应该阅读以下头文件，以充分了解 KSCrash 具有哪些功能以及如何使用它们：

+ KSCrash.h
+ KSCrashInstallation.h
+ KSCrashInstallation(SPECIFIC TYPE).h
+ Architecture.md

## Understanding the KSCrash Codebase

&emsp;快速代码教程。[CODE_TOUR](https://github.com/kstenerud/KSCrash/blob/master/CODE_TOUR.md)

### A Brief Tour of the KSCrash Code and Architecture（KSCrash 代码和体系结构简介）

&emsp;KSCrash 曾经足够简单，快速阅读源代码就足以理解它是如何工作的，但是现在它已经足够大了，应该有一些指南来帮助读者。本文档向你介绍 KSCrash 的主要代码区域。

#### The Heart of KSCrash

&emsp;KSCrash 的核心在 [KSCrash/Source/KSCrash/Recording/KSCrashC.c](https://github.com/kstenerud/KSCrash/blob/master/Source/KSCrash/Recording/KSCrashC.c) 文件。

&emsp;此文件包含 KSCrash 系统的所有 important access points。

&emsp;KSCrash.c 的功能也被 Objective-c/Swift 包装在 [KSCrash/Source/KSCrash/Recording/KSCrash.m](https://github.com/kstenerud/KSCrash/blob/master/Source/KSCrash/Recording/KSCrash.m) 文件。

&emsp;以下是 KSCrashC.c 的主要部分：

##### Installation

&emsp;`kscrash_install()` 安装并准备 KSCrash 系统以处理崩溃。你可以在安装之前或之后使用此文件中的各种配置函数（`kscrash_setMonitoring()` 等）配置 KSCrash。

##### Configuration

&emsp;所有主要配置设置都是通过 `kscrash_setXYZ()` 设置的。

##### App State

&emsp;Apple 操作环境提供了许多通知，告诉你当前的应用状态。它们被 hooked 到各种 `kscrash_notifyXYZ()` 函数中。

##### Crash Entry Point

&emsp;`onCrash` 函数是在报告崩溃后调用的主要函数。它处理检查应用程序状态，编写 JSON 崩溃报告，然后允许崩溃自然进行。

##### Report Management

&emsp;此文件还包含用于管理崩溃报告的 low level primitive 函数：`kscrash_getReportCount()`、`kscrash_getReportIDs()`、`kscrash_readReport()`、`kscrash_deleteReportWithID()`。

##### Enabling/Disabling KSCrash

&emsp;可以使用 `kscrash_setMonitoring()` 在运行时有效地启用或禁用崩溃报告。

#### Detecting Crashes

&emsp;崩溃是通过其中一个 [KSCrash/Source/KSCrash/Recording/Monitors/](https://github.com/kstenerud/KSCrash/tree/master/Source/KSCrash/Recording/Monitors) 检测到的，该 monitor 在将控制权传递给 `onCrash()` 函数之前以一致的方式设置数据。这些文件有点 tricky，因为其中一些必须跳过一些 hoops 才能解决操作系统差异，系统特性以及简单的错误。

#### Recording Crashes

&emsp;崩溃通过 [KSCrash/Source/KSCrash/Recording/KSCrashReport.c](https://github.com/kstenerud/KSCrash/blob/master/Source/KSCrash/Recording/KSCrashReport.c) 中的 `kscrashreport_writeStandardReport()` 记录到 JSON 文件中。它利用许多 [KSCrash/Source/KSCrash/Recording/Tools/](https://github.com/kstenerud/KSCrash/tree/master/Source/KSCrash/Recording/Tools) 来实现这一点。

#### Report Management

&emsp;报告管理主要在 [KSCrash/Source/KSCrash/Recording/KSCrashReportStore.c](https://github.com/kstenerud/KSCrash/blob/master/Source/KSCrash/Recording/KSCrashReportStore.c) 中完成。

#### Reporting

&emsp;报告是使用可能过于复杂的 [KSCrash/Source/KSCrash/Reporting/Filters/](https://github.com/kstenerud/KSCrash/tree/master/Source/KSCrash/Reporting/Filters) 和 [KSCrash/Source/KSCrash/Reporting/Sinks/](https://github.com/kstenerud/KSCrash/tree/master/Source/KSCrash/Reporting/Sinks) 系统完成的。通常，为了使 KSCrash 适应你的需求，你需要创建自己的 sink。

#### Installations

&emsp;[KSCrash/Source/KSCrash/Installations/](https://github.com/kstenerud/KSCrash/tree/master/Source/KSCrash/Installations) system 试图通过将大多数 filter/sink 隐藏在更简单的接口后面来使用户 API 更容易一些。它的成功是值得商榷的...

&emsp;没有代码取决于 installation code，KSCrash 实际上可以在没有它的情况下正常工作。

## Advanced Usage

### Enabling on-device symbolication（支持在设备上进行离线符号化的工作）

&emsp;大多数平台的日志解析都需要我们上传对应的符号表文件，用于日志的符号化。其实可以暂时使用这种方式直接得到解析过后的日志。开启 on-device symbolication 需要在构建版本中包含基本符号表（basic symbols），要在 **build settings** 中设置 **Strip Style** 为 **Debugging Symbols**，这样做会将最终二进制文件大小增加约 **5%** （测试这个数字大概不止 5%）。（毕竟包含了符号表，所以必会导致二进制文件大小增加。）

> &emsp;但是得到的行号还是可能有误的，如果需要具体的行号，还是需要 dsym 的解析。[使用KSCrash进行崩溃日志的采集](https://www.jianshu.com/p/7847b7aaef0b)

### Enabling advanced functionality:（启用高级功能：）

&emsp;KSCrash 具有高级功能，examining crash reports in the wild 时非常有用。有些涉及次要的权衡，因此默认情况下，它们中的大多数都是禁用的。

#### Custom User Data (userInfo in KSCrash.h)

&emsp;你可以通过在 KSCrash.h 中设置 userInfo 属性，将自定义用户数据存储到下一个崩溃报告中。

#### Zombie Tracking (KSCrashMonitorTypeZombie in KSCrashMonitorType.h)

&emsp;KSCrash 具有检测 zombie 对象实例（悬垂指向已解除分配对象的指针
：野指针、悬垂指针）的能力。它通过记录任何被释放的对象的地址和类来实现此目的。它将这些值存储在缓存中，并键控解除分配对象的地址。这意味着你设置的缓存大小越小，发生哈希冲突并丢失有关以前解除分配的对象的信息的可能性就越大。

&emsp;启用 zombie tracking 后，KSCrash 还将检测到丢失的 NSException 并打印其内容。某些类型的内存损坏或堆栈损坏崩溃可能会导致 exception 提前解除分配，从而进一步阻碍调试应用的工作，因此此功能有时非常方便。

&emsp;Trade off: Zombie tracking 的代价是为对象解除分配增加了非常轻微的开销，并保留了一些内存。

#### Deadlock Detection (KSCrashMonitorTypeMainThreadDeadlock in KSCrashMonitorType.h)

&emsp;**WARNING WARNING WARNING WARNING WARNING WARNING WARNING**

&emsp;**此功能不稳定！它可能会误报并使你的应用程序崩溃！**

&emsp;如果主线程死锁，则用户界面将无响应，用户必须手动关闭应用（对此不会有崩溃报告）。启用死锁检测后，将设置 watchdog 计时器。如果有任何内容占用主线程的时间超过 watchdog 计时器持续时间，KSCrash 将关闭应用程序，并为你提供一个堆栈跟踪（stack trace），显示主线程当时正在执行的操作。

&emsp;这很棒，但你必须小心：应用程序初始化通常发生在主线程上。如果你的初始化代码花费的时间比 watchdog 计时器长，你的应用将在启动期间被强制关闭！如果启用此功能，则必须确保正常运行的代码中，没有一个 hold 主线程的时间超过 watchdog 值！同时，你需要将计时器设置为足够低的值，以便用户不会变得不耐烦并在 watchdog 触发之前手动关闭应用程序！

&emsp;Trade off: Deadlock detection，但你必须更加小心在主线程上运行的内容！

#### Memory Introspection (introspectMemory in KSCrash.h)

&emsp;当应用崩溃时，内存中通常会有堆栈、寄存器甚至异常消息引用的对象和字符串。启用后，KSCrash 将自省这些内存区域，并将其内容存储在崩溃报告中。

&emsp;你还可以通过在 KSCrash 中设置 doNotIntrospectClasses 属性来指定不应自省的类的列表。

#### Custom crash handling code (onCrash in KSCrash.h)

&emsp;如果要在发生崩溃后执行一些额外的处理（可能是向崩溃报告中添加更多上下文数据），则可以执行此操作。

&emsp;但是，你必须确保仅使用异步安全代码，最重要的是永远不要从该方法调用 Objective-C 代码！在许多情况下，你无论如何都可以逃脱惩罚，但是在某些类型的崩溃中，忽略此警告的处理程序代码将导致崩溃处理程序崩溃！请注意，如果发生这种情况，KSCrash 将检测到它并编写完整的崩溃报告，尽管你的自定义处理程序代码可能无法完全运行。

&emsp;Trade off: 自定义崩溃处理代码，但你必须小心放入其中的内容！

#### KSCrash log redirection

&emsp;这将获取 KSCrash 将打印到控制台的任何内容，并将其写入文件。主要用它来调试 KSCrash 本身，但它可能对其他目的有用，所以为它公开了一个 API。

&emsp;上面便是 KSCrash 首页的所有内容，下面我们便深入源码来学习 KSCrash。

## KSCrash 使用过程

&emsp;下面我们跟着官方代码 [kstenerud/KSCrash](https://github.com/kstenerud/KSCrash) 来学习 KSCrash。

&emsp;在 `application:didFinishLaunchingWithOptions:` 函数中安装 **崩溃处理程序**。

```c++
- (BOOL) application:(__unused UIApplication *) application didFinishLaunchingWithOptions:(__unused NSDictionary *) launchOptions {

    // App 启动后开始安装 崩溃处理程序
    [self installCrashHandler];
    
    return YES;
}
```

&emsp;KSCrash 支持可插拔的崩溃日志后台报告架构。支持 Email、Hockey、QuincyKit、Victory 四种日志发送方式，此外还有一个 Standard 方式，就是我们提供一个发送接口，直接把崩溃报告发送到我们的服务器。

&emsp;KSCrash 框架根据不同的发送日志的方式提供几种不同的 installation，它们都是 KSCrashInstallation 的子类，KSCrashInstallation 作为一个抽象基类使用，KSCrash 框架提供了如下安装器:

+ KSCrashInstallationStandard
+ KSCrashInstallationEmail
+ KSCrashInstallationHockey
+ KSCrashInstallationQuincy
+ KSCrashInstallationVictory

&emsp;我们只能选择其中一个 installation 使用。这里我们仅以标准 installation（KSCrashInstallationStandard）作为学习的主线。  

&emsp;`installCrashHandler` 函数第一行便是调用 `makeStandardInstallation` 函数，取得 KSCrashInstallationStandard 类的单例对象 `installation`，并为它的 url 属性赋值，此 url 会用在 App 启动时如果本地有崩溃报告的话，会上传到此 url。

&emsp;接下来 KSCrashInstallationStandard 类的单例对象 `installation` 调用其 `install` 函数，此函数继承自父类 KSCrashInstallation，KSCrashInstallationStandard 作为子类并没有重写 `install` 函数，此函数的作用是安装 **崩溃处理程序**，即取得 KSCrash 类的单例对象并对其基础属性进行配置。

&emsp;KSCrash 类的单例对象便是 KSCrash 框架处理异常的的核心，KSCrash 类的单例对象初始化时:

+ 设置了默认的本地存储崩溃信息的路径（/Library/Caches/KSCrash/Simple-Example 首先获取 App 沙盒 Caches 路径，然后拼接 KSCrash 和 App 的 BundleName）
+ 设置 deleteBehaviorAfterSendAll 属性为 KSCDeleteAlways 表示发送崩溃报告成功后删除本地的崩溃记录
+ 设置 introspectMemory 属性为 YES 表示崩溃发生时 introspect memory（堆栈指针附近的任何 Objective-C 对象或 C 字符串，或者 cpu 寄存器或异常引用的任何 Objective-C 对象或 C 字符串，连同其内容都将记录在崩溃报告中）
+ catchZombies 属性设置为 NO 表示不追踪对 Objective/Swift 僵尸对象的访问
+ maxReportCount 属性设置为 5 表示删除旧报告之前磁盘上允许的最大报告数为 5
+ searchQueueNames 属性设置为 NO 表示不会尝试获取每个正在运行的线程的调度队列名称
+ monitoring 属性设置为 KSCrashMonitorTypeProductionSafeMinimal 表示监听所有在生产环境下可以进行安全监听的异常类型（即排除 KSCrashMonitorTypeZombie 和 KSCrashMonitorTypeMainThreadDeadlock 之外的所有异常类型）。

&emsp;然后 KSCrashInstallation 类的 `install` 函数，使用 @synchronized 锁以线程安全的方式，设置了 KSCrash 类的单例对象的 onCrash 属性为默认值（`static void crashCallback(const KSCrashReportWriter* writer) {...}`），然后调用 KSCrash 类的单例对象的 `install` 函数，该函数是整个 KSCrash 框架的核心，我们放在后面再看，现在 KSCrash 的使用代码我们看完了，我们先看一下 KSCrash 捕获异常的结果。

```c++
- (BOOL) application:(__unused UIApplication *) application didFinishLaunchingWithOptions:(__unused NSDictionary *) launchOptions {
    [self installCrashHandler];
    
    return YES;
}
```

```c++
- (void) installCrashHandler {
    // Create an installation (choose one)
    // 这里以 KSCrashInstallationStandard 为例
    KSCrashInstallation* installation = [self makeStandardInstallation];
    
//    KSCrashInstallation* installation = [self makeEmailInstallation];
//    KSCrashInstallation* installation = [self makeHockeyInstallation];
//    KSCrashInstallation* installation = [self makeQuincyInstallation];
//    KSCrashInstallation *installation = [self makeVictoryInstallation];
    
    // Install the crash handler. This should be done as early as possible.
    // This will record any crashes that occur, but it doesn't automatically send them.
    // 安装崩溃处理程序，这应该尽早完成，这将记录发生的任何崩溃，但不会自动发送它们（仅把崩溃报告记录在本地）。
    [installation install];
    
    // 设置本地记录的崩溃报告发出以后，怎么处理本地的崩溃报告
    [KSCrash sharedInstance].deleteBehaviorAfterSendAll = KSCDeleteNever; // TODO: Remove this

    // Send all outstanding reports. You can do this any time; it doesn't need to happen right as the app launches.
    // Advanced-Example shows how to defer displaying the main view controller until crash reporting completes.
    // 发送所有未完成的报告（内部使用 KSCrash 类的单例对象发送），你可以随时执行此操作;它不需要在应用程序启动时立即进行，Advanced-Example 演示了如何推迟显示主视图控制器，直到崩溃报告完成。
    [installation sendAllReportsWithCompletion:^(NSArray* reports, BOOL completed, NSError* error) {
        if (completed) {
            NSLog(@"🐹🐹🐹 Sent %d reports", (int)[reports count]);
        } else {
            NSLog(@"🐹🐹🐹 Failed to send reports: %@", error);
        }
    }];
}
```

```c++
- (KSCrashInstallation*) makeStandardInstallation {
    NSURL* url = [NSURL URLWithString:@"http://put.your.url.here"];
    
    KSCrashInstallationStandard* standard = [KSCrashInstallationStandard sharedInstance];
    standard.url = url;

    return standard;
}
```

## KSCrash 崩溃报告写入和发送 

&emsp;上面一个小节我们浅层次的学习了 KSCrash 的安装以及初始化的代码，这个小节我们看一下 KSCrash 对崩溃报告的记录和上传。

&emsp;首先我们在 Edit Scheme... 中关闭 Debug executable 的选项，Debug 模式下 KSCrash 不进行崩溃报告收集。我们在模拟器下调试 KSCrash，方便看本地的沙盒中写入的文件内容。初次启动时会在 /Library/Caches/KSCrash/Simple-Example/Data/ 路径下创建有两份日志文件：

+ ConsoleLog.txt 用于记录控制台的打印（初始时为空文件）。
+ CrashState.json 记录一些崩溃信息，有这些字段：version 版本、crashedLastLaunch 布尔值表示上次启动是否崩溃、activeDurationSinceLastCrash 上次运行崩溃从启动到崩溃运行了多久、backgroundDurationSinceLastCrash 上次运行崩溃从启动到崩溃在后台运行了多久、launchesSinceLastCrash 自上次崩溃启动过多少次、sessionsSinceLastCrash。

&emsp;运行如下三份典型的 crash 代码（Unix signal、Mach 异常、Objective-C 异常），App 闪退，然后在 /Library/Caches/KSCrash/Simple-Example/Reports/Simple-Example-report-0074db9096800000.json 记录三份 json 格式的崩溃文件。

```c++
- (IBAction) onCrash:(__unused id) sender {
    char* ptr = (char*)-1;
    *ptr = 10;
}
```

```c++
- (IBAction) onCrash:(__unused id) sender {
    NSArray *array = @[@(1), @(2), @(3)];
    NSLog(@"%@", array[3]);
}
```

```c++
- (IBAction) onCrash:(__unused id) sender {
    [self onCrash:sender];
}
```

&emsp;然后 App 再次启动时，会把上面写入本地的崩溃报告 json 文件上传到我们指定的后台接口中。

&emsp;Simple-Example-report-0074db9096800000.json 文件记录了详细的崩溃信息，由于内容过长这里就不展示了。

## KSCrash 源码解读

&emsp;上面我们看完了 KSCrash 的安装和使用，那么下面我们开始阅读 KSCrash 的源码。

&emsp;展开 KSCrash-iOS 的文件夹，我们能清晰的看到有四个主题：Crash Recording、Crash Reporting、Installation、KSCrash，分别对应记录、上报、安装、初始化。

### KSCrash 类分析 

&emsp;下面我们看一下 KSCrash 类的定义。

&emsp;KSCrash 类重写了 +load 和 +initialize 函数，

#### KSCrash +load 函数

&emsp;KSCrash 类的 +load 函数用来指示 KSCrash 已加载，然后对 `static KSCrash_AppState g_state;` 这个表示 App 的 KSCrash 状态的静态全局变量进行设置（App 的 KSCrash 状态：可以理解为 App 的状态或者是当发生 Crash 时 App 的状态）。 

```c++
void kscrashstate_notifyObjCLoad(void)
{
    KSLOG_TRACE("KSCrash has been loaded!");
    
    // g_state 的内存空间置为 0 
    memset(&g_state, 0, sizeof(g_state));
    
    g_state.applicationIsInForeground = false;
    g_state.applicationIsActive = true;
    
    // int gettimeofday(struct timeval * tv, struct timezone * tz);
    // gettimeofday 是计算机函数，使用 C 语言编写程序需要获得当前精确时间（1970 年 1 月 1 日到现在的时间），或者为执行计时，可以使用 gettimeofday 函数
    // 其参数 tv 是保存获取时间结果的结构体（此结构体有两个成员变量：秒和微秒），参数 tz 用于保存时区结果。它获得的时间精确到微秒（1e-6 s)量级，在一段代码前后分别使用 gettimeofday 可以计算代码执行时间。
    
    g_state.appStateTransitionTime = getCurentTime();
}
```

&emsp;这里我们看下 g_state 这个静态全局变量的类型：KSCrash_AppState 结构体，它的成员变量正对应 /Library/Caches/KSCrash/Simple-Example/Data/CrashState.json 中的内容。 

```c++
typedef struct
{
    // Saved data
    
    /** Total active time elapsed since the last crash. */
    double activeDurationSinceLastCrash;
    
    /** Total time backgrounded elapsed since the last crash. */
    double backgroundDurationSinceLastCrash;
    
    /** Number of app launches since the last crash. */
    int launchesSinceLastCrash;
    
    /** Number of sessions (launch, resume from suspend) since last crash. */
    int sessionsSinceLastCrash;
    
    /** Total active time elapsed since launch. */
    double activeDurationSinceLaunch;
    
    /** Total time backgrounded elapsed since launch. */
    double backgroundDurationSinceLaunch;
    
    /** Number of sessions (launch, resume from suspend) since app launch. */
    int sessionsSinceLaunch;
    
    /** If true, the application crashed on the previous launch. */
    bool crashedLastLaunch;
    
    // Live data
    
    /** If true, the application crashed on this launch.  */
    bool crashedThisLaunch;
    
    /** Timestamp for when the app state was last changed (active<->inactive, background<->foreground) 上次更改应用状态的时间戳（active<->inactive, background<->foreground）*/
    double appStateTransitionTime;
    
    /** If true, the application is currently active. */
    bool applicationIsActive;
    
    /** If true, the application is currently in the foreground. */
    bool applicationIsInForeground;
    
} KSCrash_AppState;
```

&emsp;KSCrash_AppState 结构体中 Saved data 部分的成员变量的值会在 App crash 时写入本地。

#### KSCrash +initialize 函数

&emsp;KSCrash 类的 +initialize 函数主要内容是订阅 App 的如下通知：

+ UIApplicationDidBecomeActiveNotification
+ UIApplicationWillResignActiveNotification
+ UIApplicationDidEnterBackgroundNotification
+ UIApplicationWillEnterForegroundNotification
+ UIApplicationWillTerminateNotification

&emsp;在通知的回调函数中在 `static KSApplicationState g_lastApplicationState = KSApplicationStateNone;` 这个全局静态变量中记录程序的状态，以及更新 `static KSCrash_AppState g_state;` 这个静态全局变量的成员变量的值。在 App 进入后台和终止时会把 g_state 的信息写入本地（/Library/Caches/KSCrash/Simple-Example/Data/CrashState.json 中）。

#### KSCrash install 函数

&emsp;下面进入最最重要的 KSCrash 类的 install 函数。

```c++
- (BOOL)install {
    _monitoring = kscrash_install(self.bundleName.UTF8String,
                                          self.basePath.UTF8String);
    if(self.monitoring == 0) {
        return false;
    }

    return true;
}
```

&emsp;install 函数内部调用了 kscrash_install 函数，kscrash_install 函数返回一个 KSCrashMonitorType 枚举值，并把此值赋给了 KSCrash 类的 `@property(nonatomic,readwrite,assign) KSCrashMonitorType monitoring;` 属性，表示当前 KSCrash 要监视 App 的哪些内容（或者说监视哪些异常行为），在上一个小节我们还记得 monitoring 属性的默认值是：KSCrashMonitorTypeProductionSafeMinimal（除 KSCrashMonitorTypeZombie 和 KSCrashMonitorTypeMainThreadDeadlock 之外的 KSCrashMonitorType 枚举的所有值）。

&emsp;下面我们看一下 KSCrashMonitorType 枚举值都有哪些，可看到它基本是和异常类型对应的：

```c++
/** Various aspects of the system that can be monitored:
 * - Mach kernel exception
 * - Fatal signal
 * - Uncaught C++ exception
 * - Uncaught Objective-C NSException
 * - Deadlock on the main thread
 * - User reported custom exception
 */
typedef enum
{
    /* Captures and reports Mach exceptions. */
    KSCrashMonitorTypeMachException      = 0x01,
    
    /* Captures and reports POSIX signals. */
    KSCrashMonitorTypeSignal             = 0x02,
    
    /* Captures and reports C++ exceptions.
     * Note: This will slightly slow down exception processing.
     */
    KSCrashMonitorTypeCPPException       = 0x04,
    
    /* Captures and reports NSExceptions. */
    KSCrashMonitorTypeNSException        = 0x08,
    
    /* Detects and reports a deadlock in the main thread. */
    KSCrashMonitorTypeMainThreadDeadlock = 0x10,
    
    /* Accepts and reports user-generated exceptions. */
    KSCrashMonitorTypeUserReported       = 0x20,
    
    /* Keeps track of and injects system information. */
    KSCrashMonitorTypeSystem             = 0x40,
    
    /* Keeps track of and injects application state. */
    KSCrashMonitorTypeApplicationState   = 0x80,
    
    /* Keeps track of zombies, and injects the last zombie NSException. */
    KSCrashMonitorTypeZombie             = 0x100,
} KSCrashMonitorType;
```

&emsp;KSCrashMonitorType 枚举值用来表示 KSCrash 框架监视的类型，它的每个值还是比较好理解的。前面几个值表示捕获并报告：Mach 异常、Unix Signals、C++ 异常、Objective-C 异常、用户自定义异常、监听系统信息、监听程序状态、监听僵尸对象访问。

#### KSCrash kscrash_install 函数

&emsp;`static volatile bool g_installed = 0;` 静态全局变量用来指示 KSCrash 是否执行过安装，同时标记 kscrash_install 函数在 App 每次运行只能执行一次，当再次调用后会直接 return g_monitoring。

```c++
KSCrashMonitorType kscrash_install(const char* appName, const char* const installPath) {
    KSLOG_DEBUG("Installing crash reporter.");

    // 如果已经执行过安装，则直接 return 当前的监听类型
    if(g_installed)
    {
        KSLOG_DEBUG("Crash reporter already installed.");
        return g_monitoring;
    }
    g_installed = 1;

    // #define KSFU_MAX_PATH_LENGTH 500
    // KSFU 前缀是 KSFileUtils.h 文件名的缩写，KSFileUtils.h 为 KSCrash 提供 基本文件 读/写 功能。 
    
    char path[KSFU_MAX_PATH_LENGTH];
    
    // 把 App 沙盒路径 /Library/Caches/KSCrash/Simple-Example/Reports 字符串复制到 path 变量中  
    snprintf(path, sizeof(path), "%s/Reports", installPath);
    
    // 创建本地路径 /Library/Caches/KSCrash/Simple-Example/Reports
    ksfu_makePath(path);
    
    // 使用 pthread_mutex_t 互斥锁进行初始化，
    // 把 App 名字记录在 g_appName 变量中，
    // 把 Reports 路径记录在 g_reportsPath 中，
    // 如果本地崩溃报告数据大于 g_maxReportCount 则把之前的旧的删除，
    // 对 g_nextUniqueIDHigh 和 g_nextUniqueIDLow 赋值，它们表示最大和最小崩溃报告 ID 值
    kscrs_initialize(appName, path);

    // 创建本地路径 /Library/Caches/KSCrash/Simple-Example/Data
    snprintf(path, sizeof(path), "%s/Data", installPath);
    ksfu_makePath(path);
    
    // 把 CrashState.json 路径记录在 g_stateFilePath 中，
    // 初始化 CrashState.json 文件
    snprintf(path, sizeof(path), "%s/Data/CrashState.json", installPath);
    kscrashstate_initialize(path);

    // ConsoleLog.txt 文件用来记录控制台输出 
    snprintf(g_consoleLogPath, sizeof(g_consoleLogPath), "%s/Data/ConsoleLog.txt", installPath);
    if(g_shouldPrintPreviousLog)
    {
        printPreviousLog(g_consoleLogPath);
    }
    kslog_setLogFilename(g_consoleLogPath, true);
    
    // ksccd 是 KSCrashCachedData.h 的首字母缩写，
    // 好像是更新 task 的线程列表，没看懂，在一个子线程做异步操作，能看到 com.apple.uikit.eventfetch-thread 线程
    ksccd_init(60);

    // kscm 是 KSCrashMonitor.c 的首字母缩写，
    // 把 onCrash 这个静态全局函数作为回调传递给崩溃处理程序，当发生崩溃时会调用它，onCrash 函数，
    // 默认进行 /Library/Caches/KSCrash/Simple-Example/Data/CrashState.json 文件记录，如果记录崩溃记录的崩溃则调用 kscrashreport_writeRecrashReport 记录崩溃问题，
    // 如果是正常崩溃则调用 kscrashreport_writeStandardReport 在本地 /Library/Caches/KSCrash/Simple-Example/Reports/Simple-Example-report-0074dbe70c800000.json 这样路径写崩溃日志
    kscm_setEventCallback(onCrash);
    
    // 设置监听类型并开启监听，最最核心的函数
    KSCrashMonitorType monitors = kscrash_setMonitoring(g_monitoring);

    // log 安装完成
    KSLOG_DEBUG("Installation complete.");

    // 根据当前 App 的状态，更新 CrashState.json 文件中的内容
    notifyOfBeforeInstallationState();

    return monitors;
}
```

&emsp;总结一下 kscrash_install 函数的整个过程：

1. 使用 g_installed 静态全局变量，保证 kscrash_install 函数内部的内容在 App 运行周期内只执行一次。
2. 创建本地路径 /Library/Caches/KSCrash/Simple-Example/Reports，用于在里面记录每个崩溃报告。
3. 记录 App 名字记录在 g_appName 静态全局变量中，Reports 路径记录在 g_reportsPath 中，如果本地崩溃报告数据大于 g_maxReportCount 则把之前的旧的删除，对 g_nextUniqueIDHigh 和 g_nextUniqueIDLow 赋值，它们表示最大和最小崩溃报告 ID 值。
4. 创建本地路径 /Library/Caches/KSCrash/Simple-Example/Data，会用于在其中存储 ConsoleLog.txt 和 CrashState.json 文件。
5. 把 CrashState.json 路径记录在 g_stateFilePath 中，初始化 CrashState.json 文件。
6. 初始化 ConsoleLog.txt 文件用来记录控制台输出。
7. ksccd_init(60) 好像是更新 task 的线程列表，没看懂，在一个子线程做异步操作，能看到 com.apple.uikit.eventfetch-thread 线程名。
8. 把 onCrash 这个静态全局函数作为回调传递给崩溃处理程序，当发生崩溃时会调用它。
9. 设置监听类型并开启监听，最最核心的函数。
10. 根据当前 App 的状态，更新 CrashState.json 文件中的内容。
11. 返回 monitors 监视类型。

#### KSCrash kscrash_setMonitoring 函数

&emsp;kscrash_setMonitoring 函数设置 KSCrash 框架监视内容，同时激活对应的监视类型。

```c++
KSCrashMonitorType kscrash_setMonitoring(KSCrashMonitorType monitors) {
    g_monitoring = monitors;
    
    // 这里确保调用 install 时，才进行激活监视类型，
    // 在 KSCrash 单例类实例初始化时给 monitoring 属性设置默认值时调用了 kscrash_setMonitoring 函数，只是把默认值记录在 g_monitoring 这个静态全局变量中，并不进行激活监视类型 
    if(g_installed) {
        // 当我们调用 KSCrash 的 install 函数时，才真正激活监视类型
        kscm_setActiveMonitors(monitors);
        return kscm_getActiveMonitors();
    }
    
    // Return what we will be monitoring in future.
    // 返回我们将来将要监视的内容。
    return g_monitoring;
}
```

&emsp;

```c++

```























+ [kstenerud/KSCrash](https://github.com/kstenerud/KSCrash)
+ [KSCrash源码分析](https://cloud.tencent.com/developer/article/1370201)
+ [iOS中Crash采集及PLCrashReporter使用](https://www.jianshu.com/p/930d7f77df6c)
+ [iOS KSCrash的使用](https://www.jianshu.com/p/d9ec5f3f144e)
+ [使用KSCrash进行崩溃日志的采集](https://www.jianshu.com/p/7847b7aaef0b)
+ [KSCrash翻译](https://www.jianshu.com/p/95102419c29b)
+ [iOS崩溃日志使用KSCrash收集、分析](https://www.jianshu.com/p/329684cf1e51)
+ [iOS开源库分析之KSCrash](https://xiaozhuanlan.com/topic/7193860452)
+ [KSCrash源码阅读（Monitors）](https://blog.csdn.net/qq_22389025/article/details/84784796)
+ [KSCrash崩溃收集原理浅析](https://www.it610.com/article/1191455498289913856.htm)
+ [KSCrash源码学习](https://www.jianshu.com/p/8c2dc3ce8545)
+ [KSCrash+Symbolicatecrash日志分析](https://www.jianshu.com/p/d88b39acea7d)
+ [了解和分析iOS Crash](https://wetest.qq.com/lab/view/404.html?from=content_zhihu)
+ [wakeup in XNU](https://djs66256.github.io/2021/04/03/2021-04-03-wakeup-in-XNU/)
















BSBacktraceLogger 源码
KSCrash 源码
PLCrashReporter 源码
GYBootingProtection 源码

+ [iOS Crash/崩溃/异常 堆栈获取](https://www.jianshu.com/p/8ece78d71b3d)
+ [iOS中Crash采集及PLCrashReporter使用](https://www.jianshu.com/p/930d7f77df6c)
+ [iOS 启动连续闪退保护方案](https://blog.csdn.net/jiang314/article/details/52574307?utm_medium=distribute.pc_relevant.none-task-blog-2~default~baidujs_title~default-0.highlightwordscore&spm=1001.2101.3001.4242.1)
+ [iOS/OSX Crash：捕捉异常](https://zhuanlan.zhihu.com/p/271282052)





## 参考链接
**参考链接:🔗**
+ [gettimeofday](https://baike.baidu.com/item/gettimeofday/3369586?fr=aladdin)
+ [iOS App 连续闪退时如何上报 crash 日志](https://zhuanlan.zhihu.com/p/35436876)













+ [Mach-维基百科](https://zh.wikipedia.org/wiki/Mach)
+ [iOS 异常信号思考](https://minosjy.com/2021/04/10/00/377/)
+ [Linux 多线程环境下 进程线程终止函数小结](https://www.cnblogs.com/biyeymyhjob/archive/2012/10/11/2720377.html)
+ [pthread_kill引发的争论](https://www.jianshu.com/p/756240e837dd)
+ [线程的信号pthread_kill()函数（线程四）](https://blog.csdn.net/littesss/article/details/71156793)
+ [原子操作atomic_fetch_add](https://www.jianshu.com/p/985fb2e9c201)
+ [iOS Crash 分析攻略](https://zhuanlan.zhihu.com/p/159301707)
+ [Handling unhandled exceptions and signals](https://www.cocoawithlove.com/2010/05/handling-unhandled-exceptions-and.html)
+ [Apple 源码文件下载列表](https://opensource.apple.com/tarballs/)
+ [iOS @try @catch异常机制](https://www.jianshu.com/p/f28b9b3f8e44)
+ [一文读懂崩溃原理](https://juejin.cn/post/6873868181635760142)
+ [软件测试之SDK开发(ios)——Mach捕获](https://blog.csdn.net/lfdanding/article/details/100024022)
+ [[史上最全] iOS Crash/崩溃/异常 捕获](https://www.jianshu.com/p/3f6775c02257)
+ [iOS Crash/崩溃/异常 堆栈获取](https://www.jianshu.com/p/8ece78d71b3d)
+ [KSCrash源码分析](https://cloud.tencent.com/developer/article/1370201)
+ [iOS线程通信和进程通信的例子（NSMachPort和NSTask，NSPipe）](https://blog.csdn.net/yxh265/article/details/51483822)
+ [iOS开发·RunLoop源码与用法完全解析(输入源，定时源，观察者，线程间通信，端口间通信，NSPort，NSMessagePort，NSMachPort，NSPortMessage)](https://sg.jianshu.io/p/07313bc6fd24)
+ [Delivering Notifications To Particular Threads](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/Notifications/Articles/Threading.html#//apple_ref/doc/uid/20001289-CEGJFDFG)
+ [iOS开发之线程间的MachPort通信与子线程中的Notification转发](https://cloud.tencent.com/developer/article/1018076)
+ [移动端监控体系之技术原理剖析](https://www.jianshu.com/p/8123fc17fe0e)
+ [iOS性能优化实践：头条抖音如何实现OOM崩溃率下降50%+](https://mp.weixin.qq.com/s?__biz=MzI1MzYzMjE0MQ==&mid=2247486858&idx=1&sn=ec5964b0248b3526836712b26ef1b077&chksm=e9d0c668dea74f7e1e16cd5d65d1436c28c18e80e32bbf9703771bd4e0563f64723294ba1324&cur_album_id=1590407423234719749&scene=189#wechat_redirect)
+ [iOS Crash之NSInvalidArgumentException](https://blog.csdn.net/skylin19840101/article/details/51941540)
+ [iOS调用reloadRowsAtIndexPaths Crash报异常NSInternalInconsistencyException](https://blog.csdn.net/sinat_27310637/article/details/62225658)
+ [iOS开发质量的那些事](https://zhuanlan.zhihu.com/p/21773994)
+ [NSException抛出异常&NSError简单介绍](https://www.jianshu.com/p/23913bbc4ee5)
+ [NSException:错误处理机制---调试中以及上架后的产品如何收集错误日志](https://blog.csdn.net/lcl130/article/details/41891273)
+ [Exception Programming Topics](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/Exceptions/Exceptions.html#//apple_ref/doc/uid/10000012-BAJGFBFB)
+ [iOS被开发者遗忘在角落的NSException-其实它很强大](https://www.jianshu.com/p/05aad21e319e)
+ [iOS runtime实用篇--和常见崩溃say good-bye！](https://www.jianshu.com/p/5d625f86bd02)
+ [异常处理NSException的使用（思维篇）](https://www.cnblogs.com/cchHers/p/15116833.html)
+ [异常统计- IOS 收集崩溃信息 NSEXCEPTION类](https://www.freesion.com/article/939519506/)
+ [NSException异常处理](https://www.cnblogs.com/fuland/p/3668004.html)
+ [iOS Crash之NSGenericException](https://blog.csdn.net/skylin19840101/article/details/51945558)
+ [iOS异常处理](https://www.jianshu.com/p/1e4d5421d29c)
+ [iOS异常处理](https://www.jianshu.com/p/59927211b745)
+ [iOS crash分类,Mach异常、Unix 信号和NSException 异常](https://blog.csdn.net/u014600626/article/details/119517507?spm=1001.2101.3001.6661.1&utm_medium=distribute.pc_relevant_t0.none-task-blog-2%7Edefault%7ECTRLIST%7Edefault-1.no_search_link&depth_1-utm_source=distribute.pc_relevant_t0.none-task-blog-2%7Edefault%7ECTRLIST%7Edefault-1.no_search_link)
+ [iOS Mach异常和signal信号](https://developer.aliyun.com/article/499180)



