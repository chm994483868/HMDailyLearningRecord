//
//  ReorderArray.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/16.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "ReorderArray.hpp"

void reorder(int* pData, unsigned int length, bool(*func)(int));
bool isEven(int n);

void reorderOddEven_1(int* pData, unsigned int length) {
    if (pData == nullptr || length <= 0)
        return;
    
    int* pBegin = pData;
    int* pEnd = pData + length - 1;
    
    while (pBegin < pEnd) {
        // 向后移动 pBegin，直到它指向偶数
        while (pBegin < pEnd && (*pBegin & 0x1) != 0)
            ++pBegin;
        
        // 向前移动 pEnd，直到它指向奇数
        while (pBegin < pEnd && (*pEnd & 0x1) == 0)
            --pEnd;
        
        if (pBegin < pEnd) {
            int temp = *pBegin;
            *pBegin = *pEnd;
            *pEnd = temp;
        }
    }
}

void reorderOddEven_2(int* pData, unsigned int length) {
    reorder(pData, length, isEven);
}

void reorder(int* pData, unsigned int length, bool(*func)(int)) {
    if (pData == nullptr || length <= 0)
        return;
    
    int* pBegin = pData;
    int* pEnd = pData + length - 1;
    
    while (pBegin < pEnd) {
        while (pBegin < pEnd && !func(*pBegin))
            ++pBegin;
        
        while (pBegin < pEnd && func(*pEnd))
            --pEnd;
        
        if (pBegin < pEnd) {
            int temp = *pBegin;
            *pBegin = *pEnd;
            *pEnd = temp;
        }
    }
}

bool isEven(int n) {
    // 奇数返回 false
    // 偶数返回 true
    return (n & 0x1) == 0;
}

void PrintArray(int numbers[], int length)
{
    if(length < 0)
        return;

    for(int i = 0; i < length; ++i)
        printf("%d\t", numbers[i]);

    printf("\n");
}
