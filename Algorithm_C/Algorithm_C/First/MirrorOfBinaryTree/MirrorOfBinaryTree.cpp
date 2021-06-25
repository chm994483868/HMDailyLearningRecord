//
//  MirrorOfBinaryTree.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/17.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "MirrorOfBinaryTree.hpp"

// 递归
void mirrorRecursively(BinaryTreeNode* pNode) {
    if (pNode == nullptr || (pNode->m_pLeft == nullptr && pNode->m_pRight == nullptr))
        return;
    
    BinaryTreeNode* temp = pNode->m_pLeft;
    pNode->m_pLeft = pNode->m_pRight;
    pNode->m_pRight = temp;
    
    if (pNode->m_pLeft == nullptr)
        mirrorRecursively(pNode->m_pLeft);
    
    if (pNode->m_pRight == nullptr)
        mirrorRecursively(pNode->m_pRight);
}

// 迭代（用栈或者队列）
void mirrorIteratively(BinaryTreeNode* pRoot) {
    if (pRoot == nullptr)
        return;
    
    // 1. 创建栈用来存节点
    std::stack<BinaryTreeNode *> stackTreeNode;
    // 2. 把根节点放进栈中
    stackTreeNode.push(pRoot);
    
    // 3. 如果栈不为空就代表还存在非叶节点，还需要继续反转节点
    while (stackTreeNode.size() > 0) {
        // 4. 取出栈顶节点
        BinaryTreeNode *pNode = stackTreeNode.top();
        stackTreeNode.pop();
        
        BinaryTreeNode* pTemp = pNode->m_pLeft;
        pNode->m_pLeft = pNode->m_pRight;
        pNode->m_pRight = pTemp;
        
        if (pNode->m_pLeft != nullptr)
            stackTreeNode.push(pNode->m_pLeft);
        
        if (pNode->m_pRight != nullptr)
            stackTreeNode.push(pNode->m_pRight);
    }
}

void mirrorRecursively_Review(BinaryTreeNode* pNode) {
    if (pNode == nullptr || (pNode->m_pLeft == nullptr && pNode->m_pRight == nullptr))
        return;
    
    BinaryTreeNode* pTemp = pNode->m_pLeft;
    pNode->m_pLeft = pNode->m_pRight;
    pNode->m_pRight = pTemp;
    
    if (pNode->m_pLeft != nullptr)
        mirrorRecursively_Review(pNode->m_pLeft);
    
    if (pNode->m_pRight != nullptr)
        mirrorRecursively_Review(pNode->m_pLeft);
}

void mirrorIteratively_Review(BinaryTreeNode* pRoot) {
    if (pRoot == nullptr)
        return;
    
    std::stack<BinaryTreeNode *> stackTreeNode;
    stackTreeNode.push(pRoot);
    
    // 或者用 !stackTreeNode.empty()
    while (stackTreeNode.size() > 0) {
        BinaryTreeNode* pNode = stackTreeNode.top();
        stackTreeNode.pop();
        
        BinaryTreeNode *pTemp = pNode->m_pLeft;
        pNode->m_pLeft = pNode->m_pRight;
        pNode->m_pRight = pTemp;
        
        if (pNode->m_pLeft != nullptr)
            stackTreeNode.push(pNode->m_pLeft);
        
        if (pNode->m_pRight != nullptr)
            stackTreeNode.push(pNode->m_pRight);
    }
}
