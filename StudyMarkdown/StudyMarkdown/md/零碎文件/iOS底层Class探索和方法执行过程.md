#  iOS底层Class探索和方法执行过程
## 1、类（class）结构
在源码中查看类信息:
⚠️⚠️⚠️ 错误追踪: `usr/include/objc/runtime.h` 中的定义是错误的
而且已经过时，且已被标注为不可见 `OBJC2_UNAVAILABLE`:
```
struct objc_class {
    Class _Nonnull isa  OBJC_ISA_AVAILABILITY;

#if !__OBJC2__
    Class _Nullable super_class                              OBJC2_UNAVAILABLE;
    const char * _Nonnull name                               OBJC2_UNAVAILABLE;
    long version                                             OBJC2_UNAVAILABLE;
    long info                                                OBJC2_UNAVAILABLE;
    long instance_size                                       OBJC2_UNAVAILABLE;
    struct objc_ivar_list * _Nullable ivars                  OBJC2_UNAVAILABLE;
    struct objc_method_list * _Nullable * _Nullable methodLists                    OBJC2_UNAVAILABLE;
    struct objc_cache * _Nonnull cache                       OBJC2_UNAVAILABLE;
    struct objc_protocol_list * _Nullable protocols          OBJC2_UNAVAILABLE;
#endif

} OBJC2_UNAVAILABLE;
/* Use `Class` instead of `struct objc_class *` */
```
✅✅✅  正确追踪：在 `objc4` 开源库中，`Project Headers/objc-runtime-new.h` 才是对的，正确是继承自 `objc_object` 的结构体，如下:
```
struct objc_class : objc_object {
// Class ISA; // 继承自 objc_object isa_t isa;
Class superclass; // 8 个字节
cache_t cache;             // formerly cache pointer and vtable // 16 个字节
class_data_bits_t bits;    // class_rw_t * plus custom rr/alloc flags

class_rw_t *data() const { // 非常重要的数据
    return bits.data();
}
void setData(class_rw_t *newData) {
    bits.setData(newData);
}

void setInfo(uint32_t set) {
    ASSERT(isFuture()  ||  isRealized());
    data()->setFlags(set);
}

void clearInfo(uint32_t clear) {
    ASSERT(isFuture()  ||  isRealized());
    data()->clearFlags(clear);
}

// set and clear must not overlap
void changeInfo(uint32_t set, uint32_t clear) {
    ASSERT(isFuture()  ||  isRealized());
    ASSERT((set & clear) == 0);
    data()->changeFlags(set, clear);
}

....

};

```
类对象继承 `objc_object`:
```
struct objc_object {
private:
    isa_t isa;

public:

    // ISA() assumes this is NOT a tagged pointer object
    Class ISA();
    
    ...
};
```
再看下 `objc_class` 中非常重要的数据信息：
```
class_rw_t *data() const {
    return bits.data();
}
```
```
struct class_rw_t {
    // Be warned that Symbolication knows the layout of this structure.
    uint32_t flags;
    uint16_t witness;
#if SUPPORT_INDEXED_ISA
    uint16_t index;
#endif

    explicit_atomic<uintptr_t> ro_or_rw_ext;

    Class firstSubclass;
    Class nextSiblingClass;
    
    ...
    
    const class_ro_t *ro() const {
        auto v = get_ro_or_rwe();
        if (slowpath(v.is<class_rw_ext_t *>())) {
            return v.get<class_rw_ext_t *>()->ro;
        }
        return v.get<const class_ro_t *>();
    }
    
    const method_array_t methods() const {
        auto v = get_ro_or_rwe();
        if (v.is<class_rw_ext_t *>()) {
            return v.get<class_rw_ext_t *>()->methods;
        } else {
            return method_array_t{v.get<const class_ro_t *>()->baseMethods()};
        }
    }
    
    const property_array_t properties() const {
        auto v = get_ro_or_rwe();
        if (v.is<class_rw_ext_t *>()) {
            return v.get<class_rw_ext_t *>()->properties;
        } else {
            return property_array_t{v.get<const class_ro_t *>()->baseProperties};
        }
    }

    const protocol_array_t protocols() const {
        auto v = get_ro_or_rwe();
        if (v.is<class_rw_ext_t *>()) {
            return v.get<class_rw_ext_t *>()->protocols;
        } else {
            return protocol_array_t{v.get<const class_ro_t *>()->baseProtocols};
        }
    }
```
问题1:为什么实例对象的方法要存在类对象中？
想象一下，如果每生成一个实例都会将所有的方法实现拷贝过去，那将会占用很大的内存，所以类生成实例的时候将实例的isa指向自己，调用函数时在isa指向的类中去执行该调用哪个方法的逻辑。

