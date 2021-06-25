//
//  IntegerIdenticalToIndex.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/24.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "IntegerIdenticalToIndex.hpp"

int getNumberSameAsIndex(const int* numbers, int length) {
    if (numbers == nullptr || length <= 0) {
        return -1;
    }
    
    int left = 0;
    int right = length - 1;
    
    while (left <= right) {
        int middle = left + ((right - left) >> 1);
        if (numbers[middle] == middle) {
            return middle;
        }
        
        if (numbers[middle] > middle) {
            right = middle - 1;
        } else {
            left = middle + 1;
        }
    }
    
    return -1;
}
