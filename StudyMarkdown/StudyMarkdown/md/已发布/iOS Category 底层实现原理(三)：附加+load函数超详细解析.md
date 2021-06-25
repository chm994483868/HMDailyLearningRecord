# iOS Category 底层实现原理(三)：附加+load函数超详细解析

## +load 函数分析
&emsp;既然写到这里了，那我们就顺便把 `+load` 函数的调用流程分析一下吧。

+ 实现 `+load` 的分类和类是非懒加载分类和非懒加载类，未实现 `+load` 函数的分类和类，是懒加载分类和懒加载类。懒加载类只有我们第一次用到它们的时候，才会执行实现。
+ `load` 函数执行是直接由其函数地址直接调用的，不是走 `msgSend` 的函数查找流程的，所以类和分类中的 `load` 函数是完全不存在 “覆盖” 行为的。它们都会执行，执行的流程的话：首先是 类一定早于分类的，然后父类一定早于子类，分类之间则是谁先编译则谁先执行。（这里刚好和不同分类中的同名函数，后编译的分类中的函数会 “覆盖” 先编译的分类相反。）
+ 正常情况我们都不应该手动调用 `load` 函数，我们只要要交给系统自己等待调用即可，且全局只会调用一次。

### 再探 load_images
&emsp;`load_images` 函数正是用来是负责调用指定的 `mach_header` 中的类和分类的 `+load` 函数的。比较 `objc4-799.1`，发现  `objc4-781` 中的 `load_images` 多了一个作用，多了如下 4 行代码。
```c++
 if (!didInitialAttachCategories && didCallDyldNotifyRegister) {
     didInitialAttachCategories = true;
     loadAllCategories();
 }
```
&emsp;在 `781` 中 `apple` 把加载 `category` 的数据延后了，之前 `799.1` 中是在 `_read_images` 中加载的。前面我们已经对此过程分析过了，下面我们主要把目光聚焦它的下半部分：
```c++
/*
* load_images
* Process +load in the given images which are being mapped in by dyld.
* 处理由 dyld 映射的给定的镜像中的类和分类中的 +load 函数。
* Locking: write-locks runtimeLock and loadMethodLock
*/
// C 语言前向声明，这两个函数我们下面再一行一行分析 
// 判断 mach_header 中是否包含非懒加载的类和分类（即实现了 load 函数的类和分类）
extern bool hasLoadMethods(const headerType *mhdr);
// 调用 load 函数前的准备
extern void prepare_load_methods(const headerType *mhdr);

void load_images(const char *path __unused, const struct mach_header *mh)
{
    if (!didInitialAttachCategories && didCallDyldNotifyRegister) {
        didInitialAttachCategories = true;
        loadAllCategories();
    }

    // Return without taking locks if there are no +load methods here.
    // 如果没有 load 函数则直接返回。
    if (!hasLoadMethods((const headerType *)mh)) return;

    // 加锁，且这是一个递归锁
    recursive_mutex_locker_t lock(loadMethodLock);

    // Discover load methods
    // 发现 load methods
    {
        mutex_locker_t lock2(runtimeLock);
        // 准备 load 函数
        prepare_load_methods((const headerType *)mh);
    }

    // Call +load methods (without runtimeLock - re-entrant)
    // 调用 +load 函数
    call_load_methods();
}
```

### hasLoadMethods
```c++
// Quick scan for +load methods that doesn't take a lock.
// 为了快速扫描 +load 函数没有进行加锁。
bool hasLoadMethods(const headerType *mhdr)
{
    size_t count;
    
    // GETSECT(_getObjc2NonlazyClassList, classref_t const, "__objc_nlclslist");
    // 判断 DATA 段下 __objc_nlclslist 区有没有非懒加载 类
    
    if (_getObjc2NonlazyClassList(mhdr, &count)  &&  count > 0) return true;
    
    // GETSECT(_getObjc2NonlazyCategoryList, category_t * const, "__objc_nlcatlist");
    // 判断 DATA 段下 __objc_nlcatlist 区有没有非懒加载 分类
    
    if (_getObjc2NonlazyCategoryList(mhdr, &count)  &&  count > 0) return true;
    
    return false;
}
```

