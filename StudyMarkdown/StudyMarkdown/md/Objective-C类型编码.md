# Objective-C类型编码  

在开发中我们会遇到后跟 `ObjCTypes:(const char *)types` 的方法，
如下：
```
+ (nullable NSMethodSignature *)signatureWithObjCTypes:(const char *)types;
+ (NSValue *)valueWithBytes:(const void *)value objCType:(const char *)type;
+ (NSValue *)value:(const void *)value withObjCType:(const char *)type;
```
ObjCType 的参数需要用 OC 的编译指令 `@encode()` 来创建，`@encode()` 返回的是 OC 类型编码（Objective-C Type Encodings）。 

## 一、Objective-C Type Encodings
|编码|意义|
|---|---|
|c| A char |
|i| An int|
|s| A short |
|l (这是小写 L)| A longl is treated as a 32-bit quantity on 64-bit programs. (在 64 位程序中，longl被视为 32 位。)|
|q| A long long |
|大写小写分界线|-----------|
|C| An unsigned char |
|I (这是大写 i)| An unsigned int |
|S| An unsigned short |
|L| An unsigned long|
|Q| An unsigned long long|
|分界线|-----------|
|f| A float |
|d| A double |
|B| A C++ bool or a C99 _Bool |
|v| A void |
|*| A character string (char *) |
|@| An object (whether statically typed or typed id) |
|#| A class object (Class) |
|:| A method selector (SEL) (选择子) |
|[array type]| An array |
|{name=type...}| A union |
|bnum| A bit field of num bits |
|^type| A pointer to type (指向类型的指针) |
|?| An unknown type(among other things, this code is used for function pointers) (未知类型，除此之外还被用于 “函数指针”) |

## 二、Objective-C Method Encodings
编译器内部有些关键字无法用 `@encode()` 返回，这些关键字也有自己的编码:
|编码|意义|
|---|---|
|r| const |
|n| in |
|N| inout |
|o| out |
|O| bycopy |
|R| byref |
|V| oneway |

