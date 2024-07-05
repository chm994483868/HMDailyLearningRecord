# Flutter 源码梳理系列（十二）：Element：At first sight

&emsp;下面开始学习整个 Flutter framework 最核心的一个类：Element。

&emsp;Element 可以看作 Widget Tree 中特定位置的 Widget 的实例化（只是对应的一个点也好，或者是此节点为根的整个子树也好）。

&emsp;任何一个 Element 节点的新建或重建，都不是当前这个一个 Element 节点的事情，它是整个以此 Element 节点为根的整个 Element 子树的新建或重建。一个 Widget 对象可以代表一颗 Element 子树的起点，一个 Element 节点也可以代表一颗 Element 子树的起点。在 Element Tree 构建过程中沿着根 Widget 一层一层的获得子级 Widget，每个 Widget 对象创建出对应的 Element 对象，然后把这些 Element 对象通过指针链接在一起，构成了整颗 Element Tree。后续的 Element 的整个学习流程，基本就是围绕这整个构建过程，拆解其中的细节进行学习。

&emsp;Widget 描述如何配置 Element，但同一个 widget 对象可以同时用于配置多个 Element 子树，因为 widget 对象是不可变的。一个 element 节点代表了在 Widget Tree 中的特定位置配置 widget 的使用。随着时间的推移，与给定 element 对象相关联的 widget 对象可能会发生变化，例如，如果父 widget 重建并为此位置创建了一个新的 widget 对象。

&emsp;Element 节点一起组成一棵 Element Tree。大多数 element 对象都有一个唯一的 child，但是一些 element（如 MultiChildRenderObjectElement 的子类）可以有多个 children（如：Row、Column Widget 等）。

&emsp;Element 的生命周期如下：

+ Flutter framework 通过调用 Widget.createElement 在将作为 element 对象初始配置的 widget 对象上来创建一个 element 对象。 (createElement 仅有的一次调用是在 Element 的 inflateWidget 中由 newWidget 调用：**final Element newChild = newWidget.createElement();**)

+ Flutter framework 框架调用 mount 方法，将新创建的 element 对象添加到树中的给定父级的给定 slot 中。mount 方法负责 inflate 任何子 widget，并根据需要调用 attachRenderObject 方法将任何关联的 render object 附加到 render object tree 中。(当上面的 newWidget.createElement() 调用完成返回创建的 Element newChild 对象，然后就会直接调用：**newChild.mount(this, newSlot);**，上面说的给定父级的给定的 slot，就是指调用 mount 函数传的两个参数 parent 和 newSlot：**void mount(Element? parent, Object? newSlot)**。而后半句根据需要调用 attachRenderObject 就是指的：RenderObjectElement 的 attachRenderObject 方法，在 RenderObjectElement 的 mount 中，它会直接调用：**void attachRenderObject(Object? newSlot)** 方法把 renderObject 附加到 RenderObject Tree 上去。)

+ 在这种情况下，该 element 被视为 "active"，可能会出现在屏幕上。（可以理解为：RenderObjectElement 可以绘制在屏幕上，而 StatefulElement/StatelessElement 则主要用来组织 Widget UI）

+ 在某些情况下，父级 widget 可能会决定更改用于配置此 element 的 widget，例如因为父级 widget 使用新 state 重建了。当发生这种情况时，Flutter framework 将使用新 widget 调用 update 函数。新 widget 将始终具有与旧 widget 相同的 runtimeType 和 key。如果父级 widget 希望在树中的此位置更改 widget 的 runtimeType 或 key，可以通过卸载此 element 并在此位置 inflate new widget 来实现。(update 函数是 Element 的一个函数，而且它由一个 @mustCallSuper 注解，字面意思的就是所有的 element 子类重写 update 函数时，必须要调用 super.update(newWidget)，这个 update 函数，几乎所有的 Element 子类都重写了，基本实现内容就是拿这个传来的 new widget 执行重建（子树），调用整个 Element 最重要的：**Element? updateChild(Element? child, Widget? newWidget, Object? newSlot)** 函数，根据入参内部会进行：新建 Element/更新 Element/移除 Element)

