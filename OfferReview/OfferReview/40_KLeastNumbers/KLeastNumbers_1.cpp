//
//  KLeastNumbers_1.cpp
//  OfferReview
//
//  Created by CHM on 2020/11/6.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "KLeastNumbers_1.hpp"

void KLeastNumbers_1::getLeastNumbers_Solution1(int* input, int n, int* output, int k) {
    if (input == nullptr || n <= 0 || output == nullptr || k <= 0 || k > n) {
        return;
    }
    
    int start = 0;
    int end = n - 1;
    
    int index = partition(input, n, start, end);
    
    // 调用前面的 partition 函数，找到前面最小的 k 个数，
    // 当 index 等于 k - 1 时，数组左边就是要找的 k 个数
    //（前 k 个数未必排序，但是它们是最小的 k 个数）
    
    while (index != k - 1) {
        if (index > k - 1) {
            end = index - 1;
            index = partition(input, n, start, end);
        } else {
            start = index + 1;
            index = partition(input, n, start, end);
        }
    }
    
    // 放进 output 数组输出
    for (unsigned int i = 0; i < k; ++i) {
        output[i] = input[i];
    }
}

void KLeastNumbers_1::getLeastNumbers_Solution2(const vector<int>& data, intSet& leastNumbers, int k) {
    leastNumbers.clear();
    
    if (k < 1 || data.size() < k) {
        return;
    }
    
    vector<int>::const_iterator iter = data.begin();
    
    // 从前往后遍历原始数组，leastNumbers 是一个限制元素个数为 k 的最大堆，
    // 当 leastNumbers 中元素少于 k 个时，直接把数组元素放进 leastNumbers，并会把它们构成一个最大堆，
    // 当 leastNumbers 中元素已满 k 个时，如果新来的数组元素小于大堆的最大值，则把堆此时的最大值移除，把这个新元素放入堆中。
    for (; iter != data.end(); ++iter) {
        if ((leastNumbers.size()) < k) {
            leastNumbers.insert(*iter);
        } else {
            
            // iterGreatest 是大堆的最大值
            setIterator iterGreatest = leastNumbers.begin();
            
            // 如果新来的数组元素小于大堆的最大值，则把堆此时的最大值移除，把新元素放入堆中
            if (*iter < *(leastNumbers.begin())) {
                // 移除之前的堆大值
                leastNumbers.erase(iterGreatest);
                // 插入新的元素
                leastNumbers.insert(*iter);
            }
        }
    }
}

// 测试代码
void KLeastNumbers_1::Test(char* testName, int* data, int n, int* expectedResult, int k) {
    if(testName != nullptr)
        printf("%s begins: \n", testName);

    vector<int> vectorData;
    for(int i = 0; i < n; ++ i)
        vectorData.push_back(data[i]);

    if(expectedResult == nullptr)
        printf("The input is invalid, we don't expect any result.\n");
    else
    {
        printf("Expected result: \n");
        for(int i = 0; i < k; ++ i)
            printf("%d\t", expectedResult[i]);
        printf("\n");
    }

    printf("Result for solution1:\n");
    int* output = new int[k];
    getLeastNumbers_Solution1(data, n, output, k);
    if(expectedResult != nullptr)
    {
        for(int i = 0; i < k; ++ i)
            printf("%d\t", output[i]);
        printf("\n");
    }

    delete[] output;

    printf("Result for solution2:\n");
    intSet leastNumbers;
    getLeastNumbers_Solution2(vectorData, leastNumbers, k);
    printf("The actual output numbers are:\n");
    for(setIterator iter = leastNumbers.begin(); iter != leastNumbers.end(); ++iter)
        printf("%d\t", *iter);
    printf("\n\n");
}

// k小于数组的长度
void KLeastNumbers_1::Test1() {
    int data[] = {4, 5, 1, 6, 2, 7, 3, 8};
    int expected[] = {1, 2, 3, 4};
    Test("Test1", data, sizeof(data) / sizeof(int), expected, sizeof(expected) / sizeof(int));
}

// k等于数组的长度
void KLeastNumbers_1::Test2() {
    int data[] = {4, 5, 1, 6, 2, 7, 3, 8};
    int expected[] = {1, 2, 3, 4, 5, 6, 7, 8};
    Test("Test2", data, sizeof(data) / sizeof(int), expected, sizeof(expected) / sizeof(int));
}

// k大于数组的长度
void KLeastNumbers_1::Test3() {
    int data[] = {4, 5, 1, 6, 2, 7, 3, 8};
    int* expected = nullptr;
    Test("Test3", data, sizeof(data) / sizeof(int), expected, 10);
}

// k等于1
void KLeastNumbers_1::Test4() {
    int data[] = {4, 5, 1, 6, 2, 7, 3, 8};
    int expected[] = {1};
    Test("Test4", data, sizeof(data) / sizeof(int), expected, sizeof(expected) / sizeof(int));
}

// k等于0
void KLeastNumbers_1::Test5() {
    int data[] = {4, 5, 1, 6, 2, 7, 3, 8};
    int* expected = nullptr;
    Test("Test5", data, sizeof(data) / sizeof(int), expected, 0);
}

// 数组中有相同的数字
void KLeastNumbers_1::Test6() {
    int data[] = {4, 5, 1, 6, 2, 7, 2, 8};
    int expected[] = {1, 2};
    Test("Test6", data, sizeof(data) / sizeof(int), expected, sizeof(expected) / sizeof(int));
}

// 输入空指针
void KLeastNumbers_1::Test7() {
    int* expected = nullptr;
    Test("Test7", nullptr, 0, expected, 0);
}

void KLeastNumbers_1::Test() {
    Test1();
    Test2();
    Test3();
    Test4();
    Test5();
    Test6();
    Test7();
}