### prepare_load_methods
```c++
void prepare_load_methods(const headerType *mhdr)
{
    size_t count, i;

    runtimeLock.assertLocked();
    
    // 取出 mhdr 中的 count 个非懒加载类
    // 取分类在下面，从这里已经可以看出 load 函数调用类早于分类
    classref_t const *classlist = 
        _getObjc2NonlazyClassList(mhdr, &count);
        
    // 循环把不同类的 load 函数添加进全局的 load 方法的数组中，且首先递归添加父类的 +load 函数
    for (i = 0; i < count; i++) {
        schedule_class_load(remapClass(classlist[i]));
    }
    
    // 取出 mhdr 中的 count 个非懒加载分类
    category_t * const *categorylist = _getObjc2NonlazyCategoryList(mhdr, &count);
    
    for (i = 0; i < count; i++) {
        // 取得 category_t
        category_t *cat = categorylist[i];
        // 取得该分类对应的类
        Class cls = remapClass(cat->cls);
        // 如果类不存在则 continue
        if (!cls) continue;  // category for ignored weak-linked class
        
        // Swift 类的延展和分类中不允许有 +load 方法
        if (cls->isSwiftStable()) {
            _objc_fatal("Swift class extensions and categories on Swift "
                        "classes are not allowed to have +load methods");
        }
        
        // 如果该类没有实现，则实现该类，realizeClassWithoutSwift 函数结尾会追加 category 数据到本类中。
        // 如果分类中有 +load 函数，那本类中的 +load 函数会不会被分类中的 +load 函数 "覆盖" 呢？
        // 其实是会覆盖的 😂
        
        // 可以验证，准备父类和子类和各自的一个分类全部都实现 +load 函数，
        // 然后在子类的 +load 函数中加一句 [super load] 调用，
        // 打印的话会发现执行的是父类的分类里面的 +load 函数
        // +load 的这个覆盖流程和其他在分类中写和本类同名的自定义函数是完全一样的。
        
        // 之所以系统自动调用时 +load 函数时没有显现 "覆盖" 特性，主要在下下面一句，
        // 下下面是直接把 cat 中的 +load 函数添加进了全局的 loadable_classes 数组中的，
        // 然后 add_class_to_loadable_list 函数中添加类的 +load 函数到全局的 loadable_classes 数组中，
        // 类查找 +load 函数是查找的 ro()->baseMethods() 函数列表，分类中的函数是被追加到了 rw 中。
        
        realizeClassWithoutSwift(cls, nil);
        // 如果还是未实现则断言
        ASSERT(cls->ISA()->isRealized());
        
        // 把 cat 中的 +load 函数添加进全局的 loadable_classes 数组
        add_category_to_loadable_list(cat);
    }
}
```

### schedule_class_load
```c++
/*
* prepare_load_methods
* Schedule +load for classes in this image, any un-+load-ed 
* superclasses in other images, and any categories in this image.
* 调度该镜像中类的 +load 函数，和其 un-+load-ed 的其他镜像中的父类，
* 以及该镜像中的任何 categories 中的 +load 函数。
*/
// Recursively schedule +load for cls and any un-+load-ed superclasses.
// 为 cls 和其 un-+load-ed 的其他镜像中的父类递归调度 +load 函数。
// cls must already be connected.
// cls 必须已经链接。
static void schedule_class_load(Class cls)
{
    if (!cls) return;
    // 如果 cls 未实现，则直接断言，
    // 正常情况下 _read_imags 以后所有的非懒加载类都应该已经实现了
    ASSERT(cls->isRealized());  // _read_images should realize

    // #define RW_LOADED (1<<23)
    // 如果 cls 的 +load 已被调用过了，则 return
    // 系统只会自动对每个类的 load 函数调用一次
    if (cls->data()->flags & RW_LOADED) return;

    // Ensure superclass-first ordering
    // 递归调度父类，保证父类的 load 函数先执行
    schedule_class_load(cls->superclass);

    // 把 cls 的 +load 函数放进一个全局的 loadable_class 数组中
    // 保存 cls 和 cls 的 +load 函数
    add_class_to_loadable_list(cls);
    
    // 设置 cls 的标志位为已经调用过 +load 函数
    cls->setInfo(RW_LOADED); 
}
```

