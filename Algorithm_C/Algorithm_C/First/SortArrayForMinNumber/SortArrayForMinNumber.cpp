//
//  SortArrayForMinNumber.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/23.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "SortArrayForMinNumber.hpp"

int compare(const void* strNumber1, const void* strNumber2);

// int 型整数用十进制表示最多只有 10 位
const int g_MaxNumberLength = 10;

char* g_StrCombine1 = new char[g_MaxNumberLength * 2 + 1];
char* g_StrCombine2 = new char[g_MaxNumberLength * 2 + 1];

void printMinNumber(const int* numbers, int length) {
    if (numbers == nullptr || length <= 0) {
        return;
    }
    
    char** strNumbers = (char**)(new int[length]);
    for (int i = 0; i < length; ++i) {
        strNumbers[i] = new char[g_MaxNumberLength + 1];
        sprintf(strNumbers[i], "%d", numbers[i]);
    }
    
    qsort(strNumbers, length, sizeof(char*), compare);
    
    for (int i = 0; i < length; ++i) {
        printf("%s", strNumbers[i]);
    }
    
    printf("\n");
    
    for (int i = 0; i < length; ++i) {
        delete [] strNumbers[i];
    }
    
    delete [] strNumbers;
}

// 如果 [strNumber1][strNumber2] > [strNumber2][strNumber1]，返回值大于 0
// 如果 [strNumber1][strNumber2] = [strNumber2][strNumber1]，返回值等于 0
// 如果 [strNumber1][strNumber2] < [strNumber2][strNumber1]，返回值小于 0
int compare(const void* strNumber1, const void* strNumber2) {
    // [strNumber1][strNumber2] 把字符串 2 拼在字符串 1 后面
    strcpy(g_StrCombine1, *(const char**)strNumber1);
    strcat(g_StrCombine1, *(const char**)strNumber2);
    
    // [strNumber2][strNumber1] 把字符串 1 拼在字符串 2 后面
    strcpy(g_StrCombine2, *(const char**)strNumber2);
    strcat(g_StrCombine2, *(const char**)strNumber1);
    
    return strcmp(g_StrCombine1, g_StrCombine2);
}

namespace SortArrayForMinNumber_Review {
int compare(const void* strNumber1, const void* strNumber2);

const int g_MaxNumberLength = 10;

char* g_StrCombine1 = new char[g_MaxNumberLength * 2 + 1];
char* g_StrCombine2 = new char[g_MaxNumberLength * 2 + 1];

void printMinNumber(const int* numbers, int length) {
    if (numbers == nullptr || length <= 0) {
        return;
    }
    
    char** strNumbers = (char**)(new int[length]);
    int i = 0;
    for (; i < length; ++i) {
        strNumbers[i] = new char[g_MaxNumberLength + 1];
        sprintf(strNumbers[i], "%d", numbers[i]);
    }
    
    qsort(strNumbers, length, sizeof(char*), compare);
    
    for (i = 0; i < length; ++i) {
        printf("%s", strNumbers[i]);
    }
    
    printf("\n");
    
    for (i = 0; i < length; ++i) {
        delete [] strNumbers[i];
    }
    delete [] strNumbers;
}

int compare(const void* strNumber1, const void* strNumber2) {
    strcpy(g_StrCombine1, *(const char**)strNumber1);
    strcat(g_StrCombine1, *(const char**)strNumber2);
    
    strcpy(g_StrCombine2, *(const char**)strNumber2);
    strcat(g_StrCombine2, *(const char**)strNumber1);
    
    return strcmp(g_StrCombine1, g_StrCombine2);
}

}
