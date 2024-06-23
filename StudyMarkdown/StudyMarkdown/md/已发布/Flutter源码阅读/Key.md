# Key

&emsp;来学习每个 Widget 中超级重要的字段：`final Key? key;`，看一下 Key 的类型都有哪些，以及怎么使用的，以及有哪些注意事项。OK，冲!

&emsp;Key 是 Widget、Element 和 SemanticsNode 的标识符（identifier）。

&emsp;只有当新 widget 的 key 与 element 关联的当前 widget 的 key 相同时，才会使用新 widget 来更新现有 element。（即 widget 的静态 canUpdate 函数，当新旧 widget 的 runtimeType 和 key 相等时，可以用新 widget 更新旧的 element，免去再新建 element 的浪费。）

&emsp;必看的官方视频：[When to Use Keys - Flutter Widgets 101 Ep. 4](https://www.youtube.com/watch?v=kn0EOS-ZiIc)，在这个视频中将学习：何时需要使用 key，将 key 放在 widget 树中的位置，以及哪种 key 更适合。更具体地，将学习到关于 key 如何工作的细微差别，如何辨别何时在 widget 层次结构中使用 key，并确定适当类型的 key 来优化基于 widget 的结构。

&emsp;在同一个父级 element 中，key 必须在各个 element 中保持唯一。Key 的子类应该要么是 LocalKey 的子类，要么是 GlobalKey 的子类。(根据使用范围划分的话，key 可以分为：Local 和 Global)

&emsp;下面我们直接去看 Key 的源码。

## Key

&emsp;Key 是一个继承自 Object 的抽象类，有两个注解 @immutable、@pragma('flutter:keep-to-string-in-subtypes') 预示了：key 以及子类都是不可变的，它们所有的字段必须是 final 修饰，以及在 key 的所有子类中保持它们重写的 toString 函数，key 的子类分别都重写了 toString 函数。(关于 @pragma('flutter:keep-to-string-in-subtypes') 的介绍，可以看下一小节的知识点扩展。)

&emsp;Key 虽然是一个抽象类，但是它提供一个工厂函数可以供我们直接使用，它的内部是由 ValueKey 来实现的。（例如：const Text(key: Key('234'), '123');，Key('234') 其实就是：ValueKey('234')。）

```dart
@immutable
@pragma('flutter:keep-to-string-in-subtypes')
abstract class Key {
  // 使用给定的 String value 构建一个 ValueKey<String>
  const factory Key(String value) = ValueKey<String>;

  // 默认的构造函数，由 key 子类使用，
  // 这样做很有用，因为 Key.new 工厂构造函数会遮蔽隐式构造函数，从而使子类可以调用，
  // LocalKey 和 GlobalKey 构造函数都会调用它：super.empty()。
  @protected
  const Key.empty();
}
```

### @pragma('flutter:keep-to-string-in-subtypes')

&emsp;为了在优化代码大小的过程中保留 Object.toString 的重写而不是将其移除，可以使用以注解。对于某些 URI（当前为 dart:ui 和 package:flutter），Dart 编译器在 profile/release 模式下会删除类中的 Object.toString 重写以减少代码大小。

&emsp;单个类可以通过以下注释选择不执行此操作：

+ @pragma('flutter:keep-to-string')
+ @pragma('flutter:keep-to-string-in-subtypes')

&emsp;例如，在以下类中，即使编译器的 --delete-tostring-package-uri 选项会将其替换为 return super.toString()，toString 方法仍将保持为 `return _buffer.toString()`。（按照惯例，dart:ui 通常作为 ui 导入，因此带有前缀。）

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

## LocalKey

&emsp;LocalKey 有点特殊，此抽象类定义出来，感觉仅仅是为了把 LocalKey 的子类和 GlobalKey 作出区分。

&emsp;在同一个父级元素中，key 必须是唯一的。相比之下，GlobalKey 必须在整个应用程序中是唯一的。

&emsp;LocalKey 直接继承自 Key 的抽象类，内部仅有一个 const 抽象构造函数。 

```dart
abstract class LocalKey extends Key {
  // 抽象的 const 构造函数。
  // 这个构造函数使得子类能够提供 const 构造函数，以便它们能在 const 表达式中被使用。
  const LocalKey() : super.empty();
}
```

&emsp;下面开始看 LocalKey 的子类：UniqueKey、ValueKey、ObjectKey。

## UniqueKey

&emsp;UniqueKey 直接继承自 LocalKey。一个我们可以直接使用的 key，一般在一组相同的 Widget 中，而我们却没有常量数据构造其它类型的 key 时，使用它（由系统为我们提供构建 key 所需的常量数据）。UniqueKey 还有一个特殊点，

&emsp;UniqueKey 是一个只等于自身的 key。而且它不能用 const 构造函数创建，如果用 const 构建函数的话意味着所有实例化的 UniqueKey 都是同一个实例，因此不是唯一的。

```dart
class UniqueKey extends LocalKey {
  // 创建一个仅等于其本身的 key
  // 这个 Key 不能使用 const 构造函数创建，因为这意味着所有实例化的键将是相同的实例，因此将不是唯一的。
  // ignore: prefer_const_constructors_in_immutables , never use const for this class
  // 忽略：prefer_const_constructors_in_immutables，从不对该类使用 const。
  UniqueKey();

  @override
  String toString() => '[#${shortHash(this)}]';
}
```

&emsp;为了保证 UniqueKey 的唯一性，它不提供 Const 构造函数。开始没有看 UniqueKey 的内容之前，只看名字，我还以为它是由系统为每个 UniqueKey 变量提供一个 Unique 值来保证它的唯一性，而实际它只是靠不提供 const 构造函数和以 Object 只等于自身来保证唯一性。

## ValueKey

&emsp;ValueKey 是一个直接继承自 LocalKey 的泛型类，它是一个使用特定类型 T 的值来标识自身的 Key。只有当两个 `ValueKey<T>` 的 value 是 operator== 时，它们的 `ValueKey<T>` 才相等。

&emsp;Valuekey 可以被子类化用来创建 value key，这样可以让碰巧使用相同 value 的其他 value key 之间不再相等。如果子类是私有的，这将导致一个 value key 类型，其 value key 不会与其他来源的 key 发生冲突，这可能会很有用，例如，如果这些 key 正在作为相同作用域中从另一个 widget 提供的 key 的后备使用。

&emsp;下面看 ValueKey 源码，首先是它的 const 构造函数，需要传递一个 value，并且两个 value 相等的 ValueKey 也是相等的。

```dart
class ValueKey<T> extends LocalKey {
  /// Creates a key that delegates its [operator==] to the given value.
  // 创建一个 key，并将其 operator == 委托给给定的 value。
  const ValueKey(this.value);
  
  // ...
}
```

### value

&emsp;ValueKey 有一个泛型字段：value，ValueKey 把自己的 operater == 委托给了它。

```dart
  final T value;
```

### operator ==

&emsp;ValueKey 重写了自己的 operator ==，可看到只要两个变量都是 ValueKey 类型，并且双方的 value 也相等的话，那么两个 ValueKey 就是相等的。

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

&emsp;ValueKey 重写了自己的 hashCode，调用 Object 的静态 hash 函数，以 runtimeType 和 value 字段一起计算一个 ValueKey 实例的哈希值。

```dart
  @override
  int get hashCode => Object.hash(runtimeType, value);
```

### toString

&emsp;因为 ValueKey 的 value 是一个任意类型 T，所以这里重写了它的 toString 函数，可以通过 toString 更全面的看出 value 的值。这样在检查 ValueKey 对象时能提供有用的信息，更利于调试或记录。

&emsp;例如 `ValueKey('123')` 打印它的 toString 是：`[<'123'>]`。`ValueKey([1, 2, 3])` 打印它的 toString 是：`[<[1, 2, 3]>]`。

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

&emsp;貌似没啥总结的，继承自 LocalKey，新增了一个泛型 T value 字段，我们可以随便使用任何类型的 value，重写了 hashCode 和 operator ==，主要靠这个 value 来保证唯一性。

&emsp;下面看一个与 ValueKey 极其相似的 key：ObjectKey。ObjectKey 也有一个 value 字段，但是它的类型是：`Object?`。在 ValueKey 中它的 value 字段被定义为了泛型 T，但是在 Dart 的语言环境之下，所有类型的基类都是 Object，所以这里和 ValueKey 比的话，那么 ObjectKey 的 value 字段也是可以使用各种类型的。那么 ValueKey 和 ObjectKey 的最主要区别在哪呢？其实主要在 operator == 上，ValueKey 的 == 中，使用的还是 value 的 == 来判等，而到了 ObjectKey 中，它的 == 中 value 的判等方式使用的是：identical。下面我们会详细看：identical 这个全局函数：`external bool identical(Object? a, Object? b);`。

## ObjectKey

&emsp;ObjectKey 是根据其 value 字段所代表的 Object 来确定自己的身份的，用来将一个 widget 的身份与生成该 widget 的 Object 的身份联系在一起。

&emsp;ObjectKey 和 ValueKey 一样，直接继承自：LocalKey。

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

&emsp;ObjectKey 也重写了 operator ==，但是这里和 ValueKey 明显不同的是，针对 value 字段的比较方式，ValueKey 比较时直接使用 value 的 operator ==，而 ObjectKey 比较时使用的 identical(other.value, value)，关于 identical 的细节，我们展开看一下：

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

&emsp;


 




## 参考链接
**参考链接:🔗**
+ [Key class](https://api.flutter.dev/flutter/dart-core/Object-class.html)
+ [When to Use Keys - Flutter Widgets 101 Ep. 4](https://www.youtube.com/watch?v=kn0EOS-ZiIc)
+ [VM-Specific Pragma Annotations](https://github.com/dart-lang/sdk/blob/main/runtime/docs/pragmas.md)
