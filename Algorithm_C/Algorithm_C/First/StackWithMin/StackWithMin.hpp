//
//  StackWithMin.hpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/17.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef StackWithMin_hpp
#define StackWithMin_hpp

// 包含 min 函数的栈
#include <stdio.h>
#include <stack>
#include <assert.h>

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
    std::stack<T> m_data; // 数据栈，存放栈的所有元素
    std::stack<T> m_min; // 辅助栈，存放栈的最小元素
};

#endif /* StackWithMin_hpp */
