//
//  LastNumberInCircle.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/11.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef LastNumberInCircle_hpp
#define LastNumberInCircle_hpp

#include <stdio.h>
#include <cstdio>
#include <list>

using namespace std;

namespace LastNumberInCircle {

// 面试题62：圆圈中最后剩下的数字
// 题目：0, 1, ⋯, n-1这n个数字排成一个圆圈，从数字0开始每次从这个圆圈里
// 删除第m个数字。求出这个圆圈里剩下的最后一个数字。

int lastRemaining_Solution1(unsigned int n, unsigned int m);
int lastRemaining_Solution2(unsigned int n, unsigned int m);

}

#endif /* LastNumberInCircle_hpp */
