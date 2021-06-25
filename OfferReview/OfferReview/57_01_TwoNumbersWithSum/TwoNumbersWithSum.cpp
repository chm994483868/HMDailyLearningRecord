//
//  TwoNumbersWithSum.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "TwoNumbersWithSum.hpp"

// 准备两个指针，一个从数组头开始，一个从数组尾开始，计算两个指针的和，
// 如果和小于 sum，则左边指针前移，如果和大于 sum，则右边指针后移。
bool TwoNumbersWithSum::findNumbersWithSum(int data[], int length, int sum, int* num1, int* num2) {
    bool found = false;
    if (length < 1 || num1 == nullptr || num2 == nullptr) {
        return found;
    }
    
    // 左边
    int ahead = length - 1;
    // 右边
    int behind = 0;
    
    while (ahead > behind) {
        // 求和
        long long curSum = data[ahead] + data[behind];
        
        if (curSum == sum) {
            // 如果和等于 sum，用 num1 和 num2 记录
            *num1 = data[behind];
            *num2 = data[ahead];
            
            // 标记找到
            found = true;
            
            // 已经找到，则跳出循环
            break;
        } else if (curSum > sum) {
            // 和太大了，左移
            --ahead;
        } else {
            // 和太小了，右移
            ++behind;
        }
    }
    
    return found;
}
