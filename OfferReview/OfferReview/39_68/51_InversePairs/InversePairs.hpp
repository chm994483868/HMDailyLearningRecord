//
//  InversePairs.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef InversePairs_hpp
#define InversePairs_hpp

#include <stdio.h>
#include <cstdio>

namespace InversePairs {

// 面试题51：数组中的逆序对
// 题目：在数组中的两个数字如果前面一个数字大于后面的数字，则这两个数字组
// 成一个逆序对。输入一个数组，求出这个数组中的逆序对的总数。

int InversePairsCore(int* data, int* copy, int start, int end);
int InversePairs(int* data, int length);

// 测试代码
void Test(char* testName, int* data, int length, int expected);

void Test1();
void Test2();
void Test3();
void Test4();
void Test5();
void Test6();
void Test7();
void Test8();

void Test();

}


#endif /* InversePairs_hpp */
