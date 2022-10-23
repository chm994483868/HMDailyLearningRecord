# SwiftUI 学习笔记（五）：Animating Views and Transitions.md

&emsp;SwiftUI 官方教程：[SwiftUI Tutorials](https://developer.apple.com/tutorials/swiftui) 仅是几个体现 SwiftUI 简单使用的小 demo 而已，简单易学，循序渐进，先看完可以对 SwiftUI 有一个大概的认知。

## 五：Animating Views and Transitions

&emsp;[Drawing and Animation - Animating Views and Transitions](https://developer.apple.com/tutorials/swiftui/animating-views-and-transitions) 为视图和过渡添加动画效果

&emsp;使用 SwiftUI 时，你可以单独对视图或视图状态的更改进行动画处理，无论效果位于何处。SwiftUI 为你处理这些组合、重叠和可中断动画的所有复杂性。

&emsp;在本节中，将对一个视图进行动画处理，该视图包含一个图表，用于跟踪用户在使用 Landmark 应用程序时进行的徒步旅行。使用 `animation(_:)` 修饰符，你将看到为视图添加动画效果是多么容易。

### Add Hiking Data to the App

&emsp;在添加动画之前，你需要对某些内容进行动画处理。在本节中，你将导入徒步旅行数据并对其进行建模，然后添加一些预构建的视图，以便在图表中静态显示该数据。

&emsp;将 hikeData.json 文件从下载文件的 "Resources" 文件夹拖到项目的 "Resources" 文件夹中。请务必选择 "Copy items if needed"，然后再点按 "Finish" 按钮。

&emsp;使用菜单项 "File > Nes > File"，在项目的 "Model" 文件夹中创建一个名为 Hike.swift 的新 Swift 文件。

&emsp;与 "Landmark" 结构体一样，"Hike" 结构体也遵循 Codable 协议，并且具有与相应数据文件中的键匹配的属性。

```swift
import Foundation

struct Hike: Codable, Hashable, Identifiable {
    var id: Int
    var name: String
    var distance: Double
    var difficulty: Int
    var observations: [Observation]
    
    static var formatter = LengthFormatter()
    
    var distanceText: String {
        Hike.formatter.string(fromValue: distance, unit: .kilometer)
    }
    
    struct Observation: Codable, Hashable {
        var distanceFromStart: Double
        
        var elevation: Range<Double>
        var pace: Range<Double>
        var heartRate: Range<Double>
    }
}
```

&emsp;将 hikes 数组加载到模型对象中。

&emsp;由于你在最初加载 hikes 数据后永远不会修改它，因此你无需使用 `@Published` 属性对其进行标记。

```swift
final class ModelData: ObservableObject {
    ...
    var hikes: [Hike] = load("hikeData.json")
}
```

&emsp;将 "Hikes" 文件夹从下载文件的 "Resources" 文件夹拖到项目的 "Views" 文件夹中。请务必选择 "Copy items if needed"，然后再点按 "Finish" 按钮。

&emsp;熟悉新 Views。它们协同工作以显示加载到模型中的 hike 数据。

&emsp;在 HikeView.swift 中，打开实时预览并尝试显示和隐藏图表。

&emsp;请务必在本教程中使用实时预览，以便可以试验每个步骤的结果。

### Add Animations to Individual Views

&emsp;使用 `animation(_:)` 在 `equatable` 视图上使用修饰符，SwiftUI 会对视图的可动画属性所做的任何更改进行动画处理。视图的颜色、不透明度、旋转、大小和其他属性都是可设置动画的。当视图非 `equatable` 时，可以使用 `animation(_:value:)` 用于在指定值更改时启动动画的修饰符。

&emsp;在 HikeView.swift 中，通过添加一个动画修饰符来打开按钮旋转的动画，该修改器从 `showDetail` 值的更改开始。

```swift
...
Button {
    showDetail.toggle()
} label: {
    Label("Graph", systemImage: "chevron.right.circle")
        .labelStyle(.iconOnly)
        .imageScale(.large)
        .rotationEffect(.degrees(showDetail ? 90 : 0))
        .padding()
        .animation(.easeInOut, value: showDetail) // 动画
}
...
```

&emsp;通过在图形可见时放大按钮来添加另一个可动画更改。动画修饰符应用于其包装视图中的所有可动画更改。

```swift
Button {
    showDetail.toggle()
} label: {
    Label("Graph", systemImage: "chevron.right.circle")
        .labelStyle(.iconOnly)
        .imageScale(.large)
        .rotationEffect(.degrees(showDetail ? 90 : 0))
        .scaleEffect(showDetail ? 1.5 : 1) // 动画
        .padding()
        .animation(.easeInOut, value: showDetail) // 动画
}
```

&emsp;将动画类型从 `easeInOut` 更改为 `spring()`。SwiftUI 包括带有预定义或自定义缓动的基本动画，以及弹簧和流体动画。你可以调整动画的速度、设置动画开始前的延迟或指定动画重复。

```swift
Button {
    showDetail.toggle()
} label: {
    Label("Graph", systemImage: "chevron.right.circle")
        .labelStyle(.iconOnly)
        .imageScale(.large)
        .rotationEffect(.degrees(showDetail ? 90 : 0))
        .scaleEffect(showDetail ? 1.5 : 1)
        .padding()
        .animation(.spring(), value: showDetail) // .easeInOut 修改为 .spring()
}
```

&emsp;尝试通过在 `scaleEffect` 修改器上方添加另一个动画修改器来关闭旋转动画。实验性质：试一试 SwiftUI。尝试组合不同的动画效果，看看有什么可能性。

```swift
Button {
    showDetail.toggle()
} label: {
    Label("Graph", systemImage: "chevron.right.circle")
        .labelStyle(.iconOnly)
        .imageScale(.large)
        .rotationEffect(.degrees(showDetail ? 90 : 0))
        .animation(nil, value: showDetail) // 修改
        .scaleEffect(showDetail ? 1.5 : 1)
        .padding()
        .animation(.spring(), value: showDetail)
}
```

&emsp;请先删除这两个动画修饰符，然后再转到下一节。

### Animate the Effects of State Changes

&emsp;现在你已经了解了如何将动画应用到各个视图，是时候在你更改状态值的地方添加动画了。

&emsp;在这里，你将动画应用于当用户点击按钮并切换 `showDetail` 状态属性时发生的所有更改。

&emsp;将调用 `showDetail.toggle()` 包裹到 `withAnimation` 函数调用中。受 `showDetail` 属性影响的两个视图 —— 显示按钮和 `HikeDetail` 视图 —— 现在都有动画过渡。

```swift
Button {
    // showDetail.toggle() 调用包裹到 withAnimation 调用中
    withAnimation {
        showDetail.toggle()
    }
} label: {
    Label("Graph", systemImage: "chevron.right.circle")
        .labelStyle(.iconOnly)
        .imageScale(.large)
        .rotationEffect(.degrees(showDetail ? 90 : 0))
        .scaleEffect(showDetail ? 1.5 : 1)
        .padding()
}
```

&emsp;放慢动画速度，看看 SwiftUI 动画是如何被中断的。

&emsp;将四秒长的基本动画传递给 `withAnimation` 函数。

&emsp;你可以将相同类型的动画传递给传递给动画的 `animation(_:value:)` 修饰语。

```swift
withAnimation(.easeInOut(duration: 4)) {
    showDetail.toggle()
}
```

&emsp;尝试在动画中间打开和关闭图形视图，在 Live Preview 中进行预览。

&emsp;在继续下一节之前，请通过删除调用的输入参数来还原 `withAnimation` 函数以使用默认动画。

### Customize View Transitions

&emsp;默认情况下，视图通过淡入和淡出在屏幕上和屏幕外进行过渡。你可以使用 `transition(_:)` 修饰符自定义此过渡。

&emsp;向有条件可见的 `HikeView` 添加一个 `transition(_:)` 修饰符。现在，图形通过滑入和滑出视线而出现和消失。

```swift
...
if showDetail {
    HikeDetail(hike: hike)
        .transition(.slide)
}
...
```

&emsp;提取刚刚作为 AnyTransition 的静态属性添加的过渡，并在视图的过渡修饰符中访问新属性。当你扩展自定义过渡时，这可以使你的代码保持干净。

```swift
extension AnyTransition {
    static var moveAndFade: AnyTransition {
        AnyTransition.slide
    }
}

...
if showDetail {
    HikeDetail(hike: hike)
        .transition(.moveAndFade)
}
...
```

&emsp;切换到使用 `move(edge:)` 过渡，以便图形从同一侧滑入和滑出。

```swift
extension AnyTransition {
    static var moveAndFade: AnyTransition {
        AnyTransition.move(edge: .trailing)
    }
}
```

&emsp;使用 `asymmetric(insertion:removal:)` 修饰符为视图出现和消失提供不同的过渡。

```swift
extension AnyTransition {
    static var moveAndFade: AnyTransition {
        //        AnyTransition.slide
        //        AnyTransition.move(edge: .trailing)
        .asymmetric(
            insertion: .move(edge: .trailing).combined(with: .opacity),
            removal: .scale.combined(with: .opacity)
        )
    }
}
```

### Compose Animations for Complex Effects

&emsp;当你单击条形下方的按钮时，图形会在三组不同的数据集之间切换。在本节中，你将使用组合动画为构成图形的胶囊提供动态的波纹过渡。

&emsp;在 `HikeView` 中，将 `showDetail` 的默认值更改为 `true`，并将预览固定到画布上。这使你可以在处理另一个文件中的动画时在上下文中查看图形。

&emsp;在 `HikeGraph.swift` 中，定义一个新的波纹动画并将其应用于每个生成的图形胶囊。

```swift
extension Animation {
    static func ripple() -> Animation {
        Animation.default
    }
}

...
GraphCapsule(
    index: index,
    color: color,
    height: proxy.size.height,
    range: observation[keyPath: path],
    overallRange: overallRange
)
.animation(.ripple())
...
```

&emsp;将动画切换为弹簧动画，减少阻尼分数以使条形跳跃。你可以在实时预览中通过在海拔、心率和配速之间切换来查看动画效果。

```swift
extension Animation {
    static func ripple() -> Animation {
        Animation.spring(dampingFraction: 0.5)
    }
}
```

&emsp;稍微加快动画速度，以缩短每个条形移动到新位置所需的时间。

```swift
extension Animation {
    static func ripple() -> Animation {
        Animation.spring(dampingFraction: 0.5)
            .speed(2)
    }
}
```

&emsp;为每个动画添加基于胶囊在图形上的位置的延迟。

```swift
extension Animation {
    static func ripple(index: Int) -> Animation {
        Animation.spring(dampingFraction: 0.5)
            .speed(2)
            .delay(0.03 * Double(index))
    }
}

...
GraphCapsule(
    index: index,
    color: color,
    height: proxy.size.height,
    range: observation[keyPath: path],
    overallRange: overallRange
)
.animation(.ripple(index: index))
...
```

&emsp;观察自定义动画在图形之间过渡时如何提供波纹效果。请务必先取消固定预览，然后再继续学习下一教程。













## 参考链接
**参考链接:🔗**
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

