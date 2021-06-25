//
//  QueueWithMax.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/11.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef QueueWithMax_hpp
#define QueueWithMax_hpp

#include <stdio.h>
#include <cstdio>
#include <deque>
#include <exception>

using namespace std;

namespace QueueWithMax {

// 面试题59（二）：队列的最大值
// 题目：给定一个数组和滑动窗口的大小，请找出所有滑动窗口里的最大值。例如，
// 如果输入数组 {2, 3, 4, 2, 6, 2, 5, 1} 及滑动窗口的大小 3，那么一共存在 6 个
// 滑动窗口，它们的最大值分别为 {4, 4, 6, 6, 6, 5}。

template<typename T>
class QueueWithMax {
public:
    QueueWithMax() : currentIndex(0) { }
    
    void push_back(T number) {
        // 如果新来的数据大于 maximums 尾部的数据，表示新的数据才会成为滑动窗口的最大值
        while (!maximums.empty() && number >= maximums.back().number) {
            maximums.pop_back();
        }
        
        // 构建 InternalData 局部变量
        InternalData internalData = { number, currentIndex };
        
        // 入队
        data.push_back(internalData);
        maximums.push_back(internalData);
        
        // 自增
        ++currentIndex;
    }
    
    void pop_front() {
        if (maximums.empty()) {
            throw new exception();
        }
        
        // 出队
        if (maximums.front().index == data.front().index) {
            maximums.pop_front();
        }
        
        // 出队
        data.pop_front();
    }
    
    T max() const {
        if (maximums.empty()) {
            throw new exception();
        }
        
        // maximums 队列头部记录的是滑动窗口的最大值
        return maximums.front().number;
    }

private:
    // InternalData 用来记录数据和其下标
    struct InternalData {
        T number;
        int index;
    };
    
    // data 保存原始数据
    deque<InternalData> data;
    // maximums 保存滑动窗口最大值
    deque<InternalData> maximums;
    // currentIndex 记录当前是第几个数字
    int currentIndex;
};

}

#endif /* QueueWithMax_hpp */
