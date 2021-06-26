# iOS《Core Animation Programming Guide》官方文档

## About Core Animation
&emsp;Core Animation 是可在 iOS 和 OS X 上使用的图形渲染（graphics rendering）和动画基础结构（animation infrastructure），可用于为应用程序的 view 和其他视觉元素制作动画。使用 Core Animation，绘制动画的每一帧所需的大部分工作都已为你完成。你所要做的就是配置一些动画参数（例如起点和终点），并告诉 Core Animation 开始。其余部分由 Core Animation 完成，将大部分实际绘图工作（actual drawing work）交给板载图形硬件（GPU）以加快渲染速度。这种自动图形加速功能可实现高帧率和流畅的动画效果，而不会给 CPU 造成负担并降低应用程序的运行速度。

&emsp;如果你在编写 iOS 应用程序，不管你是否知道，你都在使用 Core Animation。如果你正在编写 OS X 应用程序，则可以毫不费力地利用 Core Animation 的优势。Core Animation 位于 AppKit 和 UIKit 之下，并与 Cocoa 和 Cocoa Touch 的 view 工作流紧密集成。当然，Core Animation 还具有扩展应用程序 views 所公开功能的接口，并让你对应用程序动画进行更细粒度的控制。

![ca_architecture](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/c564c9a1e6f146018f27b3e546b6abf8~tplv-k3u1fbpfcp-watermark.image)

### At a Glance
&emsp;你可能永远不需要直接使用 Core Animation，但当你这样做时，你应该了解 Core Animation 作为应用程序基础结构的一部分所扮演的角色。
#### Core Animation Manages Your App’s Content（Core Animation 管理你应用的内容）
&emsp;Core Animation 本身不是一个绘图系统。它是在硬件中合成和操作应用程序内容的基础结构。此基础结构的核心是 layer 对象，你可以使用它来管理和操作内容。layer 将你的内容捕获到位图（bitmap）中，该位图可由图形硬件轻松操作。在大多数应用程序中，layer 被用作管理 view 内容的一种方式，但你也可以根据需要创建独立的 layer。

&emsp;相关章节：Core Animation Basics、Setting Up Layer Objects。
#### Layer Modifications Trigger Animations（Layer 修改触发动画）
&emsp;使用 Core Animation 创建的大多数动画都涉及对 layer 属性的修改。与 view 一样，layer 对象有一个 bounds 矩形、屏幕上的位置、不透明度、变换和许多其他可以修改的面向视觉的属性。对于大多数这些属性，更改属性的值会导致创建隐式动画，从而使 layer 从旧值动画到新值动画。在需要对生成的动画行为进行更多控制的情况下，也可以显式设置这些属性的动画。

&emsp;相关章节：Animating Layer Content、Advanced Animation Tricks、Layer Style Property Animations、Animatable Properties。
#### Layers Can Be Organized into Hierarchies（可以将 layers 组织到层次结构中）
&emsp;可以按层次排列 layers 以创建 parent-child 关系。layers 的排列以类似于 view 的方式影响它们管理的视觉内容。附加到 view 的一组 layers 的层次结构反映了相应的 view 层次结构。你还可以将独立 layers 添加到 layer 层次结构中，以将应用程序的视觉内容扩展到 views 之外。

&emsp;相关章节：Building a Layer Hierarchy。
#### Actions Let You Change a Layer’s Default Behavior（Actions 可让你更改 layer 的默认行为）
&emsp;隐式 layer 动画是使用 action 对象实现的，这些对象是实现预定义接口的通用对象。Core Animation 使用 action 对象来实现通常与 layers 关联的默认动画集。你可以创建自己的 action 对象来实现自定义动画，也可以使用它们来实现其他类型的行为。然后，将 action 对象分配给 layers 的属性之一。当该属性更改时，Core Animation 会检索你的 action 对象，并告诉它执行其 action。

&emsp;相关章节：Changing a Layer’s Default Behavior。
### How to Use This Document
&emsp;本文档适用于需要对应用程序动画进行更多控制或希望使用 layers 来提高应用程序绘图性能的开发人员。本文档还提供了有关 iOS 和 OS X 的 layers 和 views 之间 integration 的信息。iOS 和 OS X 的 layers 和 views 之间的 integration 是不同的，了解这些差异对于创建高效的动画非常重要。

