//
//  NumberOf1.cpp
//  OfferReview
//
//  Created by CHM on 2020/7/31.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "NumberOf1.hpp"

int NumberOf1::numberOf1(unsigned int n) {
    int number = 0;
    
    // 从个位开始，判断 1 出现的次数
    while (n) {
        
        // 取模判断个位数字是否是 1
        if (n % 10 == 1) {
            ++number;
        }
        
        // 取商前进，撇弃个位
        n /= 10;
    }
    
    return number;
}

// O(nlogn) 时间复杂度的解法
int NumberOf1::numberOf1Between1AndN_Solution1(unsigned int n) {
    int number = 0;
    // 从 1 遍历 到 n，然后统计这 n 个数字中 1 出现的次数
    for (unsigned int i = 1; i <= n; ++i) {
        // 累加数字中 1 出现的次数
        number += numberOf1(i);
    }
    
    // 返回总次数
    return number;
}

// 找规律求 1 出现的次数
int NumberOf1::numberOf1(const char* strN) {
    // strN 是一个字符指针，判断入参是否合规
    if (strN == nullptr || *strN < '0' || *strN > '9' || *strN == '\0') {
        return 0;
    }
    
    // 取出首字符的数字表示的值
    int first = *strN - '0';
    // 取得字符串长度，（不包含 '\0'）
    unsigned int length = static_cast<unsigned int>(strlen(strN));
    
    // 如果仅一个数字且是数字 0，那肯定返回 0
    if (length == 1 && first == 0) {
        return 0;
    }
    
    // 如果仅一个数字且是大于 0 的数字，那么包含数字 1 的数只有 1
    if (length == 1 && first > 0) {
        return 1;
    }
    
    int numFirstDigit = 0;
    
    // 1 出现在最高位时分两种情况：
    // 1): 1 出现在最高位，且最高位大于 1
    // 2): 1 出现在最高位，且最高位仅为 1
    if (first > 1) {
        // 万位是 2 （21345），在 10000~19999 这 10000 个数字的万位中，一共出现了 10^4 次
        numFirstDigit = powerBase10(length - 1);
    } else if (first == 1) {
        
        // atoi (表示 ascii to integer) 是把字符串转换成整型数的一个函数。
        // int atoi(const char *nptr) 函数会扫描参数 nptr字符串，
        // 会跳过前面的空白字符（例如空格，tab缩进）等。
        // 如果 nptr 不能转换成 int 或者 nptr 为空字符串，那么将返回 0 [1]。
        // 特别注意，该函数要求被转换的字符串是按十进制数理解的。
        // atoi输入的字符串对应数字存在大小限制（与 int 类型大小有关），若其过大可能报错-1。
        
        // 万位是 1（12345），在 10000~12345，出现的次数则是 12345 - 10000 + 1
        numFirstDigit = atoi(strN + 1) + 1;
    }
    
    // numOtherDigits 是 1346~21345 除第一位之外的数位中的数目
    int numOtherDigits = first * (length - 1) * powerBase10(length - 2);
    
    // numRecursive 是 1~1345 中的数目
    int numRecursive = numberOf1(strN + 1);
    
    return numFirstDigit + numOtherDigits + numRecursive;
}

// 10 的次方
int NumberOf1::powerBase10(unsigned int n) {
    int result = 1;
    for (unsigned int i = 0; i < n; ++i) {
        result *= 10;
    }
    return result;
}

int NumberOf1::numberOf1Between1AndN_Solution2(int n) {
    if (n <= 0) {
        return 0;
    }
    
    // 用于输入数字字符串
    char strN[50];
    sprintf(strN, "%d", n);
    
    return numberOf1(strN);
}

// 测试代码
void NumberOf1::Test(const char* testName, int n, int expected) {
    if(testName != nullptr)
        printf("%s begins: \n", testName);
    
    if(numberOf1Between1AndN_Solution1(n) == expected)
        printf("Solution1 passed.\n");
    else
        printf("Solution1 failed.\n");
    
    if(numberOf1Between1AndN_Solution2(n) == expected)
        printf("Solution2 passed.\n");
    else
        printf("Solution2 failed.\n");

    printf("\n");
}

void NumberOf1::Test() {
    Test("Test1", 1, 1);
    Test("Test2", 5, 1);
    Test("Test3", 10, 2);
    Test("Test4", 55, 16);
    Test("Test5", 99, 20);
    Test("Test6", 10000, 4001);
    Test("Test7", 21345, 18821);
    Test("Test8", 0, 0);
}
