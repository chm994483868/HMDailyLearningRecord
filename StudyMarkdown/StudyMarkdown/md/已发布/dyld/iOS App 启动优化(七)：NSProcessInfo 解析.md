# iOS App 启动优化(七)：NSProcessInfo 解析

&emsp;NSProcessInfo 是我们统计 APP 启动时间时必会用到的一个类，下面我们就通过官方文档对它进行学习。

## NSProcessInfo

&emsp;NSProcessInfo 就是系统进程信息对象，其中包含一些方法，允许你设置或检索正在运行的应用程序（即进程）的各种类型的信息。

&emsp;A collection of information about the current process.（关于当前进程的一个信息集合）

### Overview

&emsp;每个进程都有一个共享的 `NSProcessInfo` 对象，称为进程信息代理（`NSProcessInfo *info = [NSProcessInfo processInfo];`）。

&emsp;进程信息代理可以返回参数（`arguments`）（`main` 函数的参数：`char * argv[]`）、环境变量（`environment`）、主机名（`hostName`）和进程名（`name`）等信息。`processInfo` 类方法（或者叫 类属性）返回当前进程的共享代理（即通过这个类属性在当前进程，或者我们当前的程序的任何地方，都返回一个 `NSProcessInfo` 对象，它就代表我们当前的这个程序），即其对象发送消息的进程。例如，以下行返回 `NSProcessInfo` 对象，然后提供当前进程的名称：

```c++
NSString *processName = [[NSProcessInfo processInfo] processName];

// 打印：（当前项目的名字）
🤯🤯🤯 Test_ipa_simple
```

> Note
> `NSProcessInfo` 在 `macOS 10.7` 及更高版本中是线程安全的。（看到这里我们应该会意识到，`NSProcessInfo` 对象的属性和方法并不都是只读的，还有一些我们可以对其进行设置。在我们项目里面的话在任何地方都可以通过 `[NSProcessInfo processInfo]` 来取得当前进程信息对象。）

&emsp;`NSProcessInfo` 类还包括 `operatingSystem` 方法，该方法返回一个枚举常量，标识在其上执行进程的操作系统。

&emsp;如果 `NSProcessInfo` 对象无法将环境变量（environment variables）和命令行参数（command-line arguments）转换为 `Unicode` 作为 `UTF-8` 字符串，则它们会尝试解释用户默认 `C` 字符串编码中的环境变量和命令行参数。如果 `Unicode` 和 `C` 字符串转换都不起作用，`NSProcessInfo` 对象将忽略这些值。

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

&emsp;相当于:

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

&emsp;方法 `enableSuddenTermination` 和 `disableSuddenTermination` 分别减少或增加一个计数器，该计数器在第一次创建进程时的值为 1。当计数器的值为 0 时，应用程序被认为是可以安全终止的，并且可以由系统终止，而无需首先向进程发送任何通知或事件。

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

&emsp;进程的全局 ID 包括主机名、进程 ID 和时间戳，这确保了该 ID 对于网络是唯一的。此属性在每次调用其 `getter` 时生成一个新字符串，并使用计数器来保证从同一进程创建的字符串是唯一的。

##### macCatalystApp

```c++
@interface NSProcessInfo (NSProcessInfoPlatform)

@property (readonly, getter=isMacCatalystApp) BOOL macCatalystApp API_AVAILABLE(macos(10.15), ios(13.0), watchos(6.0), tvos(13.0));
@property (readonly, getter=isiOSAppOnMac) BOOL iOSAppOnMac API_AVAILABLE(macos(11.0), ios(14.0), watchos(7.0), tvos(14.0));

@end
```

&emsp;一个布尔值，指示进程是否源自于 `iOS` 应用程序并在 `macOS` 上运行。

&emsp;当此属性的值为 `YES` 时，此进程：

+ 使用 `Mac Catalyst` 构建的 `Mac` 应用程序，或在 Apple silicon 上运行的 `iOS` 应用程序。
+ 在 `Mac` 上运行。

&emsp;支持 `iOS` 和 `macOS` 的框架使用此属性来确定进程是否是使用 MacCatalyst 构建的 Mac 应用程序。要有条件地编译只在 `macOS` 中运行的源代码，请改用 `#if TARGET_OS_MACCATALYST`。

> Note
> 要区分运行在 Apple silicon 上的 iOS 应用程序和使用 Mac Catalyst 构建的 Mac 应用程序，请使用 `iOSAppOnMac` 属性。

##### iOSAppOnMac

