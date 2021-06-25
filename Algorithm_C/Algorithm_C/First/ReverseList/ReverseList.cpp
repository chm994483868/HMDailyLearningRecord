//
//  ReverseList.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/16.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "ReverseList.hpp"

ListNode* reverseList(ListNode* pHead) {
    ListNode* pReversedHead = nullptr;
    ListNode* pPrev = nullptr;
    ListNode* pNode = pHead;
    
    while (pNode != nullptr) {
        ListNode* pNext = pNode->m_pNext;
        
        // 1. 如果 pNext 是 nullptr 即表示当前链表到了尾节点
        // 而当前链表的尾节点定是翻转后链表的头节点，
        // 所以下面把 pNode 直接赋值给 pReversedHead
        if (pNext == nullptr)
            pReversedHead = pNode;
        
        // 2. 这里 pPrev 用来保存前一个节点，如果 pNode 是头节点时，pPrev 是 nllptr
        //    然后从下次开始 pPrev 每次记录前一个节点，
        pNode->m_pNext = pPrev;
        pPrev = pNode;
        
        // 3. pNext 记录下个节点，本轮将要结束时，更新 pNode
        pNode = pNext;
    }
    
    return pReversedHead;
}
