# iOS 从源码解析Runtime (六)：AutoreleasePool实现原理解读

> 上一篇文章分析了 `objc_object` 的 `retain` 和 `releasae` 等函数相关的内容，当看到 `rootAutorelease` 函数里面的 `AutoreleasePoolPage` 的时候，觉的是时候再深入学习自动释放池了，那么就由本篇开始吧。

## AutoreleasePool 大致结构图:
![AutoreleasePool结构图](https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/99d0db09905044b0acd46ccde17ee813~tplv-k3u1fbpfcp-zoom-1.image)

&emsp;在 `main.m` 中编写如下函数，然后使用 `clang -rewrite-objc main.m` 指令，把 `main.m` 转化为 `main.cpp` 文件：
```c++
// main.m 文件中:
#import <Foundation/Foundation.h>
int main(int argc, const char * argv[]) {
    
    @autoreleasepool {
    }
    
    return 0;
}

// main.cpp 相关部分：
extern "C" __declspec(dllimport) void * objc_autoreleasePoolPush(void);
extern "C" __declspec(dllimport) void objc_autoreleasePoolPop(void *);

struct __AtAutoreleasePool {

  // 构造函数
  __AtAutoreleasePool() {atautoreleasepoolobj = objc_autoreleasePoolPush();}
  
  // 析构函数
  ~__AtAutoreleasePool() {objc_autoreleasePoolPop(atautoreleasepoolobj);}
  
  // 成员变量
  void * atautoreleasepoolobj;
};

int main(int argc, const char * argv[]) {

    /* @autoreleasepool */ { __AtAutoreleasePool __autoreleasepool; 

    }

    return 0;
}

// NSObject.mm 文件中:
// objc_autoreleasePoolPush 函数实现
void *
objc_autoreleasePoolPush(void)
{
    return AutoreleasePoolPage::push();
}

// objc_autoreleasePoolPop 函数实现
NEVER_INLINE
void
objc_autoreleasePoolPop(void *ctxt)
{
    AutoreleasePoolPage::pop(ctxt);
}
```
&emsp;上面代码很清晰的告诉我们，自动释放池构造时调用 `AutoreleasePoolPage::push()` 函数，析构时调用 `AutoreleasePoolPage::pop(ctxt);` 函数。

## AutoreleasePoolPageData
&emsp;老样子我们还是首先分析其所使用的数据结构（相关的类或者结构体实现），发现这是一个很好的切入角度。每次要深入学习一个知识点时都可采取如下步骤：
1. 第一步首先找相关内容的文章，对大概的知识脉络有一个认知，尽管一些地方看不懂也没事，尽管看的模模糊糊也没事。
2. 第二步直接进入源码，源码部分一般都是简单的 `C++` 代码，然后 `Apple` 的封装也做的特别好，每个函数每个功能都特别清晰明了，看源码是最轻松也是最明了的，这时第一步看的相关原理就会在脑子里慢慢浮现慢慢被串联起来。
3. 第三步源码看完了熟络了，然后再回到第一步，然后结合着源码可看一些更高深的文章，最终可做到融会贯通。⛽️⛽️

&emsp;从 `rootAutorelease` 函数开始，按住 `command` 我们一层一层进入到 `NSObject-internal.h` 文件，它里面定义了三个结构体 `magic_t`、`AutoreleasePoolPageData`、`thread_data_t` 以及 `AutoreleasePoolPage` 的前向声明（ `class AutoreleasePoolPage`，为了在 `AutoreleasePoolPageData` 定义中有使用 `AutoreleasePoolPage` 的成员变量），正是它们构成了自动释放池的完整结构。

&emsp;首先我们先看下 `NSObject-internal.h` 文件开头的注释和几个宏定义。
> &emsp;Autorelease pool implementation
    A thread's autorelease pool is a stack of pointers. Each pointer is either an object to release, or POOL_BOUNDARY which is an autorelease pool boundary. A pool token is a pointer to the POOL_BOUNDARY for that pool. When the pool is popped, every object hotter than the sentinel is released. The stack is divided into a doubly-linked list of pages. Pages are added and deleted as necessary. Thread-local storage points to the hot page, where newly autoreleased objects are stored.
>
> &emsp;一个线程的自动释放池就是一个存放指针的栈（自动释放池整体结构是由 AutoreleasePoolPage 组成的双向链表，而每个 AutoreleasePoolPage 里面则有一个存放对象指针的栈）。栈里面的每个指针要么是等待 autorelease 的对象，要么是 POOL_BOUNDARY 自动释放池边界（实际为 #define POOL_BOUNDARY nil，同时也是 `next` 的指向）。一个 pool token 是指向 POOL_BOUNDARY 的指针。When 
the pool is popped, every object hotter than the sentinel is released. 当自动释放池执行 popped，every object hotter than the sentinel is released.。（这句没有看懂）这些栈分散位于由 AutoreleasePoolPage 构成的双向链表中。AutoreleasePoolPage 会根据需要进行添加和删除。hotPage 保存在当前线程中，当有新的 autorelease 对象添加进自动释放池时会被添加到 hotPage。

&emsp;如下宏定义:

+ `#define AUTORELEASEPOOL_VERSION 1` 自动释放池的版本号，仅当 `ABI` 的兼容性被打破时才会改变。
+ `#define PROTECT_AUTORELEASEPOOL 0` 将此设置为 1 即可进行 `mprotect()` 自动释放池的内容。（`mprotect()` 可设置自动释放池的内存区域的保护属性，限制该内存区域只可读或者可读可写）
+ `#define CHECK_AUTORELEASEPOOL (DEBUG)` 将此设置为 1 要在所有时刻都完整验证自动释放池的 `header`。（也就是 `magic_t` 的 `check()` 和 `fastcheck()`，完整验证数组的 4 个元素全部相等，还是只要验证第一个元素相等，当设置为 1 在任何地方使用 `check()` 代替 `fastcheck()`，可看出在 `Debug` 状态下是进行的完整验证，其它情况都是快速验证）

&emsp;以及开头的一段警告：`WARNING  DANGER  HAZARD  BEWARE  EEK` 告诉我们此文件的任何内容都是 `Apple` 内部使用的，它们可能在任何的版本更新中以不可预测的方式修改文件里面的内容。

