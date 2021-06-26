# iOS KVC 官方文档《Key-Value Coding Programming Guide》中文翻译

> &emsp;日常开发中我们可能已经非常习惯于使用 `valueForKey:` 和 `setValue:forKey:`，不过可能有一些细节我们未深入过，那么下面一起通过官方文档来全面的学习一下 KVC 吧！⛽️⛽️ 

## About Key-Value Coding
&emsp;Key-value coding（键值编码）是由 NSKeyValueCoding 非正式协议启用的一种机制，对象采用这种机制来提供对其属性/成员变量的间接访问。当一个对象符合键值编码时，它的属性/成员变量可以通过一个简洁、统一的消息传递接口（`setValue:forKey:`）通过字符串参数寻址。这种间接访问机制补充了实例变量（自动生成的 \_属性名 ）及其相关访问器方法（getter 方法）提供的直接访问。

&emsp;通常使用访问器方法来访问对象的属性。get 访问器（或 getter）返回属性的值。set 访问器（或 setter）设置属性的值。在 Objective-C 中，还可以直接访问属性的底层实例变量（由编译器生成的对应于属性的由 下划线和属性名拼接构成的实例变量）。以上述任何一种方式访问对象属性都是简单的，但需要调用特定于属性的方法或变量名。随着属性列表的增长或更改，访问这些属性的代码也必须随之增长或更改。相反，键值编码兼容对象提供了一个简单的消息传递接口，该接口在其所有属性中都是一致的。

&emsp;键值编码（key-value coding）是许多其他 Cocoa 技术的基础，例如 key-value observing、Cocoa bindings、Core Data 和 AppleScript-ability。在某些情况下，键值编码（key-value coding）还可以帮助简化代码。

&emsp;在上面提到 "Key-value coding（键值编码）是由 NSKeyValueCoding 非正式协议启用的一种机制"，话说 NSKeyValueCoding 是非正式协议，即它不同于我们常见的 NSCopying、NSCoding 等协议是通过 @protocol 直接来定义的，然后当哪些类要遵循此协议时在其类声明或者类延展后面添加 <NSCopying, NSCoding> 表示该类遵循此协议。而 NSKeyValueCoding 机制则是通过分类来实现的，在 Foundation 框架下有一个 NSKeyValueCoding.h 文件，其内部定义了多组分类接口，其中包括：@interface NSObject(NSKeyValueCoding)、@interface NSArray<ObjectType>(NSKeyValueCoding)、@interface NSDictionary<KeyType, ObjectType>(NSKeyValueCoding)、@interface NSMutableDictionary<KeyType, ObjectType>(NSKeyValueCoding)、@interface NSOrderedSet<ObjectType>(NSKeyValueCoding)、@interface NSSet<ObjectType>(NSKeyValueCoding)，其中 NSObject 基类已经实现了 NSKeyValueCoding 机制的所有接口，然后 NSArray、NSDictionary、NSMutableDictionary、NSOrderedSet、NSSet 这些子类则是对 setValue:forKey: 和 valueForKey: 函数进行重载。例如当对一个 NSArray 对象调用 `setValue:forKey:` 函数时，它内部是对数组中的每个元素调用 `setValue:forKey:` 函数。当对一个 NSArray 对象调用 `valueForKey:` 函数时，它返回一个数组，其中包含在数组的每个元素上调用 `valueForKey:` 的结果。返回的数组将包含 `NSNull` 元素，指代的是数组中某些元素调用 `valueForKey:` 函数返回 nil 的情况。

&emsp;那么这里我们先看一下 NSKeyValueCoding 的文档内容，然后再接着看 Key-Value Coding Programming Guide 文档。

### NSKeyValueCoding
&emsp; NSKeyValueCoding 是一种可以通过名称或键（字符串）间接访问对象属性/成员变量的机制。

&emsp;读取和设置对象的值（属性值/成员变量值）的基本方法是: 
+ `setValue:forKey:` 它设置由指定键标识的属性的值。
+ `valueForKey:` 它返回由指定键标识的属性的值。

&emsp;因此，可以以一致的方式读取和设置对象的所有属性/成员变量。

&emsp;访问属性值的默认实现方式依赖于通常由对象实现的访问器（getter）方法（或在需要时直接访问实例变量（\_属性名））。
#### valueForKey:
&emsp;返回由给定键标识的属性的值。
```c++
- (id)valueForKey:(NSString *)key;
```
&emsp;`key` 是 receiver 属性之一的名字。返回值是由 `key` 标识的属性的值。Key-Value Coding Programming Guide 中的 Accessor Search Patterns 中描述了 `valueForKey:` 用于查找要返回的正确值的搜索模式。
#### valueForKeyPath:
&emsp;返回由给定键路径标识的派生属性的值。
```c++
- (id)valueForKeyPath:(NSString *)keyPath;
```
&emsp;`keyPath` 表示属性关系的路径（有一个或多个关系），例如 "department.name" 或 "department.manager.lastName"。返回值是 `keyPath` 标识的派生属性的值。

&emsp;默认实现使用 `valueForKey:` 获取每个关系的目标对象，并将 `valueForKey:` 消息的结果返回到最终对象。

&emsp;可能看完上面还是一头雾水，下面我们通过一个例子看一下:
```c++
// Person 类有一个名字是 name 的属性 
@interface Person : NSObject
@property (nonatomic, copy) NSString *name;
@end

// Student 类有一个名字是 person 的属性
@interface Student : NSObject
@property (nonatomic, strong) Person *person;
@end

Person *person = [[Person alloc] init];
person.name = @"CHM";
Student *student = [[Student alloc] init];
student.person = person;

// 通过 valueForKeyPath: 入参为 @"person.name" 来读取 student 下 person 属性的 name
NSLog(@"📪📪 %@", [student valueForKeyPath:@"person.name"]);
```
&emsp;当我们通过 `valueForKeyPath:` 来读取 Person 的 name 属性值时，通过 @"person.name" 来读取。
#### dictionaryWithValuesForKeys:
&emsp;返回一个字典，其中包含由给定数组中的每个键标识的属性值。
```c++
- (NSDictionary<NSString *,id> *)dictionaryWithValuesForKeys:(NSArray<NSString *> *)keys;
```
&emsp;`keys` 包含标识 receiver 属性的 NSString 数组。返回值是包含 keys 中属性名称作为键的字典，相应的值是相应的属性值。

&emsp;默认实现为 keys 中的每个 key 调用 `valueForKey:`，并将字典中的 NSNull 值替换为返回的 nil 值（即当某个 key 返回 nil 时，在返回值字典中以 NSNull 替代）。（内部是调用 `valueForKey:`，不是 `valueForKeyPath:`，keys 不能包含  @"person.name" 此种形式的 key，如果未重写 receiver 的 `valueForUndefinedKey:` 函数的话会 crash）（`valueForUndefinedKey:` 函数的默认实现是抛出一个异常）

#### valueForUndefinedKey:
&emsp;由 valueForKey: 调用，当找不到与给定键对应的属性时。
```c++
- (id)valueForUndefinedKey:(NSString *)key;
```
&emsp;`key` 是一个不等于任何 receiver 属性名称的字符串。

&emsp;子类可以重写此方法以返回未定义键的备用值。默认实现引发 NSUndefinedKeyException。

#### mutableArrayValueForKey:
&emsp;返回一个可变数组代理，该代理提供对给定键指定的有序一对多关系的读写访问（即通过 key 访问对象的数组属性）。
```c++
- (NSMutableArray *)mutableArrayValueForKey:(NSString *)key;
```
&emsp;`key` 是一对多关系的名称。返回值是一个可变数组代理，它提供对 key 指定的有序一对多关系的读写访问。

&emsp;添加到可变数组中的对象与 receiver 相关，而从可变数组中删除的对象则不再相关。默认实现识别与 `valueForKey:` 相同的简单访问器方法和数组访问器方法，并遵循相同的直接实例变量访问策略，但总是返回可变集合代理对象，而不是 `valueForKey:` 将返回的不可变集合。

&emsp;Key-Value Coding Programming Guide 中的 Accessor Search Patterns 描述了 mutableArrayValueForKey: 使用的搜索模式。

&emsp;如上给 Student 添加一个 `@property (nonatomic, strong) NSMutableArray *array;` 属性时，使用如下三种方式都能正常读取：
```c++
NSLog(@"📢📢 %@", [self.student mutableArrayValueForKey:@"array"]);
NSLog(@"📢📢 %@", [self.student valueForKey:@"array"]);
NSLog(@"📢📢 %@", [self.student valueForKeyPath:@"array"]);
```
&emsp;当使用 mutableArrayValueForKey: 函数读取非数组类型属性时，如 `@property (nonatomic, strong) Person *person;`，使用 `NSLog(@"📢📢 %@", [self.student mutableArrayValueForKey:@"person"]);` 读取则直接 crash，控制台输出: Terminating app due to uncaught exception 'NSInvalidArgumentException', reason: '-[Person count]: unrecognized selector sent to instance 0x600002b4d780'。

&emsp;还有一系列的：mutableArrayValueForKeyPath:/mutableSetValueForKey:/mutableSetValueForKeyPath:/mutableOrderedSetValueForKey:/mutableOrderedSetValueForKeyPath: 函数和 mutableArrayValueForKey: 函数相似。

&emsp;这里我们再详细看一下 `mutableArrayValueForKey:` 的用途，它可以帮助我们更快速的修改 可变/不可变集合类型的属性。还是上面的测试代码，我们给 Student 添加一个不可变数组属性 `@property (nonatomic, strong) NSArray<Person *> *personArray;`，测试如下代码：
```c++
self.student = [[Student alloc] init];

Person *person2 = [[Person alloc] init];
NSMutableArray *arr = [@[person2, person2, person2] mutableCopy];
self.student.personArray = arr;

NSLog(@"🛂 打印 %@", [self.student mutableArrayValueForKey:@"personArray"]);

[[self.student mutableArrayValueForKey:@"personArray"] removeLastObject];
NSLog(@"🛂🛂 移除后 %@", [self.student mutableArrayValueForKey:@"personArray"]);

[[self.student mutableArrayValueForKey:@"personArray"] addObject:person2];
NSLog(@"🛂🛂🛂 新增后 %@", [self.student mutableArrayValueForKey:@"personArray"]);

[((NSMutableArray *)[self.student valueForKey:@"personArray"]) removeLastObject];
NSLog(@"🛂🛂🛂🛂 valueForKey 移除后 %@", self.student.personArray);

[((NSMutableArray *)[self.student valueForKey:@"personArray"]) addObject:person2];
NSLog(@"🛂🛂🛂🛂🛂 valueForKey 新增后 %@", self.student.personArray);

// 控制台打印:
 🛂 打印 (
    "<Person: 0x600001fdd0e0>",
    "<Person: 0x600001fdd0e0>",
    "<Person: 0x600001fdd0e0>"
)
 🛂🛂 移除后 (
    "<Person: 0x600001fdd0e0>",
    "<Person: 0x600001fdd0e0>"
)
 🛂🛂🛂 新增后 (
    "<Person: 0x600001fdd0e0>",
    "<Person: 0x600001fdd0e0>",
    "<Person: 0x600001fdd0e0>"
)
 🛂🛂🛂🛂 valueForKey 移除后 (
    "<Person: 0x600001fdd0e0>",
    "<Person: 0x600001fdd0e0>"
)
 🛂🛂🛂🛂🛂 valueForKey 新增后 (
    "<Person: 0x600001fdd0e0>",
    "<Person: 0x600001fdd0e0>",
    "<Person: 0x600001fdd0e0>"
)
```
&emsp;可看到通过 mutableArrayValueForKey: 我们可以直接增加和移除 personArray 不可变数组中的内容。如果我们使用 self.student.personArray 的方式的话，则要来回进行读取和赋值。

