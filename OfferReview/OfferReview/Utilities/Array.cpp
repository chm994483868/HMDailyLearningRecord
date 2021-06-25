//
//  Array.cpp
//  OfferReview
//
//  Created by HM C on 2020/7/30.
//  Copyright © 2020 CHM. All rights reserved.
//

#include "Array.hpp"

void Partition::printArray(std::string desc, int nums[], int count) {
    // 打印数组
    std::cout << desc;
    for (int i = 0; i < count; i++) {
        std::cout << " " << nums[i] << " ";
    }
    
    std::cout << "\n";
}

void Partition::swap(int* num1, int* num2) {
    // 交换两个值，入参用的指针
    int temp = *num1;
    *num1 = *num2;
    *num2 = temp;
}

int Partition::randomInRange(int min, int max) {
    // 取模用的 (max - min + 1) 谨记
    int random = rand() % (max - min + 1) + min;
    return random;
}

int Partition::partition(int data[], int length, int start, int end) {
    // 多添加了 start < 0 和 end >= length，即开始值不能小于 0，结束值不能大于等于长度
    if (data == nullptr || length <= 0 || start < 0 || end >= length) {
        throw std::exception(); // 参数错误
    }
    
    // 取 start 和 end 中间的一个随机值
    int index = randomInRange(start, end);
    // 把取到的随机值放在数据尾部
    swap(&data[index], &data[end]);
    // 用 small 记录当前小于随机值的位置，初始用 start - 1
    int small = start - 1;
    // 从 start 开始遍历，到 end 之前结束，目前 end 位置放的是取到的随机值
    for (index = start; index < end; ++index) {
        // 如果遍历到的值小于随机值
        if (data[index] < data[end]) {
            // small 自增
            ++small;
            
            // 如果当前 index 的值小于随机值，small 卡在从左到右第一个大于随机值的位置
            // 如果 small 不等于 index
            // 则交换当前的小于随机值的值和从左到右第一个大于随机值的值
            if (small != index) {
                swap(&data[small], &data[index]);
            }
        }
    }
    
    // 遍历完毕后，small 再做一次自增，因为前面的遍历结束的条件是 < end
    ++small;
    // 如果与 end 不同，则交换 small 和 end
    if (small != end) {
        swap(&data[small], &data[end]);
    }
    
    return small;
}
