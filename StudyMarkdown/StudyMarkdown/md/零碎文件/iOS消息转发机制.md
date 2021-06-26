# iOS Runtime 消息转发机制原理和实际用途
**起一个引子告诉大家一些通过这个原理可以用来实现的功能，通过这些用途，可以更深刻的理解消息转发机制的本质，让我们能对费了很长时间理解的知识点的价值得更全面的认识。因为东西搞懂了是来用的，单纯的知道原理并不会对自身的提升太高的价值。**

```
+ (BOOL)resolveInstanceMethod:(SEL)sel {
//    // 判断是否是外界调用的方法
//    if ([NSStringFromSelector(sel) isEqualToString:@"testFunction"]) {
//        // 动态给 TestMessage 添加方法
//
//        return YES;
//    }
    
    return [super resolveInstanceMethod:sel]; // 如果以上没有执行的话，转到父类执行
}

+ (BOOL)resolveClassMethod:(SEL)sel {
//    // 判断是否是外界调用的方法
//    if ([NSStringFromSelector(sel) isEqualToString:@"testClassFunction"]) {
//        // 对类添加类方法，需要将方法添加到元类中
//
//        return YES;
//    }
    
    return [super resolveClassMethod:sel];
}

//- (id)forwardingTargetForSelector:(SEL)aSelector {
//    if ([NSStringFromSelector(aSelector) isEqualToString:@"testFunction"]) {
//        return [[BackupTestMessageOther alloc] init];
//    }
//
//    return [super forwardingTargetForSelector:aSelector];
//}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector {
    // 如果返回 nil 则进行手动创建签名
    if ([super methodSignatureForSelector:aSelector] == nil) {
        NSMethodSignature *sign = [NSMethodSignature signatureWithObjCTypes:"v@:^{NSString=#}"];
        return sign;
    }
    
    return [super methodSignatureForSelector:aSelector];
    
    //    return nil;
}

// 指定一个备用对象去执行指定的函数
- (void)forwardInvocation:(NSInvocation *)anInvocation {
    // 创建备用对象
    BackupTestMessage *backUp = [[BackupTestMessage alloc] init];
    
    // 可修改选择子，也可以不修改，当时备用对象一定要能够处理对应的选择子
    SEL changeSEL = @selector(changeFunction:);
    anInvocation.selector = changeSEL;
    
    SEL sel = anInvocation.selector;
    
//    - (void)getArgument:(void *)argumentLocation atIndex:(NSInteger)idx;
    
    NSString *param = @"Parameter";
    [anInvocation setArgument:(__bridge void * _Nonnull)(param) atIndex:2];
    
    // anInvocation.target // 这个 target 就是最开始执行函数对象
    // anInvocation.methodSignature // 这个 signature 就是上面 methodSignatureForSelector: 函数返回的 // ObjCTypes: 必须有值至少是最简单的方法签名："v@:"
    
    NSLog(@"🌺🌺🌺 %s target = %@", __FUNCTION__, anInvocation.target);
    
    // 判断备用对象是否可以响应传递进来等待响应的 SEL
    if ([backUp respondsToSelector:sel]) {
        [anInvocation invokeWithTarget:backUp];
    } else {
        // 如果备用对象不能响应，则抛出异常
        [self doesNotRecognizeSelector:sel];
    }
}

// 用原对象去执行它的另外一个函数
- (void)forwardInvocation:(NSInvocation *)anInvocation {
    SEL originalSEL = anInvocation.selector;
    SEL invokeNewMethod = @selector(invokeNewMethod);
    anInvocation.selector = invokeNewMethod;
    
    // 判断备用对象是否可以响应传递进来等待响应的 SEL
    if ([anInvocation.target respondsToSelector:invokeNewMethod]) {
        [anInvocation invokeWithTarget:anInvocation.target];
    } else {
        // 如果备用对象不能响应，则抛出异常
        [self doesNotRecognizeSelector:originalSEL];
    }
}
```
## 简略分析 OC 消息发送过程

从一个最常见的方法调用开始：
`id returnValue = [someObject methodName:parameter];` 
**someObject** 为 **"接收者"(receiver)**, **methodName** 为 **"选择子"(selector)**，选择子和参数和称为 **"消息"(message)**。

