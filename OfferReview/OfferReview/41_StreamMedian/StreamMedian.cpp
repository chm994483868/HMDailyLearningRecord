//
//  StreamMedian.cpp
//  OfferReview
//
//  Created by CHM on 2020/7/31.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "StreamMedian.hpp"

// 测试代码
void StreamMedian::Test(char* testName, DynamicArray<double>& numbers, double expected) {
    if(testName != nullptr)
        printf("%s begins: ", testName);
    
    if(abs(numbers.getMedian() - expected) < 0.0000001)
        printf("Passed.\n");
    else
        printf("FAILED.\n");
}

void StreamMedian::Test() {
    DynamicArray<double> numbers;
    
    printf("Test1 begins: ");
    try
    {
        numbers.getMedian();
        printf("FAILED.\n");
    }
    catch(const exception&)
    {
        printf("Passed.\n");
    }
    
    numbers.insert(5);
    Test("Test2", numbers, 5);
    
    numbers.insert(2);
    Test("Test3", numbers, 3.5);
    
    numbers.insert(3);
    Test("Test4", numbers, 3);
    
    numbers.insert(4);
    Test("Test6", numbers, 3.5);
    
    numbers.insert(1);
    Test("Test5", numbers, 3);
    
    numbers.insert(6);
    Test("Test7", numbers, 3.5);
    
    numbers.insert(7);
    Test("Test8", numbers, 4);
    
    numbers.insert(0);
    Test("Test9", numbers, 3.5);
    
    numbers.insert(8);
    Test("Test10", numbers, 4);
}
