# iOS《Concurrency Programming Guide-Operation Queues》官方文档

## Operation Queues
&emsp;Cocoa operations 是一种面向对象的方法，用于封装要异步执行的工作。operations 被设计为与 operation queue 一起使用，或者单独使用。因为它们是基于 Objective-C 的，所以 operations 在 OS X 和 iOS 中最常用于 Cocoa-based 的应用程序。

&emsp;本章介绍如何定义和使用 operations。

### About Operation Objects（关于 Operation 对象）
&emsp;operation 对象是 NSOperation 类（在 Foundation framework 中）的实例，用于封装希望应用程序执行的工作。NSOperation 类本身是一个抽象基类，必须对其进行子类化才能执行任何有用的工作。尽管这个类是抽象的，但它确实提供了大量的 infrastructure，以尽量减少你在自己的子类中必须完成的工作量。此外，Foundation framework 提供了两个具体的子类，你可以在现有代码中使用它们。表 2-1 列出了这些类，以及如何使用每个类的摘要。

&emsp;Table 2-1  Operation classes of the Foundation framework

| Class | Description |
| --- | --- |
| NSInvocationOperation | 使用的类是基于应用程序中的对象和选择器创建 operation 对象。如果你具有已经执行所需任务的现有方法，则可以使用此类。因为它不需要子类化，所以还可以使用此类以更动态的方式创建 operation 对象。 有关如何使用此类的信息，请参见下面的 Creating an NSInvocationOperation Object |
| NSBlockOperation | as-is 使用的类用于同时执行一个或多个 block 对象。因为一个 block operation 对象可以执行多个块，所以它使用 group 语义进行操作；只有当所有相关的 blocks 都执行完毕时，operation 本身才被视为已完成。有关如何使用此类的信息，请参见下面的 Creating an NSBlockOperation Object。此类在 OS X v10.6 和更高版本中可用。有关 blocks 的更多信息，请参见 Blocks Programming Topics。 |
| NSOperation | 用于定义自定义 operation 对象的基类。子类化 NSOperation 使你能够完全控制自己操作的实现，包括更改操作执行和报告其状态的默认方式。有关如何定义自定义 operation 对象的信息，请参见 Defining a Custom Operation Object。 |

&emsp;所有 operation 对象都支持以下关键功能：

+ 支持在 operation 对象之间建立 graph-based 的依赖关系。这些依赖关系阻止给定的 operation 在其所依赖的所有 operations 完成运行之前运行。有关如何配置依赖项的信息，请参见 Configuring Interoperation Dependencies。
+ 支持可选的完成 block，该 block 在 operation 的主任务完成后执行。（仅限 OS X v10.6 及更高版本）有关如何设置完成 block 的信息，请参见 Setting Up a Completion Block。
+ 支持使用 KVO 通知监视对 operations 执行状态的更改。有关如何观察 KVO 通知的信息，请参见 Key-Value Observing Programming Guide。
+ 支持对 operations 进行优先级排序，从而影响它们的相对执行顺序。有关详细信息，请参见 Changing an Operation’s Execution Priority。
+ 支持取消语义，允许你在 operation 执行时停止操作。有关如何取消 operations 的信息，请参见 Canceling Operations。有关如何在自己的 operations 中支持取消的信息，请参见 Responding to Cancellation Events。

&emsp;Operations 旨在帮助你提高应用程序中的并发等级。Operations 也是将应用程序的行为组织和封装为简单的离散块的好方法。你可以将一个或多个 operation 对象提交到队列，并让相应的工作在一个或多个单独的线程上异步执行，而不是在应用程序的主线程上运行一些代码。

### Concurrent Versus Non-concurrent Operations（并行与非并行操作）
&emsp;尽管通常通过将 operations 添加到 operation queue 来执行操作，但不需要这样做。也可以通过调用 operation 对象的 start 方法手动执行 operation 对象，但这样做并不保证 operation 与其余代码同时运行。NSOperation 类的 isConcurrent 方法告诉你操作相对于调用其 start 方法的线程是同步运行还是异步运行。默认情况下，此方法返回 NO，这意味着 operation 在调用线程中同步运行。

&emsp;如果要实现一个并发 operation，也就是相对于调用线程异步运行的 operations，则必须编写额外的代码以异步启动该 operation。例如，你可以调度一个单独的线程、调用一个异步系统函数或执行任何其他 operations，以确保 start 方法启动任务并立即返回，而且很可能在任务完成之前返回。

&emsp;大多数开发人员不应该需要实现并发 operation 对象。如果总是将 operations 添加到 operation queue 中，则不需要实现并发 operations。将非并发 operation 提交到 operation queue 时，队列本身会创建一个线程来运行 operation。因此，将非并发 operations 添加到 operation queue 仍然会导致 operation 对象代码的异步执行。只有在需要异步执行 operations 而不将其添加到 operation queue 的情况下，才需要定义并发操作。

