//
//  IntegerIdenticalToIndex.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "IntegerIdenticalToIndex.hpp"

// 上面一题是找第一个值和下标不等的值，这一题则是找第一个值和下标相等的值
int IntegerIdenticalToIndex::getNumberSameAsIndex(const int* numbers, int length) {
    // 入参判断
    if (numbers == nullptr || length <= 0) {
        return -1;
    }
    
    int left = 0;
    int right = length - 1;
    
    while (left <= right) {
        // 中间下标
        int middle = left + ((right - left) >> 1);
        
        // 如果中间值相等，则直接返回
        if (numbers[middle] == middle) {
            return middle;
        }
        
        // 如果中间值大于 middle，则表示在左边，缩小 right
        if (numbers[middle] > middle) {
            right = middle - 1;
        } else {
            // 如果大于，则表示在右边，增大 left
            left = middle + 1;
        }
    }
    
    return -1;
}
