# iOS App 启动优化(七)：NSProcessInfo 解析

&emsp;NSProcessInfo 是我们统计 APP 启动时间时必会用到的一个类，下面我们就通过官方文档对它进行学习。

## NSProcessInfo

&emsp;NSProcessInfo 就是系统进程信息对象，其中包含一些方法，允许你设置或检索正在运行的应用程序（即进程）的各种类型的信息。

&emsp;A collection of information about the current process.（关于当前进程的一个信息集合）

### Overview

&emsp;每个进程都有一个共享的 `NSProcessInfo` 对象，称为进程信息代理（`NSProcessInfo *info = [NSProcessInfo processInfo];`）。

&emsp;进程信息代理可以返回参数（`arguments`）、环境变量（`environment `）、主机名（`hostName`）和进程名（`name`）等信息。`processInfo` 类方法返回当前进程的共享代理，即其对象发送消息的进程。例如，以下行返回 `NSProcessInfo` 对象，然后提供当前进程的名称：

```c++
NSString *processName = [[NSProcessInfo processInfo] processName];

// 打印：（当前项目的名字）
🤯🤯🤯 Test_ipa_simple
```

> Note
> `NSProcessInfo` 在 `macOS 10.7` 及更高版本中是线程安全的。（看到这里我们应该会意识到，`NSProcessInfo` 对象的属性和方法并不都是只读的，还有一些我们可以对其进行设置。在我们项目里面的话在任何地方都可以通过 `[NSProcessInfo processInfo]` 来取得当前进程信息对象。）

&emsp;`NSProcessInfo` 类还包括 `operatingSystem` 方法，该方法返回一个枚举常量，标识在其上执行进程的操作系统。

&emsp;如果 `NSProcessInfo` 对象无法将环境变量（environment variables）和命令行参数（command-line arguments）转换为 `Unicode` 作为 `UTF-8` 字符串，则它们会尝试解释用户默认 C 字符串编码中的环境变量和命令行参数。如果 Unicode 和 C 字符串转换都不起作用，`NSProcessInfo` 对象将忽略这些值。（这里描述的应该是使用 `-operatingSystemVersion` 或 `-isOperatingSystemAtLeastVersion:` 代替 `operatingSystem`）

### Managing Activities

&emsp;为了用户的利益，系统具有启发式以提高应用程序的电池寿命、性能和响应能力。你可以使用以下方法来管理或向系统提示你的应用程序有特殊要求的活动（Activities）：

+ `beginActivityWithOptions:reason:`
+ `endActivity:`
+ `performActivityWithOptions:reason:usingBlock:`

&emsp;作为对创建活动（activity）的响应，系统将禁用部分或全部启发式方法，以便你的应用程序可以快速完成，同时在用户需要时仍提供响应行为。

&emsp;当你的应用程序执行长时间运行的操作时，你可以使用活动（activities）。如果活动（activity ）可能需要不同的时间（例如，计算国际象棋游戏中的下一步），则应使用此 API。这将确保在数据量或用户计算机功能发生变化时的正确行为。你应该将你的活动归入两个主要类别之一：

1. 用户发起的：这些是​​用户明确开始的有限长度的活动。例如：导出或下载用户指定的文件。
2. 后台任务：这些是有限长度的活动，它们是应用程序正常操作的一部分，但不是由用户明确启动的。例如：自动保存、索引和自动下载文件。

&emsp;此外，如果你的应用程序需要高优先级 I/O，你可以包含 `NSActivityLatencyCritical` 标志（使用按位 OR）。你应该只将此标志用于确实需要高优先级的音频或视频录制等活动。

&emsp;如果你的活动在主线程上的事件回调中同步发生，则不需要使用此 API。

&emsp;请注意，长时间未能结束这些活动可能会对用户计算机的性能产生重大负面影响，因此请确保仅使用所需的最短时间。用户偏好（preferences）可能会覆盖你的的应用程序的请求。

&emsp;你还可以使用此 API 来控制自动终止（automatic termination）或突然终止（sudden termination）（请参阅 Sudden Termination）。例如：

```c++
id activity = [[NSProcessInfo processInfo] beginActivityWithOptions:NSActivityAutomaticTerminationDisabled reason:@"Good Reason"];

// Perform some work

[[NSProcessInfo processInfo] endActivity:activity];
```

相当于:

```c++
[[NSProcessInfo processInfo] disableAutomaticTermination:@"Good Reason"];

// Perform some work

[[NSProcessInfo processInfo] enableAutomaticTermination:@"Good Reason"];
```

