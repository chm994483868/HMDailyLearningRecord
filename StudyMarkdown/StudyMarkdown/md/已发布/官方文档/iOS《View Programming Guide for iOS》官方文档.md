# iOS《View Programming Guide for iOS》官方文档

## About Windows and Views（关于 Windows 和 Views）
&emsp;在 iOS 中，你可以使用 Windows 和 views 在屏幕上显示应用程序的内容。 Windows 本身没有任何可见的内容，但是它为应用程序的 views 提供了基本的容器。Views 定义要在 window 的哪一部分填充什么内容。例如，你可能具有显示 images、text、shapes 或其某种组合的 views。你还可以使用 views 来组织和管理其他 views。
### At a Glance
&emsp;每个应用程序至少都有一个 window 和一个 view 来呈现其内容。 UIKit 和其他系统框架提供了可用于呈现内容的预定义 views。这些 views 的范围从简单的按钮和文本标签到更复杂的视图，例如表视图（table views）、选择器视图（picker views）和滚动视图（scroll views）。在预定义 views 无法满足你需求的地方，你还可以定义 custom views 并自己管理绘图（drawing）和事件处理（event handling）。
#### Views Manage Your Application’s Visual Content（Views 管理你的应用程序的视觉内容）
&emsp;view 是 UIView 类（或其子类之一）的实例，并管理应用程序 window 中的矩形区域。Views 负责绘制内容、处理多点触控事件以及管理任何 subviews 的布局。绘图涉及使用诸如 Core Graphics、OpenGL ES 或 UIKit 之类的图形技术在 view 的矩形区域内绘制形状、图像和文本。view 通过使用手势识别器或直接处理触摸事件来响应其矩形区域中的触摸事件。在 view 层次结构中，父视图负责定位和调整其子视图的大小，并且可以动态地这样做。动态修改子视图的功能使你的 views 可以适应不断变化的条件，例如界面旋转和动画。

&emsp;你可以将 views 视为用于构建用户界面的构建块（building blocks，或者构建基础）。你通常不使用一个 view 来呈现所有内容，而是使用多个 views 来构建 view 层次结构。层次结构中的每个 view 都呈现用户界面的特定部分，并且通常针对特定类型的内容进行了优化。例如，UIKit 具有专门用于呈现图像、文本和其他类型的内容的视图。

> Relevant Chapters: View and Window Architecture, Views

#### Windows Coordinate the Display of Your Views（Windows 协调 Views 的显示）
&emsp;window 是 UIWindow 类的实例，用于处理应用程序用户界面的整体外观。 Windows 使用 views（和 views 自己的视图控制器（view controllers））来管理与可见 view 层次结构的交互以及对可见 view 层次结构的更改。在大多数情况下，你的应用程序 window 永远不会改变。创建 window 后，它保持不变，只有它显示的 views 改变。每个应用程序都有至少一个 window，在设备的主屏幕上显示该应用程序的用户界面。如果将外接显示器连接到设备，则应用程序可以创建第二个 window 以在该屏幕上显示内容。

> Relevant Chapters: Windows

#### Animations Provide the User with Visible Feedback for Interface Changes（动画为用户提供界面更改的可见反馈）
&emsp;动画为用户提供了有关 view 层次结构更改的可见反馈。系统定义了标准动画，用于呈现模式视图（modal views）并在不同 views 组之间进行转换（transitioning）。但是，view 的许多属性也可以直接设置动画。例如，通过动画，你可以更改 view 的透明度、它在屏幕上的位置、它的大小、它的背景颜色或其他属性。而且，如果你直接使用 view 的基础 Core Animation 图层对象（CALayer），则还可以执行许多其他动画。

> Relevant Chapters: Animations

#### The Role of Interface Builder（Interface Builder 的作用）
&emsp;Interface Builder 是一个应用程序，可用于以图形方式构造和配置应用程序的 windows 和 views。使用 Interface Builder，你可以组装 views 并将其放置在 nib 文件中，该文件是存储 views 和其他对象的 freeze-dried version 的资源文件。当你在运行时加载 nib 文件时，其中的对象将重新构建为实际对象，你的代码随后可以通过程序对其进行操作。

&emsp;Interface Builder 大大简化了创建应用程序的用户界面时需要做的工作。由于整个 iOS 都集成了对 Interface Builder 和 nib 文件的支持，因此只需很少的努力即可将 nib 文件整合到应用程序的设计中。

&emsp;有关如何使用 Interface Builder 的更多信息，请参见 Interface Builder User Guide。有关 view controllers 如何管理包含其 views 的 nib 文件的信息，请参见 View Controller Programming Guide for iOS 中的 Creating Custom Content View Controllers。

### See Also
&emsp;由于 views 是非常复杂和灵活的对象，因此不可能将所有行为都包含在一个文档中。但是，还有其他文档可帮助你了解管理 views 和整个用户界面的其他方面。
+ View controllers 是管理应用程序 views 的重要部分。view controller 管理单个 views 层次结构中的所有 views，并有助于在屏幕上呈现这些 views。有关 view controllers 及其角色的更多信息，请参见 View Controller Programming Guide for iOS。
+ Views 是应用程序中手势和触摸事件的主要接收者。有关使用手势识别器和直接处理触摸事件的更多信息，请参见 Event Handling Guide for iOS。
+ 自定义 views 必须使用可用的绘图技术来呈现其内容。有关使用这些技术在 views 中进行绘制的信息，请参见 Drawing and Printing Guide for iOS。
+ 在标准 view 动画不足的地方，可以使用 Core Animation。有关使用 Core Animation 实现动画的信息，请参见 Core Animation Programming Guide。

## View and Window Architecture（View 和 Window 架构）
&emsp;Views 和 windows 显示应用程序的用户界面，并处理与该界面的交互。 UIKit 和其他系统框架提供了许多 views，你几乎可以不加修改就可以使用它们。你还可以为需要不同于标准 views 显示内容的地方定义自定义 views。

&emsp;无论是使用系统 views 还是创建自己的自定义 views，都需要了解 UIView 和 UIWindow 类提供的基础结构。这些类为管理 views 的布局和表示提供了复杂的工具。了解这些工具是如何工作的对于确保在应用程序中发生更改时 views 的行为是非常重要的。

### View Architecture Fundamentals（View 架构基础）
&emsp;你可能希望在视觉上做的大多数事情都是使用 view 对象（UIView 类的实例）完成的。view 对象在屏幕上定义一个矩形区域，并处理该区域中的绘图和触摸事件。view 还可以充当其他 views 的父级，并协调这些 views 的位置和大小调整。 UIView 类完成了管理 views 之间的这些关系的大部分工作，但是你也可以根据需要自定义默认行为。

&emsp;Views 与 Core Animation layers 结合使用，以处理 view 内容的渲染和动画处理。 UIKit 中的每个 view 都由一个 layer 对象（通常是 CALayer 类的实例）支持，该对象管理该 view 的 backing store 并处理与 view 相关的动画。大多数操作都应该通过 UIView 接口执行。但是，在需要更好地控制 views 的渲染或动画行为的情况下，可以改为通过其 layer 执行操作。

&emsp;为了理解 views 和 layers 之间的关系，请看一个示例。图 1-1 显示了 ViewTransitions 示例应用程序的 view 体系结构以及与底层 Core Animation layers 的关系。应用程序中的 views 包括一个 window（也是一个 view）、一个充当容器 view 的通用 UIView 对象、一个图像 view、一个用于显示控件的工具栏和一个条形按钮项（它不是 view 本身，而是在内部管理 view）（UIBarButtonItem -> UIBarItem -> NSObject）。（实际的 ViewTransitions 示例应用程序包括用于实现 transitions 的附加图像 view。为了简单起见，并且由于该 view 通常是隐藏的，因此不包括在图 1-1 中。）每个 view 都有一个相应的 layer 对象，可以从该 view 的 layer 属性访问该对象。（由于条形按钮项不是 view，因此无法直接访问其 layer。）这些 layer 对象后面是 Core Animation 渲染对象，最终是用于管理屏幕上实际位的硬件缓冲区。

&emsp;Figure 1-1  Architecture of the views in a sample application

![view-layer-store](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/2f630851553947f8afc9f025be6dedef~tplv-k3u1fbpfcp-watermark.image)

&emsp;Core Animation layer 对象的使用对性能有重要影响。view 对象的实际绘图代码被尽可能少地调用，当调用代码时，结果被 Core Animation 缓存，并在以后尽可能多地重用。重用已经呈现的内容消除了更新 views 通常需要的昂贵的绘图周期。在可以操纵现有内容的动画中，重用此内容尤其重要。这种重用比创建新内容要节省的多。
#### View Hierarchies and Subview Management（View 层次结构和 Subview 管理）
&emsp;除了提供自己的内容外，view 还可以充当其他 views 的容器。当一个 view 包含另一个 view  时，将在两个 views 之间创建父子关系。关系中的 child view 称为 subview，parent  view 称为 superview。建立这种类型的关系对应用程序的外观和应用程序的行为都有影响。

&emsp;在视觉上，subview 的内容会掩盖其 superview 的全部或部分内容。如果 subview 是完全不透明的，则 subview 所占据的区域将完全遮挡父视图的相应区域。如果 subview 是部分透明的，则将两个 views 中的内容混合在一起，然后再显示在屏幕上。每个 superview 都将其 subviews 存储在一个有序数组中，并且该数组中的顺序也会影响每个 subview 的可见性。如果两个同级 subview 彼此重叠，则最后添加的（或已移至 subview 数组末尾的一个）出现在另一个 subview 的顶部。

&emsp;superview 与 subview 的关系也会影响几种 view 行为。更改 superview 的大小会产生连锁反应，这也会导致任何 subviews 的大小和位置也发生变化。更改 superview 的大小时，可以通过适当配置 view 来控制每个 subview 的调整大小的行为。影响 subview 的其他更改包括隐藏 superview、更改 superview 的 alpha（透明度）或对 superview 的坐标系进行数学变换。

&emsp;view 层次结构中 views 的排列也决定了应用程序如何响应事件。当触摸发生在特定 view 中时，系统会将包含触摸信息的事件对象直接发送到该 view 进行处理。但是，如果 view 不处理特定的触摸事件，它可以将事件对象传递给它的 superview。如果 superview 不处理事件，它会将事件对象传递给它的 superview，依此类推响应者链（and so on up the responder chain）。特定 view 还可以将事件对象传递给中间的响应者对象，例如视图控制器。如果没有对象处理事件，它最终会到达应用程序对象，而应用程序对象通常会丢弃它。

&emsp;有关如何创建视图层次结构的更多信息，请参见 Creating and Managing a View Hierarchy。

#### The View Drawing Cycle（View 绘图周期）
&emsp;UIView 类使用按需绘制模型（an on-demand drawing model）来呈现内容。当 view 首次出现在屏幕上时，系统会要求它绘制其内容。系统捕获该内容的 snapshot，并将该 snapshot 用作 view 的视觉表示。如果你从不更改 view 的内容，则可能永远不会再次调用 view 的绘图代码。snapshot 图像可用于涉及 view 的大多数操作。如果确实更改了内容，则会通知系统 view 已更改。然后，views 重复绘制 view 的过程并捕获绘制结果的 snapshot。

&emsp;当 view 的 contents 更改时，你不会直接重绘这些更改。而是使用 `- setNeedsDisplay` 或 `- setNeedsDisplayInRect:` 方法使 view 被标记为无效。这些方法告诉系统， view 的内容已更改，需要在下一个时机重新绘制。在启动任何重新绘制操作之前，系统将一直等到当前 run loop 结束。此延迟使你有机会一次使多个 view 无效，从层次结构中添加或删除 views、隐藏 views、调整 views 大小以及重新放置 views。你所做的所有更改都将同时在下一次绘制结果中反映出来。（即我们可以把一组对 views 的不同操作放在一起，然后在下次绘制时统一进行绘制，而不是说对 views 进行一步操作就绘制一次）

> Note: 更改 view 的几何形状不会自动导致系统重新绘制 view 的内容。view 的 contentMode 属性确定如何解释对 view 几何的更改。大多数 content modes 会在 view 范围内拉伸或重新定位现有 snapshot，而不创建新 snapshot。有关 content modes 如何影响 view 的绘制周期的更多信息，请参见 Content Modes。

&emsp;当需要渲染 view 内容时，实际的绘制过程会根据 view 及其配置而有所不同。系统 view 通常实现私有绘制方法来呈现其内容。这些相同的系统 views 经常公开可用于配置 view 实际外观的接口。对于自定义 UIView 子类，通常会重写 view 的 `- drawRect:` 方法，并使用该方法绘制 view 的内容。还有其他提供 view 内容的方法，例如直接设置 underlying layer 的 contents，但是重写 `- drawRect:` 方法是最常见的技术。

&emsp;有关如何为自定义 views 绘制内容的详细信息，请参见 Implementing Your Drawing Code。

#### Content Modes（内容模式）

&emsp;每个 view 都有一个 content mode，用于控制 view 如何响应 view geometry 中的更改而回收其内容，以及是否完全回收其内容。首次显示 view 时，它会照常渲染其内容，并将结果捕获在基础位图（underlying bitmap）中。之后，对 view 几何形状的更改并不总是导致重新创建位图。而是，contentMode 属性中的值确定是应缩放位图以适合新的 bounds 还是应将其简单固定到 view 的一个角或边缘。

&emsp;只要执行以下操作，便会应用视图的 content mode：（而不是进行重绘，那么会存在根据 content mode 进行调整位图并进行重绘吗？是存在的， 设置为 UIViewContentModeRedraw 模式的情况）

+ 更改 view frame 或 bounds 矩形的宽度或高度。
+ 将包含缩放因子（scaling factor）的变换（transform）指定给视图的 transform 属性。

&emsp;默认情况下，大多数 views 的 contentMode 属性设置为 UIViewContentModeScaleToFill，这将导致 view 内容被缩放以适应新的 frame 大小。图 1-2 显示了某些可用内容模式下发生的结果。从图中可以看出，并不是所有的内容模式都会导致 view bounds 被完全填充，而那些内容模式可能会扭曲 view 的内容。

&emsp;Figure 1-2  Content mode comparisons

![scale_aspect](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/baefe73213d042b1b56865c824855e3b~tplv-k3u1fbpfcp-watermark.image)

&emsp;内容模式有助于回收 view 的内容，但是当你特别希望 custom views 在缩放和调整大小操作期间重新绘制时，也可以将内容模式设置为 UIViewContentModeRedraw 值。将 view 的内容模式设置为该值将强制系统调用 view 的 `- drawRect:` 方法以响应几何体更改。一般来说，你应该尽可能避免使用这个值，当然也不应该在标准系统 views 中使用它。

