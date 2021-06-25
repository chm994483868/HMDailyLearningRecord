//
//  MoreThanHalfNumber_1.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/6.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "MoreThanHalfNumber_1.hpp"

// 检测入参是否正确，正确与否记录在全局变量 g_bInputInvalid 中
bool MoreThanHalfNumber_1::checkInvalidArray(int* numbers, int length) {
    g_bInputInvalid = false;
    if (numbers == nullptr || length <= 0) {
        g_bInputInvalid = true;
    }
    
    return g_bInputInvalid;
}

// 检测 number 在 numbers 中出现次数是否超过了一半
bool MoreThanHalfNumber_1::checkMoreThanHalf(int* numbers, int length, int number) {
    int count = 0;
    for (unsigned int i = 0; i < length; ++i) {
        if (numbers[i] == number) {
            ++count;
        }
    }
    
    bool isMoreThanHalf = true;
    if (count * 2 <= length) {
        // 同时标记 g_bInputInvalid 入参无效
        g_bInputInvalid = true;
        isMoreThanHalf = false;
    }
    
    return isMoreThanHalf;
}

int MoreThanHalfNumber_1::moreThanHalfNum_Solution1(int* numbers, int length) {
    if (checkInvalidArray(numbers, length)) {
        return 0;
    }
    
    int middle = length >> 1;
    int start = 0;
    int end = length - 1;
    
    int index = partition(numbers, length, start, end);
    while (index != middle) {
        if (index > middle) {
            end = index - 1;
            index = partition(numbers, length, start, end);
        } else {
            start = index + 1;
            index = partition(numbers, length, start, end);
        }
    }
    
    int result = numbers[middle];
    if (!checkMoreThanHalf(numbers, length, result)) {
        result = 0;
    }
    
    return result;
}

int MoreThanHalfNumber_1::moreThanHalfNum_Solution2(int* numbers, int length) {
    if (checkInvalidArray(numbers, length)) {
        return 0;
    }
    
    int result = numbers[0];
    int times = 1;
    
    for (unsigned int i = 1; i < length; ++i) {
        if (times == 0) {
            result = numbers[i];
            times = 1;
        } else if (numbers[i] == result) {
            ++times;
        } else {
            --times;
        }
    }
    
    if (!checkMoreThanHalf(numbers, length, result)) {
        result = 0;
    }
    
    return result;
}

// 测试代码
void MoreThanHalfNumber_1::Test(char* testName, int* numbers, int length, int expectedValue, bool expectedFlag) {
    if(testName != nullptr)
        printf("%s begins: \n", testName);

    int* copy = new int[length];
    for(int i = 0; i < length; ++i)
        copy[i] = numbers[i];

    printf("Test for solution1: ");
    int result = moreThanHalfNum_Solution1(numbers, length);
    if(result == expectedValue && g_bInputInvalid == expectedFlag)
        printf("Passed.\n");
    else
        printf("Failed.\n");

    printf("Test for solution2: ");
    result = moreThanHalfNum_Solution2(copy, length);
    if(result == expectedValue && g_bInputInvalid == expectedFlag)
        printf("Passed.\n");
    else
        printf("Failed.\n");

    delete[] copy;
}

// 存在出现次数超过数组长度一半的数字
void MoreThanHalfNumber_1::Test1() {
    int numbers[] = {1, 2, 3, 2, 2, 2, 5, 4, 2};
    Test("Test1", numbers, sizeof(numbers) / sizeof(int), 2, false);
}

// 不存在出现次数超过数组长度一半的数字
void MoreThanHalfNumber_1::Test2() {
    int numbers[] = {1, 2, 3, 2, 4, 2, 5, 2, 3};
    Test("Test2", numbers, sizeof(numbers) / sizeof(int), 0, true);
}

// 出现次数超过数组长度一半的数字都出现在数组的前半部分
void MoreThanHalfNumber_1::Test3() {
    int numbers[] = {2, 2, 2, 2, 2, 1, 3, 4, 5};
    Test("Test3", numbers, sizeof(numbers) / sizeof(int), 2, false);
}

// 出现次数超过数组长度一半的数字都出现在数组的后半部分
void MoreThanHalfNumber_1::Test4() {
    int numbers[] = {1, 3, 4, 5, 2, 2, 2, 2, 2};
    Test("Test4", numbers, sizeof(numbers) / sizeof(int), 2, false);
}

// 输入空指针
void MoreThanHalfNumber_1::Test5() {
    int numbers[] = {1};
    Test("Test5", numbers, 1, 1, false);
}

// 输入空指针
void MoreThanHalfNumber_1::Test6() {
    Test("Test6", nullptr, 0, 0, true);
}

void MoreThanHalfNumber_1::Test() {
    Test1();
    Test2();
    Test3();
    Test4();
    Test5();
    Test6();
}
