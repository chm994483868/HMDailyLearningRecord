#  iOS底层isa指向探究
## 关系总览
1. 根元类的父类指向根类，根元类的元类指向自己。
2. 根类的父类指向 nil，根类的元类指向根元类。
3. 子类的的父类沿继承体系一直向上直到根类。
4. 子类的元类的父类沿继承体系一直想上直到根元类。
5. 所有类的元类的元类指向根元类。

### 1、对象 isa 指向
#### 基础概念
> 实例对象（objc）、类对象（class）、元类对象（meta）、根元类（root meta）

示例：
`HHManager` 是 `HHStaff` 的子类，`HHStaff` 是 `NSObject` 的子类。
```objective-c
HHManager *mgr = [[HHManager alloc] init];

Class class1 = [mgr class];
Class class2 = [HHManager class];
Class class3 = object_getClass(mgr);
Class class4 = NSClassFromString(@"HHManager");

NSLog(@"👘👘👘 class1 %p-%@", class1, class1);
NSLog(@"👘👘👘 class2 %p-%@", class2, class2);
NSLog(@"👘👘👘 class3 %p-%@", class3, class3);
NSLog(@"👘👘👘 class4 %p-%@", class4, class4);

// 打印：
👘👘👘 class1 0x100003670-HHManager
👘👘👘 class2 0x100003670-HHManager
👘👘👘 class3 0x100003670-HHManager
👘👘👘 class4 0x100003670-HHManager
```
class1、class2、class3、class4 四个值都表示类对象。(整个程序中唯一的一份类对象)
> 类对象就是类的对象
> 上面的代码中 HHManager 是类，上面的四个 Class 变量都指向唯一的一份类对象。
上面代码中 `[mgr class];` 和 `[HHManager class];` 虽然都调用的是 class 方法，但是却不是同一个方法：`[mgr class];` 中的 `class` 是实例方法/静态方法，而 `[HHManager class];` 中的 `class` 是类方法。
```objective-c
+ (Class)class {
    return self;
}

- (Class)class {
    return object_getClass(self);
}

/***********************************************************************
* object_getClass.
* Locking: None. If you add locking, tell gdb (rdar://7516456).
**********************************************************************/
Class object_getClass(id obj)
{
    if (obj) return obj->getIsa();
    else return Nil;
}

```
> 从源码中可以看出 类方法 返回的是自己，而实例方法返回的是对象的 isa。（只是看函数名是 getIsa()，其中还有很多玄机）

如何知道 isa 指向谁呢？在探究这个问题之前，我们需要知道一个问题，通过 `object_getClass(mgr);` 得到的 `isa内存地址是优化过的`，而不是其真实的地址，可看下面源码：

```objective-c
inline Class 
objc_object::getIsa() 
{
    if (fastpath(!isTaggedPointer())) return ISA();

    extern objc_class OBJC_CLASS_$___NSUnrecognizedTaggedPointer;
    uintptr_t slot, ptr = (uintptr_t)this;
    Class cls;

    slot = (ptr >> _OBJC_TAG_SLOT_SHIFT) & _OBJC_TAG_SLOT_MASK;
    cls = objc_tag_classes[slot];
    if (slowpath(cls == (Class)&OBJC_CLASS_$___NSUnrecognizedTaggedPointer)) {
        slot = (ptr >> _OBJC_TAG_EXT_SLOT_SHIFT) & _OBJC_TAG_EXT_SLOT_MASK;
        cls = objc_tag_ext_classes[slot];
    }
    return cls;
}

inline Class 
objc_object::ISA() 
{
    ASSERT(!isTaggedPointer()); 
#if SUPPORT_INDEXED_ISA
    if (isa.nonpointer) {
        uintptr_t slot = isa.indexcls;
        return classForIndex((unsigned)slot);
    }
    return (Class)isa.bits;
#else
    return (Class)(isa.bits & ISA_MASK);
    // 这里 & 操作
    // #   define ISA_MASK        0x0000000ffffffff8ULL
    // ISA_MASK 转换为二进制后 3-35位都是 1，即取出 isa.bits 的 3-35 位的数据 
#endif
}
```
```
#if SUPPORT_INDEXED_ISA
...
#else 

// SUPPORT_INDEXED_ISA 定义
// Define SUPPORT_INDEXED_ISA=1 on platforms that store the class in the isa 
// field as an index into a class table.
// Note, keep this in sync with any .s files which also define it.
// Be sure to edit objc-abi.h as well.
#if __ARM_ARCH_7K__ >= 2  ||  (__arm64__ && !__LP64__)
#   define SUPPORT_INDEXED_ISA 1
#else
#   define SUPPORT_INDEXED_ISA 0
#endif
```
`SUPPORT_INDEXED_ISA` 中的内容可直接忽略，在当前平台下它的值为 0，它表示 isa_t 中存放的 Class 信息是 Class 的地址还是一个索引（根据该索引可在**类信息表**中查找该类结构地址）

