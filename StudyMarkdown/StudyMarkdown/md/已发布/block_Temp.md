# Block 临时文件


## 此段代码可作为区分全局、堆区、栈区 block 的实例代码
```c++
typedef void(^Blk_T)(void);
void (^globalBlock0)(void) = ^{
    NSLog(@"全局区的 block");
};

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        
        // 0. 在全局区定义的 NSGlobalBlock
        NSLog(@"🎉🎉🎉 GlobalBlock0 is %@", globalBlock0);
        // 🎉🎉🎉 GlobalBlock0 is <__NSGlobalBlock__: 0x100002020>
        
        // 1. 不捕获外部变量时是 NSGlobalBlock。
        //（此处即使发生赋值时 ARC 下会调用 copy，但是由于左值是 NSGlobalBlock，它调用 copy 函数时依然返回它自己）
        void (^globalBlock1)(void) = ^{ };
        NSLog(@"🎉🎉🎉 GlobalBlock1 is %@", globalBlock1);
        // 🎉🎉🎉 GlobalBlock1 is <__NSGlobalBlock__: 0x100002060>
        
        static int b = 10;
        // 2. 仅捕获外部静态局部变量的是 NSGlobalBlock
        //（此处即使发生赋值时 ARC 下会调用 copy，但是由于左值是 NSGlobalBlock，它调用 copy 函数时依然返回它自己）
        void (^globalBlock2)(void) = ^{
            b = 20;
        };
        NSLog(@"🎉🎉🎉 GlobalBlock2 is %@", globalBlock2);
        // 🎉🎉🎉 GlobalBlock2 is <__NSGlobalBlock__: 0x100002080>

        int a = 0;
        // 3. 仅捕获外部局部变量是的 NSStackBlock
        NSLog(@"🎉🎉🎉 StackBlock3 is %@", ^{ NSLog(@"%d", a); });
        // 🎉🎉🎉 StackBlock3 is <__NSStackBlock__: 0x7ffeefbff4c8>

        // 4. ARC 下 NSStackBlock 赋值给 __strong 变量时发生 copy，创建一个 NSMallocBlock 赋给右值
        // MRC 下编译器不会自动发生 copy，赋值以后右值同样也是 NSStackBlock，如果想实现和 ARC 同样效果需要手动调用 copy
        void (^mallocBlock)(void) = ^{
            NSLog(@"%d", a);
        };
        NSLog(@"🎉🎉🎉 MallocBlock4 is %@", mallocBlock);
        // 🎉🎉🎉 MallocBlock4 is <__NSMallocBlock__: 0x1005005e0>
        
        // 5. ARC 或 MRC 下赋值给 __weak/__unsafe_unretained 变量均不发生 copy，
        // 手动调用 copy 是可转为 NSMallocBlock
        // __unsafe_unretained / __weak
        __unsafe_unretained Blk_T mallocBlock2;
        mallocBlock2 = ^{
            NSLog(@"%d", a);
        };
        // mallocBlock2 是：NSStackBlock，其实应该和上面的 StackBlock 写在一起
        NSLog(@"🎉🎉🎉 MallocBlock5 is %@", mallocBlock2);
        // 🎉🎉🎉 MallocBlock5 is <__NSStackBlock__: 0x7ffeefbff518>
        
    }
    return 0;
}
```