### struct magic_t
```c++
struct magic_t {
    // 静态不可变 32 位 int 值
    static const uint32_t M0 = 0xA1A1A1A1;
    
    // 这个宏，emm....
#   define M1 "AUTORELEASE!"
    
    // m 数组占用 16 个字节，每个 uint32_t 占 4 个字节，减去第一个元素的 4 剩下是 12 
    static const size_t M1_len = 12;
    
    // 长度为 4 的 uint32_t 数组
    uint32_t m[4];

    // magic_t 的构造函数
    magic_t() {
        // 都是 12
        ASSERT(M1_len == strlen(M1));
        // 12 = 3 * 4
        ASSERT(M1_len == 3 * sizeof(m[1]));

        // m 数组第一个元素是 M0
        m[0] = M0;
        // 把 M1 复制到从 m[1] 开始的往后 12 个字节内
        // 那么 m 数组，前面 4 个字节放数字 M0 然后后面 12 个字节放字符串 AUTORELEASE!
        strncpy((char *)&m[1], M1, M1_len);
    }
    
    // 析构函数
    ~magic_t() {
        // Clear magic before deallocation.
        // magic_t 在 deallocation 之前清理数据。
        
        // This prevents some false positives in memory debugging tools.
        // 这样可以防止内存调试工具出现误报。
        
        // fixme semantically this should be memset_s(), 
        // but the compiler doesn't optimize that at all (rdar://44856676).
        // fixme 从语义上讲，这应该是 memset_s（），但是编译器根本没有对其进行优化。
        
        // 把 m 转化为一个 uint64_t 的数组， uint64_t 类型占 8 个字节，
        // 即把原本 4 个元素每个元素 4 个字节共 16 个字节的数组转化成了 2 个元素每个元素 8 个字节共 16 个字节的数组。 
        volatile uint64_t *p = (volatile uint64_t *)m;
        
        // 16 个字节置 0
        p[0] = 0; p[1] = 0;
    }

    bool check() const {
        // 检测
        // 0 元素是 M0，后面 12 个字节是 M1，和构造函数中初始化的值一模一样的话即返回 true
        return (m[0] == M0 && 0 == strncmp((char *)&m[1], M1, M1_len));
    }

    bool fastcheck() const {
#if CHECK_AUTORELEASEPOOL
        // 程序在 DEBUG 模式下执行完整比较
        return check();
#else
        // 程序在 RELEASE 模式下是只比较 m[0] 的值是 0xA1A1A1A1
        return (m[0] == M0);
#endif
    }

// M1 解除宏定义
#   undef M1
};
```
### struct AutoreleasePoolPageData
```c++
// 前向声明，AutoreleasePoolPage 是私有继承自 AutoreleasePoolPageData 的类，
// 在 AutoreleasePoolPageData 中要声明 AutoreleasePoolPage 类型的成员变量，
// 即双向链表中使用的两个指针 parent 和 child。
class AutoreleasePoolPage;

struct AutoreleasePoolPageData
{
    // struct magic_t 作为 AutoreleasePoolPage 的 header 来验证 AutoreleasePoolPage
    // 0xA1A1A1A1AUTORELEASE!
    magic_t const magic;
    
    // __unsafe_unretained 修饰的 next，看源码还是第一次见使用修饰符，
    // next 指针作为游标指向栈顶最新 add 进来的 autorelease 对象的下一个位置
    __unsafe_unretained id *next;
    
    // typedef __darwin_pthread_t pthread_t;
    // typedef struct _opaque_pthread_t *__darwin_pthread_t;
    // 原始是 struct _opaque_pthread_t 指针，
    // AutoreleasePool 是按线程一一对应的，thread 是自动释放池所处的线程。
    pthread_t const thread;
    
    // AutoreleasePool 没有单独的结构，而是由若干个 AutoreleasePoolPage 以双向链表的形式组合而成，
    // parent 和 child 这两个 AutoreleasePoolPage 指针正是构成链表用的值指针。
    AutoreleasePoolPage * const parent;
    AutoreleasePoolPage *child;
    
    // 标记每个指针的深度，例如第一个 page 的 depth 是 0，后续新增的 page 的 depth 依次递增
    uint32_t const depth;
    
    // high-water
    uint32_t hiwat;

    // 构造函数
    // 初始化列表中 parent 根据 _parent 初始化，child 初始化为 nil
    // 这里可以看出，第一个 page 的 parent 和 child 都是 nil
    // 然后第二个 page 初始化时第一个 page 作为它的 parent 传入
    // 然后第一个 page 的 child 指向 第二个 page，parent 指向 nil
    // 第二个 page 的 parent 指向第一个 page，child 此时指向 nil
    AutoreleasePoolPageData(__unsafe_unretained id* _next,
                            pthread_t _thread,
                            AutoreleasePoolPage* _parent,
                            uint32_t _depth,
                            uint32_t _hiwat) : magic(),
                                               next(_next),
                                               thread(_thread),
                                               parent(_parent),
                                               child(nil),
                                               depth(_depth),
                                               hiwat(_hiwat){
    }
};
```
### struct thread_data_t
```c++
struct thread_data_t
{
#ifdef __LP64__
    pthread_t const thread; // pthread_t 的实际类型是 struct _opaque_pthread_t 的指针，占 8 个字节
    uint32_t const hiwat; // 4 字节
    uint32_t const depth; // 4 字节
#else
    pthread_t const thread;
    uint32_t const hiwat;
    uint32_t const depth;
    uint32_t padding;
#endif
};

// 一个断言，如果 thread_data_t 的 size 不是 16 的话就会执行该断言
// 可以看到在 __LP64__ 平台同时遵循内存对齐原则下 thread_data_t size 也正是 8 + 4 + 4 = 16
C_ASSERT(sizeof(thread_data_t) == 16);
```

## AutoreleasePoolPage
&emsp;下面我们开始解读 `AutoreleasePoolPage` 源码。

### BREAKPOINT_FUNCTION
&emsp;`BREAKPOINT_FUNCTION` 宏定义在 `objc-os.h` 文件下，针对不同的运行环境它的定义是不同的。
在 `TARGET_OS_MAC` 环境下定义如下: 
```c++
/* Use this for functions that are intended to be breakpoint hooks. 
   If you do not, the compiler may optimize them away.
   BREAKPOINT_FUNCTION( void stop_on_error(void) );
   // 如果我们要对某个函数做 breakpoint hook，则需要用该宏定义声明一下。
   // 如果我们不这样做的话，编译器可能会优化它们。
   // 例如：BREAKPOINT_FUNCTION( void stop_on_error(void) );
*/
#   define BREAKPOINT_FUNCTION(prototype)                             \
    OBJC_EXTERN __attribute__((noinline, used, visibility("hidden"))) \
    prototype { asm(""); }
```

#### __attribute__((used))
&emsp;`__attribute__((used))` 的作用：

1. 用于告诉编译器在目标文件中保留一个 **静态函数** 或者 **静态变量**，即使它没有被引用。
2. 标记为 `attribute__((used))` 的函数被标记在目标文件中，以避免 **链接器** 删除未使用的节。
3. **静态变量** 也可以标记为 `used`，方法是使用 `attribute((used))`。
4. 使用 `used` 字段，即使没有任何引用，在 `Release` 下也不会被优化。