### add_class_to_loadable_list
```c++

struct loadable_class {
    Class cls;  // may be nil
    IMP method;
};

// List of classes that need +load called (pending superclass +load)
// 需要 +load 调用的类的列表（待处理超类+ load）
// This list always has superclasses first because of the way it is constructed
// 由于其构造方式，该列表始终具有超类。

// 静态全局的 loadable_class 数组
static struct loadable_class *loadable_classes = nil;
// 记录 loadable_classes 数组已使用容量
static int loadable_classes_used = 0;
// 记录 loadable_classes 数组的总容量
static int loadable_classes_allocated = 0;

/*
* add_class_to_loadable_list
* Class cls has just become connected. Schedule it for +load if
* it implements a +load method.
* cls 类刚刚建立联系。如果该 cls 实现了 +load 函数则调度它的 +load 函数。
* 其实是把该 +load 函数放在一个全局的函数数组中。
*/
void add_class_to_loadable_list(Class cls)
{
    // 临时变量，用了接收 cls 的 +load 函数
    IMP method;
    // 加锁
    loadMethodLock.assertLocked();
    
    // 取得 cls 的 +load 函数
    method = cls->getLoadMethod();
    if (!method) return;  // Don't bother if cls has no +load method
    
    // log
    if (PrintLoading) {
        _objc_inform("LOAD: class '%s' scheduled for +load", 
                     cls->nameForLogging());
    }
    
    // 如果全局的 loadable_classes 数组为空或者已满，则扩容
    if (loadable_classes_used == loadable_classes_allocated) {
        // 看到起始长度是 16，以后则在 16 的基础上每次扩大 2 倍
        // （16）（48 = 16 * 2 + 16）（112 = 48 * 2 + 16）（240 = 112 * 2 + 16）
        // 16 的倍数：1 3 7 15 31 63...
        loadable_classes_allocated = loadable_classes_allocated*2 + 16;
        
        // 只是扩充容量，并不对数据的顺序做任何处理
        loadable_classes = (struct loadable_class *)
            realloc(loadable_classes,
                              loadable_classes_allocated *
                              sizeof(struct loadable_class));
    }
    
    // 然后把类和 +load 函数放在数组
    loadable_classes[loadable_classes_used].cls = cls;
    loadable_classes[loadable_classes_used].method = method;
    // 自增 1
    loadable_classes_used++;
}
```

### getLoadMethod
```c++
/*
* objc_class::getLoadMethod
* fixme
* Called only from add_class_to_loadable_list.
* 该函数的执行仅来自 add_class_to_loadable_list 函数的调用
* 
* Locking: runtimeLock must be read- or write-locked by the caller.
*/
IMP 
objc_class::getLoadMethod()
{
    // 加锁
    runtimeLock.assertLocked();
    // 临时变量报错函数列表
    const method_list_t *mlist;

    ASSERT(isRealized());
    ASSERT(ISA()->isRealized());
    ASSERT(!isMetaClass());
    ASSERT(ISA()->isMetaClass());
    
    // 注意这里取的是 ro()->baseMethods() 的函数列表
    // 它里面保存的 +load 函数只会来自类定义时候的 +load 函数
    // 分类中的 +load 函数是被追加到 rw 中的
    
    mlist = ISA()->data()->ro()->baseMethods();
    
    if (mlist) {
        for (const auto& meth : *mlist) {
            const char *name = sel_cname(meth.name);
            if (0 == strcmp(name, "load")) {
                return meth.imp;
            }
        }
    }

    return nil;
}
```
&emsp;注意这里取的是 `ro()->baseMethods()` 的函数列表
它里面保存的 `+load` 函数只会来自类定义中的 `+load` 函数实现，分类中的 `+load` 函数是被追加到 `rw` 中的。

