//
//  NumberOf1InBinary_1.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/3.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef NumberOf1InBinary_1_hpp
#define NumberOf1InBinary_1_hpp

#include <stdio.h>

namespace NumberOf1InBinary_1 {

// 相关题目：
// 用一条语句判断一个整数是不是 2 的整数次方。
// 一个整数如果是 2 的整数次方，那么它的二进制表示中有且只有一位是 1，而其它所有位都是 0。
// ((n - 1) & n) == 0? true: false;

// 输入两个整数 m 和 n，计算需要改变 m 的二进制表示中的多少位才能得到 n。
// 分两步，第一步求这两个数的异或，第二步统计异或结果中 1 的位数。

// 15：二进制中 1 的个数
// 题目：请实现一个函数，输入一个整数，输出该数二进制表示中 1 的个数。例如
// 把 9 表示成二进制是 1001，有 2 位是 1。因此如果输入 9，该函数输出 2。

int numberOf1_Solution1(int n);
int numberOf1_Solution2(int n);

// 测试代码
void Test(int number, unsigned int expected);

void Test();

}

#endif /* NumberOf1InBinary_1_hpp */
