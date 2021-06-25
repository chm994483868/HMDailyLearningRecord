//
//  SquenceOfBST.cpp
//  Algorithm_C
//
//  Created by HM C on 2020/7/18.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "SquenceOfBST.hpp"

bool verifySquenceOfBST(int sequence[], int length) {
    if (sequence == nullptr || length <= 0)
        return false;
    
    // 1. 取得根节点。
    int root = sequence[length - 1];
    
    // 2. 在序列中找到所有左子树的节点
    // 在二叉搜索树中所有左子树的节点小于根节点
    // 这里 i 最后到达的是一个大于 root 的节点
    // 即 i 到达的是右子树的最小值的节点
    int i = 0;
    for (; i < length - 1; ++i) {
        if (sequence[i] > root) {
            break;
        }
    }
    
    // 在二叉搜索树中右子树的结点大于根节点
    int j = i;
    for(; j < length - 1; ++j) {
        // 这里遍历右子树的节点，如果有任何一个节点的值小于 root
        // 即表面当前不符合二叉搜索树
        if (sequence[j] < root) {
            return false;
        }
    }
    
    // 下面是两个递归，分别判断左右子树是不是二叉搜索树
    
    // 判断左子树是不是二叉搜索树
    bool left = true;
    if (i > 0) // i > 0 表示左子树至少要有一个节点
        left = verifySquenceOfBST(sequence, i);
    
    // 判断右子树是不是二叉搜索树
    bool right = true;
    if (i < length - 1) // i 小于 length - 1 表示至少右子树要有一个元素
        right = verifySquenceOfBST(sequence + i, length - i - 1);
    
    return (left && right);
}

//bool verifySquenceOfBST_Review(int sequence[], int length) {
//
//}
