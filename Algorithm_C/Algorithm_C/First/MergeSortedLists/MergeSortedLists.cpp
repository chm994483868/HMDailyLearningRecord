//
//  MergeSortedLists.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/16.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "MergeSortedLists.hpp"

ListNode* merge(ListNode* pHead1, ListNode* pHead2) {
    if (pHead1 == nullptr)
        return pHead2;
    else if (pHead2 == nullptr)
        return pHead1;
    
    ListNode* pMergedHead = nullptr;
    
    if (pHead1->m_nValue < pHead2->m_nValue) {
        pMergedHead = pHead1;
        pMergedHead->m_pNext = merge(pHead1->m_pNext, pHead2);
    } else {
        pMergedHead = pHead2;
        pMergedHead->m_pNext = merge(pHead1, pHead2->m_pNext);
    }
    
    return pMergedHead;
}

ListNode* merge_Review(ListNode* pHead1, ListNode* pHead2) {
    if (pHead1 == nullptr)
        return pHead2;
    else if (pHead2 == nullptr)
        return pHead1;
    
    ListNode* pMergeHead = nullptr;
    if (pHead1->m_nValue < pHead2->m_nValue) {
        pMergeHead = pHead1;
        pMergeHead->m_pNext = merge_Review(pHead1->m_pNext, pHead2);
    } else {
        pMergeHead = pHead2;
        pMergeHead->m_pNext = merge_Review(pHead1, pHead2->m_pNext);
    }
    
    return pMergeHead;
}
