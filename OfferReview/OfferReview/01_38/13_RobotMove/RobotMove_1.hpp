//
//  RobotMove_1.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/2.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef RobotMove_1_hpp
#define RobotMove_1_hpp

#include <stdio.h>

namespace RobotMove_1 {

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

#endif /* RobotMove_1_hpp */
