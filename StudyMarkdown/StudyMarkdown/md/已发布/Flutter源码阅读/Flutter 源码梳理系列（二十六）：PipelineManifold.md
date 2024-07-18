# Flutter 源码梳理系列（二十六）：PipelineManifold

# PipelineManifold

&emsp;PipelineManifold 用于管理一棵 PipelineOwner Tree。

&emsp;PipelineOwner Tree 中的所有 PipelineOwner 都附加到同一个 PipelineManifold 对象，这使的所有的 PipelineOwner 对象能够访问共享功能，例如请求视觉更新（通过调用 manifold.requestVisualUpdate()）。因此，PipelineManifold 为 PipelineOwners 提供了通常由 bindings 提供的功能，而不将 PipelineOwners 绑定到特定的 binding 实现。

&emsp;PipelineOwner Tree 的根通过将 PipelineManifold 传递给 PipelineOwner.attach 与 PipelineManifold 相连。当通过 PipelineOwner.adoptChild 收养子级 PipelineOwner 时，子级将附加到与其父级相同的 PipelineManifold 对象。 

&emsp;PipelineOwners 可以在 PipelineManifold 中注册监听器，以在 PipelineManifold 提供的某些值更改时得到通知。

## Constructors

&emsp;PipelineManifold 是一个需要实现 Listenable 接口的抽象类。即交由它的子类来实现了。（下一篇我们会着重分析 Listenable 和 ChangeNotifier 提供的 发布-订阅 机制。）

```dart
abstract class PipelineManifold implements Listenable {
  // ...
}
```

## semanticsEnabled

&emsp;PipelineOwners 连接到此 PipelineManifold 是否应收集语义信息并生成语义树。

&emsp;当此属性更改其值时，PipelineManifold 会通知其监听器（通过 addListener 和 removeListener 管理）。

&emsp;另请参阅：

+ SemanticsBinding.semanticsEnabled，PipelineManifold 实现通常用来支持此属性。

```dart
  bool get semanticsEnabled;
```

## requestVisualUpdate

&emsp;当与此 PipelineManifold 连接的 PipelineOwner 调用时，用于更新与该 PipelineOwner 关联的 RenderObject 对象的视觉外观（即执行 flush 系列函数）。

&emsp;通常，此函数的实现会安排任务（即回调 RendererBinding.drawFrame 函数）来刷新管道的各个阶段（即执行 flush 系列函数）。此函数可能会被快速连续地调用多次。实现应该小心地快速丢弃重复调用。（可能会被多处调用，调用过于频繁，但是仅需要在下一帧统一处理那些 flush 请求即可，例如：在 Render Tree 构建过程中每个节点都会调用此函数，请求进行视觉更新。）

&emsp;与此 PipelineManifold 连接的 PipelineOwner 如果已配置使用非空的 PipelineOwner.onNeedVisualUpdate 回调（看到 RendererBinding 的 rootPipelineOwner 的 onNeedVisualUpdate 是没有配置的，所以请求刷新时，还是用这个方法比较靠谱。），将调用 PipelineOwner.onNeedVisualUpdate 而不是调用此方法。

&emsp;另请参阅：

+ SchedulerBinding.ensureVisualUpdate，通常由 PipelineManifold 实现调用以实现此方法。

```dart
  void requestVisualUpdate();
```

## PipelineManifold 总结

&emsp;其实直白一点理解 PipelineManifold，它就是一个中间层。所有的 PipelineOwner 都指向它，也即是所有的 PipelineOwner 都可以直接访问它，也即是 PipelineOwner 中收集的所有脏 RenderObject 对象都可以访问它（调用它的函数）。

&emsp;当它们 PipelineOwner 想要刷新时，如在 RenderObject 需要 Layout/Paint/CompositedLayerUpdate/SemanticsUpdate/InitialSemantics 时调了五次的 `owner!.requestVisualUpdate();`，此时发出的 requestVisualUpdate 请求都交给了它们的 `PipelineManifold? _manifold` 属性：`_manifold?.requestVisualUpdate();`。那么它们的 `PipelineManifold? _manifold` 属性置在哪里呢？其实有一个全局的 PipelineManifold 默认值。

