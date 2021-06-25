//
//  PrintTreeFromTopToBottom_1.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/5.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "PrintTreeFromTopToBottom_1.hpp"

void PrintTreeFromTopToBottom_1::printFromTopToBottom(BinaryTreeNode* pRoot) {
    if (pRoot == nullptr) {
        return;
    }
    
    deque<BinaryTreeNode*> dequeTreeNode;
    dequeTreeNode.push_back(pRoot);
    
    while (!dequeTreeNode.empty()) {
        BinaryTreeNode* node = dequeTreeNode.front();
        dequeTreeNode.pop_front();
        
        printf("%d\t", node->m_nValue);
        
        if (node->m_pLeft != nullptr) {
            dequeTreeNode.push_back(node->m_pLeft);
        }
        
        if (node->m_pRight != nullptr) {
            dequeTreeNode.push_back(node->m_pRight);
        }
    }
}

// 测试代码
void PrintTreeFromTopToBottom_1::Test(char* testName, BinaryTreeNode* pRoot) {
    if(testName != nullptr)
        printf("%s begins: \n", testName);
    
    PrintTree(pRoot);
    
    printf("The nodes from top to bottom, from left to right are: \n");
    printFromTopToBottom(pRoot);
    
    printf("\n\n");
}

//            10
//         /      \
//        6        14
//       /\        /\
//      4  8     12  16
void PrintTreeFromTopToBottom_1::Test1() {
    BinaryTreeNode* pNode10 = CreateBinaryTreeNode(10);
    BinaryTreeNode* pNode6 = CreateBinaryTreeNode(6);
    BinaryTreeNode* pNode14 = CreateBinaryTreeNode(14);
    BinaryTreeNode* pNode4 = CreateBinaryTreeNode(4);
    BinaryTreeNode* pNode8 = CreateBinaryTreeNode(8);
    BinaryTreeNode* pNode12 = CreateBinaryTreeNode(12);
    BinaryTreeNode* pNode16 = CreateBinaryTreeNode(16);
    
    ConnectTreeNodes(pNode10, pNode6, pNode14);
    ConnectTreeNodes(pNode6, pNode4, pNode8);
    ConnectTreeNodes(pNode14, pNode12, pNode16);
    
    Test("Test1", pNode10);
    
    DestroyTree(pNode10);
}

//               5
//              /
//             4
//            /
//           3
//          /
//         2
//        /
//       1
void PrintTreeFromTopToBottom_1::Test2() {
    BinaryTreeNode* pNode5 = CreateBinaryTreeNode(5);
    BinaryTreeNode* pNode4 = CreateBinaryTreeNode(4);
    BinaryTreeNode* pNode3 = CreateBinaryTreeNode(3);
    BinaryTreeNode* pNode2 = CreateBinaryTreeNode(2);
    BinaryTreeNode* pNode1 = CreateBinaryTreeNode(1);
    
    ConnectTreeNodes(pNode5, pNode4, nullptr);
    ConnectTreeNodes(pNode4, pNode3, nullptr);
    ConnectTreeNodes(pNode3, pNode2, nullptr);
    ConnectTreeNodes(pNode2, pNode1, nullptr);
    
    Test("Test2", pNode5);
    
    DestroyTree(pNode5);
}

// 1
//  \
//   2
//    \
//     3
//      \
//       4
//        \
//         5
void PrintTreeFromTopToBottom_1::Test3() {
    BinaryTreeNode* pNode1 = CreateBinaryTreeNode(1);
    BinaryTreeNode* pNode2 = CreateBinaryTreeNode(2);
    BinaryTreeNode* pNode3 = CreateBinaryTreeNode(3);
    BinaryTreeNode* pNode4 = CreateBinaryTreeNode(4);
    BinaryTreeNode* pNode5 = CreateBinaryTreeNode(5);
    
    ConnectTreeNodes(pNode1, nullptr, pNode2);
    ConnectTreeNodes(pNode2, nullptr, pNode3);
    ConnectTreeNodes(pNode3, nullptr, pNode4);
    ConnectTreeNodes(pNode4, nullptr, pNode5);
    
    Test("Test3", pNode1);
    
    DestroyTree(pNode1);
}

// 树中只有1个结点
void PrintTreeFromTopToBottom_1::Test4() {
    BinaryTreeNode* pNode1 = CreateBinaryTreeNode(1);
    Test("Test4", pNode1);
    
    DestroyTree(pNode1);
}

// 树中没有结点
void PrintTreeFromTopToBottom_1::Test5() {
    Test("Test5", nullptr);
}

void PrintTreeFromTopToBottom_1::Test() {
    Test1();
    Test2();
    Test3();
    Test4();
    Test5();
}
