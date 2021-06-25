//
//  MaxInSlidingWindow.cpp
//  Algorithm_C
//
//  Created by HM C on 2020/7/25.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "MaxInSlidingWindow.hpp"

vector<int> maxInWindows(const vector<int>& num, unsigned int size) {
    // 1. 用来存放 max 的 vector
    vector<int> maxInWindows;
    if (num.size() >= size && size >= 1) {
        // 2. 用来存放当前大值下标的 deque
        deque<int> index;
        // 3. 首先找到第一个滑动窗口的最大值，并把它的下标放在 index 里面
        for (unsigned int i = 0; i < size; ++i) {
            // 4. index 里面只存放当前最大值的下标，或者可能成为最大值的元素的下标
            while (!index.empty() && num[i] >= num[index.back()]) {
                index.pop_back();
            }
            
            index.push_back(i);
        }
        
        // 4. 遍历后续的滑动窗口
        // i 是从 size 开始的
        for (unsigned int i = size; i < num.size(); ++i) {
            // 5. 把最大值放进 maxInWindows 里面
            maxInWindows.push_back(num[index.front()]);
            
            // 6. 同第 4 步，判空，然后比较大小，如果当前元素大于等于前面的元素，则把 index 尾部的数据 pop 出去
            while (!index.empty() && num[i] >= num[index.back()]) {
                index.pop_back();
            }
            
            // 7. 前面的元素要从 index pop 出去，保证比较的数据是同一个滑动窗口的
            if (!index.empty() && index.front() <= (int)(i - size)) {
                index.pop_front();
            }
            
            index.push_back(i);
        }
        
        // 8. 把最后一个滑动窗口的最大值放在 maxInWindows 里面
        // 因为前面的每个 index.push_back(i) 都是在最后面执行的
        maxInWindows.push_back(num[index.front()]);
    }
    
    return maxInWindows;
}