&emsp;有关如何创建并发操作（concurrent operation）的信息，请参见 NSOperation Class Reference 中的 Configuring Operations for Concurrent Execution。

### Creating an NSInvocationOperation Object（创建 NSInvocationOperation 对象）
&emsp;NSInvocationOperation 类是 NSOperation 的一个具体子类，在运行时，它会调用在指定对象上指定的选择器。使用此类可以避免为应用程序中的每个任务定义大量自定义 operation 对象；尤其是在你正在修改现有应用程序并且已经拥有执行必要任务所需的对象和方法的情况下。如果要调用的方法可能会根据情况发生更改，也可以使用它。例如，可以使用调用操作来执行基于用户输入动态选择的选择器。

&emsp;创建 invocation operation 的过程很简单。创建并初始化类的新实例，将要执行的所需对象和选择器传递给初始化方法。清单 2-1 显示了一个自定义类中的两个方法，它们演示了创建过程。taskWithData: 方法创建一个新的 invocation 对象，并为其提供另一个方法的名称，该方法包含任务实现。

&emsp;Listing 2-1  Creating an NSInvocationOperation object
```c++
@implementation MyCustomClass
- (NSOperation*)taskWithData:(id)data {
    NSInvocationOperation* theOp = [[NSInvocationOperation alloc] initWithTarget:self selector:@selector(myTaskMethod:) object:data];
    return theOp;
}
 
// This is the method that does the actual work of the task.
- (void)myTaskMethod:(id)data {
    // Perform the task.
}
@end
```

### Creating an NSBlockOperation Object（创建 NSBlockOperation 对象）
&emsp;NSBlockOperation 类是 NSOperation 的一个具体子类，它充当一个或多个 block 对象的包装器。此类为已经在使用 operation queues 并且不想同时创建 dispatch queues 的应用程序提供了面向对象的包装器。你还可以使用 block operations 来利用操作依赖项、KVO 通知和 dispatch queues 中可能不可用的其他功能。

&emsp;创建 block operation 时，通常在初始化时至少添加一个 block；以后可以根据需要添加更多 blocks。当执行一个 NSBlockOperation 对象时，该对象将其所有的 blocks 提交到默认优先级并发 dispatch queue。然后，对象等待所有 blocks 执行完毕。当最后一个 block 完成执行时，operation 对象将自己标记为已完成。因此，可以使用 block operation 来跟踪一组正在执行的 blocks，就像使用线程连接来合并多个线程的结果一样。区别在于，由于 block operation 本身在单独的线程上运行，所以应用程序的其他线程可以在等待 block operation 完成时继续工作。

&emsp;清单 2-2 展示了如何创建 NSBlockOperation 对象的简单示例。block 本身没有参数，也没有显著的返回结果。

&emsp;Listing 2-2  Creating an NSBlockOperation object
```c++
NSBlockOperation* theOp = [NSBlockOperation blockOperationWithBlock: ^{
   NSLog(@"Beginning operation.\n");
   // Do some work.
}];
```
&emsp;创建 block operation 对象后，可以使用 `addExecutionBlock:` 方法向其添加更多 blocks。如果需要串行执行 blocks，则必须将它们直接提交到所需的 dispatch queue。

### Defining a Custom Operation Object（定义自定义 Operation 对象）
&emsp;如果 block operation 和 invocation operation 对象不能完全满足应用程序的需要，可以直接将 NSOperation 子类化，并添加所需的任何行为。NSOperation 类为所有 operation 对象提供了一个通用的子类化点（general subclassing point）。该类还提供了大量的 infrastructure 来处理依赖项和 KVO 通知所需的大部分工作。但是，有时你可能仍需要补充现有的 infrastructure，以确保你的 operations 正常运行。你需要做的额外工作的数量取决于你是实现非并发 operation 还是并发 operation。

&emsp;定义非并发 operation 比定义并发 operation 简单得多。对于非并发 operation，你所要做的就是执行主任务并适当地响应取消事件；现有的类 infrastructure 为你完成所有其他工作。对于并发操作，必须用自定义代码替换一些现有的 infrastructure。以下部分将向你展示如何实现这两种类型的对象。

#### Performing the Main Task（执行主要任务）
&emsp;每个 operation 对象至少应实现以下方法：

+ 自定义初始化方法（A custom initialization method）
+ `main` 

&emsp;你需要一个自定义初始化方法来将 operation 对象置于已知状态（known state），并需要一个自定义 `main` 方法来执行任务。当然，你可以根据需要实现其他方法，例如：

+ 你计划从 `main` 方法的实现中调用的自定义方法
+ 用于设置数据值和访问 operation 结果的访问器方法（Accessor methods）
+ NSCoding 协议的方法，允许你 archive and unarchive operation 对象

&emsp;清单 2-3 显示了一个自定义 NSOperation 子类的启动模板。（此清单没有显示如何处理取消，但显示了通常使用的方法。有关处理取消的信息，请参见  Responding to Cancellation Events。）此类的初始化方法将单个对象作为数据参数，并在 operation 对象中存储对该对象的引用。在将结果返回给应用程序之前，`main` 方法表面上将对该数据对象起作用。

