//
//  FirstAppearingOnce.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef FirstAppearingOnce_hpp
#define FirstAppearingOnce_hpp

#include <stdio.h>
#include <cstdio>
#include <vector>
#include <limits>

using namespace std;

namespace FirstAppearingOnce {

// 面试题50（二）：字符流中第一个只出现一次的字符
// 题目：请实现一个函数用来找出字符流中第一个只出现一次的字符。例如，当从
// 字符流中只读出前两个字符"go"时，第一个只出现一次的字符是'g'。当从该字
// 符流中读出前六个字符"google"时，第一个只出现一次的字符是'l'。

class CharStatistics {
public:
    // 构造函数
    CharStatistics() : index(0) {
        // 初始化 occurrence 数组，-1 表示字符还没还没有找到
        // occurrence 下标表示字符的 ASCII 码，值表示出现的位置
        for (int i = 0; i < 256; ++i) {
            occurrence[i] = -1;
        }
    }
    
    // 把字符流中字符的 index 记录到 occurrence 中
    void Insert(char ch) {
        if (occurrence[ch] == -1) {
            // 第一次出现 occurrence[ch] 赋值为出现的位置
            occurrence[ch] = index;
        } else if (occurrence[ch] >= 0) {
            // -2 表示该字符已经出现过，重复了
            occurrence[ch] = -2;
        }
        
        index++;
    }
    
    // 第一个只出现一次的字符
    char firstAppearingOnce() {
        char ch = '\0';
        
        // minIndex 值初始为 int 类型数值的最大值
        int minIndex = numeric_limits<int>::max();
        
        // 遍历 occurrence 找到第一个出现一次的字符，该字符的 occurrence[i] 值是它在字符流中的位置
        for (int i = 0; i < 256; ++i) {
            if (occurrence[i] >= 0 && occurrence[i] < minIndex) {
                ch = (char)i;
                minIndex = occurrence[i];
            }
        }
        
        return ch;
    }
    
private:
    // occurrence[i]: A character with ASCII value i; 字符的 ASCII 码
    // occurrence[i] = -1: The character has not found; 未找到
    // occurrence[i] = -2: The character has been found for mutlple times 重复
    // occurrence[i] >= 0: The character has been found only once 出现一次
    int occurrence[256];
    int index;
};

// 测试代码
void Test(const char* testName, CharStatistics chars, char expected);

void Test();

}

#endif /* FirstAppearingOnce_hpp */