&emsp;有关可用内容模式的详细信息，请参见 UIView Class Reference。

#### Stretchable Views（可伸缩的 Views）
&emsp;你可以将 view 的一部分指定为可拉伸的，这样，当 view 的大小更改时，仅可拉伸部分中的内容会受到影响。通常，将可拉伸区域用于按钮或其他 views，其中 view 的一部分定义了可重复的模式（repeatable pattern）。你指定的可拉伸区域可以允许沿 view 的一个或两个轴拉伸。当然，当沿两个轴拉伸 view 时，view 的边缘还必须定义可重复的模式，以避免任何变形。图 1-3 显示了这种变形如何在 view 中显现出来。每个 view 原始像素的颜色被复制以填充较大 view 中的相应区域。

&emsp;Figure 1-3  Stretching the background of a button

![button_scale](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/28aa264aafbc4ebfbd15f7e543571de8~tplv-k3u1fbpfcp-watermark.image)

&emsp;你可以使用 contentStretch 属性指定 view 的可拉伸区域。此属性接受一个矩形，其值规格化为 0.0 到 1.0 。拉伸 view 时，系统将这些归一化值乘以 view 的当前 bounds 和比例因子，以确定需要拉伸的像素。标准化值的使用减轻了你每次 view bounds 更改时都更新 contentStretch 属性的需求。

&emsp;view 的内容模式在确定如何使用 view 的可拉伸区域方面也发挥着作用。仅当内容模式会导致 view 内容缩放时，才使用可拉伸区域。这意味着只有 UIViewContentModeScaleToFill、UIViewContentModeScaleScaleAspectFit 和 UIViewContentModeScaleScaleAspectFill 内容模式才支持可拉伸 view。如果你指定一种将内容固定到边缘或角落的内容模式（因此实际上不会缩放内容），则 view 将忽略可拉伸区域。

> Note: 在为 view 指定背景时，建议在创建可拉伸的 UIImage 对象之前使用 contentStretch 属性。可拉伸 view 完全在 Core Animation layer 中处理，这通常提供更好的性能。

#### Built-In Animation Support（内置动画支持）
&emsp;在每个 view 后面都有一个 layer 对象的好处之一是，你可以轻松地对许多与 view 相关的更改进行动画处理。动画是一种向用户传达信息的有用方法，在设计应用程序时应始终考虑动画。 UIView 类的许多属性都是可设置动画的，也就是说，存在从一个值到另一个值进行动画制作的半自动支持。要为这些可设置动画的属性之一执行动画，你要做的就是：

1. 告诉 UIKit 你要执行动画。
2. 更改属性的值。

&emsp;你可以在 UIView 对象上设置动画的属性包括：
+ frame - 使用此动画为 view 设置位置和大小变化。（CALayer 类的 frame 不支持动画，可使用 bounds 和 position 属性来达到同样的效果）
+ bounds - 使用此动画可对 view 大小进行动画处理。
+ center - 使用它可以动画化 view 的位置。
+ transform - 使用它旋转或缩放 view。
+ alpha - 使用它可以更改 view 的透明度。
+ backgroundColor - 使用它可以更改 view 的背景色。
+ contentStretch - 使用它来更改 view 内容的拉伸方式。

&emsp;从一组 views 转换到另一组 views 时，动画非常重要。通常，使用 view controller 来管理与用户界面各部分之间的主要更改相关联的动画。例如，对于涉及从较高级别信息 navigating 到较低级别信息的界面，通常使用 navigation controller 来管理显示每个连续级别数据的 view 之间的转换。但是，也可以使用动画而不是 view controller 在两组 views 之间创建 transitions。在标准视图控制器动画（standard view-controller animations）不能产生所需结果的地方，可以这样做。

&emsp;除了使用 UIKit 类创建的动画外，还可以使用 Core Animation layers 创建动画。降到 layer 级别可以让你对动画的计时和属性有更多的控制。

&emsp;有关如何执行基于 view 的动画的详细信息，请参见 Animations。有关使用 Core Animation 创建动画的更多信息，请参见 Core Animation Programming Guide 和 Core Animation Cookbook。

### View Geometry and Coordinate Systems（查看几何和坐标系）
&emsp;UIKit 中的默认坐标系的原点在左上角，并且轴从原点向下延伸到右侧。坐标值使用浮点数表示，无论基础屏幕分辨率如何，都可以精确地布局和定位内容。图 1-4 显示了相对于屏幕的此坐标系。除了屏幕坐标系外，windows 和 views 还定义了自己的局部坐标系，使你可以指定相对于 view 或 window 原点而不是相对于屏幕的坐标。

&emsp;Figure 1-4  Coordinate system orientation in UIKit

![native_coordinate_system](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/49aaf614797946f9bc2e7466521a8ca2~tplv-k3u1fbpfcp-watermark.image)

&emsp;因为每个 view 和 window 都定义了自己的局部坐标系，所以你需要知道在任何给定时间有效的坐标系。每次绘制 view 或更改其几何形状时，都是相对于某个坐标系进行的。对于绘图，你可以指定相对于 view 自身坐标系的坐标。如果几何形状发生变化，则可以指定相对于 superview 坐标系的坐标。 UIWindow 和 UIView 类都包含帮助你从一个坐标系转换为另一个坐标系的方法。

> Important: 一些 iOS 技术定义的默认坐标系的原点和方向与 UIKit 使用的不同。例如，Core Graphics 和 OpenGL ES 使用一个坐标系，其原点位于 view 或 window 的左下角，其 y 轴相对于屏幕向上。在绘制或创建内容时，代码必须考虑这些差异，并根据需要调整坐标值（或坐标系的默认方向）。

#### The Relationship of the Frame, Bounds, and Center Properties（Frame、Bounds 和 Center 属性的关系）
&emsp;view 对象使用其 frame、bounds 和 center 属性跟踪其大小和位置：

+ frame 属性包含框 frame 矩形，用于指定 view 在其 superview 坐标系中的大小和位置。
+ bounds 属性包含 bounds 矩形，它指定 view 在自己的局部坐标系中的大小（及其内容原点）。
+ center 属性包含 superview 坐标系中 view 的已知中心点。

&emsp;你主要使用 center 和 frame 属性来操纵当前 view 的几何形状。例如，在构建 view 层次结构或在运行时更改 view 的位置或大小时，可以使用这些属性。如果仅更改 view 的位置（而不是其大小），则首选 center 属性。即使将缩放或旋转因子添加到 view 的变换中，center 属性中的值也始终有效。对于 frame 属性中的值，情况并非如此，如果 view 的 transform 不等于 identity transform，则该属性将被视为无效。

&emsp;你主要在绘制过程中使用 bounds 属性。bounds 矩形以 view 自身的局部坐标系表示。此矩形的默认原点为（0，0），其大小与 frame 矩形的大小匹配。你在此矩形内绘制的所有内容都是 view 可见内容的一部分。如果更改 bounds 矩形的原点，则在新矩形内绘制的所有内容都会成为 view 可见内容的一部分。

&emsp;图 1-5 显示了图像视图的 frame 和 bounds 矩形之间的关系。在该图中，图像 view 的左上角位于其 superview 坐标系中的点（40、40），矩形的大小为 240 x 380 点。对于 bounds 矩形，原点为（0，0），并且矩形的大小类似地为 240 x 380 点。

&emsp;Figure 1-5  Relationship between a view's frame and bounds

![frame_bounds_rects](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/3ccd1e9953d44905909b8c5cf40c973b~tplv-k3u1fbpfcp-watermark.image)

&emsp;尽管可以独立于其他属性更改 frame、bounds 和 center 属性，但是对一个属性的更改会通过以下方式影响其他属性：

+ 设置 frame 属性时，bounds 属性中的 size 值将更改，以匹配 frame 矩形的新大小。center 属性中的值也会发生类似的更改，以匹配 frame 矩形的新中心点。
+ 设置 center 属性时，frame 中的原点值会相应更改。
+ 设置 bounds 属性的大小时，frame 属性中的 size 值将更改，以匹配 bounds 矩形的新大小。

&emsp;默认情况下，view  的 frame 不会裁剪到其 superview 的 frame。因此，位于其 superview frame 之外的所有 subviews 均会完整呈现。不过，你可以通过将 superview 的 clipsToBounds 属性设置为 YES 来更改此行为。无论 subviews 是否在视觉上被剪切，触摸事件始终会遵循目标 view 的 superview 的 bounds 矩形。换言之，发生在 view 的某个部分（位于其 superview 的 bounds 矩形之外）中的触摸事件不会传递到该 view。

#### Coordinate System Transformations（坐标系转换）
&emsp;坐标系转换提供了一种快速，轻松地更改 view（或其 contents）的方法。仿射变换（affine transform）是一种数学矩阵，它指定一个坐标系中的点如何映射到另一个坐标系中的点。你可以将仿射变换应用于整个 view，以更改 view 相对于其 superview 的大小、位置或方向。你还可以在绘图代码中使用仿射变换来对单个渲染内容执行相同类型的操作。因此，如何应用仿射变换取决于上下文：

+ 要修改整个 view，请在 view 的 transform 属性中修改仿射变换。
+ 要修改 view 的 `- drawRect:` 方法中的特定内容片段，请修改与活动图形上下文关联的仿射变换。

&emsp;当你要实现动画时，通常可以修改 view 的 transform 属性。例如，你可以使用此属性来创建围绕其中心点旋转的 view 动画。你不会使用此属性对 view 进行永久更改，例如修改 view 的位置或在 view 的坐标空间内调整 view 的大小。对于这种类型的更改，你应该改为修改 view 的 frame 矩形。

> Note: 修改 view 的 transform 属性时，所有变换都相对于 view 的中心点执行。

&emsp;在 view 的 `- drawRect:` 方法中，使用仿射变换（affine transforms）来定位和定向要绘制的项。与其在 view 中的某个位置固定对象的位置，不如相对于固定点（通常为（0，0））创建每个对象，并在绘制之前使用 transform 来定位对象。这样，如果对象在 view 中的位置发生更改，则只需修改 transform 即可，这比在新位置重新创建对象要快得多，成本也要低得多。你可以使用 CGContextGetCTM 函数检索与图形上下文关联的仿射变换，并且可以使用相关的 Core Graphics 函数在绘图期间设置或修改此 transform。

&emsp;当前变换矩阵（transformation matrix）（CTM）是在任何给定时间使用的仿射变换。操纵整个 view  的几何图形时，CTM 是存储在 view  的 transform 属性中的仿射变换。在 `- drawRect:` 方法内部，CTM 是与活动图形上下文关联的仿射变换。

&emsp;每个 subview 的坐标系都基于其 superview 的坐标系。因此，当你修改 view 的 transform 属性时，该更改会影响该视图及其所有 subviews。但是，这些更改仅影响屏幕上 view 的最终呈现。由于每个 view 都绘制其内容并相对于其自己的 bounds 布置其 subviews，因此在绘制和布局期间，它可以忽略其 superview 的变换。

&emsp;图 1-6 演示了渲染时如何在视觉上结合两个不同的旋转因子。在视图的 `- drawRect:` 方法内部，对 shape 应用 45 度旋转因子会使该 shape 看起来旋转 45 度。对 view 应用单独的 45 度旋转因子，然后使 shape 看起来像旋转了 90 度。 shape 仍然相对于绘制它的 view 旋转了 45 度，但是 view 旋转使它看起来旋转了更多。

&emsp;Figure 1-6  Rotating a view and its content

![xform_rotations](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/6985a037f33d43989327b451d38abb45~tplv-k3u1fbpfcp-watermark.image)

> Important: 如果 view 的 transform 属性不是 identity 转换，则该 view 的 frame 属性的值未定义，必须忽略。将 transforms 应用于 view 时，必须使用 view 的 bounds 和 center 属性来获取 view 的大小和位置。任何子视图的 frame 矩形仍然有效，因为它们是相对于视图的 bounds。

&emsp;有关在运行时修改视图的 transform 属性的信息，请参见 Translating、Scaling and Rotating Views。有关如何在绘图过程中使用 transforms 来定位内容的信息，请参见 Drawing and Printing Guide for iOS。

#### Points Versus Pixels（Points 与像素）

&emsp;在 iOS 中，所有坐标值和距离都是使用浮点值指定的，这些浮点值的单位称为点（points）。一个点的可测量大小因设备而异，基本上不相关。了解点的主要内容是：它们为绘图提供了一个固定的参照系。

&emsp;表 1-1 列出了纵向不同类型的基于 iOS 的设备的屏幕尺寸（以磅为单位）。首先列出宽度尺寸，然后是屏幕的高度尺寸。只要你针对这些屏幕尺寸设计界面，你的 views 就会在相应类型的设备上正确显示。

&emsp;Table 1-1  Screen dimensions for iOS-based devices

| Device | Screen dimensions(in points) |
| --- | --- |
| iPhone and iPod touch devices with 4-inch Retina display | 320 x 568 |
| Other iPhone and iPod touch devices | 320 x 480 |
| iPad | 768 x 1024 |

&emsp;用于每种类型的设备的基于点的测量系统定义了所谓的用户坐标空间。这是几乎所有代码都使用的标准坐标空间。例如，在操纵 view 的几何体或调用 Core Graphics 函数绘制 view 的内容时，可以使用点和用户坐标空间。尽管有时用户坐标空间中的坐标会直接映射到设备屏幕上的像素，但你永远不要以为是这种情况。相反，你应该始终记住以下几点：

**一个点不一定对应于屏幕上的一个像素。（One point does not necessarily correspond to one pixel on the screen.）**

&emsp;在设备级别，view 中指定的所有坐标必须在某个点转换为像素。但是，用户坐标空间中的点到设备坐标空间中的像素的映射通常由系统处理。 UIKit 和 Core Graphics 都使用主要基于矢量（vector-based）的绘图模型，其中所有坐标值都是使用点指定的。因此，如果使用 Core Graphics 绘制曲线，则无论基础屏幕的分辨率如何，都可以使用相同的值指定曲线。

&emsp;当你需要使用图像或其他基于像素的技术（如 OpenGL ES）时，iOS 提供了管理这些像素的帮助。对于作为资源存储在应用程序包中的静态图像文件，iOS 定义了用于指定不同像素密度的图像以及加载与当前屏幕分辨率最匹配的图像的约定。Views 还提供有关当前比例因子的信息，以便可以手动调整任何基于像素的图形代码以适应更高分辨率的屏幕。有关在不同屏幕分辨率下处理基于像素的内容的技术，请参见 Supporting High-Resolution Screens In Views in Drawing and Printing Guide for iOS。