+ 在某些情况下，祖先 element(ancestor element)可能会决定将当前 element（或者中间祖先 element）从树上移除，祖先 element 通过调用 deactivateChild 函数来实现这个操作（deactivateChild 函数的功能：把以入参为根节点的 Element 子树整个的从 Element Tree 上给卸载下来。）。当中间祖先 element 被移除时，该 element 的 RenderObject 就会从 RenderObject Tree 中移除，并将当前 element 添加到 owner 的非活动元素列表中，这会导致 Flutter framework 调用当前 element 的 deactivate 方法。

+ 在这种情况下，element 被认为是 "inactive"，不会出现在屏幕上。一个 element 只能保持在非活动状态直到当前帧结束，在当前帧结束时，任何仍然处于非活动状态的 element 对象将会被卸载。(即当前帧结束了，收集的那些依然处于非活动状态的 element 对象就可以被 unmount，这个是对移除的 element 的优化复用机制，主导思想就是：如果 element 能复用就不进行新建)  换句话说，如果一个 element 在当前帧没有在屏幕上展示出来，那么它将会被移除(unmounted)。

+ 如果一个 element 被重新加入到树中（例如，因为它或它的祖先之一使用的 global key 被重用了），Flutter framework 会从 owner 的非活动元素列表中移除该 element，调用该 element 的 activate 函数，然后将该 element 的 RenderObject 重新附加到 RenderObject Tree 中。（在这一点上，该 element 再次被认为是 "active"，可能会出现在屏幕上。）

+ 如果一个 element 在当前帧结束时没有重新加入到树中，那么 Flutter framework 会调用该 element 的 unmount 方法。

+ 在这种情况下，这个 element 被认为是 "defunct"，并且将来不会被加入到树中。换句话说，这个 element 已经被标记为不再需要，不会被使用到。

&mesp;OK，下面我们开始看 Element 的源码，说到底还是看代码的话，条理比较清晰，最主要能从代码实现上证明上面提到的所有概念，以及上面不懂的概念，可以通过代码去看懂它们。

&emsp;不过在正式看 Element 之前，我们还是先通过一个简单的示例代码，并通过打断点，看函数堆栈的情况去分析 Element 相关函数的执行流程。（贸然去看 Element 代码的话，真的会很容易被里面的相互调用的函数以及父类子类同一函数的穿插调用搞懵。）

&emsp;我们准备了一个极简单的页面，主要帮助我们梳理两个过程：

```dart
void main() {
  runApp(const MyUpdateAppWidget());
}

class MyUpdateAppWidget extends StatelessWidget {
  const MyUpdateAppWidget({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
          appBar: AppBar(title: const Text('Element Study')),
          body: const Center(child: OneWidget())),
    );
  }
}

class OneWidget extends StatefulWidget {
  const OneWidget({super.key});

  @override
  State<StatefulWidget> createState() => _OneWidgetState();
}

class _OneWidgetState extends State<OneWidget> {
  void _click() {
    setState(() {});
  }

  @override
  Widget build(BuildContext context) {
    debugPrint('_OneWidgetState build');

    return ElevatedButton(onPressed: _click, child: const Text('Click me'));
  }
}
```

1. Widget 对象创建到 Inflate 为 Element 对象并挂载在 Element Tree 上的过程。
2. Element.markNeedsBuild 调用 Element 对象被标记为脏后，下一帧它如何进行重建。

&emsp;这两个过程中涉及的函数调用栈是我们的关注重点，它们会把 Element 的各个函数串联起来。

&emsp;虽然我们的示例代码只是看一个 Widget 层级较少的页面，但其实复杂 Widget 层级的构建流程是一样的，只是复杂 Widget 页面有更多的完全一样的重复构建过程而已，但其实只要我们能看懂一层的构建流程即可，再多的 Widget 层级每层的构建流程也都是一样的。

&emsp;下面我们自己先轻捋一下 widget 对象和 element 对象构建，以及 Element 对象被挂载，然后慢慢构建出 Element tree 的过程。

