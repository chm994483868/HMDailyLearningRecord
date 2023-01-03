//
//  FirstAppearingOnce.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/10.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "FirstAppearingOnce.hpp"

void FirstAppearingOnce::Test(const char* testName, CharStatistics chars, char expected) {
    if(testName != nullptr)
        printf("%s begins: ", testName);

    if(chars.firstAppearingOnce() == expected)
        printf("Passed.\n");
    else
        printf("FAILED.\n");
}

void FirstAppearingOnce::Test() {
    CharStatistics chars;

    Test("Test1", chars, '\0');

    chars.Insert('g');
    Test("Test2", chars, 'g');

    chars.Insert('o');
    Test("Test3", chars, 'g');

    chars.Insert('o');
    Test("Test4", chars, 'g');

    chars.Insert('g');
    Test("Test5", chars, '\0');

    chars.Insert('l');
    Test("Test6", chars, 'l');

    chars.Insert('e');
    Test("Test7", chars, 'l');
}
