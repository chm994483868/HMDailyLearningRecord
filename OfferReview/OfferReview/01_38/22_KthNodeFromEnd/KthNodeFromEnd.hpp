//
//  KthNodeFromEnd.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/29.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef KthNodeFromEnd_hpp
#define KthNodeFromEnd_hpp

#include <stdio.h>
#include <stdlib.h>
#include "List.hpp"

using namespace List;

namespace KthNodeFromEnd {

// 求链表的中间节点。如果链表中的节点总数为奇数，则返回中间节点；
// 如果节点总数是偶数，则返回中间两个节点的任意一个。
// 可以定义两个指针，同时从链表的头节点出发，一个指针一次走一步
// 另一个指针一次走两步。当走的快的指针走到链表的末尾时，走的慢的指针正好
// 在链表的中间。

// 22：链表中倒数第k个结点
// 题目：输入一个链表，输出该链表中倒数第k个结点。为了符合大多数人的习惯，
// 本题从1开始计数，即链表的尾结点是倒数第1个结点。例如一个链表有6个结点，
// 从头结点开始它们的值依次是1、2、3、4、5、6。这个链表的倒数第3个结点是
// 值为4的结点。

ListNode* findKthToTail(ListNode* pListHead, unsigned int k);
ListNode* findMddleNode(ListNode* pListHead);

// 测试代码
// 测试要找的结点在链表中间
void Test1();
// 测试要找的结点是链表的尾结点
void Test2();
// 测试要找的结点是链表的头结点
void Test3();
// 测试空链表
void Test4();
// 测试输入的第二个参数大于链表的结点总数
void Test5();
// 测试输入的第二个参数为0
void Test6();

void Test();

}

#endif /* KthNodeFromEnd_hpp */
