# Associatedtype

&emsp;在协议中除了定义属性和方法外，我们还能 **定义类型的占位符，让实现协议的类型来指定具体的类型**。

&emsp;`associatedtype` 定义关联类型，相当于类型的占位符，让实现协议的类型来指定具体的类型。

```c++
protocol Food { }

struct Meat: Food { }
struct Grass: Food { }

protocol Animal {
//    associatedtype F
    // 这里给 F 赋值为 Food，在下面 Tiger 中可以直接使用 F，但是这样定死 F 为 Food，也失去了 associatedtype 的原有价值
    associatedtype F = Food
    
    func eat(_ food: F)
}

struct Tiger: Animal {
//    typealias F = Meat
    
    func eat(_ food: F) {
        print("eat \(food)")
    }
}

struct Sheep: Animal {
//    typealias F = Grass
    
    func eat(_ food: Grass) {
        print("eat \(food)")
    }
}
```

&emsp;添加 `associatedtype` 后，`Animal` 协议就不能被当作独立的类型使用了，如下的 `isSheep` 函数，给了我们清晰的错误提示，因为 `Animal` 包含了一个不确定的类型，所以随着 Animal 本身类型的变化，其中的 F 将无法确定。在一个协议加入了像是 `associatedtype` 或者 `Self` 的约束后，它将只能被用为泛型约束，而不能作为独立类型的占位使用。我们需要将函数改写为泛型：

```c++
func isTiger<T: Animal>(animal: T) -> Bool {
    if animal is Tiger {
        return true
    } else {
        return false
    }
}

// 报错：Protocol 'Animal' can only be used as a generic constraint because it has Self or associated type requirements
func isSheep(animal: Animal) -> Bool {
    if animal is Sheep {
        return true
    } else {
        return false
    }
}
```


## 参考链接
**参考链接:🔗**
+ [Swift associatedtype](https://www.jianshu.com/p/6bfaa5a80dcf)