## 2、属性、变量、方法存储在哪里？
### 1、 结构方式探索（控制台打印）
```
@interface HHStaff : NSObject {
    NSString *_hhName;
}
@property (nonatomic, copy) NSString *_hhNick;
@end

// 在 main.m 中初始化这个类
Class cls = NSClassFromString(@"HHStaff");
```
通过 `x` 指令打印 cls 信息，一步一步找到最后的信息：
> 找成员/属性位置

> review
> 从实例对象的的前 8 字节内存中读出类对象：
>> ``` 
>> // 示例代码
>> LGPerson *objc2 = [[LGPerson alloc] init];
>> 
>> Class cls = NSClassFromString(@"LGPerson");
>> NSLog(@"👗👗👗 %p %@ %p", cls, cls, &cls);
>> NSLog(@"Hello, World! %@ - %@",objc1,objc2);
>> // 控制台打印:
>> 👗👗👗 0x1000021d0 LGPerson 0x7ffeefbff570
>> (lldb) p objc2 // 读出 objc2 指针指向的地址
>> (LGPerson *) $2 = 0x0000000101024cc0
>> (lldb) x $2 // 读取该地址里的内容，即当前系统为 LGPerson 对象分配的内存里面保存的数据
>> 0x101024cc0: d5 21 00 00 01 80 1d 00 00 00 00 00 00 00 00 00  .!..............
>> 0x101024cd0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
>> (lldb) p/x 0x001d8001000021d5 & 0x00007ffffffffff8 // 前 8 个字节的内容与 ISA_MASK 做 & 操作
>> (long) $3 = 0x00000001000021d0 // 看到与 👗👗👗 0x1000021d0 打印的 LGPerson 类对象地址完全一样
>> ```

**继续我们上面的 找成员/属性位置**
```
(lldb) x/5gx cls
0x1000021d0: 0x00000001000021a8 (isa) 0x00000001003ee140 (superclass)
0x1000021e0: 0x00000001012180b0 0x0001802400000003 (这 16 位是 cache_t)
0x1000021f0: 0x0000000101217a34 (bits)

// 拿到 bits 地址执行 p 命令
(lldb) p 0x1000021f0
(long) $1 = 4294975984

// 通过强制转换再执行 p 命令
(lldb) p (class_data_bits_t *)0x1000021f0
(class_data_bits_t *) $2 = 0x00000001000021f0

// 想要得到 data() 里面的信息，通过 bits.data() 方法
(lldb) p $2->data()
(class_rw_t *) $3 = 0x0000000101217a30 // (class_rw_t *)

// 查看 (class_rw_t *) $3 里面有哪些信息
(lldb) p *$3

(class_rw_t) $4 = {
  flags = 2148007936
  version = 0
  witness = 1
  ro = 0x0000000100002088
  methods = {
    list_array_tt<method_t, method_list_t> = {
       = {
        list = 0x00000001000020d0
        arrayAndFlag = 4294975696
      }
    }
  }
  properties = {
    list_array_tt<property_t, property_list_t> = {
       = {
        list = 0x0000000100002168
        arrayAndFlag = 4294975848
      }
    }
  }
  protocols = {
    list_array_tt<unsigned long, protocol_list_t> = {
       = {
        list = 0x0000000000000000
        arrayAndFlag = 0
      }
    }
  }
  firstSubclass = nil
  nextSiblingClass = NSUUID
  demangledName = 0x0000000100000f76 "LGPerson"
}
// 查看 ro 里面的信息
(lldb) p $3->ro
(const class_ro_t *) $5 = 0x0000000100002088

// 看看 $5 里面是什么内容
(lldb) p *$5
(const class_ro_t) $6 = {
  flags = 388
  instanceStart = 8
  instanceSize = 24
  reserved = 0
  ivarLayout = 0x0000000100000f7f "\x02"
  name = 0x0000000100000f76 "LGPerson"
  baseMethodList = 0x00000001000020d0
  baseProtocols = 0x0000000000000000
  ivars = 0x0000000100002120
  weakIvarLayout = 0x0000000000000000
  baseProperties = 0x0000000100002168
  _swiftMetadataInitializer_NEVER_USE = {}
}
// 查看对象的信息
(lldb) p $6.ivars
(const ivar_list_t *const) $7 = 0x0000000100002120
// 查看 $7 的信息
(const ivar_list_t) $8 = {
  entsize_list_tt<ivar_t, ivar_list_t, 0> = {
    entsizeAndFlags = 32
    count = 2 // 成员变量 2 个 // 另外一个应该是 isa，但是这里没打印出来
    first = {
      offset = 0x0000000100002198
      name = 0x0000000100000ee5 "_hhName" // 我们定义时添加的成员变量
      type = 0x0000000100000f81 "@\"NSString\""
      alignment_raw = 3
      size = 8
    }
  }
}
// 有了 $6 之后 可以 p 出属性
(lldb) p $6.baseProperties
(property_list_t *const) $9 = 0x0000000100002168

// 查看 $9 信息
(lldb) p *$9
(property_list_t) $10 = {
  entsize_list_tt<property_t, property_list_t, 0> = {
    entsizeAndFlags = 16
    count = 1 // 属性数量
    first = (name = "hhNick", attributes = "T@\"NSString\",C,N,V_hhNick")
  }
}
```
这里看到了成员变量的信息 count  = 2 第一个成员变量是 first = { _hhName }
属性的信息 count = 1 第一个成员变量是 first = { hhNick }

