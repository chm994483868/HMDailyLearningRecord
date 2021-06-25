//
//  SelectSort.cpp
//  OfferReview
//
//  Created by CHM on 2020/8/4.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "SelectSort.hpp"

void SelectSort::selectSort(int nums[], int count) {
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
    
    //if (nums == nullptr || count <= 0) {
    //    return;
    //}
    //
    //for (int i = 0; i < count; ++i) {
    //    int minIndex = i; // 用于记录一趟下来找到的最小元素的下标，默认 i 位置是最小元素
    //    for (int j = i + 1; j < count; ++j) {
    //        if (nums[j] < nums[minIndex]) {
    //            minIndex = j; // 发现更小的元素，更新 minIndex
    //        }
    //    }
    //
    //    swap(&nums[i], &nums[minIndex]); // 一趟循环下来把最小的元素放在数列的头部
    //}
}

// 测试代码
void SelectSort::test(char* testName, int nums[], int count) {
    printf("%s begins: \n", testName);
    printArray("", nums, count);
    selectSort(nums, count);
    printArray("", nums, count);
}

void SelectSort::test1() {
    int nums[] = {4, 6, 1, 2, 9, 10, 20, 1, 1};
    test("test1", nums, 8);
}

void SelectSort::test2() {
    int nums[] = {1, 2, 3, 4, 5, 6, 7, 8};
    test("test2", nums, 8);
}

void SelectSort::test3() {
    int nums[] = {5, 4, 3, 2, 1};
    test("test3", nums, 5);
}

void SelectSort::test4() {
    int nums[] = {-4, 6, -2, 9, 10, -20, 1, -1};
    test("test4", nums, 8);
}

void SelectSort::test5() {
    int nums[2] = {4, 2};
    test("test5", nums, 2);
}

void SelectSort::test6() {
    int nums[] = {};
    test("test5", nums, 0);
}

void SelectSort::Test() {
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
}

//if (nums == nullptr || count <= 0) {
//    return;
//}
//
//for (int i = 0; i < count; ++i) {
//    int minIndex = i; // 用于记录一趟下来找到的最小元素的下标，默认 i 位置是最小元素
//    for (int j = i + 1; j < count; ++j) {
//        if (nums[j] < nums[minIndex]) {
//            minIndex = j; // 发现更小的元素，更新 minIndex
//        }
//    }
//
//    swap(&nums[i], &nums[minIndex]); // 一趟循环下来把最小的元素放在数列的头部
//}
