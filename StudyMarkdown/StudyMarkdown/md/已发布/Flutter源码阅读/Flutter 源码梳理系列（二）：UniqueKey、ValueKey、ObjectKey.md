# Flutter 源码梳理系列（二）：UniqueKey、ValueKey、ObjectKey

&emsp;来学习每个 Widget 中超级重要的字段：`final Key? key;`，看一下 Key 的类型都有哪些，以及怎么使用的，以及有哪些注意事项。OK，冲!

&emsp;Key 是 Widget、Element 和 SemanticsNode 的标识符（identifier）。

&emsp;只有当新 widget 的 key 与 element 关联的当前 widget 的 key 相同时，才会使用新 widget 来更新现有 element。（即 Widget 类的静态 canUpdate 函数，当新旧 widget 的 runtimeType 和 key 相等时，可以用新 widget 更新旧的 element，免去再新建 element 的浪费。）

&emsp;必看的官方视频：[When to Use Keys - Flutter Widgets 101 Ep. 4](https://www.youtube.com/watch?v=kn0EOS-ZiIc)，在这个视频中将学习：何时需要使用 key，将 key 放在 widget 树中的位置，以及哪种 key 更适合。更具体地，将学习到关于 key 如何工作的细微差别，如何辨别何时在 widget 层次结构中使用 key，并确定适当类型的 key 来优化基于 widget 的结构。

&emsp;在同一个父级 element 中，key 必须在各个 element 中保持唯一。Key 的子类应该要么是 LocalKey 的子类，要么是 GlobalKey 的子类。(根据使用范围划分的话，key 可以分为：Local 和 Global)

&emsp;下面我们直接去看 Key 的源码。

# Key

&emsp;Key 是一个继承自 Object 的抽象类，有两个注解 @immutable、@pragma('flutter:keep-to-string-in-subtypes') 预示了：key 以及子类都是不可变的，它们所有的字段必须是 final 修饰，以及在 key 的所有子类中保持它们重写的 toString 函数，key 的子类分别都重写了 toString 函数。(关于 @pragma('flutter:keep-to-string-in-subtypes') 的介绍，可以看下一小节的知识点扩展。)

&emsp;Key 虽然是一个抽象类，但是它提供一个工厂函数可以供我们直接使用，它的内部是由 ValueKey 来实现的。（例如：const Text(key: Key('234'), '123');，Key('234') 其实就是：ValueKey('234')。）

```dart
@immutable
@pragma('flutter:keep-to-string-in-subtypes')
abstract class Key {
  // 工厂构造函数，使用给定的 String value 构建一个 ValueKey<String>
  const factory Key(String value) = ValueKey<String>;

  // 默认的构造函数，由 key 的子类使用，
  // 之所以这样做是因为 const factory Key 工厂构造函数会遮蔽隐式构造函数，从而使子类难以调用创建 Key 对象的构造函数。
  
  // LocalKey 和 GlobalKey 构造函数中都会是调用：super.empty()，而不是上面的 const factory Key，
  // 毕竟 LocalKey 和 GlobalKey 不是 ValueKey<String> 的子类。
  @protected
  const Key.empty();
}
```

## @pragma('flutter:keep-to-string-in-subtypes')

&emsp;为了在优化代码大小的过程中保留 Object.toString 的重写而不是将其移除，可以使用此注解。对于某些 URI（当前为 dart:ui 和 package:flutter），Dart 编译器在 profile/release 模式下会删除类中的 Object.toString 重写以减少代码大小。

&emsp;单个类可以通过以下注释选择不执行此操作：

+ @pragma('flutter:keep-to-string')
+ @pragma('flutter:keep-to-string-in-subtypes')

&emsp;例如，在以下类中，即使编译器的 `--delete-tostring-package-uri` 选项会将其它重写的 toString 替换为 `return super.toString()`，但是这里：toString 方法仍将保持为 `return _buffer.toString()`。（按照惯例，dart:ui 通常作为 ui 导入，因此带有前缀。）

```dart
class MyStringBuffer {
  final StringBuffer _buffer = StringBuffer();

  // ...

  @ui.keepToString
  @override
  String toString() {
    return _buffer.toString();
  }
}
```

&emsp;所以综上所述，@pragma('flutter:keep-to-string') 和 @pragma('flutter:keep-to-string-in-subtypes') 就是告诉编译器保留我重写的 toString 函数，不要为了优化程序而把 toString 恢复为默认实现了。

# LocalKey

&emsp;LocalKey 类有点特殊，此抽象类定义出来，感觉仅仅是为了把 LocalKey 的子类和 GlobalKey 作出区分，LocalKey 抽象类内部基本是空的。

&emsp;在同一个父级元素中，key 必须是唯一的。相比之下，GlobalKey 必须在整个应用程序中是唯一的。

&emsp;LocalKey 是直接继承自 Key 的抽象类，内部仅有一个 const 抽象构造函数。 

```dart
abstract class LocalKey extends Key {
  // 抽象的 const 构造函数。这个构造函数使得子类能够提供 const 构造函数，以便它们能在 const 表达式中被使用。
  const LocalKey() : super.empty();
}
```

&emsp;下面开始看 LocalKey 的子类：UniqueKey、ValueKey、ObjectKey。

## UniqueKey

&emsp;UniqueKey 直接继承自 LocalKey。一个我们可以直接使用的 key，一般在一组相同的 Widget 中，而我们却没有常量数据构造其它类型的 key 时使用它（由系统为我们提供构建 key 所需的常量数据，其实是 UniqueKey 没有任何数据依赖，它仅仅依赖自己，依赖 Dart 中一个对象只等于自己的特性）。

&emsp;UniqueKey 是一个只等于自身的 key。因为它不提供 const 构造函数，所以不能用 const 构造函数创建 UniqueKey 对象，如果有 const 构建函数的话意味着所有使用 const 构造函数实例化的 UniqueKey 对象都是同一个，这就不是唯一的了。

```dart
class UniqueKey extends LocalKey {
  // 创建一个仅等于其本身的 key，这个 Key 不能使用 const 构造函数创建，因为这意味着所有实例化的键将是相同的实例，因此将不是唯一的。
  // ignore: prefer_const_constructors_in_immutables , never use const for this class
  // 忽略：prefer_const_constructors_in_immutables，从不对该类使用 const。
  UniqueKey();

  @override
  String toString() => '[#${shortHash(this)}]';
}
```

&emsp;为了保证 UniqueKey 的唯一性，它不提供 Const 构造函数。开始没有看 UniqueKey 的内容之前，只看名字，还以为它是由系统为每个 UniqueKey 变量提供一个 Unique 值来保证它的唯一性的，而实际它只是靠不提供 const 构造函数和以 Object 只等于自身来保证唯一性的。

## ValueKey

&emsp;ValueKey 是一个直接继承自 LocalKey 的泛型类，它是一个使用特定类型 T 的值来标识自身的 Key。

&emsp;只有当两个 `ValueKey<T>` 的 value 是 operator== 相等时，它们的 `ValueKey<T>` 才相等。即和上面的 UniqueKey 比，两个 ValueKey 对象是否相等时，并不是比较多两个 ValueKey 对象，而是它们的 value 值。 

&emsp;Valuekey 可以被子类化用来创建 value key，这样可以让碰巧使用相同 value 的其他 value key 之间不再相等。如果子类是私有的，这将导致一个 value key 类型，其 value key 不会与其他来源的 key 发生冲突，这可能会很有用，例如，如果这些 key 正在作为相同作用域中从另一个 widget 提供的 key 的后备使用。（这里等下看一下下面的 Valuekey 的 operator == 就明白了，因为它除了用两个 Valuekey 对象的 value 相等之外，还需要它们的 runtimeType 一样，所以，如果是使用相同 value 的不同的 Valuekey 类型对象，它们显然也是不相等的。）

&emsp;下面看 ValueKey 源码，首先是它的 const 构造函数，需要传递一个 value，并且两个 value 相等的 ValueKey 也是相等的。

```dart
class ValueKey<T> extends LocalKey {
  /// Creates a key that delegates its [operator==] to the given value.
  // 创建一个 key，并将其 operator == 委托给这个给定的 value。
  const ValueKey(this.value);
  
  // ...
}
```

### value

&emsp;ValueKey 有一个泛型字段：value，ValueKey 对象把自己的 operater == 委托给了它。

```dart
  final T value;
```

### operator ==

&emsp;ValueKey 重写了自己的 operator ==，可看到只要两个变量都是 ValueKey 类型，并且双方的 value 也相等的话，那么两个 ValueKey 就是相等的。

&emsp;即两个 ValueKey 的相等性主要是靠它的 value 字段决定的！

```dart
  @override
  bool operator ==(Object other) {
    if (other.runtimeType != runtimeType) {
      return false;
    }
    
    return other is ValueKey<T>
        && other.value == value;
  }
```

### hashCode

&emsp;ValueKey 重写了自己的 hashCode，调用 Object 类的静态 hash 函数，以 runtimeType 和 value 字段一起计算一个 ValueKey 实例的哈希值。

&emsp;即一个 ValueKey 对象的 hashCode 是由它的 runtimeType 和 value 字段决定的！（而不是 ValueKey 对象本身。）

```dart
  @override
  int get hashCode => Object.hash(runtimeType, value);
```

### toString

&emsp;因为 ValueKey 的 value 是一个泛型 T，所以这里重写了它的 toString 函数，可以通过 toString 更全面的看出 value 的值。这样在检查 ValueKey 对象时能提供更有用的信息，更利于调试或记录。

&emsp;例如：`ValueKey('123')` 打印它的 toString 是：`[<'123'>]`。`ValueKey([1, 2, 3])` 打印它的 toString 是：`[<[1, 2, 3]>]`。从而我们能直观的看到 ValueKey 对象的 value 值是什么类型。

```dart
  @override
  String toString() {
    final String valueString = T == String ? "<'$value'>" : '<$value>';
    
    // The crazy on the next line is a workaround for https://github.com/dart-lang/sdk/issues/33297
    // _TypeLiteral 返回 T 的类型
    
    if (runtimeType == _TypeLiteral<ValueKey<T>>().type) {
      return '[$valueString]';
    }
    
    return '[$T $valueString]';
  }
```

```dart
class _TypeLiteral<T> {
  Type get type => T;
}
```

### ValueKey 总结

&emsp;由于 ValueKey 使用更广泛，所以这里我们总结一下。

&emsp;ValueKey 直接继承自 LocalKey 抽象类，新增了一个泛型 T value 字段，我们可以随便使用任何类型的 value 值。ValueKey 重写了 hashCode 和 operator ==，主要就是靠这个 value 值来保证唯一性的。

&emsp;下面看一个与 ValueKey 极其相似的 key：ObjectKey。ObjectKey 也有一个 value 字段，但是它的类型是：`Object?`。在 ValueKey 中它的 value 字段被定义为了泛型 T，但是在 Dart 的语言环境之下，所有类型（非 null 外）的基类都是 Object，所以这里和 ValueKey 比的话，那么 ObjectKey 的 value 字段也是可以使用各种类型的。

&emsp;那么 ValueKey 和 ObjectKey 的最主要区别在哪呢？虽然它们都是依赖自己的 value 字段来进行判等，但是 ValueKey 的 == 中，使用的还是 value 的 == 来判等，而到了 ObjectKey 中，它的 == 中 value 的判等方式使用的是：identical。

&emsp;下面我们会详细看：identical 这个全局函数：`external bool identical(Object? a, Object? b);`。

## ObjectKey

&emsp;ObjectKey 是根据其 value 字段所代表的 Object 来确定自己的身份的，用来将一个 widget 的身份与生成该 widget 时使用的 Object（key）的身份联系在一起。

&emsp;ObjectKey 和 ValueKey 一样，都是直接继承自：LocalKey。

```dart
class ObjectKey extends LocalKey {
  /// Creates a key that uses [identical] on [value] for its [operator==].
  // 创建 ObjectKey，这个 ObjectKey 的比较操作符 operator == 会在 value 字段上使用 identical。
  const ObjectKey(this.value);
  
  // ...
  
}
```

### value

&emsp;ObjectKey 也有一个 final 修饰的名为 value 的字段，类型是 Object?。

```dart
  final Object? value;
```

### operator ==

&emsp;ObjectKey 也重写了 operator ==，但是这里和 ValueKey 明显不同的是，针对 value 字段的比较方式，ValueKey 比较时直接使用 value 的 operator == 操作符，而 ObjectKey 比较时使用的 identical(other.value, value)。关于 identical 的细节，下面我们会展开看一下。

&emsp;看到目前，我们思考一下：感觉如果是 operator == 的方式比较的话，如果比较的是 int、String 等这种较简单类型（不过在 Dart 中它们也是 Object 类型）的话会比较好，毕竟它们只要值一一对应的话就是相等的了，dart 已经为它们重写了 operator ==（不仅仅重写 == 那么简单，如果相同的数字和字符串就不再创建 int 对象 和 String 对象了），如果是值相等的话它们就是相等的。而针对 Object 基类型，目前它们的 Operator == 的默认行为是只有两个对象是同一个对象是才能相等。

&emsp;在我们的日常开发场景中，如果 id（一般是 String 类型）能保证一组对象的唯一性的话，用 ValueKey 就可以了，感觉这样绝大部分场景都是可以满足的。如果不行的话就用一组不同的对象，来区分唯一性。

&emsp;那么 ValueKey 和 ObjectKey 之间的区别就在这：identical 之间了，下面我们看一下：`external bool identical(Object? a, Object? b);`。

```dart
  @override
  bool operator ==(Object other) {
    if (other.runtimeType != runtimeType) {
      return false;
    }
    
    return other is ObjectKey
        && identical(other.value, value);
  }
```

### identical

&emsp;identical 是 dart:core 中的函数：

&emsp;identical 用于检查两个对象引用是否指向同一个对象。Dart 的值，也就是存储在变量中的内容，都是对象引用。可能有多个引用指向同一个对象。Dart 对象有一个 identity，将其与其他对象区分开来，即使其他对象的状态(对象的状态：可理解为对象的字段)相同也是如此。identical 函数可确定两个对象引用是否指向同一个对象。

&emsp;如果 identical 调用返回 true，则可以保证无法区分这两个参数。如果返回 false，则只能确定这两个参数不是同一个对象。在非常量上下文中调用生成（non-factory）构造函数，或者 non-constant list、set、map 字面量，总是会创建一个新对象，该对象与任何现有对象都不相同。

&emsp;常量规范化（Constant canonicalization）确保两个在编译时常量表达式中创建状态相同的对象的结果最终会被评估为指向同一个规范化实例的引用。示例：

```dart
print(identical(const <int>[1], const <int>[1])); // true
```

&emsp;整数和双精度浮点数是特殊情况，它们根本不允许创建新实例。如果两个整数相等，它们也总是相同的。如果两个双精度浮点数具有相同的二进制表示，它们是相同的（在 Web 平台上围绕 double.nan 和 -0.0 存在一些注意事项）。

&emsp;Record 对象没有持久的 identity。这使得编译器能够将 Record 对象分解为其部分，并稍后重建它，而无需担心创建具有相同 identity 的对象。如果所有相应字段完全相同，则 Record 对象可能与具有相同结构的另一个 Record 对象 相同，或者可能不同，但永远不会与其他任何东西相同。示例：

```dart
var o = new Object();

var isIdentical = identical(o, new Object()); // false, different objects.
isIdentical = identical(o, o); // true, same object.

isIdentical = identical(const Object(), const Object()); // true, const canonicalizes.

isIdentical = identical([1], [1]); // false, different new objects.

isIdentical = identical(const [1], const [1]); // true.
isIdentical = identical(const [1], const [2]); // false.

isIdentical = identical(2, 1 + 1); // true, integers canonicalize.

var pair = (1, "a"); // Create a record.
isIdentical = identical(pair, pair); // true or false, can be either.

var pair2 = (1, "a"); // Create another(?) record.
isIdentical = identical(pair, pair2); // true or false, can be either.

isIdentical = identical(pair, (2, "a")); // false, not identical values.
isIdentical = identical(pair, (1, "a", more: true)); // false, wrong shape.
```

&emsp;看到这里 identical 和默认 operator == 都是比较只有同一个对象的情况下才会返回 true。那么：ValueKey 和 ObjectKey 还有什么区别呢？好像它们可以直接相互替代的吗？看到下面我们会给出总结！

```dart
external bool identical(Object? a, Object? b);
```

### hashCode

&emsp;ObjectKey 的 hashCode 不同与 ValueKey，这里对 value 使用了：identityHashCode(value)，保证不管 value 是否重写了自己的 hashCode getter，依然对 value 使用 Object.hashCode 取哈希码。

&emsp;然后同 ValueKey 一样，以 runtimeType 和 value 字段一起计算一个 ObjectKey 对象的哈希值。

```dart
  @override
  int get hashCode => Object.hash(runtimeType, identityHashCode(value));
```

### toString

&emsp;返回运行时类型和 hashCoed 摘要。（这里提到了 objectRuntimeType 优化 toString 性能，后续有时间再研究）

```dart
  @override
  String toString() {
    if (runtimeType == ObjectKey) {
      return '[${describeIdentity(value)}]';
    }
    
    return '[${objectRuntimeType(this, 'ObjectKey')} ${describeIdentity(value)}]';
  }
```

## ValueKey 和 ObjectKey 总结

&emsp;ObjectKey 的内容看完了，感觉上它和 ValueKey 比区别属实很小，都有一个 Object 类型的 value 字段。但是当看它们的 operator == 判等时，就明白它们的区别在哪了！ObjectKey 更精密（缜密）一些，它使用 identical 判断 value 的一致性，意思就是必须要保证 value 字段值完全是同一个对象，才能保证两个 ObjecKey 对象是相等的。

&emsp;反观 ValueKey 的话，只要 value 字段使用 value 的 operator == 判等即可，比如 value 的 operator == 重写的话，就使用 operator == 的重写进行判等，此时两个 ValueKey 就相等了。

&emsp;下面看个例子你就明白了，我们自己定义一个 Person 类来作为 ValueKey/ObjectKey 对象的 value 使用，最主要的是重写了它的 operator == 和 hashCode getter，现在所有的 Person 对象都是相等的了：（但是当 Person 对象使用在 ObjectKey 对象的 value 值时要除外！）

```dart
class Person {
  final String name;

  const Person({required this.name});

  @override
  bool operator ==(Object other) => true;

  @override
  int get hashCode => 123;
}
```

&emsp;把 Person 对象使用在 Key 中：

```dart
    Person p1 = Person(name: '123');
    Person p2 = Person(name: '456');

    print('🐯 Person == 比较：${p1 == p2}'); // true
    print('🐯 Person identical 比较：${identical(p1, p2)}'); // false

    print('🐯 Person ValueKey: ${ValueKey(p1) == ValueKey(p2)}'); // true
    print('🐯 Person ObjectKey: ${ObjectKey(p1) == ObjectKey(p2)}'); // false
```

&emsp;即使使用 const，`${identical(p1, p2)}` 和 `${ObjectKey(p1) == ObjectKey(p2)}` 依然打印 false，需要 name 值一样，它们才能打印 true。

```dart
    Person p1 = const Person(name: '123');
    Person p2 = const Person(name: '456');
```

&emsp;当我们把重写的 Person 的 operator == 和 hashCode getter 都注释掉的话：

```dart
    Person p1 = Person(name: '123');
    Person p2 = Person(name: '123');

    print('🐯 Person == 比较：${p1 == p2}'); // false
    print('🐯 Person identical 比较：${identical(p1, p2)}'); // false

    print('🐯 Person ValueKey: ${ValueKey(p1) == ValueKey(p2)}'); // false
    print('🐯 Person ObjectKey: ${ObjectKey(p1) == ObjectKey(p2)}'); // false
```

&emsp;然后 ValueKey/ObjectKey 的 value 都用 String 的话：

```dart
    String str1 = "234";
    String str2 = "234";

    print('🐯 String == 比较：${str1 == str2}'); // true
    print('🐯 String identical 比较：${identical(str1, str2)}'); // true

    print('🐯 String ValueKey: ${ValueKey(str1) == ValueKey(str2)}'); // true
    print('🐯 String ObjectKey: ${ObjectKey(str1) == ObjectKey(str2)}'); // true
```

&emsp;通过 ObjectKey 可以看出当 String 类对象用的 **字符串值** 一样的话，两个 String 对象就是同一个对象了。 

&emsp;鉴于我们极少重写自定义类的 operator == 和 hashCode getter 的情况下，感觉使用 ValueKey 和 ObjectKey 是完全一样的，但是看名字的话总感觉 ValueKey 更轻量一些！

&emsp;如果谁都可以用的时候，至于它俩之间性能差别的话：ValueKey 呢：value 是泛型的需要做类型推断，去找对应的 operator == 实现，ObjectKey 的话直接去使用全局的 identical 去判断 value 是否是同一个！但是不知道 identical 的性能如何，大家可以讨论一下。

&emsp;看到现在，我们看了：Key、LocalKey、UniqueKey、`ValueKey<T>`、ObjectKey。它们的继承关系如下：

+ Object => Key => LocalKey
+ Object => Key => LocalKey => UniqueKey
+ Object => Key => LocalKey => ValueKey
+ Object => Key => LocalKey => ObjectKey

&emsp;下面我们继续向前还有：PageStorageKey、GlobalKey、GlobalObjectKey、LabeledGlobalKey 没看呢！

&emsp;PageStorageKey 是一个特殊的 ValueKey。

+ Object => Key => LocalKey => `ValueKey<T>` => PageStorageKey

&emsp;下面是一个附录：Record 类的介绍，有兴趣可以看一下。

# Record

&emsp;前面多次提到了 Record 类，下面我看一下它的内容。它其实类似 Swift 中的元组。

&emsp;Record 是一个抽象的不可继承的类。

```dart
abstract final class Record { // ... }
```

&emsp;Record 类是所有记录类型的超类型，但它本身不是任何对象实例的运行时类型（它是一个抽象类）。所有实现 Record 的对象都具有 Record 类型作为它们的运行时类型。一个由 Record 类型描述的记录值，由若干个字段组成，其中每个字段都可以是位置字段或命名字段。

```dart
    (int, String, {bool isValid}) triple = (1, "one", isValid: true);
    
    // 打印：triple runtimeType: (int, String, {bool isValid})
    print('triple runtimeType: ${triple.runtimeType}'); 
```
&emsp;记录值和记录类型的书写方式类似于参数列表和简化函数类型参数列表（不允许也不需要使用 required 修饰符，因为记录字段永远不会是可选的）。示例：

```dart
(int, String, {bool isValid}) triple = (1, "one", isValid: true);
```

&emsp;语法上类似：

```dart
typedef F = void Function(int, String, {bool isValid});
void callIt(F f) => f(1, "one", isValid: true);
```

&emsp;每个记录和记录类型都有一个形状，由位置字段的数量和命名字段的名称确定。例如：

```dart
(double value, String name, {String isValid}) another = (3.14, "Pi", isValid: "real");
```

&emsp;这是另一个具有相同结构的记录声明（包含两个位置字段和一个命名字段 isValid），但类型不同。在位置字段上写的名称完全是为了文档目的，它们对程序没有影响（与函数类型中位置参数上的名称相同，例如 typedef F = int Function(int value);，其中标识符 value 没有影响）。

&emsp;记录值(Record values)主要使用模式来进行解构，比如：

```dart
switch (triple) {
  case (int value, String name, isValid: bool ok): // ....
}
```

&emsp;可以使用命名的 getter 来访问各个字段，对于位置字段可使用 $1、$2 等，对于命名字段可以直接使用字段名称本身。如下：

```dart
int value = triple.$1;
String name = triple.$2;
bool ok = triple.isValid;
```

&emsp;正因为如此，一些标识符不能用作命名字段的名称:

+ Object 成员的名称：hashCode、runtimeType、toString 和 noSuchMethod。
+ 在同一记录中，使用位置 getter 的名称，如 (0，$1: 0) 是无效的，但 (0，$2: 0) 是有效的，因为该记录形状中没有使用 getter $2 的位置字段。 （这仍然会令人困惑，因此实践中应避免使用。）
+ 另外，不允许使用以下划线: `_` 开头的名称。字段名称不能是库私有的。

&emsp;记录对象的运行时类型是一个记录类型，因此是 Record 的一个子类型，并且是 Object 及其超类的传递类型。

&emsp;记录值没有持久的 identical 行为。对记录对象的引用可能随时会变成指向另一个具有相同结构和字段值的记录对象的引用。

&emsp;除此之外，记录类型只能是另一个具有相同结构的记录类型的子类型，仅当前者记录类型的字段类型是另一个记录类型相应字段类型的子类型时。也就是说，（int，String，{bool isValid}）是（num，String，{Object isValid}）的子类型，因为它们具有相同的结构，并且字段类型是逐点子类型。具有不同结构的记录类型彼此之间无关。

### runtimeType

&emsp;runtimeType 是一个代表 Record 的运行时类型的 Type 对象。（Type 是一个抽象接口类，需要提供：hashCode getter、operator ==、toString。）

&emsp;Record 的运行时类型是由记录的形状、位置字段数量和命名字段的名称，以及每个字段的运行时类型来定义的。（记录的运行时类型不依赖于字段值的 runtimeType getter，这可能被重写为 Object.runtimeType。）

&emsp;Record 类型的 Type 对象仅在与另一个 Record 类型相等时才相等，只有当另一个记录类型具有相同的形状，并且相应字段具有相同的类型时才会相等。

```dart
Type get runtimeType;
```

&emsp;后面还有 hashCode、operator ==、toString 感兴趣的可以读一下。

## 参考链接
**参考链接:🔗**
+ [Key class](https://api.flutter.dev/flutter/dart-core/Object-class.html)
+ [Record class](https://api.flutter.dev/flutter/dart-core/Record-class.html)
+ [When to Use Keys - Flutter Widgets 101 Ep. 4](https://www.youtube.com/watch?v=kn0EOS-ZiIc)
+ [VM-Specific Pragma Annotations](https://github.com/dart-lang/sdk/blob/main/runtime/docs/pragmas.md)
+ [identical function](https://api.flutter.dev/flutter/dart-core/identical.html)
+ [LocalKey class](https://api.flutter.dev/flutter/foundation/LocalKey-class.html)
