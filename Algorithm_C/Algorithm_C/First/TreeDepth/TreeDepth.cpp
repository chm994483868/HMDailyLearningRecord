//
//  TreeDepth.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/24.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "TreeDepth.hpp"

namespace TreeDepth {

struct BinaryTreeNode {
    int m_nValue;
    BinaryTreeNode* m_pLeft;
    BinaryTreeNode* m_pRight;
};

int treeDepth(const BinaryTreeNode* pRoot) {
    if (pRoot == nullptr) {
        return 0;
    }
    
    int nLeft = treeDepth(pRoot->m_pLeft);
    int nRight = treeDepth(pRoot->m_pRight);
    
    return (nLeft > nRight) ? (nLeft + 1) : (nRight + 1);
}

}
