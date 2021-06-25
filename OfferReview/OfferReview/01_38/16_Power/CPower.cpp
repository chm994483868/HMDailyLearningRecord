//
//  CPower.cpp
//  OfferReview
//
//  Created by HM C on 2020/7/28.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "CPower.hpp"

bool CPower::equal(double num1, double num2) {
    if ((num1 - num2 > -0.0000001) && (num1 - num2 < 0.0000001)) {
        return true;
    } else {
        return false;
    }
}

double CPower::powerWithUnsignedExponent(double base, unsigned int exponent) {
//    if (exponent == 0) {
//        return 1;
//    }
//
//    if (exponent == 1) {
//        return base;
//    }
//
//    double result = 1.0;
//    for (unsigned int i = 1; i <= exponent; ++i) {
//        result *= base;
//    }
//
//    return result;
    
    if (exponent == 0) {
        return 1;
    }

    if (exponent == 1) {
        return base;
    }

    double result = powerWithUnsignedExponent(base, exponent >> 1);
    result *= result;

    if ((exponent & 0x1) == 1) {
        result *= base;
    }

    return result;
}

double CPower::power(double base, int exponent) {
    g_InvalidInput = false;
    
    if (equal(base, 0.0) && exponent < 0) {
        g_InvalidInput = true;
        return 0.0;
    }
    
    unsigned int absExponent = (unsigned int)exponent;
    if (exponent < 0) {
        absExponent = (unsigned int)(-exponent);
    }
    
    double result = powerWithUnsignedExponent(base, absExponent);
    if (exponent < 0) {
        result = 1.0 / result;
    }
    
    return result;
}

// 测试代码
void CPower::Test(const char* testName, double base, int exponent, double expectedResult, bool expectedFlag) {
    double result = power(base, exponent);
    if (equal(result, expectedResult) && g_InvalidInput == expectedFlag)
        std::cout << testName << " passed" << std::endl;
    else
        std::cout << testName << " FAILED" << std::endl;
}

void CPower::Test() {
    // 底数、指数都为正数
    Test("Test1", 2, 3, 8, false);

    // 底数为负数、指数为正数
    Test("Test2", -2, 3, -8, false);

    // 指数为负数
    Test("Test3", 2, -3, 0.125, false);

    // 指数为0
    Test("Test4", 2, 0, 1, false);

    // 底数、指数都为0
    Test("Test5", 0, 0, 1, false);

    // 底数为0、指数为正数
    Test("Test6", 0, 4, 0, false);

    // 底数为0、指数为负数
    Test("Test7", 0, -4, 0, true);
}

class Solution {
public:
    bool isEqual(double num1, double num2) {
        if (num1 - num2 > -0.0000001 && num1 - num2 < 0.0000001) {
            return true;
        } else {
            return false;
        }
    }
    
    double myPowCore(double x, unsigned int n) {
        if (n == 0) {
            return 1;
        }
        
        if (n == 1) {
            return x;
        }
        
        double result = myPowCore(x, n >> 1);
        result *= result;
        
        if (n & 0x1) {
            result *= x;
        }
        
        return result;
    }
    
    double myPow(double x, int n) {
        if (isEqual(x, 0.0) && n < 0) {
            return 0.0;
        }
        
        unsigned int m = (unsigned int)n;
        if (n < 0) {
            m = (unsigned int)(-n);
        }
        
        double result = myPowCore(x, m);
        
        if (n < 0) {
            result = 1 / result;
        }
        
        return result;
    }
};
