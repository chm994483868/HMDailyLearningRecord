//
//  SerializeBinaryTrees.hpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/20.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef SerializeBinaryTrees_hpp
#define SerializeBinaryTrees_hpp

#include <stdio.h>
#include <iostream>
#include <fstream>

using namespace std;

struct BinaryTreeNode {
    int m_nValue;
    BinaryTreeNode* m_pLeft;
    BinaryTreeNode* m_pRight;
};

void serialize(const BinaryTreeNode* pRoot, ostream& stream);
bool readStream(istream& stream, int* number);
void deserialize(BinaryTreeNode** pRoot, istream& stream);

#endif /* SerializeBinaryTrees_hpp */