## 三、验证
```
NSLog(@"💍💍💍 基本数据类型:");
// c i s l q / C I S L Q / f d B v * @ # :
NSLog(@"🐣🐣🐣 char \t %s", @encode(char));
NSLog(@"🐣🐣🐣 int \t %s", @encode(int));
NSLog(@"🐣🐣🐣 short \t %s", @encode(short));
NSLog(@"🐣🐣🐣 long \t %s", @encode(long));
NSLog(@"🐣🐣🐣 long long \t %s", @encode(long long));

NSLog(@"🐣🐣🐣 unsigned char \t %s", @encode(unsigned char));
NSLog(@"🐣🐣🐣 unsigned int \t %s", @encode(unsigned int));
NSLog(@"🐣🐣🐣 unsigned short \t %s", @encode(unsigned short));
NSLog(@"🐣🐣🐣 unsigned long \t %s", @encode(unsigned long));
NSLog(@"🐣🐣🐣 unsigned long long \t %s", @encode(unsigned long long));

NSLog(@"🐣🐣🐣 float \t %s", @encode(float));
NSLog(@"🐣🐣🐣 double \t %s", @encode(double));
NSLog(@"🐣🐣🐣 bool \t %s", @encode(bool));
NSLog(@"🐣🐣🐣 BOOL \t %s", @encode(BOOL));
NSLog(@"🐣🐣🐣 _Bool \t %s", @encode(_Bool));
NSLog(@"🐣🐣🐣 void \t %s", @encode(void));
NSLog(@"🐣🐣🐣 char * \t %s", @encode(char *));

NSLog(@"🐳🐳🐳 指针和数组类型:");
NSLog(@"🐳🐳🐳 int * \t %s", @encode(int *));
NSLog(@"🐳🐳🐳 int ** \t %s", @encode(int **));
NSLog(@"🐳🐳🐳 int *** \t %s", @encode(int ***));
NSLog(@"🐳🐳🐳 int [] \t %s", @encode(int []));
NSLog(@"🐳🐳🐳 int [2] \t %s", @encode(int [2]));
NSLog(@"🐳🐳🐳 int [][3] \t %s", @encode(int [][3]));
NSLog(@"🐳🐳🐳 int [3][3] \t %s", @encode(int [3][3]));
NSLog(@"🐳🐳🐳 int [][4][4] \t %s", @encode(int [][4][4]));
NSLog(@"🐳🐳🐳 int [4][4][4] \t %s", @encode(int [4][4][4]));

NSLog(@"🦄🦄🦄 空类型:");
NSLog(@"🦄🦄🦄 void \t %s", @encode(void));
NSLog(@"🦄🦄🦄 void * \t %s", @encode(void *));
NSLog(@"🦄🦄🦄 void ** \t %s", @encode(void **));
NSLog(@"🦄🦄🦄 void *** \t %s", @encode(void ***));

NSLog(@"🍀🍀🍀 结构体类型:");
struct Person {
    int *test;
    char *anme;
    int age;
    char *birthday;
};
NSLog(@"🍀🍀🍀 struct Person \t %s", @encode(struct Person));
NSLog(@"🍀🍀🍀 CGPoint \t %s", @encode(CGPoint));
NSLog(@"🍀🍀🍀 CGRect \t %s", @encode(CGRect));

NSLog(@"🌸🌸🌸 OC类型:");
NSLog(@"🌸🌸🌸 BOOL \t %s", @encode(BOOL));
NSLog(@"🌸🌸🌸 SEL \t %s", @encode(SEL));
NSLog(@"🌸🌸🌸 id \t %s", @encode(id));
NSLog(@"🌸🌸🌸 Class \t %s", @encode(Class));
NSLog(@"🌸🌸🌸 Class * \t %s", @encode(Class *));
NSLog(@"🌸🌸🌸 NSObject class \t %s", @encode(typeof([NSObject class])));
NSLog(@"🌸🌸🌸 [NSObject class] * \t %s", @encode(typeof([NSObject class]) *));
NSLog(@"🌸🌸🌸 NSObject \t %s", @encode(NSObject));
NSLog(@"🌸🌸🌸 NSObject * \t %s", @encode(NSObject *));
NSLog(@"🌸🌸🌸 NSArray \t %s", @encode(NSArray));
NSLog(@"🌸🌸🌸 NSArray * \t %s", @encode(NSArray *));
NSLog(@"🌸🌸🌸 NSMutableArray \t %s", @encode(NSMutableArray));
NSLog(@"🌸🌸🌸 NSMutableArray * \t %s", @encode(NSMutableArray *));
NSLog(@"🌸🌸🌸 UIView \t %s", @encode(UIView));
NSLog(@"🌸🌸🌸 UIView * \t %s", @encode(UIView *));
NSLog(@"🌸🌸🌸 UIImage \t %s", @encode(UIImage));
NSLog(@"🌸🌸🌸 UIImage * \t %s", @encode(UIImage *));

// 打印:
💍💍💍 基本数据类型:
🐣🐣🐣 char      c
🐣🐣🐣 int      i
🐣🐣🐣 short      s
🐣🐣🐣 long      q // 这里 long 也是 q 和 long long 相同
🐣🐣🐣 long long      q
🐣🐣🐣 unsigned char      C
🐣🐣🐣 unsigned int      I
🐣🐣🐣 unsigned short      S
🐣🐣🐣 unsigned long      Q // 这里 unsigned long 也是 Q 和 unsigned long long 相同
🐣🐣🐣 unsigned long long      Q
🐣🐣🐣 float      f
🐣🐣🐣 double      d
🐣🐣🐣 bool      B // C++ 中的 bool
🐣🐣🐣 BOOL      c // 我们在 OC 中用 BOOL 返回的是 c
🐣🐣🐣 _Bool      B // C99 中的 _Bool
🐣🐣🐣 void      v
🐣🐣🐣 char *      * // 指向字符的指针较特殊，下面指针相关的都包含 ^
🐳🐳🐳 指针和数组类型:
🐳🐳🐳 int *      ^i
🐳🐳🐳 int **      ^^i
🐳🐳🐳 int ***      ^^^i
🐳🐳🐳 int []      ^i
🐳🐳🐳 int [2]      [2i] // 2 表示长度，i 表示类型
🐳🐳🐳 int [][3]      ^[3i]
🐳🐳🐳 int [3][3]      [3[3i]]
🐳🐳🐳 int [][4][4]      ^[4[4i]]
🐳🐳🐳 int [4][4][4]      [4[4[4i]]]
🦄🦄🦄 空类型:
🦄🦄🦄 void      v
🦄🦄🦄 void *      ^v
🦄🦄🦄 void **      ^^v
🦄🦄🦄 void ***      ^^^v
🍀🍀🍀 结构体类型:
🍀🍀🍀 struct Person      {Person=^i*i*} // Person 表示结构体名字，= 后面是对应每个成员变量: ^i(int *) *(char *) i(int) *(char *) 
🍀🍀🍀 CGPoint      {CGPoint=dd} //     CGFloat x; CGFloat y; 可验证: CGFloat 内部应该是 double 类型 
🍀🍀🍀 CGRect      {CGRect={CGPoint=dd}{CGSize=dd}} //     CGPoint origin; CGSize size;
🌸🌸🌸 OC类型:
🌸🌸🌸 BOOL      c
🌸🌸🌸 SEL      :
🌸🌸🌸 id      @ 
🌸🌸🌸 Class      #
🌸🌸🌸 Class *      ^#
🌸🌸🌸 NSObject class      #
🌸🌸🌸 [NSObject class] *      ^#
🌸🌸🌸 NSObject      {NSObject=#} // 这里验证了 NSObject 是一个结构体类型，且有一个 Class 类型的成员变量，对应了它的 isa
🌸🌸🌸 NSObject *      @ // 和 id 同，指向结构体的指针
🌸🌸🌸 NSArray      {NSArray=#}
🌸🌸🌸 NSArray *      @
🌸🌸🌸 NSMutableArray      {NSMutableArray=#}
🌸🌸🌸 NSMutableArray *      @
🌸🌸🌸 UIView      {UIView=#}
🌸🌸🌸 UIView *      @
🌸🌸🌸 UIImage      {UIImage=#}
🌸🌸🌸 UIImage *      @
```

