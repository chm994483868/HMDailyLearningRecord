//
//  MoreThanHalfNumber.hpp
//  OfferReview
//
//  Created by HM C on 2020/7/30.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef MoreThanHalfNumber_hpp
#define MoreThanHalfNumber_hpp

#include <stdio.h>
#include "Array.hpp"

using namespace Partition;
using namespace std;

namespace MoreThanHalfNumber {

// 39：数组中出现次数超过一半的数字
// 题目：数组中有一个数字出现的次数超过数组长度的一半，请找出这个数字。例
// 如输入一个长度为9的数组{1, 2, 3, 2, 2, 2, 5, 4, 2}。由于数字2在数组中
// 出现了5次，超过数组长度的一半，因此输出2。

static bool g_bInputInvalid = false;
bool checkInvalidArray(int* numbers, int length);
bool checkMoreThanHalf(int* numbers, int length, int number);

int moreThanHalfNum_Solution1(int* numbers, int length);
int moreThanHalfNum_Solution2(int* numbers, int length);

// 测试代码
void Test(char* testName, int* numbers, int length, int expectedValue, bool expectedFlag);
// 存在出现次数超过数组长度一半的数字
void Test1();
// 不存在出现次数超过数组长度一半的数字
void Test2();
// 出现次数超过数组长度一半的数字都出现在数组的前半部分
void Test3();
// 出现次数超过数组长度一半的数字都出现在数组的后半部分
void Test4();
// 输入空指针
void Test5();
// 输入空指针
void Test6();

void Test();

}

#endif /* MoreThanHalfNumber_hpp */
