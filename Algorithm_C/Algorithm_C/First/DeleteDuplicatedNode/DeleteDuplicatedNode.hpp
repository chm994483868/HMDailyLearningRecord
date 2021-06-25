//
//  DeleteDuplicatedNode.hpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/15.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef DeleteDuplicatedNode_hpp
#define DeleteDuplicatedNode_hpp

#include <stdio.h>
#include <cstdio>

struct ListNode {
    int m_nValue;
    ListNode* m_pNext;
};

void deleteDuplication(ListNode** pHead);

void startTest_DeleteDuplication();

#endif /* DeleteDuplicatedNode_hpp */
