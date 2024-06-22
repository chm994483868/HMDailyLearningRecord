# ProxyWidget/InheritedWidget/ParentDataWidget

&emsp;ProxyWidget 通过接收一个 child widget 为参数构建内容，而不是自身构建一个新的 widget，它没有像 StatelessWidget 一样的 build 函数来构建新 widget。

&emsp;ProxyWidget 通常被用作其他 widget 的基类，比如 InheritedWidget 和 ParentDataWidget。这样的设计可以让我们更方便地实现一些复杂的 widget，让它们能够更好地与 Flutter 的布局系统和数据管理系统进行交互。（直白一点理解的话，仅仅是面向对象中的继承思想的使用，把同样的特征提取出来作为一个基类使用，而在这里 “同样的特征” 就是指：InheritedWidget 和 ParentDataWidget 都有一个 final Widget child 字段。）

&emsp;另外还可以参阅（下面是官方给其它 widget 下的定义，感觉蛮贴切的！）：

+ InheritedWidget，用于引入环境状态（ambient state）的 widget，可以被子代 widget 所读取。
+ ParentDataWidget，用于填充其子 widget 的 RenderObject.parentData 插槽的 widget，从而配置父 widget 的布局。
+ StatefulWidget和State，用于在其生命周期内可以多次构建不同内容的 widget。
+ StatelessWidget，用于在给定特定配置和 BuildContext 时总是以相同方式构建的 widget。
+ Widget，用于概述一般的 widget。

&emsp;下面看 ProxyWidget 的源码。

## ProxyWidget

&emsp;ProxyWidget 依然是直接继承自 Widget 的抽象基类，它添加了一个属于 ProxyWidget 子类的新字段：final Widget child，初始化 ProxyWidget 子类时必须传入 child 参数。

```dart
abstract class ProxyWidget extends Widget {
  /// Creates a widget that has exactly one child widget.
  /// 创建只有一个 child widget 的 widget。
  const ProxyWidget({ super.key, required this.child });
  // ...
}  
```

### child

&emsp;ProxyWidget 即是 Widget 的子类，那必然每个字段都要添加 final 修饰。

&emsp;child 是树中位于此 widget 下方的 widget。此 widget 只能有一个子 widget。要布置多个子 widget，让此 widget 的子 widget 是 Row、Column 或 Stack 等拥有 children 属性的 widget，并将子 widget 提供给该 widget。

```dart
final Widget child;
```

### 总结 ProxyWidget 

&emsp;ProxyWidget 内容没啥看的，仅仅是直接继承自 Widget，然后添加一个新字段 child。而它最重要的作用仅是为了给：InheritedWidget 和 ParentDataWidget 两个超级重要的 widget 作基类使用。而这 ProxyWidget 仅仅是为了面向对象中继承基类的思想来的，哪怕直接让 InheritedWidget 和 ParentDataWidget 继承自 Widget，然后 InheritedWidget 和 ParentDataWidget 各自分别添加一个 final Widget child 也是可以的。

&emsp;下面看 ProxyWidget 的子类 InheritedWidget。

## InheritedWidget

&emsp;InheritedWidget 是用于高效传播信息到树中 widget 的基类。InheritedWidget 提供了一种方式，可以在不同的 widget 之间有效地传递数据和信息。这种传递是自上而下的，即数据从父级 widget 传递到子级 widget，使得整个应用程序的信息传递更加高效和方便。

