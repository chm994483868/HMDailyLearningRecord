//
//  FindDuplicationNoEdit.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/27.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef FindDuplicationNoEdit_hpp
#define FindDuplicationNoEdit_hpp

#include <stdio.h>
#include <iostream>

namespace FindDuplicationNoEdit {

// 3.(二)：不修改数组找出重复的数字
// 题目：在一个长度为 n+1 的数组里的所有数字都在 1 到 n 的范围内，所以数组中至
// 少有一个数字是重复的。请找出数组中任意一个重复的数字，但不能修改输入的数组。
// 例如，如果输入长度为 8 的数组 {2, 3, 5, 4, 3, 2, 6, 7}，那么对应的
// 输出是重复的数字2或者3。

int countRange(const int* numbers, int length, int start, int end);

/// 不修改数组找出重复的数字, 正数 输入有效，并且数组中存在重复的数字，返回值即为重复的数字
/// @param number 数组
/// @param length 数组长度
int getDuplication(const int* number, int length);

// 测试代码
void test(const char* testName, int* numbers, int length, int* duplications, int dupLength);
// 多个重复的数字
void test1();
// 一个重复的数字
void test2();
// 重复的数字是数组中最小的数字
void test3();
// 重复的数字是数组中最大的数字
void test4();
// 数组中只有两个数字
void test5();
// 重复的数字位于数组当中
void test6();
// 多个重复的数字
void test7();
// 一个数字重复三次
void test8();
// 没有重复的数字
void test9();
// 无效的输入
void test10();

void Test();

}

#endif /* FindDuplicationNoEdit_hpp */
