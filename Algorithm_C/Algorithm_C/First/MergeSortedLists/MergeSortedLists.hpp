//
//  MergeSortedLists.hpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/16.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef MergeSortedLists_hpp
#define MergeSortedLists_hpp

#include <stdio.h>
#include <cstdio>

struct ListNode {
    int m_nValue;
    ListNode* m_pNext;
};

ListNode* merge(ListNode* pHead1, ListNode* pHead2);

#endif /* MergeSortedLists_hpp */
