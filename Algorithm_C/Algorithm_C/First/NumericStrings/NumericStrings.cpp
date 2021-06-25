//
//  NumericStrings.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/16.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "NumericStrings.hpp"

bool scanUnsignedInteger(const char** str);
bool scanInteger(const char** str);

bool isNumeric(const char* str) {
    if (str == nullptr)
        return false;
    
    // 核心逻辑: 用 &str 指针从字符串开头一直右移直到结尾的空字符 '\0'，最后一个 return 语句再来一手 return numeric && *str == '\0' (前面的判断结果加字符串必须到了末尾，此时一定得出到底是不是数字)
    
    // 1. 首先判断整数部分
    bool numeric = scanInteger(&str);
    
    // 如果出现 '.'，接下来是数字的小数部分
    if (*str == '.') {
        ++str;
        
        // 下面一行代码用 || 的原因:
        // 1). 小数可以没有整数部分，例如 .123 等于 0.123;
        // 2). 小数点后面可以没有数字，例如 233. 等于 233.0;
        // 3). 当然小数点前面和后面可以有数字，例如 233.666;
        numeric = scanUnsignedInteger(&str) || numeric;
    }
    
    // 如果出现 'e' 或者 'E'，接下来跟着的是数字的指数部分
    if (*str == 'e' || *str == 'E') {
        ++str;
        
        // 下面一行代码用 && 的原因:
        // 1). 当 e 或 E 前面没有数字时，整个字符串不能表示数字，例如 .e1、e1;
        // 2). 当 e 或 E 后面没有整数时，整个字符不能表示数字，例如 12e、12e+5.4;
        numeric = numeric && scanInteger(&str);
    }
    
    return numeric && *str == '\0';
}

bool scanUnsignedInteger(const char** str) {
    const char* before = *str;
    while (**str != '\0' && **str >= '0' && **str <= '9')
        ++(*str);
    
    // 当 str 中存在若干 0-9 的数字时，返回 true
    // 字符串内必有 0-9 数字，要不然返回 false
    return *str > before;
}

// 整数的格式可以用 [+|-]B 表示，其中 B 为无符号整数
// **str: 是字符串本体
// *str: 是字符串地址也可以说是字符串的指针
// str: 是指向字符串指针的指针
bool scanInteger(const char** str) {
    if (**str == '+' || **str == '-')
        ++(*str);
    
    return scanUnsignedInteger(str);
}

bool scanUnsignedInteger_Review(const char** str);
bool scanInteger_Review(const char** str);

bool isNumeric_Review(const char* str) {
    if (str == nullptr)
        return false;
    
    bool numeric = scanInteger_Review(&str);
    
    if (*str == '.') {
        ++str;
        
        numeric = scanUnsignedInteger_Review(&str) || numeric;
    }
    
    if (*str == 'e' || *str == 'E') {
        ++str;
        
        numeric = scanInteger_Review(&str) && numeric;
    }
    
    return numeric && *str == '\0';
}

bool scanUnsignedInteger_Review(const char** str) {
    const char* pBefore = *str;
    while (**str != '\0' && **str >= '0' && **str <= '9')
        ++(*str);
    
    return *str > pBefore;
}

bool scanInteger_Review(const char** str) {
    if (**str == '+' || **str == '-')
        ++(*str);
    
    return scanUnsignedInteger_Review(str);
}

void Test(const char* testName, const char* str, bool expected) {
    if(testName != nullptr)
        printf("%s begins: ", testName);

    if(isNumeric(str) == expected)
        printf("Passed.\n");
    else
        printf("FAILED.\n");
}

void startTest_IsNumeric() {
    Test("Test1", "100", true);
    Test("Test2", "123.45e+6", true);
    Test("Test3", "+500", true);
    Test("Test4", "5e2", true);
    Test("Test5", "3.1416", true);
    Test("Test6", "600.", true);
    Test("Test7", "-.123", true);
    Test("Test8", "-1E-16", true);
    Test("Test9", "1.79769313486232E+308", true);
    
    printf("\n\n");
    
    Test("Test10", "12e", false);
    Test("Test11", "1a3.14", false);
    Test("Test12", "1+23", false);
    Test("Test13", "1.2.3", false);
    Test("Test14", "+-5", false);
    Test("Test15", "12e+5.4", false);
    Test("Test16", ".", false);
    Test("Test17", ".e1", false);
    Test("Test18", "e1", false);
    Test("Test19", "+.", false);
    Test("Test20", "", false);
    Test("Test21", nullptr, false);
}
