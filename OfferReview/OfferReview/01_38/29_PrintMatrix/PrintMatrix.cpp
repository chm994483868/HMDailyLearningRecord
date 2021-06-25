//
//  PrintMatrix.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/5.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "PrintMatrix.hpp"

void PrintMatrix::printNumber(int number) {
    printf("%d\t", number);
}

void PrintMatrix::PrintMatrixInCircle(int** numbers, int colums, int rows, int start) {
    int endX = colums - 1 - start;
    int endY = rows - 1 - start;
    
    // 从左到右打印一行（行不变，列增加）
    for (int i = start; i <= endX; ++i) {
        int number = numbers[start][i];
        printNumber(number);
    }
    
    // 从上到下打印一列（列不变，行增加）
    if (start < endY) {
        for (int i = start + 1; i <= endY; ++i) {
            int number = numbers[i][endX];
            printNumber(number);
        }
    }
    
    // 从右到左打印一行（行不变，列减小）
    if (start < endX && start < endY) {
        for (int i = endX - 1; i >= start; --i) {
            int number = numbers[endY][i];
            printNumber(number);
        }
    }
    
    // 从下到上打印一行（列不变，行减小）
    if (start < endX && start < endY - 1) {
        for (int i = endY - 1; i >= start + 1; --i) {
            int number = numbers[i][start];
            printNumber(number);
        }
    }
}

void PrintMatrix::PrintMatrixClockwisely(int** numbers, int colums, int rows) {
    if (numbers == nullptr || colums <= 0 || rows <= 0) {
        return;
    }
    
    int start = 0;
    
    // 列数大于 start 的 2 倍且行数大于 start 的 2 倍则能继续进行
    while (colums > start * 2 && rows > start * 2) {
        PrintMatrixInCircle(numbers, colums, rows, start);
        ++start;
    }
}

// 测试代码
void PrintMatrix::Test(int columns, int rows) {
    printf("Test Begin: %d columns, %d rows.\n", columns, rows);

    if(columns < 1 || rows < 1)
        return;

    int** numbers = new int*[rows];
    for(int i = 0; i < rows; ++i) {
        numbers[i] = new int[columns];
        for(int j = 0; j < columns; ++j)
        {
            numbers[i][j] = i * columns + j + 1;
        }
    }

    PrintMatrixClockwisely(numbers, columns, rows);
    printf("\n");

    for(int i = 0; i < rows; ++i)
        delete[] (int*)numbers[i];

    delete[] numbers;
}

void PrintMatrix::Test() {
    /*
    1
    */
    Test(1, 1);

    /*
    1    2
    3    4
    */
    Test(2, 2);

    /*
    1    2    3    4
    5    6    7    8
    9    10   11   12
    13   14   15   16
    */
    Test(4, 4);

    /*
    1    2    3    4    5
    6    7    8    9    10
    11   12   13   14   15
    16   17   18   19   20
    21   22   23   24   25
    */
    Test(5, 5);

    /*
    1
    2
    3
    4
    5
    */
    Test(1, 5);

    /*
    1    2
    3    4
    5    6
    7    8
    9    10
    */
    Test(2, 5);

    /*
    1    2    3
    4    5    6
    7    8    9
    10   11   12
    13   14   15
    */
    Test(3, 5);

    /*
    1    2    3    4
    5    6    7    8
    9    10   11   12
    13   14   15   16
    17   18   19   20
    */
    Test(4, 5);

    /*
    1    2    3    4    5
    */
    Test(5, 1);

    /*
    1    2    3    4    5
    6    7    8    9    10
    */
    Test(5, 2);

    /*
    1    2    3    4    5
    6    7    8    9    10
    11   12   13   14    15
    */
    Test(5, 3);

    /*
    1    2    3    4    5
    6    7    8    9    10
    11   12   13   14   15
    16   17   18   19   20
    */
    Test(5, 4);
}
