//
//  AssignmentOperator_2.cpp
//  OfferReview
//
//  Created by CHM on 2021/1/21.
//  Copyright © 2021 CHM. All rights reserved.
//

#include "AssignmentOperator_2.hpp"

AssignmentOperator_2::CMyString::CMyString(const char* pData) {
    if (pData == nullptr) {
        m_pData = new char[1];
        m_pData[0] = '\0';
    } else {
        unsigned long length = strlen(pData);
        m_pData = new char[length + 1];
        strcpy(m_pData, pData);
    }
}

AssignmentOperator_2::CMyString::CMyString(const CMyString& str) {
    unsigned long length = strlen(str.m_pData);
    m_pData = new char[length + 1];
    strcpy(m_pData, str.m_pData);
}

AssignmentOperator_2::CMyString::~CMyString() {
    delete [] m_pData;
}

AssignmentOperator_2::CMyString& AssignmentOperator_2::CMyString::operator=(const CMyString& str) {
//    if (this == &str) {
//        return *this;
//    }
//
//    delete [] m_pData;
//    m_pData = nullptr;
//    m_pData = new char[strlen(str.m_pData) + 1];
//    strcpy(m_pData, str.m_pData);
//
//    return *this;
    
    if (this != &str) {
        CMyString strTemp(str);
        char* pTemp = strTemp.m_pData;
        strTemp.m_pData = m_pData;
        m_pData = pTemp;
    }
    
    return *this;
}

void AssignmentOperator_2::CMyString::Print() {
    printf("%s", m_pData);
}
