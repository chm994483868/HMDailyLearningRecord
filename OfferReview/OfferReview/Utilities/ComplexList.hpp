//
//  ComplexList.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/30.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef ComplexList_hpp
#define ComplexList_hpp

#include <stdio.h>

namespace ComplexList {

struct ComplexListNode {
    int m_nValue;
    ComplexListNode* m_pNext;
    ComplexListNode* m_pSibling;
};

ComplexListNode* CreateNode(int nValue);
void BuildNodes(ComplexListNode* pNode, ComplexListNode* pNext, ComplexListNode* pSibling);
void PrintList(ComplexListNode* pHead);

}

#endif /* ComplexList_hpp */
