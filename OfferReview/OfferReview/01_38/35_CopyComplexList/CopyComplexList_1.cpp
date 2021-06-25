//
//  CopyComplexList_1.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/6.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "CopyComplexList_1.hpp"

// 复制节点，把每个复制的节点直接链接在原节点的后面
void CopyComplexList_1::cloneNodes(ComplexListNode* pHead) {
    ComplexListNode* pNode = pHead;
    while (pNode != nullptr) {
        // 新建节点 m_nValue 是原节点的 m_nValue，m_pSibling 暂时赋 nullptr
        ComplexListNode* pCloned = new ComplexListNode();
        pCloned->m_nValue = pNode->m_nValue;
        pCloned->m_pSibling = nullptr;
        
        // 注意 m_pNext 的赋值，直接链接原节点的下一个节点
        pCloned->m_pNext = pNode->m_pNext;
        
        // 原节点到 m_pNext 指向新复制出的节点
        pNode->m_pNext = pCloned;
        
        // 更新 pNode，继续下个节点的复制
        pNode = pCloned->m_pNext;
    }
}

// 链接复制节点的 m_pSibling 节点
void CopyComplexList_1::connectSiblingNodes(ComplexListNode* pHead) {
    ComplexListNode* pNode = pHead;
    while (pNode != nullptr) {
        if (pNode->m_pSibling != nullptr) {
            // 这里 pNode->m_pNext 就是我们链接在原节点后面的复制的节点，
            // 同样 pNode->m_pSibling 后面紧接着的节点也是 m_pSibling 的复制节点，
            // 所以正是作为复制节点的 m_pSibling
            pNode->m_pNext->m_pSibling = pNode->m_pSibling->m_pNext;
        }
        
        // 更新 pNode 继续进行下个节点的循环
        //（这里之所以不用判断 pNode->m_pNext 是否为 nullptr，是因为每个原节点都进行了复制操作，它的 m_pNext 必不为 nullptr）
        pNode = pNode->m_pNext->m_pNext;
    }
}

// 从复制后的长链表中拆出复制链表
ComplexListNode* CopyComplexList_1::reconnectNodes(ComplexListNode* pHead) {
    // 用于遍历原链表的临时节点
    ComplexListNode* pNode = pHead;
    
    // 准备的记录复制链表的头节点
    ComplexListNode* pClonedHead = nullptr;
    
    // 用于遍历复制链表的临时节点
    ComplexListNode* pClonedNode = nullptr;
    
    if (pNode != nullptr) {
        
        // 复制链表的头节点就是原链表头节点的 m_pNext
        pClonedHead = pClonedNode = pNode->m_pNext;
        
        // pNode 的 m_pNext 还指向它的原始的下一个节点（即原链表复原）
        pNode->m_pNext = pClonedNode->m_pNext;
        
        // 更新 pNode 到下一个节点
        pNode = pNode->m_pNext;
    }
    
    while (pNode != nullptr) {
        
        // 更新复制节点的 m_pNext 指向 pNode 的 m_pNext
        pClonedNode->m_pNext = pNode->m_pNext;
        
        // 更新 pClonedNode
        pClonedNode = pClonedNode->m_pNext;
        
        // 更新 pNode 的 m_pNext 指向它以前的 m_pNext 节点
        pNode->m_pNext = pClonedNode->m_pNext;
        
        // 更新 pNode
        pNode = pNode->m_pNext;
    }
    
    // 返回复制链表的头节点
    return pClonedHead;
}

ComplexListNode* CopyComplexList_1::clone(ComplexListNode* pHead) {
    // 复制节点，（复制的节点直接跟在原节点的后面）
    cloneNodes(pHead);
    // 链接 m_pSibling 节点
    connectSiblingNodes(pHead);
    // 重连，（即拆分原链表和新复制的链表）
    return reconnectNodes(pHead);
}

// 测试代码
void CopyComplexList_1::Test(const char* testName, ComplexListNode* pHead) {
    if(testName != nullptr)
        printf("%s begins:\n", testName);

    printf("The original list is:\n");
    PrintList(pHead);

    ComplexListNode* pClonedHead = clone(pHead);

    printf("The cloned list is:\n");
    PrintList(pClonedHead);
}

//          -----------------
//         \|/              |
//  1-------2-------3-------4-------5
//  |       |      /|\             /|\
//  --------+--------               |
//          -------------------------
void CopyComplexList_1::Test1() {
    ComplexListNode* pNode1 = CreateNode(1);
    ComplexListNode* pNode2 = CreateNode(2);
    ComplexListNode* pNode3 = CreateNode(3);
    ComplexListNode* pNode4 = CreateNode(4);
    ComplexListNode* pNode5 = CreateNode(5);

    BuildNodes(pNode1, pNode2, pNode3);
    BuildNodes(pNode2, pNode3, pNode5);
    BuildNodes(pNode3, pNode4, nullptr);
    BuildNodes(pNode4, pNode5, pNode2);

    Test("Test1", pNode1);
}

// m_pSibling指向结点自身
//          -----------------
//         \|/              |
//  1-------2-------3-------4-------5
//         |       | /|\           /|\
//         |       | --             |
//         |------------------------|
void CopyComplexList_1::Test2() {
    ComplexListNode* pNode1 = CreateNode(1);
    ComplexListNode* pNode2 = CreateNode(2);
    ComplexListNode* pNode3 = CreateNode(3);
    ComplexListNode* pNode4 = CreateNode(4);
    ComplexListNode* pNode5 = CreateNode(5);

    BuildNodes(pNode1, pNode2, nullptr);
    BuildNodes(pNode2, pNode3, pNode5);
    BuildNodes(pNode3, pNode4, pNode3);
    BuildNodes(pNode4, pNode5, pNode2);

    Test("Test2", pNode1);
}

// m_pSibling形成环
//          -----------------
//         \|/              |
//  1-------2-------3-------4-------5
//          |              /|\
//          |               |
//          |---------------|
void CopyComplexList_1::Test3() {
    ComplexListNode* pNode1 = CreateNode(1);
    ComplexListNode* pNode2 = CreateNode(2);
    ComplexListNode* pNode3 = CreateNode(3);
    ComplexListNode* pNode4 = CreateNode(4);
    ComplexListNode* pNode5 = CreateNode(5);

    BuildNodes(pNode1, pNode2, nullptr);
    BuildNodes(pNode2, pNode3, pNode4);
    BuildNodes(pNode3, pNode4, nullptr);
    BuildNodes(pNode4, pNode5, pNode2);

    Test("Test3", pNode1);
}

// 只有一个结点
void CopyComplexList_1::Test4() {
    ComplexListNode* pNode1 = CreateNode(1);
    BuildNodes(pNode1, nullptr, pNode1);

    Test("Test4", pNode1);
}

// 鲁棒性测试
void CopyComplexList_1::Test5() {
    Test("Test5", nullptr);
}

void CopyComplexList_1::Test() {
    Test1();
    Test2();
    Test3();
    Test4();
    Test5();
}