&emsp;由于此 API 返回一个对象，因此与使用自动终止 API 相比，将开始和结束配对可能更容易——如果在 `endActivity:` 调用之前释放对象，则活动将自动结束。

&emsp;该 API 还提供了一种机制来禁用系统范围的空闲睡眠（system-wide idle sleep）和显示空闲睡眠（display idle sleep）。这些会对用户体验产生很大的影响，所以一定不要忘记结束禁用睡眠的活动（包括 `NSActivityUserInitiated`）。

### Sudden Termination

&emsp;`macOS 10.6` 及更高版本包含一种机制，允许系统通过在可能的情况下终止应用程序，而不是请求应用程序自行退出，从而来更快地注销或关闭应用程序。

&emsp;你的应用程序可以在全局基础上启用此功能，然后在允许突然终止可能导致数据损坏或用户体验不佳的操作期间手动覆盖其可用性。或者，你的应用程序可以手动启用和禁用此功能。

&emsp;方法 `enableSuddenTermination` 和 `disableSuddenTermination` 分别减少或增加一个计数器，该计数器在第一次创建进程时的值为 1。当计数器的值为0时，应用程序被认为是可以安全终止的，并且可以由系统终止，而无需首先向进程发送任何通知或事件。

&emsp;通过向应用程序的 `Info.plist` 添加一个键，你的应用程序可以支持在启动时突然终止。如果 `Info.plist` 中存在 `NSSupportsSuddenTermination` 键，并且其值为 `YES`，则相当于在应用程序启动期间调用 `enableSuddenTermination`。这使得应用程序进程可以立即终止。你仍然可以通过调用 `disableSuddenTermination` 来覆盖此行为。

&emsp;通常，当应用程序延迟了必须在应用程序终止之前完成的工作时，可以禁用突然终止。例如，如果应用程序延迟将数据写入磁盘，并且启用了突然终止，则应将敏感操作与 `disableSuddenTermination` 调用放在一起，执行必要的操作，然后发送一个用于平衡的 `enableSuddenTermination` 消息。

&emsp;在不依赖于 `AppKit` 的代理或守护程序可执行文件中，你可以立即手动调用 `enableSuddenTermination`。然后，只要进程在终止之前有必须完成的工作，就可以使用 `enable` 和 `disable` 方法。

&emsp;某些 `AppKit` 功能会自动临时禁用突然终止（`sudden termination`）以确保数据完整性。

+ `NSUserDefaults` 暂时禁用突然终止（`sudden termination`），以防止在设置默认值和将包含该默认值的首选项文件写入磁盘时进程被终止。

+ `NSDocument` 暂时禁用突然终止（`sudden termination`），以防止在用户对文档进行更改和将用户更改写入磁盘时进程被终止。

> Note
> 你可以使用以下 `LLDB` 命令确定突然终止（`sudden termination`）的值。
> `print (long)[[NSClassFromString(@"NSProcessInfo") processInfo] _suddenTerminationDisablingCount]`
> 不要试图在应用程序中调用或重写 `suddenTerminationDisablingCount`（私有方法）。它只是为了调试目的而存在的，并且可能随时消失。

### Thermal State and App Performance in macOS

&emsp;`macOS` 中的 Thermal State（热状态）和应用程序性能。

&emsp;在 `macOS` 中，使用当前的热状态（`thermal state`）来确定应用程序是否应该减少系统使用。在 `macOS 10.10.3` 及更高版本中，你可以注册 `NSProcessInfoThermalStateDidChangeNotification`，以便在热状态更改时收到通知。使用 `thermalState`（ `@property(readonly) NSProcessInfoThermalState thermalState;`）查询当前状态。你的应用程序应该减少系统在 higher thermal states 下的使用。有关建议的操作，请参阅 `NSProcessInfoThermalState`。

### Topics

#### Getting the Process Information Agent 

##### processInfo

&emsp;`@property (class, readonly, strong) NSProcessInfo *processInfo;` 返回当前进程的进程信息代理（进程的共享进程信息代理。），它是一个类属性，我们可以使用 `NSProcessInfo` 直接调用取得。

&emsp;第一次调用此方法时会创建一个 `NSProcessInfo` 对象，并且在每次后续调用时都会返回相同的对象。

#### Accessing Process Information

##### arguments

&emsp;`@property (readonly, copy) NSArray<NSString *> *arguments;` 包含进程的命令行（command-line）参数的字符串数组。此数组包含在 `argv` 数组中传递的所有信息，`argv` 即 `main` 函数的参数，其第一个元素便是当前可执行文件名。如下示例代码打印：

