//
//  SymmetricalBinaryTree.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/17.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "SymmetricalBinaryTree.hpp"

bool isSymmetrical(BinaryTreeNode* pRoot) {
    return isSymmetrical(pRoot, pRoot);
}

bool isSymmetrical(BinaryTreeNode* pRoot1, BinaryTreeNode* pRoot2) {
    if (pRoot1 == nullptr && pRoot2 == nullptr)
        return true;
    
    if (pRoot1 == nullptr || pRoot2== nullptr)
        return false;
    
    if (pRoot1->m_nValue != pRoot2->m_nValue)
        return false;
    
    return isSymmetrical(pRoot1->m_pLeft, pRoot2->m_pRight) && isSymmetrical(pRoot1->m_pRight, pRoot2->m_pLeft);
}
