//
//  DigitsInSequence.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/23.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "DigitsInSequence.hpp"

int countOfIntegers(int digits);
int digitAtIndex(int index, int digits);
int beginNumber(int digits);

int digitAtIndex(int index) {
    if (index < 0) {
        return -1;
    }
    
    // 1. 从 1 位数字开始
    int digits = 1;
    while (true) {
        // 2. 取得 digits 位数字的总个数
        int numbers = countOfIntegers(digits);
        if (index < numbers * digits) {
            // 3. 取得在 当前位数的数字中的 index
            // 例如：示例中最后取得在三位数中，从 100 开始，下标为 881 的数字
            return digitAtIndex(index, digits);
        }
        
        index -= digits * numbers;
        ++digits;
    }
    
    return -1;
}

// 返回 digits 位的数字总共有多少个。
// 例如：输入 2 返回 （10～99）90 个
// 输入 3 返回（100～999）900 个
int countOfIntegers(int digits) {
    if (digits == 1) {
        return 10;
    }
    
    int count = (int)std::pow(10, digits - 1);
    return 9 * count;
}

// 当我们知道要找的那一位数字位于某个 m 位数之中后
int digitAtIndex(int index, int digits) {
    // 1. 如示例中 811 = 270 * 3 + 1， 从 100 开始的第 270 个数字即 370
    int number = beginNumber(digits) + index / digits;
    // 2. 因为 余数是 1，即求 370 中中间的一位
    // 3. 从 右 起 第 几 位
    int indexFromRight = digits - index % digits;
    // 4. 缩小取商
    for (int i = 1; i < indexFromRight; ++i) {
        number /= 10;
    }
    // 5. 取余
    return number % 10;
}

// 返回 digits 位数字的第一个数字。
// 例如：digits = 2, 返回第一个两位数 10
// digits = 3, 返回第一个三位数 100
int beginNumber(int digits) {
    if (digits == 1) {
        return 0;
    }
    
    return (int) std::pow(10, digits - 1);
}