**应用示例:**

1. 判断 NSNumber 内部的数据类型。
```
NSDictionary *dic = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:YES],@"key1",[NSNumber numberWithDouble:1.00f],@"key2",[NSNumber numberWithInt:1],@"key3",[NSNumber numberWithFloat:33.0f], @"key4", nil];

for(NSString *key in dic){
    id value = [dic valueForKey:key];
    
    if([value isKindOfClass:[NSNumber class]]){
        const char * pObjCType = [((NSNumber*)value) objCType];
        if (strcmp(pObjCType, @encode(int))  == 0) {
            NSLog(@"字典中key=%@的值是int类型,值为%d",key,[value intValue]);
        }
        if (strcmp(pObjCType, @encode(float)) == 0) {
            NSLog(@"字典中key=%@的值是float类型,值为%f",key,[value floatValue]);
        }
        if (strcmp(pObjCType, @encode(double))  == 0) {
            NSLog(@"字典中key=%@的值是double类型,值为%f",key,[value doubleValue]);
        }
        if (strcmp(pObjCType, @encode(BOOL)) == 0) {
            NSLog(@"字典中key=%@的值是bool类型,值为%i",key,[value boolValue]);
        }
    }
}
```

**参考链接🔗:**
[Objective-C类型编码](https://blog.csdn.net/ssirreplaceable/article/details/53376915)
