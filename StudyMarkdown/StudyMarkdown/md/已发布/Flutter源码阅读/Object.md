# Object

&emsp;Object class 除了 null 之外，所有 Dart 对象的基类。

&emsp;由于 Object 是 non-nullable Dart 类层次结构的根，因此每个 non-Null Dart 类都是 Object 的子类。

&emsp;当你定义一个类时，你应该考虑重写 toString 方法，以返回描述该类实例的字符串。你可能还需要定义 hashCode 和 operator ==，就像 [Dart's core libraries](https://dart.dev/libraries) 中的 [Implementing map keys](https://dart.dev/libraries/dart-core#implementing-map-keys) 部分所描述的那样(围绕 hashCode 和 operator == 展开的内容)。



## 参考链接
**参考链接:🔗**
+ [Object class](https://api.flutter.dev/flutter/dart-core/Object-class.html)
