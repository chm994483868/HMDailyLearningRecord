//
//  StackWithTwoQueues_2.cpp
//  OfferReview
//
//  Created by CHM on 2021/2/21.
//  Copyright © 2021 CHM. All rights reserved.
//

#include "StackWithTwoQueues_2.hpp"

template <typename T>
StackWithTwoQueues_2::CStack<T>::CStack(void) {}

template <typename T>
StackWithTwoQueues_2::CStack<T>::~CStack(void) {}

template <typename T>
void StackWithTwoQueues_2::CStack<T>::appendHead(const T& node) {
    if (queue1.empty()) {
        queue2.push(node);
    } else if (queue2.empty()) {
        queue1.push(node);
    }
}

template <typename T>
T StackWithTwoQueues_2::CStack<T>::deleteHead() {
    if (queue1.empty() && queue2.empty()) {
        throw std::exception();
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
    
    throw std::exception();
}
