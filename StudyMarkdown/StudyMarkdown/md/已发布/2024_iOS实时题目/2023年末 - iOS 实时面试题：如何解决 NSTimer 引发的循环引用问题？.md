# 2023年末 - iOS 实时面试题：NSTimer 循环引用问题

&emsp;首先思考下为什么只要提到 NSTimer 就会与循环引用挂钩？主要的原因是：NSTimer 对象会强引用传入的 target，所以 iOS 10 以后，苹果给了我们一个 NSTimer 的 block 为参数的 API，还特意嘱咐了我们：`- parameter:  block  The execution body of the timer; the timer itself is passed as the parameter to this block when executed to aid in avoiding cyclical references`，帮助我们避免循环引用。 

```c++
+ (NSTimer *)timerWithTimeInterval:(NSTimeInterval)interval repeats:(BOOL)repeats block:(void (NS_SWIFT_SENDABLE ^)(NSTimer *timer))block API_AVAILABLE(macosx(10.12), ios(10.0), watchos(3.0), tvos(10.0));
```

&emsp;除了循环引用，还有一条我们要谨记在心，当我们创建了一个 `repeats` 参数为 True 的 NSTimer 对象后，当不需要它时一定要调用 `- (void)invalidate;` 终止定时器，不然定时器会一直执行下去，定时器对象不会释放，那么即使 `target` 没有强引用定时器对象（没有循环引用），`target` 也会因为一直被定时器对象强引用而得不到释放，此时 `target` 便内存泄漏了。

&emsp;`invalidate` 函数很重要，不仅是用来停止定时器的，同时也会使定时器对象终止对 `target` 的强引用，相当于主动解除了引用环。

&emsp;这里还有一个有意思的点，当 `repeats` 参数使用 `false` 时，不管 `target` 与定时器对象有没有引用环，`target` 都可以在定时器执行后得到释放。这个机制可以让我们延长对象的生命周期到指定的时间。

&emsp;1️⃣：借助中间层：

```c++
#import "TimerWrapper.h"

@interface TimerWrapper ()

@property (nonatomic, strong) NSTimer *timer;

@end

@implementation TimerWrapper

- (void)startTimer {
    if (self.timer != nil) {
        [self.timer invalidate];
        self.timer = nil;
    }
    
    self.timer = [NSTimer scheduledTimerWithTimeInterval:1.0 target:self selector:@selector(timerAction:) userInfo:nil repeats:YES];
}

- (void)timerAction:(NSTimer *)timer {
    NSLog(@"Timer Wrapper：%@", timer);
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

&emsp;例如在 ViewController 中使用一个 TimerWrapper 对象开启定时器，需要在 ViewController 的 `dealloc` 函数中调用 `stopTimer` 函数，主动停止定时器。

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

&emsp;因为 NSTimer 的回调依赖于 RunLoop 中的 timerPort，当 RunLoop 中任务量大和 RunLoop Mode 切换时会导致一些 NSTimer 定时不准问题。

## 参考链接
**参考链接:🔗**
+ [iOS之NSTimer循环引用的解决方案](https://juejin.cn/post/6844903968250789896)
+ [iOS定时器循环引用分析及完美解决方案](https://juejin.cn/post/6937926586167459870)
