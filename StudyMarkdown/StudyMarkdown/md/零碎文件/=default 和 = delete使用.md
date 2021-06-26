# = default = delete

> = delete 表示删除默认构造函数。
  = default 表示（显式标识）默认存在构造函数。
  
## default 关键字
> `default` 用于显示要求编译器 **提供合成版本** (这个提供合成版本理解为“要求编译器自动生成”)的四大函数，即用于显示要求编译器自动生成四大函数(默认情况下就会自动生成，这个自动生成可以被阻止，正对应了下面的 `delete`)：
  1. 构造函数
  2. 拷贝函数(复制函数)
  3. 析构函数
  4. 赋值函数(内部会自动调用拷贝函数)
  
示例：
```c++
class A {
public:
    A() = default;
    A(const A& a) = default;
    A& operator=(A&) = default;
    ~A() = default;
    
    // 验证我们的猜想，把我们自己的函数标记为 default 
    void func() = default; // 编译器直接报错：Only special member functions may be defaulted
};
```
## delete 关键字
> 定义删除函数，在旧标准下，如果我们希望阻止拷贝可以通过显式声明拷贝构造函数和拷贝赋值函数为 `private`，即使使用私有，依然可以通过友元函数调用到，新标准下允许我们定义删除函数。
示例：
```c++
class A {
private:
    A(const A& a) = delete; // 表示删除默认复制构造函数，即不能进行默认拷贝
    const A& operator=(A&) = delete;
public:
    A() = default;
    ~A() = default;
};
```
`= delete` 可以告知编译器，不生成默认的这些成员函数。

有两点需要注意:
1. `= delete` 可以使用在任意的函数上（可以引导函数匹配过程），除了析构函数（析构函数为删除函数将导致该类型无法销毁）。
2. `= default` 只能用在编译器会自动默认生成的函数上。
3. `= delete` 必须出现在函数第一次声明的时候。

**参考链接:🔗**
[C++中 =default,=delete用法](https://www.cnblogs.com/AndrewYin/p/9282406.html)
