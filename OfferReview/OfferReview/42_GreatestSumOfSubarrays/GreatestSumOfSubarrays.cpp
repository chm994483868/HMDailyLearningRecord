//
//  GreatestSumOfSubarrays.cpp
//  OfferReview
//
//  Created by CHM on 2020/7/31.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "GreatestSumOfSubarrays.hpp"

// g_InvalidInput 表示入参是否无效，
// 如输入的数组参数为空指针、数组长度小于等于 0 的情况，此时我们让函数返回什么数字呢？
// 如果返回 0，那么又怎么区分子数组的和的最大值是 0 和无效的输入这两种情况呢？
// 因此定义一个全局变量来标记是否输入参数无效。

int GreatestSumOfSubarrays::findGreatestSumOfSubArray(int* pData, int nLength) {
    if (pData == nullptr || nLength <= 0) {
        g_InvalidInput = true;
        return 0;
    }
    
    g_InvalidInput = false;
    
    // 记录当前的和
    int nCurSum = 0;
    
    // 用于记录出现过的最大的和（这里由最大的负数开始）
    int nGreatestSum = 0x80000000;
    
    // 遍历开始
    for (unsigned int i = 0; i < nLength; ++i) {
        
        // 首先判断当前的和是否小于 0，如果小于 0 则前面的几个连续数字的和可以直接抛弃了，
        // 并直接从当前的 pData[i] 开始从新记录 nCurSum 的值
        if (nCurSum <= 0) {
            nCurSum = pData[i];
        } else {
            // 如果 nCurSum 大于 0，则继续累加当前的 pData[i]
            nCurSum += pData[i];
        }
        
        // 判断当前的和是否是出现过的最大和 nGreatestSum
        if (nCurSum > nGreatestSum) {
            nGreatestSum = nCurSum;
        }
    }
    
    // 返回最大的和
    return nGreatestSum;
}

// 测试代码
void GreatestSumOfSubarrays::Test(char* testName, int* pData, int nLength, int expected, bool expectedFlag) {
    if(testName != nullptr)
        printf("%s begins: \n", testName);

    int result = findGreatestSumOfSubArray(pData, nLength);
    if(result == expected && expectedFlag == g_InvalidInput)
        printf("Passed.\n");
    else
        printf("Failed.\n");
}

// 1, -2, 3, 10, -4, 7, 2, -5
void GreatestSumOfSubarrays::Test1() {
    int data[] = {1, -2, 3, 10, -4, 7, 2, -5};
    Test("Test1", data, sizeof(data) / sizeof(int), 18, false);
}

// 所有数字都是负数
// -2, -8, -1, -5, -9
void GreatestSumOfSubarrays::Test2() {
    int data[] = {-2, -8, -1, -5, -9};
    Test("Test2", data, sizeof(data) / sizeof(int), -1, false);
}

// 所有数字都是正数
// 2, 8, 1, 5, 9
void GreatestSumOfSubarrays::Test3() {
    int data[] = {2, 8, 1, 5, 9};
    Test("Test3", data, sizeof(data) / sizeof(int), 25, false);
}

// 无效输入
void GreatestSumOfSubarrays::Test4() {
    Test("Test4", nullptr, 0, 0, true);
}

void GreatestSumOfSubarrays::Test() {
    Test1();
    Test2();
    Test3();
    Test4();
}
