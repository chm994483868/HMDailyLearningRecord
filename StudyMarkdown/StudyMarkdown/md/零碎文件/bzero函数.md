# C语言中bzero函数

原型：`extern void bzero(void *s, int n);`
用法：`#include <string.h>`
功能：置字节字符串 `s` 的前 `n` 个字节为零。
说明：`bzero` 无返回值。
举例：
```c
// bzero.c
#include <syslib.h>
#include <string.h>
int main() {
    　　struct {
        　　int a;
        　　char s[5];
        　　float f;
        　　} tt;
    
    　　char s[20];
    　　bzero(&tt,sizeof(tt)); // struct initialization to zero
    　　bzero(s,20);
    　　clrscr();
    　　printf("Initail Success");
    　　getchar();
    　　return 0;
}
// 原型：extern void bzero(void *s, int n);
```
