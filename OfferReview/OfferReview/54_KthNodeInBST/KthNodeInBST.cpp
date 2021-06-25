//
//  KthNodeInBST.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "KthNodeInBST.hpp"

// 后序遍历二叉搜索树，既是从小到大递增的有序序列
const BinaryTreeNode* KthNodeInBST::kthNode(const BinaryTreeNode* pRoot, unsigned int k) {
    if (pRoot == nullptr || k <= 0) {
        return nullptr;
    }
    
    return kthNodeCore(pRoot, k);
}

const BinaryTreeNode* KthNodeInBST::kthNodeCore(const BinaryTreeNode* pRoot, unsigned int& k) {
    const BinaryTreeNode* target = nullptr;
    
    // 递归左子树或者左节点
    if (pRoot->m_pLeft != nullptr) {
        target = kthNodeCore(pRoot->m_pLeft, k);
    }
    
    // k 每次减 1
    if (target == nullptr) {
        // 当 k 减到 1 后的节点，就是第 k 个节点
        if (k == 1) {
            target = pRoot;
            
            // 疑问：不用在这里写个 return 而结束递归吗？
        }
        
        k--;
    }
    
    // 递归右子树或者右节点
    if (target == nullptr && pRoot->m_pRight != nullptr) {
        target = kthNodeCore(pRoot->m_pRight, k);
    }
    
    // 返回 target
    return target;
}
