#  iOS底层self/super/superclass
基本理解：
+ self 当前方法的调用者
+ super 不是一个指针，编译指示器(标识符)，在程序编译时内部会做一些特殊处理（底层会被编译成 `objc_msgSendSpuer()` 方法）
+ superclass 是一个方法，返回的结果是调用者的父类对象

> super 的作用只是告诉编译器，查找方法的时候不用找自己的方法列表，直接从父类开始找。（调用者还是我自己，然后父类的方法中打印 self，还是调用类）我们在研究方法查找流程的时候知道，发送消息是先找自己的方法，然后递归找父类的方法，而 super 就是告诉编译器，不要从我这找了，直接从父类开始吧。

每当讲解 self 与 super 的时候，都会拿这个经典的代码示例来做说明：
```
// Staff: NSObject Manager: Staff 
// 重写 Manager 的 init 方法

- (instancetype)init {
    self = [super init];
    if (self) {
        NSLog(@"🥵🥵🥵 __FUNCTION__ = %s self = %@", __FUNCTION__, NSStringFromClass([self class]));
        NSLog(@"🥵🥵🥵 __FUNCTION__ = %s super = %@", __FUNCTION__, NSStringFromClass([super class]));
        NSLog(@"🥵🥵🥵 __FUNCTION__ = %s self.superclass = %@", __FUNCTION__, NSStringFromClass([self.superclass class]));
    }
    
    return self;
}

// 打印:
🥵🥵🥵 __FUNCTION__ = -[Manager init] self = Manager
🥵🥵🥵 __FUNCTION__ = -[Manager init] super = Manager // 看到 super 的调用还是 Manager
🥵🥵🥵 __FUNCTION__ = -[Manager init] self.superclass = Staff // 用 self.superclass 才是 Staff

// Leader 继承 Manager，不重写 init 方法
🥵🥵🥵 __FUNCTION__ = -[Manager init] self = Leader
🥵🥵🥵 __FUNCTION__ = -[Manager init] super = Leader
🥵🥵🥵 __FUNCTION__ = -[Manager init] self.superclass = Manager

// 不改变继承关系，使用 [Leader new] 测试：
NSLog(@"🥵🥵🥵 __FUNCTION__ = %s self.superclass = %@", __FUNCTION__, NSStringFromClass([self.superclass.superclass class]));
// 打印：
🥵🥵🥵 __FUNCTION__ = -[Manager init] self.superclass = Manager
// 再加一级 superclass
NSLog(@"🥵🥵🥵 __FUNCTION__ = %s self.superclass = %@", __FUNCTION__, NSStringFromClass([self.superclass.superclass.superclass class]));
// 打印：
🥵🥵🥵 __FUNCTION__ = -[Manager init] self.superclass = NSObject
// 再加一级 superclass
NSLog(@"🥵🥵🥵 __FUNCTION__ = %s self.superclass = %@", __FUNCTION__, NSStringFromClass([self.superclass.superclass.superclass.superclass class]));
// 打印:
🥵🥵🥵 __FUNCTION__ = -[Manager init] self.superclass = (null)
```
> 🌰self 和 super 都是调用者
> Manager 调用 init 那么 self、super 就是 Student。
> Leader 调用 init 那么 self、super 就是 Leader。

> 按照一般的想法，super 应该是 staff，而实际上却是 Manager，是因为在理解 super 之前，容易将 super 和 superclass 混淆，可看到当用 self.superclass 或者 [self superclass] 时，打印的是 Staff 和 Manager。

## 从源码来分析 self 和 super
上面看到：super 是 **编译指示器（标识符）** 而不是指针，self 是方法调用者，是一个指针。
⚠️  只要记住 super 不是 superclass，理解 super 就很容易了。

