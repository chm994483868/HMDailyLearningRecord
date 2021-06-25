# iOS 从 libclosure-74 源码来研究 Block 实现原理(1)

# Blocks

> 本篇主要介绍 `OS X Snow Leopard(10.6)` 和 `iOS 4` 引入的 `C` 语言扩充功能 `Blocks`，究竟是如何扩充 `C` 语言的，扩充之后又有哪些优点呢？下面通过其实现来一步一步探究。

## 什么是 Blocks 
&emsp;`Blocks` 是 `C` 语言的扩充功能。可以用一句话来表示 `Blocks` 的扩充功能：带有自动变量（局部变量）的匿名函数。(对于程序员而言，命名就是工作的本质。)
何谓匿名，下面的示例代码可解释：
```c++
typedef void(^Blk_T)(int);
int i = 10;

// 注意等号右边的 block 定义(匿名)
Blk_T a = ^void (int event) {
    printf("i = %d event = %d\n", i, event);
};

// 函数定义
void Func(int event) {
    printf("buttonID: %d event = %d\n", i, event);
};
void (*funcPtr)(int) = &Func;
```
&emsp;匿名是针对有名而言的，如上代码 `Blk_T a` 等号后面的 `block` 定义是不需要取名的，而下面的 `Func` 函数定义必须给它一个函数名。

完整形式的 `block` 语法与一般的 `C` 语言函数定义相比，仅有两点不同：
1. 没有函数名。
2. 带有 `^`。

`Block` 定义范式如下:
**^ 返回值类型 参数列表 表达式**
“返回值类型” 同 `C` 语言函数的返回值类型，“参数列表” 同 `C` 语言函数的参数列表，“表达式” 同 `C` 语言函数中允许使用的表达式。

&emsp;在 `Block` 语法下，可将 `block` 语法赋值给声明为 `Block` 类型的变量中。即源代码中一旦使用 `Block` 语法就相当于生成了可赋值给 `Block` 类型变量的 “值”。`Blocks` 中由 `Block` 语法生成的值也称为 `block`。`block` 既指源代码中的 `Block` 语法，也指由 `Block` 语法所生成的值。
使用 `Block` 语法将 `block` 赋值为 `Block` 类型变量：
```c++
int (^blk)(int) = ^(int count) { return count + 1; };

// 返回值是 Block 类型的函数
// func() 和 { } 之外是描述返回类型的
void (^ func() )(int) {
    return ^(int count) { return count + 1; }; 
}
```
`Block` 类型变量可完全像通常的 `C` 语言变量一样使用，因此也可以使用指向 `Block` 类型变量的指针，即 `block` 的指针类型变量。
```c++
typedef int (^blk_t)(int);
blk_t blk = ^(int count) { return count + 1; };

// 指针赋值
blk_t* blkPtr = &blk;

// 执行 block
(*blkPrt)(10);
```
`Block` 类型变量与一般的 `C` 语言变量完全相同，可作为以下用途使用：
+ 自动变量
+ 函数参数
+ 静态变量
+ 静态全局变量
+ 全局变量

通过 `Block` 类型变量调用 `block` 与 `C` 语言通常的函数调用没有区别。

## 截获外部变量值
&emsp;`block` 是带有自动变量（局部变量）的匿名函数，**带有自动变量** 在 `block` 中表现为 **截获外部变量值**。
```c++
// 示例 🌰 1：
int val = 10;
const char* fmt = "val = %d\n";
void (^blk)(void) = ^{
    // block 内截获的是 10 和 fmt 指针指向的地址
    printf(fmt, val);
};

// blk 只是截获了 val 的瞬间值(10)去初始化 block 结构体的 val 成员变量，
// val 的值无论再怎么改写，都与 block 结构体内的值再无瓜葛
val = 2;

// 修改了 fmt 指针的指向，blk 对应 block 结构体只是截获了 fmt 指针原始指向的 char 字符串，
// 所以 blk 内打印使用的还是 "val = %d\n"
fmt = "These values were changed. val = %d\n";

blk();

// 打印结果：
// val = 10

// 示例 🌰 2：
int temp = 10;
int* val = &temp;
const char* fmt = "val = %d\n";
void (^blk)(void) = ^{
    // block 内截获的是 fmt 指针指向的地址以及 val 指针指向的地址
    printf(fmt, *val);
};

// 直接改写 val 指向的地址内的值，blk 内截获的是 val 指向的地址
*val = 20; 
fmt = "These values were changed. val = %d\n";

blk();
// 打印结果：
// val = 20

// 示例 🌰 3：
int temp = 10;
int* val = &temp;
const char* fmt = "val = %d\n";
void (^blk)(void) = ^{
    // 这里可以直接通过指针修改 val 的值
    *val = 22;
    printf(fmt, *val);
};

// 直接改写 val 指向的地址内的值，blk 截获的 val 指向的地址
*val = 20; 

fmt = "These values were changed. val = %d\n";

blk();
// 打印结果：
// val = 22

// 示例 🌰 4：
__block int val = 10;
const char* fmt = "val = %d\n";
void (^blk)(void) = ^{
    printf(fmt, val);
};

// val 用 __block 修饰后，类型已经不是 int，它已转变为结构体类型，具体细节会在下面展开
// val 已转换为一个结构体实例
val = 2;
fmt = "These values were changed. val = %d\n";

blk();
// 打印结果：
// val = 2

// 示例 🌰 5:
// 从上到下操作的一直都是 a 变量地址空间里的内容
int a = 10;
__block int* b = &a;
void (^blk)(void) = ^{
    NSLog(@"⛈⛈⛈ block 内部 b 修改前: b = %d", *b);
    *b = 20; // 修改后外部 b 也是 20
};

NSLog(@"⛈⛈⛈ b = %d", *b);
a = 30;
NSLog(@"⛈⛈⛈ b = %d", *b);

blk();

NSLog(@"⛈⛈⛈ b = %d", *b);
NSLog(@"⛈⛈⛈ a = %d", a);
// 打印结果:
⛈⛈⛈ b = 10
⛈⛈⛈ b = 30
⛈⛈⛈ block 内部 b 修改前: b = 30
⛈⛈⛈ b = 20
⛈⛈⛈ a = 20
```

> **无论 `block` 定义在哪，啥时候执行。当 `block` 执行时，用的值都是它定义时截获的基本变量值或者是截获的内存地址，如果是内存地址的话，从定义到执行这段时间，不管里面保存的值有没有被修改， `block` 执行时，使用的都是当时内存里保存的值。（定义可理解为生成 `block` 结构体实例，截获可理解为拿外部变量初始化 `block` 结构体实例的成员变量）** 

## block 的实质
&emsp;`block` 是 “带有自动变量的匿名函数”，但 `block` 究竟是什么呢？语法看上去很特别，但它实际上是作为 **极普通的 `C` 语言源码** 来处理的。通过 **支持 `block` 的编译器**，含有 `block` 语法的源代码转换为一般 `C` 语言编译器能够处理的源代码，并作为极为普通的 `C` 语言源代码被编译。
&emsp;这不过是概念上的问题，在实际编译时无法转换成我们能够理解的源代码，但 `clang(LLVM 编译器)` 具有转换为我们可读源代码的功能。通过 `-rewrite-objc` 选项就能将含有 `block` 语法的源代码转换为 `C++` 的源代码。说是 `C++`，其实也 **仅仅是使用了 `struct` 结构，其本质是 `C` 语言源代码**。

`clang -rewrite-objc 源代码文件名`，如下源代码通过 `clang` 可变换为: 

```c++
int main() {
    void (^blk)(void) = ^{ printf("Block\n"); };
    blk();

    return 0;
}
```
+ **__block_impl**
 ```c++
 struct __block_impl {
   void *isa;
   int Flags;
   int Reserved;
   void *FuncPtr;
 };
 ```
 + **__main_block_impl_0**
 ```c++
 struct __main_block_impl_0 {
   struct __block_impl impl;
   struct __main_block_desc_0* Desc;
   
   // 结构体构造函数 
   __main_block_impl_0(void *fp, struct __main_block_desc_0 *desc, int flags=0) {
     impl.isa = &_NSConcreteStackBlock;
     impl.Flags = flags;
     impl.FuncPtr = fp;
     Desc = desc;
   }
 };
 ```
 + **__main_block_func_0**
 ```c++
 static void __main_block_func_0(struct __main_block_impl_0 *__cself) {
     printf("Block\n");
 }
 ```
 + **__main_block_desc_0**
 ```c++
 static struct __main_block_desc_0 {
   size_t reserved;
   size_t Block_size;
 } __main_block_desc_0_DATA = { 0, sizeof(struct __main_block_impl_0)};
 ```
