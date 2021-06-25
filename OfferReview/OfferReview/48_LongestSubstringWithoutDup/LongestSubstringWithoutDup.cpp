//
//  LongestSubstringWithoutDup.cpp
//  OfferReview
//
//  Created by CHM on 2020/7/31.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "LongestSubstringWithoutDup.hpp"

// 判断是否有重复出现的字符
bool LongestSubstringWithoutDup::hasDuplication(const std::string& str, int position[]) {
    for (int i = 0; i < 26; ++i) {
        position[i] = -1;
    }
    
    for (int i = 0; i < str.length(); ++i) {
        int indexInPosition = str[i] - 'a';
        if (position[indexInPosition] >= 0) {
            return true;
        }
        
        position[indexInPosition] = indexInPosition;
    }
    
    return false;
}

int LongestSubstringWithoutDup::longestSubstringWithoutDuplication_1(const std::string& str) {
    int longest = 0;
    int* position = new int[26];
    for (int start = 0; start < str.length(); ++start) {
        for (int end = start; end < str.length(); ++end) {
            int count = end - start + 1;
            const std::string& substring = str.substr(start, count);
            if (!hasDuplication(substring, position)) {
                if (count > longest) {
                    longest = count;
                } else {
                    break;
                }
            }
        }
    }
    
    delete [] position;
    return longest;
}

int LongestSubstringWithoutDup::longestSubstringWithoutDuplication_2(const std::string& str) {
    
    // 记录当前不重复字符串长度
    int curLength = 0;
    
    // 记录最大长度
    int maxLength = 0;
    
    // 准备一个长度是 26 的数组，标记 a 到 z 这 26 个字符上次出现的位置
    int* position = new int[26];
    unsigned int i = 0;
    // 数组元素全部置为 -1
    for (; i < 26; ++i) {
        position[i] = -1;
    }
    
    // 遍历字符串
    for (i = 0; i < str.length(); ++i) {
        
        // 首先从 position 数组中找出该字符上次出现的位置（str[i] - 'a' 作为下标）
        int prevIndex = position[str[i] - 'a'];
        
        // 如果还没出现过或者距离超过了当前 curLength，则 curLength 自增
        if (prevIndex < 0 || i - prevIndex > curLength) {
            ++curLength;
        } else {
            // 否则就是重复出现了，curLength 的值要变小了
            
            // 更新 maxLength
            if (curLength > maxLength) {
                maxLength = curLength;
            }
            
            // 更新当前长度 curLength
            curLength = i - prevIndex;
        }
        
        // 记录字符的出现的位置
        position[str[i] - 'a'] = i;
    }
    
    // 是否更新 maxLength
    if (curLength > maxLength) {
        maxLength = curLength;
    }
    
    // 释放内存
    delete [] position;
    // 返回 maxLength
    return maxLength;
}

// 测试代码
void LongestSubstringWithoutDup::testSolution1(const std::string& input, int expected) {
    int output = longestSubstringWithoutDuplication_1(input);
    if(output == expected)
        std::cout << "Solution 1 passed, with input: " << input << std::endl;
    else
        std::cout << "Solution 1 FAILED, with input: " << input << std::endl;
}

void LongestSubstringWithoutDup::testSolution2(const std::string& input, int expected) {
    int output = longestSubstringWithoutDuplication_2(input);
    if(output == expected)
        std::cout << "Solution 2 passed, with input: " << input << std::endl;
    else
        std::cout << "Solution 2 FAILED, with input: " << input << std::endl;
}

void LongestSubstringWithoutDup::test(const std::string& input, int expected) {
    testSolution1(input, expected);
    testSolution2(input, expected);
}

void LongestSubstringWithoutDup::test1() {
    const std::string input = "abcacfrar";
    int expected = 4;
    test(input, expected);
}

void LongestSubstringWithoutDup::test2() {
    const std::string input = "acfrarabc";
    int expected = 4;
    test(input, expected);
}

void LongestSubstringWithoutDup::test3() {
    const std::string input = "arabcacfr";
    int expected = 4;
    test(input, expected);
}

void LongestSubstringWithoutDup::test4() {
    const std::string input = "aaaa";
    int expected = 1;
    test(input, expected);
}

void LongestSubstringWithoutDup::test5() {
    const std::string input = "abcdefg";
       int expected = 7;
       test(input, expected);
}

void LongestSubstringWithoutDup::test6() {
    const std::string input = "aaabbbccc";
    int expected = 2;
    test(input, expected);
}

void LongestSubstringWithoutDup::test7() {
    const std::string input = "abcdcba";
    int expected = 4;
    test(input, expected);
}

void LongestSubstringWithoutDup::test8() {
    const std::string input = "abcdaef";
    int expected = 6;
    test(input, expected);
}

void LongestSubstringWithoutDup::test9() {
    const std::string input = "a";
    int expected = 1;
    test(input, expected);
}

void LongestSubstringWithoutDup::test10() {
    const std::string input = "";
    int expected = 0;
    test(input, expected);
}

void LongestSubstringWithoutDup::Test() {
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
    test8();
    test9();
    test10();
}
