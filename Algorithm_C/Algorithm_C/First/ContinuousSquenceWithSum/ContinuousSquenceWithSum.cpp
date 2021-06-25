//
//  ContinuousSquenceWithSum.cpp
//  Algorithm_C
//
//  Created by HM C on 2020/7/25.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "ContinuousSquenceWithSum.hpp"

void printContinuousSequence(int small, int big);

void findContinuousSequence(int sum) {
    // 1. sum 为 2 时，已经无法满足题目要求，这里从 3 开始
    if (sum < 3) {
        return;
    }
    
    // 2. small 从 1 开始，big 从 2 开始
    int small = 1;
    int big = 2;
    // 3. 这个 middle 的取值，比如 sum 等于 9 时，middle 等于 5，sum = 8 时，middle 等于 4
    int middle = (1 + sum) / 2;
    // 4. 记录当前的和
    int curSum = small + big;
    // 5. small 要小于 middle，不然无法满足连续正数序列至少含有两个数， small 必须小于 sum 的一半
    while (small < middle) {
        if (curSum == sum) {
            printContinuousSequence(small, big);
        }
        
        // 6. 如果当前和大于 sum，则右移 small
        while (curSum > sum && small < middle) {
            curSum -= small;
            ++small;
            
            if (curSum == sum) {
                printContinuousSequence(small, big);
            }
        }
        
        // 7. 增大 big 找下一个满足条件的序列
        ++big;
        curSum += big;
    }
}

// 打印 small 到 big 的值
void printContinuousSequence(int small, int big) {
    for (int i = small; i <= big; ++i) {
        printf("%d", i);
    }
    
    printf("\n");
}
