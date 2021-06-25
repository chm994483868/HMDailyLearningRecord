//
//  ConvertBinarySearchTree.cpp
//  OfferReview
//
//  Created by CHM on 2020/7/30.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "ConvertBinarySearchTree.hpp"

void ConvertBinarySearchTree::convertNode(BinaryTreeNode* pNode, BinaryTreeNode** pLastNodeInList) {
    if (pNode == nullptr) {
        return;
    }
    
    BinaryTreeNode* pCurrent = pNode;
    // pCurrent 记录当前节点，然后沿着 m_pLeft 一直找下去，
    // 到底的节点一定就是值最小的节点，也就是链表的头节点。
    if (pCurrent->m_pLeft != nullptr) {
        // 一直沿着 pCurrent->m_pLeft 递归向下
        convertNode(pCurrent->m_pLeft, pLastNodeInList);
    }
    
    // 这里先把 pCurrent 的 m_pLeft 指向 *pLastNodeInList，
    // 第一次时 *pLastNodeInList 的值是 nullptr，它一直是作为当前链表的尾节点存在的，所以当一个节点进来时，m_pLeft 都是指向它，
    // 然后下面 *pLastNodeInList 的 m_pRight 指向这个当前节点，即把这个进来的节点追加到链表的尾部了。
    pCurrent->m_pLeft = *pLastNodeInList;
    if (*pLastNodeInList != nullptr) {
        (*pLastNodeInList)->m_pRight = pCurrent;
    }
    
    // 然后更新 *pLastNodeInList，往后移动到链表的最后，即更新了当前链表的尾节点。
    *pLastNodeInList = pCurrent;
    
    // 这里是递归每个左子节点的右子树或者右子节点
    if (pCurrent->m_pRight != nullptr) {
        convertNode(pCurrent->m_pRight, pLastNodeInList);
    }
}

BinaryTreeNode* ConvertBinarySearchTree::convert(BinaryTreeNode* pRootOfTree) {
    BinaryTreeNode* pLastNodeInList = nullptr;
    
    // 二叉搜索树转换为双向链表
    convertNode(pRootOfTree, &pLastNodeInList);
    
    // pLastNodeInList 指向双向链表的尾节点，
    // 从尾节点沿着 m_pLeft 遍历到链表头部节点并返回头节点
    BinaryTreeNode* pHeadOfList = pLastNodeInList;
    while (pHeadOfList != nullptr && pHeadOfList->m_pLeft != nullptr) {
        pHeadOfList = pHeadOfList->m_pLeft;
    }
    
    // 返回链表的头节点
    return pHeadOfList;
}

// 测试代码
void ConvertBinarySearchTree::PrintDoubleLinkedList(BinaryTreeNode* pHeadOfList) {
    BinaryTreeNode* pNode = pHeadOfList;

    printf("The nodes from left to right are:\n");
    while(pNode != nullptr){
        printf("%d\t", pNode->m_nValue);

        if(pNode->m_pRight == nullptr)
            break;
        pNode = pNode->m_pRight;
    }

    printf("\nThe nodes from right to left are:\n");
    while(pNode != nullptr){
        printf("%d\t", pNode->m_nValue);

        if(pNode->m_pLeft == nullptr)
            break;
        pNode = pNode->m_pLeft;
    }

    printf("\n");
}

void ConvertBinarySearchTree::DestroyList(BinaryTreeNode* pHeadOfList) {
    BinaryTreeNode* pNode = pHeadOfList;
    while(pNode != nullptr) {
        BinaryTreeNode* pNext = pNode->m_pRight;

        delete pNode;
        pNode = pNext;
    }
}

void ConvertBinarySearchTree::Test(char* testName, BinaryTreeNode* pRootOfTree) {
    if(testName != nullptr)
        printf("%s begins:\n", testName);

    PrintTree(pRootOfTree);

    BinaryTreeNode* pHeadOfList = convert(pRootOfTree);

    PrintDoubleLinkedList(pHeadOfList);
}

//            10
//         /      \
//        6        14
//       /\        /\
//      4  8     12  16
void ConvertBinarySearchTree::Test1() {
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

    DestroyList(pNode4);
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
void ConvertBinarySearchTree::Test2() {
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

    DestroyList(pNode1);
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
void ConvertBinarySearchTree::Test3() {
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

    DestroyList(pNode1);
}

// 树中只有1个结点
void ConvertBinarySearchTree::Test4() {
    BinaryTreeNode* pNode1 = CreateBinaryTreeNode(1);
    Test("Test4", pNode1);

    DestroyList(pNode1);
}

// 树中没有结点
void ConvertBinarySearchTree::Test5() {
    Test("Test5", nullptr);
}

void ConvertBinarySearchTree::Test() {
    Test1();
    Test2();
    Test3();
    Test4();
    Test5();
}
