# Optional 学习笔记

```swift
@frozen public enum Optional<Wrapped> : ExpressibleByNilLiteral {

    /// The absence of a value.
    ///
    /// In code, the absence of a value is typically written using the `nil` literal rather than the explicit `.none` enumeration case.
    case none

    /// The presence of a value, stored as `Wrapped`.
    case some(Wrapped)

    /// Creates an instance that stores the given value.
    public init(_ some: Wrapped)
    
    ...
}
```

&emsp;Optional 是一个枚举类型，并且是一个有范型关联值的枚举。



















## 参考链接
**参考链接:🔗**
+ [Optional Chaining](https://docs.swift.org/swift-book/LanguageGuide/OptionalChaining.html)
+ [Swift枚举关联值的内存探究](https://www.jianshu.com/p/1cf8fc66959f)
+ [@frozen Swift（SwiftUI中文文档手册）](https://www.jianshu.com/p/f898728b5834)
