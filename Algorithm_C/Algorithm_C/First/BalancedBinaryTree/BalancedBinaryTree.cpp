//
//  BalancedBinaryTree.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/24.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "BalancedBinaryTree.hpp"

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
    int nright = treeDepth(pRoot->m_pRight);
    
    return (nLeft > nright)? (nLeft + 1): (nright + 1);
}

bool isBalanced_Solution1(const BinaryTreeNode* pRoot) {
    if (pRoot == nullptr) {
        return true;
    }
    
    int left = treeDepth(pRoot->m_pLeft);
    int right = treeDepth(pRoot->m_pRight);
    int diff = left - right;
    if (diff > 1 || diff < -1) {
        return false;
    }
    
    return isBalanced_Solution1(pRoot->m_pLeft) && isBalanced_Solution1(pRoot->m_pRight);
}

// int* pDepth 指针参数，传递树的深度
bool isBalanced(const BinaryTreeNode* pRoot, int* pDepth);
bool isBalanced_Solution2(const BinaryTreeNode* pRoot) {
    int depth = 0;
    return isBalanced(pRoot, &depth);
}

bool isBalanced(const BinaryTreeNode* pRoot, int* pDepth) {
    if (pRoot == nullptr) {
        *pDepth = 0;
        return true;
    }
    
    int left, right;
    // 先判断左右子树各自是不是平衡二叉树，然后比较两者的深度，判断是不不是平衡二叉树
    //
    if (isBalanced(pRoot->m_pLeft, &left) && isBalanced(pRoot->m_pRight, &right)) {
        int diff = left - right;
        if (diff <= 1 && diff >= -1) {
            *pDepth = 1 + (left > right? left: right);
            return true;
        }
    }
    
    return false;
}