+ **main 函数内部**
```c++
int main(int argc, const char * argv[]) {
    /* @autoreleasepool */ { __AtAutoreleasePool __autoreleasepool; 
    NSLog((NSString *)&__NSConstantStringImpl__var_folders_24_5w9yv8jx63bgfg69gvgclmm40000gn_T_main_948e6f_mi_0);
        
    // 首先是等号左边是一个返回值和参数都是 void 的函数指针: void (*blk)(void)，
    // 等号右边去掉 &(取地址符) 前面的强制类型转换后，可看到后面是创建了一个，
    // __main_block_impl_0 结构体实例，所以此处可以理解为在栈上创建了一个 Block 结构体实例，
    // 并把它的地址转化为了一个函数指针。
    void (*blk)(void) = ((void (*)())&__main_block_impl_0((void *)__main_block_func_0,
                                                          &__main_block_desc_0_DATA));
    
    // 取出 __block_impl 里面的 FuncPtr 函数执行。
    // __main_block_func_0 函数的参数是类型是 struct __main_block_impl_0 指针，
    // 但是这里用了 __block_impl * 做强制类型转换，
    // 是因为 struct __main_block_impl_0 的第一个成员变量是 struct __block_impl impl，
    // 地址起始空间的类型是一致的（本例暂时没有用到 __cself）
    ((void (*)(__block_impl *))((__block_impl *)blk)->FuncPtr)((__block_impl *)blk);
    }

    return 0;
}

// 生成 Block 结构体（struct __main_block_impl_0）去掉转换部分可以理解为：
// 第一个参数是由 Block 语法转换的 C 语言函数指针
// 第二个参数是作为静态全局变量初始化的 __main_block_desc_0 结构体实例指针
struct __main__block_impl_0 tmp = __main_block_impl_0(__main_block_func_0, &__main_block_desc_0_DATA);
struct __main_block_impl_0 *blk = &tmp;

// 该源代码将 __main_block_impl_0 结构体类型的自动变量，
// 即栈上生成的 __main_block_impl_0 结构体实例的指针，赋值 __main_block_impl_0 结构体指针类型的变量 blk。

void (^blk)(void) = ^{printf("Block\n");};

// 将 Block 语法生成的 Block 赋给 Block 类型变量 blk。
// 它等同于将 __main_block_impl_0 结构体实例的指针赋给变量 blk。
// 源代码中的 Block 就是 __main_block_impl_0 结构体类型的自动变量，
// 即栈上生成的 __main_block_impl_0 结构体实例。

// 执行 Block 去掉转换部分可以理解为：
(*blk->impl.FuncPtr)(blk); // 参数 __cself 即是 Block 结构体实例
```
&emsp;分析下上面转换出的相关结构体，`struct __block_impl` 名字中的 `impl` 即 `implementation` 的缩写，换句话说这一部分是 `block` 的实现部分结构体，`void *isa` `C` 语言中 `void *` 为 “不确定类型指针”，`void *` 可以用来声明指针。看到 `isa` 就会联想到 `objc_class` 结构体，因此我们的 `block` 本质上也是一个对象，而且是个类对象，我们知道 **实例对象->类对象->元类** 构成了 `isa` 链中的一条，而这个 `__block_impl` 结构体占据的是中间类对象的位置，实例对象应该是生成的 `block` 变量，个人认为因此这里的 `isa` 指针会指向元类，这里的元类主要是为了说明这个 `block` 的存储区域，`int Flags` 标识符，在实现 `block` 的内部操作时会用到 `int Reserved` 注明今后版本升级所需区域大小，`Reserved` 一般就是填个 `0`。`void *FuncPtr` 函数指针 **实际执行的函数，也就是 `block` 定义中花括号里面的代码内容，最后是转化成一个 `C` 语言函数执行的**。

&emsp;如变换后的源代码所示，通过 `block` 使用的匿名函数实际上 **被作为简单的 C 语言函数来处理**( `__main_block_func_0` 函数)。另外，**根据 `block` 语法所属的函数名（此处为 `main`）和该 `block` 语法在该函数出现的顺序值（此处为 `0`）来给经 `clang` 变换的函数命名**。该函数的参数 `__cself` 是一个指向 `block` 结构体实例的指针，相当于 `C++` 实例方法中指向实例自身的变量 `this`，或是 `Objective-c` 实例方法中指向对象自身的变量 `self`。

> `static void __main_block_func_0(struct __main_block_impl_0* __cself)` 与 `C++` 的 `this` 和 `OC` 的 `self` 相同，参数 `__cself` 是 `__main_block_impl_0` 结构体的指针。

> `isa = &_NSConcreteStackBlock` 表示 `block` 实例所属的类型，为了理解它，首先要理解 `OC` 类和对象的实质，所谓 `block` 可直接理解为 `OC` 对象。

```c++
// 如果把 __main_block_impl_0 展开的话，
// 把 struct __block_impl impl 的成员变量直接展开，
// 已经几乎和 OC 对象相同
struct __main_block_impl_0 {
void* isa; // isa 是类的实例对象指向所属类的指针
int Flags; // 后面可以理解为类定义中添加的成员变量
int Reserved;
void* FuncPtr;

struct __main_block_desc_0* Desc;
};

// OC 中的实例对象和类对象所使用的数据结构 struct objc_object 和 struct objc_class
typedef struct objc_object* id;
typedef struct objc_class* Class;

// objc_object 结构体和 objc_class 结构体归根结底是在各个对象和类的实现中使用的最基本的数据结构。
```

## `block` 截获外部变量值的实质

&emsp;上一节为了观察 `block` 的最原始的形态在 `block` 中没有截获任何变量，下面我们看一下 `block` 截获外部变量时的转换结果，通过 `clang -rewrite-objc` 转换如下 `block` 定义：
```c++
int dmy = 256; // 此变量是为了对比，未使用的变量不会被 block 截获
int val = 10;
int* valPtr = &val;
const char* fmt = "val = %d\n";

void (^blk)(void) = ^{
    // block 截获了三个变量，类型分别是: int、int *、const char *
    printf(fmt, val);
    printf("valPtr = %d\n", *valPtr);
};

// val 修改为 2，valPtr 指针也跟着指为 2（直接修改了 valPtr 所指的内存区域内的数据）， 
// blk 内部调用是 *valPtr 也是 2，
// Block 的结构体实例中的 valPtr 成员变量，初始值传入的就是外部的 valPtr 指针，
// 所以它们两者指向的内存地址是一样的。

val = 2;
fmt = "These values were changed. val = %d\n";

blk();

// 打印结果:
val = 10
valPtr = 2
```
转换后的代码:
`__block_impl` 结构体保持不变：

```c++
struct __block_impl {
  void *isa;
  int Flags;
  int Reserved;
  void *FuncPtr;
};
```
&emsp;`__main_block_impl_0` 成员变量增加了，`block` 语法表达式中使用的外部变量（看似是同一个变量，其实只是同名）被作为成员变量追加到了 `__main_block_impl_0` 结构体中，且类型与外部变量完全相同。`__main_block_impl_0` 构造函数具体内容就是对 `impl` 中相应的内容进行赋值，要说明的是 `impl.isa = &_NSConcreteStackBlock` 这个是指 `block` 的存储域 和 当前 `block` 的元类，被 `block` 截获的外部变量值被放入到该结构体的成员变量中，构造函数也发生了变化，初始化列表内要给 `fmt`、`val`、`valPtr` 赋值，这里我们就能大概猜出截获外部变量的原理了，被使用的外部变量值会被存入 `block` 结构体中，而在 `block` 表达式中看似是使用外部变量其实是使用了一个名字一模一样的 `block` 结构体实例的成员变量，所以我们不能对它进行直接赋值操作。
```c++
struct __main_block_impl_0 {
  struct __block_impl impl;
  struct __main_block_desc_0* Desc;
  
  // Block 截获三个外部变量，然后 __main_block_impl_0 增加了自己对应的成员变量，
  // 且和外部的自动变量的类型是完全一致的，
  //（这里要加深记忆，后面学习 __block 变量被转化为结构体时可与其进行比较）
  const char *fmt;
  int val;
  int *valPtr;
  
  // 初始化列表里面 : fmt(_fmt), val(_val), valPtr(_valPtr)
  // 构造结构体实例时会用截获的外部变量的值进行初始化，看到参数类型也与外部变量完全相同
  __main_block_impl_0(void *fp,
                      struct __main_block_desc_0 *desc,
                      const char *_fmt,
                      int _val,
                      int *_valPtr,
                      int flags=0) : fmt(_fmt), val(_val), valPtr(_valPtr) {
    impl.isa = &_NSConcreteStackBlock;
    impl.Flags = flags;
    impl.FuncPtr = fp;
    Desc = desc;
  }
};
```
`__main_block_func_0` 函数内也使用到了 `__cself` 参数：
```c++
static void __main_block_func_0(struct __main_block_impl_0 *__cself) {

    // 可以看到通过函数传入 __main_block_impl_0 实例读取对应的截获的外部变量的值 
    const char *fmt = __cself->fmt; // bound by copy
    int val = __cself->val; // bound by copy
    int *valPtr = __cself->valPtr; // bound by copy

    printf(fmt, val);
    printf("valPtr = %d\n", *valPtr);
}
```
`__main_block_desc_0` 保持不变：
```c++
static struct __main_block_desc_0 {
  size_t reserved;
  size_t Block_size;
} __main_block_desc_0_DATA = { 0, sizeof(struct __main_block_impl_0)};
```
`main` 函数里面，`__main_block_impl_0` 结构体实例构建和 `__main_block_func_0` 函数执行保持不变：
```c++
int main(int argc, const char * argv[]) {
    /* @autoreleasepool */ { __AtAutoreleasePool __autoreleasepool; 

        NSLog((NSString *)&__NSConstantStringImpl__var_folders_24_5w9yv8jx63bgfg69gvgclmm40000gn_T_main_4ea116_mi_0);

        int dmy = 256;
        int val = 10;
        int* valPtr = &val;
        const char* fmt = "val = %d\n";
        
        // 根据传递给构造函数的参数对 struct __main_block_impl_0 中由自动变量追加的成员变量进行初始化
        void (*blk)(void) = ((void (*)())&__main_block_impl_0((void *)__main_block_func_0,
                                                              &__main_block_desc_0_DATA,
                                                              fmt,
                                                              val,
                                                              valPtr));

        val = 2;
        fmt = "These values were changed. val = %d\n";
        
        // 执行 __block_impl 中的 FuncPtr 函数，入参正是 __main_block_impl_0 实例变量 blk
        ((void (*)(__block_impl *))((__block_impl *)blk)->FuncPtr)((__block_impl *)blk);
    }

    return 0;
}
```
&emsp;**总的来说，所谓 “截获外部变量值” 意味着在执行 `block` 语法时，`block` 语法表达式使用的与外部变量同名的变量其实是 `block` 的结构体实例（即 `block` 自身）的成员变量，而这些成员变量的初始化值则来自于截获的外部变量的值。** 这里前面提到的 `block` 不能直接使用 `C` 语言数组类型的自动变量，如前所述，截获外部变量时，将值传递给结构体的构造函数进行保存，如果传入的是 `C` 数组，假设是 `a[10]`，那构造函数内部发生的赋值是 `int b[10] = a` 这是 `C` 语言规范所不允许的，`block` 是完全遵循 `C` 语言规范的。

### `__block` 说明符
&emsp;`block` 截获外部变量值，截获的是 `block` 语法定义时此外部变量瞬间的值，保存后就不能改写该值。这个不能改写该值是 `block` 的语法规定，如果截获的是指针变量的话，可以通过指针来修改内存空间里面的值。比如传入 `NSMutableArray` 变量，可以往里面添加对象，但是不能对该 `NSMutableArray` 变量进行赋值。传入 `int* val` 也可以直接用 `*val = 20` 来修改 `val` 指针指向的内存里面保存的值，并且如果截获的是指针变量的话，在 `block` 内部修改其指向内存里面的内容后，在 `block` 外部读取该指针指向的值时也与 `block` 内部的修改都是同步的。**因为本身它们操作的就是同一块内存地址**。

