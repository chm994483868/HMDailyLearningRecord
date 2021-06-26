# iOS 渲染过程全解析

> &emsp;

&emsp;为了把显示器的 **显示过程** 和 **系统的视频控制器** 进行同步，显示器（或者其他硬件）会用 **硬件时钟** 产生一系列的 **定时信号**。

&emsp;离屏渲染：屏幕之外的渲染流程（GPU 之外的渲染，需要另外给它准备一个离屏渲染缓冲区，等待合适的时机会把离屏渲染缓冲区的内容合并到帧缓冲区去，等待着显示到屏幕上）

&emsp;需要进行二次渲染，帧缓冲区在一帧之内无法渲染出想要的结果显示到屏幕上去。


&emsp;离屏渲染缓冲区需要存储空间保存数据、把离屏渲染缓冲区的数据转移到帧缓冲区去时需要时间、离屏渲染缓冲区的空间是有限的为屏幕的 2.5 倍。
大量的离屏渲染会对内存造成压力。

&emsp;离屏渲染的意义，并不是说所有的离屏渲染都是坏事，它是一种有意义的系统机制，我们能做的应是避免自己的疏忽所造成的离屏渲染。为了达到某种特殊效果，需要使用额外的缓冲区来保存中间状态，不得不使用离屏渲染，系统自动触发。

&emsp;提前把一部分渲染保存在离屏渲染缓冲区，用于复用。app 渲染的时候需要对其进行额外的渲染和合并，才会开启离屏渲染利用 **离屏渲染缓冲区** 来存放合并多个需要渲染的数据，再放进 **帧缓冲区** 里，进行显示。

&emsp;离屏渲染有时间和空间的限制，时间是 100 ms 内没有被使用，则会丢弃，空间的话是超过屏幕的 2.5 倍像素大小也会被丢弃。






## 参考链接
**参考链接:🔗**
+ [iOS Rendering 渲染全解析](https://github.com/RickeyBoy/Rickey-iOS-Notes/blob/master/笔记/iOS%20Rendering.md)
+ [iOS视觉(三) -- 离屏渲染详解](https://juejin.cn/post/6850418109845766151)
+ [iOS图像渲染及卡顿问题优化](https://juejin.cn/post/6874046143160909838)
+ [Drawing and Printing Guide for iOS](https://developer.apple.com/library/archive/documentation/2DDrawing/Conceptual/DrawingPrintingiOS/Introduction/Introduction.html#//apple_ref/doc/uid/TP40010156-CH1-SW1)

17310047859
