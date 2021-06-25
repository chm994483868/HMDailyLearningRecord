//
//  PrintTreesInZigzag.cpp
//  OfferReview
//
//  Created by CHM on 2020/7/30.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "PrintTreesInZigzag.hpp"

void PrintTreesInZigzag::print(BinaryTreeNode* pRoot) {
    if (pRoot == nullptr) {
        return;
    }
    
    // levels 是一个长度是 2 的 stack<BinaryTreeNode*> 数组，
    // 从上到下每行的节点交替放在 levels[0] 和 levels[1] 中
    stack<BinaryTreeNode*> levels[2];
    
    // current 表示当前正在打印的行的节点所在的 stack<BinaryTreeNode*>
    int current = 0;
    // next 表示下一行节点所在的 stack<BinaryTreeNode*>
    int next = 1;
    
    // 把根节点加入第一个栈
    levels[current].push(pRoot);
    
    // 只要两个栈任一个不为空就表示还有节点需要打印
    while (!levels[0].empty() || !levels[1].empty()) {
        BinaryTreeNode* node = levels[current].top();
        levels[current].pop();
        
        printf("%d ", node->m_nValue);
        
        if (current == 0) {
            // 如果 current 等于 0，则左子节点先入队，右子节点后入队，且队列是 levels[1]
            
            if (node->m_pLeft != nullptr) {
                levels[next].push(node->m_pLeft);
            }
            
            if (node->m_pRight != nullptr) {
                levels[next].push(node->m_pRight);
            }
        } else {
            // 如果 current 等于 1，则右子节点先入队，左子节点后入队，且队列是 levels[0]
            
            if (node->m_pRight != nullptr) {
                levels[next].push(node->m_pRight);
            }
            
            if (node->m_pLeft != nullptr) {
                levels[next].push(node->m_pLeft);
            }
        }
        
        // 如果当前的 stack<BinaryTreeNode*> 为空，表示一行打印完成了，则切到下一行打印
        if (levels[current].empty()) {
            // 每次进入这个 if 表示要开始打印新的一行了，current 和 next 都分别做 1 和 0 的交换
            
            printf("\n");
            current = 1 - current;
            next = 1 - next;
        }
    }
}

// 测试代码
//            8
//        6      10
//       5 7    9  11
void PrintTreesInZigzag::Test1() {
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
    printf("10 6 \n");
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
void PrintTreesInZigzag::Test2() {
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
void PrintTreesInZigzag::Test3() {
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

void PrintTreesInZigzag::Test4() {
    BinaryTreeNode* pNode5 = CreateBinaryTreeNode(5);

    printf("====Test4 Begins: ====\n");
    printf("Expected Result is:\n");
    printf("5 \n\n");

    printf("Actual Result is: \n");
    print(pNode5);
    printf("\n");

    DestroyTree(pNode5);
}

void PrintTreesInZigzag::Test5() {
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
void PrintTreesInZigzag::Test6() {
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

//                8
//        4              12
//     2     6       10      14
//   1  3  5  7     9 11   13  15
void PrintTreesInZigzag::Test7() {
    BinaryTreeNode* pNode8 = CreateBinaryTreeNode(8);
    BinaryTreeNode* pNode4 = CreateBinaryTreeNode(4);
    BinaryTreeNode* pNode12 = CreateBinaryTreeNode(12);
    BinaryTreeNode* pNode2 = CreateBinaryTreeNode(2);
    BinaryTreeNode* pNode6 = CreateBinaryTreeNode(6);
    BinaryTreeNode* pNode10 = CreateBinaryTreeNode(10);
    BinaryTreeNode* pNode14 = CreateBinaryTreeNode(14);
    BinaryTreeNode* pNode1 = CreateBinaryTreeNode(1);
    BinaryTreeNode* pNode3 = CreateBinaryTreeNode(3);
    BinaryTreeNode* pNode5 = CreateBinaryTreeNode(5);
    BinaryTreeNode* pNode7 = CreateBinaryTreeNode(7);
    BinaryTreeNode* pNode9 = CreateBinaryTreeNode(9);
    BinaryTreeNode* pNode11 = CreateBinaryTreeNode(11);
    BinaryTreeNode* pNode13 = CreateBinaryTreeNode(13);
    BinaryTreeNode* pNode15 = CreateBinaryTreeNode(15);

    ConnectTreeNodes(pNode8, pNode4, pNode12);
    ConnectTreeNodes(pNode4, pNode2, pNode6);
    ConnectTreeNodes(pNode12, pNode10, pNode14);
    ConnectTreeNodes(pNode2, pNode1, pNode3);
    ConnectTreeNodes(pNode6, pNode5, pNode7);
    ConnectTreeNodes(pNode10, pNode9, pNode11);
    ConnectTreeNodes(pNode14, pNode13, pNode15);

    printf("====Test7 Begins: ====\n");
    printf("Expected Result is:\n");
    printf("8 \n");
    printf("12 4 \n");
    printf("2 6 10 14 \n");
    printf("15 13 11 9 7 5 3 1 \n\n");

    printf("Actual Result is: \n");
    print(pNode8);
    printf("\n");

    DestroyTree(pNode8);
}

void PrintTreesInZigzag::Test() {
    Test1();
    Test2();
    Test3();
    Test4();
    Test5();
    Test6();
    Test7();
}
