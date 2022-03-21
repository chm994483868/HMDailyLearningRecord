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
    // var coordinatesArray: [Coordinates] // 测试 Codable，测试 Json 字符串中存在数组类型
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

&emsp;Decodable 协议提供了一个初始化函数，遵从 Decodable 协议的类型可以使用任何 Decoder（let decoder = JSONDecoder()）对象进行初始化，完成一个解码过程。

```swift
// A type that can decode itself from an external representation.
public protocol Decodable {
    // Creates a new instance by decoding from the given decoder.
    // This initializer throws an error if reading from the decoder fails, or if the data read is corrupted or otherwise invalid.
    init(from decoder: Decoder) throws
}
```

&emsp;这里有点绕，我们来分析一下，首先 `struct Landmark` 结构体遵循了 `Decodable` 协议，那么它就有了 `init(from decoder: Decoder) throws` 的能力，然后在我们的解码过程中，我们使用 `let decoder = JSONDecoder()` 构建了一个默认的 JSONDecoder 对象，接着调用它的 `open func decode<T>(_ type: T.Type, from data: Data) throws -> T where T : Decodable` 函数，第一个参数 type 限制为遵循 Decodable 协议，第二个参数 data 则是 Json 字符串转换而来，针对当前的示例，最后返回一个 struct Landmark 结构体实例，如果我们想自定义解码过程的话，则可以重写 struct Landmark 结构体的 init(from decoder: Decoder) throws 函数。

&emsp;开始以为 init(from decoder: Decoder) throws 函数的 decoder 参数就是我们自己构建的 let decoder = JSONDecoder() 对象，发现并不是这样，通过打断点发现是这样的，一个是 open 的系统类 JSONDecoder，一个则是系统内部的私有类 `_JSONDecoder`：

```swift
(lldb) p decoder
(JSONDecoder) $R0 = 0x000060000224c0a0 {
    ...
}
(lldb) p decoder
(Foundation.__JSONDecoder) $R1 = 0x00006000021506c0 {
    ...
}
```   

&emsp;然后又顺着搜了一下发现要卷 Swift 源码才能理清 Codable 的功能，暂且不在本篇捋了，当前的核心是学习 SwiftUI，本篇只学一些 Codable 的常规使用。

&emsp;Encodable 协议提供了一个编码方法，遵从 Encodable 协议的类型的实例可以使用任何 Encoder（let encoder = JSONEncoder()）对象创建表示（Data），完成一个编码过程。

```swift
// A type that can encode itself to an external representation.
public protocol Encodable {
    // Encodes this value into the given encoder.
    // If the value fails to encode anything, `encoder` will encode an empty keyed container in its place.
    // This function throws an error if any values are invalid for the given encoder's format.
    func encode(to encoder: Encoder) throws
}
```

&emsp;只要类型遵循 Codable 协议，那么就会默认支持 Codable 协议的 init(from:) 和 encode(to:) 方法。由于 Swift 标准库中的类型，比如 String、Int、Double 和 Foundation 框架中 Data、Date、URL 都是默认支持 Codable 协议的，所以我们自定义的结构体模型基本只需声明遵循 Codable 协议即可，便可获得编码和解码能力。

&emsp;遵循 Codable 协议的类型，只针对类型的存储属性进行解码和编码，计算属性是不包括在内的（计算属性可以理解为是一个类型的函数，不参与类型的内存布局）。 

&emsp;由于 Codable 协议被设计出来用于替代 NSCoding 协议，所以遵从 Codable 协议的对象就可以无缝的支持 NSKeyedArchiver 和 NSKeyedUnarchiver 对象进行 Archive&UnArchive 操作，把结构化的数据通过简单的方式持久化和反持久化。原有的解码过程和持久化过程需要单独处理，现在通过新的 Codable 协议一起搞定，大大提高了效率。

#### Codable 的默认实现 

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

4. Json 字符串中有 Optional values 时（空值 null），此时在模型定义时也指定对应的成员变量为可选类型即可。

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

