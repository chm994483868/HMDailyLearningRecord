//
//  GreatestSumOfSubarrays.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/31.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef GreatestSumOfSubarrays_hpp
#define GreatestSumOfSubarrays_hpp

#include <stdio.h>
#include <cstdio>

namespace GreatestSumOfSubarrays {

// 42：连续子数组的最大和
// 题目：输入一个整型数组，数组里有正数也有负数。数组中一个或连续的多个整
// 数组成一个子数组。求所有子数组的和的最大值。要求时间复杂度为O(n)。

static bool g_InvalidInput = false;
int findGreatestSumOfSubArray(int* pData, int nLength);

// 测试代码
void Test(char* testName, int* pData, int nLength, int expected, bool expectedFlag);

// 1, -2, 3, 10, -4, 7, 2, -5
void Test1();
// 所有数字都是负数
// -2, -8, -1, -5, -9
void Test2();
// 所有数字都是正数
// 2, 8, 1, 5, 9
void Test3();
// 无效输入
void Test4();

void Test();

}

#endif /* GreatestSumOfSubarrays_hpp */
