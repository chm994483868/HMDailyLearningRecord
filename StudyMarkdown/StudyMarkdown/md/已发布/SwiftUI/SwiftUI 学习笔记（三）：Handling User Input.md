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

&emsp;添加指示按钮当前状态的 `isSet`（@Binding），并为预览提供常量值。由于使用 @Binding 属性包装器，因此在此视图中所做的更改将传播回数据源。

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

&emsp;虽然示例代码很少，但是内部隐藏的信息以及 SwiftUI + Combine 不同于我们以前使用 UIKit + block + delegate 等方式下的数据在页面之间的流动，SwiftUI + Combine 的组合更灵动更优雅，以及更有前端那味了。

&emsp;下面我们就沿着数据流动方向看一下示例代码中，比较陌生的关键字、和属性包装器（Property Wrappers）的用法以及含义等内容（这些才是本篇文章的核心点）。（&emsp;`@StateObject`、`.environmentObject`、`@EnvironmentObject`、`@State`、`@Binding`、`ObservableObject`、`@Published`）

### @State

&emsp;**@State 主要用于单一视图的本地状态。** 在示例代码中有两处用到了 @State，分别是：

1. struct LandmarkList 结构体中的 `@State private var showFavoritesOnly = false` 属性，showFavoritesOnly 和列表第一行的 `Toggle(isOn:$showFavoritesOnly) { Text("Favorites only") }` 绑定在一起，记录当前在列表是否显示全部地标还是仅显示收藏的地标。

2. struct MapView 结构体中的 `@State private var region = MKCoordinateRegion()` 属性，region 和 `Map(coordinateRegion: $region).onAppear { setRegion(coordinate) }` 地图坐标范围绑定在一起，记录当前地图的坐标变化，且 struct MapView 结构体中还定义了一个 `private func setRegion(_ coordinate: CLLocationCoordinate2D) { region = MKCoordinateRegion( center: coordinate, span: MKCoordinateSpan(latitudeDelta: 0.2, longitudeDelta: 0.2)) }` 函数，可以看到函数里面修改了 struct MapView 结构体的 `region` 属性，但是前面并不需要用 `mutating` 修饰。

&emsp;@State 属性允许你修改 Struct 的属性，这些属性在普通的 Struct 里面是不允许修改的。当把 @State 放置到属性前，该属性实际上会被放到 Struct 的外部存储起来，这意味着 SwiftUI 能够随时销毁和重建 Struct 而不会丢失属性的值。

&emsp;@State 包装的属性通常是设置成私有的，不让外部使用。如果想让外部使用，则应该使用 @ObservedObject 和 @EnvironmentObject，他们能够使外部修改属性后，状态能够得到改变。建议把 @State 包装的属性都设置成私有。

### @Binding

&emsp;@Binding 用的比较少，但是也是非常重要的一个属性包装器，声明一个属性是从外部获取的，并且与外部是共享的。相当于外部传过来的时候，不是传递的值。

&emsp;在 struct FavoriteButton 结构体中定义了一个使用 @Binding 属性包装器的属性：isSet。

```swift
struct FavoriteButton: View {
    @Binding var isSet: Bool

    var body: some View {
        Button {
            isSet.toggle()
        } label: {
            Label("Toggle Favorite", systemImage: isSet ? "star.fill" : "star")
                .labelStyle(.iconOnly)
                .foregroundColor(isSet ? .yellow : .gray)
        }
    }
}
```

&emsp;在 Button 按钮的点击事件中来回切换 isSet 的值。

```swift
@frozen public struct Bool : Sendable {
    ...

    /// Toggles the Boolean variable's value. 切换布尔变量的值。
    ///
    /// Use this method to toggle a Boolean value from `true` to `false` or from `false` to `true`. 使用此方法将布尔值从 "true" 切换为 "false" 或从 "false" 切换为 "true"。
    ///
    ///     var bools = [true, false]
    ///
    ///     bools[0].toggle()
    ///     // bools == [false, false]
    @inlinable public mutating func toggle()
    
    ...
}
```

&emsp;同 @State 一样，@Binding 修饰的结构体的属性允许被修改，如果我们把 isSet 前面的 @Binding 删除，则会报错：`Cannot use mutating member on immutable value: 'self' is immutable`。

