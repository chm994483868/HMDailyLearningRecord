//
//  PrintListInReversedOrder_1.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/1.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef PrintListInReversedOrder_1_hpp
#define PrintListInReversedOrder_1_hpp

#include <stdio.h>
#include <stack>
#include <vector>
#include "List.hpp"

using namespace List;
using namespace std;

namespace PrintListInReversedOrder_1 {

// 开局的两个小题
// 往链表末尾添加一个节点
void addToTail(ListNode** pHead, int value);
// 在链表中找到第一个含有某值的节点并删除该节点的代码
void removeNode(ListNode** pHead, int value);

// 6：从尾到头打印链表
// 题目：输入一个链表的头结点，从尾到头反过来打印出每个节点的值。
void printListReversingly_Iteratively(ListNode* pHead);
void printListReversingly_Recursively(ListNode* pHead);

// 测试代码
void Test(ListNode* pHead);
// 1->2->3->4->5
void Test1();
// 只有一个结点的链表: 1
void Test2();
// 空链表
void Test3();

void Test();

}

#endif /* PrintListInReversedOrder_1_hpp */
