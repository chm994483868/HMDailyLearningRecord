//
//  TreeDepth.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef TreeDepth_hpp
#define TreeDepth_hpp

#include <stdio.h>
#include <cstdio>
#include "BinaryTree.hpp"

using namespace BinaryTree;

namespace TreeDepth {

// 面试题55（一）：二叉树的深度
// 题目：输入一棵二叉树的根结点，求该树的深度。从根结点到叶结点依次经过的
// 结点（含根、叶结点）形成树的一条路径，最长路径的长度为树的深度。

int treeDepth(const BinaryTreeNode* pRoot);

}
#endif /* TreeDepth_hpp */