&emsp;在 struct FavoriteButton_Previews 中使用到了： `Binding.constant(true)` 下面简单看一下它的介绍：

```swift
/// A property wrapper type that can read and write a value owned by a source of truth. 一种属性包装器类型，可以读取和写入实际来源拥有的值。
///
/// Use a binding to create a two-way connection between a property that stores data, and a view that displays and changes the data. 使用绑定可以在存储数据的属性与显示和更改数据的视图之间创建双向连接。 
/// A binding connects a property to a source of truth stored elsewhere, instead of storing data directly. 绑定将属性连接到存储在其他位置的事实源，而不是直接存储数据。
/// For example, a button that toggles between play and pause can create a binding to a property of its parent view using the `Binding` property wrapper. 例如，在播放和暂停之间切换的按钮可以使用 "Binding" 属性包装器创建与其父视图属性的绑定。
///
///     struct PlayButton: View {
///         @Binding var isPlaying: Bool
///
///         var body: some View {
///             Button(action: {
///                 self.isPlaying.toggle()
///             }) {
///                 Image(systemName: isPlaying ? "pause.circle" : "play.circle")
///             }
///         }
///     }
///
/// The parent view declares a property to hold the playing state, using the ``State`` property wrapper to indicate that this property is the value's source of truth. 父视图声明一个属性来保存播放状态，使用 "State" 属性包装器来指示此属性是值的真实来源。
///
///     struct PlayerView: View {
///         var episode: Episode
///         @State private var isPlaying: Bool = false
///
///         var body: some View {
///             VStack {
///                 Text(episode.title)
///                 Text(episode.showTitle)
///                 PlayButton(isPlaying: $isPlaying)
///             }
///         }
///     }
///
/// When `PlayerView` initializes `PlayButton`, it passes a binding of its state property into the button's binding property. 当 "PlayerView" 初始化 "PlayButton" 时，它会将其状态属性的绑定传递到按钮的绑定属性中。 
/// Applying the `$` prefix to a property wrapped value returns its ``State/projectedValue``, which for a state property wrapper returns a binding to the value. 将 "$" 前缀应用于属性包装值将返回其 "State/projectedValue"，对于状态属性包装器，该值返回与该值的绑定。
///
/// Whenever the user taps the `PlayButton`, the `PlayerView` updates its `isPlaying` state. 每当用户点击 "PlayButton" 时，"PlayerView" 都会更新其 "isPlaying" 状态。
@available(iOS 13.0, macOS 10.15, tvOS 13.0, watchOS 6.0, *)
@frozen @propertyWrapper @dynamicMemberLookup public struct Binding<Value> {
    ...
    
    /// Creates a binding with an immutable value. 创建具有不可变值的绑定。
    ///
    /// Use this method to create a binding to a value that cannot change. 使用此方法可以创建与无法更改的值的绑定。
    /// This can be useful when using a ``PreviewProvider`` to see how a view represents different values. 当使用 "PreviewProvider" 查看视图如何表示不同值时，这可能很有用。
    ///
    ///     // Example of binding to an immutable value.
    ///     PlayButton(isPlaying: Binding.constant(true))
    ///
    /// - Parameter value: An immutable value.
    public static func constant(_ value: Value) -> Binding<Value>
    
    ...
}
```

&emsp;在 struct LandmarkDetail 结构体中使用 FavoriteButton 时，其初始化传递了 struct Landmark 结构体的 `var isFavorite: Bool` 属性：`FavoriteButton(isSet: $modelData.landmarks[landmarkIndex].isFavorite)` 即把 struct FavoriteButton 的 isSet 的值和环境变量 modelData.landmarks 数据源中的指定的 struct Landmark 结构体实例的 isFavorite 值绑定在一起，当 isSet 发生变化时 LandmarkList 的地标列表进行刷新。

### @Published

&emsp;@Published 是 SwiftUI 很有用的属性包装器，允许我们创建出能够被 **自动观察的对象属性**，SwiftUI 会自动监视这个属性，一旦发生了改变，会自动刷新与该属性绑定的界面。@Published 包装会自动添加 willSet 方法监视属性的改变。配合 Combine 的 ObservableObject 使用。

### @ObservedObject

