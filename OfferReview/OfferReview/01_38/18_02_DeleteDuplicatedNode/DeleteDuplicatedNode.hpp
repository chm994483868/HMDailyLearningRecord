//
//  DeleteDuplicatedNode.hpp
//  OfferReview
//
//  Created by CHM on 2020/7/29.
//  Copyright © 2020 CHM. All rights reserved.
//

#ifndef DeleteDuplicatedNode_hpp
#define DeleteDuplicatedNode_hpp

#include <stdio.h>
#include <stdlib.h>
#include "List.hpp"

using namespace List;

namespace DeleteDuplicatedNode {

// 18（二）：删除链表中重复的结点
// 题目：在一个排序的链表中，如何删除重复的结点？例如，在图3.4（a）中重复
// 结点被删除之后，链表如图3.4（b）所示。

void deleteDuplication(ListNode** pHead);

// 测试代码
void Test(char* testName, ListNode** pHead, int* expectedValues, int expectedLength);
// 某些结点是重复的
void Test1();
// 没有重复的结点
void Test2();
// 除了一个结点之外其他所有结点的值都相同
void Test3();
// 所有结点的值都相同
void Test4();
// 所有结点都成对出现
void Test5();
// 除了两个结点之外其他结点都成对出现
void Test6();
// 链表中只有两个不重复的结点
void Test7();
// 结点中只有一个结点
void Test8();
// 结点中只有两个重复的结点
void Test9();
// 空链表
void Test10();

void Test();

}

#endif /* DeleteDuplicatedNode_hpp */