&emsp;这里之所以语法定为不能修改，可能的原因是因为修改了值以后是无法传出去的，只是在 `block` 内部使用，是没有意义的。就比如 `block` 定义里面截获了变量 `val`，你看着这时用的是 `val` 这个变量，其实只是把 `val` 变量的值赋值给了 `block` 结构体的 `val` 成员变量。这时在 `block` 内部修改 `val` 的值，可以理解为只是修改 `block` 结构体 `val` 成员变量的值，与 `block` 外部的 `val` 已经完全无瓜葛了，然后截获指针变量也是一样的，其实截获的只是指针变量所指向的地址，在 `block` 内部修改的只是 `block` 结构体成员变量的指向，这种修改针对外部变量而言都是毫无瓜葛的。
```c++
// 示例 🌰：
int dmy = 256;
int temp = 10;
int* val = &temp;

printf("🎉🎉 val 初始值：= %d\n", *val);

const char* fmt = "🎉 Block 内部：val = %d\n";
void (^blk)(void) = ^{
    printf(fmt, *val);
    int temp2 = 30;
    // !!!!!!!!! 这里报错 
    // Variable is not assignable (missing __block type specifier)
    val = &temp2;
    *val = 22;
};

*val = 20; // 修改 val
fmt = "These values were changed. val = %d\n";

blk();

printf("🎉🎉 val = %d\n", *val); // block 执行时把 *val 修改为 22
// 运行结果：
// 🎉🎉 val 初始值：= 10
// 🎉 Block 内部：val = 20
// 🎉🎉 val = 22
```
以上不能修改（或者理解为为其赋值）时，可以用 `__block` 说明符来修饰该变量，该变量称为 `__block` 变量。
> 注意：在 `block` 内部不能使用 `C` 语言数组，这是因为现在的 `block` 截获外部变量的方法并没有实现对 `C` 语言数组的截获，实质是因为 `C` 语言规定，数组不能直接赋值，可用 `char*` 代替。
```c++
const char text[] = "Hello"; 
void (^blk)(void) = ^{ 
  // Cannot refer to declaration with an array type inside block 
  // 这是因为现在的 Blocks 截获外部变量的方法并没有实现对 C 语言数组的截获。
  // 实质是因为 C 语言规定，数组不能直接赋值，可用 char* 代替
  printf("%c\n", text[0]);
}; 
```
回顾前面截获外部变量值的例子：

```c++
// block 定义
^{ printf(fmt, val); };

// 转换后是:
static void __main_block_func_0(struct __main_block_impl_0* __cself) {
    const char* fmt = __cself->fmt;
    int val = __cself->val;

    printf(fmt, val);
}
```
&emsp;看完转换后的源码，`block` 中所使用的被截获外部变量就如 “带有自动变量值的匿名函数” 所说，仅截获外部变量的值。在 `block` 的结构体实例中重写该成员变量也不会改变原先截获的外部变量。当试图在 `block ` 表达式内部改变同名于外部变量的成员变量时，会发生编译错误。因为在实现上不能改写被截获外部变量的值，所以当编译器在编译过程中检出给被截获外部变量赋值的操作时，便产生编译错误。理论上 `block` 内部的成员变量已经和外部变量完全无瓜葛了，理论上 `block` 结构体的成员变量是能修改的，但是这里修改的仅是结构体自己的成员变量，且又和外部完全同名，如果修改了内部成员变量开发者会误以为连带外部变量一起修改了，索性直接发生编译错误更好！（而 `__block` 变量就是为了在 `block` 表达式内修改外部变量而生的）。

在 `block` 表达式中修改外部变量的办法有两种，（这里忽略上面很多例子中出现的直接传递指针来修改变量的值）：
1. `C` 语言中有变量类型允许 `block` 改写值:
 + 静态变量
 + 静态全局变量
 + 全局变量

 &emsp;虽然 `block` 语法的匿名函数部分简单转换为了 `C`  语言函数，但从这个变换的函数中访问 **静态全局变量/全局变量** 并没有任何改变，可直接使用。**但是静态局部变量的情况下，转换后的函数原本就设置在含有 `block` 语法的函数之外，所以无法从变量作用域直接访问静态局部变量。在我们用 `clang -rewrite-objc` 转换的 `C++` 代码中可以清楚的看到静态局部变量定义在 `main` 函数内，而 `static void __main_block_func_0(struct __main_block_impl_0 *__cself){ ... }` 则是完全在外部定义的一个静态函数。**

 **这里的静态变量的访问，作用域之外，应该深入思考下，虽然代码写在了一起，但是转换后并不在同一个作用域内，能跨作用域访问数据只能靠指针了。**

 代码验证:
 ```c++
 int global_val = 1; // 全局变量
 static int static_global_val = 2; // 静态全局变量
 
 int main(int argc, const char * argv[]) {
 @autoreleasepool {
     // insert code here...
     
     // 这里如果静态局部变量是指针类型的话，
     // 那么在 block 结构体中会被转化为指向指针的指针，
     // 例如: NSMutableArray **static_val;
     
     static int static_val = 3; // 静态局部变量
     
     // 这里看似 block 表达式和 static_val 是同一个作用域的，
     // 其实它们两个完全不是同一作用域的
     void (^blk)(void) = ^{
     
        // 直接在 block 内修改两种不同的类型的外部变量
        global_val *= 2;
        static_global_val *= 2;
        
        // 静态变量则是通过指针来修改的
        static_val *= 3;
     };
     
     static_val = 12;
     blk();
                
     // static_val = 111;
     printf("static_val = %d, global_val = %d, static_global_val = %d\n", static_val,
                                                                          global_val,
                                                                          static_global_val);
 }
}
// 打印结果:
// static_val = 36, global_val = 2, static_global_val = 4

// 看到 static_val 是 36， 即 blk 执行前 static_val 修改为了 12
// 然后 blk 执行时 static_val = 12 * 3 => static_val = 36
// block 内部可以修改 static_val 且 static_val 外部的修改也会
// 传递到 blk 内部
 ```
 `clang` 转换后的源代码:
 `__main_block_impl_0` 追加了 `static_val` 指针为成员变量:
 ```c++
 struct __main_block_impl_0 {
   struct __block_impl impl;
   struct __main_block_desc_0* Desc;
   
   // int *，初始化列表传递进来的是 static_val 的指针 
   int *static_val;
   
   __main_block_impl_0(void *fp,
                       struct __main_block_desc_0 *desc,
                       int *_static_val,
                       int flags=0) : static_val(_static_val) {
     impl.isa = &_NSConcreteStackBlock;
     impl.Flags = flags;
     impl.FuncPtr = fp;
     Desc = desc;
   }
 };
 ```
 `__main_block_func_0`： 
 ```c++
 static void __main_block_func_0(struct __main_block_impl_0 *__cself) {
 
    // 从 block 结构体实例中取出 static_val 指针
    int *static_val = __cself->static_val; // bound by copy
    
    global_val *= 2;
    static_global_val *= 2;
    (*static_val) *= 3;
}
 ```
 `main` 函数：
 ```c++
 int main(int argc, const char * argv[]) {
     /* @autoreleasepool */ { __AtAutoreleasePool __autoreleasepool; 

         NSLog((NSString *)&__NSConstantStringImpl__var_folders_24_5w9yv8jx63bgfg69gvgclmm40000gn_T_main_54420a_mi_0);
         
         // static_val 初始化
         static int static_val = 3;
         
         // 看到 _static_val 入参是 &static_val
         void (*blk)(void) = ((void (*)())&__main_block_impl_0((void *)__main_block_func_0,
                                                               &__main_block_desc_0_DATA,
                                                               &static_val));
         
         // 这里的赋值只是赋值，可以和 __block 的 forwarding 指针方式寻值进行比较思考
         static_val = 12;
         
         ((void (*)(__block_impl *))((__block_impl *)blk)->FuncPtr)((__block_impl *)blk);

         printf("static_val = %d, global_val = %d, static_global_val = %d\n", static_val,
                                                                              global_val,
                                                                              static_global_val);
     }

     return 0;
 }
 ```
&emsp;可看到在 `__main_block_func_0` 内 `global_val` 和 `static_global_val` 的访问和转换前完全相同。静态变量 `static_val` 则是通过指针对其进行访问修改，在 `__main_block_impl_0` 结构体的构造函数的初始化列表中 `&static_val` 赋值给 `struct __main_block_impl_0` 的 `int *static_val` 这个成员变量，这种方式是通过地址在超出变量作用域的地方访问和修改变量。

> 静态变量的这种方法似乎也适用于外部变量的访问，但是为什么没有这么做呢？

&emsp;实际上，在由 `block` 语法生成的值 `block` 上，可以存有超过其变量作用域的被截获对象的外部变量，但是如果 `block` 不持有该变量的话，例如 `bock` 截获的是 `weak` 、`unsafe_unretained` 变量，当变量作用域结束的同时，该自动变量很可能会释放并销毁，而此时再去访问该自动变量的话，如果是 `weak` 变量则已被置为 `nil`，而如果是 `unsafe_unretained` 变量，则会直接因为野指针访问而 `crash`。**而访问静态局部变量则不会出现这种问题，静态变量是存储在静态变量区的，在程序结束前它一直都会存在，之所以会被称为局部，只是说出了作用域无法直接通过变量名访问它了（对比全局变量在整个模块的任何位置都可以直接访问），并不是说这块数据不存在了，只要我们有一个指向该静态变量的指针，那么出了作用域依然能正常访问到它，所以针对外部变量 `block` 并不能采用和静态局部变量一样的处理方式。**

