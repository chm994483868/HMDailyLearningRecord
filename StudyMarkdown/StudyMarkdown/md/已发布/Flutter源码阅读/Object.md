# Object

&emsp;Object class 是除了 null 之外，所有 Dart 对象的基类。由于 Object 是 non-nullable Dart 类层次结构的根，因此每个 non-Null Dart 类都是 Object 的子类。

&emsp;当你定义一个类时，你应该考虑重写 toString 方法，以返回描述该类实例的字符串。你可能还需要定义 hashCode 和 operator ==，就像 [Dart's core libraries](https://dart.dev/libraries) 中的 [Implementing map keys](https://dart.dev/libraries/dart-core#implementing-map-keys) 部分所描述的那样(围绕 hashCode 和 operator == 展开的内容)。

## Implementing map keys

&emsp;Dart 中的每个对象都会自动提供一个整数哈希码，因此可以将其用作 map 中的 key。但是，你可以重写 hashCode getter 来生成自定义哈希码。如果这样做了，你可能还想要重写 == 运算符。通过 == 相等的对象必须具有相同的哈希码。哈希码不必是唯一的，但应当被很好地分布，减少哈希碰撞。

&emsp;为了一致且简便地实现 hashCode getter，考虑直接使用 Object 类提供的静态哈希方法即可。

&emsp;其它一些情况：

+ 如果我们定义的类中有多个字段，而我们想要全部字段都参与生成单个哈希码，可以使用 Object.hash（它最多支持 20 个参数）。
+ 如果我们定义的类中有集合类型的字段（例如：`final List<String> path;`），而我们想要集合中所有对象生成哈希码，可以使用 Object.hashAll（如果元素顺序很重要）或 Object.hashAllUnordered。

&emsp;如下示例代码，重写 Person 类的 hashCode getter：

```dart
class Person {
  final String firstName, lastName;

  Person(this.firstName, this.lastName);

  // 使用 Object 类提供的静态哈希方法重写 hashCode，针对 firstName 和 lastName 两个字段
  @override
  int get hashCode => Object.hash(firstName, lastName);

  // 如果重写 hashCode，通常也应该重写 operator ==。（如果不写的话会收到警告或者错误）
  // 如果两个对象实例都是 person 类，并且 firstName 和 lastName 都相等的话就认为两个对象实例相等，且这两个对象会有相同的哈希码。
  @override
  bool operator ==(Object other) {
    return other is Person &&
        other.firstName == firstName &&
        other.lastName == lastName;
  }
}

void main() {
  var p1 = Person('Bob', 'Smith');
  var p2 = Person('Bob', 'Smith');
  var p3 = 'not a person';
  
  assert(p1.hashCode == p2.hashCode);
  assert(p1 == p2);
  assert(p1 != p3);
}
```

&emsp;看到这呢，其实不免想起了 const，特别是 flutter 针对 const widget 的优化。下面我们看一些示例代码，我们自己定义了一个 Person 类，来看一下 Person 的 const 实例是否相等:

```dart
// 1️⃣：无字段
class Person {
  const Person();
}

void testCost() {
  Person per1 = const Person();
  dynamic per2 = const Person();
    
  if (per == per2) {
    debugPrint('=='); // const person 都是相等的
  } else {
    debugPrint('!=');
  }
}

// 2️⃣：有字段
class Person {
  final String name;
  final String address;
  
  const Person({required this.name, required this.address});
}

void testCost() {
  Person per1 = const Person(name: "123", address: "789");
  dynamic per2 = const Person(name: "123", address: "789");
    
  if (per == per2) {
    debugPrint('=='); // 只要 Person 对象 name 和 address 的值一样，const person 就是相等的
  } else {
    debugPrint('!=');
  }
}

```

&emsp;首先需要给 Person 添加一个 const 构造函数，然后才能在 Person() 前加 const 修饰。然后 per1 和 per2 就是相等的了，不管 per1 和 per2 前面用何种类型修饰。只要是 const Person() 就都是相等的。当然如果 Person() 不加 const 的话就不是相等的。这里把 const 变量理解为统一共用了常量区的一个值也是可以的。

&emsp;这里的 Person 类是继承自 Object 的，所以它的 operator == 是直接使用的 Object 的实现。

&emsp;下面我们看一下 Object Class，探索一下它是如何默认实现 hashCode 和 operator == 的。 

## Object

&emsp;下面开始看 Object class 的源码。首先是它的 const 构造函数，const Object() 用于创建一个新的 Object 实例，Object 实例没有有意义的状态，只能通过其标识符使用。对象实例只等于自身。

```dart
// 通过使用 @pragma('vm:entry-point') ，我们指示编译器在编译期间保留此代码，以防止意外排除。
@pragma("vm:entry-point")
class Object {
  @pragma("vm:recognized", "other")
  const Object();
  
  // ...
}
```

## @pragma("vm:entry-point")

&emsp;Object 类顶部是一个注解：@pragma("vm:entry-point")。@pragma("vm:entry-point") 标记函数（或其他实体，如类）以向编译器指示它将从 native 代码中使用。如果没有这个注释，dart 编译器可能会删除 unused functions、inline them、shrink names 等，并且 native 代码将无法调用它。

&emsp;注解 @pragma(“vm:entry-point”，…) 必须放在类或成员上，以指示它可能在 AOT 模式下直接从 native 或 VM 代码中解析、分配或调用。

&emsp;Dart VM precompiler（AOT 编译器）执行整个程序的优化，例如 tree shaking 和 type flow analysis（TFA），以减小生成的编译应用的大小并提高其性能。这些优化假定编译器可以看到整个 Dart 程序，并能够发现和分析所有在运行时可能被执行的 Dart 函数和成员。虽然 Dart 代码对于 precompiler 是完全可用的(虽然 Dart 代码是完全可以用于预编译的)，但嵌入程序的 native 代码和 native 方法则无法被编译器访问。这样的 native 代码可以通过 native Dart API 回调到 Dart。

&emsp;为了指导 precompiler，程序员必须显式列出 entry points（roots） - 从 native 代码访问的 Dart 类和成员。注意，列出 entry points 是必需的：只要程序定义了调用 Dart 的 native 方法，就需要 entry points 来确保编译的正确性。

&emsp;此外，在启用混淆时，precompiler 需要知道哪些符号需要保留，以确保它们可以从 native 代码中解析出来。

&emsp;如下使用，在 Class 中，以下任何一种形式都可以附加到一个类中：

```dart
@pragma("vm:entry-point")
@pragma("vm:entry-point", true/false)
@pragma("vm:entry-point", !const bool.fromEnvironment("dart.vm.product"))
class C { ... }
```

&emsp;如果第二个参数缺失、为 null 或 true，则该类将能够直接从 native 或 VM 代码中分配。

&emsp;请注意，@pragma("vm:entry-point") 可以添加到抽象类中 -- 在这种情况下，它们的名称将在混淆中保留，但是它们不会有任何 allocation stubs。

&emsp;以下任何一种形式都可以附加到一个 procedure（包括 getter、setter 和构造函数）中：

```dart
@pragma("vm:entry-point")
@pragma("vm:entry-point", true/false)
@pragma("vm:entry-point", !const bool.fromEnvironment("dart.vm.product"))
@pragma("vm:entry-point", "get")
@pragma("vm:entry-point", "call")
void foo() { ... }
```

&emsp;如果第二个参数缺失、为 null 或为 true，则该过程（以及其闭包形式，不包括构造函数和 setter）将可以直接从 native 或 VM 代码中查找和调用。

&emsp;如果该 procedure 是一个生成构造函数，则封闭类也必须被标注为可以从 native 或 VM 代码中进行分配。

&emsp;如果注释为 "get" 或 "call"，则该过程只能供闭包使用（通过 Dart_GetField 访问）或调用（通过 Dart_Invoke 访问）。

&emsp;"@pragma("vm:entry-point", "get")" 针对构造函数或设置器是不允许的，因为它们无法被闭包化。

&emsp;以下任何一种形式都可以附加到非静态字段上。前三种形式可以附加到静态字段上。

```dart
@pragma("vm:entry-point")
@pragma("vm:entry-point", null)
@pragma("vm:entry-point", true/false)
@pragma("vm:entry-point", !const bool.fromEnvironment("dart.vm.product"))
@pragma("vm:entry-point", "get"/"set")
int foo;
```

&emsp;如果第二个参数缺失、为 null 或 true，则该字段会被标记为 native 访问，并且对于非静态字段，封闭类的接口中相应的 getter 和 setter 会被标记为 native 调用。如果使用 'get'/'set' 参数，则只标记 getter/setter。对于静态字段，隐式 getter 总是会被标记。第三种形式对于静态字段没有意义，因为它们不属于接口。

## operator ==

&emsp;等式操作符：所有 Object 的默认行为是当且仅当此 object 和另一个 object 是同一个对象时才返回 true。

&emsp;重写此方法以在类上指定不同的等式关系。重写 operator == 后仍然必须是等价关系。也就是说，它必须是符合以下要求：

+ Total：对于所有参数，它必须返回一个布尔值。不应抛出异常。
+ Reflexive：对于所有对象 o，o == o 必须为 true。
+ Symmetric：对于所有对象 o1 和 o2，o1 == o2 和 o2 == o1 要么都为 true，要么都为 false。
+ Transitive：对于所有对象 o1、o2 和 o3，如果 o1 == o2 和 o2 == o3 都为 true，则 o1 == o3 必须为 true。

&emsp;该方法还应该随时间保持一致，因此两个对象是否相等只有在至少一个对象被修改时才应更改。如果子类重写了等式操作符，应该同时重写 hashCode 方法以保持一致性。

```dart
  external bool operator ==(Object other);
```

&emsp;看到一个第一次见的关键字：external。参考这个链接：[What does external mean in Dart?](https://stackoverflow.com/questions/24929659/what-does-external-mean-in-dart)

## hashCode

&emsp;hashCode getter 用于获取对象实例的哈希码。哈希码是一个表示影响 operator == 比较的单个整数。所有对象都有哈希码。Object 默认实现的哈希码仅表示对象的身份，就像默认的 operator == 实现只在对象完全相同时才认为它们相等（参见: identityHashCode）。

&emsp;如果 operator == 被重写为使用对象状态（就是仅比较对象的某些字段值）而非身份，则哈希码也必须更改以表示该状态，否则该对象不能用于基于哈希的数据结构，例如默认的 Set 和 Map 实现。

&emsp;对于根据 operator == 相等的对象，它们的哈希码应该是相同的。对象的哈希码只有在影响相等性的方式发生改变时才应更改。对于哈希码，没有更多的要求。它们不必在同一程序的不同执行之间保持一致，也没有分布保证。

&emsp;不相等的对象可以具有相同的哈希码。从技术上讲，所有实例具有相同的哈希码也是允许的，但如果发生碰撞太频繁，可能会降低基于哈希的数据结构如 HashSet 或 HashMap 的效率。

&emsp;如果子类重写了 hashCode，那么它应该同时重写 operator == 运算符以保持一致性。

&emsp;看到这里，hashCode 和 == 在 Dart 语言中和其它语言比，并没有什么区别。然后试图去理解 hashCode getter 和 operator == 之间的关系：

1. 如果类定义中重写了其中一方，那么也必须重写另一方。（hashCode 和 == 必须同时重写。）
2. == 的对象，它们的 hashCode 一定是相等的。
3. hashCode 相同的对象，== 并不一定返回 true。
4. hashCode 都是一个纯数字。 

```dart
  external int get hashCode;
```

### identityHashCode

&emsp;identityHashCode 用于返回一个对象的标识哈希码。

&emsp;返回原始 Object.hashCode 在此对象上返回的值，即使 hashCode 已被重写。这个哈希码与相等性兼容，这意味着无论对于任何非 Record 对象，在单个程序执行期间，每次传递相同的参数时它都保证给出相同的结果。

&emsp;Record class 的标识哈希码是未定义的，因为 Record 没有持久的标识。Record 的标识和标识哈希码可以随时发生变化。

```dart
var identitySet = HashSet(equals: identical, hashCode: identityHashCode);

var dt1 = DateTime.now();
var dt2 = DateTime.fromMicrosecondsSinceEpoch(dt1.microsecondsSinceEpoch);

assert(dt1 == dt2);
identitySet.add(dt1);

print(identitySet.contains(dt1)); // true
print(identitySet.contains(dt2)); // false
```

```dart
@pragma("vm:entry-point")
external int identityHashCode(Object? object);
```

## toString

&emsp;toString 函数返回该 object 的字符串表示。默认返回：Instance of 'xxx'，xxx 是指所属的类名。

&emsp;有些类具有默认的文本表示形式，通常会配有一个静态的解析函数（例如 int.parse）。这些类将会把它们的文本表示形式作为它们的字符串表示形式。换句话说，当你打印这些类的实例时，会直接显示它们的文本形式。

&emsp;在编程中，有些类并没有真正有用的文本形式，但它们通常会重写 toString 函数，以便在检查 object 时提供有用的信息，主要用于调试或记录日志。简单说，就是重写 toString 可以让我们更方便地查看 object 的信息，方便调试代码。

```dart
external String toString();
```

&emsp;既然上面提到了，我们就看一下 int 的静态函数 parse，其实就是尝试把字符串转换为一个数字，而这里的 toString 的主旨其实就是用字符串表示一个对象，方便我们通过字符串就能识别出是哪个对象。

&emsp;下面是 int parse 函数相关的内容，有兴趣的话可以读一下。

```dart
external static int parse(String source, {int? radix});
```

&emsp;parse 函数用于将 String source 解析为一个可能带符号的整数文字值并返回。source 必须是一个非空的基数（base-radix）数字序列，可选地以减号或加号（'-' 或 '+'）为前缀。

&emsp;radix 必须在 2 到 36 的范围内。所使用的数字首先是十进制数字 0 到 9，然后是带有值 10 到 35 的字母 'a' 到 'z'。还接受具有与小写字母相同值的大写字母。

&emsp;如果没有给出 radix，则默认为 10。在这种情况下，source 数字也可以以 0x 开头，此时数字将被解释为十六进制整数字面量。当 int 由 64 位有符号整数实现时，十六进制整数文字可能表示大于 263 的值，在这种情况下，该值被解析为无符号数，并且结果值为相应的有符号整数值。

&emsp;对于任何 int n 和有效基数 r，可以保证 n == int.parse(n.toRadixString(r), radix: r)。

&emsp;如果 source 字符串不包含有效的整数文字，可选地以符号为前缀，将抛出一个 FormatException。

&emsp;而不是抛出并立即捕获 FormatException，相反，应该使用 tryParse 来处理潜在的解析错误。例如：

```dart
var value = int.tryParse(text);
if (value == null) {
  // handle the problem
  // ...
}
```

## noSuchMethod

&emsp;noSuchMethod 方法在调用一个 **类中不存在的方法或属性时** 被调用。一个 dynamic 变量可以尝试调用自己不一定存在的方法。例如下面示例代码，dynamic object 直接调用 add 方法，编译时没有问题，到运行时才会报错。

```dart
dynamic object = 1;
object.add(42); 
```

&emsp;这段无效的代码将调用整数 1 的 noSuchMethod 方法。noSuchMethod 方法中传递来的 invocation 参数表示：.add(42) 调用的相关信息且带有参数，然后抛出异常。

&emsp;类可以重写自己的 noSuchMethod 方法来为这种无效的动态调用提供自定义行为。如果一个类中定义了自己的 noSuchMethod 函数，那么该类可以不必实现它 implements interface 的所有成员方法。

&emsp;如下示例代码，尽管 MockList 类中没有任何 interface class List 方法的具体实现，该代码在编译时没有警告或错误(使用 linter 可帮助报错)。对 List 方法的调用会被转发到 noSuchMethod 中，因此该代码将记录下类似 `Invocation.method(#add, [42])` 的调用，然后如果继续执行 super.noSuchMethod(invocation) 的话，则是抛出一个异常，super 的 noSuchMethod 默认实现是抛出异常，当然我们也可以不调用 super.noSuchMethod(invocation)，那么程序就正常进行。

```dart
class MockList<T> implements List<T> {
  noSuchMethod(Invocation invocation) {
    log(invocation);
    super.noSuchMethod(invocation); // Will throw.
  }
}

void main() {
  MockList().add(42);
}
```

&emsp;如果 noSuchMethod 方法返回一个值，它将成为原始调用的结果。如果该值不是原始调用可以返回的类型，则在调用时会发生类型错误。默认行为是抛出一个 NoSuchMethodError。

```dart
  @pragma("vm:entry-point")
  @pragma("wasm:entry-point")
  external dynamic noSuchMethod(Invocation invocation);
```

## runtimeType

&emsp;runtimeType 是对象的运行时类型表示，默认都是对象所属的类。

```dart
  external Type get runtimeType;
```

## hash

&emsp;hash 是 Object 的一个静态函数。用于创建一组对象的组合哈希码。计算哈希码时，实际提供的所有参数（即使它们为 null）的 Object.hashCode 被数值组合在一起。(可看到它最多把 20 个对象的哈希码组合在一起。)

&emsp;如下面示例代码：

```dart
class SomeObject {
  final Object a, b, c;
  SomeObject(this.a, this.b, this.c);
  
  bool operator ==(Object other) =>
      other is SomeObject && a == other.a && b == other.b && c == other.c;

  // SomeObject 的哈希码是：a、b、c 三个字段的哈希码组合在一起
  int get hashCode => Object.hash(a, b, c);
}
```

&emsp;在单个程序（single program）执行过程中，当使用相同参数多次调用该函数时，计算出的值将保持一致。

&emsp;该函数生成的散列值不能保证在同一程序的不同运行中稳定，也不能保证在同一程序的不同 isolate 之间稳定。所使用的确切算法可能因不同平台或不同平台库版本的不同而有所不同，而且可能依赖于在每次程序执行时更改的值。

&emsp;当以包含实际参数的集合以相同顺序调用该函数时，hashAll 函数将与该函数给出相同的结果。

```dart
  @Since("2.14")
  static int hash(Object? object1, ...
  ...
```

## hashAll

&emsp;hashAll 是 Object 的一个静态函数。hashAll 用于为一系列对象创建一个合并的哈希码。即使对象为 null，哈希码也会为对象中的元素计算出来，通过迭代顺序将每个元素的 Object.hashCode 数值组合起来计算。`hashAll([o])` 的计算结果不会是 `o.hashCode`。

&emsp;如下示例代码，重写了 SomeObject 类的 operator == 和 hashCode getter：

```dart
class SomeObject {
  final List<String> path;
  SomeObject(this.path);
  
  bool operator ==(Object other) {
    if (other is SomeObject) {
      if (path.length != other.path.length) return false;
      
      for (int i = 0; i < path.length; i++) {
        if (path[i] != other.path[i]) return false;
      }
      
      return true;
    }
    
    return false;
  }

  int get hashCode => Object.hashAll(path);
}
```

&emsp;当在单个程序的执行过程中以相同顺序和具有相同哈希码的对象再次调用该函数时，计算出的值将保持一致。

&emsp;该函数生成的哈希值不能保证在同一程序的不同运行之间稳定，也无法保证在同一程序的不同 isolate 中运行的代码之间稳定。所使用的确切算法可能会因不同平台或不同版本的平台库而异，它可能依赖于在每次程序执行时都会更改的值。

```dart
  @Since("2.14")
  static int hashAll(Iterable<Object?> objects) {
    int hash = _hashSeed;
    
    for (var object in objects) {
      hash = SystemHash.combine(hash, object.hashCode);
    }
    
    return SystemHash.finish(hash);
  }
```

## hashAllUnordered

&emsp;hashAllUnordered 是 Object 的一个静态函数。hashAllUnordered 用于为对象集合创建一个合并的哈希码。即使对象中的元素为 null，也会为它们计算哈希码，该哈希码是以独立顺序的方式数值地组合每个元素的 Object.hashCode 计算而来的。`hashAllUnordered({o})` 的结果不是 `o.hashCode`。

&emsp;如下示例代码，使用 hashAllUnordered 可判断两个 Set 是否相等：

```dart
bool setEquals<T>(Set<T> set1, Set<T> set2) {
  var hashCode1 = Object.hashAllUnordered(set1);
  var hashCode2 = Object.hashAllUnordered(set2);
  if (hashCode1 != hashCode2) return false;
  // Compare elements ...
}
```

&emsp;当在单个程序执行过程中再次使用具有相同哈希码的对象调用函数时，计算出的值将是一致的，即使对象不一定是以相同顺序出现。

&emsp;此函数生成的哈希值不能保证在多次运行同一个程序时是稳定的。所使用的确切算法可能在不同平台之间或不同版本的平台库之间有所不同，它可能取决于在每次程序执行时改变的值。

```dart
  @Since("2.14")
  static int hashAllUnordered(Iterable<Object?> objects) {
    int sum = 0;
    int count = 0;
    const int mask = 0x3FFFFFFF;
    
    for (var object in objects) {
      int objectHash = SystemHash.smear(object.hashCode);
      sum = (sum + objectHash) & mask;
      count += 1;
    }
    
    return SystemHash.hash2(sum, count, 0);
  }
```

## Object 总结 

&emsp;Object 类的内容看完了，并没有什么印象深刻的东西：

1. 默认为所有的 Dart 类提供了 hashCode getter。
2. 默认为所有的 Dart 类提供了 operator ==，只有对象是同一个对象时才能判等。
3. 默认为所有的 Dart 类提供了 toString 函数，默认返回：Instance of 'xxx'，表示对象所属的类。
4. 提供了一个统一的机制，当调用一个对象不存在的函数时，会默认回调到：noSuchMethod，这个还是比较有用的。
5. 默认为所有的 Dart 类提供了 runtimeType getter，默认返回在运行时对象所属的类。
6. 然后是三个返回值都是整数的静态函数：Object.hash、Object.hashAll、Object.hashAllUnordered，当我们想要重写我们自定义类的 hashCode getter 时会用到。

## 参考链接
**参考链接:🔗**
+ [Object class](https://api.flutter.dev/flutter/dart-core/Object-class.html)
+ [vm:entry-point pragma](https://github.com/dart-lang/sdk/blob/master/runtime/docs/compiler/aot/entry_point_pragma.md)
+ [What does @pragma("vm:prefer-inline") mean in Flutter?](https://stackoverflow.com/questions/64314719/what-does-pragmavmprefer-inline-mean-in-flutter)
+ [VM-Specific Pragma Annotations](https://mrale.ph/dartvm/pragmas.html)
+ [Creating Objects and Classes in Dart and Flutter](https://dart.academy/creating-objects-and-classes-in-dart-and-flutter/)
+ [Objects Dart: Understanding Classes, Instances, and Methods](https://www.dhiwise.com/post/objects-dart-understanding-classes-instances-and-methods)
+ [Dart 3 in depth: New class modifiers](https://stevenosse.com/dart-3-in-depth-new-class-modifiers)
+ [What does external mean in Dart?](https://stackoverflow.com/questions/24929659/what-does-external-mean-in-dart)
