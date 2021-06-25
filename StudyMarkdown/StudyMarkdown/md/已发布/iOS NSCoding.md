# iOS NSCoding

## NSCoding

> &emsp;Adopted By: 各种 OpenStep 类
> 
> &emsp;Declared In: Foundation/NSObject.h

### Protocol Description（协议说明）
&emsp;NSCoding 协议声明了一个类必须实现的两个方法，以便可以对该类的实例进行编码和解码。此功能为归档（将对象和其他结构存储在磁盘上）和分发（将对象复制到不同的地址空间）提供了基础。有关编码的介绍，请参见 NSCoder 和 NSArchiver 类规范。

&emsp;与面向对象的设计原理一致，要编码或解码的对象负责对其实例变量进行编码和解码。编码人员通过调用 `encodeWithCoder:` 或 `initWithCoder:` 来指示对象这样做。 `encodeWithCoder:` 指示对象将其实例变量编码到提供的编码器中；一个对象可以多次接收此方法。 `initWithCoder:` 指示对象从提供的编码器中的数据初始化自身；这样，它将替换任何其他初始化方法，并且每个对象仅发送一次。任何应编码的对象类都必须采用 NSCoding 协议并实现其方法。

&emsp;当一个对象收到一个 `encodeWithCoder:` 消息时，如果它的超类也符合 NSCoding 协议，则在将消息发送给 super 之后，它应该对所有重要实例变量进行编码。对象不必对所有实例变量进行编码。一些值对于重新建立可能并不重要，而其他值则可以在解码时从相关状态派生。其他实例变量应仅在特定条件下编码（例如，使用 NSArchiver 类规范中所述的 `encodeConditionalObject:`）。

&emsp;例如，假设你要创建一个虚构的 MapView 类，该类以各种放大率显示图例和地图。 MapView 类定义了几个实例变量，包括地图名称和当前放大倍数。 MapView 类还包含几个相关视图的实例变量。 MapView 的 `encodeWithCoder:` 方法可能类似于以下内容：
```c++
- (void)encodeWithCoder:(NSCoder *)coder {
    [super encodeWithCoder:coder];
    
    [coder encodeValueOfObjCType:@encode(char *) at:mapName];
    [coder encodeValueOfObjCType:@encode(unsigned int) at:&magnification];
    [coder encodeObject:legendView];
    [coder encodeConditionalObject:auxiliaryView];
    
    return;
}
```
&emsp;本示例假定 MapView 的超类也支持 NSCoding 协议。如果你的类的超类不支持 NSCoding，则应省略调用超类的 `encodeWithCoder:` 方法的行。

&emsp;`encodeValueOfObjCType:at:` 和 `encodeObject:` 是可用于对类的实例变量进行编码的编码器方法。你可以使用编码器的这些方法和其他方法来编码 id，标量，C 数组，结构，字符串和指向任何这些类型的指针。编码器还定义了用于解码值的相应方法。有关方法列表，请参见 NSCoder、NSArchiver 和 NSUnarchiver 类规范。

&emsp;`@encode()` 编译器指令从类型表达式生成 Objective-C 类型代码。有关更多信息，请参见 Objective-C Language for more information 中的 Object-Oriented Programming。

&emsp;同样，在 `initWithCoder:` 中对象应该首先向 super 发送消息（如果适用）以初始化继承的实例变量，然后它应该解码并初始化自己的实例变量。 MapView 的 `initWithCoder:` 的实现可能如下所示：
```c++
- (id)initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
    
    [coder decodeValueOfObjCType:@encode(char *) at:mapName];
    [coder decodeValueOfObjCType:@encode(unsigned int) at:&magnification];
    
    legendView = [[coder decodeObject] retain];
    auxiliaryView = [[coder decodeObject] retain];
    
    return self;
}
```
&emsp;请注意，在上面的示例中，将 `initWithCoder:` 的返回值分配给 self。这是在子类中完成的，因为超类在其 `initWithCoder:` 的实现中可能决定返回除自身之外的对象。如果你的类的超类不支持 NSCoding，则应调用 super 的指定初始化器，而不是 `initWithCoder:`。

### Making Substitutions During Coding（在编码过程中进行替换）
&emsp;在编码或解码期间，编码器对象调用一些方法，这些方法允许被编码的对象为其自身替换替代类或实例。这允许归档在具有不同类层次结构或类的不同版本的实现之间共享（例如，类集群利用此功能）。它还允许应该维护唯一实例的类在解码时强制实施此策略（例如，对于给定的字体和大小，只需要一个 NSFont 实例）。

&emsp;替换方法由 NSObject 声明，有两种形式：通用和专用。通用方法是：

| Method | Typical Use |
| --- | --- |
| classForCoder | 允许对象在编码之前替换其自身以外的类。例如，在归档时，类集群的私有子类将替换其公共超类的名称。 |
| replacementObjectForCoder: | 允许对象在编码之前在其位置替换另一个实例。 |
| awakeAfterUsingCoder: | 允许一个对象在被解码后，用另一个对象代替自己。例如，表示字体的对象在解码后可能会释放自身，并返回具有与自身相同的字体描述的现有对象。这样，可以消除冗余对象。 |

&emsp;专用的替换方法类似于 `classForCoder` 和 `replaceObjectForCoder:`，但它们是为特定的具体编码器子类设计的（并由其调用）。 NSArchiver 调用 `classForArchiver:` 和 `replaceObjectForArchiver:`，而 NSPortCoder 调用 `classForPortCoder` 和 `replaceObjectForPortCoder:`。 （没有 `awakeAfterUsingCoder:` 的专门版本）通过实现这些专门的方法，你的类可以将其编码行为基于所使用的特定编码器类。有关这些方法的更多信息，请参见 NSObject 类规范中的方法描述，以及 NSPortCoder 类规范中的类描述。

### Method Types
&emsp;编码和解码对象。
```c++
- encodeWithCoder:
- initWithCoder:
```
### Instance Methods
#### encodeWithCoder:
```c++
- (void)encodeWithCoder:(NSCoder *)encoder
```
&emsp;使用编码器对 receiver 进行编码。
#### initWithCoder:
```c++
- (id)initWithCoder:(NSCoder *)decoder
```
&emsp;根据解码器中的数据初始化新分配的实例。返回 self。

## 参考链接
**参考链接:🔗**
+ [NSCoding](https://developer.apple.com/library/archive/documentation/LegacyTechnologies/WebObjects/WebObjects_3.5/Reference/Frameworks/ObjC/Foundation/Protocols/NSCoding/Description.html#//apple_ref/occ/intf/NSCoding)
