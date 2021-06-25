//
//  MoreThanHalfNumber.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/22.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "MoreThanHalfNumber.hpp"

int randomInRange(int min, int max) {
    int random = rand() % (max - min + 1) + min;
    return random;
}

void swap(int* num1, int* num2) {
    int temp = *num1;
    *num1 = *num2;
    *num2 = temp;
}

int partition(int data[], int length, int start, int end) {
    if (data == nullptr || length <= 0 || start < 0 || end >= length) {
        throw std::exception(); // 参数错误
    }
    
    // 1. 用 small 记录一个小值的位置
    int index = randomInRange(start, end); //从 （start ~ end）中随机取一个值，这是先假设为 0
    // 2. 先把 index 的值放在数组末尾
    swap(&data[index], &data[end]);
    // 3. 记录当前遇到的小于 data[index] 值的次数
    int small = start - 1;
    for (index = start; index < end; ++index) {
        if (data[index] < data[end]) {
            ++small;
            if (small != index) {
                swap(&data[index], &data[small]);
            }
        }
    }
    
    // 4. 最后把随机取到的数字放在指定的位置
    ++small;
    // 这里如果选中的数字刚好是数组最大值，small 自增以后会等于 end
    if (small != end) {
        swap(&data[small], &data[end]);
    }
    
    return small;
}

bool g_bInputInvalid = false;
bool checkInvalidArray(int* numbers, int length) {
    g_bInputInvalid = false;
    if (numbers == nullptr && length <= 0) {
        g_bInputInvalid = true;
    }
    
    return g_bInputInvalid;
}

bool checkMoreThanHalf(int* numbers, int length, int number) {
    int times = 0;
    for (int i = 0; i < length; ++i) {
        if (numbers[i] == number) {
            ++times;
        }
    }
    
    bool isMoreThanHalf = true;
    if (times * 2 <= length) {
        g_bInputInvalid = true;
        isMoreThanHalf = false;
    }
    
    return isMoreThanHalf;
}

int moreThanHalfNum_Solution1(int* numbers, int length) {
    if (checkInvalidArray(numbers, length)) {
        return 0;
    }
    
    int middle = length >> 1;
    int start = 0;
    int end = length - 1;
    int index = partition(numbers, length, start, end);
    
    while (index != middle) {
        if (index > middle) {
            end = index - 1;
            index = partition(numbers, length, start, end);
        } else {
            start = index + 1;
            index = partition(numbers, length, start, end);
        }
    }
    
    int result = numbers[middle];
    if (!checkMoreThanHalf(numbers, length, result)) {
        result = 0;
    }
    
    return result;
}

int moreThanHalfNum_Solution2(int* numbers, int length) {
    if (checkInvalidArray(numbers, length)) {
        return 0;
    }
    
    int result = numbers[0];
    int times = 1;
    
    for (int i = 1; i < length; ++i) {
        if (times == 0) {
            // 这里循环最后一个 times 置为 1 的 result
            // 就是要找的数字
            result = numbers[i];
            times = 1;
        } else if (numbers[i] == result) {
            ++times;
        } else {
            --times;
        }
    }
    
    if (!checkMoreThanHalf(numbers, length, result)) {
        result = 0;
    }
    
    return result;
}


int randomInRange_Review(int min, int max) {
    int random = rand() % (max - min + 1) + min;
    return random;
}

void swap_Review(int* num1, int* num2) {
    int temp = *num1;
    *num1 = *num2;
    *num2 = temp;
}

// 从数组中随机取一个数字，然后调整数组，使该随机数左边的值都小于它右边的值都大于它
int partition_Review(int data[], int length, int start, int end) {
    if (data == nullptr || length <= 0 || start < 0 || end >= length) {
        throw std::exception(); // 参数无效
    }
    
    int index = randomInRange_Review(start, end);
    swap_Review(&data[index], &data[end]);
    
    int small = start - 1;
    
    for (index = start; index < end; ++index) {
        if (data[index] < data[end]) {
            ++small;
            
            if (small != index) {
                swap_Review(&data[small], &data[index]);
            }
        }
    }
    
    ++small;
    if (small != end) {
        swap_Review(&data[small], &data[end]);
    }
    
    return small;
}

bool g_bInputInvalid_Review = false;
bool checkInvalidArray_Review(int* numbers, int length) {
    g_bInputInvalid_Review = false;
    if (numbers == nullptr && length <= 0) {
        g_bInputInvalid_Review = true;
    }
    
    return g_bInputInvalid_Review;
}

bool checkMoreThanHalf_Review(int* numbers, int length, int number) {
    int times = 0;
    for (int i = 0; i < length; ++i) {
        if (numbers[i] == number) {
            ++times;
        }
    }
    
    bool isMoreThanHalf = true;
    if (times * 2 <= length) {
        g_bInputInvalid_Review = true;
        isMoreThanHalf = false;
    }
    
    return isMoreThanHalf;
}

int moreThanHalfNum_Solution1_Review(int* numbers, int length) {
    if (checkInvalidArray_Review(numbers, length)) {
        return 0;
    }
    
    int middle = length >> 1;
    int start = 0;
    int end = length - 1;
    
    int index = partition_Review(numbers, length, start, end);
    
    while (index != middle) {
        if (index > middle) {
            end = index - 1;
            index = partition_Review(numbers, length, start, end);
        } else {
            start = index + 1;
            index = partition_Review(numbers, length, start, end);
        }
    }
    
    int result = numbers[middle];
    if (!checkMoreThanHalf_Review(numbers, length, result)) {
        result = 0;
    }
    
    return result;
}

int moreThanHalfNum_Solution2_Review(int* numbers, int length) {
    if (checkInvalidArray_Review(numbers, length)) {
        return 0;
    }
    
    int result = numbers[0];
    int times = 1;
    
    for (int i = 1; i < length; ++i) {
        if (times == 0) {
            result = numbers[i];
            times = 1;
        } else if (numbers[i] == result) {
            ++times;
        } else {
            --times;
        }
    }
    
    if (!checkMoreThanHalf_Review(numbers, length, result)) {
        result = 0;
    }
    
    return result;
}