&emsp;首先，我们知道 Widget 子类需要实现 Widget 的抽象方法：Element createElement() 由此创建 Element 对象，从这可以看出：Widget 对象必要早于 Element 对象创建，有了 widget 对象，然后调用它的 createElement 方法便可创建一个 element 对象。回顾之前的 Widget 系列学习过程中已知，不同的 Widget 子类也会分别创建不同的 Element 子类。

+ StatelessWidget -> `StatelessElement createElement() => StatelessElement(this);`
+ StatefulWidget -> `StatefulElement createElement() => StatefulElement(this);`
+ `ParentDataWidget<T extends ParentData>` -> `ParentDataElement<T> createElement() => ParentDataElement<T>(this);`
+ InheritedWidget -> `InheritedElement createElement() => InheritedElement(this);`

+ LeafRenderObjectWidget -> `LeafRenderObjectElement createElement() => LeafRenderObjectElement(this);`
+ SingleChildRenderObjectWidget -> `SingleChildRenderObjectElement createElement() => SingleChildRenderObjectElement(this);`
+ MultiChildRenderObjectWidget -> `MultiChildRenderObjectElement createElement() => MultiChildRenderObjectElement(this);`

&emsp;那么既然 widget 对象必是先于 element 对象（此 widget 对象对应的 element 对象）创建的，那我们必是要先有了 widget 对象才能调用它的 createElement 函数，创建一个 element 对象出来。那往前一点，APP 刚启动时，先有的第一个 Widget 对象是谁呢？如 ⬆️ 示例代码第一个 Widget 对象是我们传递给 runApp 函数的 `const MyUpdateAppWidget()`！

&emsp;关于 Widget Tree:

&emsp;虽然 RenderObjectWidget 子类和 ProxyWidget 子类有一些单向的 Widget 之间的父子关系，例如：ProxyWidget 有自己的子 widget：`final Widget child`， SingleChildRenderObjectWidget 也有自己的可 null 子 widget：`final Widget? child`，MultiChildRenderObjectWidget 有自己的子 widget 列表：`final List<Widget> children`，但是我们其实是并没有一棵完整的 Widget tree 的，因为如果上面的 `child` 字段指向的是：StatelessWidget 或者 StatefulWidget 子类的话，它们是没有 child 字段的，所以在代码层面我们是无法继续沿着 widget 节点继续向下延伸构建 widget tree 的。

&emsp;但是 Element 则不同，首先 Element 基类有一个 `Element? _parent` 字段，可以直接指向当前这个 Element 的父 Element，然后其它 Element 子类如：ComponentElement、SingleChildRenderObjectElement 等都有自己的：`Element? _child` 字段，即指向它们的子 Element，所以基于这样的数据结构，我们是有一棵完整的类似双向链表的 Element tree 的。（由于每个 element 对象都会引用创建自己的 widget 对象，所以实际上其实沿着 Element tree，我们也是能构建出 widget tree 的呢，毕竟 element 对象和 widget 对象是一一对应的！）

&emsp;而这个 Element tree 呢，正是通过我们日常编码的那些 Widget UI 构建起来的。大部分情况下我们日常写的都是 StatelessWidget 和 StatefulWidget 子类，刚刚说到由于它们都没有 child 字段，所以无法构建一棵完整的 widget tree，不过它们都提供了一个 build 函数，用来返回继续向下的 widget，由此我们便得以继续向下构建 Element tree。

&emsp;那么 build 函数被调用时返回 widget 对象的情况如下：

+ StatelessElement 的 `build` 函数调用时会直接调用它的 `widget` 字段的 `build` 函数，并以自己为参数（StatelessElement 对象即 BuildContext context），返回一个 widget 对象。
+ StatefulElement 的 `build` 函数调用时会直接调用它的 `state` 字段的 `build` 函数，同样也是以自己为参数（StatefulElement 对象即 BuildContext context），返回一个 widget 对象。
+ ProxyElement 的 `build` 函数被调用时则是直接返回它的 `widget` 的 `child` 字段。

