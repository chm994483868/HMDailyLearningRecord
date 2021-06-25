//
//  PrintMatrix.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/5.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef PrintMatrix_hpp
#define PrintMatrix_hpp

#include <stdio.h>
#include <cstdio>

namespace PrintMatrix {

// 29：顺时针打印矩阵
// 题目：输入一个矩阵，按照从外向里以顺时针的顺序依次打印出每一个数字。

void PrintMatrixInCircle(int** numbers, int colums, int rows, int start);
void printNumber(int number);
void PrintMatrixClockwisely(int** numbers, int colums, int rows);

// 测试代码
void Test(int colums, int rows);
void Test();

}

#endif /* PrintMatrix_hpp */
