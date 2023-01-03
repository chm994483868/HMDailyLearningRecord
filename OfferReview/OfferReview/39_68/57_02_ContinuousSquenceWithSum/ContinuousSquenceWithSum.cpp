//
//  ContinuousSquenceWithSum.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "ContinuousSquenceWithSum.hpp"

// [1, 2, 3, 4, ...]
void ContinuousSquenceWithSum::findContinuousSequence(int sum) {
    // 连续正数，两个起步，和至少是 1 + 2 = 3
    if (sum < 3) {
        return;
    }
    
    // 分别从正数 1 和 2 开始
    int small = 1;
    int big = 2;
    
    // 最大到 middle
    int middle = (1 + sum) / 2;
    
    // 当前的和
    int curSum = small + big;
    
    while (small < middle) {
        if (curSum == sum) {
            printContinuousSequence(small, big);
        }
        
        // 如果 curSum 大于要求的和，把 small 减去
        while (curSum > sum && small < middle) {
            
            // curSum 减去 small，然后 small 自增
            curSum -= small;
            small++;
            
            if (curSum == sum) {
                printContinuousSequence(small, big);
            }
        }
        
        // 增大 big
        big++;
        curSum += big;
    }
}

// 打印 small 到 bit 的连续正数
void ContinuousSquenceWithSum::printContinuousSequence(int small, int big) {
    for (int i = small; i <= big; ++i) {
        printf("%d ", i);
    }
    
    printf("\n");
}
