# iOS 组件化实践（一）：中间层方案选择

## CTMediator

&emsp;首先我们只看 CTMediator 单例类的内容，可以看到 CTMediator 以 Target-Action 的方式借助字符串和 runtime 使用 NSInvocation（NSMethodSignature、SEL）或者 NSObject 协议的 `- performSelector:withObject:` 来完成函数的调用执行，这样使用 CTMediator 这个中间件我们就可以抹掉我们日常在文件顶部使用 #import 引入的依赖（类）。借助字符串则是指在 CTMediator 类的核心函数：`- performTarget:action:params:shouldCacheTarget:` 中：
 
```c++
- (id _Nullable )performTarget:(NSString * _Nullable)targetName
                        action:(NSString * _Nullable)actionName
                        params:(NSDictionary * _Nullable)params
             shouldCacheTarget:(BOOL)shouldCacheTarget;
```

&emsp;可看到 targetName/actionName/params 三者基本以字符串类型传入，然后使用 runtime 创建 targetName 对象、创建 SEL，然后调用我们熟悉的 NSObject 协议的 `- (BOOL)respondsToSelector:(SEL)aSelector;` 函数判断对应的 SEL 是否被 targetName 对象所实现，然后当对应的 SEL 返回基本类型时使用 NSInvocation 的方式进行函数调用，或者使用我们熟悉的 NSObject 协议的 `- (id)performSelector:(SEL)aSelector withObject:(id)object;` 函数进行函数调用，当 targetName 对象创建失败或者对应的 SEL 函数不存在时都会进行安全的兜底操作。这样我们就可以借助 CTMediator 单例类不进行任何注册操作，在当前上下文环境中直接使用目标对象的类名字符串和目标函数的字符串名字完成函数调用了，把当前上下文环境与目标对象完全解耦。

&emsp;CTMediator 单例类的内容看完了，下面我们看一下它是如何在组件化方案中发挥作用的。

&emsp;首先我们创建模块（组件）时都需要依赖 CTMediator 这个单例类，然后把模块（组件）的公开 API 统一放在 CTMediator 类的一个分类中（Swift 中使用的是 CTMediator 类的 extension），而在 CTMediator 分类的实现中通过 `performTarget...` 函数指定 Target 和 Action 的字符串并把参数包装在字典中进行函数调用。然后当模块（组件）之间需要通信时，直接通过模块对应的 CTMediator 分类中定义的公开 API 完成通信，完全不需要模块中的原始文件引用依赖，这样通过 CTMediator 单例类及其分类就解除了需要通信的各个模块之间的强依赖关系，同时 CTMediator 分类中定义好的公开 API 也对函数的参数进行了一定的校验。

&emsp;然后我们每个模块（组件）需要创建一个对应 CTMediator 分类中的 Target 名字的类，并让它实际实现 CTMediator 分类中公开的 API，那么当模块之间发生通信时就会实际执行到这里。上面的三部分内容正对应了 CTMediator 项目中的三个文件夹： 

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

&emsp;BeeHive 不同于 CTMediator，它提供了完全不同的解藕方式。BeeHive 采用了 Protocol 与实现 Protocol 的指定类绑定的方式实现解藕，看起来它比 CTMediator 难理解一些，看起来更复杂一些，其实也没有，下面我们一点一点深入学习一下它。

&emsp;首先 BeeHive 有一个注册的过程，这里也对应了上面 CTMediator 中提到了 CTMediator 不需要注册的过程。而这个注册的作用，我们先不明说，留给我们进行思考，我们先看一下 BeeHive 提供的三种不同的注册方式，下面我们分别来看一下这些个注册过程，看懂了这三种注册过程，那么这个注册的作用我们也就一目了然了。

### Annotation 方式注册/注解的方式进行注册

&emsp;通过注解的进行注册，这个注册过程与 BeeHive 项目中的 BHAnnotation 类是绑定在一起的，下面我们直接学习 BHAnnotation 这个类，首先是 BHAnnotation.h 中的预处理语句和几个宏定义：

```c++

#ifndef BeehiveModSectName
#define BeehiveModSectName "BeehiveMods"
#endif

#ifndef BeehiveServiceSectName
#define BeehiveServiceSectName "BeehiveServices"
#endif


#define BeeHiveDATA(sectname) __attribute((used, section("__DATA,"#sectname" ")))

#define BeeHiveMod(name) \
class BeeHive; char * k##name##_mod BeeHiveDATA(BeehiveMods) = ""#name"";

#define BeeHiveService(servicename,impl) \
class BeeHive; char * k##servicename##_service BeeHiveDATA(BeehiveServices) = "{ \""#servicename"\" : \""#impl"\"}";
```

&emsp;首先是 BeehiveModSectName 和 BeehiveServiceSectName 两个字符串宏定义，分别用来给 module 和 service 起的在 DATA 段中存放数据的 section 名，这里一定要有 Mach-O 的基础知识，要不然会不理解这里的含义。

&emsp;下面的 BeeHiveMod 和 BeeHiveService 两个宏便是在 `__DATA` 段的指定 section 中存入指定的内容。直接把我们需要的 mod 和 service 信息在 main 函数之前就注入到 Mach-O 中去。

&emsp;在 BeeHive Example 项目中看到：`@BeeHiveMod(ShopModule)`、`@BeeHiveService(UserTrackServiceProtocol,BHUserTrackViewController)`、`@BeeHiveService(HomeServiceProtocol,BHViewController)` 三个宏的使用，把它们展开的话分别如下看着更清晰一些：

```c++
@class BeeHive; 
char * kShopModule_mod __attribute((used, section("__DATA,""BeehiveMods"" "))) = """ShopModule""";

@class BeeHive;
char * kUserTrackServiceProtocol_service __attribute((used, section("__DATA,""BeehiveServices"" "))) = "{ \"""UserTrackServiceProtocol""\" : \"""BHUserTrackViewController""\"}";

@class BeeHive;
char * kHomeServiceProtocol_service __attribute((used, section("__DATA,""BeehiveServices"" "))) = "{ \"""HomeServiceProtocol""\" : \"""BHViewController""\"}";
```

&emsp;在 DATA 段的 BeehiveMods 区中写入 `""ShopModule""` 字符串。在 DATA 段的 BeehiveServices 区中写入 `{ \"""UserTrackServiceProtocol""\" : \"""BHUserTrackViewController""\"}` 和 `{ \"""HomeServiceProtocol""\" : \"""BHViewController""\"}` 字符串，这里表明在当前项目注入了 Shop 模块、UserTrackServiceProtocol 协议的实现类是 BHUserTrackViewController、HomeServiceProtocol 协议的实现类是 BHViewController  





/Users/hmc/Library/Developer/Xcode/DerivedData/kyyApp-brhibjeslxxhbeedjiggefiycnrd/Build/Intermediates.noindex/ArchiveIntermediates/kyyApp/BuildProductsPath/Release-iphoneos/










## 参考链接
**参考链接:🔗**
+ [casatwy/CTMediator](https://github.com/casatwy/CTMediator)
+ [alibaba/BeeHive](https://github.com/alibaba/BeeHive)
+ [iOS应用架构谈 组件化方案](https://casatwy.com/iOS-Modulization.html)




