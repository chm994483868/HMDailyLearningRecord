//
//  PrintTreesInLines.hpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/17.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef PrintTreesInLines_hpp
#define PrintTreesInLines_hpp

// 分行从上到下打印二叉树
#include <stdio.h>
#include <queue>

struct BinaryTreeNode {
    int m_nValue;
    BinaryTreeNode* m_pLeft;
    BinaryTreeNode* m_pRight;
};

void printTreesInLines(BinaryTreeNode* pRoot);

#endif /* PrintTreesInLines_hpp */
