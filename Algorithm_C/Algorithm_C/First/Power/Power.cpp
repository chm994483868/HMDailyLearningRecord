//
//  Power.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/14.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "Power.hpp"

bool equal_Power(double num1, double num2);
double powerWithUnsignedExponent(double base, unsigned int exponent);

double power(double base, int exponent) {
    Power::g_InvalidInput = false;
    
    // 1. 判断如果 base 是 0，并且 exponent 小于 0，则不能对 0 取倒数
    if (equal_Power(base, 0.0) && exponent < 0) {
        Power::g_InvalidInput = true;
        return 0.0;
    }
    
    // 2. 求 exponent 的绝对值
    unsigned int absExponent = (unsigned int)exponent;
    if (exponent < 0)
        absExponent = (unsigned int)(-exponent);
    
    // 3. 求乘方
    double result = powerWithUnsignedExponent(base, absExponent);
    
    // 4. 如果指数为负，则再进行取倒数
    if (exponent < 0)
        result = 1.0 / result;
    
    return result;
}

double powerWithUnsignedExponent(double base, unsigned int exponent) {
    if (exponent == 0)
        return 1;
    if (exponent == 1)
        return base;
    
    // 1111 = 15
    // 0111 = 7
    // 0011 = 3
    // 0001 = 1
    // 0000 = 0
    
    // f(15) * f(7) * f(3) * f(1)
    
    double result = powerWithUnsignedExponent(base, exponent >> 1);
    
    result *= result;
    if ((exponent & 0x1) == 1)
        result *= base;
    
    return result;
}

double powerWithUnsignedExponent_Original(double base, unsigned int exponent) {
    int result = 1.0;
    for (int i = 1; i <= exponent; ++i) {
        result *= base;
    }
    
    return result;
}

bool equal_Power(double num1, double num2) {
    if ((num1 - num2 > -0.00000001) && (num1 - num2 < 0.0000001)) {
        return true;
    } else {
        return false;
    }
}

// ====================≤‚ ‘¥˙¬Î====================
void Test(const char* testName, double base, int exponent, double expectedResult, bool expectedFlag) {
    double result = power(base, exponent);
    
    if (equal_Power(result, expectedResult) && Power::g_InvalidInput == expectedFlag)
        std::cout << testName << " passed" << std::endl;
    else
        std::cout << testName << " FAILED" << std::endl;
}

void startTest_Power() {
    // µ◊ ˝°¢÷∏ ˝∂ºŒ™’˝ ˝
    Test("Test1", 2, 3, 8, false);

    // µ◊ ˝Œ™∏∫ ˝°¢÷∏ ˝Œ™’˝ ˝
    Test("Test2", -2, 3, -8, false);

    // ÷∏ ˝Œ™∏∫ ˝
    Test("Test3", 2, -3, 0.125, false);

    // ÷∏ ˝Œ™0
    Test("Test4", 2, 0, 1, false);

    // µ◊ ˝°¢÷∏ ˝∂ºŒ™0
    Test("Test5", 0, 0, 1, false);

    // µ◊ ˝Œ™0°¢÷∏ ˝Œ™’˝ ˝
    Test("Test6", 0, 4, 0, false);

    // µ◊ ˝Œ™0°¢÷∏ ˝Œ™∏∫ ˝
    Test("Test7", 0, -4, 0, true);
}
