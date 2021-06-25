//
//  FirstCommonNodesInLists.hpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef FirstCommonNodesInLists_hpp
#define FirstCommonNodesInLists_hpp

#include <stdio.h>
#include <cstdio>
#include "List.hpp"

using namespace List;

namespace FirstCommonNodesInLists {

unsigned int getListLength(ListNode* pHead);
ListNode* findFirstCommonNode(ListNode* pHead1, ListNode* pHead2);

}
#endif /* FirstCommonNodesInLists_hpp */
