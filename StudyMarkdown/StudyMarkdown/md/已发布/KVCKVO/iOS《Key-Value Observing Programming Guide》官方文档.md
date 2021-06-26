# iOS KVO 官方文档《Key-Value Observing Programming Guide》中文翻译

> &emsp;日常开发中我们可能已经非常习惯于使用 KVO 来监听对象的某个属性，不过可能有一些细节我们未深入过，那么下面一起通过官方文档来全面的学习一下 KVO 吧！⛽️⛽️

## Introduction to Key-Value Observing Programming Guide（键值观察编程指南简介）
&emsp;键值观察是一种机制，它允许将其他对象的指定属性的更改通知给对象。

> &emsp;Important: 为了了解 key-value observing，你必须首先了解 key-value coding。

### At a Glance（简介）
&emsp;键值观察提供了一种机制，该机制允许将其他对象的特定属性的更改通知给对象。对于应用程序中模型层和控制器层（model and controller layers）之间的通信特别有用。 （在 OS X 中，控制器层（controller layer）绑定技术在很大程度上依赖于键值观察。）控制器对象通常观察模型对象的属性，而视图对象通过控制器观察模型对象的属性。但是，此外，模型对象可能会观察其他模型对象（通常是确定从属值何时更改）甚至是自身观察自己（再次确定从属值何时更改）。

&emsp;你可以观察到包括简单属性（attributes）、一对一关系（ to-one relationships）和一对多关系（to-many relationships）的属性。一对多关系的观察者被告知所做更改的类型，以及更改涉及哪些对象。

&emsp;以下示例代码摘自上篇 KVC 中的，分别表示了不同类型的属性。
```c++
@interface BankAccount : NSObject
 
@property (nonatomic) NSNumber* currentBalance;              // An attribute 属性
@property (nonatomic) Person* owner;                         // A to-one relation 一对一关系
@property (nonatomic) NSArray< Transaction* >* transactions; // A to-many relation 一对多关系

@end
```
&emsp;一个简单的示例说明了 KVO 如何在你的应用程序中有用。假设一个 Person 对象与一个 Account 对象进行交互，该 Account 对象代表该人在银行的储蓄帐户。 Person 实例可能需要了解 Account 实例的某些方面何时更改，例如余额（balance）或利率（interestRate）。

![kvo_objects_properties](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/ab9e12bd0d1d49ec8ddd04cd1bc96bda~tplv-k3u1fbpfcp-watermark.image)

&emsp;如果这些属性是 Account 的公共属性，则该 Person 可以定期轮询 Account 以发现其变化，但这当然效率低下，并且通常不切实际。更好的方法是使用 KVO，这类似于在 Account 发生变化时 Person 接收到中断（interrupt ）。

&emsp;要使用 KVO，首先必须确保观察到的对象（在这种情况下为 Account）符合 KVO。通常，如果你的对象继承自 NSObject 并以通常的方式创建属性，则对象及其属性将自动符合 KVO 要求。也可以手动实现合规性。 KVO Compliance 描述了自动和手动键值观察之间的区别，以及如何实现两者。

&emsp;接下来，你必须注册你的 observer instance - Person 和 observed instance - Account。 Person sends an addObserver:forKeyPath:options:context: message to the Account, once for each observed key path, naming itself as the observer

![kvo_objects_add](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/f1914af9c83540b7b2359d6f307a1d16~tplv-k3u1fbpfcp-watermark.image)

&emsp;为了从 Account 接收更改通知，Person 实现了所有观察者（all observers）都必需实现的 observeValueForKeyPath:ofObject:change:context: 方法。每当注册的键路径之一发生更改时，Account 就会将此消息发送给 Person。然后，Person 可以根据更改通知采取适当的措施。

![kvo_objects_observe](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/ffbf4bbac70a4691bc83d972b64dff0b~tplv-k3u1fbpfcp-watermark.image)

&emsp;最后，当不再需要通知时，至少在释放之前，Person 实例必须通过向 Account 发送 removeObserver:forKeyPath: 消息来取消注册。

![kvo_objects_remove](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/3cc232de99304f65bc0d9fceb1f6951a~tplv-k3u1fbpfcp-watermark.image)

&emsp;Registering for Key-Value Observing 描述了注册、接收和取消注册键值观察通知的整个生命周期。

&emsp;KVO 的主要优点是，你不必实现自己的方案来在每次属性更改时发送通知。它定义良好（well-defined）的基础结构（infrastructure）具有框架级别（framework-level）的支持，使其易于采用，通常你不必向项目中添加任何代码。此外，基础结构（infrastructure）已经具备了完整的功能（full-featured），这使得支持单个属性的多个观察者以及依赖值变得很容易。

&emsp;与使用 NSNotificationCenter 的通知不同，没有 central object 为所有观察者（observers）提供更改通知。而是在进行更改时将通知直接发送到观察对象。 NSObject 提供了键值观察的基本实现，因此你几乎不需要重写这些方法。

&emsp;Key-Value Observing Implementation Details  描述了键值观察的实现方式。

&emsp;同上篇的 KVC 一样，KVO 相关的代码都声明在 NSKeyValueObserving.h 文件中，在 NSKeyValueObserving.h 文件内部同样是采用 Category 机制，把不同的函数定义在不同的 NSObject Category 中，如 NSObject + NSKeyValueObserving 中声明了所有观察者都必须实现的 -observeValueForKeyPath:ofObject:change:context: 通知函数，NSObject + NSKeyValueObserverRegistration 中声明了注册观察者和移除观察者的函数，然后 NSArray<ObjectType> + NSKeyValueObserverRegistration、NSOrderedSet<ObjectType> + NSKeyValueObserverRegistration、NSSet<ObjectType> + NSKeyValueObserverRegistration 则是分别针对 NSArray、NSOrderedSet、NSSet 类重写 KVO 相关的函数，以及新增一些 KVO 相关函数。

&emsp;那么这里我们先看一下 NSKeyValueObserving.h 的内容，然后再接着看 Key-Value Observing Programming Guide 文档。
### NSKeyValueObserving
&emsp;对象采用的一种非正式协议，用于将其他对象的指定属性的更改通知给对象。

&emsp;你可以观察到任何对象属性，包括简单属性，一对一关系和一对多关系。一对多关系的观察者被告知所做更改的类型，以及更改涉及哪些对象。

&emsp;NSObject 提供了 NSKeyValueObserving 协议的实现，该协议为所有对象提供了自动观察功能。你可以通过禁用自动观察者通知并使用此协议中的方法实现手动通知来进一步优化通知。
#### observeValueForKeyPath:ofObject:change:context:
&emsp;当相对于被观察者对象的指定键路径上的值已更改（注意这里是发生更改以后）时，通知观察者对象。
```c++
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSKeyValueChangeKey, id> *)change context:(void *)context;
```
&emsp;`keyPath`: 相对于对象的已更改值的键路径。`object`: 键路径 `keyPath` 的源对象。`change`: 一个字典，描述相对于对象的键路径 `keyPath` 上的属性值所做的更改。条目在 Change Dictionary Keys 中描述。`context`: 注册观察者以接收键值观察通知时提供的值。

&emsp;为了使对象开始在 keyPath 处发送该值的更改通知消息，可以向其发送 addObserver:forKeyPath:options:context:  消息，并命名应接收该消息的观察者对象。当你完成观察时，尤其是在释放观察者对象之前，你会向被观察者对象发送一个 removeObserver:forKeyPath: 或 removeObserver:forKeyPath:context: 取消注册观察者并停止发送更改通知消息的消息。

&emsp;假设接收者已经在相对于对象的关键路径上注册为该值的观察者，则应通知该值更改。

&emsp;更改字典始终包含一个 NSKeyValueChangeKindKey 条目，其值是包装 NSKeyValueChange 的 NSNumber（使用 -[NSNumber unsignedIntegerValue]）。NSKeyValueChange 的含义取决于键路径标识的属性类型：

