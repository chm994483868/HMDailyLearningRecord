//
//  FirstNotRepeatingChar.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/24.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "FirstNotRepeatingChar.hpp"

char firstNotRepeatingchar(const char* pString) {
    if (pString == nullptr) {
        return '\0';
    }
    
    const int tableSize = 256;
    unsigned int hashTable[tableSize];
    for (unsigned int i = 0; i < tableSize; ++i) {
        hashTable[i] = 0;
    }
    
    const char* pHashKey = pString;
    // 1.遍历字符串, 在 hashTable 数组中记录每个字符出现的次数
    // 在 hashTable 中，下标对应每个字符，下标中的值表示该字符出现的次数
    while (*(pHashKey) != '\0') {
        ++hashTable[*(pHashKey++)];
    }
    
    // 2. 再次遍历字符串，根据 hashTable 中的记录，找到第一个出现次数只有 1 的字符
    pHashKey = pString;
    while (*pHashKey != '\0') {
        if (hashTable[*pHashKey] == 1) {
            return *pHashKey;
        }
        
        pHashKey++;
    }
    
    return '\0';
}

namespace FirstNotRepeatingChar_Review {

char firstNotRepeatingchar(const char* pString) {
    if (pString == nullptr) {
        return '\0';
    }
    
    const int tableSize = 256;
    unsigned int hashTable[tableSize];
    for (unsigned int i = 0; i < tableSize; ++i) {
        hashTable[i] = 0;
    }
    
    const char* pHashKey = pString;
    while (*pHashKey != '\0') {
        ++hashTable[*(pHashKey++)];
    }
    
    pHashKey = pString;
    while (*pHashKey != '\0') {
        if (hashTable[*pHashKey] == 1) {
            return *pHashKey;
        }
        
        ++pHashKey;
    }
    
    return '\0';
}

class CharStatistics {
public:
    CharStatistics() : index(0) {
        for (int i = 0; i < 256; ++i) {
            occurrence[i] = -1;
        }
    }
    
    void insert(char ch) {
        if (occurrence[ch] == -1) {
            occurrence[ch] = index;
        } else if(occurrence[ch] >= 0) {
            occurrence[ch] = -2;
        }
        
        ++index;
    }
    
    char firstAppearingOnce() {
        char ch = '\0';
        int minIndex = numeric_limits<int>::max();
        for (int i = 0; i < 256; ++i) {
            if ((occurrence[i] >= 0 && occurrence[i] < minIndex)) {
                ch = (char)i;
                minIndex = occurrence[i];
            }
        }
        
        return ch;
    }
    
private:
    // occurrence[i]: A character with ASCII value;
    // occurrence[i] = -1; The character has not found;
    // occurrence[i] = -2; The character has been found for mutlple times
    // occurrence[i] >= 0; The character has been found only once
    int occurrence[256];
    int index;
};

}

namespace FirstNotRepeatingChar_Review {
    // 相关题目:
    // 1. 定义一个函数，输入两个字符串，从第一字符串中删除在第二个字符串中出现过的所有字符。
    // 例如："We are Students." "aeiou" ==> W r Stdnts。
    // 方案: 可以创建一个用数组实现的简单哈希表来存储第二个字符串。这样从头到尾扫描第一个字符串的每个字符时，用 O(1) 时间就能判断出该字符是不是在第二个字符串中。如果第一个字符串的长度是 n，那么总的时间复杂度是 O(n).
    // 2. 定义一个函数，删除字符串中所有重复出现的字符。
    // 例如：输入 "google"，删除重复的字符之后的结果是 "gole"。
    // 可以创建一个用布尔型数组实现的简单的哈希表。数组中的元素的意义是其下标看作 ASCII 码后
    // 对应的字母在字符串中是否已经出现，先把数组中所有的元素都设为 false。
    // 以 "google" 为例，当扫描到第一个 g 时，g 的 ASCII 码是 103，那么我们把数组中下标为
    // 103 的元素设为 true。当扫描到第二个 g 时，发现 103 元素值是 true，就知道 g 在前面已经出现过。
    // 3. 变位词。当扫描到第一个字符串中的每个字符时，为哈希表对应的项的值增加 1。
    // 接下来扫描第二个字符串，当扫描到每个字符时，为哈希表对应的项的值减去 1，如果扫描完第二个字符串后，哈希表中所有的值都是 0，那么这两个字符串就互为变位词。
}
