# Flutter 源码梳理系列（二十七）：ChangeNotifier、`ValueNotifier<T> calss`

# Listenable

&emsp;Listenable 是一个抽象接口，实现了 Listenable 接口的类表示其具有通知的能力，如实现了 Listenable 的 ChangeNotifier Mixin，它通过维护了一个监听者列表，当数据发生变化时，它可以向所有的监听者发出通知。还有我们刚刚学习过的 PipelineManifold 它需要实现 Listenable，然后它的子类 `_BindingPipelineManifold` 继承自 ChangeNotifier，它通过监听 SemanticsBinding 是否开启语义化，来控制 PipelineOwner 的 `_semanticsOwner` 初始化与销毁。

&emsp;通常，这些监听者（或者是监听器对象）用于 notify clients 已被更新。(Listenable 发出通知，所有的监听器都会接收到通知。)

&emsp;这个 Listenable 接口有两个变体：

+ ValueListenable 是一个抽象接口，它在 Listenable 接口的基础上增加了当前值（T get value）的概念。
+ Animation 是一个抽象接口，它通过增加方向（正向或反向）的概念来扩展 ValueListenable 接口。（等后面我们学习 Flutter 动画时，会着重学习它。）
            
&emsp;Flutter API 中的许多类使用或实现这些接口。特别相关的子类有：

+ ChangeNotifier，可以被子类化或混合进来，用来创建实现 Listenable 接口的对象。
+ ValueNotifier，实现了 ValueListenable 接口，具有可变值 value，当该 value 被修改时会触发通知。（它可以理解为是一个 ChangeNotifier 的特化，直接指定了一个 T value 的值，当此 value 发生变化时，通知所有的监听者，相对而言 ChangeNotifier 更加灵活和自由，我们可以根据自己的情况，或者多个值来向监听者发送通知。）

&emsp;术语 "notify clients"、 "send notifications"、"trigger notifications" 和 "fire notifications" 可以互换使用。

&emsp;Listenable 机制直白一点理解的话就是我们在其它语言中经常见到的 "发布-订阅" 模式。

&emsp;See also:

+ AnimatedBuilder：是一个 widget，它使用一个 builder 回调来在特定的 Listenable 触发其通知时重新构建。这个 widget 通常与 Animation 子类一起使用，因此得名，但并不仅限于动画，因为它可以与任何 Listenable 一起使用。它是 AnimatedWidget 的一个子类，可用于创建从 Listenable 驱动的 widget。

+ ValueListenableBuilder：是一个 widget，它使用一个 builder（构建器）回调来在一个 ValueListenable 对象触发其通知时重新构建，将对象的值提供给 builder（构建器）。

+ InheritedNotifier：是一个抽象超类，用于使用 Listenable 的通知来触发子级 widget 中声明对它们的依赖性的重建，使用 InheritedWidget 机制。

+ Listenable.merge 创建一个 Listenable，当任何其他 Listenable 中的任何一个触发其通知时触发通知。（它聚集了一组 Listenable 对象，然后当一个监听者被添加时，它会添加到所有的 Listenable 中。然后任一 Listenable 发出通知的话，此监听者都会被执行。）

&emsp;OK，下面看一下 Listenable 抽象接口的代码，代码很简单，但是主要是它提供的这个通知机制的实现以及它的思想，我们应该好好学习一下。

## Constructors

&emsp;抽象常量构造函数。该构造函数使得子类能够提供常量构造函数，从而可以在常量表达式中使用。

```dart
abstract class Listenable {
  const Listenable();
  
  // ...
}
```

## Listenable.merge

&emsp;这是一个工厂方法，返回一个 `_MergingListenable` 对象，当给定的入参 listenables 列表中的任何一个 Listenable 发出通知，监听者都会收到通知。（工厂函数，直接返回一个：`_MergingListenable 对象`，这种工厂函数写法类似之前 key 学习时遇到的：Key(String value)，会直接返回 ValueKey。）

&emsp;一旦调用工厂方法，不得向可迭代对象（`Iterable<Listenable?> listenables`）添加或删除项。这样做会导致内存泄漏或异常。

&emsp;可迭代对象可能包含空值；它们将被忽略。

