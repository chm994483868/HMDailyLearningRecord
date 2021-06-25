//
//  StringToInt.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/11.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "StringToInt.hpp"

long long StringToInt::strToIntCore(const char* digit, bool minus) {
    long long num = 0;
    
    while (*digit != '\0') {
        if (*digit >= '0' && *digit <= '9') {
            
            // 正数或者负数
            int flag = minus ? -1: 1;
            num = num * 10 + flag * (*digit - '0');
            
            // 溢出时的情况
            if ((!minus && num > 0x7FFFFFFF) || (minus && num < (signed int)0x80000000)) {
                num = 0;
                break;
            }
            
            // 指针移动
            digit++;
        } else {
            // 如果有 0 ～ 9 之外的字符则直接 break
            num = 0;
            break;
        }
    }
    
    // digit 没有移动到空字符表示字符串中有非数字字符
    if (*digit == '\0') {
        g_nStatus = kValid;
    }
    
    return num;
}

int StringToInt::strToInt(const char* str) {
    // g_nStatus 标记 str 是否能转化为整数
    g_nStatus = kInvalid;
    
    long long num = 0;
    if (str != nullptr && *str != '\0') {
        
        // minus 表示是负数还是正数
        bool minus = false;
        
        // 前进一步，撇开前面的正号和负号
        if (*str == '+') {
            ++str;
        } else if (*str == '-') {
            ++str;
            
            // 表示当前数字是负数
            minus = true;
        }
        
        // *str 不是空字符，进行字符串到数字的转换
        if (*str != '\0') {
            num = strToIntCore(str, minus);
        }
    }
    
    return (int)num;
}
