//
//  LongestSubstringWithoutDup.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/23.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "LongestSubstringWithoutDup.hpp"

int longestSubstringWithoutDuplication_2(const std::string& str) {
    int curLength = 0;
    int maxLength = 0;
    
    int* position = new int[26];
    int i = 0;
    for (; i < 26; ++i) {
        position[i] = -1;
    }
    
    for (i = 0; i < str.length(); ++i) {
        int prevIndex = position[str[i] - 'a'];
        if (prevIndex < 0 || i - prevIndex > curLength) {
            ++curLength;
        } else {
            if (curLength > maxLength) {
                maxLength = curLength;
            }
            
            curLength = i - prevIndex;
        }
        
        position[str[i] - 'a'] = i;
    }
    
    if (curLength > maxLength) {
        maxLength = curLength;
    }
    
    delete [] position;
    return maxLength;
}

namespace LongestSubstringWithoutDup_Review {

int longestSubstringWithoutDuplication_2(const std::string& str) {
    int curLength = 0;
    int maxLength = 0;
    
    int* position = new int[26];
    int i = 0;
    for (; i < 26; ++i) {
        position[i] = -1;
    }
    
    for (i = 0; i < str.length(); ++i) {
        int prevIndex = position[str[i] - 'a'];
        if (prevIndex < 0 || i - prevIndex > curLength) {
            ++curLength;
        } else {
            if (curLength > maxLength) {
                maxLength = curLength;
            }
            
            curLength = i - prevIndex;
        }
        
        position[str[i] - 'a'] = i;
    }
    
    if (curLength > maxLength) {
        maxLength = curLength;
    }
    
    return maxLength;
}

}
