//
//  DigitsInSequence.cpp
//  OfferReview
//
//  Created by CHM on 2020/7/31.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "DigitsInSequence.hpp"

// 取得 digits 位的数字总共有多少个
// 例如：输入 2 返回两位数（10～99）的个数是 90
// 输入 3 返回三位数（100～999）的个数是 900
int DigitsInSequence::countOfIntegers(int digits) {
    if (digits == 1) {
        return 10;
    }
    
    int count = (int)pow(10, digits - 1);
    return 9 * count;
}

// 当我们知道要找的那一位数字位于某 m 位数之中后，可以用如下函数找出那一位数字
// 如第 881 是某个三位数中的一位，由于 881 = 270 * 3 + 1，
// 意味着第 881 位是从 100 开始的第 270 个数字即 370 的中间一位，也就是 7
int DigitsInSequence::digitAtIndex(int index, int digits) {
    // 对应上面的情况：
    // beginNumber(digits) 是 100
    // index / digits 是 270
    // index % digits 是 1
    // digits - index % digits 表示要找的这个数字在 digits 位数中从右边是下标第几位，
    // 如 370 中 7 从右边开始算是下标为 1 的数字
    int number = beginNumber(digits) + index / digits;
    int indexFromRight = digits - index % digits;
    for (int i = 1; i < indexFromRight; ++i) {
        // 依次缩小，撇掉低位
        number /= 10;
    }
    
    // 取模求末位的数字
    return number % 10;
}

// 取得 digits 位的数的第一个数字，例如，第一个两位数是 10，第一个三位数是 100
int DigitsInSequence::beginNumber(int digits) {
    // 第一个一位数是 0，其它的情况下都是 10 的 digits - 1 次方，
    // 两位 10^1
    // 三位 10^2
    // 四位 10^3
    if (digits == 1) {
        return 0;
    }
    
    return (int)pow(10, digits - 1);
}

int DigitsInSequence::digitAtIndex(int index) {
    // 如果下标小于 0 则返回 -1
    if (index < 0) {
        return -1;
    }
    
    int digits = 1;
    while (true) {
        // 统计不同位数数字的总数，从 1 位数开始，
        // 如 1 位数的个数是 10，2 位数的个数是 90，3 位数的个数 900 总共可包含 900 + 90 + 10
        int number = countOfIntegers(digits);
        
        // 这里是确定 index 具体是位于哪个位数的数字字中，例如 1001 是三位数中的一个数字
        if (index < number * digits) {
            // 找到具体的那一位是数字是啥
            return digitAtIndex(index, digits);
        }
        
        // 从 1 位数的的个数开始减去前面的数字
        index -= number * digits;
        
        // digits 表示数字位数，从 1 开始一直累加
        ++digits;
    }
    
    // 未找到则返回 -1
    return -1;
}

// 测试代码
void DigitsInSequence::test(const char* testName, int inputIndex, int expectedOutput) {
    if(digitAtIndex(inputIndex) == expectedOutput)
        cout << testName << " passed." << endl;
    else
        cout << testName << " FAILED." << endl;
}

void DigitsInSequence::Test() {
    test("Test1", 0, 0);
    test("Test2", 1, 1);
    test("Test3", 9, 9);
    test("Test4", 10, 1);
    test("Test5", 189, 9);  // 数字99的最后一位，9
    test("Test6", 190, 1);  // 数字100的第一位，1
    test("Test7", 1000, 3); // 数字370的第一位，3
    test("Test8", 1001, 7); // 数字370的第二位，7
    test("Test9", 1002, 0); // 数字370的第三位，0
}