+ 对于任何类型的属性（属性，一对一关系，有序或无序一对多关系），NSKeyValueChangeSetting 指示观察到的对象已收到 -setValue:forKey: 消息，或者表示与键值编码兼容的 set 方法用于键已被调用，或者 -willChangeValueForKey:/-didChangeValueForKey: 对已被调用。
+ 对于一对多关系，NSKeyValueChangeInsertion、NSKeyValueChangeRemoval 和 NSKeyValueChangeReplacement 表示已将变异消息发送到由 -mutableArrayValueForKey: 消息发送给对象返回的数组，或发送到由 -mutableOrderedSetValueForKey: 消息返回的有序集合。发送到对象，或者已经调用了符合键值编码的数组或键的有序集合突变方法之一，或者 -willChange:valuesAtIndexes:forKey:/-didChange:valuesAtIndexes:forKey: 对具有否则被调用。
+ 对于一对多关系（在 Mac OS 10.4 中引入），NSKeyValueChangeInsertion、NSKeyValueChangeRemoval 和 NSKeyValueChangeReplacement 表示已将变异消息发送到由 -mutableSetValueForKey: 消息已发送至对象返回的集合，或该键之一已调用了符合键值的 -value 编码的设置突变方法，或者已另外调用了-willChangeValueForKey:withSetMutation:usingObjects:/-ddChangeValueForKey:withSetMutation:usingObjects:对。

&emsp;对于任何类型的属性，如果在观察者注册时指定了 NSKeyValueObservingOptionNew，则更改字典将包含一个 NSKeyValueChangeNewKey 条目，这是正确的更改，并且这不是事先通知。如果指定了 NSKeyValueObservingOptionOld，则更改字典包含一个 NSKeyValueChangeOldKey，这是正确的更改。有关这些条目的值，请参见 NSKeyValueObserverNotification 非正式协议方法的注释。

&emsp;对于一对多关系，更改字典始终包含一个 NSKeyValueChangeIndexesKey 条目，其值是一个 NSIndexSet，其中包含插入、移除或替换的对象的索引，除非更改是 NSKeyValueChangeSetting。

&emsp;如果在观察者注册时指定了 NSKeyValueObservingOptionPrior（在Mac OS 10.5中引入），并且此通知是在更改之前发送的通知，因此更改字典包含一个 NSKeyValueChangeNotificationIsPriorKey 条目，其值是 NSNumber 包装为 YES（使用 -[NSNumber boolValue]）。

&emsp;context 始终与在观察者注册时传递的指针相同。

&emsp;NSObject + NSKeyValueObserverRegistration 分类：在相对于接收者的关键路径上，以值的观察者身份注册或注销。这些选项确定观察者通知中包括的内容以及何时发送它们，如上所述，并且如上所述，上下文在观察者通知中传递上下文。你应该尽可能使用 -removeObserver:forKeyPath:context: 而不是 -removeObserver:forKeyPath:，因为它可以让你更精确地指定意图。当同一观察者多次注册相同的键路径，但每次都使用不同的上下文指针时，-removeObserver:forKeyPath: 在确定要删除的对象时必须猜测上下文指针，并且可能会猜错。

#### addObserver:forKeyPath:options:context:
&emsp;注册观察者对象以接收相对于接收此消息的对象的键路径的 KVO 通知。
```c++
- (void)addObserver:(NSObject *)observer forKeyPath:(NSString *)keyPath options:(NSKeyValueObservingOptions)options context:(nullable void *)context;
```
&emsp;`observer`: 注册 KVO 通知的对象。观察者（`observer`）必须实现键值观察方法 observeValueForKeyPath:ofObject:change:context:。`keyPath`: 要观察的属性相对于接收此消息的对象的键路径。此值不能为 nil。`options`: NSKeyValueObservingOptions 值的组合，用于指定观察通知中包括的内容。有关可能的值，请参见 NSKeyValueObservingOptions。`context`: 在 observeValueForKeyPath:ofObject:change:context: 中传递给观察者的任意数据。

&emsp;既不保留接收此消息的对象，也不保留观察者。调用此方法的对象最终还必须调用 removeObserver:forKeyPath: 或 removeObserver:forKeyPath:context: 方法，以在参与 KVO（participating in KVO）时注销观察者。
#### removeObserver:forKeyPath:
&emsp;停止观察者对象接收由相对于接收此消息的对象的键路径指定的属性的更改通知。
```c++
- (void)removeObserver:(NSObject *)observer forKeyPath:(NSString *)keyPath;
```
&emsp;`observer`: 要作为观察者移除的对象。`keyPath`: 相对于接收此消息的对象的键路径，已为其注册观察者以接收 KVO 更改通知。

&emsp;为先前尚未注册为观察者的对象调用 removeObserver:forKeyPath: 是错误的。在释放在 addObserver:forKeyPath:options:context: 中指定的任何对象之前，请确保调用此方法（或 removeObserver:forKeyPath:context:）。
#### removeObserver:forKeyPath:context:
&emsp;在给定上下文的情况下，停止观察者对象接收由键路径相对于接收此消息的对象相对于键路径指定的属性的更改通知。
```c++
- (void)removeObserver:(NSObject *)observer forKeyPath:(NSString *)keyPath context:(nullable void *)context API_AVAILABLE(macos(10.7), ios(5.0), watchos(2.0), tvos(9.0));
```
&emsp;`observer`: 要作为观察者移除的对象。`keyPath`: 相对于被观察者对象的键路径，已为其注册观察者以接收 KVO 更改通知。`context`: 更具体地标识要删除的观察者的任意数据。（任意数据是指 context 注册时可以是一个指向任意地址的指针）

&emsp;通过检查上下文中的值，你可以精确确定使用哪个 addObserver:forKeyPath:options:context: 调用创建观察关系。当同一观察者多次为同一键路径注册但使用不同的上下文指针时，应用程序可以专门确定停止哪个观察者对象。如果对象尚未注册为观察者却去调用 removeObserver:forKeyPath:context: 函数则是一个错误。

&emsp;在释放在 addObserver:forKeyPath:options:context: 中指定的任何对象之前，请确保调用此方法（或 removeObserver:forKeyPath:）。

&emsp;NSObject + NSKeyValueObserverNotification 分类：给定一个标识属性的键（属性，一对一关系或有序或无序一对多关系），向每个为该键注册的观察者发送 -observeValueForKeyPath:ofObject:change:context: 类型为 NSKeyValueChangeSetting 的通知消息使用其他在对象中找到键值的键路径向其他对象注册的对象。这些方法的调用必须始终配对。

&emsp;如果在观察者注册时要求使用这些方法，则通知中的更改字典将包含可选条目：
+ NSKeyValueChangeOldKey 条目（如果存在）包含调用 -willChangeValueForKey: 时由 -valueForKey: 返回的值（如果 -valueForKey: 返回 nil，则为 NSNull）。
+ NSKeyValueChangeNewKey 条目（如果存在）包含在调用 -didChangeValueForKey: 的瞬间由 -valueForKey: 返回的值（如果 -valueForKey: 返回 nil，则为 NSNull）。

#### willChangeValueForKey:
&emsp;通知被观察者对象给定属性的值即将更改。
```c++
- (void)willChangeValueForKey:(NSString *)key;
```
&emsp;`key`: 值将更改的属性的名称。

&emsp;手动实现键值观察者遵从性时，请使用此方法来通知被观察者对象 `key` 对应的属性的值即将更改。

&emsp;此方法的更改类型为 NSKeyValueChangeSetting。（NSKeyValueChangeSetting 下面会详细分析）

> &emsp;Important: 更改值后，必须使用相同的参数（key）调用相应的 didChangeValueForKey:。

&emsp;**你很少需要在子类中重写此方法，但是如果这样做，请务必调用 super。**

