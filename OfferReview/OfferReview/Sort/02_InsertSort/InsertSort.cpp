//
//  InsertSort.cpp
//  OfferReview
//
//  Created by CHM on 2020/8/4.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "InsertSort.hpp"

void InsertSort::insertSort(int nums[], int count) {
    if (nums == nullptr || count <= 0) {
        return;
    }
    
    for (int i = 1; i < count; ++i) {
        for (int j = i; j > 0 && nums[j - 1] > nums[j]; --j) {
            swap(&nums[j - 1], &nums[j]);
        }
    }
    
    //if (nums == nullptr || count <= 0) {
    //    return;
    //}
    //
    //// 把原数组在逻辑上分两个组，左边是已排序序列，右边是待排序序列
    //// 每次从右边序列取最后一个值插入左边序列中，并保持左边序列有序
    //// i 从 1 开始，0 定为左侧已排序序列第一个元素
    //for (int i = 1; i < count; ++i) {
    //    for (int j = i; j > 0 && nums[j - 1] > nums[j]; --j) {
    //        swap(&nums[j - 1], &nums[j]);
    //    }
    //}
}

// 测试代码
void InsertSort::test(char* testName, int nums[], int count) {
    printf("%s begins: \n", testName);
    printArray("", nums, count);
    insertSort(nums, count);
    printArray("", nums, count);
}

void InsertSort::test1() {
    int nums[9] = {4, 6, 1, 2, 9, 10, 20, 1, 1};
    test("test1", nums, 9);
}

void InsertSort::test2() {
    int nums[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    test("test2", nums, 8);
}

void InsertSort::test3() {
    int nums[5] = {5, 4, 3, 2, 1};
    test("test3", nums, 5);
}

void InsertSort::test4() {
    int nums[8] = {-4, 6, -2, 9, 10, -20, 1, -1};
    test("test4", nums, 8);
}

void InsertSort::test5() {
    int nums[2] = {4, 2};
    test("test5", nums, 2);
}

void InsertSort::test6() {
    int nums[] = {};
    test("test5", nums, 0);
}


void InsertSort::Test() {
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
//// 把原数组在逻辑上分两个组，左边是已排序序列，右边是待排序序列
//// 每次从右边序列取最后一个值插入左边序列中，并保持左边序列有序
//// i 从 1 开始，0 定为左侧已排序序列第一个元素
//for (int i = 1; i < count; ++i) {
//    for (int j = i; j > 0 && nums[j - 1] > nums[j]; --j) {
//        swap(&nums[j - 1], &nums[j]);
//    }
//}
