# Element

&emsp;下面开始学习整个 Flutter framework 最核心的一个类：Element。

&emsp;Element 可以看作树中特定位置的 Widget 的实例化。

&emsp;Widget 描述如何配置子树，但同一个 widget 可以同时用于配置多个子树，因为 widget 是不可变的。一个 Element 代表了在树中的特定位置配置 widget 的使用。随着时间的推移，与给定 Element 相关联的 widget 可能会发生变化，例如，如果父 widget 重建并为此位置创建了一个新的 widget。

&emsp;Element 组成一棵树。大多数 element 都有一个唯一的 child，但是一些 widget（例如 RenderObjectElement 的子类）可以有多个 child（children，如：Row、Column 等）。

&emsp;Element 的生命周期如下：

+ Flutter framework 通过调用 Widget.createElement 在将作为 element 初始配置的 widget 上来创建一个 element。 (createElement 仅有的一次调用是在 Element 的 inflateWidget 中由 newWidget 调用：**final Element newChild = newWidget.createElement();**)

+ Flutter framework 框架调用 mount 方法，将新创建的 element 添加到树中的给定父级的给定 slot 中。mount 方法负责 inflate 任何子 widget，并根据需要调用 attachRenderObject 方法将任何关联的 render object 附加到 render object tree 中。(当上面的 newWidget.createElement() 调用完成返回 Element newChild，然后就会直接调用：**newChild.mount(this, newSlot);**，上面说的给定父级的给定的 slot，就是指调用 mount 函数传的两个参数 parent 和 newSlot：**void mount(Element? parent, Object? newSlot)**。而后半句根据需要调用 attachRenderObject 就是指的：RenderObjectElement 的 attachRenderObject 方法，在 RenderObjectElement 的 mount 中，它会直接调用：**void attachRenderObject(Object? newSlot)** 方法把 renderObject 附加到 render object tree 上去。)

+ 在这种情况下，该 element 被视为 "active"，可能会出现在屏幕上。

+ 在某些情况下，父级 widget 可能会决定更改用于配置此 element 的 widget，例如因为父级 widget 使用新 state 重建了。当发生这种情况时，Flutter framework 将使用新 widget 调用 update 函数。新 widget 将始终具有与旧 widget 相同的 runtimeType 和 key。如果父级 widget 希望在树中的此位置更改 widget 的 runtimeType 或 key，可以通过卸载此 element 并在此位置 inflate new widget 来实现。(update 函数是 Element 的一个函数，而且它由一个 @mustCallSuper 注解，字面意思的就是所有的 element 子类重写 update 函数时，必须要调用 super.update(newWidget)，这个 update 函数，几乎所有的 Element 子类都重写了，基本实现内容就是拿这个传来的 new widget 执行 Rebuild，调用整个 Element 最重要的：**Element? updateChild(Element? child, Widget? newWidget, Object? newSlot)** 函数。)

+ 在某些情况下，祖先 element(ancestor element)可能会决定将当前 element（或者中间祖先 element）从树上移除，祖先 element 通过调用 deactivateChild 函数来实现这个操作。当中间祖先 element 被移除时，该 element 的 render object 就会从 render tree 中移除，并将当前 element 添加到 owner 的不活跃元素列表(inactive elements)中，这会导致 Flutter framework 调用当前 element 的 deactivate 方法。

+ 在这种情况下，element 被认为是 "inactive"，不会出现在屏幕上。一个 element 只能保持在 inactive 状态直到当前动画帧结束。在动画帧结束时，任何仍然处于 inactive 状态的 element 将会被卸载。(即当前帧结束了，收集的那些依然处于非活动状态的 element 就可以被 GC 回收了，这个是对移除的 element 的优化复用机制，主导思想就是：如果 element 能复用就不进行新建。)  换句话说，如果一个 element 在当前帧没有在屏幕上展示出来，那么它将会被移除(unmounted)。

+ 如果一个 element 被重新加入到树中（例如，因为它或它的祖先之一使用的 global key 被重用了），Flutter framework 会从 owner 的非活动元素列表(list of inactive elements)中移除该 element，调用该 element 的 activate 函数，然后将该 element 的 render object 重新附加到 render tree 中。（在这一点上，该 element 再次被认为是 "active"，可能会出现在屏幕上。）

+ 如果一个 element 在当前动画帧结束时没有重新加入到树中，那么 Flutter framework 会调用该 element 的 unmount 方法。

+ 在这种情况下，这个 element 被认为是 "defunct"，并且将来不会被加入到树中。换句话说，这个 element 已经被标记为不再需要，不会被使用到。

&mesp;OK，下面我们开始看 Element 的源码，说到底还是看代码的话，条理比较清晰，但是在正式看 Element 之前，我们先通过一个简单的示例代码，并通过打断点，看下函数堆栈。

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

1. Widget 初次在页面上呈现的过程。
2. 当调用 setState 后，Widget 的更新过程。

&emsp;这两个过程中涉及的函数调用栈是我们的关注重点，它们会把 Element 的各个函数串联起来。

&emsp;虽然我们的示例代码只是看一个 Widget 层级较少的页面，但其实复杂 Widget 层级的构建流程是一样的，只是复杂 Widget 页面有更多的完全一样的重复构建过程而已，但其实只要我们能看懂一层的构建流程即可，再多的 Widget 层级每层的构建流程也都是一样的。

&emsp;下面我们自己先轻捋一下 Widget 对象和 Element 对象构建，以及 Element 对象被挂载，然后慢慢构建出 Element tree 的过程。