### Prerequisites
&emsp;你应该已经了解目标平台的 view 架构，并且熟悉如何创建基于 view 的动画。如果没有，你应该阅读以下文档之一：
+ 对于iOS应用，你应该了解 [View Programming Guide for iOS](https://developer.apple.com/library/archive/documentation/WindowsViews/Conceptual/ViewPG_iPhoneOS/Introduction/Introduction.html#//apple_ref/doc/uid/TP40009503)  中描述的视图架构。
+ 对于OS X应用程序，你应该了解 [View Programming Guide](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/CocoaViewsGuide/Introduction/Introduction.html#//apple_ref/doc/uid/TP40002978) 中描述的视图架构。
### See Also
&emsp;有关如何使用 Core Animation 实现特定类型的动画的示例，请参见 [Core Animation Cookbook](https://developer.apple.com/library/archive/documentation/GraphicsImaging/Conceptual/CoreAnimation_Cookbook/Introduction/Introduction.html#//apple_ref/doc/uid/TP40005406)。
## Core Animation Basics（Core Animation 基础）
&emsp;Core Animation 提供了一个通用系统，用于为应用程序的 views 和其他视觉元素设置动画。Core Animation 不能代替应用程序的 views。相反，它是一种与 views integrates 的技术，可以为动画内容提供更好的性能和支持。它通过将 views 的内容缓存到位图（bitmaps）中来实现这种行为，位图可以由图形硬件（GPU）直接操作。在某些情况下，这种缓存行为可能要求你重新考虑如何呈现和管理应用程序的内容，但大多数情况下，你使用 Core Animation 时根本不知道它在那里。除了缓存 views 内容之外，Core Animation 还定义了一种方法来指定任意视觉内容，将该内容与 views integrates，并将其与其他内容一起设置动画。

&emsp;你可以使用 Core Animation 为应用程序 views 和视觉对象的更改设置动画。大多数更改与修改视觉对象的属性有关。例如，可以使用 Core Animation 来设置对 views position、size 或 opacity 的更改的动画。进行此类更改时，Core Animation 将在属性的当前值和指定的新值之间设置动画。通常不会使用 Core Animation 每秒替换 views 内容 60 次，例如在卡通动画（cartoon）中。相反，你可以使用 Core Animation 在屏幕上移动 views 的内容、淡入淡出该内容、对 views 应用任意图形变换（graphics transformations）或更改 view 的其他视觉属性。
### Layers Provide the Basis for Drawing and Animations（layers 提供绘图和动画的基础）
&emsp;layer 对象是在 3D 空间中组织的 2D 面，并且是你使用 Core Animation 所做的一切的核心。与 view 一样，layers 管理有关其 2D 面的几何图形、内容和视觉属性的信息。与 view 不同，layers 不定义自己的外观。layer 仅管理位图周围的状态信息。位图本身可以是 view 绘图本身的结果，也可以是指定的固定图像。因此，应用程序中使用的主要 layers 被认为是模型对象，因为它们主要管理数据。记住这个概念很重要，因为它会影响动画的行为。
#### The Layer-Based Drawing Model（基于 layer 的绘图模型）
&emsp;大多数 layers 不会在你的应用程序中进行任何实际绘制。相反，layer 捕获应用程序提供的内容并将其缓存在位图中，有时称为备份存储（backing store）。随后更改 layer 的属性时，所做的只是更改与 layer 对象关联的状态信息。当更改触发动画时，Core Animation 将 layer 的位图和状态信息传递给图形硬件，图形硬件使用新信息渲染位图，如图 1-1 所示。在硬件中操作位图会产生比在软件中更快的动画。

&emsp;Figure 1-1  How Core Animation draws content（Core Animation 如何绘制内容）

![basics_layer_rendering](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/7c216ac63a2a433bb0a40be831633476~tplv-k3u1fbpfcp-watermark.image)

&emsp;基于 layer 的绘图由于它操纵静态位图，因此与传统的基于 view 的绘图技术大不相同。使用基于 view 的绘图时，对 view 本身的更改通常会导致调用该 view 的 `drawRect:` 方法，以使用新参数重新绘制内容。但是以这种方式绘图成本很高，因为它是使用主线程上的 CPU 完成的。Core Animation 尽可能通过在硬件中操纵缓存的位图来实现相同或相似的效果，从而避免了这种开销。

&emsp;尽管 Core Animation 尽可能多地使用缓存的内容，但你的应用仍必须提供初始内容并不时更新。你的应用可以通过多种方式为 layer 对象提供内容，有关详细信息，请参见 Providing a Layer’s Contents。
#### Layer-Based Animations（基于 layer 的动画）
&emsp;layer 对象的数据和状态信息与该 layer 内容在屏幕上的视觉呈现是分离的。这种解耦为 Core Animation 提供了一种方法，使其自身进行插入并设置从旧状态值到新状态值的变化的动画。例如，更改 layer 的 position 属性会使 Core Animation 将 layer 从其当前位置移动到新指定的位置。对其他属性的类似更改会导致适当的动画。图 1-2 说明了可以在 layer 上执行的几种动画类型。有关触发动画的 layer 属性的列表，请参见 Animatable Properties。

&emsp;Figure 1-2  Examples of animations you can perform on layers（可以在 layer 上执行的动画示例）

![basics_animation_types](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/529172157fb94024beb0d6da0dfb33a2~tplv-k3u1fbpfcp-watermark.image)

&emsp;在动画过程中，Core Animation 用硬件为你完成所有的逐帧绘制。你所要做的就是指定动画的起点和终点，然后让 Core Animation 完成其余的工作。你还可以根据需要指定自定义时间信息和动画参数。但是，如果没有，Core Animation 将提供合适的默认值。

&emsp;有关如何启动动画和配置动画参数的更多信息，请参见 Animating Layer Content。
### Layer Objects Define Their Own Geometry（layer 对象定义自己的几何）
&emsp;layer 的一项工作是管理其内容的视觉几何（visual geometry）。视觉几何包含有关该内容的 bounds，其在屏幕上的 position 以及该 layer 是否已以任何方式 rotated、scaled 或 transformed 的信息。就像 view 一样，layer 具有 frame 和 bounds 矩形，你可以使用它们来放置 layer 及其内容。layer 还具有 view 不具备的其他属性，例如：锚点（anchor point），它定义了在其周围进行操作的点。指定 layer geometry 某些方面的方式也不同于为 view 指定该信息的方式。
#### Layers Use Two Types of Coordinate Systems（layers 使用两种类型的坐标系）
&emsp;layer 同时使用基于点（point-based）的坐标系和单位坐标系来指定内容的放置。使用哪种坐标系取决于所传递信息的类型。指定直接映射到屏幕坐标的值或必须相对于另一个 layer 指定的值（例如图层的 position 属性）时，将使用基于点的坐标。当值不应绑定到屏幕坐标时使用单位坐标，因为它是相对于其他值的。例如，layer 的 anchorPoint 属性指定相对于 layer 本身 bounds 的点，该点可以更改。

&emsp;基于点的坐标最常见的用途是指定 layer 的 size 和 position，你可以使用 layer 的 bounds 和 position 属性来进行指定。bounds 定义了 layer 本身的坐标系，并包含了屏幕上 layer 的大小。 position 属性定义了 layer 相对于其父级坐标系的位置。尽管 layer 具有 frame 属性，但该属性实际上是从 bounds 和 position 属性中的值派生的，因此使用频率较低。

&emsp;layer bounds 和 frame 矩形的方向始终与基础平台的默认方向匹配。图1-3 显示了 iOS 和 OS X 上 bounds 矩形的默认方向。在 iOS 中，bounds 矩形的原点默认位于 layer 的左上角，而在 OS X 中则位于 底部-左边（bottom-left）角。如果你在 iOS 和 OS X 版本的应用程序之间共享 Core Animation 代码，则必须考虑这些差异。

&emsp;Figure 1-3  The default layer geometries for iOS and OS X（iOS 和 OS X 的默认 layer geometries）

![layer_coords_bounds](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/ad2d402d312d45e8a8ff6f41f6707fad~tplv-k3u1fbpfcp-watermark.image)

&emsp;图1-3 中要注意的一件事是 position 属性位于 layer 的中间。该属性是其定义根据 layer 的 anchorPoint 属性中的值而更改的几个属性之一。锚点表示某些坐标的起源点，并且在  Anchor Points Affect Geometric Manipulations 中有更详细的描述。

&emsp;锚点是你使用单位坐标系指定的多个属性之一。 Core Animation 使用单位坐标来表示属性，这些属性的值在 layer 大小更改时可能会更改。你可以将单位坐标视为指定总可能值的百分比。单位坐标空间中的每个坐标的范围为 0.0 到 1.0。例如，沿 x 轴，左边缘位于坐标 0.0，右边缘位于坐标 1.0。沿 y 轴，单位坐标值的方向根据平台而变化，如图 1-4 所示。

&emsp;Figure 1-4  The default unit coordinate systems for iOS and OS X（iOS 和 OS X 的默认单位坐标系）

![layer_coords_unit](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/cd09ff0fa3f247c9b104bbb85b4ffdf7~tplv-k3u1fbpfcp-watermark.image)

> &emsp;Note: 在 OS X 10.8 之前，geometryFlipped 属性是在需要时更改 layer y 轴默认方向的一种方法。涉及翻转变换（flip transforms）时，有时必须使用此属性来校正 layer 的方向。例如，如果父 view 使用了翻转变换，则其子 view（及其对应 layer）的内容通常会被反转。在这种情况下，将子 layer 的 geometryFlipped 属性设置为 YES 是纠正问题的简便方法。在 OS X 10.8 及更高版本中，AppKit 会为你管理此属性，并且你不应对其进行修改。对于 iOS 应用程序，建议你完全不要使用 geometryFlipped 属性。

&emsp;所有坐标值（无论是点坐标还是单位坐标）都指定为浮点数。使用浮点数可让你指定可能位于法线坐标值之间的精确位置。使用浮点值很方便，尤其是在打印过程中或绘制到可能由多个像素表示一个点的 Retina 显示器时。浮点值使你可以忽略基础设备分辨率，而仅以所需的精度指定值。
#### Anchor Points Affect Geometric Manipulations（锚点影响几何操纵）
&emsp;layer 的几何相关操作是相对于该 layer 的锚点进行的，你可以使用该 layer 的 anchorPoint 属性进行访问。在操纵 layer 的 position 或 transform 属性时，锚点的影响最为明显。始终相对于 layer 的锚点指定 position 属性，并且你应用于 layer 的任何变换也都相对于锚点进行。

&emsp;图1-5 演示了如何将锚点从其默认值更改为其他值如何影响 layer 的 position 属性。即使 layer 未在其父级 bounds 内移动，将锚点从 layer 的中心移动到 layer 的 bounds 原点也会更改 position 属性中的值。

&emsp;Figure 1-5  How the anchor point affects the layer’s position property（锚点如何影响 layer 的 position 属性）

![layer_coords_anchorpoint_position](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/b613b3a352c546d29bba35ddf9e7b64e~tplv-k3u1fbpfcp-watermark.image)

&emsp;图1-6 显示了更改锚点如何影响应用于 layer 的变换。将旋转变换应用于 layer 时，旋转会围绕锚点发生。由于锚点默认情况下设置为图层的中间位置，因此通常会创建你期望的旋转行为。但是，如果更改锚点，则旋转的结果将不同。

&emsp;Figure 1-6  How the anchor point affects layer transformations（锚点如何影响 layer 转换（transformations））

![layer_coords_anchorpoint_transform](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/8668f933f653469ca4ca545b85b4af96~tplv-k3u1fbpfcp-watermark.image)

#### Layers Can Be Manipulated in Three Dimensions（可以在三个维度上操纵 layers）
&emsp;每个 layer 都有两个变换矩阵（transform matrices），可用于操纵该 layer 及其内容。 CALayer 的 transform 属性指定要同时应用于该 layer 及其嵌入式子 layer 的转换。通常，当你要修改 layer 本身时，可以使用此属性。例如，你可以使用该属性缩放或旋转 layer 或临时更改其位置。 sublayerTransform 属性定义仅适用于子 layer 的其他转换，并且最常用于向场景内容添加透视视觉效果。

&emsp;transforms 的工作方式是将坐标值乘以一个数字矩阵，以获得表示原始点的变换版本的新坐标。因为可以在三个维度上指定 Core Animation 值，所以每个坐标点都有四个值，必须将这些值乘以四乘四矩阵，如图 1-7 所示。在 Core Animation 中，图中的变换由 CATransform3D 类型表示。幸运的是，你不必直接修改此结构的字段即可执行标准转换。 Core Animation 提供了一组全面的功能，用于创建比例尺，平移和旋转矩阵以及进行矩阵比较。除了使用函数来操纵变换之外，Core Animation 还扩展了键值编码支持，使你可以使用 key path 修改变换。有关可以修改的 key path 的列表，请参见 CATransform3D Key Paths。

&emsp;Figure 1-7  Converting a coordinate using matrix math（使用矩阵数学转换坐标）

![transform_basic_math](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/f57daa03742649c8a161e8bd6fd6a971~tplv-k3u1fbpfcp-watermark.image)

+ x' = x * m11 + y * m12 + z * m13 + 1 * m14
+ y' = x * m21 + y * m22 + z * m23 + 1 * m24
+ z' = x * m31 + y * m32 + z * m33 + 1 * m34

&emsp;图1-8 显示了你可以进行的一些更常见转换的矩阵配置。将任何坐标乘以恒等（identity 矩阵）变换将返回完全相同的坐标。对于其他转换，如何修改坐标完全取决于你更改的矩阵成分。例如，仅沿 x 轴平移，你将为平移矩阵的 tx 分量提供一个非零值，并将 ty 和 tz 值保留为 0。对于旋转，你将提供适当的正弦和余弦值目标旋转角度。

&emsp;Figure 1-8  Matrix configurations for common transformations（常见转换（transformations）的矩阵配置）
![transform_manipulations](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/6ea3e5fd03fe436e85b606d08f34439d~tplv-k3u1fbpfcp-watermark.image)

&emsp;有关用于创建和操纵变换（transforms）的函数的信息，请参见 Core Animation Function Reference。
### Layer Trees Reflect Different Aspects of the Animation State（layer 树反映了动画状态的不同方面）
&emsp;使用 Core Animation 的应用程序具有三组 layer 对象。在使你的应用程序内容显示在屏幕上时，每组 layer 对象都有不同的作用：

+ 模型层树（model layer tree）（或简称 "layer tree"）中的对象是应用程序与之交互最多的对象。此树中的对象是存储任何动画的 target 值的模型对象。无论何时更改 layer 的属性，都会使用其中一个对象。
+ 表示树（presentation tree）中的对象包含任何正在运行的动画的动态值。尽管层树对象包含动画的目标值，但表示树中的对象在显示在屏幕上时会反映当前值。永远不要修改此树中的对象。相反，你可以使用这些对象读取当前动画值，或者从这些值开始创建新动画。
+ 渲染树（render tree）中的对象执行实际动画，并且是 Core Animation 专有。

&emsp;每一组 layer 对象都组织成一个层次结构，例如你的应用程序中的 views。实际上，对于为所有 view 启用 layer 的应用程序，每棵树的初始结构都与 view 层次结构完全匹配。但是，应用程序可以根据需要将其他 layer 对象（即，与 view 无关的 layer）添加到 layer 层次结构中。你可以在某些情况下执行此操作，以优化不需要 view 所有开销的内容的应用性能。图 1-9 显示了在简单的 iOS 应用中发现的 layer 的分解。示例中的 window 包含一个内容 view，该 view 本身包含一个按钮 view 和两个独立的 layer 对象。每个 view 都有一个对应的 layer 对象，该对象形成 layer 层次结构的一部分。

&emsp;Figure 1-9  Layers associated with a window（与窗口关联的 layer）

![sublayer_hierarchy](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/4f45ae85c10a4b4a813e76cafd2d6c74~tplv-k3u1fbpfcp-watermark.image)

&emsp;对于 layer 树中的每个对象，表示和渲染树（presentation and render trees）中都有一个匹配的对象，如图 1-10 所示。如前所述，应用程序主要与图层树（layer tree）中的对象一起使用，但有时可能会访问表示树（presentation tree）中的对象。具体来说，访问层树（layer tree）中对象的 presentationLayer 属性将返回表示树（presentation tree）中的相应对象。你可能要访问该对象以读取动画中间属性的当前值。

&emsp;Figure 1-10  The layer trees for a window（窗口的层树）

![sublayer_hierarchies](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/866c033240e447d89966ca1ba5a7c3f7~tplv-k3u1fbpfcp-watermark.image)

> &emsp;Important: 仅当动画正在运行时，才应访问表示树（presentation tree）中的对象。当动画正在进行时，表示树（presentation tree）包含在屏幕上显示的层值。此行为与层树（layer tree）不同，层树（layer tree）始终反映代码设置的最后一个值，并等效于动画的最终状态。
### The Relationship Between Layers and Views（图层与视图之间的关系）
&emsp;layer 不能替代应用程序的 view，也就是说，你不能仅基于 layer 对象创建可视界面。layer 为你的 view 提供基础结构。具体而言，layer 使绘制和动画化 view 内容并使其保持动画状态并保持较高的帧速率更加轻松有效。但是，很多事情是 layer 无法做到的。layer 不处理事件、绘制内容、参与响应者链或执行许多其他操作。因此，每个应用程序仍必须具有一个或多个 view 来处理这些类型的交互。

&emsp;在 iOS 中，每个 view 都有相应的 layer 对象支持，但在 OS X 中，你必须确定哪些 view 应具有 layer。在 OS X v10.8 及更高版本中，将 layer 添加到所有 view 可能是有意义的。但是，你不需要这样做，并且在不需要和不需要开销的情况下仍可以禁用 layer。分层（layers do increase）确实会增加你的应用程序的内存开销，但是它们的好处通常大于缺点，因此，最好在禁用 layer 支持之前测试应用程序的性能。

&emsp;为 view 启用 layer 支持（layer support）时，将创建称为 layer 支持（layer-backed ）的 view。在层支持的 view 中，系统负责创建基础 layer 对象，并使该 layer 与 view 保持同步。所有 iOS 视图都是基于 layer 的，并且 OS X 中的大多数 view 也是。但是，在 OS X 中，你还可以创建一个图层托管视图（layer-hosting view），该 view 是你自己提供 layer 对象的 view。对于图层托管视图（layer-hosting view），AppKit 在管理 layer 时采取了一种不干涉的方法，并且不会根据 view 的更改对其进行修改。

> &emsp;Note: 对于支持 layer 的 view，建议你尽可能操作 view 而不是其 layer。在 iOS 中，view 只是 layer 对象的薄薄包装，因此你对 layer 进行的任何操作通常都可以正常工作。但是在 iOS 和 OS X 中都存在这样的情况，即操纵 layer 而不是  view 可能无法产生所需的结果。本文档将尽可能指出这些陷阱并尝试提供方法来帮助你解决它们。

&emsp;除了与 view 关联的 layer 之外，你还可以创建没有对应 view 的 layer 对象。你可以将这些独立的 layer 对象嵌入到应用程序中的任何其他 layer 对象中，包括与 view 关联的对象。通常，你将独立的 layer 对象用作特定优化路径的一部分。例如，如果要在多个位置使用同一图像，则可以加载一次图像并将其与多个独立的 layer 对象关联，然后将这些对象添加到 layer tree 中。然后，每个 layer 都引用源映像，而不是尝试在内存中创建该映像的自己的副本。

&emsp;有关如何为应用程序 view 启用 layer 支持的信息，请参见 Enabling Core Animation Support in Your App。有关如何创建 layer 对象层次结构的信息，以及有关何时进行创建的提示，请参见 Building a Layer Hierarchy。

## Setting Up Layer Objects（设置图层对象）
&emsp;layer 对象是你使用 Core Animation 所做的一切的核心。layer 管理你应用的视觉内容，并提供用于修改该内容的样式和视觉外观的选项。尽管 iOS 应用已自动启用了 layer 支持，但 OS X 应用的开发人员必须先显式启用它，然后才能利用性能优势。启用后，你需要了解如何配置和操作应用程序的 layer 以获得所需的效果。
### Enabling Core Animation Support in Your App（在你的应用中启用 Core Animation 支持）
&emsp;在 iOS 应用中，始终启用 Core Animation，并且每个 view 都有一个 layer 支持。在 OS X 中，应用程序必须通过执行以下操作显式启用 Core Animation 支持：
+ Link against the QuartzCore framework。（iOS 应用程序必须在明确使用 Core Animation 接口的情况下，才引用此框架。）
+ 通过执行以下操作之一，为一个或多个 NSView 对象启用 layer 支持：
  + 在你的 nib 文件中，使用 View Effects inspector 为 view 启用 layer 支持。检查器显示所选 view 及其子  view 的复选框。建议你尽可能在 window 的内容 view 中启用 layer 支持。
  + 对于你以代码的方式创建的 view，请调用 view 的 `setWantsLayer:` 方法并传递 YES 值，以指示该 view 应使用 layer。

&emsp;以前述方式之一启用 layer 支持会创建一个 layer 支持的 view。使用 layer 支持的 view，系统负责创建基础 layer 对象并保持该 layer 的更新。在 OS X 中，还可以创建一个图层托管视图（layer-hosting view），从而使你的应用程序实际创建和管理底层 layer 对象。 （你不能在 iOS 中创建图层托管视图。）有关如何创建图层托管视图的更多信息，请参见 Layer Hosting Lets You Change the Layer Object in OS X。
### Changing the Layer Object Associated with a View（更改与视图关联的图层对象）
&emsp;默认情况下，layer 支持 view 创建 CALayer 类的实例，在大多数情况下，你可能不需要不同类型的 layer 对象。但是，Core Animation 提供了不同的 layer 类，每个 layer 类都提供了你可能会发现有用的专门功能。选择不同的 layer 类可以使你以简单的方式提高性能或支持特定类型的内容。例如，CATiledLayer 类经过优化，可以高效地显示大型图像。
#### Changing the Layer Class Used by UIView（更改 UIView 使用的图层类）
&emsp;你可以通过重写 iOS 的 `layerClass` 方法并返回其他类对象来更改 iOS view 使用的 layer 类型。大多数 iOS view 都会创建 CALayer 对象，并将该 layer 用作其内容的备份存储。对于你自己的大多数 view，此默认选择是一个不错的选择，你无需更改它。但是你可能会发现在某些情况下使用不同的 layer 类更为合适。例如，在以下情况下，你可能想要更改 layer 类：

+ 你的 view 使用 Metal 或 OpenGL ES 绘制内容，在这种情况下，你将使用 CAMetalLayer 或 CAEAGLLayer 对象。
+ 有一个专门的 layer 类，可以提供更好的性能。
+ 你想利用某些特殊的 Core Animation layer 类，例如粒子发射器（particle emitters）或复制器。

&emsp;更改 view 的 layer 类非常简单；清单2-1 中显示了一个示例。你所要做的就是重写 `layerClass` 方法，并返回要使用的类对象。在显示之前，view 将调用 `layerClass` 方法并使用返回的类为其自身创建一个新的 layer 对象。创建后，view 的 layer 对象无法更改。

&emsp;Listing 2-1  Specifying the layer class of an iOS view（指定 iOS 视图的图层类）
```c++
+ (Class) layerClass {
   return [CAMetalLayer class];
}
```
&emsp;有关 layer 类及其使用方式的列表，请参见 Different Layer Classes Provide Specialized Behaviors。
#### Layer Hosting Lets You Change the Layer Object in OS X（图层托管使你可以在 OS X 中更改图层对象）
&emsp;图层托管视图（layer-hosting view）是你自己创建和管理基础 layer 对象的 NSView 对象。在想要控制与 view 关联的 layer 对象的类型的情况下，可以使用图层托管。例如，你可以创建一个图层托管视图，以便可以分配默认 CALayer 类以外的其他 layer 类。如果要使用单个 view 来管理独立 layer 的层次结构，也可以使用它。

&emsp;当你调用 view 的 `setLayer:` 方法并提供一个 layer 对象时，AppKit 会采用一种不使用该 layer 的方法。通常，AppKit 会更新 view 的 layer 对象，但是在图层托管情况下，它并不能用于大多数属性。

&emsp;要创建图层托管视图，请创建 layer 对象并将其与 view 关联，然后在屏幕上显示该 view，如清单 2-2 所示。除了设置 layer 对象之外，你还必须调用 `setWantsLayer:` 方法以使 view 知道它应该使用 layer。

&emsp;Listing 2-2  Creating a layer-hosting view（创建图层托管视图）
```c++
// Create myView...
 
[myView setWantsLayer:YES];
CATiledLayer* hostedLayer = [CATiledLayer layer];
[myView setLayer:hostedLayer];
 
// Add myView to a view hierarchy.
```
&emsp;如果选择自己托管图层，则必须自行设置 contentsScale 属性，并在适当的时候 provide high-resolution content。有关 high-resolution content 和比例因子（scale factors）的更多信息，请参见 Working with High-Resolution Images。
#### Different Layer Classes Provide Specialized Behaviors（不同的层类别提供特殊的行为）
&emsp;Core Animation 定义了许多标准 layer 类，每种标准 layer 类都是针对特定用例设计的。 CALayer 类是所有 layer 对象的根类。它定义了所有 layer 对象必须支持的行为，并且是 layer 支持的 view 使用的默认类型。但是，你也可以在表 2-1 中指定一个 layer 类。

&emsp;Table 2-1  CALayer subclasses and their uses（CALayer 子类及其用途）

| Class | Usage |
| --- | --- |
| CAEmitterLayer | 用于实现基于 Core Animation 的粒子发射器系统。发射器层对象控制粒子的生成及其来源。 |
| CAGradientLayer | 用于绘制填充图层形状的颜色渐变（在任何圆角的范围内）。 |
| CAMetalLayer | 用于设置和 vend 可绘制纹理，以使用 Metal 渲染图层内容。 |
| CAEAGLLayer/CAOpenGLLayer | 用于设置备份存储和上下文，以使用 OpenGL ES（iOS）或 OpenGL（OS X）渲染图层内容。 |
| CAReplicatorLayer | 当你要自动制作一个或多个子层的副本时使用。复制器为你制作副本，并使用你指定的属性来更改副本的 appearance 或 attributes。 |
| CAScrollLayer | 用于管理由多个子层组成的较大的可滚动区域。 |
| CAShapeLayer | 用于绘制三次贝塞尔曲线样条曲线。Shape 图层对于绘制基于路径的形状非常有利，因为它们始终会产生清晰的路径，而与你绘制到图层的备份存储中的路径相反，后者在缩放时看起来并不好。但是，清晰的结果确实涉及在主线程上渲染 Shape 并缓存结果。 |
| CATextLayer | 用于呈现纯文本字符串或属性字符串。 |
| CATiledLayer | 用于管理可分为较小图块的大图像，并支持放大和缩小内容，分别渲染。 |
| CATransformLayer | 用于渲染真实的 3D 图层层次结构，而不是由其他图层类实现的平坦的图层层次结构。 |
| QCCompositionLayer | 用于渲染 Quartz Composer 合成。（仅支持 OS X） |

### Providing a Layer’s Contents（提供图层的内容）
&emsp;layer 是管理你应用程序提供的内容的数据对象。layer 的内容由包含要显示的视觉数据的位图组成。你可以通过以下三种方式之一提供该位图的内容：

+ 将图像对象直接指定给 layer 对象的 contents 属性。（此技术最适用于从不更改或很少更改的 layer 内容。）
+ 将 delegate 对象指定给 layer，并让 delegate 绘制 layer 的内容。（此技术最适合于可能定期更改并且可以由外部对象（如 view）提供的 layer 内容。）
+ 定义一个 layer 子类并重写它的一个绘图方法来自己提供 layer 内容。（如果必须创建自定义 layer 子类，或者要更改 layer 的基本绘图行为，则此技术是合适的。）

&emsp;唯一需要担心为 layer 提供内容的时间是你自己创建 layer 对象时。如果你的应用仅包含支持 layer 的 view，则无需担心使用任何前述技术来提供 layer 内容。支持 layer 的 view 会自动以最有效的方式为其关联 layer 提供内容。
#### Using an Image for the Layer’s Content（使用图像作为图层的内容）
&emsp;由于 layer 只是用于管理位图图像的容器，因此你可以将图像直接分配给 layer 的 contents 属性。将图像分配给 layer 很容易，并且可以指定要在屏幕上显示的确切图像。layer 使用你直接提供的图像对象，并且不尝试创建该图像的自己的副本。如果你的应用在多个地方使用相同的图像，此行为可以节省内存。

&emsp;你分配给 layer 的图像必须是 CGImageRef 类型。 （在 OS X v10.6 及更高版本中，你还可以分配一个 NSImage 对象。）分配图像时，请记住提供一个分辨率与本机设备的分辨率匹配的图像。对于具有 Retina 显示屏的设备，这可能还需要你调整图像的 contentsScale 属性。有关在图层上使用高分辨率内容的信息，请参见 Working with High-Resolution Images。
#### Using a Delegate to Provide the Layer’s Content（使用委托提供图层内容）
&emsp;如果 layer 的内容动态变化，则可以使用 delegate 对象在需要时提供和更新该内容。在显示时，该 layer 调用 delegate 的方法以提供所需的内容：
+ 如果 delegate 实现 `displayLayer:` 方法，则该实现负责创建位图并将其分配给 layer 的 contents 属性。
+ 如果你的 delegate 执行 `drawLayer:inContext:` 方法，Core Animation 创建位图，创建要绘制到该位图中的图形上下文，然后调用 delegate 方法来填充位图。你的 delegate 方法所要做的就是绘制到提供的图形上下文中。

&emsp;delegate 对象必须实现 `displayLayer:` 或 `drawLayer:inContext:` 方法。如果 delegate 同时实现 `displayLayer:` 和 `drawLayer:inContext:` 方法，则 layer 仅调用 `displayLayer:` 方法。

&emsp;重写 `displayLayer:` 方法最适合你的应用喜欢加载或创建要显示的位图的情况。清单2-3 显示了 `displayLayer:` 委托方法的示例实现。在此示例中，委托使用帮助器对象加载并显示所需的图像。委托方法根据其自身的内部状态选择要显示的图像，在示例中，该状态是名为 `displayYesImage` 的自定义属性。

&emsp;Listing 2-3  Setting the layer contents directly（直接设置图层内容）
```c++
- (void)displayLayer:(CALayer *)theLayer {
    // Check the value of some state property
    if (self.displayYesImage) {
        // Display the Yes image
        theLayer.contents = [someHelperObject loadStateYesImage];
    } else {
        // Display the No image
        theLayer.contents = [someHelperObject loadStateNoImage];
    }
}
```
&emsp;如果你没有预渲染的图像或帮助对象来为你创建位图，则 delegate 可以使用 `drawLayer:inContext:` 方法动态绘制内容。清单2-4 显示了 `drawLayer:inContext:` 方法的示例实现。在此示例中，delegate 使用固定宽度和当前渲染颜色绘制一条简单的弯曲路径。

&emsp;Listing 2-4  Drawing the contents of a layer（绘制图层内容）
```c++
- (void)drawLayer:(CALayer *)theLayer inContext:(CGContextRef)theContext {
    // Create path
    CGMutablePathRef thePath = CGPathCreateMutable();
 
    CGPathMoveToPoint(thePath,NULL,15.0f,15.f);
    CGPathAddCurveToPoint(thePath,
                          NULL,
                          15.f,250.0f,
                          295.0f,250.0f,
                          295.0f,15.0f);
 
    CGContextBeginPath(theContext);
    CGContextAddPath(theContext, thePath);
 
    CGContextSetLineWidth(theContext, 5);
    CGContextStrokePath(theContext);
 
    // Release the path
    CFRelease(thePath);
}
```
&emsp;对于具有自定义内容的支持 layer 的 view，你应该继续重写该 view 的方法来进行绘制。layer 支持的 view 自动使自己成为其 layer 的 delegate 并实现所需的委托方法，并且你不应更改该配置。相反，你应该实现 view 的 `drawRect:` 方法来绘制内容。

&emsp;在 OS X v10.8 及更高版本中，绘图的另一种方法是通过重写 view 的 `wantUpdateLayer` 和 `updateLayer` 方法来提供位图。重写 `wantUpdateLayer` 并返回 YES 会使 NSView 类遵循替代的呈现路径。view 不调用 `drawRect:`，而是调用你的 `updateLayer` 方法，该方法的实现必须直接将位图分配给 layer 的 `content` 属性。这是 AppKit 希望你直接设置 view layer 对象的内容的一种情况。
#### Providing Layer Content Through Subclassing（通过子类提供层内容）
&emsp;如果仍然要实现自定义 layer 类，则可以重写 layer 类的绘制方法以进行任何绘制。layer 对象本身生成自定义内容并不常见，但是 layer 当然可以管理内容的显示。例如，CATiledLayer 类通过将大图像分成较小的图块来管理大图像，这些小图块可以单独管理和呈现。因为只有 layer 具有在任何给定时间需要渲染哪些图块的信息，所以它直接管理绘图行为。

&emsp;子类别化时，可以使用以下两种技术之一来绘制 layer 的内容：

+ 重写 layer 的 `display` 方法，并使用它直接设置 layer 的 contents 属性。
+ 重写 layer 的 `drawInContext:` 方法，并使用它绘制到提供的图形上下文中。

&emsp;重写哪种方法取决于对绘图过程所需的控制程度。`display` 方法是更新 layer 内容的主要入口点，因此重写该方法将使你完全控制该过程。重写 `display` 方法还意味着你要负责创建要分配给 contents 属性的 CGImageRef。如果你只想绘制内容（或让 layer 管理绘制操作），可以重写 `drawInContext:` 方法，让 layer 为你创建备份存储。
#### Tweaking the Content You Provide（调整你提供的内容）
&emsp;当你将图像分配给 layer 的 content 属性时，该 layer 的 contentsGravity 属性确定如何操作该图像以适合当前 bounds。默认情况下，如果图像大于或小于当前 bounds，则 layer 对象将缩放图像以适合可用空间。如果layer  bounds 的宽高比与图像的高宽比不同，则可能导致图像变形。你可以使用 contentsGravity 属性来确保以最佳方式呈现你的内容。

&emsp;可以分配给 contentsGravity 属性的值分为两类：

+ 基于位置（position-based）的重力常数允许你将图像固定到 layer bounds 矩形的特定边或角，而无需缩放图像。
+ 基于缩放（scaling-based）的重力常数允许你使用几个选项中的一个来拉伸图像，其中一些选项保留纵横比，而另一些则不保留纵横比。

&emsp;图2-1 显示了基于位置的重力设置如何影响图像。除了 kCAGravityCenter 常数外，每个常数都将图像固定在 layer bounds 矩形的特定边缘或角落。 kCAGravityCenter 常数使图像在 layer 中居中。这些常数都不会使图像以任何方式缩放，因此图像始终以其原始大小进行渲染。如果图像大于 layer bounds，则可能会导致图像的一部分被裁剪；如果图像较小，则该 layer 中未被图像覆盖的部分将显示该 layer 的背景色（如果设置）。

&emsp;Figure 2-1  Position-based gravity constants for layers（图层的基于位置的重力常数）

![layer_contentsgravity1](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/2e8413a42dd84fae9c4764a9a84e247c~tplv-k3u1fbpfcp-watermark.image)

&emsp;图2-2 显示了基于缩放的重力常数如何影响图像。如果所有这些常数都不完全适合 layer 的 bounds 矩形，则将缩放图像。两种模式之间的区别在于它们如何处理图像的原始长宽比。有些模式保留它，而另一些则不保留。默认情况下，图层的 contentsGravity 属性设置为 kCAGravityResize 常数，这是唯一不保留图像长宽比的模式。

&emsp;Figure 2-2  Scaling-based gravity constants for layers（基于缩放的图层重力常数）

![positioningmask](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/c91bcb27b3c34f238f6b282e47db89f7~tplv-k3u1fbpfcp-watermark.image)

#### Working with High-Resolution Images（使用高分辨率图像）
&emsp;layer 不具备基础设备屏幕分辨率的任何固有知识。layer 仅存储指向位图的指针，并在给定可用像素的情况下以最佳方式显示它。如果将图像分配给 layer 的 content 属性，则必须通过将 layer 的 contentsScale 属性设置为适当的值来告知 Core Animation 该图像的分辨率。该属性的默认值为 1.0，适用于打算在标准分辨率屏幕上显示的图像。如果你的图像打算用于 Retina 显示屏，请将此属性的值设置为 2.0。

&emsp;仅当你直接向 layer 分配位图时，才需要更改 contentsScale 属性的值。 UIKit 和 AppKit 中基于 layer 的 view 会根据屏幕分辨率和该 view 管理的内容，自动将其 layer 的比例因子设置为适当的值。例如，如果你将 NSImage 对象分配给 OS X 中 layer 的 contents 属性，则 AppKit 会查看该图像是否同时具有标准和高分辨率版本。如果存在，则 AppKit 会为当前分辨率使用正确的变体，并将 contentsScale 属性的值设置为匹配。

&emsp;在 OS X 中，基于位置的重力常数会影响从分配给 layer 的 NSImage 对象中选择图像表示的方式。由于这些常数不会导致图像缩放，因此 Core Animation 依靠 contentsScale 属性来选择具有最合适像素密度的图像表示形式。

&emsp;在 OS X 中，layer 的委托可以实现 `layer:shouldInheritContentsScale:fromWindow` 方法并使用它来响应比例因子的变化。每当给定 window 的分辨率发生变化时，AppKit 都会自动调用该方法，这可能是因为 window 在标准分辨率和高分辨率屏幕之间移动。如果委托支持更改 layer 图像的分辨率，则此方法的实现应返回 YES。然后，该方法应根据需要更新 layer 的内容，以反映新的分辨率。
### Adjusting a Layer’s Visual Style and Appearance（调整图层的视觉样式和外观）
&emsp;layer 对象具有内置的视觉装饰（visual adornments），例如边框（border）和背景色，可以用来补充 layer 的主要内容。因为这些视觉装饰不需要任何渲染，所以在某些情况下可以将 layer 用作独立实体。你所要做的就是在 layer 上设置一个属性，layer 处理必要的绘图，包括任何动画。有关这些视觉装饰如何影响 layer 外观的其他说明，请参见 Layer Style Property Animations。
#### Layers Have Their Own Background and Border（图层具有自己的背景和边界）
&emsp;除基于图像的内容外，layer 还可以显示填充的背景和描边边框。如图 2-3 所示，背景色呈现在 layer 内容图像的后面，边框呈现在该图像的顶部。如果 layer 包含子 layer，它们也将显示在边框下方。因为背景色位于图像的后面，所以该颜色会穿透图像的任何透明部分。

&emsp;Figure 2-3  Adding a border and background to a layer（在图层上添加边框和背景）

![layer_border_background](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/51f9cac58e564c35b2fa4e2590aa93c0~tplv-k3u1fbpfcp-watermark.image)

&emsp;清单2-5 显示了为 layer 设置背景色和边框所需的代码。所有这些属性都可以设置动画。

&emsp;Listing 2-5  Setting the background color and border of a layer（设置图层的背景色和边框）
```c++
myLayer.backgroundColor = [NSColor greenColor].CGColor;
myLayer.borderColor = [NSColor blackColor].CGColor;
myLayer.borderWidth = 3.0;
```
> &emsp;Note: 可以为 layer 的背景使用任何类型的颜色，包括具有透明度或使用图案图像的颜色。不过，在使用模式图像时，请注意 Core Graphics 处理模式图像的渲染，并使用其标准坐标系进行处理，这与 iOS 中的默认坐标系不同。因此，在 iOS 上渲染的图像在默认情况下会显示为上下颠倒，除非你翻转坐标。

&emsp;如果将 layer 的背景色设置为不透明颜色，请考虑将 layer 的 opaque 属性设置为 YES。这样做可以提高在屏幕上合成 layer 时的性能，并且不需要 layer 的备份存储来管理 alpha 通道。但是，如果 layer 的 corner radius 不为零，则不能将其标记为不透明。
#### Layers Support a Corner Radius（图层支持圆角半径）
&emsp;你可以通过为 layer 添加圆角半径来为其创建圆角矩形效果。圆角半径是一种视觉装饰，可遮盖 layer bounds 矩形的部分拐角，以使基础内容得以显示，如图 2-4 所示。由于涉及到应用透明蒙版，因此除非将 masksToBounds 属性设置为 YES，否则圆角半径不会影响图层的 content 属性中的图像。但是，圆角半径始终会影响 layer 背景颜色和边框的绘制方式。

&emsp;Figure 2-4  A corner radius on a layer（图层的圆角半径）

![layer_corner_radius](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/6b56cbf027bb4b2983edc495c3e2d3fe~tplv-k3u1fbpfcp-watermark.image)

&emsp;若要将圆角半径应用于 layer，请为 layer 的 cornerRadius 属性指定一个值。指定的半径值以点为单位测量，并在显示之前应用于 layer 的所有四个角。
#### Layers Support Built-In Shadows（图层支持内置阴影）
&emsp;CALayer 类包含几个用于配置阴影效果的属性。阴影使 layer 看起来像是漂浮在其基础内容之上，从而增加了 layer 的深度。这是另一种视觉装饰，你可能会发现它在特定情况下对你的应用有用。使用 layer，你可以控制阴影的颜色，相对于 layer 内容的位置、不透明度和形状。

&emsp;默认情况下，layer 阴影的不透明度（opacity）值设置为 0，可有效隐藏阴影。将不透明度更改为非零值会导致 Core Animation 绘制阴影。由于阴影默认情况下位于 layer 的正下方，因此你可能还需要更改阴影的偏移量才能看到它。不过请务必记住，为阴影指定的偏移量是使用 layer 的本地坐标系应用的，这在 iOS 和 OS X 上是不同的。图 2-5 显示了一个阴影向下延伸到 layer 的右边。在 iOS 中，这要求为 y 轴指定一个正值，但在 OS X 中，该值必须为负。

&emsp;Figure 2-5  Applying a shadow to a layer（将阴影应用于图层）

![layer_shadows](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/4c8c23288a134dc0905e1228d65195e4~tplv-k3u1fbpfcp-watermark.image)

&emsp;将阴影添加到 layer 时，阴影是 layer 内容的一部分，但实际上延伸到 layer 的 bounds 矩形之外。因此，如果为 layer 启用 masksToBounds 属性，则阴影效果将被修剪到边缘周围。如果你的 layer 包含任何透明内容，这可能会导致奇怪的效果，即阴影的直接在 layer 下面的部分仍然可见，而超出 layer 的部分则不可见。如果要阴影但也要使用 bounds masking，请使用两层而不是一层。将蒙版应用于包含你的内容的 layer，然后将该 layer 嵌入与启用阴影效果的大小完全相同的第二图层中。

&emsp;有关如何将阴影应用于 layer 的示例，请参见 Shadow Properties。
#### Filters Add Visual Effects to OS X Views（过滤器将视觉效果添加到 OS X 视图）
&emsp;在 OS X 应用中，你可以将 Core Image 滤镜直接应用于 layer 的内容。你可以这样做来模糊或锐化 layer 的内容，更改颜色，扭曲内容或执行许多其他类型的操作。例如，图像处理程序可能会使用这些滤镜无损地修改图像，而视频编辑程序可能会使用它们来实现不同类型的视频过渡效果。而且，由于过滤器是通过硬件应用到 layer 内容的，因此渲染既快速又流畅。

> &emsp;Note: 您无法将过滤器添加到 iOS 中的 layer 对象。

&emsp;对于给定的 layer，你可以将过滤器应用于该 layer 的前景和背景内容。前景内容包括 layer 本身包含的所有内容，包括其 contents 属性中的图像，其背景颜色，其边框以及其子 layer 的内容。背景内容是直接在 layer 下面但实际上不是 layer 本身一部分的内容。大多数 layer 的背景内容是其直接 superlayer 的内容，该内容可能被该层完全或部分遮盖。例如，当你希望用户专注于 layer 的前景内容时，可以对背景内容应用模糊滤镜。

&emsp;你可以通过将 CIFilter 对象添加到 layer 的以下属性来指定过滤器：

+ filters 属性包含仅影响 layer 前景内容的过滤器数组。
+ backgroundFilters 属性包含一组过滤器，这些过滤器仅影响 layer 的背景内容。
+ compositingFilter 属性定义 layer 的前景和背景内容如何组合在一起。

&emsp;要将过滤器添加到 layer ，必须先找到并创建 CIFilter 对象，然后对其进行配置，然后再将其添加到 layer。 CIFilter 类包括几个用于查找可用的 Core Image 过滤器的类方法，例如 `filterWithName:` 方法。不过，创建过滤器只是第一步。许多滤镜具有定义滤镜如何修改图像的参数。例如，盒子模糊滤镜的输入半径参数会影响所应用的模糊量。在过滤器配置过程中，应始终为这些参数提供值。但是，不需要指定的一个常见参数就是输入图像，它由 layer 本身提供。

&emsp;将过滤器添加到 layer 时，最好在将过滤器添加到 layer 之前配置过滤器参数。这样做的主要原因是，一旦添加到 layer，就无法修改 CIFilter 对象本身。但是，你可以使用图层的 `setValue:forKeyPath:` 方法在事后更改过滤器值。

&emsp;清单2-6 显示了如何创建捏变形过滤器（滤镜）（a pinch distortion filter）并将其应用于 layer 对象。此滤镜向内捏住图层的源像素，使最接近指定中心点的那些像素失真。请注意，在该示例中，你无需指定滤镜的输入图像，因为该 layer 的图像会自动使用。

&emsp;Listing 2-6  Applying a filter to a layer（将滤镜应用于图层）
```c++
CIFilter* aFilter = [CIFilter filterWithName:@"CIPinchDistortion"];
[aFilter setValue:[NSNumber numberWithFloat:500.0] forKey:@"inputRadius"];
[aFilter setValue:[NSNumber numberWithFloat:1.25] forKey:@"inputScale"];
[aFilter setValue:[CIVector vectorWithX:250.0 Y:150.0] forKey:@"inputCenter"];
 
myLayer.filters = [NSArray arrayWithObject:aFilter];
```
&emsp;有关可用 Core Image 过滤器的信息，请参见 [Core Image Filter Reference](https://developer.apple.com/library/archive/documentation/GraphicsImaging/Reference/CoreImageFilterReference/index.html#//apple_ref/doc/uid/TP40004346)。
### The Layer Redraw Policy for OS X Views Affects Performance（OS X 视图的层重绘策略影响性能）
&emsp;在 OS X 中，layer 支持的 view 支持几种不同的策略来确定何时更新底层的内容。由于 native AppKit 绘图模型与 Core Animation 引入的模型之间存在差异，因此这些策略使将旧代码迁移到 Core Animation 变得更加容易。你可以逐个视图地配置这些策略，以确保每个 view 的最佳性能。

&emsp;每个视图定义一个 `layerContentsRedrawPolicy` 方法，该方法返回 view 的 layer 的重绘策略。你可以使用 `setLayerContentsRedrawPolicy:` 方法设置策略。为了保持与传统绘图模型的兼容性，AppKit 将重绘策略默认设置为 `NSViewLayerContentsRedrawDuringViewResize`。但是，你可以将策略更改为表 2-2 中的任何值。请注意，建议的重绘策略不是默认策略。

&emsp;Table 2-2  Layer redraw policies for OS X views（OS X 视图的图层重绘策略）

| Policy | Usage |
| --- | --- |
| NSViewLayerContentsRedrawOnSetNeedsDisplay | 这是推荐的策略。使用此策略，view geometry 更改不会自动导致 view 更新其 layer 内容。相反，layer 的现有内容会被拉伸和操纵，以便于 geometry 更改。要强制 view 重新绘制自身并更新 layer 的内容，必须显式调用 view 的 `setNeedsDisplay:`方法。此策略最接近于表示 Core Animation layers 的标准行为。但是，它不是默认策略，必须显式设置。 |
| NSViewLayerContentsRedrawDuringViewResize | 这是默认的重绘策略。此策略通过在 view 的 geometry 发生更改时重新生成 layer 的内容来保持与传统 AppKit 绘图的最大兼容性。此行为导致在调整大小操作期间在应用程序的主线程上多次调用 view 的 `drawRect:` 方法。 |
| NSViewLayerContentsRedrawBeforeViewResize | 使用此策略，AppKit 在执行任何调整大小操作之前以最终大小绘制 layer，并缓存该位图。调整大小操作使用缓存的位图作为起始图像，缩放它以适应旧的 bounds 矩形。然后将位图设置为最终大小。此行为可能导致 view 的内容在动画开始时出现拉伸或扭曲，在初始外观不重要或不明显的情况下效果更好。 |
| NSViewLayerContentsRedrawNever | 使用此策略，即使调用 `setNeedsDisplay:` 方法，AppKit 也不会更新 layer。此策略最适用于内容从不更改且 view 大小很少更改（如果有的话）的 view。例如，你可以将其用于显示固定大小内容或背景元素的 view。 |

&emsp;view 重绘策略减轻了使用独立 sublayers 来提高绘图性能的需要。在引入 view 重绘策略之前，有一些 layer 支持的 view（layer-backed views）比需要的更频繁，从而导致了性能问题。解决这些性能问题的方法是使用 sublayers 来表示 view 内容中不需要定期重绘的部分。随着 OS X v10.6 中引入重绘策略，现在建议你将 layer 支持的 view（layer-backed views）的重绘策略设置为适当的值，而不是创建显式的 sublayer 层次结构。
### Adding Custom Properties to a Layer（向图层添加自定义属性）
&emsp;CAAnimation 和 CALayer 类扩展了键值编码约定（key-value coding conventions）以支持自定义属性。可以使用此行为向 layer 添加数据，并使用定义的自定义键检索数据。甚至可以将 actions 与自定义属性相关联，以便在更改属性时执行相应的动画。

&emsp;有关如何设置和获取自定义属性的信息，请参见 Key-Value Coding Compliant Container Classes。有关向 layer 对象添加 actions 的信息，请参见 Changing a Layer’s Default Behavior。
### Printing the Contents of a Layer-Backed View（打印 Layer-Backed View 的内容）
&emsp;在打印过程中，layer 根据需要重新绘制其内容以适应打印环境。Core Animation 在渲染到屏幕时通常依赖于缓存的位图，而在打印时它会重新绘制该内容。特别是，如果 layer-backed view 使用 `drawRect:` 方法提供 layer 内容，则 Core Animation 在打印期间再次调用 `drawRect:` 以生成打印的 layer 内容。
## Animating Layer Content（为图层内容动画化）
&emsp;Core Animation 提供的 infrastructure 使你可以轻松创建应用程序 layer 的复杂动画，并扩展到拥有这些 layer 的任何 view。示例包括更改 layer frame 矩形的大小、更改其在屏幕上的位置、应用旋转变换或更改其不透明度。对于 Core Animation，启动动画通常与更改属性一样简单，但也可以创建动画并显式设置动画参数。

&emsp;有关创建更高级动画的信息，请参见 Advanced Animation Tricks。
### Animating Simple Changes to a Layer’s Properties（对图层属性的简单更改进行动画设置）
&emsp;你可以根据需要隐式或显式地执行简单动画。隐式动画使用默认的计时和动画属性来执行动画，而显式动画则要求你使用动画对象自行配置这些属性。因此，隐式动画非常适合在不需要大量代码的情况下进行更改，并且默认计时对你非常合适。

&emsp;简单的动画包括更改 layer 的属性，并让 Core Animation 随着时间的推移为这些更改设置动画。layer 定义了许多影响 layer 可见外观的属性。更改这些属性之一是设置外观更改动画的一种方法。例如，将 layer 的不透明度从 1.0 更改为 0.0 会导致 layer 淡出并变为透明。

> &emsp;Important: 尽管有时你可以直接使用 Core Animation interfaces 为 layer 支持的 view 设置动画，但这样做通常需要额外的步骤。有关如何将 Core Animation 与 layer-backed views 结合使用的更多信息，请参见 How to Animate Layer-Backed Views。

&emsp;要触发隐式动画，你所需要做的就是更新 layer 对象的属性。在 layer 树中修改 layer 对象时，这些对象会立即反映出你的更改。但是，layer 对象的视觉外观不会立即改变。相反，发生的情况是 Core Animation 使用你的更改作为触发来创建和安排一个或多个隐式动画以供执行。因此，像清单 3-1 中那样进行更改，将导致 Core Animation 为你创建一个动画对象，并安排该动画在下一个更新周期开始运行。

&emsp;Listing 3-1  Animating a change implicitly（隐式地对更改进行动画处理）
```c++
theLayer.opacity = 0.0;
```
&emsp;要使用动画对象显式进行相同的更改，请创建 CABasicAnimation 对象，然后使用该对象配置动画参数。在将动画添加到 layer 之前，可以设置动画的开始和结束值，更改持续时间或更改任何其他动画参数。清单 3-2 显示了如何使用动画对象淡出图层。创建对象时，可以为要设置动画的属性指定 key path，然后设置动画参数。要执行动画，请使用 `addAnimation:forKey:` 方法将其添加到要设置动画的 layer 中。

&emsp;Listing 3-2  Animating a change explicitly（明确地显示变化）
```c++
CABasicAnimation* fadeAnim = [CABasicAnimation animationWithKeyPath:@"opacity"];
fadeAnim.fromValue = [NSNumber numberWithFloat:1.0];
fadeAnim.toValue = [NSNumber numberWithFloat:0.0];
fadeAnim.duration = 1.0;
[theLayer addAnimation:fadeAnim forKey:@"opacity"];
 
// Change the actual data value in the layer to the final value.
theLayer.opacity = 0.0;
```

> &emsp;Tip: 创建显式动画时，建议始终将值分配给动画对象的 `fromValue` 属性。如果你未为此属性指定值，则 Core Animation 会使用 layer 的当前值作为起始值。如果你已经将该属性更新为其最终值，则可能不会产生所需的结果。

&emsp;与隐式动画（更新 layer 对象的数据值）不同，显式动画不会修改 layer 树中的数据。显式动画只生成动画。在动画结束时，Core Animation 将从 layer 中移除动画对象，并使用其当前数据值重新绘制 layer。如果希望显式动画中的更改是永久性的，还必须更新层的特性，如前一示例所示。

&emsp;隐式和显式动画通常在当前运行循环周期结束后开始执行，并且当前线程必须具有 run loop 才能执行动画。如果更改多个属性，或者将多个动画对象添加到一个 layer，则所有这些属性更改将同时进行动画处理。例如，你可以通过同时配置两个动画来淡出 layer，同时将其移出屏幕。但是，你也可以将动画对象配置为在特定时间开始。有关修改动画定时的更多信息，请参见 Customizing the Timing of an Animation。
### Using a Keyframe Animation to Change Layer Properties（使用关键帧动画更改图层属性）
&emsp;基于属性的动画将属性从开始值更改为结束值，而 CAKeyframeAnimation 对象使你可以以线性或非线性方式对一组目标值进行动画处理。关键帧动画由一组目标数据值和应达到每个值的时间组成。在最简单的配置中，你可以使用数组指定值和时间。要更改 layer 的位置，还可以使更改遵循路径。动画对象采用你指定的关键帧，并通过在给定的时间段内从一个值插入下一个值来构建动画。

&emsp;图3-1 显示了 layer 的 position 属性的 5 秒动画。对该位置进行动画处理以遵循使用 CGPathRef 数据类型指定的路径。清单 3-3 显示了此动画的代码。

&emsp;Figure 3-1  5-second keyframe animation of a layer’s position property

![keyframing](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/f8602d1a7c55488184cc52c9b5dfdd8d~tplv-k3u1fbpfcp-watermark.image)

&emsp;清单 3-3 显示了用于实现图 3-1 中的动画的代码。此示例中的 path 对象用于为动画的每一帧定义 layer 的位置。

&emsp;Listing 3-3  Creating a bounce keyframe animation（创建弹跳关键帧动画）
```c++
// create a CGPath that implements two arcs (a bounce)
CGMutablePathRef thePath = CGPathCreateMutable();
CGPathMoveToPoint(thePath,NULL,74.0,74.0);
CGPathAddCurveToPoint(thePath,NULL,74.0,500.0,
                                   320.0,500.0,
                                   320.0,74.0);
CGPathAddCurveToPoint(thePath,NULL,320.0,500.0,
                                   566.0,500.0,
                                   566.0,74.0);
 
CAKeyframeAnimation * theAnimation;
 
// Create the animation object, specifying the position property as the key path.
theAnimation=[CAKeyframeAnimation animationWithKeyPath:@"position"];
theAnimation.path=thePath;
theAnimation.duration=5.0;
 
// Add the animation to the layer.
[theLayer addAnimation:theAnimation forKey:@"position"];
```
#### Specifying Keyframe Values（指定关键帧值）
&emsp;关键帧值是关键帧动画中最重要的部分。这些值定义了动画执行过程中的行为。指定关键帧值的主要方法是作为对象数组，但是对于包含 CGPoint 数据类型（例如 layer 的 anchorPoint 和 position 属性）的值，可以改为指定 CGPathRef 数据类型。

&emsp;指定值数组时，你要放入数组中的内容取决于属性所需的数据类型。你可以将一些对象直接添加到数组中。但是，某些对象在添加之前必须强制转换为 id，并且所有标量类型或结构必须由对象包装。例如：

+ 对于采用 CGRect 的属性（例如 bounds 和 frame 属性），请将每个矩形包装到 NSValue 对象中。
+ 对于 layer 的 transform 属性，将每个 CATTransform3D 矩阵包装在 NSValue 对象中。设置此属性的动画将使关键帧动画依次将每个变换矩阵应用于 layer。
+ 对于 borderColor 属性，在将每个 CGColorRef 数据类型添加到数组之前，将其强制转换为类型 id。
+ 对于采用 CGFloat 值的属性，请在将每个值添加到数组之前将其包装到 NSNumber 对象中。
+ 设置 layer contents 属性的动画时，请指定 CGImageRef 数据类型的数组。

&emsp;对于采用 CGPoint 数据类型的属性，可以创建点数组（包装在 NSValue 对象中），也可以使用 CGPathRef 对象指定要遵循的路径。指定点阵列时，关键帧动画对象在每个连续点之间绘制一条直线，并沿着该路径。指定 CGPathRef 对象时，动画从路径的起点开始，并跟随其轮廓，包括沿任何曲面。你可以使用开放或封闭路径。
#### Specifying the Timing of a Keyframe Animation（指定关键帧动画的时间）
&emsp;关键帧动画的 timing and pacing 比基本动画的 timing and pacing 更为复杂，你可以使用几个属性来控制它：

+ calculationMode 属性定义用于计算动画计时的算法。此属性的值会影响其他计时相关属性的使用方式。
  + 线性和三维动画（即 calculationMode 属性设置为 kCAAnimationLinear 或 kCAAnimationCubic 的动画）使用提供的 timing 信息来生成动画。这些模式使你可以最大程度地控制动画时间。
  + 定时动画（即 calculationMode 属性设置为 kCAAnimationPaced 或 kCAAnimationCubicPaced 的动画）不依赖于 keyTimes 或 timingFunctions 属性提供的外部定时值。取而代之的是，隐式计算时间值，以使动画具有恒定的速度。
  + 离散动画（即 calculationMode 属性设置为 kCAAnimationDiscrete 的动画）使动画属性从一个关键帧值跳到下一个关键帧值而没有任何插值。此计算模式使用 keyTimes 属性中的值，但忽略 timingFunctions 属性。
+ keyTimes 属性指定应用每个关键帧值的时间标记。仅当计算模式设置为 kCAAnimationLinear、kCAAnimationDiscrete 或 kCAAnimationCubic 时，才使用此属性。它不用于有节奏的动画。
+ timingFunctions 属性指定要用于每个关键帧段的计时曲线。（此属性替换继承的 timingFunction 属性。）

&emsp;如果要自己处理动画定时，请使用 kCAAnimationLinear 或 kCAAnimationCubic 模式以及 keyTimes 和 timingFunctions 属性。 keyTimes 定义了应用每个关键帧值的时间点。所有中间值的时序均由 timing functions 控制，可让你将缓入或缓出曲线应用于每个段。如果未指定任何 timing functions，则 timing 是线性的。
### Stopping an Explicit Animation While It Is Running（在运行时停止显式动画）
&emsp;动画通常会运行到完成为止，但是如果需要，你可以使用以下一种技术将其停止：
+ 要从 layer 中移除单个动画对象，请调用 layer 的 `removeAnimationForKey:` 方法来移除动画对象。此方法使用传递给 `addAnimation:forKey:` 方法的键来标识动画。指定的键不能为 nil。
+ 要从 layer 中移除所有动画对象，请调用 layer 的 `removeAllAnimations` 方法。此方法会立即移除所有正在进行的动画，并使用 layer 的当前状态信息重新绘制 layer。

> &Note: 你不能直接从 layer 中删除隐式动画。

&emsp;从 layer 中删除动画时，Core Animation 会通过使用其当前值重画该 layer 来做出响应。因为当前值通常不是动画的最终值，所以这可能导致图层的外观突然跳跃。如果希望 layer 的外观保持在动画最后一帧的位置，则可以使用 presentation tree 中的对象来检索这些最终值，并将其设置在 layer 树中的对象上。

&emsp;关于暂停动画并希望 layer 的外观保持在动画最后一帧的位置可参考如下代码（在动画执行过程中点击屏幕即可保持在动画的最后一帧）:
```c++
- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view

    self.ttView = [[UIView alloc] initWithFrame:CGRectMake(50, 200, 100, 100)];
    self.ttView.backgroundColor = [UIColor greenColor];
    [self.view addSubview:self.ttView];

    CABasicAnimation* fadeAnim = [CABasicAnimation animationWithKeyPath:@"opacity"];
    fadeAnim.fromValue = [NSNumber numberWithFloat:1.0];
    fadeAnim.toValue = [NSNumber numberWithFloat:0.0];
    fadeAnim.duration = 5.0;
    [self.ttView.layer addAnimation:fadeAnim forKey:@"opacity"];
    
    // Change the actual data value in the layer to the final value.
    // self.ttView.layer.opacity = 0.0
}

// 在动画执行过程中点击屏幕即可保持在动画的最后一帧
- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [super touchesBegan:touches withEvent:event];
    
    [self.ttView.layer removeAnimationForKey:@"opacity"];
    // 设置最后一帧的 opacity 
    self.ttView.layer.opacity = self.ttView.layer.presentationLayer.opacity;
}

// 下面代码也可用于暂停动画
CFTimeInterval pausedTime = [self.ttView.layer convertTime:CACurrentMediaTime() fromLayer:nil];
self.ttView.layer.speed = 0.0;
self.ttView.layer.timeOffset = pausedTime;

// 下面的代码可恢复动画
CFTimeInterval pausedTime = [self.ttView.layer timeOffset];
self.ttView.layer.speed = 1.0;
self.ttView.layer.timeOffset = 0.0;
self.ttView.layer.beginTime = 0.0;
CFTimeInterval timeSincePause = [self.ttView.layer convertTime:CACurrentMediaTime() fromLayer:nil] - pausedTime;
self.ttView.layer.beginTime = timeSincePause;
```

&emsp;有关暂时暂停动画的信息，请参见清单5-4。
### Animating Multiple Changes Together（一起对多个更改进行动画处理）
&emsp;如果要同时将多个动画应用于 layer 对象，则可以使用 CAAnimationGroup 对象将它们组合在一起。通过提供一个配置点，使用组对象可以简化多个动画对象的管理。应用于组的时间和持续时间值会覆盖各个动画对象中的相同值。

&emsp;清单 3-4 显示了如何使用动画组同时执行两个与 border-related 相关的动画，并且持续时间相同。

&emsp;Listing 3-4  Animating two animations together
```c++
// Animation 1
CAKeyframeAnimation* widthAnim = [CAKeyframeAnimation animationWithKeyPath:@"borderWidth"];
NSArray* widthValues = [NSArray arrayWithObjects:@1.0, @10.0, @5.0, @30.0, @0.5, @15.0, @2.0, @50.0, @0.0, nil];
widthAnim.values = widthValues;
widthAnim.calculationMode = kCAAnimationPaced;
 
// Animation 2
CAKeyframeAnimation* colorAnim = [CAKeyframeAnimation animationWithKeyPath:@"borderColor"];
NSArray* colorValues = [NSArray arrayWithObjects:(id)[UIColor greenColor].CGColor,
            (id)[UIColor redColor].CGColor, (id)[UIColor blueColor].CGColor,  nil];
colorAnim.values = colorValues;
colorAnim.calculationMode = kCAAnimationPaced;
 
// Animation group
CAAnimationGroup* group = [CAAnimationGroup animation];
group.animations = [NSArray arrayWithObjects:colorAnim, widthAnim, nil];
group.duration = 5.0;
 
[myLayer addAnimation:group forKey:@"BorderChanges"];
```
&emsp;将动画分组在一起的更高级的方法是使用事务（transaction）对象。事务允许你创建嵌套的动画集，并为每个动画集指定不同的动画参数，从而提供了更大的灵活性。有关如何使用事务对象的信息，请参见 Explicit Transactions Let You Change Animation Parameters。
### Detecting the End of an Animation（检测动画的结束）
&emsp;Core Animation 为检测动画何时开始或结束提供支持。这些通知是执行与动画相关的任何内务处理任务的好时机。例如，你可以使用开始通知来设置一些相关的状态信息，并使用相应的结束通知来拆除该状态。

&emsp;有两种不同的方式可以通知动画状态：

+ 使用 `setCompletionBlock:` 方法向当前事务添加完成 block。当事务中的所有动画完成时，事务将执行完成 block。
+ 将代理指定给 CAAnimation 对象并实现 `animationDidStart:` 和 `animationDidStop:finished:` 委托方法。

&emsp;如果要将两个动画链接在一起，以便一个动画在另一个动画完成时启动，请不要使用动画通知。相反，请使用动画对象的 beginTime 属性在所需时间启动每个对象。要将两个动画链接在一起，请将第二个动画的开始时间设置为第一个动画的结束时间。有关动画和计时值的详细信息，请参见  Customizing the Timing of an Animation。
### How to Animate Layer-Backed Views（如何对 Layer-Backed Views 进行动画处理）
&emsp;如果 layer 属于 layer-backed view，建议使用 UIKit 或 AppKit 提供的基于 view 的动画接口来创建动画。有一些方法可以直接使用 Core Animation 接口设置 layer 的动画，但是如何创建这些动画取决于目标平台。
#### Rules for Modifying Layers in iOS（在 iOS 中修改图层的规则）
&emsp;因为 iOS view 总是有一个 underlying layer，UIView 类本身直接从 layer 对象派生出它的大部分数据。因此，对 layer 所做的更改也会自动反映在 view 对象中。此行为意味着你可以使用 Core Animation 或 UIView 接口进行更改。

&emsp;如果要使用 Core Animation 类来启动动画，则必须从基于 view 的动画块内部发出所有 Core Animation 调用。UIView 类在默认情况下禁用 layer 动画，但在动画块中重新启用它们。因此，在动画块之外所做的任何更改都不会设置动画。清单 3-5 展示了一个如何隐式更改 layer 的不透明度（opacity）和显式更改 layer 位置的示例。在本例中，myNewPosition 变量预先计算并由块捕获。两个动画同时开始，但不透明度动画以默认计时运行，而位置动画以其动画对象中指定的计时运行。

&emsp;Listing 3-5  Animating a layer attached to an iOS view（动画附加到 iOS 视图的图层）
```c++
[UIView animateWithDuration:1.0 animations:^{
   // Change the opacity implicitly.
   myView.layer.opacity = 0.0;
 
   // Change the position explicitly.
   CABasicAnimation* theAnim = [CABasicAnimation animationWithKeyPath:@"position"];
   theAnim.fromValue = [NSValue valueWithCGPoint:myView.layer.position];
   theAnim.toValue = [NSValue valueWithCGPoint:myNewPosition];
   theAnim.duration = 3.0;
   [myView.layer addAnimation:theAnim forKey:@"AnimateFrame"];
}];
```
#### Rules for Modifying Layers in OS X（在 OS X 中修改图层的规则）
&emsp;要为 OS X 中的 layer-backed view 制作动画变化，最好使用视图本身的接口。如果有的话，很少应该直接修改附加到你的 layer-backed 的 NSView 对象之一的层。 AppKit 负责创建和配置这些 layer 对象，并在你的应用运行时对其进行管理。修改 layer 可能导致其与 view 对象不同步，并可能导致意外结果。对于 layer-backed view，你的代码绝对不能修改 layer 对象的以下任何属性：

+ anchorPoint
+ bounds
+ compositingFilter
+ filters
+ frame
+ geometryFlipped
+ hidden
+ position
+ shadowColor
+ shadowOffset
+ shadowOpacity
+ shadowRadius
+ transform

> &emsp;Important: 前面的限制不适用于 layer-hosting views。如果创建了 layer 对象并手动将其与 view 关联，则负责修改该 layer 的属性并使相应的 view 对象保持同步。

&emsp;默认情况下，AppKit 为其 layer-backed views 禁用隐式动画。view 的 animator 代理对象会自动为你重新启用隐式动画。如果要直接设置 layer 属性的动画，还可以通过将当前 NSAnimationContext 对象的 allowsImplicitation 属性更改为 YES，以编程方式重新启用隐式动画。同样，你应该仅对不在上一列表中的可设置动画的属性执行此操作。
#### Remember to Update View Constraints as Part of Your Animation（切记在动画中更新视图约束）
&emsp;如果使用基于约束的布局规则来管理 view 的位置，则在配置该动画时，必须删除所有可能干扰动画的约束。约束会影响你对 view 的位置或大小所做的任何更改。它们还影响 view 及其子 view 之间的关系。如果要对这些项目中的任何一项进行动画处理，则可以删除约束，进行更改，然后应用所需的新约束。

&emsp;有关约束以及如何使用约束管理 view 布局的更多信息，请参见 Auto Layout Guide。
## Building a Layer Hierarchy（建立图层层次结构）
&emsp;在大多数情况下，在应用中使用 layer 的最佳方法是将其与 view 对象结合使用。但是，有时你可能需要通过向其添加其他 layer 对象来增强 view 层次结构。这样做可能会使用 layer，以提供更好的性能，或者使你实现仅使用 view 很难实现的功能。在这些情况下，你需要了解如何管理你创建的 layer 层次结构。

> &emsp;Important: 在 OS X v10.8 及更高版本中，建议你尽量减少对 layer 层次结构的使用，而只使用 layer-backed views。在该版本的 OS X 中引入的 layer 重绘策略允许你自定义 layer-backed views 的行为，并且仍然可以获得以前使用独立 layer 可能获得的性能。
### Arranging Layers into a Layer Hierarchy
&emsp;layer 层次结构在许多方面类似于 view 层次结构。你将一 layer 嵌入另一 layer 内，以在要嵌入的 layer（称为 sublayer）和父 layer（称为 superlayer）之间创建 parent-child 关系。这种亲子关系影响 sublayer 的各个方面。例如，其内容位于其父对象的内容之上，其位置是相对于其父对象的坐标系指定的，并且它受到应用于父对象的任何变换的影响。
#### Adding, Inserting, and Removing Sublayers（添加、插入和删除子层）
&emsp;每个 layer 对象都有添加、插入和删除 sublayer 的方法。表 4-1 总结了这些方法及其行为。

&emsp;Table 4-1  Methods for modifying the layer hierarchy（修改图层层次结构的方法）
| Behavior | Methods | Description |
| --- | --- | --- |
| Adding layers | addSublayer: | 向当前图层添加一个新的子图层对象。子层将添加到该层的子层列表的末尾。这将导致子层出现在其 zPosition 属性中具有相同值的所有同级对象的顶部。 |
| Inserting layers | insertSublayer:above:
insertSublayer:atIndex:
insertSublayer:below: | 将子层插入到指定索引处或相对于另一个子层的位置的子层层次结构中。在其他子图层上方或下方插入时，你仅在子图层数组中指定子图层的位置。图层的实际可见性主要取决于其 zPosition 属性中的值，其次取决于其在 sublayers 数组中的位置。 |
| Removing layers | removeFromSuperlayer | 从其父层移除子层。 |
| Exchanging layers | replaceSublayer:with: | 将一个子层交换为另一个。如果要插入的子层已经在另一个层层次结构中，则首先将其从该层次结构中删除。 |

&emsp;在处理自己创建的 layer 对象时，可以使用上述方法。你不会使用这些方法来排列属于 layer-backed views 的层。但是，layer-backed views 可以充当你自己创建的独立 layer 的父级。
#### Positioning and Sizing Sublayers（定位和调整子层大小）
&emsp;添加和插入子 layer 时，必须先设置子 layer 的大小和位置，然后该子 layer 才会出现在屏幕上。将子 layer 添加到 layer 层次结构后，可以修改其大小和位置，但是在创建 layer 时应该养成设置这些值的习惯。

&emsp;你可以使用 bounds 属性设置子 layer 的大小，并使用 position 属性设置其在其上 layer 中的位置。bounds 矩形的原点几乎总是（0，0），其大小是你想要的以磅为单位指定的 layer 的大小。 position 属性中的值是相对于 layer 的锚点来解释的，默认情况下，该锚点位于 layer 的中心。如果不为这些属性分配值，则 Core Animation 会将 layer 的初始宽度和高度设置为 0，并将位置设置为（0，0）。

```c++
myLayer.bounds = CGRectMake(0, 0, 100, 100);
myLayer.position = CGPointMake(200, 200);
```
&emsp;Important: 始终对 layer 的宽度和高度使用整数。
#### How Layer Hierarchies Affect Animations（图层层次如何影响动画）
&emsp;某些 superlayer 属性可以影响应用于其子 layer 的任何动画的行为。其中一个属性是 speed 属性，它是动画速度的倍增。默认情况下，此属性的值设置为 1.0，但将其更改为 2.0 会导致动画以两倍于其原始速度运行，从而在一半时间内完成。此属性不仅影响为其设置的 layer，而且还影响该 layer 的子 layer。这种变化也是倍增的。如果子 layer 及其 superlayer 的 speed 都为 2.0，则子 layer 上的动画将以其原始速度的 **四倍** 运行。

&emsp;其他大多数 layer 更改都会以可预测的方式影响任何包含的子 layer。例如，将旋转变换应用于 layer 会旋转该 layer 及其所有子 layer。同样，更改 layer 的不透明度会更改其子 layer 的不透明度。更改 layer 大小时，请遵循 Adjusting the Layout of Your Layer Hierarchies。
### Adjusting the Layout of Your Layer Hierarchies（调整图层层次结构的布局）
&emsp;Core Animation 支持多种选项，可根据子 layer 的更改来调整子 layer 的大小和位置。在 iOS 中，普遍使用 layer-backed views 使创建 layer 层次结构的重要性降低。仅支持手动布局更新。对于 OS X，可以使用其他几个选项，这些选项使管理 layer 层次结构更加容易。

&emsp;仅当使用创建的独立图层对象构建图层层次结构时，图层级别的布局才有意义。如果你应用的图层均与视图相关联，请使用基于视图的布局支持来更新视图的大小和位置以响应更改。
#### Using Constraints to Manage Your Layer Hierarchies in OS X（在 OS X 中使用约束来管理你的层层次结构）
&emsp;约束使你可以使用 layer 及其 superlayer 或同级层之间的一组详细关系来指定 layer 的位置和大小。定义约束需要执行以下步骤：

1. 创建一个或多个 CAConstraint 对象。使用这些对象定义约束参数。
2. 将约束对象添加到它们修改其属性的层。
3. 检索共享的 CAConstraintLayoutManager 对象并将其分配给最接近的 superlayer。

&emsp;图4-1 显示了可用于定义约束的属性以及它们影响的 layer 的外观。你可以使用约束基于其中点边缘相对于另一层的位置来更改该层的位置。你也可以使用它们来更改图层的大小。你所做的更改可以与 superlayer 成比例或相对于另一层。你甚至可以将比例因子或常数添加到结果更改中。这种额外的灵活性使得可以使用一组简单的规则非常精确地控制图层的大小和位置。

&emsp;Figure 4-1  Constraint layout manager attributes（约束布局管理器属性）

![ca_constraint](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/e43aef4f870b4619837bb190c11f3d34~tplv-k3u1fbpfcp-watermark.image)

&emsp;每个约束对象都沿同一轴封装两个图层之间的一个几何关系。每个轴最多可以分配两个约束对象，正是这两个约束确定了哪个属性是可更改的。例如，如果你为图层的左右边缘指定约束，则图层的大小会更改。如果你为图层的左边缘和宽度指定约束，则图层右边缘的位置会更改。如果你为某个图层的边缘指定了一个约束，则 Core Animation 会创建一个隐式约束，该约束将图层的大小固定在给定的尺寸上。

&emsp;创建约束时，必须始终指定三项信息：
+ 要约束的 layer 的 aspect
+ 用作参照的 layer
+ 用于比较的参照 layer 的 aspect

&emsp;清单4-1 显示了一个简单的约束，该约束将 layer 的垂直中点固定到其 superlayer 的垂直中点。当引用 superlayer 时，请使用字符串 superlayer。此字符串是保留用于引用 superlayer 的特殊名称。使用它可以消除指向该 layer 的指针或知道该 layer 名称的麻烦。它还允许你更改 superlayer 并使约束自动应用于新的父级。 （在创建与同级图层有关的约束时，必须使用其 name 属性标识同级图层。）

&emsp;Listing 4-1  Defining a simple constraint
```c++
[myLayer addConstraint:[CAConstraint constraintWithAttribute:kCAConstraintMidY relativeTo:@"superlayer" attribute:kCAConstraintMidY]];
```
&emsp;若要在运行时应用约束，必须将共享的 CAConstraintLayoutManager 对象附加到直接 superlayer。每层负责管理其子层的布局。将布局管理器分配给父级告诉 Core Animation 应用其子级定义的约束。布局管理器对象自动应用约束。将其分配给父层后，不必告诉它更新布局。

&emsp;要查看约束在特定情况下如何工作，请考虑图 4-2。在此示例中，design 要求 layerA 的宽度和高度保持不变，并且 layerA 保持在其 superlayer 内部居中。另外，B层的宽度必须与A层的宽度匹配，B层的顶边缘必须保持在A层的底边缘下方 10 个点，B层的底边缘必须保持在 superlayer 的底边缘上方 10 个点。清单 4-2 显示了用于创建此示例的子层和约束的代码。

&emsp;Figure 4-2  Example constraints based layout

![constraintsManagerExample](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/e8028b7051184c84b6798dcc0611da13~tplv-k3u1fbpfcp-watermark.image)

&emsp;Listing 4-2  Setting up constraints for your layers
```c++
// Create and set a constraint layout manager for the parent layer.
theLayer.layoutManager=[CAConstraintLayoutManager layoutManager];
 
// Create the first sublayer.
CALayer *layerA = [CALayer layer];
layerA.name = @"layerA";
layerA.bounds = CGRectMake(0.0,0.0,100.0,25.0);
layerA.borderWidth = 2.0;
 
// Keep layerA centered by pinning its midpoint to its parent's midpoint.
[layerA addConstraint:[CAConstraint constraintWithAttribute:kCAConstraintMidY
                                                 relativeTo:@"superlayer"
                                                  attribute:kCAConstraintMidY]];
[layerA addConstraint:[CAConstraint constraintWithAttribute:kCAConstraintMidX
                                                 relativeTo:@"superlayer"
                                                  attribute:kCAConstraintMidX]];
[theLayer addSublayer:layerA];
 
// Create the second sublayer
CALayer *layerB = [CALayer layer];
layerB.name = @"layerB";
layerB.borderWidth = 2.0;
 
// Make the width of layerB match the width of layerA.
[layerB addConstraint:[CAConstraint constraintWithAttribute:kCAConstraintWidth
                                                 relativeTo:@"layerA"
                                                  attribute:kCAConstraintWidth]];
 
// Make the horizontal midpoint of layerB match that of layerA
[layerB addConstraint:[CAConstraint constraintWithAttribute:kCAConstraintMidX
                                                 relativeTo:@"layerA"
                                                  attribute:kCAConstraintMidX]];
 
// Position the top edge of layerB 10 points from the bottom edge of layerA.
[layerB addConstraint:[CAConstraint constraintWithAttribute:kCAConstraintMaxY
                                                 relativeTo:@"layerA"
                                                  attribute:kCAConstraintMinY
                                                     offset:-10.0]];
 
// Position the bottom edge of layerB 10 points
//  from the bottom edge of the parent layer.
[layerB addConstraint:[CAConstraint constraintWithAttribute:kCAConstraintMinY
                                                 relativeTo:@"superlayer"
                                                  attribute:kCAConstraintMinY
                                                     offset:+10.0]];
 
[theLayer addSublayer:layerB];
```
&emsp;清单 4-2 需要注意的一件有趣的事情是，代码从不明确设置 layerB 的大小。由于定义了约束，因此每次更新布局时都会自动设置 layerB 的宽度和高度。因此，不需要使用 bounds 矩形设置大小。

> &emsp;Warning: 创建约束时，不要在约束之间创建循环引用。循环约束使得无法计算所需的布局信息。当遇到这种循环引用时，布局行为是未定义的。

#### Setting Up Autoresizing Rules for Your OS X Layer Hierarchies（为 OS X 层层次结构设置自动调整大小规则）
&emsp;自动调整大小规则是在 OS X 中调整层的大小和位置的另一种方法。使用自动调整大小规则，可以指定层的边与 superlayer 的相应边之间应保持固定距离还是可变距离。同样，你可以指定图层的宽度或高度是固定的还是可变的。关系总是在层和它的 superlayer 之间。不能使用自动调整大小规则指定同级层之间的关系。

&emsp;要设置图层的自动调整大小规则，必须为图层的 autoresizingMask 特性指定适当的常量。默认情况下，层被配置为具有固定的宽度和高度。在布局过程中，层的精确大小和位置由 Core Animation 自动计算，并涉及基于许多因素的一组复杂计算。核心动画在要求你的代理执行任何手动布局更新之前应用自动调整大小行为，因此你可以根据需要使用代理调整自动调整大小布局的结果。
#### Manually Laying Out Your Layer Hierarchies（手动布置图层层次结构）
&emsp;在 iOS 和 OS X 上，可以通过在 superlayer 的委托对象上实现 `layoutSublayersOfLayer:` 方法来手动处理布局。你可以使用该方法来调整当前嵌入在图层中的任何子图层的大小和位置。进行手动布局更新时，由你来执行必要的计算以放置每个子层。

&emsp;如果要实现自定义图层子类，则你的子类可以覆盖 `layoutSublayers` 方法，并使用该方法（而不是委托）来处理所有布局任务。仅在需要完全控制自定义图层类中子图层的位置的情况下，才应覆盖此方法。替换默认实现可防止Core Animation 在 OS X 上应用约束或自动调整大小规则。
### Sublayers and Clipping（子层和裁剪）
&emsp;与 view 不同，superlayer 不会自动裁剪位于其 bounds 矩形之外的子图层的内容。相反，默认情况下，superlayer 允许其子层完整显示。但是，可以通过将图层的 masksToBounds 属性设置为 YES 来重新启用剪切。

&emsp;图层的剪贴蒙版的形状包括该图层的圆角半径（如果已指定）。图 4-3 显示了一个图层，该图层演示了 masksToBounds 属性如何影响带有圆角的图层。如果将该属性设置为 NO，则子图层将完整显示，即使它们超出其父图层的范围也是如此。将该属性更改为 YES 将导致其内容被剪切。

&emsp;Figure 4-3  Clipping sublayers to the parent’s bounds（将子图层剪切到父对象的边界）

![clipping](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/1e12eee3f2ec4cf2a872bb9e43d97521~tplv-k3u1fbpfcp-watermark.image)

### Converting Coordinate Values Between Layers（在图层之间转换坐标值）
&emsp;有时，你可能需要将一 layer 中的坐标值转换为另一 layer 中相同屏幕位置的坐标值。 CALayer 类提供了一组可用于此目的的简单转换 routines：

+ convertPoint:fromLayer:
+ convertPoint:toLayer:
+ convertRect:fromLayer:
+ convertRect:toLayer:

&emsp;除了转换点和矩形值外，还可以使用 `convertTime:fromLayer:` 和 `convertTime:toLayer:` 方法在图层之间转换时间值。每一层都定义了自己的本地时间空间，并使用该时间空间将动画的开始和结束与系统的其余部分进行同步。这些时间空间默认情况下是同步的。但是，如果你更改一组图层的动画速度，则这些图层的时间空间会相应更改。你可以使用时间转换方法来说明所有此类因素，并确保两层的时间同步。

## Advanced Animation Tricks（高级动画技巧）
&emsp;有很多方法可以配置基于属性的动画或关键帧动画，以为你做更多的事情。需要一起或顺序执行多个动画的应用可以使用更高级的行为来同步这些动画的时间或将它们链接在一起。你还可以使用其他类型的动画对象来创建视觉过渡和其他有趣的动画效果。
### Transition Animations Support Changes to Layer Visibility（过渡动画支持对图层可见性的更改）
&emsp;顾名思义，过渡动画对象为图层创建了动画的视觉过渡。过渡对象最常见的用途是以一种协调的方式为一层的外观和另一层的消失设置动画。与基于属性的动画不同，在动画中，动画会更改图层的一个属性，而过渡动画会操纵图层的缓存图像来创建视觉效果，而仅通过更改属性就很难或不可能做到。标准的过渡类型可让你执行显示、推动、移动或淡入淡出动画。在 OS X 上，你还可以使用 Core Image 滤镜创建使用其他类型的效果（例如划像，页面卷曲，波纹或你设计的自定义效果）的过渡。

&emsp;要执行 transition 动画，请创建 CATTransition 对象并将其添加到过渡中涉及的层。使用 transition 对象指定要执行的过渡类型以及过渡动画的起点和终点。也不需要使用整个过渡动画。通过过渡对象，可以指定设置动画时要使用的开始和结束进度值。通过这些值，可以在动画的中点开始或结束动画。

&emsp;清单 5-1 显示了用于在两个视图之间创建动画 push transition 的代码。在该示例中，myView1 和 myView2 都位于同一父视图中的同一位置，但是当前仅 myView1 是可见的。按下过渡会使 myView1 向左滑动并淡入淡出，直到它被隐藏，而 myView2 从右侧滑入并变为可见。更新两个视图的 hidden 属性可确保在动画结束时两个视图的可见性正确。

&emsp;Listing 5-1  Animating a transition between two views in iOS（动画 iOS 中两个视图之间的过渡）
```c++
CATransition* transition = [CATransition animation];
transition.startProgress = 0;
transition.endProgress = 1.0;
transition.type = kCATransitionPush;
transition.subtype = kCATransitionFromRight;
transition.duration = 1.0;
 
// Add the transition animation to both layers
[myView1.layer addAnimation:transition forKey:@"transition"];
[myView2.layer addAnimation:transition forKey:@"transition"];
 
// Finally, change the visibility of the layers.
myView1.hidden = YES;
myView2.hidden = NO;
```
&emsp;当同一 transition 中涉及两个 layer 时，可以对这两个 layer 使用相同的 transition 对象。使用相同的 transition 对象也简化了必须编写的代码。但是，可以使用不同的 transition 对象，如果每个 layer 的 transition 参数不同，则肯定需要这样做。

&emsp;清单 5-2 显示了如何使用 Core Image filter 在 OS X 上实现过渡效果。在为 filter 配置了所需的参数之后，将其分配给 transition 对象的 filter 属性。此后，应用动画的过程与其他类型的动画对象相同。

&emsp;Listing 5-2  Using a Core Image filter to animate a transition on OS X（使用 Core Image 过滤器为 OS X 上的过渡设置动画）
```c++
// Create the Core Image filter, setting several key parameters.
CIFilter* aFilter = [CIFilter filterWithName:@"CIBarsSwipeTransition"];
[aFilter setValue:[NSNumber numberWithFloat:3.14] forKey:@"inputAngle"];
[aFilter setValue:[NSNumber numberWithFloat:30.0] forKey:@"inputWidth"];
[aFilter setValue:[NSNumber numberWithFloat:10.0] forKey:@"inputBarOffset"];
 
// Create the transition object
CATransition* transition = [CATransition animation];
transition.startProgress = 0;
transition.endProgress = 1.0;
transition.filter = aFilter;
transition.duration = 1.0;
 
[self.imageView2 setHidden:NO];
[self.imageView.layer addAnimation:transition forKey:@"transition"];
[self.imageView2.layer addAnimation:transition forKey:@"transition"];
[self.imageView setHidden:YES];
```
> &emsp;Note: 在动画中使用 Core Image filters 时，最棘手的部分是配置 filter 。例如，对于 bar swipe 过渡，指定过高或过低的输入角度可能会使其看起来好像没有发生过渡。如果没有看到预期的动画，请尝试将 filter 参数调整为不同的值，以查看这是否会更改结果。

### Customizing the Timing of an Animation（自定义动画的时间）
&emsp;Timing 是动画的重要组成部分，使用 Core Animation 可以通过 CAMediaTiming 协议的方法和属性为动画指定精确的定时信息。两个核心动画类采用此协议。 CAAnimation 类采用它，以便你可以在动画对象中指定计时信息。 CALayer 也采用了它，因此你可以为隐式动画配置一些与时间相关的功能，尽管包装那些动画的隐式事务对象通常会提供优先的默认时间信息。

&emsp;在考虑 timing 和 animations 时，重要的是了解 layer 对象如何随时间工作。每个 layer 都有自己的本地时间，用于管理动画时间。通常，两个不同层的本地时间足够接近，你可以为每个层指定相同的时间值，并且用户可能不会注意到任何事情。但是，层的本地时间可以通过其父层或自己的时序参数进行修改。例如，更改图层的 speed 属性会导致该图层（及其子图层）上的动画持续时间按比例更改。

&emsp;为了帮助你确保时间值适合给定的图层，CALayer 类定义了 `convertTime:fromLayer:` 和 `convertTime:toLayer:` 方法。你可以使用这些方法将固定时间值转换为图层的本地时间，或将时间值从一层转换为另一层。这些方法考虑了可能影响该图层本地时间的媒体计时属性，并返回了可与其他图层一起使用的值。清单 5-3 显示了一个示例，你应定期使用该示例来获取图层的当前本地时间。 CACurrentMediaTime 函数是一种便捷函数，它返回计算机的当前时钟时间，该方法将采用该时间并将其转换为图层的本地时间。

&emsp;Listing 5-3  Getting a layer’s current local time（获取图层的当前本地时间）
```c++
CFTimeInterval localLayerTime = [myLayer convertTime:CACurrentMediaTime() fromLayer:nil];
```
&emsp;在图层的本地时间获得时间值后，就可以使用该值来更新动画对象或图层的与时间相关的属性。使用这些计时属性，你可以实现一些有趣的动画行为，包括：
+ 使用 beginTime 属性设置动画的开始时间。通常，动画在下一个更新周期中开始。可以使用 beginTime 参数将动画开始时间延迟几秒。将两个动画链接在一起的方法是将一个动画的开始时间设置为与另一个动画的结束时间匹配。
  如果延迟动画的开始，可能还需要将 fillMode 属性设置为 kCAFillModeBackwards。此填充模式使层显示动画的开始值，即使层树中的层对象包含不同的值。如果没有此填充模式，你将看到在动画开始执行之前跳转到最终值。其他填充模式也可用。
+ autoreverses 属性使动画在指定的持续时间内执行，然后返回到动画的起始值。可以将此属性与 repeatCount 属性结合起来，在开始值和结束值之间来回设置动画。将自动反转动画的重复计数设置为整数（例如 1.0）会导致动画在其起始值停止。添加额外的半步（例如重复计数为 1.5）会导致动画在其结束值处停止。
+ 对组动画使用 timeOffset 属性可以在比其他动画晚的时间启动某些动画。

### Pausing and Resuming Animations
&emsp;要暂停动画，你可以利用图层采用 CAMediaTiming 协议并将图层动画的速度设置为 0.0 的事实。将速度设置为零会暂停动画，直到你将值更改回非零值为止。清单 5-4 给出了一个简单的示例，说明如何稍后暂停和恢复动画。

&emsp;Listing 5-4  Pausing and resuming a layer’s animations（暂停和恢复图层的动画）
```c++
-(void)pauseLayer:(CALayer*)layer {
   CFTimeInterval pausedTime = [layer convertTime:CACurrentMediaTime() fromLayer:nil];
   layer.speed = 0.0;
   layer.timeOffset = pausedTime;
}
 
-(void)resumeLayer:(CALayer*)layer {
   CFTimeInterval pausedTime = [layer timeOffset];
   layer.speed = 1.0;
   layer.timeOffset = 0.0;
   layer.beginTime = 0.0;
   CFTimeInterval timeSincePause = [layer convertTime:CACurrentMediaTime() fromLayer:nil] - pausedTime;
   layer.beginTime = timeSincePause;
}
```
### Explicit Transactions Let You Change Animation Parameters（显式事务可让你更改动画参数）
&emsp;对图层所做的每一个更改都必须是事务的一部分。CATransaction 类管理动画的创建和分组，并在适当的时间执行动画。在大多数情况下，你不需要创建自己的事务。无论何时向某个层添加显式或隐式动画，Core Animation 都会自动创建隐式事务。但是，你也可以创建显式事务来更精确地管理这些动画。

&emsp;你可以使用 CATransaction 类的方法创建和管理事务。要开始（并隐式地创建）新事务，请调用 begin 类方法。要结束该事务，请调用 commit 类方法。在这些调用之间是要包含在事务中的更改。例如，要更改图层的两个属性，可以使用清单 5-5 中的代码。

&emsp;Listing 5-5  Creating an explicit transaction
```c++
[CATransaction begin];
theLayer.zPosition=200.0;
theLayer.opacity=0.0;
[CATransaction commit];
```
&emsp;使用事务的主要原因之一是，在显式事务的范围内，可以更改持续时间、计时函数和其他参数。你还可以为整个事务分配一个完成 block，以便在动画组完成时通知你的应用程序。更改动画参数需要使用 `setValue:forKey:` 方法。例如，要将默认持续时间更改为 10 秒，你需要更改 kCATransactionAnimationDuration 键，如清单 5-6 所示。

&emsp;Listing 5-6  Changing the default duration of animations（更改动画的默认持续时间）
```c++
[CATransaction begin];
[CATransaction setValue:[NSNumber numberWithFloat:10.0f] forKey:kCATransactionAnimationDuration];
// Perform the animations
[CATransaction commit];
```
&emsp;在希望为不同的动画集提供不同的默认值的情况下，可以嵌套事务。要将一个事务嵌套在另一个事务中，只需再次调用 `begin` 类方法。每个 `begin` 调用必须与 `commit` 方法的相应调用相匹配。只有在提交最外层事务的更改之后，Core Animation 才会开始关联的动画。

&emsp;清单 5-7 显示了一个嵌套在另一个事务中的事务的示例。在本例中，内部事务更改与外部事务相同的动画参数，但使用不同的值。

&emsp;Listing 5-7  Nesting explicit transactions
```c++
[CATransaction begin]; // Outer transaction
 
// Change the animation duration to two seconds
[CATransaction setValue:[NSNumber numberWithFloat:2.0f]
                forKey:kCATransactionAnimationDuration];
// Move the layer to a new position
theLayer.position = CGPointMake(0.0,0.0);
 
[CATransaction begin]; // Inner transaction
// Change the animation duration to five seconds
[CATransaction setValue:[NSNumber numberWithFloat:5.0f]
                 forKey:kCATransactionAnimationDuration];
 
// Change the zPosition and opacity
theLayer.zPosition=200.0;
theLayer.opacity=0.0;
 
[CATransaction commit]; // Inner transaction
 
[CATransaction commit]; // Outer transaction
```
### Adding Perspective to Your Animations（为动画添加透视图）
&emsp;应用程序可以在三个空间维度上操纵图层，但为简单起见，Core Animation 使用平行投影来显示图层，该投影实质上将场景展平为二维平面。此默认行为会导致具有相同 zPosition 值的大小相同的图层显示为相同的大小，即使它们在 z 轴上相距很远也是如此。你通常可以从三个维度查看此类场景的透视图（perspective）已消失。但是，你可以通过修改图层的转换矩阵以包括透视图（perspective）信息来更改该行为。

&emsp;修改场景的透视图（perspective）时，需要修改包含正在查看的图层的 superlayer 的 sublayerTransform 矩阵。通过对所有子层应用相同的透视图信息，修改 superlayer 可简化你必须编写的代码。它还确保将透视图正确地应用于在不同平面中彼此重叠的同级子层。

&emsp;清单5-8 显示了为 parent layer 创建简单的透视变换（perspective transform）的方法。在这种情况下，自定义 eyePosition 变量指定沿 z 轴查看图层的相对距离。通常，你为 eyePosition 指定一个正值，以使图层保持预期的方向。较大的值会导致场景更平整，而较小的值会导致图层之间更明显的视觉差异。

&emsp;Listing 5-8  Adding a perspective transform to a parent layer（向父图层添加透视变换）
```c++
CATransform3D perspective = CATransform3DIdentity;
perspective.m34 = -1.0/eyePosition;
 
// Apply the transform to a parent layer.
myParentLayer.sublayerTransform = perspective;
```
&emsp;配置了 parent layer 后，你可以更改任何 child layers 的 zPosition 属性，并根据它们与 eye position 的相对距离来观察其大小如何变化。
## Changing a Layer’s Default Behavior（更改图层的默认行为）
&emsp;Core Animation 使用 action 对象实现 layer 的隐式动画行为。action 对象是符合 CAAction 协议并定义要在 layer 上执行的一些相关行为的对象。所有 CAAnimation 对象都实现了该协议，并且通常是这些对象被指定在图层属性更改时执行。

&emsp;动画属性是 action 的一种，但是你可以定义具有几乎任何所需行为的 action。不过，为此，你必须定义 action 对象并将其与应用程序的 layer 对象相关联。
### Custom Action Objects Adopt the CAAction Protocol（自定义 Action 对象采用 CAAction 协议）
&emsp;要创建自己的 action 对象，请从你的一个类中采用 CAAction 协议，并实现 `runActionForKey:object:arguments:` 方法。在该方法中，使用可用信息来执行要在 layer 上执行的任何 action。你可以使用该方法将动画对象添加到 layer，也可以使用它执行其他任务。

&emsp;定义 action 对象时，必须决定如何触发该 action。一个 action 的触发器定义了你以后用来注册该动作的 key。可以通过以下任意一种情况触发 action 对象：

+ layer 属性之一的值已更改。这可以是 layer 的任何属性，而不仅仅是可设置动画的属性。（也可以将动作与添加到图层的自定义属性相关联。）标识此 action 的键是属性的名称。
+ layer 变为可见或已添加到 layer 层次结构中。识别此 action 的 key 是 kCAOnOrderIn。
+ layer 已从 layer 层次结构中删除。识别此 action 的 key 是 kCAOnOrderOut。
+ layer 即将参与 transition 动画。识别此 action 的 key 是 kCATransition。

### Action Objects Must Be Installed On a Layer to Have an Effect（action 对象必须安装在层上才能生效）
&emsp;在可以执行 action 之前，该 layer 需要找到要执行的相应 action 对象。与 layer 相关的 action 的 key 是要修改的属性的名称或标识 action 的特殊字符串。当 layer 上发生适当的事件时，layer 将调用其 `actionForKey:` 方法来搜索与 key 关联的 action 对象。在此搜索过程中，你的应用可以将自己插入多个位置，并为该 key 提供相关的 action 对象。

&emsp;Core Animation 按以下顺序查找 action 对象：
1. 如果 layer 有一个 delegate 并且该 delegate 实现了 `actionForLayer:forKey:` 方法，layer 调用该方法。delegate 必须执行以下操作之一：
  + 返回给定 key 的 action 对象。
  + 返回 nil，如果它不处理该 action，则继续搜索。
  + 返回 NSNull 对象，在这种情况下，搜索立即结束。
2. layer 在 layer 的 actions 字典中查找给定的 key。
3. layer 在 style 字典中查找包含 key 的 actions 字典。（换句话说，style 字典包含一个 actions 键，其值也是字典。layer 在第二个字典中查找给定的 key。）
4. layer 调用 defaultActionForKey: 类方法。
5. layer 执行由 Core Animation 定义的隐式动作（如果有）。

&emsp;如果在任何适当的搜索点提供 action 对象，则 layer 将停止其搜索并执行返回的 action 对象。找到 action 对象后，该 layer 会调用该对象的 `runActionForKey:object:arguments:` 方法来执行该 action。如果你为给定 key 定义的 action 已经是 CAAnimation 类的实例，则可以使用该方法的默认实现来执行动画。如果你要定义自己的符合 CAAction 协议的自定义对象，则必须使用该方法的对象实现来执行适当的操作。

&emsp;action 对象的安装位置取决于你打算如何修改 layer。

+ 对于可能仅在特定情况下应用的 actions，或者对于已使用 delegate 对象的 layer，请提供 delegate 并实现其 `actionForLayer:forKey:` 方法。
+ 对于通常不使用 delegate 的 layer 对象，请将 action 添加到 layer 的 actions 字典中。
+ 对于与在 layer 对象上定义的自定义属性相关的 action，请将该 action 包含在 layer 的 style 字典中。
+ 对于对 layer 的行为至关重要的 action，请对 layer 进行子类化并重写 `defaultActionForKey:` 方法。

&emsp;清单 6-1 显示了用于提供 action 对象的 delegate 方法的实现。在这种情况下，delegate 将查找对 layer 的 contents 属性的更改，并使用 transition 动画将新内容交换到位。

&emsp;Listing 6-1  Providing an action using a layer delegate object（使用图层委托对象提供动作）
```c++
- (id<CAAction>)actionForLayer:(CALayer *)theLayer forKey:(NSString *)theKey {
    CATransition *theAnimation=nil;
 
    if ([theKey isEqualToString:@"contents"]) {
 
        theAnimation = [[CATransition alloc] init];
        theAnimation.duration = 1.0;
        theAnimation.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseIn];
        theAnimation.type = kCATransitionPush;
        theAnimation.subtype = kCATransitionFromRight;
    }
    return theAnimation;
}
```
### Disable Actions Temporarily Using the CATransaction Class（使用 CATransaction 类暂时禁用操作）
&emsp;你可以使用 CATransaction 类暂时禁用 layer action。更改 layer 的属性时，Core Animation 通常会创建一个隐式事务对象以使更改动起来。如果不想为更改设置动画，则可以通过创建显式事务并将其 kCATransactionDisableActions 属性设置为 true 来禁用隐式动画。清单 6-2 显示了一段代码片段，当从 layer 树中删除指定的 layer 时，该片段将禁用动画。

&emsp;Listing 6-2  Temporarily disabling a layer’s actions（暂时停用图层操作）
```c++
[CATransaction begin];
[CATransaction setValue:(id)kCFBooleanTrue forKey:kCATransactionDisableActions];
[aLayer removeFromSuperlayer];
[CATransaction commit];
```
&emsp;有关使用 transaction 对象管理动画行为的更多信息，请参见 Explicit Transactions Let You Change Animation Parameters。
## Improving Animation Performance（改善动画效果）
&emsp;Core Animation 是提高 app-based 的动画帧速率的好方法，但它的使用并不能保证性能的提高。尤其是在 OS X 中，你仍然必须选择使用 Core Animation 行为的最有效方法。与所有与性能相关的问题一样，你应该使用工具来测量和跟踪应用程序的性能，这样你就可以确保性能正在改善，而不是倒退。
### Choose the Best Redraw Policy for Your OS X Views（为你的 OS X 视图选择最佳重绘策略）
&emsp;NSView 类的默认重绘策略将保留该类的原始绘制行为，即使视图是基于图层（layer-backed）的。如果你在应用程序中使用层支持（ layer-backed）的视图，则应检查重绘策略选择，然后选择为你的应用程序提供最佳性能的选择。在大多数情况下，默认策略不是最有可能提供最佳性能的策略。相反，NSViewLayerContentsRedrawOnSetNeedsDisplay 策略更有可能减少应用程序绘制的次数并提高性能。其他策略也可能为特定类型的视图提供更好的性能。

&emsp;有关视图重绘策略的更多信息，请参见 The Layer Redraw Policy for OS X Views Affects Performance。

### Update Layers in OS X to Optimize Your Rendering Path（更新 OS X 中的图层以优化渲染路径）
&emsp;在 OS X v10.8 及更高版本中，视图有两个更新底层内容的选项。在 OS X v10.7 及更早版本中更新以图层为背景（layer-backed）的视图时，图层会将视图的 `drawRect:` 方法中的绘图命令捕获到背景位图图像中。缓存图形命令是有效的，但不是所有情况下最有效的选项。如果你知道如何直接提供层的内容而不实际呈现它们，那么可以使用 `updateLayer` 方法来实现。

&emsp;有关渲染的不同路径（包括涉及 `updateLayer` 方法的路径）的信息，请参见 Using a Delegate to Provide the Layer’s Content。
### General Tips and Tricks（一般提示和技巧）
&emsp;有几种方法可以使你的图层实现更加高效。但是，与任何此类优化一样，在尝试进行优化之前，应始终测量代码的当前性能。这为你提供了一个基准，可用于确定优化是否有效。
#### Use Opaque Layers Whenever Possible（尽可能使用不透明层）
&emsp;将图层的 opaque 属性设置为 YES，可以使 Core Animation 知道它不需要为图层维护 Alpha 通道。没有 Alpha 通道意味着合成器不需要将图层的内容与其背景内容混合在一起，从而节省了渲染时间。但是，此属性主要与作为图层支持视图一部分的图层或 Core Animation 创建基础图层位图的情况有关。如果你直接将图像分配给图层的 content 属性，则无论 opaque 属性中的值如何，该图像的 Alpha 通道都会保留。
#### Use Simpler Paths for CAShapeLayer Objects（为 CAShapeLayer 对象使用更简单的路径）
&emsp;CAShapeLayer 类通过在合成时将你提供的路径渲染到位图图像中来创建其内容。优点是该层始终以最佳分辨率绘制路径，但是该优点是以增加渲染时间为代价的。如果你提供的路径很复杂，则光栅化（rasterizing）该路径可能会变得过于昂贵。而且，如果层的大小频繁更改（因此必须频繁重绘），则绘制所花费的时间可能加起来并成为性能瓶颈。

&emsp;最小化 shape layers 绘制时间的一种方法是将复杂的形状分解为更简单的形状。使用更简单的路径并将多个 CAShapeLayer 对象彼此叠加在合成器中，比绘制一条大型复杂路径要快得多。这是因为绘制操作发生在 CPU 上，而合成发生在 GPU 上。但是，与这种性质的任何简化一样，潜在的性能提升取决于你的内容。因此，在优化之前测量代码的性能尤其重要，这样你就可以将基准用于比较。
#### Set the Layer Contents Explicitly for Identical Layers（明确设置相同图层的图层内容）
&emsp;如果要在多个图层对象中使用同一图像，请自行加载该图像并将其直接分配给那些图层对象的 content 属性。将图像分配给 contents 属性可防止该层为备份存储（backing store）分配内存。而是，图层使用你提供的图像作为备份存储（backing store）。当多个层使用同一图像时，这意味着所有这些层都共享同一内存，而不是为它们自己分配图像的副本。
#### Always Set a Layer’s Size to Integral Values（始终将图层大小设置为整数值）
&emsp;为了获得最佳结果，请始终将图层对象的宽度和高度设置为整数值。尽管你使用浮点数指定了图层 bounds 的宽度和高度，但最终还是使用图层 bounds 来创建位图图像。指定宽度和高度的整数值可简化 Core Animation 创建和管理备份存储以及其他图层信息所必须完成的工作。
#### Use Asynchronous Layer Rendering As Needed（根据需要使用异步层渲染）
&emsp;你在委托人的 `drawLayer:inContext:` 方法或视图的 `drawRect:` 方法中所做的任何绘制通常在应用程序的主线程上同步发生。但是，在某些情况下，同步绘制内容可能无法提供最佳性能。如果发现动画效果不佳，则可以尝试在图层上启用 `drawsAsynchronously` 属性，以将这些操作移至后台线程。如果这样做，请确保绘图代码是线程安全的。与往常一样，在将其放入生产代码之前，你应该始终异步测量绘图的性能。
#### Specify a Shadow Path When Adding a Shadow to Your Layer（在图层上添加阴影时指定阴影路径）
&emsp;让 Core Animation 确定阴影的形状可能会很昂贵，并且会影响应用的性能。而不是让 Core Animation 确定阴影的形状，而是使用 CALayer 的 shadowPath 属性显式指定阴影形状。当你为此属性指定路径对象时，Core Animation 将使用该形状绘制并缓存阴影效果。对于形状从未改变或很少改变的图层，这可以通过减少 Core Animation 完成的渲染量来大大提高性能。

## Appendix A: Layer Style Property Animations（图层样式属性动画）
&emsp;在渲染过程中，Core Animation 会采用图层的不同属性，并以特定顺序进行渲染。此顺序确定层的最终外观。本章说明通过设置不同的图层样式属性获得的渲染结果。

> &emsp;Note: Mac OS X 和 iOS 上可用的图层样式属性有所不同，并在本章中进行了说明。

### Geometry Properties（几何属性）
&emsp;layer 的 geometry 属性指定相对于其 parent layer 的显示方式。geometry 还指定了用于使 layer 角变圆的半径以及应用于该 layer 及其 sublayers 的变换。图 A-1 显示了示例 layer 的 bounding 矩形。

&emsp;Figure A-1  Layer geometry

![visual-geometry](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/669e07b69b3b4b9ba3a16f2164d3e53e~tplv-k3u1fbpfcp-watermark.image)

&emsp;以下 CALayer 属性指定了 layer 的 geometry：

+ bounds
+ position
+ frame (从 bounds 和 position 计算得出，并且不能设置动画)
+ anchorPoint
+ cornerRadius
+ transform
+ zPosition

> &emsp;iOS Note: 只有 iOS 3.0 及更高版本才支持 cornerRadius 属性。

### Background Properties（背景属性）
&emsp;Core Animation 渲染的第一件事是 layer 的背景。你可以为背景指定颜色。在 OS X 中，你还可以指定要应用于背景内容的 Core Image 过滤器。图 A-2 显示了一个示例 layer 的两个版本。左侧的 layer 设置有 backgroundColor 属性，而右侧的 layer 则没有背景色，但是确实有一些内容的边框，并且为其 backgroundFilters 属性分配了捏变形滤镜（pinch distortion filter）。

&emsp;Figure A-2  Layer with background color（具有背景色的图层）

![visual-background](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/76550ea6322c48c6823a3f3e6c8fd865~tplv-k3u1fbpfcp-watermark.image)

&emsp;背景过滤器（background filter）将应用于位于 layer 后面的内容，该内容主要由 parent layer 的内容组成。你可以使用 background filter 使前景层（foreground layer）内容突出。例如，通过应用模糊滤镜（blur filter）。

&emsp;以下 CALayer 属性会影响图层背景的显示：

+ backgroundColor
+ backgroundFilters (not supported in iOS)

> &emsp;Platform Note: 在 iOS 中，backgroundFilters 属性在 CALayer 类中公开，但你分配给该属性的过滤器将被忽略。

### Layer Content（图层内容）
&emsp;如果该 layer 包含任何内容，则该内容将呈现在 background color 之上。你可以通过直接设置位图，使用 delegate 指定内容或子类化图层并直接绘制内容来提供图层内容。你可以使用许多不同的绘图技术（包括 Quartz、Metal、OpenGL 和 Quartz Composer）来提供该内容。图 A-3 显示了一个示例图层，其内容是直接设置的位图。位图内容由一个高度透明的空间组成，右下角有 Automator 图标。

&emsp;Figure A-3  Layer displaying a bitmap image（显示位图图像的图层）

![visual-contents](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/ab8e6a84e5764afca16ce412508097c0~tplv-k3u1fbpfcp-watermark.image)

&emsp;具有 corner radius 的 layer 不会自动裁剪其内容。但是，将 layer 的 masksToBounds 属性设置为 YES 会导致 layer 裁剪到其 corner radius。

&emsp;以下 CALayer 属性会影响 layer 内容的显示：

+ contents
+ contentsGravity
+ masksToBounds

### Sublayers Content（子层内容）
&emsp;任何一 layer 都可以包含一个或多个 child layers，称为 sublayers。sublayers 是递归渲染的，并且相对于 parent layer 的 bounds 矩形定位。此外，Core Animation 还将 parent layer 的 sublayerTransform 应用于相对于 parent layer 锚点（anchor point）的每个 sublayer 。你可以使用 sublayer transform 将 perspective 和其他效果均等地应用于所有 layers。图 A-4 显示了具有两个 sublayers 的示例 layer。左侧的版本包含背景色，而右侧的版本则不包含背景色。

&emsp;Figure A-4  Layer displaying the sublayers content（显示子图层内容的图层）

![visual-sublayers](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/1a0bd0c450dc4c468b52d670fdcc8626~tplv-k3u1fbpfcp-watermark.image)

&emsp;将图层的 masksToBounds 属性设置为 YES 会导致所有 sublayers 都被裁剪到该图层的 bounds。

&emsp;以下 CALayer 属性会影响图层 sublayers 的显示：

+ sublayers
+ masksToBounds
+ sublayerTransform

### Border Attributes（边框属性）
&emsp;图层可以使用指定的颜色和宽度显示可选 border。border 遵循图层的 bounds 矩形，并考虑了所有 corner radius。图 A-5 显示了应用 border 后的示例图层。请注意，超出图层 bounds 的内容和子图层将在 border 下方呈现。

Figure A-5  Layer displaying the border attributes content

![visual-borderwidth](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/3411106522a649249b3402a8ba873bcf~tplv-k3u1fbpfcp-watermark.image)

&emsp;以下 CALayer 属性会影响图层 board 的显示：

+ borderColor
+ borderWidth

> &emsp;Platform Note: 只有 iOS 3.0 和更高版本才支持 borderColor 和 borderWidth 属性。

### Filters Property（过滤器属性）
&emsp;在 OS X 中，你可以将一个或多个过滤器应用于图层的内容，并使用自定义合成过滤器来指定图层的内容如何与底层图层的内容混合。图 A-6 显示了一个示例层，其中应用了 Core Image Posterize 过滤器。

&emsp;Figure A-6  Layer displaying the filters properties

![visual-filters](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/118f204f4e114dec80e7b0df3975fa45~tplv-k3u1fbpfcp-watermark.image)

&emsp;以下 CALayer 属性指定图层内容过滤器：

+ filters
+ compositingFilter

> &emsp;Platform Note: 在 iOS 中，图层会忽略你分配给它们的所有过滤器。

### Shadow Properties
&emsp;图层可以显示阴影效果，并配置其形状、不透明度、颜色、偏移和模糊半径。如果未指定自定义阴影形状，则阴影将基于图层的不完全透明的部分。图A-7 显示了同一示例图层的几种不同版本，并应用了红色阴影。左侧和中间版本包括背景色，因此阴影仅出现在图层边框周围。但是，右侧的版本不包含背景色。在这种情况下，阴影将应用于图层的内容，边框和子图层。

&emsp;Figure A-7  Layer displaying the shadow properties

![visual-shadow](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/9440dbb267a049a294468b781700ceed~tplv-k3u1fbpfcp-watermark.image)

&emsp;以下 CALayer 属性会影响图层阴影的显示：

+ shadowColor
+ shadowOffset
+ shadowOpacity
+ shadowRadius
+ shadowPath

> &emsp;Platform Note: iOS 3.2 及更高版本支持 shadowColor、shadowOffset、shadowOpacity 和 shadowRadius 属性。 iOS 3.2 和更高版本以及 OS X v10.7 和更高版本支持 shadowPath 属性。

### Opacity Property
&emsp;图层的不透明度属性决定了该图层显示多少背景内容。图 A-8 显示了不透明度设置为 0.5 的示例图层。这样可以使部分背景图像显示出来。

&emsp;Figure A-8  Layer including the opacity property

![visual-opacity](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/d1e31cdee2384a07826d9e71bc3de00c~tplv-k3u1fbpfcp-watermark.image)

&emsp;以下 CALayer 属性指定图层的不透明度：

+ opacity

### Mask Properties
&emsp;你可以使用 mask 遮盖层的全部或部分内容。mask 本身就是一个图层对象，其 Alpha 通道用于确定哪些对象被阻止以及哪些对象被传输。mask layer 内容的不透明部分使底层的内容可以显示出来，而透明部分会部分或完全遮盖底层的内容。图 A-9 显示了一个与 mask layer 和两个不同背景合成的示例层。在左侧版本中，图层的不透明度设置为 1.0。在右侧的版本中，图层的不透明度设置为 0.5，这会增加通过图层的蒙版部分传输的背景内容的数量。

&emsp;Figure A-9  Layer composited with the mask property

![visual-mask](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/fba60b432ee8419da5fab9a16c772b2a~tplv-k3u1fbpfcp-watermark.image)

&emsp;以下 CALayer 属性指定图层的遮罩：

+ mask

> &emsp;Platform Note: iOS 3.0 及更高版本支持 mask 属性。

## Appendix B: Animatable Properties（可动画属性）
&emsp;CALayer 和 CIFilter 中的许多属性都可以设置动画。本附录列出了这些属性，以及默认情况下使用的动画。

### CALayer Animatable Properties（CALayer 动画属性）
&emsp;表 B-1 列出了可以考虑设置动画的 CALayer 类的属性。对于每个属性，该表还列出了为执行隐式动画而创建的默认动画对象的类型。

Table B-1  Layer properties and their default animations（图层属性及其默认动画）

| Property | Default animation |
| --- | --- |
| anchorPoint | Uses the default implied CABasicAnimation object, described in Table B-2. |
| backgroundColor | Uses the default implied CABasicAnimation object, described in Table B-2. |
| backgroundFilters | Uses the default implied CATransition object, described in Table B-3. Sub-properties of the filters are animated using the default implied CABasicAnimation object, described in Table B-2. |
| borderColor | Uses the default implied CABasicAnimation object, described in Table B-2. |
| borderWidth | Uses the default implied CABasicAnimation object, described in Table B-2. |
| bounds | Uses the default implied CABasicAnimation object, described in Table B-2. |
| compositingFilter | Uses the default implied CATransition object, described in Table B-3. Sub-properties of the filters are animated using the default implied CABasicAnimation object, described in Table B-2. |
| contents | Uses the default implied CABasicAnimation object, described in Table B-2. |
| contentsRect | Uses the default implied CABasicAnimation object, described in Table B-2. |
| cornerRadius | Uses the default implied CABasicAnimation object, described in Table B-2. |
| doubleSided | There is no default implied animation. |
| filters | Uses the default implied CABasicAnimation object, described in Table B-2. Sub-properties of the filters are animated using the default implied CABasicAnimation object, described in Table B-2. |
| frame | This property is not animatable. You can achieve the same results by animating the bounds and position properties. |
| hidden | Uses the default implied CABasicAnimation object, described in Table B-2. |
| mask | Uses the default implied CABasicAnimation object, described in Table B-2. |
| masksToBounds | Uses the default implied CABasicAnimation object, described in Table B-2. |
| opacity | Uses the default implied CABasicAnimation object, described in Table B-2. |
| position | Uses the default implied CABasicAnimation object, described in Table B-2. |
| shadowColor | Uses the default implied CABasicAnimation object, described in Table B-2. |
| shadowOffset | Uses the default implied CABasicAnimation object, described in Table B-2. |
| shadowOpacity | Uses the default implied CABasicAnimation object, described in Table B-2. |
| shadowPath | Uses the default implied CABasicAnimation object, described in Table B-2. |
| shadowRadius | Uses the default implied CABasicAnimation object, described in Table B-2. |
| sublayers | Uses the default implied CABasicAnimation object, described in Table B-2. |
| sublayerTransform | Uses the default implied CABasicAnimation object, described in Table B-2. |
| transform | Uses the default implied CABasicAnimation object, described in Table B-2. |
| zPosition | Uses the default implied CABasicAnimation object, described in Table B-2. |


&emsp;表 B-2 列出了默认基于属性的动画的动画属性。

&emsp;Table B-2  Default Implied Basic Animation（默认隐式基本动画）

| Description | Value |
| --- | --- |
| Class | CABasicAnimation |
| Duration | 0.25 秒, 或当前 transaction 的持续时间 |
| Key path | 设置为 layer 的属性名称。 |

&emsp;表 B-3 列出了基于默认过渡（transition-based）的动画的动画对象配置。

&emsp;Table B-3  Default Implied Transition（默认隐式过渡）

| Description | Value |
| --- | --- |
| Class | CATransition |
| Duration | 0.25 秒, 或当前 transaction 的持续时间 |
| Type | Fade (kCATransitionFade) |
| Start progress | 0.0 |
| End progress | 1.0 |

### CIFilter Animatable Properties
&emsp;Core Animation 将以下可设置动画的属性添加到 Core Image 的 CIFilter 类中。这些属性仅在 OS X 上可用。

+ name
+ enabled

&emsp;有关这些添加的更多信息，请参见 CIFilter Core Animation Additions。

## Appendix C: Key-Value Coding Extensions（键值编码扩展）
&emsp;Core Animation 扩展了 NSKeyValueCoding 协议，因为它与 CAAnimation 和 CALayer 类有关。此扩展为某些 key 添加了默认值，扩展了包装约定，并为 CGPoint、CGRect、CGSize 和 CATransform3D 类型添加了 key path 支持。
### Key-Value Coding Compliant Container Classes（符合键值编码的容器类）
&emsp;CAAnimation 和 CALayer 类是符合键值编码的容器类，这意味着你可以为任意键设置值。即使键 someKey 不是 CALayer 类的声明属性，你仍然可以按以下方式为其设置值：
```c++
[theLayer setValue:[NSNumber numberWithInteger:50] forKey:@"someKey"];
```
&emsp;你还可以检索任意键的值，就像检索其他 key path 的值一样。例如，要检索先前设置的 someKey 路径的值，可以使用以下代码：
```c++
someKeyValue=[theLayer valueForKey:@"someKey"];
```
> &emsp;OS X Note: CAAnimation 和 CALayer 类 automatically archive 为这些类的实例设置的所有其他键，它们支持 NSCoding 协议。

### Default Value Support
&emsp;Core Animation 为键值编码添加了约定，从而类可以为没有设置值的 key 提供默认值。 CAAnimation 和 CALayer 类使用 `defaultValueForKey:` 类方法支持此约定。

&emsp;要为 key 提供默认值，请创建所需类的子类并重写其 `defaultValueForKey:` 方法。此方法的实现应该检查 key 参数并返回适当的默认值。清单 C-1 显示了一个 layer 对象的 `defaultValueForKey:` 方法的示例实现，该 layer 对象为 masksToBounds 属性提供默认值。

&emsp;Listing C-1  Example implementation of defaultValueForKey:
```c++
+ (id)defaultValueForKey:(NSString *)key {
    if ([key isEqualToString:@"masksToBounds"])
         return [NSNumber numberWithBool:YES];
 
    return [super defaultValueForKey:key];
}
```
### Wrapping Conventions
&emsp;当 key 的数据由标量值或 C 数据结构组成时，必须在将该类型指定给 layer 之前将其包装到对象中。类似地，在访问该类型时，必须检索一个对象，然后使用相应类的扩展来展开相应的值。表 C-1 列出了常用的 C 类型和用于包装它们的 Objective-C 类。

&emsp;Table C-1  Wrapper classes for C types

| C type | Wrapping class |
| --- | --- |
| CGPoint | NSValue |
| CGSize | NSValue |
| CGRect | NSValue |
| CATransform3D | NSValue |
| CGAffineTransform | NSAffineTransform(OS X only) |

## 参考链接
**参考链接:🔗**
+ [CALayer](https://developer.apple.com/documentation/quartzcore/calayer?language=objc)
+ [Core Animation Programming Guide](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/CoreAnimation_guide/Introduction/Introduction.html#//apple_ref/doc/uid/TP40004514-CH1-SW1)
