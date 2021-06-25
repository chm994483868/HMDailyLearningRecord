//
//  FirstCommonNodesInLists.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "FirstCommonNodesInLists.hpp"

// 如果是单链表，那么两个链表相交后后面的所有节点就是共用的了。
// 先分别计算两个链表的长度，然后准备两个指针从两个链表头节点开始向后遍历，
// 首先指向长链表头节点的指针前进长的链表长出来的部分，然后两个指针一起前进，直到两个指针指向相同时，
// 即是两个链表的第一个交点
ListNode* FirstCommonNodesInLists::findFirstCommonNode(ListNode* pHead1, ListNode* pHead2) {
    // 得到两个链表的长度
    unsigned int nLength1 = getListLength(pHead1);
    unsigned int nLength2 = getListLength(pHead2);
    int nLengthDif = nLength1 - nLength2;
    
    // 指向两个链表头节点的指针
    ListNode* pListHeadLong = pHead1;
    ListNode* pListHeadShort = pHead2;
    
    // 确保 pListHeadLong 指向长链表头节点，
    // pListHeadShort 指向短链表头节点
    if (nLength2 > nLength1) {
        pListHeadLong = pHead2;
        pListHeadShort = pHead1;
        nLengthDif = nLength2 - nLength1;
    }
    
    // 先在长链表上走几步，再同时在两个链表上遍历
    for (int i = 0; i < nLengthDif; ++i) {
        pListHeadLong = pListHeadLong->m_pNext;
    }
    
    while ((pListHeadLong != nullptr) && (pListHeadShort != nullptr) && (pListHeadLong != pListHeadShort)) {
        pListHeadLong = pListHeadLong->m_pNext;
        pListHeadShort = pListHeadShort->m_pNext;
    }
    
    // 得到第一个公共节点
    ListNode* pFirstCommonNode = pListHeadLong;
    
    return pFirstCommonNode;
}

// 统计链表节点个数
unsigned int FirstCommonNodesInLists::getListLength(ListNode* pHead) {
    unsigned int nLength = 0;
    ListNode* pNode = pHead;
    while(pNode != nullptr) {
        ++nLength;
        pNode = pNode->m_pNext;
    }
    
    return nLength;
}

