//
//  PathInTree_1.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/6.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef PathInTree_1_hpp
#define PathInTree_1_hpp

#include <stdio.h>
#include <vector>
#include "BinaryTree.hpp"

using namespace BinaryTree;
using namespace std;

namespace PathInTree_1 {

// 34：二叉树中和为某一值的路径
// 题目：输入一棵二叉树和一个整数，打印出二叉树中结点值的和为输入整数的所
// 有路径。从树的根结点开始往下一直到叶结点所经过的结点形成一条路径。

void findPath(BinaryTreeNode* pRoot, int expectedSum, std::vector<int>& path, int& currentSum);
void findPath(BinaryTreeNode* pRoot, int expectedSum);

// 测试代码
void Test(char* testName, BinaryTreeNode* pRoot, int expectedSum);
//            10
//         /      \
//        5        12
//       /\
//      4  7
// 有两条路径上的结点和为22
void Test1();
//            10
//         /      \
//        5        12
//       /\
//      4  7
// 没有路径上的结点和为15
void Test2();
//               5
//              /
//             4
//            /
//           3
//          /
//         2
//        /
//       1
// 有一条路径上面的结点和为15
void Test3();
// 1
//  \
//   2
//    \
//     3
//      \
//       4
//        \
//         5
// 没有路径上面的结点和为16
void Test4();
// 树中只有1个结点
void Test5();
// 树中没有结点
void Test6();

void Test();

}

#endif /* PathInTree_1_hpp */
