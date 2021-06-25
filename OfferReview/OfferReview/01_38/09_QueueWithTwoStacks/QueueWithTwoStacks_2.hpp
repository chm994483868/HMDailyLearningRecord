//
//  QueueWithTwoStacks_2.hpp
//  OfferReview
//
//  Created by CHM on 2021/2/21.
//  Copyright © 2021 CHM. All rights reserved.
//

#ifndef QueueWithTwoStacks_2_hpp
#define QueueWithTwoStacks_2_hpp

#include <stdio.h>
#include <stack>

namespace QueueWithTwoStacks_2 {

template <typename T>
class CQueue {
public:
    CQueue(void);
    ~CQueue(void);
    
    void appendTail(const T& node);
    T deleteHead();
    
private:
    std::stack<T> stack1;
    std::stack<T> stack2;
};

}

#endif /* QueueWithTwoStacks_2_hpp */
