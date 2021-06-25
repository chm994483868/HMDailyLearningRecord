//
//  PathInTree.cpp
//  OfferReview
//
//  Created by CHM on 2020/7/30.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "PathInTree.hpp"

// 从根节点到叶节点的完整路径
void PathInTree::findPath(BinaryTreeNode* pRoot, int expectedsum, std::vector<int>& path, int& currentSum) {
    // currentSum 加根节点的值
    currentSum += pRoot->m_nValue;
    // 把根节点放入 path vector
    path.push_back(pRoot->m_nValue);
    
    // isLeaf 判断是否是根节点
    bool isLeaf = pRoot->m_pLeft == nullptr && pRoot->m_pRight == nullptr;
    
    // 如果找到了完整路径则进行打印
    if (currentSum == expectedsum && isLeaf) {
        std::vector<int>::iterator iter = path.begin();
        for (; iter != path.end() ; ++iter) {
            printf("%d\t", *iter);
        }
        
        printf("\n");
    }
    
    // 在左子树中找路径
    if (pRoot->m_pLeft != nullptr) {
        findPath(pRoot->m_pLeft, expectedsum, path, currentSum);
    }
    
    // 在右子树中找路径
    if (pRoot->m_pRight != nullptr) {
        findPath(pRoot->m_pRight, expectedsum, path, currentSum);
    }
    
    // 如果当前完整路径的和不是 expectedsum，则回退
    currentSum -= pRoot->m_nValue;
    path.pop_back();
}

void PathInTree::findPath(BinaryTreeNode* pRoot, int expectedSum) {
    if (pRoot == nullptr) {
        return;
    }
    
    // 准备一个 vector 用来存放路径上的节点值
    std::vector<int> path;
    // 记录当前路径上节点值的和
    int currentSum = 0;
    // findPath
    findPath(pRoot, expectedSum, path, currentSum);
}

// 测试代码
void PathInTree::Test(char* testName, BinaryTreeNode* pRoot, int expectedSum) {
    if(testName != nullptr)
        printf("%s begins:\n", testName);

    findPath(pRoot, expectedSum);

    printf("\n");
}

//            10
//         /      \
//        5        12
//       /\
//      4  7
// 有两条路径上的结点和为22
void PathInTree::Test1() {
    BinaryTreeNode* pNode10 = CreateBinaryTreeNode(10);
    BinaryTreeNode* pNode5 = CreateBinaryTreeNode(5);
    BinaryTreeNode* pNode12 = CreateBinaryTreeNode(12);
    BinaryTreeNode* pNode4 = CreateBinaryTreeNode(4);
    BinaryTreeNode* pNode7 = CreateBinaryTreeNode(7);
    
    ConnectTreeNodes(pNode10, pNode5, pNode12);
    ConnectTreeNodes(pNode5, pNode4, pNode7);
    
    printf("Two paths should be found in Test1.\n");
    Test("Test1", pNode10, 22);
    
    DestroyTree(pNode10);
}

//            10
//         /      \
//        5        12
//       /\
//      4  7
// 没有路径上的结点和为15
void PathInTree::Test2() {
    BinaryTreeNode* pNode10 = CreateBinaryTreeNode(10);
    BinaryTreeNode* pNode5 = CreateBinaryTreeNode(5);
    BinaryTreeNode* pNode12 = CreateBinaryTreeNode(12);
    BinaryTreeNode* pNode4 = CreateBinaryTreeNode(4);
    BinaryTreeNode* pNode7 = CreateBinaryTreeNode(7);
    
    ConnectTreeNodes(pNode10, pNode5, pNode12);
    ConnectTreeNodes(pNode5, pNode4, pNode7);
    
    printf("No paths should be found in Test2.\n");
    Test("Test2", pNode10, 15);
    
    DestroyTree(pNode10);
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
// 有一条路径上面的结点和为15
void PathInTree::Test3() {
    BinaryTreeNode* pNode5 = CreateBinaryTreeNode(5);
    BinaryTreeNode* pNode4 = CreateBinaryTreeNode(4);
    BinaryTreeNode* pNode3 = CreateBinaryTreeNode(3);
    BinaryTreeNode* pNode2 = CreateBinaryTreeNode(2);
    BinaryTreeNode* pNode1 = CreateBinaryTreeNode(1);

    ConnectTreeNodes(pNode5, pNode4, nullptr);
    ConnectTreeNodes(pNode4, pNode3, nullptr);
    ConnectTreeNodes(pNode3, pNode2, nullptr);
    ConnectTreeNodes(pNode2, pNode1, nullptr);

    printf("One path should be found in Test3.\n");
    Test("Test3", pNode5, 15);

    DestroyTree(pNode5);
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
// 没有路径上面的结点和为16
void PathInTree::Test4() {
    BinaryTreeNode* pNode1 = CreateBinaryTreeNode(1);
    BinaryTreeNode* pNode2 = CreateBinaryTreeNode(2);
    BinaryTreeNode* pNode3 = CreateBinaryTreeNode(3);
    BinaryTreeNode* pNode4 = CreateBinaryTreeNode(4);
    BinaryTreeNode* pNode5 = CreateBinaryTreeNode(5);

    ConnectTreeNodes(pNode1, nullptr, pNode2);
    ConnectTreeNodes(pNode2, nullptr, pNode3);
    ConnectTreeNodes(pNode3, nullptr, pNode4);
    ConnectTreeNodes(pNode4, nullptr, pNode5);

    printf("No paths should be found in Test4.\n");
    Test("Test4", pNode1, 16);

    DestroyTree(pNode1);
}

// 树中只有1个结点
void PathInTree::Test5() {
    BinaryTreeNode* pNode1 = CreateBinaryTreeNode(1);

    printf("One path should be found in Test5.\n");
    Test("Test5", pNode1, 1);

    DestroyTree(pNode1);
}

// 树中没有结点
void PathInTree::Test6() {
    printf("No paths should be found in Test6.\n");
    Test("Test6", nullptr, 0);
}

void PathInTree::Test() {
    Test1();
    Test2();
    Test3();
    Test4();
    Test5();
    Test6();
}
