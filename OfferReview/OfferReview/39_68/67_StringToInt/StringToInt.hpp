//
//  StringToInt.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/11.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef StringToInt_hpp
#define StringToInt_hpp

#include <stdio.h>
#include <cstdio>

namespace StringToInt {

// 面试题67：把字符串转换成整数
// 题目：请你写一个函数 StrToInt，实现把字符串转换成整数这个功能。当然，不
// 能使用 atoi 或者其他类似的库函数。

enum States { kValid = 0, kInvalid};
int g_nStatus = kValid;

long long strToIntCore(const char* str, bool minus);
int strToInt(const char* str);

}
#endif /* StringToInt_hpp */
