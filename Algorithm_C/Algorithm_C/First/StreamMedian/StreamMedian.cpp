//
//  StreamMedian.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/22.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "StreamMedian.hpp"

template<typename T>
class DynamicArray {
public:
    void insert(T num) {
        // 偶数个
        if (((min.size() + max.size()) & 0x1) == 0) {
            if (max.size() > 0 && num < max[0]) {
                max.push_back(num);
                push_heap(max.begin(), max.end(), less<T>());
                
                num = max[0];
                
                pop_heap(max.begin(), max.end(), less<T>());
                max.pop_back();
            }
            
            min.push_back(num);
            push_heap(min.begin(), min.end(), greater<T>());
        } else {
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
        int size = min.size() + max.size();
        if (size == 0) {
            throw std::exception(); // No numbers are available
        }
        
        T median = 0;
        
        if ((size & 1) == 1) {
            median = min[0];
        } else {
            median = (min[0] + max[0]) / 2;
        }
        
        return median;
    }
    
private:
    vector<T> min;
    vector<T> max;
};