```dart
  factory Listenable.merge(Iterable<Listenable?> listenables) = _MergingListenable;
```

&emsp;`_MergingListenable` 类的内容很少，我们直接看一下它的代码。

&emsp;它的 `_children` 属性中收集的是一组 Listenable 对象，当我们向 `_MergingListenable` 添加监听者（VoidCallback，其实就是往 `_children` 中的所有 Listenable 添加此监听者，然后但凡 `_MergingListenable._children` 中任何一个 Listenable 发出通知，那么我们添加的监听者就会被执行。

### `_MergingListenable`

&emsp;构建一个 `_MergingListenable` 对象时直接传递一组 Listenable 对象。

```dart
class _MergingListenable extends Listenable {
  _MergingListenable(this._children);

  final Iterable<Listenable?> _children;
  
  // ...
}
```

#### addListener & removeListener

&emsp;当添加/移除 监听者时，就是遍历 `_children`，把此监听者从每一个 Listenable 中 添加/移除。

```dart
  @override
  void addListener(VoidCallback listener) {
    for (final Listenable? child in _children) {
      // 遍历 _children 把 listener 监听者添加到每一个 Listenable 中。
      child?.addListener(listener);
    }
  }
  
  @override
  void removeListener(VoidCallback listener) {
    for (final Listenable? child in _children) {
      // 遍历 _children 把 listener 监听者从每一个 Listenable 中移除。
      child?.removeListener(listener);
    }
  }
```

&emsp;Ok，我们继续看 Listenable 的代码。

## addListener

&emsp;注册一个闭包，当 Listenable 对象通知它的监听者时将被调用。(一个监听器/监听者就是一个 VoidCallback 闭包的形式，即一个无参返回值是 void 的函数。)

```dart
  void addListener(VoidCallback listener);
```

## removeListener

&emsp;从 Listenable 对象通知的闭包列表中移除先前注册的闭包。

```dart
  void removeListener(VoidCallback listener);
```

## Listenable 总结

&emsp;OK，Listenable 抽象接口，内容及其简单，仅仅限定了两个子类需要实现的抽象函数：addListener、removeListener 添加和移除监听者，而监听者就是一个无参数无返回值的闭包：typedef VoidCallback = void Function()，或者是一个无参返回值是 void 的函数。

# `ValueListenable<T>`

&emsp;一个暴露 value 的 Listenable 子类的抽象接口。

&emsp;这个抽象接口由 `ValueNotifier<T>` 和 `Animation<T>` 进行了实现，并允许其他 API 可以互相兼容地接受这两种实现。

+ Object -> Listenable -> ValueListenable

&emsp;OK，下面看一下 ValueListenable 中添加了哪些新内容。

## Constructors

&emsp;抽象常量构造函数。该构造函数使得子类能够提供常量构造函数，以便它们可以在常量表达式中使用。

```dart
abstract class ValueListenable<T> extends Listenable {
  const ValueListenable();
  
  // ...
}
```

## value

&emsp;ValueListenable 对象的当前值。当值改变时，会触发通过 addListener 添加的回调函数（监听者）。

```dart
  T get value;
```

&emsp;OK，ValueListenable 内容没了，仅仅是添加了一个泛型 value。

## ValueListenable 总结

&emsp;直接继承自 Listenable 的抽象泛型类 ValueListenable，直接提供了一个 `T get value` 值，当此值发生变化时，通知所有的监听者（实现手法也很简单，如 ValueNotifier 中的实现，仅是重写此 value 的 setter，在其新值到来时，直接调用 notifyListeners 函数，通知所有的监听者执行。）。

&emsp;那么它这种仅提供一个泛型 T 变量的方式想到谁了呢？是的：ValueKey 是也，它也直接提供了一个泛型 T 变量（final T value;）。

&emsp;OK，下面我们看超级重要的，切实实现了 Listenable 抽象接口的 Mixin：ChangeNotifier，看看它的内部逻辑，看看它是如何实现 Listenable 的 "发布-订阅" 模型的。

# ChangeNotifier

&emsp;ChangeNotifier：一个可以被扩展或混入（extended or mixed）的类，提供使用 VoidCallback 进行通知的变更通知 API。

