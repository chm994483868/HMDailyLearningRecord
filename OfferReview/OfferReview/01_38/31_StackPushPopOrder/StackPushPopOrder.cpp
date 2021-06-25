//
//  StackPushPopOrder.cpp
//  OfferReview
//
//  Created by CHM on 2020/7/30.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "StackPushPopOrder.hpp"

// 两个序列一个是入栈序列，一个是出栈序列，这里入栈序列中的元素不是一次全部入栈的，
// 而是部分元素入栈了，然后直接又出栈了，随后后面从没入栈过的元素再接着入栈出栈，
// 而出栈序列正是记录这些出栈元素的顺序。

bool StackPushPopOrder::isPopOrder(const int* pPush, const int* pPop, int nLength) {
    bool bPossible = false;
    
    if (pPush != nullptr && pPop != nullptr && nLength > 0) {
        
        // pNextPush 和 pNextPop 分别从头到尾指向入栈和出栈序列的每个元素
        const int* pNextPush = pPush;
        const int* pNextPop = pPop;
        
        // 准备一个栈，把每个入栈序列中的元素入栈再出栈，用来模拟出栈序列的出栈过程
        //（当栈顶元素和 *pNextPop 不等时，要继续从 pNextPush 往后查找，直到找到一个相等的元素入栈）
        stack<int> stackData;
        
        // 循环条件是 pNextPush 指向出栈序列的末尾
        while (pNextPop - pPop < nLength) {
            
            // 把 pPush 序列中的元素入栈，直到找到一个等于 pNextPop 的元素出栈
            while (stackData.empty() || stackData.top() != *pNextPop) {
                
                // 判断 pNextPush 是否到了入栈序列的末尾
                if (pNextPush - pPush == nLength) {
                    break;
                }
                
                // 把 pNextPush 入栈 stackData
                stackData.push(*pNextPush);
                // pNextPush 指针往后移动
                ++pNextPush;
            }
            
            // 如果在 pPush 一直找不到一个和 *pNextPop 相等的元素，则表明这 pPop 不是该栈的弹出序列
            if (stackData.top() != *pNextPop) {
                break;
            }
            
            // 找到了一个弹出的元素
            stackData.pop();
            // 找到了一个弹出元素，pNextPop 也往后移动一位
            ++pNextPop;
        }
        
        // 最后 pNextPop 到了出栈序列的末尾， stackData 中全部元素也都弹出了，则表明 pPop 是弹出序列
        if (stackData.empty() && pNextPop - pPop == nLength) {
            bPossible = true;
        }
    }
    
    // 返回 bPossible
    //（这里可能有一个固定思维，这里是入栈序列不是已经固定的栈，如果是固定的栈，那么它的弹出序列也是唯一的，
    // 这里是入栈的序列，它们可以对应很多种不同的出栈序列，入栈序列不是一次全部入栈的，后入栈的元素可以任意等待栈内的元素是否出栈后它再进行入栈）
    return bPossible;
}

// 测试代码
void StackPushPopOrder::Test(const char* testName, const int* pPush, const int* pPop, int nLength, bool expected) {
    if(testName != nullptr)
        printf("%s begins: ", testName);

    if(isPopOrder(pPush, pPop, nLength) == expected)
        printf("Passed.\n");
    else
        printf("failed.\n");
}

void StackPushPopOrder::Test1() {
    const int nLength = 5;
    int push[nLength] = {1, 2, 3, 4, 5};
    int pop[nLength] = {4, 5, 3, 2, 1};
    
    Test("Test1", push, pop, nLength, true);
}

void StackPushPopOrder::Test2() {
    const int nLength = 5;
    int push[nLength] = {1, 2, 3, 4, 5};
    int pop[nLength] = {3, 5, 4, 2, 1};
    
    Test("Test2", push, pop, nLength, true);
}

void StackPushPopOrder::Test3() {
    const int nLength = 5;
    int push[nLength] = {1, 2, 3, 4, 5};
    int pop[nLength] = {4, 3, 5, 1, 2};
    
    Test("Test3", push, pop, nLength, false);
}

void StackPushPopOrder::Test4() {
    const int nLength = 5;
    int push[nLength] = {1, 2, 3, 4, 5};
    int pop[nLength] = {3, 5, 4, 1, 2};
    
    Test("Test4", push, pop, nLength, false);
}

// push和pop序列只有一个数字
void StackPushPopOrder::Test5() {
    const int nLength = 1;
    int push[nLength] = {1};
    int pop[nLength] = {2};

    Test("Test5", push, pop, nLength, false);
}

void StackPushPopOrder::Test6() {
    const int nLength = 1;
    int push[nLength] = {1};
    int pop[nLength] = {1};

    Test("Test6", push, pop, nLength, true);
}

void StackPushPopOrder::Test7() {
    Test("Test7", nullptr, nullptr, 0, false);
}

void StackPushPopOrder::Test() {
    Test1();
    Test2();
    Test3();
    Test4();
    Test5();
    Test6();
    Test7();
}
