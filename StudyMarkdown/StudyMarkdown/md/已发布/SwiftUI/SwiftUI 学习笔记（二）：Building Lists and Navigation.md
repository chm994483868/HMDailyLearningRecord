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
    
    // 嵌套定义一个记录经纬度的结构体 Coordinates，并且遵循 Codable
    struct Coordinates: Hashable, Codable {
        var latitude: Double
        var longitude: Double
    }
}
```

&emsp;看到 Landmark、Coordinates 结构体都遵循了 Codable 协议，在接下来的学习之前，我们先对 Swift 4.0 推出的 Codable 协议进行学习。

### Codable

&emsp;Apple 官方在 Swift 4.0 的标准库中，引入了 Codable，它实际上是：`public typealias Codable = Decodable & Encodable`，即把编码和解码的功能综合在一起，它能够将程序内部的数据结构（结构体、枚举、类）序列化成可交换数据（Json 字符串），也能够将通用数据格式（Json 字符串）反序列化为内部使用的数据结构（结构体、枚举、类），即在 Json 这种弱类型数据和代码中使用的强类型数据之间相互转换，大大提升对象和其表示之间互相转换的体验。

&emsp;或者更直白的理解为官方下场来和 YYModel、MJExtension、SwiftyJSON... 等这些第三方的 Json 数据转换库卷。

&emsp;下面从 Codable 协议为我们带来的默认功能开始学习。

&emsp;Decodable 协议定义了一个初始化函数，遵从 Decodable 协议的类型可以使用任何 Decoder（let decoder = JSONDecoder()）对象进行初始化，完成一个解码过程。

```swift
// A type that can decode itself from an external representation.
public protocol Decodable {
    // Creates a new instance by decoding from the given decoder.
    // This initializer throws an error if reading from the decoder fails, or if the data read is corrupted or otherwise invalid.
    init(from decoder: Decoder) throws
}
```

&emsp;Encodable 协议定义了一个编码方法，遵从 Encodable 协议的类型的实例可以使用任何 Encoder（let encoder = JSONEncoder()）对象创建表示（Data），完成一个编码过程。

```swift
// A type that can encode itself to an external representation.
public protocol Encodable {
    // Encodes this value into the given encoder.
    // If the value fails to encode anything, `encoder` will encode an empty keyed container in its place.
    // This function throws an error if any values are invalid for the given encoder's format.
    func encode(to encoder: Encoder) throws
}
```

&emsp;只要类型遵循 Codable 协议，那么就会默认支持 Codable 协议的 init(from:) 和 encode(to:) 方法。由于 Swift 标准库中的类型，比如 String、Int、Double 和 Foundation 框架中 Data、Date、URL 都是默认支持 Codable 协议的，所以我们自定义的结构体模型只需声明遵循 Codable 协议即可，便可获得编码和解码能力。

&emsp;[JSONDecoder的使用](https://www.jianshu.com/p/4e7f0feeeb94)

&emsp;由于 Codable 协议被设计出来用于替代 NSCoding 协议，所以遵从 Codable 协议的对象就可以无缝的支持 NSKeyedArchiver 和 NSKeyedUnarchiver 对象进行 Archive&UnArchive 操作，把结构化的数据通过简单的方式持久化和反持久化。原有的解码过程和持久化过程需要单独处理，现在通过新的 Codable 协议一起搞定，大大提高了效率。

#### Codable 默认实现 

&emsp;以上面的 Landmark 结构体（这里为了测试数组类型的解码，给 Landmark 添加了一个数组类型的成员变量：`var coordinatesArray: [Coordinates]`）和 landmarkData.json 中一个景点的原始 Json 数据为例：

```swift
let turtleRockString = """
    {
        "id": 1001,
        "name": "Turtle Rock",
        "park": "Joshua Tree National Park",
        "state": "California",
        "description": "Suscipit ...",
        "imageName": "turtlerock",
        "coordinates": {
            "longitude": -116.166868,
            "latitude": 34.011286
        },
        "coordinatesArray": [
            {
                "longitude": -1,
                "latitude": 3
            },
            {
                "longitude": -11,
                "latitude": 34
            }
        ]
    }
"""

