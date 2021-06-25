//
//  DeleteDuplicatedNode.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/15.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "DeleteDuplicatedNode.hpp"

// 1. 重复的节点在头
// 2. 中间
// 3. 尾
// 4. 没有重复的节点
// 5. 都是重复的节点

void deleteDuplication(ListNode** pHead) {
    if (pHead == nullptr || *pHead == nullptr)
        return;
    
    ListNode* pPreNode = nullptr;
    ListNode* pNode = *pHead;
    
    while (pNode != nullptr) {
        ListNode* pNext = pNode->m_pNext;
        bool needDelete = false;
        
        if (pNext != nullptr && pNext->m_nValue == pNode->m_nValue)
            needDelete = true;
        
        if (!needDelete) {
            pPreNode = pNode;
            pNode = pNode->m_pNext;
        } else {
            int value = pNode->m_nValue;
            ListNode* pToBeDel = pNode;
            
            while (pToBeDel != nullptr && pToBeDel->m_nValue == value) {
                pNext = pToBeDel->m_pNext;
                
                delete pToBeDel;
                pToBeDel = nullptr;
                
                pToBeDel = pNext;
            }
            
            if (pPreNode == nullptr)
                *pHead = pNext;
            else
                pPreNode->m_pNext = pNext;
            
            pNode = pNext;
        }
    }
}
