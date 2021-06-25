//
//  DicesProbability.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/26.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "DicesProbability.hpp"

int g_maxValue = 6;

void probability(int number, int* pProbabilities);
void probability(int original, int current, int sum, int* pProbabilities);

void printProbability_Solution1(int number) {
    // 1. 如果🎲个数小于 1，可以直接返回了
    if (number < 1) {
        return;
    }
    
    // 2. number 个🎲，一次出现的最大和是🎲每个面都是 6，即 number * 6
    int maxSum = number * g_maxValue;
    // 3. 准备一个所有和的可能出现的次数多数组
    // 数组长度是所有和可能出现的情况，最小时是🎲每个面都是 1，即 number * 1
    // 所以最小的和和最大的和之间差距，maxSum - number, 即所有可能出现的结果即: maxSum - number + 1
    // 即下面数组里每个元素对应一个点数之和出现的次数
    int* pProbabilities = new int[maxSum - number + 1];
    // 4. 把所有元素置为 0
    for (int i = number; i <= maxSum; ++i) {
        pProbabilities[i - number] = 0;
    }
    
    // 8. 这个函数到底是什么意思
    probability(number, pProbabilities);
    
    // 5. number 个🎲撒下去，所有可能出现的情况即 6 的 number 次方个
    int total = pow((double)g_maxValue, number);
    // 6. 打印所有和的值出现的次数与总情况的比率
    for (int i = number; i <= maxSum; ++i) {
        double ratio = (double)pProbabilities[i - number] / total;
        printf("%d: %e\n", i, ratio);
    }
    // 7. 释放内存
    delete [] pProbabilities;
}

// 假设 number = 5 maxSum = 30 minSum = 6 30 - 5 + 1 = 26
void probability(int number, int* pProbabilities) {
    // i 从 1 到 6 的一个循环
    for (int i = 1; i <= g_maxValue; ++i) {
        // i = 1 时: 5 5 1 int[26]
        // i = 2 时: 5 5 2 int[26]
        // ...
        // i = 6 时: 5 5 6 int[26]
        probability(number, number, i, pProbabilities);
    }
}

// 这个递归函数又是干啥的
// 假设第一轮进来:
// original = 5 current = 5 sum = 1 int[26]
// current != 1 进入 else 里面的 6 次循环

// i = 1 时: original = 5 current = 4 sum = 2 int[26] 进入一个递归:
// original = 5 current = 4 sum = 2 int[26]
//
// original = 5 current = 5 sum = 1 int[26] 栈底
void probability(int original, int current, int sum, int* pProbabilities) {
    if (current == 1) {
        ++pProbabilities[sum - original];
    } else {
        for (int i = 1; i <= g_maxValue; ++i) {
            probability(original, current - 1, i + sum, pProbabilities);
        }
    }
}

// 5 5 1
// 5 4 2
// 5 3 3
// 5 2 4
// 5 1 5

// 1 2 3 4 5 6
// 1 2 3 4 5 6

// n = 2 的情况 f(k) = f(k-1) + f(k-2) + f(k-3) + f(k-4) + f(k-5) + f(k-6)
// k = 7 时: f(7) = f(6) + f(5) + f(4) + f(3) + f(2) + f(1)
//
// 2 3 4 5 6 7
// 3 4 5 6 7 8
// 4 5 6 7 8 9
// 5 6 7 8 9 10
// 6 7 8 9 10 11
// 7 8 9 10 11 12

// 2 3 3
