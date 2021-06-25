//
//  LastNumberInCircle.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/26.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "LastNumberInCircle.hpp"

int lastRemaining_Solution1(unsigned int n, unsigned int m) {
    // 1. 参数判断
    if (n < 1 || m < 1) {
        return -1;
    }
    
    unsigned int i = 0;
    // 2. 准备 list
    list<int> numbers;
    // 3. 把数据放入 numbers 里面
    for (i = 0; i < n; ++i) {
        numbers.push_back(i);
    }
    // 4. 拿到 numbers 的迭代器
    list<int>::iterator current = numbers.begin();
    // 5. 循环到 numbers 里面只剩下最后一个元素
    while (numbers.size() > 1) {
        // 6. 迭代器向前移动，指到待删除的元素
        for (int i = 1; i < m; ++i) {
            ++current;
            if (current == numbers.end()) {
                current = numbers.begin();
            }
        }
        
        // 7. 找到待删除元素后面的一个元素, 由于这里 current 做了一次自增操作
        // 所以对应下面的要做一次 自减操作
        list<int>::iterator next = ++current;
        // 8. 如果到了 end 则再指到 begin
        if (next == numbers.end()) {
            next = numbers.begin();
        }
        
        // 8. 自减，移除当前元素，更新 current 为 next，然后开启下轮循环
        --current;
        numbers.erase(current);
        current = next;
    }
    
    return *(current);
}

int lastRemaining_Solution2(unsigned int n, unsigned int m) {
    if (n < 1 || m < 1) {
        return -1;
    }
    
    int last = 0;
    for (int i = 2; i <= n; ++i) {
        last = (last + m) % i;
    }
    
    return last;
}
