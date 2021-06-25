#  iOS底层runtime应用之动态添加属性:成员变量

## 抛出结论
1. 编译完成的类，**不能**对其添加**变量**，**能**对其添加**属性**。
2. 运行时创建的类，**能**对其添加**变量(注册之前)/属性(任意时刻)**。

```
// 添加一个 nonatomic、copy 修饰符的 NSString
void hh_class_addProperty(Class targetClass, const char* propertyName) {
    objc_property_attribute_t type = {"T", [[NSString stringWithFormat:@"@\"%@\"", NSStringFromClass([NSString class])] UTF8String]};
    objc_property_attribute_t ownership0 = {"C", ""}; // C = copy
    objc_property_attribute_t ownership = {"N", ""}; // N = nonatomic
    objc_property_attribute_t backingivar = {"V", [NSString stringWithFormat:@"_%@", [NSString stringWithCString:propertyName encoding:NSUTF8StringEncoding]].UTF8String};
    objc_property_attribute_t attrs[] = {type, ownership0, ownership, backingivar};
    
    class_addProperty(targetClass, propertyName, attrs, 4); // 4: attrs 元素的个数
}

// 打印属性
void hh_printerProperty(Class targetClass) {
    unsigned int outCount, i;
    objc_property_t *properties = class_copyPropertyList(targetClass, &outCount);
    for (i = 0; i < outCount; ++i) {
        objc_property_t property = properties[i];
        fprintf(stdout, "%s %s\n", property_getName(property), property_getAttributes(property));
    }
}

// 打印成员变量
void hh_printerIvar(Class targetClass) {
    unsigned int count = 0;
    Ivar *ivars = class_copyIvarList(targetClass, &count);
    for (unsigned int i = 0; i < count; ++i) {
        Ivar const ivar = ivars[i];
        const char *cName = ivar_getName(ivar);
        NSString *ivarName = [NSString stringWithUTF8String:cName];
        NSLog(@"ivarName: %@", ivarName);
    }
    
    free(ivars);
    printf("ivar count = %u\n", count);
}
```
## 1、编译时
### 1.1 编译时：添加成员变量

**在编译时不能添加成员变量。**

在类的结构中，类的成员变量存储在 bits ->rw->ro 中，也就是说，编译完成的类，成员变量是 readonly(只读) 属性。只能读取，不能修改。所以不能动态添加。

换个角度来说的话，类在编译完成之后，其结构是一个结构体，系统已经完成了对其空间大小的分配，你如果这个时候再去添加，根本就没有地方给你了，怎么添加呢？

验证：
创建一个类，并对其添加一个成员变量，并打印/赋值:
```
// 创建 LGPerson 类
@interface LGPerson : NSObject
@end

// 在 main 函数添加 _hhName 的成员变量，对 _hhName 赋值并打印
class_addIvar([LGPerson class], "_hhName", sizeof(NSString *), log2(sizeof(NSString *)), "@"); // 添加一个 _hhName 的成员变量

// 打印成员变量
hh_printerIvar([LGPerson class]);

LGPerson *person = [[LGPerson alloc] init];
// 因为是动态添加，所以无法用点语法，用 kvc 的方式赋值
[person setValue:@"这是 _hhName 的 value" forUndefinedKey:@"_hhName"];
// 读取 _hhName 并打印出来
NSLog(@"%@", [person valueForKey:@"_hhName"]);

// 执行结果:
ivar count = 0
*** Terminating app due to uncaught exception 'NSUnknownKeyException', reason: '[<LGPerson 0x10075ac50> setValue:forUndefinedKey:]: this class is not key value coding-compliant for the key _hhName.'
```
从打印结果来看，说明这个 _hhName 成员变量没有添加成功，所以 ivar 的 count = 0，并且对 _hhName 进行 kvc 赋值的时候，报了 `forUndefinedKey` 错误。

### 1.2 编译时：添加属性
```
hh_class_addProperty([LGPerson class], "hhName"); // 添加属性
hh_printerProperty([LGPerson class]); // 打印属性
hh_printerIvar([LGPerson class]); // 打印成员变量

// 打印结果:
hhName T@"NSString",C,N,V_hhName // 属性添加成功
ivar count = 0 // 属性添加成功，但是没有自动添加相应的成员变量
```
> 从打印结果看，属性添加成功，只是没有自动添加一个带下划线的成员变量。如果要使用该属性的话还需要给它添加 setter 和 getter 方法才行。而运行时添加的成员变量可以直接用 kvc 的方式修改和读取。

