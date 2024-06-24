# PageStorageKey

&emsp;PageStorageKey 是一个可以用来在销毁后将 widget 状态持久化存储，并在重新创建时恢复的 key。

&emsp;在 widget 最近的祖先 PageStorage 中，每个 key 与其 value 以及其他 PageStorageKey 的祖先链都需要是唯一的。为了使得当 widget 重新创建时可以找到已保存的值，key 的 value 不能是每次 wiget 创建时都会发生变化的对象。

&emsp;



## 参考链接
**参考链接:🔗**
+ [`PageStorageKey<T> class`](https://api.flutter.dev/flutter/widgets/PageStorageKey-class.html)
+ [When to Use Keys - Flutter Widgets 101 Ep. 4](https://www.youtube.com/watch?v=kn0EOS-ZiIc)
