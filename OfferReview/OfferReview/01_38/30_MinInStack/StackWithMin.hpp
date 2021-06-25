//
//  StackWithMin.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/30.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef StackWithMin_hpp
#define StackWithMin_hpp

#include <stdio.h>
#include <stack>
#include <assert.h>

using namespace std;

namespace StackWithMin {

// 30：包含min函数的栈
// 题目：定义栈的数据结构，请在该类型中实现一个能够得到栈的最小元素的min
// 函数。在该栈中，调用min、push及pop的时间复杂度都是O(1)。

// 准备一个辅助栈 m_min 保存栈中的当前的最小值
template <typename T>
class StackWithMin {
public:
    StackWithMin() {}
    virtual ~StackWithMin() {}
    
    // 栈顶
    T& top();
    const T& top() const;
    
    // 入栈
    void push(const T& value);
    // 出栈
    void pop();
    
    // 最小值函数
    const T& min() const;
    
    // 是否为空
    bool empty() const;
    // 栈中元素个数
    size_t size() const;
private:
    // 存放数据栈
    stack<T> m_data;
    // 存放当前最小值的栈
    stack<T> m_min;
};

// 测试代码
void Test(const char* testName, const StackWithMin<int>& stack, int expected);

void Test();

}

#endif /* StackWithMin_hpp */
