//
//  MirrorOfBinaryTree_1.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/5.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef MirrorOfBinaryTree_1_hpp
#define MirrorOfBinaryTree_1_hpp

#include <stdio.h>
#include "BinaryTree.hpp"
#include <stack>

using namespace BinaryTree;

namespace MirrorOfBinaryTree_1 {

// 27：二叉树的镜像
// 题目：请完成一个函数，输入一个二叉树，该函数输出它的镜像。
void mirrorRecursively(BinaryTreeNode* pRoot);
void mirrorIteratively(BinaryTreeNode* pRoot);

// 测试代码
// 测试完全二叉树：除了叶子节点，其他节点都有两个子节点
//            8
//        6      10
//       5 7    9  11
void Test1();
// 测试二叉树：出叶子结点之外，左右的结点都有且只有一个左子结点
//            8
//          7
//        6
//      5
//    4
void Test2();
// 测试二叉树：出叶子结点之外，左右的结点都有且只有一个右子结点
//            8
//             7
//              6
//               5
//                4
void Test3();
// 测试空二叉树：根结点为空指针
void Test4();
// 测试只有一个结点的二叉树
void Test5();

void Test();

}

#endif /* MirrorOfBinaryTree_1_hpp */
