# constexpr关键字使用

> **`constexpr` 可以用来修饰变量、函数、构造函数。一旦以上任何元素被 `constexpr` 修饰，那么等于说是告诉编译器 “请大胆地将我看成编译时就能得出常量值的表达式去优化我”**。

`constexpr` 是 `C++ 11` 开始提出的关键字，其意义与 `C++ 14` 版本有一些区别。
它的作用如同它的名字 `const expression`。`constexp`的作用其实也是告诉编译器自己定义这个变量的意图（intent），从而让编译器进行额外检查，而这种额外的检查又保证了这个修饰符本身的语义，所以它更多的是为了提高代码的可读性和可维护性（不要假设代码只是给写作者一个人维护的）。`constexpr`就要求编译器在编译时检测这个表达式本身是不是编译时可以计算出常量值的。

> 1. `C++ 11` 中的 `constexpr` 指定的函数返回值和参数必须是字面值，而且必须 **有且只有一行 `return`代码**。这给函数的实现带来了更多的限制，比如通常只能通过三目运算符 + 递归来计算返回的字面值。
  2. `C++ 14` 中修改了规则，只要保证返回值和参数是字面值就行了，函数体中可以加入更多的语句，方便了灵活的计算。

> 很多人把 `constexpr` 和 `const` 相比较。

和 `const`关键字不同，`constexpr` 表示的是这个修饰的变量在输入（如果有输入的话）是常量的时候能够在编译时获得一个输出常量，而 `const` 更多的意思是这个变量本身在运行时不能被修改。
其实，`const` 并不能代表 “常量”，它**仅仅是对变量的一个修饰**，告诉编译器这个变量只能被初始化，且不能被直接修改（实际上可以通过**堆栈溢出**等方式修改）。而这个变量的值，可以在运行时也可以在编译时指定。

`constexpr` 可以用来修饰变量、函数、构造函数。一旦以上任何元素被 `constexpr` 修饰，那么等于说是告诉编译器 “请大胆地将我看成编译时就能得出常量值的表达式去优化我”。如：

> 示例代码编写环境: Xcode 11.3.1，Build Settrings 中，C Language Dialect: gnu 11，C++ Language Dialect: GNU++14[-std=gnu++14]

```c++
// 这是博主的原始代码，可能是 Xcode 已经进行了优化，并不会编译错误 🙆‍♂️
const int func() { // 这里会给一个⚠️：'const' type qualifier on return type has no effect
    return 10;
}
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        // insert code here...
        int arr[func()];
        arr[11] = 13; // 且这里索引大于10，依然能正常运行
        printf("%d\t%d\n", arr[0], arr[11]);
    }
    return 0;
}
// 打印: 12    13
```
对于 `func()`，胆小的编译器并没有足够的胆量去做编译优化，哪怕函数体就一句 return 字面值。而：
```c++
constexpr int func() {
    return 10;
}
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        // insert code here...
        int arr[func()];
        arr[0] = 12;
        // 这里会给两条警告
        arr[11] = 13; // ⚠️：Array index 11 is past the end of the array (which contains 10 elements)
        printf("%d\t%d\n", arr[0], arr[11]); // ⚠️：Array index 11 is past the end of the array (which contains 10 elements)
    }
    return 0;
}
// 打印: 12    13
```
则编译通过，编译期大胆的将 func() 做了优化，在编译期就确定了 func 计算出的值 10，而无需等到运行时再去计算。

这就是 `constexpr` 的第一个作用：给编译器足够的信心在编译期去做被 `constexpr` 修饰的表达式的优化。

`constexpr` 还有另外一个特性，虽然它本身的作用之一就是希望程序员能给编译器做优化的信心，但它却猜到了自己可能会被程序员欺骗，而编译器并不会对此 "恼羞成怒"中止编译，如：
```
int func(const int n) {
    return 10 + n;
}
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        // insert code here...
        int arr[func(1)];
        arr[0] = 12;
        arr[13] = 13; // 且这里索引为 11 依然能正常运行
        printf("%d\t%d\n", arr[0], arr[13]);
    }
    return 0;
}
// 打印:
12    13 
// 如果在 func 前面加 constexpr 会 crash，但是对 arr[x] 赋值时，x 不越界的话能正常运行。
```
程序员告诉编译器尽管信心十足地把 `func` 当做是编译期就能计算出值的程序，但却欺骗了它，程序员最终并没有传递一个常量字面量值到该函数。没有被编译器中止编译并报错的原因在于编译器并没有 100% 相信程序员，当其检测到 func 的参数是一个常量字面值的时候，编译器才会去对其优化，否则，依然会将计算任务留给运行时。
基于这个特性，constexpr 还可以被用来实现编译期的 `type traits`，比如 `STL` 中的 `is_const` 的完整实现:
```c++
template <class _Ty, _Ty _Val>
struct integral_constant {
    // convenient template for integral constant types
    static constexpr _Ty value = _Val;
    
    typedef _Ty value_type;
    typedef integral_constant<_Ty, _Val> type;
    
    constexpr operator value_type() const noexcept {
        // return stored value
        return (value);
    }
    
    constexpr value_type operator()() const noexcept {
        // return stored value
        return (value);
    }
};

typedef integral_constant<bool, true> true_type;
typedef integral_constant<bool, false> false_type;

template<class _Ty>
struct is_const: false_type {
    // determine whether _Ty is const qualified
};

template<class _Ty>
struct is_const<const _Ty>: true_type {
    // determine whether _Ty is const qualified
};

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        // insert code here...
        static_assert(is_const<int>::value, "error"); // error ❗️❗️❗️ Static_assert failed due to requirement 'is_const<int>::value' "error"
        static_assert(is_const<const int>::value, "error"); // ok
    }
    return 0;
}
```
**参考链接:🔗**
[C++11/14 constexpr 用法](https://www.jianshu.com/p/34a2a79ea947)
