//
//  MinNumberInRotatedArray.cpp
//  OfferReview
//
//  Created by CHM on 2020/7/28.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "MinNumberInRotatedArray.hpp"

// 快速排序
unsigned int MinNumberInRotatedArray::randomInRange(unsigned int start, unsigned int end) {
    int rand = (random() % (end - start + 1)) + start;
    return rand;
}

int MinNumberInRotatedArray::partition(int data[], int length, int start, int end) {
    if (data == nullptr || length <= 0 || start < 0 || end >= length) {
        throw std::exception(); // 参数错误
    }
    
    int index = randomInRange(start, end);
    swap(&data[index], &data[end]);
    int small = start - 1;
    for (index = start; index < end; ++index) {
        if (data[index] < data[end]) {
            ++small;
            
            if (small != index) {
                swap(&data[small], &data[index]);
            }
        }
    }
    
    ++small;
    if (small != end) {
        swap(&data[small], &data[end]);
    }
    
    return small;
}

void MinNumberInRotatedArray::swap(int* num1, int* num2) {
    int temp = *num1;
    *num1 = *num2;
    *num2 = temp;
}

void MinNumberInRotatedArray::quickSort(int data[], int length, int start, int end) {
    if (start == end) {
        return;
    }
    
    int index = partition(data, length, start, end);
    if (index > start) {
        quickSort(data, length, start, index - 1);
    }
    
    if (index < end) {
        quickSort(data, length, index + 1, end);
    }
}

// 员工年龄排序（计数排序）
void MinNumberInRotatedArray::sortAges(int ages[], int length) {
    if (ages == nullptr || length <= 0) {
        return;
    }
    
    const int oldestAge = 99;
    int timesOfAge[oldestAge + 1];
    int i = 0;
    for (; i <= oldestAge; ++i) {
        timesOfAge[i] = 0;
    }
    
    for (i = 0; i < length; ++i) {
        int age = ages[i];
        if (age < 0 || age > oldestAge) {
            throw std::exception(); // 年龄超过范围
        }
        
        ++timesOfAge[age];
    }
    
    int index = 0;
    for (i = 0; i <= oldestAge; ++i) {
        int count = timesOfAge[i];
        while (count > 0) {
            ages[index] = i;
            ++index;
            --count;
        }
    }
}

int MinNumberInRotatedArray::minInorder(int* numbers, int index1, int index2) {
    int result = numbers[index1];
    for (int i = index1 + 1; i <= index2; ++i) {
        if (result > numbers[i]) {
            result = numbers[i];
        }
    }
    
    return result;
}

int MinNumberInRotatedArray::min(int* numbers, int length) {
    if (numbers == nullptr || length <= 0) {
        throw std::exception(); // 参数错误
    }
    
    int index1 = 0;
    int index2 = length - 1;
    int indexMid = index1;
    while (numbers[index1] >= numbers[index2]) {
        if (index2 - index1 == 1) {
            indexMid = index2;
            break;
        }
        
        indexMid = ((index2 - index1) >> 1) + index1;
        if (numbers[index1] == numbers[index2] && numbers[indexMid] == numbers[index1]) {
            return minInorder(numbers, index1, index2);
        }
        
        if (numbers[indexMid] >= numbers[index1]) {
            index1 = indexMid;
        } else if (numbers[indexMid] <= numbers[index2]) {
            index2 = indexMid;
        }
    }
    
    return numbers[indexMid];
}

// 测试代码
void MinNumberInRotatedArray::Test(int* numbers, int length, int expected) {
    int result = 0;
    try {
        result = min(numbers, length);

        for(int i = 0; i < length; ++i)
            printf("%d ", numbers[i]);

        if(result == expected)
            printf("\tpassed\n");
        else
            printf("\tfailed\n");
    } catch (...) {
        if(numbers == nullptr)
            printf("Test passed.\n");
        else
            printf("Test failed.\n");
    }
}

void MinNumberInRotatedArray::Test() {
    // 典型输入，单调升序的数组的一个旋转
    int array1[] = { 3, 4, 5, 1, 2 };
    Test(array1, sizeof(array1) / sizeof(int), 1);

    // 有重复数字，并且重复的数字刚好的最小的数字
    int array2[] = { 3, 4, 5, 1, 1, 2 };
    Test(array2, sizeof(array2) / sizeof(int), 1);

    // 有重复数字，但重复的数字不是第一个数字和最后一个数字
    int array3[] = { 3, 4, 5, 1, 2, 2 };
    Test(array3, sizeof(array3) / sizeof(int), 1);

    // 有重复的数字，并且重复的数字刚好是第一个数字和最后一个数字
    int array4[] = { 1, 0, 1, 1, 1 };
    Test(array4, sizeof(array4) / sizeof(int), 0);

    // 单调升序数组，旋转0个元素，也就是单调升序数组本身
    int array5[] = { 1, 2, 3, 4, 5 };
    Test(array5, sizeof(array5) / sizeof(int), 1);

    // 数组中只有一个数字
    int array6[] = { 2 };
    Test(array6, sizeof(array6) / sizeof(int), 2);

    // 输入nullptr
    Test(nullptr, 0, 0);
}

class Solution {
public:
    int minArray(vector<int>& numbers) {
        if (numbers.empty()) {
            return -1;
        }
        
        while (numbers.size() > 1 && numbers.back() == numbers[0]) numbers.pop_back();
        
        unsigned long index1 = 0;
        unsigned long index2 = numbers.size() - 1;
        unsigned long indexMid = index1;
        
        while (numbers[index1] >= numbers[index2]) {
            if (index2 - index1 == 1) {
                indexMid = index2;
                break;
            }
            
            indexMid = ((index2 - index1) >> 1) + index1;
            
//            if (numbers[index1] == numbers[index2] && numbers[index1] == numbers[indexMid]) {
//                int min = numbers[0];
//                for (int i = 1; i < numbers.size(); ++i) {
//                    if (numbers[i] < min) {
//                        min = numbers[i];
//                    }
//                }
//                return min;
//            }
            
            if (numbers[indexMid] >= numbers[index1]) {
                index1 = indexMid;
            } else if (numbers[indexMid] <= numbers[index2]) {
                index2 = indexMid;
            }
        }
        
        return numbers[indexMid];
    }
};