&emsp;警告信息产生的原因可参考: [__attribute__((used))的使用问题](http://www.openedv.com/forum.php?mod=viewthread&tid=277480&extra=page=3)
```c++
// 表示该函数或变量可能不使用，这个属性可以避免编译器产生警告信息
#define __attribute_unused__ __attribute__((__unused__))

// 向编译器说明这段代码有用，即使在没有用到的情况下编译器也不会警告
#define __attribute_used__ __attribute__((__used__))
```

#### __attribute__((visibility("hidden")))
> &emsp;在 Linux 下动态库 (.so) 中，通过 GCC 的 C++ visibility 属性可以控制共享文件导出符号。在 GCC 4.0 及以上版本中，有个 visibility 属性，可以应用到函数、变量、模板以及 C++ 类。
  限制符号可见性的原因：从动态库中尽可能少地输出符号是一个好的实践经验。输出一个受限制的符号会提高程序的模块性，并隐藏实现的细节。动态库装载和识别的符号越少，程序启动和运行的速度就越快。导出所有符号会减慢程序速度，并耗用大量内存。
  default：用它定义的符号将被导出，动态库中的函数默认是可见的。
  hidden：用它定义的符号将不被导出，并且不能从其它对象进行使用，动态库中的函数是被隐藏的。
  default 意味着该方法对其它模块是可见的。而 hidden 表示该方法符号不会被放到动态符号表里，所以其它模块(可执行文件或者动态库)不可以通过符号表访问该方法。
  要定义 GNU 属性，需要包含 \_\_attribute\_\_ 和用括号括住的内容。可以将符号的可见性指定为 visibility(“hidden”)，这将不允许它们在库中被导出，但是可以在源文件之间共享。实际上，隐藏的符号将不会出现在动态符号表中，但是还被留在符号表中用于静态链接。
  导出列表由编译器在创建共享库的时候自动生成，也可以由开发人员手工编写。导出列表的原理是显式地告诉编译器可以通过外部文件从对象文件导出的符号是哪些。GNU 用户将此类外部文件称作为导出映射。[Linux下__attribute__((visibility ("default")))的使用](https://blog.csdn.net/fengbingchun/article/details/78898623)

&emsp;在 `TARGET_OS_WIN32` 环境下定义如下: 
```c++
/* Use this for functions that are intended to be breakpoint hooks.
   If you do not, the compiler may optimize them away.
   BREAKPOINT_FUNCTION( void MyBreakpointFunction(void) ); */
   // 和👆一致
   // 看宏定义的内容，意思是把函数标记为 noinline(不要内联) 就行了吗？
#   define BREAKPOINT_FUNCTION(prototype) \
    __declspec(noinline) prototype { __asm { } }
```
&emsp;用 `BREAKPOINT_FUNCTION` 修饰如下两个函数：
```c++
BREAKPOINT_FUNCTION(void objc_autoreleaseNoPool(id obj));
BREAKPOINT_FUNCTION(void objc_autoreleasePoolInvalid(const void *token));
```
&emsp;`AutoreleasePoolPage` 是一个私有继承自 `AutoreleasePoolPageData` 的类。 `thread_data_t` 是 `AutoreleasePoolPage` 的友元结构体，可直接访问 `AutoreleasePoolPage` 的私有成员变量。
```c++
class AutoreleasePoolPage : private AutoreleasePoolPageData
{
    friend struct thread_data_t;
    ...
};
```
### SIZE
&emsp;表示 `AutoreleasePoolPage` 的容量。已知在 `NSObject-internal.h` 中 `PROTECT_AUTORELEASEPOOL` 值为 `0`，那么 `SIZE` 的值是 `PAGE_MIN_SIZE`。（在 `vm_param.h` 中 `PAGE_MAX_SIZE` 和 `PAGE_MIN_SIZE` 都是 `4096`...）
```c++
    static size_t const SIZE =
#if PROTECT_AUTORELEASEPOOL
        PAGE_MAX_SIZE;  // must be multiple of vm page size
#else
        PAGE_MIN_SIZE;  // size and alignment, power of 2
#endif

#define PAGE_MIN_SIZE           PAGE_SIZE
#define PAGE_SIZE               I386_PGBYTES
#define I386_PGBYTES            4096            /* bytes per 80386 page */
```
可看到 `SIZE` 的值是 `4096` 单位是字节（保存的 `autorelease` 对象的指针，每个指针占 8 个字节）。

```c++
private:
    // typedef __darwin_pthread_key_t pthread_key_t;
    // typedef unsigned long __darwin_pthread_key_t;
    // 所以 pthread_key_t 实际是一个 unsigned long 类型
    
    // #define AUTORELEASE_POOL_KEY ((tls_key_t)__PTK_FRAMEWORK_OBJC_KEY3)
    // typedef pthread_key_t tls_key_t;
    // #define __PTK_FRAMEWORK_OBJC_KEY3    43
    
    // AUTORELEASE_POOL_KEY 
    // tls 全拼是 Thread Local Storage 表示在当前线程存储一些数据用，（已知线程都有自己的存储空间）
    // 而这些数据的存储与读取是通过这些固定的 Key 来做的。
    
    // 通过此 key 从当前线程的存储中取出 hotPage
    static pthread_key_t const key = AUTORELEASE_POOL_KEY;
    
    // SCRIBBLE
    // 在 releaseUntil 函数中，page 中的 objc_object ** 指向的对象执行 objc_release，
    // 然后它们留空的位置会放 SCRIBBLE 
    // 也就是说通过 objc_objcect ** 把指向的对象执行 release 后，
    // 把之前存放 objc_object ** 的位置放 SCRIBBLE 
    static uint8_t const SCRIBBLE = 0xA3;  // 0xA3A3A3A3 after releasing
    
    // 可保存的 id 的数量 4096 / 8 = 512 (实际可用容量是 4096 减去成员变量占用的 56 字节 )
    static size_t const COUNT = SIZE / sizeof(id);

    // EMPTY_POOL_PLACEHOLDER is stored in TLS when exactly one 
    // pool is pushed and it has never contained any objects. 
    // 当创建了一个自动释放池且未放入任何对象的时候 EMPTY_POOL_PLACEHOLDER 就会存储在 TLS 中。 
    
    // This saves memory when the top level (i.e. libdispatch) 
    // pushes and pops pools but never uses them.
    // 当 top level(例如 libdispatch) pushes 和 pools 却从不使用它们的时候可以节省内存。
    
    // 把 1 转为 objc_object **
#   define EMPTY_POOL_PLACEHOLDER ((id*)1)

    // pool 的边界是指一个 nil
#   define POOL_BOUNDARY nil

// SIZE-sizeof(*this) bytes of contents follow
```

### new/delete
```c++
// 申请空间并进行内存对齐

static void * operator new(size_t size) {
    // extern malloc_zone_t *malloc_default_zone(void); /* The initial zone */ // 初始 zone
    // extern void *malloc_zone_memalign(malloc_zone_t *zone,
    //                                   size_t alignment, 
    //                                   size_t size) 
    //                                   __alloc_size(3) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_3_0);
    // alignment 对齐长度
    // 分配一个大小为 size 的新指针，其地址是对齐的精确倍数。
    // 对齐方式必须是 2 的幂并且至少与 sizeof(void *) 一样大。 zone 必须为非 NULL。
    return malloc_zone_memalign(malloc_default_zone(), SIZE, SIZE);
}
static void operator delete(void * p) {
    // 释放内存
    return free(p);
}
```
### protect/unprotect
&emsp;已知在 `NSObject-internal.h` 中 `PROTECT_AUTORELEASEPOOL` 值为 `0`，所以这两个函数当前什么也不做。
```c++
    inline void protect() {
#if PROTECT_AUTORELEASEPOOL
        // 从 this 开始的长度为 SIZE 的内存区域只可读
        mprotect(this, SIZE, PROT_READ);
        check();
#endif
    }

    inline void unprotect() {
#if PROTECT_AUTORELEASEPOOL
        check();
        // 从 this 开始的长度为 SIZE 的内粗区域可读可写
        mprotect(this, SIZE, PROT_READ | PROT_WRITE);
#endif
    }
```
&emsp;在 `Linux` 中 `mprotect()` 函数可以用来修改一段指定内存区域的保护属性。例如指定一块区域只可读、只可写、可读可写等等。函数原型如下：
```c++
#include <unistd.h>
#include <sys/mmap.h>
int mprotect(const void *start, size_t len, int prot);
```
&emsp;`mprotect()` 函数把自 `start` 开始的、长度为 `len` 的内存区的保护属性修改为 `prot` 指定的值。`prot` 可以取以下几个值，并且可以用 `|` 将几个属性合起来使用：

1. `PROT_READ`：表示内存段内的内容可读。
2. `PROT_WRITE`：表示内存段内的内容可写。
3. `PROT_EXEC`：表示内存段中的内容可执行。
4. `PROT_NONE`：表示内存段中的内容根本没法访问。

&emsp;需要指出的是，指定的内存区间必须包含整个内存页（一般为 `4Kb` 大小, 不同体系结构和操作系统，一页的大小不尽相同。如何获得页大小呢？通过 `PAGE_SIZE` 宏或者 `getpagesize()` 系统调用即可）。区间开始的地址 `start` 必须是一个内存页的起始地址，并且区间长度 `len` 必须是页大小的整数倍。如果执行成功，则返回 `0`；如果执行失败，则返回 `-1`。具体内容可参考[mprotect 函数用法](https://www.cnblogs.com/ims-/p/13222243.html)

### AutoreleasePoolPage(AutoreleasePoolPage *newParent)/~AutoreleasePoolPage()
&emsp;`AutoreleasePoolPage` 的构造函数，看到这里用了一个 `AutoreleasePoolPage *newParent` 作为参数，我们已知自动释放池的完整结构 是一个由 `AutoreleasePoolPage` 构成的双向链表，它的成员变量 `AutoreleasePoolPage * const parent` 和 `AutoreleasePoolPage *child` 作为前后两个链接节点的链接指针，那么 `parent` 和 `child` 谁在前谁在后呢？（`parent` 在前，`child` 在后）

&emsp;根据 `AutoreleasePoolPageData` 的构造函数可知，第一个节点的 `parent`  和 `child` 都是 `nil`，当第一个 `AutoreleasePoolPage` 满了，会再创建一个 `AutoreleasePoolPage`，此时会拿第一个节点作为 `newParent` 参数来构建这第二个节点，即第一个节点的 `child` 指向第二个节点，第二个节点的 `parent` 指向第一个节点。

```c++
AutoreleasePoolPage(AutoreleasePoolPage *newParent) :
    AutoreleasePoolPageData(begin(),
                            // 当前所处的线程，这获取线程的函数好复杂，下面会进行具体分析
                            objc_thread_self(),
                            // parent 
                            newParent,
                            // 可以理解为 page 的序号，第一个节点的 depth 是 0，
                            // 第二个节点是 1，第三个节点是 2，依次累加
                            newParent ? 1+newParent->depth : 0,
                            // high-water
                            newParent ? newParent->hiwat : 0)
{ 
    if (parent) {
        // 检查 parent 节点是否合规，检查 magic 和 thread
        parent->check();
        
        // parent 节点的 child 必须为 nil，因为当前新建的 page 要作为 parent 的 child
        ASSERT(!parent->child);
        
        // 可读可写
        parent->unprotect();
        
        // 把当前节点作为入参 newParent 的 child 节点
        parent->child = this;
        
        // 只可读
        parent->protect();
    }
    
    // 只可读
    protect();
}

// 析构函数
~AutoreleasePoolPage() 
{
    // 检查
    check();
    
    // 可读可写
    unprotect();
    
    // page 里面没有 autorelease 对象否则执行断言
    ASSERT(empty());

    // Not recursive: we don't want to blow out the stack 
    // if a thread accumulates a stupendous amount of garbage
    
    // child 指向 nil 否则执行断言
    ASSERT(!child);
}
```
&emsp;看到 `AutoreleasePoolPage` 必须满足 `empty()` 和 `child` 指向 `nil`，同时还有 `magic.check()` 必须为真，还有 `thread == objc_thread_self()`，这四个条件同时满足时才能正常析构。

### busted/busted_die
```c++
// 根据 log 参数不同会决定是 _objc_fatal 或 _objc_inform
template<typename Fn>
void busted(Fn log) const {
    // 一个完整默认值的 magic_t 变量 
    magic_t right;
    
    // log
    log("autorelease pool page %p corrupted\n"
         "  magic     0x%08x 0x%08x 0x%08x 0x%08x\n"
         "  should be 0x%08x 0x%08x 0x%08x 0x%08x\n"
         "  pthread   %p\n"
         "  should be %p\n", 
         this, 
         magic.m[0], magic.m[1], magic.m[2], magic.m[3], 
         right.m[0], right.m[1], right.m[2], right.m[3], 
         this->thread, objc_thread_self());
}

__attribute__((noinline, cold, noreturn))
void busted_die() const {
    // 执行 _objc_fatal 打印
    busted(_objc_fatal);
    __builtin_unreachable();
}
```
### check/fastcheck
&emsp;检查 `magic` 是否等于默认值和检查当前所处的线程，然后 `log` 传递 `_objc_inform` 或 `_objc_fatal` 调用 `busted` 函数。 
```c++
    inline void
    check(bool die = true) const
    {
        if (!magic.check() || thread != objc_thread_self()) {
            if (die) {
                busted_die();
            } else {
                busted(_objc_inform);
            }
        }
    }

    inline void
    fastcheck() const
    {
    // #define CHECK_AUTORELEASEPOOL (DEBUG) // DEBUG 模式为 true RELEASE 模式为 false
#if CHECK_AUTORELEASEPOOL
        check();
#else
        // 如果 magic.fastcheck() 失败则执行 busted_die
        if (! magic.fastcheck()) {
            busted_die();
        }
#endif
    }
```
### begin/end/empty/full/lessThanHalfFull

#### begin
&emsp;`begin` 函数超关键的，首先要清楚一点 `begin` 是 `AutoreleasePoolPage` 中存放的 **自动释放对象** 的起点。回顾上面的的 `new` 函数的实现我们已知系统总共给 `AutoreleasePoolPage` 分配了 `4096` 个字节的空间，这么大的空间除了前面一部分空间用来保存 `AutoreleasePoolPage` 的成员变量外，剩余的空间都是用来存放自动释放对象地址的。

&emsp;`AutoreleasePoolPage` 的成员变量都是继承自 `AutoreleasePoolPageDate`，它们总共需要 `56` 个字节的空间，然后剩余 `4040` 字节空间，一个对象指针占 `8` 个字节，那么一个 `AutoreleasePoolPage` 能存放 `505` 个需要自动释放的对象。（可在 `main.m` 中引入 `#include "NSObject-internal.h"` 打印 `sizeof(AutoreleasePoolPageData)` 的值确实是 `56`。）
```c++
id * begin() {
    // (uint8_t *)this 是 AutoreleasePoolPage 的起始地址，
    // 且这里用的是 (uint8_t *) 的强制类型转换，uint8_t 占 1 个字节，
    // 然后保证 (uint8_t *)this 加 56 时是按 56 个字节前进的
    
    // sizeof(*this) 是 AutoreleasePoolPage 所有成员变量的宽度是 56 个字节，
    // 返回从 page 的起始地址开始前进 56 个字节后的内存地址。
    return (id *) ((uint8_t *)this+sizeof(*this));
}
```
#### end
```c++
id * end() {
    // (uint8_t *)this 起始地址，转为 uint8_t 指针
    // 然后前进 SIZE 个字节，刚好到 AutoreleasePoolPage 的末尾
    return (id *) ((uint8_t *)this+SIZE);
}
```

#### empty
&emsp;`next` 指针通常指向的是当前自动释放池内最后面一个自动释放对象的后面，如果此时 `next` 指向 `begin` 的位置，表示目前自动释放池内没有存放自动释放对象。

```c++
bool empty() {
    return next == begin();
}
```
#### full
&emsp;理解了 `empty` 再看 `full` 也很容易理解，`next` 指向了 `end` 的位置，表明自动释放池内已经存满了需要自动释放的对象。
```c++
bool full() { 
    return next == end();
}
```
#### lessThanHalfFull
&emsp;表示目前自动释放池存储的自动释放对象是否少于总容量的一半。`next` 与 `begin` 的距离是当前存放的自动释放对象的个数，`end` 与 `begin` 的距离是可以存放自动释放对象的总容量。
```c++
bool lessThanHalfFull() {
    return (next - begin() < (end() - begin()) / 2);
}
```

### add
&emsp;把 `autorelease` 对象放进自动释放池。
```c++
id *add(id obj)
{
    // 如果自动释放池已经满了，则执行断言
    ASSERT(!full());
    
    // 可读可写
    unprotect();
    
    // 记录当前 next 的指向，作为函数的返回值。比 `return next-1` 快，没看懂 aliasing 是什么意思
    id *ret = next;  // faster than `return next-1` because of aliasing
    
    // next 是一个 objc_object **，先使用解引用操作符 * 取出 objc_object * ，
    // 然后把 obj 赋值给它，然后 next 会做一次自增操作前进 8 个字节，指向下一个位置。
    *next++ = obj;
    
    // 只可读
    protect(); 
    
    // ret 目前正是指向 obj 的位置。（obj 是 objc_object 指针，不是 objc_object）
    return ret;
}
```

### releaseAll/releaseUntil
```c++
void releaseAll() 
{
    // 调用 releaseUntil 并传入 begin，
    // 从 next 开始，一直往后移动，直到 begin，
    // 把 begin 到 next 之间的所有自动释放对象执行一次 objc_release 操作
    releaseUntil(begin());
}
```
&emsp;从 `next` 开始一直向后移动直到到达 `stop`，把经过路径上的所有自动释放对象都执行一次 `objc_release` 操作。
```c++
    void releaseUntil(id *stop) 
    {
        // Not recursive: we don't want to blow out the stack 
        // if a thread accumulates a stupendous amount of garbage
        
        // 循环从 next 开始，一直后退，直到 next 到达 stop
        while (this->next != stop) {
            // Restart from hotPage() every time, in case -release 
            // autoreleased more objects
            
            // 取得当前的 AutoreleasePoolPage
            AutoreleasePoolPage *page = hotPage();

            // fixme I think this `while` can be `if`, but I can't prove it
            // fixme 我认为 “while” 可以是 “if”，但我无法证明
            // 我觉得也是可以用 if 代替 while
            // 一个 page 满了会生成一个新的 page 并链接为下一个 page，
            // 所以从第一个 page 开始到 hotPage 的前一个page，应该都是满的
            
            // 如果当前 page 已经空了，则往后退一步，把前一个 AutoreleasePoolPage 作为 hotPage
            while (page->empty()) {
                // 当前 page 已经空了，还没到 stop，
                // 往后走 
                page = page->parent;
                // 把 page 作为 hotPage
                setHotPage(page);
            }
            
            // 可读可写
            page->unprotect();
            
            // next 后移一步，并用解引用符取出 objc_object * 赋值给 obj
            id obj = *--page->next;
            
            // 把 page->next 开始的 sizeof(*page->next) 个字节置为 SCRIBBLE
            memset((void*)page->next, SCRIBBLE, sizeof(*page->next));
            
            // 只可读
            page->protect();
            
            // 如果 obj 不为 nil，则执行 objc_release 操作
            if (obj != POOL_BOUNDARY) {
                objc_release(obj);
            }
        }

        // 这里还是把 this 作为 hotPage，
        // 可能从 stop 所在的 page 开始到 hotPage 这些 page 本来存放自动释放对象的位置都放的是 SCRIBBLE
        setHotPage(this);

#if DEBUG
        // we expect any children to be completely empty
        // 保证从当前 page 的 child 开始，向后都是空 page
        for (AutoreleasePoolPage *page = child; page; page = page->child) {
            ASSERT(page->empty());
        }
#endif
    }
```
&emsp;从最前面的 `page` 开始一直向后移动直到到达 `stop` 所在的 `page`，并把经过的 `page` 里保存的对象都执行一次 `objc_release` 操作，把之前每个存放 `objc_object **` 的空间都置为 `SCRIBBLE`，每个 `page` 的 `next` 都指向了该 `page` 的 `begin`。

**这里有一个疑问, this  和 hotPage 可能是同一个 page 吗？**

### kill
&emsp;`release` 做的事情是遍历释放保存的自动释放对象，而 `kill` 做的事情是遍历对 `AutoreleasePoolPage` 执行 `delete` 操作。
```c++
void kill() 
{
    // Not recursive: we don't want to blow out the stack 
    // if a thread accumulates a stupendous amount of garbage
    
    AutoreleasePoolPage *page = this;
    // 从当前 page 开始一直沿着 child 链往前走，直到 AutoreleasePool 的双向链表的最后一个 page
    while (page->child) page = page->child;

    // 临时变量（死亡指针）
    AutoreleasePoolPage *deathptr;
    
    // 是 do while 循环，所以会至少进行一次 delete，
    // 即当前 page 也会被执行 delete（不同与上面的 release 操作，入参 stop 并不会执行 objc_release 操作）
    do {
        // 要执行 delete 的 page
        deathptr = page;
        
        // 记录前一个 page
        page = page->parent;
        
        // 如果当前 page 的 parent 存在的话，要把这个 parent 的 child 置为 nil
        // 这个是链表算法题的经典操作
        if (page) {
            // 可读可写
            page->unprotect();
            
            // child 置为 nil
            page->child = nil;
            
            // 可写
            page->protect();
        }
        
        // delete page
        delete deathptr;
    } while (deathptr != this);
}
```
&emsp;从当前的 `page` 开始，一直根据 `child` 链向前走直到 `child` 为空，把经过的 `page` 全部执行 `delete` 操作（包括当前 `page`）。
### tls_dealloc
&emsp;`Thread Local Stroge` `dealloc` 的时候，要把自动释放池内的所有自动释放对象执行 `release` 操作，然后所有的 `page` 执行 `kill`。 
```c++
static void tls_dealloc(void *p) 
{
    // # define EMPTY_POOL_PLACEHOLDER ((id*)1)
    // 如果 p 是空占位池则 return
    if (p == (void*)EMPTY_POOL_PLACEHOLDER) {
        // No objects or pool pages to clean up here.
        // 这里没有 objects 或者 pages 需要清理
        return;
    }

    // reinstate TLS value while we work
    // 这里直接把 p 保存在 TLS 中作为 hotPage
    setHotPage((AutoreleasePoolPage *)p);

    if (AutoreleasePoolPage *page = coldPage()) {
        // 如果 coldPage 存在（双向链表中的第一个 page）
        
        // 这个调用的函数链超级长，最终实现的是把自动释放池里的所有自动释放对象都执行
        // objc_release 然后所有的 page 执行 delete 
        if (!page->empty()) objc_autoreleasePoolPop(page->begin());  // pop all of the pools
        
        // OPTION( DebugMissingPools, 
        //         OBJC_DEBUG_MISSING_POOLS,
        //         "warn about autorelease with no pool in place, which may be a leak")
        // 警告没有池的自动释放，这可能是泄漏
        
        // OPTION( DebugPoolAllocation,
        //         OBJC_DEBUG_POOL_ALLOCATION,
        //         "halt when autorelease pools are popped out of order,
        //          and allow heap debuggers to track autorelease pools")
        // 当自动释放池顺序弹出时暂停，并允许堆调试器跟踪自动释放池
        
        if (slowpath(DebugMissingPools || DebugPoolAllocation)) {
            // pop() killed the pages already
        } else {
            // 从 page 开始一直沿着 child 向前把所有的 page 执行 delete
            // kill 只处理 page，不处理 autorelease 对象
            page->kill();  // free all of the pages
        }
    }
    
    // clear TLS value so TLS destruction doesn't loop
    // 清除 TLS 值，以便 TLS 销毁不会循环
    // 把 hotPage 置为 nil
    // static pthread_key_t const key = AUTORELEASE_POOL_KEY;
    // tls_set_direct(key, (void *)page);
    // 把 key 置为 nil
    setHotPage(nil);
}
```
### pageForPointer
&emsp;`void *p` 转为 `AutoreleasePoolPage *`，主要用于把指向 `begin()` 的指针转为 `AutoreleasePoolPage *`。
```c++
static AutoreleasePoolPage *pageForPointer(const void *p) 
{
    // 指针转为 unsigned long
    return pageForPointer((uintptr_t)p);
}

static AutoreleasePoolPage *pageForPointer(uintptr_t p) 
{
    // result 临时变量
    AutoreleasePoolPage *result;
    
    // 首先 page 创建时 malloc_zone_memalign(malloc_default_zone(), SIZE, SIZE);
    // 是根据 SIZE 进行内存对齐的，所以 每个 page 的起始地址一定是 SIZE 的整数倍
    // p 对 1024 取模
    uintptr_t offset = p % SIZE;

    // 对 4096 取模，所以 offset 的值应该是在 0~4095 之间
    // sizeof(AutoreleasePoolPage) 的值应该和 sizeof(AutoreleasePoolPageData) 一样的，都是 56
    // 同时由于 p 入参进来至少是从 page 的 begin() 位置开始的，所以说至少从 page 的起始地址偏移 56 后开始的，
    // 所以这个 offset 的范围是 [56 4095] 区间内
    ASSERT(offset >= sizeof(AutoreleasePoolPage));

    // p 减掉 offset，p 倒退到 page 的起点位置
    result = (AutoreleasePoolPage *)(p - offset);
    
    // 验证 result 是否 magic.check() 和 thread == objc_thread_self()，两个必须满足的的条件
    result->fastcheck();

    return result;
}
```
### haveEmptyPoolPlaceholder/setEmptyPoolPlaceholder
&emsp;每个线程都有自己的存储空间。这里是根据 `key` 在当前线程的存储空间里面保存一个空池。
```c++
// 两个静态内联函数
static inline bool haveEmptyPoolPlaceholder()
{
    // key 是一个静态局部变量
    // static pthread_key_t const key = AUTORELEASE_POOL_KEY;
    // # define AUTORELEASE_POOL_KEY ((tls_key_t)__PTK_FRAMEWORK_OBJC_KEY3)
    // # define EMPTY_POOL_PLACEHOLDER ((id*)1)
    
    // 在当前线程根据 key 找到一个空池
    id *tls = (id *)tls_get_direct(key);
    
    // 如果未找到则返回 false
    return (tls == EMPTY_POOL_PLACEHOLDER);
}

static inline id* setEmptyPoolPlaceholder()
{
    // 当前线程没有存储 key 对应的内容，否则执行断言
    // 这里会覆盖原始值，所以必须保证 key 下面现在没有存储 page
    ASSERT(tls_get_direct(key) == nil);
    
    // 把 EMPTY_POOL_PLACEHOLDER 根据 key 保存在当前线程的存储空间内
    tls_set_direct(key, (void *)EMPTY_POOL_PLACEHOLDER);
    
    // 返回 EMPTY_POOL_PLACEHOLDER，（(id *)1）
    return EMPTY_POOL_PLACEHOLDER;
}
```
### hotPage/setHotPage
```c++
static inline AutoreleasePoolPage *hotPage() 
{
    // 当前的 hotPage 是根据 key 保存在当前线程的存储空间内的
    AutoreleasePoolPage *result = (AutoreleasePoolPage *)tls_get_direct(key);
    
    // 如果等于 EMPTY_POOL_PLACEHOLDER 的话，返回 nil
    if ((id *)result == EMPTY_POOL_PLACEHOLDER) return nil;
    
    // result 执行 check 判断是否符合 AutoreleasePoolPage 的约束规则
    if (result) result->fastcheck();
    
    return result;
}

static inline void setHotPage(AutoreleasePoolPage *page) 
{
    // page 入参检测，判断是否符合 AutoreleasePoolPage magic 的约束规则 
    if (page) page->fastcheck();
    
    // 根据 key 把 page 保存在当前线程的存储空间内，作为 hotPage
    tls_set_direct(key, (void *)page);
}
```
### coldPage
&emsp;"冷" `page`，首先找到 `hotPage` 然后沿着它的 `parent` 走，直到最后 `parent` 为 `nil`，最后一个 `AutoreleasePoolPage` 就是 `coldPage`，返回它。这里看出来其实 `coldPage` 就是双向 `page` 链表的第一个 `page`。
```c++
static inline AutoreleasePoolPage *coldPage() 
{
    // hotPage
    AutoreleasePoolPage *result = hotPage();
    if (result) {
        // 循环一直沿着 parent 指针找，直到第一个 AutoreleasePoolPage
        while (result->parent) {
            // 沿着 parent 更新 result
            result = result->parent;
            
            // 检测 result 符合 page 规则
            result->fastcheck();
        }
    }
    return result;
}
```
### autoreleaseFast
&emsp;把对象快速放进自动释放池。
```c++
static inline id *autoreleaseFast(id obj)
{
    // hotPage
    AutoreleasePoolPage *page = hotPage();
    
    if (page && !page->full()) {
        // 如果 page 存在并且 page 未满，则直接调用 add 函数把 obj 添加到 page
        return page->add(obj);
    } else if (page) {
        // 如果 page 满了，则调用 autoreleaseFullPage 构建新 AutoreleasePoolPage，并把 obj 添加进去
        return autoreleaseFullPage(obj, page);
    } else {
        // 连 hotPage 都不存在，可能就一 EMPTY_POOL_PLACEHOLDER 在线程的存储空间内保存 
        // 如果 page 不存在，即当前线程还不存在自动释放池，构建新 AutoreleasePoolPage，并把 obj 添加进去
        return autoreleaseNoPage(obj);
    }
}
```
### autoreleaseFullPage
```c++
static __attribute__((noinline))
id *autoreleaseFullPage(id obj, AutoreleasePoolPage *page)
{
    // The hot page is full. 
    // Step to the next non-full page, adding a new page if necessary.
    // Then add the object to that page.
    // 如果 hotpage 满了，转到下一个未满的 page，如果不存在的话添加一个新的 page。  
    // 然后把 object 添加到新 page 里。
    
    // page 必须是 hotPage
    ASSERT(page == hotPage());
    // page 满了，或者自动释放池按顺序弹出时暂停，并允许堆调试器跟踪自动释放池
    
    // OPTION( DebugPoolAllocation,
    //         OBJC_DEBUG_POOL_ALLOCATION,
    //         "halt when autorelease pools are popped out of order,
    //          and allow heap debuggers to track autorelease pools")
    // 自动释放池按顺序弹出时暂停，并允许堆调试器跟踪自动释放池
    
    ASSERT(page->full()  ||  DebugPoolAllocation);

    // do while 循环里面分为两种情况
    // 1. 沿着 child 往前走，如果能找到一个非满的 page，则可以把 obj 放进去
    // 2. 如果 child 不存在或者所有的 child 都满了，
    //    则构建一个新的 AutoreleasePoolPage 并拼接在 AutoreleasePool 的双向链表中，
    //    并把 obj 添加进新 page 里面
    do {
        if (page->child) page = page->child;
        else page = new AutoreleasePoolPage(page);
    } while (page->full());

    // 设置 page 为 hotPage
    setHotPage(page);
    
    // 把 obj 添加进 page 里面，返回值是 next 之前指向的位置 (objc_object **)
    return page->add(obj);
}
```
### autoreleaseNoPage
```c++
static __attribute__((noinline))
id *autoreleaseNoPage(id obj)
{
    // "No page" could mean no pool has been pushed or an empty
    // placeholder pool has been pushed and has no contents yet
    // "No page" 可能意味着没有构建任何池，或者只有一个 EMPTY_POOL_PLACEHOLDER 占位
    
    // hotPage 不存在，否则执行断言
    ASSERT(!hotPage());

    bool pushExtraBoundary = false;
    if (haveEmptyPoolPlaceholder()) {
        // 如果线程里面存储的是 EMPTY_POOL_PLACEHOLDER
        
        // We are pushing a second pool over the empty placeholder pool
        // or pushing the first object into the empty placeholder pool.
        // 我们正在将第二个池推入空的占位符池，或者将第一个对象推入空的占位符池。
        // Before doing that, push a pool boundary on behalf of the
        // pool that is currently represented by the empty placeholder.
        // 在此之前，代表当前由空占位符表示的池来推动池边界
        
        pushExtraBoundary = true;
    }
    else if (obj != POOL_BOUNDARY  &&  DebugMissingPools) {
        // OPTION( DebugMissingPools, OBJC_DEBUG_MISSING_POOLS,
        // "warn about autorelease with no pool in place, which may be a leak")
        // 警告在没有自动释放池的情况下进行 autorelease，
        // 这可能导致内存泄漏（可能是因为没有释放池，然后对象缺少一次 objc_release 执行，导致内存泄漏）
        // 如果 obj 不为 nil 并且 DebugMissingPools。
        
        // We are pushing an object with no pool in place,
        // and no-pool debugging was requested by environment.
        // 我们正在没有自动释放池的情况下把一个对象往池里推，
        // 并且打开了 environment 的 no-pool debugging，此时会在控制台给一个提示信息。
        
        // 线程内连 EMPTY_POOL_PLACEHOLDER 都没有存储，并且如果 DebugMissingPools 打开了，则控制台输出如下信息
        _objc_inform("MISSING POOLS: (%p) Object %p of class %s "
                     "autoreleased with no pool in place - "
                     "just leaking - break on "
                     "objc_autoreleaseNoPool() to debug", 
                     objc_thread_self(), (void*)obj, object_getClassName(obj));
                     
        // obj 不为 nil，并且线程内连 EMPTY_POOL_PLACEHOLDER 都没有存储
        // 执行 objc_autoreleaseNoPool，且它是个 hook 函数             
        objc_autoreleaseNoPool(obj);
        
        // 返回 nil
        return nil;
    }
    else if (obj == POOL_BOUNDARY  &&  !DebugPoolAllocation) {
        // OPTION( DebugPoolAllocation, OBJC_DEBUG_POOL_ALLOCATION, 
        //         "halt when autorelease pools are popped out of order, 
        // and allow heap debuggers to track autorelease pools")
        // 当自动释放池顺序弹出时暂停，并允许堆调试器跟踪自动释放池
        // 如果 obj 为空，并且没有打开 DebugPoolAllocation
        
        // We are pushing a pool with no pool in place,
        // and alloc-per-pool debugging was not requested.
        // 在没有池的情况下，我们设置一个空池占位，并且不要求为池分配空间和调试。（空池占位只是一个 ((id*)1)）
        
        // Install and return the empty pool placeholder.
        
        // 根据 key 在当前线程的存储空间内保存 EMPTY_POOL_PLACEHOLDER 占位
        return setEmptyPoolPlaceholder();
    }

    // We are pushing an object or a non-placeholder'd pool.
    // 构建非占位的池

    // Install the first page.
    // 构建自动释放池的第一个真正意义的 page
    
    AutoreleasePoolPage *page = new AutoreleasePoolPage(nil);
    
    // 设置为 hotPage
    setHotPage(page);
    
    // Push a boundary on behalf of the previously-placeholder'd pool.
    // 代表先前占位符的池推边界。
    
    // 如果之前有一个 EMPTY_POOL_PLACEHOLDER 在当前线程的存储空间里面占位的话
    if (pushExtraBoundary) {
        // 池边界前进一步
        
        // 可以理解为把 next 指针往前推进了一步，并在 next 之前的指向下放了一个 nil 
        page->add(POOL_BOUNDARY);
    }
    
    // Push the requested object or pool.
    // 把 objc 放进自动释放池
    return page->add(obj);
}
```
### autoreleaseNewPage
```c++
static __attribute__((noinline))
id *autoreleaseNewPage(id obj)
{
    AutoreleasePoolPage *page = hotPage();
    // 如果 hotPage 存在则调用 autoreleaseFullPage 把 obj 放进 page 里面
    if (page) return autoreleaseFullPage(obj, page);
    // 如果 hotPage 不存在，则调用 autoreleaseNoPage 把 obj 放进自动释放池（进行新建 page）
    else return autoreleaseNoPage(obj);
}
```
**下面进入 AutoreleasePoolPage 的 public 部分：**

### autorelease
```c++
static inline id autorelease(id obj)
{
    // 如果对象不存在则执行断言
    ASSERT(obj);
    
    // 如果对象是 Tagged Pointer 则执行断言
    ASSERT(!obj->isTaggedPointer());
    
    // 调用 autoreleaseFast(obj) 函数，把 obj 快速放进自动释放池
    id *dest __unused = autoreleaseFast(obj);
    
    // 1. if (obj != POOL_BOUNDARY  &&  DebugMissingPools) 时 return nil
    // 2. if (obj == POOL_BOUNDARY  &&  !DebugPoolAllocation) 时 return EMPTY_POOL_PLACEHOLDER
    // 3. *dest == obj 正常添加
    
    ASSERT(!dest  ||  dest == EMPTY_POOL_PLACEHOLDER  ||  *dest == obj);
    
    return obj;
}
```
### push
&emsp;如果自动释放池不存在，构建一个新的 `page`。`push` 函数的作用可以理解为，调用 `AutoreleasePoolPage::push` 在当前线程的存储空间保存一个 `EMPTY_POOL_PLACEHOLDER`。 

&emsp;`autoreleaseFast` 函数比 `autoreleaseNewPage` 多了一个 `page` 还不满时就直接添加 `obj` 到 `page` 中，剩下的调用 `autoreleaseFullPage` 或者 `autoreleaseNoPage` 是一样的。

```c++
static inline void *push() 
{
    id *dest;
    if (slowpath(DebugPoolAllocation)) {
        // OPTION( DebugPoolAllocation, OBJC_DEBUG_POOL_ALLOCATION,
        // "halt when autorelease pools are popped out of order, 
        // and allow heap debuggers to track autorelease pools")
        // 当自动释放池弹出顺序时停止，并允许堆调试器跟踪自动释放池
        
        // Each autorelease pool starts on a new pool page.
        // 每个自动释放池从一个新的 page 开始
        // 调用 autoreleaseNewPage
        dest = autoreleaseNewPage(POOL_BOUNDARY);
        
    } else {
    
        // 构建一个占位池
        dest = autoreleaseFast(POOL_BOUNDARY);
    }
    
    ASSERT(dest == EMPTY_POOL_PLACEHOLDER || *dest == POOL_BOUNDARY);
    
    return dest;
}
```
### badPop
```c++
__attribute__((noinline, cold))
static void badPop(void *token)
{
    // Error. For bincompat purposes this is not fatal in executables built with old SDKs.
    // 出于 bin 的兼容目的，不能在旧 SDKs 上构建和执行，否则 _objc_fatal。

    if (DebugPoolAllocation || sdkIsAtLeast(10_12, 10_0, 10_0, 3_0, 2_0)) {
        // OBJC_DEBUG_POOL_ALLOCATION or new SDK. Bad pop is fatal.
        // 无效或者过早释放的自动释放池。
        _objc_fatal("Invalid or prematurely-freed autorelease pool %p.", token);
    }

    // Old SDK. Bad pop is warned once.
    // 如果是 旧 SDKs，发生一次警告。
    
    // 静态局部变量，保证下面的 if 只能进入一次
    static bool complained = false;
    if (!complained) {
        complained = true; // 置为 true
        _objc_inform_now_and_on_crash
            ("Invalid or prematurely-freed autorelease pool %p. "
             "Set a breakpoint on objc_autoreleasePoolInvalid to debug. "
             "Proceeding anyway because the app is old "
             "(SDK version " SDK_FORMAT "). Memory errors are likely.",
                 token, FORMAT_SDK(sdkVersion()));
    }
    
    // 执行最开始的 hook
    objc_autoreleasePoolInvalid(token);
}
```

### popPage/popPageDebug
```c++
// 这里有一个模版参数 (bool 类型的 allowDebug)，
// 直接传值，有点类似 sotreWeak 里的新值和旧值的模版参数
// 这个 void *token 的参数在函数实现里面没有用到....
template<bool allowDebug>
static void
popPage(void *token, AutoreleasePoolPage *page, id *stop)
{
    // OPTION( PrintPoolHiwat, OBJC_PRINT_POOL_HIGHWATER, 
    // "log high-water marks for autorelease pools")
    // 打印自动释放池的 high-water 标记
    // 如果允许 debug 并且打开了 OBJC_PRINT_POOL_HIGHWATER，则打印自动释放池的 hiwat（high-water “最高水位”）
    if (allowDebug && PrintPoolHiwat) printHiwat();

    // 把 stop 后面添加进自动释放池的对象全部执行一次 objc_release 操作
    page->releaseUntil(stop);

    // memory: delete empty children
    // 删除空的 page
    // OPTION( DebugPoolAllocation, OBJC_DEBUG_POOL_ALLOCATION,
    // "halt when autorelease pools are popped out of order, 
    // and allow heap debuggers to track autorelease pools")
    // 当自动释放池弹出顺序时停止，并允许堆调试器跟踪自动释放池

    if (allowDebug && DebugPoolAllocation  &&  page->empty()) {
        // 如果允许 Debug 且开启了 DebugPoolAllocation 并且 page 是空的 
        
        // special case: delete everything during page-per-pool debugging
        // 特殊情况：删除每个 page 调试期间的所有内容

        AutoreleasePoolPage *parent = page->parent;
        
        // 把 page 以及 page 之后增加的 page 都执行 delete
        page->kill();
        
        // 把 page 的 parent 设置为 hotPage
        setHotPage(parent);
    } else if (allowDebug && DebugMissingPools  &&  page->empty()  &&  !page->parent) {
        //OPTION( DebugMissingPools, OBJC_DEBUG_MISSING_POOLS, 
        // "warn about autorelease with no pool in place, which may be a leak")
        // 警告自动释放没有池占位， 这可能是一个泄漏

        // special case: delete everything for pop(top) when debugging missing autorelease pools
        // 在调试缺少自动释放池时，删除 pop（顶部）的所有内容

        // 把 page 以及 page 之后增加的 page 都执行 delete
        page->kill();
        
        // 设置 hotPage 为 nil 
        setHotPage(nil);
    } else if (page->child) {
        // 如果 page 的 child 存在
        
        // hysteresis: keep one empty child if page is more than half full
        // 如果 page 存储的自动释放对象超过了一半，则保留一个 empty child
        
        if (page->lessThanHalfFull()) {
            // 如果 page 内部保存的自动释放对象的数量少于一半
            
            // 把 page 以及 page 之后增加的 page 都执行 delete
            page->child->kill();
        }
        else if (page->child->child) {
            // 如果 page 的 child 的 child 存在
            // 则把 page->child->child 以及它之后增加的 page 全部执行 delete
            page->child->child->kill();
        }
    }
}
```
```c++
// __attribute__((cold)) 表示函数不经常调用
__attribute__((noinline, cold))
static void
popPageDebug(void *token, AutoreleasePoolPage *page, id *stop)
{
    // 模版参数 allowDebug 传递的是 true
    popPage<true>(token, page, stop);
}
```
### pop
```c++
static inline void
pop(void *token)
{
    AutoreleasePoolPage *page;
    id *stop;

    if (token == (void*)EMPTY_POOL_PLACEHOLDER) {
        // Popping the top-level placeholder pool.
        // 弹出顶级 EMPTY_POOL_PLACEHOLDER 占位符池
        
        // 取出 hotPage
        page = hotPage();
        if (!page) {
            // 如果 hotPage 不存在，则表示目前就一 EMPTY_POOL_PLACEHOLDER，说明池还没有使用过
            // Pool was never used. Clear the placeholder.
            // Pool 从未使用过。清除占位符。
            return setHotPage(nil);
        }
        // Pool was used. Pop its contents normally.
        // Pool 是使用过了。正常弹出其内容。
        // Pool pages remain allocated for re-use as usual.
        // Pool pages 保持分配以照常使用.
        
        // 第一个 page
        page = coldPage();
        // 把第一个 page 的 begin 赋值给 token
        token = page->begin();
    } else {
        // token 转为 page 
        page = pageForPointer(token);
    }
    
    stop = (id *)token;
    if (*stop != POOL_BOUNDARY) {
        if (stop == page->begin()  &&  !page->parent) {
            // Start of coldest page may correctly not be POOL_BOUNDARY:
            // 1. top-level pool is popped, leaving the cold page in place
            // 2. an object is autoreleased with no pool
        } else {
            // Error. For bincompat purposes this is
            // not fatal in executables built with old SDKs.
            return badPop(token);
        }
    }
    
    // allowDebug 为 true
    if (slowpath(PrintPoolHiwat || DebugPoolAllocation || DebugMissingPools)) {
        return popPageDebug(token, page, stop);
    }
    
    // 释放对象删除 page
    return popPage<false>(token, page, stop);
}
```
### init
```c++
static void init()
{
    // key tls_dealloc 释放对象删除 page
    int r __unused = pthread_key_init_np(AutoreleasePoolPage::key, 
                                         AutoreleasePoolPage::tls_dealloc);
    ASSERT(r == 0);
}
```
### print
&emsp;打印当前 `page` 里面的 `autorelease` 对象。
```c++
__attribute__((noinline, cold))
void print()
{
    // 打印 hotPage 和 coldPage
    _objc_inform("[%p]  ................  PAGE %s %s %s", this, 
                 full() ? "(full)" : "", 
                 this == hotPage() ? "(hot)" : "", 
                 this == coldPage() ? "(cold)" : "");
    check(false);
    
    // 打印当前池里的 autorelease 对象
    for (id *p = begin(); p < next; p++) {
        if (*p == POOL_BOUNDARY) {
            _objc_inform("[%p]  ################  POOL %p", p, p);
        } else {
            _objc_inform("[%p]  %#16lx  %s", 
                         p, (unsigned long)*p, object_getClassName(*p));
        }
    }
}
```
### printAll
&emsp;打印自动释放池里面的所有 `autorelease` 对象。
```c++
__attribute__((noinline, cold))
static void printAll() // 这是一个静态非内联并较少被调用的函数
{
    _objc_inform("##############");
    // 打印自动释放池所处的线程
    _objc_inform("AUTORELEASE POOLS for thread %p", objc_thread_self());

    AutoreleasePoolPage *page;
    
    // 统计自动释放池里面的所有对象
    ptrdiff_t objects = 0;
    // coldePage 是第一个 page
    // 沿着 child 指针一直向前，遍历所有的 page
    for (page = coldPage(); page; page = page->child) {
        // 这里是把每个 page 里的 autorelease 对象的数量全部加起来
        objects += page->next - page->begin();
    }
    
    // 打印自动释放池里面等待 objc_release 的所有 autorelease 对象的数量 
    _objc_inform("%llu releases pending.", (unsigned long long)objects);

    if (haveEmptyPoolPlaceholder()) {
        // 如果目前只是空占位池的话，打印空池
        _objc_inform("[%p]  ................  PAGE (placeholder)", 
                     EMPTY_POOL_PLACEHOLDER);
        _objc_inform("[%p]  ################  POOL (placeholder)", 
                     EMPTY_POOL_PLACEHOLDER);
    }
    else {
        // 循环打印每个 page 里面的 autorelease 对象
        for (page = coldPage(); page; page = page->child) {
            page->print();
        }
    }

    // 打印分割线
    _objc_inform("##############");
}
```
### printHiwat
&emsp;打印 `high-water`。
```c++
__attribute__((noinline, cold))
static void printHiwat()
{
    // Check and propagate high water mark Ignore high
    // water marks under 256 to suppress noise.
    // 检查并传播 high water 忽略 256 以下的 high water 以抑制噪音。
    
    // hotPage
    AutoreleasePoolPage *p = hotPage();
    
    // COUNT 固定情况下是 4096 / 8 = 512
    // p->depth 是 hotPage 的深度，第一个 page 的 depth 是 0，
    // 然后每次增加一个 page 该 page 的 depth 加 1
    // p->next - p->begin() 是该 page 内存储的 autorelease 对象的个数
    // 那么 mark 大概就是从第一个 page 到 hotpage 的 page 
    // 的数量乘以 512 然后加上 hotPage 里面保存的 autorelease 对象的数量
    
    uint32_t mark = p->depth*COUNT + (uint32_t)(p->next - p->begin());
    // 如果 mark 大于 p->hiwat 并且 mark 大于 256
    if (mark > p->hiwat  &&  mark > 256) {
        // 沿着 parent 链遍历每个 page，把每个 page 的 hiwat 置为 mark
        for( ; p; p = p->parent) {
            // 可读可写
            p->unprotect();
            
            // 修改 hiwat 为 mark
            p->hiwat = mark;
            
            // 只可读
            p->protect();
        }

        _objc_inform("POOL HIGHWATER: new high water mark of %u "
                     "pending releases for thread %p:",
                     mark, objc_thread_self());

        void *stack[128];
        
        // int backtrace(void**,int) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
        // 函数原型
        // #include <execinfo.h> 
        // int backtrace(void **buffer, int size);
        // 该函数获取当前线程的调用堆栈，获取的信息将会被存放在 buffer 中，
        // 它是一个指针数组，参数 size 用来指定 buffer
        // 中可以保存多少个 void * 元素。函数的返回值是实际返回的 void * 元素个数。
        // buffer 中的 void * 元素实际是从堆栈中获取的返回地址。 
        
        int count = backtrace(stack, sizeof(stack)/sizeof(stack[0]));
        
        // char** backtrace_symbols(void* const*,int) __OSX_AVAILABLE_STARTING(__MAC_10_5, __IPHONE_2_0);
        // 函数原型
        // char **backtrace_symbols(void *const *buffer, int size);
        // 该函数将 backtrace 函数获取的信息转化为一个字符串数组，
        // 参数 buffer 是 backtrace 获取的堆栈指针，
        // size 是 backtrace 返回值。
        // 函数返回值是一个指向字符串数组的指针，它包含 char* 元素个数为 size。
        // 每个字符串包含了一个相对于 buffer 中对应元素的可打印信息，
        // 包括函数名、函数偏移地址和实际返回地址。
        // backtrace_symbols 生成的字符串占用的内存是 malloc 出来的，
        // 但是是一次性 malloc 出来的，释放是只需要一次性释放返回的二级指针即可
        
        char **sym = backtrace_symbols(stack, count);
        
        for (int i = 0; i < count; i++) {
            _objc_inform("POOL HIGHWATER:     %s", sym[i]);
        }
        free(sym);
    }
}
```
### #undef POOL_BOUNDARY
```c++
#undef POOL_BOUNDARY
```
&emsp;至此 `AuroreleasePoolPage` 代码就全部看一遍了，所有的实现点都很清晰了。

## 参考链接
**参考链接:🔗**
+ [黑幕背后的Autorelease](http://blog.sunnyxx.com/2014/10/15/behind-autorelease/)
+ [GCC扩展 __attribute__ ((visibility("hidden")))](https://www.cnblogs.com/lixiaofei1987/p/3198665.html)
+ [Linux下__attribute__((visibility ("default")))的使用](https://blog.csdn.net/fengbingchun/article/details/78898623)
+ [backtrace函数](https://www.cnblogs.com/fangyan5218/p/10686488.html)
+ [操作系统内存管理(思维导图详解)](https://blog.csdn.net/hguisu/article/details/5713164)
+ [关于自动释放池的底层实现](https://blog.csdn.net/ZCMUCZX/article/details/80040910)