## **小测试**
+ A:
```
void exampleA() {
    // ARC 和 MRC 下均为栈区 Block
    char a = 'A';
    NSLog(@"🔔🔔🔔 %@", ^{ printf("%c\n", a);});
}
// MRC: 🔔🔔🔔 <__NSStackBlock__: 0x7ffeefbff538>
// ARC: 🔔🔔🔔 <__NSStackBlock__: 0x7ffeefbff538>

void exampleA() {
    // ARC 和 MRC 下均为全局 Block
    NSLog(@"🔔🔔🔔 %@", ^{ printf("🟪🟪🟪");});
}
// ARC: 🔔🔔🔔 <__NSGlobalBlock__: 0x100002048>
// MRC: 🔔🔔🔔 <__NSGlobalBlock__: 0x100001038>
```
```
void exampleA() {
    // ARC 和 MRC 下均为栈区 Block
    char a = 'A';
    // 没有赋值操作
    ^{
        printf("🔔🔔🔔 %c\n", a);
    }();
}

// MRC: 🔔🔔🔔 A
// ARC: 🔔🔔🔔 A
```
+ B:
```
void exampleB_addBlockToArray(NSMutableArray *array) {
    char b = 'B';
    // 原以为栈区 Block，ARC 下是堆区 Block
    // MRC 下估计是栈区 Block，执行的时候崩溃了
    [array addObject:^{
        printf("🔔🔔🔔 %c\n", b);
    }];
    NSLog(@"🔔🔔🔔 %@", array);
}

void exampleB() {
    NSMutableArray *array = [NSMutableArray array];
    exampleB_addBlockToArray(array);
    
    NSLog(@"🔔🔔🔔 %@", [array objectAtIndex:0]);
    
    void(^block)() = [array objectAtIndex:0];
    
    NSLog(@"🔔🔔🔔 %@", block);
    block();
}

// ARC: 🔔🔔🔔 ( "<__NSMallocBlock__: 0x102840050>")
        🔔🔔🔔 <__NSMallocBlock__: 0x100611690>
        🔔🔔🔔 <__NSMallocBlock__: 0x100611690>
        🔔🔔🔔 B
// MRC: 崩溃 ， 在 addObject 添加 block 时调用 copy 函数，能正常运行。
```
+ C:
```
void exampleC_addBlockToArray(NSMutableArray *array) {
  // 全局 Global 
  [array addObject:^{
    printf("🔔🔔🔔 C\n");
  }];
}

void exampleC() {
    NSMutableArray *array = [NSMutableArray array];
    exampleC_addBlockToArray(array);
    NSLog(@"🔔🔔🔔 %@", [array objectAtIndex:0]);
    void(^block)() = [array objectAtIndex:0];
    NSLog(@"🔔🔔🔔 %@", block);
    block();
}

// ARC: 🔔🔔🔔 <__NSGlobalBlock__: 0x100002068>
        🔔🔔🔔 <__NSGlobalBlock__: 0x100002068>
        🔔🔔🔔 C
        
// MRC: 🔔🔔🔔 <__NSGlobalBlock__: 0x100001078>
        🔔🔔🔔 <__NSGlobalBlock__: 0x100001078>
        🔔🔔🔔 C
```
+ D:
```
typedef void(^dBlock)();
dBlock exampleD_getBlock() {
    // ARC 栈区 block 作为函数返回值时会自动调用 copy
    // MRC 下编译直接报错: Returning block that lives on the local stack
    // 主动 block 尾部调 copy 可解决
    
    char d = 'D';
    return ^{
        printf("🔔🔔🔔 %c\n", d);
    };
}

void exampleD() {
    NSLog(@"🔔🔔🔔 %@", exampleD_getBlock());
    exampleD_getBlock()();
}
// ARC: 🔔🔔🔔 <__NSMallocBlock__: 0x100500d00>
        🔔🔔🔔 D
```
+ E:
```
typedef void(^eBlock)();
eBlock exampleE_getBlock() {
    char e = 'E';
    void(^block)() = ^{
        printf("🔔🔔🔔 %c\n", e);
    };
    NSLog(@"🔔🔔🔔 %@", block);
    return block;
}

void exampleE() {
    NSLog(@"one 🔔🔔🔔 %@", exampleE_getBlock());
    
    eBlock block = exampleE_getBlock();
    NSLog(@"two 🔔🔔🔔 %@", block);
    block();
}
// MRC 下即使是栈区 Block 也正常执行了，且两次调用函数返回的是一样的地址
// MRC: 🔔🔔🔔 <__NSStackBlock__: 0x7ffeefbff508>
        🔔🔔🔔 <__NSStackBlock__: 0x7ffeefbff508>
        
        🔔🔔🔔 <__NSStackBlock__: 0x7ffeefbff508>
        🔔🔔🔔 <__NSStackBlock__: 0x7ffeefbff508>
        🔔🔔🔔 P
        
        // 两次地址不同
// ARC: 🔔🔔🔔 <__NSMallocBlock__: 0x100550d10>
        🔔🔔🔔 <__NSMallocBlock__: 0x100550d10>
        
        🔔🔔🔔 <__NSMallocBlock__: 0x100602d00>
        🔔🔔🔔 <__NSMallocBlock__: 0x100602d00>
        🔔🔔🔔 E
```
**_Block_object_assign 源码分析之前:**
```
BLOCK_EXPORT void _Block_object_assign(void *, const void *, const int) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_3_2);
```
const int 参数对应一个枚举:

