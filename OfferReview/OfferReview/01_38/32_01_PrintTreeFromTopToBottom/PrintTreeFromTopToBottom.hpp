//
//  PrintTreeFromTopToBottom.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/30.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef PrintTreeFromTopToBottom_hpp
#define PrintTreeFromTopToBottom_hpp

#include <stdio.h>
#include <deque>
#include "BinaryTree.hpp"

using namespace std;
using namespace BinaryTree;

namespace PrintTreeFromTopToBottom {

// 32（一）：不分行从上往下打印二叉树
// 题目：从上往下打印出二叉树的每个结点，同一层的结点按照从左到右的顺序打印。
void printFromTopToBottom(BinaryTreeNode* pRoot);

// 测试代码
void Test(char* testName, BinaryTreeNode* pRoot);
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

#endif /* PrintTreeFromTopToBottom_hpp */
