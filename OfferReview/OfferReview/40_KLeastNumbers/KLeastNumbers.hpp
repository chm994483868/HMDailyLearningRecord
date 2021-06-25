//
//  KLeastNumbers.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/31.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef KLeastNumbers_hpp
#define KLeastNumbers_hpp

#include <stdio.h>
#include <cstdio>
#include <set>
#include <vector>
#include <iostream>
#include <functional>

#include "Array.hpp"

using namespace Partition;
using namespace std;

namespace KLeastNumbers {

// 40：最小的k个数
// 题目：输入n个整数，找出其中最小的k个数。例如输入4、5、1、6、2、7、3、8
// 这8个数字，则最小的4个数字是1、2、3、4。
void getLeastNumbers_Solution1(int* input, int n, int* output, int k);

typedef multiset<int, std::greater<int>>  intSet;
typedef multiset<int, std::greater<int>>::iterator setIterator;

void getLeastNumbers_Solution2(const vector<int>& data, intSet& leastNumbers, int k);

// 测试代码
void Test(char* testName, int* data, int n, int* expectedResult, int k);
// k小于数组的长度
void Test1();
// k等于数组的长度
void Test2();
// k大于数组的长度
void Test3();
// k等于1
void Test4();
// k等于0
void Test5();
// 数组中有相同的数字
void Test6();
// 输入空指针
void Test7();

void Test();

}

#endif /* KLeastNumbers_hpp */
