#  iOS Tagged Pointer

> `malloc_size()` 系统创建时 系统为对象分配了多少内存，
  `class_getInstanceSize()` 对象实际利用了多少内存，
  代码层次的理解:
  `malloc_size()` 可以认为是在 `class_getInstanceSize()` 之后进行了一次 `16` 位内存对齐。

示例代码:
// 54
```objective-c
{
            NSLog(@"%f", pow(2, 54));
            
            NSNumber *number = @(pow(2, 54));
            NSDate *date = [[NSDate alloc] init];
            CusObject *objc = [[CusObject alloc] init];
            
            NSLog(@"number sizeof: %ld", sizeof(number));
            NSLog(@"date sizeof: %ld", sizeof(date));
            NSLog(@"objc sizeof: %ld", sizeof(objc));
            NSLog(@"\n");
            
            NSLog(@"number malloc: %zu", malloc_size(CFBridgingRetain(number)));
            NSLog(@"date malloc: %zu", malloc_size(CFBridgingRetain(date)));
            NSLog(@"objc malloc: %zu", malloc_size(CFBridgingRetain(objc)));
            
            CFBridgingRelease((__bridge CFTypeRef _Nullable)(number));
            CFBridgingRelease((__bridge CFTypeRef _Nullable)(date));
            CFBridgingRelease((__bridge CFTypeRef _Nullable)(objc));
            
            NSLog(@"\n");
            
            NSLog(@"number class_getInstanceSize: %zu", class_getInstanceSize([number class]));
            NSLog(@"date class_getInstanceSize: %zu", class_getInstanceSize([date class]));
            NSLog(@"objc class_getInstanceSize: %zu", class_getInstanceSize([NSObject class]));
            NSLog(@"\n");
            
            NSNumber *number1 = @1;
            NSNumber *number2 = @2;
            NSNumber *number3 = @3;
            // NSNumber *numberFFFF = @(0xFFFF);
            
            NSNumber *numberFFFF = @(pow(2, 54));
//            CFBridgingRelease((__bridge CFTypeRef _Nullable)(numberFFFF));
            
            NSLog(@"objc pointer is %p", objc);
            NSLog(@"number1 pointer is %p", number1);
            NSLog(@"number2 pointer is %p", number2);
            NSLog(@"number3 pointer is %p", number3);
            NSLog(@"numberffff pointer is %p ", numberFFFF);
        }
```

```c++
2020-09-04 10:47:48.300696+0800 KCObjc[95117:4517257] 18014398509481984.000000
2020-09-04 10:47:48.302399+0800 KCObjc[95117:4517257] number sizeof: 8
2020-09-04 10:47:48.302748+0800 KCObjc[95117:4517257] date sizeof: 8
2020-09-04 10:47:48.303357+0800 KCObjc[95117:4517257] objc sizeof: 8
2020-09-04 10:47:48.303639+0800 KCObjc[95117:4517257] 
2020-09-04 10:47:48.303722+0800 KCObjc[95117:4517257] number malloc: 0
2020-09-04 10:47:48.303958+0800 KCObjc[95117:4517257] date malloc: 0
2020-09-04 10:47:48.304212+0800 KCObjc[95117:4517257] objc malloc: 16
2020-09-04 10:47:48.304473+0800 KCObjc[95117:4517257] 
2020-09-04 10:47:48.304741+0800 KCObjc[95117:4517257] number class_getInstanceSize: 8
2020-09-04 10:47:48.304814+0800 KCObjc[95117:4517257] date class_getInstanceSize: 8
2020-09-04 10:47:48.304862+0800 KCObjc[95117:4517257] objc class_getInstanceSize: 8
2020-09-04 10:47:48.305240+0800 KCObjc[95117:4517257] 
2020-09-04 10:47:48.305466+0800 KCObjc[95117:4517257] objc pointer is 0x102800f30
2020-09-04 10:47:48.305536+0800 KCObjc[95117:4517257] number1 pointer is 0xf99ad506c167169f
2020-09-04 10:47:48.305580+0800 KCObjc[95117:4517257] number2 pointer is 0xf99ad506c167159f
2020-09-04 10:47:48.338546+0800 KCObjc[95117:4517257] number3 pointer is 0xf99ad506c167149f
2020-09-04 10:47:48.338613+0800 KCObjc[95117:4517257] numberffff pointer is 0xb99ad506c16717ef
2020-09-04 10:47:48.339276+0800 KCObjc[95117:4517257] 🍀🍀🍀 CusObject deallocing
```

