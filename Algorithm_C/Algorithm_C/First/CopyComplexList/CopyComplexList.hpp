//
//  CopyComplexList.hpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/20.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef CopyComplexList_hpp
#define CopyComplexList_hpp

#include <stdio.h>
#include <cstdio>

struct ComplexListNode {
    int m_nValue;
    ComplexListNode* m_pNext;
    ComplexListNode* m_pSibling;
    
    ComplexListNode(int nValue) : m_nValue(nValue), m_pNext(nullptr), m_pSibling(nullptr) {}
};

ComplexListNode* Clone(ComplexListNode* pHead);

void startTest_ComplexListNode_Clone();

#endif /* CopyComplexList_hpp */
