# Flutter 源码梳理系列（三）：GlobalKey、GlobalObjectKey

&emsp;GlobalKey 是一个在整个应用程序中是独一无二的 key。GlobalKey 可以唯一标识 element 对象，GlobalKey 提供了对此标识的 element对象 关联的其他对象的访问，例如：BuildContext。对于 StatefulWidget，通过 GlobolKey 标识的 element 对象，还可以访问它的 State 对象。

&emsp;拥有 GlobalKey 的 widget 在从树中的一个位置移动到树中的另一个位置时，会重置其子树的父级。为了重置父级，一个 widget 必须在相同的 animation frame 中到达树中的新位置，同时从树中的旧位置被移除。使用 GlobakKey 重置父级 element 是相对昂贵的操作，因为该操作会触发对关联 State 和其所有后代调用 State.deactivate；然后强制所有依赖于 InheritedWidget 的 widget 重建。

&emsp;如果你不需要上述列出的任何功能，请考虑改用 ValueKey、ObjectKey 或 UniqueKey。

&emsp;不能同时在树中包含两个具有相同 GlobalKey 的 widget。尝试这样做会在运行时断言。(GlobalKey 与 Widget 只能一一对应)

&emsp;GlobalKey 不应该在每次 build 中重新创建。它们通常应该是由 State 对象拥有的长期存在的对象。

&emsp;在每次 build 中创建一个新的 GlobalKey 会丢弃与旧 key 关联的子树状态，并为新 key 创建一个全新的子树。除了影响性能外，这还可能导致子树中 widget 的行为出现意外。例如，在子树中，一个 GestureDetector 将无法跟踪正在进行的手势，因为它将在每次构建时被重新创建。

&emsp;相反，一个好的做法是让一个 State 对象拥有 GlobalKey，并在 build 方法之外实例化它，比如在 State.initState 中。

&emsp;好了，下面看 GlobalKey 的源码：

# GlobalKey

&emsp;GlobalKey 是一个直接继承自 Key 的泛型抽象类。T 必须继承自 `State<StatefulWidget>`，这里限制泛型 T 是 `State<StatefulWidget>` 的子类，是用来修饰从全局 `_globalKeyRegistry` 中取出 GlobalKey 对应的 element 的 state 的。

&emsp;GlobalKey 虽然是抽象类，但是提供了实际返回 LabeledGlobalKey 的工厂构造函数：

```dart
    final text2 = Text(
      key: GlobalKey(debugLabel: 'label'),
      'GlobalKey 工厂构造函数，实际是：LabeledGlobalKey<T>',
    );
```

&emsp;同它的直接父类 Key 一样，也提供了一个实际返回 ValueKey 的工厂构造函数：

```dart
    final text3 = Text(
      key: Key('123'),
      'Key 工厂构造函数，实际是：ValueKey<String>',
    );
```

&emsp;然后看到 GlobalKey 类有一个 @optionalTypeArgs 注解。@optionalTypeArgs：用于注解 class、mixin、extension、function、method 或 typedef 声明 C 的装饰器。指示在 C 上声明的任何类型参数都应视为可选的。analyzer 和 linter 等工具可以使用这些信息来抑制警告，否则需要提供 C 上的类型参数。

```dart
@optionalTypeArgs
abstract class GlobalKey<T extends State<StatefulWidget>> extends Key {
  // 创建一个 LabeledGlobalKey，它是一个带有用于调试的标签的 GlobalKey。
  // 该标签纯粹用于调试，不用于比较 key 的身份。
  factory GlobalKey({ String? debugLabel }) => LabeledGlobalKey<T>(debugLabel);
  
  // 创建一个没有 label 的 global key。
  // 由于工厂构造函数遮蔽了隐式构造函数，所以由子类使用，初始化列表也调用了，key 的 const Key.empty();
  const GlobalKey.constructor() : super.empty();
}
```

&emsp;然后下面是一组超重要的 getter 函数，分别获取 GlobalKey 对应的：element、widget、state。（一家人整整齐齐。）

## `_currentElement`

&emsp;私有函数，获取 GlobalKey 在全局 `_globalKeyRegistry` 中保存的 element，之所以有这个私有函数是为了名字的隐藏，在前台展示时，多把 element 唤做 context 来用。关于其中的 `WidgetsBinding.instance.buildOwner!._globalKeyRegistry` 我们学习 element 时再看，这里用到的 `_globalKeyRegistry` 是一个全局 map：`final Map<GlobalKey, Element> _globalKeyRegistry = <GlobalKey, Element>{};`。

+ Key -> GlobalKey
+ Value -> Element

```dart
  Element? get _currentElement => WidgetsBinding.instance.buildOwner!._globalKeyRegistry[this];
```

## `currentContext`

&emsp;获取 GlobalKey 对应的 build context，即 currentElement。即是使用此 GlobalKey 构建 widget 的构建上下文。如果树中没有与此 global key 匹配的 widget，则返回 null。

```dart
  BuildContext? get currentContext => _currentElement;
```

## `currentWidget`

