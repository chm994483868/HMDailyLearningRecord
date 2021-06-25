//
//  BalancedBinaryTree.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "BalancedBinaryTree.hpp"

int BalancedBinaryTree::treeDepth(const BinaryTreeNode* pRoot) {
    if (pRoot == nullptr) {
        return 0;
    }
    
    int nLeft = treeDepth(pRoot->m_pLeft);
    int nRight = treeDepth(pRoot->m_pRight);
    
    return (nLeft > nRight) ? (nLeft + 1): (nRight + 1);
}

// 平衡二叉树：任意结点的左右子树的深度相差不超过 1
// 方法 1，从根节点开始，会判断以每个节点为根节点的二叉树是否是平衡二叉树，存在大量的重复计算
bool BalancedBinaryTree::isBalanced_Solution1(const BinaryTreeNode* pRoot) {
    if (pRoot == nullptr) {
        return true;
    }
    
    int left = treeDepth(pRoot->m_pLeft);
    int right = treeDepth(pRoot->m_pRight);
    
    int diff = left - right;
    
    // 如果超过 1 则不是平衡二叉树
    if (diff > 1 || diff < -1) {
        return false;
    }
    
    return isBalanced_Solution1(pRoot->m_pLeft) && isBalanced_Solution1(pRoot->m_pRight);
}

bool BalancedBinaryTree::isBalanced_Solution2(const BinaryTreeNode* pRoot) {
    int depth = 0;
    return isBalanced(pRoot, &depth);
}

// 用后序遍历的方式遍历二叉树的每个节点，在遍历到一个节点之前就已经遍历了它的左右子树，
// 在遍历每个节点的时候记录它的深度（某一节点的深度等于它到叶节点的路径的长度），
// 可以一边遍历一边判断每个节点是不是平衡的
bool BalancedBinaryTree::isBalanced(const BinaryTreeNode* pRoot, int* pDepth) {
    if (pRoot == nullptr) {
        *pDepth = 0;
        return true;
    }
    
    int left, right;
    
    if (isBalanced(pRoot->m_pLeft, &left) && isBalanced(pRoot->m_pRight, &right)) {
        int diff = left - right;
        
        if (diff <= 1 && diff >= -1) {
            *pDepth = 1 + (left > right? left: right);
            return true;
        }
    }
    
    return false;
}