#### setValue:forKey:
&emsp;将给定键指定的 receiver 的属性设置为给定值。
```c++
- (void)setValue:(id)value forKey:(NSString *)key;
```
&emsp;`key` 是 receiver 属性之一的名称。`value` 是由 `key` 标识的属性的值。

&emsp;如果 key 标识了一对一关系，将 value 指定的对象与 receiver 相关联，如果存在，则取消先前相关的对象的关联。

&emsp;给定一个集合对象（value）和一个标识一对多关系的 key，将集合中包含的对象与 receiver 关联，如果存在以前关联的对象，则取消关联。

&emsp;Key-Value Coding Programming Guide 中的 Accessor Search Patterns 中描述了 `setValue:forKey:` 使用的搜索模式。

&emsp;在使用引用计数的环境中，如果直接访问实例变量，则将保留值。
#### setValue:forKeyPath:
&emsp;将由给定键路径标识的属性的值设置为给定值。
```c++
- (void)setValue:(id)value forKeyPath:(NSString *)keyPath;
```
&emsp;`value` 是 `keyPath` 标识的属性的值。`keyPath` 表示属性关系的路径（有一个或多个关系），例如 "department.name" 或 "department.manager.lastName"。（同 valueForKeyPath:）

&emsp;此方法的默认实现使用 `valueForKey:` 获取每个关系的目标对象，并向最终对象发送 `setValue:forKey:` 消息。（即先读取最终对象然后为其赋值）

&emsp;如果使用此方法，并且目标对象未实现该 `value` 的访问器，默认行为是该对象保留 `value`，而不是复制或分配 `value`。

#### setNilValueForKey:
&emsp;由 `setValue:forKey:` 调用，当给定标量值的 nil 值时（例如 int 或 float，即使用 `setValue:forKey:` 给非对象类型的属性赋值时）。
```c++
- (void)setNilValueForKey:(NSString *)key;
```
&emsp;子类可以重写此方法以其他方式处理请求，例如用 0 或前哨值（sentinel value）代替 nil 并再次调用 `setValue:forKey:` 或直接设置变量。默认实现是引发 NSInvalidArgumentException。

&emsp;这里是针对非对象类型的属性例如 int / float。例如上面 Student 类添加一个 int 类型的属性 `@property (nonatomic, assign) int mark;`，然后我们使用 `[self.student setValue:nil forKey:@"mark"];` 设置的话，会直接 crash，此时我们可以重写 Student 的 setNilValueForKey: 函数来处理此种情况或者防止 crash。而属性是对象类型的话 value 传 nil 不会 crash。

#### setValuesForKeysWithDictionary:
&emsp;使用给定字典中的 value 设置 receiver 的属性，使用 value 对应的 key 来标识 receiver 的属性。
```c++
- (void)setValuesForKeysWithDictionary:(NSDictionary<NSString *,id> *)keyedValues;
```
&emsp;`keyedValues` 一个字典，其 key 标识 receiver 中的属性。receiver 中的属性值设置为字典中的 key 对应的 value。

&emsp;默认实现为 `keyedValues` 中的每个键值对调用  `setValue:forKey:`，用 nil 替换 `keyedValues` 中的 NSNull 值。

#### setValue:forUndefinedKey:
&emsp;由 `setValue:forKey:` 调用，当它找不到给定键的属性时。
```c++
- (void)setValue:(id)value forUndefinedKey:(NSString *)key;
```
&emsp;`key` 一个不等于任何 receiver 属性名称的字符串。

&emsp;子类可以重写此方法以其他方式处理请求。默认实现引发 NSUndefinedKeyException。（同 `valueForUndefinedKey:`）

#### accessInstanceVariablesDirectly
&emsp;（只读）返回一个布尔值，该值指示 key-value coding 方法在未找到属性的访问器方法时是否应直接访问相应的实例变量。
```c++
@property(class, readonly) BOOL accessInstanceVariablesDirectly;
```
&emsp;如果键值编码方法应在找不到属性的访问器方法时直接访问相应的实例变量，则为 YES，否则为 NO。

&emsp;默认返回 YES。子类可以覆盖它以返回 NO，在这种情况下，key-value coding 方法将无法访问实例变量。

#### validateValue:forKey:error:
&emsp;返回一个布尔值，该值指示给定指针所指定的值对于给定键所标识的属性是否有效。
```c++
- (BOOL)validateValue:(inout id  _Nullable *)ioValue forKey:(NSString *)inKey error:(out NSError * _Nullable *)outError;
```
&emsp;`ioValue` 指向由 `inKey` 标识的属性的新值的指针。此方法可以修改或替换该值以使其有效。`inKey` receiver 属性之一的名称。`inKey` 必须指定一个属性或一对一关系。`outError` 如果必须进行验证并且 `ioValue` 不会转换为有效值，则返回时会包含一个 NSError 对象，该对象描述 `ioValue` 不是有效值的原因。

&emsp;如果 `ioValue` 指向的值对 `inKey` 标识的属性有效，或者该方法能够修改 `ioValue` 的值使其有效，则将布尔值设置为 YES；否则，将布尔值设置为 NO。

#### validateValue:forKeyPath:error:
&emsp;返回一个布尔值，该值指示给定指针指定的值对于相对于接收者的给定键路径是否有效。
```c++
- (BOOL)validateValue:(inout id  _Nullable *)ioValue forKeyPath:(NSString *)inKeyPath error:(out NSError * _Nullable *)outError;
```
&emsp;此方法的默认实现使用 `valueForKey:` 获取每个关系的目标对象，并返回对该属性调用 `validateValue:forKey:error:` 方法的结果。

&emsp;好了 NSKeyValueCoding 的内容我们差不多看完了，下面我们接着看 Key-Value Coding Programming Guide 文档。

### Using Key-Value Coding Compliant Objects（使用键值编码兼容对象）
&emsp;对象通常在继承 NSObject（直接或间接）时也会拥有 key-value coding 能力，因为 NSObject 采用 NSKeyValueCoding 协议并为基本方法提供默认实现（在 NSObject + NSKeyValueCoding 分类中默认实现了 Key-Value Coding 使用的基本方法）。此类对象允许其他对象通过紧凑的消息传递接口执行以下操作：

+ **访问对象属性。** 该协议指定方法，例如泛型 getter 函数：`valueForKey:` 和泛型 setter 函数：`setValue:forKey:`，用于通过名称或键（参数化为字符串）访问对象属性。这些方法和相关方法的默认实现使用键来定位基础数据并与之交互，如 Accessing Object Properties 中所述。

+ **操作集合属性。** 访问方法的默认实现与对象的集合属性（例如 NSArray 对象）一起使用，就像其他任何属性一样。此外，如果对象为属性定义了集合访问器方法，则它将启用对集合内容的键值访问。这通常比直接访问更有效，并允许你通过标准化的接口处理自定义集合对象，如 Accessing Collection Properties 中所述。

+ **在集合对象上调用集合运算符。** 访问 key-value coding 兼容对象中的集合属性时，可以将集合运算符插入键字符串，如 Using Collection Operators 中所述。集合运算符指示默认的 NSKeyValueCoding getter 实现对集合执行操作，然后返回新的、经过筛选的集合版本，或者返回表示集合某些特征的单个值（平均值、总和等）。

+ **访问非对象属性。** 协议的默认实现检测非对象属性，包括标量（scalars，int/float 等）和结构（structures），并自动将它们包装和展开为对象，以便在协议接口上使用，如 Representing Non-Object Values 中所述。此外，该协议声明了一种方法（`setNilValueForKey:`），允许兼容对象在通过键值编码接口对非对象属性设置 nil 值时提供适当的操作。

+ **通过键路径访问属性。** 当你具有与 key-value coding 兼容的对象的层次结构时，可以使用基于键路径的方法调用来通过单个调用在层次结构内深入（直到最终目标），获取或设置值。（即沿着 @"xxx.xx.x" 一路向下）

### Adopting Key-Value Coding for an Object（为对象采用键值编码）
&emsp;为了使你自己的对象符合键值编码的要求，请确保它们采用 NSKeyValueCoding 非正式协议并实现相应的方法，例如 `valueForKey:` 作为通用 getter 和 `setValue:forKey:` 作为通用 setter。幸运的是，如上所述，NSObject 采用了此协议，并为这些和其他必要方法提供了默认实现。因此，如果你从 NSObject（或者其许多子类中的任何一个）派生对象，则许多工作已经为你完成。（实现接口都在 NSObject + NSKeyValueCoding 分类中）

&emsp;为了使默认方法完成它们的工作，需要确保对象的访问器方法和实例变量遵循某些定义良好的（明确的）模式。这允许默认实现根据键值编码的消息查找对象的属性。然后，你可以选择通过提供验证方法和处理某些特殊情况的方法来扩展和自定义键值编码。

### Key-Value Coding with Swift
&emsp;默认情况下，从 NSObject 或其子类继承的 Swift 对象的属性符合键值编码。而在 Objective-C 中，属性的访问器和实例变量必须遵循特定的模式，Swift 中的标准属性声明自动保证了这一点。另一方面，协议的许多特性要么不相关，要么使用 Objective-C 中不存在的 native Swift 构造或技术处理得更好。例如，由于所有 Swift 属性都是对象，因此你永远不会使用默认实现对非对象属性的特殊处理。

&emsp;因此，当键值编码协议方法直接转换为 Swift 时，本指南主要关注 Objective-C，在 Objective-C 中，你需要做更多的工作来确保遵从性，并且键值编码通常是最有用的。需要在 Swift 中采用显著不同方法的情况在整个指南中均有说明。

&emsp;有关将 Swift 与 Cocoa 技术结合使用的更多信息，请阅读 Using Swift with Cocoa and Objective-C (Swift 3)。有关 Swift 的完整说明，请阅读 The Swift Programming Language (Swift 3)。

### Other Cocoa Technologies Rely on Key-Value Coding（其他 Cocoa 技术依赖键值编码）
&emsp;符合键值编码的对象可以参与各种依赖于这种访问的 Cocoa 技术，包括：
+ **键值观察（Key-value observing）。** 此机制使对象能够注册由另一个对象的属性更改驱动的异步通知，如 **Key-Value Observing Programming Guide** 中所述。
+ **Cocoa bindings.** 这一系列技术完全实现了 Model-View-Controller 范例，其中模型封装了应用程序数据，视图显示和编辑该数据，并且控制器在两者之​​间进行中介。阅读 **Cocoa Bindings Programming Topics**，以了解有关 Cocoa Bindings 的更多信息。
+ **Core Data.** 
+ **AppleScript.** 这种脚本语言可以直接控制可编写脚本的应用程序和 macOS 的许多部分。Cocoa 的脚本支持利用键值编码来获取和设置可脚本化对象中的信息。NSScriptKeyValueCoding 非正式协议中的方法提供了处理键值编码的附加功能，包括通过索引获取和设置多值键中的键值，以及将键值强制（或转换）为适当的数据类型。AppleScript Overview 提供 AppleScript 及其相关技术的高级概述。

## Accessing Object Properties（访问对象属性）
&emsp;对象（类定义）通常在其接口声明中指定属性，这些属性属于以下几类之一：
+ **Attributes.** 这些是简单值，例如标量（int/float 等）、字符串或布尔值。值对象（例如 NSNumber）和其他不可变类型（例如 NSColor）也被视为 attributes。
+ **To-one relationships.** 它们是自身具有属性的可变对象。对象的属性可以更改，而对象本身不会更改。例如，银行帐户对象可能有一个 owner 属性，该属性是 Person 对象的实例，Person 对象本身有一个 address 属性。owner 的地址可以更改，而不必更改银行账户持有的 owner 参考资料。银行账户的 owner 没有改变，只有他们的地址发生改变。
+ **To-many relationships.** 这些是集合对象。尽管也可以使用自定义集合类，但通常使用 NSArray 或 NSSet 的实例来保存此类集合。