&emsp;首先，我们知道 Widget 子类需要实现 Widget 的抽象方法：`Element createElement();` 由此创建 Element 对象，从这可以看出：Widget 对象必要早于 Element 对象先创建，有了 Widget 对象，然后调用它的 createElement 方法便可创建 Element 对象。回顾之前的 Widget 系列学习过程中已知，不同的 Widget 子类也会分别创建不同的 Element 子类。

+ StatelessWidget -> `StatelessElement createElement() => StatelessElement(this);`
+ StatefulWidget -> `StatefulElement createElement() => StatefulElement(this);`
+ `ParentDataWidget<T extends ParentData>` -> `ParentDataElement<T> createElement() => ParentDataElement<T>(this);`
+ InheritedWidget -> `InheritedElement createElement() => InheritedElement(this);`

+ LeafRenderObjectWidget -> `LeafRenderObjectElement createElement() => LeafRenderObjectElement(this);`
+ SingleChildRenderObjectWidget -> `SingleChildRenderObjectElement createElement() => SingleChildRenderObjectElement(this);`
+ MultiChildRenderObjectWidget -> `MultiChildRenderObjectElement createElement() => MultiChildRenderObjectElement(this);`

&emsp;那么既然 Widget 对象必是先于 Element 对象（此 Widget 对象对应的 Element 对象）创建的，那我们必是要先有了 Widget 对象才能调用它的 createElement 函数，创建一个 Element 对象出来。那往前一点，APP 刚启动时，先有的第一个 Widget 对象是谁呢？如 ⬆️ 示例代码第一个 Widget 对象必是我们传递给 runApp 函数的 `const MyUpdateAppWidget()` 对象！

&emsp;首先虽然 RenderObjectWidget 子类和 ProxyWidget 子类有一些单向的 Widget 之间的父子关系，例如：ProxyWidget 有自己的子 widget：`final Widget child;`， SingleChildRenderObjectWidget 有自己的可 null 子 widget：`final Widget? child;`，MultiChildRenderObjectWidget 有自己的子 widget 列表：`final List<Widget> children;`，但是我们其实是并没有一棵完整的 Widget tree 的，因为如果上面的 `child` 字段指向的是：StatelessWidget 或者 StatefulWidget 子类的话，它们是没有 child 字段的，所以无法继续沿着 widget tree 继续向下延伸了。但是 Element 则不同，首先 Element 基类有一个 `Element? _parent;` 字段，可以直接指向当前这个 Element 的父 Element，然后其它 Element 子类如：ComponentElement、SingleChildRenderObjectElement 等都有自己的：`Element? _child;` 字段，即指向它们的子 Element，所以基于这样的数据结构，我们是有一棵完整的类似双向链表的 Element tree 的。

&emsp;而这个 Element tree 呢，正是通过我们日常编码的那些 Widget tree 构建起来的。大部分情况下我们日常写的都是 StatelessWidget 和 StatefulWidget 子类，刚刚说到由于它们都没有 child 字段，所以无法构建一棵完整的 widget tree，不过它们都提供了一个 build 函数，用来返回继续向下的 widget，由此我们便得以继续向下构建 Element tree。

&emsp;那么 build 函数被调用时返回 widget 对象的情况如下：

+ StatelessElement 的 build 函数调用时会直接调用它的 widget 字段的 build 函数，并以自己为参数（StatelessElement 对象即 BuildContext context）。
+ StatefulElement 的 build 函数调用时会直接调用它的 state 字段的 build 函数，同样也是以自己为参数（StatefulElement 对象即 BuildContext context）。
+ ProxyElement 的 build 函数被调用时则是直接返回自己 widget 的 child 字段。

&emsp;那么这个 Element tree 是如何构建起来的呢？下面我们直接在 MyUpdateAppWidget 的 build 函数处打一个断点，捋一捋它的函数调用堆栈，沿着调用链走下来，你会看到它们一直是在重复的调用相同的函数，直到 Element.updateChild 函数调用时 newWidget 参数为 null 了，updateChild 函数会 return null，即没有 Widget 对象要 inflate 了，整个循环便会结束，即表示当前帧的 element tree 构建完成了。（那么什么情况下：updateChild 函数调用时 newWidget 参数为 null 呢？SingleChildRenderObjectElement 的 widget 的 child 字段为 null 时。`(widget as SingleChildRenderObjectWidget).child` 为 null。）

&emsp;虽然我们传递给 runApp 的第一个 Widget 对象是 MyUpdateAppWidget，但是直到 Flutter framework 开始着手构建 MyUpdateAppWidget 对象对应的 Element 节点时，已经到了：`_depth = 13` 的位置，Flutter framework 在前面已经插了 13 层的 Element，如果从 App 启动看到 MyUpdateAppWidget build 断点，调用堆栈过长，不利于我们理解，暂时我们就从要开始构建 MyUpdateAppWidget 对象对应的 Element 节点处开始看，首先我们先直接找到它的父级 Element 节点：`{_InheritedNotifierElement}_FocusInheritedScope` 节点，它是 InheritedElement 的子类，它的 `widget` 是 InheritedWidget 的子类，而此 `widget` 字段的 `child` 正是我们的 MyUpdateAppWidget 对象。

&emsp;下面我们把函数堆栈定位到：ComponentElement.performRebuild 处：

