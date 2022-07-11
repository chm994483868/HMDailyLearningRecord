//
//  QuickSort.cpp
//  Algorithm_C
//
//  Created by HM C on 2022/7/8.
//

#include <stdio.h>

void quickSort(int nums[], int l, int r) {
    if (l >= r) {
        return;
    }
    
    int i = l, j = r, x = nums[l];
    while (i < j) {
        while (i < j && nums[j] >= x) {
            --j;
        }
        if (i < j) {
            nums[i++] = nums[j];
        }
        
        while (i < j && nums[i] < x) {
            ++i;
        }
        if (i < j) {
            nums[j--] = nums[i];
        }
    }
    
    nums[i] = x;
    quickSort(nums, l, i - 1);
    quickSort(nums, i + 1, r);
}
