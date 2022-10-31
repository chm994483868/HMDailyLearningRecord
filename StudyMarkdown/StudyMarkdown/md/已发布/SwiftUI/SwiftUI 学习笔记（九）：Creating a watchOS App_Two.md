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

&emsp;在预览版中，创建模型数据的实例，并使用它来将 landmark 对象传递给 LandmarkDetail 结构体的初始值设定项。你还需要设置视图的 environment object。

```swift
import SwiftUI

struct LandmarkDetail: View {
    @EnvironmentObject var modelData: ModelData
    var landmark: Landmark

    var landmarkIndex: Int {
        modelData.landmarks.firstIndex(where: { $0.id == landmark.id })!
    }

    var body: some View {
        Text("Hello, World!")
    }
}

struct LandmarkDetail_Previews: PreviewProvider {
    static var previews: some View {
        let modelData = ModelData()
        return LandmarkDetail(landmark: modelData.landmarks[0])
            .environmentObject(modelData)
    }
}
```

&emsp;从 body() 方法返回一个 CircleImage 视图。在这里，你可以重用 iOS 项目中的 CircleImage 视图。由于你创建了可调整大小的图像，因此对 scaledToFill() 的调用会调整 circle 的大小，使其填充显示。

```swift
...
CircleImage(image: landmark.image.resizable())
    .scaledToFill()
}
...
```

&emsp;为 largest (44mm) 和 smallest (40mm) 表盘创建 Previews。

&emsp;通过针对最大和最小的表盘进行测试，你可以了解应用在显示屏上的缩放程度。与往常一样，你应该在所有受支持的设备大小上测试用户界面。

```swift
...
return Group {
    LandmarkDetail(landmark: modelData.landmarks[0])
        .environmentObject(modelData)
        .previewDevice("Apple Watch Series 5 - 44mm")
    
    LandmarkDetail(landmark: modelData.landmarks[1])
        .environmentObject(modelData)
        .previewDevice("Apple Watch Series 5 - 40mm")
    }
...
```

&emsp;circle image 将调整大小以填充屏幕。不幸的是，这会导致剪切。要解决剪切问题，你需要将 image 嵌入到 VStack 中并进行一些额外的布局更改，以便 circle image 适合任何 iWatch。

&emsp;将 circle image 嵌入到 VStack 中。在 image 下方显示 landmark name 及其 information。如你所见，该信息不太适合手表屏幕，但你可以通过将 VStack 放置在 scroll view 中来解决此问题。

```swift
VStack {
    CircleImage(image: landmark.image.resizable())
        .scaledToFill()
        
    Text(landmark.name)
        .font(.headline)
        .lineLimit(0)
    
    Toggle(isOn: $modelData.landmarks[landmarkIndex].isFavorite) {
        Text("Favorite")
    }
    
    Divider()
    
    Text(landmark.park)
        .font(.caption)
        .bold()
        .lineLimit(0)
    
    Text(landmark.state)
        .font(.caption)
    }
```

&emsp;在 scroll view 中环绕 vertical stack。

&emsp;这将打开视图滚动，但它会产生另一个问题：circle image 现在扩展到完整大小，并且它会调整其他 UI 元素的大小以匹配 image 大小。你需要调整 circle image 的大小，以便屏幕上仅显示 circle 和 landmark 名称。

```swift
ScrollView {
...
}
```

&emsp;将 `scaleToFill()` 更改为 `scaleToFit()` 并添加 padding。这将缩放 circle image 以匹配屏幕的宽度，并确保 landmark name 在 circle image 下方可见。

```swift
...
.scaledToFit()
...
.padding(16)
...
```

&emsp;在 divider 后添加 MapView。map 将显示在屏幕外，但如果启用实时预览，则可以向下滚动以查看它。

```swift
...
Divider()

MapView(coordinate: landmark.locationCoordiante)
    .scaledToFit()
...
```

&emsp;向 back button 添加 title。这会将 back button 的 text 设置为 "Landmarks"。

```swift
...
.navigationTitle("Landmarks")
...
```

### Add the Landmarks List

&emsp;你为 iOS 创建的 LandmarkList 也适用于你的 watchOS app，它会自动 navigates 到你刚刚为 watchOS 编译时创建的特定于 watchOS 的 watch-specific detail view。接下来，你将 list 连接到 watch 的 ContentView，以便它充当手表应用程序的顶级视图。

&emsp;在 WatchLandmarks Watch App 文件夹中选择 ContentView.swift。与 LandmarkDetail.swift 一样，watchOS target 的 content view 与 iOS target 的 content view 具有相同的名称。保持 name 和 interfaces 相同可以轻松地在 targets 之间共享文件。

&emsp;watchOS app 的 root view 显示默认的 "Hello, World!" 信息。修改 ContentView，使其显示 List view。

&emsp;请务必将 model data 作为 environment object 提供给 preview。LandmarksApp 在运行时已经在应用级别提供了此功能，就像它在 iOS 上一样，但你还必须为需要它的任何 previews 提供它。

&emsp;启动 live preview 以查看应用的行为方式。

### Create a Custom Notification Interface

&emsp;你的 watchOS 版 Landmarks 即将完成。在最后一部分中，你将创建一个 notification interface，它会在你收到指示你靠近你最喜欢的位置之一的通知时显示地标信息。

