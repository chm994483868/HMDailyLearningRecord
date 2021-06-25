//
//  ReplaceSpaces.cpp
//  OfferReview
//
//  Created by CHM on 2020/7/27.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "ReplaceSpaces.hpp"

void ReplaceSpaces::replaceBlank(char str[], int length) {
    if (str == nullptr || length <= 0) {
        return;
    }
    
    int originalLength = 0;
    int numberOfBlank = 0;
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] == ' ') {
            ++numberOfBlank;
        }
        
        ++originalLength;
        ++i;
    }
    
    int newLength = originalLength + numberOfBlank * 2;
    if (newLength > length) {
        return;
    }
    
    int indexOfOriginal = originalLength;
    int indexOfNew = newLength;
    
    while (indexOfOriginal >= 0 && indexOfNew > indexOfOriginal) {
        
        if (str[indexOfOriginal] == ' ') {
            str[indexOfNew--] = '0';
            str[indexOfNew--] = '2';
            str[indexOfNew--] = '%';
        } else {
            str[indexOfNew--] = str[indexOfOriginal];
        }
        
        --indexOfOriginal;
    }
}

string ReplaceSpaces::replaceSpace(string s) {
    unsigned long originalLength = s.length() - 1;
    int i = 0;
    for (; i < s.length(); ++i) {
        if (s[i] == ' ') {
            s += "00";
        }
    }
    
    unsigned long newLength = s.length() - 1;
    while (originalLength >= 0 && newLength > originalLength) {
        if (s[originalLength] == ' ') {
            s[newLength--] = '0';
            s[newLength--] = '2';
            s[newLength--] = '%';
        } else {
            s[newLength--] = s[originalLength];
        }

        --originalLength;
    }
    
    return s;
}

// 测试代码
void ReplaceSpaces::Test(char* testName, char str[], int length, char expected[]) {
    if(testName != nullptr)
        printf("%s begins: ", testName);

    replaceBlank(str, length);

    if(expected == nullptr && str == nullptr)
        printf("passed.\n");
    else if(expected == nullptr && str != nullptr)
        printf("failed.\n");
    else if(strcmp(str, expected) == 0) // 替换结果和预期字符串比较是否相同
        printf("passed. str = %p expected = %p \n", str, expected);
    else
        printf("failed.\n");
}

// 空格在句子中间
void ReplaceSpaces::Test1() {
    const int length = 100;

    char str[length] = "hello world";
    Test("Test1", str, length, "hello%20world");
}

// 空格在句子开头
void ReplaceSpaces::Test2() {
    const int length = 100;

    char str[length] = " helloworld";
    Test("Test2", str, length, "%20helloworld");
}

// 空格在句子末尾
void ReplaceSpaces::Test3() {
    const int length = 100;

    char str[length] = "helloworld ";
    Test("Test3", str, length, "helloworld%20");
}

// 连续有两个空格
void ReplaceSpaces::Test4() {
    const int length = 100;

    char str[length] = "hello  world";
    Test("Test4", str, length, "hello%20%20world");
}

// 传入nullptr
void ReplaceSpaces::Test5() {
    Test("Test5", nullptr, 0, nullptr);
}

// 传入内容为空的字符串
void ReplaceSpaces::Test6() {
    const int length = 100;

    char str[length] = "";
    Test("Test6", str, length, "");
}

//传入内容为一个空格的字符串
void ReplaceSpaces::Test7() {
    const int length = 100;

    char str[length] = " ";
    Test("Test7", str, length, "%20");
}

// 传入的字符串没有空格
void ReplaceSpaces::Test8() {
    const int length = 100;

    char str[length] = "helloworld";
    Test("Test8", str, length, "helloworld");
}

// 传入的字符串全是空格
void ReplaceSpaces::Test9() {
    const int length = 100;

    char str[length] = "   ";
    Test("Test9", str, length, "%20%20%20");
}

void ReplaceSpaces::Test() {
    Test1();
    Test2();
    Test3();
    Test4();
    Test5();
    Test6();
    Test7();
    Test8();
    Test9();
}
