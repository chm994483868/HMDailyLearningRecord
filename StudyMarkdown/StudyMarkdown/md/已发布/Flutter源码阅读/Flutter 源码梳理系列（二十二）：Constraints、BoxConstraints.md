# Flutter 源码梳理系列（二十二）：Constraints、BoxConstraints

&emsp;在正式开始之前，我们需要扩展一下 Constraints（约束）相关的内容，否则看到 RenderObject 布局相关的内容时会有点迷糊。本篇就先学习一下 Constraints 和 BoxConstraints 两个类的内容。

# Constraints

&emsp;Constraints 用来表示一组抽象的布局约束。具体的布局模型（例如：基于 RenderBox 的 Box 布局模型）会创建具体的子类来在父级（RenderObject）和子级（RenderObject）之间传递布局约束。（注意这里给出了方向，父级传给子级的，再具体一点就是：父级强制加给子级的布局约束。）

## Writing a Constraints subclass

&emsp;当创建一个带有新布局协议的新的 RenderObject 子类时，通常需要创建一个新的 Constraints 子类来表达布局算法的输入。（我们后面主要聚集在：RenderBox 和 BoxConstraints。）

&emsp;Constraints 子类应该是不可变的（所有字段都是 final）。除了可能发现对特定布局协议有用的字段、构造函数和辅助方法之外，还需要实现几个成员：

+ isTight getter 返回 true，如果对象表示 RenderObject 类在自身布局方面没有选择余地的情况。例如，当 BoxConstraints 的最小和最大宽度以及最小和最大高度都相等时，isTight 返回 true。（这里可以理解为当 BoxConstraints 直接把宽度和高度限制死，例如：宽度是 30，高度是 60，是一种严格限制。而不是宽松的限制，例如：高度在 30 和 60 之间，高度在 50 到 100 之间。）

+ isNormalized 这个 getter 应该在对象以规范形式表示其数据时返回 true。有时，各个字段之间可能存在冗余，导致几种不同的表示具有相同的含义。例如，一个 BoxConstraints 实例，其最小宽度大于最大宽度，等价于一个最大宽度设置为该最小宽度的实例 (`2<w<1` 等价于 `2<w<2`，因为最小约束具有优先级)。这个 getter 在 debugAssertIsValid 的默认实现中使用。（这里我们依然可以以 BoxConstraints 举例，当最小宽度大于等于 0，并且最小宽度小于等于最大宽度，且最小高度大于等于 0，并且最小高度小于等于最大高度时，isNormalized 这个 getter 就会返回 true，此时是一种标准化的约束，或者说是一个正确的约束。换一种理解方式如：我们先有一个范围  [0, double.infinity]，minWidth、maxWidth、minHeight、maxHeight，都从这里面取值，然后再加上 minWidth 小于等于 maxWidth 和 minHeight 小于等于 maxHeight，此时这个约束就是 isNormalized 为 true 的约束。）

+ debugAssertIsValid 方法用于断言 Constraints 对象是否存在任何问题。（我们使用这种方法而不是在构造函数中断言，这样我们的构造函数可以是 const，并且在构建有效约束时可以临时创建无效约束。）可以参考 BoxConstraints.debugAssertIsValid 的实现来查看可以进行的详细检查示例。（即：minWidth、maxWidth、minHeight、maxHeigt 四个属性如果用了错误的值就表示是错误的 BoxConstraints，是无效的。）（此函数就是用来判断约束有效性的，在 BoxConstraints 的构造函数中没有使用，而是在 BoxConstraints 的一些约束转换函数中使用了，用于判断原 BoxConstraints 对象是否是有效的约束。）

+ == 运算符和 hashCode getter，以便比较约束的相等性。如果一个渲染对象被给定相等的约束，那么渲染库将避免再次对对象进行布局，如果它没有被标记为 "dirty"。（例如：两个 BoxConstraints 对象，即使它们不是同一个对象，只要它们的四个属性是相等的，那么就认为两个 BoxConstraints 对象是相等。）

+ toString 方法应该描述约束条件（例如：一个 BoxConstraints 对象调用 toString 函数时，会根据它的四个属性值的不同情况打印不同的描述字符串，方便我们理解约束的意图），使它们以有用的可读形式出现在 debugDumpRenderTree 的输出中。

