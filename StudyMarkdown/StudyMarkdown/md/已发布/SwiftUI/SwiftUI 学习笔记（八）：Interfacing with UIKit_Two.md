# SwiftUI 学习笔记（八）：Interfacing with UIKit.md

&emsp;SwiftUI 官方教程：[SwiftUI Tutorials](https://developer.apple.com/tutorials/swiftui) 仅是几个体现 SwiftUI 简单使用的小 demo 而已，简单易学，循序渐进，先看完可以对 SwiftUI 有一个大概的认知。

## 八：Interfacing with UIKit

&emsp;[Framework Integration - Interfacing with UIKit](https://developer.apple.com/tutorials/swiftui/interfacing-with-uikit) 与 UIKit 交互。

&emsp;SwiftUI 与所有 Apple 平台上的现有 UI 框架无缝协作。例如，你可以将 UIKit views 和 view controllers 放置在 SwiftUI views 中，反之亦然。

&emsp;本教程介绍如何将特色 landmark 从 home screen 转换为 UIPageViewController 和 UIPageControl 的包装实例。你将使用 UIPageViewController 来显示 SwiftUI views 的轮播，并使用状态变量和绑定在整个用户界面中协调数据更新。

&emsp;按照步骤生成此项目，或下载完成的项目自行探索。

### Create a View to Represent a UIPageViewController

&emsp;要在 SwiftUI 中表示 UIKit 视图和视图控制器，请创建符合 UIViewRepresentable 和 UIViewControllerRepresentable 协议的类型。你的自定义类型创建和配置它们所代表的 UIKit 类型，而 SwiftUI 管理其生命周期并在需要时更新它们。

&emsp;在项目的 Views 文件夹中创建一个 PageView 文件夹，并添加一个名为 PageViewController.swift 的新 Swift 文件;将 PageViewController 类型声明为符合 UIViewControllerRepresentable。page view controller 存储 Page 实例数组，这些实例必须是一种 View 类型。这些是你用于在地标之间滚动的页面。

&emsp;接下来，添加 UIViewControllerRerepresentationable 协议的两个要求。

&emsp;添加一个 `makeUIViewController(context:)` 方法，以创建具有所需配置的 UIPageViewController 的方法。SwiftUI 在准备好显示视图时调用此方法一次，然后管理视图控制器的生命周期。

&emsp;添加一个 `updateUIViewController(_:context:)` 方法，内部调用 `setViewControllers(_:direction:animated:)` 方法时以提供用于显示的 view controller（pageViewController）。现在，你将创建 UIHostingController，该控制器在每次更新时托管页面 SwiftUI 视图。稍后，通过在 page view controller 的生命周期内仅初始化一次控制器，可以提高效率。

```swift
import SwiftUI
import UIKit

struct PageViewController<Page: View>: UIViewControllerRepresentable {
    var pages: [Page]
    
    func makeUIViewController(context: Context) -> UIPageViewController {
        let pageViewController = UIPageViewController(
            transitionStyle: .scroll,
            navigationOrientation: .horizontal)
        
        return pageViewController
    }
    
    func updateUIViewController(_ pageViewController: UIPageViewController, context: Context) {
        pageViewController.setViewControllers([UIHostingController(rootView: pages[0])], direction: .forward, animated: true)
    }
}
```

&emsp;在继续之前，请准备要用作 page 的 card。

&emsp;将下载的项目文件的 Resources 目录中的图像拖到应用的 Asset catalog 中。landmark 的 feature image（如果存在）的尺寸与常规图像不同。

&emsp;将返回 feature image 的 Landmark 结构体（如果存在）添加计算属性。

```swift
...
var featureImage: Image? {
    isFeatured ? Image(imageName + "_feature") : nil
}
...
```

&emsp;添加一个名为 FeatureCard 的新 SwiftUI View 文件，该文件用于显示 landmark’s feature image。

&emsp;在图像上叠加有关 landmark 的文本信息。

```swift
import SwiftUI

struct FeatureCard: View {
    var landmark: Landmark
    
    var body: some View {
        landmark.featureImage?
            .resizable()
            .aspectRatio(3 / 2, contentMode: .fit)
            .overlay {
                TextOverlay(landmark: landmark)
            }
    }
}

struct TextOverlay: View {
    var landmark: Landmark
    
    var gradient: LinearGradient {
        .linearGradient(
            Gradient(colors: [.black.opacity(0.6), .black.opacity(0)]),
            startPoint: .bottom,
            endPoint: .center)
    }
    
    var body: some View {
        ZStack(alignment: .bottomLeading) {
            gradient
            VStack(alignment: .leading) {
                Text(landmark.name)
                    .font(.title)
                    .bold()
                Text(landmark.park)
            }
            .padding()
        }
        .foregroundColor(.white)
    }
}

struct FeatureCard_Previews: PreviewProvider {
    static var previews: some View {
        FeatureCard(landmark: ModelData().features[0])
    }
}
```

&emsp;接下来，你将创建一个自定义视图来展示你的 UIViewControllerRepresentable 视图。

&emsp;创建一个名为 PageView 的新 SwiftUI 视图文件，并更新 PageView 类型以将 PageViewController 声明为子视图。预览失败，因为 Xcode 无法推断出 Page 的类型。更新 preview provide 以传递所需的视图数组，此时预览便可开始正常工作。

```swift
import SwiftUI

struct PageView<Page: View>: View {
    var pages: [Page]
    
    var body: some View {
        PageViewController(pages: pages)
    }
}

struct PageView_Previews: PreviewProvider {
    static var previews: some View {
        PageView(pages: ModelData().features.map({ FeatureCard(landmark: $0) }))
            .aspectRatio(3 / 2, contentMode: .fit)
    }
}
```

### Create the View Controller’s Data Source

&emsp;在短短的几个步骤中，你已经做了很多事情 —— PageViewController 使用 UIPageViewController 来显示来自 SwiftUI 视图的内容。现在是时候启用滑动交互以从一个 page 移动到另一个 page 了。

&emsp;表示 UIKit 视图控制器的 SwiftUI 视图可以定义 SwiftUI 管理的 Coordinator 类型，并将其作为 representable 视图上下文的一部分提供。

&emsp;在 PageViewController 中声明一个嵌套的 Coordinator 类。SwiftUI 管理你的 UIViewControllerRepresentable 类型的 coordinator，并在调用上面创建的方法时将其作为上下文的一部分提供。

&emsp;将另一个方法添加到 PageViewController 以创建 coordinator。

&emsp;SwiftUI 在 `makeUIViewController(context:)` 之前调用此 `makeCoordinator()` 方法，以便你在配置视图控制器时可以访问 coordinator 对象。

&emsp;你可以使用此 coordinator 实现常见的 Cocoa 模式，例如委托（delegates）、数据源（data sources）以及通过 target-action 响应用户事件。

&emsp;使用视图的 pages 数组初始化 coordinator 中的控制器数组。

&emsp;将 UIPageViewControllerDataSource conformance 添加到 Coordinator 类型，并实现两个必需的方法。这两种方法建立视图控制器之间的关系，以便你可以在它们之间来回轻扫。

&emsp;添加 coordinator 作为 UIPageViewController 的数据源。

&emsp;返回 PageView，打开实时预览，并测试滑动交互。

```swift
import SwiftUI
import UIKit

struct PageViewController<Page: View>: UIViewControllerRepresentable {
    var pages: [Page]
    
    func makeCoordinator() -> Coordinator {
        Coordinator(self)
    }
    
    func makeUIViewController(context: Context) -> UIPageViewController {
        let pageViewController = UIPageViewController(
            transitionStyle: .scroll,
            navigationOrientation: .horizontal)
        pageViewController.dataSource = context.coordinator
        
        return pageViewController
    }
    
    func updateUIViewController(_ pageViewController: UIPageViewController, context: Context) {
        pageViewController.setViewControllers([context.coordinator.controllers[0]], direction: .forward, animated: true)
    }
    
    class Coordinator: NSObject, UIPageViewControllerDataSource {
        var parent: PageViewController
        var controllers = [UIViewController]()
        
        init(_ pageViewController: PageViewController) {
            parent = pageViewController
            controllers = parent.pages.map { UIHostingController(rootView: $0) }
        }
        
        func pageViewController(_ pageViewController: UIPageViewController, viewControllerBefore viewController: UIViewController) -> UIViewController? {
            guard let index = controllers.firstIndex(of: viewController) else {
                return nil
            }
            
            if index == 0 {
                return controllers.last
            }
            
            return controllers[index - 1]
        }
        
        func pageViewController(_ pageViewController: UIPageViewController, viewControllerAfter viewController: UIViewController) -> UIViewController? {
            guard let index = controllers.firstIndex(of: viewController) else {
                return nil
            }
            
            if index + 1 == controllers.count {
                return controllers.first
            }
            
            return controllers[index + 1]
        }
    }
}
```

### Track the Page in a SwiftUI View’s State

&emsp;要准备添加自定义 UIPageControl，你需要一种从 PageView 中跟踪当前 Page 的方法。为此，你需要在 PageView 中声明一个 @State 属性，并将此属性的绑定向下传递到 PageViewController 视图。PageViewController 更新绑定以匹配可见 page。

&emsp;首先添加一个 currentPage 绑定作为 PageViewController 的属性。除了声明 @Binding 属性之外，还可以更新对 `setViewControllers(_:direction:animated:)` 的调用，传递 currentPage 绑定的值。

```swift
...
@Binding var currentPage: Int
...
    func updateUIViewController(_ pageViewController: UIPageViewController, context: Context) {
        pageViewController.setViewControllers(
            [context.coordinator.controllers[currentPage]], direction: .forward, animated: true)
    }
```
&emsp;在 PageView 中声明 @State 变量，并在创建子 PageViewController 时将绑定传递给该属性。请记住使用 $ 语法创建与存储为 state 的值的绑定。

```swift
...
@State private var currentPage = 0
...
    var body: some View {
        PageViewController(pages: pages, currentPage: $currentPage)
    }
```

&emsp;通过更改其初始值来测试该值是否通过绑定流向 PageViewController。向 PageView 添加一个按钮，使 page view controller 跳转到第二个视图。

&emsp;添加带有 currentPage 属性的文本视图，以便你可以密切关注 @State 属性的值。请注意，当你从一页滑动到另一页时，该值不会改变。

```swift
...
    var body: some View {
        VStack {
            PageViewController(pages: pages, currentPage: $currentPage)
            Text("Current Page: \(currentPage)")
        }
    }
...
```

&emsp;在 PageViewController.swift 中，使 Coordinator 遵循 UIPageViewControllerDelegate 协议，并添加 `pageViewController(_:didFinishAnimating:previousViewControllers:transitionCompleted completed: Bool)` 方法。

```swift
...
class Coordinator: NSObject, UIPageViewControllerDataSource, UIPageViewControllerDelegate {
...
        func pageViewController(
            _ pageViewController: UIPageViewController,
            didFinishAnimating finished: Bool,
            previousViewControllers: [UIViewController],
            transitionCompleted completed: Bool) {
            if completed,
               let visibleViewController = pageViewController.viewControllers?.first,
               let index = controllers.firstIndex(of: visibleViewController) {
                parent.currentPage = index
            }
        }
}
...
```

&emsp;除了 dataSource 之外，还将 coordinator 分配为 UIPageViewController 的委托。在两个方向上绑定链接后，文本视图将在每次轻扫后更新以显示正确的页码。

```swift
pageViewController.delegate = context.coordinator
```

### Add a Custom Page Control

&emsp;你已准备好将自定义 UIPageControl 添加到你的视图中，包装在 SwiftUI UIViewRepresentable 视图中。

&emsp;创建一个名为 PageControl 的新 SwiftUI 视图文件。更新 PageControl 类型以符合 UIViewRepresentable 协议。

&emsp;UIViewRepresentable 和 UIViewControllerRepresentable 类型具有相同的生命周期，其方法对应于它们的底层 UIKit 类型。

```swift
import SwiftUI
import UIKit

struct PageControl: UIViewRepresentable {
    var numberOfPages: Int
    @Binding var currentPage: Int

    func makeUIView(context: Context) -> UIPageControl {
        let control = UIPageControl()
        control.numberOfPages = numberOfPages

        return control
    }

    func updateUIView(_ uiView: UIPageControl, context: Context) {
        uiView.currentPage = currentPage
    }
}
```

&emsp;将 text box 替换为 page control，从 VStack 切换到 ZStack 进行布局。因为你将页数和绑​​定传递给当前页面，所以页面控件已经显示了正确的值。

```swift
...
    var body: some View {
        ZStack(alignment: .bottomTrailing) {
            PageViewController(pages: pages, currentPage: $currentPage)
            PageControl(numberOfPages: pages.count, currentPage: $currentPage)
                .frame(width: CGFloat(pages.count * 18))
                .padding(.trailing)
        }
    }
...
```

&emsp;接下来，使 page control 具有交互性，以便用户可以点击一侧或另一侧在页面之间移动。

&emsp;在 PageControl 中创建一个嵌套的 Coordinator 类型，并添加一个 makeCoordinator() 方法来创建和返回一个新的 coordinator。

&emsp;由于 UIControl 子类（如 UIPageControl）使用 target-action 模式而不是 delegation，因此此 Coordinator 实现 @objc 方法来更新 currentPage 绑定。

```swift
...
    func makeCoordinator() -> Coordinator {
        Coordinator(self)
    }
...
    class Coordinator: NSObject {
        var control: PageControl

        init(_ control: PageControl) {
            self.control = control
        }

        @objc
        func updateCurrentPage(sender: UIPageControl) {
            control.currentPage = sender.currentPage
        }
    }
```

&emsp;添加 coordinator 作为 valueChanged 事件的 target，指定 `updateCurrentPage(sender:)` 方法作为要执行的 action。

```swift
...
        control.addTarget(
            context.coordinator,
            action: #selector(Coordinator.updateCurrentPage(sender:)),
            for: .valueChanged)
...
```

&emsp;最后，在 CategoryHome 中，将 placeholder feature image 替换为新的 page View。

```swift
...
                PageView(pages: modelData.features.map { FeatureCard(landmark: $0) })
                    .aspectRatio(3 / 2, contentMode: .fit)
                    .listRowInsets(EdgeInsets())
...
```

&emsp;现在尝试所有不同的交互 —— PageView 展示了 UIKit 和 SwiftUI 视图和控制器如何协同工作。

## 参考链接
**参考链接:🔗**
+ [Interfacing with UIKit](https://developer.apple.com/tutorials/swiftui/interfacing-with-uikit)



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