&emsp;build 函数是 ComponentElement 的抽象函数，是它的一众子类要分别实现的，而作为 RenderObjectElement 子类的 SingleChildRenderObjectElement 和 MultiChildRenderObjectElement 这种的话，它们没有 build 函数，它们是直接使用自己的 `child/children` 读取到下个 element 节点所使用的 widget 对象，然后进行下个 element 节点的构建。 

&emsp;那么这个 Element tree 是如何构建起来的呢？下面我们直接在 MyUpdateAppWidget 的 build 函数处打一个断点，捋一捋它的函数调用堆栈，沿着调用链走下来，你会看到它们一直是在重复的调用相同的函数，直到 Element.updateChild 函数调用时 newWidget 参数为 null 了，updateChild 函数会 return null，即没有 Widget 对象要 inflate 了，整个循环便会结束，即表示当前帧的 element tree 构建完成了。（那么什么情况下：updateChild 函数调用时 newWidget 参数为 null 呢？SingleChildRenderObjectElement 的 widget 的 child 字段为 null 时。`(widget as SingleChildRenderObjectWidget).child` 为 null。）

&emsp;虽然我们传递给 runApp 的第一个 Widget 对象是 MyUpdateAppWidget，但是直到 Flutter framework 开始着手构建以 MyUpdateAppWidget 对象对应的 Element 节点为根的 Element 子树时，已经到了：`_depth = 13` 的位置，Flutter framework 在前面已经插了 13 层的 Element。

&emsp;如果从 App 启动看到 MyUpdateAppWidget build 断点，调用堆栈过长，不利于我们理解，暂时我们就从要开始构建 MyUpdateAppWidget 对象对应的 Element 节点处开始看，首先我们先直接找到它的父级 Element 节点：`{_InheritedNotifierElement}_FocusInheritedScope` 节点，它是 InheritedElement 的子类，它的 `widget` 是 InheritedWidget 的子类，而此 `widget` 字段的 `child` 正是我们传给 runApp 的 MyUpdateAppWidget 对象。

&emsp;下面我们把函数堆栈定位到：ComponentElement.performRebuild 处：

1️⃣：**ComponentElement.performRebuild：** `void performRebuild()` 的调用，我们直接定位到 `ComponentElement.performRebuild(framework.dart:5642) this = {_InheritedNotifierElement}_FocusInheritedScope built = {MyUpdateAppWidget}MyUpdateAppWidget` 的位置，此时正是 `_FocusInheritedScope` element 将要执行重建的位置，这里说是重建，其实是 `_FocusInheritedScope` element 节点 mount 完成后，开始执行 `_firstBuild`（初次构建），然后执行到这里的，它其实是 `_FocusInheritedScope` element 节点 mount 完成后，开始对其下级 Widget 进行构建的过程，这里只所以用了 "perform rebuild" 作函数名，是为了方便和后续更新流程复用函数而已。`_FocusInheritedScope` 是一个 InheritedElement 的子类，而它的 widget 的 child 正是我们的 MyUpdateAppWidget 对象。所以，当 `_FocusInheritedScope` 执行它的 `build` 函数时，返回的正是我们的 MyUpdateAppWidget 对象，并把它赋值给了 `built` 变量。**总结：** 当一个 Element 节点挂载到 element tree 上后，便要开始对自己的下级 widget 进行构建了(其实是下级整个 Element 子树的构建)，performRebuild 函数便是这个动作的实际入口，通过 `built = build();` 先找到这个下级 Widget 对象是谁，然后开始构建。（StatelessElement/StatefulElement/ProxyElement 三类典型的 element 的 build() 函数执行，当然还有 RenderObjectWidget 子类的找下级 Widget 的方式时读自己的 child 字段即可）所以记得此要义：当一个 Element 节点挂载完成后，要做的第一件事就是找到自己的下级 Widget 对象，然后开始构建自己的 Element 子树（`_child`）。OK，继续往下个栈帧：Element.updateChild ⬇️