&emsp;下面简单快速看一下 Constructors 的代码。它的代码很少，首先是它的构造函数。

## Constructors

&emsp;抽象常量构造函数。这个构造函数使子类能够提供常量构造函数，从而可以在常量表达式中使用它们。

```dart
@immutable
abstract class Constraints {
  const Constraints();

  // ...
}
```

## isTight

&emsp;在这些约束条件下是否有且仅有一个固定的尺寸。（如：最大最小宽度两个属性相等，最大最小高度两个属性相等。）

```dart
  bool get isTight;
```

## isNormalized

&emsp;约束是否以一致的方式表式。（是否是一个标准化、正确的约束。）

```dart
  bool get isNormalized;
```

## debugAssertIsValid

&emsp;断言 Constraints 是有效的。这可能涉及比 isNormalized 更详细的检查。

&emsp;例如，BoxConstraints 子类会验证约束条件不是 double.nan（`static const double nan = 0.0 / 0.0;`）。

&emsp;如果 isAppliedConstraint 参数设置为 true，那么将施加更严格的规则。当在布局期间检查即将应用于 RenderObject 的约束条件时，会将此参数设置为 true，而不是对可能受其他约束条件进一步影响的约束条件。例如，用于验证 RenderConstrainedBox.additionalConstraints 有效性的断言不设置此参数，但用于验证传递给 RenderObject.layout 方法的参数的断言会设置。（我们可以直接看 BoxConstraints.debugAssertIsValid 函数中的代码，可以看到 isAppliedConstraint 为 true 时，其实就是多了 minWidth 和  minHeight 不能是 double.infinity 的判断。）

&emsp;informationCollector 参数采用一个可选的回调函数，当需要抛出异常时会调用该回调。收集的信息将在错误行后的消息中包含。

&emsp;如果断言被禁用，则返回与 isNormalized 相同的结果。已知生产环境所有断言被禁用。

&emsp;开发环境下同 isNormalized 的判断条件。只要符合 isNormalized 条件的话约束就是没有问题的。

```dart
  bool debugAssertIsValid({
    bool isAppliedConstraint = false,
    InformationCollector? informationCollector,
  }) {
    assert(isNormalized);
    
    return isNormalized;
  }
}
```

## Constraints 总结

&emsp;OK，其实单看 Constraints 的内容还是有点抽象的，根本看不懂，我们是反过来看的，先去看它的子类，在整个 framework 中它只有两个子类：BoxConstraints 和 SliverConstraints，它俩一个是 Box 模型约束，一个是 Sliver 模型约束（针对滚动视图）。在这两个子类里面为了满足上面👆 Constraints 的要求，对上面的 getter 和 debugAssertIsValid 函数都进行对应的实现。

+ isTight 呢是比较简单的，例如 BoxConstraints 的四个属性：minWidth、maxWidth、minHeight、maxHeight 四个属性是父级用来限制子级的约束，四个属性合并起来就是一个宽度和高度的范围约束。例如子级在进行布局时，父级要求子级的宽度在 50 到 100 之间，高度在 60 到 200 之间，即父级要求子级最小范围在 `50 * 60`，最大范围在 `100 * 200`，或者父级直接要求子级宽度是 50 高度是 60，那么此时宽度和高度完全固定，子级必须是 `50 * 60` 的尺寸大小，那此时这个 isTight 的 getter 就返回 true。

+ isNormalized 呢也比较好理解。如果 isTight 针对的是约束的表现行为的话，isNormalized 针对的就是约束的数值是否正确。例如 BoxConstraints 的四个属性：minWidth、maxWidth、minHeight、maxHeight，只要它们的数值在正常合理的范围，那此时这个 isNormalized 的 getter 就返回 true。

+ debugAssertIsValid 呢可以说是 isNormalized 的加强，或者说是同比判断。在开发模式下判断约束的属性值是否合理。

&emsp;OK，Constraints 作为约束的抽象基类，一些概念确实不好理解，下面我们看它的直接子类 BoxConstraints，我们日常中经常使用的，它会比较好理解一些。 

