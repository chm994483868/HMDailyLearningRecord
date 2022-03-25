# SwiftUI 学习笔记（三）：Handling User Input

&emsp;SwiftUI 官方教程：[SwiftUI Tutorials](https://developer.apple.com/tutorials/swiftui) 仅是几个体现 SwiftUI 简单使用的小 demo 而已，简单易学，循序渐进，先看完可以对 SwiftUI 有一个大概的认知。

## 三：Handling User Input

&emsp;[SwiftUI Essentials - Handling User Input](https://developer.apple.com/tutorials/swiftui/handling-user-input) 处理用户输入。

&emsp;在 Landmarks 应用中，用户可以标记其收藏的地点，并筛选列表以仅显示其收藏夹（列表）。若要创建此功能，首先要向列表中添加一个开关，以便用户可以只关注其收藏夹，然后添加一个星形按钮，用户点击该按钮可将地标标记为收藏。（在导航条右上角加一个开关，来回切换显示全部景点和已收藏的景点）

### Mark the User’s Favorite Landmarks

&emsp;标记用户收藏的地标（Landmarks），首先增强列表（已收藏的在 LandmarkRow 上添加黄色的星标记），以便一目了然地向用户显示他们的收藏。向 Landmark 结构添加一个属性，以将地标的初始状态作为收藏进行读取，然后向每个显示收藏地标的 LandmarkRow 添加一颗星号。

&emsp;首先修改数据源模型，为 Landmark 结构体添加一个 `var isFavorite: Bool` 成员变量，记录对应的景点是否收藏了。

```swift
struct Landmark: Hashable, Codable, Identifiable {
    ...
    
    var isFavorite: Bool

    ...
}
```

&emsp;然后选中 LandmarkRow.swift 我们为收藏的景点所在的行右边添加一颗星。

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

&emsp;仅当当前 LandmarkRow 的 landmark 成员变量的 isFavorite 为真时才添加一个 Image 视图，且这里使用了系统提供的图片（图标），`Image(systemName: "star.fill")` 它默认是黑色的，且是基于矢量的，所以我们可以通过 `foregroundColor(_:)` 来设置它的颜色，例如这里我们把它修改为黄色。

### Filter the List View

&emsp;可以自定义列表视图（LandmarkList），使其显示所有地标，或仅显示用户的收藏列表。为此，我们需要向 LandmarkList 添加一些状态。

&emsp;状态（State）是一个值或一组值，可以随时间而变化，并且会影响视图的行为、内容或布局。使用具有 @State 修饰的属性向视图添加状态。

&emsp;@State 关键字还记得吗？使用 @State 修饰某个属性后，SwiftUI 将会把该属性存储到一个特殊的内存区域内，并且这个区域和 View struct 是隔离的。当 @State 修饰的属性的值发生变化后，SwiftUI 会根据该属性重新绘制视图。

&emsp;为 struct LandmarkList 添加一个名为 `showFavoritesOnly` 的 @State 属性，其初始值设置为 false。由于使用 State 属性来保存特定于视图及其子视图的信息，因此始终将 State 创建为 private。

&emsp;为 struct LandmarkList 添加一个名为 `filteredLandmarks` 的计算属性，通过检查 `showFavoritesOnly` 属性和 `landmarks` 全局数组中每个 `landmark` 的 `isFavorite` 属性来筛选出收藏的景点列表。

```swift
struct LandmarkList: View {
    @State private var showFavoritesOnly = false
    
    struct LandmarkList: View {
    @State private var showFavoritesOnly = true
    
    var filteredLandmarks: [Landmark] {
        landmarks.filter { landmark in
            (!showFavoritesOnly || landmark.isFavorite)
        }
    }
    
    ...
}
```

### Add a Control to Toggle the State















## 参考链接
**参考链接:🔗**
+ [[SwiftUI 知识碎片] 为什么 SwiftUI 用 "some View" 作为视图类型?](https://zhuanlan.zhihu.com/p/105213050)
+ [SwiftUI 中的 some 关键字](https://www.jianshu.com/p/6eef60ab14bc)
+ [Opaque Types](https://docs.swift.org/swift-book/LanguageGuide/OpaqueTypes.html)
+ [SwiftUI状态绑定：@State](https://www.jianshu.com/p/46cbe061c8f5)
+ [[译]理解 SwiftUI 里的属性装饰器@State, @Binding, @ObservedObject, @EnvironmentObje](https://www.cnblogs.com/xiaoniuzai/p/11417123.html)
+ [SwiftUI为啥可以这样写代码？](https://blog.csdn.net/studying_ios/article/details/104833278)


+ [swift--Codable](https://www.jianshu.com/p/3aab46dcd339)
+ [Swift 4.1 新特性 (4) Codable的改进](https://www.jianshu.com/p/8292ab49d492)
+ [Swift 4.1 新特性 (3) 合成 Equatable 和 Hashable](https://www.jianshu.com/p/2aa31c90abbd)
+ [SwiftUI 基础之06 Identifiable 有什么用](https://www.jianshu.com/p/69a9f2f88782)
+ [iOS开发 - Swift中的Codable, Hashable, CaseIterable, Identifiable.....](https://www.jianshu.com/p/06c993c5ad89)
+ [Swift之Codable实战技巧](https://zhuanlan.zhihu.com/p/50043306)
+ [Swift 4 JSON 解析进阶](https://blog.csdn.net/weixin_33962923/article/details/88986627)

## 看着看着发现 LG 都开始卷 Swift 源码了...（必学）
+ [Swift底层进阶--015：Codable源码解析](https://www.jianshu.com/p/9302f7bac319)
+ [Swift底层探索:Codable](https://www.jianshu.com/p/d591bd7f53ac)

## 针对当返回的 Json 字符串中字段的类型 和 模型定义中属性（成员变量）类型不匹配时的解析：只要有一个字段类型不匹配，整个 json 的转 model 都会失败，这是不友好的。
+ [针对 swift4 的JSONDecoder的特殊情况处理](https://www.jianshu.com/p/51c219092290)

## 学习一些 Codable 的嵌套用法、学习 Codable 中的三种容器类型（必学），还有 CodingKey（必学）。

+ [Swift5 Codable源码剖析](https://www.jianshu.com/nb/3595319)