```c++
@interface NSProcessInfo (NSProcessInfoPlatform)

@property (readonly, getter=isMacCatalystApp) BOOL macCatalystApp API_AVAILABLE(macos(10.15), ios(13.0), watchos(6.0), tvos(13.0));
@property (readonly, getter=isiOSAppOnMac) BOOL iOSAppOnMac API_AVAILABLE(macos(11.0), ios(14.0), watchos(7.0), tvos(14.0));

@end
```

&emsp;一个布尔值，指示进程是在 Mac 上运行的 iPhone 或是 iPad 应用程序。

&emsp;仅当进程是在 Mac 上运行的 iOS 应用程序时，此属性的值才为 `YES`。对于 Mac 上的所有其他应用程序，包括使用 Mac Catalyst 构建的 Mac 应用程序，该属性的值均为 `NO`。该属性也不适用于在 macOS 以外的平台上运行的进程。

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

&emsp;看到这里，是不是给我们之前那个 “iOS 应用程序怎么退出？” “答：直接写个数组越界。”，这里是不是又提供了一个 iOS 应用程序退出的新思路。

##### disableSuddenTermination

&emsp;`- (void)disableSuddenTermination API_AVAILABLE(macos(10.6)) API_UNAVAILABLE(ios, watchos, tvos);`

&emsp;禁用使用 突然终止（sudden termination）快速终止的应用程序。（仅 `macOS` 可见）

&emsp;此方法递增 突然终止计数器（sudden termination counter）。当终止计数器（termination counter）达到 0 时，应用程序允许突然终止（sudden termination）。

&emsp;默认情况下，突然终止计数器（sudden termination counter）设置为 1。这可以在应用程序的 `Info.plist` 中重写。有关更多信息和调试建议，请参见上面的 Sudden Termination 一节。

##### enableSuddenTermination

&emsp;`- (void)enableSuddenTermination API_AVAILABLE(macos(10.6)) API_UNAVAILABLE(ios, watchos, tvos);`

&emsp;启用应用程序以使用 突然终止（sudden termination.） 快速杀死。（仅 `macOS` 可见）

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

&emsp;禁用应用程序的自动终止。`reason` 参数是自动终止被禁用的原因。（仅 `macOS` 可见）

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

> &emsp;已弃用。
> &emsp;使用 `operatingSystemVersion` 或  `isOperatingSystemAtLeastVersion:` 代替。

&emsp;返回一个常量，以指示进程正在其上执行的操作系统。

&emsp;操作系统标识符。有关可能值的列表，请参阅 Constants 。在 macOS 中，它是 `NSMACHOperatingSystem`。

##### operatingSystemName

&emsp;`- (NSString *)operatingSystemName API_DEPRECATED("-operatingSystemName always returns NSMACHOperatingSystem, use -operatingSystemVersionString instead", macos(10.0,10.10), ios(2.0,8.0), watchos(2.0,2.0), tvos(9.0,9.0));`

> &emsp;已弃用。
> &emsp;使用 `operatingSystemVersionString` 代替。

&emsp;操作系统名字。在 `macOS` 中，它是 `NSMACHOperatingSystem`。

##### operatingSystemVersionString

> &emsp;Human readable, localized; appropriate for displaying to user or using in bug emails and such; NOT appropriate for parsing
>
> &emsp;人类可读，本地化；适合向用户显示或在错误电子邮件等中使用；不适合解析

&emsp;`@property (readonly, copy) NSString *operatingSystemVersionString;`

&emsp;包含正在执行进程的操作系统版本的字符串。

&emsp;操作系统版本字符串是人类可读的、本地化的，并且适合向用户显示。此字符串不适合进行分析。

```c++
NSProcessInfo *processInfo = [NSProcessInfo processInfo];
// 系统版本号
NSLog(@"✳️✳️✳️ operatingSystemVersionString:%@",processInfo.operatingSystemVersionString);

// 控制台打印
✳️✳️✳️ operatingSystemVersionString:Version 14.4 (Build 18D46)
```

##### operatingSystemVersion

&emsp;`@property (readonly) NSOperatingSystemVersion operatingSystemVersion API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0));`

&emsp;执行进程的操作系统的版本。

##### isOperatingSystemAtLeastVersion:  

&emsp;`- (BOOL) isOperatingSystemAtLeastVersion:(NSOperatingSystemVersion)version API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0));`

&emsp;返回一个布尔值，该值指示执行进程的操作系统的版本是否与给定版本相同或更高。`version` 参数是要测试的操作系统版本。

&emsp;如果进程正在执行的操作系统与给定版本相同或更新，则为 `YES`；否则为 `NO`。 