# BoxConstraints 

&emsp;BoxConstraints 用来表示对于 RenderBox 布局的不可变布局约束。这里首先要解释一下 BoxConstraints 约束，它到底约束的是什么，它有什么意义？首先它完全不同于 iOS 中的约束。Flutter 这里的 BoxConstraints，它只约束一件事情，即：尺寸、大小的约束，或者理解为宽高的约束。是的，它只是宽度和高度的约束。

&emsp;只有当以下所有关系均成立时，那么表示这个 Size 对象是符合 BoxConstraints 约束：

+ `BoxConstraints.minWidth <= Size.width <= BoxConstraints.maxWidth` 
+ `BoxConstraints.minHeight <= Size.height <= BoxConstraints.maxHeight`

&emsp;即一个 Size 对象的 width 和 height 属性，刚好在一个 BoxConstraints 对象的 minWidth 和 maxWidth 以及 minHeight 和 maxHeight 时范围时，说明这个 Size 是符合这个 BoxConstraints 约束的。

&emsp;同时 BoxConstraints 对象的四个属性值，要满足下面这些关系：

+ `0.0 <= minWidth <= maxWidth <= double.infinity`
+ `0.0 <= minHeight <= maxHeight <= double.infinity`

&emsp;同时对于每个约束，double.infinity 都是一个合法的值。

&emsp;看到这里面你应该明白了吧：BoxConstraints 只是一个宽度和高度的约束，或者理解为只是用来描述宽度和高度的可用范围。无他。

## The box layout model

&emsp;Flutter framework 中的渲染对象（RenderObject）是通过 one-pass 布局模型进行布局的，该模型沿着 Render Tree 向下传递约束（BoxConstraints），然后沿着 Render Tree 向上传递具体的几何信息（Size）。

&emsp;对于盒子（boxes），约束是 BoxConstraints 对象，如本文所述，由四个数字组成：最小宽度 minWidth，最大宽度 maxWidth，最小高度 minHeight 和最大高度 maxHeight。（即，每个 RenderBox 被看作是一个 box， 四个数字即 BoxConstraints 约束的四个属性。）

&emsp;盒子（boxes）的几何信息包括一个 Size，必须满足上述约束。（被约束的子级有一个 Size 属性，必须满足上面的约束，即 Size 的 width 和 height 必须符合 BoxConstraints 的 最小最大宽度和高度的范围。）

&emsp;每个 RenderBox（为 box widgets 提供布局模型的对象）从其父级接收 BoxConstraints，然后对其每个子级（RenderBox）进行布局，然后选择一个满足 BoxConstraints 约束的 Size。

&emsp;渲染对象独立布置其子级而不涉及布局过程。通常，父级将使用子级的 Size 来确定其位置。子级不知道自己的位置，如果其位置发生更改，它不一定会再次进行布局或重绘。

## Terminology

&emsp;当一个轴（x 轴或者 y 轴）的最小约束（minWidth、minHeight）和最大约束（maxWidth、maxHeight）相同时，该轴被称为紧密约束（isTight 属性的值为 true）。参见：BoxConstraints.tightFor、BoxConstraints.tightForFinite、tighten、hasTightWidth、hasTightHeight、isTight。

&emsp;最小约束（minWidth、minHeight）为 0.0 的轴是宽松（loose）的（无论最大约束是什么；如果最大约束也是 0.0，那么该轴同时是 tight 和 loose 的！）。参见：BoxConstraints.loose, loosen。

&emsp;一个最大约束（maxWidth、maxHeight）不是无限的轴（x 轴或者 y 轴）是有界的。参见：hasBoundedWidth，hasBoundedHeight。（即 maxWidth 或 maxHeight 的值不是 double.infinity 的。）

&emsp;如果一个轴的最大约束是无限的，那么该轴是不受限制的。如果一个轴是紧密无限的（其最小和最大约束都是无限的），那么该轴是在扩展。参见：BoxConstrants.expand。

&emsp;最小约束为无穷大的轴被称为是无限的（因为根据定义，在这种情况下最大约束也必定是无穷大）。参见: hasInfiniteWidth, hasInfiniteHeight。