### The Runtime Interaction Model for Views（Views 的运行时交互模型）
&emsp;每当用户与你的用户界面进行交互，或者你自己的代码以编程方式更改某些内容时，UIKit 内都会发生一系列复杂的事件来处理该交互。在该序列中的特定时间点，UIKit 调出你的 view 类，并为其提供机会代表你的应用程序进行响应。了解这些标注点对于了解 views 在系统中的位置非常重要。图 1-7 显示了事件的基本顺序，该顺序从用户触摸屏幕开始，到图形系统作为响应更新屏幕内容，然后结束。对于任何以编程方式启动的操作，也将发生相同的事件序列。

&emsp;Figure 1-7  UIKit interactions with your view objects

![drawing_model](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/800911ec192b464c845cc288131c575f~tplv-k3u1fbpfcp-watermark.image)

&emsp;以下步骤进一步分解了图 1-7 中的事件序列，并解释了每个阶段发生的情况以及你可能希望应用程序如何响应。

1. 用户触摸屏幕。
2. 硬件将触摸事件报告给 UIKit 框架。
3. UIKit 框架将触摸打包到一个 UIEvent 对象中并将其分派到适当的 view。 （有关 UIKit 如何将事件传递到 view 的详细说明，请参见 Event Handling Guide for iOS。）
4. view 的事件处理代码响应事件。例如，你的代码可能：

  + 更改 view 或其 subviews 的属性（frame、bounds、alpha 等）。
  + 调用 `- setNeedsLayout` 方法将 view（或其 subviews）标记为需要布局更新。
  + 调用 `- setNeedsDisplay` 或 `- setNeedsDisplayInRect:` 方法将 view（或其 subviews）标记为需要重新绘制。
  + 通知 controller 某些数据的更改。
  当然，由你决定 view 应该做哪些事情以及应该调用哪些方法。
  
5. 如果 view 的几何图形因任何原因发生更改，UIKit 将根据以下规则更新其 subviews： 
  + 如果已经为 views 配置了自动调整大小规则（autoresizing rules），UIKit 会根据这些规则调整每个 view。有关自动调整大小规则如何工作的详细信息，请参见 Handling Layout Changes Automatically Using Autoresizing Rules。
  + 如果视图实现了 `- layoutSubviews` 方法，UIKit 将调用它。
    你可以在 custom views 中重写此方法，并使用它来调整任何 subviews 的位置和大小。例如，提供较大可滚动区域的 view 将需要使用多个 subviews 作为 "tiles"，而不是创建一个大 view，该大 view 无论如何都不太适合内存。在这个方法的实现中，view 将隐藏现在在屏幕外的任何 subviews，或者重新定位它们，并使用它们来绘制新公开的内容。作为此过程的一部分，view 的布局代码还可以使任何需要重新绘制的 view 无效。
    
6. 如果任何 view 的任何部分被标记为需要重绘，UIKit 会要求 view 重绘它自己。
  对于明确定义了 `- drawRect:` 方法的 custom views，UIKit 会调用该方法。此方法的实现应尽快重绘 view 的指定区域，而别无其他。此时，请勿进行其他布局更改，也不要对应用程序的数据模型进行其他更改。此方法的目的是更新 view 的视觉内容。
  标准系统 view 通常不实现 `- drawRect:` 方法，而是在此时管理其绘制。
  
7. 任何更新的 views 都将与应用程序的其余可见内容混合在一起，并发送到图形硬件进行显示。 
8. 图形硬件将渲染的内容传输到屏幕。

> Note: 先前的更新模型主要适用于使用标准系统 view 和绘图技术的应用程序。使用 OpenGL ES 进行绘制的应用程序通常会配置一个全屏 view 并直接绘制到关联的 OpenGL ES 图形上下文。在这种情况下，该 view 仍可以处理触摸事件，但是由于它是全屏的，因此无需布置 subviews。有关使用 OpenGL ES 的更多信息，请参见 OpenGL ES Programming Guide。

&emsp;在前面的步骤中，你自己的 custom views 的主要集成点是：

+ 事件处理方法：
  `touchesBegan:withEvent:`
  `touchesMoved:withEvent:`
  `touchesEnded:withEvent:`
  `touchesCancelled:withEvent:`

+ `layoutSubviews` 方法
+ `drawRect:` 方法

&emsp;这些是 views 最常用的重写方法，但你可能不需要重写所有这些方法。如果使用手势识别器来处理事件，则不需要重写任何事件处理方法。类似地，如果 view 不包含 subviews 或其大小没有更改，则没有理由重写 `layoutSubviews` 方法。最后，只有当 view 的内容在运行时可以更改，并且你正在使用 UIKit 或 Core Graphics 等 native 技术进行绘制时，才需要 `drawRect:` 方法。

&emsp;同样重要的是要记住，这些是主要的集成点，但不是唯一的集成点。UIView 类的几个方法被设计成子类的重写点。你应该查看  UIView Class Reference 中的方法描述，以查看哪些方法可能适合你在自定义实现中重写。

### Tips for Using Views Effectively（有效使用 Views 的提示）
&emsp;对于需要绘制标准系统 view 未提供的内容的情况，custom views 很有用，但是你有责任确保 views 的性能足够好。 UIKit 尽其所能来优化与 view 相关的行为，并帮助你在 custom views 中获得良好的性能。但是，你可以通过考虑以下提示在这方面帮助 UIKit。

> Important: 优化绘图代码之前，应始终收集有关 view 当前性能的数据。测量当前性能可以让你确认是否确实存在问题，如果存在，还可以为你提供基准测量结果，你可以将其与未来的优化进行比较。

#### Views Do Not Always Have a Corresponding View Controller（视图并不总是具有对应的视图控制器）
&emsp;应用程序中的各个 views 和 view controllers 之间很少存在一对一的关系。view controller 的工作是管理 view 层次结构，该 view 层次结构通常由多个 view 组成，这些 view 用于实现某些自包含功能。对于 iPhone 应用程序，每个 view 层次结构通常填充整个屏幕，尽管对于 iPad 应用程序，视图层次结构可能仅填充屏幕的一部分。

&emsp;在设计应用程序的用户界面时，必须考虑 view controllers 将扮演的角色。view controllers 提供了许多重要的行为，例如协调 views 在屏幕上的显示、协调从屏幕上删除这些 views、释放内存以响应内存不足警告以及旋转 views 以响应界面方向的更改。规避这些行为可能会导致应用程序行为不正确或出现意外情况。

&emsp;有关更多信息，请参见 view controllers 及其在应用程序中的角色，请参见 View Controller Programming Guide for iOS。

#### Minimize Custom Drawing（最小化自定义绘图）
&emsp;虽然自定义绘图有时是必要的，但也应尽可能避免。只有当现有的系统 view 类不提供所需的外观或功能时，才应该真正执行任何 custom drawing。每当你的内容可以与现有 view 的组合进行组合时，你最好将这些 view 对象组合到自定义 view 层次结构中。

#### Take Advantage of Content Modes（利用内容模式）
&emsp;内容模式可以最大限度地减少重新绘制 view 所花费的时间。默认情况下，view 使用 UIViewContentModeScaleToFill 内容模式，该模式缩放 view 的现有内容以适合 view 的 frame 矩形。你可以根据需要更改此模式以不同方式调整内容，但如果可以，应避免使用 UIViewContentModeRedraw 内容模式。无论使用哪种内容模式，都可以通过调用 `- setNeedsDisplay` 或 `- setNeedsDisplayInRect:`强制 view 重绘其内容。

#### Declare Views as Opaque Whenever Possible（尽可能将视图声明为不透明）
&emsp;UIKit 使用每个 view 的 opaque 属性来确定该 view 是否可以优化合成操作。对于 custom view，将此属性的值设置为 YES 可以告诉 UIKit，它不需要在 view 背后呈现任何内容。较少的渲染可以提高绘图代码的性能，因此通常会鼓励这样做。当然，如果将 opaque 属性设置为 YES，则 view 必须使用完全不透明的内容完全填充其 bounds 矩形。（如尽量使用 alpha 为 NO 的图片）

#### Adjust Your View’s Drawing Behavior When Scrolling（滚动时调整视图的绘图行为）
&emsp;滚动可以在短时间内产生大量 view 更新。如果 view 的绘图代码没有得到适当的调整，view 的滚动性能可能会很差。与其试图确保 view 的内容始终是原始的，不如考虑在滚动操作开始时更改 view 的行为。例如，可以临时降低呈现内容的质量，或者在滚动过程中更改内容模式。当滚动停止时，你可以将 view 返回到以前的状态，并根据需要更新内容。

#### Do Not Customize Controls by Embedding Subviews（不要通过嵌入子视图来自定义控件）
&emsp;尽管从技术上来说可以将 subviews 添加到标准系统控件（从 UIControl 继承的对象）中，但是你永远不应以这种方式对其进行自定义。支持自定义的控件通过控件类本身中的显式且文档齐全的接口来实现。例如，UIButton 类包含用于设置按钮标题和背景图像的方法。使用定义的定制点意味着你的代码将始终正常工作。通过在按钮内嵌入自定义图像视图或标签来规避这些方法，可能会导致你的应用程序现在或将来某个按钮的实现发生变化时，表现不正确。

## Windows
&emsp;每个 iOS 应用程序都至少需要一个 window（UIWindow 类的一个实例），有些可能包含多个 window。window 对象具有以下职责：

+ 它包含你应用程序的可见内容。
+ 它在将触摸事件传递给 view 和其他应用程序对象中扮演着关键角色。
+ 它与你的应用程序的 view controllers 一起使用，以方便方向更改。

&emsp;在 iOS 中，windows 没有 title bars、close boxes 或任何其他视觉装饰。window 始终只是一个或多个 view 的空白容器。此外，应用程序不会通过显示新 windows 来更改其内容。当你想要更改显示的内容时，可以更改 window 的最前面的 view。

&emsp;大多数 iOS 应用程序在其生命周期内仅创建和使用一个 window。该 window 跨越设备的整个主屏幕，并在应用程序生命周期的早期从应用程序的主 nib 文件加载（或通过程序创建）。但是，如果应用程序支持使用外部显示器进行视频输出，则它可以创建一个附加 window 以在该外部显示器上显示内容。所有其他 windows 通常由系统创建，并且通常是响应于特定事件（例如来电）而创建的。

### Tasks That Involve Windows（Windows 涉及的任务）
&emsp;对于许多应用程序，应用程序与其 window 交互的唯一时间是在启动时创建 window。但是，你可以使用应用程序的 window 对象执行一些与应用程序相关的任务：
+ **使用 window 对象将点和矩形与 window 的本地坐标系相互转换。** 例如，如果在 window 坐标中提供了一个值，则在尝试使用它之前，可能需要将其转换为特定 view 的坐标系。有关如何转换坐标的信息，请参见 Converting Coordinates in the View Hierarchy。
+ **使用 window 通知来跟踪与 window 相关的更改。** Windows 在显示或隐藏它们或接受或放弃 key 状态时会生成通知。你可以使用这些通知在应用程序的其他部分中执行操作。有关更多信息，请参见 Monitoring Window Changes。

### Creating and Configuring a Window（创建和配置窗口）
&emsp;你可以通过编程或使用 Interface Builder 创建和配置应用程序的 main window。在任何一种情况下，你都应该在启动时创建 window，并且应该保留它，并在应用程序委托对象中存储对它的引用。如果应用程序创建了其他 windows，请让应用程序在需要时延迟创建它们。例如，如果你的应用程序支持在外部显示器上显示内容，则应等到显示器连接后，再创建相应的 window。

&emsp;无论应用程序是在前台还是后台启动，都应该在启动时创建应用程序的 main window。创建和配置 window 本身并不昂贵。但是，如果应用程序直接在后台启动，则应避免在应用程序进入前台之前使 window 可见。

#### Creating Windows in Interface Builder（在 Interface Builder 中创建 Windows）
&emsp;使用 Interface Builder 创建应用程序的 main window 很简单，因为 Xcode 项目模板可以帮你完成。每个新的 Xcode 应用程序项目都包含一个主 nib 文件（通常使用 MainWindow.xib 或其他名称），其中包含该应用程序的 main window。此外，这些模板还在应用程序委托对象中为该 window 定义了 outlet。你可以使用此 outlet 来访问代码中的 window 对象。

> Important: 在 Interface Builder 中创建 window 时，建议启用 attributes inspector 中的 Full Screen at Launch 选项。如果未启用此选项，并且 window 小于目标设备的屏幕，则某些 view 将不会接收触摸事件。这是因为 window（和所有 view 一样）不接收超出其 bounds 的触摸事件。由于 view 在默认情况下不会剪裁到 window 的 bounds，因此 view 仍然显示为可见，但事件不会到达它们。（这里类似当 subviews 的 bounds 超出其 superview 的 bounds 时不会主动对其裁剪，但是触摸超出 superview 的 bounds 之外的区域时也不会响应此事件。）启用 Full Screen at Launch 选项可确保 window 大小适合当前屏幕。

&emsp;如果要改造项目以使用 Interface Builder，则使用 Interface Builder 创建 window 是将 UIWindow 对象拖到 nib 文件中的简单问题。当然，你还应该执行以下操作：

+ 要在运行时访问 window，你应该 connect the window to an outlet，outlet 通常是在应用程序委托或 nib 文件的文件所有者（File’s Owne）中定义的。
+ 如果改造计划包括使新的 nib 文件成为应用程序的 main nib 文件，则还必须将应用程序的 Info.plist 文件中的 NSMainNibFile 键设置为 nib 文件的名称。 更改此键的值可确保在调用应用程序委托的 `- application:didFinishLaunchingWithOptions:` 方法之前，已加载 nib 文件并可供使用。

&emsp;有关创建和配置 nib 文件的详细信息，请参见 Interface Builder User Guide。有关如何在运行时将 nib 文件加载到应用程序中的信息，请参见  Resource Programming Guide 中的 Nib Files。

#### Creating a Window Programmatically（以编程方式创建窗口）
&emsp;如果你希望以编程方式创建应用程序的 main window，则应在 `- application:didFinishLaunchingWithOptions:` 应用程序委托的方法中包含类似于以下代码：
```c++
self.window = [[[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]] autorelease];
```
&emsp;在前面的例子中，self.window 假定为应用程序委托的声明属性，该属性配置为保留窗口对象（strong 修饰）。如果要为外部显示器创建 window，则将其分配给其他变量，并且需要指定表示该显示器的非主 UIScreen 对象的 bounds。

&emsp;创建 windows 时，应始终将 window 大小设置为屏幕的最大范围。不应缩小 window 的大小以容纳状态栏或任何其他项。不管怎样，状态栏总是浮在 window 的顶部，所以只有将 view 放到 window 中，才能缩小状态栏。如果你使用的是 view controllers，那么视图控制器应该自动处理 view 的大小调整。