&emsp;Listing 2-3  Defining a simple operation object
```c++
@interface MyNonConcurrentOperation : NSOperation

@property id (strong) myData;
- (id)initWithData:(id)data;

@end
 
@implementation MyNonConcurrentOperation

- (id)initWithData:(id)data {
   if (self = [super init])
      myData = data;
   return self;
}
 
- (void)main {
   @try {
      // Do some work on myData and report the results.
   }
   @catch(...) {
      // Do not rethrow exceptions.
   }
}

@end
```
&emsp;有关如何实现 NSOperation 子类的详细示例，请参见 [NSOperationSample](https://developer.apple.com/library/archive/samplecode/NSOperationSample/Introduction/Intro.html#//apple_ref/doc/uid/DTS10004184)。

#### Responding to Cancellation Events（响应取消事件）
&emsp;在一个 operation 开始执行之后，它继续执行它的任务，直到它完成或者直到你的代码显式取消该 operation 为止。取消可以在任何时候发生，甚至在 operation 开始执行之前。尽管 NSOperation 类为 clients 提供了取消 operation 的方法，但根据需要，识别取消事件是自愿的（recognizing the cancellation event is voluntary by necessity）。如果一个 operation 被完全终止，可能就没有办法回收已分配的资源。因此，operation 对象需要检查取消事件，并在它们发生在操作过程中时正常退出。

&emsp;要在 operation 对象中支持取消，你所要做的就是定期从自定义代码中调用对象的 `isCancelled` 方法，如果返回 YES，则立即返回。无论操作的持续时间如何，或者你是直接将 NSOperation 子类化还是使用其具体子类之一，支持取消操作都很重要。`isCancelled` 方法本身是非常轻量级的，可以频繁调用，而不会造成任何明显的性能损失。在设计 operation 对象时，应考虑在代码中的以下位置调用 `isCancelled` 方法：

+ 在执行任何实际工作之前
+ 在循环的每次迭代中至少一次，如果每次迭代相对较长，则更频繁
+ 在代码中相对容易中止操作的任何地方

&emsp;清单 2-4 提供了一个非常简单的示例，说明如何在 operation 对象的 `main` 方法中响应取消事件。在这种情况下，每次通过 while 循环调用 `isCancelled` 方法，允许在工作开始之前快速退出，并定期再次调用。

&emsp;Listing 2-4  Responding to a cancellation request
```c++
- (void)main {
   @try {
      BOOL isDone = NO;
 
      while (![self isCancelled] && !isDone) {
          // Do some work and set isDone to YES when finished
      }
   }
   @catch(...) {
      // Do not rethrow exceptions.
   }
}
```
&emsp;尽管前面的示例不包含清理代码，但是你自己的代码应该确保释放自定义代码分配的所有资源。

#### Configuring Operations for Concurrent Execution（为并发执行配置 Operations）
&emsp;默认情况下，Operation 对象以同步方式执行，也就是说，它们在调用其 `start` 方法的线程中执行任务。但是，由于 operation queues 为非并发 operations 提供线程，因此大多数 operations 仍然异步运行。但是，如果你计划手动执行 operations，但仍希望它们异步运行，则必须采取适当的操作以确保它们能够异步运行。你可以通过将 operations 对象定义为并发 operations 来实现这一点。

&emsp;表 2-2 列出了为实现并发 operation 而通常重写的方法。

&emsp;Table 2-2  Methods to override for concurrent operations

| Method | Description |
| --- | --- |
| start | （Required）所有并发 operations 都必须重写此方法，并用自己的自定义实现替换默认实现。要手动执行操作，可以调用其 start 方法。因此，此方法的实现是操作的起点，也是设置执行任务的线程或其他执行环境的地方。你的实现在任何时候都不能调用 super。 |
| main | （Optional）此方法通常用于实现与 operations 对象关联的任务。尽管可以在 start 方法中执行任务，但使用此方法实现任务可以使设置代码和任务代码更清晰地分离。 |
| isExecuting isFinished | （Required）并发 operations 负责设置其执行环境，并向外部 clients 报告该环境的状态。因此，并发 operations 必须维护一些状态信息，以便知道它何时执行任务以及何时完成任务。然后，它必须使用这些方法报告该状态。这些方法的实现必须能够安全地同时从其他线程调用。更改这些方法报告的值时，还必须为预期的 key path 生成适当的 KVO 通知。 |
| isConcurrent | （Required）要将 operations 标识为并发操作，请重写此方法并返回 YES。 |

&emsp;本节的其余部分展示了 MyOperation 类的示例实现，它演示了实现并发操作所需的基本代码。MyOperation 类只是在它创建的单独线程上执行自己的 main 方法。main 方法执行的实际工作与此无关。示例的重点是演示在定义并发操作时需要提供的基础结构。

&emsp;清单 2-5 显示了 MyOperation 类的接口和部分实现。MyOperation 类的 `isConcurrent`、`isExecuting` 和 `isFinished` 方法的实现相对简单。`isConcurrent` 方法应该简单地返回 YES 以指示这是一个并发操作。`isExecuting` 和 `isFinished` 方法只返回存储在类本身的实例变量中的值。

&emsp;Listing 2-5  Defining a concurrent operation
```c++
@interface MyOperation : NSOperation {
    BOOL        executing;
    BOOL        finished;
}

- (void)completeOperation;
@end
 
@implementation MyOperation
- (id)init {
    self = [super init];
    if (self) {
        executing = NO;
        finished = NO;
    }
    return self;
}
 
- (BOOL)isConcurrent {
    return YES;
}
 
- (BOOL)isExecuting {
    return executing;
}
 
- (BOOL)isFinished {
    return finished;
}
@end
```

&emsp;清单 2-6 显示了 MyOperation 的 `start` 方法。这种方法的实现是最小的，以便演示你绝对必须执行的任务。在这种情况下，该方法只需启动一个新线程并将其配置为调用 `main` 方法。该方法还更新执行成员变量，并为 `isExecuting` key path 生成 KVO 通知，以反映该值中的更改。完成工作后，这个方法返回，让新分离（新开辟）的线程执行实际任务。

&emsp;Listing 2-6  The start method
```c++
- (void)start {
   // Always check for cancellation before launching the task.
   if ([self isCancelled]) {
      // Must move the operation to the finished state if it is canceled.
      [self willChangeValueForKey:@"isFinished"];
      finished = YES;
      [self didChangeValueForKey:@"isFinished"];
      return;
   }
 
   // If the operation is not canceled, begin executing the task.
   [self willChangeValueForKey:@"isExecuting"];
   [NSThread detachNewThreadSelector:@selector(main) toTarget:self withObject:nil];
   executing = YES;
   [self didChangeValueForKey:@"isExecuting"];
}
```
&emsp;清单 2-7 显示了 MyOperation 类的其余实现。如清单 2-6 所示，`main` 方法是新线程的入口点。它执行与操作对象相关联的工作，并在该工作最终完成时调用自定义 `completeOperation` 方法。`completeOperation` 方法然后为 `isExecuting` 和 `isFinished` key path 生成所需的 KVO 通知，以反映操作状态的更改。

&emsp;Listing 2-7  Updating an operation at completion time
```c++
- (void)main {
   @try {
       // Do the main work of the operation here.
       [self completeOperation];
   }
   @catch(...) {
      // Do not rethrow exceptions.
   }
}
 
- (void)completeOperation {
    [self willChangeValueForKey:@"isFinished"];
    [self willChangeValueForKey:@"isExecuting"];
 
    executing = NO;
    finished = YES;
 
    [self didChangeValueForKey:@"isExecuting"];
    [self didChangeValueForKey:@"isFinished"];
}
```
&emsp;即使 operation 被取消，你也应该始终通知 KVO 观察者你的操作现在已经完成了它的工作。当一个 operation 对象依赖于其他 operation 对象的完成时，它监视这些对象的 isFinished key path。只有当所有对象都报告它们已完成时，依赖 operation 才会发出准备运行的信号。因此，未能生成完成通知可能会阻止在应用程序中执行其他 operations。

#### Maintaining KVO Compliance（保持 KVO 合规性）
&emsp;对于以下 key paths，NSOperation 类与 key-value observing（KVO）兼容：

+ isCancelled
+ isConcurrent
+ isExecuting
+ isFinished
+ isReady
+ dependencies
+ queuePriority
+ completionBlock

&emsp;如果你重写 `start` 方法或对 NSOperation 对象进行任何重要的自定义（而不是重写 main），则必须确保你的自定义对象对这些 key paths 保持 KVO 兼容。重写 start 方法时，你应该最关注的 key paths 是 isExecuting 和 isFinished。这些是重新实现该方法最常见的 key paths。

&emsp;如果要实现对除其他 operation 对象之外的其他对象的依赖关系的支持，还可以重写 `isReady` 方法并强制它返回 NO，直到满足自定义依赖关系。（如果实现自定义依赖项，如果仍然支持 NSOperation 类提供的默认依赖项管理系统，请确保从 `isReady` 方法调用 super。）当 operation 对象的就绪状态更改时，为 `isReady` key path 生成 KVO 通知以报告这些更改。除非重写 `addDependency:` 或 `removeDependency:` 方法，否则不必担心为依赖项 key path 生成 KVO 通知。

&emsp;尽管你可以为 NSOperation 的其他 key paths 生成 KVO 通知，但你不太可能需要这样做。如果需要取消操作，只需调用现有的 `cancel` 方法即可。类似地，不需要修改 operation 对象中的队列优先级信息。最后，除非 operation 能够动态更改其并发状态，否则不需要为 `isConcurrent` key path 提供 KVO 通知。

&emsp;有关键值观察以及如何在自定义对象中支持它的更多信息，请参见 Key-Value Observing Programming Guide。

### Customizing the Execution Behavior of an Operation Object（自定义 Operation 对象的执行行为）
&emsp;Operation 对象的配置发生在创建它们之后，但在将它们添加到队列之前。本节中描述的配置类型可以应用于所有 operation 对象，无论你是自己将 NSOperation 子类化还是使用现有子类。

#### Configuring Interoperation Dependencies（配置互操作依赖项）
&emsp;依赖项是序列化不同 operation 对象的执行的一种方法。依赖于其他 operation 的 operation 在其依赖的所有 operations 完成执行之前不能开始执行。因此，可以使用依赖关系在两个 operation 对象之间创建简单的一对一依赖关系，或者构建复杂的对象依赖关系图。

&emsp;要在两个 operation 对象之间建立依赖关系，可以使用 NSOperation 的 `addDependency:` 方法。此方法创建从当前 operation 对象到指定为参数的目标 operation 的单向依赖关系。此依赖关系意味着在目标对象完成执行之前，当前对象无法开始执行。依赖关系也不限于同一队列中的 operation。operation 对象管理它们自己的依赖关系，因此在 operation 之间创建依赖关系并将它们全部添加到不同的队列是完全可以接受的。但是，有一件事是不可接受的，那就是在 operation 之间创建循环依赖关系。这样做是程序员的错误，它将阻止受影响的 operation 运行。

&emsp;当一个 operation 的所有依赖项都已完成执行时，一个 operation 对象通常就可以执行了。（如果自定义 isReady 方法的行为，则 operation 的准备状态由设置的条件确定。）如果 operation 对象位于队列中，则队列可以随时开始执行该 operation。如果计划手动执行该 operation，则由你调用该 operation 的 start 方法。

> Important: 在运行 operations 或将它们添加到 operation queue 之前，应该始终配置依赖项。之后添加的依赖项可能不会阻止给定的 operation 对象运行。

&emsp;依赖关系依赖于每个 operation 对象在对象状态发生变化时发送适当的 KVO 通知。如果自定义 operation 对象的行为，则可能需要从自定义代码生成适当的 KVO 通知，以避免造成依赖性问题。有关 KVO 通知和 operation 对象的更多信息，请参见 Maintaining KVO Compliance。有关配置依赖项的更多信息，请参见 NSOperation Class Reference。

#### Changing an Operation’s Execution Priority（更改 Operation 的执行优先级）
&emsp;对于添加到队列中的 operation，执行顺序首先由排队操作的就绪状态决定，然后由它们的相对优先级决定。准备就绪由 operation 对其他 operation 的依赖关系决定，但优先级是 operation 对象本身的属性。默认情况下，所有新 operation 对象都具有 "normal" 优先级，但你可以根据需要通过调用对象的 `setQueuePriority:` 方法来增加或减少该优先级。

&emsp;优先级仅适用于同一操作队列中的 operation。如果你的应用程序有多个操作队列，则每个队列独立于任何其他队列对自己的 operation 进行优先级排序。因此，低优先级 operation 仍然可以在不同队列中的高优先级 operation 之前执行。

&emsp;优先级不能替代依赖关系。优先级决定操作队列仅开始执行当前准备就绪的 operation 的顺序。例如，如果队列同时包含高优先级和低优先级 operation，并且两个 operations 都已就绪，则队列首先执行高优先级 operation。但是，如果高优先级 operation 没有准备好，而低优先级 operation 准备好，则队列首先执行低优先级 operation。如果要阻止一个 operation 在另一个 operation 完成之前启动，则必须改用依赖项（如配置互操作依赖项中所述）。

#### Changing the Underlying Thread Priority（更改底层线程优先级）
&emsp;在 OS X v10.6 及更高版本中，可以配置 operation 的底层线程的执行优先级。系统中的线程策略本身由内核管理，但通常高优先级线程比低优先级线程有更多的运行机会。在 operation 对象中，将线程优先级指定为 0.0 到 1.0 之间的浮点值，其中 0.0 是最低优先级，1.0 是最高优先级。如果未指定显式线程优先级，则 operation 将以默认线程优先级 0.5 运行。

&emsp;要设置 operation 的线程优先级，必须先调用 operation 对象的 `setThreadPriority:` 方法，然后再将其添加到队列（或手动执行）。在执行 operation 时，默认的 `start` 方法使用你指定的值来修改当前线程的优先级。此新优先级仅在 operation 的 `main` 期间有效。所有其他代码（包括 operation 的完成 block）都以默认线程优先级运行。如果创建并发 operation，因此重写 `start` 方法，则必须自己配置线程优先级。

#### Setting Up a Completion Block（设置完成 block）
&emsp;在 OS X v10.6 及更高版本中，一个 operation 可以在其主任务完成执行时执行完成 block。你可以使用完成 block 来执行不属于主任务的任何工作。例如，你可以使用此 block 通知感兴趣的 clients operation 本身已完成。并发操作对象可能使用此完成 block 生成其最终 KVO 通知。

&emsp;要设置完成 block，请使用 NSOperation 的 `setCompletionBlock:` 方法。传递给此方法的 block 不应有参数和返回值。

### Tips for Implementing Operation Objects（实现 Operation 对象的技巧）
&emsp;尽管 operation 对象很容易实现，但在编写代码时，有几件事你应该注意。以下部分描述了在为 operation 对象编写代码时应考虑的因素。

#### Managing Memory in Operation Objects（管理 Operation 对象中的内存）
&emsp;以下各节描述了 operation 对象中良好内存管理的关键元素。有关 Objective-C 程序中内存管理的一般信息，请参见 Advanced Memory Management Programming Guide。
##### Avoid Per-Thread Storage（避免按线程存储）
&emsp;尽管大多数 operation 都在线程上执行，但在非并发操作的情况下，该线程通常由 operation queue 提供。如果一个 operation queue 为你提供了一个线程，你应该认为该线程属于该 queue，而不是被你的 operation 所触及。特别地，你不应该将任何数据与你自己不创建或管理的线程相关联。由操作队列管理的线程来来往往取决于系统和应用程序的需求。因此，使用 Per-Thread Storage 在 operation 之间传递数据是不可靠的，而且很可能失败。

&emsp;对于 operation 对象，在任何情况下都不应该使用 per-thread storage。初始化 operation 对象时，应该为对象提供执行其任务所需的一切。因此，operation 对象本身提供了所需的上下文存储。所有传入和传出的数据都应该存储在那里，直到可以重新集成到应用程序中或不再需要。

##### Keep References to Your Operation Object As Needed（根据需要保留对 Operation 对象的引用）
&emsp;仅仅因为 operation 对象是异步运行的，你就不应该假设你可以创建它们而忽略它们。它们仍然只是对象，由你来管理代码所需的对它们的任何引用。如果需要在操作完成后从中检索结果数据，这一点尤其重要。

&emsp;你应该始终保留自己对 operation 的引用的原因是，你以后可能没有机会向队列请求对象。队列尽一切努力尽可能快地调度和执行 operation。在许多情况下，队列几乎在添加后立即开始执行 operation。当你自己的代码返回到队列以获取对该 operation 的引用时，该 operation 可能已经完成并从队列中删除。

#### Handling Errors and Exceptions（处理错误和异常）
&emsp;因为 operation 本质上是应用程序中的离散实体，所以它们负责处理出现的任何错误或异常。在 OS X v10.6 及更高版本中，NSOperation 类提供的默认 `start` 方法不会捕获异常。（在 OS X v10.5 中，`start` 方法确实捕捉并抑制异常。）你自己的代码应该总是直接捕捉并抑制异常。它还应该检查错误代码，并根据需要通知应用程序的相应部分。如果替换 `start` 方法，同样必须捕获自定义实现中的任何异常，以防止它们离开底层线程的作用域。

&emsp;你应该准备好处理以下类型的错误情况：

+ 检查并处理 UNIX errno 样式错误代码。
+ 检查方法和函数返回的显式错误代码。
+ 捕获由你自己的代码或其他系统框架引发的异常。
+ 捕获 NSOperation 类本身引发的异常，该类在以下情况下引发异常：
  + 当 operation 尚未准备好执行但调用了其 `start` 方法时
  + 当 operation 正在执行或完成（可能是因为它被取消）并再次调用其 `start` 方法时
  + 当你尝试将完成 block 添加到已执行或已完成的 operation 时
  + 尝试检索已取消的 NSInvocationOperation 对象的结果时

&emsp;如果你的自定义代码确实遇到异常或错误，你应该采取任何必要的步骤将该错误传播到应用程序的其余部分。NSOperation 类不提供显式方法将错误结果代码或异常传递给应用程序的其他部分。因此，如果这些信息对应用程序很重要，则必须提供必要的代码。

### Determining an Appropriate Scope for Operation Objects（确定 operation 对象的适当范围）
&emsp;尽管可以向 operation queue 中添加任意数量的 operations，但这样做通常是不切实际的。与任何对象一样，NSOperation 类的实例消耗内存，并具有与其执行相关的实际成本。如果你的每个 operation 对象只做少量的工作，并且你创建了数以万计的 operations 对象，那么你可能会发现调度 operation 所花的时间比实际工作还要多。如果你的应用程序已经受到内存限制，你可能会发现仅仅在内存中有成千上万个 operation 对象可能会进一步降低性能。

&emsp;有效使用 operations 的关键是在你需要做的工作量和保持计算机繁忙之间找到一个适当的平衡点。尽量确保你的工作做得合理。例如，如果应用程序创建 100 个 operation 对象来对 100 个不同的值执行相同的任务，请考虑创建10 个 operation 对象来分别处理 10 个值。

&emsp;你还应该避免一次向队列中添加大量操作，或者避免连续向队列中添加 operation 对象的速度快于处理它们的速度。与其用 operation 对象淹没队列，不如成批创建这些对象。当一批处理完成执行时，使用完成 block 通知应用程序创建一批新处理。当你有很多工作要做时，你希望让队列中充满足够的 operation，以便计算机保持忙碌，但是你不希望一次创建太多的 operations 而导致应用程序内存不足。

&emsp;当然，你创建的 operation 对象的数量以及在每个 operation 对象中执行的工作量是可变的，并且完全取决于你的应用程序。你应该经常使用工具，比如 Instruments 来帮助你在效率和速度之间找到一个适当的平衡点。有关可用于收集代码度量的工具和其他性能工具的概述，请参阅 Performance Overview。

### Executing Operations（执行 Operations）
&emsp;最终，应用程序需要执行 operations 才能完成相关的工作。在本节中，你将学习几种执行 operation 的方法，以及如何在运行时操纵 operation 的执行。

#### Adding Operations to an Operation Queue（将 operations 添加到操作队列）
&emsp;到目前为止，执行 operation 的最简单方法是使用操作队列，它是 NSOperationQueue 类的一个实例。应用程序负责创建和维护它打算使用的任何操作队列。一个应用程序可以有任意数量的队列，但是在给定的时间点上执行的 operation 数量是有实际限制的。操作队列与系统一起工作，将并发 operation 的数量限制为适合可用内核和系统负载的值。因此，创建额外的队列并不意味着你可以执行额外的操作。

&emsp;要创建队列，可以像在任何其他对象中一样在应用程序中分配它：
```c++
NSOperationQueue* aQueue = [[NSOperationQueue alloc] init];
```
&emsp;要向队列添加 operations，可以使用 `addOperation:` 方法。在 OS X v10.6 及更高版本中，可以使用 `addOperations:waitUntilFinished:` 方法，也可以使用 `addOperationWithBlock:` 方法将 block 对象直接添加到队列（没有相应的 operation 对象）。这些方法中的每一个都将一个或多个操作排队，并通知队列它应该开始处理它们。在大多数情况下，operation 在添加到队列后不久执行，但是操作队列可能会由于以下任何原因而延迟队列操作的执行。具体地说，如果排队的 operation 依赖于尚未完成的其他 operations，则执行可能会延迟。如果操作队列本身已挂起或已在执行其最大并发操作数，则执行也可能会延迟。以下示例显示了向队列添加 operations 的基本语法。
```c++
[aQueue addOperation:anOp]; // Add a single operation
[aQueue addOperations:anArrayOfOps waitUntilFinished:NO]; // Add multiple operations
[aQueue addOperationWithBlock:^{
   /* Do something. */
}];
```
&emsp;Important: 在将 operation 对象添加到队列之前，应该对其进行所有必要的配置和修改，因为一旦添加，该 operation 可能随时都会运行，这对于更改产生的预期效果来说可能太晚了。

&emsp;尽管 NSOperationQueue 类是为并发执行 operation 而设计的，但是可以强制单个队列一次只运行一个操作。`setMaxConcurrentOperationCount:` 方法允许你为操作队列对象配置最大并发操作数。将值 1 传递给此方法会导致队列一次只执行一个 operation。尽管一次只能执行一个 operation，但执行顺序仍然基于其他因素，例如每个 operation 的准备情况及其分配的优先级。因此，序列化操作队列提供的行为与 Grand Central Dispatch 中的串行调度队列不同。如果 operation 对象的执行顺序对你很重要，那么在将 operation 添加到队列之前，应该使用依赖关系来建立该顺序。有关配置依赖项的信息，请参见 Configuring Interoperation Dependencies。

&emsp;有关使用操作队列的信息，请参见 NSOperationQueue Class Reference。有关串行调度队列的详细信息，请参见 Creating Serial Dispatch Queues。

#### Executing Operations Manually（手动执行操作）
&emsp;尽管操作队列是运行 operation 对象最方便的方式，但也可以在没有队列的情况下执行 operation。但是，如果你选择手动执行 operation，那么在代码中应该采取一些预防措施。特别是，operation 必须准备好运行，并且必须始终使用其 `start` 方法启动它。

&emsp;在 `isReady` 方法返回 YES 之前，operation 被认为无法运行。`isReady` 方法集成到 NSOperation 类的依赖关系管理系统中，以提供 operation 依赖关系的状态。只有当其依赖项被清除时（被全部执行完成后），operation 才可以自由地开始执行。

&emsp;手动执行 operation 时，应始终使用 `start` 方法开始执行。使用此方法，而不是 `main` 或其他方法，因为 `start` 方法在实际运行自定义代码之前会执行一些安全检查。特别是，默认 `start` 方法生成 operation 正确处理其依赖关系所需的 KVO 通知。如果 operation 已被取消，此方法还可以正确地避免执行该 operation；如果 operation 实际上还没有准备好运行，则会引发异常。

&emsp;如果应用程序定义了并发 operation 对象，则还应考虑在启动它们之前调用 `isConcurrent` 操作方法。如果此方法返回 NO，则本地代码可以决定是在当前线程中同步执行 operation，还是先创建一个单独的线程。然而，实现这种检查完全取决于你。

&emsp;清单 2-8 给出了一个简单的例子，说明在手动执行 operation 之前应该执行哪些检查。如果该方法返回 NO，则可以安排一个计时器，稍后再次调用该方法。然后继续重新安排计时器，直到方法返回 YES，这可能是因为 operation 被取消。

&emsp;Listing 2-8  Executing an operation object manually
```c++
- (BOOL)performOperation:(NSOperation*)anOp {
   BOOL ranIt = NO;
   
   if ([anOp isReady] && ![anOp isCancelled]) {
      if (![anOp isConcurrent])
         [anOp start];
      else
         [NSThread detachNewThreadSelector:@selector(start) toTarget:anOp withObject:nil];
         
      ranIt = YES;
   } else if ([anOp isCancelled]) {
      // If it was canceled before it was started, move the operation to the finished state.
      [self willChangeValueForKey:@"isFinished"];
      [self willChangeValueForKey:@"isExecuting"];
      executing = NO;
      finished = YES;
      [self didChangeValueForKey:@"isExecuting"];
      [self didChangeValueForKey:@"isFinished"];
 
      // Set ranIt to YES to prevent the operation from
      // being passed to this method again in the future.
      ranIt = YES;
   }
   return ranIt;
}
```

#### Canceling Operations（取消 Operations）
&emsp;一旦添加到操作队列中，operation 对象实际上就归队列所有，不能删除。使 operation 出队的唯一方法是取消 operation。你可以通过调用单个 operation 对象的 cancel 方法来取消它，也可以通过调用队列对象的 `cancelAllOperations` 方法来取消队列中的所有 operation 对象。

&emsp;只有在确定不再需要 operations 时，才应取消这些 operations。发出 cancel 命令会将 operation 对象置于 "canceled" 状态，这会阻止它运行。由于取消的 operation 仍被视为 "finished"，因此依赖于它的对象将收到相应的 KVO 通知以清除该依赖关系。因此，响应于一些重要事件（如应用程序退出或用户特别请求取消）而取消所有排队的 operations 比选择性地取消 operation 更为常见。

#### Waiting for Operations to Finish（等待 Operations 完成）
&emsp;为了获得最佳性能，你应该将 operation 设计为尽可能异步，让应用程序在 operation 执行时可以自由地执行其他工作。如果创建 operation 的代码也处理该 operation 的结果，则可以使用 NSOperation 的 `waitUntilFinished` 方法阻塞该代码，直到操作完成。不过，一般来说，如果可以的话，最好避免调用此方法。阻塞当前线程可能是一个方便的解决方案，但它确实会在代码中引入更多的序列化，并限制总体并发量。

> Important: 永远不要等待来自应用程序主线程的操作。你只能从辅助线程或其他操作中执行此操作。阻塞主线程会阻止应用程序响应用户事件，并可能使应用程序看起来没有响应。

&emsp;除了等待单个 operation 完成外，还可以通过调用 NSOperationQueue 的 `waitUntillalOperationsRefished` 方法来等待队列中的所有 operations。在等待整个队列完成时，请注意应用程序的其他线程仍然可以向队列添加 operations，从而延长等待时间。

#### Suspending and Resuming Queues（挂起和恢复队列）
&emsp;如果要临时停止 operations 的执行，可以使用 `setSuspended:` 方法挂起相应的操作队列。挂起队列不会导致已在执行的 operations 在任务执行过程中暂停。它只是防止新 operations 被安排执行。你可以挂起队列以响应用户请求，从而暂停任何正在进行的工作，因为预期用户最终可能希望恢复该工作。

## 参考链接
**参考链接:🔗**
+ [NSOperation](https://developer.apple.com/documentation/foundation/nsoperation?language=occ)
+ [NSBlockOperation](https://developer.apple.com/documentation/foundation/nsblockoperation?language=occ)
+ [NSInvocationOperation](https://developer.apple.com/documentation/foundation/nsinvocationoperation?language=occ)
+ [NSOperationQueue](https://developer.apple.com/documentation/foundation/nsoperationqueue?language=occ)
+ [Operation Queues](https://developer.apple.com/library/archive/documentation/General/Conceptual/ConcurrencyProgrammingGuide/OperationObjects/OperationObjects.html#//apple_ref/doc/uid/TP40008091-CH101-SW1)
+ [iOS 多线程：『NSOperation、NSOperationQueue』详尽总结](https://www.jianshu.com/p/4b1d77054b35)
+ [并发编程：API 及挑战](https://objccn.io/issue-2-1/)
