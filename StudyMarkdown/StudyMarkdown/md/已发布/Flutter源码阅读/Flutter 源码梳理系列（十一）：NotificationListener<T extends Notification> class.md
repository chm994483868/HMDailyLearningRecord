# Flutter 源码梳理系列（十一）：`NotificationListener<T extends Notification> class`

&emsp;在上面两篇我们学习了超级重要的 BuildOwner 和 BuildScope，然后再加上之前看到 `Element._updateInheritance` 函数内通过：`_inheritedElements = _parent?._inheritedElements` 子级 Element 节点直接拿到父级节点的 `_inheritedElements`，如果是遇到 `InheritedElement._updateInheritance` 时它会把自己添加到 `_inheritedElements` 里面。 

```dart
  @override
  void _updateInheritance() {
    // 这里有个判空操作初始化操作。
    // Element tree 从根节点开始，一层一层构建，其实直到遇到第一个 InheritedElement 节点时，Element 对象的 _inheritedElements 属性才会赋初值
    final PersistentHashMap<Type, InheritedElement> incomingWidgets =
        _parent?._inheritedElements ?? const PersistentHashMap<Type, InheritedElement>.empty();
    
    // 然后 InheritedElement 节点以自己的类型为 Key 放在这个 PersistentHashMap 里面，后续子级就可以 O(1) 时间取得对应类型的 InheritedElement 了
    _inheritedElements = incomingWidgets.put(widget.runtimeType, this);
  }
```

+ `_owner`
+ `_parentBuildScope`
+ `_inheritedElements`

&emsp;然后呢，在 Element.mount 函数内我们可以发现它们三个有一个共同点：就是它们都是由父级直接传递的。然后除了它们三个之外呢还有最后一个较重要的属性也是由父级直接往子级传递的：

+ `_NotificationNode? _notificationTree` 

&emsp;所以本篇学习一下 `_notificationTree` 相关的内容，学习完这个呢我们就可以深入 Element 啦！（其实 Element 类的源码我已经看完了，内容后面再发出来。因为如果不深入学习 Element tree 的构建过程的话，直接看这些与 Element tree 相关的内容的话，理解起来是比较难的。⛽️）

&emsp;OK，我们开始挖掘 `_notificationTree` 相关的功能。

# Element.attachNotificationTree

&emsp;首先是 Element 的 attachNotificationTree 函数，正是它在每个 Element 节点上传递 `_notificationTree`。下面是官方注释：

&emsp;attachNotificationTree 函数在 Element.mount 和 Element.activate 中调用以将该 Element 对象注册到 notification tree 中，这个方法只是为了能够实现 NotifiableElementMixin 而被暴露出来的。

&emsp;希望响应通知的 Element 子类应该使用混入（mixin）来实现。

&emsp;另请参见：NotificationListener，一个允许监听通知的 Widget。

```dart
  @protected
  void attachNotificationTree() {
    _notificationTree = _parent?._notificationTree;
  }
```

&emsp;OK，下面我们解释一下上面的注释，首先是 attachNotificationTree 函数的重写，在非 NotifiableElementMixin Elment 中呢，attachNotificationTree 函数只是传递一下父级 Element 的 `_notificationTree` 引用，而到了 NotifiableElementMixin Elment 中呢，它进行了重写：

```dart
  @override
  void attachNotificationTree() {
    _notificationTree = _NotificationNode(_parent?._notificationTree, this);
  }
```

&emsp;没错，如果当前 Element 对象是 NotifiableElementMixin 的话，就会使用该 Element 对象（this）和父级 `_parent?._notificationTree` 创建一个 `_NotificationNode` 节点。(正如 InheritedElement 中重写 `_updateInheritance` 函数一样，如果遇到 InheritedElement 节点，它就会把自己加入 `_inheritedElements` 中，而非 InheritedElement 节点的话，只是在 `_updateInheritance` 函数内传递父级 Element 的 `_inheritedElements` 引用。)

&emsp;那么谁是 notification tree 呢？正是指由此 `_NotificationNode` 节点组成的单向链表。`_NotificationNode` 类有两个属性：