1️⃣：**ComponentElement.performRebuild：** `void performRebuild()` 的调用，我们直接定位到 `ComponentElement.performRebuild(framework.dart:5642) this = {_InheritedNotifierElement}_FocusInheritedScope built = {MyUpdateAppWidget}MyUpdateAppWidget` 的位置，此时正是 `_FocusInheritedScope` element 将要执行重建的位置，这里说是重建，其实是 `_FocusInheritedScope` element 节点 mount 完成后，开始执行 `_firstBuild`，然后执行到这里的，它其实是 `_FocusInheritedScope` element 节点 mount 完成后，开始对其下级 Widget 进行构建的过程，这里只所以用了 "perform rebuild" 作函数名，是为了方便和后续更新流程复用函数而已。`_FocusInheritedScope` 是一个 InheritedElement 的子类，而它的 widget 的 child 正是我们的 MyUpdateAppWidget 变量。所以，当 `_FocusInheritedScope` 执行它的 `build` 函数时，返回的正是我们的 MyUpdateAppWidget 对象，并把它赋值给了 `built` 变量。**总结：** 当一个 Element 节点挂载到 element tree 上后，便要开始对自己的下级 widget 进行构建了，performRebuild 函数便是这个动作的实际入口，通过 `built = build();` 先找到这个下级 Widget 对象是谁，然后开始构建。（StatelessElement/StatefulElement/ProxyElement三类典型的 element 的 build() 函数执行，当然还有 RenderObjectWidget 的找下级 Widget 的方式时读自己的 child 字段即可）所以记得此要义：当一个 Element 节点挂载完成后，要做的第一件事就是找到自己的下级 Widget 对象，然后开始构建自己的下级 Element（`_child`）。OK，继续往下个栈帧：Element.updateChild ⬇️

2️⃣：**⭐️⭐️⭐️ Element.updateChild：** `_child = updateChild(_child, built, slot);` 的调用，此时 `_FocusInheritedScope` 的 `_child` 为 null，built 是我们的 MyUpdateAppWidget 变量，`_child = updateChild(_child, built, slot);` 调用要做的正是把 MyUpdateAppWidget 变量构建的 element 对象赋值给 `_FocusInheritedScope` 的 `_child`。进入 Element.updateChild 函数内部，updateChild 是一个综合函数，它会根据它的三个参数：`Element? child, Widget? newWidget, Object? newSlot` 值的情况决定本次调用执行何种操作，包括：更新 Element/失活 Element/新建 Element，而我们这次调用根据入参 `Element? child` 为 null，便直接定位到了：`newChild = inflateWidget(newWidget, newSlot)` 即新建 Element。大名鼎鼎的 inflateWidget 函数。**总结：** updateChild 函数的名字让人看着迷惑，以为只是拿着新 widget 更新 element。但是进入它内部梳理一下，会发现，函数名也只是同 performRebuild 类似的行为，为了新建/更新 Element 都可以复用此函数，针对新建的情况，它的内部实际极其简单，就是直接调用 inflateWidget 函数而已。OK，我们继续进入下个栈帧：Element.inflateWidget ⬇️

3️⃣：**Element.inflateWidget：** 大名鼎鼎的 `Element inflateWidget(Widget newWidget, Object? newSlot)` 函数，最重要的功能就是把入参 newWidget 对象膨胀为一个新 element 并把它挂载到 Element tree 上，这里我们进入它函数内部细观察它的实现，它其实做了三个极重要的事件：1. 判断传来的 widget 对象是否有 GlobalKey，如果有的话并能取到对应的 element 的话，则把这个 element 挂载到当前 element 节点下，然后使用入参 newWidget 更新 element，同时还有一个 GlobalKey 重复使用的检测，如果之前已经有用此 widget 构建的 element 挂载在 element tree 上了，这次你还来，就直接报错。2. 直接调用入参 `final Element newChild = newWidget.createElement();`，创建一个新 Element，即我们前面说了好久的，拿 Widget 对象创建 Element 对象在这里会得到执行，且会直接把 newWidget 对象赋值给新建的 Element 对象的 `_widget` 字段（即所有的 Element 对象都会直接持有创建它的 Widget 对象）。3. 执行这个新 element 的挂载：`newChild.mount(this, newSlot);`。其实当我们点击 Element.inflateWidget 栈帧时便直接定位到了：`newChild.mount(this, newSlot);`，newChild 便是使用我们的 MyUpdateAppWidget widget 对象刚创建的 StatelessElement 对象。**总结**：inflateWidget 函数，人如其名，没有什么其它的发散操作，直接由 newWidget 创建 Element 节点，让后调用 Element 节点的 mount 函数，把它挂载到 Element tree 上去。OK，我们继续进入下个栈帧：ComponentElement.mount ⬇️

