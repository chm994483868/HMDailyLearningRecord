//
//  Fibonacci.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/28.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef Fibonacci_hpp
#define Fibonacci_hpp

#include <stdio.h>

namespace Fibonacci {

// 开局的小问题
// 递归方式求 1+2+3+...+n
long long addFrom1ToN_Recursive(int n);
// 循环方式求 1+2+3+...+n
long long addFrom1ToN_Iterative(int n);

// 10：斐波那契数列
// 题目：写一个函数，输入n，求斐波那契（Fibonacci）数列的第n项。
// 方法1：递归
long long fibonacci_Solution1(unsigned int n);
// 方法2：循环
long long fibonacci_Solution2(unsigned int n);

// 测试代码
void Test(int n, int expected);

void Test();

}

#endif /* Fibonacci_hpp */
