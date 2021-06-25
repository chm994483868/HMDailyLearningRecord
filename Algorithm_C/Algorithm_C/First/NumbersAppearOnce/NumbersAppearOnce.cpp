//
//  NumbersAppearOnce.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/24.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "NumbersAppearOnce.hpp"

unsigned int findFirstBitIs1(int num);
bool isBit1(int num, unsigned int indexBit);

void findNumsAppearOnce(int data[], int length, int* num1, int* num2) {
    if (data == nullptr || length < 2) {
        return;
    }
    
    int resultExclusiveOR = 0;
    // 1. 遍历数组，所有元素进行异或，从 0 开始，0 和 0 异或是 0，1 和 0 异或还是 1
    int i = 0;
    for (; i < length; ++i) {
        resultExclusiveOR ^= data[i];
    }
    
    // 2. 找到所有数字异或结果中右起第一位是 1 的位
    unsigned int indexOf1 = findFirstBitIs1(resultExclusiveOR);
    
    // 3. 然后分别把 indexOf1 位置是 0 或 1 的元素分组进行异或
    // 两个相同的元素该位置必定都是 0 或者 1，异或时两个数直接抵消为 0
    // 且两个不同的只出现一次的数字，该位置必定不同
    *num1 = *num2 = 0;
    for (i = 0; i < length; ++i) {
        if (isBit1(data[i], indexOf1)) {
            *num1 ^= data[i];
        } else {
            *num2 ^= data[i];
        }
    }
}

// 找到 num 从右边数起第一个是 1 的位
// 如果右边第一位是 1 的话，1 & 1 == 1
// 返回 indexBit 是 0
// 所以从右边数起都是从 0 开始的
unsigned int findFirstBitIs1(int num) {
    int indexBit = 0;
    // 这里的 indexBit < 8 * sizeof(int) 条件用的是小于号，不是小于等于号，
    // 是为了忽略符号位
    while (((num & 1) == 0) && (indexBit < 8 * sizeof(int))) {
        num = num >> 1;
        ++indexBit;
    }
    
    return indexBit;
}

// 判断数字 num 的第 indexBit 位是不是 1
// 这个判断是 右起，且是从 0 开始的
// indexBit 输入 0，下面即返回右起第 0 位是不是 1，返回 true 表示是 1， 否则是 0
bool isBit1(int num, unsigned int indexBit) {
    num = num >> indexBit;
    return (num & 1);
}