&emsp;[InheritedWidgets | Decoding Flutter](https://www.youtube.com/watch?v=og-vJqLzg2c)超级重要，务必观看，涉及在何时何处使用 dependOnInheritedWidgetOfExactType，以及 InheritedWidget 是如何让其依赖者进行重建的！

&emsp;要从 build context 中获取特定类型的最近继承的 widget 的实例，请使用 BuildContext.dependOnInheritedWidgetOfExactType。当以这种方式引用时，Inherited Widget 会在其自身状态改变时促使 consumer rebuild，以保证它们的数据同步。

&emsp;[A guide to Inherited Widgets - Flutter Widgets 101 Ep. 3](https://www.youtube.com/watch?v=Zbm3hjPjQMk)

&emsp;下面是一个名为 FrogColor 的继承自 InheritedWidget 的 widget：

```dart
class FrogColor extends InheritedWidget {
  const FrogColor({
    super.key,
    required this.color,
    required super.child,
  });

  final Color color;

  static FrogColor? maybeOf(BuildContext context) {
    return context.dependOnInheritedWidgetOfExactType<FrogColor>();
  }

  static FrogColor of(BuildContext context) {
    final FrogColor? result = maybeOf(context);
    assert(result != null, 'No FrogColor found in context');
    return result!;
  }

  @override
  bool updateShouldNotify(FrogColor oldWidget) => color != oldWidget.color;
}
```

### 实现 of 和 maybeOf 方法

&emsp;通常情况是在 InheritedWidget 上提供两个静态方法 of 和 maybeOf，这两个方法调用 BuildContext.dependOnInheritedWidgetOfExactType。这允许该类定义自己的后备逻辑，以防范围内没指定类型的 Widget。

&emsp;of 方法通常返回一个非空实例，并在未找到 InheritedWidget 时断言，maybeOf 方法返回一个可空实例，在未找到 InheritedWidget 时返回 null。通常情况下，of 方法通过在内部调用 maybeOf 实现。

&emsp;有时，of 和 maybeOf 方法返回的是一些数据，而不是 inherited widget 本身；例如，在这种情况下，它可以返回一种颜色而不是 FrogColor 小部件。

&emsp;偶尔，inherited widget 是另一个类的实现细节，因此是私有的。这种情况下，of 和 maybeOf 方法通常在公共类上实现。例如，Theme 被实现为一个构建私有的 inherited widget 的 StatelessWidget（class Theme extends StatelessWidget { // ... }），Theme.of 使用 BuildContext.dependOnInheritedWidgetOfExactType 寻找该私有 inherited widget，然后返回其中的 ThemeData。

&emsp;我们经常看到的 Theme 是一个 StatelessWidget 的子类，但我们经常使用它的 of 函数：Theme.of(context)，而 Theme 的静态 of 函数内部其实是在 context 中查找一个私有的 inherited widget: `_InheritedTheme`。

```dart
Text('Example', style: Theme.of(context).textTheme.titleLarge,)

class Theme extends StatelessWidget {

// ...
static ThemeData of(BuildContext context) {
  final _InheritedTheme? inheritedTheme = context.dependOnInheritedWidgetOfExactType<_InheritedTheme>();
    ...
    
}

// ...

}
```

### 调用 of 或 maybeOf 方法

&emsp;当使用 of 或 maybeOf 方法时，context 必须是 InheritedWidget 的子孙（descendant），也就是说，context 必须是在 InheritedWidget 在树中的位置是 "below"。意思就是，在 context 必须是嵌套在 InheritedWidget 内部的。例如下面这个例子，使用的 context 是来自 Builder 的 context，而 Builder 是 FrogColor widget 的子 widget，所以这样是可以的。 

```dart
// continuing from previous example...
class MyPage extends StatelessWidget {
  const MyPage({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: FrogColor(
        color: Colors.green,
        child: Builder(
          builder: (BuildContext innerContext) {
            return Text(
              'Hello Frog',
              style: TextStyle(color: FrogColor.of(innerContext).color),
            );
          },
        ),
      ),
    );
  }
}
```

&emsp;下面看一个反例。在下面这个例子中，使用的 context 是来自 MyOtherPage widget，它是 FrogColor widget 的父级，所以这样不起作用，而且当调用 FrogColor.of 时会触发断言。这样之所以不行，是因为当我们调用 context 的 dependOnInheritedWidgetOfExactType 时，它是以 context 自身为基点，而示例代码中，是以 MyOtherPage 为基点，显然是不能找到位于 MyOtherPage 内部的 FrogColor 的。  

```dart
// continuing from previous example...

class MyOtherPage extends StatelessWidget {
  const MyOtherPage({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: FrogColor(
        color: Colors.green,
        child: Text(
          'Hello Frog',
          style: TextStyle(color: FrogColor.of(context).color),
        ),
      ),
    );
  }
}
```

&emsp;[InheritedWidget (Flutter Widget of the Week)](https://www.youtube.com/watch?v=1t-8rBCGBYw)

&emsp;另外可参考：

+ InheritedNotifier，一个 InheritedWidget 子类，其值可以是 Listenable，并且当值发送通知时，会通知依赖项。
+ InheritedModel，一个 InheritedWidget 子类，允许客户端订阅值的子 widget 的更改。

&emsp;好了，InheritedWidget 相关的文档都看完了，下面我们开始看 InheritedWidget 的源码。

### createElement

&emsp;InheritedWidget 是一个直接继承自 ProxyWidget 的抽象 Widget，它从 ProxyWidget 唯一继承来的就是 child 字段，其它的话则和 StatelessWidget、StatefulWidget 一模一样的继承自 Widget 这个基类。

&emsp;InheritedWidget 同样重写了 Widget 的 createElement 函数，直接返回 InheritedElement，再次印证了：Widget 和 Element 是一一对应的。InheritedElement 我们后面再看。

```dart
abstract class InheritedWidget extends ProxyWidget {
  /// Abstract const constructor. This constructor enables subclasses to provide
  /// const constructors so that they can be used in const expressions.
  // 抽象常量构造函数。该构造函数使得子类能够提供常量构造函数，从而可以在常量表达式中使用。
  const InheritedWidget({ super.key, required super.child });

  @override
  InheritedElement createElement() => InheritedElement(this);
  
  // ...
}
```

&emsp;下面我们看 InheritedWidget 最重要的一个抽象函数。

### updateShouldNotify

&emsp;首先 updateShouldNotify 函数是一个抽象函数，即我们自己写的所有继承自 InheritedWidget 的 Widget 必须实现这个函数。

&emsp;当 framework 是否应该通知从此 widget 继承的 widget 时。当此 widget 被重建时，有时我们需要重建继承自此 widget 的 widget，但有时则不需要。例如，如果此 widget 持有的数据与 oldWidget 持有的数据相同，则不需要重建继承自 oldWidget 的 widget。

&emsp;framework 通过使用先前在树中占据此位置的旧 widget 作为参数来区分这些情况。给定的 widget 保证与此对象具有相同的运行时类型。

&emsp;解释过于官方了，我们可以直白理解一下，就是当 InheritedWidget 被重建时，是否应通知那些通过 dependOnInheritedWidgetOfExactType 与此 InheritedWidget 建立了依赖关系的 Widget 也进行更新重建。

```dart
  @protected
  bool updateShouldNotify(covariant InheritedWidget oldWidget);
```

### convariant

&emsp;在 Dart 中，如果重写父类方法，则重写方法的参数必须与父类原始方法具有相同的类型，然而这里的 updateShouldNotify 函数被 InheritedWidget 子类重写后，oldWidget 参数的类型就变了，而 convariant 关键字，就是用来帮助抹平 oldWidget 参数类型的，加了 convariant 关键字就是表示告诉编译器，这里的 oldWidget 参数类型只要是 InheritedWidget 或者其子类都可以。 

&emsp;例如上面示例代码中继承自 InheritedWidget 的 FrogColor，重写 updateShouldNotify 函数时 oldWidget 参数的类型是 FrogColor widget。

&emsp;关于 covariant 关键字的具体细节可以看这个链接：[functioning of covariant in flutter](https://stackoverflow.com/questions/71237639/functioning-of-covariant-in-flutter)。

### 总结 InheritedWidget

&emsp;InheritedWidget 就看到这里，总结一下。

&emsp;可以看出 InheritedWidget 是一个特别重要的 Widget，首先它给我们带来了超方便的跨 widget 传值能力，让我们可以随意的在底部去读取父部 widget 的数据，同时还带来了 “依赖更新”，当父部 widget 被重建数据更新时，底部的 widget 也能及时得到更新保证数据同步。

&emsp;方便使用 InheritedWidget 能力的同时我们还有一些注意事项。

&emsp;首先是数据的读取，我们要保证静态 of(context) 函数内调用 dependOnInheritedWidgetOfExactType 函数的 context 是我们要查找的 InheritedWidget 的子级部分。

&emsp;其次是 dependOnInheritedWidgetOfExactType 的调用成本，也就是我们使用 of(context) 函数读取 InheritedWidget 数据的成本，其实是特别特别低！仅仅只是一下 （`PersistentHashMap<Type, InheritedElement>? _inheritedElements;`) hashMap 的读取操作而已，O(1) 的复杂度，能做到如此低的成本是因为 `_inheritedElements` 作为一个 Element 的字段是在 Element tree 中直接赋值向下一级一级传递的，当需要时直接读取即可。

&emsp;`InheritedWidget` 作为一个以 InheritedWidget 子类型为 Key 的 hashMap，也可以发现如果是同类型的 InheritedWidget 子类嵌套的话，当在内部的 InheritedWidget 的子级 widget 调用 of(context) 函数的话，只能向上读取到最接近读取位置的 InheritedWidget 数据。

&emsp;再下面就是依赖更新相关的了，of(context) 函数是读取，也是依赖注册，当我们调用 of(context) 函数时，其实也是对当前的 widget(element) 向读取的 InheritedWidget 建立了依赖关系，如果 InheritedWidget 更新重建的话，依赖它的子级 widget 也是会得到更新重建的(其实这里做的是把 context 添加到 InheritedElement 的： `final Map<Element, Object?> _dependents = HashMap<Element, Object?>();` 字段中，当 InheritedWidget 更新重建时，把依赖它的那些 element 也标记为重建。)。 

&emsp;其次是调用 of 函数的位置，从上面我们已知 of 函数读取的成本很低，我们可以随意调用。但是调用位置的话 framework 做了一些限制。（之所以有这些限制，是因为我们要等上面提到的：`_inheritedElements` 被赋好值以后，其实是等 element 节点挂载好以后。）

&emsp;那么在哪里调用 of 函数比较好呢？官方教程也给了几个位置：build/didChangeDependencies/didUpdateWidget。(这里提一下：didChangeDependencies 函数，它的 Dependencies 部分是谁呢？其实就是我们调用 dependOnInheritedWidgetOfExactType 时创建的对 InheritedWidget 的依赖关系，而这其实也引出了 State 的 didChangeDependencies 的调用时机，后面我们再细看，这里先有一个印象即可。)

&emsp;那么在 build/didChangeDependencies/didUpdateWidget 调用 of 函数有什么区别呢？其实主要是看：在 of 函数数据返回后，如果要对返回的数据进行 “大量的计算” 的话，那么建议在 State 的 didChangeDependencies 中调用，然后就可以在 build 中直接使用 “大量的计算” 的结果。如果直接放在 build 函数中进行 of 调用和 “大量的计算” 的话，如果 InheritedWidget 数据没有发生变化的话，我们在 build 中频繁进行 of 调用和 “大量的计算” 的话是的得不偿失的，毕竟 build 在很多情况下都会被重复调用，所以让 didChangeDependencies 专职做 of 调用比较好。

&emsp;OK，总结的可以了，InheritedWidget 先看到这里，我们继续下一个 ParentDataWidget。

## ParentDataWidget

&emsp;ParentDataWidget 是将 ParentData 信息与 RenderObjectWidget 的子 widget 关联的 widget 的基类。

&emsp;这可以用于为具有多个子 widget 的 RenderObjectWidget 提供每个子 widget 的配置。例如，Stack 使用 Positioned 父数据 widget 来定位每个子 widget。

&emsp;ParentDataWidget 针对特定类型的 ParentData。该 T 需要是 ParentData 的子类。

&emsp;如下示例展示了如何构建一个 ParentDataWidget 来配置 FrogJar widget 的子 widget，通过为每一个子 widget 指定一个 Size。

```dart
class FrogSize extends ParentDataWidget<FrogJarParentData> {
  const FrogSize({
    super.key,
    required this.size,
    required super.child,
  });

  final Size size;

  @override
  void applyParentData(RenderObject renderObject) {
    final FrogJarParentData parentData = renderObject.parentData! as FrogJarParentData;
    if (parentData.size != size) {
      parentData.size = size;
      final RenderFrogJar targetParent = renderObject.parent! as RenderFrogJar;
      targetParent.markNeedsLayout();
    }
  }

  @override
  Type get debugTypicalAncestorWidgetClass => FrogJar;
}
```

&emsp;另请参阅：

+ RenderObject，布局算法的父类。
+ RenderObject.parentData，此类配置的 slot。
+ ParentData，将放置在 RenderObject.parentData slot 中的数据的父类。ParentDataWidget 的 T 类型参数是 ParentData 的子类。
+ RenderObjectWidget，包装 RenderObject 的 widget 类。
+ StatefulWidget 和 State，适用于其生命周期内多次构建不同内容的 widget。

&emsp;可看到 ParentDataWidget 最重要的就是有一个抽象函数：applyParentData，需要所有的子类自己去实现它。下面直接看 ParentDataWidget 的源码。

### createElement

&emsp;ParentDataWidget 是一个直接继承自 ProxyWidget 的抽象泛型类，泛型参数 T 必须是 ParentData 的子类。

&emsp;抽象 const 构造函数和 InheritedWidget 一样，必须传入 child 参数。

&emsp;然后是重写 Widget 的 createElement 函数，直接返回 ParentDataElement。

```dart
abstract class ParentDataWidget<T extends ParentData> extends ProxyWidget {
  /// Abstract const constructor. This constructor enables subclasses to provide
  /// const constructors so that they can be used in const expressions.
  // 抽象常量构造函数。此构造函数使得子类能够提供常量构造函数，以便它们可以在常量表达式中使用。
  const ParentDataWidget({ super.key, required super.child });

  @override
  ParentDataElement<T> createElement() => ParentDataElement<T>(this);
  // ...
}
```

### debugIsValidRenderObject

&emsp;检查此 widget 是否可以将其 parent data 应用于提供的 renderObject。

&emsp;通常情况下，提供的 renderObject 的 RenderObject.parentData 是由返回 debugTypicalAncestorWidgetClass 的类型的祖先 RenderObjectWidget 设置的。

&emsp;在调用 applyParentData 方法之前，会调用此方法并使用相同的 RenderObject 提供给该方法。

&emsp;一个 dubug 函数，判断 renderObject.parentData 的类型是不是 T 类型。

```dart
  bool debugIsValidRenderObject(RenderObject renderObject) {
    assert(T != dynamic);
    assert(T != ParentData);
    return renderObject.parentData is T;
  }
```

### debugTypicalAncestorWidgetClass

&emsp;一个需要子类重写的 get。

&emsp;描述通常用于设置 applyParentData 将要写入的 ParentData 的 RenderObjectWidget。

&emsp;这仅在错误消息中使用，告诉用户 "typical" 包裹这个 ParentDataWidget 的 widget 是什么，通过 debugTypicalAncestorWidgetDescription。

&emsp;返回的 Type 应该描述 RenderObjectWidget 的子类。如果支持多种 Type，可以使用 debugTypicalAncestorWidgetDescription，这通常会插入该数值，但也可以被重写以描述多种 Type。

&emsp;如下的一个 ParentDataWidget 子类实现：

```dart
  @override
  Type get debugTypicalAncestorWidgetClass => FrogJar;
```

&emsp;如果 "typical" 的父类是通用的（`Foo<T>`），考虑指定典型的类型参数（例如，如果 int 是类型通常如何特化的，就指定 `Foo<int>`），或者指定上界（例如，`Foo<Object?>`）。

```dart
  Type get debugTypicalAncestorWidgetClass;
```

### debugTypicalAncestorWidgetDescription

&emsp;描述了通常用于设置父数据（ParentData）的 RenderObjectWidget。

&emsp;此信息仅在错误消息中使用，告诉用户通常会包装该 ParentDataWidget 的 widget 是什么。

&emsp;默认返回 debugTypicalAncestorWidgetClass 作为 String。可以覆盖此方法以描述更多有效父类类型。

```dart
  String get debugTypicalAncestorWidgetDescription => '$debugTypicalAncestorWidgetClass';
```

### `_debugDescribeIncorrectParentDataType`

&emsp;debug 函数，当 parentData 为 null、parentDataCreator 为 null 等的一些，报错 log，自行看看即可。

&emsp;下面看 ParentDataWidget 最重要的一个抽象函数：applyParentData，需要子类各自去实现它。

### applyParentData

&emsp;一个 ParentDataDidget 子类分别要实现的抽象函数。 

&emsp;将这个 widget 中的数据写入给定 render object 的 parent data。即是把 ParentDataWidget 子类中的 数据，写入到 renderObject 中去。

&emsp;每当检测到与子级关联的 RenderObject 具有过时的 RenderObject.parentData 时，framework 都会调用此函数。例如，如果 render object 最近插入到 render tree 中，render object 的父数据可能与此 widget 中的数据不匹配。

&emsp;预期子类重写此函数，将数据从其字段复制到给定 render object 的 RenderObject.parentData 字段中。可以保证 render object 的父级是由类型为 T 的 widget 创建的，这通常意味着该函数可以假设 render object 的 parent data 对象继承自特定类。

&emsp;如果此函数修改可能改变父级 layout 或 painting 的数据，则应调用适当的 RenderObject.markNeedsLayout 或 RenderObject.markNeedsPaint 函数来为父级执行标记。

```dart
  @protected
  void applyParentData(RenderObject renderObject);
```

### debugCanApplyOutOfTurn

&emsp;这个 widget 是否允许使用 ParentDataElement.applyWidgetOutOfTurn 方法。

&emsp;只有当此 widget 代表的 ParentData 配置对 layout 或 panint 阶段没有影响时，才应返回 true。

&emsp;另请参阅：

&emsp;ParentDataElement.applyWidgetOutOfTurn，在调试模式下验证这一点。

```dart
  @protected
  bool debugCanApplyOutOfTurn() => false;
```

&emsp;关于这些 debug 函数，官方文档上也没有什么有用的信息，暂时略过。

### 总结 ParentDataWidget

&emsp;同 InheritedWidget 一样直接继承自 ProxyWidget 的抽象基类，官方文档介绍的内容并不多，等后续看它的子类时内容才比较多。

&emsp;首先记住它有一个抽象函数 applyParentData(应用 parent data) 即可。

&emsp;ParentData 只是一个特别小的类，只有一个 detach 函数，它作为 RenderObject 的 parent data 的基类使用。在 Flutter 中，一些 render object 希望在其子对象上存储一些数据，比如子对象对父对象布局算法的输入参数，或子对象相对于其他子对象的位置关系。RenderObject.setupParentData 方法，这是 RenderObject 子类可以重写的方法，用于为子对象附加特定类型的父对象数据。后面学习 RenderObjectWidget 时再看这部分内容。

## 参考链接
**参考链接:🔗**
+ [widgets library - ProxyWidget class](https://api.flutter.dev/flutter/widgets/ProxyWidget-class.html)
+ [ParentData class](https://api.flutter.dev/flutter/rendering/ParentData-class.html)
+ [ParentDataWidget<T extends ParentData> class](https://api.flutter.dev/flutter/widgets/ParentDataWidget-class.html)