&emsp;获取 GlobalKey 对应的 widget。如果树中没有与此 global key 匹配的 widget，则返回 null。

```dart
  Widget? get currentWidget => _currentElement?.widget;
```

## `currentState`

&emsp;获取 GlobalKey 对应的 state，如果满足以下条件，当前 state 为 null：

1. 树中没有与此 global key 匹配的 widget。
2. 该 widget 不是 StatefulWidget，或者相关的 State 对象不是 T 的子类型。

```dart
  T? get currentState {
    final Element? element = _currentElement;
    if (element is StatefulElement) {
      final StatefulElement statefulElement = element;
      
      final State state = statefulElement.state;
      if (state is T) {
        return state;
      }
    }
    return null;
  }
```

## GlobalKey 总结

&emsp;GlobalKey 的内容看完了，它是一个继承自 Key 的抽象类，提供了调试时使用的一个工厂构造函数，返回一个 LabeledGlobalKey。然后它没有提供任何类似 value 的字段，这里其实我们可能就联想到了，对，它和 LocalKey 差不多，都是为自己的子类提供统一基类的。

&emsp;如以 LocalKey 为基类，发散出：UniqueKey、ValueKey、ObjectKey、PageStorageKey。而到了 GlobalKey，则发散出：LabeledGlobalKey、GlobalObjectKey。

&emsp;然后最最重要的来了，在一众的 LocalKey 的使用中，我们并没有任何接口可以根据它们去找到对应的 Widget、Element、State，关于使用 LocakKey 在树中复用 element 都是 Flutter framework 自动来进行的。而到了 GlobalKey，它提供了三个 getter：currentContext、currentWidget、currentState，可以从树中找到 GlobalKey 对应的 widget、element 和 state。

&emsp;当然由 globl key 找到 element 的方式仍然是用的最朴素的全局变量的方式，在一个全局 `Map<GlobalKey, Element>` 以键值对的方式保存它们。

&emsp;下面我们看 GlobalKey 的子类：
 
# LabeledGlobalKey

&emsp;LabeledGlobalKey 是一个直接继承自 GlobalKey 的 key，它是带有一个 string 类型的 debug 标签的 glbal key。debug 标签对于文档编写和调试很有用。该标签不会影响 key 的身份标识。同时 LabeledGlobalKey 不提供 const 构造函数，保证 LabeledGlobalKey 的全局唯一性。

```dart
@optionalTypeArgs
class LabeledGlobalKey<T extends State<StatefulWidget>> extends GlobalKey<T> {
  // 创建带有调试标签的 GlobalKey。标签不影响键的身份。
  // ignore: prefer_const_constructors_in_immutables，永远不要在此类中使用 const，保证不同的 LabeledGlobalKey 的唯一性。
  LabeledGlobalKey(this._debugLabel) : super.constructor();

  // 字符串类型的调试标签。
  final String? _debugLabel;

  @override
  String toString() {
    final String label = _debugLabel != null ? ' $_debugLabel' : '';
    if (runtimeType == LabeledGlobalKey) {
      return '[GlobalKey#${shortHash(this)}$label]';
    }
    
    return '[${describeIdentity(this)}$label]';
  }
}
```

&emsp;可以说是一个类似 value 值是 String 的 GlobalKey。

# GlobalObjectKey

&emsp;GlobalObjectKey 同 ObjectKey 一样，它的身份标识取决于作为其 value 字段所使用的 Object 对象。

&emsp;GlobalObjectKey 用于将一个 widget 与生成该 widget 时 key 参数所使用的 Object 的身份绑定在一起。

&emsp;任何以同一个 Object 创建的 GlobalObjectKey 将相等。主要体现在 hashCode 上，如果是用同一个 Object 构建两个 GlobalObjectKey，那么这两个 GlobalObjectKey 对象的 hashCode 是相同的，因为 GlobalObjectKey 类重写了它的 hashCode getter，仅仅是使用构建时传入的 Object 对象计算哈希码：`int get hashCode => identityHashCode(value);`。那么当把 hashCode 相同的 GlobalObjectKey 对象放入全局 `Map<GlobalKey, Element>` 中时，它们将会发生哈希冲突。

&emsp;如果 Object 不是私有的，那么可能会发生冲突（hashCode 相同），其中独立的 widget 将在树的不同部分重用相同 Object 作为它们的 GlobalObjectKey value，这会导致 global key 冲突。为避免这个问题，创建一个私有的 GlobalObjectKey 子类，如下：

```dart
class _MyKey extends GlobalObjectKey {
  const _MyKey(super.value);
}
```

&emsp;由于 global key 的 runtimeType 是其标识的一部分，这将防止与其他 GlobalObjectKey 发生冲突，即使它们具有相同的值。

&emsp;这里是使用了：`_MyKey` 和 `GlobalObjectKey` 的 runtimeType 不同来实现的，因为 GlobalObjectKey 的 operator == 判等时，需要两个变量的 runtimeType 相同，否则返回 false。