&emsp;清单 2-1 中声明的 BankAccount 对象（类）演示了每种类型的属性。

&emsp;Listing 2-1Properties of the BankAccount object（BankAccount 对象的属性）
```c++
@interface BankAccount : NSObject
 
@property (nonatomic) NSNumber* currentBalance;              // An attribute
@property (nonatomic) Person* owner;                         // A to-one relation
@property (nonatomic) NSArray<Transaction*>* transactions; // A to-many relation
 
@end
```
&emsp;为了维护封装，对象通常为其接口上的属性提供访问器方法。对象的作者可以显式地编写这些方法，也可以依赖编译器自动合成它们。无论哪种方式，使用这些访问器之一的代码的作者都必须在编译代码之前将属性名写入代码中。访问器方法的名称成为使用它的代码的静态部分。例如，给定清单 2-1 中声明的 bank account 对象，编译器合成一个 setter，你可以为 myAccount 实例调用它：
```c++
[myAccount setCurrentBalance:@(100.0)];
```
&emsp;这是直接的，但缺乏灵活性。另一方面，键值编码兼容对象提供了一种更通用的机制，可以使用字符串标识符访问对象的属性。

### Identifying an Object’s Properties with Keys and Key Paths（使用键和键路径识别对象的属性）
&emsp;键（key）是标识特定属性的字符串。通常，按照约定，表示属性的键是代码中显示的属性本身的名称。键必须使用 ASCII 编码，不能包含空格，并且通常以小写字母开头（尽管也有例外，例如在许多类中找到的 URL 属性）。

&emsp;因为清单 2-1 中的 BankAccount 类是符合键值编码的，所以它可以识别的 keys 如：owner、currentBalance 和 transactions，它们是其属性的名称。不需要调用 setCurrentBalance: 方法，你可以通过其键设置值：
```c++
[myAccount setValue:@(100.0) forKey:@"currentBalance"];
```
&emsp;实际上，你可以使用相同的方法设置 myAccount 对象的所有属性，使用不同的键参数。因为参数是字符串，所以它可以是在运行时操作的变量。

&emsp;键路径是一组点（.）分隔键，用于指定要遍历的对象属性序列。序列中第一个 key 的属性相对于接收方，并且每个后续 key 相对于前一个属性的值进行评估。键路径对于通过单个方法调用深入到对象的层次结构非常有用。

&emsp;例如，假定 Person 和 Address 类也符合键值编码，则应用于银行帐户实例的键路径 owner.address.street 指的是存储在银行帐户所有者地址中的街道字符串的值。

> &emsp;NOTE: 在 Swift 中，你可以使用 #keyPath 表达式，而不是使用字符串来指示键或键路径。正如 Using Swift with Cocoa and Objective-C (Swift 3) guide 中的 “键和键路径” 部分所述，这提供了编译时检查的优势。

### Getting Attribute Values Using Keys（使用键获取属性值）
&emsp;当一个对象采用 NSKeyValueCoding 协议时，它符合键值编码要求。从 NSObject 继承的对象提供了协议基本方法的默认实现，它自动采用具有某些默认行为的协议（NSObject + NSKeyValueCoding 分类中实现的方法）。此类对象至少实现以下基本的基于键的 getters：
+ `valueForKey:` - 返回由键参数命名的属性的值。如果根据 Accessor Search Patterns 中描述的规则找不到由键命名的属性，则对象会向自己发送一个 `valueForUndefinedKey:` 消息。`valueForUndefinedKey:` 的默认实现引发了 NSUndefinedKeyException，但是子类可以重写此行为并更优雅地处理这种情况。 
+ `valueForKeyPath:` - 返回相对于 receiver 的指定键路径的值。键路径序列中的任何对象，如果对特定键不兼容键值编码，即 `valueForKey:` 的默认实现找不到访问器方法，都会收到 `valueForUndefinedKey:` 消息。
+ `dictionaryWithValuesForKeys:` - 返回相对于 receiver 的键数组的值。该方法为数组中的每个键调用 `valueForKey:`。返回的 NSDictionary 包含数组中所有键的值。

> &emsp;NOTE: 集合对象（如 NSArray、NSSet 和 NSDictionary）不能包含 nil 作为值。相反，可以使用 NSNull 对象表示 nil 值。NSNull 提供表示对象属性的 nil 值的单个实例。`dictionaryWithValuesForKeys:` 和相关 `setValuesForKeysWithDictionary:` 的默认实现会自动在 NSNull（在 dictionary 参数中）和 nil（在存储的属性中）之间进行转换。

&emsp;使用键路径寻址属性时，如果键路径中除最后一个键以外的任何键是一个一对多关系（即它引用了一个集合），则返回的值是一个集合，其中包含一对多键右侧键的所有值。例如，请求键路径的 transactions.payee 返回一个数组，其中包含所有 transactions 的所有 payee 对象。这也适用于键路径中的多个数组。键路径 accounts.transactions.payee 返回一个数组，其中包含所有 accounts 中所有 transactions 的所有 payee 对象。（例如上面的例子中我们给 Student 添加一个 `@property (nonatomic, strong) NSArray<Person *> *personArray;` 这样的属性，然后 `NSLog(@"❇️❇️ %@", [self.student valueForKeyPath:@"personArray.name"]);` 这样调用时，打印的就是 personArray 属性中的每个 Person 对象的 name 构成的一个字符串数组。）

### Setting Attribute Values Using Keys（使用键设置属性值）
&emsp;与 getter 一样，key-value coding 兼容的对象还提供了一小部分通用 setter，这些 setter 基于 NSObject 中 NSKeyValueCoding 协议的实现（NSObject + NSKeyValueCoding 分类中实现的方法），具有默认行为：
+ `setValue:forKey:` - 将相对于接收消息的对象的指定键的值设置为给定值。`setValue:forKey:` 的默认实现会自动解包表示标量（int/float 等）和结构（struct）的 NSNumber 和 NSValue 对象，并将它们的值分配给属性。有关包装和展开语义的详细信息，请参见 Representing Non-Object Values。（我们对这里解释一下，例如还是我们上面的 Student 类，它有一个 int 类型的属性 `@property (nonatomic, assign) int mark;`，那么我们使用 `setValue:forKey:` 时要把 value 包成一个 NSNumber 类型，因为 value 只接收 id 类型，如: `[self.student setValue:@(99) forKey:@"mark"];`，这样我们就直接给 mark 属性赋值了，key-value coding 自动完成了解包赋值。）
  如果指定的键对应于接收 setter 消息的对象是不具有的属性，则该对象会向自己发送一个 `setValue:forUndefinedKey:` 消息且 `setValue:forUndefinedKey:` 的默认实现是引发 NSUndefinedKeyException。但是，子类可以重写此方法以自定义方式处理请求防止 crash。
+ `setValue:forKeyPath:` - 在相对于 receiver 的指定键路径处设置给定值。键路径序列中不符合特定键的键值编码的任何对象都会收到 `setValue:forUndefinedKey:` 消息。
+ `setValuesForKeysWithDictionary:` - 使用指定字典中的值设置 receiver 的属性，使用字典键标识属性。默认实现调用 `setValue:forKey:` 对于每个键值对，根据需要用 nil 替换 NSNull 对象。

&emsp;在默认实现中，当你尝试将非对象属性设置为 nil 值时，键值编码兼容对象会向自己发送 `setNilValueForKey:` 消息。`setNilValueForKey:` 的默认实现引发 NSInvalidArgumentException，但对象可能会重写 `setNilValueForKey:`  以替换默认值或标记值，如  Handling Non-Object Values 中所述。

### Using Keys to Simplify Object Access（使用键简化对象访问）
&emsp;要了解基于键的 getter 和 setter 如何简化代码，请参考以下示例。在 macOS 中，NSTableView 和 NSOutlineView 对象将标识符字符串与其每一列相关联。如果支持该 table 的 model 对象不符合键值编码，则 table 的数据源方法将被迫依次检查每个列标识符，以找到要返回的正确属性，如清单 2-2 所示。此外，将来当你将另一个属性（在本例中是 Person 对象）添加到 model 中时，还必须重新访问数据源方法，添加另一个条件以测试新属性并返回相关值。

&emsp;Listing 2-2 Implementation of data source method without key-value coding（没有键值编码的数据源方法的实现）
```c++
- (id)tableView:(NSTableView *)tableview objectValueForTableColumn:(id)column row:(NSInteger)row {
    id result = nil;
    Person *person = [self.people objectAtIndex:row];
 
    if ([[column identifier] isEqualToString:@"name"]) {
        result = [person name];
    } else if ([[column identifier] isEqualToString:@"age"]) {
        result = @([person age]);  // Wrap age, a scalar, as an NSNumber
    } else if ([[column identifier] isEqualToString:@"favoriteColor"]) {
        result = [person favoriteColor];
    } // And so on...
 
    return result;
}
```
&emsp;另一方面，清单 2-3 展示了同一数据源方法的一个更加紧凑的实现，它利用了一个键值编码兼容的 Person 对象。仅使用 `valueForKey:` getter，数据源方法使用列标识符作为键返回适当的值。除了更短之外，它还更通用，因为在以后添加新列时，只要列标识符始终与模型对象的属性名匹配，它就可以继续工作，而不会发生变化。

&emsp;Listing 2-3 Implementation of data source method with key-value coding（键值编码的数据源方法的实现）
```c++
- (id)tableView:(NSTableView *)tableview objectValueForTableColumn:(id)column row:(NSInteger)row {
    return [[self.people objectAtIndex:row] valueForKey:[column identifier]];
}
```
## Accessing Collection Properties（访问集合属性）
&emsp;与键值编码兼容的对象以公开其他属性的方式公开其多个属性。你可以像使用 `valueForKey:` 和 `setValue:forKey:`（或它们的等效键路径）一样，获取或设置 **集合对象**，就像获取其他任何对象一样。但是，当你想要操纵这些集合的内容时，通常使用协议定义的可变代理方法是最有效的。

&emsp;该协议为访问 **集合对象** 定义了三种不同的代理方法，每种方法都有一个键（key）和一个键路径(keyPath)变体：
+ `mutableArrayValueForKey:` 和 `mutableArrayValueForKeyPath:` 它们返回行为类似于 NSMutableArray 对象的代理对象。
+ `mutableSetValueForKey:` 和 `mutableSetValueForKeyPath:` 它们返回行为类似于 NSMutableSet 对象的代理对象。
+ `mutableOrderedSetValueForKey:` 和 `mutableOrderedSetValueForKeyPath:` 它们返回行为类似于 NSMutableOrderedSet 对象的代理对象。

&emsp;在代理对象上进行操作、向其中添加对象、从中删除对象或替换其中的对象时，协议的默认实现会相应地修改基础属性。这比使用 `valueForKey:` 获取一个不可变的集合对象、创建一个具有更改内容的已修改集合对象，然后再使用 `setValue:forKey:` 为指定属性赋值方便灵活很多。在许多情况下，这些方法提供了额外的好处，即维护集合对象中保存的对象的键值观察遵从性（see Key-Value Observing Programming Guide for details）。

## Using Collection Operators（使用集合运算符）
&emsp;当你向键值编码兼容的对象发送 `valueForKeyPath:` 消息时，可以在键值路径中嵌入一个集合操作符。集合操作符是前面带有 at 符号（@）的一小串关键字中的一个，它指定 getter 在返回数据之前应该执行的操作，以某种方式处理数据。NSObject 提供的 `valueForKeyPath:` 默认实现了此行为。

