//
//  MaxValueOfGifts.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/31.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef MaxValueOfGifts_hpp
#define MaxValueOfGifts_hpp

#include <stdio.h>
#include <algorithm>
#include <iostream>

namespace MaxValueOfGifts {

// 47：礼物的最大价值
// 题目：在一个m×n的棋盘的每一格都放有一个礼物，每个礼物都有一定的价值
// （价值大于0）。你可以从棋盘的左上角开始拿格子里的礼物，并每次向左或
// 者向下移动一格直到到达棋盘的右下角。给定一个棋盘及其上面的礼物，请计
// 算你最多能拿到多少价值的礼物？
int getMaxValue_solution1(const int* values, int rows, int cols);
int getMaxValue_solution2(const int* values, int rows, int cols);

// 测试代码
void test(const char* testName, const int* values, int rows, int cols, int expected);
void test1();
void test2();
void test3();
void test4();
void test5();
void test6();

void Test();

}

#endif /* MaxValueOfGifts_hpp */
