# Flutter 源码梳理系列（三十八）：Other Layer

# 前言

&emsp;在前面的学习中我们分别单独学习了具有代表性的：Layer、ContainerLayer、OffsetLayer 以及作为 PaintingContext 的 currentLayer 属性类型的 PictureLayer，而本篇呢我们则把剩余的 Layer 子类统一进行梳理一下。

&emsp;Layer 是所有 Layer 子类的抽象基类，其内部提供了构建 Layer Tree 时 Layer 节点所要使用到的指针，如：`ContainerLayer? _parent;`、`Layer? _nextSibling;`、`Layer? _previousSibling;`，以及 Layer 关于 Scene 的相关逻辑：如：

+ 控制 Layer 是否要添加到 Scence 的 `_needsAddToScene` 属性，它用来标识该 Layer 自上次被渲染后是否发生了变化，如果发生变化的话需要被重新渲染，此值默认为 true。、
+ 标记需要添加到 Scene 的：`markNeedsAddToScene` 函数，如果 Layer 需要重新进行渲染的话会调用此函数，而不是说直接去修改 `_needsAddToScene` 属性，、
+ 控制是否总是需要添加到 Scene 的 `alwaysNeedsAddToScene` getter、
+ 更新 Layer 子树是否需要添加到 Scene 的：`updateSubtreeNeedsAddToScene` 函数、
+ 待 Layer 子类各自实现的添加到 Scene 时所调用的抽象函数：`addToScene`，各 Layer 子类在自己的 `addToScene` 函数内实现自己的自定义操作，
+ 以及 Layer 子类的 `addToScene` 函数内会被进行赋值的，用于记录 Layer 渲染结果的：`ui.EngineLayer? _engineLayer;` 属性，
+ 以及最最重要的：`addToScene` 函数内部所使用的函数：`_addToSceneWithRetainedRendering`，它内部控制了 Layer 的渲染结果如何被复用的逻辑，即：当 Layer 的 `_needsAddToScene` 属性为 false 且 `_engineLayer` 属性不为 null 的话，则本次渲染直接复用之前的渲染结果 `_engineLayer` 属性，否则话还是进行重新渲染。

&emsp;在上面我们看到 Layer 的 parent 指针是：`ContainerLayer? _parent;`，即所有 Layer 节点的父级都是 ContainerLayer 类型，而这也体现了一个很重要的知识点：**只有 ContainerLayer 才能有一组子级 Layer，** 且 ContaienrLayer 作为 Layer 抽象基类的直接子类提供了一组函数：`void append(Layer child)`、`void _adoptChild(Layer child)`、`void _removeChild(Layer child)`、`void _dropChild(Layer child)`、`void removeAllChildren()` 和两个指针：`Layer? _firstChild;`、`Layer? _lastChild;`，正是通过它们为 Layer Tree 提供了基本的构建和管理逻辑。

&emsp;上面总结了 Layer 和 ContainerLayer 的核心功能，那么剩余的 Layer 子类都有哪些具体的自定义操作呢？

&emsp;此时我们可以打开 layer.dart 文件浏览一下，所有的 Layer 子类都定义在了这个文件内。在 Layer 和 ContainerLayer 之间定义的 Layer 子类，它们都是直接继承自 Layer 的，并且它们都是没有子级 Layer 的，而剩余的 Layer 子类也都是 ContainerLayer 的子类，它们则都是可以有子级 Layer 的。

&emsp;所以统一根据 Layer 是否有子级 Layer 列表，可以把所有的 Layer 分为两类：

+ ContainerLayer 可以拥有一组子级 Layer，并会统一管理这一组子级 Layer 的渲染特点。
+ 非 ContainerLayer 的叶子 Layer，代表：PictureLayer 和 TextureLayer。

&emsp;下面我们梳理一下 layer.dart 文件中定义的 Layer 子类，重点则都在它们重写的 addToScene 函数内部。

# LayerHandle

&emsp;LayerHandle 泛型类作为 RencerObject 的 `_layerHandle` 属性的类型所使用：`final LayerHandle<ContainerLayer> _layerHandle = LayerHandle<ContainerLayer>();`，然后全局搜索 .layer，会发现，虽然 RenderObject 对象有一个 `ContainerLayer? get layer` getter，但是所有的读取或者赋值 RenderObject 对象的 layer 的操作都是通过：`._layerHandle.layer` 进行的。

