//
//  TranslateNumbersToStrings.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/23.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "TranslateNumbersToStrings.hpp"

int getTranslationCount(const string& number);

int getTranslationCount(int number) {
    if (number < 0) {
        return 0;
    }
    
    string numberInString = to_string(number);
    return getTranslationCount(numberInString);
}

int getTranslationCount(const string& number) {
    unsigned long length = number.length();
    int* counts = new int[length];
    int count = 0;
    //                01234 ++ 12258
    // 假设：number = "12218" length = 5
    // length - 1 = 4
    // 第一轮: i = 4 counts[4] = 1  1
    // 第二轮: i = 3 counts[3] = 2  1
    // 第三轮: i = 2 counts[2] = 3  2
    // 第四轮: i = 1 counts[1] = 5  3
    // 第五轮: i = 0 counts[0] = 8  5
    for (unsigned long i = length - 1; i >= 0; --i) {
        count = 0;
        if (i < length - 1) {
            count = counts[i + 1];
        } else {
            count = 1;
        }
        
        if (i < length - 1) {
            
            int digit1 = number[i] - '0';
            int digit2 = number[i + 1] - '0';
            int converted = digit1 * 10 + digit2;
            
            if (converted >= 10 && converted <= 25) {
                if (i < length - 2) {
                    // f(i) = f(i+1) + g(i, i+1)*f(i+2)
                    count += counts[i + 2];
                } else {
                    count += 1;
                }
            }
        }
        
        counts[i] = count;
    }
    
    count = counts[0];
    delete [] counts;
    
    return count;
}

namespace TranslateNumbersToStrings_Review {

// f(i) = f(i + 1) + g(i, i + 1) * f(i + 2);
// 当第 i 位和第 i + 1 位两位数字拼接起来的数字在 10~25
// 的范围内时，函数 g(i, i + 1) 的值为 1，否则为 0。

int getTranslationCount(const string& number);
int getTranslationCount(int number) {
    if (number < 0) {
        return 0;
    }
    
    string numberInString = to_string(number);
    return getTranslationCount(numberInString);
}

int getTranslationCount(const string& number) {
    unsigned long length = number.length();
    int* counts = new int[length];
    int count = 0;
    
    for (unsigned long i = length - 1; i >= 0; --i) {
        count = 0;
        
        if (i < length - 1) {
            count = counts[i + 1];
        } else {
            count = 1;
        }
        
        if (i < length - 1) {
            int digit1 = number[i] - '0';
            int digit2 = number[i + 1] - '0';
            int converted = digit1 * 10 + digit2;
            if (converted >= 10 && converted <= 25) {
                if (i < length - 2) {
                    count += counts[i + 2];
                } else {
                    count += 1;
                }
            }
        }
        
        counts[i] = count;
    }
    
    count = counts[0];
    delete [] counts;
    
    return count;
}

}
