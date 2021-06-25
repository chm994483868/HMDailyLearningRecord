//
//  QueueWithTwoStacks.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/28.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef QueueWithTwoStacks_hpp
#define QueueWithTwoStacks_hpp

#include <stdio.h>
#include <stack>

namespace QueueWithTwoStacks {
                  
// 9：用两个栈实现队列
// 题目：用两个栈实现一个队列。队列的声明如下，请实现它的两个函数appendTail
// 和deleteHead，分别完成在队列尾部插入结点和在队列头部删除结点的功能。

template <typename T>
class CQueue {
public:
    CQueue(void);
    ~CQueue(void);
    
    // 在队列末尾添加一个节点
    void appendTail(const T& node);
    // 删除队列的头节点
    T deleteHead();
    
private:
    std::stack<T> stack1;
    std::stack<T> stack2;
};

// 测试代码
void Test(char actual, char expected);

void Test();

}

#endif /* QueueWithTwoStacks_hpp */
