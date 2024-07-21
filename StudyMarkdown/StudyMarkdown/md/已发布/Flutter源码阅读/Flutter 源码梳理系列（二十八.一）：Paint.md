# Flutter 源码梳理系列（二十八.一）：final class Paint

# Paint

&emsp;Paint：在绘制 Canvas 时要使用的样式的描述。（例如一组样式特征包括：颜色、线条宽度、线条转角时的风格、线条结尾时的风格、颜色滤镜、图片滤镜 等等。）

&emsp;Canvas 上的大多数 API 都需要一个 Paint 对象来描述用于该绘制操作的样式。

## Constructors

&emsp;创建一个空的 Paint 对象，其中所有字段（每个字段代表其中一个样式特征的值）均初始化为它们的默认值。

&emsp;可看到 Paint 是一个 final 修饰的类，直接说明了它没有子类，并且它不是一个抽象类，我们直接创建 Paint 对象，直接使用即可。

```dart
final class Paint {
  Paint();
  
  // ...
}
```

## from

&emsp;使用与入参 Paint other 对象相同的字段（每个字段代表其中一个样式特征的值）构造一个新的 Paint 对象。

&emsp;对返回的 Paint 对象所做的任何更改都不会影响到 other，而对 other 的任何更改也不会影响到返回的 Paint 对象。

&emsp;Backends（例如 web vs native）可能具有不同的性能特征。如果代码对性能敏感，考虑进行性能分析，并在必要时回退到重用单个 Paint 对象。

```dart
  Paint.from(Paint other) {
    // Paint 中的每个字段都是深度不可变的，因此要创建 Paint 对象的副本，
    // 我们需要复制底层数据缓冲区（_data.buffer）和对象列表（_objects）（这些对象也是深度不可变的）。
    
    // 把入参 other 的 _data.buffer 和 _objects 的值深拷贝给新建的 Paint 对象。
    _data.buffer.asUint32List().setAll(0, other._data.buffer.asUint32List());
    _objects = other._objects?.toList();
  }
```

## `_data`

&emsp;Paint 的 objects 在两个 buffers 中进行编码：

+ `_data` 是以四字节字段为单位的二进制数据，每个字段可以是 uint32_t 或 float 类型。每个字段的默认值被编码为零，以使初始化变得简单。大多数值已经具有零的默认值，但一些值，例如颜色，具有非零的默认值。要编码或解码这些值，请将值与默认值进行异或运算。

+ `_objects` 是一个包含无法编码的对象的列表，通常是 native 对象的包装器。这些对象被简单地存储在列表中，无需进行任何额外的编码。

&emsp;二进制格式必须与 paint.cc 中的反序列化代码匹配。C++ 单元测试会访问它。

&emsp;在 `_data` 中每 4 个字节存储 Paint 对象的一个样式特征点的值。如：是否抗锯齿、颜色（画笔颜色/填充颜色）、混合模式、绘制风格、画笔的线条宽度 等等。它们一个样式特征的值占用 4 个字节，连续被保存在 52 个字节中，拢共有 12 个属性，我们下面会一一学习。

```dart
  // 如果添加更多字段，请记住更新 _kDataByteCount。（是的，当前是共 52 / 4 = 13 个字段，一个字段代表一个特征值。）
  static const int _kDataByteCount = 52; // 4 * (last index + 1).
  
  // _data 中以四字节段为单位，那么现在是 52 字节，即 _data 中可以保存  13 个属性的值。
  
  @pragma('vm:entry-point')
  final ByteData _data = ByteData(_kDataByteCount);
```

## `_objects`

&emsp;目前默认是长度为 3 的 List，内部总共保存 3 个 Object：

+ `_kShaderIndex = 0`：保存一个 Shader，描边（stroking）或填充形状（filling a shape）时使用的着色器。
+ `_kColorFilterIndex = 1`：保存一个 ColorFilter，绘制形状或进行图层合成时要应用的颜色滤镜。
+ `_kImageFilterIndex = 2`：保存一个 ImageFilter，在绘制光栅图像时使用的 ImageFilter（图片滤镜）。

```dart
  // 二进制格式必须与 paint.cc 中的反序列化代码匹配。C++ 单元测试会访问它。
  @pragma('vm:entry-point')
  List<Object?>? _objects;

  static const int _kObjectCount = 3; // Must be one larger than the largest index.
  
  List<Object?> _ensureObjectsInitialized() {
    return _objects ??= List<Object?>.filled(_kObjectCount, null);
  }
```

## `_kFakeHostEndian`

