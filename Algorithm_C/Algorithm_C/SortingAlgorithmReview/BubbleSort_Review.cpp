//
//  BubbleSort_Review.cpp
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

void bubbleSort(int nums[], int count) {
    if (nums == nullptr || count <= 0) {
        return;
    }
    
    int k = count - 1;
    for (int i = 0; i < count - 1; ++i) {
        bool noChange = true;
        int n = 0;
        for (int j = 0; j < k; ++j) {
            swap(&nums[j], &nums[j + 1]);
            noChange = false;
            n = j;
        }
        
        if (noChange) {
            break;
        }
        k = n;
    }
}

void bubbleSort_1(int nums[], int count) {
    if (nums == nullptr || count <= 0) {
        return;
    }
    
    int k = count - 1;
    for (int i = 0; i < count - 1; ++i) {
        bool noChange = true;
        int n = 0;
        for (int j = 0; j < k; ++j) {
            if (nums[j] > nums[j + 1]) {
                swap(&nums[j], &nums[j + 1]);
                noChange = false;
                n = j;
            }
        }
        if (noChange) {
            break;
        }
        k = n;
    }
}
