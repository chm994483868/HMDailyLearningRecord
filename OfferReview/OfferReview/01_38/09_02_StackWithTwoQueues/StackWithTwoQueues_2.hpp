//
//  StackWithTwoQueues_2.hpp
//  OfferReview
//
//  Created by CHM on 2021/2/21.
//  Copyright © 2021 CHM. All rights reserved.
//

#ifndef StackWithTwoQueues_2_hpp
#define StackWithTwoQueues_2_hpp

#include <stdio.h>
#include <queue>

namespace StackWithTwoQueues_2 {

template <typename T>
class CStack {
public:
    CStack(void);
    ~CStack(void);
    
    void appendHead(const T& node);
    T deleteHead();
    
private:
    std::queue<T> queue1;
    std::queue<T> queue2;
};

}

#endif /* StackWithTwoQueues_2_hpp */
