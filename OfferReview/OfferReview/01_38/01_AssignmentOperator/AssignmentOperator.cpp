//
//  AssignmentOperator.cpp
//  OfferReview
//
//  Created by HM C on 2020/7/26.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "AssignmentOperator.hpp"

AssignmentOperator::CMyString::CMyString(const char* pData) {
    if (pData == nullptr) {
        // 同时如果这里抛了错，但我们还没有修改原来实例的状态，因此实例的状态还是有效的，这也保证了异常安全性
        m_pData = new char[1];
        m_pData[0] = '\0';
    } else{
        unsigned long length = strlen(pData);
        m_pData = new char[length + 1];
        strcpy(m_pData, pData);
    }
}

AssignmentOperator::CMyString::CMyString(const CMyString& str) {
    unsigned long length = strlen(str.m_pData);
    m_pData = new char[length + 1];
    strcpy(m_pData, str.m_pData);
}

AssignmentOperator::CMyString::~CMyString() {
    delete [] m_pData;
}

AssignmentOperator::CMyString& AssignmentOperator::CMyString::operator=(const CMyString& str) {
    // 一些思考：
    // char* n; n 是一个指针变量，直接打印 n 显示的是 n 这个指针指向的地址，打印 *n 显示的是 n 指向的地址里面存储的内容，打印 &n 显示的是 n 这个指针变量自己的地址。
    // int a = 2; int& m = a; m 是 a 的引用。引用 m 可以直接理解为 a 的一个别名。直接打印 m 时显示的是 2 即 a 的值，打印 &m 时显示的是 a 的地址，和 &a 是完成一致的。
    
    // 所以这个 if 里面，就可以得到合理的解释了
    // this 表示的是当前变量的地址，&str 表示的是参数的地址
    // 返回值是 CMyString 类型的引用，所以需要返回当前变量的值即: *this，就比如上面的 m 初始化时是直接用的 a。
    // 一：初级解法
    // 1. 如果是同一个 CMySting 变量，则直接返回原值
//    if (this == &str) {
//        return *this;
//    }
//
//    // 2. 释放原始值
//    delete [] m_pData;
//    m_pData = nullptr;
//
//    // 3. 为 m_pData 开辟空间，并把 str.m_pData 复制给 m_pData
//    m_pData = new char[strlen(str.m_pData) + 1];
//    strcpy(m_pData, str.m_pData);
//
//    return *this;
    // 二：考虑异常安全性的解法
    if (this != &str) {
        // 1. 调用复制构造函数，创建一个 CMyString 的临时变量
        CMyString strTemp(str);
        // 2. 取出临时变量的 m_pData 等待复制给 this->m_pData
        char* pTemp = strTemp.m_pData;
        // 3. 把原始的 m_pData 赋给临时的 tempStr，待出了下面的右边花括号 tempStr 释放时一起带着原始的 m_pData 释放
        strTemp.m_pData = m_pData;
        m_pData = pTemp;
    }
    
    return *this;
}

void AssignmentOperator::CMyString::Print() {
    printf("%s", m_pData);
}

// 测试代码
// 常规测试
void AssignmentOperator::Test1() {
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
void AssignmentOperator::Test2() {
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
void AssignmentOperator::Test3() {
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

void AssignmentOperator::Test() {
    Test1();
    Test2();
    Test3();
}