#### Adding Content to Your Window（向窗口添加内容）
&emsp;每个 window 通常都有一个根视图对象（root view object）（由相应的 view controller 管理），该对象包含代表你的内容的所有其他 views。使用单个 root view 简化了更改界面的过程。要显示新内容，你要做的就是替换 root view。要在 window 中安装 view，请使用 `- addSubview:`方法。例如，要安装由 view controller 管理的 view，你将使用类似于以下代码：
```c++
[window addSubview:viewController.view];
```
&emsp;除了上述代码，你还可以在 nib 文件中配置 window 的 rootViewController 属性。该属性提供了一种使用 nib 文件而非编程方式配置 window root view 的便捷方法。如果从其 nib 文件加载 window 时设置了此属性，则 UIKit 会自动从关联的 view controller 安装 view 作为 window 的 root view。此属性仅用于安装 root view，window 不使用它与 view controller 进行通信。

&emsp;你可以将任何所需的 view 用作 window 的 root view。根据你的界面设计，root view 可以是充当一个或多个 subviews 的容器的通用 UIView 对象，root view 可以是标准系统 view，或者 root view 可以是你定义的 custom view。通常用作 root view 的一些标准系统 views 包括 scroll views、table views 和 image views。

&emsp;在配置 window 的 root view 时，你需要设置其初始大小和在 window 中的位置。对于不包含状态栏或显示半透明状态栏的应用程序，请设置 view 大小以匹配 window 的大小。对于显示不透明状态栏的应用程序，将 view 置于状态栏下方，并相应地减小其大小。从 view 的高度减去状态栏的高度可防止遮挡 view 的顶部。

> Note: 如果 window 的 root view 由容器视图控制器（例如 tab bar controller、navigation controller 或 split-view controller）提供，则无需自己设置 view 的初始大小。容器视图控制器会根据状态栏是否可见来自动调整其 view 的大小。

#### Changing the Window Level（更改 Window 级别）
&emsp;每个 UIWindow 对象都有一个可配置的 windowLevel 属性，该属性确定该 window 相对于其他 windows 的放置方式。在大多数情况下，你无需更改应用程序 windows 的级别。在创建时，新 windows 会自动分配给 normal 窗口级别。normal 窗口级别指示该 window 显示与应用程序相关的内容。较高的 window 级别保留用于需要浮动在应用程序内容上方的信息，例如系统状态栏或警报消息。而且，尽管你可以自己将 window 分配给这些级别，但是当你使用特定的界面时，系统通常会为你执行此操作。例如，当你显示或隐藏状态栏或显示警报 view 时，系统会自动创建所需的 window 以显示这些项目。

### Monitoring Window Changes（监视 Window 更改）
&emsp;如果要跟踪应用程序内部 window 的出现或消失，可以使用以下与 window 相关的通知来进行跟踪：

+ UIWindowDidBecomeVisibleNotification
+ UIWindowDidBecomeHiddenNotification
+ UIWindowDidBecomeKeyNotification
+ UIWindowDidResignKeyNotification

&emsp;这些通知是根据你应用程序 window 中的程序更改而传递的。因此，当你的应用程序显示或隐藏 window 时，将相应地传递 UIWindowDidBecomeVisibleNotification 和 UIWindowDidBecomeHiddenNotification 通知。当你的应用程序进入后台执行状态时，不会发送这些通知。即使你的应用程序在后台时 window 未显示在屏幕上，它仍被认为在你的应用程序上下文中可见。

&emsp;UIWindowDidBecomeKeyNotification 和 UIWindowDidResignKeyNotification 通知可帮助你的应用程序跟踪哪个 window 是 key window，即哪个 window 当前正在接收键盘事件和其他与触摸无关的事件。触摸事件传递到发生触摸的 window，而没有关联坐标值的事件传递到应用程序的 key window。一次仅一个 window 可能是 key window。

### Displaying Content on an External Display（在外接显示器上显示内容）
&emsp;要在外部显示器上显示内容，必须为你的应用程序创建一个附加 window，并将其与代表外部显示器的屏幕对象相关联。默认情况下，新 window 通常与主屏幕关联。更改 window 的关联屏幕对象会导致该 window 的内容重新路由到相应的显示器。window 与正确的屏幕相关联后，你可以向其添加 view 并像在应用程序的主屏幕上一样显示它。

&emsp;UIScreen 类维护代表可用硬件显示的屏幕对象列表。通常，对于任何基于 iOS 的设备，只有一个屏幕对象代表主显示屏，但是支持连接到外部显示器的设备可以具有一个附加的屏幕对象。支持外部显示器的设备包括具有 Retina 显示器的 iPhone 和 iPod touch 设备以及 iPad。较旧的设备（例如 iPhone 3GS）不支持外部显示器。

> Note: 因为外部显示器本质上是视频输出连接，所以不应期望与外部显示器相关联的 window 中的 views 和控件发生触摸事件。此外，你的应用程序有责任根据需要更新 window 的内容。因此，要镜像 main window 的内容，你的应用程序将需要为外部显示器的 window 创建一组重复的视图，并与 main window 中的 views 一起进行更新。

&emsp;以下介绍了在外部显示器上显示内容的过程。但是，以下步骤总结了基本过程：

1. 在应用程序启动时，注册屏幕连接和断开连接通知。
2. 当需要在外部显示器上显示内容时，创建并配置一个 window。
  + 使用 UIScreen 的 screens 属性来获取外部显示器的屏幕对象。
  + 创建一个 UIWindow 对象，并根据屏幕（或你的内容）调整其大小。
  + 将用于外部显示的 UIScreen 对象分配给 window 的 screen 属性。
  + 根据需要调整屏幕对象的分辨率以支持你的内容。
  + 在 window 中添加任何适当的 views。
3. 显示 window 并正常更新。

#### Handling Screen Connection and Disconnection Notifications（处理屏幕连接和断开连接通知）
&emsp;屏幕连接和断开连接通知对于妥善处理外部显示器的更改至关重要。当用户连接或断开显示器连接时，系统会向你的应用程序发送适当的通知。你应该使用这些通知来更新应用程序状态并创建或释放与外部显示器关联的 window。

&emsp;关于连接和断开连接通知，要记住的重要一点是，即使你的应用程序在后台挂起，它们也可以随时出现。因此，最好观察在应用程序运行时持续存在的来自对象的通知，例如你的应用程序委托。如果你的应用程序已暂停，则通知将排队，直到你的应用程序退出暂停状态并开始在前台或后台运行。

&emsp;清单 2-1 显示了用于注册连接和断开连接通知的代码。应用程序委托在初始化时调用此方法，但是你也可以从应用程序中其他位置注册这些通知。清单 2-2 显示了处理程序方法的实现。

&emsp;Listing 2-1  Registering for screen connect and disconnect notifications
```c++
- (void)setupScreenConnectionNotificationHandlers {
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
 
    [center addObserver:self selector:@selector(handleScreenConnectNotification:) name:UIScreenDidConnectNotification object:nil];
    [center addObserver:self selector:@selector(handleScreenDisconnectNotification:) name:UIScreenDidDisconnectNotification object:nil];
}
```

&emsp;如果将外部显示器连接到设备后你的应用程序处于活动状态，则应为该显示器创建第二个 window，并在其中填充一些内容。内容不必是你要呈现的最终内容。例如，如果你的应用程序尚未准备好使用额外的屏幕，则可以使用第二个 window 显示一些占位符内容。如果你没有为屏幕创建 window，或者创建但未显示 window，则外部显示屏上会显示黑场。

&emsp;清单 2-2 显示了如何创建辅助 window 并将其填充一些内容。在此示例中，应用程序在用于接收屏幕连接通知的处理程序方法中创建 window。 （有关注册连接和断开通知的信息，请参见清单 2-1。）用于连接通知的处理程序方法将创建一个辅助 window，将其与新连接的屏幕相关联，并调用应用程序主视图控制器的方法以添加一些内容。内容并显示在 window 中。断开连接通知的处理程序方法释放 window 并通知主视图控制器，以便它可以相应地调整其表示形式。

&emsp;Listing 2-2  Handling connect and disconnect notifications
```c++
- (void)handleScreenConnectNotification:(NSNotification*)aNotification {
    UIScreen* newScreen = [aNotification object];
    CGRect screenBounds = newScreen.bounds;
 
    if (!_secondWindow) {
        _secondWindow = [[UIWindow alloc] initWithFrame:screenBounds];
        _secondWindow.screen = newScreen;
 
        // Set the initial UI for the window.
        [viewController displaySelectionInSecondaryWindow:_secondWindow];
    }
}
 
- (void)handleScreenDisconnectNotification:(NSNotification*)aNotification {
    if (_secondWindow) {
        // Hide and then delete the window.
        _secondWindow.hidden = YES;
        [_secondWindow release];
        _secondWindow = nil;
 
        // Update the main screen based on what is showing here.
        [viewController displaySelectionOnMainScreen];
    }
}
```

#### Configuring a Window for an External Display（为外部显示器配置窗口）
&emsp;要在外部屏幕上显示 window，必须将其与正确的屏幕对象相关联。这个过程包括定位适当的 UIScreen 对象并将其分配给窗口的 screen 属性。你可以从 UIScreen 的 screens 类方法获得 screen 对象的列表。此方法返回的数组始终至少包含一个表示主屏幕的对象。如果存在第二个对象，则该对象表示连接的外部显示器。

&emsp;清单 2-3 显示了一个在应用程序启动时调用的方法，以查看是否已经附加了外部显示器。如果是，该方法将创建一个 window，将其与外部显示器相关联，并在显示 window 之前添加一些占位符内容。在本例中，占位符内容是白色背景和表示没有要显示的内容的标签。若要显示 window，此方法将更改其隐藏属性的值，而不是调用 makeKeyAndVisible。它这样做是因为 window 只包含静态内容，不用于处理事件。

&emsp;Listing 2-3  Configuring a window for an external display
```c++
- (void)checkForExistingScreenAndInitializeIfPresent {
    if ([[UIScreen screens] count] > 1) {
        // Associate the window with the second screen.
        // The main screen is always at index 0.
        UIScreen* secondScreen = [[UIScreen screens] objectAtIndex:1];
        CGRect screenBounds = secondScreen.bounds;
 
        _secondWindow = [[UIWindow alloc] initWithFrame:screenBounds];
        _secondWindow.screen = secondScreen;
 
        // Add a white background to the window
        UIView* whiteField = [[UIView alloc] initWithFrame:screenBounds];
        whiteField.backgroundColor = [UIColor whiteColor];
 
        [_secondWindow addSubview:whiteField];
        [whiteField release];
 
        // Center a label in the view.
        NSString* noContentString = [NSString stringWithFormat:@"<no content>"];
        CGSize stringSize = [noContentString sizeWithFont:[UIFont systemFontOfSize:18]];
 
        CGRect labelSize = CGRectMake((screenBounds.size.width - stringSize.width) / 2.0,
                                    (screenBounds.size.height - stringSize.height) / 2.0,
                                    stringSize.width, stringSize.height);
 
        UILabel* noContentLabel = [[UILabel alloc] initWithFrame:labelSize];
        noContentLabel.text = noContentString;
        noContentLabel.font = [UIFont systemFontOfSize:18];
        [whiteField addSubview:noContentLabel];
 
        // Go ahead and show the window.
        _secondWindow.hidden = NO;
    }
}
```

> Important: 在显示 window 之前，应始终将屏幕与 window 关联。尽管可以更改当前可见 window 的屏幕，但这是一项昂贵的操作，应避免。

&emsp;一旦显示外部屏幕的 window，应用程序就可以像其他 window 一样开始更新它。你可以根据需要添加和删除 subviews，更改 subviews 的内容，设置 view 更改的动画，并根据需要使其内容无效。

#### Configuring the Screen Mode of an External Display（配置外接显示器的屏幕模式）
&emsp;根据你的内容，你可能需要在将 window 与其关联之前更改屏幕模式。许多屏幕支持多种分辨率，其中一些分辨率使用不同的像素长宽比。屏幕对象默认情况下使用最常见的屏幕模式，但是你可以将该模式更改为更适合你的内容的模式。例如，如果你要使用 OpenGL ES 来实现游戏，并且纹理是为 640 x 480 像素屏幕设计的，则可以更改具有更高默认分辨率的屏幕的屏幕模式。

&emsp;如果计划使用默认模式以外的其他屏幕模式，则应在将屏幕与 window 关联之前，将该模式应用于 UIScreen 对象。 UIScreenMode 类定义单个屏幕模式的属性。你可以从屏幕的 availableModes 属性获取屏幕支持的模式的列表，并在列表中进行迭代以找到符合你需求的模式。

&emsp;有关屏幕模式的更多信息，请参见 UIScreenMode Class Reference。

## Animations
&emsp;动画在用户界面的不同状态之间提供流畅的视觉过渡。在 iOS 中，动画广泛用于重新定位 view、更改其大小、从 view 层次结构中删除 view 以及隐藏 view。你可以使用动画向用户传达反馈或实现有趣的视觉效果。

&emsp;在 iOS 中，创建复杂的动画不需要你编写任何绘图代码。本章中描述的所有动画技术都使用 Core Animation 提供的内置支持。你要做的就是触发动画并让 Core Animation 处理单个帧的渲染。这使得仅需几行代码即可轻松创建复杂的动画。

### What Can Be Animated?
&emsp;UIKit 和 Core Animation 都提供对动画的支持，但是每种技术提供的支持级别不同。在 UIKit 中，使用 UIView 对象执行动画。Views 支持包含许多常见任务的基本动画集。例如，可以设置 views 属性更改的动画，或使用 transition 动画将一组 views 替换为另一组 views。

&emsp;表 4-1 列出了 UIView 类的可设置动画的属性（具有内置动画支持的属性）。具有动画效果并不意味着动画会自动发生。更改这些属性的值通常只需要立即更新属性（和 view），而无需动画。要对此类更改进行动画处理，必须从动画块内部更改属性的值，这在 Animating Property Changes in a View 中进行介绍。

&emsp;Table 4-1  Animatable UIView properties

| Property | Changes you can make |
| --- | --- |
| frame | 修改此属性，以更改 view 相对于其 superview 坐标系的大小和位置。 （如果 transform 属性不包含 identity 变换，请改为修改 bounds 或 center 属性。） |
| bounds | 修改此属性以更改 view 的大小。 |
| center | 修改此属性以更改 view 相对于其 superview 坐标系的位置。 |
| transform | 修改此属性以相对于其中心点缩放、旋转或平移 view。使用此属性的变换始终在 2D 空间中执行。 （要执行 3D 转换，你必须使用 Core Animation 为 view 的 layer 对象设置动画。） |
| alpha | 修改此属性以逐渐更改 view 的透明度。 |
| backgroundColor | 修改此属性以更改 view 的背景色。 |
| contentStretch | 修改此属性可更改 view 内容被拉伸以填充可用空间的方式。 |

