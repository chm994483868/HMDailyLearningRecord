//
//  ConstructBinaryTree_1.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/1.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "ConstructBinaryTree_1.hpp"

ConstructBinaryTree_1::BinaryTreeNode* ConstructBinaryTree_1::constructCore(int* startPreorder, int* endPreorder, int* startInorder, int* endInorder) {
    int rootValue = startPreorder[0];
    BinaryTreeNode* root = new BinaryTreeNode();
    root->m_nValue = rootValue;
    root->m_pLeft = root->m_pRight = nullptr;
    
    if (startPreorder == endPreorder) {
        if (startInorder == endInorder && *startPreorder == *startInorder) {
            return root;
        } else {
            throw std::exception(); // 入参错误
        }
    }
    
    int* rootInorder = startInorder;
    while (rootInorder <= endInorder && *rootInorder != rootValue) {
        ++rootInorder;
    }
    
    if (rootInorder == endInorder && *rootInorder != rootValue) {
        throw std::exception();
    }
    
    long leftLength = rootInorder - startInorder;
    int* leftPreorderEnd = startPreorder + leftLength;
    if (leftLength > 0) {
        root->m_pLeft = constructCore(startPreorder + 1, leftPreorderEnd, startInorder, rootInorder - 1);
    }
    
    if (leftLength < endPreorder - startPreorder) {
        root->m_pRight = constructCore(leftPreorderEnd + 1, endPreorder, rootInorder + 1, endInorder);
    }
    
    return root;
}

ConstructBinaryTree_1::BinaryTreeNode* ConstructBinaryTree_1::construct(int* preorder, int* inorder, int length) {
    if (preorder == nullptr || inorder == nullptr || length <= 0) {
        return nullptr;
    }
    
    return constructCore(preorder, preorder + length - 1, inorder, inorder + length - 1);
}

// 辅助函数
ConstructBinaryTree_1::BinaryTreeNode* ConstructBinaryTree_1::CreateBinaryTreeNode(int value) {
    BinaryTreeNode* pNode = new BinaryTreeNode();
    pNode->m_nValue = value;
    pNode->m_pLeft = nullptr;
    pNode->m_pRight = nullptr;

    return pNode;
}

void ConstructBinaryTree_1::ConnectTreeNodes(BinaryTreeNode* pParent, BinaryTreeNode* pLeft, BinaryTreeNode* pRight) {
    if(pParent != nullptr) {
        pParent->m_pLeft = pLeft;
        pParent->m_pRight = pRight;
    }
}

void ConstructBinaryTree_1::PrintTreeNode(const BinaryTreeNode* pNode) {
    if(pNode != nullptr) {
        printf("value of this node is: %d\n", pNode->m_nValue);

        if(pNode->m_pLeft != nullptr)
            printf("value of its left child is: %d.\n", pNode->m_pLeft->m_nValue);
        else
            printf("left child is nullptr.\n");

        if(pNode->m_pRight != nullptr)
            printf("value of its right child is: %d.\n", pNode->m_pRight->m_nValue);
        else
            printf("right child is nullptr.\n");
    } else {
        printf("this node is nullptr.\n");
    }

    printf("\n");
}

void ConstructBinaryTree_1::PrintTree(const BinaryTreeNode* pRoot) {
    PrintTreeNode(pRoot);

    if (pRoot != nullptr) {
        if(pRoot->m_pLeft != nullptr)
            PrintTree(pRoot->m_pLeft);

        if(pRoot->m_pRight != nullptr)
            PrintTree(pRoot->m_pRight);
    }
}
void ConstructBinaryTree_1::DestroyTree(BinaryTreeNode* pRoot) {
    if (pRoot != nullptr) {
        BinaryTreeNode* pLeft = pRoot->m_pLeft;
        BinaryTreeNode* pRight = pRoot->m_pRight;

        delete pRoot;
        pRoot = nullptr;

        DestroyTree(pLeft);
        DestroyTree(pRight);
    }
}

// 测试代码
void ConstructBinaryTree_1::Test(char* testName, int* preorder, int* inorder, int length) {
    if(testName != nullptr)
        printf("%s begins:\n", testName);

    printf("The preorder sequence is: ");
    for(int i = 0; i < length; ++ i)
        printf("%d ", preorder[i]);
    printf("\n");

    printf("The inorder sequence is: ");
    for(int i = 0; i < length; ++ i)
        printf("%d ", inorder[i]);
    printf("\n");

    try {
        BinaryTreeNode* root = construct(preorder, inorder, length);
        PrintTree(root);

        DestroyTree(root);
    } catch(std::exception& exception) {
        printf("Invalid Input.\n");
    }
}

