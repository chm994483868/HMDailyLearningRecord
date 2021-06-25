//
//  KthNodeFromEnd.cpp
//  OfferReview
//
//  Created by CHM on 2020/7/29.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "KthNodeFromEnd.hpp"

ListNode* KthNodeFromEnd::findKthToTail(ListNode* pListHead, unsigned int k) {
    if (pListHead == nullptr || k <= 0) {
        return nullptr;
    }
    
    ListNode* pAHead = pListHead;
    unsigned int i = 0;
    for (; i < k - 1; ++i) {
        pAHead = pAHead->m_pNext;
        
        if (pAHead == nullptr) {
            return nullptr;
        }
    }
    
    ListNode* pBehind = pListHead;
    while (pAHead->m_pNext != nullptr) {
        pAHead = pAHead->m_pNext;
        pBehind = pBehind->m_pNext;
    }
    
    return pBehind;
}

ListNode* KthNodeFromEnd::findMddleNode(ListNode* pListHead) {
    if (pListHead == nullptr) {
        return nullptr;
    }

    ListNode* pFast = pListHead;
    ListNode* pSlow = pListHead;
    
//    while (pFast != nullptr && pFast->m_pNext != nullptr) {
    while (pFast->m_pNext != nullptr && pFast->m_pNext->m_pNext != nullptr) {
        pSlow = pSlow->m_pNext;
        
        pFast = pFast->m_pNext;
        if (pFast != nullptr) {
            pFast = pFast->m_pNext;
        }
    }
    
    return pSlow;
}

// 测试代码
// 测试要找的结点在链表中间
void KthNodeFromEnd::Test1() {
    printf("=====Test1 starts:=====\n");
    ListNode* pNode1 = CreateListNode(1);
    ListNode* pNode2 = CreateListNode(2);
    ListNode* pNode3 = CreateListNode(3);
    ListNode* pNode4 = CreateListNode(4);
    ListNode* pNode5 = CreateListNode(5);

    ConnectListNodes(pNode1, pNode2);
    ConnectListNodes(pNode2, pNode3);
    ConnectListNodes(pNode3, pNode4);
    ConnectListNodes(pNode4, pNode5);

    printf("expected result: 4.\n");
    ListNode* pNode = findKthToTail(pNode1, 2);
    PrintListNode(pNode);
    
    printf("MDDLE expected result: 3.\n");
    ListNode* pMiddle = findMddleNode(pNode1);
    PrintListNode(pMiddle);

    DestroyList(pNode1);
}

// 测试要找的结点是链表的尾结点
void KthNodeFromEnd::Test2() {
    printf("=====Test2 starts:=====\n");
    ListNode* pNode1 = CreateListNode(1);
    ListNode* pNode2 = CreateListNode(2);
    ListNode* pNode3 = CreateListNode(3);
    ListNode* pNode4 = CreateListNode(4);
    ListNode* pNode5 = CreateListNode(5);

    ConnectListNodes(pNode1, pNode2);
    ConnectListNodes(pNode2, pNode3);
    ConnectListNodes(pNode3, pNode4);
    ConnectListNodes(pNode4, pNode5);

    printf("expected result: 5.\n");
    ListNode* pNode = findKthToTail(pNode1, 1);
    PrintListNode(pNode);
    
    printf("MDDLE expected result: 3.\n");
    ListNode* pMiddle = findMddleNode(pNode1);
    PrintListNode(pMiddle);

    DestroyList(pNode1);
}
// 测试要找的结点是链表的头结点
void KthNodeFromEnd::Test3() {
    printf("=====Test3 starts:=====\n");
    ListNode* pNode1 = CreateListNode(1);
    ListNode* pNode2 = CreateListNode(2);
    ListNode* pNode3 = CreateListNode(3);
    ListNode* pNode4 = CreateListNode(4);
    ListNode* pNode5 = CreateListNode(5);

    ConnectListNodes(pNode1, pNode2);
    ConnectListNodes(pNode2, pNode3);
    ConnectListNodes(pNode3, pNode4);
    ConnectListNodes(pNode4, pNode5);

    printf("expected result: 1.\n");
    ListNode* pNode = findKthToTail(pNode1, 5);
    PrintListNode(pNode);

    DestroyList(pNode1);
}

// 测试空链表
void KthNodeFromEnd::Test4() {
    printf("=====Test4 starts:=====\n");
    printf("expected result: nullptr.\n");
    ListNode* pNode = findKthToTail(nullptr, 100);
    PrintListNode(pNode);
}

// 测试输入的第二个参数大于链表的结点总数
void KthNodeFromEnd::Test5() {
    printf("=====Test5 starts:=====\n");
    ListNode* pNode1 = CreateListNode(1);
    ListNode* pNode2 = CreateListNode(2);
    ListNode* pNode3 = CreateListNode(3);
    ListNode* pNode4 = CreateListNode(4);
    ListNode* pNode5 = CreateListNode(5);

    ConnectListNodes(pNode1, pNode2);
    ConnectListNodes(pNode2, pNode3);
    ConnectListNodes(pNode3, pNode4);
    ConnectListNodes(pNode4, pNode5);

    printf("expected result: nullptr.\n");
    ListNode* pNode = findKthToTail(pNode1, 6);
    PrintListNode(pNode);

    DestroyList(pNode1);
}

// 测试输入的第二个参数为0
void KthNodeFromEnd::Test6() {
    printf("=====Test6 starts:=====\n");
    ListNode* pNode1 = CreateListNode(1);
    ListNode* pNode2 = CreateListNode(2);
    ListNode* pNode3 = CreateListNode(3);
    ListNode* pNode4 = CreateListNode(4);
    ListNode* pNode5 = CreateListNode(5);

    ConnectListNodes(pNode1, pNode2);
    ConnectListNodes(pNode2, pNode3);
    ConnectListNodes(pNode3, pNode4);
    ConnectListNodes(pNode4, pNode5);

    printf("expected result: nullptr.\n");
    ListNode* pNode = findKthToTail(pNode1, 0);
    PrintListNode(pNode);

    DestroyList(pNode1);
}

void KthNodeFromEnd::Test() {
    Test1();
    Test2();
    Test3();
    Test4();
    Test5();
    Test6();
}
