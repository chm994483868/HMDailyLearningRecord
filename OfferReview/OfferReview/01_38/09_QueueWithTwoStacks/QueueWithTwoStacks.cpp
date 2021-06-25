//
//  QueueWithTwoStacks.cpp
//  OfferReview
//
//  Created by CHM on 2020/7/28.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "QueueWithTwoStacks.hpp"

template <typename T>
QueueWithTwoStacks::CQueue<T>::CQueue(void) { }

template <typename T>
QueueWithTwoStacks::CQueue<T>::~CQueue(void) { }

template <typename T>
void QueueWithTwoStacks::CQueue<T>::appendTail(const T& node) {
    stack1.push(node);
}

template <typename T>
T QueueWithTwoStacks::CQueue<T>::deleteHead() {
    if (stack2.empty()) {
        while (!stack1.empty()) {
            T& data = stack1.top();
            stack1.pop();
            stack2.push(data);
        }
    }
    
    if (stack2.empty()) {
        throw std::exception(); // 栈当前为空，无法弹出数据
    }
    
    T head = stack2.top();
    stack2.pop();
    
    return head;
}

// 测试代码
void QueueWithTwoStacks::Test(char actual, char expected) {
    if(actual == expected)
        printf("Test passed.\n");
    else
        printf("Test failed.\n");
}

void QueueWithTwoStacks::Test() {
    CQueue<char> queue;
    
    queue.appendTail('a');
    queue.appendTail('b');
    queue.appendTail('c');
    
    char head = queue.deleteHead();
    Test(head, 'a');
    
    head = queue.deleteHead();
    Test(head, 'b');
    
    queue.appendTail('d');
    head = queue.deleteHead();
    Test(head, 'c');
    
    queue.appendTail('e');
    head = queue.deleteHead();
    Test(head, 'd');
    
    head = queue.deleteHead();
    Test(head, 'e');
}
