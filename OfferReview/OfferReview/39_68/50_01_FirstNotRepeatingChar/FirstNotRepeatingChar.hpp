//
//  FirstNotRepeatingChar.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef FirstNotRepeatingChar_hpp
#define FirstNotRepeatingChar_hpp

#include <stdio.h>
#include <cstdio>
#include <string>

namespace FirstNotRepeatingChar {

// 面试题50（一）：字符串中第一个只出现一次的字符
// 题目：在字符串中找出第一个只出现一次的字符。如输入 "abaccdeff"，则输出 'b'。

char firstNotRepeatingChar(const char* pString);

// 测试代码
void Test(const char* pString, char expected);

void Test();

}

#endif /* FirstNotRepeatingChar_hpp */