> 那么使用 x 指令测试下，通过 x 指令得到 isa 的真实地址与 `object_getClas(mgr);` 的地址相比的话，需要在地址基础上与 ISA_MASK 做 & 操作。

## 获得 isa 地址
### 方法一：
执行如下代码，并在 `Class class1 = object_getClass(staff);` 处打一个断点
```
NSLog(@"👗👗👗 实例对象地址: %p", staff);
Class class1 = object_getClass(staff);
NSLog(@"👗👗👗 类对象: %p %@", class1, class1); // HHStaff
class1 = object_getClass(class1);
NSLog(@"👗👗👗 元类对象: %p %@", class1, class1); // HHStaff 的元类
class1 = object_getClass(class1);
NSLog(@"👗👗👗 根元类对象: %p %@", class1, class1); // 根元类
class1 = object_getClass(class1);
NSLog(@"👗👗👗 根根元类对象: %p %@", class1, class1); // 根元类的 isa 指向自己，所以还是同一个地址
class1 = object_getClass(class1);
NSLog(@"👗👗👗 根根根元类对象: %p %@", class1, class1); // 继续向下，就一值重复了，因为 isa 一直指向的就是自己
NSLog(@"👗👗👗 ...");

// 打印:
👗👗👗 实例对象地址: 0x10056e5c0
👗👗👗 类对象: 0x100003828 HHStaff
👗👗👗 元类对象: 0x100003800 HHStaff
👗👗👗 根元类对象: 0x7fff990900f0 NSObject
👗👗👗 根根元类对象: 0x7fff990900f0 NSObject
👗👗👗 根根根元类对象: 0x7fff990900f0 NSObject
👗👗👗 ... 一直同上面重复下去
```
到了断点的时候，在控制台输入指令 `x staff` 或者 `x (上一步输出的实例对象地址)` 或者 Debug -> Debug Workflow -> View Memory，然后在 Address 框里输入上一步输出的实例对象地址并回车。
```
2020-08-15 15:32:58.174795+0800 Block[12430:419592] 👗👗👗 实例对象地址: 0x10057aa30
(lldb) x staff
0x10057aa30: 2d 38 00 00 01 80 1d 00 00 00 00 00 00 00 00 00  -8..............
0x10057aa40: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
(lldb) x 0x10057aa30
0x10057aa30: 2d 38 00 00 01 80 1d 00 00 00 00 00 00 00 00 00  -8..............
0x10057aa40: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
(lldb) po [HHStaff class]
HHStaff

(lldb) p [HHStaff class]
(Class) $2 = HHStaff
(lldb) p/x 0x 00 1d 80 01 00 00 38 2d & 0x00007ffffffffff8
error: invalid suffix 'x' on integer constant
(lldb) x 0x 00 1d 80 01 00 00 38 2d & 0x00007ffffffffff8
error: memory read takes a start address expression with an optional end address expression.
Expressions should be quoted if they contain spaces or other special characters.
(lldb) p 0x 00 1d 80 01 00 00 38 2d & 0x00007ffffffffff8
error: invalid suffix 'x' on integer constant
(lldb) p 0x001d80010000382d & 0x00007ffffffffff8
(long) $3 = 4294981672
(lldb) p/x 0x001d80010000382d & 0x00007ffffffffff8
(long) $4 = 0x0000000100003828 // 看这里的打印结果: & ISM_MASK 后，正和下一句类对象的地址完全相等。
👗👗👗 类对象: 0x100003828 HHStaff
👗👗👗 元类对象: 0x100003800 HHStaff
👗👗👗 根元类对象: 0x7fff990900f0 NSObject
👗👗👗 根根元类对象: 0x7fff990900f0 NSObject
👗👗👗 根根根元类对象: 0x7fff990900f0 NSObject
👗👗👗 ... 一直同上面重复下去
```
`p/x` 表示以 16 进制的形式打印。
看到 `p/x 0x001d80010000382d & 0x00007ffffffffff8` 结果是 `$4 = 0x0000000100003828` 与下面的 `👗👗👗 类对象: 0x100003828 HHStaff` 完全相等。仔细思考: 
```
(lldb) x staff
0x10057aa30: 2d 38 00 00 01 80 1d 00
```
前八位正是实例对象中前 8 个字节的内容，正是对应实例对象底层结构体中第一个成员变量： `Class isa;` 。

