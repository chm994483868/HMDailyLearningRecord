//
//  HeapSort.cpp
//  Algorithm_C
//
//  Created by HM C on 2022/7/13.
//

#include <stdio.h>

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void heapSort(int nums[], int count) {
    if (nums == nullptr || count <= 0) {
        return;
    }
    
    int i = (count - 1 - 1) / 2;
    
    for (; i >= 0; --i) {
        maxHeapFixdown(nums, i, count);
    }
    
    for (i = count - 1; i >= 1; --i) {
        swap(&nums[0], &nums[i]);
        maxHeapFixdown(nums, 0, i);
    }
}

void maxHeapFixdown(int nums[], int i, int n) {
    int j = i * 2 + 1;
    int temp = nums[i];
    
    while (j < n) {
        if (j + 1 < n && nums[j + 1] > nums[j]) {
            ++j;
        }
        
        if (nums[j] <= temp) {
            break;
        }
        
        swap(&nums[i], &nums[j]);
        
        i = j;
        j = i * 2 + 1;
    }
}
