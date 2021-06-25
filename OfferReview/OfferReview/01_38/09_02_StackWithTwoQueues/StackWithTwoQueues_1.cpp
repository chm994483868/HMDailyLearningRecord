//
//  StackWithTwoQueues_1.cpp
//  OfferReview
//
//  Created by HM C on 2020/11/1.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "StackWithTwoQueues_1.hpp"

template <typename T>
StackWithTwoQueues_1::CStack<T>::CStack(void) { }

template <typename T>
StackWithTwoQueues_1::CStack<T>::~CStack(void) { }

template <typename T>
void StackWithTwoQueues_1::CStack<T>::appendHead(const T& node) {
    if (queue1.empty()) {
        queue2.push(node);
    } else if (queue2.empty()) {
        queue1.push(node);
    }
}

template <typename T>
T StackWithTwoQueues_1::CStack<T>::deleteHead() {
    if (queue1.empty() && queue2.empty()) {
        throw std::exception(); // 当前栈为空
    }
    
    if (queue1.empty()) {
        while (queue2.size() > 1) {
            T& node = queue2.front();
            queue2.pop();
            
            queue1.push(node);
        }
        
        T node = queue2.front();
        queue2.pop();
        return node;
    } else if (queue2.empty()) {
        while (queue1.size() > 1) {
            T& node = queue1.front();
            queue1.pop();
            
            queue2.push(node);
        }
        
        T node = queue1.front();
        queue1.pop();
        return node;
    }
    
    throw std::exception(); // 当前栈为空
}

// 测试代码
void StackWithTwoQueues_1::Test(char actual, char expected) {
    if(actual == expected)
        printf("Test passed.\n");
    else
        printf("Test failed.\n");
}

void StackWithTwoQueues_1::Test() {
    CStack<char> stack;
    
    stack.appendHead('a');
    stack.appendHead('b');
    stack.appendHead('c');
    
    char head = stack.deleteHead();
    Test(head, 'c');
    
    head = stack.deleteHead();
    Test(head, 'b');
    
    stack.appendHead('d');
    head = stack.deleteHead();
    Test(head, 'd');
    
    stack.appendHead('e');
    head = stack.deleteHead();
    Test(head, 'e');
    
    head = stack.deleteHead();
    Test(head, 'a');
}