> 找方法存储位置
```
(lldb) p $5->baseMethodList
(method_list_t *const) $11 = 0x00000001000020d0
(lldb) p *$11
(method_list_t) $12 = {
  entsize_list_tt<method_t, method_list_t, 3> = {
    entsizeAndFlags = 26
    count = 3
    first = {
      name = "hhNick"
      types = 0x0000000100000f95 "@16@0:8"
      imp = 0x0000000100000df0 (KCObjc`-[LGPerson hhNick])
    }
  }
}
// 找到其他的方法
(lldb) p $12.get(1)
(method_t) $13 = {
  name = "setHhNick:"
  types = 0x0000000100000f9d "v24@0:8@16"
  imp = 0x0000000100000e20 (KCObjc`-[LGPerson setHhNick:])
}
(lldb) p $12.get(2)
(method_t) $14 = {
  name = ".cxx_destruct"
  types = 0x0000000100000f8d "v16@0:8"
  imp = 0x0000000100000db0 (KCObjc`-[LGPerson .cxx_destruct])
}
```
### 代码打印:
```
// 打印成员变量/属性
void testObjc_copyIvar_copyProperies(Class pClass) {
    unsigned int count = 0;
    Ivar *ivars = class_copyIvarList(pClass, &count);
    for (unsigned int i = 0; i < count; ++i) {
        Ivar const ivar = ivars[i];
        // 获取实例变量名
        const char* cName = ivar_getName(ivar);
        NSString *ivarName = [NSString stringWithUTF8String:cName];
        NSLog(@"class_copyIvarList:%@", ivarName);
    }
    free(ivars);
    
    unsigned int pCount = 0;
    objc_property_t *properties = class_copyPropertyList(pClass, &pCount);
    for (unsigned int i = 0; i < pCount; ++i) {
        objc_property_t const property = properties[i];
        // 获取属性名
        NSString *propertyName = [NSString stringWithUTF8String:property_getName(property)];
        // 获取属性值
        NSLog(@"class_copyProperiesList: %@", propertyName);
    }
    free(properties);
}

// 打印方法列表
// 如果传入类 就是打印 实例方法/静态方法
// 如果传入元类 就打印的是类方法
void testObjc_copyMethodList(Class pClass) {
    unsigned int count = 0;
    Method *methods = class_copyMethodList(pClass, &count);
    for (unsigned int i = 0; i < count; ++i) {
        Method const method = methods[i];
        // 获取方法名
        NSString *key = NSStringFromSelector(method_getName(method));
        NSLog(@"Method, name: %@", key);
    }
    free(methods);
}

void testInstanceMethod_classToMetaclass(Class pClass) {
    const char* className = class_getName(pClass);
    Class metaClass = objc_getMetaClass(className);
    
    Method method1 = class_getInstanceMethod(pClass, @selector(sayHello));
    Method method2 = class_getInstanceMethod(metaClass, @selector(sayHello));
    
    Method method3 = class_getInstanceMethod(pClass, @selector(sayHappy));
    Method method4 = class_getInstanceMethod(metaClass, @selector(sayHappy));
    
    NSLog(@"%p - %p - %p - %p", method1, method2, method3, method4);
    NSLog(@"%s", __func__);
}

