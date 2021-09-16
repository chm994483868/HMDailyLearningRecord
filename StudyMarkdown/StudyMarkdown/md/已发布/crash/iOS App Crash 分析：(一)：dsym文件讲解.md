# iOS App Crash 分析：(一)：dsym文件讲解

&emsp;

## dSYM 文件概述

&emsp;dSYM 是内存地址与函数名、文件名和行号的映射表，一般用来进行崩溃日志分析。<起始地址> <结束地址> <函数> [<文件名: 行号>]

### DWARF 

&emsp;**DWARF** 是一种被众多编译器和调试器使用的用于支持 **源码级别** 调试的 **调试文件格式**，该格式是一个固定的数据格式。sSYM 就是按照 **DWARF** 格式保存调试信息的文件，也就是说 dSYM 是一个文件。

&emsp;**Strip Linked Product** **Deployment Postprocessing**

## symbolicatecrash

## atos单/多行符号化

## dwarfdump



## 参考链接
**参考链接:🔗**
+ [dSYM文件的汇编分析](https://juejin.cn/post/6925618080941146125)
