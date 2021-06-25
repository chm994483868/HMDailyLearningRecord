//
//  QuickSort.cpp
//  OfferReview
//
//  Created by CHM on 2020/8/4.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "QuickSort.hpp"

void QuickSort::quickSort(int nums[], int l, int r) {
    if (nums == nullptr) {
        return;
    }
    
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
    
    //if (l >= r) {
    //    return;
    //}
    //
    //int i = l, j = r, x = nums[l];
    //while (i < j) {
    //    while (i < j && nums[j] >= x) {
    //        --j;
    //    }
    //    if (i < j) {
    //        nums[i++] = nums[j];
    //    }
    //
    //    while (i < j && nums[i] < x) {
    //        ++i;
    //    }
    //    if (i < j) {
    //        nums[j--] = nums[i];
    //    }
    //}
    //
    //nums[i] = x;
    //quickSort(nums, l, i - 1);
    //quickSort(nums, i + 1, r);
}

// 测试代码
void QuickSort::test(char* testName, int nums[], int count) {
    printf("%s begins: \n", testName);
    printArray("", nums, count);
    quickSort(nums, 0, count - 1);
    printArray("", nums, count);
}

void QuickSort::test1() {
    int nums[] = {4, 6, 1, 2, 9, 10, 20, 1, 1};
    test("test1", nums, 8);
}

void QuickSort::test2() {
    int nums[] = {1, 2, 3, 4, 5, 6, 7, 8};
    test("test2", nums, 8);
}

void QuickSort::test3() {
    int nums[] = {5, 4, 3, 2, 1};
    test("test3", nums, 5);
}

void QuickSort::test4() {
    int nums[] = {-4, 6, -2, 9, 10, -20, 1, -1};
    test("test4", nums, 8);
}

void QuickSort::test5() {
    int nums[2] = {4, 2};
    test("test5", nums, 2);
}

void QuickSort::test6() {
    int nums[] = {};
    test("test5", nums, 0);
}

void QuickSort::Test() {
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
}

//if (l >= r) {
//    return;
//}
//
//int i = l, j = r, x = nums[l];
//while (i < j) {
//    while (i < j && nums[j] >= x) {
//        --j;
//    }
//    if (i < j) {
//        nums[i++] = nums[j];
//    }
//
//    while (i < j && nums[i] < x) {
//        ++i;
//    }
//    if (i < j) {
//        nums[j--] = nums[i];
//    }
//}
//
//nums[i] = x;
//quickSort(nums, l, i - 1);
//quickSort(nums, i + 1, r);
