# iOS App Crash 学习：(一)：NSException 分析

> &emsp;iOS Crash 的原因是应用收到了未处理的信号，未处理的信号可能来源于三个地方：kernel(系统内核)、其它进程、App 本身。因此，Crash 异常也可以分为三种：
> + Mach 异常：是指最底层的内核级异常。用户态的开发者可以直接通过 Mach API 设置 thread、task、host 的异常端口，来捕获 Mach 异常。
> + Unix 信号：又称 BSD 信号，如果开发者没有捕获 Mach 异常，则会被 host 层的方法 ux_exception() 将异常转换为对应的 UNIX 信号，并通过方法 threadsignal() 将信号投递到出错线程。可以通过方法 signal(x, SignalHandler) 来捕获 single。 
> + NSException：应用级异常，它是未被捕获的 Objective-C 异常，导致程序向自身发送了 SIGABRT 信号而崩溃，对于未捕获的 Objective-C 异常，是可以通过 try catch 来捕获的，或者通过 NSSetUncaughtExceptionHandler(并不能阻挡程序崩溃，即该崩还是崩，我们能做的是在这里统计记录等操作) 机制来捕获。[iOS crash分类,Mach异常、Unix 信号和NSException 异常](https://blog.csdn.net/u014600626/article/details/119517507?spm=1001.2101.3001.6661.1&utm_medium=distribute.pc_relevant_t0.none-task-blog-2%7Edefault%7ECTRLIST%7Edefault-1.no_search_link&depth_1-utm_source=distribute.pc_relevant_t0.none-task-blog-2%7Edefault%7ECTRLIST%7Edefault-1.no_search_link)

&emsp;后续我们再对 Mach 异常和 Unix 信号进行深入学习，本篇先来学习我们最熟悉的 NSException。

## NSException

&emsp;系统的异常处理是一个管理非典型事件（例如发送了未被识别的消息）的过程，此过程将会中断正常的程序执行流程。如果没有足够的错误处理，遇到非典型事件时，程序可能立刻抛出或引发一种被称之为异常的行为，并结束程序的正常运行。程序抛出异常的原因多种多样，可由硬件导致也可由软件引起。异常的例子很多，包括被零除、下溢和上溢之类的数学错误，调用未定义的指令（例如，试图调用一个没有定义的方法）以及试图越界访问集合中的元素等。而 NSException 对象正是作为一个异常的载体，提示我们发生异常的原因以及发生异常时的函数调用堆栈信息等等重要的信息，来帮助我们更快速的修复造成异常的代码。[NSException异常处理](https://www.cnblogs.com/fuland/p/3668004.html)

&emsp;下面我们首先对 NSException 类包含的内容进行整理学习。 

&emsp;NSException 类是一个 NSObject 的子类，用于在 Cocoa 中描述中断程序执行正常流的特殊情况（原因），即用于描述程序正常执行被中断的原因。（An object that represents a special condition that interrupts the normal flow of program execution.）

### Declaration

```c++
#if __OBJC2__
__attribute__((__objc_exception__))
#endif
@interface NSException : NSObject <NSCopying, NSSecureCoding> {
    @private
    NSString        *name;
    NSString        *reason;
    NSDictionary    *userInfo;
    id            reserved;
}
```

&emsp;reserved 字段是一个 id 类型的保留字段，虽然它被作为保留字段，不过目前它已经被使用了，当前它会作为一个可变字典类型用来存放 `callStackReturnAddresses` 和 `callStackSymbols` 数据（对，就是我们程序崩溃时，我们最关心的函数调用堆栈数据）。

### Overview

&emsp;使用 NSException 实现 exception 处理（描述）。**exception（异常）** 是指中断正常程序执行流的一种特殊情况。每个进程都可以因不同的原因被中断执行。例如，一个应用程序可能会将文件保存在写保护（write-protected）的目录中解释为异常。从这个意义上讲 exception 相当于一个错误。另一个应用程序可能会将用户的按键（例如 Control + C）解释为异常：长时间运行的进程应该中止的指示。

### Creating and Raising an NSException Object 

#### + exceptionWithName:reason:userInfo:

```c++
typedef NSString * NSExceptionName NS_EXTENSIBLE_STRING_ENUM;

+ (NSException *)exceptionWithName:(NSExceptionName)name
                            reason:(nullable NSString *)reason
                          userInfo:(nullable NSDictionary *)userInfo;
```

&emsp;创建并返回一个 exception 对象。`name`：NSString 类型的 exception 的名字，`reason`：一个人类可读的消息字符串，总结 exception 的原因，`userInfo`：包含用户定义的与 exception 相关的信息的字典。返回值是一个 NSException 对象，或者不能创建时返回 `nil`。

#### + raise:format:

```c++

@interface NSException (NSExceptionRaisingConveniences)

+ (void)raise:(NSExceptionName)name format:(NSString *)format, ... NS_FORMAT_FUNCTION(2,3);

...
@end
```

&emsp;创建 NSException 对象并直接引发（抛出）exception 的便利函数。`name`：NSString 类型的 exception 的名字，`format`：一个人类可读的消息字符串（即异常原因），带有后面变量参数的转换规范。

```c++
[NSException raise:@"EXCEPTION_NAME" format: @"异常 reason: %@", @"测试直接引发异常", nil];
```

&emsp;例如在 viewDidLoad 函数中编写以上代码，运行程序后，程序便会直接 crash，控制台打印如下信息：

```c++
*** Terminating app due to uncaught exception 'EXCEPTION_NAME', reason: '异常 reason: 测试引入异常'
...(调用堆栈省略...)
```

#### + raise:format:arguments:

```c++
@interface NSException (NSExceptionRaisingConveniences)
...

+ (void)raise:(NSExceptionName)name format:(NSString *)format arguments:(va_list)argList NS_FORMAT_FUNCTION(2,0);

@end
```

&emsp;同上，以不同的方式追加参数。

#### - initWithName:reason:userInfo:

```c++
- (instancetype)initWithName:(NSExceptionName)aName
                      reason:(nullable NSString *)aReason
                    userInfo:(nullable NSDictionary *)aUserInfo NS_DESIGNATED_INITIALIZER;
```

&emsp;初始化并返回新分配的 exception 对象。

#### - raise

```c++
- (void)raise;
```

&emsp;引发（抛出）exception，导致程序流跳转到本地异常处理程序（即：`void uncaughtExceptionHandler(NSException *exception)` 函数，后面我们会进行详细学习）。

&emsp;在开发阶段，当某些业务逻辑的条件不满足时，我们可以创建一个 NSException 对象，然后调用 raise 函数直接抛出异常，帮助我们发现问题，或者在 try catch 语句中，当收到我们未可预知的异常时，在 catch 语句中我们继续调用 raise 函数抛出异常，方便我们发现一些未知问题。

> &emsp;When there are no exception handlers in the exception handler stack, unless the exception is raised during the posting of a notification, this method calls the uncaught exception handler, in which last-minute logging can be performed. The program then terminates, regardless of the actions taken by the uncaught exception handler.

&emsp;当异常处理程序堆栈中没有异常处理程序时，除非在发布通知期间引发异常，否则此方法调用未捕获异常处理程序（`void uncaughtExceptionHandler(NSException *exception)`），在该处理程序中可以执行最后一分钟的日志记录（实测可以执行好久，远超过 1 分钟，貌似只要函数内容执行不完就可以一直执行下去）。无论 `uncaughtExceptionHandler` 执行了什么操作，程序都会终止。

### Querying an NSException Object

#### name

```c++
typedef NSString * NSExceptionName NS_EXTENSIBLE_STRING_ENUM;

@property (readonly, copy) NSExceptionName name;
```
&emsp;一个只读的字符串，表示 NSException 对象的名字，用于唯一识别。

> &emsp;Cocoa 预先定义了一些通用异常名称，以标识可以在自己的代码中处理的异常，甚至可以引发和重新引发异常(如：在 try-catch 中捕获异常，当是未知的异常时，可以使用 @throw 或 raise 函数继续抛出异常)。你还可以创建和使用自定义异常名称。通常异常名是 NSException.h 中定义的字符串常量，记录在 Foundation Constants Reference 中。除了一般的异常名称外，Cocoa 的一些子系统还定义了自己的异常名称，例如 NSInconsistentArchiveException 和 NSFileHandleOperationException。通过将异常的名称与这些预定义的名称进行比较，可以在异常处理程序中识别捕获异常。然后你可以处理这个异常，或者，如果它不是你感兴趣的，重新抛出它。请注意，所有预定义的异常都以前缀 "NS" 开头，因此你在创建新的异常名称时应避免使用相同的前缀(避免与系统预定义的那些异常同名)。[Predefined Exceptions](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/Exceptions/Concepts/PredefinedExceptions.html)

&emsp;在 NSException.h 文件的顶部，列出了一组事先定义的异常名字，可帮助我们针对常见的抛出的异常进行归类。

```c++
/***************    Generic Exception names        ***************/

FOUNDATION_EXPORT NSExceptionName const NSGenericException;
FOUNDATION_EXPORT NSExceptionName const NSRangeException;
FOUNDATION_EXPORT NSExceptionName const NSInvalidArgumentException;
FOUNDATION_EXPORT NSExceptionName const NSInternalInconsistencyException;

FOUNDATION_EXPORT NSExceptionName const NSMallocException;

FOUNDATION_EXPORT NSExceptionName const NSObjectInaccessibleException;
FOUNDATION_EXPORT NSExceptionName const NSObjectNotAvailableException;
FOUNDATION_EXPORT NSExceptionName const NSDestinationInvalidException;
    
FOUNDATION_EXPORT NSExceptionName const NSPortTimeoutException;
FOUNDATION_EXPORT NSExceptionName const NSInvalidSendPortException;
FOUNDATION_EXPORT NSExceptionName const NSInvalidReceivePortException;
FOUNDATION_EXPORT NSExceptionName const NSPortSendException;
FOUNDATION_EXPORT NSExceptionName const NSPortReceiveException;

FOUNDATION_EXPORT NSExceptionName const NSOldStyleException;

FOUNDATION_EXPORT NSExceptionName const NSInconsistentArchiveException;
```

&emsp;我们看这一组 NSException 的名字，其中 NSRangeException 可能是我们最熟悉的，数组越界访问时产生的异常（NSException）对象的名字就是 NSRangeException。下面我们简单列一下不同的 NSExceptionName 异常所对应的一些造成此异常的情况，快速浏览即可。

##### NSGenericException

```c++
FOUNDATION_EXPORT NSExceptionName const NSGenericException;
```

&emsp;NSGenericException 是 exception 的通用名称，通常我们应该使用更具体的 exception 的名称来表示发生的异常。

1. 在 for in 循环中添加元素和删除元素：`*** Terminating app due to uncaught exception 'NSGenericException', reason: '*** Collection <__NSArrayM: 0x600003764450> was mutated while being enumerated.'` [iOS数组异常:NSGenericException,Collection <__NSArrayM: 0x61800024f7b0> was mutated while being enumerated.'](https://www.jianshu.com/p/4a5982bab58e)
2. 使用 UIActivityViewController 时，在 iPad 下必须给创建的 UIActivityViewController 对象的 .popoverPresentationController.sourceView 属性赋值，否则会 crash，iPhone 则不会：`*** Terminating app due to uncaught exception 'NSGenericException', reason: 'UIPopoverPresentationController (<UIPopoverPresentationController: 0x7fe9c95144f0>) should have a non-nil sourceView or barButtonItem set before the presentation occurs.'`。
3. iOS 10.0-10.2 时 NSURLSessionTaskMetrics 的 _initWithTask 引发的异常：`*** Terminating app due to uncaught exception 'NSGenericException', reason: 'Start date cannot be later in time than end date!'`。距离如今时间比较久了，可参考这个链接，描述的比较清楚：[iOS10系统崩溃问题修复——NSURLSessionTaskMetrics](https://www.unko.cn/2017/07/10/iOS10系统崩溃问题修复——NSURLSessionTaskMetrics/) 
4. ...

##### NSRangeException

```c++
FOUNDATION_EXPORT NSExceptionName const NSRangeException;
```

&emsp;尝试访问某些数据边界之外时发生的异常的名称。（例如字符串结尾之外）

1. 大家见的最多的数组越界访问：`*** Terminating app due to uncaught exception 'NSRangeException', reason: '*** -[__NSArrayI objectAtIndexedSubscript:]: index 3 beyond bounds [0 .. 2]'`

##### NSInvalidArgumentException

```c++
FOUNDATION_EXPORT NSExceptionName const NSInvalidArgumentException;
```

&emsp;向方法传递无效参数时发生的异常的名称，例如需要非 nil 对象时却传递了一个 nil 指针。

&emsp;非法参数异常（`NSInvalidArgumentException`）也是一个特别常见的异常，需要我们写代码时时刻注意参数的检查，避免传入非法参数导致异常，特别是传递 nil 参数导致异常。

&emsp;在日常对集合类型：NSMutableDictionary、NSMutableArray、NSDictionary 的操作中，需要特别注意的一些点如下：

+ NSMutableDictionary 添加对象:

1. `- (void)setObject:(ObjectType)anObject forKey:(KeyType <NSCopying>)aKey;`
  函数调用时 `anObject` 和 `aKey` 都不能为 nil，否则发生 NSInvalidArgumentException 异常。
  `*** Terminating app due to uncaught exception 'NSInvalidArgumentException', reason: '*** -[__NSDictionaryM setObject:forKey:]: object cannot be nil (key: key)'`
  `*** Terminating app due to uncaught exception 'NSInvalidArgumentException', reason: '*** -[__NSDictionaryM setObject:forKey:]: key cannot be nil'`
  
2. `- (void)setValue:(nullable ObjectType)value forKey:(NSString *)key;` 
  函数调用时 `key` 不能为 nil，否则发生 NSInvalidArgumentException 异常，`value` 可以为 nil，当 `value` 为 nil 时，会调用 `-removeObjectForKey:.` 函数，删除指定的 `key`。（Send -setObject:forKey: to the receiver, unless the value is nil, in which case send -removeObjectForKey:.）
  `*** Terminating app due to uncaught exception 'NSInvalidArgumentException', reason: '*** -[__NSDictionaryM setObject:forKey:]: key cannot be nil'`

3. `- (void)setObject:(nullable ObjectType)obj forKeyedSubscript:(KeyType <NSCopying>)key API_AVAILABLE(macos(10.8), ios(6.0), watchos(2.0), tvos(9.0));` 函数调用时 `obj` 不能为 nil（`key` 可以为 nil），否则发生 NSInvalidArgumentException 异常。
  `*** Terminating app due to uncaught exception 'NSInvalidArgumentException', reason: '*** -[__NSDictionaryM setObject:forKeyedSubscript:]: key cannot be nil'`
 
+ NSMutableDictionary 删除对象：  

1. `- (void)removeObjectForKey:(KeyType)aKey;` 
  函数调用时 `aKey` 不能为 nil，否则发生 NSInvalidArgumentException 异常。
  `*** Terminating app due to uncaught exception 'NSInvalidArgumentException', reason: '*** -[__NSDictionaryM removeObjectForKey:]: key cannot be nil'`

+ NSDictionary 初始化:

1. `+ (instancetype)dictionaryWithObject:(ObjectType)object forKey:(KeyType <NSCopying>)key;` 
  函数调用时 `object` 和 `key` 都不能为 nil，否则发生 NSInvalidArgumentException 异常。
  `*** Terminating app due to uncaught exception 'NSInvalidArgumentException', reason: '*** -[__NSPlaceholderDictionary initWithObjects:forKeys:count:]: attempt to insert nil object from objects[0]'` 

2. `+ (instancetype)dictionaryWithObjects:(NSArray<ObjectType> *)objects forKeys:(NSArray<KeyType <NSCopying>> *)keys;` 
  函数调用时 `objects` 和 `keys` 两个数组的元素数量必须相等，否则发生 NSInvalidArgumentException 异常。
  `*** Terminating app due to uncaught exception 'NSInvalidArgumentException', reason: '*** -[NSDictionary initWithObjects:forKeys:]: count of objects (1) differs from count of keys (2)'`
  
3. `+ (instancetype)dictionaryWithObjectsAndKeys:(id)firstObject, ... NS_REQUIRES_NIL_TERMINATION NS_SWIFT_UNAVAILABLE("Use dictionary literals instead");` 函数调用时，`(id)firstObject, ...` 参数是一个可变数量的参数，它们是被成对使用的，用 nil 做一个结尾，然后连续的一对参数，前面的值作为 value 使用，后面的值作为 key 使用。value 值在前，它和后面的第一个 key 值，组合为一个键值对插在 NSDictionary 中，当遇到第一个 nil 值的参数时便结束，所以 key 值不能为 nil，否则发生 NSInvalidArgumentException 异常，而 value 为 nil 则会被当作 NSDictionary 构建结束的标记。
  `*** Terminating app due to uncaught exception 'NSInvalidArgumentException', reason: '+[NSDictionary dictionaryWithObjectsAndKeys:]: second object of each pair must be non-nil.  Or, did you forget to nil-terminate your parameter list?'`
  由于前面的 value 为 nil 时会导致 NSDictionary 提前构建结束，所以我们可能会遇到这种情况：[慎用 dictionaryWithObjectsAndKeys：](https://www.jianshu.com/p/c723771b983b)

+ NSArray 获取匹配的下标:

1. `- (NSUInteger)indexOfObject:(ObjectType)obj inSortedRange:(NSRange)r options:(NSBinarySearchingOptions)opts usingComparator:(NSComparator NS_NOESCAPE)cmp API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0)); // binary search` 返回与使用给定 NSComparator block 的数组中的元素进行比较的对象在指定 NSRange 内的索引。此函数所以参数都不能为空：

  `obj` An object for which to search in the ordered set. If this value is nil, throws an `NSInvalidArgumentException`.
  `range` The range within the array to search for object. If r exceeds the bounds of the ordered set (if the location plus length of the range is greater than the count of the ordered set), throws an `NSRangeException`.
  `cmp` A comparator block used to compare the object obj with elements in the ordered set. If this value is NULL, throws an `NSInvalidArgumentException`.
  
+ 向 NSArray 的每个对象发送消息

1. `- (void)makeObjectsPerformSelector:(SEL)aSelector;` This method raises an NSInvalidArgumentException if aSelector is NULL. aSelector 不能为 NULL，否则发生一个 NSInvalidArgumentException 异常。aSelector 不能传参，且不能有修改原始数组的副作用。

2. `- (void)makeObjectsPerformSelector:(SEL)aSelector withObject:(id)argument;` 同上，不过可以进行传参，但同样，aSelector 不能为 NULL，否则发出一个 NSInvalidArgumentException 异常。

+ 向数组（NSArray/NSMutableArray）中添加对象时，对象不能是 nil，否则发出 NSInvalidArgumentException 异常

1. `- (NSArray<ObjectType> *)arrayByAddingObject:(ObjectType)anObject;`
2. `- (void)addObject:(ObjectType)anObject;` 

+ NSMutableArray 中插入对象

1. `- (void)insertObject:(ObjectType)anObject atIndex:(NSUInteger)index;` 如果 anObject 是 nil，则发生一个 `NSInvalidArgumentException` 异常，如果 index 大于 NSMutableArray 的元素数，则发生一个 `NSRangeException` 异常。

+ NSMutableArray 替换对象

1. `- (void)replaceObjectAtIndex:(NSUInteger)index withObject:(ObjectType)anObject;` 同样一个越界报 `NSRangeException`，一个为 nil 时，报 `NSInvalidArgumentException` 异常。

+ NSMutableArray 设置对象

1. `- (void)setObject:(ObjectType)obj atIndexedSubscript:(NSUInteger)idx;` 同样一个越界报 `NSRangeException`，一个为 nil 时，报 `NSInvalidArgumentException` 异常。
 
+ NSString 初始化

1. `- (instancetype)initWithString:(NSString *)aString;` Raises an NSInvalidArgumentException if aString is nil.

+ NSString 格式初始化

1. `- (instancetype)initWithFormat:(NSString *)format, ...;` Raises an NSInvalidArgumentException if format is nil. 

+ NSString 其他 NSString 对象初始化

1. `+ (instancetype)stringWithString:(NSString *)string;` Raises an NSInvalidArgumentException if aString is nil.

2. `- (NSString *)stringByAppendingString:(NSString *)aString;` Raises an NSInvalidArgumentException if aString is nil. 

+ NSMutableString 的格式初始化

1. `- (void)appendFormat:(NSString *)format, ...;` Raises an NSInvalidArgumentException if format is nil.

+ NSMutableString 的替换操作

1. `- (NSUInteger)replaceOccurrencesOfString:(NSString *)target withString:(NSString *)replacement options:(NSStringCompareOptions)options range:(NSRange)searchRange;` 其中三个参数都可能导致不同的异常。
  Raises an NSInvalidArgumentException if target is nil. 
  Raises an NSInvalidArgumentException if replacement is nil.
  Raises an NSRangeException if any part of searchRange lies beyond the end of the receiver.
  
+ NSString 的前后缀判断

1. `- (BOOL)hasPrefix:(NSString *)str;` `*** Terminating app due to uncaught exception 'NSInvalidArgumentException', reason: '-[__NSCFConstantString hasPrefix:]: nil argument'`

2. `- (BOOL)hasSuffix:(NSString *)str;` `*** Terminating app due to uncaught exception 'NSInvalidArgumentException', reason: '-[__NSCFConstantString hasSuffix:]: nil argument'`

+ .storyboard 或者 .xib 加载，对应的文件名不存在

1. `*** Terminating app due to uncaught exception 'NSInvalidArgumentException', reason: 'Could not find a storyboard named 'Main' in bundle NSBundle </Users/hmc/Library/Developer/CoreSimulator/Devices/2ADFB467-5FFF-4F61-872F-73F1CF1C2174/data/Containers/Bundle/Application/1DE87EF5-E2FA-44B8-8967-F565941653CB/dSYMDemo.app> (loaded)'`

##### NSInternalInconsistencyException

```c++
FOUNDATION_EXPORT NSExceptionName const NSInternalInconsistencyException;
```

&emsp;看名字大意是指 "内部矛盾" 异常，当内部断言失败时发生的异常的名称，并暗示被调用代码中存在意外情况。`Name of an exception that occurs when an internal assertion fails and implies an unexpected condition within the called code.`

1. mutating method sent to immutable object。（例如函数调用返回了一个 NSDictionary 对象，但是使用了一个 NSMutableDictionary 指针变量去接收，并在接下来把此对象当作一个可变字典进行一些改变操作。测试代码现在报 NSInvalidArgumentException 异常，而不再是 NSInternalInconsistencyException 异常了。[iOS Crash之NSInternalInconsistencyException](https://blog.csdn.net/skylin19840101/article/details/51991825)）
2. 手动创建 `UIApplication` 对象：`UIApplication *app = [[UIApplication alloc] init];`，看到 `UIApplication` 的 `init` 函数触发了断言，并随后报出了一个 `NSInternalInconsistencyException` 异常。这个大家应该都比较能理解，在当前进程中，UIApplication 作为一个单例类使用，App 启动时，系统会自动构建一个 UIApplication 对象，表示当前进程。
  `*** Assertion failure in -[UIApplication init], UIApplication.m:1469`
  `*** Terminating app due to uncaught exception 'NSInternalInconsistencyException', reason: 'There can only be one UIApplication instance.'` [NSInternalInconsistencyException](https://www.jianshu.com/p/0b227de4a90a)
3. 指定刷新 tableView 并超出当前 section 和 row 的范围。看到 `UITableView` 的 `_endCellAnimationsWithContext` 函数触发了断言，并随后报出了一个 `NSInternalInconsistencyException` 异常。
  原因：在调用 `reloadRowsAtIndexPaths` 时，依赖于 tableView 先前的状态已有要更新的 cell，它内部是先删除该 cell，再重新创建，所以当你在原先没有该 cell 的状态下调用 `reloadRowsAtIndexPaths`，会报异常你正在尝试删除不存在的 cell。reloadData 是完全重新加载，包括 cell 数量也会重新计算，不会依赖之前 tableView 的状态。[iOS调用reloadRowsAtIndexPaths Crash报异常NSInternalInconsistencyException](https://blog.csdn.net/sinat_27310637/article/details/62225658) 
  `*** Assertion failure in -[UITableView _endCellAnimationsWithContext:], UITableView.m:2097`
  `*** Terminating app due to uncaught exception 'NSInternalInconsistencyException', reason: 'attempt to delete row 6 from section 0 which only contains 5 rows before the update'`
  
##### NSMallocException

```c++
FOUNDATION_EXPORT NSExceptionName const NSMallocException;
```

&emsp;Obsolete; not currently used.（废弃了，不再使用。它用来代表内存不足的问题，无法分配足够的内存空间，比如需要分配的内存大小是一个不正常的值，比较巨大或者设备的内存空间不足以及被耗尽了。虽然说是被废弃了，但是我们一些错误的示例代码还是可以触发抛出一个这个名字的异常，如下：[iOS Crash之NSMallocException](https://blog.csdn.net/skylin19840101/article/details/51944701)

1. 需要分配的空间过大。
  ```c++
  NSMutableData *data = [[NSMutableData alloc] initWithCapacity:1];
  long long len = 203293514220329351;
  [data increaseLengthBy:len];
  ```
  运行此代码便会抛出异常：`*** Terminating app due to uncaught exception 'NSMallocException', reason: 'Failed to grow buffer'`。
  
2. 以及还有图片占用空间过大和 OOM（Out of memory）问题。

##### NSObjectInaccessibleException

```c++
FOUNDATION_EXPORT NSExceptionName const NSObjectInaccessibleException;
```

&emsp;从不应访问 remote object 的线程访问该对象时发生的异常的名称。（Name of an exception that occurs when a remote object is accessed from a thread that should not access it. 暂时未遇到过。）

##### NSObjectNotAvailableException

```c++
FOUNDATION_EXPORT NSExceptionName const NSObjectNotAvailableException;
```
&emsp;Name of an exception that occurs when the remote side of the NSConnection refused to send the message to the object because the object has never been vended.

1. `*** Terminating app due to uncaught exception 'NSObjectNotAvailableException', reason: 'UIAlertView is deprecated and unavailable for UIScene based applications, please use UIAlertController!'`

##### NSDestinationInvalidException

```c++
FOUNDATION_EXPORT NSExceptionName const NSDestinationInvalidException;
```

&emsp;Name of an exception that occurs when an internal assertion fails and implies an unexpected condition within the distributed objects. This is a distributed objects–specific exception.（当内部断言失败时发生的异常的名称，并暗示 distributed objects 中存在意外情况。这是一个分布式对象特定的例外。）

##### NSPortTimeoutException

```c++
FOUNDATION_EXPORT NSExceptionName const NSPortTimeoutException;
```

&emsp;Name of an exception that occurs when a timeout set on a port expires during a send or receive operation. This is a distributed objects–specific exception.

##### NSInvalidSendPortException

```c++
FOUNDATION_EXPORT NSExceptionName const NSInvalidSendPortException;
```

&emsp;Name of an exception that occurs when the send port of an NSConnection has become invalid. This is a distributed objects–specific exception.

##### NSInvalidReceivePortException

```c++
FOUNDATION_EXPORT NSExceptionName const NSInvalidReceivePortException;
```

&emsp;Name of an exception that occurs when the receive port of an NSConnection has become invalid. This is a distributed objects–specific exception.

##### NSPortSendException

```c++
FOUNDATION_EXPORT NSExceptionName const NSPortSendException;
```

&emsp;Generic error occurred on send.This is an NSPort-specific exception.

##### NSPortReceiveException

```c++
FOUNDATION_EXPORT NSExceptionName const NSPortReceiveException;
```

&emsp;Generic error occurred on receive.This is an NSPort-specific exception.

##### NSOldStyleException

```c++
FOUNDATION_EXPORT NSExceptionName const NSOldStyleException;
```

##### NSInconsistentArchiveException

```c++
FOUNDATION_EXPORT NSExceptionName const NSInconsistentArchiveException;
```

&emsp;The name of an exception raised by NSArchiver if there are problems initializing or encoding.

#### reason

```c++
@property (nullable, readonly, copy) NSString *reason;
```

&emsp;一个只读的字符串，可能为 nil，一个人类可读（human-readable）的字符串，用于表示 exception 发生的原因。例如我们常见的数组越界访问的错误打印：`*** -[__NSArrayI objectAtIndexedSubscript:]: index 3 beyond bounds [0 .. 2]`。

&emsp;还有一个小小的细节，NSException 类的 `- (NSString *)description { ... }` 函数就仅仅返回 NSException 实例对象的 `reason` 属性，如 `NSLog(@"🐝🐝🐝 exception: %@", exception);` 就仅输出 `exception.reason` 的值。

#### userInfo

```c++
@property (nullable, readonly, copy) NSDictionary *userInfo;
```

&emsp;一个只读的 NSDictionary，可能为 nil，包含与 exception 对象相关的特定于应用程序的数据。

&emsp;如果不存在特定于应用程序的数据，则为 nil。例如，如果某个方法的返回值导致引发异常，则该返回值可能通过该方法可供异常处理程序使用。

### Getting Exception Stack Frames

#### callStackReturnAddresses

```c++
@property (readonly, copy) NSArray<NSNumber *> *callStackReturnAddresses API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
```

&emsp;与抛出的异常相关的调用返回地址（call return addresses）。

&emsp;`callStackReturnAddresses` 是一个元素都是整数值（po 打印的是十进制整数）的 NSNumber 对象数组。每个值都是一个调用帧（call frame 即函数调用栈中的每一个函数）的返回地址。栈帧（stack frames）数组从第一次引发异常的点开始，第一项是最近的栈帧。（假装为 NSException 类的 NSException 子类或干扰异常引发机制的子类或其他 API 元素可能无法获取此信息）

#### callStackSymbols

```c++
@property (readonly, copy) NSArray<NSString *> *callStackSymbols API_AVAILABLE(macos(10.6), ios(4.0), watchos(2.0), tvos(9.0));
```

&emsp;返回包含当前异常抛出时调用堆栈符号的数组。

&emsp;包含调用堆栈符号的数组。每个元素都是一个 NSString 对象，其值的格式由 `backtrace_symbols()` 函数确定。

&emsp;返回值描述调用此方法时当前线程的调用堆栈回溯。

&emsp;callStackSymbols 和 callStackReturnAddresses 其实是保存在 exception 对象的 reserved 字段中的。如下示例，是一个很常见的数组越界导致的 crash，name、reason、userInfo 三个字段的内容我们比较熟悉，然后我们重点关注 reserved 字段（保留字段），可看到它是一个可变字典，共有两个 key：callStackReturnAddresses 和 callStackSymbols，它们的 value 分别是一个长度是 17 的数组，分别存放的是调用堆栈的返回地址和调用堆栈的符号。

![截屏2021-10-18 08.51.07.png](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/fa1e44710b694a0982ccf1145d79866b~tplv-k3u1fbpfcp-watermark.image?)

### Related Types

#### NSUncaughtExceptionHandler

```c++
// 在 Developer Documentation 中是这种写法：
typedef void (NSException * _Nonnull) NSUncaughtExceptionHandler;

// 在 NSException.h 中是这种写法：
typedef void NSUncaughtExceptionHandler(NSException *exception);
```

&emsp;给返回值是 void 入参是 NSException 指针（非空）的函数起一个 `NSUncaughtExceptionHandler` 的别名。  

#### NSExceptionName

```c++
typedef NSString *NSExceptionName;
```

&emsp;异常的名字的类型其实是 NSString。

### Functions

#### NSGetUncaughtExceptionHandler

```c++
NSUncaughtExceptionHandler * NSGetUncaughtExceptionHandler(void);
```

&emsp;返回当前最顶层的异常处理程序。

&emsp;返回一个 `NSUncaughtExceptionHandler` 函数指针，指向当前 top-level 的 error-handling function，当异常抛出程序被终止执行之前会调用此函数，我们可以在其中添加异常记录，把异常信息写入本地，等 APP 下一次启动时发送到服务器中去进行统计。（默认情况下 `NSUncaughtExceptionHandler` 返回值为空，需要我们调用 `NSSetUncaughtExceptionHandler` 函数进行提前设置。）

#### NSSetUncaughtExceptionHandler

```c++
FOUNDATION_EXPORT void NSSetUncaughtExceptionHandler(NSUncaughtExceptionHandler * _Nullable);
```

&emsp;改变（设置）当前最顶层的异常处理程序。

&emsp;所有未捕获的异常都应该进行抓取处理或者进行统计上传，作为程序运行的反馈和监测。在 OC 中我们可以使用 @try @catch 语句来捕获异常，而未捕获的异常我们还有一次统一处理的机会，我们便可以使用 `NSSetUncaughtExceptionHandler` 来设置这个函数。

```c++
    @try {
        // 有可能出现异常的代码
    } @catch (NSException *exception) {
        // 如果对象不存在
        if ([exception.name isEqualToString:NSObjectInaccessibleException]) {
            NSLog(@"Object have not exits");
        } else {
            // 抛给未处理异常函数去处理，可使用 raise 或 @throw 继续抛出异常
            [exception raise];
//            @throw exception;
        }
    } @finally {
        // 
    }
```

&emsp;首先编写一个 `NSUncaughtExceptionHandler` 类型的函数：

```c++
void uncaughtExceptionHandler(NSException *exception) {
    NSArray *stackSymbols = [exception callStackSymbols];
    NSArray *stackReturnAddress = [exception callStackReturnAddresses];
    
    NSString *crashReportString = [NSString stringWithFormat:@"异常报告：\nname:%@\nreason:%@\ncallStackSymbols:\n%@\nstackReturnAddress:\n%@", [exception name], [exception reason], [stackSymbols componentsJoinedByString:@"\n"], [stackReturnAddress componentsJoinedByString:@"\n"]];
    NSLog(@"🏵🏵🏵 crashReportString: %@", crashReportString);
    NSString *path = [[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject] stringByAppendingPathComponent:@"Exception.txt"];
    [crashReportString writeToFile:path atomically:YES encoding:NSUTF8StringEncoding error:nil];
}
```

&emsp;然后我们再调用 `NSSetUncaughtExceptionHandler` 函数把 `uncaughtExceptionHandler` 设置为统一处理未捕获异常的函数。这里还有一个点，如果我们调用 `NSSetUncaughtExceptionHandler` 之前，已经有其它引入的第三方 SDK 设置了未捕获异常的处理函数，此时我们再设置就会覆盖之前的设置（或者我们自己设置过后，又被第三方 SDK 设置了一遍，导致它把我们自己设置的未捕获异常处理函数覆盖了），所以我们可以使用 `NSGetUncaughtExceptionHandler`来获取当前的未捕获异常处理函数，并用一个函数指针记录下来，然后在我们新设置的未捕获异常处理函数中再调用一次原始的异常处理函数。

```c++
void originalUncaughtExceptionHandler(NSException *exception);

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    
    NSUncaughtExceptionHandler *currentHandler = NSGetUncaughtExceptionHandler();
    NSLog(@"✳️✳️✳️ 当前的未捕获的异常的处理程序：%p", currentHandler);
    NSSetUncaughtExceptionHandler(&uncaughtExceptionHandler);
    
    return YES;
}
```
## 参考链接
**参考链接:🔗**
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

