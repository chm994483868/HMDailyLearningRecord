//
//  ContinousCards.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/11.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "ContinousCards.hpp"

// 给快速排序比较大小规则用的函数
int ContinousCards::compare(const void* arg1, const void* arg2) {
    return *(int*)arg1 - *(int*)arg2;
}

bool ContinousCards::isContinuous(int* numbers, int length) {
    if (numbers == nullptr || length < 1) {
        return false;
    }
    
    // 数组快速排序
    qsort(numbers, length, sizeof(int), compare);
    
    int numberOfZero = 0;
    int numberOfGap = 0;
    
    // 统计数组中 0 的个数
    for (int i = 0; i < length && numbers[i] == 0; ++i) {
        ++numberOfZero;
    }
    
    // 统计数组中的间隔数目
    
    // 因为数组已经是排过序的了，所以数组中如果有 0 的话一定是位于数组前面的。
    
    // 所以这里 numbers[small] 是数组中第一个非零的数字，
    // numbers[big] 则是第二个数字，
    // 即 numbers[small] 和 numbers[big] 是一前一后两个数字
    int small = numberOfZero;
    int big = small + 1;
    
    while (big < length) {
        // 两个数相等，有对子，不可能是顺子
        if (numbers[small] == numbers[big]) {
            return false;
        }
        
        // 统计已排序数组相邻两个数字之间的距离
        numberOfGap += numbers[big] - numbers[small] - 1;
        
        // small 和 big 都前进一步
        small = big;
        big++;
    }
    
    // 如果 0 的个数小于间隔的数目，则无法构成顺子，否则可以构成顺子
    return (numberOfGap > numberOfZero) ? false: true;
}
