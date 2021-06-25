//
//  StringPermutation.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/20.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "StringPermutation.hpp"

void permutation(char* pStr, char* pBegin) {
    if (*pBegin == '\0') {
        printf("%s\n", pStr);
    } else {
        for (char* pCh = pBegin; *pCh != '\0'; ++pCh) {
            char temp = *pCh;
            *pCh = *pBegin;
            *pBegin = temp;
            
            permutation(pStr, pBegin + 1);
            
            temp = *pCh;
            *pCh = *pBegin;
            *pBegin = temp;
        }
    }
}

void permutation(char* pStr) {
    if (pStr == nullptr) {
        return;
    }
    
    permutation(pStr, pStr);
}

void permutation_Review(char* pStr, char* pBegin) {
    if (*pBegin == '\0') {
        printf("%s\n", pStr);
    } else {
        for (char* pCh = pBegin; *pCh != '\0'; ++pCh) {
            char temp = *pCh;
            *pCh = *pBegin;
            *pBegin = temp;
            
            permutation(pStr, pBegin + 1);
            
            temp = *pCh;
            *pCh = *pBegin;
            *pBegin = temp;
        }
    }
}

void permutation_Review(char* pStr) {
    if (pStr == nullptr) {
        return;
    }
    
    permutation_Review(pStr, pStr);
}
