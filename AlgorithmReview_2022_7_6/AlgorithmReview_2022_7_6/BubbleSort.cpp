//
//  BubbleSort.cpp
//  AlgorithmReview_2022_7_6
//
//  Created by HM C on 2022/7/6.
//

#include <stdio.h>

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void bubbleSort(int nums[], int count) {
    if (nums == nullptr || count <= 0) {
        return;
    }
    
    int k = count - 1; // 第二层循环的边界
    for (int i = 0; i < count - 1; ++i) { // 边界是 < count - 1 是因为和其它排序方式相比，它是每次把最大或者最小值放在序列的最右侧了，比如插入和选择排序则是把最小值或者最大值放在序列的最左侧
        bool noExchange = true; // 标记一趟比较操作下来是否发生过交换，如果没有的话表示当前序列已经有序
        int n = 0; // 记录一趟排序中最后一次交换的两个相邻元素的最小下标，此后该最小下标以后的数据就都是有序的
        for (int j = 0; j < k; ++j) {
            if (nums[j] > nums[j + 1]) {
                swap(&nums[j], &nums[j + 1]);
                noExchange = false;
                n = j;
            }
        }
        
        if (noExchange) {
            break;
        }
        
        k = n; // 更新 K 值
    }
}
