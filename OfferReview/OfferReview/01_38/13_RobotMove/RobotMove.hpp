//
//  RobotMove.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/28.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef RobotMove_hpp
#define RobotMove_hpp

#include <stdio.h>

namespace RobotMove {

// 13：机器人的运动范围
// 题目：地上有一个m行n列的方格。一个机器人从坐标(0, 0)的格子开始移动，它
// 每一次可以向左、右、上、下移动一格，但不能进入行坐标和列坐标的数位之和
// 大于k的格子。例如，当k为18时，机器人能够进入方格(35, 37)，因为3+5+3+7=18。
// 但它不能进入方格(35, 38)，因为3+5+3+8=19。请问该机器人能够到达多少个格子？
int movingCoungCore(int threshold, int rows, int cols, int row, int col, bool* visited);
bool check(int threshold, int rows, int cols, int row, int col, bool* visited);
int getDigitSum(int number);
int movingCount(int threshold, int rows, int cols);

// 测试代码
void test(char* testName, int threshold, int rows, int cols, int expected);
// 方格多行多列
void test1();
// 方格多行多列
void test2();
// 方格只有一行，机器人只能到达部分方格
void test3();
// 方格只有一行，机器人能到达所有方格
void test4();
// 方格只有一列，机器人只能到达部分方格
void test5();
// 方格只有一列，机器人能到达所有方格
void test6();
// 方格只有一行一列
void test7();
// 方格只有一行一列
void test8();
// 机器人不能进入任意一个方格
void test9();

void Test();

}

#endif /* RobotMove_hpp */
