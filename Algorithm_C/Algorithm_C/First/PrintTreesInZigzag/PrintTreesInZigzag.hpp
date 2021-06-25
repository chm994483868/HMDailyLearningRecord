//
//  PrintTreesInZigzag.hpp
//  Algorithm_C
//
//  Created by HM C on 2020/7/18.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef PrintTreesInZigzag_hpp
#define PrintTreesInZigzag_hpp

#include <stdio.h>
#include <cstdio>
#include <stack>

struct BinaryTreeNode {
    int m_nValue;
    BinaryTreeNode* m_pLeft;
    BinaryTreeNode* m_pRight;
};

void printTreesInZigzag(BinaryTreeNode *pRoot);

#endif /* PrintTreesInZigzag_hpp */
