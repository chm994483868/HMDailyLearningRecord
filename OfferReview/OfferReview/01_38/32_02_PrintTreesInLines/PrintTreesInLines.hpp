//
//  PrintTreesInLines.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/30.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef PrintTreesInLines_hpp
#define PrintTreesInLines_hpp

#include <stdio.h>
#include <deque>
#include "BinaryTree.hpp"

using namespace BinaryTree;
using namespace std;

namespace PrintTreesInLines {

// 32（二）：分行从上到下打印二叉树
// 题目：从上到下按层打印二叉树，同一层的结点按从左到右的顺序打印，每一层
// 打印到一行。
void print(BinaryTreeNode* pRoot);

// 测试代码
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
//        100
//        /
//       50
//         \
//         150
void Test6();

void Test();

}

#endif /* PrintTreesInLines_hpp */
