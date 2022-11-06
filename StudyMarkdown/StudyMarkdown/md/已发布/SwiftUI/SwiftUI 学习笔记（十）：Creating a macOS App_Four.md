# SwiftUI 学习笔记（十）：Creating a macOS App

&emsp;SwiftUI 官方教程：[SwiftUI Tutorials](https://developer.apple.com/tutorials/swiftui) 仅是几个体现 SwiftUI 简单使用的小 demo 而已，简单易学，循序渐进，先看完可以对 SwiftUI 有一个大概的认知。

## 十：Creating a macOS App

&emsp;[Framework Integration - Creating a macOS App](https://developer.apple.com/tutorials/swiftui/creating-a-macos-app) 创建一个 macOS App。

&emsp;在为 watchOS 创建 Landmarks app 的一个版本后，是时候将目光投向更大的目标了：将 Landmarks 带到 Mac。你将以到目前为止所学的所有内容为基础，完善为 iOS、watchOS 和 macOS 构建 SwiftUI 应用程序的体验。

&emsp;首先，你将向 project 添加 macOS target，然后重用之前创建的视图和数据。奠定基础后，你将添加一些针对 macOS 量身定制的新视图，并修改其他视图以更好地跨平台工作。

&emsp;按照步骤生成此项目，或下载完成的项目自行探索。

### Add a macOS Target to the Project

&emsp;首先将 macOS target 添加到 project。Xcode 为 macOS app 添加了一组新的 starter files，以及构建和运行 app 所需的 scheme。然后，你将一些现有文件添加到新 target。

&emsp;为了能够预览和运行该应用程序，请确保你的 Mac 运行的是 macOS Monterey 或更高版本。

&emsp;选择 File > New > Target，当 template sheet 出现时，选择 macOS 选项卡，选择 App template，然后单击下一步。此 template 将新的 macOS app target 添加到 project 中。

&emsp;在 sheet 中，输入 MacLandmarks 作为 Product Name。将 interface 设置为 SwiftUI，将 life cycle 设置为 SwiftUI App，将 language 设置为 Swift，然后单击 Finish。

&emsp;将 scheme 设置为 MacLandmarks > My Mac。通过将 scheme 设置为 My Mac，你可以预览、构建和运行 macOS app。在学习本教程时，你将使用其他 schemes 来关注其他 targets 如何响应共中的更改。

&emsp;在 MacLandmarks 文件夹中，选择 ContentView.swift，打开 Canvas，然后单击 Resume 以查看 preview。SwiftUI 提供默认主视图及其预览提供程序，就像 iOS 应用程序一样，使你能够预览应用程序的主窗口。

&emsp;在 Project navigator 中，从 MacLandmarks 文件夹中删除 MacLandmarksApp.swift 文件；When asked，直接选择 Move to Trash。

&emsp;与 watchOS 应用程序一样，你不需要默认的应用程序结构，因为你将重复使用已有的应用程序结构。

&emsp;接下来，你将与 macOS target 共享 iOS app 中的视图、模型和资源文件（view, model, and resource files）。

&emsp;在 Project navigator 中，按住 Command 并单击以选择以下文件：LandmarksApp.swift、LandmarkList.swift、LandmarkRow.swift、CircleImage.swift、MapView.swift 和 FavoriteButton.swift。

&emsp;其中第一个是 shared app definition。其他是适用于 macOS 的视图。

&emsp;继续按住 Command 单击以选择 Model 和 Resources 文件夹以及 Asset.xcassets 中的所有项目。

&emsp;这些 items 定义应用的 data model and resources。

&emsp;在文件检查器中，将 MacLandmarks 添加到所选文件的 Target Membership。

&emsp;添加一个 macOS 应用程序图标 set 以匹配其他 targets 的图标 set。

&emsp;选择 MacLandmarks 文件夹中的 Assets.xcasset 文件并删除空的 AppIcon 项。你将在下一步中替换它。

&emsp;将 AppIcon.appiconset 文件夹从下载的项目的 Resources 文件夹中拖到 MacLandmark 的 catalog 目录中。

&emsp;在 MacLandmarks 文件夹的 ContentView 中，将 LandmarkList 添加为 top level view，并限制 frame 大小。

&emsp;preView 不能正常构建，因为 LandmarkList 使用 LandmarkDetail，但你尚未为 macOS 应用程序定义 detail view。你将在下一节中解决这个问题。

```swift
...
struct ContentView: View {
    var body: some View {
        LandmarkList()
            .frame(minWidth: 700, minHeight: 300)
    }
}
...
```

### Create a macOS Detail View

&emsp;detail view 显示有关所选 landmark 的信息。你为 iOS 应用创建了这样的视图，但不同的平台需要不同的数据呈现方法。有时，你可以通过小的调整或条件编译跨平台重用视图，但 detail view 对于 macOS 来说差异很大，因此最好创建专用视图。你将复制 iOS detail view 作为起点，然后对其进行修改以适应 macOS 的较大显示。

&emsp;在 macOS 的 MacLandmarks 文件夹中创建一个名为 LandmarkDetail 的新自定义视图。你现在有三个名为 LandmarkDetail.swift 的文件。每个在视图层次结构中具有相同的用途，但提供针对特定平台定制的体验。

&emsp;将 iOS detail view 内容复制到 macOS detail view 中。预览失败是因为 `navigationBarTitleDisplayMode(_:)` 方法在 macOS 中不可用。

&emsp;删除 `navigationBarTitleDisplayMode(_:)` modifier，然后将 frame modifier 添加到预览中，以便你可以看到更多内容。

&emsp;你将在接下来的几个步骤中所做的更改可改进 Mac 更大显示屏的布局。

&emsp;将 landmark.park 和 landmark.state 的 Text 所处的 HStack 更改为具有 leading alignment 的 VStack，并移除 Spacer。

```swift
...
                VStack(alignment: .leading) {
                    Text(landmark.park)
                    Text(landmark.state)
                }
                .font(.subheadline)
                .foregroundColor(.secondary)
...
```

&emsp;将 MapView 下方的所有内容都包含在 VStack 中，然后将 CircleImage 和 header 的其余部分放在 HStack 中。

&emsp;从 CircleImage 中删除 offset，而是对整个 VStack 应用较小的 offset。

&emsp;向 image 添加一个 resizable() modifier，并将 CircleImage 限制为更小。

&emsp;将 ScrollView 限制为最大宽度。当用户使窗口非常宽时，这会提高可读性。

&emsp;更改 FavoriteButton 以使用 plain 按钮样式。在这里使用 plain 样式使按钮看起来更像 iOS equivalent。

&emsp;mac 更大的显示屏为你提供更多空间来实现附加功能。

&emsp;在 ZStack 中添加 "Open in Maps" 按钮，使其显示在地图右上角的顶部。确保包含 MapKit 以便能够创建你发送到地图的 MKMapItem。

```swift
import SwiftUI
import MapKit

struct LandmarkDetail: View {
    @EnvironmentObject var modelData: ModelData
    var landmark: Landmark
    
    var landmarkIndex: Int {
        modelData.landmarks.firstIndex { $0.id == landmark.id }!
    }
    
    var body: some View {
        ScrollView {
            ZStack(alignment: Alignment(horizontal: .trailing, vertical: .top)) {
                MapView(coordinate: landmark.locationCoordinate)
                    .ignoresSafeArea(edges: .top)
                    .frame(height: 300)
                
                Button("Open in Maps") {
                    let destination = MKMapItem(placemark: MKPlacemark(coordinate: landmark.locationCoordinate))
                    destination.name = landmark.name
                    destination.openInMaps()
                }
                .padding()
            }
            
            VStack(alignment: .leading, spacing: 20) {
                HStack(spacing: 24) {
                    CircleImage(image: landmark.image.resizable())
                        .frame(width: 160, height: 160)
                    
                    VStack(alignment: .leading) {
                        HStack {
                            Text(landmark.name)
                                .font(.title)
                            FavoriteButton(isSet: $modelData.landmarks[landmarkIndex].isFavorite)
                                .buttonStyle(.plain)
                        }
                        
                        VStack(alignment: .leading) {
                            Text(landmark.park)
                            Text(landmark.state)
                        }
                        .font(.subheadline)
                        .foregroundColor(.secondary)
                    }
                }
                
                Divider()
                
                Text("About \(landmark.name)")
                    .font(.title2)
                Text(landmark.description)
            }
            .padding()
            .frame(maxWidth: 700)
            .offset(y: -50)
        }
        .navigationTitle(landmark.name)
    }
}

struct LandmarkDetail_Previews: PreviewProvider {
    static let modelData = ModelData()
    
    static var previews: some View {
        LandmarkDetail(landmark: modelData.landmarks[0])
            .environmentObject(modelData)
            .frame(width: 850, height: 700)
    }
}
```

### Update the Row View

&emsp;shared 的 LandmarkRow 视图在 macOS 中工作，但鉴于新的视觉环境，值得重新审视以寻找改进。由于所有三个平台都使用此视图，因此你需要注意所做的任何更改都适用于所有平台。

&emsp;在修改 row 之前，请设置列表的预览，因为你将进行的更改取决于行在上下文中的外观。

&emsp;打开 LandmarkList.swift 并添加最小宽度。这改进了预览，但也确保了列表永远不会因为用户调整 macOS 窗口大小而变得太小。

&emsp;固定列表视图预览，以便你可以在进行更改时查看 row 在上下文中的外观。

&emsp;打开 LandmarkRow.swift 并向图像添加 corner radius 以获得更精细的外观。

&emsp;将 landmark name 包装在 VStack 中，并将 park 添加为辅助信息。

```swift
struct LandmarkRow: View {
    var landmark: Landmark

    var body: some View {
        HStack {
            landmark.image
                .resizable()
                .frame(width: 50, height: 50)
                .cornerRadius(5)
            
            VStack(alignment: .leading) {
                Text(landmark.name)
                    .bold()
                
                Text(landmark.park)
                    .font(.caption)
                    .foregroundColor(.secondary)
            }
            
            Spacer()

            if landmark.isFavorite {
                Image(systemName: "star.fill")
                    .imageScale(.medium)
                    .foregroundColor(.yellow)
            }
        }
    }
}
```

&emsp;在 row 内容周围添加 vertical padding，使每行有更多的喘息空间（breathing room）。

&emsp;这些更新改进了 macOS 的外观，但你还需要考虑使用该列表的其他平台。首先考虑 watchOS。

&emsp;选择 WatchLandmarks target 以查看列表的 watchOS 预览。

&emsp;最小行宽在这里不合适。由于你将在下一节中对列表进行此更改和其他更改，因此最好的解决方案是创建一个省略宽度约束的特定于手表的列表。

&emsp;将一个名为 LandmarkList.swift 的新 SwiftUI 视图添加到名为 WatchLandmarks Extension 文件夹中，该文件夹仅面向 WatchLandmarks Extension，并删除旧文件的 WatchLandmarks Extension target membership。

&emsp;将旧 LandmarkList 的内容复制到新 LandmarkList 中，但不使用 frame modifier。content 现在具有正确的宽度，但每一行都有太多信息。

&emsp;返回到 LandmarkRow 并添加 #if 条件，以防止 secondary 文本出现在 watchOS 版本中。

&emsp;对于 row，使用条件编译是合适的，因为差异很小。

```swift
...
                #if !os(watchOS)
                Text(landmark.park)
                    .font(.caption)
                    .foregroundColor(.secondary)
                #endif
...
```

&emsp;最后，考虑一下你的更改如何适用于 iOS。

&emsp;选择 Landmarks build target 以查看 iOS 的列表外观。这些更改适用于 iOS，因此无需为该平台进行任何更新。

### Update the List View

&emsp;与 LandmarkRow 一样，LandmarkList 已经在 macOS 上运行，但可以使用改进。例如，你可以将仅显示收藏夹的切换开关移至工具栏中的菜单，在该菜单中可以通过其他过滤控件进行连接。

&emsp;你将所做的更改适用于 macOS 和 iOS，但很难在 watchOS 上适应。幸运的是，在上一节中，你已经将列表拆分为一个单独的 watchOS 文件。

&emsp;返回到 MacLandmarks scheme，在以 iOS 和 macOS 为 target 的 LandmarkList 文件中，在新的 toolbar modifier 中添加一个包含菜单的 ToolbarItem。

&emsp;在运行应用之前，你将无法看到 toolbar 更新。将 favorites Toggle 移动到 menu 中。

&emsp;有了更多可用空间，你将添加一个用于按 category 过滤地标列表的新控件。

&emsp;添加一个 FilterCategory 枚举来描述 filter 状态。将 case strings 与 Landmark structure 中的 Category enumeration 匹配，以便可以比较它们，and include an all case to turn filtering off。

```swift
...
    enum FilterCategory: String, CaseIterable, Identifiable {
        case all = "All"
        case lakes = "Lakes"
        case rivers = "Rivers"
        case mountains = "Mountains"
        
        var id: FilterCategory { self }
    }
...
    @State private var filter = FilterCategory.all
```

&emsp;添加 filter 状态变量，默认为 all case。

&emsp;通过在列表视图中存储 filter 状态，用户可以打开多个列表视图窗口，每个窗口都有自己的 filter 设置，以便能够以不同的方式查看数据。

&emsp;更新 filteredLandmarks 以考虑新的 filter 设置，并结合给定 landmark 的类别。

&emsp;将 Picker 添加到 menu 以设置 filter 类别。由于 filter 只有几个项，因此使用 inline picker style 使它们一起显示。

```swift
...
                    Menu {
                        Picker("Category", selection: $filter) {
                            ForEach(FilterCategory.allCases) { category in
                                Text(category.rawValue).tag(category)
                            }
                        }
                        .pickerStyle(.inline)
                        
                        Toggle(isOn: $showFavoritesOnly) {
                            Text("Favorites only")
                        }
                    } label: {
                        Label("Filter", systemImage: "slider.horizontal.3")
                    }
...
```

&emsp;更新 navigation title 以匹配 filter 的状态。此更改在 iOS 应用中非常有用。

&emsp;将第二个子视图添加到 NavigationView，作为 wide layouts 中第二个视图的占位符。添加第二个子视图会自动将列表转换为使用 sidebar list style。

&emsp;运行 macOS target 并查看 menu 的运行方式。选择 Landmarks build target，并使用实时预览查看新的 filtering 是否也适用于 iOS。

### Add a Built-in Menu Command

&emsp;当你使用 SwiftUI life cycle 创建应用程序时，系统会自动创建一个包含常用 items 的菜单，例如用于关闭最前面的窗口或退出应用程序的 items。 SwiftUI 允许你添加其他具有内置行为的常用 commands，以及完全自定义的 commands。在本节中，你将添加一个系统提供的 command ，该 command 允许用户切换侧边栏，以便在将其拖动关闭后将其取回。

&emsp;返回到 MacLandmarks Target，运行 macOS 应用程序，然后将列表和详细视图之间的分隔符一直拖到左侧。当你松开鼠标按钮时，列表会消失，无法恢复。你将添加一个 command 来解决此问题。

&emsp;添加一个名为 LandmarkCommand.swift 的新文件，并将其 Target 设置为同时包含 macOS 和 iOS。你还以 iOS 为 Target，因为共享的 LandmarkList 最终将取决于你在此文件中定义的某些类型。

&emsp;Import SwiftUI 并添加符合 Commands 协议的 LandmarkCommands structure，该 structure 具有名为 body 的计算属性。

&emsp;与 View structure 一样，Commands structure 需要使用 builder semantics 的 body 的计算属性，但使用 commands 而不是 views 除外。

&emsp;将 SidebarCommands command 添加到 body 中。此 built-in command set 包括用于切换边栏的命令。

```swift
import SwiftUI

struct LandmarkCommands: Commands {
    var body: some Commands {
        SidebarCommands()
    }
}
```

&emsp;要在 App 中使用 commands，你必须将它们应用于 scene，接下来将执行此操作。

&emsp;打开 LandmarksApp.swift 文件，然后使用 commands(content:) scene modifier 应用 LandmarkCommands。Scene modifiers 的工作方式与 view modifiers 类似，不同之处在于将它们应用于 scenes 而不是 views。

```swift
...
.commands {
    LandmarkCommands()
}
...
```

&emsp;再次运行 macOS App，并且你可以使用 View > Toggle Sidebar 恢复列表视图。遗憾的是，watchOS 应用无法构建，因为 Commands 没有 watchOS 可用性。接下来你将修复此问题。

```swift
...
#if !os(watchOS)
.commands {
    LandmarkCommands()
}
#endif
```

&emsp;在 commands modifier 周围添加一个条件，以在 watchOS 应用中省略它。保存后 watchOS App 将再次构建。

### Add a Custom Menu Command

&emsp;在上一节中，你添加了一个 built-in menu command set。在本节中，你将添加一个 custom command，用于切换当前所选 landmark 的收藏状态。要了解当前选择了哪个 landmark，你将使用 focused binding。

&emsp;在 LandmarkCommands 中，使用名为 SelectedLandmarkKey 的自定义键，使用 selectedLandmark value 扩展 FocusedValues structure。

&emsp;定义 focused values 的模式类似于定义 new Environment values 的模式：使用 private key 在 system-defined 的 FocusedValues structure 上读取和写入自定义属性。

```swift
import SwiftUI

struct LandmarkCommands: Commands {
    var body: some Commands {
        SidebarCommands()
    }
}

private struct SelectedLandmarkKey: FocusedValueKey {
    typealias Value = Binding<Landmark>
}

extension FocusedValues {
    var selectedLandmark: Binding<Landmark>? {
        get { self[SelectedLandmarkKey.self] }
        set { self[SelectedLandmarkKey.self] = newValue }
    }
}
```

&emsp;添加 @FocusedBinding 属性包装器以跟踪当前选定的 landmark。你正在读取此处的值。稍后将在用户进行选择的 list view 中进行设置。

```swift
@FocusedBinding(\.selectedLandmark) var selectedLandmark
```

&emsp;将一个名为 Landmarks 的新 CommandMenu 添加到 commands 中。接下来，你将定义菜单的内容。

```swift
CommandMenu("Landmark") {
}
```

&emsp;向 menu 中添加一个按钮，用于切换所选 landmark 的收藏状态，其外观会根据当前选定的 landmark 及其状态而变化。

```swift
CommandMenu("Landmark") {
    Button("\(selectedLandmark?.isFavorite == true ? "Remove" : "Mark") as Favorite") {
        selectedLandmark?.isFavorite.toggle()
    }
    .disabled(selectedLandmark == nil)
}
```

&emsp;使用 `keyboardShortcut(_:modifiers:)` modifier 为 menu item 添加 keyboard shortcut。SwiftUI 会自动在菜单中显示 keyboard shortcut。

```swift
.keyboardShortcut("f", modifiers: [.shift, .option])
```

&emsp;菜单现在包含新 command，但你需要设置 selectedLandmark focused binding 才能使其正常工作。在 LandmarkList.swift 中，为所选 landmark 添加一个状态变量，并添加一个指示所选 landmark 索引的计算属性。

```swift
@State private var selectedLandmark: Landmark?

...
var index: Int? {
    modelData.landmarks.firstIndex(where: { $0.id == selectedLandmark?.id } )
}
```

&emsp;使用与 selected value 的 binding 初始化 List，并向 navigation link 添加标记。

```swift
List(selection: $selectedLandmark) {
    ...
    .tag(landmark)
}
```

&emsp;添加 `focusedValue(_:_:)` modifier 到 NavigationView，提供 landmarks array 中的值 binding。

```swift
.focusedValue(\.selectedLandmark, $modelData.landmarks[index ?? 0])
```

&emsp;运行 macOS 应用程序并尝试新菜单项。

### Add Preferences with a Settings Scene

&emsp;用户希望能够使用标准的 Preferences menu item 调整 macOS 应用程序的设置。你将通过添加 Settings scene 来向 MacLandmarks 添加 preferences。scene’s view 定义 preferences 窗口的内容，你将使用该窗口控制 MapView 的初始缩放级别。将值传达给 MapView，并使用 @AppStorage 属性包装器将其永久存储（在本地持久化）。

&emsp;首先，你将在 MapView 中添加一个控件，该控件将初始缩放设置为以下三个级别之一：近、中或远。在 MapView.swift 中，添加缩放枚举以表征缩放级别。

```swift
enum Zoom: String, CaseIterable, Identifiable {
    case near = "Near"
    case medium = "Medium"
    case far = "Far"
    
    var id: Zoom {
        return self
    }
}
```

&emsp;添加一个名为 zoom 的 @AppStorage 属性，该属性默认采用中等缩放级别。使用唯一标识参数的存储键，就像在 UserDefaults 中存储项目时一样，因为这是 SwiftUI 所依赖的底层机制。

```swift
@AppStorage("MapView.zoom")
private var zoom: Zoom = .medium
```

&emsp;将用于构造区域属性的经度和纬度增量更改为取决于缩放的值。

```swift
var delta: CLLocationDegrees {
    switch zoom {
    case .near: return 0.02
    case .medium: return 0.2
    case .far: return 2
    }
}

...
span: MKCoordinateSpan(latitudeDelta: delta, longitudeDelta: delta)
```

&emsp;若要确保 SwiftUI 在 delta 更改时刷新地图，你必须更改计算和应用 region 的方式。将 region 状态变量、setRegion 方法和地图的 onAppear 修饰符替换为作为 constant binding 传递给地图初始值设定项的 computed region property。

```swift
Map(coordinateRegion: .constant(region))

...
var region: MKCoordianteRegion {
    MKCoordinateRegion(
    ...
    )
}
```

&emsp;接下来，你将创建一个控制存储的缩放值的 Settings scene。创建一个名为 LandmarkSettings 的新 SwiftUI 视图，该视图仅面向 macOS 应用。

```swift
import SwiftUI

struct LandmarkSettings: View {
    var body: some View {
        Text("Hello, World!")
    }
}

struct LandmarkSettings_Previews: PreviewProvider {
    static var previews: some View {
        LandmarkSettings()
    }
}
```

&emsp;添加一个 @AppStorage 属性，该属性使用与你在 map view 中使用的相同的 key。

```swift
@AppStorage("MapView.zoom")
private var zoom: MapView.Zoom = .medium
```

&emap;添加一个通过 binding 控制缩放值的 Picker。通常使用 Form 在 settings view 中排列控件。

```swift
var body: some View {
    Form {
        Picker("Map Zoom:", selection: $zoom) {
            ForEach(MapView.Zoom.allCases) { level in
                Text(level.rawValue)
            }
        }
        .pickerStyle(.inline)
    }
    .frame(width: 300)
    .navigationTitle("Landmark Settings")
    .padding(80)
}
```

&emsp;在 LandmarksApp.swift 中，将 Settings scene 添加到你的应用程序中，但仅适用于 macOS。

```swift
#if os(macOS)
Settings {
    LandmarkSetting()
}
#endif
```

&emsp;运行应用并尝试设置 preferences。请注意，只要你更改缩放级别，地图就会刷新。

## 参考链接
**参考链接:🔗**
+ [Creating a macOS App](https://developer.apple.com/tutorials/swiftui/creating-a-macos-app)












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