#### didChangeValueForKey:
&emsp;通知被观察者对象给定属性的值已更改。
```c++
- (void)didChangeValueForKey:(NSString *)key;
```
&emsp;`key`: 值已发生更改的属性的名称。

&emsp;手动实现键值观察者遵从性时，请使用此方法来通知被观察者对象 `key` 对应的属性的值刚刚更改。始终将此方法的调用与对 willChangeValueForKey: 的调用配对。

&emsp;**你很少需要在子类中重写此方法，但是如果这样做，请确保调用 super。**

&emsp;给定一个标识一对多关系的键，将 -observeValueForKeyPath:ofObject:change:context: 传入的更改类型的通知消息发送给每个为该键注册的观察者，包括那些使用键路径向其他对象注册的观察者在此对象中找到键值。传入的类型必须为 NSKeyValueChangeInsertion、NSKeyValueChangeRemoval 或 NSKeyValueChangeReplacement。传入的索引集必须是要插入、删除或替换的对象的索引。这些方法的调用必须始终与相同的参数配对。

&emsp;如果在观察者注册时要求使用这些方法，则通知中的更改字典将包含可选条目：
+ 如果存在 NSKeyValueChangeOldKey 条目（仅适用于 NSKeyValueChangeRemoval 和 NSKeyValueChangeReplacement），则在调用 -willChangeValueForKey:valuesAtIndexes:forKey: 的那一刻，包含由 -valueForKey: 返回的数组中的索引对象数组。
+ 如果存在 NSKeyValueChangeNewKey 条目（仅适用于 NSKeyValueChangeInsertion 和 NSKeyValueChangeReplacement），则在调用 -didChangeValueForKey:valuesAtIndexes:forKey: 的瞬间，该数组将包含由 -valueForKey: 返回的数组中的索引对象。

#### willChange:valuesAtIndexes:forKey:
&emsp;通知被观察者对象，对于指定的有序一对多关系，将在给定的索引处执行指定的更改。
```c++
- (void)willChange:(NSKeyValueChange)changeKind valuesAtIndexes:(NSIndexSet *)indexes forKey:(NSString *)key;
```
&emsp;`change`: 即将进行的更改的类型。`indexes`: 更改将影响的一对多关系的索引。`key`: 有序一对多关系的属性的名称。

&emsp;手动实现键值观察合规性时，请使用此方法。

> &emsp;Important: 更改值后，必须使用相同的参数调用相应的 didChange:valuesAtIndexes:forKey:。

&emsp;**你很少需要在子类中重写此方法，但是如果这样做，请确保调用 super。**

#### didChange:valuesAtIndexes:forKey:
&emsp;通知被观察者对象，指定的一对多关系在索引上发生了指定的更改。
```c++
- (void)didChange:(NSKeyValueChange)changeKind valuesAtIndexes:(NSIndexSet *)indexes forKey:(NSString *)key;
```
&emsp;`change`: 所做更改的类型。`indexes`: 受更改影响的一对多关系的索引。`key`: 有序一对多关系的属性的名称。

&emsp;手动实现键值观察合规性时，请使用此方法。

&emsp;**你很少需要在子类中重写此方法，但是如果这样做，请确保调用 super。始终将此方法的调用与对 willChange:valuesAtIndexes:forKey: 的调用配对。**

&emsp;给定一个标识无序一对多关系的键，向每个为该键注册的观察者发送 -observeValueForKeyPath:ofObject:change:context:通知消息，包括那些使用其他在此对象中查找键值的键路径向其他对象注册的通知消息。目的。传入的突变类型对应于 NSMutableSet 方法。传入的集合必须包含将传递给相应的 NSMutableSet 方法的集合。这些方法的调用必须始终与相同的参数配对。

&emsp;使用这些方法导致的通知中更改字典中 NSKeyValueChangeKindKey 条目的值取决于传入的 mutationKind 值：

+ NSKeyValueUnionSetMutation -> NSKeyValueChangeInsertion
+ NSKeyValueMinusSetMutation -> NSKeyValueChangeRemoval
+ NSKeyValueIntersectSetMutation -> NSKeyValueChangeRemoval
+ NSKeyValueSetSetMutation -> NSKeyValueChangeReplacement

&emsp;更改字典可能还包含可选条目:
+ NSKeyValueChangeOldKey 条目（如果存在）（仅适用于 NSKeyValueChangeRemoval 和 NSKeyValueChangeReplacement）包含已删除的对象集。
+ NSKeyValueChangeNewKey 条目（如果存在）（仅用于 NSKeyValueChangeInsertion 和 NSKeyValueChangeReplacement）包含所添加的对象集。

#### willChangeValueForKey:withSetMutation:usingObjects:
&emsp;通知被观察者对象即将对指定的无序一对多关系进行指定的更改。
```c++
- (void)willChangeValueForKey:(NSString *)key withSetMutation:(NSKeyValueSetMutationKind)mutationKind usingObjects:(NSSet *)objects;
```
&emsp;`key`: 无序一对多关系的属性名称。`mutationKind`: 将要进行的更改的类型。`objects`: 更改中涉及的对象（请参阅 NSKeyValueSetMutationKind）。

&emsp;手动实现键值观察合规性时，请使用此方法。

> &emsp;Important: 更改值后，必须使用相同的参数调用相应的 didChangeValueForKey:withSetMutation:usingObjects:。

&emsp;**你很少需要在子类中重写此方法，但是如果这样做，请确保调用 super。**

#### didChangeValueForKey:withSetMutation:usingObjects:
&emsp;通知被观察者对象对指定的无序对一对多关系进行了指定的更改。
```c++
- (void)didChangeValueForKey:(NSString *)key withSetMutation:(NSKeyValueSetMutationKind)mutationKind usingObjects:(NSSet *)objects;
```
&emsp;`key`: 无序一对多关系的属性名称。`mutationKind`: 所做更改的类型。`objects`: 更改中涉及的对象（请参阅 NSKeyValueSetMutationKind）。

&emsp;手动实现键值观察合规性时，请使用此方法。对该方法的调用始终与对  willChangeValueForKey:withSetMutation:usingObjects: 的调用配对。

&emsp;**你很少需要在子类中重写此方法，但是如果这样做，请务必调用 super。**

#### automaticallyNotifiesObserversForKey:
&emsp;返回一个布尔值，该值指示被观察者对象是否支持给定键的自动键值观察。
```c++
+ (BOOL)automaticallyNotifiesObserversForKey:(NSString *)key;
```
&emsp;如果键值观察机制应自动调用 willChangeValueForKey:/didChangeValueForKey: 和  willChange:valuesAtIndexes:forKey:/didChange:valuesAtIndexes:forKey: 只要该类的实例收到该键的键值编码消息，或为该键调用改变键值编码合规性方法时返回 YES；其他情况返回 NO。

&emsp;默认实现返回 YES。从 OS X 10.5 开始，此方法的默认实现从接收类中搜索名称与模式  +automaticallyNotifiesObserversOf<Key> 匹配的方法，并返回调用该方法的结果（如果找到）。找到的任何方法都必须返回 BOOL。如果找不到这样的方法，则返回 YES。

#### keyPathsForValuesAffectingValueForKey:
&emsp;为属性的值返回一组键路径，这些属性的值会影响指定键的值。
```c++
+ (NSSet<NSString *> *)keyPathsForValuesAffectingValueForKey:(NSString *)key;
```
&emsp;`key`: 其值受键值路径影响的键。

&emsp;当键的观察者向接收类的实例注册时，键值观察本身会自动观察同一实例的所有键路径，并在观察者的值发生变化时将键的更改通知发送给观察者关键路径发生了变化。

&emsp;当键的观察者向接收类的实例注册时，键值观察自身会自动观察同一实例的所有键路径，并在任何键路径的值发生更改时向观察者发送键的更改通知。

