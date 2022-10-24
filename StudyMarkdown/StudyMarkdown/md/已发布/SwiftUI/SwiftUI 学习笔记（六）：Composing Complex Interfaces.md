# SwiftUI 学习笔记（六）：Composing Complex Interfaces.md

&emsp;SwiftUI 官方教程：[SwiftUI Tutorials](https://developer.apple.com/tutorials/swiftui) 仅是几个体现 SwiftUI 简单使用的小 demo 而已，简单易学，循序渐进，先看完可以对 SwiftUI 有一个大概的认知。

## 六：Composing Complex Interfaces

&emsp;[App Design and Layout - Composing Complex Interfaces](https://developer.apple.com/tutorials/swiftui/composing-complex-interfaces) 编写复杂接口。

&emsp;Landmarks 的 category 视图显示水平滚动的地标的垂直滚动列表。在构建此视图并将其连接到现有视图时，你将探索组合视图如何适应不同的设备大小和方向。按照步骤构建此项目，或下载已完成的项目以自行探索。

### Add a Category View

&emsp;你可以通过创建按 category 对 landmarks 进行排序的视图，同时在视图顶部突出显示特色地标，从而提供浏览地标的不同方式。

&emsp;在项目的 Views 文件夹中创建一个 Categories 文件夹，并在新文件夹中创建一个名为 CategoryHome 的自定义视图。

```swift
import SwiftUI

struct CategoryHome: View {
    var body: some View {
        Text("Hello, SwiftUI!")
    }
}

struct CategoryHome_Previews: PreviewProvider {
    static var previews: some View {
        CategoryHome()
    }
}
```

&emsp;添加一个 NavigationView 来承载不同的 categories。你可以使用导航视图以及 NavigationLink 实例和相关修饰符在你的应用程序中构建分层导航结构。

```swift
var body: some View {
    NavigationView {
        Text("Hello, SwiftUI!")
    }
}
```

&emsp;将导航栏的标题设置为 Featured。该视图在顶部展示了一个或多个特色地标。

```swift
var body: some View {
    NavigationView {
        Text("Hello, SwiftUI!")
            .navigationTitle("Featured")
    }
}
```

### Create a Category List

&emsp;类别视图将所有类别显示在垂直列中的单独行中，以便于浏览。为此，你可以合并垂直和水平堆栈，并向列表中添加滚动。

&emsp;首先从 landmarkData.json 文件中读取类别数据。在 Landmark.swift 中，向 Landmark 结构体中添加 Category 枚举和 category 属性。landmarkData.json 文件已经为每个具有三个字符串值之一的地标包含了一个类别值。通过匹配数据文件中的名称，你可以依靠结构的 Codable 一致性来加载数据。

```swift
...
    var category: Category
    enum Category: String, CaseIterable, Codable {
        case lakes = "Lakes"
        case rivers = "Rivers"
        case mountains = "Mountains"
    }
...
```

&emsp;在 ModelData.swift 中，添加一个计算类别字典（将类别名称作为键），并为每个键添加一个关联的地标数组。

```swift
...
    var categories: [String: [Landmark]] {
        Dictionary(grouping: landmarks) {  $0.category.rawValue }
    }
...
```

&emsp;在 CategoryHome.swift 中，创建一个模型数据环境对象。你现在需要访问类别，稍后还需要访问其他地标数据。

```swift
...
    @EnvironmentObject var modelData: ModelData
...

struct CategoryHome_Previews: PreviewProvider {
    static var previews: some View {
        CategoryHome()
            .environmentObject(ModelData())
    }
}
```

&emsp;使用列表显示地标中的类别。Landmark.Category 案例名称标识列表中的每个项目，它在其他类别中必须是唯一的，因为它是一个枚举。

```swift
...
var body: some View {
    NavigationView {
        List {
            ForEach(modelData.categories.keys.sorted(), id: \.self) { key in
                Text(key)
            }
        }
        .navigationTitle("Featured")
    }
}
...
```

### Create a Category Row

&emsp;Landmarks 以水平滚动的行显示每个类别。添加新的视图类型来表示行，然后在新视图中显示该类别的所有地标。重用在 Creating and Combining Views 中创建的 Landmark 视图的某些部分，以创建熟悉的地标预览。

&emsp;定义一个新的自定义视图 CategoryRow。

```swift
import SwiftUI

struct CategoryRow: View {
    var body: some View {
        Text("Hello, SwiftUI!")
    }
}

struct CategoryRow_Previews: PreviewProvider {
    static var previews: some View {
        CategoryRow()
    }
}
```

&emsp;为类别名称和该类别中的项目列表添加属性。

```swift
import SwiftUI

struct CategoryRow: View {
    var categoryName: String
    var items: [Landmark]
    
    var body: some View {
        Text("Hello, SwiftUI!")
    }
}

struct CategoryRow_Previews: PreviewProvider {
    static var landmarks = ModelData().landmarks
    
    static var previews: some View {
        CategoryRow(
            categoryName: landmarks[0].category.rawValue, items: Array(landmarks.prefix(3))
        )
    }
}
```

&emsp;显示类别的名称。

```swift
...
var body: some View {
    Text(categoryName)
        .font(.headline)
}
...
```

&emsp;将类别的 items 放在 HStack 中，并将其与类别名称分组到 VStack 中。

```swift
...
var body: some View {
    VStack(alignment: .leading) {
        Text(categoryName)
            .font(.headline)
        
        HStack(alignment: .top, spacing: 0) {
            ForEach(items) { landmark in
                Text(landmark.name)
            }
        }
    }
}
...
```

&emsp;通过指定高 `frame(width:height:)`，添加填充并将 HStack 包装在滚动视图中，为内容留出一些空间。使用更大的数据样本更新视图预览可以更轻松地确保滚动行为正确。

```swift
...
    var body: some View {
        VStack(alignment: .leading) {
            Text(categoryName)
                .font(.headline)
                .padding(.leading, 15)
                .padding(.top, 15)
            
            ScrollView(.horizontal, showsIndicators: false) {
                HStack(alignment: .top, spacing: 0) {
                    ForEach(items) { landmark in
                        Text(landmark.name)
                    }
                }
            }
            .frame(height: 185)
        }
    }
...
```

&emsp;创建一个名为 CategoryItem 的新自定义视图，其中显示一个地标。

```swift
import SwiftUI

struct CategoryItem: View {
    var landmark: Landmark
    
    var body: some View {
        VStack(alignment: .leading) {
            landmark.image
                .resizable()
                .frame(width: 155, height: 155)
                .cornerRadius(5)
            Text(landmark.name)
                .font(.caption)
        }
        .padding(.leading, 15)
    }
}

struct CategoryItem_PreView: PreviewProvider {
    static var previews: some View {
        CategoryItem(landmark: ModelData().landmarks[0])
    }
}
```

&emsp;在 CategoryRow.swift 中，将包含地标名称的文本替换为新的 CategoryItem 视图。

```swift
...
var body: some View {
    VStack(alignment: .leading) {
        Text(categoryName)
            .font(.headline)
            .padding(.leading, 15)
            .padding(.top, 15)
        
        ScrollView(.horizontal, showsIndicators: false) {
            HStack(alignment: .top, spacing: 0) {
                ForEach(items) { landmark in
                    CategoryItem(landmark: landmark)
                }
            }
        }
        .frame(height: 185)
    }
}
...
```

### Complete the Category View

&emsp;将行和特色图片添加到类别主页。

&emsp;更新 CategoryHome 的正文以将类别信息传递给行类型的实例。

```swift
...
    var body: some View {
        NavigationView {
            List {
                ForEach(modelData.categories.keys.sorted(), id: \.self) { key in
                    CategoryRow(categoryName: key, items: modelData.categories[key]!)
                }
            }
            .navigationTitle("Featured")
        }
    }
...
```

&emsp;接下来，你需要将专题地标添加到视图顶部。为此，你需要从地标数据中获取更多信息。

&emsp;在 Landmark.swift 中，添加新的 isFeatured 属性。与已添加的其他地标属性一样，此布尔值已存在于数据中 — 你只需要声明一个新属性。

```swift
...
    var isFeatured: Bool
...
```

&emsp;在 ModelData.swift 中，添加一个新的计算要素数组，该数组仅包含将 isFeatured 设置为 true 的地标。

```swift
...
    var features: [Landmark] {
        landmarks.filter { $0.isFeatured }
    }
...
```

&emsp;在 CategoryHome.swift 中，将第一个特色地标的图像添加到列表顶部。你将在后面的教程中将此视图转换为交互式轮播。目前，它使用缩放和裁剪的预览图像显示其中一个特色地标。在这两种地标预览中，将边缘插图设置为零，以便内容可以扩展到显示的边缘。

```swift
struct CategoryHome: View {
    @EnvironmentObject var modelData: ModelData
    
    var body: some View {
        NavigationView {
            List {
                modelData.features[0].image
                    .resizable()
                    .scaledToFit()
                    .frame(height: 200)
                    .clipped()
                    .listRowInsets(EdgeInsets())
                
                ForEach(modelData.categories.keys.sorted(), id: \.self) { key in
                    CategoryRow(categoryName: key, items: modelData.categories[key]!)
                }
                .listRowInsets(EdgeInsets())
            }
            .navigationTitle("Featured")
        }
    }
}
```

### Add Navigation Between Sections

&emsp;由于视图中显示了所有不同分类的地标，因此用户需要一种方法来访问应用中的每个部分。使用 navigation 和 presentation API 使类别主页、详细信息视图和收藏夹列表可从选项卡视图中导航。

&emsp;在 CategoryRow.swift 中，使用 NavigationLink 包装现有类别项。

```swift
...
            ScrollView(.horizontal, showsIndicators: false) {
                HStack(alignment: .top, spacing: 0) {
                    ForEach(items) { landmark in
                        NavigationLink {
                            LandmarkDetail(landmark: landmark)
                        } label: {
                            CategoryItem(landmark: landmark)
                        }
                    }
                }
            }
            .frame(height: 185)
...
```

&emsp;固定预览，以便你可以在类别行上看到下一步的效果。通过应用 `renderingMode(_:)` 更改类别项的导航外观和 `foregroundColor(_:)` 修饰符。作为导航链接的标签传递的文本将使用环境的主题色进行呈现，并且图像可能呈现为模板图像。你可以修改任一行为以最适合你的设计。

```swift
...
    var body: some View {
        VStack(alignment: .leading) {
            landmark.image
                .renderingMode(.original)
                .resizable()
                .frame(width: 155, height: 155)
                .cornerRadius(5)
            
            Text(landmark.name)
                .foregroundColor(.primary)
                .font(.caption)
        }
        .padding(.leading, 15)
    }
...
```

&emsp;接下来，你将修改应用程序的主内容视图以显示选项卡视图，该视图允许用户在你刚刚创建的类别视图和现有地标列表之间进行选择。取消固定预览，切换到内容视图并添加要显示的选项卡的枚举。

```swift
    enum Tab {
        case featured
        case list
    }
```

&emsp;为选项卡选择添加一个状态变量，并为其指定默认值。

```swift
    @State private var selection: Tab = .featured
```

&emsp;创建一个 Tab View，用于包装地标列表以及新的 CategoryHome。`tag(_:)` 每个视图上的修饰符都与选择属性可以采用的可能值之一匹配，以便 TabView 可以协调当用户在用户界面中进行选择时要显示的视图。为每个 tab 添加一个标签。

```swift
...
    var body: some View {
        TabView(selection: $selection) {
            CategoryHome()
                .tabItem({
                    Label("Featured", systemImage: "star")
                })
                .tag(Tab.featured)
            
            LandmarkList()
                .tabItem({
                    Label("List", systemImage: "list.bullet")
                })
                .tag(Tab.list)
        }
    }
...
```

&emsp;开始实时预览并尝试新的导航。

## 参考链接
**参考链接:🔗**
+ [Composing Complex Interfaces](https://developer.apple.com/tutorials/swiftui/composing-complex-interfaces)
