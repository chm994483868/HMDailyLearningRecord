//
//  MoreThanHalfNumber.cpp
//  OfferReview
//
//  Created by HM C on 2020/7/30.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "MoreThanHalfNumber.hpp"

// 检测入参是否正确，正确与否记录在全局变量 g_bInputInvalid 中
bool MoreThanHalfNumber::checkInvalidArray(int* numbers, int length) {
    g_bInputInvalid = false;
    if (numbers == nullptr || length <= 0) {
        g_bInputInvalid = true;
    }
    
    return g_bInputInvalid;
}

// 检测 number 在 numbers 中出现次数是否超过了一半
bool MoreThanHalfNumber::checkMoreThanHalf(int* numbers, int length, int number) {
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

// 题目给的条件是有一个数字出现的次数超过了数组长度的一半，
// 那么对数组排序的话，在数组中间下标的元素一定就是这个数字，但是排序的话时间复杂度至少是 O(nlogn)。

// 这里使用前面快速排序中用到的 partition 函数，
// 它每次在数组中随机选中一个数字，把数组在逻辑上分成两组，左边都是小于该数字的元素，
// 右边都是大于该数字的元素，然后判断这个随机值的下标与数组中间下标，当随机值下标大于数组
// 中间下标时，缩小右边边界，当随机值小于中间下标时增大左边边界，类似二分查找。

// 解法 1 会修改原始数组。
int MoreThanHalfNumber::moreThanHalfNum_Solution1(int* numbers, int length) {
    if (checkInvalidArray(numbers, length)) {
        return 0;
    }
    
    // 数组中中间下标
    int middle = length >> 1;
    
    // 左边边界
    int start = 0;
    // 右边边界
    int end = length - 1;
    
    // 随机值下标
    int index = partition(numbers, length, start, end);
    
    // 一直进行随机，直到 index == middle
    while (index != middle) {
        // 如果 index 出现在 middle 右边，则缩小右边边界
        if (index > middle) {
            end = index - 1;
            index = partition(numbers, length, start, end);
        } else {
            // 如果 index 出现在 middle 左边，则增大左边边界
            start = index + 1;
            index = partition(numbers, length, start, end);
        }
    }
    
    // 取出中间位置数值
    int result = numbers[middle];
    
    // 检测 result 是否在数组中出现次数超过数组长度的一半
    if (!checkMoreThanHalf(numbers, length, result)) {
        result = 0;
    }
    
    return result;
}

// 解法 2 不修改原始数组。
// 再看一遍题目条件，数字出现的次数超过了数组长度的一半，那么从前到后遍历数组，统计出现连续相同数字的个数。
int MoreThanHalfNumber::moreThanHalfNum_Solution2(int* numbers, int length) {
    if (checkInvalidArray(numbers, length)) {
        return 0;
    }
    
    // 从第 0 个数字开始
    int result = numbers[0];
    
    // 第 0 个数字出现的次数是 1
    int times = 1;
    
    // 从第 1 个数字开始，遇到相同的数字 times 就自增，
    // 当遇到不同数字 times 就自减，当 times 自减到 0，
    // 然后遇到下一个新数字时，result 记录该数字，times 也置为 1，
    // 由于我们要找的数字出现的次数大于数组长度的一半，所以当数组遍历完毕，
    // times 必不为 0，此时的 result 就是我们要找到数字。
    for (unsigned int i = 1; i < length; ++i) {
        if (times == 0) {
            // 遇到新数字
            result = numbers[i];
            times = 1;
        } else if (numbers[i] == result) {
            // 遇到相同的数字 times 自增
            ++times;
        } else {
            // 遇到不同数字 times 自减
            --times;
        }
    }
    
    // 检测 result 是否合规
    if (!checkMoreThanHalf(numbers, length, result)) {
        result = 0;
    }
    
    return result;
}

// 测试代码
void MoreThanHalfNumber::Test(char* testName, int* numbers, int length, int expectedValue, bool expectedFlag) {
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
void MoreThanHalfNumber::Test1() {
    int numbers[] = {1, 2, 3, 2, 2, 2, 5, 4, 2};
    Test("Test1", numbers, sizeof(numbers) / sizeof(int), 2, false);
}

// 不存在出现次数超过数组长度一半的数字
void MoreThanHalfNumber::Test2() {
    int numbers[] = {1, 2, 3, 2, 4, 2, 5, 2, 3};
    Test("Test2", numbers, sizeof(numbers) / sizeof(int), 0, true);
}

// 出现次数超过数组长度一半的数字都出现在数组的前半部分
void MoreThanHalfNumber::Test3() {
    int numbers[] = {2, 2, 2, 2, 2, 1, 3, 4, 5};
    Test("Test3", numbers, sizeof(numbers) / sizeof(int), 2, false);
}

// 出现次数超过数组长度一半的数字都出现在数组的后半部分
void MoreThanHalfNumber::Test4() {
    int numbers[] = {1, 3, 4, 5, 2, 2, 2, 2, 2};
    Test("Test4", numbers, sizeof(numbers) / sizeof(int), 2, false);
}

// 输入空指针
void MoreThanHalfNumber::Test5() {
    int numbers[] = {1};
    Test("Test5", numbers, 1, 1, false);
}

// 输入空指针
void MoreThanHalfNumber::Test6() {
    Test("Test6", nullptr, 0, 0, true);
}

void MoreThanHalfNumber::Test() {
    Test1();
    Test2();
    Test3();
    Test4();
    Test5();
    Test6();
}