&emsp;此方法说明操作系统的主要版本、次要版本和更新版本。

#### Getting Computer Information

##### processorCount

&emsp;`@property (readonly) NSUInteger processorCount API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));`

&emsp;计算机上可用的处理核心数。

&emsp;此属性值等于在当前系统上执行 `sysctl -n hw.ncpu` 命令的结果。（在我的机器上执行结果是：16）

##### activeProcessorCount

&emsp;`@property (readonly) NSUInteger activeProcessorCount API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));`

&emsp;计算机上可用的活动处理核心数。

&emsp;`processorCount` 属性报告播发处理核心的数量，而 `activeProcessorCount` 属性则反映系统上活动处理核心的实际数量。有许多不同的因素可能导致核心不活动，包括引导参数、热限制或制造缺陷。

&emsp;`processorCount` 属性报告公布的处理核心数量，而 `activeProcessorCount` 属性反映系统上可活动的（或者可用的）处理核心的实际数量。有许多不同的因素可能导致核心不活动，包括启动参数（boot arguments）、热节流（thermal throttling）或制造缺陷（manufacturing defect）。

&emsp;此属性值等于在当前系统上执行 `sysctl -n hw.logicalcpu` 命令的结果。（在 m1  的 macMini 上是 8）

##### physicalMemory

&emsp;`@property (readonly) unsigned long long physicalMemory API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));`

&emsp;计算机上的物理内存量（以字节为单位）。（即当前电脑的内存，例如我的电脑是 16 G 的，下面的代码打印 16）

```c++
NSProcessInfo *info = [NSProcessInfo processInfo];
NSLog(@"🤯🤯🤯 %lld", info.physicalMemory / 1024 / 1024 / 1024);

// 控制台打印：
Test_ipa_simple[47082:3533173] 🤯🤯🤯 16
```

##### systemUptime

&emsp;`@property (readonly) NSTimeInterval systemUptime API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));`

&emsp;系统自上次重新启动以来一直处于唤醒状态的时间量。（即自上次开机距离现在的的时间）

#### Managing Activities

##### - beginActivityWithOptions:reason:

```c++
@interface NSProcessInfo (NSProcessInfoActivity)
 ...
- (id <NSObject>)beginActivityWithOptions:(NSActivityOptions)options reason:(NSString *)reason API_AVAILABLE(macos(10.9), ios(7.0), watchos(2.0), tvos(9.0));
...
@end
```

> &emsp;Pass in an activity to this API, and a non-NULL, non-empty reason string. Indicate completion of the activity by calling the corresponding endActivity: method with the result of the beginActivityWithOptions:reason: method. The reason string is used for debugging.
> 
> &emsp;调用此 API 开始一个活动，参数传入一个活动选型，以及一个非 NULL、非空的原因字符串。通过使用 `beginActivityWithOptions:reason:` 方法的结果（一个指向活动对象的指针）调用相应的 `endActivity:` 方法来指示活动的完成。原因字符串用于调试。

&emsp;`options` 参数：活动的选项。`NSActivityOptions` 枚举列出了所有有关可能的值。

&emsp;`reason` 参数：用于调试的字符串，用于指示活动开始的原因。

&esmp;返回值是一个指向 `NSObject` 的指针，指向活动的对象。通过调用 `endActivity:` 将返回活动的对象作为参数传递来指示活动的完成。

##### - endActivity:

```c++
@interface NSProcessInfo (NSProcessInfoActivity)
...
- (void)endActivity:(id <NSObject>)activity API_AVAILABLE(macos(10.9), ios(7.0), watchos(2.0), tvos(9.0));
...
@end
```

&emsp;实例函数，结束一个给定的活动（`activety` 参数）。

&emsp;`activity` 参数：`beginActivityWithOptions:reason:` 函数返回的活动对象。

##### performActivityWithOptions:reason:usingBlock:

```c++
@interface NSProcessInfo (NSProcessInfoActivity)
...
- (void)performActivityWithOptions:(NSActivityOptions)options reason:(NSString *)reason usingBlock:(void (^)(void))block API_AVAILABLE(macos(10.9), ios(7.0), watchos(2.0), tvos(9.0));
...
@end
```
> &emsp;Synchronously perform an activity. The activity will be automatically ended after your block argument returns. The reason string is used for debugging.
> 
> &emsp;同步执行一项活动。活动将在 `block` 参数返回后（或者 `block` 内容执行完毕）自动结束。`reason` 字符串用于调试。

