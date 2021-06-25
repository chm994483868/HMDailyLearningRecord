//
//  ConvertBinarySearchTree.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/30.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef ConvertBinarySearchTree_hpp
#define ConvertBinarySearchTree_hpp

#include <stdio.h>
#include "BinaryTree.hpp"

using namespace BinaryTree;
using namespace std;

namespace ConvertBinarySearchTree {

// 36：二叉搜索树与双向链表
// 题目：输入一棵二叉搜索树，将该二叉搜索树转换成一个排序的双向链表。要求
// 不能创建任何新的结点，只能调整树中结点指针的指向。
void convertNode(BinaryTreeNode* pNode, BinaryTreeNode** pLastNodeInList);
BinaryTreeNode* convert(BinaryTreeNode* pRootOfTree);

// 测试代码
void PrintDoubleLinkedList(BinaryTreeNode* pHeadOfList);
void DestroyList(BinaryTreeNode* pHeadOfList);

void Test(char* testName, BinaryTreeNode* pRootOfTree);
//            10
//         /      \
//        6        14
//       /\        /\
//      4  8     12  16
void Test1();
//               5
//              /
//             4
//            /
//           3
//          /
//         2
//        /
//       1
void Test2();
// 1
//  \
//   2
//    \
//     3
//      \
//       4
//        \
//         5
void Test3();
// 树中只有1个结点
void Test4();
// 树中没有结点
void Test5();

void Test();

}

#endif /* ConvertBinarySearchTree_hpp */
