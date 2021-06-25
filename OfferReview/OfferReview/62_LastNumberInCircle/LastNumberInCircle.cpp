//
//  LastNumberInCircle.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/11.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "LastNumberInCircle.hpp"

// 使用链表表示圆圈
int LastNumberInCircle::lastRemaining_Solution1(unsigned int n, unsigned int m) {
    // 入参判断
    if (n < 1 || m < 1) {
        return -1;
    }
    
    // 把 [0, n - 1] 这 n 个数加入链表中
    unsigned int i = 0;
    list<int> numbers;
    for (i = 0; i < n; ++i) {
        numbers.push_back(i);
    }
    
    //迭代器
    list<int>::iterator current = numbers.begin();
    
    // while 循环，直到 numbers 剩最后一个节点
    while (numbers.size() > 1) {
        
        for (int i = 1; i < m; ++i) {
            ++current;
            
            // 当遍历到 numbers 结尾后，调转迭代器到链表头部继续遍历
            if (current == numbers.end()) {
                current = numbers.begin();
            }
        }
        
        // next 记录下一个节点
        list<int>::iterator next = ++current;
        
        // 判断 next 是否是链表结尾，如果是则需要转到链表头部
        if (next == numbers.end()) {
            next = numbers.begin();
        }
        
        // current 自减一步，（对应 next 的 ++）
        --current;
        
        // current 擦除
        numbers.erase(current);
        
        // 更新 current
        current = next;
    }
    
    // 返回 current
    return *(current);
}

// 数学的魅力
int LastNumberInCircle::lastRemaining_Solution2(unsigned int n, unsigned int m) {
    if (n < 1 || m < 1) {
        return -1;
    }
    
    int last = 0;
    for (int i = 2; i <= n; ++i) {
        last = (last + m) % i;
    }
    
    return last;
}