```c++
int main(int argc, char * argv[]) {
    NSString * appDelegateClassName;

    NSLog(@"🦁🦁🦁 %s", __func__);
    
    NSProcessInfo *info = [NSProcessInfo processInfo];
    NSLog(@"🤯🤯🤯 %@", info.arguments);
    printf("🤯🤯🤯 argc: %d \n", argc);
    printf("🤯🤯🤯 %s \n", argv[0]);
    
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
        appDelegateClassName = NSStringFromClass([AppDelegate class]);
    }
    return UIApplicationMain(argc, argv, nil, appDelegateClassName);
}

// 控制台输出：
2021-07-15 22:27:35.041884+0800 Test_ipa_simple[14236:1094002] 🦁🦁🦁 main
2021-07-15 22:27:35.042078+0800 Test_ipa_simple[14236:1094002] 🤯🤯🤯 (
    "/Users/hmc/Library/Developer/CoreSimulator/Devices/CC2922E4-A2DB-43DF-8B6F-D2987F683525/data/Containers/Bundle/Application/67BEBCD2-E89F-4742-B3B0-1827F0E98BD8/Test_ipa_simple.app/Test_ipa_simple"
)
🤯🤯🤯 argc: 1 
🤯🤯🤯 /Users/hmc/Library/Developer/CoreSimulator/Devices/CC2922E4-A2DB-43DF-8B6F-D2987F683525/data/Containers/Bundle/Application/67BEBCD2-E89F-4742-B3B0-1827F0E98BD8/Test_ipa_simple.app/Test_ipa_simple 
```

&emsp;我们也可以在 `Edit Scheme... -> Run -> Arguments -> Arguments Passed On Launch` 中添加变量，即给 `main` 函数添加启动时的参数，例如：`{"name":"iOS","arme":"参数"}`，这样我们打印 `[NSProcessInfo processInfo].arguments` 便可得如下打印：

```c++
2021-07-15 22:33:01.141910+0800 Test_ipa_simple[14299:1099660] 🤯🤯🤯 (
    "/Users/hmc/Library/Developer/CoreSimulator/Devices/CC2922E4-A2DB-43DF-8B6F-D2987F683525/data/Containers/Bundle/Application/6CC75292-B479-4FC0-A5B1-A21C11BEF2D5/Test_ipa_simple.app/Test_ipa_simple",
    "{name:iOS,arme:\U53c2\U6570}"
)
```

##### environment

&emsp;`@property (readonly, copy) NSDictionary<NSString *, NSString *> *environment;`

&emsp;启动进程的环境中的变量名称（键）及其值。（内容过多，这里就不直接贴出来了。）

##### globallyUniqueString

&emsp;`@property (readonly, copy) NSString *globallyUniqueString;`

&emsp;进程的全局唯一标识符。

&emsp;进程的全局 ID 包括主机名、进程ID和时间戳，这确保了该 ID 对于网络是唯一的。此属性在每次调用其 getter 时生成一个新字符串，并使用计数器来保证从同一进程创建的字符串是唯一的。

##### macCatalystApp

```c++
@interface NSProcessInfo (NSProcessInfoPlatform)

@property (readonly, getter=isMacCatalystApp) BOOL macCatalystApp API_AVAILABLE(macos(10.15), ios(13.0), watchos(6.0), tvos(13.0));
@property (readonly, getter=isiOSAppOnMac) BOOL iOSAppOnMac API_AVAILABLE(macos(11.0), ios(14.0), watchos(7.0), tvos(14.0));

@end
```

&emsp;一个布尔值，指示进程是否源自于 iOS 应用程序并在 macOS 上运行。

&emsp;当此属性的值为 YES 时，此进程：

+ 使用 `Mac Catalyst` 构建的 Mac 应用程序，或在 Apple silicon 上运行的 iOS 应用程序。
+ 在 Mac 上运行。

&emsp;支持 iOS 和 macOS 的框架使用此属性来确定进程是否是使用 MacCatalyst 构建的 Mac 应用程序。要有条件地编译只在 `macOS` 中运行的源代码，请改用 `#if TARGET_OS_MACCATALYST`。

> Note
> 要区分运行在 Apple silicon 上的 iOS 应用程序和使用 Mac Catalyst 构建的 Mac 应用程序，请使用 iOSAppOnMac 属性。

##### iOSAppOnMac

