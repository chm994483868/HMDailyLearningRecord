#  iOS底层class_getInstanceSize和malloc_size
示例代码：
```Objective-C
#import <Foundation/Foundation.h>
#import <objc/runtime.h> // class_getInstanceSize 在这里定义
#import <malloc/malloc.h> // malloc_size 在这里定义

#import "HHStaff.h"

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        // insert code here...
//        HHStaff *staff = [[HHStaff alloc] init];
        
        NSObject *staff = [[NSObject alloc] init];
        NSLog(@"🧚‍♂️🧚‍♂️🧚‍♂️ class_getInstanceSize => %zd", class_getInstanceSize([staff class]));
        NSLog(@"🧚‍♂️🧚‍♂️🧚‍♂️ malloc_size => %zd", malloc_size(CFBridgingRetain(staff)));
        NSLog(@"🧚‍♂️🧚‍♂️🧚‍♂️ sizeof => %zd", sizeof(staff)); // 无符号输出使用 %zd 即可
    }
    
    return 0;
}

// HHStaff 类定义（只展示成员变量部分）：
@interface HHStaff : NSObject {
    // Class isa; 补0 偏0 长8 //在 继承的 NSObject 中还有一个 Class isa; 成员变量
    int _age; // 补0 偏8 长12
    int _height; // 补0 偏12 长16
    NSString *_name; // 补0 偏16 长24
}

```
当 staff 是 NSObject 对象时，打印结果:
```
🧚‍♂️🧚‍♂️🧚‍♂️ class_getInstanceSize => 8 // 实际 NSObject 只占用 8 个字节，struct NSObject_IMPL 中只有一个 Class isa；成员变量（struct objc_class * Class），一个结构体指针
🧚‍♂️🧚‍♂️🧚‍♂️ malloc_size => 16 // 系统分配了 16 个字节给 NSObject 对象
🧚‍♂️🧚‍♂️🧚‍♂️ sizeof => 8 // 指针长度，64 位机器 __LP64__，Pointer 占 8 个字节
```
当 staff 是 HHStaff 对象时，打印结果:
```
🧚‍♂️🧚‍♂️🧚‍♂️ class_getInstanceSize => 24 // 根据内存对齐原则，可计算得出 HHStaff_IMPL 结构体的大小是 24.
🧚‍♂️🧚‍♂️🧚‍♂️ malloc_size => 32 // 实际系统为 HHStaff 对象分配了 32 个字节，（内存对齐取 16 的倍数）
🧚‍♂️🧚‍♂️🧚‍♂️ sizeof => 8 // 指针长度
```

### `class_getInstanceSize` 函数，返回一个类的底层结构体所占用内存空间的大小
```
// 返回类实例的大小，即类对象的大小，不要被名字迷惑了，不是实例对象，是类对象。
// 根据这个类的底层结构体按内存对齐原则计算出来的结构体大小
/** 
 * Returns the size of instances of a class.
 * 
 * @param cls A class object.
 * 
 * @return The size in bytes of instances of the class \e cls, or \c 0 if \e cls is \c Nil.
 */
OBJC_EXPORT size_t
class_getInstanceSize(Class _Nullable cls) 
    OBJC_AVAILABLE(10.5, 2.0, 9.0, 1.0, 2.0);
```
### `malloc_size` 返回入参指针所指向内存空间的大小
```
extern size_t malloc_size(const void *ptr);
/* Returns size of given ptr */
```
### `sizeof` 返回 入参指针大小
```
sizeof(staff)
```

