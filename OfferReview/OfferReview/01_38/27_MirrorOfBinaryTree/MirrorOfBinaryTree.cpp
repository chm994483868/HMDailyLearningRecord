//
//  MirrorOfBinaryTree.cpp
//  OfferReview
//
//  Created by HM C on 2020/7/29.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "MirrorOfBinaryTree.hpp"

void MirrorOfBinaryTree::mirrorRecursively(BinaryTreeNode* pRoot) {
    if (pRoot == nullptr) {
        return;
    }
    
    BinaryTreeNode* temp = pRoot->m_pLeft;
    pRoot->m_pLeft = pRoot->m_pRight;
    pRoot->m_pRight = temp;
    
    if (pRoot->m_pLeft != nullptr) {
        mirrorRecursively(pRoot->m_pLeft);
    }
    
    if (pRoot->m_pRight != nullptr) {
        mirrorRecursively(pRoot->m_pRight);
    }
}

void MirrorOfBinaryTree::mirrorIteratively(BinaryTreeNode* pRoot) {
    if (pRoot == nullptr) {
        return;
    }
    
    std::stack<BinaryTreeNode*> nodes;
    nodes.push(pRoot);
    
    while (!nodes.empty()) {
        BinaryTreeNode* top = nodes.top();
        nodes.pop();
        
        BinaryTreeNode* temp = top->m_pLeft;
        top->m_pLeft = top->m_pRight;
        top->m_pRight = temp;
        
        if (top->m_pLeft != nullptr) {
            nodes.push(top->m_pLeft);
        }
        
        if (top->m_pRight != nullptr) {
            nodes.push(top->m_pRight);
        }
    }
}

// 测试代码
// 测试完全二叉树：除了叶子节点，其他节点都有两个子节点
//            8
//        6      10
//       5 7    9  11
void MirrorOfBinaryTree::Test1() {
    printf("=====Test1 starts:=====\n");
    BinaryTreeNode* pNode8 = CreateBinaryTreeNode(8);
    BinaryTreeNode* pNode6 = CreateBinaryTreeNode(6);
    BinaryTreeNode* pNode10 = CreateBinaryTreeNode(10);
    BinaryTreeNode* pNode5 = CreateBinaryTreeNode(5);
    BinaryTreeNode* pNode7 = CreateBinaryTreeNode(7);
    BinaryTreeNode* pNode9 = CreateBinaryTreeNode(9);
    BinaryTreeNode* pNode11 = CreateBinaryTreeNode(11);

    ConnectTreeNodes(pNode8, pNode6, pNode10);
    ConnectTreeNodes(pNode6, pNode5, pNode7);
    ConnectTreeNodes(pNode10, pNode9, pNode11);

    PrintTree(pNode8);

    printf("=====Test1: MirrorRecursively=====\n");
    mirrorRecursively(pNode8);
    PrintTree(pNode8);

    printf("=====Test1: MirrorIteratively=====\n");
    mirrorIteratively(pNode8);
    PrintTree(pNode8);

    DestroyTree(pNode8);
}

// 测试二叉树：出叶子结点之外，左右的结点都有且只有一个左子结点
//            8
//          7
//        6
//      5
//    4
void MirrorOfBinaryTree::Test2() {
    printf("=====Test2 starts:=====\n");
    BinaryTreeNode* pNode8 = CreateBinaryTreeNode(8);
    BinaryTreeNode* pNode7 = CreateBinaryTreeNode(7);
    BinaryTreeNode* pNode6 = CreateBinaryTreeNode(6);
    BinaryTreeNode* pNode5 = CreateBinaryTreeNode(5);
    BinaryTreeNode* pNode4 = CreateBinaryTreeNode(4);

    ConnectTreeNodes(pNode8, pNode7, nullptr);
    ConnectTreeNodes(pNode7, pNode6, nullptr);
    ConnectTreeNodes(pNode6, pNode5, nullptr);
    ConnectTreeNodes(pNode5, pNode4, nullptr);

    PrintTree(pNode8);

    printf("=====Test2: MirrorRecursively=====\n");
    mirrorRecursively(pNode8);
    PrintTree(pNode8);

    printf("=====Test2: MirrorIteratively=====\n");
    mirrorIteratively(pNode8);
    PrintTree(pNode8);

    DestroyTree(pNode8);
}

// 测试二叉树：出叶子结点之外，左右的结点都有且只有一个右子结点
//            8
//             7
//              6
//               5
//                4
void MirrorOfBinaryTree::Test3() {
    printf("=====Test3 starts:=====\n");
    BinaryTreeNode* pNode8 = CreateBinaryTreeNode(8);
    BinaryTreeNode* pNode7 = CreateBinaryTreeNode(7);
    BinaryTreeNode* pNode6 = CreateBinaryTreeNode(6);
    BinaryTreeNode* pNode5 = CreateBinaryTreeNode(5);
    BinaryTreeNode* pNode4 = CreateBinaryTreeNode(4);

    ConnectTreeNodes(pNode8, nullptr, pNode7);
    ConnectTreeNodes(pNode7, nullptr, pNode6);
    ConnectTreeNodes(pNode6, nullptr, pNode5);
    ConnectTreeNodes(pNode5, nullptr, pNode4);

    PrintTree(pNode8);

    printf("=====Test3: MirrorRecursively=====\n");
    mirrorRecursively(pNode8);
    PrintTree(pNode8);

    printf("=====Test3: MirrorIteratively=====\n");
    mirrorIteratively(pNode8);
    PrintTree(pNode8);

    DestroyTree(pNode8);
}

// 测试空二叉树：根结点为空指针
void MirrorOfBinaryTree::Test4() {
    printf("=====Test4 starts:=====\n");
    BinaryTreeNode* pNode = nullptr;

    PrintTree(pNode);

    printf("=====Test4: MirrorRecursively=====\n");
    mirrorRecursively(pNode);
    PrintTree(pNode);

    printf("=====Test4: MirrorIteratively=====\n");
    mirrorIteratively(pNode);
    PrintTree(pNode);
}

// 测试只有一个结点的二叉树
void MirrorOfBinaryTree::Test5() {
    printf("=====Test5 starts:=====\n");
    BinaryTreeNode* pNode8 = CreateBinaryTreeNode(8);

    PrintTree(pNode8);

    printf("=====Test4: MirrorRecursively=====\n");
    mirrorRecursively(pNode8);
    PrintTree(pNode8);

    printf("=====Test4: MirrorIteratively=====\n");
    mirrorIteratively(pNode8);
    PrintTree(pNode8);
}

void MirrorOfBinaryTree::Test() {
    Test1();
    Test2();
    Test3();
    Test4();
    Test5();
}
