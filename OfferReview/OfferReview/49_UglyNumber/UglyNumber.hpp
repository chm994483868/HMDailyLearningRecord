//
//  UglyNumber.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef UglyNumber_hpp
#define UglyNumber_hpp

#include <stdio.h>
#include <cstdio>

namespace UglyNumber {

// 面试题49：丑数
// 题目：我们把只包含因子2、3和5的数称作丑数（Ugly Number）。求按从小到
// 大的顺序的第1500个丑数。例如6、8都是丑数，但14不是，因为它包含因子7。
// 习惯上我们把1当做第一个丑数。

bool IsUgly(int number);
int GetUglyNumber_Solution1(int index);

int Min(int number1, int number2, int number3);
int GetUglyNumber_Solution2(int index);

// 测试代码
void Test(int index, int expected);

void Test();

}

#endif /* UglyNumber_hpp */
