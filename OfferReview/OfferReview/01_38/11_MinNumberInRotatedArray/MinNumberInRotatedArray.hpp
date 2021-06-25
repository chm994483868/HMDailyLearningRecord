//
//  MinNumberInRotatedArray.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/28.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef MinNumberInRotatedArray_hpp
#define MinNumberInRotatedArray_hpp

#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <vector>

using namespace std;

namespace MinNumberInRotatedArray {

// 开局相关题目：
// 快速排序
unsigned int randomInRange(unsigned int start, unsigned int end);
int partition(int data[], int length, int start, int end);
void swap(int* num1, int* num2);
void quickSort(int data[], int length, int start, int end);
// 员工年龄排序（计数排序）
void sortAges(int ages[], int length);

// 11：旋转数组的最小数字
// 题目：把一个数组最开始的若干个元素搬到数组的末尾，我们称之为数组的旋转。
// 输入一个递增排序的数组的一个旋转，输出旋转数组的最小元素。例如数组
// {3, 4, 5, 1, 2} 为 {1, 2, 3, 4, 5}的一个旋转，该数组的最小值为 1。

int minInorder(int* numbers, int index1, int index2);
int min(int* numbers, int length);

// 测试代码
void Test(int* numbers, int length, int expected);

void Test();

}

#endif /* MinNumberInRotatedArray_hpp */
