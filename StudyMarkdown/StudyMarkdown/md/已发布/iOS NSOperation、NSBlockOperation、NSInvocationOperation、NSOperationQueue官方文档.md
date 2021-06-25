# iOS NSOperation、NSBlockOperation、NSInvocationOperation、NSOperationQueue官方文档

## NSOperation
&emsp;表示与单个任务关联的代码和数据的抽象类。
```c++
API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0))
@interface NSOperation : NSObject
```
&emsp;因为 NSOperation 类是一个抽象类，所以不直接使用它，而是使用其子类或使用系统定义的子类之一（NSInvocationOperation 或 NSBlockOperation）来执行实际任务。尽管 NSOperation 的基本实现是抽象的，但它确实包含重要的逻辑来协调任务的安全执行。这种内置逻辑的存在使你能够专注于任务的实际实现，而不是确保它与其他系统对象正确工作所需的胶水代码。

&emsp;operation 对象是 single-shot 对象，即它只执行一次任务，不能用于再次执行。通常通过将 operations 添加到操作队列（NSOperationQueue 类的实例）来执行操作。操作队列通过在子线程上运行操作来直接执行其操作，或者间接地使用 libdispatch 库（也称为 Grand Central Dispatch）。有关队列如何执行操作的更多信息，参阅 NSOperationQueue。

&emsp;如果不想使用操作队列，可以直接从代码中调用操作的 start 方法来执行操作。手动执行操作会给代码带来更大的负担，因为 starting  未处于就绪状态（ready state）的操作会触发异常。ready 属性报告了该操作是否准备就绪。
### Operation Dependencies
&emsp;依赖关系是按特定顺序执行操作的一种方便方法。可以使用 `addDependency:` 和 `removedDependency:` 方法添加和删除操作的依赖项。默认情况下，具有依赖关系的操作对象在其所有依赖操作对象完成执行之前，不会被视为就绪。但是，一旦最后一个依赖操作完成，操作对象就可以执行了。

&emsp;NSOperation 支持的依赖关系不区分依赖操作是否成功完成。（换句话说，取消一个操作同样会将其标记为已完成。）如果一个具有依赖关系的操作被取消或未成功完成其任务，则由你决定是否应继续执行该操作。这可能需要你将一些额外的错误跟踪功能合并到操作对象中。
### KVO-Compliant Properties
&emsp;NSOperation 类的几个属性都符合键值编码（KVC）和键值观察（KVO）。根据需要，你可以观察这些属性以控制应用程序的其他部分。要观察属性，请使用以下 key paths：

+ isCancelled - read-only
+ isAsynchronous - read-only
+ isExecuting - read-only
+ isFinished - read-only
+ isReady - read-only
+ dependencies - read-only // ⬆️⬆️ 以上都是只读

+ queuePriority - readable and writable // 可读可写
+ completionBlock - readable and writable // 可读可写

&emsp;尽管可以将观察者（observers）附加到这些属性，但不应使用 Cocoa bindings 将它们绑定到应用程序用户界面的元素。与用户界面相关联的代码通常只能在应用程序的主线程中执行。因为一个操作可以在任何线程中执行，所以与该操作相关联的 KVO 通知同样可以在任何线程中发生。

