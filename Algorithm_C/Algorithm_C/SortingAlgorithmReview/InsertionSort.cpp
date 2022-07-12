//
//  InsertionSort.cpp
//  Algorithm_C
//
//  Created by HM C on 2022/7/7.
//

#include <stdio.h>
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void insertSort(int nums[], int count) {
    if (nums == nullptr || count <= 0) {
        return;
    }
    
    for (int i = 1; i < count; ++i) {
        for (int j = i; j > 0 && nums[j - 1] > nums[j]; --j) {
            swap(&nums[j - 1], &nums[j]);
        }
    }
}

void insertSort(int nums[], int count) {
    if (nums == nullptr || count <= 0) {
        return;
    }
    
    for (int i = 1; i < count; ++i) {
        for (int j = i; j > 0 && nums[j - 1] > nums[j]; --j) {
            swap(&nums[j - 1], &nums[j]);
        }
    }
}

void insertSort(int nums[], int count) {
    if (nums == nullptr || count <= 0) {
        return;
    }
    
    for (int i = 1; i < count; ++i) {
        for (int j = i; j > 0 && nums[j - 1] > nums[j]; --j) {
            swap(&nums[j - 1], &nums[j]);
        }
    }
}