void testClassMethod_classToMetaclass(Class pClass) {
    const char* className = class_getName(pClass);
    Class metaClass = objc_getMetaClass(className);
    
    Method method1 = class_getClassMethod(pClass, @selector(sayHello));
    Method method2 = class_getClassMethod(metaClass, @selector(sayHello));
    
    Method method3 = class_getClassMethod(pClass, @selector(sayHappy));
    Method method4 = class_getClassMethod(metaClass, @selector(sayHappy));
    
    NSLog(@"%p - %p - %p - %p", method1, method2, method3, method4);
    NSLog(@"%s", __func__);
}

void testIMP_classToMetaclass(Class pClass) {
    const char* className = class_getName(pClass);
    Class metaClass = objc_getMetaClass(className);
    
    IMP imp1 = class_getMethodImplementation(pClass, @selector(sayHello));
    IMP imp2 = class_getMethodImplementation(metaClass, @selector(sayHello));
    
    IMP imp3 = class_getMethodImplementation(pClass, @selector(sayHappy));
    IMP imp4 = class_getMethodImplementation(metaClass, @selector(sayHappy));
    
    NSLog(@"%p - %p - %p - %p", imp1, imp2, imp3, imp4);
    NSLog(@"%s", __func__);
}
```
**这里牢牢记住这些个方法名:**
+ `Ivar _Nonnull * _Nullable
class_copyIvarList(Class _Nullable cls, unsigned int * _Nullable outCount)` 取得 cls 所有成员变量
+ `const char * _Nullable
ivar_getName(Ivar _Nonnull v)` 取得成员变量的名字
+ `objc_property_t _Nonnull * _Nullable
class_copyPropertyList(Class _Nullable cls, unsigned int * _Nullable outCount)` 取得 cls 的所有属性变量
+ `const char * _Nonnull
property_getName(objc_property_t _Nonnull property)` 取得属性的名字
+ `Method _Nonnull * _Nullable
class_copyMethodList(Class _Nullable cls, unsigned int * _Nullable outCount)` 取得 cls 的所有函数列表。（如果 cls 是类对象则返回的是实例函数列表，如果 cls 是元类，则返回的是类方法列表）
+ `SEL _Nonnull
method_getName(Method _Nonnull m)` 取得函数的 SEL(选择子)
+ `NSString *NSStringFromSelector(SEL aSelector)` 取得选择子的名字
+ `const char * _Nonnull
class_getName(Class _Nullable cls)` 取得 cls 的名字
+ `Class _Nullable
objc_getMetaClass(const char * _Nonnull name)` 取得指定类的元类，注意这里的入参是类的名字，使用  `class_getName` 获得
+ `Method _Nullable
class_getInstanceMethod(Class _Nullable cls, SEL _Nonnull name)` 根据入参的 cls 和选择子，返回对应的实例方法（从类对象或者元类对象中获取方法列表，也可从元类对象中取得类方法）

~~（这里是不区分实例方法还是类方法的，这里只针对函数，假如我们传入类和实例函数的 SEL，则返回的是实例函数，如果传入的元类和类方法的 SEL 则返回类函数，其实就算这样理解也是错误的，实际代码内部是完全不区分实例方法和类方法的，正确的理解是从传入的类的函数列表中寻找对应的入参 SEL，如果找到了返回 Method，如果找不到就返回 nil）。~~

+ `Class _Nullable
objc_getMetaClass(const char * _Nonnull name)` 根据入参的 cls 和选择子，返回对应的类方法

+ `IMP _Nullable
class_getMethodImplementation(Class _Nullable cls, SEL _Nonnull name) ` 返回函数的 IMP，这里才是不区分实例方法和类方法的，根据入参 cls 和 SEL 找到对应的 IMP 就返回，找不到的时候会返回 `_objc_msgForward` 执行消息转发。

+ `unsigned int
method_getNumberOfArguments(Method _Nonnull m)` 返回 Method 的参数数量

+ `void
method_getArgumentType(Method _Nonnull m, unsigned int index, char * _Nullable dst, size_t dst_len) ` 取得 Method 的参数类型

+ `void
method_getReturnType(Method _Nonnull m, char * _Nonnull dst, size_t dst_len) `  取得描述函数返回值类型的字符串

+ `const char * _Nullable
method_getTypeEncoding(Method _Nonnull m)` 返回的 Method 的 TypeEncoding，（包括参数和返回类型）

## 方法
在调试的过程中，最后打印出来的方法包含三个信息 `name`、`types`、`imp`，查看 objc_method 源码：
```objective-c
struct method_t {
    SEL name;
    const char *types;
    MethodListIMP imp; // using MethodListIMP = IMP;

