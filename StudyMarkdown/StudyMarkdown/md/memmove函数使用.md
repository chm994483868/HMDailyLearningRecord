# memmove 函数使用
&ensp;`memmove` 用于拷贝字节，如果目标区域和源区域有重叠的话，`memmove` 能够保证源串在被覆盖之前将重叠区域的字节拷贝到目标区域中，但复制后源内容会被更改。但是当目标区域与源区域没有重叠则和 `memcpy` 函数功能相同。

原型：`void *memmove( void* dest, const void* src, size_t count );`
头文件：`<string.h>`
功能：由 src 所指内存区域复制 count 个字节到 dest 所指内存区域。
相关函数：`memset`、`memcpy`

```
char s[] = "Golden Global view";
char m[19];
printf("🎉🎉🎉 前：%s\n", s);
memmove(m, s, strlen(s) + 1);
printf("🎉🎉🎉 m：%s\n", m);
memmove(s, s + 7, strlen(s) + 1 - 7);

printf("🎉🎉🎉 后：%s\n", s);
// 打印：
🎉🎉🎉 前：Golden Global view
🎉🎉🎉 m：Golden Global view
🎉🎉🎉 后：Global view
```
