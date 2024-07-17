# Flutter 源码梳理系列（二十五）：PipelineManifold

# PipelineManifold

&emsp;PipelineManifold 用于管理一棵 PipelineOwner Tree。

&emsp;树中的所有 PipelineOwner 都附加到同一个 PipelineManifold，这使它们能够访问共享功能，例如请求视觉更新（通过调用 requestVisualUpdate）。因此，PipelineManifold 为 PipelineOwners 提供了通常由 bindings 提供的功能，而不将 PipelineOwners 绑定到特定的 binding 实现。

&emsp;PipelineOwner Tree 的根通过将 PipelineManifold 传递给 PipelineOwner.attach 与 PipelineManifold 相连。当通过 PipelineOwner.adoptChild 收养子级 PipelineOwner 时，子级将附加到与其父级相同的 PipelineManifold。 

&emsp;PipelineOwners 可以在 PipelineManifold 中注册监听器，以在 PipelineManifold 提供的某些值更改时得到通知。

## Constructors

&emsp;PipelineManifold 是一个需要实现 Listenable 的抽象类。即交由它的子类来实现了。

```dart
abstract class PipelineManifold implements Listenable {
  // ...
}
```

## semanticsEnabled

&emsp;PipelineOwners 连接到此 PipelineManifold 是否应收集语义信息并生成语义树。

&emsp;当此属性更改其值时，PipelineManifold 会通知其侦听器（通过 addListener 和 removeListener 管理）。

&emsp;另请参阅：

+ SemanticsBinding.semanticsEnabled，PipelineManifold 实现通常用来支持此属性。

```dart
  bool get semanticsEnabled;
```

## requestVisualUpdate

&emsp;当与此 PipelineManifold 连接的 PipelineOwner 调用时，用于更新与该 PipelineOwner 关联的 RenderObject 的视觉外观（即执行 flush 系列函数）。

&emsp;通常，此函数的实现会安排任务（即回调 RendererBinding.drawFrame 函数）来刷新管道的各个阶段（即执行 flush 系列函数）。此函数可能会被快速连续地调用多次。实现应该小心地快速丢弃重复调用。（可能会被多处调用，调用过于频繁，但是仅需要在下一帧统一处理那些 flush 请求即可，例如：在 Element Tree 和 Render Tree 构建过程中每个节点都会调用此函数。）

&emsp;与此 PipelineManifold 连接的 PipelineOwner 如果已配置使用非空的 PipelineOwner.onNeedVisualUpdate 回调（看到 RendererBinding 的 rootPipelineOwner 的 onNeedVisualUpdate 是没有配置的，所以请求刷新时，还是用这个方法比较靠谱），将调用 PipelineOwner.onNeedVisualUpdate 而不是调用此方法。

&emsp;另请参阅：

+ SchedulerBinding.ensureVisualUpdate，通常由 PipelineManifold 实现调用以实现此方法。

```dart
  void requestVisualUpdate();
```

## PipelineManifold 总结

&emsp;其实直白一点理解 PipelineManifold，它就是一个中间层。所有的 PipelineOwner 都指向它，也即是所有的 PipelineOwner 都可以直接访问它。

&emsp;当它们 PipelineOwner 想要刷新时，如在 RenderObject 需要 Layout/Paint/CompositedLayerUpdate/SemanticsUpdate/InitialSemantics 时调了五次的 `owner!.requestVisualUpdate();`，此时发出的 requestVisualUpdate 请求都交给了它们的 `PipelineManifold? _manifold` 属性：`_manifold?.requestVisualUpdate();`。那么它们的 `PipelineManifold? _manifold` 属性置在哪里呢？其实有一个全局的 PipelineManifold 默认值。

&emsp;在 RendererBinding 中提供了一个 `late final PipelineManifold _manifold = _BindingPipelineManifold(this);` 变量，`_BindingPipelineManifold` 是 PipelineManifold 的直接子类，所以此 `_manifold` 是一个全局的 PipelineManifold 变量，而它呢在 RendererBinding.initInstances 函数内直接通过：`rootPipelineOwner.attach(_manifold);` 附加到了 PipelineOwner Tree 的根节点。

&emsp;然后当所有的 PipelineOwner 发出 requestVisualUpdate 请求时，都到了 `RendererBinding._manifold` 这里，而在 `_BindingPipelineManifold` 类内部，它的 requestVisualUpdate 函数则是直接调用 RendererBinding 这个 binding 的 `ensureVisualUpdate` 函数调度新的一帧生成。

&emsp;所以最后兜兜转转，通过 PipelineManifold 这个中间层，还是把任务交到了 RendererBinding 手中。

# `_BindingPipelineManifold`

&emsp;





## 参考链接
**参考链接:🔗**
+ [PipelineOwner class](https://api.flutter.dev/flutter/rendering/PipelineOwner-class.html)
+ [RendererBinding mixin](https://api.flutter.dev/flutter/rendering/RendererBinding-mixin.html)
+ [PipelineManifold class](https://api.flutter.dev/flutter/rendering/PipelineManifold-class.html)
+ [Listenable class](https://api.flutter.dev/flutter/foundation/Listenable-class.html)
+ [ChangeNotifier class](https://api.flutter.dev/flutter/foundation/ChangeNotifier-class.html)