通过控制台 LLDB 来打印看看 data() 里面究竟有什么内容：
```
(lldb) p [LGPerson class]
(Class) $0 = LGPerson
(lldb) x/5gx $0
0x100002178: 0x0000000100002150 0x00000001003ee140
0x100002188: 0x00000001003e8490 0x0000801000000000
0x100002198: 0x0000000100613594

// 强转为 class_data_bits_t *
(lldb) p (class_data_bits_t *)0x100002198
(class_data_bits_t *) $1 = 0x0000000100002198

(lldb) p $1->data()
(class_rw_t *) $2 = 0x0000000100613590

(lldb) p (class_ro_t *)$2->ro
(class_ro_t *) $3 = 0x00000001000020d8

// 查看 ro 内容
(lldb) p *$3
(class_ro_t) $4 = {
  flags = 128
  instanceStart = 8
  instanceSize = 8
  reserved = 0
  ivarLayout = 0x0000000000000000
  name = 0x0000000100000f92 "LGPerson"
  baseMethodList = 0x0000000000000000
  baseProtocols = 0x0000000000000000
  ivars = 0x0000000000000000
  weakIvarLayout = 0x0000000000000000
  baseProperties = 0x0000000000000000
  _swiftMetadataInitializer_NEVER_USE = {}
}

// 查看 rw 内容
(lldb) p *$2
(class_rw_t) $5 = {
  flags = 2148007936
  version = 0
  witness = 1
  ro = 0x00000001000020d8
  methods = {
    list_array_tt<method_t, method_list_t> = {
       = {
        list = 0x0000000000000000
        arrayAndFlag = 0
      }
    }
  }
  properties = {
    list_array_tt<property_t, property_list_t> = {
       = {
        list = 0x0000000100613620 // 看到 rw 的 properties 中有值
        arrayAndFlag = 4301338144
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
  demangledName = 0x0000000000000000
}
```
从控制台的打印来看，动态添加属性后，在 rw->properties 中有内容，而 rw->ro 中的 ivars、baseProperties 都没有内容。

> 动态添加的属性是可行的，存储在 rw 中。

