//
//  UglyNumber.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "UglyNumber.hpp"

// 判断一个数字是否是丑数
bool UglyNumber::IsUgly(int number) {
    // 任意丑数对 2/3/5 其中一个取模必是 0，
    // 然后连续取商的话最后必是 1，即最后只需要判断 number 是否是 1 即可。
    while (number % 2 == 0) {
        number /= 2;
    }
    while (number % 3 == 0) {
        number /= 3;
    }
    while (number % 5 == 0) {
        number /= 5;
    }
    
    return (number == 1) ? true: false;
}

// 从 0 开始遍历每一个整数，判断该整数是否是整数，然后记录下是第几个丑数，直到第 index 个丑数
int UglyNumber::GetUglyNumber_Solution1(int index) {
    if (index <= 0) {
        return 0;
    }
    
    int number = 0;
    int uglyFound = 0;
    while (uglyFound < index) {
        ++number;
        
        if (IsUgly(number)) {
            ++uglyFound;
        }
    }
    
    return number;
}

// 求三个数字中最小的数字
int UglyNumber::Min(int number1, int number2, int number3) {
    int min = (number1 < number2) ? number1: number2;
    min = (min < number3) ? min: number3;
    
    return min;
}

// 准备一个 index 长度的数组，按从小到大顺序记录每个丑数，直到 index。
int UglyNumber::GetUglyNumber_Solution2(int index) {
    if (index <= 0) {
        return 0;
    }
    
    // 准备一个 index 长度的数组记录丑数
    int *pUglyNumbers = new int[index];
    // 第一个丑数从 1 开始
    pUglyNumbers[0] = 1;
    // 记录当前是第几个丑数
    int nextUglyIndex = 1;
    
    // 三个指针分别记录当前大于已有丑数乘以 2 3 5 后的最小值
    int* pMultiply2 = pUglyNumbers;
    int* pMultiply3 = pUglyNumbers;
    int* pMultiply5 = pUglyNumbers;
    
    // 循环直到第 index 个丑数
    while (nextUglyIndex < index) {
        // 当前包含 2 3 5 因子的最小丑数
        int min = Min(*pMultiply2 * 2, *pMultiply3 * 3, *pMultiply5 * 5);
        pUglyNumbers[nextUglyIndex] = min;
        
        // 2 => [2, 4, 8, 16, 32, 64, ...]
        // 3 => [3, 6, 9, 12, 15, 18, ...]
        // 5 => [5, 10, 15, 20, 25, 30, ...]
        // 如上，每次从以上三个组找大于当前丑数的最小丑数
        
        // [1, 2, 3, 4, 5, 6, 8, 9, 10, 12, 15, 16, 18, 20, ...]
        // 更新 3 个指针
        while (*pMultiply2 * 2 <= pUglyNumbers[nextUglyIndex]) {
            ++pMultiply2;
        }
        while (*pMultiply3 * 3 <= pUglyNumbers[nextUglyIndex]) {
            ++pMultiply3;
        }
        while (*pMultiply5 * 5 <= pUglyNumbers[nextUglyIndex]) {
            ++pMultiply5;
        }
        
        // 自增 1，直到 index
        ++nextUglyIndex;
    }
    
    // 要找到丑数
    int ugly = pUglyNumbers[nextUglyIndex - 1];
    
    // 释放内存
    delete [] pUglyNumbers;
    
    return ugly;
}

// 测试代码
void UglyNumber::Test(int index, int expected) {
    if(GetUglyNumber_Solution1(index) == expected)
        printf("solution1 passed\n");
    else
        printf("solution1 failed\n");

    if(GetUglyNumber_Solution2(index) == expected)
        printf("solution2 passed\n");
    else
        printf("solution2 failed\n");
}

void UglyNumber::Test() {
    Test(1, 1);

    Test(2, 2);
    Test(3, 3);
    Test(4, 4);
    Test(5, 5);
    Test(6, 6);
    Test(7, 8);
    Test(8, 9);
    Test(9, 10);
    Test(10, 12);
    Test(11, 15);

    Test(1500, 859963392);

    Test(0, 0);
}
