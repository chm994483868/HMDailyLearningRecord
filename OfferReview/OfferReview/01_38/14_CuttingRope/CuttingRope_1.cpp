//
//  CuttingRope_1.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/3.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "CuttingRope_1.hpp"

int CuttingRope_1::maxProductAfterCutting_solution1(int length) {
    if (length < 2) {
        return 0;
    }
    if (length == 2) {
        return 1;
    }
    if (length == 3) {
        return 2;
    }
    
    int* products = new int[length + 1];
    products[0] = 0;
    products[1] = 1;
    products[2] = 2;
    products[3] = 3;
    
    int max = 0;
    for (int i = 4; i <= length; ++i) {
        max = 0;
        for (int j = 1; j <= i / 2; ++j) {
            int product = products[j] * products[i - j];
            if (max < product) {
                max = product;
            }
            
            products[i] = max;
        }
    }
    
    max = products[length];
    delete [] products;
    return max;
}

int CuttingRope_1::maxProductAfterCutting_solution2(int length) {
    if (length < 2) {
        return 0;
    }
    if (length == 2) {
        return 1;
    }
    if (length == 3) {
        return 2;
    }
    
    int timesOf3 = length / 3;
    if (length - timesOf3 * 3 == 1) {
        timesOf3 -= 1;
    }
    
    int timesOf2 = (length - timesOf3 * 3) / 2;
    
    return (int) (pow(3, timesOf3)) * (int) (pow(2, timesOf2));
}

// 测试代码
void CuttingRope_1::test(const char* testName, int length, int expected) {
    int result1 = maxProductAfterCutting_solution1(length);
    if(result1 == expected)
        std::cout << "Solution1 for " << testName << " passed." << std::endl;
    else
        std::cout << "Solution1 for " << testName << " FAILED." << std::endl;

    int result2 = maxProductAfterCutting_solution2(length);
    if(result2 == expected)
        std::cout << "Solution2 for " << testName << " passed." << std::endl;
    else
        std::cout << "Solution2 for " << testName << " FAILED." << std::endl;
}

void CuttingRope_1::test1() {
    int length = 1;
    int expected = 0;
    test("test1", length, expected);
}

void CuttingRope_1::test2() {
    int length = 2;
    int expected = 1;
    test("test2", length, expected);
}

void CuttingRope_1::test3() {
    int length = 3;
    int expected = 2;
    test("test3", length, expected);
}

void CuttingRope_1::test4() {
    int length = 4;
    int expected = 4;
    test("test4", length, expected);
}

void CuttingRope_1::test5() {
    int length = 5;
    int expected = 6;
    test("test5", length, expected);
}

void CuttingRope_1::test6() {
    int length = 6;
    int expected = 9;
    test("test6", length, expected);
}

void CuttingRope_1::test7() {
    int length = 7;
    int expected = 12;
    test("test7", length, expected);
}

void CuttingRope_1::test8() {
    int length = 8;
    int expected = 18;
    test("test8", length, expected);
}

void CuttingRope_1::test9() {
    int length = 9;
    int expected = 27;
    test("test9", length, expected);
}

void CuttingRope_1::test10() {
    int length = 10;
    int expected = 36;
    test("test10", length, expected);
}

void CuttingRope_1::test11() {
    int length = 50;
    int expected = 86093442;
    test("test11", length, expected);
}

void CuttingRope_1::Test() {
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
    test8();
    test9();
    test10();
    test11();
}
