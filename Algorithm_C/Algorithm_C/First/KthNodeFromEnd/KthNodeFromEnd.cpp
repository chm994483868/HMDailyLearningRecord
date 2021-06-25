//
//  KthNodeFromEnd.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/16.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "KthNodeFromEnd.hpp"

ListNode* findKthToTail(ListNode* pListHead, unsigned int k) {
    if (pListHead == nullptr || k <= 0)
        return nullptr;
    
    ListNode* pAhead = pListHead;
    for (unsigned int i = 0; i < k - 1; ++i) {
        if (pAhead->m_pNext != nullptr)
            pAhead = pAhead->m_pNext;
        else
            return nullptr;
    }
    
    ListNode* pBehind = pListHead;
    while (pAhead->m_pNext != nullptr) {
        pAhead = pAhead->m_pNext;
        pBehind = pBehind->m_pNext;
    }
    
    return pBehind;
}
