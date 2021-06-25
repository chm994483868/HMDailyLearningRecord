//
//  StringPermutation.cpp
//  OfferReview
//
//  Created by HM C on 2020/7/30.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "StringPermutation.hpp"

void StringPermutation::permutation(char* pStr, char* pBegin) {
    if (*pBegin == '\0') {
        printf("%s\n", pStr);
    } else {
        for (char* pCh = pBegin; *pCh != '\0'; ++pCh) {
            
            char temp = *pCh;
            *pCh = *pBegin;
            *pBegin = temp;
            
            // 递归每次 pBegin 前进一步
            permutation(pStr, pBegin + 1);
            
            temp = *pCh;
            *pCh = *pBegin;
            *pBegin = temp;
        }
    }
}

void StringPermutation::permutation(char* pStr) {
    if (pStr == nullptr) {
        return;
    }
    
    //
    permutation(pStr, pStr);
}

// 测试代码
void StringPermutation::Test(char* pStr) {
    if(pStr == nullptr)
        printf("Test for nullptr begins:\n");
    else
        printf("Test for %s begins:\n", pStr);

    permutation(pStr);

    printf("\n");
}

void StringPermutation::Test() {
    Test(nullptr);

    char string1[] = "";
    Test(string1);

    char string2[] = "a";
    Test(string2);

    char string3[] = "ab";
    Test(string3);

    char string4[] = "abc";
    Test(string4);
}
