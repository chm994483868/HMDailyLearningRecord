//
//  LeftRotateString.cpp
//  Algorithm_C
//
//  Created by HM C on 2020/7/25.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "LeftRotateString.hpp"

void reverse_LeftRotateString(char* pBegin, char* pEnd) {
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

char* leftRotateString(char* pStr, int n) {
    if (pStr != nullptr) {
        int nLength = static_cast<int>(strlen(pStr));
        if (nLength > 0 && n > 0 && n < nLength) {
            char* pFirstStart = pStr;
            char* pFirstEnd = pStr + n - 1;
            
            char* pSecondStart = pStr + n;
            char* pSecondEnd = pStr + nLength - 1;
            
            // 翻转字符串的前面 n 个字符
            reverse_LeftRotateString(pFirstStart, pFirstEnd);
            // 翻转字符串的后面部分
            reverse_LeftRotateString(pSecondStart, pSecondEnd);
            // 翻转整个字符串
            reverse_LeftRotateString(pFirstStart, pSecondEnd);
        }
    }
    
    return pStr;
}
