//
//  PrintListInReversedOrder_2.hpp
//  OfferReview
//
//  Created by CHM on 2021/2/21.
//  Copyright © 2021 CHM. All rights reserved.
//

#ifndef PrintListInReversedOrder_2_hpp
#define PrintListInReversedOrder_2_hpp

#include <stdio.h>
#include <stack>
#include "List.hpp"

using namespace List;
using namespace std;

namespace PrintListInReversedOrder_2 {

void addToTail(ListNode** pHead, int value);
void removeNode(ListNode** pHead, int value);

void printListReversingly_Iteratively(ListNode* pHead);
void printListReversingly_Recursively(ListNode* pHead);

}

#endif /* PrintListInReversedOrder_2_hpp */