&emsp;添加监听者的时间复杂度为 O(1)，而移除监听者和分发通知的时间复杂度为 O(N)（其中 N 是监听者的数量）。（维护监听者 List 的时间复杂度。）

## Using ChangeNotifier subclasses for data models

&emsp;数据部分可以通过扩展或混入 ChangeNotifier 来实现 Listenable 接口，从而能够与监听 Listenable 对象变化的 widget 一起使用，比如 ListenableBuilder。

&emsp;以下示例实现了一个简单的计数器，利用 ListenableBuilder 限制仅对包含计数的 Text widget 进行重构。当前计数存储在 ChangeNotifier 子类中，当其值更改时，ChangeNotifier 会重构 ListenableBuilder 的内容。

```dart
import 'package:flutter/material.dart';

/// Flutter code sample for a [ChangeNotifier] with a [ListenableBuilder].

void main() {
  runApp(const ListenableBuilderExample());
}

// 数据部分，CounterModel 混入 ChangeNotifier
class CounterModel with ChangeNotifier {
  int _count = 0;
  int get count => _count; // 向所有的订阅者提供一个 count 值

  void increment() {
    _count += 1;
    
    // 当 count 发生变化时，会调用 notifyListeners 函数通知所有的监听者。
    notifyListeners();
  }
}

class ListenableBuilderExample extends StatefulWidget {
  const ListenableBuilderExample({super.key});

  @override
  State<ListenableBuilderExample> createState() =>
      _ListenableBuilderExampleState();
}

class _ListenableBuilderExampleState extends State<ListenableBuilderExample> {
 
  // 直接在 State 中声明一个 CounterModel 属性。
  final CounterModel _counter = CounterModel();

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: const Text('ListenableBuilder Example')),
        
        body: CounterBody(counterNotifier: _counter), // 把 CounterModel 传递到自定义 CounterBody Widget 中去。
        
        floatingActionButton: FloatingActionButton(
        
          onPressed: _counter.increment, // 递增 CounterModel 的 count 值，向监听者发出通知。
          
          child: const Icon(Icons.add),
        ),
      ),
    );
  }
}

class CounterBody extends StatelessWidget {
  const CounterBody({super.key, required this.counterNotifier});

  final CounterModel counterNotifier;

  @override
  Widget build(BuildContext context) {
    return Center(
      child: Column(
        mainAxisAlignment: MainAxisAlignment.center,
        children: <Widget>[
          const Text('Current counter value:'),
          
          // 感谢 ListenableBuilder，仅当 counterValueNotifier 通知其监听者时，
          // 显示当前计数的 widget 会被重建。
          // 位于上方的 Text widget 和 CounterBody 本身不会被重建。
          ListenableBuilder(
            listenable: counterNotifier, // 告诉 counterNotifier 自己是一个监听者。
            builder: (BuildContext context, Widget? child) {
            
              return Text('${counterNotifier.count}');
              
            },
          ),
          
        ],
      ),
    );
  }
}
```

&emsp;OK，上面是 ChangeNotifier 最简单的一个使用实例，接下来看 ChangeNotifier 的源码。

## Constructors

&emsp;ChangeNotifier 是一个实现了 Listenable 的 Mixin，所以它没有构造函数，需要它的子类来实现。

```dart
mixin class ChangeNotifier implements Listenable {
  // ...
  
}
```

## `_count`

&emsp;用来记录当前这个 ChangeNotifier 对象已经添加了多少监听者。注意是现有的监听者的数量并不是 `_listeners` 列表的长度，可以理解为 `_listeners` 列表中监听者的数量。

```dart
  int _count = 0;
```

## `_listeners`

&emsp;`_listeners` 故意设置为固定长度的 `_GrowableList`，而不是 `const []`。

&emsp;`const []` 创建了一个 `_ImmutableList` 的实例，这与在该类的其他地方使用的定长 `_GrowableList` 是不同的。在该类的生命周期内保持运行时类型不变让编译器能够推断该属性的具体类型，从而提高性能。

&emsp;`_listeners` 是用来保存监听者的列表。

```dart
  static final List<VoidCallback?> _emptyListeners = List<VoidCallback?>.filled(0, null);
  List<VoidCallback?> _listeners = _emptyListeners;
```