&emsp;如果你为上述任何属性提供自定义实现，则实现必须保持 KVC 和 KVO 兼容。如果你为 NSOperation 对象定义了其他属性，建议你也使这些属性与 KVC 和 KVO 兼容。有关如何支持键值编码的信息，请参阅 [Key-Value Coding Programming Guide](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/KeyValueCoding/index.html#//apple_ref/doc/uid/10000107i)。有关如何支持键值观察的信息，请参阅 [Key-Value Observing Programming Guide](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/KeyValueObserving/KeyValueObserving.html#//apple_ref/doc/uid/10000177i)。
### Multicore Considerations
&emsp;NSOperation 类本身具有多核感知的。因此，可以安全地从多个线程中调用 NSOperation 对象的方法，而无需创建其他锁来同步对该对象的访问。此行为是必要的，因为操作通常在与创建并监视它的线程不同的线程中运行。

&emsp;当你将 NSOperation 子类化时，必须确保任何重写的方法都可以安全地从多个线程调用。如果在子类中实现自定义方法（如自定义数据访问器），还必须确保这些方法是线程安全的。因此，必须同步对操作中任何数据变量的访问，以防止潜在的数据损坏。有关同步的更多信息，请参阅 [Threading Programming Guide](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/Multithreading/Introduction/Introduction.html#//apple_ref/doc/uid/10000057i)。
### Asynchronous Versus Synchronous Operations（异步与同步操作）
&emsp;如果计划手动执行操作对象，而不是将其添加到队列中，则可以将操作设计为以同步或异步方式执行。默认情况下，操作对象是同步的。在同步操作中，操作对象不会创建单独的线程来运行其任务。当你直接从代码中调用同步操作的 start 方法时，该操作将立即在当前线程中执行。当这样一个对象的 start 方法将控制权返回给调用者时，任务本身就完成了。

&emsp;调用异步操作的 start 方法时，该方法可能会在相应任务完成之前返回。异步操作对象负责在单独的线程上调度其任务。该操作可以通过直接启动一个新线程、调用一个异步方法或将一个块提交给调度队列来执行。实际上，当 control 返回给调用方时，操作是否正在进行并不重要，只是它可能正在进行。

&emsp;如果你总是计划使用队列来执行操作，那么将它们定义为同步比较简单。但是，如果手动执行操作，可能需要将操作对象定义为异步。定义异步操作需要更多的工作，因为你必须监视任务的当前状态，并使用 KVO 通知报告该状态中的更改。但是，定义异步操作在需要确保手动执行的操作不会阻塞调用线程的情况下非常有用。（手动执行操作时默认是同步，也可把其定义为异步。使用队列来执行操作的话，同步执行还是异步执行则是根据队列本身来定的。）

&emsp;将操作添加到操作队列时，队列将忽略 asynchronous 属性的值，并始终从单独的线程调用 start 方法。因此，如果总是通过将操作添加到操作队列来运行操作，则没有理由使它们异步。

&emsp;有关如何定义同步和异步操作的信息，请参见子类说明（NSInvocationOperation 或 NSBlockOperation）。
### Subclassing Notes（关于子类化 NSOperation 时的一些注意事项）
&emsp;NSOperation 类提供了跟踪操作的执行状态的基本逻辑，但在其他情况下必须子类化才能执行任何实际工作。如何创建子类取决于操作是设计为并发执行还是非并发执行。
#### Methods to Override（重写方法）
&emsp;对于非并发操作，通常只重写一种方法：

+ main

&emsp;在这个方法中，你可以放置执行给定任务所需的代码。当然，还应该定义一个自定义初始化方法，以便更容易地创建自定义类的实例。你可能还需要定义 getter 和 setter 方法来访问操作中的数据。但是，如果确实定义了自定义 getter 和 setter 方法，则必须确保可以从多个线程安全地调用这些方法。

&emsp;如果要创建并发操作，则至少需要重写以下方法和属性：

+ start
+ asynchronous
+ executing
+ finished

&emsp;在并发操作中，start 方法负责以异步方式启动操作。无论是生成线程还是调用异步函数，都可以通过此方法完成。在启动操作时，start 方法还应该更新 executing 属性所报告的操作的执行状态。 你可以通过为 executing 的 key path 发送 KVO 通知来实现这一点，这会让感兴趣的 clients 知道操作正在运行。executing 属性还必须以线程安全的方式提供状态。

&emsp;完成或取消其任务后，并发操作对象必须为 isExecuting 和 isFinished key path 生成 KVO 通知，以标记操作的最终状态更改。（在取消的情况下，更新 isFinished key path 仍然很重要，即使操作没有完全完成其任务。排队的操作必须报告它们已完成，然后才能从队列中删除。）除了生成 KVO 通知外，对 executing 和 finished 属性的重写还应根据操作的状态继续报告准确的值。

&emsp;有关如何定义并发操作的其他信息和指南，请参见 [Concurrency Programming Guide](https://developer.apple.com/library/archive/documentation/General/Conceptual/ConcurrencyProgrammingGuide/Introduction/Introduction.html#//apple_ref/doc/uid/TP40008091)。

> &emsp;Important: 在 start 方法中，任何时候都不应该调用 super。当你定义一个并发操作时，你需要自己提供与默认 start 方法相同的行为，包括启动任务和生成适当的 KVO 通知。你的 start 方法还应该在实际启动任务之前检查操作本身是否被取消。有关取消语义的更多信息，参考下面的 Responding to the Cancel Command 部分。

&emsp;即使对于并发操作，也不需要重写除上述方法之外的其他方法。但是，如果自定义操作的依赖特性，则可能必须重写其他方法并提供其他 KVO 通知。对于依赖关系，这可能只需要为 isReady key path 提供通知。因为 dependencies 属性包含依赖操作的列表，所以对它的更改已经由默认的 NSOperation 类处理。
#### Maintaining Operation Object States（维护操作对象状态）
&emsp;操作对象在内部维护状态信息，以确定何时可以安全地执行，并在操作的生命周期内通知外部 clients 进程。自定义子类维护此状态信息，以确保代码中操作的正确执行。与操作状态相关联的 key paths 包括：
+ isReady
  isReady key path 让 clients 知道何时可以执行操作。ready 属性包含值 true（当操作准备好立即执行时），或 false（如果仍有未完成的操作依赖于它）。
  
  在大多数情况下，你不必自己管理此 key path 的状态。如果你的操作的准备状态是由非依赖操作的因素决定的，那么你可以自己提供 ready 属性的实现，并自己跟踪操作的准备状态。通常只在外部状态允许的情况下创建操作对象更简单。
  
  在 macOS 10.6 及更高版本中，如果在某个操作等待一个或多个相关操作完成时取消该操作，则这些相关操作将被忽略，并且此属性的值将更新，以反映它现在已准备好运行。此行为使操作队列有机会更快地从其队列中清除已取消的操作。

+ isExecuting
  isExecuting key path 让 clients 知道操作是否正在处理其分配的任务。如果操作正在处理其任务，则 executing 属性必须报告值 true，否则报告值 false。
  
  如果替换操作对象的 start 方法，则还必须替换 executing 属性，并在操作的执行状态更改时生成 KVO 通知。
  
+ isFinished
  isFinished key path 让 clients 知道操作已成功完成其任务或已取消并正在退出。直到 isFinished key path 的值更改为 true，操作对象才会清除依赖项。类似地，在 finished 属性包含值 true 之前，操作队列不会将操作出列。因此，将操作标记为已完成对于防止队列备份正在进行或已取消的操作至关重要。
  
  如果替换 start 方法或操作对象，还必须替换 finished 属性，并在操作完成执行或取消时生成 KVO 通知。
  
+ isCancelled
  isCancelled key path 让 clients 知道已请求取消操作。对取消的支持是自愿的，但受到鼓励，你自己的代码不必为此 key path 发送 KVO 通知。在下面 Responding to the Cancel Command 中更详细地描述了操作中取消通知的处理。

#### Responding to the Cancel Command（响应取消命令）
&emsp;一旦将操作添加到队列中，该操作就不在你的控制范围之内了。队列接管并处理该任务的调度。但是，如果你后来决定不想执行操作，因为用户按下了进度面板中的取消按钮或退出了应用程序，例如，你可以取消该操作以防止不必要地占用 CPU 时间。你可以通过调用操作对象本身的 cancel 方法或调用 NSOperationQueue 类的 cancelAllOperations 方法来实现。

&emsp;取消一个操作并不会立即迫使它停止正在做的事情。尽管所有操作都需要考虑 cancelled 属性中的值，但代码必须显式检查此属性中的值，并根据需要中止。NSOperation 的（start）默认实现包括取消检查。例如，如果在调用某个操作的 start 方法之前取消该操作，则 start 方法将退出而不启动任务。

> &emsp;Note: 在 macOS 10.6 及更高版本中，如果对位于操作队列中且具有未完成的依赖操作的操作调用 cancel 方法，则这些依赖操作随后将被忽略。因为操作已经取消，所以此行为允许队列调用操作的 start 方法以从队列中删除操作，而不调用其 main 方法。如果对不在队列中的操作调用 cancel 方法，则该操作将立即标记为已取消。在每种情况下，将操作标记为就绪或完成都会生成相应的 KVO 通知。

&emsp;在编写的任何自定义代码中，都应该始终支持取消语义。特别是，主任务代码应该定期检查 cancelled 属性的值。如果属性报告值 YES，你的操作对象应该尽快清理并退出。如果你实现了一个自定义的 start 方法，那么该方法应该包括取消的早期检查，并表现出适当的行为。你的自定义 start 方法必须准备好处理此类提前取消。

&emsp;除了在操作取消时简单地退出外，将已取消的操作移动到适当的最终状态也很重要。具体来说，如果你自己管理 finished 和 executing 属性的值（可能是因为你正在实现一个并发操作），则必须相应地更新这些属性。具体来说，必须将 finished 返回的值更改为 YES，将 execution 返回的值更改为 NO。即使操作在开始执行之前被取消，也必须进行这些更改。
### init
&emsp;返回一个初始化的 NSOperation 对象。
```c++
- (id)init
```
&emsp;你的自定义子类必须调用此方法。默认实现会初始化对象的实例变量并准备使用。该方法在当前线程（即用于分配操作对象的线程）上运行。
### start
&emsp;开始执行操作。
```c++
- (void)start;
```
&emsp;此方法的默认实现更新操作的执行状态并调用 receiver 的 main 方法。此方法还执行多个检查以确保操作可以实际运行。例如，如果 receiver 已取消或已完成，则此方法只返回而不调用 main。（在 OS X v10.5 中，如果操作已完成，此方法将引发异常。）如果操作当前正在执行或尚未准备好执行，则此方法将引发 NSInvalidArgumentException 异常。在 OS X v10.5 中，此方法自动捕获并忽略 main 方法引发的任何异常。在 macOS 10.6 及更高版本中，允许异常在该方法之外传播。你不应该允许异常从 main 方法传播出去。

> &emsp;Note: 如果某个操作仍然依赖于尚未完成的其他操作，则该操作不被视为准备就绪。

&emsp;如果要实现并发操作，则必须重写此方法并使用它来启动操作。你的自定义实现在任何时候都不能调用 super。除了为任务配置执行环境外，此方法的实现还必须跟踪操作的状态并提供适当的状态转换。当操作执行并随后完成其工作时，它应该分别为 isExecuting 和 isFinished key path 生成 KVO 通知。有关手动生成 KVO 通知的更多信息，请参阅 Key-Value Observing Programming Guide。

&emsp;如果要手动执行操作，可以显式调用此方法。但是，对已在操作队列中的操作对象调用此方法或在调用此方法后对操作进行排队是程序员的错误。一旦将操作对象添加到队列中，队列将承担该操作对象的所有责任。
### main
&emsp;执行 receiver 的非并行任务。
```c++
- (void)main;
```
&emsp;此方法的默认实现不执行任何操作。你应该重写此方法以执行所需的任务。在你的实现中，不要调用 super。此方法将在 NSOperation 提供的自动释放池中自动执行，因此不需要在实现中创建自己的自动释放池块。

&emsp;如果要实现并发操作，则不需要重写此方法，但是如果打算从自定义 start 方法调用它，则可以这样做。
### completionBlock
&emsp;该操作的主任务完成后要执行的块。
```c++
@property (nullable, copy) void (^completionBlock)(void) API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
```
&emsp;completionBlock 不带参数，也没有返回值。

&emsp;不能保证完成块的确切执行上下文，但通常是子线程。因此，你不应该使用此块来执行任何需要非常特定的执行上下文的工作。相反，你应该将该工作分流到应用程序的主线程或能够执行该工作的特定线程。例如，如果你有一个用于协调操作完成的自定义线程，则可以使用完成块 ping 该线程。

&emsp;当 finished 属性中的值更改为 YES 时，将执行你提供的 completionBlock。因为 completionBlock 在操作指示它已完成其任务之后执行，所以不能使用 completionBlock 将被视为该任务一部分的其他工作排队。一个操作对象的 finished 属性包含值 YES，根据定义，它必须完成所有与任务相关的工作。应该使用 completionBlock 来通知感兴趣的对象工作已经完成，或者执行其他可能与操作的实际任务相关但不是其一部分的任务。

&emsp;已完成的操作可能会因为被取消或成功完成其任务而完成。在编写块代码时，应该考虑到这一点。同样地，你不应该对成功完成相关操作做出任何假设，这些操作本身可能已被取消。

&emsp;在 iOS 8 及更高版本和 macOS 10.10 及更高版本中，completionBlock 开始执行后，此属性设置为 nil。
### cancel
&emsp;建议（advises）操作对象停止执行其任务。
```c++
- (void)cancel;
```
&emsp;此方法不会强制停止操作代码。相反，它会更新对象的内部标志以反映状态的变化。如果操作已完成执行，则此方法无效。取消当前在操作队列中但尚未执行的操作，可以比平常更早地从队列中删除该操作。

&emsp;在 macOS 10.6 及更高版本中，如果一个操作在队列中，但等待的是未完成的依赖操作，则这些操作随后将被忽略。因为它已经被取消，此行为允许操作队列更快地调用操作的 start 方法，并将对象从队列中清除。如果取消不在队列中的操作，此方法会立即将对象标记为已完成。在每种情况下，将对象标记为就绪或完成都会生成相应的 KVO 通知。

&emsp;在 10.6 之前的 macOS 版本中，操作对象保留在队列中，直到通过正常进程删除其所有依赖项。因此，操作必须等到其所有依赖操作完成执行，或者它们本身被取消，并调用它们的 start 方法。

### cancelled
&emsp;指示操作是否已取消的布尔值。
```c++
@property (readonly, getter=isCancelled) BOOL cancelled;
```
&emsp;此属性的默认值为 NO。调用此对象的 cancel 方法会将此属性的值设置为 YES。取消后，操作必须移至完成状态。

&emsp;取消操作不会主动阻止执行 receiver 的代码。操作对象负责定期调用此方法，并在该方法返回 YES 时自行停止。

&emsp;在完成操作任务之前，应该始终检查此属性的值，这通常意味着在自定义 main 方法的开头检查它。一个操作在开始执行之前或在执行过程中的任何时候都有可能被取消。因此，检查 main 方法开头的值（并在该方法中定期检查）可以在取消操作时尽快退出。
### executing
&emsp;一个布尔值，指示操作当前是否正在执行。
```c++
@property (readonly, getter=isExecuting) BOOL executing;
```
&emsp;如果操作当前正在执行其主任务，则此属性的值为 YES；否则，则为 NO。

&emsp;实现并发操作对象时，必须重写此属性的实现，以便返回操作的执行状态。在自定义实现中，每当操作对象的执行状态更改时，必须为 isExecuting key path 生成 KVO 通知。有关手动生成 KVO 通知的更多信息，请参阅 Key-Value Observing Programming Guide。

&emsp;对于非并发操作，不需要重新实现此属性。
### finished
&emsp;一个布尔值，指示操作是否已完成执行其任务。
```c++
@property (readonly, getter=isFinished) BOOL finished;
```
&emsp;如果操作已完成其主任务，则此属性的值为 YES；如果操作正在执行该任务或尚未启动该任务，则此属性的值为 NO。

&emsp;实现并发操作对象时，必须重写此属性的实现，以便返回操作的完成状态。在自定义实现中，每当操作对象的完成状态更改时，必须为 isFinished key path 生成 KVO 通知。有关手动生成 KVO 通知的更多信息，请参阅 Key-Value Observing Programming Guide。

&emsp;对于非并发操作，不需要重新实现此属性。
### concurrent
&emsp;一个布尔值，指示操作是否异步执行其任务。
```c++
@property (readonly, getter=isConcurrent) BOOL concurrent; // To be deprecated; use and override 'asynchronous' below
```
&emsp;请改用 asynchronous 属性。

&emsp;对于相对于当前线程异步运行的操作，此属性的值为 YES；对于在当前线程上同步运行的操作，此属性的值为 NO。此属性的默认值为 NO。

&emsp;在 macOS 10.6 及更高版本中，操作队列会忽略此属性中的值，并始终在单独的线程上启动操作。
### asynchronous
&emsp;一个布尔值，指示操作是否异步执行其任务。
```c++
@property (readonly, getter=isAsynchronous) BOOL asynchronous API_AVAILABLE(macos(10.8), ios(7.0), watchos(2.0), tvos(9.0));
```
&emsp;对于相对于当前线程异步运行的操作，此属性的值为 YES；对于在当前线程上同步运行的操作，此属性的值为 NO。此属性的默认值为 NO。

&emsp;实现异步操作对象时，必须实现此属性并返回 YES。
### ready
&emsp;一个布尔值，指示是否可以立即执行该操作。
```c++
@property (readonly, getter=isReady) BOOL ready;
```
&emsp;操作的就绪状态取决于它们对其他操作的依赖性，还可能取决于你定义的自定义条件。 NSOperation 类管理对其他操作的依赖关系，并基于这些依赖关系报告 receiver 的准备情况。

&emsp;如果要使用自定义条件来定义操作对象的就绪状态，请重新实现此属性并返回一个准确反映接收方就绪状态的值。如果这样做，你的自定义实现必须从super 获取默认属性值，并将该就绪值合并到该属性的新值中。在自定义实现中，每当操作对象的就绪状态发生更改时，都必须为 isReady key path 生成 KVO 通知。
### name
&emsp;操作的名称。
```c++
@property (nullable, copy) NSString *name API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0));
```
&emsp;为操作对象分配一个名称，以帮助在调试期间识别它。
### addDependency:
&emsp;使 receiver 依赖于指定操作的完成。
```c++
- (void)addDependency:(NSOperation *)op;
```
&emsp;`operation`: receiver 应依赖的操作。相同的依赖项不应多次添加到 receiver，并且这样做的结果是不确定的。

&emsp;直到 receiver 的所有相关操作完成执行后，才认为 receiver 准备就绪。如果 receiver 已经在执行其任务，则添加依赖项没有实际效果。此方法可能会更改 receiver 的 isReady 和 dependencies 属性。

&emsp;在一组操作之间创建任何循环依赖关系是程序员的错误。这样做可能导致操作之间出现死锁，并可能 freeze 程序。
### removeDependency:
&emsp;移除了 receiver 对指定操作的依赖。
```c++
- (void)removeDependency:(NSOperation *)op;
```
&emsp;`operation`: 从 receiver 中移除相关操作。

&emsp;此方法可能会更改 receiver 的 isReady 和 dependencies 属性。
### dependencies
&emsp;在当前对象开始执行之前必须完成执行的操作对象数组。
```c++
@property (readonly, copy) NSArray<NSOperation *> *dependencies;
```
&emsp;此属性包含 NSOperation 对象的数组。要将对象添加到此数组，请使用 `addDependency:` 方法。

&emsp;操作对象必须在其所有依赖操作完成执行后才能执行。操作在完成执行时不会从此依赖项列表中删除。可以使用此列表跟踪所有相关操作，包括已完成执行的操作。从列表中删除操作的唯一方法是使用 `removeDependency:` 方法。
### qualityOfService
&emsp;将系统资源分配给该操作的相对重要性。（执行操作时的线程优先级）
```c++
typedef NS_ENUM(NSInteger, NSQualityOfService) {
    NSQualityOfServiceUserInteractive = 0x21,
    NSQualityOfServiceUserInitiated = 0x19,
    NSQualityOfServiceUtility = 0x11,
    NSQualityOfServiceBackground = 0x09,
    NSQualityOfServiceDefault = -1
} API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0));

@property NSQualityOfService qualityOfService API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0));
```
&emsp;service levels 影响操作对象访问系统资源（如 CPU 时间、网络资源、磁盘资源等）的优先级。具有更高 quality of service 级别的操作比系统资源具有更高的优先级，因此它们可以更快地执行任务。你使用 service levels 来确保响应显式用户请求的操作优先于不太关键的工作。

&emsp;此属性反映了有效执行操作所需的最低 service levels。此属性的默认值是 NSQualityOfServiceBackground，你应该尽可能保留该值。更改 service levels 时，请使用适用于执行相应任务的最低级别。例如，如果用户启动任务并等待任务完成，请将值 NSQualityOfServiceUserInteractive 分配给此属性。如果资源可用，系统可以为操作提供更高的 service levels。see Prioritize Work with Quality of Service Classes in Energy Efficiency Guide for iOS Apps and Prioritize Work at the Task Level in Energy Efficiency Guide for Mac Apps.
### queuePriority
&emsp;操作队列中操作的执行优先级。
```c++
// 这些常量使你可以区分操作的执行顺序。
// 你可以使用这些常量来指定操作队列中等待开始的操作的相对顺序。你应该始终使用这些常数（而不是定义的值）来确定优先级。
typedef NS_ENUM(NSInteger, NSOperationQueuePriority) {
    NSOperationQueuePriorityVeryLow = -8L,
    NSOperationQueuePriorityLow = -4L,
    NSOperationQueuePriorityNormal = 0,
    NSOperationQueuePriorityHigh = 4,
    NSOperationQueuePriorityVeryHigh = 8
};

@property NSOperationQueuePriority queuePriority;
```
&emsp;此属性包含操作的相对优先级。此值用于影响操作出列和执行的顺序。返回值总是对应于预定义的常量之一。（有关有效值的列表，请参阅 NSOperationQueuePriority。）如果未显式设置优先级，则此方法返回 NSOperationQueuePriorityNormal。

&emsp;你仅应根据需要使用优先级值来对非依赖操作的相对优先级进行分类。优先级值不应用于实现不同操作对象之间的依赖关系管理。如果需要在操作之间建立依赖关系，请改用 `addDependency:` 方法。

&emsp;如果尝试指定的优先级值与定义的常量之一不匹配（-8L、-4L、0、4、8），则操作对象会自动调整你指定的值，使其朝向 NSOperationQueuePriorityNormal 优先级，并在第一个有效常量值处停止。例如，如果指定值 -10，则操作将调整该值以匹配 NSOperationQueuePriorityVeryLow 常量。类似地，如果指定 +10，此操作将调整该值以匹配 NSOperationQueuePriorityVeryHigh 常量。
### waitUntilFinished
&emsp;阻止当前线程的执行，直到操作对象完成其任务。
```c++
- (void)waitUntilFinished API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
```
&emsp;操作对象决不能对自身调用此方法，并且应避免对提交到与自身相同的操作队列的任何操作调用此方法。这样做会导致操作死锁。相反，应用程序的其他部分可能会根据需要调用此方法，以阻止其他任务在目标操作对象完成之前完成。在不同操作队列中的操作上调用此方法通常是安全的，尽管如果每个操作都在另一个操作上等待，仍然可能创建死锁。

&emsp;此方法的典型用法是从最初创建操作的代码中调用它。将操作提交到队列后，你将调用此方法以等待该操作完成执行。

&emsp;NSOperation 相关的内容就这么多，下面看一下它的两个子类：NSBlockOperation 和 NSInvocationOperation。
## NSBlockOperation
&emsp;一种管理一个或多个 blocks 的并发执行的操作。
```c++
API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0))
@interface NSBlockOperation : NSOperation
```
&emsp;NSBlockOperation 类是 NSOperation 的一个具体子类，它管理一个或多个 blocks 的并发执行。可以使用此对象一次执行多个 blocks，而不必为每个 block 创建单独的操作对象。当执行多个 blocks 时，只有当所有 blocks 都已完成执行时，操作本身才被视为已完成。

&emsp;添加到操作的 block 以默认优先级调度到适当的工作队列。block 本身不应该对其执行环境的配置进行任何假设。
### blockOperationWithBlock:
&emsp;创建并返回一个 NSBlockOperation 对象，并将指定的 block 添加到该对象。
```c++
+ (instancetype)blockOperationWithBlock:(void (^)(void))block;
```
&emsp;`block`: 要添加到新 block 操作对象列表中的 block。该 block 应该没有参数，也没有返回值。
### addExecutionBlock:
&emsp;将指定的 block 添加到 receiver 要执行的 blocks 列表中。
```c++
- (void)addExecutionBlock:(void (^)(void))block;
```
&emsp;`block`: 要添加到 receiver 列表中的 block。该 block 应该没有参数，也没有返回值。

&emsp;指定的 block 不应对其执行环境做任何假设。在 receiver 正在执行或已完成时调用此方法会引发 NSInvalidArgumentException 异常。
### executionBlocks
&emsp;与 receiver 关联的 block。
```c++
@property (readonly, copy) NSArray<void (^)(void)> *executionBlocks;
```
&emsp;该数组中的 block 是最初使用 `addExecutionBlock:` 方法添加的 block 的副本。
## NSInvocationOperation
&emsp;一种操作，用于管理指定为 invocation 的单个封装任务的执行。
```c++
API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0))
NS_SWIFT_UNAVAILABLE("NSInvocation and related APIs not available")
@interface NSInvocationOperation : NSOperation
```
&emsp;NSInvocationOperation 类是 NSOperation 的具体子类，你可以使用它来发起一个操作，该操作包括在指定对象上调用 selector。此类实现非并发操作。
### initWithTarget:selector:object:
&emsp;返回使用指定的 target 和 selector 初始化的 NSInvocationOperation 对象。
```c++
- (nullable instancetype)initWithTarget:(id)target selector:(SEL)sel object:(nullable id)arg;
```
&emsp;`target`: 定义指定选择器的对象。`sel`: 运行操作时要调用的选择器。选择器可以采用 0 或 1 个参数。如果它接受一个参数，则该参数的类型必须为 id。该方法的返回类型可以是 void，标量值或可以作为id类型返回的对象。`arg`: 要传递给 `sel` 的参数对象。如果 `sel` 不带参数，则指定 nil。

&emsp;Return Value: 初始化的 NSInvocationOperation 对象；如果 `target` 对象未实现指定的 selector（`sel`），则为 nil。

&emsp;如果使用非空（non-void）返回类型指定 selector，则可以在操作完成执行后通过调用 result 方法来获取返回值。receiver 告诉 invocation 对象保留其参数。
### initWithInvocation:
&emsp;返回使用指定的 `invocation` 对象初始化的 NSInvocationOperation 对象。
```c++
- (instancetype)initWithInvocation:(NSInvocation *)inv NS_DESIGNATED_INITIALIZER;
```
&emsp;`inv`: 调用对象，用于标识 target 对象，selector 和参数对象。

&emsp;Return Value: 初始化的 NSInvocationOperation 对象；如果无法初始化，则返回 nil。

&emsp;此方法是指定的初始化程序。receiver 告诉 NSInvocation 对象保留其参数（NSInvocation 类的 retainArguments 函数）。
### invocation
&emsp;receiver 的调用对象（NSInvocation）。
```c++
@property (readonly, retain) NSInvocation *invocation;
```
&emsp;NSInvocation 对象，用于标识 target 对象，selector 和用于执行操作任务的参数。
### result
&emsp;invocation 或方法的结果。
```c++
@property (nullable, readonly, retain) id result;
```
&emsp;方法返回的对象或包含非返回值的 NSValue 对象（如果不是对象）。如果方法或调用尚未完成执行，则为 null。

&emsp;如果在方法或调用的执行期间引发了异常，则访问此属性将再次引发该异常。如果操作被取消，或者调用或方法的返回类型为 void，则访问此属性将引发异常；否则，返回 false。请参阅 Result Exceptions。
### Result Exceptions
&emsp;如果调用 result 方法时发生错误，则由 NSInvocationOperation 引发的异常的名称。
```c++
FOUNDATION_EXPORT NSExceptionName const NSInvocationOperationVoidResultException API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
FOUNDATION_EXPORT NSExceptionName const NSInvocationOperationCancelledException API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
```
&emsp;NSInvocationOperationVoidResultException 如果为具有无效返回类型的调用方法调用 result 方法，则引发异常的名称。

&emsp;NSInvocationOperationCancelledException 如果取消操作后调用 result 方法，则会引发异常的名称。

&emsp;NSBlockOperation 和 NSInvocationOperation 就这么多内容，下面看一下比较重要的操作队列 NSOperationQueue 类。 
## NSOperationQueue（操作队列）
&emsp;规范操作（NSOperation）执行的队列。
```c++
API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0))
@interface NSOperationQueue : NSObject <NSProgressReporting>
```
&emsp;操作队列根据它们的优先级和准备状态执行其排队的 NSOperation 对象。在将操作添加到操作队列之后，该操作将保留在其队列中，直到它报告完成任务为止。添加操作后，你无法直接从队列中删除该操作。

> &emsp;Note: 操作队列将保留操作直到操作完成，而队列本身将保留直到所有操作完成。用未完成的操作暂停操作队列可能会导致内存泄漏。

### Determining Execution Order（确定执行顺序）
&emsp;队列中的操作根据它们的就绪性、优先级和互操作依赖性进行组织，并相应地执行。如果所有排队的操作都具有相同的 queuePriority，并且当它们放入队列时准备好执行（即，它们的 ready 属性返回 YES），则它们将按照提交到队列的顺序执行。否则，操作队列总是执行相对于其他就绪操作具有最高优先级的操作。

&emsp;但是，你永远不应该依赖队列语义来确保操作的特定执行顺序，因为操作准备状态的更改可能会更改生成的执行顺序。互操作依赖项为操作提供绝对的执行顺序，即使这些操作位于不同的操作队列中。在操作对象的所有依赖操作完成执行之前，操作对象不会被认为已准备好执行。
### Canceling Operations（取消操作）
&emsp;完成它的任务并不一定意味着操作完成了该任务；操作也可以被取消。取消操作对象会将对象留在队列中，但会通知对象应尽快停止其任务。对于当前正在执行的操作，这意味着操作对象的工作代码必须检查取消状态，停止正在执行的操作，并将自身标记为已完成。对于已排队但尚未执行的操作，队列仍必须调用操作对象的 start 方法，以便它可以处理取消事件并将自身标记为已完成。

> &emsp;Note: 取消操作会导致该操作忽略其可能具有的任何依赖关系。此行为使队列可以尽快执行操作的 start 方法。依次使用 start 方法将操作移至完成状态，以便可以将其从队列中删除。
### KVO-Compliant Properties（符合 KVO 的属性）
&emsp;NSOperationQueue 类符合键值编码（KVC）和键值观察（KVO）。你可以根据需要观察这些属性，以控制应用程序的其他部分。要观察属性，请使用以下关键路径：

+ operations - read-only
+ operationCount - read-only // ⬆️⬆️ 只读

+ maxConcurrentOperationCount - readable and writable // ⬇️⬇️ 可读可写
+ suspended - readable and writable
+ name - readable and writable

&emsp;尽管可以将观察者附加到这些属性，但不应使用 Cocoa binding 将它们绑定到应用程序用户界面的元素。与用户界面关联的代码通常只能在应用程序的主线程中执行。但是，与操作队列关联的 KVO 通知可能发生在任何线程中。
### Thread Safety
&emsp;使用多个线程中的单个 NSOperationQueue 对象是安全的，而无需创建其他锁来同步对该对象的访问。

&emsp;操作队列使用 Dispatch 框架来启动其操作的执行。结果，无论操作被指定为同步还是异步，操作始终在单独的线程上执行。
### mainQueue
&emsp;返回与主线程关联的操作队列。
```c++
@property (class, readonly, strong) NSOperationQueue *mainQueue API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
```
&emsp;Return Value: 绑定到主线程的默认操作队列。

&emsp;返回的队列在应用程序的主线程上一次执行一个操作。在主线程上执行的操作与必须在主线程上执行的其他任务交织在一起，例如事件服务和应用程序用户界面的更新。队列在运行循环公共模式（run loop common modes）中执行这些操作，如 NSRunLoopCommonModes 常量所示。队列的 underlyingQueue 属性的值是主线程的调度队列；不能将此属性设置为其他值。
### currentQueue
&emsp;返回启动当前操作的操作队列。
```c++
@property (class, readonly, strong, nullable) NSOperationQueue *currentQueue API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
```
&emsp;Return Value: 启动操作的操作队列，如果无法确定该队列，则为 nil。

&emsp;你可以从正在运行的操作对象中使用此方法，以获取对启动它的操作队列的引用。从正在运行的操作的上下文外部调用此方法通常会导致返回 nil。
### addOperation:
&emsp;将指定的操作（NSOperation）添加到 receiver。
```c++
- (void)addOperation:(NSOperation *)op;
```
&emsp;`op`: 要添加到队列的操作。

&emsp;添加后，指定的操作将保留在队列中，直到完成执行为止。

> &emsp;Important: 一个操作对象一次最多只能在一个操作队列中，如果该操作已经在另一个队列中，则此方法将引发 NSInvalidArgumentException 异常。类似地，如果操作当前正在执行或已完成执行，则此方法会引发 NSInvalidArgumentException 异常。

### addOperations:waitUntilFinished:
&emsp;将指定的操作添加到队列中。
```c++
- (void)addOperations:(NSArray<NSOperation *> *)ops waitUntilFinished:(BOOL)wait API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
```
&emsp;`ops`: 要添加到队列的操作。`wait`: 如果 YES，则阻塞当前线程，直到所有指定的操作完成执行。如果为 NO，则将操作添加到队列中，并且 control 立即返回到调用方。

&emsp;一个操作对象一次最多只能在一个操作队列中，如果当前正在执行或已完成，则不能添加该操作对象。如果对于 `ops` 参数中的任何操作，这些错误条件中的任何一个为 true，则此方法将引发 NSInvalidArgumentException 异常。

&emsp;添加后，指定的操作将保留在队列中，直到其 finished 方法返回 YES。
### addOperationWithBlock:
&emsp;在操作中包装指定的 block，并将其添加到 receiver。
```c++
- (void)addOperationWithBlock:(void (^)(void))block API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
```
&emsp;`block`: 从操作执行的 block。该 block 不带参数，没有返回值。

&emsp;此方法首先将单个 block 包装到操作对象中，从而向 receiver 添加单个 block。不应尝试获取对新创建的操作对象的引用或确定其类型信息。
### cancelAllOperations
&emsp;取消所有排队和正在执行的操作。
```c++
- (void)cancelAllOperations;
```
&emsp;此方法对队列中当前进行的所有操作调用 cancel 方法。

&emsp;取消操作不会自动将其从队列中删除，也不会停止当前正在执行的操作。对于排队等待执行的操作，队列在识别出已取消并将其移至完成状态之前，仍必须尝试执行该操作。对于已经执行的操作，操作对象本身必须检查取消并停止正在执行的操作，以便它可以移至完成状态。在这两种情况下，完成的（或取消的）操作仍有机会在从队列中删除之前执行其完成块。
### waitUntilAllOperationsAreFinished
&emsp;阻塞当前线程，直到所有 receiver 都已排队并且正在执行的操作完成为止。
```c++
- (void)waitUntilAllOperationsAreFinished;
```
&emsp;调用该方法时，该方法将阻止当前线程，并等待 receiver 的当前操作和排队的操作完成执行。当前线程被阻塞时，receiver 继续启动已排队的操作并监视正在执行的操作。在这段时间内，当前线程无法向队列添加操作，但是其他线程可以。一旦所有挂起的操作完成，此方法将返回。

&emsp;如果队列中没有任何操作，则此方法立即返回。
### qualityOfService
&emsp;应用于使用队列执行的操作的默认 service level。
```c++
@property NSQualityOfService qualityOfService API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0));
```
&emsp;此属性指定应用于添加到队列的操作对象的 service level。如果操作对象设置了显式 service level，则使用该值。此属性的默认值取决于创建队列的方式。对于你自己创建的队列，默认值为 NSOperationQualityOfServiceBackground。对于 mainQueue 方法返回的队列，默认值为 NSOperationQualityOfServiceUserInteractive，不能更改。

&emsp;service levels 影响操作对象访问系统资源（如 CPU 时间、网络资源、磁盘资源等）的优先级。具有更高 quality of service level 的操作比系统资源具有更高的优先级，因此它们可以更快地执行任务。你使用 service levels 来确保响应显式用户请求的操作优先于不太关键的工作。
### maxConcurrentOperationCount
&emsp;可以同时执行的最大排队操作数。
```c++
@property NSInteger maxConcurrentOperationCount;
```
&emsp;此属性中的值仅影响当前队列同时执行的操作。其他操作队列也可以并行执行其最大数量的操作。

&emsp;减少并发操作的数量不会影响当前正在执行的任何操作。指定值 NSOperationQueueDefaultMaxConcurrentOperationCount（建议使用此值）会使系统根据系统条件设置最大操作数。

&emsp;此属性的默认值为 NSOperationQueueDefaultMaxConcurrentOperationCount。你可以使用键值观察来监视对此属性值的更改。配置观察者以监视操作队列的 maxConcurrentOperationCount key path。
### NSOperationQueueDefaultMaxConcurrentOperationCount
&emsp;队列中要同时执行的默认最大操作数。
```c++
static const NSInteger NSOperationQueueDefaultMaxConcurrentOperationCount = -1;
```
&emsp;该数字是根据当前系统条件动态确定的。
### suspended
&emsp;一个布尔值，指示队列是否正在积极调度要执行的操作。
```c++
@property (getter=isSuspended) BOOL suspended;
```
&emsp;当此属性的值为 NO 时，队列将主动启动队列中准备执行的操作。将此属性设置为 YES 可防止队列启动任何排队操作，但已执行的操作将继续执行。你可以继续将操作添加到挂起的队列中，但在将此属性更改为 NO 之前，不会计划执行这些操作。

&emsp;仅当操作完成执行时，才将其从队列中删除。但是，为了完成执行，必须首先开始操作。因为挂起的队列不会启动任何新操作，所以它不会删除当前正在排队且未执行的任何操作（包括取消的操作）。

&emsp;你可以使用键值观察来监视对此属性值的更改。配置观察者以监视操作队列的 suspended key path。

&emsp;此属性的默认值为 NO。
### name
&emsp;操作队列的名称。
```c++
@property (nullable, copy) NSString *name API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
```
&emsp;name 为你提供了一种在运行时标识操作队列的方法。工具还可以使用该名称在调试或分析代码期间提供其他上下文。

&emsp;此属性的默认值是一个字符串，其中包含操作队列的内存地址。你可以使用键值观察来监视对此属性值的更改。配置观察者以监视操作队列的 name key path。
### underlyingQueue
&emsp;用于执行操作的调度队列。
```c++
@property (nullable, assign /* actually retain */) dispatch_queue_t underlyingQueue API_AVAILABLE(macos(10.10), ios(8.0), watchos(2.0), tvos(9.0));
```
&emsp;此属性的默认值为 nil。你可以将此属性的值设置为现有的调度队列，以使排队的操作散布在提交给该调度队列的 blocks 中。

&emsp;仅当队列中没有操作时才应设置此属性的值；当 operationCount 不等于 0 时设置此属性的值将引发 NSInvalidArgumentException。此属性的值不能是 dispatch_get_main_queue 返回的值。为基础调度队列设置的 quality-of-service 级别将覆盖为操作队列的 qualityOfService 属性设置的任何值。

> &emsp;Note: 如果 OS_OBJECT_IS_OBJC 为 YES，则此属性自动保留其分配的队列。
### progress
&emsp;表示队列中执行的操作的总进度。
```c++
@property (readonly, strong) NSProgress *progress API_AVAILABLE(macos(10.15), ios(13.0), tvos(13.0), watchos(6.0));
```
&emsp;`progress` 属性表示队列中执行的操作的总进度。默认情况下，在设置进度的 `totalUnitCount` 之前，NSOperationQueue 不会报告进度。设置进度的 `totalUnitCount` 属性后，队列将选择参与进度报告。启用后，对于在 `main` 结束之前完成的操作，每个操作将为队列的总体进度贡献 1 个完成单位（重写 `start` 且不调用 `super` 的操作不会对进度有所贡献）。更新进度的 `totalUnitCount` 时，应特别注意比赛条件，并应注意避免“落后进度”。例如：当 NSOperationQueue 的进度为 5/10，表示已完成 50％，并且有 90 多个操作要添加时，`totalUnitCount` 将使进度报告为 5/100，表示 5％。在此示例中，这意味着任何进度条都将从显示 50％ 跳回到 5％，这可能是不希望的。在需要调整 `totalUnitCount` 的情况下，建议通过使用 `addBarrierBlock:` API 来实现此目的，以确保线程安全。这样可以确保不会发生意外的执行状态，从而可以适应潜在的向后移动进度方案。
```c++
NSOperationQueue *queue = [[NSOperationQueue alloc] init];
queue.progress.totalUnitCount = 10;
```
### addBarrierBlock:
&emsp;当 NSOperationQueue 完成所有入队操作时，`addBarrierBlock:` 方法将执行该块，并阻止任何后续操作被执行，直到 barrier block 完成为止。此行为类似于 dispatch_barrier_async 函数。
```c++
- (void)addBarrierBlock:(void (^)(void))barrier API_AVAILABLE(macos(10.15), ios(13.0), tvos(13.0), watchos(6.0));
```
## 参考链接
**参考链接:🔗**
+ [NSOperation](https://developer.apple.com/documentation/foundation/nsoperation?language=occ)
+ [NSBlockOperation](https://developer.apple.com/documentation/foundation/nsblockoperation?language=occ)
+ [NSInvocationOperation](https://developer.apple.com/documentation/foundation/nsinvocationoperation?language=occ)
+ [NSOperationQueue](https://developer.apple.com/documentation/foundation/nsoperationqueue?language=occ)
+ [Operation Queues](https://developer.apple.com/library/archive/documentation/General/Conceptual/ConcurrencyProgrammingGuide/OperationObjects/OperationObjects.html#//apple_ref/doc/uid/TP40008091-CH101-SW1)
+ [iOS 多线程：『NSOperation、NSOperationQueue』详尽总结](https://www.jianshu.com/p/4b1d77054b35)
+ [并发编程：API 及挑战](https://objccn.io/issue-2-1/)
