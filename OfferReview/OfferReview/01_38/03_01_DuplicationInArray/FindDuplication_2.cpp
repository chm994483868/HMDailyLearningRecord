//
//  FindDuplication_2.cpp
//  OfferReview
//
//  Created by CHM on 2021/1/29.
//  Copyright © 2021 CHM. All rights reserved.
//

#include "FindDuplication_2.hpp"

bool FindDuplication_2::duplicate(int numbers[], int length, int* duplication) {
    if (numbers == nullptr || length <= 0) {
        return false;
    }
    
    int i = 0;
    for (; i < length; ++i) {
        if (numbers[i] < 0 || numbers[i] > length - 1) {
            return false;
        }
    }
    
    for (i = 0; i < length; ++i) {
        while (numbers[i] != i) {
            if (numbers[i] == numbers[numbers[i]]) {
                *duplication = numbers[i];
                return true;
            }
            
            int temp = numbers[i];
            numbers[i] = numbers[temp];
            numbers[temp] = temp;
        }
    }
    
    return false;
}
