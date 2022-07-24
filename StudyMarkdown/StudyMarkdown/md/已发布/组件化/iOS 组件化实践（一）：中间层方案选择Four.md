# iOS 组件化实践（一）：中间层方案选择

&emsp;这里主要分析 [casatwy/CTMediator](https://github.com/casatwy/CTMediator) 和 [alibaba/BeeHive](https://github.com/alibaba/BeeHive) 两个库的源码。

## CTMediator

&emsp;首先我们只看 CTMediator 单例类的内容，可以看到 CTMediator 以 Target-Action 的方式借助字符串和 runtime 使用 NSInvocation（NSMethodSignature、SEL）或者 NSObject 协议的 `- performSelector:withObject:` 来完成函数的调用执行，这样使用 CTMediator 这个中间件我们就可以抹掉我们日常在文件顶部使用 #import 引入的依赖（类）。借助字符串则是指在 CTMediator 类的核心函数：`- performTarget:action:params:shouldCacheTarget:` 中：
 
```c++
- (id _Nullable )performTarget:(NSString * _Nullable)targetName
                        action:(NSString * _Nullable)actionName
                        params:(NSDictionary * _Nullable)params
             shouldCacheTarget:(BOOL)shouldCacheTarget;
```

&emsp;可看到 targetName/actionName/params 三者基本以字符串类型传入，然后使用 runtime 创建 targetName 对象、创建 SEL，然后调用我们熟悉的 NSObject 协议的 `- (BOOL)respondsToSelector:(SEL)aSelector;` 函数判断对应的 SEL 是否被 targetName 对象所实现，然后当对应的 SEL 返回基本类型时使用 NSInvocation 的方式进行函数调用，或者使用我们熟悉的 NSObject 协议的 `- (id)performSelector:(SEL)aSelector withObject:(id)object;` 函数进行函数调用，当 targetName 对象创建失败或者对应的 SEL 函数不存在时都会进行安全的兜底操作。这样我们就可以借助 CTMediator 单例类不进行任何注册操作，在当前上下文环境中直接使用目标对象的类名字符串和目标函数的字符串名字完成函数调用了，把当前上下文环境与目标对象完全解耦。

&emsp;CTMediator 单例类的内容基本就上面那些了，下面我们看一下它是如何在组件化方案中发挥作用的。

&emsp;首先我们创建模块（组件）时都需要依赖 CTMediator 这个单例类，然后把模块（组件）的公开 API 统一放在 CTMediator 类的一个分类中（Swift 中使用的是 CTMediator 类的 extension），而在 CTMediator 分类的实现中通过 `performTarget...` 函数指定 Target 和 Action 的字符串并把参数包装在字典中进行函数调用。然后当模块（组件）之间需要通信时，直接通过模块对应的 CTMediator 分类中定义的公开 API 完成通信，完全不需要模块中的原始文件引用依赖，这样通过 CTMediator 单例类及其分类就解除了需要通信的各个模块之间的强依赖关系，同时 CTMediator 分类中定义好的公开 API 也对函数的参数进行了一定的校验。

&emsp;然后我们每个模块（组件）需要创建一个对应 CTMediator 分类中的 Target 名字的类，并让它实际实现 CTMediator 分类中公开的 API，那么当模块之间发生通信时就会实际执行到这里。

&emsp;刚刚描述的三部分内容正对应了 CTMediator 项目中的三个文件夹： 

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

&emsp;BeeHive 不同于 CTMediator，它提供了完全不同的解藕方式。BeeHive 采用了 Protocol 与实现 Protocol 的指定类绑定的方式实现解耦，看起来它比 CTMediator 难理解一些，看起来更复杂一些，主要是它内部自定义多个名词，以及更多的源码，其实也没什么，我们很容易就能看懂，下面我们一点一点深入学习一下它。

&emsp;首先 BeeHive 有一个注册的过程，这里也对应了上面 CTMediator 中提到了 CTMediator 不需要注册的过程。而这个注册的作用，我们先不明说，留给我们进行思考，我们先看一下 BeeHive 提供的三种不同的注册方式，下面我们分别来看一下这些个注册过程，看懂了这三种注册过程，那么这个注册的作用我们也就一目了然了。

### Annotation 方式注册/注解的方式进行注册

&emsp;通过注解的方式进行注册，注册过程中所涉及的实现细节是与 BeeHive 项目中的 BHAnnotation 类文件绑定在一起的，实际 BHAnnotation 类是一个空类，它其中没有定义任何内容，它的 .h .m 文件仅是用来盛放注解所涉及到的代码的。下面我们直接学习 BHAnnotation.h .m 中的内容。

&emsp;首先是 BHAnnotation.h 中的预处理语句和几个宏定义：

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

&emsp;BeehiveModSectName 和 BeehiveServiceSectName 两个字符串宏定义，分别用来给 module 和 service 起的在 DATA 段中存放数据的 section 名，这里一定要有 Mach-O 的基础知识，要不然会不理解这里的含义。

&emsp;下面的 BeeHiveMod 和 BeeHiveService 两个宏便是在 `__DATA` 段的指定 section 中存入指定的内容（字符串）。直接把我们需要的 mod 和 service 信息在 main 函数调用之前就注入到 Mach-O 中去。

&emsp;在 BeeHive Example 项目中看到：`@BeeHiveMod(ShopModule)`、`@BeeHiveService(UserTrackServiceProtocol,BHUserTrackViewController)`、`@BeeHiveService(HomeServiceProtocol,BHViewController)` 三个宏的使用，把它们展开的话分别如下，看着更清晰一些：

```c++
@class BeeHive; // BeeHive 类的前向声明 
char * kShopModule_mod __attribute((used, section("__DATA,""BeehiveMods"" "))) = """ShopModule""";

@class BeeHive;
char * kUserTrackServiceProtocol_service __attribute((used, section("__DATA,""BeehiveServices"" "))) = "{ \"""UserTrackServiceProtocol""\" : \"""BHUserTrackViewController""\"}";

@class BeeHive;
char * kHomeServiceProtocol_service __attribute((used, section("__DATA,""BeehiveServices"" "))) = "{ \"""HomeServiceProtocol""\" : \"""BHViewController""\"}";
```

&emsp;在 DATA 段的 BeehiveMods 区中写入 `""ShopModule""` 字符串。在 DATA 段的 BeehiveServices 区中写入 `{ \"""UserTrackServiceProtocol""\" : \"""BHUserTrackViewController""\"}` 和 `{ \"""HomeServiceProtocol""\" : \"""BHViewController""\"}` 字符串，这里表明在当前项目注入了 Shop 模块、UserTrackServiceProtocol 协议的实现类是 BHUserTrackViewController、HomeServiceProtocol 协议的实现类是 BHViewController，这也对应了在 BeeHive 项目中通过协议创建对象：

```c++
id<HomeServiceProtocol> homeVc = [[BeeHive shareInstance] createService:@protocol(HomeServiceProtocol)];
id<UserTrackServiceProtocol> v4 = [[BeeHive shareInstance] createService:@protocol(UserTrackServiceProtocol)];
```

&emsp;创建的 homeVc 和 V4 两个变量分别就是 BHViewController 和 BHUserTrackViewController 控制器实例。

&emsp;下面我们接着来看 BHAnnotation.m 文件中的几个函数：

#### BHReadConfiguration

&emsp;读取指定 image section 中的数据，保存的是作为配置信息的一些字符串。mhp 是当前可执行文件启动过程中加载的 image 的 header 指针。

&emsp;单纯看 BHReadConfiguration 函数的话，其实其内容很简单，传入 image header（mhp）指针和 sectionName 字符串，然后在这个 image 中读取 DATA 段中此 section 的内容。其中指针转换、循环取内容的代码看起来可能有点绕，其实是当我们在 section 中保存的是字符串时，此时 section 并不是直接保存字符串的内容，而是字符串的指针（地址），字符串的实际内容位于 TEXT 段的 `__cstring` section 中，其实项目中出现的字符串字面量都会保存在这个 section 中，如截图中所示：

![截屏2022-07-21 20.59.09.png](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/6c988927c78a4249b1714028a4d33797~tplv-k3u1fbpfcp-watermark.image?)

```c++
NSArray<NSString *>* BHReadConfiguration(char *sectionName,const struct mach_header *mhp)
{
    NSMutableArray *configs = [NSMutableArray array];
    unsigned long size = 0;
    
#ifndef __LP64__
    uintptr_t *memory = (uintptr_t*)getsectiondata(mhp, SEG_DATA, sectionName, &size);
#else
    const struct mach_header_64 *mhp64 = (const struct mach_header_64 *)mhp;
    
    // #define SEG_DATA "__DATA" /* the tradition UNIX data segment */
    uintptr_t *memory = (uintptr_t*)getsectiondata(mhp64, SEG_DATA, sectionName, &size);
#endif
    
    // sectionName 区中的数据长度，然后除以一个指针长度，可得出指针个数，这里是因为我们在指定的 section 中保存的都是字符串，所以我们直接读取时，是一个指针。  
    unsigned long counter = size/sizeof(void*);
    
    // 遍历指针读出指向的字符串并保存在一个数组中
    for(int idx = 0; idx < counter; ++idx){
        // 字符地址
        char *string = (char*)memory[idx];
        // 从此地址中读取字符转换为字符串
        NSString *str = [NSString stringWithUTF8String:string];
        if(!str)continue;
        
        BHLog(@"config = %@", str);
        
        // 然后把这些 "配置" 信息保存在数组中并返回
        if(str) [configs addObject:str];
    }
    
    return configs;
}
```

#### initProphet

&emsp;initProphet 函数比较特殊，它被添加了 `__attribute__((constructor))` 修饰，这样 initProphet 函数会在 main 函数之前得到调用，而它的内部只有一行代码，即把 dyld_callback 函数注册为 dyld 加载 image 的回调，这样在 APP 启动过程中每一个 image 被加载后 dyld_callback 函数就会被调用一次，打印一下可发现在 BeeHive 启动过程中 dyld_callback 函数被调用了多次。 

```c++
/*
The following functions allow you to install callbacks which will be called by dyld whenever an image is loaded or unloaded. During a call to _dyld_register_func_for_add_image() the callback func is called for every existing image. Later, it is called as each new image is loaded and bound (but initializers not yet run). The callback registered with _dyld_register_func_for_remove_image() is called after any terminators in an image are run and before the image is un-memory-mapped.
 */
 
extern void _dyld_register_func_for_add_image(void (*func)(const struct mach_header* mh, intptr_t vmaddr_slide));
extern void _dyld_register_func_for_remove_image(void (*func)(const struct mach_header* mh, intptr_t vmaddr_slide));
```

&emsp;如果通过函数 `_dyld_register_func_for_add_image` 注册 image 被加载时的回调函数，那么每当后续有新的 image 被加载但未初始化前 dyld 就会调用注册的回调函数，回调函数的两个入参分别表示加载的 image 的头结构和对应的 Slide 值（虚拟内存偏移值）。如果在调用 `_dyld_register_func_for_add_image` 时系统已经加载了某些 image，则会分别对这些加载完毕的每个 image 调用注册的回调函数。如果你通过函数 `_dyld_register_func_for_remove_image` 注册了 image 被卸载时的回调函数时，那么每当 image 被卸载前都会调用注册的回调函数，回调函数的两个入参分别表示卸载的 image 的头结构和对应的 Slide 值。这两个函数的作用通常用来做程序加载 image 的监控以及一些统计处理。

```c++
__attribute__((constructor))
void initProphet() {
    _dyld_register_func_for_add_image(dyld_callback);
}
```

&emsp;那么在 dyld_callback 函数中调用下面的打印 image name 的函数，可以看到在 main 函数之前在模拟器环境下已经有 335 个 image 被加载，而我们保存在 section 中的自定义数据位于 `xxx/BeeHive_Example.app/BeeHive_Example` image 中，这里如果我们把我们的模块拆分做一个私有 pod 的话，通过 pod 方式把模块引入主工程中，那么我们在模块中通过注解在指定 section 中添加的自定义数据就会位于模块的 image 中，或者说是 pod 子项目的 image 中。即这里对应了虽然 BeeHive 需要一个注册过程，但是并不是说我们必须在主工程中一个一个的把需要的模块进行手动注册，而是我们只要把我们需要的模块导入工程即可（常选 pod 方式），然后在程序启动的过程中扫描所有的 image，自动找出其中所有需要进行注册的 moudels 和 services 进行注册。这种方式注册简单方便，每个模块（组件）可以在自己自行注册，不需要集中注册，整体流程对开发者比较友好。  

```c++
void printImagePath(const struct mach_header *mhp) {
    int dyld_count = _dyld_image_count();
    NSLog(@"☘️☘️☘️ %d", dyld_count);
    for (int i = 0; i < dyld_count; i++) {
        const struct mach_header* image_header_pointer = _dyld_get_image_header(i);
        if (image_header_pointer == mhp) {
            const char * imagePath = _dyld_get_image_name(i);
            
            NSString *res = [NSString stringWithUTF8String:imagePath];
            NSString *imageName = [res componentsSeparatedByString:@"/"].lastObject;
            
            NSLog(@"🍀🍀🍀 %@", imageName);
//            NSLog(@"🍀🍀🍀 %@", res);
        }
    }
}
```

&emsp;xcode 控制台部分打印截取。

```c++

...
2022-07-23 22:00:58.765894+0800 BeeHive_Example[47221:3018663] ☘️☘️☘️ 335
2022-07-23 22:00:58.766079+0800 BeeHive_Example[47221:3018663] 🍀🍀🍀 /Users/hmc/Library/Developer/CoreSimulator/Devices/377B8219-5922-46DB-9112-79701E6CC006/data/Containers/Bundle/Application/EC97BACE-90F1-4C31-B852-FDC525CEBEB0/BeeHive_Example.app/BeeHive_Example
2022-07-23 22:00:58.766805+0800 BeeHive_Example[47221:3018663] config = ShopModule
2022-07-23 22:00:58.767542+0800 BeeHive_Example[47221:3018663] ShopModule init
2022-07-23 22:00:58.767784+0800 BeeHive_Example[47221:3018663] config = { "HomeServiceProtocol" : "BHViewController"}
2022-07-23 22:00:58.767854+0800 BeeHive_Example[47221:3018663] config = { "UserTrackServiceProtocol" : "BHUserTrackViewController"}
...

```

#### dyld_callback

&emsp;dyld_callback 函数中主要进行读取 BeehiveMods 和 BeehiveServices section 中的数据，然后把 module 信息和 services 信息保存到 BHModuleManager 和 BHServiceManager 单例类的属性中去，即完成了注册过程。

```c++
static void dyld_callback(const struct mach_header *mhp, intptr_t vmaddr_slide)
{
    printImagePath(mhp);
    
    // register dynamic module
    
    // 读取 Data 段 BeehiveMods 区中的数据，如果存在的话 
    NSArray *mods = BHReadConfiguration(BeehiveModSectName, mhp);
    
    // 在本项目中读出了：ShopModule
    for (NSString *modName in mods) {
        Class cls;
        if (modName) {
            cls = NSClassFromString(modName);
            
            if (cls) {
                // 把读出的 module 注册到 BHModuleManager 单例类的属性中去
                [[BHModuleManager sharedManager] registerDynamicModule:cls];
            }
        }
    }
    
    // register services
    
    // 读取 Data 段 BeehiveServices 区中的数据，如果存在的话
    NSArray<NSString *> *services = BHReadConfiguration(BeehiveServiceSectName,mhp);
    
    // 在本项目中读出了：{ "HomeServiceProtocol" : "BHViewController"}、{ "UserTrackServiceProtocol" : "BHUserTrackViewController"}
    for (NSString *map in services) {
        NSData *jsonData =  [map dataUsingEncoding:NSUTF8StringEncoding];
        NSError *error = nil;
        id json = [NSJSONSerialization JSONObjectWithData:jsonData options:0 error:&error];
        if (!error) {
            if ([json isKindOfClass:[NSDictionary class]] && [json allKeys].count) {
                
                NSString *protocol = [json allKeys][0];
                NSString *clsName  = [json allValues][0];
                
                if (protocol && clsName) {
                    // 把 protocol 和实现该 protocol 的 clsName 成对注册到 BHServiceManager 单例类的 allServicesDict 属性中去
                    [[BHServiceManager sharedManager] registerService:NSProtocolFromString(protocol) implClass:NSClassFromString(clsName)];
                }
                
            }
        }
    }
}
```

&emsp;到这里通过注解的方式注册就看完了，大意也就是在 APP 启动之前读取一些自定义数据待后续进行使用。下面我们看第二种注册方式。

### 读取 .plist 文件内容的方式进行注册

&emsp;把需要的 modules 和 services 数据保存在 .plist 文件中，然后在 `application:didFinishLaunchingWithOptions:` 函数中进行读取。

&emsp;在 TestAppDelegate.m 文件中有如下代码：

```c++
- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    [BHContext shareInstance].application = application;
    [BHContext shareInstance].launchOptions = launchOptions;
    
    [BHContext shareInstance].moduleConfigName = @"BeeHive.bundle/BeeHive"; // 可选，默认为 BeeHive.bundle/BeeHive.plist
    [BHContext shareInstance].serviceConfigName = @"BeeHive.bundle/BHService";
    
    [BeeHive shareInstance].enableException = YES;
    [[BeeHive shareInstance] setContext:[BHContext shareInstance]];
    
    [[BHTimeProfiler sharedTimeProfiler] recordEventTime:@"BeeHive::super start launch"];

    [super application:application didFinishLaunchingWithOptions:launchOptions];
    
    ...
    
    return YES;
}
```

&emsp;BHContext 是一个单例类，保存许多上下文信息。其中 moduleConfigName 和 serviceConfigName 两个属性分别记录了 .plist 文件的路径和名字。在 BeeHive 类的 setContext 函数中会对 .plist 文件进行读取并对其中的 modules 和 services 数据：

```c++
-(void)setContext:(BHContext *)context
{
    _context = context;
    
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        [self loadStaticServices];
        [self loadStaticModules];
    });
}
```

&emsp;




































## 参考链接
**参考链接:🔗**
+ [casatwy/CTMediator](https://github.com/casatwy/CTMediator)
+ [alibaba/BeeHive](https://github.com/alibaba/BeeHive)
+ [iOS应用架构谈 组件化方案](https://casatwy.com/iOS-Modulization.html)
+ [深入iOS系统底层之 image 文件操作API介绍](https://blog.csdn.net/ios8988/article/details/89510599)






