//
//  ContinousCards.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/26.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "ContinousCards.hpp"

namespace ContinousCards {

int compare(const void* arg1, const void* arg2);
bool isContinuous(int* numbers, int length) {
    if (numbers == nullptr || length < 1) {
        return false;
    }
    
    qsort(numbers, length, sizeof(int), compare);
    
    int numberOfZero = 0;
    int numberOfGap = 0;
    
    // 统计数组中 0 的个数
    for (int i = 0; i < length && numbers[i] == 0; ++i) {
        ++numberOfZero;
    }
    
    // 统计数组中的间隔数目
    // 因为数组排序后 0 都在前面，small 则从第一个不是 0 的元素开始
    int small = numberOfZero;
    int big = small + 1;
    while (big < length) {
        // 两个数相等，有对子，不可能是顺子
        if (numbers[small] == numbers[big]) {
            return false;
        }
        
        numberOfGap += numbers[big] - numbers[small] - 1;
        small = big;
        ++big;
    }
    
    return (numberOfGap > numberOfZero) ? false: true;
}

int compare(const void* arg1, const void* arg2) {
    return *(int*)arg1 - *(int*)arg2;
}

}