4️⃣：**ComponentElement.mount：** `newChild.mount(this, newSlot);（void mount(Element? parent, Object? newSlot)）` 函数的调用，newChild 是新创建的 MyUpdateAppWidget widget 对象对应的 element 对象，入参 this 是我们的 `_FocusInheritedScope` element，现在便是要把这新 element 对象挂载到它的父级 element 下了。ComponentElement.mount 内首先是调用父类的 mount 函数，即 Element.mount，内部做了不少事情：首先把当前 element 对象的 `_parent` 字段指向它的父级 Element，更新自己的 `_slot`，是的，没错，所有的 Element 的 `_slot` 都是直接由父级传递下来的。然后更新 `_lifecycleState` 为 active，表示当前 Element 节点处于活动状态啦，然后是 `_depth` 深度信息是用父深度加 1，下面还会进行 `_owner` 和 `_parentBuildScope` 的赋值。再接下来便是一个较重要的操作：**如果这个 element 的 widget 有 GlobalKey 的话，会把 global key 和 element 对象作为键值对直接保存在全局 map 中。** 然后还有两个传递父级的 `_inheritedElements` 和 `_notificationTree` 字段直接赋值给子级 element。此时，新建的 Element 就挂载完成到父级 Element 下了，然后就要开始这个子级 Element 下面的新的 element 节点 的构建了。那么这里就呼应到 1️⃣ 中提到的：`_FocusInheritedScope` element 节点 mount 完成后，开始执行 `_firstBuild` 了。而这里不同的是 `_FocusInheritedScope` 是系统自动构建的，我们看起来不清晰。而这次不同了，要执行 `_firstBuild` 的是我们的 MyUpdateAppWidget widget 对应的 StatelessElement 对象，这里也更好可以看一下：StatelessElement 的 build 和上面 `_FocusInheritedScope` InheritedElement(ProxyElement) 的 build 的不同。**总结：** 从数据结构角度理解 mount 函数的话，此函数的作用就是构建双向链表的下一个节点。然后作为 Element 节点，它会持有那些在整个 Element tree 中附加的数据，例如：`_owner`、`_parentBuildScope`、`_inheritedElements`、`_notificationTree` 四个字段的值都是直接拿 `_parent` 的值赋值给子级 element，`_inheritedElements` 字段在我们学习 InheritedWidget 时介绍过它：以 O(1) 的复杂度便可使子级 widget 取得 InheritedWidget 的数据。其它三个字段，后序我们再介绍。OK，我们继续进入下个栈帧：ComponentElement._firstBuild ⬇️（终究是逃离不开轮回的宿命，当这个 Element 挂载到 Element tree 上以后，就要开始构建下个 Element 了，重复的命运开始啦！）

5️⃣：**ComponentElement._firstBuild => Element.rebuild => ComponentElement.performRebuild：** `void _firstBuild()` 函数的调用，即我们前面获得了我们的 MyUpdateAppWidget 对象构建的 StatelessElement 对象，然后执行它的第一次构建，因为是 StatelessElement 类型所以它直接调用了父类 ComponentElement 的 `_firstBuild` 内部仅有一个 `rebuild();` 函数调用，这里再多提一下，如果是 StatefulElement 类型的话（例如我们示例代码中 OneWidget，它则会构建 StatefulElement 对象），会特殊一点，它重写了自己的 `_firstBuild` 函数，多了两个操作调用它的：`state.initState()` 和 `state.didChangeDependencies();` 后，然后再调用 `super._firstBuild();`，State 相关的内容也极其重要，我们后面再深入学。那么继续看 rebuild 函数，它也是极其简单，由于 Element 一众子类都没有重写它，所以都会直接调用到 Element 基类的 rebuild 函数，仅做了一件事，调用 `performRebuild();` 函数，即：回到了：**ComponentElement.performRebuild** 的调用🤩。OK，我们继续进入下个栈帧：StatelessElement.build ⬇️

6️⃣：**StatelessElement.build：** 这里根据 Element 的类型是 StatelessElement 所以调用到它的 build 函数，而它则是调用自己的 widget 的 build 函数。即调用到我们的 MyUpdateAppWidget 对象的 build 函数，然后便命中🎯了我们的断点：`MyUpdateAppWidget.build` 函数，然后我们在其内部返回了一个 MaterialApp widget 对象，接下来便开始使用 MaterialApp widget 对象构建下一级的 Element 节点啦！然后这样一种重复下去，直到没有新的 widget 对象需要构建 element 节点了为止！

&emsp;OK，看了这些函数堆栈，我们大概有一个印象了：首先 Element Tree 是一级一级构建的，当一个 Element 对象挂载到 Element Tree 上以后（mount），就要开始进入它下一级的构建了，首先需要找到下一级的 Widget 对象，然后把此 Widget 对象 inflate 为 element，然后再把它 mount 到 element tree 上，然后再重复下一级的构建。

&emsp;在整个连续的调用堆栈中一直重复的有这些函数：

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

&emsp;其中最令人迷惑的是：`Element.updateChild(Element? child, Widget? newWidget, Object? newSlot)` 函数，它的主要职责如同它的名字，更新当前这个父级 Element 节点的子级 Element，而这个更新呢又大致分为三个情况：新建子级 Element/更新子级 Element/移除子级 Element，在初次构建 Element tree 的过程中，它主要担任新建子级 Element 的任务。然后另外一个令人迷惑的是：`performRebuild` 函数，它的最主要的作用就是：取得当前父级 Element 节点的构建下个子级 Element 节点所用的 Widget，只看它的名字以为只有重建才用到它，但其实它是一个公共函数，新建和重建 Element 节点时都是调用它。

&emsp;而更新 Element tree 的函数调用堆栈呢，同新建类似，我们可以在示例代码 `_OneWidgetState` 的 build 函数打一个断点，然后再点击屏幕上的 Click me 按钮，即进入 OneWidget 对象对应的 Element 节点的更新流程了。

&emsp;与新建不同的是，新建时当一个子 element 节点挂载完成后，会调用 `_firstBuild` 进入下个 Element 节点的构建，而更新的话则是针对标记过的 Element 节点进行直接更新，事先已经把需要更新的 Element 节点收集下来了，然后直接以具体的 element 节点为参数，调用 `BuildScope：void _tryRebuild(Element element)` 函数，然后同上 5️⃣ 处进入 Element 的重建流程，即：**BuildScope._tryRebuild => Element.rebuild => ComponentElement.performRebuild** 流程。具体的代码细节我们到对应的源码分析时再详细展开。

