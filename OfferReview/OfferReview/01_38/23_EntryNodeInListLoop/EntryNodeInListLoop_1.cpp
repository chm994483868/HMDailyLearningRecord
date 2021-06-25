//
//  EntryNodeInListLoop_1.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/4.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "EntryNodeInListLoop_1.hpp"

ListNode* EntryNodeInListLoop_1::meetingNode(ListNode* pHead) {
    if (pHead == nullptr) {
        return nullptr;
    }
    
    ListNode* pSlow = pHead->m_pNext;
    if (pSlow == nullptr) {
        return nullptr;
    }
    
    ListNode* pFast = pSlow->m_pNext;
    while (pFast != nullptr && pSlow != nullptr) {
        
        // 如 pFast 等于 pSlow 即 找到了环中的一个节点
        if (pFast == pSlow) {
            return pFast;
        }
        
        // pSlow 前进一步
        pSlow = pSlow->m_pNext;
        
        // pFast 前进两步
        pFast = pFast->m_pNext;
        if (pFast != nullptr) {
            pFast = pFast->m_pNext;
        }
    }
    
    return nullptr;
}

ListNode* EntryNodeInListLoop_1::entryNodeOfLoop(ListNode* pHead) {
    if (pHead == nullptr) {
        return nullptr;
    }
    
    ListNode* pMeetingNode = meetingNode(pHead);
    if (pMeetingNode == nullptr) {
        return nullptr;
    }
    
    // 统计环中的节点个数
    unsigned int nNodesOfLoop = 1;
    ListNode* pNode1 = pMeetingNode;
    while (pNode1->m_pNext != pMeetingNode) {
        pNode1 = pNode1->m_pNext;
        ++nNodesOfLoop;
    }
    
    // 前进 nNodesOfLoop 步
    pNode1 = pHead;
    for (unsigned int i = 0; i < nNodesOfLoop; ++i) {
        pNode1 = pNode1->m_pNext;
    }
    
    // 一起前进，当 pNode1 等于 pNode2 时，表示已经到了入口节点
    ListNode* pNode2 = pHead;
    while (pNode1 != pNode2) {
        pNode1 = pNode1->m_pNext;
        pNode2 = pNode2->m_pNext;
    }
    
    return pNode1;
}

// 测试代码
void EntryNodeInListLoop_1::Test(char* testName, ListNode* pHead, ListNode* entryNode) {
    if(testName != nullptr)
        printf("%s begins: ", testName);
    
    if(entryNodeOfLoop(pHead) == entryNode)
        printf("Passed.\n");
    else
        printf("FAILED.\n");
}

// A list has a node, without a loop
void EntryNodeInListLoop_1::Test1() {
    ListNode* pNode1 = CreateListNode(1);

    Test("Test1", pNode1, nullptr);

    DestroyList(pNode1);
}

// A list has a node, with a loop
void EntryNodeInListLoop_1::Test2() {
    ListNode* pNode1 = CreateListNode(1);
    ConnectListNodes(pNode1, pNode1);

    Test("Test2", pNode1, pNode1);

    delete pNode1;
    pNode1 = nullptr;
}

// A list has multiple nodes, with a loop
void EntryNodeInListLoop_1::Test3() {
    ListNode* pNode1 = CreateListNode(1);
    ListNode* pNode2 = CreateListNode(2);
    ListNode* pNode3 = CreateListNode(3);
    ListNode* pNode4 = CreateListNode(4);
    ListNode* pNode5 = CreateListNode(5);

    ConnectListNodes(pNode1, pNode2);
    ConnectListNodes(pNode2, pNode3);
    ConnectListNodes(pNode3, pNode4);
    ConnectListNodes(pNode4, pNode5);
    ConnectListNodes(pNode5, pNode3);

    Test("Test3", pNode1, pNode3);

    delete pNode1;
    pNode1 = nullptr;
    delete pNode2;
    pNode2 = nullptr;
    delete pNode3;
    pNode3 = nullptr;
    delete pNode4;
    pNode4 = nullptr;
    delete pNode5;
    pNode5 = nullptr;
}

// A list has multiple nodes, with a loop
void EntryNodeInListLoop_1::Test4() {
    ListNode* pNode1 = CreateListNode(1);
    ListNode* pNode2 = CreateListNode(2);
    ListNode* pNode3 = CreateListNode(3);
    ListNode* pNode4 = CreateListNode(4);
    ListNode* pNode5 = CreateListNode(5);

    ConnectListNodes(pNode1, pNode2);
    ConnectListNodes(pNode2, pNode3);
    ConnectListNodes(pNode3, pNode4);
    ConnectListNodes(pNode4, pNode5);
    ConnectListNodes(pNode5, pNode1);

    Test("Test4", pNode1, pNode1);

    delete pNode1;
    pNode1 = nullptr;
    delete pNode2;
    pNode2 = nullptr;
    delete pNode3;
    pNode3 = nullptr;
    delete pNode4;
    pNode4 = nullptr;
    delete pNode5;
    pNode5 = nullptr;
}

// A list has multiple nodes, with a loop
void EntryNodeInListLoop_1::Test5() {
    ListNode* pNode1 = CreateListNode(1);
    ListNode* pNode2 = CreateListNode(2);
    ListNode* pNode3 = CreateListNode(3);
    ListNode* pNode4 = CreateListNode(4);
    ListNode* pNode5 = CreateListNode(5);

    ConnectListNodes(pNode1, pNode2);
    ConnectListNodes(pNode2, pNode3);
    ConnectListNodes(pNode3, pNode4);
    ConnectListNodes(pNode4, pNode5);
    ConnectListNodes(pNode5, pNode5);

    Test("Test5", pNode1, pNode5);

    delete pNode1;
    pNode1 = nullptr;
    delete pNode2;
    pNode2 = nullptr;
    delete pNode3;
    pNode3 = nullptr;
    delete pNode4;
    pNode4 = nullptr;
    delete pNode5;
    pNode5 = nullptr;
}

// A list has multiple nodes, without a loop
void EntryNodeInListLoop_1::Test6() {
    ListNode* pNode1 = CreateListNode(1);
    ListNode* pNode2 = CreateListNode(2);
    ListNode* pNode3 = CreateListNode(3);
    ListNode* pNode4 = CreateListNode(4);
    ListNode* pNode5 = CreateListNode(5);

    ConnectListNodes(pNode1, pNode2);
    ConnectListNodes(pNode2, pNode3);
    ConnectListNodes(pNode3, pNode4);
    ConnectListNodes(pNode4, pNode5);

    Test("Test6", pNode1, nullptr);

    DestroyList(pNode1);
}

// Empty list
void EntryNodeInListLoop_1::Test7() {
    Test("Test7", nullptr, nullptr);
}

void EntryNodeInListLoop_1::Test() {
    Test1();
    Test2();
    Test3();
    Test4();
    Test5();
    Test6();
    Test7();
}
