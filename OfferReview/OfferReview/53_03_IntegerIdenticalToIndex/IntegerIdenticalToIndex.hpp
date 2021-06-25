//
//  IntegerIdenticalToIndex.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef IntegerIdenticalToIndex_hpp
#define IntegerIdenticalToIndex_hpp

#include <stdio.h>
#include <cstdio>

namespace IntegerIdenticalToIndex {

// 面试题53（三）：数组中数值和下标相等的元素
// 题目：假设一个单调递增的数组里的每个元素都是整数并且是唯一的。请编程实
// 现一个函数找出数组中任意一个数值等于其下标的元素。例如，在数组{-3, -1,
// 1, 3, 5}中，数字3和它的下标相等。

int getNumberSameAsIndex(const int* numbers, int length);

}


#endif /* IntegerIdenticalToIndex_hpp */
