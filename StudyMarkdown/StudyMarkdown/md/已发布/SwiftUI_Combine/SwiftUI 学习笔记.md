# SwiftUI 学习笔记

&emsp;SwiftUI 官方教程：[SwiftUI Tutorials](https://developer.apple.com/tutorials/swiftui) 仅是几个体现 SwiftUI 简单使用的小 demo 而已，简单易学，循序渐进，先看完可以对 SwiftUI 有一个大概的认知。

## 一：Creating and Combining Views

&emsp;[SwiftUI Essentials - Creating and Combining Views](https://developer.apple.com/tutorials/swiftui/creating-and-combining-views) 创建和组合 Views。

### some View 

&emsp;首先 `LandmarksApp.swift` 和 `ContentView.swift` 两个初始文件，看到一个尤其明显的关键字：`some`，它是在 Swift 5.1 中引入的 `Opaque Types` 强大特性中使用到的一个关键字。

```swift
@main
struct LandmarksApp: App {
    var body: some Scene {
        WindowGroup {
            ContentView()
        }
    }
}

struct ContentView: View {
    var body: some View {
        Text("Hello, World!")
            .padding()
    }
}
```

&emsp;之前在 associatedtype、some、protocol 三者联系中对 some 有一些了解。在 [Opaque Types](https://docs.swift.org/swift-book/LanguageGuide/OpaqueTypes.html#ID613) 文档中详细介绍了一些 "不透明类型" 和 some 相关的知识点。

&emsp;那么 some 关键字用在 SwiftUI 中起个什么作用呢？我们可以尝试直接把 some 关键字删了，然后 Xcode 就会给我们提示一个同样的错误提示：`Protocol 'View' can only be used as a generic constraint because it has Self or associated type requirements`，之前在测试函数返回值类型是有 associatedtype 的 protocol 时 xcode 报过同样的错误，这里就联系上了，说明 view 也是一个有 associatedtype 的 protocol。

&emsp;那么如果就是不想使用 some 关键字标示 var body 的类型并且让 Xcode 不报错怎么办呢？我们可以如下这样，直接完全指明 body 的类型：

```swift
//    var body: some View {
//        VStack {
//            Text("Hello, SwiftUI!")
//            Image(systemName: "video.fill")
//            Button("Touch Me") { print(type(of: self.body)) }
//        }
//    }

var body: VStack<TupleView<(Text, Image, Button<Text>)>> {
    VStack {
        Text("Hello, SwiftUI!")
        Image(systemName: "video.fill")
        Button("Touch Me") { print(type(of: self.body)) }
    }
}
```

&emsp;根据 body 内部的组件内容可看到当前 body 的类型是：`VStack<TupleView<(Text, Image, Button<Text>)>>`，并且稍微动一下其中的内容就发现 body 的类型变化了，而 `some View` 正是把这个冗长的类型 "抹消" 调，让编译器自己去推断 body 的类型。（some View 说的是："它将返回一个特定的 View 类型，比如 Button、Text 以及各种视图组合，但我不想说具体是啥"） 

&emsp;关于 SwiftUI 中的 View 呢？它是一个 protocol，一个视图的容器，并不是我们在 UIKit 中使用的具体的一个 UIView 类。后面我们会分析下 SwiftUI 中的 View，现在我们目光主要集中在 some 中。

> &emsp;SwiftUI 高度依赖 Swift 5.1 引入的一个强大特性，它叫 "opaque return types" ，它可以用于函数、方法和属性返回一些值，无需向调用 API 的客户端揭示该值的具体类型。每一次你看到 some View 的地方就是它了。它表示 "某个遵循 View 协议的特定类型，但我们不必说具体是什么。或者表示它将返回一个特定的 view 类型，比如 Button 或者 Text 或者各种视图的组合，但我不想说具体是啥。"[[SwiftUI 知识碎片] 为什么 SwiftUI 用 "some View" 作为视图类型?](https://zhuanlan.zhihu.com/p/105213050)

&emsp;看完 some 下面我们继续分析 Landmarks 工程中的其他文件。

### MapView.swift  

&emsp;`struct MapView: View` 用于展示景点的地图位置，内容比较简单，首先引入了 MapKit 框架，然后定义了一个经纬度坐标位置及范围，

```swift
import SwiftUI
import MapKit

struct MapView: View {
    @State private var region = MKCoordinateRegion(center: CLLocationCoordinate2D(latitude: 34.011_286, longitude: -116.166_868),
                                                   span: MKCoordinateSpan(latitudeDelta: 0.2, longitudeDelta: 0.2))
    
    var body: some View {
        // 在这里变量前面用了 $ 符号，Dart 中也有这个设定
        Map(coordinateRegion: $region)
    }
}

struct MapView_Previews: PreviewProvider {
    static var previews: some View {
        MapView()
    }
}

```














## 参考链接
**参考链接:🔗**
+ [[SwiftUI 知识碎片] 为什么 SwiftUI 用 "some View" 作为视图类型?](https://zhuanlan.zhihu.com/p/105213050)
+ [SwiftUI 中的 some 关键字](https://www.jianshu.com/p/6eef60ab14bc)
+ [Opaque Types](https://docs.swift.org/swift-book/LanguageGuide/OpaqueTypes.html)
