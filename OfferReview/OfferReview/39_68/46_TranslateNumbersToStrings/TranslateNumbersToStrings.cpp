//
//  TranslateNumbersToStrings.cpp
//  OfferReview
//
//  Created by CHM on 2020/7/31.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "TranslateNumbersToStrings.hpp"

// f(i) = f(i+1) + g(i, i + 1) * f(i + 2) g(i, i + 1) 的值为 0 或者 为 1
// 这里如每个数字只能翻译为一个字符，例如 666 那就仅有一种翻译方式就是 fff，当前后两个数字能连在一起且值在 [0, 25] 范围内时，
// 就会出现不同的翻译方式。如 123 可以 1 2 3 abc 也可以 1 23 a x 也可以 12 3 l c。

int TranslateNumbersToStrings::getTranslationCount(const string& number) {
    // string 长度
    unsigned long length = number.length();
    
    // int 数组
    int* counts = new int[length];
    
    // count 用来统计总共有多少种不同的翻译方法
    int count = 0;
    for (unsigned long i = length - 1; i >= 0; --i) {
        count = 0;
        
        // 从倒数数字开始，先取前一个数字有多少种转换方式
        if (i < length - 1) {
            count = counts[i + 1];
        } else {
            count = 1;
        }
        
        // 主要进行判断相邻的两个数字是否能一起转换
        if (i < length - 1) {
            // number[i] 字符转数字
            int digit1 = number[i] - '0';
            // number[i + 1] 字符转数字
            int digit2 = number[i + 1] - '0';
            
            // 把两个数字拼在一起，
            // 如果在 [0, 25] 的范围内则两者可以合并转化为一个字符
            int converted = digit1 * 10 + digit2;
            
            // 如果在 [0, 25] 的范围内，则可以多一种转化方式
            if (converted >= 10 && converted <= 25) {
                if (i < length - 2) {
                    count += counts[i + 2];
                } else {
                    // 加 1
                    count += 1;
                }
            }
        }

        // 赋值，用数组 counts 记录数字能翻译的不同方式的数量
        counts[i] = count;
    }

    // 从 length - 1 开始到 0，counts[0] 中记录的是最大的不同的转换方式
    count = counts[0];
    
    // 释放内存
    delete [] counts;

    return count;
}

int TranslateNumbersToStrings::getTranslationCount(int number) {
    if (number < 0) {
        return 0;
    }
    
    // 数字转为字符串
    string numberString = to_string(number);
    
    return getTranslationCount(numberString);
}

// 测试代码
void TranslateNumbersToStrings::Test(const string& testName, int number, int expected) {
    if(getTranslationCount(number) == expected)
        cout << testName << " passed." << endl;
    else
        cout << testName << " FAILED." << endl;
}

void TranslateNumbersToStrings::Test1() {
    int number = 0;
    int expected = 1;
    Test("Test1", number, expected);
}

void TranslateNumbersToStrings::Test2() {
    int number = 10;
    int expected = 2;
    Test("Test2", number, expected);
}

void TranslateNumbersToStrings::Test3() {
    int number = 125;
    int expected = 3;
    Test("Test3", number, expected);
}

void TranslateNumbersToStrings::Test4() {
    int number = 126;
    int expected = 2;
    Test("Test4", number, expected);
}

void TranslateNumbersToStrings::Test5() {
    int number = 426;
    int expected = 1;
    Test("Test5", number, expected);
}

void TranslateNumbersToStrings::Test6() {
    int number = 100;
    int expected = 2;
    Test("Test6", number, expected);
}

void TranslateNumbersToStrings::Test7() {
    int number = 101;
    int expected = 2;
    Test("Test7", number, expected);
}

void TranslateNumbersToStrings::Test8() {
    int number = 12258;
    int expected = 5;
    Test("Test8", number, expected);
}

void TranslateNumbersToStrings::Test9() {
    int number = -100;
    int expected = 0;
    Test("Test9", number, expected);
}

void TranslateNumbersToStrings::Test() {
    Test1();
    Test2();
    Test3();
    Test4();
    Test5();
    Test6();
    Test7();
    Test8();
    Test9();
}