### 什么是 `size_t` ？
> size_t 类型定义在cstddef头文件中，该文件是C标准库的头文件stddef.h的C++版。
> 
> 它是一个与机器相关的unsigned（无符号）类型，其大小足以保证存储内存中对象的大小。
> 
> 例如：我们常用的sizeof()操作的返回值就是 size_t类型的
> 
> 例如：在用下标访问元素时，vector使用vector::size_type作为下标类型，而数组下标的正确类型则是size_t。
> 
> vector使用的下标实际也是size_t，源码是typedef size_t size_type。
> 
> 因为size_t类型的数据其实是保存了一个整数，所以它也可以做加减乘除，也可以转化为int并赋值给int类型的变量。
> 
> 为什么要有size_t类型？
> 
> 在C++中，设计 size_t 就是为了适应多个平台的 。size_t的引入增强了程序在不同平台上的可移植性。size_t是针对系统定制的一种数据类型，一般是整型，因为C/C++标准只定义最低的位数，而不是必需的固定位数。而且在内存里，对数的高位对齐存储还是低位对齐存储各系统都不一样。为了提高代码的可移植性，就有必要定义这样的数据类型。一般这种类型都会定义到它具体占几位内存等。
> 
> 例如：在32位系统中size_t是4字节的，而在64位系统中，size_t是8字节的，这样利用该类型可以增强程序的可移植性。

通过以上分析，可以得出一个结论：
**系统分配了 16 个字节给 NSObject 对象（通过 malloc_size() 函数获得），但是 NSObject 内部只使用了 8 个字节的空间（64 位环境下，可以通过 class_getInstanceSize() 函数获得，如果使用 sizeof() 的话获取的是 staff 指针的长度）。**

## 解析 `class_getInstanceSize()` 源码

```
size_t class_getInstanceSize(Class cls)
{
    if (!cls) return 0;
    return cls->alignedInstanceSize();
}
```
`alignedInstanceSize` 函数:
```
// 返回一个类的 ivar（成员变量）所占空间的大小
// Class's ivar size rounded up to a pointer-size boundary.
uint32_t alignedInstanceSize() const {
    return word_align(unalignedInstanceSize());
}
```
`unalignedInstanceSize()`
```
// May be unaligned depending on class's ivars.
uint32_t unalignedInstanceSize() const {
    ASSERT(isRealized());
    return data()->ro()->instanceSize;
}
```
`word_align()` 
```
static inline uint32_t word_align(uint32_t x) {
    return (x + WORD_MASK) & ~WORD_MASK;
}
static inline size_t word_align(size_t x) {
    return (x + WORD_MASK) & ~WORD_MASK;
}
```
在当前的平台下 `#   define WORD_MASK 7UL`
```
#ifdef __LP64__
#   define WORD_SHIFT 3UL
#   define WORD_MASK 7UL
#   define WORD_BITS 64
#else
#   define WORD_SHIFT 2UL
#   define WORD_MASK 3UL
#   define WORD_BITS 32
#endif
```
`return (x + WORD_MASK) & ~WORD_MASK;` 即 `return (x + Ox7) & 0x0` 
`return (x + 0b0000 0111) & 0b1111 1111 1111 1111 1111 .... 1111 1000` 即 任意 x 进来都是把 8 的次方 
x = 1 => 8
x = 2 => 8
...
x = 8 => 8
x = 9 => 16
x = 10 => 16
...
x = 13 => 16

即该操作是对 8 对齐，保证返回长度是 8 的倍数。
这里可以理解为 OC 内部求类对象的结构体长度时是遵循 8 字节对齐的。

## 通过 lldb 命令来输出一个对象的内存表现情况：

```
(lldb) print staff
(NSObject *) $0 = 0x000000010053e240
(lldb) p staff
(NSObject *) $1 = 0x000000010053e240
(lldb) po staff
<NSObject: 0x10053e240>

(lldb) memory read 0x10053e240
0x10053e240: 19 01 09 99 ff ff 1d 00 00 00 00 00 00 00 00 00  ................
0x10053e250: 2d 5b 4e 53 50 61 74 68 43 6f 6d 70 6f 6e 65 6e  -[NSPathComponen
```
从 `memory read 0x10053e240` 输出结果可以看出 `19 01 09 99 ff ff 1d 00 00 00 00 00 00 00 00 00` 共 16 个字节的长度，其中 [19 01 09 99 ff ff 1d] 是有值得说明已经有数据存在。

**参考链接:**
[iOS 底层 - class_getInstanceSize、malloc_size](https://www.jianshu.com/p/2d8b51c21430)