    struct SortBySELAddress :
        public std::binary_function<const method_t&,
                                    const method_t&, bool>
    {
        bool operator() (const method_t& lhs,
                         const method_t& rhs)
        { return lhs.name < rhs.name; }
    };
};
```
分别代表: 方法名、方法类型（方法编码）、方法实现
方法名很简单，这里我们来研究方法类型（方法编码）究竟是怎样的含义
定义一个类 `Student`  并实现几个方法（在 .m 文件中实现，.h 中可以不定义）:
```objective-c
@implementation Student

- (NSString *)methodOne:(int)a str:(NSString *)str {
    return @"";
}

- (NSArray *)methodTwo:(NSArray *)a str:(NSString *)str count:(NSInteger)count {
    return [NSArray new];
}

- (void)redBook {
    
}

+ (NSInteger)methodForClass:(NSInteger)a time:(long)time {
    return 1;
}

- (void)methodInfo:(id)obj {
    unsigned int methodCount = 0;
    Method *methodList = class_copyMethodList([obj class], &methodCount);
    
    for (NSInteger i = 0; i < methodCount; ++i) {
        Method method = methodList[i];
        SEL methodName = method_getName(method);
        
        NSLog(@"✳️✳️✳️ 方法名：%@", NSStringFromSelector(methodName));
        
        // 获取方法的参数类型
        unsigned int argumentsCount = method_getNumberOfArguments(method);
        char argName[512] = {};
        for (unsigned int j = 0; j < argumentsCount; ++j) {
            method_getArgumentType(method, j, argName, 512);
            
            NSLog(@"第 %u 个参数类型为: %s", j, argName);
        }
        
        char returnType[512] = {};
        method_getReturnType(method, returnType, 512);
        NSLog(@"💠💠💠 返回值类型: %s", returnType);
        
        // type encoding
        NSLog(@"Ⓜ️Ⓜ️Ⓜ️ TypeEncoding: %s", method_getTypeEncoding(method));
    }
}

