#  C++ 强制类型转换符 reinterpret_cast
> 主要作用把一个指针转换成一个整数等。

## 简介
```c++
reinterpret_cast<type-id> (expression)
```
`type-id`必须是一个指针、引用、算术类型、函数指针或者成员指针。它可以把一个指针转换成一个整数，也可以把一个整数转换成一个指针（先把一个指针转换成一个整数，再把该整数转换成原类型的指针，还可以得到原先的指针值）。

## 用法
该运算符的用法比较多。操作符修改了操作数类型，但仅仅是重新解释了给出的对象的**比特模型**而**没有进行二进制转换**。

例如：
```c++
int *n = new int;
double *d = reinterpret_cast<double*>(n);
```
在进行计算以后, `d` 包含无用值. 这是因为 `reinterpret_cast` 仅仅是复制 `n` 的比特位到 `d`, 没有进行必要的分析。因此, 需要谨慎使用 `reinterpret_cast`.

### `static_cast` 与 `reinterpret_cast`:
`reinterpret_cast`是为了映射到一个完全不同类型的意思，这个关键词在我们需要把类型映射回原有类型时用到它。我们映射到的类型仅仅是为了故弄玄虚和其他目的，这是所有映射中最危险的。(这句话是`C++`编程思想中的原话)
`static_cast`和`reinterpret_cast`的区别主要在于**多重继承**，比如:

```c++
class A {
    public:
    int m_a;
};
 
class B {
    public:
    int m_b;
};
 
class C : public A, public B {};
```
那么对于以下代码：
```c++
C c;
printf("%p, %p, %p", &c, reinterpret_cast<B*>(&c), static_cast <B*>(&c));
```
前两个的输出值是相同的，最后一个则会在原基础上偏移 4 个字节，这是因为`static_cast`计算了父子类指针转换的偏移量，并将之转换到正确的地址（`c`里面有`m_a`,`m_b`，转换为`B*`指针后指到`m_b`处），而`reinterpret_cast`却不会做这一层转换。
因此, 你需要谨慎使用 `reinterpret_cast`.

**参考链接:🔗**
[reinterpret_cast](https://baike.baidu.com/item/reinterpret_cast/9303204?fr=aladdin)