## `_notificationCallStackDepth` & `_reentrantlyRemovedListeners`

&emsp;两个标识值。

```dart
  // 用于在 notifyListeners 函数中记录：
  // ⚠️ 当 notifyListeners 调用时，针对 _listeners 列表中的监听者迭代执行，
  // 其中可能会递归调用 notifyListeners 函数，
  // 使用 _notificationCallStackDepth 记录递归结束。
  int _notificationCallStackDepth = 0;
  
  // 用于在 removeListener 函数中记录：
  // ⚠️ 当前要删除 _listeners 中的某个监听者了，
  // 但是 notifyListeners 函数正在遍历这个 _listeners 列表，
  // 所以暂时把这个监听者的索引处置为 null，并且不尝试去缩短 _listeners 的长度。
  int _reentrantlyRemovedListeners = 0;
```

## addListener

&emsp;添加一个监听者，在 Listenable 对象发生改变时会被调用（通知）。

&emsp;如果已经添加了指定的监听者，这会添加一个同样的监听者（即同一个监听者，可以被添加多次。），并且必须在停止调用之前删除与添加相同次数的监听者。

&emsp;在调用 dispose 方法后，不能再调用此方法。

&emsp;如果一个监听者被添加两次，并在迭代过程（）中被删除一次（例如作为响应通知），它仍然会被调用。另一方面，如果它被删除了和注册次数一样多的次数，那么它将不再被调用。这种奇怪的行为是 ChangeNotifier 无法确定哪个监听器被移除了的结果，因为它们是相同的，因此一旦知道仍有任何监听器注册，它仍然会保守地调用所有监听器。（因为保存监听者用的上面的 `List<VoidCallback?> _listeners` 列表，它不会进行去重，只要有监听者添加进来，它就接收，所以同一个监听者，可以被添加多次。）

```dart
  @override
  void addListener(VoidCallback listener) {
    // _count 记录现有的监听者的数量，当监听者已经装满了 _listeners，
    // 又有新的的监听者要添加进来时，则对 _listeners 扩容为 2 倍。
    // 所以 _listeners.length 会以：1 2 4 8 16 ... 这样的规律递增。
    if (_count == _listeners.length) {
    
      if (_count == 0) {
        // 初始时，为 _listeners 申请长度是 1 的 List
        _listeners = List<VoidCallback?>.filled(1, null);
      } else {
      
        // 申请现有 _listeners.length 长度两倍的 List
        final List<VoidCallback?> newListeners =
            List<VoidCallback?>.filled(_listeners.length * 2, null);
        
        // 把旧的 Listenable 转移到新的 List 中    
        for (int i = 0; i < _count; i++) {
          newListeners[i] = _listeners[i];
        }
        
        // 然后把旧的 _listeners 指向新的 List
        _listeners = newListeners;
      }
    }
    
    // 把入参监听者 listener 放在 _listeners 中，
    // _count 依次递增，记录监听者的数量。
    _listeners[_count++] = listener;
  }
```

## `_removeAt`

&emsp;移除 `_listeners` 列表中指定索引的监听者。由于 `_listeners` 是一个固定长度的列表，所以当需要移除指定索引的元素时，这个索引后面的元素全部要往前移动一位。

&emsp;除了把指定索引的监听者移除之外，还做了一个优化，即当把此监听者移除以后，`_listeners` 列表内元素占用是否小于等于二分之一了，如果是的话，代表着 `_listeners` 列表内有一半的空间是空着的，此时就会缩短 `_listeners` 列表的长度。这样可以减小 N 的值，可以优化移除监听者和分发通知的时间复杂度 O(N)。