&emsp;在 RendererBinding 中提供了一个 `late final PipelineManifold _manifold = _BindingPipelineManifold(this);` 变量，`_BindingPipelineManifold` 是 PipelineManifold 的直接子类，所以此 `_manifold` 是一个全局的 PipelineManifold 变量，而它呢在 RendererBinding.initInstances 函数内直接通过：`rootPipelineOwner.attach(_manifold);` 附加到了 PipelineOwner Tree 的根节点。

&emsp;然后当所有的 PipelineOwner 发出 requestVisualUpdate 请求时，都到了 `RendererBinding._manifold` 这里，而在 `_BindingPipelineManifold` 类内部，它的 requestVisualUpdate 函数则是直接调用 RendererBinding 这个 binding 的 `ensureVisualUpdate` 函数调度新的一帧生成。

&emsp;所以最后兜兜转转，通过 PipelineManifold 这个中间层，还是把任务交到了 RendererBinding 手中。

&emsp;下面我们看一下目前全局唯一的一个 PipelineManifold 子类：`_BindingPipelineManifold` 的内容。

# `_BindingPipelineManifold`

&emsp;在 RendererBinding Mixin 中第一个属性是它的单例属性 instance，第二个属性便是：`late final PipelineManifold _manifold = _BindingPipelineManifold(this);`，可见 manifold 的重要性，并且我们看到 `_BindingPipelineManifold` 构造函数是直接把当前的 binding 传进去了，是的没错：`_manifold` 会直接引用这个 RendererBinding.instance。

## Constructors

&emsp;构造函数需要传入一个 binding。然后它继承 ChangeNotifier，以满足 Listenable 接口的要求。

```dart
class _BindingPipelineManifold extends ChangeNotifier implements PipelineManifold {
  _BindingPipelineManifold(this._binding) {
  
    // 添加 semanticsEnabled 值发生变化的监听，当发生变化时，
    // 会回调 PipelineOwner 的 _updateSemanticsOwner 函数。
    _binding.addSemanticsEnabledListener(notifyListeners);
  }
  
  // ...
}
```

## `_binding`

&emsp;最重要的属性，`_BindingPipelineManifold` 类完成的最重要的两个功能，其实都是由此 binding 来实现的。

```dart
  final RendererBinding _binding;
```

## requestVisualUpdate

&emsp;请求视觉刷新，在 PipelineOwner 的刷新阶段中都要调用它。

```dart
  @override
  void requestVisualUpdate() {
    _binding.ensureVisualUpdate();
  }
```

## semanticsEnabled

&emsp;当前 semanticsEnabled 的值，开或者关。

```dart
  @override
  bool get semanticsEnabled => _binding.semanticsEnabled;
```

## dispose

&emsp;当销毁时需要移除监听。

```dart
  @override
  void dispose() {
    // 移除监听。
    _binding.removeSemanticsEnabledListener(notifyListeners);
    
    super.dispose();
  }
```

## `_BindingPipelineManifold` 总结

&emsp;一个超简单的类，大概作为了 Binding 与我们的 RenderObject 对象之间的中间层。特别是对 RenderObject 而言超级重要的请求视觉更新，是通过它传递到了 Binding 中，再传递到 Flutter engine。

&emsp;然后另外一个是当 SemanticsBinding.semanticsEnabled 值变化时，通知到 PipelineOwner 中进行 `_updateSemanticsOwner` 回调，根据开或者关，创建或者销毁 SemanticsOwner，即回调到 PipelineOwner 里是否处理与语义化相关的内容。

&emsp;内容都比较简单，快速浏览即可，但是还是有必要看一下，不断加深对 Binding 的理解，继而掌握 Flutter 整个框架。

## 参考链接
**参考链接:🔗**
+ [PipelineOwner class](https://api.flutter.dev/flutter/rendering/PipelineOwner-class.html)
+ [RendererBinding mixin](https://api.flutter.dev/flutter/rendering/RendererBinding-mixin.html)
+ [PipelineManifold class](https://api.flutter.dev/flutter/rendering/PipelineManifold-class.html)
+ [Listenable class](https://api.flutter.dev/flutter/foundation/Listenable-class.html)
+ [ChangeNotifier class](https://api.flutter.dev/flutter/foundation/ChangeNotifier-class.html)