&emsp;当一个键路径包含一个集合操作符时，该操作符前面的键路径的任何部分（称为 left key path）表示相对于消息的接收者要操作的集合。如果将消息直接发送到集合对象（如 NSArray 实例），则可能会忽略 left key path。操作符后面的键路径部分（称为 right key path）指定运算符应处理的集合中的元素的属性。除 @count 之外的所有集合运算符都需要正确的键路径。图 4-1说明了 operator key path 格式。（下面的代码示例可以帮助我们理解这 3 部分的含义）

&emsp;Figure 4-1 Operator key path format

![Operator key path format](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/786d1febd5b543dc92799f76a4fd652c~tplv-k3u1fbpfcp-watermark.image)

&emsp;集合运算符表现出三种基本的行为类型：
+ Aggregation Operators（集合运算符）以某种方式合并集合的对象，并返回通常与在正确的键路径中命名的属性的数据类型匹配的单个对象。 @count 运算符是一个例外，它不使用任何右键路径，并且始终返回 NSNumber 实例。
+ Array Operators（数组运算符）返回一个 NSArray 实例，其中包含命名集合中保存的对象的某些子集。
+ Nesting Operators（嵌套运算符）处理包含其他集合的集合，并根据运算符返回以某种方式组合嵌套集合对象的 NSArray 或 NSSet 实例。

### Sample Data
&emsp;下面的描述包括演示如何调用每个操作符的代码段，以及这样做的结果。这些依赖于 Listing 2-1 所示的 BankAccount 类，它包含一个 Transaction 对象数组。每一个都表示一个简单的 checkbook entry，如 Listing 4-1 所示。（如下是简单的两个类定义）

&emsp;Listing 2-1Properties of the BankAccount object（BankAccount 对象的属性）
```c++
@interface BankAccount : NSObject
 
@property (nonatomic) NSNumber* currentBalance;              // An attribute
@property (nonatomic) Person* owner;                         // A to-one relation
@property (nonatomic) NSArray< Transaction* >* transactions; // A to-many relation
 
@end
```
&emsp;Listing 4-1 Interface declaration for the Transaction object（Transaction 对象的接口声明）
```c++
@interface Transaction : NSObject
 
@property (nonatomic) NSString* payee;   // To whom
@property (nonatomic) NSNumber* amount;  // How much
@property (nonatomic) NSDate* date;      // When
 
@end
```
&emsp;为了便于讨论，假设你的 BankAccount 实例有一个用 Listing 4-1 中所示的数据填充的 transactions 数组，并且你从 BankAccount 对象内部进行示例调用。（self 表示的是一个 BankAccount 对象）

Table 4-1 Example data for the Transactions objects（Transactions 对象的示例数据）
| payee values | amount values formatted as currency | date values formatted as month day, year |
| -- | -- | -- |
| Green Power | $120.00 | Dec 1, 2015 |
| Green Power | $150.00 | Jan 1, 2016 |
| Green Power | $170.00 | Feb 1, 2016 |
| Car Loan | $250.00 | Jan 15, 2016 |
| Car Loan | $250.00 | Feb 15, 2016 |
| Car Loan | $250.00 | Mar 15, 2016 |
| General Cable | $120.00 | Dec 1, 2015 |
| General Cable | $155.00 | Jan 1, 2016 |
| General Cable | $120.00 | Feb 1, 2016 |
| Mortgage | $1,250.00 | Jan 15, 2016 |
| Mortgage | $1,250.00 | Feb 15, 2016 |
| Mortgage | $1,250.00 | Mar 15, 2016 |
| Animal Hospital | $600.00 | Jul 15, 2016 |

### Aggregation Operators
&emsp;聚合运算符处理一个数组或一组属性，生成反映集合某些方面的单个值。

#### @avg（求平均值）
&emsp;指定 @avg 运算符时，`valueForKeyPath:` 读取由集合的每个元素的右键路径指定的属性，将其转换为 double（用 0 代替 nil 值），并计算这些值的算术平均值，然后返回存储在 NSNumber 实例中的结果。

&emsp;要获取 Table 4-1 中的样本数据中的平均交易金额：
```c++
// transactionAverage 是 self.transactions 数组中的每个 Transaction 对象的 amount 属性的平均值。
NSNumber *transactionAverage = [self.transactions valueForKeyPath:@"@avg.amount"];
```
&emsp;transactionAverage 的格式化结果是 456.54 美元。
#### @count
&emsp;当指定 @count 运算符时，`valueForKeyPath:` 返回的 NSNumber 实例表示集合中的对象数。正确的键路径（如果存在）将被忽略。

&emsp;要获取交易中交易对象的数量：
```c++
// numberOfTransactions 是 self.transactions 数组中的元素的个数。
NSNumber *numberOfTransactions = [self.transactions valueForKeyPath:@"@count"];
```
&emsp;numberOfTransactions 的值为 13。
#### @max
&emsp;当指定 @max 运算符时，`valueForKeyPath:` 在由右键路径命名的集合条目中搜索并返回最大的条目。搜索使用 `compare:` 方法进行比较，该方法由许多 Foundation 类（例如 NSNumber 类）定义。因此，由右键路径指示的属性必须包含一个对该消息有意义响应的对象（即集合中的元素必须实现了 `compare:` 函数）。搜索将忽略 nil 的集合条目。

&emsp;要获取 table 4-1 中列出的 transaction 中的日期值的最大值（即最近一笔事务的日期），请执行以下操作：
```c++
NSDate *latestDate = [self.transactions valueForKeyPath:@"@max.date"];
```
&emsp;格式化的 latestDate 值为 Jul 15, 2016。
#### @min
&emsp;指定 @min 运算符时，`valueForKeyPath:` 在由右键路径命名的集合项中搜索，并返回最小的项。搜索使用 `compare:` 方法进行比较，这是由许多基础类（如 NSNumber 类）定义的。因此，右键路径所指示的属性必须包含对该消息作出有意义响应的对象。搜索将忽略 nil 的集合条目。

&emsp;要获取 table 4-1 中列出的 transactions 中最早的 transaction 的日期值中的最小值，请执行以下操作：
```c++
NSDate *earliestDate = [self.transactions valueForKeyPath:@"@min.date"];
```
&emsp;格式化的 earliestDate 值为 Dec 1, 2015。

#### @sum
&emsp;指定 @sum 运算符时，`valueForKeyPath:` 读取由右键路径为集合的每个元素指定的属性，将其转换为 double（用 0 代替 nil 值），并计算这些值的和。然后返回存储在 NSNumber 实例中的结果。
```c++
NSNumber *amountSum = [self.transactions valueForKeyPath:@"@sum.amount"];
```
&emsp;amountSum 的格式化结果为 $ 5,935.00。

### Array Operators
&emsp;数组运算符使 `valueForKeyPath:` 返回一个对象数组，该对象数组与右键路径指示的一组特定对象相对应。

> &emsp;IMPORTANT: 如果使用数组运算符时任何叶子对象（leaf objects）为 nil，则 `valueForKeyPath:` 方法将引发异常。

#### @distinctUnionOfObjects
&emsp;当指定 @distinctUnionOfObjects 运算符时，`valueForKeyPath:` 创建并返回一个数组，该数组包含与右键路径指定的属性相对应的集合的不同对象。

&emsp;要获得省略重复值的 transactions 中 transaction 的 payee 属性值的集合，请执行以下操作：
```c++
// distinctPayees 是 self.transactions 数组中的每个 Transaction 对象的 payee 的值组成的字符串数组（忽略重复的 payee）。
NSArray *distinctPayees = [self.transactions valueForKeyPath:@"@distinctUnionOfObjects.payee"];
```
&emsp;生成的 distinctPayees 数组包含以下字符串中的每个字符串的一个实例：Car Loan、General Cable、Animal Hospital、Green Power、Mortgage。

> &emsp;NOTE: @unionOfObjects 运算符提供类似的行为，但不删除重复的对象。
#### @unionOfObjects
&emsp;当指定 @unionOfObjects 运算符时，`valueForKeyPath:` 创建并返回一个数组，该数组包含与右键路径指定的属性相对应的集合的所有对象。与 @distinctUnionOfObjects 不同，不会删除重复的对象。

&emsp;要获取 transactions 中 transaction 的 payee 属性值的集合：
```c++
// payees 是 self.transactions 数组中的每个 Transaction 对象的 payee 的值组成的字符串数组（不忽略重复的 payee）。
NSArray *payees = [self.transactions valueForKeyPath:@"@unionOfObjects.payee"];
```
&emsp;所得的 payees 数组包含以下字符串：Green Power、Green Power、Green Power、Car Loan、Car Loan、Car Loan、General Cable、General Cable、General Cable、Mortgage、Mortgage、Mortgage、Animal Hospital。（注意重复项）

> &emsp;NOTE: 与 @distinctUnionOfArrays 运算符提供类似的行为，但是删除重复的对象。

### Nesting Operators
&emsp;嵌套运算符对嵌套集合进行操作，其中集合本身的每个条目都包含一个集合。

> &emsp;IMPORTANT: 使用嵌套运算符时，如果任何叶子对象（leaf objects）为 nil，则 `valueForKeyPath:` 方法将引发异常。

&emsp;对于以下描述，考虑第二个数据数组，称为 moreTransactions，填充 Table 4-2 中的数据，并与原始事务数组（来自 Sample Data 部分）一起收集到一个嵌套数组中：
```c++
NSArray* moreTransactions = @[<# transaction data #>]; // Table 4-2 中的数据
NSArray* arrayOfArrays = @[self.transactions, moreTransactions];
```
&emsp;
Table 4-2 Hypothetical Transaction data in the moreTransactions array（moreTransactions 数组中的假设交易数据）
| payee values | amount values formatted as currency | date values formatted as month day, year |
| -- | -- | -- |
| General Cable - Cottage | $120.00 | Dec 18, 2015 |
| General Cable - Cottage | $150.00 | Jan 9, 2016 |
| General Cable - Cottage | $120.00 | Dec 1, 2016 |
| Second Mortgage | $1,250.00 | Nov 15, 2016 |
| Second Mortgage | $1,250.00 | Sep 20, 2016 |
| Second Mortgage | $1,250.00 | Jun 14, 2016 |
| Hobby Shop | $600.00 | Jun 14, 2016 |

#### @distinctUnionOfArrays
&emsp;当指定 @distinctUnionOfArrays 运算符时，`valueForKeyPath:` 创建并返回一个数组，该数组包含与右键路径指定的属性相对应的所有集合的组合的不同对象。

&emsp;要在 arrayOfArrays 中的所有数组之间获取 payee 属性的不同值：
```c++
NSArray *collectedDistinctPayees = [arrayOfArrays valueForKeyPath:@"@distinctUnionOfArrays.payee"];
```
&emsp;所得的 collectedDistinctPayees 数组包含以下值：Hobby Shop、Mortgage、Animal Hospital、Second Mortgage、Car Loan、General Cable - Cottage、General Cable、Green Power。

> &emsp;NOTE: 与 @unionOfArrays 运算符提供类似的行为，但不会删除重复的对象。
#### @unionOfArrays
&emsp;当指定 @unionOfArrays 运算符时，`valueForKeyPath:` 创建并返回一个数组，该数组包含与由右键路径指定的属性相对应的所有集合的组合的所有对象，而不会删除重复项。