&emsp;LayerHandle 是一个用来防止 Layer 的平台图形资源被释放的句柄。

&emsp;Layer 对象保留原生资源，比如 EngineLayers 和 Picture 对象。这些对象可能会间接或直接保留大块的纹理内存。

&emsp;只要有能够将其添加到 Scene 中的对象在，Layer 的 native 资源必须被保留。通常情况下，这个对象要么是其 Layer.parent，要么是一个未被释放的 RenderObject，它将其附加到一个 ContainerLayer 上。Layer 会自动保留其子级 Layer 的一个句柄，RenderObjects 会自动保留指向其 RenderObject.layer 和使用 PaintingContext.canvas 画入的任何 PictureLayers 的句柄。一旦获得了至少一个句柄并且所有句柄都被释放，Layer 会自动释放其资源。创建额外 Layer 对象的 RenderObjects 必须像 RenderObject.layer 的实现那样手动管理该 Layer 的句柄。

&emsp;RenderObject.layer 会自动被管理。

&emsp;如果一个 RenderObject 除了它的 RenderObject.layer 外还创建了额外 Layer，而且打算独立地重用这些 Layer，它必须创建该 Layer 的句柄，并在不再需要该 Layer 时进行释放。例如，如果在 RenderObject.paint 中重新创建或将现有 Layer 置空，应该释放旧 layer 的句柄。它还应释放 RenderObject.dispose 中持有的任何 Layer 句柄。

&emsp;要释放 LayerHandle，将其的 layer 属性设置为 null。

## Constructors

&emsp;可看到 LayerHandle 的泛型 T 必须继承自 Layer。创建一个新的 LayerHandle，如果 layer 参数不为 null 的话，则默认把其 refCount 加 1。

```dart
class LayerHandle<T extends Layer> {
  /// 创建一个新的 Layer 句柄，可选择引用一个 [Layer]。
  LayerHandle([this._layer]) {
    
    // 如果 _layer 参数不为 null，则默认把其引用计数 refCount 加 1
    if (_layer != null) {
      _layer!._refCount += 1;
    }
  }
  // ...
}
```

## layer

&emsp;该 LayerHandle 对象使其资源处于活动状态的 Layer。设置一个新值或 null 将会清除先前持有的 Layer，如果该 Layer 没有其他打开的句柄。

```dart
  T? _layer;
  T? get layer => _layer;
  
  set layer(T? layer) {
  
    // 如果入参 layer 已经被调用过 dispose 函数的话，则不能用于创建 LayerHandle 对象。
    
    // 如果入参 layer 和当前 LayerHandle 对象的 layer 属性是同一个，则直接 return 即可。
    if (identical(layer, _layer)) {
      return;
    }
    
    // 首先调用旧 layer 的 unref 函数，即表示当前的 LayerHandle 对象不再使用旧的 layer 属性了。
    _layer?._unref();
    
    // 赋值
    _layer = layer;
    
    // 然后更新 _layer 属性的 refCount 的值，加 1。
    if (_layer != null) {
      _layer!._refCount += 1;
    }
  }
```

## LayerHandle 总结

&emsp;如 RenderObject 类中对 LayerHandle 类的使用，即保证了 Layer 资源的正确使用和正确销毁。

# PictureLayer

&emsp;PictureLayer 一个包含 Picture 的合成图层。

&emsp;PictureLayer 始终是 Layer Tree 中的叶子节点。PictureLayer 还负责处理它们所持有的 Picture 对象的销毁，通常情况下，当它们的父级以及参与绘制 Picture 的所有 RenderObjects 都已被销毁时，这个销毁过程会被执行。

+ Object -> Layer -> PictureLayer

## picture

&emsp;picture 是这该 PictureLayer 记录的 Picture 对象。picture 的坐标系与该 PictureLayer 的坐标系匹配。

&emsp;在更改此属性后必须显式地重新合成场景（如在 Layer 中描述的那样）。

```dart
  ui.Picture? get picture => _picture;
  ui.Picture? _picture;
  
  set picture(ui.Picture? picture) {
  
    // 标记当前 PictureLayer 对象的 _needsAddToScene 属性为 true，
    // 即表示下一帧 PictureLayer 对象需要重新渲染。
    markNeedsAddToScene();
    
    // 销毁旧的 picture
    _picture?.dispose();
    
    // 持有新的 picture
    _picture = picture;
  }
```

