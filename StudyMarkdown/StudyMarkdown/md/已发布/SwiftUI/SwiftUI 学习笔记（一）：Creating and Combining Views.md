# SwiftUI 学习笔记（一）：Creating and Combining Views

&emsp;SwiftUI 官方教程：[SwiftUI Tutorials](https://developer.apple.com/tutorials/swiftui) 仅是几个体现 SwiftUI 简单使用的小 demo 而已，简单易学，循序渐进，先看完可以对 SwiftUI 有一个大概的认知。

## 一：Creating and Combining Views

&emsp;[SwiftUI Essentials - Creating and Combining Views](https://developer.apple.com/tutorials/swiftui/creating-and-combining-views) 创建和组合 Views。

&emsp;本教程将指导我们构建 Landmarks（地标）- 一个用于发现和共享你喜欢的地方的应用程序。首先构建显示地标详细信息的视图。为了对视图进行布局，Landmarks 使用堆栈（stacks）来组合和分层图像和文本视图组件。要将地图添加到视图中，需要包括一个标准的 MapKit 组件。当优化视图的设计时，Xcode 会提供实时反馈，以便可以看到这些更改如何转化为代码。

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

&emsp;那么 some 关键字用在 SwiftUI 中起个什么作用呢？我们可以尝试直接把 some 关键字删了，然后 Xcode 就会给我们提示一个同样的错误：`Protocol 'View' can only be used as a generic constraint because it has Self or associated type requirements`，之前在测试函数返回值类型是一个包含 associatedtype 的 protocol 时 xcode 报过同样的错误，这里就联系上了，说明 view 也是一个有 associatedtype 的 protocol。

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

&emsp;根据 body 内部的组件内容可看到当前 body 的类型是：`VStack<TupleView<(Text, Image, Button<Text>)>>`，并且稍微动一下其中的内容后打印就发现 body 的类型变化了，而 `some View` 正是把这个冗长的类型 "抹消" 掉，让编译器自己去推断 body 的类型。（some View 说的是："它将返回一个特定的 View 类型，比如 Button、Text 以及各种视图组合，但我不想说具体是啥"） 

&emsp;关于 SwiftUI 中的 View 呢？它是一个 protocol，一个视图的容器，并不是我们在 UIKit 中使用的具体的一个 UIView 视图。后面我们会分析下 SwiftUI 中的 View，现在我们目光主要集中在 some 中。

> &emsp;SwiftUI 高度依赖 Swift 5.1 引入的一个强大特性，它叫 "opaque return types" ，它可以用于函数、方法和属性返回一些值，无需向调用 API 的客户端揭示该值的具体类型。每一次你看到 some View 的地方就是它了。它表示 "某个遵循 View 协议的特定类型，但我们不必说具体是什么。或者表示它将返回一个特定的 View，比如 Button 或者 Text 或者各种视图的组合，但我不想说具体是啥。"[[SwiftUI 知识碎片] 为什么 SwiftUI 用 "some View" 作为视图类型?](https://zhuanlan.zhihu.com/p/105213050)

&emsp;看完 some 下面我们继续分析 Landmarks 工程中的其他文件。

### MapView.swift  

&emsp;`struct MapView: View` 用于展示景点的地图位置，内容比较简单，首先引入了 MapKit 框架，然后定义了一个用于表示经纬度坐标位置及范围的变量：`region`。(struct MKCoordinateRegion 用于描述以特定纬度和经度为中心的矩形地理区域。)

```swift
import SwiftUI
import MapKit

struct MapView: View {
    // Create a private state variable that holds the region information for the map.
    // You use the @State attribute to establish a source of truth for data in your app that you can modify from more than one view. SwiftUI manages the underlying storage and automatically updates views that depend on the value.
    @State private var region = MKCoordinateRegion(center: CLLocationCoordinate2D(latitude: 34.011_286, longitude: -116.166_868),
                                                   span: MKCoordinateSpan(latitudeDelta: 0.2, longitudeDelta: 0.2))
    
    var body: some View {
        // By prefixing a state variable with $, you pass a binding, which is like a reference to the underlying value. When the user interacts with the map, the map updates the region value to match the part of the map that’s currently visible in the user interface.
        Map(coordinateRegion: $region)
    }
}
```

> &emsp;@State 是 Swift 5.1 引入的新关键词，官方的定义有些抽象：A persistent value of a given type, through which a view reads and monitors the value. 一个给给定类型的持久化值，通过这个值 view 可以读取并监控这个数值。用大白话讲，@State 就是一个标签，贴之前视图是不可以修改这个值；贴了之后，只要你修改这变量，界面就会跟着同步修改，这个是现代界面语言都是支持的特性。[SwiftUI 基础之@State 有什么用](https://www.jianshu.com/p/854b8f7a604f)

> &emsp;使用 @State 修饰某个属性后，SwiftUI 将会把该属性存储到一个特殊的内存区域内，并且这个区域和 View struct 是隔离的。当 @State 修饰的属性的值发生变化后，SwiftUI 会根据该属性重新绘制视图。
  + 与 Flutter 对比：SwiftUI 的 @State 与 Flutter 中的 StatefulWidget 类似，都是通过修改 State 中的属性，来更新视图，不同的是实现方式不同。
  + 与 Vue 对比：SwiftUI 的 @State 与 Vue 中 data 很相似，SwiftUI 中 @State 修饰的属性值发生变化后，会更新视图，Vue 中，data 中的属性值发生变化后，引用该属性的元素，也会发生变化。
  [SwiftUI属性装饰器（@State）](https://www.jianshu.com/p/693f65ce6d63)

&emsp;那么总结下来：`@State` 关键字是做什么的呢？当 @State 修饰的属性更新的时候，view 将重新校验 UI，并且更新自身。从原理上说，我们只要改变了 view 里面被关键词 `@State` 修饰的属性，整个 view 的 `body` 就会被重新渲染。

&emsp;在上面的示例代码中 `MapView` 的 `body` 中的 `Map` 使用 `$` 和 `@State` 修饰的 `region` 属性双向绑定在一起，当 `region` 的值发生变化的时候，`Map` 就会更新，同时 `Map` 发生更新时，`region` 的值也会跟着更新。

&emsp;下面我们看一下在 SwiftUI 中 `@state` 和 `$` 两者结合的状态双向绑定的知识点。

> &emsp;我们知道结构体是不可变的，所以我们不能随意改变它的值，在创建一个结构体的方法之后，如果想在方法中修改结构体属性的值（即使此属性是 var），我们需要添加 `mutating` 关键字，比如 `mutating func doSomeThing()`，但是 Swift 不允许声明可修改的计算型属性，比如不能这样写：`mutating var body: some View { ... }`，如下：
  
  ```swift
  struct ContentView: View {
    @State private var tapCount = 0
    
    // 'mutating' may only be used on 'func' declarations
    var body: some View {
        Button("Tap Count: \(tapCount)") {
            // Left side of mutating operator isn't mutable: 'self' is immutable
            self.tapCount += 1
        }
    }
    
    mutating func changeTapCount() {
        tapCount += 1
    }
  }
  ```
  
  那么程序运行过程中，我们想改变属性的值，但是 Swift 的 struct 不允许这样的操作，那该咋办呢？Swift 提供了一种特殊的解决方案：属性包装器。它们是一些特殊的关键字，我们可以放置在属性前面来给属性提供超能力！在上面存储状态的例子中，我们是可以使用 SwiftUI 中的 @State 属性包装器。在上面的示例代码中给 `tapCount` 前面加一个 `@State` 便可解决此问题。@State 使得我们可以超出 struct 本身的限制，动态修改属性的值。SwiftUI 为什么不直接使用类来表示一个 view 呢？我们知道类中可以随意的修改属性的值。其实随着学习 SwiftUI 的深入你会发现，在声明式的 SwiftUI 中，创建和销毁 stuct 是很频繁的操作，如果换成类，那会严重影响到程序的性能。（在使用 @State 包装一个属性的时候，苹果推荐我们为属性加上 private 的访问权限。）
  
  @State 属性包装器已经允许我们自由修改结构体，这样我们就能在程序发生变化的时候，更新我们的界面了。但上面做的还不够，大家有没有想过，如果一个输入框的输入内容改变了，存储输入内容的属性该如何更新呢？这就需要讲到双向绑定了。我们希望在页面的 UI 发生变化的时候，对应的属性也能随之改变，真正做到 "页面时状态的函数"。还是拿输入框来说，我们用一个属性绑定了输入框，这样输入框可以展示我们属性中的值，但同时当输入框的内容有任何改变的时候同时也更新我们的属性，这就是双向绑定！在 Swift 中我们使用一个特殊符号来表示 `$`，在属性前加上 `$` 就意味着会读取属性的值同时任何改变也会更新属性的值。比如像这样：
  
  ```swift
  struct ContentView: View {
    @State private var name = "chm"
    
    var body: some View {
        Form {
            Text("Hello \(name)")
            TextField("Enter your name", text: $name)
        }
    }
  }
  ```
  
  &emsp;运行后在输入框输入任何字符串，会看到输入框上面面会出现 Hello 输入框中输入的字符串，在 Text 中我们使用 `name` 而不是 `$name` 是因为这里是取值，并不需要双向绑定。`name` 是和 TextField 双向绑定的，所以请记住：看到属性前有 `$` 符号，就表明这是一个双向绑定，属性的值读的同时也会被改。
  
  **自定义的双向绑定：**
  
  &emsp;SwiftUI 允许我们使用 Binding 类型来自定义双向绑定，我们可以实现自定义的 `set` 和 `get` 实现。比如下面的代码：
  
  ```swift
  struct ContentView: View {
    @State private var username = ""
    
    var body: some View {
        
        let binding = Binding(
            get: { self.username },
            set: { self.username = $0 }
        )
        
        return VStack {
            Text(username)
            TextField("enter your name", text: binding)
        }
    }
  }
  ```
  
  我们自定义了一个 `binding`，并提供了自己的 `set` 和 `get`，在使用的地方，可以看到在 `binding` 前面不需要添加 `$` 符号。那自定义双向绑定的好处是什么呢？我们可以在自定义绑定的 `set` 和 `get` 中添加自己的逻辑，比如对数据的处理，或者先进行额外的操作，再进行 `set` 和 `get`，这样的应用场景应该还是蛮多的。[SwiftUI如何修改页面状态？@state的使用](https://blog.csdn.net/studying_ios/article/details/103294890)

&emsp;好了我们又学会了一个 "双向数据绑定" 的知识点。

### CircleImage.swift

&emsp;CircleImage 内容比较简单，内部是一个裁切为圆形、加了边框、加了阴影的 Image，用来展示景点的图片。

```swift
struct CircleImage: View {
    var body: some View {
        // Replace the text view with the image of Turtle Rock by using the Image(_:) initializer, passing it the name of the image to display.
        Image("turtlerock")
            // Add a call to clipShape(Circle()) to apply the circular clipping shape to the image.
            // The Circle type is a shape that you can use as a mask, or as a view by giving the circle a stroke or fill.
            .clipShape(Circle())
            // Create another circle with a gray stroke, and then add it as an overlay to give the image a border.
            .overlay { Circle().stroke(.white, lineWidth: 4) }
            // Next, add a shadow with a 7 point radius.
            .shadow(radius: 7)
    }
}
```

### ContentView.swift

&emsp;最后在 ContentView 中使用 Stack 完成各个视图的组装。

```swift
struct ContentView: View {

    var body: some View {
        VStack {
            MapView()
                // To allow the map content to extend to the top edge of the screen, add the ignoresSafeArea(edges: .top) modifier to the map view.
                .ignoresSafeArea(edges: .top)
                // 高度 300
                .frame(height: 300)
            
            // 景点的图片，沿 y 轴向上偏移 130，同时底部 padding 也向上延伸 130，保证下面视图的顶部对齐
            CircleImage()
                .offset(y: -130)
                .padding(.bottom, -130)
                
            // 使用 Stack 可以合并视图，
            VStack(alignment: .leading) {
                Text("Turtle Rock")
                    .font(.title)

                HStack {
                    Text("Joshua Tree National Park")
                    
                    // To direct the layout to use the full width of the device, separate the park and the state by adding a Spacer to the horizontal stack holding the two text views.
                    // A spacer expands to make its containing view use all of the space of its parent view, instead of having its size defined only by its contents.
                    Spacer()
                    
                    Text("California")
                }
                // Finally, move the subheadline font modifier from each Text view to the HStack containing them, and apply the secondary color to the subheadline text.
                // When you apply a modifier to a layout view like a stack, SwiftUI applies the modifier to all the elements contained in the group.
                .font(.subheadline)
                .foregroundColor(.secondary)
                
                // Add a divider and some additional descriptive text for the landmark.
                Divider()

                Text("About Turtle Rock")
                    .font(.title2)
                Text("Descriptive text goes here.")
            }
            // Finally, use the padding() modifier method to give the landmark’s name and details a little more space.
            .padding()
            
            // Add a spacer at the bottom of the outer VStack to push the content to the top of the screen.
            Spacer()
        }
    }
}
```

&emsp;在本节中我们学习了 `some` 关键字的作用，学习了 `@State` 关键字的作用，学习了 `$` 标注的双向数据绑定，学习了使用 Binding 类型来完成双向绑定，以及一些简单的 SwiftUI 视图组合技巧，那么我们下篇见！

## 参考链接
**参考链接:🔗**
+ [[SwiftUI 知识碎片] 为什么 SwiftUI 用 "some View" 作为视图类型?](https://zhuanlan.zhihu.com/p/105213050)
+ [SwiftUI 中的 some 关键字](https://www.jianshu.com/p/6eef60ab14bc)
+ [Opaque Types](https://docs.swift.org/swift-book/LanguageGuide/OpaqueTypes.html)
+ [SwiftUI状态绑定：@State](https://www.jianshu.com/p/46cbe061c8f5)
+ [[译]理解 SwiftUI 里的属性装饰器@State, @Binding, @ObservedObject, @EnvironmentObje](https://www.cnblogs.com/xiaoniuzai/p/11417123.html)
+ [SwiftUI为啥可以这样写代码？](https://blog.csdn.net/studying_ios/article/details/104833278)
