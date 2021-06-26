# iOS 从源码解析Runtime (十)：聚焦cache_t objc-msg-arm64.s补充(3)

> 由于上篇字数的限制，剩余最后的几个函数只能在此新开一篇了。

#### objc_msgSend
&emsp;终于来到了我们最核心的 `objc_msgSend` 函数。
```c++
/*
 *
 * 函数声明，两个函数入参一样，一个是执行函数并返回 id 返回值。
 * 一个则是在 self 中找到指定 SEL 的 IMP。
 * id objc_msgSend(id self, SEL _cmd, ...);
 * IMP objc_msgLookup(id self, SEL _cmd, ...);
 * 
 * objc_msgLookup ABI:
 * IMP returned in x17 找到的 IMP 保存在 x17 寄存器中（然后通过 br x17 执行该 IMP）
 * x16 reserved for our use but not used x16 寄存器则是保留寄存器
 *
 */

#if SUPPORT_TAGGED_POINTERS
    .data // 数据内容
    .align 3 // 2^3 = 8 字节对齐
    
    // 定义一个全局的标记 _objc_debug_taggedpointer_classes
    .globl _objc_debug_taggedpointer_classes 
_objc_debug_taggedpointer_classes:
    
    // .fill repeat, size, value 含义是反复拷贝 size 个字节，重复 repeat 次，
    // 其中 size 和 value 是可选的，默认值分别是 1 和 0 
    // 全部填充 0
    
    .fill 16, 8, 0
    
    // 同上
    // 定义一个全局标记 _objc_debug_taggedpointer_ext_classes
    .globl _objc_debug_taggedpointer_ext_classes
_objc_debug_taggedpointer_ext_classes:
    
    // .fill repeat, size, value 含义是反复拷贝 size 个字节，重复 repeat 次，
    // 其中 size 和 value 是可选的，默认值分别是 1 和 0 
    // 全部填充 0
    
    .fill 256, 8, 0
    
#endif

    // 来了 来了
    // 这里用了 ENTRY
    // 回顾我们前面的 ENTRY 定义
    
    // .macro ENTRY /* name */
    // .text
    // .align 5
    // .globl    $0
    // $0:
    // .endmacro
    
    // $0 表示是 _objc_msgSend
    // 那么整体的含义就是：
    // _objc_msgSend 是一个代码段，然后是 2^5 = 32 个字节对齐
    // 然后用 global 修饰，大概可以理解是一个全局函数。（大概这样）
    
    ENTRY _objc_msgSend
    // UNWIND 展开信息生成，无窗口
    UNWIND _objc_msgSend, NoFrame
    
    // p0 和 空 对比，即判断接收者是否存在，
    // 其中 p0 是 objc_msgSend 的第一个参数，
    // 消息接收者 receiver 
    cmp    p0, #0            // nil check and tagged pointer check
    
#if SUPPORT_TAGGED_POINTERS
    // le 小于，支持 tagged pointer 的流程
    // 跳转到 LNilOrTagged 标签处执行 Taggend Pointer 对象的函数查找及执行
    b.le    LNilOrTagged        //  (MSB tagged pointer looks negative)
#else
    // p0 等于 0 的话，则跳转到 LReturnZero 标签处
    // 置 0 返回 nil 并直接结束 _objc_msgSend 函数
    b.eq    LReturnZero
#endif
    
    // p0 即 receiver 肯定存在的流程，实际规定是 p0 - p7 是接收函数参数的寄存器  
    // 根据对象拿出 isa，即从 x0 寄存器指向的地址取出 isa，存入 p13 寄存器  
    ldr    p13, [x0]        // p13 = isa
    
    // GetClassFromIsa_p16：从 isa 中获取类指针并存放在通用寄存器 p16 中
    //  and p16, $0, #ISA_MASK
    // 在 __LP64__ 下通过 p16 = isa(正是 p13) & ISA_MASK，拿出 shiftcls 信息，得到 class 信息
    
    GetClassFromIsa_p16 p13        // p16 = class
    
// 本地标签（表示获得 isa 完成）    
LGetIsaDone:
    // calls imp or objc_msgSend_uncached   
    // CacheLookup 前面已经详细解析过
    
    // 如果有 isa，走到 CacheLookup 即缓存查找流程，也就是所谓的 sel-imp 快速查找流程，
    // NORMAL 和 _objc_msgSend
    CacheLookup NORMAL, _objc_msgSend

#if SUPPORT_TAGGED_POINTERS
LNilOrTagged:
    // nil 检测，如果是 nil 的话也跳转到 LReturnZero 标签处
    // 置 0 返回 nil 并结束 _objc_msgSend 函数
    b.eq    LReturnZero        // nil check

    // tagged
    
    // 将 _objc_debug_taggedpointer_classes 所在的页的基址读入 x10 寄存器
    adrp    x10, _objc_debug_taggedpointer_classes@PAGE
    
    // x10 = x10 + _objc_debug_taggedpointer_classes(page 中的偏移量) 
    // x10 基址根据偏移量进行内存偏移 
    add    x10, x10, _objc_debug_taggedpointer_classes@PAGEOFF
    
    // 无符号位域提取指令
    // UBFX Wd, Wn, #lsb, #width ; 32-bit
    // UBFX Xd, Xn, #lsb, #width ; 64-bit
    // 作用：从 Wn 寄存器的第 lsb 位开始，提取 width 位到 Wd 寄存器，剩余高位用 0 填充
    
    // 从 x0 的第 60 位开始，
    // 提取 4 位到 x11 寄存器，其它位用 0 填充
    ubfx    x11, x0, #60, #4
    
    // 读出 taggedn pointer 所属的类并保存在 x16 中
    ldr    x16, [x10, x11, LSL #3]
    
    adrp    x10, _OBJC_CLASS_$___NSUnrecognizedTaggedPointer@PAGE
    add    x10, x10, _OBJC_CLASS_$___NSUnrecognizedTaggedPointer@PAGEOFF
    
    cmp    x10, x16
    
    // 跳转到 LGetIsaDone 标签
    b.ne    LGetIsaDone

    // ext tagged
    adrp    x10, _objc_debug_taggedpointer_ext_classes@PAGE
    add    x10, x10, _objc_debug_taggedpointer_ext_classes@PAGEOFF
    
    ubfx    x11, x0, #52, #8
    ldr    x16, [x10, x11, LSL #3]
    
    // 跳转到 LGetIsaDone 标签
    b    LGetIsaDone
// SUPPORT_TAGGED_POINTERS
#endif

// 传入 nil 对象是：
LReturnZero:
    // x0 is already zero x0已经为零
    // 置 0 
    mov    x1, #0
    movi    d0, #0
    movi    d1, #0
    movi    d2, #0
    movi    d3, #0
    
    // return 结束执行
    ret 
    
    // LExit 结束 _objc_msgSend 函数执行
    END_ENTRY _objc_msgSend
```
#### _objc_msgLookup
&emsp;查找 `IMP`。
```c++
    // _objc_msgLookup 函数实现部分
    ENTRY _objc_msgLookup
    UNWIND _objc_msgLookup, NoFrame
    // 同上 nil 检测
    cmp    p0, #0            // nil check and tagged pointer check
#if SUPPORT_TAGGED_POINTERS
    // 在 tagged pointer 中查找 IMP
    b.le    LLookup_NilOrTagged    //  (MSB tagged pointer looks negative)
#else
    // 如果找不到对应的 IMP 则返回 nil 
    b.eq    LLookup_Nil
#endif
    
    // x0 第一个参数（isa），保存在 p13 寄存器中
    ldr    p13, [x0]        // p13 = isa
    // 根据 p13 取出对应的类保存在 p16 中
    GetClassFromIsa_p16 p13        // p16 = class
LLookup_GetIsaDone:
    // returns imp
    // 查找 imp 并返回
    CacheLookup LOOKUP, _objc_msgLookup

// Tagged Pointer 的查找过程（首先找到所属的类）
#if SUPPORT_TAGGED_POINTERS
LLookup_NilOrTagged:
    b.eq    LLookup_Nil    // nil check

    // tagged
    adrp    x10, _objc_debug_taggedpointer_classes@PAGE
    add    x10, x10, _objc_debug_taggedpointer_classes@PAGEOFF
    ubfx    x11, x0, #60, #4
    ldr    x16, [x10, x11, LSL #3]
    adrp    x10, _OBJC_CLASS_$___NSUnrecognizedTaggedPointer@PAGE
    add    x10, x10, _OBJC_CLASS_$___NSUnrecognizedTaggedPointer@PAGEOFF
    cmp    x10, x16
    b.ne    LLookup_GetIsaDone

LLookup_ExtTag:    
    adrp    x10, _objc_debug_taggedpointer_ext_classes@PAGE
    add    x10, x10, _objc_debug_taggedpointer_ext_classes@PAGEOFF
    ubfx    x11, x0, #52, #8
    ldr    x16, [x10, x11, LSL #3]
    b    LLookup_GetIsaDone
// SUPPORT_TAGGED_POINTERS
#endif

// 未找到 IMP 的话
LLookup_Nil:
    // 将 __objc_msgNil 所在的页的基址读入 x17 寄存器 
    adrp    x17, __objc_msgNil@PAGE
    
    // x17 = x17 + __objc_msgNil(page 中的偏移量) 
    // x17 基址根据偏移量进行内存偏移 
    add    x17, x17, __objc_msgNil@PAGEOFF
    
    // return 结束执行
    ret
    
    // _objc_msgLookup 内容结束
    END_ENTRY _objc_msgLookup

```
#### __objc_msgNil
&emsp;未找到 `IMP` 时的结束工作。
```c++
    // 私有静态函数
    STATIC_ENTRY __objc_msgNil

    // x0 is already zero
    // x0 已经为零
    // 置 0
    
    mov    x1, #0
    movi    d0, #0
    movi    d1, #0
    movi    d2, #0
    movi    d3, #0
    
    // return 结束函数
    ret
    
    // 结束 __objc_msgNil 函数
    END_ENTRY __objc_msgNil

```
#### _objc_msgSendSuper/_objc_msgSendSuper2/_objc_msgLookupSuper2
&emsp;即我们日常使用的 `[super xxxxx]` 函数调用，它的第一个参数 `receiver` 同样是我们的 `self` 并不是 `self` 的父类，而它与 `_objc_msgSend` 仅有的不同的地方就是 `_objc_msgSendSuper` 直接去父类中查找。
```c++
    // _objc_msgSendSuper 函数
    ENTRY _objc_msgSendSuper
    UNWIND _objc_msgSendSuper, NoFrame
    
    // x0 是第一个参数 isa，这里有点迷，是把 [x0] 同时放入 p0 和 x16 吗
    ldp    p0, p16, [x0]        // p0 = real receiver, p16 = class
    
    // calls imp or objc_msgSend_uncached
    // 同 _objc_msgSend 执行 imp
    CacheLookup NORMAL, _objc_msgSendSuper

    END_ENTRY _objc_msgSendSuper

    // no _objc_msgLookupSuper

    ENTRY _objc_msgSendSuper2
    UNWIND _objc_msgSendSuper2, NoFrame

    ldp    p0, p16, [x0]        // p0 = real receiver, p16 = class
    
    // p16 偏移后，现在是 superclass (objc_class 的第二个成员变量是 superclass，第一个 isa 则指向 元类)
    ldr    p16, [x16, #SUPERCLASS]    // p16 = class->superclass
    
    // 执行 imp
    CacheLookup NORMAL, _objc_msgSendSuper2

    END_ENTRY _objc_msgSendSuper2

    // _objc_msgLookupSuper2 在父类中查找
    ENTRY _objc_msgLookupSuper2
    UNWIND _objc_msgLookupSuper2, NoFrame

    ldp    p0, p16, [x0]        // p0 = real receiver, p16 = class
    
    // 找到 superclass
    ldr    p16, [x16, #SUPERCLASS]    // p16 = class->superclass
    
    // 查找
    CacheLookup LOOKUP, _objc_msgLookupSuper2

    END_ENTRY _objc_msgLookupSuper2
```
#### MethodTableLookup
```c++
.macro MethodTableLookup
    
    // push frame
    SignLR
    stp    fp, lr, [sp, #-16]!
    mov    fp, sp

    // save parameter registers: x0..x8, q0..q7
    // 保存方法参数到寄存器中
    
    sub    sp, sp, #(10*8 + 8*16)
    stp    q0, q1, [sp, #(0*16)]
    stp    q2, q3, [sp, #(2*16)]
    stp    q4, q5, [sp, #(4*16)]
    stp    q6, q7, [sp, #(6*16)]
    stp    x0, x1, [sp, #(8*16+0*8)]
    stp    x2, x3, [sp, #(8*16+2*8)]
    stp    x4, x5, [sp, #(8*16+4*8)]
    stp    x6, x7, [sp, #(8*16+6*8)]
    str    x8,     [sp, #(8*16+8*8)]

    // lookUpImpOrForward(obj, sel, cls, LOOKUP_INITIALIZE | LOOKUP_RESOLVER)
    // receiver and selector already in x0 and x1
    // receiver 和 selector 已经在 x0 和 x1 中了 
    
    mov    x2, x16
    mov    x3, #3
    
    // 如果缓存中未找到，则跳转到 _lookUpImpOrForward（c 函数） 去方法列表中去找函数，
    // objc-runtime-new.mm 中的 lookUpImpOrForward 函数过于简单，就不在这里展开了。
    bl    _lookUpImpOrForward

    // IMP in x0
    mov    x17, x0
    
    // restore registers and return
    // 恢复寄存器并返回
    
    ldp    q0, q1, [sp, #(0*16)]
    ldp    q2, q3, [sp, #(2*16)]
    ldp    q4, q5, [sp, #(4*16)]
    ldp    q6, q7, [sp, #(6*16)]
    ldp    x0, x1, [sp, #(8*16+0*8)]
    ldp    x2, x3, [sp, #(8*16+2*8)]
    ldp    x4, x5, [sp, #(8*16+4*8)]
    ldp    x6, x7, [sp, #(8*16+6*8)]
    ldr    x8,     [sp, #(8*16+8*8)]

    // 处理栈顶和栈底
    mov    sp, fp
    ldp    fp, lr, [sp], #16
    
    AuthenticateLR

.endmacro

    // __objc_msgSend_uncached
    
    STATIC_ENTRY __objc_msgSend_uncached
    UNWIND __objc_msgSend_uncached, FrameWithNoSaves

    // THIS IS NOT A CALLABLE C FUNCTION
    // Out-of-band p16 is the class to search
    
    MethodTableLookup
    TailCallFunctionPointer x17

    END_ENTRY __objc_msgSend_uncached

    STATIC_ENTRY __objc_msgLookup_uncached
    UNWIND __objc_msgLookup_uncached, FrameWithNoSaves

    // THIS IS NOT A CALLABLE C FUNCTION
    // Out-of-band p16 is the class to search
    
    MethodTableLookup
    ret

    END_ENTRY __objc_msgLookup_uncached

    // _cache_getImp
    STATIC_ENTRY _cache_getImp

    // p16 中保存 class
    GetClassFromIsa_p16 p0
    CacheLookup GETIMP, _cache_getImp

LGetImpMiss:
    // p0 置 0
    mov    p0, #0
    // return 结束函数
    ret
    
    // _cache_getImp 函数结束
    END_ENTRY _cache_getImp
```
#### _objc_msgForward
```c++
/*
*
* id _objc_msgForward(id self, SEL _cmd,...);
*
* _objc_msgForward is the externally-callable function returned
by things like method_getImplementation().
* _objc_msgForward是 由 method_getImplementation() 之类的东西返回的外部调用函数。
*
* _objc_msgForward_impcache is the function pointer
* actually stored in method caches.
* _objc_msgForward_impcache 是​​实际存储在方法缓存中的函数指针。
*
*/

    STATIC_ENTRY __objc_msgForward_impcache

    // No stret specialization.
    b    __objc_msgForward

    END_ENTRY __objc_msgForward_impcache
    
    // __objc_msgForward 函数
    ENTRY __objc_msgForward
    
    // 将 __objc_forward_handler 所在的页的基址读入 x10 寄存器
    adrp    x17, __objc_forward_handler@PAGE
    ldr    p17, [x17, __objc_forward_handler@PAGEOFF]
    
    // $0 = function pointer value
    // br    $0
    // 跳转到 x17
    TailCallFunctionPointer x17
    
    END_ENTRY __objc_msgForward
    
    // 不同的跳转实现
    
    ENTRY _objc_msgSend_noarg
    // _objc_msgSend
    b    _objc_msgSend
    END_ENTRY _objc_msgSend_noarg

    ENTRY _objc_msgSend_debug
    // _objc_msgSend
    b    _objc_msgSend
    END_ENTRY _objc_msgSend_debug

    ENTRY _objc_msgSendSuper2_debug
    // _objc_msgSendSuper2
    b    _objc_msgSendSuper2
    END_ENTRY _objc_msgSendSuper2_debug

    
    ENTRY _method_invoke
    // x1 is method triplet instead of SEL
    // x1 是方法三元祖而不是 SEL
    
    add    p16, p1, #METHOD_IMP
    ldr    p17, [x16]
    ldr    p1, [x1, #METHOD_NAME]
    
    // 方法跳转
    TailCallMethodListImp x17, x16
    
    END_ENTRY _method_invoke

#endif
```
## 参考链接
**参考链接:🔗**
+ [方法查找流程 objc_msg_arm64.s](https://www.jianshu.com/p/a8668b81c5d6)
+ [OC 底层探索 09、objc_msgSend 流程 1-缓存查找](https://www.cnblogs.com/zhangzhang-y/p/13704597.html)
+ [汇编指令解读](https://blog.csdn.net/peeno/article/details/53068412)
+ [objc-msg-arm64源码深入分析](https://www.jianshu.com/p/835ae53372ba)
+ [汇编语言学习笔记](https://chipengliu.github.io/2019/04/05/asm-note/)
+ [iOS汇编教程：理解ARM](https://www.jianshu.com/p/544464a5e630)
+ [汇编跳转指令B、BL、BX、BLX 和 BXJ的区别](https://blog.csdn.net/bytxl/article/details/49883103)
+ [iOS开发同学的arm64汇编入门](https://blog.cnbluebox.com/blog/2017/07/24/arm64-start/)
+ [C语言栈区的讲解(基于ARM)以及ARM sp,fp寄存器的作用](https://blog.csdn.net/Lcloud671/article/details/78232401)
+ [.align 5的是多少字节对齐](https://blog.csdn.net/yunying_si/article/details/9736173?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522160185721219724839257560%2522%252C%2522scm%2522%253A%252220140713.130102334..%2522%257D&request_id=160185721219724839257560&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~all~first_rank_v2~rank_v28-3-9736173.pc_first_rank_v2_rank_v28&utm_term=.align+&spm=1018.2118.3001.4187)
+ [解读objc_msgSend](https://www.jianshu.com/p/75a4737741fd)
+ [ARM汇编指令](https://blog.csdn.net/qq_27522735/article/details/75043870)
+ [翻译-为什么objc_msgSend必须用汇编实现](http://tutuge.me/2016/06/19/translation-why-objcmsgsend-must-be-written-in-assembly/)
+ [iOS runtime 底层详解、内部原理、场景应用](https://www.it610.com/article/1278625871489417216.htm)
+ [iOS - Runtime 中 Class、消息机制、super 关键字](https://www.jianshu.com/p/2faae9f0dcb3)
+ [深入iOS系统底层之汇编语言](https://juejin.im/post/6844903560140816398)
+ [操作系统内存管理(思维导图详解)](https://blog.csdn.net/hguisu/article/details/5713164)
+ [ARM指令浅析2（adrp、b）](https://blog.csdn.net/liao392781/article/details/79162919?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522160193907419724839222280%2522%252C%2522scm%2522%253A%252220140713.130102334..%2522%257D&request_id=160193907419724839222280&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~all~first_rank_v2~rank_v28-2-79162919.pc_first_rank_v2_rank_v28&utm_term=adrp&spm=1018.2118.3001.4187)
+ [Arm64汇编：UBFX指令](https://blog.csdn.net/LQMIKU/article/details/104361219?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522160194046219725271750548%2522%252C%2522scm%2522%253A%252220140713.130102334..%2522%257D&request_id=160194046219725271750548&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~all~first_rank_v2~rank_v28-1-104361219.pc_first_rank_v2_rank_v28&utm_term=ubfx&spm=1018.2118.3001.4187)
+ [第9部分- Linux ARM汇编 语法](https://blog.csdn.net/notbaron/article/details/106578015?biz_id=102&utm_term=汇编%20UXTW&utm_medium=distribute.pc_search_result.none-task-blog-2~all~sobaiduweb~default-2-106578015&spm=1018.2118.3001.4187)
+ [CBZ和CBNZ指令使用说明《realview编译工具》](http://blog.sina.com.cn/s/blog_3fd4da4a0102vvyg.html)
+ [二、linux 内核OOPS（1）](https://blog.csdn.net/qq_20678703/article/details/52237784?utm_medium=distribute.pc_aggpage_search_result.none-task-blog-2~all~sobaiduend~default-1-52237784.nonecase&utm_term=oops出错是什么意思&spm=1000.2123.3001.4430)
+ [BRAA, BRAAZ, BRAB, BRABZ](https://developer.arm.com/docs/ddi0596/h/base-instructions-alphabetic-order/braa-braaz-brab-brabz-branch-to-register-with-pointer-authentication)