&emsp;OK，大概预热到这里吧，下面我们开始直接看 element 的源码。








## element

&emsp;看 Element 的声明，直接继承自 DiagnosticableTree 没啥特别的的，然后便是最重要的：Element 需要实现 BuildContext 抽象类中所有的抽象方法。而这个 BuildContext 就是我们在 StatelessWidget 的 build 和 State 的 build 函数中见了无数次的：BuildContext context 参数，其实 BuildContext 只是定了很多 getter 和抽象方法的一个抽象类，最终实现呢，全部落在了需要实现它的 Element 类身上，而实际在 Widget/State 的 build 函数中传递来的 context 参数便是它们对应的 Element 对象。

&emsp;BuildContext 类中定义了较多 getter 和抽象函数，我们先不去看，暂时还先看 Element 类，毕竟 Element 会实现 BuildContext 抽象类的所有要求。

```dart
abstract class Element extends DiagnosticableTree implements BuildContext {
    // ...
}
```

### constructor

&emsp;Element 的构造函数：创建一个使用给定 widget 作为其配置的 Element 对象，通常由子 Widget 重写 Widget 的：`Element createElement()` 进行调用。几乎所有的子 Widget 都重写了 createElement 函数，构建一个子 Element。（毕竟它是一个抽象类的抽象函数，子类还是要进行自己实现此函数的。）

```dart
  Element(Widget widget)
    : _widget = widget {
    if (kFlutterMemoryAllocationsEnabled) {
      FlutterMemoryAllocations.instance.dispatchObjectCreated(
        library: _flutterWidgetsLibrary,
        className: '$Element',
        object: this,
      );
    }
  }
```

&emsp;然后看到初始化列表，直接把 widget 参数赋值给了 Element 的 `_widget` 字段，从这里可以看出，Element 会引用它对应的 Widget 的，毕竟这 Widget 是此 Element 的配置信息嘛！被 Element 所引用很正常。

&emsp;Element 初始化列表下面跟的一段代码，看起来怪怪的。

&emsp;它们是为了在 Flutter 应用中追踪内存分配的情况的。如果 kFlutterMemoryAllocationsEnabled 为 true，表示内存分配的追踪功能已经开启，则调用 FlutterMemoryAllocations.instance.dispatchObjectCreated 方法，将创建的对象相关信息传递给追踪系统。在这里将传递所属库（library）、类名（className）以及具体对象实例（object）。

```dart
// 默认情况下，在 debug 模式下该常量为 true，在 profile 和 release 模式下为 false。
// 要在 release 模式下启用，请传递编译标志：--dart-define=flutter.memory_allocations=true。
const bool kFlutterMemoryAllocationsEnabled = _kMemoryAllocations || kDebugMode;
```

&emsp;FlutterMemoryAllocations 是一个用于监听对象生命周期事件的类。FlutterMemoryAllocations 可以监听 Flutter framework 中 disposable 对象的创建和销毁事件。要监听其他对象事件，调用 FlutterMemoryAllocations.dispatchObjectEvent。使用该类时应该满足 kFlutterMemoryAllocationsEnabled 的条件，以确保在禁用内存分配时不会通过该类的代码增加应用程序的大小。这个类经过优化，适用于处理大量事件流和少量的添加或移除监听器。

&emsp;看了一下感觉此类就是一个监听器，用来监听对象的创建和销毁。例如在 framework.dart 中搜索：FlutterMemoryAllocations.instance，总共发现了四处调用，分别是针对 Element 和 State 的创建和销毁：

1. 在 Element 的构造函数处调用：dispatchObjectCreated，在 unmount 函数处调用 dispatchObjectDisposed。
2. 在 State 的 initState 函数处调用：dispatchObjectCreated，在 dispose 函数处调用 dispatchObjectDisposed。

&emsp;统计 Element 和 State 对象的创建和销毁，一一对应，至于其它的深入使用，后续再研究。

### `_parent`

&emsp;element 有一个指向它的父级 element 的引用，在 mount 时会给 `_parent` 赋值，然后 ComponentElement 有 `Element? _child;`（StatelessElement/StatefulElement/ProxyElement 都是 ComponentElement 子类），SingleChildRenderObjectElement 有 `Element? _child;`，MultiChildRenderObjectElement 有 `late List<Element> _children;`，现在可以看出来了吧，element tree 其实是一个双向链表结构。 

```dart
  Element? _parent;
```

### `_notificationTree`

### operator ==

&emsp;用于比较两个 Element 的相等性，看代码的话，它加了 @nonVirtual 注解，表示所有的 Element 子类都不能重写 operator ==，它们的对象比较时也都使用 Element 的 ==，然后是比较方式，是使用全局的 identical 函数，需要完全保证两个 Element 对象是同一个对象。

&emsp;看代码明明是 other 和 this 的比较，是两个 Element 的比较，但是注释中说的是 Widget 的比较：

&emsp;比较两个 widget 是否相等。当一个 widget 通过 operator == 与另一个 widget 比较相等时，假定更新是多余的，跳过更新该树分支的工作。通常不建议在任何具有 children widget 的 widget 上重写 operator ==，因为正确的实现将不得不转移到 children widget 的相等运算符，这是一个 O(N²) 的操作：每个 child widget 都需要遍历它的所有 children widget，树上的每一步。

