# SwiftUI 学习笔记（四）：Drawing Paths and Shapes

&emsp;SwiftUI 官方教程：[SwiftUI Tutorials](https://developer.apple.com/tutorials/swiftui) 仅是几个体现 SwiftUI 简单使用的小 demo 而已，简单易学，循序渐进，先看完可以对 SwiftUI 有一个大概的认知。

## 四：Drawing Paths and Shapes

&emsp;[Drawing and Animation - Drawing Paths and Shapes](https://developer.apple.com/tutorials/swiftui/drawing-paths-and-shapes) 处理用户输入。

&emsp;每当用户访问其列表中的地标时，都会收到 badge。当然，要让用户获得 badge，需要创建一个 badge。本教程将引导你完成通过组合路径（paths）和形状（shapes）来创建 badge 的过程，然后使用表示该位置的另一个形状叠加这些路径和形状。

&emsp;如果要为不同类型的地标创建多个 badge，请尝试尝试叠加符号、改变重复次数或更改各种角度和比例。

### Create Drawing Data for a Badge View

&emsp;要创建 badge，需要首先定义可用于为 badge 背景绘制六边形形状的数据。

&emsp;创建 HexagonParameters.swift 文件，在其中创建一个名为 HexagonParameters 的结构体，我们将使用 struct HexagonParameters 结构体来定义六边形的形状。定义一个 struct Segment 结构体来容纳代表六边形一侧的三个点，导入 CoreGraphics，以便可以使用 CGPoint 类型。

&emsp;每条边从前一条边结束的地方开始，沿着一条直线移动到第一个点，然后越过拐角处的贝塞尔曲线移动到第二个点。第三个点控制曲线的形状。

&emsp;创建一个 segments 数组来保存 struct Segment 结构体。

&emsp;为六边形的六个线段添加数据，六边形的每一边各一个。这些值存储为单位平方的一小部分，其原点位于左上角，正 x 位于右侧，正 y 位于下方。稍后，将使用这些分数（fractions）来查找具有给定大小的六边形的实际点。

&emsp;添加一个调整值，用于调整六边形的形状。

```swift
import Foundation
import CoreGraphics

struct HexagonParameters {
    struct Segment {
        let line: CGPoint
        let curve: CGPoint
        let control: CGPoint
    }
    
    static let adjustment: CGFloat = 0.085
    
    static let segments = [
        Segment(
            line:    CGPoint(x: 0.60, y: 0.05),
            curve:   CGPoint(x: 0.40, y: 0.05),
            control: CGPoint(x: 0.50, y: 0.00)
        ),
        Segment(
            line:    CGPoint(x: 0.05, y: 0.20 + adjustment),
            curve:   CGPoint(x: 0.00, y: 0.30 + adjustment),
            control: CGPoint(x: 0.00, y: 0.25 + adjustment)
        ),
        Segment(
            line:    CGPoint(x: 0.00, y: 0.70 - adjustment),
            curve:   CGPoint(x: 0.05, y: 0.80 - adjustment),
            control: CGPoint(x: 0.00, y: 0.75 - adjustment)
        ),
        Segment(
            line:    CGPoint(x: 0.40, y: 0.95),
            curve:   CGPoint(x: 0.60, y: 0.95),
            control: CGPoint(x: 0.50, y: 1.00)
        ),
        Segment(
            line:    CGPoint(x: 0.95, y: 0.80 - adjustment),
            curve:   CGPoint(x: 1.00, y: 0.70 - adjustment),
            control: CGPoint(x: 1.00, y: 0.75 - adjustment)
        ),
        Segment(
            line:    CGPoint(x: 1.00, y: 0.30 + adjustment),
            curve:   CGPoint(x: 0.95, y: 0.20 + adjustment),
            control: CGPoint(x: 1.00, y: 0.25 + adjustment)
        )
    ]
}

```

### Draw the Badge Background

&emsp;使用 SwiftUI 中的图形 API 绘制自定义标志形状。

&emsp;创建一个名字为 BadgeBackground.swift 的 SwiftUI View，

&emsp;在 BadgeBackground.swift 中，向 badge 添加一个 Path 形状，然后应用 fill() 修饰符将该形状转换为视图。你可以使用 Paths
来组合线条、曲线和其他绘图基元，以形成更复杂的形状，如 badge 的六边形背景。

&emsp;向路径添加一个起点，假设容器大小为 100 x 100 px。
















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
