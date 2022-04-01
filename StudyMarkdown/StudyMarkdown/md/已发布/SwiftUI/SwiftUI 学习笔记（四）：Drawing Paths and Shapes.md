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

&emsp;move(to:) 方法将绘图光标移动到形状的边界内，就好像一支假想的钢笔或铅笔悬停在该区域上，等待开始绘制。

&emsp;为形状数据的每个点绘制线条以创建粗略的六边形形状。addLine(to:) 方法获取单个点并绘制它。对 addLine(to:) 的连续调用在前一点开始一条线，然后继续到新点。

&emsp;如果你的六边形看起来有点不寻常，请不要担心，这是因为你忽略了形状拐角处每个段的弯曲部分。接下来，将对此进行说明。

&emsp;使用 addQuadCurve(to:control:) 方法为 badge 的角绘制贝塞尔曲线的方法。

&emsp;在 GeometryReader 中包装路径，以便 badge 可以使用其包含视图的大小，该视图定义大小，而不是对值（100）进行硬编码。使用几何图形的两个维度中的最小部分，可以在其包含视图不是正方形时保留 badge 的纵横比。

&emsp;使用 xScale 在 x 轴上缩放形状，然后添加 xOffset 以在其几何图形中重新定位形状。

&emsp;将纯黑色背景替换为渐变以匹配设计。

&emsp;应用 `aspectRatio(_:contentMode:)` 渐变填充的修饰符。

&emsp;通过保留 1：1 的纵横比，badge 将保持其在视图中心的位置，即使其祖先视图不是正方形的也是如此。

```swift
import SwiftUI

struct BadgeBackground: View {
    var body: some View {
        GeometryReader { geometry in
            Path { path in
                var width: CGFloat = min(geometry.size.width, geometry.size.height)
                let height = width
                
                let xScale: CGFloat = 0.832
                let xOffset = (width * (1.0 - xScale)) / 2.0
                
                width *= xScale
                path.move(
                    to: CGPoint(
                        x: width * 0.95 + xOffset,
                        y: height * (0.20 + HexagonParameters.adjustment)
                    )
                )
                
                HexagonParameters.segments.forEach { segment in
                    path.addLine(
                        to: CGPoint(
                            x: width * segment.line.x + xOffset,
                            y: height * segment.line.y
                        )
                    )
                    
                    path.addQuadCurve(
                        to: CGPoint(
                            x: width * segment.curve.x,
                            y: height * segment.curve.y
                        ),
                        control: CGPoint(
                            x: width * segment.control.x + xOffset,
                            y: height * segment.control.y
                        )
                    )
                }
            }
            .fill(.linearGradient(
                Gradient(colors: [Self.gradientStart, Self.gradientEnd]),
                startPoint: UnitPoint(x: 0.5, y: 0),
                endPoint: UnitPoint(x: 0.5, y: 0.6)))
        }
        .aspectRatio(1, contentMode: .fit)
    }
    
    static let gradientStart = Color(red: 239.0 / 255, green: 120.0 / 255, blue: 221.0 / 255)
    static let gradientEnd = Color(red: 239.0 / 255, green: 172.0 / 255, blue: 120.0 / 255)
}
```

### Draw the Badge Symbol

&emsp;Landmarks badge 的中心有一个自定义 insignia 基于 Landmarks 应用图标中显示的山峰。

&emsp;mountain symbol 由两种形状组成：一种表示山顶的 snowcap，另一种表示沿途的植被。将使用两个部分三角形的形状来绘制它们，这些形状由一个小间隙分开。

&emsp;首先，你将为你的应用提供一个图标，以建立徽章的外观。

&emsp;把下载的图标资源拖入项目中。

&emsp;创建一个自定义视图：BadgeSymbol.swift。

&emsp;使用路径 API 绘制符号的顶部。调整与间距、topWidth 和 topHeight 常量关联的数字乘数，以查看它们如何影响整体形状。

&emsp;绘制符号的底部。

&emsp;使用 move(to:) 修饰符，用于在同一路径中的多个形状之间插入间隙。

&emsp;用设计中的紫色填充符号。

```swift
import SwiftUI

struct BadgeSymbol: View {
    static let symbolColor = Color(red: 79.0 / 255, green: 79.0 / 255, blue: 191.0 / 255)
    
    var body: some View {
        GeometryReader { geometry in
            Path { path in
                let width = min(geometry.size.width, geometry.size.height)
                let height = width * 0.75
                let spacing = width * 0.030
                let middle = width * 0.5
                let topWidth = width * 0.226
                let topHeight = height * 0.488
                
                path.addLines([
                    CGPoint(x: middle, y: spacing),
                    CGPoint(x: middle - topWidth, y: topHeight - spacing),
                    CGPoint(x: middle, y: topHeight / 2 + spacing),
                    CGPoint(x: middle + topWidth, y: topHeight - spacing),
                    CGPoint(x: middle, y: spacing)
                ])
                
                path.move(to: CGPoint(x: middle, y: topHeight / 2 + spacing * 3))
                path.addLines([
                    CGPoint(x: middle - topWidth, y: topHeight + spacing),
                    CGPoint(x: spacing, y: height - spacing),
                    CGPoint(x: width - spacing, y: height - spacing),
                    CGPoint(x: middle + topWidth, y: topHeight + spacing),
                    CGPoint(x: middle, y: topHeight / 2 + spacing * 3)
                ])
            }
            .fill(Self.symbolColor)
        }
    }
}
```

&emsp;创建新的 RotatedBadgeSymbol 视图以封装旋转符号的概念。调整预览中的角度以测试旋转的效果。

```swift
import SwiftUI

struct RotatedBadgeSymbol: View {
    let angle: Angle
    
    var body: some View {
        BadgeSymbol()
            .padding(-60)
            .rotationEffect(angle, anchor: .bottom)
    }
}
```

### Combine the Badge Foreground and Background

&emsp;badge 设计要求在 badge 背景之上旋转和重复多次山体形状。

&emsp;定义用于旋转的新类型，并利用 ForEach 视图将相同的调整应用于山体形状的多个副本。

&emsp;创建 Badge.swift SwiftUI 视图。

&emsp;将 BadgeBackground 放置在 Badge 的 body 中。

&emsp;将 badge’s symbol 放置在 ZStack 中，将其置于 BadgeBackground 下。

&emsp;现在看来，与预期的设计和背景的相对大小相比，badge symbol 太大了。通过读取周围的几何并缩放符号来校正 badge symbol 的大小。

&emsp;添加 ForEach 视图以旋转和显示 badge symbol 的副本。

&emsp;完整的 360° 旋转分为八个部分，通过重复 mountain symbol 来创建类似太阳的图案。








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