&emsp;要获取 arrayOfArrays 内所有数组中的 payee 属性值：
```c++
NSArray *collectedPayees = [arrayOfArrays valueForKeyPath:@"@unionOfArrays.payee"];
```
&emsp;所得的 collectedPayees 数组包含以下值：Green Power、Green Power、Green Power、Car Loan、Car Loan、Car Loan、General Cable、General Cable、General Cable、Mortgage、Mortgage、Mortgage、Animal Hospital、General Cable - Cottage、General Cable - Cottage、General Cable - Cottage、Second Mortgage、Second Mortgage、Second Mortgage、Hobby Shop。

> &emsp;NOTE: 与 @distinctUnionOfArrays 运算符提供类似的行为，但是删除重复的对象。

#### @distinctUnionOfSets
&emsp;当你指定 @distinctUnionOfSets 运算符时，`valueForKeyPath:` 创建并返回一个 NSSet 对象，该对象包含与右键路径指定的属性相对应的所有集合的组合的不同对象。

&emsp;此操作符的行为与 @distinctUnionOfArrays 类似，只是它需要一个包含对象的 NSSet 实例的 NSSet 实例，而不是 NSArray 实例的 NSArray 实例。此外，它还返回一个 NSSet 实例。假设示例数据存储在集合而不是数组中，那么示例调用和结果与 @distinctUnionOfArrays 的结果相同。

## Representing Non-Object Values（表示非对象值）
&emsp;NSObject 提供的键值编码协议方法的默认实现同时处理对象和非对象属性。默认实现会自动在对象参数或返回值与非对象属性之间进行转换。这允许基于键的 getter 和 setter 的签名保持一致，即使存储的属性是标量或结构。

> &emsp;NOTE: 由于 Swift 中的所有属性都是对象，因此本节仅介绍 Objective-C 属性。

&emsp;当你调用协议的一个 getter（如 `valueForKey:`）时，默认实现将根据 Accessor Search Patterns 中描述的规则确定为指定键提供值的特定访问器方法或实例变量。如果返回值不是对象，getter 将使用该值初始化 NSNumber 对象（对于标量 int/float 等）或 NSValue对象（对于结构 struct），并返回该值。

&emsp;类似地，默认情况下，setter `setValue:forKey:` 在给定特定键的情况下，确定属性的访问器或实例变量所需的数据类型。如果数据类型不是对象，那么 setter 首先向传入的 Value 对象发送一个适当的 <type>Value 消息来提取底层数据，并将其存储。

> &emsp;NOTE: 当你使用非对象属性的 nil 值调用其中一个键值编码协议 setter 时，setter 没有明显的常规操作过程可采取。因此，它向接收 setter 调用的对象发送 `setNilValueForKey:` 消息。此方法的默认实现会引发 NSInvalidArgumentException 异常，但子类可能会重写此行为，如处理非对象值中所述，例如设置标记值或提供有意义的默认值。

### Wrapping and Unwrapping Scalar Types（包装和解包标量类型 int/float 等）
&emsp;Table 5-1 列出了默认键值编码实现使用 NSNumber 实例包装的标量类型。对于每种数据类型，该表显示了用于从基础属性值初始化 NSNumber 以提供 getter 返回值的创建方法。然后，它显示了在 set 操作期间用于从 setter 输入参数中提取值的访问器方法。

&emsp;Table 5-1 Scalar types as wrapped in NSNumber objects（包装在 NSNumber 对象中的标量类型）
| Data type | Creation method | Accessor method |
| -- | -- | -- |
| BOOL | numberWithBool: | boolValue (in iOS) charValue (in macOS)* |
| char | numberWithChar: | charValue |
| double | numberWithDouble: | doubleValue |
| float | numberWithFloat: | floatValue |
| int | numberWithInt: | intValue |
| long | numberWithLong: | longValue |
| long long | numberWithLongLong: | longLongValue |
| short | numberWithShort: | shortValue |
| unsigned char | numberWithUnsignedChar: | unsignedChar |
| unsigned int | numberWithUnsignedInt: | unsignedInt |
| unsigned long | numberWithUnsignedLong: | unsignedLong |
| unsigned long long | numberWithUnsignedLongLong: | unsignedLongLong |
| unsigned short | numberWithUnsignedShort: | unsignedShort |

> &emsp;NOTE: 在 macOS 中，由于历史原因，BOOL 被定义为 signed char 类型，而 KVC 不区分这两种类型。因此，不应将 @"true" 或 @"YES" 等字符串值传递给 `setValue:forKey:` 当键是布尔类型时。KVC 将尝试调用 charValue（因为 BOOL 本身就是 char），但是 NSString 没有实现这个方法，这将导致运行时错误。相反，只传递一个 NSNumber 对象，例如 @(1) 或 @(YES)，作为参数值 `setValue:forKey:` 当键是布尔类型时。此限制不适用于 iOS，其中 BOOL 的类型定义为本机布尔类型 BOOL，而 KVC 调用 boolValue，它适用于 NSNumber 对象或格式正确的 NSString 对象。

### Wrapping and Unwrapping Structures（包装和展开结构 Struct）
&emsp;Table 5-2 显示了创建和访问器方法，默认访问器使用它们来包装和解包常见的 NSPoint、NSRange、NSRect、NSSize 结构。
```c++
struct CGPoint {
    CGFloat x;
    CGFloat y;
};

typedef struct _NSRange {
    NSUInteger location;
    NSUInteger length;
} NSRange;

struct CGRect {
    CGPoint origin;
    CGSize size;
};

struct CGSize {
    CGFloat width;
    CGFloat height;
};
```
&emsp;Table 5-2 Common struct types as wrapped using NSValue（使用 NSValue 包装的常见结构类型）
| Data type | Creation method | Accessor method |
| -- | -- | -- |
| NSPoint | valueWithPoint: | pointValue |
| NSRange | valueWithRange: | rangeValue |
| NSRect | valueWithRect:(macOS only) | rectValue |
| NSSize | valueWithSize: | sizeValue |