&emsp;此方法的默认实现在接收类中搜索名称与 +keyPathsForValuesAffecting<Key> 匹配的方法，如果找到该方法，则返回调用该方法的结果。任何这样的方法都必须返回 NSSet。如果没有找到这样的方法，则返回一个NSSet，该 NSSet 是根据先前不赞成使用的 setKeys:triggerChangeNotificationsForDependentKey: 方法的先前调用提供的信息计算得出的，以实现向后二进制兼容性。

&emsp;当某个属性的 getter 方法使用其他属性的值（包括按键路径定位的属性）计算要返回的值时，可以重写此方法。重写通常应该调用 super 并返回一个集合，该集合包含执行该操作所产生的集合中的任何成员（以免干扰超类中此方法的重写）。

> &emsp;Note: 使用 category 将计算的属性添加到现有类时，请勿覆盖此方法，不支持覆盖 category 中的方法。在这种情况下，请实现匹配的 +keyPathsForValuesAffecting<Key> 以利用此机制。

#### setKeys:triggerChangeNotificationsForDependentKey:
&emsp;如果给定数组中指定的任何属性发生更改，则将被观察者对象配置为发布给定属性的更改通知。

> &emsp;Deprecated: 请改用 keyPathsForValuesAffectingValueForKey: 方法代替。

```c++
+ (void)setKeys:(NSArray *)keys triggerChangeNotificationsForDependentKey:(NSString *)dependentKey;
```
&emsp;`keys`: 由 `dependentKey` 标识的属性值所依赖的属性的名称。`dependentKey`: 属性的名称，其值取决于 `keys` 指定的属性。

&emsp;对键中的任何键进行 will- 和 did-change KVO 通知方法的调用会自动为 dependentKey 调用相应的更改通知方法。被观察的对象没有接收 willChange 或 didChange 消息来生成通知。

&emsp;在创建接收类的任何实例之前，应先注册依赖项，因此通常可以在类的 initialize 方法中调用此方法，如以下示例所示。
```c++
+ (void)initialize {
    [self setKeys:@[@"firstName", @"lastName"] triggerChangeNotificationsForDependentKey:@"fullName"];
}
```

#### observationInfo
&emsp;返回一个指针，该指针标识有关向被观察者对象注册的所有观察者的信息。
```c++
@property void *observationInfo;
```
&emsp;该指针标识有关已向被观察者对象注册的所有观察者的信息，在注册时使用的选项等。

&emsp;此方法的默认实现从内存地址为键的被观察者对象的全局字典中检索信息。

&emsp;为了提高性能，可以覆盖此属性和 observationInfo，以将不透明数据指针存储在实例变量中。覆盖此属性不得尝试将消息发送到存储的数据。

&emsp;`observationInfo` 打印示例：
```c++
🟠🟠🟠 <NSKeyValueObservationInfo 0x60000059a5a0> (
<NSKeyValueObservance 0x600000b7b420: Observer: 0x7f9a7070fa40, Key path: name, Options: <New: YES, Old: YES, Prior: YES> Context: 0x0, Property: 0x600000b67a50>
<NSKeyValueObservance 0x600000b7af10: Observer: 0x6000007fd300, Key path: name, Options: <New: YES, Old: YES, Prior: YES> Context: 0x0, Property: 0x600000b67a50>
)
```
#### NSKeyValueChange
&emsp;可以观察到的变化类型。（change 字典中 NSKeyValueChangeKindKey 为 key 时对应的 value 值）
```c++
typedef NS_ENUM(NSUInteger, NSKeyValueChange) {
    NSKeyValueChangeSetting = 1,
    NSKeyValueChangeInsertion = 2,
    NSKeyValueChangeRemoval = 3,
    NSKeyValueChangeReplacement = 4,
};
```
&emsp;这些常量作为传递给 observeValueForKeyPath:ofObject:change:context: 函数中的更改字典中，NSKeyValueChangeKindKey 为 key 时的 value 值，指示所做更改的类型。

+ NSKeyValueChangeSetting: 指示被观察者的键路径的值已设置为新值。当观察对象的 keyPath 对应是：属性、一对一 和一对多关系的属性时，可能会发生这种更改。
+ NSKeyValueChangeInsertion: 指示已将对象插入到正在观察的一对多关系中。
+ NSKeyValueChangeRemoval: 指示已从观察的一对多关系中删除了一个对象。
+ NSKeyValueChangeReplacement: 指示已从观察的一对多关系中替换了一个对象。

#### NSKeyValueChangeKey
&emsp;可以显示在更改字典中的键。
```c++
typedef NSString * NSKeyValueChangeKey NS_STRING_ENUM;
```
&emsp;这些常量在传递给 observeValueForKeyPath:ofObject:change:context: 函数的 change 字典中用作键。

+ NSKeyValueChangeKindKey: 一个包含与 NSKeyValueChange 枚举之一相对应的值的 NSNumber 对象，指示发生了哪种更改。
```c++
FOUNDATION_EXPORT NSKeyValueChangeKey const NSKeyValueChangeKindKey;
```
&emsp;NSKeyValueChangeSetting 的值指示观察到的对象已收到 setValue:forKey: 消息，或者已调用该键的与键值编码兼容的 set 方法，或者 willChangeValueForKey: 或 didChangeValueForKey: 方法之一已被调用。

&emsp;NSKeyValueChangeInsertion、NSKeyValueChangeRemoval 或 NSKeyValueChangeReplacement 的值表示已向遵循键值的集合代理发送了变异消息，或者已调用了该键的其中一个符合键值编码的集合变异方法，或者某个集合将更改，或者某个方法已更改以其他方式调用。

&emsp;你可以在 NSNumber 对象上使用 unsignedIntegerValue 方法来检索更改种类的值。

+ NSKeyValueChangeIndexesKey: 如果 NSKeyValueChangeKindKey 条目的值是 NSKeyValueChangeInsertion、NSKeyValueChangeRemoval 或 NSKeyValueChangeReplacement，则此键的值是 NSIndexSet 对象，其中包含已插入、已删除或已替换对象的索引。
```c++
FOUNDATION_EXPORT NSKeyValueChangeKey const NSKeyValueChangeIndexesKey;
```
+ NSKeyValueChangeNewKey: 如果 NSKeyValueChangeKindKey 条目的值是 NSKeyValueChangeSetting，并且在注册观察者时指定了 NSKeyValueObservingOptionNew，则此键的值是该属性的新值。
```c++
FOUNDATION_EXPORT NSKeyValueChangeKey const NSKeyValueChangeNewKey;
```
&emsp;对于 NSKeyValueChangeInsertion 或 NSKeyValueChangeReplacement，如果在注册观察者时指定了 NSKeyValueObservingOptionNew，则此键的值是一个 NSArray 实例，该实例分别包含已插入或替换其他对象的对象。

+ NSKeyValueChangeNotificationIsPriorKey: 如果在注册观察者时指定了 NSKeyValueObservingOptionPrior 选项，则会在更改之前发送此通知。
```c++
FOUNDATION_EXPORT NSKeyValueChangeKey const NSKeyValueChangeNotificationIsPriorKey API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0));
```
&emsp;更改字典包含一个 NSKeyValueChangeNotificationIsPriorKey 条目，其值是一个包含布尔值 YES 的 NSNumber 对象。

+ NSKeyValueChangeOldKey: 如果 NSKeyValueChangeKindKey 条目的值是 NSKeyValueChangeSetting，并且在注册观察者时指定了 NSKeyValueObservingOptionOld，则此键的值是更改属性之前的值。
```c++
FOUNDATION_EXPORT NSKeyValueChangeKey const NSKeyValueChangeOldKey;
```
&emsp;对于 NSKeyValueChangeRemoval 或 NSKeyValueChangeReplacement，如果在注册观察者时指定了 NSKeyValueObservingOptionOld，则该值是一个 NSArray 实例，其中分别包含已删除或已被其他对象替换的对象。

