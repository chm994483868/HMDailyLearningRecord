# iOS《Quartz 2D Programming Guide》官方文档-Graphics Contexts

## Introduction
&emsp;Core Graphics，也被称为 Quartz 2D，是一个先进的二维绘图引擎，可用于 iOS、tvOS 和 macOS 应用程序开发。Quartz 2D 提供低级别、轻量级的 2D 渲染，无论显示或打印设备如何，都具有无与伦比的输出保真度。Quartz 2D 是分辨率和设备无关。

&emsp;Quartz 2D API 易于使用，并提供对强大功能的使用，如透明层（transparency layers）、基于路径的绘图（path-based drawing）、离屏渲染（offscreen rendering）、高级颜色管理（advanced color management）、抗锯齿渲染（anti-aliased rendering）以及 PDF 文档创建（creation）、显示（display）和解析（parsing）。

### Who Should Read This Document?
&emsp;本文档适用于需要执行以下任何任务的开发人员：

+ Draw graphics（绘制图形）
+ Provide graphics editing capabilities in an application（在应用程序中提供图形编辑功能）
+ Create or display bitmap images（创建或显示位图图像）
+ Work with PDF documents（处理 PDF 文档）

### Organization of This Document（本文档的组织结构）
&emsp;本文档分为以下章节：

+ Quartz 2D Overview 描述了页面、绘图目标、Quartz 不透明数据类型、图形状态、坐标和内存管理，并介绍了 Quartz 如何 “under the hood” 工作（如何在引擎下工作）。
+ Graphics Contexts 描述了各种绘图目标，并提供了创建各种图形上下文的分步说明。
+ Paths 讨论构成路径的基本元素，演示如何创建和绘制路径，演示如何设置剪裁区域，并解释混合模式如何影响绘制。
+ Color and Color Spaces 讨论颜色值和透明度的 alpha 值，并描述如何创建颜色空间、设置颜色、创建颜色对象和设置渲染意图。
+ Transforms 描述当前变换矩阵并说明如何修改它，说明如何设置仿射变换，说明如何在用户和设备空间之间进行转换，并提供有关 Quartz 执行的数学运算的背景信息。
+ Patterns 定义了一个模式及其部分，告诉 Quartz 如何渲染它们，并展示了如何创建彩色和模版模式。
+ Shadows 描述了什么是阴影，解释了它们是如何工作的，并演示了如何使用它们进行绘制。
+ Gradients 讨论轴向和径向渐变，并演示如何创建和使用 CGShading 和 CGGradient 对象。
+ Transparency Layers 给出了透明层的外观示例，讨论了它们的工作方式，并提供了实现它们的逐步说明。
+ Data Management in Quartz 2D 讨论了如何将数据移入和移出 Quartz。
+ Bitmap Images and Image Masks 描述了构成位图图像定义的内容，并演示了如何将位图图像用作 Quartz 绘图原语（primitive）。它还描述了可以在图像上使用的遮罩技术，并显示了在绘制图像时使用混合模式可以实现的各种效果。
+ Core Graphics Layer Drawing 描述了如何创建和使用 drawing layers 来实现高性能的图形化绘制或在屏幕外绘制（to draw offscreen）。
+ PDF Document Creation, Viewing, and Transforming 演示如何打开和查看 PDF 文档、对其应用转换、创建 PDF 文件、访问 PDF 元数据、添加链接以及添加安全功能（如密码保护）。
+ PDF Document Parsing 描述了如何使用 CGPDFScanner 和 CGPDFContentStream 对象来解析和检查 PDF 文档。
+ PostScript Conversion 概述了可在 Mac OS X 中用于将 PostScript 文件转换为 PDF 文档的函数。这些功能在 iOS 中不可用。
+ Text 描述了 Quartz 2D 对文本和 glyph 的低级支持，以及提供高级和 Unicode 文本支持的替代方案。本文还讨论了如何复制字体变体。
+ Glossary 定义了本指南中使用的术语。

