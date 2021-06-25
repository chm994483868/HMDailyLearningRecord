//
//  QueueWithTwoStacks_2.cpp
//  OfferReview
//
//  Created by CHM on 2021/2/21.
//  Copyright © 2021 CHM. All rights reserved.
//

#include "QueueWithTwoStacks_2.hpp"

template <typename T>
QueueWithTwoStacks_2::CQueue<T>::CQueue(void) { }

template <typename T>
QueueWithTwoStacks_2::CQueue<T>::~CQueue(void) { }

template <typename T>
void QueueWithTwoStacks_2::CQueue<T>::appendTail(const T& node) {
    stack1.push(node);
}

template <typename T>
T QueueWithTwoStacks_2::CQueue<T>::deleteHead() {
    if (stack2.empty()) {
        while (!stack1.empty()) {
            T& data = stack1.top();
            stack1.pop();
            stack2.push(data);
        }
    }
    
    if (stack2.empty()) {
        throw std::exception();
    }
    
    T head = stack2.top();
    stack2.pop();
    
    return head;
}
