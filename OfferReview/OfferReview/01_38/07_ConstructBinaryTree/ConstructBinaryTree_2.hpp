//
//  ConstructBinaryTree_2.hpp
//  OfferReview
//
//  Created by CHM on 2021/2/21.
//  Copyright © 2021 CHM. All rights reserved.
//

#ifndef ConstructBinaryTree_2_hpp
#define ConstructBinaryTree_2_hpp

#include <stdio.h>
#include <exception>

using namespace std;

namespace ConstructBinaryTree_2 {

struct BinaryTreeNode {
    int m_nValue;
    BinaryTreeNode* m_pLeft;
    BinaryTreeNode* m_pRight;
};

BinaryTreeNode* constructCore(int* startPreorder, int* endPreorder, int* startInorder, int* endInorder);
BinaryTreeNode* construct(int* preorder, int* inorder, int length);

}

#endif /* ConstructBinaryTree_2_hpp */