func decode<T: Decodable>(_ jsonString: String) -> T {
    guard let data = jsonString.data(using: .utf8) else {
        fatalError("\(jsonString) cannot be converted to Data")
    }
    
    do {
        let decoder = JSONDecoder()
        return try decoder.decode(T.self, from: data)
    } catch {
        fatalError("Couldn't parse \(jsonString) as \(T.self):\n\(error)")
    }
}

func encode<T: Encodable>(_ model: T) -> String {
    let data: Data
    
    do {
        let encoder = JSONEncoder()
        encoder.outputFormatting = .prettyPrinted
        data = try encoder.encode(model)
    } catch {
        fatalError("\(model) cannot be converted to Data")
    }
    
    guard let string = String(data: data, encoding: .utf8) else {
        fatalError("\(data) cannot be converted to string")
    }
    
    return string
}

let turtleRock: Landmark = decode(turtleRockString)
let encodeTurtleRockString: String = encode(turtleRock)
```

&emsp;decode 函数中整个解码的过程非常简单，创建一个解码器（JSONDecoder()），这个解码器的 decode 方法需要传入两个参数，第一个参数指定 data 转成的数据结构的类型，这个类型是将弱类型（Json 数据）转换成强类型的关键，第二个参数传入原始的 data 数据。

&emsp;编码过程与解码过程基本对应，系统提供了一个 JSONEncoder 对象用于编码。创建编码器，然后传入值给它进行编码，编码器通过 Data 实例的方式返回一个字节的集合，这里为了方便显示，我们将它转为了字符串返回。

&emsp;然后我们直接打印 `turtleRock` 实例，便可看到 `turtleRockString` Json 串中的数据已经全部转换到 Landmark 结构体实例中。然后我们直接打印 `encodeTurtleRockString` 字符串，便可看到 turtleRock 实例的成员变量可被编码为一个结构化的 Json 字符串。

```swift
// print(turtleRock)
Landmark(id: 1001, name: "Turtle Rock", park: "Joshua Tree National Park", state: "California", description: "Suscipit ...", imageName: "turtlerock", coordinates: Landmarks.Landmark.Coordinates(latitude: 34.011286, longitude: -116.166868), coordinatesArray: [Landmarks.Landmark.Coordinates(latitude: 3.0, longitude: -1.0), Landmarks.Landmark.Coordinates(latitude: 34.0, longitude: -11.0)])

// print(encodeTurtleRockString)
{
  "coordinates" : {
    "longitude" : -116.16686799999999,
    "latitude" : 34.011285999999998
  },
  "coordinatesArray" : [
    {
      "longitude" : -1,
      "latitude" : 3
    },
    {
      "longitude" : -11,
      "latitude" : 34
    }
  ],
  "id" : 1001,
  "park" : "Joshua Tree National Park",
  "description" : "Suscipit ...",
  "imageName" : "turtlerock",
  "state" : "California",
  "name" : "Turtle Rock"
}
```

&emsp;这里看到了一个细节点，原始 Json 字符串中 "coordinates": { "longitude": -116.166868, "latitude": 34.011286 } 的经纬度值是 -116.166868/34.011286 当转换为 Landmark 实例时还是同样的值同样的精确度，但是当我们把 Landmark 实例编码为 Json 字符串时，发现精度值发生了变化：-116.16686799999999/34.011285999999998。（暂时不知道如何处理这种精确度丢失问题） 

&emsp;除了上面 Json 字符串中都是基础类型的键值对外，还有其他一些特殊情况： 

1. Json（JavaScript Object Notation）字符串中存在嵌套（对象、字典）

&emsp;上面的示例中 Landmark 结构体还嵌套了一个 Coordinates 结构体，只要 Coordinates 同样也遵循 Codable，那么就能从 `turtle_rockString` json 串中直接解析出经纬度的值赋值到 Landmark 结构体的 `var coordinates: Coordinates` 成员变量中。

2. Json 字符串中包含数组（数组中的模型要遵循 Codable）

&emsp;上面的示例中 Landmark 结构体中的 `var coordinatesArray: [Coordinates]` 成员变量，数据也得到了正确的解析。

&emsp;针对上面的 1 和 2 条，由于 Swift 4.0 支持条件一致性，所有当数组（Array）中每个元素遵从 Codable 协议、字典（Dictionary）中对应的 key 和 value 遵从 Codable 协议，整体对象就遵从 Codable 协议。

&emsp;在 Swift/Collection/Array 中可看到 Array 遵循 Codable 协议：

```swift
extension Array : Encodable where Element : Encodable {
    public func encode(to encoder: Encoder) throws
}

