//
//  CopyComplexList.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/30.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef CopyComplexList_hpp
#define CopyComplexList_hpp

#include <stdio.h>
#include "ComplexList.hpp"

using namespace ComplexList;
using namespace std;

namespace CopyComplexList {

// 35：复杂链表的复制
// 题目：请实现函数ComplexListNode* Clone(ComplexListNode* pHead)，复
// 制一个复杂链表。在复杂链表中，每个结点除了有一个m_pNext指针指向下一个
// 结点外，还有一个m_pSibling 指向链表中的任意结点或者nullptr。
void cloneNodes(ComplexListNode* pHead);
void connectSiblingNodes(ComplexListNode* pHead);
ComplexListNode* reconnectNodes(ComplexListNode* pHead);
ComplexListNode* clone(ComplexListNode* pHead);

// 测试代码
void Test(const char* testName, ComplexListNode* pHead);
//          -----------------
//         \|/              |
//  1-------2-------3-------4-------5
//  |       |      /|\             /|\
//  --------+--------               |
//          -------------------------
void Test1();
// m_pSibling指向结点自身
//          -----------------
//         \|/              |
//  1-------2-------3-------4-------5
//         |       | /|\           /|\
//         |       | --             |
//         |------------------------|
void Test2();
// m_pSibling形成环
//          -----------------
//         \|/              |
//  1-------2-------3-------4-------5
//          |              /|\
//          |               |
//          |---------------|
void Test3();
// 只有一个结点
void Test4();
// 鲁棒性测试
void Test5();

void Test();

}

#endif /* CopyComplexList_hpp */