### See Also
&emsp;以下是使用 Quartz 2D 的所有人的必备阅读资料：
+ [Core Graphics Framework Reference](https://developer.apple.com/documentation/coregraphics) 提供了一个完整的参考 Quartz 2D 应用程序编程接口。
+ [Color Management Overview](https://developer.apple.com/library/archive/documentation/GraphicsImaging/Conceptual/csintro/csintro_intro/csintro_intro.html#//apple_ref/doc/uid/TP30001148) 是对色彩感知原理、色彩空间和色彩管理系统的简要介绍。
+ Mailing lists。加入 [quartz-dev](https://lists.apple.com) mailing list，讨论使用 Quartz 2D 的问题。

## Overview of Quartz 2D（Quartz 2D 概述）
&emsp;Quartz 2D 是一个二维绘图引擎，可以在 iOS 环境中访问，也可以从内核之外的所有 Mac OS X 应用程序环境访问。你可以使用 Quartz 2D 应用程序编程接口（API）访问功能，如基于路径的绘图（path-based drawing）、透明绘制（painting with transparency）、着色（shading）、图形阴影（drawing shadows）、透明层（transparency layers）、颜色管理（color management）、抗锯齿渲染（anti-aliased rendering）、PDF 文档生成和 PDF 元数据访问。只要有可能，Quartz 2D 就会利用图形硬件的强大功能。

&emsp;在 MacOS X 中，Quartz 2D 可以与所有其他图形和成像技术—Core Image、Core Video、OpenGL 和 QuickTime —配合使用。可以使用 QuickTime 函数 GraphicsImportCreateCGImage 从 QuickTime 图形导入器在 Quartz 中创建图像。有关详细信息，请参见 QuickTime 框架参考。在 Mac OS X 中，在 Quartz 2D 和 Core Image 之间移动数据（Moving Data Between Quartz 2D and Core Image in Mac OS X）描述了如何向 Core Image 提供图像，Core Image 是一个支持图像处理的框架。

&emsp;类似地，在 iOS 中，Quartz 2D 使用所有可用的图形和动画技术，如 Core Animation、OpenGL ES 和 UIKit 类。

### The Page
&emsp;Quartz 2D 使用 painter 的模型进行成像。在 painter 的模型中，每个连续的绘图操作都会对输出 "canvas"（通常称为 page）应用一层 "paint"。可以通过附加的绘图操作覆盖更多的绘图来修改 page 上的绘图。不能修改 page 上绘制的对象，除非覆盖更多的绘制。此模型允许你从少量强大的原语构建极其复杂的图像。

&emsp;图 1-1 显示了 painter 模型的工作原理。为了获得图形顶部的图像，首先绘制左侧的形状，然后绘制实体形状。实心形状覆盖第一个形状，除了第一个形状的周长外，其他形状都被遮挡。图 1-1 底部按相反顺序绘制图形，首先绘制实体图形。正如你所见，在 painter 的模型中，绘画顺序很重要。

![painters_model](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/b5473fac8a0043799e793bfb04450a6c~tplv-k3u1fbpfcp-watermark.image)

&emsp;page 可以是一张真正的纸（如果输出设备是打印机）；也可以是一张虚拟的纸（如果输出设备是 PDF 文件）；甚至可以是位图图像。page 的确切性质取决于你使用的特定图形上下文。

### Drawing Destinations: The Graphics Context
&emsp;图形上下文是一种不透明的数据类型（CGContextRef），它封装了 Quartz 用于将图像绘制到输出设备（如 PDF 文件、位图或显示器上的窗口）的信息。图形上下文中的信息包括图形绘图参数和 page 上的特定于设备的绘画表示。Quartz 中的所有对象都被绘制到或包含在图形上下文中。

&emsp;可以将图形上下文视为绘图目标，如图 1-2 所示。使用 Quartz 绘制时，所有设备特定的特征都包含在所使用的特定类型的图形上下文中。换言之，你只需为同一序列的 Quartz 绘图例程提供不同的图形上下文，就可以将相同的图像绘制到不同的设备上。你不需要执行任何特定于设备的计算；Quartz 会为你执行。

&emsp;Figure 1-2  Quartz drawing destinations

![draw_destinations](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/0b76c444cb854fda9aa3e221e00bff86~tplv-k3u1fbpfcp-watermark.image)

&emsp;这些图形上下文可用于你的应用程序：
+ （bitmap graphics context）位图图形上下文允许你在位图中绘制 RGB 颜色、CMYK 颜色或灰度。位图是像素的矩形阵列（或光栅），每个像素代表图像中的一个点。位图图像（Bitmap images）也称为采样图像（sampled images）。请参见 Creating a Bitmap Graphics Context。

+ （PDF graphics context）PDF 图形上下文允许你创建 PDF 文件。在 PDF 文件中，drawing 将作为命令序列保留。PDF 文件和位图之间存在一些显著的差异：
  + 与位图不同，PDF文件可能包含多个 page。
  + 从不同设备上的 PDF 文件绘制 page 时，生成的图像将针对该设备的显示特性进行优化。
  + PDF 文件本质上是独立于分辨率的，在不牺牲图像细节的情况下，可以无限地增大或减小文件的大小。用户对位图图像的感知质量与要查看位图的分辨率有关。
请参见 Creating a PDF Graphics Context。

+ （window graphics context）window 图形上下文是可用于绘制到窗口中的图形上下文。请注意，因为 Quartz 2D 是一个图形引擎，而不是一个窗口管理系统，所以你可以使用其中一个应用程序框架来获取窗口的图形上下文。有关详细信息，请参见 Creating a Window Graphics Context in Mac OS X 。

+ （layer context ）图层上下文（CGLayerRef）是与其他图形上下文关联的屏幕外图形目标。它的设计是为了在将层绘制到创建它的图形上下文时获得最佳性能。对于屏幕外绘制，图层上下文可能比位图图形上下文更好。请参见 Core Graphics Layer Drawing。

+ （PostScript graphics context）如果要在 Mac OS X 中打印，则将内容发送到由打印框架管理的 PostScript 图形上下文。有关详细信息，请参见 Obtaining a Graphics Context for Printing。

### Quartz 2D Opaque Data Types
&emsp;除了图形上下文之外，Quartz 2D API 还定义了各种不透明的数据类型。因为 API 是 Core Graphics 框架的一部分，所以对其进行操作的数据类型和例程使用 CG 前缀。

&emsp;Quartz 2D 从不透明的数据类型创建对象，应用程序对这些数据类型进行操作以实现特定的图形输出。图 1-3 显示了将绘图操作应用于 Quartz 2D 提供的三个对象时可以获得的各种结果。例如：

+ 通过创建 PDF 页面对象，对图形上下文应用旋转操作，并要求 Quartz 2D 将页面绘制到图形上下文，可以旋转和显示 PDF 页面。
+ 可以通过创建图案对象、定义构成图案的形状，以及设置 Quartz 2D 以在图案绘制到图形上下文时将图案用作绘画来绘制图案。
+ 通过创建着色对象，提供确定着色中每个点的颜色的函数，然后要求 Quartz 2D 将着色用作填充颜色，可以使用轴向或径向着色填充区域。

&emsp;Figure 1-3  Opaque data types are the basis of drawing primitives in Quartz 2D

![drawing_primitives](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/f2598ce6b52f4764aedf65690eb7302a~tplv-k3u1fbpfcp-watermark.image)

&emsp;Quartz 2D 中可用的不透明数据类型包括：

+ CGPathRef，用于矢量图形以创建填充或笔划的路径。请参见 Paths。
+ CGImageRef，用于根据你提供的示例数据表示位图图像和位图图像掩码。请参见 Bitmap Images and Image Masks。
+ CGLayerRef，用于表示可用于重复绘制（如背景或图案）和屏幕外绘制的绘图层。请参见 Core Graphics Layer Drawing。
+ CGPatternRef，用于重复绘制。请参见 Patterns。
+ CGShadingRef 和 CGGradientRef，用于绘制渐变。请参见 Gradients。
+ CGFunctionRef，用于定义接受任意数量浮点参数的回调函数。为着色（shading）创建渐变（gradients）时，将使用此数据类型。请参见 Gradients。
+ CGColorRef 和 CGColorSpaceRef，用来告诉 Quartz 如何解释颜色。请参见 Color and Color Spaces。
+ CGImageSourceRef 和 CGImageDestinationRef，用于将数据移入和移出 Quartz。请参见 Data Management in Quartz 2D 和 Image I/O Programming Guide。
+ CGFontRef，用于绘制文本。请参见 Text。
+ CGPDFDictionaryRef、CGPDFObjectRef、CGPDFPageRef、CGPDFStream、CGPDFStringRef 和 CGPDFArrayRef，提供对 PDF 元数据的访问。请参见 PDF Document Creation, Viewing, and Transforming。
+ CGPDFScannerRef 和 CGPDFContentStreamRef，用于解析 PDF 元数据。请参见 PDF Document Parsing。
+ CGPSConverterRef，用于将 PostScript 转换为 PDF。它在 iOS 中不可用。请参见 PostScript Conversion。

### Graphics States
&emsp;Quartz 根据当前 graphics state 中的参数修改绘图操作的结果。graphics state 包含参数，否则这些参数将作为绘图例程（drawing routines）的参数。绘制到图形上下文的例程参考 graphics state 以确定如何呈现其结果。例如，当你调用函数来设置填充颜色时，你正在修改存储在当前 graphics state 中的值。当前 graphics state 的其他常用元素包括线宽、当前位置和文本字体大小。

&emsp;graphics context 包含一堆（a stack of） graphics states。（应该翻译为 “图形上下文包含一个图形状态堆栈”）当 Quartz 创建图形上下文时，堆栈为空。保存图形状态时，Quartz 会将当前图形状态的副本推入到堆栈上。恢复图形状态时，Quartz 会将图形状态从堆栈顶部弹出。弹出状态变为当前图形状态。

&emsp;要保存当前图形状态，请使用函数 `CGContextSaveGState` 将当前图形状态的副本推入到堆栈上。要还原以前保存的图形状态，请使用函数 `CGContextRestoreGState` 将当前图形状态替换为堆栈顶部的图形状态。

&emsp;请注意，并非当前绘图环境的所有 aspects 都是图形状态的元素。例如，当前路径不被视为图形状态的一部分，因此在调用函数 `CGContextSaveGState` 时不会保存。调用此函数时保存的图形状态参数如表 1-1 所示。

&emsp;Table 1-1  Parameters that are associated with the graphics state（与图形状态关联的参数）
| Parameters | Discussed in this chapter |
| --- | --- |
| Current transformation matrix (CTM) | Transforms |
| Clipping area | Paths |
| Line: width, join, cap, dash, miter limit | Paths |
| Accuracy of curve estimation (flatness) | Paths |
| Anti-aliasing setting | Graphics Contexts |
| Color: fill and stroke settings | Color and Color Spaces |
| Alpha value (transparency) | Color and Color Spaces |
| Rendering intent | Color and Color Spaces |
| Color space: fill and stroke settings | Color and Color Spaces |
| Text: font, font size, character spacing, text drawing mode | Text |
| Blend mode | Paths and Bitmap Images and Image Masks |

### Quartz 2D Coordinate Systems
&emsp;如图 1-4 所示，坐标系定义了用于表示要在页面上绘制的对象的位置和大小的位置范围。可以在用户空间坐标系或更简单的用户空间中指定图形的位置和大小。坐标被定义为浮点值。

&emsp;Figure 1-4  The Quartz coordinate system

![quartz_coordinates](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/b454ea014df644feb3ca038f87927c5f~tplv-k3u1fbpfcp-watermark.image)

&emsp;由于不同的设备具有不同的基本成像能力，因此必须以独立于设备的方式定义图形的位置和大小。例如，屏幕显示设备可以每英寸显示不超过 96 个像素，而打印机可以每英寸显示 300 个像素。如果在设备级别定义坐标系（在本例中为 96 像素或 300 像素），则在该空间中绘制的对象无法在其他设备上复制（reproduced 复制、重现、再版），而不会产生可见的失真。它们会显得太大或太小。

&emsp;Quartz 通过使用 current transformation matrix（CTM）将单独的坐标系用户空间映射到输出设备空间的坐标系来实现设备独立性。矩阵是用来有效描述一组相关方程的数学结构。当前变换矩阵是一种特殊类型的矩阵，称为仿射变换，它通过应用平移、旋转和缩放操作（移动、旋转和调整坐标系大小的计算）将点从一个坐标空间映射到另一个坐标空间。

&emsp;当前变换矩阵还有一个次要用途：它允许你变换对象的绘制方式。例如，要绘制旋转 45 度的长方体，请先旋转页面的坐标系（CTM），然后再绘制长方体。Quartz 使用旋转坐标系绘制到输出设备。

&emsp;用户空间中的点由坐标对（x，y）表示，其中 x 表示沿水平轴（左和右）的位置，y 表示垂直轴（上和下）。用户坐标空间的原点是点（0,0）。原点位于页面的左下角，如图 1-4 所示。在 Quartz 的默认坐标系中，x 轴从页面的左侧向右侧移动时会增加。当 y 轴从页面底部向顶部移动时，其值会增加。

&emsp;一些技术使用不同于 Quartz 使用的默认坐标系来设置图形上下文。相对于 Quartz，这种坐标系是一种修改后的坐标系，在执行某些 Quartz 绘图操作时必须进行补偿。最常见的修改坐标系将原点放置在上下文的左上角，并将 y 轴更改为指向页面底部。你可能会看到使用此特定坐标系的几个地方如下所示：

+ 在 Mac OS X中，NSView 的子类重写其 isFlipped 方法以返回 YES。
+ 在 iOS 中，UIView 返回的绘图上下文。
+ 在 iOS 中，通过调用 `UIGraphicsBeginImageContextWithOptions` 函数创建的图形上下文。

&emsp;UIKit 返回具有修改坐标系的 Quartz 图形上下文的原因是 UIKit 使用不同的默认坐标约定；它将转换应用于它创建的 Quartz 上下文，以便它们与它的约定相匹配。如果应用程序希望使用相同的绘图例程绘制 UIView 对象和 PDF 图形上下文（由 Quartz 创建并使用默认坐标系），则需要应用变换，以便 PDF 图形上下文接收相同的修改坐标系。为此，应用一个转换，将原点转换到 PDF 上下文的左上角，并将 y 坐标缩放 -1（应该是乘以负 1）。

&emsp;使用缩放变换来消除 y 坐标会改变 Quartz 绘图中的一些约定。例如，如果调用 `CGContextDrawImage` 将图像绘制到上下文中，则在将图像绘制到目标中时，转换会对图像进行修改。类似地，路径绘制例程接受指定在默认坐标系中是顺时针还是逆时针方向绘制圆弧的参数。如果修改了坐标系，结果也会被修改，就像图像在镜子中反射一样。在图 1-5 中，将相同的参数传递到 Quartz 会在默认坐标系中产生一个顺时针的弧，在 y 坐标被变换取反后产生一个逆时针的弧。

&emsp;Figure 1-5  Modifying the coordinate system creates a mirrored image.（修改坐标系将创建一个镜像图像。）

![flipped_coordinates](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/c76ff5ca622143a9b28b038db19904c1~tplv-k3u1fbpfcp-watermark.image)

&emsp;由你的应用程序来调整它对应用了转换的上下文所做的任何 Quartz 调用。例如，如果希望图像或 PDF 正确地绘制到图形上下文中，应用程序可能需要临时调整图形上下文的 CTM。在 iOS 中，如果使用 UIImage 对象包装创建的 CGImage 对象，则不需要修改 CTM。UIImage 对象自动补偿 UIKit 应用的修改后的坐标系。

> Important: 如果你计划在 iOS 上直接针对 Quartz 编写应用程序，那么上面的讨论对于理解这一点非常重要，但这还不够。在 iOS 3.2 及更高版本上，当 UIKit 为应用程序创建图形上下文时，它还会对上下文进行其他更改，以匹配默认的 UIKit 约定。特别是，不受 CTM 影响的模式和阴影将分别进行调整，以便它们的约定与 UIKit 的坐标系匹配。在这种情况下，你的应用程序无法使用与 CTM 等效的机制来更改 Quartz 创建的上下文，以匹配 UIKit 提供的上下文的行为；你的应用程序必须识别它所引入的上下文类型，并调整其行为以匹配上下文的期望。

### Memory Management: Object Ownership
&emsp;Quartz 使用 Core Foundation 内存管理模型，其中对象是引用计数的。创建 Core Foundation 对象时，引用计数为 1。你可以通过调用函数来保留对象来增加引用计数，通过调用函数来释放对象来减少引用计数。当引用计数减少到 0 时，对象被释放。此模型允许对象安全地共享对其他对象的引用。

&emsp;需要牢记一些简单的规则：

+ 如果你创建或复制一个对象，你就拥有它，因此你必须释放它。也就是说，一般来说，如果从名称中带有 “Create” 或 “Copy” 字样的函数中获取对象，则必须在处理完该对象后释放该对象。否则，会导致内存泄漏。
+ 如果从名称中不包含单词 “Create” 或 “Copy” 的函数中获取对象，则不拥有对该对象的引用，并且不能释放该对象。该对象将在将来某个时候由其所有者释放。
+ 如果你不拥有某个对象并且需要将其保留在周围，则必须保留该对象，并在处理完该对象后将其释放。可以使用特定于对象的 Quartz 2D 函数来保留和释放该对象。例如，如果收到对 CGColorspace 对象的引用，则可以使用函数 `CGColorSpaceRetain` 和 `CGColorSpaceRelease` 根据需要保留和释放该对象。你也可以使用 Core Foundation  础函数 `CFRetain` 和 `CFRelease`，但必须注意不要将 NULL 传递给这些函数。

## Graphics Contexts
&emsp;图形上下文表示绘图目标。它包含绘图参数和绘图系统执行任何后续绘图命令所需的所有设备特定信息。图形上下文定义基本绘图属性，例如绘制时要使用的颜色、剪裁区域、线宽和样式信息、字体信息、合成选项以及其他一些属性。

&emsp;你可以通过使用 Quartz 上下文创建函数或使用 Mac OS X 框架或 iOS 中的 UIKit 框架提供的更高级函数来获取图形上下文。Quartz 为各种风格的 Quartz 图形上下文提供函数，包括位图和 PDF，你可以使用它们创建自定义内容。

&emsp;本章介绍如何为各种绘图目标创建图形上下文。图形上下文在代码中由数据类型 CGContextRef 表示，CGContextRef 是一种不透明的数据类型。获取图形上下文后，可以使用 Quartz 2D 函数绘制到上下文，对上下文执行操作（如转换），并更改图形状态（graphics state）参数，如线宽和填充颜色。

### Drawing to a View Graphics Context in iOS
&emsp;要在 iOS 应用程序中绘制到屏幕，需要设置 UIView 对象并实现其 `drawRect:` 方法来执行绘制。当视图在屏幕上可见并且其内容需要更新时，将调用视图的 `drawRect:` 方法。在调用自定义 `drawRect:` 方法之前，视图对象会自动配置其绘图环境，以便代码可以立即开始绘图。作为此配置的一部分，UIView 对象为当前图形环境创建图形上下文（CGContextRef 不透明类型）。通过调用 UIKit 函数 `UIGraphicsGetCurrentContext`，可以在 `drawRect:` 方法中获得此图形上下文。

&emsp;整个 UIKit 使用的默认坐标系与 Quartz 使用的坐标系不同。在 UIKit 中，原点位于左上角，正 y 值向下。UIView 对象通过将原点平移到视图的左上角并将 y 轴乘以 -1 来反转 y 轴，从而修改 Quartz 图形上下文的 CTM 以匹配 UIKit 约定。有关修改的坐标系以及绘图代码中的含义的详细信息，请参见 Quartz 2D Coordinate Systems。

&emsp;UIView 对象的详细描述信息在 View Programming Guide for iOS 中。

### Creating a Window Graphics Context in Mac OS X
&emsp;在 Mac OS X 中绘图时，需要创建一个适合所用框架的 window 图形上下文。Quartz 2D API 本身不提供获取 windows 图形上下文的函数。相反，你可以使用 Cocoa 框架来获取在 Cocoa 中创建的窗口的上下文。

&emsp;你可以使用以下代码从 Cocoa 应用程序的 `drawRect:` 例程中获取 Quartz 图形上下文：
```c++
CGContextRef myContext = [[NSGraphicsContext currentContext] graphicsPort];
```
&emsp;方法 `currentContext` 返回当前线程的 NSGraphicsContext 实例。`graphicsPort` 方法返回 receiver 表示的低级别、特定于平台的图形上下文，这是一个 Quartz 图形上下文。（不要被方法名搞混了，它们是历史的。）更多信息请参见 NSGraphicsContext Class Reference。

&emsp;获取图形上下文后，可以在 Cocoa 应用程序中调用任何 Quartz 2D 绘图函数。你也可以将 Quartz 2D 调用与 Cocoa 绘图调用混合使用。如图 2-1 所示，你可以看到 Quartz 2D 绘制到 Cocoa 视图的示例。该图形由两个重叠的矩形组成，一个不透明的红色矩形和一个部分透明的蓝色矩形。你将了解有关颜色和颜色空间中透明度的更多信息。控制多少颜色可以 “透视” 颜色的能力是 Quartz 2D 的标志性功能之一。

&emsp;Figure 2-1  A view in the Cocoa framework that contains Quartz drawing（Cocoa 框架中包含 Quartz 绘图的视图）

![cocoa_draw](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/45338667aba140b6a47ba655b68786cb~tplv-k3u1fbpfcp-watermark.image)

&emsp;要创建图 2-1 中的图形，首先创建一个 Cocoa 应用程序 Xcode 项目。在 Interface Builder 中，将自定义视图拖到窗口中并将其子类化。然后为子类视图编写一个实现，类似于清单 2-1 所示。对于本例，子类视图命名为 MyQuartzView。视图的 `drawRect:` 方法包含所有的 Quartz 绘图代码。下面列出了每一行代码的详细说明。

> Note: 每次需要绘制视图时，都会自动调用 NSView 类的 `drawRect:` 方法。要了解有关重写 `drawRect:` 方法的更多信息，请参见 NSView Class Reference。

&emsp;Listing 2-1  Drawing to a window graphics context
```c++
@implementation MyQuartzView
 
- (id)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame:frameRect];
    return self;
}
 
- (void)drawRect:(NSRect)rect
{
    CGContextRef myContext = [[NSGraphicsContext // 1
                                currentContext] graphicsPort];
                                
   // ********** Your drawing code here ********** // 2
    CGContextSetRGBFillColor (myContext, 1, 0, 0, 1); // 3
    CGContextFillRect (myContext, CGRectMake (0, 0, 200, 100 )); // 4
    CGContextSetRGBFillColor (myContext, 0, 0, 1, .5); // 5
    CGContextFillRect (myContext, CGRectMake (0, 0, 100, 200)); // 6
  }
 
@end
```
&emsp;代码是这样的：
1. 获取视图的图形上下文。
2. 在这里插入绘图代码。接下来的四行代码是使用 Quartz 2D 函数的示例。
3. 设置完全不透明的红色填充颜色。有关颜色和 Alpha（设置不透明度）的信息，请参见 Color and Color Spaces。
4. 填充一个原点为（0, 0）且宽度为 200 且高度为 100 的矩形。有关绘制矩形的信息，请参见 Paths。
5. 设置部分为透明的蓝色填充颜色。
6. 填充一个原点为（0, 0）且宽度为 100 且高度为 200 的矩形。

### Creating a PDF Graphics Context
&emsp;创建 PDF 图形上下文并绘制到该上下文时，Quartz 会将图形记录为一系列写入文件的 PDF 绘图命令。你可以为 PDF 输出提供一个位置和一个默认的媒体框—一个指定页面边界的矩形。图 2-2 显示了绘制到 PDF 图形上下文，然后在预览中打开生成的 PDF 的结果。

&emsp;Figure 2-2  A PDF created by using CGPDFContextCreateWithURL（使用 CGPDFContextCreateWithURL 创建的 PDF）

![pdf_context_draw](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/1b9c4ac6ffb4485dbf5803807eba7c07~tplv-k3u1fbpfcp-watermark.image)

&emsp;Quartz 2D API 提供了两个创建 PDF 图形上下文的函数：

+ `CGPDFContextCreateWithURL`，当你要将 PDF 输出的位置指定为 Core Foundation URL 时使用它。清单 2-2 展示了如何使用此函数创建 PDF 图形上下文。
+ `CGPDFContextCreate`，当你希望将 PDF 输出发送给数据使用者时使用它。（有关更多信息，请参见 Data Management in Quartz 2D）清单 2-3 显示了如何使用此函数创建 PDF 图形上下文。

&emsp;每个清单后面都有对每行代码的详细解释。

> iOS Note: iOS 中的 PDF 图形上下文使用 Quartz 提供的默认坐标系，而不应用 transform 来匹配 UIKit 坐标系。如果应用程序计划在 PDF 图形上下文和 UIView 对象提供的图形上下文之间共享图形代码，则应用程序应修改 PDF 图形上下文的 CTM 以修改坐标系。

&emsp;Listing 2-2  Calling CGPDFContextCreateWithURL to create a PDF graphics context（调用 CGPDFContextCreateWithURL 创建 PDF 图形上下文）
```c++
CGContextRef MyPDFContextCreate (const CGRect *inMediaBox, CFStringRef path) {
    CGContextRef myOutContext = NULL;
    CFURLRef url;
 
    url = CFURLCreateWithFileSystemPath (NULL, // 1
                                path,
                                kCFURLPOSIXPathStyle,
                                false);
    if (url != NULL) {
        myOutContext = CGPDFContextCreateWithURL (url, // 2
                                        inMediaBox,
                                        NULL);
        CFRelease(url); // 3
    }
    return myOutContext; // 4
}
```
&emsp;代码是这样的：
1. 调用 Core Foundation 函数，从提供给 `MyPDFContextCreate` 函数的 CFString 对象创建 CFURL 对象。传递 NULL 作为第一个参数以使用默认分配器。你还需要指定一个路径样式，在本例中是 POSIX 样式的路径名。
2. 调用 Quartz 2D 函数，使用刚刚创建的 PDF 位置（作为 CFURL 对象）和指定 PDF 边界的矩形创建 PDF 图形上下文。矩形（CGRect）被传递给 `MyPDFContextCreate` 函数，是 PDF 的默认页面媒体边界框。
3. 释放 CFURL 对象。
4. 返回 PDF 图形上下文。当不再需要图形上下文时，调用者必须释放它。

&emsp;Listing 2-3  Calling CGPDFContextCreate to create a PDF graphics context（调用 CGPDFContextCreate 创建 PDF 图形上下文）
```c++
CGContextRef MyPDFContextCreate (const CGRect *inMediaBox, CFStringRef path) {
    CGContextRef        myOutContext = NULL;
    CFURLRef            url;
    CGDataConsumerRef   dataConsumer;
 
    url = CFURLCreateWithFileSystemPath (NULL, // 1
                                        path,
                                        kCFURLPOSIXPathStyle,
                                        false);
 
    if (url != NULL) {
        dataConsumer = CGDataConsumerCreateWithURL (url);// 2
        if (dataConsumer != NULL) {
            myOutContext = CGPDFContextCreate (dataConsumer, // 3
                                        inMediaBox,
                                        NULL);
            CGDataConsumerRelease (dataConsumer);// 4
        }
        CFRelease(url);// 5
    }
    return myOutContext;// 6
}
```
&emsp;代码是这样的：
1. 调用 Core Foundation 函数，从提供给 `MyPDFContextCreate` 函数的 CFString 对象创建 CFURL 对象。传递 NULL 作为第一个参数以使用默认分配器。你还需要指定一个路径样式，在本例中是 POSIX 样式的路径名。
2. 使用 CFURL 对象创建 Quartz dataConsumer 对象。如果你不想使用 CFURL 对象（例如，你希望将 PDF 数据放置在 CFURL 对象无法指定的位置），则可以从应用程序中实现的一组回调函数中创建 dataConsumer。有关更多信息，请参见 Data Management in Quartz 2D。
3. 调用 Quartz 2D 函数创建 PDF 图形上下文，将 dataConsumer 和传递给 `MyPDFContextCreate` 函数的矩形（CGRect 类型）作为参数传递。此矩形是 PDF 的默认页面媒体边界框。
4. 释放 dataConsumer。
5. 释放 CFURL 对象。
6. 返回 PDF 图形上下文。当不再需要图形上下文时，调用者必须释放它。

&emsp;清单 2-4 展示了如何调用 MyPDFContextCreate 例程并绘制到它。下面列出了每一行代码的详细说明。

&emsp;Listing 2-4  Drawing to a PDF graphics context
```c++
   CGRect mediaBox; // 1

   mediaBox = CGRectMake (0, 0, myPageWidth, myPageHeight); // 2
   myPDFContext = MyPDFContextCreate (&mediaBox, CFSTR("test.pdf")); // 3

   CFStringRef myKeys[1]; // 4
   CFTypeRef myValues[1];
   myKeys[0] = kCGPDFContextMediaBox;
   myValues[0] = (CFTypeRef)CFDataCreate(NULL, (const UInt8 *)&mediaBox, sizeof(CGRect));
   CFDictionaryRef pageDictionary = CFDictionaryCreate(NULL, (const void **)myKeys,
                                                       (const void **)myValues, 1,
                                                       &kCFTypeDictionaryKeyCallBacks,
                                                       &kCFTypeDictionaryValueCallBacks);
   CGPDFContextBeginPage(myPDFContext, &pageDictionary); // 5 开始
       // ********** Your drawing code here ********** // 6
       CGContextSetRGBFillColor (myPDFContext, 1, 0, 0, 1);
       CGContextFillRect (myPDFContext, CGRectMake (0, 0, 200, 100 ));
       CGContextSetRGBFillColor (myPDFContext, 0, 0, 1, .5);
       CGContextFillRect (myPDFContext, CGRectMake (0, 0, 100, 200 ));
   CGPDFContextEndPage(myPDFContext); // 7 结束
   CFRelease(pageDictionary); // 8
   CFRelease(myValues[0]);
   CGContextRelease(myPDFContext);
```
&emsp;代码是这样的：
1. 声明用于定义 PDF 媒体框（mediaBox）的矩形的变量。
2. 将媒体框（mediaBox）的原点设置为（0, 0），将宽度和高度设置为应用程序提供的变量。
3. 调用函数 `MyPDFContextCreate`（参见清单 2-3）以获取 PDF 图形上下文，并提供媒体框（mediaBox）和路径名。宏 CFSTR 将字符串转换为 CFStringRef 数据类型。
4. 使用页面选项设置 dictionary。在本例中，仅指定媒体框（kCGPDFContextMediaBox）。你不必传递用于设置 PDF 图形上下文的矩形。你在此处添加的媒体框将取代你传递的用于设置 PDF 图形上下文的矩形。
5. 表示页面开始。此函数用于面向页面的图形，这就是 PDF 绘图。
6. 调用 Quartz 2D 绘图函数。将此代码和以下四行代码替换为适合你的应用程序的图形代码。
7. 指示 PDF 页面结束。
8. 当不再需要 dictionary 和 PDF 图形上下文时，释放它们。

&emsp;你可以将任何内容写入适合你的应用程序的 PDF 图像、文本、路径图，还可以添加链接和加密。有关更多信息，请参见 PDF Document Creation, Viewing, and Transforming。

### Creating a Bitmap Graphics Context
&emsp;位图图形上下文接受指向包含位图存储空间的内存缓冲区的指针。在位图图形上下文中绘制时，缓冲区将更新。释放图形上下文后，将以指定的像素格式完全更新位图。

> Note: 位图图形上下文有时用于在屏幕外绘制。在决定为此目的使用位图图形上下文之前，请参见 Core Graphics Layer Drawing。 CGLayer 对象（CGLayerRef）已针对屏幕外绘图进行了优化，因为 Quartz 尽可能在视频卡（GPU）上缓存图层。

> iOS Note: iOS 应用程序应该使用 `UIGraphicsBeginImageContextWithOptions` 函数，而不是使用这里描述的低级 Quartz 函数。如果应用程序使用 Quartz 创建屏幕外位图，则位图图形上下文使用的坐标系是默认的 Quartz 坐标系。相反，如果应用程序通过调用函数 `UIGraphicsBeginImageContextWithOptions` 来创建图像上下文，则 UIKit 对上下文的坐标系应用与对 UIView 对象的图形上下文相同的转换。这样，应用程序就可以使用相同的绘图代码，而不必担心不同的坐标系。虽然应用程序可以手动调整坐标变换矩阵以获得正确的结果，但在实践中，这样做对性能没有好处。

&emsp;使用函数 `CGBitmapContextCreate` 创建位图图形上下文。此函数采用以下参数：
```c++
CGContextRef CGBitmapContextCreate(void *data, size_t width, size_t height, size_t bitsPerComponent, size_t bytesPerRow, CGColorSpaceRef space, uint32_t bitmapInfo);
```
+ data。提供指向内存中要渲染图形的目标的指针。此内存块的大小应至少为（bytesPerRow * height）字节。
+ width。指定位图的宽度（以像素为单位）。
+ height。指定位图的高度（以像素为单位）。
+ bitsPerComponent。指定要用于内存中像素的每个组件的位数。例如，对于 32 位像素格式和 RGB 颜色空间，可以为每个组件指定 8 位的值。请参见 Supported Pixel Formats。（RGB 24 RGBA 32）
+ bytesPerRow。指定位图的每一行要使用的内存字节数。

> Tip: 创建位图图形上下文时，如果确保 data 和 bytesPerRow 是 16 字节对齐的，则会获得最佳性能。

+ colorspace。用于位图上下文的颜色空间。创建位图图形上下文时，可以提供 Gray、RGB、CMYK 或 NULL 颜色空间。有关颜色空间和颜色管理原则的详细信息，请参见 Color Management Overview。有关在 Quartz 中创建和使用颜色空间的信息，请参见 Color and Color Spaces。有关支持的颜色空间的信息，请参见 Bitmap Images and Image Masks 章中的 Color Spaces and Bitmap Layout。
+ bitmapInfo。位图布局信息，用 CGBitmapInfo 常量表示，指定位图是否应包含 alpha 分量、alpha 分量（如果有）在像素中的相对位置、alpha 分量是否是预乘的以及颜色分量是整数值还是浮点值。有关这些常量是什么、何时使用以及位图图形上下文和图像的 Quartz 支持的像素格式的详细信息，请参见 Bitmap Images and Image Masks 章中的 Color Spaces and Bitmap Layout。

&emsp;清单 2-5 展示了如何创建位图图形上下文。在生成的位图图形上下文中绘图时，Quartz 会将绘图作为位图数据记录在指定的内存块中。下面列出了每一行代码的详细说明。

&emsp;Listing 2-5  Creating a bitmap graphics context
```c++
CGContextRef MyCreateBitmapContext (int pixelsWide, int pixelsHigh) {
    CGContextRef    context = NULL;
    CGColorSpaceRef colorSpace;
    void *          bitmapData;
    int             bitmapByteCount;
    int             bitmapBytesPerRow;
 
    bitmapBytesPerRow   = (pixelsWide * 4);// 1
    bitmapByteCount     = (bitmapBytesPerRow * pixelsHigh);
 
    colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);// 2
    bitmapData = calloc(bitmapByteCount, sizeof(uint8_t) );// 3
    if (bitmapData == NULL)
    {
        fprintf (stderr, "Memory not allocated!");
        return NULL;
    }
    context = CGBitmapContextCreate(bitmapData,// 4
                                    pixelsWide,
                                    pixelsHigh,
                                    8,      // bits per component
                                    bitmapBytesPerRow,
                                    colorSpace,
                                    kCGImageAlphaPremultipliedLast);
    if (context== NULL)
    {
        free(bitmapData);// 5
        fprintf(stderr, "Context not created!");
        return NULL;
    }
    CGColorSpaceRelease(colorSpace);// 6
 
    return context;// 7
}
```
&emsp;代码是这样的：
1. 声明一个变量以表示每行的字节数。本例中位图中的每个像素由 4 个字节表示；红、绿、蓝 和 alpha 各 8 位。
2. 创建通用 RGB 颜色空间。也可以创建 CMYK 颜色空间。有关更多信息以及通用颜色空间与设备相关颜色空间的讨论，请参见 Color and Color Spaces。
3. 调用 calloc 函数来创建并清除存储位图数据的内存块。本例创建一个 32 位 RGBA 位图（即，每像素 32 位的数组，每个像素包含 8 位的红、绿、蓝和 alpha 信息）。位图中的每个像素占用 4 字节的内存。在 Mac OS X 10.6 和 iOS 4 中，如果将 NULL 作为位图数据传递，则可以省略此步骤，Quartz 会自动为位图分配空间。
4. 创建位图图形上下文，提供位图数据、位图的宽度和高度、每个组件的位数、每行的字节数、颜色空间以及指定位图是否应包含 alpha 通道及其在像素中的相对位置的常量。常数 kCGImageAlphaPremultipliedLast 表示 alpha 分量存储在每个像素的最后一个字节中，并且颜色分量已经与该 alpha 值相乘。有关预乘 alpha 的详细信息，请参见 The Alpha Value。
5. 如果由于某种原因没有创建上下文（创建失败），请释放为位图数据分配的内存。
6. 释放颜色空间。
7. 返回位图图形上下文。当不再需要图形上下文时，调用者必须释放它。

&emsp;清单 2-6 显示了调用 `MyCreateBitmapContext` 来创建位图图形上下文的代码，使用位图图形上下文来创建 CGImage 对象，然后将结果图像绘制到窗口图形上下文。图 2-3 显示了绘制到窗口的图像。下面列出了每一行代码的详细说明。

&emsp;Listing 2-6  Drawing to a bitmap graphics context
```c++
   CGRect myBoundingBox; // 1

   myBoundingBox = CGRectMake(0, 0, myWidth, myHeight); // 2
   myBitmapContext = MyCreateBitmapContext(400, 300); // 3
   
   // ********** Your drawing code here ********** // 4
   CGContextSetRGBFillColor(myBitmapContext, 1, 0, 0, 1);
   CGContextFillRect(myBitmapContext, CGRectMake (0, 0, 200, 100));
   CGContextSetRGBFillColor(myBitmapContext, 0, 0, 1, .5);
   CGContextFillRect(myBitmapContext, CGRectMake (0, 0, 100, 200));
   
   myImage = CGBitmapContextCreateImage(myBitmapContext); // 5
   CGContextDrawImage(myContext, myBoundingBox, myImage); // 6
   
   char *bitmapData = CGBitmapContextGetData(myBitmapContext); // 7
   CGContextRelease(myBitmapContext); // 8
   if (bitmapData) free(bitmapData); // 9
   CGImageRelease(myImage); // 10
```
&emsp;代码是这样的：
1. 声明一个变量以存储边界框的原点和尺寸，Quartz 将在其中绘制从位图图形上下文创建的图像。
2. 将边界框的原点设置为（0, 0），将宽度和高度设置为先前声明的变量，但其声明在此代码中未显示。
3. 调用应用程序提供的函数 `MyCreateBitmapContext`（参见清单 2-5）来创建一个宽 400 像素、高 300 像素的位图上下文。你可以使用适合你的应用程序的任何尺寸创建位图图形上下文。
4. 调用 Quartz 2D 函数以绘制到位图图形上下文中。你将用适合你的应用程序的绘图代码替换此代码和接下来的四行代码。
5. 从位图图形上下文创建 Quartz 2D 图像（CGImageRef）。
6. 将图像绘制到窗口图形上下文中由边界框（myBoundingBox）指定的位置。边界框（myBoundingBox）指定在用户空间中绘制图像的位置和尺寸。
7. 获取与位图图形上下文关联的位图数据。
8. 在不再需要时释放位图图形上下文。
9. 释放位图数据（如果存在）。
10. 当不再需要时释放图像。

&emsp;Figure 2-3  An image created from a bitmap graphics context and drawn to a window graphics context（从位图图形上下文创建并绘制到窗口图形上下文的图像）

![cocoa_draw1](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/9aeefecb42c54274a8d4a581dda90f40~tplv-k3u1fbpfcp-watermark.image)

#### Supported Pixel Formats
&emsp;表 2-1 总结了位图图形上下文支持的像素格式、相关颜色空间（cs）以及该格式首次可用的 Mac OS X 版本。像素格式指定为每像素位（bits per pixel）（bpp）和每分量位（bits per component）（bpc）。该表还包括与该像素格式相关联的位图信息常量。有关每个位图信息格式常量表示什么的详细信息，请参见 CGImage Reference 。

&emsp;Table 2-1  Pixel formats supported for bitmap graphics contexts（位图图形上下文支持的像素格式）

| CS | Pixel format and bitmap information constant | Availability |
| --- | --- | --- |
| Null | 8 bpp, 8 bpc, kCGImageAlphaOnly 没有颜色数据，只有一个 Alpha 通道 | Mac OS X, iOS |
| Gray | 8 bpp, 8 bpc,kCGImageAlphaNone 没有 Alpha 通道 | Mac OS X, iOS |
| Gray | 8 bpp, 8 bpc,kCGImageAlphaOnly 没有颜色数据，只有一个 Alpha 通道 | Mac OS X, iOS |
| Gray | 16 bpp, 16 bpc, kCGImageAlphaNone 没有 Alpha 通道 | Mac OS X |
| Gray | 32 bpp, 32 bpc, kCGImageAlphaNone \| kCGBitmapFloatComponents 没有 Alpha 通道且位图的组成部分是浮点值 | Mac OS X |
| RGB | 16 bpp, 5 bpc, kCGImageAlphaNoneSkipFirst 没有 Alpha 通道。如果像素的总大小大于颜色空间中颜色分量数量所需的空间，则忽略最高有效位 | Mac OS X, iOS |
| RGB | 32 bpp, 8 bpc, kCGImageAlphaNoneSkipFirst 没有 Alpha 通道。如果像素的总大小大于颜色空间中颜色分量数量所需的空间，则忽略最高有效位 | Mac OS X, iOS |
| RGB | 32 bpp, 8 bpc, kCGImageAlphaNoneSkipLast 没有 Alpha 通道 | Mac OS X, iOS |
| RGB | 32 bpp, 8 bpc, kCGImageAlphaPremultipliedFirst Alpha 分量存储在每个像素的最高有效位中，并且颜色分量已经乘以该 Alpha 值。例如，premultiplied ARGB | Mac OS X, iOS |
| RGB | 32 bpp, 8 bpc, kCGImageAlphaPremultipliedLast Alpha 分量存储在每个像素的最低有效位中，并且颜色分量已经与该 Alpha 值相乘。例如，premultiplied RGBA | Mac OS X, iOS |
| RGB | 64 bpp, 16 bpc, kCGImageAlphaPremultipliedLast Alpha 分量存储在每个像素的最低有效位中，并且颜色分量已经与该 Alpha 值相乘。例如，premultiplied RGBA | Mac OS X |
| RGB | 64 bpp, 16 bpc, kCGImageAlphaNoneSkipLast 没有 Alpha 通道 | Mac OS X |
| RGB | 128 bpp, 32 bpc, kCGImageAlphaNoneSkipLast \| kCGBitmapFloatComponents | Mac OS X |
| RGB | 128 bpp, 32 bpc, kCGImageAlphaPremultipliedLast \| kCGBitmapFloatComponents | Mac OS X |
| CMYK | 32 bpp, 8 bpc, kCGImageAlphaNone 没有 Alpha 通道 | Mac OS X |
| CMYK | 64 bpp, 16 bpc, kCGImageAlphaNone 没有 Alpha 通道 | Mac OS X |
| CMYK | 128 bpp, 32 bpc, kCGImageAlphaNone \| kCGBitmapFloatComponents | Mac OS X |

#### Anti-Aliasing
&emsp;位图图形上下文支持抗锯齿（anti-aliasing），这是一种在绘制文本或形状时人为更正位图图像中有时出现的锯齿（jagged）（或锯齿）边的过程。当位图的分辨率明显低于眼睛的分辨率时，会出现这些锯齿状边缘。为了使对象在位图中看起来平滑，Quartz 对形状轮廓周围的像素使用不同的颜色。通过这种方式混合颜色，形状看起来很平滑。你可以在图 2-4 中看到使用抗锯齿的效果。通过调用函数 `CGContextSetShouldAntialias`，可以关闭特定位图图形上下文的抗锯齿。抗锯齿设置是图形状态（graphics state）的一部分。

&emsp;通过使用函数 `CGContextSetAllowsAntialiasing`，可以控制是否允许对特定图形上下文进行抗锯齿。将 true 传递给此函数以允许抗锯齿，传递 false 则不允许抗锯齿。此设置不是图形状态（graphics state）的一部分。当上下文和图形状态设置置为 true 时，Quartz 执行抗锯齿。

&emsp;Figure 2-4  A comparison of aliased and anti-aliasing drawing（锯齿和抗锯齿绘图的比较）

![antialias](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/c56d0c661402408b84dcda98bf163f95~tplv-k3u1fbpfcp-watermark.image)

### Obtaining a Graphics Context for Printing
&emsp;Mac OS X 中的 Cocoa 应用程序通过自定义 NSView 子类实现打印。通过调用其 `print:` 方法通知视图进行打印。然后，视图创建一个图形上下文，该上下文以打印机为目标，并调用其 `drawRect:` 方法。应用程序使用与绘制到屏幕相同的绘图代码绘制到打印机。它还可以自定义 `drawRect:` 对打印机图像的调用，该图像与发送到屏幕的图像不同。

&emsp;有关在 Cocoa 中进行打印的详细讨论，请参见 Printing Programming Guide for Mac。

## 参考链接
**参考链接:🔗**
+ [Quartz 2D Programming Guide](https://developer.apple.com/library/archive/documentation/GraphicsImaging/Conceptual/drawingwithquartz2d/Introduction/Introduction.html#//apple_ref/doc/uid/TP30001066)
+ [Core Graphics Framework Reference](https://developer.apple.com/documentation/coregraphics)
