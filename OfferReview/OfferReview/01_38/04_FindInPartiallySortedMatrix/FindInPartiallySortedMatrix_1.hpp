//
//  FindInPartiallySortedMatrix_1.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/1.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef FindInPartiallySortedMatrix_1_hpp
#define FindInPartiallySortedMatrix_1_hpp

#include <stdio.h>
#include <cstdio>

namespace FindInPartiallySortedMatrix_1 {

// 4：二维数组中的查找
// 题目：在一个二维数组中，每一行都按照从左到右递增的顺序排序，每一列都按
// 照从上到下递增的顺序排序。请完成一个函数，输入这样的一个二维数组和一个
// 整数，判断数组中是否含有该整数。

bool find(int* matrix, int rows, int colums, int number);

// 测试代码
void Test(char* testName, int* matrix, int rows, int columns, int number, bool expected);
// 要查找的数在数组中
void Test1();
// 要查找的数不在数组中
void Test2();
// 要查找的数是数组中最小的数字
void Test3();
// 要查找的数是数组中最大的数字
void Test4();
// 要查找的数比数组中最小的数字还小
void Test5();
// 要查找的数比数组中最大的数字还大
void Test6();
// 鲁棒性测试，输入空指针
void Test7();

void Test();

}

#endif /* FindInPartiallySortedMatrix_1_hpp */