&emsp;如果我们确实在大端机器上运行，我们需要在这里做一些更智能的处理。我们没有使用 Endian.Host，因为它不是一个编译时常量，无法在 set/get 调用中传播。

&emsp;这个 const 主要表示当前运行的机器是大端还是小端。下面👇从 `_data` 的 ByteData 的指定位取出的数据，转换为 Int32 的值时，要根据这个大端还是小端来决定。

```dart
const Endian _kFakeHostEndian = Endian.little;
```

&emsp;OK，从下面开始，我们依次看看这共 52 个字节的数据，每 4 个字节是一个特征点的值。下面我们依次看看每个特征点都代表着什么。

## isAntiAlias

&emsp;在 Canvas 上绘制的线条（lines）和图像（images）是否应用抗锯齿技术。默认是 true。

&emsp;isAntiAlias 的值保存在 `_data` 的前 4 个字节中。

```dart
  // 0（0x0） 左移 2 位还是 0，即 _kIsAntiAliasOffset 是 0
  static const int _kIsAntiAliasIndex = 0;
  static const int _kIsAntiAliasOffset = _kIsAntiAliasIndex << 2;
  
  // 从位于偏移量 0 的字节开始，从 ByteData 对象 _data 中获取一个 32 位整数。
  // 即取 _data 前 4 个字节的数据。
  
  // 1️⃣ 第 1 个属性，位于前 4 个字节。
  bool get isAntiAlias {
    return _data.getInt32(_kIsAntiAliasOffset, _kFakeHostEndian) == 0;
  }
  
  // 同上设置时也是存放在 _data 的前 4 个字节。
  set isAntiAlias(bool value) {
  
    // 我们将 true 编码为 0，将 false 编码为 1，因为默认值总是被编码为零，而默认值是真。
    final int encoded = value ? 0 : 1;
    
    _data.setInt32(_kIsAntiAliasOffset, encoded, _kFakeHostEndian);
  }
```

&emsp;估计看完这第一个特征值心里会犯嘀咕，就为了存一个 bool 值占用了 4 个字节。

## color

&emsp;在描边（stroking）或填充形状（filling a shape）时要使用的颜色。默认值是不透明的黑色。（这里的描边还是填充形状其实是代表了不同的绘制目的：例如我们要画一个圆环或者实心圆的时候，我们可以用同样的一个圆形 Path，然后我们就可以选择不同的绘制风格（即：PaintingStyle style，下面会学习。）来让绘制结果是圆环还是实心圆，然后这个 color 就代表了圆环的颜色或者实心圆的颜色。）

&emsp;这个 color 在合成时（compositing）未被使用。要对一个图层（layer）上色，可以使用 colorFilter。

```dart
  // Must be kept in sync with the default in paint.cc.
  static const int _kColorDefault = 0xFF000000;
  
  // 1（0x1） 左移 2 位是 4（0x100），即 _kColorOffset 是 4
  static const int _kColorIndex = 1;
  static const int _kColorOffset = _kColorIndex << 2;
  
  // 从位于偏移量 4 的字节开始，从 ByteData 对象 _data 中获取一个 32 位整数。
  
  // 2️⃣ 同上：第 2 个属性，位置也来到了第 4 位字节
  Color get color {
    final int encoded = _data.getInt32(_kColorOffset, _kFakeHostEndian);
    return Color(encoded ^ _kColorDefault);
  }
  
  // 同上设置时也是存放在 _data 的前 4 个字节后再偏 4 个字节。
  set color(Color value) {
    final int encoded = value.value ^ _kColorDefault;
    _data.setInt32(_kColorOffset, encoded, _kFakeHostEndian);
  }
```

&emsp;颜色值由 ARGB 组成，占用 4 个字节，没有什么问题。

## blendMode

&emsp;在绘制形状（shape）或合成图层（a layer is composited）时应用的混合模式（blend mode）。默认值是 BlendMode.srcOver。

&emsp;源颜色（source colors）来自正在绘制的形状（例如来自 Canvas.drawPath）或者正在合成的图层（layer being composited）（在 Canvas.saveLayer 和 Canvas.restore 调用之间绘制的图形），如果有应用了 colorFilter，则源颜色（source colors）经过其处理。

&emsp;目标颜色（destination colors）来自形状（shape）或图层（layer）将要合成到的背景上。

&emsp;直白一点理解 blendMode，就是当上层和下层两个像素合并在一起时，用何种算法来决定合成后显示出来的新像素的颜色。源颜色（source colors）可以理解为前景的颜色，目标颜色（destination colors）可以理解为背景板的颜色。把前景和背景的像素值合并在一起。