```dart
  Person p1 = Person(name: '123');
  
  final tempGlobalObjectKey = GlobalObjectKey(p1);
  final temp_MyKey = _MyKey(p1);
  final tempGlobalObjectKey2 = GlobalObjectKey(p1);
 
  print('🐯 GloblObjectKey 比较：${temp_MyKey == tempGlobalObjectKey}'); // false
  print('🐯 GloblObjectKey 比较：${tempGlobalObjectKey2 == tempGlobalObjectKey}'); // true
```

&emsp;OK，下面我们看一下 GlobalObjectKey 的源码：

```dart
@optionalTypeArgs
class GlobalObjectKey<T extends State<StatefulWidget>> extends GlobalKey<T> { 
  // 创建一个 global key，并在 value 上使用 identical 来进行 operator == 判定。
  const GlobalObjectKey(this.value) : super.constructor();
  
  // ...
}
```

&emsp;GlobalObjectKey 是一个直接继承自 GlobalKey 的泛型类。同时提供了一个 const 构造函数，但是注释也提到两个 GlobalObjectKey 对象判等是比较的它们的 value 字段。所以这里并不怕 const GlobalObjectKey 对象相等。 

## value

&emsp;GlobalObjectKey 有一个 Object 类型的字段，而且 GlobalObjectKey 完全以此 value 来进行 == 判定。

```dart
  final Object value;
```

## operator ==

&emsp;看到 GlobalObjectKey 和 ObjectKey 类似，都是直接以 identical 来进行 value 比较。当 value 是同一个对象时，两个 GlobalObjectKey 将相等。

```dart
  @override
  bool operator ==(Object other) {
    if (other.runtimeType != runtimeType) {
      return false;
    }
    
    return other is GlobalObjectKey<T>
        && identical(other.value, value);
  }
```

## hashCode

&emsp;GlobalObjectKey 的 hashCode getter，仅使用 value 的原始 Object.hash 来计算 hashCode，而在 ValueKey/ObjectKey 中，它们还把 runtimeType 加入了计算哈希值。

```dart
  @override
  int get hashCode => identityHashCode(value);
```

## toString

&emsp;toString 打印没啥特别的，裁切掉 `<State<StatefulWidget>>`，防止 toString 打印过长。

```dart
  @override
  String toString() {
    String selfType = objectRuntimeType(this, 'GlobalObjectKey');
    // The runtimeType string of a GlobalObjectKey() returns 'GlobalObjectKey<State<StatefulWidget>>'
    // because GlobalObjectKey is instantiated to its bounds. To avoid cluttering the output
    // we remove the suffix.
    const String suffix = '<State<StatefulWidget>>';
    if (selfType.endsWith(suffix)) {
      selfType = selfType.substring(0, selfType.length - suffix.length);
    }
    return '[$selfType ${describeIdentity(value)}]';
  }
```

## GlobalObjectKey 总结

&emsp;GlobalObjectKey 除了从 GlobalKey 继承来的可以从全局 `_globalKeyRegistry` 中取对应的：widget、element、state 之外和 ObjectKey 还挺接近的，特别是 value、operator == 和 hashCode，它们都是严格限制 value 是同一个对象。GlobalObjectKey 更严格的时，它对应的 element 只能在整棵树上出现一次。

&emsp;GlobalObjectKey 的泛型 T 指当从全局 `_globalKeyRegistry` 中取出对应的 element 是 StatefulElement 时它的 state 字段的类型。(即当我们把 GlobalObjectKey 作为 StatefulWidget 的 key 时，取对应的 State 对象使用。)

&emsp;其它的就是它们的唯一性都是直接靠它们的 value 字段决定的，并不是靠它们对象自身。其的暂时没想到了，整体看下来并不难，主要还是牢记它们的 operator == 判等和 hashCode getter，后面我们学习 element 时会用到它们的判等性以及插入集合中。

&emsp;LocalKey 和 GlobalKey 的相关的类看完了，如果只是看它们的类定义的话，就仅仅是一些重写了 operator == 和 hashCode 的类而已，当时当和 Widget 配合使用时就会显得复杂起来。后面我们学习 Element 时再深入看它们的使用。

+ Object => Key => `GlobalKey<T>` => GlobalObjectKey。
+ Object => Key => `GlobalKey<T>` => LabeledGlobalKey。

## 参考链接
**参考链接:🔗**
+ [`GlobalKey<T extends State<StatefulWidget>> class`](https://api.flutter.dev/flutter/widgets/GlobalKey-class.html)
+ [When to Use Keys - Flutter Widgets 101 Ep. 4](https://www.youtube.com/watch?v=kn0EOS-ZiIc)
+ [Key class](https://api.flutter.dev/flutter/dart-core/Object-class.html)
+ [Record class](https://api.flutter.dev/flutter/dart-core/Record-class.html)
+ [VM-Specific Pragma Annotations](https://github.com/dart-lang/sdk/blob/main/runtime/docs/pragmas.md)
+ [identical function](https://api.flutter.dev/flutter/dart-core/identical.html)
+ [LocalKey class](https://api.flutter.dev/flutter/foundation/LocalKey-class.html)