### add_category_to_loadable_list
```c++

struct loadable_category {
    Category cat;  // may be nil
    IMP method;
};

// List of categories that need +load called (pending parent class +load)
// 需要 +load 调用的类别列表（待处理的父类 +load）
static struct loadable_category *loadable_categories = nil;
// 记录 loadable_categories 数组已使用的容量
static int loadable_categories_used = 0;
// 记录 loadable_categories 数组的总容量
static int loadable_categories_allocated = 0;

/*
* add_category_to_loadable_list
* Category cat's parent class exists and the category has been attached to its class. 
* Category cat 的本类存在，并且这 category 的数据已经附加到该类上。
*
* Schedule this category for +load after its parent class 
* becomes connected and has its own +load method called.
* 在其所属类建立连接并调用其自己的 +load方法之后，才会调度分类自己的 +load 函数。
*
*/
void add_category_to_loadable_list(Category cat)
{
    IMP method;
    
    // 加锁
    loadMethodLock.assertLocked();
    
    // 从 cat 的 classMethods 中找到 +load 函数
    method = _category_getLoadMethod(cat);

    // Don't bother if cat has no +load method
    if (!method) return;

    // log
    if (PrintLoading) {
        _objc_inform("LOAD: category '%s(%s)' scheduled for +load", 
                     _category_getClassName(cat), _category_getName(cat));
    }
    
    // 下面和 add_class_to_loadable_list 函数模式完全相同，把 cat 和 load 函数的 IMP 添加到 loadable_categories 数组中
    if (loadable_categories_used == loadable_categories_allocated) {
        loadable_categories_allocated = loadable_categories_allocated*2 + 16;
        loadable_categories = (struct loadable_category *)
            realloc(loadable_categories,
                              loadable_categories_allocated *
                              sizeof(struct loadable_category));
    }

    loadable_categories[loadable_categories_used].cat = cat;
    loadable_categories[loadable_categories_used].method = method;
    loadable_categories_used++;
}
```
### call_load_methods
```c++
/*
* call_load_methods
* Call all pending class and category +load methods.
* 调用所有待处理的类和类别的 +load 函数。
*
* Class +load methods are called superclass-first. 
* 父类的 +load 函数调用是优先的。
*
* Category +load methods are not called until after the parent class's +load.
* 直到 Category 所属类的 +load 调用之后才调用 Category 中的 +load 方法。
* 
* This method must be RE-ENTRANT, because a +load could trigger more image mapping. 
* 此方法必须是 RE-ENTRANT，因为 +load 可能触发更多 镜像被映射，然后镜像又有 +load 需要执行。
*
* In addition, the superclass-first ordering must be preserved in the face of re-entrant calls. 
* 此外，面对重入调用，仍然保留 superclass 优先调用。
*
* Therefore, only the OUTERMOST call of this function will do anything, 
* and that call will handle all loadable classes, even those generated while it was running.
* 因此，仅 OUTERMOST 调用会执行任何操作，该调用将处理所有可加载的类，甚至那些在运行时生成的。
*
* The sequence below preserves +load ordering in the face of image loading during a +load, 
* and make sure that no +load method is forgotten because it was added during a +load call.
* 下面的序列在执行 +load 函数时保持镜像加载时的 +load 顺序，并确保没有忘记 +load方法，因为它是在 +load 调用期间添加的。
*
* Sequence:
* 1. Repeatedly call class +loads until there aren't any more
* 1. 重复调用 class 的 +loads，直到没有更多
* 2. Call category +loads ONCE.
* 2. 调用 category 中的 +load
* 3. Run more +loads if:
* 3. 运行 +load 函数的其他情况：
*    (a) there are more classes to load, OR
*    (a) 还有更多要加载的类，
*    (b) there are some potential category +loads that have still never been attempted.
*    (b) 有些潜在的类别 +load 函数没有被附加。
* Category +loads are only run once to ensure "parent class first" ordering,
* even if a category +load triggers a new loadable class and a new loadable category attached to that class. 
* category 的 +load 函数调用不同于 class，直接一个 for 循环，category 这样（ ONCE ）是为了保证 class 的 load 函数优先于 category 的 load 函数。
* 即使类别 +load 触发了一个新的可加载类，并且该类附加了一个新的可加载类别，此时要保证 类 优先于 分类调用 +load 函数。
* 
* Locking: loadMethodLock must be held by the caller 
*   All other locks must not be held.
*/
void call_load_methods(void)
{
    static bool loading = NO;
    bool more_categories;

    // 加锁
    loadMethodLock.assertLocked();

    // Re-entrant calls do nothing; the outermost call will finish the job.
    // Re-entrant 是无效的，必须等最外层调用完成。
    if (loading) return;
    loading = YES;

    // 创建一个自动释放池
    // push
    void *pool = objc_autoreleasePoolPush();

    do {
        // 1. Repeatedly call class +loads until there aren't any more
        // 1. 重复调用 class 的 +loads，直到没有更多
        while (loadable_classes_used > 0) {
            call_class_loads();
        }

        // 2. Call category +loads ONCE
        // 2. 调用分类的 +load 函数
        more_categories = call_category_loads();

        // 3. Run more +loads if there are classes OR more untried categories
        // 3. 如果有类或更多未调用的类别，则执行 +load 
    } while (loadable_classes_used > 0  ||  more_categories);
    
    // pop
    objc_autoreleasePoolPop(pool);

    loading = NO;
}
```

