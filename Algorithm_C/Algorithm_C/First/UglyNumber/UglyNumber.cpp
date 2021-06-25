//
//  UglyNumber.cpp
//  Algorithm_C
//
//  Created by HM C on 2020/7/23.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "UglyNumber.hpp"

bool isUgly(int number) {
    while (number % 2 == 0) {
        number /= 2;
    }
    
    while (number % 3 == 0) {
        number /= 3;
    }
    
    while (number % 5 == 0) {
        number /= 5;
    }
    
    return (number == 1) ? true : false;
}

int getUglyNumber_Solution1(int index) {
    if (index <= 0) {
        return 0;
    }
    
    int number = 0;
    int uglyFound = 0;
    while (uglyFound < index) {
        ++number;
        
        if (isUgly(number)) {
            ++uglyFound;
        }
    }
    
    return number;
}

int min(int number1, int number2, int number3);

int getUglyNumber_Solution2(int index) {
    if (index <= 0) {
        return 0;
    }
    
    int *pUglyNumbers = new int[index];
    pUglyNumbers[0] = 1;
    int nextUglyIndex = 1;
    
    int* pMultiply2 = pUglyNumbers;
    int* pMultiply3 = pUglyNumbers;
    int* pMultiply5 = pUglyNumbers;
    
    while (nextUglyIndex < index) {
        int min_mark = min(*pMultiply2 * 2, *pMultiply3 * 3, *pMultiply5 * 5);
        pUglyNumbers[nextUglyIndex] = min_mark;
        
        while (*pMultiply2 * 2 <= pUglyNumbers[nextUglyIndex]) {
            ++pMultiply2;
        }
        
        while (*pMultiply3 * 3 <= pUglyNumbers[nextUglyIndex]) {
            ++pMultiply3;
        }
        
        while (*pMultiply5 * 5 <= pUglyNumbers[nextUglyIndex]) {
            ++pMultiply5;
        }
        
        ++nextUglyIndex;
    }
    
    int ugly = pUglyNumbers[nextUglyIndex - 1];
    delete [] pUglyNumbers;
    return ugly;
}

int min(int number1, int number2, int number3) {
    int min = (number1 < number2) ? number1: number2;
    min = (min < number3) ? min: number3;
    return min;
}

namespace UglyNumber {
bool isUgly(int number) {
    while (number % 2 == 0) {
        number /= 2;
    }
    
    while (number % 3 == 0) {
        number /= 3;
    }
    
    while (number % 5 == 0) {
        number /= 5;
    }
    
    return (number == 1) ? true: false;
}

int GetUglyNumber_Solution1(int index) {
    if (index <= 0) {
        return 0;
    }
    
    int number = 0;
    int uglyFound = 0;
    // 假设 1 是第一个丑数
    while (uglyFound < index) {
        ++number;
        
        if (isUgly(number)) {
            ++uglyFound;
        }
    }
    
    return number;
}

int Min(int number1, int number2, int number3);

int GetUglyNumber_Solution2(int index) {
    if (index <= 0) {
        return 0;
    }
    
    int* pUglyNumbers = new int[index];
    pUglyNumbers[0] = 1;
    int nextUglyIndex = 1;
    
    // 1. 这三个指针分别记录当前 2 3 5 对应的最小值
    int* pMultiply2 = pUglyNumbers;
    int* pMultiply3 = pUglyNumbers;
    int* pMultiply5 = pUglyNumbers;
    
    while (nextUglyIndex < index) {
        // 2. 从 1 开始，取得它们 *2 *3 *5 之后的最小值
        int min = Min(*pMultiply2 * 2, *pMultiply3 * 3, *pMultiply5 * 5);
        pUglyNumbers[nextUglyIndex] = min;
        
        // 3. 下面 3 个 while 语句，对应上面的 min 选中 2 3 5 中的谁就把谁的指针往后移动
        // 例如:
        // pUglyNumbers = [1, 2, ......(待添加)]
        // 第一轮循环: min = 2 取自 *pMultiply2 * 2 == 2, 所以
        // pMultiply2 向后移动一位 pMultiply 指向 2
        // 而 pMultiply3 和 pMultiply5 因为它们乘 3 或 5 得值大于 2
        // 所以它们还是保持不动，等下次循环
        while (*pMultiply2 * 2 <= pUglyNumbers[nextUglyIndex]) {
            ++pMultiply2;
        }
        
        while (*pMultiply3 * 3 <= pUglyNumbers[nextUglyIndex]) {
            ++pMultiply3;
        }
        
        while (*pMultiply5 * 5 <= pUglyNumbers[nextUglyIndex]) {
            ++pMultiply5;
        }
        
        ++nextUglyIndex;
    }
    // 4. 这里减 1 是因为 pUglyNumbers 是从 0 开始的
    int ugly = pUglyNumbers[nextUglyIndex - 1];
    delete [] pUglyNumbers;
    return ugly;
}

int Min(int number1, int number2, int number3) {
    int min = (number1 < number2)? number1: number2;
    min = (min < number3)? min: number3;
    
    return min;
}

}