## addToScene

&emsp;重写 addToScene 方法，以将此 PictureLayer 上传到引擎。把该 PictureLayer 对象的 picture 添加到场景中去。

&emsp;SceneBuilder 的 addPicture 方法，用于向 Scene 中添加一幅图片（Picture 对象）。Picture 根据给定的偏移（默认是 Offset.zero）进行光栅化处理。

&emsp;如果 Picture 在后续帧中会重复使用，渲染可能会被缓存以降低绘制该 Picture 的成本。Picture 是否被缓存取决于后端实现。当考虑到缓存时，是否对 Picture 进行缓存取决于 Picture 绘制的频率和绘制成本。要禁用此缓存，请将 willChangeHint 参数设置为 true。要强制缓存（在支持缓存的后端中），请将 isComplexHint 参数设置为 true。当两者都设置时，willChangeHint 优先。

&emsp;一般来说，设置这些提示并不是非常有用的。支持缓存 Picture 的后端仅会对已经渲染过三次的 Picture 进行缓存；因此，将 willChangeHint 设置为 true 以避免缓存一个每一帧都会更改的动态 Picture 是多余的，这种 Picture 本来就不会被缓存。同样，支持缓存 Picture 的后端在这方面相对积极，因此任何足够复杂以需要缓存的 Picture 可能已经被缓存，即使没有设置 isComplexHint 为 true。

```dart
  @override
  void addToScene(ui.SceneBuilder builder) {
    // 直接调用 SceneBuilder 的 addPicture 函数，可看到 PictureLayer 作为叶子节点，
    // 并不会像 ContainerLayer 的子类那样，在自己的 addToScene 函数内，会记录 SceneBuilder 调用 pushXXX 返回的 EngineLayer 对象，
    // 并会递归调用自己的所有子级 Layer 的 addToScene 函数。
    builder.addPicture(Offset.zero, picture!, isComplexHint: isComplexHint, willChangeHint: willChangeHint);
  }
```

# TextureLayer

&emsp;TextureLayer 是一种将后端纹理映射到矩形的合成图层。

&emsp;后端纹理是可以应用（映射）到 Flutter 视图区域的图像。它们是使用特定于平台的纹理注册表创建、管理和更新的。通常，这是由一个插件完成的，该插件与主机平台的视频播放器、摄像头或OpenGL API，或类似的图像源进行集成。

&emsp;TextureLayer 使用整数 ID 引用其后端纹理。纹理 ID 是从纹理注册表获取的，并作用域限定在 Flutter 视图中。纹理 ID 可以在注销后重新使用，由注册表自行决定。使用注册表当前未知的纹理 ID 将会导致一个空白矩形。

&emsp;一旦插入到 Layer Tree 中，TextureLayer 会根据后端（例如，视频帧到达时）自主重绘。这种重绘通常不涉及执行 Dart 代码。

&emsp;TextureLayer 始终是 Layer Tree 中的叶子节点。

+ Object -> Layer -> TextureLayer

## addToScene

&emsp;重写 addToScene 方法，以将此 TextureLayer 上传到引擎。把该 PictureLayer 对象的 picture 添加到场景中去。

&emsp;SceneBuilder 的 addTexture 方法，用于向 Scene 中添加一个后端纹理。

&emsp;该纹理被缩放到给定的大小，并在给定的偏移处进行栅格化。

&emsp;如果 freeze 为 true，则添加到 Scene 中的纹理不会更新为新的帧。当调整嵌入式 Android 视图的大小时，会使用 freeze：调整 Android 视图大小时有一个短暂的时间段，在此期间框架无法确定最新的纹理帧是具有前一个大小还是新的大小，为解决此问题，框架在调整 Android 视图大小之前 "freezes" 纹理，当确定具有新大小的帧已准备就绪时再解除冻结。

```dart
  @override
  void addToScene(ui.SceneBuilder builder) {
    // 直接调用 SceneBuilder 的 addTexture 函数
    builder.addTexture(
      textureId,
      offset: rect.topLeft,
      width: rect.width,
      height: rect.height,
      freeze: freeze,
      filterQuality: filterQuality,
    );
  }
``` 

# ClipRectLayer

&emsp;ClipRectLayer 是一个使用矩形裁剪其子级 Layer 的复合图层。

+ Object -> Layer -> ContainerLayer -> ClipRectLayer

## addToScene

