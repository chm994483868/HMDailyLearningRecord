//
//  MirrorOfBinaryTree.hpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/17.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef MirrorOfBinaryTree_hpp
#define MirrorOfBinaryTree_hpp

// 二叉树的镜像（反转二叉树）
#include <stdio.h>
#include <cstdio>
#include <stack>

struct BinaryTreeNode {
    int m_nValue;
    BinaryTreeNode* m_pLeft;
    BinaryTreeNode* m_pRight;
};

void mirrorRecursively(BinaryTreeNode* pNode);
void mirrorIteratively(BinaryTreeNode* pRoot);

#endif /* MirrorOfBinaryTree_hpp */
