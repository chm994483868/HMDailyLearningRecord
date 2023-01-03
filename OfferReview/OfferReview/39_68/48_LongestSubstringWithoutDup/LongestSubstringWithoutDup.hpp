//
//  LongestSubstringWithoutDup.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/31.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef LongestSubstringWithoutDup_hpp
#define LongestSubstringWithoutDup_hpp

#include <stdio.h>
#include <string>
#include <iostream>

namespace LongestSubstringWithoutDup {

// 48：最长不含重复字符的子字符串
// 题目：请从字符串中找出一个最长的不包含重复字符的子字符串，计算该最长子
// 字符串的长度。假设字符串中只包含从'a'到'z'的字符。
// 方法一：蛮力法
bool hasDuplication(const std::string& str, int position[]);
int longestSubstringWithoutDuplication_1(const std::string& str);

//// 方法二：动态规划
int longestSubstringWithoutDuplication_2(const std::string& str);

// 测试代码
void testSolution1(const std::string& input, int expected);
void testSolution2(const std::string& input, int expected);
void test(const std::string& input, int expected);
void test1();
void test2();
void test3();
void test4();
void test5();
void test6();
void test7();
void test8();
void test9();
void test10();

void Test();
    
}

#endif /* LongestSubstringWithoutDup_hpp */