```c++
@interface NSProcessInfo (NSProcessInfoPlatform)

@property (readonly, getter=isMacCatalystApp) BOOL macCatalystApp API_AVAILABLE(macos(10.15), ios(13.0), watchos(6.0), tvos(13.0));
@property (readonly, getter=isiOSAppOnMac) BOOL iOSAppOnMac API_AVAILABLE(macos(11.0), ios(14.0), watchos(7.0), tvos(14.0));

@end
```

&emsp;一个布尔值，指示进程是在 Mac 上运行的 iPhone 或是 iPad 应用程序。

&emsp;仅当进程是在 Mac 上运行的 iOS 应用程序时，此属性的值才为 YES。对于 Mac 上的所有其他应用程序，包括使用 Mac Catalyst 构建的 Mac 应用程序，该属性的值均为 NO。该属性也不适用于在 macOS 以外的平台上运行的进程。

##### processIdentifier

&emsp;`@property (readonly) int processIdentifier;`

&emsp;进程的标识符（通常称为进程 ID）。

##### processName

&emsp;`@property (copy) NSString *processName;`

&emsp;进程的名称。

&emsp;进程名用于注册应用程序默认值，并在错误消息中使用。它不唯一标识进程。

> Warning
> 用户默认值和环境的其他方面可能取决于进程名称，因此在更改它时要非常小心。以这种方式设置进程名不是线程安全的。

#### Accessing User Information

&emsp;`macOS` 中的每个用户帐户都有一个全名（例如 “Johnny Appleseed”）和一个帐户名称（例如 “jappleseed”）。你可以从 “系统偏好设置” 的 “用户与群组” 面板中查看这些名称，并且可以使用任一名称登录 Mac。

##### userName

```c++
@interface NSProcessInfo (NSUserInformation)

@property (readonly, copy) NSString *userName API_AVAILABLE(macosx(10.12)) API_UNAVAILABLE(ios, watchos, tvos);
@property (readonly, copy) NSString *fullUserName API_AVAILABLE(macosx(10.12)) API_UNAVAILABLE(ios, watchos, tvos);

@end
```

&emsp;返回当前用户的帐户名。（仅 macOS 可见）

##### fullUserName

```c++
@interface NSProcessInfo (NSUserInformation)

@property (readonly, copy) NSString *userName API_AVAILABLE(macosx(10.12)) API_UNAVAILABLE(ios, watchos, tvos);
@property (readonly, copy) NSString *fullUserName API_AVAILABLE(macosx(10.12)) API_UNAVAILABLE(ios, watchos, tvos);

@end
```

&emsp;返回当前用户的全名。（仅 macOS 可见）

#### Sudden Application Termination

> Disable or reenable the ability to be quickly killed. The default implementations of these methods increment or decrement, respectively, a counter whose value is 1 when the process is first created. When the counter's value is 0 the application is considered to be safely killable and may be killed by the operating system without any notification or event being sent to the process first. If an application's Info.plist has an NSSupportsSuddenTermination entry whose value is true then NSApplication invokes -enableSuddenTermination automatically during application launch, which typically renders the process killable right away. You can also manually invoke -enableSuddenTermination right away in, for example, agents or daemons that don't depend on AppKit. After that, you can invoke these methods whenever the process has work it must do before it terminates. For example:
> 
> - NSUserDefaults uses these to prevent process killing between the time at which a default has been set and the time at which the preferences file including that default has been written to disk.
> - NSDocument uses these to prevent process killing between the time at which the user has made a change to a document and the time at which the user's change has been written to disk.
> - You can use these whenever your application defers work that must be done before the application terminates. If for example your application ever defers writing something to disk, and it has an NSSupportsSuddenTermination entry in its Info.plist so as not to contribute to user-visible delays at logout or shutdown time, it must invoke -disableSuddenTermination when the writing is first deferred and -enableSuddenTermination after the writing is actually done.

