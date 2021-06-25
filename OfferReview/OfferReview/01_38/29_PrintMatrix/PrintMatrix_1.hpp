//
//  PrintMatrix_1.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/5.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef PrintMatrix_1_hpp
#define PrintMatrix_1_hpp

#include <stdio.h>
#include <cstdio>

namespace PrintMatrix_1 {

void PrintMatrixInCircle(int** numbers, int colums, int rows, int start);
void printNumber(int number);
void PrintMatrixClockwisely(int** numbers, int colums, int rows);

// 测试代码
void Test(int colums, int rows);
void Test();

}


#endif /* PrintMatrix_1_hpp */
