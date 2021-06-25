//
//  NumericStrings.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/29.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef NumericStrings_hpp
#define NumericStrings_hpp

#include <stdio.h>

namespace NumericStrings {

// 20：表示数值的字符串
// 题目：请实现一个函数用来判断字符串是否表示数值（包括整数和小数）。例如，
// 字符串“+100”、“5e2”、“-123”、“3.1416”及“-1E-16”都表示数值，但“12e”、
// “1a3.14”、“1.2.3”、“+-5”及“12e+5.4”都不是

// 每次指针往前扫描遇到不符合的条件则停止扫描，并不是一次扫描完毕...
// 在最后的 return 里面自会判断，是否到达了字符串末尾
// 如果没有到达字符串末尾，则不符合...
bool scanUnsignedInteger(const char** str);
bool scanInteger(const char** str);

bool isNumeric(const char* str);

// 测试代码
void Test(const char* testName, const char* str, bool expected);

void Test();

}

#endif /* NumericStrings_hpp */
