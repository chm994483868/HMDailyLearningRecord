### mach 异常

&emsp;Objective-C 的异常处理（指 `@try` `@catch` 和 `NSSetUncaughtExceptionHandler` 机制，它们对内存访问错误、重复释放等错误是无能为力的，这种错误抛出的是 `signal`，所以需要专门做 `signal` 处理）不能得到 signal，如果要处理它需要利用 unix 标准的 signal 机制，注册 `SIGABRT`、`SIGBUS`、`SIGSEGV` 等信号发生时的处理函数。

&emsp;例如我们编写如下函数，然后直接运行，程序会直接 crash 并中止运行，然后 `NSLog(@"✳️✳️✳️ objc: %@", objc);` 行报出：`Thread 1: EXC_BAD_ACCESS (code=1, address=0x3402e8d4c25c)` 指出我们的程序此时有一个 `EXC_BAD_ACCESS` 异常，导致退出，且此时可发现我们通过 `NSSetUncaughtExceptionHandler` 设置的 **未捕获异常处理函数** 在程序中止之前并没有被执行！ 

```c++
    __unsafe_unretained NSObject *objc = [[NSObject alloc] init];
    NSLog(@"✳️✳️✳️ objc: %@", objc);
```

&emsp;在测试除零操作时，发现如下代码在 xcode 12.4 下会 crash，报出：`Thread 1: EXC_ARITHMETIC (code=EXC_I386_DIV, subcode=0x0)` 错误，而在 xcode 13 下面则程序正常运行没有报错退出。

```c++
int a = 0;
int b = 1;
int result = b / a;
NSLog(@"🏵🏵🏵 %d", result);
```

&emsp;Objective-C 的异常如果不做任何处理的话（@try @catch 捕获处理），便会触发程序中止退出，此时造成退出的原因是程序向自身发送了 `SIGABRT` 信号。（对于未捕获的 Objective-C 异常，我们可以通过 `NSSetUncaughtExceptionHandler` 函数设置 **未捕获异常处理函数** 在其中记录存储异常日志，然后在 APP 下次启动时进行上传（**未捕获异常处理函数** 函数执行完毕后，程序必被终止，此时没有机会给我们进行网络请求上传数据），如果异常日志记录得当，然后再配合一些异常发生时用户的操作行为数据，那么可以分析和解决大部分的崩溃问题。）

&emsp;上篇我们已经分析过 Objective-C 的异常捕获处理，下面我们看下如何进行 signal 处理。

```c++
#import "AppDelegate.h"
#import <execinfo.h>

void mySignalHandler(int signal) {
    NSMutableString *mstr = [[NSMutableString alloc] init];
    [mstr appendString:@"Stack:\n"];
    
    void *callstack[128];
    int frames = backtrace(callstack, 128);
    char ** strs = backtrace_symbols(callstack, frames);
    
    printf("🏵🏵🏵 char ** 怎么打印：%p", strs);
    
//    kill(<#pid_t#>, <#int#>)
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    
    // 信号量截断
    signal(SIGABRT, mySignalHandler);
    signal(SIGILL, mySignalHandler);
    signal(SIGSEGV, mySignalHandler);
    signal(SIGFPE, mySignalHandler);
    signal(SIGBUS, mySignalHandler);
    signal(SIGPIPE, mySignalHandler);
    
    return YES;
}
```

&emsp;
