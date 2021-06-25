//
//  ConvertBinarySearchTree.hpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/20.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef ConvertBinarySearchTree_hpp
#define ConvertBinarySearchTree_hpp

#include <stdio.h>
#include <cstdio>

using namespace std;

struct BinaryTreeNode {
    int m_nValue;
    BinaryTreeNode* m_pLeft;
    BinaryTreeNode* m_pRight;
};

BinaryTreeNode* convert(BinaryTreeNode* pRootOfTree);

#endif /* ConvertBinarySearchTree_hpp */
