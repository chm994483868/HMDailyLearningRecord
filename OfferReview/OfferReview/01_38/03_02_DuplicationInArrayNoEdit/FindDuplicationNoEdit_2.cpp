//
//  FindDuplicationNoEdit_2.cpp
//  OfferReview
//
//  Created by CHM on 2021/1/29.
//  Copyright © 2021 CHM. All rights reserved.
//

#include "FindDuplicationNoEdit_2.hpp"

int FindDuplicationNoEdit_2::countRange(const int* numbers, int length, int start, int end) {
    if (numbers == nullptr) {
        return 0;
    }
    
    int count = 0;
    for (int i = 0; i < length; ++i) {
        if (numbers[i] >= start && numbers[i] <= end) {
            ++count;
        }
    }
    
    return count;
}

int FindDuplicationNoEdit_2::getDuplication(const int* number, int length) {
    if (number == nullptr || length <= 0) {
        return -1;
    }
    
    int start = 1;
    int end = length - 1;
    
    while (start <= end) {
        int middle = ((end - start) >> 1) + start;
        int count = countRange(number, length, start, middle);
        
        if (start == end) {
            if (count > 1) {
                return start;
            } else {
                break;
            }
        }
        
        if (count > (middle - start + 1)) {
            end = middle;
        } else {
            start = middle + 1;
        }
    }
    
    return -1;
}
