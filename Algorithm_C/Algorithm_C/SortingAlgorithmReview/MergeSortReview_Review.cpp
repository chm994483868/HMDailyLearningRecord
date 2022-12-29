//
//  MergeSortReview_Review.cpp
//  Algorithm_C
//
//  Created by HM C on 2022/12/28.
//

#include <stdio.h>
void mergeArray(int nums[], int first, int mid, int last, int temp[]);

void mergeSort(int nums[], int first, int last, int temp[]) {
    if (first >= last) {
        return;
    }
    
    int mid = ((last - first) >> 1) + first; // 右移一位等于除以 2，可防止 (first + last) / 2 中求和溢出
    mergeSort(nums, first, mid, temp); // 递归拆 [first...mid] 的数据
    mergeSort(nums, mid + 1, last, temp); // 递归拆 [mid + 1...last] 的数据
    
    mergeArray(nums, first, mid, last, temp); // 递归结束开始合并
}

void mergeArray(int nums[], int first, int mid, int last, int temp[]) {
    if (nums[mid] <= nums[mid + 1]) {
        return;
    }
    
    // 两个有序序列 nums[first...mid] nums[mid + 1...last] 合并
    int i = first, j = mid + 1; // i 和 j 表示各从两个数列的最左侧开始
    int m = mid, n = last; // m 和 n 表示两个数列的最右侧边界
    int k = 0; // k 用于记录两个数列的数据总个数
    
    while (i <= m && j <= n) {
        if (nums[i] <= nums[j]) {
            temp[k++] = nums[i++]; // 把 nums[first...mid] 中较小元素放在 temp 中
        } else {
            temp[k++] = nums[j++]; // 把 nums[mid + 1...last] 中较小元素放在 temp 中
        }
    }
    
    while (i <= m) { // 把 nums[first...mid] 中剩余的元素放在 temp 中
        temp[k++] = nums[i++];
    }
    
    while (j <= n) { // 把 nums[mid + 1...last] 中剩余的元素放在 temp 中
        temp[k++] = nums[j++];
    }
    
    for (i = 0; i < k; ++i) { // temp 中是按顺序合并好的元素，然后放回到 nums 中
        nums[first + i] = temp[i];
    }
}

void mergeArray_1(int nums[], int first, int mid, int last, int temp[]);
void mergeSort_1(int nums[], int first, int last, int temp[]) {
    if (first >= last) {
        return;
    }
    
    int mid = ((last - first) >> 1) + first;
    mergeSort_1(nums, first, mid, temp);
    mergeSort_1(nums, mid + 1, last, temp);
    mergeArray_1(nums, first, mid, last, temp);
}

void mergeArray_1(int nums[], int first, int mid, int last, int temp[]) {
    if (nums[mid] <= nums[mid + 1]) {
        return;
    }
    
    int i = first, j = mid + 1;
    int m = mid, n = last;
    int k = 0;
    
    while (i <= m && j <= n) {
        if (nums[i] <= nums[j]) {
            temp[k++] = nums[i++];
        } else {
            temp[k++] = nums[j++];
        }
    }
    
    while (i <= m) {
        temp[k++] = nums[i++];
    }
    
    while (j <= n) {
        temp[k++] = nums[j++];
    }
    
    for (i = 0; i < k; ++i) {
        nums[first + i] = temp[i];
    }
}

void mergeArray_2(int nums[], int first, int mid, int last, int temp[]);
void mergeSort_2(int nums[], int first, int last, int temp[]) {
    if (first >= last) {
        return;
    }
    
    int mid = ((last - first) >> 1) + first;
    mergeSort_2(nums, first, mid, temp);
    mergeSort_2(nums, mid + 1, last, temp);
    mergeArray_2(nums, first, mid, last, temp);
}

void mergeArray_2(int nums[], int first, int mid, int last, int temp[]) {
    if (nums[mid] < nums[mid + 1]) {
        return;
    }
    
    int i = first, j = mid + 1;
    int m = mid, n = last;
    int k = 0;
    
    while (i <= m && j <= n) {
        if (nums[i] <= nums[j]) {
            temp[k++] = nums[i++];
        } else {
            temp[k++] = nums[j++];
        }
    }
    
    while (i <= m) {
        temp[k++] = temp[i++];
    }
    
    while (j <= n) {
        temp[k++] = temp[j++];
    }
    
    for (i = 0; i < k; ++i) {
        nums[first + i] = temp[i];
    }
}
