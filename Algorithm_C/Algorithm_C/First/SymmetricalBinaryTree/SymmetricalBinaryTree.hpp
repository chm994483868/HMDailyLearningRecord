//
//  SymmetricalBinaryTree.hpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/17.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef SymmetricalBinaryTree_hpp
#define SymmetricalBinaryTree_hpp

#include <stdio.h>
#include <cstdio>

// 对称的二叉树
struct BinaryTreeNode {
    int m_nValue;
    BinaryTreeNode* m_pLeft;
    BinaryTreeNode* m_pRight;
};

bool isSymmetrical(BinaryTreeNode* pRoot1, BinaryTreeNode* pRoot2);
bool isSymmetrical(BinaryTreeNode* pRoot);

#endif /* SymmetricalBinaryTree_hpp */
