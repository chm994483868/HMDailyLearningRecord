//
//  NextNodeInBinaryTrees_2.hpp
//  OfferReview
//
//  Created by CHM on 2021/2/21.
//  Copyright © 2021 CHM. All rights reserved.
//

#ifndef NextNodeInBinaryTrees_2_hpp
#define NextNodeInBinaryTrees_2_hpp

#include <stdio.h>

namespace NextNodeInBinaryTrees_2 {

struct BinaryTreeNode {
    int m_nValue;
    BinaryTreeNode* m_pParent;
    BinaryTreeNode* m_pLeft;
    BinaryTreeNode* m_pRight;
};

BinaryTreeNode* getNext(BinaryTreeNode* pNode);

}

#endif /* NextNodeInBinaryTrees_2_hpp */
