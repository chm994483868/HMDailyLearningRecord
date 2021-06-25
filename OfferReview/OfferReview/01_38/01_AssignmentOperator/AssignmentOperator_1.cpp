//
//  AssignmentOperator_1.cpp
//  OfferReview
//
//  Created by HM C on 2020/10/31.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "AssignmentOperator_1.hpp"

AssignmentOperator_1::CMyString::CMyString(const char* pData) {
    if (pData == nullptr) {
        // 同时如果这里抛了错，但我们还没有修改原来实例的状态，因此实例的状态还是有效的，这也保证了异常安全性
        m_pData = new char[1];
        m_pData[0] = '\0';
    } else {
        unsigned long length = strlen(pData);
        m_pData = new char[length + 1];
        strcpy(m_pData, pData);
    }
}

AssignmentOperator_1::CMyString::CMyString(const CMyString& str) {
    unsigned long length = strlen(str.m_pData);
    m_pData = new char[length + 1];
    strcpy(m_pData, str.m_pData);
}

AssignmentOperator_1::CMyString::~CMyString() {
    delete [] m_pData;
}

AssignmentOperator_1::CMyString& AssignmentOperator_1::CMyString::operator=(const CMyString& str) {
    // 一：初级解法
//    // 1. 如果是同一个 CMyString 变量，则直接返回原值
//    if (this == &str) { // 地址比较
//        return *this;
//    }
//    // 2. 释放原始值
//    delete [] m_pData;
//    m_pData = nullptr;
//    // 3. 为 m_pData 开辟空间，并把 str.m_pData 复制给 m_pData
//    m_pData = new char[strlen(str.m_pData) + 1];
//    strcpy(m_pData, str.m_pData);
//
//    return *this;
    // 二：考虑异常安全性的解法
    if (this != &str) {
        CMyString strTemp(str);
        char* pTemp = strTemp.m_pData;
        strTemp.m_pData = m_pData;
        m_pData = pTemp;
    }
    
    return *this;
}

void AssignmentOperator_1::CMyString::Print() {
    printf("%s", m_pData);
}

// 测试代码
// 常规测试
void AssignmentOperator_1::Test1() {
    printf("Test1 begins:\n");

    const char* text = "Hello world";
    CMyString str1(text);
    CMyString str2;
    str2 = str1;

    printf("The expected result is: %s.\n", text);

    printf("The actual result is: ");
    str2.Print();
    
    printf(".\n");
}

// 赋值给自己
void AssignmentOperator_1::Test2() {
    printf("Test2 begins:\n");

    const char* text = "Hello world";
    CMyString str1(text);
    str1 = str1;

    printf("The expected result is: %s.\n", text);

    printf("The actual result is: ");
    str1.Print();
    
    printf(".\n");
}

// 连续赋值
void AssignmentOperator_1::Test3() {
    printf("Test3 begins:\n");

    const char* text = "Hello world";

    CMyString str1(text);
    CMyString str2, str3;
    str3 = str2 = str1;

    printf("The expected result is: %s.\n", text);

    printf("The actual result is: ");
    str2.Print();
    printf(".\n");

    printf("The expected result is: %s.\n", text);

    printf("The actual result is: ");
    str3.Print();
    printf(".\n");
}

void AssignmentOperator_1::Test() {
    Test1();
    Test2();
    Test3();
}
