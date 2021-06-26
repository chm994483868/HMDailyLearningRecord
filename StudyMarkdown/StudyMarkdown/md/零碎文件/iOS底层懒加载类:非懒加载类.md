#  iOS底层懒加载类/非懒加载类

懒加载类和非懒加载类的区分很简单，就是看类有没有实现 `load` 方法。
> 非懒加载类:
> 在 App 启动时就开始对其进行实现，
> 因为这个类实现了 load 方法，load 方法实在启动的时候就会被调用的，既然要调用方法，就需要对类进行实现，这个是很好理解的。

⚠️⚠️: 还有两种情况会导致类即使不发送消息，也会提前实现
1. 子类实现了 load 方法：如果一个类（SuperA）有子类（ChildB），子类实现了 load 方法，那么这个类（SuperA）也可以认为是一个非懒加载类。因为在进行类的加载的时候，SuperA 并不在非懒加载类的列表（下面的代码有打印非懒加载类列表），但是在进行 ChildB 初始化的时候，有一个递归加载，加载其父类，也就是说，在这个阶段 SpuerA 也已经开始实现了。

> 非懒加载类
> 没有实现 `load` 方法，在第一次对其发送消息的时候（调用方法之前），才初始化。目的：降低启动的时间，减少启动的工作量。比如一个项目中有 10000 个类，没有必要在启动的时候就全部加载出来，因为这个时候根本就不需要用到它。

**验证哪些类是非懒加载类？**
```
_objc_init()
->
_dyld_objc_notify_register(&map_images, load_images, unmap_image); // 中的 map_images
->
map_images_nolock()
->
_read_images()
->
// 在 // Realize non-lazy classes (for +load methods and static instances)
// 下面的
for (i = 0; i < count; ++i) {
    Class cls = remapClass(classlist[i]);
    // 添加这行代码，打印非懒加载类名
    printf("non-lazy classes: %s\n", cls->mangledName());
    if (!cls) continue;
    ....
}
```
创建几个类并实现部分类的 load 方法，运行并查看打印：
```
// 在 objc4-781 中间添加代码没有打印
// 在 objc4-779.1 添加代码，打印如下:

non-lazy classes: OS_dispatch_queue
non-lazy classes: OS_dispatch_channel
non-lazy classes: OS_dispatch_source
non-lazy classes: OS_dispatch_mach
non-lazy classes: OS_dispatch_queue_runloop
non-lazy classes: OS_dispatch_semaphore
non-lazy classes: OS_dispatch_group
non-lazy classes: OS_dispatch_workloop
non-lazy classes: OS_dispatch_queue_serial
non-lazy classes: OS_dispatch_queue_concurrent
non-lazy classes: OS_dispatch_queue_main
non-lazy classes: OS_dispatch_queue_global
non-lazy classes: OS_dispatch_queue_pthread_root
non-lazy classes: OS_dispatch_queue_mgr
non-lazy classes: OS_dispatch_queue_attr
non-lazy classes: OS_dispatch_mach_msg
non-lazy classes: OS_dispatch_io
non-lazy classes: OS_dispatch_operation
non-lazy classes: OS_dispatch_disk
non-lazy classes: OS_voucher
non-lazy classes: OS_dispatch_data_empty
non-lazy classes: OS_xpc_connection
non-lazy classes: OS_xpc_service
non-lazy classes: OS_xpc_null
non-lazy classes: OS_xpc_bool
non-lazy classes: OS_xpc_double
non-lazy classes: OS_xpc_pointer
non-lazy classes: OS_xpc_date
non-lazy classes: OS_xpc_data
non-lazy classes: OS_xpc_string
non-lazy classes: OS_xpc_uuid
non-lazy classes: OS_xpc_fd
non-lazy classes: OS_xpc_shmem
non-lazy classes: OS_xpc_mach_send
non-lazy classes: OS_xpc_array
non-lazy classes: OS_xpc_dictionary
non-lazy classes: OS_xpc_error
non-lazy classes: OS_xpc_endpoint
non-lazy classes: OS_xpc_serializer
non-lazy classes: OS_xpc_pipe
non-lazy classes: OS_xpc_mach_recv
non-lazy classes: OS_xpc_bundle
non-lazy classes: OS_xpc_service_instance
non-lazy classes: OS_xpc_activity
non-lazy classes: OS_xpc_file_transfer
non-lazy classes: OS_xpc_int64
non-lazy classes: OS_xpc_uint64
non-lazy classes: OS_os_log
non-lazy classes: OS_os_activity
non-lazy classes: __IncompleteProtocol
non-lazy classes: Protocol
non-lazy classes: __NSUnrecognizedTaggedPointer
non-lazy classes: NSObject // 请注意：NSObject 是一个非懒加载类
non-lazy classes: __NSArray0
non-lazy classes: __NSPlaceholderArray
non-lazy classes: __NSPlaceholderSet
non-lazy classes: NSConstantArray
non-lazy classes: __NSDictionary0
non-lazy classes: NSConstantDictionary
non-lazy classes: __NSPlaceholderDate
non-lazy classes: __NSPlaceholderTimeZone
non-lazy classes: __NSPlaceholderOrderedSet
non-lazy classes: NSConstantDate
non-lazy classes: __NSPlaceholderDictionary
non-lazy classes: NSConstantIntegerNumber
non-lazy classes: NSConstantFloatNumber
non-lazy classes: NSConstantDoubleNumber
non-lazy classes: NSApplication
non-lazy classes: NSBinder
non-lazy classes: NSColorSpaceColor
non-lazy classes: NSNextStepFrame
non-lazy classes: _DKEventQuery
non-lazy classes: LGPerson
+[LGPerson load] // 看到调用了 LGPerson 的 load 方法

Hello, World! // 看到以上的所有打印调用都是在 main 函数之前执行的...
```
## 非懒加载类的初始化流程
还是进入 `read_images()` 的 非懒加载类的方法这里
`Realize non-lazy classes (for +load methods and static instances) `里面的
`realizeClassWithoutSwift()` 方法:
```
static Class realizeClassWithoutSwift(Class cls, Class previously)
{
    ro = (const class_ro_t *)cls->data();
    if (ro->flags & RO_FUTURE) {        // 已经初始化过的类才会进入这里
     ...
    } else {
  // 第一次初始化类 创建一个新的rw  并对rw的ro赋值
        rw = (class_rw_t *)calloc(sizeof(class_rw_t), 1);
        rw->ro = ro;
        rw->flags = RW_REALIZED|RW_REALIZING;
        cls->setData(rw);
    }

    isMeta = ro->flags & RO_META;
#if FAST_CACHE_META
    if (isMeta) cls->cache.setBit(FAST_CACHE_META);
#endif
    rw->version = isMeta ? 7 : 0;  // old runtime went up to 6


    // Choose an index for this class.
    // Sets cls->instancesRequireRawIsa if indexes no more indexes are available
    cls->chooseClassArrayIndex();

//递归初始化父类和元类
    supercls = realizeClassWithoutSwift(remapClass(cls->superclass), nil);
    metacls = realizeClassWithoutSwift(remapClass(cls->ISA()), nil);
//.....  修改flags标记、关联父类、元类 等一系列操作
    return cls;
}
```
## 懒加载类的初始化流程
我们说懒加载类是在第一次发送消息（调用方法）的时候才初始化。
具体看看怎样的流程。
在调用方法的时候，底层会首先找到 `lookupImpOrForward()` 方法：