#### NSKeyValueSetMutationKind
&emsp;你可以对无序集合进行的突变类型。
```c++
typedef NS_ENUM(NSUInteger, NSKeyValueSetMutationKind) {
    NSKeyValueUnionSetMutation = 1,
    NSKeyValueMinusSetMutation = 2,
    NSKeyValueIntersectSetMutation = 3,
    NSKeyValueSetSetMutation = 4
};
```
&emsp;这些常量被指定为方法 willChangeValueForKey:withSetMutation:usingObjects: 和 didChangeValueForKey:withSetMutation:usingObjects: 的 mutationKind 参数。它们的语义分别与 NSMutableSet 的 -unionSet:、-minusSet:、-intersectSet: 和 -setSet: 方法完全对应。

+ NSKeyValueUnionSetMutation: 指示已将指定集中的观察者添加到被观察者对象。这种突变会导致 NSKeyValueChangeInsertion 的 NSKeyValueChangeKindKey 值。
+ NSKeyValueMinusSetMutation: 指示正在从被观察者对象中移除指定集合中的观察者。这种突变会导致 NSKeyValueChangeRemoval 的 NSKeyValueChangeKindKey 值。
+ NSKeyValueIntersectSetMutation: 指示正在从被观察者对象中移除不在指定集合中的观察者。这种突变会导致 NSKeyValueChangeRemoval 的 NSKeyValueChangeKindKey 值。
+ NSKeyValueSetSetMutation: 指示一组观察者正在替换被观察者对象中的现有对象。这种突变会导致 NSKeyValueChangeReplacement 的 NSKeyValueChangeKindKey 值。

&emsp;以上就是 NSKeyValueObserving 在 Developer Documentation 文档中的所有内容，可能对一些内容会有一些迷糊，不要紧，在下面的 Key-Value Observing Programming Guide 中会有详细的解释以及示例代码，那么继续学习吧！

## Registering for Key-Value Observing（注册键值观察）
&emsp;你必须执行以下步骤才能使对象接收到符合 KVO 的属性的键值观察通知：

+ 使用 addObserver:forKeyPath:options:context: 方法向观察者注册被观察者对象。
+ 实现 observeValueForKeyPath:ofObject:change:context: 在 observer 内部接受更改通知消息。
+ 使用 removeObserver:forKeyPath: 方法注销 observer 当它不再应该接收消息时。至少，在 observer 从内存中释放之前调用此方法。

> &emsp;Important: 并非所有类的所有属性都符合 KVO。你可以按照 KVO Compliance 中所述的步骤，确保自己的类符合 KVO。通常，Apple 提供的框架中的属性只有在有文档记录的情况下才符合 KVO。

### Registering as an Observer（注册为观察者）
&emsp;观察者对象（observing object）首先通过发送 addObserver:forKeyPath:options:context: 消息向被观察者对象（observed object）注册自己，将其自身作为观察者（observer）和要观察的属性的关键路径传递。观察者（observer）还指定了一个 options 参数和一个 context 指针来管理通知的各个方面。
#### Options（观察选项）
```c++
// - (void)addObserver:(NSObject *)observer forKeyPath:(NSString *)keyPath options:(NSKeyValueObservingOptions)options context:(nullable void *)context;
// - (void)observeValueForKeyPath:(nullable NSString *)keyPath ofObject:(nullable id)object change:(nullable NSDictionary<NSKeyValueChangeKey, id> *)change context:(nullable void *)context;

// 用于 -addObserver:forKeyPath:options:context: 和 -addObserver:toObjectsAtIndexes:forKeyPath:options:context: 函数的选项
typedef NS_OPTIONS(NSUInteger, NSKeyValueObservingOptions) {
    // 通知中发送的更改字典（(NSDictionary<NSKeyValueChangeKey,id> *)change）是否应分别包含 NSKeyValueChangeNewKey 和 NSKeyValueChangeOldKey 条目。
    NSKeyValueObservingOptionNew = 0x01,
    NSKeyValueObservingOptionOld = 0x02,
    
    // 在观察者注册方法返回之前，是否应立即将通知发送给观察者。
    // 如果还指定了 NSKeyValueObservingOptionNew，则通知中的更改字典将始终包含 NSKeyValueChangeNewKey 条目，但绝不包含 NSKeyValueChangeOldKey 条目。
    //（在初始通知中，观察到的属性的当前值可能是旧的，但对于观察者却是新的。）
    // 你可以使用此选项，而不是同时显式调用观察者的 -observeValueForKeyPath:ofObject:change:context: 方法。
    // 当此选项与 -addObserver:toObjectsAtIndexes:forKeyPath:options:context: 一起使用时，将向要添加观察者的每个索引对象发送一个通知。
    
    // 当调用 -addObserver:forKeyPath:options:context: 函数注册观察者并在 options 参数中包含 NSKeyValueObservingOptionInitial 选项时，
    // 此时会立即调用一次观察者的 -observeValueForKeyPath:ofObject:change:context: 函数，如果 options 参数是 NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew，
    // 则在 change 字典中将始终包含 NSKeyValueChangeNewKey 值。（如果被观察者的 keyPath 对应的属性的值为 nil 则change 字典中 NSKeyValueChangeNewKey 的值为 NULL）
    
    // 当 options 参数是 NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew 时，change 字典打印结果：（keyPath 对应属性值是 nil/有值 时）
    // {kind = 1; new = "<null>";} {kind = 1; new = CHM;}
    // 当 options 参数是 NSKeyValueObservingOptionInitial 时，打印 change 字典，仅包含 kind: {kind = 1;}
    
    // 即 NSKeyValueObservingOptionInitial 选项的作用仅是为了在注册观察者时收到一次通知。
    //（连带着可以包含 new 值或者不包含 new 值，看开发者意愿，如果选项是 NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionOld 时，change 字典也只包含 kind）
    
    NSKeyValueObservingOptionInitial API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0)) = 0x04,
    
    // 是否应在每次更改之前和之后将单独的通知发送给观察者，而不是在更改之后将单个通知发送给观察者。
    // 更改之前发送的通知中的 chnage 字典始终包含 NSKeyValueChangeNotificationIsPriorKey 条目，其值为 [NSNumber numberWithBool: YES]，但从不包含 NSKeyValueChangeNewKey 条目。
    
    // options 是 NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld | NSKeyValueObservingOptionPrior 时，
    // 更改前打印：{kind = 1; notificationIsPrior = 1; old = CHM;}
    // 更改后打印：{kind = 1; new = JAY; old = CHM;}
    
    // options 是 NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld 时，只收到一个更改后的打印，和上面的同时使用三个选项时是相同的，
    // 更改后打印：{kind = 1; new = JAY; old = CHM;}
    
    // 当观察者自己的 KVO 兼容性要求它为其自身的属性之一调用 -willChange... 方法之一时，可以使用此选项，并且该属性的值取决于所观察对象的属性的值。
    //（在这种情况下，为响应更改后收到的 -observeValueForKeyPath:ofObject:change:context: 消息而轻易地适当调用 -willChange... 为时已晚。）
    
    // 指定此选项后，更改后发送的通知中的 change 字典包含与未指定此选项时将包含的条目相同的条目，但 NSOrderedSets 表示的有序唯 一对多关系 除外。
    // 对于这些更改，对于 NSKeyValueChangeInsertion 和 NSKeyValueChangeReplacement 更改，
    // will-change 通知的 change 字典包含一个 NSKeyValueChangeIndexesKey（和 NSKeyValueChangeOldKey，如果是替换，则在注册时指定 NSKeyValueObservingOptionOld 选项），它给出了操作可能更改的索引（和对象）。
    // 更改之后，第二个通知包含报告实际更改内容的条目。对于 NSKeyValueChangeRemoval 更改，按索引清除是精确的。
    
    NSKeyValueObservingOptionPrior API_AVAILABLE(macos(10.5), ios(2.0), watchos(2.0), tvos(9.0)) = 0x08
};
```
&emsp;options 参数，指定为（多个）选项常量的按位 or（或者单个的选项常量），既会影响通知中提供的更改字典的内容，又会影响生成通知的方式。

