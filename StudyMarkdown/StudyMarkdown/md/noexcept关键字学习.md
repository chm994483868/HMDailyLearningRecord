#  noexcept关键字学习

> `noexcept` 关键字是 C++ 11 带来的新特性。(no exception)

## 关键字 noexcept
通过它的名字我们似乎能看出一些端倪。该关键字告诉编译器，函数中不会发生异常（有点类似于 `Swift` 中的 `try!`），这有利于编译器对程序做更多的优化。
如果在运行时，`noexecpt` 函数向外抛出了异常（如果函数内部捕捉了异常并完成处理，这种情况不算抛出异常），程序会直接终止，调用`std::terminate()`函数，该函数内部会调用 `std::abort()` 终止程序。从 `C++ 11` 开始，我们能看到很多代码当中都有关键字 `noexcept`。

## C++ 异常处理
`C++`中的异常处理是 **在运行时而不是编译时检测的**。为了实现运行时检测，**编译器创建额外的代码**，然而这会**妨碍程序优化**。
在实践中，一般两种异常抛出方式是常用的：

1. 一个操作或者函数可能会抛出一个异常。
2. 一个操作或者函数 **不可能抛出任何异常**。

后面这一种方式中在以往的`C++`版本中常用`throw()`表示，在`C++ 11`中已经被 `noexcept` 代替。
```c++
void swap(Type& x, Type& y) throw () { // C++ 11 之前
    x.swap(y);
}

void swap2(Type& x, Type& y) noexcept { // C++ 11
    x.swap(y);
}
```

## 有条件的 noexcept
上面单独使用 `noexcept`，表示其所限定的 `swap` 函数绝对不发生异常。然而，使用方式可以更加灵活，表明在一定条件下不发生异常。
```c++
void swap(Type& x, Type& y) noexcept(noexcept(x.swap(y))) { // C++11
    x.swap(y);
}
```
它表示，如果操作 `x.swap(y)` 不发生异常，那么函数 `swap(Type& x, Type& y)` 一定不发生异常。
一个更好的示例是 `std::pair` 中的移动分配函数（`move assignment`），它表明，如果类型 `T1` 和 `T2` 的移动分配（`move assign`）过程中不发生异常，那么该移动构造函数就不会发生异常。
```c++
pair& operator=(pair&& __p)
noexcept(__and_<is_nothrow_move_assignable<_T1>,
                is_nothrow_move_assignable<_T2>>::value) {
    first = std::forward<first_type>(__p.first);
    second = std::forward<second_type>(__p.second);
    return *this;
}
```

## 什么时候该使用 noexcept ?
使用noexcept表明函数或操作不会发生异常，会给编译器更大的优化空间。然而，并不是加上 `noexcept` 就能提高效率。
以下情形鼓励使用 `noexcept`:

1. 移动构造函数（`move constructor`）
2. 移动分配函数（`move assignment`）
3. 析构函数（`destructor`）
4. 叶子函数（`Leaf Function`）

> 叶子函数是指在函数内部**不分配栈空间**，也不调用其它函数，也不存储非易失性寄存器，也不处理异常。
> 在新版本的编译器中，析构函数是默认加上关键字 `noexcept`的。
  下面代码可以检测编译器是否给析构函数加上关键字`noexcept`。
  ```c++
  // xcode 下能正常运行
  // 即使把 ~X() {}; 注释掉，依然能正常运行
  struct X {
      ~X() {};
  //    ~X() = delete;
  };
  
  void func() { // 没有 noexcept
  }
  int main(int argc, const char * argv[]) {
      @autoreleasepool {
          // insert code here...
          X x;
          static_assert(noexcept(x.~X()), "Ouch!");
          static_assert(noexcept(func()), "Ouch!"); // ❗️编译器 error: Static_assert failed due to requirement 'noexcept(funcTest())' "Ouch!" 
      }
      return 0;
  }
  ```

最后强调一句，在不是以上情况或者没把握的情况下，不要轻易使用 `noexcept`。

**参考链接:🔗**
[C++11 带来的新特性（3）—关键字noexcept](https://www.cnblogs.com/sword03/p/10020344.html)