&emsp;`options` 参数：活动的选项，`NSActivityOptions` 枚举列出了所有有关可能的值。

&emsp;`reason` 参数：用于调试的字符串，用于指示活动开始的原因。

&emsp;`block` 参数：包含活动要执行的内容。

##### performExpiringActivityWithReason:usingBlock:

```c++
@interface NSProcessInfo (NSProcessInfoActivity)
...
- (void)performExpiringActivityWithReason:(NSString *)reason usingBlock:(void(^)(BOOL expired))block API_AVAILABLE(ios(8.2), watchos(2.0), tvos(9.0)) API_UNAVAILABLE(macos);
...
@end
```
&emsp;异步执行指定的 `block`，并在进程即将挂起时通知你。

> &emsp;Perform an expiring background task, which obtains an expiring task assertion on iOS. 
> The block contains any work which needs to be completed as a background-priority task. 
> The block will be scheduled on a system-provided concurrent queue. After a system-specified time, the block will be called with the `expired` parameter set to YES. 
> The `expired` parameter will also be YES if the system decides to prematurely terminate a previous non-expiration invocation of the block.
>
> &emsp;执行过期后台任务，在 `iOS` 上获取过期任务断言。该 `block` 包含需要作为后台优先任务完成的任何工作。该 `block` 将被安排在系统提供的并发队列中。在系统指定的时间之后，将调用该 `block`，并将 `expired` 参数设置为 `YES`。如果系统决定提前终止之前未到期的 `block` 调用，`expired` 参数也将为 `YES`。

&emsp;`reason` 参数：用于调试的字符串，用于指示活动开始的原因。此参数不能为 `nil` 或空字符串。

&emsp;`block` 参数：包含活动要执行的内容的 block 块。该 `block` 没有返回值并采用以下 `BOOL` 类型的 `expired` 参数：

&emsp;`expired` 参数：一个布尔值，指示进程是否将被挂起。如果值为 `YES`，则进程将被挂起，因此你应该采取任何必要的步骤来停止正在进行的工作。如果 `NO`，则启动计划任务。

&emsp;当进程在后台执行时，使用此方法执行任务。此方法将 `block` 排队，以便在并发队列上异步执行。当你的进程在后台时，该方法会尝试执行任务断言，以确保你的 `block` 有时间执行。如果无法执行任务断言，或者分配给任务断言的时间已过期，则系统将执行 `block` 并将参数设置为 `YES`。如果它能够接受任务断言，它将执行 `block`，并为 `expried` 参数传递 `NO`。

&emsp;如果你的 `block` 仍在执行并且系统需要挂起该进程，则系统将第二次执行你的 `block`，并将 `expired` 参数设置为 `YES`。你的 `block` 必须准备好处理这种情况。当 `expired` 参数为 `YES` 时，尽快停止任何正在进行的任务。

#### Getting the Thermal State

##### thermalState

```c++
@interface NSProcessInfo (NSProcessInfoThermalState)

@property (readonly) NSProcessInfoThermalState thermalState API_AVAILABLE(macosx(10.10.3), ios(11.0), watchos(4.0), tvos(11.0));

@end
```

> &emsp;Retrieve the current thermal state of the system. On systems where thermal state is unknown or unsupported, the value returned from the thermalState property is always NSProcessInfoThermalStateNominal.
> 
> &emsp;检索系统的当前热状态。在热状态未知或不受支持的系统上，从 `thermalState` 属性返回的值始终为 `NSProcessInfoThermalStateNominal`。

&emsp;返回带有系统当前热状态的 `NSProcessInfoThermalState`（一个枚举值）。在较高的热状态下，你的应用应减少系统资源的使用。有关更多信息，参阅 `NSProcessInfoThermalState` 枚举。

#### Determining Whether Low Power Mode is Enabled（确定是否启用低功耗模式）

##### lowPowerModeEnabled

```c++s
@interface NSProcessInfo (NSProcessInfoPowerState)

@property (readonly, getter=isLowPowerModeEnabled) BOOL lowPowerModeEnabled API_AVAILABLE(ios(9.0), watchos(2.0), tvos(9.0)) API_UNAVAILABLE(macos);

@end
```

> &emsp;Retrieve the current setting of the system for the low power mode setting. On systems where the low power mode is unknown or unsupported, the value returned from the lowPowerModeEnabled property is always NO
>
> &emsp;为低功耗模式设置检索系统的当前设置。在低功耗模式未知或不受支持的系统上，从 `lowPowerModeEnabled` 属性返回的值始终为 `NO`

&emsp;一个布尔值，指示是否在 `iOS` 设备上启用了低功耗模式。

