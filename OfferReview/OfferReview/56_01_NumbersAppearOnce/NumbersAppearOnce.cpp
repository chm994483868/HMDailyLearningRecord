//
//  NumbersAppearOnce.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "NumbersAppearOnce.hpp"

// 判断数字 num 的第 indexBit 位是不是 1
bool NumbersAppearOnce::isBit1(int num, unsigned int indexBit) {
    num = num >> indexBit;
    return (num & 1);
}

// 找到 num 从右边数起第一个是 1 的位
unsigned int NumbersAppearOnce::findFirstBitIs1(int num) {
    // 从 0 开始，记录 num 右移的次数
    int indexBit = 0;
    
    // num 每次右移 1 位，直到第一个 1 被移到二进制表示的最右边
    while (((num & 1) == 0) && (indexBit < 8 * sizeof(int))) {
        num = num >> 1;
        ++indexBit;
    }
    
    return indexBit;
}

void NumbersAppearOnce::findNumsAppearOnce(int data[], int length, int* num1, int* num2) {
    // 入参判断，数组不为 nullptr，元素个数至少 2 个
    if (data == nullptr || length < 2) {
        return;
    }
    
    // 全数组的元素做异或操作
    int resultExclusiveOR = 0;
    for (int i = 0; i < length; ++i) {
        resultExclusiveOR ^= data[i];
    }
    
    // 找到异或结果中从右边起第一个是 1 的位
    unsigned int indexOf1 = findFirstBitIs1(resultExclusiveOR);
    
    *num1 = *num2 = 0;
    
    // 数组分为两个组，把 indexOf1 位是 1 的一组，不是 1 的一组，
    // 然后两组数再做异或，最后 *num1 和 *num2 就是要找的两个只出现一次的数字
    for (int j = 0; j < length; ++j) {
        if (isBit1(data[j], indexOf1)) {
            *num1 ^= data[j];
        } else {
            *num2 ^= data[j];
        }
    }
}



