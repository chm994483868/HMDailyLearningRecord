//
//  PrintTreesInLines.cpp
//  OfferReview
//
//  Created by CHM on 2020/7/30.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "PrintTreesInLines.hpp"

void PrintTreesInLines::print(BinaryTreeNode* pRoot) {
    if (pRoot == nullptr) {
        return;
    }
    
    // 准备一个队列，并把根节点放进队列
    deque<BinaryTreeNode*> dequeTreeNode;
    dequeTreeNode.push_back(pRoot);
    
    // nextLevel 用来统计第一行之外的一行中要打印的节点个数
    int nextLevel = 0;
    
    // toBePrinted 表示本行要打印的节点个数，默认值为 1 代指根节点的一行就一个节点
    int toBePrinted = 1;
    
    while (!dequeTreeNode.empty()) {
        
        // 出队列并打印节点
        BinaryTreeNode* node = dequeTreeNode.front();
        printf("%d ", node->m_nValue);
        
        // 如果左节点不为空，则放进队列
        if (node->m_pLeft != nullptr) {
            dequeTreeNode.push_back(node->m_pLeft);
            // 统计节点
            ++nextLevel;
        }
        
        if (node->m_pRight != nullptr) {
            dequeTreeNode.push_back(node->m_pRight);
            // 统计节点
            ++nextLevel;
        }
        
        dequeTreeNode.pop_front();
        
        // 本行要打印的节点个数减 1
        --toBePrinted;
        
        // 如果本行的节点打印完了，则打印换行，并更新 toBePrinted 为下一行要打印的节点个数，
        // nextLevel 则置为 0，继续统计下下一行的节点个数
        if (toBePrinted == 0) {
            printf("\n");
            toBePrinted = nextLevel;
            nextLevel = 0;
        }
    }
}

// 测试代码
//            8
//        6      10
//       5 7    9  11
void PrintTreesInLines::Test1() {
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

    printf("====Test1 Begins: ====\n");
    printf("Expected Result is:\n");
    printf("8 \n");
    printf("6 10 \n");
    printf("5 7 9 11 \n\n");

    printf("Actual Result is: \n");
    print(pNode8);
    printf("\n");

    DestroyTree(pNode8);
}

//            5
//          4
//        3
//      2
void PrintTreesInLines::Test2() {
    BinaryTreeNode* pNode5 = CreateBinaryTreeNode(5);
    BinaryTreeNode* pNode4 = CreateBinaryTreeNode(4);
    BinaryTreeNode* pNode3 = CreateBinaryTreeNode(3);
    BinaryTreeNode* pNode2 = CreateBinaryTreeNode(2);

    ConnectTreeNodes(pNode5, pNode4, nullptr);
    ConnectTreeNodes(pNode4, pNode3, nullptr);
    ConnectTreeNodes(pNode3, pNode2, nullptr);

    printf("====Test2 Begins: ====\n");
    printf("Expected Result is:\n");
    printf("5 \n");
    printf("4 \n");
    printf("3 \n");
    printf("2 \n\n");

    printf("Actual Result is: \n");
    print(pNode5);
    printf("\n");

    DestroyTree(pNode5);
}

//        5
//         4
//          3
//           2
void PrintTreesInLines::Test3() {
    BinaryTreeNode* pNode5 = CreateBinaryTreeNode(5);
    BinaryTreeNode* pNode4 = CreateBinaryTreeNode(4);
    BinaryTreeNode* pNode3 = CreateBinaryTreeNode(3);
    BinaryTreeNode* pNode2 = CreateBinaryTreeNode(2);

    ConnectTreeNodes(pNode5, nullptr, pNode4);
    ConnectTreeNodes(pNode4, nullptr, pNode3);
    ConnectTreeNodes(pNode3, nullptr, pNode2);

    printf("====Test3 Begins: ====\n");
    printf("Expected Result is:\n");
    printf("5 \n");
    printf("4 \n");
    printf("3 \n");
    printf("2 \n\n");

    printf("Actual Result is: \n");
    print(pNode5);
    printf("\n");

    DestroyTree(pNode5);
}

void PrintTreesInLines::Test4() {
    BinaryTreeNode* pNode5 = CreateBinaryTreeNode(5);

    printf("====Test4 Begins: ====\n");
    printf("Expected Result is:\n");
    printf("5 \n\n");

    printf("Actual Result is: \n");
    print(pNode5);
    printf("\n");

    DestroyTree(pNode5);
}

void PrintTreesInLines::Test5() {
    printf("====Test5 Begins: ====\n");
    printf("Expected Result is:\n");

    printf("Actual Result is: \n");
    print(nullptr);
    printf("\n");
}

//        100
//        /
//       50
//         \
//         150
void PrintTreesInLines::Test6() {
    BinaryTreeNode* pNode100 = CreateBinaryTreeNode(100);
    BinaryTreeNode* pNode50 = CreateBinaryTreeNode(50);
    BinaryTreeNode* pNode150 = CreateBinaryTreeNode(150);

    ConnectTreeNodes(pNode100, pNode50, nullptr);
    ConnectTreeNodes(pNode50, nullptr, pNode150);

    printf("====Test6 Begins: ====\n");
    printf("Expected Result is:\n");
    printf("100 \n");
    printf("50 \n");
    printf("150 \n\n");

    printf("Actual Result is: \n");
    print(pNode100);
    printf("\n");
}

void PrintTreesInLines::Test() {
    Test1();
    Test2();
    Test3();
    Test4();
    Test5();
    Test6();
}
