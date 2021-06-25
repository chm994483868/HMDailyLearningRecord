//
//  NumberOf1.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/22.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "NumberOf1.hpp"

int numberOf1(unsigned int n);
int numberOf1Between1AndN_Solution1(unsigned int n) {
    int number = 0;
    for (unsigned int i = 1; i <= n; ++i) {
        number += numberOf1(i);
    }
    
    return number;
}

int numberOf1(unsigned int n) {
    int number = 0;
    
    while (n) {
        if (n % 10 == 1) {
            ++number;
        }
        
        n /= 10;
    }
    
    return number;
}

int numberOf1_Review(unsigned int n) {
    int number = 0;
    
    while (n) {
        if (n % 10 == 1) {
            ++number;
        }
        
        n /= 10;
    }
    
    return number;
}

int numberOf1Between1AndN_Solution1_Review(unsigned int n) {
    int number = 0;
    for (int i = 1; i <= n; ++i) {
        number += numberOf1_Review(i);
    }
    
    return number;
}

int numberof1(const char* strN);
int powerBase10(unsigned int n);

int numberOf1Between1AndN_Solution2_Review(int n) {
    if (n <= 0) {
        return 0;
    }
    
    char strN[50];
    
    // int sprintf(char *string, char *format [,argument,...]);
    // 功能: 把格式化的数据写入某个字符串缓冲区
    // 返回值: 如果成功，则返回写入的字符总数，不包括字符串追加在字符串末尾的空字符
    // 如果失败，则返回一个负数
    // sprintf 返回以 format 为格式 argument 为内容组成的结果被写入 string 的字节数，
    // 结束字符 ‘\0’ 不计入内。即，如果 “Hello” 被写入空间足够大的 string 后，函数 sprintf 返回 5
    sprintf(strN, "%d", n); // 把输入的数字 n 转化为字符串放在字符数组 strN 里面
    
    return numberof1(strN);
}

int numberof1(const char* strN) {
    if (!strN || *strN < '0' || *strN > '9' || *strN == '\0') {
        return 0;
    }
    
    // 1. 把字符串第一个字符转化为 int
    int first = *strN - '0';
    // 2. 取得字符长度
    // static_cast 是一个 C++ 运算符，功能是把一个表达式转换为某种类型，
    // 但没有运行时类型检查来保证转换的安全性。
    unsigned int length = static_cast<unsigned int>(strlen(strN));
    
    // 3. 如果原始的 n 就是一位数，且等于 0，那么不存在 1
    if (length == 1 && first == 0) {
        return 0;
    }
    
    // 4. 如果原始的 n 就是一位数，但是大于 0，则唯一的情况就是 1~n 包含一个 1，那么 1 存在的个数必定是 1
    if (length == 1 && first > 0) {
        return 1;
    }
    
    // 假设 strN 是 "21345"
    // numFirstDigit 是数字 10000-19999 的第一个位中 1 的数目
    int numFirstDigit = 0;
    if (first > 1) {
        numFirstDigit = powerBase10(length - 1);
    } else if (first == 1) {
        // C 语言库函数
        // int atoi(const char *nptr);
        // atoi (表示 ascii to integer)是把字符串转换成整型数的一个函数，
        // int atoi(const char *nptr) 函数会扫描参数 nptr 字符串，会跳过前面的空白字符（例如空格，tab缩进）等。
        // 如果 nptr 不能转换成 int 或者 nptr为空字符串，那么将返回 0。
        // 特别注意，该函数要求被转换的字符串是按十进制数理解的。
        // atoi输入的字符串对应数字存在大小限制（与int类型大小有关），若其过大可能报错-1。
        numFirstDigit = atoi(strN + 1) + 1;
    }
    
    // numOtherDigits 是 01346-21345 除了第一位之外的数位中 1 的数目
    int numOtherDigits = first * (length - 1) * powerBase10(length - 2);
    // numRecursive 是 1-1345 中 1 的数目
    
    // 递归计算下面剩下数字中包含的 1
    int numRecursive = numberof1(strN + 1);
    
    return numFirstDigit + numOtherDigits + numRecursive;
}

// 输入 n, 返回 10 的 n 次方
int powerBase10(unsigned int n) {
    int result = 1;
    for (unsigned int i = 0; i < n; ++i) {
        result *= 10;
    }
    
    return result;
}