&emsp;如果重新构建 widget 比检查 widget 的参数是否相等更昂贵，而且预计 widget 经常会被相同的参数重新构建，那么有时一个 leaf widget（没有 children 的 widget）实现这个方法是合理的。但是通常情况下，如果已知 widget 不会更改，最好在 build 方法中缓存使用的 widget，这样效率更高。

```dart
  @nonVirtual
  @override
  // ignore: avoid_equals_and_hash_code_on_mutable_classes, hash_and_equals
  bool operator ==(Object other) => identical(this, other);
```

### slot

&emsp;slot 类似我们在 iOS 原生中 View 的 frame，都是表示子级在父级中的位置。（不同的是 iOS 原生中一般我们画的每个 View 都会正常显示在画面上，而 Widget/Element 则不一定，只有 RenderObjectWidget 才有机会显示在屏幕上。）

&emsp;slot 是父级设置的信息，用来定义子级在其父级的子级列表中的位置。当父级调用 updateChild 方法来 inflate 子 widget 时，子 widget 的 slot 位置会确定。等后面我们学习 RenderObjectElement 时再详细学习 slot 相关的内容。（在 Flutter 中只有能创建 RenderObject 的 RenderObjectWidget 才有机会绘制在屏幕上。）

```dart
  Object? get slot => _slot;
  Object? _slot;
```

&emsp;这里要先注意一下的点，slot 是一个 Object 类型，针对不同的 widget 它有不同的类型，不像是 iOS 中所有的 frame 都是 CGRect 一个类型的。

### depth

&emsp;depth 是一个简单的 int 值，表示 element 在 element tree 中的深度值，如果存在父级，则保证整数大于父级，树的根节点必须具有大于 0 的深度。

&emsp;depth getter 内部有一个断言：如果 element 处于 `_ElementLifecycle.initial` 状态的话，会报错：Depth 信息仅在 element 已经 mount 后才可读取。

```dart
  int get depth {
    // ...
    return _depth;
  }
  late int _depth;
```

### widget/mounted

&emsp;widget 这里是 Element 直接引用了它对应的 widget。

&emsp;widget 表示该 element 的配置信息。应避免在 Element 的子类型中重写此字段，以提供一个更具体的 widget 类型（例如在 StatelessElement 中重写此字段为： StatelessWidget get widget，直接指定其为 StatelessWidget 类型）。相反，在需要更具体类型时，应在任何调用处进行类型转换。这样可以避免在构建阶段的 "getter" 中产生大量的类型转换开销，而在该过程中访问到这个字段 —— 而且不会使用更具体的 widget 的类型信息。

&emsp;mounted 已挂载标识，如果 Element 的 widget 字段不为 null，就表示已经挂载。

```dart
  @override
  Widget get widget => _widget!;
  Widget? _widget;
  
  @override
  bool get mounted => _widget != null;
```

### owner

&emsp;owner 是一个在 Element tree 上传递的全局对象，并且是管理 element 生命周期的对象。后面我们会详细看这个 BuildOwner 类型。

```dart
  @override
  BuildOwner? get owner => _owner;
  BuildOwner? _owner;
```

### buildScope

&emsp;当这个 Element 已经被 mounted 时，通常访问这个 getter 才是安全的。默认实现会返回父 Element 的 buildScope，因为在大多数情况下，一个 Element 一旦其祖先不再是脏的就可以准备重建。一个值得注意的例外是 LayoutBuilder 的子孙节点，在接收到约束之前不应该进行重建。LayoutBuilder 的 Element 重写了 buildScope 以确保其所有子孙节点在接收到约束之前都不会进行重建。如果你选择重写这个 getter 来建立自己的 BuildScope，需要在适当的时候手动调用 BuildOwner.buildScope，并传入你的 BuildScope 的根 Element，因为 Flutter framework 不会尝试注册或管理自定义的 BuildScope。如果你重写这个 getter，请始终返回相同的 BuildScope 实例。不支持在运行时更改此 getter 返回的值。updateChild 方法忽略 buildScope：如果父 Element 在一个具有不同 BuildScope 的子 Element 上调用 updateChild]，子 Element 可能仍然会重建。

&emsp;有点晕，后面再来看。

```dart
  BuildScope get buildScope => _parentBuildScope!;
  // 父 Element build scope 的缓存值。当该 Element mount 或 reparent 时，缓存会被更新。
  BuildScope? _parentBuildScope;
```

### renderObject

&emsp;当前 element 或者此 element 下的 element 的 render object。(即不仅在此 element 找，而且会顺着 element 链继续往下找。)

&emsp;如果此 element 是 RenderObjectElement 的话，那么直接返回它的 renderObject 就可以了。否则，此 getter 会沿 element 树向下查找，直到找到一个 RenderObjectElement 返回它的 renderObject 为止。

&emsp;当然还有沿着 element 树找不到 render object 的情况。element 树中的某些位置没有对应的 render object 的，在这种情况下，此 getter 返回 null。这可能发生在 element 位于 View 之外的情况下，因为只有位于 view 中的 element 子树有与之关联的 render tree。

```dart
  RenderObject? get renderObject {
    Element? current = this;
    while (current != null) {
      if (current._lifecycleState == _ElementLifecycle.defunct) {
        break;
      } else if (current is RenderObjectElement) {
        return current.renderObject;
      } else {
        // 这里则是遍历往当前 element 的 child 中去找
        current = current.renderObjectAttachingChild;
      }
    }
    return null;
  }
```

### renderObjectAttachingChild

&emsp;返回此 element 的子 element，该子 element 将在此 element 的祖先中插入一个 RenderObject，以构建 render 树。

