#  volatile关键字使用
## volatile
> **`volatile` 是一个类型修饰符。`volatile` 的作用是作为指令关键字，**确保本条指令不会因编译器的优化而省略**.**

## volatile 的特性
+ 保证了不同线程对这个变量进行操作时的可见性，即一个线程修改了某个变量的值，这新值对其他线程来说是立即可见的。（实现可见性）
+ 禁止进行指令重排序。（实现有序性）
+ `volatile` 只能保证对单次读/写的原子性。`i++` 这种操作不能保证原子性。
关于`volatile` 原子性可以理解为把对`volatile`变量的单个读/写，看成是使用同一个锁对这些单个读/写操作做了同步，就跟下面的`SoWhat`跟`SynSoWhat`功能类似哦。
```
class SoWhat {
    volatile int i = 0; // volatile 修饰的变量
    public int getI() {
        return i; // 单个 volatile 变量的读
    }
    
    public void setI(int j) {
        this.i = j; // 单个 volatile 变量的写
    }
    
    public void inc() {
        i++; // 复合多个 volatile 变量
    }
}

class SynSoWhat {
    int i = 0;
    public synchronized int getI() {
        return i;
    }

    public synchronized void setI(int j) {
        this.i = j;
    }

    public void inc() { 
        // 普通方法调用
        int tmp = getI();
        // 调用已同步方法
        tmp = tmp + 1;
        // 普通写方法
        setI(tmp);
        // 调用已同步方法
    }
}
```
## 为什么用 `volatile`?
`C/C++` 中的 `volatile `关键字和 `const` 对应，用来修饰变量，通常用于建立语言级别的 `memory barrie`r。这是 `BS` 在 《The C++ Programming Language》对 `volatile` 修饰词的说明：

> A volatile specifier is a hint to a compiler that an object may change its value in ways not specified by the language so that aggressive optimizations must be avoided.

`volatile` 关键字是一种类型修饰符，用它声明的类型变量表示可以被某些编译器未知的因素更改，比如：操作系统、硬件或者其它线程等。遇到这个关键字声明的变量，**编译器对访问该变量的代码就不再进行优化，从而可以提供对特殊地址的稳定访问**。声明时语法：`int volatile vInt;` 当要求使用 `volatile` 声明的变量的值的时候，系统总是重新从它所在的内存读取数据，即使它前面的指令刚刚从该处读取过数据。而且读取的数据立刻被保存。例如：
```c
vliatile int i = 10;
int a = i;
...
// 其他代码，并未明确告诉编译器，对 i 进行过操作
int b = i;
```
`volatile` 指出 `i` 是随时可能发生变化的，每次使用它的时候必须从 `i` 的地址中读取，因而编译器生成的汇编代码会重新从 `i` 的地址读取数据放在 `b` 中。而优化做法是，由于编译器发现两次从 `i` 读数据的代码之间的代码没有对 `i` 进行过操作，**它会自动把上次读的数据放在 `b` 中。而不是重新从 `i` 里面读**。这样以来，如果 `i` 是一个寄存器变量或者表示一个端口数据就容易出错，所以说 `volatile` 可以保证对特殊地址的稳定访问。
```c
// 此代码在 xcode 下无法运行
int i = 10;
int a = i;
printf("i = %d\n", a);

// 下面汇编语句的作用就是改变内存中 i 的值
// 但是又不让编译器知道
__asm {
    mov dword ptr [ebp-4], 20h
}
int b = i;
printf("i = %d\n", b);
// 会打印:
i = 10;
i = 10;

// 使用 volatile 后
// 会打印:
i = 10;
i = 32;
```
其实不只是 **内嵌汇编操纵栈** 这种方式属于 **编译无法识别的变量改变**，另外更多的可能是**多线程并发访问共享变量时**，一个线程改变了变量的值，怎样让改变后的值对其它线程 visible。一般说来，volatile用在如下的几个地方：
1. 中断服务程序中修改的供其它程序检测的变量需要加 `volatile`；
2. 多任务环境下各任务间共享的标志应该加 `volatile`；
3. 存储器映射的硬件寄存器通常也要加 `volatile` 说明，因为每次对它的读写都可能由不同意义；

## volatile 指针
和 `const` 修饰词类似，`const` 有常量指针和指针常量的说法，`volatile` 也有相应的概念。
修饰由指针指向的对象、数据是 `const` 或 `volatile` 的：(指向的值不可变)
```c
const char* cpch;
volatile char* vpch;
```
指针自身的值——一个代表地址的整数变量，是 `const` 或 `volatile` 的：（指向不可变）
```c
char* const pchc;
char* volatile pchv;
```
**注意:**
1. 可以把一个 `非volatile int` 赋给 `volatile int`，但是不能把 `非volatile` 对象赋给一个 `volatile` 对象。
2. 除了基本类型外，对用户定义类型也可以用 `volatile` 类型进行修饰。
3. `C++`中一个有 `volatile` 标识符的类只能访问它接口的子集，一个由类的实现者控制的子集。用户只能用 `const_cast` 来获得对类型接口的完全访问。此外，`volatile` 向 `const` 一样会从类传递到它的成员。
## 多线程下的 volatile
有些变量是用 `volatile` 关键字声明的。当两个线程都要用到某一个变量且该变量的值会被改变时，应该用 `volatile` 声明，该关键字的作用是防止优化编译器把变量从内存装入 `CPU` 寄存器中。如果变量被装入寄存器，那么两个线程有可能一个使用内存中的变量，一个使用寄存器中的变量，这会造成程序的错误执行。`volatile` 的意思是让编译器每次操作该变量时一定要从内存中真正取出，而不是使用已经存在寄存器中的值。
如下示例:
```c
volatile  BOOL  bStop  =  FALSE;
```
1. 在一个线程中:
```c
while ( !bStop) { ... }
bStop = FALSE;
return;
```
2. 在另外一个线程，要终止上面的线程循环:
```c
// 等待上面的线程终止，如果 bStop 不使用 volatile 修饰，那么这个循环将是一个死循环，因为 bStop 已经读取到了寄存器中，寄存器中 bStop 的值永远不会变成 FALSE，加上 volatile，程序在执行时，每次均从内存中读出 bStop 的值，就不会死循环了。
bStop = TRUE;
while ( bStop ); 
```
**这个关键字是用来设定某个对象的存储位置在内存中，而不是寄存器中**。因为一般的**对象编译器**可能会**将其的拷贝放在寄存器中**用以**加快指令的执行速度**，如下代码：
```c
...  
int  nMyCounter  =  0;  
for(;  nMyCounter<100;nMyCounter++)  
{  
...  
}  
...
```
在此段代码中，`nMyCounter ` 的拷贝可能存放到某个寄存器中（循环中，对 `nMyCounter` 的测试及操作总是对此寄存器中的值进行），但是另外又有段代码执行了这样的操作：`nMyCounter -= 1`; 这个操作中，对 `nMyCounter` 的改变是对内存中的 `nMyCounter` 进行操作，于是出现了这样一个现象：**nMyCounter 的改变不同步**。

**参考链接:🔗**
[C/C++ 中 volatile 关键字详解](https://www.runoob.com/w3cnote/c-volatile-keyword.html)
