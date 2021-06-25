//
//  Fibonacci_1.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/2.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "Fibonacci_1.hpp"

// 递归方式求 1+2+3+...+n
long long Fibonacci_1::addFrom1ToN_Recursive(int n) {
    return n <= 0? 0: n + addFrom1ToN_Recursive(n - 1);
}

// 循环方式求 1+2+3+...+n
long long Fibonacci_1::addFrom1ToN_Iterative(int n) {
    int sum = 0;
    for (int i = 1; i <= n; ++i) {
        sum += i;
    }
    return sum;
}

long long Fibonacci_1::fibonacci_Solution1(unsigned int n) {
    if (n <= 0) {
        return 0;
    }
    
    if (n == 1) {
        return 1;
    }
    
    return fibonacci_Solution1(n - 1) + fibonacci_Solution1(n - 2);
}

long long Fibonacci_1::fibonacci_Solution2(unsigned int n) {
    int results[] = {0, 1};
    
    if (n < 2) {
        return results[n];
    }
    
    int fibonacciOne = 0;
    int fibonacciTwo = 1;
    int fibN = 0;
    for (int i = 2; i <= n; ++i) {
        fibN = fibonacciOne + fibonacciTwo;
        fibonacciOne = fibonacciTwo;
        
        fibonacciTwo = fibN;
    }
    
    return fibN;
}

// 测试代码
void Fibonacci_1::Test(int n, int expected) {
    printf("fibonacci_Solution1 = %lld", fibonacci_Solution1(n));
    
    if(fibonacci_Solution1(n) == expected)
        printf("Test for %d in solution1 passed.\n", n);
    else
        printf("Test for %d in solution1 failed.\n", n);
    
    printf("fibonacci_Solution2 = %lld", fibonacci_Solution2(n));
    if(fibonacci_Solution2(n) == expected)
        printf("Test for %d in solution2 passed.\n", n);
    else
        printf("Test for %d in solution2 failed.\n", n);
}

void Fibonacci_1::Test() {
    Test(0, 0);
    Test(1, 1);
    Test(2, 1);
    Test(3, 2);
    Test(4, 3);
    Test(5, 5);
    Test(6, 8);
    Test(7, 13);
    Test(8, 21);
    Test(9, 34);
    Test(10, 55);
    
    Test(40, 102334155);
    
    Test(45, 22222);
}