&emsp;如果此 element 没有任何需要将 RenderObject 附加到此 element 的祖先的子 element，则返回 null。因此，RenderObjectElement 将返回 null，因为其子 Element 将其 RenderObject 插入到 RenderObjectElement 本身而不是其祖先中。

&emsp;此外，对于 hoist 它们自己独立渲染树并且不扩展祖先渲染树的 Element，可能会返回 null。

&emsp;Element 的 visitChildren 函数是个空实现，但是 ComponentElement/SingleChildRenderObjectElement/MultiChildRenderObjectElement 实现了它，并且返回它们的 `_child`，MultiChildRenderObjectElement 的话则是遍历它的：`_children`。

&emsp;Element 并没有 `_child` 字段，但是它的子类添加了此字段，然后配合 `_parent` 字段，可以看出 element tree 是一个双向链表结构。

```dart
  @protected
  Element? get renderObjectAttachingChild {
    Element? next;
    visitChildren((Element child) {
      next = child;
    });
    return next;
  }
```

### `_lifecycleState`

&emsp;这被用来验证 element 对象以有秩序的方式在生命周期中移动。

&emsp;表示 element 目前处于生命周期的哪个状态。

```dart
enum _ElementLifecycle {
  initial,
  active,
  inactive,
  defunct,
}

  _ElementLifecycle _lifecycleState = _ElementLifecycle.initial;
```

### visitChildren

&emsp;这是一个比较特殊的函数，它的参数 visitor 是一个参数是 Element，返回值是 void 的函数：`typedef ElementVisitor = void Function(Element element);`。

&emsp;而且这个函数比较特殊，在 Element 中它是一个空实现，而在有 child 的 Element 子类中要重写它，比如：ComponentElement 重写了 visitChildren：当它的 `_child` 不为 null 时，调用 `vistor(_child)`。然后 SingleChildRenderObjectElement 和 ComponentElement 一样的重写，MultiChildRenderObjectElement 则是对自己 `_children` 中的每个 Element child 调用 `vistor(child)`。

```dart
  void visitChildren(ElementVisitor visitor) { }
```

### visitChildElements

&emsp;仅仅是为了实现 BuildContext 的 visitChildElements 抽象函数，内部是封装的 visitChildren 函数。

```dart
  @override
  void visitChildElements(ElementVisitor visitor) {
    visitChildren(visitor);
  }
```

&emsp;下面我们进入 Element 的核心函数之一：updateChild，超级重要。

### updateChild

&emsp;在 updateChild 函数前有三个很重要的注解：

+ @pragma('dart2js:tryInline') 向 dart2js(Dart-to-JavaScript compiler) 建议在优化期间应优先考虑内联 updateChild 函数。
+ @pragma('vm:prefer-inline') 向 Dart VM 建议在优化期间应优先考虑内联 updateChild 函数。
+ @pragma('wasm:prefer-inline') 向 WebAssembly 建议在优化期间应优先考虑内联 updateChild 函数。

&emsp;貌似有点可惜，此内联优化并不是针对 native APP 生产环境。

&emsp;updateChild 函数的作用：使用给定的新配置更新给定的子 element。该方法是 widget system 的核心。每当我们根据更新的配置要添加、更新或删除子 element 时，就会调用它。

&emsp;newSlot 参数指定了 element slot 的新值。

&emsp;如果子 element 为 null（Element? child 参数），而 newWidget 不为 null（Widget? newWidget 参数），则表示我们有一个需要创建 Element 并配置为 newWidget 的新 child Element。

&emsp;如果 newWidget 为 null（Widget? newWidget 参数），而子 element 不为 null（Element? child 参数），则需要将其移除，因为它不再具有配置。

&emsp;如果两者均不为 null，则需要将子 element 的配置更新为 newWidget 给出的新配置。如果 newWidget 可以提供给现有子 element（由 Widget.canUpdate 决定），则提供。否则，旧子 element 需要被处理并为新配置创建新的 child Element。

&emsp;如果两者均为 null，则我们既没有子 element，也将不会有子 element，因此我们不做任何操作。

&emsp;updateChild 方法返回新的 child element（如果必须创建一个），或者传入的 child element（如果只需要更新子元素），或 null（如果移除了子 element 且没有替换）。

&emsp;下表总结了上述内容:

  |                     | **newWidget == null**  | **newWidget != null**   |
  | :-----------------: | :--------------------- | :---------------------- |
  |  **child == null**  |  Returns null.         |  Returns new Element. |
  |  **child != null**  |  Old child is removed, returns null. | Old child updated if possible, returns child or new Element. |

&emsp;只有在 newWidget 不为 null 时才会使用 newSlot 参数。如果 child 为 null（或者旧 child 无法更新），那么通过 inflateWidget 将 newSlot 分配给为 child 创建的新 Element。如果 child 不为 null（并且旧 child 可以更新），那么通过 updateSlotForChild 给予 newSlot 来更新其 slot，以防它在上次构建后发生移动。

&emsp;好了，针对上述的 5 种情况，我们看代码：

