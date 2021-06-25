//
//  MaxValueOfGifts.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/23.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "MaxValueOfGifts.hpp"

int getMaxValue_Solution1(const int* values, int rows, int cols) {
    if (values == nullptr || rows <= 0 || cols <= 0) {
        return 0;
    }
    
    int** maxValues = new int*[rows];
    for (int i = 0; i < rows; ++i) {
        maxValues[i] = new int[cols];
    }
    
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int left = 0;
            int up = 0;
            
            if (i > 0) {
                up = maxValues[i - 1][j];
            }
            
            if (j > 0) {
                left = maxValues[i][j - 1];
            }
            
            // 用 maxValues[i][j] 来记录 [i][j] 节点，从 up 或 left 过来时和的最大值
            maxValues[i][j] = std::max(left, up) + values[i * cols + j];
        }
    }
    
    int maxValue = maxValues[rows - 1][cols - 1];
    for (int i = 0; i < rows; ++i) {
        delete [] maxValues[i];
    }
    delete [] maxValues;
    
    return maxValue;
}

int getMaxValue_Solution2(const int* values, int rows, int cols) {
    if (values == nullptr || rows <= 0 || cols <= 0) {
        return 0;
    }
    
    int* maxValues = new int[cols];
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int left = 0;
            int up = 0;
            
            if (i > 0) {
                up = maxValues[j];
            }
            
            if (j > 0) {
                left = maxValues[j - 1];
            }
            
            maxValues[j] = std::max(left, up) + values[i * cols + j];
        }
    }
    
    int maxValue = maxValues[cols - 1];
    
    delete [] maxValues;
    return maxValue;
}

namespace MaxValueOfGifts_Review {
int getMaxValue_solution1(const int* values, int rows, int cols) {
    if (values == nullptr || rows <= 0 || cols <= 0) {
        return 0;
    }
    
    // 记录每个节点 从 up 和 left 过来时和当前节点的和的最大值
    int** maxValues = new int*[rows];
    for (int i = 0; i < rows; ++i) {
        maxValues[i] = new int[cols];
    }
    
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int left = 0;
            int up = 0;
            
            // 当前的节点的上面节点
            if (i > 0) {
                up = maxValues[i - 1][j];
            }
            
            // 当前节点的左面节点
            if (j > 0) {
                left = maxValues[i][j - 1];
            }
            
            maxValues[i][j] = std::max(left, up) + values[i * cols + j];
        }
    }
    
    int maxValue = maxValues[rows - 1][cols - 1];
    for (int i = 0; i < rows; ++i) {
        delete [] maxValues[i];
    }
    
    delete [] maxValues;
    
    return maxValue;
}

}
