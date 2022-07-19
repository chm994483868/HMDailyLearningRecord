# iOS 组件化实践（一）：中间层方案选择

## CTMediator

&emsp;首先我们只看 CTMediator 单例类的内容，可以看到 CTMediator 以 Target-Action 的方式借助字符串和 runtime 使用 NSInvocation（NSMethodSignature、SEL）或者 NSObject 协议的 `- performSelector:withObject:` 来完成函数的调用执行，这样使用 CTMediator 这个中间件我们就可以抹掉我们日常在文件顶部使用 #import 引入的依赖（类）。借助字符串则是指在 CTMediator 类的核心函数：`- performTarget:action:params:shouldCacheTarget:` 中：
 
```c++
- (id _Nullable )performTarget:(NSString * _Nullable)targetName
                        action:(NSString * _Nullable)actionName
                        params:(NSDictionary * _Nullable)params
             shouldCacheTarget:(BOOL)shouldCacheTarget;
```

&emsp;可看到 targetName/actionName/params 三者基本以字符串传入，然后使用 runtime 创建 targetName 对象、创建 SEL，然后调用我们熟悉的 NSObject 协议的 `- (BOOL)respondsToSelector:(SEL)aSelector;` 函数判断对应的 SEL 是否被 targetName 对象所实现，然后当对应的 SEL 返回基本类型时使用 NSInvocation 的方式进行函数调用，或者使用我们熟悉的 NSObject 协议的 `- (id)performSelector:(SEL)aSelector withObject:(id)object;` 函数进行函数调用，当 targetName 对象对象创建失败或者对应的 SEL 函数不存在时都会进行安全的兜底操作。这样我们就可以借助 CTMediator 单例类不进行任何注册操作，在当前上下文环境中直接使用目标对象的类名字符串和目标函数的字符串名字完成函数调用了，把当前上下文环境与目标对象完全解耦。

&emsp;CTMediator 单例类的内容看完了，下面我们看一下它是如何在组件化方案中发挥作用的。

&emsp;首先我们创建模块（组件）时都需要依赖 CTMediator 这个单例类，然后把模块（组件）的公开 API 统一放在 CTMediator 类的一个分类中（Swift 中使用的是 CTMediator 类的 extension）。然后当模块（组件）之间需要通信时，直接通过模块对应的 CTMediator 分类中定义的公开 API 完成通信，完全不需要模块中的原始文件引用依赖，这样通过 CTMediator 单例类及其分类就解除了需要通信的各个模块之间的强依赖关系，同时 CTMediator 分类中定义好的公开 API 也对函数的参数进行了一定的校验。

&emsp;CTMediator 项目中有三个标准文件夹：

+ Categories（它里面是每个模块的公开 API 对应的 CTMediator 的一个分类，实际应用中，这是一个单独的 repo，所用需要调度其他模块的人，只需要依赖这个 repo。这个 repo 由 target-action 维护者维护）
+ CTMediator（这也是单独的 repo，完整的中间件就这 100 行代码）
+ DemoModule（target-action 所在的模块，也就是提供服务的模块，这也是单独的 repo，但无需被其他人依赖，其他人通过 CTMediator category 调用到这里的功能）

&emsp;CTMediator 文件夹中最核心的是 CTMediator 单例类的实现，它提供了两种方式的 Target-Action 调用，一种是我们直接传入 targetName、actionName、params 进行调用，一种是通过类似 `scheme://[target]/[action]?[params]`（`url sample: aaa://targetA/actionB?id=1234`）URL 的形式，内部则是对这个 URL 进行处理，首先提取出其中的 Target/Action/Params 然后再进行直接的 Target-Action 调用。

```c++
- (BOOL)respondsToSelector:(SEL)aSelector;
- (id)performSelector:(SEL)aSelector withObject:(id)object;
```

> &emsp;The mediator with no regist process to split your iOS Project into multiple project.
  没有注册流程的 mediator 将你的 iOS Project 拆分为多个 project。

&emsp;CTMediator 帮助你将项目划分为多个项目（这里是指引入多个自己制作的 pod 库），并使用 Target-Action 模式让 subprojects 相互通信。没有注册过程！


## BeeHive

&emsp;

//# iOS 组件化实践（一）：中间层方案选择
//
//&emsp;CTMediator 以 Target-Action 的方式使用 NSInvocation/NSMethodSignature/SEL 来完成函数的调用执行。以 CTMediator 这个单例类作为中间层，然后把一个模块的（或者一个组件的）公共 API 通过 CTMediator 分类的形式放出来，让需要使用它的模块通过该模块对应的 CTMediator 分类完成调用，这样便解除了各个模块（组件）间的引用依赖。
//
//&emsp;CTMediator 项目中有三个标准文件夹：
//
//+ CTMediator（这也是单独的 repo，完整的中间件就这 100 行代码）
//+ Categories（实际应用中，这是一个单独的 repo，所用需要调度其他模块的人，只需要依赖这个 repo。这个 repo 由 target-action 维护者维护）
//+ DemoModule（target-action 所在的模块，也就是提供服务的模块，这也是单独的 repo，但无需被其他人依赖，其他人通过 category 调用到这里的功能）
//
//&emsp;CTMediator 文件夹中最核心的是 CTMediator 单例类的实现，它提供了两种方式的 Target-Action 调用，一种是我们直接传入 targetName、actionName、params 进行调用，一种是通过类似 `scheme://[target]/[action]?[params]`（`aaa://targetA/actionB?id=1234`） URL 的形式，内部则是对这个 URL 进行处理，首先提取出其中的 Target/Action/Params 然后再进行直接的 Target-Action 调用。
//
//```c++
//- (BOOL)respondsToSelector:(SEL)aSelector;
//- (id)performSelector:(SEL)aSelector withObject:(id)object;
//```
//
//&emsp;




















## 参考链接
**参考链接:🔗**
+ [casatwy/CTMediator](https://github.com/casatwy/CTMediator)
+ [alibaba/BeeHive](https://github.com/alibaba/BeeHive)
+ [iOS应用架构谈 组件化方案](https://casatwy.com/iOS-Modulization.html)

