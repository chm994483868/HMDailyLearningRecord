//
//  PrintTreeFromTopToBottom.hpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/17.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef PrintTreeFromTopToBottom_hpp
#define PrintTreeFromTopToBottom_hpp

// 从上到下打印二叉树
#include <stdio.h>
#include <cstdio>
#include <deque>

struct BinaryTreeNode {
    int m_nValue;
    BinaryTreeNode* m_pLeft;
    BinaryTreeNode* m_pRight;
};

void printFromTopToBottom(BinaryTreeNode* pRoot);

#endif /* PrintTreeFromTopToBottom_hpp */
