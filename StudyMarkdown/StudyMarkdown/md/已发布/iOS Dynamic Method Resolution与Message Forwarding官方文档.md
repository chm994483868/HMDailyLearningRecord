# iOS Dynamic Method Resolution与Message Forwarding官方文档翻译

> &emsp;本篇从官方文档来详细学习动态方法解析和消息转发流程。

## Messaging
&emsp;本章首先介绍如何将消息表达式转换为 objc_msgSend 函数调用，以及如何按名称引用方法。然后解释了如何利用 objc_msgsend，以及如果需要，如何绕过动态绑定（dynamic binding）。（动态绑定就是沿着对象的 isa 在类的继承体系中查找具体的函数实现的过程）
### The objc_msgSend Function
&emsp;在 Objective-C 中，消息直到运行时才绑定到方法实现。编译器将转换如下消息表达式，进入对消息传递函数 objc_msgSend 的调用。
```c++
[receiver message]
```
&emsp;此函数将接收方和消息中提到的方法的名称（即方法选择器）作为其两个主要参数：
```c++
objc_msgSend(receiver, selector)
```
&emsp;消息中传递的所有参数也都传递给 objc_msgSend：
```c++
objc_msgSend(receiver, selector, arg1, arg2, ...)
```
&emsp;消息传递功能执行动态绑定所需的所有操作：

+ 它首先查找选择器引用的 procedure（方法实现）。由于同一方法可以通过不同的类实现，因此它找到的精确过程取决于接收者的类。
+ 然后，它将调用该 procedure（方法实现），并将接收对象（指向其数据的指针）以及为该方法指定的所有参数传递给该 procedure（方法实现）。
+ 最后，它将 procedure（方法实现）的返回值作为自己的返回值传递。

> &emsp;Note: 编译器生成对消息传递功能的调用。你永远不要在编写的代码中直接调用它。

&emsp;消息传递的关键在于编译器为每个类和对象构建的结构。每个类结构都包含以下两个基本元素：
+ 指向 superclass 的指针。
+ 类调度表。此表中的条目将方法选择器与其标识的方法的类特定地址相关联。setOrigin: 方法的选择器与 setOrigin: 的地址（方法实现）相关联，display 方法的选择器与 display 的地址相关联，依此类推。

&emsp;创建新对象时，会为其分配内存，并初始化其实例变量。对象变量中的第一个是指向其类结构的指针。这个名为 isa 的指针给对象访问它的类的权限，并通过该类访问它从中继承的所有类。

> &emsp;Note: 虽然严格来说 isa 指针不是语言的一部分，但是对象要与 Objective-C 运行时系统一起工作，isa 指针是必需的。在结构定义的任何字段中，对象都需要与结构 objc_object（在 objc/objc.h 中定义）“等价”。但是，你很少需要创建自己的根对象，并且从 NSObject 或 NSProxy 继承的对象自动具有 isa 变量。

&emsp;类和对象结构的这些元素如图 3-1 所示。

&emsp;Figure 3-1  Messaging Framework

![messaging1](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/15220a26b58f4ede8edc321dad10eefd~tplv-k3u1fbpfcp-watermark.image)

&emsp;当一条消息被发送到一个对象时，消息传递函数跟随对象的 isa 指针，指向类结构，在那里它在调度表中查找方法选择器。如果在那里找不到选择器，objc_msgSend 将跟随指向超类的指针，并尝试在其调度表中找到选择器。连续的失败会导致 objc_ msgSend 爬升类层次结构，直到它到达 NSObject 类。一旦找到选择器，函数将调用表中输入的方法，并将接收对象的数据结构传递给它。（可参考前面方法执行流程的文章）

&emsp;这是在运行时选择方法实现的方式，或者用面向对象编程的术语来说，方法是动态绑定到消息的。

&emsp;为了加快消息传递过程，运行时系统会在使用方法时缓存它们的选择器和地址。每个类都有一个单独的缓存，它可以包含继承方法和类中定义的方法的选择器。在搜索调度表之前，消息传递例程首先检查接收对象类的缓存（理论上，曾经使用过的方法可能会再次使用）。如果方法选择器在缓存中，消息传递只比函数调用稍微慢一点。一旦程序运行足够长的时间来 “预热” 缓存，它发送的几乎所有消息都会找到一个缓存方法。当程序运行时，缓存动态增长以容纳新消息。（可认真学习 cache_t 来补充方法缓存方面的知识点）
### Using Hidden Arguments
&emsp;当 objc_msgSend 找到实现方法的 procedure 时，它将调用该 procedure 并将消息中的所有参数传递给该过程。它还向 procedure 传递两个隐藏参数：

+ 接收对象（self）
+ 方法的选择器（_cmd）

&emsp;这些参数为每个方法实现提供了有关调用它的消息表达式的两部分的显式信息。它们被称为 “隐藏”，因为它们没有在定义方法的源代码中声明。在编译代码时会将它们插入到实现中。

&emsp;尽管这些参数没有显式声明，源代码仍然可以引用它们（就像它可以引用接收对象的实例变量一样）。方法将接收对象称为 self，将其自身的选择器称为 \_cmd。在下面的示例中，\_cmd 引用 strange 方法的选择器，self 引用接收 strange 消息的对象。
```c++
- strange {
    id  target = getTheReceiver();
    SEL method = getTheMethod();
 
    if ( target == self || method == _cmd )
        return nil;
    return [target performSelector:method];
}
```
&emsp;在这两个参数中，self 更有用。实际上，这是接收对象的实例变量对方法定义可用的方式。
### Getting a Method Address
&emsp;绕过动态绑定的唯一方法是获取方法的地址并直接调用它，就好像它是一个函数一样。当一个特定的方法将连续执行很多次，并且你希望避免每次执行该方法时的消息传递开销时，这种方法可能非常合适。

&emsp;使用 NSObject 类中定义的方法 methodForSelector:，你可以要求一个指向实现该方法的 procedure 的指针，然后使用该指针来调用该 procedure。 methodForSelector: 返回的指针必须仔细转换为适当的函数类型。返回类型和参数类型都应包含在强制类型转换中。

&emsp;下面的示例显示了如何调用实现 setFilled: 方法的过程:
```c++
void (*setter)(id, SEL, BOOL);
int i;
 
setter = (void (*)(id, SEL, BOOL))[target methodForSelector:@selector(setFilled:)];
for ( i = 0 ; i < 1000 ; i++ )
    setter(targetList[i], @selector(setFilled:), YES);
```
&emsp;传递给该 procedure 的前两个参数是接收对象（self）和方法选择器（\_cmd）。这些参数隐藏在方法语法中，但是在将方法作为函数调用时必须将其明确显示。使用 methodForSelector: 规避动态绑定可以节省消息传递所需的大部分时间。但是，仅在重复多次发送特定消息的情况下，这种节省才是可观的，如上面的 for 循环所示。注意 methodForSelector: 由 Cocoa 运行时系统（runtime system）提供；这不是 Objective-C 语言本身的功能。