### call_class_loads
```c++
/*
* call_class_loads
* Call all pending class +load methods.
* 调用所有待处理的类的 +load 方法。
*
* If new classes become loadable, +load is NOT called for them.
* 如果新类成为 loadable，则不会为它们调用 +load。
*
* Called only by call_load_methods().
* 此函数仅被 call_load_methods() 调用。
*/
static void call_class_loads(void)
{
    int i;
    
    // Detach current loadable list.
    // 分离当前的可加载列表。
    // 用一个临时变量接收 loadable_classes
    struct loadable_class *classes = loadable_classes;
    // 取出占用
    int used = loadable_classes_used;
    
    // 把 "原始数据" 置 nil 置 0
    // 置为 nil
    loadable_classes = nil;
    // 置 0
    loadable_classes_allocated = 0;
    loadable_classes_used = 0;
    
    // Call all +loads for the detached list.
    // 循环调用 +load 函数
    for (i = 0; i < used; i++) {
        // 取得所属类
        Class cls = classes[i].cls;
        // 取得函数 IMP，并赋给一个函数指针
        load_method_t load_method = (load_method_t)classes[i].method;
        
        if (!cls) continue; 
        // log
        if (PrintLoading) {
            _objc_inform("LOAD: +[%s load]\n", cls->nameForLogging());
        }
        
        // 通过函数指针执行 load 
        (*load_method)(cls, @selector(load));
    }
    
    // Destroy the detached list.
    if (classes) free(classes);
}
```

