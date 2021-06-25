//
//  AddTwoNumbers.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/26.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "AddTwoNumbers.hpp"

int add(int num1, int num2) {
    int sum, carry;
    do {
        sum = num1 ^ num2; // num1 和 num2 按位进行异或，只有二者对应是 1 或 0， 0 或者 1 的位保留了下来，其它位都变成了 0，即使都是 1 的，也变成 0，可以理解为二者进行了一个没有进位的加法运算
        carry = (num1 & num2) << 1; // num1 和 num2 进行按位与运行，只有二者对应位都是 1 的时候，才会得到 1， 然后进行左移操作，表示进位，如果不存在二者对应位都是 1 的情况，那么 carry 等于 0，表示二者相加没有进位操作，可以直接结束循环了
        num1 = sum;
        num2 = carry;
    } while (num2 != 0);
    // 上面的手法好巧妙🦷🦷🦷
    
    return num1;
}