&emsp;你可以通过指定选项 NSKeyValueObservingOptionOld 选择从更改之前接收被观察者的属性的值（属性的旧值）。你可以使用选项 NSKeyValueObservingOptionNew 来请求属性的新值（属性的旧值）。你可以通过这些选项的按位 or 来接收新旧值。

&emsp;当 options 参数中包含 NSKeyValueObservingOptionInitial 选项时，指示被观察者立即发出一个改变通知（在 addObserver:forKeyPath:options:context: 返回之前）。你可以使用此附加的一次性通知在 observer 中建立属性的初始值。（即在注册观察者时立即发送一次通知，如果 options 中包含 NSKeyValueObservingOptionNew，则此次发送的通知的字典中还同时包含 keyPath 对应属性的当前值，我们可以在观察者中记录此初始值用作其他用途）

&emsp;通过包含选项 NSKeyValueObservingOptionPrior，可以指示被观察对象在属性更改之前发送通知（除了更改之后的常规通知之外）。change 字典中通过包含 value 为 NSNumber 包装的 YES， key 是 NSKeyValueChangeNotificationIsPriorKey 来表示此次通知是一个预更改通知。（如上面的，更改前打印通知中的 change 字典的内容：{kind = 1; notificationIsPrior = 1; old = CHM;}）

&emsp;That key is not otherwise present. You can use the prechange notification when the observer’s own KVO compliance requires it to invoke one of the -willChange… methods for one of its properties that depends on an observed property. The usual post-change notification comes too late to invoke willChange… in time.
#### Context（上下文）
&emsp;addObserver:forKeyPath:options:context: 消息中的 context 指针包含任意数据，这些数据将在相应的更改通知中传递回观察者。你可以指定 NULL 并完全依靠键路径字符串来确定变更通知的来源，但是这种方法可能会导致对象的父类由于不同的原因而观察到相同的键路径，因此可能会引起问题。

&emsp;一种更安全、更可扩展的方法是使用 context 确保你收到的通知是发给观察者的，而不是超类的。

&emsp;类中唯一命名的静态变量的地址提供了良好的 context。在超类或子类中以类似方式选择的 context 不太可能重叠。你可以为整个类选择一个 context，然后依靠通知消息中的键路径字符串来确定更改的内容。另外，你可以为每个被观察者的键路径创建一个不同的 context，从而完全不需要进行字符串比较，从而可以更有效地进行通知解析。Listing 1 显示了以这种方式选择的 balance 和 interestRate 属性的示例 context。

&emsp;Listing 1  Creating context pointers（创建 context 指针）
```c++
static void *PersonAccountBalanceContext = &PersonAccountBalanceContext;
static void *PersonAccountInterestRateContext = &PersonAccountInterestRateContext;
```
&emsp;Listing 2 中的示例演示了 Person 实例如何使用给定的 context 指针将自己注册为 Account 实例的 balance 和 interestRate 属性的观察者。

&emsp;Listing 2 Registering the inspector as an observer of the balance and interestRate properties（将 inspector 注册为 balance 和 interestRate 属性的观察者）
```c++
- (void)registerAsObserverForAccount:(Account*)account {
    [account addObserver:self
              forKeyPath:@"balance"
                 options:(NSKeyValueObservingOptionNew |
                          NSKeyValueObservingOptionOld)
                 context:PersonAccountBalanceContext];
 
    [account addObserver:self
              forKeyPath:@"interestRate"
                 options:(NSKeyValueObservingOptionNew |
                          NSKeyValueObservingOptionOld)
                  context:PersonAccountInterestRateContext];
}
```
> &emsp;键值观察 addObserver:forKeyPath:options:context: 方法未维护对观察者对象（self）、被观察者对象（account）或 context 的强引用。你应该确保在必要时维护对观察者对象（self）、被观察者对象（account）以及 context 的强引用。
### Receiving Notification of a Change（接收变更通知）
&emsp;当被观察者的属性的值更改时，观察者将收到一条 observeValueForKeyPath:ofObject:change:context: 消息。所有观察者都必须实现此方法。

&emsp;观察对象提供触发通知的键路径，它本身作为相关对象，包含有关更改的详细信息的字典，以及在为该键路径注册观察者时提供的上下文指针。

&emsp;change 字典的 NSKeyValueChangeKindKey 条目提供有关发生的更改类型的信息。如果被观察者对象的值已更改，则 NSKeyValueChangeKindKey 条目将返回 NSKeyValueChangeSetting。根据注册观察者时指定的选项（options），change 字典中的 NSKeyValueChangeOldKey 和 NSKeyValueChangeNewKey 条目包含更改前后的属性值。如果属性是对象，则直接提供值。如果属性是标量（int/float 等）或 C 结构（struct），则将值包装在 NSValue 对象中（与 key-value coding 一样）。

&emsp;如果被观察者属性是一个一对多关系，则 NSKeyValueChangeKindKey 条目还通过分别返回 NSKeyValueChangeInsertion、NSKeyValueChangeRemoval 或 NSKeyValueChangeReplacement 来指示是否插入、删除或替换了 relationship 中的对象。

&emsp;NSKeyValueChangeIndexesKey 的更改字典条目是一个 NSIndexSet 对象，用于指定已更改关系中的索引。如果在注册观察者时将 NSKeyValueObservingOptionNew 或 NSKeyValueObservingOptionOld 指定为选项，则更改字典中的 NSKeyValueChangeOldKey 和 NSKeyValueChangeNewKey 条目是包含更改前后相关对象值的数组。

&emsp;Listing 3 中的示例展示了 Person 观察者的 observeValueForKeyPath:ofObject:change:context: 实现，该实现记录了 Listing 2 中注册的属性 balance 和 interestRate 的新旧值。

&emsp;Listing 3  Implementation of observeValueForKeyPath:ofObject:change:context:（observeValueForKeyPath:ofObject:change:context: 实现）
```c++
- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary *)change
                       context:(void *)context {
 
    if (context == PersonAccountBalanceContext) {
        // Do something with the balance…
 
    } else if (context == PersonAccountInterestRateContext) {
        // Do something with the interest rate…
 
    } else {
        // Any unrecognized context must belong to super
        // 任何无法识别的 context 必须属于super
        [super observeValueForKeyPath:keyPath
                             ofObject:object
                               change:change
                               context:context];
    }
}
```
&emsp;如果在注册观察者时指定了 NULL 上下文，则将通知的键路径与要观察的键路径进行比较，以确定已更改的内容。如果你对所有被观察者的键路径使用了单个上下文，则首先要根据通知的上下文进行测试，然后找到匹配项，然后使用键路径字符串比较来确定具体更改的内容。如果你为每个键路径提供了唯一的上下文，如此处所示，则一系列简单的指针比较会同时告诉你通知是否针对此观察者，如果是，则更改了哪个键路径。

&emsp;在任何情况下，观察者均应在无法识别上下文（或在简单情况下，是任何键路径）时始终调用超类的 observeValueForKeyPath:ofObject:change:context: 实现，因为这意味着超类也已经注册了通知。

> &emsp;Note: 如果通知传播到类层次结构的顶部，则 NSObject 会引发 NSInternalInconsistencyException，因为这是编程错误：子类无法使用其注册的通知。
### Removing an Object as an Observer（移除作为观察者的对象）
&emsp;通过向被观察者对象发送 removeObserver:forKeyPath:context:  消息，并指定观察者对象、键路径和上下文，可以移除 key-value observer。Listing 4 中的示例显示 Person 除去自己作为 balance 和 interestRate 的观察者。