&emsp;当 size 满足 BoxConstraints 描述时，该 size 被认为是受限制的。参见：constrain, constrainWidth, constrainHeight, constrainDimensions, constrainSizeAndAttemptToPreserveAspectRatio, isSatisfiedBy.

+ Object -> Constraints -> BoxConstraints

&emsp;OK，下面我们开始看 BoxConstraints 的源码。

## Constructors

&emsp;BoxConstraints 提供了一组构造函数。给予 minWidth、maxWidth、minHeight、maxHeight 不同的值。

```dart
class BoxConstraints extends Constraints {
  // 使用给定的约束条件创建 BoxConstraints。
  const BoxConstraints({
    this.minWidth = 0.0,
    this.maxWidth = double.infinity,
    this.minHeight = 0.0,
    this.maxHeight = double.infinity,
  });

  // 创建仅受给定 Size 约束的 BoxConstraints。
  // 即直接给出固定的 宽度 和 高度。
  BoxConstraints.tight(Size size)
    : minWidth = size.width,
      maxWidth = size.width,
      minHeight = size.height,
      maxHeight = size.height;
  
  // 创建需要指定宽度或高度的 BoxConstraints。
  const BoxConstraints.tightFor({
    double? width,
    double? height,
  }) : minWidth = width ?? 0.0,
       maxWidth = width ?? double.infinity,
       minHeight = height ?? 0.0,
       maxHeight = height ?? double.infinity;

  // 创建要求给定宽度或高度的 BoxConstraints，除非它们是无限的。
  const BoxConstraints.tightForFinite({
    double width = double.infinity,
    double height = double.infinity,
  }) : minWidth = width != double.infinity ? width : 0.0,
       maxWidth = width != double.infinity ? width : double.infinity,
       minHeight = height != double.infinity ? height : 0.0,
       maxHeight = height != double.infinity ? height : double.infinity;
  
  // 创建禁止大于给定大小的 BoxConstraints。
  BoxConstraints.loose(Size size)
    : minWidth = 0.0,
      maxWidth = size.width,
      minHeight = 0.0,
      maxHeight = size.height;
  
  // 创建一个可以扩展填充另一个 BoxConstraints 的 BoxConstraints。
  // 如果给定了宽度或高度，则约束将在给定维度上严格要求确切的给定值。
  const BoxConstraints.expand({
    double? width,
    double? height,
  }) : minWidth = width ?? double.infinity,
       maxWidth = width ?? double.infinity,
       minHeight = height ?? double.infinity,
       maxHeight = height ?? double.infinity;

  // 创建与给定视图约束相匹配的 BoxConstraints。
  BoxConstraints.fromViewConstraints(ui.ViewConstraints constraints)
      : minWidth = constraints.minWidth,
        maxWidth = constraints.maxWidth,
        minHeight = constraints.minHeight,
        maxHeight = constraints.maxHeight;

  // ...
}
```

## minWidth、maxWidth、minHeight、maxHeight

&emsp;BoxConstraints 约束的四个属性值，也代表了此约束的约束内容。仅用来约束 Box 的宽度和高度范围。

```dart
  // 满足约束条件的最小宽度。
  final double minWidth;
  
  // 满足约束条件的最大宽度，可能是 [double.infinity]
  final double maxWidth;

  // 满足约束条件的最小高度
  final double minHeight;
  
  // 满足约束条件的最大高度，可能是 [double.infinity]
  final double maxHeight;
```

## copyWith

&emsp;创建此 BoxConstraints 的副本，但用新值替换给定的字段。

```dart
  BoxConstraints copyWith({
    double? minWidth,
    double? maxWidth,
    double? minHeight,
    double? maxHeight,
  }) {
    return BoxConstraints(
      minWidth: minWidth ?? this.minWidth,
      maxWidth: maxWidth ?? this.maxWidth,
      minHeight: minHeight ?? this.minHeight,
      maxHeight: maxHeight ?? this.maxHeight,
    );
  }
```

## deflate

&emsp;返回新的 BoxConstraints，其尺寸减去给定的边缘尺寸。（针对水平和垂直两个方向进行缩进。）

