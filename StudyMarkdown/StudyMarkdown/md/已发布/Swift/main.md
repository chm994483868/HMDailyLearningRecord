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
  
+ [Swift笔记1：源码编译(5.3.1)](https://juejin.cn/post/6914265565540384782)
+ [Swift源码编译-让底层更清晰](https://www.jianshu.com/p/5cd66d7d2daf)
+ [Mac安装CMake](https://www.jianshu.com/p/7fff1f77dd9d)
+ [MacOS 下安装 autoconf 、 automake](https://www.jianshu.com/p/cbd651911434)


[Swift CI 页面](https://ci.swift.org)
macOS 11.4(20F71) [下载介绍页面](https://mrmacintosh.com/macos-big-sur-full-installer-database-download-directly-from-apple/) 下载地址：[macOS 11.4(20F71)](http://swcdn.apple.com/content/downloads/55/59/071-00696-A_4T69TQR1VO/9psvjmwyjlucyg708cqjeaiylrvb0xph94/InstallAssistant.pkg)

[Xcode_13_beta_4](https://download.developer.apple.com/Developer_Tools/Xcode_13_beta_4/Xcode_13_beta_4.xip)

https://cloud.tencent.com/developer/article/1744552
https://xcodereleases.com
https://mrmacintosh.com/macos-big-sur-full-installer-database-download-directly-from-apple/

+ [Swift笔记1：源码编译(5.3.1)](https://juejin.cn/post/6914265565540384782)
+ [Swift(5.3.2) 源码编译](https://github.com/ShenYj/ShenYj.github.io/wiki/Swift-源码编译)
https://forums.swift.org/t/failure-when-building-swift-project/46964
