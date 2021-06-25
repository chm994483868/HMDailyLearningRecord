//
//  TranslateNumbersToStrings.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/31.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef TranslateNumbersToStrings_hpp
#define TranslateNumbersToStrings_hpp

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

namespace TranslateNumbersToStrings {

// 46：把数字翻译成字符串
// 题目：给定一个数字，我们按照如下规则把它翻译为字符串：0翻译成"a"，1翻
// 译成"b"，⋯⋯，11翻译成"l"，⋯⋯，25翻译成"z"。一个数字可能有多个翻译。例
// 如12258有5种不同的翻译，它们分别是"bccfi"、"bwfi"、"bczi"、"mcfi"和
// "mzi"。请编程实现一个函数用来计算一个数字有多少种不同的翻译方法。

int getTranslationCount(const string& number);
int getTranslationCount(int number);

// 测试代码
void Test(const string& testName, int number, int expected);
void Test1();
void Test2();
void Test3();
void Test4();
void Test5();
void Test6();
void Test7();
void Test8();
void Test9();

void Test();

}

#endif /* TranslateNumbersToStrings_hpp */
