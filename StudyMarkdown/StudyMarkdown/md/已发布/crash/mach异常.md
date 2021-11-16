### mach 异常

&emsp;Objective-C 的异常处理（指 @try @catch 和 NSSetUncaughtExceptionHandler 机制，它们对内存访问错误、重复释放等错误是无能为力的，这种错误抛出的是 signal，所以需要专门做 signal 处理）不能得到 signal，如果要处理它需要利用 unix 标准的 signal 机制，注册 SIGABRT、SIGBUS、SIGSEGV 等信号发生时的处理函数。



