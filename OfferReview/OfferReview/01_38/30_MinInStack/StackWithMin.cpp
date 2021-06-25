//
//  StackWithMin.cpp
//  OfferReview
//
//  Created by CHM on 2020/7/30.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "StackWithMin.hpp"

template <typename T>
T& StackWithMin::StackWithMin<T>::top() {
    return m_data.top();
}

template <typename T>
const T& StackWithMin::StackWithMin<T>::top() const {
    return m_data.top();
}

// 每次入栈，都比较 value 和当前 m_min 的栈顶元素，如果 value 更小则把 value 放入 m_min 中，
// 如果当前 m_min 栈顶元素更小，则把 m_min 的栈顶元素再次入栈
template <typename T>
void StackWithMin::StackWithMin<T>::push(const T& value) {
    // 数据正常入栈 m_data
    m_data.push(value);
    
    // 如果是第一个元素，或者 value 值更小，则入栈 m_min
    if (m_min.empty() || value < m_min.top()) {
        m_min.push(value);
    } else {
        // 如果 m_min 栈顶元素更小，则再次入栈这个栈顶元素
        m_min.push(m_min.top());
    }
}

template <typename T>
void StackWithMin::StackWithMin<T>::pop() {
    assert(m_data.size() > 0 && m_min.size() > 0);
    
    // 出栈时，双方都进行出栈
    m_data.pop();
    m_min.pop();
}

template <typename T>
const T& StackWithMin::StackWithMin<T>::min() const {
    assert(m_data.size() > 0 && m_min.size() > 0);
    
    // m_min 的栈顶元素一直保持的是当前 m_data 中的最小值
    return m_min.top();
}

template <typename T>
bool StackWithMin::StackWithMin<T>::empty() const {
    return m_data.empty();
}

template <typename T>
size_t StackWithMin::StackWithMin<T>::size() const {
    return m_data.size();
}

// 测试代码
void StackWithMin::Test(const char* testName, const StackWithMin<int>& stack, int expected) {
    if(testName != nullptr)
        printf("%s begins: ", testName);

    if(stack.min() == expected)
        printf("Passed.\n");
    else
        printf("Failed.\n");
}

void StackWithMin::Test() {
    StackWithMin<int> stack;

    stack.push(3);
    Test("Test1", stack, 3);

    stack.push(4);
    Test("Test2", stack, 3);

    stack.push(2);
    Test("Test3", stack, 2);

    stack.push(3);
    Test("Test4", stack, 2);

    stack.pop();
    Test("Test5", stack, 2);

    stack.pop();
    Test("Test6", stack, 3);

    stack.pop();
    Test("Test7", stack, 3);

    stack.push(0);
    Test("Test8", stack, 0);
}
