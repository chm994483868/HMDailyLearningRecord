//
//  main.c
//  OfferReview
//
//  Created by HM C on 2023/1/2.
//

#include <iostream>

#include "BubbleSort.hpp"
#include "InsertSort.hpp"
#include "ShellSort.hpp"
#include "SelectSort.hpp"
#include "QuickSort.hpp"
#include "MergeSort.hpp"
#include "HeapSort.hpp"

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    
    // 1. 冒泡排序测试
    BubbleSort::Test();
    // 2. 插入排序测试
    InsertSort::Test();
    // 3. 希尔排序测试
    ShellSort::Test();
    // 4. 选择排序
    SelectSort::Test();
    // 5. 快速排序
    QuickSort::Test();
    // 6. 归并排序
    MergeSort::Test();
    // 7. 堆排序测试
    HeapSort::Test();
    
    return 0;
}
