//
//  KthNodeFromEnd.hpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/16.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef KthNodeFromEnd_hpp
#define KthNodeFromEnd_hpp

#include <stdio.h>
#include <cstdio>

struct ListNode {
    int m_nValue;
    ListNode* m_pNext;
};

ListNode* findKthToTail(ListNode* pListHead, unsigned int k);

// 相关题目
// 求链表的中间节点。如果链表中的节点总数为奇数，则返回中间节点；如果节点总数是偶数，
// 则返回中间两个节点的任意一个。
// 我们可以定义两个指针，同时从链表的头节点出发，一个指针一次走一步，另一个指针一次走两步。
// 当走的快的指针走到链表的末尾时，走的慢的指针正好在链表的中间。

#endif /* KthNodeFromEnd_hpp */
