//
//  StackPushPopOrder.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/17.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "StackPushPopOrder.hpp"

bool isPopOrder(const int* pPush, const int* pPop, int nLength) {
    bool bPossible = false;
    
    if (pPush != nullptr && pPop != nullptr && nLength > 0) {
        // 1. 先分别准备指向 压入序列的头指针、弹出序列的头指针和辅助栈
        const int* pNextPush = pPush;
        const int* pNextPop = pPop;
        std::stack<int> stackData;
        
        // 2. 如果弹出序列的指针没有移动到最后则继续进行循环
        // 这个循环的本质就是依次找到 《弹出序列》 中的元素
        while (pNextPop - pPop < nLength) {
            // 3. 开始时辅助栈是空的
            //    当入和出的第一个元素相同时，第一次循环后 stackData 也变为 empty 了
            //    当前栈顶的元素不是要弹出的元素
            
            // 当辅助栈的栈顶元素不是要弹出的元素
            // 先压入一些数字入栈
            while (stackData.empty() || stackData.top() != *pNextPop) {
                // 如果所有数字都压入辅助栈了，退出循环
                
                // 4. 如果指向压入序列的指针已经移动到了该序列的最后一个元素的后面，
                //    则说明压入序列已经为空了，则跳出循环，（尽管可能没有找到对应的要弹出的元素）
                if (pNextPush - pPush == nLength)
                    break;
                
                // 5. 把压入序列的元素放进栈中
                stackData.push(*pNextPush);
                
                // 6. 移动压入序列的指针
                ++pNextPush;
            }
            
            // 7. 如果此时，栈顶元素还是和弹出序列元素不同，且上面的 while 结束了，有一种情况是
            //    pNexpush - pPush == nLength 了，且栈顶的元素和 *pNextPop 元素不同
            if (stackData.top() != *pNextPop)
                break;
            
            // 8. 找到了相同的元素，辅助栈弹出栈顶元素，弹出序列指针往后移动一位
            stackData.pop();
            ++pNextPop;
        }
        
        // 9. 如果辅助栈为空了，弹出序列指针也移动到最后一个元素后面了，
        //    说明弹出序列是压入序列的一个弹出序列
        if (stackData.empty() && pNextPop - pPop == nLength)
            bPossible = true;
    }
    
    return bPossible;
}

bool isPopOrder_Review(const int* pPush, const int* pPop, int nLength) {
    bool bPossible = false;
    
    if (pPush != nullptr && pPop != nullptr && nLength > 0) {
        
        const int* pNextPush = pPush;
        const int* pNextPop = pPop;
        std::stack<int> stackData;
        
        while (pNextPop - pPop < nLength) {
            
            while (stackData.empty() || stackData.top() != *pNextPop) {
                
                if (pNextPush - pPush == nLength)
                    break;
                
                stackData.push(*pNextPush);
                ++pNextPush;
            }
            
            if (stackData.top() != *pNextPop)
                break;
            
            stackData.pop();
            ++pNextPop;
        }
        
        if (stackData.empty() && pNextPop - pPop == nLength)
            bPossible = true;
    }
    
    return bPossible;
}
