//
//  MinNumberInRotatedArray_2.cpp
//  OfferReview
//
//  Created by CHM on 2021/2/22.
//  Copyright © 2021 CHM. All rights reserved.
//

#include "MinNumberInRotatedArray_2.hpp"

unsigned int MinNumberInRotatedArray_2::randomInRange(unsigned int start, unsigned int end) {
    int rand = (random() % (end - start + 1)) + start;
    return rand;
}

int MinNumberInRotatedArray_2::partition(int data[], int length, int start, int end) {
    if (data == nullptr || length <= 0 || start < 0 || end >= length) {
        throw std::exception();
    }
    
    int index = randomInRange(start, end);
    swap(&data[index], &data[end]);
    int small = start - 1;
    for (index = start; index < end; ++index) {
        if (data[index] < data[end]) {
            ++small;
            
            if (small != index) {
                swap(&data[small], &data[index]);
            }
        }
    }
    
    ++small;
    if (small != end) {
        swap(&data[small], &data[end]);
    }
    
    return small;
}

void MinNumberInRotatedArray_2::swap(int* num1, int* num2) {
    int temp = *num1;
    *num1 = *num2;
    *num2 = temp;
}

void MinNumberInRotatedArray_2::quickSort(int data[], int length, int start, int end) {
    if (start == end) {
        return;
    }
    
    int index = partition(data, length, start, end);
    if (index > start) {
        quickSort(data, length, start, index - 1);
    }
    
    if (index < end) {
        quickSort(data, length, index + 1, end);
    }
}

void MinNumberInRotatedArray_2::sortAges(int ages[], int length) {
    if (ages == nullptr || length <= 0) {
        return;
    }
    
    const int oldestAge = 99;
    int timesOfAge[oldestAge + 1];
    int i = 0;
    for (; i <= oldestAge; ++i) {
        timesOfAge[i] = 0;
    }
    
    for (i = 0; i < length; ++i) {
        int age = ages[i];
        if (age < 0 || age > oldestAge) {
            throw std::exception();
        }
        
        ++timesOfAge[age];
    }
    
    int index = 0;
    for (i = 0; i <= oldestAge; ++i) {
        int count = timesOfAge[i];
        while (count > 0) {
            ages[index] = i;
            ++index;
            --count;
        }
    }
}
