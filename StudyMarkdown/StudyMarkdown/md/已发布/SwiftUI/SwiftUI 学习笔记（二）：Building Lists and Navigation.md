# SwiftUI 学习笔记（二）：Building Lists and Navigation

&emsp;SwiftUI 官方教程：[SwiftUI Tutorials](https://developer.apple.com/tutorials/swiftui) 仅是几个体现 SwiftUI 简单使用的小 demo 而已，简单易学，循序渐进，先看完可以对 SwiftUI 有一个大概的认知。

## 二：Building Lists and Navigation

&emsp;[SwiftUI Essentials - Creating and Combining Views](https://developer.apple.com/tutorials/swiftui/building-lists-and-navigation) 创建和组合 Views。

&emsp;设置基本地标详细信息视图后，需要为用户提供一种查看地标完整列表以及查看每个位置的详细信息的方法。创建可显示有关任何地标的信息的视图，并动态生成滚动列表，用户可以点击该滚动列表以查看地标的详细信息视图。要微调 UI，将使用 Xcode 的 canvas（画布）以不同的设备大小呈现多个预览。

### Landmark.swift

&emsp;创建一个 Landmark 模型（struct Landmark 结构体）在上个教程中，我们都是硬编码信息输入到所有自定义视图中（ContentView 中是 "Turtle Rock"，CircleImage 中指定的名字是 "turtlerock" 的图片，MapView 中固定的经纬度坐标）。现在我们创建一个模型（Landmark 结构体）来存储可以传递到视图中的数据。在 Landmark 结构体中包含一些与 landmarkData.json 数据文件中某些 keys 的名称匹配的属性。（landmarkData.json 中是一组地点信息数据）

&emsp;使 Landmark 结构体 遵循 Codable 协议，可以更轻松地在 Landmark 结构体和 landmarkData.json 数据文件之间移动数据。在后面，我们将依靠 Codable protocol 的可解码组件（Decodable component ）从 landmarkData.json 文件中读取数据。

```swift
import Foundation
import SwiftUI
import CoreLocation

struct Landmark: Hashable, Codable, Identifiable {
    var id: Int
    var name: String
    var park: String
    var state: String
    var description: String
    
    // imageName 私有，不必对外界暴露，外界只需要下面的通过 imageName 从 asset catalog 中读取 Image 的计算属性 iamge 即可
    private var imageName: String
    
    var image: Image {
        Image(imageName)
    }
    
    // 同上，私有的 coordinates 属性，记录从本地 json 文件中读取的经纬度信息，locationCoordinate 计算属性，根据 coordinates 中的经纬度信息，构建 CLLocationCoordinate2D 实例
    private var coordinates: Coordinates
    var locationCoordinate: CLLocationCoordinate2D {
        CLLocationCoordinate2D(
            latitude: coordinates.latitude,
            longitude: coordinates.longitude)
    }
    
    // 嵌套定义一个记录经纬度的结构体 Coordinates
    struct Coordinates: Hashable, Codable {
        var latitude: Double
        var longitude: Double
    }
}
```

&emsp;看到 Landmark、Coordinates 结构体都遵循了 Codable 协议，在接下来的学习之前，我们先对 Swift 4.0 推出的 Codable 协议进行学习。

### Codable

```swift
public typealias Codable = Decodable & Encodable
```

### Hashable

&emsp;只有遵循了 Hashable 协议 才能被添加到 Set 中，或者用作 Dictionary 的 key 值。

```swift
public protocol Hashable : Equatable {

    /// The hash value.
    ///
    /// Hash values are not guaranteed to be equal across different executions of
    /// your program. Do not save hash values to use during a future execution.
    ///
    /// - Important: `hashValue` is deprecated as a `Hashable` requirement. To
    ///   conform to `Hashable`, implement the `hash(into:)` requirement instead.
    var hashValue: Int { get }

    /// Hashes the essential components of this value by feeding them into the
    /// given hasher.
    ///
    /// Implement this method to conform to the `Hashable` protocol. The
    /// components used for hashing must be the same as the components compared
    /// in your type's `==` operator implementation. Call `hasher.combine(_:)`
    /// with each of these components.
    ///
    /// - Important: Never call `finalize()` on `hasher`. Doing so may become a
    ///   compile-time error in the future.
    ///
    /// - Parameter hasher: The hasher to use when combining the components
    ///   of this instance.
    func hash(into hasher: inout Hasher)
}
```


### Identifiable

&emsp;











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