2️⃣：**⭐️⭐️⭐️ Element.updateChild：** `_child = updateChild(_child, built, slot);` 的调用，此时 `_FocusInheritedScope` 的 `_child` 为 null，built 是我们的 MyUpdateAppWidget 对象，`_child = updateChild(_child, built, slot);` 调用要做的正是把 MyUpdateAppWidget 对象创建的 element 对象赋值给 `_FocusInheritedScope` 的 `_child`。进入 Element.updateChild 函数内部，updateChild 是一个综合函数，它会根据它的三个参数：`Element? child, Widget? newWidget, Object? newSlot` 值的情况决定本次调用执行何种操作，包括：更新 Element/失活 Element/新建 Element，而我们这次调用根据入参 `Element? child` 为 null，便直接定位到了：`newChild = inflateWidget(newWidget, newSlot)` 即新建 Element。下面要调用大名鼎鼎的 inflateWidget 函数。**总结：** updateChild 函数的名字让人看着迷惑，以为只是拿着新 widget 对象更新 element 对象。但是进入它内部梳理一下，会发现，函数名也只是同 performRebuild 类似的行为，为了新建/更新 Element 都可以复用此函数，针对新建的情况，它的内部实际极其简单，就是直接调用 inflateWidget 函数而已。OK，我们继续进入下个栈帧：Element.inflateWidget ⬇️

3️⃣：**Element.inflateWidget：** 大名鼎鼎的 `Element inflateWidget(Widget newWidget, Object? newSlot)` 函数，最重要的功能就是把入参 newWidget 对象膨胀为一个新 element 对象并把它挂载到 Element tree 上，这里我们进入它函数内部仔细观察它的实现，它其实做了三个极重要的事件：1. 判断传来的 widget 对象是否有 GlobalKey，如果有的话并能取到对应的 element 对象，则把这个 element 挂载到当前 element 节点下，然后使用入参 newWidget 更新 element（构建 element 子树），同时还有一个 GlobalKey 重复使用的检测，如果之前已经有用此 widget 构建的 element 挂载在 element tree 上了，这次你还来，就直接报错。2. 直接调用入参 newWidget 的 createElement 函数：`final Element newChild = newWidget.createElement()`，创建一个新的 element 对象，即我们前面说了好久的，拿 widget 对象创建 element 对象在这里会得到验证，且会直接把 newWidget 对象赋值给新建的 element 对象的 `_widget` 字段（即所有的 Element 对象都会直接持有创建它的 Widget 对象）。3. 执行这个新 element 对象的挂载：`newChild.mount(this, newSlot)`。其实当我们点击 Element.inflateWidget 栈帧时便直接定位到了：`newChild.mount(this, newSlot)`，newChild 便是使用我们的 MyUpdateAppWidget 对象刚创建的 StatelessElement 对象。**总结**：inflateWidget 函数，人如其名，没有什么其它的发散操作，直接由入参 newWidget 创建 Element 节点，然后调用 Element 节点的 mount 函数，把它挂载到 Element tree 上去。OK，我们继续进入下个栈帧：ComponentElement.mount ⬇️

