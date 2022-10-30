# SwiftUI 学习笔记（九）：Creating a watchOS App.md

&emsp;SwiftUI 官方教程：[SwiftUI Tutorials](https://developer.apple.com/tutorials/swiftui) 仅是几个体现 SwiftUI 简单使用的小 demo 而已，简单易学，循序渐进，先看完可以对 SwiftUI 有一个大概的认知。

## 九：Creating a watchOS App

&emsp;[Framework Integration - Creating a watchOS App](https://developer.apple.com/tutorials/swiftui/creating-a-watchos-app) 创建一个 watchOS App。

&emsp;本教程让你有机会应用你已经学到的有关 SwiftUI 的大部分知识，并且毫不费力地将 Landmarks 应用程序迁移到 watchOS。在复制你为 iOS 应用程序创建的共享数据和视图之前，你将首先将 watchOS targe 添加到你的项目中。准备好所有 assets 后，你将自定义 SwiftUI 视图以在 watchOS 上显示详细信息和列表视图。

&emsp;按照步骤构建此项目，或下载完成的项目以自行探索。

### Add a watchOS Target

&emsp;若要创建 watchOS 应用，请首先将 watchOS target 添加到 project。Xcode 将 watchOS app 的 groups and files 添加到你的项目中，以及 build and run app 所需的 schemes。

&emsp;选择 File > New > Target。当模板表出现时，选择 watchOS 选项卡，选择 Watch App for iOS App 模板，然后单击下一步。此模板将一个新的 watchOS 应用程序添加到你的项目中，将 iOS 应用程序设置为其 companion（伴侣）。

&emsp;在 sheet 中，输入 WatchLandmarks 作为 Product Name。将 Interface 选项设置为 SwiftUI，将 Life Cycle 选项设置为 SwiftUI 应用程序，将 Language 设置为 Swift。选中 Include Notification Scene 复选框，然后单击 Finish。

&emsp;如果 Xcode 提示激活 WatchLandmarks (Complication) scheme，请点按 Cancel。

&emsp;你将希望从 WatchLandmarks scheme 开始，接下来选择该方案。选择 WatchLandmarks scheme。这使你可以 build and run 你的 watchOS 应用程序。

&emsp;选择 WatchLandmarks Extension project，然后选择到 project 的 General 选项卡;选中 Supports Running Without iOS App Installation 复选框。尽可能创建一个独立的 watchOS 应用程序。独立 watchOS 应用不需要 iOS companion 应用。

### Share Files Between Targets

&emsp;设置 watchOS target 后，你需要共享 iOS target 中的一些资源。你将重复使用 Landmark 应用程序的数据模型、一些资源文件以及两个平台无需修改即可显示的任何视图。

&emsp;首先，删除 watchOS 应用的入口点。你不需要它，因为你将重用 LandmarksApp.swift 中定义的入口点（entry point）。

&emsp;在 Project navigator 中，删除 WatchLandmarks Watch App 文件夹中的 WatchLandmarksApp.swift 文件；系统提示时，直接选择 Move to trash。

&emsp;此文件 Watch-specific app definition。它的名称取决于 Xcode project 名称，但它始终是 WatchLandmarks Extension 组中的第一项。在接下来的几个步骤中，你将重复使用 iOS 应用定义。

&emsp;接下来，选择 watchOS Target 可以与现有 iOS Target 共享的所有文件，包括应用的入口点。

&emsp;在 Project navigator 中，按住 Command 键单击以选择以下文件：LandmarksApp.swift、LandmarkList.swift、LandmarkRow.swift、CircleImage.swift、MapView.swift。

&emsp;其中第一个是共享应用程序定义。其他是应用程序可以在 watchOS 上显示的视图，无需更改。

&emsp;继续按住 Command 键单击以添加以下模型文件：ModelData.swift、Landmark.swift、Hike.swift、Profile.swift。这些项定义应用的数据模型。你不会使用模型的所有方面，但需要所有文件才能成功编译应用。

&emsp;继续按住 Command 单击以添加模型加载的资源文件：landmarkData.json、hikeData.json 和 Assets.xcassets。

&emsp;在 File inspector 中，选中 Target Membership section 中的 WatchLandmarks Watch App 复选框。

&emsp;最后，添加一个与你已有的 iOS 应用程序图标匹配的 watchOS 应用程序图标。

&emsp;在 WatchLandmarks Watch App 文件夹中选择 Assets.xcasset 文件，然后删除空的 AppIcon 项，在下一步我们将替换它。

&emsp;将 AppIcon.appiconset 从下载项目的 Resources 文件夹中拖到 WatchLandmarks Watch App 的 Assets.xcassets 中。

&emsp;稍后，当你创建通知时，系统会显示应用的图标，以帮助识别通知的来源。

### Create the Detail View

&emsp;现在，iOS target resources 已到位，可用于处理 watchOS app，你需要创建 watch-specific 的视图来显示 landmark 详细信息。要测试详细信息视图，你将为最大和最小的手表尺寸创建自定义预览，并对圆形视图进行一些更改，以便所有内容都适合表盘。

&emsp;在 WatchLandmarks Watch App 文件夹中创建一个名为 LandmarkDetail.swift 的自定义视图。此文件与 iOS 项目中同名的文件通过其 Target membership 进行区分 — 它仅适用于 Watch Extension target。

&emsp;将 modelData、Landmark 和 landmarkIndex 属性添加到新的 LandmarkDetail 结构体中。这些属性与 Handling User Input 中添加的属性相同。

&emsp;在预览版中，创建模型数据的实例，并使用它来将 landmark 对象传递给 LandmarkDetail 结构体的初始值设定项。你还需要设置视图的环境对象。














## 参考链接
**参考链接:🔗**
+ [Creating a watchOS App](https://developer.apple.com/tutorials/swiftui/creating-a-watchos-app)







+ [SwiftUI状态绑定：@State](https://www.jianshu.com/p/46cbe061c8f5)
+ [[译]理解 SwiftUI 里的属性装饰器@State, @Binding, @ObservedObject, @EnvironmentObje](https://www.cnblogs.com/xiaoniuzai/p/11417123.html)
+ [[SwiftUI 100 天] 用 @EnvironmentObject 从环境中读取值](https://zhuanlan.zhihu.com/p/146608338)
+ [SwiftUI 2.0 —— @StateObject 研究](https://zhuanlan.zhihu.com/p/151286558)
+ [Swift 5.5 新特性](https://zhuanlan.zhihu.com/p/395147531)
+ [SwiftUI之属性包装](https://www.jianshu.com/p/28623e017445)
+ [Swift 中的属性包装器 - Property Wrappers](https://www.jianshu.com/p/8a019631b4db)



## 看着看着发现 LG 都开始卷 Swift 源码了...（必学）
+ [Swift底层进阶--015：Codable源码解析](https://www.jianshu.com/p/9302f7bac319)
+ [Swift底层探索:Codable](https://www.jianshu.com/p/d591bd7f53ac)

## 针对当返回的 Json 字符串中字段的类型 和 模型定义中属性（成员变量）类型不匹配时的解析：只要有一个字段类型不匹配，整个 json 的转 model 都会失败，这是不友好的。
+ [针对 swift4 的JSONDecoder的特殊情况处理](https://www.jianshu.com/p/51c219092290)

## 学习一些 Codable 的嵌套用法、学习 Codable 中的三种容器类型（必学），还有 CodingKey（必学）。

+ [Swift5 Codable源码剖析](https://www.jianshu.com/nb/3595319)






1. 京东快递，明天晚上 8:00 买维 B/C/E。
2. 找对象的标准。
3. 再尝试领取一下爱奇艺中的京东会员。
4. 富士康走路回家。
5. 天冷了，冬天的衣服怎么回家拿。