Block_private.h 文件 332 行:
```
// Runtime support functions used by compiler when generating copy/dispose helpers
// 当编译器生成 copy/dispose helpers 时 Runtime 支持的函数
// Values for _Block_object_assign() and _Block_object_dispose() parameters
// 作为 _Block_object_assign() 和 _Block_object_dispose() 函数的参数
enum {
    // see function implementation for a more complete description of these fields and combinations
    // 有关这些字段及其组合的更完整说明，参见函数实现
    
    // OC 对象类型
    BLOCK_FIELD_IS_OBJECT   =  3,  // id, NSObject, __attribute__((NSObject)), block, ...
    // 为另一个 Block
    BLOCK_FIELD_IS_BLOCK    =  7,  // a block variable
    // 为一个被 __block 修饰后生成的结构体
    BLOCK_FIELD_IS_BYREF    =  8,  // the on stack structure holding the __block variable
    // 被 __weak 修饰过的弱引用，只在 Block_byref 管理内部对象内存时使用
    // 也就是 __block __weak id;
    BLOCK_FIELD_IS_WEAK     = 16,  // declared __weak, only used in byref copy helpers
    // 在处理 Block_byref 内部对象内存的时候会加一个额外标记，配合上面的枚举一起使用
    BLOCK_BYREF_CALLER      = 128, // called from __block (byref) copy/dispose support routines.
};

enum {
    // 上述情况的整合，即以上都会包含 copy_dispose 助手
    BLOCK_ALL_COPY_DISPOSE_FLAGS = 
        BLOCK_FIELD_IS_OBJECT | BLOCK_FIELD_IS_BLOCK | BLOCK_FIELD_IS_BYREF |
        BLOCK_FIELD_IS_WEAK | BLOCK_BYREF_CALLER
};
```
源码验证：
```
NSObject *is_object = [[NSObject alloc] init]; // 对象类型
void (^is_block)() = ^{ NSLog(@"is_block 参数"); }; // block 
__block NSObject *is_byref = [[NSObject alloc] init]; // __block 对象
NSObject *tt = [[NSObject alloc] init];
__block __unsafe_unretained NSObject *is_weak = tt; // __weak __block 同时修饰

NSLog(@"⛈⛈⛈ is_byref retainCount = %lu ---%p---%p", (unsigned long)[is_byref arcDebugRetainCount], is_byref, &is_byref); // 堆区 栈区

void (^aBlock)() = ^{
    NSLog(@"⛈⛈⛈ is_object retainCount = %lu ---%p---%p", (unsigned long)[is_object arcDebugRetainCount], is_object, &is_object);
    is_block();
    
    NSLog(@"⛈⛈⛈ is_byref retainCount = %lu ---%p---%p", (unsigned long)[is_byref arcDebugRetainCount], is_byref, &is_byref);
    NSLog(@"⛈⛈⛈ is_weak retainCount = %lu ---%p---%p", (unsigned long)[is_weak arcDebugRetainCount], is_weak, &is_weak);
    NSLog(@"⛈⛈⛈ is_only_weak retainCount = %lu ---%p---%p", (unsigned long)[is_only_weak arcDebugRetainCount], is_only_weak, &is_only_weak);
};

// 部分转换后的代码:

struct __main_block_impl_1 {
  struct __block_impl impl;
  struct __main_block_desc_1* Desc;
  
  // 捕获的变量
  NSObject *is_object;
  struct __block_impl *is_block;
  NSObject *is_only_weak;
  __Block_byref_is_byref_0 *is_byref; // by ref
  __Block_byref_is_weak_1 *is_weak; // by ref
    
  __main_block_impl_1(void *fp, struct __main_block_desc_1 *desc, NSObject *_is_object, void *_is_block, NSObject *_is_only_weak, __Block_byref_is_byref_0 *_is_byref, __Block_byref_is_weak_1 *_is_weak, int flags=0) : is_object(_is_object), is_block((struct __block_impl *)_is_block), is_only_weak(_is_only_weak), is_byref(_is_byref->__forwarding), is_weak(_is_weak->__forwarding) {
    impl.isa = &_NSConcreteStackBlock;
    impl.Flags = flags;
    impl.FuncPtr = fp;
    Desc = desc;
  }
};

// copy
static void __main_block_copy_1(struct __main_block_impl_1*dst, struct __main_block_impl_1*src) {
    
    _Block_object_assign((void*)&dst->is_object, (void*)src->is_object, 3/*BLOCK_FIELD_IS_OBJECT*/);
    _Block_object_assign((void*)&dst->is_block, (void*)src->is_block, 7/*BLOCK_FIELD_IS_BLOCK*/);
    _Block_object_assign((void*)&dst->is_byref, (void*)src->is_byref, 8/*BLOCK_FIELD_IS_BYREF*/);
    _Block_object_assign((void*)&dst->is_weak, (void*)src->is_weak, 8/*BLOCK_FIELD_IS_BYREF*/);
    _Block_object_assign((void*)&dst->is_only_weak, (void*)src->is_only_weak, 3/*BLOCK_FIELD_IS_OBJECT*/);
    
}

// dispose
static void __main_block_dispose_1(struct __main_block_impl_1*src) {
    
    _Block_object_dispose((void*)src->is_object, 3/*BLOCK_FIELD_IS_OBJECT*/);
    _Block_object_dispose((void*)src->is_block, 7/*BLOCK_FIELD_IS_BLOCK*/);
    _Block_object_dispose((void*)src->is_byref, 8/*BLOCK_FIELD_IS_BYREF*/);
    _Block_object_dispose((void*)src->is_weak, 8/*BLOCK_FIELD_IS_BYREF*/);
    _Block_object_dispose((void*)src->is_only_weak, 3/*BLOCK_FIELD_IS_OBJECT*/);
    
}

```