下面看 clang 命令转换后的代码，cd 到 Mananer.m 的上一级目录，执行 `clang -rewrite-objc Manager.m`:
可看到这些底层代码:
```
static instancetype _I_Manager_init(Manager * self, SEL _cmd) {
    self = ((Manager *(*)(__rw_objc_super *, SEL))(void *)objc_msgSendSuper)((__rw_objc_super){(id)self, (id)class_getSuperclass(objc_getClass("Manager"))}, sel_registerName("init"));
    if (self) {
        NSLog((NSString *)&__NSConstantStringImpl__var_folders_24_5w9yv8jx63bgfg69gvgclmm40000gn_T_Manager_476c8f_mi_0, __FUNCTION__, NSStringFromClass(((Class (*)(id, SEL))(void *)objc_msgSend)((id)self, sel_registerName("class"))));
        NSLog((NSString *)&__NSConstantStringImpl__var_folders_24_5w9yv8jx63bgfg69gvgclmm40000gn_T_Manager_476c8f_mi_1, __FUNCTION__, NSStringFromClass(((Class (*)(__rw_objc_super *, SEL))(void *)objc_msgSendSuper)((__rw_objc_super){(id)self, (id)class_getSuperclass(objc_getClass("Manager"))}, sel_registerName("class"))));
        NSLog((NSString *)&__NSConstantStringImpl__var_folders_24_5w9yv8jx63bgfg69gvgclmm40000gn_T_Manager_476c8f_mi_2, __FUNCTION__, NSStringFromClass(((Class (*)(id, SEL))(void *)objc_msgSend)((id)((Class (*)(id, SEL))(void *)objc_msgSend)((id)self, sel_registerName("superclass")), sel_registerName("class"))));
    }

    return self;
}
```
简化后可以看到:
```
// 第一条 [self class] 对应:
NSStringFromClass(((Class (*)(id, SEL))(void *)objc_msgSend)((id)self, sel_registerName("class")))
// 正常调用 第一个参数是 self，第二个是 "class" 选择子
=> objc_msgSend((id)self, sel_registerName("class"))

// 第二条 [super class] 对应:
NSStringFromClass(((Class (*)(__rw_objc_super *, SEL))(void *)objc_msgSendSuper)((__rw_objc_super){(id)self, (id)class_getSuperclass(objc_getClass("Manager"))}, sel_registerName("class")))

// 调用了 objc_msgSendSuper 函数，且它的第一个入参，与上个方法极不同：
=> objc_msgSendSuper((__rw_objc_super){(id)self, (id)class_getSuperclass(objc_getClass("Manager"))}, sel_registerName("class"))

// 看到它的第一个参数是这样的：
=> (__rw_objc_super){(id)self, (id)class_getSuperclass(objc_getClass("Manager"))}

// 第三条打印:
NSStringFromClass(((Class (*)(id, SEL))(void *)objc_msgSend)((id)((Class (*)(id, SEL))(void *)objc_msgSend)((id)self, sel_registerName("superclass")), sel_registerName("class")))

// 第一个参数是这样的，这个我们也n能一目了，从 self 中执行 superclass 函数找到它的父类
// 且起始函数用的还是 objc_msgSend 
=> (id)((Class (*)(id, SEL))(void *)objc_msgSend)((id)self, sel_registerName("superclass"))
```
从以上信息我们可以得出：
+ 1. 调用 `[self class];` 时，底层调用的还是 `objc_msgSend(...)`
+ 2. 调用 `[super class];` 时，底层调用的是 `objc_msgSendSuper(...)`
+ 3. super 调用的方法，从父类开始查找 `(id)class_getSuperclass(objc_getClass("Manager"))`
+ 4. 看到 `objc_msgSendSuper(...)` 和 `objc_msgSend(...)` 一个极大的不同，它的第一个入参形参是：`__rw_objc_super *`，针对上面的例子，实参传入的是：`{(id)self, (id)class_getSuperclass(objc_getClass("Manager"))}` 

