//
//  MergeSort.cpp
//  OfferReview
//
//  Created by CHM on 2020/8/4.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "MergeSort.hpp"

// 将有两个有序数列 a[first...mid] 和 a[mid...last] 合并
void MergeSort::mergeSort(int nums[], int first, int last, int temp[]) {
    if (nums == nullptr) {
        return;
    }
    
    if (first >= last) {
        return;
    }
    
    int mid = ((last - first) >> 1) + first;
    
    mergeSort(nums, first, mid, temp);
    mergeSort(nums, mid + 1, last, temp);
    
    mergeArray(nums, first, mid, last, temp);
    
    //if (first >= last) {
    //    return;
    //}
    //
    //int mid = ((last - first) >> 1) + first; // 右移一位等于除以 2，可防止 (first + last) / 2 中求和溢出
    //
    //mergeSort(nums, first, mid, temp); // 递归拆 [first...mid] 的数据
    //mergeSort(nums, mid + 1, last, temp); // 递归拆 [mid + 1...last] 的数据
    //
    //mergeArray(nums, first, mid, last, temp); // 递归结束开始合并
}

void MergeSort::mergeArray(int nums[], int first, int mid, int last, int temp[]) {
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
        temp[k++] = nums[i++];
    }
    
    while (j <= n) {
        temp[k++] = nums[j++];
    }
    
    for (i = 0; i < k; ++i) {
        nums[first + i] = temp[i];
    }
    
    //if (nums[mid] <= nums[mid + 1]) { // 左边序列的最大值小于右边序列的最小值，表示已经是有序的了，直接 return 就好了
    //    return;
    //}
    //
    //// 两个有序序列 nums[first...mid] nums[mid+1...last] 合并
    //int i = first, j = mid + 1; // i 和 j 表示各从两个数列的最左侧开始
    //int m = mid, n = last; // m 和 n 表示两个数列的最右侧边界
    //int k = 0; // k 用于记录两个数列的数据总个数
    //
    //while (i <= m && j <= n) {
    //    if (nums[i] <= nums[j]) {
    //        temp[k++] = nums[i++]; // 把 nums[first...mid] 中较小元素放在 temp 中
    //    } else {
    //        temp[k++] = nums[j++]; // 把 nums[mid+1...last] 中较小元素放在 temp 中
    //    }
    //}
    //
    //while (i <= m) { // 把 nums[first...mid] 中剩余的元素放在 temp 中
    //    temp[k++] = nums[i++];
    //}
    //
    //while (j <= n) { // 把 nums[mid+1...last] 中剩余的元素放在 temp 中
    //    temp[k++] = nums[j++];
    //}
    //
    //for (i = 0; i < k; ++i) { // temp 中是按顺序合并好的元素，然后放回到 nums 中
    //    nums[first + i] = temp[i];
    //}
}

// 测试代码
void MergeSort::test(char* testName, int nums[], int count) {
    printf("%s begins: \n", testName);
    printArray("", nums, count);
    
    int* temp = new int[count];
    mergeSort(nums, 0, count - 1, temp);
    delete [] temp;
    
    printArray("", nums, count);
}

void MergeSort::test1() {
    int nums[] = {4, 6, 1, 2, 9, 10, 20, 1, 1};
    test("test1", nums, 8);
}

void MergeSort::test2() {
    int nums[] = {1, 2, 3, 4, 5, 6, 7, 8};
    test("test2", nums, 8);
}

void MergeSort::test3() {
    int nums[] = {5, 4, 3, 2, 1};
    test("test3", nums, 5);
}

void MergeSort::test4() {
    int nums[] = {-4, 6, -2, 9, 10, -20, 1, -1};
    test("test4", nums, 8);
}

void MergeSort::test5() {
    int nums[2] = {4, 2};
    test("test5", nums, 2);
}

void MergeSort::test6() {
    int nums[] = {};
    test("test5", nums, 0);
}

void MergeSort::Test() {
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
}

//if (first >= last) {
//    return;
//}
//
//int mid = ((last - first) >> 1) + first; // 右移一位等于除以 2，可防止 (first + last) / 2 中求和溢出
//
//mergeSort(nums, first, mid, temp); // 递归拆 [first...mid] 的数据
//mergeSort(nums, mid + 1, last, temp); // 递归拆 [mid + 1...last] 的数据
//
//mergeArray(nums, first, mid, last, temp); // 递归结束开始合并


//if (nums[mid] <= nums[mid + 1]) { // 左边序列的最大值小于右边序列的最小值，表示已经是有序的了，直接 return 就好了
//    return;
//}
//
//// 两个有序序列 nums[first...mid] nums[mid+1...last] 合并
//int i = first, j = mid + 1; // i 和 j 表示各从两个数列的最左侧开始
//int m = mid, n = last; // m 和 n 表示两个数列的最右侧边界
//int k = 0; // k 用于记录两个数列的数据总个数
//
//while (i <= m && j <= n) {
//    if (nums[i] <= nums[j]) {
//        temp[k++] = nums[i++]; // 把 nums[first...mid] 中较小元素放在 temp 中
//    } else {
//        temp[k++] = nums[j++]; // 把 nums[mid+1...last] 中较小元素放在 temp 中
//    }
//}
//
//while (i <= m) { // 把 nums[first...mid] 中剩余的元素放在 temp 中
//    temp[k++] = nums[i++];
//}
//
//while (j <= n) { // 把 nums[mid+1...last] 中剩余的元素放在 temp 中
//    temp[k++] = nums[j++];
//}
//
//for (i = 0; i < k; ++i) { // temp 中是按顺序合并好的元素，然后放回到 nums 中
//    nums[first + i] = temp[i];
//}