4️⃣：**ComponentElement.mount：** `newChild.mount(this, newSlot);（void mount(Element? parent, Object? newSlot)）` 函数的调用，newChild 是新创建的 MyUpdateAppWidget 对象对应的 element 对象，入参 this 是我们的 `_FocusInheritedScope` element，现在便是要把这新 element 对象挂载到它的父级 element 下了。ComponentElement.mount 内首先是调用父类的 mount 函数，即 Element.mount，内部做了不少事情：首先把当前 element 对象的 `_parent` 字段指向它的父级 Element，更新自己的 `_slot`，是的，没错，所有的 Element 的 `_slot` 都是直接由父级传递下来的。然后更新 `_lifecycleState` 为 active，表示当前 Element 节点处于活动状态啦，然后是 `_depth` 深度信息是用父深度加 1，下面还会进行 `_owner` 和 `_parentBuildScope` 的赋值。再接下来便是一个较重要的操作：**如果这个 element 的 widget 有 GlobalKey 的话，会把 global key 和 element 对象作为键值对直接保存在全局 map 中。** 然后还有两个传递父级的 `_inheritedElements` 和 `_notificationTree` 字段直接赋值给子级 element。此时，新建的 Element 就挂载完成到父级 Element 下了，然后就要开始这个子级 Element 下面的新的 element 节点 的构建了。那么这里就呼应到 1️⃣ 中提到的：`_FocusInheritedScope` element 节点 mount 完成后，开始执行 `_firstBuild` 了。而这里不同的是 `_FocusInheritedScope` 是系统自动构建的，我们看起来不清晰。而这次不同了，要执行 `_firstBuild` 的是我们的 MyUpdateAppWidget widget 对应的 StatelessElement 对象，这里也更好可以看一下：StatelessElement 的 build 和上面 `_FocusInheritedScope` InheritedElement(ProxyElement) 的 build 的不同。**总结：** 从数据结构角度理解 mount 函数的话，此函数的作用就是构建双向链表的下一个节点。然后作为 Element 节点，它会持有那些在整个 Element tree 中附加的数据，例如：`_owner`、`_parentBuildScope`、`_inheritedElements`、`_notificationTree` 四个字段的值都是直接拿 `_parent` 的值赋值给子级 element，`_inheritedElements` 字段在我们学习 InheritedWidget 时介绍过它：以 O(1) 的复杂度便可使子级 widget 取得 InheritedWidget 的数据。其它三个字段，后序我们再介绍（其实前面几篇已经全部详细介绍过了，可以再翻回去看看）。OK，我们继续进入下个栈帧：ComponentElement._firstBuild ⬇️（终究是逃离不开轮回的宿命，当这个 Element 挂载到 Element tree 上以后，就要开始构建下个离自己最近的 Element 节点了，重复的命运开始啦！）

5️⃣：**ComponentElement._firstBuild => Element.rebuild => ComponentElement.performRebuild：** `void _firstBuild()` 函数的调用，即我们前面获得了我们的 MyUpdateAppWidget 对象创建的 StatelessElement 对象，然后执行它的第一次构建，因为是 StatelessElement 类型所以它直接调用了父类 ComponentElement 的 `_firstBuild` 函数，其内部仅有一个 `rebuild()` 函数调用，这里再多提一下，如果是 StatefulElement 类型的话（例如我们示例代码中 OneWidget，它则会构建 StatefulElement 对象），会特殊一点，它重写了自己的 `_firstBuild` 函数，多了两个操作调用它的：`state.initState()` 和 `state.didChangeDependencies()` 后，然后再调用 `super._firstBuild()`，State 相关的内容也极其重要，我们后面再深入学（其实前面几篇已经全部详细介绍过了，可以再翻回去看看）。那么继续看 rebuild 函数，它也是极其简单，由于 Element 一众子类都没有重写它，所以都会直接调用到 Element 基类的 rebuild 函数，仅做了一件事，调用 `performRebuild()` 函数，即：回到了：1️⃣：**ComponentElement.performRebuild** 的调用🤩。OK，我们继续进入下个栈帧：StatelessElement.build ⬇️

6️⃣：**StatelessElement.build：** 这里根据 Element 的类型是 StatelessElement 所以调用到它的 build 函数，而它则是调用自己的 widget 字段的 build 函数。即调用到我们的 MyUpdateAppWidget 对象的 build 函数，然后便命中🎯了我们的断点：`MyUpdateAppWidget.build` 函数，然后我们在其内部返回了一个 MaterialApp widget 对象，接下来便开始使用 MaterialApp widget 对象构建下一级的 Element 节点啦！然后这样一种重复下去，直到没有新的 widget 对象需要构建 element 节点了为止！

&emsp;OK，看了这些函数堆栈，我们大概有一个印象了：首先 Element Tree 是一层（级）一层（级）构建的，当一个 Element 对象挂载到 Element Tree 上以后（mount），就要开始进入它下一层（级）的构建了，首先需要找到下一级所用的 Widget 对象，然后把此 Widget 对象 inflate 为 element 对象，然后再把它 mount 到 element tree 上，然后如此重复，直到没有新的 Widget 对象需要构建了。

