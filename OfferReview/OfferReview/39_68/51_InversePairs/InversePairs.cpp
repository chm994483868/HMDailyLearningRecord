//
//  InversePairs.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "InversePairs.hpp"

int InversePairs::InversePairs(int* data, int length) {
    // 入参判断
    if (data == nullptr || length <= 0) {
        return 0;
    }
    
    // 把 data 数组中的数据复制到 copy 数组中
    // copy 作为归并排序的辅助数组
    int* copy = new int[length];
    for (int i = 0; i < length; ++i) {
        copy[i] = data[i];
    }
    
    // 核心函数
    int count = InversePairsCore(data, copy, 0, length - 1);
    
    // 释放 copy 的内存
    delete [] copy;
    
    return count;
}

int InversePairs::InversePairsCore(int* data, int* copy, int start, int end) {
    if (start == end) {
        copy[start] = data[start];
        return 0;
    }
    
    // start 与中间值的距离
    int length = (end - start) / 2;
    // 递归拆分数据
    int left = InversePairsCore(copy, data, start, start + length);
    int right = InversePairsCore(copy, data, start + length + 1, end);
    
    // i 初始化为前半段最后一个数字的下标
    int i = start + length;
    // j 初始化为后半段最后一个数字的下标
    int j = end;
    
    int indexCopy = end;
    int count = 0;
    
    // 合并数组
    while (i >= start && j >= start + length + 1) {
        if (data[i] > data[j]) {
            copy[indexCopy--] = data[i--];
            
            // 在这里统计逆序对数，如果 data[i] > data[j]，
            // 则 data[i] 能和 data[j] 后的所有数据组成逆序对，
            // j 后面有 j - (start + length) 个数字
            count += j - start - length;
        } else {
            copy[indexCopy--] = data[j--];
        }
    }
    
    for (; i >= start; --i) {
        copy[indexCopy--] = data[i];
    }
    
    for (; j >= start + length + 1; --j) {
        copy[indexCopy--] = data[j];
    }
    
    return left + right + count;
}
