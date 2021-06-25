# iOS《View Controller Programming Guide for iOS》官方文档

## Overview

### The Role of View Controllers（视图控制器的职责）
&emsp;视图控制器是应用程序内部结构的基础。每个应用程序都至少有一个视图控制器，大多数应用程序都有几个。每个视图控制器管理应用程序用户界面的一部分以及该界面与底层数据之间的交互。视图控制器还促进用户界面不同部分之间的转换。

&emsp;因为它们在你的应用程序中扮演着如此重要的角色，视图控制器几乎是你所做一切的中心。UIViewController 类定义了用于管理视图、处理事件、从一个视图控制器转换到另一个视图控制器以及与应用程序的其他部分协调的方法和属性。你可以将 UIViewController 子类化（或其子类之一），并添加实现应用程序行为所需的自定义代码。

&emsp;有两种类型的视图控制器：

+ 内容视图控制器（Content view controllers）管理应用程序内容的一部分，是你创建的视图控制器的主要类型。
+ 容器视图控制器（Container view controllers）从其他视图控制器（称为子视图控制器）收集信息，并以便于导航或以不同方式显示这些视图控制器的内容来显示这些信息。

&emsp;大多数应用程序是两种类型的视图控制器的混合体。

#### View Management（视图管理）
&emsp;视图控制器最重要的作用是管理视图的层次结构。每个视图控制器都有一个包含所有视图控制器内容的根视图（root view）。向该根视图添加显示内容所需的视图。图 1-1 说明了视图控制器与其视图之间的内置关系。视图控制器始终有一个对其根视图的引用，并且每个视图都有对其子视图的强引用。

&emsp;Figure 1-1Relationship between a view controller and its views（视图控制器与其视图之间的关系）

![VCPG_ControllerHierarchy_fig_1-1](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/b70df0872474476586eab8afc0af14cb~tplv-k3u1fbpfcp-watermark.image)

> &emsp;通常的做法是使用 outlet 访问视图控制器视图层次结构中的其他视图。因为视图控制器管理其所有视图的内容，所以 outlets 允许你存储对所需视图的引用。当从情节提要加载视图时，输出口本身会自动连接到实际视图对象。

##### Outlet
&emsp;outlet 是用符号 IBOutlet 注释的属性，可以在 nib 文件或 storyboard 中以图形方式设置其值。在类的接口中声明一个 outlet，并在 outlet 和 nib 文件或 storyboard 中的另一个对象之间建立连接。当文件被加载时，就建立了连接。

![outlet_generic](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/8edd285423684dfe8357fef2de3a2f36~tplv-k3u1fbpfcp-watermark.image)

&emsp;将 outlet 定义为类型限定符为 IBOutlet 的属性。
```c++
@property (nonatomic, weak) IBOutlet UITextField *nameField;
```
&emsp;符号 IBOutlet 仅由 Xcode 使用，用于确定属性何时为 outlet；它没有实际值。

&emsp;通过 outlet，代码中的对象可以获得对 nib 文件或 storyboard 中定义的对象的引用，然后从该文件加载。包含 outlet 的对象通常是自定义控制器对象，如 view controller。你经常定义 outlet，以便可以发送消息以查看 UIKit 框架（在 iOS 中）和 AppKit 框架（在 OS X 中）的对象。

&emsp;内容视图控制器自行管理其所有视图。容器视图控制器管理自己的视图以及来自一个或多个子视图控制器的根视图。容器不管理其子容器的内容。它只管理根视图，根据容器的设计调整和放置它。图 1-2 说明了 split 视图控制器及其子控制器之间的关系。split 视图控制器管理其子视图的总体大小和位置，但子视图控制器管理这些视图的实际内容。

&emsp;Figure 1-2 View controllers can manage content from other view controllers（视图控制器可以从其他视图控制器管理内容）

![VCPG_ContainerViewController_fig_1-2](https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/fdd33139b2084ea784634496cd476d42~tplv-k3u1fbpfcp-watermark.image)

&emsp;有关管理视图控制器视图的信息，请参见 Managing View Layout。

#### Data Marshaling（数据整理（排列））
&emsp;视图控制器充当它管理的视图和应用程序数据之间的中介。UIViewController 类的方法和属性允许你管理应用程序的可视表示（visual presentation）。当你将 UIViewController 子类化时，你可以添加在子类中管理数据所需的任何变量。添加自定义变量会创建一个如图 1-3 所示的关系，其中视图控制器会引用你的数据和用于表示该数据的视图。在两者之间来回移动数据是你的责任。

&emsp;Figure 1-3 A view controller mediates between data objects and views（视图控制器在数据对象和视图之间进行调解）

![VCPG_CustomSubclasses_fig_1-3](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/59d4c75e85094ed98ff7a7b810435f79~tplv-k3u1fbpfcp-watermark.image)

&emsp;你应该始终在视图控制器和数据对象中保持清晰的职责分离。确保数据结构完整性的大多数逻辑都属于数据对象本身。视图控制器可能会验证来自视图的输入，然后将该输入打包为数据对象所需的格式，但是你应该最小化视图控制器在管理实际数据中的角色。

&emsp;UIDocument 对象是与视图控制器分开管理数据的一种方法。document 对象是一个控制器对象，它知道如何将数据读写到持久存储中。当你创建子类时，你可以添加任何逻辑和方法来提取数据并将其传递给视图控制器或应用程序的其他部分。视图控制器可以存储它接收到的任何数据的副本，以便更容易地更新视图，但是 document 仍然拥有真实的数据。