> &emsp;禁用或重新启用快速被杀死的能力。这些方法的默认实现分别递增或递减一个计数器，当进程首次创建时其值为 1。当计数器的值为 0 时，应用程序被认为是可以安全地终止的，并且可以由操作系统终止，而不首先向进程发送任何通知或事件。如果应用程序的 `Info.plist` 有一个值为 `true` 的 `NSSupportsSuddenTermination` 条目，那么 `NSApplication` 会在应用程序启动期间自动调用 `-enableSsuddenTermination`，这通常会使进程立即终止。例如，你还可以在不依赖 `AppKit` 的代理或守护程序中立即手动调用 `-enableSumddenTermination`。之后，只要进程在终止之前有必须完成的工作，就可以调用这些方法。
> + `NSUserDefaults` 使用这些来防止在设置默认值和将包含该默认值的首选项文件（preferences file）写入磁盘之间的进程终止。
> + `NSDocument` 使用这些来防止在用户对文档进行更改和将用户更改写入磁盘之间的进程终止。
> + 当应用程序延迟必须在应用程序终止之前完成的工作时，可以使用这些命令。例如，如果你的的应用程序曾经延迟将某些内容写入磁盘，并且它的 `Info.plist` 中有一个 `NSSupportsSuddenTermination` 条目，以便在注销或关机时不会造成用户可见的延迟，它必须在第一次延迟写入时调用 `-disablesuddenternimination`，并在实际完成写入后调用 `-enablesumddenternimination`。

##### disableSuddenTermination

&emsp;`- (void)disableSuddenTermination API_AVAILABLE(macos(10.6)) API_UNAVAILABLE(ios, watchos, tvos);`

&emsp;禁用使用 突然终止（sudden termination）快速终止的应用程序。（仅 macOS 可见）

&emsp;此方法递增 突然终止计数器（sudden termination counter）。当终止计数器（termination counter）达到 0 时，应用程序允许突然终止（sudden termination）。

&emsp;默认情况下，突然终止计数器（sudden termination counter）设置为 1。这可以在应用程序的 `Info.plist` 中重写。有关更多信息和调试建议，请参见上面的 Sudden Termination 一节。

##### enableSuddenTermination

&emsp;`- (void)enableSuddenTermination API_AVAILABLE(macos(10.6)) API_UNAVAILABLE(ios, watchos, tvos);`

&emsp;启用应用程序以使用 突然终止（sudden termination.） 快速杀死。（仅 macOS 可见）

&emsp;此方法减少突然终止计数器（sudden termination counter）。当终止计数器达到 0 时，应用程序允许突然终止（ sudden termination）。

&emsp;默认情况下，突然终止计数器（sudden termination counter）设置为 1。这可以在应用程序的 `Info.plist` 中重写。有关更多信息和调试建议，请参见上面的 Sudden Termination 一节。

#### Controlling Automatic Termination

> &emsp;Increment or decrement the counter tracking the number of automatic quit opt-out requests. When this counter is greater than zero, the app will be considered 'active' and ineligible for automatic termination.
  An example of using this would be disabling autoquitting when the user of an instant messaging application signs on, due to it requiring a background connection to be maintained even if the app is otherwise inactive.
  Each pair of calls should have a matching "reason" argument, which can be used to easily track why an application is or is not automatically terminable.
  A given reason can be used more than once at the same time (for example: two files are transferring over the network, each one disables automatic termination with the reason @"file transfer in progress")
> 
> &emsp;增加或减少跟踪自动退出选择退出请求数量的计数器。当此计数器大于零时，应用程序将被视为 “活动” 且不符合自动终止的条件。
  使用此功能的一个示例是在即时消息应用程序的用户登录时禁用自动退出，因为即使应用程序处于非活动状态，它也需要维护后台连接。
  `disableAutomaticTermination:` 和 `enableAutomaticTermination:` 每对调用都应该有一个匹配的 `reason` 参数，它可以用来轻松跟踪应用程序为什么可以或不可以自动终止。
  给定的原因可以同时使用多次（例如：两个文件正在通过网络传输，每个文件都以 `@"file transfer in progress"` 的原因禁用自动终止）

##### disableAutomaticTermination:

&emsp;`- (void)disableAutomaticTermination:(NSString *)reason API_AVAILABLE(macos(10.7)) API_UNAVAILABLE(ios, watchos, tvos);`

&emsp;禁用应用程序的自动终止。`reason` 参数是自动终止被禁用的原因。（仅 macOS 可见）

&emsp;此方法增加自动终止计数器（automatic termination counter）。当计数器大于 0 时，应用程序被认为是活动的，不符合自动终止的条件。例如，你可以在即时消息应用程序的用户登录时禁用自动终止，因为即使应用程序处于非活动状态，应用程序也需要保持后台连接。

&emsp;`reason` 参数用于跟踪应用程序为什么可以或不可自动终止，并且可以通过调试工具进行检查。例如，如果在通过网络传输文件之前禁用自动终止，则可以传递字符串 `@"file transfer in progress"`。在传输完成后使用 `enableAutomaticTermination:` 重新启用自动终止时，你应该传递匹配的字符串。一个给定的理由可以同时使用多次；例如，如果同时传输两个文件，则可以为每个文件禁用自动终止，并传递相同的原因字符串。

