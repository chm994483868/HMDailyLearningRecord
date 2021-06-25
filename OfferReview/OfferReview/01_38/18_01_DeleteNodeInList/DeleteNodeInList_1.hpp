//
//  DeleteNodeInList_1.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/3.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef DeleteNodeInList_1_hpp
#define DeleteNodeInList_1_hpp

#include <stdio.h>
#include <stdlib.h>
#include "List.hpp"

using namespace List;

namespace DeleteNodeInList_1 {

// 18（一）：在O(1)时间删除链表结点
// 题目：给定单向链表的头指针和一个结点指针，定义一个函数在O(1)时间删除该结点。

void deleteNode(ListNode** pListHead, ListNode* pToBeDeleted);

// 测试代码
void Test(ListNode* pListHead, ListNode* pNode);
// 链表中有多个结点，删除中间的结点
void Test1();
// 链表中有多个结点，删除尾结点
void Test2();
// 链表中有多个结点，删除头结点
void Test3();
// 链表中只有一个结点，删除头结点
void Test4();
// 链表为空
void Test5();

void Test();

}

#endif /* DeleteNodeInList_1_hpp */
