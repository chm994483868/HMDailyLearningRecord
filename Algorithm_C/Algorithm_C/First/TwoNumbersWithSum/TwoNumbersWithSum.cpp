//
//  TwoNumbersWithSum.cpp
//  Algorithm_C
//
//  Created by HM C on 2020/7/25.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "TwoNumbersWithSum.hpp"

bool findNumbersWithSum(int data[], int length, int sum, int* num1, int* num2) {
    // 1. 初始一个标记
    bool found = false;
    if (data == nullptr || length < 1 || num1 == nullptr || num2 == nullptr) {
        return found;
    }
    
    // 2. 两个标记，开头和结尾
    int aHead = length - 1;
    int behind = 0;
    
    while (aHead > behind) {
        // 3. 当前两个值的和
        long long curSum = data[aHead] + data[behind];
        // 4. 如果相等即表示找到了
        if (curSum == sum) {
            *num1 = data[behind];
            *num2 = data[aHead];
            found = true;
            break;
        } else if (curSum > sum) {
            // 5. 大于的话，左移
            --aHead;
        } else {
            // 6. 小于的话，右移
            ++behind;
        }
    }
    
    return found;
}
