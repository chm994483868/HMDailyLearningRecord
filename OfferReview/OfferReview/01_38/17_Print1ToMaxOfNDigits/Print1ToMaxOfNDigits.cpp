//
//  Print1ToMaxOfNDigits.cpp
//  OfferReview
//
//  Created by HM C on 2020/7/28.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "Print1ToMaxOfNDigits.hpp"

void Print1ToMaxOfNDigits::printNumber(char* number) {
    bool isBeginning0 = true;
    unsigned long nLength = strlen(number);
    unsigned long i = 0;
    for (; i < nLength; ++i) {
        if (isBeginning0 && number[i] != '0') {
            isBeginning0 = false;
        }
        
        if (!isBeginning0) {
            printf("%c", number[i]);
        }
    }
    
    printf("\t");
}

bool Print1ToMaxOfNDigits::increment(char* number) {
    bool isOverflow = false;
    int nTakeOver = 0;
    unsigned long nLength = strlen(number);
    unsigned long i = nLength - 1;
    
    for (; i >= 0; --i) {
        int sum = number[i] - '0' + nTakeOver;
        if (i == nLength - 1) {
            ++sum;
        }
        
        if (sum >= 10) {
            if (i == 0) {
                isOverflow = true;
            } else {
                sum -= 10;
                nTakeOver = 1;
                number[i] = '0' + sum;
            }
        } else {
            number[i] = '0' + sum;
            break;
        }
    }
    
    return isOverflow;
}

void Print1ToMaxOfNDigits::print1ToMaxOfNDigits_1(int n) {
    if (n <= 0) {
        return;
    }
    
    char* number = new char[n + 1];
    memset(number, '0', n);
    number[n] = '\0';
    
    while (!increment(number)) {
        printNumber(number);
    }
    
    delete [] number;
}

void Print1ToMaxOfNDigits::print1ToMaxOfNDigitsRecursively(char* number, int length, int index) {
    if (index == length - 1) {
        printNumber(number);
        return;
    }
    
    for (int i = 0; i < 10; ++i) {
        number[index + 1] = i + '0';
        print1ToMaxOfNDigitsRecursively(number, length, index + 1);
    }
}

void Print1ToMaxOfNDigits::print1ToMaxOfNDigits_2(int n) {
    if (n <= 0) {
        return;
    }
    
    char* number = new char[n + 1];
    number[n] = '\0';
    
    for (int i = 0; i < 10; ++i) {
        number[0] = i + '0';
        print1ToMaxOfNDigitsRecursively(number, n, 0);
    }
}

// 测试代码
void Print1ToMaxOfNDigits::Test(int n) {
    printf("Test for %d begins:\n", n);

    print1ToMaxOfNDigits_1(n);
    print1ToMaxOfNDigits_2(n);

    printf("\nTest for %d ends.\n", n);
}

void Print1ToMaxOfNDigits::Test() {
    Test(1);
    Test(2);
    Test(3);
    Test(0);
    Test(-1);
}

class Solution {
public:
    
    bool increment(char* number) {
        bool isOverflow = false;
        int nTakeOver = 0;
        unsigned long length = strlen(number);
        unsigned long i = length - 1;
        for (; i >= 0; --i) {
            int sum = number[i] - '0' + nTakeOver;
            if (i == length - 1) {
                ++sum;
            }
            
            if (sum > 10) {
                if (i == 0) {
                    isOverflow = true;
                } else {
                    sum -= 10;
                    nTakeOver = 1;
                    number[i] = '0' + sum;
                }
            } else {
                number[i] = '0' + sum;
                break;
            }
        }
        
        return isOverflow;
    }
    
    vector<int> printNumbers(int n) {
        vector<int> b;
        
        if (n <= 0) {
            return b;
        }
        
        char* numbers = new char[n + 1];
        memset(numbers, '0', n);
        numbers[n] = '\0';
        
        while (!increment(numbers)) {
            b.push_back(atoi(numbers));
        }
        
        delete [] numbers;
        
        return b;
    }
};
