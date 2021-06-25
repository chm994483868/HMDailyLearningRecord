//
//  PrintMatrix.hpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/17.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef PrintMatrix_hpp
#define PrintMatrix_hpp

// 顺时针打印矩阵
#include <stdio.h>
#include <cstdio>

void printMatrixInCircle(int** numbers, int columns, int rows, int start);
void printNumber(int number);
void printMatrixClockwisely(int** numbers, int columns, int rows);

#endif /* PrintMatrix_hpp */