示例代码:
```c++
// block 不持有 __weak object
void (^blk)(void);

{
    NSObject *object = [[NSObject alloc] init];
    NSObject * __weak object2 = object;
  
    // object2 是 object 的弱引用，所以 blk 截获的只是一个弱引用变量,
  	// block 并不会持有该变量，出了下面花括号，object 被释放废弃，object2 也被置为 nil 
    blk = ^{
        NSLog(@"object2 = %@", object2);
    };
}

blk();
//打印：
object2 = (null)

// block 持有 object
void (^blk)(void);
{
    NSObject *object = [[NSObject alloc] init];

    // 出了花括号 object 依然存在，因为它被 blk 持有
    blk = ^{
        NSLog(@"object = %@", object);
    };
}
blk();

// 打印：
object = <NSObject: 0x10059cee0>
```

2. 第二种是使用 `__block` 说明符。更准确的表达方式为 "`__block` 存储域说明符"（`__block storage-class-specifier`）。

`C` 语言中有以下存储域类说明符:

+ `typedef`
+ `extern`
+ `static`
+ `auto`
+ `register`

&emsp;`__block` 说明符类似于 `static`、`auto` 和 `register` 说明符，他们用于指定将变量设置到哪个存储域中。例如: `auto` 表示作为变量存储在栈中，`static` 表示作为静态变量存储在数据区。

**对于使用 `__block` 修饰的变量，不管在 `block` 中有没有使用它，都会相应的给它生成一个结构体实例。**

在前面编译错误的源代码的自动变量声明上追加 `__block` 说明符：
```c++
int main(int argc, const char* argv[]) {
const char* fmt = "val = %d\n";
__block int val = 10;
void (^blk)(void) = ^{
    val = 20; // 此处能正常修改变量
    printf(fmt, val);
};

val = 30;

blk();
return 0;
}
```
&emsp;根据 `clang -rewrite-objc` 转换结果发现，`__block val` 被转化为了 `struct __Block_byref_val_0` （`0` 表示当前是第几个 `__block` 变量）结构体实例。
（`__Block_byref_val_0` 命名规则是 `__Block` 做前缀，然后是 `byref` 表示是被 `__block` 修饰的变量，`val` 表示原始的变量名，`0` 表示当前是第几个 `__block` 变量）

`__Block_byref_val_0`
```c++
struct __Block_byref_val_0 {
  void *__isa;
__Block_byref_val_0 *__forwarding; // 指向自己的指针
 int __flags;
 int __size;
 int val;
};
```
且 `__Block_byref_val_0` 单独拿出来的定义，这样可以在多个 `block` 中重用。

&emsp;如果 `__block` 修饰的是对象类型的话，则 `struct __Block_byref_val_0` 会多两个函数指针类型的成员变量： `__Block_byref_id_object_copy`、`__Block_byref_id_object_dispose`，用于把 `__block` 变量复制到堆区和释放。

`__Block_byref_m_Parray_1`

```c++
struct __Block_byref_m_Parray_1 {
  void *__isa;
__Block_byref_m_Parray_1 *__forwarding;
 int __flags;
 int __size;
 
 void (*__Block_byref_id_object_copy)(void*, void*);
 void (*__Block_byref_id_object_dispose)(void*);
 
 NSMutableArray *m_Parray;
};
```
`__block_impl`，作为一个被复用的结构体，保持不变：
```c++
struct __block_impl {
  void *isa;
  int Flags;
  int Reserved;
  void *FuncPtr;
};
```
`__main_block_impl_0`
```c++
struct __main_block_impl_0 {
  struct __block_impl impl;
  struct __main_block_desc_0* Desc;
  
  // 看到新增了两个成员变量
  // 已知在 block 定义中截获了 fmt 和 val 两个外部变量
  // fmt 和前面的转换没有区别
  const char *fmt;
  
  // val 是一个 __Block_byref_val_0 结构体指针
  __Block_byref_val_0 *val; // by ref
  
  // 首先看到的是 __Block_byref_val_0 * _val 参数，
  // 但是在初始化列表中用的是 val(_val->forwarding 指针)
  // 初始化用的 _val->forwarding
  
  __main_block_impl_0(void *fp,
                      struct __main_block_desc_0 *desc,
                      const char *_fmt,
                      __Block_byref_val_0 *_val,
                      int flags=0) : fmt(_fmt), val(_val->__forwarding) {
    impl.isa = &_NSConcreteStackBlock;
    impl.Flags = flags;
    impl.FuncPtr = fp;
    Desc = desc;
  }
};
```
`__main_block_func_0`
```c++
static void __main_block_func_0(struct __main_block_impl_0 *__cself) {

// 首先从 __main_block_impl_0 结构体实例中取出 val 和 fmt
__Block_byref_val_0 *val = __cself->val; // bound by ref
const char *fmt = __cself->fmt; // bound by copy

// 又看到 val->forwarding-val 
// 先找到 forwarding 然后又取 val 然后给它赋值 20
(val->__forwarding->val) = 20;

// 这里看到实际用 val 截获下来的就是一个 __Block_byref_val_0 结构体实例，
// 对它进行赋值的时候需要通过 forwarding 指针进行，且下面的使用也是通过 forwarding 指针。

printf(fmt, (val->__forwarding->val));

}
```
&emsp;刚刚在 `block` 中向静态变量赋值时只是使用了指向该静态变量的指针，而向 `__block` 变量赋值更复杂，`__main_block_impl_0` 结构体实例持有指向 `__block` 变量的 `__Block_byref_val_0` 结构体实例的指针。`__Block_byref_val_0` 结构体实例的成员变量 `__forwarding` 持有指向该实例自身的指针，通过成员变量 `__forwarding` 访问成员变量 `val`。( 成员变量 `val` 是该实例自身持有的变量，它相当于原外部变量。)

&emsp;继续往下看转换后的 `.cpp` 文件，见到了两个新函数：`__main_block_copy_0` 和 `__main_block_dispose_0`：  （`BLOCK_FIELD_IS_BYREF` 后面会讲） ，目前已发现的有如下情况时会生成下面这一对 `copy` 和 `dispose` 函数：

1. 当 `block` 截获对象类型变量时（如：`NSObject` `NSMutableArray` 对象）会有如下的 `copy` 和 `dispose` 函数生成。

2. 当在 `block` 内部使用 `__block` 变量时（即使是基本型如 `__block int a = 10`）会有如下的 `copy` 和 `dispose` 函数生成。

   当 `block` 表达式内使用外部对象变量和外部 `__block` 变量，以及外部 `block` 时会生成这一对 `copy` 和 `dispose`函数。

`__main_block_copy_0`

```c++
// 内部调用的 _Block_object_assign 函数用的第一个参数: (void*)&dst->val 第二个参数: (void*)src->val
static void __main_block_copy_0(struct __main_block_impl_0*dst,
                                struct __main_block_impl_0*src) {
  _Block_object_assign((void*)&dst->val,
                       (void*)src->val,
                       8/*BLOCK_FIELD_IS_BYREF*/);
}
```
`__main_block_dispose_0`
```c++
// 内部调用的 _Block_object_dispose 函数入参: (void*)src->val
static void __main_block_dispose_0(struct __main_block_impl_0*src) {
  _Block_object_dispose((void*)src->val, 8/*BLOCK_FIELD_IS_BYREF*/);
}
```
&emsp;看到 `__main_block_copy_0` 和 `__main_block_dispose_0` 内部分别调用了 `_Block_object_assign` 和 `_Block_object_dispose` 后面会对该对函数通过源码进行分析，且它们的参数都是使用的 `struct __main_block_impl_0` 的 `val` 成员变量。

这里再延伸一下，当我们使用对象类型的 `__block` 时，例如前面的: `__block NSMutableArray *m_Parray`，它被转换为如下结构体：
`__Block_byref_m_Parray_1`
```c++
struct __Block_byref_m_Parray_1 {
  void *__isa;
__Block_byref_m_Parray_1 *__forwarding;
 int __flags;
 int __size;
 
 void (*__Block_byref_id_object_copy)(void*, void*);
 void (*__Block_byref_id_object_dispose)(void*);
 
 NSMutableArray *m_Parray;
};
```
它内部的 `__Block_byref_id_object_copy` 和 `__Block_byref_id_object_dispose` 在结构体初始化时使用了两个全局函数来初始化: `__Block_byref_id_object_copy_131`、`__Block_byref_id_object_dispose_131`。
```c++
__attribute__((__blocks__(byref))) __Block_byref_m_Parray_1 m_Parray = {(void*)0,
                                                                        (__Block_byref_m_Parray_1 *)&m_Parray,
                                                                        33554432,
                                                                        sizeof(__Block_byref_m_Parray_1),
                                                                        __Block_byref_id_object_copy_131,
                                                                        __Block_byref_id_object_dispose_131,
                                                                        ((NSMutableArray * _Nonnull (*)(id, SEL))(void *)objc_msgSend)((id)objc_getClass("NSMutableArray"), sel_registerName("array"))};
```
在转换后的 `.cpp` 文件中全局搜索 `__Block_byref_id_object_copy_131` 和 `__Block_byref_id_object_dispose_131` 可找到其定义：
```c++
// 内部调用的也是 _Block_object_assign
static void __Block_byref_id_object_copy_131(void *dst, void *src) {
 _Block_object_assign((char*)dst + 40, *(void * *) ((char*)src + 40), 131);
}

// 内部调用的也是 _Block_object_dispose
static void __Block_byref_id_object_dispose_131(void *src) {
 _Block_object_dispose(*(void * *) ((char*)src + 40), 131);
}
```
&emsp;其中硬编码 `40` 对应的是 `struct __Block_byref_m_Parray_1` 前面 `6` 个成员变量，一共 `40` 个字节的宽度，从偏移 `__Block_byref_m_Parray_1` 实例的起始地址偏移 `40` 字节后刚好是 `NSMutableArray *m_Parray` 的位置，且这里不管是 `NSMutableArray` 还是其它对象类型，都是固定的 `40` 个字节，既所有的对象类型的 `__block` 变量都可以通用 `__Block_byref_id_object_copy_131` 和 `__Block_byref_id_object_dispose_131` 函数。