```dart
  void _removeAt(int index) {
    // 由于性能原因，保存监听者的列表是不可变的。
    // 如果在 notifyListeners 迭代之外添加了很多监听器，
    // 然后又将其删除，我们仍然希望缩小这个列表。
    // 我们只会在实际监听者数量是 _listeners 列表长度的一半时进行此操作。
    
    _count -= 1;
    
    // 如果当前 _listeners 列表的长度大于等于 _count 的二倍，
    // 即表示 _listeners 列表有一半都是空的情况的话，则缩小 _listeners 列表的长度。
    
    if (_count * 2 <= _listeners.length) {
      // 再申请一个长度是 _count 的列表。作为新的 _listeners 的值。
      // 即把 _listeners 列表缩短到现有监听者的数量。
      // 因为它依然是每次移除一个元素时就会进行缩短判断，
      // 所以整体而言 _listeners 列表的长度依然是维持在 1 2 4 8 16 ... 这样的长度值。
      final List<VoidCallback?> newListeners = List<VoidCallback?>.filled(_count, null);
      
      // 接下来把 _listeners 中的监听者，移动到新列表中。
      
      // index 之前的监听者的位置还不变，还在同一位置。
      // 把 index 之前的监听者移动到相同的索引位置。
      for (int i = 0; i < index; i++) {
        newListeners[i] = _listeners[i];
      }

      // index 之前的移动完成以后，开始移动 index 之后的监听者。
      // 它们整体上向前提升一位。
      
      // 位于该 index 之后的监听者索引缩小 1，相当于是整体向前提升一位。
      for (int i = index; i < _count; i++) {
        newListeners[i] = _listeners[i + 1];
      }

      // 然后把这个新列表赋值给 _listeners
      _listeners = newListeners;
    } else {
      // 当 _listeners 中监听者的数量仍然超过 _listeners 列表长度的一半时，
      // 我们只需移动我们的监听者，以避免为整个列表重新分配内存。
      
      // index 索引前面的监听者完全不动，index 之后的监听者的索引整体向前提升一位，
      // 把原 index 处直接覆盖掉。
      for (int i = index; i < _count; i++) {
        _listeners[i] = _listeners[i + 1];
      }
      
      // 然后把最后一位置为 null
      _listeners[_count] = null;
    }
  }
```

## removeListener

&emsp;从当对象发生变化时通知的闭包列表（`_listeners`）中删除先前注册的闭包。（即移除指定的监听者。）

&emsp;如果给定的监听者（listener）未添加过，则该调用会被忽略。

&emsp;如果 dispose 已被调用，该方法会立即返回。

&emsp;如果一个监听者被添加两次，并且在迭代过程中被移除一次（例如作为对通知的响应），它仍然会被调用。另一方面，如果它被移除的次数与注册的次数相同，那么它将不再被调用。这种奇怪的行为是由于 ChangeNotifier 无法确定被移除的监听者是哪一个，因为它们是相同的，因此它会保守地在仍然知道有任何监听者被注册时仍然调用所有监听者。（`_listeners` 只是一个单纯的 List，没有去重机制，所有添加进来的监听者都会被照单全收。）

&emsp;当在两个将所有注册转发到一个共同上游对象的独立对象上注册监听器时，可能会意外地观察到这种令人惊讶的行为。

```dart
  @override
  void removeListener(VoidCallback listener) {
    // 由于我们在渲染对象和覆盖层之间的帧调度逻辑，
    // 基于可用性原因允许在已释放实例上调用此方法。
    // 通常情况下，这个实例的所有者会在监听者之前一个帧被释放。
    // 允许在释放后调用此方法可以让监听者更容易进行适当的清理工作。
    
    for (int i = 0; i < _count; i++) {
      final VoidCallback? listenerAtIndex = _listeners[i];
      
      // 遍历到了指定的监听者。
      if (listenerAtIndex == listener) {
    
        // 如果 _notificationCallStackDepth 大于零，表示当前 notifyListeners 函数正在执行，
        // 它正在迭代 _listeners 列表。而此时我们却想从 _listeners 列表中移除元素，
        // 那么我们只把 _listeners 列表中这个指定监听者的索引处置为 null，
        // 并不去缩小 _listeners 列表，去修改它。
        
        if (_notificationCallStackDepth > 0) {
          // 在 notifyListeners 迭代过程中，我们不会调整列表的大小，
          // 而是将我们想要移除的监听器置为 null。
          // 在所有 notifyListeners 迭代结束时，我们将有效地调整列表的大小。
          
          // 把这个入参 listener 的索引处置为 null，即表示把 listener 从 _listeners 列表中删除。
          // 但是并不像 _removeAt 那样，会判断是否缩短 _listeners 列表的长度。
          _listeners[i] = null;
          
          // 用 _reentrantlyRemovedListeners 记录下，
          // 从 _listeners 列表中删除了监听者，
          // 但是并没有去判断是否缩短 _listeners 列表的长度。
          _reentrantlyRemovedListeners++;
        } else {
          
          // 当我们在 `notifyListeners` 迭代之外时，我们可以有效地缩小列表。
          // 即当前 notifyListeners 没有被调用，那么直接通过 _removeAt 移除指定索引监听者即可，
          // 同时还会根据 _listeners 列表剩余元素数量继而缩短它，提高后序性能。
          _removeAt(i);
        }
        
        // 如果找到了对应的要移除的监听器，处理完毕，则可以跳出循环了。
        // 注意这里并不会继续遍历下去，假如后序有与入参 listener 相同的监听者也并不会被移除掉。
        break;
      }
    }
  }
```