```dart
  @protected
  @pragma('dart2js:tryInline')
  @pragma('vm:prefer-inline')
  @pragma('wasm:prefer-inline')
  Element? updateChild(Element? child, Widget? newWidget, Object? newSlot) {
    if (newWidget == null) {
    // 1️⃣：针对 newWidget 为 null 时，如果 child 不为空，则现在已经不需要它了，直接把它失活了先。
    
      if (child != null) {
        // 1️⃣1️⃣：child 不为 null，但是 widget 已经无了，所以也需要把这旧 element 失活等待内存回收♻️。
    
        // deactivateChild 函数所有的 Element 仅有这一个，子类都没有重写。
        // deactivateChild 把这个指定的 Element child 失活，主要做了三件事：
        // 1. 把 child 的 _parent 置为 null。
        // 2. 把 child 的 render object 从 render tree 上分离。
        // 3. 把 child 放入 owner!._inactiveElements 这个全局的 非活动 Element Set 中，等待被复用。
        deactivateChild(child);
      }
      
      // 旧 element 已经被移除啦，updateChild 返回 null 即可
      return null;
    }

    // 临时变量，主要用来记录返回值，针对下面这些 newWidget 不为 null 的情况，
    // updateChild 函数要么返回新建的 element，要么返回得到更新的旧 element。
    final Element newChild;
    
    if (child != null) {
      // 2️⃣：针对 child 不为 null 的情况，此 if 内尽量尝试对 child 进行更新，实在不行时才进行新建。
      
      bool hasSameSuperclass = true;
      // 原代码这里有一段 hasSameSuperclass 的解释，
      // 主要为了防止热重载时，StatefulWidget 和 StatelessWidget 的相互替换导致问题，
      // 而且断言 assert 代码在生产环境时会被移除，为了减少理解负担，我们就直接给它删掉了。
      
      if (hasSameSuperclass && child.widget == newWidget) {
        // 2️⃣1️⃣：我们之前学习 const 时已经看过 flutter 对它的优化了，
        // 如果 newWidget 是被 const 构造函数构建，并且加了 const 修饰那么新旧 widget 就是同一个对象，
        // 这里如果 newSlot 与 child element 的 slot 不同的话只需更新 child 的 slot 即可。
        if (child.slot != newSlot) {
          updateSlotForChild(child, newSlot);
        }
        
        // 记录下，还是旧 element 返回
        newChild = child;
      } else if (hasSameSuperclass && Widget.canUpdate(child.widget, newWidget)) {
        // 2️⃣2️⃣：如果 newWidget 和旧 widget：runtimeType 和 key 一样的话，那么可以直接更新 child 即可，不需新建。
        
        // 如需要更新 slot，则更新 slot
        if (child.slot != newSlot) {
          updateSlotForChild(child, newSlot);
        }
        
        // 此处原有一段 FlutterTimeline 的代码，生产环境无用，暂时删掉不看。
        
        // newWidget 仅需更新 child 即可
        child.update(newWidget);

        newChild = child;
      } else {
        // 2️⃣3️⃣：针对 child 不能用 newWidget 进行更新的情况，只能新建 element 了，
        // 把旧的 element 给它失活了。 
        deactivateChild(child);
        
        // 使用 newWidget 和 newSlot 开始构建新的子 element tree
        newChild = inflateWidget(newWidget, newSlot);
      }
    } else {
      // 3️⃣：针对 child 为 null 的情况，只能进行新子 element tree 的构建了。
      newChild = inflateWidget(newWidget, newSlot);
    }

    // 把新的 element 或者是更新过的旧 element 返回即可
    return newChild;
  }
```

&emsp;updateChild 函数的内容还是比较清晰的，针对三个参数 Element? child、Widget? newWidget、Object? newSlot 值不同的情况进行不同的处理。

&emsp;我们看到其中一种最省事的情况：新旧 Widget 相等时，widget 不进行任何操作（此 newWidget 的 build 也不会被执行），没有任何开销，仅有的一点是新旧 slot 不同的话，会更新下 element 的 slot。而这个最省事的情况就是对应了官方推荐的优化技巧：提取封装子 widget，声明 const 构造函数，使用时添加 const 修饰使用常量表达式。

&emsp;Element.updateChild -> StatefulElement.update -> Element.rebuild -> StatefulElement.performRebuild -> ComponentElement.performRebuild -> StatefulElement.build -> `_ThreeWidgetState.build`。

1. BuildOwner.buildScope -> `BuildScope._flushDirtyElements` -> `BuildScope._tryRebuild` -> Element.rebuild -> StatefulElement.performRebuild(ComponentElement.performRebuild) -> StatefulElement.build -> `_OneWidgetState.build` -> 调完这个 build 就拿到返回到新 widget 啦，接下来就是调用 element 的：`_child = updateChild(_child, built, slot);` 啦，这个 build 返回的 widget 就是当前 element 的下一个节点的 widget，然后沿着 element 链顺序向下更新 element 节点。

2. 所以到这里就要理解 StatelessWidget/State 的 build 函数返回的 widget 就是当前 element 节点的下个节点的 widget，即：`_child._widget` 就是这个 build 返回的 widget 了。




## 参考链接
**参考链接:🔗**
+ [Element class](https://api.flutter.dev/flutter/widgets/Element-class.html)
+ [kFlutterMemoryAllocationsEnabled top-level constant](https://api.flutter.dev/flutter/foundation/kFlutterMemoryAllocationsEnabled-constant.html)
+ [Flutter | Understanding the MemoryAllocations](https://medium.com/@maciejbrzezinski/flutter-what-is-memoryallocations-1ee2eb0a8670)
+ [Flutter | How to create a custom binding and inject your own BuildOwner](https://medium.com/@maciejbrzezinski/flutter-how-to-create-a-custom-binding-and-inject-your-own-buildowner-f60ef320537b)
+ [Support for WebAssembly (Wasm)](https://docs.flutter.dev/platform-integration/web/wasm)
+ []()
