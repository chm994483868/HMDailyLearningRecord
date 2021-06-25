//
//  FirstCommonNodesInLists.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/24.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "FirstCommonNodesInLists.hpp"

struct ListNode {
    int m_nValue;
    ListNode* m_pNext;
};

unsigned int getListLength(ListNode* pHead);

ListNode* findFirstCommonNode(ListNode* pHead1, ListNode* pHead2) {
    unsigned int nLength1 = getListLength(pHead1);
    unsigned int nLength2 = getListLength(pHead2);
    int nLengthDif = nLength1 - nLength2; // 此时 nLengthDif 可能为正值或者负值
    
    ListNode* pListHeadLong = pHead1;
    ListNode* pListHeadShort = pHead2;
    
    if (nLength2 > nLength1) {
        pListHeadLong = pHead2;
        pListHeadShort = pHead1;
        nLengthDif = nLength2 - nLength1;
    }
    
    for (int i = 0; i < nLengthDif; ++i) {
        pListHeadLong = pListHeadLong->m_pNext;
    }
    
    while ((pListHeadLong != nullptr) && (pListHeadShort != nullptr) && (pListHeadLong != pListHeadShort)) {
        pListHeadLong = pListHeadLong->m_pNext;
        pListHeadShort = pListHeadShort->m_pNext;
    }
    
    ListNode* pFirstCommonNode = pListHeadLong;
    
    return pFirstCommonNode;
}

unsigned int getListLength(ListNode* pHead) {
    unsigned int nLength = 0;
    ListNode* pNode = pHead;
    while (pNode != nullptr) {
        ++nLength;
        pNode = pNode->m_pNext;
    }
    
    return nLength;
}
