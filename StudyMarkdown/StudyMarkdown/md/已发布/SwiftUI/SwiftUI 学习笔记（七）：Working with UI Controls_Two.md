# SwiftUI 学习笔记（七）：Working with UI Controls.md

&emsp;SwiftUI 官方教程：[SwiftUI Tutorials](https://developer.apple.com/tutorials/swiftui) 仅是几个体现 SwiftUI 简单使用的小 demo 而已，简单易学，循序渐进，先看完可以对 SwiftUI 有一个大概的认知。

## 七：Working with UI Controls

&emsp;[App Design and Layout - Working with UI Controls](https://developer.apple.com/tutorials/swiftui/working-with-ui-controls) 使用 UI 控件。

&emsp;在 Landmarks 应用程序中，用户可以创建个人资料来表达自己的个性。要使用户能够更改其个人资料，你需要添加编辑模式并设计 preferences screen。

&emsp;你将使用各种常见的用户界面控件进行数据输入，并在用户保存其更改时更新 Landmarks 模型类型。下面按照步骤构建此项目，或下载已完成的项目以自行探索。

### Display a User Profile

&emsp;Landmarks 应用在本地存储一些配置详细信息和首选项。在用户编辑其详细信息之前，它们将显示在没有任何编辑控件的摘要视图中。

&emsp;首先在名为 Profile.swift 的新 Swift 文件中定义用户配置文件，并把它添加到项目的 Model 文件夹中。

```swift
import Foundation

struct Profile {
    var username: String
    var prefersNotifications = true
    var seasonalPhoto = Season.winter
    var goalDate = Date()

    static let `default` = Profile(username: "g_kumar")

    enum Season: String, CaseIterable, Identifiable {
        case spring = "🌷"
        case summer = "🌞"
        case autumn = "🍂"
        case winter = "☃️"

        var id: String { rawValue }
    }
}
```

&emsp;接下来，在 Views 文件夹下创建一个名为 Profiles 的新文件夹，然后在该文件夹下创建一个名为 ProfileHost 的视图，该视图是一个显示存储的 profile 文件的用户名的文本视图。ProfileHost 视图将托管配置文件信息的静态摘要视图和编辑模式。

```swift
import SwiftUI

struct ProfileHost: View {
    @State private var draftProfile = Profile.default

    var body: some View {
        Text("Profile for: \(draftProfile.username)")
    }
}

struct ProfileHost_Previews: PreviewProvider {
    static var previews: some View {
        ProfileHost()
    }
}
```

&emsp;在 Profiles 文件夹中创建一个名为 ProfileSummary 的视图，该视图采用配置文件实例并显示一些基本的用户信息。ProfileSummary 采用 Profile 值，而不是与 Profile 的绑定，因为父视图 ProfileHost 管理此视图的状态。

```swift
import SwiftUI

struct ProfileSummary: View {
    var profile: Profile

    var body: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 10) {
                Text(profile.username)
                    .bold()
                    .font(.title)

                Text("Notifications: \(profile.prefersNotifications ? "On": "Off" )")
                Text("Seasonal Photos: \(profile.seasonalPhoto.rawValue)")
                Text("Goal Date: ") + Text(profile.goalDate, style: .date)
            }
        }
    }
}

struct ProfileSummary_Previews: PreviewProvider {
    static var previews: some View {
        ProfileSummary(profile: Profile.default)
    }
}
```

&emsp;更新 ProfileHost 以显示新的摘要视图。

```swift
...
        VStack(alignment: .trailing, spacing: 20) {
            ProfileSummary(profile: draftProfile)
        }
        .padding()
...
```

&emsp;在 Hikes 文件夹中创建一个名为 HikeBadge 的新视图，该视图由 Drawing Paths and Shapes 章中的 Badge 以及一些有关 hike 的描述性文本组成。Badge 只是一个图形，所以 HikeBadge 中的文本以及 `accessibilityLabel(_:)` 修饰符使 Badge 的含义对其他用户更清晰。Badge 的绘制逻辑生成的结果取决于其渲染的 frame 的大小。为确保所需的外观，请在 300 x 300 的 frame 中渲染。要获得最终图形所需的大小，请缩放渲染结果并将其放置在相对较小的 frame 中。

```swift
import SwiftUI

struct HikeBadge: View {
    var name: String

    var body: some View {
        VStack(alignment: .center) {
            Badge()
                .frame(width: 300, height: 300)
                .scaleEffect(1.0 / 3.0)
                .frame(width: 100, height: 100)
            
            Text(name)
                .font(.caption)
                .accessibilityLabel("Badge for \(name).")
        }
    }
}

struct HikeBadge_Previews: PreviewProvider {
    static var previews: some View {
        HikeBadge(name: "Preview Testing")
    }
}
```

&emsp;更新 ProfileSummary 以添加多个具有不同色调和获得徽章的原因的徽章。

```swift
...
Divider()

VStack(alignment: .leading) {
    Text("Completed Badges")
        .font(.headline)
    
    ScrollView(.horizontal) {
        HStack {
            HikeBadge(name: "First Hike")
            HikeBadge(name: "Earth Day")
                .hueRotation(Angle(degrees: 90))
            HikeBadge(name: "Tenth Hike")
                .grayscale(0.5)
                .hueRotation(Angle(degrees: 45))
        }
        .padding(.bottom)
    }
}
...
```

&emsp;通过包含 Animating Views and Transitions 中的 HikeView 来完成 profile summary。要使用 hike 数据，还需要添加模型数据环境对象。

```swift
...
@EnvironmentObject var modelData: ModelData
...
Divider()

VStack(alignment: .leading) {
    Text("Recent Hikes")
        .font(.headline)
    
    HikeView(hike: modelData.hikes[0])
}
...
struct ProfileSummary_Previews: PreviewProvider {
    static var previews: some View {
        ProfileSummary(profile: Profile.default)
            .environmentObject(ModelData())
    }
}
```

&emsp;在 CategoryHome.swift 中，使用 toolbar 修饰符将用户 profile button 添加到导航栏，并在用户点击它时显示 ProfileHost 视图。

```swift
...
@State private var showingProfile = false
...
    .toolbar {
        Button {
            showingProfile.toggle()
        } label: {
            Label("User Profile", systemImage: "person.crop.circle")
        }
    }
    .sheet(isPresented: $showingProfile) {
        ProfileHost()
            .environmentObject(modelData)
    }
```

&emsp;添加 listStyle 修饰符以选取更适合内容的列表样式。

```swift
.listStyle(.inset)
```

&emsp;切换到实时预览并尝试点击 profile 按钮以检查 profile summary。

### Add an Edit Mode

&emsp;用户需要在查看或编辑他们的个人资料详细信息之间切换。你将通过向现有 ProfileHost 添加一个 EditButton 来添加一个编辑模式，然后创建一个带有用于编辑单个值的控件的视图。

&emsp;选择 ProfileHost 并将模型数据作为环境对象添加到预览中。尽管该视图不使用具有 @EnvironmentObject 属性的属性，但该视图的子视图 ProfileSummary 却使用了，所以没有修饰符，预览会失败。

&emsp;添加一个 Environment 视图属性，该属性从环境的 \.edit 模式中键关闭。添加一个 Environment 视图属性，该属性关闭环境的 `\.editMode`。SwiftUI 在环境中为可以使用 @Environment 属性包装器访问的值提供存储。访问 editMode 值以读取或写入编辑范围。

```swift
@Environment(\.editMode) var editMode
```

&emsp;创建一个 Edit 按钮，用于打开和关闭环境的 editMode 值。EditButton 控制你在上一步中访问的相同 editMode 环境值。

```swift
...
HStack {
    Spacer()
    EditButton()
}
...
```

&emsp;更新 ModelData 类以包括用户 profile 的实例，该实例即使在用户关闭 profile 视图后仍会保留。

```swift
...
@Published var profile = Profile.default
...
```

&emsp;从环境中读取用户的 profile 数据，以将数据的控制权传递给 profile host。为了避免在确认任何编辑之前（例如在用户输入其名称时）更新全局应用程序状态，编辑视图将对其自身的副本进行操作。

```swift
...
@EnvironmentObject var modelData: ModelData
...
ProfileSummary(profile: modelData.profile)
```

&emsp;添加一个条件视图，该视图显示静态 profile 或 Edit mode 的视图。你可以通过运行实时预览并点击编辑按钮来查看进入编辑模式的效果。目前，编辑模式视图只是一个静态文本字段。

```swift
...
if editMode?.wrappedValue == .inactive {
    ProfileSummary(profile: modelData.profile)
} else {
    Text("Profile Editor")
}
...
```

### Define the Profile Editor

&emsp;用户 profile 编辑器主要由更改 profile 中各个详细信息的不同控件组成。个人资料中的某些项目（如徽章）不可由用户编辑，因此不会显示在编辑器中。为了与 profile summary 保持一致，你将在编辑器中以相同的顺序添加 profile 详细信息。

&emsp;创建一个名为 ProfileEditor 的新视图，并包括与用户 profile 的 draft copy 的绑定。视图中的第一个控件是 TextField，它控制和更新字符串绑定（在本例中为用户选择的显示名称）。在创建文本字段时提供标签和字符串绑定。

```swift
import SwiftUI

struct ProfileEditor: View {
    @Binding var profile: Profile
    
    var body: some View {
        List {
            HStack {
                Text("Username").bold()
                Divider()
                TextField("Username", text: $profile.username)
            }
        }
    }
}

struct ProfileEditor_Previews: PreviewProvider {
    static var previews: some View {
        ProfileEditor(profile: .constant(.default))
    }
}
```

&emsp;更新 ProfileHost 中的条件内容以包括 profile editor 并传递 profile 绑定。现在，当你点击 Edit 时，将显示 ProfileEditor 视图。

```swift
...
if editMode?.wrappedValue == .inactive {
    ProfileSummary(profile: modelData.profile)
} else {
    ProfileEditor(profile: $draftProfile)
}
...
```

&emsp;添加一个切换开关，该开关对应于用户接收有关 landmark-related 事件的通知的首选项。Toggles 是打开或关闭的控件，因此它们非常适合布尔值，如是或否首选项。

```swift
...
Toggle(isOn: $profile.prefersNotifications) {
    Text("Enable Notifications").bold()
}
...
```

&emsp;将 Picker 控件及其标签放在 VStack 中，以使 landmark 照片具有可选的首选季节。

```swift
...
VStack(alignment: .leading, spacing: 20) {
    Text("Seasonal Photo").bold()
    
    Picker("Seasonal Photo", selection: $profile.seasonalPhoto) {
        ForEach(Profile.Season.allCases) { season in
            Text(season.rawValue).tag(season)
        }
    }
    .pickerStyle(.segmented)
}
...
```

&emsp;最后，在 season selector 下方添加一个 DatePicker，使 landmark visitation goal date 可修改。

```swift
...
    var dateRange: ClosedRange<Date> {
        let min = Calendar.current.date(byAdding: .year, value: -1, to: profile.goalDate)!
        let max = Calendar.current.date(byAdding: .year, value: 1, to: profile.goalDate)!
        return min...max
    }
...
DatePicker(selection: $profile.goalDate, in: dateRange, displayedComponents: .date) {
    Text("Goal Date").bold()
}
```

### Delay Edit Propagation

&emsp;要使编辑在用户退出编辑模式之前不会生效，请在编辑期间使用其 profile 的 draft copy，然后仅在用户确认编辑时才将 draft copy 分配给 real copy 。

&emsp;将取消按钮添加到 ProfileHost。与 EditButton 提供的 Done 按钮不同，Cancel 按钮不会将编辑内容应用于其闭包中的实际 profile 数据。

```swift
...
HStack {
    if editMode?.wrappedValue == .active {
        Button("Cancel", role: .cancel) {
            draftProfile = modelData.profile
            editMode?.animation().wrappedValue = .inactive
        }
    }
    
    Spacer()
    EditButton()
}
...
```

&emsp;应用 `onAppear(perform:)` 和 `Disappear(perform:)` 修饰符，用正确的 profile 数据填充编辑器，并在用户点击 Done 按钮时更新持久 profile。否则，旧值将在下次激活编辑模式时显示。

```swift
...
ProfileEditor(profile: $draftProfile)
    .onAppear {
        draftProfile = modelData.profile
    }
    .onDisappear {
        modelData.profile = draftProfile
    }
...
```

## 参考链接
**参考链接:🔗**
+ [Working with UI Controls](https://developer.apple.com/tutorials/swiftui/working-with-ui-controls)



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



