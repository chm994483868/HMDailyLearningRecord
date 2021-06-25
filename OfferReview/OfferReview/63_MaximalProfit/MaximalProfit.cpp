//
//  MaximalProfit.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/11.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "MaximalProfit.hpp"

int MaximalProfit::maxDiff(const int* numbers, unsigned length) {
    // 入参判断，至少有一个买入价格和一个卖出价格
    if (numbers == nullptr || length < 2) {
        return 0;
    }
    
    // 最小值从 0 下标元素开始
    int min = numbers[0];
    // 首先记录 numbers[1] 和 numbers[0] 的差价
    int maxDiff = numbers[1] - min;
    
    for (int i = 2; i < length; ++i) {
        
        // 用于记录买入最小值
        if (numbers[i - 1] < min) {
            min = numbers[i - 1];
        }
        
        // 记录当前价格和最小值的差价
        int currentDiff = numbers[i] - min;
        
        // 判断 maxDiff 记录最大差价
        if (currentDiff > maxDiff) {
            maxDiff = currentDiff;
        }
    }
    
    // 返回最大差价
    return maxDiff;
}
