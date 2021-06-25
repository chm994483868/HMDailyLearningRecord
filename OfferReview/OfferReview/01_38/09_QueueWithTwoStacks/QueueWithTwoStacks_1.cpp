//
//  QueueWithTwoStacks_1.cpp
//  OfferReview
//
//  Created by HM C on 2020/11/1.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "QueueWithTwoStacks_1.hpp"

template <typename T>
QueueWithTwoStacks_1::CQueue<T>::CQueue(void) { }

template <typename T>
QueueWithTwoStacks_1::CQueue<T>::~CQueue(void) { }

template <typename T>
void QueueWithTwoStacks_1::CQueue<T>::appendTail(const T& node) {
    stack1.push(node);
}

template <typename T>
T QueueWithTwoStacks_1::CQueue<T>::deleteHead() {
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
    
    T head =stack2.top();
    stack2.pop();
    
    return head;
}

// 测试代码
void QueueWithTwoStacks_1::Test(char actual, char expected) {
    if(actual == expected)
        printf("Test passed.\n");
    else
        printf("Test failed.\n");
}

void QueueWithTwoStacks_1::Test() {
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