**这里针对 __block 变量解释一下：**
+ __block NSObject *object = [[NSObject alloc] init]; 
+ __Block_byref_object_0 结构体
+ 首先 NSObject 对象是处于堆区的，__block 结构体实例是处于栈区的。
+ Block 发生 copy 操作从栈区到堆区时：原始的 NSObject 对象是不动的，是 __block 结构体实例被复制到了堆区。
+ 且复制以后，原始栈区的 __block 结构体实例会断开对 NSObject 对象的引用
+ 堆区的 __block 结构体实例持有 NSObject 对象实例，NSObject 对象实例的引用计数此时还是 1
```
__block NSObject *object = [[NSObject alloc] init];
NSLog(@"⛈⛈⛈ object retainCount = %lu ---%p---%p", (unsigned long)[object arcDebugRetainCount], object, &object); // 堆区 栈区

void (^aBlock)() = ^{
    NSLog(@"⛈⛈⛈ object retainCount = %lu ---%p---%p", (unsigned long)[object arcDebugRetainCount], object, &object);
};

aBlock(); // 堆区 堆区
void (^bBlock)() = [aBlock copy];
bBlock(); // 堆区 堆区
NSObject *temp = object;
bBlock(); // 堆区 堆区
aBlock(); // 堆区 堆区
NSLog(@"⛈⛈⛈ object retainCount = %lu ---%p---%p", (unsigned long)[object arcDebugRetainCount], object, &object); // 堆区 堆区

// 打印：
⛈⛈⛈ object retainCount = 1 ---0x100738890---0x7ffeefbff578
⛈⛈⛈ object retainCount = 1 ---0x100738890---0x10073a628
⛈⛈⛈ object retainCount = 1 ---0x100738890---0x10073a628
⛈⛈⛈ object retainCount = 2 ---0x100738890---0x10073a628
⛈⛈⛈ object retainCount = 2 ---0x100738890---0x10073a628
⛈⛈⛈ object retainCount = 2 ---0x100738890---0x10073a628
```