&emsp;Json 字符串中 name 为可选，那么在 Landmark 中把 name 成员变量定义为一个可选类型，否则当 Json 字符串中的 name 返回 null 时会打印如下错误信息（valueNotFound，Expected String value but found null instead. 预期为 String 的值却发现了 null）：

```swift
valueNotFound(Swift.String, Swift.DecodingError.Context(codingPath: [CodingKeys(stringValue: "name", intValue: nil)], debugDescription: "Expected String value but found null instead.", underlyingError: nil))
```

5. Json 字符串中存在嵌套对象，且此对象有可能是个 空对象（{}，注意这里和上面的 null 是不同的处理情况）时，如下把上面的 turtleRockString 字符串中的 coordinates 置为一个空对象：

```swift
let turtleRockString = """
    {
        ...
        "coordinates": {},
        ...
    }
"""
```

&emsp;此时我们如果直接运行的话就会打印如下错误信息（keyNotFound，No value associated with key CodingKeys(stringValue: \"latitude\", intValue: nil) (\"latitude\"). 没有找到 CodingKeys 中与 latitude 关联的值）：

```swift
keyNotFound(CodingKeys(stringValue: "latitude", intValue: nil), Swift.DecodingError.Context(codingPath: [CodingKeys(stringValue: "coordinates", intValue: nil)], debugDescription: "No value associated with key CodingKeys(stringValue: \"latitude\", intValue: nil) (\"latitude\").", underlyingError: nil))
```

&emsp;那么我们会联想到上面的 Optional values（空值 null）的情况，会想到把 Landmark 结构体中的 `private var coordinates: Coordinates` 成员变量也设置为可选类型：`Coordinates?`，再次运行会发现依然打印上面同样的错误信息。此时我们定睛一看，错误信息中提到没有找到与 `latitude` 关联的值，那么我们直接把 Coordinates 结构体的 `latitude` 和 `longitude` 两个成员变量设置为可选类型。此时便可正常解码和编码。

```swift
struct Coordinates: Hashable, Codable {
    var latitude: Double?
    var longitude: Double?
}
```

&emsp;把 Coordinates 的每个成员变量设置为可选类型，这样当 coordinates 返回为 {} 时，latitude 和 longitude 自动解析为 nil。（`print(turtleRock.coordinates)`: Coordinates(latitude: nil, longitude: nil)）   

```swift
// print(turtleRock)
Landmark(id: 1001, name: "Turtle Rock", park: "Joshua Tree National Park", state: "California", description: "Suscipit ...", imageName: "turtlerock", coordinates: Optional(Landmarks.Landmark.Coordinates(latitude: nil, longitude: nil)), coordinatesArray: [Landmarks.Landmark.Coordinates(latitude: Optional(3.0), longitude: Optional(-1.0)), Landmarks.Landmark.Coordinates(latitude: Optional(34.0), longitude: Optional(-11.0))])

// coordinatesArray: [Landmarks.Landmark.Coordinates(latitude: 3.0, longitude: -1.0), Landmarks.Landmark.Coordinates(latitude: 34.0, longitude: -11.0)]
// 和上面的对比，coordinatesArray 成员变量中的 Coordinates 的 latitude 和 longitude 都变成了可选，当我们使用时需要解包。    

// print(encodeTurtleRockString)
{
  "coordinates" : {

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

&emsp;所以为了保证当服务器给我们返回 `{}` 或者 `null` 时程序都能正常解码，我们需要把 coordinates 属性，以及 Coordinates 结构的各个成员变量都定义为可选类型。

&emsp;所以到了这里我们可能会发现一个问题，就是我们不知道 Json 字符串中哪些字段会返回空值，我们又不能完全相信服务器返回给我们的字段必定是有值的，哪天疏忽了返回了空值可咋整，此时我们在定义类型时就会不得不把所有的成员变量都定为可选值了。

6. 遵循 Codable 协议的类型中定义了一个非可选的属性值，例如在上面的 struct Landmark 结构体中添加一个 var xxx: String 成员变量，然后在解码时 Json 字符串中又不包含此属性的话会打印如下错误信息（keyNotFound，No value associated with key CodingKeys(stringValue: \"xxx\", intValue: nil) (\"xxx\"). 没有找到 CodingKeys 中与 xxx 关联的值）：

```swift
keyNotFound(CodingKeys(stringValue: "xxx", intValue: nil), Swift.DecodingError.Context(codingPath: [], debugDescription: "No value associated with key CodingKeys(stringValue: \"xxx\", intValue: nil) (\"xxx\").", underlyingError: nil))
```

&emsp;此时我们需要把 xxx 定义为可选类型才能正常解码。（例如某天 Web 没有返回之前预定的必定返回的字段时，而此字段又指定的是非可选的话，那么 Codable 解码时会发生 crash，所以这里又增加了一条原因，此时我们在定义类型时就会不得不把所有的成员变量都定为可选值了。）

#### Codable 的进阶使用

&emsp;上面的嵌套、数组类型的成员变量、可选的成员变量、Json 字符串本身是模型数组、空对象、空值等等，这些情况中都是采用了 Codable 的默认实现，我们不需要添加什么自定义操作，Codable 自动帮我们完成了数据到模型的转换。那有哪些需要我们自定义的操作才能完成数据到模型的转换呢？下面一起来梳理一下。

&emsp;虽然 Codable 的默认实现足够应付大多数情形了，但是有时候我们还是存在一些自定义需求。为了处理这类自定义问题，我们就必须自己覆盖 Codable 的一些默认实现。

1. protocol Decoder 协议中 unkeyedContainer 的使用

```swift
/// A type that can decode values from a native format into in-memory representations.
public protocol Decoder {
    /// Returns the data stored in this decoder as represented in a container appropriate for holding values with no keys.
    ///
    /// - returns: An unkeyed container view into this decoder.
    /// - throws: `DecodingError.typeMismatch` if the encountered stored value is not an unkeyed container.
    func unkeyedContainer() throws -> UnkeyedDecodingContainer
}
```

&emsp;那么什么情况下我们会遇到，不带键的数据呢？没错，大概就是基本类型构成的数组，例如上面示例中的经纬度坐标，直接把经纬度坐标放在一个数组中时：

```swift
let turtleRockString = """
    {
        ...
        "coordinates": [-116.166868, 34.011286],
        ...
    }
"""
```

&emsp;那么此时我们可以把 struct Landmark 结构体的 coordinates 成员变量的类型由 struct Coordinates 类型修改为 `[Double]` 数组，没错，这样确实也能正常解码，但是如果我们就是想要使用 struct Coordinates 类型的 coordinates 呢，并且当数据返回的是经纬度的 Double 数组时，也能把经纬度正常解码到 struct Coordinates 结构体的 latitude 和 longitude 两个成员变量上，那么我们可以如下修改 struct Coordinates 结构体：

```swift
    struct Coordinates: Hashable, Codable {
        var latitude: Double
        var longitude: Double
        
        init(from decoder: Decoder) throws {
            var contaioner = try decoder.unkeyedContainer()
            
            latitude = try contaioner.decode(Double.self)
            longitude = try contaioner.decode(Double.self)
        }
    }
```

&emsp;打印编码解码结果，可看到 turtleRock 和 encodeTurtleRockString 都正常打印了，且 encodeTurtleRockString 编码的字符串中，coordinates 是根据 struct Coordinates 结构体来编码的，如果我们想 latitude 和 longitude 的值转回 Double 数组的话我们需要自己重写 struct Coordinates 结构体的 `func encode(to encoder: Encoder) throws` 函数。

```swift
// print(turtleRock)
Landmark(id: 1001, name: "Turtle Rock", park: "Joshua Tree National Park", state: "California", description: "Suscipit ...", imageName: "turtlerock", coordinates: Landmarks.Landmark.Coordinates(latitude: -116.166868, longitude: 34.011286))
// print(encodeTurtleRockString)
{
  "coordinates" : {
    "longitude" : 34.011285999999998,
    "latitude" : -116.16686799999999
  },
  "id" : 1001,
  "park" : "Joshua Tree National Park",
  "description" : "Suscipit ...",
  "imageName" : "turtlerock",
  "state" : "California",
  "name" : "Turtle Rock"
}
```

2. open class JSONDecoder 类的 `open var dateDecodingStrategy: JSONDecoder.DateDecodingStrategy` 属性的使用。（日期的转换策略）

&emsp;我们经常需要需要跟日期打交道，日期数据可能以不同形式展现下发，最常见的日期标准是 [ISO8601](https://zh.wikipedia.org/wiki/ISO_8601) 和 [RFC3339](https://tools.ietf.org/html/rfc3339)，举例来说：

```swift
1985-04-12T23:20:50.52Z          // 1

1996-12-19T16:39:57-08:00        // 2
1996-12-20T00:39:57Z             // 3

1990-12-31T23:59:60Z             // 4
1990-12-31T15:59:60-08:00        // 5
1937-01-01T12:00:27.87+00:20     // 6
```

&emsp;上面这些都是日期表示格式，但是只有第二个和第三个示例是 Swift 中 Codable 可以解码的，我们首先来看如何解码：

```swift
let turtleRockString = """
    {
        ...
        
        "updated":"2018-04-20T14:15:00-0700"
    }
"""

struct Landmark: Hashable, Codable, Identifiable {
    ...
    
    var updated: Date?
    
    ...
}

func decode<T: Decodable>(_ jsonString: String) -> T {
        ...
        
        let decoder = JSONDecoder()
        decoder.dateDecodingStrategy = .iso8601
        
        ...
}

// print(turtleRock) 打印中 updated 值如下：
updated: Optional(2018-04-20 21:15:00 +0000)
```

&emsp;JSONDecoder 提供了一个方便的机制可以解析日期类型，根据你的需求设置一下 dateDecodingStrategy 属性为 DateDecodingStrategy.iso8601 就可以解码符合标准（ISO8601 DateFormatter）的日期格式了。

&emsp;另一种常用的日期格式是时间戳（timestamp），时间戳是指格林威治时间 1970 年 01 月 01 日 00 时 00 分 00 秒起至现在的总秒数。

```swift
let turtleRockString = """
    {
        ...
        
        "updated":1540650536
    }
"""

struct Landmark: Hashable, Codable, Identifiable {
    ...
    
    var updated: Date?
    
    ...
}

func decode<T: Decodable>(_ jsonString: String) -> T {
        ...
        
        let decoder = JSONDecoder()
        decoder.dateDecodingStrategy = .secondsSince1970
        
        ...
}

// print(turtleRock) 打印中 updated 值如下：
 updated: Optional(2018-10-27 14:28:56 +0000)
```

&emsp;解码时间戳格式日期需要将 JSONDecoder 的 dateDecodingStrategy 设置为 DateDecodingStrategy.secondsSince1970（秒为单位）或 DateDecodingStrategy.millisecondsSince1970（毫秒为单位）。

&emsp;那么如果不是刚才提到的可以默认支持的解码格式怎么办？JSONDecoder 对象也提供了定制化方式：我们以前面提到的第一种格式为例，1985-04-12T23:20:50.52Z，通过扩展 DateFormatter 定义一个新的 iso8601Full，把这个作为参数传入 dateDecodingStrategy。

```swift
extension DateFormatter {
    static let iso8601Full: DateFormatter = {
        let formatter = DateFormatter()
        formatter.dateFormat = "yyyy-MM-dd'T'HH:mm:ss.SSSZZZZZ"
        formatter.calendar = Calendar(identifier: .iso8601)
        formatter.timeZone = TimeZone(secondsFromGMT: 0)
        formatter.locale = Locale(identifier: "en_US_POSIX")
        return formatter
    }()
}
```

```swift
...

let decoder = JSONDecoder()
decoder.dateDecodingStrategy = .formatted(DateFormatter.iso8601Full)

...
```

&emsp;decoder.dateDecodingStrategy = .formatted(DateFormatter.iso8601Full) 提供一个定制化的日期格式工具，我们可以根据需求定制日期的解码格式。[Swift之Codable实战技巧](https://zhuanlan.zhihu.com/p/50043306)

&emsp;dateDecodingStrategy 属性在 JSONDecoder 类中的相关信息如下：

```swift
open class JSONDecoder {

    /// The strategy to use for decoding `Date` values.
    public enum DateDecodingStrategy {

        /// Defer to `Date` for decoding. This is the default strategy.
        case deferredToDate

        /// Decode the `Date` as a UNIX timestamp from a JSON number.
        case secondsSince1970

        /// Decode the `Date` as UNIX millisecond timestamp from a JSON number.
        case millisecondsSince1970

        /// Decode the `Date` as an ISO-8601-formatted string (in RFC 3339 format).
        @available(macOS 10.12, iOS 10.0, watchOS 3.0, tvOS 10.0, *)
        case iso8601

        /// Decode the `Date` as a string parsed by the given formatter.
        case formatted(DateFormatter)

        /// Decode the `Date` as a custom value decoded by the given closure.
        case custom((_ decoder: Decoder) throws -> Date)
    }
    
    ...

    /// The strategy to use in decoding dates. Defaults to `.deferredToDate`.
    open var dateDecodingStrategy: JSONDecoder.DateDecodingStrategy
    
    ...
}    
```

3. open class JSONDecoder 类的 `open var keyDecodingStrategy: JSONDecoder.KeyDecodingStrategy` 属性的使用。（系统提供的变量名从蛇形命令法到小驼峰命名法的自动转换）

&emsp;Web 服务中使用 Json 时一般使用蛇形命名法（snake_case_keys），把名称转换为小写字符串，并用下划线（`_`）代替空格来连接这些字符，与此不同的是 Swift API 设计指南中预先把对类型的转换定义为 UpperCamelCase（大驼峰命名），其他所有东西都定义为 lowerCamelCase（小驼峰命名）。由于这种需求十分普遍，在 Swift 4.1 时 JSONDecoder 添加了 keyDecodingStrategy 属性，可以在不同的书写惯例之间方便地转换。如果有这样的键值 `image_Name`，就会转换成 `imageName`。[Swift之Codable实战技巧](https://zhuanlan.zhihu.com/p/50043306)

&emsp;如上实例代码中，构建 JSONDecoder 对象后直接指定其 keyDecodingStrategy 属性。（键解码策略）

```swift
let decoder = JSONDecoder()
decoder.keyDecodingStrategy = .convertFromSnakeCase
```

&emsp;keyDecodingStrategy 属性在 JSONDecoder 类中的相关信息如下： 

```swift
open class JSONDecoder {
    ...
    
    /// The strategy to use for automatically changing the value of keys before decoding.
    public enum KeyDecodingStrategy {

        /// Use the keys specified by each type. This is the default strategy.
        case useDefaultKeys

        /// Convert from "snake_case_keys" to "camelCaseKeys" before attempting to match a key with the one specified by each type.
        /// 
        /// The conversion to upper case uses `Locale.system`, also known as the ICU "root" locale. This means the result is consistent regardless of the current user's locale and language preferences.
        ///
        /// Converting from snake case to camel case:
        /// 1. Capitalizes the word starting after each `_`
        /// 2. Removes all `_`
        /// 3. Preserves starting and ending `_` (as these are often used to indicate private variables or other metadata).
        /// For example, `one_two_three` becomes `oneTwoThree`. `_one_two_three_` becomes `_oneTwoThree_`.
        ///
        /// - Note: Using a key decoding strategy has a nominal performance cost, as each string key has to be inspected for the `_` character.
        case convertFromSnakeCase

        /// Provide a custom conversion from the key in the encoded JSON to the keys specified by the decoded types.
        /// The full path to the current decoding position is provided for context (in case you need to locate this key within the payload). The returned key is used in place of the last component in the coding path before decoding.
        /// If the result of the conversion is a duplicate key, then only one value will be present in the container for the type to decode from.
        case custom((_ codingPath: [CodingKey]) -> CodingKey)
    }
    
    ...
    
    /// The strategy to use for decoding keys. Defaults to `.useDefaultKeys`.
    open var keyDecodingStrategy: JSONDecoder.KeyDecodingStrategy
    
    ...
}
```

&emsp;但是还可能有特殊情况，Web 服务的开发者可能某些时候大意了，也没有遵守蛇形命名法，而是很随意的处理了，那么如果我们想对键值进行校正，该如何处理？这就引出了下个点。

4. 当遵循 Codable 协议的类型属性（成员变量）名和 Json 字符串中的字段名不同时，如何进行自定义匹配映射。

&emsp;解决办法是：使用 CodingKeys 枚举指定一个明确的映射。

&emsp;Swift 会寻找符合 CodingKey 协议的名为 CodingKeys 的子类型（如下枚举类型）。这是一个标记为 private 的枚举类型，对于名称不匹配的键对应的枚举值指定一个明确的 String 类型的原始值，如下：

```swift
struct Landmark: Hashable, Codable, Identifiable {
    ...
    
    private enum CodingKeys: String, CodingKey {
        case id
        case name
        case park
        case state
        case description
        case imageName = "imageNameXXX"
        case coordinates
    }
}
```

&emsp;如上默认会把 Json 字符串中 imageNameXXX 字段的值指定给 struct Landmark 实例的 imageName 成员变量，其它成员变量值的话还使用 Json 字符串中一一对应的字段值。

&emsp;上面示例代码中，我们修改 turtleRockString 字符串中的 imageName 字段值为 imageNameXXX，然后进行解码编码：

```swift
let turtleRockString = """
    {
        ...
        
        "imageNameXXX": "turtlerock",
        
        ...
    }
"""
```

&emsp;打印编码解码结果，看到 "imageNameXXX": "turtlerock" 的值解码到 Landmark 结构体实例的 imageName 成员变量中，然后在 encodeTurtleRockString 字符串中，看到编码的默认实现亦是把 Landmark 结构体实例的 imageName 成员变量编码为："imageNameXXX" : "turtlerock"。

```swift
// print(turtleRock)
Landmark(id: 1001, name: "Turtle Rock", park: "Joshua Tree National Park", state: "California", description: "Suscipit ...", imageName: "turtlerock", coordinates: Landmarks.Landmark.Coordinates(latitude: 34.011286, longitude: -116.166868))

// print(encodeTurtleRockString)
{
  "name" : "Turtle Rock",
  "id" : 1001,
  "park" : "Joshua Tree National Park",
  "description" : "Suscipit ...",
  "coordinates" : {
    "longitude" : -116.16686799999999,
    "latitude" : 34.011285999999998
  },
  "state" : "California",
  "imageNameXXX" : "turtlerock"
}
```

5. 由 字符串/整型 转换为枚举类型。

&emsp;在 TableView 的列表中我们经常会遇到不同类型的 cell，例如：图片、视频、超链接等等类型，然后针对不同的类型，服务端一般会给我们返回一个类型的字符串，如：pic、video、link，甚至直接返回数字：1、2、3 这样，而在代码中使用时，我们一般更希望将这种类型字符串（整型数字）转换成枚举值，方便使用。下面举两个简单的例子来说明如何从字符串或者整型数据转换成枚举类型。

```swift
let turtleRockString = """
    {
        ...
        
        "template": "video", // 也可能是：pic、link
        
        ...
    }
"""
```

&emsp;template 代表当前 Json 字符串模型的类型，其值是一个字符串类型：

```swift
struct Landmark: Hashable, Codable, Identifiable {
    ...
    
    var template: Template
    
    ...
    
    enum Template: String, Codable {
        case VIDEO = "video"
        case PIC = "pic"
        case LINK = "link"
    }
}
```

&emsp;我们在 struct Landmark 结构体内部嵌套定义一个 enum Template 枚举，它的原始值是 String 类型，并且遵循 Codable 协议，列举出所有可能的类型和对应的字符串值，然后在 struct Landmark 结构体中定义 template 成员变量的类型为 Template 枚举，Codable 就可以自动完成从字符串到枚举类型的转换。

&emsp;打印编码解码结果，看到 template 的值是：Template.VIDEO，编码结果中 "template" : "video" 也正常编码。

```swift
// print(turtleRock)
Landmark(id: 1001, name: "Turtle Rock", park: "Joshua Tree National Park", state: "California", description: "Suscipit ...", imageName: "turtlerock", template: Landmarks.Landmark.Template.VIDEO, coordinates: Landmarks.Landmark.Coordinates(latitude: 34.011286, longitude: -116.166868))

// print(encodeTurtleRockString) 
{
  "template" : "video",
  "coordinates" : {
    "longitude" : -116.16686799999999,
    "latitude" : 34.011285999999998
  },
  "id" : 1001,
  "park" : "Joshua Tree National Park",
  "description" : "Suscipit ...",
  "imageName" : "turtlerock",
  "state" : "California",
  "name" : "Turtle Rock"
}
```

&emsp;同样，如果 template 的值是整型数字的话，我们只需要把 enum Template 枚举值指定为对应的原始值即可，如下修改：

```swift
struct Landmark: Hashable, Codable, Identifiable {
    ...
    
    var template: Template
    
    ...
    
    enum Template: Int, Codable {
        case VIDEO = 1
        case PIC = 2
        case LINK = 3
    }
}
```

6. 










&emsp;如果给一个遵循 Codable 协议的类型定义 CodingKeys 枚举的话，至少需要给其中一个枚举值指定一个 rawValue，否则我们需要重写该类型的 init(from decoder: Decoder) throws 函数，为类型的每个属性（成员变量）调用 container.decode 指定该属性（成员变量）所属的类型和 key 值，如下：

```swift
    init(from decoder: Decoder) throws {
        let container = try decoder.container(keyedBy: CodingKeys.self)
        id = try container.decode(Int.self, forKey: .id)
        name = try container.decode(String.self, forKey: .name)
        ...
    }
```

&emsp;





+ 还有一种情况，当服务器返回的字段类型和我们预定义的模型的类型不匹配时，也会解码失败！需要处理

+ 字段匹配
+ 处理键名和属性名不匹配
+ 两端键值不匹配
+ 定制日期格式处理
+ 枚举值 从字符串解析枚举类型 从整型解析枚举类型
+ 动态键值结构
+ 特殊类型







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

## 看着看着发现 LG 都开始卷 Swift 源码了...（必学）
+ [Swift底层进阶--015：Codable源码解析](https://www.jianshu.com/p/9302f7bac319)
+ [Swift底层探索:Codable](https://www.jianshu.com/p/d591bd7f53ac)

## 针对当返回的 Json 字符串中字段的类型 和 模型定义中属性（成员变量）类型不匹配时的解析：只要有一个字段类型不匹配，整个 json 的转 model 都会失败，这是不友好的。
+ [针对 swift4 的JSONDecoder的特殊情况处理](https://www.jianshu.com/p/51c219092290)

## 学习一些 Codable 的嵌套用法、学习 Codable 中的三种容器类型（必学），还有 CodingKey（必学）。

+ [Swift5 Codable源码剖析](https://www.jianshu.com/nb/3595319)
