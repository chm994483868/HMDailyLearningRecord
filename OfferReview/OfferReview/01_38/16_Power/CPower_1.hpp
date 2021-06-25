//
//  CPower_1.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/3.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef CPower_1_hpp
#define CPower_1_hpp

#include <stdio.h>
#include <iostream>

namespace CPower_1 {

// 16：数值的整数次方
// 题目：实现函数double Power(double base, int exponent)，求 base 的 exponent 次方。
// 不得使用库函数，同时不需要考虑大数问题。

static bool g_InvalidInput = false;
bool equal(double num1, double num2);
double powerWithUnsignedExponent(double base, unsigned int exponent);

double power(double base, int exponent);

// 测试代码
void Test(const char* testName, double base, int exponent, double expectedResult, bool expectedFlag);

void Test();

}

#endif /* CPower_1_hpp */