```dart
  BoxConstraints deflate(EdgeInsetsGeometry edges) {
    final double horizontal = edges.horizontal;
    final double vertical = edges.vertical;
    
    final double deflatedMinWidth = math.max(0.0, minWidth - horizontal);
    final double deflatedMinHeight = math.max(0.0, minHeight - vertical);
    
    return BoxConstraints(
      minWidth: deflatedMinWidth,
      maxWidth: math.max(deflatedMinWidth, maxWidth - horizontal),
      minHeight: deflatedMinHeight,
      maxHeight: math.max(deflatedMinHeight, maxHeight - vertical),
    );
  }
```

## loosen

&emsp;返回一个新的 BoxConstraints，取消了最小宽度和高度的要求。（即把当前 BoxConstraints 对象的 minWidth 和 minHeight 限制去除。）

```dart
  BoxConstraints loosen() {
    return BoxConstraints(
      maxWidth: maxWidth,
      maxHeight: maxHeight,
    );
  }
```

## enforce

&emsp;返回一个新的 BoxConstraints，该约束在尽量接近原始约束的同时也要符合给定的约束。

```dart
  BoxConstraints enforce(BoxConstraints constraints) {
    return BoxConstraints(
      minWidth: clampDouble(minWidth, constraints.minWidth, constraints.maxWidth),
      maxWidth: clampDouble(maxWidth, constraints.minWidth, constraints.maxWidth),
      minHeight: clampDouble(minHeight, constraints.minHeight, constraints.maxHeight),
      maxHeight: clampDouble(maxHeight, constraints.minHeight, constraints.maxHeight),
    );
  }
```

## tighten

&emsp;返回新的 BoxConstraints，其宽度和/或高度尽可能接近给定的宽度和高度，同时仍然遵守原始的盒约束。

```dart
  BoxConstraints tighten({ double? width, double? height }) {
    return BoxConstraints(
      minWidth: width == null ? minWidth : clampDouble(width, minWidth, maxWidth),
      maxWidth: width == null ? maxWidth : clampDouble(width, minWidth, maxWidth),
      minHeight: height == null ? minHeight : clampDouble(height, minHeight, maxHeight),
      maxHeight: height == null ? maxHeight : clampDouble(height, minHeight, maxHeight),
    );
  }
```

## flipped

&emsp;一个具有翻转后的宽度和高度约束的 BoxConstraints。

```dart
  BoxConstraints get flipped {
    return BoxConstraints(
      minWidth: minHeight,
      maxWidth: maxHeight,
      minHeight: minWidth,
      maxHeight: maxWidth,
    );
  }
```

## widthConstraints & heightConstraints

&emsp;返回具有相同宽度约束但高度不受约束的框约束。返回具有相同高度约束但宽度不受约束的框约束。

```dart
  BoxConstraints widthConstraints() => BoxConstraints(minWidth: minWidth, maxWidth: maxWidth);
  BoxConstraints heightConstraints() => BoxConstraints(minHeight: minHeight, maxHeight: maxHeight);
```

## constrainWidth & constrainHeight

&emsp;返回既满足约束条件又尽可能接近给定宽度的宽度。返回既满足约束条件又尽可能接近给定高度的高度。

```dart
  double constrainWidth([ double width = double.infinity ]) {
    return clampDouble(width, minWidth, maxWidth);
  }

  double constrainHeight([ double height = double.infinity ]) {
    return clampDouble(height, minHeight, maxHeight);
  }
```

## constrain

&emsp;返回既满足约束条件又尽可能接近给定大小的尺寸。

```dart
  Size constrain(Size size) {
    Size result = Size(constrainWidth(size.width), constrainHeight(size.height));
    return result;
  }
```

## constrainDimensions

&emsp;返回既满足约束条件又尽可能接近给定宽度和高度的尺寸。当你已经有一个 Size 时，更倾向于使用 constrain，它直接将相同算法应用于 Size。

```dart
  Size constrainDimensions(double width, double height) {
    return Size(constrainWidth(width), constrainHeight(height));
  }
```

## constrainSizeAndAttemptToPreserveAspectRatio

