//
//  DeleteNodeInList.cpp
//  OfferReview
//
//  Created by CHM on 2020/7/29.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "DeleteNodeInList.hpp"

void DeleteNodeInList::deleteNode(ListNode** pListHead, ListNode* pToBeDeleted) {
    if (pListHead == nullptr || *pListHead == nullptr || pToBeDeleted == nullptr) {
        return;
    }
    
    if (pToBeDeleted->m_pNext != nullptr) {
        ListNode* pNext = pToBeDeleted->m_pNext;
        
        pToBeDeleted->m_nValue = pNext->m_nValue;
        pToBeDeleted->m_pNext = pNext->m_pNext;
        
        delete pNext;
        pNext = nullptr;
    } else if (*pListHead == pToBeDeleted) {
        delete pToBeDeleted;
        pToBeDeleted = nullptr;
        *pListHead = nullptr;
    } else {
        ListNode* pNode = *pListHead;
        while (pNode->m_pNext != pToBeDeleted) {
            pNode = pNode->m_pNext;
        }
        
        pNode->m_pNext = nullptr;
        delete pToBeDeleted;
        pToBeDeleted = nullptr;
    }
}

// 测试代码
void DeleteNodeInList::Test(ListNode* pListHead, ListNode* pNode) {
    printf("The original list is: \n");
    PrintList(pListHead);

    printf("The node to be deleted is: \n");
    PrintListNode(pNode);

    deleteNode(&pListHead, pNode);
    
    printf("The result list is: \n");
    PrintList(pListHead);
}

// 链表中有多个结点，删除中间的结点
void DeleteNodeInList::Test1() {
    ListNode* pNode1 = CreateListNode(1);
    ListNode* pNode2 = CreateListNode(2);
    ListNode* pNode3 = CreateListNode(3);
    ListNode* pNode4 = CreateListNode(4);
    ListNode* pNode5 = CreateListNode(5);

    ConnectListNodes(pNode1, pNode2);
    ConnectListNodes(pNode2, pNode3);
    ConnectListNodes(pNode3, pNode4);
    ConnectListNodes(pNode4, pNode5);

    Test(pNode1, pNode3);

    DestroyList(pNode1);
}

// 链表中有多个结点，删除尾结点
void DeleteNodeInList::Test2() {
    ListNode* pNode1 = CreateListNode(1);
    ListNode* pNode2 = CreateListNode(2);
    ListNode* pNode3 = CreateListNode(3);
    ListNode* pNode4 = CreateListNode(4);
    ListNode* pNode5 = CreateListNode(5);
    
    ConnectListNodes(pNode1, pNode2);
    ConnectListNodes(pNode2, pNode3);
    ConnectListNodes(pNode3, pNode4);
    ConnectListNodes(pNode4, pNode5);
    
    Test(pNode1, pNode5);
    
    DestroyList(pNode1);
}

// 链表中有多个结点，删除头结点
void DeleteNodeInList::Test3() {
    ListNode* pNode1 = CreateListNode(1);
    ListNode* pNode2 = CreateListNode(2);
    ListNode* pNode3 = CreateListNode(3);
    ListNode* pNode4 = CreateListNode(4);
    ListNode* pNode5 = CreateListNode(5);
    
    ConnectListNodes(pNode1, pNode2);
    ConnectListNodes(pNode2, pNode3);
    ConnectListNodes(pNode3, pNode4);
    ConnectListNodes(pNode4, pNode5);
    
    Test(pNode1, pNode1);
    
    DestroyList(pNode1);
}

// 链表中只有一个结点，删除头结点
void DeleteNodeInList::Test4() {
    ListNode* pNode1 = CreateListNode(1);

    Test(pNode1, pNode1);
}

// 链表为空
void DeleteNodeInList::Test5() {
    Test(nullptr, nullptr);
}

void DeleteNodeInList::Test() {
    Test1();
    Test2();
    Test3();
    Test4();
    Test5();
}

/**
 * Definition for singly-linked list.
 * struct ListNode {
 *     int val;
 *     ListNode *next;
 *     ListNode(int x) : val(x), next(NULL) {}
 * };
 */
class Solution {
    struct ListNode {
        int val;
        ListNode *next;
        ListNode(int x) : val(x), next(NULL) {}
    };
    
public:
    ListNode* deleteNode(ListNode* head, int val) {
        if (head == nullptr) {
            return nullptr;
        }
        
        ListNode* pNode = head;
        ListNode* pDeletedNode = nullptr;
        ListNode* pNewHeadNode = head;
        
        if (pNode->val == val) {
            pDeletedNode = pNode;
            pNewHeadNode = pNode->next;
        }
        
        while (pNode->next != nullptr && pNode->next->val != val) {
            pNode = pNode->next;
        }
        
        if (pNode->next != nullptr && pNode->next->val == val) {
            pDeletedNode = pNode->next;
            pNode->next = pNode->next->next;
        }
        
        if (pDeletedNode != nullptr) {
            delete pDeletedNode;
            pDeletedNode = nullptr;
        }
        
        return pNewHeadNode;
    }
};