&emsp;重写 addToScene 方法，以将此 ClipRectLayer 上传到引擎。

&emsp;可看到 ContainerLayer 的 addToScene 和上面的非 ContainerLayer 有很大的不同，首先是会使用 engineLayer 属性记录下当前 builder.pushClipRect 的返回值，即当前 ClipRectLayer 的渲染结果，然后会调用 addChildrenToScene 函数，在自己的子级 Layer 列表中递归调用 addToScene 函数，然后最后执行 builder.pop，把通过 builder.pushClipRect 添加的裁剪操作出栈，防止影响当前的 SceneBuilder 进行后续的 ContainerLayer 的渲染操作。

&emsp;当前 ClipRectLayer 对象的所有子级 Layer 都会在它的裁剪区域进行渲染。 

```dart
  @override
  void addToScene(ui.SceneBuilder builder) {
    bool enabled = true;
    
    engineLayer = builder.pushClipRect(
      clipRect!,
      clipBehavior: clipBehavior,
      oldLayer: _engineLayer as ui.ClipRectEngineLayer?,
    );
    
    addChildrenToScene(builder);
    builder.pop();
  }
```

&emap;然后是如出一辙的：

+ ClipRRectLayer：是一个使用圆角矩形裁剪其子级 Layer 的复合图层。
+ ClipPathLayer：是一个使用路径裁剪其子级 Layer 的复合图层。

# ColorFilterLayer

&emsp;ColorFilterLayer 是一个将 ColorFilter 应用到其子级 Layer 的组合图层。

+ Object -> Layer -> ContainerLayer -> ColorFilterLayer

## addToScene

&emsp;可看到如上 ContainerLayer 子类一样的流程：首先通过 builder.pushColorFilter 调用在当前的 SceneBuilder 堆栈中添加颜色滤镜的绘制操作，然后开始在其子级 Layer 列表中递归调用 addToScene 函数，然后最后再执行 builder.pop 操作，把 builder.pushColorFilter 添加的颜色滤镜绘制操作出栈。

```dart
  @override
  void addToScene(ui.SceneBuilder builder) {
  
    engineLayer = builder.pushColorFilter(
      colorFilter!,
      oldLayer: _engineLayer as ui.ColorFilterEngineLayer?,
    );
    
    addChildrenToScene(builder);
    
    builder.pop();
  }
```

&emsp;然后是如出一辙的：

+ ImageFilterLayer：是一个将 ImageFilter 应用到其子级 Layer 的组合图层。
+ TransformLayer：是一个将 Transform 应用到其子级 Layer 的组合图层。
+ OpacityLayer：是一个将 Opacity 应用到其子级 Layer 的组合图层。
+ ShaderMaskLayer：是一个将 Shader 应用到其子级 Layer 的组合图层。
+ BackdropFilterLayer：是一个将 ImageFilter、BlendMode 应用到其子级 Layer 的组合图层。

&emsp;ContainerLayer 和非 ContainerLayer 的 addToScene 函数内部区别还是比较明显的，内容整体看下来并不难，快速浏览即可。

## 参考链接
**参考链接:🔗**
+ [OffsetLayer class](https://api.flutter.dev/flutter/rendering/OffsetLayer-class.html)
+ [`LayerHandle<T extends Layer> class`](https://api.flutter.dev/flutter/rendering/LayerHandle-class.html)
+ [PictureLayer class](https://api.flutter.dev/flutter/rendering/PictureLayer-class.html)
+ [TextureLayer class](https://api.flutter.dev/flutter/rendering/TextureLayer-class.html)
+ [ClipRectLayer class](https://api.flutter.dev/flutter/rendering/ClipRectLayer-class.html)
+ [ClipRRectLayer class](https://api.flutter.dev/flutter/rendering/ClipRRectLayer-class.html)
+ [ClipPathLayer class](https://api.flutter.dev/flutter/rendering/ClipPathLayer-class.html)
+ [ColorFilterLayer class](https://api.flutter.dev/flutter/rendering/ColorFilterLayer-class.html)
+ [ImageFilterLayer class](https://api.flutter.dev/flutter/rendering/ImageFilterLayer-class.html)
+ [TransformLayer class](https://api.flutter.dev/flutter/rendering/TransformLayer-class.html)
+ [OpacityLayer](https://api.flutter.dev/flutter/rendering/OpacityLayer-class.html)
