//
//  Accumulate.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/26.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "Accumulate.hpp"

// 解法一: 利用构造函数求解
namespace Accumulate_TEMP {

class Temp {
public:
    Temp() {
        ++N;
        Sum += N;
    }
    
    static void Reset() {
        N = 0;
        Sum = 0;
    }
    
    static unsigned int GetSum() {
        return Sum;
    }
    
private:
    static unsigned int N;
    static unsigned int Sum;
};

unsigned int Temp::N = 0;
unsigned int Temp::Sum = 0;

unsigned int sum_Solution1(unsigned int n) {
    Temp::Reset();
    
    // 这里 Temp 的构造函数执行了 n 次
    // Temp 类中 静态变量 Sum 记录了 1 + 2 + 3 +...+ n 的和
    Temp* a = new Temp[n];
    delete [] a;
    a = NULL;
    
    return Temp::GetSum();
}

}

// 解法二: 利用虚函数求解
namespace Accumulate_A {
    
class A;
A* Array[2];

class A {
public:
    virtual unsigned int Sum(unsigned int n) {
        return 0;
    }
};

class B: public A {
public:
    virtual unsigned int Sum(unsigned int n) {
        return Array[!!n]->Sum(n - 1) + n;
    }
};

int Sum_Solution2(int n) {
    A a;
    B b;
    Array[0] = &a;
    Array[1] = &b;
    
    int value = Array[1]->Sum(n);
    
    return value;
}

// 解法三: 利用函数指针求解
typedef unsigned int (*fun)(unsigned int);

unsigned int Solutiona3_Teminator(unsigned int n) {
    return 0;
}

unsigned int Sum_Solution3(unsigned int n) {
    static fun f[2] = {Solutiona3_Teminator, Sum_Solution3};
    return n + f[!!n](n - 1);
}

}

// 解法四: 利用模版类型求解
namespace Accumulate_Template {

template <unsigned int n> struct Sum_Solution4 {
    enum Value { N = Sum_Solution4<n - 1>::N + n };
};

template <> struct Sum_Solution4<1> {
    enum Value { N = 1 };
};

template <> struct Sum_Solution4<0> {
    enum Value { N = 0 };
};

}
