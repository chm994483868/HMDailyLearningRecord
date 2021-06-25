//
//  MissingNumber.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "MissingNumber.hpp"


// 只有一个数字不在该数组中，即从该数字开始数字和下标和值是不相等的了，
// 如: [0, 1, 2, 4, 5, 6] 4 位于下标 3 处，则自 3 下标以后，下标值和数组值都不再相等了。
int MissingNumber::getMissingNumber(const int* numbers, int length) {
    if (numbers == nullptr || length <= 0) {
        return -1;
    }
    
    int left = 0;
    int right = length - 1;
    
    while (left <= right) {
        // 中间下标
        int middle = (right + left) >> 1;
        
        if (numbers[middle] != middle) {
            // 如果中间下标值和数组值不相等
            if (middle == 0 || numbers[middle - 1] == middle - 1) {
                // 如果中间下标是 0 下标，或者 middle - 1 下标等于 numbers[middle - 1]，
                // 即表示 middle 便是第一个和自己下标不相等的值，直接返回
                return middle;
            }
            
            // 如果不是的话，表示第一个与自己下标不等的值在左边，缩小 right
            right = middle - 1;
        } else {
            // 否则在右边，增大 left
            left = middle + 1;
        }
    }
    
    // 一直到最右边，则返回 length
    if (left == length) {
        return length;
    }
    
    // 无效的输入，比如数组不是按要求排序的，
    // 或者有数字不在 0 到 n-1 范围之内
    return -1;
}
