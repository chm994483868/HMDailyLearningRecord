//
//  NumberOf1InBinary.cpp
//  OfferReview
//
//  Created by CHM on 2020/7/28.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "NumberOf1InBinary.hpp"

int NumberOf1InBinary::numberOf1_Solution1(int n) {
    int count = 0;
    unsigned int flag = 1;
    // 这里 flag 要向右移动 32 位才能结束 while 循环
    while (flag) {
        if (n & flag) {
            ++count;
        }
        
        flag = flag << 1;
    }
    
    return count;
}

int NumberOf1InBinary::numberOf1_Solution2(int n) {
    int count = 0;
    // 这里很巧妙，while 循环的次数就是 1 的个数
    while (n) {
        ++count;
        n = (n - 1) & n; // 减 1 后再做 与 操作，后面的 1 全部被去掉了...
    }
    
    return count;
}

// 测试代码
void NumberOf1InBinary::Test(int number, unsigned int expected) {
    int actual = numberOf1_Solution1(number);
    if (actual == expected)
        printf("Solution1: actual = %d Test for %d passed.\n", actual, number);
    else
        printf("Solution1: actual = %d Test for %d failed.\n", actual, number);

    actual = numberOf1_Solution2(number);
    if (actual == expected)
        printf("Solution2: actual = %d Test for %d passed.\n", actual, number);
    else
        printf("Solution2: actual = %d Test for %d failed.\n", actual, number);

    printf("\n");
}

void NumberOf1InBinary::Test() {
    // 输入0，期待的输出是0
    Test(0, 0);

    // 输入1，期待的输出是1
    Test(1, 1);

    // 输入10，期待的输出是2
    Test(10, 2); // 1010
    
    // 输入 -8，
    Test(8, 1);
    Test(0x00000008, 1);
    Test(-8, 29);
    Test(0xFFFFFFF8, 29);

    // 输入0x7FFFFFFF，期待的输出是31
    Test(0x7FFFFFFF, 31);

    // 输入0xFFFFFFFF（负数），期待的输出是32
    Test(0xFFFFFFFF, 32);

    // 输入0x80000000（负数），期待的输出是1
    Test(0x80000000, 1);
}

//class Solution {
//public:
//    int hammingWeight(uint32_t n) {
//        int count = 0;
//        while (n) {
//            n = (n - 1) & n;
//            ++count;
//        }
//        return count;
//    }
//};

