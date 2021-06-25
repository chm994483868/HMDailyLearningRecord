//
//  StreamMedian.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/31.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef StreamMedian_hpp
#define StreamMedian_hpp

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cstdio>
#include <algorithm>
#include <vector>
#include <functional>

using namespace std;

namespace StreamMedian {

// 41：数据流中的中位数
// 题目：如何得到一个数据流中的中位数？如果从数据流中读出奇数个数值，那么
// 中位数就是所有数值排序之后位于中间的数值。如果从数据流中读出偶数个数值，
// 那么中位数就是所有数值排序之后中间两个数的平均值。
template <typename T>
class DynamicArray {
public:
    // 要保证最大堆中的所有数据都要小于最小堆中的数据。
    // 当数据的总数目是偶数时，按照前面的分配规则会把新数据插入最小堆。
    
    // 如果此时这个新的数据比最大堆中的一些数据要小，那该怎么办呢？⬅️⬅️
    // 可以先把这个新的数据插入最大堆，接着把最大堆中最大的数字拿出来插入最小堆。
    // 由于最终插入最小堆的数字是原最大堆中的最大的数字，这样就保证了最小堆中所有数字都大于最大堆中的数字。
    
    void insert(T num) {
        // 判断当前总数目是奇数个还是偶数个
        if (((min.size() + max.size()) & 1) == 0) {
            // 当前是偶数个数据，把新数据插入最小堆
            
            // 如果最大堆有值，且新插入的值小于最大堆的最大值
            if (max.size() > 0 && num < max[0]) {
                // num 入堆
                max.push_back(num);
                // 调整堆化
                push_heap(max.begin(), max.end(), less<T>());
                
                // num 取出，最大堆的最大值
                num = max[0];
                
                // 把最大值出堆
                pop_heap(max.begin(), max.end(), less<T>());
                max.pop_back();
            }
            
            // 把 num 插入最小堆
            min.push_back(num);
            // 堆化
            push_heap(min.begin(), min.end(), greater<T>());
        } else {
            // 同上
            if (min.size() > 0 && min[0] < num) {
                min.push_back(num);
                push_heap(min.begin(), min.end(), greater<T>());
                
                num = min[0];
                
                pop_heap(min.begin(), min.end(), greater<T>());
                min.pop_back();
            }
            
            max.push_back(num);
            push_heap(max.begin(), max.end(), less<T>());
        }
    }
    
    T getMedian() {
        // 总数目个数
        int size = min.size() + max.size();
        
        if (size == 0) {
            throw exception(); // no numbers are available
        }
        
        T median = 0;
        if ((size & 1) == 1) {
            // 如果总数目是奇数，则中位数是最小堆或者最大堆的最值
            median = min[0];
        } else {
            // 如果总数目是偶数，就是最小堆和最大堆的最值的和的一半
            median = (min[0] + max[0]) / 2;
        }
        
        return median;
    }
    
private:
    // 数据在逻辑排列中，max 在左边 min 在右边
    // 根据准备的两个指针，一个指向左边的最大值，一个指向右边部分的最小值
    // 在数据的总数目是偶数时把新数据插入最小堆，否则插入最大堆。
    
    // min 和 max 的数据个数相差不能超过 1，
    // 即要么数目相等，要么相差 1 个元素
    
    // min 作为最小堆使用
    vector<T> min;
    // max 作为最大堆使用
    vector<T> max;
};

// 测试代码
void Test(char* testName, DynamicArray<double>& numbers, double expected);

void Test();

}

#endif /* StreamMedian_hpp */
