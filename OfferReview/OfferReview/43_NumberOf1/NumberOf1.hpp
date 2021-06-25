//
//  NumberOf1.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/31.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef NumberOf1_hpp
#define NumberOf1_hpp

#include <stdio.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>

namespace NumberOf1 {

// 43：从 1 到 n 整数中 1 出现的次数
// 题目：输入一个整数 n，求从 1 到 n 这 n 个整数的十进制表示中 1 出现的次数。例如
// 输入 12，从 1 到 12 这些整数中包含 1 的数字有 1，10，11 和 12，1 一共出现了 5 次。

int numberOf1(unsigned int n);
int numberOf1Between1AndN_Solution1(unsigned int n);

int numberOf1(const char* strN);
int powerBase10(unsigned int n);
int numberOf1Between1AndN_Solution2(int n);

// 测试代码
void Test(const char* testName, int n, int expected);

void Test();

}

#endif /* NumberOf1_hpp */
