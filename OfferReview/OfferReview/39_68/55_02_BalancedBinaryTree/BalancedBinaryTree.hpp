//
//  BalancedBinaryTree.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef BalancedBinaryTree_hpp
#define BalancedBinaryTree_hpp

#include <stdio.h>
#include "BinaryTree.hpp"

using namespace BinaryTree;

namespace BalancedBinaryTree {

// 面试题55（二）：平衡二叉树
// 题目：输入一棵二叉树的根结点，判断该树是不是平衡二叉树。如果某二叉树中
// 任意结点的左右子树的深度相差不超过1，那么它就是一棵平衡二叉树。

int treeDepth(const BinaryTreeNode* pRoot);
bool isBalanced_Solution1(const BinaryTreeNode* pRoot);

bool isBalanced(const BinaryTreeNode* pRoot, int* pDepth);
bool isBalanced_Solution2(const BinaryTreeNode* pRoot);

}

#endif /* BalancedBinaryTree_hpp */
