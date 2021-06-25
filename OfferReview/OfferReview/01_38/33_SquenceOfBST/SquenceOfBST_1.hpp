//
//  SquenceOfBST_1.hpp
//  OfferReview
//
//  Created by HM C on 2020/11/5.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef SquenceOfBST_1_hpp
#define SquenceOfBST_1_hpp

#include <stdio.h>

namespace SquenceOfBST_1 {

// 33：二叉搜索树的后序遍历序列
// 题目：输入一个整数数组，判断该数组是不是某二叉搜索树的后序遍历的结果。
// 如果是则返回 true，否则返回 false。假设输入的数组的任意两个数字都互不相同。

bool verifySquenceOfBST(int sequence[], int length);

// 测试代码
void Test(const char* testName, int sequence[], int length, bool expected);
//            10
//         /      \
//        6        14
//       /\        /\
//      4  8     12  16
void Test1();
//           5
//          / \
//         4   7
//            /
//           6
void Test2();
//               5
//              /
//             4
//            /
//           3
//          /
//         2
//        /
//       1
void Test3();
// 1
//  \
//   2
//    \
//     3
//      \
//       4
//        \
//         5
void Test4();
// 树中只有1个结点
void Test5();
void Test6();
void Test7();
void Test8();

void Test();

}

#endif /* SquenceOfBST_1_hpp */
