//
//  NumericStrings_1.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/4.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "NumericStrings_1.hpp"

bool NumericStrings_1::scanUnsignedInteger(const char** str) {
    const char* before = *str;
    while (**str != '\0' && **str >= '0' && **str <= '9') {
        ++(*str);
    }
    
    return *str > before;
}

bool NumericStrings_1::scanInteger(const char** str) {
    if (**str == '+' || **str == '-') {
        ++(*str);
    }
    
    return scanUnsignedInteger(str);
}

bool NumericStrings_1::isNumeric(const char* str) {
    if (str == nullptr) {
        return false;
    }
    
    // 1. 判断前面的整数部分可以是 +/- 开头，或者直接省略了符号，
    //    numeric 也可能是 false，比如是从小数点开始的数字就没有整数部分: .10
    bool numeric = scanInteger(&str);
    
    // 2. 到了小数点部分，必须不能包含 +/- 所以使用 scanUnsignedInteger 函数判断，
    //    由于小数点前面的整数部分可有可无，所以后面用的或（ || numeric ）
    if (*str == '.') {
        ++str;
        numeric = scanUnsignedInteger(&str) || numeric;
    }
    
    // 3. 到了指数部分，类似整数部分可以包含 +/- 或者不包含，
    //    后面用的 && numeric，因为在数字表示中前面必须有内容才能出现指数部分
    if (*str == 'e' || *str == 'E') {
        ++str;
        
        numeric = scanInteger(&str) && numeric;
    }
    
    // 4. numeric 为真，并且字符串到了末尾，才表示该字符串确实表示的是一个数值
    return numeric && *str == '\0';
}

// 测试代码
void NumericStrings_1::Test(const char* testName, const char* str, bool expected) {
    if(testName != nullptr)
        printf("%s begins: ", testName);
    
    if(isNumeric(str) == expected)
        printf("Passed.\n");
    else
        printf("FAILED.\n");
}

void NumericStrings_1::Test() {
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