### 方法二：
直接在控制台输出，进入断点后:
```
Block[12531:427789] 👗👗👗 实例对象地址: 0x100724cb0
(lldb) p/x staff->isa
(Class) $0 = 0x001d80010000382d HHStaff
(lldb) 
```
然后点击 `step` 按钮，断点继续往下走一步:
```
2020-08-15 16:46:15.132813+0800 Block[13124:450038] 👗👗👗 实例对象地址: 0x103035170
(lldb) p/x staff->isa
(Class) $0 = 0x001d800100002789 HHStaff
(lldb) p/x 0x001d800100002789 & 0x00007ffffffffff8
(long) $1 = 0x0000000100002788
(lldb) x $1
0x100002788: 60 27 00 00 01 00 00 00 18 01 09 99 ff 7f 00 00  `'..............
0x100002798: 60 63 04 03 01 00 00 00 03 00 00 00 10 80 01 00  `c..............
(lldb) p/x 0x0000000100002760 & 0x00007ffffffffff8
(long) $2 = 0x0000000100002760
2020-08-15 16:48:17.544656+0800 Block[13124:450038] 👗👗👗 类对象: 0x100002788 HHStaff
2020-08-15 16:48:42.402109+0800 Block[13124:450038] 👗👗👗 元类对象: 0x100002760 HHStaff
(lldb) x $2
0x100002760: f0 00 09 99 ff 7f 00 00 f0 00 09 99 ff 7f 00 00  ................
0x100002770: 00 52 61 00 01 00 00 00 07 00 00 00 31 e0 05 00  .Ra.........1...
(lldb) p/x 00007fff990900f0 & 0x00007ffffffffff8
error: invalid digit 'f' in octal constant
2020-08-15 16:52:33.339634+0800 Block[13124:450038] 👗👗👗 根元类对象: 0x7fff990900f0 NSObject
2020-08-15 16:53:07.363945+0800 Block[13124:450038] 👗👗👗 根根元类对象: 0x7fff990900f0 NSObject
2020-08-15 16:53:15.704802+0800 Block[13124:450038] 👗👗👗 根根根元类对象: 0x7fff990900f0 NSObject
(lldb) 
```
由此可见 实例对象的 isa 就是 类对象。用同样的方法，我们可以一步一步的验证对象的 isa 指向、元类对象的 isa 指向、根元类的 isa 指向。
> 实例对象 isa(前 8 个字节) & ISA_MASK -> 类对象
> 类对象 isa(前 8 个字节) & ISA_MASK -> 元类对象
> 元类对象 isa -> 根元类
> 根元类 isa -> 自己

此结果验证了开篇图中的
Instance of Subclass ->
Subclass(Class) ->
Subclass(meta) - >
Root class(meta) -> Root class(meta)

