//
//  CPower.hpp
//  OfferReview
//
//  Created by HM C on 2020/7/28.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef CPower_hpp
#define CPower_hpp

#include <stdio.h>
#include <iostream>

using namespace std;

namespace CPower {

// 16：数值的整数次方
// 题目：实现函数double Power(double base, int exponent)，求base的exponent
// 次方。不得使用库函数，同时不需要考虑大数问题。
static bool g_InvalidInput = false;
bool equal(double num1, double num2);
double powerWithUnsignedExponent(double base, unsigned int exponent);

double power(double base, int exponent);

// 测试代码
void Test(const char* testName, double base, int exponent, double expectedResult, bool expectedFlag);

void Test();

}

#endif /* CPower_hpp */
