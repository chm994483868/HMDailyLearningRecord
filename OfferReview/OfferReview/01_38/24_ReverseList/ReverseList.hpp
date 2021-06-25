//
//  ReverseList.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/29.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef ReverseList_hpp
#define ReverseList_hpp

#include <stdio.h>
#include <stdlib.h>
#include "List.hpp"

using namespace List;

namespace ReverseList {

// 24：反转链表
// 题目：定义一个函数，输入一个链表的头结点，反转该链表并输出反转后链表的
// 头结点。

ListNode* reverseList(ListNode* pHead);

// 测试代码
ListNode* Test(ListNode* pHead);
// 输入的链表有多个结点
void Test1();
// 输入的链表只有一个结点
void Test2();
// 输入空链表
void Test3();

void Test();

}

#endif /* ReverseList_hpp */
