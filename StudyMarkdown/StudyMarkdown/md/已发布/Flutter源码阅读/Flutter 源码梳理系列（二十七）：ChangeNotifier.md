# Flutter 源码梳理系列（二十七）：ChangeNotifier

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

&emsp;数据结构可以通过扩展或混合 ChangeNotifier 来实现 Listenable 接口，从而能够与监听 Listenable 对象变化的 widget 一起使用，比如 ListenableBuilder。

&emsp;以下示例实现了一个简单的计数器，利用 ListenableBuilder 限制仅对包含计数的 Text widget 进行重构。当前计数存储在 ChangeNotifier 子类中，当其值更改时，ChangeNotifier 会重构 ListenableBuilder 的内容。

```dart
import 'package:flutter/material.dart';

/// Flutter code sample for a [ChangeNotifier] with a [ListenableBuilder].

void main() {
  runApp(const ListenableBuilderExample());
}

class CounterModel with ChangeNotifier {
  int _count = 0;
  int get count => _count;

  void increment() {
    _count += 1;
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
  final CounterModel _counter = CounterModel();

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: const Text('ListenableBuilder Example')),
        body: CounterBody(counterNotifier: _counter),
        floatingActionButton: FloatingActionButton(
          onPressed: _counter.increment,
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
          
          // 感谢 ListenableBuilder，仅当 counterValueNotifier 通知其监听器时，
          // 显示当前计数的 widget 会被重建。
          // 位于上方的 Text widget 和 CounterBody 本身不会被重建。
          ListenableBuilder(
            listenable: counterNotifier,
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

&emsp;OK，接下来看 ChangeNotifier 的源码。

## Constructors

&emsp;ChangeNotifier 是一个实现了 Listenable 的 Mixin，所以它没有构造函数。

```dart
mixin class ChangeNotifier implements Listenable {
  // ...
  
}
```

## `_count`

&emsp;

```dart
  int _count = 0;
```

## `_listeners`

&emsp;`_listeners` 故意设置为固定长度的 `_GrowableList`，而不是 `const []`。

&emsp;`const []` 创建了一个 `_ImmutableList` 的实例，这与在该类的其他地方使用的定长 `_GrowableList` 是不同的。在该类的生命周期内保持运行时类型不变让编译器能够推断该属性的具体类型，从而提高性能。

```dart
  static final List<VoidCallback?> _emptyListeners = List<VoidCallback?>.filled(0, null);
  List<VoidCallback?> _listeners = _emptyListeners;
```

##

```dart
  int _notificationCallStackDepth = 0;
  int _reentrantlyRemovedListeners = 0;
```

## `_creationDispatched`

&emsp;如果为真，则表示此实例的事件 `ObjectCreated` 已派发给 `FlutterMemoryAllocations`。由于 `ChangeNotifier` 被用作 mixin，它没有构造函数，所以我们使用 `addListener` 来派发事件。

```dart
  bool _creationDispatched = false;
```

## hasListeners

&emsp;当前是否已注册任何监听器。

&emsp;客户端不应依赖此值来确定它们的行为，因为当另一个监听器开始或停止监听时，会导致极难追踪的错误。子类可能会使用这些信息来确定是否在没有监听器时执行任何工作；例如，在添加监听器时恢复一个 Stream，在移除监听器时暂停它。

&emsp;通常情况下，可以通过覆盖 addListener 方法，先检查 hasListeners 是否为 false，然后调用 super.addListener()，如果为 false，则开始执行需要的工作以确定何时调用 notifyListeners；类似地，通过重写 removeListener 方法，先调用 super.removeListener()，再检查 hasListeners 是否为 false，如果为 false，则停止相同的工作。

&emsp;如果 dispose 已被调用，则此方法返回 false。

```dart
  @protected
  bool get hasListeners => _count > 0;
