//
//  ShellSort.hpp
//  OfferReview
//
//  Created by CHM on 2020/8/4.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef ShellSort_hpp
#define ShellSort_hpp

#include <stdio.h>
#include <stdlib.h>
#include "Array.hpp"

using namespace Partition;
using namespace std;

namespace ShellSort {

void shellSort(int nums[], int count);

// 测试代码
void test(string testName, int nums[], int count);

void test1();
void test2();
void test3();
void test4();
void test5();
void test6();

void Test();

}

#endif /* ShellSort_hpp */
