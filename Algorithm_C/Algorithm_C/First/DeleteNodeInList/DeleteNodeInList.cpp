//
//  DeleteNodeInList.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/15.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "DeleteNodeInList.hpp"
// 基于一个假设：要删除的节点的确在链表中。
// 需要 O(n) 的时间才能判断链表中是否包含某一个节点
void deleteNode(ListNode** pListHead, ListNode* pToBeDeleted) {
    if (*pListHead == nullptr || pListHead == nullptr || pToBeDeleted == nullptr)
        return;
    
    // 1. 要删除的节点不是尾节点
    if (pToBeDeleted->m_pNext != nullptr) {
        ListNode *pNext = pToBeDeleted->m_pNext;
        pToBeDeleted->m_nValue = pNext->m_nValue;
        pToBeDeleted->m_pNext = pNext->m_pNext;
        
        delete pNext;
        pNext = nullptr;
    }
    // 2. 链表只有一个节点，要删除的节点即是头节点也是尾节点
    else if (*pListHead == pToBeDeleted) {
        delete pToBeDeleted;
        pToBeDeleted = nullptr;
        *pListHead = nullptr;
    }
    // 3. 链表有多个节点，要删除的节点是尾节点，需要找到要删除节点的前一个节点
    else {
        ListNode* pNode = *pListHead;
        while (pNode->m_pNext != pToBeDeleted) {
            pNode = pNode->m_pNext;
        }
        
        pNode->m_pNext = nullptr;
        delete pToBeDeleted;
        pToBeDeleted = nullptr;
    }
}
