//
//  CopyComplexList.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/20.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "CopyComplexList.hpp"

ComplexListNode* CreateNode(int nValue);
void BuildNodes(ComplexListNode* pNode, ComplexListNode* pNext, ComplexListNode* pSibling);
void PrintList(ComplexListNode* pHead);

ComplexListNode* CreateNode(int nValue) {
    ComplexListNode* pNode = new ComplexListNode(nValue);
    pNode->m_nValue = nValue;
    pNode->m_pNext = nullptr;
    pNode->m_pSibling = nullptr;
    
    return pNode;
}

void BuildNodes(ComplexListNode* pNode, ComplexListNode* pNext, ComplexListNode* pSibling) {
    if (pNode != nullptr) {
        pNode->m_pNext = pNext;
        pNode->m_pSibling = pSibling;
    }
}

void PrintList(ComplexListNode* pHead) {
    ComplexListNode* pNode = pHead;
    
    while (pNode != nullptr) {
        printf("The value of this node is: %d.\n", pNode->m_nValue);
        
        if (pNode->m_pSibling != nullptr) {
            printf("The value of its sibling is: %d.\n", pNode->m_pSibling->m_nValue);
        } else {
            printf("This node does not have a sibling.\n");
        }
        
        printf("\n");
        
        pNode = pNode->m_pNext;
    }
}

void CloneNodes(ComplexListNode* pHead);
void ConnectSiblingNodes(ComplexListNode* pHead);
ComplexListNode* ReconnectNodes(ComplexListNode* pHead);

ComplexListNode* Clone(ComplexListNode* pHead) {
    // 1. 复制 node
    CloneNodes(pHead);
    // 2. 复制 sibling
    ConnectSiblingNodes(pHead);
    // 3. 切开
    return ReconnectNodes(pHead);
}

void CloneNodes(ComplexListNode* pHead) {
    ComplexListNode* pNode = pHead;
    while (pNode != nullptr) {
        ComplexListNode* pCloned = new ComplexListNode(pNode->m_nValue);
        // 1. pCloned 的 next 是 pNode 的 next
        pCloned->m_pNext = pNode->m_pNext;
        pCloned->m_pSibling = nullptr;
        
        // 2. pNode 的 next 是 pCloned
        pNode->m_pNext = pCloned;
        
        // 3. 更新 pNode
        // 这里只能用 pCloned->m_pNext
        // 原始的 pNode->m_pNext 已经指向了 pCloned
        pNode = pCloned->m_pNext;
    }
}

void ConnectSiblingNodes(ComplexListNode* pHead) {
    ComplexListNode* pNode = pHead;
    while (pNode != nullptr) {
        ComplexListNode* pCloned = pNode->m_pNext;
        
        if (pNode->m_pSibling != nullptr) {
            pCloned->m_pSibling = pNode->m_pSibling->m_pNext;
        }
        
        // 更新 pNode，联合上面的一次 m_pNext 和下面的这次 m_pNext
        // pNode 更新了两步，到下一个 pNode
        pNode = pCloned->m_pNext;
    }
}

ComplexListNode* ReconnectNodes(ComplexListNode* pHead) {
    ComplexListNode* pNode = pHead;
    ComplexListNode* pClonedHead = nullptr;
    ComplexListNode* pClonedNode = nullptr;
    
    if (pNode != nullptr) {
        pClonedHead = pClonedNode = pNode->m_pNext;
        pNode->m_pNext = pClonedNode->m_pNext;
        pNode = pNode->m_pNext;
    }
    
    // 更新下个节点
    while (pNode != nullptr) {
        pClonedNode->m_pNext = pNode->m_pNext;
        pClonedNode = pClonedNode->m_pNext;
        
        pNode->m_pNext = pClonedNode->m_pNext;
        pNode = pNode->m_pNext;
    }
    
    return pClonedHead;
}

void CloneNodes_Review(ComplexListNode* pHead) {
    ComplexListNode* pNode = pHead;
    while (pNode != nullptr) {
        ComplexListNode* pCloned = new ComplexListNode(pNode->m_nValue);
        pCloned->m_nValue = pNode->m_nValue;
        pCloned->m_pNext = pNode->m_pNext;
        pCloned->m_pSibling = nullptr;
        
        pNode->m_pNext = pCloned;
        pNode = pCloned->m_pNext;
    }
}

void ConnectSiblingNodes_Review(ComplexListNode* pHead) {
    ComplexListNode* pNode = pHead;
    while (pNode != nullptr) {
        ComplexListNode* pCloned = pNode->m_pNext;
        
        if (pNode->m_pSibling != nullptr) {
            if (pCloned != nullptr) {
                pCloned->m_pSibling = pNode->m_pSibling->m_pNext;
            }
            
            pNode = pCloned->m_pNext;
        }
    }
}

ComplexListNode* ReconnectNodes_Review(ComplexListNode* pHead) {
    ComplexListNode* pNode = pHead;
    ComplexListNode* pClonedHead = nullptr;
    ComplexListNode* pClonedNode = nullptr;
    
    if (pNode != nullptr) {
        pClonedHead = pClonedNode = pNode->m_pNext;
        pNode->m_pNext = pClonedNode->m_pNext;
        pNode = pNode->m_pNext;
    }
    
    while (pNode != nullptr) {
        pClonedNode->m_pNext = pNode->m_pNext;
        pClonedNode = pClonedNode->m_pNext;
        
        pNode->m_pNext = pClonedNode->m_pNext;
        pNode = pNode->m_pNext;
    }
    
    return pClonedHead;
}