+ `_NotificationNode? parent`
+ `NotifiableElementMixin? current`
    
&emsp;然后配合 Element Tree 构建过程中对它的使用，可看出 `_NotificationNode` 节点是组成了一个单向链表，而且是一个反向的单向链表，每个链表节点只有指向自己父节点的指针。所以看到这里能明白了吧：在 Element tree 构建过程中：当遇到 NotifiableElementMixin Element 时，就创建一个 notification tree 节点，如果是非 NotifiableElementMixin Element 则只是传递一下 notification tree 节点的引用。

&emsp;可以看出 notification tree 作为一个链表，仅是引用 Element Tree 中的 NotifiableElementMixin Element，这样的话，当有通知发出时，就可以减小查找到符合通知类型的链表节点的时间复杂度。是一种很好的优化！ 

&emsp;OK，注释看到这里，下面我们看一下参考类：NotificationListener Widget 的使用。

&emsp;NotificationListener 是一个允许监听通知的 Widget，不妨我们先看一个 NotificationListener Widget 极简单的示例代码，你就明白这：notification tree、允许监听通知 之类的名词了。

```dart
import 'package:flutter/material.dart';

void main() { runApp(const MyApp()); }

class CustomNotification extends Notification {
  final String message;
  CustomNotification(this.message);
}

class OtherNotification extends Notification {
  final String message;
  OtherNotification(this.message);
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: const Text('NotificationListener Example')),
        body: NotificationListener<CustomNotification>(
          onNotification: (notification) {
            print('收到 custom notification 2 号: ${notification.message}');
            return false;
          },
          child: NotificationListener<CustomNotification>(
            onNotification: (notification) {
              print('收到 custom notification 1 号: ${notification.message}');
              return false;
            },
            child: Builder(
              builder: (BuildContext context) {
                return ElevatedButton(
                  onPressed: () { 
                    // 这里可以关注一下，上面的 NotificationListener 只接收 CustomNotification 类型的通知，
                    // 如果我们发送的是 OtherNotification 通知的话，是与它们无关的。
                    CustomNotification('Hello').dispatch(context);
                    // OtherNotification('world').dispatch(context);
                  },
                  child: const Text('Send Custom Notification'),
                );
              },
            ),
          ),
        ),
      ),
    );
  }
}
```

&emsp;如实例代码，只要我们点击 'Send Custom Notification' 按钮，两个父级 NotificationListener Widget 就可以收到通知了，没错，此通知是沿着 Element tree 向上传递的。也正如 NotificationListener 注释所说它是一个可以监听从底层往上冒泡的通知的 Widget。

&emsp;Ok，我们下面开始看代码，开始之前呢，我们先顺着 “发起通知-传递通知-接收通知（处理通知）-继续传递” 这个流程捋一下这个通知的逻辑。

&emsp;1️⃣：发起通知（通知的起点）：首先我们想要发送什么通知我们就需要自定义一个继承自 `Notification` 的类（例如上面示例中的：CustomNotification 和 OtherNotification）。`Notification` 类是系统为我们提供的一个抽象类：`abstract class Notification`，它呢最重要的就是为其子类提供了一个超简单的 `dispatch` 函数，如示例代码中，我们通过：`OtherNotification('world').dispatch(context)`，发起了通知，实际呢就被转换为：`context.dispatchNotification(OtherNotification('world'))`，OK，这样，我们的 OtherNotification 通知就发出去了，继续向下看。

&emsp;2️⃣：传递通知：BuildContext context 承接着我们发出的通知，这里用到了它的：`dispatchNotification` 抽象函数，而我们的 Element 实现了这个抽象函数，仅有一行代码：`_notificationTree?.dispatchNotification(notification)`，即：Element 对象直接把这个事情交给了自己的 `_notificationTree` 属性。

&emsp;`_notificationTree` 属性是一个 `_NotificationNode` 类，`_NotificationNode` 类仅有两个属性和一个函数：

    + `_NotificationNode? parent` 属性，
    + `NotifiableElementMixin? current` 属性，
    + `dispatchNotification` 函数，
    
