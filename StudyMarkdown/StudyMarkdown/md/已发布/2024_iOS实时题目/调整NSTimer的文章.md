# 2023年末 - iOS 实时面试题：NSTimer 循环引用问题

&emsp;首先思考下为什么只要提到 NSTimer 就会与循环引用挂钩？主要的原因是：NSTimer 对象会强引用传入的 target，所以 iOS 10 以后，苹果给了我们一个 NSTimer 的 block 为参数的 API，还特意嘱咐了我们：`- parameter:  block  The execution body of the timer; the timer itself is passed as the parameter to this block when executed to aid in avoiding cyclical references`，帮助我们避免循环引用。 

```c++
+ (NSTimer *)timerWithTimeInterval:(NSTimeInterval)interval repeats:(BOOL)repeats block:(void (NS_SWIFT_SENDABLE ^)(NSTimer *timer))block API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0));
```

&emsp;除了循环引用，还有一条我们要谨记在心，当我们创建了一个 `repeats` 参数为 True 的 NSTimer 对象后，当不需要它时一定要调用 `- (void)invalidate;` 终止定时器，不然定时器会一直执行下去，定时器对象不会释放，那么即使 `target` 没有强引用定时器对象（没有循环引用），`target` 也会因为一直被定时器对象强引用而得不到释放，此时 `target` 便内存泄漏了。

&emsp;`invalidate` 函数很重要，不仅是用来停止定时器的，同时也会使定时器对象终止对 `target` 的强引用。若是定时器对象与 `target` 有相互引用的话，相当于主动解除了引用环。例如最简陋的方式：我们在 `viewDidDisappear` 函数中主动调用定时器的 `invalidate` 函数，那么我们的 `ViewController` 还是能正常执行 `dealloc` 并释放的。

&emsp;这里还有一个有意思的点，当 `repeats` 参数使用 `false` 时，不管 `target` 与定时器对象有没有引用环，`target` 都可以在定时器执行后得到释放，定时器对象不再强引用 `target`，这个机制可以让我们延长对象的生命周期到指定的时间。

&emsp;1️⃣：借助中间层，NSObject 子类：

```c++
#import "TimerWrapper.h"

typedef void(^TimerBlock)(void);

@interface TimerWrapper ()

@property (nonatomic, strong) NSTimer *timer;
@property (nonatomic, copy) TimerBlock block;

@end

@implementation TimerWrapper

- (void)startTimer:(void(^)(void))block {
    self.block = block;
    
    if (self.timer != nil) {
        [self.timer invalidate];
        self.timer = nil;
    }
    
    self.timer = [NSTimer scheduledTimerWithTimeInterval:1.0 target:self selector:@selector(timerAction:) userInfo:nil repeats:YES];
}

- (void)timerAction:(NSTimer *)timer {
    if (self.block) {
        self.block();
    }
}

- (void)stopTimer {
    if (self.timer == nil) {
        return;
    }
    
    [self.timer invalidate];
    self.timer = nil;
}

- (void)dealloc {
    NSLog(@"TimerWrapper dealloc");
}

@end
```

&emsp;例如在 ViewController 中使用一个 TimerWrapper 对象开启定时器，然后在 ViewController 的 `dealloc` 函数中调用 `stopTimer` 函数，主动停止定时器。

&emsp;2⃣️：借助中间层，NSProxy 子类：

```c++
#import "TimerProxy.h"

@interface TimerProxy ()

@property (nonatomic, weak) id target;

@end

@implementation TimerProxy

+ (instancetype)proxyWithTarget:(id)target {
    TimerProxy *proxy = [TimerProxy alloc];
    proxy.target = target;
    
    return proxy;
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)sel {
    return [self.target methodSignatureForSelector:sel];
}

- (void)forwardInvocation:(NSInvocation *)invocation {
    [invocation invokeWithTarget:self.target];
}

@end
```

&emsp;`TimerProxy` 弱引用一个 `target`，这个 `target` 可以是我们的 ViewController。在 `TimerProxy` 内部，通过 `forwardInvocation` 把 `TimerProxy` 对象接收的消息转发到 `target` 中去，即把定时器的回调转回到指定的 `target` 中去。这样我们的 ViewController 仅被 `TimerProxy` 弱引用，可以得正常释放，然后在它的 `dealloc` 函数中，终止定时器，破开定时器对象和 `TimerProxy` 对象的强引用，双方都得到了正常的终止和释放。

&emsp;2️⃣：使用 block，iOS 10 以后系统为我们提供了 Block 形式的定时器，那么 iOS 10 之前呢，其实我们也可以手动实现一个：

```c++
#import "NSTimer+Test.h"

@implementation NSTimer (Test)

+ (NSTimer *)cus_scheduledTimerWithTimeInterval:(NSTimeInterval)interval repeats:(BOOL)repeats block:(void (^)(NSTimer * _Nonnull))block {
    return [NSTimer scheduledTimerWithTimeInterval:interval target:self selector:@selector(handle:) userInfo:[block copy] repeats:repeats];
}

+ (void)handle:(NSTimer *)timer {
    void(^block)(NSTimer * _Nonnull) = timer.userInfo;
    if (block != nil) {
        block(timer);
    }
}

- (void)dealloc {
    NSLog(@"timer dealloc 执行！");
}

@end
```

&emsp;通过分类给 NSTimer 添加一个创建 NSTimer 对象的类函数，创建 NSTimer 时我们通过 `userInfo` 参数传递一个自己的自定义 `block`，然后 NSTimer 对象创建时 `target` 传递的是 NSTimer 类对象，因为类对象完全不需要考虑释放问题，它是全局唯一且不需要释放的，所以我们完全不用考虑我们使用自己的对象作为 `target` 时引发的强引用问题。最后当 NSTimer 的 `selector` 执行时，我们从 `userInfo` 中读取到我们传递到 `block` 进行执行，完美避开了引用环问题，实现了和 iOS 10 以后类似的 Block 形式的 NSTimer 创建。     

&emsp;还有 `CADisplayLink` 也是和 `NSTimer` 一样会强引用传入的 `target`，我们可以使用同样的方式解除它们的循环引用。

&emsp;因为 NSTimer 的回调依赖于 RunLoop 中的 timerPort，当 RunLoop 中任务量大和 RunLoop Mode 切换时会导致一些 NSTimer 定时不准问题。

## 参考链接
**参考链接:🔗**
+ [iOS之NSTimer循环引用的解决方案](https://juejin.cn/post/6844903968250789896)
+ [iOS定时器循环引用分析及完美解决方案](https://juejin.cn/post/6937926586167459870)