&emsp;希望延长 iPhone 电池寿命的用户可以在 “设置”>“电池” 下启用 “低电量模式”。在低电量模式下，iOS 通过制定某些节能措施来延长电池寿命，例如降低 CPU 和 GPU 性能、降低屏幕亮度以及暂停自主活动和后台活动。你的应用程序可以随时查询 `lowPowerModeEnabled` 属性以确定低功耗模式是否处于活动状态。

&emsp;当 iOS 设备的电源状态（低功耗模式启用或禁用）发生更改时，你的应用程序也可以注册以接收通知。要注册电源状态变化的通知，请将消息 `addObserver:selector:name:object:` 发送到应用程序的默认通知中心（`NSNotificationCenter` 的实例）。向其传递要调用的选择器和 `NSProcessInfoPowerStateDidChangeNotification` 的通知名称。一旦应用程序收到电源状态更改的通知，它应该查询 `isLowPowerModeEnabled` 以确定当前的电源状态。如果低功耗模式处于活动状态，则应用程序可以采取适当的步骤来减少活动。否则，可以恢复正常操作。

&emsp;关于更详细的信息，可参考：[Energy Efficiency and the User Experience](https://developer.apple.com/library/archive/documentation/Performance/Conceptual/EnergyGuide-iOS/index.html#//apple_ref/doc/uid/TP40015243) 中的 [React to Low Power Mode on iPhones](https://developer.apple.com/library/archive/documentation/Performance/Conceptual/EnergyGuide-iOS/LowPowerMode.html#//apple_ref/doc/uid/TP40015243-CH31) 部分。

###### NSProcessInfoPowerStateDidChangeNotification

```c++
FOUNDATION_EXTERN NSNotificationName const NSProcessInfoPowerStateDidChangeNotification API_AVAILABLE(ios(9.0), watchos(2.0), tvos(9.0)) API_UNAVAILABLE(macos);
```

> &emsp;NSProcessInfoPowerStateDidChangeNotification is posted once any power usage mode of the system has changed. Once the notification is posted, use the isLowPowerModeEnabled property to retrieve the current state of the low power mode setting of the system.
> &emsp;When this notification is posted your application should attempt to reduce power usage by reducing potentially costly computation and other power using activities like network activity or keeping the screen on if the low power mode setting is enabled.
> &emsp;This notification is posted on the global dispatch queue. Register for it using the default notification center. The object associated with the notification is NSProcessInfo.processInfo.
> 
> &emsp;一旦系统的任何电源使用模式发生更改，就会发布 `NSProcessInfoPowerStateDidChangeNotification`。发布通知后，请使用 `isLowPowerModeEnabled` 属性检索系统的低功耗模式设置的当前状态。
> &emsp;;当此通知发布时，你的应用程序应尝试通过减少潜在的昂贵计算和其他使用活动（如网络活动）或在启用低功耗模式设置时保持屏幕开启的功耗来降低功耗。
> &emsp;此通知发布在全局调度队列上。使用默认通知中心注册。与通知关联的对象是 `NSProcessInfo.processInfo`。

&emsp;当 iOS 设备的电源状态（启用或禁用低功耗模式）发生变化时发布（发出此通知）。

&emsp;发布通知后，查询 `lowPowerModeEnabled` 属性以确定设备的当前电源状态。如果低功耗模式处于活动状态，则应用程序可以采取适当的步骤来减少活动。否则，可以恢复正常操作。

&emsp;通知对象是 `NSProcessInfo` 实例。

#### Constants

##### NSOperatingSystemVersion

```c++
typedef struct {
    NSInteger majorVersion;
    NSInteger minorVersion;
    NSInteger patchVersion;
} NSOperatingSystemVersion;
```

&emsp;操作系统版本的结构体，在 `NSProcessInfo` 类的 `operatingSystemVersion` 属性和 `isOperatingSystemAtLeastVersion:` 方法中使用。

&emsp;`majorVersion` 主版本号，如 10.9.3 版中的 10。

&emsp;`minorVersion` 次版本号，例如 10.9.3 版本中的 9。

&emsp;`patchVersion` 更新版本号，如 10.9.3 版中的 3。

##### NSActivityOptions

```c++
typedef NS_OPTIONS(uint64_t, NSActivityOptions) {
    
    // Used for activities that require the screen to stay powered on.
    // 用于需要屏幕保持开机状态（屏幕一直处于点亮状态）的活动（要求屏幕保持通电状态的标志。）。（1 左移 40 位）
    NSActivityIdleDisplaySleepDisabled = (1ULL << 40),
    
    // Used for activities that require the computer to not idle sleep. This is included in NSActivityUserInitiated.
    // 用于要求计算机不能闲置睡眠的活动。这包含在 NSActivityUserInitiated 中。
    NSActivityIdleSystemSleepDisabled = (1ULL << 20),
    
    // Prevents sudden termination. This is included in NSActivityUserInitiated.
    // 防止突然终止。这包括在 NSActivityUserInitiated 中。
    NSActivitySuddenTerminationDisabled = (1ULL << 14),
    
    // Prevents automatic termination. This is included in NSActivityUserInitiated.
    // 防止自动终止。这包括在NSActivityUserInitiated中。
    NSActivityAutomaticTerminationDisabled = (1ULL << 15),
    
    // ----
    // Sets of options.
    // 选项集合
    
    // App is performing a user-requested action.
    // 应用正在执行用户请求的操作。（指示应用程序正在执行用户请求的操作的标志。）
    NSActivityUserInitiated = (0x00FFFFFFULL | NSActivityIdleSystemSleepDisabled),
    
    // 标志以指示应用程序正在执行用户请求的操作，但系统可以在空闲时休眠。
    NSActivityUserInitiatedAllowingIdleSystemSleep = (NSActivityUserInitiated & ~NSActivityIdleSystemSleepDisabled),
    
    // App has initiated some kind of work, but not as the direct result of user request.
    // 标志，指示应用程序已启动某种工作，但不是用户请求的直接结果。
    NSActivityBackground = 0x000000FFULL,
    
    // Used for activities that require the highest amount of timer and I/O precision available. Very few applications should need to use this constant.
    // 用于需要最高可用计时器和 I/O 精度的活动。很少有应用程序需要使用这个常量。
    NSActivityLatencyCritical = 0xFF00000000ULL,
} API_AVAILABLE(macos(10.9), ios(7.0), watchos(2.0), tvos(9.0));
```

> &emsp;The system has heuristics to improve battery life, performance, and responsiveness of applications for the benefit of the user. This API can be used to give hints to the system that your application has special requirements. In response to creating one of these activities, the system will disable some or all of the heuristics so your application can finish quickly while still providing responsive behavior if the user needs it.
> &emsp;为了用户的利益，系统具有启发式以提高应用程序的电池寿命、性能和响应能力。此 API 可用于向系统提示你的应用程序有特殊要求。为了响应创建这些活动之一，系统将禁用部分或全部启发式方法，以便你的应用程序可以快速完成，同时在用户需要时仍提供响应行为。
> 
> &emsp;These activities can be used when your application is performing a long-running operation. If the activity can take different amounts of time (for example, calculating the next move in a chess game), it should use this API. This will ensure correct behavior when the amount of data or the capabilities of the user's computer varies. You should put your activity into one of two major categories:
> &emsp;当你的应用程序正在执行长时间运行的操作时，可以使用这些活动。如果活动可能需要不同的时间（例如，计算国际象棋游戏中的下一步），则应使用此 API。这将确保在数据量或用户计算机功能发生变化时的正确行为。你应该将你的活动归入两个主要类别之一：
> 
> &emsp;User initiated: These are finite length activities that the user has explicitly started. Examples include exporting or downloading a user specified file.
> &emsp;用户发起：这些是用户显式启动的有限长度活动。示例包括导出或下载用户指定的文件。
>
> &emsp;Background: These are finite length activities that are part of the normal operation of your application but are not explicitly started by the user. Examples include autosaving, indexing, and automatic downloading of files.
> &emsp;Background：这些是有限长度的活动，是应用程序正常操作的一部分，但不是由用户显式启动的。示例包括文件的自动保存、索引和自动下载。
> 
> &emsp;In addition, if your application requires high priority IO, you can include the 'NSActivityLatencyCritical' flag (using a bitwise or). This should be reserved for activities like audio or video recording.
> &emsp;此外，如果你的应用程序需要高优先级 IO，你可以包含 `NSActivityLatencyCritical` 标志（使用按位 `or`）。这应该保留用于音频或视频录制等活动。
> 
> &emsp;If your activity takes place synchronously inside an event callback on the main thread, you do not need to use this API.
> &emsp;如果你的活动在主线程上的事件回调中同步发生，则不需要使用此 API。
> 
> &emsp;Be aware that failing to end these activities for an extended period of time can have significant negative impacts to the performance of your user's computer, so be sure to use only the minimum amount of time required. User preferences may override your application’s request.
> &emsp;请注意，长时间未能结束这些活动可能会对用户计算机的性能产生重大负面影响，因此请确保仅使用所需的最短时间。用户偏好（preferences）可能会覆盖你的应用程序的请求。
> 
> &emsp;This API can also be used to control auto termination or sudden termination. 
> &emsp;这个 API 还可以用来控制自动终止或突然终止。
>
>   `id activity = [NSProcessInfo.processInfo beginActivityWithOptions:NSActivityAutomaticTerminationDisabled reason:@"Good Reason"];`
>   // work
>  `[NSProcessInfo.processInfo endActivity:activity];`
> 
> &emsp;is equivalent to:
> &emsp;相当于：
>
>  ` [NSProcessInfo.processInfo disableAutomaticTermination:@"Good Reason"];`
>   // work
>    `[NSProcessInfo.processInfo enableAutomaticTermination:@"Good Reason"]`
>
> &emsp;Since this API returns an object, it may be easier to pair begins and ends. If the object is deallocated before the -endActivity: call, the activity will be automatically ended.
> &emsp;由于此 API 返回一个对象，因此将开始和结束配对可能更容易。如果在 `-endActivity:` 调用之前释放对象，则活动将自动结束。
> 
> &emsp;This API also provides a mechanism to disable system-wide idle sleep and display idle sleep. These can have a large impact on the user experience, so be sure not to forget to end activities that disable sleep (including NSActivityUserInitiated).
> &emsp;这个 API 还提供了一种机制来禁用系统范围的空闲睡眠和显示空闲睡眠。这些可能会对用户体验产生很大影响，因此请确保不要忘记结束禁用睡眠的活动（包括 `NSActivityUserInitiated`）。

&emsp;用于 `beginActivityWithOptions:reason:` 函数和 `performActivityWithOptions:reason:usingBlock:` 函数的选项标志。

&emsp;要将这些单个标志之一包含在其中一个集合中，请使用按位 `OR`；例如，在演示过程中，你可能会使用：

```c++
NSActivityUserInitiated | NSActivityIdleDisplaySleepDisabled
```

&emsp;要从其中一个集合中排除，请使用按位 `AND` 和 `NOT`；例如，在用户发起的操作期间，如果注销，则可以在没有应用程序交互的情况下安全终止该操作，你可以使用：

```c++
NSActivityUserInitiated & ~NSActivitySuddenTerminationDisabled
```

##### NSProcessInfoThermalState

```c++
// Describes the current thermal state of the system.
// 描述系统的当前热状态。
typedef NS_ENUM(NSInteger, NSProcessInfoThermalState) {
    // No corrective action is needed.
    // 无需采取纠正措施。（热状态在正常范围内。）
    NSProcessInfoThermalStateNominal,

    // The system has reached a state where fans may become audible (on systems which have fans). Recommendation: Defer non-user-visible activity.
    // 系统已达到可以听到风扇声音的状态（在有风扇的系统上）。建议：推迟非用户可见的活动。
    NSProcessInfoThermalStateFair,

    // Fans are running at maximum speed (on systems which have fans), system performance may be impacted. Recommendation: reduce application's usage of CPU, GPU and I/O, if possible. Switch to lower quality visual effects, reduce frame rates.
    // 风扇以最大速度运行（在有风扇的系统上），系统性能可能会受到影响。建议：如果可能，减少应用程序对 CPU、GPU 和 I/O 的使用。切换到较低质量的视觉效果，降低帧率。
    NSProcessInfoThermalStateSerious,
    
    // System performance is significantly impacted and the system needs to cool down. Recommendation: reduce application's usage of CPU, GPU, and I/O to the minimum level needed to respond to user actions. Consider stopping use of camera and other peripherals if your application is using them.
    // 系统性能受到显着影响，系统需要冷却。建议：将应用程序对 CPU、GPU 和 I/O 的使用降低到响应用户操作所需的最低水平。如果你的应用程序正在使用相机和其他外围设备，请考虑停止使用它们。
    NSProcessInfoThermalStateCritical
} API_AVAILABLE(macosx(10.10.3), ios(11.0), watchos(4.0), tvos(11.0));
```

&emsp;用于指示系统热状态的值。

&emsp;`NSProcessInfo` 类使用这些值作为热状态（thermalState）的返回值。

&emsp;有关在不同热状态下测试你的应用程序的信息，请参阅 [Test under adverse device conditions (iOS)](https://help.apple.com/xcode/mac/current/#/dev308429d42)

###### NSProcessInfoThermalStateNominal

&emsp;`NSProcessInfoThermalStateNominal` 热状态在正常范围内。  

###### NSProcessInfoThermalStateFair

&emsp;`NSProcessInfoThermalStateFair` 热状态略有升高。系统采取措施减少热状态，例如运行风扇和停止不立即执行用户需要的工作的后台服务。减少或推迟后台工作，例如通过网络预取内容或更新数据库索引。

###### NSProcessInfoThermalStateSerious

&emsp;`NSProcessInfoThermalStateSerious` 热状态高。系统采取适度的步骤来减少热状态，这会降低性能。风扇以最大速度运行。

&emsp;减少产生热量和消耗电池的资源的使用，例如：

+ 减少或推迟 I/O 操作，例如网络和蓝牙

+ 降低要求的定位精度水平

+ 通过停止或延迟工作来减少 CPU 和 GPU 的使用

+ 将目标帧速率从 60 FPS 降低到 30 FPS

+ 通过使用较少的粒子或较低分辨率的纹理来降低渲染内容的细节级别

&emsp;有关如何减少应用对这些资源的使用的更多详细信息，请参考：[Energy Efficiency and the User Experienc](https://developer.apple.com/library/archive/documentation/Performance/Conceptual/EnergyGuide-iOS/index.html#//apple_ref/doc/uid/TP40015243) 和 [Energy Efficiency Guide for Mac Apps](https://developer.apple.com/library/archive/documentation/Performance/Conceptual/power_efficiency_guidelines_osx/index.html#//apple_ref/doc/uid/TP40013929)。

##### NSProcessInfoThermalStateDidChangeNotification

```c++
FOUNDATION_EXTERN NSNotificationName const NSProcessInfoThermalStateDidChangeNotification API_AVAILABLE(macosx(10.10.3), ios(11.0), watchos(4.0), tvos(11.0));
```

> &emsp;NSProcessInfoThermalStateDidChangeNotification is posted once the thermal state of the system has changed. Once the notification is posted, use the thermalState property to retrieve the current thermal state of the system.
> &emsp;`NSProcessInfoThermalStateDidChangeNotification` 在系统的热状态发生更改后发布。发布通知后，使用 `thermalState` 属性检索系统的当前热状态。
> 
> &emsp;You can use this opportunity to take corrective action in your application to help cool the system down. Work that could be done in the background or at opportunistic times should be using the Quality of Service levels in NSOperation or the NSBackgroundActivityScheduler API.
> &emsp;你可以利用这个机会在应用程序中采取纠正措施来帮助冷却系统。可以在后台或机会主义时间完成的工作应该使用 `NSOperation` 或 `NSBackgroundActivityScheduler` API 中的服务质量级别。
>
> &emsp;This notification is posted on the global dispatch queue. Register for it using the default notification center. The object associated with the notification is NSProcessInfo.processInfo.
> &emsp;此通知发布在全局调度队列上。使用默认通知中心注册它。与通知关联的对象是 `NSProcessInfo.processInfo`。

&emsp;是一个通知的名字，当系统的热状态发生变化时发布（发出此通知）。

&emsp;至此 `NSProcesshidp` 的内容就全部都看完了，基本上就是对当前的应用程序（进程）的信息进行获取，然后根据不同的状态对进程进行一些状态的调整应对，以优化我们的设备更好的运行。

## 参考链接
**参考链接:🔗**
+ [iOS 启动优化 + 监控实践](https://juejin.cn/post/6844904194877587469)
+ [哈啰出行iOS App首屏秒开优化](https://juejin.cn/post/6948990967324082183)
+ [抖音研发实践：基于二进制文件重排的解决方案 APP启动速度提升超15%](https://mp.weixin.qq.com/s/Drmmx5JtjG3UtTFksL6Q8Q)
+ [iOS App冷启动治理：来自美团外卖的实践](https://juejin.cn/post/6844903733231353863)
+ [APP启动时间最精确的记录方式](https://juejin.cn/post/6844903997153755150)
+ [iOS如何获取当前时间--看我就够了](https://juejin.cn/post/6905671622037307405)
+ [启动优化](https://juejin.cn/post/6983513854546444296)
+ [iOS 优化篇 - 启动优化之Clang插桩实现二进制重排](https://juejin.cn/post/6844904130406793224#heading-29)
+ [懒人版二进制重排](https://juejin.cn/post/6844904192193085448#heading-7)
+ [我是如何让微博绿洲的启动速度提升30%的](https://juejin.cn/post/6844904143111323661)
+ [App性能优化小结](https://juejin.cn/post/6844903704886247431)
