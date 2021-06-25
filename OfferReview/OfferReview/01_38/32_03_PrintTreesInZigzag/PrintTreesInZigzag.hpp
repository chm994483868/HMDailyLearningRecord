//
//  PrintTreesInZigzag.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/30.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef PrintTreesInZigzag_hpp
#define PrintTreesInZigzag_hpp

#include <stdio.h>
#include <stack>
#include "BinaryTree.hpp"

using namespace BinaryTree;
using namespace std;

namespace PrintTreesInZigzag {

// 32（三）：之字形打印二叉树
// 题目：请实现一个函数按照之字形顺序打印二叉树，即第一行按照从左到右的顺
// 序打印，第二层按照从右到左的顺序打印，第三行再按照从左到右的顺序打印，
// 其他行以此类推。
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
//                8
//        4              12
//     2     6       10      14
//   1  3  5  7     9 11   13  15
void Test7();

void Test();

}

#endif /* PrintTreesInZigzag_hpp */
