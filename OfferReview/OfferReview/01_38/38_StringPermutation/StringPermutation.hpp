//
//  StringPermutation.hpp
//  OfferReview
//
//  Created by HM C on 2020/7/30.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef StringPermutation_hpp
#define StringPermutation_hpp

#include <stdio.h>

namespace StringPermutation {

// 38：字符串的排列
// 题目：输入一个字符串，打印出该字符串中字符的所有排列。例如输入字符串abc，
// 则打印出由字符a、b、c所能排列出来的所有字符串abc、acb、bac、bca、cab和cba。
void permutation(char* pStr, char* pBegin);
void permutation(char* pStr);

// 测试代码
void Test(char* pStr);
void Test();

}

#endif /* StringPermutation_hpp */