&emsp;上面是 Objective-C 中发送消息的过程，如果在动态绑定的过程中即使一直查找到 NSObject 也没有找到方法选择器对应的方法实现的话，则会进入消息转发过程。那么下面就就进入本篇文章的核心内容（动态解析（dynamically resolve）和消息转发（Message Forwarding））。

&emsp;在学习动态解析和消息转发之前我们首先对一些知识点做一个铺垫学习，包括：Selector 定义、NSMethodSignature 类文档、NSInvocation 类文档、class_addMethod 函数文档、resolveInstanceMethod: 函数定义、resolveClassMethod: 函数定义、forwardingTargetForSelector: 函数定义、methodSignatureForSelector: 函数定义、forwardInvocation: 函数定义、doesNotRecognizeSelector: 函数定义。

## Selector
&emsp;选择器（Selector）是用于选择要为对象执行的方法的名称，或者是在编译源代码时替换该名称的唯一标识符。选择器本身不起任何作用。它只是标识一个方法。唯一使选择器方法名不同于普通字符串的是编译器确保选择器是唯一的。选择器之所以有用，是因为（与运行时一起）它的作用就像一个动态函数指针，对于给定的名称，它会自动指向一个方法的实现，该方法适用于与它一起使用的任何类。假设你有一个方法运行的选择器，类 Dog、Athlete 和 ComputerSimulation（每个类都实现了一个 run 方法）。选择器可以与每个类的实例一起使用，以调用其 run 方法，即使每个类的实现可能不同。
### Getting a Selector
&emsp;编译的选择器是 SEL 类型。获取选择器有两种常见方法：
+ 在编译时，使用编译器指令 @selector。
```c++
SEL aSelector = @selector(methodName);
```
+ 在运行时，使用 NSSelectorFromString 函数，其中 string 是方法的名称：
```c++
SEL aSelector = NSSelectorFromString(@"methodName");
```
&emsp;当你想让你的代码发送一个直到执行阶段才知道其名称的消息时，你可以使用从字符串建立的选取器。
### Using a Selector
&emsp;可以以 selector 为参数调用 performSelector: 方法和其他类似方法来执行 selector 方法。
```c++
SEL aSelector = @selector(run);

[aDog performSelector:aSelector];
[anAthlete performSelector:aSelector];
[aComputerSimulation performSelector:aSelector];
```
&emsp;（你可以在特殊情况下使用此技术，例如在实现使用 Target-Action 设计模式的对象时。通常情况下，你只需直接调用该方法。）
## NSMethodSignature
&emsp;方法的返回值和参数的类型信息记录。
```c++
NS_SWIFT_UNAVAILABLE("NSInvocation and related APIs not available")
@interface NSMethodSignature : NSObject
```
&emsp;使用 NSMethodSignature 对象转发接收对象不响应的消息，尤其是在分布式对象（distributed objects）的情况下。通常，你可以使用 NSObject 的 methodSignatureForSelector: 实例方法创建 NSMethodSignature 对象（在 macOS 10.5 及更高版本中，你还可以使用 signatureWithobjType:）。然后使用它创建一个 NSInvocation 对象，该对象作为参数传递给 forwardInvocation: 消息，以将调用发送到任何其他可以处理该消息的对象。在默认情况下，NSObject 调用 doesNotRecognizeSelector: 这会引发异常。对于分布式对象，NSInvocation 对象使用 NSMethodSignature 对象中的信息进行编码，并发送到消息接收方表示的真实对象。

&emsp;Type Encodings: NSMethodSignature 对象是用一个字符数组初始化的，该数组表示方法的返回和参数类型的字符串编码。可以使用 @encode() 编译器指令获取特定类型的字符串编码。因为字符串编码是特定于实现的，所以不应该硬编码这些值。

&emsp;方法签名由方法返回类型的一个或多个字符组成，后跟隐式参数 self 和 \_cmd 的字符串编码，后跟零个或多个显式参数。可以使用 methodReturnType 和 methodReturnLength 属性确定字符串编码和返回类型的长度。可以使用 getArgumentTypeAtIndex: 方法和 numberOfArguments 属性分别访问参数。

&emsp;例如，NSString 实例方法 containsString: 具有以下参数的方法签名：

1. @encode(BOOL) (c) 返回类型是 c。BOOL 的类型编码是 c（BOOL 内部实际是由 char 类型表示的）。
2. @encode(id) (@) 对应第一个隐式参数 receiver 是 self。id 的类型编码是 @。
3. @encode(SEL) (:) 对应第二个隐式参数 \_cmd 是 SEL。SEL 的类型编码是 :。
4. @encode(NSString *) (@) 对应与第一个显示参数。NSString * 的类型编码是 @。

&emsp;综上 containsString:  的方法签名就是: c@:@。

