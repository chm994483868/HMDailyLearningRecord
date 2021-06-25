//
//  Fibonacci_2.cpp
//  OfferReview
//
//  Created by CHM on 2021/2/22.
//  Copyright © 2021 CHM. All rights reserved.
//

#include "Fibonacci_2.hpp"

long long Fibonacci_2::addFrom1ToN_Recursive(int n) {
    return n <= 0? 0: n + addFrom1ToN_Recursive(n - 1);
}

long long Fibonacci_2::addFrom1ToN_Iterative(int n) {
    int sum = 0;
    for (int i = 1; i <= n; ++i) {
        sum += i;
    }
    
    return sum;
}

long long Fibonacci_2::fibonacci_Solution1(unsigned int n) {
    if (n <= 0) {
        return 0;
    }
    
    if (n == 1) {
        return 1;
    }
    
    return fibonacci_Solution1(n - 1) + fibonacci_Solution1(n - 2);
}

long long Fibonacci_2::fibonacci_Solution2(unsigned int n) {
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
