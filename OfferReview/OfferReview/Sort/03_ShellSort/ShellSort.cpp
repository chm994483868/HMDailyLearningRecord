//
//  ShellSort.cpp
//  OfferReview
//
//  Created by CHM on 2020/8/4.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "ShellSort.hpp"

void ShellSort::shellSort(int nums[], int count) {
    if (nums == nullptr || count <= 0) {
        return;
    }
    
    for (int gap = count / 2; gap > 0; gap = gap / 2) {
        for (int i = gap; i < count; ++i) {
            for (int j = i - gap; j >= 0 && nums[j] > nums[j + gap]; j -= gap) {
                swap(&nums[j], &nums[j + gap]);
            }
        }
    }
    
    for (int gap = count / 2; gap > 0; gap = gap / 2) {
        for (int i = 0; i < gap; ++i) {
            for (int j = i + gap; j < count; j = j + gap) {
                for (int k = j - gap; k >= 0 && nums[k] > nums[k + gap]; k -= gap) {
                    swap(&nums[k], &nums[k + gap]);
                }
            }
        }
    }
    
    //    if (nums == nullptr || count <= 0) {
    //        return;
    //    }
    //
    //    //    for (int gap = count / 2; gap > 0; gap /= 2) {
    //    //        for (int i = 0; i < gap; ++i) {
    //    //            for (int j = i + gap; j < count; j += gap) {
    //    //                for (int k = j - gap; k >= 0 && nums[k] > nums[k + gap]; k -= gap) {
    //    //                    swap(&nums[k], &nums[k + gap]);
    //    //                }
    //    //            }
    //    //        }
    //    //    }
    //
    //    for (int gap = count / 2; gap > 0; gap /= 2) {
    //        for (int i = gap; i < count; ++i) {
    //            for (int j = i - gap; j >= 0 && nums[j] > nums[j + gap]; j -= gap) { // j 是左边元素
    //                swap(&nums[j], &nums[j + gap]);
    //            }
    //        }
    //    }
    
    
}

// 测试代码
void ShellSort::test(char* testName, int nums[], int count) {
    printf("%s begins: \n", testName);
    printArray("", nums, count);
    shellSort(nums, count);
    printArray("", nums, count);
}

void ShellSort::test1() {
    int nums[] = {4, 6, 1, 2, 9, 10, 20, 1, 1};
    test("test1", nums, 8);
}

void ShellSort::test2() {
    int nums[] = {1, 2, 3, 4, 5, 6, 7, 8};
    test("test2", nums, 8);
}

void ShellSort::test3() {
    int nums[] = {5, 4, 3, 2, 1};
    test("test3", nums, 5);
}

void ShellSort::test4() {
    int nums[] = {-4, 6, -2, 9, 10, -20, 1, -1};
    test("test4", nums, 8);
}

void ShellSort::test5() {
    int nums[2] = {4, 2};
    test("test5", nums, 2);
}

void ShellSort::test6() {
    int nums[] = {};
    test("test5", nums, 0);
}

void ShellSort::Test() {
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
}

//    if (nums == nullptr || count <= 0) {
//        return;
//    }
//
//    //    for (int gap = count / 2; gap > 0; gap /= 2) {
//    //        for (int i = 0; i < gap; ++i) {
//    //            for (int j = i + gap; j < count; j += gap) {
//    //                for (int k = j - gap; k >= 0 && nums[k] > nums[k + gap]; k -= gap) {
//    //                    swap(&nums[k], &nums[k + gap]);
//    //                }
//    //            }
//    //        }
//    //    }
//
//    for (int gap = count / 2; gap > 0; gap /= 2) {
//        for (int i = gap; i < count; ++i) {
//            for (int j = i - gap; j >= 0 && nums[j] > nums[j + gap]; j -= gap) { // j 是左边元素
//                swap(&nums[j], &nums[j + gap]);
//            }
//        }
//    }