&emsp;@ObservedObject 告诉 SwiftUI，这个对象是可以被观察的，里面含有被 @Published 包装了的属性。@ObservedObject 包装的对象，必须遵循 ObservableObject 协议。也就是说必须是 class 对象，不能是 struct。@ObservedObject 允许外部进行访问和修改。 

&emsp;在 ModelData.swift 中定义了遵循 ObservableObject 协议的 class ModelData 类，ModelData 类内部 landmarks 属性用 @Published 包装。

```swift
import Combine

final class ModelData: ObservableObject {
    @Published var landmarks: [Landmark] = load("landmarkData.json")
}
```

&emsp;ObservableObject 协议继承自 AnyObject 协议。所有类都隐式遵循 AnyObject 协议。

```swift
/// A type of object with a publisher that emits before the object has changed. 一种对象类型，其 publisher 在对象更改之前发出。
///
/// By default an ``ObservableObject`` synthesizes an ``ObservableObject/objectWillChange-2oa5v`` publisher that emits the changed value before any of its `@Published` properties changes. 默认情况下，"ObservableObject" 合成一个 "ObservableObject/objectWillChange-2oa5v" 发布者，该发布者在其任何 "@Published" 属性更改之前发出更改的值。
///
///     class Contact: ObservableObject {
///         @Published var name: String
///         @Published var age: Int
///
///         init(name: String, age: Int) {
///             self.name = name
///             self.age = age
///         }
///
///         func haveBirthday() -> Int {
///             age += 1
///             return age
///         }
///     }
///
///     let john = Contact(name: "John Appleseed", age: 24)
///     cancellable = john.objectWillChange
///         .sink { _ in
///             print("\(john.age) will change")
///     }
///     print(john.haveBirthday())
///     // Prints "24 will change"
///     // Prints "25"
@available(iOS 13.0, macOS 10.15, tvOS 13.0, watchOS 6.0, *)
public protocol ObservableObject : AnyObject {

    /// The type of publisher that emits before the object has changed. 在对象更改之前发出的 publisher 类型。
    associatedtype ObjectWillChangePublisher : Publisher = ObservableObjectPublisher where Self.ObjectWillChangePublisher.Failure == Never

    /// A publisher that emits before the object has changed. 在对象更改之前发出的 publisher。
    var objectWillChange: Self.ObjectWillChangePublisher { get }
}
```

### @EnvironmentObject

&emsp;从名字上可以看出，这个属性包装器是针对全局环境的。通过它，我们可以避免在初始 View 时创建 ObservableObject, 而是从环境中获取 ObservableObject。通过这种方式，我们能在复杂应用之中轻易地共享数据。

&emsp;在示例代码中，struct LandmarksApp 中定义一个 `@StateObject private var modelData = ModelData()` 属性，然后 body 中通过 `ContentView().environmentObject(modelData)` 把 modelData 实例作为 ContentView 视图的环境变量，那么在 ContentView 视图的所有子视图中，都可以读取 modelData 实例，且当 modelData 发生变化时，读取使用 modelData 的 ContentView 子视图都会被刷新。看到 struct LandmarkList 和 struct LandmarkDetail 分别通过：`@EnvironmentObject var modelData: ModelData` 从环境中获取 modelData。

```swift
@available(iOS 13.0, macOS 10.15, tvOS 13.0, watchOS 6.0, *)
extension View {

    /// Supplies an `ObservableObject` to a view subhierarchy. 向视图子层次结构提供 "ObservableObject"。
    ///
    /// The object can be read by any child by using `EnvironmentObject`. 任何子对象都可以通过使用 "EnvironmentObject" 来读取。
    ///
    /// - Parameter object: the object to store and make available to the view's subhierarchy. 要存储并提供给视图的子层次结构的对象。
    @inlinable public func environmentObject<T>(_ object: T) -> some View where T : ObservableObject

}
```

### @StateObject

&emsp;














## 参考链接
**参考链接:🔗**
+ [SwiftUI状态绑定：@State](https://www.jianshu.com/p/46cbe061c8f5)
+ [[译]理解 SwiftUI 里的属性装饰器@State, @Binding, @ObservedObject, @EnvironmentObje](https://www.cnblogs.com/xiaoniuzai/p/11417123.html)
+ [[SwiftUI 100 天] 用 @EnvironmentObject 从环境中读取值](https://zhuanlan.zhihu.com/p/146608338)
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
