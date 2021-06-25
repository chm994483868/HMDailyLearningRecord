//
//  EntryNodeInListLoop.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/16.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "EntryNodeInListLoop.hpp"

ListNode* meetingNode(ListNode* pHead) {
    if (pHead == nullptr)
        return nullptr;
    
    ListNode* pSlow = pHead->m_pNext;
    // 1. 此处表示链表就一个节点，没有环存在可以直接返回了
    if (pSlow == nullptr)
        return nullptr;
    // 2. 此处 pFast 是 pSlow 的下一个节点，可以理解为
    //    此时，指针1 和指针2 同时从头节点出发，指针 1 向前移动一步，指针 2 向前移动了两步
    ListNode* pFast = pSlow->m_pNext;
    while (pFast != nullptr && pSlow != nullptr) {
        if (pFast == pSlow)
            return pFast;
        
        // 3. pSlow 一次向前移动一步
        pSlow = pSlow->m_pNext;
        // 4. pFast 一次向前移动两步
        pFast = pFast->m_pNext;
        if (pFast != nullptr)
            pFast = pFast->m_pNext;
    }
    
    return nullptr;
}

ListNode* entryNodeOfLoop(ListNode* pHead) {
    // 1. 找到环中的一个节点
    ListNode* pMeetingNode = meetingNode(pHead);
    if (pMeetingNode == nullptr)
        return nullptr;
    
    // 2. 得到环中节点的数目
    int nodesInLoop = 1;
    ListNode* pNode1 = pMeetingNode;
    while (pNode1->m_pNext != pMeetingNode) {
        pNode1 = pNode1->m_pNext;
        ++nodesInLoop;
    }
    
    // 3. 先移动 pNode1，次数为环中节点的数目
    pNode1 = pHead;
    for (int i = 0; i < nodesInLoop; ++i)
        pNode1 = pNode1->m_pNext;
    
    // 4. 再移动 pNode1 和 pNode2
    ListNode* pNode2 = pHead;
    while (pNode1 != pNode2) {
        pNode1 = pNode1->m_pNext;
        pNode2 = pNode2->m_pNext;
    }
    
    return pNode1;
}
