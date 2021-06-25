//
//  MergeSortedLists.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/29.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef MergeSortedLists_hpp
#define MergeSortedLists_hpp

#include <stdio.h>
#include <stdlib.h>
#include "List.hpp"

using namespace List;

namespace MergeSortedLists {

// 25：合并两个排序的链表
// 题目：输入两个递增排序的链表，合并这两个链表并使新链表中的结点仍然是按
// 照递增排序的。例如输入图3.11中的链表1和链表2，则合并之后的升序链表如链
// 表3所示。

ListNode* merge(ListNode* pHead1, ListNode* pHead2);

// 测试代码
ListNode* Test(char* testName, ListNode* pHead1, ListNode* pHead2);
// list1: 1->3->5
// list2: 2->4->6
void Test1();
// 两个链表中有重复的数字
// list1: 1->3->5
// list2: 1->3->5
void Test2();
// 两个链表都只有一个数字
// list1: 1
// list2: 2
void Test3();
// 一个链表为空链表
// list1: 1->3->5
// list2: 空链表
void Test4();
// 两个链表都为空链表
// list1: 空链表
// list2: 空链表
void Test5();

void Test();

}

#endif /* MergeSortedLists_hpp */
