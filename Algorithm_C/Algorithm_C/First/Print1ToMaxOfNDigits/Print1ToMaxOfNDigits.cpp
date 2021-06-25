//
//  Print1ToMaxOfNDigits.cpp
//  Algorithm_C
//
//  Created by HM C on 2020/7/14.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "Print1ToMaxOfNDigits.hpp"

void printNumber(char* number);
bool increment(char* number);
void print1ToMaxOfNDigitsRecursively(char* number, int length, int index);

void print1ToMaxOfNDigits_1(int n) {
    if (n <= 0)
        return;
    
    char* number = new char[n + 1];
    memset(number, '0', n);
    number[n] = '\0';
    
    while (!increment(number)) {
        printNumber(number);
    }
    
    delete [] number;
}

bool increment(char* number) {
    // 1. 标示当前是否到了 n 位数的最大值
    bool isOverflow = false;
    // 2. 标示本次操作有没有进位
    int nTakeOver = 0;
    unsigned long nLength = strlen(number);
    // n = 4;
    // number = "0000";
    for (unsigned long i = nLength - 1; i >= 0; --i) {
        // 3. 字符转为相同的 int 值并加上来自右侧的进位值（按目前的操作 nTakeOver 最大为 1）
        // 这里沙雕了，如果是加减操作，往高位进位时永远是 1，乘法的进位才会大于 1 😓😓
        // 按乘法口诀 9 * 9 = 81，它最大进位是 8
        // 所以这个进位标示在初始化时为 0，nSum 大于等于 10 时被标记为 1
        // 即使下次下次循环也不需要把它置为 0，因为如果再没有进位，本次循环就结束了。
        int nSum = number[i] - '0' + nTakeOver;
        // 4. 如果当前是字符串最右侧的字符，即个位数，做一次自增运算
        if (i == nLength - 1)
            ++nSum;
        
        // 5. 如果当前值大于等于 10 了，表示这个时候要进位了
        if (nSum >= 10) {
            // 6. 如果目前处于 0 位，即字符串最左侧，即目前肯定是 "9999" 了，再进位就超过 4 位数字的最大值了，即此时我们已经打印到了最大值的边界了，就要完成所有打印了
            if (i == 0)
                isOverflow = true;
            else {
                // 7. 如果不是最左侧，就正常进位
                nSum -= 10;
                // 8. 设置进位标记
                nTakeOver = 1;
                number[i] = '0' + nSum; // 数字转为相同的字符
            }
        } else {
            number[i] = '0' + nSum;
            break; // 直接结束循环，进行下个数字的循环
            // 这个循环其实结束的很快，除非遇到这样的 "00099999" +1 要变成 "00100000"
            // 直到前面变成 1 的时候，本次循环才能结束，如果是普通的个位数字 +1
            // 且不发生进位的就可以跳出循环，执行下面的 return 语句了
        }
    }
    
    return isOverflow;
}

void print1ToMaxOfNDigits_2(int n) {
    if (n <= 0)
        return;
    
    char* number = new char[n + 1];
    number[n] = '\0';
    
    for (int i = 0; i < 10; ++i) {
        number[0] = i + '0';
        print1ToMaxOfNDigitsRecursively(number, n, 0);
    }
    
    delete [] number;
}

void print1ToMaxOfNDigitsRecursively(char* number, int length, int index) {
    if (index == length - 1) {
        printNumber(number);
        return;
    }
    
    for (int i = 0; i < 10; ++i) {
        number[index + 1] = i + '0';
        print1ToMaxOfNDigitsRecursively(number, length, index + 1);
    }
}

void printNumber(char* number) {
    // 1. 标示当前是 0 位
    bool isBeginning0 = true;
    unsigned long nLength = strlen(number);
    
    for (int i = 0; i < nLength; ++i) {
        // 2. 从遇到第一个零开始把 isBeginning0 置为 false
        // char* number = "00007892"; 从左到右开始打印，7 前面的零全部忽略
        if (isBeginning0 && number[i] != '0')
            isBeginning0 = false;
        
        // 3. 从非零开始打印字符
        if (!isBeginning0)
            printf("%c", number[i]);
    }
    
    printf("\t");
}

void Test(int n) {
    printf("Test for %d begins:\n", n);

    print1ToMaxOfNDigits_1(n);

    printf("\nTest for %d ends.\n", n);
}

void startTest_Print1ToMaxOfNDigits() {
    Test(1);
    Test(2);
    Test(3);
    Test(0);
    Test(-1);
}
