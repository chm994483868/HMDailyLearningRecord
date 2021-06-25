//
//  ReverseWordsInSentence.cpp
//  Algorithm_C
//
//  Created by HM C on 2020/7/25.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "ReverseWordsInSentence.hpp"

void reverse(char* pBegin, char* pEnd) {
    if (pBegin == nullptr || pEnd == nullptr) {
        return;
    }
    
    while (pBegin < pEnd) {
        char temp = *pBegin;
        *pBegin = *pEnd;
        *pEnd = temp;
        
        ++pBegin;
        --pEnd;
    }
}

char* reverseSentence(char* pData) {
    if (pData == nullptr) {
        return nullptr;
    }
    
    char* pBegin = pData;
    char* pEnd = pData;
    
    // 1. 从头开始向后移动 pEnd，直到 pEend 指向最后的空字符
    while (*pEnd != '\0') {
        ++pEnd;
    }
    // 2. pEnd 往前走一位，指向最后一个字符
    --pEnd;
    // 3. 翻转整个字符串
    reverse(pBegin, pEnd);
    // 4. 翻转句子中的每个单词
    // 开始时 pBegin 和 pEnd 都指向字符串的开头
    pBegin = pEnd = pData;
    // 循环结束的条件是 pBegin 到达字符串的后面的空字符
    // We are student.
    // .tneduts era eW
    while (*pBegin != '\0') {
        if (*pBegin == ' ') {
            // 5): 同时把 pEnd 和 pBegin 向后移动一位，同时指向下一个单词的首字母
            ++pBegin;
            ++pEnd;
            // 2): 直到 pEnd 指到了第一个空格(后面的 pEnd == '\0' 是指，最后一个单词它的后面是 '\0')
        } else if (*pEnd == ' ' || *pEnd == '\0') {
            // 3): 翻转当前的单词
            reverse(pBegin, --pEnd);
            // 4): 此时 pEnd 自增 1 指向这个单词的后面的空格，pBegin 也指向这个单词后面的空格
            pBegin = ++pEnd; // 6): 最后一个单词翻转完成后，此二者同时指向字符串最后的空字符
        } else {
            // 1): 开始时 pEnd 一直向后移动
            ++pEnd;
        }
    }
    
    return pData;
}
