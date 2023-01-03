//
//  main.c
//  OfferReview
//
//  Created by HM C on 2023/1/2.
//

#include <iostream>
#include "BubbleSort.hpp"
#include "HeapSort.hpp"

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    
    // 1. 冒泡排序测试
    BubbleSort::Test();
    // 2. 堆排序测试
    HeapSort::Test();
    
    return 0;
}
