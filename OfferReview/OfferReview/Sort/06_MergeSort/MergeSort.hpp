//
//  MergeSort.hpp
//  OfferReview
//
//  Created by CHM on 2020/8/4.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef MergeSort_hpp
#define MergeSort_hpp

#include <stdio.h>
#include "Array.hpp"

using namespace Partition;

namespace MergeSort {

void mergeSort(int nums[], int first, int last, int temp[]);
void mergeArray(int nums[], int first, int mid, int last, int temp[]);

// 测试代码
void test(char* testName, int nums[], int count);

void test1();
void test2();
void test3();
void test4();
void test5();
void test6();

void Test();

}

#endif /* MergeSort_hpp */
