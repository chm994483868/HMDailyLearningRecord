//
//  NumberOf1InBinary.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/28.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef NumberOf1InBinary_hpp
#define NumberOf1InBinary_hpp

#include <stdio.h>

namespace NumberOf1InBinary {

// 相关题目:
// 用一条语句判断一个整数是不是 2 的整数次方。
// 一个整数如果是 2 的整数次方，那么它的二进制表示中有且只有一位是 1，而其它所有位都是 0。
// ((n - 1) & n) == 0? true: false;

// 输入两个整数 m 和 n，计算需要改变 m 的二进制表示中的多少位才能得到 n。
// 分两步，第一步求这两个数的异或；第二步统计异或结果中 1 的位数。

// 15：二进制中1的个数
// 题目：请实现一个函数，输入一个整数，输出该数二进制表示中1的个数。例如
// 把9表示成二进制是1001，有2位是1。因此如果输入9，该函数输出2。

int numberOf1_Solution1(int n);
int numberOf1_Solution2(int n);

// 测试代码
void Test(int number, unsigned int expected);

void Test();

}

#endif /* NumberOf1InBinary_hpp */
