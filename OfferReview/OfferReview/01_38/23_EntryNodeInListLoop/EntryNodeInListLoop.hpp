//
//  EntryNodeInListLoop.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/29.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef EntryNodeInListLoop_hpp
#define EntryNodeInListLoop_hpp

#include <stdio.h>
#include <stdlib.h>
#include "List.hpp"

using namespace List;

namespace EntryNodeInListLoop {

// 23：链表中环的入口结点
// 题目：一个链表中包含环，如何找出环的入口结点？例如，在图3.8的链表中，
// 环的入口结点是结点3。

ListNode* meetingNode(ListNode* pHead);
ListNode* entryNodeOfLoop(ListNode* phead);

// 测试代码
void Test(char* testName, ListNode* pHead, ListNode* entryNode);
// A list has a node, without a loop
void Test1();
// A list has a node, with a loop
void Test2();
// A list has multiple nodes, with a loop
void Test3();
// A list has multiple nodes, with a loop
void Test4();
// A list has multiple nodes, with a loop
void Test5();
// A list has multiple nodes, without a loop
void Test6();
// Empty list
void Test7();

void Test();

}

#endif /* EntryNodeInListLoop_hpp */