```

## addListener

&emsp;注册一个闭包，在 Listenable 对象发生改变时会被调用。

&emsp;如果已经注册了给定的闭包，会添加一个额外的实例，并且必须在停止调用之前删除与添加相同次数的实例。

&emsp;在调用 dispose 方法后，不能再调用此方法。

&emsp;如果一个监听器被添加两次，并在迭代过程中被删除一次（例如作为响应通知），它仍然会被调用。另一方面，如果它被删除了和注册次数一样多的次数，那么它将不再被调用。这种奇怪的行为是 ChangeNotifier 无法确定哪个监听器被移除了的结果，因为它们是相同的，因此一旦知道仍有任何监听器注册，它仍然会保守地调用所有监听器。

&emsp;当在两个分别将所有注册传递到共同上游对象的对象上注册监听器时，可能会意外观察到此令人惊讶的行为。

&emsp;另请参阅：

+ removeListener，从通知对象发生改变时被调用的闭包列表中移除先前注册的闭包。

```dart
  @override
  void addListener(VoidCallback listener) {
    // 当 _count 记录现有的监听者的数量，当监听者已装满 _listeners 时，
    // 并又有新的的监听者要添加进来时，则把 _listeners 扩容为 2 倍。
    
    if (_count == _listeners.length) {
    
      if (_count == 0) {
        // 初始时，申请长度是 1 的 List
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
    
    // 把监听者放在 _listeners 中，_count 依次递增，记录监听者的数量
    _listeners[_count++] = listener;
  }
```

## `_removeAt`

&emsp;

```dart
  void _removeAt(int index) {
    // 由于性能原因，保存监听器的列表是不可变的。
    // 如果在 notifyListeners 迭代之外添加了很多监听器，
    // 然后又将其删除，我们仍然希望缩小这个列表。
    // 我们只会在实际监听器数量是列表长度的一半时进行此操作。
    
    _count -= 1;
    
    // 如果当前 _listeners 列表的长度大于 _count 的二倍，
    // 即表示 _listeners 列表有一半都是空的情况的话，则缩小 _listeners 列表的长度。
    if (_count * 2 <= _listeners.length) {
      final List<VoidCallback?> newListeners = List<VoidCallback?>.filled(_count, null);

      // index 之前的监听器在同一位置。
      for (int i = 0; i < index; i++) {
        newListeners[i] = _listeners[i];
      }

      // 索引移动后，位于该 index 之后的监听器会向列表的开始移动。
      for (int i = index; i < _count; i++) {
        newListeners[i] = _listeners[i + 1];
      }

      _listeners = newListeners;
    } else {
      // 当监听器的数量超过列表长度的一半时，我们只需移动我们的监听器，以避免为整个列表重新分配内存。
      for (int i = index; i < _count; i++) {
        _listeners[i] = _listeners[i + 1];
      }
      
      _listeners[_count] = null;
    }
  }
```

## removeListener

&emsp;从当对象发生变化时通知的闭包列表中删除先前注册的闭包。

&emsp;如果给定的监听器未注册，则该调用会被忽略。

&emsp;如果 dispose 已被调用，该方法会立即返回。

&emsp;如果一个监听器被添加两次，并且在迭代过程中被移除一次（例如作为对通知的响应），它仍然会被调用。另一方面，如果它被移除的次数与注册的次数相同，那么它将不再被调用。这种奇怪的行为是由于 ChangeNotifier 无法确定被移除的监听器是哪一个，因为它们是相同的，因此它会保守地在仍然知道有任何监听器被注册时仍然调用所有监听器。

&emsp;当在两个将所有注册转发到一个共同上游对象的独立对象上注册监听器时，可能会意外地观察到这种令人惊讶的行为。

&emsp;另请参阅：

+ addListener，将一个闭包注册为在对象发生变化时调用。

```dart
  @override
  void removeListener(VoidCallback listener) {
    // This method is allowed to be called on disposed instances for usability
    // reasons. Due to how our frame scheduling logic between render objects and
    // overlays, it is common that the owner of this instance would be disposed a
    // frame earlier than the listeners. Allowing calls to this method after it
    // is disposed makes it easier for listeners to properly clean up.
    
    // 由于我们在渲染对象和覆盖层之间的帧调度逻辑，
    // 基于可用性原因允许在已释放实例上调用此方法。
    // 通常情况下，这个实例的所有者会在侦听器之前一个帧被释放。
    // 允许在释放后调用此方法可以让侦听器更容易进行适当的清理工作。
    for (int i = 0; i < _count; i++) {
      final VoidCallback? listenerAtIndex = _listeners[i];
      if (listenerAtIndex == listener) {
        if (_notificationCallStackDepth > 0) {

          // 在 notifyListeners 迭代过程中，我们不会调整列表的大小，
          // 而是将我们想要移除的监听器置为 null。
          // 在所有 notifyListeners 迭代结束时，我们将有效地调整列表的大小。
          _listeners[i] = null;
          _reentrantlyRemovedListeners++;
          
        } else {
          
          // 当我们在 `notifyListeners` 迭代之外时，我们可以有效地缩小列表。
          _removeAt(i);
        }
        
        // 如果找到了对应的要移除的监听器，处理完毕，则可以跳出循环了。
        break;
      }
    }
  }
```

## dispose

&emsp;释放对象使用的任何资源。在调用此方法之后，对象将不再处于可用状态，应将其丢弃（在对象被处理后调用 addListener 会抛出异常）。

&emsp;应该只由对象的所有者调用此方法。

&emsp;此方法不会通知监听器，并在调用后清除监听器列表。此类的使用者必须在处理之前立即决定是否通知监听器。

```dart
  @mustCallSuper
  void dispose() {
    
    // 在调用 "notifyListeners()" 时，$this 上的 "dispose()" 方法被调用。
    // 这可能会导致错误，因为它在使用列表时修改了监听器列表。
    
    _listeners = _emptyListeners;
    _count = 0;
  }
```

## notifyListeners

&emsp;调用所有已注册的监听器。

&emsp;每当对象发生更改时调用此方法，以通知对象可能已更改的任何客户端。在此迭代中添加的监听器将不被访问。在此迭代中移除的监听器将在移除后不再被访问。

&emsp;监听器抛出的异常将被捕获并使用 FlutterError.reportError 报告。

&emsp;此方法在调用 dispose 后不能再调用。

&emsp;当重入地移除已注册多次的监听器（例如作为对通知的响应）时，可能会导致出现意外行为。请参阅 removeListener 处的讨论。

```dart
  @protected
  @visibleForTesting
  @pragma('vm:notify-debugger-on-exception')
  void notifyListeners() {
    if (_count == 0) {
      return;
    }

    // To make sure that listeners removed during this iteration are not called,
    // we set them to null, but we don't shrink the list right away.
    // By doing this, we can continue to iterate on our list until it reaches
    // the last listener added before the call to this method.
    
    // 为确保在此迭代过程中删除的监听器不会被调用，我们将它们设置为 null，但并不立即收缩列表。
    // 通过这样做，我们可以继续迭代我们的列表，直到达到在调用此方法之前添加的最后一个监听器。

    // To allow potential listeners to recursively call notifyListener, we track
    // the number of times this method is called in _notificationCallStackDepth.
    // Once every recursive iteration is finished (i.e. when _notificationCallStackDepth == 0),
    // we can safely shrink our list so that it will only contain not null
    // listeners.
    
    // 为了允许潜在的监听者递归调用 `notifyListener`，
    // 我们在 `_notificationCallStackDepth` 中追踪这个方法被调用的次数。
    // 一旦每个递归迭代都完成（即当 `_notificationCallStackDepth == 0` 时），
    // 我们可以安全地缩小我们的列表，使其仅包含非空监听器。

    _notificationCallStackDepth++;

    final int end = _count;
    for (int i = 0; i < end; i++) {
      try {
        _listeners[i]?.call();
      } catch (exception, stack) {
        // ...
      }
    }

    _notificationCallStackDepth--;

    if (_notificationCallStackDepth == 0 && _reentrantlyRemovedListeners > 0) {
      // We really remove the listeners when all notifications are done.
      final int newLength = _count - _reentrantlyRemovedListeners;
      if (newLength * 2 <= _listeners.length) {
        // As in _removeAt, we only shrink the list when the real number of
        // listeners is half the length of our list.
        final List<VoidCallback?> newListeners = List<VoidCallback?>.filled(newLength, null);

        int newIndex = 0;
        for (int i = 0; i < _count; i++) {
          final VoidCallback? listener = _listeners[i];
          if (listener != null) {
            newListeners[newIndex++] = listener;
          }
        }

        _listeners = newListeners;
      } else {
        // Otherwise we put all the null references at the end.
        for (int i = 0; i < newLength; i += 1) {
          if (_listeners[i] == null) {
            // We swap this item with the next not null item.
            int swapIndex = i + 1;
            while (_listeners[swapIndex] == null) {
              swapIndex += 1;
            }
            _listeners[i] = _listeners[swapIndex];
            _listeners[swapIndex] = null;
          }
        }
      }

      _reentrantlyRemovedListeners = 0;
      _count = newLength;
    }
  }
```




## 参考链接
**参考链接:🔗**
+ [Listenable class](https://api.flutter.dev/flutter/foundation/Listenable-class.html)
+ [`ValueListenable<T> class`](https://api.flutter.dev/flutter/foundation/ValueListenable-class.html)
+ [ChangeNotifier class](https://api.flutter.dev/flutter/foundation/ChangeNotifier-class.html)