&emsp;Animated view transitions 是你对 view 层次进行更改的一种方法，可以超越 view controller 提供的那些方法。尽管你应该使用 view controllers 来管理简洁的 view 层次结构，但是有时你可能希望替换全部或部分 view 层次结构。在这种情况下，你可以使用 view-based 的 transitions 来为 view 的添加和删除添加动画效果。

&emsp;在你想要执行更复杂的动画或 UIView 类不支持的动画的地方，可以使用 Core Animation 和视图的基础 layer 来创建动画。由于 view 和 layer 对象错综复杂地链接在一起，因此对 view layer 的更改会影响 view 本身。使用 Core Animation，可以为 view 的 layer 设置以下类型的动画：

+ layer 的大小和位置
+ 执行 transformations 时使用的 center 点
+ Transformations 为 3D 空间中的 layer 或其 sublayers
+ 在 layer 层次结构中添加或删除 layer 
+ layer 相对于其他同级图层（sibling layers）的 Z 顺序（Z-order ）
+ layer 的阴影（shadow）
+ layer 的 border（包括 layer 的 corners 是否圆角）
+ 在调整大小操作期间拉伸的 layer 部分
+ layer 的不透明度（opacity）
+ 超出 layer bounds 的 sublayers 的裁剪行为
+ layer 的当前内容（contents）
+ layer 的栅格化行为（rasterization behavior）

> Note: 如果 view 承载自定义 layer 对象（即没有关联 view 的 layer 对象），则必须使用 Core Animation 来设置对其所做任何更改的动画。

&emsp;尽管本章介绍了一些 Core Animation 行为，但它是从 view  代码启动它们相关的。有关如何使用 Core Animation 为 layers 设置动画的详细信息，请参见 Core Animation Programming Guide 和 Core Animation Cookbook。

### Animating Property Changes in a View（在 View 中动画化属性更改）
&emsp;为了对 UIView 类的属性更改设置动画，必须将这些更改包装在动画块中。术语 animation block 在一般意义上用于指代指定可设置动画的更改的任何代码。在 iOS4 及更高版本中，使用 block object 创建动画块。在早期版本的 iOS 中，可以使用 UIView 类的特殊类方法标记动画块的开始和结束。（即在两个类方法调用之间包裹动画代码）这两种技术支持相同的配置选项，并对动画执行提供相同的控制量。然而，只要可能，就首选基于块的方法。

&emsp;以下各节重点介绍为 view 属性的更改设置动画所需的代码。有关如何在多组 views 之间创建 animated transitions 的信息，请参见 Creating Animated Transitions Between Views。

#### Starting Animations Using the Block-Based Methods（使用基于块的方法开始动画）
&emsp;在 iOS 4 及更高版本中，使用基于 block 的类方法来启动动画。有几种基于 block 的方法可以为 animation block 提供不同级别的配置。这些方法是：

+ `+ animateWithDuration:animations:`
+ `+ animateWithDuration:animations:completion:`
+ `+ animateWithDuration:delay:options:animations:completion:`

&emsp;因为这些是类方法，所以使用它们创建的 animation block  不会绑定到单个 view。因此，可以使用这些方法创建一个包含对多个 views 的更改的动画。例如，清单 4-1 显示了在一个 view 中淡入而在一秒钟内淡出另一个 view 所需的代码。当此代码执行时，指定的动画会立即在另一个线程上启动，以避免阻塞当前线程或应用程序的主线程。

&emsp;Listing 4-1  Performing a simple block-based animation
```c++
[UIView animateWithDuration:1.0 animations:^{
        firstView.alpha = 0.0;
        secondView.alpha = 1.0;
}];
```
&emsp;上一个示例中的动画仅使用缓入、缓出动画曲线运行一次。如果要更改默认动画参数，必须使用 `+ animateWithDuration:delay:options:animations:completion:` 执行动画。使用此方法可以自定义以下动画参数：

+ 开始动画之前的延迟时间
+ 动画期间要使用的时序曲线的类型
+ 动画应重复的次数
+ 动画到达终点时是否应自动反转自身
+ 动画进行期间是否将触摸事件传递给 views
+ 动画是应该中断正在进行的动画，还是等到完成后再开始播放

&emsp;另一件事是 `+ animateWithDuration:animations:completion:` 和 `+ animateWithDuration:delay:options:animations:completion:` 支持指定完成处理程序块的功能。你可以使用完成处理程序向应用程序发出特定动画已完成的信号。完成处理程序也是将单独的动画链接在一起的方法。

&emsp;清单 4-2 展示了一个 animation block 的示例，它使用完成处理程序在第一个动画完成后启动一个新的动画。第一次调用 `+ animateWithDuration:delay:options:animations:completion:` 设置淡出动画并使用一些自定义选项对其进行配置。当动画完成时，它的完成处理程序运行并设置动画的后半部分，该后半部分在延迟后淡入视图。

&emsp;使用完成处理程序是链接多个动画的主要方式。

&emsp;Listing 4-2  Creating an animation block with custom options
```c++
- (IBAction)showHideView:(id)sender {
    // Fade out the view right away
    [UIView animateWithDuration:1.0
        delay: 0.0
        options: UIViewAnimationOptionCurveEaseIn
        animations:^{
        
             thirdView.alpha = 0.0;
             
        }
        completion:^(BOOL finished){
            // Wait one second and then fade in the view
            [UIView animateWithDuration:1.0
                 delay: 1.0
                 options:UIViewAnimationOptionCurveEaseOut
                 animations:^{
                 
                    thirdView.alpha = 1.0;
                    
                 }
                 completion:nil];
        }];
}
```

> Important：在涉及某个属性的动画正在进行时更改该属性的值不会停止当前动画。取而代之的是，当前动画将继续并设置为刚指定给属性的新值的动画。

#### Starting Animations Using the Begin/Commit Methods（使用 Begin/Commit 方法开始动画）
&emsp;如果你的应用程序在 iOS 3.2 及更早版本中运行，则必须使用 `+ beginAnimations:context:` 和 UIView 的 `+ commitAnimations` 类方法来定义 animation blocks。这些方法标记 animation blocks 的开始和结束。在调用 `commitAnimations` 方法后，在这些方法之间更改的任何可设置动画的属性都将设置为其新值的动画。动画在另一个线程上执行，以避免阻塞当前线程或应用程序的主线程。

> Note: 如果你正在为 iOS 4 或更高版本编写应用程序，则应该使用 block-based 的方法来设置内容的动画。有关如何使用这些方法的信息，请参见 Starting Animations Using the Block-Based Methods。

&emsp;清单 4-3 显示了实现与清单 4-1 相同的行为所需的代码，但是使用了 `begin/commit` 方法。如清单 4-1 所示，此代码淡出一个视图，而淡出另一个视图的时间超过 1 秒。但是，在本例中，必须使用单独的方法调用设置动画的持续时间。

&emsp;Listing 4-3  Performing a simple begin/commit animation
```c++
   [UIView beginAnimations:@"ToggleViews" context:nil];
   [UIView setAnimationDuration:1.0];

   // Make the animatable changes.
   firstView.alpha = 0.0;
   secondView.alpha = 1.0;

   // Commit the changes and perform the animation.
   [UIView commitAnimations];
```
&emsp;默认情况下，animation block 中所有可设置动画的属性更改都将设置动画。如果要为某些更改设置动画，但不为其他更改设置动画，请使用 `setAnimationsEnabled:` 方法暂时禁用动画，进行任何不希望设置动画的更改，然后再次调用 `setAnimationsEnabled:` 以重新启用动画。你可以通过调用 `areAnimationsEnabled` 类方法来确定动画是在当前是启用的。

> Note: 在涉及某个属性的动画正在进行时更改该属性的值不会停止当前动画。相反，动画将继续并以刚指定给属性的新值设置动画。

##### Configuring the Parameters for Begin/Commit Animations（配置 Begin/Commit 动画的参数）
&emsp;要为 begin/commit animation block 配置动画参数，可以使用几种 UIView 类方法中的任意一种。表 4-2 列出了这些方法，并描述了如何使用它们来配置动画。这些方法中的大多数应该只从 begin/commit animation block 内部调用，但是有些方法也可以用于 block-based 的动画。如果不从 animation block 调用这些方法之一，则使用相应属性的默认值。有关与每个方法关联的默认值的详细信息，请参见 UIView Class Reference。

&emsp;Table 4-2  Methods for configuring animation blocks
| Method | Usage |
| --- | --- |
| setAnimationStartDate: setAnimationDelay: | 使用这些方法中的任何一个来指定执行应该在何时开始执行。如果指定的开始日期在过去（或延迟为 0），动画将尽快开始。 |
| setAnimationDuration: | 使用此方法可以设置执行动画的时间。 |
| setAnimationCurve: | 使用此方法可以设置动画的时间曲线。这控制动画是线性执行还是在特定时间更改速度。 |
| setAnimationRepeatCount: setAnimationRepeatAutoreverses: | 使用这些方法可以设置动画重复的次数，以及动画在每个完整循环结束时是否反向运行。有关使用这些方法的更多信息，请参见 Implementing Animations That Reverse Themselves。 |
| setAnimationDelegate: setAnimationWillStartSelector: setAnimationDidStopSelector: | 使用这些方法可以在动画之前或之后立即执行代码。有关使用 delegate 的更多信息，请参见 Configuring an Animation Delegate。|
| setAnimationBeginsFromCurrentState: | 使用此方法可以立即停止所有先前的动画，并从停止点开始新的动画。如果将 NO 传递给此方法，而不是 YES，则新动画直到之前的动画停止后才开始执行。 |

&emsp;清单 4-4 显示了实现与清单 4-2 中的代码相同的行为所需的代码，但是使用 `begin/commit` 方法。和前面一样，此代码淡出视图，等待一秒钟，然后淡入。为了实现动画的第二部分，代码设置了一个动画委托并实现了一个 did-stop 处理程序方法。然后，该处理程序方法设置动画的后半部分并运行它们。

&emsp;Listing 4-4  Configuring animation parameters using the begin/commit methods
```c++
// This method begins the first animation.
- (IBAction)showHideView:(id)sender {
    [UIView beginAnimations:@"ShowHideView" context:nil];
    [UIView setAnimationCurve:UIViewAnimationCurveEaseIn];
    [UIView setAnimationDuration:1.0];
    [UIView setAnimationDelegate:self];
    [UIView setAnimationDidStopSelector:@selector(showHideDidStop:finished:context:)];
 
    // Make the animatable changes.
    thirdView.alpha = 0.0;
 
    // Commit the changes and perform the animation.
    [UIView commitAnimations];
}
 
// Called at the end of the preceding animation.
- (void)showHideDidStop:(NSString *)animationID finished:(NSNumber *)finished context:(void *)context {
    [UIView beginAnimations:@"ShowHideView2" context:nil];
    [UIView setAnimationCurve:UIViewAnimationCurveEaseOut];
    [UIView setAnimationDuration:1.0];
    [UIView setAnimationDelay:1.0];
 
    thirdView.alpha = 1.0;
 
    [UIView commitAnimations];
}
```
##### Configuring an Animation Delegate（配置动画委托）
&emsp;如果要在动画之前或之后立即执行代码，则必须将代理对象和开始或停止选择器与 `begin/commit` 动画块相关联。使用 UIView 的 `+ setAnimationDelegate:` 类方法设置委托对象，并使用 `+ setAnimationWillStartSelector:` 和 `+ setAnimationDidStopSelector:` 类方法设置开始和停止选择器。在动画过程中，动画系统会在适当的时间调用委托方法，以使你有机会执行代码。

&emsp;动画委托方法的签名必须类似于以下内容：
```c++
- (void)animationWillStart:(NSString *)animationID context:(void *)context;
- (void)animationDidStop:(NSString *)animationID finished:(NSNumber *)finished context:(void *)context;
```
&emsp;两个方法的 animationID 和 context 参数与传递给 `+ beginAnimations:context:` 动画块开头的方法：

+ animationID - 应用程序提供的字符串，用于标识动画。
+ context - 应用程序提供的对象，可用于将其他信息传递给 delegate。

&emsp;`setAnimationDidStopSelector:` selector 方法有一个额外的参数，如果动画运行到完成状态，这个布尔值就是 YES。如果此参数的值为 NO，则动画已被另一个动画取消或提前停止。

> Note: 尽管可以在 block-based 的方法中使用动画代理，但通常不需要在那里使用它们。相反，将要运行的任何代码放在块开头的动画之前，并将要在动画完成后运行的任何代码放在完成处理程序中。

#### Nesting Animation Blocks（嵌套动画块）
&emsp;通过嵌套其他 animation blocks，可以为 animation blocks 的各个部分指定不同的计时和配置选项。顾名思义，嵌套 animation blocks 是在现有 animation blocks 中创建的新 animation blocks。嵌套动画与任何父动画同时启动，但使用它们自己的配置选项运行（大多数情况下）。默认情况下，嵌套动画确实会继承父级的持续时间和动画曲线，但甚至可以根据需要覆盖这些选项。

&emsp;清单 4-5 展示了如何使用嵌套动画来更改整个组中某些动画的计时、持续时间和行为的示例。在这种情况下，两个 views 将淡入完全透明状态，但另一个 view 对象的透明度在最终隐藏之前会来回更改几次。嵌套动画块中使用的 UIViewAnimationOptionOverrideInheritedCurve 和 UIViewAnimationOptionOverrideInheritedDuration keys 允许为第二个动画修改第一个动画中的曲线和持续时间值。如果这些 keys 不存在，则将使用外部 animation block 的持续时间和曲线。

&emsp;Listing 4-5  Nesting animations that have different configurations
```c++
[UIView animateWithDuration:1.0
       delay: 1.0
       options:UIViewAnimationOptionCurveEaseOut
       animations:^{
       
           aView.alpha = 0.0;

           // Create a nested animation that has a different
           // duration, timing curve, and configuration.
           [UIView animateWithDuration:0.2
                delay:0.0
                options: UIViewAnimationOptionOverrideInheritedCurve |
                         UIViewAnimationOptionCurveLinear |
                         UIViewAnimationOptionOverrideInheritedDuration |
                         UIViewAnimationOptionRepeat |
                         UIViewAnimationOptionAutoreverse
                animations:^{
                
                     [UIView setAnimationRepeatCount:2.5];
                     anotherView.alpha = 0.0;
                     
                }
                completion:nil];

       }
       completion:nil];
```
&emsp;如果使用 begin/commit 方法创建动画，嵌套的工作方式与基于块的方法基本相同。每次连续调用 `+ beginAnimations:context:` 在已打开的 animation block 中创建新的嵌套 animation block，你可以根据需要进行配置。所做的任何配置更改都将应用于最近打开的 animation block。在提交和执行动画之前，必须通过调用 `commitAnimations` 关闭所有 animation blocks。

