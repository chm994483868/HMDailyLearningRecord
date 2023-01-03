//
//  KthNodeInBST.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef KthNodeInBST_hpp
#define KthNodeInBST_hpp

#include <stdio.h>
#include "BinaryTree.hpp"

using namespace BinaryTree;

namespace KthNodeInBST {

// 面试题54：二叉搜索树的第 k 个结点
// 题目：给定一棵二叉搜索树，请找出其中的第 k 大的结点。

const BinaryTreeNode* kthNodeCore(const BinaryTreeNode* pRoot, unsigned int& k);
const BinaryTreeNode* kthNode(const BinaryTreeNode* pRoot, unsigned int k);

}
#endif /* KthNodeInBST_hpp */
