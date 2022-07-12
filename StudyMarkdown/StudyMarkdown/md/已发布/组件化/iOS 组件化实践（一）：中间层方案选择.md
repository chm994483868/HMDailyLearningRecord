# iOS 组件化实践（一）：中间层方案选择

&emsp;CTMediator 以 Target-Action 的方式完成函数执行，借助 NSInvocation/NSMethodSignature/SEL 方式完成函数调用，可以通过中间层的调用，抹掉文件顶部的 import ，解除组件间的引用依赖。

&emsp;CTMediator 项目中有三个标准文件夹：

+ Categories（实际应用中，这是一个单独的 repo，所用需要调度其他模块的人，只需要依赖这个 repo。这个 repo 由 target-action 维护者维护）
+ CTMediator（这也是单独的 repo，完整的中间件就这 100 行代码）
+ DemoModule（target-action 所在的模块，也就是提供服务的模块，这也是单独的 repo，但无需被其他人依赖，其他人通过 category 调用到这里的功能）

&emsp;




























## 参考链接
**参考链接:🔗**
+ [casatwy/CTMediator](https://github.com/casatwy/CTMediator)