#### Implementing Animations That Reverse Themselves（实现颠倒自己的动画）
&emsp;在结合 repeat count 创建可逆动画时，请考虑为 repeat count 指定非整数值。对于自动反转动画，动画的每个完整循环都涉及从原始值到新值再到新值的动画制作。如果希望动画以新值结束，则将 0.5 添加到 repeat count 会导致动画完成以新值结束所需的额外半个周期。如果不包括这半步，动画将设置为原始值，然后快速捕捉到新值，这可能不是你想要的视觉效果。

### Creating Animated Transitions Between Views（在视图之间创建动画过渡）
&emsp;View transitions 有助于隐藏与在 view 层次结构中添加、删除、隐藏或显示 view 相关的突然更改。使用 view transitions 可以实现以下类型的更改：

+ **更改现有 view 的可见 subviews。** 如果要对现有 view 进行相对较小的更改，通常会选择此选项。
+ **将 view 层次结构中的一个 view 替换为其他 view。** 通常，当你要替换跨越整个或大部分屏幕的 view 层次结构时，会选择此选项。

> Important: View transitions 不应与 view controllers 启动的 transitions 相混淆，例如 modal 模式视图控制器（presentation of modal view controllers）或将新视图控制器推到导航堆栈上（pushing of new view controllers onto a navigation stack）。View transitions 仅影响 view 层次，而 view-controller transitions 也会更改 active view controller。因此，对于 view transitions，启动 transition 时处于活动状态的 view controller 在 transition 完成时保持 active 状态。
> 有关如何使用 view controllers 来呈现新内容的更多信息，请参见 View Controller Programming Guide for iOS。

#### Changing the Subviews of a View（更改视图的子视图）
&emsp;通过更改 view 的 subviews，可以对 view 进行适度更改。例如，可以添加或删除 subviews 以在两种不同的状态之间切换 superview。动画完成时，将显示相同的 view，但其内容现在不同。

&emsp;在 iOS 4 及更高版本中，使用 `+ transitionWithView:duration:options:animations:completion:` 为 view 启动 transition animation 的方法。在传递给此方法的 animations block 中，通常设置动画的唯一更改是与显示、隐藏、添加或删除 subviews 相关联的更改。将动画限制为该集允许视图创建视图的前后版本的 snapshot 图像，并在两个图像之间设置动画，这样效率更高。但是，如果需要设置其他更改的动画，则可以在调用该方法时包括 UIViewAnimationOptionAllowAnimatedContent 选项。包括该选项可防止 view 创建 snapshot 并直接设置所有更改的动画。

&emsp;清单 4-6 是一个如何使用 transition animation 使其看起来好像添加了新的文本输入页的示例。在本例中，main view 包含两个嵌入的 text views。文本视图的配置相同，但其中一个始终可见，而另一个始终隐藏。当用户点击按钮创建一个新页面时，此方法切换两个视图的可见性，从而生成一个新的空页面，其中一个空文本视图准备接受文本。转换完成后，视图使用私有方法保存旧页中的文本，并重置现在隐藏的文本视图，以便以后可以重用。然后，视图排列其指针，以便在用户请求另一个新页面时，它可以准备执行相同的操作。

&emsp;Listing 4-6  Swapping an empty text view for an existing one
```c++
- (IBAction)displayNewPage:(id)sender {
    [UIView transitionWithView:self.view
        duration:1.0
        options:UIViewAnimationOptionTransitionCurlUp
        animations:^{
        
            currentTextView.hidden = YES;
            swapTextView.hidden = NO;
            
        }
        completion:^(BOOL finished){
            // Save the old text and then swap the views.
            [self saveNotes:temp];
 
            UIView* temp = currentTextView;
            currentTextView = swapTextView;
            swapTextView = temp;
            
        }];
}
```
&emsp;如果需要在 iOS 3.2 及更早版本中执行 view transitions，可以使用 `setAnimationTransition:forView:cache:` 方法指定 transitions 的参数。传递给该方法的 view 与你作为第一个参数传递给该 view 的视图相同 `transitionWithView:duration:options:animations:completion:`。清单 4-7 显示了需要创建的 animation block 的基本结构。请注意，要实现清单 4-6 中所示的完成块，需要使用 did-stop 处理程序配置动画代理，如 Configuring an Animation Delegate 中所述。

&emsp;Listing 4-7  Changing subviews using the begin/commit methods
```c++
[UIView beginAnimations:@"ToggleSiblings" context:nil];
[UIView setAnimationTransition:UIViewAnimationTransitionCurlUp forView:self.view cache:YES];
[UIView setAnimationDuration:1.0];

// Make your changes

[UIView commitAnimations];
```
#### Replacing a View with a Different View（用其他视图替换视图）
&emsp;当你希望你的界面有很大的不同时，可以替换 view。因为这种技术只交换 views（而不是 view controllers），所以你需要负责适当地设计应用程序的 controller 对象。这种技术只是使用一些 standard transitions 快速呈现新 views 的一种方法。

&emsp;在 iOS 4 及更高版本中，使用 `transitionFromView:toView:duration:options:completion:` 在两个 views 之间转换。此方法实际上从层次结构中删除第一个 view 并插入另一个 view，因此如果要保留第一个 view，应该确保有对它的引用。如果要隐藏 view 而不是将其从 view 层次结构中删除，请将 UIViewAnimationOptionShowHideTransitionViews 作为选项之一传递。

&emsp;清单 4-8 显示了在由单个视图控制器管理的两个主视图之间交换所需的代码。在本例中，视图控制器的根视图始终显示两个子视图（primaryView 或 secondaryView）中的一个。每个视图呈现相同的内容，但方式不同。视图控制器使用 displayingPrimary 成员变量（布尔值）跟踪在任何给定时间显示的视图。翻转方向根据显示的视图而改变。

&emsp;Listing 4-8  Toggling between two views in a view controller
```c++
- (IBAction)toggleMainViews:(id)sender {
    [UIView transitionFromView:(displayingPrimary ? primaryView : secondaryView)
        toView:(displayingPrimary ? secondaryView : primaryView)
        duration:1.0
        options:(displayingPrimary ? UIViewAnimationOptionTransitionFlipFromRight :
                    UIViewAnimationOptionTransitionFlipFromLeft)
        completion:^(BOOL finished) {
            if (finished) {
                displayingPrimary = !displayingPrimary;
            }
    }];
}
```
> Note: 除了交换 views 外，view controller 代码还需要管理主视图和次视图的加载和卸载。有关视图控制器如何加载和卸载视图的信息，请参见 View Controller Programming Guide for iOS。

### Linking Multiple Animations Together（将多个动画链接在一起）
&emsp;UIView 动画接口支持链接单独的 animation blocks，以便它们按顺序而不是同时执行。链接 animation blocks 的过程取决于使用的是 block-based 的动画方法还是 begin/commit 方法：

+ 对于 block-based 的动画，请使用 `animateWithDuration:animations:completion:` 和 `animateWithDuration:delay:options:animations:completion:` 方法支持的完成处理程序来执行任何后续动画。
+ 对于 begin/commit 动画，请将代理对象和 did-stop 选择器与动画相关联。有关如何将代理与动画关联的信息，请参见 Configuring an Animation Delegate。

&emsp;将动画链接在一起的另一种方法是使用具有不同延迟因子的嵌套动画，以便在不同时间启动动画。有关如何嵌套动画的详细信息，请参见 Nesting Animation Blocks.。

### Animating View and Layer Changes Together（一起对视图和图层更改进行动画处理）
&emsp;应用程序可以根据需要自由混合 view-based 和 layer-based 的动画代码，但配置动画参数的过程取决于 layer 的所有者。更改 view 拥有的 layer 与更改 view 本身是相同的，并且应用于 layer 属性的任何动画都会考虑当前 view-based 的 animation block 的动画参数。对于你自己创建的 layer，情况并非如此。自定义 layer 对象忽略 view-based 的 animation block 参数，而使用默认的 Core Animation 参数。

&emsp;如果要自定义所创建 layer 的动画参数，则必须直接使用 Core Animation。通常，使用 Core Animation 设置动画，涉及创建 CABasicAnimation 对象或 CAAnimation 的其他一些具体子类。然后，将该动画添加到相应的 layer。你可以在 view-based 的 animation block 的内部或外部应用动画。

&emsp;清单 4-9 显示了一个同时修改 view 和自定义 layer 的动画。本例中的 view 在其 bounds 的中心包含一个自定义 CALayer 对象。动画逆时针旋转 view，同时顺时针旋转 layer。因为旋转方向相反，所以 layer 保持其相对于屏幕的原始方向，并且看起来没有明显的旋转。但是，该 layer 下面的 view 会旋转 360 度并返回到其原始方向。本示例主要演示如何混合 view 和 layer 动画。这种类型的混合不应用于需要精确定时的情况。

&emsp;Listing 4-9  Mixing view and layer animations
```c++
[UIView animateWithDuration:1.0
    delay:0.0
    options: UIViewAnimationOptionCurveLinear
    animations:^{
        // Animate the first half of the view rotation.
        CGAffineTransform  xform = CGAffineTransformMakeRotation(DEGREES_TO_RADIANS(-180));
        backingView.transform = xform;
 
        // Rotate the embedded CALayer in the opposite direction.
        CABasicAnimation*    layerAnimation = [CABasicAnimation animationWithKeyPath:@"transform"];
        layerAnimation.duration = 2.0;
        layerAnimation.beginTime = 0; //CACurrentMediaTime() + 1;
        layerAnimation.valueFunction = [CAValueFunction functionWithName:kCAValueFunctionRotateZ];
        layerAnimation.timingFunction = [CAMediaTimingFunction
                        functionWithName:kCAMediaTimingFunctionLinear];
        layerAnimation.fromValue = [NSNumber numberWithFloat:0.0];
        layerAnimation.toValue = [NSNumber numberWithFloat:DEGREES_TO_RADIANS(360.0)];
        layerAnimation.byValue = [NSNumber numberWithFloat:DEGREES_TO_RADIANS(180.0)];
        [manLayer addAnimation:layerAnimation forKey:@"layerAnimation"];
    }
    completion:^(BOOL finished){
        // Now do the second half of the view rotation.
        [UIView animateWithDuration:1.0
             delay: 0.0
             options: UIViewAnimationOptionCurveLinear
             animations:^{
                 CGAffineTransform  xform = CGAffineTransformMakeRotation(DEGREES_TO_RADIANS(-359));
                 backingView.transform = xform;
             }
             completion:^(BOOL finished){
                 backingView.transform = CGAffineTransformIdentity;
         }];
}];
```

> Note: 在清单 4-9 中，你还可以在基于视图的动画块之外创建并应用 CABasicAnimation 对象，以获得相同的结果。所有动画最终都依赖于 Core Animation 来执行。因此，如果它们几乎同时提交，它们将一起运行。

&emsp;如果需要在 view-based 的动画和 layer-based 的动画之间进行精确计时，则建议您使用 Core Animation 创建所有动画。你可能会发现，无论如何，使用 Core Animation 更容易执行某些动画。例如，清单 4-9 中的 view-based 的旋转需要多步序列才能旋转 180 度以上，而 Core Animation 部分使用的旋转值函数从开始到结束一直旋转到中间值。

&emsp;有关如何使用 Core Animation 创建和配置动画的详细信息，请参见 Core Animation Programming Guide 和 Core Animation Cookbook。


## Views
&emsp;因为 view 对象是应用程序与用户交互的主要方式，所以它们有许多职责。以下是一些：

+ 布局和 subview 管理
  + view 相对于其 superview 定义了自己的默认大小调整行为。
  + view 可以管理 subviews 列表。
  + view 可以根据需要覆盖其 subviews 的大小和位置。
  + view 可以将其坐标系中的点转换为其他 views 或 window 的坐标系。
+ 绘图和动画
  + view 在其矩形区域中绘制内容。
  + 某些 view 属性可以设置为新值时附加动画。
+ 事件处理
  + view 可以接收触摸事件。
  + view 参与 responder chain。

&emsp;本章重点介绍创建、管理和绘制 views 以及处理 view 层次结构的布局和管理的步骤。有关如何在 views 中处理触摸事件（和其他事件）的信息，请参见 Event Handling Guide for iOS。

### Creating and Configuring View Objects（创建和配置视图对象）
&emsp;以编程方式或使用 Interface Builder 将 views 创建为自包含（self-contained）对象，然后将它们组装到 view 层次结构中以供使用。
#### Creating View Objects Using Interface Builder（使用 Interface Builder 创建视图对象）
&emsp;创建 view 的最简单方法是使用 Interface Builder 以图形方式组装 views。从 Interface Builder，你可以向界面添加 views，将这些 views 排列成层次结构，配置每个 view 的设置，并将 view 相关行为连接到代码。因为 Interface Builder 使用实时 view 对象，也就是说，在设计时看到的 view 类的实际实例就是在运行时得到的。然后将这些活动对象保存在 nib 文件中，该文件是一个资源文件，用于保存对象的状态和配置。

&emsp;通常创建 nib 文件是为了存储应用程序的某个 view controller 的整个 view 层次结构。nib 文件的顶层通常包含一个表示视图控制器的视图对象。（视图控制器本身通常由文件的所有者对象表示。）顶级 view 的大小应适合目标设备，并包含要显示的所有其他 views。很少使用 nib 文件只存储视图控制器视图层次结构的一部分。

&emsp;将 nib 文件与 view controller 一起使用时，只需使用 nib 文件信息初始化 view controller。view controller 在适当的时间处理 view 的加载和卸载。但是，如果 nib 文件未与 view controller 关联，则可以使用 NSBundle 或 UINib 对象手动加载 nib 文件内容，后者使用 nib 文件中的数据重新构建 view 对象。

&emsp;有关如何使用 Interface Builder 创建和配置 view 的详细信息，请参见 nterface Builder User Guide。有关 view controller 如何加载和管理其关联的 nib 文件的信息，请参见 View Controller Programming Guide for iOS 中的 Creating Custom Content View Controllers 。有关如何从 nib 文件以编程方式加载 view 的详细信息，请参见 Resource Programming Guide 中的 Nib Files。