&emsp;然后配合 Element Tree 构建过程中对它的使用，`_NotificationNode` 节点组成了一个单向链表，而且是一个反向的链表，即每个链表节点只有指向其父节点的指针。而这恰恰符合了 NotificationListener 的传递方向。然后是 `current` 属性，正是当前的 Element 对象。即：`_NotificationNode` 是一个反向单向的链表，链表的每个节点就是引用这当前这个 Element 对象。正如：NotifiableElementMixin 中重写的 attachNotificationTree 所示，当 Element tree 构建过程中，每遇到一个 NotifiableElementMixin 节点，就会为 `_NotificationNode` 反向单向的链表添加一个链表节点。

&emsp;继续看 `_NotificationNode` 类，它的 `dispatchNotification` 函数内部就是执行当前这个链表节点的 element 对象的 `onNotification` 函数，并且由它的返回值决定是否继续往上传，如果继续的话就沿着它的：`_NotificationNode? parent` 指针向上递归调用 parent 的 `dispatchNotification` 函数。

&emsp;3️⃣：接收通知（处理通知）：当沿着 `_notificationTree` 链执行每个链表节点的 element 的 onNotification 函数时，如果遇到了匹配类型的 `Notification` 的话，就会执行通知，例如：`class _NotificationElement<T extends Notification>` Element 的 `onNotification` 函数内部，如果自己的 Widget 有 onNotification 回调并且传来的通知 `Notification` 类型是自己的泛型 T 的话，那么就会执行 Widget 的 onNotification 回调。如我们示例代码中的：`onNotification: (notification) { return false;}`，正是执行此回调。

&emsp;4️⃣：继续传递：默认情况下 `class _NotificationElement<T extends Notification>` Element 的 `onNotification` 在遇到不是自己 T 类型的 Notification 时返回 false，表示告诉 `_NotificationNode` 链表，此通知我不接收（或者我不处理这种通知），你可以继续向上传递了。如果遇到自己 T 类型的 Notification 时，可以由外部编写 NotificationListener Widget 的开发者执行自己的 onNotification 回调，自己决定返回 false 还是 true。如果返回 true 的话，就表示这个通知只有我自己处理，不要往上传了。返回 false 的话表示还可以继续向上传。例如上面我们实例代码中，如果 1 号位我们返回 true，那么 2 号位就不会执行了，如果 返回 false 的话，1 号 和 2 号都会执行。 

&emsp;OK，通知的流程就这么些，下面我们看源码。我们先不直接看 NotificationListener widget 的内容，我们看 NotificationListener widget 对应的 `class _NotificationElement<T extends Notification>` Element，它混入了 NotifiableElementMixin。我们看先从 NotifiableElementMixin 的内容开始看。

# NotifiableElementMixin

&emsp;NotifiableElementMixin 这个类允许接收由子级 Element 对象发出的 Notification 对象。如 `_NotificationElement` Element 就是混入了 NotifiableElementMixin：`class _NotificationElement<T extends Notification> extends ProxyElement with NotifiableElementMixin`，由此具备了接收 Notification 通知的能力。

&emsp;注意这里需要是子级，如我们实例代码中：`CustomNotification('Hello').dispatch(context)` 发出通知时，用的这个 context 一定要时子级的，如 InheritedWidget 的 of 函数调用时传入的 context 一定要是要找的 InheritedElement 的子级 Element，否则都是找不到的。

```dart
mixin NotifiableElementMixin on Element { //... }
```

## onNotification

&emsp;当 Element tree 中的此位置接收到适当类型的通知时调用。返回 true 以取消通知冒泡。返回 false 以允许通知继续分发到更多的祖先节点（在 notification tree 中继续向上传递）。

&emsp;这是一个抽象函数，所有的 NotifiableElementMixin 的子类都要自己实现。如 `class _NotificationElement<T extends Notification> extends ProxyElement with NotifiableElementMixin` 中对此函数的重写：如果 `Notification notification` 类型是自己的泛型 T 的话就会进行处理，否则话直接返回 fale，让通知继续传递。

&emsp;注意 

```dart
  bool onNotification(Notification notification);
```