`__main_block_desc_0` 新增了成员变量：
```c++
static struct __main_block_desc_0 {
  size_t reserved;
  size_t Block_size;
  
  // copy 函数指针
  void (*copy)(struct __main_block_impl_0*, struct __main_block_impl_0*);
  // dispose 函数指针
  void (*dispose)(struct __main_block_impl_0*);
  
  // 看到下面的静态全局变量初始化用的是上面两个新增的函数 
} __main_block_desc_0_DATA = { 0,
                               sizeof(struct __main_block_impl_0),
                               __main_block_copy_0,
                               __main_block_dispose_0};
```
`main` 函数内部：
```c++
int main(int argc, const char * argv[]) {
    /* @autoreleasepool */ { __AtAutoreleasePool __autoreleasepool; 

        NSLog((NSString *)&__NSConstantStringImpl__var_folders_24_5w9yv8jx63bgfg69gvgclmm40000gn_T_main_3821a8_mi_0);
        
        // fmt 定义不变
        const char* fmt = "val = %d\n";
        
        // 由 val 创建 __Block_byref_val_0 结构体实例，
        // 成员变量 __isa、__forwarding、__flags、__size、val
        
        // 一手 (void*)0，把 0 转成一个 void* 指针
        // __forwarding 用的是该结构体自己的地址
        // size 就是 sizeof(__Block_byref_val_0)
        // val 的值是就是初始值 10
        
        // __block int val = 10; 一行被转换为如下结构体的初始化
        __attribute__((__blocks__(byref))) __Block_byref_val_0 val = {(void*)0,
                                                                      (__Block_byref_val_0 *)&val,
                                                                      0,
                                                                      sizeof(__Block_byref_val_0),
                                                                      10};
        
        // 如前所示的 __main_block_impl_0 结构体实例
        void (*blk)(void) = ((void (*)())&__main_block_impl_0((void *)__main_block_func_0,
                                                              &__main_block_desc_0_DATA,
                                                              fmt,
                                                              (__Block_byref_val_0 *)&val,
                                                              570425344));
                                                              
        // 此时为 val 赋值，其实操作的都是 __Block_byref_val_0 实例中的 val 成员变量，
        // 而完全不再是可能以为的 int val 
        (val.__forwarding->val) = 30;
        
        // 如前所示 (*blk).impl->FuncPtr 函数执行
        ((void (*)(__block_impl *))((__block_impl *)blk)->FuncPtr)((__block_impl *)blk);
    }

    return 0;
}
```
`__block` 变量转化为 `__Block_byref_val_0` 结构体类型的自动变量，即栈上生成的 `struct __Block_byref_val_0` 结构体实例。

对象类型的 `__block` 变量的情况单独拿出来说一下:
```c++
// __block NSObject *object = [[NSObject alloc] init];

__attribute__((__blocks__(byref))) __Block_byref_object_4 object = {(void*)0,(__Block_byref_object_4 *)&object, 33554432, sizeof(__Block_byref_object_4), __Block_byref_id_object_copy_131, __Block_byref_id_object_dispose_131, ((NSObject *(*)(id, SEL))(void *)objc_msgSend)((id)((NSObject *(*)(id, SEL))(void *)objc_msgSend)((id)objc_getClass("NSObject"), sel_registerName("alloc")), sel_registerName("init"))};

// 简化后
__Block_byref_object_4 object = {
                                 (void*)0, // isa
                                 (__Block_byref_object_4 *)&object, // __forwarding
                                 33554432, // __flags
                                 sizeof(__Block_byref_object_4), // __size
                                 __Block_byref_id_object_copy_131, // __Block_byref_id_object_copy
                                 __Block_byref_id_object_dispose_131, // __Block_byref_id_object_dispose
                                 
                                 ((NSObject *(*)(id, SEL))(void *)objc_msgSend)((id)((NSObject *(*)(id, SEL))(void *)objc_msgSend)
                                 ((id)objc_getClass("NSObject"), sel_registerName("alloc")), sel_registerName("init")) // obj
                                 }
```
&emsp;其中 `__flags = 33554432` 即 `1 << 25`，`BLOCK_HAS_COPY_DISPOSE = (1 << 25)`, 表示 `struct __Block_byref_object_4` 拥有 `copy` 和 `dispose` 函数，基本类型的 `__block` 变量的结构体初始化时 `__flags` 值 `0`。

## `block` 存储域
&emsp;通过前面的学习可知，`block` 转换为 `block` 的结构体实例，`__block` 变量转换为 `__block` 变量结构体实例。

&emsp;**`block` 也可作为 `OC` 对象**。将 `block` 当作 `OC` 对象来看时，该 `block` 的类为 `_NSConcreteStackBlock`、`_NSConcreteGlobalBlock`、`_NSConcreteMallocBlock` 三种类型之一。 由名称中含有 `stack` 可知，该类的对象 `block` 设置在栈上，同样由 `global` 可知，与全局变量一样，设置在程序的数据区域（`.data` 区）中，`malloc` 设置在由 `malloc` 函数分配的内存块（即堆）中。

| 类 | 设置对象的存储域 |
| --- | --- |
| _NSConcreteStackBlock | 栈 |
| _NSConcreteGlobalBlock | 程序的数据区域(.data 区) |
| _NSConcreteMallocBlock | 堆 |

&emsp;**在记述全局变量的地方使用 `block` 语法** 时，生成的 `block` 为 `_NSConcreteGlobalBlock` 类型。
**`block` 具体属于哪种类型，不能通过 `clang` 转换代码看出, `block` 的实际的 `isa` 是在运行时来动态确定的。**

&emsp;如下 `_NSConcreteGlobalBlock` 类型的 `block`：
```c++
void (^blk)(void) = ^{ printf("全局区的 _NSConcreteGlobalBlock Block！\n"); };

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        // insert code here...
        NSLog(@"🎉🎉🎉 Hello, World!");
        
        blk();
        
        NSLog(@"❄️❄️❄️ block isa: %@", blk);
    }
}

// 打印:
全局区的 _NSConcreteGlobalBlock Block！
❄️❄️❄️ block isa: <__NSGlobalBlock__: 0x100002068>
```
&emsp;此 `block` 即该 `block` 结构体实例存储在程序的数据区域中，因为在使用全局变量的地方不能使用自动变量，所以不存在对自动变量进行截获。由此 `block` 用结构体实例的内容不依赖于执行时的状态，所以整个程序中只需要一个实例。因此将 `block` 用结构体实例设置在与全局变量相同的数据区域中即可。

&ensp;只在截获自动变量时，`block` 结构体实例截获的值才会根据执行时的状态变化。
**即使在函数内而不在记述全局变量的地方定义 `block` 表达式，只要 `block` 不截获自动变量，就可以将 `block` 用结构体实例设置在程序的数据区域，即为全局 `block`。**

如下 `block` 定义在 `main` 函数内，但是没有截获外部变量：
```c++
// 当前在 main 函数内，不捕获外部自动变量
void (^globalBlock)(void) = ^{
    NSLog(@"❄️❄️❄️ 测试 block isa");
};

globalBlock();
NSLog(@"❄️❄️❄️ block isa: %@", globalBlock);

// 打印结果:
❄️❄️❄️ 测试 block isa
❄️❄️❄️ block isa: <__NSGlobalBlock__: 0x100002088> // 全局 block
```

**对于没有要截获自动变量的 `block`，我们不需要依赖于其运行时的状态--捕获的变量，这样我们就不涉及到 `block` 的 `copy` 情况，因此是放在数据区。**

**此外要注意的是，通过 `clang` 编译出来的 `isa` 在第二种情况下会显示成 `stackblock`，这是因为 `OC` 是一门动态语言，真正的元类还是在运行的情况下确定的，这种情况下可以使用 `lldb` 调试器查看。**

**虽然通过 `clang` 转换的源代码通常是 `_NSConcreteStackBlock` 类对象，但实现上却有不同。总结如下:**

+ 记述全局变量的地方有 `block` 语法时
+ `block` 语法的表达式中不截获外部自动变量时

以上情况下，`block` 为 `_NSConcreteGlobalBlock` 类型，即 `block` 配置在程序的数据区域中。除此之外 `block` 语法生成的 `block` 为 `_NSConcreteStackBlock` 类，且设置在栈上。

```c++
// 不捕获外部自动变量是 global
void (^globalBlock)(void) = ^{
    NSLog(@"❄️❄️❄️ 测试 block isa");
};

int a = 2;
// ARC 下会被复制到堆区，MRC 下不会进行复制
// 右边栈区 block 赋值给左侧 block 时，会被复制到堆区
void (^mallocBlock)(void) = ^{
    NSLog(@"❄️❄️❄️ 测试 block isa a = %d", a);
};

globalBlock();
mallocBlock();

NSLog(@"❄️❄️❄️ globalBlock isa: %@", globalBlock);
NSLog(@"❄️❄️❄️ mallocBlock isa: %@", mallocBlock);

// 栈区 block
NSLog(@"❄️❄️❄️ stackBlock isa: %@", ^{ NSLog(@"❄️❄️❄️ a = %d", a); });

// 打印：
❄️❄️❄️ 测试 block isa
❄️❄️❄️ 测试 block isa a = 2
❄️❄️❄️ globalBlock isa: <__NSGlobalBlock__: 0x100002088>
❄️❄️❄️ mallocBlock isa: <__NSMallocBlock__: 0x100540fa0>
❄️❄️❄️ stackBlock isa: <__NSStackBlock__: 0x7ffeefbff4e0>
```
&emsp;配置在全局变量区的 `block`，从变量作用域外也可以通过指针安全的使用，但设置在栈上的 `block`，如果其所属的变量作用域结束，该 `block` 就被废弃，由于 `__Block` 变量也配置在栈上，同样的，如果其所属的变量作用域结束，则该 `__block` 变量也会被废弃。