扩展:
```
// 给 LGPerson 定义时添加一个成员变量 originalVar 和一个属性 originalPro
@interface LGPerson : NSObject{
    NSString *originalVar;
}

@property (nonatomic, copy) NSString *originalPro;
@end 

hh_class_addProperty([LGPerson class], "addPro"); // 添加属性
hh_printerProperty([LGPerson class]); // 打印属性
hh_printerIvar([LGPerson class]); // 打印成员变量

// 打印结果:
// 通过 class_copyPropertyList 得到两个属性，一个是我们动态添加的一个是原始定义的
🏓🏓🏓 addPro T@"NSString",C,N,V_addPro
🏓🏓🏓 originalPro T@"NSString",C,N,V_originalPro

// 首先是成员变量只有定义时的 originalVar 和自动添加的和属性对应的: _originalPro
🏉🏉🏉 ivarName: originalVar
🏉🏉🏉 ivarName: _originalPro

🥋🥋🥋 ivar count = 2
```
验证属性和成员变量都位于哪里：
1. 从 ro 开始：
```
(lldb) p [LGPerson class]
(Class) $0 = LGPerson
(lldb) x/5gx $0 // 打印类里面的内容，根据 struct objc_class 定义可知，下面地址对应 objc_class 的每个成员变量
0x100003248: 0x0000000100003220 (isa) 0x00000001003ef140 (superclass) // 它俩各占 8 个字节 
0x100003258: 0x00000001003e9490 0x0000802400000000 (cacha) // 占 16 字节
0x100003268: 0x0000000100666b24 (bits)  // 占 8 个字节
(lldb) p (class_data_bits_t *)0x100003268 // 强转为 class_data_bit_t * 指针
(class_data_bits_t *) $1 = 0x0000000100003268
(lldb) p $1->data() // 通过 class_data_bit_t 的 data() 函数，取得 class_rw_t *
(class_rw_t *) $3 = 0x0000000100666b20
(lldb) p $3->ro // 取得 class_ro_t *
(const class_ro_t *) $4 = 0x00000001000030e8
(lldb) p *$4 // 打印 class_ro_t 的内容
(const class_ro_t) $5 = {
  flags = 388
  instanceStart = 8
  instanceSize = 24
  reserved = 0
  ivarLayout = 0x0000000100001f6b "\x02"
  name = 0x0000000100001f62 "LGPerson"
  baseMethodList = 0x0000000100003130
  baseProtocols = 0x0000000000000000
  ivars = 0x0000000100003180
  weakIvarLayout = 0x0000000000000000
  baseProperties = 0x00000001000031c8
  _swiftMetadataInitializer_NEVER_USE = {}
}
```
2. 打印 class_ro_t 的内容，看到 `baseMethodList = 0x0000000100003130` 、`ivars = 0x0000000100003180` 和 `baseProperties = 0x00000001000031c8` 是有内容的，那先看 `baseMethodList`:
```
(lldb) p $4->baseMethodList
(method_list_t *const) $6 = 0x0000000100003130
(lldb) p *$6
(method_list_t) $7 = {
  entsize_list_tt<method_t, method_list_t, 3> = {
    entsizeAndFlags = 26
    count = 3 // 表示有 3 个函数
    first = {
      name = "originalPro"
      types = 0x0000000100001f81 "@16@0:8"
      imp = 0x0000000100001c80 (KCObjc`-[LGPerson originalPro])
    }
  }
}
```
3. 看到 `count = 3` 表示有 3 个函数，第一个函数是 `-[LGPerson originalPro]`  这个是 originalPro 属性的 get 函数，接下来我们一个一个打印看剩下的两个函数是什么:
*阅读 `struct entsize_list_tt {...}` 定义，看到可用 `Element& get(uint32_t i) const` 函数来读取每个位置的函数：*
```
(lldb) p $7.get(0)
(method_t) $9 = {
  name = "originalPro"
  types = 0x0000000100001f81 "@16@0:8"
  imp = 0x0000000100001c80 (KCObjc`-[LGPerson originalPro])
}

(lldb) p $7.get(1)
(method_t) $10 = {
  name = "setOriginalPro:"
  types = 0x0000000100001f89 "v24@0:8@16"
  imp = 0x0000000100001cb0 (KCObjc`-[LGPerson setOriginalPro:])
}

