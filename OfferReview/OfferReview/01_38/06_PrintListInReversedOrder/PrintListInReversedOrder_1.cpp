//
//  PrintListInReversedOrder_1.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/1.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "PrintListInReversedOrder_1.hpp"

// 往链表末尾添加一个节点
void PrintListInReversedOrder_1::addToTail(ListNode** pHead, int value) {
    if (pHead == nullptr) {
        return;
    }
    
    ListNode* pNew = new ListNode();
    pNew->m_nValue = value;
    pNew->m_pNext = nullptr;
    
    if (*pHead == nullptr) {
        *pHead = pNew;
    } else {
        ListNode* pNode = *pHead;
        while (pNode->m_pNext != nullptr) {
            pNode = pNode->m_pNext;
        }
        
        pNode->m_pNext = pNew;
    }
}

void PrintListInReversedOrder_1::removeNode(ListNode** pHead, int value) {
    if (pHead == nullptr || *pHead == nullptr) {
        return;
    }
    
    ListNode* pToBeDeleted = nullptr;
    if ((*pHead)->m_nValue == value) {
        pToBeDeleted = *pHead;
        *pHead = (*pHead)->m_pNext;
    } else {
        ListNode* pNode = *pHead;
        while (pNode->m_pNext != nullptr && pNode->m_pNext->m_nValue != value) {
            pNode = pNode->m_pNext;
        }
        
        if (pNode->m_pNext != nullptr && pNode->m_pNext->m_nValue == value) {
            pToBeDeleted = pNode->m_pNext;
            pNode->m_pNext = pNode->m_pNext->m_pNext;
        }
    }
    
    if (pToBeDeleted != nullptr) {
        delete pToBeDeleted;
        pToBeDeleted = nullptr;
    }
}

void PrintListInReversedOrder_1::printListReversingly_Iteratively(ListNode* pHead) {
    if (pHead == nullptr) {
        return;
    }
    
    std::stack<ListNode*> nodes;
    ListNode* pNode = pHead;
    while (pNode != nullptr) {
        nodes.push(pNode);
        pNode = pNode->m_pNext;
    }
    
    while (!nodes.empty()) {
        ListNode* node = nodes.top();
        printf("%d\t", node->m_nValue);
        nodes.pop();
    }
}

void PrintListInReversedOrder_1::printListReversingly_Recursively(ListNode* pHead) {
    if (pHead != nullptr) {
        if (pHead->m_pNext != nullptr) {
            printListReversingly_Recursively(pHead->m_pNext);
        }
        
        printf("%d\t", pHead->m_nValue);
    }
}

// 测试代码
void PrintListInReversedOrder_1::Test(ListNode* pHead) {
    PrintList(pHead);
    printListReversingly_Iteratively(pHead);
    printf("\n");
    printListReversingly_Recursively(pHead);
}

// 1->2->3->4->5
void PrintListInReversedOrder_1::Test1() {
    printf("Test1 begins.\n");

    ListNode* pNode1 = CreateListNode(1);
    ListNode* pNode2 = CreateListNode(2);
    ListNode* pNode3 = CreateListNode(3);
    ListNode* pNode4 = CreateListNode(4);
    ListNode* pNode5 = CreateListNode(5);

    ConnectListNodes(pNode1, pNode2);
    ConnectListNodes(pNode2, pNode3);
    ConnectListNodes(pNode3, pNode4);
    ConnectListNodes(pNode4, pNode5);

    Test(pNode1);

    DestroyList(pNode1);
}

// 只有一个结点的链表: 1
void PrintListInReversedOrder_1::Test2() {
    printf("\nTest2 begins.\n");

    ListNode* pNode1 = CreateListNode(1);

    Test(pNode1);

    DestroyList(pNode1);
}

// 空链表
void PrintListInReversedOrder_1::Test3() {
    printf("\nTest3 begins.\n");

    Test(nullptr);
}

void PrintListInReversedOrder_1::Test() {
    Test1();
    Test2();
    Test3();
}

/**
 * Definition for singly-linked list.
 * struct ListNode {
 *      int val;
 *      ListNode* next;
 *      ListNode(int x) : val(x), next(NULL) {}
 * };
 */

class Solution {
    struct ListNode {
        int val;
        ListNode* next;
        ListNode(int x) : val(x), next(NULL) {}
    };
    
public:
    vector<int> reversePrint(ListNode* head) {
//        // 1. Iteratively
//        vector<int> b;
//        stack<ListNode*> nodeStack;
//        ListNode* pNode = head;
//        while (pNode != nullptr) {
//            nodeStack.push(pNode);
//            pNode = pNode->next;
//        }
//
//        while (!nodeStack.empty()) {
//            ListNode* node = nodeStack.top();
//            b.push_back(node->val);
//            nodeStack.pop();
//        }
//
//        return b;
        
        // 2. Recursively
        vector<int> b;
        reversePrint(head, &b);
        return b;
    }
    
    void reversePrint(ListNode* head, vector<int>* b) {
        if (head != nullptr) {
            if (head->next != nullptr) {
                reversePrint(head->next, b);
            }
            
            (*b).push_back(head->val);
        }
    }
};
