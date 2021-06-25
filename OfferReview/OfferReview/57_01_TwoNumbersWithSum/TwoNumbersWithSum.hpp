//
//  TwoNumbersWithSum.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef TwoNumbersWithSum_hpp
#define TwoNumbersWithSum_hpp

#include <stdio.h>
#include <cstdio>

namespace TwoNumbersWithSum {

// 面试题57（一）：和为 s 的两个数字
// 题目：输入一个递增排序的数组和一个数字 s，在数组中查找两个数，使得它们
// 的和正好是 s。如果有多对数字的和等于 s，输出任意一对即可。

bool findNumbersWithSum(int data[], int length, int sum, int* num1, int* num2);

}
#endif /* TwoNumbersWithSum_hpp */
