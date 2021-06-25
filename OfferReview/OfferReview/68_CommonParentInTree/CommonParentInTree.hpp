//
//  CommonParentInTree.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/11.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef CommonParentInTree_hpp
#define CommonParentInTree_hpp

#include <stdio.h>
#include <list>
#include "BinaryTree.hpp"

using namespace BinaryTree;
using namespace std;

namespace CommonParentInTree {

// 面试题68：树中两个结点的最低公共祖先
// 题目：输入两个树结点，求它们的最低公共祖先。

bool getNodePath(const BinaryTreeNode* pRoot, const BinaryTreeNode* pNode, list<const BinaryTreeNode*>& path);
}

#endif /* CommonParentInTree_hpp */