## attachNotificationTree

&emsp;构建 notification tree 的一个节点，前面详细解析过了，这里不再展开。只有 NotifiableElementMixin 对 attachNotificationTree 进行了重写，其它类都是直接使用 Element.attachNotificationTree。

```dart
  @override
  void attachNotificationTree() {
    _notificationTree = _NotificationNode(_parent?._notificationTree, this);
  }
```

&emsp;下面看一下 `_NotificationNode` 类的内容。

# `_NotificationNode`

&emsp;`_NotificationNode` 中 current 属性指向当前的 NotifiableElementMixin 节点，parent 指向自己的父级节点。

```dart
class _NotificationNode {
  _NotificationNode(this.parent, this.current);

  NotifiableElementMixin? current;
  _NotificationNode? parent;
  
  // ...
}
``` 

## dispatchNotification

&emsp;dispatchNotification 函数中如果当前 NotifiableElementMixin 节点处理通知后返回 true 的话，则通知传递就到此为止了，如果返回 false 的话就继续调用 parent?.dispatchNotification(notification)，继续向上传递。

```dart
  void dispatchNotification(Notification notification) {
    if (current?.onNotification(notification) ?? true) {
      return;
    }
    
    // 向上级节点继续传递
    parent?.dispatchNotification(notification);
  }
```

&emsp;然后接下来我们看一下系统为我们提供的发送通知时使用的抽象基类：`abstract class Notification`。

# abstract class Notification

&emsp;一种可以向上冒泡至 widget tree 的通知。你可以使用 is 运算符通过检查通知的 runtimeType 来确定通知的类型。

&emsp;要在子树中监听通知，请使用 NotificationListener。

&emsp;要发送通知，请调用要发送的通知上的 dispatch 方法。通知将传递给具有相应类型参数的任何祖先 BuildContext 下的 NotificationListener Widget。

```dart
abstract class Notification {
  // 抽象常量构造函数。该构造函数使得子类能够提供常量构造函数，从而可以在常量表达式中使用它们。
  const Notification();
  // ...

}
```

## dispatch

&emsp;在给定的 build context 开始冒泡此通知。通知将传递给具有适当类型参数的任何祖先的 NotificationListener Widget，而给定的 BuildContext。如果 BuildContext 为 null，则不会分发该通知。

&emsp;dispatchNotification 是 BuildContext 的一个抽象函数，已由 Element 实现。

```dart
  void dispatch(BuildContext? target) {
    // 直接调用 BuildContext 的 dispatchNotification 函数
    target?.dispatchNotification(this);
  }
```

&emsp;dispatchNotification 由 Element 进行了实现，即直接交给 `_notificationTree` notification tree 处理。

```dart
  @override
  void dispatchNotification(Notification notification) {
    _notificationTree?.dispatchNotification(notification);
  }
```

&emsp;至此呢，跟通知相关的：创建通知、发送通知 都看完了，下面就是交由我们的 NotificationListener Widget 处理通知了，下面看一下它的内容。

# NotificationListener

&emsp;监听在 notification tree 上传递的通知的 Widget，而且它是 ProxyWidget 的子类。只有在其 runtimeType 是 T 的子类型时，通知才会触发 onNotification 回调。要分发通知，可使用 Notification.dispatch 方法。

+ Object -> DiagnosticableTree -> Widget -> ProxyWidget -> NotificationListener

&emsp;既然是 ProxyWidget 的子类，UI 部分则交给 child 实现，主要关注 onNotification 处理通知部分。 

```dart
class NotificationListener<T extends Notification> extends ProxyWidget {
  // 创建一个监听通知的 widget
  const NotificationListener({
    super.key,
    required super.child,
    this.onNotification,
  });
  
  //...
  
}
```

## onNotification

&emsp;NotificationListenerCallback Notification 监听器的签名。

&emsp;返回 true 可以取消通知冒泡。返回 false 可以允许通知继续被分发到更进一步的祖先。

&emsp;当监听 ListView、NestedScrollView、GridView 或任何滚动小部件中的滚动事件时，NotificationListener 是很有用的。由 NotificationListener.onNotification 使用。

