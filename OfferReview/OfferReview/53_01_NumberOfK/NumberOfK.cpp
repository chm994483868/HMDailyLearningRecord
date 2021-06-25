//
//  NumberOfK.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "NumberOfK.hpp"

// 找到数组中第一个 k 的下标，如果数组中不存在 k，返回 -1
int NumberOfK::getFirstK(const int* data, int length, int k, int start, int end) {
    if (start > end) {
        return -1;
    }
    
    // 数组中间下标
    int middleIndex = (start + end) / 2;
    // 数组中间值
    int middleData = data[middleIndex];
    
    if (middleData == k) {
        // 如果中间值正是 k
        if ((middleIndex > 0 && data[middleIndex - 1] != k) || middleIndex == 0) {
            // 如果中间下标大于 0，且中间下标前面一个下标的值不是 k，或者中间下标是 0，表示当前的 k 已经是数组的第一个 k
            return middleIndex;
        } else {
            // 否则当前 k 不是第一个 k，第一个 k 在左边，所以更新 end
            end = middleIndex - 1;
        }
    } else if (middleData > k) {
        // 如果中间值大于 k，表示第一个 k 在左边，同样更新 end
        end = middleIndex - 1;
    } else {
        // 如果小于 k，则表示第一个 k 在右边，则更新 start
        start = middleIndex + 1;
    }
    
    // 递归查找第一个 k
    return getFirstK(data, length, k, start, end);
}

// 同上
// 找到数组中最后一个 k 的下标。如果数组中不存在 k，返回 -1
int NumberOfK::getLastK(const int* data, int length, int k, int start, int end) {
    if (start > end) {
        return -1;
    }
    
    int middleIndex = (start + end) / 2;
    int middleData = data[middleIndex];
    
    if (middleData == k) {
        if ((middleIndex < length - 1 && data[middleIndex + 1] != k) || middleIndex == length - 1) {
            return middleIndex;
        } else {
            start = middleIndex + 1;
        }
    } else if (middleData < k) {
        start = middleIndex + 1;
    } else {
        end = middleIndex - 1;
    }
    
    return getLastK(data, length, k, start, end);
}

int NumberOfK::getNumberOfK(const int* data, int length, int k) {
    int number = 0;
    
    if (data != nullptr && length > 0) {
        // 找到第一个 k 的位置，找到最后一个 k 的位置
        int first = getFirstK(data, length, k, 0, length - 1);
        int last = getLastK(data, length, k, 0, length - 1);
        
        // 两者的距离，即是 k 重复的次数
        if (first > - 1 && last > -1) {
            number = last - first + 1;
        }
    }
    
    // 返回重复次数
    return number;
}
