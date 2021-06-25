//
//  PrintTreesInLines.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/17.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "PrintTreesInLines.hpp"

void printTreesInLines(BinaryTreeNode* pRoot) {
    if (pRoot == nullptr)
        return;
    
    // 1. 先准备一个队列，并把根节点放进队列里面
    std::queue<BinaryTreeNode*> nodes;
    nodes.push(pRoot);
    
    int nextLevel = 0; // 记录下层待打印的节点数，
    int toBePrinted = 1; // 记录本层待打印的节点数
    
    // 模拟打印一个完全二叉树的情况:
    // 打印第一层时：第一次进 while 循环：只有一个根节点 toBePrinted == 1, 执行到底部时: nextLevel 是 2，进入 if 以后，toBePrinted 置为 2，且 nextLevel 置为 0
    // 打印第二层时：第二次进 while 循环：有根节点的左右子节点，打印左子节点时： nextLevel == 2，接着 toBePrinted == 1, 打印 右子节点时，nextLevel == 4, 进入 if 语句， toBePrinted 被置为 4, nextLevel 被置为 0
    // 打印第三层时：结束时 nextLevel 加了 8 次，toBePrinted 被置为 8，nextLevel 被置 0
    // 依次循环下去...
    
    while (!nodes.empty()) {
        BinaryTreeNode* pNode = nodes.front();
        printf("%d ", pNode->m_nValue);
        
        if (pNode->m_pLeft != nullptr) {
            nodes.push(pNode->m_pLeft);
            ++nextLevel;
        }
        
        if (pNode->m_pRight != nullptr) {
            nodes.push(pNode->m_pRight);
            ++nextLevel;
        }
        
        nodes.pop();
        --toBePrinted;
        
        if (toBePrinted == 0) {
            printf("\n");
            toBePrinted = nextLevel;
            nextLevel = 0;
        }
    }
}

void printTreesInLines_Review(BinaryTreeNode* pRoot) {
    if (pRoot == nullptr)
        return;
    // 1. 创建一个队列，并把根节点放进去
    std::queue<BinaryTreeNode*> nodes;
    nodes.push(pRoot);
    
    int nexLevel = 0;
    int toBePrinted = 1;
    
    while (!nodes.empty()) {
        // 2. 取出队列头部的元素打印
        BinaryTreeNode* pNode = nodes.front();
        printf("%d", pNode->m_nValue);
        
        if (pNode->m_pLeft) {
            nodes.push(pNode->m_pLeft);
            ++nexLevel;
        }
        
        if (pNode->m_pRight) {
            nodes.push(pNode->m_pRight);
            ++nexLevel;
        }
        
        nodes.pop();
        --toBePrinted;
        
        if (toBePrinted == 0) {
            printf("\n");
            toBePrinted = nexLevel;
            nexLevel = 0;
        }
    }
}
