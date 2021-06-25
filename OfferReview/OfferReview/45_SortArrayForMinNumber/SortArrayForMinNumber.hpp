//
//  SortArrayForMinNumber.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/31.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef SortArrayForMinNumber_hpp
#define SortArrayForMinNumber_hpp

#include <stdio.h>
#include <string>
#include <algorithm>

namespace SortArrayForMinNumber {

// 45：把数组排成最小的数
// 题目：输入一个正整数数组，把数组里所有数字拼接起来排成一个数，打印能拼
// 接出的所有数字中最小的一个。例如输入数组{3, 32, 321}，则打印出这3个数
// 字能排成的最小数字321323。
int compare(const void* strNumber1, const void* strNumber2);

// int 型整数用十进制表示最多只有 10 位
const int g_MaxNumberLength = 10;

char* g_StrCombine1 = new char[g_MaxNumberLength * 2 + 1];
char* g_StrCombine2 = new char[g_MaxNumberLength * 2 + 1];

void PrintMinNumber(const int* numbers, int length);

// 测试代码
void Test(const char* testName, int* numbers, int length, const char* expectedResult);

void Test1();
void Test2();
void Test3();
void Test4();
void Test5();
void Test6();

void Test();

}

#endif /* SortArrayForMinNumber_hpp */
