# SwiftUI 学习笔记（三）：Handling User Input

&emsp;SwiftUI 官方教程：[SwiftUI Tutorials](https://developer.apple.com/tutorials/swiftui) 仅是几个体现 SwiftUI 简单使用的小 demo 而已，简单易学，循序渐进，先看完可以对 SwiftUI 有一个大概的认知。

## 三：Handling User Input

&emsp;[SwiftUI Essentials - Handling User Input](https://developer.apple.com/tutorials/swiftui/handling-user-input) 处理用户输入。

&emsp;在 Landmarks 应用中，用户可以标记其收藏的地点，并筛选列表以仅显示其收藏的地标。若要创建此功能，首先要向列表中添加一个开关，以便用户可以只关注其收藏列表，然后在地标详情页面添加一个星形按钮，用户点击该按钮可将地标标记为收藏。（在列表第一行右边加一个开关，点击来回切换显示全部地标或者已收藏的地标）

### Mark the User’s Favorite Landmarks

&emsp;标记用户收藏的地标（Landmarks），首先增强列表（已收藏的在 LandmarkRow 上添加黄色的星标记），以便一目了然地向用户显示他们的收藏。向 Landmark 结构添加一个属性，以将地标的初始状态作为收藏进行读取，然后向每个显示收藏地标的 LandmarkRow 添加一颗星号。

&emsp;首先修改数据源模型，为 Landmark 结构体添加一个 `var isFavorite: Bool` 成员变量，记录对应的地标是否收藏了。

```swift
struct Landmark: Hashable, Codable, Identifiable {
    ...
    
    var isFavorite: Bool

    ...
}
```

&emsp;然后选中 LandmarkRow.swift 我们为收藏的地标所在的行右边添加一颗星。

```swift
struct LandmarkRow: View {
    var landmark: Landmark

    var body: some View {
        HStack {
            landmark.image
                .resizable()
                .frame(width: 50, height: 50)
            Text(landmark.name)

            Spacer()
            
            // 这里根据数据源判断是否添加一个 Image 视图
            if landmark.isFavorite {
                Image(systemName: "star.fill")
                    // Because system images are vector based, you can change their color with the foregroundColor(_:) modifier.
                    .foregroundColor(.yellow)
            }
        }
    }
}
```

&emsp;仅当当前 LandmarkRow 的 landmark 成员变量的 isFavorite 为 true 时才添加一个 Image 视图，且这里使用了系统提供的图片（star 图标）：`Image(systemName: "star.fill")`， 它默认是黑色的，且是基于矢量的，所以我们可以通过 `foregroundColor(_:)` 来设置它的颜色，例如这里我们把它修改为黄色。

### Filter the List View

&emsp;可以自定义列表视图（LandmarkList），使其显示所有地标，或仅显示用户的收藏列表。为此，我们需要向 LandmarkList 添加一些状态。状态（State）是一个值或一组值，可以随时间而变化，并且会影响视图的行为、内容或布局，使用具有 @State 修饰的属性向视图添加状态。

&emsp;@State 关键字还记得吗？使用 @State 修饰某个属性后，SwiftUI 将会把该属性存储到一个特殊的内存区域内，并且这个区域和 View struct 是隔离的。当 @State 修饰的属性的值发生变化后，SwiftUI 会根据该属性重新绘制视图。

&emsp;为 struct LandmarkList 添加一个名为 `showFavoritesOnly` 的 @State 属性，其初始值设置为 false，那么后续当 `showFavoritesOnly` 值发生变化时，LandmarkList 就会进行 "刷新"。由于使用 State 属性来保存特定于视图及其子视图的信息，因此始终将 State 创建为 private。

&emsp;为 struct LandmarkList 添加一个名为 `filteredLandmarks` 的计算属性，通过检查 `showFavoritesOnly` 属性和 `landmarks` 全局数组中每个 `landmark` 的 `isFavorite` 属性来筛选出收藏的地标列表。

&emsp;当 showFavoritesOnly 为 false（表达的含义是不要仅显示收藏的地标）时 (!showFavoritesOnly || landmark.isFavorite) 必为真，此时过滤出来的 filteredLandmarks 数组即包含 landmarks 数组中所有元素，所以即使置换 LandmarkList 的数据源，此时显示的仍是所有的地标元素，当把 showFavoritesOnly 置为 true 时，表示仅显示收藏的地标，此时从 landmarks 数组中过滤出 isFavorite 值为 true 的 landmark（地标），此时置换数据源后 LandmarkList 仅显示收藏的地标。

```swift
struct LandmarkList: View {
    @State private var showFavoritesOnly = false
    
    // 通过检查 showFavoritesOnly 属性和每个 landmark.isFavorite 值来计算 LandmarkList 列表的筛选版本（仅显示收藏的地标）
    var filteredLandmarks: [Landmark] {
        landmarks.filter { landmark in
            (!showFavoritesOnly || landmark.isFavorite)
        }
    }
    
    ...
}
```

&emsp;使用列表中地标列表的过滤版本。把 LandmarkList 的数据源从 landmarks 置换为 filteredLandmarks：

```swift
struct LandmarkList: View {
    ...
    
    var body: some View {
        NavigationView {
            // 把 landmarks 替换为 filteredLandmarks
            List(filteredLandmarks) { landmark in
                NavigationLink {
                    LandmarkDetail(landmark: landmark)
                } label: {
                    LandmarkRow(landmark: landmark)
                }
            }
            .navigationTitle("Landmarks")
        }
    }
}
```

&emsp;然后把 showFavoritesOnly 修改为 true，表示列表仅显示收藏的地标，运行项目，发现列表仅显示 3 个右边有一个黄色星号的收藏的地标。（这里我们多看一眼 List(filteredLandmarks) { ... } 的构建方式，它是直接把数据传递给 List，后续会转变为另一种方式）

### Add a Control to Toggle the State（添加控件以切换状态）

&emsp;若要授予用户对列表筛选器的控制权，需要添加一个可以更改 showFavoritesOnly 值的控件。可以通过将 binding 传递到切换控件来执行此操作。binding 充当对可变状态的引用。当用户点击开关从关到开，然后再次关闭时，控件将使用 binding 相应地更新视图的状态。

&emsp;创建嵌套的 ForEach 组以将 landmark 转换为行（LandmarkRow）。若要在列表中合并静态视图和动态视图，或者要组合两组或更多不同的动态视图组，请使用 ForEach 类型，而不是将数据收集传递到 List。看到这里可能一脸懵逼这是在说什么！其实直白的理解就是在 List 列表中如果我们要显示不同的行怎么操作呢？类比在 UIKit 中我们使用 UITableView 时，第一个行我们显示一个类型 1 的 Cell 第二行我们想显示类型 2 的 Cell，上面示例中我们使用 `List(filteredLandmarks) { ... }` 的方式构建 List，List 的每一行都是一个 LandmarkRow，如果我们想在第一个行添加一个静态行，一个其它类型的行，那么我们可以切换到 `List { ... }` 的形式，在其中插入任意自己想要显示的行。

```swift
struct LandmarkList: View {
    ...
    
    var body: some View {
        NavigationView {
//            List(filteredLandmarks) { landmark in
//                NavigationLink {
//                    LandmarkDetail(landmark: landmark)
//                } label: {
//                    LandmarkRow(landmark: landmark)
//                }
//            }
            
            List {
                ForEach(filteredLandmarks) { landmark in
                    NavigationLink {
                        LandmarkDetail(landmark: landmark)
                    } label: {
                        LandmarkRow(landmark: landmark)
                    }
                }
            }
            .navigationTitle("Landmarks")
        }
    }
}
```

&emsp;添加一个切换视图（Toggle）作为 List 的第一个子视图（第一行），传递 binding 以显示 FavoritesOnly。使用 $ 前缀访问与状态变量或其属性之一的绑定。

```swift
struct LandmarkList: View {
    ...
    
    var body: some View {
        NavigationView {
//            List(filteredLandmarks) { landmark in
//                NavigationLink {
//                    LandmarkDetail(landmark: landmark)
//                } label: {
//                    LandmarkRow(landmark: landmark)
//                }
//            }
            
            List {
                // List 的第一行显示这个 Toggle，并且与 showFavoritesOnly 双向绑定在一起（忘记 @State 和 $ 前缀的话，可以去复习下第一篇）
                Toggle(isOn: $showFavoritesOnly) {
                    Text("Favorites only")
                }
                
                // List 的其它行依然显示 LandmarkRow
                ForEach(filteredLandmarks) { landmark in
                    NavigationLink {
                        LandmarkDetail(landmark: landmark)
                    } label: {
                        LandmarkRow(landmark: landmark)
                    }
                }
            }
            .navigationTitle("Landmarks")
        }
    }
}
```

&emsp;然后运行项目，点击 Toggle 切换显示全部地标/收藏的地标。 

### Use an Observable Object for Storage（使用可观察对象进行存储）

&emsp;要准备让用户控制哪些特定地标（Landmark）是被收藏的，首先将地标数据存储在可观察对象（observable object）中。

&emsp;可观察对象（observable object）是数据的自定义对象，可以从 SwiftUI 环境（SwiftUI’s environment）存储中绑定到视图。SwiftUI 监视对可观察对象（observable objects）的任何可能影响视图的更改，并在更改后显示视图的正确版本（刷新页面）。

&emsp;从 Combine 框架声明符合 ObservableObject 协议的新的模型类型（`class ModelData: ObservableObject { ... }`）。SwiftUI 订阅可观察对象（observable object），并在数据更改时更新需要刷新的任何视图。

```swift
import Combine

final class ModelData: ObservableObject {
    
}
```

&emsp;然后把 landmarks 全局数组移动到 ModelData 中：

```swift
import Combine

final class ModelData: ObservableObject {
    var landmarks: [Landmark] = load("landmarkData.json")
}
```

&emsp;可观察对象需要发布对其数据的任何更改，以便其订阅者可以获取更改。将 @Published 属性添加到地标数组。

```swift
    @Published var landmarks: [Landmark] = load("landmarkData.json")
```

### Adopt the Model Object in Your Views（在视图中采用 ModelData 对象）

&emsp;创建 ModelData 对象后，需要更新视图以将其用作应用的数据存储。

&emsp;在 LandmarkList.swift 中，向视图添加 `@EnvironmentObject` 属性声明和 `environmentObject(_:)`修改符到 LandmarkList_Previews 中。

```swift
struct LandmarkList: View {
    @EnvironmentObject var modelData: ModelData
    @State private var showFavoritesOnly = false
    ...
}

struct LandmarkList_Previews: PreviewProvider {
    static var previews: some View {
        LandmarkList()
            .environmentObject(ModelData())
    }
}
```

&emsp;在筛选地标时，使用 modelData.landmark 作为数据。

```swift
...
    var filteredLandmarks: [Landmark] {
        modelData.landmarks.filter { landmark in
            (!showFavoritesOnly || landmark.isFavorite)
        }
    }
...
```

&emsp;更新 LandmarkDetail 的预览视图以使用环境中的 ModelData 对象。

```swift
struct LandmarkDetail_Previews: PreviewProvider {
    static var previews: some View {
        LandmarkDetail(landmark: ModelData().landmarks[0])
    }
}
```

&emsp;更新 LandmarkRow 的预览视图以使用 ModelData 对象。

```swift
struct LandmarkRow_Previews: PreviewProvider {
    // 静态变量 landmarks
    static var landmarks = ModelData().landmarks
    
    static var previews: some View {
        Group {
            LandmarkRow(landmark: landmarks[0])
            LandmarkRow(landmark: landmarks[1])
        }
        .previewLayout(.fixed(width: 300, height: 70))
    }
}
```

&emsp;更新 ContentView 的预览视图以将模型对象添加到环境中，从而使该对象可用于任何子视图。如果任何子视图需要环境中的模型对象，但正在预览的视图没有 `environmentObject(_:)`， 则预览将失败修饰语。

```swift
struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
            .environmentObject(ModelData())
    }
}
```

&emsp;接下来，当你在模拟器或设备中运行应用时，将更新应用实例，以将模型对象放在环境中。更新 LandmarksApp 以创建一个模型实例，并使用 `environmentObject(_:)` 修饰符将其提供给 ContentView。

&emsp;使用 @StateObject 特性在应用的生命周期内仅初始化给定属性的模型对象一次。当在应用程序实例中使用该属性时（如下所示）以及在视图中使用它时，情况确实如此。

```swift
@main
struct LandmarksApp: App {
    @StateObject private var modelData = ModelData()
    
    var body: some Scene {
        WindowGroup {
            ContentView()
                .environmentObject(modelData)
        }
    }
}
```

### Create a Favorite Button for Each Landmark

&emsp;Landmarks 应用现在可以在地标的筛选视图和未过滤视图之间切换，但收藏地标列表仍然是硬编码的。若要允许用户添加和删除收藏地标，需要将收藏按钮添加到地标详细信息视图（LandmarkDetail）中。

&emsp;首先创建一个可重用的收藏按钮（FavoriteButton）。

```swift
import SwiftUI

struct FavoriteButton: View {
    var body: some View {
        Text("Hello, SwiftUI!")
    }
}

struct FavoriteButton_Previews: PreviewProvider {
    static var previews: some View {
        FavoriteButton()
    }
}
```

&emsp;添加指示按钮当前状态的 `isSet` 绑定，并为预览提供常量值。由于使用绑定，因此在此视图中所做的更改将传播回数据源。

```swift
import SwiftUI

struct FavoriteButton: View {
    @Binding var isSet: Bool
    
    var body: some View {
        Text("Hello, SwiftUI!")
    }
}

struct FavoriteButton_Previews: PreviewProvider {
    static var previews: some View {
        FavoriteButton(isSet: .constant(true))
    }
}
```

&emsp;创建一个按钮，其中包含切换 `isSet` 状态的操作，并根据状态更改其外观。

```swift
struct FavoriteButton: View {
    @Binding var isSet: Bool
    
    var body: some View {
        Button {
            isSet.toggle()
        } label: {
            Image(systemName: isSet ? "star.fill" : "start")
                .foregroundColor(isSet ? Color.yellow : Color.gray)
        }
    }
}
```

&emsp;接下来，将把 FavoriteButton 添加到详细信息视图（Land markDetail）中，将按钮的 `isSet` 属性绑定到给定地标的 `isFavorite` 属性。

&emsp;切换到 LandmarkDetail.swift，并通过将输入地标与模型数据进行比较来计算输入地标的索引。若要支持此功能，还需要访问环境的模型数据（`@EnvironmentObject var modelData: ModelData`）。

```swift
struct LandmarkDetail: View {
    @EnvironmentObject var modelData: ModelData
    var landmark: Landmark
    
    var landmarkIndex: Int {
        modelData.landmarks.firstIndex(where: {$0.id == landmark.id })!
    }
    
    ...
}
```

&emsp;将地标的名称（Text）和新收藏按钮（FavoriteButton）嵌入到 HStack 中，FavoriteButton 提供与美元符号（$）的 isFavorite 属性的绑定。将 landmarkIndex 与 modelData 对象结合使用，以确保该按钮更新存储在模型对象中的地标的 isFavorite 属性。（就是保证 FavoriteButton 开关更新的是当前这个地标的 modelData.landmarks 中的 Landmark 数据源）

```swift
...

HStack {
    Text(landmark.name)
        .font(.title)
    FavoriteButton(isSet: $modelData.landmarks[landmarkIndex].isFavorite)
}

...
```
&emsp;至此，就完成了此节的所有代码。当从列表导航到某个地标的详细信息并点击收藏按钮时，当返回列表时，这些更改将持续存在。由于两个视图访问环境中的同一模型对象，因此这两个视图保持一致性。（以前我们可能会使用 block 或者 delegate 做回调更新数据，现在 Combine + SwiftUI 简直优雅的一批）

&emsp;下面我们看一下示例代码中，比较陌生的关键字、修饰符的用法以及含义。

### @EnvironmentObject

&emsp;

### @Binding

&emsp;

### @ObservedObject

&emsp;


&emsp;`@StateObject`、`.environmentObject`、`@EnvironmentObject`、`@State`、`@Binding`、`ObservableObject`、`@Published`、``










## 参考链接
**参考链接:🔗**
+ [SwiftUI状态绑定：@State](https://www.jianshu.com/p/46cbe061c8f5)
+ [[译]理解 SwiftUI 里的属性装饰器@State, @Binding, @ObservedObject, @EnvironmentObje](https://www.cnblogs.com/xiaoniuzai/p/11417123.html)
+ [[SwiftUI 100 天] 用 @EnvironmentObject 从环境中读取值](https://zhuanlan.zhihu.com/p/146608338)



## 看着看着发现 LG 都开始卷 Swift 源码了...（必学）
+ [Swift底层进阶--015：Codable源码解析](https://www.jianshu.com/p/9302f7bac319)
+ [Swift底层探索:Codable](https://www.jianshu.com/p/d591bd7f53ac)

## 针对当返回的 Json 字符串中字段的类型 和 模型定义中属性（成员变量）类型不匹配时的解析：只要有一个字段类型不匹配，整个 json 的转 model 都会失败，这是不友好的。
+ [针对 swift4 的JSONDecoder的特殊情况处理](https://www.jianshu.com/p/51c219092290)

## 学习一些 Codable 的嵌套用法、学习 Codable 中的三种容器类型（必学），还有 CodingKey（必学）。

+ [Swift5 Codable源码剖析](https://www.jianshu.com/nb/3595319)
