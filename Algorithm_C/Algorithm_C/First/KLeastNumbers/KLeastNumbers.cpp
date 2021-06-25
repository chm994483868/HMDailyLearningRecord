//
//  KLeastNumbers.cpp
//  Algorithm_C
//
//  Created by CHM on 2020/7/22.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "KLeastNumbers.hpp"

namespace KLeastNumbers {

int randomInRange(int min, int max) {
    int random = rand() % (max - min + 1) + min;
    return random;
}

void swap(int* num1, int* num2) {
    int temp = *num1;
    *num1 = *num2;
    *num2 = temp;
}

int partition(int* data, int length, int start, int end) {
    if (data == nullptr || length <= 0 || start < 0 || end >= length) {
        throw std::exception(); // 参数错误
    }

    // 1. 从 start ～ end 中随机取一个值，作为随机下标
    int index = randomInRange(start, end);
    // 2. 把这个随机下标对应的值和数组最后一个元素做交换
    swap(&data[index], &data[end]);
    // 3. small 标记从 -1 开始，遍历数组时，每次遇到比随机值小的数字时自增 1
    // 当遇到比随机值大的数字时，会停下来，等再遇到比随机值小的数字时，用于交换数组元素。
    int small = start - 1;

    // 4. 调整数组，数组左边都是小于随机值的元素，数组右边都是大于随机值的元素。
    for (index = start; index < end; ++index) {
        if (data[index] < data[end]) {
            ++small;

            if (small != index) {
                swap(&data[small], &data[index]);
            }
        }
    }

    // 5. 遍历结束，small 自增 1 ，指向一个大于 data[end] 的数值
    ++small;
    if (small != end) {
        // 6. 交换值，使这个随机值位于正确的位置，左边都是大于它的元素，右边都是大于它的元素
        // 并在下面返回这个随机值的下标
        swap(&data[small], &data[end]);
    }

    return small;
}

void getLeastNumbers_Solution1(int* input, int n, int* output, int k) {
    if (input == nullptr || output == nullptr || k > n || n <= 0 || k<= 0) {
        return;
    }

    int start = 0;
    int end = n - 1;

    int index = partition(input, n, start, end);

    while (index != k - 1) {
        if (index > k - 1) {
            end = index - 1;
            index = partition(input, n, start, end);
        } else {
            start = index + 1;
            index = partition(input, n, start, end);
        }
    }

    for (int i = 0; i < k; ++i) {
        output[i] = input[i];
    }
}

typedef multiset<int, std::greater<int>> intSet; // 一个有序的集合，begin -> end 递减
typedef multiset<int, std::greater<int>>::iterator setIterator;

void getLeastNumbers_Solution2(const vector<int>& data, intSet& leastNumbers, int k) {
    // 1. 清除
    leastNumbers.clear();
    // 2. 判断参数是否有效
    if (k < 1 || data.size() < k) {
        return;
    }

    // 3. data 数据入口
    vector<int>::const_iterator iter = data.begin();

    // 4. 迭代遍历 data
    for (; iter != data.end(); ++iter) {

        // 5. 如果 leastNumbers 还没有被填满数据，则把 data 数据直接插入
        if ((leastNumbers.size()) < k) {
            leastNumbers.insert(*iter);
        } else {
            // 6. 如果 leastNumbers 装满了，首先取得 leastNumbers 里的最大值
            // 如果当前 data 的数值小于 leastNumbers 的最大值，则把 leastNumbers 里的
            // 最大值移除了，并把 data 的数值插入 leastNumbers 里面
            setIterator iterGreatest = leastNumbers.begin();
            if (*iter < *(leastNumbers.begin())) {
                leastNumbers.erase(iterGreatest);
                leastNumbers.insert(*iter);
            }
        }

    }
}

void getLeastNumbers_Solution1_Review(int* input, int n, int* output, int k) {
    if (input == nullptr || output == nullptr || n <= 0 || k > n || k <= 0) {
        return;
    }
    
    int start = 0;
    int end = n - 1;
    
    int index = partition(input, n, start, end);
    
    while (index != k - 1) {
        if (index > k - 1) {
            end = index - 1;
            index = partition(input, n, start, end);
        } else {
            start = index + 1;
            index = partition(input, n, start, end);
        }
    }
    
    for (index = 0; index < k; ++index) {
        output[index] = input[index];
    }
}

void getLeastNumbers_Solution2_Review(const vector<int>& data, intSet& leastNumbers, int k) {
    leastNumbers.clear();
    
    if (k < 1 || data.size() < k) {
        return;
    }
    
    vector<int>::const_iterator iter = data.begin();
    for (; iter != data.end(); ++iter) {
        if (leastNumbers.size() < k) {
            leastNumbers.insert(*iter);
        } else {
            setIterator iterGreatest = leastNumbers.begin();
            if (*iter < *(leastNumbers.begin())) {
                leastNumbers.erase(iterGreatest);
                leastNumbers.insert(*iter);
            }
        }
    }
}


}

