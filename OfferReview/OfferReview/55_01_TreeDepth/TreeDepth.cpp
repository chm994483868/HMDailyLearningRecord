//
//  TreeDepth.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "TreeDepth.hpp"

int TreeDepth::treeDepth(const BinaryTreeNode* pRoot) {
    if (pRoot == nullptr) {
        return 0;
    }
    
    // 递归左右子树
    int nLeft = treeDepth(pRoot->m_pLeft);
    int nRight = treeDepth(pRoot->m_pRight);
    
    return (nLeft > nRight) ? (nLeft + 1) : (nRight + 1);
}
