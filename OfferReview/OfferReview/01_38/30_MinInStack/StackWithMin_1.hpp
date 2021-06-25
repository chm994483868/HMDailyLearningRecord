//
//  StackWithMin_1.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/5.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef StackWithMin_1_hpp
#define StackWithMin_1_hpp

#include <stdio.h>
#include <stack>
#include <assert.h>

using namespace std;

namespace StackWithMin_1 {

// 30：包含min函数的栈
// 题目：定义栈的数据结构，请在该类型中实现一个能够得到栈的最小元素的 min
// 函数。在该栈中，调用 min、push 及 pop 的时间复杂度都是 O(1)。

template <typename T>
class StackWithMin {
public:
    StackWithMin() {}
    virtual ~StackWithMin() {}
    
    T& top();
    const T& top() const;
    
    void push(const T& value);
    void pop();
    
    const T& min() const;
    
    bool empty() const;
    size_t size() const;
    
private:
    stack<T> m_data;
    stack<T> m_min;
};

// 测试代码
void Test(const char* testName, const StackWithMin<int>& stack, int expected);

void Test();

}

#endif /* StackWithMin_1_hpp */
