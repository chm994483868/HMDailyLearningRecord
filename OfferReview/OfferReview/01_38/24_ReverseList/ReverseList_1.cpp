//
//  ReverseList_1.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/4.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "ReverseList_1.hpp"

ListNode* ReverseList_1::reverseList(ListNode* pHead) {
    ListNode* pReverseHead = nullptr;
    ListNode* pNode = pHead;
    ListNode* pPrev = nullptr;
    
    while (pNode != nullptr) {
        ListNode* pNext = pNode->m_pNext;
        
        if (pNext == nullptr) {
            pReverseHead = pNode;
        }
        
        pNode->m_pNext = pPrev;
        pPrev = pNode;
        pNode = pNext;
    }
    
    return pReverseHead;
}

// 测试代码
ListNode* ReverseList_1::Test(ListNode* pHead) {
    printf("The original list is: \n");
    PrintList(pHead);

    ListNode* pReversedHead = reverseList(pHead);

    printf("The reversed list is: \n");
    PrintList(pReversedHead);

    return pReversedHead;
}

// 输入的链表有多个结点
void ReverseList_1::Test1() {
    ListNode* pNode1 = CreateListNode(1);
    ListNode* pNode2 = CreateListNode(2);
    ListNode* pNode3 = CreateListNode(3);
    ListNode* pNode4 = CreateListNode(4);
    ListNode* pNode5 = CreateListNode(5);

    ConnectListNodes(pNode1, pNode2);
    ConnectListNodes(pNode2, pNode3);
    ConnectListNodes(pNode3, pNode4);
    ConnectListNodes(pNode4, pNode5);

    ListNode* pReversedHead = Test(pNode1);

    DestroyList(pReversedHead);
}

// 输入的链表只有一个结点
void ReverseList_1::Test2() {
    ListNode* pNode1 = CreateListNode(1);

    ListNode* pReversedHead = Test(pNode1);

    DestroyList(pReversedHead);
}

// 输入空链表
void ReverseList_1::Test3() {
    Test(nullptr);
}

void ReverseList_1::Test() {
    Test1();
    Test2();
    Test3();
}
