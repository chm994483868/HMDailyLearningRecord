//
//  LeftRotateString.cpp
//  OfferReview
//
//  Created by HM C on 2020/11/11.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "LeftRotateString.hpp"

// 三步完成，还是使用前面的字符串翻转函数，
// 这里是先翻转字符串前面 n 个字符，
// 再翻转字符串的后面部分，
// 最后翻转整个字符串，

char* LeftRotateString::leftRotateString(char* pStr, int n) {
    if (pStr != nullptr) {
        
        // 获取字符串长度
        int nLength = static_cast<int>(strlen(pStr));
        
        if (nLength > 0 && n > 0 && n < nLength) {
            
            // 把字符串在逻辑上分为两个部分:
            // 字符串 1 [pStr, pStr + n - 1]
            char* pFirstStart = pStr;
            char* pFirstEnd = pStr + n - 1;
            
            // 字符串 2 [pStr + n, pStr + nLength - 1]
            char* pSecondStart = pStr + n;
            char* pSecondEnd = pStr + nLength - 1;
            
            // 翻转字符串的前面 n 个字符
            reverse(pFirstStart, pFirstEnd);
            // 翻转字符串的后面部分
            reverse(pSecondStart, pSecondEnd);
            // 翻转整个字符串
            reverse(pFirstStart, pSecondEnd);
        }
    }
    
    return pStr;
}

// 翻转字符串，准备两个指针，一个从字符串头部开始，一个从尾部开始，
// 左边指针递增，右边指针递减，交换两个指针指向的字符串
void LeftRotateString::reverse(char* pBegin, char* pEnd) {
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