```dart
typedef NotificationListenerCallback<T extends Notification> = bool Function(T notification);
```

&emsp;当 notification tree 中的此位置收到适当类型（T 类型以及子类）的通知时调用。

&emsp;返回 true 以取消通知冒泡。返回 false 可以允许通知继续被分发到更进一步的祖先。

&emsp;通知有不同的派发时机。主要有两种可能性：在帧之间派发 和 在布局期间派发。

&emsp;对于在布局期间派发的通知，例如从 LayoutChangedNotification 继承的通知，响应通知调用 State.setState 太晚了（由于通知向上冒泡，因此定义上布局当前正在后代中进行）。对于依赖布局的小部件，请考虑使用 LayoutBuilder。

```dart
  final NotificationListenerCallback<T>? onNotification;
```

## createElement

&emsp;NotificationListener 的 Element 混入了 NotifiableElementMixin，由此才能接收通知，下面详细看一下。 

```dart
  @override
  Element createElement() {
    return _NotificationElement<T>(this);
  }
```

# `_NotificationElement`

&emsp;同样是直接继承自 ProxyElement 并混入了 NotifiableElementMixin。

```dart
class _NotificationElement<T extends Notification> extends ProxyElement with NotifiableElementMixin {
  _NotificationElement(NotificationListener<T> super.widget);
  // ...
}
```

&emsp;主要聚焦在它的 onNotification 函数中：

## onNotification

&emsp;onNotification 函数内部也很清晰，取得它的 Widget，如果 Widget 的 onNotification 不为 null，并且传递来的 Notification 是自己的泛型 T 的话，则执行 Widget 的 onNotification 回调，否则的默认返回 false 表示通知未得到处理，让通知继续在 notification tree 中传递。 

```dart
  @override
  bool onNotification(Notification notification) {
    final NotificationListener<T> listener = widget as NotificationListener<T>;
    
    if (listener.onNotification != null && notification is T) {
      return listener.onNotification!(notification);
    }
    
    return false;
  }
```

&emsp;至此 NotificationListener 和 `_notificationTree` 的内容就看完啦，下面我们总结一下。

# `_notificationTree` 总结

&emsp;`_notificationTree` => notification tree，继 widget tree、element tree 之后的又一颗树，一颗比较短的树🌲，例如上面我们的超级简单的示例中，我们在按钮点击事件处打一个断点，看看目前所有 element 节点共享的 notification tree 到底有多高，数了一下，发现除了我们自己的两个 notification node（`NotificationListener<CustomNotification>`）节点之外，还有另外 5 个节点，也就是说除了这 5 个之外，后续我们自己添加几个才会有几个：

5. `NotificationListener<LayoutChangedNotification>`
4. `NotificationListener<ScrollNotification>`
3. `NotificationListener<ScrollMetricsNotification>`
2. `NotificationListener<NavigationNotification>`
1. `NotificationListener<NavigationNotification>`

&emsp;Flutter framework 默认为我们的 notification tree 添加了 5 个节点，所以我们的示例总共也才 7 个节点，相比于我们断点处 depth 已经是 151 的 element 节点深度相比已经是很浅了，所以即使通知发出后，沿着 notification tree 一个节点一个节点的传递，开销还是比较小的，再加上我们自己可以控制通知是否继续向上传递，所以总体下来开销还可以更小。对于开始看之前还以为是沿着 element tree 一个节点一个节点的传递，那开销确实有点大。

&emsp;notification tree 很巧妙的构建方式👍。

&emsp;那么为了回头复习方便，我们简要总结下。

1️⃣：notification tree 是如何构建的？

&emsp;在 Element Tree 一层一层的构建过程中 Element.mount 和 Element.activate 函数内会调用 attachNotificationTree 函数，它们内部仅是传递 `_notificationTree` 引用（`_notificationTree = _parent?._notificationTree`），然后当遇到 NotifiableElementMixin Element 节点时，由于它重写了 attachNotificationTree 函数，它的内部会真正的为 notification tree 创建一个节点：`_notificationTree = _NotificationNode(_parent?._notificationTree, this)`，所以整个 Element tree 构建完成，notification tree 也构建完成了，并且有多少 NotifiableElementMixin Element 节点，就有多少个 notification tree 节点，然后其它非 NotifiableElementMixin Element 节点的 `_notificationTree` 则☝️指向离自己最近的一个 `_NotificationNode`。

