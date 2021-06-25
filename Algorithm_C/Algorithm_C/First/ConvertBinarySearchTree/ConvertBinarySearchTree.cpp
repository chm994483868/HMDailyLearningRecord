//
//  ConvertBinarySearchTree.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/20.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "ConvertBinarySearchTree.hpp"

void convertNode(BinaryTreeNode* pNode, BinaryTreeNode** pLastNodeInList);

BinaryTreeNode* convert(BinaryTreeNode* pRootOfTree) {
    // 1. 这两行的处理极为重要
    BinaryTreeNode* pLastNodeInList = nullptr;
    convertNode(pRootOfTree, &pLastNodeInList);
    
    // 2. 这里只是为了找到双向链表的头
    BinaryTreeNode* pHeadOfList = pLastNodeInList;
    while (pHeadOfList != nullptr && pHeadOfList->m_pLeft != nullptr) {
        pHeadOfList = pHeadOfList->m_pLeft;
    }
    
    return pHeadOfList;
}

void convertNode(BinaryTreeNode* pNode, BinaryTreeNode** pLastNodeInList) {
    if (pNode == nullptr) {
        return;
    }
    
    // 1. 这里一直对左子树进行递归
    BinaryTreeNode* pCurrent = pNode;
    if (pCurrent->m_pLeft != nullptr) {
        convertNode(pCurrent->m_pLeft, pLastNodeInList);
    }
    
    // 2. 当前节点的左子树指针指向 pLastNodeInList
    pCurrent->m_pLeft = *pLastNodeInList;
    
    // 3. 如果 pLastNodeInList 不为空的时候，把它的右子树指针指向 pCurrent;
    if (*pLastNodeInList != nullptr) {
        (*pLastNodeInList)->m_pRight = pCurrent;
    }
    
    // 4. 更新 pLastNodeInList 为当前的指针 pCurrent
    *pLastNodeInList = pCurrent;
    
    // 5. 对右子树进行同样的递归
    if (pCurrent->m_pRight != nullptr) {
        convertNode(pCurrent->m_pRight, pLastNodeInList);
    }
}

void convertNode_Review(BinaryTreeNode* pNode, BinaryTreeNode** pLastNodeInList);

BinaryTreeNode* convertzhi_Review(BinaryTreeNode* pRootOfTree) {
    // 1. 准备一个指向链表最后一个节点的指针
    BinaryTreeNode* pLastNodeInList = nullptr;
    convertNode_Review(pRootOfTree, &pLastNodeInList);
    
    // 2. 找到头节点
    BinaryTreeNode* pHeadOfList = pLastNodeInList;
    while (pHeadOfList != nullptr && pHeadOfList->m_pLeft != nullptr) {
        pHeadOfList = pHeadOfList->m_pLeft;
    }
    
    return pHeadOfList;
}

void convertNode_Review(BinaryTreeNode* pNode, BinaryTreeNode** pLastNodeInList) {
    if (pNode == nullptr) {
        return;
    }
    
    BinaryTreeNode* pCurrent = pNode;
    // 1. 左子节点递归
    if (pCurrent->m_pLeft != nullptr) {
        convertNode_Review(pCurrent->m_pLeft, pLastNodeInList);
    }
    
    // 通过 pLastNodeInList 串起所有节点
    // m_pLeft 指向前 m_pRight 指向后
    // <------ ||| ----->
    pCurrent->m_pLeft = *pLastNodeInList;
    if (*pLastNodeInList != nullptr) {
        (*pLastNodeInList)->m_pRight = pCurrent;
    }
    *pLastNodeInList = pCurrent;
    
    // 2. 右子节点递归
    if (pCurrent->m_pRight != nullptr) {
        convertNode_Review(pCurrent->m_pRight, pLastNodeInList);
    }
}

// fun(5)

// 4
// 3
// 2
// 1
// end
// 顺序向下执行 fun2(1)
// end <== fun2
// 顺序向下执行 fun2(2)
// 1   <== fun2
// end <== fun1
// end <== fun2
// 顺序向下执行 fun2(3)
// 2   <== fun2
// 1   <== fun1
// end <== fun1
// end <== fun2
// 1   <== fun2
// end <== fun1
// end <== fun2
// 顺序向下执行 fun2(4)
// 3
// 2
// 1
// end
// end
// 1
// end
// end
// 2
// 1
// end
// end
// 1
// end
// end

// rec(11)
void rec_TEST(int n) {
    if (n > 0) {
        rec_TEST(n - 1);
    }
    
    printf("n = %d\t", n);
    printf("最后一句了");
}