#### Creating View Objects Programmatically（以编程方式创建视图对象）
&emsp;如果希望以编程方式创建 views，可以使用标准的 allocation/initialization pattern。view 的默认初始化方法是 `initWithFrame:` 方法，它设置 view 相对于其父视图（即将建立）的初始大小和位置。例如，要创建新的通用 UIView 对象，可以使用类似以下代码：
```c++
CGRect  viewRect = CGRectMake(0, 0, 100, 100);
UIView* myView = [[UIView alloc] initWithFrame:viewRect];
```
> Note: 尽管所有 views 都支持 `initWithFrame:` 方法，但有些 view 可能有你应该使用的首选初始化方法。有关任何自定义初始化方法的信息，请参见该类的参考文档。

&emsp;创建 view 后，必须将其添加到 window（或 window 中的另一个 view）中，才能使其可见。有关如何将 view 添加到 view 层次结构的信息，请参见 Adding and Removing Subviews。

#### Setting the Properties of a View（设置视图的属性）
&emsp;UIView 类有几个声明的属性，用于控制 view 的外观和行为。这些属性用于操纵 view 的大小和位置、view 的透明度、背景色及其渲染行为。所有这些属性都有适当的默认值，你可以稍后根据需要进行更改。你还可以使用 Inspector 窗口从 Interface Builder 中配置许多这些属性。

&emsp;表 3-1 列出了一些更常用的属性（和一些方法），并描述了它们的用法。相关属性列在一起，以便你可以看到影响 view 某些方面的选项。

&emsp;Table 3-1  Usage of some key view properties
&emsp;......

### Modifying Views at Runtime（在运行时修改视图）
&emsp;当应用程序接收到来自用户的输入时，它们会根据该输入调整其用户界面。应用程序可以通过重新排列视图、更改视图的大小或位置、隐藏或显示视图或加载一组全新的视图来修改视图。在 iOS 应用程序中，有几个地方和方法可以执行这些类型的操作：
+ 在视图控制器中：
  + 视图控制器必须在显示视图之前创建视图。它可以从 nib 文件加载视图，也可以通过编程方式创建视图。当不再需要这些视图时，它就会处理掉它们。
  + 当设备改变方向时，视图控制器可能会调整视图的大小和位置以匹配。作为调整新方向的一部分，它可能会隐藏一些视图并显示其他视图。
  + 当视图控制器管理可编辑内容时，在切换到编辑模式或从编辑模式切换时，它可能会调整其视图层次结构。例如，它可能会添加额外的按钮和其他控件，以便于编辑其内容的各个方面。这可能还需要调整任何现有视图的大小以容纳额外的控件。
+ 在动画块中：
  + 如果要在用户界面中的不同视图集之间转换，可以隐藏一些视图，并从动画块内部显示其他视图。
  + 实现特殊效果时，可以使用动画块修改视图的各种特性。例如，要对视图大小的更改设置动画，可以更改其 frame 矩形的大小。
+ 其他方式：
  + 当发生触摸事件或手势时，你的界面可能会通过加载新的视图集或更改当前视图集来做出响应。有关处理事件的信息，请参见 iOS 事件处理指南。
  + 当用户与滚动视图交互时，一个大的可滚动区域可能会隐藏和显示平铺子视图。有关支持可滚动内容的更多信息，请参阅 Scroll View Programming Guide For iOS。
  + 当键盘出现时，你可以重新定位或调整视图的大小，使它们不在键盘下面。有关如何与键盘交互的信息，请参阅适用于 iOS 的文本编程指南。
  
&emsp;视图控制器是启动视图更改的常用位置。因为视图控制器管理与所显示内容相关联的视图层次结构，所以它最终要对这些视图发生的一切负责。加载视图或处理方向更改时，视图控制器可以添加新视图、隐藏或替换现有视图，并进行任意数量的更改，以使视图准备好显示。如果实现了对编辑视图内容的支持，则 `setEditing:animated:` 方法为你提供了一个将视图转换为可编辑版本和从可编辑版本转换视图的位置。
  
&emsp;动画块是启动视图相关更改的另一个常见位置。UIView 类中内置的动画支持使对视图属性的更改设置动画变得容易。你也可以使用 `transitionWithView:duration:options:animations:completion:` 或 `transitionFromView:toView:duration:options:completion:`  将整组视图交换为新视图的方法。

### Interacting with Core Animation Layers（与 Core Animation 层交互）
&emsp;每个视图对象都有一个专用的Core Animation Layers，用于管理视图内容在屏幕上的显示和动画。尽管可以对视图对象进行大量处理，但也可以根据需要直接处理相应的 layer 对象。视图的图层对象存储在视图的 layer 属性中。

#### Changing the Layer Class Associated with a View（更改与视图关联的图层类）
&emsp;创建视图后，无法更改与视图关联的图层类型。因此，每个视图都使用 `layerClass` 类方法来指定其图层对象的类。此方法的默认实现返回 CALayer 类，更改此值的唯一方法是子类化、重写该方法并返回不同的值。可以更改此值以使用不同类型的 layer。例如，如果视图使用平铺来显示大的可滚动区域，则可能需要使用 CATiledLayer 类来支持视图。

&emsp;`layerClass` 方法的实现应该只是创建所需的类对象并返回它。例如，使用平铺的视图将具有此方法的以下实现：
```c++
+ (Class)layerClass {
    return [CATiledLayer class];
}
```
&emsp;每个视图在其初始化过程的早期调用其 `layerClass` 方法，并使用返回的类创建其 layer 对象。此外，视图始终将自己指定为其图层对象的 delegate。此时，视图拥有其层，视图和层之间的关系不得更改。也不能指定与任何其他图层对象的代理相同的视图。更改视图的所有权或委托关系将在应用程序中导致绘图问题和潜在崩溃。

&emsp;有关 Core Animation 提供的不同类型的层对象的详细信息，请参见 Core Animation Reference Collection。

#### Embedding Layer Objects in a View（在视图中嵌入图层对象）
&emsp;如果希望主要处理图层对象而不是视图，可以根据需要将自定义图层对象合并到视图层次中。自定义图层对象是不属于视图的 CALayer 的任何实例。通常以编程方式创建自定义层，并使用 Core Animation 例程合并它们。自定义层不接收事件或参与响应者链，但会根据 Core Animation 规则绘制自身并响应其父视图或层中的大小更改。
&emsp;清单 3-2 显示了一个视图控制器的 viewDidLoad 方法示例，该视图控制器创建一个自定义图层对象并将其添加到其根视图中。该层用于显示已设置动画的静态图像。不是将层添加到视图本身，而是将其添加到视图的 layer。

&emsp;Listing 3-2  Adding a custom layer to a view
```c++
- (void)viewDidLoad {
    [super viewDidLoad];
 
    // Create the layer.
    CALayer* myLayer = [[CALayer alloc] init];
 
    // Set the contents of the layer to a fixed image. And set
    // the size of the layer to match the image size.
    UIImage layerContents = [[UIImage imageNamed:@"myImage"] retain];
    CGSize imageSize = layerContents.size;
 
    myLayer.bounds = CGRectMake(0, 0, imageSize.width, imageSize.height);
    myLayer = layerContents.CGImage;
 
    // Add the layer to the view.
    CALayer*    viewLayer = self.view.layer;
    [viewLayer addSublayer:myLayer];
 
    // Center the layer in the view.
    CGRect        viewBounds = backingView.bounds;
    myLayer.position = CGPointMake(CGRectGetMidX(viewBounds), CGRectGetMidY(viewBounds));
 
    // Release the layer, since it is retained by the view's layer
    [myLayer release];
}
```
&emsp;如果需要，可以添加任意数量的子 layer 并将它们排列成子层层次结构。但是，在某些情况下，这些图层必须附着到视图的图层对象。
有关如何直接使用层的信息，请参见 Core Animation Programming Guide。

### Defining a Custom View（定义自定义视图）
&emsp;如果标准系统 views 不能完全满足你的需要，你可以定义一个自定义视图。自定义视图使你能够完全控制应用程序内容的外观以及如何处理与该内容的交互。

> Note: 如果你使用 OpenGL ES 进行绘图，那么应该使用 GLKView 类而不是 UIView 子类。有关如何使用 OpenGL ES 绘图的更多信息，请参见 OpenGL ES Programming Guide。

#### Checklist for Implementing a Custom View（实施自定义视图的清单）
&emsp;自定义视图的任务是呈现内容并管理与该内容的交互。但是，自定义视图的成功实现不仅仅涉及绘制和处理事件。以下清单包括在实现自定义视图时可以重写的更重要的方法（以及可以提供的行为）：

+ 为你的 views 定义适当的初始化方法：
  + 对于计划以编程方式创建的视图，请重写 `initWithFrame:` 方法或定义自定义初始化方法。
  + 对于计划从 nib 文件加载的视图，请重写 `initWithCoder:` 方法。使用此方法初始化视图并将其置于已知状态。
+ 实现一个 `dealloc` 方法来处理所有自定义数据的清理。
+ 要处理任何自定义绘图，请重写 `drawRect:` 方法并在那里进行图形绘制。
+ 设置视图的 `autoresizingMask` 属性以定义其自动调整大小行为。
+ 如果你的视图类管理一个或多个整体子视图，请执行以下操作：
  + 在视图的初始化序列中创建这些子视图。
  + 在创建时设置每个子视图的 `autoresizingMask` 属性。
  + 如果你的子视图需要自定义布局，请重写 `layoutSubviews` 方法并在那里实现布局代码。
+ 要处理基于触摸的事件，请执行以下操作：
  + 使用 `addGestureRecognizer:` 方法将任何合适的手势识别器附加到视图。
  + 对于要自己处理触摸的情况，请重写 `touchesBegan:withEvent:`、`touchesMoved:withEvent:`、`touchesEnded:withEvent:` 和 `touchesCancelled:withEvent:` 方法。 （请记住，无论你要重写其他与触摸相关的其他方法，都应始终覆盖 `touchesCancelled:withEvent:` 方法。）
+ 如果希望视图的打印版本看起来与屏幕上的版本不同，请实现 `drawRect:forViewPrintFormatter:` 方法。有关如何在视图中支持打印的详细信息，请参见 Drawing and Printing Guide for iOS。

&emsp;除了重写方法之外，请记住，可以对视图的现有属性和方法做很多事情。例如：contentMode 和 contentStretch 属性允许你更改视图的最终呈现外观，并且可能比自己重新绘制内容更可取。除了 UIView 类本身之外，还可以直接或间接配置视图的底层 CALayer 对象的许多方面。甚至可以更改 layer 对象本身的类。

&emsp;有关视图类的方法和属性的更多信息，请参见 UIView Class Reference。

#### Initializing Your Custom View（初始化你的自定义视图）
&emsp;你定义的每个新视图对象都应该包含一个自定义的 `initWithFrame:` initializer 方法。此方法负责在创建时初始化类，并将视图对象置于已知状态。在代码中以编程方式创建视图实例时，可以使用此方法。

&emsp;清单 3-3 显示了一个标准 `initWithFrame:` 方法的 skeletal 实现。此方法首先调用方法的继承实现，然后在返回初始化对象之前初始化类的实例变量和状态信息。传统上首先调用继承的实现，这样如果出现问题，就可以中止自己的初始化代码并返回 nil。

&emsp;Listing 3-3  Initializing a view subclass
```c++
- (id)initWithFrame:(CGRect)aRect {
    self = [super initWithFrame:aRect];
    if (self) {
          // setup the initial properties of the view
          ...
       }
    return self;
}
```
&emsp;如果你计划从 nib 文件加载自定义视图类的实例，那么应该注意，在 iOS 中，nib 加载代码不使用 `initWithFrame:` 方法来实例化新的视图对象。相反，它使用 `initWithCoder:` 方法，该方法是 NSCoding 协议的一部分。

&emsp;即使视图采用 NSCoding 协议，Interface Builder 也不知道视图的自定义属性，因此不会将这些属性编码到 nib 文件中。因此，你自己的 `initWithCoder:` 方法应该尽可能执行任何初始化代码，以将视图置于已知状态。你还可以在视图类中实现 `awakeFromNib` 方法，并使用该方法执行额外的初始化。

#### Implementing Your Drawing Code（实施绘图代码）
&emsp;对于需要进行自定义绘制的视图，需要重写 `drawRect:` 方法并在那里进行绘制。只有在万不得已的情况下才建议使用自定义绘图。一般来说，如果你可以使用其他视图来显示你的内容，这是首选。

&emsp;`drawRect:` 方法的实现应该只做一件事：绘制内容。此方法不适合更新应用程序的数据结构或执行与绘图无关的任何任务。它应该配置绘图环境、绘制内容，并尽快退出。如果你的 `drawRect:` 方法可能经常被调用，那么你应该尽一切可能优化绘图代码，并且每次调用该方法时尽可能少地进行绘图。

&emsp;在调用视图的 `drawRect:` 方法之前，UIKit 会为视图配置基本的绘图环境。具体来说，它创建一个图形上下文，并调整坐标系和剪裁区域以匹配视图的坐标系和可见边界。因此，在调用 `drawRect:` 方法时，可以开始使用 native 绘图技术（如 UIKit 和 Core Graphics）绘制内容。可以使用 UIGraphicsGetCurrentContext 函数获取指向当前图形上下文的指针。

> Important: 当前图形上下文仅在对视图的 `drawRect:` 方法的一次调用期间有效。UIKit 可能会为随后对该方法的每次调用创建不同的图形上下文，因此你不应该尝试缓存该对象并在以后使用它。

&emsp;清单 3-4 展示了 `drawRect:` 方法的一个简单实现，该方法在视图周围绘制一个 10 像素宽的红色边框。由于 UIKit 绘图操作将 Core Graphics 用于其底层实现，因此可以混合绘图调用（如图所示）以获得预期的结果。

&emsp;Listing 3-4  A drawing method
```c++
- (void)drawRect:(CGRect)rect {
    CGContextRef context = UIGraphicsGetCurrentContext();
    CGRect    myFrame = self.bounds;
 
    // Set the line width to 10 and inset the rectangle by
    // 5 pixels on all sides to compensate for the wider line.
    CGContextSetLineWidth(context, 10);
    CGRectInset(myFrame, 5, 5);
 
    [[UIColor redColor] set];
    UIRectFrame(myFrame);
}
```
&emsp;如果你知道视图的图形代码始终使用不透明内容覆盖视图的整个表面，则可以通过将视图的 opaque 属性设置为 YES 来提高系统性能。将视图标记为不透明时，UIKit 会避免位于视图后面的图形内容。这不仅减少了绘图所花费的时间，而且还可以最大限度地减少将视图与其他内容合成所必须完成的工作。但是，只有当你知道视图的内容完全不透明时，才应该将此属性设置为 YES。如果视图不能保证其内容始终不透明，则应将属性设置为 NO。

