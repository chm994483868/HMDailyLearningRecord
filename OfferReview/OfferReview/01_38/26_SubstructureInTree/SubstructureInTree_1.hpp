//
//  SubstructureInTree_1.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/4.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef SubstructureInTree_1_hpp
#define SubstructureInTree_1_hpp

#include <stdio.h>

namespace SubstructureInTree_1 {

// 26：树的子结构
// 题目：输入两棵二叉树 A 和 B，判断 B 是不是 A 的子结构。
struct BinaryTreeNode {
    double m_dbValue;
    BinaryTreeNode* m_pLeft;
    BinaryTreeNode* m_pRight;
};

bool doesTree1HaveTree2(BinaryTreeNode* pRoot1, BinaryTreeNode* pRoot2);
bool equal(double num1, double num2);
bool hasSubtree(BinaryTreeNode* pRoot1, BinaryTreeNode* pRoot2);

// 辅助代码
BinaryTreeNode* CreateBinaryTreeNode(double dbValue);
void ConnectTreeNodes(BinaryTreeNode* pParent, BinaryTreeNode* pLeft, BinaryTreeNode* pRight);
void DestroyTree(BinaryTreeNode* pRoot);

// 测试代码
void Test(char* testName, BinaryTreeNode* pRoot1, BinaryTreeNode* pRoot2, bool expected);
// 树中结点含有分叉，树B是树A的子结构
//                  8                8
//              /       \           / \
//             8         7         9   2
//           /   \
//          9     2
//               / \
//              4   7
void Test1();
// 树中结点含有分叉，树B不是树A的子结构
//                  8                8
//              /       \           / \
//             8         7         9   2
//           /   \
//          9     3
//               / \
//              4   7
void Test2();
// 树中结点只有左子结点，树B是树A的子结构
//                8                  8
//              /                   /
//             8                   9
//           /                    /
//          9                    2
//         /
//        2
//       /
//      5
void Test3();
// 树中结点只有左子结点，树B不是树A的子结构
//                8                  8
//              /                   /
//             8                   9
//           /                    /
//          9                    3
//         /
//        2
//       /
//      5
void Test4();
// 树中结点只有右子结点，树B是树A的子结构
//       8                   8
//        \                   \
//         8                   9
//          \                   \
//           9                   2
//            \
//             2
//              \
//               5
void Test5();
// 树A中结点只有右子结点，树B不是树A的子结构
//       8                   8
//        \                   \
//         8                   9
//          \                 / \
//           9               3   2
//            \
//             2
//              \
//               5
void Test6();
// 树 A 为空树
void Test7();
// 树 B 为空树
void Test8();
// 树A和树B都为空
void Test9();

void Test();

}

#endif /* SubstructureInTree_1_hpp */
