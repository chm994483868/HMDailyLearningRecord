//
//  ReplaceSpaces_2.cpp
//  OfferReview
//
//  Created by CHM on 2021/1/29.
//  Copyright © 2021 CHM. All rights reserved.
//

#include "ReplaceSpaces_2.hpp"

void ReplaceSpaces_2::replaceBlank(char str[], int length) {
    if (str == nullptr || length <= 0) {
        return;
    }
    
    int originalLength = 0;
    int numberOfBlank = 0;
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] == ' ') {
            ++numberOfBlank;
        }
        
        ++originalLength;
        ++i;
    }
    
    int newLength = originalLength + numberOfBlank * 2;
    if (newLength > length) {
        return;
    }
    
    int indexOfOriginal = originalLength;
    int indexOfNew = newLength;
    
    while (indexOfOriginal >= 0 && indexOfNew > originalLength) {
        if (str[indexOfOriginal] == ' ') {
            str[indexOfNew--] = '0';
            str[indexOfNew--] = '2';
            str[indexOfNew--] = '%';
        } else {
            str[indexOfNew--] = str[indexOfOriginal];
        }
        
        --indexOfOriginal;
    }
}
