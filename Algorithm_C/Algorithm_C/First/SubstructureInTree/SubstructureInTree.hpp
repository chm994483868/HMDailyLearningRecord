//
//  SubstructureInTree.hpp
//  Algorithm_C
//
//  Created by HM C on 2020/7/16.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef SubstructureInTree_hpp
#define SubstructureInTree_hpp

#include <stdio.h>
#include <cstdio>

struct BinaryTreeNode {
    double m_dbValue;
    BinaryTreeNode* m_pLeft;
    BinaryTreeNode* m_pRight;
};

bool hasSubtree(BinaryTreeNode* pRoot1, BinaryTreeNode* pRoot2);

#endif /* SubstructureInTree_hpp */