// 普通二叉树
//              1
//           /     \
//          2       3
//         /       / \
//        4       5   6
//         \         /
//          7       8
void ConstructBinaryTree_1::Test1() {
    const int length = 8;
    int preorder[length] = {1, 2, 4, 7, 3, 5, 6, 8};
    int inorder[length] = {4, 7, 2, 1, 5, 3, 8, 6};

    Test("Test1", preorder, inorder, length);
}

// 所有结点都没有右子结点
//            1
//           /
//          2
//         /
//        3
//       /
//      4
//     /
//    5
void ConstructBinaryTree_1::Test2() {
    const int length = 5;
    int preorder[length] = {1, 2, 3, 4, 5};
    int inorder[length] = {5, 4, 3, 2, 1};

    Test("Test2", preorder, inorder, length);
}

// 所有结点都没有左子结点
//            1
//             \
//              2
//               \
//                3
//                 \
//                  4
//                   \
//                    5
void ConstructBinaryTree_1::Test3() {
    const int length = 5;
    int preorder[length] = {1, 2, 3, 4, 5};
    int inorder[length] = {1, 2, 3, 4, 5};

    Test("Test3", preorder, inorder, length);
}

// 树中只有一个结点
void ConstructBinaryTree_1::Test4() {
    const int length = 1;
    int preorder[length] = {1};
    int inorder[length] = {1};
    
    Test("Test4", preorder, inorder, length);
}

// 完全二叉树
//              1
//           /     \
//          2       3
//         / \     / \
//        4   5   6   7
void ConstructBinaryTree_1::Test5() {
    const int length = 7;
    int preorder[length] = {1, 2, 4, 5, 3, 6, 7};
    int inorder[length] = {4, 2, 5, 1, 6, 3, 7};

    Test("Test5", preorder, inorder, length);
}

// 输入空指针
void ConstructBinaryTree_1::Test6() {
    Test("Test6", nullptr, nullptr, 0);
}

// 输入的两个序列不匹配
void ConstructBinaryTree_1::Test7() {
    const int length = 7;
    int preorder[length] = {1, 2, 4, 5, 3, 6, 7};
    int inorder[length] = {4, 2, 8, 1, 6, 3, 7};

    Test("Test7: for unmatched input", preorder, inorder, length);
}

void ConstructBinaryTree_1::Test() {
    Test1();
    Test2();
    Test3();
    Test4();
    Test5();
    Test6();
    Test7();
}

class Solution {
    struct TreeNode {
        int val;
        TreeNode* left;
        TreeNode* right;
        TreeNode(int x) : val(x), left(NULL), right(NULL) {}
    };
    
public:
    TreeNode* buildTree(vector<int>& preorder, vector<int>& inorder) {
        if (preorder.empty() || inorder.empty()) {
            return nullptr;
        }
        
        int rootValue = preorder[0];
        TreeNode* root = new TreeNode(rootValue);
        
        if (preorder.size() == 1) {
            if (inorder.size() == 1 && preorder[0] == inorder[0]) {
                return root;
            } else {
                return nullptr;
            }
        }
        
        int rootInorderIndex = -1;
        for (int i = 0; i < inorder.size(); ++i) {
            if (inorder[i] == rootValue) {
                rootInorderIndex = i;
                break;
            }
        }
        
        if (rootInorderIndex == -1) {
            return nullptr;
        }
        
        int numOfLeftTreeNode = rootInorderIndex + 1;
        if (rootInorderIndex > 0) {
            vector<int> leftPreorder;
            leftPreorder.assign(preorder.begin() + 1, preorder.begin() + numOfLeftTreeNode);
            
            vector<int> leftInorder;
            leftInorder.assign(inorder.begin(), inorder.begin() + numOfLeftTreeNode);
            
            root->left = buildTree(leftPreorder, leftInorder);
        }
        
        if (rootInorderIndex < (inorder.size() - 1)) {
            vector<int> rightPreorder;
            rightPreorder.assign(preorder.begin() + numOfLeftTreeNode + 1, preorder.end());
            
            vector<int> rightInorder;
            rightInorder.assign(inorder.begin() + numOfLeftTreeNode + 2, inorder.end());
            
            root->right = buildTree(rightPreorder, rightInorder);
        }
        
        return root;
    }
};
