//
//  ReorderArray_1.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/4.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "ReorderArray_1.hpp"

void ReorderArray_1::reorder(int* pData, unsigned int length, bool(*func)(int)) {
    if (pData == nullptr || length <= 0 || func == nullptr) {
        return;
    }
    
    int* pBegin = pData;
    int* pEnd = pData + length - 1;
    
    while (pBegin < pEnd) {
        while (pBegin < pEnd && (*func)(*pBegin)) {
            ++pBegin;
        }
        
        while (pBegin < pEnd && !(*func)(*pEnd)) {
            --pEnd;
        }
        
        if (pBegin < pEnd) {
            int temp = *pBegin;
            *pBegin = *pEnd;
            *pEnd = temp;
        }
    }
}

bool ReorderArray_1::isEven(int n) {
    return n & 0x1; // 返回 true 表示 n 是奇数，返回 false 表示 n 是偶数
}

void ReorderArray_1::reorderOddEven_2(int* pData, unsigned int length) {
    reorder(pData, length, isEven);
}

// 测试代码
void ReorderArray_1::PrintArray(int numbers[], int length) {
    if(length < 0)
        return;
    
    for(int i = 0; i < length; ++i)
        printf("%d\t", numbers[i]);
    
    printf("\n");
}

void ReorderArray_1::Test(char* testName, int numbers[], int length) {
    if(testName != nullptr)
        printf("%s begins:\n", testName);

    int* copy = new int[length];
    for(int i = 0; i < length; ++i) {
        copy[i] = numbers[i];
    }

//    printf("Test for solution 1:\n");
//    PrintArray(numbers, length);
//    RreorderOddEven_1(numbers, length);
//    PrintArray(numbers, length);

    printf("Test for solution 2:\n");
    PrintArray(copy, length);
    reorderOddEven_2(copy, length);
    PrintArray(copy, length);

    delete[] copy;
}

void ReorderArray_1::Test1() {
    int numbers[] = {1, 2, 3, 4, 5, 6, 7};
    Test("Test1", numbers, sizeof(numbers)/sizeof(int));
}

void ReorderArray_1::Test2() {
    int numbers[] = {2, 4, 6, 1, 3, 5, 7};
    Test("Test2", numbers, sizeof(numbers)/sizeof(int));
}

void ReorderArray_1::Test3() {
    int numbers[] = {1, 3, 5, 7, 2, 4, 6};
    Test("Test3", numbers, sizeof(numbers)/sizeof(int));
}

void ReorderArray_1::Test4() {
    int numbers[] = {1};
    Test("Test4", numbers, sizeof(numbers)/sizeof(int));
}

void ReorderArray_1::Test5() {
    int numbers[] = {2};
    Test("Test5", numbers, sizeof(numbers)/sizeof(int));
}

void ReorderArray_1::Test6() {
    Test("Test6", nullptr, 0);
}

void ReorderArray_1::Test() {
    Test1();
    Test2();
    Test3();
    Test4();
    Test5();
    Test6();
}