&ensp;`block` 提供了将 `block` 和 `__block` 结构体实例从栈上复制到堆上的方法来解决这个问题。将配置在栈上的 `block` 复制到堆上，这样即使 `block` 语法记述的变量作用域结束，堆上的 `block` 还可以继续存在。

 + 不会有任何一个 `block` 一上来就被存在堆区，请牢记这一点！
 + `_NSConcreteMallocBlock` 存在的意义和 `autorelease` 一样，就是为了能延长 block 的作用域。
 + 我们将 `block` 和 `__blcok` 结构体实例从栈区复制到堆区，这样就算栈上的 `block` 被废弃了，还是可以使用堆上那一个。
 + 可以联想我们在 `ARC` 是如何处理返回值中的 `__strong` 的，大概同理。

 **在这里要思考一个问题：在栈上和堆上同时有一个 `block` 的情况下，我们的赋值，修改，废弃操作应该怎样管理？**

 复制到堆上的 `block` `isa` 会指向 `_NSConcreteMallocBlock`，即 `impl.isa = &_NSConcreteMallocBlock`;

 **栈上的 `__block` 结构体实例成员变量 `__forwarding` 指向堆上 `__block` 结构体实例，堆上的 `__block` 结构体实例成员变量 `__forwarding` 指向它自己，那么不管是从栈上的 `__block` 变量还是从堆上的 `__block` 变量都能够访问同一块 `__block` 实例内容。**

 代码示例：
 ```c++
 // 这个 a 是位于栈区的 __Block_byref_a_0 结构体实例，已经不是 int 类型
 __block int a = 2;

 // 下面 block 被复制到堆区，a 也同时被复制到 堆区
 void (^mallocBlock)(void) = ^{
     // 堆上 a 的 __forwarding 指向自己
     // a->__forwarding->a 自增
     ++a;
     NSLog(@"❄️❄️❄️ 测试 block isa a = %d", a);
 };

 // 下面的 a 还是在栈区的 __Block_byref_a_0 结构体实例，
 // 但是它的 __forwrding 指针是指向上面被复制堆区的 a 的，
 // 这样不管是栈区 a 还是 堆区 a，当操作 int a = 2 时，这个数值 a 都是同一个。
 ++a;
 ```

 `block` 提供的复制方法究竟是什么呢？实际上在 `ARC`  下，大多数情形下编译器会恰当的进行判断，自动生成将 `block` 从栈复制到堆上的代码。

 > 赋值时 `block` 自动从栈区复制到堆区的两个场景：

 ```c++
 // 场景一：
 // 用 clang -rewrite-objc 能转换成功
 typedef int(^BLK)(int);

 BLK func(int rate) {
     // 右边栈区 block 复制到堆区，并被 temp 持有
     BLK temp = ^(int count){ return rate * count; };
     return temp;
 }

 // 下面的代码用 clang -rewrite-objc 转换失败，改成上面就会成功，（用中间变量接收一下，（ARC 自动调用一次 copy 函数））
 typedef int(^BLK)(int);

 BLK func(int rate) {
     // 此时直接返回栈区 block 不行 
     return ^(int count){ return rate * count; };
 }

 // 失败描述，用 clang 转换失败，但是直接执行该函数是正常的
 // clang 转换错误描述说返回一个位于栈区的 block，
 // 栈区 block 出了下面花括号就被释放了，所以不能返回，
 // 同时也说明了 clang 不能动态的把栈区 block 复制到堆区，
 // 而上面有临时变量赋值时，则已经把等号右边的 block 复制到堆区，并赋值给了 temp。
 
 // 而执行时正常，是编译器能动态的把栈区 block 复制到堆区。
 
 returning block that lives on the local stack
 return ^(int count){ return rate * count; };
            ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 64 warnings and 1 error generated.

 // 场景二:
 BLK __weak blk;
 {
     NSObject *object = [[NSObject alloc] init];
     
     // NSObject * __weak object2 = object;
     
     void (^strongBlk)(void) = ^{
         NSLog(@"object = %@", object);
     };
     
     // blk 是一个弱引用变量，用一个 strong 赋值给他，
     // 它不持有该 strong 变量
     blk = strongBlk;
 }

 // blk();
 printf("blk = %p\n", blk);
 
 // 打印正常，出了花括号，block 结构体实例已经释放了:
 blk = 0x0

 BLK __weak blk;
 {
     NSObject *object = [[NSObject alloc] init];
     // NSObject * __weak object2 = object;
     // void (^strongBlk)(void) = ^{
     // NSLog(@"object = %@", object);
     // };

     // 这里给了警告: 
     // Assigning block literal to a weak variable; object will be released after assignment
     blk = ^{
         NSLog(@"object = %@", object);
     };
     
     printf("内部 blk = %p\n", blk);
 }

 // blk();
 printf("blk = %p\n", blk);
 
 // 打印：出了花括号，打印了 blk 不为 0x0，还是栈区 block 的地址
 // 打印了一个栈区 block 地址（即等号右边的栈区 block 地址）
 
 // 这里的原因是 拿一个栈区的 block 结构体实例去给一个 weak 变量赋值，并没有走真正的 weak 流程
 
 内部 blk = 0x7ffeefbff538
 blk = 0x7ffeefbff538
 
 ```
 &emsp;看一下下面这个返回 `block` 的函数:
 ```c++
 typedef int (^blk_t)(int);
 blk_t func(int rate) {
     return ^(int count) {
         return rate * count;
     };
 }
 ```
 &emsp;源代码为返回配置在栈上的 `block` 的函数。即程序执行中从 **该函数** 返回 **函数调用方** 时变量作用域结束，因此栈上的 `block` 也被废弃。虽然看似有这样的问题，但是该源代码通过对应 `ARC` 的编译器可转换为如下:
 ```c++
 blk_t func(int rate) {
 blk_t tmp = &__func_block_impl_0(__func_block_func_0, &__func_block_desc_0_DATA, rate);

 // 引用 +1
 tmp = objc_retainBlock(tmp);

 // 又被放进自动释放池
 return objc_autoreleaseReturnValue(tmp);
 }
 ```
 另外，因为 `ARC` 处于有效状态，所以 `blk_t tmp` 实际上与附有 `__strong` 修饰符的 `blk_t __strong tmp` 相同。
 在 `objc4` 找到  `objc_retainBlock` 函数实际上就是 `Block_copy` 函数:
 ```c++
 // 在 NSObject.mm 文件 31 行
 //
 // The -fobjc-arc flag causes the compiler to issue calls to objc_{retain/release/autorelease/retain_block}
 //
 
 id objc_retainBlock(id x) {
     return (id)_Block_copy(x);
 }

 // usr/include/Block.h 中找到
 // Create a heap based copy of a Block or simply add a reference to an existing one.
 // 创建基于堆的 Block 副本，或仅添加对现有 Block 的引用。（已经在堆上的 block 调用 copy 函数，引用计数增加）
 // This must be paired with Block_release to recover memory, even when running under Objective-C Garbage Collection.
 // 如果在 OC 的垃圾回收机制下使用时必须与 "Block_release" 配对使用。

 BLOCK_EXPORT void *_Block_copy(const void *aBlock)
     __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_3_2);
 ```
 即:
 ```c++
 tmp = _Block_copy(tmp);
 return objc_autoreleaseReturnValue(tmp);
 ```
 分析:
 ```c++
 // 第一步，__func_block_impl_0 结构体实例生成
 // 将通过 Block 语法生成的 Block， 
 // 即配置在栈上的 Block 用结构体实例赋值给相当于 Block 类型的变量 tmp 中
 blk_t tmp = &__func_block_impl_0(__func_block_func_0, &__func_block_desc_0_DATA, rate);

 // 第二步，_Block_copy 函数执行
 // _Block_copy 函数，将栈上的 Block 复制到堆上。
 // 复制后，将堆上的地址作为指针赋值给变量 tmp。
 tmp = _Block_copy(tmp);

 // 第三步，将堆上的 Block 作为 OC 对象，
 // 注册到 autoreleasepool 中，然后返回该对象
 return objc_autoreleaseReturnValue(tmp);
 ```
 **在 `ARC` 下，将 `block` 作为函数返回值返回时，编译器会自动生成复制到堆上的代码。**

&emsp;前面说大部分情况下编译器会适当的进行判断，不过在此之外的情况下需要 **手动**生成代码（自己调用 `copy` 函数），将 `block` 从栈上复制到 **堆**上（`_Block_copy` 函数的注释已经说了，它是创建基于堆的 `block` 副本），即我们自己主动调用 `copy` 实例方法。

**编译器不能进行判断时是什么样的状况呢？**

+ 向方法或函数的参数中传递 `block` 时。但是如果在方法或函数 **中** 适当的复制了传递过来的参数，那么就不必在调用该方法或函数前手动复制了。

以下方法或函数不用手动复制，编译器会给进行自动复制:

+ `Cocoa` 框架的方法且方法名中含有 `usingBlock` 等时
+ `Grand Central Dispatch` 的 `API`
+ 将 `block` 赋值给类的附有 `__strong` 修饰符的 `id` 类型或 `block` 类型成员变量时【当然这种情况就是最多的，只要赋值一个 `block` 变量就会自动进行复制】

**`NSArray` 的 `enumerateObjectsUsingBlock` 以及 `dispatch_async` 函数就不用手动复制。`NSArray` 的 `initWithObjects` 上传递 `block` 时需要手动复制。**

下面是个 🌰：
```c++
id obj = [Son getBlockArray];
void (^blk)(void) = [obj objectAtIndex:0];
blk();

// 对 block 主动调用 copy 函数，能正常运行 
+ (id)getBlockArray {
    int val = 10;
    return [[NSArray alloc] initWithObjects:[^{NSLog(@"blk0: %d", val);} copy], [^{NSLog(@"blk1: %d", val);} copy], nil];
}

// 如下如果不加 copy 函数，则运行崩溃
+ (id)getBlockArray {
    int val = 10;
    return [[NSArray alloc] initWithObjects:^{NSLog(@"blk0: %d", val);}, ^{NSLog(@"blk1: %d", val);}, nil];
}

// 崩溃原因: 不主动调用 copy 时，getBlockArray 函数执行结束后，栈上的 block 被废弃了，
// 编译器对此种情况不能判断是否需要复制。
// 也可以不判断全部情况都复制，但是将 Block 从栈复制到堆是相当消耗 CPU 的。
// 当 block 在栈上也能使用时，从栈上复制到堆上，就只是浪费 CPU 资源。
// 此时需要我们判断，自行手动复制。
```
| Block 的类 | 副本源的配置存储域 | 复制效果 |
| --- | --- | --- |
| _NSConcreteStackBlock | 栈 | 从栈复制到堆 |
| _NSConcreteGlobalBlock | 程序的数据区域 | 什么也不做|
| _NSConcreteMallocBlock | 堆|引用计数增加 |
不管 `Block` 配置在何处，用 `copy` 方法复制都不会引起任何问题，在不确定时调用 `copy` 方法即可。

