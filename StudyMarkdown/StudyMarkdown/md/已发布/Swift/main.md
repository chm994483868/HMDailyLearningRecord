#  待学清单

+ [Swift-文章汇总](https://www.jianshu.com/p/fadbefa4acad)

&emsp;觉得直接开 Swift 源码有点早，毕竟 Swift 的灵活使用还达不到，先学习梳理 kingfish 的源码，然后再学习 Swift 源码。直接开 Swift 源码好了，再慢点就晚了。


1. 判断一个对象是否遵循某个协议，也可以使用 `as`。
```c++
...
if let r = self as? MainDataViewReloadable {
    alert.addAction(reloadAction(r))
}
...
```

2. `class func setAnimationsEnabled(_ enabled: Bool)`
  &emsp;默认情况下启用动画。如果禁用动画，后续动画块中的代码仍会执行，但实际上不会发生动画。因此，您在动画块内所做的任何更改都会立即反映出来，而不是进行动画处理。无论您使用基于块的动画方法还是开始/提交动画方法，都是如此。此方法仅影响调用后提交的那些动画。如果您在现有动画运行时调用此方法，这些动画将继续运行，直到它们到达它们的自然终点。
  