// 打印:
✳️✳️✳️ 方法名：methodInfo:
第 0 个参数类型为: @
第 1 个参数类型为: :
第 2 个参数类型为: @
💠💠💠 返回值类型: v
Ⓜ️Ⓜ️Ⓜ️ TypeEncoding: v24@0:8@16
✳️✳️✳️ 方法名：methodOne:str:
第 0 个参数类型为: @
第 1 个参数类型为: :
第 2 个参数类型为: i
第 3 个参数类型为: @
💠💠💠 返回值类型: @
Ⓜ️Ⓜ️Ⓜ️ TypeEncoding: @28@0:8i16@20
✳️✳️✳️ 方法名：methodTwo:str:count:
第 0 个参数类型为: @
第 1 个参数类型为: :
第 2 个参数类型为: @
第 3 个参数类型为: @
第 4 个参数类型为: q
💠💠💠 返回值类型: @
Ⓜ️Ⓜ️Ⓜ️ TypeEncoding: @40@0:8@16@24q32
✳️✳️✳️ 方法名：redBook
第 0 个参数类型为: @
第 1 个参数类型为: :
💠💠💠 返回值类型: v
Ⓜ️Ⓜ️Ⓜ️ TypeEncoding: v16@0:8
```
### 问题
1. 方法的参数个数和打印的参数不一致
2. 没有打印类方法

打印的参数不是我们定义的函数的参数，实际是底层的 `objc_msgSend` 函数的参数，在底层调用 objc_msgSend 的时候，有两个固定参数：(id)self 和 SEL op，分别是方法的调用者和调用的方法名，后面是跟的其他的参数信息。

```
void
objc_msgSend(void /* id self, SEL op, ... */ )
```
在 `Student.m` 所在文件夹下，执行命令 `clang -rewrite-objc Student.m`，查看生成的 `Student.ccp` 文件:

调用方法 1 的时候会被转换为如下:
```
NSString *oneRes = ((NSString *(*)(id, SEL, int, NSString *))(void *)objc_msgSend)(
(id)stu,
sel_registerName("methodOne:str:"),
(int)oneParm1,
(NSString *)oneParm2
);
```
首先前面是一个函数指针 `(NSString *(*)(id, SEL, int, NSString *))`
然后后面的小括号里面对应了函数的 4 个参数。

⚠️⚠️⚠️ 提示：
在 `objc_msgSend` 函数的定义处有一句注释特别重要: **These functions must be cast to an appropriate function pointer type before being called.** 在调用 `objc_msgSend` 函数之前必须把它强制转化为对应函数指针类型。

调用方法 2 的时候会被转换为如下:
```
NSArray *twoRes = ((NSArray *(*)(id, SEL, NSArray *, NSString *, NSInteger))(void *)objc_msgSend)(
(id)stu,
sel_registerName("methodTwo:str:count:"),
(NSArray *)twoParm1,
(NSString *)twoParm2,
(NSInteger)twoParm3
);
```
调用方法 3 的时候会被转换为如下:
```
((void (*)(id, SEL))(void *)objc_msgSend)(
(id)stu,
sel_registerName("redBook")
);
```
这样我们就理解了函数参数的数量。
+ 方法编码的含义：
这里用方法 1 的 `Ⓜ️Ⓜ️Ⓜ️ TypeEncoding: @28@0:8i16@20` 和 方法 3 的 `Ⓜ️Ⓜ️Ⓜ️ TypeEncoding: v16@0:8` 来比较分析： 
1. 第一个符号表示返回值的标识，1 是 `@` 符号表示返回 OC 对象，3 是 `v` 表示返回 `void`。
> NSString -> @
> void -> v
> int -> i
> float -> f
> double -> d 
> 等等，具体可用 `@encode()` 来验证。
2. 方法 1 的 28 表示所有参数的总长度，同方法 3 的 16，再往后 @ 表示第一个参数的类型，对应函数调用的 self 类型，0 表示从第 0 位开始，: 表示第二个参数的类型，对应 SEL，8 表示从第 8 位开始，因为前面的一个参数(self) 占 8 个字节。下面开始是自定义参数，因为 方法 3 没有自定义函数，所以只有 self 和 SEL 参数，就结束了。接着看 方法 1，i 表示第三个参数是 int 类型，16 表示从第 16 开始，因为前面的两个参数共占据了 16 个字节，self(8) SEL(8). @ 表示第四个参数的类型，这里是 NSString *类型，20 表示从第 20 位开始，前面三个参数共占 20 个字节，self(8)，SEL(8)，int(4)，最后一个参数类型是 NSString(8),所以最前面的总长度是 28.

具体什么类型占用多少个字节，可参考前面的文章，前面有篇文章单独讲过。
```
// 打印结果:
char --> c                  int --> i
short --> s                 long --> q
long long --> q             unsigned char --> C
unsigned int --> I          unsigned short --> S
unsigned long --> Q         float --> f
bool --> B                  void --> v
char * --> *                id --> @
Class --> #                 SEL --> :
int[] --> [3i]              struct --> {person=*i}
union --> (union_type=*i)   int[] --> ^i
```
类方法去哪了？
函数调用时:
```
// 传入的 cls 是类对象
Method *methodList = class_copyMethodList([obj class], &methodCount);
```
那么要获取类方法的话，应该传入元类:
```
Class cls = [obj class];
Class metaCls = object_getClass(cls);
Method *methodList = class_copyMethodList(metaCls, &methodCount);
```
执行结果:
```
✳️✳️✳️ 方法名：methodForClass:time:
第 0 个参数类型为: @
第 1 个参数类型为: :
第 2 个参数类型为: q
第 3 个参数类型为: q
💠💠💠 返回值类型: q
Ⓜ️Ⓜ️Ⓜ️ TypeEncoding: q32@0:8q16q24
```
## 验证方法存储位置
在 LGPerson 中定义并实现两个函数:
```
- (void)je_instanceMethod;
+ (void)je_classMethod;
```
三个相关 API：
```
// 从 cls(类对象/元类对象) 获取实例方法
Method _Nullable
class_getInstanceMethod(Class _Nullable cls, SEL _Nonnull name) // 同时会去搜索超类，而 class_copyMethodList 则不。
// 从 cls(类对象/元类对象) 获取类方法（如果传入的是类对象，则会先找其元类，然后再去查找方法）
Method _Nullable
class_getClassMethod(Class _Nullable cls, SEL _Nonnull name)

// 获取 IMP，如果找不到会返回消息转发，不是返回 nil
IMP _Nullable
class_getMethodImplementation(Class _Nullable cls, SEL _Nonnull name)
```
### 测试 1:
```
Method method1 = class_getInstanceMethod([LGPerson class], @selector(je_instanceMethod));
Method method2 = class_getInstanceMethod(objc_getMetaClass("LGPerson"), @selector(je_instanceMethod));

Method method3 = class_getInstanceMethod([LGPerson class], @selector(je_classMethod));
Method method4 = class_getInstanceMethod(objc_getMetaClass("LGPerson"), @selector(je_classMethod));

