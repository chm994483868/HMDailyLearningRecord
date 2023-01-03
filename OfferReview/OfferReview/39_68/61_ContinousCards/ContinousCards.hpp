//
//  ContinousCards.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/11.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef ContinousCards_hpp
#define ContinousCards_hpp

#include <stdio.h>
#include <cstdio>
#include <cstdlib>

namespace ContinousCards {

// 面试题61：扑克牌的顺子
// 题目：从扑克牌中随机抽5张牌，判断是不是一个顺子，即这5张牌是不是连续的。
// 2〜10 为数字本身，A 为 1，J 为 11，Q 为 12，K 为 13，而大、小王可以看成任意数字。

int compare(const void* arg1, const void* arg2);
bool isContinuous(int* numbers, int length);

}


#endif /* ContinousCards_hpp */
