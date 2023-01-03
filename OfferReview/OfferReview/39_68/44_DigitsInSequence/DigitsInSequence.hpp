//
//  DigitsInSequence.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/31.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef DigitsInSequence_hpp
#define DigitsInSequence_hpp

#include <stdio.h>
#include <math.h>
#include <iostream>
#include <algorithm>

using namespace std;

namespace DigitsInSequence {

// 44：数字序列中某一位的数字
// 题目：数字以0123456789101112131415⋯的格式序列化到一个字符序列中。在这
// 个序列中，第5位（从0开始计数）是5，第13位是1，第19位是4，等等。请写一
// 个函数求任意位对应的数字。

int countOfIntegers(int digits);
int digitAtIndex(int index, int digits);
int beginNumber(int digits);

int digitAtIndex(int index);

// 测试代码
void test(const char* testName, int inputIndex, int expectedOutput);

void Test();

}

#endif /* DigitsInSequence_hpp */
