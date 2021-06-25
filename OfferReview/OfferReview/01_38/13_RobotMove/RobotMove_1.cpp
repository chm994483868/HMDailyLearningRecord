//
//  RobotMove_1.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/2.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "RobotMove_1.hpp"

int RobotMove_1::movingCoungCore(int threshold, int rows, int cols, int row, int col, bool* visited) {
    int count = 0;
    if (check(threshold, rows, cols, row, col, visited)) {
        visited[row * cols + col] = true;
        count = 1 + movingCoungCore(threshold, rows, cols, row - 1, col, visited) + movingCoungCore(threshold, rows, cols, row, col - 1, visited) + movingCoungCore(threshold, rows, cols, row + 1, col, visited) + movingCoungCore(threshold, rows, cols, row, col + 1, visited);
    }
    
    return count;
}

bool RobotMove_1::check(int threshold, int rows, int cols, int row, int col, bool* visited) {
    if (row >= 0 && row < rows && col >= 0 && col < cols && getDigitSum(row) + getDigitSum(col) <= threshold && !visited[row * cols + col]) {
        return true;
    } else {
        return false;
    }
}

int RobotMove_1::getDigitSum(int number) {
    int sum = 0;
    while (number > 0) {
        sum += number % 10;
        number /= 10;
    }
    
    return sum;
}

int RobotMove_1::movingCount(int threshold, int rows, int cols) {
    if (threshold < 0 || rows <= 0 || cols <= 0) {
        return 0;
    }
    
    bool* visited = new bool[rows * cols];
    for (int i = 0; i < rows * cols; ++i) {
        visited[i] = false;
    }
    
    int count = movingCoungCore(threshold, rows, cols, 0, 0, visited);
    
    delete [] visited;
    return count;
}

// 测试代码
void RobotMove_1::test(char* testName, int threshold, int rows, int cols, int expected) {
    if(testName != nullptr)
        printf("%s begins: ", testName);

    if(movingCount(threshold, rows, cols) == expected)
        printf("Passed.\n");
    else
        printf("FAILED.\n");
}

// 方格多行多列
void RobotMove_1::test1() {
    test("Test1", 5, 10, 10, 21);
}

// 方格多行多列
void RobotMove_1::test2() {
    test("Test2", 15, 20, 20, 359);
}
// 方格只有一行，机器人只能到达部分方格
void RobotMove_1::test3() {
    test("Test3", 10, 1, 100, 29);
}
// 方格只有一行，机器人能到达所有方格
void RobotMove_1::test4() {
    test("Test4", 10, 1, 10, 10);
}
// 方格只有一列，机器人只能到达部分方格
void RobotMove_1::test5() {
    test("Test5", 15, 100, 1, 79);
}

// 方格只有一列，机器人能到达所有方格
void RobotMove_1::test6() {
    test("Test6", 15, 10, 1, 10);
}

// 方格只有一行一列
void RobotMove_1::test7() {
    test("Test7", 15, 1, 1, 1);
}

// 方格只有一行一列
void RobotMove_1::test8() {
    test("Test8", 0, 1, 1, 1);
}

// 机器人不能进入任意一个方格
void RobotMove_1::test9() {
    test("Test9", -10, 10, 10, 0);
}

void RobotMove_1::Test() {
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
    test8();
    test9();
}