**block 捕获的对象类型变量，在block 结构体中有个对应的对象类型指针，一直指向该对象类型的实例。**
**__block 结构体实例的对象类型的成员变量作为一个指针，一直指向该对象的实例。**

**堆区地址小于栈区地址，基本类型存在栈区**

// 这里

```
// 示例 1：
NSObject *obj = [[NSObject alloc] init];
__block NSObject *object = obj;
NSLog(@"⛈⛈⛈ obj retainCount = %lu", (unsigned long)[obj arcDebugRetainCount]);
// 打印：
⛈⛈⛈ obj retainCount = 2 // 被 obj 和 object 持有

// 示例 2:
__block NSObject *object = [[NSObject alloc] init];
NSLog(@"⛈⛈⛈ object retainCount = %lu", (unsigned long)[object arcDebugRetainCount]);
// 打印：
⛈⛈⛈ object retainCount = 1 // 只被 object 持有

// 示例 3:
__block NSObject *object = [[NSObject alloc] init];
^{
    NSLog(@"%@", object);
};

NSLog(@"⛈⛈⛈ object retainCount = %lu", (unsigned long)[object arcDebugRetainCount]);
// 打印：
⛈⛈⛈ object retainCount = 1 // 只被 object 持有，栈区的 block 持有 object 结构体 

// 示例 4：对比上面不用 __block 修饰，引用为 2，一次被变量 object 持有，一次被 block 持有
// 接下来为了区分变量在栈区还是堆区，打印它们的地址方便比较
NSObject *object = [[NSObject alloc] init];
NSLog(@"⛈⛈⛈ object retainCount = %lu ---%p---%p", (unsigned long)[object arcDebugRetainCount], object, &object);
^{
    NSLog(@"⛈⛈⛈ object retainCount = %lu ---%p---%p", (unsigned long)[object arcDebugRetainCount], object, &object);
}();

NSLog(@"⛈⛈⛈ object retainCount = %lu ---%p---%p", (unsigned long)[object arcDebugRetainCount], object, &object);
// 打印：这里有个细节，看三次 object 打印地址是相同的，都指向原始的 NSObject 对象，这没有问题，
// 第二行 block 内部的打印 &object 地址不同与上下两次，因为这个 object 是 block 结构体的 object 成员变量（类型是个指针）的地址
// 上下两次还是栈中的 object 变量
⛈⛈⛈ object retainCount = 1 ---0x102800750---0x7ffeefbff578
⛈⛈⛈ object retainCount = 2 ---0x102800750---0x7ffeefbff560
⛈⛈⛈ object retainCount = 2 ---0x102800750---0x7ffeefbff578

// 示例 5：
 __block NSObject *object = [[NSObject alloc] init];
 NSLog(@"⛈⛈⛈ object retainCount = %lu ---%p---%p", (unsigned long)[object arcDebugRetainCount], object, &object);
^{
    NSLog(@"⛈⛈⛈ object retainCount = %lu ---%p---%p", (unsigned long)[object arcDebugRetainCount], object, &object);
}();

NSLog(@"⛈⛈⛈ object retainCount = %lu ---%p---%p", (unsigned long)[object arcDebugRetainCount], object, &object);

// 打印：都是 1 ，只被 __block 变量 object 持有，在栈 Block中，自始只有强指针指向 object，就是__block生成的结构体。
// 三次地址完全相同，这里涉及到 __block 变量对应的结构体中的 __forwarding 指针
// 上下是：
// &(object.__forwarding->object)
// 中间 block 执行是：
&(object->__forwarding->object)
// 至此取的的 object 就是原始的 NSObject 对象
⛈⛈⛈ object retainCount = 1 ---0x102802820---0x7ffeefbff578
⛈⛈⛈ object retainCount = 1 ---0x102802820---0x7ffeefbff578
⛈⛈⛈ object retainCount = 1 ---0x102802820---0x7ffeefbff578

// 示例 6:
 __block NSObject *object = [[NSObject alloc] init];
 NSLog(@"⛈⛈⛈ object retainCount = %lu ---%p---%p", (unsigned long)[object arcDebugRetainCount], object, &object);
void (^aBlock)() = ^{
    NSLog(@"⛈⛈⛈ object retainCount = %lu ---%p---%p", (unsigned long)[object arcDebugRetainCount], object, &object);
};
aBlock();

NSLog(@"⛈⛈⛈ object retainCount = %lu ---%p---%p", (unsigned long)[object arcDebugRetainCount], object, &object);
// 打印：

// 极其重要的一句: "并断开栈中的obj结构体对obj对象的指向" "并断开栈中的obj结构体对obj对象的指向" "并断开栈中的obj结构体对obj对象的指向" 

可以看到，obj的内存地址一直在栈中，而执行了BlockCopy后，obj指针的地址从栈变到了堆中，而obj的引用计数一直是1。在copy操作之后，结构体obj也被复制到了堆中，并断开栈中的obj结构体对obj对象的指向。那如果这个时候取栈中的obj不就有问题了？__forwarding就派上用场了，上面编译的结果发现，结构体对象在使用obj的时候会使用obj->__forwarding->obj，如果所有__forwarding都指向自己，这一步还有什么意义？栈Block在执行copy操作后，栈obj结构体的__forwarding就会指向copy到堆中的obj结构体。此时再取值，操作的就是同一份指针了。证明如下:

// 示例 7：
__block NSObject *object = [[NSObject alloc] init];
NSLog(@"⛈⛈⛈ object retainCount = %lu ---%p---%p", (unsigned long)[object arcDebugRetainCount], object, &object);

void (^aBlock)() = ^{
    NSLog(@"⛈⛈⛈ object retainCount = %lu ---%p---%p", (unsigned long)[object arcDebugRetainCount], object, &object);
};

aBlock();
void (^bBlock)() = [aBlock copy];
bBlock();
aBlock();
NSLog(@"⛈⛈⛈ object retainCount = %lu ---%p---%p", (unsigned long)[object arcDebugRetainCount], object, &object);

// MRC 下打印：
⛈⛈⛈ object retainCount = 1 ---0x10065bd50---0x7ffeefbff570 // 原始状态: 堆区 栈区
⛈⛈⛈ object retainCount = 1 ---0x10065bd50---0x7ffeefbff570 // 堆区 栈区 // 这里虽然发生了赋值操作，但是并没有主动被复制到堆区

⛈⛈⛈ object retainCount = 1 ---0x10065bd50---0x1010083f8 // 堆区 堆区 // 这里开始主动调用了 copy 才被复制到堆区
⛈⛈⛈ object retainCount = 1 ---0x10065bd50---0x1010083f8 // 堆区 堆区
⛈⛈⛈ object retainCount = 1 ---0x10065bd50---0x1010083f8 // 堆区 堆区

// ARC 下打印：
⛈⛈⛈ object retainCount = 1 ---0x1006002e0---0x7ffeefbff578  // 原始状态: // 堆区 栈区
⛈⛈⛈ object retainCount = 1 ---0x1006002e0---0x1007396e8 // 堆区 堆区 // 这里发生了赋值操作，__block 变量被复制到堆区
⛈⛈⛈ object retainCount = 1 ---0x1006002e0---0x1007396e8 // 堆区 堆区
⛈⛈⛈ object retainCount = 1 ---0x1006002e0---0x1007396e8 // 堆区 堆区
⛈⛈⛈ object retainCount = 1 ---0x1006002e0---0x1007396e8 // 堆区 堆区

由上可知，在copy之前，aBlock的打印结果都是初始化生成的指针，而copy之后打印就和bBlock的打印结果相同了。总结一下就是，在栈中的obj结构体__forwarding指向的就是栈中的自己，执行copy之后，会在堆中生成一份obj结构体并断开栈中对obj的引用，此时堆中的obj结构体__forwarding就指向自己，而栈中的__forwarding就指向堆中的obj结构体。下面也会通过分析源码来具体解释。

__block NSObject *object = [[NSObject alloc] init];
NSLog(@"⛈⛈⛈ object retainCount = %lu ---%p---%p", (unsigned long)[object arcDebugRetainCount], object, &object); // 堆区 栈区

void (^aBlock)() = ^{
    NSLog(@"⛈⛈⛈ object retainCount = %lu ---%p---%p", (unsigned long)[object arcDebugRetainCount], object, &object);
};

NSObject *temp = object; // + 1
aBlock(); // 堆区 堆区
void (^bBlock)() = [aBlock copy];
bBlock(); // 堆区 堆区
aBlock(); // 堆区 堆区
NSLog(@"⛈⛈⛈ object retainCount = %lu ---%p---%p", (unsigned long)[object arcDebugRetainCount], object, &object); // 堆区 堆区
// 打印：
⛈⛈⛈ object retainCount = 1 ---0x10053e1b0---0x7ffeefbff578
⛈⛈⛈ object retainCount = 2 ---0x10053e1b0---0x10053e988
⛈⛈⛈ object retainCount = 2 ---0x10053e1b0---0x10053e988
⛈⛈⛈ object retainCount = 2 ---0x10053e1b0---0x10053e988
⛈⛈⛈ object retainCount = 2 ---0x10053e1b0---0x10053e988
```