##### enableAutomaticTermination:

&emsp;`- (void)enableAutomaticTermination:(NSString *)reason API_AVAILABLE(macos(10.7)) API_UNAVAILABLE(ios, watchos, tvos);`

&emsp;启用应用程序的自动终止。`reason` 参数是启用自动终止的原因。（仅 macOS 可见）

&emsp;此方法减少自动终止计数器（automatic termination counter.）。当计数器为 0 时，应用程序可以自动终止。

&emsp;`reason` 参数用于跟踪应用程序为什么可以或不可自动终止，并且可以通过调试工具进行检查。例如，如果在通过网络传输文件之前禁用自动终止，则可以传递字符串 `@"file transfer in progress"`。在传输完成后使用 `enableAutomaticTermination:` 重新启用自动终止时，你应该传递匹配的字符串。一个给定的理由可以同时使用多次；例如，如果同时传输两个文件，则可以为每个文件禁用自动终止，并传递相同的原因字符串。

##### automaticTerminationSupportEnabled

> &emsp;Marks the calling app as supporting automatic termination. Without calling this or setting the equivalent Info.plist key (NSSupportsAutomaticTermination), the above methods (disableAutomaticTermination:/enableAutomaticTermination:) have no effect, although the counter tracking automatic termination opt-outs is still kept up to date to ensure correctness if this is called later. Currently, passing NO has no effect. This should be called during -applicationDidFinishLaunching or earlier.
> 
> &emsp;置为 `YES` 将调用应用程序标记为支持自动终止。不调用它或设置等效的 `Info.plist` 键 (`NSSupportsAutomaticTermination`)，上述方法 (`disableAutomaticTermination:` / `enableAutomaticTermination:`) 无效，尽管计数器跟踪自动终止选择退出仍然保持最新以确保正确性，如果这是稍后调用。目前，传递 `NO` 没有任何效果。这应该在 `-applicationDidFinishLaunching` 或更早的时候调用。
 
&emsp;`@property BOOL automaticTerminationSupportEnabled API_AVAILABLE(macos(10.7)) API_UNAVAILABLE(ios, watchos, tvos);`

&emsp;一个布尔值，指示应用程序是否支持自动终止。

&emsp;不设置此属性或设置等效的 `Info.plist` 键 (`NSSupportsAutomaticTermination`)，方法 `disableAutomaticTermination:` 和 `enableAutomaticTermination:` 无效，尽管计数器跟踪自动终止选择退出仍保持最新以确保稍后调用时的正确性。目前，将此属性设置为 `NO` 无效。此属性应在应用程序委托方法 `applicationDidFinishLaunching:` 或更早前设置。

#### Getting Host Information

##### hostName

&emsp;`@property (readonly, copy) NSString *hostName;` 

&emsp;正在执行进程的主机的名称。例如我的机器打印: `✳️✳️✳️ hostName:hmdemac-mini.local`

##### operatingSystem

&emsp;`- (NSUInteger)operatingSystem API_DEPRECATED("-operatingSystem always returns NSMACHOperatingSystem, use -operatingSystemVersion or -isOperatingSystemAtLeastVersion: instead", macos(10.0,10.10), ios(2.0,8.0), watchos(2.0,2.0), tvos(9.0,9.0));`

&emsp;已弃用。







&emsp;

```c++
@interface NSProcessInfo : NSObject {
@private
    NSDictionary    *environment;
    NSArray        *arguments;
    NSString        *hostName;
    NSString        *name;
    NSInteger        automaticTerminationOptOutCounter;
}
```

+  @property (readonly, copy) NSArray<NSString *> *arguments; 传入 main 函数中的参数 (可在 Edit Scheme... -> Run -> Arguments -> Arguments Passed On Launch 中添加变量: `{"name":"iOS","arme":"参数"}` )
+ @property (readonly, copy) NSString *hostName; 域名
+ @property (copy) NSString *processName; 进程名称
+ @property (readonly) int processIdentifier; 进程 ID
+ @property (readonly, copy) NSString *globallyUniqueString; 进程全球唯一编号
+ @property (readonly, copy) NSString *operatingSystemVersionString; @property (readonly) NSOperatingSystemVersion operatingSystemVersion API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0)); 系统版本号
+ @property (readonly) NSTimeInterval systemUptime API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0)); 时间段：设备上一次开机至今时间段 



### NSProcessInfo + NSProcessInfoPowerState

