//
//  GreatestSumOfSubarrays.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/22.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "GreatestSumOfSubarrays.hpp"

namespace GreatestSumOfSubarrays {

bool g_InvalidInput = false;
int findGreatestSumOfSubArray(int* pData, int nLength) {
    if (pData == nullptr || nLength <= 0) {
        g_InvalidInput = true;
        return 0;
    }
    
    g_InvalidInput = false;
    int nCurSum = 0;
    int nGreatestSum = 0x80000000;
    
    for (int i = 0; i < nLength; ++i) {
        if (nCurSum <= 0) {
            // 这里是除了 刚开始时 nCurSum 等于 0
            // 其它情况下，如果当前 nCurSum 是小于等于 0 的值
            // 那么它和后面的任何值相加都不会得到比之前的和最大时的值大
            // 且可以直接撇弃前面连续元素的累加和了
            //（它们累加的和是小于等于 0 的情况，那要它们还有啥用）
            nCurSum = pData[i];
        } else {
            nCurSum += pData[i];
        }
        
        if (nCurSum > nGreatestSum) {
            nGreatestSum = nCurSum;
        }
    }
    
    return nGreatestSum;
}

int findGreatestSumOfSubArray_Review(int* pData, int nLength) {
    if (pData == nullptr || nLength <= 0) {
        g_InvalidInput = true;
        return 0;
    }
    
    g_InvalidInput = false;
    int nCurSum = 0;
    int nGreatestSum = 0x80000000;
    
    for (int i = 0; i < nLength; ++i) {
        if (nCurSum <= 0) {
            nCurSum = pData[i];
        } else {
            nCurSum += pData[i];
        }
        
        if (nCurSum > nGreatestSum) {
            nGreatestSum = nCurSum;
        }
    }
    
    return nGreatestSum;
}

}
