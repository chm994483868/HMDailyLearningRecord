//
//  FindInPartiallySortedMatrix_1.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/1.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "FindInPartiallySortedMatrix_1.hpp"

bool FindInPartiallySortedMatrix_1::find(int* matrix, int rows, int columns, int number) {
    bool found = false;
    
    if (matrix != nullptr && rows > 0 && columns > 0) {
//        // 从右上角开始
//        int row = 0;
//        int column = columns - 1;
//
//        while (row < rows && column >= 0) {
//            int current = matrix[row * columns + column];
//            if (current == number) {
//                found = true;
//                break;
//            } else if (current > number) {
//                --column;
//            } else {
//                ++row;
//            }
//        }
        
        // 从左下角开始
        int row = rows - 1;
        int column = 0;
        
        while (row >= 0 && column < columns) {
            int current = matrix[row * columns + column];
            if (current == number) {
                found = true;
                break;
            } else if (current > number) {
                --row;
            } else {
                ++column;
            }
        }
    }
    
    return found;
}

// 测试代码
void FindInPartiallySortedMatrix_1::Test(char* testName, int* matrix, int rows, int columns, int number, bool expected) {
    if(testName != nullptr)
        printf("%s begins: ", testName);

    bool result = find(matrix, rows, columns, number);
    if(result == expected)
        printf("Passed.\n");
    else
        printf("Failed.\n");
}

//  1   2   8   9
//  2   4   9   12
//  4   7   10  13
//  6   8   11  15
// 要查找的数在数组中
void FindInPartiallySortedMatrix_1::Test1() {
    int matrix[][4] = {{1, 2, 8, 9}, {2, 4, 9, 12}, {4, 7, 10, 13}, {6, 8, 11, 15}};
    Test("Test1", (int*)matrix, 4, 4, 7, true);
}

//  1   2   8   9
//  2   4   9   12
//  4   7   10  13
//  6   8   11  15
// 要查找的数不在数组中
void FindInPartiallySortedMatrix_1::Test2() {
    int matrix[][4] = {{1, 2, 8, 9}, {2, 4, 9, 12}, {4, 7, 10, 13}, {6, 8, 11, 15}};
    Test("Test2", (int*)matrix, 4, 4, 5, false);
}

//  1   2   8   9
//  2   4   9   12
//  4   7   10  13
//  6   8   11  15
// 要查找的数是数组中最小的数字
void FindInPartiallySortedMatrix_1::Test3() {
    int matrix[][4] = {{1, 2, 8, 9}, {2, 4, 9, 12}, {4, 7, 10, 13}, {6, 8, 11, 15}};
    Test("Test3", (int*)matrix, 4, 4, 1, true);
}

//  1   2   8   9
//  2   4   9   12
//  4   7   10  13
//  6   8   11  15
// 要查找的数是数组中最大的数字
void FindInPartiallySortedMatrix_1::Test4() {
    int matrix[][4] = {{1, 2, 8, 9}, {2, 4, 9, 12}, {4, 7, 10, 13}, {6, 8, 11, 15}};
    Test("Test4", (int*)matrix, 4, 4, 15, true);
}

//  1   2   8   9
//  2   4   9   12
//  4   7   10  13
//  6   8   11  15
// 要查找的数比数组中最小的数字还小
void FindInPartiallySortedMatrix_1::Test5() {
    int matrix[][4] = {{1, 2, 8, 9}, {2, 4, 9, 12}, {4, 7, 10, 13}, {6, 8, 11, 15}};
    Test("Test5", (int*)matrix, 4, 4, 0, false);
}

//  1   2   8   9
//  2   4   9   12
//  4   7   10  13
//  6   8   11  15
// 要查找的数比数组中最大的数字还大
void FindInPartiallySortedMatrix_1::Test6() {
    int matrix[][4] = {{1, 2, 8, 9}, {2, 4, 9, 12}, {4, 7, 10, 13}, {6, 8, 11, 15}};
    Test("Test6", (int*)matrix, 4, 4, 16, false);
}

// 鲁棒性测试，输入空指针
void FindInPartiallySortedMatrix_1::Test7() {
    Test("Test7", nullptr, 0, 0, 16, false);
}

void FindInPartiallySortedMatrix_1::Test() {
    Test1();
    Test2();
    Test3();
    Test4();
    Test5();
    Test6();
    Test7();
}
