//
//  SerializeBinaryTrees.cpp
//  OfferReview
//
//  Created by CHM on 2020/7/30.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "SerializeBinaryTrees.hpp"

// 序列化二叉树
void SerializeBinaryTrees::serialize(const BinaryTreeNode* pRoot, ostream& stream) {
    // 如果根节点是 nullptr，则输出一个 $ 并 return，
    // 同时它还是递归结束的的条件
    if (pRoot == nullptr) {
        stream << "$,";
        return;
    }
    
    // 输出节点的值和一个逗号
    stream << pRoot->m_nValue << ',';
    
    // 递归序列化左子树
    serialize(pRoot->m_pLeft, stream);
    
    // 递归序列化右子树
    serialize(pRoot->m_pRight, stream);
}

// readStream 每次从流中读出一个数字或者一个字符 '$'，
// 当从流中读出的是一个数字时，函数返回 true，否则返回 false
bool SerializeBinaryTrees::readStream(istream& stream, int* number) {
    // 流结束
    if(stream.eof())
        return false;
    
    // 长度是 32 的 char 数组
    char buffer[32];
    // 空字符
    buffer[0] = '\0';
    
    char ch;
    // 键盘输入到 ch 中
    stream >> ch;
    int i = 0;
    // 输入逗号表示一个完整的字符输入结束
    while(!stream.eof() && ch != ',') {
        buffer[i++] = ch;
        stream >> ch;
    }
    
    // 当输入 $ 时表示一个 nullptr 节点，否则就是正常的节点的值
    bool isNumeric = false;
    if(i > 0 && buffer[0] != '$') {
    
    // atoi (表示 ascii to integer) 是把字符串转换成整型数的一个函数。
    // int atoi(const char *nptr) 函数会扫描参数 nptr字符串，
    // 会跳过前面的空白字符（例如空格，tab缩进）等。
    // 如果 nptr 不能转换成 int 或者 nptr 为空字符串，那么将返回 0 [1]。
    // 特别注意，该函数要求被转换的字符串是按十进制数理解的。
    // atoi输入的字符串对应数字存在大小限制（与 int 类型大小有关），若其过大可能报错-1。
    
        *number = atoi(buffer);
        isNumeric = true;
    }
    
    return isNumeric;
}

// 反序列化二叉树
void SerializeBinaryTrees::deserialize(BinaryTreeNode** pRoot, istream& stream) {
    int number;
    // 注意这里 pRoot 是 BinaryTreeNode**
    // 如果读出了一个数字，则构建节点
    if (readStream(stream, &number)) {
        // 构建新节点
        *pRoot = new BinaryTreeNode();
        (*pRoot)->m_nValue = number;
        (*pRoot)->m_pLeft = nullptr;
        (*pRoot)->m_pRight = nullptr;
        
        // 传入左子节点的指针地址递归
        deserialize(&((*pRoot)->m_pLeft), stream);
        // 传入右子节点的指针地址递归
        deserialize(&((*pRoot)->m_pRight), stream);
    }
}

// 测试代码
bool SerializeBinaryTrees::isSameTree(const BinaryTreeNode* pRoot1, const BinaryTreeNode* pRoot2) {
    if(pRoot1 == nullptr && pRoot2 == nullptr)
        return true;

    if(pRoot1 == nullptr || pRoot2 == nullptr)
        return false;

    if(pRoot1->m_nValue != pRoot2->m_nValue)
        return false;

    return isSameTree(pRoot1->m_pLeft, pRoot2->m_pLeft) &&
        isSameTree(pRoot1->m_pRight, pRoot2->m_pRight);
}

void SerializeBinaryTrees::Test(const char* testName, const BinaryTreeNode* pRoot) {
    if(testName != nullptr)
        printf("%s begins: \n", testName);

    PrintTree(pRoot);

    char* fileName = "test.txt";
    ofstream fileOut;
    fileOut.open(fileName);

    serialize(pRoot, fileOut);
    fileOut.close();

    // print the serialized file
    ifstream fileIn1;
    char ch;
    fileIn1.open(fileName);
    while(!fileIn1.eof())
    {
        fileIn1 >> ch;
        cout << ch;
    }
    fileIn1.close();
    cout << endl;

    ifstream fileIn2;
    fileIn2.open(fileName);
    BinaryTreeNode* pNewRoot = nullptr;
    deserialize(&pNewRoot, fileIn2);
    fileIn2.close();

    PrintTree(pNewRoot);

    if(isSameTree(pRoot, pNewRoot))
        printf("The deserialized tree is same as the oritinal tree.\n\n");
    else
        printf("The deserialized tree is NOT same as the oritinal tree.\n\n");

    DestroyTree(pNewRoot);
}

