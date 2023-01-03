//
//  NumberOfK.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef NumberOfK_hpp
#define NumberOfK_hpp

#include <stdio.h>
#include <cstdio>

namespace NumberOfK {

// 面试题53（一）：数字在排序数组中出现的次数
// 题目：统计一个数字在排序数组中出现的次数。例如输入排序数组{1, 2, 3, 3,
// 3, 3, 4, 5}和数字3，由于3在这个数组中出现了4次，因此输出4。

int getFirstK(const int* data, int length, int k, int start, int end);
int getLastK(const int* data, int length, int k, int start, int end);
int getNumberOfK(const int* data, int length, int k);

}

#endif /* NumberOfK_hpp */
