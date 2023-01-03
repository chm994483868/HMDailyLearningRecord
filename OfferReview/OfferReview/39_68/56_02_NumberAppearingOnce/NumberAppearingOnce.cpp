//
//  NumberAppearingOnce.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "NumberAppearingOnce.hpp"

int NumberAppearingOnce::findNumberAppearingOnce(int numbers[], int length) {
    // 入参判断
    if (numbers == nullptr || length <= 0) {
        throw new std::exception();
    }
    
    // 准备一个长度是 32 的 int 数组，把数组中每个数字的二进制位加起来，
    // 因为其它数字都出现了 3 次，所以相加后，每个二进制位是 3 的倍数，或者不是 3 的倍数，
    // 如果某位是 3 的倍数，表示数组中只出现一次的数字的该位是 0，否则是 1
    int bitSum[32] = {0};
    
    for (int i = 0; i < length; ++i) {
        
        // 把每个数字的二进制表示对应的每位相加
        int bitMask = 1;
        for (int j = 31; j >= 0; --j) {
            int bit = numbers[i] & bitMask;
            
            // 加 1
            if (bit != 0) {
                bitSum[j] += 1;
            }
            
            // 依次统计每位
            bitMask = bitMask << 1;
        }
    }
    
    // 统计每个是 1 的位
    int result = 0;
    for (int i = 0; i < 32; ++i) {
        result = result << 1;
        result += bitSum[i] % 3;
    }
    
    return result;
}
