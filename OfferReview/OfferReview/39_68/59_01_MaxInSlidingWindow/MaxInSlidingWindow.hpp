//
//  MaxInSlidingWindow.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/11.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef MaxInSlidingWindow_hpp
#define MaxInSlidingWindow_hpp

#include <stdio.h>
#include <cstdio>
#include <vector>
#include <deque>

using namespace std;

namespace MaxInSlidingWindow {

// 面试题59（一）：滑动窗口的最大值
// 题目：给定一个数组和滑动窗口的大小，请找出所有滑动窗口里的最大值。例如，
// 如果输入数组{2, 3, 4, 2, 6, 2, 5, 1}及滑动窗口的大小3，那么一共存在6个
// 滑动窗口，它们的最大值分别为{4, 4, 6, 6, 6, 5}。

vector<int> maxInWindows(const vector<int>& num, unsigned int size);

}
#endif /* MaxInSlidingWindow_hpp */
