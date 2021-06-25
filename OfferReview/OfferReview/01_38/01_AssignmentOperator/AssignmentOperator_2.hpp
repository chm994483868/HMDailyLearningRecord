//
//  AssignmentOperator_2.hpp
//  OfferReview
//
//  Created by CHM on 2021/1/21.
//  Copyright © 2021 CHM. All rights reserved.
//

#ifndef AssignmentOperator_2_hpp
#define AssignmentOperator_2_hpp

#include <stdio.h>
#include <cstdio>
#include <cstring>

using namespace std;

namespace AssignmentOperator_2 {

class CMyString {
public:
    CMyString(const char* pData = nullptr);
    CMyString(const CMyString& str);
    ~CMyString(void);
    
    CMyString& operator = (const CMyString& str);
    
    void Print();
    
private:
    char* m_pData;
};

}

#endif /* AssignmentOperator_2_hpp */