void Test_ComplexListNode(const char* testName, ComplexListNode* pHead)
{
    if(testName != nullptr)
        printf("%s begins:\n", testName);

    printf("The original list is:\n");
    PrintList(pHead);

    ComplexListNode* pClonedHead = Clone(pHead);

    printf("The cloned list is:\n");
    PrintList(pClonedHead);
}

//          -----------------
//         \|/              |
//  1-------2-------3-------4-------5
//  |       |      /|\             /|\
//  --------+--------               |
//          -------------------------
void Test1_ComplexListNode()
{
    ComplexListNode* pNode1 = CreateNode(1);
    ComplexListNode* pNode2 = CreateNode(2);
    ComplexListNode* pNode3 = CreateNode(3);
    ComplexListNode* pNode4 = CreateNode(4);
    ComplexListNode* pNode5 = CreateNode(5);

    BuildNodes(pNode1, pNode2, pNode3);
    BuildNodes(pNode2, pNode3, pNode5);
    BuildNodes(pNode3, pNode4, nullptr);
    BuildNodes(pNode4, pNode5, pNode2);

    Test_ComplexListNode("Test1", pNode1);
}

// m_pSibling÷∏œÚΩ·µ„◊‘…Ì
//          -----------------
//         \|/              |
//  1-------2-------3-------4-------5
//         |       | /|\           /|\
//         |       | --             |
//         |------------------------|
void Test2_ComplexListNode()
{
    ComplexListNode* pNode1 = CreateNode(1);
    ComplexListNode* pNode2 = CreateNode(2);
    ComplexListNode* pNode3 = CreateNode(3);
    ComplexListNode* pNode4 = CreateNode(4);
    ComplexListNode* pNode5 = CreateNode(5);

    BuildNodes(pNode1, pNode2, nullptr);
    BuildNodes(pNode2, pNode3, pNode5);
    BuildNodes(pNode3, pNode4, pNode3);
    BuildNodes(pNode4, pNode5, pNode2);

    Test_ComplexListNode("Test2", pNode1);
}

// m_pSibling–Œ≥…ª∑
//          -----------------
//         \|/              |
//  1-------2-------3-------4-------5
//          |              /|\
//          |               |
//          |---------------|
void Test3_ComplexListNode()
{
    ComplexListNode* pNode1 = CreateNode(1);
    ComplexListNode* pNode2 = CreateNode(2);
    ComplexListNode* pNode3 = CreateNode(3);
    ComplexListNode* pNode4 = CreateNode(4);
    ComplexListNode* pNode5 = CreateNode(5);

    BuildNodes(pNode1, pNode2, nullptr);
    BuildNodes(pNode2, pNode3, pNode4);
    BuildNodes(pNode3, pNode4, nullptr);
    BuildNodes(pNode4, pNode5, pNode2);

    Test_ComplexListNode("Test3", pNode1);
}

// ÷ª”–“ª∏ˆΩ·µ„
void Test4_ComplexListNode()
{
    ComplexListNode* pNode1 = CreateNode(1);
    BuildNodes(pNode1, nullptr, pNode1);

    Test_ComplexListNode("Test4", pNode1);
}

// ¬≥∞Ù–‘≤‚ ‘
void Test5_ComplexListNode()
{
    Test_ComplexListNode("Test5", nullptr);
}

void startTest_ComplexListNode_Clone() {
    Test1_ComplexListNode();
    Test2_ComplexListNode();
    Test3_ComplexListNode();
    Test4_ComplexListNode();
    Test5_ComplexListNode();
}

void cloneNodes_Review(ComplexListNode* pHead) {
    ComplexListNode* pNode = pHead;
    while (pNode != nullptr) {
        ComplexListNode* pCloned = new ComplexListNode(pHead->m_nValue);
        pCloned->m_nValue = pNode->m_nValue;
        pCloned->m_pNext = pNode->m_pNext;
        pCloned->m_pSibling = nullptr;
        
        pNode->m_pNext = pCloned;
        pNode = pCloned->m_pNext;
    }
}

void connectSiblingNodes_Review(ComplexListNode* pHead) {
    ComplexListNode* pNode = pHead;
    while (pNode != nullptr) {
        ComplexListNode* pCloned = pNode->m_pNext;
        
        if (pNode->m_pSibling != nullptr) {
            pCloned->m_pSibling = pNode->m_pSibling->m_pNext;
        }
        
        pNode = pCloned->m_pNext;
    }
}

ComplexListNode* reconnectNodes_Review(ComplexListNode* pHead) {
    ComplexListNode* pNode = pHead;
    ComplexListNode* pClonedHead = nullptr;
    ComplexListNode* pClonedNode = nullptr;
    
    if (pNode != nullptr) {
        pClonedHead = pClonedNode = pNode->m_pNext;
        pNode->m_pNext = pClonedNode->m_pNext;
        pNode = pNode->m_pNext;
    }
    
    while (pNode != nullptr) {
        pClonedNode->m_pNext = pNode->m_pNext;
        pClonedNode = pClonedNode->m_pNext;
        
        pNode->m_pNext = pClonedNode->m_pNext;
        pNode = pNode->m_pNext;
    }
    
    return pClonedHead;
}

ComplexListNode* clone_Review(ComplexListNode* pHead) {
    cloneNodes_Review(pHead);
    connectSiblingNodes_Review(pHead);
    return reconnectNodes_Review(pHead);
}
