//
//  NextNodeInBinaryTrees_2.cpp
//  OfferReview
//
//  Created by CHM on 2021/2/21.
//  Copyright © 2021 CHM. All rights reserved.
//

#include "NextNodeInBinaryTrees_2.hpp"

NextNodeInBinaryTrees_2::BinaryTreeNode* NextNodeInBinaryTrees_2::getNext(BinaryTreeNode* pNode) {
    if (pNode == nullptr) {
        return nullptr;
    }
    
    BinaryTreeNode* pNext = nullptr;
    if (pNode->m_pRight != nullptr) {
        BinaryTreeNode* pRight = pNode->m_pRight;
        while (pRight->m_pLeft != nullptr) {
            pRight = pRight->m_pLeft;
        }
        pNext = pRight;
    } else if (pNode->m_pParent != nullptr) {
        BinaryTreeNode* pCurrent = pNode;
        BinaryTreeNode* pParent = pNode->m_pParent;
        
        while (pParent != nullptr && pCurrent == pParent->m_pRight) {
            pCurrent = pParent;
            pParent = pParent->m_pParent;
        }
        
        pNext = pParent;
    }
    
    return pNext;
}
