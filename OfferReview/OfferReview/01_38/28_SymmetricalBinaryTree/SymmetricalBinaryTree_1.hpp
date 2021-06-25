//
//  SymmetricalBinaryTree_1.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/5.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef SymmetricalBinaryTree_1_hpp
#define SymmetricalBinaryTree_1_hpp

#include <stdio.h>
#include "BinaryTree.hpp"

using namespace BinaryTree;

namespace SymmetricalBinaryTree_1 {

// 28：对称的二叉树
// 题目：请实现一个函数，用来判断一棵二叉树是不是对称的。如果一棵二叉树和
// 它的镜像一样，那么它是对称的。
bool isSymmetrical(BinaryTreeNode* pRoot1, BinaryTreeNode* pRoot2);
bool isSymmetrical(BinaryTreeNode* pRoot);

// 测试代码
void Test(char* testName, BinaryTreeNode* pRoot, bool expected);
//            8
//        6      6
//       5 7    7 5
void Test1();
//            8
//        6      9
//       5 7    7 5
void Test2();
//            8
//        6      6
//       5 7    7
void Test3();
//               5
//              / \
//             3   3
//            /     \
//           4       4
//          /         \
//         2           2
//        /             \
//       1               1
void Test4();
//               5
//              / \
//             3   3
//            /     \
//           4       4
//          /         \
//         6           2
//        /             \
//       1               1
void Test5();
//               5
//              / \
//             3   3
//            /     \
//           4       4
//          /         \
//         2           2
//                      \
//                       1
void Test6();
// 只有一个结点
void Test7();
// 没有结点
void Test8();
// 所有结点都有相同的值，树对称
//               5
//              / \
//             5   5
//            /     \
//           5       5
//          /         \
//         5           5
void Test9();
// 所有结点都有相同的值，树不对称
//               5
//              / \
//             5   5
//            /     \
//           5       5
//          /       /
//         5       5
void Test10();

void Test();

}

#endif /* SymmetricalBinaryTree_1_hpp */
