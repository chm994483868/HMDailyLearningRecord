//
//  SortArrayForMinNumber.cpp
//  OfferReview
//
//  Created by CHM on 2020/7/31.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "SortArrayForMinNumber.hpp"

void SortArrayForMinNumber::PrintMinNumber(const int* numbers, int length) {
    if (numbers == nullptr || length <= 0) {
        return;
    }
    
    // 元素是 char** 的数组
    char** strNumbers = (char**)(new int[length]);
    // 把 numbers 中的内容放入 strNumbers 中
    for (int i = 0; i < length; ++i) {
        strNumbers[i] = new char[g_MaxNumberLength + 1];
        sprintf(strNumbers[i], "%d", numbers[i]);
    }
    
    // 根据 compare 函数排序，如 mn < nm，则 m 就排在 n 前面，
    qsort(strNumbers, length, sizeof(char*), compare);
    
    // 打印 strNumbers，即排好序的数字
    for (int i = 0; i < length; ++i) {
        printf("%s", strNumbers[i]);
    }
    printf("\n");
    
    // 释放内存
    for (int i = 0; i < length; ++i) {
        delete [] strNumbers[i];
    }
    
    delete [] strNumbers;
}

// 如果[strNumber1][strNumber2] > [strNumber2][strNumber1], 返回值大于0
// 如果[strNumber1][strNumber2] = [strNumber2][strNumber1], 返回值等于0
// 如果[strNumber1][strNumber2] < [strNumber2][strNumber1], 返回值小于0
int SortArrayForMinNumber::compare(const void* strNumber1, const void* strNumber2) {
    // [strNumber1][strNumber2]
    strcpy(g_StrCombine1, *(const char**)g_StrCombine1);
    strcat(g_StrCombine1, *(const char**)g_StrCombine2);
    
    // [strNumber2][strNumber1]
    strcpy(g_StrCombine2, *(const char**)g_StrCombine2);
    strcat(g_StrCombine2, *(const char**)g_StrCombine1);
    
    return strcmp(g_StrCombine1, g_StrCombine2);
}

// 测试代码
void SortArrayForMinNumber::Test(const char* testName, int* numbers, int length, const char* expectedResult) {
    if(testName != nullptr)
        printf("%s begins:\n", testName);

    if(expectedResult != nullptr)
        printf("Expected result is: \t%s\n", expectedResult);

    printf("Actual result is: \t");
    PrintMinNumber(numbers, length);

    printf("\n");
}

void SortArrayForMinNumber::Test1() {
    int numbers[] = {3, 5, 1, 4, 2};
    Test("Test1", numbers, sizeof(numbers)/sizeof(int), "12345");
}

void SortArrayForMinNumber::Test2() {
    int numbers[] = {3, 32, 321};
    Test("Test2", numbers, sizeof(numbers)/sizeof(int), "321323");
}

void SortArrayForMinNumber::Test3() {
    int numbers[] = {3, 323, 32123};
    Test("Test3", numbers, sizeof(numbers)/sizeof(int), "321233233");
}

void SortArrayForMinNumber::Test4() {
    int numbers[] = {1, 11, 111};
    Test("Test4", numbers, sizeof(numbers)/sizeof(int), "111111");
}

// 数组中只有一个数字
void SortArrayForMinNumber::Test5() {
    int numbers[] = {321};
    Test("Test5", numbers, sizeof(numbers)/sizeof(int), "321");
}

void SortArrayForMinNumber::Test6() {
    Test("Test6", nullptr, 0, "Don't print anything.");
}


void SortArrayForMinNumber::Test() {
    Test1();
    Test2();
    Test3();
    Test4();
    Test5();
    Test6();
}