### call_category_loads
```c++
/*
* call_category_loads
* Call some pending category +load methods.
* 调用类别的 +load 方法.
*
* The parent class of the +load-implementing categories has all of its categories attached, in case some are lazily waiting for +initalize.
* 实现了 +load 函数的分类会附加到其所属的的类中，如果该类是懒加载的。
* 
* Don't call +load unless the parent class is connected.
* 除非分类所属类已链接，否则不要调用 +load。
*
* If new categories become loadable, +load is NOT called, and they are
* added to the end of the loadable list, and we return TRUE.
* 如果新类别变为可加载，则不直接调用 +load，会将它们添加到可加载列表的末尾，并且返回 TRUE。
* 
* Return FALSE if no new categories became loadable.
* 如果没有新类别可加载，则返回 FALSE。
*
* Called only by call_load_methods().
* 仅被 call_load_methods() 调用。
*
*/
static bool call_category_loads(void)
{
    int i, shift;
    bool new_categories_added = NO;
    
    // Detach current loadable list.
    // 用临时变量接收 loadable_categories 的数据，并把 loadable_categories 相关数据置为初始态
    struct loadable_category *cats = loadable_categories;
    int used = loadable_categories_used;
    int allocated = loadable_categories_allocated;
    
    // 置为 nil，置为 0
    loadable_categories = nil;
    loadable_categories_allocated = 0;
    loadable_categories_used = 0;

    // Call all +loads for the detached list.
    // 循环调用 cats 中 cat 的 load 函数
    for (i = 0; i < used; i++) {
        // 取得 cat 
        Category cat = cats[i].cat;
        // 取得 load 的 IMP，并赋给一个函数指针
        load_method_t load_method = (load_method_t)cats[i].method;
        
        Class cls;
        if (!cat) continue;
        
        // 取得分类所属的 cls
        cls = _category_getClass(cat);
        
        // 如果 cls 存在 且 isLoadable 是 true 
        // isLoadable 默认为 true
        //    bool isLoadable() {
        //        ASSERT(isRealized());
        //        return true;  // any class registered for +load is definitely loadable
        //    }
        
        if (cls  &&  cls->isLoadable()) {
            // log
            if (PrintLoading) {
                _objc_inform("LOAD: +[%s(%s) load]\n", 
                             cls->nameForLogging(), 
                             _category_getName(cat));
            }
            // load 函数执行
            (*load_method)(cls, @selector(load));
            
            // 把 cat 置为 nil 
            cats[i].cat = nil;
        }
    }

    // Compact detached list (order-preserving)
    // 把 cats 中 cat 不是 nil 的元素集中到数组左边
    shift = 0;
    for (i = 0; i < used; i++) {
        if (cats[i].cat) {
            cats[i-shift] = cats[i];
        } else {
            shift++;
        }
    }
    
    // shift 记录 cats 数组中元素的 cat 为 nil 的元素个数
    // used 表示 cats 目前实际占用容量
    used -= shift;

    // Copy any new +load candidates from the new list to the detached list.
    // 将所有新的 +load 候选者 从 loadable_categories 复制到 cats。
    // new_categories_added 记录是否有 category 需要调用 +load 
    new_categories_added = (loadable_categories_used > 0);
    for (i = 0; i < loadable_categories_used; i++) {
        // 扩容
        if (used == allocated) {
            allocated = allocated*2 + 16;
            cats = (struct loadable_category *)
                realloc(cats, allocated *
                                  sizeof(struct loadable_category));
        }
        // 赋值
        // usee 自增
        cats[used++] = loadable_categories[i];
    }

    // Destroy the new list.
    // 把 loadable_categories 的数据复制到 cats 中以后，
    // 释放 loadable_categories
    if (loadable_categories) free(loadable_categories);

    // Reattach the (now augmented) detached list. 
    // But if there's nothing left to load, destroy the list.
    // 如果目前 cats 有内容，则把 cats 的内容都放回 loadable_categories
    if (used) {
        loadable_categories = cats;
        loadable_categories_used = used;
        loadable_categories_allocated = allocated;
    } else {
        // 如果没有到话，释放 cats
        if (cats) free(cats);
        // 把 loadable_categories 置 nil 置 0
        loadable_categories = nil;
        loadable_categories_used = 0;
        loadable_categories_allocated = 0;
    }

    // log
    if (PrintLoading) {
        if (loadable_categories_used != 0) {
            _objc_inform("LOAD: %d categories still waiting for +load\n",
                         loadable_categories_used);
        }
    }
    
    // new_categories_added = (loadable_categories_used > 0);
    return new_categories_added;
}
```
&emsp;觉得把注释都读完，`+load` 的脉络能极其清晰，这里就不再总结了。

## 参考链接
**参考链接:🔗**
+ [结合 category 工作原理分析 OC2.0 中的 runtime](http://www.cocoachina.com/articles/17293)
+ [深入理解Objective-C：Category](https://tech.meituan.com/2015/03/03/diveintocategory.html)
+ [iOS 捋一捋Category加载流程及+load](https://www.jianshu.com/p/fd176e806cf3)
+ [iOS开发之runtime（17）：_dyld_objc_notify_register方法介绍](https://xiaozhuanlan.com/topic/6453890217)
+ [iOS开发之runtime(27): _read_images 浅析](https://xiaozhuanlan.com/topic/1452730698)
