//
//  KthNodeInBST.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/24.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "KthNodeInBST.hpp"

struct BinaryTreeNode {
    int m_nValue;
    BinaryTreeNode* m_pLeft;
    BinaryTreeNode* m_pRight;
};

const BinaryTreeNode* kthNodeCore(const BinaryTreeNode* pRoot, unsigned int& k);

const BinaryTreeNode* kthNode(const BinaryTreeNode* pRoot, unsigned int k) {
    if (pRoot == nullptr || k == 0) {
        return nullptr;
    }
    
    return kthNodeCore(pRoot, k);
}

//        5
//     /    \
//   3       7
//  / \     /  \
// 2   4   6    8

const BinaryTreeNode* kthNodeCore(const BinaryTreeNode* pRoot, unsigned int& k) {
    const BinaryTreeNode* target = nullptr;
    
    if (pRoot->m_pLeft != nullptr) {
        target = kthNodeCore(pRoot->m_pLeft, k);
    }
    
    if (target == nullptr) {
        if (k == 1) {
            target = pRoot;
        }
        
        --k;
    }
    
    if (target == nullptr && pRoot->m_pRight != nullptr) {
        target = kthNodeCore(pRoot->m_pRight, k);
    }
    
    return target;
}

namespace KthNodeInBST_Review {

const BinaryTreeNode* kthNodeCore_Review(const BinaryTreeNode* pRoot, unsigned int& k) {
    const BinaryTreeNode* target = nullptr;
    
    if (pRoot->m_pLeft != nullptr) {
        target = kthNodeCore_Review(pRoot->m_pLeft, k);
    }
    
    if (target == nullptr) {
        if (k == 1) {
            target = pRoot;
        }
        
        --k;
    }
    
    if (target == nullptr && pRoot->m_pRight != nullptr) {
        target = kthNodeCore_Review(pRoot->m_pRight, k);
    }
    
    return target;
}

}