&emsp;自动包装和展开不限于 NSPoint、NSRange、NSRect 和 NSSize。结构类型（即 Objective-C 类型编码字符串以 { 开头的类型）可以包装在 NSValue 对象中。例如，考虑 Listing 5-1 中声明的结构和类接口。

&emsp;Listing 5-1 A sample class using a custom structure（使用自定义结构的示例类）
```c++
typedef struct {
    float x, y, z;
} ThreeFloats;
 
@interface MyClass
@property (nonatomic) ThreeFloats threeFloats;
@end
```
&emsp;使用名为 myClass 的类的实例，可以通过键值编码获得 threeFloats 值：
```c++
NSValue* result = [myClass valueForKey:@"threeFloats"];
```
&emsp;`valueForKey:` 的默认实现会调用 threeFloats getter，然后返回包装在 NSValue 对象中的结果。

&emsp;同样，你可以使用键值编码设置 threeFloats 值：
```c++
ThreeFloats floats = {1., 2., 3.};
NSValue* value = [NSValue valueWithBytes:&floats objCType:@encode(ThreeFloats)];

[myClass setValue:value forKey:@"threeFloats"];
```
&emsp;默认实现使用 `getValue:` 消息解包该值，然后使用结果结构调用 `setThreeFloats:`。

## Validating Properties（验证属性）
&emsp;键值编码协议定义了支持属性验证的方法。就像使用基于键的访问器来读写键值编码兼容对象的属性一样，还可以按键（或键路径）验证属性。当你调用  `validateValue:forKey:error:`（或 `validateValue:forKeyPath:error:`）方法时，协议的默认实现会在接收验证消息的对象（或键路径末尾的对象）中搜索一种方法，其名称与模式 `validate<Key>:error:` 匹配。如果对象没有这样的方法，则默认情况下验证成功，默认实现返回 YES。当存在特定于属性的验证方法时，默认实现将返回调用该方法的结果。

> &emsp;NOTE: 通常只使用 Objective-C 中描述的验证。在 Swift 中，属性验证更习惯于依赖编译器对选项和强类型检查（optionals and strong type checking）的支持来处理，同时使用内置的 `willSet` 和 `didSet` 属性观察者来测试任何运行时 API contracts，如 The Swift Programming Language (Swift 3) 中 Property Observers 的部分的所述。

&emsp;由于特定于属性的验证方法通过引用接收值和错误参数，因此验证具有三种可能的结果：
1. 验证方法认为 value 对象有效，并返回 YES 而不更改值或 error。
2. 验证方法认为 value 对象无效，但选择不更改它。在这种情况下，该方法返回 NO 并将错误引用（如果有，则调用方提供）设置为指示失败原因的 NSError 对象。
3. 验证方法认为 value 对象无效，但创建一个新的有效对象作为替换。在本例中，该方法返回 YES，同时保持 error 对象不变。在返回之前，该方法修改值引用以指向新的值对象。当方法进行修改时，它总是创建一个新对象，而不是修改旧对象，即使值对象是可变的。

&emsp;Listing 6-1 显示了如何调用 name 字符串验证的示例。

&emsp;Listing 6-1 Validation of the name property（验证 name 属性）
```c++
Person* person = [[Person alloc] init];

NSError* error;
NSString* name = @"John";

if (![person validateValue:&name forKey:@"name" error:&error]) {
    NSLog(@"%@",error);
}
```

### Automatic Validation
&emsp;一般来说，键值编码协议及其默认实现都没有定义任何机制来自动执行验证。相反，你可以在适合你的应用程序时使用验证方法。

&emsp;某些其他 Cocoa 技术在某些情况下会自动执行验证。例如，保存托管对象上下文时，Core Data 会自动执行验证（请参阅 Core Data Programming Guide）。另外，在 macOS 中，Cocoa bindings 允许你指定自动进行验证（有关更多信息，请参阅  Cocoa Bindings Programming Topics）。

## Accessor Search Patterns（访问者搜索模式）
&emsp;NSObject 提供的 NSKeyValueCoding 协议的默认实现使用一组明确定义的规则将基于键的访问器调用映射到对象的底层属性。这些协议方法使用一个键参数来搜索它们自己的对象实例，以查找访问器、实例变量和遵循某些命名约定的相关方法。尽管很少修改此默认搜索，但了解它的工作方式会很有帮助，既可以跟踪键值编码对象的行为，也可以使你自己的对象兼容。

> &emsp;NOTE: 本节中的描述使用 <key> 或 <Key> 作为键的占位符，当该键在键值编码协议方法之一中作为参数出现时。然后由该方法用作辅助方法调用的一部分或变量名查找。映射的属性名称遵循占位符的大小写。例如对于 getters <key> 和 is<Key>，名为 hidden 的属性将映射为 hidden 和 isHidden。

### Search Pattern for the Basic Getter（基本 Getter 的搜索模式）
&emsp;在给定键参数作为输入的情况下，`valueForKey:` 的默认实现执行以下过程。（在接收 `valueForKey:` 调用的类实例内部进行操作）

1. 在实例中搜索第一个名为 `get<Key>`、`<key>`、`is<Key>` 或 `\_<key>` 的访问器方法。如果找到了，则调用它并继续执行步骤 5 并返回结果。否则继续下一步。（**get<Key>、<key>、is<Key> 或 \_<key>**）

2. 如果找不到简单的访问器方法，在实例中搜索名称与 `countOf<Key>`、`objectIn<Key>AtIndex:`（对应于 NSArray 类定义的原始方法） 和 `<key>AtIndexes:`（对应于 NSArray 的 `objectsAtIndexes:` 方法）模式匹配的方法。
  如果找到其中的第一个以及其他两个中的至少一个，则创建一个响应所有 NSArray 方法的集合代理对象（collection proxy object），并返回该对象。否则，请继续执行步骤 3。
  代理对象随后将接收到的任何 NSArray 消息转换为 `countOf<Key>`、`objectIn<Key>AtIndex:` 和 `<Key>AtIndexes:` 消息的组合，并将其转换为创建它的键值编码兼容对象。如果原始对象还实现了一个名为 `get<Key>:range:` 之类的可选方法，则代理对象也将在适当时使用该方法。实际上，代理对象与键值编码兼容的对象一起工作，允许底层属性的行为就像 NSArray 一样，即使它不是。
  
3. 如果找不到简单的访问器方法或数组访问方法组，请查找名为 `countOf<Key>`、`enumeratorOf<Key>` 和 `memberOf<Key>` 的三重方法。（对应于 NSSet 类定义的原始方法）
  如果找到所有三个方法，请创建一个响应所有 NSSet 方法的集合代理对象，并返回该对象。否则，继续执行步骤 4。
  代理对象随后将接收到的任何 NSSet 消息转换为 `countOf<Key>`、`enumeratorOf<Key>` 和 `memberOf<Key>` 消息的某种组合，以创建它的对象。实际上，代理对象与键值编码兼容对象一起工作，使得基础属性的行为就像 NSSet 一样，即使它不是 NSSet。
  
4. 如果找不到简单的访问器方法或集合访问方法组，并且如果 receiver 的类方法 `accessInstanceVariablesDirectly` 返回 YES，则按该顺序搜索名为 `\_<key>`、`\_is<Key>`、`<key>` 或 `is<Key>` 的实例变量。如果找到，则直接获取实例变量的值并继续执行步骤 5。否则，继续进行步骤 6。（**\_<key>、\_is<Key>、<key> 或 is<Key>**）

5. 如果检索到的属性值是对象指针，则只需返回结果。
  如果该值是 NSNumber 支持的标量类型，则将其存储在 NSNumber 实例中并返回它。
  如果结果是 NSNumber 不支持的标量类型，则转换为 NSValue 对象并返回该对象。
  
6. 如果所有方法均失败，则调用 `valueForUndefinedKey:`。默认情况下，这会引发一个异常，但是 NSObject 的子类可以提供特定于键的行为（子类重写 `valueForUndefinedKey:` 函数）。

### Search Pattern for the Basic Setter（基本 Setter 的搜索模式）
&emsp;`setValue:forKey:` 的默认实现，给定 key 和 value 参数作为输入，尝试将名为 key 的属性设置为 value，使用以下过程在接收到调用的对象内部：（对于非对象的属性，则为 value 的展开版本，如 Representing Non-Object Values 中所述）

1. 按此顺序查找名为 `set<Key>:` 或 `\_set<Key>` 的第一个访问器。如果找到了，则使用 value（或根据需要解包 value 的值）调用它并完成。（**set<Key>:、\_set<Key>**）

2. 如果未找到简单的访问器，并且类方法 `accessInstanceVariablesDirectly` 返回 YES，按该顺序查找名称类似于 `\_<key>`、`\_is<Key>`、`<key>` 或 `is<Key>` 的实例变量。如果找到，则直接使用 value（或根据需要解包 value 的值）设置实例变量并完成操作。（**\_<key>、\_is<Key>、<key> 或 is<Key>**）

3. 在找不到访问器或实例变量时，调用 `setValue:forUndefinedKey:`。这在默认情况下会引发异常，但 NSObject 的子类可能会提供键特定的行为。（由子类重写 `setValue:forUndefinedKey:`）

### Search Pattern for Mutable Arrays（可变数组的搜索模式）
&emsp;`mutableArrayValueForKey:` 的默认实现，给定一个 key 参数作为输入，为接收访问器调用的对象内的名为 key 的属性返回一个可变的代理数组，使用以下过程：

1. 搜索一对名字如 `insertObject:in<Key>AtIndex:` 和 `removeObjectFrom<Key>AtIndex:`（分别对应于 NSMutableArray 的原始方法 `insertObject:atIndex:` 和 `removeObjectAtIndex:`）的方法，或者名字如 `insert<Key>:atIndexes:` 和 `remove<Key>AtIndexes:`（对应于 NSMutableArray 的 `insertObjects:atIndexes:` 和 `removeObjectsAtIndexes:` 方法）的方法。
  如果对象具有至少一种 insertion 方法和至少一种 removal 方法，通过发送 `insertObject:in<Key>AtIndex:`、`removeObjectFrom<Key>AtIndex:`、`insert<Key>:atIndexes:` 消息的某种组合，返回一个响应 NSMutableArray 消息的代理对象，并将 `remove<Key>AtIndexes:` 消息发送给 `mutableArrayValueForKey:` 的原始接收者。
  当接收到 `mutableArrayValueForKey:` 消息的对象还实现了一个可选的 replace 对象方法，其名称类似于 `replaceObjectIn<Key>AtIndex:withObject:` 或 `replace<Key>AtIndexes:with<Key>:` 时，代理对象也会在适当的时候也利用这些对象以获得最佳性能。
  
2. 如果对象没有可变数组方法，查找名称与 `set<Key>:` 匹配的访问器方法。 在这种情况下，通过向 `mutableArrayValueForKey:` 的原始接收者发出 `set<Key>:` 消息，返回响应 NSMutableArray 消息的代理对象。

> &emsp;NOTE: 该步骤中描述的机制比上一步的机制效率低得多，因为它可能涉及重复创建新的集合对象而不是修改现有集合对象。因此，在设计自己的键值编码兼容对象时，通常应避免使用它。

3. 如果既没有找到可变数组方法，也没有找到访问器，并且如果 receiver 的类方法 `accessInstanceVariablesDirectly` 返回 YES，则按该顺序搜索名为 `\_<key>` 或 `<key>` 的实例变量。
  如果找到了这样的实例变量，则返回一个代理对象，该代理对象将接收到的每个 NSMutableArray 消息转发到该实例变量的值，该值通常是 NSMutableArray 或其子类之一的实例。
  
4. 如果所有其他操作都失败，则发送给 `mutableArrayValueForKey:` 消息的原始接收方只要接收到 NSMutableArray 消息时则发出 `setValue:forUndefinedKey:` 消息。
  `setValue:forUndefinedKey:` 引发 NSUndefinedKeyException，但子类可以重写此方法。

## Achieving Basic Key-Value Coding Compliance（实现基本键值编码合规性）
&emsp;当对对象采用键值编码时，你依赖于 NSKeyValueCoding 协议的默认实现，方法是让你的对象继承自 NSObject 或其许多子类之一。反过来，默认实现依赖于你按照某些良好定义的模式，所定义对象的实例变量（或 ivar）和访问器方法，以便在接收键值编码的消息时可以将键字符串与属性相关联。例如 `valueForKey:` 和 `setValue:forKey:`。

&emsp;你通常通过简单地使用 @property 语句声明属性，并允许编译器自动合成 ivar 和访问器，来遵守 Objective-C 中的标准模式。默认情况下，编译器遵循预期的模式。

> &emsp;NOTE: 在 Swift 中，只需以通常的方式声明属性即可自动生成适当的访问器，而你永远不会直接与 ivars 进行交互。有关 Swift 中的属性的更多信息，请阅读 The Swift Programming Language（Swift 3）中的 Properties。有关与 Swift 中的 Objective-C 属性进行交互的特定信息，请阅读 Using Swift with Cocoa and Objective-C (Swift 3) 中的 Accessing Properties。

&emsp;如果确实需要在 Objective-C 中手动实现访问器或 ivar，请遵循本节中的指导原则以保持基本的遵从性。要提供增强与任何语言的对象集合属性交互的附加功能，请实现 Defining Collection Methods 中描述的方法。要通过键值验证进一步增强对象，请实现 Adding Validation 中描述的方法。

> &emsp;NOTE: 键值编码的默认实现比此处描述的更广泛的 ivars 和访问器可以使用。如果你有使用其他变量或访问器约定的旧版代码，请检查 Accessor Search Patterns 中的搜索模式，以确定默认实现是否可以找到你的对象的属性。

### Basic Getters
&emsp;要实现返回属性值的 getter，同时可能还要执行其他自定义工作，请使用名为属性的方法，例如 title 字符串属性：
```c++
- (NSString*)title {
   // Extra getter logic…
   
   return _title;
}
```
&emsp;对于具有布尔类型的属性，你也可以使用以 is 前缀的方法，例如名为 hidden 的 Boolean 类型属性：
```c++
- (BOOL)isHidden {
   // Extra getter logic…
 
   return _hidden;
}
```
&emsp;当属性是标量或结构时，键值编码的默认实现将值包装到对象中，以便在协议方法的接口上使用，如 Representing Non-Object Values 中所述。你不需要做任何特殊的事情来支持这种行为。

### Basic Setters
&emsp;要实现用于存储属性值的 setter，请使用带有以单词 set 开头的属性大写名称首字母的方法。对于 hidden 属性：
```c++
- (void)setHidden:(BOOL)hidden {
    // Extra setter logic…
 
   _hidden = hidden;
}
```
> &emsp;WARNING: 切勿从 `set<Key>:` 方法内部调用 Validating Properties 中描述的 validation 方法。

&emsp;当属性是非对象类型（如 Boolean 类型的 hidden 属性）时，协议的默认实现将检测基础数据类型，并解开来自 `setValue:forKey:` 的对象值（在这种情况下为 NSNumber 实例），然后再将其应用于 setter，如  Representing Non-Object Values 中所述。你不需要在 setter 本身中处理这个问题。但是，如果可能会将 nil 值写入非对象属性，则可以重写 `setNilValueForKey:` 以处理这种情况，如 Handling Non-Object Values 中所述。hidden 属性的适当行为可能只是将 nil 解释为 NO：
```c++
- (void)setNilValueForKey:(NSString *)key {
    if ([key isEqualToString:@"hidden"]) {
        [self setValue:@(NO) forKey:@"hidden"];
    } else {
        [super setNilValueForKey:key];
    }
}
```
&emsp;即使在允许编译器合成 setter 的情况下，也可以提供上述方法重写（如果适用）。

### Instance Variables
&emsp;当某个键值编码访问器方法的默认实现找不到属性的访问器时，它会直接查询其类的 `accessInstanceVariablesDirectly` 方法，查看该类是否允许直接使用实例变量。默认情况下，该类方法返回 YES，但你可以重写该方法以返回 NO。

&emsp;如果你确实允许使用 ivars，请确保以常规方式命名，并使用带下划线（\_）前缀的属性名称。通常，编译器会在自动合成属性时为你执行此操作，但是如果你使用显式的 @synthesize 指令，则可以自己强制执行此命名：
```c++
@synthesize title = _title;
```
&emsp;在某些情况下，不使用 @synthesis 指令或允许编译器自动合成属性，而是使用 @dynamic 指令通知编译器将在运行时提供 getter 和 setter。这样做可以避免自动合成 getter，这样就可以提供集合访问器，如 Defining Collection Methods 中所述。在这种情况下，你自己声明 ivar 作为接口声明的一部分：
```c++
@interface MyObject : NSObject {
    NSString* _title;
}
 
@property (nonatomic) NSString* title;
 
@end
```

## Defining Collection Methods（定义集合方法）
&emsp;当你使用标准命名约定创建访问器（getter）和 ivar 时（如 Achieving Basic Key-Value Coding Compliance 中所述），键值编码协议的默认实现可以根据键值编码消息找到它们。这一点对于表示一对多关系的集合类型的属性和其他一对一关系的属性一样正确。但是，如果代替集合属性的基本访问器或在集合属性的基础上实现集合访问器方法，则可以：

+ **Model to-many relationships with classes other than NSArray or NSSet（使用 NSArray 或 NSSet 以外的类建立一对多关系模型）.** 在对象中实现集合方法时，键值 getter 的默认实现返回一个代理对象，该代理对象调用这些方法以响应它接收到的后续 NSArray 或 NSSet 消息。底层属性对象不必是 NSArray 或 NSSet 本身，因为代理对象使用你的集合方法提供预期的行为。
+ **Achieve increased performance when mutating the contents of a to-many relationship（更改一对多关系的内容时实现更高的性能）.**
  协议的默认实现使用你的集合方法来改变基础属性，而不是使用 basic setter 重复创建新的集合对象来响应每一个更改。（即我们上面提到的使用 `mutableArrayValueForKey:` 函数来直接修改 personArray 属性数组）
+ **Provide key-value observing compliant access to the contents of your object’s collection properties（提供对 key-value observing 的合规访问权限，以访问对象的集合属性的内容）.** 有关键值观察的更多信息，请阅读 Key-Value Observing Programming Guide。

&emsp;你可以实现两类集合访问器中的一种，具体取决于你希望关系的行为类似于索引的有序集合（如 NSArray）还是无序的唯一集合（如 NSSet）。在这两种情况下，你至少要实现一组方法来支持对属性的读取访问，然后再添加一组方法来启用集合内容的可变性。

> &emsp;NOTE: 键值编码协议不声明本节中描述的方法。相反，NSObject 提供的协议的默认实现会在符合键值编码的对象中查找这些方法，如 Accessor Search Patterns 中所述，并使用它们处理作为协议一部分的键值编码消息。

&emsp;**对应我们上面的定义的 Student 类，当我们在 @implementation Student 内输入以下函数的前面的几个字母 Xcode 都会提示我们生成对应的函数。**
&emsp;**对应我们上面的定义的 Student 类，当我们在 @implementation Student 内输入以下函数的前面的几个字母 Xcode 都会提示我们生成对应的函数。**

### Accessing Indexed Collections（访问索引集合）
&emsp;你可以添加索引访问器方法（indexed accessor methods），以提供一种用于对有序关系中的对象进行计数（counting）、检索（retrieving）、添加（adding）和替换（replacing ）的机制。底层对象通常是 NSArray 或 NSMutableArray 的实例，但是如果你提供集合访问器（collection accessors），则可以将实现这些方法的任何对象属性当作数组来操作。

#### Indexed Collection Getters（索引集合获取器）
&emsp;对于没有默认 getter 的集合类型属性，如果提供以下索引集合 getter 方法，则该协议的默认实现将响应 `valueForKey:` 消息，返回一个行为类似于 NSArray 的代理对象，但调用以下集合方法来完成其工作。

> &emsp;NOTE: 在现代 Objective-C 中，编译器默认为每个属性合成一个 getter，因此默认实现不会创建使用本节中方法的只读代理（请注意基本 getter 的访问器搜索顺序）。你可以通过不声明属性（仅依赖于 ivar）或将属性声明为 @dynamic（表示你计划在运行时提供访问器行为）来绕过此问题。无论哪种方式，编译器都不会提供默认 getter，默认实现使用以下方法。 

+ countOf<Key>
  该方法以 NSUInteger 的形式返回一对多关系中的对象数，就像 NSArray 基本方法 count 一样。实际上，当基础属性是 NSArray 时，可以使用该方法来提供结果。
  例如，对于表示银行交易列表并由 NSArray 支持的一对多关系（称为 transactions）：
```c++
- (NSUInteger)countOfTransactions {
    return [self.transactions count];
}
```
+ objectIn<Key>AtIndex: 或 <key>AtIndexes:
  第一个返回对象在一对多关系中指定索引处的对象，而第二个返回在 NSIndexSet 参数指定的索引处的对象数组。它们分别对应于 NSArray 方法 `objectAtIndex:` 和 `objectsAtIndexes:`。你只需要实现其中一个。transactions 数组的相应方法是：
```c++
- (id)objectInTransactionsAtIndex:(NSUInteger)index {
    return [self.transactions objectAtIndex:index];
}
 
- (NSArray *)transactionsAtIndexes:(NSIndexSet *)indexes {
    return [self.transactions objectsAtIndexes:indexes];
}
```
+ get<Key>:range:
  此方法是可选的，但可以提高性能。它从集合中返回属于指定范围内的对象，并与 NSArray 的 `getObjects:range:` 方法相对应。 transactions 数组的实现是
```c++
- (void)getTransactions:(Transaction * __unsafe_unretained *)buffer range:(NSRange)inRange {
    [self.transactions getObjects:buffer range:inRange];
}
```

#### Indexed Collection Mutators
&emsp;支持与索引访问器的可变一对多关系需要实现一组不同的方法。当你提供这些 setter 方法时，响应 `mutableArrayValueForKey:` 消息的默认实现将返回一个代理对象，其行为类似于 NSMutableArray 对象，但使用对象的方法来执行其工作。这通常比直接返回 NSMutableArray 对象更有效。它还使一对多关系的内容符合键值观察（请参阅 Key-Value Observing Programming Guide）。

&emsp;为了使你的对象键值编码符合可变的有序一对多关系，请实现以下方法：

+ insertObject:in<Key>AtIndex: 或 insert<Key>:atIndexes: 
  第一个接收要插入的对象和一个整数，该整数指定要插入该对象的索引。第二种方法在传递的 NSIndexSet 指定的索引处向集合中插入一个对象数组。这些方法类似于 NSMutableArray 的 `insertObject:atIndex:` 和 `insertObjects:atIndexes:`。 只需要这些方法中的一种。
  对于声明为 NSMutableArray 的 transactions 对象：
```c++
- (void)insertObject:(Transaction *)transaction inTransactionsAtIndex:(NSUInteger)index {
    [self.transactions insertObject:transaction atIndex:index];
}
 
- (void)insertTransactions:(NSArray *)transactionArray atIndexes:(NSIndexSet *)indexes {
    [self.transactions insertObjects:transactionArray atIndexes:indexes];
}
```
+ removeObjectFrom<Key>AtIndex: 或 remove<Key>AtIndexes:
  第一个接收一个 NSUInteger 值，该值指定要从 relationship 中删除的对象的索引。第二个接收一个 NSIndexSet 对象，该对象指定要从 relationship 中删除的对象的索引。这些方法分别对应于 NSMutableArray 的 `removeObjectAtIndex:` 和 `removeObjectsAtIndexes:`。这些方法只需要一种。
  对于 transactions 对象：
```c++
- (void)removeObjectFromTransactionsAtIndex:(NSUInteger)index {
    [self.transactions removeObjectAtIndex:index];
}
 
- (void)removeTransactionsAtIndexes:(NSIndexSet *)indexes {
    [self.transactions removeObjectsAtIndexes:indexes];
}
```
+ replaceObjectIn<Key>AtIndex:withObject: 或 replace<Key>AtIndexes:with<Key>:
  这些替换访问器为代理对象提供了一种直接替换集合中对象的方法，而不必依次删除一个对象并插入另一个对象。它们对应于 NSMutableArray 的 `replaceObjectAtIndex:withObject:` 和 `replaceObjectsAtIndexes:withObjects:`。当对应用程序进行分析发现性能问题时，可以选择提供这些方法。
  对于 transactions 对象：
```c++
- (void)replaceObjectInTransactionsAtIndex:(NSUInteger)index withObject:(id)anObject {
    [self.transactions replaceObjectAtIndex:index withObject:anObject];
}
 
- (void)replaceTransactionsAtIndexes:(NSIndexSet *)indexes withTransactions:(NSArray *)transactionArray {
    [self.transactions replaceObjectsAtIndexes:indexes withObjects:transactionArray];
}
```

### Accessing Unordered Collections
&emsp;你可以添加无序集合访问器方法，以提供一种机制来访问和改变无序关系中的对象。通常，此关系是 NSSet 或 NSMutableSet 对象的实例。但是，在实现这些访问器时，可以启用任何类来建模关系，并使用键值编码进行操作，就像它是 NSSet 的实例一样。

#### Unordered Collection Getters
&emsp;当你提供以下集合 getter 方法来返回集合中的对象数、迭代集合对象并测试集合中是否已存在对象时，协议的默认实现将响应 `valueForKey:` 消息，返回一个行为类似 NSSet 的代理对象，但调用以下集合方法来完成其工作。

> &emsp;NOTE: 在现代的 Objective-C 中，编译器默认情况下会为每个属性合成一个 getter，因此默认实现不会创建使用此部分中方法的只读代理（请注意 Search Pattern for the Basic Getter）。你可以通过不声明属性（仅依靠 ivar）或将属性声明为 @dynamic 来解决此问题，这表明你计划在运行时提供访问器行为。无论哪种方式，编译器都不会提供默认的 getter，并且默认的实现使用以下方法。

+ countOf<Key>
  此必需方法返回 relationship 中与 NSSet 方法 count 相对应的项目数。当基础对象是 NSSet 时，可以直接调用此方法。例如，对于一个名为 employee 的 NSSet 对象，其中包含 Employee 对象：
```c++
- (NSUInteger)countOfEmployees {
    return [self.employees count];
}
```
+ enumeratorOf<Key> 
  此必需方法返回一个 NSEnumerator 实例，该实例用于遍历 relationship 中的 items。有关枚举器的更多信息，请参见 Collections Programming Topics 中的 Enumeration: Traversing a Collection’s Elements。该方法对应于 NSSet 类中的 objectEnumerator。For the employees set:
```c++
- (NSEnumerator *)enumeratorOfEmployees {
    return [self.employees objectEnumerator];
}
```
+ memberOf<Key>: 
  此方法将作为参数传递的对象与集合中的对象进行比较，并返回匹配的对象作为结果，如果找不到匹配的对象，则返回 nil。如果手动实现比较方法，则通常使用 isEqual: 来比较对象。当基础对象是 NSSet 对象时，可以使用等效的 member: 方法：
```c++
- (Employee *)memberOfEmployees:(Employee *)anObject {
    return [self.employees member:anObject];
}
```

#### Unordered Collection Mutators
&emsp;要支持与无序访问器的可变的一对多关系，需要实现额外的方法。实现可变无序访问器，以允许你的对象提供无序集合代理对象，以响应 `mutableSetValueForKey:` 方法。实现这些访问器比依赖访问器直接返回可变对象以对 relationship 中的数据进行更改要高效得多。这也使你的类的集合中的对象符合 key-value observing 兼容性（请参阅  Key-Value Observing Programming Guide）。

&emsp;为了对可变的无序一对多 relationship 进行键值编码，请实施以下方法：

+ add<Key>Object: 或 add<Key>: 
  这些方法将单个项或一组项添加到 relationship 中。向 relationship 中添加一组项时，请确保 relationship 中尚未存在等效对象。这些方法类似于 NSMutableSet 的 `addObject:` 和 `unionSet:`。只需要这些方法中的一种。对于 employees 集合如下：
```c++
- (void)addEmployeesObject:(Employee *)anObject {
    [self.employees addObject:anObject];
}
 
- (void)addEmployees:(NSSet *)manyObjects {
    [self.employees unionSet:manyObjects];
}
```
+ remove<Key>Object: 或 remove<Key>: 
  这些方法从 relationship 中删除单个项或一组项。它们类似于 NSMutableSet 的 `removeObject:` 和 `minusSet:`。这些方法只需要一种。例如：
```c++
- (void)removeEmployeesObject:(Employee *)anObject {
    [self.employees removeObject:anObject];
}
 
- (void)removeEmployees:(NSSet *)manyObjects {
    [self.employees minusSet:manyObjects];
}
```
+ intersect<Key>: 
  接收到 NSSet 参数的此方法将从 relationship 中删除输入集和集合集都不通用的所有对象。这等效于 NSMutableSet 的 intersectSet:。当分析表明出现与集合内容更新有关的性能问题时，可以选择实现此方法。例如：
```c++
- (void)intersectEmployees:(NSSet *)otherObjects {
    return [self.employees intersectSet:otherObjects];
}
```

## Handling Non-Object Values（处理非对象值）
&emsp;通常，与键值编码兼容的对象依赖键值编码的默认实现来自动包装和解包非对象属性，如 Representing Non-Object Values 中所述。但是，可以重写默认行为。这样做的最常见原因是处理在非对象属性上存储 nil 值的尝试。

> &emsp;NOTE: 由于 Swift 中的所有属性都是对象，因此本节仅适用于 Objective-C 属性。

&emsp;如果键值编码兼容对象收到了 `setValue:forKey:` 将 nil 作为非对象属性的 value 的消息，默认实现没有适当的通用操作过程。因此，它会向自己发送一条 `setNilValueForKey:` 消息，你可以重写它。`setNilValueForKey:` 的默认实现引发 NSInvalidArgumentException 异常，但你可以提供适当的、特定于实现的行为。

&emsp;例如，Listing 10-1 中的代码响应了将一个人的年龄设置为 nil 值的尝试，其内部把年龄设置为 0，这对于浮点值更合适。请注意，重写方法为它没有显式处理的任何键调用其超类的 `setNilValueForKey:` 函数。（`[super setNilValueForKey:key]`）
 
&emsp;Listing 10-1 Example implementation of setNilValueForKey:（`setNilValueForKey:` 的示例实现）
```c++
- (void)setNilValueForKey:(NSString *)key {
    if ([key isEqualToString:@"age"]) {
        [self setValue:@(0) forKey:@"age"];
    } else {
        [super setNilValueForKey:key];
    }
}
```
> &emsp;NOTE: 为了向后兼容，当对象重写了不推荐使用的 `unableToSetNilForKey:` 方法时，`setValue:forKey:` 会调用该方法而不是 `setNilValueForKey:`。

## Adding Validation
&emsp;键值编码协议定义了按键或键路径验证属性的方法。这些方法的默认实现反过来依赖于你按照与访问器方法类似的命名模式定义的方法。具体来说，你可以为你想要验证的名称为 key 的任何属性提供 `validate<Key>:error:` 方法。默认实现会搜索这个值，以响应键编码的 `validateValue:forKey:error:` 消息。

&emsp;如果不为属性提供验证方法，则协议的默认实现假定该属性的验证成功，而不管值是多少。这意味着你选择逐个属性进行验证。

> &emsp;NOTE: 你通常只使用 Objective-C 中描述的验证。在 Swift 中，属性验证更习惯于依赖编译器对选项和强类型检查的支持来处理，同时使用内置的 `willSet` 和 `didSet` 属性观察者来测试任何运行时 API contracts，如 The Swift Programming Language (Swift 3) 的 property Observators 部分所述 。

### Implementing a Validation Method
&emsp;当你为属性提供验证方法时，该方法通过引用（指针的地址（id *，NSError **）为了在函数内部修改原始指针的指向）接收两个参数：要验证的 value 对象的指针地址和用于返回错误信息的 NSError 对象指针的地址。因此，你的验证方法可以采取以下三种操作之一：

+ 当值 value 有效时，返回 YES 而不更改 value 或 error。
+ 如果 value 无效，并且你不能或不想提供有效的替代方法，将 error 参数设置为指示失败原因的 NSError 对象，并返回 NO。

> &emsp;IMPORTANT: 在尝试设置 error 引用之前，先判断该 NSError ** error 参数是否不为 nil。

+ 如果 value 无效，但知道有效的替代方法，请创建有效对象，将 value 指向这个创建的有效对象，然后返回 YES 而不修改 error。如果提供另一个 value，则始终返回新对象，而不是修改正在验证的对象，即使原始对象是可变的。（这里大概是指 value 入参是一个自己的临时变量什么的吗？而不是要验证的对象的属性）

&emsp;Listing 11-1 演示了一个 name string 属性的验证方法，该方法确保 value 对象不是 nil，并且名称是最小长度。如果验证失败，此方法不会替换其他值。

&emsp;Listing 11-1 Validation method for the name property（name 属性的验证方法）
```c++
- (BOOL)validateName:(id *)ioValue error:(NSError * __autoreleasing *)outError{
    if ((*ioValue == nil) || ([(NSString *)*ioValue length] < 2)) {
        if (outError != NULL) {
            *outError = [NSError errorWithDomain:PersonErrorDomain
                                            code:PersonInvalidNameCode
                                        userInfo:@{ NSLocalizedDescriptionKey
                                                    : @"Name too short" }];
        }
        return NO;
    }
    return YES;
}
```
### Validation of Scalar Values
&emsp;验证方法期望 value 参数是一个对象类型，因此，非对象属性的值会被包装在 NSValue 或 NSNumber 对象中，如 Representing Non-Object Values 中所述。Listing 11-2 中的示例演示了标量属性 age 的验证方法。在这种情况下，通过创建设置为零的有效值并返回 YES 来处理一个潜在的无效条件，即 nil age 值。还可以在 `setNilValueForKey:` 重载中处理这个特定条件，因为类的用户可能不会调用验证方法而去直接给 age 设置 nil 值。

&emsp;Listing 11-2 Validation method for a scalar property
```c++
- (BOOL)validateAge:(id *)ioValue error:(NSError * __autoreleasing *)outError {
    if (*ioValue == nil) {
        // Value is nil: Might also handle in setNilValueForKey
        // Value 是 nil，可能还会在 setNilValueForKey 函数中处理
        *ioValue = @(0);
    } else if ([*ioValue floatValue] < 0.0) {
        if (outError != NULL) {
            *outError = [NSError errorWithDomain:PersonErrorDomain
                                            code:PersonInvalidAgeCode
                                        userInfo:@{ NSLocalizedDescriptionKey
                                                    : @"Age cannot be negative" }];
        }
        return NO;
    }
    return YES;
}
```
## Describing Property Relationships（描述属性关系）
&emsp;类描述（Class descriptions）提供了一种方法来描述类中的一对一和一对多属性。通过定义类属性之间的这些关系，可以使用键值编码对这些属性进行更智能、更灵活的操作。

### Class Descriptions
&emsp;NSClassDescription 是一个基类，提供获取类元数据的接口。类描述对象记录特定类的对象的可用属性以及该类的对象与其他对象之间的关系（一对一、一对多和反向）。例如，`attributeKeys` 方法返回为类定义的所有属性的列表；`ToAnyRelationshipKeys` 和 `ToOnRelationshipKeys` 方法返回定义多对一关系的键数组；`inverseRelationshipKey:` 返回从所提供键的关系的目标指向 receiver  的关系的名称。

&emsp;NSClassDescription 不定义用于定义关系的方法。具体的子类必须定义这些方法。一旦创建，你可以使用 NSClassDescription 的 `registerClassDescription:forClass:` 类方法注册一个类描述。

&emsp;NSScriptClassDescription 是 Cocoa 中提供的 NSClassDescription 的唯一具体子类。它封装了应用程序的脚本信息。

## Designing for Performance
&emsp;键值编码是高效的，尤其是当你依靠默认实现来完成大部分工作时，但是它确实添加了一个间接级别，该级别比直接的方法调用稍慢。只有当你可以从它提供的灵活性中获益或者允许你的对象参与依赖于它的 Cocoa 技术时，才使用键值编码。

### Overriding Key-Value Coding Methods（覆盖键值编码方法）
&emsp;通常，通过确保对象继承自 NSObject，然后提供本文档中描述的特定于属性的访问器和相关方法，即可以使得对象符合键值编码的要求。很少需要重写键值编码访问器的默认实现，例如 `valueForKey:` 和 `setValue:forKey:`，或基于键的验证方法，如 `validateValue:forKey:`。因为这些实现会缓存有关运行时环境的信息以提高效率，所以如果你重写它们以引入自定义逻辑，请确保在返回前调用超类中的默认实现。

### Optimizing To-Many Relationships（优化一对多关系）
&emsp;当实现一对多关系时，在许多情况下，尤其是对于可变集合，访问器的索引形式可显着提高性能。有关更多信息，请参见 Accessing Collection Properties 和 Defining Collection Methods。

## Compliance Checklist（符合 KVC 编码要求的检查表）
&emsp;请执行本节中概述的步骤，以确保你的对象符合键值编码。有关详细信息，请参见前面的部分。

### Attribute and To-One Relationship Compliance（属性和一对一关系的合规性）
&emsp;对于作为属性或一对一关系的每个属性：

&emsp;☑️ 实现名为 `<key>` 或 `is<Key>` 的方法，或创建实例变量 `<key>` 或  `\_<key>`。编译器通常在自动合成属性时为你执行此操作。（即当我们在类定义中添加一个属性时，编译器通常会自动为我们生成该属性的 setter getter 和一个名字是 \_属性名 的成员变量。）

> &emsp;NOTE: 尽管属性名称通常以小写字母开头，但是协议的默认实现也可以使用以大写字母开头的名称，例如 URL。

&emsp;☑️ 如果该属性是可变的，则实现 `set<Key>:` 方法。允许编译器自动合成（automatically synthesize）属性时，编译器通常会为你执行此操作。

> &emsp;IMPORTANT: 如果你重写默认的 setter，请确保不要调用协议的任何验证方法（validation methods）。

&emsp;☑️ 如果该属性是标量，请重写 `setNilValueForKey:` 方法，以优雅地处理将 nil 值赋给标量属性的情况。

### Indexed To-Many Relationship Compliance（一对多关系的合规性）
&emsp;对于是一对多关系的每个属性（例如 NSArray 属性）：

&emsp;☑️ 实现一个名为 `<key>` 的方法，该方法返回一个数组，或者具有一个名为 `<key>` 或 `\_<key>` 的数组实例变量。编译器通常会在自动合成属性时为你执行此操作。

&emsp;☑️ 或者，实现方法 `countOf<Key>` 以及 `objectIn<Key>AtIndex:` 和 `<key>AtIndexes:` 中的一个或两个。

&emsp;☑️ （可选）实现 `get<Key>:range:` 以提高性能。

&emsp;此外，如果属性是可变的：

&emsp;☑️ 实现 `insertObject:in<Key>AtIndex:` 和 `insert<Key>:atIndexes:` 中的一个或两个方法。

&emsp;☑️ 实现 `removeObjectFrom<Key>AtIndex:` 和 `remove<Key>AtIndexes:` 中的一个或两个方法。

&emsp;☑️ （可选）实现 `replaceObjectIn<Key>AtIndex:withObject:` 或 `replace<Key>AtIndexes:with<Key>:` 以提高性能。

### Unordered To-Many Relationship Compliance（无序一对多关系的合规性）
&emsp;对于是无序一对多关系的每个属性（例如 NSSet 属性）：

&emsp;☑️ 实现一个名为 `<key>` 的方法，该方法返回一个集合，或者具有一个名为 `<key>` 或 `\_<key>` 的 NSSet 类型的实例变量。编译器通常会在自动合成属性时为你执行此操作。

&emsp;☑️ 或者，实现方法 `countOf<Key>`、`enumeratorOf<Key>` 和 `memberOf<Key>:`。

&emsp;此外，如果属性是可变的：

&emsp;☑️ 实现方法 `add<Key>Object:` 和 `add<Key>:` 中的一个或两个。

&emsp;☑️ 实现方法 `remove<Key>Object:` 和 `remove<Key>:` 中的一个或两个。

&emsp;☑️（可选）实现 `intersect<Key>:` 以提高性能。

### Validation
&emsp;选择验证是否需要它：

&emsp;实现 `validate<Key>:error:` 方法，返回指示 value 的有效性的布尔值，并在适当时返回一个描述错误原因的 NSError 对象的引用。

## 参考链接
**参考链接:🔗**
+ [Key-Value Coding Programming Guide](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/KeyValueCoding/index.html#//apple_ref/doc/uid/10000107i)
