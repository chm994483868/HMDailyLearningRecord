//
//  MissingNumber.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef MissingNumber_hpp
#define MissingNumber_hpp

#include <stdio.h>
#include <cstdio>

namespace MissingNumber {

// 面试题53（二）：0 到 n-1 中缺失的数字
// 题目：一个长度为n-1的递增排序数组中的所有数字都是唯一的，并且每个数字
// 都在范围0到n-1之内。在范围0到n-1的n个数字中有且只有一个数字不在该数组
// 中，请找出这个数字。

int getMissingNumber(const int* numbers, int length);

}

#endif /* MissingNumber_hpp */