## dispose

&emsp;释放 ChangeNotifier 对象使用的任何资源。在调用此方法之后，ChangeNotifier 对象将不再处于可用状态，应将其丢弃（在对象被处理后调用 addListener 会抛出异常）。

&emsp;应该只由 ChangeNotifier 对象的所有者调用此方法。

&emsp;此方法不会通知监听者，并在调用后清除监听者列表。此类的使用者必须在处理之前立即决定是否通知监听者。

```dart
  @mustCallSuper
  void dispose() {
    
    // 在调用 "notifyListeners()" 时，当前 ChangeNotifier 对象上的 "dispose()" 方法被调用。
    // 这可能会导致错误，因为它在使用列表时修改了监听器列表。
    
    // 直接把 _listeners 置为空占位 List。
    _listeners = _emptyListeners;
    // 把 _count 置为 0，表示目前已经没有监听者啦。
    _count = 0;
  }
```

## notifyListeners

&emsp;调用所有已注册的监听者。（最重要的函数来了。）

&emsp;每当 ChangeNotifier 对象发生更改时调用此方法，以通知对象可能已更改的任何客户端。在此迭代中添加的监听者将不被访问（即在迭代 `_listeners` 列表时还向里面添加的监听者，那这个监听者在本次迭代中不会被调用）。在此迭代中移除的监听者将在移除后不再被访问（会直接被置为 null。）。

&emsp;监听者抛出的异常将被捕获并使用 FlutterError.reportError 报告。

&emsp;此方法在调用 dispose 后不能再调用。

&emsp;当重入地移除已注册多次的监听器（例如作为对通知的响应）时，可能会导致出现意外行为。（即根据索引正迭代列表呢，而去移除列表元素并修改列表长度，这都是不允许的。）

