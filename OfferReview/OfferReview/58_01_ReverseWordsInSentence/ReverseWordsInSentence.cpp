//
//  ReverseWordsInSentence.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "ReverseWordsInSentence.hpp"

// 翻转字符串，准备两个指针，一个从字符串头部开始，一个从尾部开始，
// 左边指针递增，右边指针递减，交换两个指针指向的字符串
void ReverseWordsInSentence::reverse(char* pBegin, char* pEnd) {
    if (pBegin == nullptr || pEnd == nullptr) {
        return;
    }
    
    while (pBegin < pEnd) {
        // 交换字符
        char temp = *pBegin;
        *pBegin = *pEnd;
        *pEnd = temp;
        
        // 左边指针递增
        ++pBegin;
        // 右边指针递减
        --pEnd;
    }
}

char* ReverseWordsInSentence::reverseSentence(char* pData) {
    // 判空
    if (pData == nullptr) {
        return nullptr;
    }
    
    // 准备两个指针 pBegin 和 pEnd，分别指向开头和末尾
    char* pBegin = pData;
    char* pEnd = pData;
    while (*pEnd != '\0') {
        ++pEnd;
    }
    --pEnd;
    
    // 翻转整个句子
    reverse(pBegin, pEnd);
    
    // 翻转句子中的每个单词
    //（三个指针都是从头开始）
    pBegin = pEnd = pData;
    
    while (*pBegin != '\0') {
        // 首先是 pBegin 指向字符串开头，
        // pEnd 从头往前走，直到遇到空格
        if (*pBegin == ' ') {
            // 空格时，两者都进行前移
            ++pBegin;
            ++pEnd;
        } else if (*pEnd == ' ' || *pEnd == '\0') {
            // *pEnd 是一个空格时，表示 pEnd 到了一个单词的末尾，
            // [pBegin, --pEnd] 刚好是一个单词的区间，翻转一个单词
            reverse(pBegin, --pEnd);
            
            // 更新 pBegin 到下一个单词的开头
            pBegin = ++pEnd;
        } else {
            // pEnd 正常前进，直到遇到空格
            ++pEnd;
        }
    }
    
    return pData;
}
