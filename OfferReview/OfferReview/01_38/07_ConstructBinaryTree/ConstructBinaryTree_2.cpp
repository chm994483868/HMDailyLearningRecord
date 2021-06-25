//
//  ConstructBinaryTree_2.cpp
//  OfferReview
//
//  Created by CHM on 2021/2/21.
//  Copyright © 2021 CHM. All rights reserved.
//

#include "ConstructBinaryTree_2.hpp"

ConstructBinaryTree_2::BinaryTreeNode* ConstructBinaryTree_2::constructCore(int* startPreorder, int* endPreorder, int* startInorder, int* endInorder) {
    int rootValue = startPreorder[0];
    
    BinaryTreeNode* root = new BinaryTreeNode();
    root->m_nValue = rootValue;
    root->m_pLeft = root->m_pRight = nullptr;
    
    if (startPreorder == endPreorder) {
        if (startInorder == endInorder && *startPreorder == *startInorder) {
            return root;
        } else {
            throw std::exception();
        }
    }
    
    int* rootInorder = startInorder;
    while (rootInorder <= endInorder && *rootInorder != rootValue) {
        ++rootInorder;
    }
    
    if (rootInorder == endInorder && *rootInorder != rootValue) {
        throw std::exception();
    }
    
    long leftLength = rootInorder - startInorder;
    int* leftPreorderEnd = startPreorder + leftLength;
    
    if (leftLength > 0) {
        root->m_pLeft = constructCore(startPreorder + 1, leftPreorderEnd, startInorder, rootInorder - 1);
    }
    
    if (leftLength < endPreorder - startPreorder) {
        root->m_pRight = constructCore(leftPreorderEnd + 1, endPreorder, rootInorder + 1, endInorder);
    }
    
    return root;
}

ConstructBinaryTree_2::BinaryTreeNode* ConstructBinaryTree_2::construct(int* preorder, int* inorder, int length) {
    if (preorder == nullptr || inorder == nullptr || length <= 0) {
        return nullptr;
    }
    
    return constructCore(preorder, preorder + length - 1, inorder, inorder + length - 1);
}