```dart
  @protected
  @visibleForTesting
  @pragma('vm:notify-debugger-on-exception')
  void notifyListeners() {
    // 如果 _count 值为 0 了，表示目前还没有添加监听者，或者 ChangeNotifier 对象已经调用了 dispose 函数等待销毁了，
    // 那此时都直接 return 即可。
    if (_count == 0) {
      return;
    }
    
    // 为确保在此迭代过程中删除的监听者不会被调用，我们将它们设置为 null，但并不立即收缩列表。
    // 通过这样做，我们可以继续迭代我们的列表，直到达到在调用此方法之前添加的最后一个监听器。
    // 正如上面 👆 removeListener 函数的内容一样，正是对此描述的代码实现。

    // To allow potential listeners to recursively call notifyListener, we track
    // the number of times this method is called in _notificationCallStackDepth.
    // Once every recursive iteration is finished (i.e. when _notificationCallStackDepth == 0),
    // we can safely shrink our list so that it will only contain not null
    // listeners.
    
    // 为了允许潜在的监听者递归调用 notifyListener，（即在监听者回调中又调用了 notifyListener 函数。）
    // 我们在 _notificationCallStackDepth 中追踪这个方法被调用的次数。
    // 一旦每个递归迭代都完成（即当 _notificationCallStackDepth == 0 时），
    // 我们可以安全地缩小我们的列表，使其仅包含非空监听器。（即优化 _listeners 的长度，以及把里面的 null 元素清理掉。）

    // 记录 notifyListeners 被调用的次数，
    // 递增 1
    _notificationCallStackDepth++;

    // 这里用一个临时变量 end，固定住 for 循环的次数。
    final int end = _count;
    
    for (int i = 0; i < end; i++) {
      try {
      
        // 依次执行添加的监听者。
        // ⚠️ 这些回调里面可能会递归的调用 notifyListeners 函数，
        // 或者会调用上面👆的 addListener 和 removeListener 函数，都是有可能。
        // 所以上面的 int end 可以固定 for 循环的次数，即是添加了新的监听者 _count 的值变了，
        // 也不影响本次的 for 循环的次数，依然是最初的 _count 的次数。
        _listeners[i]?.call();
        
      } catch (exception, stack) {
        // ...
      }
    }
    
    // 记录 notifyListeners 被调用的次数，
    // 递减 1，和上面的 _notificationCallStackDepth++ 是匹配的，
    // 即使上面👆 _listeners[i]?.call() 内部递归调用了 notifyListeners 函数，
    // 那么当 _notificationCallStackDepth 减少到 0 是，说明递归结束了。
    _notificationCallStackDepth--;

    // 然后 _listeners 内的监听者执行完毕以后，开始对现有现场进行打扫。
    
    // 两个标识 _notificationCallStackDepth == 0 标识，_listeners 迭代已经结束了，即使有递归。
    // _reentrantlyRemovedListeners > 0 说明在 _listeners 迭代过程中有监听者被移除了。
    // 那么这样的话需要处理现场，否则什么也不用做。
    
    if (_notificationCallStackDepth == 0 && _reentrantlyRemovedListeners > 0) {
      // 当所有监听者都完成后，我们才会真正移除监听器。
      // _reentrantlyRemovedListeners > 0 表示在上面监听者执行过程中，有监听者被移除了，
      // 并且 _reentrantlyRemovedListeners 的值就是被移除的监听者的数量。
      
      // newLength 就表示目前剩余的监听者的数量了。
      final int newLength = _count - _reentrantlyRemovedListeners;
      
      // 同样，如果目前 _listeners 列表中有一半的空间是空的话，则要缩短 _listeners 的长度了。
      if (newLength * 2 <= _listeners.length) {
        // 就像在 _removeAt 方法中一样，我们只有在监听者的实际数量是列表长度的一半时才会缩小列表。
        final List<VoidCallback?> newListeners = List<VoidCallback?>.filled(newLength, null);

        // 把 _listeners 列表中监听者移动到新列表中去，并且清理掉其中的 null。
        int newIndex = 0;
        for (int i = 0; i < _count; i++) {
          final VoidCallback? listener = _listeners[i];
          
          // 这里要有一个判空，如果这个位置的监听者被移除了，
          // 那么要把这个空位置清理掉。
          if (listener != null) {
            newListeners[newIndex++] = listener;
          }
        }
        
        // 新的 _listeners 
        _listeners = newListeners;
      } else {
        // 即虽然 _listeners 中有监听者被移除，但是 _listeners 列表中整体监听者的占用数量还是超过了一半，
        // 那么就会遍历 _listeners 列表，把其中的 null 元素都移动到 _listeners 的末尾去。
        
        // 否则，我们将所有的空引用放在末尾。
        
        for (int i = 0; i < newLength; i += 1) {
        
          // 如果遇到了 null 元素，就往 null 后面找非 null 的元素，放在 null 这个位置，
          // 把 null 放在它的位置。
          // 感觉这个算法好慢O(N 的平方了)，唯一的好处时维持了 _listeners 中监听者的现有顺序。
           
          if (_listeners[i] == null) {
            // We swap this item with the next not null item.
            int swapIndex = i + 1;
            
            // 找到 null 后面距离最近的非 null 的监听者。
            while (_listeners[swapIndex] == null) {
              swapIndex += 1;
            }
            
            // 交换 null 与非 null
            _listeners[i] = _listeners[swapIndex];
            _listeners[swapIndex] = null;
          }
        }
      }
      
      // _listeners 列表整理完毕后，
      // _reentrantlyRemovedListeners 置回 0，待下次使用。
      _reentrantlyRemovedListeners = 0;
      
      // _count 更新为目前真实的 _listeners 列表中监听者的数量。
      _count = newLength;
    }
  }
```