**堆 Block __NSMallocblock__ 内存由 ARC 控制，没有强指针指向时释放。而在 MRC 中，赋值不会执行 copy 操作，所以左侧 block 依然存在于栈中，所以在 MRC 中一般都需要执行 copy，否则很容易造成 crash.在 ARC 中，当 Block 作为属性被 strong、copy 修饰或被强指针引用或作为返回值时，都会默认执行 copy。而 MRC 中，只有被 copy 修饰时，block 才会执行 copy。所以 MRC 中 Block 都需要用 copy 修饰，而在 ARC 中用 copy 修饰只是沿用了 MRC 的习惯，此时用 copy 和 strong效果是相同的。**

**Block 在捕获外部变量的操作基本一致，都是在生成结构体的时候将所有 Block 里用到的外部变量作为属性保存起来。self.block 里面调用 self 会造成循环引用，因为 Block 捕获了 self 并把 self 当做一个值保存了起来。**

**Block里的a只是copy过去的a的值，在Block里改变a的值也不会影响外面，编译器避免这个错误就报错。**

**同样的，捕获对象时也是对指针的copy，生成一个指针指向obj对象，所以如果在Block中直接让obj指针指向另外一个对象也会报错。这点编译器已经加了注释 // bound by copy。**

**多了__main_block_copy_0 和 __main_block_dispose_0 这两个函数，并在描述 __main_block_desc_0 结构体中保存了这两个函数指针。这就是上面所说的 copy_dispose 助手，C 语言结构体中，编译器很难处理对象的初始化和销毁操作，所以使用 runtime 来管理相关内存。BLOCK_FIELD_IS_OBJECT 是在捕获对象时添加的特别标识，此时是3，下面会细讲。**

**此 Block 是为栈 Block_NSConcreteStackBlock，不过在 ARC 中，因为赋值给 aBlock，会执行一次 copy，将其中栈中 copy 到堆中，所以在 MRC 中 aBlock 为 _NSConcreteStackBlock，在 ARC 中就是 _NSConcreteMallocBlock。**

+++++++++++++++++++++++++++++++++++++


 






