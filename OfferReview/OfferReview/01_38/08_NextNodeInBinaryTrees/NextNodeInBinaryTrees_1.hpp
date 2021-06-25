//
//  NextNodeInBinaryTrees_1.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/1.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef NextNodeInBinaryTrees_1_hpp
#define NextNodeInBinaryTrees_1_hpp

#include <stdio.h>

namespace NextNodeInBinaryTrees_1 {

// 8：二叉树的下一个结点
// 题目：给定一棵二叉树和其中的一个结点，如何找出中序遍历顺序的下一个结点？
// 树中的结点除了有两个分别指向左右子结点的指针以外，还有一个指向父结点的指针。

struct BinaryTreeNode {
    int m_nValue;
    BinaryTreeNode* m_pParent;
    BinaryTreeNode* m_pLeft;
    BinaryTreeNode* m_pRight;
};

BinaryTreeNode* getNext(BinaryTreeNode* pNode);

// 辅助函数
BinaryTreeNode* CreateBinaryTreeNode(int value);
void ConnectTreeNodes(BinaryTreeNode* pParent, BinaryTreeNode* pLeft, BinaryTreeNode* pRight);
void PrintTreeNode(BinaryTreeNode* pNode);
void PrintTree(BinaryTreeNode* pRoot);
void DestroyTree(BinaryTreeNode* pRoot);
// 测试代码
void Test(char* testName, BinaryTreeNode* pNode, BinaryTreeNode* expected);
//            8
//        6      10
//       5 7    9  11
void Test1_7();
//            5
//          4
//        3
//      2
void Test8_11();
//        2
//         3
//          4
//           5
void Test12_15();
void Test16();

void Test();

}

#endif /* NextNodeInBinaryTrees_1_hpp */
