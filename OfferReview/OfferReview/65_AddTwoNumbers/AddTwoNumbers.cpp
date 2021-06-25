//
//  AddTwoNumbers.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/11.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "AddTwoNumbers.hpp"

// 用位运算进行加法操作
int AddTwoNumbers::add(int num1, int num2) {
    int sum, carry;
    do {
        sum = num1 ^ num2; // 异或结果可表示二进制表示逐位相加，但是不做进位
        carry = (num1 & num2) << 1; // carry 则表示要进行进位的位
        
        // 第三步，如果 carray 是 0 的话，不需要进行进位，即 sum 即是 num1 + num2 的结果，
        // 但是如果 carray 不是 0，则要进行 sum 和 carry 相加求和才能最后得 num1 + num2 的结果
        
        // while 条件正是判断是否发生了进位，如果发生了进位则持续进行以位操作所代替的加法操作
        num1 = sum;
        num2 = carry;
    } while (num2 != 0);
    
    return num1;
}
