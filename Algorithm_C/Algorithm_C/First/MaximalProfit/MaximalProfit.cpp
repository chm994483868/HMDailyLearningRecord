//
//  MaximalProfit.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/26.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "MaximalProfit.hpp"

int maxDiff(const int* numbers, unsigned int length) {
    if (numbers == nullptr && length < 2) {
        return 0;
    }
    
    // 1. min 从 0 开始
    int min = numbers[0];
    // 2. 首先取得 1 和 0 的差价并初始化为当前的最大差价
    int maxDiff = numbers[1] - min;
    
    // 3. 然后遍历
    for (int i = 2; i < length; ++i) {
        
        // 4. 更新当前的最小值
        if (numbers[i - 1] < min) {
            min = numbers[i - 1];
        }
        
        // 5. 取得当前值与最小值的差价
        int currentDiff = numbers[i] - min;
        
        // 6. 更新最大的差价值
        if (currentDiff > maxDiff) {
            maxDiff = currentDiff;
        }
    }
    
    return maxDiff;
}