&emsp;`NSProcessInfo` 的 `NSProcessInfoPowerState` 分类仅有一个 `lowPowerModeEnabled` 属性。

&emsp;检索系统当前是否设置了低功耗模式。在低功耗模式未知或不受支持的系统上，从 `lowPowerModeEnabled` 属性返回的值始终为 `NO`。

```c++
@interface NSProcessInfo (NSProcessInfoPowerState)

// Retrieve the current setting of the system for the low power mode setting.
// On systems where the low power mode is unknown or unsupported,
// the value returned from the lowPowerModeEnabled property is always NO.

@property (readonly, getter=isLowPowerModeEnabled) BOOL lowPowerModeEnabled API_AVAILABLE(ios(9.0), watchos(2.0), tvos(9.0)) API_UNAVAILABLE(macos);

@end
```


## 参考链接
**参考链接:🔗**
+ [iOS 启动优化 + 监控实践](https://juejin.cn/post/6844904194877587469) // 进行中...

+ [哈啰出行iOS App首屏秒开优化](https://juejin.cn/post/6948990967324082183) // 未开始
+ [抖音研发实践：基于二进制文件重排的解决方案 APP启动速度提升超15%](https://mp.weixin.qq.com/s/Drmmx5JtjG3UtTFksL6Q8Q) // 未开始
+ [iOS App冷启动治理：来自美团外卖的实践](https://juejin.cn/post/6844903733231353863)  // 未开始
+ [APP启动时间最精确的记录方式](https://juejin.cn/post/6844903997153755150)  // 未开始
+ [iOS如何获取当前时间--看我就够了](https://juejin.cn/post/6905671622037307405)  // 未开始
+ [启动优化](https://juejin.cn/post/6983513854546444296)  // 未开始
+ [iOS 优化篇 - 启动优化之Clang插桩实现二进制重排](https://juejin.cn/post/6844904130406793224#heading-29)  // 未开始
+ [懒人版二进制重排](https://juejin.cn/post/6844904192193085448#heading-7)  // 未开始
+ [我是如何让微博绿洲的启动速度提升30%的](https://juejin.cn/post/6844904143111323661)  // 未开始
+ [App性能优化小结](https://juejin.cn/post/6844903704886247431)  // 未开始














/Users/hmc/Documents/GitHub/APPLE_开源代码/objc4_debug/objc4-781

















&emsp;做逆向和静态分析的时候必会看到的文件格式。

&emsp;每个进程都会被分配一个虚拟地址空间，进程寻址的范围就是在这个虚拟地址空间进行的，虚拟地址到物理地址之间有一个映射表进行管理。

&emsp;编译器或任何创建 Mach-O 文件的工具都可以定义额外的节名。这些额外的名称没有出现在表 1 中。

&emsp;在Mach-O文件中的每个section都包含类型和一组属性标记。在中间对象文件中，这个类型和属性决定了静态连接器怎么将section拷贝到最终产品中。对象文件分析工具（例如otool）用类型和属性决定怎么读取和现实这些section。有些section类型和属性是动态连接器用到的。


## Hello World 和编译器

### 预处理

+ 符号化（Tokenization）
+ 宏定义的展开
+ `#include` 的展开

### 语法和语义分析

+ 将符号化后的内容转化为一棵解析树（parse tree）
+ 解析树做语义分析
+ 输出一棵抽象语法树（Abstract Syntax Tree*(AST)）

### 生成代码和优化

+ 将 AST 转换为更低级的中间码（LLVM IR）
+ 对生成的中间码做优化
+ 生成特定目标代码
+ 输出汇编代码

### 汇编器

+ 将汇编代码转换为目标对象文件

### 链接器

+ 将多个目标对象文件合并为一个可执行文件（或者一个动态库）



任意的片段

使用链接符号 -sectcreate 我们可以给可执行文件以 section 的方式添加任意的数据。这就是如何将一个 Info.plist 文件添加到一个独立的可执行文件中的方法。Info.plist 文件中的数据需要放入到 __TEXT segment 里面的一个 __info_plist section 中。可以将 -sectcreate segname sectname file 传递给链接器（通过将下面的内容传递给 clang）：

-Wl,-sectcreate,__TEXT,__info_plist,path/to/Info.plist
同样，-sectalign 规定了对其方式。如果你添加的是一个全新的 segment，那么需要通过 -segprot 来规定 segment 的保护方式 (读/写/可执行)。这些所有内容在链接器的帮助文档中都有，例如 ld(1)。

我们可以利用定义在 /usr/include/mach-o/getsect.h 中的函数 getsectdata() 得到 section，例如 getsectdata() 可以得到指向 section 数据的一个指针，并返回相关 section 的长度。


&emsp;阅读 kyson 老师的 runtime 的专栏。

&emsp;**今天早上的任务就是把 kyson 老师的 runtime 文章全部看完。**

&emsp;






## 参考链接
**参考链接:🔗**
+ [iOS 启动优化 + 监控实践](https://juejin.cn/post/6844904194877587469)
+ [抖音品质建设 - iOS启动优化《实战篇》](https://juejin.cn/post/6921508850684133390)
+ [深入理解MachO数据解析规则](https://juejin.cn/post/6947843156163428383)
+ [探秘 Mach-O 文件](http://hawk0620.github.io/blog/2018/03/22/study-mach-o-file/)
+ [深入剖析Macho (1)](http://satanwoo.github.io/2017/06/13/Macho-1/)
+ [Mach-O 可执行文件](https://objccn.io/issue-6-3/)
+ [巧用nm命令](https://zhuanlan.zhihu.com/p/52984601)

[](https://github.com/zjh171/RuntimeSample)

[](https://xiaozhuanlan.com/runtime)

[](https://blog.csdn.net/jasonblog/article/details/49909209)

[](http://hawk0620.github.io/blog/2018/03/22/study-mach-o-file/)

[](https://easeapi.com/blog/blog/57-ios-dumpdecrypted.html)

[](https://blog.csdn.net/lovechris00/article/details/81561627)

[](https://juejin.cn/post/6844904194877587469)

[](https://www.jianshu.com/p/782c0eb7bc10)

[](https://www.jianshu.com/u/58e5946c7e09)

[](https://juejin.cn/post/6947843156163428383)

[](https://mp.weixin.qq.com/s/vt2LjEbgYsnU1ZI5P9atRw)

[](https://blog.csdn.net/weixin_30463341/article/details/99201551)

[](https://www.cnblogs.com/zhanggui/p/9991455.html)

[](https://objccn.io/issue-6-3/)

+ [iOS dyld详解](https://zhangyu.blog.csdn.net/article/details/92835911?utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromMachineLearnPai2%7Edefault-4.control&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromMachineLearnPai2%7Edefault-4.control)


## 模仿 static_init 调用构造函数

&emsp;在前面的 \_objc_init 过程解析中我们详细分析了 static_init 函数，已知它是

[iOS开发之runtime（11）：Mach-O 犹抱琵琶半遮面](https://xiaozhuanlan.com/topic/0328479651)


&emsp;全局搜索 \__objc_init_func 

## iOS 启动优化 + 监控实践

&emsp;但是每个版本排查启动增量会耗费不少时间,想做一个自动化的启动监控流程来降低这方面的时间成本。

+ 启动流程、
+ 如何优化、
+ push 启动优化、
+ 二进制重排、
+ 后续计划

[iOS 启动优化 + 监控实践](https://juejin.cn/post/6844904194877587469)


&emsp;NSProcessInfo 是我们统计 APP 启动时间时必会用到的一个类，下面我们就通过官方文档对它进行学习。

/Users/hmc/Documents/GitHub/APPLE_开源代码/objc4_debug/objc4-781

&emsp;做逆向和静态分析的时候必会看到的文件格式。

&emsp;每个进程都会被分配一个虚拟地址空间，进程寻址的范围就是在这个虚拟地址空间进行的，虚拟地址到物理地址之间有一个映射表进行管理。

&emsp;编译器或任何创建 Mach-O 文件的工具都可以定义额外的节名。这些额外的名称没有出现在表 1 中。

&emsp;在Mach-O文件中的每个section都包含类型和一组属性标记。在中间对象文件中，这个类型和属性决定了静态连接器怎么将section拷贝到最终产品中。对象文件分析工具（例如otool）用类型和属性决定怎么读取和现实这些section。有些section类型和属性是动态连接器用到的。

## 加载过程


&emsp;当你点击一个 icon 启动应用程序的时候，系统在内部大致做了如下几件事：

+ 内核（OS Kernel）创建一个进程，分配虚拟的进程空间等等，加载动态链接器。
+ 通过动态链接器加载主二进制程序引用的库、绑定符号。
+ 启动程序

&emsp;struct mach_header_64 这个结构体代表的都是 Mach-O 文件的一些元信息，它的作用是让内核在读取该文件创建虚拟进程空间的时候，检查文件的合法性以及当前硬件的特性是否能支持程序的运行。