> 看到 `struct __rw_objc_super` 定义：
```
struct __rw_objc_super { 
    struct objc_object *object; 
    struct objc_object *superClass; 
    __rw_objc_super(struct objc_object *o, struct objc_object *s) : object(o), superClass(s) {} 
};
```
从源码中查看 `objc_msgSend(...)` 和 `objc_msgSendSuper(...)` 函数:
Public Headers/message.h P62
```
OBJC_EXPORT void
objc_msgSend(void /* id self, SEL op, ... */ )
    OBJC_AVAILABLE(10.0, 2.0, 9.0, 1.0, 2.0);

OBJC_EXPORT void
objc_msgSendSuper(void /* struct objc_super *super, SEL op, ... */ )
    OBJC_AVAILABLE(10.0, 2.0, 9.0, 1.0, 2.0);
    
    #ifndef OBJC_SUPER
    #define OBJC_SUPER

    /// Specifies the superclass of an instance. 
    /// 指定实例的父类
    
    struct objc_super {
        /// Specifies an instance of a class.
        /// 指定的一个类的实例
        __unsafe_unretained _Nonnull id receiver;

        /// Specifies the particular superclass of the instance to message.
        /// 指定要发送消息的实例的特定父类
    #if !defined(__cplusplus)  &&  !__OBJC2__
        /* For compatibility with old objc-runtime.h header */
        // 兼容老版本的 runtime
        __unsafe_unretained _Nonnull Class class;
    #else
    
        __unsafe_unretained _Nonnull Class super_class;
    #endif
        /* super_class is the first class to search */
        /* 父类是指第一父类 */
    };
    #endif
```
objc_msgSend 相对比较好理解，对应到这里的例子：[self class] 就是 objc_msgSend( 调用者( self ), 调用方法( @selecter( class ) ), ...(其他参数) )

[super class] 中 objc_msgSendSuper( ) 的参数是一个 objc_super 结构体，结构体的第一个参数和 objc_msgSend 一样，也是 receive ( self )，只是通过第二个参数 super_class 知道，方法从父类去找。

> 可以理解为 objc_msgSendSuper( 调用者 ( self ),  从哪开始找 ( super_class ), 调用方法 ( @selecter( class )), ...(其他参数) )

## self 再探
上面说了 super 的理解，接下来对 self 梳理一遍：
```
// 子类重写父类方法，父类方法里面打印 self
// 在子类的重写的里面调用 [super xxxx];
// 看到父类的方法里面的 self 打印的还是子类
// 父类
- (void)readBook {
    NSLog(@"%@", self);
    NSLog(@"%@", NSStringFromClass([self class]));
}
// 子类
- (void)readBook {
    [super readBook];
    NSLog(@"Manager invoke");
}
```
> 上面的代码中，在 Staff 里面的 [self class] 依然是 Manager，因为最开始就说过，self 是调用者，在这里是 Manager 里面通过 super 调用了 readBook，所以调用者依然是 Manager，去 Staff 里面执行，所以 self 依然是 Manager。

## superclass
+ superclass 是一个方法，返回的结果是调用者的父类对象（且有类方法和实例方法）
从源码中看：
```
+ (Class)superclass {
    return self->superclass;
}

- (Class)superclass {
    return [self class]->superclass;
}

+ (Class)class {
    return self;
}

- (Class)class {
    return object_getClass(self);
}
```
看到如果 self 是实例对象，则通过 `[self class]` 先找到它的类对象，然后返回类对象的 `superclass`，如果 self 是类对象的话，则直接返回它的 superclass。

调用 superclass 就是获取对象所在类的 superclass 从对象本质（结构体）中看：
```
struct objc_class : objc_object {
// Class ISA;
Class superclass; // <--- ⚠️⚠️⚠️ 就是它，结构体的成员变量 superclass
cache_t cache;             // formerly cache pointer and vtable
class_data_bits_t bits;    // class_rw_t * plus custom rr/alloc flags

class_rw_t *data() const {
    return bits.data();
}
void setData(class_rw_t *newData) {
    bits.setData(newData);
}
...
};

```

**参考链接:🔗**
[iOS底层-self、super、superclass](https://www.jianshu.com/p/1195fae5a711)