NSLog(@"method1 - %p \n\
          method2 - %p\n\
          method3 - %p\n\
          method4 - %p",method1,method2,method3,method4);
          // 打印结果:
          method1 - 0x1000031a8  // 从类对象中获取实例方法，可以读到
          method2 - 0x0 // 从元类对象中获取实例方法，不能读到
          method3 - 0x0 // 从类对象中获取类方法，不能读到
          method4 - 0x100003140 // 从元类对象中读取类方法，可以读到
```
结论：
> method1 和method4 是 有值的，2、3 为nil，也就是说：从类对象中能拿到实例方法，从元类中可以拿到类方法，换句话就是：实例方法在类对象中，而类方法在元类对象中。

### 测试 2:
```
Method method1 = class_getClassMethod([LGPerson class], @selector(je_instanceMethod));
Method method2 = class_getClassMethod(objc_getMetaClass("LGPerson"), @selector(je_instanceMethod));

Method method3 = class_getClassMethod([LGPerson class], @selector(je_classMethod));
Method method4 = class_getClassMethod(objc_getMetaClass("LGPerson"), @selector(je_classMethod));
NSLog(@"method1 - %p \n\
method2 - %p\n\
method3 - %p\n\
method4 - %p",method1,method2,method3,method4);
// 打印:
method1 - 0x0 // 入参是从类对象中获取实例方法，返回 nil
method2 - 0x0 // 入参是从元类对象获取实例方法，返回 nil
method3 - 0x100003140 // 从类对象获取类方法，找到了且与下面完全相同
method4 - 0x100003140 // 从元类对象中获取类方法，找到了
```
⚠️⚠️⚠️ 3 和 4 都有值，且是一样的，盲猜的话 4 应该正常返回，3 应该返回 nil，这里查看源码验证，为什么 3 从类对象中获取到了类方法：
```
// Source/objc-class.mm P580
/***********************************************************************
* class_getClassMethod.  Return the class method for the specified
* class and selector.
**********************************************************************/
Method class_getClassMethod(Class cls, SEL sel)
{
    if (!cls  ||  !sel) return nil;
    // cls->getMeta() 获取 cls 的元类，如果 cls 是类对象则返回它的元类，如果已经是元类对象了，则返回它自己
    return class_getInstanceMethod(cls->getMeta(), sel);
}

// Project Headers/objc-runtime-new.h P1549
// NOT identical to this->ISA when this is a metaclass
Class getMeta() {
    if (isMetaClass()) return (Class)this;
    else return this->ISA();
}

    bool isMetaClass() {
        ASSERT(this);
        ASSERT(isRealized());
#if FAST_CACHE_META
        return cache.getBit(FAST_CACHE_META);
#else
        return data()->flags & RW_META;
#endif
    }
```
结论：
首先要明白，类方法是存储在元类的方法列表中，这里传入的 cls 如果是 [LGPerson class] 只是一个类对象，而不是元类，那么会自动去找其元类，并在其元类中找到相应的方法，如果是传入的元类，那么就直接在其自身的方法列表中去找。3 和 4 虽然写法上不一样，但是进入源码中看一下，其实意思是一样的，所以最后的打印结果是一样的。

### 测试 3：
```
IMP imp1 = class_getMethodImplementation([LGPerson class], @selector(je_instanceMethod));
IMP imp2 = class_getMethodImplementation([LGPerson class], @selector(je_classMethod));
IMP imp3 = class_getMethodImplementation(objc_getMetaClass("LGPerson"), @selector(je_instanceMethod));
IMP imp4 = class_getMethodImplementation(objc_getMetaClass("LGPerson"), @selector(je_classMethod));
NSLog(@"imp1 = %p \n\
    imp2 - %p\n\
    imp3 - %p\n\
    imp4 - %p",imp1,imp2,imp3,imp4);
// 打印：
imp1 = 0x100001620 
imp2 - 0x7fff65594dc0
imp3 - 0x7fff65594dc0
imp4 - 0x1000015f0
```
2 和 3 竟然有值，而且还是有值，看地址的值觉得是栈区地址，看源码验证：
```
IMP class_getMethodImplementation(Class cls, SEL sel)
{
    IMP imp;

    if (!cls  ||  !sel) return nil;

    imp = lookUpImpOrNil(nil, sel, cls, LOOKUP_INITIALIZE | LOOKUP_RESOLVER);

    // Translate forwarding function to C-callable external version
    if (!imp) {
        return _objc_msgForward;
    }

    return imp;
}
```
看到 如果 imp 为空，则 `return _objc_msgForward;` 返回消息转发。
在控制台打印:
```
(lldb) po imp1
(KCObjc`-[LGPerson je_instanceMethod])

(lldb) po imp2
(libobjc.A.dylib`_objc_msgForward)

