//
//  NumberAppearingOnce.cpp
//  Algorithm_C
//
//  Created by HM C on 2020/7/25.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "NumberAppearingOnce.hpp"

int findNumberAppearingOnce(int numbers[], int length) {
    if (numbers == nullptr || length <= 0) {
        throw std::exception(); // Invalid input.
    }
    // 1. 首先准备一个长度为 32 的 int 数组
    int bitSum[32] = {0};
    
    // 2. 外层遍历数组中的每个元素
    for (int i = 0; i < length; ++i) {
        
        // 3. 内层循环统计一个数字中哪些位是 1，并把结果统计在步骤 1 中准备的数组中
        // 4. 这个 bitMask 每次向左边移动 1 位，用于统计 numbers[i] 中该位置是 1 还是 0
        int bitMask = 1;
        for (int j = 31; j >= 0; --j) {
            // 5. & 一下，看看该位置是 1 还是 0
            int bit = numbers[i] & bitMask;
            // 6. 如果不是 0，就表示该位置是 1，则在步骤 1 准备的数组中，对应的位置 + 1
            if (bit != 0) {
                bitSum[j] += 1;
            }
            // 7. 向左移动
            bitMask = bitMask << 1;
        }
    }
    
    // 8. 整理结果，如果该位置是 1 就加 1，然后左移 1 位，最后即是统计结果
    int result = 0;
    for (int i = 0; i < 32; ++i) {
        result = result << 1;
        result += bitSum[i] % 3;
    }
    
    return result;
}