&emsp;另请参阅：

+ Canvas.saveLayer 使用其 Paint 的 blendMode 来在调用 Canvas.restore 时对图层进行合成。
+ BlendMode 讨论了在使用 Canvas.saveLayer 时如何使用 blendMode。

```dart
  // Must be kept in sync with the default in paint.cc.
  // 默认值即 BlendMode.srcOver 枚举值的 index 值。
  static final int _kBlendModeDefault = BlendMode.srcOver.index;
  
  // 2（0x10） 左移 2 位是 8（0x1000），即 _kBlendModeOffset 是 8
  static const int _kBlendModeIndex = 2;
  static const int _kBlendModeOffset = _kBlendModeIndex << 2;
  
  // 3️⃣ 同上：第 3 个属性，位置也来到了第 8 位字节
  BlendMode get blendMode {
    final int encoded = _data.getInt32(_kBlendModeOffset, _kFakeHostEndian);
    
    return BlendMode.values[encoded ^ _kBlendModeDefault];
  }
  
  set blendMode(BlendMode value) {
    final int encoded = value.index ^ _kBlendModeDefault;
    
    _data.setInt32(_kBlendModeOffset, encoded, _kFakeHostEndian);
  }
```

&emsp;BlendMode.srcOver 它表示最直观的情况，形状被绘制在底部内容的上层，透明区域显示目标图层。这对应于 "Source over Destination" Porter-Duff 运算器，也称为 Painter's Algorithm（绘图算法）。如下效果：（大概可以理解为源颜色，直接覆盖在目标颜色上面。）

![image.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/1ef36f04f1a04f1ab1e7dceb21744c69~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1721634297&x-orig-sign=4P%2F9JIqKOjDF20wJ1ICNLO%2B5r44%3D)

&emsp;关于其它的 BlendMode enum 值，我们可以看一下 BlendMode enum 枚举的内容。

### BlendMode enum

&emsp;BlendMode enum 表示了一组在 Canvas 上绘制时要使用的算法。

&emsp;在将形状（shape）或图像（image）绘制到 Canvas 上时，可以使用不同的算法来混合像素。BlendMode 的不同值指定了不同的算法。

&emsp;每个算法都有两个输入，即源图像（正在绘制的图像）和目标图像（源图像将被复合到其中的图像）。目标图像通常被视为背景。源图像和目标图像都有四个颜色通道，即红色、绿色、蓝色和 alpha 通道。这些通道通常用范围在 0.0 到 1.0 之间的数字表示。算法的输出也具有相同的四个通道，其中的值是根据源图像和目标图像计算得到的。

&emsp;每个值的文档描述了算法的工作原理。在每种情况下，一个图像展示了将源图像与目标图像混合的输出。在以下图像中，目标由具有水平线和不透明风景照片的图像表示，而源由具有垂直线（相同但旋转了）和鸟剪贴画图像的图像表示。src 模式仅显示源图像，而 dst 模式仅显示目标图像。在下方文档中，透明度由棋盘格图案来进行说明。clear 模式清除源和目标，导致输出完全透明的结果（由实心棋盘格图案来说明）。

&emsp;这些图像中的水平和垂直条显示了红色、绿色和蓝色通道以不同不透明度级别，然后是所有三个颜色通道在相同的不透明度级别下，然后是所有三个颜色通道都设置为零的情况下，以相同的不透明度级别，然后是两个显示红/绿/蓝重复渐变的条，第一个完全不透明，第二个为部分不透明，最后是一个条，其中三个颜色通道设置为零，但透明度以重复的渐变变化。

&emsp;当使用 Canvas.saveLayer 和 Canvas.restore 时，会在 Canvas.restore 被调用时应用给定的 Canvas.saveLayer 的 Paint 对象的混合模式。每次调用 Canvas.saveLayer 都会引入一个新的图层（layer），用于绘制形状和图像；当调用 Canvas.restore 时，该图层将会与父图层进行合成，其中源是最近绘制的形状和图像，目的地是父图层。（对于第一次的 Canvas.saveLayer 调用，父图层就是 Canvas 本身。）

+ Object -> Enum -> BlendMode

