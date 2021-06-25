//
//  MaxInSlidingWindow.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/11.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "MaxInSlidingWindow.hpp"

vector<int> MaxInSlidingWindow::maxInWindows(const vector<int>& num, unsigned int size) {
    
    // 滑动窗口
    vector<int> maxInWindows;
    
    if (num.size() >= size && size >= 1) {
        
        // 两端开口的队列 index，用来记录 num 中可能作为最大值的各元素的下标
        deque<int> index;
        for (unsigned int i = 0; i < size; ++i) {
            // num[i] 大于 num[index.back()] 表示前面的数字都不可能成为滑动窗口的最大值，把它们移除
            while (!index.empty() && num[i] >= num[index.back()]) {
                index.pop_back();
            }
            
            // 入队
            index.push_back(i);
        }
        
        for (unsigned int i = size; i < num.size(); ++i) {
            
            // 滑动窗口的最大值每次都是位于 index.front() 头部
            maxInWindows.push_back(num[index.front()]);
            
            // 去尾
            while (!index.empty() && num[i] >= num[index.back()]) {
                index.pop_back();
            }
            
            // 去头
            
            // index.front() <= (int)(i - size)
            // 表示队列头部的数字下标 和 i 的距离大于等于滑动窗口的大小，
            // 该位置的数字已经滑出滑动窗口了，需要把它出队
            if (!index.empty() && index.front() <= (int)(i - size)) {
                index.pop_front();
            }
            
            // 入队
            index.push_back(i);
        }
        
        maxInWindows.push_back(num[index.front()]);
    }
    
    return maxInWindows;
}