&emsp;本节仅介绍如何在收到通知后显示通知。它没有描述如何设置或发送通知。

&emsp;打开 NotificationView.swift 并创建一个显示有关 Landmark、title 和消息信息的视图。由于任何通知值都可以为 nil，因此预览将显示通知视图的两个版本。第一个仅显示未提供数据时的默认值，第二个显示你提供的标题、消息和位置。

```swift
import SwiftUI

struct NotificationView: View {
    var title: String?
    var message: String?
    var landmark: Landmark?
    
    var body: some View {
        VStack {
            if landmark != nil {
                CircleImage(image: landmark!.image.resizable())
                    .scaledToFit()
            }
            
            Text(title ?? "Unknown Landmark")
                .font(.headline)
            
            Divider()
            
            Text(message ?? "You are within 5 miles of one of your favorite landmarks.")
                .font(.caption)
        }
        .lineLimit(0)
    }
}

struct NotificationView_Previews: PreviewProvider {
    static var previews: some View {
        Group {
            NotificationView()
            NotificationView(title: "Turtle Rock",
                             message: "You are within 5 miles of Turtle Rock.",
                             landmark: ModelData().landmarks[0])
        }
    }
}
```

&emsp;打开 NotificationController 并添加 landmark、title 和 message properties。这些属性存储有关传入通知的值。

```swift
import WatchKit
import SwiftUI
import UserNotifications

class NotificationController: WKUserNotificationHostingController<NotificationView> {
    var landmark: Landmark?
    var title: String?
    var message: String?
    
    override var body: NotificationView {
        NotificationView()
    }
    
    override func willActivate() {
        // This method is called when watch view controller is about to be visible to user
        super.willActivate()
    }
    
    override func didDeactivate() {
        // This method is called when watch view controller is no longer visible
        super.didDeactivate()
    }
    
    override func didReceive(_ notification: UNNotification) {
        // This method is called when a notification needs to be presented.
        // Implement it if you use a dynamic notification interface.
        // Populate your dynamic notification interface as quickly as possible.
    }
}
```

&emsp;更新 body() 方法以使用这些属性。此方法实例化你之前创建的通知视图。

```swift
...
    override var body: NotificationView {
        NotificationView(title: title, message: message, landmark: landmark)
    }
...
```

&emsp;定义 LandmarkIndexKey。

&emsp;你可以使用此 key 从通知中提取 landmark 索引。

```swift
...
    let landmarkIndexKey = "landmarkIndex"
...
```

&emsp;更新 `didReceive(_:)` 方法分析通知中的数据。此方法更新控制器的属性。调用此方法后，系统会使控制器的 body 属性失效，该属性将更新导航视图。然后，系统会在 Apple Watch 上显示通知。

```swift
...
    override func didReceive(_ notification: UNNotification) {
        // This method is called when a notification needs to be presented.
        // Implement it if you use a dynamic notification interface.
        // Populate your dynamic notification interface as quickly as possible.
        
        let modelData = ModelData()
        let notificationData = notification.request.content.userInfo as? [String: Any]
        
        let aps = notificationData?["aps"] as? [String: Any]
        let alert = aps?["alert"] as? [String: Any]
        
        title = alert?["title"] as? String
        message = alert?["body"] as? String
        
        if let index = notificationData?[landmarkIndexKey] as? Int {
            landmark = modelData.landmarks[index]
        }
    }
...
```

&emsp;当 Apple Watch 收到通知时，它会在你的 app 中查找与通知类别关联的场景。

```swift
...
        #if os(watchOS)
        WKNotificationScene(controller: NotificationController.self, category: "LandmarkNear")
        #endif
...
```

&emsp;转到 LandmarksApp.swif 并使用 LandmarkNear 类别添加 WKNotificationScene。该 scene 仅对 watchOS 有意义，因此请添加条件编译。

&emsp;将测试 payload 配置为使用 LandmarkNear 类别并传递通知控制器所需的数据。选择 PushNotificationPayload.apns 文件，并更新标题、正文、类别和地标索引属性。请务必将类别设置为 LandmarkNear。还可以删除本教程中未使用的任何键，例如副标题、WatchKit 模拟器操作和自定义键。payload 文件在远程通知中模拟从服务器发送的数据。

```swift
{
    "aps": {
        "alert": {
            "title": "Silver Salmon Creek",
            "body": "You are within 5 miles of Silver Salmon Creek."
        },
        "category": "LandmarkNear",
        "thread-id": "5280"
    },

    "landmarkIndex": 1,

    "WatchKit Simulator Actions": [
        {
            "title": "First Button",
            "identifier": "firstButtonAction"
        }
    ],

    "customKey": "Use this file to define a testing payload for your notifications. The aps dictionary specifies the category, alert text and title. The WatchKit Simulator Actions array can provide info for one or more action buttons in addition to the standard Dismiss button. Any other top level keys are custom payload. If you have multiple such JSON files in your project, you'll be able to select them when choosing to debug the notification interface of your Watch App."
}
```

&emsp;选择 Landmarks-Watch (Notification) scheme，然后生成并运行应用。首次运行 notification scheme 时，系统会请求发送通知的权限。选择允许。

&emsp;授予权限后，模拟器会显示可滚动的通知，其中包括：帮助将地标应用标识为发件人的应用图标、通知视图以及通知操作的按钮。

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

