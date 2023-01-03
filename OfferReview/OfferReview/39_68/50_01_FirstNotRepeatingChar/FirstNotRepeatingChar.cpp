//
//  FirstNotRepeatingChar.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "FirstNotRepeatingChar.hpp"

char FirstNotRepeatingChar::firstNotRepeatingChar(const char* pString) {
    if (pString == nullptr) {
        return '\0';
    }
    
    // 准备一个长度是 256 的 int 数组，每个元素初始值为 0。
    // 数组下标对应字符的 ASCII 码 ，每个值则对应该字符出现的次数。
    const int tableSize = 256;
    unsigned int hashTable[tableSize];
    for (unsigned int i = 0; i < tableSize; ++i) {
        hashTable[i] = 0;
    }
    
    // 遍历字符串，记录每个字符出现的次数
    const char* pHashKey = pString;
    while (*(pHashKey) != '\0') {
        hashTable[*(pHashKey++)]++;
    }
    
    // 从 pString 遍历每个字符出现的次数，找到第一个 1 时直接返回即可
    pHashKey = pString;
    while (*pHashKey != '\0') {
        if (hashTable[*pHashKey] == 1) {
            return *pHashKey;
        }
        
        pHashKey++;
    }
    
    return '\0';
}

// 测试代码
void FirstNotRepeatingChar::Test(const char* pString, char expected) {
    if(firstNotRepeatingChar(pString) == expected)
        printf("Test passed.\n");
    else
        printf("Test failed.\n");
}

void FirstNotRepeatingChar::Test() {
    // 常规输入测试，存在只出现一次的字符
    Test("google", 'l');

    // 常规输入测试，不存在只出现一次的字符
    Test("aabccdbd", '\0');

    // 常规输入测试，所有字符都只出现一次
    Test("abcdefg", 'a');

    // 鲁棒性测试，输入nullptr
    Test(nullptr, '\0');
}
