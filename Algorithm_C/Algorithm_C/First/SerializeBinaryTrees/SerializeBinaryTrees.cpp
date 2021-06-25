//
//  SerializeBinaryTrees.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/20.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "SerializeBinaryTrees.hpp"

void serialize(const BinaryTreeNode* pRoot, ostream& stream) {
    if (pRoot == nullptr) {
        stream << "$,";
        return;
    }
    
    stream << pRoot->m_nValue << ',';
    serialize(pRoot->m_pLeft, stream);
    serialize(pRoot->m_pRight, stream);
}

bool readStream(istream& stream, int* number) {
    if (stream.eof()) {
        return false;
    }
    
    char buffer[32];
    buffer[0] = '\0';
    
    char ch;
    stream >> ch;
    int i = 0;
    while (!stream.eof() && ch != ',') {
        buffer[i++] = ch;
        stream >> ch;
    }
    
    bool isNumeric = false;
    if (i > 0 && buffer[0] != '$') {
        *number = atoi(buffer);
        isNumeric = true;
    }
    
    return isNumeric;
}

void deserialize(BinaryTreeNode** pRoot, istream& stream) {
    int number;
    if (readStream(stream, &number)) {
        *pRoot = new BinaryTreeNode();
        (*pRoot)->m_nValue = number;
        (*pRoot)->m_pLeft = nullptr;
        (*pRoot)->m_pRight = nullptr;
        
        deserialize(&((*pRoot)->m_pLeft), stream);
        deserialize(&((*pRoot)->m_pRight), stream);
    }
}

bool readStream_Review(istream& stream, int* number) {
    if (stream.eof()) {
        return false;
    }
    
    char buffer[32];
    buffer[0] = '\0';
    
    char ch;
    stream >> ch;
    int i = 0;
    while (!stream.eof() && ch != ',') {
        buffer[i++] = ch;
        stream >> ch;
    }
    
    bool isNumeric = false;
    if (i > 0 && buffer[0] != '$') {
        *number = atoi(buffer);
        isNumeric = true;
    }
    
    return isNumeric;
}

void serialize_Review(const BinaryTreeNode* pRoot, ostream& stream) {
    if (pRoot == nullptr) {
        stream << "$,";
        return;
    }
    
    stream << pRoot->m_nValue << ',';
    serialize_Review(pRoot->m_pLeft, stream);
    serialize_Review(pRoot->m_pRight, stream);
}

void deserialize_Review(BinaryTreeNode** pRoot, istream& stream) {
    int number;
    if (readStream(stream, &number)) {
        *pRoot = new BinaryTreeNode();
        (*pRoot)->m_nValue = number;
        (*pRoot)->m_pLeft = nullptr;
        (*pRoot)->m_pRight = nullptr;
        
        deserialize_Review(&((*pRoot)->m_pLeft), stream);
        deserialize_Review(&((*pRoot)->m_pRight), stream);
    }
}
