//
//  ReverseList.hpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/16.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef ReverseList_hpp
#define ReverseList_hpp

#include <stdio.h>
#include <cstdio>

struct ListNode {
    int m_nValue;
    ListNode* m_pNext;
};

ListNode* reverseList(ListNode* pHead);

#endif /* ReverseList_hpp */