// 55
```c++
{
            NSLog(@"%f", pow(2, 55));
            
            NSNumber *number = @(pow(2, 55));
            NSDate *date = [[NSDate alloc] init];
            CusObject *objc = [[CusObject alloc] init];
            
            NSLog(@"number sizeof: %ld", sizeof(number));
            NSLog(@"date sizeof: %ld", sizeof(date));
            NSLog(@"objc sizeof: %ld", sizeof(objc));
            NSLog(@"\n");
            
            NSLog(@"number malloc: %zu", malloc_size(CFBridgingRetain(number)));
            NSLog(@"date malloc: %zu", malloc_size(CFBridgingRetain(date)));
            NSLog(@"objc malloc: %zu", malloc_size(CFBridgingRetain(objc)));
            
            CFBridgingRelease((__bridge CFTypeRef _Nullable)(number));
            CFBridgingRelease((__bridge CFTypeRef _Nullable)(date));
            CFBridgingRelease((__bridge CFTypeRef _Nullable)(objc));
            
            NSLog(@"\n");
            
            NSLog(@"number class_getInstanceSize: %zu", class_getInstanceSize([number class]));
            NSLog(@"date class_getInstanceSize: %zu", class_getInstanceSize([date class]));
            NSLog(@"objc class_getInstanceSize: %zu", class_getInstanceSize([NSObject class]));
            NSLog(@"\n");
            
            NSNumber *number1 = @1;
            NSNumber *number2 = @2;
            NSNumber *number3 = @3;
            // NSNumber *numberFFFF = @(0xFFFF);
            
            NSNumber *numberFFFF = @(pow(2, 55));
//            CFBridgingRelease((__bridge CFTypeRef _Nullable)(numberFFFF));
            
            NSLog(@"objc pointer is %p", objc);
            NSLog(@"number1 pointer is %p", number1);
            NSLog(@"number2 pointer is %p", number2);
            NSLog(@"number3 pointer is %p", number3);
            NSLog(@"numberffff pointer is %p ", numberFFFF);
        }
```
```c++
2020-09-04 10:49:25.701356+0800 KCObjc[95134:4518802] 36028797018963968.000000
2020-09-04 10:49:25.702072+0800 KCObjc[95134:4518802] number sizeof: 8
2020-09-04 10:49:25.702238+0800 KCObjc[95134:4518802] date sizeof: 8
2020-09-04 10:49:25.702342+0800 KCObjc[95134:4518802] objc sizeof: 8
2020-09-04 10:49:25.702382+0800 KCObjc[95134:4518802] 
2020-09-04 10:49:25.702439+0800 KCObjc[95134:4518802] number malloc: 32
2020-09-04 10:49:25.702474+0800 KCObjc[95134:4518802] date malloc: 0
2020-09-04 10:49:25.702508+0800 KCObjc[95134:4518802] objc malloc: 16
2020-09-04 10:49:25.702551+0800 KCObjc[95134:4518802] 
2020-09-04 10:49:25.702594+0800 KCObjc[95134:4518802] number class_getInstanceSize: 8
2020-09-04 10:49:25.702640+0800 KCObjc[95134:4518802] date class_getInstanceSize: 8
2020-09-04 10:49:25.702676+0800 KCObjc[95134:4518802] objc class_getInstanceSize: 8
2020-09-04 10:49:25.702710+0800 KCObjc[95134:4518802] 
2020-09-04 10:49:25.702747+0800 KCObjc[95134:4518802] objc pointer is 0x100501a90
2020-09-04 10:49:25.702782+0800 KCObjc[95134:4518802] number1 pointer is 0xcccc79878ccb08b9
2020-09-04 10:49:25.702948+0800 KCObjc[95134:4518802] number2 pointer is 0xcccc79878ccb0bb9
2020-09-04 10:49:25.815778+0800 KCObjc[95134:4518802] number3 pointer is 0xcccc79878ccb0ab9
2020-09-04 10:49:25.815850+0800 KCObjc[95134:4518802] numberffff pointer is 0x100601930
2020-09-04 10:49:25.816220+0800 KCObjc[95134:4518802] 🍀🍀🍀 CusObject deallocing
```

`isTaggedPointer`:
```c++
inline bool 
objc_object::isTaggedPointer() 
{
    return _objc_isTaggedPointer(this);
}
```
`_objc_isTaggedPointer`:
```c++
#if (TARGET_OS_OSX || TARGET_OS_IOSMAC) && __x86_64__
    // 64-bit Mac - tag bit is LSB
#   define OBJC_MSB_TAGGED_POINTERS 0
#else
    // Everything else - tag bit is MSB
#   define OBJC_MSB_TAGGED_POINTERS 1
#endif

#if OBJC_MSB_TAGGED_POINTERS
#   define _OBJC_TAG_MASK (1UL<<63)
...
#else
#   define _OBJC_TAG_MASK 1UL
...
#endif

static inline bool 
_objc_isTaggedPointer(const void * _Nullable ptr)
{
    return ((uintptr_t)ptr & _OBJC_TAG_MASK) == _OBJC_TAG_MASK;
}
```

## 参考链接:
**参考链接:🔗**
+ [深入理解 Tagged Pointer](https://www.infoq.cn/article/deep-understanding-of-tagged-pointer/)
+ [字面量(Literal)](http://www.nscookies.com/literal/)
+ [【译】采用Tagged Pointer的字符串](http://www.cocoachina.com/articles/13449)
+ [iOS Tagged Pointer](https://www.jianshu.com/p/e354f9137ba8)