2️⃣：notification tree 的结构是什么样的？

&emsp;首先不同于 Element 节点的树状结构（如 SingleChildRenderObjectElement 有一个指向子级的指针 child，MultiChildRenderObjectElement 有一组指向子级的指针 children），由于 notification tree 中每个节点都是使用 `_NotificationNode` 类构造，`_NotificationNode` 类仅有一个指向父级的 `_NotificationNode? parent` 指针和一个指向当前 `NotifiableElementMixin? current` 的指针，所以 notification tree 形态上更像是一个单向的反向链表。当有某种类型的通知发出时，正是沿着这个反向的链表，向上查找对应可以监听这种通知类型的 NotifiableElementMixin。

# NotificationListener 总结

&emsp;上面已经总结了 notification tree 的内容，到这里大概只剩下：Notification、NotifiableElementMixin、dispatchNotification 了。

&emsp;首先呢 Flutter framework 为我们提供了一个通知的抽象基类：`Notification`，当我们想要发何种类型通知时，我们就可以继承这个 Notification 类封装自己的通知类型，如添加一些字段存储我们想要向上👆通知的内容。然后直接调用它的 dispatch 函数，例如：`CustomNotification('Hello').dispatch(context)`，这样一个 CustomNotification 类型的通知就发送出去了，其实内部是调用的 BuildContext 的发送通知的接口 BuildContext.dispatchNotification。

&emsp;然后呢 BuildContext 定义了一个发送通知的接口：dispatchNotification，Element 基类实现了它，仅有一行：`_notificationTree?.dispatchNotification(notification)`，即直接交给自己的 `_notificationTree` 属性调度通知，这样 notification tree 就会从当前开始一层一层让 `_NotificationNode` 节点处理此通知，`_NotificationNode` 节点把通知交给自己的 `NotifiableElementMixin? current` 处理，默认情况下 `NotifiableElementMixin? current` 只处理自己泛型 T 的通知，如果不是自己泛型 T 的通知的话就默认返回 false 表示自己不处理这种通知，此时 `_NotificationNode` 节点就会向上传给自己的 parent 继续处理。即使 `NotifiableElementMixin? current` 接收到了自己泛型 T 的通知，那么它也可以在自己处理完毕后决定返回 true 或 false 决定让不让此通知继续沿着 notification tree 向上传播。

&emsp;NotificationListener widget 的话，它自己对应的 Element（`_NotificationElement`）混入了 NotifiableElementMixin，然后实现了它的 onNotification 抽象函数。当通知到来时，`_NotificationElement` 的 onNotification 会首先得到执行，它会判断 NotificationListener widget 是否有 onNotification 回调并且判断通知类型是否是自己的泛型 T，如果两个条件都满足的话，它就会执行 NotificationListener widget 的 onNotification 回调，如果否的话，就会返回 false 表示未处理这个通知，让通知得以继续向上传播。当然如果 NotificationListener widget 的 onNotification 回调得以执行的话，它也会返回一个 bool 值来决定是否让通知继续向上传播📣。

&emsp;OK，`_notificationTree` 就到这里吧，下篇我们开始 Element 类。

## 参考链接
**参考链接:🔗**
+ [NotifiableElementMixin mixin](https://api.flutter.dev/flutter/widgets/NotifiableElementMixin-mixin.html)
+ [`NotificationListener<T extends Notification> class`](https://api.flutter.dev/flutter/widgets/NotificationListener-class.html)
+ [NotificationListener (Flutter Widget of the Week)](https://www.youtube.com/watch?v=cAnFbFoGM50)
+ [Notification class](https://api.flutter.dev/flutter/widgets/Notification-class.html)
+ [LayoutChangedNotification class](https://api.flutter.dev/flutter/widgets/LayoutChangedNotification-class.html)
