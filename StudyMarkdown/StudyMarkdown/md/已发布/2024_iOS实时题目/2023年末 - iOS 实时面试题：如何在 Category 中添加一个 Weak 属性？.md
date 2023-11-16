# 2023年末 - iOS 实时面试题：如何在 Category 中添加一个 Weak 属性？

&emsp;当我们想要在分类中添加属性时，需要手动为属性添加 Setter 和 Getter 函数，否则读取或者写入属性值时会因为没有对应的 Setter 和 Getter 函数而直接触发 `unrecognized selector sent to instance` 崩溃，那么我们便借助 Associated Object 机制手动为属性添加 Setter 和 Getter 函数，看到这里便引出第一个面试题：分类中添加的属性可以使用 KVO 监听吗？答案很显然是可以的，然后接着往下，既然刚刚我们提到了关联对象，那么我们再延伸一下，我们手动实现 Setter 函数时会使用 `objc_setAssociatedObject` 这个函数，它的的最后一个参数表示关联策略，但是其中没有对应 `weak` 属性修饰符的策略，那么我们怎么手动实现一个呢？

```c++
Terminating app due to uncaught exception 'NSInvalidArgumentException', reason: '-[NSObject name]: unrecognized selector sent to instance 0x6000000290a0'
Terminating app due to uncaught exception 'NSInvalidArgumentException', reason: '-[NSObject setName:]: unrecognized selector sent to instance 0x600000010330'
```

```c++
- (void)setName:(NSString *)name {
    objc_setAssociatedObject(self, @selector(name), name, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}

- (NSString *)name {
    return objc_getAssociatedObject(self, _cmd);
}
```

&emsp;不由自主打开 Xcode，可以看到关联策略中，唯独缺少 `OBJC_ASSOCIATION_WEAK`，但是有 `assign` 和 `retain`，最先想到的可能就是利用中间层使用 `OBJC_ASSOCIATION_RETAIN_NONATOMIC` + `weak` 属性来实现。 

```c++
/* Associative References */

/**
 * Policies related to associative references.
 * These are options to objc_setAssociatedObject()
 */
typedef OBJC_ENUM(uintptr_t, objc_AssociationPolicy) {
    OBJC_ASSOCIATION_ASSIGN = 0,           /**< Specifies an unsafe unretained reference to the associated object. */
    OBJC_ASSOCIATION_RETAIN_NONATOMIC = 1, /**< Specifies a strong reference to the associated object. 
                                            *   The association is not made atomically. */
    OBJC_ASSOCIATION_COPY_NONATOMIC = 3,   /**< Specifies that the associated object is copied. 
                                            *   The association is not made atomically. */
    OBJC_ASSOCIATION_RETAIN = 01401,       /**< Specifies a strong reference to the associated object.
                                            *   The association is made atomically. */
    OBJC_ASSOCIATION_COPY = 01403          /**< Specifies that the associated object is copied.
                                            *   The association is made atomically. */
};
```

&emsp;1️⃣：借助中间类，关联值被中间类弱引用：

```c++
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface WeakWrapper : NSObject

@property(nonatomic, weak) id weakObj;

@end

NS_ASSUME_NONNULL_END

#import "WeakWrapper.h"

@implementation WeakWrapper

- (instancetype)initWithWeakObj:(id)weakObj {
    self = [super init];
    
    if (self) {
        _weakObj = weakObj;
    }
    
    return self;
}

@end
```

```c++
- (void)setObj:(NSObject *)obj {
    WeakWrapper *wrapper = objc_getAssociatedObject(self, @selector(obj));
    if (!wrapper) {
        wrapper = [[WeakWrapper alloc] initWithWeakObj:obj];
    } else {
        wrapper.weakObj = obj;
    }
    
    objc_setAssociatedObject(self, @selector(obj), wrapper, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
}

- (NSObject *)obj {
    WeakWrapper *wrapper = objc_getAssociatedObject(self, _cmd);
    return wrapper.weakObj;
}
```

&emsp;2️⃣：借助 block 弱引用关联值：

```c++
- (void)setObj:(NSObject *)obj {
    id __weak weakObj = obj;
    id (^block)(void) = ^{ return weakObj; };
    objc_setAssociatedObject(self, @selector(obj), block, OBJC_ASSOCIATION_COPY_NONATOMIC);
}

- (NSObject *)obj {
    id (^block)(void) = objc_getAssociatedObject(self, _cmd);
    return (block ? block() : nil);
}
```

&emsp;3️⃣：关联策略使用 `OBJC_ASSOCIATION_ASSIGN`，重写关联值的 `dealloc` 函数，当 `dealloc` 执行时回调宿主对象把关联值置 `nil`。 

## 参考链接
**参考链接:🔗**
+ [Weak Associated Object](https://sunsetroads.github.io/2020/03/22/weak-associated-object/)
+ [OC-关联对象的发散应用--weak关联对象的实现](https://juejin.cn/post/6875637139833372685)
+ [11-iOS关联对象实现weak属性](https://developer.aliyun.com/article/1321927)
+ [【iOS进阶】- objc_setAssociatedObject实现weak属性](https://blog.nowcoder.net/n/d665ab4157a14cf7b71422ae0d4c490e?from=nowcoder_improve)
