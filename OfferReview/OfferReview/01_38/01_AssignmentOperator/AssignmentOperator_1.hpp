//
//  AssignmentOperator_1.hpp
//  OfferReview
//
//  Created by HM C on 2020/10/31.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef AssignmentOperator_1_hpp
#define AssignmentOperator_1_hpp

#include <stdio.h>
#include <cstdio>
#include <cstring>

using namespace std;

namespace AssignmentOperator_1 {

// 1: 赋值运算符函数
// 题目：如下为类型 CMyString 的声明，请为该类型添加赋值运算符函数

class CMyString {
public:
    CMyString(const char* pData = nullptr); // 构造函数，pData 参数默认为 nullptr
    CMyString(const CMyString& str); // 复制构造函数，参数的类型为常量引用
    ~CMyString(void); // 析构函数
    
    CMyString& operator = (const CMyString& str);
    
    void Print();
    
private:
    char* m_pData;
};

// 测试代码
// 常规测试
void Test1();
// 赋值给自己
void Test2();
// 连续赋值
void Test3();

void Test();

}

#endif /* AssignmentOperator_1_hpp */
