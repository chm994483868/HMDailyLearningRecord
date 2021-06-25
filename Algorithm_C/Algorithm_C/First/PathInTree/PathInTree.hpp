//
//  PathInTree.hpp
//  Algorithm_C
//
//  Created by HM C on 2020/7/18.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef PathInTree_hpp
#define PathInTree_hpp

#include <stdio.h>
#include <cstdio>
#include <vector>

struct BinaryTreeNode {
    int m_nValue;
    BinaryTreeNode* m_pLeft;
    BinaryTreeNode* m_pRight;
};

void findPathTree(BinaryTreeNode* pRoot, int expectedSum);
void startTest_FindPathTree();

#endif /* PathInTree_hpp */