> UIDocument 用于管理应用程序数据的离散部分的抽象基类。`@interface UIDocument : NSObject`。使用 UIDocument 及其底层体系结构的应用程序可以从其文档中获得许多好处：

  + 在后台队列上异步读写数据。因此，在进行读写操作时，应用程序对用户的响应不会受到影响。
  + 与 cloud services 自动集成的 document 文件的协调读写。
  + 支持发现文档的不同版本之间的冲突（如果发生）。
  + 安全地保存 document 数据，方法是先将数据写入临时文件，然后用它替换当前 document 文件。
  + 适时自动保存 document 数据；此机制支持处理挂起行为。
  
  在 Model-View-Controller 设计模式中，UIDocument 对象是模型对象或模型控制器对象，它管理 document 的数据或聚合模型对象，这些对象共同构成 document 的数据。你通常将它与一个视图控制器配对，该视图控制器管理呈现 document 内容的视图。UIDocument 不支持管理 document 视图。
  
  Document-based 的应用程序包括可以生成多个 documents 的应用程序，每个文档都有自己的文件系统位置。document-based 的应用程序必须为其 documents 创建 UIDocument 的子类。有关详细信息，请参见下面的 "Subclassing Notes"。
  
  Note: 如果使用数据库存储文档数据，请创建 UIManagedDocument 类的子类，而不是 UIDocument；UIManagedDocument 是 UIDocument 的子类。
  
  UIDocument 体系结构中 document 的主要属性是其文件 URL。通过调用 initWithFileURL: 初始化 document 子类的实例时，必须传递一个文件 URL，在应用程序沙盒中定位 document 文件。UIDocument 确定文件 URL 中的文件类型（与文件扩展名关联的统一类型标识符）和文档名称（文件名组件）。你可以重写 fileType 和 localizedName  属性的访问器方法，以提供不同的值。
  
  ...
  
#### User Interactions（用户交互）
&emsp;视图控制器是响应者对象，能够处理响应者链中的事件。尽管视图控制器能够做到这一点，但它们很少直接处理触摸事件。相反，视图通常处理自己的触摸事件，并将结果报告给关联代理或目标对象（通常是视图控制器）的方法。因此，视图控制器中的大多数事件都是使用委托方法或 action 方法处理的。

#### Resource Management（资源管理）
&emsp;视图控制器对其视图及其创建的任何对象承担全部责任。UIViewController 类自动处理视图管理的大部分方面。例如，UIKit 会自动释放不再需要的任何与视图相关的资源。在 UIViewController 子类中，你负责管理显式创建的任何对象。

&emsp;当可用内存不足时，UIKit 会要求应用程序释放不再需要的任何资源。一种方法是调用视图控制器的 didReceiveMemoryWarning 方法。使用该方法可以删除对不再需要或以后可以轻松重新创建的对象的引用。例如，可以使用该方法删除缓存数据。当内存不足时，尽可能释放内存是很重要的。消耗过多内存的应用程序可能会被系统直接终止以恢复内存。

#### Adaptivity（适应性）
&emsp;视图控制器负责其视图的表示，并调整该表示以匹配底层环境。每个 iOS 应用程序都应该能够在 iPad 和几种不同尺寸的 iPhone 上运行。与其为每个设备提供不同的视图控制器和视图层次结构，不如使用单个视图控制器使其视图适应不断变化的空间需求。

&emsp;在 iOS 中，视图控制器需要处理 coarse-grained 更改和 fine-grained 更改。coarse-grained 更改发生在视图控制器的 traits 更改时。traits 是描述整个环境的属性，例如显示比例。最重要的两个特性是视图控制器的水平和垂直大小类，它们指示视图控制器在给定维度中有多少空间。您可以使用大小类更改来更改视图的布局方式，如图 1-4 所示。当 horizontal size class 是规则的时，视图控制器利用额外的水平空间来排列其内容。当 horizontal size class 比较紧凑时，视图控制器将垂直排列其内容。

&emsp;Figure 1-4 Adapting views to size class changes（调整视图以适应大小类更改）

![VCPG_SizeClassChanges_fig_1-4](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/5cc53a2cbc3846469cf02c0b21d1678c~tplv-k3u1fbpfcp-watermark.image)


&emsp;在给定的 size class 中，随时可能发生更 ine-grained 的大小更改。当用户将 iPhone 从纵向旋转到横向时，size class 可能不会改变，但屏幕尺寸通常会改变。使用 Auto Layout 时，UIKit 会自动调整视图的大小和位置以匹配新尺寸。视图控制器可以根据需要进行其他调整。

&emsp;有关适应性的信息，请参见 The Adaptive Model。















## 参考链接
**参考链接:🔗**
+ [View Controller Programming Guide for iOS](https://developer.apple.com/library/archive/featuredarticles/ViewControllerPGforiPhoneOS/index.html#//apple_ref/doc/uid/TP40007457-CH2-SW1)
+ [Cocoa Application Competencies for iOS-Outlet](https://developer.apple.com/library/archive/documentation/General/Conceptual/Devpedia-CocoaApp/Outlet.html#//apple_ref/doc/uid/TP40009071-CH4)
+ [UIDocument](https://developer.apple.com/documentation/uikit/uidocument)
