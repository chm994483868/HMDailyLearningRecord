//
//  QueueWithMax.cpp
//  Algorithm_C
//
//  Created by HM C on 2020/7/25.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "QueueWithMax.hpp"

template <typename T>
class QueueWithMax {
public:
    QueueWithMax() : currentIndex(0) {}
    void push_back(T number){
        while (!maximums.empty() && number >= maximums.back().number) {
            maximums.pop_back();
        }
        
        InternalData internalData = { number, currentIndex };
        data.push_back(internalData);
        maximums.push_back(internalData);
        
        currentIndex++;
    }
    
    void pop_front() {
        if (maximums.empty()) {
            throw new exception(); // queue is empty
        }
        
        if (maximums.front().index == data.front().index) {
            maximums.pop_front();
        }
        
        data.pop_front();
    }
    
    T max() const {
        if (maximums.empty()) {
            throw new exception(); // queue is empty
        }
        
        return maximums.front().number;
    }
    
private:
    struct InternalData {
        T number;
        int index;
    };
    
    deque<InternalData> data;
    deque<InternalData> maximums;
    int currentIndex;
};
