//
//  HeapSort_Review.cpp
//  Algorithm_C
//
//  Created by HM C on 2022/12/28.
//

#include <stdio.h>
void maxHeapFixdown(int nums[], int i, int n);

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

void maxHeapFixDown_1(int nums[], int i, int n);
void heapSort_1(int nums[], int count) {
    if (nums == nullptr || count <= 0) {
        return;
    }
    int i = (count - 1 - 1) / 2; // 最后一个元素的父节点
    // 建堆
    for (; i >= 0; --i) {
        maxHeapFixDown_1(nums, i, count);
    }
    
    // 排序，每次把 0 节点放到数组的最后面，然后重新堆化数据
    for (i = count - 1; i >= 1; --i) {
        swap(&nums[0], &nums[i]);
        maxHeapFixDown_1(nums, 0, i);
    }
}

void maxHeapFixDown_1(int nums[], int i, int n) {
    int j = i * 2 + 1; // 左子节点
    int temp = nums[i]; // 节点的值
    
    while (j < n) {
        // 找到 i 节点的左右子节点中的较大值
        if (j + 1 < n && nums[j + 1] > nums[j]) {
            ++j;
        }
        
        // 如果父节点的值大于等于左右子节点的值，则不用交换
        if (nums[j] <= temp) {
            break;
        }
        
        // 把子节点中较大的值赋给父节点
        swap(&nums[i], &nums[j]);
        
        // 更新 i 和 j
        i = j;
        j = i * 2 + 1;
    }
}
