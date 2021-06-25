//
//  List.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/29.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef List_hpp
#define List_hpp

#include <stdio.h>
#include <stdlib.h>

namespace List {

struct ListNode {
    int m_nValue;
    ListNode* m_pNext;
};

// 辅助代码
ListNode* CreateListNode(int value);
void ConnectListNodes(ListNode* pCurrent, ListNode* pNext);
void PrintListNode(ListNode* pNode);
void PrintList(ListNode* pHead);
void DestroyList(ListNode* pHead);

}

#endif /* List_hpp */
