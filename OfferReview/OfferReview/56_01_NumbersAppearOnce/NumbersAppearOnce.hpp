//
//  NumbersAppearOnce.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef NumbersAppearOnce_hpp
#define NumbersAppearOnce_hpp

#include <stdio.h>
#include <cstdio>

namespace NumbersAppearOnce {

// 面试题56（一）：数组中只出现一次的两个数字
// 题目：一个整型数组里除了两个数字之外，其他的数字都出现了两次。请写程序
// 找出这两个只出现一次的数字。要求时间复杂度是 O(n)，空间复杂度是 O(1)。

unsigned int findFirstBitIs1(int num);
bool isBit1(int num, unsigned int indexBit);
void findNumsAppearOnce(int data[], int length, int* num1, int* num2);

}

#endif /* NumbersAppearOnce_hpp */