//            8
//        6      10
//       5 7    9  11
void SerializeBinaryTrees::Test1() {
    BinaryTreeNode* pNode8 = CreateBinaryTreeNode(8);
    BinaryTreeNode* pNode6 = CreateBinaryTreeNode(6);
    BinaryTreeNode* pNode10 = CreateBinaryTreeNode(10);
    BinaryTreeNode* pNode5 = CreateBinaryTreeNode(5);
    BinaryTreeNode* pNode7 = CreateBinaryTreeNode(7);
    BinaryTreeNode* pNode9 = CreateBinaryTreeNode(9);
    BinaryTreeNode* pNode11 = CreateBinaryTreeNode(11);

    ConnectTreeNodes(pNode8, pNode6, pNode10);
    ConnectTreeNodes(pNode6, pNode5, pNode7);
    ConnectTreeNodes(pNode10, pNode9, pNode11);

    Test("Test1", pNode8);

    DestroyTree(pNode8);
}

//            5
//          4
//        3
//      2
void SerializeBinaryTrees::Test2() {
    BinaryTreeNode* pNode5 = CreateBinaryTreeNode(5);
    BinaryTreeNode* pNode4 = CreateBinaryTreeNode(4);
    BinaryTreeNode* pNode3 = CreateBinaryTreeNode(3);
    BinaryTreeNode* pNode2 = CreateBinaryTreeNode(2);
    
    ConnectTreeNodes(pNode5, pNode4, nullptr);
    ConnectTreeNodes(pNode4, pNode3, nullptr);
    ConnectTreeNodes(pNode3, pNode2, nullptr);
    
    Test("Test2", pNode5);
    
    DestroyTree(pNode5);
}

//        5
//         4
//          3
//           2
void SerializeBinaryTrees::Test3() {
    BinaryTreeNode* pNode5 = CreateBinaryTreeNode(5);
    BinaryTreeNode* pNode4 = CreateBinaryTreeNode(4);
    BinaryTreeNode* pNode3 = CreateBinaryTreeNode(3);
    BinaryTreeNode* pNode2 = CreateBinaryTreeNode(2);

    ConnectTreeNodes(pNode5, nullptr, pNode4);
    ConnectTreeNodes(pNode4, nullptr, pNode3);
    ConnectTreeNodes(pNode3, nullptr, pNode2);

    Test("Test3", pNode5);

    DestroyTree(pNode5);
}

void SerializeBinaryTrees::Test4() {
    BinaryTreeNode* pNode5 = CreateBinaryTreeNode(5);

    Test("Test4", pNode5);

    DestroyTree(pNode5);
}

void SerializeBinaryTrees::Test5() {
    Test("Test5", nullptr);
}

void SerializeBinaryTrees::Test6() {
    BinaryTreeNode* pNode1 = CreateBinaryTreeNode(5);
    BinaryTreeNode* pNode2 = CreateBinaryTreeNode(5);
    BinaryTreeNode* pNode3 = CreateBinaryTreeNode(5);
    BinaryTreeNode* pNode4 = CreateBinaryTreeNode(5);
    BinaryTreeNode* pNode5 = CreateBinaryTreeNode(5);
    BinaryTreeNode* pNode61 = CreateBinaryTreeNode(5);
    BinaryTreeNode* pNode62 = CreateBinaryTreeNode(5);
    BinaryTreeNode* pNode71 = CreateBinaryTreeNode(5);
    BinaryTreeNode* pNode72 = CreateBinaryTreeNode(5);

    ConnectTreeNodes(pNode1, nullptr, pNode2);
    ConnectTreeNodes(pNode2, nullptr, pNode3);
    ConnectTreeNodes(pNode3, pNode4, nullptr);
    ConnectTreeNodes(pNode4, pNode5, nullptr);
    ConnectTreeNodes(pNode5, pNode61, pNode62);
    ConnectTreeNodes(pNode61, pNode71, nullptr);
    ConnectTreeNodes(pNode62, nullptr, pNode72);

    Test("Test6", pNode1);

    DestroyTree(pNode1);
}

void SerializeBinaryTrees::Test() {
    Test1();
    Test2();
    Test3();
    Test4();
    Test5();
    Test6();
}