编译器会将此消息转换为一条 **标准的 C 语言函数调用**，所调用的函数也就是消息传递机制中的核心函数 `objc_msgSend()` 
```
// 函数原型如下：
void objc_msgSend(id self, SEL cmd,...)
```
函数接收 2 个或者 2 个以上的参数，前两个参数依次代表**接收者**、**选择子**，这两个是一定有的参数，后续的就是消息中的参数，顺序不变。
编译器会将刚才的例子中的消息转换为如下函数:
```
id returnValue = objc_msgSend(someObject, @selector(methodName), parameter);
```
`objc_msgSend` 函数会**依据接收者和选择子的类型调用适当的方法**。为此，该方法会在接收者所属的类中搜寻其 “方法列表” (list of methods)，如果能找到**与选择子名称相符的方法**，就跳转到它的实现代码。如果找不到，那就**沿着继承体系继续向上查找**，等找到合适的方法之后再跳转。如果最终还是找不到相符的方法，那就执行 “消息转发” (message-forwarding) 操作。

当 OC 对象接收到**无法解读的消息**后，就会启动 “消息转发” 机制。如果转发机制无法响应的话就会抛出 crash，控制台会有类似如下打印:
```
unrecognized selector sent to instance 0x100502750
*** Terminating app due to uncaught exception 'NSInvalidArgumentException', reason: '-[TestMessage noImplementationMethod]: unrecognized selector sent to instance 0x100502750'
```
这个错误可能是任何一个 iOS 开发者一眼就能看出问题所在: **调用的方法没有被实现**。

## 什么情况下最容易遇到？
+ 一般是在对子类和父类之间做类型转换后，忽略了原始对象类型中并没有实现对应的方法。
+ 使用 `performSelector` 系列函数来执行函数调用（此时编译器会警告，`Undeclared selector 'testFunction'`），执行时函数没有实现且转发失败的话会抛 crash。
+ 还有一种情况可能是我们一时马虎在 .h 中定义了函数，但是在 .m 中忘记实现了，然后该方法恰好又被调用了。😂

```
TestMessage *test = [[TestMessage alloc] init];
[test performSelector:@selector(testFunction)];
[test noImplementationMethod];
```

***假装这里有一张消息转发过程的结构图***

我们可以经由此过程告诉对象应该如何处理未知消息。
消息转发机制共分为 3 个步骤：
1. Method resolution 方法解析处理阶段。
2. Fast forwarding 快速转发阶段（如果可以的话返回一个备用响应对象）。
3. Normal forwarding 常规转发阶段（完整的消息转发）。

```
// 下面这几个函数名，首先在脑子中形成一个印象
// Method resolution
resolveInstanceMethod:
resolveClassMethod:

// Fast forwarding
forwardingTargetForSelector:

// Normal forwarding
forwardInvocation:
methodSignatureForSelector:

// Crash
doesNotRecognizeSelector:
```
## 一、Method resolution 方法解析处理阶段。
+ 对象在收到无法解读的消息后，首先将调用其所属类的下列类方法：
```
+ (BOOL)resolveInstanceMethod:(SEL)sel;
```
该方法的参数就是那个未知的选择子，其返回值为 BOOL 类型，表示这个类是否能新增一个实例方法用以处理这个选择子。**在继续往下执行转发机制之前**，本类有机会新增一个处理此选择子的方法。假如尚未实现的方法不是实例方法而是类方法，那么运行期系统就会调用另外一个方法，该方法与 `resolveInstanceMethod:` 类似，叫做 `resolveClassMethod:`。使用这个方法前提是：**相关方法的实现代码已经写好，只等着运行的时候动态插在类里面就可以了。** 如果该方法返回 NO，就会进入下一个阶段: **返回一个备用响应对象**。

## 二、Fast forwarding 快速转发阶段（如果可以的话返回一个备用响应对象）。

***后面阶段都针对对象来处理，不考虑类方法***

+ 在这一阶段，运行期系统会询问接收者：**能不能把这条消息转给其他接收者来处理。** 与之对应的处理函数是:
```
- (id)forwardingTargetForSelector:(SEL)aSelector;
```
该方法的参数还是那个未知的选择子，若当前接收者能找到备用接收者（对象），则将其返回，若是找不到，就返回 nil。在一个对象内部，可能还有**一系列其他对象**，原对象可经由此方法将能够处理该选择子的相关内部对象返回，这样，在外界看来似乎是原对象亲自处理了这个消息。如果该方法返回 nil，就会进入下一个阶段：**完整的消息转发机制**。

## 三、Normal forwarding 常规转发阶段（完整的消息转发）。
如果第二步返回 self 或者 nil ,则说明没有可以响应的目标 则进入第三步。
第三步的消息转发机制**本质上跟第二步是一样的都是切换接受消息的对象**，但是第三步切换响应目标更复杂一些，第二步里面只需返回一个可以响应的对象就可以了，第三步还需要**手动**将响应方法切换给**备用响应对象**。