(lldb) p $7.get(2)
(method_t) $11 = {
  name = ".cxx_destruct"
  types = 0x0000000100001f79 "v16@0:8"
  imp = 0x0000000100001c40 (KCObjc`-[LGPerson .cxx_destruct])
}
```
4. 看到 3 个函数分别是: originalPro 属性的 get 和 set 函数，以及一个 C++ 的析构函数。

5. 接下来我们查看 ro 的 `ivars = 0x0000000100003180`:
```
(lldb) p $4->ivars
(const ivar_list_t *const) $12 = 0x0000000100003180
(lldb) p *$12
(const ivar_list_t) $13 = {
  entsize_list_tt<ivar_t, ivar_list_t, 0> = {
    entsizeAndFlags = 32
    count = 2
    first = {
      offset = 0x0000000100003210
      name = 0x0000000100001e7f "originalVar"
      type = 0x0000000100001f6d "@\"NSString\""
      alignment_raw = 3
      size = 8
    }
  }
}
```
6. 数据类型依然是 `entsize_list_tt`，看到 count = 2, 且第一个成员变量是 originalVar，接下来我们依次打印:
```
(lldb) p $13.get(0)
(ivar_t) $14 = {
  offset = 0x0000000100003210
  name = 0x0000000100001e7f "originalVar"
  type = 0x0000000100001f6d "@\"NSString\""
  alignment_raw = 3
  size = 8
}
(lldb) p $13.get(1)
(ivar_t) $15 = {
  offset = 0x0000000100003218
  name = 0x0000000100001e8b "_originalPro"
  type = 0x0000000100001f6d "@\"NSString\""
  alignment_raw = 3
  size = 8
}
```
7. 看到成员变量一个是 `originalVar` 第二个是 `_originalPro`，对应原始的属性变量并在名字前面加了下划线。且这个是在类中定义属性时，自动添加的成员变量。

8. 接下来看 `baseProperties = 0x00000001000031c8`:
```
(lldb) p $4->baseProperties
(property_list_t *const) $16 = 0x00000001000031c8
(lldb) p *$16
(property_list_t) $17 = {
  entsize_list_tt<property_t, property_list_t, 0> = {
    entsizeAndFlags = 16
    count = 1
    first = (name = "originalPro", attributes = "T@\"NSString\",C,N,V_originalPro")
  }
}
```
9. 看到只有一个 `originalPro` 属性。对应定义，且动态添加的 `hh_class_addProperty([LGPerson class], "addPro");` 在 ro 中没有找到。

10. 接下来看 class_rw_t 的内容：
```
(lldb) p *$3
(class_rw_t) $18 = {
  flags = 2148007936
  version = 0
  witness = 1
  ro = 0x00000001000030e8
  methods = {
    list_array_tt<method_t, method_list_t> = {
       = {
        list = 0x0000000100003130
        arrayAndFlag = 4294979888
      }
    }
  }
  properties = {
    list_array_tt<property_t, property_list_t> = {
       = {
        list = 0x0000000101117ee1
        arrayAndFlag = 4312891105
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
  demangledName = 0x0000000000000000
}
```
11. 看到 `methods` 和 `properties` 是有值的，首先看一下 `class_rw_t` 中的定义：
```
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
12. 看到 `v.is<class_rw_ext_t *>()` 不为真时，是直接从 `ro` 取得对应的  `baseMethods`、`baseProperties`、`baseProtocols`。

13. 首先看一下 `rw` 的 `methods`:
```
(lldb) p $18.methods
(method_array_t) $20 = {
  list_array_tt<method_t, method_list_t> = {
     = {
      list = 0x0000000100003130
      arrayAndFlag = 4294979888
    }
  }
}
(lldb) p $20.count()
(uint32_t) $22 = 3
(lldb) p $20.beginLists()
(method_list_t **) $23 = 0x0000000100666b30
(lldb) p *$23
(method_list_t *) $24 = 0x0000000100003130
(lldb) p *$24 // 发现是 entsize_list_tt，对应 ro 中数据类型
(method_list_t) $25 = {
  entsize_list_tt<method_t, method_list_t, 3> = {
    entsizeAndFlags = 26
    count = 3
    first = {
      name = "originalPro"
      types = 0x0000000100001f81 "@16@0:8"
      imp = 0x0000000100001c80 (KCObjc`-[LGPerson originalPro])
    }
  }
}
(lldb) p $25.get(0)
(method_t) $26 = {
  name = "originalPro"
  types = 0x0000000100001f81 "@16@0:8"
  imp = 0x0000000100001c80 (KCObjc`-[LGPerson originalPro])
}
(lldb) p $25.get(1)
(method_t) $27 = {
  name = "setOriginalPro:"
  types = 0x0000000100001f89 "v24@0:8@16"
  imp = 0x0000000100001cb0 (KCObjc`-[LGPerson setOriginalPro:])
}
(lldb) p $25.get(2)
(method_t) $29 = {
  name = ".cxx_destruct"
  types = 0x0000000100001f79 "v16@0:8"
  imp = 0x0000000100001c40 (KCObjc`-[LGPerson .cxx_destruct])
}
(lldb) 
```
14. 通过转化发现 `rw->methods` 其实是 `ro->baseMethodList`。

15. 接下来看 `rw` 的 `properties`:
```
p $2->properties
(property_array_t) $4 = {
  list_array_tt<property_t, property_list_t> = {
     = {
      list = 0x0000000101a195d1
      arrayAndFlag = 4322334161
    }
  }
}
(lldb) p $4.array()
(list_array_tt<property_t, property_list_t>::array_t *) $6 = 0x0000000101a195d0
(lldb) p $4.count()
(uint32_t) $7 = 2
(lldb) p $4.begin()
(iterator) $8 = {
  lists = 0x0000000101a195d8
  listsEnd = 0x0000000101a195e8
  m = {
    entsize = 16
    index = 0
    element = 0x0000000101a192b8
  }
  mEnd = {
    entsize = 16
    index = 1
    element = 0x0000000101a192c8
  }
}
(lldb) p $4.endLists()
(property_list_t **) $9 = 0x0000000101a195e8
(lldb) p $4.beginLists()
(property_list_t **) $10 = 0x0000000101a195d8
(lldb) p *$9
(property_list_t *) $11 = 0x0000000000000000 // 这里是 0
(lldb) p *$10
(property_list_t *) $12 = 0x0000000101a192b0
(lldb) p *$12
(property_list_t) $13 = {
  entsize_list_tt<property_t, property_list_t, 0> = {
    entsizeAndFlags = 16
    count = 1
    first = (name = "addPro", attributes = "T@\"NSString\",C,N,V_addPro")
  }
}
(lldb) 
```
16. 虽然 `count()` 打印出来是 2，但是死活只能打印 `first = (name = "addPro", attributes = "T@\"NSString\",C,N,V_addPro")` 这里对应动态的添加的 `hh_class_addProperty([LGPerson class], "addPro")`。只在 `rw->properties` 找到了 `addPro`，也说明动态的添加的属性到了 `rw` 里面，并不包含在 `ro` 中。

**扩展到此结束。**

## 2、运行时
### 2.1 运行时：添加成员变量
`可行`
动态创建一个类，并对其添加一个成员变量，并打印/赋值：
```
// 动态创建类
Class student = objc_allocateClassPair([NSObject class], "HHStudent", 0);
// 添加成员变量 1<<aligment
// ivar - ro - ivarlist
class_addIvar(student, "hhName", sizeof(NSString *), log2(sizeof(NSString *)), "@");
// 注册到内存
objc_registerClassPair(student);

id person = [student alloc];
[person setValue:@"student name value" forKey:@"hhName"];

NSLog(@"🍔🍔🍔 %@", [person valueForKey:@"hhName"]);
// 打印:
🍔🍔🍔 student name value
```
> 从上面代码来看，`可以动态创建类并动态添加成员变量`。
> 请注意：
> 动态添加成员变量必须在类注册到内存之前，即 `objc_registerClassPair` 方法之前，原理和编译时不能添加成员变量是一样的。

通过 LLDB 打印的方式来看看具体的 `rw` `ro` 的结构。
```
(lldb) p student
(Class) $0 = HHStudent
(lldb) x/5gx $0
0x10185e8e0: 0x000000010185e910 0x00000001003ef140
0x10185e8f0: 0x00000001007208b0 0x0002801800000007
0x10185e900: 0x000000010185e944
(lldb) p (class_data_bits_t *)0x10185e900
(class_data_bits_t *) $1 = 0x000000010185e900
(lldb) p $1->data()
(class_rw_t *) $2 = 0x000000010185e940
(lldb) p $2->ro
(const class_ro_t *) $3 = 0x000000010185eeb0
(lldb) p *$3
(const class_ro_t) $4 = { // ro 的内容
  flags = 0
  instanceStart = 8
  instanceSize = 16
  reserved = 0
  ivarLayout = 0x00000001003e95a8 ""
  name = 0x0000000100001ecd "HHStudent"
  baseMethodList = 0x0000000000000000
  baseProtocols = 0x0000000000000000
  ivars = 0x000000010185ef50
  weakIvarLayout = 0x00000001003e95a8 ""
  baseProperties = 0x0000000000000000
  _swiftMetadataInitializer_NEVER_USE = {}
}
(lldb) p $4.ivars
(const ivar_list_t *const) $5 = 0x000000010185ef50
(lldb) p *$5
(const ivar_list_t) $6 = { // ro.ivars 的内容
  entsize_list_tt<ivar_t, ivar_list_t, 0> = {
    entsizeAndFlags = 32
    count = 1 // 看到一个一个 成员变量
    first = {
      offset = 0x0000000101850b10
      name = 0x0000000100001ed7 "hhName" // 名字正是我们的添加的 "hhName"
      type = 0x0000000100001ede "@"
      alignment_raw = 3
      size = 8
    }
  }
}

// rw 的内容：
(lldb) p *$2
(class_rw_t) $7 = {
  flags = 2315255808
  version = 0
  witness = 0
  ro = 0x000000010185eeb0
  methods = {
    list_array_tt<method_t, method_list_t> = {
       = {
        list = 0x0000000000000000
        arrayAndFlag = 0
      }
    }
  }
  properties = {
    list_array_tt<property_t, property_list_t> = {
       = {
        list = 0x0000000000000000 // 为空
        arrayAndFlag = 0
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
  demangledName = 0x0000000000000000
}
```
通过控制台，在 ro 里面打印出了成员变量 `hhName`。

### 2.2 运行时：添加属性
```
void hh_Setter(NSString *value){
    printf("🍿🍿🍿 %s\n",__func__);
}

NSString *hh_Name(){
    printf("🍿🍿🍿 %s\n",__func__);
    return @"hh_Name value";
}

// 创建类
Class student = objc_allocateClassPair([NSObject class], "HHStudent", 0);
// 注册到内存
objc_registerClassPair(student);
// 添加 property
hh_class_addProperty(student, "subject");
// 打印属性
hh_printerProperty(student);

// 添加 setter + getter 方法
class_addMethod(student, @selector(setSubject:), (IMP)hh_Setter, "v@:@");
class_addMethod(student, @selector(subject), (IMP)hh_Name, "@@:");

id stu = [student alloc];

[stu setValue:@"student value" forKey:@"subject"];

NSLog(@"🍿🍩🍩🍿 %@", [stu valueForKey:@"subject"]);

// objc_registerClassPair(student); // 这行代码也可以写在这里

// 打印结果:
🏓🏓🏓 subject T@"NSString",C,N,V_subject // 表示属性添加成功
🍿🍿🍿 hh_Setter // 这里是 setter 函数被调用，但是 hh_Setter 里面只有一行打印，是怎么做到赋值的呢？
🍿🍿🍿 hh_Name // 这里是 getter 函数被调用
🍿🍩🍩🍿 hh_Name value // 这里打印的是 "hh_Name value" 不是 setValue 赋值给的 student value
```
> 从打印结果看，`可以动态创建类并动态添加属性`
> 请注意:
> 添加的属性 `必须要实现其 set、get 方法才能正常进行赋值`，因为动态添加的属性，是在运行时的，编译器没有为其自动生成 set、get、成员变量。（同时还有 setter 和 getter 方法）

`objc_registerClassPair(student);` 可以写在最后面，也就是说，动态创建属性，可以在注册类之前，也可以在注册类之后。

**源码解释：**
为何无法在注册类之后、已编译的类动态添加成员变量?

看 `class_addIvar()` 的源码:
```
BOOL 
class_addIvar(Class cls, const char *name, size_t size, 
              uint8_t alignment, const char *type)
{
    ...
    // Can only add ivars to in-construction classes.
    // 只能在构建类的过程中，添加成员变量
    // class allocated but not yet registered // class 已经分配，但是还没有注册
    // #define RW_CONSTRUCTING       (1<<26)
    if (!(cls->data()->flags & RW_CONSTRUCTING)) {
        return NO;
    }
    ....
};
```
`objc_registerClassPair(Class cls)` 源码:
```
void objc_registerClassPair(Class cls)
{
    ...
    // 将 cls 和 cls->isa 设置成 RW_CONSTRUCTING
    // Clear "under construction" bit, set "done constructing" bit
    cls->ISA()->changeInfo(RW_CONSTRUCTED, RW_CONSTRUCTING | RW_REALIZING);
    cls->changeInfo(RW_CONSTRUCTED, RW_CONSTRUCTING | RW_REALIZING);

    // Add to named class table.
    addNamedClass(cls, cls->data()->ro()->name);
}
```
在 `class_addIvar` 中，如果是 `RW_CONSTRUCTING` 就不能添加成员变量，而在 `objc_registerClassPair` 中将 `cls` 、`cls->isa` 设置为 `RW_CONSTRUCTING` 所以在注册类之后就无法添加成员变量了。

**参考链接:🔗**
[iOS底层--runtime应用之动态添加属性/成员变量](https://www.jianshu.com/p/720889646d3b)