&emsp;返回一个尺寸，试图满足以下条件，依次为：

1. 尺寸必须满足这些约束。
2. 返回尺寸的宽高比与给定尺寸的宽高比相匹配。
3. 返回的尺寸尽可能大，同时仍然等于或小于给定尺寸。

```dart
  Size constrainSizeAndAttemptToPreserveAspectRatio(Size size) {
    if (isTight) {
      Size result = smallest;
      return result;
    }

    double width = size.width;
    double height = size.height;
    
    final double aspectRatio = width / height;

    if (width > maxWidth) {
      width = maxWidth;
      height = width / aspectRatio;
    }

    if (height > maxHeight) {
      height = maxHeight;
      width = height * aspectRatio;
    }

    if (width < minWidth) {
      width = minWidth;
      height = width / aspectRatio;
    }

    if (height < minHeight) {
      height = minHeight;
      width = height * aspectRatio;
    }

    Size result = Size(constrainWidth(width), constrainHeight(height));
    return result;
  }
```

## biggest & smallest

&emsp;满足约束条件的最大尺寸。满足约束条件的最小尺寸。

```dart
  Size get biggest => Size(constrainWidth(), constrainHeight());
  Size get smallest => Size(constrainWidth(0.0), constrainHeight(0.0));
```

## hasTightWidth & hasTightHeight

&emsp;是否有一个确切的宽度值满足约束条件。是否有一个确切的高度值满足约束条件。

```dart
  bool get hasTightWidth => minWidth >= maxWidth;
  bool get hasTightHeight => minHeight >= maxHeight;
```

## isTight

&emsp;是否有严格满足约束条件的唯一大小（Size，最大最小宽度和最大最小高度一致）。

```dart
  @override
  bool get isTight => hasTightWidth && hasTightHeight;
```

## hasBoundedWidth & hasBoundedHeight & hasInfiniteWidth & hasInfiniteHeight

&emsp;是否存在对最大宽度的上限。是否存在对最大高度的上限。

&emsp;是否 宽度/高度 约束为无限制。

&emsp;这样的约束用于指示一个 Box 应该像其他约束一样变大。如果约束是无限的，那么它们必须有其他（非无限的）约束被强制执行，或者必须在能够用它们推导出 RenderBox.size 的尺寸之前被收紧。（物理屏幕的宽度和高度）

```dart
  bool get hasBoundedWidth => maxWidth < double.infinity;
  bool get hasBoundedHeight => maxHeight < double.infinity;
  bool get hasInfiniteWidth => minWidth >= double.infinity;
  bool get hasInfiniteHeight => minHeight >= double.infinity;
```

## isSatisfiedBy

&emsp;给定的尺寸是否符合约束条件。

```dart
  bool isSatisfiedBy(Size size) {
    return (minWidth <= size.width) && (size.width <= maxWidth) &&
           (minHeight <= size.height) && (size.height <= maxHeight);
  }
```

## `*/~/%`

```dart
  // 将每个约束参数乘以给定的因子。
  BoxConstraints operator*(double factor) {
    return BoxConstraints(
      minWidth: minWidth * factor,
      maxWidth: maxWidth * factor,
      minHeight: minHeight * factor,
      maxHeight: maxHeight * factor,
    );
  }
  
  // 将每个约束参数除以给定的因子。
  BoxConstraints operator/(double factor) {
    return BoxConstraints(
      minWidth: minWidth / factor,
      maxWidth: maxWidth / factor,
      minHeight: minHeight / factor,
      maxHeight: maxHeight / factor,
    );
  }
  
  // 按照给定因子的倒数，将每个约束参数按最接近的整数四舍五入。
  BoxConstraints operator~/(double factor) {
    return BoxConstraints(
      minWidth: (minWidth ~/ factor).toDouble(),
      maxWidth: (maxWidth ~/ factor).toDouble(),
      minHeight: (minHeight ~/ factor).toDouble(),
      maxHeight: (maxHeight ~/ factor).toDouble(),
    );
  }
  
  // 将每个约束参数与给定的因子取余。
  BoxConstraints operator%(double value) {
    return BoxConstraints(
      minWidth: minWidth % value,
      maxWidth: maxWidth % value,
      minHeight: minHeight % value,
      maxHeight: maxHeight % value,
    );
  }
```

