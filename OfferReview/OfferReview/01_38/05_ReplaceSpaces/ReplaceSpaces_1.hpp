//
//  ReplaceSpaces_1.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/1.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef ReplaceSpaces_1_hpp
#define ReplaceSpaces_1_hpp

#include <stdio.h>
#include <iostream>
#include <cstring>
#include <string>
#include <algorithm>

using namespace std;

namespace ReplaceSpaces_1 {

// 5：替换空格
// 题目：请实现一个函数，把字符串中的每个空格替换成 "%20"。例如输入 “We are happy.”，
// 则输出 “We%20are%20happy.”。

void replaceBlank(char str[], int length);
string replaceSpace(string s);

// 测试代码
void Test(char* testName, char str[], int length, char expected[]);
// 空格在句子中间
void Test1();
// 空格在句子开头
void Test2();
// 空格在句子末尾
void Test3();
// 连续有两个空格
void Test4();
// 传入nullptr
void Test5();
// 传入内容为空的字符串
void Test6();
//传入内容为一个空格的字符串
void Test7();
// 传入的字符串没有空格
void Test8();
// 传入的字符串全是空格
void Test9();

void Test();

}

#endif /* ReplaceSpaces_1_hpp */
