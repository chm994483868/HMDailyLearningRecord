# SwiftUI 学习笔记

&emsp;SwiftUI 官方教程：[SwiftUI Tutorials](https://developer.apple.com/tutorials/swiftui) 仅是几个体现 SwiftUI 简单使用的小 demo 而已，简单易学，循序渐进，先看完可以对 SwiftUI 有一个大概的认知。

## 一：Creating and Combining Views

&emsp;[SwiftUI Essentials - Creating and Combining Views](https://developer.apple.com/tutorials/swiftui/creating-and-combining-views) 创建和组合 Views。

&emsp;首先 `LandmarksApp.swift` 和 `ContentView.swift` 两个初始文件，看到一个从没见过的关键字：`some`，那么 `some` 的作用是什么呢？

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

> &emsp;SwiftUI 高度依赖 Swift 5.1 引入的一个强大特性，它叫 "opaque return types" ，它可以用于函数、方法和属性返回一些值，无需向调用 API 的客户端揭示该值的具体类型。每一次你看到 some View 的地方就是它了。它表示 "某个遵循View协议的特定类型，但我们不必说具体是什么"。
> &emsp;some View 说的是："它将返回一个特定的 view 类型，比如 Button 或者 Text，但我不想说具体是啥。"
> &emsp;如果我们在 `ContentView` 中插入一个 Button：`Button("Touch Me") { print(type(of: self.body)) } .frame(width: 50, height: 50).background(Color.red)` 点击 Button 时打印：`type(of: self.body)`，我们会得到一个超长的输出：
> &emsp;`TupleView<(ModifiedContent<Text, _PaddingLayout>, ModifiedContent<ModifiedContent<Button<Text>, _FrameLayout>, _BackgroundStyleModifier<Color>>)>` 看到是由 ModifiedContent 包裹当前 body 中各组件，然后组成 TupleView，即这个 self.body 的类型是和当前它内部的组件的布局决定的。

&emsp;[[SwiftUI 知识碎片] 为什么 SwiftUI 用 “some View” 作为视图类型?](https://zhuanlan.zhihu.com/p/105213050)
[SwiftUI 中的some关键字](https://www.jianshu.com/p/6eef60ab14bc)

&emsp;这不会在 Swift 中编译，因为 swift 不能把带有关联类型的协议类型作为返回类型。这个时候就可以使用 some 关键字，添加这个关键字代表，你和编译器都确定这个函数总会返回一个特定的具体类型-只是你不知道是哪一种
















## 参考链接
**参考链接:🔗**
+ [iOS内存映射mmap详解](https://www.jianshu.com/p/13f254cf58a7)
+ [Swift编译器中间码SIL类型系统](https://www.jianshu.com/p/fb6923e3a7be)
+ [Combine与SwiftUI](https://www.jianshu.com/p/9a8603ca77a1)
+ [Swift Combine 入门导读](https://icodesign.me/posts/swift-combine/)
+ [Swift UI 学习资料](https://juejin.cn/post/6844903912928083975)
