//
//  Print1ToMaxOfNDigits.hpp
//  OfferReview
//
//  Created by HM C on 2020/7/28.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef Print1ToMaxOfNDigits_hpp
#define Print1ToMaxOfNDigits_hpp

#include <stdio.h>
#include <memory>
#include <vector>

using namespace std;

namespace Print1ToMaxOfNDigits {

// 17：打印 1 到最大的 n 位数
// 题目：输入数字 n，按顺序打印出从 1 最大的 n 位十进制数。比如输入 3，则
// 打印出 1、2、3 一直到最大的 3 位数即 999。

void printNumber(char* number);
bool increment(char* number);
void print1ToMaxOfNDigits_1(int n);

void print1ToMaxOfNDigitsRecursively(char* number, int length, int index);
void print1ToMaxOfNDigits_2(int n);

// 测试代码
void Test(int n);

void Test();

}

#endif /* Print1ToMaxOfNDigits_hpp */
