# 2023年末 - iOS 实时面试题：如何在低版本 iOS 中使用仅高版本支持的 SwiftUI API？

&emsp;1⃣️：例如 `Color.cyan` 是 iOS 15 后才支持的，那么我们的 iOS 14 起的 SwiftUI 项目就不支持 `cyan` 颜色了，那么我们可以在 Color 的 Extension 中添加一个兼容的 `cyan` 颜色。

```c++
            Text("Hello, world!")
//                .foregroundColor(.cyan)
                .foregroundColor(.newCyan)
                
extension Color {
    static let newCyan: Self = {
        if #available(iOS 15.0, *) {
            return .cyan
        } else {
            return Color("cyan")
        }
    }()
}
```

&emsp;2⃣️：例如 `listRowSeparator(.hidden)` 隐藏 `List` 的分割线是 iOS 15 后才支持的，那么我们可以给 View 添加一个 Extension 函数来兼容。

```c++
//        .listRowSeparator(.hidden)
        .newHiddenListRowSeparator()
        
extension View {
    func newHiddenListRowSeparator() -> some View {
        if #available(iOS 15.0, *) {
            return listRowSeparator(.hidden)
        } else {
            return self
        }
    }
}
```

&emsp;3⃣️：例如：`scrollDisabled(true)` 禁止 ScrollView 滚动是 iOS 16 后才支持的，那么我们可以给 View 添加一个 Extension 函数来兼容。

```c++
//            .scrollDisabled(true)
            .newScrollDisabled(true)

extension View {
    func newScrollDisabled(_ disabled: Bool) -> some View {
        if #available(iOS 16.0, *) {
            return scrollDisabled(disabled)
        } else {
            return self
        }
    }
}
```

&emsp;4⃣️：例如：`.tint(Color.red)` iOS 16 以后才支持，那么我们可以给 View 添加一个 Extension 函数来兼容。

```c++
//                .tint(Color.red)
                .newTint(Color.red)
                
extension View {
    func newTint(_ color: Color) -> some View {
        if #available(iOS 16.0, *) {
            return tint(color)
        } else {
            return accentColor(color)
        }
    }
}
```

## 参考链接
**参考链接:🔗**
+ [一种比较好的 SwiftUI API 兼容方案](https://zhuanlan.zhihu.com/p/663966195)
+ [如何 HotReload Objective C 代码——用 SwiftUI](https://hite.me/how-to-hotReload-Objective-c/)
+ [如何 HotReload Objective-C 代码——用 SwiftUI](https://www.jianshu.com/p/50e99b38615b)
