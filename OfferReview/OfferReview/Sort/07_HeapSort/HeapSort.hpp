//
//  HeapSort.hpp
//  OfferReview
//
//  Created by CHM on 2020/8/4.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef HeapSort_hpp
#define HeapSort_hpp

#include <stdio.h>
#include "Array.hpp"

using namespace Partition;

namespace HeapSort {

void heapSort(int nums[], int count);
void maxHeapFixdown(int nums[], int i, int n);

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

#endif /* HeapSort_hpp */