&emsp;具体详情可以参考 [Objective-C Runtime Programming Guide](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/ObjCRuntimeGuide/Introduction/Introduction.html#//apple_ref/doc/uid/TP40008048) 的 [Type Encodings](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/ObjCRuntimeGuide/Articles/ocrtTypeEncodings.html#//apple_ref/doc/uid/TP40008048-CH100) 部分的内容。
### signatureWithObjCTypes:
&emsp;返回给定 Objective-C 方法类型字符串的 NSMethodSignature 对象。
```c++
+ (nullable NSMethodSignature *)signatureWithObjCTypes:(const char *)types;
```
&emsp;`types`: 包含方法参数类型编码的字符数组。

&emsp;Return Value: `types` 中给定 Objective-C 方法类型字符串的 NSMethodSignature 对象。
### getArgumentTypeAtIndex:
&emsp;返回给定索引处参数的类型编码。
```c++
- (const char *)getArgumentTypeAtIndex:(NSUInteger)idx NS_RETURNS_INNER_POINTER;
```
&emsp;索引以 0 开头。隐式参数 self（类型为 id）和 \_cmd（类型为 SEL）位于索引 0 和 1 处；显式参数从索引 2 开始。

> &emsp;Important: 如果 index 超过参数个数，则引发 NSInvalidArgumentException。
### numberOfArguments
&emsp;只读的参数个数属性。
```c++
@property (readonly) NSUInteger numberOfArguments;
```
&emsp;始终至少有两个参数，因为 NSMethodSignature 对象包含隐式参数 self 和 \_cmd，这是传递给每个方法实现的前两个参数。
### frameLength
&emsp;参数加在一起在堆栈中占据的字节数。
```c++
@property (readonly) NSUInteger frameLength;
```
&emsp;此数字随应用程序运行的硬件体系结构而变化。
### methodReturnType
&emsp;一个 C 字符串，以 Objective-C 类型编码对方法的返回类型进行编码。
```c++
@property (readonly) const char *methodReturnType NS_RETURNS_INNER_POINTER;
```
### methodReturnLength
&emsp;返回值所需的字节数。
```c++
@property (readonly) NSUInteger methodReturnLength;
```
### isOneway
&emsp;通过分布式对象调用时接收方是否异步。
```c++
- (BOOL)isOneway;
```
&emsp;Return Value: 如果通过分布式对象调用 receiver 时异步，则为 YES，否则为 NO。

&emsp;如果该方法是 oneway，则远程消息的发件人不会阻塞等待 reply。（例如 dispatch_async 和 dispatch_sync 两个函数一个会直接返回，一个则等 block 执行完毕才返回）
## NSInvocation
&emsp;描绘为对象的 Objective-C 消息。（即以 NSInvocation 对象描述 Objective-C 消息）
```c++
NS_SWIFT_UNAVAILABLE("NSInvocation and related APIs not available")
@interface NSInvocation : NSObject
```
&emsp;NSInvocation 对象主要由 NSTimer 对象和分布式对象系统用于在对象之间和应用程序之间存储和转发消息。NSInvocation 对象包含 Objective-C 消息的所有元素：目标（target）、选择器（selector）、参数（arguments）和返回值（return value）。这些元素中的每一个都可以直接设置，并且在调度 NSInvocation 对象时会自动设置返回值。

&emsp;NSInvocation 对象可以重复调度到不同的目标（targets）；可以在调度之间修改其参数以获得不同的结果；甚至可以将其选择器（selector）更改为具有相同方法签名（参数和返回类型）的另一个。这种灵活性使 NSInvocation 在重复具有许多参数和变体的消息时非常有用；你可以在每次将 NSInvocation 对象调度到新目标之前根据需要修改它，而不是为每条消息重新键入稍微不同的表达式。

&emsp;NSInvocation 不支持使用可变数量的参数或 union 参数调用方法。应该使用 invocationWithMethodSignature: 类方法创建 NSInvocation 对象；不应该使用 alloc 和 init 创建这些对象。

&emsp;默认情况下，此类不 retain 所包含调用的参数。如果在创建 NSInvocation 实例和使用 NSInvocation 实例之间这些对象可能会消失，那么你应该自己显式保留这些对象，或者调用 retainArguments 方法让 NSInvocation 对象自己保留它们。

> &emsp;Note: NSInvocation 符合 NSCoding 协议，但仅支持通过 NSPortCoder 进行编码。 NSInvocation 不支持 archiving。

### invocationWithMethodSignature:
&emsp;返回一个 NSInvocation 对象，该对象能够使用给定的 method signature 构造消息。
```c++
+ (NSInvocation *)invocationWithMethodSignature:(NSMethodSignature *)sig;
```
&emsp;`sig`: 封装方法签名的对象。

&emsp;必须先使用 NSInvocation 设置其选择器，然后使用 setArgument:atIndex: 设置其参数，然后才能对其进行调用。不要使用 alloc/init 方法来创建 NSInvocation 对象。
### selector
&emsp;NSInvocation 对象的选择器；如果尚未设置，则为 0。
```c++
@property SEL selector;
```
### target
&emsp;NSInvocation 对象的 target；如果 NSInvocation 对象还没有设置 target，则为 nil。
```c++
@property (nullable, assign) id target;
```
&emsp;target 是通过 NSInvocation 类的 invoke 函数发送的消息的接收者。
### setArgument:atIndex:
&emsp;设置 NSInvocation 对象的参数。
```c++
- (void)setArgument:(void *)argumentLocation atIndex:(NSInteger)idx;
```
&emsp;`argumentLocation`: 要分配给 NSInvocation 对象的参数的无类型内存空间的起始地址。`index`: 一个整数，指定参数的索引。索引 0 和 1 分别表示隐藏参数 self 和 \_cmd；你应该直接使用 `target` 和 `selector` 属性设置这些值。对于通常在消息中传递的参数，请使用索引 2 和更大的值。

&emsp;此方法将 `argumentLocation` 的内容复制为索引处的参数。复制的字节数由参数大小决定。

&emsp;当参数值是一个对象时，将指针传递到应该从中复制对象的变量（或内存）：
```c++
NSArray *anArray;
[invocation setArgument:&anArray atIndex:3];
```
&emsp;如果 index 的值大于 `selector` 的实际参数个数，则此方法引发 NSInvalidArgumentException。
### getArgument:atIndex:
&emsp;通过间接返回指定索引处的 NSInvocation 对象的参数。
```c++
- (void)getArgument:(void *)argumentLocation atIndex:(NSInteger)idx;
```
&emsp;`argumentLocation`: 一个无类型的内存空间的起始地址，用于保存返回的参数。（一段内存空间的起始地址）`index`: 一个整数，指定要获取的参数的索引。索引 0 和 1 分别表示隐藏参数 self 和 \_cmd；这些值可以直接使用 `target` 和 `selector`方法检索。对于通常在消息中传递的参数，请使用索引 2 和更大的值。 

&emsp;此方法将索引处存储的参数复制到 `argumentLocation` 指向的存储中。`argumentLocation` 的大小必须足够大以容纳参数值。当参数值是一个对象时，将指针传递到该对象应放入的变量（或内存）：
```c++
NSArray *anArray;
[invocation getArgument:&anArray atIndex:3];
```
&emsp;如果 index 大于 `selector` 的实际参数数量，则此方法引发 NSInvalidArgumentException。
### argumentsRetained
&emsp;如果 NSInvocation 对象保留其 arguments，则为 YES，否则为 NO。
```c++
@property (readonly) BOOL argumentsRetained;
```
### retainArguments
&emsp;如果 NSInvocation 对象尚未执行过此操作，则 retain NSInvocation 对象的 target 和所有对象 arguments，并复制其所有 C 字符串参数和 blocks。如果已设置 returnvalue，则也会 retained 或复制该值。
```c++
- (void)retainArguments;
```
&emsp;在调用此方法之前，argumentsRetained 属性返回 NO；否则，返回 false。之后，它返回 YES。

&emsp;为了提高效率，新创建的 NSInvocation 对象不保留或复制其参数，也不保留其目标、复制 C 字符串或复制任何关联的 blocks。如果要缓存 NSInvocation 对象，则应该指示该对象保留其参数，因为这些参数可能会在调用之前释放。NSTimer 对象总是指示它们的调用保留它们的参数，因为在计时器触发之前通常有一个延迟。
### setReturnValue:
&emsp;设置 NSInvocation 对象的返回值。
```c++
- (void)setReturnValue:(void *)retLoc;
```
&emsp;`retLoc`: 一个无类型的内存空间的起始地址，其内容被复制为 NSInvocation 对象的返回值。

&emsp;通常在发送 `invoke` 或 `invokeWithTarget:` 消息时设置此值。
### getReturnValue:
&emsp;获取 NSInvocation 对象的返回值。
```c++
- (void)getReturnValue:(void *)retLoc;
```
&emsp;`retLoc`: NSInvocation 对象将其返回值复制到的无类型的内存空间的起始地址。它应该足够大以容纳该值。

&emsp;使用 NSMethodSignature 的 methodReturnLength 方法确定 `retLoc` 所需的大小：
```c++
NSUInteger length = [[myInvocation methodSignature] methodReturnLength];
buffer = (void *)malloc(length);
[invocation getReturnValue:buffer];
```
&emsp;当返回值是一个对象时，将指针传递到该对象应放入的变量（或内存）：
```c++
id anObject;
NSArray *anArray;

[invocation1 getReturnValue:&anObject];
[invocation2 getReturnValue:&anArray];
```
&emsp;如果从未 `invoke` 过 NSInvocation 对象，则此方法的结果是不确定的。
### invoke
&emsp;将 NSInvocation 对象的消息（带有参数）发送到其 `target` 并设置返回值（setReturnValue:）。
```c++
- (void)invoke;
```
&emsp;你必须先设置 NSInvocation 对象的 `target`、`selector` 和参数值（setArgument:atIndex:），然后才能调用此方法。
### invokeWithTarget:
&emsp;设置 NSInvocation 对象的 `target`，将 NSInvocation 对象的消息（带有参数）发送到该 `target`，并设置返回值（setReturnValue:）。
```c++
- (void)invokeWithTarget:(id)target;
```
&emsp;你必须先设置 NSInvocation 对象的 `selector` 和参数值（setArgument:atIndex:），然后才能调用此方法。
### methodSignature
&emsp;NSInvocation 对象的方法签名。
```c++
@property (readonly, retain) NSMethodSignature *methodSignature;
```
## class_addMethod
&emsp;向具有给定名称和实现的类中添加新方法。
```c++
BOOL class_addMethod(Class cls, SEL name, IMP imp, const char *types);
```
&emsp;`cls`: 要向其添加方法的类。`name`: 一个选择器，指定要添加的方法的名称。`imp`: 该函数是新方法的实现。该函数必须至少包含两个参数 self 和 \_cmd。`types`: 字符数组，描述方法参数和返回值的类型。有关可能的值，请参见 [Objective-C Runtime Programming Guide](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/ObjCRuntimeGuide/Introduction/Introduction.html#//apple_ref/doc/uid/TP40008048) 中的 [Type Encodings](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/ObjCRuntimeGuide/Articles/ocrtTypeEncodings.html#//apple_ref/doc/uid/TP40008048-CH100)。因为函数必须至少有两个参数 self 和 \_cmd，所以第二个和第三个字符必须是 “@:”（第一个字符是返回值类型）。

&emsp;Return Value: 如果成功添加了方法，则为 YES，否则为 NO（例如，该类已经包含具有该名称的方法实现时也会返回 NO）。

&emsp;class_addMethod 将添加超类的实现的重写，但不会替换此类中的现有实现。要更改现有的实现，请使用 method_setImplementation。（`IMP method_setImplementation(Method m, IMP imp);` 设置方法的实现。返回值是该方法的先前实现。）

&emsp;Objective-C 方法只是一个 C 函数，它至少接受两个参数 self 和 \_cmd。例如，给定以下功能：
```c++
void myMethodIMP(id self, SEL _cmd) {
    // implementation ....
}
```
&emsp;你可以像这样将其作为方法动态添加到类中（称为 resolveThisMethodDynamically）：
```c++
class_addMethod([self class], @selector(resolveThisMethodDynamically), (IMP)myMethodIMP, "v@:");
```
## resolveInstanceMethod:
&emsp;动态地为实例方法的给定选择器（sel）提供实现。
```c++
+ (BOOL)resolveInstanceMethod:(SEL)sel OBJC_AVAILABLE(10.5, 2.0, 9.0, 1.0, 2.0);
```
&emsp;`sel`: 要解析（resolve）的选择器（SEL）的名称。

&emsp;Return Value: 如果找到方法并将其添加到 receiver，则为 YES，否则为 NO。（这里可能对 “并将其添加到 receiver” 感到疑惑，即根据 sel 参数在 resolveInstanceMethod: 函数内部为指定的类动态添加（class_addMethod）指定的方法实现。）

&emsp;这个方法和 resolveClassMethod: 允许你为给定的选择器动态地提供一个实现。（一个是实例方法一个是类方法）

&emsp;Objective-C 方法就是一个至少包含两个参数 self 和 \_cmd 的 C 函数。使用 class_addMethod 函数，可以将函数作为方法添加到类中。给定以下函数：
```c++
void dynamicMethodIMP(id self, SEL _cmd) {
    // implementation ....
}
```
&emsp;可以使用 resolveInstanceMethod: 将其作为方法（称为 resolveThisMethodDynamically）动态添加到类中，如下所示：
```c++
+ (BOOL)resolveInstanceMethod:(SEL)aSEL {
    if (aSEL == @selector(resolveThisMethodDynamically)) {
          class_addMethod([self class], aSEL, (IMP)dynamicMethodIMP, "v@:");
          return YES;
    }
    
    return [super resolveInstanceMethod:aSel];
}
```
&emsp;This method is called before the Objective-C forwarding mechanism is invoked. If respondsToSelector: or instancesRespondToSelector: is invoked, the dynamic method resolver is given the opportunity to provide an IMP for the given selector first.

&emsp;在调用 Objective-C 转发机制之前调用此方法。如果调用 respondsToSelector: 或 instancesRespondToSelector:，则动态方法解析器（dynamic method resolver）将有机会首先为给定的选择器提供 IMP。
## resolveClassMethod:
&emsp;为类方法的给定选择器（sel）动态提供实现。
```c++
+ (BOOL)resolveClassMethod:(SEL)sel OBJC_AVAILABLE(10.5, 2.0, 9.0, 1.0, 2.0);
```
&emsp;此方法允许你动态提供给定选择器的实现，同 resolveInstanceMethod: 函数。（为 sel 动态添加实现）
## forwardingTargetForSelector:
&emsp;返回未识别消息应首先指向的对象。（Returns the object to which unrecognized messages should first be directed.）
```c++
- (id)forwardingTargetForSelector:(SEL)aSelector OBJC_AVAILABLE(10.5, 2.0, 9.0, 1.0, 2.0);
```
&emsp;`aSelector`: receiver 未实现的方法的选择器。

&emsp;Return Value: 未识别的消息应首先指向的对象。

&emsp;如果对象实现（或继承）此方法，并返回一个非 nil（非 self）结果，那么返回的对象将被用作新的 receiver 对象，消息调度将恢复到这个新对象（调用返回的新对象的 aSelector 方法）。（显然，如果你从这个方法返回 self，代码将落入一个无限循环。）

&emsp;如果你在非根类（非 NSObject）中实现此方法，如果你的类对于给定的选择器没有要返回的内容，那么你应该返回调用 super 实现的结果（return [super forwardingTargetForSelector:aSelector];）。

&emsp;这种方法使对象有机会在更昂贵的 forwardInvocation: 机制接管之前重定向发送给它的未知消息。当你只想将消息重定向到另一个对象时，这非常有用，并且可以比常规转发快一个数量级。如果转发的目标是捕获 NSInvocation，或者在转发过程中操纵参数或返回值，那么它就没有用了。

&emsp;objc-781 下 NSObject  类的 forwardingTargetForSelector 函数的默认实现，是直接返回 nil。
```c++
+ (id)forwardingTargetForSelector:(SEL)sel {
    return nil;
}

- (id)forwardingTargetForSelector:(SEL)sel {
    return nil;
}
```
## methodSignatureForSelector:
&emsp;返回一个 NSMethodSignature 对象，该对象包含由给定选择器标识的方法的描述。
```c++
- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector OBJC_SWIFT_UNAVAILABLE("");
```
&emsp;`aSelector`: 标识要返回实现地址的方法的选择器。当 receiver 是实例时，aSelector 应该标识实例方法；当 receiver 是类时，aSelector 应该标识类方法。

&emsp;Return Value: 一个 NSMethodSignature 对象，其中包含由 aSelector 标识的方法的描述，如果找不到该方法，则为 nil。

&emsp;该方法用于 protocols 的实现。此方法还用于必须创建 NSInvocation 对象的情况，例如在消息转发期间。如果你的对象维护 delegate 或能够处理它不直接实现的消息，则应该重写此方法以返回适当的方法签名。
## forwardInvocation:
&emsp;被子类重写以将消息转发到其他对象。
```c++
- (void)forwardInvocation:(NSInvocation *)anInvocation OBJC_SWIFT_UNAVAILABLE("");
```
&emsp;当一个对象被发送一条没有相应方法（实现）的消息时，运行时系统给接收者一个机会将消息委托给另一个接收者。它通过创建表示消息的 NSInvocation 对象并向接收方发送 forwardInvocation: 包含此 NSInvocation 对象作为参数的消息来代理消息。然后，接收方的 forwardInvocation: 方法可以选择将消息转发到另一个对象。（如果该对象也不能响应消息，它也将有机会转发消息。）

&emsp;因此，forwardInvocation: 消息允许对象与其他对象建立关系，对于某些消息，这些对象将代替它执行。在某种意义上，转发对象能够 “继承” 它将消息转发到的对象的一些特征。

> &emsp;Important: 要响应对象本身无法识别的方法，除了 forwardInvocation: 之外，还必须重写  methodSignatureForSelector:。转发消息的机制使用从 methodSignatureForSelector: 获取的信息（NSMethodSignature 对象）来创建要转发的 NSInvocation 对象。重写方法必须为给定的选择器提供适当的方法签名，方法可以是预先制定一个方法签名，也可以是向另一个对象请求一个方法签名。

&emsp;forwardInvocation: 方法的实现有两个任务：
+ 定位一个对象，该对象可以响应在 `anInvocation` 中编码的消息。对于所有消息，此对象不必相同。
+ 使用 `anInvocation` 将消息发送到该对象。调用将保存结果，运行时系统将提取该结果并将其传递给原始发送者。

&emsp;在一个简单的情况下，对象只将消息转发到一个目的地（如下面示例中假设的 friend 实例变量），forwardInvocation: 方法可以如下所示：
```c++
- (void)forwardInvocation:(NSInvocation *)invocation {
    SEL aSelector = [invocation selector];
 
    if ([friend respondsToSelector:aSelector])
        [invocation invokeWithTarget:friend];
    else
        [super forwardInvocation:invocation];
}
```
&emsp;转发的消息必须具有固定数量的参数；不支持可变数量的参数（采用 printf() 样式）。

&emsp;转发消息的返回值将返回给原始发件人。所有类型的返回值都可以传递给发送方：id 类型、结构体（structures）、双精度浮点数。

&emsp;forwardInvocation: 方法的实现可以做的不仅仅是转发消息。forwardInvocation: 例如，可以用于合并响应各种不同消息的代码，从而避免了为每个选择器编写单独方法的必要性。forwardInvocation: 方法可能还会在对给定消息的响应中包含其他几个对象，而不是只将其转发给一个对象。

&emsp;NSObject 的 forwardInvocation: 实现只是调用 doesNotRecognizeSelector: 方法；它不转发任何消息。因此，如果选择不实现 forwardInvocation:，则向对象发送无法识别的消息将引发异常。

&emsp;objc-781 下 NSObject 类的 forwardInvocation: 函数的默认实现:
```c++
+ (void)forwardInvocation:(NSInvocation *)invocation {
    [self doesNotRecognizeSelector:(invocation ? [invocation selector] : 0)];
}

- (void)forwardInvocation:(NSInvocation *)invocation {
    [self doesNotRecognizeSelector:(invocation ? [invocation selector] : 0)];
}
```
## doesNotRecognizeSelector:
&emsp;处理 receiver 无法识别的消息。
```c++
- (void)doesNotRecognizeSelector:(SEL)aSelector;
```
&emsp;`aSelector`: 用于标识 receiver 未实现或识别的方法的选择器。

&emsp;每当对象接收到它无法响应或转发的 aSelector 消息时，运行时系统就会调用此方法。此方法反过来引发 NSInvalidArgumentException，并生成错误消息。

&emsp;任何 doesNotRecognizeSelector: 消息通常只由运行时系统发送。但是，可以在程序代码中使用它来防止方法被继承。例如，NSObject 子类可能会放弃 copy 或 init 方法，方法是重新实现它以包含 doesNotRecogniteSelector: 消息，如下所示：
```c++
- (id)copy {
    [self doesNotRecognizeSelector:_cmd];
}
```
&emsp;\_cmd 变量是传递给当前选择器的每个方法的隐藏参数；在本例中，它标识 copy 方法的选择器。此代码防止子类的实例响应 copy 消息，或阻止超类转发 copy 消息，尽管 respondsToSelector: 仍将报告 receiver 有权访问 copy 方法（即 [self respondsToSelector:@selector(copy)] 调用返回 YES）。

&emsp;如果重写此方法，则必须在实现结束时调用 super 或引发 NSInvalidArgumentException 异常。换句话说，这个方法不能正常返回；它必须总是导致抛出异常。

&emsp;objc-781 下 doesNotRecognizeSelector 的实现：
```c++
// Replaced by CF (throws an NSException)
+ (void)doesNotRecognizeSelector:(SEL)sel {
    _objc_fatal("+[%s %s]: unrecognized selector sent to instance %p", 
                class_getName(self), sel_getName(sel), self);
}

// Replaced by CF (throws an NSException)
- (void)doesNotRecognizeSelector:(SEL)sel {
    _objc_fatal("-[%s %s]: unrecognized selector sent to instance %p", 
                object_getClassName(self), sel_getName(sel), self);
}
```

&emsp;好了，大概前置知识就这么多了，下面我们首先学习动态绑定失败后的第一个补救措施：Dynamic Method Resolution（动态方法解析）。
## Dynamic Method Resolution（动态方法解析）
&emsp;本章描述如何动态提供方法的实现。

&emsp;在某些情况下，你可能希望动态地提供方法的实现。例如，Objective-C 声明属性功能（请参阅 [The Objective-C Programming Language](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/ObjectiveC/Chapters/ocProperties.html#//apple_ref/doc/uid/TP30001163-CH17) 中的 [Declared Properties](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/ObjectiveC/Chapters/ocProperties.html#//apple_ref/doc/uid/TP30001163-CH17)）包括的 @dynamic 指令：
```c++
@dynamic propertyName;
```
&emsp;告诉编译器将动态提供与该属性关联的方法。

&emsp;你可以实现 `resolveInstanceMethod:` 和 `resolveClassMethod:` 方法，分别为实例和类方法的给定选择器动态提供实现。Objective-C 方法只是一个 C 函数，它至少接受两个参数 self 和 \_cmd。你可以使用函数 `class_addMethod` 将函数作为方法添加到类中。因此，给出以下功能：
```c++
void dynamicMethodIMP(id self, SEL _cmd) {
    // implementation ....
}
```
&emsp;你可以使用 resolveInstanceMethod: 将 `dynamicMethodIMP` 作为方法动态添加到类中（称为 `resolveThisMethodDynamically`）：
```c++
@implementation MyClass
+ (BOOL)resolveInstanceMethod:(SEL)aSEL {
    if (aSEL == @selector(resolveThisMethodDynamically)) {
          class_addMethod([self class], aSEL, (IMP) dynamicMethodIMP, "v@:");
          return YES;
    }
    
    return [super resolveInstanceMethod:aSEL];
}
@end
```
&emsp;转发方法（如 Message Forwarding 中所述）和动态方法解析在很大程度上是正交的。类有机会在转发机制启动之前动态解析方法。如果调用了 `respondsToSelector:` 或 `instancesRespondToSelector:`，则动态方法解析器将有机会首先为选择器提供 IMP。如果你实现 `resolveInstanceMethod:` 但希望特定的选择器实际通过转发机制转发，那么你需要为这些选择器返回 NO。

### Dynamic Loading
&emsp;
&emsp;Objective-C 程序可以在运行时加载和链接新的类和类别。新的代码被合并到程序中，并与一开始加载的类和类别进行相同的处理。动态加载可以用来做很多不同的事情。例如，系统偏好设置应用程序中的各个模块是动态加载的。

> &emsp;下面是原文，暂时看不懂是什么意思。 
>
> &emsp;In the Cocoa environment, dynamic loading is commonly used to allow applications to be customized. Others can write modules that your program loads at runtime—much as Interface Builder loads custom palettes and the OS X System Preferences application loads custom preference modules. The loadable modules extend what your application can do. They contribute to it in ways that you permit but could not have anticipated or defined yourself. You provide the framework, but others provide the code.
> 
> &emsp;Although there is a runtime function that performs dynamic loading of Objective-C modules in Mach-O files (objc_loadModules, defined in objc/objc-load.h), Cocoa’s NSBundle class provides a significantly more convenient interface for dynamic loading—one that’s object-oriented and integrated with related services. See the NSBundle class specification in the Foundation framework reference for information on the NSBundle class and its use. See OS X ABI Mach-O File Format Reference for information on Mach-O files.

&emsp;上面是动态方法解析相关的知识点，如果动态方法解析无法补救时，继续走下一个流程：Fast forwarding 快速转发阶段（如果可以的话返回一个备用响应对象）和 Normal forwarding 常规转发阶段（完整的消息转发）。

## Message Forwarding
&emsp;向不处理该消息的对象发送消息是错误的。但是，在宣布错误之前，运行时系统会给接收对象第二次处理消息的机会。
### Forwarding
&emsp;如果向不处理该消息的对象发送消息，则在宣布错误之前，运行时将向该对象发送 forwardInvocation: 消息，其中 NSInvocation 对象作为其唯一参数。NSInvocation 对象将封装原始消息及其传递的参数。

&emsp;你可以实现 forwardInvocation: 方法，以对消息提供默认响应，或以其他方式避免错误。顾名思义，forwardInvocation: 通常用于将消息转发到另一个对象。

&emsp;要查看转发的范围和意图，请设想以下场景：首先，假设你正在设计一个对象，该对象可以响应一个名为 negotiate 的消息，并且你希望它的响应包含另一类型对象的响应。你可以通过将 negotiate 消息传递给你实现的 negotiate 方法体中的其他对象来轻松完成这一点。

&emsp;更进一步，假设你希望对象对 negotiate 消息的响应正好是在另一个类中实现的响应。实现这一点的一种方法是使你的类从另一个类继承方法。然而，这样安排事情可能是不可能的。你的类和实现 negotiate 的类位于继承层次结构的不同分支中可能是一个很好的原因。

&emsp;即使你的类无法继承 negotiate 方法，你仍然可以通过实现该方法的版本来 “借用” 该方法，该方法将消息简单地传递到另一个类的实例：
```c++
- (id)negotiate {
    if ( [someOtherObject respondsTo:@selector(negotiate)] )
        return [someOtherObject negotiate];
    return self;
}
```
&emsp;这种处理方式可能会有些麻烦，尤其是当你希望将对象传递给另一个对象的消息很多时。你必须实现一种方法，才能涵盖你想从另一类中借用的每种方法。此外，在编写代码时，如果你不知道可能要转发的全部消息，则不可能处理这些情况。该集合可能取决于运行时的事件，并且随着将来实现新的方法和类而可能会更改。

&emsp;forwardInvocation: 消息提供的第二种机会是针对此问题的临时解决方案，它是动态的而不是静态的。它的工作方式如下：当对象由于没有与消息中的选择器匹配的方法而无法响应消息时，运行时系统会通过向其发送 forwardInvocation: 消息来通知对象。每个对象都从 NSObject 类继承了 forwardInvocation: 方法。但是，该方法的 NSObject 版本仅调用 dosNotRecognizeSelector:。通过覆盖 NSObject 的版本并实现自己的版本，你可以利用 forwardInvocation: 消息提供的机会将消息转发给其他对象。

&emsp;要转发消息，所有 forwardInvocation: 方法需要做的是：

+ 确定消息应该放在哪里，然后
+ 把它和它的原始参数一起发送到那里。

&emsp;可以使用 NSInvocation 类的 invokeWithTarget: 方法发送消息：
```c++
- (void)forwardInvocation:(NSInvocation *)anInvocation {
    if ([someOtherObject respondsToSelector:[anInvocation selector]])
        [anInvocation invokeWithTarget:someOtherObject];
    else
        [super forwardInvocation:anInvocation];
}
```
&emsp;转发的消息的返回值将返回给原始发件人（original sender）。所有类型的返回值都可以传递给发送方，包括 id、结构（struct）和双精度浮点数（double-precision floating-point numbers）。

&emsp;forwardInvocation: 方法可以充当未识别消息的分发中心，将它们分发给不同的接收者。或者它可以是一个中转站，将所有消息发送到同一个目的地。它可以将一条消息转换成另一条消息，或者简单地 “吞下（swallow）” 一些消息，这样就没有响应也没有错误。forwardInvocation: 方法还可以将多个消息合并到单个响应中。forwardInvocation: 做什么取决于实现者。然而，它提供了在转发链中链接对象的机会，为程序设计开辟了可能性。

> &emsp;Note: forwardInvocation: 方法仅在消息不调用名义上的接收器中的现有方法时才获得处理消息的权限。例如，如果你希望你的对象将 negotiate 消息转发给另一个对象，则它不能有自己的 negotiate 方法。如果是这样的话，消息就永远不会到达 forwardInvocation:。

&emsp;有关转发（forwarding）和调用（invocations）的更多信息，请参阅 Foundation framework reference 中的 NSInvocation 类规范。
### Forwarding and Multiple Inheritance（转发和多重继承）
&emsp;转发模仿了继承，可用于将多重继承的某些效果借给 Objective-C 程序。如图 5-1 所示，通过转发消息来响应消息的对象似乎借用或 “继承” 了另一个类中定义的方法实现。

&emsp;Figure 5-1  Forwarding

![forwarding](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/92953dc970564a689d91459a5a69d05b~tplv-k3u1fbpfcp-watermark.image)

&emsp;在此插图中，Warrior 类的实例将 negotiate 消息转发到 Diplomat 类的实例。 Warrior 看起来像 Diplomat 一样进行 Warrior。它似乎对 Warrior 的消息作出了回应，并且出于所有实际目的，它的确作出了回应（尽管实际上是 Diplomat 在做这项工作）。

&emsp;转发消息的对象从继承层次结构的两个分支（其自己的分支以及响应消息的对象的分支）“继承” 了方法。在上面的示例中，Warrior 类似乎继承自 Diplomat 及其自己的超类。

&emsp;转发提供了你通常希望从多重继承中获得的大部分功能。然而，两者之间有一个重要的区别：多重继承在单个对象中结合了不同的功能。它趋向于大的、多方面的对象。另一方面，转发将不同的责任分配给不同的对象。它将问题分解为更小的对象，但以对消息发送者透明的方式关联这些对象。
### Surrogate Objects（替代对象）
&emsp;转发不仅可以模拟多重继承，还可以开发表示或 “覆盖” 更重要对象的轻量级对象。代理代表另一个对象，并向其发送消息。

&emsp;在 [The Objective-C Programming Language](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/ObjectiveC/Introduction/introObjectiveC.html#//apple_ref/doc/uid/TP30001163) 的 “远程消息传递（Remote Messaging）” 中讨论的代理就是这样的代理。代理负责将消息转发到远程接收者的管理细节，确保通过连接复制和检索参数值，依此类推。但是它并没有尝试做更多的事情；它不会复制远程对象的功能，而只是为远程对象提供一个本地地址，该地址可以在另一个应用程序中接收消息。

&emsp;其他类型的代理对象也是可能的。例如，假设你有一个处理大量数据的对象，也许它创建了一个复杂的映像或读取了磁盘上文件的内容。设置该对象可能会很耗时，因此你更喜欢在确实需要时或系统资源暂时空闲时以懒加载的方式进行操作。同时，此对象至少需要一个占位符，以使应用程序中的其他对象正常运行。

&emsp;在这种情况下，你可以首先创建一个轻量级的代理对象，而不是完整的对象。这个对象可以自己做一些事情，比如回答有关数据的问题，但大多数情况下，它只会为较大的对象保留一个位置，并在时机成熟时向它转发消息。当代理项的 forwardInvocation: 方法首先接收到一条指向另一个对象的消息时，它将确保该对象存在，如果该对象不存在，它将创建它。较大对象的所有消息都经过代理项，因此，就程序的其余部分而言，代理项和较大对象将是相同的。
### Forwarding and Inheritance
&emsp;尽管转发模仿继承，但 NSObject 类从不混淆两者。`respondsToSelector:` 和 `isKindOfClass:` 等方法只查看继承层次结构，而不查看转发链。例如，如果询问 Warrior 对象是否响应 negotiate 消息，
```c++
if ( [aWarrior respondsToSelector:@selector(negotiate)] )
...
```
&emsp;答案是 NO，即使它可以毫无错误地接收 negotiate 消息并在某种意义上将它们转发给 Diplomat 来响应它们。

&emsp;在许多情况下，NO 是正确的答案。但事实并非如此。如果使用转发来设置代理对象或扩展类的功能，则转发机制应该与继承一样透明。如果你希望你的对象像他们真正继承了转发消息的对象的行为一样工作，则需要重新实现 `responsToSelector:` 和 `isKindOfClass:` 方法以包括你的转发算法：
```c++
- (BOOL)respondsToSelector:(SEL)aSelector {
    if ( [super respondsToSelector:aSelector] )
        return YES;
    else {
        /* Here, test whether the aSelector message can     *
         * be forwarded to another object and whether that  *
         * object can respond to it. Return YES if it can.  */
         
        // 在这里，测试 aSelector 消息是否可以转发到另一个对象以及该对象是否可以响应。如果可以，则返回 YES。
        
    }
    return NO;
}
```
&emsp;除了 `responsesToSelector:` 和 `isKindOfClass:` 外，`instancesRespondToSelector:` 方法还应镜像转发算法。如果使用协议，则应该将 `conformsToProtocol:` 方法同样添加到列表中。同样，如果对象转发其收到的任何远程消息，则它应具有 `methodSignatureForSelector:` 的版本， 该函数可以返回对最终响应所转发消息的方法的准确描述。例如，如果对象能够将消息转发到其代理，则可以实现 `methodSignatureForSelector:` 如下：
```c++
- (NSMethodSignature*)methodSignatureForSelector:(SEL)selector{
    NSMethodSignature* signature = [super methodSignatureForSelector:selector];
    if (!signature) {
       signature = [surrogate methodSignatureForSelector:selector];
    }
    return signature;
}
```
&emsp;你可以考虑将转发算法放在私有代码中，并使用所有这些方法 forwardInvocation: 进行调用。

> &emsp;Note: 这是一种先进的技术，只适用于没有其他解决方案的情况。它不是用来代替继承的。如果必须使用此技术，请确保完全了解进行转发的类和要转发到的类的行为。

&emsp;本部分提到的方法在 Foundation framework reference 的 NSObject 类规范中进行了描述。有关 invokeWithTarget: 的信息，请参见 Foundation framework reference 中的 NSInvocation 类规范。

&emsp;以上就是 Message Forwarding 文档的全部内容了。

## Using NSInvocation
&emsp;创建 NSInvocation 对象需要几个步骤。考虑自定义类 MyCalendar 的以下方法：
```c++
– (BOOL)updateAppointmentsForDate:(NSDate *)aDate
```
&emsp;`updateAppointmentsForDate:` 将 NSDate 对象作为其唯一参数，并根据是否可以更新 appointments 而不会发生冲突而返回 YES 或 NO。以下代码片段为其设置了一个 NSInvocation 对象：
```c++
SEL theSelector;
NSMethodSignature *aSignature;
NSInvocation *anInvocation;
 
theSelector = @selector(updateAppointmentsForDate:);
aSignature = [MyCalendar instanceMethodSignatureForSelector:theSelector];
anInvocation = [NSInvocation invocationWithMethodSignature:aSignature];
[anInvocation setSelector:theSelector];
```
&emsp;前两行获取 `updateAppointmentsForDate:` 方法的 NSMethodSignature 对象。最后两行实际上是创建 NSInvocation 对象并设置其选择器。请注意，可以将选择器设置为与 `updateAppointmentsForDate:` 签名匹配的任何选择器。这些方法均可以与 `anInvocation` 一起使用：（返回值和参数类型完全一样即可（方法签名完全一样即可））
```c++
– (BOOL)clearAppointmentsForDate:(NSDate *)aDate
– (BOOL)isAvailableOnDate:(NSDate *)aDate
– (BOOL)setMeetingTime:(NSDate *)aDate
```
&emsp;在调度之前，必须设置 `anInvocation` 的 `target` 和参数：
```c++
MyCalendar *userDatebook;    /* Assume this exists. */
NSDate *todaysDate;          /* Assume this exists. */
 
[anInvocation setTarget:userDatebook];
[anInvocation setArgument:&todaysDate atIndex:2]; // index 从 2 开始
```
&emsp;`setArgument:atIndex:` 将指定的参数设置为提供的值。每个方法都有两个隐藏参数，即 `target` 和 `selector`（其索引分别为 0 和 1），因此实际上需要设置的第一个参数位于索引 2。在这种情况下，`todaysDate` 是 `updateAppointmentsForDate:` 的 NSDate 参数。

&emsp;要调度 NSInvocation 对象，请发送 `invoke` 或 `invokeWithTarget:` 消息。仅当 NSInvocation 对象具有 `target` 时，`invoke` 才会产生结果。调度后，NSInvocation 对象包含消息的返回值，`getReturnValue:` 将生成:
```c++
BOOL result;
 
[anInvocation invoke];
[anInvocation getReturnValue:&result];
```
&emsp;NSInvocation 不支持使用可变数量的参数或 union 参数的方法调用。
### Saving NSInvocation Objects for Later Use（保存 NSInvocation 对象以备后用）
&emsp;由于 NSInvocation 对象并不总是需要保留其参数，因此默认情况下它不需要这样做。如果它们自动释放，则可能导致对象参数和 `target` 无效。如果计划缓存 NSInvocation 对象或在应用程序执行期间重复调度它，则应向其发送一个 `retainArguments` 消息。此方法保留 `target` 和所有对象参数，并复制 C 字符串，以使它们不会丢失，因为另一个对象释放了它们。
### Using NSInvocation Objects with Timers（将 NSInvocation 对象与计时器一起使用）
&emsp;假设上面创建的 NSInvocation 对象正在时间管理应用程序中使用，该应用程序允许多个用户为其他人（例如小组会议）设置约会。此应用程序可能允许每隔几分钟自动更新用户的日历，以便用户始终知道他们的日程安排。这种自动更新可以通过使用 NSInvocation 对象设置 NSTimer 对象来实现。

&emsp;给定上面的 NSInvocation 对象，这就像调用一个 NSTimer 方法一样简单：
```c++
[NSTimer scheduledTimerWithInterval:600 invocation:anInvocation repeats:YES];
```
&emsp;这行代码设置了一个 NSTimer 对象，该对象每 10 分钟（600 秒）发送一次 InInvocation。请注意，NSTimer 对象始终指示其 NSInvocation 对象保留其参数。因此，你不需要自己发送 keepArguments。有关计时器的更多信息，请参见 [Timer Programming Topics](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/Timers/Timers.html#//apple_ref/doc/uid/10000061i) 。

## 参考链接
**参考链接:🔗**
+ [Selector](https://developer.apple.com/library/archive/documentation/General/Conceptual/DevPedia-CocoaCore/Selector.html#//apple_ref/doc/uid/TP40008195-CH48)
+ [Type Encodings](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/ObjCRuntimeGuide/Articles/ocrtTypeEncodings.html#//apple_ref/doc/uid/TP40008048-CH100)
+ [Messaging](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/ObjCRuntimeGuide/Articles/ocrtHowMessagingWorks.html#//apple_ref/doc/uid/TP40008048-CH104-SW1)
+ [Dynamic Method Resolution](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/ObjCRuntimeGuide/Articles/ocrtDynamicResolution.html#//apple_ref/doc/uid/TP40008048-CH102-SW1)
+ [Message Forwarding](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/ObjCRuntimeGuide/Articles/ocrtForwarding.html#//apple_ref/doc/uid/TP40008048-CH105-SW1)
+ [Using NSInvocation](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/DistrObjects/Tasks/invocations.html#//apple_ref/doc/uid/20000744-CJBBACJH)
