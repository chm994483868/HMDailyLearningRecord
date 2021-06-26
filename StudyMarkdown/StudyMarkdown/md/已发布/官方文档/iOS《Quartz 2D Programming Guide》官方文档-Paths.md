# iOS《Quartz 2D Programming Guide》官方文档-Graphics Contexts

## Paths
&emsp;路径定义一个或多个形状或子路径。子路径可以由直线、曲线或两者组成。它可以打开也可以关闭。子路径可以是简单的形状，如直线、圆、矩形或星形，也可以是更复杂的形状，如山脉的轮廓或抽象涂鸦。图 3-1 显示了你可以创建的一些路径。直线（在图的左上角）是虚线；直线也可以是实线。弯曲路径（中间顶部）由多条曲线组成，是一条开放路径。同心圆是填充的，但不是笔划的。加利福尼亚州是一条封闭的道路，由许多曲线和线条组成，这条道路既有笔划又有填充物。星星说明了填充路径的两个选项，你将在本章后面阅读。

&emsp;Figure 3-1  Quartz supports path-based drawing（Quartz 支持基于路径的绘图）
![path_vector_examples](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/0ec7adf900704aa8859c798cc37f429e~tplv-k3u1fbpfcp-watermark.image)

&emsp;在本章中，你将了解构成路径的构建块、如何绘制和绘制路径以及影响路径外观的参数。

### Path Creation and Path Painting
&emsp;路径创建和路径绘制是两个独立的任务。首先创建一条路径。如果要渲染路径，请请求 Quartz 对其进行绘制。如图 3-1 所示，你可以选择笔划路径、填充路径或同时笔划和填充路径。也可以使用路径将其他对象的图形约束在路径的边界内，从而创建一个剪裁区域。

&emsp;图 3-2 显示了已绘制的路径，该路径包含两个子路径。左边的子路径是矩形，右边的子路径是由直线和曲线组成的抽象形状。每个子路径被填充，其轮廓被抚摸。

&emsp;图 3-2 显示了已绘制的路径，其中包含两个子路径。左边的子路径是一个矩形，右边的子路径是由直线和曲线组成的抽象形状。每个子路径都被填充并描出其轮廓。

&emsp;Figure 3-2  A path that contains two shapes, or subpaths（包含两个形状或子路径的路径）

![subpaths](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/9e153aae2841442eb8c301bc1d449eaa~tplv-k3u1fbpfcp-watermark.image)

&emsp;图 3-3 显示了独立绘制的多条路径。每条路径都包含一条随机生成的曲线，其中一些是填充的，而另一些是笔划的。图形被剪裁区域约束到圆形区域。

&emsp;Figure 3-3  A clipping area constrains drawing（裁剪区域限制绘图）

![circle_clipping](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/b43b29b041d84a68a00f85f47c13a71a~tplv-k3u1fbpfcp-watermark.image)

### The Building Blocks
&emsp;子路径由直线、圆弧和曲线构建。Quartz 还提供了方便的函数，可以通过单个函数调用添加矩形和椭圆。点也是路径的基本构建块，因为点定义形状的起点和终点位置。

#### Points
&emsp;点是在用户空间中指定位置的 x 和 y 坐标。你可以调用函数 `CGContextMoveToPoint` 来指定新子路径的起始位置。Quartz 跟踪当前点，这是用于构建路径的最后一个位置。例如，如果调用函数 `CGContextMoveToPoint` 将位置设置为（10, 10），则会将当前点移动到（10, 10）。如果然后画一条 50 个单位长的水平线，则线上的最后一点（即（60, 10））将成为当前点。直线、圆弧和曲线始终从当前点开始绘制。

&emsp;大多数情况下，通过向 Quartz 函数传递两个浮点值来指定 x 和 y 坐标来指定点。有些函数需要传递一个 CGPoint 数据结构，该结构包含两个浮点值。

#### Lines
&emsp;直线由其端点定义。其起点始终假定为当前点，因此在创建直线时，仅指定其端点。使用函数 `CGContextAddLineToPoint` 将一行附加到子路径。

&emsp;通过调用函数 `CGContextAddLines`，可以将一系列连接的行添加到路径中。将一组点传递给此函数。第一个点必须是第一条线的起点；其余点是端点。Quartz 从第一个点开始一个新的子路径，并将一条直线段连接到每个端点。