## `__block` 变量存储域
&emsp;使用 `__block` 变量的 `block` 从栈复制到堆上时，`__block` 变量也会受到影响。
|  __block 变量的配置存储域  |  Block 从栈复制到堆时的影响  |
|  ---  |  ---  |
|  栈  |  从栈复制到堆并被 Block 持有  |
|  堆  |  被 Block 持有  |

&emsp;若在一个栈上 `block` 中使用 `__block` 变量，使用的 `__block` 变量也配置在栈上，当该 `block` 从栈复制到堆时，这些 `__block` 变量也全部被从栈复制到堆，此时，`block` 持有 `__block` 变量，即使在该 `block` 已复制到堆的情形下，复制 `block` 也对所使用的 `__block` 变量没有任何影响。

**使用 `__block` 变量的 `block` 持有 `__block` 变量。如果 `block` 被废弃，它所持有的 `__block` 变量也会被释放。**

回顾 `__block` 变量用结构体成员变量 `__forwarding` 的原因：**不管 `__block` 变量配置在栈上还是在堆上，都能够正确的访问该变量。**
通过 `block` 的复制，`__block` 变量也从栈上复制到堆上。此时可同时访问栈上的 `__block` 变量和堆上的 `__block` 变量。

示例代码：
```c++
__block int val = 0;

// 使用 copy 方法复制使用 __block 变量的 block，
// block 和 __block 变量两者均从栈复制到堆， 
// 在 block 语法的表达式中使用初始化后的 __block 变量，做了自增运算

void (^blk)(void) = [^{++val;} copy];

// 在 block 语法之后使用与 block 无关的变量，
// 此时的 val 是第一行生成的 __block 结构体实例，
// block 语法表达式中使用的 val 是 block 结构体自己的成员变量 val，
// 在 block 结构体初始化时初始化列表里面 val 初始化是用的:val(_val->__forwarding) { }

// 且此时在 block 内部通过 val 找到的 val 和外部 __block 结构体实例的 val 找到的 val 是同一个
// 在 block 表达式内 val 是一个 struct __Block_byref_val_0 的指针，在外面 __block int val = 0; 则是
// 一个 struct __Block_byref_val_0 的实例，然后 Block 结构体初始化时会使用该实例，即做到了内外统一。
// 而 __forwarding 指针用的很巧妙它是为了统一栈区 和 堆区的 __block 结构体指向统一来使用的

++val;

// 通过 clang 转换，看到两次自增运算均转换为如下形式:

// block 表达式内部：
// 首先找到 block 结构体实例的成员变量 val 
__Block_byref_val_0 *val = __cself->val; // bound by ref
// val 是结构体 __Block_byref_val_0 指针
++(val->__forwarding->val);

// 外部：
++(val.__forwarding->val);

blk();

// 且此行打印语句也是用的 val.__forwarding->val
NSLog(@"val = %d", val);
```
在变换后的 `block` 语法的函数中（`__main_block_func_0`），`val` 为复制到堆上的 `__block` 变量用结构体实例，而 `block` 语法之外的 `val`，为复制前栈上的 `__block` 变量用结构体实例。

**超级重要的一句：**
**但是栈上的 `__block` 变量用结构体实例在 `__block` 变量从栈复制到堆上时，会将成员变量 `__forwarding` 的值替换为复制目标堆上的 `__block` 变量用结构体实例的地址**。

至此，无论是在 `Block` 语法中、`Block` 语法外使用 `__block` 变量，还是 `__block` 变量配置在栈上或堆上，都可以顺利的访问到同一个 `__block` 变量。

**所有使用 `val` 的地方实际都转化为了: `val->__forwarding->val`（`block` 内部）或者 `val.__forwarding->val`（外部，是结构体实例可以直接使用 ）。**

## `block` 持有截获的对象
&emsp;在 `OC` 中，`C` 语言结构体不能含有附有 `__strong` 修饰符的变量，因为编译器不知道应何时进行 `C` 语言结构体的初始化和废弃操作，不能很好地管理内存。但是 `OC`  运行时库能准确的把握 `block` 从栈复制到堆以及堆上的 `block` 被废弃的时机，因此 `block` 用结构体中即使含有附有 `__strong` 修饰符或者 `__weak` 修饰符的变量，也可以恰当的进行初始化和废弃。为此需要 `__main_block_copy_0` 和 `__main_block_dispose_0` 函数，并把他们放在了 `__main_block_desc_0` 结构体的成员变量 `copy` 和 `dispose` 中。

`__main_block_copy_0` 函数使用 `_Block_object_assign` 函数将对象类型对象赋值给 `block` 用结构体的成员变量中并持有该对象。

`_Block_object_assign` 函数调用相当于 `retain` 实例方法的函数，将对象赋值在对象类型的结构体成员变量中。
`__main_block_dispose_0` 调用 `_Block_object_dispose`，释放赋值在 `block` 用结构体成员变量中的对象。
`_Block_object_dispose` 函数调用相当于 `release` 实例方法的函数，释放赋值在对象类型的结构体成员变量中的对象。

转换代码中 `__main_block_desc_0` 中的 `copy` 和 `dispose` 从没使用过，那什么时候会使用呢？

（这些方法都是编译器自己去调用的，我们不会主动调用它们。）

| 函数 | 调用时机 |
| --- | --- |
| copy 函数 | 栈上的 block 复制到堆时 |
| dispose 函数 | 堆上的 block 被废弃时 |

栈上 `block` 复制到堆上时的情况:

+ 调用 `block` 的 `copy` 实例方法时
+ `block` 作为函数返回值返回时
+ 将 `block` 赋值给附有 `__strong` 修饰符 `id` 类型的变量或 `block` 类型成员变量时
+ 在方法名中含有 `usingBlock` 的 `Cocoa` 框架方法或 `Grand Central Dispatch` 的 `API` 中传递 `block` 时

&emsp;这些情况下，编译器自动的将对象的 `block` 作为参数并调用 `_Block_copy` 函数，这与手动调用 `block` 的 `copy` 实例方法的效果相同。`usingBlock` 和 `GCD` 中传递 `block` 时，在该方法或函数内部对传递过来的 `block` 调用 `block` 的 `copy` 实例方法或者 `_Block_copy` 函数。

看似从栈复制到堆上，其实可归结为 `_Block_copy` 函数被调用时 `block` 从栈复制到堆。

相对，释放复制到堆上的 `block` 后，谁都不持有 `block` 而使其被废弃时调用 `dispose` 函数，这相当于对象的 `dealloc` 实例方法。

有了这些构造，通过使用附有 `__strong` 修饰符的自动变量，`block` 中截获的对象就能够超出其作用域而存在。

在使用 `__block` 变量时，已经用到 `copy` 和 `dispose` 函数：

`_Block_object_assign` 和 `_Block_object_dispose` 最后的参数有所不同:

截获对象时和使用 `__block` 变量时的不同：

| 对象 | BLOCK_FIELD_IS_OBJECT |
| __block 对象 | BLOCK_FIELD_IS_BYREF |

通过 `BLOCK_FIELD_IS_OBJECT`  和 `BLOCK_FIELD_IS_BYREF` 区分 `copy` 函数和 `dispose` 函数的对象类型是对象还是 `__block` 变量。

`copy` 函数持有截获的对象、`dispose` 函数释放截获的对象。

**`block` 中使用的赋值给附有 `__strong` 修饰符的自动变量的对象和复制到堆上的 `__block` 变量由于被堆上的 `block` 所持有，因而可超出其变量作用域而存在。**

## Block 循环引用
&emsp;如果在 `block` 中使用附有 `strong` 修饰符的对象类型自动变量，那么当 `block` 从栈复制到堆时，该对象为 `block` 所持有，不复制也会持有的，`block` 结构体初始化的时候已经将其捕获。

