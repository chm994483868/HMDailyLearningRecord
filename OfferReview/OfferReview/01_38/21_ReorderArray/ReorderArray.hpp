//
//  ReorderArray.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/29.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef ReorderArray_hpp
#define ReorderArray_hpp

#include <stdio.h>

namespace ReorderArray {

// 21：调整数组顺序使奇数位于偶数前面
// 题目：输入一个整数数组，实现一个函数来调整该数组中数字的顺序，使得所有
// 奇数位于数组的前半部分，所有偶数位于数组的后半部分。
void reorder(int* pData, unsigned int length, bool(*func)(int));
bool isEven(int n);
void reorderOddEven_2(int* pData, unsigned int length);
    
// 测试代码
void PrintArray(int numbers[], int length);
void Test(char* testName, int numbers[], int length);
void Test1();
void Test2();
void Test3();
void Test4();
void Test5();
void Test6();

void Test();

}

#endif /* ReorderArray_hpp */
