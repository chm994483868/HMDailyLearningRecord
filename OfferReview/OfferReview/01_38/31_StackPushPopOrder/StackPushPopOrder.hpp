//
//  StackPushPopOrder.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/30.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef StackPushPopOrder_hpp
#define StackPushPopOrder_hpp

#include <stdio.h>
#include <stack>

using namespace std;

namespace StackPushPopOrder {

// 31：栈的压入、弹出序列
// 题目：输入两个整数序列，第一个序列表示栈的压入顺序，请判断第二个序列是
// 否为该栈的弹出顺序。假设压入栈的所有数字均不相等。例如序列1、2、3、4、
// 5是某栈的压栈序列，序列4、5、3、2、1是该压栈序列对应的一个弹出序列，但
// 4、3、5、1、2就不可能是该压栈序列的弹出序列。
bool isPopOrder(const int* pPush, const int* pPop, int nLength);

// 测试代码
void Test(const char* testName, const int* pPush, const int* pPop, int nLength, bool expected);
void Test1();
void Test2();
void Test3();
void Test4();
// push和pop序列只有一个数字
void Test5();
void Test6();
void Test7();

void Test();

}

#endif /* StackPushPopOrder_hpp */
