//
//  CuttingRope_1.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/3.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef CuttingRope_1_hpp
#define CuttingRope_1_hpp

#include <stdio.h>
#include <iostream>
#include <cmath>

namespace CuttingRope_1 {

// 14：剪绳子
// 题目：给你一根长度为 n 绳子，请把绳子剪成 m 段（m、n 都是整数，n>1 并且 m≥1）。
// 每段的绳子的长度记为 k[0]、k[1]、⋯⋯、k[m]。k[0] * k[1] * ⋯ *k[m] 可能的最大乘积是多少？
// 例如当绳子的长度是8时，我们把它剪成长度分别为2、3、3的三段，此时得到最大的乘积18。

// 动态规划
int maxProductAfterCutting_solution1(int length);
// 贪婪算法
int maxProductAfterCutting_solution2(int length);

// 测试代码
void test(const char* testName, int length, int expected);
void test1();
void test2();
void test3();
void test4();
void test5();
void test6();
void test7();
void test8();
void test9();
void test10();
void test11();

void Test();

}


#endif /* CuttingRope_1_hpp */
