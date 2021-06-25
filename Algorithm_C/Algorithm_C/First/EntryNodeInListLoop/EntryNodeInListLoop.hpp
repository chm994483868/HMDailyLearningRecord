//
//  EntryNodeInListLoop.hpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/16.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef EntryNodeInListLoop_hpp
#define EntryNodeInListLoop_hpp

#include <stdio.h>
#include <cstdio>

struct ListNode {
    int m_nValue;
    ListNode* m_pNext;
};

ListNode* meetingNode(ListNode* pHead);
ListNode* entryNodeOfLoop(ListNode* pHead);

#endif /* EntryNodeInListLoop_hpp */