&emsp;BlendMode enum 具体的枚举值就不一一列举了，可以直接看这个链接🔗：[BlendMode enum](https://api.flutter.dev/flutter/dart-ui/BlendMode.html)。

## style

&emsp;是在形状（shapes）内部绘制、形状边缘绘制，还是两者都绘制。默认值是 PaintingStyle.fill。

```dart
  // 3（0x11） 左移 2 位是 12（0x1100），即 _kStyleOffset 是 12
  static const int _kStyleIndex = 3;
  static const int _kStyleOffset = _kStyleIndex << 2;
  
  // 4️⃣ 同上：第 4 个属性，位置也来到了第 12 位字节
  PaintingStyle get style {
    return PaintingStyle.values[_data.getInt32(_kStyleOffset, _kFakeHostEndian)];
  }
  
  set style(PaintingStyle value) {
    final int encoded = value.index;
    _data.setInt32(_kStyleOffset, encoded, _kFakeHostEndian);
  }
```

### PaintingStyle enum

&emsp;在 Canvas 上绘制形状（shapes）和路径（paths）的策略。（即上面👆说的绘制圆环还是实心圆，可以由 PaintingStyle 来决定。）

+ PaintingStyle.fill：将颜色应用于形状的内部。例如，当应用于 Canvas.drawCircle 调用时，结果是一个给定大小的圆盘被涂抹。
+ PaintingStyle.stroke：将颜色应用到形状的边缘。例如，当应用于 Canvas.drawCircle 调用时，结果是给定大小的环被涂上颜色。绘制在边缘的线条宽度将由 Paint.strokeWidth 属性给定。

## strokeWidth

&emsp;在 PaintingStyle.stroke 的绘制样式下绘制在边缘的线条的宽度。宽度以逻辑像素表示，沿路径垂直方向测量。默认为 0.0，对应于一条发丝线宽度。

```dart
  static const int _kStrokeWidthIndex = 4;
  static const int _kStrokeWidthOffset = _kStrokeWidthIndex << 2;

  // 5️⃣ 同上：第 5 个属性，位置也来到了第 16 位字节
  double get strokeWidth {
    return _data.getFloat32(_kStrokeWidthOffset, _kFakeHostEndian);
  }
  
  set strokeWidth(double value) {
    final double encoded = value;
    _data.setFloat32(_kStrokeWidthOffset, encoded, _kFakeHostEndian);
  }
```

## strokeCap

&emsp;当 PaintingStyle style 设置为 PaintingStyle.stroke 时，在绘制线条末端时放置的收尾效果。默认设置为 StrokeCap.butt，即无效果。

```dart
  static const int _kStrokeCapIndex = 5;
  static const int _kStrokeCapOffset = _kStrokeCapIndex << 2;
  
  // 6️⃣ 同上：第 6 个属性，位置也来到了第 20 位字节
  StrokeCap get strokeCap {
    return StrokeCap.values[_data.getInt32(_kStrokeCapOffset, _kFakeHostEndian)];
  }
  
  set strokeCap(StrokeCap value) {
    final int encoded = value.index;
    _data.setInt32(_kStrokeCapOffset, encoded, _kFakeHostEndian);
  }
```

### StrokeCap enum

&emsp;用于 line 结尾的样式。

+ StrokeCap.butt：使用平坦边缘和无延伸结束绘制轮廓。与 StrokeCap.square 相比，它具有相同的形状，但 StrokeCap.square 会延伸到 line 的末端，超出半个笔画宽度（strokeWidth）。如下示意图：

![image.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/8a2ef59f7c86480bbb787afde5d2eacb~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1721638150&x-orig-sign=l9rTWqXuGrwIHszWyjlINLThtPg%3D)

+ StrokeCap.round：用一个半圆形扩展来开始和结束轮廓。如下示意图中，cap 被着色以突出显示：在正常使用时，它与 line 的颜色相同。如下示意图：

![image.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/1e1c81bb22084a16a9eaa9493d818f05~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1721638212&x-orig-sign=85sBS8ArhQMqB4Yfg8UHnwgqzCE%3D)

+ StrokeCap.square：使用半方形延伸来开始和结束轮廓。这类似于将每个轮廓延伸一半的画笔宽度（由 Paint.strokeWidth 给定）。如下示意图中，对 cap 进行了着色以突出显示它：在正常使用时，它与 line 的颜色相同。与 StrokeCap.butt 相比，它们形状相同，但 StrokeCap.butt 不会延伸到 line 的末端。如下示意图：

![image.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/73555d8cc71e48e692db63aa5b83de25~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1721638268&x-orig-sign=z8qkPZRslt0K1ozVUOITZHd4kic%3D)

## strokeJoin

&emsp;在各段 lines 之间放置的完成形式。这适用于当 PaintingStyle style 设置为 PaintingStyle.stroke 时绘制的 lines，不适用于使用 Canvas.drawPoints 绘制为线的点。默认为 StrokeJoin.miter，即尖角。

```dart
  static const int _kStrokeJoinIndex = 6;
  static const int _kStrokeJoinOffset = _kStrokeJoinIndex << 2;

  // 7️⃣ 同上：第 7 个属性，位置也来到了第 24 位字节
  StrokeJoin get strokeJoin {
    return StrokeJoin.values[_data.getInt32(_kStrokeJoinOffset, _kFakeHostEndian)];
  }
  
  set strokeJoin(StrokeJoin value) {
    final int encoded = value.index;
    _data.setInt32(_kStrokeJoinOffset, encoded, _kFakeHostEndian);
  }
```

&emsp;关于 StrokeJoin enum 枚举值不同的效果，可以参考这个页面：[strokeJoin property](https://api.flutter.dev/flutter/dart-ui/Paint/strokeJoin.html)

## strokeMiterLimit

&emsp;当各段 lines 连接处被设置为 StrokeJoin.miter 并且当 PaintingStyle style 设置为 PaintingStyle.stroke 时，描边线段上可以绘制斜接线的限制。如果超过了这个限制，那么将绘制 StrokeJoin.bevel 类型的连接。如果各 lines 线段之间的角度发生动画变化，可能会导致角点 'popping'。

&emsp;这个限制可以表达为斜角线长度的限制。默认为 4.0。将限制设为零会导致始终使用 StrokeJoin.bevel 接合。

&emsp;可参考这个页面的视频效果：[strokeMiterLimit property](https://api.flutter.dev/flutter/dart-ui/Paint/strokeMiterLimit.html)

```dart
  static const int _kStrokeMiterLimitIndex = 7;
  static const int _kStrokeMiterLimitOffset = _kStrokeMiterLimitIndex << 2;
  
  // 8️⃣ 同上：第 8 个属性，位置也来到了第 28 位字节
  double get strokeMiterLimit {
    return _data.getFloat32(_kStrokeMiterLimitOffset, _kFakeHostEndian);
  }
  
  set strokeMiterLimit(double value) {
    final double encoded = value - _kStrokeMiterLimitDefault;
    _data.setFloat32(_kStrokeMiterLimitOffset, encoded, _kFakeHostEndian);
  }
```

## filterQuality

&emsp;控制性能与质量之间的权衡，用于对位图进行采样，如 ImageShader，或在绘制图像时，如：
 
+ Canvas.drawImage，
+ Canvas.drawImageRect，
+ Canvas.drawImageNine，
+ Canvas.drawAtlas。

&emsp;默认值是 FilterQuality.none。

```dart
  static const int _kFilterQualityIndex = 8;
  static const int _kFilterQualityOffset = _kFilterQualityIndex << 2;
  
  // 9️⃣ 同上：第 9 个属性，位置也来到了第 32 位字节
  FilterQuality get filterQuality {
    return FilterQuality.values[_data.getInt32(_kFilterQualityOffset, _kFakeHostEndian)];
  }
  
  set filterQuality(FilterQuality value) {
    final int encoded = value.index;
    _data.setInt32(_kFilterQualityOffset, encoded, _kFakeHostEndian);
  }
```

### FilterQuality enum

&emsp;图像采样在采样图像的 ImageFilter 和 Shader 对象以及渲染图像的 Canvas 操作中的质量水平。

&emsp;通常，在放大图像时，质量最低为 none，较高为 low 和 medium，对于非常大的放大比例（超过 10 倍），质量最高为 high。

&emsp;在缩小图像时，medium 提供了最佳质量，特别是在将图像缩小到一半以下或对这些缩小之间的比例因子进行动画处理时。否则，low 和 high 为缩小 50% 到 100% 之间提供了类似的效果，但图像可能会失去细节，并在 50% 以下出现漏失。

&emsp;在缩放图像放大和缩小，或者缩放比例未知的情况下，选择 medium 中等质量通常是一个良好的平衡选择，以获得高质量的结果。

![image.png](https://p0-xtjj-private.juejin.cn/tos-cn-i-73owjymdk6/47b5ad4ae3a74ae3ae99f5f599860e06~tplv-73owjymdk6-watermark.image?policy=eyJ2bSI6MywidWlkIjoiMTU5MTc0ODU2OTA3NjA3OCJ9&rk3s=e9ecf3d6&x-orig-authkey=f32326d3454f2ac7e96d3d06cdbb035152127018&x-orig-expires=1721649317&x-orig-sign=7kMLrmqf2Iu9sWn0zA36h4E9fW0%3D)

&emsp;在使用 `--web-renderer=html` 选项构建 Web 时，filter quality 不起作用。所有图像都将使用各自浏览器的默认设置进行渲染。

+ FilterQuality.none：这是最快的 filtering 方法，尽管也是质量最低的。这个数值会导致一个 "Nearest Neighbor" 算法，当图像被放大或缩小时，它会简单地重复或消除像素。
+ FilterQuality.low：比起 none，更高质量；比 medium 速度更快。这个数值会产生一个 "Bilinear" 算法，该算法在图像中的像素之间平滑插值。
+ FilterQuality.medium：这是一种全方位最优的 filtering 方法，仅在极大比例因子下才比 high 的方式差。这个数值通过利用 Mipmap 来改进低质量指定的 "Bilinear" 算法，该 Mipmap 预先计算出图像的高质量较低分辨率，大小为原始图像的一半（以及四分之一和八分之一等），然后在它们之间进行混合，以防止在小尺寸上丢失细节。
+ FilterQuality.high：当通过比例因子放大图像超过 5-10 倍时，获得最佳可能的质量。当图片被缩小时，如果缩放比例小于 0.5x，或者在动画缩放因子时，可能会比中等尺寸效果更差。这个选项也是最慢的。这个数值得到了一个标准的 "Bicubic" 算法，该算法使用了一个三阶方程来平滑像素之间的突变，并保留了一些边缘的感觉，避免结果中的尖峰。

## maskFilter

&emsp;一个蒙版滤镜（例如 模糊）可以应用到一个形状（shape）被绘制后、被合成到图像前。（要么为 none 要么为 blur，目前仅有模糊一种滤镜，具体的模糊效果可以参考：[BlurStyle enum](https://api.flutter.dev/flutter/dart-ui/BlurStyle.html)）

```dart
  static const int _kMaskFilterIndex = 9;
  static const int _kMaskFilterOffset = _kMaskFilterIndex << 2;
  
  static const int _kMaskFilterBlurStyleIndex = 10;
  static const int _kMaskFilterBlurStyleOffset = _kMaskFilterBlurStyleIndex << 2;

  static const int _kMaskFilterSigmaIndex = 11;
  static const int _kMaskFilterSigmaOffset = _kMaskFilterSigmaIndex << 2;

  // 1️⃣0️⃣ 同上：第 10 个属性，位置也来到了第 36 位字节
  // 1️⃣1️⃣ 同上：第 11 个属性，位置也来到了第 40 位字节
  // 1️⃣2️⃣ 同上：第 12 个属性，位置也来到了第 44 位字节
  
  MaskFilter? get maskFilter {
    // 首先取第 10 个属性 MaskFilter，查看当前由没有 蒙版滤镜
    switch (_data.getInt32(_kMaskFilterOffset, _kFakeHostEndian)) {
      case MaskFilter._TypeNone:
        // None
        return null;
      case MaskFilter._TypeBlur:
      
        // Blur，有蒙版滤镜，则再读取第 11 个属性和 12 个属性，构建一个 MaskFilter 对象，
        // const MaskFilter.blur(
        //   this._style,
        //   this._sigma,
        // );
  
        return MaskFilter.blur(
          BlurStyle.values[_data.getInt32(_kMaskFilterBlurStyleOffset, _kFakeHostEndian)],
          _data.getFloat32(_kMaskFilterSigmaOffset, _kFakeHostEndian),
        );
    }
    return null;
  }
  
  // 同上，setter 也是，根据 value 分别设置第 10 11 12 个属性的值。
  set maskFilter(MaskFilter? value) {
    if (value == null) {
      _data.setInt32(_kMaskFilterOffset, MaskFilter._TypeNone, _kFakeHostEndian);
      _data.setInt32(_kMaskFilterBlurStyleOffset, 0, _kFakeHostEndian);
      _data.setFloat32(_kMaskFilterSigmaOffset, 0.0, _kFakeHostEndian);
    } else {
    
      // 目前我们只支持一种 MaskFilter，所以如果不为 null 的话，我们不需要检查它的类型是什么。
      
      _data.setInt32(_kMaskFilterOffset, MaskFilter._TypeBlur, _kFakeHostEndian);
      _data.setInt32(_kMaskFilterBlurStyleOffset, value._style.index, _kFakeHostEndian);
      _data.setFloat32(_kMaskFilterSigmaOffset, value._sigma, _kFakeHostEndian);
    }
  }
```

## invertColors

&emsp;当绘制时，图像的颜色是否被反转。反转图像的颜色会应用一个新的颜色滤镜，该滤镜将与任何用户提供的颜色滤镜组合。这主要用于在 iOS 上实现智能反转。

```dart
  static const int _kInvertColorIndex = 12;
  static const int _kInvertColorOffset = _kInvertColorIndex << 2;
  
  // 1️⃣3️⃣ 同上：第 13 个属性，位置也来到了第 48 位字节。
  
  // 同时也是最后一个属性了，目前 _data 共 52 个字节，
  // invertColors 放置在最后 4 个字节中。
  
  bool get invertColors {
    return _data.getInt32(_kInvertColorOffset, _kFakeHostEndian) == 1;
  }
  
  set invertColors(bool value) {
    _data.setInt32(_kInvertColorOffset, value ? 1 : 0, _kFakeHostEndian);
  }
```

## shader

&emsp;描边（stroking）或填充形状（filling a shape）时使用的着色器。当为 null 时，将使用 color 代替。

&emsp;另请参阅：

+ Gradient，一种绘制颜色渐变的 shader。
+ ImageShader，一种用于平铺图像的 shader。
+ colorFilter，用于覆盖 shader 的 colorFilter。
+ color，如果 shader 和 colorFilter 为空，则使用 color。

&emsp;关于 Shader 的内容，我们后续再学习。本篇先聚焦于 Paint。

```dart
  static const int _kShaderIndex = 0;
 
  // 直接从 _objects 中取
  Shader? get shader {
    return _objects?[_kShaderIndex] as Shader?;
  }
  
  set shader(Shader? value) {
    _ensureObjectsInitialized()[_kShaderIndex] = value;
  }
```

## colorFilter

&emsp;在绘制形状或合成图层时要应用的颜色滤镜。

&emsp;在绘制形状时，colorFilter 会覆盖 color 和 shader。

&emsp;关于 ColorFilter 的内容，我们后续再学习。本篇先聚焦于 Paint。[ColorFilter class](https://api.flutter.dev/flutter/dart-ui/ColorFilter-class.html)

```dart
  static const int _kColorFilterIndex = 1;
  
  // 直接从 _objects 中取。
  ColorFilter? get colorFilter {
    final _ColorFilter? nativeFilter = _objects?[_kColorFilterIndex] as _ColorFilter?;
    return nativeFilter?.creator;
  }
  
  set colorFilter(ColorFilter? value) {
    final _ColorFilter? nativeFilter = value?._toNativeColorFilter();
    
    if (nativeFilter == null) {
      if (_objects != null) {
        _objects![_kColorFilterIndex] = null;
      }
    } else {
      _ensureObjectsInitialized()[_kColorFilterIndex] = nativeFilter;
    }
  }
```

## imageFilter

&emsp;在绘制光栅图像时使用的 ImageFilter。例如，要使用 Canvas.drawImage 对图像进行模糊处理，可以应用 ImageFilter.blur：

&emsp;关于 ImageFilter 的内容，我们后续再学习。本篇先聚焦于 Paint。[ImageFilter class](https://api.flutter.dev/flutter/dart-ui/ImageFilter-class.html)

```dart
void paint(Canvas canvas, Size size) {
  canvas.drawImage(
    _image,
    ui.Offset.zero,
    Paint()..imageFilter = ui.ImageFilter.blur(sigmaX: 0.5, sigmaY: 0.5),
  );
}
```

```dart
  static const int _kImageFilterIndex = 2;
  
  // 直接从 _objects 中取。
  ImageFilter? get imageFilter {
    final _ImageFilter? nativeFilter = _objects?[_kImageFilterIndex] as _ImageFilter?;
    return nativeFilter?.creator;
  }
  
  set imageFilter(ImageFilter? value) {
    if (value == null) {
      if (_objects != null) {
        _objects![_kImageFilterIndex] = null;
      }
    } else {
      final List<Object?> objects = _ensureObjectsInitialized();
      
      final _ImageFilter? imageFilter = objects[_kImageFilterIndex] as _ImageFilter?;
      if (imageFilter?.creator != value) {
        objects[_kImageFilterIndex] = value._toNativeImageFilter();
      }
    }
  }
```

## Paint 总结

&emsp;OK，Paint 类的内容看完了，它内部可以认为是没有任何函数，仅仅是为了保存 Canvas 在本次绘制操作时要使用的样式（或者是一组样式特征）。你甚至可以把 Paint 类看作是一个仅用来存储数据的 ByteData 或 List，它内部仅仅是用来存储 52 个字节的二进制数据和 3 个 Object 对象。

&emsp;首先是 `final ByteData _data = ByteData(_kDataByteCount);`，一个固定 52 个字节的 ByteData，每 4 个字节保存如下的一个样式（样式特征）：

+ bool get isAntiAlias：在 Canvas 上绘制的线条（lines）和图像（images）是否应用抗锯齿技术。默认是 true。

+ Color get color：在描边（stroking）或填充形状（filling a shape）时要使用的颜色。默认值是不透明的黑色。

+ BlendMode get blendMode：在绘制形状（shape）或合成图层（a layer is composited）时应用的混合模式（blend mode）。默认值是 BlendMode.srcOver。

+ PaintingStyle get style：是在形状（shapes）内部绘制、形状边缘绘制，还是两者都绘制。默认值是 PaintingStyle.fill。

+ double get strokeWidth：在 PaintingStyle.stroke 的 style 绘制样式下绘制在边缘的线条的宽度。宽度以逻辑像素表示，沿路径垂直方向测量。默认为 0.0，对应于一条发丝线宽度。

+ StrokeCap get strokeCap：当 PaintingStyle style 设置为 stroke 时，在绘制线条末端时放置的收尾效果。默认设置为 StrokeCap.butt，即无效果。

+ StrokeJoin get strokeJoin：在各段 lines 之间放置的完成形式。这适用于当 PaintingStyle style 设置为 PaintingStyle.stroke 时绘制的 lines，不适用于使用 Canvas.drawPoints 绘制为线的点。默认为 StrokeJoin.miter，即尖角。

+ double get strokeMiterLimit：当各段 lines 连接处被设置为 StrokeJoin.miter 并且当 PaintingStyle style 设置为 PaintingStyle.stroke 时，描边线段上可以绘制斜接线的限制。如果超过了这个限制，那么将绘制 StrokeJoin.bevel 类型的连接。默认为 4.0。

+ FilterQuality get filterQuality：控制性能与质量之间的权衡，用于对位图进行采样，如 ImageShader，或在绘制图像时。默认值是 FilterQuality.none。

+ MaskFilter? get maskFilter：一个蒙版滤镜（例如 模糊）可以应用到一个形状（shape）被绘制后、被合成到图像前。要么为 none 要么为 blur，目前仅有 blur 模糊这一种滤镜。

+ bool get invertColors：当绘制时，图像的颜色是否被反转。反转图像的颜色会应用一个新的颜色滤镜，该滤镜将与任何用户提供的颜色滤镜组合。这主要用于在 iOS 上实现智能反转。

&emsp;然后是 `List<Object?>? _objects = List<Object?>.filled(_kObjectCount, null);`，一个固定长度是 3 的 List，分别保存如下 3 个对象，如下：

+ Shader? get shader：在描边（stroking）或填充形状（filling a shape）时使用的着色器。当为 null 时，将使用 color 代替。
+ ColorFilter? get colorFilter：在绘制形状或合成图层时要应用的颜色滤镜。
+ ImageFilter? get imageFilter：在绘制光栅图像时使用的 ImageFilter。

&emsp;没了，Paint 类中就这么多内容。最主要的是我们要记住以上样式设置对绘制的影响，以后在绘制操作中多实践即可。

&emsp;OK，Paint 内容就到这里吧，快速浏览即可。

## 参考链接
**参考链接:🔗**
+ [Canvas class](https://api.flutter.dev/flutter/dart-ui/Canvas-class.html)
+ [PictureRecorder class](https://api.flutter.dev/flutter/dart-ui/PictureRecorder-class.html)
+ [Picture class](https://api.flutter.dev/flutter/dart-ui/Picture-class.html)
+ [Flutter 3.7 之快速理解 toImageSync 是什么？能做什么？](https://guoshuyu.cn/home/wx/Flutter-N18.html)
+ [Paint class](https://api.flutter.dev/flutter/dart-ui/Paint-class.html)
+ [BlendMode enum](https://api.flutter.dev/flutter/dart-ui/BlendMode.html)
+ [PaintingStyle enum](https://api.flutter.dev/flutter/dart-ui/PaintingStyle.html)
+ [StrokeCap enum](https://api.flutter.dev/flutter/dart-ui/StrokeCap.html)
+ [FilterQuality enum](https://api.flutter.dev/flutter/dart-ui/FilterQuality.html)
+ [MaskFilter class](https://api.flutter.dev/flutter/dart-ui/MaskFilter-class.html)
+ [BlurStyle enum](https://api.flutter.dev/flutter/dart-ui/BlurStyle.html)
+ [ColorFilter class](https://api.flutter.dev/flutter/dart-ui/ColorFilter-class.html)
+ [ImageFilter class](https://api.flutter.dev/flutter/dart-ui/ImageFilter-class.html)
