//
//  ConstructBinaryTree_1.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/1.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef ConstructBinaryTree_1_hpp
#define ConstructBinaryTree_1_hpp

#include <stdio.h>
#include <vector>
#include <exception>

using namespace std;

namespace ConstructBinaryTree_1 {

// 7：重建二叉树
// 题目：输入某二叉树的前序遍历和中序遍历的结果，请重建出该二叉树。假设输
// 入的前序遍历和中序遍历的结果中都不含重复的数字。例如输入前序遍历序列{1,
// 2, 4, 7, 3, 5, 6, 8}和中序遍历序列{4, 7, 2, 1, 5, 3, 8, 6}，则重建出
// 如下所示的二叉树并输出它的头结点。
//
//              1
//           /     \
//          2       3
//         /       / \
//        4       5   6
//         \         /
//          7       8

struct BinaryTreeNode {
    int m_nValue;
    BinaryTreeNode* m_pLeft;
    BinaryTreeNode* m_pRight;
};

BinaryTreeNode* constructCore(int* startPreorder, int* endPreorder, int* startInorder, int* endInorder);
BinaryTreeNode* construct(int* preorder, int* inorder, int length);

// 辅助函数
BinaryTreeNode* CreateBinaryTreeNode(int value);
void ConnectTreeNodes(BinaryTreeNode* pParent, BinaryTreeNode* pLeft, BinaryTreeNode* pRight);
void PrintTreeNode(const BinaryTreeNode* pNode);
void PrintTree(const BinaryTreeNode* pRoot);
void DestroyTree(BinaryTreeNode* pRoot);
// 测试代码
void Test(char* testName, int* preorder, int* inorder, int length);
// 普通二叉树
//              1
//           /     \
//          2       3
//         /       / \
//        4       5   6
//         \         /
//          7       8
void Test1();
// 所有结点都没有右子结点
//            1
//           /
//          2
//         /
//        3
//       /
//      4
//     /
//    5
void Test2();
// 所有结点都没有左子结点
//            1
//             \
//              2
//               \
//                3
//                 \
//                  4
//                   \
//                    5
void Test3();
// 树中只有一个结点
void Test4();
// 完全二叉树
//              1
//           /     \
//          2       3
//         / \     / \
//        4   5   6   7
void Test5();
// 输入空指针
void Test6();
// 输入的两个序列不匹配
void Test7();

void Test();

}

#endif /* ConstructBinaryTree_1_hpp */
