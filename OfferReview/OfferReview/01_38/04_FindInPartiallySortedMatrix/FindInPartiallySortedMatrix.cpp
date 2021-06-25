//
//  FindInPartiallySortedMatrix.cpp
//  OfferReview
//
//  Created by CHM on 2020/7/27.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "FindInPartiallySortedMatrix.hpp"

bool FindInPartiallySortedMatrix::find(int* matrix, int rows, int columns, int number) {
    bool found = false;
    
    if (matrix != nullptr && rows > 0 && columns > 0) {
//        // 从右上角开始（第一行和最后一列）
//        int row = 0;
//        int column = columns - 1;
//
//        // 循环结束的条件是行数达到最大，列数达到最小
//        while (row < rows && column >= 0) {
//            // 取出值
//            int current = matrix[row * columns + column];
//
//            if (current == number) {
//                // 如果相等，即找到了
//                found = true;
//                break;
//            } else if (current > number) {
//                // 如果大于要找的值，则缩小列
//                --column;
//            } else {
//                // 如果小于要找的值，则扩大行
//                ++row;
//            }
//        }
        
        // 从左下角开始（最后一行和第一列）
        int row = rows - 1;
        int column = 0;
        
        // 循环结束的条件是列数达到最大，行数达到最小
        while (row >= 0 && column < columns) {
            // 取出值
            int current = matrix[row * columns + column];
            
            if (current == number) {
                // 如果相等，即找到了
                found = true;
                break;
            } else if (current > number) {
                // 如果大于要找的值，则缩小行
                --row;
            } else {
                // 如果小于要找的值，则扩大列
                ++column;
            }
        }
    }
    
    return found;
}

// 测试代码
void FindInPartiallySortedMatrix::Test(char* testName, int* matrix, int rows, int columns, int number, bool expected) {
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
void FindInPartiallySortedMatrix::Test1() {
    int matrix[][4] = {{1, 2, 8, 9}, {2, 4, 9, 12}, {4, 7, 10, 13}, {6, 8, 11, 15}};
    Test("Test1", (int*)matrix, 4, 4, 7, true);
}

//  1   2   8   9
//  2   4   9   12
//  4   7   10  13
//  6   8   11  15
// 要查找的数不在数组中
void FindInPartiallySortedMatrix::Test2() {
    int matrix[][4] = {{1, 2, 8, 9}, {2, 4, 9, 12}, {4, 7, 10, 13}, {6, 8, 11, 15}};
    Test("Test2", (int*)matrix, 4, 4, 5, false);
}

//  1   2   8   9
//  2   4   9   12
//  4   7   10  13
//  6   8   11  15
// 要查找的数是数组中最小的数字
void FindInPartiallySortedMatrix::Test3() {
    int matrix[][4] = {{1, 2, 8, 9}, {2, 4, 9, 12}, {4, 7, 10, 13}, {6, 8, 11, 15}};
    Test("Test3", (int*)matrix, 4, 4, 1, true);
}

//  1   2   8   9
//  2   4   9   12
//  4   7   10  13
//  6   8   11  15
// 要查找的数是数组中最大的数字
void FindInPartiallySortedMatrix::Test4() {
    int matrix[][4] = {{1, 2, 8, 9}, {2, 4, 9, 12}, {4, 7, 10, 13}, {6, 8, 11, 15}};
    Test("Test4", (int*)matrix, 4, 4, 15, true);
}

//  1   2   8   9
//  2   4   9   12
//  4   7   10  13
//  6   8   11  15
// 要查找的数比数组中最小的数字还小
void FindInPartiallySortedMatrix::Test5() {
    int matrix[][4] = {{1, 2, 8, 9}, {2, 4, 9, 12}, {4, 7, 10, 13}, {6, 8, 11, 15}};
    Test("Test5", (int*)matrix, 4, 4, 0, false);
}

//  1   2   8   9
//  2   4   9   12
//  4   7   10  13
//  6   8   11  15
// 要查找的数比数组中最大的数字还大
void FindInPartiallySortedMatrix::Test6() {
    int matrix[][4] = {{1, 2, 8, 9}, {2, 4, 9, 12}, {4, 7, 10, 13}, {6, 8, 11, 15}};
    Test("Test6", (int*)matrix, 4, 4, 16, false);
}

// 鲁棒性测试，输入空指针
void FindInPartiallySortedMatrix::Test7() {
    Test("Test7", nullptr, 0, 0, 16, false);
}

void FindInPartiallySortedMatrix::Test() {
    Test1();
    Test2();
    Test3();
    Test4();
    Test5();
    Test6();
    Test7();
}
