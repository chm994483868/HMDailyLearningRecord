//
//  NumericStrings.cpp
//  OfferReview
//
//  Created by CHM on 2020/7/29.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "NumericStrings.hpp"

bool NumericStrings::scanUnsignedInteger(const char** str) {
    const char* before = *str;
    while (**str != '\0' && **str >= '0' && **str <= '9') {
        ++(*str);
    }
    
    return *str > before;
}

bool NumericStrings::scanInteger(const char** str) {
    if (**str == '+' || **str == '-') {
        ++(*str);
    }
    
    return scanUnsignedInteger(str);
}

bool NumericStrings::isNumeric(const char* str) {
    if (str == nullptr) {
        return false;
    }
    
    bool numeric = scanInteger(&str);
    
    if (*str == '.') {
        ++str;
        
        numeric = scanUnsignedInteger(&str) || numeric;
    }
    
    if (*str == 'e' || *str == 'E') {
        ++str;
        
        numeric = scanInteger(&str) && numeric;
    }
    
    return numeric && *str == '\0';
}

// 测试代码
void NumericStrings::Test(const char* testName, const char* str, bool expected) {
    if(testName != nullptr)
        printf("%s begins: ", testName);
    
    if(isNumeric(str) == expected)
        printf("Passed.\n");
    else
        printf("FAILED.\n");
}

void NumericStrings::Test() {
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
