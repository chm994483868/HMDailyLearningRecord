//
//  StackWithMin.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/17.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "StackWithMin.hpp"

template <typename T> void StackWithMin<T>::push(const T& value) {
    // 把新元素添加到辅助栈
    m_data.push(value);
    
    // 当新元素比之前的最小元素小时，把新元素插入辅助栈里;
    // 否则把之前的最小元素重复插入辅助栈里
    if (m_min.size() == 0 || value < m_min.top())
        m_min.push(value);
    else
        m_min.push(m_min.top());
}

template <typename T> void StackWithMin<T>::pop() {
    assert(m_data.size() > 0 && m_min.size() > 0);
    
    m_data.pop();
    m_min.pop();
}

template <typename T> const T& StackWithMin<T>::min() const {
    assert(m_data.size() > 0 && m_min.size() > 0);
    return m_min.top();
}

template <typename T> T& StackWithMin<T>::top() {
    return m_data.top();
}

template <typename T> const T& StackWithMin<T>::top() const {
    return m_data.top();
}

template <typename T> bool StackWithMin<T>::empty() const {
    return m_data.empty();
}

template <typename T> size_t StackWithMin<T>::size() const {
    return m_data.size();
}