&emsp;Listing 4 Removing the inspector as an observer of balance and interestRate（移除作为 balance 和 interestRate 属性观察者的 inspector）
```c++
- (void)unregisterAsObserverForAccount:(Account*)account {
    [account removeObserver:self
                 forKeyPath:@"balance"
                    context:PersonAccountBalanceContext];
 
    [account removeObserver:self
                 forKeyPath:@"interestRate"
                    context:PersonAccountInterestRateContext];
}
```
&emsp;收到 removeObserver:forKeyPath:context: 消息后，观察对象将不再收到指定键路径和对象的任何 observeValueForKeyPath:ofObject:change:context: 消息。

&emsp;移除观察者时，请记住以下几点：
+ 如果尚未注册为观察者，则请求以观察者身份移除会导致 NSRangeException。 你可以对 removeObserver:forKeyPath:context: 进行一次调用，以对应对 addObserver:forKeyPath:options:context: 的调用，或者，如果在你的应用中不可行，则将 removeObserver:forKeyPath:context: 调用在 try/catch 块内以处理潜在的异常。
+ 观察者释放后，观察者不会自动将其自身移除。被观察者对象继续发送通知，而忽略了观察者的状态。但是，与发送到已释放对象的任何其他消息一样，更改通知会触发内存访问异常。因此，你必须确保观察者在从内存中消失之前将自己移除。
+ 该协议无法询问对象是观察者还是被观察者。构建你的代码时以避免 release 相关的错误。一种典型的模式是在观察者初始化期间（例如，在 init 或 viewDidLoad 中）注册为观察者，并在释放过程中（通常在 dealloc 中）注销，以确保成对和有序地添加和删除消息，并且在从内存中释放观察者之前，未对其进行注册。
## KVO Compliance（KVO 合规性）
&emsp;为了被视为与特定属性的 KVO 兼容，类必须确保：
+ 该类必须与属性的键值编码兼容，如 key-value coding compliant 中所述。
  KVO 支持与 KVC 相同的数据类型，包括 Objective-C 对象以及 Scalar and Structure Support 中列出的标量和结构。
+ 该类为属性发出 KVO 更改通知。
+ 相关键已正确注册（请参阅 Registering Dependent Keys）。

&emsp;有两种技术可以确保发出更改通知（change notifications）。自动支持由 NSObject 提供，默认情况下可用于符合键值编码的类的所有属性。通常，如果遵循标准的 Cocoa 编码和命名约定，就可以使用自动更改通知，而无需编写任何其他代码。

&emsp;手动更改通知提供了对何时发出通知的附加控制，并且需要附加编码。通过实现类方法 automaticallyNotifiesObserversForKey:，可以控制子类属性的自动通知。
### Automatic Change Notification（自动更改通知）
&emsp;NSObject 提供了自动键值更改通知（automatic key-value change notification）的基本实现。自动键值更改通知通知观察者使用键值兼容的访问器以及键值编码方法所做的更改。例如：mutableArrayValueForKey: 返回的集合代理对象也支持自动通知。

&emsp;Listing 1 所示的示例导致属性名的任何观察者都会收到更改通知。

&emsp;Listing 1  Examples of method calls that cause KVO change notifications to be emitted（导致发出 KVO 更改通知的方法调用示例）
```c++
// Call the accessor method. 调用访问器方法
[account setName:@"Savings"];
 
// Use setValue:forKey:.
[account setValue:@"Savings" forKey:@"name"];
 
// Use a key path, where 'account' is a kvc-compliant property of 'document'.
[document setValue:@"Savings" forKeyPath:@"account.name"];
 
// Use mutableArrayValueForKey: to retrieve a relationship proxy object.
Transaction *newTransaction = <#Create a new transaction for the account#>;
NSMutableArray *transactions = [account mutableArrayValueForKey:@"transactions"];
[transactions addObject:newTransaction];
```
### Manual Change Notification（手动更改通知）
&emsp;在某些情况下，你可能希望控制通知过程，例如，将由于特定于应用程序的原因而不必要的触发通知最小化，或者将多个更改分组到单个通知中。手动更改通知提供了执行此操作的方法。

&emsp;手动和自动通知并不相互排斥。你可以自由地发出手动通知，除了自动通知已经执行。更典型的是，你可能希望完全控制特定属性的通知。在本例中，你将重写 automaticallyNotifiesObserversForKey: 的 NSObject 实现。对于要排除其自动通知的属性，automaticallyNotifiesObserversForKey: 的子类实现应返回 NO。子类实现应为任何无法识别的键调用 super。Listing 2 中的示例启用了 balance 属性的手动通知，允许超类确定所有其他键的通知。

&emsp;Listing 2  Example implementation of automaticallyNotifiesObserversForKey:（automaticallyNotifiesObserversForKey: 函数到实现示例）
```c++
+ (BOOL)automaticallyNotifiesObserversForKey:(NSString *)theKey {
    BOOL automatic = NO;
    if ([theKey isEqualToString:@"balance"]) {
        // 当是 balance 时返回 NO
        automatic = NO;
    } else {
        // 其他属性的情况下则是调用其父类实现
        automatic = [super automaticallyNotifiesObserversForKey:theKey];
    }
    
    return automatic;
}
```
&emsp;要实现手动观察者通知，请在更改值之前调用 willChangeValueForKey:，在更改值之后调用 didChangeValueForKey:。Listing 3 中的示例实现了 balance 属性的手动通知。

&emsp;Listing 3  Example accessor method implementing manual notification（实现手动通知的访问器方法示例）
```c++
- (void)setBalance:(double)theBalance {
    [self willChangeValueForKey:@"balance"];
    _balance = theBalance;
    [self didChangeValueForKey:@"balance"];
}
```
&emsp;你可以通过首先检查值是否已更改来最小化发送不必要的通知。Listing 4 中的示例测试 balance 的值，并仅在其发生更改时提供通知。

&emsp;Listing 4  Testing the value for change before providing notification（在提供通知之前测试更改值）
```c++
- (void)setBalance:(double)theBalance {
    // 判断 _balance 的当前值和 theBalance 不相等时才发出更改通知
    if (theBalance != _balance) {
        [self willChangeValueForKey:@"balance"];
        _balance = theBalance;
        [self didChangeValueForKey:@"balance"];
    }
}
```
&emsp;如果单个操作导致多个键发生更改，则必须嵌套更改通知，如 Listing 5 所示。

&emsp;Listing 5  Nesting change notifications for multiple keys（嵌套多个键的更改通知）
```c++
- (void)setBalance:(double)theBalance {
    // balance 和 itemChanged 两个属性都发生改变
    [self willChangeValueForKey:@"balance"];
    [self willChangeValueForKey:@"itemChanged"];
    
    _balance = theBalance;
    _itemChanged = _itemChanged+1;
    
    [self didChangeValueForKey:@"itemChanged"];
    [self didChangeValueForKey:@"balance"];
}
```
&emsp;在有序一对多关系的情况下，不仅必须指定更改的键，还必须指定更改的类型和所涉及对象的索引。更改类型是指定 NSKeyValueChangeInsertion、NSKeyValueChangeRemoval 或 NSKeyValueChangeReplacement 的 NSKeyValueChange。受影响对象的索引作为 NSIndexSet 对象传递。

&emsp;Listing 6 中的代码片段演示了如何在 transactions 所示的一对多关系中包装对象的删除。

&emsp;Listing 6  Implementation of manual observer notification in a to-many relationship（在一对多关系中实现手动观察者通知）
```c++
- (void)removeTransactionsAtIndexes:(NSIndexSet *)indexes {
    [self willChange:NSKeyValueChangeRemoval valuesAtIndexes:indexes forKey:@"transactions"];
 
    // Remove the transaction objects at the specified indexes.
 
    [self didChange:NSKeyValueChangeRemoval valuesAtIndexes:indexes forKey:@"transactions"];
}
```
## Registering Dependent Keys（注册依赖的 Keys）
&emsp;在许多情况下，一个属性的值取决于另一个对象中一个或多个其他属性的值。如果某个属性的值发生更改，则派生属性（derived property）的值也应标记为更改。如何确保为这些依赖属性发布键值观察通知取决于关系的基数（cardinality of the relationship）。
### To-One Relationships（一对一）
&emsp;要为一对一关系自动触发通知，你应该重写 keyPathsForValuesAffectingValueForKey: 或实现一个合适的方法，该方法遵循它为注册依赖键定义的模式。

