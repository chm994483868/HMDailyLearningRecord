//
//  SubstructureInTree.cpp
//  Algorithm_C
//
//  Created by HM C on 2020/7/16.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "SubstructureInTree.hpp"

bool doesTree1HaveTree2(BinaryTreeNode* pRoot1, BinaryTreeNode* pRoot2);
bool equal(double num1, double num2);

bool hasSubtree(BinaryTreeNode* pRoot1, BinaryTreeNode* pRoot2) {
    bool result = false;
    if (pRoot1 != nullptr && pRoot2 != nullptr) {
        if (equal(pRoot1->m_dbValue, pRoot2->m_dbValue))
            result = doesTree1HaveTree2(pRoot1, pRoot2);
        
        if (!result)
            result = hasSubtree(pRoot1->m_pLeft, pRoot2);
        
        if (!result) {
            result = hasSubtree(pRoot1->m_pRight, pRoot2);
        }
    }
    
    return result;
}

bool doesTree1HaveTree2(BinaryTreeNode* pRoot1, BinaryTreeNode* pRoot2) {
    if (pRoot2 == nullptr)
        return true;
    
    if (pRoot1 == nullptr)
        return false;
    
    if (!equal(pRoot1->m_dbValue, pRoot2->m_dbValue))
        return false;
    
    return doesTree1HaveTree2(pRoot1->m_pLeft, pRoot2->m_pLeft) && doesTree1HaveTree2(pRoot1->m_pRight, pRoot2->m_pRight);
}

bool equal(double num1, double num2) {
    if (num1 - num2 > -0.0000001 && num1 - num2 < 0.0000001)
        return true;
    else
        return false;
}
