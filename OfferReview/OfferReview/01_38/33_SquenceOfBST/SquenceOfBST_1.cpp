//
//  SquenceOfBST_1.cpp
//  OfferReview
//
//  Created by HM C on 2020/11/5.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "SquenceOfBST_1.hpp"

// 二叉树搜索树的特点是：所有的左子节点都小于父节点，所有的右子节点都大于父节点
// 二叉树的后序遍历序列是左右根则顺序，则首先可断定的是后序序列的最后一个元素一定是树的根节点
bool SquenceOfBST_1::verifySquenceOfBST(int sequence[], int length) {
    if (sequence == nullptr || length <= 0) {
        return false;
    }
    
    // 因为是后序序列，所以最后一个元素一定是二叉树的根节点，首先取出根节点的值
    int nRootValue = sequence[length - 1];
    
    // 在后序序列中找到左子树中的根节点的位置的后面一个位置，已知左子树的节点全部小于根节点，
    // 右子树的节点全部大于根节点，所以可通过便利 [0, length-1) 找到左子树和右子树的分界
    unsigned int nLeftIndexEnd = 0;
    for (; nLeftIndexEnd < length - 1; ++nLeftIndexEnd) {
        if (sequence[nLeftIndexEnd] > nRootValue) {
            break;
        }
    }
    
    // 这里是判断所有右子树的节点都大于根节点，否则直接返回 false
    unsigned int nRightStart = nLeftIndexEnd;
    for (; nRightStart < length - 1; ++nRightStart) {
        if (sequence[nRightStart] < nRootValue) {
            return false;
        }
    }
    
    // 下面开始递归判断左右子树后序序列是否符合二叉搜索树的规则
    bool bLeft = true;
    // 如果 nLeftIndexEnd 大于 0 表示存在左子树，下面是递归判断左子树是否也是二叉搜索树
    if (nLeftIndexEnd > 0) {
        bLeft = verifySquenceOfBST(sequence, nLeftIndexEnd);
    }
    
    bool bRight = true;
    // 如果 nLeftIndexEnd 小于 length - 1 表示存在右子树，下面是递归判断右子树是否也是二叉搜索树
    if (nLeftIndexEnd < length - 1) {
        bRight = verifySquenceOfBST(sequence + nLeftIndexEnd, length - nLeftIndexEnd - 1);
    }
    
    // 返回值（左右子树必须全部都是二叉搜索树）
    return bLeft && bRight;
}

// 测试代码
void SquenceOfBST_1::Test(const char* testName, int sequence[], int length, bool expected) {
    if(testName != nullptr)
        printf("%s begins: ", testName);
    
    if(verifySquenceOfBST(sequence, length) == expected)
        printf("passed.\n");
    else
        printf("failed.\n");
}

//            10
//         /      \
//        6        14
//       /\        /\
//      4  8     12  16
void SquenceOfBST_1::Test1() {
    int data[] = {4, 8, 6, 12, 16, 14, 10};
    Test("Test1", data, sizeof(data)/sizeof(int), true);
}

//           5
//          / \
//         4   7
//            /
//           6
void SquenceOfBST_1::Test2() {
    int data[] = {4, 6, 7, 5};
    Test("Test2", data, sizeof(data)/sizeof(int), true);
}

//               5
//              /
//             4
//            /
//           3
//          /
//         2
//        /
//       1
void SquenceOfBST_1::Test3() {
    int data[] = {1, 2, 3, 4, 5};
    Test("Test3", data, sizeof(data)/sizeof(int), true);
}

// 1
//  \
//   2
//    \
//     3
//      \
//       4
//        \
//         5
void SquenceOfBST_1::Test4() {
    int data[] = {5, 4, 3, 2, 1};
    Test("Test4", data, sizeof(data)/sizeof(int), true);
}

// 树中只有1个结点
void SquenceOfBST_1::Test5() {
    int data[] = {5};
    Test("Test5", data, sizeof(data)/sizeof(int), true);
}

void SquenceOfBST_1::Test6() {
    int data[] = {7, 4, 6, 5};
    Test("Test6", data, sizeof(data)/sizeof(int), false);
}

void SquenceOfBST_1::Test7() {
    int data[] = {4, 6, 12, 8, 16, 14, 10};
    Test("Test7", data, sizeof(data)/sizeof(int), false);
}

void SquenceOfBST_1::Test8() {
    Test("Test8", nullptr, 0, false);
}

void SquenceOfBST_1::Test() {
    Test1();
    Test2();
    Test3();
    Test4();
    Test5();
    Test6();
    Test7();
    Test8();
}