&emsp;另一种提高绘图性能的方法，特别是在滚动期间，是将视图的 ClearContextBeforeDrawing 属性设置为 NO。当此属性设置为 YES 时，UIKIt 会在调用方法之前自动用透明黑色填充 `drawRect:` 方法要更新的区域。将此属性设置为 NO 可以消除填充操作的开销，但会给应用程序带来用内容填充传递给 `drawRect:` 方法的更新矩形的负担。

#### Responding to Events
&emsp;视图对象是 UIResponder 类的 responder 对象实例，因此能够接收触摸事件。当发生触摸事件时，window 将相应的事件对象分派到发生触摸的视图。如果你的视图对某个事件不感兴趣，它可以忽略该事件，或者将其传递给响应者链，由其他对象处理。

&emsp;除了直接处理触摸事件外，视图还可以使用手势识别器来检测轻触、滑动、挤压和其他类型的常见触摸相关手势。手势识别器完成了跟踪触摸事件并确保它们遵循正确的标准来将它们限定为目标手势的艰巨工作。应用程序不必跟踪触摸事件，你可以创建手势识别器，为其指定适当的目标对象和操作方法，并使用 `addGestureRecognizer:` 方法将其安装到视图中。然后，当相应的手势出现时，手势识别器调用 action 方法。

&emsp;如果你喜欢直接处理触摸事件，则可以为视图实现以下方法，这些方法在 iOS 事件处理指南中有更详细的描述：

+ touchesBegan:withEvent:
+ touchesMoved:withEvent:
+ touchesEnded:withEvent:
+ touchesCancelled:withEvent:

&emsp;视图的默认行为是一次只响应一次触摸。如果用户放下第二根手指，系统将忽略触摸事件，并且不会将其报告给你的视图。如果计划从视图的事件处理程序方法跟踪多指手势，则需要通过将视图的 `multipleTouchEnabled` 属性设置为 YES 来启用多点触控事件。

&emsp;一些视图（如标签和图像）最初完全禁用事件处理。你可以通过更改视图的 `userInteractionEnabled` 属性的值来控制视图是否能够接收触摸事件。你可以临时将此属性设置为 NO，以防止用户在长时间操作挂起时操纵视图的内容。为了防止事件到达任何视图，还可以使用 UIApplication 对象的 `beginIgnoringInteractionEvents` 和 `endIgnoringInteractionEvents` 方法。这些方法影响整个应用程序的事件传递，而不仅仅是单个视图。

> Note: UIView 的动画方法通常在动画进行时禁用触摸事件。可以通过适当配置动画来覆盖此行为。有关执行动画的详细信息，请参见 Animations。

&emsp;在处理触摸事件时，UIKit 使用 `hitTest:withEvent:` 和 `pointInside:withEvent:` 的 UIView 的方法，以确定触摸事件是否发生在给定视图的 bounds 内。尽管你很少需要重写这些方法，但你可以这样做来实现视图的自定义触摸行为。例如，可以重写这些方法以防止子视图处理触摸事件。

#### Cleaning Up After Your View（查看后清理）
&emsp;如果视图类分配任何内存、存储对任何自定义对象的引用，或者保存在视图释放时必须释放的资源，则必须实现 `dealloc` 方法。当视图的引用计数为零时，系统将调用 `dealloc` 方法，现在是取消分配视图的时候了。这个方法的实现应该释放视图持有的任何对象或资源，然后调用继承的实现，如清单 3-5 所示。不应使用此方法执行任何其他类型的任务。

&emsp;Listing 3-5  Implementing the dealloc method
```c++
- (void)dealloc {
    // Release a retained UIColor object
    [color release];
 
    // Call the inherited implementation
    [super dealloc];
}
```

## UIView

### Laying out Subviews（布局子视图）
&emsp;如果你的应用未使用 Auto Layout，则手动设置视图的布局。
#### - layoutSubviews
&emsp;布局子视图。
```c++
// override point. called by layoutIfNeeded automatically. 
// As of iOS 6.0, when constraints-based layout is used the base implementation applies the constraints-based layout, otherwise it does nothing.
- (void)layoutSubviews;
```
&emsp;由 `- layoutIfNeeded` 自动调用。从 iOS 6.0 开始，使用基于约束（constraints-based）的布局时，基本实现将应用基于约束的布局，否则不执行任何操作。

&emsp;此方法的默认实现在 iOS 5.1 及更低版本上不执行任何操作。否则，默认实现将使用你设置的任何约束来确定任何子视图的大小（size）和位置（position）。

&emsp;子类可以根据需要重写此方法，以更精确地布局其子视图。仅当子视图的自动调整大小（autoresizing）和基于约束（constraint-based）的行为没有提供所需的行为时，才应重写此方法。你可以使用实现直接设置子视图的 frame rectangles。

&emsp;你不应该直接调用此方法。如果要强制更新布局，请在下次 drawing update 之前调用 `- setNeedsLayout` 方法。如果要立即更新视图的布局，请调用 `- layoutIfNeeded` 方法。
#### - setNeedsLayout
&emsp;使 UIView 的当前布局无效，并在下一个更新周期内触发布局更新。
```c++
// Allows you to perform layout before the drawing cycle happens.
// -layoutIfNeeded forces layout early.
- (void)setNeedsLayout;
```
&emsp;允许你在绘图周期发生之前执行布局。

&emsp;如果要调整视图子视图的布局，请在应用程序的主线程上调用此方法。此方法记录请求并立即返回。因为此方法不强制立即更新，而是等待下一个更新周期，所以可以使用它在更新任何视图之前使多个视图的布局无效。此行为允许你将所有布局更新合并到一个更新周期，这通常会提高性能。
#### - layoutIfNeeded
&emsp;如果布局更新正在等待中，请立即布置子视图。
```c++
- (void)layoutIfNeeded;
```
&emsp;使用此方法可强制视图立即更新其布局。使用 Auto Layout 时，布局引擎会根据需要更新视图的位置，以满足约束的更改。使用接收消息的视图作为根视图，此方法从根视图开始布局子视图。如果根视图没有被标记为需要更新布局，则此方法将退出，而不修改布局或调用任何与布局相关的回调。
#### requiresConstraintBasedLayout
&emsp;一个布尔值，指示 UIView 是否依赖于基于约束的布局系统。
```c++
/* constraint-based layout engages lazily when someone tries to use it (e.g., adds a constraint to a view).  
 * If you do all of your constraint set up in -updateConstraints, you might never even receive updateConstraints if no one makes a constraint.  
 * To fix this chicken and egg problem, override this method to return YES if your view needs the window to use constraint-based layout.  
 */
@property(class, nonatomic, readonly) BOOL requiresConstraintBasedLayout API_AVAILABLE(ios(6.0));
```
&emsp;Return Value: 如果视图必须位于使用基于约束的布局的 window 中才能正常运行，则为 YES，否则为 NO。

&emsp;如果自定义视图无法使用自动调整大小正确地布局，则应覆盖它以返回 YES。
#### translatesAutoresizingMaskIntoConstraints
&emsp;一个布尔值，它确定视图的自动调整大小蒙版是否转换为 Auto Layout 约束。
```c++
/* By default, the autoresizing mask on a view gives rise to constraints that fully determine the view's position. 
 * This allows the auto layout system to track the frames of views whose layout is controlled manually (through -setFrame:, for example).
 * When you elect to position the view using auto layout by adding your own constraints, 
 * you must set this property to NO. IB will do this for you.
 */
@property(nonatomic) BOOL translatesAutoresizingMaskIntoConstraints API_AVAILABLE(ios(6.0)); // Default YES
```
&emsp;如果此属性的值为 YES，则系统将创建一组约束，这些约束将复制视图的自动调整大小蒙版所指定的行为。这也使你可以使用视图的 frame，bounds 或 center 属性来修改视图的大小和位置，从而可以在 Auto Layout 中创建基于 frame 的静态布局。

&emsp;请注意，自动调整大小遮罩约束完全指定视图的大小和位置；因此，不能添加其他约束来修改此大小或位置，而不会引入冲突。如果要使用 Auto Layout 动态计算视图的大小和位置，则必须将此属性设置为 NO，然后为视图提供一组无歧义、无冲突的约束。

&emsp;默认情况下，以编程方式创建的任何视图的属性均设置为 YES。如果在 Interface Builder 中添加视图，则系统会自动将此属性设置为 NO。
### Drawing and Updating the View（绘制和更新视图）
#### - drawRect:
&emsp;在传入的矩形内绘制 UIView 的图像。
```c++
- (void)drawRect:(CGRect)rect;
```
&emsp;`rect`: 视图范围中需要更新的部分。第一次绘制视图时，此矩形通常是视图的整个可见范围。但是，在后续的绘图操作中，矩形可能仅指定视图的一部分。

&emsp;此方法的默认实现不执行任何操作。使用 Core Graphics 和 UIKit 等技术绘制视图内容的子类应重写此方法并在那里实现其绘制代码。如果视图以其他方式设置其内容，则不需要重写此方法。例如，如果视图仅显示背景色，或者视图直接使用底层对象（CALayer）设置其内容，则不需要重写此方法。

&emsp;调用此方法时，UIKit 已经为你的视图适当地配置了绘图环境，你可以简单地调用渲染内容所需的任何绘图方法和函数。具体来说，UIKit 创建和配置用于绘制的图形上下文，并调整该上下文的转换，使其原点与视图边框的原点匹配。可以使用 UIGraphicsGetCurrentContext 函数获取对图形上下文的引用，但不要建立对图形上下文的强引用，因为它可以在调用 `- drawRect:` 方法之间更改。

&emsp;类似地，如果使用 OpenGL ES 和 GLKView 类进行绘制，那么 GLKit 在调用此方法（或 `glkView:drawInRect:` GLKView 委托的方法），因此你只需发出渲染内容所需的任何 OpenGL ES 命令。有关如何使用 OpenGL ES 绘图的更多信息，请参见 OpenGL ES Programming Guide。

&emsp;应将任何图形限制为 rect 参数中指定的矩形。此外，如果视图的 opaque 属性设置为 YES，则 `drawRect:` 方法必须用不透明内容完全填充指定的矩形。

&emsp;如果直接将 UIView 子类化，则此方法的实现不需要调用 super。但是，如果你正在子类化一个不同的视图类，那么应该在实现的某个时候调用 super。

&emsp;当第一次显示视图或发生使视图的可见部分无效的事件时，将调用此方法。你不应该自己直接调用这个方法。若要使视图的一部分无效，从而导致该部分被重新绘制，请改为调用 `setNeedsDisplay` 或 `setNeedsDisplayInRect:` 方法。
#### - setNeedsDisplay
&emsp;将 UIView 的整个 bounds 矩形标记为需要重绘。
```c++
- (void)setNeedsDisplay;
```
&emsp;你可以使用此方法或 `- setNeedsDisplayInRect:` 通知系统你的视图内容需要重绘。此方法记录请求并立即返回。该视图实际上直到下一个绘图周期才被重绘，此时所有无效的视图都将更新。

&emsp;Note: 如果视图由 CAEAGLLayer 对象支持，则此方法无效。它仅适用于使用本机绘图技术（例如 UIKit 和 Core Graphics）来呈现其内容的视图。

&emsp;应该使用此方法请求仅当视图的内容或外观更改时才重新绘制视图。如果仅更改视图的几何图形，则通常不会重新绘制视图。而是根据视图的 contentMode 属性中的值调整其现有内容。重新显示现有内容可以避免重新绘制未更改的内容，从而提高性能。
#### - setNeedsDisplayInRect:
&emsp;将 UIView 的指定矩形标记为需要重绘。
```c++
- (void)setNeedsDisplayInRect:(CGRect)rect;
```
&emsp;`rect`: 接收器的矩形区域标记为无效；它应该在接收器的坐标系中指定。

&emsp;可以使用此方法或 `- setNeedsDisplay` 通知系统需要重画视图的内容。此方法将指定的矩形添加到视图的当前无效矩形列表中，并立即返回。在下一个绘图周期之前，视图实际上不会重新绘制，此时所有无效的视图都会更新。

&emsp;Note: 如果视图由 CAEAGLLayer 对象支持，则此方法无效。它仅适用于使用本机绘图技术（例如 UIKit 和 Core Graphics）来呈现其内容的视图。

&emsp;应该使用此方法请求仅当视图的内容或外观更改时才重新绘制视图。如果仅更改视图的几何图形，则通常不会重新绘制视图。而是根据视图的 contentMode 属性中的值调整其现有内容。重新显示现有内容可以避免重新绘制未更改的内容，从而提高性能。
#### contentScaleFactor
&emsp;应用于视图的比例因子。
```c++
@property(nonatomic) CGFloat    contentScaleFactor API_AVAILABLE(ios(4.0));
```
&emsp;比例因子确定视图中的内容如何从逻辑坐标空间（以点为单位）映射到设备坐标空间（以像素为单位）。此值通常为 1.0 或 2.0。较高的比例因子表示视图中的每个点由底层中的多个像素表示。例如，如果比例因子为 2.0，图幅大小为 50 x 50 点，则用于显示该内容的位图大小为 100 x 100 像素。

&emsp;此属性的默认值是与当前显示视图的屏幕相关联的比例因子。如果你的自定义视图实现了一个自定义 `- drawRect:` 方法并与窗口相关联，或者如果你使用 GLKView 类来绘制 OpenGL ES 内容，那么你的视图将以屏幕的完整分辨率进行绘制。对于系统视图，即使在高分辨率屏幕上，此属性的值也可能为 1.0。

&emsp;通常，不需要修改此属性中的值。但是，如果应用程序使用 OpenGL ES 绘制，则可能需要更改比例因子，以牺牲图像质量来获得渲染性能。有关如何调整 OpenGL ES 渲染环境的详细信息，请参见 OpenGL ES Programming Guide 中的 Supporting High-Resolution Displays 部分。
#### - tintColorDidChange
&emsp;当 tintColor 属性更改时由系统调用。
```c++
/*
 * The -tintColorDidChange message is sent to appropriate subviews of a view when its tintColor
 * is changed by client code or to subviews in the view hierarchy of a view whose tintColor is
 * implicitly changed when its superview or tintAdjustmentMode changes.
 */
- (void)tintColorDidChange API_AVAILABLE(ios(7.0));
```
&emsp;当你的代码更改该视图上的 tintColor 属性的值时，系统将在该视图上调用此方法。此外，系统在继承了更改的交互色的子视图上调用此方法。

&emsp;在你的实现中，根据需要刷新视图渲染。

## 参考链接
**参考链接:🔗**
+ [UIView](https://developer.apple.com/documentation/uikit/uiview?language=objc)
+ [View Programming Guide for iOS](https://developer.apple.com/library/archive/documentation/WindowsViews/Conceptual/ViewPG_iPhoneOS/Introduction/Introduction.html#//apple_ref/doc/uid/TP40009503)