&emsp;例如，一个人的全名取决于名字和姓氏。返回全名的方法可以编写如下：
```c++
- (NSString *)fullName {
    return [NSString stringWithFormat:@"%@ %@",firstName, lastName];
}
```
&emsp;当 firstName 或 lastName 属性更改时，必须通知观察 fullName 属性的应用程序，因为它们会影响属性的值。

&emsp;一种解决方案是重写 keyPathsForValuesAffectingValueForKey: 指定 person 的 fullName 属性依赖于 lastName 和 firstName 属性。Listing 1 展示了这种依赖关系的一个示例实现：

&emsp;Listing 1  Example implementation of keyPathsForValuesAffectingValueForKey:
```c++
+ (NSSet *)keyPathsForValuesAffectingValueForKey:(NSString *)key {
    NSSet *keyPaths = [super keyPathsForValuesAffectingValueForKey:key];
 
    if ([key isEqualToString:@"fullName"]) {
        NSArray *affectingKeys = @[@"lastName", @"firstName"];
        keyPaths = [keyPaths setByAddingObjectsFromArray:affectingKeys];
    }
    
    return keyPaths;
}
```
&emsp;重写通常应该调用 super 并返回一个集合，该集合包含执行该操作所产生的集合中的任何成员（以免干扰超类中此方法的重写）。

&emsp;也可以通过实现一个遵循命名约定 keyPathsForValuesAffecting<Key> 的类方法来实现相同的结果，其中 <Key> 是依赖于这些值的属性（首字母大写）的名称。使用此模式，可以将 Listing 1 中的代码重写为名为 keyPathsForValuesAffectingFullName 的类方法，如 Listing 2 所示。

&emsp;Listing 2  Example implementation of the keyPathsForValuesAffecting<Key> naming convention
```c++
+ (NSSet *)keyPathsForValuesAffectingFullName {
    return [NSSet setWithObjects:@"lastName", @"firstName", nil];
}
```
&emsp;在使用 category 向现有类添加计算属性时，不能重写 keyPathsForValuesAffectingValueForKey: 方法，因为不应重写 category 中的方法。在这种情况下，实现一个匹配的 keyPathsForValuesAffecting<Key> 类方法来利用这种机制。

> &emsp;Note: 不能通过实现 keyPathsForValuesAffectingValueForKey: 来设置一对多个关系的依赖关系。相反，你必须观察 to-many 集合中每个对象的适当属性，并通过自己更新依赖键来响应其值的更改。下一节展示了处理这种情况的策略。

### To-Many Relationships（一对多）
&emsp;keyPathsForValuesAffectingValueForKey: 方法不支持包含一对多关系的键路径。例如，假设你有一个 Department 对象，它与一个 Employee 具有一对多关系（employees），Employee 具有 salary 属性。你可能希望 Department 对象具有 totalSalary 属性，该属性依赖于 relationship 中所有 Employees 的 salaries。例如，不能使用 keyPathsForValuesAffectingTotalSalary 函数返回 employees.salary 作为一个 key。

&emsp;两种情况下都有两种可能的解决方案：
1. 可以使用键值观察将父级（在本例中为 Department）注册为所有子级（在本例中为 Employees）的相关属性的观察者。当子对象添加到关系中或从关系中移除时，必须将父对象作为观察者添加和移除（请参见  Registering for Key-Value Observing）。在 observeValueForKeyPath:ofObject:change:context: 方法更新依赖值以响应更改，如以下代码片段所示：
```c++
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
    if (context == totalSalaryContext) {
        [self updateTotalSalary];
    } else
    // deal with other observations and/or invoke super...
}
 
- (void)updateTotalSalary {
    [self setTotalSalary:[self valueForKeyPath:@"employees.@sum.salary"]];
}
 
- (void)setTotalSalary:(NSNumber *)newTotalSalary {
    if (totalSalary != newTotalSalary) {
        [self willChangeValueForKey:@"totalSalary"];
        _totalSalary = newTotalSalary;
        [self didChangeValueForKey:@"totalSalary"];
    }
}
 
- (NSNumber *)totalSalary {
    return _totalSalary;
}
```
2. 如果你使用的是 Core Data，那么可以在应用程序的通知中心注册父级，作为其 managed object context 的观察者。父类应以类似于键值观察的方式回应子类发布的相关变更通知。

## Key-Value Observing Implementation Details（Key-Value Observing 实现详情）
&emsp;自动键值观察是使用 isa 交换技术实现的。

&emsp;顾名思义，isa 指针指向维护调度表的对象类。这个调度表本质上包含指向类实现的方法的指针以及其他数据。

&emsp;当一个观察者为一个对象的属性注册时，被观察者对象的 isa 指针被修改，指向一个中间类而不是真类（NSKVONotifying_XXX）。因此，isa 指针的值不一定反映实例的实际类。

&emsp;决不能依赖 isa 指针来确定类成员身份。相反，应该使用 class 方法来确定对象实例的类。

> &emsp;KVO 是通过 isa-swizzling 实现的。基本的流程就是编译器自动为被观察者对象创造一个派生类（此派生类的父类是被观察者对象所属类），并将被观察者对象的 isa  指向这个派生类。如果用户注册了对此目标对象的某一个属性的观察，那么此派生类会重写这个属性的 setter 方法，并在其中添加进行通知的代码。Objective-C 在发送消息的时候，会通过 isa 指针找到当前对象所属的类对象。而类对象中保存着当前对象的实例方法，因此在向此对象发送消息时候，实际上是发送到了派生类对象的方法。由于编译器对派生类的方法进行了 override，并添加了通知代码，因此会向注册的观察者对象发送通知。注意派生类只重写注册了观察者的属性方法。

&emsp;如下示例代码中定义的 Student 类，当对其 name 属性注册了观察者后，打印其 class 和 isa 如下：
```c++
@interface Student : NSObject
@property (nonatomic, copy) NSString *name;
@end

// 对 self.student 添加观察者后 class 函数返回的依然是 Student
NSLog(@"🤍🤍 %@", [self.student class]);

// object_getClass 方法返回 isa 指向却是 NSKVONotifying_Student
NSLog(@"🤍🤍 %@", object_getClass(self.student));

// 控制台打印:
🤍🤍 Student
🤍🤍 NSKVONotifying_Student
 
// 然后移除 self.student 的观察者后，object_getClass(self.student) 返回的则是 Student。
```

> &emsp;简而言之，苹果使用了一种 isa 交换的技术，当 student 被观察后，student 对象的 isa 指针被指向了一个新建的 Student 的子类 NSKVONotifying_Student，且这个子类重写了被观察值的 setter 方法和 class 方法，dealloc 和 \_isKVO 方法，然后使 student 对象的 isa 指针指向这个新建的类，然后事实上 student 变为了NSKVONotifying_Student 的实例对象，执行方法要从这个类的方法列表里找。dealloc 方法：观察移除后使 class 变回去 Student（通过 isa 指向）, \_isKVO 方法判断被观察者自己是否同时也观察了其他对象。（同时苹果警告我们，通过 isa 获取类的类型是不可靠的，通过 class 方法才能得到正确的类）[用代码探讨 KVC/KVO 的实现原理](https://juejin.cn/post/6844903587898753037)

## 参考链接
**参考链接:🔗**
+ [Key-Value Observing Programming Guide](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/KeyValueObserving/KeyValueObserving.html#//apple_ref/doc/uid/10000177-BCICJDHA)
+ [iOS KVC和KVO详解](https://juejin.cn/post/6844903602545229831#heading-22)
+ [用代码探讨 KVC/KVO 的实现原理](https://juejin.cn/post/6844903587898753037)