#### Arcs
&emsp;圆弧是圆段。Quartz 提供了两个创建圆弧的函数。函数 `CGContextAddArc` 从一个圆创建一个曲线段。指定圆心、半径和半径角（以弧度为单位）。通过指定 2π 的径向角，可以创建一个完整的圆。图 3-4 显示了独立绘制的多条路径。每条路径都包含一个随机生成的圆；有些是填充的，有些是笔划的。

&emsp;Figure 3-4  Multiple paths; each path contains a randomly generated circle（多路径；每个路径包含一个随机生成的圆）

![circles](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/161d858955654a389962c1d5acdf7d0a~tplv-k3u1fbpfcp-watermark.image)

&emsp;函数 `CGContextAddArcToPoint` 非常适合用于圆角矩形。Quartz 使用你提供的端点创建两条切线。还可以提供 Quartz 切割圆弧的圆的半径。弧的中心点是两个半径的交点，每个半径垂直于两条切线中的一条。弧的每个端点是其中一条切线上的一个切点，如图 3-5 所示。圆的红色部分是实际绘制的。

&emsp;Figure 3-5  Defining an arc with two tangent lines and a radius（用两条切线和一条半径定义一条弧）

![rounded_corner](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/1053fad671e74e9bb64e870b1ae2b7c8~tplv-k3u1fbpfcp-watermark.image)

&emsp;如果当前路径已经包含子路径，Quartz 会附加一条从当前点到圆弧起点的直线段。如果当前路径为空，Quartz 将在弧的起点处创建新的子路径，并且不添加初始直线段。

#### Curves
&emsp;二次和三次 Bézier 曲线是代数曲线，可以指定任意数量的有趣曲线形状。这些曲线上的点通过对起点和终点以及一个或多个控制点应用多项式公式来计算。以这种方式定义的形状是矢量图形的基础。一个公式比一个位数组要紧凑得多，它的优点是曲线可以以任何分辨率重新创建。

&emsp;图 3-6 显示了通过独立绘制多条路径创建的各种曲线。每条路径都包含一条随机生成的曲线；有些是填充的，有些是笔划的。

&emsp;Figure 3-6  Multiple paths; each path contains a randomly generated curve（多路径；每个路径包含随机生成的曲线）

![bezier_paths](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/13f76dea17e14dd59efd5a5727c5d968~tplv-k3u1fbpfcp-watermark.image)

&emsp;在许多数学文本和描述计算机图形学的在线资源中讨论了产生二次和三次 Bézier 曲线的多项式公式，以及如何从公式中生成曲线的细节。这里不讨论这些细节。

&emsp;使用函数 `CGContextAddCurveToPoint`，使用控制点和指定的端点，从当前点附加一条三次 Bézier 曲线。图 3-7 显示了从图中所示的当前点、控制点和端点得到的三次 Bézier 曲线。两个控制点的位置决定了曲线的几何图形。如果控制点都在起点和终点上方，则曲线向上拱起。如果控制点都低于起点和终点，则曲线向下拱起。




#### Closing a Subpath

#### Ellipses

#### Rectangles

### Creating a Path

### Painting a Path

#### Parameters That Affect Stroking

#### Functions for Stroking a Path

#### Filling a Path

#### Setting Blend Modes

##### Normal Blend Mode

##### Multiply Blend Mode

##### Screen Blend Mode

##### Overlay Blend Mode

##### Darken Blend Mode

##### Lighten Blend Mode

##### Color Dodge Blend Mode

##### Color Burn Blend Mode

##### Soft Light Blend Mode

##### Hard Light Blend Mode

##### Difference Blend Mode

##### Exclusion Blend Mode

##### Hue Blend Mode

##### Saturation Blend Mode

##### Color Blend Mode

##### Luminosity Blend Mode

### Clipping to a Path













## 参考链接
**参考链接:🔗**
+ [Quartz 2D Programming Guide](https://developer.apple.com/library/archive/documentation/GraphicsImaging/Conceptual/drawingwithquartz2d/Introduction/Introduction.html#//apple_ref/doc/uid/TP30001066)
+ [Core Graphics Framework Reference](https://developer.apple.com/documentation/coregraphics)
