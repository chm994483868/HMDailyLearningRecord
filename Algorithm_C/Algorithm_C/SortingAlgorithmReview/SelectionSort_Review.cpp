//
//  SelectionSort_Review.cpp
//  Algorithm_C
//
//  Created by HM C on 2022/12/28.
//

#include <stdio.h>

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void selectionSort(int nums[], int count) {
    if (nums == nullptr || count <= 0) {
        return;
    }
    
    for (int i = 0; i < count; ++i) {
        int minIndex = i; // 用于记录一趟下来找到的最小元素的下标，默认 i 位置是最小元素
        for (int j = i + 1; j < count; ++j) {
            if (nums[j] < nums[minIndex]) {
                minIndex = j; // 发现更小的元素，更新 minIndex
            }
        }
        
        swap(&nums[i], &nums[minIndex]);
    }
}

void selectionSort_1(int nums[], int count) {
    if (nums == nullptr || count <= 0) {
        return;
    }
    
    for (int i = 0; i < count; ++i) {
        int minIndex = i;
        for (int j = i + 1; j < count; ++j) {
            if (nums[j] < nums[minIndex]) {
                minIndex = j;
            }
        }
        swap(&nums[i], &nums[minIndex]);
    }
}