extension Array : Decodable where Element : Decodable {
    public init(from decoder: Decoder) throws
}
```

&emsp;在 Swift/Collection/HashedCollections/Dictionary 中看到 Dictionary 遵循 Codable 协议：

```swift
extension Dictionary : Encodable where Key : Encodable, Value : Encodable {
    // 这句注释超重要，下面我们会学习到 keyed container 和 unkeyed container
    // If the dictionary uses `String` or `Int` keys, the contents are encoded in a keyed container. Otherwise, the contents are encoded as alternating key-value pairs in an unkeyed container.
    public func encode(to encoder: Encoder) throws
}

extension Dictionary : Decodable where Key : Decodable, Value : Decodable {
    public init(from decoder: Decoder) throws
}
```

3. Json 字符串是一个模型数组时，如下形式时，此时在 `return try decoder.decode(T.self, from: data)` 中传入类型时需要传输数组类型，例如: `[Landmark]`

```swift
[
    {
        "id": 1001,
        "name": "Turtle Rock",
        ...
    },
    {
        "id": 1002,
        "name": "Silver Salmon Creek",
        ...
    },
]
```

&emsp;在下面的 ModelData.swift 文件中：`var landmarks: [Landmark] = load("landmarkData.json")` 正是，从 landmarkData.json 文件中读出一个模型数组的 Json 字符串，然后解码为一个 Landmark 数组。

4. Json 字符串中有 Optional values 时，此时在模型定义时也指定对应的成员变量为可选类型即可。

```swift
let turtle_rockString = """
    {
        "id": 1001,
        "name": null,
        "park": "Joshua Tree National Park",
        ...
    }
"""
```

```swift
struct Landmark: Hashable, Codable, Identifiable {
    var id: Int
    var name: String?
    var park: String
    ...
}
```

&emsp;Json 字符串中 name 为可选，那么在 Landmark 中把 name 成员变量定义为一个可选类型。

5. Json 字符串中存在嵌套对象，且此对象有可能是个 空对象 时，如下把上面的 turtleRockString 字符串中的 coordinates 置为一个空对象：

```swift
let turtleRockString = """
    {
        ...
        "coordinates": {},
        ...
    }
"""
```

&emsp;此时我们如果直接运行的话就会报一个错误：

```swift
keyNotFound(CodingKeys(stringValue: "latitude", intValue: nil), Swift.DecodingError.Context(codingPath: [CodingKeys(stringValue: "coordinates", intValue: nil)], debugDescription: "No value associated with key CodingKeys(stringValue: \"latitude\", intValue: nil) (\"latitude\").", underlyingError: nil))
```

```swift
keyNotFound(CodingKeys(stringValue: "latitude", intValue: nil), Swift.DecodingError.Context(codingPath: [CodingKeys(stringValue: "coordinates", intValue: nil)], debugDescription: "No value associated with key CodingKeys(stringValue: \"latitude\", intValue: nil) (\"latitude\").", underlyingError: nil))
```

&emsp;那么我们会联想到上面的 Optional values 会想到把 结构体中 `private var coordinates: Coordinates` 成员变量设置为可选类型，再次运行会发现依然报错。

#### Codable 进阶

&emsp;上面的嵌套、数组类型的成员变量、可选的成员变量、Json 字符串本身是模型数组，这四种情况中都是采用了 Codable 的默认实现，我们不需要添加任何自定义操作，Codable 自动帮我们完成了数据到模型的转换。那有哪些需要我们自定义的操作才能完成数据到模型的转换呢？下面一起来梳理一下。

&emsp;虽然 Codable 的默认实现足够应付大多数情形了，但是有时候我们还是存在一些自定义需求。为了处理这类自定义问题，我们就必须自己覆盖默认的 Codable 实现。







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
+ [Swift之Codable实战技巧](https://zhuanlan.zhihu.com/p/50043306)
+ [Swift 4 JSON 解析进阶](https://blog.csdn.net/weixin_33962923/article/details/88986627)