以上是有继承关系的类进行打印，用 NSObject 测试：
打印结果:
```
👗👗👗 实例对象地址: 0x100533910
👗👗👗 类对象: 0x7fff99090118 NSObject
👗👗👗 元类对象: 0x7fff990900f0 NSObject
👗👗👗 根元类对象: 0x7fff990900f0 NSObject
👗👗👗 根根元类对象: 0x7fff990900f0 NSObject
👗👗👗 根根根元类对象: 0x7fff990900f0 NSObject
👗👗👗 ... 一直同上面重复下去
```
## 父类指向
源码:
```
+ (Class)superclass {
    return self->superclass;
}

- (Class)superclass {
    return [self class]->superclass;
}
```
superclass 类方法和实例方法，返回的结果都是类的 superclass 数据。
示例代码:
```
HHStaff *staff = [[HHStaff alloc] init];
Class class1 = [staff superclass];
NSLog(@"👗👗👗 一父类 - %p %@  其父类的类对象-%p", class1, class1, [HHStaff class]);
class1 = [class1 superclass];
NSLog(@"👗👗👗 二父类 - %p %@  其父类的类对象-%p", class1, class1, [NSObject class]);
class1 = [class1 superclass];
NSLog(@"👗👗👗 三父类 - %p %@", class1, class1);
class1 = [class1 superclass];
NSLog(@"👗👗👗 四父类 - %p %@", class1, class1);
class1 = [class1 superclass];
NSLog(@"👗👗👗 五父类 - %p %@", class1, class1);

// 打印:
👗👗👗 一父类 - 0x7fff99090118 NSObject  其父类的类对象-0x100002798
👗👗👗 二父类 - 0x0 (null)  其父类的类对象-0x7fff99090118
👗👗👗 三父类 - 0x0 (null)
👗👗👗 四父类 - 0x0 (null)
👗👗👗 五父类 - 0x0 (null)
```
`[staff superclass];` 可拆解成 `[[staff class] superclass];` 也就是 `[[HHStaff class] superclass]`
由此打印可以验证文章开头的那张图的对象的父类走向
Subclass(class) 【Student class】 ->
Superclass(class) 【【Student class】 superclass】 ->
Root class(class) ->
nil

### 元类的父类的走向
```
HHStaff *objc = [[HHStaff alloc] init];

NSLog(@"👗👗👗 >>>>>> 待验证属性 >>>>>>");
Class class1 = object_getClass([HHStaff class]);
NSLog(@"👗👗👗一 HHStaff的元类对象 - %p %@", class1, class1);
NSLog(@"👗👗👗一 NSObject的类对象 - %p %@", [NSObject class], [NSObject class]);

class1 = object_getClass([NSObject class]);
NSLog(@"👗👗👗一 NSObject的(根)元类对象 - %p %@\n", class1, class1);

NSLog(@"👗👗👗 >>>>>> objc 相关属性>>>>>>");
class1 = object_getClass([objc class]);  // 元类
class1 = [class1 superclass];
NSLog(@"👗👗👗一 元类对象所属父类 - %p %@", class1, class1);

class1 = [class1 superclass];
NSLog(@"👗👗👗二 父类 - %p %@", class1, class1);
class1 = [class1 superclass];
NSLog(@"👗👗👗三 父类 - %p %@", class1, class1);
class1 = [class1 superclass];
NSLog(@"👗👗👗四 父类 - %p %@", class1, class1);
class1 = [class1 superclass];
NSLog(@"👗👗👗五 父类 - %p %@", class1, class1);

// 打印结果:
// 下面描述中出现的根类都表示是 NSObject 类
👗👗👗 >>>>>> 待验证属性 >>>>>>
👗👗👗 一 HHStaff的元类对象 - 0x100003778 HHStaff
👗👗👗 一 NSObject的类对象 - 0x7fff99090118 NSObject
👗👗👗 一 NSObject的(根)元类对象 - 0x7fff990900f0 NSObject

👗👗👗 >>>>>> objc 相关属性>>>>>>
👗👗👗 一 元类对象所属父类 - 0x7fff990900f0 NSObject // 看到 HHStaff 的元类的父类指向向了根元类
👗👗👗 二 父类 - 0x7fff99090118 NSObject // 看到根元类的父类指向了根类
👗👗👗 三 父类 - 0x0 (null)
👗👗👗 四 父类 - 0x0 (null)
👗👗👗 五 父类 - 0x0 (null)
```
这里 "%@" 符号打印的还是元类还是 NSObject，为什么表示类似 NSMetaObject 等这样的呢，至少在名字上双方作出区别？
有待考究！

由此打印可以验证文章开头的那张图的元类对象的父类走向
Subclass(meta) ->
Superclass(meta) ->
Root class(meta) ->
Root class(class) -> nil

**参考链接:**
[iOS 底层-- isa指向探究](https://www.jianshu.com/p/4eedbd8793db)
