//
//  SerializeBinaryTrees.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/30.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef SerializeBinaryTrees_hpp
#define SerializeBinaryTrees_hpp

#include <stdio.h>
#include <iostream>
#include <fstream>
#include "BinaryTree.hpp"

using namespace BinaryTree;
using namespace std;

namespace SerializeBinaryTrees {

// 37：序列化二叉树
// 题目：请实现两个函数，分别用来序列化和反序列化二叉树。
void serialize(const BinaryTreeNode* pRoot, ostream& stream);
bool readStream(istream& stream, int* number);
void deserialize(BinaryTreeNode** pRoot, istream& stream);

// 测试代码
bool isSameTree(const BinaryTreeNode* pRoot1, const BinaryTreeNode* pRoot2);
void Test(const char* testName, const BinaryTreeNode* pRoot);
//            8
//        6      10
//       5 7    9  11
void Test1();
//            5
//          4
//        3
//      2
void Test2();
//        5
//         4
//          3
//           2
void Test3();
void Test4();
void Test5();
void Test6();

void Test();

}

#endif /* SerializeBinaryTrees_hpp */