示例代码：
```c++
// 🌰 1. 
id array = [[NSMutableArray alloc] init];
{
    NSLog(@"⛈⛈⛈ array retainCount = %lu", (unsigned long)[array arcDebugRetainCount]);
    
    ^(id obj) {
        [array addObject:obj];
        NSLog(@"⛈⛈⛈ array retainCount = %lu", (unsigned long)[array arcDebugRetainCount]);
    };
    
    NSLog(@"⛈⛈⛈ array retainCount = %lu", (unsigned long)[array arcDebugRetainCount]);
}
NSLog(@"⛈⛈⛈ array retainCount = %lu", (unsigned long)[array arcDebugRetainCount]);

// 打印：
⛈⛈⛈ array retainCount = 1 // array 持有
⛈⛈⛈ array retainCount = 2 // array 和 栈上 block 同时持有
⛈⛈⛈ array retainCount = 1 // 出了花括号，栈上 block 释放，只剩下 array 持有

// 🌰 2.
id array = [[NSMutableArray alloc] init];
{
    NSLog(@"⛈⛈⛈ array retainCount = %lu", (unsigned long)[array arcDebugRetainCount]);
    
    blk = ^(id obj) {
        [array addObject:obj];
        NSLog(@"⛈⛈⛈  Block array retainCount = %lu", (unsigned long)[array arcDebugRetainCount]);
    };
    
    NSLog(@"⛈⛈⛈ array retainCount = %lu", (unsigned long)[array arcDebugRetainCount]);
}
NSLog(@"⛈⛈⛈ array retainCount = %lu", (unsigned long)[array arcDebugRetainCount]);

if (blk != nil) {
    blk([[NSObject alloc] init]);
    blk([[NSObject alloc] init]);
    blk([[NSObject alloc] init]);
}
// 打印：
⛈⛈⛈ array retainCount = 1 // array 持有
⛈⛈⛈ array retainCount = 3 // 花括号内，栈上 block 持有、复制到堆的 block 持有、array 持有，总共是 3
⛈⛈⛈ array retainCount = 2 // 这里减 1 是栈上 block 出了花括号后释放，同时也释放了 array，所以这里减 1
⛈⛈⛈  Block array retainCount = 2 // 这里 block 执行 3 次打印都是 2，此时 array 持有和堆上的 block blk 持有
⛈⛈⛈  Block array retainCount = 2
⛈⛈⛈  Block array retainCount = 2

// 🌰 3.
id array = [[NSMutableArray alloc] init];
{
    NSLog(@"⛈⛈⛈ array retainCount = %lu", (unsigned long)[array arcDebugRetainCount]);
    
    blk = ^(id obj) {
        [array addObject:obj];
        NSLog(@"⛈⛈⛈  Block array retainCount = %lu", (unsigned long)[array arcDebugRetainCount]);
    };
    
    NSLog(@"⛈⛈⛈ array retainCount = %lu", (unsigned long)[array arcDebugRetainCount]);
}

NSLog(@"⛈⛈⛈ array retainCount = %lu", (unsigned long)[array arcDebugRetainCount]);

if (blk != nil) {
    blk([[NSObject alloc] init]);
    blk([[NSObject alloc] init]);
    blk([[NSObject alloc] init]);
}

blk = nil;

NSLog(@"⛈⛈⛈ array retainCount = %lu", (unsigned long)[array arcDebugRetainCount]);
// 打印：
⛈⛈⛈ array retainCount = 1
⛈⛈⛈ array retainCount = 3
⛈⛈⛈ array retainCount = 2
⛈⛈⛈  Block array retainCount = 2
⛈⛈⛈  Block array retainCount = 2
⛈⛈⛈  Block array retainCount = 2 // 上面的打印完全同 2
⛈⛈⛈ array retainCount = 1 // 只有这里，blk 三次执行完毕后，blk 赋值 空，blk 释放，同时释放 array，所以还剩下 array 持有，retainCount 为 1

// 🌰 4.
{
    id array = [[NSMutableArray alloc] init];
    NSLog(@"⛈⛈⛈ array retainCount = %lu", (unsigned long)[array arcDebugRetainCount]);
    
    blk = ^(id obj) {
        [array addObject:obj];
        NSLog(@"⛈⛈⛈  Block array retainCount = %lu", (unsigned long)[array arcDebugRetainCount]);
    };
    
    NSLog(@"⛈⛈⛈ array retainCount = %lu", (unsigned long)[array arcDebugRetainCount]);
}

// NSLog(@"⛈⛈⛈ array retainCount = %lu", (unsigned long)[array arcDebugRetainCount]);

if (blk != nil) {
    blk([[NSObject alloc] init]);
    blk([[NSObject alloc] init]);
    blk([[NSObject alloc] init]);
}
// 打印：
⛈⛈⛈ array retainCount = 1 // 对象创建时为 1
⛈⛈⛈ array retainCount = 3 // 栈上 block 持有和复制到堆时堆上 block 持有 
                             // 出了花括号以后，栈上 block 释放，array 局部变量释放
                             // 剩下的 1 是堆上的 block 持有的
                             // 所以下面 block 指向时，打印都是 1
⛈⛈⛈  Block array retainCount = 1 // 出了花括号以后变量 array 释放，还剩下 block blk 自己持有，所以打印 1
⛈⛈⛈  Block array retainCount = 1
⛈⛈⛈  Block array retainCount = 1
```
```
- (id)init {
    self = [super init];
    blk_ = ^{ NSLog(@"self = %@", self);};
    return self;
}

// 依然会捕获 self,对编译器而言，obj_ 只不过是对象用结构体的成员变量。
// blk_ = ^{ NSLog(@"obj_ = %@", self->obj_); };

- (id)init {
    self = [super init];
    blk_ = ^{
        NSLog(@"obj_ = %@", obj_);
        };
        
    return self;    
}

// 除了 __weak self 也可用:
id __weak obj = obj_;
blk_ = ^{ NSLog(@"obj_ = %@", obj); };
```
&emsp;该源代码中，由于 `block` 存在时，持有该 `block` 的 `Object` 对象即赋值在变量 `tmp` 中的 `self` 必定存在，因此不需要判断变量 `tmp` 的值是否为 `nil`。
在 `iOS 4` 和 `OS X 10.6` 中，可以用 `_unsafe_unretained` 代替 `__weak` 修饰符，此处即可代替，且不必担心悬垂指针。

&emsp;**由于 `block` 语法赋值在了成员变量 `blk_` 中，因此通过 `block` 语法生成在栈上的 `block` 此时由栈复制到堆上，并持有所使用的 `self`。**

&emsp;在为避免循环引用而使用 `__weak` 修饰符时，虽说可以确认使用附有 `__weak` 修饰符的变量时，是否为 `nil`，但更有必要使之生存，以使用赋值给附有 `__weak` 修饰符变量的对象。（意思就比如上面，`block` 表达式开始执行时，首先判断 `self` 是否是 `nil`，如果不是 `nil` 才有必要继续往下执行，在往下执行的过程中并且希望 `self` 一直存在，不要正在使用时，竟被释放了，如果是单线程则无需考虑，但是在多线程开发时一定要考虑到这一点。）

&emsp;**在 `Block` 里面加 `__strong` 修饰 `weakSelf` 取得 `strongSelf`，防止 `block` 结构体实例的 `self` 成员变量过早释放。`block` 从外界所捕获的对象和在 `block` 内部使用 `__strong` 强引用的对象，差别就在于一个是在定义的时候就会影响对象的引用计数, 一个是在 `block` 运行的时候才强引用对象，且 `block` 表达式执行完毕还是会 `-1`。**

**`__weak` 修饰的对象被 `block` 引用，不会影响对象的释放，而 `__strong` 在 `block` 内部修饰的对象，会保证，在使用这个对象在 `scope` 内，这个对象都不会被释放，出了 `scope`，引用计数就会 `-1`，且 `__strong` 主要是用在多线程运用中，如果只使用单线程，则只需要使用 `__weak` 即可。**

用 `__block` 变量来避免循环引用，原理是在 `block` 内部对捕获的变量赋值为 `nil`，硬性破除引用环。
```c++
- (id)init {
    self = [super init];
    __block id tmp = self;
    blk_ = ^{
        NSLog(@"self = %@", tmp);
        tmp = nil;
    };
}
```

**对使用 `__block` 变量避免循环引用的方法和使用 `__weak` 修饰符及 `__unsafe_unretained` 修饰符避免循环引用的方法做比较:**

`__block` 优点：
+ 通过 `__block` 变量可控制对象的持有期间。
+ 在不能使用 `__weak` 修饰符的环境中不使用 `__unsafe_unretained` 修饰符即可（不必担心访问悬垂指针）
+ 在执行 `block` 时可动态决定是否将 `nil` 或其他对象赋值在 `__block` 变量中。

`__block` 缺点：
+ 为避免循环引用必须执行 `block`。

## `copy/release`
`ARC` 无效时，一般需要手动将 `block` 从栈复制到堆，另外，由于 `ARC` 无效，所以肯定要手动释放复制的 `block`。此时可用 `copy` 实例方法来复制，用 `release` 实例方法来释放。

```c++
void (^blk_on_heap)(void) = [blk_on_stack copy];
[blk_on_heap release];
```

只要 `block` 有一次 **复制并配置在堆上**，就可通过 **`retain` 实例方法** 持有。

```c++
[blk_on_heap retain];
```
但是对于 **配置在栈上的 `block`  调用 `retain` 实例方法则不起作用**。
```c++
[blk_on_stack retain];
```
该源代码中，虽然对赋值给 `blk_on_stack` 的栈上的 `block` 调用了 `retain` 实例方法，**但实际上对此源代码不起任何作用**。因此**推荐使用 `copy` 实例方法来持有 `block`**。

另外，由于 `block` 是 `C` 语言的扩展，所以在 `C` 语言中也可以使用 `block` 语法。此时使用 “`Block_copy` 函数” 和 “`Block_release` 函数” 代替 `copy/release` 实例方法。使用方法以及引用计数的思考方式与 `OC` 中的 `copy/release` 实例方法相同。
```c++
// 把栈上的 block 复制到堆上
void (^blk_on_heap)(void) = Block_copy(blk_on_stack);
// 释放堆上的 block
Block_release(blk_on_heap);
```
`Block_copy` 函数就是之前出现过的 `_Block_copy` 函数，即 `OC` 运行时库所使用的为 `C` 语言而准备的函数。释放堆上的 `block` 时也同样调用 `OC` 运行时库的 `Block_release` 函数。

&emsp;不管是 `ARC` 还是 `MRC` 栈区 `block` 都不会持有 `__block` 对象。
&emsp;**`ARC` 无效时，`__block` 说明符被用来避免 `block` 中的循环引用，这是由于当 `block` 从栈复制到堆时，若 `block` 使用的变量为附有 `__block` 说明符的 `id` 类型或对象类型的自动变量，不会被 `retain`；若 `block` 使用变量为没有 `__block` 说明符的 `id` 类型或对象类型的自动变量，则被 `retain`。**

由于 `ARC` 有效时和无效时 `__block` 说明符的用途有很大区别，因此编写源代码时，必须知道源代码是在 `ARC` 有效情况下编译还是无效情况下编译。

由于篇幅过长，下篇会通过源码分析：
1. `ARC` 下栈区 `block` 持有外部对象变量和外部 `__block` 变量。
2. 栈区 `block` 复制到堆区的过程，`__block` 变量从栈区复制到堆区的过程，以及堆区 `block`  的复制操作，堆区 `__block` 变量的复制操作。
3. 全局 `block` 的复制和释放。
4. 在 `block` 内部使用 `__strong`。
...

## 参考链接
**参考链接:🔗**
+ [libclosure-74](https://opensource.apple.com/source/libclosure/libclosure-74/)
+ [深入理解Block之Block的类型](https://www.jianshu.com/p/0855b68d1c1d)
+ [深入研究Block捕获外部变量和__block实现原理](https://www.jianshu.com/p/ee9756f3d5f6)
+ [【iOS】Block Hook概念+BlockHook第三方库分析（基本原理已完结，补充libffi方法解释）](https://blog.csdn.net/qq_32792839/article/details/99842250)
+ [最详细的block底层](https://www.cnblogs.com/MrTao/archive/2004/01/13/6824967.html)
+ [dispatch_async的block中是否该使用_weak self](https://www.jianshu.com/p/c374b7727d79)
+ [dispatch_async的block里面需要__weak self 吗？ #41](https://github.com/ibireme/YYKit/issues/41)
+ [线程安全类的设计](https://objccn.io/issue-2-4/)
