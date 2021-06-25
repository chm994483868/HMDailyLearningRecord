//
//  List.cpp
//  OfferReview
//
//  Created by CHM on 2020/7/29.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "List.hpp"

// 辅助代码
List::ListNode* List::CreateListNode(int value) {
    ListNode* pNode = new ListNode();
    pNode->m_nValue = value;
    pNode->m_pNext = nullptr;

    return pNode;
}

void List::ConnectListNodes(ListNode* pCurrent, ListNode* pNext) {
    if(pCurrent == nullptr) {
        printf("Error to connect two nodes.\n");
        exit(1);
    }

    pCurrent->m_pNext = pNext;
}

void List::PrintListNode(ListNode* pNode) {
    if(pNode == nullptr) {
        printf("The node is nullptr\n");
    } else {
        printf("The key in node is %d.\n", pNode->m_nValue);
    }
}

void List::PrintList(ListNode* pHead) {
    printf("PrintList starts.\n");
    
    ListNode* pNode = pHead;
    while(pNode != nullptr) {
        printf("%d\t", pNode->m_nValue);
        pNode = pNode->m_pNext;
    }

    printf("\nPrintList ends.\n");
}

void List::DestroyList(ListNode* pHead) {
    ListNode* pNode = pHead;
    while(pNode != nullptr) {
        pHead = pHead->m_pNext;
        delete pNode;
        pNode = pHead;
    }
}
