//
//  PrintTreeFromTopToBottom.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/17.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "PrintTreeFromTopToBottom.hpp"

void printFromTopToBottom(BinaryTreeNode* pRoot) {
    if (pRoot == nullptr)
        return;
    
    std::deque<BinaryTreeNode *> dequeTreeNode;
    dequeTreeNode.push_back(pRoot);
    
    while (dequeTreeNode.size()) {
        BinaryTreeNode* pNode = dequeTreeNode.front();
        dequeTreeNode.pop_front();
        
        printf("%d ", pNode->m_nValue);
        
        if (pNode->m_pLeft != nullptr)
            dequeTreeNode.push_back(pNode->m_pLeft);
        
        if (pNode->m_pRight != nullptr) {
            dequeTreeNode.push_back(pNode->m_pRight);
        }
    }
}

void printFromTopToBottom_Review(BinaryTreeNode* pRoot) {
    if (pRoot == nullptr)
        return;
    
    std::deque<BinaryTreeNode*> dequeTreeNode;
    dequeTreeNode.push_back(pRoot);
    
    while (dequeTreeNode.size()) {
        BinaryTreeNode* pNode = dequeTreeNode.front();
        dequeTreeNode.pop_front();
        
        printf("%d ", pNode->m_nValue);
        
        if (pNode->m_pLeft)
            dequeTreeNode.push_back(pNode->m_pLeft);
        
        if (pNode->m_pRight)
            dequeTreeNode.push_back(pNode->m_pRight);
    }
}
