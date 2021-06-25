//
//  MoreThanHalfNumber_1.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/6.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef MoreThanHalfNumber_1_hpp
#define MoreThanHalfNumber_1_hpp

#include <stdio.h>
#include "Array.hpp"

using namespace Partition;
using namespace std;

namespace MoreThanHalfNumber_1 {

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

#endif /* MoreThanHalfNumber_1_hpp */
