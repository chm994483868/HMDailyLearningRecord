//
//  RegularExpressions.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/15.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "RegularExpressions.hpp"

bool matchCore(const char* str, const char* pattern);

bool match(const char* str, const char* pattern) {
    if (str == nullptr || pattern == nullptr)
        return false;
    
    return matchCore(str, pattern);
}

bool matchCore(const char* str, const char* pattern) {
    if (*str == '\0' && *pattern == '\0')
        return true;
    
    if (*str != '\0' && *pattern == '\0')
        return false;
    
    // 如果 pattern 的第二个字符是 * 是最特殊的情况
    if (*(pattern + 1) == '*') {
        // 如果 pattern 和 str 的第一个字符一样或者 pattern 的第一个字符是 .
        if (*pattern == *str || (*pattern == '.' && *str != '\0'))
            // 1). 对应 str 的 第一位 x 与 pattern 的前两位 x* 直接匹配
            // 2). 对应 str 的 第一位 x 与 pattern 的 xxxxx* (前面 n 个 x ) 的类型匹配
            // 3). 对应 直接忽略 pattern 的前两位 x* 0 个字符
            return matchCore(str + 1, pattern + 2) ||
            matchCore(str + 1, pattern) ||
            matchCore(str, pattern + 2);
        else
            // pattern 和 str 第一个字符不同，且 pattern 第一个字符不是 .
            // 则只能 pattern 直接右移两位，把前两个字符(x*)直接忽略
            return matchCore(str, pattern + 2);
    }
    
    // 如果双方第一个字符相同或者 pattern 第一个字符是 . ，则双方同时右移 1 位
    if (*str == *pattern || (*pattern == '.' && *str != '\0'))
        return matchCore(str + 1, pattern + 1);
    
    // 双方第一个字符就不同，直接返回 false
    return false;
}

void Test(const char* testName, const char* string, const char* pattern, bool expected)
{
    if(testName != nullptr)
        printf("%s begins: ", testName);

    if(match(string, pattern) == expected)
        printf("Passed.\n");
    else
        printf("FAILED.\n");
}

void startTest_Match() {
    Test("Test01", "", "", true);
    Test("Test02", "", ".*", true);
    Test("Test03", "", ".", false);
    Test("Test04", "", "c*", true);
    Test("Test05", "a", ".*", true);
    Test("Test06", "a", "a.", false);
    Test("Test07", "a", "", false);
    Test("Test08", "a", ".", true);
    Test("Test09", "a", "ab*", true);
    Test("Test10", "a", "ab*a", false);
    Test("Test11", "aa", "aa", true);
    Test("Test12", "aa", "a*", true);
    Test("Test13", "aa", ".*", true);
    Test("Test14", "aa", ".", false);
    Test("Test15", "ab", ".*", true);
    Test("Test16", "ab", ".*", true);
    Test("Test17", "aaa", "aa*", true);
    Test("Test18", "aaa", "aa.a", false);
    Test("Test19", "aaa", "a.a", true);
    Test("Test20", "aaa", ".a", false);
    Test("Test21", "aaa", "a*a", true);
    Test("Test22", "aaa", "ab*a", false);
    Test("Test23", "aaa", "ab*ac*a", true);
    Test("Test24", "aaa", "ab*a*c*a", true);
    Test("Test25", "aaa", ".*", true);
    Test("Test26", "aab", "c*a*b", true);
    Test("Test27", "aaca", "ab*a*c*a", true);
    Test("Test28", "aaba", "ab*a*c*a", false);
    Test("Test29", "bbbba", ".*a*a", true);
    Test("Test30", "bcbbabab", ".*a*a", false);
}