怎么知道是调用这个方法呢？
方法一、底层源码看汇编 慢慢分析
方法二、汇编断点追踪 打开汇编模式（debug->always show disassembly）在 sendMsg call/jmp 通过 in 进入查看，最能能看到 `lookUpImpOrForward at objc-runtime-new.mm: 5989`
```
// 创建一个 LGPerson 类，并且没有实现 load 方法 并且没有子类
LGPerson *per = [LGPerson alloc]; // 在这行上打断点
```
运行，来到这行断点之后，在 `lookUpImpOrForward`里面也打上断点:
```
IMP lookUpImpOrForward(id inst, SEL sel, Class cls, int behavior)
{
❌❌
注意点：这里的cls 是元类，因为这里调用的是类方法 + (id)alloc
如何去验证？   
往下走，在`realizeClassWithoutSwift`方法中 有一个 isMeta的判断，那里为true
❌❌

.......
// 因为类还没有实现，所以会进入这个判断
  if (slowpath(!cls->isRealized())) {
        cls = realizeClassMaybeSwiftAndLeaveLocked(cls, runtimeLock);
    }

//如果类还没有初始化
if (slowpath((behavior & LOOKUP_INITIALIZE) && !cls->isInitialized())) {
        cls = initializeAndLeaveLocked(cls, inst, runtimeLock);
}
......
}

//继续追踪进去，
static Class
realizeClassMaybeSwiftMaybeRelock(Class cls, mutex_t& lock, bool leaveLocked)
{
.....
    if (!cls->isSwiftStable_ButAllowLegacyForNow()) {
        realizeClassWithoutSwift(cls, nil);
        if (!leaveLocked) lock.unlock();
    }
......
}
```
**以上的断点函数都能进去，但是以目前的水平好多都看不懂啊，还有很多标识位都不知道是干啥的**

调用alloc方法之后的流程
lookUpImpOrForward（）
-> realizeClassMaybeSwiftAndLeaveLocked() 传入的是元类，对元类进行实现
-> realizeClassWithoutSwift
->
出来之后 进入lookUpImpOrForward -> initializeAndLeaveLocked 对类进行初始化
->realizeClassWithoutSwift
简单点说：元类进行一次realizeClassWithoutSwift方法 之后 在对 类进行一次 realizeClassWithoutSwift方法。

**参考链接:🔗**
[iOS底层--懒加载类/非懒加载类](https://www.jianshu.com/p/701e3ce35f5b)