(lldb) po imp3
(libobjc.A.dylib`_objc_msgForward)

(lldb) po imp4
(KCObjc`+[LGPerson je_classMethod])
```
## 方法的调用流程
### 实例方法调用流程
如代码:
```
Student *obj = [Student new];
[obj redBook];
```
+ clang -rewrite-objc xxx 后：(转换命令：xcrun -sdk iphoneos clang -arch arm64 -rewrite-objc 类名.m -o out.cpp)
```
Student *obj = ((Student *(*)(id, SEL))(void *)objc_msgSend)(
                                                            (id)objc_getClass("Student"), sel_registerName("new")
                                                             );

((void (*)(id, SEL))(void *)objc_msgSend)(
                                          (id)obj,
                                          sel_registerName("redBook")
                                          );
```
+ 执行过程：
1. 判断 obj 是否是 nil，如果是 nil，什么都不会方法。
2. 在 `对象` 的 <缓存方法列表> (也就是类对象的缓存中) 中去找要调用的方法，如果找到的话直接调用。
3. `对象` 的 <缓存方法列表> 里没有找到，就去 <类 的缓存列表> 去找，如果没有找到，就去方法列表中找，找到了就调用并缓存。
4. 还没找到，说明这个类自己没有实现该方法，就会通过 superclass 去向其父类里行 步骤2、步骤3。
5. 当父类指向 nil 的时候还是没有找到 [对象的类的父类->父类的父类->...-> NSObject -> nil]，那么就是没有了，就进行动态解析。
6. 如果没有进行动态解析，那么就会 crash。

上面说了，在方法列表中找到之后的操作是将其缓存起来并调用，如果直接在类对象中找到了方法，我们知道是直接缓存在类对象的缓存信息中。那么如果是在 superclass 中找到方法，缓存在哪个位置呢？在源码中能找到答案：

```
/***********************************************************************
* log_and_fill_cache
* Log this method call. If the logger permits it, fill the method cache.
* cls is the method whose cache should be filled. 
* implementer is the class that owns the implementation in question.
**********************************************************************/
static void
log_and_fill_cache(Class cls, IMP imp, SEL sel, id receiver, Class implementer)
{
#if SUPPORT_MESSAGE_LOGGING
    if (slowpath(objcMsgLogEnabled && implementer)) {
        bool cacheIt = logMessageSend(implementer->isMetaClass(), 
                                      cls->nameForLogging(),
                                      implementer->nameForLogging(), 
                                      sel);
        if (!cacheIt) return;
    }
#endif
    cache_fill(cls, sel, imp, receiver);
}
```

注意: **// Found the method in a superclass. Cache it in this class.**

补充:
类的方法列表位置：
`class_rw_t* data() -> class_ro_t* ro -> baseMethodList`

## 类方法的调用过程
类方法的调用过程和实例方法步骤大致一致，只是找方法的地方不一样。

1. 在类的 <缓存方法列表> [也就是元类的缓存中] 中去找要调用的方法，找到直接调用。
2. 类 的 <缓存方法列表> 里没有找到，就去 <类的元类 方法列表> 里找，找到了调用并缓存。
3. 还没找到，说明这个类自己没有了，就会通过 superclass 去其元类的父类里执行步骤 1、步骤 2。
4. 直到最后的父类指向 nil 的时候 [元类 -> 元类的父类（和父类的元类是一个东西）-> 根元类， 根元类的父类 -> NSObject -> nil] 还没找到，那么就是没有了，就进行动态解析。
5. 如果没有进行动态解析，那么就会 crash。

## 验证根元类的父类是 NSObject
给 NSObject 添加一个分类，分类 .h 定义两个同名的实例函数和类函数，在 .m 中只实现实例函数。然后分别用 NSObject 和 NSObject 的实例对象调用刚刚的函数，看到用 NSObject 类名调用的类函数并没有 crash，而且去到了实例函数里面执行。
当执行类函数时，去到自己的元类里找函数实现，发现找不到，这时候只能去其父类中找，然后就到了 NSObject  里面，然后在 NSObject 的方法列表里找到了同名的实例函数，就可以执行该函数了。

**参考链接:**
[iOS 底层--Class探索和方法执行过程](https://www.jianshu.com/p/7ee7c5987912)