## lerp

&emsp;在两个 BoxConstraints 之间进行线性插值。

&emsp;如果其中一个为 null，则此函数会从一个所有字段均设为 0.0 的 BoxConstraints 对象开始进行插值。

&emsp;参数 t 表示时间轴上的位置，其中 0.0 表示插值尚未开始，返回 a（或与 a 等效的内容），1.0 表示插值已完成，返回 b（或与 b 等效的内容），介于两者之间的值表示插值在时间轴上处于 a 和 b 之间的相关点。插值可以超出 0.0 和 1.0，因此负值和大于 1.0 的值均有效（并且可以通过 Curves.elasticInOut 等曲线轻松生成）。

&emsp;通常从 `Animation<double>`（如 AnimationController）中获取 t 的值。

```dart
  static BoxConstraints? lerp(BoxConstraints? a, BoxConstraints? b, double t) {
    if (identical(a, b)) {
      return a;
    }
    
    if (a == null) {
      return b! * t;
    }
    
    if (b == null) {
      return a * (1.0 - t);
    }
    
    return BoxConstraints(
      minWidth: a.minWidth.isFinite ? ui.lerpDouble(a.minWidth, b.minWidth, t)! : double.infinity,
      maxWidth: a.maxWidth.isFinite ? ui.lerpDouble(a.maxWidth, b.maxWidth, t)! : double.infinity,
      minHeight: a.minHeight.isFinite ? ui.lerpDouble(a.minHeight, b.minHeight, t)! : double.infinity,
      maxHeight: a.maxHeight.isFinite ? ui.lerpDouble(a.maxHeight, b.maxHeight, t)! : double.infinity,
    );
  }
```

## isNormalized

&emsp;返回对象的约束是否已经被标准化。 如果最小值小于或等于相应的最大值，则约束被认为是标准化的。

&emsp;例如，一个 BoxConstraints 对象，其最小宽度为 100.0，最大宽度为 90.0，则该约束并非标准化。

&emsp;大多数 BoxConstraints 上的 API 都希望约束是标准化的，并且在它们未被标准化时行为是未定义的。 在调试模式下，如果约束未被标准化，许多这些 API 将会断言。

```dart
  @override
  bool get isNormalized {
    return minWidth >= 0.0 &&
           minWidth <= maxWidth &&
           minHeight >= 0.0 &&
           minHeight <= maxHeight;
```

## normalize

&emsp;返回一个已标准化的 BoxConstraints。

&emsp;返回的最大宽度至少与最小宽度一样大。同样，返回的最大高度至少与最小高度一样大。

```dart
  BoxConstraints normalize() {
    if (isNormalized) {
      return this;
    }
    
    final double minWidth = this.minWidth >= 0.0 ? this.minWidth : 0.0;
    final double minHeight = this.minHeight >= 0.0 ? this.minHeight : 0.0;
    
    return BoxConstraints(
      minWidth: minWidth,
      maxWidth: minWidth > maxWidth ? minWidth : maxWidth,
      minHeight: minHeight,
      maxHeight: minHeight > maxHeight ? minHeight : maxHeight,
    );
  }
```

## Constraints 总结

&emsp;不同于 iOS 的约束，BoxConstraints 只与 Size 相关，四个属性值：minWidth、maxWidth、minHeight、maxHeight 来限制 Box 的 size，即来限制 RenderBox 的宽度和高度的可用范围。然后提供了一组不同的便利构造函数来为四个属性传递不同的值，应对不同的约束情况。其它还提供了一些转换函数以及操作符，来对四个属性做一些基础变换。其它还提供了一些判断函数，来对四个属性值的范围做一些基础的判断。都是比较简单的，快速浏览即可。

## 参考链接
**参考链接:🔗**
+ [Constraints class](https://api.flutter.dev/flutter/rendering/Constraints-class.html)
+ [BoxConstraints class](https://api.flutter.dev/flutter/rendering/BoxConstraints-class.html)
+ [Layout widgets](https://docs.flutter.dev/ui/widgets/layout)