&emsp;然后呢还是那个重要的点，有加深一些理解了吗？➡️ 任何一个 Element 节点的新建或重建，都不是当前这个一个 Element 节点的事情，它是整个以此 Element 节点为根的整个 Element 子树的新建或重建。

&emsp;在整个连续的调用堆栈中一直重复的有这些函数，等我们对这些函数一眼看到就知道它们做了什么的时候，大概就是对整个 Element 体系融会贯通了。

+ Element? Element.updateChild(Element? child, Widget? newWidget, Object? newSlot)、
+ Element Element.inflateWidget(Widget newWidget, Object? newSlot)、
+ void ComponentElement.mount(Element? parent, Object? newSlot)、
+ `void StatefulElement._firstBuild()`、
+ `void ComponentElement._firstBuild()`、
+ void Element.rebuild({bool force = false})、
+ void StatefulElement.performRebuild()、
+ void ComponentElement.performRebuild()、
+ void StatelessElement.build()、
+ void StatefulElement.build()、

&emsp;除了其中较直白的函数外，其中最令人迷惑的是：`Element.updateChild(Element? child, Widget? newWidget, Object? newSlot)` 函数，它的主要职责如同它的名字，更新当前这个父级 Element 节点的子级 Element，而这个更新呢又大致分为三个情况：新建子级 Element/更新子级 Element/移除子级 Element，在初次构建 Element tree 的过程中，它主要担任新建子级 Element 的任务。（对，看似更新，其实还包含了新建，而且新建呢还包含了两种情况：页面初次构建、页面变化之前旧的 Element 对象不能复用，则只能废弃掉旧的 Element 对象新建新的 Element 对象。）

&emsp;然后另外一个容易令人迷惑的是：`performRebuild` 函数，它的最主要的作用就是：**取得当前 Element 节点的构建下个子级 Element 节点所用的 widget 对象**（针对 ComponentElement 子类而言，RenderObjectElement 子类的话则只需读一下自己的 widget 的 child 字段值），然后继续进行接下来的构建子级 element 的工作，只看它的名字以为只有重建才用到它，但其实它是一个公共函数，新建和重建 Element 节点时都是调用它。

&emsp;而更新 Element tree 的函数调用堆栈呢，同新建类似，我们可以在示例代码 `_OneWidgetState` 的 build 函数打一个断点，然后再点击屏幕上的 Click me 按钮，即进入 OneWidget 对象对应的 Element 节点的更新流程了。

&emsp;与新建不同的是，新建时当一个子 element 节点挂载完成后，会调用 `_firstBuild` 进入下个 Element 节点的构建，而更新的话则是针对标记过的 Element 节点进行直接更新，事先已经把需要更新的 Element 节点收集下来了（脏 Element 列表），然后直接以具体的 element 节点为参数，调用 `BuildScope：void _tryRebuild(Element element)` 函数，然后同上 5️⃣ 处进入 Element 的子树的重建流程，即：**BuildScope._tryRebuild => Element.rebuild => ComponentElement.performRebuild** 流程。具体的代码细节我们到对应的源码分析时再详细展开。

&emsp;OK，大概预热到这里吧，下面我们开始直接看 element 的源码。

## 参考链接
**参考链接:🔗**
+ [Element class](https://api.flutter.dev/flutter/widgets/Element-class.html)
+ [kFlutterMemoryAllocationsEnabled top-level constant](https://api.flutter.dev/flutter/foundation/kFlutterMemoryAllocationsEnabled-constant.html)
+ [Flutter | Understanding the MemoryAllocations](https://medium.com/@maciejbrzezinski/flutter-what-is-memoryallocations-1ee2eb0a8670)
+ [Flutter | How to create a custom binding and inject your own BuildOwner](https://medium.com/@maciejbrzezinski/flutter-how-to-create-a-custom-binding-and-inject-your-own-buildowner-f60ef320537b)
+ [Support for WebAssembly (Wasm)](https://docs.flutter.dev/platform-integration/web/wasm)