## ChangeNotifier 总结

&emsp;OK，ChangeNotifier 内容看完了，实际实现 "发布-订阅" 模式的代码逻辑很简单，addListener 添加监听者，removeListener 移除监听者，notifyListeners 通知所有的监听者执行，然后其中添加了一些对存储监听者的列表的优化，例如：当需要添加新的监听者时，监听者列表已存满时会把它扩容为二倍，当需要移除监听者时，会判断监听者列表的占用是否少于一半了，如果是的话会把监听者列表缩短为一半长度，继而提高 notifyListeners 的性能。同时 notifyListeners 内部也会判断在监听者执行过程中是否调用了 removeListener，继而在 notifyListeners 内对监听者列表遍历结束后，整理监听者列表：缩容到一半长度或者把非 null 的监听者保持原有顺序移动到监听者列表的前面去。

&emsp;OK，下面我们看一下实现了 ChangeNotifier 的特殊情况：ValueNotifier。

# ValueNotifier

&emsp;ValueNotifier：一个持有单个数值的 ChangeNotifier。（单个值的表现形式：`T get value => _value;`）

&emsp;当 value 被替换为与旧值通过等式运算符 == 进行评估不相等的内容时，该 ValueNotifier 对象会通知其监听者。

## Limitations

&emsp;因为这个 ValueNotifier 类只在 value 的标识发生变化时通知监听者，所以当 value 本身内部的可变状态发生变化时，并不会通知监听者。

&emsp;例如，一个 `ValueNotifier<List<int>>` 当列表的内容发生变化时不会通知它的监听者。

&emsp;因此，最好将这个类与只使用不可变数据类型的情况下配合使用。(例如：SemanticsBinding 内部的：`late final ValueNotifier<bool> _semanticsEnabled = ValueNotifier<bool>(platformDispatcher.semanticsEnabled)`。)

&emsp;对于可变数据类型，考虑直接扩展 ChangeNotifier。

+ Object -> ChangeNotifier -> ValueNotifier。

## Constructors

&emsp;创建一个包装了 value 的 ChangeNotifier。可以看到 ValueNotifier 直接继承自 ChangeNotifier 并实现了 ValueListenable 抽象接口。

```dart
class ValueNotifier<T> extends ChangeNotifier implements ValueListenable<T> {
  ValueNotifier(this._value) {
    // ...
  }
```

## value

&emsp;value 属性表示存储在 ValueNotifier 中的当前值。当将 value 替换为通过相等运算符 == 评估为不等于旧 value 的内容时，该 ValueNotifier 对象将通知其监听者。

&emsp;直接在 value 的 setter 中比较新旧 value 的值，如果新值来了，那么通知所有的监听者执行。

```dart
  @override
  T get value => _value;
  T _value;
  
  set value(T newValue) {
    // 如果新旧值相等的话，直接 return，并不会发出通知。
    if (_value == newValue) {
      return;
    }
    
    // 新值
    _value = newValue;
    
    // 通知所有的监听者执行。
    notifyListeners();
  }
```

## ValueNotifier 总结

&emsp;看到 ValueNotifier 的内容算是 ChangeNotifier 的一种特殊情况，它直接存储了一个泛型 T value，当 value 发生变化时通知监听者执行。当然注意这里的变化是指：新旧 value 通过 operator == 判等返回 false，如果 value 是一个 `List<int>`，那么 value 中添加或者移除了元素的话，value 是没有发生变化的，此时监听者并不会得到执行的，所以使用 ValueNotifier 时，我们是要注意使用场景的。因此，ValueNotifier 最好只在使用不可变数据类型的情况下使用，例如：`ValueNotifier<bool>`、`ValueNotifier<int>` 等。

## 参考链接
**参考链接:🔗**
+ [Listenable class](https://api.flutter.dev/flutter/foundation/Listenable-class.html)
+ [`ValueListenable<T> class`](https://api.flutter.dev/flutter/foundation/ValueListenable-class.html)
+ [ChangeNotifier class](https://api.flutter.dev/flutter/foundation/ChangeNotifier-class.html)
+ [`ValueNotifier<T> class`](https://api.flutter.dev/flutter/foundation/ValueNotifier-class.html)