+ 如果转发已经来到了这一阶段的话，就需要启动完整的消息转发机制了。首先创建 `NSInvocation` 对象，把与尚未处理的那条消息有关的全部细节封装到里面。此对象包含 `选择子(selector)`、`目标(target)`、及`参数(parameter)`。在触发 `NSInvocation` 对象时，**消息派发系统**会**亲自将消息指派**给目标对象。此阶段会调用下边的两个方法来转发消息：
```
// 返回SEL方法的签名，返回的签名是根据方法的参数来封装的
- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector;

- (void)forwardInvocation:(NSInvocation *)anInvocation;
```
`forwardInvocation` 方法可以实现的很简单：只需要改变调用目标，使消息在新目标上得以调用即可。
当然，还有一种比较有用的实现方法：先以某种方式改变消息，比如追加参数、替换选择子等。
```
@property SEL selector;

- (void)getReturnValue:(void *)retLoc;
- (void)setReturnValue:(void *)retLoc; // 修改返回值

- (void)getArgument:(void *)argumentLocation atIndex:(NSInteger)idx;
- (void)setArgument:(void *)argumentLocation atIndex:(NSInteger)idx; // 添加参数
```
实现 `forwardInvocation` 方法时，如果发现某调用操作不应该本类操作，则需要调用 super 的同名方法，直至 NSObject。如果最后调用了 `NSObject` 类的方法，那么该方法还会继续调用 `doesNotRecognizeSelector`  从而抛出 crash。

**第三步需要手动将响应方法切换给备用响应对象。**

```
// 对应 NSInvocation 类两个函数:
- (void)invoke;
- (void)invokeWithTarget:(id)target;
```
在三个步骤的每一步，消息接受者都还有机会去处理消息。同时，**越往后面处理代价越高**，最好的情况是在第一步就处理消息，**这样 runtime 会在处理完后缓存结果**，下回再发送同样消息的时候，可以提高处理效率。
第二步转移消息的接受者也比进入转发流程的代价要小，如果到最后一步 `forwardInvocation` 的话，就需要处理完整的 `NSInvocation` 对象了。

**实际用途:**
1. JSPatch --iOS动态化更新方案
具体实现bang神已经在下面两篇博客内进行了详细的讲解，非常精妙的使用了，消息转发机制来进行JS和OC的交互，从而实现iOS的热更新。虽然去年苹果大力整改热更新让JSPatch的审核通过率在有一段时间里面无法过审，但是后面bang神对源码进行代码混淆之后，基本上是可以过审了。不论如何，这个动态化方案都是技术的一次进步，不过目前是被苹果爸爸打压的。不过如果在bang神的平台上用正规混淆版本别自己乱来，通过率还是可以的。有兴趣的同学可以看看这两篇原理文章，这里只摘出来用到消息转发的部分。

[bang 神](http://blog.cnbang.net/tech/2808/)
[bang 神](http://blog.cnbang.net/tech/2855/)

2. 为 @dynamic 实现方法
使用 @synthesize 可以为 @property 自动生成 getter 和 setter 方法（现 Xcode 版本中，会自动生成），而 @dynamic 则是告诉编译器，不用生成 getter 和 setter 方法。当使用 @dynamic 时，我们可以使用消息转发机制，来动态添加 getter 和 setter 方法。当然你也用其他的方法来实现。

3. 实现多重代理
利用消息转发机制可以无代码侵入的实现多重代理，让不同对象可以同时代理同个回调，然后在各自负责的区域进行相应的处理，降低了代码的耦合程度。

[链接🔗](https://blog.csdn.net/kingjxust/article/details/49559091)

4. 间接实现多继承
Objective-C 本身不支持多继承，这是因为消息机制名称查找发生在运行时而非编译时，很难解决多个基类可能导致的二义性问题，但是可以通过消息转发机制在内部创建多个功能的对象，把不能实现的功能给转发到其他对象上去，这样就做出来一种多继承的假象。转发和继承相似，可用于为 OC 编程添加一些多继承的效果，一个对象把消息转发出去，就好像他把另一个对象中放法接过来或者“继承”一样。消息转发弥补了 objc 不支持多继承的性质，也避免了因为多继承导致单个类变得臃肿复杂。

**参考链接:**
[Objective-C 的消息转发机制](https://www.jianshu.com/p/03f4a95e43d8)
[iOS Runtime 消息转发机制原理和实际用途](https://www.jianshu.com/p/fdd8f5225f0c)
