//
//  DeleteNodeInList.hpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/15.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef DeleteNodeInList_hpp
#define DeleteNodeInList_hpp

#include <stdio.h>
#include <cstdio>

struct ListNode {
    int m_nValue;
    ListNode* m_pNext;
};

void deleteNode(ListNode** pListHead, ListNode* pToBeDeleted);

void startTest_DeleteNode();

#endif /* DeleteNodeInList_hpp */
