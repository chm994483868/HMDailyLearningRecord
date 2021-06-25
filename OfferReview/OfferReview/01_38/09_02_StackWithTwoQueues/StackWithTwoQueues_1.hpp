//
//  StackWithTwoQueues_1.hpp
//  OfferReview
//
//  Created by HM C on 2020/11/1.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef StackWithTwoQueues_1_hpp
#define StackWithTwoQueues_1_hpp

#include <stdio.h>
#include <queue>

namespace StackWithTwoQueues_1 {
    
// 9.(二)：用两个队列实现栈
// 题目：用两个队列实现一个栈。栈的声明如下，请实现它的两个函数appendHead
// 和deleteHead，分别完成在栈顶部插入结点和在栈头部删除结点的功能。

template <typename T>
class CStack {
public:
    CStack(void);
    ~CStack(void);
    // 在栈顶部添加一个节点
    void appendHead(const T& node);
    // 删除栈顶部节点
    T deleteHead();
    
private:
    std::queue<T> queue1;
    std::queue<T> queue2;
};

// 测试代码
void Test(char actual, char expected);

void Test();

}

#endif /* StackWithTwoQueues_1_hpp */
