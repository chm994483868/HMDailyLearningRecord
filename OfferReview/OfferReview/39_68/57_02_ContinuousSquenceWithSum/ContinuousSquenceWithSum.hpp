//
//  ContinuousSquenceWithSum.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef ContinuousSquenceWithSum_hpp
#define ContinuousSquenceWithSum_hpp

#include <stdio.h>
#include <cstdio>

namespace ContinuousSquenceWithSum {

// 面试题57（二）：和为 s 的连续正数序列
// 题目：输入一个正数 s，打印出所有和为 s 的连续正数序列（至少含有两个数）。
// 例如输入 15，由于 1+2+3+4+5 = 4+5+6 = 7+8 = 15，所以结果打印出 3 个连续序列 1〜5、
// 4〜6 和 7〜8。

void printContinuousSequence(int small, int big);
void findContinuousSequence(int sum);

}
#endif /* ContinuousSquenceWithSum_hpp */
